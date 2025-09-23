/**
 * \brief 
 * Add brief description here.
 * 
 * \details 
 * Add detailed description here.
 * 
 * \file: TESTBASE.h
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


#ifndef _TESTBASE_H_
#define _TESTBASE_H_

#include <stdio.h>

/* Should not be used directly */
#define TB_Printf(f, fmt, ...)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        fprintf((f), (fmt), ##__VA_ARGS__);                                                                            \
    } while (0)

#if (TB_DEVICE_INTERFACE_DEBUG || TB_DEVICE_INTERFACE_VIRTUAL)
/* Only should be used in debug mode */
#define TB_DBG(fmt, ...) TB_Printf(stdout, (fmt), ##__VA_ARGS__)
/* Only should be used when a error occurred */
#define TB_ERR(fmt, ...) TB_Printf(stderr, (fmt), ##__VA_ARGS__)
#else
#define TB_DBG(fmt, ...)
#define TB_ERR(fmt, ...)
#endif

#define TB_DEV_PATH_MAX 128U

/**
 *  Primitive Data Type
 */
typedef unsigned char TB_U8;
typedef signed char TB_S8;
typedef unsigned short int TB_U16;
typedef signed short int TB_S16;
typedef unsigned int TB_U32;
typedef signed int TB_S32;
typedef unsigned long long TB_U64;
typedef signed long long TB_S64;
typedef float TB_Float;
typedef double TB_Double;

typedef unsigned char TB_Byte;
typedef unsigned short int TB_Word;
typedef unsigned int TB_DWord;

#define TB_NULL_PTR ((void *)0)

typedef unsigned char TB_Bool;
#define TB_TRUE 1
#define TB_FALSE 0

#define TB_MIN(a, b) (((a) < (b)) ? a : b)
#define TB_MAX(a, b) (((a) > (b)) ? a : b)

/**
 *  Device Data Type
 */
typedef unsigned long long Aurora_Handler;
typedef const void *TB_Handler;
#define TB_NULL_HANDLER ((void *)0)

typedef unsigned int TB_RegVal;
typedef unsigned long long TB_RegAddr;
typedef unsigned int *TB_RegValPtr;

/* Bit operation */
#define TB_BITMASK(val, bitnum) ((TB_U32)(val) << bitnum)
#define TB_BIT_SET(val, mask) (val) |= (TB_U32)(mask)
#define TB_BIT_CLR(val, mask) (val) &= (TB_U32)(~(mask))
#define TB_BIT_ISSET(val, bitnum) (val) & ((TB_U32)(1 << (bitnum)))

/* Register operation */
#define TB_REG_ADDR(base, offset) ((TB_RegAddr)(base) + (offset))
#define TB_REG(base, offset) (*(TB_RegValPtr)TB_REG_ADDR((base), (offset)))
#define TB_REG_READ(base, offset) TB_REG(base, offset)
#define TB_REG_WRITE(base, offset, value) TB_REG(base, offset) = (TB_U32)(value)
#define TB_REG_BIT_SET(base, offset, mask) TB_REG(base, offset) |= (TB_U32)(mask)
#define TB_REG_BIT_CLR(base, offset, mask) TB_REG(base, offset) &= ((TB_U32) ~(mask))
/* Float register value operation */
#define TB_REG_FLOAT(base, offset) (*(TB_Float *)TB_REG_ADDR((base), (offset)))
#define TB_REG_READFLOAT(base, offset) TB_REG_FLOAT(base, offset)
#define TB_REG_WRITEFLOAT(base, offset, value) TB_REG_FLOAT(base, offset) = (TB_Float)(value)

#define TB_FREQ_HZ 1
#define TB_FREQ_KHZ (TB_FREQ_HZ * 1000)
#define TB_FREQ_MHZ (TB_FREQ_KHZ * 1000)

#define TB_MEM_KB 1024
#define TB_MEM_MB (TB_MEM_KB * 1024)

/**
 *  TESTBASE Device API Return Value
 */
typedef int TB_Return;
#define TB_RETURN_NOT_OK -1
#define TB_RETURN_OK 0

/**
 *  TESTBASE Device API Error Code
 */
#define TB_ERR_DEVICE_NOT_EXIST 1
#define TB_ERR_OPEN_DEVICE_FAILURE 2
#define TB_ERR_MMAP_DEVICE_FAILURE 3
#define TB_ERR_ALLOC_HANDLER_FAILURE 4
#define TB_ERR_INVALID_HANDLER 5
#define TB_ERR_INVALID_ARGUMENTS 6
#define TB_ERR_INVALID_CHANNEL 7
#define TB_ERR_INVALID_FAULT_CODE 8
/**
 *  TESTBASE Device API Version Type
 */
typedef struct
{
    TB_U8 major;
    TB_U8 minor;
    TB_U8 revision;
} TB_Version;

/**
 *  TESTBASE Device API Compatibility information
 */
typedef enum
{
    TB_DEV_TYPE_CPCIE,
    TB_DEV_TYPE_PCIE,
    TB_DEV_TYPE_PCIE_EXIO,
    TB_DEV_TYPE_USB
} TB_DeviceType;

typedef struct
{
    TB_DeviceType type;
    TB_U8 hwMajor;
    TB_U8 hwMinor;
    TB_U8 hwRevision;
    TB_U8 fwMajor;
    TB_U8 fwMinor;
    TB_U8 fwRevision;
} TB_Compatibility;
#endif
