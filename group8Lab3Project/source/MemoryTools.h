/*******************************************************************************
* EECE444 Lab 3 Code
*   MemoryTools.c is a module containing memory interface and validation. It contains
*   a public function for checksum and an interface for EEPROM via SPI - configured
*   for function generator use
*
* 01/25/2022 Nick Coyle, Aili Emory, Dominic Danis
*******************************************************************************/

#ifndef MEMORYTOOLS_H_
#define MEMORYTOOLS_H_
/*******************************************************************************
*   Enumerated type for UI states. Held in MemoryTools publically because both
*   AppLab3 and MemoryTools will need to use.
*******************************************************************************/
typedef enum {DEFAULT, SINEWAVE, PULSE_TRAIN} UI_STATES_T;
/*******************************************************************************
*   Structure for state, frequency and level. Held in MemoryTools publically
*   because both AppLab3 and MemoryTools will need to use.
*******************************************************************************/
typedef struct{
    UI_STATES_T state;
    INT16U freq;
    INT8U level;
} SAVED_CONFIG;
/*******************************************************************************
*   MemLoad() will load the current configurations, validate them and return
*   either the valid configuration or the default configuration
*******************************************************************************/
SAVED_CONFIG MemLoad(void);
/*******************************************************************************
*   SaveConfig() will save the passed configurations to EEPROM via SPI
*******************************************************************************/
void SaveConfig(SAVED_CONFIG);
/*******************************************************************************
*   MemChkSum() Given 2 addresses this calculates and returns a checksum
*   checksum is defined as the 16-bit sum of each byte in the block of memory
*   it will rollover
*   Nick Coyle 10/11/2021
*******************************************************************************/
INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr);

#endif /* MEMORYTOOLS_H_ */
