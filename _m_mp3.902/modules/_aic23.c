/*
ћодуль провер€ет работу aic23
*/

#include <NXP\iolpc3130.h>
#include "arm_comm.h"
#include "drv_i2c.h"
#include "tlv320aic23.h"
#include "includes.h"

/** local definitions **/
#define AIC23_ADDR 0x1A

#define UINT_MAX  4.2950e+009


//---------------------AIC23
static unsigned short tlv320aic23_reg[16];
/*
250  * Common Crystals used
251  * 11.2896 Mhz /128 = *88.2k  /192 = 58.8k
252  * 12.0000 Mhz /125 = *96k    /136 = 88.235K
253  * 12.2880 Mhz /128 = *96k    /192 = 64k
254  * 16.9344 Mhz /128 = 132.3k /192 = *88.2k
255  * 18.4320 Mhz /128 = 144k   /192 = *96k
*/
 
/*
259  * Normal BOSR 0-256/2 = 128, 1-384/2 = 192
260  * USB BOSR 0-250/2 = 125, 1-272/2 = 136
261 */

static long bosr_usb_divisor_table[4];
static unsigned short sr_valid_mask[4];


static unsigned char sr_adc_mult_table[16];
static unsigned char sr_dac_mult_table[16];
 

static unsigned long get_score(long adc, long adc_l, long adc_h, long need_adc,long dac, long dac_l, long dac_h, long need_dac)
{
long diff_adc;
long diff_dac;

	if ((adc >= adc_l) && (adc <= adc_h) && (dac >= dac_l) && (dac <= dac_h)) 
	{
		diff_adc = need_adc - adc;
		diff_dac = need_dac - dac;
		return abs(diff_adc) + abs(diff_dac);
	}
		return UINT_MAX;
}


/*
 41  * AIC23 register cache
 42  */

void _AIC23_init()
{
        I2C1_Master_Init(); /*Init I2C interface*/
//init cash
	tlv320aic23_reg[TLV320AIC23_LINVOL]=	0x0097; 	
	tlv320aic23_reg[TLV320AIC23_RINVOL]=	0x0097;
	tlv320aic23_reg[TLV320AIC23_LCHNVOL]=	0x00F9;
	tlv320aic23_reg[TLV320AIC23_RCHNVOL]=	0x00F9;
	tlv320aic23_reg[TLV320AIC23_ANLG]=	0x001A;
	tlv320aic23_reg[TLV320AIC23_DIGT]=	0x0004;
	tlv320aic23_reg[TLV320AIC23_PWR]=	0x0007;
	tlv320aic23_reg[TLV320AIC23_DIGT_FMT]=	0x0001;
	tlv320aic23_reg[TLV320AIC23_SRATE]=	0x0020;
	tlv320aic23_reg[TLV320AIC23_ACTIVE]=	0x0000;
	tlv320aic23_reg[TLV320AIC23_RESET]=	0x0000;	
	tlv320aic23_reg[11]=			0x0000;
	tlv320aic23_reg[12]=			0x0000;
	tlv320aic23_reg[13]=			0x0000;
	tlv320aic23_reg[14]=			0x0000;
	tlv320aic23_reg[15]=			0x0000;
	
	
	
	bosr_usb_divisor_table[0]=93750; 
	bosr_usb_divisor_table[1]=96000; 
	bosr_usb_divisor_table[2]=62500; 
	bosr_usb_divisor_table[3]=88235;


	sr_valid_mask[0] = LOWER_GROUP|UPPER_GROUP;
	sr_valid_mask[1] = LOWER_GROUP;
	sr_valid_mask[2] = LOWER_GROUP|UPPER_GROUP;
	sr_valid_mask[3] = UPPER_GROUP;
	
 
	sr_adc_mult_table[0]=66;
	sr_adc_mult_table[1]=66;
	sr_adc_mult_table[2]=11;
	sr_adc_mult_table[3]=11;
	sr_adc_mult_table[4]=0;
	sr_adc_mult_table[5]=0;
	sr_adc_mult_table[6]=44;
	sr_adc_mult_table[7]=132;
	sr_adc_mult_table[8]=66;
	sr_adc_mult_table[9]=66;
	sr_adc_mult_table[10]=12;
	sr_adc_mult_table[11]=12;
	sr_adc_mult_table[12]=0;
	sr_adc_mult_table[13]=0;
	sr_adc_mult_table[14]=0;
	sr_adc_mult_table[15]=132;
	
	sr_dac_mult_table[0]=66;
	sr_dac_mult_table[1]=11;
	sr_dac_mult_table[2]=66;
	sr_dac_mult_table[3]=11;
	sr_dac_mult_table[4]=0;
	sr_dac_mult_table[5]=0;
	sr_dac_mult_table[6]=44;
	sr_dac_mult_table[7]=132;
	sr_dac_mult_table[8]=66;
	sr_dac_mult_table[9]=12;
	sr_dac_mult_table[10]=66;
	sr_dac_mult_table[11]=12;
	sr_dac_mult_table[12]=0;
	sr_dac_mult_table[13]=0;
	sr_dac_mult_table[14]=0;
	sr_dac_mult_table[15]=132;
}


/*
 51  * read tlv320aic23 register cache
 52  */
static unsigned short tlv320aic23_read_reg_cache(unsigned short reg)
{

	if (reg >= sizeof(tlv320aic23_reg))
		return -1;
    return tlv320aic23_reg[reg];
}

/*
 63  * write tlv320aic23 register cache
*/
static void tlv320aic23_write_reg_cache(unsigned short reg, unsigned short value)
{

	if (reg >= sizeof(tlv320aic23_reg))
		return;
	tlv320aic23_reg[reg] = value;
}

/*
 75  * write to the tlv320aic23 register space
*/
static unsigned short tlv320aic23_write(unsigned short reg, unsigned short value)
{
 
unsigned char data[2];
t_i2c_mtfr_cfg transfer;
 
       /* TLV320AIC23 has 7 bit address and 9 bits of data
 84          * so we need to switch one data bit into reg and rest
 85          * of data into val
         */
 
	if (reg > 9 && reg != 15) 
	{
		return -1;
	}
 
	data[0] = (reg << 1) | (value >> 8 & 0x01);
        data[1] = value & 0xff;
 
        tlv320aic23_write_reg_cache(reg, value);
        
        transfer.slv_addr = AIC23_ADDR;
        transfer.mode = mode_tx;
        transfer.txsize = 2;
        transfer.txdata = (unsigned char *)data;
        if(done !=  I2C1_Master_Transfer(&transfer)) 
          I2C1_Master_Init();
         		 	
 
    return 0;
}


static unsigned short tlv320aic23_set_bias_level(unsigned short level)
{
unsigned short reg;
	reg = tlv320aic23_read_reg_cache(TLV320AIC23_PWR) & 0xff7f;

    switch (level) 
    {
    case SND_SOC_BIAS_ON:
/* vref/mid, osc on, dac unmute */
		reg &= ~(TLV320AIC23_DEVICE_PWR_OFF | TLV320AIC23_OSC_OFF | TLV320AIC23_DAC_OFF);
		tlv320aic23_write(TLV320AIC23_PWR, reg);
	break;
	case SND_SOC_BIAS_PREPARE:
	break;
	case SND_SOC_BIAS_STANDBY:
/* everything off except vref/vmid, */
	tlv320aic23_write(TLV320AIC23_PWR, reg | TLV320AIC23_CLK_OFF);
	break;
	case SND_SOC_BIAS_OFF:
	/* everything off, dac mute, inactive */
		tlv320aic23_write(TLV320AIC23_ACTIVE, 0x0);
		tlv320aic23_write(TLV320AIC23_PWR, 0xffff);
	break;
	}

	return 0;
}

static unsigned short tlv320aic23_set_dai_fmt(void)
{
unsigned short iface_reg;
 
	iface_reg = tlv320aic23_read_reg_cache(TLV320AIC23_DIGT_FMT) & (~0x03);
/* set master/slave audio interface */
	//iface_reg |= TLV320AIC23_MS_MASTER;
/* interface format */	
#if 1	
	iface_reg |= TLV320AIC23_FOR_I2S; //I2S
#else	
	//iface_reg |= TLV320AIC23_LRP_ON;
	iface_reg |= TLV320AIC23_FOR_DSP | TLV320AIC23_LRP_ON;/* | TLV320AIC23_IWL_32;*/
#endif	
	

	tlv320aic23_write(TLV320AIC23_DIGT_FMT, iface_reg);

    return 0;
}

//#pragma CODE_SECTION(find_rate, ".funcs")
static long find_rate(unsigned short mclk, unsigned long need_adc, unsigned long need_dac)
{
long i, j;
long best_i;
long best_j;
long best_div = 0;
unsigned long best_score;
long adc_l, adc_h, dac_l, dac_h;
long base;
long mask;
long adc;
long dac;
long score;

	best_i = -1;
	best_j = -1;
 	best_score = UINT_MAX;
 	adc=0x0000;
 	dac=0x0000;
 	
	need_adc *= SR_MULT;
	need_dac *= SR_MULT;
/*
311          * rates given are +/- 1/32
312          */
	adc_l = need_adc - (need_adc >> 5);
	adc_h = need_adc + (need_adc >> 5);
	dac_l = need_dac - (need_dac >> 5);
	dac_h = need_dac + (need_dac >> 5);
	for (i = 0; i < sizeof(bosr_usb_divisor_table); i++) 
	{
	
		base = bosr_usb_divisor_table[i];
		mask = sr_valid_mask[i];
		for (j = 0; j < sizeof(sr_adc_mult_table); j++, mask >>= 1) 
		{		
			if ((mask & 1) == 0)
				continue;
			adc = base * sr_adc_mult_table[j];
			dac = base * sr_dac_mult_table[j];			
    		score = get_score(adc, adc_l, adc_h, need_adc,dac, dac_l, dac_h, need_dac);
			if (best_score > score) 
			{
				best_score = score;
				best_i = i;
				best_j = j;
				best_div = 0;
			}
			score = get_score((adc >> 1), adc_l, adc_h, need_adc,(dac >> 1), dac_l, dac_h, need_dac);
/* prefer to have a /2 */
			if ((score != UINT_MAX) && (best_score >= score)) 
			{
				best_score = score;
				best_i = i;
				best_j = j;
				best_div = 1;
			}
		}
	}
	return (best_j << 2) | best_i | (best_div << TLV320AIC23_CLKIN_SHIFT);
}


static unsigned short set_sample_rate_control(unsigned long mclk,unsigned long sample_rate_adc, unsigned long sample_rate_dac)
{
/* Search for the right sample rate */
long data;

	data = find_rate(mclk, sample_rate_adc, sample_rate_dac);
	if (data < 0) 
	{
		return -1;
	}
	tlv320aic23_write(TLV320AIC23_SRATE, data );
	return 0;
	
}




unsigned short tlv320aic23_probe(unsigned short sample_rate)
{
	unsigned short reg;
        
        _AIC23_init();
        
        tlv320aic23_write(TLV320AIC23_ACTIVE, 0x0);
        for(volatile int i = 0; 10000 > i; i++);
/* Reset codec */
	tlv320aic23_write(TLV320AIC23_RESET, 0);
        for(volatile int i = 0; 10000 > i; i++);
	
/* power on device */
	tlv320aic23_set_bias_level(SND_SOC_BIAS_ON);
        for(volatile int i = 0; 10000 > i; i++);
	tlv320aic23_write(TLV320AIC23_DIGT, TLV320AIC23_ADCHP_ON);
        for(volatile int i = 0; 10000 > i; i++);

/* Unmute input */
	reg = tlv320aic23_read_reg_cache(TLV320AIC23_LINVOL);
	tlv320aic23_write(TLV320AIC23_LINVOL,(reg & (~TLV320AIC23_LIM_MUTED)) |(TLV320AIC23_LRS_ENABLED));
        for(volatile int i = 0; 10000 > i; i++);

	reg = tlv320aic23_read_reg_cache(TLV320AIC23_RINVOL);
	tlv320aic23_write(TLV320AIC23_RINVOL, (reg & (~TLV320AIC23_LIM_MUTED)) | TLV320AIC23_LRS_ENABLED);
        for(volatile int i = 0; 10000 > i; i++);

	reg = tlv320aic23_read_reg_cache(TLV320AIC23_ANLG);
	tlv320aic23_write(TLV320AIC23_ANLG, (reg) & (~TLV320AIC23_BYPASS_ON) & (~TLV320AIC23_MICM_MUTED));
        for(volatile int i = 0; 10000 > i; i++);

/* Default output volume */
	tlv320aic23_write(TLV320AIC23_LCHNVOL, 0x0080+__k);
        for(volatile int i = 0; 10000 > i; i++);
        tlv320aic23_write(TLV320AIC23_RCHNVOL, 0x0080+__k);
	for(volatile int i = 0; 10000 > i; i++);
       
/*формат передачи audio*/	
	tlv320aic23_set_dai_fmt();
        for(volatile int i = 0; 10000 > i; i++);
        
/*sample rate playback*/	
	set_sample_rate_control(MCLK,sample_rate, sample_rate);
        for(volatile int i = 0; 10000 > i; i++);
        

	tlv320aic23_write(TLV320AIC23_ACTIVE, 0x1);
        for(volatile int i = 0; 10000 > i; i++);
 
	return 0;
}

void _aic23_svolume(unsigned short _p)
{
	tlv320aic23_write(TLV320AIC23_LCHNVOL, 0x0080+_p);
        for(volatile int i = 0; 10000 > i; i++);
        tlv320aic23_write(TLV320AIC23_RCHNVOL, 0x0080+_p);
	for(volatile int i = 0; 10000 > i; i++);
}

void _set_srate(unsigned short sample_rate)
{
 /*sample rate playback*/	
	set_sample_rate_control(MCLK,sample_rate, sample_rate);
        for(volatile int i = 0; 10000 > i; i++);
 
}





