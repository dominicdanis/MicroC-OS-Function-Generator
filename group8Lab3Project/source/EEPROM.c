/*******************************************************************************
* EECE444 Lab 3 Code
*   EEPROM.c is a module that will read and write to a 93LC56B EEPROM using SPI
*
* 02/24/2022 Nick Coyle, Aili Emory, Dominic Danis
*******************************************************************************/
#include "MCUType.h"               /* Include header files                    */
#include "EEPROM.h"
#include "MemoryTools.h"
#include "os.h"

/*
 *  5 bits for each level
 *  14 bits for each frequency
 *  1 bit for interface
 *
 *  39 bits total
 * */
#define SET_MASK 0xFFFF
#define STATE_MASK 0x20
#define PULSE_L_MASK 0x7C0
#define SINE_L_MASK 0xF800
#define ADDR1 0x1
#define ADDR2 0x2
#define ADDR3 0x3
#define EWEN 0x04C0
#define EWDS 0x0400
#define ERAL 0x0480

/*  CurrentOutput is 3 INT16Us composed of
 *  CurrentOutput[0] = SineFreq
 *  CurrentOutput[1] = PulseFreq
 *  CurrentOutput[2] = (15:11 - SineLevel), (10:6 - Pulse Level), (5 - current interface) (4:0 - unassigned)
 * */
static INT16U CurrentOutput[3];
static INT8U validateConfig(SAVED_CONFIG);
static void EEPROMCmd(INT16U cmd);
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

    SAVED_CONFIG config;
    config.pulse_freq = 0;
    config = EEPROMGetConfig();
    EEPROMSaveSineFreq(0);
    config = EEPROMGetConfig();
    EEPROMSaveSineFreq(1234);

}

void EEPROMSaveState(INT8U state){
    OS_ERR os_err;

    CurrentOutput[2] &= (SET_MASK & (state>>5));
    EEPROMCmd(EWEN);
    EEPROMWrite(ADDR3,CurrentOutput[2]);
    OSTimeDly(6,OS_OPT_TIME_DLY,&os_err);
    EEPROMCmd(EWDS);
}
void EEPROMSaveSineFreq(INT16U sine_freq){
    OS_ERR os_err;

    CurrentOutput[0] = sine_freq;
    EEPROMCmd(EWEN);
    EEPROMWrite(ADDR1,CurrentOutput[0]);
    OSTimeDly(6,OS_OPT_TIME_DLY,&os_err);
    EEPROMCmd(EWDS);
}
void EEPROMSaveSineLevel(INT8U sine_level){
    OS_ERR os_err;

    CurrentOutput[2] &= (SET_MASK & (sine_level>>11));
    EEPROMCmd(EWEN);
    EEPROMWrite(ADDR3,CurrentOutput[2]);
    OSTimeDly(6,OS_OPT_TIME_DLY,&os_err);
    EEPROMCmd(EWDS);
}
void EEPROMSavePulseFreq(INT16U pulse_freq){
    OS_ERR os_err;

    CurrentOutput[1] = pulse_freq;
    EEPROMCmd(EWEN);
    EEPROMWrite(ADDR2,CurrentOutput[1]);
    OSTimeDly(6,OS_OPT_TIME_DLY,&os_err);
    EEPROMCmd(EWDS);
}
void EEPROMSavePulseLevel(INT8U pulse_level){
    OS_ERR os_err;

    CurrentOutput[2] &= (SET_MASK & (pulse_level>>6));
    EEPROMCmd(EWEN);
    EEPROMWrite(ADDR3,CurrentOutput[2]);
    OSTimeDly(6,OS_OPT_TIME_DLY,&os_err);
    EEPROMCmd(EWDS);
}

SAVED_CONFIG EEPROMGetConfig(void){
    SAVED_CONFIG current;
    CurrentOutput[0] = EEPROMRead(ADDR1);
    CurrentOutput[1] = EEPROMRead(ADDR2);
    CurrentOutput[2] = EEPROMRead(ADDR3);
    current.sine_freq = CurrentOutput[0];
    current.pulse_freq = CurrentOutput[1];
    current.state = (INT8U)(CurrentOutput[2] & STATE_MASK);
    current.pulse_level = (INT8U)(CurrentOutput[2] & PULSE_L_MASK);
    current.sine_level = (INT8U)(CurrentOutput[2] & SINE_L_MASK);
    //validate stuff
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
