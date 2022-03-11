/*****************************************************************************************
 * SineGeneration Module
 * Contains single task, which will pend on a DMA flag, calculate sinewave values based
 * on current configurations and store in DMA ping pong buffer
 * 02/14/2022 Dominic Danis
 *****************************************************************************************/

#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "arm_math.h"
#include "SineGeneration.h"
#include "DMA.h"
#include "K65TWR_GPIO.h"

#define TS 44739                /* TS = Ts=1/48000 scaled up by (2^31) */
#define BIT31_MASK 0x80000000   /* 2^31 */
#define BIT15_MASK 0x8000       /* 2^15 */
#define BIT20_MASK 0x100000     /* 2^20 */
#define AMP_SCALE 1490          /* (3/(20*3.3))*(2^15) and rounded up */
#define DC_OFF 2047             /* halfway point of DAC0 = 2048-1 */

static INT16U sine_vals[SAMPLES_PER_BLOCK];

/****************************************************************************************
* Allocate task control block
****************************************************************************************/
static OS_TCB sineGenTaskTCB;
/****************************************************************************************
* Allocate task stack space.
****************************************************************************************/
static CPU_STK sineGenTaskTaskStk[APP_CFG_SINEGEN_TASK_STK_SIZE];
typedef struct{
    INT16U frequency;
    INT8U level;
}SINE_SPECS;
/****************************************************************************************
* Private Resources
****************************************************************************************/
static SINE_SPECS CurrentSpecs;
static OS_MUTEX SineMutexKey;
/*****************************************************************************************
* Task Function Prototypes.
*****************************************************************************************/
static void sineGenTask(void *p_arg);

/*****************************************************************************************
* Init function - creates task and Mutex.
*****************************************************************************************/
void SineGenInit(void){
    OS_ERR os_err;
    OSTaskCreate(&sineGenTaskTCB,
                "sineGen Task ",
                sineGenTask,
                (void *) 0,
                APP_CFG_SINEGEN_TASK_PRIO,
                &sineGenTaskTaskStk[0],
                (APP_CFG_SINEGEN_TASK_STK_SIZE / 10u),
                APP_CFG_SINEGEN_TASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);

    OSMutexCreate(&SineMutexKey,"Sine Mutex", &os_err);
}
/*****************************************************************************************
* Public setter function to set frequency
*****************************************************************************************/
void SinewaveSetFreq(INT16U freq){
    OS_ERR os_err;
    OSMutexPend(&SineMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    CurrentSpecs.frequency = freq;
    OSMutexPost(&SineMutexKey, OS_OPT_POST_NONE, &os_err);
}
/*****************************************************************************************
* Public setter function to set amplitude
*****************************************************************************************/
void SinewaveSetLevel(INT8U level){
    OS_ERR os_err;
    OSMutexPend(&SineMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    CurrentSpecs.level = level;
    OSMutexPost(&SineMutexKey, OS_OPT_POST_NONE, &os_err);
}
/*****************************************************************************************
* Getter function for frequency
*****************************************************************************************/
INT16U SinewaveGetFreq(void){
    INT16U freq;
    OS_ERR os_err;
    OSMutexPend(&SineMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    freq = CurrentSpecs.frequency;
    OSMutexPost(&SineMutexKey, OS_OPT_POST_NONE, &os_err);
    return freq;
}
/*****************************************************************************************
* Getter function for level
*****************************************************************************************/
INT8U SinewaveGetLevel(void){
    INT8U level;
    OS_ERR os_err;
    OSMutexPend(&SineMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    level = CurrentSpecs.level;
    OSMutexPost(&SineMutexKey, OS_OPT_POST_NONE, &os_err);
    return level;
}

/*****************************************************************************************
* sineGenTask - unfinished. Will pend on DMA ISR, get current configurations, compute sine values
* store in PP buffer
*****************************************************************************************/
static void sineGenTask(void *p_arg){
    INT32U sine_val;
    INT32U argument = 0;
    INT8U index = 0;
    INT16U frequency;
    INT8U level;
    OS_ERR os_err;
    (void)p_arg;

    while(1) {
        DB4_TURN_OFF();                             /* Turn off debug bit while waiting */
        index = DMAReadyPend(0, &os_err);           /* pend on the DMA */
        DB4_TURN_ON();
        frequency = SinewaveGetFreq();
        level = SinewaveGetLevel();
        for(INT16U i=0; i<SAMPLES_PER_BLOCK; i++){
            sine_val = arm_sin_q31(argument);
            if((sine_val & BIT31_MASK) > 0){                  // value is negative
                sine_val = (~sine_val & (~BIT31_MASK));       // drop the sign bit and invert
                sine_val = ((sine_val + BIT15_MASK) >> 15);   // round down to 16 bits
                sine_val = (sine_val) * (AMP_SCALE*level);    // 16bit*16bit = 32bit result
                sine_val = ((sine_val + BIT20_MASK) >> 20);   // round down to 12 bits
                sine_vals[i] = (DC_OFF - ((INT16U)(sine_val))); // add DC offset
            }else{                                            // value is positive
                sine_val = ((sine_val + BIT15_MASK) >> 15);   // round down to 16 bits
                sine_val = (sine_val) * (AMP_SCALE*level);    // 16bit*16bit = 32bit result
                sine_val = ((sine_val + BIT20_MASK) >> 20);   // round down to 12 bits
                sine_vals[i] = ((INT16U)sine_val + DC_OFF);   // add DC offset
            }
            argument = ((argument + (TS*frequency)) & (~BIT31_MASK)); // mask the sign bit
        }
        DMAFillBuffer(index, sine_vals);
    }
}
