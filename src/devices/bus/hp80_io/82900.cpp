// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    82900.cpp

    82900 module (CP/M auxiliary processor)

*********************************************************************/

#include "emu.h"
#include "82900.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

hp82900_io_card_device::hp82900_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP82900_IO_CARD , tag , owner , clock),
	  device_hp80_io_interface(mconfig, *this),
	  m_cpu(*this , "cpu"),
	  m_translator(*this , "xlator"),
	  m_rom(*this , "rom")
{
}

hp82900_io_card_device::~hp82900_io_card_device()
{
}

void hp82900_io_card_device::install_read_write_handlers(address_space& space , uint16_t base_addr)
{
	space.install_readwrite_handler(base_addr, base_addr + 1, read8sm_delegate(*m_translator, FUNC(hp_1mb5_device::cpu_r)), write8sm_delegate(*m_translator, FUNC(hp_1mb5_device::cpu_w)));
}

void hp82900_io_card_device::inten()
{
	m_translator->inten();
}

void hp82900_io_card_device::clear_service()
{
	m_translator->clear_service();
}

static INPUT_PORTS_START(hp82900_port)
	PORT_START("SC")
	PORT_CONFNAME(0xf , 3 , "Select Code")
INPUT_PORTS_END

ioport_constructor hp82900_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp82900_port);
}

void hp82900_io_card_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(65536);

	save_pointer(NAME(m_ram) , 65536);
	save_item(NAME(m_rom_enabled));
	save_item(NAME(m_addr_latch));
}

void hp82900_io_card_device::device_reset()
{
	m_rom_enabled = true;
}

ROM_START(hp82900)
	ROM_REGION(0x800 , "rom" , 0)
	ROM_LOAD("82900-60002.bin" , 0 , 0x800 , CRC(48745bbb) SHA1(fb4427f729eedba5ac01809718b841c7bdd85e1f))
ROM_END

WRITE_LINE_MEMBER(hp82900_io_card_device::reset_w)
{
	LOG("reset_w %d\n" , state);
	m_cpu->set_input_line(INPUT_LINE_RESET , state);
	if (state) {
		// When reset is asserted, clear state
		device_reset();
	}
}

uint8_t hp82900_io_card_device::cpu_mem_r(offs_t offset)
{
	if (m_rom_enabled) {
		return m_rom[ offset & 0x7ff ];
	} else {
		return m_ram[ offset ];
	}
}

void hp82900_io_card_device::cpu_mem_w(offs_t offset, uint8_t data)
{
	m_ram[ offset ] = data;
}

uint8_t hp82900_io_card_device::cpu_io_r(offs_t offset)
{
	m_rom_enabled = false;

	uint8_t res;
	if (BIT(offset , 6) && (m_addr_latch & 0x82) == 0) {
		res = m_translator->uc_r(m_addr_latch & 1);
	} else {
		res = ~0;
	}
	return res;
}

void hp82900_io_card_device::cpu_io_w(offs_t offset, uint8_t data)
{
	m_rom_enabled = false;
	if (BIT(offset , 6) && (m_addr_latch & 0x82) == 0) {
		m_translator->uc_w(m_addr_latch & 1 , data);
	} else if (BIT(offset , 7)) {
		m_addr_latch = data;
	}
}

void hp82900_io_card_device::cpu_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000 , 0xffff).rw(FUNC(hp82900_io_card_device::cpu_mem_r) , FUNC(hp82900_io_card_device::cpu_mem_w));
}

void hp82900_io_card_device::cpu_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00 , 0xff).rw(FUNC(hp82900_io_card_device::cpu_io_r) , FUNC(hp82900_io_card_device::cpu_io_w));
}

void hp82900_io_card_device::z80_m1_w(uint8_t data)
{
	// 1 wait state on each M1 cycle
	m_cpu->adjust_icount(-1);
}

const tiny_rom_entry *hp82900_io_card_device::device_rom_region() const
{
	return ROM_NAME(hp82900);
}

void hp82900_io_card_device::device_add_mconfig(machine_config &config)
{
	Z80(config , m_cpu , XTAL(8'000'000) / 2);
	m_cpu->set_addrmap(AS_PROGRAM , &hp82900_io_card_device::cpu_mem_map);
	m_cpu->set_addrmap(AS_IO , &hp82900_io_card_device::cpu_io_map);
	m_cpu->refresh_cb().set(FUNC(hp82900_io_card_device::z80_m1_w));

	HP_1MB5(config, m_translator, 0);
	m_translator->irl_handler().set(FUNC(hp82900_io_card_device::irl_w));
	m_translator->halt_handler().set(FUNC(hp82900_io_card_device::halt_w));
	m_translator->reset_handler().set(FUNC(hp82900_io_card_device::reset_w));
	m_translator->int_handler().set([this](int state) { m_cpu->set_input_line(INPUT_LINE_IRQ0 , !state); });
}

// device type definition
DEFINE_DEVICE_TYPE(HP82900_IO_CARD, hp82900_io_card_device, "hp82900", "HP82900 card")
