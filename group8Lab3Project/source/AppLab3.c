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
#include "uCOSTSI.h"
#include "EEPROM.h"

#define FREQ_LIMIT_HIGH 10000
#define FREQ_LIMIT_LOW 10
#define ASCII_CODE_ZERO 48
#define DEFAULT_FREQ 1000
#define DEFAULT_LEVEL 10

typedef enum {SINEWAVE, PULSE_TRAIN} UI_STATES_T;
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
static void appDispHelper(UI_STATES_T current_state);
static INT8U intLen(INT16U input);

/*****************************************************************************************
 * main()
 *****************************************************************************************/
void main(void) {
	OS_ERR  os_err;

	K65TWR_BootClock();
	CPU_IntDis();               		/* Disable all interrupts, OS will enable them  */

	OSInit(&os_err);                    /* Initialize uC/OS-III                         */

	OSTaskCreate(&appStartTaskTCB,             /* Address of TCB assigned to task */
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
			(OS_OPT_TASK_NONE), 			   /* Options */
			&os_err);                          /* Ptr to error code destination */

	OSStart(&os_err);               		   /*Start multitasking(i.e. give control to uC/OS)    */
}

/*****************************************************************************************
 * STARTUP TASK
 * This should run once and be deleted. Could restart everything by creating.
 *****************************************************************************************/
static void appStartTask(void *p_arg) {
	OS_ERR os_err;
	SAVED_CONFIG loaded_state;
	UI_STATES_T current;
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
	SineGenInit();
	DMAInit();
	TSIInit();
	EEPROMInit();
	SineGenInit();
	PulseTrainInit();
	SineGenInit();

    loaded_state = EEPROMGetConfig();
    if(loaded_state.state==0){
        current = SINEWAVE;
    }
    else{
        current = PULSE_TRAIN;
    }
    OSMutexPend(&appUIStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
        UIState = current;
    OSMutexPost(&appUIStateKey, OS_OPT_POST_NONE, &os_err);
    SinewaveSetLevel(loaded_state.sine_level);
    SinewaveSetFreq(loaded_state.sine_freq);
    PulseTrainSetLevel(loaded_state.pulse_level);
    PulseTrainSetFreq(loaded_state.pulse_freq);
    appDispHelper(current);
	PulseTrainInit();
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
			EEPROMSaveState(0);
			break;
		case DC2:	/* B Key */
			current_state = PULSE_TRAIN;
			EEPROMSaveState(1);
			break;
		case DC4:	/* D Key */
			current_state = SINEWAVE;
			EEPROMSaveState(0);
			SinewaveSetFreq(DEFAULT_FREQ);
			SinewaveSetLevel(DEFAULT_LEVEL);
			PulseTrainSetFreq(DEFAULT_FREQ);
			PulseTrainSetLevel(DEFAULT_LEVEL);
			break;
		case '#': 	/* ENTER Key */
			if(user_freq >= FREQ_LIMIT_LOW && user_freq <= FREQ_LIMIT_HIGH){
				LcdDispClear(LCD_LAYER_USER_FREQ);
				LcdDispClear(LCD_LAYER_FREQ);
				if(current_state == PULSE_TRAIN) {
					PulseTrainSetFreq(user_freq);
					EEPROMSavePulseFreq(user_freq);
				} else {
					SinewaveSetFreq(user_freq);
					EEPROMSaveSineFreq(user_freq);
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

		appDispHelper(current_state);
	}
}
/*****************************************************************************************
* appTouchSensorTask
* UI to edit the function generator level.
* There are 21 linear levels from 0-20.
* Pends on input from touch sensors.
* 02/16/2022 Aili Emory
 *****************************************************************************************/
static void appTouchSensorTask(void *p_arg){
    OS_FLAGS cur_sense_flags;
    UI_STATES_T current_state;
    INT8U level;
    OS_ERR os_err;
    (void)p_arg;

    while(1){
        cur_sense_flags = TSIPend(100,&os_err);
        if(os_err == OS_ERR_TIMEOUT){
            cur_sense_flags = 0;
        }else{
        }
        OSMutexPend(&appUIStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
            current_state = UIState;                                            /* Determine Current State */
        OSMutexPost(&appUIStateKey, OS_OPT_POST_NONE, &os_err);
		if(current_state == SINEWAVE){
            level = SinewaveGetLevel();
        }else if(current_state == PULSE_TRAIN){
            level = PulseTrainGetLevel();
        } else {
        	// do nothing
        }
        if((cur_sense_flags & (1<<BRD_PAD1_CH)) != 0){                          /* Increment Level */
            if(level == 20){
                //do nothing
            }
            else{
                level = level +1;
            }
        }
        else{
        }
        if((cur_sense_flags & (1<<BRD_PAD2_CH)) != 0){                          /* Decrement Level */
            if(level == 0){
                //do nothing
            }
            else{
                level = level -1;
            }
        }
        else{
        }

        if(current_state == SINEWAVE){                                          /* Set Level, Display value */
            SinewaveSetLevel(level);
            EEPROMSaveSineLevel(level);
        }else{
            PulseTrainSetLevel(level);
            EEPROMSavePulseLevel(level);
        }
        appDispHelper(current_state);
    }
}

/*****************************************************************************************
* appDispHelper
* Helper function to prevent code duplication. Displays the state, freq, and level
* on the LCD.
*****************************************************************************************/
static void appDispHelper(UI_STATES_T current_state) {
	INT8U level;
	INT16U freq;
	INT8U lenFreq;

	if(current_state == PULSE_TRAIN){
		freq = PulseTrainGetFreq();
		lenFreq = intLen(freq);
		level = 5*PulseTrainGetLevel();				/* multiply by 5 to convert to percentage out of 100% */
		LcdDispString(LCD_ROW_1, LCD_COL_12,LCD_LAYER_UI_STATE,"PULSE");
		LcdDispDecWord(LCD_ROW_2, LCD_COL_1,LCD_LAYER_FREQ,(INT32U)freq, lenFreq, LCD_DEC_MODE_AL);
		LcdDispString(LCD_ROW_2, lenFreq+1,LCD_LAYER_FREQ,"Hz   ");
		if(level == 100){
			LcdDispDecWord(LCD_ROW_2, LCD_COL_13,LCD_LAYER_LEVEL,(INT32U)level, 3, LCD_DEC_MODE_AL);
		}else if(level > 9){
			LcdDispChar(LCD_ROW_2, LCD_COL_13,LCD_LAYER_LEVEL,' ');
			LcdDispDecWord(LCD_ROW_2, LCD_COL_14,LCD_LAYER_LEVEL,(INT32U)level, 2, LCD_DEC_MODE_AL);
		}else{
			LcdDispString(LCD_ROW_2, LCD_COL_13,LCD_LAYER_LEVEL,"  ");
			LcdDispDecWord(LCD_ROW_2, LCD_COL_15,LCD_LAYER_LEVEL,(INT32U)level, 1, LCD_DEC_MODE_AL);
		}
		LcdDispChar(LCD_ROW_2, LCD_COL_16,LCD_LAYER_LEVEL,'%');
	}else if(current_state == SINEWAVE){
		freq = SinewaveGetFreq();
		lenFreq = intLen(freq);
		level = SinewaveGetLevel();
		LcdDispString(LCD_ROW_1, LCD_COL_12,LCD_LAYER_UI_STATE," SINE");
		LcdDispDecWord(LCD_ROW_2, LCD_COL_1,LCD_LAYER_FREQ,(INT32U)freq, lenFreq, LCD_DEC_MODE_AL);
		LcdDispString(LCD_ROW_2, lenFreq+1,LCD_LAYER_FREQ,"Hz   ");
		LcdDispString(LCD_ROW_2, LCD_COL_13,LCD_LAYER_LEVEL,"  ");
		if(level > 9) {
			LcdDispDecWord(LCD_ROW_2, LCD_COL_15,LCD_LAYER_LEVEL,(INT32U)level, 2, LCD_DEC_MODE_AL);
		}else{
			LcdDispChar(LCD_ROW_2, LCD_COL_15,LCD_LAYER_LEVEL,' ');
			LcdDispDecWord(LCD_ROW_2, LCD_COL_16,LCD_LAYER_LEVEL,(INT32U)level, 1, LCD_DEC_MODE_AL);
		}
	}else{
		// do nothing
	}
}

/****************************************************************************************
 * intLen
 * Return the number of digits in an integer
 ****************************************************************************************/
static INT8U intLen(INT16U input){
	INT8U lenInput = 0;
	while(input > 0){
		input = input/10;
		lenInput++;
	}
	return lenInput;
}
