// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

 Craft, an ATmega88-based demo by Linus Åkesson

**********************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"

namespace {

#define MASTER_CLOCK        20000000

#define VISIBLE_CYCLES      480
#define HSYNC_CYCLES        155
#define LINE_CYCLES         (VISIBLE_CYCLES + HSYNC_CYCLES)
#define VISIBLE_LINES       480
#define VSYNC_LINES         45
#define LINES_PER_FRAME     (VISIBLE_LINES + VSYNC_LINES)
#define CYCLES_PER_FRAME    (LINES_PER_FRAME * LINE_CYCLES)
#define PIXELS_PER_FRAME    (CYCLES_PER_FRAME)

class lft_craft_state : public driver_device
{
public:
	lft_craft_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{
	}

	void craft(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void prg_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	void port_b_w(uint8_t data);
	void port_c_w(uint8_t data);
	void port_d_w(uint8_t data);

	void init_palette(palette_device &palette) const;
	void video_update();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<atmega88_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t m_gpio_b = 0;
	uint8_t m_gpio_c = 0;

	uint32_t m_last_cycles = 0;
	uint64_t m_frame_start_cycle = 0;

	uint8_t m_latched_color = 0;
	std::unique_ptr<uint8_t[]> m_pixels;
};

//**************************************************************************
//  GPIO
//**************************************************************************

void lft_craft_state::port_b_w(uint8_t data)
{
	const uint8_t old = m_gpio_b;
	m_gpio_b = data;
	const uint8_t changed = data ^ old;

	if (BIT(changed, 1) && BIT(data, 1))
	{
		m_frame_start_cycle = machine().time().as_ticks(MASTER_CLOCK);
		video_update();
	}
	if (BIT(changed, 3))
	{
		video_update();
		m_latched_color = (data & 0x08) ? (m_gpio_c & 0x3f) : 0x3f;
	}
}

void lft_craft_state::port_c_w(uint8_t data)
{
	m_gpio_c = data;
	video_update();
	m_latched_color = data;
}

void lft_craft_state::port_d_w(uint8_t data)
{
	m_dac->write((data & 0x02) | ((data & 0xf4) >> 2));
}

//**************************************************************************
//  MEMORY
//**************************************************************************

void lft_craft_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void lft_craft_state::data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}

//**************************************************************************
//  VIDEO
//**************************************************************************

void lft_craft_state::init_palette(palette_device &palette) const
{
	for (int i = 0; i < 0x40; i++)
	{
		uint8_t r = 0x55 * ((i & 0x30) >> 4);
		uint8_t g = 0x55 * ((i & 0x0c) >> 2);
		uint8_t b = 0x55 * (i & 0x03);
		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void lft_craft_state::video_update()
{
	uint64_t cycles = machine().time().as_ticks(MASTER_CLOCK);
	uint32_t frame_cycles = (uint32_t)(cycles - m_frame_start_cycle);

	if (m_last_cycles < frame_cycles)
	{
		for (uint32_t pixidx = m_last_cycles; pixidx < frame_cycles && pixidx < PIXELS_PER_FRAME; pixidx++)
		{
			m_pixels[pixidx] = m_latched_color;
		}
	}

	m_last_cycles = frame_cycles;
}

uint32_t lft_craft_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = m_palette->pens();
	for(int y = 0; y < LINES_PER_FRAME; y++)
	{
		uint32_t *dst = &bitmap.pix(y);
		uint8_t *src = &m_pixels[y * LINE_CYCLES];
		for(int x = 0; x < LINE_CYCLES; x++)
		{
			*dst++ = pens[*src++];
		}
	}
	return 0;
}

//**************************************************************************
//  MACHINE
//**************************************************************************

static INPUT_PORTS_START( empty_input )
INPUT_PORTS_END

void lft_craft_state::machine_start()
{
	m_pixels = std::make_unique<uint8_t[]>(PIXELS_PER_FRAME);

	save_item(NAME(m_gpio_b));
	save_item(NAME(m_gpio_c));
	save_item(NAME(m_last_cycles));
	save_item(NAME(m_frame_start_cycle));

	save_item(NAME(m_latched_color));
	save_pointer(NAME(m_pixels), PIXELS_PER_FRAME);
}

void lft_craft_state::machine_reset()
{
	m_gpio_b = 0;
	m_gpio_c = 0;

	m_frame_start_cycle = 0;
	m_last_cycles = 0;

	m_latched_color = 0;
	memset(&m_pixels[0], 0, PIXELS_PER_FRAME);
}

void lft_craft_state::craft(machine_config &config)
{
	ATMEGA88(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &lft_craft_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &lft_craft_state::data_map);
	m_maincpu->set_eeprom_tag("eeprom");
	m_maincpu->gpio_in<atmega88_device::GPIOB>().set([this]() { return m_gpio_b; });
	m_maincpu->gpio_in<atmega88_device::GPIOC>().set([this]() { return m_gpio_c; });
	m_maincpu->gpio_out<atmega88_device::GPIOB>().set(FUNC(lft_craft_state::port_b_w));
	m_maincpu->gpio_out<atmega88_device::GPIOC>().set(FUNC(lft_craft_state::port_c_w));
	m_maincpu->gpio_out<atmega88_device::GPIOD>().set(FUNC(lft_craft_state::port_d_w));

	PALETTE(config, m_palette, FUNC(lft_craft_state::init_palette), 64);
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK, 635, 47, 527, 525, 36, 516);
	m_screen->set_screen_update(FUNC(lft_craft_state::screen_update));

	SPEAKER(config, "avr8").front_center();

	DAC_6BIT_R2R(config, m_dac, 0).add_route(0, "avr8", 0.25); // pd1/pd2/pd4/pd5/pd6/pd7 + 2k(x7) + 1k(x5)
}

ROM_START( craft )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "craft.bin", 0x0000, 0x2000, CRC(2e6f9ad2) SHA1(75e495bf18395d74289ca7ee2649622fc4010457) )
	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom.raw", 0x0000, 0x0200, CRC(e18a2af9) SHA1(81fc6f2d391edfd3244870214fac37929af0ac0c) )
ROM_END

} // anonymous namespace


/*   YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT        CLASS            INIT        COMPANY            FULLNAME */
CONS(2008, craft,    0,      0,      craft,      empty_input, lft_craft_state, empty_init, u8"Linus Åkesson", "Craft", MACHINE_IMPERFECT_GRAPHICS)
