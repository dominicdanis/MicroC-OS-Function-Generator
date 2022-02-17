/*******************************************************************************
* EECE444 Lab 3 Code
*   MemoryTools.c is a module containing memory interface and validation. It contains
*   a public function for checksum and an interface for EEPROM via SPI - configured
*   for function generator use
*
* 01/25/2022 Nick Coyle, Aili Emory, Dominic Danis
*******************************************************************************/
#include "MCUType.h"               /* Include header files                    */
#include "MemoryTools.h"

static INT8U validateConfig(SAVED_CONFIG);

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
/*******************************************************************************
*   MemLoad() will load the current configurations, validate them and return
*   either the valid configuration or the default configuration
*******************************************************************************/
SAVED_CONFIG MemLoad(void){
    SAVED_CONFIG current;
    //current = some function to read from SPI
    if(validateConfig(current)==0){
        current.state = SINEWAVE;
        current.freq = 1000;
        current.level = 10;
    }
    else{}
    return current;
}
/*******************************************************************************
*   SaveConfig() will save the passed configurations to EEPROM via SPI
*******************************************************************************/
void SaveConfig(SAVED_CONFIG current){
    //do something here - with SPI to save current to EERPOM
}
/*******************************************************************************
*   validateConfig() will validate a current configuration given Lab 3 requirements
*   and return a boolean result
*******************************************************************************/
static INT8U validateConfig(SAVED_CONFIG current){
    INT8U valid = 1;
    if(current.level>20){
        valid = 0;
    }
    else{}
    if(current.freq>10000 || current.freq<1){
        valid = 0;
    }
    else{}
    return valid;
}


