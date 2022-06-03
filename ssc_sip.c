/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2005,2006,2007,2009 Nokia Corporation.
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
 *  - see comments marked with 'XXX'
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
#include <stdint.h>

#include <sofia-sip/sip_extra.h>
#include <sofia-sip/soa_tag.h>

#include "ssc_log.h"

#include "ssc_sip.h"
#include "ssc_oper.h"
#include "ssc_oper_container.h"

/* Function prototypes
 * ------------------- */

void ssc_i_fork (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_i_invite (nua_t * nua, ssc_t * ssc, nua_handle_t * nh,
      ssc_oper_t * op, sip_t const *sip, tagi_t tags[]);
void ssc_i_subscribe (nua_t *nua, ssc_t *ssc, nua_handle_t *nh,
      ssc_oper_t *op, const sip_t *sip, tagi_t tags[]);
void ssc_i_register (nua_t *nua, ssc_t *ssc, nua_handle_t *nh,
      ssc_oper_t *op, const sip_t *sip, tagi_t tags[]);
void ssc_i_state (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_i_active (nua_t * nua, ssc_t * ssc, nua_handle_t * nh,
      ssc_oper_t * op, sip_t const *sip, tagi_t tags[]);
void ssc_i_prack (nua_t * nua, ssc_t * ssc, nua_handle_t * nh,
      ssc_oper_t * op, sip_t const *sip, tagi_t tags[]);
void ssc_i_bye (nua_t * nua, ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      sip_t const *sip, tagi_t tags[]);
void ssc_i_cancel (nua_t * nua, ssc_t * ssc, nua_handle_t * nh,
      ssc_oper_t * op, sip_t const *sip, tagi_t tags[]);
void ssc_r_message (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_i_message (nua_t * nua, ssc_t * ssc, nua_handle_t * nh,
      ssc_oper_t * op, sip_t const *sip, tagi_t tags[]);
void ssc_i_info (nua_t * nua, ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      sip_t const *sip, tagi_t tags[]);
void ssc_i_notify (nua_t * nua, ssc_t * ssc, nua_handle_t * nh,
      ssc_oper_t * op, sip_t const *sip, tagi_t tags[]);
void ssc_i_error (nua_t * nua, ssc_t * ssc, nua_handle_t * nh,
      ssc_oper_t * op, int status, char const *phrase,
      tagi_t tags[]);
void ssc_i_options (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);

void ssc_r_info (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_r_bye (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_r_register (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_r_unregister (int status, char const *phrase, nua_t * nua,
      ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      sip_t const *sip, tagi_t tags[]);
void ssc_r_publish (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_r_invite (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_r_media_event (int status, char const *phrase, nua_t * nua,
      ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      sip_t const *sip, tagi_t tags[]);
void ssc_r_shutdown (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_r_get_params (int status, char const *phrase, nua_t * nua,
      ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      sip_t const *sip, tagi_t tags[]);
void ssc_r_subscribe (int status, char const *phrase, nua_t * nua,
      ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      sip_t const *sip, tagi_t tags[]);
void ssc_r_unsubscribe (int status, char const *phrase, nua_t * nua,
      ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      sip_t const *sip, tagi_t tags[]);
void ssc_r_notify (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_r_options (int status, char const *phrase, nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[]);
void ssc_r_media_describe (int status, char const *phrase, nua_t * nua,
      ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      sip_t const *sip, tagi_t tags[]);

// wrapper for operation creation. creates the operation and then adds
// it into a lookup table.
// fixme: add option to get the to-uri & sip-uri to use as the key for
//   operation lookup from the tag list.  for now, the uri is passed
//   through on the fixed arg list.
// note: if the to-uri or from-uri are not specified, then whatever is
//   currently set in the nua handle associated with the operation will
//   be used. these might not be correct if the request as not yet been
//   sent and/or a response received. therefore it is recommended to
//   always specifiy the uris to use for lookup when creating the operation.
static ssc_oper_t *priv_oper_create(
      ssc_t *ssc,
      sip_method_t method, const char *name,
      const char *req_address,
      const char *to_uri, const char *from_uri,
      tag_type_t tag, tag_value_t value, ...);

static ssc_oper_t *priv_oper_create_uri(
      ssc_t *ssc,
      sip_method_t method, const char *name,
      const char *req_address,
      const sip_to_t *to_uri, const sip_from_t *from_uri,
      tag_type_t tag, tag_value_t value, ...)
   __attribute__ ((unused));   // silence gcc warning; can remove this when first use is added

static ssc_oper_t *priv_oper_create_with_handle(
      ssc_t *ssc,
      sip_method_t method, const char *name,
      nua_handle_t *nh,
      const sip_from_t *from,
      const char *to_uri, const char *from_uri)
   __attribute__ ((unused));   // silence gcc warning; can remove this when first use is added

static ssc_oper_t *priv_oper_create_uri_with_handle(
      ssc_t *ssc,
      sip_method_t method, const char *name,
      nua_handle_t *nh,
      const sip_from_t *from,
      const sip_to_t *to_uri, const sip_from_t *from_uri);

static void priv_callback (nua_event_t event, int status, char const *phrase,
      nua_t * nua, ssc_t * ssc, nua_handle_t * nh,
      ssc_oper_t * op, sip_t const *sip, tagi_t tags[]);

static char *priv_parse_domain (su_home_t * home, const char *sip_aor);
static void priv_destroy_oper_with_disconnect (ssc_t * self,
      ssc_oper_t * oper);

static void parseSipContacts (ssc_t *ssc, const sip_t *sip, uint8_t *numContacts, SscSipContact contacts[], uint8_t max);
static void parseSipAccept (ssc_t *ssc, const sip_t *sip, uint8_t *numAccept, SscSipAccept accept[]);

/* Function definitions
 * -------------------- */

int ssc_match_nniType(ssc_t *ssc, uint8_t nniTypeVal)
{
   int rc = -1;
   if (!ssc) return rc;

   uint8_t c = ssc->nniType.count;
   if (c > NNI_TYPE_LIST_MAX) c = NNI_TYPE_LIST_MAX;

   uint8_t i;
   for (i=0; i<c; ++i)
   {
      if (ssc->nniType.type[i] == nniTypeVal)
      {
         rc = 0;
         break;
      }
   }
   return rc;
}

void ssc_clear_nniType (ssc_t *ssc)
{
   if (!ssc) return;
   ssc->nniType.count = 0;
}

void ssc_add_nniType(ssc_t *ssc, uint8_t nniTypeVal)
{
   if (!ssc) return;

   if (ssc->nniType.count >= NNI_TYPE_LIST_MAX)
   {
      SSCError("cannot add nni identifier, nniType list is full!");
      return;
   }

   ssc->nniType.type[ssc->nniType.count++] = nniTypeVal;
}

ssc_oper_t *priv_oper_create(
      ssc_t *ssc,
      sip_method_t method, const char *name,
      const char *req_address,
      const char *to_uri, const char *from_uri,
      tag_type_t tag, tag_value_t value, ...)
{
   ssc_oper_t *op = NULL;

   ssc_oper_create_t o;
   o.use_handle = 0;
   o.use_uri = 0;
   o.nh.create.nua = ssc->ssc_nua;
   o.nh.create.req_addr = req_address;
   o.id.str.to = to_uri;
   o.id.str.from = from_uri;

   ta_list ta;
   ta_start(ta, tag, value);

   op = ssc_oper_create(
         ssc,
         method, name,
         &o,
         ta_tags(ta));

   ta_end(ta);

   return op;
}

ssc_oper_t *priv_oper_create_uri(
      ssc_t *ssc,
      sip_method_t method, const char *name,
      const char *req_address,
      const sip_to_t *to_uri, const sip_from_t *from_uri,
      tag_type_t tag, tag_value_t value, ...)
{
   ssc_oper_t *op = NULL;

   ssc_oper_create_t o;
   o.use_handle = 0;
   o.use_uri = 1;
   o.nh.create.nua = ssc->ssc_nua;
   o.nh.create.req_addr = req_address;
   o.id.uri.to = to_uri;
   o.id.uri.from = from_uri;

   ta_list ta;
   ta_start(ta, tag, value);

   op = ssc_oper_create(
         ssc,
         method, name,
         &o,
         ta_tags(ta));

   ta_end(ta);

   return op;
}

ssc_oper_t *priv_oper_create_with_handle(
      ssc_t *ssc,
      sip_method_t method, const char *name,
      nua_handle_t *nh,
      const sip_from_t *from,
      const char *to_uri, const char *from_uri)
{
   ssc_oper_t *op = NULL;

   ssc_oper_create_t o;
   o.use_handle = 1;
   o.use_uri = 0;
   o.nh.use.handle = nh;
   o.nh.use.from = from;
   o.id.str.to = to_uri;
   o.id.str.from = from_uri;

   op = ssc_oper_create(
         ssc,
         method, name,
         &o,
         TAG_END());

   return op;
}

ssc_oper_t *priv_oper_create_uri_with_handle(
      ssc_t *ssc,
      sip_method_t method, const char *name,
      nua_handle_t *nh,
      const sip_from_t *from,
      const sip_to_t *to_uri, const sip_from_t *from_uri)
{
   ssc_oper_t *op = NULL;

   ssc_oper_create_t o;
   o.use_handle = 1;
   o.use_uri = 1;
   o.nh.use.handle = nh;
   o.nh.use.from = from;
   o.id.uri.to = to_uri;
   o.id.uri.from = from_uri;

   op = ssc_oper_create(
         ssc,
         method, name,
         &o,
         TAG_END());

   return op;
}

ssc_t * ssc_create (su_home_t * home, su_root_t * root, const ssc_conf_t * conf)
{
   ssc_t *ssc;
   char *caps_str = NULL;
   char *userdomain = NULL;

   ssc = su_zalloc (home, sizeof (*ssc));
   if (!ssc)
      return NULL;
   if (!conf)
      return NULL;

   ssc_clear_nniType (ssc);

   ssc->ssc_name = "UA";
   ssc->ssc_home = home;
   ssc->ssc_root = root;
	ssc->ssc_nua_cb = priv_callback;

   ssc->cb_i_register = ssc_i_register;
   ssc->cb_i_invite = ssc_i_invite;
   ssc->cb_i_message = ssc_i_message;
   ssc->cb_i_options = ssc_i_options;
   ssc->cb_i_bye = ssc_i_bye;
   ssc->cb_i_cancel = ssc_i_cancel;

   ssc->cb_r_register = ssc_r_register;
   ssc->cb_r_invite = ssc_r_invite;
   ssc->cb_r_options = ssc_r_options;
   ssc->cb_r_message = ssc_r_message;

   ssc->cb_i_invite_extra = NULL;

   /* step: find out the home domain of the account */
   if (conf->ssc_aor)
      userdomain = priv_parse_domain (home, conf->ssc_aor);

   ssc->ssc_address = su_strdup (home, conf->ssc_aor);

   /* step: update for P-Asserted-identity */
   sip_update_default_mclass(sip_extend_mclass(NULL));

   /* step: launch the SIP stack */

	SSCDebugLow("Proxy: '%s'", conf->ssc_proxy?conf->ssc_proxy:"<nil>");
fprintf(stderr,"Proxy: '%s'\n", conf->ssc_proxy?conf->ssc_proxy:"<nil>");
fprintf(stderr,"CallIF: %s RegIF: %s\n",
   conf->ssc_call_bind_addr?conf->ssc_call_bind_addr:"<nil>",
   conf->ssc_reg_bind_addr?conf->ssc_reg_bind_addr:"<nil>");
fflush(stderr);

	/* create Call/Base NUA context */
   ssc->ssc_nua = nua_create (root,
         priv_callback, ssc,
			NUTAG_URL(conf->ssc_call_bind_addr),
	      //NUTAG_URL("sip:10.9.4.7:5060;maddr=192.168.122.4"),
			//NUTAG_URL("sip:10.9.4.7:5060"),
			//SIPTAG_ROUTE_STR("<sip:192.168.122.4:5060>"),
	      //NUTAG_URL("sip:192.168.122.4:5060"),

         //NTATAG_STATELESS(1),
         SIPTAG_USER_AGENT_STR("eschat"),
         TAG_IF (conf->ssc_aor,
            SIPTAG_FROM_STR (conf->ssc_aor)),
         TAG_IF (conf->ssc_proxy,
            NUTAG_PROXY (/*"sip:10.9.4.7:5060"*/conf->ssc_proxy)),
         TAG_IF (conf->ssc_registrar,
            NUTAG_REGISTRAR (conf->ssc_registrar)),
         TAG_IF (conf->ssc_contact,
            NUTAG_URL (conf->ssc_contact)),
         TAG_IF (conf->ssc_media_addr,
            NUTAG_MEDIA_ADDRESS
            (conf->ssc_media_addr)),
         /* note: use of STUN for signaling disabled */
         /* TAG_IF(conf->ssc_stun_server, STUNTAG_SERVER(conf->ssc_stun_server)), */
         /* TAG_IF(userdomain, STUNTAG_DOMAIN(userdomain)), */
         /* Used in OPTIONS */
         TAG_IF (caps_str,
            SOATAG_USER_SDP_STR (caps_str)),
         SOATAG_AF (SOA_AF_IP4_IP6),
	      NUTAG_MEDIA_ENABLE(0),
		             NUTAG_OUTBOUND("no-validate no-options-keepalive"),
		             //NUTAG_KEEPALIVE(0),
		             //NUTAG_KEEPALIVE_STREAM(0),
		             //NUTAG_CALLEE_CAPS(0),
		             //NUTAG_M_FEATURES("expires=0"),
	      NTATAG_SIPFLAGS(conf->ssc_flags),
	      TAG_NULL ());

   /* step: free the static caps */
   free (caps_str);

   if (ssc->ssc_nua)
   {
      nua_set_params (ssc->ssc_nua,
#ifdef SIPREC_SERVER
            NUTAG_SUPPORTED("siprec"),
#endif
            NUTAG_AUTOACK(0),  // tell sofia not to send ACKs, we need to send them
                               // in order to control some of the headers used.
            NUTAG_ALLOW("REGISTER"),
            NUTAG_MEDIA_ENABLE(0),
            NUTAG_ENABLEMESSAGE (1),
            NUTAG_ENABLEINVITE (1),
            NUTAG_SESSION_TIMER (0),
            NUTAG_AUTOANSWER (0),
            TAG_IF (conf->ssc_certdir,
               NUTAG_CERTIFICATE_DIR (conf->ssc_certdir)),
            TAG_NULL ());
      nua_get_params (ssc->ssc_nua, TAG_ANY (), TAG_NULL ());
   }
   else
   {
      ssc_destroy (ssc);
      ssc = NULL;
   }

   su_free (home, userdomain);

   return ssc;
}

/** 
 * Disconnects GObject signal 'state-changed' and destroys
 * operator handle.
 */
static void priv_destroy_oper_with_disconnect (ssc_t * self, ssc_oper_t * op)
{
   ssc_oper_destroy (self, op);
}

void ssc_destroy (ssc_t * self)
{
   su_home_t *home = NULL;

   if (!self)
      return;

   home = self->ssc_home;

   if (self->ssc_address)
      su_free (home, self->ssc_address);

   su_free (home, self);
}


void parseSipContacts (ssc_t *ssc, const sip_t *sip, uint8_t *numContacts, SscSipContact contacts[], uint8_t max)
{
	int contactCount = 0;
	int ival;
	sip_contact_t *currentContact = NULL;

   if (!ssc || !sip) return;

   currentContact = sip->sip_contact;

	while (currentContact && contactCount < max)
	{
		contacts[contactCount].contactDisplay = currentContact->m_display;
		contacts[contactCount].contactUri = url_as_string(ssc->ssc_home, currentContact->m_url);
		contacts[contactCount].contactUser = currentContact->m_url->url_user;
		contacts[contactCount].contactHost = currentContact->m_url->url_host;
		contacts[contactCount].expiresPresent = 0;

		if (currentContact->m_expires)
		{
			ival = atoi(currentContact->m_expires);
			if (ival >= 0)
			{
				contacts[contactCount].expiresPresent = 1;
				contacts[contactCount].expiresValue = (uint32_t)ival;
			}
		}
		++contactCount;
		currentContact = currentContact->m_next;
	}
	
	if (numContacts)
	{
		*numContacts = contactCount;
	}
}

void parseSipAccept (ssc_t *ssc, const sip_t *sip, uint8_t *numAccept, SscSipAccept accept[])
{
	int acceptCount = 0;
	sip_accept_t *currentAccept = NULL;

   if (!ssc || !sip) return;

   currentAccept = sip->sip_accept;

	while (currentAccept)
	{
		accept[acceptCount].type = currentAccept->ac_type;
		accept[acceptCount].subtype = currentAccept->ac_subtype;
		++acceptCount;
		currentAccept = currentAccept->ac_next;
	}

   if (numAccept)
   {
      *numAccept = acceptCount;
   }
}

/**
 * Parses domain part of SIP address given in 'sip_aor'.
 * The return substring is duplicated using 'home' and
 * the ownership is transfered to the caller.
 */
static char * priv_parse_domain (su_home_t * home, const char *sip_aor)
{
   char *result = NULL, *i;

   if (!home || !sip_aor) return NULL;

   /* remove sip prefix */
   if (strncmp ("sip:", sip_aor, 4) == 0)
   {
      sip_aor += 4;
   }

   /* skip userinfo */
   if (strstr (sip_aor, "@"))
   {
      while (*sip_aor && *sip_aor++ != '@');
   }

   /* copy rest of the string */
   result = su_strdup (home, sip_aor);

   /* mark end (at port or uri-parameters defs) */
   for (i = result; *i; i++)
   {
      if (*i == ';' || *i == ':')
         *i = 0;
   }

   return result;
}

/**
 * Callback for events delivered by the SIP stack.
 *
 * See libsofia-sip-ua/nua/nua.h documentation.
 */
static void priv_callback (nua_event_t event,
      int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   SSCDebugHigh("SSC priv_callback: EVENT: %s [%d]", nua_event_name(event), event);
	SSCDebugHigh("op <%p> -> userData <%p>", op, (op!=NULL?op->userData:NULL));

   if (!ssc)
   {
      SSCError("NULL SSC context ptr!");
      return;
   }

   switch (event)
   {
      case nua_r_shutdown:
         ssc_r_shutdown (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_get_params:
         ssc_r_get_params (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_register:
			ssc->cb_r_register (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_unregister:
			ssc_r_unregister (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_options:
         ssc->cb_i_options (nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_options:
         ssc->cb_r_options (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_invite:
         ssc->cb_r_invite (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_fork:
         ssc_i_fork (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_invite:
         ssc->cb_i_invite (nua, ssc, nh, op, sip, tags);
         break;

		case nua_i_subscribe:
			ssc_i_subscribe (nua, ssc, nh, op, sip, tags);
			break;

		case nua_i_register:
			ssc->cb_i_register (nua, ssc, nh, op, sip, tags);
			break;

      case nua_i_prack:
         ssc_i_prack (nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_state:
         ssc_i_state (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_bye:
         ssc_r_bye (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_bye:
         ssc->cb_i_bye (nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_message:
         ssc->cb_r_message (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_message:
         ssc->cb_i_message (nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_info:
         // FIXME: error! 
         break;

      case nua_i_info:
         // FIXME: error! 
         break;

      case nua_r_refer:
         // FIXME: error! 
         break;

      case nua_i_refer:
         // FIXME: error! 
         break;

      case nua_r_subscribe:
         ssc_r_subscribe (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_unsubscribe:
         ssc_r_unsubscribe (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_publish:
         ssc_r_publish (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_r_notify:
         ssc_r_notify (status, phrase, nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_notify:
         ssc_i_notify (nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_cancel:
         ssc->cb_i_cancel (nua, ssc, nh, op, sip, tags);
         break;

      case nua_i_error:
         ssc_i_error (nua, ssc, nh, op, status, phrase, tags);
         break;

      case nua_i_active:
			break;

      case nua_i_ack:
			break;

      case nua_i_terminated:
         break;

      default:
			SSCDebugHigh ("%s: unhandled event: %03d %s",
					ssc->ssc_name, status, phrase);

         if (ssc_oper_find_by_handle (ssc, nh) == NULL)
         {
            /* note: unknown handle, not associated to any existing 
             *       call, message, registration, etc, so it can
             *       be safely destroyed */
            SU_DEBUG_1 (("NOTE: destroying handle %p.", nh));
            nua_handle_destroy (nh);
         }

         break;

   }

   if (ssc->ssc_event_cb)
      ssc->ssc_event_cb (ssc, (int) event, ssc->userData);
}

/* ====================================================================== */

int priv_str_chr_count (const char *data, int chr)
{
   int count = 0;
   for (; *data; data++)
   {
      if (*data == chr)
         ++count;
   }

   return count;
}


/**
 * Prints verbose error information to stdout.
 */
void ssc_i_error (nua_t * nua, ssc_t * ssc, nua_handle_t * nh, ssc_oper_t * op,
      int status, char const *phrase, tagi_t tags[])
{
   SSCDebugHigh ("%s: error %03d %s", ssc?ssc->ssc_name:"", status, phrase);
	if (ssc && ssc->ssc_error_cb)
	{
		ssc->ssc_error_cb(ssc, status, phrase, op);
	}
}

/**
 * Lists all active operations to stdout.
 */
void ssc_list (ssc_t * ssc)
{
   ssc_oper_t *op;

   if (!ssc) return;

   SSCDebugHigh ("%s: listing active handles", ssc?ssc->ssc_name:"");
   for (op = ssc->ssc_operations; op; op = op->op_next)
   {
      if (op->op_ident)
      {
         SSCDebugHigh ("\t%s to %s",
               sip_method_name (op->op_method, op->op_method_name),
               op->op_ident);
      }
   }
}

/*
 * creates MIME payload with SDP and metadata, generating the payload structure and placing the pointer into payloadOut
 *
 * returns 0 on success, -1 on failure
 *
 */
int ssc_create_siprec_mime(
      const char *sdp,
      const char *metadata,
      msg_t **mimeMsgOut,
      sip_payload_t **payloadOut,
      sip_content_type_t **contentTypeOut)
{
   msg_header_t *header = NULL;
   char *ptr;
   size_t len, offset;

   // Create the msg.
   msg_t *msg = msg_create(sip_default_mclass(), 0);
   if (!msg)
   {
      return -1;
   }

   // Use the msg home.
   su_home_t *home = msg_home(msg);


   // The contentType header.
   sip_content_type_t *contentType = NULL;
   contentType = sip_content_type_make(home, "multipart/mixed");

   // The sub parts.
   msg_multipart_t *multipart = NULL;
   msg_multipart_t *multipart2 = NULL;
   multipart = msg_multipart_create(
         home, "application/sdp", sdp, strlen(sdp));
   multipart2 = msg_multipart_create(
         home, 
         "application/rs-metadata+xml", 
         metadata, strlen(metadata));

   // check memory allocs
   if (!contentType || !multipart || !multipart2)
   {
      msg_destroy(msg);
      return -2;
   }

   multipart->mp_next = multipart2;

   // Add delimiters to multipart, and boundary parameter to content-type
   if (msg_multipart_complete(home, contentType, multipart) < 0)
   {
      msg_destroy(msg);
      return -3;         // Error
   }

   // Combine multipart components into the chain
   header = NULL;
   if (msg_multipart_serialize(&header, multipart) < 0)
   {
      msg_destroy(msg);
      return -4;         // Error
   }

   // Encode all multipart components
   len = msg_multipart_prepare(msg, multipart, 0);
   if (len < 0)
   {
      msg_destroy(msg);
      return -5;         // Error
   }

   sip_payload_t *pl = sip_payload_create(home, NULL, len);

   // Copy each element from multipart to pl_data
   if (pl && pl->pl_data)
   {
      ptr = pl->pl_data;
      for (offset = 0, header = (msg_header_t *)multipart; offset < len; header = header->sh_succ) 
      {
         memcpy(ptr + offset, header->sh_data, header->sh_len);
         offset += header->sh_len;
      }
   }

   // The msg is the memory home...  msg_destroy(msg) to free it all.
   *mimeMsgOut = msg;
   *payloadOut = pl;
   *contentTypeOut = contentType;

   return 0;
}

/**
 * Sends an outgoing OPTIONS request.
 *
 * @param ssc context pointer
 * @param destination SIP URI
 */
ssc_oper_t *ssc_options (ssc_t * ssc, ssc_config_t *config)
{
   ssc_oper_t *op;

   SSCDebugLow("SSC Options");

   if (!ssc)
   {
      SSCError("null SSC context ptr!");
      return NULL;
   }

   if (!config)
   {
      SSCError("null SSC config ptr");
      return NULL;
   }

   SSCDebugLow("op create");
   const char *reqAddr = config->requestUri;
   if (!reqAddr)
   {
      reqAddr = config->toUri;
   }

   op = priv_oper_create(
         ssc,
         SIP_METHOD_OPTIONS,
         reqAddr,
         config->toUri, config->fromUri,
         TAG_END());

   if (op)
   {
      //op->op_callstate &= !opc_pending;

SSCDebugLow("nua_options tags:");
SSCDebugLow("requestUri - '%s'", config->requestUri);
SSCDebugLow("targetAddress - '%s'", config->targetAddress);
SSCDebugLow("via - '%s'", config->via);
SSCDebugLow("to - '%s'", config->toUri);
SSCDebugLow("from - '%s'", config->fromUri);
SSCDebugLow("callId - '%s'", config->callId);
SSCDebugLow("accept - '%s'", config->accept);
SSCDebugLow("allow - '%s'", config->allow);

      nua_options (op->op_handle,
                   TAG_IF((config->targetAddress[0] != '\0'), NUTAG_PROXY(config->targetAddress)),
                   TAG_IF((config->via[0] != '\0'), SIPTAG_VIA_STR(config->via)),
                   TAG_IF((config->toUri[0] != '\0'), SIPTAG_TO_STR(config->toUri)),
                   TAG_IF((config->fromUri[0] != '\0'), SIPTAG_FROM_STR(config->fromUri)),
                   TAG_IF((config->accept[0] != '\0'), SIPTAG_ACCEPT_STR(config->accept)),
                   TAG_IF((config->allow[0] != '\0'), SIPTAG_ALLOW_STR(config->allow)),
                   TAG_IF((config->callId[0] != '\0'), SIPTAG_CALL_ID_STR(config->callId)),
                   TAG_END());

      //op->op_callstate |= opc_sent;
      SSCDebugHigh ("%s: OPTIONS to %s", ssc->ssc_name, op->op_ident);
   }
   return op;
}

/**
 * Sends an outgoing INVITE request.
 *
 * @param ssc context pointer
 * @param destination SIP URI
 */
ssc_oper_t *ssc_invite (ssc_t * ssc, ssc_config_t *config)
{
   ssc_oper_t *op;

char *branchStr, branchBuf[1024];

   int paidValid = 0;
   sip_p_asserted_identity_t paid = SIP_P_ASSERTED_IDENTITY_INIT();

SSCDebugLow("SSC Invite");

   if (!ssc)
   {
      SSCError("null SSC context ptr!");
      return NULL;
   }

   if (!config)
   {
      SSCError("null SSC config ptr");
      return NULL;
   }

   if (config->paidDisplay && config->paidSipUri)
   {

      paid.paid_next = NULL;
      paid.paid_display = config->paidDisplay;
      url_d(paid.paid_url, config->paidSipUri);

      paidValid = 1;
   }

   SSCDebugLow("op create");
   const char *reqAddr = config->requestUri;
   if (!reqAddr)
   {
      reqAddr = config->toUri;
   }

   op = priv_oper_create(
         ssc,
         SIP_METHOD_INVITE,
         reqAddr,
         config->toUri, config->fromUri,
         TAG_IF(paidValid, SIPTAG_P_ASSERTED_IDENTITY(&paid)),
         TAG_IF(paidValid, SIPTAG_PRIVACY_STR("id")),
         TAG_END());

   if (op)
   {
		sip_request_t *sipreq = NULL;

      op->op_callstate &= !opc_pending;

      if (config->sdp || config->payload)
      {
         char contactBuffer[1024];
         char eschatSipHeader[1024];
         char eschatTalkerLocationHeader[1024];
         char eschatOrganizationHeader[1024];

         contactBuffer[0] = '\0';
         eschatSipHeader[0] = '\0';
         eschatTalkerLocationHeader[0] = '\0';
         eschatOrganizationHeader[0] = '\0';

         if (config->postProcessContactUri == 0)
         {
            strcat(contactBuffer, config->contactUri);
			}
			else
			{
				if (config->contactUri[0] != '\0')
				{
					snprintf(contactBuffer, 1024, "<%s%s>%s", config->contactUri, config->contactUriFeatures, config->contactFeatures);
				}
			}

         const char *eschatCallHeaderName = NULL;

         switch (config->headerFormat) {
         case SIP_HEADER_FORMAT_NONE:
            // No header
            break;
         case SIP_HEADER_FORMAT_ESCHAT_CALL:
            eschatCallHeaderName = "ESChat-Call";
            break;
         case SIP_HEADER_FORMAT_X_ESCHAT_CALL:
            eschatCallHeaderName = "X-ESChat-Call";
            break;
         }

         if (eschatCallHeaderName != NULL)
         {
            if (config->callType == SIP_CALL_TYPE_ADHOC)
            {
               snprintf(
                     eschatSipHeader,
                     sizeof(eschatSipHeader),
                     "%s: Adhoc;;%s",
                     eschatCallHeaderName,
                     config->callId);
            }
            else if (config->callType == SIP_CALL_TYPE_GROUP)
            {
               snprintf(
                     eschatSipHeader,
                     sizeof(eschatSipHeader),
                     "%s: Group;%s;%s",
                     eschatCallHeaderName,
                     config->groupName,
                     config->callId);
            }
         }

         if (config->talkerLocation[0] != '\0' &&
             config->headerFormat != SIP_HEADER_FORMAT_NONE)
         {
            snprintf(eschatTalkerLocationHeader,
                     sizeof(eschatTalkerLocationHeader),
                     "X-ESChat-Talker-Location: %s",
                     config->talkerLocation);
         }

         if (config->customerName[0] != '\0' &&
             config->headerFormat != SIP_HEADER_FORMAT_NONE)
         {
            snprintf(eschatOrganizationHeader,
                     sizeof(eschatOrganizationHeader),
                     "X-ESChat-Organization: %s;%s;%s",
                     config->customerName,
                     config->customerId,
                     config->departmentId);
         }

         if (config->payload)
			{
            SSCDebugHigh ("%s: about to make a call with local payload:\n%s",
                  ssc->ssc_name, config->payload->pl_data);
				// guard: make sure SDP is not also included
				config->sdp[0] = '\0';
			}
         else if (config->sdp[0] != '\0')
         {
            SSCDebugHigh ("%s: about to make a call with local SDP:\n%s",
                  ssc->ssc_name, config->sdp);
         }

			if (strlen(config->requestUri) > 0)
			{
				sipreq = sip_request_create(ssc->ssc_home, SIP_METHOD_INVITE, (const url_string_t *)config->requestUri, NULL);
			}

			if (config->contentType)
			{
				config->contentTypeStr[0] = '\0';
			}

branchBuf[0] = '\0';
branchStr = NULL;
if (config->via)
{
	strcpy(branchBuf, config->via);
	branchStr = strstr(branchBuf, "branch=");
	if (branchStr)
	{
		branchStr += 7;
	}
}

SSCDebugLow("nua_invite tags:");
SSCDebugLow("requestUri - '%s'", config->requestUri);
SSCDebugLow("targetAddress - '%s'", config->targetAddress);
SSCDebugLow("via - '%s'", config->via);
SSCDebugLow("to - '%s'", config->toUri);
SSCDebugLow("from - '%s'", config->fromUri);
SSCDebugLow("callId - '%s'", config->callId);
SSCDebugLow("accept - '%s'", config->accept);
SSCDebugLow("allow - '%s'", config->allow);
SSCDebugLow("eschat - '%s'", eschatSipHeader);
SSCDebugLow("contact - '%s'", contactBuffer);
SSCDebugLow("priority - '%s'", config->priority);
SSCDebugLow("talkerLocation - '%s'", config->talkerLocation);
SSCDebugLow("customerName - '%s'", config->customerName);
SSCDebugLow("customerId - '%s'", config->customerId);
SSCDebugLow("departmentId - '%s'", config->departmentId);
SSCDebugLow("acceptContact - '%s'", config->acceptContact);
SSCDebugLow("require - '%s'", config->require);
SSCDebugLow("content type - <%p>", config->contentType);
SSCDebugLow("content type_s - '%s'", config->contentTypeStr);
SSCDebugLow("route - '%s'", config->route);
SSCDebugLow("expires - <%p> '%s'", config->expires, config->expires);
SSCDebugLow("payload - <%p>", config->payload);
SSCDebugLow("payloadStr - '%s'", config->payloadStr);
SSCDebugLow("sdp - '%s'", config->sdp);
SSCDebugLow("contentLen - '%s'", config->contentLength);
SSCDebugLow("def_branch: '%s'", branchStr?branchStr:"<nil>");

         nua_invite (op->op_handle,
			            NUTAG_AUTOANSWER(0),
			            NUTAG_INVITE_TIMER(10),
                     TAG_IF((config->route[0] != '\0'), SIPTAG_ROUTE_STR(config->route)),
			            TAG_IF((config->targetAddress[0] != '\0'), NUTAG_PROXY(config->targetAddress)),
			            TAG_IF(sipreq, SIPTAG_REQUEST(sipreq)),
			            TAG_IF((config->via[0] != '\0'), SIPTAG_VIA_STR(config->via)),
			            TAG_IF((config->toUri[0] != '\0'), SIPTAG_TO_STR(config->toUri)),
			            TAG_IF((config->fromUri[0] != '\0'), SIPTAG_FROM_STR(config->fromUri)),
			            TAG_IF((config->accept[0] != '\0'), SIPTAG_ACCEPT_STR(config->accept)),
			            TAG_IF((config->allow[0] != '\0'), SIPTAG_ALLOW_STR(config->allow)),
			            TAG_IF((eschatSipHeader[0] != '\0'), SIPTAG_HEADER_STR(eschatSipHeader)),
			            TAG_IF((eschatTalkerLocationHeader[0] != '\0'), SIPTAG_HEADER_STR(eschatTalkerLocationHeader)),
			            TAG_IF((eschatOrganizationHeader[0] != '\0'), SIPTAG_HEADER_STR(eschatOrganizationHeader)),
			            TAG_IF((contactBuffer[0] != '\0'), SIPTAG_CONTACT_STR(contactBuffer)),
                     TAG_IF((config->priority[0] != '\0'), SIPTAG_PRIORITY_STR(config->priority)),
			            TAG_IF((config->acceptContact[0] != '\0'), SIPTAG_ACCEPT_CONTACT_STR(config->acceptContact)),
			            TAG_IF((config->require[0] != '\0'), SIPTAG_REQUIRE_STR(config->require)),
			            TAG_IF(config->contentType, SIPTAG_CONTENT_TYPE(config->contentType)),
			            TAG_IF((config->contentTypeStr[0] != '\0'), SIPTAG_CONTENT_TYPE_STR(config->contentTypeStr)),
			            TAG_IF((config->expires && config->expires[0] != '\0'), SIPTAG_EXPIRES_STR(config->expires)),
                     TAG_IF((config->contentLength[0] != '\0'), SIPTAG_CONTENT_LENGTH_STR(config->contentLength)),
                     TAG_IF(config->payload, SIPTAG_PAYLOAD(config->payload)),
                     TAG_IF((config->payloadStr[0] != '\0'), SIPTAG_PAYLOAD_STR(config->payloadStr)),
                     TAG_IF((config->payloadStr[0] == '\0' && config->sdp[0] != '\0'), SIPTAG_PAYLOAD_STR(config->sdp)),
			            TAG_END());

         op->op_callstate |= opc_sent;
         SSCDebugHigh ("%s: INVITE to %s", ssc->ssc_name, op->op_ident);
      }
      else
      {
         op->op_callstate |= opc_none;
         SSCDebugHigh
            ("ERROR: no SDP provided by media subsystem, aborting call.");
         priv_destroy_oper_with_disconnect (ssc, op);
         op = NULL;
      }
   }
   return op;
}

/*
 * send outgoing REGISTER request
 */
ssc_oper_t *ssc_register(ssc_t *ssc, ssc_config_t *config)
{
   return ssc_register_op(ssc, config, NULL);
}

ssc_oper_t *ssc_register_op(ssc_t *ssc, ssc_config_t *config, ssc_oper_t *op)
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return NULL;
   }

   if (!config)
   {
      SSCError("%s: NULL SSC config ptr!", __func__);
      return NULL;
   }

   if (!op)
   {
      op = priv_oper_create(
	                     ssc,
	                     SIP_METHOD_REGISTER,
	                     config->toUri,
                        config->toUri, config->fromUri,
	                     TAG_END());
   }

	if (op)
	{
		uint16_t trgtAddrLen = 0;
		char contactBuffer[1024];
		sip_request_t *sip_req = NULL;

      // we are sending a new request on this operation so make sure that
      // the sip msg description ptr is cleared since we do not have a 
      // response msg to referr to yet.
      // we only need to worry about this here because register operations
      // are maintained as long as the registration exists and can be
      // re-used when epttd requests to send a REGISTER request for an
      // existing registration.
      op->sip = NULL;

		contactBuffer[0] = '\0';
		if (config->postProcessContactUri == 0)
		{
			strcat(contactBuffer, config->contactUri);
		}
		else
		{
			if (config->contactUri[0] != '\0')
			{
				snprintf(contactBuffer, 1024, "<%s%s>%s",
							config->contactUri,
							config->contactUriFeatures,
							config->contactFeatures);
			}
		}

		if (strlen(config->requestUri) > 0)
		{
			sip_req = sip_request_create(ssc->ssc_home, SIP_METHOD_REGISTER, (const url_string_t *)config->requestUri, NULL);
		}

SSCDebugLow("nua_register tags:");
SSCDebugLow("targetAddress - '%s'", config->targetAddress);
SSCDebugLow("contactBuffer - '%s'", contactBuffer);
SSCDebugLow("req - <%p> '%s'", sip_req, config->requestUri);
SSCDebugLow("via - '%s'", config->via);
SSCDebugLow("toUri - '%s'", config->toUri);
SSCDebugLow("fromUri - '%s'", config->fromUri);
SSCDebugLow("callId - '%s'", config->callId);
SSCDebugLow("accept - '%s'", config->accept);
SSCDebugLow("allow - '%s'", config->allow);
SSCDebugLow("expires - '%s'", config->expires);
SSCDebugLow("content-type - '%s'", config->contentTypeStr);
SSCDebugLow("content-len - '%s'", config->contentLength);

SSCDebugLow("payload: ");
SSCDebugLow("%s", config->payloadStr);

		trgtAddrLen = strlen(config->targetAddress);
		if (trgtAddrLen > 64)
		{
			SSCWarning("Target address too long! %u [>%u]", trgtAddrLen, 64);
		}

		nua_register(op->op_handle,
		             NUTAG_OUTBOUND("no-validate no-options-keepalive"),
		             NUTAG_DIALOG(0),
		             TAG_IF(sip_req, SIPTAG_REQUEST(sip_req)),
                   TAG_IF((contactBuffer[0] != '\0'), SIPTAG_CONTACT_STR(contactBuffer)),
                   TAG_IF((config->targetAddress[0] != '\0'), NUTAG_PROXY(config->targetAddress)),
                   TAG_IF((config->via[0] != '\0'), SIPTAG_VIA_STR(config->via)),
                   TAG_IF((config->toUri[0] != '\0'), SIPTAG_TO_STR(config->toUri)),
                   TAG_IF((config->fromUri[0] != '\0'), SIPTAG_FROM_STR(config->fromUri)),
                   TAG_IF((config->callId[0] != '\0'), SIPTAG_CALL_ID_STR(config->callId)),
                   TAG_IF((config->accept[0] != '\0'), SIPTAG_ACCEPT_STR(config->accept)),
                   TAG_IF((config->allow[0] != '\0'), SIPTAG_ALLOW_STR(config->allow)),
                   TAG_IF((config->expires[0] != '\0'), SIPTAG_EXPIRES_STR(config->expires)),
                   TAG_IF((config->contentTypeStr[0] != '\0'), SIPTAG_CONTENT_TYPE_STR(config->contentTypeStr)),
                   TAG_IF((config->contentLength[0] != '\0'), SIPTAG_CONTENT_LENGTH_STR(config->contentLength)),
                   TAG_IF((config->payloadStr[0] != '\0'), SIPTAG_PAYLOAD_STR(config->payloadStr)),
                   TAG_END());

		if (sip_req)
		{
			su_free(ssc->ssc_home, sip_req);
		}
	}
	else
	{
		SSCError("%s: failed to create ssc operation", __func__);
	}
	
	return op;
}

/**
 * Callback for response to outgoing REGISTER
 */
void ssc_r_register (int status, const char *phrase,
        nua_t *nua, ssc_t *ssc,
        nua_handle_t *nh, ssc_oper_t *op, const sip_t *sip,
        tagi_t tags[])
{
	SSCDebugHigh("%s: REGISTER resp: %03d(%d) %s",
	   ssc?ssc->ssc_name:"<nil>", 
      status,
	   (sip && sip->sip_status?sip->sip_status->st_status:0), 
      phrase);

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (!op)
   {
      SSCError("%s: NULL SSC operation ptr!", __func__);
      return;
   }

   // this is a warning because if the sip ptr was set to something
   // different then this indicates that we may need to review the
   // situation in case it is due to an error that occurred earlier.
   // though we will correct the value of op-sip below, there may
   // be other integrity issues caused by the error.
   if (op->sip != sip)
   {
      SSCWarning(":%s: WARN: op<%p>->sip<%p> != sip<%p>", __func__,
            op, op->sip, sip);
   }

   // update the operation's response message descriptor to point to the current
   // response message.
   op->sip = sip;

	if (status >= 200 && status <300)
	{
      SSCDebugHigh("RX success reg resp..");
		if (op->userData)
		{
			if (ssc->ssc_msg_ok_cb)
			{
            SSCDebugHigh("exp=%u con=%u", 
                  sip->sip_expires != NULL,
                  sip->sip_contact != NULL);

            // make sure content is null terminated
            if (sip->sip_payload && sip->sip_payload->pl_data)
            {
               sip->sip_payload->pl_data[sip->sip_payload->pl_len] = '\0';
            }

				ssc->ssc_msg_ok_cb(
                  op->userData,
                  status,
                  sip->sip_expires,
                  sip->sip_contact,
                  sip->sip_content_type,
                  sip->sip_payload);
			}
	
			// registration exchange (dialog) is terminated

         // FIXME: this callback should be renamed.. 
         //  we are not actually deleting the operation yet
         //  (because sofia maintains internal state for the
         //  lifetime of a registration so that it can drive
         //  registration refresh). we are just indicating
         //  that the sip dialog has completed so the
         //  resource for the transaction can be released.
			if (ssc->ssc_oper_destroyed_cb) 
            ssc->ssc_oper_destroyed_cb(op->userData);

			op->userData = NULL;
		}
	}
	else if (status >= 300)
	{
      // note: the destroy function calls ssc_oper_destroyed_cb for us
      //   if one is defined. so we don't need to call it explicitly
      //   here.
		priv_destroy_oper_with_disconnect(ssc, op);
                op = NULL;
        }
}

/**
 * callback for response to outgoing un-REGISTER
 */
void ssc_r_unregister (int status, const char *phrase,
        nua_t *nua, ssc_t *ssc,
        nua_handle_t *nh, ssc_oper_t *op, const sip_t *sip,
        tagi_t tags[])
{
	SSCDebugHigh("%s: un-REGISTER resp: %03d(%d) %s",
	   ssc?ssc->ssc_name:"<nil>", status,
	   (sip && sip->sip_status?sip->sip_status->st_status:0), phrase);

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (!op)
   {
      SSCError("%s: NULL SSC operation ptr!", __func__);
      return;
   }

   // update the operation's response message descriptor to point to the current
   // response message.
	op->sip = sip;

	if (status >= 200 && status < 300)
	{
		if (ssc->ssc_msg_ok_cb)
		{
			ssc->ssc_msg_ok_cb(op->userData, status, NULL, NULL, NULL, NULL);
		}

		priv_destroy_oper_with_disconnect(ssc, op);
                op = NULL;
        }
	else if (status >= 300)
	{
		priv_destroy_oper_with_disconnect(ssc, op);
                op = NULL;
        }
}

/**
 * Callback for response to an outgoing OPTIONS request.
 */
void ssc_r_options (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
	SSCDebugHigh("%s: OPTIONS resp: %03d(%d) %s",
	   ssc?ssc->ssc_name:"<nil>", 
      status,
	   (sip && sip->sip_status?sip->sip_status->st_status:0), 
      phrase);

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (!op)
   {
      SSCError("%s: NULL SSC operation ptr!", __func__);
      return;
   }

   // this is a warning because if the sip ptr was set to something
   // different then this indicates that we may need to review the
   // situation in case it is due to an error that occurred earlier.
   // though we will correct the value of op-sip below, there may
   // be other integrity issues caused by the error.
   if (op->sip != sip)
   {
      SSCWarning(":%s: WARN: op<%p>->sip<%p> != sip<%p>", __func__,
            op, op->sip, sip);
   }

   // update the operation's response message descriptor to point to the current
   // response message.
   op->sip = sip;

	if (status >= 200 && status <300)
	{
		if (op->userData)
		{
			if (ssc->ssc_opt_ok_cb)
			{
            SSCDebugHigh("exp=%u con=%u", 
                  sip->sip_expires != NULL,
                  sip->sip_contact != NULL);

            // make sure content is null terminated
            if (sip->sip_payload && sip->sip_payload->pl_data)
            {
               sip->sip_payload->pl_data[sip->sip_payload->pl_len] = '\0';
            }

				ssc->ssc_opt_ok_cb(
                  op->userData,
                  status,
                  sip->sip_expires,
                  sip->sip_contact,
                  sip->sip_content_type,
                  sip->sip_payload);
			}
	
			// registration exchange (dialog) is terminated
			//priv_destroy_oper_with_disconnect(ssc, op);
			if (ssc->ssc_oper_destroyed_cb) 
            ssc->ssc_oper_destroyed_cb(op->userData);

			op->userData = NULL;
		}
	}
	else if (status >= 300)
	{
		priv_destroy_oper_with_disconnect(ssc, op);
                op = NULL;
        }
}

/**
 * Callback for response to an outgoing INVITE request.
 */
void ssc_r_invite (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   RxResponseParams rxResponse;

   SSCDebugHigh ("%s: INVITE: %03d %s", ssc?ssc->ssc_name:"<nil>", status, phrase);

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (!op)
   {
      SSCError("%s: NULL SSC operation ptr!", __func__);
      return;
   }

   // this is a warning because if the sip ptr was set to something
   // different then this indicates that we may need to review the
   // situation in case it is due to an error that occurred earlier.
   // though we will correct the value of op-sip below, there may
   // be other integrity issues caused by the error.
   if (op->sip != sip)
   {
      SSCWarning(":%s: WARN: op<%p>->sip<%p> != sip<%p>", __func__,
            op, op->sip, sip);
   }

   // update the operation's response message descriptor to point to the current
   // response message.
	op->sip = sip;

   if (status >= 300)
   {
      op->op_callstate &= ~opc_sent;

      if (ssc->ssc_invite_failure_cb)
      {
         ssc->ssc_invite_failure_cb(op->userData, status);
      }
   }
   else if (status >= 200)
   {
      nua_ack(op->op_handle,
            TAG_IF((sip && sip->sip_to), SIPTAG_TO(sip->sip_to)), TAG_END());

      if (ssc->ssc_ok_cb)
      {
         rxResponse.contentType = NULL;
         rxResponse.contentLen = 0;
         rxResponse.content = NULL;

         if (sip->sip_content_type &&
             su_casenmatch(sip->sip_content_type->c_type, "multipart/", 10))
         {
            SSCDebugLow("parse multipart..");
            rxResponse.mp = msg_multipart_parse(ssc->ssc_home, sip->sip_content_type, (sip_payload_t *)sip->sip_payload);

            if (!rxResponse.mp)
            {
               // FIXME: aaggh! what do we do now??
               SSCError("ERROR: multipart mime content failed parsing");
               return;
            }

            ssc->ssc_inv_ok_cb(op->userData, &rxResponse);
         }

         else
         {
            /*
            if (sip->sip_content_type) rxResponse.contentType = sip->sip_content_type->c_type;
            if (sip->sip_content_length) rxResponse.contentLen = sip->sip_content_length->l_length;
            if (sip->sip_payload) rxResponse.content = sip->sip_payload->pl_data;
            */
            ssc->ssc_ok_cb(op->userData, sip->sip_payload->pl_data, sip->sip_payload->pl_len);
         }
            
         /*
         if (sip->sip_content_type &&
             su_casenmatch(sip->sip_content_type->c_type, "application/sdp", 15)) 
         {
            ssc->ssc_ok_cb(op->userData, sip->sip_payload->pl_data);
         }
         */

         //ssc->ssc_ok_cb(op->userData, &rxResponse);
      }
   }
   else if (status >= 100)
   {
      if (ssc->ssc_1xx_cb)
      {
         ssc->ssc_1xx_cb(op->userData, status);
      }
   }
}

/**
 * Incoming call fork.
 */
void ssc_i_fork (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   nua_handle_t *nh2 = NULL;

   SSCDebugHigh ("%s: call fork: %03d %s", ssc?ssc->ssc_name:"<nil>", status, phrase);

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   /* We just release forked calls. */
   tl_gets (tags, NUTAG_HANDLE_REF (nh2), TAG_END ());
   if (!nh2) return;

   nua_bye (nh2, TAG_END ());
   nua_handle_destroy (nh2);
}

/**
 * Incoming INVITE request.
 */
void ssc_i_invite (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   /* Incoming call */
	RxInviteParams rxInvite;
   rxInvite.paramLength = 0;
   rxInvite.paramBuffer = NULL;

   sip_request_t const *requestUri;
   sip_contact_t const *contactUri;
   sip_from_t const *from;
   sip_to_t const *to;

   if (!sip) 
   {
      SSCError("NULL sip object received");
      return;
   }

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   to = sip->sip_to;
   from = sip->sip_from;
   requestUri = sip->sip_request;
   contactUri = sip->sip_contact;

   /* some header sanity checks */

   // TODO- the following should maybe cause an error response
   // (such as '500 internal error' or '400 bad request') instead
   // of just ignoring the message
   if (!to)
   {
      SSCError("to is NULL");
      return;
   }

   if (!to->a_url)
   {
      SSCError("to has NULL URL");
      return;
   }

   if (!to->a_url->url_user)
   {
      SSCError("unable to parse username from To: URL");
      return;
   }

   if (!to->a_url->url_host)
   {
      SSCError("unable to parse host from To: URL");
      return;
   }

   if (!from)
   {
      SSCError("from is NULL");
      return;
   }

   if (!from->a_url)
   {
      SSCError("from has NULL URL");
      return;
   }

   if (!from->a_url->url_user)
   {
      SSCError("unable to parse username from From: URL");
      return;
   }

   if (!from->a_url->url_host)
   {
      SSCError("unable to parse host from From: URL");
      return;
   }

   if (!requestUri)
   {
      SSCError("requestUri is NULL");
      return;
   }

   if (!requestUri->rq_url)
   {
      SSCError("request URI has null URL");
      return;
   }

   if (!contactUri) 
   {
      SSCError("contactUri is NULL");
      return;
   }

   /* end header field sanity checks */

	char *tok;

	rxInvite.request.user = requestUri->rq_url->url_user;
	rxInvite.request.host = requestUri->rq_url->url_host;
	rxInvite.request.paramCount = 0;
	tok = strtok((char *)requestUri->rq_url->url_params, ";");
	while (tok && rxInvite.request.paramCount < MAX_URL_PARAM)
	{
		rxInvite.request.params[rxInvite.request.paramCount++] = tok;
		tok = strtok(NULL, ";");
	}

   rxInvite.to.user = to->a_url->url_user;
   rxInvite.to.host = to->a_url->url_host;
	rxInvite.to.paramCount = 0;
   /* don't bother with to: paramters for now */

	rxInvite.from.user = from->a_url->url_user;
	rxInvite.from.host = from->a_url->url_host;
	rxInvite.from.paramCount = 0;
	tok = strtok((char *)from->a_url->url_params, ";");
	while (tok && rxInvite.from.paramCount < MAX_URL_PARAM)
	{
		rxInvite.from.params[rxInvite.from.paramCount++] = tok;
		tok = strtok(NULL, ";");
	}

   rxInvite.priority = NULL;
   if (sip->sip_priority)
   {
      rxInvite.priority = sip->sip_priority->g_string;
   }

	parseSipContacts(ssc, sip, NULL, rxInvite.contact, 1);

	rxInvite.mp = NULL;

SSCDebugLow("checking content type..");
   rxInvite.contentType = NULL;
   rxInvite.contentLen = 0;
   rxInvite.content = NULL;

   if (sip->sip_content_type &&
         su_casenmatch(sip->sip_content_type->c_type, "multipart/", 10)) 
   {
SSCDebugLow("parse multipart..");
      rxInvite.mp = msg_multipart_parse(ssc->ssc_home,
                    sip->sip_content_type,
                    (sip_payload_t *)sip->sip_payload);

      if (!rxInvite.mp)
      {
         // FIXME ... error handling ...
         SSCDebugHigh("ERROR: multipart failed parsing");
      }
   }
   else
   {
      if (sip->sip_content_type)
      {
         rxInvite.contentType = sip->sip_content_type->c_type;
      }

      if (sip->sip_content_length)
      {
         rxInvite.contentLen = sip->sip_content_length->l_length;
      }

      if (sip->sip_payload)
      {
         rxInvite.content = sip->sip_payload->pl_data;
      }
//SSCDebugLow("get remote sdp");
//      tl_gets (tags,
//            SOATAG_REMOTE_SDP_STR_REF (params.remoteSdp), 
//            TAG_END ());
   }
//SSCDebugLow("remote SDP: <%p>", params.remoteSdp);

   if (op)
   {
      op->op_callstate |= opc_recv;
   }
   else
   {
      if ((op = priv_oper_create_uri_with_handle (ssc, SIP_METHOD_INVITE, nh, from, to, from)))
      {
         op->op_callstate = opc_recv;
      }
      else
      {
         nua_respond (nh, SIP_500_INTERNAL_SERVER_ERROR, TAG_END ());
         nua_handle_destroy (nh);
      }
   }

   if (op)
   {
      if (op->op_callstate == opc_recv)
      {
         SSCDebugHigh ("%s: incoming call From: %s", ssc->ssc_name,
               op->op_ident);
         SSCDebugHigh ("   Request URI: <" URL_PRINT_FORMAT ">",
               URL_PRINT_ARGS (requestUri->rq_url));
         SSCDebugHigh ("   To URI: <" URL_PRINT_FORMAT ">",
               URL_PRINT_ARGS (to->a_url));
         SSCDebugHigh ("   From URI: <" URL_PRINT_FORMAT ">",
               URL_PRINT_ARGS (from->a_url));
         SSCDebugHigh ("   Contact URI: %s%s<" URL_PRINT_FORMAT ">",
               contactUri->m_display ? contactUri->m_display : "",
               contactUri->m_display ? " " : "", URL_PRINT_ARGS (contactUri->m_url));


         if (ssc->cb_i_invite_extra)
         {
            uint8_t paramBuffer[1024];
            uint16_t paramLength = 0;

            ssc->cb_i_invite_extra(
                  ssc,
                  sip,
                  paramBuffer,
                  sizeof(paramBuffer),
                  &paramLength);

            rxInvite.paramLength = paramLength;
            rxInvite.paramBuffer = paramBuffer;
         }

         if (ssc->ssc_invite_cb)
         {
            ssc->ssc_invite_cb(ssc->userData, op, &rxInvite);
         }
      }
      else
      {
         SSCDebugHigh ("%s: re-INVITE from: %s", ssc->ssc_name, op->op_ident);
      }
   }
}

void ssc_i_subscribe (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
	RxSubscribeParams params;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

	if (!sip)
	{
		SSCError("NULL sip message struct");
		return;
	}

	if (!sip->sip_to)
	{
		SSCError("'to:' header missing");
		return;
	}
	
	if (!sip->sip_from)
	{
		SSCError("'from:' header missing");
		return;
	}

	if (!sip->sip_request)
	{
		SSCError("request URI missing");
		return;
	}

	params.requestUser = sip->sip_request->rq_url->url_user;
	//params.toUri = url_as_string(ssc->ssc_home, sip->sip_to->a_url);
	params.fromUri = url_as_string(ssc->ssc_home, sip->sip_from->a_url);

	parseSipAccept(ssc, sip, &params.numAccept, params.accept);
	parseSipContacts(ssc, sip, NULL, params.contact, 1);

	params.event = NULL;
	if (sip->sip_event)
	{
		params.event = sip->sip_event->o_type;
	}

	params.expiresPresent = 0;
	if (sip->sip_expires)
	{
		params.expiresPresent = 1;
		params.expiresValue = sip->sip_expires->ex_date;
	}

	if (ssc->ssc_subscribe_cb)
	{
		ssc->ssc_subscribe_cb(ssc->userData, op, &params);
	}
}

void ssc_i_register (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   const sip_from_t *reqFrom;

   sip_contact_t *respContact;
   sip_to_t *respTo;
   sip_from_t *respFrom;
   sip_call_id_t *respCallId;
   sip_cseq_t *respCSeq;
   sip_expires_t *respExpires;

   char payloadBuf[256];
   char contactExpires[64];

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

	if (!sip)
	{
		SSCError("NULL sip message struct");
		return;
	}

   reqFrom = sip->sip_from;

	if (!sip->sip_to)
	{
		SSCError("'to:' header missing");
		return;
	}
	
   if (!sip->sip_to->a_url)
   {
      SSCError("%s: to has NULL URL", __func__);
      return;
   }

   if (!sip->sip_to->a_url[0].url_user ||
	    !sip->sip_to->a_url[0].url_host)
	{
		SSCError("badly formed to: url");
		return;
	}

	if (!sip->sip_from)
	{
		SSCError("'from:' header missing");
		return;
	}

   if (!sip->sip_from->a_url)
   {
      SSCError("%s: from has NULL URL", __func__);
      return;
   }

   if (!sip->sip_contact)
   {
      SSCError("%s: missing Contact header", __func__);
      return;
   }

   if (!sip->sip_expires)
   {
      SSCError("%s: missing Expires header", __func__);
      return;
   }

   // force hardcoded issi-like 200 response
   // (this is a hack.  it was added as a quick way to deal with
   //  the issi register-query that EFJ RFSS sends.  If it does
   //  not get an answer, subsequent ISSI operations fail. so
   //  we always answer the register with success.
   //  future version will need to pass all received registeres
   //  up to epttd so that epttd can parse them. We have a request
   //  to support register-query triggering restriation establishment
   //  and registration refresh.)

   respTo = sip_to_dup(ssc->ssc_home, sip->sip_to);
   respFrom = sip_from_dup(ssc->ssc_home, sip->sip_from);
   respCallId = sip_call_id_dup(ssc->ssc_home, sip->sip_call_id);
   respCSeq = sip_cseq_dup(ssc->ssc_home, sip->sip_cseq);

   respContact = sip_contact_dup(ssc->ssc_home, sip->sip_contact);
   sprintf(contactExpires, "expires=%lu", (unsigned long)43200 /*sip->sip_expires->ex_date*/);
   sip_contact_add_param(ssc->ssc_home, respContact, contactExpires);
   
   respExpires = sip_expires_dup(ssc->ssc_home, sip->sip_expires);

   sprintf((char *)respContact->m_url[0].url_host, "01.002.ABCDE.p25dr");
   sprintf(payloadBuf, "g-rfhangt:10\r\ng-ccsetupT:32767\r\ng-intmode:0\r\ng-man90-alias:ESChat Test\r\n");

   nua_respond(nh, 200, "OK",
         NUTAG_WITH_THIS(nua),
         SIPTAG_TO(respTo),
         SIPTAG_FROM(respFrom),
         SIPTAG_CALL_ID(respCallId),
         SIPTAG_CSEQ(respCSeq),
         SIPTAG_CONTACT(respContact),
         SIPTAG_EXPIRES(respExpires),
         SIPTAG_CONTENT_TYPE_STR("application/x-tia-p25-issi"),
         SIPTAG_PAYLOAD_STR(payloadBuf),
         TAG_END());
}

/** 
 * Answers a call 
 *
 * See also ssc_i_invite().
 */
void ssc_answer (ssc_oper_t * op, int status, char const *phrase, ssc_config_t *config)
{
   if (op != NULL && config != NULL)
   {
		if (status >= 200 && status < 300)
		{
         const char *content = NULL;

         // content contains SDP only
         if (config->sdp[0] != '\0')
         {
            content = config->sdp;
         }

         // multipart content overrides SDP-only content
			if (config->payloadStr[0] != '\0')
         {
            content = config->payloadStr;
         }

         if (content)
			{
				char contactBuffer[1024];

				if (config->contactUri[0] != '\0')
				{
					snprintf(contactBuffer, 1024, "<%s%s>%s", config->contactUri, config->contactUriFeatures, config->contactFeatures);
				}
				else
				{
					contactBuffer[0] = '\0';
				}

				if (status >= 200 && status < 300)
				{
					op->op_callstate |= opc_sent;
				}
				else
				{
					op->op_callstate = opc_none;
				}

				SSCDebugHigh("response: 200 OK");
				SSCDebugHigh("status: %d", status);
				SSCDebugHigh("phrase: '%s'", phrase);
				SSCDebugHigh("via: '%s'", config->via);
				SSCDebugHigh("contactBuffer: '%s'", contactBuffer);
				SSCDebugHigh("expires: '%s'", config->expires);
				SSCDebugHigh("contentType_s: '%s'", config->contentTypeStr);
				SSCDebugHigh("contentLen: '%s'", config->contentLength);
				SSCDebugHigh("content: '%s'", content);

				nua_respond (op->op_handle, status, phrase,
						TAG_IF(strlen(contactBuffer), SIPTAG_CONTACT_STR(contactBuffer)),
						TAG_IF(strlen(config->expires), SIPTAG_SESSION_EXPIRES_STR(config->expires)),
				      TAG_IF(strlen(config->via), SIPTAG_VIA_STR(config->via)),
						//TAG_IF(strlen(config->sdp), SOATAG_USER_SDP_STR (config->sdp)),
						//TAG_IF(strlen(config->sdp), SIPTAG_PAYLOAD_STR(config->sdp)),
						TAG_IF(strlen(config->contentTypeStr), SIPTAG_CONTENT_TYPE_STR(config->contentTypeStr)),
						TAG_IF(strlen(config->contentLength), SIPTAG_CONTENT_LENGTH_STR(config->contentLength)),
                  SIPTAG_PAYLOAD_STR(content),
						//SOATAG_RTP_SORT (SOA_RTP_SORT_REMOTE),
						//SOATAG_RTP_SELECT (SOA_RTP_SELECT_ALL),
						TAG_END ());
			}
			else
			{
				SSCDebugHigh
					("ERROR: no SDP provided by media subsystem, unable to answer call.");
				op->op_callstate = opc_none;
				nua_respond (op->op_handle, 500, "Not Acceptable Here",
						TAG_END ());
			}
		}
		else // status < 200 || status >= 300
      {
         /* call rejected */
         nua_respond (op->op_handle, status, phrase, TAG_END ());
         priv_destroy_oper_with_disconnect (op->op_ssc, op);
         op = NULL;
      }
   }
   else
   {
      SSCWarning("NULL operation object received");
   }
}

/**
 * Incoming PRACK request.
 */
void ssc_i_prack (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   sip_rack_t const *rack;

   if (!sip) return;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   rack = sip->sip_rack;

   SSCDebugHigh ("%s: received PRACK %u", ssc->ssc_name,
         rack ? rack->ra_response : 0);

   if (op == NULL)
      nua_handle_destroy (nh);
}

/**
 * Callback issued for any change in operation state.
 */
void ssc_i_state (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   int offer_recv = 0, answer_recv = 0, offer_sent = 0, answer_sent = 0;
   int ss_state = nua_callstate_init;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   tl_print (stdout, "", tags);

   tl_gets (tags,
         NUTAG_CALLSTATE_REF (ss_state),
         NUTAG_OFFER_RECV_REF (offer_recv),
         NUTAG_ANSWER_RECV_REF (answer_recv),
         NUTAG_OFFER_SENT_REF (offer_sent),
         NUTAG_ANSWER_SENT_REF (answer_sent),
         TAG_END ());

   SSCDebugHigh("SSC state change: now: %d", ss_state);

   if (op && sip)
   {
      // this is a warning because if the sip ptr was set to something
      // different then this indicates that we may need to review the
      // situation in case it is due to an error that occurred earlier.
      // though we will correct the value of op-sip below, there may
      // be other integrity issues caused by the error.
      if (op->sip != sip)
      {
         SSCWarning(":%s: WARN: op<%p>->sip<%p> != sip<%p>", __func__,
               op, op->sip, sip);
      }

      // update the operation's sip msg ptr to point to the received response
      // if this call state change event is associated with one.
      op->sip = sip;
   }

   switch ((enum nua_callstate) ss_state)
   {
      case nua_callstate_received:
         break;

      case nua_callstate_early:
         break;

      case nua_callstate_completing:
         break;

      case nua_callstate_ready:
         if (op->op_prev_state != ss_state)
         {
            if (op->op_prev_state == nua_callstate_completing)
            {
               if (ssc->ssc_ack_cb)
               {
                  ssc->ssc_ack_cb(op->userData);
               }
            }
         }
         break;

      case nua_callstate_terminated:
         if (op)
         {
            SSCDebugHigh ("%s: call to %s is terminated", ssc->ssc_name,
                  op->op_ident);
            op->op_callstate = 0;
            priv_destroy_oper_with_disconnect (ssc, op);
            op = NULL;
         }
         break;

      default:
         break;
   }

   if (op)
   {
      op->op_prev_state = ss_state;
   }

   if (ssc->ssc_call_state_cb)
   {
      ssc->ssc_call_state_cb (ssc, op, ss_state, ssc->userData);
   }
}

/**
 * Sends a ACK request to an active operation
 */
void ssc_ack (ssc_oper_t * op)
{
/*
   SSCDebugLow("ssc_ack(<%p>, <%p>)", op, cfg);
   SSCDebugLow("  To - '%s'", cfg->toUri);
   */

   if (op)
   {
      SSCDebugHigh ("ACK to %s", op->op_ident);
      nua_ack(op->op_handle, 
            //TAG_IF((cfg->toUri[0] != '\0'), SIPTAG_TO_STR(cfg->toUri)),
            TAG_END());
   }
   else
   {
      SSCWarning("NULL operation object received");
   }
}

/**
 * Sends a RINGING request to an active operation
 */
void ssc_ringing (ssc_oper_t * op)
{
   if (op)
   {
      SSCDebugHigh ("RINGING to %s", op->op_ident);
      nua_respond(op->op_handle, SIP_180_RINGING, TAG_END());
   }
   else
   {
      SSCWarning("NULL operation object received");
   }
}

/**
 * Sends a BYE request to an active operation 
 */
void ssc_bye (ssc_oper_t * op, ssc_config_t *cfg)
{
   if (op && cfg)
   {
      SSCDebugHigh ("BYE to %s [via '%s']", op->op_ident,cfg->targetAddress);
      SSCDebugLow("to - '%s'", cfg->toUri);
      nua_bye (op->op_handle,
         TAG_IF((cfg->targetAddress[0] != '\0'), NUTAG_PROXY(cfg->targetAddress)),
         TAG_IF((cfg->toUri[0] != '\0'), SIPTAG_TO_STR(cfg->toUri)),
		   TAG_END());
      op->op_callstate = 0;
   }
   else
   {
      SSCWarning("NULL operation object received");
   }
}

/**
 * Callback for an outgoing BYE request.
 */
void ssc_r_bye (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (!op)
   {
      SSCError("%s: NULL SSC operation ptr!", __func__);
      return;
   }

   if (op->op_handle != nh)
   {
      SSCError("%s: operation handle mismatch! <%p> != <%p>", __func__,
         op->op_handle, nh);
      return;
   }

   SSCDebugHigh ("%s: BYE: %03d %s", ssc->ssc_name, status, phrase);
   if (status < 200)
      return;

   if (status >= 200 && status < 300)
   {
      nua_ack(op->op_handle,
            TAG_IF((sip && sip->sip_to), SIPTAG_TO(sip->sip_to)), TAG_END());
   }
      
   if (ssc->ssc_bye_cb)
   {
      ssc->ssc_bye_cb(op->userData);
   }
}

/**
 * Incoming BYE request. Note, call state related actions are
 * done in the ssc_i_state() callback.
 */
void ssc_i_bye (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (op && op->op_handle != nh)
   {
      SSCError("%s: operation handle mismatch! <%p> != <%p>", __func__,
         op->op_handle, nh);
      return;
   }

   SSCDebugHigh ("%s: BYE received", ssc->ssc_name);

   if (ssc->ssc_bye_cb)
   {
      ssc->ssc_bye_cb(op->userData);
   }
}

/**
 * Cancels a call operation currently in progress (if any).
 */
void ssc_cancel (ssc_oper_t * op)
{
   if (op)
   {
      SSCDebugHigh ("CANCEL %s to %s",
            op->op_method_name, op->op_ident);
      nua_cancel (op->op_handle, TAG_END ());
   }
   else
   {
      SSCWarning("NULL operation object received");
   }
}

/**
 * Incoming CANCEL.
 */
void ssc_i_cancel (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (op && op->op_handle != nh)
   {
      SSCError("%s: operation handle mismatch! <%p> != <%p>", __func__,
         op->op_handle, nh);
      return;
   }

   SSCDebugHigh ("%s: CANCEL received", ssc->ssc_name);

   if (ssc->ssc_cancel_cb)
   {
		if (op->userData)
		{
			ssc->ssc_cancel_cb(op->userData);
		}
		else
		{
			SSCWarning("CANCEL received but NULL transaction context!");
		}
   }
}


void ssc_message (ssc_t * ssc, const char *destination, const char *msg)
{
   ssc_oper_t *op = NULL;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   op = priv_oper_create (ssc, SIP_METHOD_MESSAGE, destination, "", "", TAG_END());

   if (op)
   {

      SSCDebugHigh ("%s: sending message to %s", ssc->ssc_name, op->op_ident);

      nua_message (op->op_handle,
            SIPTAG_CONTENT_TYPE_STR ("text/plain"),
            SIPTAG_PAYLOAD_STR (msg), TAG_END ());
   }
}

void ssc_r_message (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   SSCDebugHigh ("%s: MESSAGE: %d %s", ssc?ssc->ssc_name:"", status, phrase);

   if (status < 200)
      return;

   if (status == 401 || status == 407)
   {
      // FIXME: error
   }

   if (status >= 200 && status < 300)
   {
      nua_ack(op->op_handle,
         TAG_IF((sip && sip->sip_to), SIPTAG_TO(sip->sip_to)), TAG_END());
   }
}

void ssc_i_message (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   /* Incoming message */
   sip_from_t const *from;
   sip_to_t const *to;
   sip_subject_t const *subject;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (!sip)
   {
      SSCError("%s: NULL SIP request ptr!", __func__);
      return;
   }

   if (!sip->sip_from)
   {
      SSCError("%s: MESSAGE with NULL from component", __func__);
      return;
   }

   if (!sip->sip_to)
   {
      SSCError("%s: MESSAGE with NULL to component", __func__);
      return;
   }

   from = sip->sip_from;
   to = sip->sip_to;
   subject = sip->sip_subject;

   SSCDebugHigh ("%s: new message ", ssc->ssc_name);
   SSCDebugHigh ("\tFrom: %s%s" URL_PRINT_FORMAT "",
         from->a_display ? from->a_display : "", from->a_display ? " " : "",
         URL_PRINT_ARGS (from->a_url));
   if (subject)
      SSCDebugHigh ("\tSubject: %s", subject->g_value);
   ssc_print_payload (ssc, sip->sip_payload);

   if (op == NULL)
      op = priv_oper_create_uri_with_handle (ssc, SIP_METHOD_MESSAGE, nh, from, to, from);
   if (op == NULL)
      nua_handle_destroy (nh);
}


/*---------------------------------------*/
void ssc_subscribe (ssc_t * ssc, char *destination)
{
   ssc_oper_t *op;
   char const *event = "presence";
   char const *supported = NULL;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (strncasecmp (destination, "list ", 5) == 0)
   {
      destination += 5;
      while (*destination == ' ')
         destination++;
      supported = "eventlist";
   }

   op = priv_oper_create (ssc, SIP_METHOD_SUBSCRIBE, destination, "", "", TAG_END ());

   if (op)
   {
      SSCDebugHigh ("%s: SUBSCRIBE %s to %s", ssc->ssc_name?ssc->ssc_name:"<nil>", event, op->op_ident?op->op_ident:"<nil>");
      nua_subscribe (op->op_handle,
            SIPTAG_EXPIRES_STR ("3600"),
            SIPTAG_ACCEPT_STR ("application/cpim-pidf+xml;q=0.5, "
               "application/pidf-partial+xml"),
            TAG_IF (supported,
               SIPTAG_ACCEPT_STR ("multipart/related, "
                  "application/rlmi+xml")),
            SIPTAG_SUPPORTED_STR (supported),
            SIPTAG_EVENT_STR (event), TAG_END ());
   }
}

void ssc_watch (ssc_t * ssc, char *event)
{
   ssc_oper_t *op;
   char *destination;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   destination = strchr (event, ' ');
   while (destination && *destination == ' ')
      *destination++ = '\0';

   op = priv_oper_create (ssc, SIP_METHOD_SUBSCRIBE, destination, "", "", TAG_END ());

   if (op)
   {
      SSCDebugHigh ("%s: SUBSCRIBE %s to %s", ssc->ssc_name?ssc->ssc_name:"<nil>", event, op->op_ident?op->op_ident:"<nil>");
      nua_subscribe (op->op_handle, SIPTAG_EVENT_STR (event), TAG_END ());
   }
}

void ssc_r_subscribe (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   SSCDebugHigh ("%s: SUBSCRIBE: %03d %s", ssc->ssc_name?ssc->ssc_name:"<nil>", status, phrase);

   if (status < 200)
      return;
   if (status >= 300 && op)
      op->op_persistent = 0;
   if (status == 401 || status == 407)
   {
      // FIXME: error
   }
   if (status >= 200 && status < 300)
   {
      nua_ack(op->op_handle,
            TAG_IF((sip && sip->sip_to), SIPTAG_TO(sip->sip_to)), TAG_END());
   }
}

/*---------------------------------------*/
void ssc_notify (ssc_t * ssc, char *destination)
{
   ssc_oper_t *op = ssc_oper_find_call_embryonic (ssc);

   if (op)
   {
      SSCDebugHigh ("%s: not follow refer, NOTIFY(503)", ssc->ssc_name?ssc->ssc_name:"<nil>");

      nua_cancel (op->op_handle, TAG_END ());
      ssc_oper_destroy (ssc, op);
   }
   else
   {
      SSCDebugHigh ("%s: no REFER to NOTIFY", ssc->ssc_name?ssc->ssc_name:"<nil>");
   }
}

/*---------------------------------------*/
void ssc_i_notify (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (sip)
   {
      sip_from_t const *from = sip->sip_from;
      sip_event_t const *event = sip->sip_event;
      sip_content_type_t const *content_type = sip->sip_content_type;
      sip_payload_t const *payload = sip->sip_payload;

      if (op)
         SSCDebugHigh ("%s: NOTIFY from %s", ssc->ssc_name?ssc->ssc_name:"<nil>", op->op_ident?op->op_ident:"<nil>");
      else
         SSCDebugHigh ("%s: rogue NOTIFY from " URL_PRINT_FORMAT "",
               ssc->ssc_name?ssc->ssc_name:"<nil>", URL_PRINT_ARGS (from->a_url));
      if (event)
         SSCDebugHigh ("\tEvent: %s", event->o_type);
      if (content_type)
         SSCDebugHigh ("\tContent type: %s", content_type->c_type);
      fputs ("", stdout);
      ssc_print_payload (ssc, payload);
   }
   else if (op)
      SSCDebugHigh ("%s: SUBSCRIBE/NOTIFY timeout for %s", ssc->ssc_name?ssc->ssc_name:"<nil>",
            op->op_ident?op->op_ident:"<nil>");
}

/*---------------------------------------*/
void ssc_r_notify (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   /* Respond to notify */
   SSCDebugHigh ("%s: notify: %d %s", (ssc && ssc->ssc_name)?ssc->ssc_name:"<nil>", status, phrase);

   if (status < 200)
      return;

   if (status == 401 || status == 407)
   {
      // FIXME: error
   }

   if (status >= 200 && status < 300)
   {
      nua_ack(op->op_handle,
            TAG_IF((sip && sip->sip_to), SIPTAG_TO(sip->sip_to)), TAG_END());
   }
}

/*---------------------------------------*/

void ssc_i_options (nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{

}

/*---------------------------------------*/

void ssc_unsubscribe (ssc_t * ssc, char *destination)
{
   ssc_oper_t *op = ssc_oper_find_by_method (ssc, sip_method_subscribe);

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!",__func__);
      return;
   }

   if (op)
   {
      SSCDebugHigh ("%s: un-SUBSCRIBE to %s", ssc->ssc_name?ssc->ssc_name:"<nil>", op->op_ident?op->op_ident:"<nil>");
      nua_unsubscribe (op->op_handle, TAG_END ());
   }
   else
      SSCDebugHigh ("%s: no subscriptions", ssc->ssc_name?ssc->ssc_name:"<nil>");
}

void ssc_r_unsubscribe (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!",__func__);
      return;
   }

   SSCDebugHigh ("%s: un-SUBSCRIBE: %03d %s", ssc->ssc_name?ssc->ssc_name:"<nil>", status, phrase);

   if (status < 200)
      return;

   ssc_oper_destroy (ssc, op);
}

void ssc_publish (ssc_t * ssc, const char *note)
{
   ssc_oper_t *op;
   sip_payload_t *pl = NULL;
   char *address;
   char *xmlnote = NULL;
   int open;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (!ssc->ssc_home)
   {
      SSCError("%s: SSC context has NULL home", __func__);
      return;
   }

   open = note == NULL || note[0] != '-';

   if (note && strcmp (note, "-") != 0)
      xmlnote = su_sprintf (ssc->ssc_home, "<note>%s</note>",
            open ? note : note + 1);

   pl = sip_payload_format
      (ssc->ssc_home,
       "<?xml version='1.0' encoding='UTF-8'?>"
       "<presence xmlns='urn:ietf:params:xml:ns:cpim-pidf'"
       "          entity='%s'>"
       "  <tuple id='%s'>"
       "    <status><basic>%s</basic></status>"
       "%s"
       "  </tuple>"
       "</presence>",
       ssc->ssc_address?ssc->ssc_address:"<nil>", ssc->ssc_name?ssc->ssc_name:"<nil>",
       open ? "open" : "closed", xmlnote ? xmlnote : "");

   if ((op = ssc_oper_find_by_method (ssc, sip_method_publish)))
   {
      SSCDebugHigh ("%s: %s %s", ssc->ssc_name?ssc->ssc_name:"<nil>", op->op_method_name?op->op_method_name:"<nil>", op->op_ident?op->op_ident:"<nil>");
      nua_publish (op->op_handle,
            SIPTAG_PAYLOAD (pl),
            TAG_IF (pl,
               SIPTAG_CONTENT_TYPE_STR
               ("application/cpim-pidf+xml")), TAG_NULL ());

      su_free (ssc->ssc_home, pl);
      return;
   }

   address = su_strdup (ssc->ssc_home, ssc->ssc_address);

   if ((op = priv_oper_create (ssc, SIP_METHOD_PUBLISH, address, "", "",
               SIPTAG_EVENT_STR ("presence"), TAG_END ())))
   {
      SSCDebugHigh ("%s: %s %s", ssc->ssc_name?ssc->ssc_name:"<nil>", op->op_method_name?op->op_method_name:"<nil>", op->op_ident?op->op_ident:"<nil>");
      nua_publish (op->op_handle,
            SIPTAG_CONTENT_TYPE_STR ("application/cpim-pidf+xml"),
            SIPTAG_PAYLOAD (pl), TAG_END ());
   }

   su_free (ssc->ssc_home, pl);
   su_free (ssc->ssc_home, address);
}

void ssc_unpublish (ssc_t * ssc)
{
   ssc_oper_t *op;
   char *address;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if ((op = ssc_oper_find_by_method (ssc, sip_method_publish)))
   {
      SSCDebugHigh ("%s: %s %s", ssc->ssc_name?ssc->ssc_name:"<nil>", op->op_method_name?op->op_method_name:"<nil>", op->op_ident?op->op_ident:"<nil>");
      nua_publish (op->op_handle, SIPTAG_EXPIRES_STR ("0"), TAG_NULL ());
      return;
   }

   address = su_strdup (ssc->ssc_home, ssc->ssc_address);

   if ((op = priv_oper_create (ssc, SIP_METHOD_PUBLISH, address, "", "",
               SIPTAG_EVENT_STR ("presence"), TAG_END ())))
   {
      SSCDebugHigh ("%s: un-%s %s", ssc->ssc_name?ssc->ssc_name:"<nil>", op->op_method_name?op->op_method_name:"<nil>",
            op->op_ident?op->op_ident:"<nil>");
      nua_publish (op->op_handle, SIPTAG_EXPIRES_STR ("0"), TAG_END ());
   }

   su_free (ssc->ssc_home, address);
}

/**
 * Sets the public address used for invites, messages,
 * registrations, etc method.
 */
void ssc_set_public_address (ssc_t * ssc, const char *address)
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   if (address)
   {
      su_free (ssc->ssc_home, ssc->ssc_address);
      ssc->ssc_address = su_strdup (ssc->ssc_home, address);

      nua_set_params (ssc->ssc_nua,
            SIPTAG_FROM_STR (ssc->ssc_address), TAG_NULL ());
   }
}

/**
 * Callback for an outgoing PUBLISH request.
 */
void ssc_r_publish (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   SSCDebugHigh ("%s: PUBLISH: %03d %s", ssc->ssc_name?ssc->ssc_name:"<nil>", status, phrase);

   if (status < 200)
      return;

   if (status == 401 || status == 407)
   {
      // FIXME: error
   }
   else if (status >= 300)
      ssc_oper_destroy (ssc, op);
   else if (!sip->sip_expires || sip->sip_expires->ex_delta == 0)
      ssc_oper_destroy (ssc, op);
}

void ssc_r_shutdown (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   SSCDebugHigh ("%s: nua_shutdown: %03d %s", ssc->ssc_name?ssc->ssc_name:"<nil>", status, phrase);

   if (status < 200)
      return;

   if (ssc->ssc_exit_cb)
      ssc->ssc_exit_cb ();
}

/**
 * Result callback for nua_r_get_params request.
 */
void ssc_r_get_params (int status, char const *phrase,
      nua_t * nua, ssc_t * ssc,
      nua_handle_t * nh, ssc_oper_t * op, sip_t const *sip,
      tagi_t tags[])
{
   sip_from_t const *from = NULL;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   SSCDebugHigh ("%s: nua_r_getparams: %03d %s", ssc->ssc_name?ssc->ssc_name:"<nil>", status, phrase);
   //tl_print (stdout, "", tags);

   tl_gets (tags, SIPTAG_FROM_REF (from), TAG_END ());

   if (from)
   {
      char const *new_address =
         sip_header_as_string (ssc->ssc_home, (sip_header_t *) from);
      if (new_address)
      {
         su_free (ssc->ssc_home, (char *) ssc->ssc_address);
         ssc->ssc_address = su_strdup (ssc->ssc_home, new_address);
      }
   }
}

/**
 * Prints SIP message payload to stdout.
 */
void ssc_print_payload (ssc_t * ssc, sip_payload_t const *pl)
{
   fputs ("", stdout);
   if (pl)
   {
      fwrite (pl->pl_data, pl->pl_len, 1, stdout);
      if (pl->pl_len < 1 ||
            (pl->pl_data[pl->pl_len - 1] != '\n' ||
             pl->pl_data[pl->pl_len - 1] != '\r'))
         fputs ("\n", stdout);
      else
         fputs ("", stdout);
   }
}

void ssc_print_settings (ssc_t * ssc)
{
   SSCDebugHigh ("SIP address...........: %s", (ssc && ssc->ssc_address)?ssc->ssc_address:"<nil>");
}

void ssc_param (ssc_t * ssc, char *param, char *s)
{
   tag_type_t tag = NULL, *list;
   tag_value_t value = 0;
   char *ns = NULL, *sep;
   su_home_t home[1] = { SU_HOME_INIT (home) };
   int scanned;

   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   enter;

   if ((sep = strstr (param, "::")))
   {
      ns = param, *sep = '\0', param = sep + 2;
   }
   else if ((sep = strstr (param, ".")))
   {
      ns = param, *sep = '\0', param = sep + 1;
   }
   else if ((sep = strstr (param, ":")))
   {
      ns = param, *sep = '\0', param = sep + 1;
   }

   if (!ns || strcmp (ns, "nua") == 0)
      for (list = nua_tag_list; (tag = *list); list++)
      {
         if (strcmp (tag->tt_name, param) == 0)
         {
            ns = "found";
            break;
         }
      }
   if (!ns || strcmp (ns, "nta") == 0)
      for (list = nta_tag_list; (tag = *list); list++)
      {
         if (strcmp (tag->tt_name, param) == 0)
         {
            ns = "found";
            break;
         }
      }
   if (!ns || strcmp (ns, "sip") == 0)
      for (list = sip_tag_list; (tag = *list); list++)
      {
         if (strcmp (tag->tt_name, param) == 0)
         {
            ns = "found";
            break;
         }
      }


   if (!tag)
   {
      SSCDebugHigh ("sofsip: unknown parameter %s::%s", ns ? ns : "", param);
      return;
   }

   scanned = t_scan (tag, home, s, &value);
   if (scanned <= 0)
   {
      SSCDebugHigh ("sofsip: invalid value for %s::%s", ns ? ns : "", param);
      return;
   }

   nua_set_params (ssc->ssc_nua, tag, value, TAG_NULL ());
   nua_get_params (ssc->ssc_nua, tag, (tag_value_t) 0, TAG_NULL ());

   su_home_deinit (home);
}

void ssc_shutdown (ssc_t * ssc)
{
   if (!ssc)
   {
      SSCError("%s: NULL SSC context ptr!", __func__);
      return;
   }

   enter;

   SSCDebugHigh ("%s: quitting (this can take some time)", ssc->ssc_name?ssc->ssc_name:"<nil>");

   nua_shutdown (ssc->ssc_nua);
}
