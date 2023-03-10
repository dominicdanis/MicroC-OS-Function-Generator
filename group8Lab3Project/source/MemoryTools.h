/*******************************************************************************
* MemoryTools contains a public function for checksum and an interface for
* EEPROM via SPI - configured for function generator use.
* 01/25/2022 Nick Coyle
*******************************************************************************/

#ifndef MEMORYTOOLS_H_
#define MEMORYTOOLS_H_
INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr);

#endif /* MEMORYTOOLS_H_ */
