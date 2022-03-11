/*****************************************************************************************
 * SineGeneration Module
 * Contains single task, which will pend on a DMA flag, calculate sinewave values based
 * on current configurations and store in DMA ping pong buffer
 * 02/14/2022 Dominic Danis
 *****************************************************************************************/
#ifndef SINE_GENERATION_H_
#define SINE_GENERATION_H_
/*****************************************************************************************
* Init function - creates task and Mutex.
* Dominic Danis 3/10/2022
*****************************************************************************************/
void SineGenInit(void);
/*****************************************************************************************
* Public setter function to set frequency
* Dominic Danis 3/10/2022
*****************************************************************************************/
void SinewaveSetFreq(INT16U freq);
/*****************************************************************************************
* Public setter function to set level
* Dominic Danis 3/10/2022
*****************************************************************************************/
void SinewaveSetLevel(INT8U level);
/*****************************************************************************************
* Getter function for frequency
* Dominic Danis 3/10/2022
*****************************************************************************************/
INT16U SinewaveGetFreq(void);
/*****************************************************************************************
* Getter function for level
* Dominic Danis 3/10/2022
*****************************************************************************************/
INT8U SinewaveGetLevel(void);

#endif
