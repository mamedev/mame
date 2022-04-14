// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

PHUNSY (Philipse Universal System)

2010-11-04 Skeleton driver.
2012-05-24 Cassette added.
2014-01-13 Quickload added.

http://www.tubedata.info/phunsy/index.html

Baud Rate ~ 6000 baud
W command to save data, eg 800-8FFW
R command to read data, eg 1100R to load the file at 1100,
   or R to load the file where it came from.
The tape must already be playing the leader when you press the Enter
   key, or it errors immediately.

Rom banking (in U bank):
 0U: RAM
 1U: MDCR program
 2U: Disassembler
 3U: Label handler


****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/keyboard.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define LOG 1

class phunsy_state : public driver_device
{
public:
	phunsy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cass(*this, "cassette")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{
	}

	void phunsy(machine_config &config);

	void init_phunsy();

private:
	uint8_t phunsy_data_r();
	void phunsy_ctrl_w(uint8_t data);
	void phunsy_data_w(uint8_t data);
	void kbd_put(u8 data);
	DECLARE_READ_LINE_MEMBER(cass_r);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	void phunsy_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void phunsy_data(address_map &map);
	void phunsy_io(address_map &map);
	void phunsy_mem(address_map &map);

	uint8_t       m_data_out = 0U;
	uint8_t       m_keyboard_input = 0U;
	virtual void machine_reset() override;
	required_device<s2650_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cass;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};


WRITE_LINE_MEMBER( phunsy_state::cass_w )
{
	m_cass->output(state ? -1.0 : +1.0);
}

READ_LINE_MEMBER(phunsy_state::cass_r)
{
	return (m_cass->input() > 0.03) ? 0 : 1;
}

void phunsy_state::phunsy_mem(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("roms", 0);
	map(0x0800, 0x0fff).ram();
	map(0x1000, 0x17ff).ram().share("videoram"); // Video RAM
	map(0x1800, 0x1fff).bankr("bankru").bankw("bankwu"); // Banked RAM/ROM
	map(0x2000, 0x3fff).ram();
	map(0x4000, 0x7fff).bankrw("bankq"); // Banked RAM
}

void phunsy_state::phunsy_io(address_map &map)
{
	map.unmap_value_high();
}

void phunsy_state::phunsy_data(address_map &map)
{
	map.unmap_value_high();
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).w(FUNC(phunsy_state::phunsy_ctrl_w));
	map(S2650_DATA_PORT, S2650_DATA_PORT).rw(FUNC(phunsy_state::phunsy_data_r), FUNC(phunsy_state::phunsy_data_w));
}


void phunsy_state::phunsy_ctrl_w(uint8_t data)
{
	if (LOG)
		logerror("%s: phunsy_ctrl_w %02x\n", machine().describe_context(), data);

	// Q-bank
	membank("bankq")->set_entry(data & 15);

	// U-bank
	data >>= 4;

	if (data < 4)
		membank("bankru")->set_entry(data);
}


void phunsy_state::phunsy_data_w(uint8_t data)
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
	m_speaker->level_w(BIT(data, 1));

	/* b4 - -REV MDCR output */
	/* b5 - -FWD MDCR output */
	/* b6 - -WCD MDCR output */
	/* b7 - WDA MDCR output */
}


uint8_t phunsy_state::phunsy_data_r()
{
	uint8_t data = 0xff;

	//if (LOG)
		//logerror("%s: phunsy_data_r\n", machine().describe_context());

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


/* Input ports */
static INPUT_PORTS_START( phunsy )
INPUT_PORTS_END


void phunsy_state::kbd_put(u8 data)
{
	if (data)
		m_keyboard_input = data;
}


void phunsy_state::machine_reset()
{
	membank("bankru")->set_entry(0); // point at ram
	membank("bankq" )->set_base( memregion("ram_4000")->base() );
	m_keyboard_input = 0xFF;
	m_data_out = 0;
}


void phunsy_state::phunsy_palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, pal3bit(i), pal3bit(i), pal3bit(i));
}


uint32_t phunsy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=0;

	for (uint8_t y = 0; y < 32; y++)
	{
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma+64; x++)
			{
				uint8_t const chr = m_p_videoram[x];

				uint8_t gfx,col;
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

static GFXDECODE_START( gfx_phunsy )
	GFXDECODE_ENTRY( "chargen", 0x0000, phunsy_charlayout, 1, 3 )
GFXDECODE_END

// quickloads can start from various addresses, and the files have no header.
QUICKLOAD_LOAD_MEMBER(phunsy_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t i;
	uint16_t quick_addr = 0x1800;
	std::vector<uint8_t> quick_data;
	image_init_result result = image_init_result::FAIL;
	int quick_length = image.length();
	if (quick_length > 0x4000)
	{
		image.seterror(image_error::INVALIDIMAGE, "File too long");
		image.message(" File too long");
	}
	else
	{
		quick_data.resize(quick_length);
		membank("bankru")->set_entry(0); // point at ram

		uint16_t exec_addr = quick_addr + 2;

		for (i = 0; i < quick_length; i++)
			space.write_byte(i+quick_addr, quick_data[i]);

		/* display a message about the loaded quickload */
		image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

		// Start the quickload
		m_maincpu->set_state_int(S2650_R0, exec_addr>>8);
		m_maincpu->set_state_int(S2650_R1, 0x08);
		m_maincpu->set_state_int(S2650_R2, 0xe0);
		m_maincpu->set_state_int(S2650_R3, 0x83);
		m_maincpu->set_state_int(S2650_PC, exec_addr);

		result = image_init_result::PASS;
	}

	return result;
}

void phunsy_state::init_phunsy()
{
	uint8_t *main = memregion("maincpu")->base();
	uint8_t *roms = memregion("roms")->base();
	uint8_t *ram = memregion("ram_4000")->base();

	membank("bankru")->configure_entry(0, &main[0x1800]);
	membank("bankwu")->configure_entry(0, &main[0x1800]);
	membank("bankru")->configure_entries(1, 3, &roms[0x800], 0x800);
	membank("bankq")->configure_entries(0, 16, &ram[0], 0x4000);

	membank("bankru")->set_entry(0); // point at ram
	membank("bankwu")->set_entry(0); // always write to ram
	membank("bankq")->set_entry(0);
}

void phunsy_state::phunsy(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(1'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &phunsy_state::phunsy_mem);
	m_maincpu->set_addrmap(AS_IO, &phunsy_state::phunsy_io);
	m_maincpu->set_addrmap(AS_DATA, &phunsy_state::phunsy_data);
	m_maincpu->sense_handler().set(FUNC(phunsy_state::cass_r));
	m_maincpu->flag_handler().set(FUNC(phunsy_state::cass_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	/* Display (page 12 of pdf)
	   - 8Mhz clock
	   - 64 6 pixel characters on a line.
	   - 16us not active, 48us active: ( 64 * 6 ) * 60 / 48 => 480 pixels wide
	   - 313 line display of which 256 are displayed.
	*/
	screen.set_raw(XTAL(8'000'000), 480, 0, 64*6, 313, 0, 256);
	screen.set_screen_update(FUNC(phunsy_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_phunsy);
	PALETTE(config, "palette", FUNC(phunsy_state::phunsy_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(phunsy_state::kbd_put));

	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* quickload */
	QUICKLOAD(config, "quickload", "bin", attotime::from_seconds(2)).set_load_callback(FUNC(phunsy_state::quickload_cb));
}


/* ROM definition */
ROM_START( phunsy )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x2000, "roms", 0 )
	ROM_LOAD( "phunsy_bios.bin", 0x0000, 0x0800, CRC(a789e82e) SHA1(b1c130ab2b3c139fd16ddc5dc7bdcaf7a9957d02))
	ROM_LOAD( "pdcr.bin",        0x0800, 0x0800, CRC(74bf9d0a) SHA1(8d2f673615215947f033571f1221c6aa99c537e9))
	ROM_LOAD( "dass.bin",        0x1000, 0x0800, CRC(13380140) SHA1(a999201cb414abbf1e10a7fcc1789e3e000a5ef1))
	ROM_LOAD( "labhnd.bin",      0x1800, 0x0800, CRC(1d5a106b) SHA1(a20d09e32e21cf14db8254cbdd1d691556b473f0))

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD( "ph_char1.bin", 0x0200, 0x0100, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee))
	ROM_CONTINUE(0x0100, 0x0100)
	ROM_LOAD( "ph_char2.bin", 0x0000, 0x0100, CRC(3d5786d3) SHA1(8cf87d83be0b5e4becfa9fd6e05b01250a2feb3b))
	ROM_CONTINUE(0x0300, 0x0100)

	/* 16 x 16KB RAM banks */
	ROM_REGION( 0x40000, "ram_4000", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY            FULLNAME  FLAGS */
COMP( 1980, phunsy, 0,      0,      phunsy,  phunsy, phunsy_state, init_phunsy, "J.F.P. Philipse", "PHUNSY", MACHINE_NOT_WORKING )
