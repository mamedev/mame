// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
/*
    vlm5030.c

    Sanyo VLM5030 emulator

    Written by Tatsuyuki Satoh
    Based on TMS5220 simulator (tms5220.c)

                 +-------,_,-------+
        GND   -- |  1           40 | <-    RST
  (gnd) TST1  -> |  2           39 | ??    TST4
        OSC2  ck |  3     _     38 | ??    TST3
        OSC1  ck |  4    (_)    37 | ??    TST2
        D0    -> |  5           36 | ->    DAO
        D1    -> |  6           35 | --    VREF (+5v through 5.6k resistor)
        D2    -> |  7           34 | ->    MTE
        D3    -> |  8      V    33 | ->    /ME
        D4    -> |  9      L    32 | <-    VCU
        D5    -> | 10      M    31 | <-    START
        D6    -> | 11      5    30 | ->    BSY
        D7    -> | 12      0    29 | --    Vdd (+5v)
        A0    <- | 13      3    28 | ->    A15
        A1    <- | 14      0    27 | ->    A14
        A2    <- | 15           26 | ->    A13
        A3    <- | 16     _     25 | ->    A12
        A4    <- | 17    (_)    24 | ->    A11
        A5    <- | 18           23 | ->    A10
        A6    <- | 19           22 | ->    A9
        A7    <- | 20           21 | ->    A8
                 +-----------------+

TST1 is probably a test mode enable pin, must be grounded for normal operation.
TST2-4 are some sort of test pins but can be left floating?
VREF is probably the 0v ref for the output dac
DAO is the output dac
/ME is connected to the voice data rom /OE enable
START strobes in a byte of data over the data bus from host cpu
OSC1/2 are to both ends of a 3.579545MHz xtal with a 100pf cap from each end to gnd
VCU makes the data bus select the upper 8 bits of the word register internally instead of the lower 8 bits. it is only useful if you need more than 256 phrases in rom? (recheck this)
MTE is an output for roms which need to be clocked to latch address before use, or for a latch sitting in front of the voice rom address lines? (recheck this)
RST not only resets the chip on its rising edge but grabs a byte of mode state data from the data bus on its falling edge? (recheck this)

  note:
    memory read cycle(==sampling rate) = 122.9u(440clock)
    interpolator (LC8109 = 2.5ms)      = 20 * samples(125us)
    frame time (20ms)                  =  4 * interpolator
    9bit DAC is composed of 5bit Physical and 3bitPWM.

  todo:
    Noise Generator circuit without 'machine.rand()' function.

----------- command format (Analytical result) ----------

1)end of speech (8bit)
:00000011:

2)silent some frame (8bit)
:????SS01:

SS : number of silent frames
   00 = 2 frame
   01 = 4 frame
   10 = 6 frame
   11 = 8 frame

3)-speech frame (48bit)
function:   6th  :  5th   :   4th  :   3rd  :   2nd  : 1st    :
end     :   ---  :  ---   :   ---  :   ---  :   ---  :00000011:
silent  :   ---  :  ---   :   ---  :   ---  :   ---  :0000SS01:
speech  :11111122:22233334:44455566:67778889:99AAAEEE:EEPPPPP0:

EEEEE  : energy : volume 0=off,0x1f=max
PPPPP  : pitch  : 0=noize , 1=fast,0x1f=slow
111111 : K1     : 48=off
22222  : K2     : 0=off,1=+min,0x0f=+max,0x10=off,0x11=+max,0x1f=-min
                : 16 == special function??
3333   : K3     : 0=off,1=+min,0x07=+max,0x08=-max,0x0f=-min
4444   : K4     :
555    : K5     : 0=off,1=+min,0x03=+max,0x04=-max,0x07=-min
666    : K6     :
777    : K7     :
888    : K8     :
999    : K9     :
AAA    : K10    :

 ---------- chirp table information ----------

DAC PWM cycle == 88system clock , (11clock x 8 pattern) = 40.6KHz
one chirp     == 5 x PWM cycle == 440systemclock(8,136Hz)

chirp  0   : volume 10- 8 : with filter
chirp  1   : volume  8- 6 : with filter
chirp  2   : volume  6- 4 : with filter
chirp  3   : volume   4   : no filter ??
chirp  4- 5: volume  4- 2 : with filter
chirp  6-11: volume  2- 0 : with filter
chirp 12-..: vokume   0   : silent

 ---------- digial output information ----------
 when ME pin = high , some status output to A0..15 pins

  A0..8   : DAC output value (abs)
  A9      : DAC sign flag , L=minus,H=Plus
  A10     : energy reload flag (pitch pulse)
  A11..15 : unknown

  [DAC output value(signed 6bit)] = A9 ? A0..8 : -(A0..8)

*/
#include "emu.h"
#include "vlm5030.h"

/* interpolator per frame   */
#define FR_SIZE 4
/* samples per interpolator */
#define IP_SIZE_SLOWER  (240/FR_SIZE)
#define IP_SIZE_SLOW    (200/FR_SIZE)
#define IP_SIZE_NORMAL  (160/FR_SIZE)
#define IP_SIZE_FAST    (120/FR_SIZE)
#define IP_SIZE_FASTER  ( 80/FR_SIZE)


/* phase value */
enum {
	PH_RESET,
	PH_IDLE,
	PH_SETUP,
	PH_WAIT,
	PH_RUN,
	PH_STOP,
	PH_END
};

/* Pull in the ROM tables */
#include "tms5110r.inc"

/*
  speed parameter
SPC SPB SPA
 1   0   1  more slow (05h)     : 42ms   (150%) : 60sample
 1   1   x  slow      (06h,07h) : 34ms   (125%) : 50sample
 x   0   0  normal    (00h,04h) : 25.6ms (100%) : 40samplme
 0   0   1  fast      (01h)     : 20.2ms  (75%) : 30sample
 0   1   x  more fast (02h,03h) : 12.2ms  (50%) : 20sample
*/
static const int vlm5030_speed_table[8] =
{
	IP_SIZE_NORMAL,
	IP_SIZE_FAST,
	IP_SIZE_FASTER,
	IP_SIZE_FASTER,
	IP_SIZE_NORMAL,
	IP_SIZE_SLOWER,
	IP_SIZE_SLOW,
	IP_SIZE_SLOW
};

const device_type VLM5030 = &device_creator<vlm5030_device>;

vlm5030_device::vlm5030_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VLM5030, "VLM5030", tag, owner, clock, "vlm5030", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channel(nullptr),
		m_coeff(nullptr),
		m_rom(nullptr),
		m_address_mask(0),
		m_address(0),
		m_pin_BSY(0),
		m_pin_ST(0),
		m_pin_VCU(0),
		m_pin_RST(0),
		m_latch_data(0),
		m_vcu_addr_h(0),
		m_parameter(0),
		m_phase(PH_RESET),
		m_frame_size(0),
		m_pitch_offset(0),
		m_interp_step(0),
		m_interp_count(0),
		m_sample_count(0),
		m_pitch_count(0),
		m_old_energy(0),
		m_old_pitch(0),
		m_target_energy(0),
		m_target_pitch(0),
		m_new_energy(0),
		m_new_pitch(0),
		m_current_energy(0),
		m_current_pitch(0)
{
		memset(m_old_k, 0, sizeof(m_old_k));
		memset(m_new_k, 0, sizeof(m_new_k));
		memset(m_current_k, 0, sizeof(m_current_k));
		memset(m_target_k, 0, sizeof(m_target_k));
		memset(m_x, 0, sizeof(m_x));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

/* start VLM5030 with sound rom              */
/* speech_rom == 0 -> use sampling data mode */

void vlm5030_device::device_start()
{
	m_coeff = &vlm5030_coeff;

	/* reset input pins */
	m_pin_RST = m_pin_ST = m_pin_VCU= 0;
	m_latch_data = 0;

	device_reset();
	m_phase = PH_IDLE;

	m_rom = region()->base();
	m_address_mask = (region()->bytes() - 1) & 0xffff;

	m_channel = machine().sound().stream_alloc(*this, 0, 1, clock() / 440);

	/* don't restore "UINT8 *m_rom" when use vlm5030_set_rom() */

	save_item(NAME(m_address));
	save_item(NAME(m_pin_BSY));
	save_item(NAME(m_pin_ST));
	save_item(NAME(m_pin_VCU));
	save_item(NAME(m_pin_RST));
	save_item(NAME(m_latch_data));
	save_item(NAME(m_vcu_addr_h));
	save_item(NAME(m_parameter));
	save_item(NAME(m_phase));
	save_item(NAME(m_interp_count));
	save_item(NAME(m_sample_count));
	save_item(NAME(m_pitch_count));
	save_item(NAME(m_old_energy));
	save_item(NAME(m_old_pitch));
	save_item(NAME(m_old_k));
	save_item(NAME(m_target_energy));
	save_item(NAME(m_target_pitch));
	save_item(NAME(m_target_k));
	save_item(NAME(m_x));
	machine().save().register_postload(save_prepost_delegate(FUNC(vlm5030_device::restore_state), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vlm5030_device::device_reset()
{
	m_phase = PH_RESET;
	m_address = 0;
	m_vcu_addr_h = 0;
	m_pin_BSY = 0;

	m_old_energy = m_old_pitch = 0;
	m_new_energy = m_new_pitch = 0;
	m_current_energy = m_current_pitch = 0;
	m_target_energy = m_target_pitch = 0;
	memset(m_old_k, 0, sizeof(m_old_k));
	memset(m_new_k, 0, sizeof(m_new_k));
	memset(m_current_k, 0, sizeof(m_current_k));
	memset(m_target_k, 0, sizeof(m_target_k));
	m_interp_count = m_sample_count = m_pitch_count = 0;
	memset(m_x, 0, sizeof(m_x));
	/* reset parameters */
	setup_parameter( 0x00);
}

int vlm5030_device::get_bits(int sbit,int bits)
{
	int offset = m_address + (sbit>>3);
	int data;

	data = m_rom[offset&m_address_mask] +
			(((int)m_rom[(offset+1)&m_address_mask])*256);
	data >>= (sbit&7);
	data &= (0xff>>(8-bits));

	return data;
}

/* get next frame */
int vlm5030_device::parse_frame()
{
	unsigned char cmd;
	int i;

	/* remember previous frame */
	m_old_energy = m_new_energy;
	m_old_pitch = m_new_pitch;
	for(i=0;i<=9;i++)
		m_old_k[i] = m_new_k[i];

	/* command byte check */
	cmd = m_rom[m_address&m_address_mask];
	if( cmd & 0x01 )
	{   /* extend frame */
		m_new_energy = m_new_pitch = 0;
		for(i=0;i<=9;i++)
			m_new_k[i] = 0;
		m_address++;
		if( cmd & 0x02 )
		{   /* end of speech */

			/* logerror("VLM5030 %04X end \n",m_address ); */
			return 0;
		}
		else
		{   /* silent frame */
			int nums = ( (cmd>>2)+1 )*2;
			/* logerror("VLM5030 %04X silent %d frame\n",m_address,nums ); */
			return nums * FR_SIZE;
		}
	}
	/* pitch */
	m_new_pitch  = ( m_coeff->pitchtable[get_bits(1,m_coeff->pitch_bits)] + m_pitch_offset )&0xff;
	/* energy */
	m_new_energy = m_coeff->energytable[get_bits(6,m_coeff->energy_bits)];

	/* 10 K's */
	m_new_k[9] = m_coeff->ktable[9][get_bits(11,m_coeff->kbits[9])];
	m_new_k[8] = m_coeff->ktable[8][get_bits(14,m_coeff->kbits[8])];
	m_new_k[7] = m_coeff->ktable[7][get_bits(17,m_coeff->kbits[7])];
	m_new_k[6] = m_coeff->ktable[6][get_bits(20,m_coeff->kbits[6])];
	m_new_k[5] = m_coeff->ktable[5][get_bits(23,m_coeff->kbits[5])];
	m_new_k[4] = m_coeff->ktable[4][get_bits(26,m_coeff->kbits[4])];
	m_new_k[3] = m_coeff->ktable[3][get_bits(29,m_coeff->kbits[3])];
	m_new_k[2] = m_coeff->ktable[2][get_bits(33,m_coeff->kbits[2])];
	m_new_k[1] = m_coeff->ktable[1][get_bits(37,m_coeff->kbits[1])];
	m_new_k[0] = m_coeff->ktable[0][get_bits(42,m_coeff->kbits[0])];

	m_address+=6;
	logerror("VLM5030 %04X voice \n",m_address );
	//fprintf(stderr,"*** target Energy, Pitch, and Ks = %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",m_new_energy, m_new_pitch, m_new_k[0], m_new_k[1], m_new_k[2], m_new_k[3], m_new_k[4], m_new_k[5], m_new_k[6], m_new_k[7], m_new_k[8], m_new_k[9]);
	return FR_SIZE;
}

/* realtime update */
void vlm5030_device::update()
{
	m_channel->update();
}

/* setup parameteroption when RST=H */
void vlm5030_device::setup_parameter(UINT8 param)
{
	/* latch parameter value */
	m_parameter = param;

	/* bit 0,1 : 4800bps / 9600bps , interporator step */
	if(param&2) /* bit 1 = 1 , 9600bps */
		m_interp_step = 4; /* 9600bps : no interporator */
	else if(param&1) /* bit1 = 0 & bit0 = 1 , 4800bps */
		m_interp_step = 2; /* 4800bps : 2 interporator */
	else    /* bit1 = bit0 = 0 : 2400bps */
		m_interp_step = 1; /* 2400bps : 4 interporator */

	/* bit 3,4,5 : speed (frame size) */
	m_frame_size = vlm5030_speed_table[(param>>3) &7];

	/* bit 6,7 : low / high pitch */
	if(param&0x80)  /* bit7=1 , high pitch */
		m_pitch_offset = -8;
	else if(param&0x40) /* bit6=1 , low pitch */
		m_pitch_offset = 8;
	else
		m_pitch_offset = 0;
}


void vlm5030_device::restore_state()
{
	int i;

	int interp_effect = FR_SIZE - (m_interp_count%FR_SIZE);
	/* restore parameter data */
	setup_parameter( m_parameter);

	/* restore current energy,pitch & filter */
	m_current_energy = m_old_energy + (m_target_energy - m_old_energy) * interp_effect / FR_SIZE;
	if (m_old_pitch > 1)
		m_current_pitch = m_old_pitch + (m_target_pitch - m_old_pitch) * interp_effect / FR_SIZE;
	for (i = 0; i <= 9 ; i++)
		m_current_k[i] = m_old_k[i] + (m_target_k[i] - m_old_k[i]) * interp_effect / FR_SIZE;
}

/* set speech rom address */
void vlm5030_device::set_rom(void *speech_rom)
{
	m_rom = (UINT8 *)speech_rom;
}

/* get BSY pin level */
READ_LINE_MEMBER( vlm5030_device::bsy )
{
	update();
	return m_pin_BSY;
}

/* latch contoll data */
WRITE8_MEMBER( vlm5030_device::data_w )
{
	m_latch_data = (UINT8)data;
}

/* set RST pin level : reset / set table address A8-A15 */
WRITE_LINE_MEMBER( vlm5030_device::rst )
{
	if( m_pin_RST )
	{
		if( !state )
		{   /* H -> L : latch parameters */
			m_pin_RST = 0;
			setup_parameter( m_latch_data);
		}
	}
	else
	{
		if( state )
		{   /* L -> H : reset chip */
			m_pin_RST = 1;
			if( m_pin_BSY )
			{
				device_reset();
			}
		}
	}
}

/* set VCU pin level : ?? unknown */
WRITE_LINE_MEMBER( vlm5030_device::vcu )
{
	/* direct mode / indirect mode */
	m_pin_VCU = state;
}

/* set ST pin level  : set table address A0-A7 / start speech */
WRITE_LINE_MEMBER( vlm5030_device::st )
{
	int table;

	if( m_pin_ST != state )
	{
		/* pin level is change */
		if( !state )
		{   /* H -> L */
			m_pin_ST = 0;

			if( m_pin_VCU )
			{   /* direct access mode & address High */
				m_vcu_addr_h = ((int)m_latch_data<<8) + 0x01;
			}
			else
			{
				/* start speech */
				/* check access mode */
				if( m_vcu_addr_h )
				{   /* direct access mode */
					m_address = (m_vcu_addr_h&0xff00) + m_latch_data;
					m_vcu_addr_h = 0;
				}
				else
				{   /* indirect accedd mode */
					table = (m_latch_data&0xfe) + (((int)m_latch_data&1)<<8);
					m_address = (((int)m_rom[table&m_address_mask])<<8)
									|        m_rom[(table+1)&m_address_mask];
#if 0
/* show unsupported parameter message */
if( m_interp_step != 1)
	popmessage("No %d %dBPS parameter",table/2,m_interp_step*2400);
#endif
				}
				update();
				/* logerror("VLM5030 %02X start adr=%04X\n",table/2,m_address ); */
				/* reset process status */
				m_sample_count = m_frame_size;
				m_interp_count = FR_SIZE;
				/* clear filter */
				/* start after 3 sampling cycle */
				m_phase = PH_RUN;
			}
		}
		else
		{   /* L -> H */
			m_pin_ST = 1;
			/* setup speech , BSY on after 30ms? */
			m_phase = PH_SETUP;
			m_sample_count = 1; /* wait time for busy on */
			m_pin_BSY = 1; /* */
		}
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void vlm5030_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int buf_count=0;
	int interp_effect;
	int i;
	int u[11];
	stream_sample_t *buffer = outputs[0];

	/* running */
	if( m_phase == PH_RUN || m_phase == PH_STOP )
	{
		/* playing speech */
		while (samples > 0)
		{
			int current_val;

			/* check new interpolator or  new frame */
			if( m_sample_count == 0 )
			{
				if( m_phase == PH_STOP )
				{
					m_phase = PH_END;
					m_sample_count = 1;
					goto phase_stop; /* continue to end phase */
				}
				m_sample_count = m_frame_size;
				/* interpolator changes */
				if ( m_interp_count == 0 )
				{
					/* change to new frame */
					m_interp_count = parse_frame(); /* with change phase */
					if ( m_interp_count == 0 )
					{   /* end mark found */
						m_interp_count = FR_SIZE;
						m_sample_count = m_frame_size; /* end -> stop time */
						m_phase = PH_STOP;
					}
					/* Set old target as new start of frame */
					m_current_energy = m_old_energy;
					m_current_pitch = m_old_pitch;
					for(i=0;i<=9;i++)
						m_current_k[i] = m_old_k[i];
					/* is this a zero energy frame? */
					if (m_current_energy == 0)
					{
						/*osd_printf_debug("processing frame: zero energy\n");*/
						m_target_energy = 0;
						m_target_pitch = m_current_pitch;
						for(i=0;i<=9;i++)
							m_target_k[i] = m_current_k[i];
					}
					else
					{
						/*osd_printf_debug("processing frame: Normal\n");*/
						/*osd_printf_debug("*** Energy = %d\n",m_current_energy);*/
						/*osd_printf_debug("proc: %d %d\n",last_fbuf_head,fbuf_head);*/
						m_target_energy = m_new_energy;
						m_target_pitch = m_new_pitch;
						for(i=0;i<=9;i++)
							m_target_k[i] = m_new_k[i];
					}
				}
				/* next interpolator */
				/* Update values based on step values 25% , 50% , 75% , 100% */
				m_interp_count -= m_interp_step;
				/* 3,2,1,0 -> 1,2,3,4 */
				interp_effect = FR_SIZE - (m_interp_count%FR_SIZE);
				m_current_energy = m_old_energy + (m_target_energy - m_old_energy) * interp_effect / FR_SIZE;
				if (m_old_pitch > 1)
					m_current_pitch = m_old_pitch + (m_target_pitch - m_old_pitch) * interp_effect / FR_SIZE;
				for (i = 0; i <= 9 ; i++)
					m_current_k[i] = m_old_k[i] + (m_target_k[i] - m_old_k[i]) * interp_effect / FR_SIZE;
			}
			/* calcrate digital filter */
			if (m_old_energy == 0)
			{
				/* generate silent samples here */
				current_val = 0x00;
			}
			else if (m_old_pitch <= 1)
			{   /* generate unvoiced samples here */
				current_val = (machine().rand()&1) ? m_current_energy : -m_current_energy;
			}
			else
			{
				/* generate voiced samples here */
				current_val = ( m_pitch_count == 0) ? m_current_energy : 0;
			}

			/* Lattice filter here */
			u[10] = current_val;
			for (i = 9; i >= 0; i--)
				u[i] = u[i+1] - ((-m_current_k[i] * m_x[i]) / 512);
			for (i = 9; i >= 1; i--)
				m_x[i] = m_x[i-1] + ((-m_current_k[i-1] * u[i-1]) / 512);
			m_x[0] = u[0];

			/* clipping, buffering */
			if (u[0] > 511)
				buffer[buf_count] = 511<<6;
			else if (u[0] < -511)
				buffer[buf_count] = UINT32(-511)<<6;
			else
				buffer[buf_count] = (u[0] << 6);
			buf_count++;

			/* sample count */
			m_sample_count--;
			/* pitch */
			m_pitch_count++;
			if (m_pitch_count >= m_current_pitch )
				m_pitch_count = 0;
			/* size */
			samples--;
		}
/*      return;*/
	}
	/* stop phase */
phase_stop:
	switch( m_phase )
	{
	case PH_SETUP:
		if( m_sample_count <= samples)
		{
			m_sample_count = 0;
			/* logerror("VLM5030 BSY=H\n" ); */
			/* pin_BSY = 1; */
			m_phase = PH_WAIT;
		}
		else
		{
			m_sample_count -= samples;
		}
		break;
	case PH_END:
		if( m_sample_count <= samples)
		{
			m_sample_count = 0;
			/* logerror("VLM5030 BSY=L\n" ); */
			m_pin_BSY = 0;
			m_phase = PH_IDLE;
		}
		else
		{
			m_sample_count -= samples;
		}
	}
	/* silent buffering */
	while (samples > 0)
	{
		buffer[buf_count++] = 0x00;
		samples--;
	}
}
