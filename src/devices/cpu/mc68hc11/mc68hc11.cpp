// license:BSD-3-Clause
// copyright-holders:Ville Linde, Angelo Salese, hap
/*
   Motorola MC68HC11 emulator

   Written by Ville Linde & Angelo Salese

TODO:
- Interrupts handling is really bare-bones, just to make Hit Poker happy;
- Timers are really sketchy as per now, only TOC1 is emulated so far;
- Complete opcodes hook-up;
- Emulate the MC68HC12 (same as HC11 with a bunch of new opcodes);

 */

#include "emu.h"
#include "debugger.h"
#include "mc68hc11.h"
#include "hc11dasm.h"

enum
{
	HC11_PC = 1,
	HC11_SP,
	HC11_A,
	HC11_B,
	HC11_IX,
	HC11_IY
};

#define CC_S    0x80
#define CC_X    0x40
#define CC_H    0x20
#define CC_I    0x10
#define CC_N    0x08
#define CC_Z    0x04
#define CC_V    0x02
#define CC_C    0x01

static const int div_tab[4] = { 1, 4, 8, 16 };


DEFINE_DEVICE_TYPE(MC68HC11A1, mc68hc11a1_device, "mc68hc11a1", "Motorola MC68HC11A1")
DEFINE_DEVICE_TYPE(MC68HC11D0, mc68hc11d0_device, "mc68hc11d0", "Motorola MC68HC11D0")
DEFINE_DEVICE_TYPE(MC68HC11F1, mc68hc11f1_device, "mc68hc11f1", "Motorola MC68HC11F1")
DEFINE_DEVICE_TYPE(MC68HC11K1, mc68hc11k1_device, "mc68hc11k1", "Motorola MC68HC11K1")
DEFINE_DEVICE_TYPE(MC68HC11M0, mc68hc11m0_device, "mc68hc11m0", "Motorola MC68HC11M0")


mc68hc11_cpu_device::mc68hc11_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t internal_ram_size, uint16_t reg_block_size, uint8_t init_value, address_map_constructor reg_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_data_config("data", ENDIANNESS_BIG, 8, internal_ram_size > 1024 ? 11
		: internal_ram_size > 512 ? 10
		: internal_ram_size > 256 ? 9 : 8, 0, address_map_constructor(FUNC(mc68hc11_cpu_device::ram_map), this))
	, m_io_config("I/O", ENDIANNESS_BIG, 8, reg_block_size > 128 ? 8 : reg_block_size > 64 ? 7 : 6, 0, reg_map)
	, m_port_input_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_port_output_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_analog_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_spi2_data_input_cb(*this)
	, m_spi2_data_output_cb(*this)
	, m_internal_ram_size(internal_ram_size)
	, m_reg_block_size(reg_block_size)
	, m_init_value(init_value)
{
}

void mc68hc11_cpu_device::ram_map(address_map &map)
{
	map(0, m_internal_ram_size - 1).ram();
}

mc68hc11a1_device::mc68hc11a1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11A1, tag, owner, clock, 256, 64, 0x01,
		address_map_constructor(FUNC(mc68hc11a1_device::io_map), this)) // TODO: also has 512 bytes EEPROM
{
}

mc68hc11d0_device::mc68hc11d0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11D0, tag, owner, clock, 192, 64, 0x00,
		address_map_constructor(FUNC(mc68hc11d0_device::io_map), this))
{
}

mc68hc11f1_device::mc68hc11f1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11F1, tag, owner, clock, 1024, 96, 0x01,
		address_map_constructor(FUNC(mc68hc11f1_device::io_map), this)) // TODO: also has 512 bytes EEPROM
{
}

mc68hc11k1_device::mc68hc11k1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11K1, tag, owner, clock, 768, 128, 0x00,
		address_map_constructor(FUNC(mc68hc11k1_device::io_map), this)) // TODO: also has 640 bytes EEPROM
{
}

mc68hc11m0_device::mc68hc11m0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11M0, tag, owner, clock, 1280, 256, 0x00,
		address_map_constructor(FUNC(mc68hc11m0_device::io_map), this))
{
}

device_memory_interface::space_config_vector mc68hc11_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}

void mc68hc11_cpu_device::device_resolve_objects()
{
	for (auto &cb : m_port_input_cb)
		cb.resolve_safe(0xff);
	for (auto &cb : m_port_output_cb)
		cb.resolve_safe();
	for (auto &cb : m_analog_cb)
		cb.resolve_safe(0);
	m_spi2_data_input_cb.resolve_safe(0xff);
	m_spi2_data_output_cb.resolve_safe();
}

std::unique_ptr<util::disasm_interface> mc68hc11_cpu_device::create_disassembler()
{
	return std::make_unique<hc11_disassembler>();
}


#define HC11OP(XX)      mc68hc11_cpu_device::hc11_##XX

/*****************************************************************************/
/* Internal registers */

template <int P>
uint8_t mc68hc11_cpu_device::port_r()
{
	uint8_t dir = m_port_dir[P];
	return (m_port_data[P] & dir) | (dir == 0xff ? 0 : m_port_input_cb[P](0, ~dir) & ~dir);
}

template <int P>
void mc68hc11_cpu_device::port_w(uint8_t data)
{
	uint8_t dir = m_port_dir[P];
	uint8_t old_data = std::exchange(m_port_data[P], data);
	if ((old_data & dir) != (data & dir))
		m_port_output_cb[P](0, data & dir, dir);
}

template <int P>
uint8_t mc68hc11_cpu_device::ddr_r()
{
	return m_port_dir[P];
}

template <int P>
void mc68hc11_cpu_device::ddr_w(uint8_t data)
{
	m_port_dir[P] = data;
	if (data != 0x00)
		m_port_output_cb[P](0, m_port_data[P] & data, data);
}

uint8_t mc68hc11_cpu_device::pioc_r()
{
	return 0;
}

uint8_t mc68hc11_cpu_device::pactl_r()
{
	return m_pactl;
}

void mc68hc11_cpu_device::pactl_w(uint8_t data)
{
	m_pactl = data & 0x73;
}

uint8_t mc68hc11a1_device::pactl_r()
{
	return (ddr_r<0>() & 0x80) | mc68hc11_cpu_device::pactl_r();
}

void mc68hc11a1_device::pactl_w(uint8_t data)
{
	mc68hc11_cpu_device::pactl_w(data & 0x73);
	ddr_w<0>((data & 0x80) | 0x78);
}

uint8_t mc68hc11d0_device::pactl_r()
{
	return (ddr_r<0>() & 0x88) | mc68hc11_cpu_device::pactl_r();
}

void mc68hc11d0_device::pactl_w(uint8_t data)
{
	mc68hc11_cpu_device::pactl_w(data & 0x73);
	ddr_w<0>((data & 0x88) | 0x70);
}

uint8_t mc68hc11_cpu_device::tcnt_r(offs_t offset)
{
	return (m_tcnt >> (BIT(offset, 0) ? 0 : 8)) & 0xff;
}

void mc68hc11_cpu_device::tcnt_w(offs_t offset, uint8_t data)
{
	logerror("HC11: TCNT%c register write %02x!\n", BIT(offset, 0) ? 'L' : 'H', data);
}

uint8_t mc68hc11_cpu_device::toc1_r(offs_t offset)
{
	return (m_toc1 >> (BIT(offset, 0) ? 0 : 8)) & 0xff;
}

void mc68hc11_cpu_device::toc1_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_toc1 = (data & 0xff) | (m_toc1 & 0xff00);
	else // TODO: inhibit count for one bus cycle
		m_toc1 = (data << 8) | (m_toc1 & 0xff);
}

uint8_t mc68hc11_cpu_device::tmsk1_r()
{
	return m_tmsk1;
}

void mc68hc11_cpu_device::tmsk1_w(uint8_t data)
{
	m_tmsk1 = data;
}

uint8_t mc68hc11_cpu_device::tflg1_r()
{
	return m_tflg1;
}

void mc68hc11_cpu_device::tflg1_w(uint8_t data)
{
	m_tflg1 &= ~data;
}

void mc68hc11_cpu_device::tmsk2_w(uint8_t data)
{
	m_pr = data & 3;
}

template <int N>
uint8_t mc68hc11_cpu_device::spcr_r()
{
	return 0;
}

template <int N>
uint8_t mc68hc11_cpu_device::spsr_r()
{
	return 0x80;
}

template <int N>
uint8_t mc68hc11_cpu_device::spdr_r()
{
	if (N == 1)
		return m_spi2_data_input_cb();
	else
		return 0;
}

template <int N>
void mc68hc11_cpu_device::spdr_w(uint8_t data)
{
	if (N == 1)
		m_spi2_data_output_cb(data);
}

uint8_t mc68hc11_cpu_device::adctl_r()
{
	return 0x80;
}

void mc68hc11_cpu_device::adctl_w(uint8_t data)
{
	m_adctl = data;
}

uint8_t mc68hc11_cpu_device::adr_r(offs_t offset)
{
	if (m_adctl & 0x10)
		return m_analog_cb[(m_adctl & 0x4) + offset]();
	else
		return m_analog_cb[m_adctl & 0x7]();
}

uint8_t mc68hc11_cpu_device::opt2_r()
{
	return 0;
}

uint8_t mc68hc11_cpu_device::init_r()
{
	int reg_page = (m_reg_position >> 12) & 0xf;
	int ram_page = (m_ram_position >> 12) & 0xf;

	return (ram_page << 4) | reg_page;
}

void mc68hc11_cpu_device::init_w(uint8_t data)
{
	// TODO: only writeable during first 64 E cycles
	int reg_page = data & 0xf;
	int ram_page = (data >> 4) & 0xf;

	if (reg_page == ram_page && m_init_value == 0x00)
	{
		m_reg_position = reg_page << 12;
		m_ram_position = (ram_page << 12) + m_reg_block_size;
	}
	else
	{
		m_reg_position = reg_page << 12;
		m_ram_position = ram_page << 12;
	}
}

uint8_t mc68hc11_cpu_device::scbd_r(offs_t offset)
{
	return 0;
}

uint8_t mc68hc11_cpu_device::sccr1_r()
{
	return 0;
}

uint8_t mc68hc11_cpu_device::sccr2_r()
{
	return 0;
}

uint8_t mc68hc11_cpu_device::scsr1_r()
{
	return 0x40;
}

uint8_t mc68hc11_cpu_device::scrdl_r()
{
	return 0;
}

uint8_t mc68hc11_cpu_device::opt4_r()
{
	return 0;
}

void mc68hc11a1_device::io_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(mc68hc11a1_device::port_r<0>), FUNC(mc68hc11a1_device::port_w<0>)); // PORTA
	map(0x02, 0x02).r(FUNC(mc68hc11a1_device::pioc_r)); // PIOC
	map(0x03, 0x03).rw(FUNC(mc68hc11a1_device::port_r<2>), FUNC(mc68hc11a1_device::port_w<2>)); // PORTC
	map(0x04, 0x04).rw(FUNC(mc68hc11a1_device::port_r<1>), FUNC(mc68hc11a1_device::port_w<1>)); // PORTB
	map(0x05, 0x05).nopw(); // PORTCL
	map(0x07, 0x07).rw(FUNC(mc68hc11a1_device::ddr_r<2>), FUNC(mc68hc11a1_device::ddr_w<2>)); // DDRC
	map(0x08, 0x08).rw(FUNC(mc68hc11a1_device::port_r<3>), FUNC(mc68hc11a1_device::port_w<3>)); // PORTD
	map(0x09, 0x09).rw(FUNC(mc68hc11a1_device::ddr_r<3>), FUNC(mc68hc11a1_device::ddr_w<3>)); // DDRD
	map(0x0a, 0x0a).r(FUNC(mc68hc11a1_device::port_r<4>)); // PORTE
	map(0x0e, 0x0f).rw(FUNC(mc68hc11a1_device::tcnt_r), FUNC(mc68hc11a1_device::tcnt_w)); // TCNT
	map(0x16, 0x17).rw(FUNC(mc68hc11a1_device::toc1_r), FUNC(mc68hc11a1_device::toc1_w)); // TOC1
	map(0x22, 0x22).rw(FUNC(mc68hc11a1_device::tmsk1_r), FUNC(mc68hc11a1_device::tmsk1_w)); // TMSK1
	map(0x23, 0x23).rw(FUNC(mc68hc11a1_device::tflg1_r), FUNC(mc68hc11a1_device::tflg1_w)); // TFLG1
	map(0x24, 0x24).w(FUNC(mc68hc11a1_device::tmsk2_w)); // TMSK2
	map(0x26, 0x26).rw(FUNC(mc68hc11a1_device::pactl_r), FUNC(mc68hc11a1_device::pactl_w)); // PACTL
	map(0x28, 0x28).r(FUNC(mc68hc11a1_device::spcr_r<0>)).nopw(); // SPCR
	map(0x29, 0x29).r(FUNC(mc68hc11a1_device::spsr_r<0>)).nopw(); // SPSR
	map(0x2a, 0x2a).rw(FUNC(mc68hc11a1_device::spdr_r<0>), FUNC(mc68hc11a1_device::spdr_w<0>)); // SPDR
	map(0x2c, 0x2c).r(FUNC(mc68hc11a1_device::sccr1_r)).nopw(); // SCCR1
	map(0x2d, 0x2d).r(FUNC(mc68hc11a1_device::sccr2_r)).nopw(); // SCCR2
	map(0x30, 0x30).rw(FUNC(mc68hc11a1_device::adctl_r), FUNC(mc68hc11a1_device::adctl_w)); // ADCTL
	map(0x31, 0x34).r(FUNC(mc68hc11a1_device::adr_r)); // ADR1-ADR4
	map(0x39, 0x39).nopw(); // OPTION
	map(0x3a, 0x3a).nopw(); // COPRST (watchdog)
	map(0x3b, 0x3b).nopw(); // PPROG (EEPROM programming)
	map(0x3d, 0x3d).rw(FUNC(mc68hc11a1_device::init_r), FUNC(mc68hc11a1_device::init_w)); // INIT
	map(0x3f, 0x3f).nopw(); // CONFIG
}

void mc68hc11d0_device::io_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(mc68hc11d0_device::port_r<0>), FUNC(mc68hc11d0_device::port_w<0>)); // PORTA
	map(0x02, 0x02).r(FUNC(mc68hc11d0_device::pioc_r)); // PIOC
	map(0x03, 0x03).rw(FUNC(mc68hc11d0_device::port_r<2>), FUNC(mc68hc11d0_device::port_w<2>)); // PORTC
	map(0x04, 0x04).rw(FUNC(mc68hc11d0_device::port_r<1>), FUNC(mc68hc11d0_device::port_w<1>)); // PORTB
	map(0x06, 0x06).rw(FUNC(mc68hc11d0_device::ddr_r<1>), FUNC(mc68hc11d0_device::ddr_w<1>)); // DDRB
	map(0x07, 0x07).rw(FUNC(mc68hc11d0_device::ddr_r<2>), FUNC(mc68hc11d0_device::ddr_w<2>)); // DDRC
	map(0x08, 0x08).rw(FUNC(mc68hc11d0_device::port_r<3>), FUNC(mc68hc11d0_device::port_w<3>)); // PORTD
	map(0x09, 0x09).rw(FUNC(mc68hc11d0_device::ddr_r<3>), FUNC(mc68hc11d0_device::ddr_w<3>)); // DDRD
	map(0x0e, 0x0f).rw(FUNC(mc68hc11d0_device::tcnt_r), FUNC(mc68hc11d0_device::tcnt_w)); // TCNT
	map(0x16, 0x17).rw(FUNC(mc68hc11d0_device::toc1_r), FUNC(mc68hc11d0_device::toc1_w)); // TOC1
	map(0x22, 0x22).rw(FUNC(mc68hc11d0_device::tmsk1_r), FUNC(mc68hc11d0_device::tmsk1_w)); // TMSK1
	map(0x23, 0x23).rw(FUNC(mc68hc11d0_device::tflg1_r), FUNC(mc68hc11d0_device::tflg1_w)); // TFLG1
	map(0x24, 0x24).w(FUNC(mc68hc11d0_device::tmsk2_w)); // TMSK2
	map(0x26, 0x26).rw(FUNC(mc68hc11d0_device::pactl_r), FUNC(mc68hc11d0_device::pactl_w)); // PACTL
	map(0x28, 0x28).r(FUNC(mc68hc11d0_device::spcr_r<0>)).nopw(); // SPCR
	map(0x29, 0x29).r(FUNC(mc68hc11d0_device::spsr_r<0>)).nopw(); // SPSR
	map(0x2a, 0x2a).rw(FUNC(mc68hc11d0_device::spdr_r<0>), FUNC(mc68hc11d0_device::spdr_w<0>)); // SPDR
	map(0x2c, 0x2c).r(FUNC(mc68hc11d0_device::sccr1_r)).nopw(); // SCCR1
	map(0x2d, 0x2d).r(FUNC(mc68hc11d0_device::sccr2_r)).nopw(); // SCCR2
	map(0x2e, 0x2e).lr8([] { return 0xc0; }, "scsr_r").nopw(); // SCSR
	map(0x2f, 0x2f).nopw(); // SCDR
	map(0x39, 0x39).nopw(); // OPTION
	map(0x3a, 0x3a).nopw(); // COPRST (watchdog)
	map(0x3d, 0x3d).rw(FUNC(mc68hc11d0_device::init_r), FUNC(mc68hc11d0_device::init_w)); // INIT
	map(0x3f, 0x3f).nopw(); // CONFIG
}

void mc68hc11f1_device::io_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(mc68hc11f1_device::port_r<0>), FUNC(mc68hc11f1_device::port_w<0>)); // PORTA
	map(0x01, 0x01).rw(FUNC(mc68hc11f1_device::ddr_r<0>), FUNC(mc68hc11f1_device::ddr_w<0>)); // DDRA
	map(0x02, 0x02).rw(FUNC(mc68hc11f1_device::port_r<6>), FUNC(mc68hc11f1_device::port_w<6>)); // PORTG
	map(0x03, 0x03).rw(FUNC(mc68hc11f1_device::ddr_r<6>), FUNC(mc68hc11f1_device::ddr_w<6>)); // DDRG
	map(0x04, 0x04).rw(FUNC(mc68hc11f1_device::port_r<1>), FUNC(mc68hc11f1_device::port_w<1>)); // PORTB
	map(0x05, 0x05).rw(FUNC(mc68hc11f1_device::port_r<5>), FUNC(mc68hc11f1_device::port_w<5>)); // PORTF
	map(0x06, 0x06).rw(FUNC(mc68hc11f1_device::port_r<2>), FUNC(mc68hc11f1_device::port_w<2>)); // PORTC
	map(0x07, 0x07).rw(FUNC(mc68hc11f1_device::ddr_r<2>), FUNC(mc68hc11f1_device::ddr_w<2>)); // DDRC
	map(0x08, 0x08).rw(FUNC(mc68hc11f1_device::port_r<3>), FUNC(mc68hc11f1_device::port_w<3>)); // PORTD
	map(0x09, 0x09).rw(FUNC(mc68hc11f1_device::ddr_r<3>), FUNC(mc68hc11f1_device::ddr_w<3>)); // DDRD
	map(0x0a, 0x0a).r(FUNC(mc68hc11f1_device::port_r<4>)); // PORTE
	map(0x0b, 0x0b).nopw(); // CFORC
	map(0x0c, 0x0c).nopw(); // OC1M
	map(0x0d, 0x0d).nopw(); // OC1D
	map(0x0e, 0x0f).rw(FUNC(mc68hc11f1_device::tcnt_r), FUNC(mc68hc11f1_device::tcnt_w)); // TCNT
	map(0x10, 0x11).nopr(); // TIC1
	map(0x12, 0x13).nopr(); // TIC2
	map(0x14, 0x15).nopr(); // TIC3
	map(0x16, 0x17).rw(FUNC(mc68hc11f1_device::toc1_r), FUNC(mc68hc11f1_device::toc1_w)); // TOC1
	map(0x22, 0x22).rw(FUNC(mc68hc11f1_device::tmsk1_r), FUNC(mc68hc11f1_device::tmsk1_w)); // TMSK1
	map(0x23, 0x23).rw(FUNC(mc68hc11f1_device::tflg1_r), FUNC(mc68hc11f1_device::tflg1_w)); // TFLG1
	map(0x24, 0x24).w(FUNC(mc68hc11f1_device::tmsk2_w)); // TMSK2
	map(0x25, 0x25).nopr(); // TFLG2
	map(0x26, 0x26).rw(FUNC(mc68hc11f1_device::pactl_r), FUNC(mc68hc11f1_device::pactl_w)); // PACTL
	map(0x27, 0x27).nopw(); // PACNT
	map(0x28, 0x28).r(FUNC(mc68hc11f1_device::spcr_r<0>)).nopw(); // SPCR
	map(0x29, 0x29).r(FUNC(mc68hc11f1_device::spsr_r<0>)).nopw(); // SPSR
	map(0x2a, 0x2a).rw(FUNC(mc68hc11f1_device::spdr_r<0>), FUNC(mc68hc11f1_device::spdr_w<0>)); // SPDR
	map(0x2b, 0x2b).nopw(); // BAUD
	map(0x2c, 0x2c).r(FUNC(mc68hc11f1_device::sccr1_r)).nopw(); // SCCR1
	map(0x2d, 0x2d).r(FUNC(mc68hc11f1_device::sccr2_r)).nopw(); // SCCR2
	map(0x2e, 0x2e).r(FUNC(mc68hc11f1_device::scsr1_r)); // SCSR
	map(0x2f, 0x2f).nopw(); // SCDR
	map(0x30, 0x30).rw(FUNC(mc68hc11f1_device::adctl_r), FUNC(mc68hc11f1_device::adctl_w)); // ADCTL
	map(0x31, 0x34).r(FUNC(mc68hc11f1_device::adr_r)); // ADR1-ADR4
	map(0x35, 0x35).nopw(); // BPROT
	map(0x38, 0x38).nopw(); // OPT2
	map(0x39, 0x39).nopw(); // OPTION
	map(0x3a, 0x3a).nopw(); // COPRST (watchdog)
	map(0x3b, 0x3b).nopw(); // PPROG (EEPROM programming)
	map(0x3c, 0x3c).nopw(); // HPRIO
	map(0x3d, 0x3d).rw(FUNC(mc68hc11f1_device::init_r), FUNC(mc68hc11f1_device::init_w)); // INIT
	map(0x3e, 0x3e).nopw(); // TEST1
	map(0x3f, 0x3f).nopw(); // CONFIG
	map(0x5c, 0x5c).nopw(); // CSSTRH
	map(0x5d, 0x5d).nopw(); // CSSTRL
	map(0x5e, 0x5e).nopw(); // CSGADR
	map(0x5f, 0x5f).nopw(); // CSGSIZ
}

void mc68hc11k1_device::io_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(mc68hc11k1_device::port_r<0>), FUNC(mc68hc11k1_device::port_w<0>)); // PORTA
	map(0x01, 0x01).rw(FUNC(mc68hc11k1_device::ddr_r<0>), FUNC(mc68hc11k1_device::ddr_w<0>)); // DDRA
	map(0x02, 0x02).rw(FUNC(mc68hc11k1_device::ddr_r<1>), FUNC(mc68hc11k1_device::ddr_w<1>)); // DDRB
	map(0x03, 0x03).rw(FUNC(mc68hc11k1_device::ddr_r<5>), FUNC(mc68hc11k1_device::ddr_w<5>)); // DDRF
	map(0x04, 0x04).rw(FUNC(mc68hc11k1_device::port_r<1>), FUNC(mc68hc11k1_device::port_w<1>)); // PORTB
	map(0x05, 0x05).rw(FUNC(mc68hc11k1_device::port_r<5>), FUNC(mc68hc11k1_device::port_w<5>)); // PORTF
	map(0x06, 0x06).rw(FUNC(mc68hc11k1_device::port_r<2>), FUNC(mc68hc11k1_device::port_w<2>)); // PORTC
	map(0x07, 0x07).rw(FUNC(mc68hc11k1_device::ddr_r<2>), FUNC(mc68hc11k1_device::ddr_w<2>)); // DDRC
	map(0x08, 0x08).rw(FUNC(mc68hc11k1_device::port_r<3>), FUNC(mc68hc11k1_device::port_w<3>)); // PORTD
	map(0x09, 0x09).rw(FUNC(mc68hc11k1_device::ddr_r<3>), FUNC(mc68hc11k1_device::ddr_w<3>)); // DDRD
	map(0x0a, 0x0a).r(FUNC(mc68hc11k1_device::port_r<4>)); // PORTE
	map(0x0e, 0x0f).rw(FUNC(mc68hc11k1_device::tcnt_r), FUNC(mc68hc11k1_device::tcnt_w)); // TCNT
	map(0x16, 0x17).rw(FUNC(mc68hc11k1_device::toc1_r), FUNC(mc68hc11k1_device::toc1_w)); // TOC1
	map(0x22, 0x22).rw(FUNC(mc68hc11k1_device::tmsk1_r), FUNC(mc68hc11k1_device::tmsk1_w)); // TMSK1
	map(0x23, 0x23).rw(FUNC(mc68hc11k1_device::tflg1_r), FUNC(mc68hc11k1_device::tflg1_w)); // TFLG1
	map(0x24, 0x24).w(FUNC(mc68hc11k1_device::tmsk2_w)); // TMSK2
	map(0x26, 0x26).rw(FUNC(mc68hc11k1_device::pactl_r), FUNC(mc68hc11k1_device::pactl_w)); // PACTL
	map(0x28, 0x28).r(FUNC(mc68hc11k1_device::spcr_r<0>)).nopw(); // SPCR
	map(0x29, 0x29).r(FUNC(mc68hc11k1_device::spsr_r<0>)).nopw(); // SPSR
	map(0x2a, 0x2a).rw(FUNC(mc68hc11k1_device::spdr_r<0>), FUNC(mc68hc11k1_device::spdr_w<0>)); // SPDR
	map(0x30, 0x30).rw(FUNC(mc68hc11k1_device::adctl_r), FUNC(mc68hc11k1_device::adctl_w)); // ADCTL
	map(0x31, 0x34).r(FUNC(mc68hc11k1_device::adr_r)); // ADR1-ADR4
	map(0x38, 0x38).r(FUNC(mc68hc11k1_device::opt2_r)).nopw(); // OPT2
	map(0x39, 0x39).nopw(); // OPTION
	map(0x3a, 0x3a).nopw(); // COPRST (watchdog)
	map(0x3b, 0x3b).nopw(); // PPROG (EEPROM programming)
	map(0x3d, 0x3d).rw(FUNC(mc68hc11k1_device::init_r), FUNC(mc68hc11k1_device::init_w)); // INIT
	map(0x3f, 0x3f).nopw(); // CONFIG
	map(0x70, 0x71).r(FUNC(mc68hc11k1_device::scbd_r)).nopw(); // SCBD
	map(0x72, 0x72).r(FUNC(mc68hc11k1_device::sccr1_r)).nopw(); // SCCR1
	map(0x73, 0x73).r(FUNC(mc68hc11k1_device::sccr2_r)).nopw(); // SCCR2
	map(0x74, 0x74).r(FUNC(mc68hc11k1_device::scsr1_r)).nopw(); // SCSR1
	map(0x77, 0x77).r(FUNC(mc68hc11k1_device::scrdl_r)).nopw(); // SCRDL
	map(0x7c, 0x7c).rw(FUNC(mc68hc11k1_device::port_r<7>), FUNC(mc68hc11k1_device::port_w<7>)); // PORTH
	map(0x7d, 0x7d).rw(FUNC(mc68hc11k1_device::ddr_r<7>), FUNC(mc68hc11k1_device::ddr_w<7>)); // DDRH
	map(0x7e, 0x7e).rw(FUNC(mc68hc11k1_device::port_r<6>), FUNC(mc68hc11k1_device::port_w<6>)); // PORTG
	map(0x7f, 0x7f).rw(FUNC(mc68hc11k1_device::ddr_r<6>), FUNC(mc68hc11k1_device::ddr_w<6>)); // DDRG
}

void mc68hc11m0_device::io_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(mc68hc11m0_device::port_r<0>), FUNC(mc68hc11m0_device::port_w<0>)); // PORTA
	map(0x01, 0x01).rw(FUNC(mc68hc11m0_device::ddr_r<0>), FUNC(mc68hc11m0_device::ddr_w<0>)); // DDRA
	map(0x02, 0x02).rw(FUNC(mc68hc11m0_device::ddr_r<1>), FUNC(mc68hc11m0_device::ddr_w<1>)); // DDRB
	map(0x03, 0x03).rw(FUNC(mc68hc11m0_device::ddr_r<5>), FUNC(mc68hc11m0_device::ddr_w<5>)); // DDRF
	map(0x04, 0x04).rw(FUNC(mc68hc11m0_device::port_r<1>), FUNC(mc68hc11m0_device::port_w<1>)); // PORTB
	map(0x05, 0x05).rw(FUNC(mc68hc11m0_device::port_r<5>), FUNC(mc68hc11m0_device::port_w<5>)); // PORTF
	map(0x06, 0x06).rw(FUNC(mc68hc11m0_device::port_r<2>), FUNC(mc68hc11m0_device::port_w<2>)); // PORTC
	map(0x07, 0x07).rw(FUNC(mc68hc11m0_device::ddr_r<2>), FUNC(mc68hc11m0_device::ddr_w<2>)); // DDRC
	map(0x08, 0x08).rw(FUNC(mc68hc11m0_device::port_r<3>), FUNC(mc68hc11m0_device::port_w<3>)); // PORTD
	map(0x09, 0x09).rw(FUNC(mc68hc11m0_device::ddr_r<3>), FUNC(mc68hc11m0_device::ddr_w<3>)); // DDRD
	map(0x0a, 0x0a).r(FUNC(mc68hc11m0_device::port_r<4>)); // PORTE
	map(0x0e, 0x0f).rw(FUNC(mc68hc11m0_device::tcnt_r), FUNC(mc68hc11m0_device::tcnt_w)); // TCNT
	map(0x16, 0x17).rw(FUNC(mc68hc11m0_device::toc1_r), FUNC(mc68hc11m0_device::toc1_w)); // TOC1
	map(0x22, 0x22).rw(FUNC(mc68hc11m0_device::tmsk1_r), FUNC(mc68hc11m0_device::tmsk1_w)); // TMSK1
	map(0x23, 0x23).rw(FUNC(mc68hc11m0_device::tflg1_r), FUNC(mc68hc11m0_device::tflg1_w)); // TFLG1
	map(0x24, 0x24).w(FUNC(mc68hc11m0_device::tmsk2_w)); // TMSK2
	map(0x26, 0x26).rw(FUNC(mc68hc11m0_device::pactl_r), FUNC(mc68hc11m0_device::pactl_w)); // PACTL
	map(0x28, 0x28).r(FUNC(mc68hc11m0_device::spcr_r<0>)).nopw(); // SPCR1
	map(0x29, 0x29).r(FUNC(mc68hc11m0_device::spsr_r<0>)).nopw(); // SPSR1
	map(0x2a, 0x2a).rw(FUNC(mc68hc11m0_device::spdr_r<0>), FUNC(mc68hc11m0_device::spdr_w<0>)); // SPDR1
	map(0x30, 0x30).rw(FUNC(mc68hc11m0_device::adctl_r), FUNC(mc68hc11m0_device::adctl_w)); // ADCTL
	map(0x31, 0x34).r(FUNC(mc68hc11m0_device::adr_r)); // ADR1-ADR4
	map(0x38, 0x38).r(FUNC(mc68hc11m0_device::opt2_r)).nopw(); // OPT2
	map(0x39, 0x39).nopw(); // OPTION
	map(0x3a, 0x3a).nopw(); // COPRST (watchdog)
	map(0x3d, 0x3d).rw(FUNC(mc68hc11m0_device::init_r), FUNC(mc68hc11m0_device::init_w)); // INIT
	map(0x3f, 0x3f).nopw(); // CONFIG
	map(0x70, 0x71).r(FUNC(mc68hc11m0_device::scbd_r)).nopw(); // SCBD
	map(0x72, 0x72).r(FUNC(mc68hc11m0_device::sccr1_r)).nopw(); // SCCR1
	map(0x73, 0x73).r(FUNC(mc68hc11m0_device::sccr2_r)).nopw(); // SCCR2
	map(0x74, 0x74).r(FUNC(mc68hc11m0_device::scsr1_r)); // SCSR1
	map(0x77, 0x77).r(FUNC(mc68hc11m0_device::scrdl_r)).nopw(); // SCRDL
	map(0x7c, 0x7c).rw(FUNC(mc68hc11m0_device::port_r<7>), FUNC(mc68hc11m0_device::port_w<7>)); // PORTH
	map(0x7d, 0x7d).rw(FUNC(mc68hc11m0_device::ddr_r<7>), FUNC(mc68hc11m0_device::ddr_w<7>)); // DDRH
	map(0x7e, 0x7e).rw(FUNC(mc68hc11m0_device::port_r<6>), FUNC(mc68hc11m0_device::port_w<6>)); // PORTG
	map(0x7f, 0x7f).rw(FUNC(mc68hc11m0_device::ddr_r<6>), FUNC(mc68hc11m0_device::ddr_w<6>)); // DDRG
	map(0x88, 0x88).r(FUNC(mc68hc11m0_device::spcr_r<1>)).nopw(); // SPCR2
	map(0x89, 0x89).r(FUNC(mc68hc11m0_device::spsr_r<1>)).nopw(); // SPSR2
	map(0x8a, 0x8a).rw(FUNC(mc68hc11m0_device::spdr_r<1>), FUNC(mc68hc11m0_device::spdr_w<1>)); // SPDR2
	map(0x8b, 0x8b).r(FUNC(mc68hc11m0_device::opt4_r)).nopw(); // OPT4
}

/*****************************************************************************/

uint8_t mc68hc11_cpu_device::FETCH()
{
	return m_cache->read_byte(m_pc++);
}

uint16_t mc68hc11_cpu_device::FETCH16()
{
	uint16_t w;
	w = m_cache->read_word(m_pc);
	m_pc += 2;
	return w;
}

uint8_t mc68hc11_cpu_device::READ8(uint32_t address)
{
	if(address >= m_reg_position && (address - m_reg_position) < m_reg_block_size)
	{
		return m_io->read_byte(address-m_reg_position);
	}
	else if(address >= m_ram_position && address < m_ram_position+m_internal_ram_size)
	{
		return m_data->read_byte(address-m_ram_position);
	}
	return m_program->read_byte(address);
}

void mc68hc11_cpu_device::WRITE8(uint32_t address, uint8_t value)
{
	if(address >= m_reg_position && (address - m_reg_position) < m_reg_block_size)
	{
		m_io->write_byte(address-m_reg_position, value);
		return;
	}
	else if(address >= m_ram_position && address < m_ram_position+m_internal_ram_size)
	{
		m_data->write_byte(address-m_ram_position, value);
		return;
	}
	m_program->write_byte(address, value);
}

uint16_t mc68hc11_cpu_device::READ16(uint32_t address)
{
	return (READ8(address) << 8) | (READ8(address+1));
}

void mc68hc11_cpu_device::WRITE16(uint32_t address, uint16_t value)
{
	WRITE8(address+0, (value >> 8) & 0xff);
	WRITE8(address+1, (value >> 0) & 0xff);
}

/*****************************************************************************/


#include "hc11ops.hxx"
#include "hc11ops.h"

void mc68hc11_cpu_device::device_start()
{
	int i;

	/* clear the opcode tables */
	for(i=0; i < 256; i++) {
		hc11_optable[i] = &HC11OP(invalid);
		hc11_optable_page2[i] = &HC11OP(invalid);
		hc11_optable_page3[i] = &HC11OP(invalid);
		hc11_optable_page4[i] = &HC11OP(invalid);
	}
	/* fill the opcode tables */
	for(i=0; i < sizeof(hc11_opcode_list)/sizeof(hc11_opcode_list_struct); i++)
	{
		switch(hc11_opcode_list[i].page)
		{
			case 0x00:
				hc11_optable[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0x18:
				hc11_optable_page2[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0x1A:
				hc11_optable_page3[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0xCD:
				hc11_optable_page4[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
		}
	}

	m_program = &space(AS_PROGRAM);
	m_cache = m_program->cache<0, 0, ENDIANNESS_BIG>();
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	save_item(NAME(m_pc));
	save_item(NAME(m_ix));
	save_item(NAME(m_iy));
	save_item(NAME(m_sp));
	save_item(NAME(m_ppc));
	save_item(NAME(m_ccr));
	save_item(NAME(m_d.d8.a));
	save_item(NAME(m_d.d8.b));
	save_item(NAME(m_adctl));
	save_item(NAME(m_ad_channel));
	save_item(NAME(m_ram_position));
	save_item(NAME(m_reg_position));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_wait_state));
	save_item(NAME(m_stop_state));
	save_item(NAME(m_tflg1));
	save_item(NAME(m_tmsk1));
	save_item(NAME(m_toc1));
	save_item(NAME(m_tcnt));
//  save_item(NAME(m_por));
	save_item(NAME(m_pr));
	save_item(NAME(m_pactl));
	save_item(NAME(m_frc_base));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_dir));

	m_pc = 0;
	m_d.d16 = 0;
	m_ix = 0;
	m_iy = 0;
	m_sp = 0;
	m_ppc = 0;
	m_adctl = 0;
	m_ad_channel = 0;
	m_irq_state[0] = m_irq_state[1] = 0;
	m_ram_position = 0;
	m_reg_position = 0;
	m_tflg1 = 0;
	m_tmsk1 = 0;
	std::fill(std::begin(m_port_data), std::end(m_port_data), 0x00);

	state_add( HC11_PC, "PC", m_pc).formatstr("%04X");
	state_add( HC11_SP, "SP", m_sp).formatstr("%04X");
	state_add( HC11_A,  "A", m_d.d8.a).formatstr("%02X");
	state_add( HC11_B,  "B", m_d.d8.b).formatstr("%02X");
	state_add( HC11_IX, "IX", m_ix).formatstr("%04X");
	state_add( HC11_IY, "IY", m_iy).formatstr("%04X");

	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_ccr).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}


void mc68hc11_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				(m_ccr & CC_S) ? 'S' : '.',
				(m_ccr & CC_X) ? 'X' : '.',
				(m_ccr & CC_H) ? 'H' : '.',
				(m_ccr & CC_I) ? 'I' : '.',
				(m_ccr & CC_N) ? 'N' : '.',
				(m_ccr & CC_Z) ? 'Z' : '.',
				(m_ccr & CC_V) ? 'V' : '.',
				(m_ccr & CC_C) ? 'C' : '.');
			break;
	}
}


void mc68hc11_cpu_device::device_reset()
{
	m_pc = READ16(0xfffe);
	m_wait_state = 0;
	m_stop_state = 0;
	m_ccr = CC_X | CC_I | CC_S;
	init_w(m_init_value);
	m_toc1 = 0xffff;
	m_tcnt = 0xffff;
//  m_por = 1; // for first timer overflow / compare stuff
	m_pr = 3; // timer prescale
	m_pactl = 0;
	m_frc_base = 0;
	std::fill(std::begin(m_port_dir), std::end(m_port_dir), 0x00);
}

void mc68hc11a1_device::device_reset()
{
	mc68hc11_cpu_device::device_reset();

	m_port_data[0] &= 0x87;
	m_port_data[1] = 0x00;
	ddr_w<0>(0x78);
	ddr_w<1>(0xff);
}

void mc68hc11d0_device::device_reset()
{
	mc68hc11_cpu_device::device_reset();

	m_port_data[0] &= 0x7f;
	ddr_w<0>(0x70);
}

/*
IRQ table vectors:
0xffd6: SCI
0xffd8: SPI
0xffda: Pulse Accumulator Input Edge
0xffdc: Pulse Accumulator Overflow
0xffde: Timer Overflow
0xffe0: Timer Output Capture 5
0xffe2: Timer Output Capture 4
0xffe4: Timer Output Capture 3
0xffe6: Timer Output Capture 2
0xffe8: Timer Output Capture 1
0xffea: Timer Input Capture 3
0xffec: Timer Input Capture 2
0xffee: Timer Input Capture 1
0xfff0: Real Time Int
0xfff2: IRQ
0xfff4: XIRQ
0xfff6: SWI (Trap IRQ)
0xfff8: Illegal Opcode (NMI)
0xfffa: CO-Processor Fail
0xfffc: Clock Monitor
0xfffe: RESET
*/

void mc68hc11_cpu_device::check_irq_lines()
{
	if( m_irq_state[MC68HC11_IRQ_LINE]!=CLEAR_LINE && (!(m_ccr & CC_I)) )
	{
		uint16_t pc_vector;

		if(m_wait_state == 0)
		{
			PUSH16(m_pc);
			PUSH16(m_iy);
			PUSH16(m_ix);
			PUSH8(REG_A);
			PUSH8(REG_B);
			PUSH8(m_ccr);
		}
		pc_vector = READ16(0xfff2);
		SET_PC(pc_vector);
		m_ccr |= CC_I; //irq taken, mask the flag
		if(m_wait_state == 1) { m_wait_state = 2; }
		if(m_stop_state == 1) { m_stop_state = 2; }
		standard_irq_callback(MC68HC11_IRQ_LINE);
	}

	/* check timers here */
	{
		int divider = div_tab[m_pr & 3];
		uint64_t cur_time = total_cycles();
		uint32_t add = (cur_time - m_frc_base) / divider;

		if (add > 0)
		{
			for(uint32_t i=0;i<add;i++)
			{
				m_tcnt++;
				if(m_tcnt == m_toc1)
				{
					m_tflg1 |= 0x80;
					m_irq_state[MC68HC11_TOC1_LINE] = ASSERT_LINE;
				}
			}

			m_frc_base = cur_time;
		}
	}

	if( m_irq_state[MC68HC11_TOC1_LINE]!=CLEAR_LINE && (!(m_ccr & CC_I)) && m_tmsk1 & 0x80)
	{
		uint16_t pc_vector;

		if(m_wait_state == 0)
		{
			PUSH16(m_pc);
			PUSH16(m_iy);
			PUSH16(m_ix);
			PUSH8(REG_A);
			PUSH8(REG_B);
			PUSH8(m_ccr);
		}
		pc_vector = READ16(0xffe8);
		SET_PC(pc_vector);
		m_ccr |= CC_I; //irq taken, mask the flag
		if(m_wait_state == 1) { m_wait_state = 2; }
		if(m_stop_state == 1) { m_stop_state = 2; }
		standard_irq_callback(MC68HC11_TOC1_LINE);
		m_irq_state[MC68HC11_TOC1_LINE] = CLEAR_LINE; // auto-ack irq
	}

}

void mc68hc11_cpu_device::execute_set_input(int inputnum, int state)
{
	m_irq_state[inputnum] = state;
	if (state == CLEAR_LINE) return;
	check_irq_lines();
}

void mc68hc11_cpu_device::execute_run()
{
	while(m_icount > 0)
	{
		uint8_t op;

		check_irq_lines();

		m_ppc = m_pc;
		debugger_instruction_hook(m_pc);

		op = FETCH();
		(this->*hc11_optable[op])();
	}
}
