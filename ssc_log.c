
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

#include "ssc_log.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

static FILE *globalLogFile = NULL;

// Must be called from main thread
void sscSetLogFile(FILE *logFile)
{
   globalLogFile = logFile;
}

// Must be called from main thread
FILE *sscGetLogFile()
{
   if (globalLogFile == NULL)
   {
      return stdout;
   }
   return globalLogFile;
}

void sscLogTimestamp(FILE *logFile)
{
   struct timeval stamp;
   struct tm hms;
   gettimeofday(&stamp, NULL);
   gmtime_r(&stamp.tv_sec, &hms);
   fprintf(
         logFile,
         "%02d:%02d:%02d.%06ld ",
         hms.tm_hour,
         hms.tm_min,
         hms.tm_sec,
         stamp.tv_usec);
}

