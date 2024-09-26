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
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include <cstdarg>


namespace {

//#define JUICEBOX_ENTER_DEBUG_MENU
//#define JUICEBOX_DISPLAY_ROM_ID

#define VERBOSE_LEVEL ( 0 )

class juicebox_state : public driver_device
{
public:
	juicebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s3c44b0(*this, "s3c44b0")
		, m_smartmedia(*this, "smartmedia")
		, m_port_g(*this, "PORTG")
	{ }

	void juicebox(machine_config &config);

	void init_juicebox();

	DECLARE_INPUT_CHANGED_MEMBER(port_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<s3c44b0_device> m_s3c44b0;
	required_device<smartmedia_image_device> m_smartmedia;
	required_ioport m_port_g;

	uint32_t port[9];

	struct jb_smc_t
	{
		int add_latch;
		int cmd_latch;
		int busy;
	};

	jb_smc_t smc;

	#if defined(JUICEBOX_ENTER_DEBUG_MENU) || defined(JUICEBOX_DISPLAY_ROM_ID)
	int port_g_read_count;
	#endif
	uint32_t juicebox_nand_r(offs_t offset, uint32_t mem_mask = ~0);
	void juicebox_nand_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);
	void smc_reset();
	void smc_init();
	uint8_t smc_read();
	void smc_write(uint8_t data);
	uint32_t s3c44b0_gpio_port_r(offs_t offset);
	void s3c44b0_gpio_port_w(offs_t offset, uint32_t data);
	//void s3c44b0_i2s_data_w(offs_t offset, uint16_t data);
	void juicebox_map(address_map &map) ATTR_COLD;
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
	smc.busy = 0;
}

void juicebox_state::smc_init( )
{
	verboselog(5, "smc_init\n");
	smc_reset();
}

uint8_t juicebox_state::smc_read( )
{
	uint8_t data;
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

void juicebox_state::smc_write( uint8_t data)
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

uint32_t juicebox_state::s3c44b0_gpio_port_r(offs_t offset)
{
	uint32_t data = port[offset];
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
			data = (data & ~0x1F) | (m_port_g->read() & 0x1F);
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
//  data = ((machine().rand() & 0xFF) << 24) | ((machine().rand() & 0xFF) << 16) | ((machine().rand() & 0xFF) << 8) | ((machine().rand() & 0xFF) << 0);
	return data;
}

void juicebox_state::s3c44b0_gpio_port_w(offs_t offset, uint32_t data)
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

uint32_t juicebox_state::juicebox_nand_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	if (ACCESSING_BITS_0_7) data = data | (smc_read() <<  0);
	if (ACCESSING_BITS_8_15) data = data | (smc_read() <<  8);
	if (ACCESSING_BITS_16_23) data = data | (smc_read() << 16);
	if (ACCESSING_BITS_24_31) data = data | (smc_read() << 24);
	verboselog( 5, "juicebox_nand_r %08X %08X %08X\n", offset, mem_mask, data);
	return data;
}

void juicebox_state::juicebox_nand_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( 5, "juicebox_nand_w %08X %08X %08X\n", offset, mem_mask, data);
	if (ACCESSING_BITS_0_7) smc_write((data >>  0) & 0xFF);
	if (ACCESSING_BITS_8_15) smc_write((data >>  8) & 0xFF);
	if (ACCESSING_BITS_16_23) smc_write((data >> 16) & 0xFF);
	if (ACCESSING_BITS_24_31) smc_write((data >> 24) & 0xFF);
}

// ...

INPUT_CHANGED_MEMBER(juicebox_state::port_changed)
{
	m_s3c44b0->request_eint(param);
}

// ...

void juicebox_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	smc_init();

	space.install_readwrite_handler(0x01c00000, 0x01c0000b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::cpuwrap_r)), write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::cpuwrap_w)));
	space.install_readwrite_handler(0x01d00000, 0x01d0002b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::uart_0_r)),  write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::uart_0_w)));
	space.install_readwrite_handler(0x01d04000, 0x01d0402b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::uart_1_r)),  write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::uart_1_w)));
	space.install_readwrite_handler(0x01d14000, 0x01d14013, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::sio_r)),     write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::sio_w)));
	space.install_readwrite_handler(0x01d18000, 0x01d18013, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::iis_r)),     write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::iis_w)));
	space.install_readwrite_handler(0x01d20000, 0x01d20057, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::gpio_r)),    write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::gpio_w)));
	space.install_readwrite_handler(0x01d30000, 0x01d3000b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::wdt_r)),     write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::wdt_w)));
	space.install_readwrite_handler(0x01d40000, 0x01d4000b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::adc_r)),     write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::adc_w)));
	space.install_readwrite_handler(0x01d50000, 0x01d5004f, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::pwm_r)),     write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::pwm_w)));
	space.install_readwrite_handler(0x01d60000, 0x01d6000f, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::iic_r)),     write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::iic_w)));
	space.install_readwrite_handler(0x01d80000, 0x01d8000f, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::clkpow_r)),  write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::clkpow_w)));
	space.install_readwrite_handler(0x01e00000, 0x01e0003f, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::irq_r)),     write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::irq_w)));
	space.install_readwrite_handler(0x01e80000, 0x01e8001b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::zdma_0_r)),  write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::zdma_0_w)));
	space.install_readwrite_handler(0x01e80020, 0x01e8003b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::zdma_1_r)),  write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::zdma_1_w)));
	space.install_readwrite_handler(0x01f00000, 0x01f00047, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::lcd_r)),     write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::lcd_w)));
	space.install_readwrite_handler(0x01f80000, 0x01f8001b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::bdma_0_r)),  write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::bdma_0_w)));
	space.install_readwrite_handler(0x01f80020, 0x01f8003b, read32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::bdma_1_r)),  write32s_delegate(*m_s3c44b0, FUNC(s3c44b0_device::bdma_1_w)));
}

void juicebox_state::machine_reset()
{
	m_maincpu->reset();
	smc_reset();
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void juicebox_state::juicebox_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom();
	map(0x04000000, 0x04ffffff).rw(FUNC(juicebox_state::juicebox_nand_r), FUNC(juicebox_state::juicebox_nand_w));
	map(0x0c000000, 0x0c1fffff).ram().mirror(0x00600000);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void juicebox_state::init_juicebox()
{
	// do nothing
}

void juicebox_state::juicebox(machine_config &config)
{
	ARM7(config, m_maincpu, 66000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &juicebox_state::juicebox_map);

	PALETTE(config, "palette").set_entries(32768);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(240, 160);
	screen.set_visarea(0, 240 - 1, 0, 160 - 1);
	screen.set_screen_update("s3c44b0", FUNC(s3c44b0_device::video_update));

	SPEAKER(config, "speaker").front_center();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC

	S3C44B0(config, m_s3c44b0, 10000000);
	m_s3c44b0->set_cpu("maincpu");
	m_s3c44b0->gpio_port_r_cb().set(FUNC(juicebox_state::s3c44b0_gpio_port_r));
	m_s3c44b0->gpio_port_w_cb().set(FUNC(juicebox_state::s3c44b0_gpio_port_w));
	m_s3c44b0->i2s_data_w_cb().set("dac", FUNC(dac_word_interface::data_w));

	SMARTMEDIA(config, m_smartmedia, 0);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("juicebox");
}

static INPUT_PORTS_START( juicebox )
	PORT_START( "PORTG" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, 0) PORT_NAME("RETURN") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, 0) PORT_NAME("PLAY") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, 0) PORT_NAME("FORWARD") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, 0) PORT_NAME("REVERSE") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, juicebox_state, port_changed, 0) PORT_NAME("STAR") PORT_PLAYER(1)
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( juicebox )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "juicebox", "Juice Box v.28 08242004" )
	ROMX_LOAD( "juicebox.rom", 0, 0x800000, CRC(ac731197) SHA1(8278891c3531b3b6b5fec2a97a3ef6f0de1ac81d), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "uclinuxp", "uClinux 2.4.24-uc0 (patched)" )
	ROMX_LOAD( "newboot.rom", 0, 0x1A0800, CRC(443f48b7) SHA1(38f0dc07b5cf02b972a851aa9e87f5d93d03f629), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "uclinux", "uClinux 2.4.24-uc0" )
	ROMX_LOAD( "image.rom", 0, 0x19E400, CRC(6c0308bf) SHA1(5fe21a38a4cd0d86bb60920eb100138b0e924d90), ROM_BIOS(2) )
ROM_END

} // Anonymous namespace


COMP(2004, juicebox, 0, 0, juicebox, juicebox, juicebox_state, init_juicebox, "Mattel", "Juice Box", 0)
