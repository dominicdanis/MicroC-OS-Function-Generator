/*******************************************************************************
 * uCOS TSI Module
 * Contains single task, which pends on a time delay of 5ms.
 * This module is not complete. Code must be written
 * to post a semaphore only when there is a sensor state change.
 * 02/17/2022 Aili Emory
 *******************************************************************************/
#include "os.h"     /*Header files: Dependencies*/
#include "app_cfg.h"
#include "MCUType.h"
#include "uCOSTSI.h"
#include "k65TWR_GPIO.h"

typedef enum {PROC1START2, PROC2START1} TSI_TASK_STATE_T;
typedef struct{
    INT16U baseline;
    INT16U offset;
    INT16U threshold;
}TOUCH_LEVEL_T;
typedef struct{             /*Synchronous Buffer*/
    INT16U buffer;
    OS_SEM flag;
}TSI_BUFFER;
/**********************************************************************************
* Allocate task control block
**********************************************************************************/
static OS_TCB tsiTaskTCB;
/*************************************************************************
* Allocate task stack space
*************************************************************************/
static CPU_STK tsiTaskStk[APP_CFG_TSI_TASK_STK_SIZE];

#define MAX_NUM_ELECTRODES 16U
#define E1_TOUCH_OFFSET  0x0400U    // Touch offset from baseline
#define E2_TOUCH_OFFSET  0x0400U    // Determined experimentally
#define TSI0_ENABLE()    TSI0->GENCS |= TSI_GENCS_TSIEN_MASK
#define TSI0_DISABLE()   TSI0->GENCS &= ~TSI_GENCS_TSIEN_MASK

static TOUCH_LEVEL_T tsiSensorLevels[MAX_NUM_ELECTRODES];   /*Private Resources*/
static void tsiStartScan(INT8U channel);
static void tsiProcScan(INT8U channel);
static void tsiChCalibration(INT8U channel);
static void tsiTask(void *p_arg);
static TSI_BUFFER tsiBuffer;

static INT16U tsiSensorFlags = 0;

/********************************************************************************
 * K65TWR_TSI0Init: Initializes TSI0 module
 * -Public
 ********************************************************************************/
void tsiInit(void){
    OS_ERR os_err;

    SIM->SCGC5 |= SIM_SCGC5_TSI(1);         /*Turn on clock to TSI module*/
    SIM->SCGC5 |= SIM_SCGC5_PORTB(1);

    PORTB->PCR[18]=PORT_PCR_MUX(0);         /*Set electrode pins to ALT0*/
    PORTB->PCR[19]=PORT_PCR_MUX(0);
    tsiSensorLevels[BRD_PAD1_CH].offset = E1_TOUCH_OFFSET;
    tsiSensorLevels[BRD_PAD2_CH].offset = E2_TOUCH_OFFSET;

    TSI0->GENCS = ((TSI_GENCS_EXTCHRG(5))|      /*16 consecutive scans, Prescale divide by 32, software trigger*/
                   (TSI_GENCS_REFCHRG(5))|      /*16uA ext. charge current, 16uA Ref. charge current, .592V dV*/
                   (TSI_GENCS_DVOLT(1))|
                   (TSI_GENCS_PS(5))|
                   (TSI_GENCS_NSCN(15)));
    TSI0_ENABLE();
    tsiChCalibration(BRD_PAD1_CH);
    tsiChCalibration(BRD_PAD2_CH);

    tsiBuffer.buffer = 0x00;           /*Initialize the TSI Buffer and semaphore*/
    OSSemCreate(&(tsiBuffer.flag),"TSI Semaphore",0,&os_err);

    OSTaskCreate((OS_TCB     *)&tsiTaskTCB,         /*Create the TSI task*/
                (CPU_CHAR   *)"uCOS TSI Task",
                (OS_TASK_PTR ) tsiTask,
                (void       *) 0,
                (OS_PRIO     ) APP_CFG_TSI_TASK_PRIO,
                (CPU_STK    *)&tsiTaskStk[0],
                (CPU_STK     )(APP_CFG_TSI_TASK_STK_SIZE / 10u),
                (CPU_STK_SIZE) APP_CFG_TSI_TASK_STK_SIZE,
                (OS_MSG_QTY  ) 0,
                (OS_TICK     ) 0,
                (void       *) 0,
                (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR     *)&os_err);
}
/********************************************************************************
* tsiCalibration: Calibration to find non-touch baseline for a channel
*                 channel - the channel to calibrate, range 0-15
*                 Note - the sensor must not be pressed when this is executed.
* -Private
 ********************************************************************************/
static void tsiChCalibration(INT8U channel){
        tsiStartScan(channel);
        while((TSI0->GENCS & TSI_GENCS_EOSF_MASK) == 0){} //wait for scan to finish
        TSI0->GENCS |= TSI_GENCS_EOSF(1);    //Clear flag
        tsiSensorLevels[channel].baseline = (INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK);
        tsiSensorLevels[channel].threshold = tsiSensorLevels[channel].baseline +
                                             tsiSensorLevels[channel].offset;
}
/********************************************************************************
* tsiTask: uCOS Task
*            Processes and starts alternate sensors each time through to avoid
*            blocking.
*            In order to not block the task period should be > 5ms.
*            To not miss a press, the task period should be < ~25ms.
* -Private
  ********************************************************************************/
static void tsiTask(void *p_arg){
    OS_ERR os_err;
    TSI_TASK_STATE_T tsiTaskState = PROC1START2;
    (void)p_arg;

    while(1){
    OSTimeDly(5,OS_OPT_TIME_PERIODIC,&os_err);

    tsiStartScan(BRD_PAD1_CH);
        switch(tsiTaskState){
        case PROC1START2:
            tsiProcScan(BRD_PAD1_CH);
            tsiStartScan(BRD_PAD2_CH);
            tsiTaskState = PROC2START1;
            break;
        case PROC2START1:
            tsiProcScan(BRD_PAD2_CH);
            tsiStartScan(BRD_PAD1_CH);
            tsiTaskState = PROC1START2;
            break;
        default:
            tsiTaskState = PROC1START2;
            break;
        }
    }
}
/********************************************************************************
* TSIStartScan: Starts a scan of a TSI sensor.
*               channel - the TSI channel to be started. Range 0-15
* -Private
 ********************************************************************************/
static void tsiStartScan(INT8U channel){
    TSI0->DATA = TSI_DATA_TSICH(channel);       //set channel
    TSI0->DATA |= TSI_DATA_SWTS(1);             //start a scan sequence
}
/********************************************************************************
* TSIProcScan: Waits for the scan to complete, then sets the appropriate
*              flags is a touch was detected.
*              Note the scan must be started before this is called.
*              channel - the channel to be processed
* -Private
 ********************************************************************************/
static void tsiProcScan(INT8U channel){
    OS_ERR os_err;
    while((TSI0->GENCS & TSI_GENCS_EOSF_MASK) == 0){}
    TSI0->GENCS |= TSI_GENCS_EOSF(1);                   /*Clear flag*/

    if((INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK) > tsiSensorLevels[channel].threshold){  /* Process channel */
        tsiSensorFlags |= (INT16U)(1<<channel);
        tsiBuffer.buffer = tsiSensorFlags;
        (void)OSSemPost(&(tsiBuffer.flag), OS_OPT_POST_1, &os_err);   /* Signal new data in buffer */
    }else{
    }
}
/********************************************************************
* TSIPend(): A function to provide access to the TSI buffer via a
*            semaphore. Returns value of sensor flag variable and clears it to
*            receive sensor press only one time.
* - Public
********************************************************************/
INT16U tsiPend(INT16U tout, OS_ERR *os_err){
    OSSemPend(&(tsiBuffer.flag),tout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, os_err);
    tsiSensorFlags = 0;
    return(tsiBuffer.buffer);
}
