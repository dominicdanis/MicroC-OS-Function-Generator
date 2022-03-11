/*******************************************************************************
* EECE444 Lab 3 Code
* EEPROM.c is a module that will read and write to a 93LC56B EEPROM using SPI
*
* 02/24/2022 Dominic Danis
*
* Includes functions by Todd Morton in SPI notes
*******************************************************************************/
#ifndef EEPROM_H_
#define EEPROM_H_

/*Structure Definition for all parameters that must be kept track of*/
typedef struct{
    INT8U state;
    INT16U sine_freq;
    INT8U sine_level;
    INT16U pulse_freq;
    INT8U pulse_level;
    INT16U checksum;
} SAVED_CONFIG;

/*Public Functions*/

/*****************************************************************************************
* EEPROMInit
* Function to initialize SPI from communication with 93LC56B EEPROM
* SPI is mapped to PTD11-14
* 3/3/2022
 *****************************************************************************************/
void EEPROMInit(void);
/*****************************************************************************************
* EEPROMGetConfig
* Reads the and loads config from EEPROM into EEPROMCurrent. Calculates checksum of saved values
* and compares to saved checksum. Will return loaded value if they match. Will return default
* configuration if they don't.
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
SAVED_CONFIG EEPROMGetConfig(void);
/*****************************************************************************************
* EEPROMSaveState
* Updates locally, stored copy of state, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* Dominic Danis 03/03/2022
 *****************************************************************************************/
void EEPROMSaveState(INT8U state);
/*****************************************************************************************
* EEPROMSaveSineFreq
* Updates locally, stored copy of sine_freq, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
void EEPROMSaveSineFreq(INT16U sine_freq);
/*****************************************************************************************
* EEPROMSaveSineLevel
* Updates locally, stored copy of sine_level, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
void EEPROMSaveSineLevel(INT8U sine_level);
/*****************************************************************************************
* EEPROMSavePulseFreq
* Updates locally, stored copy of pulse_freq, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
void EEPROMSavePulseFreq(INT16U pulse_freq);
/*****************************************************************************************
* EEPROMSavePulseLevel
* Updates locally, stored copy of pulse_level, calculates a checksum
* and writes entire EEPROMBLOCK to EEPROM
* 03/03/2022 Dominic Danis
 *****************************************************************************************/
void EEPROMSavePulseLevel(INT8U pulse_level);

#endif
