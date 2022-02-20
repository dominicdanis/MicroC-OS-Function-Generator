/*****************************************************************************************
 * DMA Module
 * 02/14/2022 Nick Coyle, Aili Emory, Dominic Danis
 *****************************************************************************************/

#ifndef DMA_H_
#define DMA_H_

/*****************************************************************************************************
* Definition of sample stream macros/constants
*****************************************************************************************************/
#define NUM_BLOCKS                  1
#define BYTES_PER_SAMPLE     		2
#define SAMPLES_PER_BLOCK           1024
#define BYTES_PER_BLOCK             (SAMPLES_PER_BLOCK*BYTES_PER_SAMPLE)
#define BYTES_PER_BUFFER            (NUM_BLOCKS*BYTES_PER_BLOCK)

/*****************************************************************************************************
* Definition of global VARIABLES
*****************************************************************************************************/
/* Right now this is a global buffer so the DMA and processing module can access them.
 * Should replace with a function.
 * TDM 02/10/2017
 */

// THIS IS THE PINGPONG BUFFER
//extern INT16S DMABuffer[NUM_BLOCKS][SAMPLES_PER_BLOCK] = {{23}};

/*****************************************************************************************************
* Declaration of public functions
*****************************************************************************************************/
void DMAInit(void);
INT8U DMAPend(OS_TICK tout, OS_ERR *os_err_ptr);

#endif /* DMA_H_ */
