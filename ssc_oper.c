/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2005-2006 Nokia Corporation.
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

/**@file ssc_sip.c Interface towards libsofia-sip-ua.
 * 
 * @author Kai Vehmanen <kai.vehmanen@nokia.com>
 * @author Pekka Pessi <pekka.pessi@nokia.com>
 */

/*
 * Status:
 *  - works
 *
 * Todo:
 *  - separete ssc_t into sofsip_cli and ssc_sip specific
 *    structs
 *
 * Notes:
 *  - none
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <sofia-sip/su.h>

#include "ssc_log.h"

#include "ssc_sip.h"
#include "ssc_oper.h"
#include "ssc_oper_container.h"

static ssc_oper_t *priv_ssc_oper_create(
      ssc_t *ssc,
      nua_t *nua,
      sip_method_t method,
      const char *name,
      const char *address,
      tag_type_t tag, tag_value_t value, ...);

static ssc_oper_t *priv_ssc_oper_create_with_handle(
      ssc_t *ssc,
      sip_method_t method,
      const char *name,
      nua_handle_t *nh,
      const sip_from_t *from);

ssc_oper_t *ssc_oper_find(
      ssc_t *ssc,
      const char *toUri,
      const char *fromUri)
{
   return ssc_oper_find_method(ssc, toUri, fromUri, sip_method_unknown);
}
      
ssc_oper_t *ssc_oper_find_method(
      ssc_t *ssc,
      const char *toUri,
      const char *fromUri,
      sip_method_t method)
{
   sip_to_t *toUrl = sip_to_make(ssc->ssc_home, toUri);
   if (!toUrl)
   {
      return NULL;
   }

   sip_to_t *fromUrl = sip_to_make(ssc->ssc_home, fromUri);
   if (!fromUrl)
   {
      su_free(ssc->ssc_home, toUrl);
      return NULL;
   }

   const char *matchToUser = "<nil>";
   const char *matchToHost = "<nil>";
   const char *matchFromUser = "<nil>";
   const char *matchFromHost = "<nil>";

   if (toUrl->a_url)
   {
      if (toUrl->a_url->url_user) { matchToUser = toUrl->a_url->url_user; }
      if (toUrl->a_url->url_host) { matchToHost = toUrl->a_url->url_host; }
   }

   if (fromUrl->a_url)
   {
      if (fromUrl->a_url->url_user) { matchFromUser = fromUrl->a_url->url_user; }
      if (fromUrl->a_url->url_host) { matchFromHost = fromUrl->a_url->url_host; }
   }

   SSCDebugMed("%s: looking for existing op for: <to-'%s@%s', from-'%s@%s'>..", __func__,
         matchToUser, matchToHost,
         matchFromUser, matchFromHost);

   ssc_oper_t *opc = ssc->ssc_operations;
   ssc_oper_t *match = NULL;

   while (opc)
   {
      if (opc->op_handle)
      {
         const sip_to_t *cmpToUrl  = nua_handle_remote(opc->op_handle);
         const sip_to_t *cmpFromUrl = nua_handle_local(opc->op_handle);
         const char *cmpToUser = "<nil>";
         const char *cmpToHost = "<nil>";
         const char *cmpFromUser = "<nil>";
         const char *cmpFromHost = "<nil>";

         if (!cmpToUrl || !cmpFromUrl)
         {
            SSCDebugMed("%s:  skipping op with bad uri..", __func__);
            opc = opc->op_next;
            continue;
         }

         if (cmpToUrl->a_url)
         {
            if (cmpToUrl->a_url->url_user) { cmpToUser = cmpToUrl->a_url->url_user; }
            if (cmpToUrl->a_url->url_host) { cmpToHost = cmpToUrl->a_url->url_host; }
         }

         if (cmpFromUrl->a_url)
         {
            if (cmpFromUrl->a_url->url_user) { cmpFromUser = cmpFromUrl->a_url->url_user; }
            if (cmpFromUrl->a_url->url_host) { cmpFromHost = cmpFromUrl->a_url->url_host; }
         }

         SSCDebugMed("%s:  chk op: <to-'%s@%s', from-'%s@%s'>", __func__,
               cmpToUser, cmpToHost,
               cmpFromUser, cmpFromHost);


         if ( strcasecmp(matchToUser, cmpToUser) == 0 &&
              strcasecmp(matchToHost, cmpToHost) == 0 &&
              strcasecmp(matchFromUser, cmpFromUser) == 0 &&
              strcasecmp(matchFromHost, cmpFromHost) == 0 )
         {
            if (method <= sip_method_unknown || opc->op_method == method)
            {
               SSCDebugMed("%s: match found!", __func__);
               match = opc;
               break;
            }
         }
      }

      opc = opc->op_next;
   }

   if (!match)
   {
      SSCDebugMed("%s: no match found!", __func__);
   }

   // release the temporary objs created for url parsing
   su_free(ssc->ssc_home, toUrl);
   su_free(ssc->ssc_home, fromUrl);

   return match;
}

ssc_oper_t *ssc_oper_create(
      ssc_t *ssc,
      sip_method_t method,
      const char *name,
      const ssc_oper_create_t *param,
      tag_type_t tag, tag_value_t value, ...)
{
   ssc_oper_t *op = NULL;

   ta_list ta;
   ta_start(ta, tag, value);

   if (param->use_handle)
   {
      // create SSC operation using existing NUA handle
      op = priv_ssc_oper_create_with_handle(
            ssc,
            method, name,
            param->nh.use.handle,
            param->nh.use.from);
   }
   else
   {
      // create SSC operation using new NUA handle
      op = priv_ssc_oper_create(
            ssc,
            param->nh.create.nua,
            method, name,
            param->nh.create.req_addr,
            ta_tags(ta));
   }

   ta_end(ta);

   if (op)
   {
      /* add the operation to the op container */

      int rv = OC_FAILURE;

      if (param->use_uri)
      {
         /* uri is sofia struct */
         rv = ssc_oc_add_op_uri(
               ssc, op,
               param->id.uri.to,
               param->id.uri.from);
      }
      else
      {
         /* uri is c-string */
         rv = ssc_oc_add_op_uri_str(
               ssc, op,
               param->id.str.to,
               param->id.str.from);
      }

      if (rv != OC_SUCCESS)
      {
         // we failed to store the operation, so release it and bail out.
         SSCError("%s: failed to add new operation to container", __func__);
         ssc_oper_destroy(ssc, op);
         op = NULL;
      }
   }

   return op;
}

/**
 * Creates a new operation object and stores it the list of
 * active operations for 'cli'.
 */
ssc_oper_t *priv_ssc_oper_create(ssc_t *ssc, 
             nua_t *nua,
			    sip_method_t method,
			    char const *name,
			    char const *address,
			    tag_type_t tag, tag_value_t value, ...)
{
  ssc_oper_t *op, *old;

  ta_list ta;
   
  enter;
  
  if (address) {
    int have_url = 1;
    sip_to_t *to;

    to = sip_to_make(ssc->ssc_home, address);

    if (to == NULL) {
      SSCDebugHigh("%s: %s: invalid address: %s", ssc->ssc_name, name, address);
      return NULL;
    }

    /* Try to make sense out of the URL */
    if (url_sanitize(to->a_url) < 0) {
      SSCDebugHigh("%s: %s: invalid address", ssc->ssc_name, name);
      return NULL;
    }

    if (!(op = su_zalloc(ssc->ssc_home, sizeof(*op)))) {
      SSCDebugHigh("%s: %s: cannot create handle", ssc->ssc_name, name);
      return NULL;
    }

    op->op_next = ssc->ssc_operations;
    op->op_prev_state = -1;
    op->op_ssc = ssc;
    ssc->ssc_operations = op;      

    if (method == sip_method_register)
      have_url = 0;
    
    ta_start(ta, tag, value); 
     
	 op->op_handle = nua_handle(nua, op, TAG_IF(have_url, NUTAG_URL(to->a_url)),SIPTAG_TO(to), ta_tags(ta));

    ta_end(ta);  
     
    op->op_ident = sip_header_as_string(ssc->ssc_home, (sip_header_t *)to);

    ssc_oper_assign(op, method, name);
    
    su_free(ssc->ssc_home, to);
  }
  else if (method || name) 
    ssc_oper_assign(op = old, method, name);
  else
    return old;

  if (!op) {
    if (address)
      SSCDebugHigh("%s: %s: invalid destination", ssc->ssc_name, name);
    else
      SSCDebugHigh("%s: %s: no destination", ssc->ssc_name, name);
    return NULL;
  }

  return op;
}

/**
 * Creates an operation handle and binds it to
 * an existing handle 'nh' (does not create a new nua 
 * handle with nua_handle()).
 */
ssc_oper_t *priv_ssc_oper_create_with_handle(ssc_t *ssc, 
					sip_method_t method,
					char const *name,
					nua_handle_t *nh,
					sip_from_t const *from)
{
  ssc_oper_t *op;

  enter;

  if ((op = su_zalloc(ssc->ssc_home, sizeof(*op)))) {
    op->op_next = ssc->ssc_operations;
    ssc->ssc_operations = op;      

    ssc_oper_assign(op, method, name);
    nua_handle_bind(op->op_handle = nh, op);
    op->op_ident = sip_header_as_string(ssc->ssc_home, (sip_header_t*)from);
    op->op_ssc = ssc;
  }
  else {
    SSCDebugHigh("%s: cannot create operation object for %s", 
	   ssc->ssc_name, name);
  }

  return op;
}

/** 
 * Deletes operation and attached handles and identities 
 */
void ssc_oper_destroy(ssc_t *ssc, ssc_oper_t *op)
{
  ssc_oper_t **prev;
  int active_invites = 0;

  if (!op)
    return;

  ssc_oc_rem_op(ssc, op);

  /* Remove from queue */
  for (prev = &ssc->ssc_operations; 
       *prev && *prev != op; 
       prev = &(*prev)->op_next)
    ;
  if (*prev)
    *prev = op->op_next, op->op_next = NULL;

  if (op->op_handle)
    nua_handle_destroy(op->op_handle), op->op_handle = NULL;

  for (prev = &ssc->ssc_operations; 
       *prev; 
       prev = &(*prev)->op_next) {
    if ((*prev)->op_method == sip_method_invite) ++active_invites;
  }


   if (ssc->ssc_oper_destroyed_cb)
   {
      ssc->ssc_oper_destroyed_cb(op->userData);
   }

  su_free(ssc->ssc_home, op);
}

/**
 * Assigns flags to operation object based on method type
 */
void ssc_oper_assign(ssc_oper_t *op, sip_method_t method, char const *name)
{
  if (!op)
    return;

  op->op_method = method, op->op_method_name = name;

  op->op_persistent = 
    method == sip_method_subscribe ||
    method == sip_method_register ||
    method == sip_method_publish;

	op->sip = NULL;
}

/**
 * Finds a call operation (an operation that has non-zero
 * op_callstate).
 */
ssc_oper_t *ssc_oper_find_call(ssc_t *ssc)
{
  ssc_oper_t *op;

  for (op = ssc->ssc_operations; op; op = op->op_next)
    if (op->op_callstate)
      break;

  return op;
}

/**
 * Finds call operation that is in process.
 */
ssc_oper_t *ssc_oper_find_call_in_progress(ssc_t *ssc)
{
  ssc_oper_t *op;

  for (op = ssc->ssc_operations; op; op = op->op_next)
    if (op->op_callstate & opc_sent) /* opc_sent bit is on? */
      break;

  return op;
}

ssc_oper_t *ssc_oper_find_call_embryonic(ssc_t *ssc)
{
  ssc_oper_t *op;

  for (op = ssc->ssc_operations; op; op = op->op_next)
    if (op->op_callstate == 0 && op->op_method == sip_method_invite)
      break;

  return op;
}
  
/**
 * Finds an unanswered call operation.
 */
ssc_oper_t *ssc_oper_find_unanswered(ssc_t *ssc)
{
  ssc_oper_t *op;

  for (op = ssc->ssc_operations; op; op = op->op_next)
    if (op->op_callstate == opc_recv)
      break;

  return op;
}

/**
 * Finds an operation by nua handle.
 */
ssc_oper_t *ssc_oper_find_by_handle(ssc_t *ssc, nua_handle_t *handle)
{
  ssc_oper_t *op;

  for (op = ssc->ssc_operations; op; op = op->op_next)
    if (op->op_handle == handle)
      break;

  return op;
}

/**
 * Finds an operation by method.
 */
ssc_oper_t *ssc_oper_find_by_method(ssc_t *ssc, sip_method_t method)
{
  ssc_oper_t *op;

  for (op = ssc->ssc_operations; op; op = op->op_next)
    if (op->op_method == method && op->op_persistent)
      break;

  return op;
}

/** 
 * Finds an operation by call state
 */
ssc_oper_t *ssc_oper_find_by_callstate(ssc_t *ssc, int callstate)
{
  ssc_oper_t *op;

  for (op = ssc->ssc_operations; op; op = op->op_next)
    if (op->op_callstate & callstate)
      break;

  return op;
}

/**
 * Find a register operation.
 */
ssc_oper_t *ssc_oper_find_register(ssc_t *ssc)
{
  ssc_oper_t *op;

  for (op = ssc->ssc_operations; op; op = op->op_next)
    if (op->op_method == sip_method_register && op->op_persistent)
      break;

  return op;
}

/**
 * Checks whether 'op' is a valid handle or not.
 *
 * @return op if valid, NULL otherwise
 */
ssc_oper_t *ssc_oper_check(ssc_t *ssc, ssc_oper_t *op)
{
  ssc_oper_t *tmp;

  for (tmp = ssc->ssc_operations; tmp; tmp = op->op_next)
    if (tmp == op)
      return op;

  return NULL;
}
