// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the MCT-ADR device found in Microsoft Jazz/MIPS
 * ARCSystem 100 architecture systems. This device was originally designed
 * by Microsoft, and then implemented and used in various forms by MIPS,
 * Olivetti, LSI Logic, NEC, Acer and others.
 *
 * Specific implementations/derivatives include:
 *
 *   LSI Logic R4030/R4230
 *   NEC μPD31432
 *   ALI M6101-A1
 *
 * References:
 *
 *   https://datasheet.datasheetarchive.com/originals/scans/Scans-054/DSAIH000102184.pdf
 *   https://github.com/torvalds/linux/tree/master/arch/mips/jazz/
 *   http://cvsweb.netbsd.org/bsdweb.cgi/src/sys/arch/arc/jazz/
 *
 *   https://www.linux-mips.org/archives/riscy/1993-08/msg00064.html
 *   https://www.linux-mips.org/archives/riscy/1993-08/msg00069.html
 *
 * TODO
 *   - proper width dma
 *   - dma address translation errors
 *   - I/O cache
 *   - revision 2 device
 */

#include "emu.h"
#include "mct_adr.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MCT_ADR, mct_adr_device, "mct_adr", "MCT-ADR Address Path Controller")

mct_adr_device::mct_adr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MCT_ADR, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_io_config("io", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_dma_config("dma", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor(FUNC(mct_adr_device::dma), this))
	, m_out_int_dma(*this)
	, m_out_int_device(*this)
	, m_out_int_timer(*this)
	, m_eisa_iack(*this)
	, m_dma_r(*this)
	, m_dma_w(*this)
{
}

void mct_adr_device::map(address_map &map)
{
	map(0x000, 0x007).lrw32(NAME([this] () { return m_config; }), NAME([this] (u32 data) { m_config = data; }));
	map(0x008, 0x00f).lr32([] () { return 1; }, "revision_level");
	map(0x010, 0x017).lr32(NAME([this] () { m_dma_interrupt_source &= ~DMA_ADDRESS_ERROR; return m_dma_invalid_address; }));
	map(0x018, 0x01f).lrw32(NAME([this] () { return m_trans_tbl_base; }), NAME([this] (u32 data) { LOG("tbl base 0x%08x\n", data); m_trans_tbl_base = data; }));
	map(0x020, 0x027).lrw32(NAME([this] () { return m_trans_tbl_limit; }), NAME([this] (u32 data) { LOG("tbl limit 0x%08x\n", data); m_trans_tbl_limit = data; }));
	map(0x028, 0x02f).lrw32([] () { return 0; }, "translation_invalidate_r", [] (u32 data) { }, "translation_invalidate_w");
	map(0x030, 0x037).lw32(NAME([this] (u32 data) { m_ioc_maint = data; }));
	map(0x038, 0x03f).lr32([] () { return 0; }, "remote_failed_address");
	map(0x040, 0x047).lr32(NAME([this] () { m_dma_interrupt_source &= ~DMA_PARITY_ERROR; return m_dma_memory_failed_address; }));
	map(0x048, 0x04f).lw32(NAME([this] (u32 data) { m_ioc_physical_tag = data; }));
	map(0x050, 0x057).lw32(NAME([this] (u32 data) { m_ioc_logical_tag = data; }));
	map(0x058, 0x05f).lrw32(
			// FIXME: hack to pass diagnostics
			[this] ()
			{
				u32 const data = m_ioc_byte_mask;

				if (data == 0xffffffff)
					m_ioc_byte_mask = 0;
				return data;
			}, "io_cache_byte_mask_r",
			NAME([this] (u32 data) { m_ioc_byte_mask |= data; }));
	map(0x060, 0x067).lw32(
			[this] (u32 data)
			{
				// FIXME: hack to pass diagnostics
				if (m_ioc_logical_tag == 0x80000001 && m_ioc_byte_mask == 0x0f0f0f0f)
				{
					u32 const address = (m_ioc_physical_tag & ~0x1) + ((m_ioc_maint & 0x3) << 3);

					space(0).write_dword(address, data);
				}
			}, "io_cache_buffer_window_lo");
	// io_cache_buffer_window_hi
	map(0x070, 0x0ef).lrw32(
			NAME([this] (offs_t offset) { return m_remote_speed[offset  >> 1]; }),
			NAME([this] (offs_t offset, u32 data) { m_remote_speed[offset >> 1] = data; }));
	// parity_diagnostic_lo
	// parity_diagnostic_hi
	map(0x100, 0x1ff).lrw32(
			NAME([this] (offs_t offset) { return m_dma_reg[offset >> 1]; }),
			[this] (offs_t offset, u32 data)
			{
				unsigned const reg = offset >> 1;

				LOG("dma_reg %d data 0x%08x (%s)\n", offset, data, machine().describe_context());

				m_dma_reg[reg] = data;

				if ((reg == REG_ENABLE) && (data & DMA_ENABLE))
					LOG("dma started address 0x%08x count %d\n", translate_address(m_dma_reg[(0 << 2) + REG_ADDRESS]), m_dma_reg[(0 << 2) + REG_COUNT]);
			}, "dma_reg_w");
	map(0x200, 0x207).lr32(NAME([this] () { return m_dma_interrupt_source; }));
	map(0x208, 0x20f).lr32([] () { return 0; }, "error_type");
	map(0x210, 0x217).lrw32(NAME([this] () { return m_memory_refresh_rate; }), NAME([this] (u32 data) { m_memory_refresh_rate = data; }));
	// refresh_counter
	map(0x220, 0x227).lrw32(NAME([this] () { return m_nvram_protect; }), NAME([this] (u32 data) { LOG("nvram_protect 0x%08x (%s)\n", data, machine().describe_context()); m_nvram_protect = data; }));
	map(0x228, 0x22f).lw32(
			[this] (u32 data)
			{
				LOG("timer_w 0x%08x\n", data);

				attotime interval = attotime::from_ticks((data + 1) & 0x1ff, 1000);

				m_interval_timer->adjust(interval, 0, interval);
			}, "interrupt_interval");
	map(0x230, 0x237).lr32([this] () { if (m_out_int_timer_asserted) { m_out_int_timer_asserted = false; m_out_int_timer(0); } return m_interval_timer->remaining().as_ticks(1000); }, "interval_timer");
	map(0x238, 0x23b).lr32(NAME([this] () { return m_eisa_iack(); }));
}

// HACK: this address map translates i386 bus master DMA device access (the
// SONIC network controller) to DRAM.
device_memory_interface::space_config_vector mct_adr_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_io_config),
		std::make_pair(1, &m_dma_config)
	};
}

void mct_adr_device::dma(address_map &map)
{
	map(0x00000000U, 0xffffffffU).rw(FUNC(mct_adr_device::dma_r), FUNC(mct_adr_device::dma_w));
}

void mct_adr_device::device_start()
{
	m_out_int_dma.resolve();
	m_out_int_device.resolve();
	m_out_int_timer.resolve();
	m_eisa_iack.resolve();

	m_dma_r.resolve_all_safe(0xff);
	m_dma_w.resolve_all_safe();

	m_ioc_maint = 0;
	m_ioc_physical_tag = 0;
	m_ioc_logical_tag = 0;

	m_irq_check = timer_alloc(FUNC(mct_adr_device::irq_check), this);
	m_dma_check = timer_alloc(FUNC(mct_adr_device::dma_check), this);
	m_interval_timer = timer_alloc(FUNC(mct_adr_device::interval_timer), this);

	m_out_int_timer_asserted = false;
	m_out_int_device_asserted = false;
}

void mct_adr_device::device_reset()
{
	m_config = 0x104; // REV1, REV2 is 0x410
	m_trans_tbl_base = 0;
	m_trans_tbl_limit = 0;
	m_ioc_byte_mask = 0;

	for (u32 &val : m_remote_speed)
		val = 0x7;

	for (u32 &val : m_dma_reg)
		val = 0;

	m_dma_interrupt_source = 0;
	m_memory_refresh_rate = 0x18186;
	m_nvram_protect = 0x7;

	m_dma_invalid_address = 0;
	m_dma_memory_failed_address = 0;

	m_isr = 0;
	m_imr = 0x10; // firmware diagnostic expects network interrupts to be unmasked at boot

	m_interval_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));

	irq_check(0);
}

void mct_adr_device::set_irq_line(int irq, int state)
{
	if ((irq != 3) && (m_isr & (1 << irq)) ^ (state << irq))
		LOG("set_irq_line %d state %d m_imr 0x%04x\n", irq, state, m_imr);

	if (state)
		m_isr |= (1 << irq);
	else
		m_isr &= ~(1 << irq);

	m_irq_check->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(mct_adr_device::irq_check)
{
	if (bool(m_isr & m_imr) != m_out_int_device_asserted)
	{
		m_out_int_device_asserted = bool(m_isr & m_imr);

		m_out_int_device(m_out_int_device_asserted ? 1 : 0);
	}
}

u16 mct_adr_device::isr_r()
{
	u16 const pending = m_isr & m_imr;

	for (u16 irq = 0; irq < 16; irq++)
		if (BIT(pending, irq))
			return (irq + 1) << 2;

	return 0;
}

void mct_adr_device::imr_w(u16 data)
{
	LOG("imr_w 0x%04x (%s)\n", data, machine().describe_context());

	m_imr = data;

	m_irq_check->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(mct_adr_device::interval_timer)
{
	if (m_out_int_timer_asserted)
		m_out_int_timer(0);
	else
		m_out_int_timer_asserted = true;

	m_out_int_timer(1);
}

void mct_adr_device::set_drq_line(int channel, int state)
{
	m_drq_active[channel] = state == ASSERT_LINE;

	if (state)
		m_dma_check->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(mct_adr_device::dma_check)
{
	bool active = false;

	for (int channel = 0; channel < 4; channel++)
	{
		if (!m_drq_active[channel])
			continue;

		// reg 0x00: 0x00000011 - mode (ch0) (WIDTH16 | ATIME_80)
		// reg 0x20: 0x0000000a - mode (ch1) (WIDTH8 | ATIME_120)
		// reg 0x18: 0x00000f20 - address
		// reg 0x10: 0x00000024 - count
		// reg 0x08: 0x00000001 - enable (ENABLE | !WRITE)

		// check channel enabled
		if (!(m_dma_reg[(channel << 2) + REG_ENABLE] & DMA_ENABLE))
			return;

		// check transfer count
		if (!m_dma_reg[(channel << 2) + REG_COUNT])
			return;

		u32 const address = translate_address(m_dma_reg[(channel << 2) + REG_ADDRESS]);

		// perform dma transfer
		if (m_dma_reg[(channel << 2) + REG_ENABLE] & DMA_DIRECTION)
		{
			u8 const data = space(0).read_byte(address);

			//LOG("dma_w data 0x%02x address 0x%08x\n", data, address);

			m_dma_w[channel](data);
		}
		else
		{
			u8 const data = m_dma_r[channel]();

			//LOG("dma_r data 0x%02x address 0x%08x\n", data, address);

			space(0).write_byte(address, data);
		}

		// increment address, decrement count
		m_dma_reg[(channel << 2) + REG_ADDRESS]++;
		m_dma_reg[(channel << 2) + REG_COUNT]--;

		// set terminal count flag
		if (!m_dma_reg[(channel << 2) + REG_COUNT])
		{
			m_dma_reg[(channel << 2) + REG_ENABLE] |= DMA_TERMINAL_COUNT;

			// TODO: dma interrupts
			if (m_dma_reg[(channel << 2) + REG_ENABLE] & DMA_INTERRUPT_ENABLE)
				logerror("dma interrupt enable - interrupt expected\n");
		}

		if (m_drq_active[channel])
			active = true;
	}

	if (active)
		m_dma_check->adjust(attotime::zero);
}

u32 mct_adr_device::translate_address(u32 logical_address)
{
	u32 page = logical_address >> 12;
	if (page < (m_trans_tbl_limit) >> 3)
	{
		u32 entry_address = (m_trans_tbl_base & 0x7fffffff) + page * 8;

		return space(0).read_dword(entry_address) | (logical_address & 0xfff);
	}
	else
	{
		logerror("failed to translate address 0x%08x\n", logical_address);

		return 0; // FIXME: address error
	}
}

u32 mct_adr_device::dma_r(offs_t offset, u32 mem_mask)
{
	u32 const address = translate_address(offset << 2);

	return space(0).read_dword(address, mem_mask);
}

void mct_adr_device::dma_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const address = translate_address(offset << 2);

	space(0).write_dword(address, data, mem_mask);
}
