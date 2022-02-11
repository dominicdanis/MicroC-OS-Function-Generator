/*****************************************************************************************
* EE444 Lab2 Stop Watch Counter Module
* The counter counts in increments of 10ms when the stop watch is enabled.
* Reset sets the counter to 0.
* A periodic timed task loop makes the counter task increment every 10ms using the
* kernel timer.
* Uses a synchronous buffer to contain the counter data and signal changes.
* A mutex protects the counter control data.
* 01/20/2022 Nick Coyle
*****************************************************************************************/

#ifndef SWCOUNTER_H_
#define SWCOUNTER_H_

/*****************************************************************************************
* SWCountPend() - A function to provide access to the count via a semaphore.
*****************************************************************************************/
INT32U SWCountPend(INT16U tout, OS_ERR *os_err);

/*****************************************************************************************
* SWCounterInit() - Initialization routine for the SW counter module
******************************************************************************************/
void SWCounterInit(void);

/******************************************************************************************
* SWCntrCntrlSet() - Stop or start the stopwatch or reset it to 0.
******************************************************************************************/
void SWCntrCntrlSet(INT8U enable, INT8U reset);

#endif /* SWCOUNTER_H_ */
