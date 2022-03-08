/*****************************************************************************************
 * SineGeneration Module
 * Contains single task, which will pend on a DMA flag, calculate sinewave values based
 * on current configurations and store in DMA ping pong buffer
 * 02/14/2022 Dominic Danis
 *****************************************************************************************/
#ifndef SINE_GENERATION_H_
#define SINE_GENERATION_H_

void SineGenInit(void);
void SinewaveSetFreq(INT16U freq);
void SinewaveSetLevel(INT8U level);
INT16U SinewaveGetFreq(void);
INT8U SinewaveGetLevel(void);

#endif
