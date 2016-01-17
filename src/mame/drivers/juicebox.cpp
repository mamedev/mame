// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
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
#include "softlist.h"

//#define JUICEBOX_ENTER_DEBUG_MENU
//#define JUICEBOX_DISPLAY_ROM_ID

#define VERBOSE_LEVEL ( 0 )

struct jb_smc_t
{
	int add_latch;
	int cmd_latch;
	int busy;
};

class juicebox_state : public driver_device
{
public:
	juicebox_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_dac(*this, "dac"),
			m_s3c44b0(*this, "s3c44b0"),
			m_smartmedia(*this, "smartmedia")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_device<s3c44b0_device> m_s3c44b0;
	required_device<smartmedia_image_device> m_smartmedia;
	UINT32 port[9];
	jb_smc_t smc;

	#if defined(JUICEBOX_ENTER_DEBUG_MENU) || defined(JUICEBOX_DISPLAY_ROM_ID)
	int port_g_read_count;
	#endif
	DECLARE_READ32_MEMBER(juicebox_nand_r);
	DECLARE_WRITE32_MEMBER(juicebox_nand_w);
	DECLARE_DRIVER_INIT(juicebox);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_INPUT_CHANGED_MEMBER(port_changed);
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);
	void smc_reset( );
	void smc_init( );
	UINT8 smc_read( );
	void smc_write( UINT8 data);
	DECLARE_READ32_MEMBER(s3c44b0_gpio_port_r);
	DECLARE_WRITE32_MEMBER(s3c44b0_gpio_port_w);
	DECLARE_WRITE16_MEMBER(s3c44b0_i2s_data_w);
};

inline void juicebox_state::verboselog(int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine().describe_context(), buf);
	}
}


/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/


// SMARTMEDIA

void juicebox_state::smc_reset( )
{
	verboselog(5, "smc_reset\n");
	smc.add_latch = 0;
	smc.cmd_latch = 0;
}

void juicebox_state::smc_init( )
{
	verboselog(5, "smc_init\n");
	smc_reset();
}

UINT8 juicebox_state::smc_read( )
{
	UINT8 data;
	if (m_smartmedia->is_present())
	{
		data = m_smartmedia->data_r();
	}
	else
	{
		data = 0xFF;
	}
	verboselog(5, "smc_read %08X\n", data);
	return data;
}

void juicebox_state::smc_write( UINT8 data)
{
	verboselog(5, "smc_write %08X\n", data);
	if (m_smartmedia->is_present())
	{
		if (smc.cmd_latch)
		{
			verboselog(5, "smartmedia_command_w %08X\n", data);
			m_smartmedia->command_w(data);
		}
		else if (smc.add_latch)
		{
			verboselog(5, "smartmedia_address_w %08X\n", data);
			m_smartmedia->address_w(data);
		}
		else
		{
			verboselog(5, "smartmedia_data_w %08X\n", data);
			m_smartmedia->data_w(data);
		}
	}
}

READ32_MEMBER(juicebox_state::s3c44b0_gpio_port_r)
{
	UINT32 data = port[offset];
	switch (offset)
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
			if (smc.cmd_latch) data = data | 0x00000020;
			if (smc.add_latch) data = data | 0x00000040;
			if (!smc.busy) data = data | 0x00000080;
		}
		break;
		case S3C44B0_GPIO_PORT_G :
		{
			data = 0x0000009F;
			data = (data & ~0x1F) | (ioport( "PORTG")->read() & 0x1F);
			#if defined(JUICEBOX_ENTER_DEBUG_MENU)
			if (port_g_read_count++ < 1)
			{
				data = 0x00000095; // PLAY + REVERSE
			}
			#elif defined(JUICEBOX_DISPLAY_ROM_ID)
			if (port_g_read_count++ < 3)
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

WRITE32_MEMBER(juicebox_state::s3c44b0_gpio_port_w)
{
	port[offset] = data;
	switch (offset)
	{
		case S3C44B0_GPIO_PORT_F :
		{
			smc.cmd_latch = ((data & 0x00000020) != 0);
			smc.add_latch = ((data & 0x00000040) != 0);
			verboselog( 5, "s3c44b0_gpio_port_w - nand cle %d ale %d\n", (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0);
		}
		break;
	}
}

// ...

READ32_MEMBER(juicebox_state::juicebox_nand_r)
{
	UINT32 data = 0;
	if (mem_mask & 0x000000FF) data = data | (smc_read() <<  0);
	if (mem_mask & 0x0000FF00) data = data | (smc_read() <<  8);
	if (mem_mask & 0x00FF0000) data = data | (smc_read() << 16);
	if (mem_mask & 0xFF000000) data = data | (smc_read() << 24);
	verboselog( 5, "juicebox_nand_r %08X %08X %08X\n", offset, mem_mask, data);
	return data;
}

WRITE32_MEMBER(juicebox_state::juicebox_nand_w)
{
	verboselog( 5, "juicebox_nand_w %08X %08X %08X\n", offset, mem_mask, data);
	if (mem_mask & 0x000000FF) smc_write((data >>  0) & 0xFF);
	if (mem_mask & 0x0000FF00) smc_write((data >>  8) & 0xFF);
	if (mem_mask & 0x00FF0000) smc_write((data >> 16) & 0xFF);
	if (mem_mask & 0xFF000000) smc_write((data >> 24) & 0xFF);
}

// I2S

WRITE16_MEMBER(juicebox_state::s3c44b0_i2s_data_w )
{
	m_dac->write_signed16(data ^ 0x8000);
}

// ...

INPUT_CHANGED_MEMBER(juicebox_state::port_changed)
{
	m_s3c44b0->request_eint((FPTR)param);
}

// ...

void juicebox_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	smc_init();

	space.install_readwrite_handler(0x01c00000, 0x01c0000b, 0, 0, read32_delegate(FUNC(s3c44b0_device::cpuwrap_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::cpuwrap_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d00000, 0x01d0002b, 0, 0, read32_delegate(FUNC(s3c44b0_device::uart_0_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::uart_0_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d04000, 0x01d0402b, 0, 0, read32_delegate(FUNC(s3c44b0_device::uart_1_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::uart_1_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d14000, 0x01d14013, 0, 0, read32_delegate(FUNC(s3c44b0_device::sio_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::sio_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d18000, 0x01d18013, 0, 0, read32_delegate(FUNC(s3c44b0_device::iis_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::iis_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d20000, 0x01d20057, 0, 0, read32_delegate(FUNC(s3c44b0_device::gpio_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::gpio_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d30000, 0x01d3000b, 0, 0, read32_delegate(FUNC(s3c44b0_device::wdt_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::wdt_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d40000, 0x01d4000b, 0, 0, read32_delegate(FUNC(s3c44b0_device::adc_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::adc_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d50000, 0x01d5004f, 0, 0, read32_delegate(FUNC(s3c44b0_device::pwm_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::pwm_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d60000, 0x01d6000f, 0, 0, read32_delegate(FUNC(s3c44b0_device::iic_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::iic_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01d80000, 0x01d8000f, 0, 0, read32_delegate(FUNC(s3c44b0_device::clkpow_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::clkpow_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01e00000, 0x01e0003f, 0, 0, read32_delegate(FUNC(s3c44b0_device::irq_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::irq_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01e80000, 0x01e8001b, 0, 0, read32_delegate(FUNC(s3c44b0_device::zdma_0_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::zdma_0_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01e80020, 0x01e8003b, 0, 0, read32_delegate(FUNC(s3c44b0_device::zdma_1_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::zdma_1_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01f00000, 0x01f00047, 0, 0, read32_delegate(FUNC(s3c44b0_device::lcd_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::lcd_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01f80000, 0x01f8001b, 0, 0, read32_delegate(FUNC(s3c44b0_device::bdma_0_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::bdma_0_w), &(*m_s3c44b0)));
	space.install_readwrite_handler(0x01f80020, 0x01f8003b, 0, 0, read32_delegate(FUNC(s3c44b0_device::bdma_1_r), &(*m_s3c44b0)),  write32_delegate(FUNC(s3c44b0_device::bdma_1_w), &(*m_s3c44b0)));
}

void juicebox_state::machine_reset()
{
	m_maincpu->reset();
	smc_reset();
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

static MACHINE_CONFIG_START( juicebox, juicebox_state )
	MCFG_CPU_ADD("maincpu", ARM7, 66000000)
	MCFG_CPU_PROGRAM_MAP(juicebox_map)

	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(240, 160)
	MCFG_SCREEN_VISIBLE_AREA(0, 240 - 1, 0, 160 - 1)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SCREEN_UPDATE_DEVICE("s3c44b0", s3c44b0_device, video_update)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)


	MCFG_DEVICE_ADD("s3c44b0", S3C44B0, 10000000)
	MCFG_S3C44B0_GPIO_PORT_R_CB(READ32(juicebox_state, s3c44b0_gpio_port_r))
	MCFG_S3C44B0_GPIO_PORT_W_CB(WRITE32(juicebox_state, s3c44b0_gpio_port_w))
	MCFG_S3C44B0_I2S_DATA_W_CB(WRITE16(juicebox_state, s3c44b0_i2s_data_w))

	MCFG_DEVICE_ADD("smartmedia", SMARTMEDIA, 0)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","juicebox")
MACHINE_CONFIG_END

static INPUT_PORTS_START( juicebox )
	PORT_START( "PORTG" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, (void *)nullptr) PORT_NAME("RETURN") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, (void *)nullptr) PORT_NAME("PLAY") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, (void *)nullptr) PORT_NAME("FORWARD") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, (void *)nullptr) PORT_NAME("REVERSE") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, (void *)nullptr) PORT_NAME("STAR") PORT_PLAYER(1)
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
