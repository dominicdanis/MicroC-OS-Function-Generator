/*******************************************************************************
* EECE444 Lab 3 Code
*   EEPROM.c is a module that will read and write to a 93LC56B EEPROM using SPI
*
* 02/24/2022 Nick Coyle, Aili Emory, Dominic Danis
*******************************************************************************/
#include "MCUType.h"               /* Include header files                    */
#include "EEPROM.h"
#include "MemoryTools.h"

/*
 *  we are sending:
 *  1 bit for interface
 *  16 bits for sinewave frequency
 *  8 bits for sinewave level
 *  16 bits for pulsetrain frequency
 *  8 bits for pulse train duty cycle
 *
 *  49 bits total.
 *
 *  Idea - the levels and interface can be merged into 1 16 bit block.
 *
 *  to check if the startup is valid we will do a checksum and see if it's above a max value
 *
 *  need functions for reading and writing
 *
 *  function for validating
 *
 *  defined constants for several places in memory
 * */
static INT8U validateConfig(SAVED_CONFIG);
static INT16U EEPROMRead(INT8U addr);
static void EEPROMWrite(INT8U addr, INT16U wr_data);
static INT16U EEPROMXfr16(INT32U pushr);

void EEPROMInit(void){
    //Clocks and connecting pins
    SIM->SCGC3 |= SIM_SCGC3_SPI2(1);
    SIM->SCGC5 |= SIM_SCGC5_PORTD(1);
    PORTD->PCR[11] = PORT_PCR_MUX(2);
    PORTD->PCR[12] = PORT_PCR_MUX(2);
    PORTD->PCR[13] = PORT_PCR_MUX(2);
    PORTD->PCR[14] = PORT_PCR_MUX(2);

    //CTAR config for writing
    SPI2->CTAR[0] = SPI_CTAR_BR(3)|SPI_CTAR_PBR(2)|SPI_CTAR_FMSZ(15)|
                    SPI_CTAR_CPHA(0)|SPI_CTAR_CPOL(0);
    //CTAR config for reading
    SPI2->CTAR[1] = SPI_CTAR_BR(3)|SPI_CTAR_PBR(2)|SPI_CTAR_FMSZ(15)|
                    SPI_CTAR_CPHA(1)|SPI_CTAR_CPOL(0);
    //Controller with transfer FIFOs disabled
    SPI2->MCR = SPI_MCR_MSTR(1)|SPI_MCR_DIS_TXF(1)|SPI_MCR_DIS_RXF(1);
}



void EEPROMSaveState(INT8U state){
    EEPROMWrite(0,state);
}
void EEPROMSaveSineFreq(INT16U sine_freq){

}
void EEPROMSaveSineLevel(INT8U sine_level){

}
void EEPROMSavePulseFreq(INT16U pulse_freq){

}
void EEPROMSavePulseLevel(INT8U pulse_level){

}

SAVED_CONFIG EEPROMGetConfig(void){
    SAVED_CONFIG current;
    current.state = (INT8U)EEPROMRead(0);
    current.pulse_freq = EEPROMRead(0);
    current.pulse_level = EEPROMRead(0);
    current.sine_freq = EEPROMRead(0);
    current.sine_level = EEPROMRead(0);
    return current;
}

static INT16U EEPROMRead(INT8U addr){
    INT16U rd_value;
    //send command and address
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA((0x6<<8)|addr));
    rd_value = EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1)|SPI_PUSHR_CTAS(1)|
                      SPI_PUSHR_TXDATA(0x0000));
    return rd_value;
}

static void EEPROMWrite(INT8U addr, INT16U wr_data){
    //send command and address
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA((0x5<<8) | (addr & 0x7F)));
    //send data
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA(wr_data));
}

static INT16U EEPROMXfr16(INT32U pushr){
    //Wait for previous transmit and clear flag
    while((SPI2->SR & SPI_SR_TFFF_MASK) == 0){}
    SPI2->SR |= SPI_SR_TFFF_MASK;
    //Initiate transfer
    SPI2->PUSHR = pushr;
    //Wait for received data and clear flag
    while((SPI2->SR & SPI_SR_RFDF_MASK) == 0){}
    SPI2->SR |= SPI_SR_RFDF_MASK;
    //return recieved data
    return SPI2->POPR;
}

static void EEPROMCmd(INT16U cmd){
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(0)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA(cmd));
}



/*******************************************************************************
*   validateConfig() will validate a current configuration given Lab 3 requirements
*   and return a boolean result
*******************************************************************************/

/*
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
*/
