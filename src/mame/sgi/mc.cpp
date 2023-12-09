// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    mc.cpp

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#include "emu.h"
#include "mc.h"

#define LOG_UNKNOWN     (1U << 1)
#define LOG_READS       (1U << 2)
#define LOG_WRITES      (1U << 3)
#define LOG_RPSS        (1U << 4)
#define LOG_WATCHDOG    (1U << 5)
#define LOG_MEMCFG      (1U << 6)
#define LOG_MEMCFG_EXT  (1U << 7)
#define LOG_EEPROM      (1U << 8)
#define LOG_DMA         (1U << 9)
#define LOG_DEFAULT     (LOG_READS | LOG_WRITES | LOG_RPSS | LOG_WATCHDOG | LOG_UNKNOWN)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_MC, sgi_mc_device, "sgi_mc", "SGI Memory Controller")

sgi_mc_device::sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_MC, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_eeprom(*this, finder_base::DUMMY_TAG)
	, m_simms(*this, "SIMMS")
	, m_int_dma_done_cb(*this)
	, m_eisa_present(*this, true)
	, m_dma_timer(nullptr)
	, m_last_update_time(attotime::zero)
	, m_watchdog(0)
	, m_sys_id(0)
	, m_rpss_divider(0)
	, m_refcnt_preload(0)
	, m_refcnt(0)
	, m_gio64_arb_param(0)
	, m_arb_cpu_time(0)
	, m_arb_burst_time(0)
	, m_memcfg{}
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
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sgi_mc_device::device_start()
{
	m_sys_id = 0x03; // rev. C MC
	m_sys_id |= m_eisa_present() << 4;

	m_dma_timer = timer_alloc(FUNC(sgi_mc_device::perform_dma), this);
	m_dma_timer->adjust(attotime::never);

	save_item(NAME(m_last_update_time));
	save_item(NAME(m_cpu_control));
	save_item(NAME(m_watchdog));
	save_item(NAME(m_sys_id));
	save_item(NAME(m_rpss_divider));
	save_item(NAME(m_refcnt_preload));
	save_item(NAME(m_refcnt));
	save_item(NAME(m_gio64_arb_param));
	save_item(NAME(m_arb_cpu_time));
	save_item(NAME(m_arb_burst_time));
	save_item(NAME(m_memcfg));
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

	m_space = &m_maincpu->space(AS_PROGRAM);

	u16 const simms = m_simms->read();

	// allocate installed memory
	for (unsigned bank = 0; bank < 4; bank++)
	{
		u32 const size = BIT(simms, bank * 4, 3);

		switch (size)
		{
		case 0:
			LOG("ram bank %c empty\n", bank + 'A');
			m_ram[bank].reset();
			break;
		default:
			LOG("ram bank %c size %dM (%s rank)\n",
				bank + 'A', 1U << (size + 1), BIT(simms, bank * 4 + 3) ? "dual" : "single");
			m_ram[bank] = std::make_unique<u8[]>(1U << (size + 21));
			break;
		}
	}

	// assume memory configuration is invalidated by reset
	memcfg_w(0, 0);
	memcfg_w(1, 0);
}

void sgi_mc_device::set_cpu_buserr(uint32_t address, uint64_t mem_mask)
{
	m_cpu_error_addr = address;
	m_cpu_error_status = 0x00000400;
	uint64_t mask = 0x00000000000000ffULL;
	for (uint32_t bit = 0; bit < 8; bit++)
	{
		if (mem_mask & mask)
		{
			m_cpu_error_status |= (1 << bit);
		}
		mask <<= 8;
	}
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

TIMER_CALLBACK_MEMBER(sgi_mc_device::perform_dma)
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

void sgi_mc_device::update_count()
{
	const uint32_t divide = (m_rpss_divider & 0xff) + 1;
	const uint32_t increment = (m_rpss_divider >> 8) & 0xff;
	const uint32_t freq = clock() / divide;
	const attotime elapsed = machine().scheduler().time() - m_last_update_time;
	/* Quantise elapsed to the clock frequency */
	const auto ticks = elapsed.as_ticks(freq);
	m_last_update_time += attotime::from_ticks(ticks, freq);
	m_rpss_counter += uint32_t(ticks * increment);
}

uint32_t sgi_mc_device::read(offs_t offset, uint32_t mem_mask)
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
		LOGMASKED(LOG_MEMCFG, "%s: Memory Configuration Register %d Read: %08x & %08x\n", machine().describe_context(), index, m_memcfg[index], mem_mask);
		return m_memcfg[index];
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
		LOGMASKED(LOG_RPSS, "%s: RPSS Counter Read: %08x & %08x\n", machine().describe_context(), m_rpss_counter, mem_mask);
		update_count();
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

void sgi_mc_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
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
		update_count();
		m_rpss_divider = data;
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
		memcfg_w(index, data);
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

/*
 * When the configured memory size for a bank exceeds the physical memory
 * installed in the bank, the physical memory is mirrored in the configured
 * range. When dual ranks are configured, the configured space is divided in
 * two and each rank of the installed memory mirrored in the corresponding
 * half. When the configured and installed number of ranks do not match, either
 * half of the installed memory will be inaccessible, or half of the configured
 * range will not be mapped to anything.
 *
 * The lowest 512KiB of whatever is mapped into physical memory segment 0
 * (address 0x0800'0000) is also mapped at address 0.
 */
void sgi_mc_device::memcfg_w(offs_t offset, u32 data)
{
	LOGMASKED(LOG_MEMCFG, "memcfg%d 0x%08x\n", offset, data);

	 // remove existing mapping
	for (unsigned bank = 0; bank < 2; bank++)
	{
		// if previous memory configuration was valid, unmap the whole configured range
		if (BIT(m_memcfg[offset], 29 - bank * 16))
		{
			u32 const conf_base = BIT(m_memcfg[offset], 16 - bank * 16, 8) << 22;
			u32 const conf_size = (BIT(m_memcfg[offset], 24 - bank * 16, 5) + 1) << 22;

			LOGMASKED(LOG_MEMCFG, "unmap bank %c 0x%08x-0x%08x\n",
				offset * 2 + bank + 'A', conf_base, conf_base + conf_size - 1);
			m_space->unmap_readwrite(conf_base, conf_base + conf_size - 1);

			if (conf_base == 0x0800'0000U)
				m_space->unmap_readwrite(0x0000'0000, 0x0007'ffff);
		}
	}

	m_memcfg[offset] = data;

	u16 const simms = m_simms->read();

	// install ram into memory map
	for (unsigned bank = 0; bank < 2; bank++)
	{
		// bank configuration valid?
		if (!BIT(m_memcfg[offset], 29 - bank * 16))
			continue;

		// simms installed in bank?
		if (!BIT(simms, (offset * 2 + bank) * 4, 3))
			continue;

		// configured base, rank and size/rank
		u32 const conf_base = BIT(m_memcfg[offset], 16 - bank * 16, 8) << 22;
		unsigned const conf_rank = BIT(m_memcfg[offset], 30 - bank * 16);
		u32 const conf_size = (BIT(m_memcfg[offset], 24 - bank * 16, 5) + 1) << (22 - conf_rank);

		// installed rank and size/rank
		unsigned const inst_rank = BIT(simms, (offset * 2 + bank) * 4 + 3);
		u32 const inst_size = 1U << (BIT(simms, (offset * 2 + bank) * 4, 3) + 21 - inst_rank);

		// resulting number of ranks and size/rank
		unsigned const ranks = std::min(conf_rank, inst_rank) + 1;
		u32 const size = std::min(conf_size, inst_size);

		for (unsigned rank = 0; rank < ranks; rank++)
		{
			LOGMASKED(LOG_MEMCFG, "remap bank %c rank %d from 0x%08x-0x%08x mirror 0x%08x size 0x%08x offset 0x%08x\n",
				offset * 2 + bank + 'A', rank, conf_base + conf_size * rank, conf_base + conf_size * rank + size - 1, (conf_size - 1) ^ (size - 1), size, size * rank);
			m_space->install_ram(conf_base + conf_size * rank, conf_base + conf_size * rank + size - 1, (conf_size - 1) ^ (size - 1), &m_ram[offset * 2 + bank][size * rank]);
		}

		if (conf_base == 0x0800'0000U)
			m_space->install_ram(0x0000'0000, 0x0007'ffff, &m_ram[offset * 2 + bank][0]);
	}
}

static INPUT_PORTS_START(mc)
	PORT_START("VALID")
	PORT_CONFNAME(0x0f, 0x00, "Valid Banks")

	PORT_START("SIMMS")
	PORT_CONFNAME(0x000f, 0x0003, "RAM bank A") PORT_CONDITION("VALID", 0x01, EQUALS, 0x01)
	PORT_CONFSETTING(0x0000, "Empty")
	PORT_CONFSETTING(0x0001, "4x1M")
	PORT_CONFSETTING(0x000a, "4x2M")
	PORT_CONFSETTING(0x0003, "4x4M")
	PORT_CONFSETTING(0x000c, "4x8M")
	PORT_CONFSETTING(0x0005, "4x16M")
	PORT_CONFSETTING(0x000e, "4x32M")

	PORT_CONFNAME(0x00f0, 0x0000, "RAM bank B") PORT_CONDITION("VALID", 0x02, EQUALS, 0x02)
	PORT_CONFSETTING(0x0000, "Empty")
	PORT_CONFSETTING(0x0010, "4x1M")
	PORT_CONFSETTING(0x00a0, "4x2M")
	PORT_CONFSETTING(0x0030, "4x4M")
	PORT_CONFSETTING(0x00c0, "4x8M")
	PORT_CONFSETTING(0x0050, "4x16M")
	PORT_CONFSETTING(0x00e0, "4x32M")

	PORT_CONFNAME(0x0f00, 0x0000, "RAM bank C") PORT_CONDITION("VALID", 0x04, EQUALS, 0x04)
	PORT_CONFSETTING(0x0000, "Empty")
	PORT_CONFSETTING(0x0100, "4x1M")
	PORT_CONFSETTING(0x0a00, "4x2M")
	PORT_CONFSETTING(0x0300, "4x4M")
	PORT_CONFSETTING(0x0c00, "4x8M")
	PORT_CONFSETTING(0x0500, "4x16M")
	PORT_CONFSETTING(0x0e00, "4x32M")

	PORT_CONFNAME(0xf000, 0x0000, "RAM bank D") PORT_CONDITION("VALID", 0x08, EQUALS, 0x08)
	PORT_CONFSETTING(0x0000, "Empty")
	PORT_CONFSETTING(0x1000, "4x1M")
	PORT_CONFSETTING(0xa000, "4x2M")
	PORT_CONFSETTING(0x3000, "4x4M")
	PORT_CONFSETTING(0xc000, "4x8M")
	PORT_CONFSETTING(0x5000, "4x16M")
	PORT_CONFSETTING(0xe000, "4x32M")
INPUT_PORTS_END

ioport_constructor sgi_mc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mc);
}
