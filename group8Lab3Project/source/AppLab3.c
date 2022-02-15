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
#include "MemoryTools.h"

#define LOW_ADDR_BIN 0x00000000U
#define HIGH_ADDR_BIN 0x001FFFFFU

typedef enum {CLEAR, COUNT, HOLD} UI_STATES_T;

/*****************************************************************************************
* Private Resources
*****************************************************************************************/
/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB appStartTaskTCB;
/*****************************************************************************************
* Allocate task stack space.
*****************************************************************************************/
static CPU_STK appStartTaskStk[APP_CFG_START_TASK_STK_SIZE];
/*****************************************************************************************
* Task Function Prototypes. 
*****************************************************************************************/
static void  appStartTask(void *p_arg);

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
    INT16U chksum;
    (void)p_arg;                       				/* Avoid compiler warning for unused variable   */

    OS_CPU_SysTickInitFreq(SYSTEM_CLOCK);
    GpioDBugBitsInit();
    LcdInit();

    /* display the chksum on the LCD */
    chksum = MemChkSum((INT8U *)LOW_ADDR_BIN, (INT8U *)HIGH_ADDR_BIN);
    LcdDispByte(LCD_ROW_2, LCD_COL_1, LCD_LAYER_CHKSUM, (INT8U)(chksum>>8));
    LcdDispByte(LCD_ROW_2, LCD_COL_3, LCD_LAYER_CHKSUM, (INT8U)(chksum));

    KeyInit();

    OSTaskDel((OS_TCB *)0, &os_err);
}
