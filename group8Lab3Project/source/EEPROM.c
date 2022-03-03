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

#define SET_MASK 0xFFFF
#define STATE_MASK 0x20
#define PULSE_L_MASK 0x7C0
#define SINE_L_MASK 0xF800
#define ADDR1 0x1
#define ADDR2 0x20
#define ADDR3 0x40
#define EWEN 0x04C0
#define EWDS 0x0400
#define ERAL 0x0480

static void EEPROMCmd(INT16U cmd);
static INT16U EEPROMRead(INT8U addr);
static void EEPROMWrite(INT8U addr, INT16U wr_data);
static INT16U EEPROMXfr16(INT32U pushr);
static void EEPROMSaveConfig(void);
typedef union{
    SAVED_CONFIG Config;
    INT16U ConfigArr[(sizeof(SAVED_CONFIG)+1)/2];
} EEPROMBLOCK;
EEPROMBLOCK EEPROMCurrent;

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
    EEPROMCurrent.Config.state = state;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}

void EEPROMSaveSineFreq(INT16U sine_freq){
    EEPROMCurrent.Config.sine_freq = sine_freq;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}
void EEPROMSaveSineLevel(INT8U sine_level){
    EEPROMCurrent.Config.sine_level = sine_level;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}
void EEPROMSavePulseFreq(INT16U pulse_freq){
    EEPROMCurrent.Config.pulse_freq = pulse_freq;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}
void EEPROMSavePulseLevel(INT8U pulse_level){
    EEPROMCurrent.Config.pulse_level = pulse_level;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}

void EEPROMSaveConfig(void){
    OS_ERR os_err;
    INT8U addr = 0;
    EEPROMCmd(EWEN);
    for(INT8U increment = 0; increment<((sizeof(SAVED_CONFIG)+1)/2); increment++){          /*Write the entire stored array*/
        EEPROMWrite(addr,EEPROMCurrent.ConfigArr[increment]);
        addr++;
        OSTimeDly(7,OS_OPT_TIME_DLY,&os_err);
    }
    EEPROMCmd(EWDS);
}

SAVED_CONFIG EEPROMGetConfig(void){
    INT8U addr = 0;
    INT16U cs = 0;
    for(INT8U increment = 0; increment<((sizeof(SAVED_CONFIG)+1)/2); increment++){          /*Read the entire array*/
        EEPROMCurrent.ConfigArr[increment] = EEPROMRead(addr);
        addr++;
    }
    cs = EEPROMCurrent.Config.checksum;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    if(cs != EEPROMCurrent.Config.checksum){                                                /*Verify the loaded values agree with loaded checksum*/
        EEPROMCurrent.Config.state = 0;                                                     /*Set to defaults*/
        EEPROMCurrent.Config.sine_freq = 1000;
        EEPROMCurrent.Config.sine_level = 10;
        EEPROMCurrent.Config.pulse_freq = 1000;
        EEPROMCurrent.Config.pulse_level = 10;
    }
    else{}
    return EEPROMCurrent.Config;
}

static INT16U EEPROMRead(INT8U addr){
    INT16U rd_value;
    //send command and address
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA((0x6<<8)|addr));
    rd_value = EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(0)|SPI_PUSHR_CTAS(1)|
                      SPI_PUSHR_TXDATA(0x0000));
    return rd_value;
}

static void EEPROMWrite(INT8U addr, INT16U wr_data){
    //send command and address
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA((0x5<<8) | (addr & 0x7F)));
    //send data
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(0)|SPI_PUSHR_CTAS(0)|
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
