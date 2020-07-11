// license:BSD-3-Clause
// copyright-holders:Sterophonick, Phil Thomas
/***************************************************************************

   Skeleton driver for Gigatron TTL Microcomputer
   Driver by Sterophonick

***************************************************************************/

//There is a pre-existing emulator here https://github.com/PhilThomas/gigatron
//It's just a matter of translating it to MAME.

#include "emu.h"
#include "cpu/gigatron/gigatron.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK 6250000
#define VSYNC      0x80
#define HSYNC      0x40

class gigatron_state : public driver_device
{
public:
	gigatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_io_inputs(*this, "GAMEPAD")
	{
	}

	void gigatron(machine_config &config);


private:

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void prog_map(address_map &map);
	void data_map(address_map &map);

	uint16_t lights_changed;

	//Video Generation stuff
	uint8_t machineOut;
	uint8_t row;
	uint8_t col;
	uint8_t pixel;

	void blinkenlights(uint8_t data);
	void video_draw(u8 data);
	uint8_t inputs();

	std::unique_ptr<bitmap_ind16> m_bitmap_render;
	std::unique_ptr<bitmap_ind16> m_bitmap_buffer;

	required_device<gigatron_cpu_device> m_maincpu;
	required_ioport m_io_inputs;
};

void gigatron_state::video_start()
{
	m_bitmap_render = std::make_unique<bitmap_ind16>(640, 480);
	m_bitmap_buffer = std::make_unique<bitmap_ind16>(640, 480);
}

void gigatron_state::video_draw(u8 data)
{
	uint8_t out = data;
	uint8_t falling = machineOut & ~out;

	if (falling & VSYNC)
	{
		row = 0;
		pixel = 0;
	}

	if (falling & HSYNC)
	{
		col = 0;
		row++;
	}

	machineOut = out;

	if ((out & (VSYNC | HSYNC)) != (VSYNC | HSYNC))
	{
		return;
	}

	if((row >= 0 && row < 480) && (col >= 0 && col < 640))
	{
		//uint16_t *dest;
		//uint8_t tPixel = pixel;
		//uint8_t r = (out << 6) & 0xC0;
		//uint8_t g = (out << 4) & 0xC0;
		//uint8_t b = (out << 2) & 0xC0;
	}
}

uint32_t gigatron_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void gigatron_state::prog_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
}

void gigatron_state::data_map(address_map &map)
{
	map(0x0000, 0x7fff).ram();
}

void gigatron_state::machine_start()
{
}

void gigatron_state::machine_reset()
{
}

void gigatron_state::blinkenlights(uint8_t data)
{
	uint16_t light = data & 0xF;
	lights_changed ^= light;
}

uint8_t gigatron_state::inputs()
{
	return m_io_inputs->read() ^ 0xFF;
}

static INPUT_PORTS_START(gigatron)
	PORT_START("GAMEPAD")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) // START
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SELECT ) PORT_PLAYER(1)    // SELECT
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("B Button") PORT_PLAYER(1)    // B Button
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("A Button") PORT_PLAYER(1)    // A Button
INPUT_PORTS_END

void gigatron_state::gigatron(machine_config &config)
{
	GTRON(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gigatron_state::prog_map);
	m_maincpu->set_addrmap(AS_DATA, &gigatron_state::data_map);
	m_maincpu->outx_cb().set(FUNC(gigatron_state::blinkenlights));
	m_maincpu->out_cb().set(FUNC(gigatron_state::video_draw));
	m_maincpu->ir_cb().set(FUNC(gigatron_state::inputs));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.98);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(gigatron_state::screen_update));

	/* sound hardware */
	//SPEAKER(config, "mono").front_center();
}

ROM_START( gigatron )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v5a", "Gigatron ROM V5a")
	ROMX_LOAD( "gigrom5a.rom",  0x0000, 0x20000, CRC(DCC071A6) SHA1(F82059BA0227FF48E4C687B90C8445DA30213EE2),ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v4", "Gigatron ROM V4")
	ROMX_LOAD( "gigrom4.rom",  0x0000, 0x20000, CRC(78995109) SHA1(2395fc48e64099836111f5aeca39ddbf4650ea4e),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "Gigatron ROM V3")
	ROMX_LOAD( "gigrom3.rom",  0x0000, 0x20000, CRC(1536efbe) SHA1(959268069e761a01d620396eedb9abc1ee63c421),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v2", "Gigatron ROM V2")
	ROMX_LOAD( "gigrom2.rom",  0x0000, 0x20000, CRC(b4a3d936) SHA1(c93f417d589144b912c79f85b9e942d66242c2c3),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v1", "Gigatron ROM V1")
	ROMX_LOAD( "gigrom1.rom",  0x0000, 0x20000, CRC(8ea5a2af) SHA1(e5758d5cc467c3476bd8f992fd45dfcdf06d0430),ROM_BIOS(4))
ROM_END

COMP(2018, gigatron, 0, 0, gigatron, gigatron, gigatron_state, empty_init, "Marcel van Kervinck / Walter Belgers", "Gigatron TTL Microcomputer", MACHINE_IS_SKELETON)
