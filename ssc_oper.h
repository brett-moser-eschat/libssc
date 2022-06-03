/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2006 Nokia Corporation.
 *
 * Contact: Kai Vehmanen <kai.vehmanen@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/*
 * This file has been changed by San Luis Aviation Inc. starting on 2013-04-11
 * All changes made to this library are made freely available under the original terms of the GNU Lesser General Public License version 2.1
 */

#ifndef HAVE_SSC_OPER_H
#define HAVE_SSC_OPER_H

/**@file ssc_oper.h Handling active operations initated 
 *       by the application.
 *
 * @author Kai Vehmanen <Kai.Vehmanen@nokia.com>
 * @author Pekka Pessi <Pekka.Pessi@nokia.com>
 */

typedef struct ssc_oper_s ssc_oper_t;

/* define type of context pointers for callbacks */
#define NUA_IMAGIC_T    ssc_oper_t
#define NUA_HMAGIC_T    ssc_oper_t

#include <sofia-sip/nua.h>
#include <sofia-sip/sip.h>
#include <sofia-sip/sip_header.h>
#include <sofia-sip/sip_status.h>
#include <sofia-sip/su_debug.h>

#if HAVE_FUNC
#define enter (void)SU_DEBUG_9(("%s: entering\n", __func__))
#elif HAVE_FUNCTION
#define enter (void)SU_DEBUG_9(("%s: entering\n", __FUNCTION__))
#else
#define enter (void)0
#endif

struct ssc_oper_s {
  ssc_oper_t   *op_next;

  /**< Remote end identity
   *
   * Contents of To: when initiating, From: when receiving.
   */
  char const   *op_ident;	

  /** NUA handle */ 
  nua_handle_t *op_handle;
  
  ssc_t        *op_ssc;         /**< backpointer to owner */
  
  /** How this handle was used initially */
  sip_method_t  op_method;	/**< REGISTER, INVITE, MESSAGE, or SUBSCRIBE */
  char const   *op_method_name;

	const sip_t *sip; /* sip exploded struct of the last message received on this
	                     operation context. (NULL if there is no sip message that
	                     we are aware of) */
  /** Call state. 
   *
   * - opc_sent when initial INVITE has been sent
   * - opc_recv when initial INVITE has been received
   * - opc_complate when 200 Ok has been sent/received
   * - opc_active when media is used
   * - opc_sent when re-INVITE has been sent
   * - opc_recv when re-INVITE has been received
   */
  enum { 
    opc_none, 
    opc_sent = 1, 
    opc_recv = 2, 
    opc_complete = 3, 
    opc_active = 4,
    opc_sent_hold = 8,             /**< Call put on hold */
    opc_pending = 16               /**< Waiting for local resources */
  } op_callstate;

  int           op_prev_state;     /**< Previous call state */

  unsigned      op_persistent : 1; /**< Is this handle persistent? */
  unsigned      op_referred : 1;
  unsigned :0;

  void *userData; /* magic for callbacks */
  void *oc_node; /*used by operator container, do not touch! */
};

// search the SSC operation list for a matching SIP
// operation context.
// note: currently this is just a linear search of the list. Since there will be
//   an existing operation for every established registration, established session,
//   and in-progress transaction, there could be quite a number of these allocated
//   at one time.  So maybe consider switching this to a hash on to-uri+from-uri .
// @param[in] ssc      ptr to SSC context
// @param[in] touri    To: URI to match. Comparison is case sensitive.
//                     URI is parsed and only the username and host portions are used for the comparison
//                     TODO: allow wildcard matching?
// @param[in] fromuri  From: URI to match. Comparison is case sensitive.
//                     URI is parsed and only the username and host portions are used for the comparison
// @param[in] method   [optional] if provided, limit to operations that are running the indicated
//                                SIP method
// @return pointer to the matching SSC operation or NULL if none found.
ssc_oper_t *ssc_oper_find(
      ssc_t *ssc,
      const char *touri,
      const char *fromuri);
ssc_oper_t *ssc_oper_find_method(
      ssc_t *ssc,
      const char *touri,
      const char *fromuri,
      sip_method_t method);

// parameter structure used to create SSC operations
typedef struct ssc_oper_create_s
{
   unsigned use_handle : 1;   // if set, then use the NUA handle provided
                              // in the struct. otherwise, allocate a new one.

   unsigned use_uri : 1;      // if set, then to-URI and from-URI are specified as
                              // pointers to sofia structs, otherwise they are c-strings.
   union
   {
      struct
      {
         // these items are used when @p use_handle is unset,
         // indicating that a new NUA handle is to be created.
         nua_t *nua;  // must be set to a valid NUA context for the new handle.
         const char *req_addr; // URI to associate with the new handle
      } create;

      struct
      {
         // these items are used when @p use_handle is set, indicating
         // that a pre-existing NUA handle is being provided.
         nua_handle_t *handle; // pointer to the NUA handle to use with the new operation.
         const sip_from_t *from; // from URI.  FIXME: I don't know what SSC was using this for.
                                 // it gets used to set op_ident field of the operation, but
                                 // it's not obvious how that is later used or intended to be used.
      } use;
   } nh;

   union
   {
      // this section holds the URIs that are to be applied as a key
      // for operation lookups.  format depends on the @p use_uri setting.
      struct
      {
         // these are used when @p use_uri is unset.
         const char *to;   // c-string representing the To-URI
         const char *from; // c-string representing the From-URI
      } str;

      struct
      {
         // these are used when @p use_uri is set.
         const sip_to_t *to;     // sofia struct representing the To-URI
         const sip_from_t *from; // sofia struct representing the From-URI
      } uri;
   } id;

} ssc_oper_create_t;

// create a new SIP operation context. the creation is
// performed according to the ssc_oper_create_t parameter
// structure defined above.
// if successful, the new operation will be added to the
// SSC's operation container for later lookup.
//
// @return on success, returns a pointer to the new operation
//         context object. NULL on failure.
ssc_oper_t *ssc_oper_create(
      ssc_t *ssc,
      sip_method_t method,
      const char *name,
      const ssc_oper_create_t *params,
      tag_type_t tag, tag_value_t value, ...);

// release a SIP operation context and remove it from the
// SSC's operation list.
void ssc_oper_destroy(ssc_t *ssc, ssc_oper_t *op);

void ssc_oper_assign(ssc_oper_t *op, sip_method_t method, char const *name);

ssc_oper_t *ssc_oper_find_call(ssc_t *ssc);
ssc_oper_t *ssc_oper_find_call_in_progress(ssc_t *ssc);
ssc_oper_t *ssc_oper_find_call_embryonic(ssc_t *ssc);
ssc_oper_t *ssc_oper_find_unanswered(ssc_t *ssc);
ssc_oper_t *ssc_oper_find_by_handle(ssc_t *ssc, nua_handle_t *handle); 
ssc_oper_t *ssc_oper_find_by_method(ssc_t *ssc, sip_method_t method);
ssc_oper_t *ssc_oper_find_by_callstate(ssc_t *ssc, int callstate);
ssc_oper_t *ssc_oper_find_register(ssc_t *ssc);
ssc_oper_t *ssc_oper_check(ssc_t *ssc, ssc_oper_t *op);

#endif /* HAVE_SSC_OPER_H */
