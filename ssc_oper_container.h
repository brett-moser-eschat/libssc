#pragma once 
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

/// provides a collection of SSC operations.  Previous SSC just used a linked list
/// of operations and did not have any mechanism for complex searches. we need to
/// look up operations by URI strings pretty often and expectation is there could
/// be thousands of allocated operations (since there will be one per live
/// registration).  Abstracting the collection a bit allows us to implement faster
/// lookups under the hood with fewer string compares without pushing a lot of
/// details out to the user.
///
/// when adding an op to the collection, these functions will take care of needed
/// allocations within the SSC context; hence, a public oc_init() or oc_create()
/// function is not provided.
/// Be sure to call ssc_oc_free() or ssc_oc_freeNoDestroy() to release collection
/// resources before destroying the SSC!
/// Also, when destroying an operation that has been added to the collection, be
/// sure to remove it first.
/// The collection does not need to be explicitly emptied before destruction
/// (see comments for ssc_oc_free() and ssc_oc_freeNoDestroy()).
///

#include "ssc_sip.h"
#include "ssc_oper.h"

#include <sofia-sip/sip.h>

/// some simple return codes
#define OC_SUCCESS   0
#define OC_FAILURE  -1

/// destruct the collection associated with the indicated SSC context and
/// destroy the collection's contents.
///
/// if the SSC does not carry an op collection then nothing is done.
/// if the collection contains any operations then these will be destroyed.
/// if the SSC has op destroy callbacks defined then these will be called
/// before destroying each operation.
///
/// @param[in]  ssc    ptr to SSC context to use
void ssc_oc_free(ssc_t *ssc);

/// destruct the collection associated with the indicated SSC context
/// without destroying any of its contents.
///
/// if the SSC does not carry an op collection then nothing is done.
/// if the collection contains any operations then they are dumped but not
/// destroyed (caller must have some other way of tracking them so that
/// the caller can destroy them later when the time is right).
///
/// @param[in]  ssc    ptr to SSC context to use
void ssc_oc_free_no_destroy(ssc_t *ssc);

/// returns the total number of operations currently stored in the collection
///
/// @param[in]  ssc    ptr to SSC context to use
/// 
/// @return number of operations
unsigned ssc_oc_size(const ssc_t *ssc);

/// returns the number of operations currently stored in the collection that
/// match the indicated SIP method.
///
/// @param[in]  ssc      ptr to SSC context to use
/// @param[in]  method   SIP method type to look for.  sip_method_invalid
///                      is treated as 'match all' so it effectively becomes
///                      the same as calling ssc_oc_size().
///
/// @return number of matching operations
unsigned ssc_op_size_by_method(const ssc_t *ssc, sip_method_t method);

/// insert an operation into the collection.
///
/// if the collection has not been created yet (first time add) then it will
/// be created as part of the add action.
///
/// the to-uri and from-uri will be extracted from the remote and local URIs
/// in the NUA handle within the SSC operation. Thes will be used as keys
/// for the operation mapping.
///
/// the operation object must remain valid as long as it is stored in the
/// collection.
///
/// @param[in]  ssc    ptr to the SSC context to use
/// @param[in]  op     ptr to the SSC operation to add
///
/// @return OC_SUCCESS on successful add, OC_FAILURE if an error occurred.
int ssc_oc_add_op(ssc_t *ssc, ssc_oper_t *op);

/// insert an operation into the collection.
///
/// if the collection has not been created yet (first time add) then it will
/// be created as part of the add action.
///
/// the provided to-uri and from-uri will be used as keys for the operation
/// mapping. the uri strings are deep-copied so the caller is free to destroy
/// them after the call.
///
/// the operation object is not deep copied and must remain valid as long
/// as it is stored in the collection.
///
/// @param[in]  ssc    ptr to the SSC context to use
/// @param[in]  op     ptr to the SSC operation to add
/// @param[in]  to     sofia struct containing the parsed to-uri
/// @param[in]  from   sofia struct containing the parsed from-uri
///
/// @return OC_SUCCESS on successful add, OC_FAILURE if an error occurred.
int ssc_oc_add_op_uri(ssc_t *ssc, ssc_oper_t *op, const sip_to_t *to, const sip_to_t *from);

/// insert an operation into the collection.
///
/// if the collection has not been created yet (first time add) then it will
/// be created as part of the add action.
///
/// the provided to-uri and from-uri will be used as keys for the operation
/// mapping. the uri strings are deep-copied so the caller is free to destroy
/// them after the call.
///
/// the operation object is not deep copied and must remain valid as long
/// as it is stored in the collection.
///
/// @param[in]  ssc    ptr to the SSC context to use
/// @param[in]  op     ptr to the SSC operation to add
/// @param[in]  to     c-string containing to-uri
/// @param[in]  from   c-string containing from-uri
///
/// @return OC_SUCCESS on successful add, OC_FAILURE if an error occurred.
int ssc_oc_add_op_uri_str(ssc_t *ssc, ssc_oper_t *op, const char *to, const char *from);

/// remove an operation from the collection.
/// if the collection does not exist then no action is taken.
///
/// @param[in]  ssc    ptr to the SSC context to use
/// @param[in]  op     ptr to the SSC operation to be removed
void ssc_oc_rem_op(const ssc_t *ssc, ssc_oper_t *op);

/// search the collection for a matching SSC operation.
/// (this function is equivalent to calling ssc_oc_find_uri_method() with
/// sip_method_invalid.)
///
/// the operation is matched by case sensitive comparison on the to-URI and
/// From-URI being used for the SIP request. Only the username and host
/// portions of the URI are used; any other parts (eg. tags) are ignored.
/// NULL ptrs and empty strings will be treated as matching each other.
///
/// @param[in]  ssc    ptr to the SSC context to use
/// @param[in]  to     sofia struct containing the to-uri to be matched
/// @param[in]  from   sofia struct containing the from-uri to be matched
///
/// @return pointer to the operation found or NULL if not matched.
ssc_oper_t *ssc_oc_find_uri(
      const ssc_t *ssc,
      const sip_to_t *to,
      const sip_to_t *from);

/// search the collection for a matching SSC operation.
/// (this function is equivalent to calling ssc_oc_find_uri_str_method() with
///  sip_method_invalid.)
///
/// the operation is matched by case sensitive comparison on the To-URI and
/// From-URI being used for the SIP request. Only the username and host
/// portions of the URI are used; any other parts (eg. tags) are parsed
/// out and ignored.
/// NULL ptrs and empty strings will be treated as matching each other.
///
/// @param[in]  ssc    ptr to the SSC context to use
/// @param[in]  to     c-string containing the URI to match on the To: header field
/// @param[in]  from   c-string containing the URI to match on the From: header field
///
/// @return pointer to the operation found or NULL if not matched.
ssc_oper_t *ssc_oc_find_uri_str(
      const ssc_t *ssc,
      const char *to,
      const char *from);

/// search the collection for a matching SSC operation.
///
/// the operation is matched by case sensitive comparison on the to-URI and
/// From-URI being used for the SIP request. Only the username and host
/// portions of the URI are used; any other parts (eg. tags) are ignored.
/// NULL ptrs and empty strings will be treated as matching each other.
///
/// the operation must also match the SIP method indicated. if sip_method_invalid
/// is specified, then method becomes a wildcard (action becomes the same
/// as ssc_oc_find_uri()).
///
/// if sip_method_unknown is provided, then only operations with unknown methods
/// are considered.
///
/// @param[in]  ssc      ptr to the SSC context to use
/// @param[in]  to       sofia struct containing the URI to match on the To: header field
/// @param[in]  from     sofia struct containing the URI to match on the From: header field
/// @param[in]  method   SIP request method to match.
///                      sip_method_invalid == match any,
///                      sip_method_unknown == match unknowns only
///
/// @return pointer to the operation found or NULL if not matched.
ssc_oper_t *ssc_oc_find_uri_method(
      const ssc_t *ssc,
      const sip_to_t *to,
      const sip_to_t *from,
      sip_method_t method);

/// search the collection for a matching SSC operation.
///
/// the operation is matched by case sensitive comparison on the To-URI and
/// From-URI being used for the SIP request. Only the username and host
/// portions of the URI are used; any other parts (eg. tags) are parsed
/// out and ignored.
/// NULL ptrs and empty strings will be treated as matching each other.
///
/// the operation must also match the SIP method indicated. if sip_method_invalid
/// is specified, then method becomes a wildcard (action becomes the same
/// as ssc_oc_find_uri()).
///
/// if sip_method_unknown is provided, then only operations with unknown methods
/// are considered.
///
/// @param[in]  ssc      ptr to the SSC context to use
/// @param[in]  to       c-string containing the URI to match on the To: header field
/// @param[in]  from     c-string containing the URI to match on the From: header field
/// @param[in]  method   SIP request method to match.
///                      sip_method_invalid == match any,
///                      sip_method_unknown == match unknowns only
///
/// @return pointer to the operation found or NULL if not matched.
ssc_oper_t *ssc_oc_find_uri_str_method(
      const ssc_t *ssc,
      const char *to,
      const char *from,
      sip_method_t method);

/// search the collection for a matching SSC operation.
///
/// the operation is matched by case sensitive comparison on the to-URI and
/// From-URI being used for the SIP request. Only the username and host
/// portions of the URI are used; any other parts (eg. tags) are ignored.
/// NULL ptrs and empty strings will be treated as matching each other.
///
/// only operations with unknown method type will be checked.  A case sensitive
/// match on method name will be done.  NULL ptr for name will match any unknown
/// method (so it becomes the same action as
///    ssc_oc_find_uri_method(ssc, to, from, sip_method_unknown)).
///
/// @param[in]  ssc      ptr to the SSC context to use
/// @param[in]  to       sofia struct containing the URI to match on the To: header field
/// @param[in]  from     sofia struct containing the URI to match on the From: header field
/// @param[in]  name     c-string containing the method name to be matched. (NULL == match any.)
///
/// @return pointer to the operation found or NULL if not matched.
ssc_oper_t *ssc_oc_find_uri_unknown_method(
      const ssc_t *ssc,
      const sip_to_t *to,
      const sip_to_t *from,
      const char *name);

/// search the collection for a matching SSC operation.
///
/// the operation is matched by case sensitive comparison on the To-URI and
/// From-URI being used for the SIP request. Only the username and host
/// portions of the URI are used; any other parts (eg. tags) are parsed
/// out and ignored.
/// NULL ptrs and empty strings will be treated as matching each other.
///
/// only operations with unknown method type will be checked.  A case sensitive
/// match on method name will be done.  NULL ptr for name will match any unknown
/// method (so it becomes the same action as
///    ssc_oc_find_uri_str_method(ssc, to, from, sip_method_unknown)).
///
/// @param[in]  ssc      ptr to the SSC context to use
/// @param[in]  to       c-string containing the URI to match on the To: header field
/// @param[in]  from     c-string containing the URI to match on the From: header field
/// @param[in]  name     c-string containing the method name to be matched. (NULL == match any.)
///
/// @return pointer to the operation found or NULL if not matched.
ssc_oper_t *ssc_oc_find_uri_str_unknown_method(
      const ssc_t *ssc,
      const char *to,
      const char *from,
      const char *name);

