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
/*******************************************************************************************
* DMAInInit
* Initializes DMA for an input stream from ADC0 to ping-pong buffers
* 02/25/2022 Nick Coyle
*******************************************************************************************/
void DMAInit(void);
/****************************************************************************************
 * DMA Flag
 * The DMA ISR is going to post this semaphore every time it gets halfway or all the way
 * to the end of the ping pong buffer. The sinegen buffer filling task will call this to
 * get the current index it should be writing to.
 * 08/30/2015 TDM
 ***************************************************************************************/
INT8U DMAReadyPend(OS_TICK tout, OS_ERR *os_err_ptr);
/*******************************************************************************************
* DMAFillBuffer
* Fills the DMA ping pong buffer
* 02/25/2022 Nick Coyle
*******************************************************************************************/
void DMAFillBuffer(INT8U index, INT16U *samples);

#endif /* DMA_H_ */
