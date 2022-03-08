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

#define TS 44739
#define BIT31_MASK 2147483648	/* 1<<31 */
#define AMP_SCALE 95325

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
	INT64U temp = 0;
	INT16U frequency;
	INT8U level;
    OS_ERR os_err;
    (void)p_arg;

    while(1) {
    	DB4_TURN_OFF();                             /* Turn off debug bit while waiting */
    	index = DMAReadyPend(0, &os_err);                   //pend on the DMA
    	DB4_TURN_ON();
    	frequency = SinewaveGetFreq();
    	level = SinewaveGetLevel();

    	for(INT16U i=0; i<SAMPLES_PER_BLOCK; i++){
    		sine_val = arm_sin_q31(argument);
    		if((sine_val & BIT31_MASK) > 0){				//value is negative
    			sine_val = (sine_val<<1);
    			sine_val = (~sine_val);
    			temp = (AMP_SCALE*level);                    //multiply level and scaler to find Vpeak
    			temp *= sine_val;                           //find offset for each sample
    			temp = temp>>42;                            //scale back to a 12 bit
    			sine_vals[i] = (2048 - ((INT16U)(temp)));
    		}else{											//value is positive
    			sine_val = (sine_val<<1);
    			temp = (AMP_SCALE*level);
    			temp *= sine_val;
    			temp = temp>>42;
    			sine_vals[i] = ((INT16U)temp + 2048);
    		}

    		argument += (TS*frequency);
    		if(argument >= BIT31_MASK) {
    			argument -= BIT31_MASK;
    		}else{
    			// do nothing
    		}
    	}
    	DMAFillBuffer(index, sine_vals);
    }
}
