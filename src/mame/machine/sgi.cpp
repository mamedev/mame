// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    sgi.c

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#include "emu.h"
#include "sgi.h"

#define LOG_READS		(1 << 0)
#define LOG_WRITES		(1 << 1)
#define LOG_WATCHDOG	(1 << 2)
#define LOG_MEMCFG		(1 << 3)
#define LOG_UNKNOWN		(1 << 4)
#define LOG_DEFAULT		(LOG_READS | LOG_WRITES | LOG_WATCHDOG)

#define VERBOSE			(0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_MC, sgi_mc_device, "sgi_mc", "SGI Memory Controller")

sgi_mc_device::sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_MC, tag, owner, clock),
		m_rpss_timer(nullptr),
		m_cpu_control0(0),
		m_cpu_control1(0),
		m_watchdog(0),
		m_sys_id(0),
		m_rpss_divider(0),
		m_refcnt_preload(0),
		m_refcnt(0),
		m_gio64_arb_param(0),
		m_arb_cpu_time(0),
		m_arb_burst_time(0),
		m_mem_config0(0),
		m_mem_config1(0),
		m_cpu_mem_access_config(0),
		m_gio_mem_access_config(0),
		m_cpu_error_addr(0),
		m_cpu_error_status(0),
		m_gio_error_addr(0),
		m_gio_error_status(0),
		m_sys_semaphore(0),
		m_gio_lock(0),
		m_eisa_lock(0),
		m_gio64_translate_mask(0),
		m_gio64_substitute_bits(0),
		m_dma_int_cause(0),
		m_dma_control(0),
		m_dma_tlb_entry0_hi(0),
		m_dma_tlb_entry0_lo(0),
		m_dma_tlb_entry1_hi(0),
		m_dma_tlb_entry1_lo(0),
		m_dma_tlb_entry2_hi(0),
		m_dma_tlb_entry2_lo(0),
		m_dma_tlb_entry3_hi(0),
		m_dma_tlb_entry3_lo(0),
		m_rpss_counter(0),
		m_dma_mem_addr(0),
		m_dma_size(0),
		m_dma_stride(0),
		m_dma_gio64_addr(0),
		m_dma_mode(0),
		m_dma_count(0),
		m_dma_running(0),
		m_rpss_divide_counter(0),
		m_rpss_divide_count(0),
		m_rpss_increment(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sgi_mc_device::device_start()
{
	// if Indigo2, ID appropriately
	if (!strcmp(machine().system().name, "ip244415"))
	{
		m_sys_id = 0x11; // rev. B MC, EISA bus present
	}
	else
	{
		m_sys_id = 0x01; // rev. B MC, no EISA bus
	}

	m_rpss_timer = timer_alloc(TIMER_RPSS);
	m_rpss_timer->adjust(attotime::never);

	save_item(NAME(m_cpu_control0));
	save_item(NAME(m_cpu_control1));
	save_item(NAME(m_watchdog));
	save_item(NAME(m_sys_id));
	save_item(NAME(m_rpss_divider));
	save_item(NAME(m_refcnt_preload));
	save_item(NAME(m_refcnt));
	save_item(NAME(m_gio64_arb_param));
	save_item(NAME(m_arb_cpu_time));
	save_item(NAME(m_arb_burst_time));
	save_item(NAME(m_mem_config0));
	save_item(NAME(m_mem_config1));
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
	save_item(NAME(m_dma_tlb_entry0_hi));
	save_item(NAME(m_dma_tlb_entry0_lo));
	save_item(NAME(m_dma_tlb_entry1_hi));
	save_item(NAME(m_dma_tlb_entry1_lo));
	save_item(NAME(m_dma_tlb_entry2_hi));
	save_item(NAME(m_dma_tlb_entry2_lo));
	save_item(NAME(m_dma_tlb_entry3_hi));
	save_item(NAME(m_dma_tlb_entry3_lo));
	save_item(NAME(m_rpss_counter));
	save_item(NAME(m_dma_mem_addr));
	save_item(NAME(m_dma_size));
	save_item(NAME(m_dma_stride));
	save_item(NAME(m_dma_gio64_addr));
	save_item(NAME(m_dma_mode));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_dma_running));
	save_item(NAME(m_semaphore));
	save_item(NAME(m_rpss_divide_counter));
	save_item(NAME(m_rpss_divide_count));
	save_item(NAME(m_rpss_increment));
}

void sgi_mc_device::device_reset()
{
	memset(m_semaphore, 0, sizeof(uint32_t) * 16);
	m_rpss_timer->adjust(attotime::from_hz(10000000), 0, attotime::from_hz(10000000));
	m_rpss_divider = 0x0104;
	m_rpss_divide_counter = 4;
	m_rpss_divide_count = 4;
	m_rpss_increment = 1;
}

READ32_MEMBER(sgi_mc_device::read)
{
	offset <<= 2;
	switch (offset & ~4)
	{
	case 0x0000:
		LOGMASKED(LOG_READS, "%s: CPU Control 0 Read: %08x & %08x\n", machine().describe_context(), m_cpu_control0, mem_mask);
		return m_cpu_control0;
	case 0x0008:
		LOGMASKED(LOG_READS, "%s: CPU Control 1 Read: %08x & %08x\n", machine().describe_context(), m_cpu_control1, mem_mask);
		return m_cpu_control1;
	case 0x0010:
		LOGMASKED(LOG_WATCHDOG, "%s: Watchdog Timer Read: %08x & %08x\n", machine().describe_context(), m_watchdog, mem_mask);
		return m_watchdog;
	case 0x0018:
		LOGMASKED(LOG_READS, "%s: System ID Read: %08x & %08x\n", machine().describe_context(), m_sys_id, mem_mask);
		return m_sys_id;
	case 0x0028:
		LOGMASKED(LOG_READS, "%s: RPSS Divider Read: %08x & %08x\n", machine().describe_context(), m_rpss_divider, mem_mask);
		return m_rpss_divider;
	case 0x0030:
		LOGMASKED(LOG_READS, "%s: R4000 EEPROM Read\n", machine().describe_context());
		return 0;
	case 0x0040:
		LOGMASKED(LOG_READS, "%s: Refresh Count Preload Read: %08x & %08x\n", machine().describe_context(), m_refcnt_preload, mem_mask);
		return m_refcnt_preload;
	case 0x0048:
		LOGMASKED(LOG_READS, "%s: Refresh Count Read: %08x & %08x\n", machine().describe_context(), m_refcnt, mem_mask);
		return m_refcnt;
	case 0x0080:
		LOGMASKED(LOG_READS, "%s: GIO64 Arbitration Param Read: %08x & %08x\n", machine().describe_context(), m_gio64_arb_param, mem_mask);
		return m_gio64_arb_param;
	case 0x0088:
		LOGMASKED(LOG_READS, "%s: Arbiter CPU Time Read: %08x & %08x\n", machine().describe_context(), m_arb_cpu_time, mem_mask);
		return m_arb_cpu_time;
	case 0x0098:
		LOGMASKED(LOG_READS, "%s: Arbiter Long Burst Time Read: %08x & %08x\n", machine().describe_context(), m_arb_burst_time, mem_mask);
		return m_arb_burst_time;
	case 0x00c0:
		LOGMASKED(LOG_MEMCFG, "%s: Memory Configuration Register 0 Read: %08x & %08x\n", machine().describe_context(), m_mem_config0, mem_mask);
		return m_mem_config0;
	case 0x00c8:
		LOGMASKED(LOG_MEMCFG, "%s: Memory Configuration Register 1 Read: %08x & %08x\n", machine().describe_context(), m_mem_config1, mem_mask);
		return m_mem_config1;
	case 0x00d0:
		LOGMASKED(LOG_READS, "%s: CPU Memory Access Config Params Read: %08x & %08x\n", machine().describe_context(), m_cpu_mem_access_config, mem_mask);
		return m_cpu_mem_access_config;
	case 0x00d8:
		LOGMASKED(LOG_READS, "%s: GIO Memory Access Config Params Read: %08x & %08x\n", machine().describe_context(), m_gio_mem_access_config, mem_mask);
		return m_gio_mem_access_config;
	case 0x00e0:
		LOGMASKED(LOG_READS, "%s: CPU Error Address Read: %08x & %08x\n", machine().describe_context(), m_cpu_error_addr, mem_mask);
		return m_cpu_error_addr;
	case 0x00e8:
		LOGMASKED(LOG_READS, "%s: CPU Error Status Read: %08x & %08x\n", machine().describe_context(), m_cpu_error_status, mem_mask);
		return m_cpu_error_status;
	case 0x00f0:
		LOGMASKED(LOG_READS, "%s: GIO Error Address Read: %08x & %08x\n", machine().describe_context(), m_gio_error_addr, mem_mask);
		return m_gio_error_addr;
	case 0x00f8:
		LOGMASKED(LOG_READS, "%s: GIO Error Status Read: %08x & %08x\n", machine().describe_context(), m_gio_error_status, mem_mask);
		return m_gio_error_status;
	case 0x0100:
		LOGMASKED(LOG_READS, "%s: System Semaphore Read: %08x & %08x\n", machine().describe_context(), m_sys_semaphore, mem_mask);
		return m_sys_semaphore;
	case 0x0108:
		LOGMASKED(LOG_READS, "%s: GIO Lock Read: %08x & %08x\n", machine().describe_context(), m_gio_lock, mem_mask);
		return m_gio_lock;
	case 0x0110:
		LOGMASKED(LOG_READS, "%s: EISA Lock Read: %08x & %08x\n", machine().describe_context(), m_eisa_lock, mem_mask);
		return m_eisa_lock;
	case 0x0150:
		LOGMASKED(LOG_READS, "%s: GIO64 Translation Address Mask Read: %08x & %08x\n", machine().describe_context(), m_gio64_translate_mask, mem_mask);
		return m_gio64_translate_mask;
	case 0x0158:
		LOGMASKED(LOG_READS, "%s: GIO64 Translation Address Substitution Bits Read: %08x & %08x\n", machine().describe_context(), m_gio64_substitute_bits, mem_mask);
		return m_gio64_substitute_bits;
	case 0x0160:
		LOGMASKED(LOG_READS, "%s: DMA Interrupt Cause: %08x & %08x\n", machine().describe_context(), m_dma_int_cause, mem_mask);
		return m_dma_int_cause;
	case 0x0168:
		LOGMASKED(LOG_READS, "%s: DMA Control Read: %08x & %08x\n", machine().describe_context(), m_dma_control, mem_mask);
		return m_dma_control;
	case 0x0180:
		LOGMASKED(LOG_READS, "%s: DMA TLB Entry 0 High Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry0_hi, mem_mask);
		return m_dma_tlb_entry0_hi;
	case 0x0188:
		LOGMASKED(LOG_READS, "%s: DMA TLB Entry 0 Low Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry0_lo, mem_mask);
		return m_dma_tlb_entry0_lo;
	case 0x0190:
		LOGMASKED(LOG_READS, "%s: DMA TLB Entry 1 High Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry1_hi, mem_mask);
		return m_dma_tlb_entry1_hi;
	case 0x0198:
		LOGMASKED(LOG_READS, "%s: DMA TLB Entry 1 Low Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry1_lo, mem_mask);
		return m_dma_tlb_entry1_lo;
	case 0x01a0:
		LOGMASKED(LOG_READS, "%s: DMA TLB Entry 2 High Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry2_hi, mem_mask);
		return m_dma_tlb_entry2_hi;
	case 0x01a8:
		LOGMASKED(LOG_READS, "%s: DMA TLB Entry 2 Low Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry2_lo, mem_mask);
		return m_dma_tlb_entry2_lo;
	case 0x01b0:
		LOGMASKED(LOG_READS, "%s: DMA TLB Entry 3 High Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry3_hi, mem_mask);
		return m_dma_tlb_entry3_hi;
	case 0x01b8:
		LOGMASKED(LOG_READS, "%s: DMA TLB Entry 3 Low Read: %08x & %08x\n", machine().describe_context(), m_dma_tlb_entry3_lo, mem_mask);
		return m_dma_tlb_entry3_lo;
	case 0x1000:
		LOGMASKED(LOG_READS, "%s: RPSS 100ns Counter Read: %08x & %08x\n", machine().describe_context(), m_rpss_counter, mem_mask);
		return m_rpss_counter;
	case 0x2000:
	case 0x2008:
		LOGMASKED(LOG_READS, "%s: DMA Memory Address Read: %08x & %08x\n", machine().describe_context(), m_dma_mem_addr, mem_mask);
		return m_dma_mem_addr;
	case 0x2010:
		LOGMASKED(LOG_READS, "%s: DMA Line Count and Width Read: %08x & %08x\n", machine().describe_context(), m_dma_size, mem_mask);
		return m_dma_size;
	case 0x2018:
		LOGMASKED(LOG_READS, "%s: DMA Line Zoom and Stride Read: %08x & %08x\n", machine().describe_context(), m_dma_stride, mem_mask);
		return m_dma_stride;
	case 0x2020:
	case 0x2028:
		LOGMASKED(LOG_READS, "%s: DMA GIO64 Address Read: %08x & %08x\n", machine().describe_context(), m_dma_gio64_addr, mem_mask);
		return m_dma_gio64_addr;
	case 0x2030:
		LOGMASKED(LOG_READS, "%s: DMA Mode Write: %08x & %08x\n", machine().describe_context(), m_dma_mode, mem_mask);
		return m_dma_mode;
	case 0x2038:
		LOGMASKED(LOG_READS, "%s: DMA Zoom Count Read: %08x & %08x\n", machine().describe_context(), m_dma_count, mem_mask);
		return m_dma_count;
	case 0x2040:
		LOGMASKED(LOG_READS, "%s: DMA Start Read\n", machine().describe_context());
		return 0;
	case 0x2048:
		LOGMASKED(LOG_READS, "%s: VDMA Running Read: %08x & %08x\n", machine().describe_context(), m_dma_running, mem_mask);
		if (m_dma_running == 1)
		{
			m_dma_running = 0;
			return 0x00000040;
		}
		else
		{
			return 0;
		}
	case 0x10000:
	case 0x11000:
	case 0x12000:
	case 0x13000:
	case 0x14000:
	case 0x15000:
	case 0x16000:
	case 0x17000:
	case 0x18000:
	case 0x19000:
	case 0x1a000:
	case 0x1b000:
	case 0x1c000:
	case 0x1d000:
	case 0x1e000:
	case 0x1f000:
	{
		const uint32_t data = m_semaphore[(offset - 0x10000) >> 12];
		LOGMASKED(LOG_READS, "%s: Semaphore %d Read: %08x & %08x\n", machine().describe_context(), (offset - 0x10000) >> 12, data, mem_mask);
		m_semaphore[(offset - 0x10000) >> 12] = 1;
		return data;
	}
	default:
		LOGMASKED(LOG_READS | LOG_UNKNOWN, "%s: Unmapped MC read: %08x & %08x\n", machine().describe_context(), 0x1fa00000 + offset, mem_mask);
		return 0;
	}
	return 0;
}

WRITE32_MEMBER( sgi_mc_device::write )
{
	offset <<= 2;
	switch (offset & ~4)
	{
	case 0x0000:
		LOGMASKED(LOG_WRITES, "%s: CPU Control 0 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_cpu_control0 = data;
		break;
	case 0x0008:
		LOGMASKED(LOG_WRITES, "%s: CPU Control 1 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_cpu_control1 = data;
		break;
	case 0x0010:
		LOGMASKED(LOG_WATCHDOG, "%s: Watchdog Timer Clear\n", machine().describe_context());
		m_watchdog = 0;
		break;
	case 0x0028:
		LOGMASKED(LOG_WRITES, "%s: RPSS Divider Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_rpss_divider = data;
		m_rpss_divide_count = (int)(m_rpss_divider & 0xff);
		m_rpss_divide_counter = m_rpss_divide_count;
		m_rpss_increment = (m_rpss_divider >> 8) & 0xff;
		break;
	case 0x0030:
		LOGMASKED(LOG_WRITES, "%s: R4000 EEPROM Write: CS:%d, SCK:%d, SO:%d\n", machine().describe_context(),
			BIT(data, 1), BIT(data, 2), BIT(data, 3));
		break;
	case 0x0040:
		LOGMASKED(LOG_WRITES, "%s: Refresh Count Preload Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_refcnt_preload = data;
		break;
	case 0x0080:
		LOGMASKED(LOG_WRITES, "%s: GIO64 Arbitration Param Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio64_arb_param = data;
		break;
	case 0x0088:
		LOGMASKED(LOG_WRITES, "%s: Arbiter CPU Time Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_arb_cpu_time = data;
		break;
	case 0x0098:
		LOGMASKED(LOG_WRITES, "%s: Arbiter Long Burst Time Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_arb_burst_time = data;
		break;
	case 0x00c0:
		LOGMASKED(LOG_MEMCFG, "%s: Memory Configuration Register 0 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_mem_config0 = data;
		break;
	case 0x00c8:
		LOGMASKED(LOG_MEMCFG, "%s: Memory Configuration Register 1 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_mem_config1 = data;
		break;
	case 0x00d0:
		LOGMASKED(LOG_WRITES, "%s: CPU Memory Access Config Params Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_cpu_mem_access_config = data;
		break;
	case 0x00d8:
		LOGMASKED(LOG_WRITES, "%s: GIO Memory Access Config Params Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio_mem_access_config = data;
		break;
	case 0x00e8:
		LOGMASKED(LOG_WRITES, "%s: CPU Error Status Clear\n", machine().describe_context());
		m_cpu_error_status = 0;
		break;
	case 0x00f8:
		LOGMASKED(LOG_WRITES, "%s: GIO Error Status Clear\n", machine().describe_context());
		m_gio_error_status = 0;
		break;
	case 0x0100:
		LOGMASKED(LOG_WRITES, "%s: System Semaphore Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_sys_semaphore = data;
		break;
	case 0x0108:
		LOGMASKED(LOG_WRITES, "%s: GIO Lock Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio_lock = data;
		break;
	case 0x0110:
		LOGMASKED(LOG_WRITES, "%s: EISA Lock Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_eisa_lock = data;
		break;
	case 0x0150:
		LOGMASKED(LOG_WRITES, "%s: GIO64 Translation Address Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio64_translate_mask = data;
		break;
	case 0x0158:
		LOGMASKED(LOG_WRITES, "%s: GIO64 Translation Address Substitution Bits Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_gio64_substitute_bits = data;
		break;
	case 0x0160:
		LOGMASKED(LOG_WRITES, "%s: DMA Interrupt Cause Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_int_cause = data;
		break;
	case 0x0168:
		LOGMASKED(LOG_WRITES, "%s: DMA Control Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_control = data;
		break;
	case 0x0180:
		LOGMASKED(LOG_WRITES, "%s: DMA TLB Entry 0 High Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry0_hi = data;
		break;
	case 0x0188:
		LOGMASKED(LOG_WRITES, "%s: DMA TLB Entry 0 Low Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry0_lo = data;
		break;
	case 0x0190:
		LOGMASKED(LOG_WRITES, "%s: DMA TLB Entry 1 High Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry1_hi = data;
		break;
	case 0x0198:
		LOGMASKED(LOG_WRITES, "%s: DMA TLB Entry 1 Low Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry1_lo = data;
		break;
	case 0x01a0:
		LOGMASKED(LOG_WRITES, "%s: DMA TLB Entry 2 High Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry2_hi = data;
		break;
	case 0x01a8:
		LOGMASKED(LOG_WRITES, "%s: DMA TLB Entry 2 Low Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry2_lo = data;
		break;
	case 0x01b0:
		LOGMASKED(LOG_WRITES, "%s: DMA TLB Entry 3 High Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry3_hi = data;
		break;
	case 0x01b8:
		LOGMASKED(LOG_WRITES, "%s: DMA TLB Entry 3 Low Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_tlb_entry3_lo = data;
		break;
	case 0x2000:
		LOGMASKED(LOG_WRITES, "%s: DMA Memory Address Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_mem_addr = data;
		break;
	case 0x2008:
		LOGMASKED(LOG_WRITES, "%s: DMA Memory Address + Default Params Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_mem_addr = data;
		break;
	case 0x2010:
		LOGMASKED(LOG_WRITES, "%s: DMA Line Count and Width Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_size = data;
		break;
	case 0x2018:
		LOGMASKED(LOG_WRITES, "%s: DMA Line Zoom and Stride Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_stride = data;
		break;
	case 0x2020:
		LOGMASKED(LOG_WRITES, "%s: DMA GIO64 Address Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_gio64_addr = data;
		break;
	case 0x2028:
		LOGMASKED(LOG_WRITES, "%s: DMA GIO64 Address Write + Start DMA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_gio64_addr = data;
		// Start DMA
		m_dma_running = 1;
		break;
	case 0x2030:
		LOGMASKED(LOG_WRITES, "%s: DMA Mode Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_mode = data;
		break;
	case 0x2038:
		LOGMASKED(LOG_WRITES, "%s: DMA Zoom Count + Byte Count Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_count = data;
		break;
	case 0x2040:
		LOGMASKED(LOG_WRITES, "%s: DMA Start Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		// Start DMA
		m_dma_running = 1;
		break;
	case 0x2070:
		LOGMASKED(LOG_WRITES, "%s: DMA GIO64 Address Write + Default Params Write + Start DMA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_dma_gio64_addr = data;
		// Start DMA
		m_dma_running = 1;
		break;
	case 0x10000:
	case 0x11000:
	case 0x12000:
	case 0x13000:
	case 0x14000:
	case 0x15000:
	case 0x16000:
	case 0x17000:
	case 0x18000:
	case 0x19000:
	case 0x1a000:
	case 0x1b000:
	case 0x1c000:
	case 0x1d000:
	case 0x1e000:
	case 0x1f000:
		LOGMASKED(LOG_WRITES, "%s: Semaphore %d Write: %08x & %08x\n", machine().describe_context(), (offset - 0x10000) >> 12, data, mem_mask);
		m_semaphore[(offset - 0x10000) >> 12] = data & 1;
		break;
	default:
		LOGMASKED(LOG_WRITES | LOG_UNKNOWN, "%s: Unmapped MC write: %08x: %08x & %08x\n", machine().describe_context(), 0x1fa00000 + offset, data, mem_mask);
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
}
