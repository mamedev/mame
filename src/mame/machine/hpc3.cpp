// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC3 "High-performance Peripheral Controller" emulation

**********************************************************************/

#include "emu.h"
#include "machine/hpc3.h"

#define LOG_UNKNOWN		(1 << 0)
#define LOG_PBUS_DMA	(1 << 1)
#define LOG_SCSI		(1 << 2)
#define LOG_ETHERNET	(1 << 3)
#define LOG_PBUS4		(1 << 4)
#define LOG_CHAIN		(1 << 5)

#define VERBOSE		(0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_HPC3, hpc3_device, "hpc3", "SGI HPC3")

hpc3_device::hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HPC3, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_wd33c93(*this, finder_base::DUMMY_TAG)
	, m_wd33c93_2(*this, finder_base::DUMMY_TAG)
	, m_ioc2(*this, finder_base::DUMMY_TAG)
	, m_ldac(*this, finder_base::DUMMY_TAG)
	, m_rdac(*this, finder_base::DUMMY_TAG)
	, m_mainram(*this, ":mainram")
{
}

void hpc3_device::device_start()
{
	save_item(NAME(m_enetr_nbdp));
	save_item(NAME(m_enetr_cbp));

	save_item(NAME(m_unk0));
	save_item(NAME(m_unk1));
	save_item(NAME(m_ic_unk0));
	save_item(NAME(m_scsi0_desc));
	save_item(NAME(m_scsi0_addr));
	save_item(NAME(m_scsi0_flags));
	save_item(NAME(m_scsi0_byte_count));
	save_item(NAME(m_scsi0_next_addr));
	save_item(NAME(m_scsi0_dma_ctrl));
	save_item(NAME(m_pio_config));

	for (uint32_t i = 0; i < 8; i++)
	{
		save_item(NAME(m_pbus_dma[i].m_active), i);
		save_item(NAME(m_pbus_dma[i].m_cur_ptr), i);
		save_item(NAME(m_pbus_dma[i].m_desc_ptr), i);
		save_item(NAME(m_pbus_dma[i].m_desc_flags), i);
		save_item(NAME(m_pbus_dma[i].m_next_ptr), i);
		save_item(NAME(m_pbus_dma[i].m_bytes_left), i);
		save_item(NAME(m_pbus_dma[i].m_config), i);

		m_pbus_dma[i].m_timer = timer_alloc(TIMER_PBUS_DMA + i);
		m_pbus_dma[i].m_timer->adjust(attotime::never);
	}
}

void hpc3_device::device_reset()
{
	m_enetr_nbdp = 0x80000000;
	m_enetr_cbp = 0x80000000;

	m_unk0 = 0;
	m_unk1 = 0;
	m_ic_unk0 = 0;

	m_scsi0_desc = 0;
	m_scsi0_addr = 0;
	m_scsi0_flags = 0;
	m_scsi0_byte_count = 0;
	m_scsi0_next_addr = 0;
	m_scsi0_dma_ctrl = 0;

	for (uint32_t i = 0; i < 8; i++)
	{
		m_pbus_dma[i].m_active = 0;
		m_pbus_dma[i].m_cur_ptr = 0;
		m_pbus_dma[i].m_desc_ptr = 0;
		m_pbus_dma[i].m_desc_flags = 0;
		m_pbus_dma[i].m_next_ptr = 0;
		m_pbus_dma[i].m_bytes_left = 0;
		m_pbus_dma[i].m_config = 0;

		m_pbus_dma[i].m_active = false;
		m_pbus_dma[i].m_timer->adjust(attotime::never);
	}
}

void hpc3_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_PBUS_DMA+1:
	case TIMER_PBUS_DMA+2:
		do_pbus_dma(id - TIMER_PBUS_DMA);
		break;
	case TIMER_PBUS_DMA+0:
	case TIMER_PBUS_DMA+3:
	case TIMER_PBUS_DMA+4:
	case TIMER_PBUS_DMA+5:
	case TIMER_PBUS_DMA+6:
	case TIMER_PBUS_DMA+7:
		LOGMASKED(LOG_UNKNOWN, "HPC3: Ignoring active PBUS DMA on channel %d\n", id - TIMER_PBUS_DMA);
		break;
	default:
		assert_always(false, "Unknown id in hpc3_device::device_timer");
	}
}

void hpc3_device::do_pbus_dma(uint32_t channel)
{
	pbus_dma_t &dma = m_pbus_dma[channel];

	if (dma.m_active && (channel == 1 || channel == 2))
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		uint16_t temp16 = space.read_dword(dma.m_cur_ptr) >> 16;
		int16_t stemp16 = (int16_t)((temp16 >> 8) | (temp16 << 8));

		if (channel == 1)
			m_ldac->write(stemp16);
		else
			m_rdac->write(stemp16);

		dma.m_cur_ptr += 4;
		dma.m_bytes_left -= 4;

		if (dma.m_bytes_left == 0)
		{
			if (!BIT(dma.m_desc_flags, 31))
			{
				dma.m_desc_ptr = dma.m_next_ptr;
				LOGMASKED(LOG_PBUS_DMA, "Channel %d Next PBUS_DMA_DescPtr = %08x\n", channel, dma.m_desc_ptr); fflush(stdout);
				dma.m_cur_ptr = space.read_dword(dma.m_desc_ptr);
				dma.m_desc_flags = space.read_dword(dma.m_desc_ptr + 4);
				dma.m_bytes_left = dma.m_desc_flags & 0x7fffffff;
				dma.m_next_ptr = space.read_dword(dma.m_desc_ptr + 8);
				LOGMASKED(LOG_PBUS_DMA, "Channel %d Next PBUS_DMA_CurPtr = %08x\n", channel, dma.m_cur_ptr); fflush(stdout);
				LOGMASKED(LOG_PBUS_DMA, "Channel %d Next PBUS_DMA_BytesLeft = %08x\n", channel, dma.m_bytes_left); fflush(stdout);
				LOGMASKED(LOG_PBUS_DMA, "Channel %d Next PBUS_DMA_NextPtr = %08x\n", channel, dma.m_next_ptr); fflush(stdout);
			}
			else
			{
				dma.m_active = false;
				dma.m_timer->adjust(attotime::never);
				return;
			}
		}
		dma.m_timer->adjust(attotime::from_hz(44100));
	}
	else
	{
		dma.m_timer->adjust(attotime::never);
	}
}

READ32_MEMBER(hpc3_device::hd_enet_r)
{
	switch (offset)
	{
	case 0x0004/4:
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI0DESC Read: %08x (%08x): %08x\n", machine().describe_context(), 0x1fb90000 + (offset << 2), mem_mask, m_scsi0_desc);
		return m_scsi0_desc;
	case 0x1004/4:
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI0DMACTRL Read: %08x (%08x): %08x\n", machine().describe_context(), 0x1fb90000 + (offset << 2), mem_mask, m_scsi0_dma_ctrl);
		return m_scsi0_dma_ctrl;
	case 0x4000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 ENETR CBP Read: %08x (%08x): %08x\n", machine().describe_context(), 0x1fb90000 + (offset << 2), mem_mask, m_enetr_nbdp);
		return m_enetr_cbp;
	case 0x4004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 ENETR NBDP Read: %08x (%08x): %08x\n", machine().describe_context(), 0x1fb90000 + (offset << 2), mem_mask, m_enetr_nbdp);
		return m_enetr_nbdp;
	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown HPC3 ENET/HDx Read: %08x (%08x)\n", machine().describe_context(), 0x1fb90000 + (offset << 2), mem_mask);
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::hd_enet_w)
{
	switch (offset)
	{
	case 0x0004/4:
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI0DESC Write: %08x\n", machine().describe_context(), data);
		m_scsi0_desc = data;
		break;
	case 0x1004/4:
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI0DMACTRL Write: %08x\n", machine().describe_context(), data);
		m_scsi0_dma_ctrl = data;
		break;
	case 0x4000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 ENETR CBP Write: %08x\n", machine().describe_context(), data);
		m_enetr_cbp = data;
		break;
	case 0x4004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 ENETR NBDP Write: %08x\n", machine().describe_context(), data);
		m_enetr_nbdp = data;
		break;
	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown HPC3 ENET/HDx write: %08x (%08x): %08x\n", machine().describe_context(), 0x1fb90000 + (offset << 2), mem_mask, data);
		break;
	}
}

template<uint32_t index>
READ32_MEMBER(hpc3_device::hd_r)
{
	switch (offset)
	{
	case 0x0000/4:
	case 0x4000/4:
		if (ACCESSING_BITS_0_7)
		{
			const uint8_t ret = index ? m_wd33c93_2->read(space, 0) : m_wd33c93->read(space, 0);
			LOGMASKED(LOG_SCSI, "%s: SCSI%d Read 0: %02x\n", machine().describe_context(), index, ret);
			return ret;
		}
		break;
	case 0x0004/4:
	case 0x4004/4:
		if (ACCESSING_BITS_0_7)
		{
			const uint8_t ret = index ? m_wd33c93_2->read(space, 1) : m_wd33c93->read(space, 1);
			LOGMASKED(LOG_SCSI, "%s: SCSI%d Read 1: %02x\n", machine().describe_context(), index, ret);
			return ret;
		}
		break;
	default:
		LOGMASKED(LOG_SCSI | LOG_UNKNOWN, "%s: %s: Unknown HPC3 HD%d Read: %08x & %08x\n", machine().describe_context(), machine().describe_context(),
			index, 0x1fbc4000 + (offset << 2) + index * 0x8000, mem_mask);
		break;
	}
	return 0;
}

template<uint32_t index>
WRITE32_MEMBER(hpc3_device::hd_w)
{
	switch (offset)
	{
	case 0x0000:
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_SCSI, "%s: SCSI%d Write 0 = %02x\n", machine().describe_context(), index, (uint8_t)data);
			index ? m_wd33c93_2->write(space, 0, data & 0xff) : m_wd33c93->write(space, 0, data & 0xff);
		}
		break;
	case 0x0001:
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_SCSI, "%s: SCSI%d Write 1 = %02x\n", machine().describe_context(), index, (uint8_t)data);
			index ? m_wd33c93_2->write(space, 1, data & 0xff) : m_wd33c93->write(space, 1, data & 0xff);
		}
		break;
	default:
		LOGMASKED(LOG_SCSI | LOG_UNKNOWN, "%s: %s: Unknown HPC3 HD%d Write: %08x = %08x & %08x\n", machine().describe_context(), machine().describe_context(),
			index, 0x1fbc4000 + (offset << 2) + index * 0x8000, data, mem_mask);
		break;
	}
}

template DECLARE_READ32_MEMBER(hpc3_device::hd_r<0>);
template DECLARE_READ32_MEMBER(hpc3_device::hd_r<1>);
template DECLARE_WRITE32_MEMBER(hpc3_device::hd_w<0>);
template DECLARE_WRITE32_MEMBER(hpc3_device::hd_w<1>);

READ32_MEMBER(hpc3_device::pbus4_r)
{
	switch (offset)
	{
	case 0x0004/4:
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PBUS4 Unknown 0 Read: (%08x): %08x\n", machine().describe_context(), mem_mask, m_unk0);
		return m_unk0;
	case 0x000c/4:
		LOGMASKED(LOG_PBUS4, "%s: Interrupt Controller(?) Read: (%08x): %08x\n", machine().describe_context(), mem_mask, m_ic_unk0);
		return m_ic_unk0;
	case 0x0014/4:
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PBUS4 Unknown 1 Read: (%08x): %08x\n", machine().describe_context(), mem_mask, m_unk1);
		return m_unk1;
	default:
		LOGMASKED(LOG_PBUS4 | LOG_UNKNOWN, "%s: Unknown HPC3 PBUS4 Read: %08x (%08x)\n", machine().describe_context(), 0x1fbd9000 + (offset << 2), mem_mask);
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::pbus4_w)
{
	switch (offset)
	{
	case 0x0004/4:
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PBUS4 Unknown 0 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_unk0 = data;
		break;
	case 0x000c/4:
		LOGMASKED(LOG_PBUS4, "%s: Interrupt Controller(?) Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_ic_unk0 = data;
		break;
	case 0x0014/4:
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PBUS4 Unknown 1 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_unk1 = data;
		break;
	default:
		LOGMASKED(LOG_PBUS4 | LOG_UNKNOWN, "%s: Unknown HPC3 PBUS4 Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbd9000 + (offset << 2), data, mem_mask);
		break;
	}
}

READ32_MEMBER(hpc3_device::pbusdma_r)
{
	uint32_t channel = offset / (0x2000/4);
	LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Read: %08x & %08x\n", machine().describe_context(), channel, 0x1fb80000 + offset*4, mem_mask);
	return 0;
}

WRITE32_MEMBER(hpc3_device::pbusdma_w)
{
	uint32_t channel = offset / (0x2000/4);
	pbus_dma_t &dma = m_pbus_dma[channel];

	switch (offset & 0x07ff)
	{
	case 0x0000/4:
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Buffer Pointer Write: %08x\n", machine().describe_context(), channel, data);
		break;
	case 0x0004/4:
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Descriptor Pointer Write: %08x\n", machine().describe_context(), channel, data);
		dma.m_desc_ptr = data;
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS_DMA_DescPtr = %08x\n", machine().describe_context(), dma.m_desc_ptr); fflush(stdout);
		dma.m_cur_ptr = space.read_dword(dma.m_desc_ptr);
		dma.m_desc_flags = space.read_dword(dma.m_desc_ptr + 4);
		dma.m_next_ptr = space.read_dword(dma.m_desc_ptr + 8);
		dma.m_bytes_left = dma.m_desc_flags & 0x7fffffff;
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS_DMA_CurPtr = %08x\n", machine().describe_context(), dma.m_cur_ptr); fflush(stdout);
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS_DMA_BytesLeft = %08x\n", machine().describe_context(), dma.m_bytes_left); fflush(stdout);
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS_DMA_NextPtr = %08x\n", machine().describe_context(), dma.m_next_ptr); fflush(stdout);
		break;
	case 0x1000/4:
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Control Register Write: %08x\n", machine().describe_context(), channel, data);
		if (data & PBUS_CTRL_ENDIAN)
			LOGMASKED(LOG_PBUS_DMA, "    Little Endian\n");
		else
			LOGMASKED(LOG_PBUS_DMA, "    Big Endian\n");

		if (data & PBUS_CTRL_RECV)
			LOGMASKED(LOG_PBUS_DMA, "    RX DMA\n");
		else
			LOGMASKED(LOG_PBUS_DMA, "    TX DMA\n");

		if (data & PBUS_CTRL_FLUSH)
			LOGMASKED(LOG_PBUS_DMA, "    Flush for RX\n");
		if (data & PBUS_CTRL_DMASTART)
			LOGMASKED(LOG_PBUS_DMA, "    Start DMA\n");

		if (data & PBUS_CTRL_LOAD_EN)
			LOGMASKED(LOG_PBUS_DMA, "    Load Enable\n");

		LOGMASKED(LOG_PBUS_DMA, "    High Water Mark: %04x bytes\n", (data & PBUS_CTRL_HIGHWATER) >> 8);
		LOGMASKED(LOG_PBUS_DMA, "    FIFO Begin: Row %04x\n", (data & PBUS_CTRL_FIFO_BEG) >> 16);
		LOGMASKED(LOG_PBUS_DMA, "    FIFO End: Row %04x\n", (data & PBUS_CTRL_FIFO_END) >> 24);

		if (((data & PBUS_CTRL_DMASTART) && (data & PBUS_CTRL_LOAD_EN)) && (channel == 1 || channel == 2))
		{
			LOGMASKED(LOG_PBUS_DMA, "    Starting DMA\n");
			dma.m_timer->adjust(attotime::from_hz(44100));
			dma.m_active = true;
		}
		break;
	default:
		LOGMASKED(LOG_PBUS_DMA | LOG_UNKNOWN, "%s: Unknown PBUS DMA Channel %d Write: %08x = %08x & %08x\n", machine().describe_context(), channel, 0x1fb80000 + offset*4, data, mem_mask);
		break;
	}
}

READ32_MEMBER(hpc3_device::dma_config_r)
{
	const uint32_t channel = (offset >> 7) & 7;
	const uint32_t data = m_pbus_dma[channel].m_config;
	LOGMASKED(LOG_PBUS_DMA, "%s: Read Channel %d DMA Configuration: %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
	return data;
}

WRITE32_MEMBER(hpc3_device::dma_config_w)
{
	const uint32_t channel = (offset >> 7) & 7;
	COMBINE_DATA(&m_pbus_dma[channel].m_config);

	LOGMASKED(LOG_PBUS_DMA, "%s: Write Channel %d DMA Configuration: %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
	LOGMASKED(LOG_PBUS_DMA, "    DMA Read State D3 gio_clk cycles: %d\n", BIT(data, 0) ? 2 : 3);
	LOGMASKED(LOG_PBUS_DMA, "    DMA Read State D4 gio_clk cycles: %d\n", (data >> 1) & 0xf);
	LOGMASKED(LOG_PBUS_DMA, "    DMA Read State D5 gio_clk cycles: %d\n", (data >> 5) & 0xf);
	LOGMASKED(LOG_PBUS_DMA, "    DMA Write State D3 gio_clk cycles: %d\n", BIT(data, 9) ? 2 : 3);
	LOGMASKED(LOG_PBUS_DMA, "    DMA Write State D4 gio_clk cycles: %d\n", (data >> 10) & 0xf);
	LOGMASKED(LOG_PBUS_DMA, "    DMA Write State D5 gio_clk cycles: %d\n", (data >> 14) & 0xf);
	LOGMASKED(LOG_PBUS_DMA, "    Device Bit Width: %d\n", BIT(data, 18) ? 16 : 32);
	LOGMASKED(LOG_PBUS_DMA, "    Even Address Bytes on %s\n", BIT(data, 19) ? "15..8" : "7..0");
	LOGMASKED(LOG_PBUS_DMA, "    Device %s Real-Time\n", BIT(data, 21) ? "is" : "is not");
	LOGMASKED(LOG_PBUS_DMA, "    Burst Count: %d\n", (data >> 22) & 0x1f);
	LOGMASKED(LOG_PBUS_DMA, "    %sUse Unsynchronized DREQ\n", BIT(data, 27) ? "" : "Do Not ");
}

READ32_MEMBER(hpc3_device::pio_config_r)
{
	uint32_t channel = (offset >> 6) & 15;
	if (channel >= 10)
	{
		channel = (channel & 1) ? 9 : 8;
	}

	const uint32_t data = m_pio_config[channel];
	LOGMASKED(LOG_PBUS_DMA, "%s: Read Channel %d PIO Configuration: %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
	return data;
}

WRITE32_MEMBER(hpc3_device::pio_config_w)
{
	uint32_t channel = (offset >> 6) & 15;
	if (channel >= 10)
	{
		channel = (channel & 1) ? 9 : 8;
	}

	COMBINE_DATA(&m_pio_config[channel]);
	LOGMASKED(LOG_PBUS_DMA, "%s: Write Channel %d PIO Configuration: %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
	LOGMASKED(LOG_PBUS_DMA, "    PIO Read State P2 gio_clk cycles: %d\n", BIT(data, 0) ? 1 : 2);
	LOGMASKED(LOG_PBUS_DMA, "    PIO Read State P3 gio_clk cycles: %d\n", (data >> 1) & 0xf);
	LOGMASKED(LOG_PBUS_DMA, "    PIO Read State P4 gio_clk cycles: %d\n", (data >> 5) & 0xf);
	LOGMASKED(LOG_PBUS_DMA, "    PIO Write State P2 gio_clk cycles: %d\n", BIT(data, 9) ? 1 : 2);
	LOGMASKED(LOG_PBUS_DMA, "    PIO Write State P3 gio_clk cycles: %d\n", (data >> 10) & 0xf);
	LOGMASKED(LOG_PBUS_DMA, "    PIO Write State P4 gio_clk cycles: %d\n", (data >> 14) & 0xf);
	LOGMASKED(LOG_PBUS_DMA, "    Device Bit Width: %d\n", BIT(data, 18) ? 16 : 32);
	LOGMASKED(LOG_PBUS_DMA, "    Even Address Bytes on %s\n", BIT(data, 19) ? "15..8" : "7..0");
}

READ32_MEMBER(hpc3_device::unkpbus0_r)
{
	LOGMASKED(LOG_UNKNOWN, "%s: Unknown PBUS Read: %08x & %08x\n", machine().describe_context(), 0x1fbc8000 + offset*4, mem_mask);
	return 0;
}

WRITE32_MEMBER(hpc3_device::unkpbus0_w)
{
	LOGMASKED(LOG_UNKNOWN, "%s: Unknown PBUS Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbc8000 + offset*4, data, mem_mask);
}

void hpc3_device::dump_chain(address_space &space, uint32_t ch_base)
{
	LOGMASKED(LOG_CHAIN, "node: %08x %08x %08x (len = %x)\n", space.read_dword(ch_base), space.read_dword(ch_base+4), space.read_dword(ch_base+8), space.read_dword(ch_base+4) & 0x3fff);

	if ((space.read_dword(ch_base+8) != 0) && !(space.read_dword(ch_base+4) & 0x80000000))
	{
		dump_chain(space, space.read_dword(ch_base+8));
	}
}

void hpc3_device::fetch_chain(address_space &space)
{
	m_scsi0_addr = space.read_dword(m_scsi0_desc);
	m_scsi0_flags = space.read_dword(m_scsi0_desc+4);
	m_scsi0_byte_count = m_scsi0_flags & 0x3fff;
	m_scsi0_next_addr = space.read_dword(m_scsi0_desc+8);
	LOGMASKED(LOG_CHAIN, "Fetching chain from %08x: %08x %08x %08x (length %04x)\n", m_scsi0_desc, m_scsi0_addr, m_scsi0_flags, m_scsi0_next_addr, m_scsi0_byte_count);
}

bool hpc3_device::decrement_chain(address_space &space)
{
	m_scsi0_byte_count--;
	if (m_scsi0_byte_count == 0)
	{
		if (BIT(m_scsi0_flags, 31))
		{
			return false;
		}
		m_scsi0_desc = m_scsi0_next_addr;
		fetch_chain(space);
	}
	return true;
}

WRITE_LINE_MEMBER(hpc3_device::scsi_irq)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (state)
	{
		uint8_t dma_buffer[4096];
		if (m_wd33c93->get_dma_count())
		{
			LOGMASKED(LOG_SCSI, "m_wd33c93->get_dma_count() is %d\n", m_wd33c93->get_dma_count());
			if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE)
			{
				if (m_scsi0_dma_ctrl & HPC3_DMACTRL_IRQ)
					LOGMASKED(LOG_SCSI, "IP22: Unhandled SCSI DMA IRQ\n");
			}

			bool big_endian = (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN);
			if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE)
			{
				if (m_scsi0_dma_ctrl & HPC3_DMACTRL_DIR)
				{
					// HPC3 DMA: host to device
					int byte_count = m_wd33c93->get_dma_count();
					//dump_chain(space, m_scsi0_desc);
					fetch_chain(space);

					LOGMASKED(LOG_SCSI, "DMA to SCSI device: %d bytes from %08x\n", byte_count, m_scsi0_addr);

					if (byte_count <= 512)
					{
						for (int i = 0; i < byte_count; i++)
						{
							dma_buffer[big_endian ? BYTE4_XOR_BE(i) : BYTE4_XOR_LE(i)] = space.read_byte(m_scsi0_addr+i);
							if (!decrement_chain(space))
								break;
						}

						m_wd33c93->dma_write_data(byte_count, dma_buffer);
					}
					else
					{
						int dstoffs = 0;
						while (byte_count)
						{
							int sub_count = std::min(512, byte_count);

							for (int i = 0; i < sub_count; i++)
							{
								dma_buffer[big_endian ? BYTE4_XOR_BE(dstoffs+i) : BYTE4_XOR_LE(dstoffs+i)] = space.read_byte(m_scsi0_addr);
								m_scsi0_addr++;
								if (!decrement_chain(space))
									break;
							}

							m_wd33c93->dma_write_data(sub_count, dma_buffer);

							byte_count -= sub_count;
						}
					}

					// clear DMA on the controller too
					m_wd33c93->clear_dma();
				}
				else
				{
					// HPC3 DMA: device to host
					int byte_count = m_wd33c93->get_dma_count();
					//dump_chain(space, m_scsi0_desc);
					fetch_chain(space);

					LOGMASKED(LOG_SCSI, "DMA from SCSI device: %d bytes to %08x\n", byte_count, m_scsi0_addr);

					if (byte_count < 512)
					{
						m_wd33c93->dma_read_data(byte_count, dma_buffer);

						for (int i = 0; i < byte_count; i++)
						{
							space.write_byte(big_endian ? BYTE4_XOR_BE(m_scsi0_addr+i) : BYTE4_XOR_LE(m_scsi0_addr+i), dma_buffer[i]);
							if (!decrement_chain(space))
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
								space.write_byte(big_endian ? BYTE4_XOR_BE(m_scsi0_addr) : BYTE4_XOR_LE(m_scsi0_addr), dma_buffer[i]);
								m_scsi0_addr++;
								if (!decrement_chain(space))
									break;
							}

							byte_count -= sub_count;
						}
					}

					// clear DMA on the controller too
					m_wd33c93->clear_dma();
				}
			}
		}

		// clear HPC3 DMA active flag
		m_scsi0_dma_ctrl &= ~HPC3_DMACTRL_ENABLE;

		// set the interrupt
		m_ioc2->raise_local0_irq(ioc2_device::INT3_LOCAL0_SCSI0);
	}
	else
	{
		m_ioc2->lower_local0_irq(ioc2_device::INT3_LOCAL0_SCSI0);
	}
}
