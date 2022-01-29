// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/***************************************************************************

    cop452.h

    Frequency generator & counter

***************************************************************************/

#include "emu.h"
#include "cop452.h"

// Debugging
//#define VERBOSE LOG_GENERAL
#define VERBOSE 0
#include "logmacro.h"

// Device type definition
DEFINE_DEVICE_TYPE(COP452, cop452_device, "cop452", "National Semiconductor COP452 frequency generator")

cop452_device::cop452_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
: device_t(mconfig , COP452 , tag , owner , clock)
	, m_out_handlers(*this)
{
}

WRITE_LINE_MEMBER(cop452_device::cs_w)
{
	m_cs = state;
	if (m_cs) {
		// CS removed
		m_spi_state = 0;
		m_sr = 0;
	}
}

WRITE_LINE_MEMBER(cop452_device::sk_w)
{
	if (!m_cs && !m_sk && state) {
		// Rising edge on SK
		LOG("bit %d %u\n" , m_di , m_spi_state);
		if (m_spi_state == 0 && m_di) {
			// Got start bit
			m_spi_state = 1;
		} else if (m_spi_state >= 1 && m_spi_state < 6) {
			// Shifting instruction in
			m_sr = (m_sr << 1) | m_di;
			m_spi_state++;
			if (m_spi_state == 6) {
				LOG("Inst = %x\n" , m_sr);
				m_spi_state = 22;
				unsigned idx = !BIT(m_sr , 0);
				char reg = idx ? 'B' : 'A';
				switch (m_sr) {
				case 0b00000:
				case 0b00001:
					// LDRA/B
					LOG("LDR%c\n" , reg);
					m_spi_state = 6;
					m_reg[ idx ] = 0;
					break;

				case 0b00010:
				case 0b00011:
					// RDRA/B
					// TODO: not implemented ATM
					LOG("RDR%c\n" , reg);
					break;

				case 0b00100:
				case 0b00101:
					// TRCA/B
					LOG("TRC%c\n" , reg);
					m_cnt[ idx ] = m_reg[ idx ];
					set_timer(idx);
					break;

				case 0b00110:
				case 0b00111:
					// TCRA/B
					// TODO:
					LOG("TCR%c\n" , reg);
					break;

				case 0b01000:
					// CK1
					LOG("CK1\n");
					m_clk_div_4 = false;
					break;

				case 0b01001:
					// CK4
					LOG("CK4\n");
					m_clk_div_4 = true;
					break;

				default:
					if (m_sr & 0b10000) {
						// LDM
						m_mode = m_sr & 0b01111;
						LOG("LDM %x\n" , m_mode);
						set_timer(0);
						set_timer(1);
						if (m_mode == MODE_NUMBER_PULSES) {
							// Always start with OA = 1
							set_output(0 , true);
						} else if (m_mode == MODE_WHITE_NOISE ||
								   m_mode == MODE_GATED_WHITE) {
							// Preset bit 15 & 16 of register A when entering
							// white noise modes
							m_reg[ 0 ] |= 0x8000;
							m_regA_b16 = true;
						}
					} else {
						// Unknown instruction
						LOG("Unknown instruction\n");
					}
					break;
				}
			}
		} else if (m_spi_state >= 6 && m_spi_state < 22) {
			// Loading A/B register
			unsigned idx = !BIT(m_sr , 0);
			char reg = idx ? 'B' : 'A';
			m_reg[ idx ] = (m_reg[ idx ] << 1) | m_di;
			m_spi_state++;
			if (m_spi_state == 22) {
				LOG("REG%c = %04x\n" , reg , m_reg[ idx ]);
			}
		}
	}
	m_sk = state;
}

WRITE_LINE_MEMBER(cop452_device::di_w)
{
	m_di = state;
}

READ_LINE_MEMBER(cop452_device::do_r)
{
	// TODO:
	return 0;
}

void cop452_device::device_start()
{
	m_out_handlers.resolve_all_safe();

	m_timers[ 0 ] = timer_alloc(0);
	m_timers[ 1 ] = timer_alloc(1);

	save_item(NAME(m_mode));
	save_item(NAME(m_clk_div_4));
	save_item(NAME(m_cs));
	save_item(NAME(m_sk));
	save_item(NAME(m_di));
	save_item(NAME(m_out));
	save_item(NAME(m_regA_b16));
	save_item(NAME(m_reg));
	save_item(NAME(m_cnt));
	save_item(NAME(m_spi_state));
	save_item(NAME(m_sr));
}

void cop452_device::device_reset()
{
	// Set reset mode
	m_mode = MODE_RESET;
	m_clk_div_4 = true;
	m_out[ 0 ] = m_out[ 1 ] = true;
	set_output(0 , false);
	set_output(1 , false);
	m_spi_state = 0;
	m_sr = 0;
	m_timers[ 0 ]->reset();
	m_timers[ 1 ]->reset();
}

void cop452_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (m_mode) {
	case MODE_DUAL_FREQ:
		toggle_n_reload(id);
		break;

	case MODE_TRIG_PULSE:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_NUMBER_PULSES:
		if (id == 0) {
			toggle_n_reload(0);
			if (!m_out[ 0 ]) {
				// It seems that cnt B decrements each time OA goes low
				if (m_cnt[ 1 ] != 0) {
					m_cnt[ 1 ]--;
				} else {
					// End of pulse train
					toggle_n_reload(1);
					m_mode = MODE_RESET;
				}
			}
		}
		break;

	case MODE_DUTY_CYCLE:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_FREQ_COUNT:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_DUAL_COUNT:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_WAVE_MEAS:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_TRIG_COUNT:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_WHITE_NOISE:
	case MODE_GATED_WHITE:
		{
			if (id == 0) {
				// Reg A & its 17th bit (m_regA_b16) form a 17-bit LFSR
				// LFSR uses X^17+X^14+1 polynomial to generate a pseudo-random
				// maximal-length sequence
				bool feedback = m_regA_b16 ^ BIT(m_reg[ 0 ] , 13);
				m_regA_b16 = BIT(m_reg[ 0 ] , 15);
				m_reg[ 0 ] <<= 1;
				m_reg[ 0 ] |= feedback;
			} else {
				toggle_n_reload(1);
			}
			bool new_out_0 = m_regA_b16;
			if (m_mode == MODE_GATED_WHITE) {
				new_out_0 &= m_out[ 1 ];
			}
			set_output(0 , new_out_0);
		}
		break;

	default:
		break;
	}

	set_timer(id);
}

attotime cop452_device::counts_to_attotime(unsigned counts) const
{
	if (m_clk_div_4) {
		return clocks_to_attotime((counts + 1) * 4);
	} else {
		return clocks_to_attotime(counts + 1);
	}
}

void cop452_device::set_timer(unsigned idx)
{
	attotime target = attotime::never;

	switch (m_mode) {
	case MODE_DUAL_FREQ:
		// Cnt A & B count independently
		target = counts_to_attotime(m_cnt[ idx ]);
		break;

	case MODE_TRIG_PULSE:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_NUMBER_PULSES:
		// Cnt A generates OA frequency
		// Cnt B counts the periods to output
		if (idx == 0) {
			target = counts_to_attotime(m_cnt[ 0 ]);
		}
		break;

	case MODE_DUTY_CYCLE:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_FREQ_COUNT:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_DUAL_COUNT:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_WAVE_MEAS:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_TRIG_COUNT:
		// TODO: NOT IMPLEMENTED
		break;

	case MODE_WHITE_NOISE:
	case MODE_GATED_WHITE:
		// Cnt A is not used. Timer 0 expires once per internal clock period.
		// Cnt B generates squarewave signal on OB
		if (idx == 0) {
			target = counts_to_attotime(0);
		} else {
			target = counts_to_attotime(m_cnt[ 1 ]);
		}
		break;

	default:
		break;
	}

	m_timers[ idx ]->adjust(target);
}

void cop452_device::set_output(unsigned idx , bool state)
{
	if (m_out[ idx ] != state) {
		m_out[ idx ] = state;
		LOG("OUT %u=%d @%s\n" , idx , state , machine().time().as_string());
		m_out_handlers[ idx ](state);
	}
}

void cop452_device::toggle_output(unsigned idx)
{
	set_output(idx , !m_out[ idx ]);
}

void cop452_device::toggle_n_reload(unsigned idx)
{
	// Toggle output OA/OB and reload its associated counter
	toggle_output(idx);
	m_cnt[ idx ] = m_reg[ idx ];
}
