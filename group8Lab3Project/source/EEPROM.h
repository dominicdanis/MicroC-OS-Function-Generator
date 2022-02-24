/*******************************************************************************
* EECE444 Lab 3 Code
*   EEPROM.c is a module that will read and write to a 93LC56B EEPROM using SPI
*
* 02/24/2022 Nick Coyle, Aili Emory, Dominic Danis
*******************************************************************************/

/*******************************************************************************
*   Structure for state, frequency and level. Held in MemoryTools publically
*   because both AppLab3 and MemoryTools will need to use.
*******************************************************************************/
typedef struct{
    INT8U state;
    INT16U sine_freq;
    INT8U sine_level;
    INT16U pulse_freq;
    INT8U pulse_level;
} SAVED_CONFIG;

void EEPROMInit(void);
SAVED_CONFIG EEPROMGetConfig(void);
void EEPROMSaveState(INT8U state);
void EEPROMSaveSineFreq(INT16U sine_freq);
void EEPROMSaveSineLevel(INT8U sine_level);
void EEPROMSavePulseFreq(INT16U pulse_freq);
void EEPROMSavePulseLevel(INT8U pulse_level);
