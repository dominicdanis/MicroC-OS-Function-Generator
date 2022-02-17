/*******************************************************************************
 * uCOS TSI Module
 * Contains single task, which pends on a time delay of 5ms.
 * This module is not complete. Code must be written
 * to post a semaphore only when there is a sensor state change.
 * 02/17/2022 Aili Emory
 *******************************************************************************/                                                                                                                                                                            #ifndef K65TWR_TSI_H_
#define K65TWR_TSI_H_

#define BRD_PAD1_CH  12U
#define BRD_PAD2_CH  11U

INT16U tsiPend(INT16U tout, OS_ERR *os_err);    /* Pend on sensor press*/
                                                /* tout - semaphore timeout           */
                                                /* err - destination of err code     */
                                                /* Error codes are identical to a semaphore */

void tsiInit(void); /* TSI Initialization*/

#endif

