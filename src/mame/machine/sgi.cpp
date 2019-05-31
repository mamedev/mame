// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    sgi.c

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#include "emu.h"
#include "sgi.h"

#define LOG_UNKNOWN     (1 << 0)
#define LOG_READS       (1 << 1)
#define LOG_WRITES      (1 << 2)
#define LOG_RPSS        (1 << 3)
#define LOG_WATCHDOG    (1 << 4)
#define LOG_MEMCFG      (1 << 5)
#define LOG_MEMCFG_EXT  (1 << 6)
#define LOG_EEPROM      (1 << 7)
#define LOG_DMA         (1 << 8)
#define LOG_DEFAULT     (LOG_READS | LOG_WRITES | LOG_RPSS | LOG_WATCHDOG | LOG_UNKNOWN)

#define VERBOSE         (LOG_DMA)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_MC, sgi_mc_device, "sgi_mc", "SGI Memory Controller")

sgi_mc_device::sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_MC, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_eeprom(*this, finder_base::DUMMY_TAG)
	, m_int_dma_done_cb(*this)
	, m_rpss_timer(nullptr)
	, m_dma_timer(nullptr)
	, m_watchdog(0)
	, m_sys_id(0)
	, m_rpss_divider(0)
	, m_refcnt_preload(0)
	, m_refcnt(0)
	, m_gio64_arb_param(0)
	, m_arb_cpu_time(0)
	, m_arb_burst_time(0)
	, m_cpu_mem_access_config(0)
	, m_gio_mem_access_config(0)
	, m_cpu_error_addr(0)
	, m_cpu_error_status(0)
	, m_gio_error_addr(0)
	, m_gio_error_status(0)
	, m_sys_semaphore(0)
	, m_gio_lock(0)
	, m_eisa_lock(0)
	, m_gio64_translate_mask(0)
	, m_gio64_substitute_bits(0)
	, m_dma_int_cause(0)
	, m_dma_control(0)
	, m_rpss_counter(0)
	, m_dma_mem_addr(0)
	, m_dma_size(0)
	, m_dma_stride(0)
	, m_dma_gio64_addr(0)
	, m_dma_mode(0)
	, m_dma_count(0)
	, m_dma_run(0)
	, m_eeprom_ctrl(0)
	, m_rpss_divide_counter(0)
	, m_rpss_divide_count(0)
	, m_rpss_increment(0)
{
}

void sgi_mc_device::device_resolve_objects()
{
	m_int_dma_done_cb.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sgi_mc_device::device_start()
{
	// if Indigo2, ID appropriately
	if (!strcmp(machine().system().name, "ip244415"))
	{
		m_sys_id = 0x13; // rev. C MC, EISA bus present
	}
	else
	{
		m_sys_id = 0x03; // rev. C MC, no EISA bus
	}

	m_rpss_timer = timer_alloc(TIMER_RPSS);
	m_rpss_timer->adjust(attotime::never);

	m_dma_timer = timer_alloc(TIMER_DMA);
	m_dma_timer->adjust(attotime::never);

	save_item(NAME(m_cpu_control));
	save_item(NAME(m_watchdog));
	save_item(NAME(m_sys_id));
	save_item(NAME(m_rpss_divider));
	save_item(NAME(m_refcnt_preload));
	save_item(NAME(m_refcnt));
	save_item(NAME(m_gio64_arb_param));
	save_item(NAME(m_arb_cpu_time));
	save_item(NAME(m_arb_burst_time));
	save_item(NAME(m_mem_config));
	save_item(NAME(m_cpu_mem_access_config));
	save_item(NAME(m_gio_mem_access_config));
	save_item(NAME(m_cpu_error_addr));
	save_item(NAME(m_cpu_error_status));
	save_item(NAME(m_gio_error_addr));
	save_item(NAME(m_gio_error_status));
	save_item(NAME(m_sys_semaphore));
	save_item(NAME(m_gio_lock));
	save_item(NAME(m_eisa_lock));
	save_item(NAME(m_gio64_translate_mask));
	save_item(NAME(m_gio64_substitute_bits));
	save_item(NAME(m_dma_int_cause));
	save_item(NAME(m_dma_control));
	save_item(NAME(m_dma_tlb_entry_hi));
	save_item(NAME(m_dma_tlb_entry_lo));
	save_item(NAME(m_rpss_counter));
	save_item(NAME(m_dma_mem_addr));
	save_item(NAME(m_dma_size));
	save_item(NAME(m_dma_stride));
	save_item(NAME(m_dma_gio64_addr));
	save_item(NAME(m_dma_mode));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_dma_run));
	save_item(NAME(m_eeprom_ctrl));
	save_item(NAME(m_semaphore));
	save_item(NAME(m_rpss_divide_counter));
	save_item(NAME(m_rpss_divide_count));
	save_item(NAME(m_rpss_increment));
}

void sgi_mc_device::device_reset()
{
	m_cpu_control[0] = 0;
	m_cpu_control[1] = 0;
	m_watchdog = 0;
	m_rpss_divider = 0x0104;
	m_refcnt_preload = 0;
	m_refcnt = 0;
	m_gio64_arb_param = 0;
	m_arb_cpu_time = 0;
	m_arb_burst_time = 0;
	m_mem_config[0] = 0;
	m_mem_config[1] = 0;
	m_cpu_mem_access_config = 0;
	m_gio_mem_access_config = 0;
	m_cpu_error_addr = 0;
	m_cpu_error_status = 0;
	m_gio_error_addr = 0;
	m_gio_error_status = 0;
	m_sys_semaphore = 0;
	m_gio_lock = 0;
	m_eisa_lock = 0;
	m_gio64_translate_mask = 0;
	m_gio64_substitute_bits = 0;
	m_dma_int_cause = 0;
	m_dma_control = 0;
	for (int i = 0; i < 4; i++)
	{
		m_dma_tlb_entry_hi[i] = 0;
		m_dma_tlb_entry_lo[i] = 0;
	}
	m_rpss_counter = 0;
	m_dma_mem_addr = 0;
	m_dma_size = 0;
	m_dma_stride = 0;
	m_dma_gio64_addr = 0;
	m_dma_mode = 0;
	m_dma_count = 0;
	m_dma_run = 0;
	m_eeprom_ctrl = 0;
	memset(m_semaphore, 0, sizeof(uint32_t) * 16);
	m_rpss_timer->adjust(attotime::from_hz(10000000), 0, attotime::from_hz(10000000));
	m_rpss_divide_counter = 4;
	m_rpss_divide_count = 4;
	m_rpss_increment = 1;

	m_space = &m_maincpu->space(AS_PROGRAM);
}

void sgi_mc_device::set_cpu_buserr(uint32_t address)
{
	m_cpu_error_addr = address;
	m_cpu_error_status = 0x00000400;
	if (address & 1)
		m_cpu_error_status |= 0x000000f0;
	else
		m_cpu_error_status |= 0x0000000f;
}

uint32_t sgi_mc_device::dma_translate(uint32_t address)
{
	for (int entry = 0; entry < 4; entry++)
	{
		if ((address & 0xffc00000) == (m_dma_tlb_entry_hi[entry] & 0xffc00000))
		{
			const uint32_t vpn_lo = (address & 0x003ff000) >> 12;
			const uint32_t pte = m_space->read_dword(((m_dma_tlb_entry_lo[entry] & 0x003fffc0) << 6) + (vpn_lo << 2));
			const uint32_t offset = address & 0xfff;
			return ((pte & 0x03ffffc0) << 6) + offset;
		}
	}
	return 0;
}

void sgi_mc_device::dma_immediate()
{
	uint32_t memory_addr = m_dma_mem_addr;
	uint32_t linecount = get_line_count();
	uint32_t zoomcount = get_zoom_count();
	uint32_t bytecount = get_byte_count();
	const uint32_t gio_addr = m_dma_gio64_addr;
	const uint32_t linewidth = get_line_width();
	const uint32_t linezoom = get_line_zoom();
	const uint32_t stride = get_stride();
	m_dma_size &= 0x0000ffff;
	m_dma_count = 0;
	while (linecount > 0)
	{
		linecount--;
		while (zoomcount > 0)
		{
			zoomcount--;
			while (bytecount > 0)
			{
				if (m_dma_mode & MODE_TO_HOST)
				{
					if (m_dma_mode & MODE_FILL)
					{   // Fill mode
						m_space->write_dword(dma_translate(memory_addr), m_dma_gio64_addr);
						if (m_dma_mode & MODE_DIR)
							memory_addr += 4;
						else
							memory_addr -= 4;
						bytecount -= 4;
					}
					else
					{
						uint32_t length = 8;
						uint64_t shift = 56;
						if (bytecount < 8)
							length = bytecount;

						uint64_t data = m_space->read_qword(gio_addr);
						for (uint32_t i = 0; i < length; i++)
						{
							m_space->write_byte(dma_translate(memory_addr), (uint8_t)(data >> shift));
							if (m_dma_mode & MODE_DIR)
								memory_addr++;
							else
								memory_addr--;
							shift -= 8;
						}

						bytecount -= length;
					}
				}
				else
				{
					uint32_t length = 8;
					uint64_t shift = 56;
					if (bytecount < 8)
						length = bytecount;

					uint64_t data = 0;
					for (uint32_t i = 0; i < length; i++)
					{
						data |= (uint64_t)m_space->read_byte(dma_translate(memory_addr)) << shift;
						if (m_dma_mode & MODE_DIR)
							memory_addr++;
						else
							memory_addr--;
						shift -= 8;
					}

					m_space->write_qword(gio_addr, data);
					bytecount -= length;
				}
			}
			bytecount = linewidth;

			if (zoomcount > 0)
			{
				if (m_dma_mode & MODE_DIR)
					memory_addr -= linewidth;
				else
					memory_addr += linewidth;
			}
		}
		zoomcount = linezoom;
		memory_addr += stride;
	}

	m_dma_mem_addr = memory_addr;

	m_dma_timer->adjust(attotime::never);
	m_dma_run |= (1 << 3);
	m_dma_run &= ~(1 << 6);
	if (BIT(m_dma_control, 4))
	{
		m_dma_int_cause |= (1 << 3);
		m_int_dma_done_cb(ASSERT_LINE);
	}
}

READ32_MEMBER(sgi_mc_device::read)
{
	switch (offset & ~1)
	{
	case 0x0000/4:
	case 0x0008/4:
	{
		const uint32_t index = (offset >> 3) & 1;
		LOGMASKED(LOG_READS, "%s: CPU Control %d Read: %08x & %08x\n", machine().describe_context(), index, m_cpu_control[index], mem_mask);
		return m_cpu_control[index];
	}
	case 0x0010/4:
		LOGMASKED(LOG_WATCHDOG, "%s: Watchdog Timer Read: %08x & %08x\n", machine().describe_context(), m_watchdog, mem_mask);
		return m_watchdog;
	case 0x0018/4:
		LOGMASKED(LOG_READS, "%s: System ID Read: %08x & %08x\n", machine().describe_context(), m_sys_id, mem_mask);
		return m_sys_id;
	case 0x0028/4:
		LOGMASKED(LOG_RPSS, "%s: RPSS Divider Read: %08x & %08x\n", machine().describe_context(), m_rpss_divider, mem_mask);
		return m_rpss_divider;
	case 0x0030/4:
	{
		// Disabled - we don't have a dump from real hardware, and IRIX 5.x freaks out with default contents.
		uint32_t ret = (m_eeprom_ctrl & ~0x10);// | m_eeprom->do_read() << 4;
		LOGMASKED(LOG_READS, "%s: R4000 EEPROM Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		return ret;
	}
	case 0x0040/4:
		LOGMASKED(LOG_READS, "%s: Refresh Count Preload Read: %08x & %08x\n", machine().describe_context(), m_refcnt_preload, mem_mask);
		return m_refcnt_preload;
	case 0x0048/4:
		LOGMASKED(LOG_READS, "%s: Refresh Count Read: %08x & %08x\n", machine().describe_context(), m_refcnt, mem_mask);
		return m_refcnt;
	case 0x0080/4:
		LOGMASKED(LOG_READS, "%s: GIO64 Arbitration Param Read: %08x & %08x\n", machine().describe_context(), m_gio64_arb_param, mem_mask);
		return m_gio64_arb_param;
	case 0x0088/4:
		LOGMASKED(LOG_READS, "%s: Arbiter CPU Time Read: %08x & %08x\n", machine().describe_context(), m_arb_cpu_time, mem_mask);
		return m_arb_cpu_time;
	case 0x0098/4:
		LOGMASKED(LOG_READS, "%s: Arbiter Long Burst Time Read: %08x & %08x\n", machine().describe_context(), m_arb_burst_time, mem_mask);
		return m_arb_burst_time;
	case 0x00c0/4:
	case 0x00c8/4:
	{
		const uint32_t index = (offset >> 1) & 1;
		LOGMASKED(LOG_MEMCFG, "%s: Memory Configuration Register %d Read: %08x & %08x\n", machine().describe_context(), index, m_mem_config[index], mem_mask);
		return m_mem_config[index];
	}
	case 0x00d0/4:
		LOGMASKED(LOG_READS, "%s: CPU Memory Access Config Params Read: %08x & %08x\n", machine().describe_context(), m_cpu_mem_access_config, mem_mask);
		return m_cpu_mem_access_config;
	case 0x00d8/4:
		LOGMASKED(LOG_READS, "%s: GIO Memory Access Config Params Read: %08x & %08x\n", machine().describe_context(), m_gio_mem_access_config, mem_mask);
		return m_gio_mem_access_config;
	case 0x00e0/4:
		LOGMASKED(LOG_READS, "%s: CPU Error Address Read: %08x & %08x\n", machine().describe_context(), m_cpu_error_addr, mem_mask);
		return m_cpu_error_addr;
	case 0x00e8/4:
		LOGMASKED(LOG_READS, "%s: CPU Error Status Read: %08x & %08x\n", machine().describe_context(), m_cpu_error_status, mem_mask);
		return m_cpu_error_status;
	case 0x00f0/4:
		LOGMASKED(LOG_READS, "%s: GIO Error Address Read: %08x & %08x\n", machine().describe_context(), m_gio_error_addr, mem_mask);
		return m_gio_error_addr;
	case 0x00f8/4:
		LOGMASKED(LOG_READS, "%s: GIO Error Status Read: %08x & %08x\n", machine().describe_context(), m_gio_error_status, mem_mask);
		return m_gio_error_status;
	case 0x0100/4:
		LOGMASKED(LOG_READS, "%s: System Semaphore Read: %08x & %08x\n", machine().describe_context(), m_sys_semaphore, mem_mask);
		return m_sys_semaphore;
	case 0x0108/4:
		LOGMASKED(LOG_READS, "%s: GIO Lock Read: %08x & %08x\n", machine().describe_context(), m_gio_lock, mem_mask);
		return m_gio_lock;
	case 0x0110/4:
		LOGMASKED(LOG_READS, "%s: EISA Lock Read: %08x & %08x\n", machine().describe_context(), m_eisa_lock, mem_mask);
		return m_eisa_lock;
	case 0x0150/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: GIO64 Translation Address Mask Read: %08x & %08x\n", machine().describe_context(), m_gio64_translate_mask, mem_mask);
		return m_gio64_translate_mask;
	case 0x0158/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: GIO64 Translation Address Substitution Bits Read: %08x & %08x\n", machine().describe_context(), m_gio64_substitute_bits, mem_mask);
		return m_gio64_substitute_bits;
	case 0x0160/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Interrupt Cause: %08x & %08x\n", machine().describe_context(), m_dma_int_cause, mem_mask);
		return m_dma_int_cause;
	case 0x0168/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Control Read: %08x & %08x\n", machine().describe_context(), m_dma_control, mem_mask);
		return m_dma_control;
	case 0x0180/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA TLB Entry 0 High Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry_hi[0], mem_mask);
		return m_dma_tlb_entry_hi[0];
	case 0x0188/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA TLB Entry 0 Low Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry_lo[0], mem_mask);
		return m_dma_tlb_entry_lo[0];
	case 0x0190/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA TLB Entry 1 High Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry_hi[1], mem_mask);
		return m_dma_tlb_entry_hi[1];
	case 0x0198/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA TLB Entry 1 Low Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry_lo[1], mem_mask);
		return m_dma_tlb_entry_lo[1];
	case 0x01a0/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA TLB Entry 2 High Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry_hi[2], mem_mask);
		return m_dma_tlb_entry_hi[2];
	case 0x01a8/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA TLB Entry 2 Low Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry_lo[2], mem_mask);
		return m_dma_tlb_entry_lo[2];
	case 0x01b0/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA TLB Entry 3 High Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry_hi[3], mem_mask);
		return m_dma_tlb_entry_hi[3];
	case 0x01b8/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA TLB Entry 3 Low Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry_lo[3], mem_mask);
		return m_dma_tlb_entry_lo[3];
	case 0x1000/4:
		LOGMASKED(LOG_RPSS, "%s: RPSS 100ns Counter Read: %08x & %08x\n", machine().describe_context(), m_rpss_counter, mem_mask);
		return m_rpss_counter;
	case 0x2000/4:
	case 0x2008/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Memory Address Read: %08x & %08x\n", machine().describe_context(), m_dma_mem_addr, mem_mask);
		return m_dma_mem_addr;
	case 0x2010/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Line Count and Width Read: %08x & %08x\n", machine().describe_context(), m_dma_size, mem_mask);
		return m_dma_size;
	case 0x2018/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Line Zoom and Stride Read: %08x & %08x\n", machine().describe_context(), m_dma_stride, mem_mask);
		return m_dma_stride;
	case 0x2020/4:
	case 0x2028/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA GIO64 Address Read: %08x & %08x\n", machine().describe_context(), m_dma_gio64_addr, mem_mask);
		return m_dma_gio64_addr;
	case 0x2030/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Mode Write: %08x & %08x\n", machine().describe_context(), m_dma_mode, mem_mask);
		return m_dma_mode;
	case 0x2038/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Zoom Count Read: %08x & %08x\n", machine().describe_context(), m_dma_count, mem_mask);
		return m_dma_count;
	case 0x2040/4:
		LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Start Read\n", machine().describe_context());
		return 0;
	case 0x2048/4:
	{
		if (!(m_dma_run & 0x40))
		{
			LOGMASKED(LOG_READS | LOG_DMA, "%s: DMA Run Read: %08x & %08x\n", machine().describe_context(), m_dma_run, mem_mask);
		}
		return m_dma_run;
	}
	case 0x10000/4:
	case 0x11000/4:
	case 0x12000/4:
	case 0x13000/4:
	case 0x14000/4:
	case 0x15000/4:
	case 0x16000/4:
	case 0x17000/4:
	case 0x18000/4:
	case 0x19000/4:
	case 0x1a000/4:
	case 0x1b000/4:
	case 0x1c000/4:
	case 0x1d000/4:
	case 0x1e000/4:
	case 0x1f000/4:
	{
		const uint32_t index = (offset - 0x4000) >> 10;
		const uint32_t data = m_semaphore[index];
		LOGMASKED(LOG_READS, "%s: Semaphore %d Read: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		m_semaphore[index] = 1;
		return data;
	}
	default:
		LOGMASKED(LOG_READS | LOG_UNKNOWN, "%s: Unmapped MC read: %08x & %08x\n", machine().describe_context(), 0x1fa00000 + offset*4, mem_mask);
		return 0;
	}
	return 0;
}

WRITE32_MEMBER( sgi_mc_device::write )
{
	switch (offset & ~1)
	{
	case 0x0000/4:
	case 0x0008/4:
	{
		const uint32_t index = (offset >> 1) & 1;
		LOGMASKED(LOG_WRITES, "%s: CPU Control %d Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		m_cpu_control[index] = data;
		break;
	}
	case 0x0010/4:
		LOGMASKED(LOG_WATCHDOG, "%s: Watchdog Timer Clear\n", machine().describe_context());
		m_watchdog = 0;
		break;
	case 0x0028/4:
		LOGMASKED(LOG_RPSS, "%s: RPSS Divider Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_rpss_divider = data;
		m_rpss_divide_count = (int)(m_rpss_divider & 0xff);
		m_rpss_divide_counter = m_rpss_divide_count;
		m_rpss_increment = (m_rpss_divider >> 8) & 0xff;
		break;
	case 0x0030/4:
		LOGMASKED(LOG_WRITES, "%s: R4000 EEPROM Write: CS:%d, SCK:%d, SO:%d\n", machine().describe_context(),
			BIT(data, 1), BIT(data, 2), BIT(data, 3));
		m_eeprom_ctrl = data;
		m_eeprom->di_write(BIT(data, 3));
		m_eeprom->cs_write(BIT(data, 1));
		m_eeprom->clk_write(BIT(data, 2));
		break;
	case 0x0040/4:
		LOGMASKED(LOG_WRITES, "%s: Refresh Count Preload Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_refcnt_preload = data;
		break;
	case 0x0080/4:
		LOGMASKED(LOG_WRITES, "%s: GIO64 Arbitration Param Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio64_arb_param = data;
		break;
	case 0x0088/4:
		LOGMASKED(LOG_WRITES, "%s: Arbiter CPU Time Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_arb_cpu_time = data;
		break;
	case 0x0098/4:
		LOGMASKED(LOG_WRITES, "%s: Arbiter Long Burst Time Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_arb_burst_time = data;
		break;
	case 0x00c0/4:
	case 0x00c8/4:
	{
		static const char* const s_mem_size[32] = {
				"256Kx36", "512Kx36, 2 subbanks", "Invalid", "1Mx36", "Invalid", "Invalid", "Invalid", "2Mx36, 2 subbanks",
				"Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "4Mx36",
				"Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "Invalid",
				"Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "Invalid", "8Mx36, 2 subbanks" };
		const uint32_t index = (offset >> 1) & 1;
		LOGMASKED(LOG_MEMCFG, "%s: Memory Configuration Register %d Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		LOGMASKED(LOG_MEMCFG_EXT, "%s:     Bank %d, Base Address: %02x\n", machine().describe_context(), index, (data >> 16) & 0xff);
		LOGMASKED(LOG_MEMCFG_EXT, "%s:     Bank %d, SIMM Size: %02x (%s)\n", machine().describe_context(), index, (data >> 24) & 0x1f, s_mem_size[(data >> 24) & 0x1f]);
		LOGMASKED(LOG_MEMCFG_EXT, "%s:     Bank %d, Valid: %d\n", machine().describe_context(), index, BIT(data, 29));
		LOGMASKED(LOG_MEMCFG_EXT, "%s:     Bank %d, # Subbanks: %d\n", machine().describe_context(), index, BIT(data, 30) + 1);
		LOGMASKED(LOG_MEMCFG_EXT, "%s:     Bank %d, Base Address: %02x\n", machine().describe_context(), index+1, data & 0xff);
		LOGMASKED(LOG_MEMCFG_EXT, "%s:     Bank %d, SIMM Size: %02x (%s)\n", machine().describe_context(), index+1, (data >> 8) & 0x1f, s_mem_size[(data >> 8) & 0x1f]);
		LOGMASKED(LOG_MEMCFG_EXT, "%s:     Bank %d, Valid: %d\n", machine().describe_context(), index+1, BIT(data, 13));
		LOGMASKED(LOG_MEMCFG_EXT, "%s:     Bank %d, # Subbanks: %d\n", machine().describe_context(), index+1, BIT(data, 14) + 1);
		m_mem_config[index] = data;
		break;
	}
	case 0x00d0/4:
		LOGMASKED(LOG_WRITES, "%s: CPU Memory Access Config Params Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_cpu_mem_access_config = data;
		break;
	case 0x00d8/4:
		LOGMASKED(LOG_WRITES, "%s: GIO Memory Access Config Params Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio_mem_access_config = data;
		break;
	case 0x00e8/4:
		LOGMASKED(LOG_WRITES, "%s: CPU Error Status Clear\n", machine().describe_context());
		m_maincpu->set_input_line(4, CLEAR_LINE);
		m_cpu_error_status = 0;
		break;
	case 0x00f8/4:
		LOGMASKED(LOG_WRITES, "%s: GIO Error Status Clear\n", machine().describe_context());
		m_maincpu->set_input_line(4, CLEAR_LINE);
		m_gio_error_status = 0;
		break;
	case 0x0100/4:
		LOGMASKED(LOG_WRITES, "%s: System Semaphore Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_sys_semaphore = data;
		break;
	case 0x0108/4:
		LOGMASKED(LOG_WRITES, "%s: GIO Lock Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio_lock = data;
		break;
	case 0x0110/4:
		LOGMASKED(LOG_WRITES, "%s: EISA Lock Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_eisa_lock = data;
		break;
	case 0x0150/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: GIO64 Translation Address Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio64_translate_mask = data;
		break;
	case 0x0158/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: GIO64 Translation Address Substitution Bits Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio64_substitute_bits = data;
		break;
	case 0x0160/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Interrupt Cause Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_int_cause = data;
		if (m_dma_int_cause == 0)
			m_int_dma_done_cb(CLEAR_LINE);
		break;
	case 0x0168/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Control Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_control = data;
		break;
	case 0x0180/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA TLB Entry 0 High Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry_hi[0] = data;
		break;
	case 0x0188/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA TLB Entry 0 Low Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry_lo[0] = data;
		break;
	case 0x0190/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA TLB Entry 1 High Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry_hi[1] = data;
		break;
	case 0x0198/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA TLB Entry 1 Low Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry_lo[1] = data;
		break;
	case 0x01a0/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA TLB Entry 2 High Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry_hi[2] = data;
		break;
	case 0x01a8/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA TLB Entry 2 Low Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry_lo[2] = data;
		break;
	case 0x01b0/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA TLB Entry 3 High Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry_hi[3] = data;
		break;
	case 0x01b8/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA TLB Entry 3 Low Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry_lo[3] = data;
		break;
	case 0x2000/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Memory Address Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_mem_addr = data;
		break;
	case 0x2008/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Memory Address + Default Params Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_mem_addr = data;
		break;
	case 0x2010/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Line Count and Width Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_count &= 0xffff0000;
		m_dma_count |= data & 0x0000ffff;
		m_dma_size = data;
		break;
	case 0x2018/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Line Zoom and Stride Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_count &= 0x0000ffff;
		m_dma_count |= data & 0x03ff0000;
		m_dma_stride = data;
		break;
	case 0x2020/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA GIO64 Address Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_gio64_addr = data;
		break;
	case 0x2028/4:
	{
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA GIO64 Address Write + Start DMA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_gio64_addr = data;
		m_dma_run |= 0x40;
		m_dma_timer->adjust(attotime::from_ticks(4, 33333333), 0, attotime::from_hz(33333333));
		break;
	}
	case 0x2030/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Mode Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_mode = data;
		break;
	case 0x2038/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Zoom Count + Byte Count Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_count = data;
		break;
	case 0x2040/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA Start Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (data & 1)
		{
			m_dma_run |= 0x40;
			m_dma_timer->adjust(attotime::from_hz(33333333), 0, attotime::from_hz(33333333));
		}
		break;
	case 0x2070/4:
		LOGMASKED(LOG_WRITES | LOG_DMA, "%s: DMA GIO64 Address Write + Default Params Write + Start DMA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_gio64_addr = data;
		m_dma_size = 0x0001000c;
		m_dma_stride = 0x00010000;
		m_dma_count = 0x0001000c;
		m_dma_mode = 0x00000028;
		m_dma_run |= 0x40;
		m_dma_timer->adjust(attotime::from_hz(33333333), 0, attotime::from_hz(33333333));
		break;
	case 0x10000/4:
	case 0x11000/4:
	case 0x12000/4:
	case 0x13000/4:
	case 0x14000/4:
	case 0x15000/4:
	case 0x16000/4:
	case 0x17000/4:
	case 0x18000/4:
	case 0x19000/4:
	case 0x1a000/4:
	case 0x1b000/4:
	case 0x1c000/4:
	case 0x1d000/4:
	case 0x1e000/4:
	case 0x1f000/4:
	{
		const uint32_t index = (offset - 0x10000/4) >> 10;
		LOGMASKED(LOG_WRITES, "%s: Semaphore %d Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		m_semaphore[index] = data & 1;
		break;
	}
	default:
		LOGMASKED(LOG_WRITES | LOG_UNKNOWN, "%s: Unmapped MC write: %08x: %08x & %08x\n", machine().describe_context(), 0x1fa00000 + offset*4, data, mem_mask);
		break;
	}
}

void sgi_mc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_RPSS)
	{
		m_rpss_divide_counter--;
		if (m_rpss_divide_counter < 0)
		{
			m_rpss_divide_counter = m_rpss_divide_count;
			m_rpss_counter += m_rpss_increment;
		}
	}
	else if (id == TIMER_DMA)
	{
		dma_immediate();
	}
}
