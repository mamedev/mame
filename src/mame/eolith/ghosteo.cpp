// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/************************************************************************

Eolith Ghost Hardware driver

1. CPU
- 16/32bit RISC Microprocessor(ARM920T CPU core)
- Separate 16KB Instruction and 16KB Data Cache
- Operating Frequency : Up to 203MHz

2. MEMORY
- Main Memory : 32Mbytes(Maximum 128Mbytes) SDRAM
- NAND FLASH : 16M,32M, 64M, 128M, 256Mbytes

3. VIDEO
- RGB (5:6:5) format
- Supports 1,2,4 or 8 bpp(bit-per-pixel) palette color displays
- Supports 16 bpp non-palette true-color displays
- Supports multiple screen size
Typical actual screen size : 320*240, 640*480
Maximum virtual screen size is 4Mbytes
Maximum virtual screen size in 64K color mode : 2048*1024
- Rendering Performance : 480 Objects per sec.(256*256 sprite)

4. SOUND
Internal Audio Engine or External System Available
TDA1519(Philips)Stereo Power AMP
- Internal Audio Engine : 1-ch IIS-bus for audio interface
Serial, 8-/16-bit per channel data transfers
128 Bytes (64-Byte + 64-Byte) FIFO for Tx/Rx
- External System(Option) : General MIDI Chipset QDSP1000
MIDI 16th Channel(32 Poly) using as Effect
Effect EPROM : 512Kbyte or 1Mbyte
MIDI Background Music EPROM:512Kbyte

5. Board Feature
- JAMMA Standard ( Application of PACK TYPE ) : 1P,2P ? Joystick (Up, Down, Left, Right), 4 Push Buttons, Start Button
- 1P,2P ? Coin, Coin Counter
- On Board Service Button, On Board Test Button
- Photo Gun : 1P, 2P Photo Gun (or Light Pen) (Option)
- ETC : Security System Available
Multi I/O Port Available
Coin Counter Available
Gun Shooting Effect Available (Option)
Hopper, Ticket Counter, Prize System (Option)

6. Development
- On-chip ICEbreaker debug support with JTAG-based debugging solution
- MultiICE, OPENICE etc.
- Compiler : ADS, SDT


ToDo: verify QS1000 hook-up


*/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/gen_latch.h"
#include "machine/i2cmem.h"
//#include "machine/nandflash.h"
#include "machine/s3c2410.h"
#include "sound/qs1000.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define NAND_LOG 0

enum nand_mode_t
{
	NAND_M_INIT,        // initial state
	NAND_M_READ         // read page data
};

struct nand_t
{
	nand_mode_t mode{};
	int page_addr = 0;
	int byte_addr = 0;
	int addr_load_ptr = 0;
};


class ghosteo_state : public driver_device
{
public:
	ghosteo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_i2cmem(*this, "i2cmem")
		, m_s3c2410(*this, "s3c2410")
		, m_qs1000(*this, "qs1000")
		, m_soundlatch(*this, "soundlatch")
		, m_system_memory(*this, "systememory")
		, m_flash(*this, "flash")
		, m_qs1000_bank(*this, "qs1000_bank")
	{
	}

	void ghosteo(machine_config &config) ATTR_COLD;
	void touryuu(machine_config &config) ATTR_COLD;
	void bballoon(machine_config &config) ATTR_COLD;

	void init_touryuu() ATTR_COLD;
	void init_bballoon() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<i2cmem_device> m_i2cmem;
	required_device<s3c2410_device> m_s3c2410;
	required_device<qs1000_device> m_qs1000;
	required_device<generic_latch_8_device> m_soundlatch;
	required_shared_ptr<uint32_t> m_system_memory;
	required_region_ptr<uint8_t> m_flash;
	memory_bank_creator m_qs1000_bank;

	int m_security_count = 0;
	uint32_t m_bballoon_port[20]{};
	struct nand_t m_nand;
	uint32_t bballoon_speedup_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t touryuu_port_10000000_r();

	void qs1000_p1_w(uint8_t data);
	void qs1000_p2_w(uint8_t data);
	void qs1000_p3_w(uint8_t data);

	int m_rom_pagesize = 0;
	uint32_t s3c2410_gpio_port_r(offs_t offset);
	void s3c2410_gpio_port_w(offs_t offset, uint32_t data);
	uint32_t s3c2410_core_pin_r(offs_t offset);
	void s3c2410_nand_command_w(uint8_t data);
	void s3c2410_nand_address_w(uint8_t data);
	uint8_t s3c2410_nand_data_r();
	void s3c2410_nand_data_w(uint8_t data);
	void s3c2410_i2c_scl_w(int state);
	int s3c2410_i2c_sda_r();
	void s3c2410_i2c_sda_w(int state);
	void bballoon_map(address_map &map) ATTR_COLD;
	void touryuu_map(address_map &map) ATTR_COLD;
};




/*
Power management:
Normal, Idle, Slow & Power-off
4-ch 16 bit PWM ( Pulse Width Modulation), & 1-ch 16-bit timer for OS
RTC : 32.768 KHz, alarm interrupt
GPIO :117 (multiplexed I/O)
3-ch UARTs
4 ch DMA Controllers
8-ch 10-bit A/D (Max. 500KSPS), including TSP Controller
TFT LCD/STN LCD Controller (16bit, 640?480 maximum)
16-bit Watch-dog Timer
1-ch IIC-Bus Interface
IIS-Bus Interface
Screen Size: up to 640 x 480
2-ch SPI (Synchronous Serial I/O)
SD Host/MMC (Multi Media Card) Interface
USB Host/Device Interface
Debug & TEST
NAND Flash Controller (4KB internal buffer)
24-ch external interrupts Controller (Wake-up source 16-ch)
*/

void ghosteo_state::qs1000_p1_w(uint8_t data)
{
}

void ghosteo_state::qs1000_p2_w(uint8_t data)
{
}

void ghosteo_state::qs1000_p3_w(uint8_t data)
{
	// .... .xxx - Data ROM bank (64kB)
	// ...x .... - ?
	// ..x. .... - /IRQ clear

	m_qs1000_bank->set_entry(data & 0x07);

	if (!BIT(data, 5))
		m_soundlatch->acknowledge_w();
}


// GPIO

static const uint8_t security_data[] = { 0x01, 0xC4, 0xFF, 0x22, 0xFF, 0xFF, 0xFF, 0xFF };

uint32_t ghosteo_state::s3c2410_gpio_port_r(offs_t offset)
{
	uint32_t data = m_bballoon_port[offset];
	switch (offset)
	{
		case S3C2410_GPIO_PORT_F :
		{
			data = (data & ~0xFF) | security_data[m_security_count]; // bballoon security @ 0x3001BD68
		}
		break;
		case S3C2410_GPIO_PORT_G :
		{
			data = data ^ 0x20;
			m_bballoon_port[offset] = data;
		}
		break;
	}
	return data;
}

void ghosteo_state::s3c2410_gpio_port_w(offs_t offset, uint32_t data)
{
	uint32_t old_value = m_bballoon_port[offset];
	m_bballoon_port[offset] = data;
	switch (offset)
	{
		case S3C2410_GPIO_PORT_F :
		{
			switch (data)
			{
				case 0x04 : m_security_count = 0; break;
				case 0x44 : m_security_count = 2; break;
			}
		}
		break;
		case S3C2410_GPIO_PORT_G :
		{
			// 0 -> 1
			if (((data & 0x10) != 0) && ((old_value & 0x10) == 0))
			{
				#if NAND_LOG
				logerror( "security_count %d -> %d\n", m_security_count, m_security_count + 1);
				#endif
				m_security_count++;
				if (m_security_count > 7) m_security_count = 0;
			}
		}
		break;
	}
}

// CORE

/*

OM[1:0] = 00b : Enable NAND flash controller auto boot mode

NAND flash memory page size should be 512Bytes.

NCON : NAND flash memory address step selection
0 : 3 Step addressing
1 : 4 Step addressing

*/

uint32_t ghosteo_state::s3c2410_core_pin_r(offs_t offset)
{
	int data = 0;
	switch (offset)
	{
		case S3C2410_CORE_PIN_NCON : data = 1; break;
		case S3C2410_CORE_PIN_OM0  : data = 0; break;
		case S3C2410_CORE_PIN_OM1  : data = 0; break;
	}
	return data;
}

// NAND

void ghosteo_state::s3c2410_nand_command_w(uint8_t data)
{
	struct nand_t &nand = m_nand;
	#if NAND_LOG
	logerror( "s3c2410_nand_command_w %02X\n", data);
	#endif
	switch (data)
	{
		case 0xFF :
		{
			nand.mode = NAND_M_INIT;
				m_s3c2410->frnb_w(1);
		}
		break;
		case 0x00 :
		{
			nand.mode = NAND_M_READ;
			nand.page_addr = 0;
			nand.addr_load_ptr = 0;
		}
		break;
	}
}

void ghosteo_state::s3c2410_nand_address_w(uint8_t data)
{
	struct nand_t &nand = m_nand;
	#if NAND_LOG
	logerror( "s3c2410_nand_address_w %02X\n", data);
	#endif
	switch (nand.mode)
	{
		case NAND_M_INIT :
		{
			logerror( "nand: unexpected address port write\n");
		}
		break;
		case NAND_M_READ :
		{
			if (nand.addr_load_ptr == 0)
			{
				nand.byte_addr = data;
			}
			else
			{
				nand.page_addr = (nand.page_addr & ~(0xFF << ((nand.addr_load_ptr - 1) * 8))) | (data << ((nand.addr_load_ptr - 1) * 8));
			}
			nand.addr_load_ptr++;
			if ((nand.mode == NAND_M_READ) && (nand.addr_load_ptr == 4))
			{
				m_s3c2410->frnb_w(0);
				m_s3c2410->frnb_w(1);
			}
		}
		break;
	}
}

uint8_t ghosteo_state::s3c2410_nand_data_r()
{
	struct nand_t &nand = m_nand;
	uint8_t data = 0;
	switch (nand.mode)
	{
		case NAND_M_INIT :
		{
			logerror( "nand: unexpected address port read\n");
		}
		break;
		case NAND_M_READ :
		{
			if (nand.byte_addr < m_rom_pagesize)
			{
				data = *(m_flash + nand.page_addr * m_rom_pagesize + nand.byte_addr);
			}
			else
			{
				if ((nand.byte_addr >= 0x200) && (nand.byte_addr < 0x204))
				{
					uint8_t mecc[4];
					m_s3c2410->s3c2410_nand_calculate_mecc( m_flash + nand.page_addr * 0x200, 0x200, mecc);
					data = mecc[nand.byte_addr-0x200];
				}
				else
				{
					data = 0xFF;
				}
			}
			nand.byte_addr++;
			if (nand.byte_addr == 0x210)
			{
				nand.byte_addr = 0;
				nand.page_addr++;
				if (nand.page_addr == 0x10000) nand.page_addr = 0;
			}
		}
		break;
	}
	#if NAND_LOG
	logerror( "s3c2410_nand_data_r %02X\n", data);
	#endif
	return data;
}

void ghosteo_state::s3c2410_nand_data_w(uint8_t data)
{
	#if NAND_LOG
	logerror( "s3c2410_nand_data_w %02X\n", data);
	#endif
}

// I2C

void ghosteo_state::s3c2410_i2c_scl_w(int state)
{
//  logerror( "s3c2410_i2c_scl_w %d\n", state ? 1 : 0);
	m_i2cmem->write_scl(state);
}

int ghosteo_state::s3c2410_i2c_sda_r()
{
	int state;
	state = m_i2cmem->read_sda();
//  logerror( "s3c2410_i2c_sda_r %d\n", state ? 1 : 0);
	return state;
}

void ghosteo_state::s3c2410_i2c_sda_w(int state)
{
//  logerror( "s3c2410_i2c_sda_w %d\n", state ? 1 : 0);
	m_i2cmem->write_sda(state);
}

uint32_t ghosteo_state::touryuu_port_10000000_r()
{
	uint32_t port_g = m_bballoon_port[S3C2410_GPIO_PORT_G];
	uint32_t data = 0xFFFFFFFF;
	switch (port_g)
	{
		case 0x8 : data = ioport( "10000000-08")->read(); break;
		case 0x9 : data = ioport( "10000000-09")->read(); break;
		case 0xA : data = ioport( "10000000-0A")->read(); break;
		case 0xB : data = ioport( "10000000-0B")->read(); break;
		case 0xC : data = ioport( "10000000-0C")->read(); break;
	}
//  logerror( "touryuu_port_10000000_r (%08X) -> %08X\n", port_g, data);
	return data;
}


void ghosteo_state::bballoon_map(address_map &map)
{
	map(0x10000000, 0x10000003).portr("10000000");
	map(0x10100000, 0x10100003).portr("10100000");
	map(0x10200000, 0x10200003).portr("10200000");
	map(0x10300000, 0x10300000).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask32(0x000000ff).cswidth(32);
	map(0x30000000, 0x31ffffff).ram().share("systememory").mirror(0x02000000);
}

void ghosteo_state::touryuu_map(address_map &map)
{
	map(0x10000000, 0x10000003).r(FUNC(ghosteo_state::touryuu_port_10000000_r));
	map(0x10100000, 0x10100003).portr("10100000");
	map(0x10200000, 0x10200003).portr("10200000");
	map(0x10300000, 0x10300000).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask32(0x000000ff).cswidth(32);
	map(0x30000000, 0x31ffffff).ram().share("systememory").mirror(0x02000000);
}


/*
static INPUT_PORTS_START( bballoon )
    PORT_START("10000000")
    PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
    PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
    PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
    PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
    PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
    PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
    PORT_BIT( 0xFFFFFFC0, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
    PORT_START("10100000")
    PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
    PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
    PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
    PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
    PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
    PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
    PORT_BIT( 0xFFFFFFC0, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
    PORT_START("10200000")
    PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
    PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
    PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
    PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_START3 )
    PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_SERVICE1 ) // "test button"
    PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_START4 )
    PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE2 ) // "service button"
    PORT_BIT( 0xFFFFFF00, IP_ACTIVE_LOW, IPT_START5 )
INPUT_PORTS_END
*/

static INPUT_PORTS_START( bballoon )
	PORT_START("10000000")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xFFFFFFC0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("10100000")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xFFFFFFC0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("10200000")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xFFFFFF50, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*

  Touryuumon

  +----+----+----+----+----+
  | 1  | 1  | 1  | FF | 1  |
  +----+----+----+----+----+
  | C  | G  | K  | Ti | Rn |
  +----+----+----+----+----+
  | D  | H  | L  | Po | Be |
  +----+----+----+----+----+
  | A  | E  | I  | M  | Kn |
  +----+----+----+----+----+
  | B  | F  | J  | N  | Re |
  +----+----+----+----+----+----+----+
  | Sv | Ts | Be | 2s | 1s | 2c | 1c |
  +----+----+----+----+----+----+----+

*/

static INPUT_PORTS_START( touryuu )
	PORT_START("10000000-08")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1 (5)")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1 (3)")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1 (2)")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1 (1)")
	PORT_BIT( 0xFFFFFFE0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("10000000-09")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) // labeled "Ti" in test mode
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0xFFFFFFE0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("10000000-0A")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0xFFFFFFE0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("10000000-0B")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0xFFFFFFE0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("10000000-0C")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0xFFFFFFE0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("10100000")
	PORT_BIT( 0xFFFFFFFF, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("10200000")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xFFFFFF50, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

uint32_t ghosteo_state::bballoon_speedup_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t ret = m_s3c2410->s3c24xx_lcd_r(offset+0x10/4, mem_mask);


	int pc = m_maincpu->pc();

	// these are vblank waits
	if (pc == 0x3001c0e4 || pc == 0x3001c0d8)
	{
		// BnB Arcade
		m_maincpu->spin_until_time(attotime::from_usec(20));
	}
	else if (pc == 0x3002b580 || pc == 0x3002b550)
	{
		// Happy Tour
		m_maincpu->spin_until_time(attotime::from_usec(20));
	}
	//else
	//  printf("speedup %08x %08x\n", pc, ret);

	return ret;
}

void ghosteo_state::machine_start()
{
	// Set up the QS1000 program ROM banking, taking care not to overlap the internal RAM
	m_qs1000->cpu().space(AS_IO).install_read_bank(0x0100, 0xffff, m_qs1000_bank);
	m_qs1000_bank->configure_entries(0, 8, memregion("qs1000:cpu")->base()+0x100, 0x10000);

	m_security_count = 0;
}

void ghosteo_state::machine_reset()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4d000010, 0x4d000013,read32s_delegate(*this, FUNC(ghosteo_state::bballoon_speedup_r)));
}

void ghosteo_state::ghosteo(machine_config &config)
{
	/* basic machine hardware */
	ARM9(config, m_maincpu, 200000000);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(455, 262);
	screen.set_visarea(0, 320-1, 0, 256-1);
	screen.set_screen_update("s3c2410", FUNC(s3c2410_device::screen_update));

	PALETTE(config, "palette").set_entries(256);

	S3C2410(config, m_s3c2410, 12000000);
	m_s3c2410->set_palette_tag("palette");
	m_s3c2410->set_screen_tag("screen");
	m_s3c2410->core_pin_r_callback().set(FUNC(ghosteo_state::s3c2410_core_pin_r));
	m_s3c2410->gpio_port_r_callback().set(FUNC(ghosteo_state::s3c2410_gpio_port_r));
	m_s3c2410->gpio_port_w_callback().set(FUNC(ghosteo_state::s3c2410_gpio_port_w));
	m_s3c2410->i2c_scl_w_callback().set(FUNC(ghosteo_state::s3c2410_i2c_scl_w));
	m_s3c2410->i2c_sda_r_callback().set(FUNC(ghosteo_state::s3c2410_i2c_sda_r));
	m_s3c2410->i2c_sda_w_callback().set(FUNC(ghosteo_state::s3c2410_i2c_sda_w));
	m_s3c2410->nand_command_w_callback().set(FUNC(ghosteo_state::s3c2410_nand_command_w));
	m_s3c2410->nand_address_w_callback().set(FUNC(ghosteo_state::s3c2410_nand_address_w));
	m_s3c2410->nand_data_r_callback().set(FUNC(ghosteo_state::s3c2410_nand_data_r));
	m_s3c2410->nand_data_w_callback().set(FUNC(ghosteo_state::s3c2410_nand_data_w));

//  samsung_k9f5608u0d_device &nand(SAMSUNG_K9F5608U0D(config, "nand", 0));  // or another variant with ID 0xEC 0x75 ?
//  nand.rnb_wr_callback().set(m_s3c2410, FUNC(s3c2410_device::s3c24xx_pin_frnb_w));

	I2C_24C16(config, "i2cmem", 0); // M24CL16-S

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(m_qs1000, FUNC(qs1000_device::set_irq));
	m_soundlatch->set_separate_acknowledge(true);

	QS1000(config, m_qs1000, XTAL(24'000'000));
	m_qs1000->set_external_rom(true);
	m_qs1000->p1_in().set("soundlatch", FUNC(generic_latch_8_device::read));
	m_qs1000->p1_out().set(FUNC(ghosteo_state::qs1000_p1_w));
	m_qs1000->p2_out().set(FUNC(ghosteo_state::qs1000_p2_w));
	m_qs1000->p3_out().set(FUNC(ghosteo_state::qs1000_p3_w));
	m_qs1000->add_route(0, "lspeaker", 1.0);
	m_qs1000->add_route(1, "rspeaker", 1.0);
}

void ghosteo_state::bballoon(machine_config &config)
{
	ghosteo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ghosteo_state::bballoon_map);
}

void ghosteo_state::touryuu(machine_config &config)
{
	ghosteo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ghosteo_state::touryuu_map);
}


/*
Balloon & Balloon
Eolith, 2003

PCB Layout
----------

GHOST Ver1.1 2003.03.28
|---------------------------------------------|
|TDA1519A              B2.U20  LED      RESET |
|         QS1000                              |
|DA1311A            24MHz                  LED|
|                                             |
|                                             |
|                  |---------|                |
|   QS1001A.U17    |ARM 312  |                |
|J                 |SAMSUNG  |                |
|A       B1.U16    |S3C2410  |   IC42S16800   |
|M                 |         |                |
|M                 |---------|                |
|A                                            |
|                                IC42S16800   |
|                            12MHz            |
|                    M24CL16-S                |
|                                             |
|                                             |
|                             *       FLASH.U1|
|      SERVICE                                |
|DSW1  TEST                                   |
|---------------------------------------------|
Notes:
      *          - Unknown SOIC20 chip, surface scratched
      FLASH.U1   - Samsung K9F5608U0B 256MBit NAND FlashROM (TSOP48)
      U16        - 27C801 8MBit EPROM
      U20        - 29F040 EEPROM
      IC42S16800 - ISSI IC42S16800 4(2)M x 8(16) Bits x 4 Banks (128MBit) SDRAM

      QS1000     - QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
      QS1001A    - QDSP QS1001A 512k x8 MaskROM (SOP32)

      qs1001a.u17 was not dumped from this PCB, but is a standard sample rom found on many Eolith games
                  see eolith.cpp and vegaeo.cpp drivers
*/

// The NAND dumps are missing the ECC data.  We calculate it on the fly, because the games require it, but really it should be dumped hence the 'BAD DUMP' flags
ROM_START( bballoon )
	ROM_REGION( 0x2000000, "flash", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "flash.u1",     0x000000, 0x2000000, BAD_DUMP CRC(73285634) SHA1(4d0210c1bebdf3113a99978ffbcd77d6ee854168) ) // missing ECC data

	// banked every 0x10000 bytes ?
	ROM_REGION( 0x080000, "qs1000:cpu", 0 )
	ROM_LOAD( "b2.u20",       0x000000, 0x080000, CRC(0a12334c) SHA1(535b5b34f28435517218100d70147d87809f485a) )

	ROM_REGION( 0x1000000, "qs1000", 0 )
	ROM_LOAD( "b1.u16",       0x000000, 0x100000, CRC(c42c1c85) SHA1(e1f49d556ffd6bc27142a7784c3bb8e37999857d) ) /* QDSP samples (SFX) */
	ROM_LOAD( "qs1001a.u17",  0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) /* QDSP wavetable rom */
ROM_END

ROM_START( hapytour ) /* Same hardware: GHOST Ver1.1 2003.03.28 */
	ROM_REGION( 0x2000000, "flash", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "flash.u1",     0x000000, 0x2000000, BAD_DUMP CRC(49deb7f9) SHA1(708a27d7177cf6261a49ded975c2bbb6c2427742) ) // missing ECC data

	// banked every 0x10000 bytes ?
	ROM_REGION( 0x080000, "qs1000:cpu", 0 )
	ROM_LOAD( "ht.u20",       0x000000, 0x080000, CRC(c0581fce) SHA1(dafce679002534ffabed249a92e6b83301b8312b) )

	ROM_REGION( 0x1000000, "qs1000", 0 )
	ROM_LOAD( "ht.u16",       0x000000, 0x100000, CRC(6a590a3a) SHA1(c1140f70c919661162334db66c6aa0ad656bfc47) ) /* QDSP samples (SFX) */
	ROM_LOAD( "qs1001a.u17",  0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) /* QDSP wavetable rom */
ROM_END


ROM_START( touryuu )
	ROM_REGION( 0x4200000, "flash", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "u1.bin",     0x000000, 0x4200000, CRC(49b6856e) SHA1(639123d2fabac4e79c9315fb87f72b13f9ae8761) )

	// banked every 0x10000 bytes ?
	ROM_REGION( 0x080000, "qs1000:cpu", 0 )
	ROM_LOAD( "4m.eeprom_c.s,bad1h.u20",       0x000000, 0x080000, CRC(f81a6530) SHA1(c7fa412102328d06823e73d7d07cadfc25db6d28) )

	ROM_REGION( 0x1000000, "qs1000", 0 )
	ROM_LOAD( "8m.eprom_c.s,f8b1h.u16",       0x000000, 0x100000, CRC(238a85ab) SHA1(ddd79429c0c1e67fcbca1e4ebded97ea46229f0b) ) /* QDSP samples (SFX) */
	ROM_LOAD( "qs1001a.u17",  0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) /* QDSP wavetable rom */
ROM_END

void ghosteo_state::init_bballoon()
{
	m_rom_pagesize = 0x200; // extra data is missing from the FLASH dumps and needs to be simulated
}

void ghosteo_state::init_touryuu()
{
	m_rom_pagesize = 0x210;
}

} // Anonymous namespace


GAME( 2003, bballoon, 0, bballoon, bballoon, ghosteo_state, init_bballoon, ROT0, "Eolith",          "BnB Arcade",         MACHINE_IMPERFECT_SOUND )
GAME( 2005, hapytour, 0, bballoon, bballoon, ghosteo_state, init_bballoon, ROT0, "GAV Company",     "Happy Tour",         MACHINE_IMPERFECT_SOUND )
GAME( 2005, touryuu,  0, touryuu,  touryuu,  ghosteo_state, init_touryuu,  ROT0, "Yuki Enterprise", "Touryuumon (V1.1)?", MACHINE_IMPERFECT_SOUND ) // On first boot inputs won't work, TODO: hook-up default eeprom
