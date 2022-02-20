/*****************************************************************************************
 * DMA Module
 *
 * 02/18/2022 Nick Coyle, Aili Emory, Dominic Danis
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
void DMA2_DMA18_IRQHandler(void);
void PIT0_IRQHandler(void); /* PIT interrupt service routine*/

/*******************************************************************************************
* Private Functions Declarations
*******************************************************************************************/
DMA_BLOCK_RDY dmaBlockRdy;

static void DAC0Init(void);  /* Init the DAC */
static void PitInit(void);

/*******************************************************************************************
* Global Variables
*******************************************************************************************/
/* Right now these are global buffers so the processing module can access them. Should
 * replace with a function. TDM 08/30/2015
 */
// The following structure matches the DMA order, and is sized to implement
// full double-buffering (ping-pong).
INT16U DMABuffer[1][SAMPLES_PER_BLOCK] =  {{2047,
		2059,
		2072,
		2084,
		2097,
		2109,
		2122,
		2134,
		2147,
		2160,
		2172,
		2185,
		2197,
		2210,
		2222,
		2235,
		2247,
		2260,
		2272,
		2285,
		2297,
		2310,
		2322,
		2335,
		2347,
		2359,
		2372,
		2384,
		2397,
		2409,
		2421,
		2434,
		2446,
		2458,
		2471,
		2483,
		2495,
		2507,
		2520,
		2532,
		2544,
		2556,
		2568,
		2581,
		2593,
		2605,
		2617,
		2629,
		2641,
		2653,
		2665,
		2677,
		2689,
		2701,
		2713,
		2725,
		2736,
		2748,
		2760,
		2772,
		2784,
		2795,
		2807,
		2819,
		2830,
		2842,
		2853,
		2865,
		2876,
		2888,
		2899,
		2911,
		2922,
		2933,
		2945,
		2956,
		2967,
		2979,
		2990,
		3001,
		3012,
		3023,
		3034,
		3045,
		3056,
		3067,
		3078,
		3089,
		3099,
		3110,
		3121,
		3132,
		3142,
		3153,
		3163,
		3174,
		3184,
		3195,
		3205,
		3215,
		3226,
		3236,
		3246,
		3256,
		3266,
		3277,
		3287,
		3297,
		3306,
		3316,
		3326,
		3336,
		3346,
		3355,
		3365,
		3375,
		3384,
		3394,
		3403,
		3413,
		3422,
		3431,
		3440,
		3450,
		3459,
		3468,
		3477,
		3486,
		3495,
		3504,
		3512,
		3521,
		3530,
		3538,
		3547,
		3556,
		3564,
		3572,
		3581,
		3589,
		3597,
		3605,
		3614,
		3622,
		3630,
		3638,
		3645,
		3653,
		3661,
		3669,
		3676,
		3684,
		3691,
		3699,
		3706,
		3714,
		3721,
		3728,
		3735,
		3742,
		3749,
		3756,
		3763,
		3770,
		3777,
		3783,
		3790,
		3797,
		3803,
		3810,
		3816,
		3822,
		3828,
		3835,
		3841,
		3847,
		3853,
		3859,
		3864,
		3870,
		3876,
		3881,
		3887,
		3892,
		3898,
		3903,
		3908,
		3914,
		3919,
		3924,
		3929,
		3934,
		3939,
		3943,
		3948,
		3953,
		3957,
		3962,
		3966,
		3971,
		3975,
		3979,
		3983,
		3987,
		3991,
		3995,
		3999,
		4003,
		4006,
		4010,
		4013,
		4017,
		4020,
		4024,
		4027,
		4030,
		4033,
		4036,
		4039,
		4042,
		4045,
		4047,
		4050,
		4053,
		4055,
		4058,
		4060,
		4062,
		4064,
		4066,
		4068,
		4070,
		4072,
		4074,
		4076,
		4078,
		4079,
		4081,
		4082,
		4083,
		4085,
		4086,
		4087,
		4088,
		4089,
		4090,
		4091,
		4091,
		4092,
		4093,
		4093,
		4094,
		4094,
		4094,
		4094,
		4094,
		4095,
		4094,
		4094,
		4094,
		4094,
		4094,
		4093,
		4093,
		4092,
		4091,
		4091,
		4090,
		4089,
		4088,
		4087,
		4086,
		4085,
		4083,
		4082,
		4081,
		4079,
		4078,
		4076,
		4074,
		4072,
		4070,
		4068,
		4066,
		4064,
		4062,
		4060,
		4058,
		4055,
		4053,
		4050,
		4047,
		4045,
		4042,
		4039,
		4036,
		4033,
		4030,
		4027,
		4024,
		4020,
		4017,
		4013,
		4010,
		4006,
		4003,
		3999,
		3995,
		3991,
		3987,
		3983,
		3979,
		3975,
		3971,
		3966,
		3962,
		3957,
		3953,
		3948,
		3943,
		3939,
		3934,
		3929,
		3924,
		3919,
		3914,
		3908,
		3903,
		3898,
		3892,
		3887,
		3881,
		3876,
		3870,
		3864,
		3859,
		3853,
		3847,
		3841,
		3835,
		3828,
		3822,
		3816,
		3810,
		3803,
		3797,
		3790,
		3783,
		3777,
		3770,
		3763,
		3756,
		3749,
		3742,
		3735,
		3728,
		3721,
		3714,
		3706,
		3699,
		3691,
		3684,
		3676,
		3669,
		3661,
		3653,
		3645,
		3638,
		3630,
		3622,
		3614,
		3605,
		3597,
		3589,
		3581,
		3572,
		3564,
		3556,
		3547,
		3538,
		3530,
		3521,
		3512,
		3504,
		3495,
		3486,
		3477,
		3468,
		3459,
		3450,
		3440,
		3431,
		3422,
		3413,
		3403,
		3394,
		3384,
		3375,
		3365,
		3355,
		3346,
		3336,
		3326,
		3316,
		3306,
		3297,
		3287,
		3277,
		3266,
		3256,
		3246,
		3236,
		3226,
		3215,
		3205,
		3195,
		3184,
		3174,
		3163,
		3153,
		3142,
		3132,
		3121,
		3110,
		3099,
		3089,
		3078,
		3067,
		3056,
		3045,
		3034,
		3023,
		3012,
		3001,
		2990,
		2979,
		2967,
		2956,
		2945,
		2933,
		2922,
		2911,
		2899,
		2888,
		2876,
		2865,
		2853,
		2842,
		2830,
		2819,
		2807,
		2795,
		2784,
		2772,
		2760,
		2748,
		2736,
		2725,
		2713,
		2701,
		2689,
		2677,
		2665,
		2653,
		2641,
		2629,
		2617,
		2605,
		2593,
		2581,
		2568,
		2556,
		2544,
		2532,
		2520,
		2507,
		2495,
		2483,
		2471,
		2458,
		2446,
		2434,
		2421,
		2409,
		2397,
		2384,
		2372,
		2359,
		2347,
		2335,
		2322,
		2310,
		2297,
		2285,
		2272,
		2260,
		2247,
		2235,
		2222,
		2210,
		2197,
		2185,
		2172,
		2160,
		2147,
		2134,
		2122,
		2109,
		2097,
		2084,
		2072,
		2059,
		2047,
		2034,
		2021,
		2009,
		1996,
		1984,
		1971,
		1959,
		1946,
		1933,
		1921,
		1908,
		1896,
		1883,
		1871,
		1858,
		1846,
		1833,
		1821,
		1808,
		1796,
		1783,
		1771,
		1758,
		1746,
		1734,
		1721,
		1709,
		1696,
		1684,
		1672,
		1659,
		1647,
		1635,
		1622,
		1610,
		1598,
		1586,
		1573,
		1561,
		1549,
		1537,
		1525,
		1512,
		1500,
		1488,
		1476,
		1464,
		1452,
		1440,
		1428,
		1416,
		1404,
		1392,
		1380,
		1368,
		1357,
		1345,
		1333,
		1321,
		1309,
		1298,
		1286,
		1274,
		1263,
		1251,
		1240,
		1228,
		1217,
		1205,
		1194,
		1182,
		1171,
		1160,
		1148,
		1137,
		1126,
		1114,
		1103,
		1092,
		1081,
		1070,
		1059,
		1048,
		1037,
		1026,
		1015,
		1004,
		994,
		983,
		972,
		961,
		951,
		940,
		930,
		919,
		909,
		898,
		888,
		878,
		867,
		857,
		847,
		837,
		827,
		816,
		806,
		796,
		787,
		777,
		767,
		757,
		747,
		738,
		728,
		718,
		709,
		699,
		690,
		680,
		671,
		662,
		653,
		643,
		634,
		625,
		616,
		607,
		598,
		589,
		581,
		572,
		563,
		555,
		546,
		537,
		529,
		521,
		512,
		504,
		496,
		488,
		479,
		471,
		463,
		455,
		448,
		440,
		432,
		424,
		417,
		409,
		402,
		394,
		387,
		379,
		372,
		365,
		358,
		351,
		344,
		337,
		330,
		323,
		316,
		310,
		303,
		296,
		290,
		283,
		277,
		271,
		265,
		258,
		252,
		246,
		240,
		234,
		229,
		223,
		217,
		212,
		206,
		201,
		195,
		190,
		185,
		179,
		174,
		169,
		164,
		159,
		154,
		150,
		145,
		140,
		136,
		131,
		127,
		122,
		118,
		114,
		110,
		106,
		102,
		98,
		94,
		90,
		87,
		83,
		80,
		76,
		73,
		69,
		66,
		63,
		60,
		57,
		54,
		51,
		48,
		46,
		43,
		40,
		38,
		35,
		33,
		31,
		29,
		27,
		25,
		23,
		21,
		19,
		17,
		15,
		14,
		12,
		11,
		10,
		8,
		7,
		6,
		5,
		4,
		3,
		2,
		2,
		1,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		1,
		2,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		10,
		11,
		12,
		14,
		15,
		17,
		19,
		21,
		23,
		25,
		27,
		29,
		31,
		33,
		35,
		38,
		40,
		43,
		46,
		48,
		51,
		54,
		57,
		60,
		63,
		66,
		69,
		73,
		76,
		80,
		83,
		87,
		90,
		94,
		98,
		102,
		106,
		110,
		114,
		118,
		122,
		127,
		131,
		136,
		140,
		145,
		150,
		154,
		159,
		164,
		169,
		174,
		179,
		185,
		190,
		195,
		201,
		206,
		212,
		217,
		223,
		229,
		234,
		240,
		246,
		252,
		258,
		265,
		271,
		277,
		283,
		290,
		296,
		303,
		310,
		316,
		323,
		330,
		337,
		344,
		351,
		358,
		365,
		372,
		379,
		387,
		394,
		402,
		409,
		417,
		424,
		432,
		440,
		448,
		455,
		463,
		471,
		479,
		488,
		496,
		504,
		512,
		521,
		529,
		537,
		546,
		555,
		563,
		572,
		581,
		589,
		598,
		607,
		616,
		625,
		634,
		643,
		653,
		662,
		671,
		680,
		690,
		699,
		709,
		718,
		728,
		738,
		747,
		757,
		767,
		777,
		787,
		796,
		806,
		816,
		827,
		837,
		847,
		857,
		867,
		878,
		888,
		898,
		909,
		919,
		930,
		940,
		951,
		961,
		972,
		983,
		994,
		1004,
		1015,
		1026,
		1037,
		1048,
		1059,
		1070,
		1081,
		1092,
		1103,
		1114,
		1126,
		1137,
		1148,
		1160,
		1171,
		1182,
		1194,
		1205,
		1217,
		1228,
		1240,
		1251,
		1263,
		1274,
		1286,
		1298,
		1309,
		1321,
		1333,
		1345,
		1357,
		1368,
		1380,
		1392,
		1404,
		1416,
		1428,
		1440,
		1452,
		1464,
		1476,
		1488,
		1500,
		1512,
		1525,
		1537,
		1549,
		1561,
		1573,
		1586,
		1598,
		1610,
		1622,
		1635,
		1647,
		1659,
		1672,
		1684,
		1696,
		1709,
		1721,
		1734,
		1746,
		1758,
		1771,
		1783,
		1796,
		1808,
		1821,
		1833,
		1846,
		1858,
		1871,
		1883,
		1896,
		1908,
		1921,
		1933,
		1946,
		1959,
		1971,
		1984,
		1996,
		2009,
		2021,
		2034
}};

/*******************************************************************************************
* Function Code
********************************************************************************************
DMAInInit
    Initializes DMA for an input stream from ADC0 to ping-pong buffers
    Parameters: none
    Return: none
*******************************************************************************************/
void DMAInit(void){
    OS_ERR os_err;

    OSSemCreate(&dmaBlockRdy.flag, "Block Ready", 0, &os_err);

    // dmaBlockRdy.index indicates the buffer currently not being used by the DMA in the Ping-Pong scheme.
    // It uses the DONE bit in the CSR to determine where the DMA is at. If DONE is 1, the DMA just finished
    // the [1] block and will start filling the [0] block. So, when DONE is one, we want the processing to use
    // the [1] block to avoid collisions with the DMA.
    // Since the DMA starts with the [0] block, initialize the index to the [1] block.

    dmaBlockRdy.index = 1;

    //enable DMA clocks
    SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
    SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;

    //Make sure DMAMUX is disabled
    DMAMUX->CHCFG[DMA_CH] |= DMAMUX_CHCFG_ENBL(0)|DMAMUX_CHCFG_TRIG(0);

    //source address is the lookup table, DMABuffer, the ping pong buffer in this case
    DMA0->TCD[DMA_CH].SADDR = DMA_SADDR_SADDR(DMABuffer);

    //Source data is 16-bits
    DMA0->TCD[DMA_CH].ATTR = DMA_ATTR_SMOD(0) | DMA_ATTR_SSIZE(SIZE_CODE_16BIT) | DMA_ATTR_DMOD(0) | DMA_ATTR_DSIZE(SIZE_CODE_16BIT);

    //No offset for source data address.
    DMA0->TCD[DMA_CH].SOFF = DMA_SOFF_SOFF(BYTES_PER_SAMPLE);

    //This is the value to be added to the address at the end of a major loop.
    DMA0->TCD[DMA_CH].SLAST = DMA_SLAST_SLAST(-(BYTES_PER_BLOCK));

    //the destination address, DADDR, is &DAC0->DAT[0].DATL and the offset is
    //0 because the transfer always goes to the same location
    DMA0->TCD[DMA_CH].DADDR = DMA_DADDR_DADDR(&DAC0->DAT[0].DATL);
    DMA0->TCD[DMA_CH].DOFF = DMA_DOFF_DOFF(0);

    //This is the value to be added to the address at the end of a major loop. Since this is the
    //DAC address, it never changes so the offset is set to 0.
    DMA0->TCD[DMA_CH].DLAST_SGA = DMA_DLAST_SGA_DLASTSGA(0);

    //Minor loop size should be set to the sample size since we want one sample transferred
    //with each trigger so set DMA_NBYTES_MLNO_NBYTES to 2
    DMA0->TCD[DMA_CH].NBYTES_MLNO = DMA_NBYTES_MLNO_NBYTES(BYTES_PER_SAMPLE);

    //Now we need to set the number of minor loops in the whole block. In this case, there are
    //64 samples in the lookup table so we set DMA_CITER_ELINKNO_CITER and
    //DMA_BITER_ELINKNO_BITER to 64
    DMA0->TCD[DMA_CH].CITER_ELINKNO = DMA_CITER_ELINKNO_ELINK(0) | DMA_CITER_ELINKNO_CITER(SAMPLES_PER_BLOCK);
    DMA0->TCD[DMA_CH].BITER_ELINKNO = DMA_BITER_ELINKNO_ELINK(0) | DMA_BITER_ELINKNO_BITER(SAMPLES_PER_BLOCK);

    //we need to set the bits in the CSR register to completely initialize the TCD. This
    //line will do this assuming that we are not using interrupts etc.
    DMA0->TCD[DMA_CH].CSR = DMA_CSR_ESG(0) | DMA_CSR_MAJORELINK(0) | DMA_CSR_BWC(3) | DMA_CSR_INTHALF(0) | DMA_CSR_INTMAJOR(0) | DMA_CSR_DREQ(0) | DMA_CSR_START(0);

    //Finally, we enable the DMAMUX, and enable the DMA for the ‘always enabled’ channel 60
    DMAMUX->CHCFG[DMA_CH] = DMAMUX_CHCFG_ENBL(1)|DMAMUX_CHCFG_TRIG(1)|DMAMUX_CHCFG_SOURCE(60);

	//All set to go, enable DMA channel(s)!
    DMA0->SERQ = DMA_SERQ_SERQ(DMA_CH);

    DAC0Init();
    PitInit();
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
    PIT->CHANNEL[0].TCTRL = (PIT_TCTRL_TIE(1)|PIT_TCTRL_TEN(1)); /* enable interrupts, start Timer 0 */
}

/***********************************************************************
 * DAC0Init - Initialize DAC0 and set output reference (3.3V)
 * Nick Coyle 11/13/2021
 ************************************************************************/
static void DAC0Init(void){
    SIM->SCGC2 |= SIM_SCGC2_DAC0(1);  /* enable DAC clock */
    DAC0->C0 |= DAC_C0_DACEN(1);      /* set bit 7 to enable DAC */
    DAC0->C0 |= DAC_C0_DACRFS(1);     /* set bit 6 for DACREF_1 so VDDA 3.3V ref */
    DAC0->C0 |= DAC_C0_DACSWTRG(1);   /* set bit 5 select software trigger */
}

/****************************************************************************************
 * DMA Interrupt Handler for the sample stream
 * 08/30/2015 TDM
 ***************************************************************************************/
void DMA2_DMA18_IRQHandler(void){
    OS_ERR os_err;
    OSIntEnter();
    DB2_TURN_ON();
    DMA0->CINT = DMA_CINT_CINT(DMA_CH);
    if((DMA0->TCD[DMA_CH].CSR & DMA_CSR_DONE_MASK) != 0){
        dmaBlockRdy.index = 1;      //set buffer index to opposite of DMA
    }else{
        dmaBlockRdy.index = 0;
    }
    OSSemPost(&(dmaBlockRdy.flag),OS_OPT_POST_1,&os_err);
    DB2_TURN_OFF();
    OSIntExit();
}
/****************************************************************************************
 * DMA Flag
 * 08/30/2015 TDM
 ***************************************************************************************/
INT8U DMAPend(OS_TICK tout, OS_ERR *os_err_ptr){
    OSSemPend(&(dmaBlockRdy.flag), tout, OS_OPT_PEND_BLOCKING,(void *)0, os_err_ptr);
    return dmaBlockRdy.index;
}
