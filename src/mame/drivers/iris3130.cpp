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
        Most everything

    Interrupts:
        M68K:
            6 - DUART

****************************************************************************/

#include "emu.h"

#include "imagedev/snapquik.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/bankdev.h"
#include "machine/mc146818.h"
#include "machine/mc68681.h"

#include <vector>

#define LOG_RTC             (1 << 0)
#define LOG_INVALID_SEGMENT (1 << 1)
#define LOG_OTHER           (1 << 2)

#define VERBOSE     (0)

#include "logmacro.h"

class iris3000_state : public driver_device
{
public:
	iris3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_storagercpu(*this, "storager"),
		m_mainram(*this, "mainram"),
		m_duarta(*this, "duart68681a"),
		m_duartb(*this, "duart68681b"),
		m_text_data_space(*this, "text_data"),
		m_stack_space(*this, "stack"),
		m_kernel_space(*this, "kernel"),
		m_system_space(*this, "system"),
		m_multibus_mem_space(*this, "multibus_mem"),
		m_multibus_io_space(*this, "multibus_io"),
		m_geometry_pipe_space(*this, "geometry_pipe"),
		m_fpa_space(*this, "fpa"),
		m_page_table_map(*this, "ptmap"),
		m_rtc(*this, "rtc"),
		m_dips(*this, "DIPS")
	{
	}

	void iris3130(machine_config &config);

	DECLARE_QUICKLOAD_LOAD_MEMBER(load_romboard);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void text_data_map(address_map &map);
	void stack_map(address_map &map);
	void kernel_map(address_map &map);
	void system_map(address_map &map);
	void multibus_mem_map(address_map &map);
	void multibus_io_map(address_map &map);
	void geometry_pipe_map(address_map &map);
	void fpa_map(address_map &map);

	uint32_t mmu_r(offs_t offset, uint32_t mem_mask = ~0);
	void mmu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t mouse_buttons_r();
	void mouse_buttons_w(uint8_t data);
	uint16_t mouse_quad_r(offs_t offset, uint16_t mem_mask = ~0);
	void mouse_quad_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dips_r(offs_t offset, uint16_t mem_mask = ~0);
	uint8_t clock_ctrl_r();
	void clock_ctrl_w(uint8_t data);
	uint8_t clock_data_r();
	void clock_data_w(uint8_t data);
	uint8_t kernel_base_r(offs_t offset);
	void kernel_base_w(offs_t offset, uint8_t data);
	uint16_t status_r(offs_t offset, uint16_t mem_mask = ~0);
	void status_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t parity_ctrl_r();
	void parity_ctrl_w(uint8_t data);
	uint8_t multibus_prot_r();
	void multibus_prot_w(uint8_t data);
	uint32_t page_table_map_r(offs_t offset, uint32_t mem_mask = ~0);
	void page_table_map_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t text_data_base_r(offs_t offset, uint16_t mem_mask = ~0);
	void text_data_base_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t text_data_limit_r(offs_t offset, uint16_t mem_mask = ~0);
	void text_data_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t stack_base_r(offs_t offset, uint16_t mem_mask = ~0);
	void stack_base_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t stack_limit_r(offs_t offset, uint16_t mem_mask = ~0);
	void stack_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t romboard_r(offs_t offset);
	void romboard_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(duarta_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(duartb_irq_handler);

	void mem_map(address_map &map);
	void storager_map(address_map& map);

	required_device<m68020_device> m_maincpu;
	required_device<m68000_device> m_storagercpu;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<mc68681_device> m_duarta;
	required_device<mc68681_device> m_duartb;

	required_device<address_map_bank_device> m_text_data_space;
	required_device<address_map_bank_device> m_stack_space;
	required_device<address_map_bank_device> m_kernel_space;
	required_device<address_map_bank_device> m_system_space;
	required_device<address_map_bank_device> m_multibus_mem_space;
	required_device<address_map_bank_device> m_multibus_io_space;
	required_device<address_map_bank_device> m_geometry_pipe_space;
	required_device<address_map_bank_device> m_fpa_space;

	required_shared_ptr<uint32_t> m_page_table_map;
	required_device<mc146818_device> m_rtc;
	required_ioport m_dips;

	enum
	{
		MOUSE_BUTTON_RIGHT      = 0x01,
		MOUSE_BUTTON_MIDDLE     = 0x02,
		MOUSE_BUTTON_LEFT       = 0x04,
		BOARD_REV1              = 0x60, /* Board revision - #1 */
		BOARD_REV2              = 0x50, /* Board revision - #2 */

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

	std::vector<uint8_t> m_file_data;
	uint8_t m_mouse_buttons;
	uint16_t m_mouse_quadrature;
	uint16_t m_text_data_base;
	uint16_t m_text_data_limit;
	uint16_t m_stack_base;
	uint16_t m_stack_limit;
	uint16_t m_status;
	uint8_t m_parity_ctrl;
	uint8_t m_multibus_prot;
};

QUICKLOAD_LOAD_MEMBER(iris3000_state::load_romboard)
{
	m_file_data.resize(quickload_size);

	if (!quickload_size || image.fread(&m_file_data[0], quickload_size) != quickload_size)
	{
		m_file_data.clear();
		return image_init_result::FAIL;
	}
	return image_init_result::PASS;
}

/***************************************************************************
    MACHINE FUNCTIONS
***************************************************************************/

uint32_t iris3000_state::mmu_r(offs_t offset, uint32_t mem_mask)
{
	const uint8_t type = (offset >> 26) & 0xf;
	const uint32_t vaddr = offset & 0x03ffffff;

	const uint32_t page_offset = vaddr & 0x000003ff;
	const uint32_t page_index = (vaddr >> 10) & 0x3fff;
	const uint32_t phys_page = m_page_table_map[page_index] & 0x1fff;
	const uint32_t paddr = (phys_page << 10) | page_offset;

	uint32_t ret;
	switch (type)
	{
	case 0: // Text/Data Segment
		ret = m_text_data_space->read32(paddr, mem_mask);
		break;
	case 1: // Stack Segment
		ret = m_stack_space->read32(paddr, mem_mask);
		break;
	case 2: // Kernel Segment
		ret = m_kernel_space->read32(paddr, mem_mask);
		break;
	case 3: // System Segment
		ret = m_system_space->read32(vaddr, mem_mask);
		break;
	case 4: // Multibus Memory Segment
		ret = m_multibus_mem_space->read32(vaddr, mem_mask);
		break;
	case 5: // Multibus I/O Segment
		ret = m_multibus_io_space->read32(vaddr, mem_mask);
		break;
	case 6: // Geometry Pipe Segment
		ret = m_geometry_pipe_space->read32(vaddr, mem_mask);
		break;
	case 15: // Floating-Point Accelerator Segment
		ret = m_fpa_space->read32(vaddr, mem_mask);
		break;
	default: // Unused/Unknown Segment
		LOGMASKED(LOG_INVALID_SEGMENT, "%s: Invalid segment read: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
		ret = 0;
		break;
	}
	return ret;
}

void iris3000_state::mmu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint8_t type = (offset >> 26) & 0xf;
	const uint32_t vaddr = offset & 0x03ffffff;

	const uint32_t page_offset = vaddr & 0x000003ff;
	const uint32_t page_index = (vaddr >> 10) & 0x3fff;
	const uint32_t phys_page = m_page_table_map[page_index] & 0x1fff;
	const uint32_t paddr = (phys_page << 10) | page_offset;

	switch (type)
	{
	case 0: // Text/Data Segment
		m_text_data_space->write32(paddr, data, mem_mask);
		break;
	case 1: // Stack Segment
		m_stack_space->write32(paddr, data, mem_mask);
		break;
	case 2: // Kernel Segment
		m_kernel_space->write32(paddr, data, mem_mask);
		break;
	case 3: // System Segment
		m_system_space->write32(vaddr, data, mem_mask);
		break;
	case 4: // Multibus Memory Segment
		m_multibus_mem_space->write32(vaddr, data, mem_mask);
		break;
	case 5: // Multibus I/O Segment
		m_multibus_io_space->write32(vaddr, data, mem_mask);
		break;
	case 6: // Geometry Pipe Segment
		m_geometry_pipe_space->write32(vaddr, data, mem_mask);
		break;
	case 15: // Floating-Point Accelerator Segment
		m_fpa_space->write32(vaddr, data, mem_mask);
		break;
	default: // Unused/Unknown Segment
		LOGMASKED(LOG_INVALID_SEGMENT, "%s: Invalid segment write: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
		break;
	}
}

uint8_t iris3000_state::mouse_buttons_r()
{
	LOGMASKED(LOG_OTHER, "%s: mouse_buttons_r: %02x\n", machine().describe_context(), m_mouse_buttons | BOARD_REV1);
	return m_mouse_buttons | BOARD_REV1;
}

void iris3000_state::mouse_buttons_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: mouse_buttons_w (ignored): %02x\n", machine().describe_context(), data);
}

uint16_t iris3000_state::mouse_quad_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: mouse_quad_r: %04x & %04x\n", machine().describe_context(), m_mouse_quadrature, mem_mask);
	return m_mouse_quadrature;
}

void iris3000_state::mouse_quad_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: mouse_quad_w (ignored): %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

uint16_t iris3000_state::dips_r(offs_t offset, uint16_t mem_mask)
{
	const uint16_t data = m_dips->read();
	LOGMASKED(LOG_OTHER, "%s: dips_r: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	return data;
}

uint8_t iris3000_state::clock_ctrl_r()
{
	const uint8_t data = m_rtc->read(1);
	LOGMASKED(LOG_RTC, "%s: clock_ctrl_r: %02x\n", machine().describe_context(), data);
	return data;
}

void iris3000_state::clock_ctrl_w(uint8_t data)
{
	LOGMASKED(LOG_RTC, "%s: clock_ctrl_w: %02x\n", machine().describe_context(), data);
	m_rtc->write(1, data);
}

uint8_t iris3000_state::clock_data_r()
{
	uint8_t data = m_rtc->read(0);
	LOGMASKED(LOG_RTC, "%s: clock_data_r: %02x\n", machine().describe_context(), data);
	return data;
}

void iris3000_state::clock_data_w(uint8_t data)
{
	LOGMASKED(LOG_RTC, "%s: clock_data_w: %02x\n", machine().describe_context(), data);
	m_rtc->write(0, data);
}

uint8_t iris3000_state::kernel_base_r(offs_t offset)
{
	switch(offset)
	{
		default:
			LOGMASKED(LOG_OTHER, "%s: kernel_base_r: Unknown Register %08x\n", machine().describe_context(), 0x36000000 + offset);
			break;
	}
	return 0;
}

void iris3000_state::kernel_base_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		default:
			LOGMASKED(LOG_OTHER, "%s: kernel_base_w: Unknown Register %08x = %02x\n", machine().describe_context(), 0x36000000 + offset, data);
			break;
	}
}

uint16_t iris3000_state::status_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: status_r: %04x & %04x\n", machine().describe_context(), m_status, mem_mask);
	return m_status;
}

void iris3000_state::status_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: status_w: %04x & %04x (DIAG:%x, ENABEXT:%d, ENABINT:%d, BINIT:%d, NOTBOOT:%d)\n", machine().describe_context(), data, mem_mask,
		data & 0xf, BIT(data, STATUS_ENABEXT), BIT(data, STATUS_ENABINT), BIT(data, STATUS_BINIT), BIT(data, STATUS_NOTBOOT));
	LOGMASKED(LOG_OTHER, "%s:                       (USERFPA:%d, USERGE:%d, SLAVE:%d, ENABCBRQ:%d)\n", machine().describe_context(),
		BIT(data, STATUS_USERFPA), BIT(data, STATUS_USERGE), BIT(data, STATUS_SLAVE), BIT(data, STATUS_ENABCBRQ));
	LOGMASKED(LOG_OTHER, "%s:                       (NOTGEMASTER:%d, GENBAD:%d, ENABWDOG:%d, QUICK_TOUT:%d)\n", machine().describe_context(),
		BIT(data, STATUS_NOTGEMASTER), BIT(data, STATUS_GENBAD), BIT(data, STATUS_ENABWDOG), BIT(data, STATUS_QUICK_TOUT));
	COMBINE_DATA(&m_status);
}

uint8_t iris3000_state::parity_ctrl_r()
{
	LOGMASKED(LOG_OTHER, "%s: parity_ctrl_r: %02x\n", m_parity_ctrl);
	return m_parity_ctrl;
}

void iris3000_state::parity_ctrl_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: parity_ctrl_w: %02x\n", machine().describe_context(), data);
	m_parity_ctrl = data;
}

uint8_t iris3000_state::multibus_prot_r()
{
	LOGMASKED(LOG_OTHER, "%s: multibus_prot_r: %02x\n", machine().describe_context(), m_multibus_prot);
	return m_multibus_prot;
}

void iris3000_state::multibus_prot_w(uint8_t data)
{
	LOGMASKED(LOG_OTHER, "%s: multibus_prot_w: %02x\n", machine().describe_context(), data);
	m_multibus_prot = data;
}

uint32_t iris3000_state::page_table_map_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: page_table_map_r: %08x = %08x & %08x\n", machine().describe_context(), 0x3b000000 + (offset << 2), m_page_table_map[offset], mem_mask);
	return m_page_table_map[offset];
}

void iris3000_state::page_table_map_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: page_table_map_w: %08x = %08x & %08x\n", machine().describe_context(), 0x3b000000 + (offset << 2), data, mem_mask);
	COMBINE_DATA(&m_page_table_map[offset]);
}

uint16_t iris3000_state::text_data_base_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: text_data_base_r: %04x & %04x\n", machine().describe_context(), m_text_data_base, mem_mask);
	return m_text_data_base;
}

void iris3000_state::text_data_base_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: text_data_base_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_text_data_base);
}


uint16_t iris3000_state::text_data_limit_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: text_data_limit_r: %04x & %04x\n", machine().describe_context(), m_text_data_limit, mem_mask);
	return m_text_data_limit;
}

void iris3000_state::text_data_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: text_data_limit_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_text_data_limit);
}


uint16_t iris3000_state::stack_base_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: stack_base_r: %04x & %04x\n", machine().describe_context(), m_stack_base, mem_mask);
	return m_stack_base;
}

void iris3000_state::stack_base_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: stack_base_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_stack_base);
}


uint16_t iris3000_state::stack_limit_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: stack_limit_r: %04x & %04x\n", machine().describe_context(), m_stack_limit, mem_mask);
	return m_stack_limit;
}

void iris3000_state::stack_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_OTHER, "%s: stack_limit_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_stack_limit);
}

uint8_t iris3000_state::romboard_r(offs_t offset)
{
	const uint8_t data = offset < m_file_data.size() ? m_file_data[offset] : 0;
	LOGMASKED(LOG_OTHER, "%s: romboard_r: %08x = %02x\n", machine().describe_context(), 0x40000000 + offset, data);
	return data;
}

void iris3000_state::romboard_w(offs_t offset, uint8_t data)
{
	if (offset > m_file_data.size())
		m_file_data.resize(offset + 1);
	m_file_data[offset] = data;
	LOGMASKED(LOG_OTHER, "%s: romboard write: %08x = %02x\n", machine().describe_context(), 0x40000000 + offset, data);
}

void iris3000_state::machine_start()
{
}

void iris3000_state::machine_reset()
{
	uint32_t *src = (uint32_t*)(memregion("maincpu")->base());
	uint32_t *dst = m_mainram;
	memcpy(dst, src, 8);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void iris3000_state::text_data_map(address_map &map)
{
	map(0x0000000, 0x03fffff).ram().share("mainram");
}

void iris3000_state::stack_map(address_map &map)
{
	map(0x0000000, 0x03fffff).ram().share("mainram");
}

void iris3000_state::kernel_map(address_map &map)
{
	map(0x0000000, 0x03fffff).ram().share("mainram");
}

void iris3000_state::system_map(address_map &map)
{
	map(0x0000000, 0x0017fff).rom().region("maincpu", 0);
	map(0x0800000, 0x0800003).rw(FUNC(iris3000_state::mouse_buttons_r), FUNC(iris3000_state::mouse_buttons_w));
	map(0x1000000, 0x1000003).rw(FUNC(iris3000_state::mouse_quad_r), FUNC(iris3000_state::mouse_quad_w));
	map(0x1800000, 0x1800003).r(FUNC(iris3000_state::dips_r));
	map(0x2000000, 0x200000f).rw(m_duarta, FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0x2800000, 0x280000f).rw(m_duartb, FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0x3000000, 0x30007ff).ram();
	map(0x4000000, 0x4000003).rw(FUNC(iris3000_state::clock_ctrl_r), FUNC(iris3000_state::clock_ctrl_w));
	map(0x5000000, 0x5000003).rw(FUNC(iris3000_state::clock_data_r), FUNC(iris3000_state::clock_data_w));
	map(0x6000000, 0x6000003).rw(FUNC(iris3000_state::kernel_base_r), FUNC(iris3000_state::kernel_base_w));
	map(0x8000000, 0x8000003).rw(FUNC(iris3000_state::status_r), FUNC(iris3000_state::status_w));
	map(0x9000000, 0x9000003).rw(FUNC(iris3000_state::parity_ctrl_r), FUNC(iris3000_state::parity_ctrl_w));
	map(0xa000000, 0xa000003).rw(FUNC(iris3000_state::multibus_prot_r), FUNC(iris3000_state::multibus_prot_w));
	map(0xb000000, 0xb00ffff).rw(FUNC(iris3000_state::page_table_map_r), FUNC(iris3000_state::page_table_map_w)).share("ptmap");
	map(0xc000000, 0xc000003).rw(FUNC(iris3000_state::text_data_base_r), FUNC(iris3000_state::text_data_base_w));
	map(0xd000000, 0xd000003).rw(FUNC(iris3000_state::text_data_limit_r), FUNC(iris3000_state::text_data_limit_w));
	map(0xe000000, 0xe000003).rw(FUNC(iris3000_state::stack_base_r), FUNC(iris3000_state::stack_base_w));
	map(0xf000000, 0xf000003).rw(FUNC(iris3000_state::stack_limit_r), FUNC(iris3000_state::stack_limit_w));
}

void iris3000_state::multibus_mem_map(address_map &map)
{
	map(0x0200000, 0x03fffff).r(FUNC(iris3000_state::romboard_r));
}

void iris3000_state::multibus_io_map(address_map &map)
{
}

void iris3000_state::geometry_pipe_map(address_map &map)
{
}

void iris3000_state::fpa_map(address_map &map)
{
}

void iris3000_state::mem_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(iris3000_state::mmu_r), FUNC(iris3000_state::mmu_w));
}

void iris3000_state::storager_map(address_map& map)
{
	map(0x00000000, 0x0000ffff).rom().region("storagercpu", 0);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

WRITE_LINE_MEMBER(iris3000_state::duarta_irq_handler)
{
	m_maincpu->set_input_line(M68K_IRQ_6, state);
}

WRITE_LINE_MEMBER(iris3000_state::duartb_irq_handler)
{
	m_maincpu->set_input_line(M68K_IRQ_6, state);
}

static DEVICE_INPUT_DEFAULTS_START( ip2_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void iris3000_state::iris3130(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &iris3000_state::mem_map);

	M68000(config, m_storagercpu, 10000000);
	m_storagercpu->set_addrmap(AS_PROGRAM, &iris3000_state::storager_map);

	ADDRESS_MAP_BANK(config, "text_data").set_map(&iris3000_state::text_data_map).set_options(ENDIANNESS_BIG, 32, 32);
	ADDRESS_MAP_BANK(config, "stack").set_map(&iris3000_state::stack_map).set_options(ENDIANNESS_BIG, 32, 32);
	ADDRESS_MAP_BANK(config, "kernel").set_map(&iris3000_state::kernel_map).set_options(ENDIANNESS_BIG, 32, 32);
	ADDRESS_MAP_BANK(config, "system").set_map(&iris3000_state::system_map).set_options(ENDIANNESS_BIG, 32, 32);
	ADDRESS_MAP_BANK(config, "multibus_mem").set_map(&iris3000_state::multibus_mem_map).set_options(ENDIANNESS_BIG, 32, 32);
	ADDRESS_MAP_BANK(config, "multibus_io").set_map(&iris3000_state::multibus_io_map).set_options(ENDIANNESS_BIG, 32, 32);
	ADDRESS_MAP_BANK(config, "geometry_pipe").set_map(&iris3000_state::geometry_pipe_map).set_options(ENDIANNESS_BIG, 32, 32);
	ADDRESS_MAP_BANK(config, "fpa").set_map(&iris3000_state::fpa_map).set_options(ENDIANNESS_BIG, 32, 32);

	MC68681(config, m_duarta, 3.6864_MHz_XTAL); /* Y3 3.6864MHz Xtal ??? copy-over from dectalk */
	m_duarta->irq_cb().set(FUNC(iris3000_state::duarta_irq_handler));
	m_duarta->b_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));

	MC68681(config, m_duartb, 3.6864_MHz_XTAL); /* Y3 3.6864MHz Xtal ??? copy-over from dectalk */
	m_duartb->irq_cb().set(FUNC(iris3000_state::duartb_irq_handler));

	MC146818(config, m_rtc, 4.194304_MHz_XTAL);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_duarta, FUNC(mc68681_device::rx_b_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(ip2_terminal));

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", ""));
	quickload.set_load_callback(FUNC(iris3000_state::load_romboard));
}

static INPUT_PORTS_START( iris3130 )
	PORT_START("DIPS")
	PORT_DIPNAME( 0x8000, 0x8000, "Master/Slave" )
	PORT_DIPSETTING(    0x0000, "Slave" )
	PORT_DIPSETTING(    0x8000, "Master" )
	PORT_BIT( 0x6000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x1800, 0x1800, "RS232 Console Speed" )
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
	PORT_DIPSETTING(    0x000c, "Stoarger Hard Disk Boot" )
	PORT_DIPSETTING(    0x000d, "DSD Tape Boot (1)" )
	PORT_DIPSETTING(    0x000e, "DSD Tape Boot (2)" )
	PORT_DIPSETTING(    0x000f, "DSD Hard Disk Boot" )
INPUT_PORTS_END

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( iris3130 )
	ROM_REGION32_BE(0x18000, "maincpu", 0)
	ROM_LOAD( "sgi-ip2-u91.nolabel.od",    0x00000, 0x8000, CRC(32e1f6b5) SHA1(2bd928c3fe2e364b9a38189158e9bad0e5271a59) )
	ROM_LOAD( "sgi-ip2-u92.nolabel.od",    0x08000, 0x8000, CRC(13dbfdb3) SHA1(3361fb62f7a8c429653700bccfc3e937f7508182) )
	ROM_LOAD( "sgi-ip2-u93.ip2.2-008.od",  0x10000, 0x8000, CRC(bf967590) SHA1(1aac48e4f5531a25c5482f64de5cd3c7a9931f11) )

	ROM_REGION16_BE(0x10000, "storagercpu", 0)
	ROM_LOAD16_BYTE( "5808423a.bin", 0x0000, 0x8000, CRC(161e6a90) SHA1(d4dcbf630a83e4c5994d8331ac85d81130400e33) )
	ROM_LOAD16_BYTE( "5808523a.bin", 0x0001, 0x8000, CRC(4c99e4b8) SHA1(899855e54c4520816ad43eb19b972b45783ccb6b) )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                 FULLNAME           FLAGS
COMP( 1985, iris3130, 0,      0,      iris3130, iris3130, iris3000_state, empty_init, "Silicon Graphics Inc", "IRIS 3130 (IP2)", MACHINE_IS_SKELETON )
