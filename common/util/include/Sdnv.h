//Implementation of https://tools.ietf.org/html/rfc6256
// Using Self-Delimiting Numeric Values in Protocols
#ifndef _SDNV_UTIL_H
#define _SDNV_UTIL_H 1




#include <stdlib.h>
#include <stdint.h>
#include "hdtn_util_export.h"

//must be at least sizeof(_m128i)
#define SDNV_DECODE_MINIMUM_SAFE_BUFFER_SIZE 16

//return output size
HDTN_UTIL_EXPORT unsigned int SdnvGetNumBytesRequiredToEncode(const uint64_t valToEncodeU64);

//return output size
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU32(uint8_t * outputEncoded, const uint32_t valToEncodeU32, const uint64_t bufferSize);
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU32BufSize8(uint8_t * outputEncoded, const uint32_t valToEncodeU32);

//return output size
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU64(uint8_t * outputEncoded, const uint64_t valToEncodeU64, const uint64_t bufferSize);
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU64BufSize10(uint8_t * outputEncoded, const uint64_t valToEncodeU64);

//return decoded value (return invalid number that must be ignored on failure)
//  also sets parameter numBytes taken to decode (set to 0 on failure)
HDTN_UTIL_EXPORT uint32_t SdnvDecodeU32(const uint8_t * inputEncoded, uint8_t * numBytes, const uint64_t bufferSize);

//return decoded value (return invalid number that must be ignored on failure)
//  also sets parameter numBytes taken to decode (set to 0 on failure)
HDTN_UTIL_EXPORT uint64_t SdnvDecodeU64(const uint8_t * inputEncoded, uint8_t * numBytes, const uint64_t bufferSize);


//return output size
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU32Classic(uint8_t * outputEncoded, const uint32_t valToEncodeU32, const uint64_t bufferSize);
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU32ClassicBufSize5(uint8_t * outputEncoded, const uint32_t valToEncodeU32);

//return output size
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU64Classic(uint8_t * outputEncoded, const uint64_t valToEncodeU64, const uint64_t bufferSize);
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU64ClassicBufSize10(uint8_t * outputEncoded, const uint64_t valToEncodeU64);

//return decoded value (return invalid number that must be ignored on failure)
//  also sets parameter numBytes taken to decode (set to 0 on failure)
HDTN_UTIL_EXPORT uint32_t SdnvDecodeU32Classic(const uint8_t * inputEncoded, uint8_t * numBytes, const uint64_t bufferSize);
	
//return decoded value (return invalid number that must be ignored on failure)
//  also sets parameter numBytes taken to decode (set to 0 on failure)
HDTN_UTIL_EXPORT uint64_t SdnvDecodeU64Classic(const uint8_t * inputEncoded, uint8_t * numBytes, const uint64_t bufferSize);
	
#ifdef USE_SDNV_FAST
//return output size
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU32FastBufSize8(uint8_t * outputEncoded, const uint32_t valToEncodeU32);

//return output size
HDTN_UTIL_EXPORT unsigned int SdnvEncodeU64FastBufSize10(uint8_t * outputEncoded, const uint64_t valToEncodeU64);

//return decoded value (return invalid number that must be ignored on failure)
//  also sets parameter numBytes taken to decode (set to 0 on failure)
HDTN_UTIL_EXPORT uint32_t SdnvDecodeU32FastBufSize8(const uint8_t * data, uint8_t * numBytes);

//return decoded value (return invalid number that must be ignored on failure)
//  also sets parameter numBytes taken to decode (set to 0 on failure)
HDTN_UTIL_EXPORT uint64_t SdnvDecodeU64FastBufSize16(const uint8_t * data, uint8_t * numBytes);

//return num values decoded this iteration
HDTN_UTIL_EXPORT unsigned int SdnvDecodeMultipleU64Fast(const uint8_t * data, uint8_t * numBytes, uint64_t * decodedValues, unsigned int decodedRemaining);


# ifdef SDNV_SUPPORT_AVX2_FUNCTIONS //must also support USE_SDNV_FAST
//return num values decoded this iteration
HDTN_UTIL_EXPORT unsigned int SdnvDecodeMultiple256BitU64Fast(const uint8_t * data, uint8_t * numBytes, uint64_t * decodedValues, unsigned int decodedRemaining);
# endif //#ifdef SDNV_SUPPORT_AVX2_FUNCTIONS
#endif //#ifdef USE_SDNV_FAST


#endif      // _SDNV_UTIL_H 
