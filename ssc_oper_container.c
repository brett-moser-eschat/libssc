
/// added by San Luis Aviation Inc. Not part of original SSC
/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2013-2022 San Luis Aviation Inc.
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

#include "ssc_oper_container.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sofia-sip/sip.h>
#include <sofia-sip/su.h>

#include "ssc_log.h"
#include "ssc_sip.h"
#include "ssc_oper.h"

#define OC_BUCKETS 4098
#define OC_NUM_METHODS 15

#define OC_OP_DESTROY 1
#define OC_OP_NO_DESTROY 0

#define OC_METHOD_MATCH_ANY  0
#define OC_METHOD_MATCH_ID  1
#define OC_METHOD_MATCH_NAME  2

struct ssc_op_container_s;

typedef struct ssc_oc_str_s
{
   unsigned len;
   unsigned hash;
   char *str;
} ssc_oc_str_t;

typedef struct ssc_oc_uri_s
{
   ssc_oc_str_t user;
   ssc_oc_str_t host;
} ssc_oc_uri_t;

typedef struct ssc_oc_op_s
{
   ssc_oper_t *op;
   
   struct ssc_op_container_s *oc;

   unsigned s_index;
   unsigned m_index;

   ssc_oc_uri_t to;
   ssc_oc_uri_t from;

   sip_method_t method;
   ssc_oc_str_t method_name;

   struct ssc_oc_op_s *next;
   struct ssc_oc_op_s *prev;
} ssc_oc_op_t;

typedef struct ssc_slot_s
{
   ssc_oc_op_t *method[OC_NUM_METHODS];
} ssc_slot_t;

typedef struct ssc_op_container_s
{
   unsigned count;

   ssc_slot_t slot[OC_BUCKETS];
} ssc_op_container_t;

typedef struct ssc_oc_id_s
{
   const char *to_user;
   const char *to_host;
   const char *from_user;
   const char *from_host;
} ssc_oc_id_t;

typedef struct ssc_oc_find_param_s
{
   ssc_oc_id_t id;
   unsigned match_method;
   unsigned m_index;
   const char *m_name;
} ssc_oc_find_param_t;

#define PRIV_OC_EXTRACT_URI(x, u, h) \
   if (x && x->a_url) \
   { \
      if (x->a_url->url_user) { u = x->a_url->url_user; } \
      if (x->a_url->url_host) { h = x->a_url->url_host; } \
   } 

static int priv_oc_hash_str_cpy(const ssc_t *ssc, const char *str, ssc_oc_str_t *hash_str);
static int priv_oc_hash_str(const char *str, ssc_oc_str_t *hash_str);
static int priv_oc_cmp_str(const ssc_oc_str_t *strA, const ssc_oc_str_t *strB) __attribute__ ((unused));
static void priv_oc_free(ssc_t *ssc, int opDestroy);
static void priv_oc_str_free(const ssc_t *ssc, ssc_oc_str_t *s);
static void priv_oc_op_free(const ssc_t *ssc, ssc_oc_op_t *oc_op);
static unsigned priv_oc_count_elements(const ssc_oc_op_t *oc_op);
static int priv_oc_add(ssc_t *ssc, ssc_oper_t *op, const ssc_oc_id_t *id);
static ssc_oper_t *priv_oc_find(const ssc_op_container_t *oc, const ssc_oc_find_param_t *findParams);
static ssc_oper_t *priv_oc_find_any(ssc_oc_op_t *l, const ssc_oc_uri_t *to, const ssc_oc_uri_t *from);
static ssc_oper_t *priv_oc_find_method_name(ssc_oc_op_t *l, const ssc_oc_uri_t *to, const ssc_oc_uri_t *from, const ssc_oc_str_t *name);

void ssc_oc_free(ssc_t *ssc)
{
   return priv_oc_free(ssc, OC_OP_DESTROY);
}

void ssc_oc_free_no_destroy(ssc_t *ssc)
{
   return priv_oc_free(ssc, OC_OP_NO_DESTROY);
}

unsigned ssc_oc_size(const ssc_t *ssc)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return 0;
   }

   ssc_op_container_t *oc = (ssc_op_container_t *)ssc->ssc_oc;

   unsigned s = 0;
   if (oc)
   {
      s = oc->count;
   }

   return s;
}

unsigned ssc_op_size_by_method(const ssc_t *ssc, sip_method_t method)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return 0;
   }

   if (method < sip_method_invalid)
   {
      SSCError("%s: method enum out of bounds (val: %d, min: %d)", __func__, method, sip_method_invalid);
      return 0;
   }

   unsigned size=0;
   unsigned mindex=0;
   
   if (method != sip_method_invalid)
   {
      mindex = (unsigned)method;
      if (mindex >= OC_NUM_METHODS)
      {
         SSCError("%s: method enum out of bounds (val: %u, max: %u)", __func__, mindex, OC_NUM_METHODS-1);
         return 0;
      }
   }

   ssc_op_container_t *oc = (ssc_op_container_t *)ssc->ssc_oc;

   unsigned i,m;
   for (i=0; i<OC_BUCKETS; ++i)
   {
      if (method == sip_method_invalid)
      {
         for (m=0; m<OC_NUM_METHODS; ++m)
         {
            size += priv_oc_count_elements(oc->slot[i].method[m]);
         }
      }
      else
      {
         size += priv_oc_count_elements(oc->slot[i].method[mindex]);
      }
   }

   return size;
}

int ssc_oc_add_op(ssc_t *ssc, ssc_oper_t *op)
{
   if (!op) { return OC_FAILURE; }
   if (op->oc_node)
   {
      SSCError("%s: ssc op already in container", __func__);
      return OC_FAILURE;
   }

   const sip_to_t *opTo = nua_handle_remote(op->op_handle);
   const sip_to_t *opFrom = nua_handle_local(op->op_handle);
   const char *opToUser = "<nil>";
   const char *opToHost = "<nil>";
   const char *opFromUser = "<nil>";
   const char *opFromHost = "<nil>";

   if (!opTo)
   {
      SSCError("%s: ssc operation has bad To URI", __func__);
      return OC_FAILURE;
   }

   if (!opFrom)
   {
      SSCError("%s: ssc operation has bad From URI", __func__);
      return OC_FAILURE;
   }

   PRIV_OC_EXTRACT_URI(opTo, opToUser, opToHost);
   PRIV_OC_EXTRACT_URI(opFrom, opFromUser, opFromHost);

   ssc_oc_id_t id;
   id.to_user = opToUser;
   id.to_host = opToHost;
   id.from_user = opFromUser;
   id.from_host = opFromHost;

   return priv_oc_add(ssc, op, &id);
}

int ssc_oc_add_op_uri(ssc_t *ssc, ssc_oper_t *op, const sip_to_t *to, const sip_to_t *from)
{
   if (!op) { return OC_FAILURE; }
   if (op->oc_node)
   {
      SSCError("%s: ssc op already in container", __func__);
      return OC_FAILURE;
   }
   
   const char *opToUser = "<nil>";
   const char *opToHost = "<nil>";
   const char *opFromUser = "<nil>";
   const char *opFromHost = "<nil>";

   PRIV_OC_EXTRACT_URI(to, opToUser, opToHost);
   PRIV_OC_EXTRACT_URI(from, opFromUser, opFromHost);

   ssc_oc_id_t id;
   id.to_user = opToUser;
   id.to_host = opToHost;
   id.from_user = opFromUser;
   id.from_host = opFromHost;

   return priv_oc_add(ssc, op, &id);
}

int ssc_oc_add_op_uri_str(ssc_t *ssc, ssc_oper_t *op, const char *to, const char *from)
{
   if (!op) { return OC_FAILURE; }
   if (op->oc_node)
   {
      SSCError("%s: ssc op already in container", __func__);
      return OC_FAILURE;
   }
   
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return OC_FAILURE;
   }

   if (!ssc->ssc_home)
   {
      SSCError("%s: NULL ssc home context ptr", __func__);
      return OC_FAILURE;
   }

   const char *opToUser = "<nil>";
   const char *opToHost = "<nil>";
   const char *opFromUser = "<nil>";
   const char *opFromHost = "<nil>";

   sip_to_t *toUri = NULL;
   sip_to_t *fromUri = NULL;

   if (to)
   {
      toUri = sip_to_make(ssc->ssc_home, to);
   }

   if (from)
   {
      fromUri = sip_to_make(ssc->ssc_home, from);
   }

   PRIV_OC_EXTRACT_URI(toUri, opToUser, opToHost);
   PRIV_OC_EXTRACT_URI(fromUri, opFromUser, opFromHost);

   ssc_oc_id_t id;
   id.to_user = opToUser;
   id.to_host = opToHost;
   id.from_user = opFromUser;
   id.from_host = opFromHost;

   int rv = priv_oc_add(ssc, op, &id);

   if (toUri)
   {
      su_free(ssc->ssc_home, toUri);
   }

   if (fromUri)
   {
      su_free(ssc->ssc_home, fromUri);
   }

   return rv;
}

void ssc_oc_rem_op(const ssc_t *ssc, ssc_oper_t *op)
{
   if (!ssc) { return; }
   if (!ssc->ssc_oc) { return; }
   if (!op) { return; }
   if (!op->oc_node) { return; }

   ssc_op_container_t *oc = ssc->ssc_oc;
   ssc_oc_op_t *oc_op = (ssc_oc_op_t *)op->oc_node;

   if (!oc_op->oc)
   {
      SSCError("%s: oc node is corrupted", __func__);
      return;
   }

   if (oc != oc_op->oc)
   {
      SSCError("%s: attempt to remove op from wrong container", __func__);
      return;
   }

   if (!oc_op->prev)
   {
      // this node is at the head. update head to point to next.
      oc->slot[oc_op->s_index].method[oc_op->m_index] = oc_op->next;
   }
   else
   {
      oc_op->prev->next = oc_op->next;
   }

   if (oc_op->next)
   {
      oc_op->next->prev = oc_op->prev;
   }

   oc_op->prev = NULL;
   oc_op->next = NULL;
   op->oc_node = NULL;

   if (oc->count > 0) { --oc->count; }
   
   priv_oc_op_free(ssc, oc_op);
}

ssc_oper_t *ssc_oc_find_uri(const ssc_t *ssc, const sip_to_t *to, const sip_to_t *from)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return NULL;
   }

   const ssc_op_container_t *oc = ssc->ssc_oc;
   if (!oc)
   {
      SSCError("%s: ssc context does not have an operation container", __func__);
      return NULL;
   }

   ssc_oper_t *op = NULL;
   ssc_oc_find_param_t findParams;

   findParams.match_method = OC_METHOD_MATCH_ANY;

   findParams.id.to_user = "<nil>";
   findParams.id.to_host = "<nil>";
   findParams.id.from_user = "<nil>";
   findParams.id.from_host = "<nil>";

   PRIV_OC_EXTRACT_URI(to, findParams.id.to_user, findParams.id.to_host);
   PRIV_OC_EXTRACT_URI(from, findParams.id.from_user, findParams.id.from_host);

   op = priv_oc_find(oc, &findParams);

   return op;
}

ssc_oper_t *ssc_oc_find_uri_str(const ssc_t *ssc, const char *to, const char *from)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return NULL;
   }

   const ssc_op_container_t *oc = ssc->ssc_oc;
   if (!oc)
   {
      SSCError("%s: ssc context does not have an operation container", __func__);
      return NULL;
   }

   ssc_oper_t *op = NULL;
   ssc_oc_find_param_t findParams;

   findParams.match_method = OC_METHOD_MATCH_ANY;

   findParams.id.to_user = "<nil>";
   findParams.id.to_host = "<nil>";
   findParams.id.from_user = "<nil>";
   findParams.id.from_host = "<nil>";

   sip_to_t *toUri = NULL;
   sip_from_t *fromUri = NULL;

   if (to)
   {
      toUri = sip_to_make(ssc->ssc_home, to);
   }

   if (from)
   {
      fromUri = sip_to_make(ssc->ssc_home, from);
   }

   PRIV_OC_EXTRACT_URI(toUri, findParams.id.to_user, findParams.id.to_host);
   PRIV_OC_EXTRACT_URI(fromUri, findParams.id.from_user, findParams.id.from_host);

   op = priv_oc_find(oc, &findParams);

   if (toUri)
   {
      su_free(ssc->ssc_home, toUri);
   }

   if (fromUri)
   {
      su_free(ssc->ssc_home, fromUri);
   }

   return op;
}

ssc_oper_t *ssc_oc_find_uri_method(const ssc_t *ssc, const sip_to_t *to, const sip_to_t *from, sip_method_t method)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return NULL;
   }

   const ssc_op_container_t *oc = ssc->ssc_oc;
   if (!oc)
   {
      SSCError("%s: ssc context does not have an operation container", __func__);
      return NULL;
   }

   if (method < sip_method_invalid ||
       (unsigned)method >= OC_NUM_METHODS)
   {
      SSCError("%s: op method out of range. val: %d (%d..%u)", __func__,
         method, sip_method_invalid, OC_NUM_METHODS-1);
      return NULL;
   }

   ssc_oper_t *op = NULL;
   ssc_oc_find_param_t findParams;

   findParams.match_method = OC_METHOD_MATCH_ANY;
   if (method >= sip_method_unknown)
   {
      findParams.match_method = OC_METHOD_MATCH_ID;
      findParams.m_index = (unsigned)method;
   }

   findParams.id.to_user = "<nil>";
   findParams.id.to_host = "<nil>";
   findParams.id.from_user = "<nil>";
   findParams.id.from_host = "<nil>";

   PRIV_OC_EXTRACT_URI(to, findParams.id.to_user, findParams.id.to_host);
   PRIV_OC_EXTRACT_URI(from, findParams.id.from_user, findParams.id.from_host);

   op = priv_oc_find(oc, &findParams);

   return op;
}

ssc_oper_t *ssc_oc_find_uri_str_method(const ssc_t *ssc, const char *to, const char *from, sip_method_t method)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return NULL;
   }

   const ssc_op_container_t *oc = ssc->ssc_oc;
   if (!oc)
   {
      SSCError("%s: ssc context does not have an operation container", __func__);
      return NULL;
   }

   if (method < sip_method_invalid ||
       (unsigned)method >= OC_NUM_METHODS)
   {
      SSCError("%s: op method out of range. val: %d (%d..%u)", __func__,
         method, sip_method_invalid, OC_NUM_METHODS-1);
      return NULL;
   }

   ssc_oper_t *op = NULL;
   ssc_oc_find_param_t findParams;

   findParams.match_method = OC_METHOD_MATCH_ANY;
   if (method >= sip_method_unknown)
   {
      findParams.match_method = OC_METHOD_MATCH_ID;
      findParams.m_index = (unsigned)method;
   }

   findParams.id.to_user = "<nil>";
   findParams.id.to_host = "<nil>";
   findParams.id.from_user = "<nil>";
   findParams.id.from_host = "<nil>";

   sip_to_t *toUri = NULL;
   sip_from_t *fromUri = NULL;

   if (to)
   {
      toUri = sip_to_make(ssc->ssc_home, to);
   }

   if (from)
   {
      fromUri = sip_to_make(ssc->ssc_home, from);
   }

   PRIV_OC_EXTRACT_URI(toUri, findParams.id.to_user, findParams.id.to_host);
   PRIV_OC_EXTRACT_URI(fromUri, findParams.id.from_user, findParams.id.from_host);

   op = priv_oc_find(oc, &findParams);

   if (toUri)
   {
      su_free(ssc->ssc_home, toUri);
   }

   if (fromUri)
   {
      su_free(ssc->ssc_home, fromUri);
   }

   return op;
}

ssc_oper_t *ssc_oc_find_uri_unknown_method(const ssc_t *ssc, const sip_to_t *to, const sip_from_t *from, const char *name)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return NULL;
   }

   const ssc_op_container_t *oc = ssc->ssc_oc;
   if (!oc)
   {
      SSCError("%s: ssc context does not have an operation container", __func__);
      return NULL;
   }

   ssc_oper_t *op = NULL;
   ssc_oc_find_param_t findParams;

   if (name && name[0])
   {
      findParams.match_method = OC_METHOD_MATCH_NAME;
      findParams.m_name = name;
   }
   else
   {
      findParams.match_method = OC_METHOD_MATCH_ID;
      findParams.m_index = 0;
   }

   findParams.id.to_user = "<nil>";
   findParams.id.to_host = "<nil>";
   findParams.id.from_user = "<nil>";
   findParams.id.from_host = "<nil>";

   PRIV_OC_EXTRACT_URI(to, findParams.id.to_user, findParams.id.to_host);
   PRIV_OC_EXTRACT_URI(from, findParams.id.from_user, findParams.id.from_host);

   op = priv_oc_find(oc, &findParams);

   return op;
}

ssc_oper_t *ssc_oc_find_uri_str_unknown_method(const ssc_t *ssc, const char *to, const char *from, const char *name)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return NULL;
   }

   const ssc_op_container_t *oc = ssc->ssc_oc;
   if (!oc)
   {
      SSCError("%s: ssc context does not have an operation container", __func__);
      return NULL;
   }

   ssc_oper_t *op = NULL;
   ssc_oc_find_param_t findParams;

   if (name && name[0])
   {
      findParams.match_method = OC_METHOD_MATCH_NAME;
      findParams.m_name = name;
   }
   else
   {
      findParams.match_method = OC_METHOD_MATCH_ID;
      findParams.m_index = 0;
   }

   findParams.id.to_user = "<nil>";
   findParams.id.to_host = "<nil>";
   findParams.id.from_user = "<nil>";
   findParams.id.from_host = "<nil>";

   sip_to_t *toUri = NULL;
   sip_from_t *fromUri = NULL;

   if (to)
   {
      toUri = sip_to_make(ssc->ssc_home, to);
   }

   if (from)
   {
      fromUri = sip_to_make(ssc->ssc_home, from);
   }

   PRIV_OC_EXTRACT_URI(toUri, findParams.id.to_user, findParams.id.to_host);
   PRIV_OC_EXTRACT_URI(fromUri, findParams.id.from_user, findParams.id.from_host);

   op = priv_oc_find(oc, &findParams);

   if (toUri)
   {
      su_free(ssc->ssc_home, toUri);
   }

   if (fromUri)
   {
      su_free(ssc->ssc_home, fromUri);
   }

   return op;
}


/** internal functions follow **/

unsigned priv_oc_count_elements(const ssc_oc_op_t *oc_op)
{
   unsigned c = 0;

   const ssc_oc_op_t *p = oc_op;
   while (p)
   {
      ++c;
      p = p->next;
   }
   return c;
}

int priv_oc_add(ssc_t *ssc, ssc_oper_t *op, const ssc_oc_id_t *id)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return OC_FAILURE;
   }

   if (!ssc->ssc_home)
   {
      SSCError("%s: NULL ssc home context ptr", __func__);
      return OC_FAILURE;
   }

   ssc_op_container_t *oc = ssc->ssc_oc;

   // create a new collection object in this SSC context if we need one
   if (!oc)
   {
      oc = (ssc_op_container_t *)su_zalloc(ssc->ssc_home, sizeof(ssc_op_container_t));
      if (!oc)
      {
         SSCError("%s: alloc fail - %zu bytes", __func__, sizeof(ssc_op_container_t));
         return OC_FAILURE;
      }

      ssc->ssc_oc = oc;
   }

   // operation method bounds checks..
   if (op->op_method <= sip_method_invalid)
   {
      SSCError("%s: method out of range (val: %d)", __func__, op->op_method);
      return OC_FAILURE;
   }

   unsigned mindex = 0;
   if (op->op_method >= sip_method_unknown)
   {
      mindex = (unsigned)op->op_method;
      if (mindex >= OC_NUM_METHODS)
      {
         SSCError("%s: method out of range (val: %u, max: %u)", __func__, mindex, OC_NUM_METHODS-1);
         return OC_FAILURE;
      }
   }

   // create collectable node for the operation
   ssc_oc_op_t *oc_op = (ssc_oc_op_t *)su_zalloc(ssc->ssc_home, sizeof(ssc_oc_op_t));
   if (!oc_op)
   {
      SSCError("%s: alloc fail - %zu bytes", __func__, sizeof(ssc_oc_op_t));
      return OC_FAILURE;
   }

   op->oc_node = oc_op;

   oc_op->oc = oc;
   oc_op->op = op;
   oc_op->method = op->op_method;
   oc_op->m_index = mindex;

   // hash method str if this is an 'unknown' method
   if (op->op_method == sip_method_unknown)
   {
      if (priv_oc_hash_str_cpy(ssc, op->op_method_name, &oc_op->method_name) == OC_FAILURE)
      {
         priv_oc_op_free(ssc, oc_op);
         return OC_FAILURE;
      }
   }

   // hash URI strings..

   if (priv_oc_hash_str_cpy(ssc, id->to_user, &oc_op->to.user) == OC_FAILURE)
   {
      priv_oc_op_free(ssc, oc_op);
      return OC_FAILURE;
   }

   if (priv_oc_hash_str_cpy(ssc, id->to_host, &oc_op->to.host) == OC_FAILURE)
   {
      priv_oc_op_free(ssc, oc_op);
      return OC_FAILURE;
   }

   if (priv_oc_hash_str_cpy(ssc, id->from_user, &oc_op->from.user) == OC_FAILURE)
   {
      priv_oc_op_free(ssc, oc_op);
      return OC_FAILURE;
   }

   if (priv_oc_hash_str_cpy(ssc, id->from_host, &oc_op->from.host) == OC_FAILURE)
   {
      priv_oc_op_free(ssc, oc_op);
      return OC_FAILURE;
   }

   // insert into table bucket

   unsigned sindex = (oc_op->to.user.hash +
      oc_op->to.host.hash +
      oc_op->from.user.hash +
      oc_op->from.host.hash) % OC_BUCKETS;

   oc_op->s_index = sindex;
   ssc_oc_op_t *n = oc->slot[sindex].method[mindex];
   oc->slot[sindex].method[mindex] = oc_op;
   oc_op->next = n;
   oc_op->prev = NULL;

   if (n)
   {
      n->prev = oc_op;
   }

   ++oc->count;

   return OC_SUCCESS;
}

ssc_oper_t *priv_oc_find(const ssc_op_container_t *oc, const ssc_oc_find_param_t *findParams)
{
   ssc_oper_t *op = NULL;

   ssc_oc_uri_t cmpToUri;
   ssc_oc_uri_t cmpFromUri;
   ssc_oc_str_t cmpMethodName;

   if (priv_oc_hash_str(findParams->id.to_user, &cmpToUri.user) == OC_FAILURE)
   {
      return NULL;
   }

   if (priv_oc_hash_str(findParams->id.to_host, &cmpToUri.host) == OC_FAILURE)
   {
      return NULL;
   }

   if (priv_oc_hash_str(findParams->id.from_user, &cmpFromUri.user) == OC_FAILURE)
   {
      return NULL;
   }

   if (priv_oc_hash_str(findParams->id.from_host, &cmpFromUri.host) == OC_FAILURE)
   {
      return NULL;
   }

   if (findParams->match_method == OC_METHOD_MATCH_NAME)
   {
      if (priv_oc_hash_str(findParams->m_name, &cmpMethodName) == OC_FAILURE)
      {
         return NULL;
      }

      cmpMethodName.str = (char *)findParams->m_name;
   }

   cmpToUri.user.str = (char *)findParams->id.to_user;
   cmpToUri.host.str = (char *)findParams->id.to_host;
   cmpFromUri.user.str = (char *)findParams->id.from_user;
   cmpFromUri.host.str = (char *)findParams->id.from_host;

   unsigned sindex = (cmpToUri.user.hash +
         cmpToUri.host.hash +
         cmpFromUri.user.hash +
         cmpFromUri.host.hash) % OC_BUCKETS;

   if (findParams->match_method == OC_METHOD_MATCH_ANY)
   {
      unsigned m;
      for ( m=0; m < OC_BUCKETS && !op; ++m)
      {
         op = priv_oc_find_any(oc->slot[sindex].method[m], &cmpToUri, &cmpFromUri);
      }
   }
   else if (findParams->match_method == OC_METHOD_MATCH_ID)
   {
      op = priv_oc_find_any(oc->slot[sindex].method[findParams->m_index], &cmpToUri, &cmpFromUri);
   }
   else
   {
      op = priv_oc_find_method_name(oc->slot[sindex].method[0], &cmpToUri, &cmpFromUri, &cmpMethodName);
   }

   return op;
}

ssc_oper_t *priv_oc_find_any(ssc_oc_op_t *opList, const ssc_oc_uri_t *cmpTo, const ssc_oc_uri_t *cmpFrom)
{
   ssc_oper_t *op = NULL;

   for (;opList && !op; opList = opList->next)
   {
      if (opList->to.user.len != cmpTo->user.len) continue;
      if (opList->to.user.hash != cmpTo->user.hash) continue;
      if (opList->to.host.len != cmpTo->host.len) continue;
      if (opList->to.host.hash != cmpTo->host.hash) continue;

      if (strncmp(opList->to.user.str, cmpTo->user.str, cmpTo->user.len)) continue;
      if (strncmp(opList->to.host.str, cmpTo->host.str, cmpTo->host.len)) continue;
      if (strncmp(opList->from.user.str, cmpFrom->user.str, cmpFrom->user.len)) continue;
      if (strncmp(opList->from.host.str, cmpFrom->host.str, cmpFrom->host.len)) continue;

      op = opList->op;
      break;
   }

   return op;
}

ssc_oper_t *priv_oc_find_method_name(ssc_oc_op_t *opList, const ssc_oc_uri_t *cmpTo, const ssc_oc_uri_t *cmpFrom, const ssc_oc_str_t *cmpName)
{
   ssc_oper_t *op = NULL;

   for (;opList && !op; opList = opList->next)
   {
      if (opList->to.user.len != cmpTo->user.len) continue;
      if (opList->to.user.hash != cmpTo->user.hash) continue;
      if (opList->to.host.len != cmpTo->host.len) continue;
      if (opList->to.host.hash != cmpTo->host.hash) continue;

      if (opList->method_name.len != cmpName->len) continue;
      if (opList->method_name.hash != cmpName->hash) continue;

      if (strncmp(opList->to.user.str, cmpTo->user.str, cmpTo->user.len)) continue;
      if (strncmp(opList->to.host.str, cmpTo->host.str, cmpTo->host.len)) continue;
      if (strncmp(opList->from.user.str, cmpFrom->user.str, cmpFrom->user.len)) continue;
      if (strncmp(opList->from.host.str, cmpFrom->host.str, cmpFrom->host.len)) continue;

      if (strncmp(opList->method_name.str, cmpName->str, cmpName->len)) continue;

      op = opList->op;
      break;
   }

   return op;
}

void priv_oc_str_free(const ssc_t *ssc, ssc_oc_str_t *s)
{
   if (!s) { return; }

   if (s->str)
   {
      su_free(ssc->ssc_home, s->str);
   }

}

void priv_oc_op_free(const ssc_t *ssc, ssc_oc_op_t *oc_op)
{
   if (!oc_op) { return; }

   if (oc_op->op) { oc_op->op->oc_node = NULL; }

   priv_oc_str_free(ssc, &oc_op->to.user);
   priv_oc_str_free(ssc, &oc_op->to.host);
   priv_oc_str_free(ssc, &oc_op->from.user);
   priv_oc_str_free(ssc, &oc_op->from.host);

   priv_oc_str_free(ssc, &oc_op->method_name);
}

void priv_oc_free(ssc_t *ssc, int opDestroy)
{
   if (!ssc)
   {
      SSCError("%s: NULL ssc context ptr", __func__);
      return;
   }

   if (!ssc->ssc_home)
   {
      SSCError("%s: NULL ssc home context ptr", __func__);
      return;
   }

   if (!ssc->ssc_oc) { return; }

   ssc_op_container_t *oc = (ssc_op_container_t *)ssc->ssc_oc;
   ssc->ssc_oc = NULL;

   unsigned b,m;
   for(b=0; b<OC_BUCKETS; ++b)
   {
      for (m=0; m<OC_NUM_METHODS; ++m)
      {
         ssc_oc_op_t *oc_op = oc->slot[b].method[m];
         ssc_oper_t *op;
         oc->slot[b].method[m] = NULL;

         ssc_oc_op_t *n = NULL;
         while (oc_op)
         {
            n = oc_op->next;
            op = oc_op->op;

            priv_oc_op_free(ssc, oc_op);

            if (opDestroy == OC_OP_DESTROY)
            {
               ssc_oper_destroy(ssc, op);
            }

            oc_op = n;
         }
      }
   }

   su_free(ssc->ssc_home, oc);
}

int priv_oc_hash_str_cpy(const ssc_t *ssc, const char *str, ssc_oc_str_t *hash_str)
{
   if (priv_oc_hash_str(str, hash_str) == OC_FAILURE)
   {
      return OC_FAILURE;
   }

   if (hash_str->len > 0)
   {
      hash_str->str = (char *)su_alloc(ssc->ssc_home, hash_str->len+1);
      if (!hash_str->str)
      {
         SSCError("%s: alloc fail len - %u bytes", __func__, hash_str->len+1);
         return OC_FAILURE;
      }

      memcpy(hash_str->str, str, hash_str->len);
      hash_str->str[hash_str->len] = 0;
   }

   return OC_SUCCESS;
}


int priv_oc_hash_str(const char *str, ssc_oc_str_t *hash_str)
{
   if (!hash_str)
   {
      SSCError("%s: hash_str NULL ptr", __func__);
      return OC_FAILURE;
   }

   hash_str->len = 0;
   hash_str->hash = 0;
   hash_str->str = NULL;

   const unsigned int p = 31;
   const unsigned int m = 1e9 + 9;
   unsigned pow = 1;

   if (!str) {return OC_SUCCESS;}

   for (; str[hash_str->len]; ++hash_str->len)
   {
      hash_str->hash = (hash_str->hash + str[hash_str->len] * pow) % m;
      pow = (pow * p) % m;
   }

   return OC_SUCCESS;
}

int priv_oc_cmp_str(const ssc_oc_str_t *strA, const ssc_oc_str_t *strB)
{
   if (!strA || !strB) { return OC_FAILURE; }

   if (strA->len != strB->len) { return OC_FAILURE; }

   if (strA->hash != strB->hash) { return OC_FAILURE; }

   if (strA->str && strB->str)
   {
      if (strncmp(strA->str, strB->str, strB->len)) { return OC_FAILURE; }
   }

   return OC_SUCCESS;
}
