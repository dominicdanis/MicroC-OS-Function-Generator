/*******************************************************************************
 * uCOS TSI Module
 * 02/17/2022 Aili Emory, Nick Coyle, Dominic Danis
 *******************************************************************************/                                                                                                                                                                            #ifndef K65TWR_TSI_H_
#define K65TWR_TSI_H_
#define BRD_PAD1_CH  12U
#define BRD_PAD2_CH  11U

OS_FLAGS TSIPend(OS_TICK tout, OS_ERR *os_err);
void TSIInit(void); /* TSI Initialization*/

#endif

