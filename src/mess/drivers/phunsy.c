/***************************************************************************

        PHUNSY (Philipse Universal System)

        04/11/2010 Skeleton driver.

        http://www.tubedata.info/phunsy/index.html

        Cassette added 2012-05-24
        Baud Rate ~ 6000 baud
        W command to save data, eg 800-8FFW
        R command to read data, eg 1100R to load the file at 1100,
               or R to load the file where it came from.
        The tape must already be playing the leader when you press the Enter
        key, or it errors immediately.

****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/keyboard.h"
#include "sound/speaker.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"


#define LOG 1


class phunsy_state : public driver_device
{
public:
	phunsy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_speaker(*this, SPEAKER_TAG),
	m_cass(*this, CASSETTE_TAG),
	m_videoram(*this, "videoram") { }

	DECLARE_READ8_MEMBER( phunsy_data_r );
	DECLARE_WRITE8_MEMBER( phunsy_1800_w );
	DECLARE_WRITE8_MEMBER( phunsy_ctrl_w );
	DECLARE_WRITE8_MEMBER( phunsy_data_w );
	DECLARE_WRITE8_MEMBER( kbd_put );
	DECLARE_READ8_MEMBER(cass_r);
	DECLARE_WRITE8_MEMBER(cass_w);
	const UINT8 *m_p_chargen;
	UINT8       m_data_out;
	UINT8       m_keyboard_input;
	UINT8       m_q_bank;
	UINT8       m_u_bank;
	UINT8       m_ram_1800[0x800];
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cass;
	required_shared_ptr<UINT8> m_videoram;
	virtual void palette_init();
};


WRITE8_MEMBER( phunsy_state::phunsy_1800_w )
{
	if ( m_u_bank == 0 )
		m_ram_1800[offset] = data;
}

WRITE8_MEMBER( phunsy_state::cass_w )
{
	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
}

READ8_MEMBER( phunsy_state::cass_r )
{
	return (m_cass->input() > 0.03) ? 0 : 1;
}

static ADDRESS_MAP_START(phunsy_mem, AS_PROGRAM, 8, phunsy_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff) AM_ROM
	AM_RANGE( 0x0800, 0x0fff) AM_RAM
	AM_RANGE( 0x1000, 0x17ff) AM_RAM AM_SHARE("videoram") // Video RAM
	AM_RANGE( 0x1800, 0x1fff) AM_RAM_WRITE( phunsy_1800_w ) AM_ROMBANK("bank1") // Banked RAM/ROM
	AM_RANGE( 0x4000, 0xffff) AM_RAMBANK("bank2") // Banked RAM
ADDRESS_MAP_END


WRITE8_MEMBER( phunsy_state::phunsy_ctrl_w )
{
	if (LOG)
		logerror("%s: phunsy_ctrl_w %02x\n", machine().describe_context(), data);

	m_u_bank = data >> 4;
	m_q_bank = data & 0x0F;

	switch( m_u_bank )
	{
	case 0x00:  /* RAM */
		membank( "bank1" )->set_base( m_ram_1800 );
		break;
	case 0x01:  /* MDCR program */
	case 0x02:  /* Disassembler */
	case 0x03:  /* Label handler */
		membank( "bank1" )->set_base( memregion("maincpu")->base() + ( 0x800 * m_u_bank ) );
		break;
	default:    /* Not used */
		break;
	}

	membank( "bank2" )->set_base( memregion("ram_4000")->base() + 0x4000 * m_q_bank );
}


WRITE8_MEMBER( phunsy_state::phunsy_data_w )
{
	if (LOG)
		logerror("%s: phunsy_data_w %02x\n", machine().describe_context(), data);

	m_data_out = data;

	/* b0 - TTY out */
	/* b1 - select MDCR / keyboard */
	/* b2 - Clear keyboard strobe signal */
	if ( data & 0x04 )
	{
		m_keyboard_input |= 0x80;
	}

	/* b3 - speaker output (manual says it is bit 1)*/
	speaker_level_w(m_speaker, BIT(data, 1));

	/* b4 - -REV MDCR output */
	/* b5 - -FWD MDCR output */
	/* b6 - -WCD MDCR output */
	/* b7 - WDA MDCR output */
}


READ8_MEMBER( phunsy_state::phunsy_data_r )
{
	UINT8 data;

	if (LOG)
		logerror("%s: phunsy_data_r\n", machine().describe_context());

	if ( m_data_out & 0x02 )
	{
		/* MDCR selected */
		/* b0 - TTY input */
		/* b1 - SK1 switch input */
		/* b2 - SK2 switch input */
		/* b3 - -WEN MDCR input */
		/* b4 - -CIP MDCR input */
		/* b5 - -BET MDCR input */
		/* b6 - RDA MDCR input */
		/* b7 - RDC MDCR input */
		data = 0xFF;
	}
	else
	{
		/* Keyboard selected */
		/* b0-b6 - ASCII code from keyboard */
		/* b7    - strobe signal */
		data = m_keyboard_input;
	}

	return data;
}


static ADDRESS_MAP_START( phunsy_io, AS_IO, 8, phunsy_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( S2650_CTRL_PORT, S2650_CTRL_PORT ) AM_WRITE( phunsy_ctrl_w )
	AM_RANGE( S2650_DATA_PORT,S2650_DATA_PORT) AM_READWRITE( phunsy_data_r, phunsy_data_w )
	AM_RANGE(S2650_SENSE_PORT, S2650_FO_PORT) AM_READWRITE(cass_r, cass_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( phunsy )
INPUT_PORTS_END


WRITE8_MEMBER( phunsy_state::kbd_put )
{
	if (data)
		m_keyboard_input = data;
}


static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(phunsy_state, kbd_put)
};


void phunsy_state::machine_reset()
{
	membank( "bank1" )->set_base( m_ram_1800 );
	membank( "bank2" )->set_base( memregion("ram_4000")->base() );

	m_u_bank = 0;
	m_q_bank = 0;
	m_keyboard_input = 0xFF;
}


void phunsy_state::palette_init()
{
	for ( int i = 0; i < 8; i++ )
	{
		int j = ( i << 5 ) | ( i << 2 ) | ( i >> 1 );

		palette_set_color_rgb( machine(), i, j, j, j );
	}
}


void phunsy_state::video_start()
{
	m_p_chargen = memregion( "chargen" )->base();
}


UINT32 phunsy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx,col;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 32; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma+64; x++)
			{
				chr = m_videoram[x];

				if (BIT(chr, 7))
				{
					/* Graphics mode */
					gfx = 0;
					col = ( chr >> 4 ) & 7;
					if ( (BIT(chr, 0) && (!BIT(ra, 2))) || (BIT(chr, 2) && (BIT(ra, 2))) )
						gfx = 0x38;
					if ( (BIT(chr, 1) && (!BIT(ra, 2))) || (BIT(chr, 3) && (BIT(ra, 2))) )
						gfx |= 7;
				}
				else
				{
					/* ASCII mode */
					gfx = m_p_chargen[(chr<<3) | ra];
					col = 7;
				}

				/* Display a scanline of a character (6 pixels) */
				*p++ = BIT( gfx, 5 ) ? col : 0;
				*p++ = BIT( gfx, 4 ) ? col : 0;
				*p++ = BIT( gfx, 3 ) ? col : 0;
				*p++ = BIT( gfx, 2 ) ? col : 0;
				*p++ = BIT( gfx, 1 ) ? col : 0;
				*p++ = BIT( gfx, 0 ) ? col : 0;
			}
		}
		ma+=64;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout phunsy_charlayout =
{
	5, 7,                   /* 6 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( phunsy )
	GFXDECODE_ENTRY( "chargen", 0x0000, phunsy_charlayout, 1, 3 )
GFXDECODE_END


static MACHINE_CONFIG_START( phunsy, phunsy_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(phunsy_mem)
	MCFG_CPU_IO_MAP(phunsy_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	/* Display (page 12 of pdf)
	   - 8Mhz clock
	   - 64 6 pixel characters on a line.
	   - 16us not active, 48us active: ( 64 * 6 ) * 60 / 48 => 480 pixels wide
	   - 313 line display of which 256 are displayed.
	*/
	MCFG_SCREEN_RAW_PARAMS(XTAL_8MHz, 480, 0, 64*6, 313, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(phunsy_state, screen_update)
	MCFG_GFXDECODE(phunsy)
	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
	MCFG_CASSETTE_ADD( CASSETTE_TAG, default_cassette_interface )
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( phunsy )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "phunsy_bios.bin", 0x0000, 0x0800, CRC(a789e82e) SHA1(b1c130ab2b3c139fd16ddc5dc7bdcaf7a9957d02))
	ROM_LOAD( "pdcr.bin",        0x0800, 0x0800, CRC(74bf9d0a) SHA1(8d2f673615215947f033571f1221c6aa99c537e9))
	ROM_LOAD( "dass.bin",        0x1000, 0x0800, CRC(13380140) SHA1(a999201cb414abbf1e10a7fcc1789e3e000a5ef1))
	ROM_LOAD( "labhnd.bin",      0x1800, 0x0800, CRC(1d5a106b) SHA1(a20d09e32e21cf14db8254cbdd1d691556b473f0))

	ROM_REGION( 0x0400, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "ph_char1.bin", 0x0200, 0x0100, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee))
	ROM_CONTINUE(0x0100, 0x0100)
	ROM_LOAD( "ph_char2.bin", 0x0000, 0x0100, CRC(3d5786d3) SHA1(8cf87d83be0b5e4becfa9fd6e05b01250a2feb3b))
	ROM_CONTINUE(0x0300, 0x0100)

	/* 16 x 16KB RAM banks */
	ROM_REGION( 0x40000, "ram_4000", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY        FULLNAME       FLAGS */
COMP( 1980, phunsy,  0,       0,     phunsy,    phunsy, driver_device,   0, "J.F.P. Philipse", "PHUNSY", GAME_NOT_WORKING )
