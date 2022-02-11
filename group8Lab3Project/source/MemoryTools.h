/*******************************************************************************
* EECE344 Lab 2 Code
*   MemoryTools.c is a module with a public function used to return the checksum
*   of 2 memory addresses
*
* Nick Coyle, 10/11/2021
*******************************************************************************/

#ifndef MEMORYTOOLS_H_
#define MEMORYTOOLS_H_

/*******************************************************************************
*   MemChkSum() Given 2 addresses this calculates and returns a checksum
*   checksum is defined as the 16-bit sum of each byte in the block of memory
*   it will rollover
*   Nick Coyle 10/11/2021
*******************************************************************************/
INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr);

#endif /* MEMORYTOOLS_H_ */
