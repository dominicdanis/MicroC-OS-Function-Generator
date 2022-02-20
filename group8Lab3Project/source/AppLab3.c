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
#include "SineGeneration.h"
#include "PulseTrain.h"
#include "MemoryTools.h"
#include "SineGeneration.h"
#include "PulseTrain.h"
#include "MemoryTools.h"
#include "DMA.h"


#define FREQ_LIMIT_HIGH 10000
#define FREQ_LIMIT_LOW 10
#define ASCII_CODE_ZERO 48
#define DEFAULT_FREQ 1000
#define DEFAULT_LEVEL 10

typedef enum {DEFAULT, SINEWAVE, PULSE_TRAIN} UI_STATES_T;

/*****************************************************************************************
 * Private Resources
 *****************************************************************************************/
static UI_STATES_T UIState;								 /* UI state machine 	     	 */
static OS_MUTEX appUIStateKey;							 /* MUTEX key for the UIState    */

/*****************************************************************************************
 * Allocate task control blocks
 *****************************************************************************************/
static OS_TCB appStartTaskTCB;
static OS_TCB appProcessKeyTaskTCB;
static OS_TCB appTouchSensorTaskTCB;

/*****************************************************************************************
 * Allocate task stack space.
 *****************************************************************************************/
static CPU_STK appStartTaskStk[APP_CFG_START_TASK_STK_SIZE];
static CPU_STK appProcessKeyTaskStk[APP_CFG_APP_PROCESS_KEY_TASK_STK_SIZE];
static CPU_STK appTouchSensorTaskStk[APP_CFG_APP_TOUCH_SENSOR_TASK_STK_SIZE];

/*****************************************************************************************
 * Task Function Prototypes.
 *****************************************************************************************/
static void  appStartTask(void *p_arg);
static void  appProcessKeyTask(void *p_arg);
static void  appTouchSensorTask(void *p_arg);

/*****************************************************************************************
 * Other Function Prototypes.
 *****************************************************************************************/


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
	UI_STATES_T current_state;

	(void)p_arg;                       				/* Avoid compiler warning for unused variable   */

	OS_CPU_SysTickInitFreq(SYSTEM_CLOCK);
	GpioDBugBitsInit();

	OSMutexCreate(&appUIStateKey, "App UIState Mutex", &os_err);

	OSTaskCreate(&appProcessKeyTaskTCB,           /* Create appProcessKeyTask                    */
			"App Process Key Task",
			appProcessKeyTask,
			(void *) 0,
			APP_CFG_APP_PROCESS_KEY_TASK_PRIO,
			&appProcessKeyTaskStk[0],
			(APP_CFG_APP_PROCESS_KEY_TASK_STK_SIZE / 10u),
			APP_CFG_APP_PROCESS_KEY_TASK_STK_SIZE,
			0,
			0,
			(void *) 0,
			(OS_OPT_TASK_NONE),
			&os_err);

	OSTaskCreate(&appTouchSensorTaskTCB,    		/* Create appTouchSensorTask                    */
			"App Touch Sensor Task",
			appTouchSensorTask,
			(void *) 0,
			APP_CFG_APP_TOUCH_SENSOR_TASK_PRIO,
			&appTouchSensorTaskStk[0],
			(APP_CFG_APP_TOUCH_SENSOR_TASK_STK_SIZE / 10u),
			APP_CFG_APP_TOUCH_SENSOR_TASK_STK_SIZE,
			0,
			0,
			(void *) 0,
			(OS_OPT_TASK_NONE),
			&os_err);

	LcdInit();
	KeyInit();
	DMAInit();

	if(0) {
	//if(MemIsValid()) {
		// read values from EEPROM
		//current_state = MemLoadState();
		OSMutexPend(&appUIStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
			UIState = current_state;
			//current_state = MemLoadState();
		OSMutexPost(&appUIStateKey, OS_OPT_POST_NONE, &os_err);

		if(current_state == PULSE_TRAIN) {
			LcdDispString(LCD_ROW_1, LCD_COL_12,LCD_LAYER_UI_STATE,"PULSE");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_1,LCD_LAYER_FREQ,(INT32U)PulseTrainGetFreq(), 5, LCD_DEC_MODE_AL);
			LcdDispString(LCD_ROW_2, LCD_COL_6,LCD_LAYER_FREQ,"Hz");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_15,LCD_LAYER_FREQ,(INT32U)PulseTrainGetLevel(), 2, LCD_DEC_MODE_AL);
		} else {
			LcdDispString(LCD_ROW_1, LCD_COL_12,LCD_LAYER_UI_STATE," SINE");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_1,LCD_LAYER_FREQ,(INT32U)SinewaveGetFreq(), 5, LCD_DEC_MODE_AL);
			LcdDispString(LCD_ROW_2, LCD_COL_6,LCD_LAYER_FREQ,"Hz");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_15,LCD_LAYER_FREQ,(INT32U)SinewaveGetLevel(), 2, LCD_DEC_MODE_AL);
		}
	} else {
		UIState = DEFAULT;
		PulseTrainSetFreq(DEFAULT_FREQ);
		PulseTrainSetLevel(DEFAULT_LEVEL);
		LcdDispString(LCD_ROW_1, LCD_COL_12,LCD_LAYER_UI_STATE," SINE");
		LcdDispDecWord(LCD_ROW_2, LCD_COL_1,LCD_LAYER_FREQ,(INT32U)DEFAULT_FREQ, 4, LCD_DEC_MODE_AL);
		LcdDispString(LCD_ROW_2, LCD_COL_5,LCD_LAYER_FREQ,"Hz");
		LcdDispDecWord(LCD_ROW_2, LCD_COL_15,LCD_LAYER_FREQ,(INT32U)DEFAULT_LEVEL, 2, LCD_DEC_MODE_AL);
	}

	OSTaskDel((OS_TCB *)0, &os_err);
}

/*****************************************************************************************
 * appProcessKeyTask
 * UI to edit the function generator frequency and type of wave.
 * Display the current UIState on the LCD.
 * Pends on a keypress from the uCOSKey Module.
 * 02/12/2022 Nick Coyle
 *****************************************************************************************/
static void appProcessKeyTask(void *p_arg){
	OS_ERR os_err;
	INT8C kchar;
	INT8U number_value;
	INT16U user_freq;
	UI_STATES_T current_state;
	(void)p_arg;
	user_freq = 0;

	OSMutexPend(&appUIStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
		current_state = UIState;
	OSMutexPost(&appUIStateKey, OS_OPT_POST_NONE, &os_err);

	while(1){
		DB1_TURN_OFF();                             /* Turn off debug bit while waiting */
		kchar = KeyPend(0, &os_err);
		DB1_TURN_ON();                         		/* Turn on debug bit while ready/running*/

		switch(kchar) {
		case '*':	/* * Key Not Used */
			break;
		case DC1: 	/* A Key */
			current_state = SINEWAVE;
			break;
		case DC2:	/* B Key */
			current_state = PULSE_TRAIN;
			break;
		case DC4:	/* D Key */
			current_state = DEFAULT;
			break;
		case '#': 	/* ENTER Key */
			if(user_freq >= FREQ_LIMIT_LOW && user_freq <= FREQ_LIMIT_HIGH){
				LcdDispClear(LCD_LAYER_USER_FREQ);
				LcdDispClear(LCD_LAYER_FREQ);
				if(current_state == PULSE_TRAIN) {
					PulseTrainSetFreq(user_freq);
				} else {
					SinewaveSetFreq(user_freq);
				}
				user_freq = 0;
			} else {
				// do nothing
			}
			break;
		case DC3: 	/* BACKSPACE Key */
			if(user_freq > 9) {
				user_freq = user_freq / 10;
				LcdDispClear(LCD_LAYER_USER_FREQ);
				LcdDispDecWord(LCD_ROW_1, LCD_COL_1, LCD_LAYER_USER_FREQ, (INT32U)user_freq, 5, LCD_DEC_MODE_AL);
			} else {
				LcdDispClear(LCD_LAYER_USER_FREQ);
				user_freq = 0;
			}
			break;
		default:	/* Any Number Keys */
			number_value = kchar - ASCII_CODE_ZERO;
			if(user_freq == 0) {
				if(kchar != '0') {
					user_freq = number_value;
					LcdDispDecWord(LCD_ROW_1, LCD_COL_1, LCD_LAYER_USER_FREQ, (INT32U)user_freq, 5, LCD_DEC_MODE_AL);
				} else {
					// do nothing
				}
			} else {
				if(((user_freq*10) + number_value) <= FREQ_LIMIT_HIGH){
					user_freq = (user_freq*10) + number_value;
					LcdDispDecWord(LCD_ROW_1, LCD_COL_1, LCD_LAYER_USER_FREQ, (INT32U)user_freq, 5, LCD_DEC_MODE_AL);
				} else {
					// do nothing
				}
			}
			break;
		}

		// store the state
		OSMutexPend(&appUIStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
			UIState = current_state;
		OSMutexPost(&appUIStateKey, OS_OPT_POST_NONE, &os_err);

		if(current_state == PULSE_TRAIN) {
			LcdDispString(LCD_ROW_1, LCD_COL_12,LCD_LAYER_UI_STATE,"PULSE");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_1,LCD_LAYER_FREQ,(INT32U)PulseTrainGetFreq(), 5, LCD_DEC_MODE_AL);
			LcdDispString(LCD_ROW_2, LCD_COL_6,LCD_LAYER_FREQ,"Hz");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_15,LCD_LAYER_FREQ,(INT32U)PulseTrainGetLevel(), 2, LCD_DEC_MODE_AL);
		} else {
			LcdDispString(LCD_ROW_1, LCD_COL_12,LCD_LAYER_UI_STATE," SINE");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_1,LCD_LAYER_FREQ,(INT32U)SinewaveGetFreq(), 5, LCD_DEC_MODE_AL);
			LcdDispString(LCD_ROW_2, LCD_COL_6,LCD_LAYER_FREQ,"Hz");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_15,LCD_LAYER_FREQ,(INT32U)SinewaveGetLevel(), 2, LCD_DEC_MODE_AL);
		}
	}
}

/*****************************************************************************************
* appTouchSensorTask
* UI to edit the function generator level.
* There are 21 linear levels from 0-20.
* Pends on input from touch sensors.
 *****************************************************************************************/
static void appTouchSensorTask(void *p_arg){
	OS_ERR os_err;
	INT8C kchar;
	(void)p_arg;

	while(1) {
		DB0_TURN_OFF();                         	/* Turn off debug bit while waiting     */
		kchar = KeyPend(0, &os_err);
		DB0_TURN_ON();                          	/* Turn on debug bit while ready/running*/
	}
}

