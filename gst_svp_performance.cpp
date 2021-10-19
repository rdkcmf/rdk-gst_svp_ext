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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include <string>
#include <map>

extern "C" {
#include <sec_security_datatype.h>
#include <sec_security.h>
}

#include "gst_svp_logging.h"
#include "GstPerf.h"

GstPerf::GstPerf(const char* szName) 
: m_deltaTime(0)
{
    m_strName = szName;
    m_startTime = TimeStamp();
}
GstPerf::~GstPerf()
{
    m_deltaTime = TimeStamp() - m_startTime;
    GstPerf_ReportTime(m_strName, m_deltaTime);
}

uint64_t GstPerf::TimeStamp() 
{
    struct timeval  timeStamp;
    uint64_t        retVal = 0;

    gettimeofday(&timeStamp, NULL);

    // Convert timestamp to Micro Seconds
    retVal = (uint64_t)(((uint64_t)timeStamp.tv_sec * 1000000) + timeStamp.tv_usec);

    return retVal;
}

typedef struct _TimingStats
{
    std::string strName;
    uint64_t nTotalTime;
    double   nTotalAvg;
    uint64_t nTotalMax;
    uint64_t nTotalMin;
    uint64_t nTotalCount;
    uint64_t nIntervalTime;
    double   nIntervalAvg;
    uint64_t nIntervalMax;
    uint64_t nIntervalMin;
    uint64_t nIntervalCount;
} TimingStats;

static std::map<std::string, TimingStats> s_timingMap;
#define MAX_INTERVAL_COUNT 1000

static void GstPerf_IncrementData(TimingStats& stats, uint64_t deltaTime)
{
    // Increment totals
    stats.nTotalTime += deltaTime;
    stats.nTotalCount++;
    if(stats.nTotalMin > deltaTime) {
        stats.nTotalMin = deltaTime;
    }
    if(stats.nTotalMax < deltaTime) {
        stats.nTotalMax = deltaTime;
    }
    stats.nTotalAvg = (double)stats.nTotalTime / (double)stats.nTotalCount;

    // Increment intervals
    stats.nIntervalTime += deltaTime;
    stats.nIntervalCount++;
    if(stats.nIntervalMin > deltaTime) {
        stats.nIntervalMin = deltaTime;
    }
    if(stats.nIntervalMax < deltaTime) {
        stats.nIntervalMax = deltaTime;
    }
    stats.nIntervalAvg = (double)stats.nIntervalTime / (double)stats.nIntervalCount;

    return;
}

static void GstPerf_LogData(TimingStats& stats)
{
    LOG(eWarning, "SVP Performance Timing (in ms): %s (Count, Max, Min, Avg) Total %lld, %0.3lf, %0.3lf, %0.3lf Interval %lld, %0.3lf, %0.3lf, %0.3lf\n",
            stats.strName.c_str(),
            stats.nTotalCount, ((double)stats.nTotalMax) / 1000.0, ((double)stats.nTotalMin) / 1000.0, stats.nTotalAvg / 1000.0,
            stats.nIntervalCount, ((double)stats.nIntervalMax) / 1000.0, ((double)stats.nIntervalMin) / 1000.0, stats.nIntervalAvg / 1000.0);
}

static void GstPerf_ResetInterval(TimingStats& stats)
{
    stats.nIntervalTime     = 0;
    stats.nIntervalAvg      = 0;
    stats.nIntervalMax      = 0;
    stats.nIntervalMin      = 0;
    stats.nIntervalCount    = 0;

    return;
}

void GstPerf_ReportTime(std::string strName, uint64_t deltaTime)
{
    std::map<std::string, TimingStats>::iterator it;
    // Find and existing key
    it = s_timingMap.find(strName);
    if (it != s_timingMap.end()) {
        // Found Key
        GstPerf_IncrementData(it->second, deltaTime);

        if(it->second.nIntervalCount >= MAX_INTERVAL_COUNT) {
            GstPerf_LogData(it->second);
            GstPerf_ResetInterval(it->second);
        }
    }
    else {
        // New element
        LOG(eWarning, "Creating new stats element for %s\n", strName.c_str());
        TimingStats stats;
        memset(&stats, 0, sizeof(TimingStats));
        stats.nTotalMin     = 1000000;      // Preset Min values to pickup the inital value
        stats.nIntervalMin  = 1000000;
        stats.strName = strName;
        GstPerf_IncrementData(stats, deltaTime);
        s_timingMap[strName] = stats;
    }

    return;
}

#if 0
static bool load_library(const char * szLibraryName, void** ppHandle)
{
    dlerror();  // clear error

    void * library_handle = dlopen(szLibraryName, RTLD_LAZY);
    if(library_handle == NULL) {
        char* error = dlerror();
        if (error != NULL) {
            LOG(eError, "Could not open library <%s> error = %s\n", szLibraryName, error);
        }
        LOG(eError, "ERROR: could not open library %s\n", szLibraryName);
        *ppHandle = NULL;

        return false;
    }

    // Success
    *ppHandle = library_handle;
    return true;
}

static bool link_function(void* pLibrary, void** ppFunc, const char* szFuncName)
{
    *ppFunc = dlsym(pLibrary, szFuncName);
    if (!*ppFunc) {
        /* no such symbol */
        LOG(eError, "Error for %s : %s\n", szFuncName, dlerror());
        return false;
    }
    return true;
}
#endif

Sec_Result GstPerf_SecOpaqueBuffer_Malloc(SEC_SIZE bufLength, Sec_OpaqueBufferHandle **handle)
{
   GstPerf perf("SecOpaqueBuffer_Malloc");
    return SecOpaqueBuffer_Malloc(bufLength, handle);
}

Sec_Result GstPerf_SecOpaqueBuffer_Write(Sec_OpaqueBufferHandle *handle, SEC_SIZE offset, SEC_BYTE *data, SEC_SIZE length)
{
    GstPerf perf("SecOpaqueBuffer_Write");
    return SecOpaqueBuffer_Write(handle, offset, data, length);
}

Sec_Result GstPerf_SecOpaqueBuffer_Free(Sec_OpaqueBufferHandle *handle)
{
    GstPerf perf("SecOpaqueBuffer_Free");
    return SecOpaqueBuffer_Free(handle);
}

Sec_Result GstPerf_SecOpaqueBuffer_Release(Sec_OpaqueBufferHandle *handle, Sec_ProtectedMemHandle **svpHandle)
{
    GstPerf perf("SecOpaqueBuffer_Release");
    return SecOpaqueBuffer_Release(handle, svpHandle);
}

Sec_Result GstPerf_SecOpaqueBuffer_Copy(Sec_OpaqueBufferHandle *out, SEC_SIZE out_offset, Sec_OpaqueBufferHandle *in, SEC_SIZE in_offset, SEC_SIZE num_to_copy)
{
    GstPerf perf("SecOpaqueBuffer_Copy");
    return SecOpaqueBuffer_Copy(out, out_offset, in, in_offset, num_to_copy);
}

#ifdef ENABLE_OCDM_PROFILING
#error ENABLE_OCDM_PROFILING
// Forward declarations
typedef uint32_t OpenCDMError;
struct OpenCDMSession;
#ifndef EXTERNAL
    #ifdef _MSVC_LANG
        #ifdef OCDM_EXPORTS
        #define EXTERNAL __declspec(dllexport)
        #else
        #define EXTERNAL __declspec(dllimport)
        #endif
    #else
        #define EXTERNAL __attribute__ ((visibility ("default")))
    #endif
#endif

EXTERNAL OpenCDMError opencdm_session_decrypt(struct OpenCDMSession* session,
                                    uint8_t encrypted[],
                                    const uint32_t encryptedLength,
                                    const uint8_t* IV, uint16_t IVLength,
                                    const uint8_t* keyId, const uint16_t keyIdLength,
                                    uint32_t initWithLast15,
                                    uint8_t* streamInfo,
                                    uint16_t streamInfoLength);

uint32_t (*lcl_opencdm_session_decrypt)(struct OpenCDMSession* session,
                                    uint8_t encrypted[],
                                    const uint32_t encryptedLength,
                                    const uint8_t* IV, uint16_t IVLength,
                                    const uint8_t* keyId, const uint16_t keyIdLength,
                                    uint32_t initWithLast15,
                                    uint8_t* streamInfo,
                                    uint16_t streamInfoLength);

OpenCDMError GstPerf_opencdm_session_decrypt(struct OpenCDMSession* session,
                                                    uint8_t encrypted[],
                                                    const uint32_t encryptedLength,
                                                    const uint8_t* IV, uint16_t IVLength,
                                                    const uint8_t* keyId, const uint16_t keyIdLength,
                                                    uint32_t initWithLast15,
                                                    uint8_t* streamInfo,
                                                    uint16_t streamInfoLength)
{
    static void *library_handle = NULL;
    static bool bInitLibrary = false;
    static const char* szLibraryName = "libocdm.so";
    
    if(bInitLibrary == false) {
        if(library_handle == NULL) {
            load_library(szLibraryName, &library_handle);
            LOG(eWarning, "library_handle = %p\n", library_handle);
        }
        if(library_handle) {
            // Library loaded, link symbol
            if(link_function(library_handle, (void**)&lcl_opencdm_session_decrypt, "opencdm_session_decrypt")) {
                // Success
                bInitLibrary = true;
            }
        }
        else {
            LOG(eError, "Invalid Library handle for %s\n", szLibraryName);
        }
    }

    GstPerf perf("opencdm_session_decrypt");

    if(lcl_opencdm_session_decrypt) {
        return lcl_opencdm_session_decrypt(session, encrypted, encryptedLength, IV, IVLength, 
                                    keyId, keyIdLength, initWithLast15, 
                                    streamInfo, streamInfoLength);
    }
    else {
        LOG(eError, "Dynamic function not linked\n");
        return 0x80004005;      // ERROR_FAIL
    }
}
#endif // ENABLE_OCDM_PROFILING