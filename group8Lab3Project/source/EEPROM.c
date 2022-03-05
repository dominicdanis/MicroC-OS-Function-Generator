/*******************************************************************************
* EECE444 Lab 3 Code
* EEPROM.c is a module that will read and write to a 93LC56B EEPROM using SPI
*
* 02/24/2022 Nick Coyle, Aili Emory, Dominic Danis
*
* Includes functions by Todd Morton in SPI notes
*******************************************************************************/
#include "MCUType.h"               /* Include header files                    */
#include "EEPROM.h"
#include "MemoryTools.h"
#include "os.h"
/*Defined Constants for EEPROM Commands*/
#define EWEN 0x04C0
#define EWDS 0x0400
#define ERAL 0x0480
/*Private functions*/
static void EEPROMCmd(INT16U cmd);
static INT16U EEPROMRead(INT8U addr);
static void EEPROMWrite(INT8U addr, INT16U wr_data);
static INT16U EEPROMXfr16(INT32U pushr);
static void EEPROMSaveConfig(void);
/*Union for access by structure or by INT16U blocks*/
typedef union{
    SAVED_CONFIG Config;
    INT16U ConfigArr[(sizeof(SAVED_CONFIG)+1)/2];
} EEPROMBLOCK;
/*Locally stored EEPROMBLOCK*/
static EEPROMBLOCK EEPROMCurrent;
/*****************************************************************************************
* EEPROMInit
* Function to initialize SPI from communication with 93LC56B EEPROM
* SPI is mapped to PTD11-14
* 3/3/2022
 *****************************************************************************************/
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
/*****************************************************************************************
* EEPROMSaveState
* Updates locally, stored copy of state, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* Dominic Danis 03/03/2022
 *****************************************************************************************/
void EEPROMSaveState(INT8U state){
    EEPROMCurrent.Config.state = state;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}
/*****************************************************************************************
* EEPROMSaveSineFreq
* Updates locally, stored copy of sine_freq, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
void EEPROMSaveSineFreq(INT16U sine_freq){
    EEPROMCurrent.Config.sine_freq = sine_freq;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}
/*****************************************************************************************
* EEPROMSaveSineLevel
* Updates locally, stored copy of sine_level, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
void EEPROMSaveSineLevel(INT8U sine_level){
    EEPROMCurrent.Config.sine_level = sine_level;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}
/*****************************************************************************************
* EEPROMSavePulseFreq
* Updates locally, stored copy of pulse_freq, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
void EEPROMSavePulseFreq(INT16U pulse_freq){
    EEPROMCurrent.Config.pulse_freq = pulse_freq;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}
/*****************************************************************************************
* EEPROMSavePulseLevel
* Updates locally, stored copy of pulse_level, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
void EEPROMSavePulseLevel(INT8U pulse_level){
    EEPROMCurrent.Config.pulse_level = pulse_level;
    EEPROMCurrent.Config.checksum = 0;
    EEPROMCurrent.Config.checksum = MemChkSum((INT8U *)EEPROMCurrent.ConfigArr, (INT8U *)EEPROMCurrent.ConfigArr+(sizeof(SAVED_CONFIG)+1)/2);
    EEPROMSaveConfig();
}
/*****************************************************************************************
* EEPROMSaveConfig
* Writes the current configuration held in EEPROMCurrent to the EEPROM
 *****************************************************************************************/
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
/*****************************************************************************************
* EEPROMGetConfig
* Reads the and loads config from EEPROM into EEPROMCurrent. Calculates checksum of saved values
* and compares to saved checksum. Will return loaded value if they match. Will return default
* configuration if they don't.
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
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
/*****************************************************************************************
* EEEPROMRead
* Sends an instruction to initiate a read on SPI with address to be read from
* Sends dummy data, reads and returns data input on SPI
* From SPI notes Todd Morton
 *****************************************************************************************/
static INT16U EEPROMRead(INT8U addr){
    INT16U rd_value;
    //send command and address
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA((0x6<<8)|addr));
    rd_value = EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(0)|SPI_PUSHR_CTAS(1)|
                      SPI_PUSHR_TXDATA(0x0000));
    return rd_value;
}
/*****************************************************************************************
* EEEPROMWrite
* Sends write command and address to be written to
* Sends data to be written
* From SPI notes Todd Morton
 *****************************************************************************************/
static void EEPROMWrite(INT8U addr, INT16U wr_data){
    //send command and address
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(1)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA((0x5<<8) | (addr & 0x7F)));
    //send data
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(0)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA(wr_data));
}
/*****************************************************************************************
* EEEPROMXfr16
* Transfers given pushr vaues and returns received data on SPI
* From SPI notes Todd Morton
 *****************************************************************************************/
static INT16U EEPROMXfr16(INT32U pushr){
    //Wait for previous transmit and clear flag
    while((SPI2->SR & SPI_SR_TFFF_MASK) == 0){}
    SPI2->SR |= SPI_SR_TFFF_MASK;
    //Initiate transfer
    SPI2->PUSHR = pushr;
    //Wait for received data and clear flag
    while((SPI2->SR & SPI_SR_RFDF_MASK) == 0){}
    SPI2->SR |= SPI_SR_RFDF_MASK;
    //return received data
    return SPI2->POPR;
}
/*****************************************************************************************
* EEEPROMCmd
* Formats given command and sends it to Xfr16 function
* From SPI notes Todd Morton
 *****************************************************************************************/
static void EEPROMCmd(INT16U cmd){
    (void)EEPROMXfr16(SPI_PUSHR_PCS(1)|SPI_PUSHR_CONT(0)|SPI_PUSHR_CTAS(0)|
                      SPI_PUSHR_TXDATA(cmd));
}
