/*****************************************************************************************
 * DMA Module
 * DMA writes values from the ping pong buffer to the DAC.
 * DMA configured to use hardware triggers from PIT timer.
 * 02/25/2022 Nick Coyle
 * Includes functions by Todd Morton in DMA notes
 *****************************************************************************************/

#ifndef DMA_H_
#define DMA_H_

#define NUM_BLOCKS                  2
#define BYTES_PER_SAMPLE     		2
#define SAMPLES_PER_BLOCK           1024
#define BYTES_PER_BLOCK             (SAMPLES_PER_BLOCK*BYTES_PER_SAMPLE)
#define BYTES_PER_BUFFER            (NUM_BLOCKS*BYTES_PER_BLOCK)

/******************************************************************************************
* Public functions
******************************************************************************************/
void DMAInit(void);
INT8U DMAReadyPend(OS_TICK tout, OS_ERR *os_err_ptr);
void DMAFillBuffer(INT8U index, INT16U *samples);

#endif /* DMA_H_ */
