// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Hiromitsu Shioya
#include "emu.h"
#include "msm5232.h"

#define CLOCK_RATE_DIVIDER 16

/*
    OKI MSM5232RS
    8 channel tone generator
*/

DEFINE_DEVICE_TYPE(MSM5232, msm5232_device, "msm5232", "MSM5232")

msm5232_device::msm5232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSM5232, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_noise_cnt(0), m_noise_step(0), m_noise_rng(0), m_noise_clocks(0), m_UpdateStep(0), m_control1(0), m_control2(0), m_gate(0), m_chip_clock(0), m_rate(0)
	, m_gate_handler_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm5232_device::device_start()
{
	int rate = clock()/CLOCK_RATE_DIVIDER;
	int voicenum;

	init(clock(), rate);

	m_stream = stream_alloc(0, 11, rate);

	/* register with the save state system */
	save_item(NAME(m_EN_out16));
	save_item(NAME(m_EN_out8));
	save_item(NAME(m_EN_out4));
	save_item(NAME(m_EN_out2));
	save_item(NAME(m_noise_cnt));
	save_item(NAME(m_noise_rng));
	save_item(NAME(m_noise_clocks));
	save_item(NAME(m_control1));
	save_item(NAME(m_control2));
	save_item(NAME(m_gate));
	save_item(NAME(m_chip_clock));
	save_item(NAME(m_rate));

	/* register voice-specific data for save states */
	for (voicenum = 0; voicenum < 8; voicenum++)
	{
		VOICE *voice = &m_voi[voicenum];

		save_item(NAME(voice->mode), voicenum);
		save_item(NAME(voice->TG_count_period), voicenum);
		save_item(NAME(voice->TG_cnt), voicenum);
		save_item(NAME(voice->TG_out16), voicenum);
		save_item(NAME(voice->TG_out8), voicenum);
		save_item(NAME(voice->TG_out4), voicenum);
		save_item(NAME(voice->TG_out2), voicenum);
		save_item(NAME(voice->egvol), voicenum);
		save_item(NAME(voice->eg_sect), voicenum);
		save_item(NAME(voice->counter), voicenum);
		save_item(NAME(voice->eg), voicenum);
		save_item(NAME(voice->eg_arm), voicenum);
		save_item(NAME(voice->ar_rate), voicenum);
		save_item(NAME(voice->dr_rate), voicenum);
		save_item(NAME(voice->pitch), voicenum);
		save_item(NAME(voice->GF), voicenum);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msm5232_device::device_reset()
{
	for (int i=0; i<8; i++)
	{
		write(i, 0x80);
		write(i, 0x00);
	}
	m_noise_cnt     = 0;
	m_noise_rng     = 1;
	m_noise_clocks  = 0;

	m_control1      = 0;
	m_EN_out16[0]   = 0;
	m_EN_out8[0]    = 0;
	m_EN_out4[0]    = 0;
	m_EN_out2[0]    = 0;

	m_control2      = 0;
	m_EN_out16[1]   = 0;
	m_EN_out8[1]    = 0;
	m_EN_out4[1]    = 0;
	m_EN_out2[1]    = 0;

	gate_update();
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void msm5232_device::device_stop()
{
#ifdef SAVE_SAMPLE
	fclose(sample[8]);
#endif
#ifdef SAVE_SEPARATE_CHANNELS
	fclose(sample[0]);
	fclose(sample[1]);
	fclose(sample[2]);
	fclose(sample[3]);
	fclose(sample[4]);
	fclose(sample[5]);
	fclose(sample[6]);
	fclose(sample[7]);
#endif
}

void msm5232_device::set_capacitors(double cap1, double cap2, double cap3, double cap4, double cap5, double cap6, double cap7, double cap8)
{
	m_external_capacitance[0] = cap1;
	m_external_capacitance[1] = cap2;
	m_external_capacitance[2] = cap3;
	m_external_capacitance[3] = cap4;
	m_external_capacitance[4] = cap5;
	m_external_capacitance[5] = cap6;
	m_external_capacitance[6] = cap7;
	m_external_capacitance[7] = cap8;
}

// Default chip clock is 2119040 Hz
// At this clock chip generates exactly 440.0 Hz signal on 8' output when pitch data=0x21


// ROM table to convert from pitch data into data for programmable counter and binary counter
// Chip has 88x12bits ROM   (addressing (in hex) from 0x00 to 0x57)
#define ROM(counter,bindiv) (counter|(bindiv<<9))

static const uint16_t MSM5232_ROM[88]={
/* higher values are Programmable Counter data (9 bits) */
/* lesser values are Binary Counter shift data (3 bits) */

/* 0 */ ROM (506, 7),

/* 1 */ ROM (478, 7),/* 2 */ ROM (451, 7),/* 3 */ ROM (426, 7),/* 4 */ ROM (402, 7),
/* 5 */ ROM (379, 7),/* 6 */ ROM (358, 7),/* 7 */ ROM (338, 7),/* 8 */ ROM (319, 7),
/* 9 */ ROM (301, 7),/* A */ ROM (284, 7),/* B */ ROM (268, 7),/* C */ ROM (253, 7),

/* D */ ROM (478, 6),/* E */ ROM (451, 6),/* F */ ROM (426, 6),/*10 */ ROM (402, 6),
/*11 */ ROM (379, 6),/*12 */ ROM (358, 6),/*13 */ ROM (338, 6),/*14 */ ROM (319, 6),
/*15 */ ROM (301, 6),/*16 */ ROM (284, 6),/*17 */ ROM (268, 6),/*18 */ ROM (253, 6),

/*19 */ ROM (478, 5),/*1A */ ROM (451, 5),/*1B */ ROM (426, 5),/*1C */ ROM (402, 5),
/*1D */ ROM (379, 5),/*1E */ ROM (358, 5),/*1F */ ROM (338, 5),/*20 */ ROM (319, 5),
/*21 */ ROM (301, 5),/*22 */ ROM (284, 5),/*23 */ ROM (268, 5),/*24 */ ROM (253, 5),

/*25 */ ROM (478, 4),/*26 */ ROM (451, 4),/*27 */ ROM (426, 4),/*28 */ ROM (402, 4),
/*29 */ ROM (379, 4),/*2A */ ROM (358, 4),/*2B */ ROM (338, 4),/*2C */ ROM (319, 4),
/*2D */ ROM (301, 4),/*2E */ ROM (284, 4),/*2F */ ROM (268, 4),/*30 */ ROM (253, 4),

/*31 */ ROM (478, 3),/*32 */ ROM (451, 3),/*33 */ ROM (426, 3),/*34 */ ROM (402, 3),
/*35 */ ROM (379, 3),/*36 */ ROM (358, 3),/*37 */ ROM (338, 3),/*38 */ ROM (319, 3),
/*39 */ ROM (301, 3),/*3A */ ROM (284, 3),/*3B */ ROM (268, 3),/*3C */ ROM (253, 3),

/*3D */ ROM (478, 2),/*3E */ ROM (451, 2),/*3F */ ROM (426, 2),/*40 */ ROM (402, 2),
/*41 */ ROM (379, 2),/*42 */ ROM (358, 2),/*43 */ ROM (338, 2),/*44 */ ROM (319, 2),
/*45 */ ROM (301, 2),/*46 */ ROM (284, 2),/*47 */ ROM (268, 2),/*48 */ ROM (253, 2),

/*49 */ ROM (478, 1),/*4A */ ROM (451, 1),/*4B */ ROM (426, 1),/*4C */ ROM (402, 1),
/*4D */ ROM (379, 1),/*4E */ ROM (358, 1),/*4F */ ROM (338, 1),/*50 */ ROM (319, 1),
/*51 */ ROM (301, 1),/*52 */ ROM (284, 1),/*53 */ ROM (268, 1),/*54 */ ROM (253, 1),

/*55 */ ROM (253, 1),/*56 */ ROM (253, 1),

/*57 */ ROM (13, 7)
};
#undef ROM


#define STEP_SH (16)    /* step calculations accuracy */


/* save output as raw 16-bit sample */
/* #define SAVE_SAMPLE */
/* #define SAVE_SEPARATE_CHANNELS */
#if defined SAVE_SAMPLE || defined SAVE_SEPARATE_CHANNELS
static FILE *sample[9];
#endif



/*
 * Resistance values are guesswork, default capacitance is mentioned in the datasheets
 *
 * Two errors in the datasheet, one probable, one certain
 * - it mentions 0.39uF caps, but most boards have 1uF caps and expect datasheet timings
 *
 * - the 330ms timing of decay2 has been measured to be 250ms (which
 *   also matches the duty cycle information for the rest of the table)
 *
 * In both cases it ends up with smaller resistor values, which are
 * easier to do on-die.
 *
 * The timings are for a 90% charge/discharge of the external
 * capacitor through three possible resistors, one for attack, two for
 * decay.
 *
 * Expected timings are 2ms, 40ms and 250ms respectively with a 1uF
 * capacitor.
 *
 * exp(-t/(r*c)) = (100% - 90%) => r = -r/(log(0.1)*c)
 *
 *   2ms ->    870 ohms
 *  40ms ->  17400 ohms
 * 250ms -> 101000 ohms
 */


static constexpr double R51 =    870;    // attack resistance
static constexpr double R52 =  17400;    // decay 1 resistance
static constexpr double R53 = 101000;    // decay 2 resistance


void msm5232_device::init_tables()
{
	// sample rate = chip clock !!!  But :
	// highest possible frequency is chipclock/13/16 (pitch data=0x57)
	// at 2MHz : 2000000/13/16 = 9615 Hz

	m_UpdateStep = int(double(1 << STEP_SH) * double(m_rate) / double(m_chip_clock));
	//logerror("clock=%i Hz rate=%i Hz, UpdateStep=%i\n", m_chip_clock, m_rate, m_UpdateStep);

	double const scale = double(m_chip_clock) / double(m_rate);
	m_noise_step = ((1 << STEP_SH) / 128.0) * scale; // step of the rng reg in 16.16 format
	//logerror("noise step=%8x\n", m_noise_step);

	for (int i = 0; i < 8; i++)
	{
		double const clockscale = double(m_chip_clock) / 2119040.0;
		int const rcp_duty_cycle = 1 << ((i & 4) ? (i & ~2) : i); // bit 1 is ignored if bit 2 is set
		m_ar_tbl[i] = (rcp_duty_cycle / clockscale) * R51;
	}

	for (int i = 0; i < 8; i++)
	{
		double const clockscale = double(m_chip_clock) / 2119040.0;
		int const rcp_duty_cycle = 1 << ((i & 4) ? (i & ~2) : i); // bit 1 is ignored if bit 2 is set
		m_dr_tbl[i] = (rcp_duty_cycle / clockscale) * R52;
		m_dr_tbl[i + 8] = (rcp_duty_cycle / clockscale) * R53;
	}


#ifdef SAVE_SAMPLE
	sample[8]=fopen("sampsum.pcm","wb");
#endif
#ifdef SAVE_SEPARATE_CHANNELS
	sample[0]=fopen("samp0.pcm","wb");
	sample[1]=fopen("samp1.pcm","wb");
	sample[2]=fopen("samp2.pcm","wb");
	sample[3]=fopen("samp3.pcm","wb");
	sample[4]=fopen("samp4.pcm","wb");
	sample[5]=fopen("samp5.pcm","wb");
	sample[6]=fopen("samp6.pcm","wb");
	sample[7]=fopen("samp7.pcm","wb");
#endif
}


void msm5232_device::init_voice(int i)
{
	m_voi[i].ar_rate= m_ar_tbl[0] * m_external_capacitance[i];
	m_voi[i].dr_rate= m_dr_tbl[0] * m_external_capacitance[i];
	m_voi[i].rr_rate= m_dr_tbl[0] * m_external_capacitance[i]; /* this is constant value */
	m_voi[i].eg_sect= -1;
	m_voi[i].eg     = 0.0;
	m_voi[i].eg_arm = 0;
	m_voi[i].pitch  = -1.0;
}


void msm5232_device::gate_update()
{
	int new_state = (m_control2 & 0x20) ? m_voi[7].GF : 0;

	if (m_gate != new_state)
	{
		m_gate = new_state;
		m_gate_handler_cb(new_state);
	}
}

void msm5232_device::init(int clock, int rate)
{
	int j;

	m_chip_clock = clock;
	m_rate  = rate ? rate : 44100;  /* avoid division by 0 */

	init_tables();

	for (j=0; j<8; j++)
	{
		memset(&m_voi[j],0,sizeof(VOICE));
		init_voice(j);
	}
}


void msm5232_device::write(offs_t offset, uint8_t data)
{
	if (offset > 0x0d)
		return;

	m_stream->update();

	if (offset < 0x08) /* pitch */
	{
		const int ch = offset & 7;

		m_voi[ch].GF = BIT(data, 7);
		if (ch == 7)
			gate_update();

		if (data & 0x80)
		{
			if (data >= 0xd8)
			{
				/*if ((data&0x7f) != 0x5f) logerror("MSM5232: WRONG PITCH CODE = %2x\n",data&0x7f);*/
				m_voi[ch].mode = 1;     /* noise mode */
				m_voi[ch].eg_sect = 0;  /* Key On */
			}
			else
			{
				if (m_voi[ch].pitch != (data & 0x7f))
				{
					int n;

					m_voi[ch].pitch = data & 0x7f;

					const uint32_t pg = MSM5232_ROM[data & 0x7f];

					m_voi[ch].TG_count_period = (pg & 0x1ff) * m_UpdateStep / 2;

					n = (pg >> 9) & 7;  /* n = bit number for 16' output */
					m_voi[ch].TG_out16 = 1<<n;
										/* for 8' it is bit n-1 (bit 0 if n-1<0) */
										/* for 4' it is bit n-2 (bit 0 if n-2<0) */
										/* for 2' it is bit n-3 (bit 0 if n-3<0) */
					n = (n > 0) ? (n - 1) : 0;
					m_voi[ch].TG_out8  = 1 << n;

					n = (n > 0) ? (n - 1) : 0;
					m_voi[ch].TG_out4  = 1 << n;

					n = (n > 0) ? (n - 1) : 0;
					m_voi[ch].TG_out2  = 1 << n;
				}
				m_voi[ch].mode = 0;     /* tone mode */
				m_voi[ch].eg_sect = 0;  /* Key On */
			}
		}
		else
		{
			if (!m_voi[ch].eg_arm)      /* arm = 0 */
				m_voi[ch].eg_sect = 2;  /* Key Off -> go to release */
			else                        /* arm = 1 */
				m_voi[ch].eg_sect = 1;  /* Key Off -> go to decay */
		}
	}
	else
	{
		switch(offset)
		{
		case 0x08:  /* group1 attack */
			for (int i = 0; i < 4; i++)
				m_voi[i].ar_rate   = m_ar_tbl[data & 0x7] * m_external_capacitance[i];
			break;

		case 0x09:  /* group2 attack */
			for (int i = 0; i < 4; i++)
				m_voi[i + 4].ar_rate = m_ar_tbl[data & 0x7] * m_external_capacitance[i+4];
			break;

		case 0x0a:  /* group1 decay */
			for (int i = 0; i < 4; i++)
				m_voi[i].dr_rate   = m_dr_tbl[data & 0xf] * m_external_capacitance[i];
			break;

		case 0x0b:  /* group2 decay */
			for (int i = 0; i < 4; i++)
				m_voi[i + 4].dr_rate = m_dr_tbl[data & 0xf] * m_external_capacitance[i + 4];
			break;

		case 0x0c:  /* group1 control */

			/*if (m_control1 != data)
			    logerror("msm5232: control1 ctrl=%x OE=%x\n", data&0xf0, data&0x0f);*/

			/*if (data & 0x10)
			    popmessage("msm5232: control1 ctrl=%2x\n", data);*/

			m_control1 = data;

			for (int i = 0; i < 4; i++)
			{
				if ((data & 0x10) && (m_voi[i].eg_sect == 1))
					m_voi[i].eg_sect = 0;
				m_voi[i].eg_arm = data & 0x10;
			}

			m_EN_out16[0] = (data & 1) ? ~0 : 0;
			m_EN_out8[0]  = (data & 2) ? ~0 : 0;
			m_EN_out4[0]  = (data & 4) ? ~0 : 0;
			m_EN_out2[0]  = (data & 8) ? ~0 : 0;

			break;

		case 0x0d:  /* group2 control */

			/*if (m_control2 != data)
			    logerror("msm5232: control2 ctrl=%x OE=%x\n", data&0xf0, data&0x0f);*/

			/*if (data & 0x10)
			    popmessage("msm5232: control2 ctrl=%2x\n", data);*/

			m_control2 = data;
			gate_update();

			for (int i = 0; i < 4; i++)
			{
				if ((data & 0x10) && (m_voi[i + 4].eg_sect == 1))
					m_voi[i + 4].eg_sect = 0;
				m_voi[i + 4].eg_arm = data & 0x10;
			}

			m_EN_out16[1] = (data&1) ? ~0:0;
			m_EN_out8[1]  = (data&2) ? ~0:0;
			m_EN_out4[1]  = (data&4) ? ~0:0;
			m_EN_out2[1]  = (data&8) ? ~0:0;

			break;
		}
	}
}



#define VMIN    0
#define VMAX    32768


void msm5232_device::EG_voices_advance()
{
	VOICE *voi = &m_voi[0];
	int samplerate = m_rate;
	int i;

	i = 8;
	do
	{
		switch(voi->eg_sect)
		{
		case 0: /* attack */

			/* capacitor charge */
			if (voi->eg < VMAX)
			{
				voi->counter -= (int)((VMAX - voi->eg) / voi->ar_rate);
				if ( voi->counter <= 0 )
				{
					int n = -voi->counter / samplerate + 1;
					voi->counter += n * samplerate;
					if ( (voi->eg += n) > VMAX )
						voi->eg = VMAX;
				}
			}

			/* when ARM=0, EG switches to decay as soon as cap is charged to VT (EG inversion voltage; about 80% of MAX) */
			if (!voi->eg_arm)
			{
				if(voi->eg >= VMAX * 80/100 )
				{
					voi->eg_sect = 1;
				}
			}
			else
			/* ARM=1 */
			{
				/* when ARM=1, EG stays at maximum until key off */
			}

			voi->egvol = voi->eg / 16; /*32768/16 = 2048 max*/

			break;

		case 1: /* decay */

			/* capacitor discharge */
			if (voi->eg > VMIN)
			{
				voi->counter -= (int)((voi->eg - VMIN) / voi->dr_rate);
				if ( voi->counter <= 0 )
				{
					int n = -voi->counter / samplerate + 1;
					voi->counter += n * samplerate;
					if ( (voi->eg -= n) < VMIN )
						voi->eg = VMIN;
				}
			}
			else /* voi->eg <= VMIN */
			{
				voi->eg_sect =-1;
			}

			voi->egvol = voi->eg / 16; /*32768/16 = 2048 max*/

			break;

		case 2: /* release */

			/* capacitor discharge */
			if (voi->eg > VMIN)
			{
				voi->counter -= (int)((voi->eg - VMIN) / voi->rr_rate);
				if ( voi->counter <= 0 )
				{
					int n = -voi->counter / samplerate + 1;
					voi->counter += n * samplerate;
					if ( (voi->eg -= n) < VMIN )
						voi->eg = VMIN;
				}
			}
			else /* voi->eg <= VMIN */
			{
				voi->eg_sect =-1;
			}

			voi->egvol = voi->eg / 16; /*32768/16 = 2048 max*/

			break;

		default:
			break;
		}

		voi++;
		i--;
	} while (i>0);

}

static int o2,o4,o8,o16,solo8,solo16;

void msm5232_device::TG_group_advance(int groupidx)
{
	VOICE *voi = &m_voi[groupidx*4];
	int i;

	o2 = o4 = o8 = o16 = solo8 = solo16 = 0;

	i=4;
	do
	{
		int out2, out4, out8, out16;

		out2 = out4 = out8 = out16 = 0;

		if (voi->mode==0)   /* generate square tone */
		{
			int left = 1<<STEP_SH;
			do
			{
				int nextevent = left;

				if (voi->TG_cnt&voi->TG_out16)  out16+=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out8)   out8 +=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out4)   out4 +=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out2)   out2 +=voi->TG_count;

				voi->TG_count -= nextevent;

				while (voi->TG_count <= 0)
				{
					voi->TG_count += voi->TG_count_period;
					voi->TG_cnt++;
					if (voi->TG_cnt&voi->TG_out16)  out16+=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out8 )  out8 +=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out4 )  out4 +=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out2 )  out2 +=voi->TG_count_period;

					if (voi->TG_count > 0)
						break;

					voi->TG_count += voi->TG_count_period;
					voi->TG_cnt++;
					if (voi->TG_cnt&voi->TG_out16)  out16+=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out8 )  out8 +=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out4 )  out4 +=voi->TG_count_period;
					if (voi->TG_cnt&voi->TG_out2 )  out2 +=voi->TG_count_period;
				}
				if (voi->TG_cnt&voi->TG_out16)  out16-=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out8 )  out8 -=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out4 )  out4 -=voi->TG_count;
				if (voi->TG_cnt&voi->TG_out2 )  out2 -=voi->TG_count;

				left -=nextevent;

			}while (left>0);
		}
		else    /* generate noise */
		{
			if (m_noise_clocks&8)   out16+=(1<<STEP_SH);
			if (m_noise_clocks&4)   out8 +=(1<<STEP_SH);
			if (m_noise_clocks&2)   out4 +=(1<<STEP_SH);
			if (m_noise_clocks&1)   out2 +=(1<<STEP_SH);
		}

		/* calculate signed output */
		o16 += ( (out16-(1<<(STEP_SH-1))) * voi->egvol) >> STEP_SH;
		o8  += ( (out8 -(1<<(STEP_SH-1))) * voi->egvol) >> STEP_SH;
		o4  += ( (out4 -(1<<(STEP_SH-1))) * voi->egvol) >> STEP_SH;
		o2  += ( (out2 -(1<<(STEP_SH-1))) * voi->egvol) >> STEP_SH;

		if (i == 1 && groupidx == 1)
		{
			solo16 += ( (out16-(1<<(STEP_SH-1))) << 11) >> STEP_SH;
			solo8  += ( (out8 -(1<<(STEP_SH-1))) << 11) >> STEP_SH;
		}

		voi++;
		i--;
	}while (i>0);

	/* cut off disabled output lines */
	o16 &= m_EN_out16[groupidx];
	o8  &= m_EN_out8 [groupidx];
	o4  &= m_EN_out4 [groupidx];
	o2  &= m_EN_out2 [groupidx];
}


/* macro saves feet data to mono file */
#ifdef SAVE_SEPARATE_CHANNELS
	#define SAVE_SINGLE_CHANNEL(j,val) \
	{   signed int pom= val; \
	if (pom > 32767) pom = 32767; else if (pom < -32768) pom = -32768; \
	fputc((unsigned short)pom&0xff,sample[j]); \
	fputc(((unsigned short)pom>>8)&0xff,sample[j]);  }
#else
	#define SAVE_SINGLE_CHANNEL(j,val)
#endif

/* first macro saves all 8 feet outputs to mixed (mono) file */
/* second macro saves one group into left and the other in right channel */
#if 1   /*MONO*/
	#ifdef SAVE_SAMPLE
		#define SAVE_ALL_CHANNELS \
		{   signed int pom = buf1[i] + buf2[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
		}
	#else
		#define SAVE_ALL_CHANNELS
	#endif
#else   /*STEREO*/
	#ifdef SAVE_SAMPLE
		#define SAVE_ALL_CHANNELS \
		{   signed int pom = buf1[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
		pom = buf2[i]; \
		fputc((unsigned short)pom&0xff,sample[8]); \
		fputc(((unsigned short)pom>>8)&0xff,sample[8]); \
		}
	#else
		#define SAVE_ALL_CHANNELS
	#endif
#endif


/* MAME Interface */
void msm5232_device::device_post_load()
{
	init_tables();
}

void msm5232_device::set_clock(int clock)
{
	if (m_chip_clock != clock)
	{
		m_stream->update ();
		m_chip_clock = clock;
		m_rate = clock/CLOCK_RATE_DIVIDER;
		init_tables();
		m_stream->set_sample_rate(m_rate);
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void msm5232_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &buf1 = outputs[0];
	auto &buf2 = outputs[1];
	auto &buf3 = outputs[2];
	auto &buf4 = outputs[3];
	auto &buf5 = outputs[4];
	auto &buf6 = outputs[5];
	auto &buf7 = outputs[6];
	auto &buf8 = outputs[7];
	auto &bufsolo1 = outputs[8];
	auto &bufsolo2 = outputs[9];
	auto &bufnoise = outputs[10];
	int i;

	for (i=0; i<buf1.samples(); i++)
	{
		/* calculate all voices' envelopes */
		EG_voices_advance();

		TG_group_advance(0);   /* calculate tones group 1 */
		buf1.put_int(i, o2, 32768);
		buf2.put_int(i, o4, 32768);
		buf3.put_int(i, o8, 32768);
		buf4.put_int(i, o16, 32768);

		SAVE_SINGLE_CHANNEL(0,o2)
		SAVE_SINGLE_CHANNEL(1,o4)
		SAVE_SINGLE_CHANNEL(2,o8)
		SAVE_SINGLE_CHANNEL(3,o16)

		TG_group_advance(1);   /* calculate tones group 2 */
		buf5.put_int(i, o2, 32768);
		buf6.put_int(i, o4, 32768);
		buf7.put_int(i, o8, 32768);
		buf8.put_int(i, o16, 32768);

		bufsolo1.put_int(i, solo8, 32768);
		bufsolo2.put_int(i, solo16, 32768);

		SAVE_SINGLE_CHANNEL(4,o2)
		SAVE_SINGLE_CHANNEL(5,o4)
		SAVE_SINGLE_CHANNEL(6,o8)
		SAVE_SINGLE_CHANNEL(7,o16)

		SAVE_ALL_CHANNELS

		/* update noise generator */
		{
			int cnt = (m_noise_cnt+=m_noise_step) >> STEP_SH;
			m_noise_cnt &= ((1<<STEP_SH)-1);
			while (cnt > 0)
			{
				int tmp = m_noise_rng & (1<<16);        /* store current level */

				if (m_noise_rng&1)
					m_noise_rng ^= 0x24000;
				m_noise_rng>>=1;

				if ( (m_noise_rng & (1<<16)) != tmp )   /* level change detect */
					m_noise_clocks++;

				cnt--;
			}
		}

		bufnoise.put(i, (m_noise_rng & (1<<16)) ? 1.0 : 0.0);
	}
}
