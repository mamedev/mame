// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC3 "High-performance Peripheral Controller" emulation

**********************************************************************/

#include "emu.h"
#include "machine/hpc3.h"

#define LOG_UNKNOWN     (1 << 0)
#define LOG_PBUS_DMA    (1 << 1)
#define LOG_SCSI        (1 << 2)
#define LOG_SCSI_DMA    (1 << 3)
#define LOG_SCSI_IRQ    (1 << 4)
#define LOG_ETHERNET    (1 << 5)
#define LOG_CHAIN       (1 << 6)
#define LOG_EEPROM      (1 << 7)
#define LOG_ALL         (LOG_UNKNOWN | LOG_PBUS_DMA | LOG_SCSI | LOG_SCSI_DMA | LOG_SCSI_IRQ | LOG_ETHERNET | LOG_CHAIN | LOG_EEPROM)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_HPC3, hpc3_device, "hpc3", "SGI HPC3")

hpc3_device::hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HPC3, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_pio_space_config{
		{"PIO channel 0", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 1", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 2", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 3", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 4", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 5", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 6", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 7", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 8", ENDIANNESS_LITTLE, 16, 8, -1},
		{"PIO channel 9", ENDIANNESS_LITTLE, 16, 8, -1}}
	, m_gio64_space(*this, finder_base::DUMMY_TAG, -1)
	, m_hal2(*this, finder_base::DUMMY_TAG)
	, m_enet_rd_cb(*this)
	, m_enet_wr_cb(*this)
	, m_enet_rxrd_cb(*this)
	, m_enet_txwr_cb(*this)
	, m_enet_d8_rd_cb(*this)
	, m_enet_d8_wr_cb(*this)
	, m_enet_reset_cb(*this)
	, m_enet_loopback_cb(*this)
	, m_enet_intr_out_cb(*this)
	, m_hd_rd_cb{{*this}, {*this}}
	, m_hd_wr_cb{{*this}, {*this}}
	, m_hd_dma_rd_cb{{*this}, {*this}}
	, m_hd_dma_wr_cb{{*this}, {*this}}
	, m_hd_reset_cb{{*this}, {*this}}
	, m_bbram_rd_cb(*this)
	, m_bbram_wr_cb(*this)
	, m_eeprom_dati_cb(*this)
	, m_eeprom_dato_cb(*this)
	, m_eeprom_clk_cb(*this)
	, m_eeprom_cs_cb(*this)
	, m_eeprom_pre_cb(*this)
	, m_dma_complete_int_cb(*this)
{
}

device_memory_interface::space_config_vector hpc3_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PIO0, &m_pio_space_config[0]),
		std::make_pair(AS_PIO1, &m_pio_space_config[1]),
		std::make_pair(AS_PIO2, &m_pio_space_config[2]),
		std::make_pair(AS_PIO3, &m_pio_space_config[3]),
		std::make_pair(AS_PIO4, &m_pio_space_config[4]),
		std::make_pair(AS_PIO5, &m_pio_space_config[5]),
		std::make_pair(AS_PIO6, &m_pio_space_config[6]),
		std::make_pair(AS_PIO7, &m_pio_space_config[7]),
		std::make_pair(AS_PIO8, &m_pio_space_config[8]),
		std::make_pair(AS_PIO9, &m_pio_space_config[9])
	};
}

void hpc3_device::device_resolve_objects()
{
	m_enet_rd_cb.resolve();
	m_enet_wr_cb.resolve_safe();
	m_enet_rxrd_cb.resolve_safe(0);
	m_enet_txwr_cb.resolve_safe();
	m_enet_d8_rd_cb.resolve_safe(0);
	m_enet_d8_wr_cb.resolve_safe();
	m_enet_reset_cb.resolve_safe();
	m_enet_loopback_cb.resolve_safe();
	m_enet_intr_out_cb.resolve_safe();
	for (int index = 0; index < 2; index++)
	{
		m_hd_rd_cb[index].resolve();
		m_hd_wr_cb[index].resolve();
		m_hd_dma_rd_cb[index].resolve_safe(0);
		m_hd_dma_wr_cb[index].resolve_safe();
		m_hd_reset_cb[index].resolve_safe();
	}
	m_bbram_rd_cb.resolve_safe(0);
	m_bbram_wr_cb.resolve_safe();
	m_eeprom_dati_cb.resolve_safe(0);
	m_eeprom_dato_cb.resolve_safe();
	m_eeprom_clk_cb.resolve_safe();
	m_eeprom_cs_cb.resolve_safe();
	m_eeprom_pre_cb.resolve_safe();
	m_dma_complete_int_cb.resolve_safe();
}

void hpc3_device::device_start()
{
	for (uint32_t i = 0; i < 10; i++)
		m_pio_space[i] = &space(AS_PIO0 + i);

	save_item(NAME(m_intstat));
	save_item(NAME(m_misc));
	save_item(NAME(m_cpu_aux_ctrl));
	save_item(NAME(m_pio_config));

	for (uint32_t i = 0; i < 2; i++)
	{
		save_item(NAME(m_scsi_dma[i].m_cbp), i);
		save_item(NAME(m_scsi_dma[i].m_nbdp), i);
		save_item(NAME(m_scsi_dma[i].m_ctrl), i);
		save_item(NAME(m_scsi_dma[i].m_bc), i);
		save_item(NAME(m_scsi_dma[i].m_count), i);
		save_item(NAME(m_scsi_dma[i].m_dmacfg), i);
		save_item(NAME(m_scsi_dma[i].m_piocfg), i);
		save_item(NAME(m_scsi_dma[i].m_drq), i);
		save_item(NAME(m_scsi_dma[i].m_big_endian), i);
		save_item(NAME(m_scsi_dma[i].m_to_device), i);
		save_item(NAME(m_scsi_dma[i].m_active), i);
	}

	for (uint32_t i = 0; i < 2; i++)
	{
		save_item(NAME(m_enet_dma[i].m_cbp), i);
		save_item(NAME(m_enet_dma[i].m_nbdp), i);
		save_item(NAME(m_enet_dma[i].m_bc), i);
		save_item(NAME(m_enet_dma[i].m_ctrl), i);
		save_item(NAME(m_enet_dma[i].m_gio_fifo_ptr), i);
		save_item(NAME(m_enet_dma[i].m_dev_fifo_ptr), i);
	}
	save_item(NAME(m_enet_reset));
	save_item(NAME(m_enet_dmacfg));
	save_item(NAME(m_enet_piocfg));

	for (uint32_t i = 0; i < 8; i++)
	{
		save_item(NAME(m_pbus_dma[i].m_active), i);
		save_item(NAME(m_pbus_dma[i].m_cur_ptr), i);
		save_item(NAME(m_pbus_dma[i].m_desc_ptr), i);
		save_item(NAME(m_pbus_dma[i].m_desc_flags), i);
		save_item(NAME(m_pbus_dma[i].m_next_ptr), i);
		save_item(NAME(m_pbus_dma[i].m_bytes_left), i);
		save_item(NAME(m_pbus_dma[i].m_config), i);
		save_item(NAME(m_pbus_dma[i].m_control), i);

		m_pbus_dma[i].m_timer = timer_alloc(TIMER_PBUS_DMA + i);
		m_pbus_dma[i].m_timer->adjust(attotime::never);
	}

	m_pbus_fifo = make_unique_clear<uint32_t[]>(96);
	m_scsi_fifo[0] = make_unique_clear<uint32_t[]>(96);
	m_scsi_fifo[1] = make_unique_clear<uint32_t[]>(96);
	m_enet_fifo[ENET_RECV] = make_unique_clear<uint32_t[]>(32);
	m_enet_fifo[ENET_XMIT] = make_unique_clear<uint32_t[]>(40);

	save_pointer(NAME(m_pbus_fifo), 96);
	save_pointer(NAME(m_scsi_fifo[0]), 96);
	save_pointer(NAME(m_scsi_fifo[1]), 96);
	save_pointer(NAME(m_enet_fifo[ENET_RECV]), 32);
	save_pointer(NAME(m_enet_fifo[ENET_XMIT]), 40);
}

void hpc3_device::device_reset()
{
	m_cpu_aux_ctrl = 0;

	memset(m_scsi_dma, 0, sizeof(scsi_dma_t) * 2);
	memset(m_enet_dma, 0, sizeof(enet_dma_t) * 2);
	m_enet_dmacfg = 0;
	m_enet_piocfg = 0;

	m_enet_dma[ENET_RECV].m_cbp = 0x80000000;
	m_enet_dma[ENET_XMIT].m_cbp = 0x80000000;
	m_enet_dma[ENET_RECV].m_nbdp = 0x80000000;
	m_enet_dma[ENET_XMIT].m_nbdp = 0x80000000;

	for (uint32_t i = 0; i < 8; i++)
	{
		m_pbus_dma[i].m_active = 0;
		m_pbus_dma[i].m_cur_ptr = 0;
		m_pbus_dma[i].m_desc_ptr = 0;
		m_pbus_dma[i].m_desc_flags = 0;
		m_pbus_dma[i].m_next_ptr = 0;
		m_pbus_dma[i].m_bytes_left = 0;
		m_pbus_dma[i].m_config = 0;
		m_pbus_dma[i].m_control = 0;

		m_pbus_dma[i].m_active = false;
		m_pbus_dma[i].m_timer->adjust(attotime::never);
	}

	m_intstat = 0;
	m_dma_complete_int_cb(0);
}

void hpc3_device::map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rw(FUNC(hpc3_device::pbusdma_r), FUNC(hpc3_device::pbusdma_w));
	map(0x00010000, 0x0001ffff).rw(FUNC(hpc3_device::hd_enet_r), FUNC(hpc3_device::hd_enet_w));
	map(0x00020000, 0x000202ff).rw(FUNC(hpc3_device::fifo_r<FIFO_PBUS>), FUNC(hpc3_device::fifo_w<FIFO_PBUS>)); // PBUS FIFO
	map(0x00028000, 0x000282ff).rw(FUNC(hpc3_device::fifo_r<FIFO_SCSI0>), FUNC(hpc3_device::fifo_w<FIFO_SCSI0>)); // SCSI0 FIFO
	map(0x0002a000, 0x0002a2ff).rw(FUNC(hpc3_device::fifo_r<FIFO_SCSI1>), FUNC(hpc3_device::fifo_w<FIFO_SCSI1>)); // SCSI1 FIFO
	map(0x0002c000, 0x0002c0ff).rw(FUNC(hpc3_device::fifo_r<FIFO_ENET_RECV>), FUNC(hpc3_device::fifo_w<FIFO_ENET_RECV>)); // ENET Recv FIFO
	map(0x0002e000, 0x0002e13f).rw(FUNC(hpc3_device::fifo_r<FIFO_ENET_XMIT>), FUNC(hpc3_device::fifo_w<FIFO_ENET_XMIT>)); // ENET Xmit FIFO
	map(0x00030000, 0x00030003).r(FUNC(hpc3_device::intstat_r));
	map(0x00030004, 0x00030007).rw(FUNC(hpc3_device::misc_r), FUNC(hpc3_device::misc_w));
	map(0x00030008, 0x0003000b).rw(FUNC(hpc3_device::eeprom_r), FUNC(hpc3_device::eeprom_w));
	map(0x0003000c, 0x0003000f).r(FUNC(hpc3_device::intstat_r));
	map(0x00040000, 0x00047fff).rw(FUNC(hpc3_device::hd_r<0>), FUNC(hpc3_device::hd_w<0>));
	map(0x00048000, 0x0004ffff).rw(FUNC(hpc3_device::hd_r<1>), FUNC(hpc3_device::hd_w<1>));
	map(0x00054000, 0x000544ff).rw(FUNC(hpc3_device::enet_r), FUNC(hpc3_device::enet_w));
	map(0x00058000, 0x0005bfff).rw(FUNC(hpc3_device::pio_data_r), FUNC(hpc3_device::pio_data_w));
	map(0x0005c000, 0x0005cfff).rw(FUNC(hpc3_device::dma_config_r), FUNC(hpc3_device::dma_config_w));
	map(0x0005d000, 0x0005dfff).rw(FUNC(hpc3_device::pio_config_r), FUNC(hpc3_device::pio_config_w));
	map(0x00060000, 0x0007ffff).rw(FUNC(hpc3_device::bbram_r), FUNC(hpc3_device::bbram_w));
}

void hpc3_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_PBUS_DMA+0:
	case TIMER_PBUS_DMA+1:
	case TIMER_PBUS_DMA+2:
	case TIMER_PBUS_DMA+3:
		do_pbus_dma(id - TIMER_PBUS_DMA);
		break;
	case TIMER_PBUS_DMA+4:
	case TIMER_PBUS_DMA+5:
	case TIMER_PBUS_DMA+6:
	case TIMER_PBUS_DMA+7:
		LOGMASKED(LOG_UNKNOWN, "HPC3: Ignoring active PBUS DMA on channel %d\n", id - TIMER_PBUS_DMA);
		break;
	default:
		throw emu_fatalerror("Unknown id in hpc3_device::device_timer");
	}
}

void hpc3_device::do_pbus_dma(uint32_t channel)
{
	pbus_dma_t &dma = m_pbus_dma[channel];

	if (dma.m_active && channel < 4)
	{
		uint16_t temp16 = m_gio64_space->read_dword(dma.m_cur_ptr) >> 16;
		int16_t stemp16 = (int16_t)(BIT(m_pbus_dma[channel].m_config, 19) ? temp16 : swapendian_int16(temp16));

		m_hal2->dma_write(channel, stemp16);

		dma.m_cur_ptr += 4;
		dma.m_bytes_left -= 4;

		if (dma.m_bytes_left == 0)
		{
			if (BIT(dma.m_desc_flags, 29))
			{
				LOGMASKED(LOG_PBUS_DMA, "Raising channel %d IRQ\n", channel);
				m_intstat |= 1 << channel;
				m_dma_complete_int_cb(1);
			}
			if (!BIT(dma.m_desc_flags, 31))
			{
				dma.m_desc_ptr = dma.m_next_ptr;
				LOGMASKED(LOG_PBUS_DMA, "Channel %d Next PBUS_DMA_DescPtr = %08x\n", channel, dma.m_desc_ptr); fflush(stdout);
				dma.m_cur_ptr = m_gio64_space->read_dword(dma.m_desc_ptr);
				dma.m_desc_flags = m_gio64_space->read_dword(dma.m_desc_ptr + 4);
				dma.m_bytes_left = dma.m_desc_flags & 0x3fff;
				dma.m_next_ptr = m_gio64_space->read_dword(dma.m_desc_ptr + 8);
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
		dma.m_timer->adjust(m_hal2->get_rate(channel));
	}
	else
	{
		dma.m_timer->adjust(attotime::never);
	}
}

READ32_MEMBER(hpc3_device::enet_r)
{
	switch (offset)
	{
		case 0x000/4:
			LOGMASKED(LOG_ETHERNET, "%s: HPC3: enet_r: Read MAC Address bytes 0-3, 0x80675309 & %08x\n", machine().describe_context(), mem_mask);
			return 0x80675309;
		default:
			LOGMASKED(LOG_ETHERNET, "%s: HPC3: enet_r: Read Unknown Register %08x & %08x\n", machine().describe_context(), 0x1fbd4000 + (offset << 2), mem_mask);
			return 0;
	}
}

WRITE32_MEMBER(hpc3_device::enet_w)
{
	switch (offset)
	{
		default:
			LOGMASKED(LOG_ETHERNET, "%s: HPC3: enet_w: Write Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x1fbd4000 + (offset << 2), data, mem_mask);
			break;
	}
}

READ32_MEMBER(hpc3_device::hd_enet_r)
{
	switch (offset)
	{
	case 0x0000/4:
	case 0x2000/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d Current Buffer Pointer Read: %08x & %08x\n", machine().describe_context(), channel, m_scsi_dma[channel].m_cbp, mem_mask);
		return m_scsi_dma[channel].m_cbp;
	}
	case 0x0004/4:
	case 0x2004/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d Next Buffer Desc Pointer Read: %08x & %08x\n", machine().describe_context(), channel, m_scsi_dma[channel].m_nbdp, mem_mask);
		return m_scsi_dma[channel].m_nbdp;
	}
	case 0x1000/4:
	case 0x3000/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		const uint32_t ret = (m_scsi_dma[channel].m_count & 0x3fff) | (m_scsi_dma[channel].m_bc & 0xffffc000);
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d Buffer Count Read: %08x & %08x\n", machine().describe_context(), channel, ret, mem_mask);
		return ret;
	}
	case 0x1004/4:
	case 0x3004/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		uint32_t ret = m_scsi_dma[channel].m_ctrl;
		if (BIT(m_intstat, channel + 8))
		{
			ret |= HPC3_DMACTRL_IRQ;
			if (!machine().side_effects_disabled())
			{
				LOGMASKED(LOG_SCSI_IRQ, "Lowering SCSI %d IRQ\n", channel);
				m_intstat &= ~(0x100 << channel);
				if (m_intstat == 0)
					m_dma_complete_int_cb(0);
			}
		}
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d Control Read: %08x & %08x\n", machine().describe_context(), channel, ret, mem_mask);
		return ret;
	}
	case 0x1008/4:
	case 0x3008/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d GIO FIFO Pointer Read: %08x & %08x\n", machine().describe_context(), channel, 0, mem_mask);
		return 0;
	}
	case 0x100c/4:
	case 0x300c/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d Device FIFO Pointer Read: %08x & %08x\n", machine().describe_context(), channel, 0, mem_mask);
		return 0;
	}
	case 0x1010/4:
	case 0x3010/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d DMA Config Read: %08x & %08x\n", machine().describe_context(), channel, m_scsi_dma[channel].m_dmacfg, mem_mask);
		return m_scsi_dma[channel].m_dmacfg;
	}
	case 0x1014/4:
	case 0x3014/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d PIO Config Read: %08x & %08x\n", machine().describe_context(), channel, m_scsi_dma[channel].m_piocfg, mem_mask);
		return m_scsi_dma[channel].m_piocfg;
	}
	case 0x4000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver Current Buffer Pointer Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_RECV].m_cbp);
		return m_enet_dma[ENET_RECV].m_cbp;
	case 0x4004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver Next Buffer Desc Pointer Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_RECV].m_nbdp);
		return m_enet_dma[ENET_RECV].m_nbdp;
	case 0x5000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver Buffer Count Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_RECV].m_bc);
		return m_enet_dma[ENET_RECV].m_bc;
	case 0x5004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver DMA Control Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_RECV].m_ctrl);
		return m_enet_dma[ENET_RECV].m_ctrl;
	case 0x5008/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver GIO FIFO Pointer Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_RECV].m_gio_fifo_ptr);
		return m_enet_dma[ENET_RECV].m_gio_fifo_ptr;
	case 0x500c/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver Device FIFO Pointer Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_RECV].m_dev_fifo_ptr);
		return m_enet_dma[ENET_RECV].m_dev_fifo_ptr;
	case 0x5014/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Reset Register Read: %08x\n", machine().describe_context(), m_enet_reset);
		return m_enet_reset;
	case 0x5018/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet DMA Config Read: %08x\n", machine().describe_context(), m_enet_dmacfg);
		return m_enet_dmacfg;
	case 0x501c/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet PIO Config Read: %08x\n", machine().describe_context(), m_enet_piocfg);
		return m_enet_piocfg;
	case 0x6000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter Current Buffer Pointer Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_XMIT].m_cbp);
		return m_enet_dma[ENET_XMIT].m_cbp;
	case 0x6004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter Next Buffer Desc Pointer Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_XMIT].m_nbdp);
		return m_enet_dma[ENET_XMIT].m_nbdp;
	case 0x7000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter Buffer Count Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_XMIT].m_bc);
		return m_enet_dma[ENET_XMIT].m_bc;
	case 0x7004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter DMA Control Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_XMIT].m_ctrl);
		return m_enet_dma[ENET_XMIT].m_ctrl;
	case 0x7008/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter GIO FIFO Pointer Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_XMIT].m_gio_fifo_ptr);
		return m_enet_dma[ENET_XMIT].m_gio_fifo_ptr;
	case 0x700c/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter Device FIFO Pointer Read: %08x\n", machine().describe_context(), m_enet_dma[ENET_XMIT].m_dev_fifo_ptr);
		return m_enet_dma[ENET_XMIT].m_dev_fifo_ptr;
	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown HPC3 ENET/HDx Read: %08x & %08x\n", machine().describe_context(), 0x1fb90000 + (offset << 2), mem_mask);
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::hd_enet_w)
{
	switch (offset)
	{
	case 0x0004/4:
	case 0x2004/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d Next Buffer Desc Pointer Write: %08x\n", machine().describe_context(), channel, data);
		m_scsi_dma[channel].m_nbdp = data;
		break;
	}
	case 0x1000/4:
	case 0x3000/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d Buffer Count Write: %08x\n", machine().describe_context(), channel, data);
		m_scsi_dma[channel].m_bc = data;
	}
	case 0x1004/4:
	case 0x3004/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d DMA Control Write: %08x\n", machine().describe_context(), channel, data);
		const bool was_active = m_scsi_dma[channel].m_active;
		if (data & HPC3_DMACTRL_WRMASK)
		{
			m_scsi_dma[channel].m_ctrl = data & ~HPC3_DMACTRL_IRQ & ~HPC3_DMACTRL_ENABLE & ~HPC3_DMACTRL_WRMASK;
			if (was_active)
				m_scsi_dma[channel].m_ctrl |= HPC3_DMACTRL_ENABLE;
		}
		else
		{
			m_scsi_dma[channel].m_ctrl = data & ~HPC3_DMACTRL_IRQ & ~HPC3_DMACTRL_WRMASK;
			m_scsi_dma[channel].m_active = (m_scsi_dma[channel].m_ctrl & HPC3_DMACTRL_ENABLE);
		}
		m_scsi_dma[channel].m_to_device = (m_scsi_dma[channel].m_ctrl & HPC3_DMACTRL_DIR);
		m_scsi_dma[channel].m_big_endian = (m_scsi_dma[channel].m_ctrl & HPC3_DMACTRL_ENDIAN);
		if (!was_active && m_scsi_dma[channel].m_active)
		{
			fetch_chain(channel);
		}
		m_hd_reset_cb[channel](BIT(data, 6));
		if (BIT(data, 3))
		{
			scsi_fifo_flush(channel);
		}
		if (m_scsi_dma[channel].m_drq && m_scsi_dma[channel].m_active)
		{
			do_scsi_dma(channel);
		}
		break;
	}
	case 0x1010/4:
	case 0x3010/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d DMA Config Write: %08x\n", machine().describe_context(), channel, data);
		m_scsi_dma[channel].m_dmacfg = data;
		break;
	}
	case 0x1014/4:
	case 0x3014/4:
	{
		const uint32_t channel = (offset & 0x2000/4) ? 1 : 0;
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI%d PIO Config Write: %08x\n", machine().describe_context(), channel, data);
		m_scsi_dma[channel].m_piocfg = data;
		break;
	}
	case 0x4000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver Current Buffer Pointer Write: %08x\n", machine().describe_context(), data);
		m_enet_dma[ENET_RECV].m_cbp = data;
		break;
	case 0x4004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver Next Buffer Desc Pointer Write: %08x\n", machine().describe_context(), data);
		m_enet_dma[ENET_RECV].m_nbdp = data;
		break;
	case 0x5000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver Buffer Count Write (ignored): %08x\n", machine().describe_context(), data);
		break;
	case 0x5004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver DMA Control Write: %08x\n", machine().describe_context(), data);
		m_enet_dma[ENET_RECV].m_ctrl = data;
		break;
	case 0x5008/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver GIO FIFO Pointer Write (ignored): %08x\n", machine().describe_context(), data);
		break;
	case 0x500c/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Receiver Device FIFO Pointer Write (ignored): %08x\n", machine().describe_context(), data);
		break;
	case 0x5014/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Reset Register Write: %08x\n", machine().describe_context(), data);
		m_enet_reset = data;
		break;
	case 0x5018/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet DMA Config Write: %08x\n", machine().describe_context(), data);
		m_enet_dmacfg = data;
		break;
	case 0x501c/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet PIO Config Write: %08x\n", machine().describe_context(), data);
		m_enet_piocfg = data;
		break;
	case 0x6000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter Current Buffer Pointer Write (ignored): %08x\n", machine().describe_context(), data);
		break;
	case 0x6004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter Next Buffer Desc Pointer Write: %08x\n", machine().describe_context(), data);
		m_enet_dma[ENET_XMIT].m_nbdp = data;
		break;
	case 0x7000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter Buffer Count Write (ignored): %08x\n", machine().describe_context(), data);
		break;
	case 0x7004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter DMA Control Write: %08x\n", machine().describe_context(), data);
		m_enet_dma[ENET_XMIT].m_ctrl = data;
		break;
	case 0x7008/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter GIO FIFO Pointer Write (ignored): %08x\n", machine().describe_context(), data);
		break;
	case 0x700c/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet Transmitter Device FIFO Pointer Write (ignored): %08x\n", machine().describe_context(), data);
		break;
	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown HPC3 ENET/HDx write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fb90000 + (offset << 2), data, mem_mask);
		break;
	}
}

WRITE_LINE_MEMBER(hpc3_device::enet_txrdy_w)
{
	// TODO
}

WRITE_LINE_MEMBER(hpc3_device::enet_rxrdy_w)
{
	// TODO
}

WRITE_LINE_MEMBER(hpc3_device::enet_rxdc_w)
{
	// TODO
}

WRITE_LINE_MEMBER(hpc3_device::enet_txret_w)
{
	// TODO
}

WRITE_LINE_MEMBER(hpc3_device::enet_intr_in_w)
{
	// TODO
}

template<hpc3_device::fifo_type_t Type>
READ32_MEMBER(hpc3_device::fifo_r)
{
	uint32_t ret = 0;
	if (Type == FIFO_PBUS)
		ret = m_pbus_fifo[offset >> 1];
	else if (Type == FIFO_SCSI0)
		ret = m_scsi_fifo[0][offset >> 1];
	else if (Type == FIFO_SCSI1)
		ret = m_scsi_fifo[1][offset >> 1];
	else if (Type == FIFO_ENET_RECV)
		ret = m_enet_fifo[ENET_RECV][offset >> 1];
	else if (Type == FIFO_ENET_XMIT)
		ret = m_enet_fifo[ENET_XMIT][offset >> 1];
	logerror("Reading %08x from %d FIFO offset %08x (%08x)\n", ret, Type, offset, offset >> 1);
	return ret;
}

template<hpc3_device::fifo_type_t Type>
WRITE32_MEMBER(hpc3_device::fifo_w)
{
	logerror("Writing %08x to %d FIFO offset %08x (%08x)\n", data, Type, offset, offset >> 2);
	if (Type == FIFO_PBUS)
		m_pbus_fifo[offset >> 2] = data;
	else if (Type == FIFO_SCSI0)
		m_scsi_fifo[0][offset >> 1] = data;
	else if (Type == FIFO_SCSI1)
		m_scsi_fifo[1][offset >> 1] = data;
	else if (Type == FIFO_ENET_RECV)
		m_enet_fifo[ENET_RECV][offset >> 2] = data;
	else if (Type == FIFO_ENET_XMIT)
		m_enet_fifo[ENET_XMIT][offset >> 2] = data;
}

template READ32_MEMBER(hpc3_device::fifo_r<hpc3_device::FIFO_PBUS>);
template READ32_MEMBER(hpc3_device::fifo_r<hpc3_device::FIFO_SCSI0>);
template READ32_MEMBER(hpc3_device::fifo_r<hpc3_device::FIFO_SCSI1>);
template READ32_MEMBER(hpc3_device::fifo_r<hpc3_device::FIFO_ENET_RECV>);
template READ32_MEMBER(hpc3_device::fifo_r<hpc3_device::FIFO_ENET_XMIT>);
template WRITE32_MEMBER(hpc3_device::fifo_w<hpc3_device::FIFO_PBUS>);
template WRITE32_MEMBER(hpc3_device::fifo_w<hpc3_device::FIFO_SCSI0>);
template WRITE32_MEMBER(hpc3_device::fifo_w<hpc3_device::FIFO_SCSI1>);
template WRITE32_MEMBER(hpc3_device::fifo_w<hpc3_device::FIFO_ENET_RECV>);
template WRITE32_MEMBER(hpc3_device::fifo_w<hpc3_device::FIFO_ENET_XMIT>);

template<uint32_t index>
READ32_MEMBER(hpc3_device::hd_r)
{
	if (ACCESSING_BITS_0_7 && !m_hd_rd_cb[index].isnull())
	{
		const uint8_t ret = m_hd_rd_cb[index](offset);
		LOGMASKED(LOG_SCSI, "%s: SCSI%d Read %02x: %02x\n", machine().describe_context(), index, offset, ret);
		return ret;
	}
	else
	{
		LOGMASKED(LOG_SCSI | LOG_UNKNOWN, "%s: Unknown HPC3 HD%d Read: %08x & %08x\n", machine().describe_context(),
			index, 0x1fbc4000 + (offset << 2) + index * 0x8000, mem_mask);
		return 0;
	}
}

template<uint32_t index>
WRITE32_MEMBER(hpc3_device::hd_w)
{
	if (ACCESSING_BITS_0_7 && !m_hd_wr_cb[index].isnull())
	{
		LOGMASKED(LOG_SCSI, "%s: SCSI%d Write %02x = %02x\n", machine().describe_context(), index, offset, (uint8_t)data);
		m_hd_wr_cb[index](offset, data & 0xff);
	}
	else
	{
		LOGMASKED(LOG_SCSI | LOG_UNKNOWN, "%s: Unknown HPC3 HD%d Write: %08x = %08x & %08x\n", machine().describe_context(),
			index, 0x1fbc4000 + (offset << 2) + index * 0x8000, data, mem_mask);
	}
}

template READ32_MEMBER(hpc3_device::hd_r<0>);
template READ32_MEMBER(hpc3_device::hd_r<1>);
template WRITE32_MEMBER(hpc3_device::hd_w<0>);
template WRITE32_MEMBER(hpc3_device::hd_w<1>);

uint32_t hpc3_device::pio_data_r(offs_t offset)
{
	uint32_t channel = (offset >> 8) & 15;
	if (channel >= 10)
	{
		channel = (channel & 1) ? 9 : 8;
	}

	switch ((m_pio_config[channel] >> 18) & 3)
	{
	default:
	case 0: // 8-bit, data on PBUS 7:0
		return m_pio_space[channel]->read_word(offset & 0xff, 0x00ff) & 0xff;

	case 2: // 8-bit, data on PBUS 15:8
		return m_pio_space[channel]->read_word(offset & 0xff, 0xff00) >> 8;

	case 1: // 16-bit, odd high
	case 3: // 16-bit, even high
		return m_pio_space[channel]->read_word(offset & 0xff, 0xffff);
	}
}

void hpc3_device::pio_data_w(offs_t offset, uint32_t data)
{
	uint32_t channel = (offset >> 8) & 15;
	if (channel >= 10)
	{
		channel = (channel & 1) ? 9 : 8;
	}

	switch ((m_pio_config[channel] >> 18) & 3)
	{
	case 0: // 8-bit, data on PBUS 7:0
		m_pio_space[channel]->write_word(offset & 0xff, data & 0xffff, 0x00ff);
		break;

	case 2: // 8-bit, data on PBUS 15:8
		m_pio_space[channel]->write_word(offset & 0xff, swapendian_int16(data & 0xffff), 0xff00);
		break;

	case 1: // 16-bit, odd high
	case 3: // 16-bit, even high
		m_pio_space[channel]->write_word(offset & 0xff, data & 0xffff, 0xffff);
		break;
	}
}

READ32_MEMBER(hpc3_device::pbusdma_r)
{
	uint32_t channel = offset / (0x2000/4);
	pbus_dma_t &dma = m_pbus_dma[channel];

	uint32_t ret = 0;
	switch (offset & 0x07ff)
	{
	case 0x0000/4:
		ret = dma.m_cur_ptr;
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Buffer Pointer Read: %08x & %08x\n", machine().describe_context(), channel, ret, mem_mask);
		break;
	case 0x0004/4:
		ret = dma.m_desc_ptr;
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Descriptor Pointer Read: %08x & %08x\n", machine().describe_context(), channel, ret, mem_mask);
		break;
	case 0x1000/4:
		ret = (dma.m_timer->remaining() != attotime::never) ? 2 : 0;
		if (BIT(m_intstat, channel))
		{
			ret |= 1;
			if (!machine().side_effects_disabled())
			{
				LOGMASKED(LOG_PBUS_DMA, "Lowering channel %d IRQ\n", channel);
				m_intstat &= ~(1 << channel);
				if (m_intstat == 0)
					m_dma_complete_int_cb(0);
			}
		}
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Control Read: %08x & %08x\n", machine().describe_context(), channel, ret, mem_mask);
		break;
	default:
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Unknown Read: %08x & %08x\n", machine().describe_context(), channel, 0x1fb80000 + (offset << 2), mem_mask);
		break;
	}
	return ret;
}

WRITE32_MEMBER(hpc3_device::pbusdma_w)
{
	uint32_t channel = offset / (0x2000/4);
	pbus_dma_t &dma = m_pbus_dma[channel];

	switch (offset & 0x07ff)
	{
	case 0x0004/4:
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS DMA Channel %d Descriptor Pointer Write: %08x\n", machine().describe_context(), channel, data);
		dma.m_desc_ptr = data;
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS_DMA_DescPtr = %08x\n", machine().describe_context(), dma.m_desc_ptr);
		dma.m_cur_ptr = space.read_dword(dma.m_desc_ptr);
		dma.m_desc_flags = space.read_dword(dma.m_desc_ptr + 4);
		dma.m_next_ptr = space.read_dword(dma.m_desc_ptr + 8);
		dma.m_bytes_left = dma.m_desc_flags & 0x3fff;
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS_DMA_CurPtr = %08x\n", machine().describe_context(), dma.m_cur_ptr);
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS_DMA_BytesLeft = %08x\n", machine().describe_context(), dma.m_bytes_left);
		LOGMASKED(LOG_PBUS_DMA, "%s: PBUS_DMA_NextPtr = %08x\n", machine().describe_context(), dma.m_next_ptr);
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

		if (((data & PBUS_CTRL_DMASTART) && (data & PBUS_CTRL_LOAD_EN)) && channel < 4)
		{
			LOGMASKED(LOG_PBUS_DMA, "    Starting DMA\n");
			attotime rate = m_hal2->get_rate(channel);
			if (rate != attotime::zero)
			{
				dma.m_timer->adjust(rate);
				dma.m_active = true;
			}
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
	LOGMASKED(LOG_PBUS_DMA, "    Device Bit Width: %d\n", BIT(data, 18) ? 16 : 8);
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
	LOGMASKED(LOG_PBUS_DMA, "    Device Bit Width: %d\n", BIT(data, 18) ? 16 : 8);
	LOGMASKED(LOG_PBUS_DMA, "    Even Address Bytes on %s\n", BIT(data, 19) ? "15..8" : "7..0");
}

uint32_t hpc3_device::bbram_r(offs_t offset)
{
	return m_bbram_rd_cb(offset);
}

void hpc3_device::bbram_w(offs_t offset, uint32_t data)
{
	m_bbram_wr_cb(offset, data);
}

void hpc3_device::dump_chain(uint32_t base)
{
	const uint32_t addr = m_gio64_space->read_dword(base);
	const uint32_t ctrl = m_gio64_space->read_dword(base+4);
	const uint32_t next = m_gio64_space->read_dword(base+8);

	LOGMASKED(LOG_CHAIN, "Chain Node:\n");
	LOGMASKED(LOG_CHAIN, "    Addr: %08x\n", addr);
	LOGMASKED(LOG_CHAIN, "    Ctrl: %08x\n", ctrl);
	LOGMASKED(LOG_CHAIN, "    Next: %08x\n", next);

	if (next != 0 && !BIT(ctrl, 31))
	{
		dump_chain(next);
	}
}

void hpc3_device::fetch_chain(int channel)
{
	scsi_dma_t &dma = m_scsi_dma[channel];
	const uint32_t desc_addr = dma.m_nbdp;
	dma.m_cbp = m_gio64_space->read_dword(desc_addr);
	dma.m_bc = m_gio64_space->read_dword(desc_addr+4);
	dma.m_nbdp = m_gio64_space->read_dword(desc_addr+8);
	dma.m_count = dma.m_bc & 0x3fff;

	LOGMASKED(LOG_CHAIN, "Fetching chain from %08x:\n", desc_addr);
	LOGMASKED(LOG_CHAIN, "    Addr: %08x\n", dma.m_cbp);
	LOGMASKED(LOG_CHAIN, "    Ctrl: %08x\n", dma.m_bc);
	LOGMASKED(LOG_CHAIN, "    Next: %08x\n", dma.m_nbdp);
}

void hpc3_device::decrement_chain(int channel)
{
	scsi_dma_t &dma = m_scsi_dma[channel];
	dma.m_count--;
	if (dma.m_count == 0)
	{
		if (BIT(dma.m_bc, 29))
		{
			LOGMASKED(LOG_SCSI_IRQ, "Raising SCSI %d IRQ\n", channel);
			m_intstat |= 0x100 << channel;
			m_dma_complete_int_cb(1);
		}
		if (BIT(dma.m_bc, 31))
		{
			dma.m_active = false;
			dma.m_ctrl &= ~HPC3_DMACTRL_ENABLE;
			return;
		}
		fetch_chain(channel);
	}
}

void hpc3_device::scsi_fifo_flush(int channel)
{
	scsi_dma_t &dma = m_scsi_dma[channel];

	LOGMASKED(LOG_SCSI_DMA, "Flushing SCSI %d FIFO\n", channel);

	if (BIT(dma.m_bc, 29))
	{
		LOGMASKED(LOG_SCSI_IRQ, "Raising SCSI %d IRQ\n", channel);
		m_intstat |= 0x100 << channel;
		m_dma_complete_int_cb(1);
	}

	dma.m_active = false;
	dma.m_ctrl &= ~(HPC3_DMACTRL_ENABLE | HPC3_DMACTRL_FLUSH);
}

void hpc3_device::scsi_drq(bool state, int channel)
{
	scsi_dma_t &dma = m_scsi_dma[channel];
	dma.m_drq = state;

	if (dma.m_drq && dma.m_active)
	{
		do_scsi_dma(channel);
	}
}

void hpc3_device::do_scsi_dma(int channel)
{
	scsi_dma_t &dma = m_scsi_dma[channel];

	const uint32_t addr = dma.m_big_endian ? BYTE4_XOR_BE(dma.m_cbp) : BYTE4_XOR_LE(dma.m_cbp);
	if (dma.m_to_device)
		m_hd_dma_wr_cb[channel](m_gio64_space->read_byte(addr));
	else
		m_gio64_space->write_byte(addr, m_hd_dma_rd_cb[channel]());

	dma.m_cbp++;
	decrement_chain(channel);

	if (!dma.m_active)
	{
		// clear HPC3 DMA active flag
		dma.m_ctrl &= ~HPC3_DMACTRL_ENABLE;
	}
}

WRITE_LINE_MEMBER(hpc3_device::scsi0_drq)
{
	scsi_drq(state, 0);
}

WRITE_LINE_MEMBER(hpc3_device::scsi1_drq)
{
	scsi_drq(state, 1);
}

READ32_MEMBER(hpc3_device::intstat_r)
{
	return m_intstat;
}

uint32_t hpc3_device::misc_r()
{
	return m_misc;
}

void hpc3_device::misc_w(uint32_t data)
{
	LOGMASKED(LOG_PBUS_DMA, "%s: Write miscellaneous register: %08x\n", machine().describe_context(), data);
	LOGMASKED(LOG_PBUS_DMA, "    Real time devices %sabled\n", BIT(data, 0) ? "en" : "dis");
	LOGMASKED(LOG_PBUS_DMA, "    DMA descriptors are %s endian\n", BIT(data, 1) ? "little" : "big");
	m_misc = data & 3;
}

uint32_t hpc3_device::eeprom_r()
{
	uint32_t ret = (m_cpu_aux_ctrl & ~0x10) | (m_eeprom_dati_cb() << 4);
	LOGMASKED(LOG_EEPROM, "%s: HPC Serial EEPROM Read: %08x\n", machine().describe_context(), ret);
	return ret;
}

void hpc3_device::eeprom_w(uint32_t data)
{
	m_cpu_aux_ctrl = data;
	LOGMASKED(LOG_EEPROM, "%s: HPC Serial EEPROM Write: %08x\n", machine().describe_context(), data);
	m_eeprom_pre_cb(BIT(data, 0));
	m_eeprom_dato_cb(BIT(data, 3));
	m_eeprom_cs_cb(BIT(data, 1));
	m_eeprom_clk_cb(BIT(data, 2));
}
