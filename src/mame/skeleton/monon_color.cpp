// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************
   Monon Color - Chinese handheld

   see
   https://gbatemp.net/threads/the-monon-color-a-new-video-game-console-from-china.467788/
   https://twitter.com/Splatoon2weird/status/1072182093206052864

   uses AX208 CPU (custom 8051 @ 96Mhz with single cycle instructions, extended '16 bit' opcodes + integrated video, jpeg decoder etc.)
   https://docplayer.net/52724058-Ax208-product-specification.html

***************************************************************************/

#include "emu.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/axc51/axc51.h"
#include "sound/dac.h"

#define LOG_VDP (1U <<  1)
#define LOG_SPI (1U <<  2)

//#define VERBOSE     (LOG_VDP)
#define VERBOSE     (0)

#include "logmacro.h"

class monon_color_state : public driver_device
{
public:
	monon_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_spiptr(*this, "flash"),
		m_debugin(*this, "DEBUG%u", 0U)
	{ }

	void monon_color(machine_config &config);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
private:
	enum
	{
		READY_FOR_COMMAND = 0x00,
		READY_FOR_ADDRESS2 = 0x01,
		READY_FOR_ADDRESS1 = 0x02,
		READY_FOR_ADDRESS0 = 0x03,

		READY_FOR_HSADDRESS2 = 0x04,
		READY_FOR_HSADDRESS1 = 0x05,
		READY_FOR_HSADDRESS0 = 0x06,
		READY_FOR_HSDUMMY = 0x07,

		READY_FOR_WRITEADDRESS2 = 0x08,
		READY_FOR_WRITEADDRESS1 = 0x09,
		READY_FOR_WRITEADDRESS0 = 0x0a,
		READY_FOR_WRITE = 0x0b,

		READY_FOR_READ = 0x0c,
		READY_FOR_STATUS_READ = 0x0d,
	};

	required_device<generic_slot_device> m_cart;
	required_device<ax208_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<dac_8bit_r2r_twos_complement_device> m_dac;
	required_region_ptr<uint8_t> m_spiptr;
	required_ioport_array<4> m_debugin;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

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

	uint8_t read_from_video_device();
	void write_to_video_device(uint8_t data);

	void dacout0_w(uint8_t data);
	void dacout1_w(uint8_t data);

	uint8_t spibuf_r();
	void spibuf_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(spidir_w);

	void do_draw_inner(int pal_to_use, int start, int step, int pixmask, int amount);
	void do_draw(int amount, int pal_to_use);
	void do_palette(int amount, int pal_to_use);

	rgb_t m_linebuf[240];
	rgb_t m_vidbuffer[320 * 240];
	int16_t m_bufpos_y;
	uint32_t m_bufpos_x;
	uint8_t m_storeregs[0x20];
	uint8_t m_dacbyte;

	uint8_t m_out2state;

	uint32_t m_spiaddr;
	uint8_t m_spi_state;
	uint8_t m_spilatch;
	bool m_spidir;

	uint8_t m_out4data;
	uint8_t m_out3data;
	uint8_t m_out1data;
	uint8_t m_out0data;

	uint8_t m_curpal[0x800 * 3];
};

void monon_color_state::machine_start()
{
	save_item(NAME(m_dacbyte));
	save_item(NAME(m_out2state));
	save_item(NAME(m_spiaddr));
	save_item(NAME(m_spi_state));
	save_item(NAME(m_spilatch));
	save_item(NAME(m_spidir));
	save_item(NAME(m_out4data));
	save_item(NAME(m_out3data));
	save_item(NAME(m_out1data));
	save_item(NAME(m_out0data));
	save_item(NAME(m_linebuf));
	save_item(NAME(m_vidbuffer));
	save_item(NAME(m_bufpos_x));
	save_item(NAME(m_bufpos_y));
	save_item(NAME(m_curpal));
	save_item(NAME(m_storeregs));
}

uint8_t monon_color_state::spibuf_r()
{
	uint8_t ret = m_spilatch;
	return ret;
}

WRITE_LINE_MEMBER(monon_color_state::spidir_w)
{
	m_spidir = state;
}

void monon_color_state::spibuf_w(uint8_t data)
{
	if (!m_spidir) // Send to SPI
	{
		switch (m_spi_state)
		{
		case READY_FOR_COMMAND:
			if (data == 0x03)
			{
				m_spi_state = READY_FOR_ADDRESS2;
			}
			else if (data == 0x05)
			{
				m_spi_state = READY_FOR_STATUS_READ;
			}
			else if (data == 0x0b)
			{
				m_spi_state = READY_FOR_HSADDRESS2;
			}
			else if (data == 0x06)
			{
				// write enable
				m_spi_state = READY_FOR_COMMAND;
			}
			else if (data == 0x04)
			{
				// write disable
				m_spi_state = READY_FOR_COMMAND;
			}
			else if (data == 0x02)
			{
				// page program
				m_spi_state = READY_FOR_WRITEADDRESS2;
			}
			else
			{
				fatalerror( "SPI set to unknown mode %02x\n", data);
			}
			break;

		case READY_FOR_WRITEADDRESS2:
			m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16);
			m_spi_state = READY_FOR_WRITEADDRESS1;
			break;

		case READY_FOR_WRITEADDRESS1:
			m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8);
			m_spi_state = READY_FOR_WRITEADDRESS0;
			break;

		case READY_FOR_WRITEADDRESS0:
			m_spiaddr = (m_spiaddr & 0xffff00) | (data);
			m_spi_state = READY_FOR_WRITE;
			LOGMASKED(LOG_SPI, "SPI set to page WRITE mode with address %08x\n", m_spiaddr);
			break;

		case READY_FOR_WRITE:
			LOGMASKED(LOG_SPI, "Write SPI data %02x\n", data);

			m_spiptr[(m_spiaddr++) & 0xffffff] = data;

			break;


		case READY_FOR_ADDRESS2:
			m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16);
			m_spi_state = READY_FOR_ADDRESS1;
			break;

		case READY_FOR_ADDRESS1:
			m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8);
			m_spi_state = READY_FOR_ADDRESS0;
			break;

		case READY_FOR_ADDRESS0:
			m_spiaddr = (m_spiaddr & 0xffff00) | (data);
			m_spi_state = READY_FOR_READ;
			m_spidir = 1;
			LOGMASKED(LOG_SPI, "SPI set to READ mode with address %08x\n", m_spiaddr);
			break;

		case READY_FOR_HSADDRESS2:
			m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16);
			m_spi_state = READY_FOR_HSADDRESS1;
			break;

		case READY_FOR_HSADDRESS1:
			m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8);
			m_spi_state = READY_FOR_HSADDRESS0;
			break;

		case READY_FOR_HSADDRESS0:
			m_spiaddr = (m_spiaddr & 0xffff00) | (data);
			m_spi_state = READY_FOR_HSDUMMY;
			break;

		case READY_FOR_HSDUMMY:
			m_spi_state = READY_FOR_READ;
			m_spidir = 1;
			LOGMASKED(LOG_SPI, "SPI set to High Speed READ mode with address %08x\n", m_spiaddr);
			break;
		}
	}
	else
	{
		if (m_spi_state == READY_FOR_READ)
		{
			m_spilatch = m_spiptr[(m_spiaddr++)& 0xffffff];
		}
		else if (m_spi_state == READY_FOR_STATUS_READ)
		{
			m_spilatch = 0x00;
		}
		else
		{
			m_spilatch = 0x00;
		}
	}
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
	m_out2state = 0;
	m_spiaddr = 0;
	m_spi_state = 0;
	m_spilatch = 0;
	m_spidir = false;
	m_out4data = 0;
	m_out3data = 0;
	m_out1data = 0;
	m_out0data = 0;
	m_bufpos_x = 0;
	m_bufpos_y = 239;

	std::fill(std::begin(m_storeregs), std::end(m_storeregs), 0);
	std::fill(std::begin(m_linebuf), std::end(m_linebuf), 0);	
	std::fill(std::begin(m_vidbuffer), std::end(m_vidbuffer), 0);
}



uint32_t monon_color_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t* videoram = m_vidbuffer;

	for (int y = 0; y < 240; y++)
	{
		int count = (y * 320);
		for(int x = 0; x < 320; x++)
		{
			rgb_t pixel = videoram[count++];
			bitmap.pix(y, x) = pixel;
		}
	}

	return 0;
}

static INPUT_PORTS_START( monon_color )
	PORT_START("DEBUG0") // Port 0
	PORT_DIPNAME( 0x01, 0x01, "DEBUG0" )
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

	PORT_START("DEBUG1") // Port 1
	PORT_DIPNAME( 0x01, 0x01, "DEBUG1" )
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

	PORT_START("DEBUG2") // Port 2
	PORT_DIPNAME( 0x01, 0x01, "DEBUG2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) // not directly a button, inputs are read across multiple ports and multiplexed, see fixed bank code in purcfs 5D10
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
	PORT_DIPNAME( 0x01, 0x01, "DEBUG3" )
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
INPUT_PORTS_END

uint8_t monon_color_state::in0_r()
{
	return m_debugin[0]->read();
}

uint8_t monon_color_state::in1_r()
{
	return m_debugin[1]->read();
}

uint8_t monon_color_state::in2_r()
{
	return m_debugin[2]->read();
}

uint8_t monon_color_state::in3_r()
{
	return m_out3data;
}

uint8_t monon_color_state::in4_r()
{
	return m_debugin[3]->read();
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

void monon_color_state::out2_w(uint8_t data)
{
	if ((data & 0x01) != (m_out2state & 0x01))
	{
		if (data & 0x01)
		{

			// nothing?
		}
		else
		{
			m_spi_state = READY_FOR_COMMAND;
		}
	}
	m_out2state = data;
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
			if ((real_ypos >= 0) && (real_ypos < 240))
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

		m_palette->set_pen_color(entry, rgb_t(m_curpal[(entry * 3) + 2], m_curpal[(entry * 3) + 1],	m_curpal[(entry * 3) + 0]));
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

		//	popmessage("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", m_storeregs[0x00], m_storeregs[0x01], m_storeregs[0x02], m_storeregs[0x03], m_storeregs[0x04], m_storeregs[0x05], m_storeregs[0x06], m_storeregs[0x07], m_storeregs[0x08], m_storeregs[0x09], m_storeregs[0x0a], m_storeregs[0x0b], m_storeregs[0x0c], m_storeregs[0x0d], m_storeregs[0x0e], m_storeregs[0x0f],
		//		m_storeregs[0x10], m_storeregs[0x11], m_storeregs[0x12], m_storeregs[0x13], m_storeregs[0x14], m_storeregs[0x15], m_storeregs[0x16], m_storeregs[0x17], m_storeregs[0x18], m_storeregs[0x19], m_storeregs[0x1a], m_storeregs[0x1b], m_storeregs[0x1c], m_storeregs[0x1d], m_storeregs[0x1e], m_storeregs[0x1f]);

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

			if (m_bufpos_x == 320)
			{
				LOGMASKED(LOG_VDP, "------------------------------------------------------------------------------------------------\n");
				m_bufpos_x = 0;
			}

			m_bufpos_y = 239;
		}
	}

	//  (m_storeregs[0x1c] == 0x11) backgrounds (maybe no trans pen?)
	//  (m_storeregs[0x1c] == 0x01) most elements
	//  (m_storeregs[0x1c] == 0x0a) blended elements

	if (m_out0data == 0x0c)
	{
		if ((m_storeregs[0x1c] == 0x0a) || (m_storeregs[0x1c] == 0x01) || (m_storeregs[0x1c] == 0x11))
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
					for (int i = 0; i < 240; i++)
						m_vidbuffer[(i * 320) + m_bufpos_x] = m_linebuf[i];
				}
			}
		}
		else if (m_storeregs[0x1c] == 0x00)
		{

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

void monon_color_state::monon_color(machine_config &config)
{
	/* basic machine hardware */
	AX208(config, m_maincpu, 96000000/2);
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

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(monon_color_state::screen_update));
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0*8, 320-1, 0*8, 240-1);

	PALETTE(config, m_palette).set_entries(0x800);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.500); // should this be in the AX208 device?

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "monon_color_cart", "bin"));
	cartslot.set_width(GENERIC_ROM8_WIDTH);
	cartslot.set_device_load(FUNC(monon_color_state::cart_load));
	cartslot.set_must_be_loaded(true);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("monon_color");
}

DEVICE_IMAGE_LOAD_MEMBER( monon_color_state::cart_load )
{
	uint32_t size = m_cart->common_get_size("rom");
	std::vector<uint8_t> temp;
	temp.resize(size);
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(&temp[0], size, "rom");

	memcpy(memregion("flash")->base(), &temp[0], size);

	return image_init_result::PASS;
}

ROM_START( mononcol )
	ROM_REGION( 0x1000000, "flash", ROMREGION_ERASE00 )
ROM_END

CONS( 2014, mononcol,    0,          0,  monon_color,  monon_color,    monon_color_state, empty_init,    "M&D",   "Monon Color", MACHINE_IS_SKELETON )
