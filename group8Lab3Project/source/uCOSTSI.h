/*******************************************************************************
* uCOS TSI Module
* Implements capacitive touch sensing.
* There is calibration of the sensors. When a sensor is touched, a flag is set, in a flag group
* and OSFlagPost() is called. The flag is only set, at the start of a sensor press. The current
* sensor state and last sensor state are compared to determine the edge of the sensor press start.
*
* 02/17/2022 Aili Emory
* Includes functions by Todd Morton in Flag notes and TSI notes
*******************************************************************************/
#ifndef UCOSTSI_H_
#define UCOSTSI_H_

#define K65TWR_TSI_H_
#define BRD_PAD1_CH  12U
#define BRD_PAD2_CH  11U
/********************************************************************
* TSIPend(): TSIPend provides access to the TSI buffer via an
*            Event flag. Returns value of sensor flag variable and clears it to
*            receive sensor press only one time.
* - Public
* 02/17/2022 Aili Emory
********************************************************************/
OS_FLAGS TSIPend(OS_TICK tout, OS_ERR *os_err);
/********************************************************************************
 * TSIInit: Initializes TSI0 module
 * -Public
 * 02/17/2022 Aili Emory
 ********************************************************************************/
void TSIInit(void); /* TSI Initialization*/

#endif /*UCOSTSI_H_*/

