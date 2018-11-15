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
#include "bus/rs232/hlemouse.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd512.h"
#include "bus/scsi/scsihd.h"
#include "bus/sgikbd/sgikbd.h"
//#include "cpu/dsp56k/dsp56k.h"
#include "cpu/mips/mips1.h"
#include "cpu/mips/mips3.h"
#include "machine/dp8573.h"
#include "machine/eepromser.h"
#include "machine/pit8253.h"
#include "machine/sgi.h"
#include "machine/wd33c93.h"
#include "machine/z80scc.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define ENABLE_ENTRY_GFX	(1)

#define LOG_UNKNOWN		(1 << 0)
#define LOG_INT			(1 << 1)
#define LOG_HPC			(1 << 2)
#define LOG_EEPROM		(1 << 4)
#define LOG_DMA			(1 << 5)
#define LOG_SCSI		(1 << 6)
#define LOG_SCSI_DMA	(1 << 7)
#define LOG_DUART0		(1 << 8)
#define LOG_DUART1		(1 << 9)
#define LOG_DUART2		(1 << 10)
#define LOG_PIT			(1 << 11)
#define LOG_DSP			(1 << 12)
#define LOG_GFX			(1 << 13)
#define LOG_GFX_CMD		(1 << 14)
#define LOG_DUART		(LOG_DUART0 | LOG_DUART1 | LOG_DUART2)
#define LOG_ALL			(LOG_UNKNOWN | LOG_INT | LOG_HPC | LOG_EEPROM | LOG_DMA | LOG_SCSI | LOG_SCSI_DMA | LOG_DUART | LOG_PIT | LOG_DSP | LOG_GFX | LOG_GFX_CMD)

#define VERBOSE			(LOG_ALL & ~(LOG_DUART0 | LOG_DUART1 | LOG_SCSI | LOG_SCSI_DMA | LOG_EEPROM | LOG_PIT | LOG_DSP))
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
		, m_rtc(*this, "rtc")
		, m_dsp_ram(*this, "dspram")
		, m_palette(*this, "palette")
	{
	}

	void indigo_base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ32_MEMBER(hpc_r);
	DECLARE_WRITE32_MEMBER(hpc_w);
	DECLARE_READ32_MEMBER(int_r);
	DECLARE_WRITE32_MEMBER(int_w);
	DECLARE_READ32_MEMBER(dsp_ram_r);
	DECLARE_WRITE32_MEMBER(dsp_ram_w);
	DECLARE_READ32_MEMBER(entry_r);
	DECLARE_WRITE32_MEMBER(entry_w);
	DECLARE_WRITE_LINE_MEMBER(scsi_irq);

	void set_timer_int_clear(uint32_t data);
	DECLARE_WRITE_LINE_MEMBER(timer0_int);
	DECLARE_WRITE_LINE_MEMBER(timer1_int);
	DECLARE_WRITE_LINE_MEMBER(timer2_int);
	DECLARE_WRITE_LINE_MEMBER(duart0_int_w);
	DECLARE_WRITE_LINE_MEMBER(duart1_int_w);
	DECLARE_WRITE_LINE_MEMBER(duart2_int_w);

	void duart_int_w(int channel, int status);
	void raise_local_irq(int channel, uint8_t source_mask);
	void lower_local_irq(int channel, uint8_t source_mask);
	void update_irq(int channel);

	void fetch_chain(address_space &space);
	void advance_chain(address_space &space);
	void scsi_dma();

	void do_rex_command();

	static void cdrom_config(device_t *device);
	void indigo_map(address_map &map);

	uint32_t screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

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

	enum
	{
		REX15_PAGE0_SET				= 0x00000000,
		REX15_PAGE0_GO				= 0x00000800,
		REX15_PAGE1_SET				= 0x00004790,
		REX15_PAGE1_GO				= 0x00004f90,

		REX15_P0REG_COMMAND			= 0x00000000,
		REX15_P0REG_XSTARTI			= 0x0000000c,
		REX15_P0REG_YSTARTI			= 0x0000001c,
		REX15_P0REG_XYMOVE			= 0x00000034,
		REX15_P0REG_COLORREDI		= 0x00000038,
		REX15_P0REG_COLORGREENI		= 0x00000040,
		REX15_P0REG_COLORBLUEI		= 0x00000048,
		REX15_P0REG_COLORBACK		= 0x0000005c,
		REX15_P0REG_ZPATTERN		= 0x00000060,
		REX15_P0REG_XENDI			= 0x00000084,
		REX15_P0REG_YENDI			= 0x00000088,

		REX15_P1REG_WCLOCKREV		= 0x00000054,
		REX15_P1REG_CFGDATA			= 0x00000058,
		REX15_P1REG_CFGSEL			= 0x0000005c,
		REX15_P1REG_VC1_ADDRDATA	= 0x00000060,
		REX15_P1REG_CFGMODE			= 0x00000068,
		REX15_P1REG_XYOFFSET		= 0x0000006c,

		REX15_OP_NOP				= 0x00000000,
		REX15_OP_DRAW				= 0x00000001,

		REX15_OP_FLAG_BLOCK			= 0x00000008,
		REX15_OP_FLAG_LENGTH32		= 0x00000010,
		REX15_OP_FLAG_QUADMODE		= 0x00000020,
		REX15_OP_FLAG_XYCONTINUE	= 0x00000080,
		REX15_OP_FLAG_STOPONX		= 0x00000100,
		REX15_OP_FLAG_STOPONY		= 0x00000200,
		REX15_OP_FLAG_ENZPATTERN	= 0x00000400,
		REX15_OP_FLAG_LOGICSRC		= 0x00080000,
		REX15_OP_FLAG_ZOPAQUE		= 0x00800000,
		REX15_OP_FLAG_ZCONTINUE		= 0x01000000,

		REX15_WRITE_ADDR			= 0x00,
		REX15_PALETTE_RAM			= 0x01,
		REX15_PIXEL_READ_MASK		= 0x02,
		REX15_CONTROL				= 0x06
	};

	struct hpc_t
	{
		uint8_t m_misc_status;
		uint32_t m_cpu_aux_ctrl;
		uint32_t m_parbuf_ptr;
		uint32_t m_local_int_status[2];
		uint32_t m_local_int_mask[2];
		bool m_int_status[2];
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
		bool m_scsi0_dma_end;
	};

	struct lg1_t
	{
		uint32_t m_config_sel;
		uint32_t m_write_addr;
		uint32_t m_control;

		uint32_t m_command;
		uint32_t m_x_start_i;
		uint32_t m_y_start_i;
		uint32_t m_xy_move;
		uint32_t m_color_red_i;
		uint32_t m_color_green_i;
		uint32_t m_color_blue_i;
		uint32_t m_color_back;
		uint32_t m_z_pattern;
		uint32_t m_x_end_i;
		uint32_t m_y_end_i;
		uint32_t m_x_curr_i;
		uint32_t m_y_curr_i;

		uint8_t m_palette_idx;
		uint8_t m_palette_channel;
		uint8_t m_palette_entry[3];
		bool m_waiting_for_palette_addr;
		uint8_t m_pix_read_mask[256];
	};

	enum
	{
		HPC_DMACTRL_RESET	= 0x01,
		HPC_DMACTRL_FLUSH	= 0x02,
		HPC_DMACTRL_TO_MEM	= 0x10,
		HPC_DMACTRL_ENABLE	= 0x80
	};

	required_device<cpu_device> m_maincpu;
	required_device<wd33c93_device> m_wd33c93;
	required_device_array<scc8530_device, 3> m_scc;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<pit8254_device> m_pit;
	required_device<dp8573_device> m_rtc;
	required_shared_ptr<uint32_t> m_dsp_ram;
	required_device<palette_device> m_palette;

	hpc_t m_hpc;
	lg1_t m_lg1;
	std::unique_ptr<uint8_t[]> m_framebuffer;
	uint8_t m_duart_int_status;

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
	m_framebuffer = std::make_unique<uint8_t[]>(1024*768);

	save_item(NAME(m_hpc.m_misc_status));
	save_item(NAME(m_hpc.m_cpu_aux_ctrl));
	save_item(NAME(m_hpc.m_parbuf_ptr));
	save_item(NAME(m_hpc.m_local_int_status));
	save_item(NAME(m_hpc.m_local_int_mask));
	save_item(NAME(m_hpc.m_int_status));
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
	save_item(NAME(m_hpc.m_scsi0_dma_end));

	save_item(NAME(m_lg1.m_config_sel));
	save_item(NAME(m_lg1.m_write_addr));
	save_item(NAME(m_lg1.m_control));
	save_item(NAME(m_lg1.m_command));
	save_item(NAME(m_lg1.m_x_start_i));
	save_item(NAME(m_lg1.m_y_start_i));
	save_item(NAME(m_lg1.m_xy_move));
	save_item(NAME(m_lg1.m_color_red_i));
	save_item(NAME(m_lg1.m_color_green_i));
	save_item(NAME(m_lg1.m_color_blue_i));
	save_item(NAME(m_lg1.m_color_back));
	save_item(NAME(m_lg1.m_z_pattern));
	save_item(NAME(m_lg1.m_x_end_i));
	save_item(NAME(m_lg1.m_y_end_i));
	save_item(NAME(m_lg1.m_x_curr_i));
	save_item(NAME(m_lg1.m_y_curr_i));
	save_item(NAME(m_lg1.m_palette_idx));
	save_item(NAME(m_lg1.m_palette_channel));
	save_item(NAME(m_lg1.m_palette_entry));
	save_item(NAME(m_lg1.m_waiting_for_palette_addr));
	save_item(NAME(m_lg1.m_pix_read_mask));

	save_item(NAME(m_duart_int_status));

	save_pointer(NAME(&m_framebuffer[0]), 1024*768);
}

void indigo_state::machine_reset()
{
	memset(&m_hpc, 0, sizeof(hpc_t));
	memset(&m_lg1, 0, sizeof(lg1_t));
	memset(&m_framebuffer[0], 0, 1024*768);
	m_duart_int_status = 0;
}

READ32_MEMBER(indigo_state::hpc_r)
{
	if (offset >= 0x0e00/4 && offset <= 0x0e7c/4)
		return m_rtc->read(space, offset - 0xe00/4);

	switch (offset)
	{
	case 0x005c/4:
		LOGMASKED(LOG_HPC | LOG_UNKNOWN, "%s: HPC Unknown Read: %08x & %08x\n",
			machine().describe_context(), 0x1fb80000 + offset*4, mem_mask);
		return 0;
	case 0x0094/4:
		LOGMASKED(LOG_SCSI_DMA, "%s: HPC SCSI DMA Control Register Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_scsi0_dma_ctrl, mem_mask);
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
		uint32_t ret = (m_hpc.m_cpu_aux_ctrl & ~0x10) | m_eeprom->do_read() << 4;
		LOGMASKED(LOG_EEPROM, "%s: HPC Serial EEPROM Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		return ret;
	}
	case 0x01c0/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 0 Status Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_local_int_status[0], mem_mask);
		return m_hpc.m_local_int_status[0];
	case 0x01c4/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 0 Mask Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_local_int_mask[0], mem_mask);
		return m_hpc.m_local_int_mask[0];
	case 0x01c8/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 1 Status Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_local_int_status[1], mem_mask);
		return m_hpc.m_local_int_status[1];
	case 0x01cc/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 1 Mask Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_local_int_mask[1], mem_mask);
		return m_hpc.m_local_int_mask[1];
	case 0x01d4/4:
		LOGMASKED(LOG_HPC, "%s: HPC VME Interrupt Mask 0 Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_vme_intmask0, mem_mask);
		return m_hpc.m_vme_intmask0;
	case 0x01d8/4:
		LOGMASKED(LOG_HPC, "%s: HPC VME Interrupt Mask 1 Read: %08x & %08x\n", machine().describe_context(), m_hpc.m_vme_intmask1, mem_mask);
		return m_hpc.m_vme_intmask1;
	case 0x01f0/4:
	{
		const uint8_t data = m_pit->read(0);
		LOGMASKED(LOG_PIT, "%s: Read Timer Count0 Register: %02x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case 0x01f4/4:
	{
		const uint8_t data = m_pit->read(1);
		LOGMASKED(LOG_PIT, "%s: Read Timer Count1 Register: %02x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case 0x01f8/4:
	{
		const uint8_t data = m_pit->read(2);
		LOGMASKED(LOG_PIT, "%s: Read Timer Count2 Register: %02x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case 0x01fc/4:
	{
		const uint8_t data = m_pit->read(3);
		LOGMASKED(LOG_PIT, "%s: Read Timer Control Register: %02x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case 0x0d00/4:
	case 0x0d10/4:
	case 0x0d20/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		uint32_t ret = m_scc[index]->ba_cd_r(space, 3);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel B Control Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d04/4:
	case 0x0d14/4:
	case 0x0d24/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ba_cd_r(space, 2);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel B Data Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d08/4:
	case 0x0d18/4:
	case 0x0d28/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ba_cd_r(space, 1);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel A Control Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d0c/4:
	case 0x0d1c/4:
	case 0x0d2c/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ba_cd_r(space, 0);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel A Data Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
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
	m_hpc.m_scsi0_dma_addr = space.read_dword(m_hpc.m_scsi0_dma_desc+4);
	m_hpc.m_scsi0_dma_next = space.read_dword(m_hpc.m_scsi0_dma_desc+8);
	m_hpc.m_scsi0_dma_length = m_hpc.m_scsi0_dma_flag & 0x1fff;
	LOGMASKED(LOG_SCSI_DMA, "Fetched SCSI DMA Descriptor block:\n");
	LOGMASKED(LOG_SCSI_DMA, "    Ctrl: %08x\n", m_hpc.m_scsi0_dma_flag);
	LOGMASKED(LOG_SCSI_DMA, "    Addr: %08x\n", m_hpc.m_scsi0_dma_addr);
	LOGMASKED(LOG_SCSI_DMA, "    Next: %08x\n", m_hpc.m_scsi0_dma_next);
	LOGMASKED(LOG_SCSI_DMA, "    Length: %04x\n", m_hpc.m_scsi0_dma_length);
	m_hpc.m_scsi0_dma_end = BIT(m_hpc.m_scsi0_dma_addr, 31);
	m_hpc.m_scsi0_dma_addr &= 0x0fffffff;
	m_hpc.m_scsi0_dma_next &= 0x0fffffff;
}

void indigo_state::advance_chain(address_space &space)
{
	m_hpc.m_scsi0_dma_addr++;
	m_hpc.m_scsi0_dma_length--;
	if (m_hpc.m_scsi0_dma_length == 0)
	{
		if (m_hpc.m_scsi0_dma_end)
		{
			LOGMASKED(LOG_SCSI_DMA, "HPC: Disabling SCSI DMA due to end of chain\n");
			m_hpc.m_scsi0_dma_active = false;
			m_hpc.m_scsi0_dma_ctrl &= ~HPC_DMACTRL_ENABLE;
		}
		else
		{
			m_hpc.m_scsi0_dma_desc = m_hpc.m_scsi0_dma_next;
			fetch_chain(space);
		}
	}
}

WRITE32_MEMBER(indigo_state::hpc_w)
{
	if (offset >= 0x0e00/4 && offset <= 0x0e7c/4)
	{
		m_rtc->write(space, offset - 0xe00/4, (uint8_t)data);
		return;
	}

	switch (offset)
	{
	case 0x0090/4:
		LOGMASKED(LOG_SCSI_DMA, "%s: HPC SCSI DMA Descriptor Pointer Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_scsi0_dma_desc = data;
		fetch_chain(space);
		break;

	case 0x0094/4:
		LOGMASKED(LOG_SCSI_DMA, "%s: HPC SCSI DMA Control Register Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
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
		m_hpc.m_cpu_aux_ctrl = data;
		LOGMASKED(LOG_EEPROM, "%s: HPC Serial EEPROM Write: %08x & %08x\n", machine().describe_context(), data, mem_mask );
		if (BIT(data, 0))
		{
			LOGMASKED(LOG_EEPROM, "    CPU board LED on\n");
		}
		m_eeprom->di_write(BIT(data, 3));
		m_eeprom->cs_write(BIT(data, 1));
		m_eeprom->clk_write(BIT(data, 2));
		break;
	case 0x01c0/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 0 Status Write (Ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x01c4/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 0 Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_local_int_mask[0] = data;
		break;
	case 0x01c8/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 1 Status Write (Ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x01cc/4:
		LOGMASKED(LOG_HPC, "%s: HPC Local Interrupt 1 Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_hpc.m_local_int_mask[1] = data;
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
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Count0 Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (ACCESSING_BITS_24_31)
			m_pit->write(0, (uint8_t)(data >> 24));
		else if (ACCESSING_BITS_0_7)
			m_pit->write(0, (uint8_t)data);
		return;
	case 0x01f4/4:
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Count1 Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pit->write(1, (uint8_t)data);
		return;
	case 0x01f8/4:
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Count2 Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pit->write(2, (uint8_t)data);
		return;
	case 0x01fc/4:
		LOGMASKED(LOG_PIT, "%s: HPC Write Timer Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pit->write(3, (uint8_t)data);
		return;
	case 0x0d00/4:
	case 0x0d10/4:
	case 0x0d20/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ba_cd_w(space, 3, (uint8_t)data);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel B Control Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d04/4:
	case 0x0d14/4:
	case 0x0d24/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ba_cd_w(space, 2, (uint8_t)data);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel B Data Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d08/4:
	case 0x0d18/4:
	case 0x0d28/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ba_cd_w(space, 1, (uint8_t)data);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel A Control Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d0c/4:
	case 0x0d1c/4:
	case 0x0d2c/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ba_cd_w(space, 0, (uint8_t)data);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel A Data Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
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
		LOGMASKED(LOG_SCSI, "SCSI: Set IRQ\n");
		int count = m_wd33c93->get_dma_count();
		LOGMASKED(LOG_SCSI_DMA, "SCSI: count %d, active %d\n", count, m_hpc.m_scsi0_dma_active);
		if (count && m_hpc.m_scsi0_dma_active)
			scsi_dma();

		raise_local_irq(0, LOCAL0_SCSI);
	}
	else
	{
		LOGMASKED(LOG_SCSI, "SCSI: Clear IRQ\n");
		lower_local_irq(0, LOCAL0_SCSI);
	}
}

void indigo_state::set_timer_int_clear(uint32_t data)
{
	if (BIT(data, 0))
	{
		LOGMASKED(LOG_PIT | LOG_INT, "Clearing Timer 0 Interrupt: %d\n", data);
		m_maincpu->set_input_line(MIPS3_IRQ2, CLEAR_LINE);
	}
	if (BIT(data, 1))
	{
		LOGMASKED(LOG_PIT | LOG_INT, "Clearing Timer 1 Interrupt: %d\n", data);
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
		LOGMASKED(LOG_DUART0 << channel, "Raising DUART Interrupt: %02x\n", m_duart_int_status);
		raise_local_irq(0, LOCAL0_DUART);
	}
	else
	{
		LOGMASKED(LOG_DUART0 << channel, "Lowering DUART Interrupt\n");
		lower_local_irq(0, LOCAL0_DUART);
	}
}

void indigo_state::raise_local_irq(int channel, uint8_t source_mask)
{
	m_hpc.m_local_int_status[channel] |= source_mask;

	bool old_status = m_hpc.m_int_status[channel];
	m_hpc.m_int_status[channel] = (m_hpc.m_local_int_status[channel] & m_hpc.m_local_int_mask[channel]);

	if (old_status != m_hpc.m_int_status[channel])
		update_irq(channel);
}

void indigo_state::lower_local_irq(int channel, uint8_t source_mask)
{
	m_hpc.m_local_int_status[channel] &= ~source_mask;

	bool old_status = m_hpc.m_int_status[channel];
	m_hpc.m_int_status[channel] = (m_hpc.m_local_int_status[channel] & m_hpc.m_local_int_mask[channel]);

	if (old_status != m_hpc.m_int_status[channel])
		update_irq(channel);
}

void indigo_state::update_irq(int channel)
{
	LOGMASKED(LOG_INT, "%s IRQ%d: %02x & %02x\n", channel, m_hpc.m_int_status[channel] ? "Asserting" : "Clearing",
		m_hpc.m_local_int_status[channel], m_hpc.m_local_int_mask[channel]);
	m_maincpu->set_input_line(MIPS3_IRQ0, m_hpc.m_int_status[channel] ? ASSERT_LINE : CLEAR_LINE);
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

READ32_MEMBER(indigo_state::dsp_ram_r)
{
	LOGMASKED(LOG_DSP, "%s: DSP RAM Read: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbe0000 + offset*4, m_dsp_ram[offset], mem_mask);
	return m_dsp_ram[offset];
}

WRITE32_MEMBER(indigo_state::dsp_ram_w)
{
	LOGMASKED(LOG_DSP, "%s: DSP RAM Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbe0000 + offset*4, data, mem_mask);
	COMBINE_DATA(&m_dsp_ram[offset]);
}

READ32_MEMBER(indigo_state::entry_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
	case REX15_PAGE0_GO/4:
		LOGMASKED(LOG_GFX, "%s: LG1 Read: Status(?) (Go) %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		do_rex_command();
		break;
	case 0x0014/4:
		ret = 0x033c0000;
		LOGMASKED(LOG_GFX, "%s: LG1 Read: Presence Detect(?) %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		break;
	default:
		LOGMASKED(LOG_GFX, "%s: Unknown LG1 Read: %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		break;
	}
	return ret;
}

void indigo_state::do_rex_command()
{
	if (m_lg1.m_command == 0)
	{
		return;
	}
	if (m_lg1.m_command == 0x30080329)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		bool copy = (m_lg1.m_command & REX15_OP_FLAG_LOGICSRC);
		const uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		const uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		const uint32_t end_x = m_lg1.m_x_end_i;
		const uint32_t end_y = m_lg1.m_y_end_i;
		const uint32_t src_start_x = start_x + (m_lg1.m_xy_move >> 16);
		const uint32_t src_start_y = start_y + (uint16_t)m_lg1.m_xy_move;;

		LOGMASKED(LOG_GFX, "LG1: Command %08x: Block copy from %d,%d-%d,%d inclusive.\n", m_lg1.m_command, start_x, start_y, end_x, end_y);
		if (copy)
		{
			for (uint32_t y = start_y, src_y = src_start_y; y <= end_y; y++, src_y++)
				for (uint32_t x = start_x, src_x = src_start_x; x <= end_x; x++, src_x++)
					m_framebuffer[y*1024 + x] = m_framebuffer[src_y*1024 + src_x];
		}
		else
		{
			for (uint32_t y = start_y; y <= end_y; y++)
				for (uint32_t x = start_x; x <= end_x; x++)
					m_framebuffer[y*1024 + x] = m_lg1.m_color_red_i;
		}
	}
	else if (m_lg1.m_command == 0x30000329)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		uint32_t end_x = m_lg1.m_x_end_i;
		uint32_t end_y = m_lg1.m_y_end_i;

		LOGMASKED(LOG_GFX, "LG1: Command %08x: Block draw from %d,%d-%d,%d inclusive.\n", m_lg1.m_command, start_x, start_y, end_x, end_y);
		for (uint32_t y = start_y; y <= end_y; y++)
		{
			for (uint32_t x = start_x; x <= end_x; x++)
			{
				m_framebuffer[y*1024 + x] = m_lg1.m_color_red_i;
			}
		}
	}
	else if (m_lg1.m_command == 0x300005a1 ||
             m_lg1.m_command == 0x300005a9 ||
             m_lg1.m_command == 0x300005b9)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		uint32_t end_x = m_lg1.m_x_end_i;
		LOGMASKED(LOG_GFX, "LG1: Command %08x: Pattern draw from %d-%d at %d\n", m_lg1.m_command, start_x, end_x, start_y);
		for (uint32_t x = start_x; x <= end_x && x < (start_x + 32); x++)
		{
			if (BIT(m_lg1.m_z_pattern, 31 - (x - start_x)))
			{
				m_framebuffer[start_y*1024 + x] = m_lg1.m_color_red_i;
			}
			m_lg1.m_x_curr_i++;
		}

		if (m_lg1.m_command & REX15_OP_FLAG_BLOCK)
		{
			if (m_lg1.m_x_curr_i > m_lg1.m_x_end_i)
			{
				m_lg1.m_y_curr_i--;
				m_lg1.m_x_curr_i = m_lg1.m_x_start_i;
			}
		}
	}
	else
	{
		LOGMASKED(LOG_GFX_CMD | LOG_UNKNOWN, "%s: Unknown LG1 command: %08x\n", machine().describe_context(), m_lg1.m_command);
	}
}

WRITE32_MEMBER(indigo_state::entry_w)
{
	bool go = (offset >= REX15_PAGE1_GO/4) || (offset >= REX15_PAGE0_GO/4 && offset < REX15_PAGE1_SET/4);

	switch (offset)
	{
		case (REX15_PAGE0_SET+REX15_P0REG_COMMAND)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COMMAND)/4:
			m_lg1.m_command = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 Command Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			if (go)
				do_rex_command();
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_XSTARTI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_XSTARTI)/4:
			m_lg1.m_x_start_i = data;
			m_lg1.m_x_curr_i = m_lg1.m_x_start_i;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 XStartI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_YSTARTI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_YSTARTI)/4:
			m_lg1.m_y_start_i = data;
			m_lg1.m_y_curr_i = m_lg1.m_y_start_i;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 YStartI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_XYMOVE)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_XYMOVE)/4:
			m_lg1.m_xy_move = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 XYMove Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			if (go)
				do_rex_command();
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_COLORREDI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COLORREDI)/4:
			m_lg1.m_color_red_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ColorRedI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_COLORGREENI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COLORGREENI)/4:
			m_lg1.m_color_green_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ColorGreenI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_COLORBLUEI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COLORBLUEI)/4:
			m_lg1.m_color_blue_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ColorBlueI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_COLORBACK)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COLORBACK)/4:
			m_lg1.m_color_back = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ColorBlueI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_ZPATTERN)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_ZPATTERN)/4:
			m_lg1.m_z_pattern = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ZPattern Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			if (go)
				do_rex_command();
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_XENDI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_XENDI)/4:
			m_lg1.m_x_end_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 XEndI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_YENDI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_YENDI)/4:
			m_lg1.m_y_end_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 YEndI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;

		case (REX15_PAGE1_SET+REX15_P1REG_CFGSEL)/4:
		case (REX15_PAGE1_GO+REX15_P1REG_CFGSEL)/4:
			m_lg1.m_config_sel = data;
			switch (data)
			{
				case REX15_WRITE_ADDR:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Write Addr)\n", machine().describe_context(), data);
					break;
				case REX15_PALETTE_RAM:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Palette RAM)\n", machine().describe_context(), data);
					break;
				case REX15_CONTROL:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Control)\n", machine().describe_context(), data);
					break;
				case REX15_PIXEL_READ_MASK:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Pixel Read Mask)\n", machine().describe_context(), data);
					break;
				default:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Unknown)\n", machine().describe_context(), data);
					break;
			}
			break;
		case (REX15_PAGE1_SET+REX15_P1REG_CFGDATA)/4:
		case (REX15_PAGE1_GO+REX15_P1REG_CFGDATA)/4:
			if (go) // Ignore 'Go' writes for now, unsure what they do
				break;
			switch (m_lg1.m_config_sel)
			{
				case REX15_WRITE_ADDR:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write: Setting WriteAddr to %08x\n", machine().describe_context(), data);
					m_lg1.m_write_addr = data;
					if (m_lg1.m_control == 0xf)
					{
						LOGMASKED(LOG_GFX, "%s: LG1 about to set palette entry %02x\n", machine().describe_context(), (uint8_t)data);
						m_lg1.m_palette_idx = (uint8_t)data;
						m_lg1.m_palette_channel = 0;
					}
					break;
				case REX15_PALETTE_RAM:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write: Setting Palette RAM %02x entry %d to %02x\n", machine().describe_context(),
						m_lg1.m_palette_idx, m_lg1.m_palette_channel, (uint8_t)data);
					m_lg1.m_palette_entry[m_lg1.m_palette_channel++] = (uint8_t)data;
					if (m_lg1.m_palette_channel == 3)
					{
						m_palette->set_pen_color(m_lg1.m_palette_idx, m_lg1.m_palette_entry[0], m_lg1.m_palette_entry[1], m_lg1.m_palette_entry[2]);
					}
					break;
				case REX15_CONTROL:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write: Setting Control to %08x\n", machine().describe_context(), data);
					m_lg1.m_control = data;
					break;
				case REX15_PIXEL_READ_MASK:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Pixel Read Mask, entry %02x)\n", machine().describe_context(), data, m_lg1.m_palette_idx);
					m_lg1.m_pix_read_mask[m_lg1.m_palette_idx] = (uint8_t)data;
					break;
				default:
					LOGMASKED(LOG_GFX, "%s: LG1 Unknown ConfigData Write = %08x\n", machine().describe_context(), data);
					break;
			}
			break;
		default:
			LOGMASKED(LOG_GFX, "%s: Unknown LG1 Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, data, mem_mask);
			break;
	}
}

uint32_t indigo_state::screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *pens = m_palette->palette()->entry_list_raw();

	for (int y = 0; y < 768; y++)
	{
		uint32_t *dst = &bitmap.pix32(y);
		uint8_t *src = &m_framebuffer[y*1024];
		for (int x = 0; x < 1024; x++)
		{
			*dst++ = pens[*src++];
		}
	}
	return 0;
}

void indigo_state::indigo_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().share("share1");
	map(0x08000000, 0x0fffffff).ram().share("share2");
	map(0x10000000, 0x13ffffff).ram().share("share3");
	map(0x18000000, 0x1bffffff).ram().share("share4");
	map(0x1f3f0000, 0x1f3fffff).rw(FUNC(indigo_state::entry_r), FUNC(indigo_state::entry_w));
	map(0x1fb80000, 0x1fb8ffff).rw(FUNC(indigo_state::hpc_r), FUNC(indigo_state::hpc_w));
	map(0x1fbd9000, 0x1fbd903f).rw(FUNC(indigo_state::int_r), FUNC(indigo_state::int_w));
	map(0x1fbe0000, 0x1fbfffff).rw(FUNC(indigo_state::dsp_ram_r), FUNC(indigo_state::dsp_ram_w)).share("dspram");
}

void indigo3k_state::mem_map(address_map &map)
{
	indigo_map(map);
	map(0x1fc00000, 0x1fc3ffff).rom().share("share5").region("user1", 0);
}

void indigo4k_state::mem_map(address_map &map)
{
	indigo_map(map);
	map(0x1fa00000, 0x1fa1ffff).rw("sgi_mc", FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fc00000, 0x1fc7ffff).rom().share("share5").region("user1", 0);
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

static void indigo_mice(device_slot_interface &device)
{
	device.option_add("sgimouse", SGI_HLE_SERIAL_MOUSE);
}

void indigo_state::indigo_base(machine_config &config)
{
	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(1024, 768);
	screen.set_visarea(0, 1024-1, 0, 768-1);
	screen.set_screen_update(FUNC(indigo_state::screen_update));

	PALETTE(config, m_palette, 256);

	SPEAKER(config, "mono").front_center();

	SCC8530N(config, m_scc[0], SCC_PCLK);
	m_scc[0]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[0]->out_int_callback().set(FUNC(indigo_state::duart0_int_w));
	m_scc[0]->out_txda_callback().set("keyboard", FUNC(sgi_keyboard_port_device::write_txd));

	SCC8530N(config, m_scc[1], SCC_PCLK);
	m_scc[1]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[1]->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtra_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsa_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtrb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_int_callback().set(FUNC(indigo_state::duart1_int_w));

	SCC8530N(config, m_scc[2], SCC_PCLK);
	m_scc[2]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[2]->out_int_callback().set(FUNC(indigo_state::duart2_int_w));

	SGIKBD_PORT(config, "keyboard", default_sgi_keyboard_devices, "hlekbd").rxd_handler().set(m_scc[0], FUNC(z80scc_device::rxa_w));

	rs232_port_device &mouseport(RS232_PORT(config, "mouseport", indigo_mice, "sgimouse"));
	mouseport.set_fixed(true);
	mouseport.rxd_handler().set(m_scc[0], FUNC(scc8530_device::rxa_w));
	mouseport.cts_handler().set(m_scc[0], FUNC(scc8530_device::ctsa_w));
	mouseport.dcd_handler().set(m_scc[0], FUNC(scc8530_device::dcda_w));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.cts_handler().set(m_scc[1], FUNC(scc8530_device::ctsa_w));
	rs232a.dcd_handler().set(m_scc[1], FUNC(scc8530_device::dcda_w));
	rs232a.rxd_handler().set(m_scc[1], FUNC(scc8530_device::rxa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.cts_handler().set(m_scc[1], FUNC(scc8530_device::ctsb_w));
	rs232b.dcd_handler().set(m_scc[1], FUNC(scc8530_device::dcdb_w));
	rs232b.rxd_handler().set(m_scc[1], FUNC(scc8530_device::rxb_w));

	scsi_port_device &scsi(SCSI_PORT(config, "scsi"));
	scsi.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_1));
	scsi.set_slot_device(2, "cdrom", RRD45, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));
	scsi.slot(2).set_option_machine_config("cdrom", cdrom_config);

	DP8573(config, m_rtc);

	WD33C93(config, m_wd33c93);
	m_wd33c93->set_scsi_port("scsi");
	m_wd33c93->irq_cb().set(FUNC(indigo_state::scsi_irq));

	EEPROM_93C56_16BIT(config, m_eeprom);

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(1000000);
	m_pit->set_clk<1>(1000000);
	m_pit->set_clk<2>(1500000);
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
