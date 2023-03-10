/**********************************************************************************
* MCUType.h - Include the MCU header and definitions and the standard defined types
*             to be used for the application.
*
* TDM 10/04/2017 Initial version to deprecate includes.h
* TDM 09/10/2018 Deprecated INT8C. Not needed for C99, MISRA, or MCUXpresso
*                Added C99 type option.
**********************************************************************************
* Make sure it is included only one time 
**********************************************************************************/
#ifndef  MCU_TYPE_PRESENT
#define  MCU_TYPE_PRESENT

/*********************************************************************************
 * MCU
 *********************************************************************************/
#include "MK65F18.h"
#define ARM_MATH_CM4
/*********************************************************************************
 * Standard types to include
 ********************************************************************************/
#define APP_TYPE_UCOS_EN    1
#define APP_TYPE_CMSIS_EN   0
#define APP_TYPE_WWU_EN     1
#define APP_TYPE_C99_EN     0

#if APP_TYPE_UCOS_EN
#include "cpu.h"
#endif

#if APP_TYPE_CMSIS_EN
#include "arm_math.h"
#endif

#if APP_TYPE_C99_EN
#include "stdint.h"
#endif

#if APP_TYPE_WWU_EN

/**********************************************************************************
* Standard WWU type definitions
**********************************************************************************/
typedef char                INT8C;
typedef unsigned char       INT8U;
typedef signed char         INT8S;
typedef unsigned short      INT16U;
typedef signed short        INT16S;
typedef unsigned long       INT32U;
typedef signed long         INT32S;
typedef unsigned long long  INT64U;
typedef signed long long    INT64S;
typedef float               FP32;
typedef double              FP64;

#endif
/**********************************************************************************
* General Defined Constants
**********************************************************************************/
#define FALSE    0
#define TRUE     1

#endif
