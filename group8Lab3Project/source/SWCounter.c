/***************************************************************************************
* EE444 Lab2 Stop Watch Counter Module
* The counter counts in increments of 10ms when the stop watch is enabled.
* Reset sets the counter to 0.
* A periodic timed task loop makes the counter task increment every 10ms using the
* kernel timer.
* Uses a synchronous buffer to contain the counter data and signal changes.
* A mutex protects the counter data.
* 01/20/2022 Nick Coyle
***************************************************************************************/
#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "SWCounter.h"
#include "K65TWR_GPIO.h"

typedef struct{ 						/* synchronous buffer */
    INT32U count;
    OS_SEM flag;
}SWCOUNTER_BUFFER;

typedef struct{
	INT8U enable;
	INT8U reset;
}SWCOUNTER_CONTROL;

/****************************************************************************************
* Private Resources
****************************************************************************************/
static SWCOUNTER_BUFFER swCntrBuffer;
static SWCOUNTER_CONTROL swCntrCntrl;
static OS_MUTEX swCntrCntrlKey;

/*****************************************************************************************
* Task Function Prototypes.
*****************************************************************************************/
static void swCounterTask(void *p_arg);

/****************************************************************************************
* Allocate task control block
****************************************************************************************/
static OS_TCB swCounterTaskTCB;

/****************************************************************************************
* Allocate task stack space.
****************************************************************************************/
static CPU_STK swCounterTaskStk[APP_CFG_SWCOUNTER_TASK_STK_SIZE];

/****************************************************************************************
* SWCountPend() - A function to provide access to the count via a semaphore.
*    - Public
*****************************************************************************************/
INT32U SWCountPend(INT16U tout, OS_ERR *os_err){
    (void)OSSemPend(&(swCntrBuffer.flag),tout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, os_err);
    return swCntrBuffer.count;
}

/*****************************************************************************************
* SWCounterInit() - Initialization routine for the SW counter module
*    - Public
*****************************************************************************************/
void SWCounterInit(void){
    OS_ERR os_err;

    // Create mutex key, semaphore, and task
    OSMutexCreate(&swCntrCntrlKey, "Stop Watch Mutex", &os_err);
    OSSemCreate(&(swCntrBuffer.flag),"SWCounter Semaphore",0,&os_err);

    //Create the key task
    OSTaskCreate((OS_TCB     *)&swCounterTaskTCB,
                (CPU_CHAR   *)"uCOS swCounter Task ",
                (OS_TASK_PTR ) swCounterTask,
                (void       *) 0,
                (OS_PRIO     ) APP_CFG_SWCOUNTER_TASK_PRIO,
                (CPU_STK    *)&swCounterTaskStk[0],
                (CPU_STK     )(APP_CFG_SWCOUNTER_TASK_STK_SIZE / 10u),
                (CPU_STK_SIZE) APP_CFG_SWCOUNTER_TASK_STK_SIZE,
                (OS_MSG_QTY  ) 0,
                (OS_TICK     ) 0,
                (void       *) 0,
                (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR     *)&os_err);

    SWCntrCntrlSet(0, 1); 											 /* Initialize the counter */
}

/*****************************************************************************************
* swCounterTask() - A periodic task to update the count in 10ms intervals.
* Only updates the count when the stopwatch is enabled.
* Resets the count to 0 when reset is high.
* Access to SWCOUNTER_CONTROL pends on a mutex since multiple tasks access that data.
*    - Private
*****************************************************************************************/
static void swCounterTask(void *p_arg) {
	OS_ERR os_err;
	INT8U enable;
	INT8U reset;
	(void)p_arg;

	while(1) {
		DB2_TURN_OFF();                             					/* Turn off debug bit while waiting */
		OSTimeDly(10, OS_OPT_TIME_PERIODIC, &os_err); 					/* timed task loop, period 10ms */
		DB2_TURN_ON();                          						/* Turn on debug bit while ready/running*/

		/* task code */
		OSMutexPend(&swCntrCntrlKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
		enable = swCntrCntrl.enable;
		reset = swCntrCntrl.reset;
		OSMutexPost(&swCntrCntrlKey, OS_OPT_POST_NONE, &os_err);

		if(reset == 1) {
			swCntrBuffer.count = 0;
			(void)OSSemPost(&(swCntrBuffer.flag), OS_OPT_POST_1, &os_err);   /* Signal new data in buffer */
		} else if(enable == 1 && reset == 0) {
			swCntrBuffer.count = swCntrBuffer.count + 10; 					/* increment the count by 10ms */
			(void)OSSemPost(&(swCntrBuffer.flag), OS_OPT_POST_1, &os_err);  /* Signal new data in buffer */
		} else {
			// do nothing
		}
	}
}

/******************************************************************************************
* SWCntrCntrlSet() - Stop or start the stopwatch or reset it to 0.
* Access to SWCOUNTER_CONTROL pends on a mutex since multiple tasks access that data.
*    - Public
******************************************************************************************/
void SWCntrCntrlSet(INT8U enable, INT8U reset) {
	OS_ERR os_err;

	OSMutexPend(&swCntrCntrlKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
	swCntrCntrl.enable = enable;
	swCntrCntrl.reset = reset;
	OSMutexPost(&swCntrCntrlKey, OS_OPT_POST_NONE, &os_err);
}
