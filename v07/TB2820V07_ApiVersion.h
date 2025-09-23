/**
 * Auto generated, do not edit.
 * 
 * \brief 
 * The version define of module TB2820V07_Api.
 * 
 * \details 
 * The version define of module TB2820V07_Api.
 * 
 * \file: TB2820V07_ApiVersion.h
 * 
 * \author -
 *         -
 * 
 * \version
 * __Revision History__
 * Version | Date | Author | Comments
 *  :--------:|:--------:|:--------:|:--------------
 * 1.0.0 | - | - | N/A
 * 
 * 
 * \copyright
 * This software is the property of HiRain Technologies.Any information
 * contained in this doc should not be reproduced, or used, or disclosed
 * without the written authorization from HiRain Technologies.
 */

#ifndef _TB2820V07_API_VERSION_H_
#define _TB2820V07_API_VERSION_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "TESTBASE.h"

#define TB2820V07_API_VERSION_MAJOR      3
#define TB2820V07_API_VERSION_MINOR      1
#define TB2820V07_API_VERSION_REVISION   0

const TB_Version * TB2820V07_Api_GetVersion();
const TB_Compatibility * TB2820V07_Api_GetCompatibility(TB_U8 *num);

#ifdef __cplusplus
}
#endif

#endif
