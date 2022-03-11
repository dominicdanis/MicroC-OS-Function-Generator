/*****************************************************************************************
* PulseTrain Module
* Contains frequency and level data for the Pulse Train. The functions that access this data
* must use a mutex. When the setter functions (for frequency and level) are called, the functions
* set the current specs and they set the values in the FTM3 registers, to reflect the new frequency or level.
*
* 02/15/2022 Aili Emory
*****************************************************************************************/
#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "PulseTrain.h"

#define BUS_FREQ 60000000         /*60 MHz*/

static INT16U Mod_Value;
/****************************************************************************************/
typedef struct{
    INT16U frequency;
    INT8U level;
}PULSE_TRAIN_SPECS;
/****************************************************************************************
* Private Resources
****************************************************************************************/
static PULSE_TRAIN_SPECS CurrentSpecs;
static OS_MUTEX PulseTrainMutexKey;
/*****************************************************************************************
* Init function - creates task and Mutex.
*****************************************************************************************/
void PulseTrainInit(void){
    OS_ERR os_err;

    OSMutexCreate(&PulseTrainMutexKey,"Pulse Train Mutex", &os_err);

    SIM->SCGC3 |= SIM_SCGC3_FTM3(1);   /* Enable clock gate for FTM3 */
    SIM->SCGC5 |= SIM_SCGC5_PORTE(1);  /* Enable clock gate for PORTE */
    PORTE->PCR[8] = PORT_PCR_MUX(6);   /* Set PCR for FTM output */
    FTM3->CONTROLS[3].CnSC = FTM_CnSC_ELSA(0)|FTM_CnSC_ELSB(1);   /*PWM polarity */
}
/*****************************************************************************************
* Public setter function to set frequency
*****************************************************************************************/
void PulseTrainSetFreq(INT16U freq){
    INT8U ps_value;
    INT8U scaler_value;
    OS_ERR os_err;
    OSMutexPend(&PulseTrainMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    CurrentSpecs.frequency = freq;
    OSMutexPost(&PulseTrainMutexKey, OS_OPT_POST_NONE, &os_err);

    /*PS - Prescaler Factor Selection
     *  0..Divide by 1
     *  1..Divide by 2
     *  2..Divide by 4
     *  3..Divide by 8
     *  4..Divide by 16
     *  5..Divide by 32
     *  6..Divide by 64
     *  7..Divide by 128*/
    if(freq > 1000){
        ps_value = 0;
        scaler_value = 1;
    }
    else if(freq > 100){
        ps_value = 4;
        scaler_value = 16;
    }
    else{
        ps_value = 7;
        scaler_value = 128;
    }
    /* Bus clock, center-aligned, dynamic prescaler set by the frequency */
    FTM3->SC = FTM_SC_CLKS(1)|FTM_SC_CPWMS(1)|FTM_SC_PS(ps_value);
    Mod_Value = BUS_FREQ / (scaler_value*freq*2);
    FTM3->MOD = FTM_MOD_MOD(Mod_Value);     /* Set pulse train period */
}
/*****************************************************************************************
* Public setter function to set amplitude
*****************************************************************************************/
void PulseTrainSetLevel(INT8U level){
    INT16U cnV;
    OS_ERR os_err;
    OSMutexPend(&PulseTrainMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    CurrentSpecs.level = level;
    OSMutexPost(&PulseTrainMutexKey, OS_OPT_POST_NONE, &os_err);

    cnV = (Mod_Value *level*5)/100;
    FTM3->CONTROLS[3].CnV =  FTM_CnV_VAL(cnV);  /* Set pulse train duty cycle */
}
/*****************************************************************************************
* Getter function for frequency
*****************************************************************************************/
INT16U PulseTrainGetFreq(void){
    INT16U freq;
    OS_ERR os_err;
    OSMutexPend(&PulseTrainMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    freq = CurrentSpecs.frequency;
    OSMutexPost(&PulseTrainMutexKey, OS_OPT_POST_NONE, &os_err);
    return freq;
}
/*****************************************************************************************
* Getter function for level
*****************************************************************************************/
INT8U PulseTrainGetLevel(void){
    INT8U level;
    OS_ERR os_err;
    OSMutexPend(&PulseTrainMutexKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    level = CurrentSpecs.level;
    OSMutexPost(&PulseTrainMutexKey, OS_OPT_POST_NONE, &os_err);
    return level;
}
