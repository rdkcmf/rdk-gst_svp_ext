/* Copyright (C) 2019 RDK Management.  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS. OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/
#include "gst_svp_logging.h"

#include <unistd.h> // for getipd()

static bool s_VerboseLog = false;
void GstSvpLogging(eLogLevel level, const char* function, int line, const char * format, ...)
{    
#define LOG_MESSAGE_SIZE 4096
    char logMessage[LOG_MESSAGE_SIZE];

    if(s_VerboseLog == false && level < eWarning) {
        // No logging
        return;
    }

    // Generate the log string
    va_list ap;
    va_start(ap, format);
    vsnprintf(logMessage, LOG_MESSAGE_SIZE, format, ap);
    va_end(ap);

    FILE* fpOut = stdout;
    if(level == eError) {
        fpOut = stderr;
    }

    // printf for now.
    fprintf(fpOut, "Process ID %d : %s(%d) : %s", getpid(), function, line, logMessage);
    // fprintf(fpOut, "%s(%d) : %s", function, line, logMessage);
    fflush(fpOut);
    return;
}

#define BUF_SIZE 8192
void DebugBinaryData(char* szName, uint8_t* pData, size_t nSize)
{
    size_t     idx     = 0;
    uint8_t    buffer[BUF_SIZE];
    uint8_t*   ptr     = buffer;

    size_t bufAvail = (BUF_SIZE/4 - strlen(szName));
    if(nSize > bufAvail) {
        nSize = bufAvail;
        LOG(eTrace, "Size truncated to %d\n", nSize);
    }

    sprintf((char*)ptr, "%s", (char*)szName);
    ptr += strlen(szName);
    while(idx < nSize) {
        if(idx % 16 == 0) sprintf((char*)(ptr++), "\n");
        sprintf((char*)ptr, "%02X ", pData[idx]);
        ptr +=3;
        idx++;
    }
    sprintf((char*)(ptr++), "\n");

    LOG(eTrace, "%s", buffer);
}

static void __attribute__((constructor)) LogModuleInit();
static void __attribute__((destructor)) LogModuleTerminate();

// This function is assigned to execute as a library init
//  using __attribute__((constructor))
static void ModuleInit()
{
    LOG(eWarning, "GST SVP Logging initialize extending logging set to %d\n", s_VerboseLog);
    const char *env_log_level = getenv("GSTSVPEXT_EXTENDED_LOGGING");
    if(strncasecmp(env_log_level, "true", strlen("true")) == 0) {
      s_VerboseLog = true;
      LOG(eWarning, "Enabling GSTSVPEXT extended logging %d", s_VerboseLog);
    }

}
// This function is assigned to execute as library unload
// using __attribute__((destructor))
static void LogModuleTerminate()
{
    LOG(eWarning, "GST SVP Logging terminate\n");
}