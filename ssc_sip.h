/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2005-2006 Nokia Corporation.
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

#ifndef HAVE_SSC_SIP_H
#define HAVE_SSC_SIP_H

/**@file ssc_sip.h Interface towards Sofia-SIP
 *
 * @author Kai Vehmanen <Kai.Vehmanen@nokia.com>
 * @author Pekka Pessi <Pekka.Pessi@nokia.com>
 */

typedef struct ssc_s ssc_t;
typedef struct ssc_auth_item_s ssc_auth_item_t;
typedef struct ssc_conf_s ssc_conf_t;

/* define type of context pointers for callbacks */
#define NUA_MAGIC_T     ssc_t
#define SOA_MAGIC_T     ssc_t

#include "ssc_oper.h"
#include "ssc_types.h"

#include <sofia-sip/sip.h>
#include <sofia-sip/sip_header.h>
#include <sofia-sip/sip_status.h>
#include <sofia-sip/nua.h>
#include <sofia-sip/nua_tag.h>
#include <sofia-sip/soa.h>
#include <sofia-sip/su_tag_io.h>
#include <sofia-sip/su_tagarg.h>
#include <sofia-sip/sl_utils.h>
#include <sofia-sip/su_debug.h>

// The maximum number of contacts that we support in a message that
// may have a variable number of them.
// TODO: review SIP spec and determine a reasonable value for this
// and what standard-appropriate action to take in the event that
// a message exceeds it. For now we just ignore any additional
// contacts received. Current value was just chosen arbitrarily.
#define MAX_CONTACTS 16

// The max number of content type entries supported in the Accept header.
// (see TODO notes for max contacts above)
#define MAX_ACCEPT_TYPES 4

// The max number of URL parameters to support
#define MAX_URL_PARAM 4

#define MAX_UNKNOWN_HEADERS 4

// types for records that can came multiplicity
typedef struct _SscSipContact
{
	const char *contactDisplay;
	const char *contactUri;
	const char *contactUser;
	const char *contactHost;
	uint8_t expiresPresent;
	uint32_t expiresValue;
} SscSipContact;

typedef struct _SscSipAccept
{
	const char *type;
	const char *subtype;
} SscSipAccept;

// structs for passing params back out to callbacks.
// we try to parse the sofia structs as much as possible and
// return the results in basic C types and types defined above.
// struct msg_multipart_t is currently an exception.
typedef struct _UserUri
{
	const char *user;
	const char *host;
	uint8_t paramCount;
	const char *params[MAX_URL_PARAM];
} UserUri;

typedef struct _RxInviteParams
{
	UserUri request;
	UserUri to;
	UserUri from;
	SscSipContact contact[1];
   const char *priority;
   const char *unknown[MAX_UNKNOWN_HEADERS];
	const char *contentType;
	uint32_t contentLen;
	const char *content;
	msg_multipart_t *mp;

   uint16_t paramLength;
   uint8_t *paramBuffer;

} RxInviteParams;

typedef struct _RxResponseParams
{
   const char *contentType;
   uint32_t contentLen;
   const char *content;
   msg_multipart_t *mp;
} RxResponseParams;

typedef struct _RxRegisterParams
{
	const char *toUser;
	const char *toHost;
	const char *fromUri;
	uint8_t numContacts;
	SscSipContact contacts[MAX_CONTACTS];
	uint8_t expiresPresent;
	uint32_t expiresValue;
} RxRegisterParams;

typedef struct _RxSubscribeParams
{
	const char *requestUser;
	const char *fromUri;
	const char *event;
	uint8_t numAccept;
	SscSipAccept accept[MAX_ACCEPT_TYPES];
	SscSipContact contact[1];
	uint8_t expiresPresent;
	uint32_t expiresValue;
} RxSubscribeParams;

typedef void (*ssc_req_callback_f)(
      nua_t *nua,
      ssc_t *ssc,
      nua_handle_t *nh,
      ssc_oper_t *op,
      const sip_t *sip,
      tagi_t tags[]);

typedef void (*ssc_resp_callback_f)(
      int status,
      const char *phrase,
      nua_t *nua,
      ssc_t *ssc,
      nua_handle_t *nh,
      ssc_oper_t *op,
      const sip_t *sip,
      tagi_t tags[]);

typedef void (*ssc_req_extra_callback_f)(
      ssc_t *ssc,
      const sip_t *sip,
      uint8_t *paramBufer,
      size_t paramBufferSize,
      uint16_t *paramBufferUsed);
      
// callback function pointers for SSC events
typedef void (*ssc_exit_cb)(void);
typedef void (*ssc_event_cb)(ssc_t *ssc, nua_event_t event, void *context);
typedef void (*ssc_auth_req_cb)(ssc_t *ssc, const ssc_auth_item_t *authitem, void *context);
typedef void (*ssc_call_state_cb)(ssc_t *ssc, ssc_oper_t *oper, int ss_state, void *context);
typedef void (*ssc_error_cb)(ssc_t *ssc, int status, const char *phrase, ssc_oper_t *oper);

/*
 * return non-0 if resource was not allocated
 */
typedef uint8_t (*ssc_invite_cb)(void *sscUserData, ssc_oper_t *oper, RxInviteParams *params);
typedef uint8_t (*ssc_register_cb)(void *sscUserData, ssc_oper_t *oper, RxRegisterParams *params);
typedef uint8_t (*ssc_subscribe_cb)(void *sscUserData, ssc_oper_t *oper, RxSubscribeParams *params);

typedef void (*ssc_oper_destroyed_cb)(void *operUserData);
typedef void (*ssc_invite_failure_cb)(void *operUserData, int status);
typedef void (*ssc_cancel_cb)(void *operUserData);
typedef void (*ssc_bye_cb)(void *operUserData);
typedef void (*ssc_ack_cb)(void *operUserData);
typedef void (*ssc_ok_cb)(void *operUserData, const char *remoteSdp, usize_t remoteSdpLen);
typedef void (*ssc_inv_ok_cb)(void *operUserData, RxResponseParams *params);
typedef void (*ssc_1xx_cb)(void *operUserData, int status);

typedef void (*ssc_opt_ok_cb)(
      void *operUserData, 
      uint16_t status,
      sip_expires_t *expires,
      sip_contact_t *contact,
      sip_content_type_t *contentType,
      sip_payload_t *payload);
typedef void (*ssc_msg_ok_cb)(
      void *operUserData, 
      uint16_t status,
      sip_expires_t *expires,
      sip_contact_t *contact,
      sip_content_type_t *contentType,
      sip_payload_t *payload);

/**
 * Structure for storing pending authentication requests
 */
struct ssc_auth_item_s {
  char         *ssc_scheme;     /**< Scheme */
  char         *ssc_realm;      /**< Realm part */
  char         *ssc_username;   /**< Username part, if known */
  ssc_oper_t   *ssc_op;         /**< Operation this auth item is related to */
};

/**
 * structure for storing the Network-to-Network Interface
 * type identifiers.
 */
#define NNI_TYPE_LIST_MAX 8   // maximum number of NNI type identifer elements supported by
                              // a single sip stack interface
struct ssc_nni_type_s {
   uint8_t count;  // number of type codes contained
   uint8_t type[NNI_TYPE_LIST_MAX];
};

typedef struct ssc_nni_type_s ssc_nni_type_t;

/**
 * Instance data for ssc_sip_t objects.
 */
struct ssc_s {
  su_home_t    *ssc_home;	/**< Our memory home */
  char const   *ssc_name;	/**< Our name */
  su_root_t    *ssc_root;       /**< Pointer to application root */

  nua_t        *ssc_nua;        /**< Pointer to NUA object */
  ssc_oper_t   *ssc_operations;	/**< Remote destinations */

  char         *ssc_address;    /**< Current AOR */

  void         *userData; /**< Context for callbacks */
  void         *ssc_ext; /* optional extension for protocol specific data */
  void         *ssc_oc; /* operator container */

  ssc_nni_type_t nniType;

  nua_callback_f ssc_nua_cb;

  ssc_exit_cb         ssc_exit_cb;        /**< Callback to signal stack shutdown */
  ssc_event_cb        ssc_event_cb;
  ssc_call_state_cb   ssc_call_state_cb;

  /* Options to override processing */
  ssc_req_callback_f cb_i_register;
  ssc_req_callback_f cb_i_invite;
  ssc_req_callback_f cb_i_message;
  ssc_req_callback_f cb_i_options;
  ssc_req_callback_f cb_i_bye;
  ssc_req_callback_f cb_i_cancel;
 
  ssc_resp_callback_f cb_r_register;
  ssc_resp_callback_f cb_r_invite;
  ssc_resp_callback_f cb_r_options;
  ssc_resp_callback_f cb_r_message;

  ssc_req_extra_callback_f cb_i_invite_extra;

  /* callbacks */
  ssc_invite_cb ssc_invite_cb;
  ssc_register_cb ssc_register_cb;
  ssc_subscribe_cb ssc_subscribe_cb;
  ssc_ok_cb ssc_ok_cb;
  ssc_inv_ok_cb ssc_inv_ok_cb;
  ssc_1xx_cb ssc_1xx_cb;
  ssc_opt_ok_cb ssc_opt_ok_cb;
  ssc_msg_ok_cb ssc_msg_ok_cb;
  ssc_bye_cb ssc_bye_cb;
  ssc_ack_cb ssc_ack_cb;
  ssc_cancel_cb ssc_cancel_cb;
  ssc_oper_destroyed_cb ssc_oper_destroyed_cb;
  ssc_invite_failure_cb ssc_invite_failure_cb;
  ssc_error_cb ssc_error_cb;
};

/** 
 * Configuration data for ssc_create().
 */
struct ssc_conf_s {
  const char   *ssc_aor;        /**< Public SIP address aka AOR (SIP URI) */
  const char   *ssc_certdir;	/**< Directory for TLS certs (directory path) */
  const char   *ssc_contact;	/**< SIP contact URI (local address to use) */
  const char   *ssc_media_addr;	/**< Media address (hostname, IP address) */
  const char   *ssc_media_impl;	/**< Media address (hostname, IP address) */
  const char   *ssc_proxy;	/**< SIP outbound proxy (SIP URI) */
  const char   *ssc_registrar;	/**< SIP registrar (SIP URI) */
  const char   *ssc_stun_server;/**< STUN server address (hostname, IP address) */
  int           ssc_proto_type;
  const char   *ssc_reg_bind_addr;
  const char   *ssc_call_bind_addr;
  int           ssc_flags;
};

#if HAVE_FUNC
#define enter (void)SU_DEBUG_9(("%s: entering\n", __func__))
#elif HAVE_FUNCTION
#define enter (void)SU_DEBUG_9(("%s: entering\n", __FUNCTION__))
#else
#define enter (void)0
#endif

ssc_t *ssc_create(su_home_t *home, su_root_t *root, const ssc_conf_t *conf);
void ssc_destroy(ssc_t *self);

void ssc_answer(ssc_oper_t *op, int status, char const *phrase, ssc_config_t *config);
void ssc_ringing (ssc_oper_t *op);
void ssc_ack (ssc_oper_t *op);
void ssc_bye(ssc_oper_t *op, ssc_config_t *config);
void ssc_cancel(ssc_oper_t *op);

// Builds the payloadOut and contentTypeOut from the sdp and metadata.
// The mimeMsg is a msg_t object with the memory allocations for the 
// sip_payload_t and the sip_content_type_t.
int ssc_create_siprec_mime(
      const char *sdp,
      const char *metadata,
      msg_t **mimeMsg,
      sip_payload_t **payloadOut,
      sip_content_type_t **contentTypeOut);

ssc_oper_t *ssc_invite(ssc_t *ssc, ssc_config_t *config);

// generate and send an outgoing REGISTER request.
// this creates a new NUA handle to manage the request.
// ( internally just a wrapper for
//   ssc_register_op(ssc, config, NULL);  )
ssc_oper_t *ssc_register(ssc_t *ssc, ssc_config_t *config);

// generate and send an outgoing REGISTER request.
// this uses the NUA handle in the provided SSC operation to
// manage the request.  If the SSC operation ptr is NULL,
// then a new operation (and new handle) will be created.
ssc_oper_t *ssc_register_op(ssc_t *ssc, ssc_config_t *config, ssc_oper_t *op);

// generate and send an outgoing OPTIONS request.
// this creates a new NUA handle to manage the request.
ssc_oper_t *ssc_options(ssc_t *ssc, ssc_config_t *config);

void ssc_list(ssc_t *ssc);
void ssc_message(ssc_t *ssc, const char *destination, const char *msg);
void ssc_param(ssc_t *cli, char *param, char *s);
void ssc_publish(ssc_t *ssc, const char *note);
void ssc_unpublish(ssc_t *ssc);
void ssc_set_public_address(ssc_t *ssc, const char *aor);
void ssc_shutdown(ssc_t *ssc);
void ssc_subscribe(ssc_t *ssc, char *destination);
void ssc_unsubscribe(ssc_t *ssc, char *destination);
void ssc_watch(ssc_t *ssc, char *event);

void ssc_print_payload(ssc_t *ssc, sip_payload_t const *pl);
void ssc_print_settings(ssc_t *ssc);

int ssc_match_nniType(ssc_t *ssc, uint8_t nniType);
void ssc_clear_nniType(ssc_t *ssc);
void ssc_add_nniType(ssc_t *ssc, uint8_t nniType);

#endif /* HAVE_SSC_SIP_H */
