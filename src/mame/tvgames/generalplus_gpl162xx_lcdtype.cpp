// license:BSD-3-Clause
// copyright-holders:David Haywood

/* These contain a similar game selection to the devices in unk6502_st2xxx.cpp but on updated hardware

   The hardware appears to be an abuse of the GPL16250 SoC. The palette and sprite banks are used, but as work-ram
   rather than for their intended purpose, and the rest of the GPL16250 video hardware is either entirely bypassed
   or doesn't exist.  All video is software rendered and output directly to the LCD Controller.

   If this is confirmed via a decap then this should be merged with the GPL16250 implementation.

   The coding of these is similar to the unk6502_st2xxx.cpp too, with all game specific function calls being loaded
   on the fly from the SPI to a tiny portion of work RAM, with graphics likewise being loaded and decompressed for
   every draw call.

   pcp8718 / pcp8728 / bkid218
   to access test mode hold up + left on startup.
   (bkid218 cursor in test menu can't be moved in emulation, works on unit, why?)
*/

// TODO: convert to use generic_spi_flash.cpp (but there seems to be some buffering of writes / reads?)

#include "emu.h"

#include "cpu/unsp/unsp.h"
#include "machine/bl_handhelds_menucontrol.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_GPL162XX_LCDTYPE            (1U << 1)
#define LOG_GPL162XX_LCDTYPE_SELECT_SIM (1U << 2)
#define LOG_GPL162XX_LCDTYPE_IO_PORT    (1U << 3)

#define LOG_ALL             (LOG_GPL162XX_LCDTYPE | LOG_GPL162XX_LCDTYPE_SELECT_SIM | LOG_GPL162XX_LCDTYPE_IO_PORT)
#define VERBOSE             (0)

#include "logmacro.h"


namespace {

class gpl162xx_lcdtype_state : public driver_device
{
public:
	gpl162xx_lcdtype_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainrom(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_spirom(*this, "spi"),
		m_io_in0(*this, "IN0"),
		m_io_in1(*this, "IN1"),
		m_menucontrol(*this, "menucontrol")
	{ }

	void gpl162xx_lcdtype(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<unsp_20_device> m_maincpu;
	required_region_ptr<uint16_t> m_mainrom;
	required_shared_ptr<uint16_t> m_mainram;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	void map(address_map &map) ATTR_COLD;

	required_region_ptr<uint8_t> m_spirom;

	uint16_t unk_7abf_r();
	uint16_t io_7860_r();
	uint16_t unk_780f_r();

	void io_7860_w(uint16_t data);
	uint16_t m_7860;

	void unk_7862_w(uint16_t data);
	void unk_7863_w(uint16_t data);

	void unk_7868_w(uint16_t data);
	uint16_t unk_7868_r();
	uint16_t m_7868;

	void bankswitch_707e_w(uint16_t data);
	uint16_t bankswitch_707e_r();
	uint16_t m_707e_bank;

	void bankswitch_703a_w(uint16_t data);
	uint16_t bankswitch_703a_r();
	uint16_t m_703a_bank;

	void bankedram_7300_w(offs_t offset, uint16_t data);
	uint16_t bankedram_7300_r(offs_t offset);
	uint16_t m_bankedram_7300[0x400];

	void bankedram_7400_w(offs_t offset, uint16_t data);
	uint16_t bankedram_7400_r(offs_t offset);
	uint16_t m_bankedram_7400[0x800];

	void system_dma_params_channel0_w(offs_t offset, uint16_t data);
	uint16_t system_dma_params_channel0_r(offs_t offset);

	uint16_t m_dmaregs[8];

	void lcd_w(uint16_t data);
	void lcd_command_w(uint16_t data);

	uint16_t spi_misc_control_r();
	uint16_t spi_rx_fifo_r();
	void spi_tx_fifo_w(uint16_t data);

	void spi_control_w(uint16_t data);

	void spi_process_tx_data(uint8_t data);
	uint8_t spi_process_rx();
	uint8_t spi_rx();
	uint8_t spi_rx_fast();

	uint8_t m_rx_fifo[5]; // actually 8 bytes? or 8 half-bytes?

	uint32_t m_spiaddress;

	uint16_t unk_78a1_r();
	uint16_t m_78a1;
	uint16_t unk_78d8_r();
	void unk_78d8_w(uint16_t data);

	enum spistate : const int
	{
	   SPI_STATE_READY = 0,
	   SPI_STATE_WAITING_HIGH_ADDR = 1,
	   SPI_STATE_WAITING_MID_ADDR = 2,
	   SPI_STATE_WAITING_LOW_ADDR = 3,
	   // probably not
	   SPI_STATE_WAITING_DUMMY1_ADDR = 4,
	   SPI_STATE_WAITING_DUMMY2_ADDR = 5,
	   SPI_STATE_READING = 6,

	   SPI_STATE_WAITING_HIGH_ADDR_FAST = 8,
	   SPI_STATE_WAITING_MID_ADDR_FAST = 9,
	   SPI_STATE_WAITING_LOW_ADDR_FAST = 10,
	   SPI_STATE_WAITING_LOW_ADDR_FAST_DUMMY = 11,
	   SPI_STATE_READING_FAST = 12

	};

	spistate m_spistate;

	enum lcdstate : const int
	{
		LCD_STATE_READY = 0,
		LCD_STATE_WAITING_FOR_COMMAND = 1,
		LCD_STATE_PROCESSING_COMMAND = 2
	};

	lcdstate m_lcdstate;
	int m_lastlcdcommand;

	uint8_t m_displaybuffer[0x40000];
	int m_lcdaddr;

	uint16_t io_7870_r();
	required_ioport m_io_in0;
	required_ioport m_io_in1;
	required_device<bl_handhelds_menucontrol_device> m_menucontrol;

	void screen_vblank(int state);
};

uint32_t gpl162xx_lcdtype_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count = 0;
	for (int y = 0; y < 256; y++)
	{
		uint32_t *const dst = &bitmap.pix(y);

		for (int x = 0; x < 320; x++)
		{
			// 8-bit values get pumped through a 256 word table in internal ROM and converted to words
			uint16_t dat = m_displaybuffer[(count * 2) + 1] | (m_displaybuffer[(count * 2) + 0] << 8);

			int b = ((dat >> 0) & 0x1f) << 3;
			int g = ((dat >> 5) & 0x3f) << 2;
			int r = ((dat >> 11) & 0x1f) << 3;

			dst[x] = (r << 16) | (g << 8) | (b << 0);
			count++;
		}
	}

	return 0;
}


static INPUT_PORTS_START( gpl162xx_lcdtype )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED ) // causes lag if state is inverted, investigate
	PORT_DIPNAME( 0x0002, 0x0002, "P0:0002" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0002, "0002" )
	PORT_DIPNAME( 0x0004, 0x0000, "Show Vs in Test Mode" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0004, "0004" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("menucontrol", bl_handhelds_menucontrol_device, status_r)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("menucontrol", bl_handhelds_menucontrol_device, data_r)
	PORT_DIPNAME( 0x0020, 0x0020, "P0:0020" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0020, "0020" )
	PORT_DIPNAME( 0x0040, 0x0040, "P0:0040" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0040, "0040" )
	PORT_DIPNAME( 0x0080, 0x0080, "P0:0080" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0080, "0080" )
	PORT_DIPNAME( 0x0100, 0x0100, "P0:0100" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0100, "0100" )
	PORT_DIPNAME( 0x0200, 0x0000, "Battery Level" )
	PORT_DIPSETTING(      0x0000, "Normal" )
	PORT_DIPSETTING(      0x0200, "Low" )
	PORT_DIPNAME( 0x0400, 0x0400, "P0:0400" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0400, "0400" )
	PORT_DIPNAME( 0x0800, 0x0800, "P0:0800" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0800, "0800" )
	PORT_DIPNAME( 0x1000, 0x1000, "P0:1000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x1000, "1000" )
	PORT_DIPNAME( 0x2000, 0x2000, "P0:2000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x2000, "2000" )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "P1:0001" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0001, "0001" )
	PORT_DIPNAME( 0x0002, 0x0002, "P1:0002" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0002, "0002" )
	PORT_DIPNAME( 0x0004, 0x0004, "P1:0004" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0004, "0004" )
	PORT_DIPNAME( 0x0008, 0x0008, "P1:0008" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0008, "0008" )
	PORT_DIPNAME( 0x0010, 0x0010, "P1:0010" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0010, "0010" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("SOUND")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("ON/OFF")
	PORT_DIPNAME( 0x1000, 0x1000, "P1:1000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x1000, "1000" )
	PORT_DIPNAME( 0x2000, 0x2000, "P1:2000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x2000, "2000" )
	PORT_DIPNAME( 0x4000, 0x4000, "P1:4000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x4000, "4000" )
	PORT_DIPNAME( 0x8000, 0x8000, "P1:8000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x8000, "8000" )
INPUT_PORTS_END

uint16_t gpl162xx_lcdtype_state::unk_7abf_r()
{
	return 0x0001;
}

uint16_t gpl162xx_lcdtype_state::io_7860_r()
{
	uint16_t ret = m_io_in0->read();
	LOGMASKED(LOG_GPL162XX_LCDTYPE_IO_PORT, "%s: io_7860_r %02x\n", machine().describe_context(), ret);
	return ret;
}

void gpl162xx_lcdtype_state::io_7860_w(uint16_t data)
{
	m_menucontrol->data_w((data & 0x10)>>4);
	m_menucontrol->clock_w((data & 0x20)>>5);

	m_7860 = data;
}

void gpl162xx_lcdtype_state::unk_7862_w(uint16_t data)
{
}

void gpl162xx_lcdtype_state::unk_7863_w(uint16_t data)
{
	// probably port direction (or 7862 is?)
	if (data == 0x3cf7)
	{
		m_menucontrol->reset_w(1);
	}
}

uint16_t gpl162xx_lcdtype_state::unk_780f_r()
{
	return 0x0002;
}

uint16_t gpl162xx_lcdtype_state::spi_misc_control_r()
{
	LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: spi_misc_control_r\n", machine().describe_context());
	return 0x0000;
}

uint16_t gpl162xx_lcdtype_state::spi_rx_fifo_r()
{
	if (m_spistate == SPI_STATE_READING_FAST)
		return spi_rx_fast();

	LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: spi_rx_fifo_r\n", machine().describe_context());
	return spi_rx();
}

void gpl162xx_lcdtype_state::spi_process_tx_data(uint8_t data)
{
	LOGMASKED(LOG_GPL162XX_LCDTYPE,"transmitting %02x\n", data);

	switch (m_spistate)
	{
	case SPI_STATE_READY:
	{
		if (data == 0x03)
		{
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"set to read mode (need address) %02x\n", data);
			m_spistate = SPI_STATE_WAITING_HIGH_ADDR;
		}
		else if (data == 0x0b)
		{
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"set to fast read mode (need address) %02x\n", data);
			m_spistate = SPI_STATE_WAITING_HIGH_ADDR_FAST;
			//machine().debug_break();
		}
		else
		{
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"invalid state request %02x\n", data);
		}
		break;
	}

	case SPI_STATE_WAITING_HIGH_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xff00ffff) | data << 16;
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"set to high address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_WAITING_MID_ADDR;
		break;
	}

	case SPI_STATE_WAITING_MID_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xffff00ff) | data << 8;
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"set to mid address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_WAITING_LOW_ADDR;
		break;
	}

	case SPI_STATE_WAITING_LOW_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xffffff00) | data;
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"set to low address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_READING;
		break;
	}

	case SPI_STATE_READING:
	{
		// writes when in read mode clock in data?
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"write while in read mode (clock data?)\n", data, m_spiaddress);
		break;
	}

	case SPI_STATE_WAITING_DUMMY1_ADDR:
	{
		m_spistate = SPI_STATE_WAITING_DUMMY2_ADDR;
		break;
	}

	case SPI_STATE_WAITING_DUMMY2_ADDR:
	{
	//  m_spistate = SPI_STATE_READY;
		break;
	}


	case SPI_STATE_WAITING_HIGH_ADDR_FAST:
	{
		m_spiaddress = (m_spiaddress & 0xff00ffff) | data << 16;
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"set to high address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_WAITING_MID_ADDR_FAST;
		break;
	}

	case SPI_STATE_WAITING_MID_ADDR_FAST:
	{
		m_spiaddress = (m_spiaddress & 0xffff00ff) | data << 8;
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"set to mid address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_WAITING_LOW_ADDR_FAST;
		break;
	}

	case SPI_STATE_WAITING_LOW_ADDR_FAST:
	{
		m_spiaddress = (m_spiaddress & 0xffffff00) | data;
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"set to low address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_WAITING_LOW_ADDR_FAST_DUMMY;

		break;
	}

	case SPI_STATE_WAITING_LOW_ADDR_FAST_DUMMY:
	{
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"dummy write %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_READING_FAST;
		break;
	}

	case SPI_STATE_READING_FAST:
	{
		// writes when in read mode clock in data?
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"write while in read mode (clock data?)\n", data, m_spiaddress);
		break;
	}

	}
}

uint8_t gpl162xx_lcdtype_state::spi_process_rx()
{

	switch (m_spistate)
	{
	case SPI_STATE_READING:
	case SPI_STATE_READING_FAST:
	{
		uint8_t dat = m_spirom[m_spiaddress & 0x7fffff];

		LOGMASKED(LOG_GPL162XX_LCDTYPE,"reading SPI %02x from SPI Address %08x (adjusted word offset %08x)\n", dat, m_spiaddress, (m_spiaddress/2)+0x20000);
		m_spiaddress++;
		return dat;
	}

	default:
	{
		LOGMASKED(LOG_GPL162XX_LCDTYPE,"reading FIFO in unknown state\n");
		return 0x00;
	}
	}

	return 0x00;
}


uint8_t gpl162xx_lcdtype_state::spi_rx()
{
	uint8_t ret = m_rx_fifo[0];

	m_rx_fifo[0] = m_rx_fifo[1];
	m_rx_fifo[1] = m_rx_fifo[2];
	m_rx_fifo[2] = m_rx_fifo[3];
	m_rx_fifo[3] = spi_process_rx();

	return ret;
}

uint8_t gpl162xx_lcdtype_state::spi_rx_fast()
{
	uint8_t ret = m_rx_fifo[0];

	m_rx_fifo[0] = m_rx_fifo[1];
	m_rx_fifo[1] = m_rx_fifo[2];
	m_rx_fifo[2] = m_rx_fifo[3];
	m_rx_fifo[3] = m_rx_fifo[4];
	m_rx_fifo[4] = spi_process_rx();

	return ret;
}

void gpl162xx_lcdtype_state::spi_tx_fifo_w(uint16_t data)
{
	data &= 0x00ff;
	LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: spi_tx_fifo_w %04x\n", machine().describe_context(), data);

	spi_process_tx_data(data);
}

// this is probably 'port b' but when SPI is enabled some points of this can become SPI control pins
// it's accessed after each large data transfer, probably to reset the SPI into 'ready for command' state?
void gpl162xx_lcdtype_state::unk_7868_w(uint16_t data)
{
	LOGMASKED(LOG_GPL162XX_LCDTYPE_IO_PORT, "%s: unk_7868_w %04x (Port B + SPI reset?)\n", machine().describe_context(), data);

	if ((m_7868 & 0x0100) != (data & 0x0100))
	{
		if (!(data & 0x0100))
		{
			for (int i = 0; i < 4; i++)
				m_rx_fifo[i] = 0xff;

			m_spistate = SPI_STATE_READY;
		}
	}

	m_7868 = data;
}

uint16_t gpl162xx_lcdtype_state::unk_7868_r()
{
	return m_7868;
}

void gpl162xx_lcdtype_state::bankswitch_707e_w(uint16_t data)
{
	LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: bankswitch_707e_w %04x\n", machine().describe_context(), data);
	m_707e_bank = data;
}

uint16_t gpl162xx_lcdtype_state::bankswitch_707e_r()
{
	return m_707e_bank;
}


void gpl162xx_lcdtype_state::bankswitch_703a_w(uint16_t data)
{
	LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: bankswitch_703a_w %04x\n", machine().describe_context(), data);
	m_703a_bank = data;
}

uint16_t gpl162xx_lcdtype_state::bankswitch_703a_r()
{
	return m_703a_bank;
}

void gpl162xx_lcdtype_state::bankedram_7300_w(offs_t offset, uint16_t data)
{
	offset |= (m_703a_bank & 0x000c) << 6;
	m_bankedram_7300[offset] = data;
}

uint16_t gpl162xx_lcdtype_state::bankedram_7300_r(offs_t offset)
{
	offset |= (m_703a_bank & 0x000c) << 6;
	return m_bankedram_7300[offset];
}

void gpl162xx_lcdtype_state::bankedram_7400_w(offs_t offset, uint16_t data)
{
	if (m_707e_bank & 1)
	{
		m_bankedram_7400[offset + 0x400] = data;
	}
	else
	{
		m_bankedram_7400[offset] = data;
	}
}

uint16_t gpl162xx_lcdtype_state::bankedram_7400_r(offs_t offset)
{
	if (m_707e_bank & 1)
	{
		return m_bankedram_7400[offset + 0x400];
	}
	else
	{
		return m_bankedram_7400[offset];
	}
}

void gpl162xx_lcdtype_state::system_dma_params_channel0_w(offs_t offset, uint16_t data)
{
	m_dmaregs[offset] = data;

	switch (offset)
	{
		case 0:
		{
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_w %01x %04x (DMA Mode)\n", machine().describe_context(), offset, data);

			uint16_t mode = m_dmaregs[0];
			uint32_t source = m_dmaregs[1] | (m_dmaregs[4] << 16);
			uint32_t dest = m_dmaregs[2] | (m_dmaregs[5] << 16) ;
			uint32_t length = m_dmaregs[3] | (m_dmaregs[6] << 16);

			if ((mode != 0x0200) && (mode != 0x4009) && (mode != 0x6009))
				fatalerror("unknown dma mode write %04x\n", data);

			if ((mode == 0x4009) || (mode == 0x6009))
			{
				address_space& mem = m_maincpu->space(AS_PROGRAM);

				for (int i = 0; i < length; i++)
				{
					uint16_t dat = mem.read_word(source);

					if (mode & 0x2000)
					{
						// Racing Car and Elevator Action need this logic
						mem.write_word(dest, dat & 0xff);
						dest++;
						mem.write_word(dest, dat >> 8);
						dest++;
					}
					else
					{
						mem.write_word(dest, dat);
						dest++;
					}

					source++;
				}
			}

			break;
		}
		case 1:
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_w %01x %04x (DMA Source Low)\n", machine().describe_context(), offset, data);
			break;

		case 2:
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_w %01x %04x (DMA Dest Low)\n", machine().describe_context(), offset, data);
			break;

		case 3:
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_w %01x %04x (DMA Length Low)\n", machine().describe_context(), offset, data);
			break;

		case 4:
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_w %01x %04x (DMA Source High)\n", machine().describe_context(), offset, data);
			break;

		case 5:
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_w %01x %04x (DMA Dest High)\n", machine().describe_context(), offset, data);
			break;

		case 6:
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_w %01x %04x (DMA Length High)\n", machine().describe_context(), offset, data);
			break;

		case 7:
			LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_w %01x %04x (DMA unknown)\n", machine().describe_context(), offset, data);
			break;
	}
}

uint16_t gpl162xx_lcdtype_state::system_dma_params_channel0_r(offs_t offset)
{
	LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: system_dma_params_channel0_r %01x\n", machine().describe_context(), offset);
	return m_dmaregs[offset];
}

uint16_t gpl162xx_lcdtype_state::io_7870_r()
{
	LOGMASKED(LOG_GPL162XX_LCDTYPE_IO_PORT, "%s: io_7870_r (IO port)\n", machine().describe_context() );
	return m_io_in1->read();
}

void gpl162xx_lcdtype_state::spi_control_w(uint16_t data)
{
	LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: spi_control_w %04x\n", machine().describe_context(), data);
}

uint16_t gpl162xx_lcdtype_state::unk_78a1_r()
{
	// checked in interrupt, code skipped entirely if this isn't set
	return m_78a1;
}

uint16_t gpl162xx_lcdtype_state::unk_78d8_r()
{
	return 0xffff;
}

void gpl162xx_lcdtype_state::unk_78d8_w(uint16_t data)
{
	// written in IRQ, possible ack
	if (data & 0x8000)
		m_78a1 &= ~0x8000;
}


void gpl162xx_lcdtype_state::map(address_map &map)
{
	// there are calls to 01xxx and 02xxx regions
	// (RAM populated by internal ROM?, TODO: check to make sure code copied there isn't from SPI ROM like the GPL16250 bootstrap
	//  does from NAND, it doesn't seem to have a header in the same format at least)
	map(0x000000, 0x006fff).ram().share("mainram");

	// registers at 7xxx are similar to GPL16250, but not identical? (different video system? or just GPL16250 with the video part unused?)

	map(0x00703a, 0x00703a).rw(FUNC(gpl162xx_lcdtype_state::bankswitch_703a_r), FUNC(gpl162xx_lcdtype_state::bankswitch_703a_w));
	map(0x00707e, 0x00707e).rw(FUNC(gpl162xx_lcdtype_state::bankswitch_707e_r), FUNC(gpl162xx_lcdtype_state::bankswitch_707e_w));

	map(0x007100, 0x0071ff).ram(); // rowscroll on gpl16250
	map(0x007300, 0x0073ff).rw(FUNC(gpl162xx_lcdtype_state::bankedram_7300_r), FUNC(gpl162xx_lcdtype_state::bankedram_7300_w)); // palette on gpl16250
	map(0x007400, 0x0077ff).rw(FUNC(gpl162xx_lcdtype_state::bankedram_7400_r), FUNC(gpl162xx_lcdtype_state::bankedram_7400_w)); // spriteram on gpl16250

	map(0x00780f, 0x00780f).r(FUNC(gpl162xx_lcdtype_state::unk_780f_r));


	map(0x007860, 0x007860).rw(FUNC(gpl162xx_lcdtype_state::io_7860_r),FUNC(gpl162xx_lcdtype_state::io_7860_w)); // Port A?
	map(0x007862, 0x007862).w(FUNC(gpl162xx_lcdtype_state::unk_7862_w));
	map(0x007863, 0x007863).w(FUNC(gpl162xx_lcdtype_state::unk_7863_w));

	map(0x007868, 0x007868).rw(FUNC(gpl162xx_lcdtype_state::unk_7868_r), FUNC(gpl162xx_lcdtype_state::unk_7868_w));  // Port B?

	map(0x007870, 0x007870).r(FUNC(gpl162xx_lcdtype_state::io_7870_r)); // Port C?

	map(0x0078a1, 0x0078a1).r(FUNC(gpl162xx_lcdtype_state::unk_78a1_r));

	map(0x0078d8, 0x0078d8).rw(FUNC(gpl162xx_lcdtype_state::unk_78d8_r), FUNC(gpl162xx_lcdtype_state::unk_78d8_w));


	map(0x007940, 0x007940).w(FUNC(gpl162xx_lcdtype_state::spi_control_w));
	// 7941 SPI Transmit Status
	map(0x007942, 0x007942).w(FUNC(gpl162xx_lcdtype_state::spi_tx_fifo_w));
	// 7943 SPI Receive Status
	map(0x007944, 0x007944).r(FUNC(gpl162xx_lcdtype_state::spi_rx_fifo_r));
	map(0x007945, 0x007945).r(FUNC(gpl162xx_lcdtype_state::spi_misc_control_r));

	map(0x007a80, 0x007a87).rw(FUNC(gpl162xx_lcdtype_state::system_dma_params_channel0_r), FUNC(gpl162xx_lcdtype_state::system_dma_params_channel0_w));

	map(0x007abf, 0x007abf).r(FUNC(gpl162xx_lcdtype_state::unk_7abf_r));

	// there are calls to 0x0f000 (internal ROM?)
	map(0x00f000, 0x00ffff).rom().region("maincpu", 0x00000);

	// external LCD controller
	map(0x200000, 0x200000).w(FUNC(gpl162xx_lcdtype_state::lcd_command_w));
	map(0x20fc00, 0x20fc00).w(FUNC(gpl162xx_lcdtype_state::lcd_w));
}

void gpl162xx_lcdtype_state::lcd_command_w(uint16_t data)
{
	data &= 0xff;

	switch (m_lcdstate)
	{
	case LCD_STATE_READY:
	case LCD_STATE_PROCESSING_COMMAND:
	{
		if (data == 0x0000)
		{
			m_lcdstate = LCD_STATE_WAITING_FOR_COMMAND;
			m_lastlcdcommand = 0;
		}
		break;
	}

	case LCD_STATE_WAITING_FOR_COMMAND:
	{
		m_lastlcdcommand = data;
		m_lcdstate = LCD_STATE_PROCESSING_COMMAND;
		break;
	}

	}
}

void gpl162xx_lcdtype_state::lcd_w(uint16_t data)
{
	data &= 0xff; // definitely looks like 8-bit port as 16-bit values are shifted and rewritten
	LOGMASKED(LOG_GPL162XX_LCDTYPE,"%s: lcd_w %02x\n", machine().describe_context(), data);

	if ((m_lcdstate == LCD_STATE_PROCESSING_COMMAND) && (m_lastlcdcommand == 0x22))
	{
		m_displaybuffer[m_lcdaddr] = data;
		m_lcdaddr++;

		if (m_lcdaddr >= ((320 * 240) * 2))
			m_lcdaddr = 0;

		//m_lcdaddr &= 0x3ffff;
	}
}


void gpl162xx_lcdtype_state::machine_start()
{
}


void gpl162xx_lcdtype_state::machine_reset()
{
	m_spistate = SPI_STATE_READY;
	m_spiaddress = 0;

	for (int i = 0; i < 320*240*2; i++)
		m_displaybuffer[i] = 0x00;

	m_lcdaddr = 0;

	m_lcdstate = LCD_STATE_READY;
	m_lastlcdcommand = 0;

	m_78a1 = 0;

// first menu index is stored here
//  m_spirom[0x16000] = gamenum & 0xff;
//  m_spirom[0x16001] = (gamenum>>8) & 0xff;
}

void gpl162xx_lcdtype_state::screen_vblank(int state)
{
	if (state)
	{
		// probably a timer
		m_maincpu->set_input_line(UNSP_IRQ4_LINE, ASSERT_LINE);
		m_78a1 |= 0x8000;
	}
	else
	{
		m_maincpu->set_input_line(UNSP_IRQ4_LINE, CLEAR_LINE);
	}
}

void gpl162xx_lcdtype_state::gpl162xx_lcdtype(machine_config &config)
{

	UNSP_20(config, m_maincpu, 96000000); // unknown CPU, unsp20 based, 96Mhz is listed as the maximum for most unSP2.0 chips, and appears correct here
	m_maincpu->set_addrmap(AS_PROGRAM, &gpl162xx_lcdtype_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 320-1, 0*8, 240-1);
	m_screen->set_screen_update(FUNC(gpl162xx_lcdtype_state::screen_update));
	m_screen->screen_vblank().set(FUNC(gpl162xx_lcdtype_state::screen_vblank));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x8000);

	BL_HANDHELDS_MENUCONTROL(config, m_menucontrol, 0);
	m_menucontrol->set_is_unsp_type_hack();

}

// pcp8718 and pcp8728 both contain user data (player name?) and will need to be factory defaulted once they work
// the ROM code is slightly different between them

ROM_START( pcp8718 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.rom", 0x000000, 0x2000, CRC(ea119561) SHA1(a2680577e20fe1155efc40a5781cf1ec80ccec3a) )

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASEFF )
	//ROM_LOAD16_WORD_SWAP( "8718_en25f32.bin", 0x000000, 0x400000, CRC(cc138db4) SHA1(379af3d94ae840f52c06416d6cf32e25923af5ae) ) // bad dump, some blocks are corrupt
	ROM_LOAD( "eyecare_25q32av1g_ef4016.bin", 0x000000, 0x400000, CRC(58415e10) SHA1(b1adcc03f2ad8d741544204671677740e904ce1a) )
ROM_END

ROM_START( pcp8728 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.rom", 0x000000, 0x2000, CRC(ea119561) SHA1(a2680577e20fe1155efc40a5781cf1ec80ccec3a) )

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD( "pcp 8728 788 in 1.bin", 0x000000, 0x400000, CRC(60115f21) SHA1(e15c39f11e442a76fae3823b6d510178f6166926) )
ROM_END

ROM_START( bkid218 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.rom", 0x000000, 0x2000, CRC(ea119561) SHA1(a2680577e20fe1155efc40a5781cf1ec80ccec3a) )

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD( "218n1_25q64csig_c84017.bin", 0x000000, 0x800000, CRC(94f35dbd) SHA1(a1bd6defd2465ae14753cd83be5c31f99e9158ec) )
ROM_END

} // anonymous namespace


CONS( 200?, pcp8718,      0,       0,      gpl162xx_lcdtype,   gpl162xx_lcdtype, gpl162xx_lcdtype_state, empty_init, "PCP", "PCP 8718 - HD 360 Degrees Rocker Palm Eyecare Console - 788 in 1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 200?, pcp8728,      0,       0,      gpl162xx_lcdtype,   gpl162xx_lcdtype, gpl162xx_lcdtype_state, empty_init, "PCP", "PCP 8728 - 788 in 1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // what name was this sold under?
CONS( 200?, bkid218,      0,       0,      gpl162xx_lcdtype,   gpl162xx_lcdtype, gpl162xx_lcdtype_state, empty_init, "BornKid", "Handheld Game Console BC-19 - 218 in 1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
