
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
#ifndef ssc_types_h
#define ssc_types_h

#define MAX_SSC_ACCEPT_CONTACT 256
#define MAX_SSC_SESSION_EXPIRES 256
#define MAX_SSC_REQUIRES 256
#define MAX_SSC_CONTACT_URI_FEATURES 256
#define MAX_SSC_CONTACT_FEATURES 256
#define MAX_SSC_URI 256
#define MAX_CONTENT_TYPE_SIZE 64

#define MAX_SDP_SIZE 4096
#define MAX_SSC_PAYLOAD_STR_SIZE (MAX_SDP_SIZE + 256)

// 157kb: includes 153kb for receiver list participant xml items.
#define MAX_RS_METADATA_SIZE 160768

#define MAX_BOUNDARY_SIZE 128
#define MAX_CONTENT_PARTS 4
#define MAX_HEADER_AREA_SIZE 128
#define MAX_CONTENT_AREA_SIZE 128

#define SIP_CALL_TYPE_NONE 0
#define SIP_CALL_TYPE_GROUP 1
#define SIP_CALL_TYPE_ADHOC 2


#ifndef SIP_HEADER_FORMAT
#define SIP_HEADER_FORMAT 1
typedef enum _SipHeaderFormat
{
   SIP_HEADER_FORMAT_NONE = 0x00,
   SIP_HEADER_FORMAT_ESCHAT_CALL = 0x01,
   SIP_HEADER_FORMAT_X_ESCHAT_CALL = 0x02,
} SipHeaderFormat;
#endif

// ISSI spececific attributes
typedef struct _ssc_config_issi_t
{
   // <0 -> do not include,
   //  0 -> include w/ value == 0
   // >0 -> include w/ value == 1
   short c_resavail;
   short c_groupcalltype;
   short c_protected;
} ssc_config_issi_t;


typedef struct _ssc_config_t
{
	/*
	 * for all string fields, 0 length string == 'do not include'
	 * OR 'let stack choose the value' if the corresponding
	 * field/parameter is not-optional.
	 *
	 * for object pointer fields, NULL == 'do not include' OR 'let
	 * stack specify'.
	 */

	/* SIP transport; URI of dest host */
	char targetAddress[MAX_SSC_URI];

	/* SIP header fields */
	char requestUri[MAX_SSC_URI];
	char toUri[MAX_SSC_URI];
	char fromUri[MAX_SSC_URI];
	char callId[MAX_SSC_URI];
	char accept[MAX_SSC_URI];
	char allow[MAX_SSC_URI];
	char via[MAX_SSC_URI];
	char expires[MAX_SSC_SESSION_EXPIRES];
	char contactUri[MAX_SSC_URI];
   char contactUriFeatures[MAX_SSC_CONTACT_URI_FEATURES];
   char contactFeatures[MAX_SSC_CONTACT_FEATURES];
	char acceptContact[MAX_SSC_ACCEPT_CONTACT];
	char require[MAX_SSC_REQUIRES];
   char route[MAX_SSC_URI];

   char priority[MAX_SSC_URI];

   char talkerLocation[MAX_SSC_URI];

   char customerName[MAX_SSC_URI];
   char customerId[MAX_SSC_URI];
   char departmentId[MAX_SSC_URI];

   // when TRUE, use valus of contactUriFeatures and contactFeatures fields
   // to construct final contact field value for use in the message. when
   // FALSE, use the value of contactUri without making any changes to it.
   unsigned short postProcessContactUri;

   // special value used for generating proprietary
   // 'ESChat-Call:' header.
   unsigned char callType;
   // Use X-ESChat-Call instead of ESChat-Call
   unsigned char headerFormat;

   // if present, ptr to struct defining 'Content-Type:' value to
   // include.
   sip_content_type_t *contentType;
	char contentTypeStr[MAX_CONTENT_TYPE_SIZE];

	/* message body info: */

	char contentLength[MAX_SSC_URI];

   // used if payload is null
   char sdp[MAX_SDP_SIZE];  

   // if payload is non-null, use it insted of SDP
   sip_payload_t *payload;
   
   // ..or use payload specified as a string
   char payloadStr[MAX_SSC_PAYLOAD_STR_SIZE];

	// used in message content creation
   char rsMetadata[MAX_RS_METADATA_SIZE];  

   // If not NULL then this is holding memory...
   msg_t *mimeMsgHome;

   // when TRUE, indicates that we need to structure the payload for our
   // INVITEs using P25 defined multipart MIME layout.
   // (default: FALSE)
   unsigned char p25InviteUseMultipart;

   // issi attributes  (used if p25 multipart is indicated)
   ssc_config_issi_t issi;

	// TODO: document these
   const char *groupName;
   const char *paidDisplay;
   char *paidSipUri;
	uint8_t isFocus;
	uint8_t sessionType;

   /* multipart content for register */

   // boundry string to use for multipart separator
   char contentBoundary[MAX_BOUNDARY_SIZE];

   // the total number of multipart sections to include
   uint8_t numContentParts;

   // header blobs for each multipart section (may be empty string "", and
   // a blank line will be placed betwen header area and content area so
   // there is no need to end the header blob with a blank line.
   char headerParts[MAX_CONTENT_PARTS][MAX_HEADER_AREA_SIZE];

   // content blobs for each multipart section (may be empty string "", and
   // a blank line will automatically be placed between header area and
   // content area so there is no need to start the content blob with a
   // blank line.
   char contentParts[MAX_CONTENT_PARTS][MAX_CONTENT_AREA_SIZE];
} ssc_config_t;

// call this function to initialize ssc_config_t struct contents
void setSscDefaults( ssc_config_t *);

// Free stuff as needed.
void cleanupSsc( ssc_config_t *);

#endif // ssc_types_h
