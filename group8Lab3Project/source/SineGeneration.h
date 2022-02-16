#ifndef SINE_GENERATION_H_
#define SINE_GENERATION_H_

void SineGenInit(void);
void SinewaveSetFreq(INT16U freq);
void SinewaveSetLevel(INT8U level);
INT16U SinewaveGetFreq(void);
INT8U SinewaveGetLevel(void);

#endif
