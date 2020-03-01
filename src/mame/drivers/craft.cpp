// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

 "Craft" demo platform by Linus Akesson

**********************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"

#define MASTER_CLOCK    20000000

#define VISIBLE_CYCLES      480
#define HSYNC_CYCLES        155
#define LINE_CYCLES         (VISIBLE_CYCLES + HSYNC_CYCLES)
#define VISIBLE_LINES       480
#define VSYNC_LINES         45
#define LINES_PER_FRAME     (VISIBLE_LINES + VSYNC_LINES)
#define CYCLES_PER_FRAME    (LINES_PER_FRAME * LINE_CYCLES)
#define PIXELS_PER_FRAME    (CYCLES_PER_FRAME)

class craft_state : public driver_device
{
public:
	craft_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_dac(*this, "dac")
	{
	}

	void craft(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void craft_prg_map(address_map &map);
	void craft_data_map(address_map &map);
	void craft_io_map(address_map &map);

	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(port_w);

	void video_update();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<avr8_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<dac_byte_interface> m_dac;

	uint32_t m_last_cycles;
	uint64_t m_frame_start_cycle;

	uint8_t m_port_b;
	uint8_t m_port_c;
	uint8_t m_port_d;

	uint8_t m_latched_color;
	std::unique_ptr<uint8_t[]> m_pixels;
};

//**************************************************************************
//  GPIO
//**************************************************************************

READ8_MEMBER(craft_state::port_r)
{
	switch (offset)
	{
		case 0x00: // Port A
			break;
		case 0x01: // Port B
			return m_port_b;
		case 0x02: // Port C
			return m_port_c;
		case 0x03: // Port D
			return m_port_d;
	}

	return 0;
}

WRITE8_MEMBER(craft_state::port_w)
{
	switch (offset)
	{
		case AVR8_IO_PORTA:
			break;

		case AVR8_IO_PORTB:
		{
			uint8_t old_port_b = m_port_b;
			uint8_t pins = data;
			uint8_t changed = pins ^ old_port_b;
			if (BIT(changed, 1) && BIT(data, 1))
			{
				m_frame_start_cycle = machine().time().as_ticks(MASTER_CLOCK);
				video_update();
			}
			if (BIT(changed, 3))
			{
				video_update();
				m_latched_color = (pins & 0x08) ? (m_port_c & 0x3f) : 0x3f;
			}
			m_port_b = data;
			break;
		}

		case AVR8_IO_PORTC:
			video_update();
			m_port_c = data;
			m_latched_color = m_port_c;
			break;

		case AVR8_IO_PORTD:
			m_port_d = data;
			m_dac->write((data & 0x02) | ((data & 0xf4) >> 2));
			break;
	}
}

//**************************************************************************
//  MEMORY
//**************************************************************************

void craft_state::craft_prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void craft_state::craft_data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}

void craft_state::craft_io_map(address_map &map)
{
	map(AVR8_IO_PORTA, AVR8_IO_PORTD).rw(FUNC(craft_state::port_r), FUNC(craft_state::port_w));
}

//**************************************************************************
//  VIDEO
//**************************************************************************

void craft_state::video_update()
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
	else
	{
		uint32_t end_clear = sizeof(m_pixels) - m_last_cycles;
		uint32_t start_clear = frame_cycles;
		end_clear = (end_clear > PIXELS_PER_FRAME) ? (PIXELS_PER_FRAME - m_last_cycles) : end_clear;
		start_clear = (start_clear > PIXELS_PER_FRAME) ? PIXELS_PER_FRAME : start_clear;
		if (m_last_cycles < PIXELS_PER_FRAME)
		{
			//memset(&m_pixels[0] + m_last_cycles, 0, end_clear);
		}
		if (start_clear < PIXELS_PER_FRAME)
		{
			//memset(&m_pixels[0], 0, start_clear);
		}
	}

	m_last_cycles = frame_cycles;
}

uint32_t craft_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for(int y = 0; y < LINES_PER_FRAME; y++)
	{
		uint32_t *line = &bitmap.pix32(y);
		for(int x = 0; x < LINE_CYCLES; x++)
		{
			uint8_t pixel = m_pixels[y * LINE_CYCLES + x];
			uint8_t r = 0x55 * ((pixel & 0x30) >> 4);
			uint8_t g = 0x55 * ((pixel & 0x0c) >> 2);
			uint8_t b = 0x55 * (pixel & 0x03);
			line[x] = 0xff000000 | (r << 16) | (g << 8) | b;
		}
	}
	return 0;
}

//**************************************************************************
//  MACHINE
//**************************************************************************

static INPUT_PORTS_START( craft )
INPUT_PORTS_END

void craft_state::machine_start()
{
	m_pixels = std::make_unique<uint8_t[]>(PIXELS_PER_FRAME);

	save_item(NAME(m_last_cycles));
	save_item(NAME(m_frame_start_cycle));

	save_item(NAME(m_port_b));
	save_item(NAME(m_port_c));
	save_item(NAME(m_port_d));

	save_item(NAME(m_latched_color));
	save_pointer(NAME(m_pixels), PIXELS_PER_FRAME);
}

void craft_state::machine_reset()
{
	m_frame_start_cycle = 0;
	m_last_cycles = 0;

	m_port_b = 0;
	m_port_c = 0;
	m_port_d = 0;

	m_latched_color = 0;
	memset(&m_pixels[0], 0, PIXELS_PER_FRAME);
}

void craft_state::craft(machine_config &config)
{
	/* basic machine hardware */
	ATMEGA88(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &craft_state::craft_prg_map);
	m_maincpu->set_addrmap(AS_DATA, &craft_state::craft_data_map);
	m_maincpu->set_addrmap(AS_IO, &craft_state::craft_io_map);
	m_maincpu->set_eeprom_tag("eeprom");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK, 635, 47, 527, 525, 36, 516);
	m_screen->set_screen_update(FUNC(craft_state::screen_update));

	/* sound hardware */
	SPEAKER(config, "avr8").front_center();
	DAC_6BIT_R2R(config, "dac", 0).add_route(0, "avr8", 0.25); // pd1/pd2/pd4/pd5/pd6/pd7 + 2k(x7) + 1k(x5)
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

ROM_START( craft )
	ROM_REGION( 0x2000, "maincpu", 0 )  /* Main program store */
	ROM_LOAD( "craft.bin", 0x0000, 0x2000, CRC(2e6f9ad2) SHA1(75e495bf18395d74289ca7ee2649622fc4010457) )
	ROM_REGION( 0x200, "eeprom", 0 )  /* on-die eeprom */
	ROM_LOAD( "eeprom.raw", 0x0000, 0x0200, CRC(e18a2af9) SHA1(81fc6f2d391edfd3244870214fac37929af0ac0c) )
ROM_END

/*   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY          FULLNAME */
CONS(2008, craft, 0,      0,      craft,   craft, craft_state, empty_init, "Linus Akesson", "Craft", MACHINE_IMPERFECT_GRAPHICS)
