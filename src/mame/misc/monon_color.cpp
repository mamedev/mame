// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************
   Monon Color - Chinese handheld

   see
   https://gbatemp.net/threads/the-monon-color-a-new-video-game-console-from-china.467788/
   https://twitter.com/Splatoon2weird/status/1072182093206052864

   uses AX208 CPU (custom 8051 @ 96Mhz with single cycle instructions, extended '16 bit' opcodes + integrated video, jpeg decoder etc.)
   https://docplayer.net/52724058-Ax208-product-specification.html

   Emulation Notes:

   The Monon makes very little use of many of the AX208 features, for example

   - Encryption is only enabled once, for a throw-away transfer that is never
   checked.
   - JPEG RAM is simply used for extra RAM.
   - USB support not used
   - SD card support not used
   - Most IRQ levels not used
   - Onboard LCDC not used

   While the SFX used by each game are done through the AX208 DAC the music
   is not.

   Instead the music is supplied by a glob in each cartridge, which differs
   for each game.  This glob is assumed to be a self-contained MCU, responding
   to music requests.  Swapping the game ROM onto a different cartridge
   results in incorrect music, or silence, and in some cases hangs if you go
   into the volume adjustment menu.

   There is currently no way of reading the music data from this glob, nor
   has the exact die underneath it been identified.

   Games save data directly to the SPI ROM in the 0x50000-0x5ffff region

   Some games made use of either a card reader, or a badge reader on the
   cartridge, this is not yet emulated.  The card reader was based on
   barcode-like patterns, the badge reader looks to also be based on barcodes

   AXC51_TMR1PWML / AXC51_TMR1PWMH (Timer 1 Duty Registers) in the AX208
   core are responsible for screen brightness (can be adjusted in the pause
   menu)  There's no Timer 1 IRQ Handler in the code, but presumably the LCD
   is driven directly off the timer pins.

   Some games can be linked via the Infrared support provided by the AX208,
   this is not currently supported.

***************************************************************************/

#include "emu.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "bus/mononcol/slot.h"
#include "bus/mononcol/carts.h"

#include "cpu/axc51/axc51.h"
#include "cpu/m6502/m65ce02.h"
#include "sound/dac.h"

#define LOG_VDP (1U << 1)
#define LOG_MUSICMCUCOMMS (1U << 2)

//#define VERBOSE     (LOG_VDP)
#define VERBOSE     (0)

#include "logmacro.h"


namespace {

class monon_color_state : public driver_device
{
public:
	monon_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_maincpu(*this, "maincpu"),
		m_musicmcu(*this, "musicmcu"),
		m_musicrom(*this, "musicmcu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_debugin(*this, "DEBUG%u", 0U),
		m_controls(*this, "CONTROLS%u", 0U)
	{ }

	void monon_color(machine_config &config);
protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	static constexpr unsigned VIDEO_WIDTH = 320;
	static constexpr unsigned VIDEO_HEIGHT = 240;

	required_device<mononcol_cartslot_device> m_cart;
	required_device<ax208_cpu_device> m_maincpu;
	required_device<m65ce02_device> m_musicmcu;
	required_region_ptr<uint8_t> m_musicrom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<dac_8bit_r2r_twos_complement_device> m_dac;
	required_ioport_array<4> m_debugin;
	required_ioport_array<4> m_controls;

	rgb_t m_linebuf[VIDEO_HEIGHT];
	std::unique_ptr<rgb_t[]> m_vidbuffer;
	int16_t m_bufpos_y;
	uint32_t m_bufpos_x;
	uint8_t m_storeregs[0x20];
	uint8_t m_dacbyte;

	uint8_t m_out4data;
	uint8_t m_out3data;
	uint8_t m_out2data;
	uint8_t m_out1data;
	uint8_t m_out0data;

	uint8_t m_curpal[0x800 * 3];

	void music_mem(address_map &map) ATTR_COLD;
	uint8_t music_rts_r();

	uint8_t in0_r();
	uint8_t in1_r();
	uint8_t in2_r();
	uint8_t in3_r();
	uint8_t in4_r();
	void out0_w(uint8_t data);
	void out1_w(uint8_t data);
	void out2_w(uint8_t data);
	void out3_w(uint8_t data);
	void out4_w(uint8_t data);

	uint8_t read_current_inputs();

	uint8_t read_from_video_device();
	void write_to_video_device(uint8_t data);

	void dacout0_w(uint8_t data);
	void dacout1_w(uint8_t data);

	uint8_t spibuf_r()
	{
		return m_cart->read();
	}

	void spidir_w(int state)
	{
		m_cart->dir_w(state);
	}

	void spibuf_w(uint8_t data)
	{
		m_cart->write(data);
	}

	void get_music_command_bit(uint8_t bit);
	bool m_music_direction_iswrite;
	uint16_t m_music_latch;
	uint8_t m_music_bitpos;
	uint8_t m_music_response_bit;
	uint8_t m_music_bits_in_needed;

	void do_draw_inner(int pal_to_use, int start, int step, int pixmask, int amount);
	void do_draw(int amount, int pal_to_use);
	void do_palette(int amount, int pal_to_use);
};

void monon_color_state::machine_start()
{
	m_vidbuffer = std::make_unique<rgb_t[]>(VIDEO_WIDTH * VIDEO_HEIGHT);

	save_item(NAME(m_dacbyte));
	save_item(NAME(m_out4data));
	save_item(NAME(m_out3data));
	save_item(NAME(m_out2data));
	save_item(NAME(m_out1data));
	save_item(NAME(m_out0data));
	save_item(NAME(m_linebuf));
	save_pointer(NAME(m_vidbuffer), VIDEO_WIDTH * VIDEO_HEIGHT);
	save_item(NAME(m_bufpos_x));
	save_item(NAME(m_bufpos_y));
	save_item(NAME(m_curpal));
	save_item(NAME(m_storeregs));

	save_item(NAME(m_music_direction_iswrite));
	save_item(NAME(m_music_latch));
	save_item(NAME(m_music_bitpos));
	save_item(NAME(m_music_response_bit));
	save_item(NAME(m_music_bits_in_needed));
}


void monon_color_state::machine_reset()
{
	/*  block starting at e000 in flash is not code? (or encrypted?)
	    no code to map at 0x9000 in address space (possible BIOS?)
	    no code in flash ROM past the first 64kb(?) which is basically the same on all games, must be some kind of script interpreter? J2ME maybe?

	    there are 5 different 'versions' of the code in the dumped ROMs, where the code is the same the roms match up until 0x50000 after which the game specific data starts

	    by game number:

	    103alt                           (earliest? doesn't have bank9)
	    101,102,103,104,105              (1st revision)
	    106,107                          (2nd revision)
	    201                              (3rd revision)
	    202,203,204,205,301,302,303,304  (4th revision)
	*/

	m_dacbyte = 0;
	m_out2data = 0;

	m_out4data = 0;
	m_out3data = 0;
	m_out1data = 0;
	m_out0data = 0;
	m_bufpos_x = 0;
	m_bufpos_y = 239;

	std::fill(std::begin(m_linebuf), std::end(m_linebuf), 0);
	std::fill_n(m_vidbuffer.get(), VIDEO_WIDTH * VIDEO_HEIGHT, 0);
	std::fill(std::begin(m_storeregs), std::end(m_storeregs), 0);
	std::fill(std::begin(m_curpal), std::end(m_curpal), 0);

	m_music_direction_iswrite = true;
	m_music_latch = 0;
	m_music_bitpos = 0;
	m_music_response_bit = 0;
	m_music_bits_in_needed = 0;

	// don't need it running for now as the program is incomplete
	m_musicmcu->suspend(SUSPEND_REASON_DISABLE, 1);
}



uint32_t monon_color_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t const *videoram = m_vidbuffer.get();

	for (int y = 0; y < VIDEO_HEIGHT; y++)
	{
		int count = y * VIDEO_WIDTH;
		for(int x = 0; x < VIDEO_WIDTH; x++)
		{
			rgb_t pixel = videoram[count++];
			bitmap.pix(y, x) = pixel;
		}
	}

	return 0;
}

static INPUT_PORTS_START( monon_color )
	PORT_START("DEBUG0") // Port 0
	PORT_DIPNAME( 0x01, 0x01, "PORT0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // input related (read bits here)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // input related (read bits here)
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DEBUG1") // Port 1
	PORT_DIPNAME( 0x01, 0x01, "PORT1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // input related (mux select bits here)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // input related (mux select bits here)
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DEBUG2") // Port 2
	PORT_DIPNAME( 0x01, 0x01, "PORT2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // input related (read bits here)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DEBUG3") // Port 4
	PORT_DIPNAME( 0x01, 0x01, "PORT4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )  // input related (enables reading?)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("CONTROLS0")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("CONTROLS1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Right Trigger")

	PORT_START("CONTROLS2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Left Trigger")

	PORT_START("CONTROLS3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Menu")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Pause")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

/*
    muxselect = p1 & 0x18;

    0x30 input bits in p0
    0x04 input bits in p2

    0x04 in port 4 combine inputs?
*/

uint8_t monon_color_state::read_current_inputs()
{
	uint8_t iomux = (m_out1data & 0x18) >> 3;

	if ((m_out4data & 0x04) == 0x00)
	{
		if (iomux == 0x00)
		{
			uint8_t in = (m_controls[0]->read() | m_controls[1]->read() | m_controls[2]->read() | m_controls[3]->read()) ^ 0x7;
			return in;
		}
		else if (iomux == 0x01)
		{
			return m_controls[0]->read() ^ 0x7;
		}
		else if (iomux == 0x02)
		{
			return m_controls[1]->read() ^ 0x7;
		}
		else if (iomux == 0x03)
		{
			return m_controls[2]->read() ^ 0x7;
		}
	}
	else
	{
		return m_controls[3]->read() ^ 0x7;
	}

	return 0x00;
}

uint8_t monon_color_state::in0_r()
{
	uint8_t in = read_current_inputs();
	uint8_t ret = m_debugin[0]->read() & 0xcf;
	if (in & 0x01) ret |= 0x10;
	if (in & 0x02) ret |= 0x20;

	return ret;
}

uint8_t monon_color_state::in1_r()
{
	uint8_t ret = m_debugin[1]->read() & 0xe7;
	ret |= m_out1data & 0x18;

	return ret;
}

uint8_t monon_color_state::in2_r()
{
	uint8_t in = read_current_inputs();
	uint8_t ret = m_out2data & 0x7b;
	ret |= m_music_response_bit << 7;
	if (in & 0x04) ret |= 0x04;

	return ret;
}

uint8_t monon_color_state::in3_r()
{
	return m_out3data;
}

uint8_t monon_color_state::in4_r()
{
	uint8_t ret = m_debugin[3]->read() & 0xfb;
	ret |= m_out4data & 0x04;
	return ret;
}

void monon_color_state::dacout0_w(uint8_t data)
{
	m_dacbyte ^= 1;

	if (m_dacbyte == 1)
		m_dac->write(data);
}

void monon_color_state::dacout1_w(uint8_t data)
{
	// mono sound, not used?
}



void monon_color_state::out0_w(uint8_t data)
{
	m_out0data = data & 0x1f;
}

void monon_color_state::out1_w(uint8_t data)
{
	// out1 goes e5, e5, e7, e7 when WRITING a byte to the video device
	//        or e3, e3, e7, e7 when READING a byte from the video device
	//
	// presumably this actually causes the data from P3 to be written to the
	// device, or data from the device to be latched in P3

	if ((m_out1data == 0xe5) && (data == 0xe7))
	{
		write_to_video_device(m_out3data);
	}
	else if ((m_out1data == 0xe3) && (data == 0xe7))
	{
		m_out3data = read_from_video_device();
	}

	m_out1data = data;

}

void monon_color_state::get_music_command_bit(uint8_t bit)
{
	bit &= 1;

	if (m_music_bitpos == 0)
	{
		// first bit isn't part of the data? but isn't 0/1 depending on if it's a read/write either?
		//LOGMASKED(LOG_MUSICMCUCOMMS, "%s: started read/write command\n", machine().describe_context());
		m_music_bitpos++;
	}
	else
	{
		if (m_music_direction_iswrite)
		{
			m_music_latch = m_music_latch << 1;
			m_music_latch |= bit;

			m_music_bitpos++;

			if (m_music_bitpos == 17)
			{
				//LOGMASKED(LOG_MUSICMCUCOMMS, "%s: sent sound word %04x to MCU\n", machine().describe_context(), m_music_latch);
				m_music_bitpos = 0;

				switch (m_music_latch & 0xf000)
				{

				case 0x0000:
					LOGMASKED(LOG_MUSICMCUCOMMS, "0-00x set volume to %04x\n", m_music_latch & 0x0fff);
					m_music_latch = 0;
					break;

				case 0x2000:
					LOGMASKED(LOG_MUSICMCUCOMMS, "2-00x play music %04x\n", m_music_latch & 0x0fff);
					m_music_latch = 0;
					break;

				case 0x3000:
					LOGMASKED(LOG_MUSICMCUCOMMS, "3-000 stop all %04x\n", m_music_latch & 0x0fff);
					m_music_latch = 0;
					break;

				case 0x4000:
					LOGMASKED(LOG_MUSICMCUCOMMS, "4-000 reset? %04x\n", m_music_latch & 0x0fff);
					m_music_latch = 0;
					break;

				case 0xa000:
					// Spams this command a variable amount of this between scenes

					// There is a protection check involving the axxx commands
					// The game expects to read a byte from the music MCU and
					// compares it with a byte in the SPI ROM from e000 - efff
					//
					// This data in the SPI ROM is 65ce02 code that maps at
					// 0x2000, so the check is presumably checking a random byte
					// from part of the internal music MCU ROM against the table
					// stored in ROM.  Sadly the internal program is larger than
					// the 0x1000 bytes of it that have been duplicated in the
					// SPI ROM for this check.
					//
					// If it doesn't get this, it will loop until by chance it gets the same
					// value, which results in random delays (eg. when moving the volume
					// slider from 0 to any other value)
					//
					// mechcycla (the game with the earliest firmware) will jump to an infinite
					// loop if this is wrong, this doesn't happen with mechcycl, which uses a
					// later firmware.
					//
					// unfortunately no way of getting the MCU to read out anytihng
					// other than the single 0x1000 range it checks using the axxx commands
					// has been found.

					LOGMASKED(LOG_MUSICMCUCOMMS, "a-xxx status return wanted %04x\n", m_music_latch & 0x0fff);
					m_music_direction_iswrite = false;
					m_music_bits_in_needed = 9;
					m_music_latch = m_musicrom[m_music_latch & 0xfff];
					break;

				default:
					LOGMASKED(LOG_MUSICMCUCOMMS, "x-xxx unknown MCU write %04x\n", m_music_latch);
					m_music_latch = 0;
					break;

				}
			}

		}
		else
		{
			m_music_response_bit = (m_music_latch & 0x80)>>7;
			m_music_latch <<= 1;

			m_music_bitpos++;

			if (m_music_bitpos == m_music_bits_in_needed)
			{
				//LOGMASKED(LOG_MUSICMCUCOMMS, "%s: finished clocking in MCU response to read?\n", machine().describe_context());
				m_music_bitpos = 0;
				m_music_direction_iswrite = true;
			}
		}
	}
}

void monon_color_state::out2_w(uint8_t data)
{
	// on the cartridge PCB the connections to the music MCU glob are marked P21, P25 and P27
	// This is where the signals go on the AX208 (Port 2 bit 1, Port 2 bit 5, Port 2 bit 7)

	// m-C- --Ms

	// m = from MCU serial data (in)
	// C = clock MCU serial data (out)
	// M = to MCU serial data (out)
	// s = SPI Chip Enable / Reset?

	if ((data & 0x01) != (m_out2data & 0x01))
	{
		if (data & 0x01)
		{
			// nothing?
		}
		else
		{
			m_cart->set_ready();
		}
	}

	if ((data & 0x20) != (m_out2data & 0x20))
	{
		if (data & 0x20)
		{
			// .. nothing
		}
		else
		{
			// sends m_out2data & 0x02 to external device / latches a read bit into m_out2data & 0x80
			//logerror("%s: send / receive music MCU, bit written %d\n", machine().describe_context(), (data & 0x02) >> 1);
			get_music_command_bit((data & 0x02) >> 1);
		}
	}

	m_out2data = data;
}

uint8_t monon_color_state::read_from_video_device()
{
	LOGMASKED(LOG_VDP,"%s: read_from_video_device (m_out0data is %02x)\n", machine().describe_context(), m_out0data);

	if (m_out0data == 0x0c)
		return 0xff ^ 0x02;

	if (m_out0data == 0x1b)
		return 0x00;

	if (m_out0data == 0x17)
		return 0x00;

	return 0x00;
}

void monon_color_state::do_draw_inner(int pal_to_use, int start, int step, int pixmask, int amount)
{
	uint8_t blend;

	if (m_storeregs[0x1c] == 0x0a) // see drgnbma main character, doesn't set this, expects solid
		blend = (m_storeregs[0x09] & 0x3f) << 2;
	else
		blend = 0;

	int yadjust = (m_storeregs[0x16] << 8) | m_storeregs[0x1a];
	yadjust >>= 7;
	yadjust -= 8;

	for (int i = 0; i < amount; i++)
	{
		spibuf_w(0x00); // clock
		uint8_t pix = spibuf_r();

		for (int i = start; i >= 0; i -= step)
		{
			int real_ypos = m_bufpos_y;
			real_ypos -= yadjust;
			if ((real_ypos >= 0) && (real_ypos < VIDEO_HEIGHT))
			{
				uint8_t pixx = (pix >> i) & pixmask;
				uint8_t newr = m_curpal[((pixx + pal_to_use) * 3) + 2];
				uint8_t newg = m_curpal[((pixx + pal_to_use) * 3) + 1];
				uint8_t newb = m_curpal[((pixx + pal_to_use) * 3) + 0];
				rgb_t rgb = rgb_t(newr, newg, newb);

				if (rgb != rgb_t(0xdc, 0x32, 0xdc)) // magic transparency colour?!
				{

					if (blend != 0x00)
					{
						rgb_t behind = m_linebuf[real_ypos];
						behind.scale8(0xff - blend);
						rgb.scale8(blend);

						rgb += behind;
						m_linebuf[real_ypos] = rgb;
					}
					else
					{
						m_linebuf[real_ypos] = rgb;

					}
				}
			}
			m_bufpos_y--;
		}
	}
}
void monon_color_state::do_draw(int amount, int pal_to_use)
{
	if (m_storeregs[0x12] == 0x13)  // 8bpp mode
	{
		do_draw_inner(pal_to_use, 0, 1, 0xff, amount);
	}
	else if (m_storeregs[0x12] == 0x12) // 4bpp mode
	{
		do_draw_inner(pal_to_use, 4, 4, 0xf, amount);
	}
	else if (m_storeregs[0x12] == 0x11) // 2bpp mode
	{
		do_draw_inner(pal_to_use, 6, 2, 0x3, amount);
	}
	else if (m_storeregs[0x12] == 0x10)  // 1bpp mode
	{
		do_draw_inner(pal_to_use, 7, 1, 1, amount);
	}

	m_bufpos_y = 239;
}

void monon_color_state::do_palette(int amount, int pal_to_use)
{
	for (int i = 0; i < amount; i++)
	{
		spibuf_w(0x00); // clock
		uint8_t romdat = spibuf_r();

		int address = i + pal_to_use * 3;
		m_curpal[address] = romdat;
		int entry = address / 3;

		m_palette->set_pen_color(entry, rgb_t(m_curpal[(entry * 3) + 2], m_curpal[(entry * 3) + 1], m_curpal[(entry * 3) + 0]));
	}
}



void monon_color_state::write_to_video_device(uint8_t data)
{
	static const char* const names[] =
	{
		"(Direct Data)", // Direct Data port, after setting 0x8 / 0x04 ? writes basic 'maximum intensity' R/G/B palettes here on startup
		"(Transfer Size MSB)",
		"(Allow/Disallow SPI Access?)",
		"(unknown 0x03)",
		"(Direct VRAM Address Select MSB?)",
		"(unknown 0x05)",
		"(unknown 0x06)",
		"(unknown 0x07)",
		"(Direct VRAM Address Select LSB?)",
		"(Blend level, when enabled)",
		"(unknown 0x0a)",
		"(unknown 0x0b)",
		"(Trigger Transfer/Mode)",
		"(unknown 0x0d)",
		"(Transfer Size LSB)",
		"(unknown 0x0f)",
		"(Configure Magic Trans Pen B?)", // on startup
		"(Palette Select)",
		"(BPP Select)",
		"(unknown 0x13)",
		"(Configure Magic Trans Pen R?)", // on startup
		"(unknown 0x15)",
		"(Y Adjust MSB)",
		"(used 0x17)", // some kind of command / data? (writes a 320x240 set of 00 here at startup, followed by a 0x1f 'clock' write every time?)
		"(Configure Magic Trans Pen G?)", // on startup
		"(unknown 0x19)",
		"(Y Adjust LSB)",
		"(Column Advance+More?)",
		"(Draw Mode, alpha enable, trans pen disable?)",
		"(unknown 0x1d)",
		"(Extra Palette Select?)", // written when there is text on the screen some kind of extra base select (LSB?) in 1bpp mode? seems to be for palettes directly uploaded at startup
		"(Clock for 0x17?)", // clock for 0x17? always writes after reading / writing 0x1f?
	};

	LOGMASKED(LOG_VDP, "%s: write_to_video_device (m_out0data is %02x) %s (data is %02x)\n", machine().describe_context(), m_out0data, names[m_out0data], data);

	if (m_out4data == 0x03)
	{
		if (m_out0data < 0x20)
		{
			if (m_out0data >= 0x10) // when out4data is 0x03 registers(?) used are also always >= 0x10?
				m_storeregs[m_out0data] = data;
		}

		//  popmessage("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", m_storeregs[0x00], m_storeregs[0x01], m_storeregs[0x02], m_storeregs[0x03], m_storeregs[0x04], m_storeregs[0x05], m_storeregs[0x06], m_storeregs[0x07], m_storeregs[0x08], m_storeregs[0x09], m_storeregs[0x0a], m_storeregs[0x0b], m_storeregs[0x0c], m_storeregs[0x0d], m_storeregs[0x0e], m_storeregs[0x0f],
		//      m_storeregs[0x10], m_storeregs[0x11], m_storeregs[0x12], m_storeregs[0x13], m_storeregs[0x14], m_storeregs[0x15], m_storeregs[0x16], m_storeregs[0x17], m_storeregs[0x18], m_storeregs[0x19], m_storeregs[0x1a], m_storeregs[0x1b], m_storeregs[0x1c], m_storeregs[0x1d], m_storeregs[0x1e], m_storeregs[0x1f]);

	}
	else if (m_out4data == 0x09)
	{
		if (m_out0data < 0x20)
		{
			if (m_out0data < 0x10) // when out4data is 0x09 registers(?) used are also always < 0x10?
			{
				m_storeregs[m_out0data] = data;
			}
		}
	}

	// it uploads some colours at startup (and sometimes changes values using this during the game)
	// it is unclear how the address of 0x8e relates to palette entries 0x400+
	// these seem to be used for the 1bpp text see purcfs name entry, lolfight stats
	if (m_out0data == 0x00)
	{
		if (m_storeregs[0x04] == 0x8e)
		{
			int addressx = (m_storeregs[0x08]++);
			int address = addressx + 0x400 * 3;
			m_curpal[address] = data;
			int entry = address / 3;
			m_palette->set_pen_color(entry, rgb_t(m_curpal[(entry * 3) + 2], m_curpal[(entry * 3) + 1], m_curpal[(entry * 3) + 0]));
		}
	}

	if (m_out0data == 0x1b)
	{
		if (data == 0x81)
		{
			LOGMASKED(LOG_VDP, "Finished Column %d\n", m_bufpos_x);
			m_bufpos_x++;

			if (m_bufpos_x == VIDEO_WIDTH)
			{
				LOGMASKED(LOG_VDP, "------------------------------------------------------------------------------------------------\n");
				m_bufpos_x = 0;
			}

			m_bufpos_y = 239;
		}
	}

	//  (m_storeregs[0x1c] == 0x11) backgrounds (maybe no trans pen?)
	//  (m_storeregs[0x1c] == 0x01) most elements
	//  (m_storeregs[0x1c] == 0x00) some elements in purceb and zombhunt
	//  (m_storeregs[0x1c] == 0x0a) blended elements

	if (m_out0data == 0x0c)
	{
		if ((m_storeregs[0x1c] == 0x00) || (m_storeregs[0x1c] == 0x0a) || (m_storeregs[0x1c] == 0x01) || (m_storeregs[0x1c] == 0x11))
		{
			uint16_t amount = m_storeregs[0x0e] | (m_storeregs[0x01] << 8);

			//if (amount != 0x00)
			{
				int pal_to_use = (m_storeregs[0x11] << 8) | m_storeregs[0x1e];

				pal_to_use >>= 5;
				pal_to_use &= 0x7ff;


				// d4/d6 are odd-even columns
				// d0/d2 are odd-even columns, after frame data? (writing them seems conditional on something else?)
				// da/de are palettes? (maybe bit 0x04 is just ignored here, these aren't column specific)

				if ((data == 0xde) || (data == 0xda))
				{
					if (m_storeregs[0x02] == 0x20)
						do_palette(amount, pal_to_use);
				}
				else if ((data == 0xd2) || (data == 0xd6))
				{
					if (m_storeregs[0x02] == 0x20)
						do_draw(amount, pal_to_use);

				}
				else if ((data == 0xd0) || (data == 0xd4))
				{
					// assume there's some kind of line column buffer, the exact swap trigger is unknown
					for (int i = 0; i < VIDEO_HEIGHT; i++)
						m_vidbuffer[(i * VIDEO_WIDTH) + m_bufpos_x] = m_linebuf[i];
				}
			}
		}
		else
		{
			fatalerror("m_storeregs[0x1c] set to unknown value %02x while drawing\n", m_storeregs[0x1c]);
		}
	}
}

void monon_color_state::out3_w(uint8_t data)
{
	m_out3data = data;
}

void monon_color_state::out4_w(uint8_t data)
{
	m_out4data = data;
}


uint8_t monon_color_state::music_rts_r()
{
	return 0x60;
}

void monon_color_state::music_mem(address_map &map)
{
	map(0x0000, 0x01ff).ram();

	map(0x2000, 0x2fff).rom().region("musicmcu", 0x0000);
	map(0x3000, 0x3fff).r(FUNC(monon_color_state::music_rts_r)); // likely ROM here, lots of JSR calls

	map(0xa000, 0xafff).r(FUNC(monon_color_state::music_rts_r)); // likely ROM here, lots of JSR calls

	map(0xfff0, 0xffff).rom().region("musicmcu", 0x1ff0);
}


void monon_color_state::monon_color(machine_config &config)
{
	/* basic machine hardware */
	AX208(config, m_maincpu, 96000000/2); // divider is not correct, need to check if this is configured to run at full speed
	m_maincpu->port_in_cb<0>().set(FUNC(monon_color_state::in0_r));
	m_maincpu->port_in_cb<1>().set(FUNC(monon_color_state::in1_r));
	m_maincpu->port_in_cb<2>().set(FUNC(monon_color_state::in2_r));
	m_maincpu->port_in_cb<3>().set(FUNC(monon_color_state::in3_r));
	m_maincpu->port_in_cb<4>().set(FUNC(monon_color_state::in4_r));
	m_maincpu->port_out_cb<0>().set(FUNC(monon_color_state::out0_w));
	m_maincpu->port_out_cb<1>().set(FUNC(monon_color_state::out1_w));
	m_maincpu->port_out_cb<2>().set(FUNC(monon_color_state::out2_w));
	m_maincpu->port_out_cb<3>().set(FUNC(monon_color_state::out3_w));
	m_maincpu->port_out_cb<4>().set(FUNC(monon_color_state::out4_w));
	m_maincpu->spi_in_cb().set(FUNC(monon_color_state::spibuf_r));
	m_maincpu->spi_out_cb().set(FUNC(monon_color_state::spibuf_w));
	m_maincpu->spi_out_dir_cb().set(FUNC(monon_color_state::spidir_w));
	m_maincpu->dac_out_cb<0>().set(FUNC(monon_color_state::dacout0_w));
	m_maincpu->dac_out_cb<1>().set(FUNC(monon_color_state::dacout1_w));

	M65CE02(config, m_musicmcu, 4000000);
	m_musicmcu->set_addrmap(AS_PROGRAM, &monon_color_state::music_mem);

	/* video hardware */

	// is this a 3:4 (true vertical) 240x320 screen rotated on its side?
	// the scan direction for drawing is highly unusual
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(120);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_screen_update(FUNC(monon_color_state::screen_update));
	m_screen->set_size(VIDEO_WIDTH, VIDEO_HEIGHT);
	m_screen->set_visarea(0*8, VIDEO_WIDTH-1, 0*8, VIDEO_HEIGHT-1);

	PALETTE(config, m_palette).set_entries(0x800);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.500); // should this be in the AX208 device?

	mononcol_cartslot_device &cartslot(MONONCOL_CARTSLOT(config, "cartslot", mononcol_plain_slot));
	cartslot.set_must_be_loaded(true);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("monon_color");
}

ROM_START( mononcol )
	// TODO: each cartridge has a music MCU inside of it.  This data appears to be 0x1000 bytes of the
	//       program that is checked as 'security' and can be found in the SPI ROMs.  A way to dump
	//       the full ROM is needed, at which point the game specific music MCU ROMs should be
	//       moved into the software list.  This just allows us to study what exists of this sound
	//       program until then.
	ROM_REGION( 0x2000, "musicmcu", ROMREGION_ERASEFF )
	ROM_LOAD( "music_mcu.bin", 0x0000, 0x1000, BAD_DUMP CRC(54224c67) SHA1(4a4e1d6995e6c8a36e3979bc22b8e9a1ea8f954f) )
	// fake boot vector, as we don't have one, but the code above expects to boot at 0x2000
	ROM_FILL( 0x1ffc, 0x1, 0x00 )
	ROM_FILL( 0x1ffd, 0x1, 0x20 )
ROM_END

} // anonymous namespace


CONS( 2014, mononcol,    0,          0,  monon_color,  monon_color,    monon_color_state, empty_init,    "M&D",   "Monon Color", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
