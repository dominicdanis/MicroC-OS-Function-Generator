/*****************************************************************************************
 * SineGeneration Module
 * Contains single task, which will pend on a DMA flag, calculate sinewave values based
 * on current configurations and store in DMA ping pong buffer
 * 02/14/2022 Nick Coyle, Aili Emory, Dominic Danis
 *****************************************************************************************/

#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "arm_math.h"
#include "SineGeneration.h"

#define TS 44739

/****************************************************************************************
* Allocate task control block
****************************************************************************************/
static OS_TCB sineGenTaskTCB;
/****************************************************************************************
* Allocate task stack space.
****************************************************************************************/
static CPU_STK sineGenTaskTaskStk[APP_CFG_SINEGEN_TASK_STK_SIZE];
typedef struct{
    INT8U frequency;
    INT8U amplitude;
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
* Private Function Prototypes
*****************************************************************************************/
static SINE_SPECS sineGetSpecs(void);

/*****************************************************************************************
* Init function - creates task and Mutex.
*****************************************************************************************/
void SineGenInit(void){
    OS_ERR os_err;
    OSTaskCreate((OS_TCB *)&sineGenTaskTCB,
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
                (OS_ERR     *)&os_err);
    OSMutexCreate(&SineMutexKey,"Sine Mutex", &os_err);
}
/*****************************************************************************************
* Public setter function to set frequency
*****************************************************************************************/
void SinewaveSetFreq(INT8U freq){
    OS_ERR os_err;
    OSMutexPend(&SineMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    CurrentSpecs.frequency = freq;
    OSMutexPost(&SineMutexKey, OS_OPT_POST_NONE, &os_err);
}
/*****************************************************************************************
* Public setter function to set amplitude
*****************************************************************************************/
void SinewaveSetAmp(INT8U amp){
    OS_ERR os_err;

    OSMutexPend(&SineMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    CurrentSpecs.amplitude = amp;
    OSMutexPost(&SineMutexKey, OS_OPT_POST_NONE, &os_err);
}
/*****************************************************************************************
* Getter function for SW specs. Unfinished. Will change to a public getter and return
* level/frequency
*****************************************************************************************/
static SINE_SPECS sineGetSpecs(void){
    SINE_SPECS current_specs;
    OS_ERR os_err;

    OSMutexPend(&SineMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    current_specs = CurrentSpecs;
    OSMutexPost(&SineMutexKey, OS_OPT_POST_NONE, &os_err);
    return current_specs;
}

/*****************************************************************************************
* sineGenTask - unfinished. Will pend on DMA ISR, get current configurations, compute sine values
* store in PP buffer
*****************************************************************************************/
static void sineGenTask(void *p_arg){
    INT32U index;                               //These variable may change as sine math gets worked out
    SINE_SPECS current_specs;
    INT32S generated_values;
    INT8U scaled_freq;
    OS_ERR os_err;
    (void)p_arg;

    index = IndexPend();                        //pend on the DMA
    current_specs = sineGetSpecs();              //grab specs for calculation
    generated_values = arm_sin_q15(TS*current_specs.frequency);
    generated_values *= current_specs.amplitude;
    //store in pp buffer
}

