
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdint.h>

#include <sofia-sip/sip_extra.h>
#include <sofia-sip/soa_tag.h>

#include "ssc_sip.h"
#include "ssc_oper.h"
#include "ssc_oper_container.h"

#include "ssc_types.h"

// set defaults for ssc_config_t
void setSscDefaults(ssc_config_t *config)
{
   config->targetAddress[0] = '\0';
   config->requestUri[0] = '\0';
   config->toUri[0] = '\0';
   config->fromUri[0] = '\0';
   config->callId[0] = '\0';
   config->accept[0] = '\0';
   config->allow[0] = '\0';
   config->via[0] = '\0';
   config->expires[0] = '\0';
   config->contactUri[0] = '\0';
   config->contactUriFeatures[0] = '\0';
   config->contactFeatures[0] = '\0';
   config->acceptContact[0] = '\0';
   config->require[0] = '\0';
   config->route[0] = '\0';
   config->priority[0] = '\0';

   config->talkerLocation[0] = '\0';

   config->customerName[0] = '\0';
   config->customerId[0] = '\0';
   config->departmentId[0] = '\0';

   config->postProcessContactUri = 1;

   config->callType = SIP_CALL_TYPE_NONE;
   config->headerFormat = SIP_HEADER_FORMAT_ESCHAT_CALL;

   config->contentLength[0] = '\0';
   config->contentType = NULL;
   config->contentTypeStr[0] = '\0';
   config->sdp[0] = '\0';
   config->payload = NULL;
   config->payloadStr[0] = '\0';
   config->rsMetadata[0] = '\0';

   config->mimeMsgHome = NULL;

   config->p25InviteUseMultipart = 0;
   config->issi.c_resavail = -1;
   config->issi.c_groupcalltype = -1;
   config->issi.c_protected = -1;

   config->groupName = NULL;
   config->paidDisplay = NULL;
   config->paidSipUri = NULL;

   config->isFocus = 0;
   config->sessionType = 0;

   config->numContentParts = 0;
   config->contentBoundary[0] = '\0';
   memset(config->headerParts, 0, MAX_HEADER_AREA_SIZE * MAX_CONTENT_PARTS);
   memset(config->contentParts, 0, MAX_CONTENT_AREA_SIZE * MAX_CONTENT_PARTS);
}

void cleanupSsc(ssc_config_t *config)
{
   if (config->mimeMsgHome)
   {
      msg_destroy(config->mimeMsgHome);
      config->mimeMsgHome = NULL;

      // These are set too by ssc_create_siprec_mime() likely.
      config->payload = NULL;
      config->contentType = NULL;
   }
}

