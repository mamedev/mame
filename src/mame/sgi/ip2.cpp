// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/****************************************************************************

    SGI IRIS 3130 skeleton driver

        0x00000000 - ?              RAM (?)
        0x30000000 - 0x30017fff     ROM (3x32k)
        0x30800000 - 0x30800000     Mouse Buttons (1)
        0x31000000 - 0x31000001     Mouse Quadrature (2)
        0x31800000 - 0x31800001     DIP Switches
        0x32000000 - 0x3200000f     DUART0 (serial console on channel B at 19200 baud 8N1, channel A set to 600 baud 8N1 (mouse?))
        0x32800000 - 0x3280000f     DUART1 (printer/modem?)
        0x33000000 - 0x330007ff     SRAM (2k)
        0x34000000 - 0x34000000     Clock Control (1)
        0x35000000 - 0x35000000     Clock Data (1)
        0x36000000 - 0x36000000     Kernel Base (1)
        0x38000000 - 0x38000001     Status Register (2)
        0x39000000 - 0x39000000     Parity control (1)
        0x3a000000 - 0x3a000000     Multibus Protection (1)
        0x3b000000 - 0x3b000003     Page Table Map Base (4)
        0x3c000000 - 0x3c000001     Text/Data Base (2)
        0x3d000000 - 0x3d000001     Text/Data Limit (2)
        0x3e000000 - 0x3e000001     Stack Base (2)
        0x3f000000 - 0x3f000001     Stack Limit (2)

    TODO:
     - MMU protection and faults
     - Multibus protection
     - bus errors and interrupts

    Interrupts:
        M68K:
            6 - DUART

****************************************************************************/

/*
 * WIP notes
 * --
 * bus errors: timeout (multibus or ge), illegal segment, access map/limit, fpa
 * interrupts: autovectors not used?, user vectors are:
 *
 *   65 multibus 0,1
 *   66 multibus 2
 *   67 multibus 3
 *   68 multibus 4
 *   69 multibus 5
 *   70 multibus 6
 *   71 multibus 7
 *   80 duart 0
 *   81 duart 1
 *   82 external interrupt (from multibus)
 *   83 clock
 *   85 parity
 *   86 mouse quadrature
 *   87 mouse connect
 *
 * 3 fbc
 * 4 ge
 */
#include "emu.h"
#include "ip2.h"
#include "iris_kbd.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68020.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"

#define LOG_INVALID_SEGMENT (1U << 2)
#define LOG_OTHER           (1U << 3)

//#define VERBOSE     (LOG_GENERAL|LOG_INVALID_SEGMENT)

#include "logmacro.h"

namespace {

class sgi_ip2_device
	: public device_t
	, public device_multibus_interface
{
public:
	sgi_ip2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SGI_IP2, tag, owner, clock)
		, device_multibus_interface(mconfig, *this)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_duart(*this, "duart%u", 0U)
		, m_port(*this, "port%u", 1U)
		, m_tod(*this, "tod")
		, m_nvram(*this, "nvram")
		, m_default_space(*this, "default")
		, m_system_space(*this, "system")
		, m_switch(*this, "SWITCH")
		, m_page(nullptr)
		, m_map(nullptr)
		, m_installed(false)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void default_map(address_map &map) ATTR_COLD;
	void system_map(address_map &map) ATTR_COLD;

	uint32_t mmu_r(offs_t offset, uint32_t mem_mask = ~0);
	void mmu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t mouse_buttons_r();
	void mouse_buttons_w(uint8_t data);
	uint16_t mouse_quad_r(offs_t offset, uint16_t mem_mask = ~0);
	void mouse_quad_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	u8 tod_ctrl_r();
	void tod_ctrl_w(u8 data);
	u8 tod_data_r();
	void tod_data_w(u8 data);
	uint8_t kernel_base_r(offs_t offset);
	void kernel_base_w(offs_t offset, uint8_t data);
	uint16_t status_r(offs_t offset, uint16_t mem_mask = ~0);
	void status_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t parity_ctrl_r();
	void parity_ctrl_w(uint8_t data);
	uint8_t multibus_prot_r();
	void multibus_prot_w(uint8_t data);
	uint16_t text_data_base_r(offs_t offset, uint16_t mem_mask = ~0);
	void text_data_base_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t text_data_limit_r(offs_t offset, uint16_t mem_mask = ~0);
	void text_data_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t stack_base_r(offs_t offset, uint16_t mem_mask = ~0);
	void stack_base_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t stack_limit_r(offs_t offset, uint16_t mem_mask = ~0);
	void stack_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void mem_map(address_map &map) ATTR_COLD;

	u32 page_r(offs_t offset);
	void page_w(offs_t offset, u32 data, u32 mem_mask);

	// Multibus access helpers
	offs_t map(offs_t offset) const;
	u16 mem_r(offs_t offset, u16 mem_mask);
	void mem_w(offs_t offset, u16 data, u16 mem_mask);
	u16 map_r(offs_t offset, u16 mem_mask);
	void map_w(offs_t offset, u16 data, u16 mem_mask);

	required_device<m68020_device> m_cpu;
	required_shared_ptr<uint32_t> m_ram;
	required_device_array<mc68681_device, 2> m_duart;
	required_device_array<rs232_port_device, 4> m_port;
	required_device<mc146818_device> m_tod;
	required_device<nvram_device> m_nvram;

	required_device<address_map_bank_device> m_default_space;
	required_device<address_map_bank_device> m_system_space;

	required_ioport m_switch;

	enum
	{
		MOUSE_BUTTON_RIGHT      = 0x01,
		MOUSE_BUTTON_MIDDLE     = 0x02,
		MOUSE_BUTTON_LEFT       = 0x04,
		BOARD_REVB              = 0x00, // higher revisions (N << 5)
		BOARD_REVA              = 0x10,

		MOUSE_XFIRE     = 0x01, /* X Quadrature Fired, active low */
		MOUSE_XCHANGE   = 0x02, /* MOUSE_XCHANGE ? x-- : x++ */
		MOUSE_YFIRE     = 0x04, /* Y Quadrature Fired, active low */
		MOUSE_YCHANGE   = 0x08, /* MOUSE_YCHANGE ? y-- : y++ */

		PAR_UR      = 0x01, /* Check parity on user-mode reads */
		PAR_UW      = 0x02, /* Check parity on user-mode writes */
		PAR_KR      = 0x04, /* Check parity on kernel-mode reads */
		PAR_KW      = 0x08, /* Check parity on kernel-mode writes */
		PAR_DIS0    = 0x10, /* Disable access to DUART0 and LEDs */
		PAR_DIS1    = 0x20, /* Disable access to DUART1 */
		PAR_MBR     = 0x40, /* Check parity on multibus reads */
		PAR_MBW     = 0x80, /* Check parity on multibus writes */

		MBP_DCACC   = 0x01, /* Display controller access (I/O page 4) */
		MBP_UCACC   = 0x02, /* Update controller access (I/O page 3) */
		MBP_GFACC   = 0x04, /* Allow GF access (I/O page 1) */
		MBP_DMACC   = 0x08, /* Allow GL2 DMA access (0x8nnnnn - x0bnnnnn) */
		MBP_LIOACC  = 0x10, /* Allow lower I/O access (0x0nnnnn - 0x7nnnnn) */
		MBP_HIOACC  = 0x20, /* Allow upper I/O access (0x8nnnnn - 0xfnnnnn) */
		MBP_LMACC   = 0x40, /* Allow lower memory access (0x0nnnnn - 0x7nnnnn) */
		MBP_HMACC   = 0x80, /* Allow upper memory access (0x8nnnnn - 0xfnnnnn) */

		STATUS_DIAG0        = 0,
		STATUS_DIAG1        = 1,
		STATUS_DIAG2        = 2,
		STATUS_DIAG3        = 3,
		STATUS_ENABEXT      = 4,
		STATUS_ENABINT      = 5,
		STATUS_BINIT        = 6,
		STATUS_NOTBOOT      = 7,
		STATUS_USERFPA      = 8,
		STATUS_USERGE       = 9,
		STATUS_SLAVE        = 10,
		STATUS_ENABCBRQ     = 11,
		STATUS_NOTGEMASTER  = 12,
		STATUS_GENBAD       = 13,
		STATUS_ENABWDOG     = 14,
		STATUS_QUICK_TOUT   = 15
	};

	enum page_mask : u32
	{
		PAGE_PFNUM = 0x0000'1fff,
		PAGE_P     = 0x3000'0000, // protection (0=none, 1=read, 2=system, 3=read/write)
		PAGE_R     = 0x4000'0000, // referenced
		PAGE_M     = 0x8000'0000, // modified

		PAGE_ALL   = 0xf000'1fff,
	};
	enum tod_ctrl_mask : u8
	{
		TOD_AS = 0x01, // address strobe
		TOD_DS = 0x02, // data strobe
		TOD_RE = 0x04, // read enable
		TOD_CE = 0x08, // clock enable
	};

	uint8_t m_mouse_buttons;
	uint16_t m_mouse_quadrature;
	uint16_t m_text_data_base;
	uint16_t m_text_data_limit;
	uint16_t m_stack_base;
	uint16_t m_stack_limit;
	uint16_t m_status;
	uint8_t m_parity_ctrl;
	uint8_t m_multibus_prot;

	u8 m_tod_ctrl;
	u8 m_tod_addr;

	std::unique_ptr<u32[]> m_page; // page table
	std::unique_ptr<u16[]> m_map;  // Multibus map

	bool m_installed;
};

void sgi_ip2_device::device_start()
{
	m_page = std::make_unique<u32[]>(16384); // AM2167-35PC x 17 (16384x1 SRAM)
	m_map = std::make_unique<u16[]>(256); // AM2148-55DC 1Kx4 SRAM (x4)

	save_pointer(NAME(m_page), 16384);
	save_pointer(NAME(m_map), 256);

	m_mouse_buttons = 0;
}

void sgi_ip2_device::device_reset()
{
	if (!m_installed)
	{
		// TODO: configuration switches, slave mode
		m_bus->space(AS_PROGRAM).install_readwrite_handler(0x00'0000, 0x0f'ffff,
			emu::rw_delegate(*this, FUNC(sgi_ip2_device::mem_r)),
			emu::rw_delegate(*this, FUNC(sgi_ip2_device::mem_w)));

		m_bus->space(AS_PROGRAM).install_readwrite_handler(0x10'0000, 0x1f'ffff,
			emu::rw_delegate(*this, FUNC(sgi_ip2_device::map_r)),
			emu::rw_delegate(*this, FUNC(sgi_ip2_device::map_w)));

		m_installed = true;
	}

	uint32_t *src = (uint32_t *)(memregion("cpu")->base());
	uint32_t *dst = m_ram;
	memcpy(dst, src, 8);
}

/***************************************************************************
    MACHINE FUNCTIONS
***************************************************************************/

// page table probably has 16384 entries of 16 bits each?

uint32_t sgi_ip2_device::mmu_r(offs_t offset, uint32_t mem_mask)
{
	const uint8_t type = (offset >> 26) & 0xf;
	const uint32_t vaddr = offset & 0x03ffffff;

	const uint32_t page_offset = vaddr & 0x000003ff;
	const uint32_t page_index = (vaddr >> 10) & 0x3fff;
	const uint32_t phys_page = m_page[page_index] & PAGE_PFNUM;
	const uint32_t paddr = (phys_page << 10) | page_offset;

	uint32_t ret;
	switch (type)
	{
	case 0: // Text/Data Segment
	case 1: // Stack Segment
	case 2: // Kernel Segment
		ret = m_default_space->read32(paddr, mem_mask);
		break;
	case 3: // System Segment
		ret = m_system_space->read32(vaddr, mem_mask);
		break;
	default: // Unused/Unknown Segment
		LOGMASKED(LOG_INVALID_SEGMENT, "%s: Invalid segment read: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
		ret = 0;
		break;
	}
	return ret;
}

void sgi_ip2_device::mmu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint8_t type = (offset >> 26) & 0xf;
	const uint32_t vaddr = offset & 0x03ff'ffff;

	const uint32_t page_offset = vaddr & 0x000003ff;
	const uint32_t page_index = (vaddr >> 10) & 0x3fff;
	const uint32_t phys_page = m_page[page_index] & 0x1fff;
	const uint32_t paddr = (phys_page << 10) | page_offset;

	switch (type)
	{
	case 0: // Text/Data Segment
	case 1: // Stack Segment
	case 2: // Kernel Segment
		m_default_space->write32(paddr, data, mem_mask);
		break;
	case 3: // System Segment
		m_system_space->write32(vaddr, data, mem_mask);
		break;
	default: // Unused/Unknown Segment
		LOGMASKED(LOG_INVALID_SEGMENT, "%s: Invalid segment write: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
		break;
	}
}

uint8_t sgi_ip2_device::mouse_buttons_r()
{
	LOGMASKED(LOG_OTHER, "%s: mouse_buttons_r: %02x\n", machine().describe_context(), m_mouse_buttons);
	return m_mouse_buttons;
}

void sgi_ip2_device::mouse_buttons_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: mouse_buttons_w (ignored): %02x\n", machine().describe_context(), data);
}

uint16_t sgi_ip2_device::mouse_quad_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: mouse_quad_r: %04x & %04x\n", machine().describe_context(), m_mouse_quadrature, mem_mask);
	return m_mouse_quadrature;
}

void sgi_ip2_device::mouse_quad_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: mouse_quad_w (ignored): %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

u8 sgi_ip2_device::tod_ctrl_r()
{
	return m_tod_ctrl;
}

void sgi_ip2_device::tod_ctrl_w(u8 data)
{
	m_tod_ctrl = data;
}

u8 sgi_ip2_device::tod_data_r()
{
	if (m_tod_ctrl == (TOD_RE | TOD_DS))
		return m_tod->read_direct(m_tod_addr);
	else
		return m_tod_addr;
}

void sgi_ip2_device::tod_data_w(u8 data)
{
	if (m_tod_ctrl == TOD_CE)
		m_tod->write_direct(m_tod_addr, data);
	else
		m_tod_addr = data;
}

uint8_t sgi_ip2_device::kernel_base_r(offs_t offset)
{
	switch(offset)
	{
		default:
			LOGMASKED(LOG_OTHER, "%s: kernel_base_r: Unknown Register %08x\n", machine().describe_context(), 0x36000000 + offset);
			break;
	}
	return 0;
}

void sgi_ip2_device::kernel_base_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		default:
			LOGMASKED(LOG_OTHER, "%s: kernel_base_w: Unknown Register %08x = %02x\n", machine().describe_context(), 0x36000000 + offset, data);
			break;
	}
}

uint16_t sgi_ip2_device::status_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: status_r: %04x & %04x\n", machine().describe_context(), m_status, mem_mask);
	return m_status;
}

void sgi_ip2_device::status_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: status_w: %04x & %04x (DIAG:%x, ENABEXT:%d, ENABINT:%d, BINIT:%d, NOTBOOT:%d)\n", machine().describe_context(), data, mem_mask,
		data & 0xf, BIT(data, STATUS_ENABEXT), BIT(data, STATUS_ENABINT), BIT(data, STATUS_BINIT), BIT(data, STATUS_NOTBOOT));
	LOGMASKED(LOG_OTHER, "%s:                       (USERFPA:%d, USERGE:%d, SLAVE:%d, ENABCBRQ:%d)\n", machine().describe_context(),
		BIT(data, STATUS_USERFPA), BIT(data, STATUS_USERGE), BIT(data, STATUS_SLAVE), BIT(data, STATUS_ENABCBRQ));
	LOGMASKED(LOG_OTHER, "%s:                       (NOTGEMASTER:%d, GENBAD:%d, ENABWDOG:%d, QUICK_TOUT:%d)\n", machine().describe_context(),
		BIT(data, STATUS_NOTGEMASTER), BIT(data, STATUS_GENBAD), BIT(data, STATUS_ENABWDOG), BIT(data, STATUS_QUICK_TOUT));
	COMBINE_DATA(&m_status);
}

uint8_t sgi_ip2_device::parity_ctrl_r()
{
	LOGMASKED(LOG_OTHER, "%s: parity_ctrl_r: %02x\n", m_parity_ctrl);
	return m_parity_ctrl;
}

void sgi_ip2_device::parity_ctrl_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: parity_ctrl_w: %02x\n", machine().describe_context(), data);
	m_parity_ctrl = data;
}

uint8_t sgi_ip2_device::multibus_prot_r()
{
	LOGMASKED(LOG_OTHER, "%s: multibus_prot_r: %02x\n", machine().describe_context(), m_multibus_prot);
	return m_multibus_prot;
}

void sgi_ip2_device::multibus_prot_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: multibus_prot_w: %02x\n", machine().describe_context(), data);
	m_multibus_prot = data;
}

uint16_t sgi_ip2_device::text_data_base_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: text_data_base_r: %04x & %04x\n", machine().describe_context(), m_text_data_base, mem_mask);
	return m_text_data_base;
}

void sgi_ip2_device::text_data_base_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: text_data_base_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_text_data_base);
}


uint16_t sgi_ip2_device::text_data_limit_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: text_data_limit_r: %04x & %04x\n", machine().describe_context(), m_text_data_limit, mem_mask);
	return m_text_data_limit;
}

void sgi_ip2_device::text_data_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: text_data_limit_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_text_data_limit);
}


uint16_t sgi_ip2_device::stack_base_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: stack_base_r: %04x & %04x\n", machine().describe_context(), m_stack_base, mem_mask);
	return m_stack_base;
}

void sgi_ip2_device::stack_base_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: stack_base_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_stack_base);
}


uint16_t sgi_ip2_device::stack_limit_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: stack_limit_r: %04x & %04x\n", machine().describe_context(), m_stack_limit, mem_mask);
	return m_stack_limit;
}

void sgi_ip2_device::stack_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: stack_limit_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_stack_limit);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void sgi_ip2_device::default_map(address_map &map)
{
	map(0x0000000, 0x03fffff).ram().share("ram");
}

void sgi_ip2_device::system_map(address_map &map)
{
	map(0x000'0000, 0x001'7fff).rom().region("cpu", 0);

	map(0x080'0000, 0x080'0000).rw(FUNC(sgi_ip2_device::mouse_buttons_r), FUNC(sgi_ip2_device::mouse_buttons_w));
	map(0x100'0000, 0x100'0001).rw(FUNC(sgi_ip2_device::mouse_quad_r), FUNC(sgi_ip2_device::mouse_quad_w));
	map(0x180'0000, 0x180'0001).lr16([this]() { return m_switch->read(); }, "switch_r");
	map(0x200'0000, 0x200'000f).rw(m_duart[0], FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0x280'0000, 0x280'000f).rw(m_duart[1], FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0x300'0000, 0x300'07ff).ram().share("nvram");
	map(0x400'0000, 0x400'0000).rw(FUNC(sgi_ip2_device::tod_ctrl_r), FUNC(sgi_ip2_device::tod_ctrl_w));
	map(0x500'0000, 0x500'0000).rw(FUNC(sgi_ip2_device::tod_data_r), FUNC(sgi_ip2_device::tod_data_w));
	map(0x600'0000, 0x600'0000).rw(FUNC(sgi_ip2_device::kernel_base_r), FUNC(sgi_ip2_device::kernel_base_w));
	map(0x800'0000, 0x800'0001).rw(FUNC(sgi_ip2_device::status_r), FUNC(sgi_ip2_device::status_w));
	map(0x900'0000, 0x900'0000).rw(FUNC(sgi_ip2_device::parity_ctrl_r), FUNC(sgi_ip2_device::parity_ctrl_w));
	map(0xa00'0000, 0xa00'0000).rw(FUNC(sgi_ip2_device::multibus_prot_r), FUNC(sgi_ip2_device::multibus_prot_w));
	map(0xb00'0000, 0xb00'7fff).rw(FUNC(sgi_ip2_device::page_r), FUNC(sgi_ip2_device::page_w));
	map(0xc00'0000, 0xc00'0001).rw(FUNC(sgi_ip2_device::text_data_base_r), FUNC(sgi_ip2_device::text_data_base_w));
	map(0xd00'0000, 0xd00'0001).rw(FUNC(sgi_ip2_device::text_data_limit_r), FUNC(sgi_ip2_device::text_data_limit_w));
	map(0xe00'0000, 0xe00'0001).rw(FUNC(sgi_ip2_device::stack_base_r), FUNC(sgi_ip2_device::stack_base_w));
	map(0xf00'0000, 0xf00'0001).rw(FUNC(sgi_ip2_device::stack_limit_r), FUNC(sgi_ip2_device::stack_limit_w));
}

void sgi_ip2_device::mem_map(address_map &map)
{
	map(0x0000'0000, 0xffff'ffff).rw(FUNC(sgi_ip2_device::mmu_r), FUNC(sgi_ip2_device::mmu_w));

	map(0x4000'0000, 0x401f'ffff).lrw16(
		[this](offs_t offset, u16 mem_mask) { return m_bus->space(AS_PROGRAM).read_word(offset << 1, mem_mask); }, "mem_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { m_bus->space(AS_PROGRAM).write_word(offset << 1, data, mem_mask); }, "mem_w");

	map(0x5000'0000, 0x5000'ffff).lrw16(
		[this](offs_t offset, u16 mem_mask) { return m_bus->space(AS_IO).read_word(offset << 1, mem_mask); }, "pio_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { m_bus->space(AS_IO).write_word(offset << 1, data, mem_mask); }, "pio_w");

	map(0x6000'0000, 0x6fff'ffff).noprw(); // geometry pipe
	map(0xf000'0000, 0xffff'ffff).noprw(); // floating point accelerator
}

void keyboard_devices(device_slot_interface &device)
{
	device.option_add("kbd", IRIS_KBD);
}

void sgi_ip2_device::device_add_mconfig(machine_config &config)
{
	M68020(config, m_cpu, 32_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &sgi_ip2_device::mem_map);

	ADDRESS_MAP_BANK(config, "default").set_map(&sgi_ip2_device::default_map).set_options(ENDIANNESS_BIG, 32, 32);
	ADDRESS_MAP_BANK(config, "system").set_map(&sgi_ip2_device::system_map).set_options(ENDIANNESS_BIG, 32, 32);

	input_merger_any_high_device &irq6(INPUT_MERGER_ANY_HIGH(config, "irq6"));
	irq6.output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ6);

	MC68681(config, m_duart[0], 3.6864_MHz_XTAL);
	m_duart[0]->irq_cb().set(irq6, FUNC(input_merger_any_high_device::in_w<0>));
	m_duart[0]->b_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));

	MC68681(config, m_duart[1], 3.6864_MHz_XTAL);
	m_duart[1]->irq_cb().set(irq6, FUNC(input_merger_any_high_device::in_w<1>));

	RS232_PORT(config, m_port[0], keyboard_devices, nullptr);
	RS232_PORT(config, m_port[1], default_rs232_devices, "terminal");
	RS232_PORT(config, m_port[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_port[3], default_rs232_devices, nullptr);

	m_duart[0]->a_tx_cb().set(m_port[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_port[1], FUNC(rs232_port_device::write_txd));
	m_duart[1]->a_tx_cb().set(m_port[2], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_port[3], FUNC(rs232_port_device::write_txd));

	m_port[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_port[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));
	m_port[2]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_port[3]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));

	MC146818(config, m_tod, 32.768_kHz_XTAL);

	NVRAM(config, m_nvram); // AM2148-55DC x 4 (1024x4 SRAM)
}

offs_t sgi_ip2_device::map(offs_t offset) const
{
	return u32(m_map[offset >> 11]) << 11 | (offset & 0x7ff);
}

u32 sgi_ip2_device::page_r(offs_t offset)
{
	return m_page[offset];
}
void sgi_ip2_device::page_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: page_w 0x%04x data 0x%08x\n", machine().describe_context(), offset << 2, data);

	m_page[offset] = data & PAGE_ALL;
}

u16 sgi_ip2_device::mem_r(offs_t offset, u16 mem_mask)
{
	u16 data = 0;

	if (m_status & STATUS_SLAVE)
	{
		auto ram = util::big_endian_cast<u16>(m_ram.target());
		offs_t const physical = map(offset);
		data = ram[physical];

		LOG("%s: mem_r 0x%06x physical 0x%06x data 0x%04x\n", machine().describe_context(), offset << 1, physical << 1, data);

		//if ((offset < 0x8000) && (m_status & STATUS_MBOX))
		//  irq4_w<EXCEPTION_MBOX>(1);
	}
	else
		LOG("%s: mem_r access disabled\n", machine().describe_context());

	return data;
}

void sgi_ip2_device::mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_status & STATUS_SLAVE)
	{
		auto ram = util::big_endian_cast<u16>(m_ram.target());
		offs_t const physical = map(offset);

		LOG("%s: mem_w 0x%06x physical 0x%06x data 0x%04x\n", machine().describe_context(), offset << 1, physical << 1, data);

		COMBINE_DATA(&ram[physical]);

		//if ((offset < 0x8000) && (m_status & STATUS_MBOX))
		//  irq4_w<EXCEPTION_MBOX>(1);
	}
	else
		LOG("%s: mem_w access disabled\n", machine().describe_context());
}

u16 sgi_ip2_device::map_r(offs_t offset, u16 mem_mask)
{
	if (false) //m_status & STATUS_EN0)
	{
		LOG("%s: map_r access disabled\n", machine().describe_context());

		return 0;
	}
	else
		return m_map[offset >> 11];
}

void sgi_ip2_device::map_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("%s: map_w page 0x%03x data 0x%04x\n", machine().describe_context(), offset >> 11, data);

	if (false) //m_status & (STATUS_EN0 | STATUS_EN1))
		LOG("map_w access disabled\n");
	else
		m_map[offset >> 11] = data;
}

static INPUT_PORTS_START(sgi_ip2)
	PORT_START("SWITCH")
	PORT_DIPNAME( 0x8000, 0x8000, "Master/Slave" )
	PORT_DIPSETTING(    0x0000, "Slave" )
	PORT_DIPSETTING(    0x8000, "Master" )
	PORT_BIT( 0x6000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x1800, 0x0000, "RS232 Console Speed" )
	PORT_DIPSETTING(    0x0000, "9600 Baud" )
	PORT_DIPSETTING(    0x0800, "300 Baud" )
	PORT_DIPSETTING(    0x1000, "1200 Baud" )
	PORT_DIPSETTING(    0x1800, "19200 Baud" )
	PORT_DIPNAME( 0x0700, 0x0000, "Display Setting" )
	PORT_DIPSETTING(    0x0000, "60Hz Non-Interlaced / 60Hz Non-Interlaced" )
	PORT_DIPSETTING(    0x0100, "60Hz Non-Interlaced / 30Hz Interlaced" )
	PORT_DIPSETTING(    0x0200, "60Hz Non-Interlaced / NTSC RS 170A" )
	PORT_DIPSETTING(    0x0300, "60Hz Non-Interlaced / PAL" )
	PORT_DIPSETTING(    0x0400, "30Hz Interlaced / 60Hz Non-Interlaced" )
	PORT_DIPSETTING(    0x0500, "30Hz Interlaced / 30Hz Interlaced" )
	PORT_DIPSETTING(    0x0600, "30Hz Interlaced / NTSC RS 170A" )
	PORT_DIPSETTING(    0x0700, "30Hz Interlaced / PAL" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0000, "Enable Dual-Head Display" )
	PORT_DIPSETTING(    0x0000, "Use Primary Display" )
	PORT_DIPSETTING(    0x0040, "Use Secondary Display" )
	PORT_DIPNAME( 0x0020, 0x0000, "Verbose Boot" )
	PORT_DIPSETTING(    0x0000, "Be Verbose" )
	PORT_DIPSETTING(    0x0020, "Be Quiet" )
	PORT_DIPNAME( 0x0010, 0x0000, "Auto-Boot" )
	PORT_DIPSETTING(    0x0000, "Enter PROM Monitor" )
	PORT_DIPSETTING(    0x0010, "Auto-Boot" )
	PORT_DIPNAME( 0x000f, 0x0005, "Boot Media" )
	PORT_DIPSETTING(    0x0000, "Hard Disk (IP, SD, MD)" )
	PORT_DIPSETTING(    0x0001, "Cartridge Tape" )
	PORT_DIPSETTING(    0x0002, "Floppy Disk (SF, MF)" )
	PORT_DIPSETTING(    0x0003, "Ethernet using XNS" )
	PORT_DIPSETTING(    0x0005, "Enter PROM Monitor" )
	PORT_DIPSETTING(    0x0006, "Boot from PROM Board" )
	PORT_DIPSETTING(    0x0007, "TCP/UDP Netboot" )
	PORT_DIPSETTING(    0x0009, "Interphase SMD Disk Boot" )
	PORT_DIPSETTING(    0x000a, "Storager Tape Boot (1)" )
	PORT_DIPSETTING(    0x000b, "Storager Tape Boot (2)" )
	PORT_DIPSETTING(    0x000c, "Storager Hard Disk Boot" )
	PORT_DIPSETTING(    0x000d, "DSD Tape Boot (1)" )
	PORT_DIPSETTING(    0x000e, "DSD Tape Boot (2)" )
	PORT_DIPSETTING(    0x000f, "DSD Hard Disk Boot" )
INPUT_PORTS_END

ioport_constructor sgi_ip2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sgi_ip2);
}

ROM_START(sgi_ip2)
	ROM_DEFAULT_BIOS("v3010")
	ROM_SYSTEM_BIOS(0, "v3010", "IRIS Monitor Version 3.0.10 July 1, 1987")

	ROM_REGION32_BE(0x18000, "cpu", 0)
	ROMX_LOAD( "sgi-ip2-u91.nolabel.od",    0x00000, 0x8000, CRC(32e1f6b5) SHA1(2bd928c3fe2e364b9a38189158e9bad0e5271a59), ROM_BIOS(0)) // IP2/0 5000-484-08  U91
	ROMX_LOAD( "sgi-ip2-u92.nolabel.od",    0x08000, 0x8000, CRC(13dbfdb3) SHA1(3361fb62f7a8c429653700bccfc3e937f7508182), ROM_BIOS(0)) // IP2/1 5000-456-08  U92
	ROMX_LOAD( "sgi-ip2-u93.ip2.2-008.od",  0x10000, 0x8000, CRC(bf967590) SHA1(1aac48e4f5531a25c5482f64de5cd3c7a9931f11), ROM_BIOS(0)) // IP2/2 5000-455-08  U93
ROM_END

const tiny_rom_entry *sgi_ip2_device::device_rom_region() const
{
	return ROM_NAME(sgi_ip2);
}

} // Anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SGI_IP2, device_multibus_interface, sgi_ip2_device, "sgi_ip2", "Silicon Graphics IP2")
