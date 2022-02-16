/*
 * PulseTrain.h
 * 02/15/2022 Nick Coyle, Dominic Danis, Aili Emory
 */

#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "PulseTrain.h"

/****************************************************************************************
* Allocate task control block
****************************************************************************************/
static OS_TCB pulseTrainTaskTCB;
/****************************************************************************************
* Allocate task stack space.
****************************************************************************************/
static CPU_STK pulseTrainTaskStk[APP_CFG_PULSETRAIN_TASK_STK_SIZE];
typedef struct{
    INT16U frequency;
    INT8U level;
}SINE_SPECS;
/****************************************************************************************
* Private Resources
****************************************************************************************/
static SINE_SPECS CurrentSpecs;
static OS_MUTEX PulseTrainMutexKey;
/*****************************************************************************************
* Task Function Prototypes.
*****************************************************************************************/
static void pulseTrainTask(void *p_arg);
/*****************************************************************************************
* Private Function Prototypes
*****************************************************************************************/
static SINE_SPECS pulseTrainGetSpecs(void);
/*****************************************************************************************
* Init function - creates task and Mutex.
*****************************************************************************************/
void PulseTrainInit(void){
    OS_ERR os_err;
    OSTaskCreate((OS_TCB *)&pulseTrainTaskTCB,
                "Pulse Train Task ",
                pulseTrainTask,
                (void *) 0,
                APP_CFG_PULSETRAIN_TASK_PRIO,
                &pulseTrainTaskStk[0],
                (APP_CFG_PULSETRAIN_TASK_STK_SIZE / 10u),
                APP_CFG_PULSETRAIN_TASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                (OS_ERR     *)&os_err);
    OSMutexCreate(&PulseTrainMutexKey,"Pulse Train Mutex", &os_err);
}
/*****************************************************************************************
* Public setter function to set frequency
*****************************************************************************************/
void PulseTrainSetFreq(INT16U freq){
    OS_ERR os_err;

}
/*****************************************************************************************
* Public setter function to set amplitude
*****************************************************************************************/
void PulseTrainSetLevel(INT8U level){
    OS_ERR os_err;

}
/*****************************************************************************************
* Getter function for frequency
*****************************************************************************************/
INT16U PulseTrainGetFreq(void){
    INT16U freq;
    OS_ERR os_err;
   freq=0;
    return freq;
}
/*****************************************************************************************
* Getter function for level
*****************************************************************************************/
INT8U PulseTrainGetLevel(void){
    INT8U level;
    OS_ERR os_err;
   level=0;
    return level;
}
/*****************************************************************************************
* Getter function for PulseTrain specs.
*****************************************************************************************/
static SINE_SPECS pulseTrainGetSpecs(void){
    SINE_SPECS current_specs;
    OS_ERR os_err;

    return current_specs;
}


/*****************************************************************************************
*
*****************************************************************************************/
static void pulseTrainTask(void *p_arg){
    INT32U index;                               //These variable may change as sine math gets worked out
    SINE_SPECS current_specs;
    INT32S generated_values;
    INT8U scaled_freq;
    OS_ERR os_err;
    (void)p_arg;


}
