// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Hiromitsu Shioya
/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/

/*
  Changelog, Hiromitsu Shioya 02/05/2002
  fix start address decode timing. (sample loop bug.)

    Changelog, Mish, August 1999:
        Removed interface support for different memory regions per channel.
        Removed interface support for differing channel volume.

        Added bankswitching.
        Added support for multiple chips.

        (Nb:  Should different memory regions per channel be needed
        the bankswitching function can set this up).

NS990821
support for the k007232_VOL() macro.
added external port callback, and functions to set the volume of the channels

*/


#include "emu.h"
#include "k007232.h"

#define   BASE_SHIFT    (12)


#if 0
static const int kdac_note[] = {
	261.63/8, 277.18/8,
	293.67/8, 311.13/8,
	329.63/8,
	349.23/8, 369.99/8,
	392.00/8, 415.31/8,
	440.00/8, 466.16/8,
	493.88/8,

	523.25/8,
};

static const float kdaca_fn[][2] = {
	/* B */
	{ 0x03f, 493.88/8 },        /* ?? */
	{ 0x11f, 493.88/4 },        /* ?? */
	{ 0x18f, 493.88/2 },        /* ?? */
	{ 0x1c7, 493.88   },
	{ 0x1e3, 493.88*2 },
	{ 0x1f1, 493.88*4 },        /* ?? */
	{ 0x1f8, 493.88*8 },        /* ?? */
	/* A+ */
	{ 0x020, 466.16/8 },        /* ?? */
	{ 0x110, 466.16/4 },        /* ?? */
	{ 0x188, 466.16/2 },
	{ 0x1c4, 466.16   },
	{ 0x1e2, 466.16*2 },
	{ 0x1f1, 466.16*4 },        /* ?? */
	{ 0x1f8, 466.16*8 },        /* ?? */
	/* A */
	{ 0x000, 440.00/8 },        /* ?? */
	{ 0x100, 440.00/4 },        /* ?? */
	{ 0x180, 440.00/2 },
	{ 0x1c0, 440.00   },
	{ 0x1e0, 440.00*2 },
	{ 0x1f0, 440.00*4 },        /* ?? */
	{ 0x1f8, 440.00*8 },        /* ?? */
	{ 0x1fc, 440.00*16},        /* ?? */
	{ 0x1fe, 440.00*32},        /* ?? */
	{ 0x1ff, 440.00*64},        /* ?? */
	/* G+ */
	{ 0x0f2, 415.31/4 },
	{ 0x179, 415.31/2 },
	{ 0x1bc, 415.31   },
	{ 0x1de, 415.31*2 },
	{ 0x1ef, 415.31*4 },        /* ?? */
	{ 0x1f7, 415.31*8 },        /* ?? */
	/* G */
	{ 0x0e2, 392.00/4 },
	{ 0x171, 392.00/2 },
	{ 0x1b8, 392.00   },
	{ 0x1dc, 392.00*2 },
	{ 0x1ee, 392.00*4 },        /* ?? */
	{ 0x1f7, 392.00*8 },        /* ?? */
	/* F+ */
	{ 0x0d0, 369.99/4 },        /* ?? */
	{ 0x168, 369.99/2 },
	{ 0x1b4, 369.99   },
	{ 0x1da, 369.99*2 },
	{ 0x1ed, 369.99*4 },        /* ?? */
	{ 0x1f6, 369.99*8 },        /* ?? */
	/* F */
	{ 0x0bf, 349.23/4 },        /* ?? */
	{ 0x15f, 349.23/2 },
	{ 0x1af, 349.23   },
	{ 0x1d7, 349.23*2 },
	{ 0x1eb, 349.23*4 },        /* ?? */
	{ 0x1f5, 349.23*8 },        /* ?? */
	/* E */
	{ 0x0ac, 329.63/4 },
	{ 0x155, 329.63/2 },        /* ?? */
	{ 0x1ab, 329.63   },
	{ 0x1d5, 329.63*2 },
	{ 0x1ea, 329.63*4 },        /* ?? */
	{ 0x1f4, 329.63*8 },        /* ?? */
	/* D+ */
	{ 0x098, 311.13/4 },        /* ?? */
	{ 0x14c, 311.13/2 },
	{ 0x1a6, 311.13   },
	{ 0x1d3, 311.13*2 },
	{ 0x1e9, 311.13*4 },        /* ?? */
	{ 0x1f4, 311.13*8 },        /* ?? */
	/* D */
	{ 0x080, 293.67/4 },        /* ?? */
	{ 0x140, 293.67/2 },        /* ?? */
	{ 0x1a0, 293.67   },
	{ 0x1d0, 293.67*2 },
	{ 0x1e8, 293.67*4 },        /* ?? */
	{ 0x1f4, 293.67*8 },        /* ?? */
	{ 0x1fa, 293.67*16},        /* ?? */
	{ 0x1fd, 293.67*32},        /* ?? */
	/* C+ */
	{ 0x06d, 277.18/4 },        /* ?? */
	{ 0x135, 277.18/2 },        /* ?? */
	{ 0x19b, 277.18   },
	{ 0x1cd, 277.18*2 },
	{ 0x1e6, 277.18*4 },        /* ?? */
	{ 0x1f2, 277.18*8 },        /* ?? */
	/* C */
	{ 0x054, 261.63/4 },
	{ 0x12a, 261.63/2 },
	{ 0x195, 261.63   },
	{ 0x1ca, 261.63*2 },
	{ 0x1e5, 261.63*4 },
	{ 0x1f2, 261.63*8 },        /* ?? */

	{ -1, -1 },
};
#endif

/*************************************************************/


const device_type K007232 = &device_creator<k007232_device>;

k007232_device::k007232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K007232, "K007232 PCM Controller", tag, owner, clock, "k007232", __FILE__),
		device_sound_interface(mconfig, *this),
		m_rom(*this, DEVICE_SELF),
		m_port_write_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007232_device::device_start()
{
	/* Set up the chips */
	m_pcmlimit  = region()->bytes();

	m_port_write_handler.resolve();

	for (int i = 0; i < KDAC_A_PCM_MAX; i++)
	{
		m_addr[i] = 0;
		m_start[i] = 0;
		m_step[i] = 0;
		m_play[i] = 0;
		m_bank[i] = 0;
	}
	m_vol[0][0] = 255;  /* channel A output to output A */
	m_vol[0][1] = 0;
	m_vol[1][0] = 0;
	m_vol[1][1] = 255;  /* channel B output to output B */

	for (auto & elem : m_wreg)
		elem = 0;

	m_stream = machine().sound().stream_alloc(*this, 0 , 2, clock()/128);

	KDAC_A_make_fncode();

	save_item(NAME(m_vol));
	save_item(NAME(m_addr));
	save_item(NAME(m_start));
	save_item(NAME(m_step));
	save_item(NAME(m_bank));
	save_item(NAME(m_play));
	save_item(NAME(m_wreg));
}

void k007232_device::KDAC_A_make_fncode()
{
#if 0
	int i, j, k;
	float fn;
	for( i = 0; i < 0x200; i++ )  fncode[i] = 0;

	i = 0;
	while( (int)kdaca_fn[i][0] != -1 ){
	fncode[(int)kdaca_fn[i][0]] = kdaca_fn[i][1];
	i++;
	}

	i = j = 0;
	while( i < 0x200 ){
	if( fncode[i] != 0 ){
		if( i != j ){
	fn = (fncode[i] - fncode[j]) / (i - j);
	for( k = 1; k < (i-j); k++ )
		fncode[k+j] = fncode[j] + fn*k;
	j = i;
		}
	}
	i++;
	}
#if 0
	for( i = 0; i < 0x200; i++ )
	logerror("fncode[%04x] = %.2f\n", i, fncode[i] );
#endif

#else
	int i;
	for( i = 0; i < 0x200; i++ )
	{
	//fncode[i] = (0x200 * 55) / (0x200 - i);
	m_fncode[i] = (32 << BASE_SHIFT) / (0x200 - i);
//  m_fncode[i] = ((0x200 * 55.2 / 880) / (0x200 - i));
	// = 512 * 55.2 / 220 / (512 - i) = 128 / (512 - i)
	//    logerror("2 : fncode[%04x] = %.2f\n", i, fncode[i] );
	}

#endif
}


/************************************************/
/*    Konami PCM write register                 */
/************************************************/
WRITE8_MEMBER( k007232_device::write )
{
	int r = offset;
	int v = data;

	m_stream->update();

	m_wreg[r] = v;          /* stock write data */

	if (r == 0x0c){
	/* external port, usually volume control */
	if (!m_port_write_handler.isnull()) m_port_write_handler(0, v, mem_mask);
	return;
	}
	else if( r == 0x0d ){
	/* loopflag. */
	return;
	}
	else{
	int  reg_port;

	reg_port = 0;
	if (r >= 0x06){
		reg_port = 1;
		r -= 0x06;
	}

	switch (r){
	case 0x00:
	case 0x01:
	{
				/**** address step ****/
		int idx = (((((unsigned int)m_wreg[reg_port*0x06 + 0x01])<<8)&0x0100) | (((unsigned int)m_wreg[reg_port*0x06 + 0x00])&0x00ff));
#if 0
		if( !reg_port && r == 1 )
	logerror("%04x\n" ,idx );
#endif

		m_step[reg_port] = m_fncode[idx];
		break;
	}
	case 0x02:
	case 0x03:
	case 0x04:
		break;
	case 0x05:
				/**** start address ****/
		m_start[reg_port] =
	((((unsigned int)m_wreg[reg_port*0x06 + 0x04]<<16)&0x00010000) |
		(((unsigned int)m_wreg[reg_port*0x06 + 0x03]<< 8)&0x0000ff00) |
		(((unsigned int)m_wreg[reg_port*0x06 + 0x02]    )&0x000000ff) |
		m_bank[reg_port]);
		if (m_start[reg_port] < m_pcmlimit ){
	m_play[reg_port] = 1;
	m_addr[reg_port] = 0;
		}
		break;
	}
	}
}

/************************************************/
/*    Konami PCM read register                  */
/************************************************/
READ8_MEMBER( k007232_device::read )
{
	int r = offset;
	int  ch = 0;

	if( r == 0x0005 || r == 0x000b ){
	ch = r/0x0006;
	r  = ch * 0x0006;

	m_start[ch] =
		((((unsigned int)m_wreg[r + 0x04]<<16)&0x00010000) |
		(((unsigned int)m_wreg[r + 0x03]<< 8)&0x0000ff00) |
		(((unsigned int)m_wreg[r + 0x02]    )&0x000000ff) |
		m_bank[ch]);

	if (m_start[ch] <  m_pcmlimit ){
		m_play[ch] = 1;
		m_addr[ch] = 0;
	}
	}
	return 0;
}

/*****************************************************************************/

void k007232_device::set_volume(int channel,int volumeA,int volumeB)
{
	m_vol[channel][0] = volumeA;
	m_vol[channel][1] = volumeB;
}

void k007232_device::set_bank(int chABank, int chBBank )
{
	m_bank[0] = chABank<<17;
	m_bank[1] = chBBank<<17;
}

/*****************************************************************************/


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k007232_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i;

	memset(outputs[0],0,samples * sizeof(*outputs[0]));
	memset(outputs[1],0,samples * sizeof(*outputs[1]));

	for( i = 0; i < KDAC_A_PCM_MAX; i++ )
	{
		if (m_play[i])
	{
		int volA,volB,j,out;
		unsigned int addr, old_addr;
		//int cen;

		/**** PCM setup ****/
		addr = m_start[i] + ((m_addr[i]>>BASE_SHIFT)&0x000fffff);
		volA = m_vol[i][0] * 2;
		volB = m_vol[i][1] * 2;
#if 0
		cen = (volA + volB) / 2;
		volA = (volA + cen) < 0x1fe ? (volA + cen) : 0x1fe;
		volB = (volB + cen) < 0x1fe ? (volB + cen) : 0x1fe;
#endif

		for( j = 0; j < samples; j++ )
		{
			old_addr = addr;
			addr = m_start[i] + ((m_addr[i]>>BASE_SHIFT)&0x000fffff);
			while (old_addr <= addr)
		{
			if( (m_rom[old_addr] & 0x80) || old_addr >= m_pcmlimit )
			{
				/* end of sample */

				if( m_wreg[0x0d]&(1<<i) )
			{
				/* loop to the beginning */
				m_start[i] =
				((((unsigned int)m_wreg[i*0x06 + 0x04]<<16)&0x00010000) |
					(((unsigned int)m_wreg[i*0x06 + 0x03]<< 8)&0x0000ff00) |
					(((unsigned int)m_wreg[i*0x06 + 0x02]    )&0x000000ff) |
					m_bank[i]);
				addr = m_start[i];
				m_addr[i] = 0;
				old_addr = addr; /* skip loop */
			}
				else
			{
				/* stop sample */
				m_play[i] = 0;
			}
				break;
			}

			old_addr++;
		}

			if (m_play[i] == 0)
		break;

			m_addr[i] += m_step[i];

			out = (m_rom[addr] & 0x7f) - 0x40;

			outputs[0][j] += out * volA;
			outputs[1][j] += out * volB;
		}
	}
	}
}
