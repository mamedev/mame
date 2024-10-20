// license:BSD-2-Clause
// copyright-holders:Sterophonick, Phil Thomas
/***************************************************************************

    Driver for Gigatron TTL Microcomputer by Sterophonick

    Based on Gigatron.js by Phil Thomas
    https://github.com/PhilThomas/gigatron

***************************************************************************/

#include "emu.h"
#include "cpu/gigatron/gigatron.h"
#include "screen.h"
#include "sound/dac.h"
#include "speaker.h"

#include "gigatron.lh"


namespace {

#define MAIN_CLOCK 6250000
#define VSYNC      0x80
#define HSYNC      0x40

/***************************************************************************

    TODO

    Hook up a quikload for loading .gt1 files
    HLE the keyboard and Pluggy McPlugface

***************************************************************************/


//**************************************************************************
//  Driver Definition
//**************************************************************************

class gigatron_state : public driver_device
{
public:
	gigatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_screen(*this, "screen")
		, m_io_inputs(*this, "GAMEPAD")
		, m_blinken(*this, "blinken%u", 1U)
	{
	}

	void gigatron(machine_config &config);

private:

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

	void prog_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	uint8_t m_lights; //blinkenlights

	//Video Generation stuff
	uint8_t m_out;
	int16_t m_row;
	int16_t m_col;
	uint8_t m_pixel;

	uint8_t m_dacoutput;

	void port_outx(uint8_t data);
	void port_out(uint8_t data);

	std::unique_ptr<bitmap_rgb32> m_bitmap_render;

	required_device<gigatron_cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;
	required_device<screen_device> m_screen;
	required_ioport m_io_inputs;

	output_finder<4> m_blinken;
};

//**************************************************************************
//  Video
//**************************************************************************

void gigatron_state::video_start()
{
	m_bitmap_render = std::make_unique<bitmap_rgb32>(640, 480);
}

void gigatron_state::video_reset()
{
	uint32_t *dest = &m_bitmap_render->pix(0, 0);
	for(uint32_t i = 0; i < 640*480; i++)
		*dest++ = 0;
}

void gigatron_state::port_out(uint8_t data)
{
	m_pixel = data;
	uint8_t out = m_pixel;
	uint8_t falling = m_out & ~out;

	if (falling & VSYNC)
	{
		m_row = -36;
		m_pixel = 0;
	}

	if (falling & HSYNC)
	{
		m_col = -4;
		m_row++;
	}

	m_out = out;

	if ((out & (VSYNC | HSYNC)) != (VSYNC | HSYNC))
	{
		return;
	}

	if((m_row >= 0 && m_row < 480) && (m_col >= 0 && m_col < 640))
	{
		uint8_t r = (out << 6) & 0xC0;
		uint8_t g = (out << 4) & 0xC0;
		uint8_t b = (out << 2) & 0xC0;
		uint32_t *dest = &m_bitmap_render->pix(m_row, m_col);
		for(uint8_t i = 0; i < 4; i++)
			*dest++ = b|(g<<8)|(r<<16);
	}
	m_col += 4;
}

//6-bit color, VGA
uint32_t gigatron_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap_render, 0, 0, 0, 0, cliprect);
	video_reset();

	m_blinken[0] = BIT(m_lights, 3);
	m_blinken[1] = BIT(m_lights, 2);
	m_blinken[2] = BIT(m_lights, 1);
	m_blinken[3] = BIT(m_lights, 0);

	return 0;
}

//**************************************************************************
//  Memory Map
//**************************************************************************

void gigatron_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void gigatron_state::data_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
}

//**************************************************************************
//  Machine
//**************************************************************************

static INPUT_PORTS_START(gigatron)
	PORT_START("GAMEPAD")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1) // START
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)    // SELECT
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B Button") PORT_PLAYER(1)    // B Button
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A Button") PORT_PLAYER(1)    // A Button
INPUT_PORTS_END

void gigatron_state::machine_start()
{
	//blinkenlights
	m_blinken.resolve();

	//Savestate stuff
	save_item(NAME(m_lights));
	save_item(NAME(m_out));
	save_item(NAME(m_row));
	save_item(NAME(m_col));
	save_item(NAME(m_pixel));
	save_item(NAME(m_dacoutput));
}

void gigatron_state::machine_reset()
{
	m_dacoutput = 0;
	m_dac->write(0);
	m_lights = 0;
	m_out = 0;
	m_row = 0;
	m_col = 0;
	m_pixel = 0;
}

void gigatron_state::port_outx(uint8_t data)
{
	//Write sound to DAC
	m_dacoutput = (data & 0xF0) >> 4;
	m_dac->write(m_dacoutput);

	//Blinkenlights
	m_lights = data & 0xF;
}

void gigatron_state::gigatron(machine_config &config)
{
	config.set_default_layout(layout_gigatron);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_4BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5);

	GTRON(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gigatron_state::prog_map);
	m_maincpu->set_addrmap(AS_DATA, &gigatron_state::data_map);
	m_maincpu->outx_cb().set(FUNC(gigatron_state::port_outx));
	m_maincpu->out_cb().set(FUNC(gigatron_state::port_out));
	m_maincpu->ir_cb().set_ioport("GAMEPAD").invert();

	/* video hardware */
	screen_device &m_screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	m_screen.set_refresh_hz(59.98);
	m_screen.set_size(640, 480);
	m_screen.set_visarea(0, 640-1, 0, 480-1);
	m_screen.set_screen_update(FUNC(gigatron_state::screen_update));
}

ROM_START( gigatron )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v5a", "Gigatron ROM v5a")
	ROMX_LOAD( "gigrom5a.rom",  0x0000, 0x20000, CRC(dcc071a6) SHA1(f82059ba0227ff48e4c687b90c8445da30213ee2),ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v4", "Gigatron ROM v4")
	ROMX_LOAD( "gigrom4.rom",  0x0000, 0x20000, CRC(78995109) SHA1(2395fc48e64099836111f5aeca39ddbf4650ea4e),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "Gigatron ROM v3")
	ROMX_LOAD( "gigrom3.rom",  0x0000, 0x20000, CRC(1536efbe) SHA1(959268069e761a01d620396eedb9abc1ee63c421),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v2", "Gigatron ROM v2")
	ROMX_LOAD( "gigrom2.rom",  0x0000, 0x20000, CRC(b4a3d936) SHA1(c93f417d589144b912c79f85b9e942d66242c2c3),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v1", "Gigatron ROM v1")
	ROMX_LOAD( "gigrom1.rom",  0x0000, 0x20000, CRC(8ea5a2af) SHA1(e5758d5cc467c3476bd8f992fd45dfcdf06d0430),ROM_BIOS(4))
ROM_END

} // anonymous namespace


COMP(2018, gigatron, 0, 0, gigatron, gigatron, gigatron_state, empty_init, "Marcel van Kervinck / Walter Belgers", "Gigatron TTL Microcomputer", MACHINE_SUPPORTS_SAVE)
