#ifndef SINE_GENERATION_H_
#define SINE_GENERATION_H_

void SineGenInit(void);
void SinewaveSetFreq(INT16U freq);
void SinewaveSetLevel(INT8U level);
INT16U SineWaveGetFreq(void);
INT8U SineWaveGetLevel(void);

#endif
