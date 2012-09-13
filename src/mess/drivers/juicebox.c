/*******************************************************************************

    Mattel Juice Box

    (C) 2011 Tim Schuerewegen

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c44b0.h"
#include "machine/smartmed.h"
#include "sound/dac.h"
#include "rendlay.h"

//#define JUICEBOX_ENTER_DEBUG_MENU
//#define JUICEBOX_DISPLAY_ROM_ID

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context(), buf);
	}
}

typedef struct
{
	int add_latch;
	int cmd_latch;
	int busy;
} smc_t;

class juicebox_state : public driver_device
{
public:
	juicebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	UINT32 port[9];
	device_t *s3c44b0;
	smc_t smc;
	dac_device *dac;
	#if defined(JUICEBOX_ENTER_DEBUG_MENU) || defined(JUICEBOX_DISPLAY_ROM_ID)
	int port_g_read_count;
	#endif
	DECLARE_READ32_MEMBER(juicebox_nand_r);
	DECLARE_WRITE32_MEMBER(juicebox_nand_w);
	DECLARE_DRIVER_INIT(juicebox);
	virtual void machine_start();
	virtual void machine_reset();
};

/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/


// SMARTMEDIA

static void smc_reset( running_machine &machine)
{
	juicebox_state *state = machine.driver_data<juicebox_state>();
	verboselog( machine, 5, "smc_reset\n");
	state->smc.add_latch = 0;
	state->smc.cmd_latch = 0;
}

static void smc_init( running_machine &machine)
{
	verboselog( machine, 5, "smc_init\n");
	smc_reset( machine);
}

static UINT8 smc_read( running_machine &machine)
{
	smartmedia_image_device *smartmedia = machine.device<smartmedia_image_device>( "smartmedia");
	UINT8 data;
	if (smartmedia->is_present())
	{
		data = smartmedia->data_r();
	}
	else
	{
		data = 0xFF;
	}
	verboselog( machine, 5, "smc_read %08X\n", data);
	return data;
}

static void smc_write( running_machine &machine, UINT8 data)
{
	juicebox_state *state = machine.driver_data<juicebox_state>();
	smartmedia_image_device *smartmedia = machine.device<smartmedia_image_device>( "smartmedia");
	verboselog( machine, 5, "smc_write %08X\n", data);
	if (smartmedia->is_present())
	{
		if (state->smc.cmd_latch)
		{
			verboselog( machine, 5, "smartmedia_command_w %08X\n", data);
			smartmedia->command_w(data);
		}
		else if (state->smc.add_latch)
		{
			verboselog( machine, 5, "smartmedia_address_w %08X\n", data);
			smartmedia->address_w(data);
		}
		else
		{
			verboselog( machine, 5, "smartmedia_data_w %08X\n", data);
			smartmedia->data_w(data);
		}
	}
}

static UINT32 s3c44b0_gpio_port_r( device_t *device, int port)
{
	juicebox_state *juicebox = device->machine().driver_data<juicebox_state>();
	UINT32 data = juicebox->port[port];
	switch (port)
	{
		case S3C44B0_GPIO_PORT_A :
		{
			data = 0x00000210;
		}
		break;
		case S3C44B0_GPIO_PORT_B :
		{
			data = 0x00000000;
		}
		break;
		case S3C44B0_GPIO_PORT_C :
		{
			// 0C0062C4 -> bit 8 is turned on
			//data |= 1 << 13;
			data = 0x0000FF15;
			data &= (1 << 2) | (1 << 9) | (1 << 10) | (1 << 11);
			data |= (1 << 2) | (1 << 9) | (1 << 10) | (1 << 11);
		}
		break;
		case S3C44B0_GPIO_PORT_D :
		{
			data = 0x00000010;
		}
		break;
		case S3C44B0_GPIO_PORT_E :
		{
			data = 0x000000DF;
		}
		break;
		case S3C44B0_GPIO_PORT_F :
		{
			data = data | (0 << 7);
			data = 0x0000009F;
			data &= ~0xE0;
			if (juicebox->smc.cmd_latch) data = data | 0x00000020;
			if (juicebox->smc.add_latch) data = data | 0x00000040;
			if (!juicebox->smc.busy) data = data | 0x00000080;
		}
		break;
		case S3C44B0_GPIO_PORT_G :
		{
			data = 0x0000009F;
			data = (data & ~0x1F) | (device->machine().root_device().ioport( "PORTG")->read() & 0x1F);
			#if defined(JUICEBOX_ENTER_DEBUG_MENU)
			if (juicebox->port_g_read_count++ < 1)
			{
				data = 0x00000095; // PLAY + REVERSE
			}
			#elif defined(JUICEBOX_DISPLAY_ROM_ID)
			if (juicebox->port_g_read_count++ < 3)
			{
				data = 0x0000008A; // RETURN + FORWARD + STAR
			}
			#endif
		}
		break;
	}
//  data = ((rand() & 0xFF) << 24) | ((rand() & 0xFF) << 16) | ((rand() & 0xFF) << 8) | ((rand() & 0xFF) << 0);
	return data;
}

static void s3c44b0_gpio_port_w( device_t *device, int port, UINT32 data)
{
	juicebox_state *juicebox = device->machine().driver_data<juicebox_state>();
	juicebox->port[port] = data;
	switch (port)
	{
		case S3C44B0_GPIO_PORT_F :
		{
			juicebox->smc.cmd_latch = ((data & 0x00000020) != 0);
			juicebox->smc.add_latch = ((data & 0x00000040) != 0);
			verboselog( device->machine(), 5, "s3c44b0_gpio_port_w - nand cle %d ale %d\n", (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0);
		}
		break;
	}
}

// ...

READ32_MEMBER(juicebox_state::juicebox_nand_r)
{
	UINT32 data = 0;
	if (mem_mask & 0x000000FF) data = data | (smc_read(machine()) <<  0);
	if (mem_mask & 0x0000FF00) data = data | (smc_read(machine()) <<  8);
	if (mem_mask & 0x00FF0000) data = data | (smc_read(machine()) << 16);
	if (mem_mask & 0xFF000000) data = data | (smc_read(machine()) << 24);
	verboselog( machine(), 5, "juicebox_nand_r %08X %08X %08X\n", offset, mem_mask, data);
	return data;
}

WRITE32_MEMBER(juicebox_state::juicebox_nand_w)
{
	verboselog( machine(), 5, "juicebox_nand_w %08X %08X %08X\n", offset, mem_mask, data);
	if (mem_mask & 0x000000FF) smc_write(machine(), (data >>  0) & 0xFF);
	if (mem_mask & 0x0000FF00) smc_write(machine(), (data >>  8) & 0xFF);
	if (mem_mask & 0x00FF0000) smc_write(machine(), (data >> 16) & 0xFF);
	if (mem_mask & 0xFF000000) smc_write(machine(), (data >> 24) & 0xFF);
}

// I2S

static WRITE16_DEVICE_HANDLER( s3c44b0_i2s_data_w )
{
	juicebox_state *juicebox = device->machine().driver_data<juicebox_state>();
	juicebox->dac->write_signed16(data ^ 0x8000);
}

// ...

static INPUT_CHANGED( port_changed )
{
	running_machine &machine = field.machine();
	juicebox_state *juicebox = machine.driver_data<juicebox_state>();
	s3c44b0_request_eint( juicebox->s3c44b0, (FPTR)param);
}

// ...

void juicebox_state::machine_start()
{
	s3c44b0 = machine().device( "s3c44b0");
	smc_init( machine());
	dac = machine().device<dac_device>( "dac");
}

void juicebox_state::machine_reset()
{
	machine().device("maincpu")->reset();
	smc_reset( machine());
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( juicebox_map, AS_PROGRAM, 32, juicebox_state )
	AM_RANGE(0x00000000, 0x007fffff) AM_ROM
	AM_RANGE(0x04000000, 0x04ffffff) AM_READWRITE(juicebox_nand_r, juicebox_nand_w )
	AM_RANGE(0x0c000000, 0x0c1fffff) AM_RAM AM_MIRROR(0x00600000)
ADDRESS_MAP_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

DRIVER_INIT_MEMBER(juicebox_state,juicebox)
{
	// do nothing
}

static S3C44B0_INTERFACE( juicebox_s3c44b0_intf )
{
	// GPIO (port read / port write)
	{ s3c44b0_gpio_port_r, s3c44b0_gpio_port_w },
	// I2C (scl write / sda read / sda write)
	{ NULL, NULL, NULL },
	// ADC (data read)
	{ NULL },
	// I2S (data write)
	{ s3c44b0_i2s_data_w }
};

static MACHINE_CONFIG_START( juicebox, juicebox_state )
	MCFG_CPU_ADD("maincpu", ARM7, 66000000)
	MCFG_CPU_PROGRAM_MAP(juicebox_map)

	MCFG_PALETTE_LENGTH(32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(240, 160)
	MCFG_SCREEN_VISIBLE_AREA(0, 240 - 1, 0, 160 - 1)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_VIDEO_START(s3c44b0)
	MCFG_SCREEN_UPDATE_STATIC(s3c44b0)

	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 1.0)


	MCFG_S3C44B0_ADD("s3c44b0", 10000000, juicebox_s3c44b0_intf)

	MCFG_SMARTMEDIA_ADD("smartmedia")
	MCFG_SMARTMEDIA_INTERFACE("smartmedia")
	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","juicebox")
MACHINE_CONFIG_END

static INPUT_PORTS_START( juicebox )
	PORT_START( "PORTG" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED(port_changed, (void *)0) PORT_NAME("RETURN") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED(port_changed, (void *)0) PORT_NAME("PLAY") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED(port_changed, (void *)0) PORT_NAME("FORWARD") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CHANGED(port_changed, (void *)0) PORT_NAME("REVERSE") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CHANGED(port_changed, (void *)0) PORT_NAME("STAR") PORT_PLAYER(1)
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( juicebox )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "juicebox", "Juice Box v.28 08242004" )
	ROMX_LOAD( "juicebox.rom", 0, 0x800000, CRC(ac731197) SHA1(8278891c3531b3b6b5fec2a97a3ef6f0de1ac81d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "uclinuxp", "uClinux 2.4.24-uc0 (patched)" )
	ROMX_LOAD( "newboot.rom", 0, 0x1A0800, CRC(443f48b7) SHA1(38f0dc07b5cf02b972a851aa9e87f5d93d03f629), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "uclinux", "uClinux 2.4.24-uc0" )
	ROMX_LOAD( "image.rom", 0, 0x19E400, CRC(6c0308bf) SHA1(5fe21a38a4cd0d86bb60920eb100138b0e924d90), ROM_BIOS(3) )
ROM_END

COMP(2004, juicebox, 0, 0, juicebox, juicebox, juicebox_state, juicebox, "Mattel", "Juice Box", 0)
