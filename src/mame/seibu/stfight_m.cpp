// license:BSD-3-Clause
// copyright-holders:Mark McDougall
/***************************************************************************

  stfight.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/stfight.h"

#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"


/*

Encryption PAL 16R4 on CPU board

          +---U---+
     CP --|       |-- VCC
 ROM D1 --|       |-- ROM D0          M1 = 0                M1 = 1
 ROM D3 --|       |-- (NC)
 ROM D4 --|       |-- D6         D6 = D1 ^^ D3          D6 = / ( D1 ^^ D0 )
 ROM D6 --|       |-- D4         D4 = / ( D6 ^^ A7 )    D4 = D3 ^^ A0
     A0 --|       |-- D3         D3 = / ( D0 ^^ A1 )    D3 = D4 ^^ A4
     A1 --|       |-- D0         D0 = D1 ^^ D4          D0 = / ( D6 ^^ A0 )
     A4 --|       |-- (NC)
     A7 --|       |-- /M1
    GND --|       |-- /OE
          +-------+

*/


void stfight_state::init_stfight()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (uint32_t A = 0; A < 0x8000; ++A)
	{
		uint8_t src = rom[A];

		// decode opcode
		m_decrypted_opcodes[A] =
				( src & 0xA6 ) |
				( ( ( ( src << 2 ) ^ src ) << 3 ) & 0x40 ) |
				( ~( ( src ^ ( A >> 1 ) ) >> 2 ) & 0x10 ) |
				( ~( ( ( src << 1 ) ^ A ) << 2 ) & 0x08 ) |
				( ( ( src ^ ( src >> 3 ) ) >> 1 ) & 0x01 );

		// decode operand
		rom[A] =
				( src & 0xA6 ) |
				( ~( ( src ^ ( src << 1 ) ) << 5 ) & 0x40 ) |
				( ( ( src ^ ( A << 3 ) ) << 1 ) & 0x10 ) |
				( ( ( src ^ A ) >> 1 ) & 0x08 ) |
				( ~( ( src >> 6 ) ^ A ) & 0x01 );
	}

	// Set clock prescaler FM:1/2 PSG:1/1
	m_ym[0]->write(0, 0x2f);
	m_ym[1]->write(0, 0x2f);
}

void stfight_state::machine_start()
{
	m_main_bank->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);
	m_main_bank->set_entry(0);

	m_int1_timer = timer_alloc(FUNC(stfight_state::rst08_tick), this);

	save_item(NAME(m_coin_state));
	save_item(NAME(m_fm_data));

	save_item(NAME(m_cpu_to_mcu_empty));
	save_item(NAME(m_cpu_to_mcu_data));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_port_c_out));

	save_item(NAME(m_vck2));
	save_item(NAME(m_adpcm_reset));
	save_item(NAME(m_adpcm_data_offs));
}


void stfight_state::machine_reset()
{
	m_fm_data = 0;
	m_cpu_to_mcu_empty = true;
	m_adpcm_reset = true;

	// Coin signals are active low
	m_coin_state = 3;
}

// It's entirely possible that this bank is never switched out
// - in fact I don't even know how/where it's switched in!
void stfight_state::stfight_bank_w(uint8_t data)
{
	m_main_bank->set_entry(bitswap(data, 7, 2));
}

/*
 *      CPU 1 timed interrupt - 60Hz???
 */

TIMER_CALLBACK_MEMBER(stfight_state::rst08_tick)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80
}

INTERRUPT_GEN_MEMBER(stfight_state::stfight_vb_interrupt)
{
	// Do a RST10
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80
	m_int1_timer->adjust(attotime::from_hz(120));
}

/*
 *      Hardware handlers
 */

void stfight_state::stfight_io_w(uint8_t data)
{
	// TODO: What is bit 4?
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

uint8_t stfight_state::stfight_coin_r()
{
	return m_coin_state;
}

void stfight_state::stfight_coin_w(uint8_t data)
{
	// Acknowledge coin signals (active low)
	if (!BIT(data, 0))
		m_coin_state |= 1;

	if (!BIT(data, 1))
		m_coin_state |= 2;
}

/*
 *      Machine hardware for MSM5205 ADPCM sound control
 */

WRITE_LINE_MEMBER(stfight_state::stfight_adpcm_int)
{
	if (!state)
		return;

	// Falling edge triggered interrupt at half the rate of /VCK?
	m_mcu->set_input_line(M68705_IRQ_LINE, m_vck2 ? ASSERT_LINE : CLEAR_LINE);
	m_vck2 = !m_vck2;

	if (!m_adpcm_reset)
	{
		uint8_t adpcm_data = m_samples[(m_adpcm_data_offs >> 1) & 0x7fff];

		if (!BIT(m_adpcm_data_offs, 0))
			adpcm_data >>= 4;
		++m_adpcm_data_offs;

		m_msm->data_w(adpcm_data & 0x0f);
	}
}


/*
 *      Machine hardware for YM2303 FM sound control
 */

void stfight_state::stfight_fm_w(uint8_t data)
{
	// The sound cpu ignores any FM data without bit 7 set
	m_fm_data = 0x80 | data;
}

uint8_t stfight_state::stfight_fm_r()
{
	uint8_t const data = m_fm_data;

	// Acknowledge the command
	if (!machine().side_effects_disabled())
		m_fm_data &= ~0x80;

	return data;
}


/*
 *  MCU communications
 */

void stfight_state::stfight_mcu_w(uint8_t data)
{
	m_cpu_to_mcu_data = data & 0x0f;
	m_cpu_to_mcu_empty = false;
}

void stfight_state::stfight_68705_port_a_w(uint8_t data)
{
	m_port_a_out = data;
}

uint8_t stfight_state::stfight_68705_port_b_r()
{
	return
			(m_coin_mech->read() << 6) |
			(m_cpu_to_mcu_empty ? 0x10 : 0x00) |
			(m_cpu_to_mcu_data & 0x0f);
}

void stfight_state::stfight_68705_port_b_w(uint8_t data)
{
	// Acknowledge Z80 command
	if (!BIT(data, 5))
		m_cpu_to_mcu_empty = true;
}

void stfight_state::stfight_68705_port_c_w(uint8_t data)
{
	// Signal a valid coin on the falling edge
	if (BIT(m_port_c_out, 0) && !BIT(data, 0))
		m_coin_state &= ~1;
	if (BIT(m_port_c_out, 1) && !BIT(data, 1))
		m_coin_state &= ~2;

	// Latch ADPCM data address when dropping the reset line
	m_adpcm_reset = BIT(data, 2);
	if (!m_adpcm_reset && BIT(m_port_c_out, 2))
		m_adpcm_data_offs = m_port_a_out << 9;
	m_msm->reset_w(m_adpcm_reset ? ASSERT_LINE : CLEAR_LINE);

	// Generate NMI on host CPU (used on handshake error or stuck coin)
	m_maincpu->set_input_line(INPUT_LINE_NMI, BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);

	m_port_c_out = data;
}
