/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *  Copyright (C) 2020 flexiWAN Ltd.
 *  This file is part of the FWABF plugin.
 *  The FWABF plugin is fork of the FDIO VPP ABF plugin.
 *  It enhances ABF with functionality required for Flexiwan Multi-Link feature.
 *  For more details see official documentation on the Flexiwan Multi-Link.
 */

#ifndef __FWABF_H__
#define __FWABF_H__

#define FWABF_PLUGIN_VERSION_MAJOR 1
#define FWABF_PLUGIN_VERSION_MINOR 0


#include <plugins/fwabf/fwabf_links.h>

/**
 * An Flexiwan ACL Based Forwarding 'policy'.
 * This comprises the ACL index to match against and the forwarding
 * path to take if the match is successful.
 *
 * In comparison to original ABF plugin where the FWABF was forked of,
 * the FWABF policy uses flexiwan path labels and other criteria to choose link
 * for packet forwarding. The link can be either WAN interface if Direct Internet
 * Access (DIA) is wanted by user for this packet, of VXLAN/GRE tunnel.
 * To use flexiwan path labels user has to label Tunnel and WAN interfaces.
 * The links might be groupped, so user can prioritize groups of links to choose
 * link from.
 * FWABF policy consist of paclet class agains which packet should be matched,
 * and action to be performed on match. The packet class is implemented by ACL
 * rule and is referenced by ACL index. The action is specified in this module.
 *
 * ABF policies are then 'attached' to interfaces onto unicast-ip4/6 arcs.
 * When vlib buffer is received by FWABF vlib graph node, it is matched against
 * ACL database. If match was found, the packet will be routed by policy.
 * If no match was found, the vlib buffer will continue down to the interface's
 * feature arc.
 * In case of match, the policy is fetched using the matched ACL rule-id,
 * as it was provided by user on policy creation, and it identifies the policy
 * by 1:1 relation. Than policy labels are used to find right WAN interface or
 * tunnel to forward packet on, and the correspondent DPO is fetched
 * from the fwabf_sw_interface_db database.
 */

typedef enum fwabf_selection_alg_t_ {
    FWABF_SELECTION_RANDOM,
    FWABF_SELECTION_ORDERED
} fwabf_selection_alg_t;

typedef struct fwabf_policy_link_group_t_ {
    fwabf_selection_alg_t alg;     /* Choose link randomly or use list order */
    fwabf_label_t*        links;   /* List of links. For now (March 2020) links can be choosen by labels only */
} fwabf_policy_link_group_t;

typedef enum {
    FWABF_FALLBACK_DEFAULT_ROUTE,
    FWABF_FALLBACK_DROP
} fwabf_fallback_t;

typedef struct fwabf_policy_action_t_
{
    fwabf_fallback_t           fallback;  /* If no usable link was found - drop or use input-ip4/6 feature arc */
    fwabf_selection_alg_t      alg;       /* Choose group of links randomly or use list order */
    fwabf_policy_link_group_t* link_groups;
} fwabf_policy_action_t;

typedef struct abf_policy_t_
{
  /**
   * Linkage into the FIB graph
   */
  //fib_node_t ap_node;       // moved to fwabf_sw_interface_t

  /**
   * ACL index to match.
   * The ACL rule implements policy packet class.
   */
  u32 ap_acl;

  /**
   * Policy action - what link to use for packet forwarding.
   */
  fwabf_policy_action_t action;

  // nnoww - moved to fwabf_sw_interface_t
  /**
   * The path-list describing how to forward in case of a match
   */
  //fib_node_index_t ap_pl;

  // nnoww - moved to fwabf_sw_interface_t
  /**
   * Sibling index on the path-list
   */
  //u32 ap_sibling;

  /**
   * The policy ID - as configured by the client
   */
  u32 ap_id;

} abf_policy_t;

/**
 * Get an ABF object from its VPP index
 */
extern abf_policy_t *fwabf_policy_get (index_t index);

/**
 * Get an DPO to use for packet forwarding according to policy
 *
 * @param index     index of abf_policy_t in pool
 * @param dpo_proto the IP4/IP6
 * @return VPP's object index
 */
extern dpo_id_t fwabf_policy_get_dpo (index_t index, dpo_proto_t dpo_proto);

/**
 * Find a ABF object from the client's policy ID
 *
 * @param policy_id Client's defined policy ID
 * @return VPP's object index
 */
extern index_t fwabf_policy_find (u32 policy_id);

// nnoww - moved to fwabf_sw_interface_t
/**
 * The FIB node type for ABF policies
 */
//extern fib_node_type_t abf_policy_fib_node_type;

/**
 * Create an ABF Policy
 *
 * @param policy_id User defined Policy ID
 * @param acl_index The ACL the policy with match on (packet class)
 * @param action The action to be made in case of match
 */
extern u32 abf_policy_add (
              u32                     policy_id,
			        u32                     acl_index,
			        fwabf_policy_action_t * action);

/**
 * Delete policy.
 *
 * @param policy_id User defined Policy ID
 */
extern int abf_policy_delete (u32 policy_id);

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */

#endif
