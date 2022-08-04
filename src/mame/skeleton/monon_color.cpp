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
#include "cpu/axc51/axc51.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

class monon_color_state : public driver_device
{
public:
	monon_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spiptr(*this, "flash")
	{ }

	void monon_color(machine_config &config);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	required_device<generic_slot_device> m_cart;
	required_device<ax208_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_spiptr;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void out3_w(u8 data);
	void out4_w(u8 data);

	uint32_t m_spiaddr = 0;
	uint8_t m_spi_state = 0;
	uint8_t m_spilatch = 0;
	bool m_spidir = false;

	uint8_t spibuf_r();
	void spibuf_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(spidir_w);

	uint8_t m_vidbuffer[256 * 256];
	int bufpos = 0;
};

void monon_color_state::machine_start()
{
}

uint8_t monon_color_state::spibuf_r()
{
	// HACK while we figure things out

	logerror("%s: sfr_read AXC51_SPIBUF %02x\n", machine().describe_context(), m_spilatch);


	return m_spilatch;
	//return m_sfr_regs[AXC51_SPIBUF - 0x80];
}

WRITE_LINE_MEMBER(monon_color_state::spidir_w)
{
	m_spidir = state;
}

void monon_color_state::spibuf_w(uint8_t data)
{
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

		READY_FOR_READ = 0x08,
	};

	if (!m_spidir) // Send to SPI
	{
		logerror("%s: sfr_write AXC51_SPIBUF %02x\n", machine().describe_context(), data);

		switch (m_spi_state)
		{
		case READY_FOR_COMMAND:
		case READY_FOR_READ:
			if (data == 0x03)
			{
				// set read mode
				logerror("SPI Read Command\n");
				m_spi_state = READY_FOR_ADDRESS2;
			}
			else if (data == 0x05)
			{
				logerror("SPI Status Command\n");
			}
			else if (data == 0x0b)
			{
				logerror("SPI Fast Read Command\n");
				m_spi_state = READY_FOR_HSADDRESS2;
			}
			else
			{
				logerror("SPI unknown Command\n");
			}

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
			logerror("SPI Address set to %08x\n", m_spiaddr);
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
			logerror("SPI Address set to %08x\n", m_spiaddr);
			break;

		case READY_FOR_HSDUMMY:
			m_spi_state = READY_FOR_READ;
			break;
		}
	}
	else
	{
		if (m_spi_state == READY_FOR_READ)
		{
			m_spilatch = m_spiptr[m_spiaddr++];
			logerror("%s: sfr_write AXC51_SPIBUF (clock read, latching in %02x)\n", machine().describe_context(), m_spilatch);			
		}
		else
		{
			logerror("%s: sfr_write AXC51_SPIBUF (clock read)\n", machine().describe_context(), m_spi_state);			
			m_spi_state = 0x00;
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
}

void monon_color_state::video_start()
{
}


uint32_t monon_color_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t* videoram = m_vidbuffer;

	for (int y = 0; y < 256; y++)
	{
		int count = (y * 256);
		for(int x = 0; x < 256; x++)
		{
			uint8_t pixel = videoram[count++];
			bitmap.pix(y, x) = pixel;
		}
	}
	return 0;
}

static INPUT_PORTS_START( monon_color )
INPUT_PORTS_END


void monon_color_state::out4_w(u8 data)
{
//	m_vidbuffer[bufpos++] = data;
//	if (bufpos == (256 * 256))
//		bufpos = 0;
}

void monon_color_state::out3_w(u8 data)
{
	m_vidbuffer[bufpos++] = data;
	if (bufpos == (256 * 256))
		bufpos = 0;
}

void monon_color_state::monon_color(machine_config &config)
{
	/* basic machine hardware */
	AX208(config, m_maincpu, 96000000); // (8051 / MCS51 derived) incomplete core!
	m_maincpu->port_out_cb<3>().set(FUNC(monon_color_state::out3_w));
	m_maincpu->port_out_cb<4>().set(FUNC(monon_color_state::out4_w));
	m_maincpu->spi_in_cb().set(FUNC(monon_color_state::spibuf_r));
	m_maincpu->spi_out_cb().set(FUNC(monon_color_state::spibuf_w));
	m_maincpu->spi_out_dir_cb().set(FUNC(monon_color_state::spidir_w));

	

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(monon_color_state::screen_update));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

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
