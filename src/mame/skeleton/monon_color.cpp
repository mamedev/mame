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
#include "sound/dac.h"

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
		m_spiptr(*this, "flash")
	{ }

	void monon_color(machine_config &config);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;


	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	required_device<generic_slot_device> m_cart;
	required_device<ax208_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<dac_8bit_r2r_twos_complement_device> m_dac;
	required_region_ptr<uint8_t> m_spiptr;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	u8 in0_r();
	u8 in1_r();
	u8 in2_r();
	u8 in3_r();
	u8 in4_r();
	void out0_w(u8 data);
	void out1_w(u8 data);
	void out2_w(u8 data);
	void out3_w(u8 data);
	void out4_w(u8 data);

	uint8_t read_from_video_device();
	void write_to_video_device(u8 data);

	void dacout0_w(u8 data);
	void dacout1_w(u8 data);

	int m_dacbyte = 0;

	uint8_t m_out2state = 0;

	uint32_t m_spiaddr = 0;
	uint8_t m_spi_state = 0;
	uint8_t m_spilatch = 0;
	bool m_spidir = false;

	uint8_t m_out4data = 0;
	uint8_t m_out3data = 0;
	uint8_t m_out1data = 0;
	uint8_t m_out0data = 0;

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
		READY_FOR_STATUS_READ = 0x09,
	};

	uint8_t spibuf_r();
	void spibuf_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(spidir_w);

	uint8_t m_vidbuffer[360 * 240];
	int m_bufpos = 0;
	uint8_t m_storeregs0[0x20];
	uint8_t m_storeregs1[0x20];


};

void monon_color_state::machine_start()
{
	save_item(NAME(m_out2state));
	save_item(NAME(m_spiaddr));
	save_item(NAME(m_spi_state));
	save_item(NAME(m_spilatch));
	save_item(NAME(m_spidir));
	save_item(NAME(m_vidbuffer));
	save_item(NAME(m_bufpos));

	std::fill(std::begin(m_storeregs0), std::end(m_storeregs0), 0);
	std::fill(std::begin(m_storeregs1), std::end(m_storeregs1), 0);
	std::fill(std::begin(m_vidbuffer), std::end(m_vidbuffer), 0);
}

uint8_t monon_color_state::spibuf_r()
{
//	if (!(m_out2state & 0x01))
//		return 0x00;

	//logerror("%s: sfr_read AXC51_SPIBUF %02x\n", machine().describe_context(), m_spilatch);

	return m_spilatch;
}

WRITE_LINE_MEMBER(monon_color_state::spidir_w)
{
	m_spidir = state;
}

void monon_color_state::spibuf_w(uint8_t data)
{
//	if (!(m_out2state & 0x01))
//		return;


	if (!m_spidir) // Send to SPI
	{
		//logerror("%s: sfr_write AXC51_SPIBUF %02x\n", machine().describe_context(), data);

		switch (m_spi_state)
		{
		case READY_FOR_COMMAND:
			if (data == 0x03)
			{
				// set read mode
			//	logerror("SPI Read Command\n");
				m_spi_state = READY_FOR_ADDRESS2;
			}
			else if (data == 0x05)
			{
			//	logerror("SPI Status Command\n");
				m_spi_state = READY_FOR_STATUS_READ;
			}
			else if (data == 0x0b)
			{
			//	logerror("SPI Fast Read Command\n");
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
			logerror("SPI HS Address set to %08x\n", m_spiaddr);
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
		//	logerror("%s: sfr_write AXC51_SPIBUF (clock read, data read latching in %02x)\n", machine().describe_context(), m_spilatch);			
		}
		else if (m_spi_state == READY_FOR_STATUS_READ)
		{
			m_spilatch = 0x00;
		//	logerror("%s: sfr_write AXC51_SPIBUF (clock read, status read)\n", machine().describe_context(), m_spi_state);			
		}
		else
		{
			m_spilatch = 0x00;
		//	logerror("%s: sfr_write AXC51_SPIBUF (unknown read, status read)\n", machine().describe_context(), m_spi_state);			
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


	m_out2state = 0;
	m_spiaddr = 0;
	m_spi_state = 0;
	m_spilatch = 0;
	m_spidir = false;
	//m_vidbuffer[256 * 256];
	m_bufpos = 0;
}



uint32_t monon_color_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t* videoram = m_vidbuffer;

	for (int y = 0; y < 240; y++)
	{
		int count = (y * 360);
		for(int x = 0; x < 360; x++)
		{
			uint8_t pixel = videoram[count++];
			bitmap.pix(y, x) = pixel;
		}
	}
	return 0;
}

static INPUT_PORTS_START( monon_color )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x0001, 0x0001, "4" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END





u8 monon_color_state::in0_r()
{
	return machine().rand();// ioport("IN0")->read();
}

u8 monon_color_state::in1_r()
{
	return ioport("IN1")->read();
}

u8 monon_color_state::in2_r()
{
	return ioport("IN2")->read();
}

u8 monon_color_state::in3_r()
{
	return m_out3data;
}

u8 monon_color_state::in4_r()
{
	return ioport("IN4")->read();
}

void monon_color_state::dacout0_w(u8 data)
{
	//logerror("%s: dacout0_w %02x\n", machine().describe_context().c_str(), data);
	m_dacbyte ^= 1;

	if (m_dacbyte == 1)
		m_dac->write(data);
}

void monon_color_state::dacout1_w(u8 data)
{
	logerror("%s: dacout1_w %02x\n", machine().describe_context().c_str(), data);
}



void monon_color_state::out0_w(u8 data)
{
	m_out0data = data;
}

void monon_color_state::out1_w(u8 data)
{
	//logerror("out1 %02x\n", data);

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

void monon_color_state::out2_w(u8 data)
{
	if ((data & 0x01) != (m_out2state & 0x01))
	{
		if (data & 0x01)
		{
		//	logerror("low to high on port 2\n");
			m_spi_state = READY_FOR_COMMAND;
		}
		else
		{
		//	logerror("high to low on port 2\n");
		}
	}
	m_out2state = data;
}

uint8_t monon_color_state::read_from_video_device()
{
	if ((m_out0data != 0x00) && (m_out0data != 0x1b) && (m_out0data != 0x0a) && (m_out0data != 0x0c) && (m_out0data != 0x17) && (m_out0data != 0x1f) && (m_out0data != 0xff))
	{
		fatalerror("unknown out when using in");
	}

	return 0x00;
}

void monon_color_state::write_to_video_device(u8 data)
{

/*

startup only

out3 data 00 (out0 b0)
out3 data 00 (out0 b0)
out3 data dc (out0 10)
out3 data 32 (out0 18)
out3 data dc (out0 14)
out3 data c0 (out0 0b)
out3 data 00 (out0 07)
out3 data 00 (out0 0f)
out3 data 00 (out0 0d)
out3 data c0 (out0 0a)
out3 data 00 (out0 0f)
out3 data 00 (out0 07)
out3 data 00 (out0 0b)

*/
	// these are 2 pairs of read/write codepaths before accessing the video device
	// one of them sets port 4 to be 0x03, the other sets it to be 0x09
	// are there multiple devices?

	if (m_out4data == 0x03)
	{
		if (m_out0data < 0x20)
		{
			if (m_out0data >= 0x10) // when out4data is 0x03 registers(?) used are also always >= 0x10?
				m_storeregs0[m_out0data] = data;
			else
			{
				logerror("out4 mode is 0x03, m_out0data is <0x10  %02x\n", m_out0data);
			}
		}

		popmessage("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", m_storeregs0[0x00], m_storeregs0[0x01], m_storeregs0[0x02], m_storeregs0[0x03], m_storeregs0[0x04], m_storeregs0[0x05], m_storeregs0[0x06], m_storeregs0[0x07], m_storeregs0[0x08], m_storeregs0[0x09], m_storeregs0[0x0a], m_storeregs0[0x0b], m_storeregs0[0x0c], m_storeregs0[0x0d], m_storeregs0[0x0e], m_storeregs0[0x0f],
			m_storeregs0[0x10], m_storeregs0[0x11], m_storeregs0[0x12], m_storeregs0[0x13], m_storeregs0[0x14], m_storeregs0[0x15], m_storeregs0[0x16], m_storeregs0[0x17], m_storeregs0[0x18], m_storeregs0[0x19], m_storeregs0[0x1a], m_storeregs0[0x1b], m_storeregs0[0x1c], m_storeregs0[0x1d], m_storeregs0[0x1e], m_storeregs0[0x1f]);

		/*
		if (m_out0data == 0x17) // not much after startup
			return;

		if (m_out0data == 0x09)
			return;

		if (m_out0data == 0x0c) // more interesting
			return;

		if (m_out0data == 0x1b)
			return;

		if (m_out0data == 0x1c) // main port?
			return;

		if (m_out0data == 0x02)
			return;

		if (m_out0data == 0x12)
			return;

		if (m_out0data == 0x11)
			return;

		if (m_out0data == 0x1e)
			return;

		if (m_out0data == 0x01)
			return;

		if (m_out0data == 0x1a)
			return;

		if (m_out0data == 0x16)
			return;

		if (m_out0data == 0x0e)
			return;

		if (m_out0data == 0x1f)
			return;

		if (m_out0data == 0x00)
			return;

		if (m_out0data == 0x04)
			return;

		if (m_out0data == 0x08)
			return;
		*/

		{
			m_vidbuffer[m_bufpos++] = data;
			if (m_bufpos == (360 * 240))
				m_bufpos = 0;
		}
	}
	else if (m_out4data == 0x09)
	{
		if (m_out0data < 0x20)
		{
			if (m_out0data < 0x10) // when out4data is 0x09 registers(?) used are also always < 0x10?
				m_storeregs1[m_out0data] = data;
			else
			{
				logerror("out4 mode is 0x09, m_out0data is >=0x10  %02x\n", m_out0data);
			}
		}
	}
	else
	{
		fatalerror("write to port 3 with unknown port 4 state %02x\n", m_out4data);
	}
}



void monon_color_state::out3_w(u8 data)
{
	m_out3data = data;
}

void monon_color_state::out4_w(u8 data)
{
	m_out4data = data;
}


void monon_color_state::monon_color(machine_config &config)
{
	/* basic machine hardware */
	AX208(config, m_maincpu, 96000000); // (8051 / MCS51 derived) incomplete core!
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
	m_screen->set_size(360, 240);
	m_screen->set_visarea(0*8, 360-1, 0*8, 240-1);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.500); 

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
