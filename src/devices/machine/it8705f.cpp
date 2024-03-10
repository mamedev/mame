// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Winbond IT8705F LPC Super I/O

TODO:
- Move stuff from sis950_lpc

**************************************************************************************************/

#include "emu.h"
#include "machine/it8705f.h"

//#include "machine/ds128x.h"
#include "machine/pckeybrd.h"

#include <algorithm>

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(IT8705F, it8705f_device, "it8705f", "ITE IT8705F LPC Super I/O")

it8705f_device::it8705f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IT8705F, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(it8705f_device::config_map), this))
	, m_lpt(*this, "lpt")
	, m_logical_view(*this, "logical_view")
//  , m_irq1_callback(*this)
//  , m_irq8_callback(*this)
//  , m_irq9_callback(*this)
//  , m_txd1_callback(*this)
//  , m_ndtr1_callback(*this)
//  , m_nrts1_callback(*this)
//  , m_txd2_callback(*this)
//  , m_ndtr2_callback(*this)
//  , m_nrts2_callback(*this)
	, m_index(0)
	, m_logical_index(0)
	, m_lock_sequence_index(0)
{
	std::fill(std::begin(m_activate), std::end(m_activate), false);
}

it8705f_device::~it8705f_device()
{
}

void it8705f_device::device_start()
{
	set_isa_device();
	//m_isa->set_dma_channel(0, this, true);
	//m_isa->set_dma_channel(1, this, true);
	//m_isa->set_dma_channel(2, this, true);
	//m_isa->set_dma_channel(3, this, true);
	remap(AS_IO, 0, 0x400);

}

void it8705f_device::device_reset()
{
	m_index = 0;
	m_lock_sequence_index = 0;

	m_lpt_address = 0x0378;
	m_lpt_irq_line = 7;
	m_lpt_drq_line = 4; // disabled
//  m_lpt_mode = 0x3f;
}

device_memory_interface::space_config_vector it8705f_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void it8705f_device::device_add_mconfig(machine_config &config)
{
	PC_LPT(config, m_lpt);
//  m_lpt->irq_handler().set(FUNC(it8705f_device::irq_parallel_w));
}


void it8705f_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// TODO: apparently installs two ports, an alias at 0x4e/0x4f
		m_isa->install_device(0x002e, 0x002f, read8sm_delegate(*this, FUNC(it8705f_device::read)), write8sm_delegate(*this, FUNC(it8705f_device::write)));

		// can't map below 0x100
		if (m_activate[3] & 1 && m_lpt_address & 0xf00)
		{
			m_isa->install_device(m_lpt_address, m_lpt_address + 3, read8sm_delegate(*m_lpt, FUNC(pc_lpt_device::read)), write8sm_delegate(*m_lpt, FUNC(pc_lpt_device::write)));
		}
	}
}

uint8_t it8705f_device::read(offs_t offset)
{
	if (m_lock_sequence_index != 4)
		return 0;

	if (offset == 0)
		return m_index;

	return space().read_byte(m_index);
}

void it8705f_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		if (m_lock_sequence_index == 4)
			m_index = data;
		else
		{
			// TODO: 0xaa for lock_seq[3] with 0x4e alias
			const u8 lock_seq[4] = { 0x87, 0x01, 0x55, 0x55 };
			if (data == lock_seq[m_lock_sequence_index])
				m_lock_sequence_index ++;
			else
				m_lock_sequence_index = 0;
		}
	}
	else
	{
		if (m_lock_sequence_index == 4)
			space().write_byte(m_index, data);
	}
}

void it8705f_device::config_map(address_map &map)
{
	map(0x02, 0x02).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(data, 1))
				m_lock_sequence_index = 0;
			// TODO: bit 0
		})
	);
	map(0x07, 0x07).lr8(NAME([this] () { return m_logical_index; })).w(FUNC(it8705f_device::logical_device_select_w));
	map(0x20, 0x20).lr8(NAME([] () { return 0x87; })); // device ID
	map(0x21, 0x21).lr8(NAME([] () { return 0x05; })); // revision
//  map(0x22, 0x22) Configuration Select and Chip Version
//  map(0x23, 0x23) Software Suspend
//  map(0x24, 0x24) Clock Selection and Flash ROM I/F control
//  map(0x25, 0x2a) LDN5 GPIO set multi-function pins
//  map(0x2b, 0x2b) LDN4 alternate GPIO set multi-function pins
//  map(0x2e, 0x2f) LDNF4 Test Modes

	map(0x30, 0xff).view(m_logical_view);
	// FDC
	m_logical_view[0](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<0>), FUNC(it8705f_device::activate_w<0>));
	m_logical_view[0](0x31, 0xff).unmaprw();
	// UART1
	m_logical_view[1](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<1>), FUNC(it8705f_device::activate_w<1>));
	m_logical_view[1](0x31, 0xff).unmaprw();
	// UART2
	m_logical_view[2](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<2>), FUNC(it8705f_device::activate_w<2>));
	m_logical_view[2](0x31, 0xff).unmaprw();
	// LPT
	m_logical_view[3](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<3>), FUNC(it8705f_device::activate_w<3>));
	m_logical_view[3](0x60, 0x61).lrw8(
		NAME([this] (offs_t offset) {
			return (m_lpt_address >> (offset * 8)) & 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset * 8;
			m_lpt_address &= 0xff << shift;
			m_lpt_address |= data << (shift ^ 8);
			LOG("LD3 (LPT): remap %04x ([%d] %02x)\n", m_lpt_address, offset, data);

			remap(AS_IO, 0, 0x400);
		})
	);
	//m_logical_view[3](0x62, 0x63) secondary base address
	//m_logical_view[3](0x64, 0x65) POST data port base address
	m_logical_view[3](0x70, 0x70).lrw8(
			NAME([this] () {
			return m_lpt_irq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_lpt_irq_line = data & 0xf;
			LOG("LD3 (LPT): irq routed to %02x\n", m_lpt_irq_line);
		})
	);
	m_logical_view[3](0x74, 0x74).lrw8(
			NAME([this] () {
			return m_lpt_drq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_lpt_drq_line = data & 0x7;
			LOG("LD3 (LPT): drq %s (%02x)\n", BIT(m_lpt_drq_line, 2) ? "disabled" : "enabled", data);
		})
	);
	// Environment controller / HW monitor
	m_logical_view[4](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<4>), FUNC(it8705f_device::activate_w<4>));
	m_logical_view[4](0x31, 0xff).unmaprw();
	// GPIO
	m_logical_view[5](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<5>), FUNC(it8705f_device::activate_w<5>));
	m_logical_view[5](0x31, 0xff).unmaprw();
	// Game port
	m_logical_view[6](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<6>), FUNC(it8705f_device::activate_w<6>));
	m_logical_view[6](0x31, 0xff).unmaprw();
	// Consumer IR
	m_logical_view[7](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<7>), FUNC(it8705f_device::activate_w<7>));
	m_logical_view[7](0x31, 0xff).unmaprw();
	// MIDI port
	m_logical_view[8](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<8>), FUNC(it8705f_device::activate_w<8>));
	m_logical_view[8](0x31, 0xff).unmaprw();
}

/*
 * Global register space
 */

void it8705f_device::logical_device_select_w(offs_t offset, u8 data)
{
	m_logical_index = data;
	if (m_logical_index <= 0x8)
		m_logical_view.select(m_logical_index);
	else
		LOG("Attempt to select an unmapped device with %02x\n", data);
}

template <unsigned N> u8 it8705f_device::activate_r(offs_t offset)
{
	return m_activate[N];
}

template <unsigned N> void it8705f_device::activate_w(offs_t offset, u8 data)
{
	m_activate[N] = data & 1;
	LOG("LN%d Device %s\n", N, data & 1 ? "enabled" : "disabled");
	remap(AS_IO, 0, 0x400);
}

#if 0
void it8705f_device::request_irq(int irq, int state)
{
	switch (irq)
	{
	case 1:
		m_irq1_callback(state);
		break;
	case 3:
		m_isa->irq3_w(state);
		break;
	case 4:
		m_isa->irq4_w(state);
		break;
	case 5:
		m_isa->irq5_w(state);
		break;
	case 6:
		m_isa->irq6_w(state);
		break;
	case 7:
		m_isa->irq7_w(state);
		break;
	case 8:
		m_irq8_callback(state);
		break;
	case 9:
		m_irq9_callback(state);
		break;
	case 10:
		m_isa->irq10_w(state);
		break;
	case 11:
		m_isa->irq11_w(state);
		break;
	case 12:
		m_isa->irq12_w(state);
		break;
	case 14:
		m_isa->irq14_w(state);
		break;
	case 15:
		m_isa->irq15_w(state);
		break;
	}
}

/*
 * Device #3 (Parallel)
 */

void it8705f_device::irq_parallel_w(int state)
{
	if (m_activate[3] == false)
		return;
	request_irq(m_lpt_irq_line, state ? ASSERT_LINE : CLEAR_LINE);
}

#endif
