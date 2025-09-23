/**
 * Auto generated, do not edit.
 * 
 * \brief 
 * The version implementation of module TB2820V07_Api.
 * 
 * \details 
 * The version implementation of module TB2820V07_Api.
 * 
 * \file: TB2820V07_ApiVersion.c
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

#include "TB2820V07_ApiVersion.h"

static const TB_Version _TB2820V07_ApiVersion = 
{
    3, 1, 0
};

static const TB_Compatibility _TB2820V07_ApiCompatibility[1] = 
{
    {TB_DEV_TYPE_CPCIE,2,1,0,7,0,0},
};

const TB_Version *TB2820V07_Api_GetVersion()
{
    return &_TB2820V07_ApiVersion;
}

const TB_Compatibility *TB2820V07_Api_GetCompatibility(TB_U8 *num)
{
    if (num == TB_NULL_PTR)
    {
        return (const TB_Compatibility *)TB_NULL_PTR;
    }

    *num = 1;
    return &_TB2820V07_ApiCompatibility[0];
}

