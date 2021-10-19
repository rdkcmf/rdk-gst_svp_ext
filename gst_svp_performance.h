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

#ifndef __GST_SVP_PERFORMANCE_H__
#define __GST_SVP_PERFORMANCE_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "GstPerf.h"

extern "C" {
#include <sec_security_datatype.h>
#include <sec_security.h>
}


// Feature Control
#define GST_SVP_PERF 1

#if !defined(__OCDM_WRAPPER_H_)
    // Only define for OCDM component, 
    //#define ENABLE_OCDM_PROFILING 1
#endif

#ifdef GST_SVP_PERF
// Sec API Opaque Buffers
#define SecOpaqueBuffer_Malloc(a, b) GstPerf_SecOpaqueBuffer_Malloc(a, b)
#define SecOpaqueBuffer_Write(a, b, c, d) GstPerf_SecOpaqueBuffer_Write(a, b, c, d)
#define SecOpaqueBuffer_Free(a) GstPerf_SecOpaqueBuffer_Free(a)
#define SecOpaqueBuffer_Release(a, b) GstPerf_SecOpaqueBuffer_Release(a, b)
#define SecOpaqueBuffer_Copy(a, b, c, d, e) GstPerf_SecOpaqueBuffer_Copy(a, b, c, d, e)

#ifdef ENABLE_OCDM_PROFILING
// OCDM
#define opencdm_session_decrypt(a, b, c, d, e, f, g, h, i, j) GstPerf_opencdm_session_decrypt(a, b, c, d, e, f, g, h, i, j)
#endif // ENABLE_OCDM_PROFILING
// Netflix
#endif // GST_SVP_PERF

// SecAPI Opaque Buffer Functions
inline Sec_Result GstPerf_SecOpaqueBuffer_Malloc(SEC_SIZE bufLength, Sec_OpaqueBufferHandle **handle);
inline Sec_Result GstPerf_SecOpaqueBuffer_Write(Sec_OpaqueBufferHandle *handle, SEC_SIZE offset, SEC_BYTE *data, SEC_SIZE length);
inline Sec_Result GstPerf_SecOpaqueBuffer_Free(Sec_OpaqueBufferHandle *handle);
inline Sec_Result GstPerf_SecOpaqueBuffer_Release(Sec_OpaqueBufferHandle *handle, Sec_ProtectedMemHandle **svpHandle);
inline Sec_Result GstPerf_SecOpaqueBuffer_Copy(Sec_OpaqueBufferHandle *out, SEC_SIZE out_offset, Sec_OpaqueBufferHandle *in, SEC_SIZE in_offset, SEC_SIZE num_to_copy);

#ifdef ENABLE_OCDM_PROFILING
// OCDM Decrypt
#if !defined(__OCDM_WRAPPER_H_) // Is open_cdm.h in the include path already
// Forward declarations
typedef uint32_t OpenCDMError;
struct OpenCDMSession;
#endif

OpenCDMError GstPerf_opencdm_session_decrypt(struct OpenCDMSession* session,
                                                    uint8_t encrypted[],
                                                    const uint32_t encryptedLength,
                                                    const uint8_t* IV, uint16_t IVLength,
                                                    const uint8_t* keyId, const uint16_t keyIdLength,
                                                    uint32_t initWithLast15,
                                                    uint8_t* streamInfo,
                                                    uint16_t streamInfoLength);
#endif //ENABLE_OCDM_PROFILING

#endif // __GST_SVP_PERFORMANCE_H__