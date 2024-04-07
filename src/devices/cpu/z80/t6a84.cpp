// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Toshiba T6A84, TLCS-Z80 ASSP Family

***************************************************************************/

#include "emu.h"
#include "t6a84.h"

#define LOG_PAGE_R (1U << 1)
#define LOG_PAGE_W (1U << 2)
#define LOG_MEM    (1U << 3)

//#define VERBOSE (LOG_PAGE_R | LOG_PAGE_W | LOG_MEM)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(T6A84, t6a84_device, "t6a84", "Toshiba T6A84")

void t6a84_device::internal_io_map(address_map &map) const
{
	map.global_mask(0xff);
	map(0xfc, 0xfc).rw(FUNC(t6a84_device::stack_page_r), FUNC(t6a84_device::stack_page_w));
	map(0xfd, 0xfd).rw(FUNC(t6a84_device::data_page_r), FUNC(t6a84_device::data_page_w));
	map(0xfe, 0xfe).rw(FUNC(t6a84_device::code_page_r), FUNC(t6a84_device::code_page_w));
	map(0xff, 0xff).rw(FUNC(t6a84_device::vector_page_r), FUNC(t6a84_device::vector_page_w));
}

t6a84_device::t6a84_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	t6a84_device(mconfig, T6A84, tag, owner, clock, address_map_constructor(FUNC(t6a84_device::internal_io_map), this))
{ }

t6a84_device::t6a84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor io_map)
	: z80_device(mconfig, type, tag, owner, clock)
	, m_program_space_config("program", ENDIANNESS_LITTLE, 8, 20, 0, 16, 0)
	, m_data_space_config("data", ENDIANNESS_LITTLE, 8, 20, 0, 16, 0)
	, m_stack_space_config("stack", ENDIANNESS_LITTLE, 8, 20, 0, 16, 0)
	, m_io_space_config("io", ENDIANNESS_LITTLE, 8, 16, 0, io_map)
	, m_code_page(0)
	, m_delay_code_page(0)
	, m_is_delay_code_page_set(false)
	, m_prev_code_page(0)
	, m_data_page(8)
	, m_stack_page(8)
	, m_vector_page(0)
{
	// Interrupt vectors need to be fetched and executed from their corresponding page.
	// For simplicity, we switch pages via callbacks, instead of using a dedicated address space.
	irqfetch_cb().set([this](int state) {
		LOGMASKED(LOG_PAGE_W, "IRQ FETCH %02x => %02x\n", m_code_page, m_vector_page);
		m_prev_code_page = m_code_page;
		m_code_page = m_vector_page;
	});
	reti_cb().set([this](int state) {
		LOGMASKED(LOG_PAGE_W, "IRQ RET %02x => %02x\n", m_code_page, m_prev_code_page);
		m_code_page = m_prev_code_page;
	});
	branch_cb().set([this](int state) {
		LOGMASKED(LOG_PAGE_W, "BRANCH %02x => %02x\n", m_code_page, m_prev_code_page);
		/*
			When setting a code page, it only becomes effective after jumping to a far address in that page.
			Any instructions fetched and executed before that jump still use the previous code page.
			This can be seen in Sega Ferie Kitten, when test program at page 7 gets mapped, as we are still
			executing on page 0, but we expect to start executing that program when jumping to RST0:

			ROM_00::1ea9 3e 07      LD   A,0x7
			ROM_00::1eab d3 fe      OUT  (DAT_io_00fe),A
			ROM_00::1ead c3 00 00   JP   RST0
		*/
		if (!machine().side_effects_disabled() && m_is_delay_code_page_set) {
			m_code_page = m_delay_code_page;
			m_is_delay_code_page_set = false;
		}
	});
}

void t6a84_device::device_start()
{
	z80_device::device_start();

	space(AS_PROGRAM).cache(m_args);
	space(AS_DATA).specific(m_data);
	space(AS_STACK).specific(m_stack);

	save_item(NAME(m_code_page));
	save_item(NAME(m_delay_code_page));
	save_item(NAME(m_is_delay_code_page_set));
	save_item(NAME(m_prev_code_page));
	save_item(NAME(m_data_page));
	save_item(NAME(m_stack_page));
	save_item(NAME(m_vector_page));
}

void t6a84_device::device_reset()
{
	m_code_page = 0;
	m_delay_code_page = 0;
	m_is_delay_code_page_set = false;
	m_prev_code_page = 0;
	m_data_page = 8;
	m_stack_page = 8;
	m_vector_page = 0;

	z80_device::device_reset();
}

device_memory_interface::space_config_vector t6a84_device::memory_space_config() const
{
	auto r = z80_device::memory_space_config();
	r.emplace_back(AS_DATA, &m_data_space_config);
	r.emplace_back(AS_STACK, &m_stack_space_config);
	for (auto it = r.begin(); it != r.end(); ++it) {
		if ((*it).first == AS_IO) {
			(*it).second = &m_io_space_config;
		} else if ((*it).first == AS_PROGRAM) {
			(*it).second = &m_program_space_config;
		}
	}

	return r;
}

bool t6a84_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	if (spacenum == AS_PROGRAM) {
		address = code_address(address);
	} else if (spacenum == AS_DATA) {
		address = data_address(address);
	} else if (spacenum == AS_STACK) {
		address = stack_address(address);
	}

	target_space = &space(spacenum);

	return true;
}

uint32_t t6a84_device::code_address(uint16_t address)
{
	const uint32_t page_address = PAGE_SIZE * m_code_page + address;
	LOGMASKED(LOG_MEM, "CODE @ %06x => %06x\n", address, page_address);

	return page_address;
}

uint32_t t6a84_device::data_address(uint16_t address)
{
	const uint32_t page_address = PAGE_SIZE * m_data_page + address;
	LOGMASKED(LOG_MEM, "DATA @ %06x => %06x\n", address, page_address);

	return page_address;
}

uint32_t t6a84_device::stack_address(uint16_t address)
{
	const uint32_t page_address = PAGE_SIZE * m_stack_page + address;
	LOGMASKED(LOG_MEM, "STACK @ %06x => %06x\n", address, page_address);

	return page_address;
}

uint8_t t6a84_device::stack_read(uint16_t addr)
{
	return m_stack.read_byte(translate_memory_address(addr));
}

void t6a84_device::stack_write(uint16_t addr, uint8_t value)
{
	m_stack.write_byte(translate_memory_address((uint32_t) addr), value);
}

uint8_t t6a84_device::data_page_r()
{
	LOGMASKED(LOG_PAGE_R, "data_page_r: %02x @ %06x\n", m_data_page, pc());
	return m_data_page;
}

uint8_t t6a84_device::stack_page_r()
{
	LOGMASKED(LOG_PAGE_R, "stack_page_r: %02x @ %06x\n", m_stack_page, pc());
	return m_stack_page;
}

uint8_t t6a84_device::code_page_r()
{
	LOGMASKED(LOG_PAGE_R, "code_page_r: %02x @ %06x\n", m_code_page, pc());
	return m_code_page;
}

uint8_t t6a84_device::vector_page_r()
{
	LOGMASKED(LOG_PAGE_R, "vector_page_r: %02x @ %06x\n", m_vector_page, pc());
	return m_vector_page;
}

void t6a84_device::data_page_w(uint8_t page)
{
	LOGMASKED(LOG_PAGE_W, "data_page_w: %02x @ %06x\n", page, pc());
	m_data_page = page;
}

void t6a84_device::stack_page_w(uint8_t page)
{
	LOGMASKED(LOG_PAGE_W, "stack_page_w: %02x @ %06x\n", page, pc());
	m_stack_page = page;
}

void t6a84_device::code_page_w(uint8_t page)
{
	LOGMASKED(LOG_PAGE_W, "code_page_w: %02x @ %06x\n", page, pc());
	m_delay_code_page = page;
	m_is_delay_code_page_set = true;
}

void t6a84_device::vector_page_w(uint8_t page)
{
	LOGMASKED(LOG_PAGE_W, "vector_page_w: %02x @ %06x\n", page, pc());
	m_vector_page = page;
}
