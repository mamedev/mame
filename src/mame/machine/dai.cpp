// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Nathan Woods
/***************************************************************************

  machine/dai.c

  Functions to emulate general aspects of DAI (RAM, ROM, interrupts, I/O ports)

  Krzysztof Strzecha
  Nathan Woods

***************************************************************************/

#include "emu.h"
#include "includes/dai.h"

#define DEBUG_DAI_PORTS 0

#define LOG_DAI_PORT_R(_port, _data, _comment) do { if (DEBUG_DAI_PORTS) logerror ("DAI port read : %04x, Data: %02x (%s)\n", _port, _data, _comment); } while (0)
#define LOG_DAI_PORT_W(_port, _data, _comment) do { if (DEBUG_DAI_PORTS) logerror ("DAI port write: %04x, Data: %02x (%s)\n", _port, _data, _comment); } while (0)

TIMER_DEVICE_CALLBACK_MEMBER(dai_state::tms_timer)
{
	m_tms5501->xi7_w(BIT(m_io_keyboard[8]->read(), 2));
	m_tms_timer->adjust(attotime::from_hz(100));
}


/* Discrete I/O devices */


/* Memory */

void dai_state::stack_interrupt_circuit_w(uint8_t data)
{
	m_tms5501->sens_w(1);
	m_tms5501->sens_w(0);
}

uint8_t dai_state::keyboard_r()
{
	uint8_t data = 0x00;

	for (u8 i = 0; i < 8; i++)
		if (m_keyboard_scan_mask & (1 << i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void dai_state::keyboard_w(uint8_t data)
{
	m_keyboard_scan_mask = data;
}

IRQ_CALLBACK_MEMBER(dai_state::int_ack)
{
	return m_tms5501->get_vector();
}

void dai_state::machine_start()
{
	membank("bank2")->configure_entries(0, 4, m_rom + 0x2000, 0x1000);
	m_tms_timer->adjust(attotime::from_hz(100));
	save_item(NAME(m_paddle_select));
	save_item(NAME(m_paddle_enable));
	save_item(NAME(m_cassette_motor));
	save_item(NAME(m_keyboard_scan_mask));
	save_item(NAME(m_4_colours_palette));
}

void dai_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xc000, 0xc7ff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

/***************************************************************************

    Discrete Devices IO

    FD00    POR1:   IN  bit 0   -
                bit 1   -
                bit 2   PIPGE: Page signal
                bit 3   PIDTR: Serial output ready
                bit 4   PIBU1: Button on paddle 1 (1 = closed)
                bit 5   PIBU1: Button on paddle 2 (1 = closed)
                bit 6   PIRPI: Random data
                bit 7   PICAI: Cassette input data

    FD01    PDLST:  IN  Single pulse used to trigger paddle timer circuit

    FD04    POR1:   OUT bit 0-3 Volume oscillator channel 0
                bit 4-7 Volume oscillator channel 1

    FD05    POR1:   OUT bit 0-3 Volume oscillator channel 2
                bit 4-7 Volume random noise generator

    FD06    POR0:   OUT bit 0   POCAS: Cassette data output
                bit 1-2 PDLMSK: Paddle select
                bit 3   PDPNA:  Paddle enable
                bit 4   POCM1:  Cassette 1 motor control (0 = run)
                bit 5   POCM2:  Cassette 2 motor control (0 = run)
                bit 6-7         ROM bank switching
***************************************************************************/

uint8_t dai_state::io_discrete_devices_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch(offset & 0x0f) {
	case 0:
		data = m_io_keyboard[8]->read();
		data |= 0x08;           // serial ready
		if (machine().rand()&0x01)
			data |= 0x40;       // random number generator
		if (m_cassette->input() > 0.01)
			data |= 0x80;       // tape input
		break;

	default:
		LOG_DAI_PORT_R (offset, data, "discrete devices - unmapped");
		break;
	}
	return data;
}

void dai_state::io_discrete_devices_w(offs_t offset, uint8_t data)
{
	switch(offset & 0x000f) {
	case 0x04:
		m_sound->set_volume(offset, data);
		LOG_DAI_PORT_W (offset, data&0x0f, "discrete devices - osc. 0 volume");
		LOG_DAI_PORT_W (offset, BIT(data, 4, 4), "discrete devices - osc. 1 volume");
		break;

	case 0x05:
		m_sound->set_volume(offset, data);
		LOG_DAI_PORT_W (offset, data&0x0f, "discrete devices - osc. 2 volume");
		LOG_DAI_PORT_W (offset, BIT(data, 4, 4), "discrete devices - noise volume");
		break;

	case 0x06:
		m_paddle_select = BIT(data, 1, 2);
		m_paddle_enable = BIT(data, 3);
		m_cassette_motor[0] = BIT(data, 4);
		m_cassette_motor[1] = BIT(data, 5);
		m_cassette->change_state(m_cassette_motor[0]?CASSETTE_MOTOR_DISABLED:CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		m_cassette->output(BIT(data, 0) ? -1.0 : 1.0);
		membank("bank2")->set_entry(BIT(data, 6, 2));
		LOG_DAI_PORT_W (offset, BIT(data, 1, 2), "discrete devices - paddle select");
		LOG_DAI_PORT_W (offset, BIT(data, 3), "discrete devices - paddle enable");
		LOG_DAI_PORT_W (offset, BIT(data, 4), "discrete devices - cassette motor 1");
		LOG_DAI_PORT_W (offset, BIT(data, 5), "discrete devices - cassette motor 2");
		LOG_DAI_PORT_W (offset, BIT(data, 6, 2), "discrete devices - ROM bank");
		break;

	default:
		LOG_DAI_PORT_W (offset, data, "discrete devices - unmapped");
		break;
	}
}

/***************************************************************************

    PIT8253

    Offset need to be shifted by 1 to right, because the PIT is
    connected to A1 and A2

***************************************************************************/

uint8_t dai_state::pit_r(offs_t offset)
{
	return m_pit->read(BIT(offset, 1, 2));
}

void dai_state::pit_w(offs_t offset, uint8_t data)
{
	m_pit->write(BIT(offset, 1, 2), data);
}

/***************************************************************************

    AMD 9911 mathematical coprocesor

***************************************************************************/

uint8_t dai_state::amd9511_r()
{
	/* optional and not present at this moment */
	return 0xff;
}

void dai_state::amd9511_w(offs_t offset, uint8_t data)
{
	logerror ("Writing to AMD9511 math chip, %04x, %02x\n", offset, data);
}
