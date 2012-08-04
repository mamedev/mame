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

6. Develoment
- On-chip ICEbreaker debug support with JTAG-based debugging solution
- MultiICE, OPENICE etc.
- Compiler : ADS, SDT


*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/s3c2410.h"
//#include "machine/smartmed.h"
#include "machine/i2cmem.h"

#define NAND_LOG 0

enum nand_mode_t
{
	NAND_M_INIT,		// initial state
	NAND_M_READ,		// read page data
};

struct nand_t
{
	nand_mode_t mode;
	int page_addr;
	int byte_addr;
	int addr_load_ptr;
};


class ghosteo_state : public driver_device
{
public:
	ghosteo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_system_memory(*this, "systememory"){ }

	required_shared_ptr<UINT32> m_system_memory;
	int m_security_count;
	UINT32 m_bballoon_port[20];
	struct nand_t m_nand;
	DECLARE_WRITE32_MEMBER(sound_w);
	DECLARE_READ32_MEMBER(bballoon_speedup_r);
	DECLARE_READ32_MEMBER(touryuu_port_10000000_r);

	int m_rom_pagesize;
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

// GPIO

static const UINT8 security_data[] = { 0x01, 0xC4, 0xFF, 0x22, 0xFF, 0xFF, 0xFF, 0xFF };

static UINT32 s3c2410_gpio_port_r( device_t *device, int port, UINT32 mask)
{
	ghosteo_state *state = device->machine().driver_data<ghosteo_state>();
	UINT32 data = state->m_bballoon_port[port];
	switch (port)
	{
		case S3C2410_GPIO_PORT_F :
		{
			data = (data & ~0xFF) | security_data[state->m_security_count]; // bballoon security @ 0x3001BD68
		}
		break;
		case S3C2410_GPIO_PORT_G :
		{
			data = data ^ 0x20;
			state->m_bballoon_port[port] = data;
		}
		break;
	}
	return data;
}

static void s3c2410_gpio_port_w( device_t *device, int port, UINT32 mask, UINT32 data)
{
	ghosteo_state *state = device->machine().driver_data<ghosteo_state>();
	UINT32 old_value = state->m_bballoon_port[port];
	state->m_bballoon_port[port] = data;
	switch (port)
	{
		case S3C2410_GPIO_PORT_F :
		{
			switch (data)
			{
				case 0x04 : state->m_security_count = 0; break;
				case 0x44 : state->m_security_count = 2; break;
			}
		}
		break;
		case S3C2410_GPIO_PORT_G :
		{
			// 0 -> 1
			if (((data & 0x10) != 0) && ((old_value & 0x10) == 0))
			{
				#if NAND_LOG
				logerror( "security_count %d -> %d\n", state->m_security_count, state->m_security_count + 1);
				#endif
				state->m_security_count++;
				if (state->m_security_count > 7) state->m_security_count = 0;
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

static int s3c2410_core_pin_r( device_t *device, int pin)
{
	int data = 0;
	switch (pin)
	{
		case S3C2410_CORE_PIN_NCON : data = 1; break;
		case S3C2410_CORE_PIN_OM0  : data = 0; break;
		case S3C2410_CORE_PIN_OM1  : data = 0; break;
	}
	return data;
}

// NAND

static WRITE8_DEVICE_HANDLER( s3c2410_nand_command_w )
{
	ghosteo_state *state = device->machine().driver_data<ghosteo_state>();
	struct nand_t &nand = state->m_nand;
//  device_t *nand = device->machine().device( "nand");
	#if NAND_LOG
	logerror( "s3c2410_nand_command_w %02X\n", data);
	#endif
	switch (data)
	{
		case 0xFF :
		{
			nand.mode = NAND_M_INIT;
			s3c2410_pin_frnb_w( device, 1);
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

static WRITE8_DEVICE_HANDLER( s3c2410_nand_address_w )
{
	ghosteo_state *state = device->machine().driver_data<ghosteo_state>();
	struct nand_t &nand = state->m_nand;
//  device_t *nand = device->machine().device( "nand");
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
				s3c2410_pin_frnb_w( device, 0);
				s3c2410_pin_frnb_w( device, 1);
			}
		}
		break;
	}
}

static READ8_DEVICE_HANDLER( s3c2410_nand_data_r )
{
	ghosteo_state *state = device->machine().driver_data<ghosteo_state>();
	struct nand_t &nand = state->m_nand;
//  device_t *nand = device->machine().device( "nand");
	UINT8 data = 0;
	switch (nand.mode)
	{
		case NAND_M_INIT :
		{
			logerror( "nand: unexpected address port read\n");
		}
		break;
		case NAND_M_READ :
		{
			UINT8 *flash = (UINT8 *)device->machine().root_device().memregion( "user1")->base();
			if (nand.byte_addr < state->m_rom_pagesize)
			{
				data = *(flash + nand.page_addr * state->m_rom_pagesize + nand.byte_addr);
			}
			else
			{
				if ((nand.byte_addr >= 0x200) && (nand.byte_addr < 0x204))
				{
					UINT8 mecc[4];
					s3c2410_nand_calculate_mecc( flash + nand.page_addr * 0x200, 0x200, mecc);
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

static WRITE8_DEVICE_HANDLER( s3c2410_nand_data_w )
{
//  device_t *nand = device->machine().device( "nand");
	#if NAND_LOG
	logerror( "s3c2410_nand_data_w %02X\n", data);
	#endif
}

// I2C

static WRITE_LINE_DEVICE_HANDLER( s3c2410_i2c_scl_w )
{
	device_t *i2cmem = device->machine().device( "i2cmem");
//  logerror( "s3c2410_i2c_scl_w %d\n", state ? 1 : 0);
	i2cmem_scl_write( i2cmem, state);
}

static READ_LINE_DEVICE_HANDLER( s3c2410_i2c_sda_r )
{
	device_t *i2cmem = device->machine().device( "i2cmem");
	int state;
	state = i2cmem_sda_read( i2cmem);
//  logerror( "s3c2410_i2c_sda_r %d\n", state ? 1 : 0);
	return state;
}

static WRITE_LINE_DEVICE_HANDLER( s3c2410_i2c_sda_w )
{
	device_t *i2cmem = device->machine().device( "i2cmem");
//  logerror( "s3c2410_i2c_sda_w %d\n", state ? 1 : 0);
	i2cmem_sda_write( i2cmem, state);
}

WRITE32_MEMBER(ghosteo_state::sound_w)
{
	if ((data >= 0x20) && (data <= 0x7F))
	{
		logerror( "sound_w: music %d\n", data - 0x20);
	}
	else if ((data >= 0x80) && (data <= 0xFF))
	{
		logerror( "sound_w: effect %d\n", data - 0x80);
	}
	else
	{
		logerror( "sound_w: unknown (%d)\n", data);
	}
}

READ32_MEMBER( ghosteo_state::touryuu_port_10000000_r )
{
	ghosteo_state *state = machine().driver_data<ghosteo_state>();
	UINT32 port_g = state->m_bballoon_port[S3C2410_GPIO_PORT_G];
	UINT32 data = 0xFFFFFFFF;
	switch (port_g)
	{
		case 0x8 : data = machine().root_device().ioport( "10000000-08")->read(); break;
		case 0x9 : data = machine().root_device().ioport( "10000000-09")->read(); break;
		case 0xA : data = machine().root_device().ioport( "10000000-0A")->read(); break;
		case 0xB : data = machine().root_device().ioport( "10000000-0B")->read(); break;
		case 0xC : data = machine().root_device().ioport( "10000000-0C")->read(); break;
	}
//  logerror( "touryuu_port_10000000_r (%08X) -> %08X\n", port_g, data);
	return data;
}


static ADDRESS_MAP_START( bballoon_map, AS_PROGRAM, 32, ghosteo_state )
	AM_RANGE(0x10000000, 0x10000003) AM_READ_PORT("10000000")
	AM_RANGE(0x10100000, 0x10100003) AM_READ_PORT("10100000")
	AM_RANGE(0x10200000, 0x10200003) AM_READ_PORT("10200000")
	AM_RANGE(0x10300000, 0x10300003) AM_WRITE(sound_w)
	AM_RANGE(0x30000000, 0x31ffffff) AM_RAM AM_SHARE("systememory") AM_MIRROR(0x02000000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( touryuu_map, AS_PROGRAM, 32, ghosteo_state )
	AM_RANGE(0x10000000, 0x10000003) AM_READ(touryuu_port_10000000_r)
	AM_RANGE(0x10100000, 0x10100003) AM_READ_PORT("10100000")
	AM_RANGE(0x10200000, 0x10200003) AM_READ_PORT("10200000")
	AM_RANGE(0x10300000, 0x10300003) AM_WRITE(sound_w)
	AM_RANGE(0x30000000, 0x31ffffff) AM_RAM AM_SHARE("systememory") AM_MIRROR(0x02000000)
ADDRESS_MAP_END


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
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE2 )
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
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xFFFFFF50, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const s3c2410_interface bballoon_s3c2410_intf =
{
	// CORE (pin read / pin write)
	{ s3c2410_core_pin_r, NULL },
	// GPIO (port read / port write)
	{ s3c2410_gpio_port_r, s3c2410_gpio_port_w },
	// I2C (scl write / sda read / sda write)
	{ s3c2410_i2c_scl_w, s3c2410_i2c_sda_r, s3c2410_i2c_sda_w },
	// ADC (data read)
	{ NULL },
	// I2S (data write)
	{ NULL },
	// NAND (command write / address write / data read / data write)
	{ s3c2410_nand_command_w, s3c2410_nand_address_w, s3c2410_nand_data_r, s3c2410_nand_data_w }
};

static const i2cmem_interface bballoon_i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, 0, 256
};

static const i2cmem_interface touryuu_i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, 0, 1024
};

device_t* s3c2410;

READ32_MEMBER(ghosteo_state::bballoon_speedup_r)
{
	UINT32 ret = s3c2410_lcd_r(s3c2410, offset+0x10/4, mem_mask);


	int pc = cpu_get_pc(&space.device());

	// these are vblank waits
	if (pc == 0x3001c0e4 || pc == 0x3001c0d8)
	{
		// BnB Arcade
		device_spin_until_time(&space.device(), attotime::from_usec(20));
	}
	else if (pc == 0x3002b580 || pc == 0x3002b550)
	{
		// Happy Tour
		device_spin_until_time(&space.device(), attotime::from_usec(20));
	}
	//else
	//  printf("speedup %08x %08x\n", pc, ret);

	return ret;
}

static MACHINE_RESET( bballoon )
{
	ghosteo_state *state = machine.driver_data<ghosteo_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x4d000010, 0x4d000013,read32_delegate(FUNC(ghosteo_state::bballoon_speedup_r), state));
	s3c2410 = machine.device("s3c2410");
}

static MACHINE_CONFIG_START( ghosteo, ghosteo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM9, 200000000)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(455, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_STATIC(s3c2410)

	MCFG_PALETTE_LENGTH(256)

	MCFG_MACHINE_RESET( bballoon )

	MCFG_VIDEO_START(s3c2410)

	MCFG_S3C2410_ADD("s3c2410", 12000000, bballoon_s3c2410_intf)

//  MCFG_NAND_ADD("nand", 0xEC, 0x75)
//  MCFG_DEVICE_CONFIG(bballoon_nand_intf)

//  MCFG_I2CMEM_ADD("i2cmem", 0xA0, 0, 0x100, NULL)

	/* sound hardware */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bballoon, ghosteo )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bballoon_map)
	MCFG_I2CMEM_ADD("i2cmem", bballoon_i2cmem_interface)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( touryuu, ghosteo )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(touryuu_map)
	MCFG_I2CMEM_ADD("i2cmem", touryuu_i2cmem_interface)
MACHINE_CONFIG_END


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
                  see eolith.c and vegaeo.c drivers
*/

// The NAND dumps are missing the ECC data.  We calculate it on the fly, because the games require it, but really it should be dumped hence the 'BAD DUMP' flags
ROM_START( bballoon )
	ROM_REGION( 0x2000000, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "flash.u1",     0x000000, 0x2000000, BAD_DUMP CRC(73285634) SHA1(4d0210c1bebdf3113a99978ffbcd77d6ee854168) ) // missing ECC data

	// banked every 0x10000 bytes ?
	ROM_REGION( 0x080000, "user2", 0 )
	ROM_LOAD( "b2.u20",       0x000000, 0x080000, CRC(0a12334c) SHA1(535b5b34f28435517218100d70147d87809f485a) )

	ROM_REGION( 0x100000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "b1.u16",       0x000000, 0x100000, CRC(c42c1c85) SHA1(e1f49d556ffd6bc27142a7784c3bb8e37999857d) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u17",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

ROM_START( hapytour ) /* Same hardware: GHOST Ver1.1 2003.03.28 */
	ROM_REGION( 0x2000000, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "flash.u1",     0x000000, 0x2000000, BAD_DUMP CRC(49deb7f9) SHA1(708a27d7177cf6261a49ded975c2bbb6c2427742) ) // missing ECC data

	// banked every 0x10000 bytes ?
	ROM_REGION( 0x080000, "user2", 0 )
	ROM_LOAD( "ht.u20",       0x000000, 0x080000, CRC(c0581fce) SHA1(dafce679002534ffabed249a92e6b83301b8312b) )

	ROM_REGION( 0x100000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "ht.u16",       0x000000, 0x100000, CRC(6a590a3a) SHA1(c1140f70c919661162334db66c6aa0ad656bfc47) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u17",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END


ROM_START( touryuu )
	ROM_REGION( 0x4200000, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "u1.bin",     0x000000, 0x4200000, CRC(49b6856e) SHA1(639123d2fabac4e79c9315fb87f72b13f9ae8761) )

	// banked every 0x10000 bytes ?
	ROM_REGION( 0x080000, "user2", 0 )
	ROM_LOAD( "4m.eeprom_c.s(bad1h).u20",       0x000000, 0x080000, CRC(f81a6530) SHA1(c7fa412102328d06823e73d7d07cadfc25db6d28) )

	ROM_REGION( 0x100000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "8m.eprom_c.s(f8b1h).u16",       0x000000, 0x100000, CRC(238a85ab) SHA1(ddd79429c0c1e67fcbca1e4ebded97ea46229f0b) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u17",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

static DRIVER_INIT( bballoon )
{
	ghosteo_state *state = machine.driver_data<ghosteo_state>();
	state->m_rom_pagesize = 0x200; // extra data is missing from the FLASH dumps and needs to be simulated
}

static DRIVER_INIT( touryuu )
{
	ghosteo_state *state = machine.driver_data<ghosteo_state>();
	state->m_rom_pagesize = 0x210;
}

GAME( 2003, bballoon, 0, bballoon, bballoon, ghosteo_state, bballoon, ROT0, "Eolith", "BnB Arcade", GAME_NO_SOUND )
GAME( 2005, hapytour, 0, bballoon, bballoon, ghosteo_state, bballoon, ROT0, "GAV Company", "Happy Tour", GAME_NO_SOUND )
GAME( 2005, touryuu,  0, touryuu, touryuu, ghosteo_state, touryuu, ROT0, "Yuki Enterprise", "Touryuumon (V1.1)?", GAME_NO_SOUND ) // On first boot inputs won't work, TODO: hook-up default eeprom
