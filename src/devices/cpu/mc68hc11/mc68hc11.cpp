// license:BSD-3-Clause
// copyright-holders:Ville Linde, Angelo Salese, AJR
/*
   Motorola MC68HC11 emulator

   Written by Ville Linde & Angelo Salese

TODO:
- Timers are really sketchy as per now;
- Complete opcodes hook-up;
- Emulate the MC68HC12 (same as HC11 with a bunch of new opcodes);

 */

#include "emu.h"
#include "mc68hc11.h"
#include "hc11dasm.h"

#include <tuple>

#define LOG_IRQ (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

enum
{
	HC11_PC = 1,
	HC11_SP,
	HC11_CCR,
	HC11_A,
	HC11_B,
	HC11_D,
	HC11_IX,
	HC11_IY,
	HC11_CONFIG,
	HC11_INIT,
	HC11_OPTION
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
DEFINE_DEVICE_TYPE(MC68HC11E1, mc68hc11e1_device, "mc68hc11e1", "Motorola MC68HC11E1")
DEFINE_DEVICE_TYPE(MC68HC811E2, mc68hc811e2_device, "mc68hc811e2", "Motorola MC68HC811E2")
DEFINE_DEVICE_TYPE(MC68HC11F1, mc68hc11f1_device, "mc68hc11f1", "Motorola MC68HC11F1")
DEFINE_DEVICE_TYPE(MC68HC11K1, mc68hc11k1_device, "mc68hc11k1", "Motorola MC68HC11K1")
DEFINE_DEVICE_TYPE(MC68HC11M0, mc68hc11m0_device, "mc68hc11m0", "Motorola MC68HC11M0")


mc68hc11_cpu_device::mc68hc11_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t ram_size, uint16_t reg_block_size, uint16_t rom_size, uint16_t eeprom_size, uint8_t init_value, uint8_t config_mask, uint8_t option_mask)
	: cpu_device(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this, (config_mask & 0xf9) != 0)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, address_map_constructor(FUNC(mc68hc11_cpu_device::internal_map), this))
	, m_irq_asserted(false)
	, m_port_input_cb(*this, 0xff)
	, m_port_output_cb(*this)
	, m_analog_cb(*this, 0)
	, m_spi2_data_input_cb(*this, 0xff)
	, m_spi2_data_output_cb(*this)
	, m_ram_view(*this, "ram")
	, m_reg_view(*this, "regs")
	, m_eeprom_view(*this, "eeprom")
	, m_eeprom_data(*this, "eeprom")
	, m_internal_ram_size(ram_size)
	, m_reg_block_size(reg_block_size)
	, m_internal_rom_size(rom_size)
	, m_internal_eeprom_size(eeprom_size)
	, m_init_value(init_value)
	, m_config_mask(config_mask)
	, m_option_mask(option_mask)
	, m_config(config_mask & (m_internal_rom_size == 0 ? 0xfd : 0xff) & (m_internal_eeprom_size == 0 ? 0xfe : 0xff))
{
}

void mc68hc11_cpu_device::internal_map(address_map &map)
{
	// TODO: internal ROM

	map(0x0000, 0xffff).view(m_ram_view);
	for (int pos = 0; pos < 16; pos++)
		m_ram_view[pos](pos << 12, (pos << 12) + m_internal_ram_size - 1).ram().share("ram");
	if (m_init_value == 0x00)
		for (int pos = 0; pos < 16; pos++)
			m_ram_view[pos + 16]((pos << 12) + m_reg_block_size, (pos << 12) + m_reg_block_size + m_internal_ram_size - 1).ram().share("ram");

	if (m_internal_eeprom_size != 0)
	{
		// TODO: allow EEPROM writes in programming mode only, latching address and data
		map(0x0000, 0xffff).view(m_eeprom_view);
		if ((m_config_mask & 0xf0) == 0)
			m_eeprom_view[0](0xb600, 0xb7ff).ram().share(m_eeprom_data);
		else
			for (int pos = 0; pos < 16; pos++)
				m_eeprom_view[pos](((pos + 1) << 12) - m_internal_eeprom_size, ((pos + 1) << 12) - 1).ram().share(m_eeprom_data);
	}

	map(0x0000, 0xffff).view(m_reg_view);
	for (int pos = 0; pos < 16; pos++)
	{
		m_reg_view[pos](pos << 12, (pos << 12) + m_reg_block_size - 1).unmaprw();
		mc68hc11_reg_map(m_reg_view[pos], pos << 12);
	}
}

mc68hc11a1_device::mc68hc11a1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11A1, tag, owner, clock, 256, 64, 0, 512, 0x01, 0x0f, 0xfb)
{
}

mc68hc11d0_device::mc68hc11d0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11D0, tag, owner, clock, 192, 64, 0, 0, 0x00, 0x06, 0x3b)
{
}

mc68hc11e1_device::mc68hc11e1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11E1, tag, owner, clock, 512, 64, 0, 512, 0x01, 0x0f, 0xfb)
{
}

mc68hc811e2_device::mc68hc811e2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC811E2, tag, owner, clock, 256, 64, 0, 2048, 0x01, 0xf5, 0xfb)
{
}

mc68hc11f1_device::mc68hc11f1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11F1, tag, owner, clock, 1024, 96, 0, 512, 0x01, 0xf5, 0xff)
{
	m_config &= 0x0f;
}

mc68hc11k1_device::mc68hc11k1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11K1, tag, owner, clock, 768, 128, 0, 640, 0x00, 0xbf, 0xff)
{
}

mc68hc11m0_device::mc68hc11m0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68hc11_cpu_device(mconfig, MC68HC11M0, tag, owner, clock, 1280, 256, 0, 0, 0x00, 0x06, 0xff)
{
}

device_memory_interface::space_config_vector mc68hc11_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

std::unique_ptr<util::disasm_interface> mc68hc11_cpu_device::create_disassembler()
{
	return std::make_unique<hc11_disassembler>();
}


bool mc68hc11_cpu_device::nvram_read(util::read_stream &file)
{
	std::error_condition err;
	size_t actual;

	std::tie(err, actual) = read(file, &m_eeprom_data[0], m_internal_eeprom_size);
	if (err || (actual != m_internal_eeprom_size))
		return false;

	std::tie(err, actual) = read(file, &m_config, 1);
	if (err || (actual != 1))
		return false;

	return true;
}

bool mc68hc11_cpu_device::nvram_write(util::write_stream &file)
{
	std::error_condition err;
	size_t actual;

	std::tie(err, actual) = write(file, &m_eeprom_data[0], m_internal_eeprom_size);
	if (err)
		return false;

	std::tie(err, actual) = write(file, &m_config, 1);
	if (err)
		return false;

	return true;
}

void mc68hc11_cpu_device::nvram_default()
{
	if ((m_config_mask & 0xf9) == 0)
		return;

	if (m_internal_eeprom_size != 0)
	{
		// use region if it exists
		memory_region *region = memregion("eeprom");
		if (region != nullptr)
			std::copy_n(&region->as_u8(), m_internal_eeprom_size, &m_eeprom_data[0]);
		else
			std::fill_n(&m_eeprom_data[0], m_internal_eeprom_size, 0xff);
	}
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
	m_pactl = data & 0x77;
}

uint8_t mc68hc11a1_device::pactl_ddra7_r()
{
	return (ddr_r<0>() & 0x80) | pactl_r();
}

void mc68hc11a1_device::pactl_ddra7_w(uint8_t data)
{
	pactl_w(data & 0x73);
	ddr_w<0>((data & 0x80) | 0x78);
}

uint8_t mc68hc11_cpu_device::pactl_ddra_r()
{
	return (ddr_r<0>() & 0x88) | pactl_r();
}

void mc68hc11_cpu_device::pactl_ddra_w(uint8_t data)
{
	pactl_w(data & 0x77);
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

uint8_t mc68hc11_cpu_device::toc_r(offs_t offset)
{
	return (m_toc[offset >> 1] >> (BIT(offset, 0) ? 0 : 8)) & 0xff;
}

void mc68hc11_cpu_device::toc_w(offs_t offset, uint8_t data)
{
	uint16_t &toc = m_toc[offset >> 1];
	if (BIT(offset, 0))
		toc = (data & 0xff) | (toc & 0xff00);
	else // TODO: inhibit count for one bus cycle
		toc = (data << 8) | (toc & 0xff);
}

uint8_t mc68hc11_cpu_device::tctl1_r()
{
	return m_tctl1;
}

void mc68hc11_cpu_device::tctl1_w(uint8_t data)
{
	m_tctl1 = data;
}

uint8_t mc68hc11_cpu_device::tctl2_r()
{
	return m_tctl2;
}

void mc68hc11_cpu_device::tctl2_w(uint8_t data)
{
	m_tctl2 = data;
}

uint8_t mc68hc11_cpu_device::tmsk1_r()
{
	return m_tmsk1;
}

void mc68hc11_cpu_device::tmsk1_w(uint8_t data)
{
	for (int i = 0; i < 5; i++)
		if (BIT(m_tflg1 & (m_tmsk1 ^ data), 7 - i))
			set_irq_state(0x0b + i, BIT(data, 7 - i));
	m_tmsk1 = data;
}

uint8_t mc68hc11_cpu_device::tflg1_r()
{
	return m_tflg1;
}

void mc68hc11_cpu_device::tflg1_w(uint8_t data)
{
	for (int i = 0; i < 5; i++)
		if (BIT(m_tflg1 & data, 7 - i))
			set_irq_state(0x0b + i, false);
	m_tflg1 &= ~data;
}

uint8_t mc68hc11_cpu_device::tflg2_r()
{
	return m_tflg2;
}

void mc68hc11_cpu_device::tflg2_w(uint8_t data)
{
	if (BIT(m_tflg2 & data, 7))
		set_irq_state(0x10, false);
	if (BIT(m_tflg2 & data, 6))
		set_irq_state(0x07, false);
	m_tflg2 &= ~data;
}

uint8_t mc68hc11_cpu_device::tmsk2_r()
{
	return m_tmsk2;
}

void mc68hc11_cpu_device::tmsk2_w(uint8_t data)
{
	if (BIT(m_tflg2 & (m_tmsk2 ^ data), 7))
		set_irq_state(0x10, BIT(data, 7));
	if (BIT(m_tflg2 & (m_tmsk2 ^ data), 6))
		set_irq_state(0x07, BIT(data, 6));

	// TODO: prescaler bits are time-protected
	m_tmsk2 = data;
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

uint8_t mc68hc11_cpu_device::config_r()
{
	return m_config;
}

uint8_t mc68hc11_cpu_device::config_1s_r()
{
	return m_config | ~m_config_mask;
}

void mc68hc11_cpu_device::config_w(uint8_t data)
{
	// TODO: CONFIG also has EEPROM protection
	m_config = data & m_config_mask;
	if (m_internal_eeprom_size != 0)
	{
		if (BIT(m_config, 0))
			m_eeprom_view.select(((m_config_mask & 0xf0) == 0xb0 ? m_init2 : m_config) >> 4);
		else
			m_eeprom_view.disable();
	}
}

uint8_t mc68hc11_cpu_device::init_r()
{
	return m_init;
}

void mc68hc11_cpu_device::init_w(uint8_t data)
{
	// TODO: only writeable during first 64 E cycles
	m_init = data;

	int reg_page = data & 0xf;
	int ram_page = (data >> 4) & 0xf;

	if (reg_page == ram_page && m_init_value == 0x00)
	{
		m_reg_view.select(reg_page);
		m_ram_view.select(ram_page + 16);
	}
	else
	{
		m_reg_view.select(reg_page);
		m_ram_view.select(ram_page);
	}
}

uint8_t mc68hc11_cpu_device::init2_r()
{
	return m_init2;
}

void mc68hc11_cpu_device::init2_w(uint8_t data)
{
	m_init2 = data & 0xf0;
	assert(m_internal_eeprom_size != 0);
	if (BIT(m_config, 0))
		m_eeprom_view.select(data >> 4);
}

uint8_t mc68hc11_cpu_device::option_r()
{
	return m_option;
}

void mc68hc11_cpu_device::option_w(uint8_t data)
{
	// TODO: only writeable during first 64 E cycles (except bits 7, 6, 3)
	m_option = data & m_option_mask;
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

uint8_t mc68hc11d0_device::reg01_r()
{
	// "Reserved" according to datasheet; asma2k reads from here due to a programming error and writes value back to TFLG1
	return 0xff;
}

void mc68hc11a1_device::mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base)
{
	block(base + 0x00, base + 0x00).rw(FUNC(mc68hc11a1_device::port_r<0>), FUNC(mc68hc11a1_device::port_w<0>)); // PORTA
	block(base + 0x02, base + 0x02).r(FUNC(mc68hc11a1_device::pioc_r)); // PIOC
	block(base + 0x03, base + 0x03).rw(FUNC(mc68hc11a1_device::port_r<2>), FUNC(mc68hc11a1_device::port_w<2>)); // PORTC
	block(base + 0x04, base + 0x04).rw(FUNC(mc68hc11a1_device::port_r<1>), FUNC(mc68hc11a1_device::port_w<1>)); // PORTB
	block(base + 0x05, base + 0x05).nopw(); // PORTCL
	block(base + 0x07, base + 0x07).rw(FUNC(mc68hc11a1_device::ddr_r<2>), FUNC(mc68hc11a1_device::ddr_w<2>)); // DDRC
	block(base + 0x08, base + 0x08).rw(FUNC(mc68hc11a1_device::port_r<3>), FUNC(mc68hc11a1_device::port_w<3>)); // PORTD
	block(base + 0x09, base + 0x09).rw(FUNC(mc68hc11a1_device::ddr_r<3>), FUNC(mc68hc11a1_device::ddr_w<3>)); // DDRD
	block(base + 0x0a, base + 0x0a).r(FUNC(mc68hc11a1_device::port_r<4>)); // PORTE
	block(base + 0x0e, base + 0x0f).rw(FUNC(mc68hc11a1_device::tcnt_r), FUNC(mc68hc11a1_device::tcnt_w)); // TCNT
	block(base + 0x16, base + 0x1f).rw(FUNC(mc68hc11a1_device::toc_r), FUNC(mc68hc11a1_device::toc_w)); // TOC1-TOC5
	block(base + 0x20, base + 0x20).rw(FUNC(mc68hc11a1_device::tctl1_r), FUNC(mc68hc11a1_device::tctl1_w)); // TCTL1
	block(base + 0x21, base + 0x21).rw(FUNC(mc68hc11a1_device::tctl2_r), FUNC(mc68hc11a1_device::tctl2_w)); // TCTL2
	block(base + 0x22, base + 0x22).rw(FUNC(mc68hc11a1_device::tmsk1_r), FUNC(mc68hc11a1_device::tmsk1_w)); // TMSK1
	block(base + 0x23, base + 0x23).rw(FUNC(mc68hc11a1_device::tflg1_r), FUNC(mc68hc11a1_device::tflg1_w)); // TFLG1
	block(base + 0x24, base + 0x24).rw(FUNC(mc68hc11a1_device::tmsk2_r), FUNC(mc68hc11a1_device::tmsk2_w)); // TMSK2
	block(base + 0x25, base + 0x25).rw(FUNC(mc68hc11a1_device::tflg2_r), FUNC(mc68hc11a1_device::tflg2_w)); // TFLG2
	block(base + 0x26, base + 0x26).rw(FUNC(mc68hc11a1_device::pactl_ddra7_r), FUNC(mc68hc11a1_device::pactl_ddra7_w)); // PACTL
	block(base + 0x28, base + 0x28).r(FUNC(mc68hc11a1_device::spcr_r<0>)).nopw(); // SPCR
	block(base + 0x29, base + 0x29).r(FUNC(mc68hc11a1_device::spsr_r<0>)).nopw(); // SPSR
	block(base + 0x2a, base + 0x2a).rw(FUNC(mc68hc11a1_device::spdr_r<0>), FUNC(mc68hc11a1_device::spdr_w<0>)); // SPDR
	block(base + 0x2c, base + 0x2c).r(FUNC(mc68hc11a1_device::sccr1_r)).nopw(); // SCCR1
	block(base + 0x2d, base + 0x2d).r(FUNC(mc68hc11a1_device::sccr2_r)).nopw(); // SCCR2
	block(base + 0x30, base + 0x30).rw(FUNC(mc68hc11a1_device::adctl_r), FUNC(mc68hc11a1_device::adctl_w)); // ADCTL
	block(base + 0x31, base + 0x34).r(FUNC(mc68hc11a1_device::adr_r)); // ADR1-ADR4
	block(base + 0x39, base + 0x39).rw(FUNC(mc68hc11a1_device::option_r), FUNC(mc68hc11a1_device::option_w)); // OPTION
	block(base + 0x3a, base + 0x3a).nopw(); // COPRST (watchdog)
	block(base + 0x3b, base + 0x3b).nopw(); // PPROG (EEPROM programming)
	block(base + 0x3d, base + 0x3d).rw(FUNC(mc68hc11a1_device::init_r), FUNC(mc68hc11a1_device::init_w)); // INIT
	block(base + 0x3f, base + 0x3f).rw(FUNC(mc68hc11a1_device::config_r), FUNC(mc68hc11a1_device::config_w)); // CONFIG
}

void mc68hc11d0_device::mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base)
{
	block(base + 0x00, base + 0x00).rw(FUNC(mc68hc11d0_device::port_r<0>), FUNC(mc68hc11d0_device::port_w<0>)); // PORTA
	block(base + 0x01, base + 0x01).r(FUNC(mc68hc11d0_device::reg01_r));
	block(base + 0x02, base + 0x02).r(FUNC(mc68hc11d0_device::pioc_r)); // PIOC
	block(base + 0x03, base + 0x03).rw(FUNC(mc68hc11d0_device::port_r<2>), FUNC(mc68hc11d0_device::port_w<2>)); // PORTC
	block(base + 0x04, base + 0x04).rw(FUNC(mc68hc11d0_device::port_r<1>), FUNC(mc68hc11d0_device::port_w<1>)); // PORTB
	block(base + 0x06, base + 0x06).rw(FUNC(mc68hc11d0_device::ddr_r<1>), FUNC(mc68hc11d0_device::ddr_w<1>)); // DDRB
	block(base + 0x07, base + 0x07).rw(FUNC(mc68hc11d0_device::ddr_r<2>), FUNC(mc68hc11d0_device::ddr_w<2>)); // DDRC
	block(base + 0x08, base + 0x08).rw(FUNC(mc68hc11d0_device::port_r<3>), FUNC(mc68hc11d0_device::port_w<3>)); // PORTD
	block(base + 0x09, base + 0x09).rw(FUNC(mc68hc11d0_device::ddr_r<3>), FUNC(mc68hc11d0_device::ddr_w<3>)); // DDRD
	block(base + 0x0e, base + 0x0f).rw(FUNC(mc68hc11d0_device::tcnt_r), FUNC(mc68hc11d0_device::tcnt_w)); // TCNT
	block(base + 0x16, base + 0x1f).rw(FUNC(mc68hc11d0_device::toc_r), FUNC(mc68hc11d0_device::toc_w)); // TOC1-TOC4, TI4/O5
	block(base + 0x20, base + 0x20).rw(FUNC(mc68hc11d0_device::tctl1_r), FUNC(mc68hc11d0_device::tctl1_w)); // TCTL1
	block(base + 0x21, base + 0x21).rw(FUNC(mc68hc11d0_device::tctl2_r), FUNC(mc68hc11d0_device::tctl2_w)); // TCTL2
	block(base + 0x22, base + 0x22).rw(FUNC(mc68hc11d0_device::tmsk1_r), FUNC(mc68hc11d0_device::tmsk1_w)); // TMSK1
	block(base + 0x23, base + 0x23).rw(FUNC(mc68hc11d0_device::tflg1_r), FUNC(mc68hc11d0_device::tflg1_w)); // TFLG1
	block(base + 0x24, base + 0x24).rw(FUNC(mc68hc11d0_device::tmsk2_r), FUNC(mc68hc11d0_device::tmsk2_w)); // TMSK2
	block(base + 0x25, base + 0x25).rw(FUNC(mc68hc11d0_device::tflg2_r), FUNC(mc68hc11d0_device::tflg2_w)); // TFLG2
	block(base + 0x26, base + 0x26).rw(FUNC(mc68hc11d0_device::pactl_ddra_r), FUNC(mc68hc11d0_device::pactl_ddra_w)); // PACTL
	block(base + 0x28, base + 0x28).r(FUNC(mc68hc11d0_device::spcr_r<0>)).nopw(); // SPCR
	block(base + 0x29, base + 0x29).r(FUNC(mc68hc11d0_device::spsr_r<0>)).nopw(); // SPSR
	block(base + 0x2a, base + 0x2a).rw(FUNC(mc68hc11d0_device::spdr_r<0>), FUNC(mc68hc11d0_device::spdr_w<0>)); // SPDR
	block(base + 0x2c, base + 0x2c).r(FUNC(mc68hc11d0_device::sccr1_r)).nopw(); // SCCR1
	block(base + 0x2d, base + 0x2d).r(FUNC(mc68hc11d0_device::sccr2_r)).nopw(); // SCCR2
	block(base + 0x2e, base + 0x2e).lr8([] { return 0xc0; }, "scsr_r").nopw(); // SCSR
	block(base + 0x2f, base + 0x2f).nopw(); // SCDR
	block(base + 0x39, base + 0x39).rw(FUNC(mc68hc11d0_device::option_r), FUNC(mc68hc11d0_device::option_w)); // OPTION
	block(base + 0x3a, base + 0x3a).nopw(); // COPRST (watchdog)
	block(base + 0x3d, base + 0x3d).rw(FUNC(mc68hc11d0_device::init_r), FUNC(mc68hc11d0_device::init_w)); // INIT
	block(base + 0x3f, base + 0x3f).rw(FUNC(mc68hc11d0_device::config_r), FUNC(mc68hc11d0_device::config_w)); // CONFIG
}

void mc68hc11e1_device::mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base)
{
	block(base + 0x00, base + 0x00).rw(FUNC(mc68hc11e1_device::port_r<0>), FUNC(mc68hc11e1_device::port_w<0>)); // PORTA
	block(base + 0x02, base + 0x02).r(FUNC(mc68hc11e1_device::pioc_r)); // PIOC
	block(base + 0x03, base + 0x03).rw(FUNC(mc68hc11e1_device::port_r<2>), FUNC(mc68hc11e1_device::port_w<2>)); // PORTC
	block(base + 0x04, base + 0x04).rw(FUNC(mc68hc11e1_device::port_r<1>), FUNC(mc68hc11e1_device::port_w<1>)); // PORTB
	block(base + 0x05, base + 0x05).nopw(); // PORTCL
	block(base + 0x07, base + 0x07).rw(FUNC(mc68hc11e1_device::ddr_r<2>), FUNC(mc68hc11e1_device::ddr_w<2>)); // DDRC
	block(base + 0x08, base + 0x08).rw(FUNC(mc68hc11e1_device::port_r<3>), FUNC(mc68hc11e1_device::port_w<3>)); // PORTD
	block(base + 0x09, base + 0x09).rw(FUNC(mc68hc11e1_device::ddr_r<3>), FUNC(mc68hc11e1_device::ddr_w<3>)); // DDRD
	block(base + 0x0a, base + 0x0a).r(FUNC(mc68hc11e1_device::port_r<4>)); // PORTE
	block(base + 0x0e, base + 0x0f).rw(FUNC(mc68hc11e1_device::tcnt_r), FUNC(mc68hc11e1_device::tcnt_w)); // TCNT
	block(base + 0x16, base + 0x1f).rw(FUNC(mc68hc11e1_device::toc_r), FUNC(mc68hc11e1_device::toc_w)); // TOC1-TOC4, TI4/O5
	block(base + 0x20, base + 0x20).rw(FUNC(mc68hc11e1_device::tctl1_r), FUNC(mc68hc11e1_device::tctl1_w)); // TCTL1
	block(base + 0x21, base + 0x21).rw(FUNC(mc68hc11e1_device::tctl2_r), FUNC(mc68hc11e1_device::tctl2_w)); // TCTL2
	block(base + 0x22, base + 0x22).rw(FUNC(mc68hc11e1_device::tmsk1_r), FUNC(mc68hc11e1_device::tmsk1_w)); // TMSK1
	block(base + 0x23, base + 0x23).rw(FUNC(mc68hc11e1_device::tflg1_r), FUNC(mc68hc11e1_device::tflg1_w)); // TFLG1
	block(base + 0x24, base + 0x24).rw(FUNC(mc68hc11e1_device::tmsk2_r), FUNC(mc68hc11e1_device::tmsk2_w)); // TMSK2
	block(base + 0x25, base + 0x25).rw(FUNC(mc68hc11e1_device::tflg2_r), FUNC(mc68hc11e1_device::tflg2_w)); // TFLG2
	block(base + 0x26, base + 0x26).rw(FUNC(mc68hc11e1_device::pactl_ddra_r), FUNC(mc68hc11e1_device::pactl_ddra_w)); // PACTL
	block(base + 0x28, base + 0x28).r(FUNC(mc68hc11e1_device::spcr_r<0>)).nopw(); // SPCR
	block(base + 0x29, base + 0x29).r(FUNC(mc68hc11e1_device::spsr_r<0>)).nopw(); // SPSR
	block(base + 0x2a, base + 0x2a).rw(FUNC(mc68hc11e1_device::spdr_r<0>), FUNC(mc68hc11e1_device::spdr_w<0>)); // SPDR
	block(base + 0x2c, base + 0x2c).r(FUNC(mc68hc11e1_device::sccr1_r)).nopw(); // SCCR1
	block(base + 0x2d, base + 0x2d).r(FUNC(mc68hc11e1_device::sccr2_r)).nopw(); // SCCR2
	block(base + 0x30, base + 0x30).rw(FUNC(mc68hc11e1_device::adctl_r), FUNC(mc68hc11e1_device::adctl_w)); // ADCTL
	block(base + 0x31, base + 0x34).r(FUNC(mc68hc11e1_device::adr_r)); // ADR1-ADR4
	block(base + 0x39, base + 0x39).rw(FUNC(mc68hc11e1_device::option_r), FUNC(mc68hc11e1_device::option_w)); // OPTION
	block(base + 0x3a, base + 0x3a).nopw(); // COPRST (watchdog)
	block(base + 0x3b, base + 0x3b).nopw(); // PPROG (EEPROM programming)
	block(base + 0x3d, base + 0x3d).rw(FUNC(mc68hc11e1_device::init_r), FUNC(mc68hc11e1_device::init_w)); // INIT
	block(base + 0x3f, base + 0x3f).rw(FUNC(mc68hc11e1_device::config_r), FUNC(mc68hc11e1_device::config_w)); // CONFIG
}

void mc68hc811e2_device::mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base)
{
	block(base + 0x00, base + 0x00).rw(FUNC(mc68hc811e2_device::port_r<0>), FUNC(mc68hc811e2_device::port_w<0>)); // PORTA
	block(base + 0x02, base + 0x02).r(FUNC(mc68hc811e2_device::pioc_r)); // PIOC
	block(base + 0x03, base + 0x03).rw(FUNC(mc68hc811e2_device::port_r<2>), FUNC(mc68hc811e2_device::port_w<2>)); // PORTC
	block(base + 0x04, base + 0x04).rw(FUNC(mc68hc811e2_device::port_r<1>), FUNC(mc68hc811e2_device::port_w<1>)); // PORTB
	block(base + 0x05, base + 0x05).nopw(); // PORTCL
	block(base + 0x07, base + 0x07).rw(FUNC(mc68hc811e2_device::ddr_r<2>), FUNC(mc68hc811e2_device::ddr_w<2>)); // DDRC
	block(base + 0x08, base + 0x08).rw(FUNC(mc68hc811e2_device::port_r<3>), FUNC(mc68hc811e2_device::port_w<3>)); // PORTD
	block(base + 0x09, base + 0x09).rw(FUNC(mc68hc811e2_device::ddr_r<3>), FUNC(mc68hc811e2_device::ddr_w<3>)); // DDRD
	block(base + 0x0a, base + 0x0a).r(FUNC(mc68hc811e2_device::port_r<4>)); // PORTE
	block(base + 0x0e, base + 0x0f).rw(FUNC(mc68hc811e2_device::tcnt_r), FUNC(mc68hc811e2_device::tcnt_w)); // TCNT
	block(base + 0x16, base + 0x1f).rw(FUNC(mc68hc811e2_device::toc_r), FUNC(mc68hc811e2_device::toc_w)); // TOC1-TOC4, TI4/O5
	block(base + 0x20, base + 0x20).rw(FUNC(mc68hc811e2_device::tctl1_r), FUNC(mc68hc811e2_device::tctl1_w)); // TCTL1
	block(base + 0x21, base + 0x21).rw(FUNC(mc68hc811e2_device::tctl2_r), FUNC(mc68hc811e2_device::tctl2_w)); // TCTL2
	block(base + 0x22, base + 0x22).rw(FUNC(mc68hc811e2_device::tmsk1_r), FUNC(mc68hc811e2_device::tmsk1_w)); // TMSK1
	block(base + 0x23, base + 0x23).rw(FUNC(mc68hc811e2_device::tflg1_r), FUNC(mc68hc811e2_device::tflg1_w)); // TFLG1
	block(base + 0x24, base + 0x24).rw(FUNC(mc68hc811e2_device::tmsk2_r), FUNC(mc68hc811e2_device::tmsk2_w)); // TMSK2
	block(base + 0x25, base + 0x25).rw(FUNC(mc68hc811e2_device::tflg2_r), FUNC(mc68hc811e2_device::tflg2_w)); // TFLG2
	block(base + 0x26, base + 0x26).rw(FUNC(mc68hc811e2_device::pactl_ddra_r), FUNC(mc68hc811e2_device::pactl_ddra_w)); // PACTL
	block(base + 0x28, base + 0x28).r(FUNC(mc68hc811e2_device::spcr_r<0>)).nopw(); // SPCR
	block(base + 0x29, base + 0x29).r(FUNC(mc68hc811e2_device::spsr_r<0>)).nopw(); // SPSR
	block(base + 0x2a, base + 0x2a).rw(FUNC(mc68hc811e2_device::spdr_r<0>), FUNC(mc68hc811e2_device::spdr_w<0>)); // SPDR
	block(base + 0x2c, base + 0x2c).r(FUNC(mc68hc811e2_device::sccr1_r)).nopw(); // SCCR1
	block(base + 0x2d, base + 0x2d).r(FUNC(mc68hc811e2_device::sccr2_r)).nopw(); // SCCR2
	block(base + 0x30, base + 0x30).rw(FUNC(mc68hc811e2_device::adctl_r), FUNC(mc68hc811e2_device::adctl_w)); // ADCTL
	block(base + 0x31, base + 0x34).r(FUNC(mc68hc811e2_device::adr_r)); // ADR1-ADR4
	block(base + 0x39, base + 0x39).rw(FUNC(mc68hc811e2_device::option_r), FUNC(mc68hc811e2_device::option_w)); // OPTION
	block(base + 0x3a, base + 0x3a).nopw(); // COPRST (watchdog)
	block(base + 0x3b, base + 0x3b).nopw(); // PPROG (EEPROM programming)
	block(base + 0x3d, base + 0x3d).rw(FUNC(mc68hc811e2_device::init_r), FUNC(mc68hc811e2_device::init_w)); // INIT
	block(base + 0x3f, base + 0x3f).rw(FUNC(mc68hc811e2_device::config_r), FUNC(mc68hc811e2_device::config_w)); // CONFIG
}

void mc68hc11f1_device::mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base)
{
	block(base + 0x00, base + 0x00).rw(FUNC(mc68hc11f1_device::port_r<0>), FUNC(mc68hc11f1_device::port_w<0>)); // PORTA
	block(base + 0x01, base + 0x01).rw(FUNC(mc68hc11f1_device::ddr_r<0>), FUNC(mc68hc11f1_device::ddr_w<0>)); // DDRA
	block(base + 0x02, base + 0x02).rw(FUNC(mc68hc11f1_device::port_r<6>), FUNC(mc68hc11f1_device::port_w<6>)); // PORTG
	block(base + 0x03, base + 0x03).rw(FUNC(mc68hc11f1_device::ddr_r<6>), FUNC(mc68hc11f1_device::ddr_w<6>)); // DDRG
	block(base + 0x04, base + 0x04).rw(FUNC(mc68hc11f1_device::port_r<1>), FUNC(mc68hc11f1_device::port_w<1>)); // PORTB
	block(base + 0x05, base + 0x05).rw(FUNC(mc68hc11f1_device::port_r<5>), FUNC(mc68hc11f1_device::port_w<5>)); // PORTF
	block(base + 0x06, base + 0x06).rw(FUNC(mc68hc11f1_device::port_r<2>), FUNC(mc68hc11f1_device::port_w<2>)); // PORTC
	block(base + 0x07, base + 0x07).rw(FUNC(mc68hc11f1_device::ddr_r<2>), FUNC(mc68hc11f1_device::ddr_w<2>)); // DDRC
	block(base + 0x08, base + 0x08).rw(FUNC(mc68hc11f1_device::port_r<3>), FUNC(mc68hc11f1_device::port_w<3>)); // PORTD
	block(base + 0x09, base + 0x09).rw(FUNC(mc68hc11f1_device::ddr_r<3>), FUNC(mc68hc11f1_device::ddr_w<3>)); // DDRD
	block(base + 0x0a, base + 0x0a).r(FUNC(mc68hc11f1_device::port_r<4>)); // PORTE
	block(base + 0x0b, base + 0x0b).nopw(); // CFORC
	block(base + 0x0c, base + 0x0c).nopw(); // OC1M
	block(base + 0x0d, base + 0x0d).nopw(); // OC1D
	block(base + 0x0e, base + 0x0f).rw(FUNC(mc68hc11f1_device::tcnt_r), FUNC(mc68hc11f1_device::tcnt_w)); // TCNT
	block(base + 0x10, base + 0x11).nopr(); // TIC1
	block(base + 0x12, base + 0x13).nopr(); // TIC2
	block(base + 0x14, base + 0x15).nopr(); // TIC3
	block(base + 0x16, base + 0x1f).rw(FUNC(mc68hc11f1_device::toc_r), FUNC(mc68hc11f1_device::toc_w)); // TOC1-TOC4, TI4/O5
	block(base + 0x20, base + 0x20).rw(FUNC(mc68hc11f1_device::tctl1_r), FUNC(mc68hc11f1_device::tctl1_w)); // TCTL1
	block(base + 0x21, base + 0x21).rw(FUNC(mc68hc11f1_device::tctl2_r), FUNC(mc68hc11f1_device::tctl2_w)); // TCTL2
	block(base + 0x22, base + 0x22).rw(FUNC(mc68hc11f1_device::tmsk1_r), FUNC(mc68hc11f1_device::tmsk1_w)); // TMSK1
	block(base + 0x23, base + 0x23).rw(FUNC(mc68hc11f1_device::tflg1_r), FUNC(mc68hc11f1_device::tflg1_w)); // TFLG1
	block(base + 0x24, base + 0x24).rw(FUNC(mc68hc11f1_device::tmsk2_r), FUNC(mc68hc11f1_device::tmsk2_w)); // TMSK2
	block(base + 0x25, base + 0x25).rw(FUNC(mc68hc11f1_device::tflg2_r), FUNC(mc68hc11f1_device::tflg2_w)); // TFLG2
	block(base + 0x26, base + 0x26).rw(FUNC(mc68hc11f1_device::pactl_r), FUNC(mc68hc11f1_device::pactl_w)); // PACTL
	block(base + 0x27, base + 0x27).nopw(); // PACNT
	block(base + 0x28, base + 0x28).r(FUNC(mc68hc11f1_device::spcr_r<0>)).nopw(); // SPCR
	block(base + 0x29, base + 0x29).r(FUNC(mc68hc11f1_device::spsr_r<0>)).nopw(); // SPSR
	block(base + 0x2a, base + 0x2a).rw(FUNC(mc68hc11f1_device::spdr_r<0>), FUNC(mc68hc11f1_device::spdr_w<0>)); // SPDR
	block(base + 0x2b, base + 0x2b).nopw(); // BAUD
	block(base + 0x2c, base + 0x2c).r(FUNC(mc68hc11f1_device::sccr1_r)).nopw(); // SCCR1
	block(base + 0x2d, base + 0x2d).r(FUNC(mc68hc11f1_device::sccr2_r)).nopw(); // SCCR2
	block(base + 0x2e, base + 0x2e).r(FUNC(mc68hc11f1_device::scsr1_r)); // SCSR
	block(base + 0x2f, base + 0x2f).nopw(); // SCDR
	block(base + 0x30, base + 0x30).rw(FUNC(mc68hc11f1_device::adctl_r), FUNC(mc68hc11f1_device::adctl_w)); // ADCTL
	block(base + 0x31, base + 0x34).r(FUNC(mc68hc11f1_device::adr_r)); // ADR1-ADR4
	block(base + 0x35, base + 0x35).nopw(); // BPROT
	block(base + 0x38, base + 0x38).nopw(); // OPT2
	block(base + 0x39, base + 0x39).rw(FUNC(mc68hc11f1_device::option_r), FUNC(mc68hc11f1_device::option_w)); // OPTION
	block(base + 0x3a, base + 0x3a).nopw(); // COPRST (watchdog)
	block(base + 0x3b, base + 0x3b).nopw(); // PPROG (EEPROM programming)
	block(base + 0x3c, base + 0x3c).nopw(); // HPRIO
	block(base + 0x3d, base + 0x3d).rw(FUNC(mc68hc11f1_device::init_r), FUNC(mc68hc11f1_device::init_w)); // INIT
	block(base + 0x3e, base + 0x3e).nopw(); // TEST1
	block(base + 0x3f, base + 0x3f).rw(FUNC(mc68hc11f1_device::config_1s_r), FUNC(mc68hc11f1_device::config_w)); // CONFIG
	block(base + 0x5c, base + 0x5c).nopw(); // CSSTRH
	block(base + 0x5d, base + 0x5d).nopw(); // CSSTRL
	block(base + 0x5e, base + 0x5e).nopw(); // CSGADR
	block(base + 0x5f, base + 0x5f).nopw(); // CSGSIZ
}

void mc68hc11k1_device::mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base)
{
	block(base + 0x00, base + 0x00).rw(FUNC(mc68hc11k1_device::port_r<0>), FUNC(mc68hc11k1_device::port_w<0>)); // PORTA
	block(base + 0x01, base + 0x01).rw(FUNC(mc68hc11k1_device::ddr_r<0>), FUNC(mc68hc11k1_device::ddr_w<0>)); // DDRA
	block(base + 0x02, base + 0x02).rw(FUNC(mc68hc11k1_device::ddr_r<1>), FUNC(mc68hc11k1_device::ddr_w<1>)); // DDRB
	block(base + 0x03, base + 0x03).rw(FUNC(mc68hc11k1_device::ddr_r<5>), FUNC(mc68hc11k1_device::ddr_w<5>)); // DDRF
	block(base + 0x04, base + 0x04).rw(FUNC(mc68hc11k1_device::port_r<1>), FUNC(mc68hc11k1_device::port_w<1>)); // PORTB
	block(base + 0x05, base + 0x05).rw(FUNC(mc68hc11k1_device::port_r<5>), FUNC(mc68hc11k1_device::port_w<5>)); // PORTF
	block(base + 0x06, base + 0x06).rw(FUNC(mc68hc11k1_device::port_r<2>), FUNC(mc68hc11k1_device::port_w<2>)); // PORTC
	block(base + 0x07, base + 0x07).rw(FUNC(mc68hc11k1_device::ddr_r<2>), FUNC(mc68hc11k1_device::ddr_w<2>)); // DDRC
	block(base + 0x08, base + 0x08).rw(FUNC(mc68hc11k1_device::port_r<3>), FUNC(mc68hc11k1_device::port_w<3>)); // PORTD
	block(base + 0x09, base + 0x09).rw(FUNC(mc68hc11k1_device::ddr_r<3>), FUNC(mc68hc11k1_device::ddr_w<3>)); // DDRD
	block(base + 0x0a, base + 0x0a).r(FUNC(mc68hc11k1_device::port_r<4>)); // PORTE
	block(base + 0x0e, base + 0x0f).rw(FUNC(mc68hc11k1_device::tcnt_r), FUNC(mc68hc11k1_device::tcnt_w)); // TCNT
	block(base + 0x16, base + 0x1f).rw(FUNC(mc68hc11k1_device::toc_r), FUNC(mc68hc11k1_device::toc_w)); // TOC1-TOC4, TI4/O5
	block(base + 0x20, base + 0x20).rw(FUNC(mc68hc11k1_device::tctl1_r), FUNC(mc68hc11k1_device::tctl1_w)); // TCTL1
	block(base + 0x21, base + 0x21).rw(FUNC(mc68hc11k1_device::tctl2_r), FUNC(mc68hc11k1_device::tctl2_w)); // TCTL2
	block(base + 0x22, base + 0x22).rw(FUNC(mc68hc11k1_device::tmsk1_r), FUNC(mc68hc11k1_device::tmsk1_w)); // TMSK1
	block(base + 0x23, base + 0x23).rw(FUNC(mc68hc11k1_device::tflg1_r), FUNC(mc68hc11k1_device::tflg1_w)); // TFLG1
	block(base + 0x24, base + 0x24).rw(FUNC(mc68hc11k1_device::tmsk2_r), FUNC(mc68hc11k1_device::tmsk2_w)); // TMSK2
	block(base + 0x25, base + 0x25).rw(FUNC(mc68hc11k1_device::tflg2_r), FUNC(mc68hc11k1_device::tflg2_w)); // TFLG2
	block(base + 0x26, base + 0x26).rw(FUNC(mc68hc11k1_device::pactl_r), FUNC(mc68hc11k1_device::pactl_w)); // PACTL
	block(base + 0x28, base + 0x28).r(FUNC(mc68hc11k1_device::spcr_r<0>)).nopw(); // SPCR
	block(base + 0x29, base + 0x29).r(FUNC(mc68hc11k1_device::spsr_r<0>)).nopw(); // SPSR
	block(base + 0x2a, base + 0x2a).rw(FUNC(mc68hc11k1_device::spdr_r<0>), FUNC(mc68hc11k1_device::spdr_w<0>)); // SPDR
	block(base + 0x30, base + 0x30).rw(FUNC(mc68hc11k1_device::adctl_r), FUNC(mc68hc11k1_device::adctl_w)); // ADCTL
	block(base + 0x31, base + 0x34).r(FUNC(mc68hc11k1_device::adr_r)); // ADR1-ADR4
	block(base + 0x37, base + 0x37).rw(FUNC(mc68hc11k1_device::init2_r), FUNC(mc68hc11k1_device::init2_w)); // INIT2
	block(base + 0x38, base + 0x38).r(FUNC(mc68hc11k1_device::opt2_r)).nopw(); // OPT2
	block(base + 0x39, base + 0x39).rw(FUNC(mc68hc11k1_device::option_r), FUNC(mc68hc11k1_device::option_w)); // OPTION
	block(base + 0x3a, base + 0x3a).nopw(); // COPRST (watchdog)
	block(base + 0x3b, base + 0x3b).nopw(); // PPROG (EEPROM programming)
	block(base + 0x3d, base + 0x3d).rw(FUNC(mc68hc11k1_device::init_r), FUNC(mc68hc11k1_device::init_w)); // INIT
	block(base + 0x3f, base + 0x3f).rw(FUNC(mc68hc11k1_device::config_1s_r), FUNC(mc68hc11k1_device::config_w)); // CONFIG
	block(base + 0x70, base + 0x71).r(FUNC(mc68hc11k1_device::scbd_r)).nopw(); // SCBD
	block(base + 0x72, base + 0x72).r(FUNC(mc68hc11k1_device::sccr1_r)).nopw(); // SCCR1
	block(base + 0x73, base + 0x73).r(FUNC(mc68hc11k1_device::sccr2_r)).nopw(); // SCCR2
	block(base + 0x74, base + 0x74).r(FUNC(mc68hc11k1_device::scsr1_r)).nopw(); // SCSR1
	block(base + 0x77, base + 0x77).r(FUNC(mc68hc11k1_device::scrdl_r)).nopw(); // SCRDL
	block(base + 0x7c, base + 0x7c).rw(FUNC(mc68hc11k1_device::port_r<7>), FUNC(mc68hc11k1_device::port_w<7>)); // PORTH
	block(base + 0x7d, base + 0x7d).rw(FUNC(mc68hc11k1_device::ddr_r<7>), FUNC(mc68hc11k1_device::ddr_w<7>)); // DDRH
	block(base + 0x7e, base + 0x7e).rw(FUNC(mc68hc11k1_device::port_r<6>), FUNC(mc68hc11k1_device::port_w<6>)); // PORTG
	block(base + 0x7f, base + 0x7f).rw(FUNC(mc68hc11k1_device::ddr_r<6>), FUNC(mc68hc11k1_device::ddr_w<6>)); // DDRG
}

void mc68hc11m0_device::mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base)
{
	block(base + 0x00, base + 0x00).rw(FUNC(mc68hc11m0_device::port_r<0>), FUNC(mc68hc11m0_device::port_w<0>)); // PORTA
	block(base + 0x01, base + 0x01).rw(FUNC(mc68hc11m0_device::ddr_r<0>), FUNC(mc68hc11m0_device::ddr_w<0>)); // DDRA
	block(base + 0x02, base + 0x02).rw(FUNC(mc68hc11m0_device::ddr_r<1>), FUNC(mc68hc11m0_device::ddr_w<1>)); // DDRB
	block(base + 0x03, base + 0x03).rw(FUNC(mc68hc11m0_device::ddr_r<5>), FUNC(mc68hc11m0_device::ddr_w<5>)); // DDRF
	block(base + 0x04, base + 0x04).rw(FUNC(mc68hc11m0_device::port_r<1>), FUNC(mc68hc11m0_device::port_w<1>)); // PORTB
	block(base + 0x05, base + 0x05).rw(FUNC(mc68hc11m0_device::port_r<5>), FUNC(mc68hc11m0_device::port_w<5>)); // PORTF
	block(base + 0x06, base + 0x06).rw(FUNC(mc68hc11m0_device::port_r<2>), FUNC(mc68hc11m0_device::port_w<2>)); // PORTC
	block(base + 0x07, base + 0x07).rw(FUNC(mc68hc11m0_device::ddr_r<2>), FUNC(mc68hc11m0_device::ddr_w<2>)); // DDRC
	block(base + 0x08, base + 0x08).rw(FUNC(mc68hc11m0_device::port_r<3>), FUNC(mc68hc11m0_device::port_w<3>)); // PORTD
	block(base + 0x09, base + 0x09).rw(FUNC(mc68hc11m0_device::ddr_r<3>), FUNC(mc68hc11m0_device::ddr_w<3>)); // DDRD
	block(base + 0x0a, base + 0x0a).r(FUNC(mc68hc11m0_device::port_r<4>)); // PORTE
	block(base + 0x0e, base + 0x0f).rw(FUNC(mc68hc11m0_device::tcnt_r), FUNC(mc68hc11m0_device::tcnt_w)); // TCNT
	block(base + 0x16, base + 0x1f).rw(FUNC(mc68hc11m0_device::toc_r), FUNC(mc68hc11m0_device::toc_w)); // TOC1-TOC4, TI4/O5
	block(base + 0x20, base + 0x20).rw(FUNC(mc68hc11m0_device::tctl1_r), FUNC(mc68hc11m0_device::tctl1_w)); // TCTL1
	block(base + 0x21, base + 0x21).rw(FUNC(mc68hc11m0_device::tctl2_r), FUNC(mc68hc11m0_device::tctl2_w)); // TCTL2
	block(base + 0x22, base + 0x22).rw(FUNC(mc68hc11m0_device::tmsk1_r), FUNC(mc68hc11m0_device::tmsk1_w)); // TMSK1
	block(base + 0x23, base + 0x23).rw(FUNC(mc68hc11m0_device::tflg1_r), FUNC(mc68hc11m0_device::tflg1_w)); // TFLG1
	block(base + 0x24, base + 0x24).rw(FUNC(mc68hc11m0_device::tmsk2_r), FUNC(mc68hc11m0_device::tmsk2_w)); // TMSK2
	block(base + 0x25, base + 0x25).rw(FUNC(mc68hc11m0_device::tflg2_r), FUNC(mc68hc11m0_device::tflg2_w)); // TFLG2
	block(base + 0x26, base + 0x26).rw(FUNC(mc68hc11m0_device::pactl_r), FUNC(mc68hc11m0_device::pactl_w)); // PACTL
	block(base + 0x28, base + 0x28).r(FUNC(mc68hc11m0_device::spcr_r<0>)).nopw(); // SPCR1
	block(base + 0x29, base + 0x29).r(FUNC(mc68hc11m0_device::spsr_r<0>)).nopw(); // SPSR1
	block(base + 0x2a, base + 0x2a).rw(FUNC(mc68hc11m0_device::spdr_r<0>), FUNC(mc68hc11m0_device::spdr_w<0>)); // SPDR1
	block(base + 0x30, base + 0x30).rw(FUNC(mc68hc11m0_device::adctl_r), FUNC(mc68hc11m0_device::adctl_w)); // ADCTL
	block(base + 0x31, base + 0x34).r(FUNC(mc68hc11m0_device::adr_r)); // ADR1-ADR4
	block(base + 0x38, base + 0x38).r(FUNC(mc68hc11m0_device::opt2_r)).nopw(); // OPT2
	block(base + 0x39, base + 0x39).rw(FUNC(mc68hc11m0_device::option_r), FUNC(mc68hc11m0_device::option_w)); // OPTION
	block(base + 0x3a, base + 0x3a).nopw(); // COPRST (watchdog)
	block(base + 0x3d, base + 0x3d).rw(FUNC(mc68hc11m0_device::init_r), FUNC(mc68hc11m0_device::init_w)); // INIT
	block(base + 0x3f, base + 0x3f).rw(FUNC(mc68hc11m0_device::config_r), FUNC(mc68hc11m0_device::config_w)); // CONFIG
	block(base + 0x70, base + 0x71).r(FUNC(mc68hc11m0_device::scbd_r)).nopw(); // SCBD
	block(base + 0x72, base + 0x72).r(FUNC(mc68hc11m0_device::sccr1_r)).nopw(); // SCCR1
	block(base + 0x73, base + 0x73).r(FUNC(mc68hc11m0_device::sccr2_r)).nopw(); // SCCR2
	block(base + 0x74, base + 0x74).r(FUNC(mc68hc11m0_device::scsr1_r)); // SCSR1
	block(base + 0x77, base + 0x77).r(FUNC(mc68hc11m0_device::scrdl_r)).nopw(); // SCRDL
	block(base + 0x7c, base + 0x7c).rw(FUNC(mc68hc11m0_device::port_r<7>), FUNC(mc68hc11m0_device::port_w<7>)); // PORTH
	block(base + 0x7d, base + 0x7d).rw(FUNC(mc68hc11m0_device::ddr_r<7>), FUNC(mc68hc11m0_device::ddr_w<7>)); // DDRH
	block(base + 0x7e, base + 0x7e).rw(FUNC(mc68hc11m0_device::port_r<6>), FUNC(mc68hc11m0_device::port_w<6>)); // PORTG
	block(base + 0x7f, base + 0x7f).rw(FUNC(mc68hc11m0_device::ddr_r<6>), FUNC(mc68hc11m0_device::ddr_w<6>)); // DDRG
	block(base + 0x88, base + 0x88).r(FUNC(mc68hc11m0_device::spcr_r<1>)).nopw(); // SPCR2
	block(base + 0x89, base + 0x89).r(FUNC(mc68hc11m0_device::spsr_r<1>)).nopw(); // SPSR2
	block(base + 0x8a, base + 0x8a).rw(FUNC(mc68hc11m0_device::spdr_r<1>), FUNC(mc68hc11m0_device::spdr_w<1>)); // SPDR2
	block(base + 0x8b, base + 0x8b).r(FUNC(mc68hc11m0_device::opt4_r)).nopw(); // OPT4
}

/*****************************************************************************/

uint8_t mc68hc11_cpu_device::FETCH()
{
	return m_cache.read_byte(m_pc++);
}

uint16_t mc68hc11_cpu_device::FETCH16()
{
	uint16_t w;
	w = m_cache.read_word(m_pc);
	m_pc += 2;
	return w;
}

uint8_t mc68hc11_cpu_device::READ8(uint32_t address)
{
	return m_program.read_byte(address);
}

void mc68hc11_cpu_device::WRITE8(uint32_t address, uint8_t value)
{
	m_program.write_byte(address, value);
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

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

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
	save_item(NAME(m_init));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_asserted));
	save_item(NAME(m_wait_state));
	save_item(NAME(m_stop_state));
	save_item(NAME(m_tctl1));
	save_item(NAME(m_tctl2));
	save_item(NAME(m_tflg1));
	save_item(NAME(m_tmsk1));
	save_item(NAME(m_toc));
	save_item(NAME(m_tcnt));
//  save_item(NAME(m_por));
	save_item(NAME(m_tflg2));
	save_item(NAME(m_tmsk2));
	save_item(NAME(m_pactl));
	save_item(NAME(m_frc_base));
	save_item(NAME(m_reset_time));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_dir));
	save_item(NAME(m_config));
	save_item(NAME(m_option));
	if ((m_config_mask & 0xf0) == 0xb0)
		save_item(NAME(m_init2));

	m_pc = 0;
	m_d.d16 = 0;
	m_ix = 0;
	m_iy = 0;
	m_sp = 0;
	m_ppc = 0;
	m_adctl = 0;
	m_ad_channel = 0;
	m_irq_state = 0;
	m_init = 0;
	m_init2 = 0;
	m_option = 0;
	m_tflg1 = 0;
	m_tmsk1 = 0;
	std::fill(std::begin(m_port_data), std::end(m_port_data), 0x00);

	state_add( HC11_PC,  "PC", m_pc);
	state_add( HC11_SP,  "SP", m_sp);
	state_add( HC11_CCR, "CCR", m_ccr);
	state_add( HC11_A,   "A", m_d.d8.a);
	state_add( HC11_B,   "B", m_d.d8.b);
	state_add( HC11_D,   "D", m_d.d16).noshow();
	state_add( HC11_IX,  "IX", m_ix);
	state_add( HC11_IY,  "IY", m_iy);

	using namespace std::placeholders;
	state_add( HC11_CONFIG, "CONFIG", m_config, std::bind(&mc68hc11_cpu_device::config_w, this, _1)).mask(m_config_mask).formatstr("%02X");
	state_add( HC11_INIT, "INIT", m_init, std::bind(&mc68hc11_cpu_device::init_w, this, _1));
	state_add( HC11_OPTION, "OPTION", m_option).mask(m_option_mask);

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
	if ((m_config_mask & 0xf9) == 0)
	{
		m_config = m_config_mask;
		if (m_internal_rom_size == 0)
			m_config &= 0xfd;
	}
	else
	{
		if ((m_config_mask & 0xf0) == 0xb0)
			m_init2 = 0;
		if (m_internal_eeprom_size != 0)
		{
			if (BIT(m_config, 0))
				m_eeprom_view.select(m_config_mask < 0xf0 ? 0 : m_config >> 4);
			else
				m_eeprom_view.disable();
		}
	}

	m_wait_state = 1;
	m_stop_state = 0;
	m_ccr = CC_X | CC_I | CC_S;
	init_w(m_init_value);
	m_option = 0x10;
	std::fill(std::begin(m_toc), std::end(m_toc), 0xffff);
	m_tcnt = 0xffff;
//  m_por = 1; // for first timer overflow / compare stuff
	m_tctl1 = 0;
	m_tctl2 = 0;
	m_tflg1 = 0;
	m_tflg2 = 0;
	m_tmsk2 = 3; // timer prescale
	m_pactl = 0;
	m_irq_state = 0x80000000 | (m_irq_state & 0x04000000);
	if (m_irq_asserted)
		set_irq_state(0x06, true);
	m_frc_base = m_reset_time = total_cycles();
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

	m_port_data[0] &= 0x8f;
	ddr_w<0>(0x70);
}

void mc68hc11e1_device::device_reset()
{
	mc68hc11_cpu_device::device_reset();

	m_port_data[0] &= 0x8f;
	m_port_data[1] = 0x00;
	ddr_w<0>(0x70);
	ddr_w<1>(0xff);
}

void mc68hc811e2_device::device_reset()
{
	mc68hc11_cpu_device::device_reset();

	m_port_data[0] &= 0x8f;
	m_port_data[1] = 0x00;
	ddr_w<0>(0x70);
	ddr_w<1>(0xff);
}

void mc68hc11f1_device::device_reset()
{
	mc68hc11_cpu_device::device_reset();

	m_option = 0x00;
}

static const char *const s_irq_names[32] =
{
	"RESET",            // vectored from $FFFE,FF
	"Clock Monitor",    // vectored from $FFFC,FD
	"COP Failure",      // vectored from $FFFA,FB
	"Illegal Opcode",   // vectored from $FFF8,F9
	"SWI",              // vectored from $FFF6,F7
	"XIRQ",             // vectored from $FFF4,F5
	"IRQ",              // vectored from $FFF2,F3
	"RTI",              // vectored from $FFF0,F1
	"IC1",              // vectored from $FFEE,EF
	"IC2",              // vectored from $FFEC,ED
	"IC3",              // vectored from $FFEA,EB
	"OC1",              // vectored from $FFE8,E9
	"OC2",              // vectored from $FFE6,E7
	"OC3",              // vectored from $FFE4,E5
	"OC4",              // vectored from $FFE2,E3
	"OC5",              // vectored from $FFE0,E1
	"Timer Overflow",   // vectored from $FFDE,DF
	"PAOV",             // vectored from $FFDC,DD
	"PAI",              // vectored from $FFDA,DB
	"SPI",              // vectored from $FFD8,D9
	"SCI",              // vectored from $FFD6,F7
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void mc68hc11_cpu_device::check_irq_lines()
{
	/* check timers here */
	{
		int divider = div_tab[m_tmsk2 & 3];
		uint64_t cur_time = total_cycles();
		uint32_t add = (cur_time - m_frc_base) / divider;

		if (add > 0)
		{
			if ((((cur_time - m_reset_time) ^ (m_frc_base - m_reset_time)) >> ((m_pactl & 3) + 13)) > 0)
			{
				m_tflg2 |= 0x40;
				if (BIT(m_tmsk2, 6))
					set_irq_state(0x07, true);
			}

			for (int i = 0; i < 5; i++)
			{
				if (add >= uint16_t(m_toc[i] - m_tcnt))
				{
					m_tflg1 |= 0x80 >> i;
					if (BIT(m_tmsk1, 7 - i))
						set_irq_state(0x0b + i, true);
				}
			}

			if (add >= 0x10000 - m_tcnt)
			{
				m_tflg2 |= 0x80;
				if (BIT(m_tmsk2, 7))
					set_irq_state(0x10, true);
			}

			m_tcnt += add;
			m_frc_base = cur_time;
		}
	}

	uint32_t irq_state = m_irq_state;
	if (m_ccr & CC_X)
		irq_state &= ~0x04000000; // mask XIRQ out
	if (irq_state != 0 && (!(m_ccr & CC_I) || (irq_state >= 0x04000000)))
	{
		int level = count_leading_zeros_32(irq_state); // TODO: respect HPRIO setting
		standard_irq_callback(level, m_pc);

		if(m_wait_state == 0)
		{
			PUSH16(m_pc);
			PUSH16(m_iy);
			PUSH16(m_ix);
			PUSH8(REG_A);
			PUSH8(REG_B);
			PUSH8(m_ccr);
		}
		// TODO: vectors differ in bootstrap and special test modes
		uint16_t pc_vector = READ16(0xfffe - level * 2);
		SET_PC(pc_vector);
		m_ccr |= CC_I; //irq taken, mask the flag
		if (level < 0x06)
			m_ccr |= CC_X;
		if(m_wait_state == 1) { m_wait_state = 0; }
		if(m_stop_state == 1) { m_stop_state = 2; }
		if (level < 0x05 || (level == 0x06 && BIT(m_option, 5)))
			set_irq_state(level, false); // auto-ack edge-triggered IRQ
	}
}

void mc68hc11_cpu_device::set_irq_state(uint8_t irqn, bool state)
{
	LOGMASKED(LOG_IRQ, "%s: %s interrupt %s\n", machine().describe_context(), s_irq_names[irqn], state ? "requested" : "cleared");
	if (state)
		m_irq_state |= 0x80000000 >> irqn;
	else
		m_irq_state &= ~(0x80000000 >> irqn);
}

void mc68hc11_cpu_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case MC68HC11_IRQ_LINE:
		if (!m_irq_asserted && state != CLEAR_LINE)
			set_irq_state(0x06, true);
		else if (m_irq_asserted && state == CLEAR_LINE && !BIT(m_option, 5))
			set_irq_state(0x06, false);
		m_irq_asserted = state != CLEAR_LINE;
		break;

	case MC68HC11_XIRQ_LINE:
		set_irq_state(0x05, state != CLEAR_LINE);
		break;

	default:
		logerror("Unknown input %d = %d\n", inputnum, state);
		break;
	}
}

void mc68hc11_cpu_device::execute_run()
{
	while(m_icount > 0)
	{
		uint8_t op;

		check_irq_lines();

		if (m_wait_state != 0)
		{
			debugger_wait_hook();
			m_icount = 0;
		}
		else
		{
			m_ppc = m_pc;
			debugger_instruction_hook(m_pc);

			op = FETCH();
			(this->*hc11_optable[op])();
		}
	}
}
