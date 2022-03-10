/*****************************************************************************************
* PulseTrain Module
* Contains frequency and level data for the Pulse Train. The functions that access this data
* must use a mutex. When the setter functions (for frequency and level) are called, the functions
* set the current specs and they set the values in the FTM3 registers, to reflect the new frequency or level.
* 02/15/2022 Aili Emory
*****************************************************************************************/

#ifndef PULSETRAIN_H_
#define PULSETRAIN_H_

void PulseTrainInit(void);
void PulseTrainSetFreq(INT16U freq);
void PulseTrainSetLevel(INT8U level);
INT16U PulseTrainGetFreq(void);
INT8U PulseTrainGetLevel(void);


#endif /* PULSETRAIN_H_ */
