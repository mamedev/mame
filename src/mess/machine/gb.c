/***************************************************************************

  gb.c

  Machine file to handle emulation of the Nintendo Game Boy.

Cardridge port pinouts:
Pin  Name     Description
1    VCC      +5 VDC
2    PHI      CPU clock ?
3    /WR      Write
4    /RD      Read
5    /CS      SRAM select
6    A0       Address 0
7    A1       Address 1
8    A2       Address 2
9    A3       Address 3
10   A4       Address 4
11   A5       Address 5
12   A6       Address 6
13   A7       Address 7
14   A8       Address 8
15   A9       Address 9
16   A10      Address 10
17   A11      Address 11
18   A12      Address 12
19   A13      Address 13
20   A14      Address 14
21   A15      Address 15
22   D0       Data 0
23   D1       Data 1
24   D2       Data 2
25   D3       Data 3
26   D4       Data 4
27   D5       Data 5
28   D6       Data 6
29   D7       Data 7
30   /RST     Reset
31   AUDIOIN  Never used ?
32   GND      Ground


TODO:
- YongYong mapper:
  - During start there are 2 writes to 5000 and 5003, it is still unknown what these do.
- Story of La Sa Ma mapper:
  - This should display the Gowin logo on boot on both DMG and CGB (Not implemented yet)
- ATV Racing/Rocket Games mapper:
  - How did this overlay the official Nintendo logo at BIOS check time? (Some Sachen titles use a similar trick)


  Changes:

    13/2/2002       AK - MBC2 and MBC3 support and added NVRAM support.
    23/2/2002       AK - MBC5 support, and MBC2 RAM support.
    13/3/2002       AK - Tidied up the MBC code, window layer now has it's
                         own palette. Tidied init code.
    15/3/2002       AK - More init code tidying with a slight hack to stop
                         sound when the machine starts.
    19/3/2002       AK - Changed NVRAM code to the new battery_* functions.
    24/3/2002       AK - Added MBC1 mode switching, and partial MBC3 RTC support.
    28/3/2002       AK - Improved LCD status timing and interrupts.
                         Free memory when we shutdown instead of leaking.
    31/3/2002       AK - Handle IO memory reading so we return 0xFF for registers
                         that are unsupported.
     7/4/2002       AK - Free memory from battery load/save. General tidying.
    13/4/2002       AK - Ok, don't free memory when we shutdown as that causes
                         a crash on reset.
    28/4/2002       AK - General code tidying.
                         Fixed MBC3's RAM/RTC banking.
                         Added support for games with more than 128 ROM banks.
    12/6/2002       AK - Rewrote the way bg and sprite palettes are handled.
                         The window layer no longer has it's own palette.
                         Added Super Game Boy support.
    13/6/2002       AK - Added Game Boy Color support.

    17/5/2004       WP - Added Megaduck/Cougar Boy support.
    13/6/2005       WP - Added support for bootstrap rom banking.

***************************************************************************/
#define __MACHINE_GB_C

#include "emu.h"
#include "cpu/lr35902/lr35902.h"
#include "imagedev/cartslot.h"
#include "machine/ram.h"
#include "audio/gb.h"
#include "includes/gb.h"

/* Memory bank controller types */
enum {
	MBC_NONE=0,     /*  32KB ROM - No memory bank controller         */
	MBC_MBC1,       /*  ~2MB ROM,   8KB RAM -or- 512KB ROM, 32KB RAM */
	MBC_MBC2,       /* 256KB ROM,  32KB RAM                          */
	MBC_MMM01,      /*    ?? ROM,    ?? RAM                          */
	MBC_MBC3,       /*   2MB ROM,  32KB RAM, RTC                     */
	MBC_MBC4,       /*    ?? ROM,    ?? RAM                          */
	MBC_MBC5,       /*   8MB ROM, 128KB RAM (32KB w/ Rumble)         */
	MBC_TAMA5,      /*    ?? ROM     ?? RAM - What is this?          */
	MBC_HUC1,       /*    ?? ROM,    ?? RAM - Hudson Soft Controller */
	MBC_HUC3,       /*    ?? ROM,    ?? RAM - Hudson Soft Controller */
	MBC_MBC6,       /*    ?? ROM,  32KB SRAM                         */
	MBC_MBC7,       /*    ?? ROM,    ?? RAM                          */
	MBC_WISDOM,     /*    ?? ROM,    ?? RAM - Wisdom tree controller */
	MBC_MBC1_KOR,   /*   1MB ROM,    ?? RAM - Korean MBC1 variant    */
	MBC_YONGYONG,   /*    ?? ROM,    ?? RAM - Appears in Sonic 3D Blast 5 pirate */
	MBC_LASAMA,     /*    ?? ROM,    ?? RAM - Appears in La Sa Ma */
	MBC_ATVRACIN,
	MBC_MEGADUCK,   /* MEGADUCK style banking                        */
	MBC_UNKNOWN,    /* Unknown mapper                                */
};

/* machine.device<ram_device>(RAM_TAG)->pointer() layout defines */
#define CGB_START_VRAM_BANKS    0x0000
#define CGB_START_RAM_BANKS ( 2 * 8 * 1024 )

#define JOYPAD      m_gb_io[0x00]   /* Joystick: 1.1.P15.P14.P13.P12.P11.P10       */
#define SIODATA     m_gb_io[0x01]   /* Serial IO data buffer                       */
#define SIOCONT     m_gb_io[0x02]   /* Serial IO control register                  */
#define DIVREG      m_gb_io[0x04]   /* Divider register (???)                      */
#define TIMECNT     m_gb_io[0x05]   /* Timer counter. Gen. int. when it overflows  */
#define TIMEMOD     m_gb_io[0x06]   /* New value of TimeCount after it overflows   */
#define TIMEFRQ     m_gb_io[0x07]   /* Timer frequency and start/stop switch       */



/*
  Prototypes
*/


#ifdef MAME_DEBUG
/* #define V_GENERAL*/      /* Display general debug information */
/* #define V_BANK*/         /* Display bank switching debug information */
#endif

void gb_state::gb_init_regs()
{
	/* Initialize the registers */
	SIODATA = 0x00;
	SIOCONT = 0x7E;

	gb_io_w(m_maincpu->space(AS_PROGRAM), 0x05, 0x00);       /* TIMECNT */
	gb_io_w(m_maincpu->space(AS_PROGRAM), 0x06, 0x00);       /* TIMEMOD */
}


void gb_state::gb_init()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	gb_sound_w(machine().device("custom"), space, 0x16, 0x00);       /* Initialize sound hardware */

	m_divcount = 0;
	m_triggering_irq = 0;
	m_gb_io[0x07] = 0xF8;        /* Upper bits of TIMEFRQ register are set to 1 */
}


MACHINE_START_MEMBER(gb_state,gb)
{
	/* Allocate the serial timer, and disable it */
	m_gb_serial_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gb_state::gb_serial_timer_proc),this));
	m_gb_serial_timer->enable( 0 );

	MACHINE_START_CALL_MEMBER( gb_video );
}

MACHINE_START_MEMBER(gb_state,gbc)
{
	/* Allocate the serial timer, and disable it */
	m_gb_serial_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gb_state::gb_serial_timer_proc),this));
	m_gb_serial_timer->enable( 0 );

	MACHINE_START_CALL_MEMBER( gbc_video );
}

MACHINE_RESET_MEMBER(gb_state,gb)
{
	gb_init();

	gb_video_reset(GB_VIDEO_DMG);

	/* Enable BIOS rom */
	m_bios_disable = FALSE;

	m_divcount = 0x0004;
}


MACHINE_START_MEMBER(gb_state,sgb)
{
	m_sgb_packets = -1;
	m_sgb_tile_data = auto_alloc_array_clear(machine(), UINT8, 0x2000 );

	/* Allocate the serial timer, and disable it */
	m_gb_serial_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gb_state::gb_serial_timer_proc),this));
	m_gb_serial_timer->enable( 0 );

	MACHINE_START_CALL_MEMBER( gb_video );
}

MACHINE_RESET_MEMBER(gb_state,sgb)
{
	gb_init();

	gb_video_reset(GB_VIDEO_SGB);

	gb_init_regs();


	/* Enable BIOS rom */
	m_bios_disable = FALSE;

	memset(m_sgb_tile_data, 0, 0x2000);

	m_sgb_window_mask = 0;
	memset(m_sgb_pal_map, 0, sizeof(m_sgb_pal_map));
	memset(m_sgb_atf_data, 0, sizeof(m_sgb_atf_data));

	/* HACKS for Donkey Kong Land 2 + 3.
	   For some reason that I haven't figured out, they store the tile
	   data differently.  Hacks will go once I figure it out */
	m_sgb_hack = 0;

	if (m_cartslot->m_cart)  // make sure cart is in
	{
		if (strncmp((const char*)(m_cartslot->m_cart->get_rom_base() + 0x134), "DONKEYKONGLAND 2", 16) == 0 ||
			strncmp((const char*)(m_cartslot->m_cart->get_rom_base() + 0x134), "DONKEYKONGLAND 3", 16) == 0)
				m_sgb_hack = 1;
	}

	m_divcount = 0x0004;
}

MACHINE_RESET_MEMBER(gb_state,gbpocket)
{
	gb_init();

	gb_video_reset(GB_VIDEO_MGB);

	gb_init_regs();

	m_bios_disable = TRUE;

	/* Initialize the Sound registers */
	gb_sound_w(machine().device("custom"), generic_space(), 0x16,0x80);
	gb_sound_w(machine().device("custom"), generic_space(), 0x15,0xF3);
	gb_sound_w(machine().device("custom"), generic_space(), 0x14,0x77);

	m_divcount = 0xABC8;
}

MACHINE_RESET_MEMBER(gb_state,gbc)
{
	int ii;

	gb_init();

	gb_video_reset( GB_VIDEO_CGB );

	gb_init_regs();

	/* Enable BIOS rom */
	m_bios_disable = FALSE;

	/* Allocate memory for internal ram */
	for (ii = 0; ii < 8; ii++)
	{
		m_GBC_RAMMap[ii] = machine().device<ram_device>(RAM_TAG)->pointer() + CGB_START_RAM_BANKS + ii * 0x1000;
		memset(m_GBC_RAMMap[ii], 0, 0x1000);
	}
}

WRITE8_MEMBER(gb_state::gb_io_w)
{
	static const UINT8 timer_shifts[4] = {10, 4, 6, 8};

	switch (offset)
	{
	case 0x00:                      /* JOYP - Joypad */
		JOYPAD = 0xCF | data;
		if (!(data & 0x20))
			JOYPAD &= (m_inputs->read() >> 4) | 0xF0;
		if (!(data & 0x10))
			JOYPAD &= m_inputs->read() | 0xF0;
		return;
	case 0x01:                      /* SB - Serial transfer data */
		break;
	case 0x02:                      /* SC - SIO control */
		switch( data & 0x81 )
		{
		case 0x00:
		case 0x01:
		case 0x80:              /* enabled & external clock */
			m_SIOCount = 0;
			break;
		case 0x81:              /* enabled & internal clock */
			SIODATA = 0xFF;
			m_SIOCount = 8;
			m_gb_serial_timer->adjust(m_maincpu->cycles_to_attotime(512), 0, m_maincpu->cycles_to_attotime(512));
			m_gb_serial_timer->enable( 1 );
			break;
		}
		break;
	case 0x04:                      /* DIV - Divider register */
		/* Force increment of TIMECNT register */
		if ( m_divcount >= 16 )
			gb_timer_increment();
		m_divcount = 0;
		return;
	case 0x05:                      /* TIMA - Timer counter */
		/* Check if the counter is being reloaded in this cycle */
		if ( m_reloading && ( m_divcount & ( m_shift_cycles - 1 ) ) == 4 )
		{
			data = TIMECNT;
		}
		break;
	case 0x06:                      /* TMA - Timer module */
		/* Check if the counter is being reloaded in this cycle */
		if ( m_reloading && ( m_divcount & ( m_shift_cycles - 1 ) ) == 4 )
		{
			TIMECNT = data;
		}
		break;
	case 0x07:                      /* TAC - Timer control */
		data |= 0xF8;
		/* Check if timer is just disabled or the timer frequency is changing */
		if ( ( ! ( data & 0x04 ) && ( TIMEFRQ & 0x04 ) ) || ( ( data & 0x04 ) && ( TIMEFRQ & 0x04 ) && ( data & 0x03 ) != ( TIMEFRQ & 0x03 ) ) )
		{
			/* Check if TIMECNT should be incremented */
			if ( ( m_divcount & ( m_shift_cycles - 1 ) ) >= ( m_shift_cycles >> 1 ) )
			{
				gb_timer_increment();
			}
		}
		m_shift = timer_shifts[data & 0x03];
		m_shift_cycles = 1 << m_shift;
		break;
	case 0x0F:                      /* IF - Interrupt flag */
		data &= 0x1F;
		machine().device<lr35902_cpu_device>(":maincpu")->set_if( data );
		break;
	}

	m_gb_io[offset] = data;
}

WRITE8_MEMBER(gb_state::gb_io2_w)
{
	if (offset == 0x10)
	{
		/* disable BIOS ROM */
		m_bios_disable = TRUE;
		//printf("here again?\n");
	}
	else
		gb_video_w(space, offset, data);
}

#ifdef MAME_DEBUG
static const char *const sgbcmds[26] =
{
	"PAL01   ",
	"PAL23   ",
	"PAL03   ",
	"PAL12   ",
	"ATTR_BLK",
	"ATTR_LIN",
	"ATTR_DIV",
	"ATTR_CHR",
	"SOUND   ",
	"SOU_TRN ",
	"PAL_SET ",
	"PAL_TRN ",
	"ATRC_EN ",
	"TEST_EN ",
	"ICON_EN ",
	"DATA_SND",
	"DATA_TRN",
	"MLT_REG ",
	"JUMP    ",
	"CHR_TRN ",
	"PCT_TRN ",
	"ATTR_TRN",
	"ATTR_SET",
	"MASK_EN ",
	"OBJ_TRN ",
	"????????",
};
#endif

WRITE8_MEMBER(gb_state::sgb_io_w)
{
	UINT8 *sgb_data = m_sgb_data;

	switch( offset )
	{
		case 0x00:
			switch (data & 0x30)
			{
			case 0x00:                 /* start condition */
				if (m_sgb_start)
					logerror("SGB: Start condition before end of transfer ??\n");
				m_sgb_bitcount = 0;
				m_sgb_start = 1;
				m_sgb_rest = 0;
				JOYPAD = 0x0F & ((m_inputs->read() >> 4) | m_inputs->read() | 0xF0);
				break;
			case 0x10:                 /* data true */
				if (m_sgb_rest)
				{
					/* We should test for this case , but the code below won't
					   work with the current setup */
#if 0
					if (m_sgb_bytecount == 16)
					{
						logerror("SGB: end of block is not zero!");
						m_sgb_start = 0;
					}
#endif
					sgb_data[m_sgb_bytecount] >>= 1;
					sgb_data[m_sgb_bytecount] |= 0x80;
					m_sgb_bitcount++;
					if (m_sgb_bitcount == 8)
					{
						m_sgb_bitcount = 0;
						m_sgb_bytecount++;
					}
					m_sgb_rest = 0;
				}
				JOYPAD = 0x1F & ((m_inputs->read() >> 4) | 0xF0);
				break;
			case 0x20:              /* data false */
				if (m_sgb_rest)
				{
					if( m_sgb_bytecount == 16 && m_sgb_packets == -1 )
					{
#ifdef MAME_DEBUG
						logerror("SGB: %s (%02X) pkts: %d data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
								sgbcmds[sgb_data[0] >> 3],sgb_data[0] >> 3, sgb_data[0] & 0x07, sgb_data[1], sgb_data[2], sgb_data[3],
								sgb_data[4], sgb_data[5], sgb_data[6], sgb_data[7],
								sgb_data[8], sgb_data[9], sgb_data[10], sgb_data[11],
								sgb_data[12], sgb_data[13], sgb_data[14], sgb_data[15]);
#endif
						m_sgb_packets = sgb_data[0] & 0x07;
						m_sgb_start = 0;
					}
					if (m_sgb_bytecount == (m_sgb_packets << 4) )
					{
						switch( sgb_data[0] >> 3 )
						{
							case 0x00:  /* PAL01 */
								m_sgb_pal[0*4 + 0] = sgb_data[1] | (sgb_data[2] << 8);
								m_sgb_pal[0*4 + 1] = sgb_data[3] | (sgb_data[4] << 8);
								m_sgb_pal[0*4 + 2] = sgb_data[5] | (sgb_data[6] << 8);
								m_sgb_pal[0*4 + 3] = sgb_data[7] | (sgb_data[8] << 8);
								m_sgb_pal[1*4 + 0] = sgb_data[1] | (sgb_data[2] << 8);
								m_sgb_pal[1*4 + 1] = sgb_data[9] | (sgb_data[10] << 8);
								m_sgb_pal[1*4 + 2] = sgb_data[11] | (sgb_data[12] << 8);
								m_sgb_pal[1*4 + 3] = sgb_data[13] | (sgb_data[14] << 8);
								break;
							case 0x01:  /* PAL23 */
								m_sgb_pal[2*4 + 0] = sgb_data[1] | (sgb_data[2] << 8);
								m_sgb_pal[2*4 + 1] = sgb_data[3] | (sgb_data[4] << 8);
								m_sgb_pal[2*4 + 2] = sgb_data[5] | (sgb_data[6] << 8);
								m_sgb_pal[2*4 + 3] = sgb_data[7] | (sgb_data[8] << 8);
								m_sgb_pal[3*4 + 0] = sgb_data[1] | (sgb_data[2] << 8);
								m_sgb_pal[3*4 + 1] = sgb_data[9] | (sgb_data[10] << 8);
								m_sgb_pal[3*4 + 2] = sgb_data[11] | (sgb_data[12] << 8);
								m_sgb_pal[3*4 + 3] = sgb_data[13] | (sgb_data[14] << 8);
								break;
							case 0x02:  /* PAL03 */
								m_sgb_pal[0*4 + 0] = sgb_data[1] | (sgb_data[2] << 8);
								m_sgb_pal[0*4 + 1] = sgb_data[3] | (sgb_data[4] << 8);
								m_sgb_pal[0*4 + 2] = sgb_data[5] | (sgb_data[6] << 8);
								m_sgb_pal[0*4 + 3] = sgb_data[7] | (sgb_data[8] << 8);
								m_sgb_pal[3*4 + 0] = sgb_data[1] | (sgb_data[2] << 8);
								m_sgb_pal[3*4 + 1] = sgb_data[9] | (sgb_data[10] << 8);
								m_sgb_pal[3*4 + 2] = sgb_data[11] | (sgb_data[12] << 8);
								m_sgb_pal[3*4 + 3] = sgb_data[13] | (sgb_data[14] << 8);
								break;
							case 0x03:  /* PAL12 */
								m_sgb_pal[1*4 + 0] = sgb_data[1] | (sgb_data[2] << 8);
								m_sgb_pal[1*4 + 1] = sgb_data[3] | (sgb_data[4] << 8);
								m_sgb_pal[1*4 + 2] = sgb_data[5] | (sgb_data[6] << 8);
								m_sgb_pal[1*4 + 3] = sgb_data[7] | (sgb_data[8] << 8);
								m_sgb_pal[2*4 + 0] = sgb_data[1] | (sgb_data[2] << 8);
								m_sgb_pal[2*4 + 1] = sgb_data[9] | (sgb_data[10] << 8);
								m_sgb_pal[2*4 + 2] = sgb_data[11] | (sgb_data[12] << 8);
								m_sgb_pal[2*4 + 3] = sgb_data[13] | (sgb_data[14] << 8);
								break;
							case 0x04:  /* ATTR_BLK */
								{
									UINT8 I, J, K, o;
									for( K = 0; K < sgb_data[1]; K++ )
									{
										o = K * 6;
										if( sgb_data[o + 2] & 0x1 )
										{
											for( I = sgb_data[ o + 4]; I <= sgb_data[o + 6]; I++ )
											{
												for( J = sgb_data[o + 5]; J <= sgb_data[o + 7]; J++ )
												{
													m_sgb_pal_map[I][J] = sgb_data[o + 3] & 0x3;
												}
											}
										}
									}
								}
								break;
							case 0x05:  /* ATTR_LIN */
								{
									UINT8 J, K;
									if( sgb_data[1] > 15 )
										sgb_data[1] = 15;
									for( K = 0; K < sgb_data[1]; K++ )
									{
										if( sgb_data[K + 1] & 0x80 )
										{
											for( J = 0; J < 20; J++ )
											{
												m_sgb_pal_map[J][sgb_data[K + 1] & 0x1f] = (sgb_data[K + 1] & 0x60) >> 5;
											}
										}
										else
										{
											for( J = 0; J < 18; J++ )
											{
												m_sgb_pal_map[sgb_data[K + 1] & 0x1f][J] = (sgb_data[K + 1] & 0x60) >> 5;
											}
										}
									}
								}
								break;
							case 0x06:  /* ATTR_DIV */
								{
									UINT8 I, J;
									if( sgb_data[1] & 0x40 ) /* Vertical */
									{
										for( I = 0; I < sgb_data[2]; I++ )
										{
											for( J = 0; J < 20; J++ )
											{
												m_sgb_pal_map[J][I] = (sgb_data[1] & 0xC) >> 2;
											}
										}
										for( J = 0; J < 20; J++ )
										{
											m_sgb_pal_map[J][sgb_data[2]] = (sgb_data[1] & 0x30) >> 4;
										}
										for( I = sgb_data[2] + 1; I < 18; I++ )
										{
											for( J = 0; J < 20; J++ )
											{
												m_sgb_pal_map[J][I] = sgb_data[1] & 0x3;
											}
										}
									}
									else /* Horizontal */
									{
										for( I = 0; I < sgb_data[2]; I++ )
										{
											for( J = 0; J < 18; J++ )
											{
												m_sgb_pal_map[I][J] = (sgb_data[1] & 0xC) >> 2;
											}
										}
										for( J = 0; J < 18; J++ )
										{
											m_sgb_pal_map[sgb_data[2]][J] = (sgb_data[1] & 0x30) >> 4;
										}
										for( I = sgb_data[2] + 1; I < 20; I++ )
										{
											for( J = 0; J < 18; J++ )
											{
												m_sgb_pal_map[I][J] = sgb_data[1] & 0x3;
											}
										}
									}
								}
								break;
							case 0x07:  /* ATTR_CHR */
								{
									UINT16 I, sets;
									UINT8 x, y;
									sets = (sgb_data[3] | (sgb_data[4] << 8) );
									if( sets > 360 )
										sets = 360;
									sets >>= 2;
									sets += 6;
									x = sgb_data[1];
									y = sgb_data[2];
									if( sgb_data[5] ) /* Vertical */
									{
										for( I = 6; I < sets; I++ )
										{
											m_sgb_pal_map[x][y++] = (sgb_data[I] & 0xC0) >> 6;
											if( y > 17 )
											{
												y = 0;
												x++;
												if( x > 19 )
													x = 0;
											}

											m_sgb_pal_map[x][y++] = (sgb_data[I] & 0x30) >> 4;
											if( y > 17 )
											{
												y = 0;
												x++;
												if( x > 19 )
													x = 0;
											}

											m_sgb_pal_map[x][y++] = (sgb_data[I] & 0xC) >> 2;
											if( y > 17 )
											{
												y = 0;
												x++;
												if( x > 19 )
													x = 0;
											}

											m_sgb_pal_map[x][y++] = sgb_data[I] & 0x3;
											if( y > 17 )
											{
												y = 0;
												x++;
												if( x > 19 )
													x = 0;
											}
										}
									}
									else /* horizontal */
									{
										for( I = 6; I < sets; I++ )
										{
											m_sgb_pal_map[x++][y] = (sgb_data[I] & 0xC0) >> 6;
											if( x > 19 )
											{
												x = 0;
												y++;
												if( y > 17 )
													y = 0;
											}

											m_sgb_pal_map[x++][y] = (sgb_data[I] & 0x30) >> 4;
											if( x > 19 )
											{
												x = 0;
												y++;
												if( y > 17 )
													y = 0;
											}

											m_sgb_pal_map[x++][y] = (sgb_data[I] & 0xC) >> 2;
											if( x > 19 )
											{
												x = 0;
												y++;
												if( y > 17 )
													y = 0;
											}

											m_sgb_pal_map[x++][y] = sgb_data[I] & 0x3;
											if( x > 19 )
											{
												x = 0;
												y++;
												if( y > 17 )
													y = 0;
											}
										}
									}
								}
								break;
							case 0x08:  /* SOUND */
								/* This command enables internal sound effects */
								/* Not Implemented */
								break;
							case 0x09:  /* SOU_TRN */
								/* This command sends data to the SNES sound processor.
								   We'll need to emulate that for this to be used */
								/* Not Implemented */
								break;
							case 0x0A:  /* PAL_SET */
								{
									UINT16 index_, J, I;

									/* Palette 0 */
									index_ = (UINT16)(sgb_data[1] | (sgb_data[2] << 8)) * 4;
									m_sgb_pal[0] = m_sgb_pal_data[index_];
									m_sgb_pal[1] = m_sgb_pal_data[index_ + 1];
									m_sgb_pal[2] = m_sgb_pal_data[index_ + 2];
									m_sgb_pal[3] = m_sgb_pal_data[index_ + 3];
									/* Palette 1 */
									index_ = (UINT16)(sgb_data[3] | (sgb_data[4] << 8)) * 4;
									m_sgb_pal[4] = m_sgb_pal_data[index_];
									m_sgb_pal[5] = m_sgb_pal_data[index_ + 1];
									m_sgb_pal[6] = m_sgb_pal_data[index_ + 2];
									m_sgb_pal[7] = m_sgb_pal_data[index_ + 3];
									/* Palette 2 */
									index_ = (UINT16)(sgb_data[5] | (sgb_data[6] << 8)) * 4;
									m_sgb_pal[8] = m_sgb_pal_data[index_];
									m_sgb_pal[9] = m_sgb_pal_data[index_ + 1];
									m_sgb_pal[10] = m_sgb_pal_data[index_ + 2];
									m_sgb_pal[11] = m_sgb_pal_data[index_ + 3];
									/* Palette 3 */
									index_ = (UINT16)(sgb_data[7] | (sgb_data[8] << 8)) * 4;
									m_sgb_pal[12] = m_sgb_pal_data[index_];
									m_sgb_pal[13] = m_sgb_pal_data[index_ + 1];
									m_sgb_pal[14] = m_sgb_pal_data[index_ + 2];
									m_sgb_pal[15] = m_sgb_pal_data[index_ + 3];
									/* Attribute File */
									if( sgb_data[9] & 0x40 )
										m_sgb_window_mask = 0;
									m_sgb_atf = (sgb_data[9] & 0x3f) * (18 * 5);
									if( sgb_data[9] & 0x80 )
									{
										for( J = 0; J < 18; J++ )
										{
											for( I = 0; I < 5; I++ )
											{
												m_sgb_pal_map[I * 4][J] = (m_sgb_atf_data[(J * 5) + m_sgb_atf + I] & 0xC0) >> 6;
												m_sgb_pal_map[(I * 4) + 1][J] = (m_sgb_atf_data[(J * 5) + m_sgb_atf + I] & 0x30) >> 4;
												m_sgb_pal_map[(I * 4) + 2][J] = (m_sgb_atf_data[(J * 5) + m_sgb_atf + I] & 0xC) >> 2;
												m_sgb_pal_map[(I * 4) + 3][J] = m_sgb_atf_data[(J * 5) + m_sgb_atf + I] & 0x3;
											}
										}
									}
								}
								break;
							case 0x0B:  /* PAL_TRN */
								{
									UINT16 I, col;

									for( I = 0; I < 2048; I++ )
									{
										col = ( m_lcd.gb_vram_ptr[ 0x0800 + (I*2) + 1 ] << 8 ) | m_lcd.gb_vram_ptr[ 0x0800 + (I*2) ];
										m_sgb_pal_data[I] = col;
									}
								}
								break;
							case 0x0C:  /* ATRC_EN */
								/* Not Implemented */
								break;
							case 0x0D:  /* TEST_EN */
								/* Not Implemented */
								break;
							case 0x0E:  /* ICON_EN */
								/* Not Implemented */
								break;
							case 0x0F:  /* DATA_SND */
								/* Not Implemented */
								break;
							case 0x10:  /* DATA_TRN */
								/* Not Implemented */
								break;
							case 0x11:  /* MLT_REQ - Multi controller request */
								if (sgb_data[1] == 0x00)
									m_sgb_controller_mode = 0;
								else if (sgb_data[1] == 0x01)
									m_sgb_controller_mode = 2;
								break;
							case 0x12:  /* JUMP */
								/* Not Implemented */
								break;
							case 0x13:  /* CHR_TRN */
								if( sgb_data[1] & 0x1 )
									memcpy( m_sgb_tile_data + 4096, m_lcd.gb_vram_ptr + 0x0800, 4096 );
								else
									memcpy( m_sgb_tile_data, m_lcd.gb_vram_ptr + 0x0800, 4096 );
								break;
							case 0x14:  /* PCT_TRN */
								{
									int I;
									UINT16 col;
									if( m_sgb_hack )
									{
										memcpy( m_sgb_tile_map, m_lcd.gb_vram_ptr + 0x1000, 2048 );
										for( I = 0; I < 64; I++ )
										{
											col = ( m_lcd.gb_vram_ptr[ 0x0800 + (I*2) + 1 ] << 8 ) | m_lcd.gb_vram_ptr[ 0x0800 + (I*2) ];
											m_sgb_pal[SGB_BORDER_PAL_OFFSET + I] = col;
										}
									}
									else /* Do things normally */
									{
										memcpy( m_sgb_tile_map, m_lcd.gb_vram_ptr + 0x0800, 2048 );
										for( I = 0; I < 64; I++ )
										{
											col = ( m_lcd.gb_vram_ptr[ 0x1000 + (I*2) + 1 ] << 8 ) | m_lcd.gb_vram_ptr[ 0x1000 + (I*2) ];
											m_sgb_pal[SGB_BORDER_PAL_OFFSET + I] = col;
										}
									}
								}
								break;
							case 0x15:  /* ATTR_TRN */
								memcpy( m_sgb_atf_data, m_lcd.gb_vram_ptr + 0x0800, 4050 );
								break;
							case 0x16:  /* ATTR_SET */
								{
									UINT8 J, I;

									/* Attribute File */
									if( sgb_data[1] & 0x40 )
										m_sgb_window_mask = 0;
									m_sgb_atf = (sgb_data[1] & 0x3f) * (18 * 5);
									for( J = 0; J < 18; J++ )
									{
										for( I = 0; I < 5; I++ )
										{
											m_sgb_pal_map[I * 4][J] = (m_sgb_atf_data[(J * 5) + m_sgb_atf + I] & 0xC0) >> 6;
											m_sgb_pal_map[(I * 4) + 1][J] = (m_sgb_atf_data[(J * 5) + m_sgb_atf + I] & 0x30) >> 4;
											m_sgb_pal_map[(I * 4) + 2][J] = (m_sgb_atf_data[(J * 5) + m_sgb_atf + I] & 0xC) >> 2;
											m_sgb_pal_map[(I * 4) + 3][J] = m_sgb_atf_data[(J * 5) + m_sgb_atf + I] & 0x3;
										}
									}
								}
								break;
							case 0x17:  /* MASK_EN */
								m_sgb_window_mask = sgb_data[1];
								break;
							case 0x18:  /* OBJ_TRN */
								/* Not Implemnted */
								break;
							case 0x19:  /* ? */
								/* Called by: dkl,dkl2,dkl3,zeldadx
								   But I don't know what it is for. */
								/* Not Implemented */
								break;
							case 0x1E:  /* Used by bootrom to transfer the gb cart header */
								break;
							case 0x1F:  /* Used by bootrom to transfer the gb cart header */
								break;
							default:
								logerror( "SGB: Unknown Command 0x%02x!\n", sgb_data[0] >> 3 );
						}

						m_sgb_start = 0;
						m_sgb_bytecount = 0;
						m_sgb_packets = -1;
					}
					if( m_sgb_start )
					{
						sgb_data[m_sgb_bytecount] >>= 1;
						m_sgb_bitcount++;
						if (m_sgb_bitcount == 8)
						{
							m_sgb_bitcount = 0;
							m_sgb_bytecount++;
						}
					}
					m_sgb_rest = 0;
				}
				JOYPAD = 0x2F & (m_inputs->read() | 0xF0);
				break;
			case 0x30:                 /* rest condition */
				if (m_sgb_start)
					m_sgb_rest = 1;
				if (m_sgb_controller_mode)
				{
					m_sgb_controller_no++;
					if (m_sgb_controller_no == m_sgb_controller_mode)
						m_sgb_controller_no = 0;
					JOYPAD = 0x3F - m_sgb_controller_no;
				}
				else
					JOYPAD = 0x3F;

				/* Hack to let cartridge know it's running on an SGB */
				if ( (sgb_data[0] >> 3) == 0x1F )
					JOYPAD = 0x3E;
				break;
			}
			return;
		default:
			/* we didn't handle the write, so pass it to the GB handler */
			gb_io_w( space, offset, data );
			return;
	}

	m_gb_io[offset] = data;
}

/* Interrupt Enable register */
READ8_MEMBER(gb_state::gb_ie_r)
{
	return machine().device<lr35902_cpu_device>(":maincpu")->get_ie();
}

WRITE8_MEMBER(gb_state::gb_ie_w)
{
	machine().device<lr35902_cpu_device>(":maincpu")->set_ie( data & 0x1F );
}

/* IO read */
READ8_MEMBER(gb_state::gb_io_r)
{
	switch(offset)
	{
		case 0x04:
			return ( m_divcount >> 8 ) & 0xFF;
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x05:
		case 0x06:
		case 0x07:
			return m_gb_io[offset];
		case 0x0F:
			/* Make sure the internal states are up to date */
			return 0xE0 | machine().device<lr35902_cpu_device>(":maincpu")->get_if();
		default:
			/* It seems unsupported registers return 0xFF */
			return 0xFF;
	}
}


INTERRUPT_GEN_MEMBER(gb_state::gb_scanline_interrupt)
{
}

TIMER_CALLBACK_MEMBER(gb_state::gb_serial_timer_proc)
{
	/* Shift in a received bit */
	SIODATA = (SIODATA << 1) | 0x01;
	/* Decrement number of handled bits */
	m_SIOCount--;
	/* If all bits done, stop timer and trigger interrupt */
	if ( ! m_SIOCount )
	{
		SIOCONT &= 0x7F;
		m_gb_serial_timer->enable( 0 );
		m_maincpu->set_input_line(SIO_INT, ASSERT_LINE);
	}
}

void gb_state::gb_timer_check_irq()
{
	m_reloading = 0;
	if ( m_triggering_irq )
	{
		m_triggering_irq = 0;
		if ( TIMECNT == 0 )
		{
			TIMECNT = TIMEMOD;
			m_maincpu->set_input_line(TIM_INT, ASSERT_LINE );
			m_reloading = 1;
		}
	}
}

void gb_state::gb_timer_increment()
{
	gb_timer_check_irq();

	TIMECNT += 1;
	if ( TIMECNT == 0 )
	{
		m_triggering_irq = 1;
	}
}

WRITE8_MEMBER( gb_state::gb_timer_callback )
{
	UINT16 old_gb_divcount = m_divcount;
	m_divcount += data;

	gb_timer_check_irq();

	if ( TIMEFRQ & 0x04 )
	{
		UINT16 old_count = old_gb_divcount >> m_shift;
		UINT16 new_count = m_divcount >> m_shift;
		if ( data > m_shift_cycles )
		{
			gb_timer_increment();
			old_count++;
		}
		if ( new_count != old_count )
		{
			gb_timer_increment();
		}
		if ( new_count << m_shift < m_divcount )
		{
			gb_timer_check_irq();
		}
	}
}

WRITE8_MEMBER(gb_state::gbc_io2_w)
{
	switch( offset )
	{
		case 0x0D:  /* KEY1 - Prepare speed switch */
			machine().device<lr35902_cpu_device>(":maincpu")->set_speed(data);
			return;
		case 0x10:  /* BFF - Bios disable */
			m_bios_disable = TRUE;
			return;
		case 0x16:  /* RP - Infrared port */
			break;
		case 0x30:  /* SVBK - RAM bank select */
			m_GBC_RAMBank = data & 0x7;
			if (!m_GBC_RAMBank)
				m_GBC_RAMBank = 1;
			m_rambank->set_base(m_GBC_RAMMap[m_GBC_RAMBank]);
			break;
		default:
			break;
	}
	gbc_video_w( space, offset, data );
}

READ8_MEMBER(gb_state::gbc_io2_r)
{
	switch( offset )
	{
	case 0x0D:  /* KEY1 */
		return machine().device<lr35902_cpu_device>(":maincpu")->get_speed();
	case 0x16:  /* RP - Infrared port */
		break;
	case 0x30:  /* SVBK - RAM bank select */
		return m_GBC_RAMBank;
	default:
		break;
	}
	return gbc_video_r( space, offset );
}

/****************************************************************************

  Megaduck routines

 ****************************************************************************/

MACHINE_START_MEMBER(megaduck_state,megaduck)
{
	/* Allocate the serial timer, and disable it */
	m_gb_serial_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gb_state::gb_serial_timer_proc),this));
	m_gb_serial_timer->enable( 0 );

	MACHINE_START_CALL_MEMBER( gb_video );
}

MACHINE_RESET_MEMBER(megaduck_state,megaduck)
{
	/* We may have to add some more stuff here, if not then it can be merged back into gb */
	gb_init();
	
	m_bios_disable = TRUE;

	gb_video_reset( GB_VIDEO_DMG );
}

/*
 Map megaduck video related area on to regular Game Boy video area

 Different locations of the video registers:
 Register      Game Boy   MegaDuck
 LCDC          FF40       FF10  (See different bit order below)
 STAT          FF41       FF11
 SCY           FF42       FF12
 SCX           FF43       FF13
 LY            FF44       FF18
 LYC           FF45       FF19
 DMA           FF46       FF1A
 BGP           FF47       FF1B
 OBP0          FF48       FF14
 OBP1          FF49       FF15
 WY            FF4A       FF16
 WX            FF4B       FF17
 Unused        FF4C       FF4C (?)
 Unused        FF4D       FF4D (?)
 Unused        FF4E       FF4E (?)
 Unused        FF4F       FF4F (?)

 Different LCDC register

 Game Boy       MegaDuck
 0                      6       - BG & Window Display : 0 - Off, 1 - On
 1                      0       - OBJ Display: 0 - Off, 1 - On
 2                      1       - OBJ Size: 0 - 8x8, 1 - 8x16
 3                      2       - BG Tile Map Display: 0 - 9800, 1 - 9C00
 4                      4       - BG & Window Tile Data Select: 0 - 8800, 1 - 8000
 5                      5       - Window Display: 0 - Off, 1 - On
 6                      3       - Window Tile Map Display Select: 0 - 9800, 1 - 9C00
 7                      7       - LCD Operation

 **************/

READ8_MEMBER(megaduck_state::megaduck_video_r)
{
	UINT8 data;

	if ( (offset & 0x0C) && ((offset & 0x0C) ^ 0x0C) )
	{
		offset ^= 0x0C;
	}
	data = gb_video_r( space, offset );
	if ( offset )
		return data;
	return BITSWAP8(data,7,0,5,4,6,3,2,1);
}

WRITE8_MEMBER(megaduck_state::megaduck_video_w)
{
	if ( !offset )
	{
		data = BITSWAP8(data,7,3,5,4,2,1,0,6);
	}
	if ( (offset & 0x0C) && ((offset & 0x0C) ^ 0x0C) )
	{
		offset ^= 0x0C;
	}
	gb_video_w(space, offset, data );
}

/* Map megaduck audio offset to game boy audio offsets */

static const UINT8 megaduck_sound_offsets[16] = { 0, 2, 1, 3, 4, 6, 5, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

WRITE8_MEMBER(megaduck_state::megaduck_sound_w1)
{
	gb_sound_w(machine().device("custom"), space, megaduck_sound_offsets[offset], data );
}

READ8_MEMBER(megaduck_state::megaduck_sound_r1)
{
	return gb_sound_r( machine().device("custom"), space, megaduck_sound_offsets[offset] );
}

WRITE8_MEMBER(megaduck_state::megaduck_sound_w2)
{
	switch(offset)
	{
		case 0x00:  gb_sound_w(machine().device("custom"), space, 0x10, data ); break;
		case 0x01:  gb_sound_w(machine().device("custom"), space, 0x12, data ); break;
		case 0x02:  gb_sound_w(machine().device("custom"), space, 0x11, data ); break;
		case 0x03:  gb_sound_w(machine().device("custom"), space, 0x13, data ); break;
		case 0x04:  gb_sound_w(machine().device("custom"), space, 0x14, data ); break;
		case 0x05:  gb_sound_w(machine().device("custom"), space, 0x16, data ); break;
		case 0x06:  gb_sound_w(machine().device("custom"), space, 0x15, data ); break;
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F:
			break;
	}
}

READ8_MEMBER(megaduck_state::megaduck_sound_r2)
{
	return gb_sound_r(machine().device("custom"), space, 0x10 + megaduck_sound_offsets[offset]);
}

