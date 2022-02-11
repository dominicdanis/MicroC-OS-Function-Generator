/*****************************************************************************************
* Lab2 EE444 MicroC/OS Function Generator Team Project
* A function generator that runs under MicroC/OS, a preemptive multitasking kernel.
* 01/25/2022 Nick Coyle, Aili Emory, Dominic Danis
*****************************************************************************************/
#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "K65TWR_ClkCfg.h"
#include "K65TWR_GPIO.h"
#include "LcdLayered.h"
#include "uCOSKey.h"
#include "SWCounter.h"
#include "MemoryTools.h"

#define LOW_ADDR_BIN 0x00000000U
#define HIGH_ADDR_BIN 0x001FFFFFU

typedef enum {CLEAR, COUNT, HOLD} UI_STATES_T;

/*****************************************************************************************
* Private Resources
*****************************************************************************************/
static UI_STATES_T CurrentState;							/* UI state machine 		*/
static INT32U appTimerCount;								/* store the count 			*/
static OS_MUTEX appTimerCntrKey;							/* MUTEX key for the count  */

/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB appStartTaskTCB;
static OS_TCB appTimerDisplayTaskTCB;
static OS_TCB appTimerControlTaskTCB;

/*****************************************************************************************
* Allocate task stack space.
*****************************************************************************************/
static CPU_STK appStartTaskStk[APP_CFG_START_TASK_STK_SIZE];
static CPU_STK appTimerDisplayTaskStk[APP_CFG_APP_TIMER_DISPLAY_TASK_STK_SIZE];
static CPU_STK appTimerControlTaskStk[APP_CFG_APP_TIMER_CONTROL_TASK_STK_SIZE];

/*****************************************************************************************
* Task Function Prototypes. 
*****************************************************************************************/
static void  appStartTask(void *p_arg);
static void  appTimerDisplayTask(void *p_arg);
static void  appTimerControlTask(void *p_arg);

/*****************************************************************************************
* Other Function Prototypes.
*****************************************************************************************/
static void appDispCntHelper(INT8U row, INT8U layer, INT32U count);

/*****************************************************************************************
* main()
*****************************************************************************************/
void main(void) {
    OS_ERR  os_err;

    K65TWR_BootClock();
    CPU_IntDis();               		/* Disable all interrupts, OS will enable them  */

    OSInit(&os_err);                    /* Initialize uC/OS-III                         */

    OSTaskCreate(&appStartTaskTCB,                  /* Address of TCB assigned to task */
                 "Start Task",                      /* Name you want to give the task */
                 appStartTask,                      /* Address of the task itself */
                 (void *) 0,                        /* p_arg is not used so null ptr */
                 APP_CFG_START_TASK_PRIO,           /* Priority you assign to the task */
                 &appStartTaskStk[0],               /* Base address of task�s stack */
                 (APP_CFG_START_TASK_STK_SIZE/10u), /* Watermark limit for stack growth */
                 APP_CFG_START_TASK_STK_SIZE,       /* Stack size */
                 0,                                 /* Size of task message queue */
                 0,                                 /* Time quanta for round robin */
                 (void *) 0,                        /* Extension pointer is not used */
                 (OS_OPT_TASK_NONE), 				/* Options */
                 &os_err);                          /* Ptr to error code destination */

    OSStart(&os_err);               /*Start multitasking(i.e. give control to uC/OS)    */
}

/*****************************************************************************************
* STARTUP TASK
* This should run once and be deleted. Could restart everything by creating.
*****************************************************************************************/
static void appStartTask(void *p_arg) {
    OS_ERR os_err;
    INT16U chksum;
    (void)p_arg;                       				/* Avoid compiler warning for unused variable   */

    OS_CPU_SysTickInitFreq(SYSTEM_CLOCK);
    GpioDBugBitsInit();

    OSMutexCreate(&appTimerCntrKey, "App Timer Mutex", &os_err);

    OSTaskCreate(&appTimerDisplayTaskTCB,           /* Create appTimerDisplayTask                    */
                "App Timer Display Task",
				appTimerDisplayTask,
                (void *) 0,
				APP_CFG_APP_TIMER_DISPLAY_TASK_PRIO,
                &appTimerDisplayTaskStk[0],
                (APP_CFG_APP_TIMER_DISPLAY_TASK_STK_SIZE / 10u),
				APP_CFG_APP_TIMER_DISPLAY_TASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);

    OSTaskCreate(&appTimerControlTaskTCB,    		/* Create appTimerControlTask                    */
                "App Timer Control Task",
				appTimerControlTask,
                (void *) 0,
				APP_CFG_APP_TIMER_CONTROL_TASK_PRIO,
                &appTimerControlTaskStk[0],
                (APP_CFG_APP_TIMER_CONTROL_TASK_STK_SIZE / 10u),
				APP_CFG_APP_TIMER_CONTROL_TASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);

    LcdInit();

    /* display the chksum on the LCD */
    chksum = MemChkSum((INT8U *)LOW_ADDR_BIN, (INT8U *)HIGH_ADDR_BIN);
    LcdDispByte(LCD_ROW_2, LCD_COL_1, LCD_LAYER_CHKSUM, (INT8U)(chksum>>8));
    LcdDispByte(LCD_ROW_2, LCD_COL_3, LCD_LAYER_CHKSUM, (INT8U)(chksum));

    KeyInit();
    SWCounterInit();

    CurrentState = CLEAR; 							/* state should be CLEAR out of reset */

    OSTaskDel((OS_TCB *)0, &os_err);
}

/*****************************************************************************************
* appTimerDisplayTask
* Display the stopwatch count on the LCD.
* Pends on a semaphore from the stopwatch module.
* Uses appTimerCntrKey mutex to protect the count as other tasks access it too.
*****************************************************************************************/
static void appTimerDisplayTask(void *p_arg){
    OS_ERR os_err;
    INT32U count;
    (void)p_arg;
    
    while(1){
        DB1_TURN_OFF();                             /* Turn off debug bit while waiting */
        count = SWCountPend(0, &os_err);
        DB1_TURN_ON();                         		/* Turn on debug bit while ready/running*/

        OSMutexPend(&appTimerCntrKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
        appTimerCount = count;
        OSMutexPost(&appTimerCntrKey, OS_OPT_POST_NONE, &os_err);

        appDispCntHelper(LCD_ROW_1, LCD_LAYER_TIMER, count); /* display the TIMER on the LCD */
    }
}

/*****************************************************************************************
* appTimerControlTask
* Set the state of the stopwatch depending on button presses from the keypad.
* Pends on a semaphore from the key module.
* Uses appTimerCntrKey mutex to protect the count as other tasks access it too.
*****************************************************************************************/
static void appTimerControlTask(void *p_arg){
    OS_ERR os_err;
    INT8C kchar;
    INT32U count;
    (void)p_arg;

    while(1) {
        DB0_TURN_OFF();                         	/* Turn off debug bit while waiting     */
        kchar = KeyPend(0, &os_err);
        DB0_TURN_ON();                          	/* Turn on debug bit while ready/running*/

        OSMutexPend(&appTimerCntrKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
        count = appTimerCount;
        OSMutexPost(&appTimerCntrKey, OS_OPT_POST_NONE, &os_err);

        if(kchar == '*') {							/* a '*' key press advances the UI state machine */
        	switch(CurrentState) {					/* the UI state machine 				    	 */
        	    case CLEAR:
        	       	CurrentState = COUNT;
        	       	SWCntrCntrlSet(1,0);
        	       	break;
        	    case COUNT:
        	       	CurrentState = HOLD;
        	       	SWCntrCntrlSet(0,0);
        	       	break;
        	    case HOLD:
        	       	CurrentState = CLEAR;
        	       	SWCntrCntrlSet(0,1);
        	       	break;
        	    default:
        	       	break;
        	}
        } else if (kchar == '#'){					/* a '#' keypress does lap capture				*/
        	appDispCntHelper(LCD_ROW_2, LCD_LAYER_LAP, count);
        } else {
        	// do nothing
        }
    }
}

/*****************************************************************************************
* appDispCntHelper
* Helper function to prevent code duplication. Displays the count on the given LCD_LAYER.
* Formats the count in minutes:seconds.tens_of_ms
* Rolls over to 0 when count exceeds 99:59.99
*****************************************************************************************/
static void appDispCntHelper(INT8U row, INT8U layer, INT32U count) {
	LcdDispDecWord(row, LCD_COL_1, layer, ((count % 6000000) / 60000), 2, LCD_DEC_MODE_LZ); /* minutes */
	LcdDispChar(row, LCD_COL_3, layer, ':');
	LcdDispDecWord(row, LCD_COL_4, layer, ((count % 60000) / 1000), 2, LCD_DEC_MODE_LZ); /* seconds */
	LcdDispChar(row, LCD_COL_6, layer, '.');
	LcdDispDecWord(row, LCD_COL_7, layer, (((count % 60000) % 1000) / 10), 2, LCD_DEC_MODE_LZ); /* tens of ms */
}

/****************************************************************************************/
