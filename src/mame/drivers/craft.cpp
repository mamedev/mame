// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

 ATmega88-based demo platforms by Linus Åkesson

**********************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"

#define VISIBLE_CYCLES      480
#define HSYNC_CYCLES        155
#define LINE_CYCLES         (VISIBLE_CYCLES + HSYNC_CYCLES)
#define VISIBLE_LINES       480
#define VSYNC_LINES         45
#define LINES_PER_FRAME     (VISIBLE_LINES + VSYNC_LINES)
#define CYCLES_PER_FRAME    (LINES_PER_FRAME * LINE_CYCLES)
#define PIXELS_PER_FRAME    (CYCLES_PER_FRAME)

class lft_state : public driver_device
{
public:
	lft_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
	{
	}

	void base_config(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual uint32_t master_clock() const = 0;

	void prg_map(address_map &map);
	void data_map(address_map &map);
	void io_map(address_map &map);

	uint8_t port_r(offs_t offset);
	void port_w(offs_t offset, uint8_t data);

	virtual void port_update(const offs_t offset, const uint8_t old, const uint8_t data) = 0;

	required_device<avr8_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;

	enum port : uint8_t
	{
		PORT_A,
		PORT_B,
		PORT_C,
		PORT_D,

		PORT_COUNT
	};

	uint8_t m_ports[PORT_COUNT];
};

class powernin_state : public lft_state
{
public:
	powernin_state(const machine_config &mconfig, device_type type, const char *tag)
		: lft_state(mconfig, type, tag)
	{
	}

	void powernin(machine_config &config);

private:
	virtual uint32_t master_clock() const override { return 8000000; }

	virtual void port_update(const offs_t offset, const uint8_t old, const uint8_t data) override;
};

class craft_state : public lft_state
{
public:
	craft_state(const machine_config &mconfig, device_type type, const char *tag)
		: lft_state(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{
	}

	void craft(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual uint32_t master_clock() const override { return 20000000; }

	virtual void port_update(const offs_t offset, const uint8_t old, const uint8_t data) override;

	void init_palette(palette_device &palette) const;
	void video_update();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint32_t m_last_cycles;
	uint64_t m_frame_start_cycle;

	uint8_t m_latched_color;
	std::unique_ptr<uint8_t[]> m_pixels;
};

//**************************************************************************
//  GPIO
//**************************************************************************

uint8_t lft_state::port_r(offs_t offset)
{
	return m_ports[offset];
}

void lft_state::port_w(offs_t offset, uint8_t data)
{
	const uint8_t old = m_ports[offset];
	m_ports[offset] = data;
	port_update(offset, old, data);
}

void craft_state::port_update(const offs_t offset, const uint8_t old, const uint8_t data)
{
	const uint8_t changed = data ^ old;
	switch (offset)
	{
	case AVR8_IO_PORTA:
		break;

	case AVR8_IO_PORTB:
		if (BIT(changed, 1) && BIT(data, 1))
		{
			m_frame_start_cycle = machine().time().as_ticks(master_clock());
			video_update();
		}
		if (BIT(changed, 3))
		{
			video_update();
			m_latched_color = (data & 0x08) ? (m_ports[PORT_C] & 0x3f) : 0x3f;
		}
		break;

	case AVR8_IO_PORTC:
		video_update();
		m_latched_color = data;
		break;

	case AVR8_IO_PORTD:
		m_dac->write((data & 0x02) | ((data & 0xf4) >> 2));
		break;
	}
}

void powernin_state::port_update(const offs_t offset, const uint8_t old, const uint8_t data)
{
	if (offset == AVR8_IO_PORTD)
		m_dac->write(data);
}

//**************************************************************************
//  MEMORY
//**************************************************************************

void lft_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void lft_state::data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}

void lft_state::io_map(address_map &map)
{
	map(AVR8_IO_PORTA, AVR8_IO_PORTD).rw(FUNC(lft_state::port_r), FUNC(lft_state::port_w));
}

//**************************************************************************
//  VIDEO
//**************************************************************************

void craft_state::init_palette(palette_device &palette) const
{
	for (int i = 0; i < 0x40; i++)
	{
		uint8_t r = 0x55 * ((i & 0x30) >> 4);
		uint8_t g = 0x55 * ((i & 0x0c) >> 2);
		uint8_t b = 0x55 * (i & 0x03);
		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void craft_state::video_update()
{
	uint64_t cycles = machine().time().as_ticks(master_clock());
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
	const pen_t *pens = m_palette->pens();
	for(int y = 0; y < LINES_PER_FRAME; y++)
	{
		uint32_t *dst = &bitmap.pix32(y);
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

void lft_state::machine_start()
{
	save_item(NAME(m_ports));
}

void lft_state::machine_reset()
{
	memset(m_ports, 0, PORT_COUNT);
}

void craft_state::machine_start()
{
	lft_state::machine_start();

	m_pixels = std::make_unique<uint8_t[]>(PIXELS_PER_FRAME);

	save_item(NAME(m_last_cycles));
	save_item(NAME(m_frame_start_cycle));

	save_item(NAME(m_latched_color));
	save_pointer(NAME(m_pixels), PIXELS_PER_FRAME);
}

void craft_state::machine_reset()
{
	lft_state::machine_reset();

	m_frame_start_cycle = 0;
	m_last_cycles = 0;

	m_latched_color = 0;
	memset(&m_pixels[0], 0, PIXELS_PER_FRAME);
}

void lft_state::base_config(machine_config &config)
{
	/* basic machine hardware */
	ATMEGA88(config, m_maincpu, master_clock());
	m_maincpu->set_addrmap(AS_PROGRAM, &lft_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &lft_state::data_map);
	m_maincpu->set_addrmap(AS_IO, &lft_state::io_map);
	m_maincpu->set_eeprom_tag("eeprom");

	/* sound hardware */
	SPEAKER(config, "avr8").front_center();
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, m_dac, 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, m_dac, -1.0, DAC_VREF_NEG_INPUT);
}

void craft_state::craft(machine_config &config)
{
	base_config(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(master_clock(), 635, 47, 527, 525, 36, 516);
	m_screen->set_screen_update(FUNC(craft_state::screen_update));

	PALETTE(config, m_palette, FUNC(craft_state::init_palette), 64);

	DAC_6BIT_R2R(config, m_dac, 0).add_route(0, "avr8", 0.25); // pd1/pd2/pd4/pd5/pd6/pd7 + 2k(x7) + 1k(x5)
}

void powernin_state::powernin(machine_config &config)
{
	base_config(config);

	DAC_8BIT_R2R(config, m_dac, 0).add_route(0, "avr8", 0.9);
}

ROM_START( craft )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "craft.bin", 0x0000, 0x2000, CRC(2e6f9ad2) SHA1(75e495bf18395d74289ca7ee2649622fc4010457) )
	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom.raw", 0x0000, 0x0200, CRC(e18a2af9) SHA1(81fc6f2d391edfd3244870214fac37929af0ac0c) )
ROM_END

ROM_START( powernin )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "powernin.bin", 0x0000, 0x2000, CRC(67458936) SHA1(26a86846a24dd974723a66bea6c22baf51c7bec9) )
	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom.raw", 0x0000, 0x0200, CRC(bd7bc39f) SHA1(9d0ac37bb3ec8c95990fd37a962a17a95ce97aa0) )
ROM_END

/*   YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT        CLASS           INIT        COMPANY          FULLNAME */
CONS(2008, craft,    0,      0,      craft,      empty_input, craft_state,    empty_init, "Linus Åkesson", "Craft", MACHINE_IMPERFECT_GRAPHICS)
CONS(2009, powernin, 0,      0,      powernin,   empty_input, powernin_state, empty_init, "Linus Åkesson", "Power Ninja Action Challenge", 0)
