/*****************************************************************************************
* PulseTrain Module
* Contains frequency and level data for the Pulse Train. The functions that access this data
* must use a mutex. When the setter functions (for frequency and level) are called, the functions
* set the current specs and they set the values in the FTM3 registers, to reflect the new frequency or level.
*
* 02/15/2022 Aili Emory
*****************************************************************************************/
#ifndef PULSETRAIN_H_
#define PULSETRAIN_H_
/*****************************************************************************************
* Init function - creates task and Mutex.
* 2/15/2022 Aili Emory
*****************************************************************************************/
void PulseTrainInit(void);
/*****************************************************************************************
* Public setter function to set frequency
* 02/15/2022 Aili Emory
*****************************************************************************************/
void PulseTrainSetFreq(INT16U freq);
/*****************************************************************************************
* Public setter function to set duty cycle
* 02/15/2022 Aili Emory
*****************************************************************************************/
void PulseTrainSetLevel(INT8U level);
/*****************************************************************************************
* Getter function for frequency
* 02/15/2022 Aili Emory
*****************************************************************************************/
INT16U PulseTrainGetFreq(void);
/*****************************************************************************************
* Getter function for level
* 02/15/2022 Aili Emory
*****************************************************************************************/
INT8U PulseTrainGetLevel(void);

#endif /* PULSETRAIN_H_ */
