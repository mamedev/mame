// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************\
*
*   SGI Indigo workstation
*
*  Skeleton Driver
*
*  Todo: Everything
*
*  Memory map:
*
*  1fa00000 - 1fa02047      Memory Controller
*  1fb80000 - 1fb9a7ff      HPC1 CHIP0
*  1fc00000 - 1fc7ffff      BIOS
*
\*********************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "bus/scsi/scsihd.h"
#include "cpu/mips/mips1.h"
#include "cpu/mips/mips3.h"
#include "machine/eepromser.h"
#include "machine/pit8253.h"
#include "machine/sgi.h"
#include "machine/wd33c93.h"
#include "machine/z80scc.h"
#include "screen.h"
#include "speaker.h"

#define LOG_UNKNOWN		(1 << 0)
#define LOG_INT			(1 << 1)
#define LOG_HPC			(1 << 2)
#define LOG_RTC			(1 << 3)
#define LOG_EEPROM		(1 << 4)
#define LOG_DMA			(1 << 5)
#define LOG_SCSI		(1 << 6)
#define LOG_SCSI_DMA	(1 << 7)
#define LOG_DUART		(1 << 8)
#define LOG_PIT			(1 << 9)
#define LOG_ALL			(LOG_UNKNOWN | LOG_INT | LOG_HPC | LOG_RTC | LOG_EEPROM | LOG_DMA | LOG_SCSI | LOG_SCSI_DMA | LOG_DUART | LOG_PIT)

#define VERBOSE			(0)
#include "logmacro.h"

class indigo_state : public driver_device
{
public:
	indigo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_wd33c93(*this, "wd33c93")
		, m_scc(*this, "scc%u", 0U)
		, m_eeprom(*this, "eeprom")
		, m_pit(*this, "pit")
	{
	}

	void indigo_base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_RTC = 0;

	DECLARE_READ32_MEMBER(hpc_r);
	DECLARE_WRITE32_MEMBER(hpc_w);
	DECLARE_READ32_MEMBER(int_r);
	DECLARE_WRITE32_MEMBER(int_w);
	DECLARE_WRITE_LINE_MEMBER(scsi_irq);

	void set_timer_int_clear(uint32_t data);
	DECLARE_WRITE_LINE_MEMBER(timer0_int);
	DECLARE_WRITE_LINE_MEMBER(timer1_int);
	DECLARE_WRITE_LINE_MEMBER(timer2_int);
	DECLARE_WRITE_LINE_MEMBER(duart0_int_w);
	DECLARE_WRITE_LINE_MEMBER(duart1_int_w);
	DECLARE_WRITE_LINE_MEMBER(duart2_int_w);

	void duart_int_w(int channel, int status);
	void raise_local0_irq(uint8_t source_mask);
	void lower_local0_irq(uint8_t source_mask);
	void raise_local1_irq(uint8_t source_mask);
	void lower_local1_irq(uint8_t source_mask);

	void fetch_chain(address_space &space);
	void advance_chain(address_space &space);
	void scsi_dma();

	static void cdrom_config(device_t *device);
	void indigo_map(address_map &map);

	enum
	{
		LOCAL0_FIFO_GIO0	= 0x01,
		LOCAL0_PARALLEL		= 0x02,
		LOCAL0_SCSI			= 0x04,
		LOCAL0_ETHERNET		= 0x08,
		LOCAL0_GFX_DMA		= 0x10,
		LOCAL0_DUART		= 0x20,
		LOCAL0_GIO1			= 0x40,
		LOCAL0_VME0			= 0x80,

		LOCAL1_GR1_CASE		= 0x02,
		LOCAL1_VME1			= 0x08,
		LOCAL1_DSP			= 0x10,
		LOCAL1_ACFAIL		= 0x20,
		LOCAL1_VIDEO		= 0x40,
		LOCAL1_RETRACE_GIO2	= 0x80
	};

	struct hpc_t
	{
		uint8_t m_misc_status;
		uint32_t m_parbuf_ptr;
		uint32_t m_local_int0_status;
		uint32_t m_local_int0_mask;
		uint32_t m_local_int1_status;
		uint32_t m_local_int1_mask;
		uint32_t m_vme_intmask0;
		uint32_t m_vme_intmask1;
		uint32_t m_scsi0_dma_desc;
		uint32_t m_scsi0_dma_ctrl;
		uint32_t m_scsi0_dma_addr;
		uint32_t m_scsi0_dma_flag;
		uint32_t m_scsi0_dma_next;
		uint16_t m_scsi0_dma_length;
		bool m_scsi0_dma_to_mem;
		bool m_scsi0_dma_active;
	};

	enum
	{
		HPC_DMACTRL_RESET	= 0x01,
		HPC_DMACTRL_FLUSH	= 0x02,
		HPC_DMACTRL_TO_MEM	= 0x10,
		HPC_DMACTRL_ENABLE	= 0x80
	};

	enum
	{
		RTC_DAYOFWEEK	= 0x0e,
		RTC_YEAR		= 0x0b,
		RTC_MONTH		= 0x0a,
		RTC_DAY			= 0x09,
		RTC_HOUR		= 0x08,
		RTC_MINUTE		= 0x07,
		RTC_SECOND		= 0x06,
		RTC_HUNDREDTH	= 0x05
	};

	struct rtc_t
	{
		uint8_t m_ram[32];
	};

	required_device<cpu_device> m_maincpu;
	required_device<wd33c93_device> m_wd33c93;
	required_device_array<scc85230_device, 3> m_scc;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<pit8254_device> m_pit;

	hpc_t m_hpc;
	rtc_t m_rtc;
	uint8_t m_duart_int_status;

	emu_timer *m_rtc_timer;

	void indigo_timer_rtc();

	static char const *const RS232A_TAG;
	static char const *const RS232B_TAG;

	static const XTAL SCC_PCLK;
	static const XTAL SCC_RXA_CLK;
	static const XTAL SCC_TXA_CLK;
	static const XTAL SCC_RXB_CLK;
	static const XTAL SCC_TXB_CLK;
};

/*static*/ char const *const indigo_state::RS232A_TAG = "rs232a";
/*static*/ char const *const indigo_state::RS232B_TAG = "rs232b";

/*static*/ const XTAL indigo_state::SCC_PCLK = 10_MHz_XTAL;
/*static*/ const XTAL indigo_state::SCC_RXA_CLK = 3.6864_MHz_XTAL; // Needs verification
/*static*/ const XTAL indigo_state::SCC_TXA_CLK = XTAL(0);
/*static*/ const XTAL indigo_state::SCC_RXB_CLK = 3.6864_MHz_XTAL; // Needs verification
/*static*/ const XTAL indigo_state::SCC_TXB_CLK = XTAL(0);

class indigo3k_state : public indigo_state
{
public:
	indigo3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: indigo_state(mconfig, type, tag)
	{
	}

	void indigo3k(machine_config &config);

protected:
	void mem_map(address_map &map);
};

class indigo4k_state : public indigo_state
{
public:
	indigo4k_state(const machine_config &mconfig, device_type type, const char *tag)
		: indigo_state(mconfig, type, tag)
	{
	}

	void indigo4k(machine_config &config);

protected:
	void mem_map(address_map &map);
};

void indigo_state::machine_start()
{
	m_rtc_timer = timer_alloc(TIMER_RTC);
	m_rtc_timer->adjust(attotime::never);

	save_item(NAME(m_hpc.m_misc_status));
	save_item(NAME(m_hpc.m_parbuf_ptr));
	save_item(NAME(m_hpc.m_local_int0_status));
	save_item(NAME(m_hpc.m_local_int0_mask));
	save_item(NAME(m_hpc.m_local_int1_status));
	save_item(NAME(m_hpc.m_local_int1_mask));
	save_item(NAME(m_hpc.m_vme_intmask0));
	save_item(NAME(m_hpc.m_vme_intmask1));

	save_item(NAME(m_hpc.m_scsi0_dma_desc));
	save_item(NAME(m_hpc.m_scsi0_dma_ctrl));
	save_item(NAME(m_hpc.m_scsi0_dma_addr));
	save_item(NAME(m_hpc.m_scsi0_dma_flag));
	save_item(NAME(m_hpc.m_scsi0_dma_next));
	save_item(NAME(m_hpc.m_scsi0_dma_length));
	save_item(NAME(m_hpc.m_scsi0_dma_to_mem));
	save_item(NAME(m_hpc.m_scsi0_dma_active));

	save_item(NAME(m_rtc.m_ram));

	save_item(NAME(m_duart_int_status));
}

void indigo_state::machine_reset()
{
	memset(&m_hpc, 0, sizeof(hpc_t));

	m_duart_int_status = 0;

	// update RTC every 1/100th of a second
	m_rtc_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

READ32_MEMBER(indigo_state::hpc_r)
{
	if (offset >= 0x0e00/4 && offset <= 0x0e7c/4)
	{
		uint8_t ret = m_rtc.m_ram[offset - 0xe00/4];
		LOGMASKED(LOG_RTC, "%s: RTC RAM[0x%02x] Read: %02x\n", machine().describe_context(), offset - 0xe00/4, ret);
		return ret;
	}
	switch (offset)
	{
	case 0x005c/4:
		LOGMASKED(LOG_HPC | LOG_UNKNOWN, "%s: HPC Unknown Read: %08x & %08x\n",
			machine().describe_context(), 0x1fb80000 + offset*4, mem_mask);
		return 0;
	case 0x0094/4:
		LOGMASKED(LOG_HPC | LOG_DMA, "%s: HPC SCSI DMA Control Register Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_scsi0_dma_ctrl, mem_mask);
		return m_hpc.m_scsi0_dma_ctrl;
	case 0x00ac/4:
		LOGMASKED(LOG_HPC, "%s: HPC Parallel Buffer Pointer Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_parbuf_ptr, mem_mask);
		return m_hpc.m_parbuf_ptr;
	case 0x00c0/4:
		LOGMASKED(LOG_HPC, "%s: HPC Endianness Read: %08x & %08x\n", machine().describe_context(), 0x00000040, mem_mask);
		return 0x00000040;
	case 0x0120/4:
	{
		uint32_t ret = m_wd33c93->read(space, 0) << 8;
		LOGMASKED(LOG_SCSI, "%s: HPC SCSI Offset 0 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		return ret;
	}
	case 0x0124/4:
	{
		uint32_t ret = m_wd33c93->read(space, 1) << 8;
		LOGMASKED(LOG_SCSI, "%s: HPC SCSI Offset 1 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		return ret;
	}
	case 0x01b0/4:
		LOGMASKED(LOG_HPC, "%s: HPC Misc. Status Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_misc_status, mem_mask);
		return m_hpc.m_misc_status;
	case 0x01bc/4:
	{
		uint32_t ret = m_eeprom->do_read() << 4;
		LOGMASKED(LOG_EEPROM, "%s: HPC Serial EEPROM Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		return ret;
	}
	case 0x01c0/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 0 Status Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_local_int0_status, mem_mask);
		return m_hpc.m_local_int0_status;
	case 0x01c4/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 0 Mask Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_local_int0_mask, mem_mask);
		return m_hpc.m_local_int0_mask;
	case 0x01c8/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 1 Status Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_local_int1_status, mem_mask);
		return m_hpc.m_local_int1_status;
	case 0x01cc/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 1 Mask Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_local_int1_mask, mem_mask);
		return m_hpc.m_local_int1_mask;
	case 0x01d4/4:
		LOGMASKED(LOG_HPC, "%s: HPC VME Interrupt Mask 0 Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_vme_intmask0, mem_mask);
		return m_hpc.m_vme_intmask0;
	case 0x01d8/4:
		LOGMASKED(LOG_HPC, "%s: HPC VME Interrupt Mask 1 Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_vme_intmask1, mem_mask);
		return m_hpc.m_vme_intmask1;
	case 0x01f0/4:
	{
		const uint8_t data = 0;//m_pit->read(0);
		LOGMASKED(LOG_PIT, "%s: Read Timer Count0 Register (Disabled): %02x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case 0x01f4/4:
	{
		const uint8_t data = 0;//m_pit->read(1);
		LOGMASKED(LOG_PIT, "%s: Read Timer Count1 Register (Disabled): %02x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case 0x01f8/4:
	{
		const uint8_t data = 0;//m_pit->read(2);
		LOGMASKED(LOG_PIT, "%s: Read Timer Count2 Register (Disabled): %02x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case 0x01fc/4:
	{
		const uint8_t data = 0;//m_pit->read(3);
		LOGMASKED(LOG_PIT, "%s: Read Timer Control Register (Disabled): %02x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case 0x0d00/4:
	case 0x0d10/4:
	case 0x0d20/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		uint32_t ret = m_scc[index]->ba_cd_r(space, 3);
		LOGMASKED(LOG_DUART, "%s: HPC DUART%d Channel B Control Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d04/4:
	case 0x0d14/4:
	case 0x0d24/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ba_cd_r(space, 2);
		LOGMASKED(LOG_DUART, "%s: HPC DUART%d Channel B Data Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d08/4:
	case 0x0d18/4:
	case 0x0d28/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ba_cd_r(space, 1);
		LOGMASKED(LOG_DUART, "%s: HPC DUART%d Channel A Control Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d0c/4:
	case 0x0d1c/4:
	case 0x0d2c/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ba_cd_r(space, 0);
		LOGMASKED(LOG_DUART, "%s: HPC DUART%d Channel A Data Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	default:
		LOGMASKED(LOG_HPC | LOG_UNKNOWN, "%s: Unknown HPC Read: %08x & %08x\n", machine().describe_context(), 0x1fb80000 + offset*4, mem_mask);
		return 0;
	}
	return 0;
}

void indigo_state::fetch_chain(address_space &space)
{
	m_hpc.m_scsi0_dma_flag = space.read_dword(m_hpc.m_scsi0_dma_desc);
	m_hpc.m_scsi0_dma_addr = space.read_dword(m_hpc.m_scsi0_dma_desc+4) & 0x7fffffff;
	m_hpc.m_scsi0_dma_next = space.read_dword(m_hpc.m_scsi0_dma_desc+8);
	m_hpc.m_scsi0_dma_length = m_hpc.m_scsi0_dma_flag & 0x3fff;
	LOGMASKED(LOG_HPC | LOG_DMA, "Fetched SCSI DMA Descriptor block:\n");
	LOGMASKED(LOG_HPC | LOG_DMA, "    Ctrl: %08x\n", m_hpc.m_scsi0_dma_flag);
	LOGMASKED(LOG_HPC | LOG_DMA, "    Addr: %08x\n", m_hpc.m_scsi0_dma_addr);
	LOGMASKED(LOG_HPC | LOG_DMA, "    Next: %08x\n", m_hpc.m_scsi0_dma_next);
	LOGMASKED(LOG_HPC | LOG_DMA, "    Length: %04x\n", m_hpc.m_scsi0_dma_length);
}

void indigo_state::advance_chain(address_space &space)
{
	m_hpc.m_scsi0_dma_addr++;
	m_hpc.m_scsi0_dma_length--;
	if (m_hpc.m_scsi0_dma_length == 0)
	{
		m_hpc.m_scsi0_dma_desc = m_hpc.m_scsi0_dma_next;
		fetch_chain(space);
		if (m_hpc.m_scsi0_dma_addr == 0 || m_hpc.m_scsi0_dma_length == 0)
		{
			LOGMASKED(LOG_SCSI_DMA, "HPC: Disabling SCSI DMA due to end of chain\n");
			m_hpc.m_scsi0_dma_active = false;
			m_hpc.m_scsi0_dma_ctrl &= ~HPC_DMACTRL_ENABLE;
		}
	}
}

WRITE32_MEMBER(indigo_state::hpc_w)
{
	if (offset >= 0x0e00/4 && offset <= 0x0e7c/4)
	{
		LOGMASKED(LOG_RTC, "%s: RTC RAM[%02x] Write: %02x\n", machine().describe_context(), offset - 0xe00/4, (uint8_t)data);
		m_rtc.m_ram[offset - 0xe00/4] = (uint8_t)data;

		switch (offset - 0xe00/4)
		{
		case 0:
			break;
		case 4:
			if (!BIT(m_rtc.m_ram[0x00], 7))
			{
				if (BIT(data, 7))
				{
					m_rtc.m_ram[0x19] = m_rtc.m_ram[RTC_SECOND];
					m_rtc.m_ram[0x1a] = m_rtc.m_ram[RTC_MINUTE];
					m_rtc.m_ram[0x1b] = m_rtc.m_ram[RTC_HOUR];
					m_rtc.m_ram[0x1c] = m_rtc.m_ram[RTC_DAY];
					m_rtc.m_ram[0x1d] = m_rtc.m_ram[RTC_MONTH];
				}
			}
			break;
		}
		return;
	}

	switch (offset)
	{
	case 0x0090/4:
		LOGMASKED(LOG_HPC | LOG_DMA, "%s: HPC SCSI DMA Descriptor Pointer: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_scsi0_dma_desc = data;
		fetch_chain(space);
		break;

	case 0x0094/4:
		LOGMASKED(LOG_HPC | LOG_DMA, "%s: HPC SCSI DMA Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_scsi0_dma_ctrl = data &~ (HPC_DMACTRL_FLUSH | HPC_DMACTRL_RESET);
		m_hpc.m_scsi0_dma_to_mem = (m_hpc.m_scsi0_dma_ctrl & HPC_DMACTRL_TO_MEM);
		m_hpc.m_scsi0_dma_active = (m_hpc.m_scsi0_dma_ctrl & HPC_DMACTRL_ENABLE);
		break;

	case 0x00ac/4:
		LOGMASKED(LOG_HPC, "%s: HPC Parallel Buffer Pointer Write: %08x (%08x)\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_parbuf_ptr = data;
		break;
	case 0x0120/4:
		if (ACCESSING_BITS_8_15)
		{
			LOGMASKED(LOG_SCSI, "%s: HPC SCSI Controller Address Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_wd33c93->write(space, 0, (uint8_t)(data >> 8));
		}
		break;
	case 0x0124/4:
		if (ACCESSING_BITS_8_15)
		{
			LOGMASKED(LOG_SCSI, "%s: HPC SCSI Controller Data Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_wd33c93->write(space, 1, (uint8_t)(data >> 8));
		}
		break;
	case 0x01b0/4:
		LOGMASKED(LOG_HPC, "%s: HPC Misc. Status Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (BIT(data, 0))
			LOGMASKED(LOG_HPC, "    Force DSP hard reset\n" );

		if (BIT(data, 1))
			LOGMASKED(LOG_HPC, "    Force IRQA\n" );

		if (BIT(data, 2))
			LOGMASKED(LOG_HPC, "    Set IRQA polarity high\n" );
		else
			LOGMASKED(LOG_HPC, "    Set IRQA polarity low\n" );

		if (BIT(data, 3))
			LOGMASKED(LOG_HPC, "    SRAM size: 32K\n" );
		else
			LOGMASKED(LOG_HPC, "    SRAM size:  8K\n" );

		m_hpc.m_misc_status = data;
		break;
	case 0x01bc/4:
		LOGMASKED(LOG_EEPROM, "%s: HPC Serial EEPROM Write: %08x & %08x\n", machine().describe_context(), data, mem_mask );
		if (BIT(data, 0))
		{
			LOGMASKED(LOG_EEPROM, "    CPU board LED on\n");
		}
		m_eeprom->di_write(BIT(data, 3));
		m_eeprom->cs_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->clk_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case 0x01c0/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 0 Status Write (Ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x01c4/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 0 Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_local_int0_mask = data;
		break;
	case 0x01c8/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 1 Status Write (Ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x01cc/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 1 Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_local_int1_mask = data;
		break;
	case 0x01d4/4:
		LOGMASKED(LOG_HPC, "%s: HPC VME Interrupt Mask 0 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_vme_intmask0 = data;
		break;
	case 0x01d8/4:
		LOGMASKED(LOG_HPC, "%s: HPC VME Interrupt Mask 1 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_vme_intmask1 = data;
		break;
	case 0x01e0/4:
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Interrupt Clear Register: %08x\n", machine().describe_context(), data);
		set_timer_int_clear(data);
		break;
	case 0x01f0/4:
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Count0 Register (Disabled): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		//if (ACCESSING_BITS_24_31)
		//	m_pit->write(0, (uint8_t)(data >> 24));
		//else if (ACCESSING_BITS_0_7)
		//	m_pit->write(0, (uint8_t)data);
		return;
	case 0x01f4/4:
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Count1 Register (Disabled): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		//m_pit->write(1, (uint8_t)data);
		return;
	case 0x01f8/4:
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Count2 Register (Disabled): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		//m_pit->write(2, (uint8_t)data);
		return;
	case 0x01fc/4:
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Control Register (Disabled): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		//m_pit->write(3, (uint8_t)data);
		return;
	case 0x0d00/4:
	case 0x0d10/4:
	case 0x0d20/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ba_cd_w(space, 3, (uint8_t)data);
		LOGMASKED(LOG_DUART, "%s: HPC DUART%d Channel B Control Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d04/4:
	case 0x0d14/4:
	case 0x0d24/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ba_cd_w(space, 2, (uint8_t)data);
		LOGMASKED(LOG_DUART, "%s: HPC DUART%d Channel B Data Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d08/4:
	case 0x0d18/4:
	case 0x0d28/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ba_cd_w(space, 1, (uint8_t)data);
		LOGMASKED(LOG_DUART, "%s: HPC DUART%d Channel A Control Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d0c/4:
	case 0x0d1c/4:
	case 0x0d2c/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ba_cd_w(space, 0, (uint8_t)data);
		LOGMASKED(LOG_DUART, "%s: HPC DUART%d Channel A Data Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	default:
		LOGMASKED(LOG_HPC | LOG_UNKNOWN, "%s: Unknown HPC write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fb80000 + offset*4, data, mem_mask);
		break;
	}
}

void indigo_state::scsi_dma()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int byte_count = m_wd33c93->get_dma_count();

	LOGMASKED(LOG_SCSI_DMA, "HPC: Transferring %d bytes %s %08x %s SCSI0\n",
		byte_count, m_hpc.m_scsi0_dma_to_mem ? "to" : "from", m_hpc.m_scsi0_dma_addr, m_hpc.m_scsi0_dma_to_mem ? "from" : "to");

	uint8_t dma_buffer[512];
	if (m_hpc.m_scsi0_dma_to_mem)
	{
		// HPC SCSI DMA: device to host
		if (byte_count <= 512)
		{
			m_wd33c93->dma_read_data(byte_count, dma_buffer);

			for (int i = 0; i < byte_count; i++)
			{
				LOGMASKED(LOG_SCSI_DMA, "HPC: Reading %02x to %08x\n", dma_buffer[i], m_hpc.m_scsi0_dma_addr);
				space.write_byte(m_hpc.m_scsi0_dma_addr, dma_buffer[i]);
				advance_chain(space);
				if (!m_hpc.m_scsi0_dma_active)
					break;
			}
		}
		else
		{
			while (byte_count)
			{
				int sub_count = m_wd33c93->dma_read_data(512, dma_buffer);

				for (int i = 0; i < sub_count; i++)
				{
					LOGMASKED(LOG_SCSI_DMA, "HPC: Reading %02x to %08x\n", dma_buffer[i], m_hpc.m_scsi0_dma_addr);
					space.write_byte(m_hpc.m_scsi0_dma_addr, dma_buffer[i]);
					advance_chain(space);
					if (!m_hpc.m_scsi0_dma_active)
						break;
				}

				byte_count -= sub_count;
				if (!m_hpc.m_scsi0_dma_active)
					break;
			}
		}
	}
	else
	{
		// HPC SCSI DMA: host to device
		if (byte_count <= 512)
		{
			for (int i = 0; i < byte_count; i++)
			{
				dma_buffer[i] = space.read_byte(m_hpc.m_scsi0_dma_addr);
				LOGMASKED(LOG_SCSI_DMA, "HPC: Writing %02x from %08x\n", dma_buffer[i], m_hpc.m_scsi0_dma_addr);
				advance_chain(space);
				if (!m_hpc.m_scsi0_dma_active)
					break;
			}

			m_wd33c93->dma_write_data(byte_count, dma_buffer);
		}
		else
		{
			while (byte_count)
			{
				int sub_count = std::min(512, byte_count);

				for (int i = 0; i < sub_count; i++)
				{
					dma_buffer[i] = space.read_byte(m_hpc.m_scsi0_dma_addr);
					LOGMASKED(LOG_SCSI_DMA, "HPC: Writing %02x from %08x\n", dma_buffer[i], m_hpc.m_scsi0_dma_addr);
					advance_chain(space);
					if (!m_hpc.m_scsi0_dma_active)
						break;
				}

				m_wd33c93->dma_write_data(sub_count, dma_buffer);

				if (!m_hpc.m_scsi0_dma_active)
				{
					break;
				}
				else
				{
					byte_count -= sub_count;
				}
			}
		}
	}

	// clear DMA on the controller
	m_wd33c93->clear_dma();
}

WRITE_LINE_MEMBER(indigo_state::scsi_irq)
{
	if (state)
	{
		LOGMASKED(LOG_SCSI | LOG_INT, "SCSI: Set IRQ\n");
		int count = m_wd33c93->get_dma_count();
		LOGMASKED(LOG_SCSI, "SCSI: count %d, active %d\n", count, m_hpc.m_scsi0_dma_active);
		if (count && m_hpc.m_scsi0_dma_active)
			scsi_dma();

		raise_local0_irq(LOCAL0_SCSI);
	}
	else
	{
		LOGMASKED(LOG_SCSI | LOG_INT, "SCSI: Clear IRQ\n");
		lower_local0_irq(LOCAL0_SCSI);
	}
}

void indigo_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RTC:
		indigo_timer_rtc();
		break;
	default:
		assert_always(false, "Unknown id in indigo_state::device_timer");
	}
}

void indigo_state::set_timer_int_clear(uint32_t data)
{
	if (BIT(data, 0))
	{
		LOGMASKED(LOG_PIT | LOG_INT, "Clearing Timer 0 Interrupt: %d\n", state);
		m_maincpu->set_input_line(MIPS3_IRQ2, CLEAR_LINE);
	}
	if (BIT(data, 1))
	{
		LOGMASKED(LOG_PIT | LOG_INT, "Clearing Timer 1 Interrupt: %d\n", state);
		m_maincpu->set_input_line(MIPS3_IRQ3, CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER(indigo_state::timer0_int)
{
	LOGMASKED(LOG_PIT, "Timer0 Interrupt: %d\n", state);
	if (state)
		m_maincpu->set_input_line(MIPS3_IRQ2, ASSERT_LINE);
}

WRITE_LINE_MEMBER(indigo_state::timer1_int)
{
	LOGMASKED(LOG_PIT, "Timer2 Interrupt: %d\n", state);
	if (state)
		m_maincpu->set_input_line(MIPS3_IRQ3, ASSERT_LINE);
}

WRITE_LINE_MEMBER(indigo_state::timer2_int)
{
	LOGMASKED(LOG_PIT, "Timer2 Interrupt (Disabled): %d\n", state);
}

WRITE_LINE_MEMBER(indigo_state::duart0_int_w) { duart_int_w(0, state); }
WRITE_LINE_MEMBER(indigo_state::duart1_int_w) { duart_int_w(1, state); }
WRITE_LINE_MEMBER(indigo_state::duart2_int_w) { duart_int_w(2, state); }

void indigo_state::duart_int_w(int channel, int state)
{
	m_duart_int_status &= ~(1 << channel);
	m_duart_int_status |= state << channel;

	if (m_duart_int_status)
	{
		LOGMASKED(LOG_PIT, "Raising DUART Interrupt: %02x\n", m_duart_int_status);
		raise_local0_irq(LOCAL0_DUART);
	}
	else
	{
		LOGMASKED(LOG_PIT, "Lowering DUART Interrupt\n");
		lower_local0_irq(LOCAL0_DUART);
	}
}

void indigo_state::raise_local0_irq(uint8_t source_mask)
{
	m_hpc.m_local_int0_status |= source_mask;
	LOGMASKED(LOG_INT, "%s IRQ0: %02x & %08x\n", (m_hpc.m_local_int0_status & m_hpc.m_local_int0_mask) ? "Asserting" : "Clearing",
		m_hpc.m_local_int0_status, m_hpc.m_local_int0_mask);
	m_maincpu->set_input_line(MIPS3_IRQ0, (m_hpc.m_local_int0_status & m_hpc.m_local_int0_mask) ? ASSERT_LINE : CLEAR_LINE);
}

void indigo_state::lower_local0_irq(uint8_t source_mask)
{
	m_hpc.m_local_int0_status &= ~source_mask;
	LOGMASKED(LOG_INT, "%s IRQ0: %02x & %08x\n", (m_hpc.m_local_int0_status & m_hpc.m_local_int0_mask) ? "Asserting" : "Clearing",
		m_hpc.m_local_int0_status, m_hpc.m_local_int0_mask);
	m_maincpu->set_input_line(MIPS3_IRQ0, (m_hpc.m_local_int0_status & m_hpc.m_local_int0_mask) ? ASSERT_LINE : CLEAR_LINE);
}

void indigo_state::raise_local1_irq(uint8_t source_mask)
{
	m_hpc.m_local_int1_status |= source_mask;
	LOGMASKED(LOG_INT, "%s IRQ1: %02x & %08x\n", (m_hpc.m_local_int1_status & m_hpc.m_local_int1_mask) ? "Asserting" : "Clearing",
		m_hpc.m_local_int1_status, m_hpc.m_local_int1_mask);
	m_maincpu->set_input_line(MIPS3_IRQ1, (m_hpc.m_local_int1_status & m_hpc.m_local_int1_mask) ? ASSERT_LINE : CLEAR_LINE);
}

void indigo_state::lower_local1_irq(uint8_t source_mask)
{
	m_hpc.m_local_int1_status &= ~source_mask;
	LOGMASKED(LOG_INT, "%s IRQ1: %02x & %08x\n", (m_hpc.m_local_int1_status & m_hpc.m_local_int1_mask) ? "Asserting" : "Clearing",
		m_hpc.m_local_int1_status, m_hpc.m_local_int1_mask);
	m_maincpu->set_input_line(MIPS3_IRQ1, (m_hpc.m_local_int1_status & m_hpc.m_local_int1_mask) ? ASSERT_LINE : CLEAR_LINE);
}

READ32_MEMBER(indigo_state::int_r)
{
	LOGMASKED(LOG_INT, "%s: INT Read: %08x & %08x\n", machine().describe_context(), 0x1fbd9000 + offset*4, mem_mask);
	return 0;
}

WRITE32_MEMBER(indigo_state::int_w)
{
	LOGMASKED(LOG_INT, "%s: INT Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbd9000 + offset*4, data, mem_mask);
}

void indigo_state::indigo_timer_rtc()
{
	m_rtc.m_ram[RTC_HUNDREDTH]++;

	if ((m_rtc.m_ram[RTC_HUNDREDTH] & 0x0f) == 0x0a)
	{
		m_rtc.m_ram[RTC_HUNDREDTH] -= 0x0a;
		m_rtc.m_ram[RTC_HUNDREDTH] += 0x10;
		if ((m_rtc.m_ram[RTC_HUNDREDTH] & 0xa0) == 0xa0)
		{
			m_rtc.m_ram[RTC_HUNDREDTH] = 0;
			m_rtc.m_ram[RTC_SECOND]++;

			if ((m_rtc.m_ram[RTC_SECOND] & 0x0f) == 0x0a)
			{
				m_rtc.m_ram[RTC_SECOND] -= 0x0a;
				m_rtc.m_ram[RTC_SECOND] += 0x10;
				if (m_rtc.m_ram[RTC_SECOND] == 0x60)
				{
					m_rtc.m_ram[RTC_SECOND] = 0;
					m_rtc.m_ram[RTC_MINUTE]++;

					if ((m_rtc.m_ram[RTC_MINUTE] & 0x0f) == 0x0a)
					{
						m_rtc.m_ram[RTC_MINUTE] -= 0x0a;
						m_rtc.m_ram[RTC_MINUTE] += 0x10;
						if (m_rtc.m_ram[RTC_MINUTE] == 0x60)
						{
							m_rtc.m_ram[RTC_MINUTE] = 0;
							m_rtc.m_ram[RTC_HOUR]++;

							if ((m_rtc.m_ram[RTC_HOUR] & 0x0f) == 0x0a)
							{
								m_rtc.m_ram[RTC_HOUR] -= 0x0a;
								m_rtc.m_ram[RTC_HOUR] += 0x10;
								if (m_rtc.m_ram[RTC_HOUR] == 0x24)
								{
									m_rtc.m_ram[RTC_HOUR] = 0;
									m_rtc.m_ram[RTC_DAY]++;
								}
							}
						}
					}
				}
			}
		}
	}
}

void indigo_state::indigo_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("share1");
	map(0x08000000, 0x08ffffff).ram().share("share2");
	map(0x09000000, 0x097fffff).ram().share("share3");
	map(0x0a000000, 0x0a7fffff).ram().share("share4");
	map(0x0c000000, 0x0c7fffff).ram().share("share5");
	map(0x10000000, 0x107fffff).ram().share("share6");
	map(0x18000000, 0x187fffff).ram().share("share7");
	map(0x1fb80000, 0x1fb8ffff).rw(FUNC(indigo_state::hpc_r), FUNC(indigo_state::hpc_w));
	map(0x1fbd9000, 0x1fbd903f).rw(FUNC(indigo_state::int_r), FUNC(indigo_state::int_w));
}

void indigo3k_state::mem_map(address_map &map)
{
	indigo_map(map);
	map(0x1fc00000, 0x1fc3ffff).rom().share("share2").region("user1", 0);
}

void indigo4k_state::mem_map(address_map &map)
{
	indigo_map(map);
	map(0x1fa00000, 0x1fa1ffff).rw("sgi_mc", FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fc00000, 0x1fc7ffff).rom().share("share8").region("user1", 0);
}

static INPUT_PORTS_START( indigo )
	PORT_START("unused")
	PORT_BIT ( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void indigo_state::cdrom_config(device_t *device)
{
	cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	cdda->add_route(ALL_OUTPUTS, ":mono", 1.0);
}

void indigo_state::indigo_base(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SCC85230(config, m_scc[0], SCC_PCLK);
	m_scc[0]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[0]->out_int_callback().set(FUNC(indigo_state::duart0_int_w));

	SCC85230(config, m_scc[1], SCC_PCLK);
	m_scc[1]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[1]->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtra_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsa_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtrb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_int_callback().set(FUNC(indigo_state::duart1_int_w));

	SCC85230(config, m_scc[2], SCC_PCLK);
	m_scc[2]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[2]->out_int_callback().set(FUNC(indigo_state::duart2_int_w));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.cts_handler().set(m_scc[1], FUNC(scc85230_device::ctsa_w));
	rs232a.dcd_handler().set(m_scc[1], FUNC(scc85230_device::dcda_w));
	rs232a.rxd_handler().set(m_scc[1], FUNC(scc85230_device::rxa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.cts_handler().set(m_scc[1], FUNC(scc85230_device::ctsb_w));
	rs232b.dcd_handler().set(m_scc[1], FUNC(scc85230_device::dcdb_w));
	rs232b.rxd_handler().set(m_scc[1], FUNC(scc85230_device::rxb_w));

	scsi_port_device &scsi(SCSI_PORT(config, "scsi"));
	scsi.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_1));
	scsi.set_slot_device(2, "cdrom", SCSICD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));
	scsi.slot(2).set_option_machine_config("cdrom", cdrom_config);

	WD33C93(config, m_wd33c93);
	m_wd33c93->set_scsi_port("scsi");
	m_wd33c93->irq_cb().set(FUNC(indigo_state::scsi_irq));

	EEPROM_93C56_16BIT(config, m_eeprom);

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(1000000);
	m_pit->set_clk<1>(1000000);
	m_pit->set_clk<2>(1000000);
	m_pit->out_handler<0>().set(FUNC(indigo_state::timer0_int));
	m_pit->out_handler<1>().set(FUNC(indigo_state::timer1_int));
	m_pit->out_handler<2>().set(FUNC(indigo_state::timer2_int));
}

void indigo3k_state::indigo3k(machine_config &config)
{
	indigo_base(config);

	R3000A(config, m_maincpu, 33.333_MHz_XTAL, 32768, 32768);
	downcast<r3000a_device &>(*m_maincpu).set_endianness(ENDIANNESS_BIG);
	m_maincpu->set_addrmap(AS_PROGRAM, &indigo3k_state::mem_map);
}

void indigo4k_state::indigo4k(machine_config &config)
{
	indigo_base(config);
	mips3_device &cpu(R4400BE(config, m_maincpu, 50000000*3));
	cpu.set_icache_size(32768);
	cpu.set_dcache_size(32768);
	cpu.set_addrmap(AS_PROGRAM, &indigo4k_state::mem_map);

	SGI_MC(config, "sgi_mc");
}

ROM_START( indigo3k )
	ROM_REGION32_BE( 0x40000, "user1", 0 )
	ROM_SYSTEM_BIOS( 0, "401-rev-c", "SGI Version 4.0.1 Rev C LG1/GR2, Jul 9, 1992" ) // dumped over serial connection from boot monitor and swapped
	ROMX_LOAD( "ip12prom.070-8088-xxx.u56", 0x000000, 0x040000, CRC(25ca912f) SHA1(94b3753d659bfe50b914445cef41290122f43880), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "401-rev-d", "SGI Version 4.0.1 Rev D LG1/GR2, Mar 24, 1992" ) // dumped with EPROM programmer
	ROMX_LOAD( "ip12prom.070-8088-002.u56", 0x000000, 0x040000, CRC(ea4329ef) SHA1(b7d67d0e30ae8836892f7170dd4757732a0a3fd6), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1) )
ROM_END

ROM_START( indigo4k )
	ROM_REGION32_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip20prom.070-8116-004.bin", 0x000000, 0x080000, CRC(940d960e) SHA1(596aba530b53a147985ff3f6f853471ce48c866c), ROM_GROUPDWORD | ROM_REVERSE )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS           INIT        COMPANY                 FULLNAME                                          FLAGS
COMP( 1991, indigo3k, 0,      0,      indigo3k, indigo, indigo3k_state, empty_init, "Silicon Graphics Inc", "IRIS Indigo (R3000, 33MHz)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1993, indigo4k, 0,      0,      indigo4k, indigo, indigo4k_state, empty_init, "Silicon Graphics Inc", "IRIS Indigo (R4400, 150MHz, Ver. 4.0.5D Rev A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
