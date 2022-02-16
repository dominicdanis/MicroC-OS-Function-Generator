/*
 * PulseTrain.h
 * 02/15/2022 Nick Coyle, Dominic Danis, Aili Emory
 */

#ifndef PULSETRAIN_H_
#define PULSETRAIN_H_

void PulseTrainInit(void);
void PulseTrainSetFreq(INT16U freq);
void PulseTrainSetLevel(INT8U level);
INT16U PulseTrainGetFreq(void);
INT8U PulseTrainGetLevel(void);


#endif /* PULSETRAIN_H_ */
