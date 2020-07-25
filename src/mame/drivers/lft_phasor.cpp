// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

 Phasor, an ATmega88-based demo by Linus Åkesson

**********************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"

#define MASTER_CLOCK		17734470

#define SAMPLES_PER_FRAME    (355255)

class lft_phasor_state : public driver_device
{
public:
	lft_phasor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{
	}

	void phasor(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void prg_map(address_map &map);
	void data_map(address_map &map);
	void io_map(address_map &map);

	uint8_t port_r(offs_t offset);
	void port_w(offs_t offset, uint8_t data);

	void init_palette(palette_device &palette) const;
	void video_update();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<avr8_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	enum port : uint8_t
	{
		PORT_A,
		PORT_B,
		PORT_C,
		PORT_D,

		PORT_COUNT
	};

	uint8_t m_ports[PORT_COUNT];

	uint64_t m_last_cycles;
	uint64_t m_frame_start_cycle;

	uint8_t m_latched_sample;
	bool m_in_blanking;
	uint64_t m_blanking_start;
	uint32_t m_sample_x;
	uint32_t m_sample_y;
	std::unique_ptr<uint8_t[]> m_samples;
};

//**************************************************************************
//  GPIO
//**************************************************************************

uint8_t lft_phasor_state::port_r(offs_t offset)
{
	return m_ports[offset];
}

void lft_phasor_state::port_w(offs_t offset, uint8_t data)
{
	m_ports[offset] = data;

	switch (offset)
	{
	case AVR8_IO_PORTB:
		video_update();
		break;

	case AVR8_IO_PORTC:
		m_dac->write(data & 0x3f);
		break;

	case AVR8_IO_PORTD:
		//video_update();
		m_latched_sample = data;
		break;
	}
}

//**************************************************************************
//  MEMORY
//**************************************************************************

void lft_phasor_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void lft_phasor_state::data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}

void lft_phasor_state::io_map(address_map &map)
{
	map(AVR8_IO_PORTA, AVR8_IO_PORTD).rw(FUNC(lft_phasor_state::port_r), FUNC(lft_phasor_state::port_w));
}

//**************************************************************************
//  VIDEO
//**************************************************************************

void lft_phasor_state::init_palette(palette_device &palette) const
{
	for (int i = 0; i < 0x10; i++)
	{
		uint8_t gray = (uint8_t)i;
		gray |= gray << 4;
		palette.set_pen_color(i, rgb_t(gray, gray, gray));
	}
}

void lft_phasor_state::video_update()
{
	const uint64_t cycles = machine().time().as_ticks(MASTER_CLOCK);

	if (cycles == m_last_cycles)
		return;

	if (m_latched_sample == 0 && !m_in_blanking)
	{
		m_in_blanking = true;
		m_blanking_start = cycles;
	}
	else if (m_latched_sample != 0 && m_in_blanking)
	{
		m_in_blanking = false;
		const uint64_t blank_duration = cycles - m_blanking_start;
		if (blank_duration < 80) // Approximate length of hblank
		{
			m_sample_y++;
			m_sample_x = 0;
		}
		else
		{
			m_sample_y = 0;
			m_sample_x = 0;
			m_frame_start_cycle = machine().time().as_ticks(MASTER_CLOCK);
		}
	}

	if (m_last_cycles < cycles && !m_in_blanking)
	{
		const uint8_t shift = (m_ports[PORT_B] & 4);
		uint32_t sample_pix = m_sample_y * 1135 + m_sample_x;
		for (uint64_t idx = m_last_cycles; idx < cycles && sample_pix < SAMPLES_PER_FRAME; idx++)
		{
			m_samples[sample_pix++] = (m_latched_sample >> shift) & 0x0f;
			m_sample_x++;
		}
	}

	m_last_cycles = cycles;
}

uint32_t lft_phasor_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = m_palette->pens();
	for(int y = 0; y < 313; y++)
	{
		uint32_t *dst = &bitmap.pix32(y);
		uint8_t *src = &m_samples[y * 1135];
		for(int x = 0; x < 1135; x++)
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

void lft_phasor_state::machine_start()
{
	m_samples = std::make_unique<uint8_t[]>(SAMPLES_PER_FRAME);

	save_item(NAME(m_ports));
	save_item(NAME(m_last_cycles));
	save_item(NAME(m_frame_start_cycle));

	save_item(NAME(m_latched_sample));
	save_item(NAME(m_in_blanking));
	save_item(NAME(m_blanking_start));
	save_item(NAME(m_sample_x));
	save_item(NAME(m_sample_y));
	save_pointer(NAME(m_samples), SAMPLES_PER_FRAME);
}

void lft_phasor_state::machine_reset()
{
	memset(m_ports, 0, PORT_COUNT);

	m_frame_start_cycle = 0;
	m_last_cycles = 0;

	m_latched_sample = 0;
	m_in_blanking = true;
	m_blanking_start = 0;
	m_sample_x = 0;
	m_sample_y = 0;
	memset(&m_samples[0], 0, SAMPLES_PER_FRAME);
}

void lft_phasor_state::phasor(machine_config &config)
{
	ATMEGA88(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &lft_phasor_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &lft_phasor_state::data_map);
	m_maincpu->set_addrmap(AS_IO, &lft_phasor_state::io_map);
	m_maincpu->set_eeprom_tag("eeprom");

	PALETTE(config, m_palette, FUNC(lft_phasor_state::init_palette), 0x10);
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK, 1135, 0, 1064, 313, 6, 310);
	m_screen->set_screen_update(FUNC(lft_phasor_state::screen_update));

	SPEAKER(config, "avr8").front_center();
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, m_dac, 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, m_dac, -1.0, DAC_VREF_NEG_INPUT);

	DAC_6BIT_R2R(config, m_dac, 0).add_route(0, "avr8", 0.5);
}

ROM_START( phasor )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "phasor.bin", 0x0000, 0x2000, CRC(300ef49b) SHA1(36b26137f5e8359dc9c2b746621a98bdd6634d2f) )
	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom.raw", 0x0000, 0x0200, CRC(49036547) SHA1(d98c4d02771e80499c56dd71ad3d07597102f9b7) )
ROM_END

/*   YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT        CLASS             INIT        COMPANY            FULLNAME */
CONS(2010, phasor,   0,      0,      phasor,     empty_input, lft_phasor_state, empty_init, u8"Linus Åkesson", "Phasor", MACHINE_IMPERFECT_GRAPHICS)
