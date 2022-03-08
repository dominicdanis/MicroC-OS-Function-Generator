/*******************************************************************************
* MemoryTools contains a public function for checksum and an interface for
* EEPROM via SPI - configured for function generator use.
* 01/25/2022 Nick Coyle
*******************************************************************************/
#include "MCUType.h"               /* Include header files                    */
#include "MemoryTools.h"

/*******************************************************************************
 *   MemChkSum() Given 2 addresses this calculates and returns a checksum
 *   checksum is defined as the 16-bit sum of each byte in the block of memory
 *   Nick Coyle 10/11/2021
 *******************************************************************************/
INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr) {
    INT16U chk_sum = 0;
    INT8U *start = startaddr;

    while(start < endaddr) {
        chk_sum += (INT16U)*start;        /* start is 1 byte so explicit cast for course conventions */
        start++;
    }
    chk_sum += (INT16U)*start;            /* get the last byte outside the loop to prevent terminal count bug */
    return chk_sum;
}
