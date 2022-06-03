
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
#ifndef SSC_LOG_H
#define SSC_LOG_H


#if 0
#include <syslog.h>
#define SSCLogVarArgBase(fmt, ...) vsyslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define SSCLogMacroBase(level, zzz, ...)                                        \
   syslog(level, "%s:%u:: " zzz, __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#if 0
#define SSCLogVarArgBase(fmt, ...) vprintf(fmt, ##__VA_ARGS__)
#define SSCLogMacroBase(level, zzz, ...)                                        \
   fprintf(stdout, "[" level "] " "%s:%u:: " zzz "\n", __FILE__, __LINE__, ##__VA_ARGS__), fflush(stdout)
#endif

#include <stdio.h>
extern FILE *sscGetLogFile();
extern void sscSetLogFile(FILE *logFile);
extern void sscLogTimestamp(FILE *logFile);

extern const char *DEFAULT_LOG_FILE_PATH;
#define MAX_LOG_PATH_LEN 256

#define SSCLogVarArgBase(fmt, ...)                                              \
   do                                                                          \
   {                                                                           \
      FILE *SSCLogVarArgBase_logFile = sscGetLogFile();                        \
      sscLogTimestamp(SSCLogVarArgBase_logFile);                                   \
      vfprintf(SSCLogVarArgBase_logFile, fmt, ##__VA_ARGS__);                   \
      fflush(SSCLogVarArgBase_logFile);                                         \
   } while (0)

#define SSCLogMacroBase(level, zzz, ...)                                        \
   do                                                                          \
   {                                                                           \
      FILE *SSCLogMacroBase_logFile = sscGetLogFile();                         \
      sscLogTimestamp(SSCLogMacroBase_logFile);                                    \
      fprintf(                                                                 \
            SSCLogMacroBase_logFile,                                            \
            "[" level "] "                                                     \
            "%s:%u:: " zzz "\n",                                               \
            __FILE__,                                                          \
            __LINE__,                                                          \
            ##__VA_ARGS__);                                                    \
      fflush(SSCLogMacroBase_logFile);                                          \
   } while (0)

#define SSCDebugHigh(zzz, ...) SSCLogMacroBase("HIGH  ", zzz, ##__VA_ARGS__)
#define SSCDebugMed(zzz, ...) SSCLogMacroBase("MED   ", zzz, ##__VA_ARGS__)
#define SSCDebugLow(zzz, ...) SSCLogMacroBase("LOW   ", zzz, ##__VA_ARGS__)
#define SSCWarning(zzz, ...) SSCLogMacroBase("WARN  ", zzz, ##__VA_ARGS__)
#define SSCError(zzz, ...) SSCLogMacroBase("ERROR ", zzz, ##__VA_ARGS__)

#endif // SSC_LOG_H
