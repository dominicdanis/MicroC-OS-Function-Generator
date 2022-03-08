/*****************************************************************************************
 * DMA Module
 *
 * 02/18/2022 Nick Coyle, Aili Emory, Dominic Danis
 * Includes functions by Todd Morton in DMA notes
 *****************************************************************************************/

#include "MCUType.h"
#include "app_cfg.h"
#include "os.h"
#include "K65TWR_GPIO.h"
#include "DMA.h"

#define DMA_CH            0
#define SIZE_CODE_16BIT   001

typedef struct{
    INT8U index;
    OS_SEM flag;
}DMA_BLOCK_RDY;

/*******************************************************************************************
* Public Functions Declarations
*******************************************************************************************/
void DMA0_DMA16_IRQHandler(void);

/*******************************************************************************************
* Private Functions Declarations
*******************************************************************************************/
static void DAC0Init(void);  /* Initialize the Digital to Analog Converter */
static void PitInit(void);	 /* Initialize the Periodic Interrupt Timer */

/****************************************************************************************
* Private Resources
****************************************************************************************/
static DMA_BLOCK_RDY dmaBlockRdy;

/*******************************************************************************************
* Ping Pong Buffer (Private)
*******************************************************************************************/
// This is the Ping Pong Buffer, full double-buffering (ping-pong).
// Should only be accessible externally via DMAFillBuffer()
INT16U dmaBuffer[NUM_BLOCKS][SAMPLES_PER_BLOCK];

/*******************************************************************************************
* DMAFillBuffer
* Fills the DMA ping pong buffer
*******************************************************************************************/
void DMAFillBuffer(INT8U index, INT16U *samples) {
	for(int i=0; i<SAMPLES_PER_BLOCK;i++) {
		dmaBuffer[index][i] = *samples;
		samples++;
	}
}

/*******************************************************************************************
DMAInInit
    Initializes DMA for an input stream from ADC0 to ping-pong buffers
    Parameters: none
    Return: none
*******************************************************************************************/
void DMAInit(void){
    OS_ERR os_err;

    OSSemCreate(&(dmaBlockRdy.flag), "Block Ready", 0, &os_err);

    // dmaBlockRdy.index indicates the buffer currently not being used by the DMA in the Ping-Pong scheme.
    // It uses the DONE bit in the CSR to determine where the DMA is at. If DONE is 1, the DMA just finished
    // the [1] block and will start taking from the [0] block. So, when DONE is one, we want Sinegen to use
    // the [1] block to avoid collisions with the DMA.
    // Since the DMA starts with the [0] block, initialize the index to the [1] block.

    dmaBlockRdy.index = 1;

    //enable DMA clocks
    SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
    SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;

    //Make sure DMAMUX is disabled
    DMAMUX->CHCFG[DMA_CH] |= DMAMUX_CHCFG_ENBL(0)|DMAMUX_CHCFG_TRIG(0);

    //source address is the lookup table, DMABuffer, the ping pong buffer in this case
    DMA0->TCD[DMA_CH].SADDR = DMA_SADDR_SADDR(dmaBuffer);

    //Source data is 16-bits
    DMA0->TCD[DMA_CH].ATTR = DMA_ATTR_SMOD(0) | DMA_ATTR_SSIZE(SIZE_CODE_16BIT) | DMA_ATTR_DMOD(0) | DMA_ATTR_DSIZE(SIZE_CODE_16BIT);

    //2 byte offset for source data address.
    DMA0->TCD[DMA_CH].SOFF = DMA_SOFF_SOFF(BYTES_PER_SAMPLE);

    //This is the value to be added to the address at the end of a major loop.
    DMA0->TCD[DMA_CH].SLAST = DMA_SLAST_SLAST(-(BYTES_PER_BUFFER));

    //The destination address, DADDR, is &DAC0->DAT[0].DATL and the offset is
    //0 because the transfer always goes to the same location
    DMA0->TCD[DMA_CH].DADDR = DMA_DADDR_DADDR(&DAC0->DAT[0].DATL);
    DMA0->TCD[DMA_CH].DOFF = DMA_DOFF_DOFF(0);

    //This is the value to be added to the destination address at the end of a major loop.
    // Since this is the DAC address, it never changes so the offset is set to 0.
    DMA0->TCD[DMA_CH].DLAST_SGA = DMA_DLAST_SGA_DLASTSGA(0);

    //Minor loop size should be set to the sample size since we want one sample transferred
    //with each trigger so set DMA_NBYTES_MLNO_NBYTES to 2
    DMA0->TCD[DMA_CH].NBYTES_MLNO = DMA_NBYTES_MLNO_NBYTES(BYTES_PER_SAMPLE);

    //Now we need to set the number of minor loops in the whole block. In this case, there are
    //SAMPLES_PER_BLOCK samples in the lookup table block so we set DMA_CITER_ELINKNO_CITER and
    //DMA_BITER_ELINKNO_BITER to SAMPLES_PER_BLOCK
    DMA0->TCD[DMA_CH].CITER_ELINKNO = DMA_CITER_ELINKNO_ELINK(0) | DMA_CITER_ELINKNO_CITER(NUM_BLOCKS*SAMPLES_PER_BLOCK);
    DMA0->TCD[DMA_CH].BITER_ELINKNO = DMA_BITER_ELINKNO_ELINK(0) | DMA_BITER_ELINKNO_BITER(NUM_BLOCKS*SAMPLES_PER_BLOCK);

    //we need to set the bits in the CSR register to completely initialize the TCD.
    //Enable interrupt at half filled Tx buffer and end of major loop.
    //This allows "ping-pong" buffer processing.
    DMA0->TCD[DMA_CH].CSR = DMA_CSR_ESG(0) | DMA_CSR_MAJORELINK(0) | DMA_CSR_BWC(3) | DMA_CSR_INTHALF(1) | DMA_CSR_INTMAJOR(1) | DMA_CSR_DREQ(0) | DMA_CSR_START(0);

    //Finally, we enable the DMAMUX, and enable the DMA for the ‘always enabled’ channel 60
    DMAMUX->CHCFG[DMA_CH] = DMAMUX_CHCFG_ENBL(1)|DMAMUX_CHCFG_TRIG(1)|DMAMUX_CHCFG_SOURCE(60);

	//enable DMA interrupt
	NVIC_EnableIRQ(DMA_CH);

	//All set to go, enable DMA channel!
    DMA0->SERQ = DMA_SERQ_SERQ(DMA_CH);

    DAC0Init();
    PitInit();
}

/***********************************************************************
 * DAC0Init - Initialize DAC0 and set output reference (3.3V)
 * Nick Coyle 11/13/2021
 ************************************************************************/
static void DAC0Init(void){
    SIM->SCGC2 |= SIM_SCGC2_DAC0(1);  /* enable DAC clock */
    DAC0->C0 |= DAC_C0_DACEN(1);      /* set bit 7 to enable DAC */
    DAC0->C0 |= DAC_C0_DACRFS(1);     /* set bit 6 for DACREF_1 so VDDA 3.3V ref */
    DAC0->C0 |= DAC_C0_DACSWTRG(1);   /* set bit 1 select hardware trigger */
}

/***********************************************************************
 * PitInit - Initialize the PIT
 * Todd Morton 11/09/2020
 * Nick Coyle 11/13/2021 revised the comments
 * Nick Coyle 02/19/2022 changed period for 48kHz sample rate
 ************************************************************************/
static void PitInit(void){
	//the PIT needs to be configured to generate triggers at the desired sample rate and the DAC
	//must be configured with the DMAEN bit set so the samples come from the DMA
	SIM->SCGC6 |= SIM_SCGC6_PIT(1);    /* turn on PIT clock */
    PIT->MCR = PIT_MCR_MDIS(0);        /* enable PIT clock */
    PIT->CHANNEL[0].LDVAL = 1249;      /* set Tep to 20.083us for fs=48kHz (Buss clock = 60MHz) */
    //PIT->CHANNEL[0].TCTRL = (PIT_TCTRL_TIE(1)|PIT_TCTRL_TEN(1)); /* enable interrupts, start Timer 0 */
    PIT->CHANNEL[0].TCTRL = (PIT_TCTRL_TEN(1)); /* start Timer 0 */
}


/****************************************************************************************
 * DMA Interrupt Handler for the sample stream
 * 08/30/2015 TDM
 ***************************************************************************************/
void DMA0_DMA16_IRQHandler(void){
    OS_ERR os_err;
    OSIntEnter();
    DB2_TURN_ON();
    DMA0->CINT = DMA_CINT_CINT(DMA_CH);

    if((DMA0->TCD[DMA_CH].CSR & DMA_CSR_DONE_MASK) != 0){
        dmaBlockRdy.index = 1;      //set buffer index to opposite of DMA
    }else{
        dmaBlockRdy.index = 0;
    }

    (void)OSSemPost(&(dmaBlockRdy.flag),OS_OPT_POST_1,&os_err);
    DB2_TURN_OFF();
    OSIntExit();
}


/****************************************************************************************
 * DMA Flag
 * The DMA ISR is going to post this semaphore every time it gets halfway or all the way
 * to the end of the ping pong buffer. The sinegen buffer filling task will call this to
 * get the current index it should be writing to.
 * 08/30/2015 TDM
 ***************************************************************************************/
INT8U DMAReadyPend(OS_TICK tout, OS_ERR *os_err_ptr){
    (void)OSSemPend(&(dmaBlockRdy.flag), tout, OS_OPT_PEND_BLOCKING,(void *)0, os_err_ptr);
    return dmaBlockRdy.index;
}
