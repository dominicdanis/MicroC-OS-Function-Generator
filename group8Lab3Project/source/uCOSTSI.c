/*******************************************************************************
 * uCOS TSI Module
 * Implements capacitive touch sensing.
 * There is calibration of the sensors. When a sensor is touched, a flag is set, in a flag group
 * and OSFlagPost() is called. The flag is only set, at the start of a sensor press. The current
 * sensor state and last sensor state are compared to determine the edge of the sensor press start.
 *
 * 02/17/2022 Aili Emory
 * Includes functions by Todd Morton in Flag notes and TSI notes
 *******************************************************************************/
#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "uCOSTSI.h"
#include "k65TWR_GPIO.h"

typedef enum {T_ON, T_OFF} T_STATE;
typedef struct{
    INT16U baseline;
    INT16U offset;
    INT16U threshold;
    T_STATE state;
}TOUCH_LEVEL_T;
static OS_FLAG_GRP tsiFlags;
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
/********************************************************************************
 * TSIInit: Initializes TSI0 module
 * From TSI notes Todd Morton
 * 02/17/2022 Aili Emory - added flag group and task
 ********************************************************************************/
void TSIInit(void){
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

    OSFlagCreate(&tsiFlags,"TSI FLags",0,&os_err);

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
* From TSI notes Todd Morton
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
*          TSI sensors are scanned sequentially.
*          There are two pending service calls to do two things sequentially.
* From Flag notes Todd Morton
  ********************************************************************************/
static void tsiTask(void *p_arg){
    OS_ERR os_err;
    (void)p_arg;

    while(1){
        tsiStartScan(BRD_PAD1_CH);
        OSTimeDly(6,OS_OPT_TIME_PERIODIC,&os_err);
        tsiProcScan(BRD_PAD1_CH);
        DB3_TURN_ON();
        tsiStartScan(BRD_PAD2_CH);
        OSTimeDly(6,OS_OPT_TIME_PERIODIC,&os_err);
        tsiProcScan(BRD_PAD2_CH);
        DB3_TURN_OFF();
    }
}
/********************************************************************************
* TSIStartScan: Starts a scan of a TSI sensor.
*               channel - the TSI channel to be started. Range 0-15
* From TSI notes Todd Morton
 ********************************************************************************/
static void tsiStartScan(INT8U channel){
    TSI0->DATA = TSI_DATA_TSICH(channel);       //set channel
    TSI0->DATA |= TSI_DATA_SWTS(1);             //start a scan sequence
}
/********************************************************************************
* TSIProcScan: TSI scanner signals an event flag group when a sensor is touched.
*              The sensor must be released before a flag is signaled again.
* From Flag notes Todd Morton
 ********************************************************************************/
static void tsiProcScan(INT8U channel){
    OS_ERR os_err;
    INT16U tsi_touch_count;
    T_STATE cur_state;
    while((TSI0->GENCS & TSI_GENCS_EOSF_MASK) == 0){}
        TSI0->GENCS |= TSI_GENCS_EOSF(1); //Clear flag
        tsi_touch_count = (TSI0->DATA & TSI_DATA_TSICNT_MASK);
        if((tsi_touch_count > tsiSensorLevels[channel].threshold)){
            cur_state = T_ON;
        }else{
            cur_state = T_OFF;
        }
        if((cur_state == T_ON) && (tsiSensorLevels[channel].state == T_OFF)){
            DB2_TURN_ON();
            (void)OSFlagPost(&tsiFlags,
                    (OS_FLAGS)(1<<channel),OS_OPT_POST_FLAG_SET, &os_err);
            DB2_TURN_OFF();
            tsiSensorLevels[channel].state = T_ON;
        }else if(cur_state == T_OFF){
            tsiSensorLevels[channel].state = T_OFF;
        }else{
     }
}
/********************************************************************
* TSIPend(): TSIPend provides access to the TSI buffer via an
*            Event flag. Returns value of sensor flag variable and clears it to
*            receive sensor press only one time.
* From Flag notes Todd Morton
********************************************************************/
OS_FLAGS TSIPend(OS_TICK tout, OS_ERR *os_err){
    OS_FLAGS sflags;
    sflags = OSFlagPend (&tsiFlags, 0xFFFF,tout,OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME,(void *)0,os_err);
    return sflags;
}
