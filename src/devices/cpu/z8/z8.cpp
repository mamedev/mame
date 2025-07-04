// license:BSD-3-Clause
// copyright-holders:Curt Coder, AJR
/**********************************************************************

    Zilog Z8 Single-Chip MCU emulation

**********************************************************************/

/*

    TODO:

    - strobed I/O
    - instruction pipeline
    - internal diagnostic ROM in data space (requires high voltage reset)
    - what really happens when register pairs are unaligned?

    Note that A8–A15 outputs are not enabled on Port 0 upon reset, except
    on some later ROMless versions such as Z8691. This may redirect
    external memory accesses, including program fetches, to FFxx until
    P01M is written to. Z8681 is particularly affected by this.

*/

#include "emu.h"
#include "z8.h"
#include "z8dasm.h"

#define LOG_TIMER       (1U << 1)
#define LOG_RECEIVE     (1U << 2)
#define LOG_TRANSMIT    (1U << 3)
#define LOG_IRQ         (1U << 4)

#define VERBOSE 0
#include "logmacro.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define Z8_P3_DAV0                  0x04    /* not supported */
#define Z8_P3_DAV1                  0x08    /* not supported */
#define Z8_P3_DAV2                  0x02    /* not supported */
#define Z8_P3_RDY0                  0x20    /* not supported */
#define Z8_P3_RDY1                  0x10    /* not supported */
#define Z8_P3_RDY2                  0x40    /* not supported */
#define Z8_P3_IRQ0                  0x04
#define Z8_P3_IRQ1                  0x08
#define Z8_P3_IRQ2                  0x02
#define Z8_P3_IRQ3                  0x01
#define Z8_P3_SIN                   0x01
#define Z8_P3_SOUT                  0x80
#define Z8_P3_TIN                   0x02
#define Z8_P3_TOUT                  0x40
#define Z8_P3_DM                    0x10    /* not supported */

#define Z8_PRE0_COUNT_MODULO_N      0x01

#define Z8_PRE1_COUNT_MODULO_N      0x01
#define Z8_PRE1_INTERNAL_CLOCK      0x02

#define Z8_TMR_LOAD_T0              0x01
#define Z8_TMR_ENABLE_T0            0x02
#define Z8_TMR_LOAD_T1              0x04
#define Z8_TMR_ENABLE_T1            0x08
#define Z8_TMR_TIN_MASK             0x30
#define Z8_TMR_TIN_EXTERNAL_CLK     0x00
#define Z8_TMR_TIN_GATE             0x10
#define Z8_TMR_TIN_TRIGGER          0x20
#define Z8_TMR_TIN_RETRIGGER        0x30
#define Z8_TMR_TOUT_MASK            0xc0
#define Z8_TMR_TOUT_OFF             0x00
#define Z8_TMR_TOUT_T0              0x40
#define Z8_TMR_TOUT_T1              0x80
#define Z8_TMR_TOUT_INTERNAL_CLK    0xc0    /* not supported */

#define Z8_P01M_P0L_MODE_MASK       0x03
#define Z8_P01M_P0L_MODE_OUTPUT     0x00
#define Z8_P01M_P0L_MODE_INPUT      0x01
#define Z8_P01M_P0L_MODE_A8_A11     0x02
#define Z8_P01M_INTERNAL_STACK      0x04
#define Z8_P01M_P1_MODE_MASK        0x18
#define Z8_P01M_P1_MODE_OUTPUT      0x00
#define Z8_P01M_P1_MODE_INPUT       0x08
#define Z8_P01M_P1_MODE_AD0_AD7     0x10    /* not supported */
#define Z8_P01M_P1_MODE_HI_Z        0x18    /* not supported */
#define Z8_P01M_EXTENDED_TIMING     0x20    /* not supported */
#define Z8_P01M_P0H_MODE_MASK       0xc0
#define Z8_P01M_P0H_MODE_OUTPUT     0x00
#define Z8_P01M_P0H_MODE_INPUT      0x40
#define Z8_P01M_P0H_MODE_A12_A15    0x80

#define Z8_P3M_P2_ACTIVE_PULLUPS    0x01
#define Z8_P3M_P0_STROBED           0x04    /* not supported */
#define Z8_P3M_P33_P34_MASK         0x18
#define Z8_P3M_P33_P34_INPUT_OUTPUT 0x00
#define Z8_P3M_P33_P34_INPUT_DM     0x08    /* not supported */
#define Z8_P3M_P33_P34_INPUT_DM_2   0x10    /* not supported */
#define Z8_P3M_P33_P34_DAV1_RDY1    0x18    /* not supported */
#define Z8_P3M_P2_STROBED           0x20    /* not supported */
#define Z8_P3M_P3_SERIAL            0x40    /* not supported */
#define Z8_P3M_PARITY               0x80    /* not supported */

#define Z8_IMR_ENABLE               0x80
#define Z8_IMR_RAM_PROTECT          0x40    /* not supported */

#define Z8_IRQ_MASK                 0x3f

#define Z8_FLAGS_F1                 0x01
#define Z8_FLAGS_F2                 0x02
#define Z8_FLAGS_H                  0x04
#define Z8_FLAGS_D                  0x08
#define Z8_FLAGS_V                  0x10
#define Z8_FLAGS_S                  0x20
#define Z8_FLAGS_Z                  0x40
#define Z8_FLAGS_C                  0x80

enum
{
	CC_F = 0, CC_LT, CC_LE, CC_ULE, CC_OV, CC_MI, CC_Z, CC_C,
	CC_T, CC_GE, CC_GT, CC_UGT, CC_NOV, CC_PL, CC_NZ, CC_NC
};

/***************************************************************************
    MACROS
***************************************************************************/

DEFINE_DEVICE_TYPE(Z8601,   z8601_device,   "z8601",   "Zilog Z8601")
DEFINE_DEVICE_TYPE(UB8830D, ub8830d_device, "ub8830d", "UB8830D")
DEFINE_DEVICE_TYPE(Z8611,   z8611_device,   "z8611",   "Zilog Z8611")
DEFINE_DEVICE_TYPE(Z8671,   z8671_device,   "z8671",   "Zilog Z8671")
DEFINE_DEVICE_TYPE(Z8681,   z8681_device,   "z8681",   "Zilog Z8681")
DEFINE_DEVICE_TYPE(Z8682,   z8682_device,   "z8682",   "Zilog Z8682")
DEFINE_DEVICE_TYPE(Z86E02,  z86e02_device,  "z86e02",  "Zilog Z86E02")


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void z8_device::program_map(address_map &map)
{
	if (m_rom_size > 0)
		map(0x0000, m_rom_size - 1).rom().region(DEVICE_SELF, 0);
}

void z8_device::preprogrammed_map(address_map &map)
{
	map(0x0000, m_rom_size - 1).rom().region("internal", 0);
}

void z8_device::register_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).rw(FUNC(z8_device::p0_read), FUNC(z8_device::p0_write));
	map(0x01, 0x01).rw(FUNC(z8_device::p1_read), FUNC(z8_device::p1_write));
	map(0x02, 0x02).rw(FUNC(z8_device::p2_read), FUNC(z8_device::p2_write));
	map(0x03, 0x03).rw(FUNC(z8_device::p3_read), FUNC(z8_device::p3_write));
	map(0x04, 0x7f).ram();
	map(0xf0, 0xf0).rw(FUNC(z8_device::sio_read), FUNC(z8_device::sio_write));
	map(0xf1, 0xf1).rw(FUNC(z8_device::tmr_read), FUNC(z8_device::tmr_write));
	map(0xf2, 0xf2).rw(FUNC(z8_device::t1_read), FUNC(z8_device::t1_write));
	map(0xf3, 0xf3).w(FUNC(z8_device::pre1_write));
	map(0xf4, 0xf4).rw(FUNC(z8_device::t0_read), FUNC(z8_device::t0_write));
	map(0xf5, 0xf5).w(FUNC(z8_device::pre0_write));
	map(0xf6, 0xf6).w(FUNC(z8_device::p2m_write));
	map(0xf7, 0xf7).w(FUNC(z8_device::p3m_write));
	map(0xf8, 0xf8).w(FUNC(z8_device::p01m_write));
	map(0xf9, 0xf9).w(FUNC(z8_device::ipr_write));
	map(0xfa, 0xfa).rw(FUNC(z8_device::irq_read), FUNC(z8_device::irq_write));
	map(0xfb, 0xfb).rw(FUNC(z8_device::imr_read), FUNC(z8_device::imr_write));
	map(0xfc, 0xfc).rw(FUNC(z8_device::flags_read), FUNC(z8_device::flags_write));
	map(0xfd, 0xfd).rw(FUNC(z8_device::rp_read), FUNC(z8_device::rp_write));
	map(0xfe, 0xfe).rw(FUNC(z8_device::sph_read), FUNC(z8_device::sph_write));
	map(0xff, 0xff).rw(FUNC(z8_device::spl_read), FUNC(z8_device::spl_write));
}


z8_device::z8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t rom_size, bool preprogrammed)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, preprogrammed ? address_map_constructor(FUNC(z8_device::preprogrammed_map), this) : address_map_constructor(FUNC(z8_device::program_map), this))
	, m_data_config("data", ENDIANNESS_BIG, 8, 16, 0)
	, m_register_config("register", ENDIANNESS_BIG, 8, 8, 0, address_map_constructor(FUNC(z8_device::register_map), this))
	, m_input_cb(*this, 0xff)
	, m_output_cb(*this)
	, m_rom_size(rom_size)
	, m_input{0xff, 0xff, 0xff, 0x0f}
{
	assert(((rom_size - 1) & rom_size) == 0);
}


z8601_device::z8601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8601, tag, owner, clock, 0x800, false)
{
}


ub8830d_device::ub8830d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, UB8830D, tag, owner, clock, 0x800, false)
{
}


z8611_device::z8611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8611, tag, owner, clock, 0x1000, false)
{
}


z8671_device::z8671_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8671, tag, owner, clock, 0x800, true)
{
}

ROM_START(z8671)
	ROM_REGION(0x0800, "internal", 0)
	ROM_LOAD("z8671.bin", 0x0000, 0x0800, CRC(3fceeb76) SHA1(290a24c77debd2e280fe31380287838c5fb7cabd))
ROM_END

const tiny_rom_entry *z8671_device::device_rom_region() const
{
	return ROM_NAME(z8671);
}


z8681_device::z8681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8681, tag, owner, clock, 0, false)
{
}


z8682_device::z8682_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8682, tag, owner, clock, 0x800, true)
{
}

ROM_START(z8682)
	// Zilog admits that this nominally ROMless type uses a "small internal ROM"
	ROM_REGION(0x0800, "internal", ROMREGION_ERASEFF)
	ROM_LOAD("testrom.bin", 0x0000, 0x0038, CRC(b2239f28) SHA1(9d27957ba0f15657eac5a7295157af6ee51cb261) BAD_DUMP) // typed in from "Z8 MCU Test Mode" application note
ROM_END

const tiny_rom_entry *z8682_device::device_rom_region() const
{
	return ROM_NAME(z8682);
}


z86e02_device::z86e02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z86E02, tag, owner, clock, 0x200, false)
{
}


std::unique_ptr<util::disasm_interface> z8_device::create_disassembler()
{
	return std::make_unique<z8_disassembler>();
}


device_memory_interface::space_config_vector z8_device::memory_space_config() const
{
	// Separate data space is optional
	if (has_configured_map(AS_DATA))
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_DATA,    &m_data_config),
			std::make_pair(AS_IO,      &m_register_config)
		};
	}
	else
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_register_config)
		};
	}
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

uint16_t z8_device::mask_external_address(uint16_t addr)
{
	switch (m_p01m & (Z8_P01M_P0L_MODE_A8_A11 | Z8_P01M_P0H_MODE_A12_A15))
	{
		case 0:
			addr = (addr & 0x00ff) | p0_read() << 8;
			break;

		case Z8_P01M_P0L_MODE_A8_A11:
			addr = (addr & 0x0fff) | (p0_read() & 0xf0) << 8;
			break;

		case Z8_P01M_P0H_MODE_A12_A15:
			addr = (addr & 0xf0ff) | (p0_read() & 0x0f) << 8;
			break;
	}
	return addr;
}


uint8_t z8_device::fetch()
{
	uint16_t real_pc = (m_pc < m_rom_size) ? m_pc : mask_external_address(m_pc);
	uint8_t data = m_cache.read_byte(real_pc);

	m_pc++;

	return data;
}


uint8_t z8_device::fetch_opcode()
{
	m_ppc = (m_pc < m_rom_size) ? m_pc : mask_external_address(m_pc);
	debugger_instruction_hook(m_ppc);

	uint8_t data = m_cache.read_byte(m_ppc);

	m_pc++;

	return data;
}


uint16_t z8_device::fetch_word()
{
	// ensure correct order of operations by using separate instructions
	uint16_t data = fetch() << 8;
	data |= fetch();

	return data;
}


uint8_t z8_device::p0_read()
{
	uint8_t data = 0;
	uint8_t mask = 0;

	switch (m_p01m & Z8_P01M_P0L_MODE_MASK)
	{
	case Z8_P01M_P0L_MODE_OUTPUT:
		data = m_output[0] & 0x0f;
		break;
	case Z8_P01M_P0L_MODE_INPUT:
		mask = 0x0f;
		break;
	default: /* A8...A11 */
		data = 0x0f;
		break;
	}

	switch (m_p01m & Z8_P01M_P0H_MODE_MASK)
	{
	case Z8_P01M_P0H_MODE_OUTPUT:
		data |= m_output[0] & 0xf0;
		break;
	case Z8_P01M_P0H_MODE_INPUT:
		mask |= 0xf0;
		break;
	default: /* A12...A15 */
		data |= 0xf0;
		break;
	}

	if (!(m_p3m & Z8_P3M_P0_STROBED))
	{
		if (mask)
			m_input[0] = m_input_cb[0](0, mask);
	}

	data |= m_input[0] & mask;
	return data;
}

void z8_device::p0_write(uint8_t data)
{
	uint8_t mask = 0;

	m_output[0] = data;
	if ((m_p01m & Z8_P01M_P0L_MODE_MASK) == Z8_P01M_P0L_MODE_OUTPUT)
		mask |= 0x0f;
	if ((m_p01m & Z8_P01M_P0H_MODE_MASK) == Z8_P01M_P0H_MODE_OUTPUT)
		mask |= 0xf0;
	if (mask)
		m_output_cb[0](0, data & mask, mask);
}

uint8_t z8_device::p1_read()
{
	uint8_t data = 0;
	uint8_t mask = 0;

	switch (m_p01m & Z8_P01M_P1_MODE_MASK)
	{
	case Z8_P01M_P1_MODE_OUTPUT:
		data = m_output[1];
		break;
	case Z8_P01M_P1_MODE_INPUT:
		mask = 0xff;
		break;
	default: /* AD0..AD7 */
		data = 0xff;
		break;
	}

	if ((m_p3m & Z8_P3M_P33_P34_MASK) != Z8_P3M_P33_P34_DAV1_RDY1)
	{
		if (mask)
			m_input[1] = m_input_cb[1](0, mask);
	}

	data |= m_input[1] & mask;
	return data;
}

void z8_device::p1_write(uint8_t data)
{
	uint8_t mask = 0;

	m_output[1] = data;
	if ((m_p01m & Z8_P01M_P1_MODE_MASK) == Z8_P01M_P1_MODE_OUTPUT)
		mask = 0xff;
	if (mask)
		m_output_cb[1](0, data & mask, mask);
}

uint8_t z8_device::p2_read()
{
	uint8_t mask = m_p2m;

	// open drain lines can be externally driven where data = 1
	if (!(m_p3m & Z8_P3M_P2_ACTIVE_PULLUPS))
		mask |= m_output[2];

	if (!(m_p3m & Z8_P3M_P2_STROBED))
	{
		if (mask)
			m_input[2] = m_input_cb[2](0, mask);
	}

	return (m_input[2] & mask) | (m_output[2] & ~mask);
}

void z8_device::p2_write(uint8_t data)
{
	uint8_t mask = m_p2m ^ 0xff;

	m_output[2] = data;
	if (mask)
		m_output_cb[2](0, data & mask, mask);
}

void z8_device::p3_update_output()
{
	uint8_t output = m_output[3] & 0xf0;

	if ((m_tmr & Z8_TMR_TOUT_MASK) != Z8_TMR_TOUT_OFF)
		output = (output & ~Z8_P3_TOUT) | (m_tout ? Z8_P3_TOUT : 0);
	if ((m_p3m & Z8_P3M_P3_SERIAL) != 0)
		output = (output & ~Z8_P3_SOUT) | ((m_transmit_sr == 0 || BIT(m_transmit_sr, 0)) ? Z8_P3_SOUT : 0);

	if (m_p3_output != output)
	{
		m_output_cb[3](0, output, output ^ m_p3_output);
		m_p3_output = output;
	}
}

uint8_t z8_device::p3_read()
{
	uint8_t mask = 0x0f;
	uint8_t inputs = m_input[3] & m_input_cb[3](0, mask);

	// TODO: special port 3 modes
	//if (!(m_p3m & 0x7c))
	//{
	//}

	return (inputs & mask) | (m_p3_output & ~mask);
}

void z8_device::p3_write(uint8_t data)
{
	m_output[3] = data & 0xf0;

	// TODO: special port 3 modes
	//if (!(m_p3m & 0x7c))
	//{
	//}

	p3_update_output();
}

bool z8_device::get_serial_in()
{
	return (m_input[3] & Z8_P3_SIN) != 0;
}

void z8_device::sio_receive()
{
	if (m_receive_started)
	{
		m_receive_count = (m_receive_count + 1) & 15;
		if (m_receive_count == 8)
		{
			if (m_receive_sr == 0)
			{
				if (!get_serial_in())
				{
					// start bit validated
					m_receive_sr |= 1 << 9;
					m_receive_parity = false;
					LOGMASKED(LOG_RECEIVE, "Start bit validated\n");
				}
				else
				{
					// false start bit
					m_receive_started = false;
					m_receive_count = 0;
				}
			}
			else
			{
				// shift in data, parity or stop bit
				m_receive_sr >>= 1;
				if (get_serial_in())
					m_receive_sr |= 1 << 9;

				if (BIT(m_receive_sr, 0))
				{
					// received full character
					m_receive_buffer = (m_receive_sr & 0x1fe) >> 1;
					request_interrupt(3);
					m_receive_started = false;
					m_receive_count = 0;
					LOGMASKED(LOG_RECEIVE, "Character received: %02X\n", m_receive_buffer);
				}
				else
				{
					if (BIT(m_receive_sr, 9))
						m_receive_parity = !m_receive_parity;

					// parity replaces received bit 7 if selected
					if (BIT(m_receive_sr, 1) && (m_p3m & Z8_P3M_PARITY) != 0)
					{
						LOGMASKED(LOG_RECEIVE, "%d parity bit shifted in\n", BIT(m_receive_sr, 9));
						if (m_receive_parity)
							m_receive_sr |= 1 << 9;
						else
							m_receive_sr &= ~(1 << 9);
					}
					else
						LOGMASKED(LOG_RECEIVE, "%d data bit shifted in\n", BIT(m_receive_sr, 9));
				}
			}
		}
	}
	else
	{
		// start bit is high-low transition
		m_receive_sr >>= 1;
		if (get_serial_in())
			m_receive_sr |= 1 << 9;
		else if (BIT(m_receive_sr, 8))
		{
			LOGMASKED(LOG_RECEIVE, "Start bit noticed\n");
			m_receive_started = true;
			m_receive_sr = 0;
			m_receive_count = 0;
		}
	}
}

void z8_device::sio_transmit()
{
	if (m_transmit_sr == 0)
		return;

	m_transmit_count = (m_transmit_count + 1) & 15;
	if (m_transmit_count == 0)
	{
		m_transmit_sr >>= 1;
		if (m_transmit_sr == 0)
		{
			LOGMASKED(LOG_TRANSMIT, "Transmit register empty\n");
			request_interrupt(4);
		}
		else
		{
			// parity replaces received bit 7 if selected
			if ((m_transmit_sr >> 1) == 3 && (m_p3m & Z8_P3M_PARITY) != 0)
			{
				if (m_transmit_parity)
					m_transmit_sr |= 1;
				else
					m_transmit_sr &= ~1;
				LOGMASKED(LOG_TRANSMIT, "%d parity bit shifted out\n", BIT(m_transmit_sr, 0));
			}
			else
			{
				LOGMASKED(LOG_TRANSMIT, "%d %s bit shifted out\n", BIT(m_transmit_sr, 0),
						BIT(m_transmit_sr, 10) ? "start" : m_transmit_sr > 3 ? "data" : "stop");
				if (BIT(m_transmit_sr, 0))
					m_transmit_parity = !m_transmit_parity;
			}

			// serial output
			p3_update_output();
		}
	}
}

uint8_t z8_device::sio_read()
{
	return m_receive_buffer;
}

void z8_device::sio_write(uint8_t data)
{
	LOGMASKED(LOG_TRANSMIT, "(%04X): Character to transmit: %02X\n", m_ppc, data);

	// overwrite shift register with data + 1 start bit + 2 stop bits
	m_transmit_sr = (m_transmit_sr & 1) | (uint16_t(data) << 2) | (3 << 10);
	m_transmit_parity = false;

	// synchronize the shift clock
	m_transmit_count = 15;
}

template <int T>
void z8_device::timer_start()
{
	unsigned prescaler = (m_pre[T] >> 2) ? (m_pre[T] >> 2) : 64;
	unsigned full_count = (m_count[T] ? m_count[T] - 1 : 255) * prescaler + (m_pre_count[T] ? m_pre_count[T] : 64);
	m_internal_timer[T]->adjust(cycles_to_attotime(4 * full_count));
}

template <int T>
void z8_device::timer_stop()
{
	if (!m_internal_timer[T]->enabled())
		return;

	unsigned prescaler = (m_pre[T] >> 2) ? (m_pre[T] >> 2) : 64;
	unsigned remaining = attotime_to_cycles(m_internal_timer[T]->remaining() / 4);

	m_count[T] = remaining / prescaler + 1;
	m_pre_count[T] = (remaining % prescaler + 1) & 0x3f;

	m_internal_timer[T]->enable(false);
}

template <int T>
void z8_device::timer_end()
{
	if ((m_tmr & Z8_TMR_TOUT_MASK) == (T == 0 ? Z8_TMR_TOUT_T0 : Z8_TMR_TOUT_T1))
		tout_toggle();

	if (T == 0 && (m_p3m & Z8_P3M_P3_SERIAL) != 0)
	{
		sio_receive();
		sio_transmit();
	}
	else
		request_interrupt(4 + T);

	m_pre_count[T] = m_pre[T] >> 2;
	if (m_pre[T] & Z8_PRE0_COUNT_MODULO_N)
		m_count[T] = m_t[T];
	else
		m_tmr &= ~(T == 0 ? Z8_TMR_ENABLE_T0 : Z8_TMR_ENABLE_T1);
}

void z8_device::t1_trigger()
{
	switch (m_tmr & Z8_TMR_TIN_MASK)
	{
	case Z8_TMR_TIN_EXTERNAL_CLK:
		m_pre_count[1]--;
		if (m_pre_count[1] == 0)
		{
			m_pre_count[1] = m_pre[1];
			if ((m_tmr & Z8_TMR_ENABLE_T1) != 0)
			{
				m_count[1]--;
				if (m_count[1] == 0)
					timer_end<1>();
			}
		}
		break;

	case Z8_TMR_TIN_GATE:
		timer_stop<1>();
		break;

	case Z8_TMR_TIN_TRIGGER:
		if (m_internal_timer[1]->enabled())
			break;
		[[fallthrough]];
	case Z8_TMR_TIN_RETRIGGER:
		if ((m_tmr & Z8_TMR_ENABLE_T1) != 0)
		{
			m_count[1] = m_t[1];
			m_pre_count[1] = m_pre[1] >> 2;
			timer_start<1>();
		}
		break;
	}
}

void z8_device::tout_init()
{
	m_tout = true;
	p3_update_output();
}

void z8_device::tout_toggle()
{
	m_tout = !m_tout;
	p3_update_output();
}

uint8_t z8_device::tmr_read()
{
	return m_tmr;
}

void z8_device::tmr_write(uint8_t data)
{
	m_tmr = data & ~(Z8_TMR_LOAD_T0 | Z8_TMR_LOAD_T1); // actually reset on next internal clock

	bool t1_internal = (m_pre[1] & Z8_PRE1_INTERNAL_CLOCK) != 0;
	bool t0_load = (data & Z8_TMR_LOAD_T0) != 0;
	bool t1_load = (data & Z8_TMR_LOAD_T1) != 0;
	bool t0_enable = (data & Z8_TMR_ENABLE_T0) != 0;
	bool t1_enable = (data & Z8_TMR_ENABLE_T1) != 0;

	if (!t1_internal && ((data & Z8_TMR_TIN_MASK) == Z8_TMR_TIN_GATE))
	{
		if ((m_input[3] & Z8_P3_TIN) != 0)
			t1_internal = true;
		else
			t1_enable = false;
	}

	if (t0_load)
	{
		m_count[0] = m_t[0];
		m_pre_count[0] = m_pre[0] >> 2;

		if ((m_pre[0] & Z8_PRE0_COUNT_MODULO_N) != 0)
		{
			unsigned prescaler = (m_pre[0] >> 2) ? (m_pre[0] >> 2) : 64;
			unsigned count = (m_t[0] ? m_t[0] : 256) * prescaler;
			LOGMASKED(LOG_TIMER, "(%04X): Load T0 at %.2f Hz\n", m_ppc, clock() / 8.0 / count);
		}

		if ((data & Z8_TMR_TOUT_MASK) == Z8_TMR_TOUT_T0)
			tout_init();
	}

	if (t0_enable)
	{
		if (t0_load || !m_internal_timer[0]->enabled())
			timer_start<0>();
	}
	else
		timer_stop<0>();

	if (t1_load)
	{
		m_count[1] = m_t[1];
		m_pre_count[1] = m_pre[1] >> 2;

		if (t1_internal && (m_pre[1] & Z8_PRE0_COUNT_MODULO_N) != 0)
		{
			unsigned prescaler = (m_pre[1] >> 2) ? (m_pre[1] >> 2) : 64;
			unsigned count = (m_t[1] ? m_t[1] : 256) * prescaler;
			LOGMASKED(LOG_TIMER, "(%04X): Load T1 at %.2f Hz\n", m_ppc, clock() / 8.0 / count);
		}

		if ((data & Z8_TMR_TOUT_MASK) == Z8_TMR_TOUT_T1)
			tout_init();
	}

	if (t1_enable)
	{
		if (t1_internal && (t1_load || !m_internal_timer[1]->enabled()))
			timer_start<1>();
	}
	else
		timer_stop<1>();
}

uint8_t z8_device::t0_read()
{
	if (!m_internal_timer[0]->enabled())
		return m_count[0];

	unsigned prescaler = (m_pre[0] >> 2) ? (m_pre[0] >> 2) : 64;
	unsigned remaining = attotime_to_cycles(m_internal_timer[0]->remaining() / 4);

	return remaining / prescaler + 1;
}

void z8_device::t0_write(uint8_t data)
{
	m_t[0] = data;
}

uint8_t z8_device::t1_read()
{
	if (!m_internal_timer[1]->enabled())
		return m_count[1];

	unsigned prescaler = (m_pre[1] >> 2) ? (m_pre[1] >> 2) : 64;
	unsigned remaining = attotime_to_cycles(m_internal_timer[1]->remaining() / 4);

	return remaining / prescaler + 1;
}

void z8_device::t1_write(uint8_t data)
{
	m_t[1] = data;
}

void z8_device::pre0_write(uint8_t data)
{
	if (m_internal_timer[0]->enabled())
	{
		timer_stop<0>();
		m_pre[0] = data;
		timer_start<0>();
	}
	else
		m_pre[0] = data;
}

void z8_device::pre1_write(uint8_t data)
{
	bool was_enabled = m_internal_timer[1]->enabled();
	if (was_enabled)
		timer_stop<1>();

	m_pre[1] = data;

	if ((data & Z8_PRE1_INTERNAL_CLOCK) != 0
		? (m_tmr & Z8_TMR_ENABLE_T1) != 0
		: was_enabled && (m_tmr & Z8_TMR_TIN_MASK) != Z8_TMR_TIN_EXTERNAL_CLK)
		timer_start<1>();
}

void z8_device::p01m_write(uint8_t data)
{
	m_p01m = data;
}

void z8_device::p2m_write(uint8_t data)
{
	m_p2m = data;
}

void z8_device::p3m_write(uint8_t data)
{
	if ((data & Z8_P3M_P3_SERIAL) == 0)
	{
		m_transmit_sr = 0;
		m_transmit_count = 0;
		m_receive_started = false;
		m_receive_count = 0;
	}

	m_p3m = data;
	p3_update_output();
}

void z8_device::ipr_write(uint8_t data)
{
	m_ipr = data;
}

uint8_t z8_device::irq_read()
{
	return m_irq;
}

void z8_device::irq_write(uint8_t data)
{
	if (m_irq_initialized)
		m_irq = data;
}

uint8_t z8_device::imr_read()
{
	return m_imr;
}

void z8_device::imr_write(uint8_t data)
{
	m_imr = data;
}

uint8_t z8_device::flags_read()
{
	return m_flags;
}

void z8_device::flags_write(uint8_t data)
{
	m_flags = data;
}

uint8_t z8_device::rp_read()
{
	return m_rp;
}

void z8_device::rp_write(uint8_t data)
{
	m_rp = data;
}

uint8_t z8_device::sph_read()
{
	return m_sp.b.h;
}

void z8_device::sph_write(uint8_t data)
{
	m_sp.b.h = data;
}

uint8_t z8_device::spl_read()
{
	return m_sp.b.l;
}

void z8_device::spl_write(uint8_t data)
{
	m_sp.b.l = data;
}


uint16_t z8_device::register_pair_read(uint8_t offset)
{
	return m_regs.read_word_unaligned(offset);
}

void z8_device::register_pair_write(uint8_t offset, uint16_t data)
{
	m_regs.write_word_unaligned(offset, data);
}

uint8_t z8_device::get_working_register(int offset) const
{
	return (m_rp & 0xf0) | (offset & 0x0f);
}

uint8_t z8_device::get_register(uint8_t offset) const
{
	if ((offset & 0xf0) == 0xe0)
		return get_working_register(offset & 0x0f);
	else
		return offset;
}

uint8_t z8_device::get_intermediate_register(int offset)
{
	return register_read(get_register(offset));
}

void z8_device::stack_push_byte(uint8_t src)
{
	if (m_p01m & Z8_P01M_INTERNAL_STACK)
	{
		// SP <- SP - 1 (predecrement)
		uint8_t sp = m_sp.b.l - 1;
		m_sp.b.l = sp;

		// @SP <- src
		register_write(sp, src);
	}
	else
	{
		// SP <- SP - 1 (predecrement)
		uint16_t sp = m_sp.w - 1;
		m_sp.w = sp;

		// @SP <- src
		m_data.write_byte(mask_external_address(sp), src);
	}
}

void z8_device::stack_push_word(uint16_t src)
{
	if (m_p01m & Z8_P01M_INTERNAL_STACK)
	{
		// SP <- SP - 2 (predecrement)
		uint8_t sp = m_sp.b.l - 2;
		m_sp.b.l = sp;

		// @SP <- src
		register_pair_write(sp, src);
	}
	else
	{
		// SP <- SP - 2 (predecrement)
		uint16_t sp = m_sp.w - 2;
		m_sp.w = sp;

		// @SP <- src
		m_data.write_word_unaligned(mask_external_address(sp), src);
	}
}

uint8_t z8_device::stack_pop_byte()
{
	if (m_p01m & Z8_P01M_INTERNAL_STACK)
	{
		// @SP <- src
		uint8_t sp = m_sp.b.l;
		uint8_t byte = register_read(sp);

		// SP <- SP + 1 (postincrement)
		m_sp.b.l = sp + 1;

		return byte;
	}
	else
	{
		// @SP <- src
		uint16_t sp = m_sp.w;
		uint8_t byte = m_data.read_byte(mask_external_address(sp));

		// SP <- SP + 1 (postincrement)
		m_sp.w = sp + 1;

		return byte;
	}
}

uint16_t z8_device::stack_pop_word()
{
	if (m_p01m & Z8_P01M_INTERNAL_STACK)
	{
		// @SP <- src
		uint8_t sp = m_sp.b.l;
		uint16_t word = register_pair_read(sp);

		// SP <- SP + 2 (postincrement)
		m_sp.b.l = sp + 2;

		return word;
	}
	else
	{
		// @SP <- src
		uint16_t sp = m_sp.w;
		uint16_t word = m_data.read_word_unaligned(mask_external_address(sp));

		// SP <- SP + 2 (postincrement)
		m_sp.w = sp + 2;

		return word;
	}
}

void z8_device::set_flag(uint8_t flag, int state)
{
	if (state)
		m_flags |= flag;
	else
		m_flags &= ~flag;
}

#define set_flag_h(state)   set_flag(Z8_FLAGS_H, state);
#define set_flag_d(state)   set_flag(Z8_FLAGS_D, state);
#define set_flag_v(state)   set_flag(Z8_FLAGS_V, state);
#define set_flag_s(state)   set_flag(Z8_FLAGS_S, state);
#define set_flag_z(state)   set_flag(Z8_FLAGS_Z, state);
#define set_flag_c(state)   set_flag(Z8_FLAGS_C, state);

/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

#define INSTRUCTION(mnemonic) void z8_device::mnemonic(uint8_t opcode, int *cycles)

INSTRUCTION( illegal )
{
	logerror("Z8: PC = %04x, Illegal opcode = %02x\n", m_pc - 1, opcode);
}

#include "z8ops.hxx"

/***************************************************************************
    OPCODE TABLES
***************************************************************************/

const z8_device::z8_opcode_map z8_device::Z8601_OPCODE_MAP[256] =
{
	{ &z8_device::dec_R1, 6, 5 },   { &z8_device::dec_IR1, 6, 5 },  { &z8_device::add_r1_r2, 10, 5 },   { &z8_device::add_r1_Ir2, 10, 5 },
	{ &z8_device::add_R2_R1, 10, 5 },   { &z8_device::add_IR2_R1, 10, 5 },  { &z8_device::add_R1_IM, 10, 5 },   { &z8_device::add_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::rlc_R1, 6, 5 },   { &z8_device::rlc_IR1, 6, 5 },  { &z8_device::adc_r1_r2, 6, 5 },    { &z8_device::adc_r1_Ir2, 6, 5 },
	{ &z8_device::adc_R2_R1, 10, 5 },   { &z8_device::adc_IR2_R1, 10, 5 },  { &z8_device::adc_R1_IM, 10, 5 },   { &z8_device::adc_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::inc_R1, 6, 5 },   { &z8_device::inc_IR1, 6, 5 },  { &z8_device::sub_r1_r2, 6, 5 },    { &z8_device::sub_r1_Ir2, 6, 5 },
	{ &z8_device::sub_R2_R1, 10, 5 },   { &z8_device::sub_IR2_R1, 10, 5 },  { &z8_device::sub_R1_IM, 10, 5 },   { &z8_device::sub_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::jp_IRR1, 8, 0 },  { &z8_device::srp_IM, 6, 1 },   { &z8_device::sbc_r1_r2, 6, 5 },    { &z8_device::sbc_r1_Ir2, 6, 5 },
	{ &z8_device::sbc_R2_R1, 10, 5 },   { &z8_device::sbc_IR2_R1, 10, 5 },  { &z8_device::sbc_R1_IM, 10, 5 },   { &z8_device::sbc_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::da_R1, 8, 5 },    { &z8_device::da_IR1, 8, 5 },   { &z8_device::or_r1_r2, 6, 5 },     { &z8_device::or_r1_Ir2, 6, 5 },
	{ &z8_device::or_R2_R1, 10, 5 },    { &z8_device::or_IR2_R1, 10, 5 },   { &z8_device::or_R1_IM, 10, 5 },    { &z8_device::or_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::pop_R1, 10, 5 },  { &z8_device::pop_IR1, 10, 5 }, { &z8_device::and_r1_r2, 6, 5 },    { &z8_device::and_r1_Ir2, 6, 5 },
	{ &z8_device::and_R2_R1, 10, 5 },   { &z8_device::and_IR2_R1, 10, 5 },  { &z8_device::and_R1_IM, 10, 5 },   { &z8_device::and_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::com_R1, 6, 5 },   { &z8_device::com_IR1, 6, 5 },  { &z8_device::tcm_r1_r2, 6, 5 },    { &z8_device::tcm_r1_Ir2, 6, 5 },
	{ &z8_device::tcm_R2_R1, 10, 5 },   { &z8_device::tcm_IR2_R1, 10, 5 },  { &z8_device::tcm_R1_IM, 10, 5 },   { &z8_device::tcm_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::push_R2, 10, 1 }, { &z8_device::push_IR2, 12, 1 },{ &z8_device::tm_r1_r2, 6, 5 },     { &z8_device::tm_r1_Ir2, 6, 5 },
	{ &z8_device::tm_R2_R1, 10, 5 },    { &z8_device::tm_IR2_R1, 10, 5 },   { &z8_device::tm_R1_IM, 10, 5 },    { &z8_device::tm_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::decw_RR1, 10, 5 },{ &z8_device::decw_IR1, 10, 5 },{ &z8_device::lde_r1_Irr2, 12, 0 }, { &z8_device::ldei_Ir1_Irr2, 18, 0 },
	{ &z8_device::illegal, 0, 0 },     { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::di, 6, 1 },

	{ &z8_device::rl_R1, 6, 5 },    { &z8_device::rl_IR1, 6, 5 },   { &z8_device::lde_r2_Irr1, 12, 0 }, { &z8_device::ldei_Ir2_Irr1, 18, 0 },
	{ &z8_device::illegal, 0, 0 },     { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::ei, 6, 1 },

	{ &z8_device::incw_RR1, 10, 5 },{ &z8_device::incw_IR1, 10, 5 },{ &z8_device::cp_r1_r2, 6, 5 },     { &z8_device::cp_r1_Ir2, 6, 5 },
	{ &z8_device::cp_R2_R1, 10, 5 },    { &z8_device::cp_IR2_R1, 10, 5 },   { &z8_device::cp_R1_IM, 10, 5 },    { &z8_device::cp_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::ret, 14, 0 },

	{ &z8_device::clr_R1, 6, 5 },   { &z8_device::clr_IR1, 6, 5 },  { &z8_device::xor_r1_r2, 6, 5 },    { &z8_device::xor_r1_Ir2, 6, 5 },
	{ &z8_device::xor_R2_R1, 10, 5 },   { &z8_device::xor_IR2_R1, 10, 5 },  { &z8_device::xor_R1_IM, 10, 5 },   { &z8_device::xor_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::iret, 16, 0 },

	{ &z8_device::rrc_R1, 6, 5 },   { &z8_device::rrc_IR1, 6, 5 },  { &z8_device::ldc_r1_Irr2, 12, 0 }, { &z8_device::ldci_Ir1_Irr2, 18, 0 },
	{ &z8_device::illegal, 0, 0 },     { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },      { &z8_device::ld_r1_x_R2, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::rcf, 6, 5 },

	{ &z8_device::sra_R1, 6, 5 },   { &z8_device::sra_IR1, 6, 5 },  { &z8_device::ldc_r2_Irr1, 12, 0 }, { &z8_device::ldci_Ir2_Irr1, 18, 0 },
	{ &z8_device::call_IRR1, 20, 0 },  { &z8_device::illegal, 0, 0 },      { &z8_device::call_DA, 20, 0 },     { &z8_device::ld_r2_x_R1, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::scf, 6, 5 },

	{ &z8_device::rr_R1, 6, 5 },    { &z8_device::rr_IR1, 6, 5 },   { &z8_device::illegal, 0, 0 },      { &z8_device::ld_r1_Ir2, 6, 5 },
	{ &z8_device::ld_R2_R1, 10, 5 },    { &z8_device::ld_IR2_R1, 10, 5 },   { &z8_device::ld_R1_IM, 10, 5 },    { &z8_device::ld_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::ccf, 6, 5 },

	{ &z8_device::swap_R1, 8, 5 },  { &z8_device::swap_IR1, 8, 5 }, { &z8_device::illegal, 0, 0 },      { &z8_device::ld_Ir1_r2, 6, 5 },
	{ &z8_device::illegal, 0, 0 },      { &z8_device::ld_R2_IR1, 10, 5 },   { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::nop, 6, 0 }
};

/***************************************************************************
    TIMER CALLBACKS
***************************************************************************/

template <int T>
TIMER_CALLBACK_MEMBER(z8_device::timeout)
{
	timer_end<T>();

	if (m_pre[T] & Z8_PRE0_COUNT_MODULO_N)
		timer_start<T>();
	else
	{
		m_count[T] = 0;
		m_internal_timer[T]->enable(false);
	}
}

/***************************************************************************
    INITIALIZATION
***************************************************************************/

void z8_device::device_start()
{
	/* set up the state table */
	{
		state_add(Z8_PC,         "PC",        m_pc).callimport();
		state_add(STATE_GENPC,   "GENPC",     m_pc).callimport().noshow();
		state_add(STATE_GENPCBASE, "CURPC",   m_ppc).callimport().noshow();
		state_add(Z8_SP,         "SP",        m_sp.w);
		state_add(Z8_RP,         "RP",        m_rp);
		state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).noshow().formatstr("%6s");
		state_add(Z8_IMR,        "IMR",       m_imr);
		state_add(Z8_IRQ,        "IRQ",       m_irq);
		state_add(Z8_IPR,        "IPR",       m_ipr);
		state_add(Z8_P0,         "P0",        m_output[0]);
		if (m_rom_size > 0)
			state_add(Z8_P1, "P1",        m_output[1]);
		state_add(Z8_P2,         "P2",        m_output[2]);
		state_add(Z8_P3,         "P3",        m_output[3]).mask(0xf0);
		state_add(Z8_P01M,       "P01M",      m_p01m);
		state_add(Z8_P2M,        "P2M",       m_p2m);
		state_add(Z8_P3M,        "P3M",       m_p3m);
		state_add(Z8_PRE0,       "PRE0",      m_pre[0]);
		state_add(Z8_T0,         "T0",        m_t[0]);
		state_add(Z8_PRE1,       "PRE1",      m_pre[1]);
		state_add(Z8_T1,         "T1",        m_t[1]);
		state_add(Z8_TMR,        "TMR",       m_tmr);
		state_add(Z8_TOUT,       "TOUT",      m_tout);

		for (int regnum = 0; regnum < 16; regnum++)
		{
			state_add<uint8_t>(Z8_R0 + regnum, string_format("R%d", regnum).c_str(),
				[this, regnum]() { auto dis = machine().disable_side_effects(); return register_read((m_rp & 0xf0) | regnum); },
				[this, regnum](uint8_t val) { auto dis = machine().disable_side_effects(); register_write((m_rp & 0xf0) | regnum, val); });
		}

		for (int regnum = 0; regnum < 16; regnum += 2)
		{
			state_add<uint16_t>(Z8_RR0 + (regnum / 2), string_format("RR%d", regnum).c_str(),
				[this, regnum]() { auto dis = machine().disable_side_effects(); return register_pair_read((m_rp & 0xf0) | regnum); },
				[this, regnum](uint16_t val) { auto dis = machine().disable_side_effects(); register_pair_write((m_rp & 0xf0) | regnum, val); }).noshow();
		}
	}

	/* find address spaces */
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(has_space(AS_DATA) ? AS_DATA : AS_PROGRAM).specific(m_data);
	space(AS_IO).specific(m_regs);

	/* allocate timers */
	m_internal_timer[0] = timer_alloc(FUNC(z8_device::timeout<0>), this);
	m_internal_timer[1] = timer_alloc(FUNC(z8_device::timeout<1>), this);

	/* Clear state */
	std::fill(std::begin(m_irq_line), std::end(m_irq_line), CLEAR_LINE);
	std::fill(std::begin(m_output), std::end(m_output), 0);
	std::fill(std::begin(m_t), std::end(m_t), 0);
	std::fill(std::begin(m_count), std::end(m_count), 0);
	std::fill(std::begin(m_pre), std::end(m_pre), 0);
	std::fill(std::begin(m_pre_count), std::end(m_pre_count), 0);
	m_pc = 0;
	m_ppc = 0;
	m_sp.w = 0;
	m_rp = 0;
	m_flags = 0;
	m_p01m = 0;
	m_p2m = 0;
	m_p3m = 0;
	m_p3_output = 0;
	m_tmr = 0;
	m_tout = true;
	m_transmit_sr = 0;
	m_transmit_count = 0;
	m_transmit_parity = false;
	m_receive_buffer = 0;
	m_receive_sr = 0;
	m_receive_count = 0;
	m_receive_parity = false;
	m_receive_started = false;
	m_irq_taken = false;
	m_irq_initialized = false;

	/* register for state saving */
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_sp.w));
	save_item(NAME(m_rp));
	save_item(NAME(m_flags));
	save_item(NAME(m_input));
	save_item(NAME(m_output));
	save_item(NAME(m_p01m));
	save_item(NAME(m_p2m));
	save_item(NAME(m_p3m));
	save_item(NAME(m_p3_output));
	save_item(NAME(m_tmr));
	save_item(NAME(m_t));
	save_item(NAME(m_tout));
	save_item(NAME(m_transmit_sr));
	save_item(NAME(m_transmit_count));
	save_item(NAME(m_transmit_parity));
	save_item(NAME(m_receive_buffer));
	save_item(NAME(m_receive_sr));
	save_item(NAME(m_receive_count));
	save_item(NAME(m_receive_parity));
	save_item(NAME(m_receive_started));
	save_item(NAME(m_count));
	save_item(NAME(m_pre));
	save_item(NAME(m_pre_count));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_irq_taken));
	save_item(NAME(m_irq_initialized));

	set_icountptr(m_icount);
}

/***************************************************************************
    INTERRUPTS
***************************************************************************/

void z8_device::request_interrupt(int irq)
{
	assert(irq >= 0 && irq < 6);

	if (m_irq_initialized)
	{
		m_irq |= 1 << irq;
		LOGMASKED(LOG_IRQ, "%s: IRQ%d requested\n", machine().time().to_string(), irq);
	}
}

void z8_device::take_interrupt(int irq)
{
	//logerror("Taking IRQ%d (previous PC = %04X)\n", irq, m_pc);
	m_irq_taken = true;

	// disable interrupts
	m_imr &= ~Z8_IMR_ENABLE;

	// acknowledge the IRQ
	m_irq &= ~(1 << irq);
	standard_irq_callback(irq, m_pc);

	// get the interrupt vector address
	uint16_t vector = irq * 2;
	if (m_rom_size == 0)
		vector = mask_external_address(vector);

	// push registers onto stack
	stack_push_word(m_pc);
	stack_push_byte(m_flags);

	// branch to the vector
	m_pc = m_cache.read_byte(vector) << 8;
	m_pc |= m_cache.read_byte(vector + 1);
}

void z8_device::process_interrupts()
{
	m_irq_taken = false;
	uint8_t pending_irqs = m_imr & m_irq & Z8_IRQ_MASK;
	if (!(m_imr & Z8_IMR_ENABLE) || pending_irqs == 0)
		return;

	int group_a[2] = { 5, 3 };
	int group_b[2] = { 2, 0 };
	int group_c[2] = { 1, 4 };

	if (BIT(m_ipr, 5))
		std::swap(group_a[0], group_a[1]);
	if (BIT(m_ipr, 2))
		std::swap(group_b[0], group_b[1]);
	if (BIT(m_ipr, 1))
		std::swap(group_c[0], group_c[1]);

	switch ((m_ipr & 0x18) >> 2 | (m_ipr & 0x01))
	{
		case 0: // (000) reserved according to Zilog (but must process at least IRQ4)
		case 1: // (001) C > A > B
			if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			else if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			else if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			break;

		case 2: // (010) A > B > C
			if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			else if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			else if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			break;

		case 3: // (011) A > C > B
			if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			else if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			else if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			break;

		case 4: // (100) B > C > A
			if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			else if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			else if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			break;

		case 5: // (101) C > B > A
			if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			else if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			else if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			break;

		case 6: // (110) B > A > C
		case 7: // (111) reserved according to Zilog
			if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			else if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			else if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			break;
	}
}

/***************************************************************************
    EXECUTION
***************************************************************************/

void z8_device::execute_run()
{
	do
	{
		process_interrupts();
		if (m_irq_taken)
		{
			// interrupt processing takes 58 external clock cycles
			m_icount -= 27;
		}
		else
		{
			// fetch opcode
			uint8_t opcode = fetch_opcode();
			int cycles = Z8601_OPCODE_MAP[opcode].execution_cycles;

			// execute instruction
			(this->*(Z8601_OPCODE_MAP[opcode].function))(opcode, &cycles);

			m_icount -= cycles;
		}
	}
	while (m_icount > 0);
}

/***************************************************************************
    RESET
***************************************************************************/

void z8_device::device_reset()
{
	m_pc = 0x000c;
	m_rp = 0x00;
	m_irq = 0x00;
	m_imr &= ~Z8_IMR_ENABLE;
	m_irq_initialized = false;

	m_pre[0] &= ~Z8_PRE0_COUNT_MODULO_N;
	m_pre[1] &= ~(Z8_PRE1_COUNT_MODULO_N | Z8_PRE1_INTERNAL_CLOCK);
	m_tmr = 0x00;
	timer_stop<0>();
	timer_stop<1>();

	m_output[3] = 0xf0;
	p01m_write(0x4d);
	p2m_write(0xff);
	p3m_write(0x00);
}


/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

void z8_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case Z8_PC:
			m_ppc = m_pc;
			break;

		case STATE_GENPCBASE:
			m_pc = m_ppc;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(z8) called for unexpected value\n");
	}
}

void z8_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c",
					m_flags & Z8_FLAGS_C ? 'C' : '.',
					m_flags & Z8_FLAGS_Z ? 'Z' : '.',
					m_flags & Z8_FLAGS_S ? 'S' : '.',
					m_flags & Z8_FLAGS_V ? 'V' : '.',
					m_flags & Z8_FLAGS_D ? 'D' : '.',
					m_flags & Z8_FLAGS_H ? 'H' : '.');
			break;
	}
}


void z8_device::execute_set_input(int inputnum, int state)
{
	switch ( inputnum )
	{
	// IRQ0 input is P32 (also DAV0/RDY0 handshake, not emulated)
	case INPUT_LINE_IRQ0:
		if (state != CLEAR_LINE && m_irq_line[0] == CLEAR_LINE)
			request_interrupt(0);
		m_irq_line[0] = state;

		if (state != CLEAR_LINE && (m_input[3] & Z8_P3_IRQ0) != 0)
			m_input[3] &= ~Z8_P3_IRQ0;
		else if (state == CLEAR_LINE && (m_input[3] & Z8_P3_IRQ0) == 0)
			m_input[3] |= Z8_P3_IRQ0;

		break;

	// IRQ1 input is P33
	case INPUT_LINE_IRQ1:
		if (state != CLEAR_LINE && m_irq_line[1] == CLEAR_LINE)
			request_interrupt(1);
		m_irq_line[1] = state;

		if (state != CLEAR_LINE && (m_input[3] & Z8_P3_IRQ1) != 0)
			m_input[3] &= ~Z8_P3_IRQ1;
		else if (state == CLEAR_LINE && (m_input[3] & Z8_P3_IRQ1) == 0)
			m_input[3] |= Z8_P3_IRQ1;

		break;

	// IRQ2 input is P31 (also TIN and DAV2/RDY2 handshake, latter not emulated)
	case INPUT_LINE_IRQ2:
		if (state != CLEAR_LINE && m_irq_line[2] == CLEAR_LINE)
			request_interrupt(2);
		m_irq_line[2] = state;

		if (state != CLEAR_LINE && (m_input[3] & Z8_P3_IRQ2) != 0)
		{
			m_input[3] &= ~Z8_P3_IRQ2;
			if ((m_pre[1] & Z8_PRE1_INTERNAL_CLOCK) == 0)
				t1_trigger();
		}
		else if (state == CLEAR_LINE && (m_input[3] & Z8_P3_IRQ2) == 0)
		{
			m_input[3] |= Z8_P3_IRQ2;
			if ((m_pre[1] & Z8_PRE1_INTERNAL_CLOCK) == 0 && (m_tmr & Z8_TMR_TIN_MASK) == Z8_TMR_TIN_GATE)
				timer_start<1>();
		}

		break;

	// IRQ3 input is P30 (also serial DI)
	case INPUT_LINE_IRQ3:
		if (state != CLEAR_LINE && m_irq_line[3] == CLEAR_LINE && (m_p3m & Z8_P3M_P3_SERIAL) == 0)
			request_interrupt(3);
		m_irq_line[3] = state;

		if (state != CLEAR_LINE && (m_input[3] & Z8_P3_IRQ3) != 0)
			m_input[3] &= ~Z8_P3_IRQ3;
		else if (state == CLEAR_LINE && (m_input[3] & Z8_P3_IRQ3) == 0)
			m_input[3] |= Z8_P3_IRQ3;

		break;
	}
}
