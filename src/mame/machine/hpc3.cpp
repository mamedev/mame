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
#define LOG_PBUS4       (1 << 6)
#define LOG_CHAIN       (1 << 7)
#define LOG_ALL         (LOG_UNKNOWN | LOG_PBUS_DMA | LOG_SCSI | LOG_SCSI_DMA | LOG_SCSI_IRQ | LOG_ETHERNET | LOG_PBUS4 | LOG_CHAIN)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_HPC3, hpc3_device, "hpc3", "SGI HPC3")

hpc3_device::hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HPC3, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_wd33c93(*this, finder_base::DUMMY_TAG)
	, m_wd33c93_2(*this, finder_base::DUMMY_TAG)
	, m_eeprom(*this, "eeprom")
	, m_rtc(*this, "rtc")
	, m_ioc2(*this, "ioc2")
	, m_hal2(*this, "hal2")
	, m_ldac(*this, "ldac")
	, m_rdac(*this, "rdac")
{
}

void hpc3_device::device_start()
{
	save_item(NAME(m_enetr_nbdp));
	save_item(NAME(m_enetr_cbp));
	save_item(NAME(m_cpu_aux_ctrl));
	save_item(NAME(m_pio_config));

	for (uint32_t i = 0; i < 2; i++)
	{
		save_item(NAME(m_scsi_dma[i].m_desc), i);
		save_item(NAME(m_scsi_dma[i].m_addr), i);
		save_item(NAME(m_scsi_dma[i].m_ctrl), i);
		save_item(NAME(m_scsi_dma[i].m_length), i);
		save_item(NAME(m_scsi_dma[i].m_next), i);
		save_item(NAME(m_scsi_dma[i].m_irq), i);
		save_item(NAME(m_scsi_dma[i].m_drq), i);
		save_item(NAME(m_scsi_dma[i].m_big_endian), i);
		save_item(NAME(m_scsi_dma[i].m_to_device), i);
		save_item(NAME(m_scsi_dma[i].m_active), i);
	}

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
}

void hpc3_device::device_reset()
{
	m_enetr_nbdp = 0x80000000;
	m_enetr_cbp = 0x80000000;
	m_cpu_aux_ctrl = 0;

	memset(m_scsi_dma, 0, sizeof(scsi_dma_t) * 2);

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

	m_cpu_space = &m_maincpu->space(AS_PROGRAM);
}

void hpc3_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0);
	m_ldac->add_route(ALL_OUTPUTS, "lspeaker", 0.25);

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0);
	m_rdac->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	voltage_regulator_device &vreg = VOLTAGE_REGULATOR(config, "vref");
	vreg.add_route(0, "ldac",  1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, "rdac",  1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, "ldac", -1.0, DAC_VREF_NEG_INPUT);
	vreg.add_route(0, "rdac", -1.0, DAC_VREF_NEG_INPUT);

	SGI_HAL2(config, m_hal2);
	SGI_IOC2_GUINNESS(config, m_ioc2, m_maincpu);

	DS1386_8K(config, m_rtc, 32768);

	EEPROM_93C56_16BIT(config, m_eeprom);
}

void hpc3_device::map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rw(FUNC(hpc3_device::pbusdma_r), FUNC(hpc3_device::pbusdma_w));
	map(0x00010000, 0x0001ffff).rw(FUNC(hpc3_device::hd_enet_r), FUNC(hpc3_device::hd_enet_w));
	map(0x00030008, 0x0003000b).rw(FUNC(hpc3_device::eeprom_r), FUNC(hpc3_device::eeprom_w));
	map(0x00040000, 0x00047fff).rw(FUNC(hpc3_device::hd_r<0>), FUNC(hpc3_device::hd_w<0>));
	map(0x00048000, 0x0004ffff).rw(FUNC(hpc3_device::hd_r<1>), FUNC(hpc3_device::hd_w<1>));
	map(0x00054000, 0x000544ff).rw(FUNC(hpc3_device::enet_r), FUNC(hpc3_device::enet_w));
	map(0x00058000, 0x000583ff).rw(m_hal2, FUNC(hal2_device::read), FUNC(hal2_device::write));
	map(0x00058400, 0x000587ff).ram(); // hack
	map(0x00059000, 0x000593ff).rw(FUNC(hpc3_device::pbus4_r), FUNC(hpc3_device::pbus4_w));
	map(0x00059800, 0x00059bff).rw(m_ioc2, FUNC(ioc2_device::read), FUNC(ioc2_device::write));
	map(0x0005c000, 0x0005cfff).rw(FUNC(hpc3_device::dma_config_r), FUNC(hpc3_device::dma_config_w));
	map(0x0005d000, 0x0005dfff).rw(FUNC(hpc3_device::pio_config_r), FUNC(hpc3_device::pio_config_w));
	map(0x00060000, 0x000604ff).rw(m_rtc, FUNC(ds1386_device::data_r), FUNC(ds1386_device::data_w)).umask32(0x000000ff);
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
		uint16_t temp16 = m_cpu_space->read_dword(dma.m_cur_ptr) >> 16;
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
				dma.m_cur_ptr = m_cpu_space->read_dword(dma.m_desc_ptr);
				dma.m_desc_flags = m_cpu_space->read_dword(dma.m_desc_ptr + 4);
				dma.m_bytes_left = dma.m_desc_flags & 0x7fffffff;
				dma.m_next_ptr = m_cpu_space->read_dword(dma.m_desc_ptr + 8);
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

READ32_MEMBER(hpc3_device::enet_r)
{
	switch (offset)
	{
		case 0x000/4:
			logerror("%s: HPC3: enet_r: Read MAC Address bytes 0-3, 0x80675309 & %08x\n", machine().describe_context(), mem_mask);
			return 0x80675309;
		default:
			logerror("%s: HPC3: enet_r: Read Unknown Register %08x & %08x\n", machine().describe_context(), 0x1fbd4000 + (offset << 2), mem_mask);
			return 0;
	}
}

WRITE32_MEMBER(hpc3_device::enet_w)
{
	switch (offset)
	{
		default:
			logerror("%s: HPC3: enet_w: Write Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x1fbd4000 + (offset << 2), data, mem_mask);
			break;
	}
}

READ32_MEMBER(hpc3_device::hd_enet_r)
{
	switch (offset)
	{
	case 0x0004/4:
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI0 Desc Address Read: %08x & %08x\n", machine().describe_context(), m_scsi_dma[0].m_desc, mem_mask);
		return m_scsi_dma[0].m_desc;
	case 0x1004/4:
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI0 DMA Control Read: %08x & %08x\n", machine().describe_context(), m_scsi_dma[0].m_ctrl, mem_mask);
		return m_scsi_dma[0].m_ctrl;
	case 0x4000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet CBP Read: %08x & %08x\n", machine().describe_context(), m_enetr_nbdp, mem_mask);
		return m_enetr_cbp;
	case 0x4004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet NBDP Read: %08x & %08x\n", machine().describe_context(), m_enetr_nbdp, mem_mask);
		return m_enetr_nbdp;
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
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI0 Desc Address Write: %08x\n", machine().describe_context(), data);
		m_scsi_dma[0].m_desc = data;
		fetch_chain(0);
		break;
	case 0x1004/4:
		LOGMASKED(LOG_SCSI, "%s: HPC3 SCSI0 DMA Control Write: %08x\n", machine().describe_context(), data);
		m_scsi_dma[0].m_ctrl = data;
		m_scsi_dma[0].m_to_device = (m_scsi_dma[0].m_ctrl & HPC3_DMACTRL_DIR);
		m_scsi_dma[0].m_big_endian = (m_scsi_dma[0].m_ctrl & HPC3_DMACTRL_ENDIAN);
		m_scsi_dma[0].m_active = (m_scsi_dma[0].m_ctrl & HPC3_DMACTRL_ENABLE);
		m_scsi_dma[0].m_irq = (m_scsi_dma[0].m_ctrl & HPC3_DMACTRL_IRQ);
		m_wd33c93->reset_w(BIT(data, 6));
		if (m_scsi_dma[0].m_drq && m_scsi_dma[0].m_active)
		{
			do_scsi_dma(0);
		}
		break;
	case 0x4000/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet CBP Write: %08x\n", machine().describe_context(), data);
		m_enetr_cbp = data;
		break;
	case 0x4004/4:
		LOGMASKED(LOG_ETHERNET, "%s: HPC3 Ethernet NBDP Write: %08x\n", machine().describe_context(), data);
		m_enetr_nbdp = data;
		break;
	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown HPC3 ENET/HDx write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fb90000 + (offset << 2), data, mem_mask);
		break;
	}
}

template<uint32_t index>
READ32_MEMBER(hpc3_device::hd_r)
{
	if (index && !m_wd33c93_2)
		return 0;

	switch (offset)
	{
	case 0x0000/4:
	case 0x4000/4:
		if (ACCESSING_BITS_0_7)
		{
			const uint8_t ret = index ? m_wd33c93_2->indir_addr_r() : m_wd33c93->indir_addr_r();
			LOGMASKED(LOG_SCSI, "%s: SCSI%d Read 0: %02x\n", machine().describe_context(), index, ret);
			return ret;
		}
		break;
	case 0x0004/4:
	case 0x4004/4:
		if (ACCESSING_BITS_0_7)
		{
			const uint8_t ret = index ? m_wd33c93_2->indir_reg_r() : m_wd33c93->indir_reg_r();
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
	if (index && !m_wd33c93_2)
		return;

	switch (offset)
	{
	case 0x0000:
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_SCSI, "%s: SCSI%d Write 0 = %02x\n", machine().describe_context(), index, (uint8_t)data);
			index ? m_wd33c93_2->indir_addr_w(data & 0xff) : m_wd33c93->indir_addr_w(data & 0xff);
		}
		break;
	case 0x0001:
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_SCSI, "%s: SCSI%d Write 1 = %02x\n", machine().describe_context(), index, (uint8_t)data);
			index ? m_wd33c93_2->indir_reg_w(data & 0xff) : m_wd33c93->indir_reg_w(data & 0xff);
		}
		break;
	default:
		LOGMASKED(LOG_SCSI | LOG_UNKNOWN, "%s: %s: Unknown HPC3 HD%d Write: %08x = %08x & %08x\n", machine().describe_context(), machine().describe_context(),
			index, 0x1fbc4000 + (offset << 2) + index * 0x8000, data, mem_mask);
		break;
	}
}

template READ32_MEMBER(hpc3_device::hd_r<0>);
template READ32_MEMBER(hpc3_device::hd_r<1>);
template WRITE32_MEMBER(hpc3_device::hd_w<0>);
template WRITE32_MEMBER(hpc3_device::hd_w<1>);

READ32_MEMBER(hpc3_device::pbus4_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
	case 0x0000/4:
		ret = m_ioc2->get_local_int_status(0);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Local0 Interrupt Status Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x0004/4:
		ret = m_ioc2->get_local_int_mask(0);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Local0 Interrupt Mask Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x0008/4:
		ret = m_ioc2->get_local_int_status(1);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Local1 Interrupt Status Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x000c/4:
		ret = m_ioc2->get_local_int_mask(1);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Local1 Interrupt Mask Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x0010/4:
		ret = m_ioc2->get_map_int_status();
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Mappable Interrupt Status: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x0014/4:
		ret = m_ioc2->get_map_int_mask(0);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Mapped Interrupt 0 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x0018/4:
		ret = m_ioc2->get_map_int_mask(1);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Mapped Interrupt 1 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x0030/4:
		ret = m_ioc2->get_pit_reg(0);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PIT Counter 0 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x0034/4:
		ret = m_ioc2->get_pit_reg(1);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PIT Counter 1 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x0038/4:
		ret = m_ioc2->get_pit_reg(2);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PIT Counter 2 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	case 0x003c/4:
		ret = m_ioc2->get_pit_reg(3);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PIT Control Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		break;
	default:
		LOGMASKED(LOG_PBUS4 | LOG_UNKNOWN, "%s: Unknown HPC3 PBUS4 Read: %08x (%08x)\n", machine().describe_context(), 0x1fbd9000 + (offset << 2), mem_mask);
		break;
	}
	return ret;
}

WRITE32_MEMBER(hpc3_device::pbus4_w)
{
	switch (offset)
	{
	case 0x0004/4:
		m_ioc2->set_local_int_mask(0, data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Local0 Interrupt Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x000c/4:
		m_ioc2->set_local_int_mask(1, data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Local1 Interrupt Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0014/4:
		m_ioc2->set_map_int_mask(0, data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Mapped Interrupt 0 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0018/4:
		m_ioc2->set_map_int_mask(1, data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Mapped Interrupt 1 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0020/4:
		m_ioc2->set_timer_int_clear(data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 INT3 Timer Interrupt Clear Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0030/4:
		m_ioc2->set_pit_reg(0, (uint8_t)data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PIT Counter 0 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0034/4:
		m_ioc2->set_pit_reg(1, (uint8_t)data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PIT Counter 1 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0038/4:
		m_ioc2->set_pit_reg(2, (uint8_t)data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PIT Counter 2 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x003c/4:
		m_ioc2->set_pit_reg(3, (uint8_t)data);
		LOGMASKED(LOG_PBUS4, "%s: HPC3 PIT Control Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
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
		dma.m_bytes_left = dma.m_desc_flags & 0x7fffffff;
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

void hpc3_device::dump_chain(uint32_t base)
{
	const uint32_t addr = m_cpu_space->read_dword(base);
	const uint32_t ctrl = m_cpu_space->read_dword(base+4);
	const uint32_t next = m_cpu_space->read_dword(base+8);

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
	dma.m_addr = m_cpu_space->read_dword(dma.m_desc);
	dma.m_ctrl = m_cpu_space->read_dword(dma.m_desc+4);
	dma.m_next = m_cpu_space->read_dword(dma.m_desc+8);
	dma.m_length = dma.m_ctrl & 0x3fff;

	LOGMASKED(LOG_CHAIN, "Fetching chain from %08x:\n", dma.m_desc);
	LOGMASKED(LOG_CHAIN, "    Addr: %08x\n", dma.m_addr);
	LOGMASKED(LOG_CHAIN, "    Ctrl: %08x\n", dma.m_ctrl);
	LOGMASKED(LOG_CHAIN, "    Next: %08x\n", dma.m_next);
}

void hpc3_device::decrement_chain(int channel)
{
	scsi_dma_t &dma = m_scsi_dma[channel];
	dma.m_length--;
	if (dma.m_length == 0)
	{
		if (BIT(dma.m_ctrl, 31))
		{
			dma.m_active = false;
			dma.m_ctrl &= ~HPC3_DMACTRL_ENABLE;
			return;
		}
		dma.m_desc = dma.m_next;
		fetch_chain(channel);
	}
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

	if (dma.m_to_device)
		m_wd33c93->dma_w(m_cpu_space->read_byte(dma.m_big_endian ? BYTE4_XOR_BE(dma.m_addr) : BYTE4_XOR_LE(dma.m_addr)));
	else
		m_cpu_space->write_byte(dma.m_big_endian ? BYTE4_XOR_BE(dma.m_addr) : BYTE4_XOR_LE(dma.m_addr), m_wd33c93->dma_r());

	dma.m_addr++;
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

#if 0
void hpc3_device::scsi_dma(int channel)
{
	int byte_count = channel ? m_wd33c93_2->get_dma_count() : m_wd33c93->get_dma_count();
	scsi_dma_t &dma = m_scsi_dma[channel];

	LOGMASKED(LOG_SCSI_DMA, "HPC3: Transferring %d bytes %s %08x %s SCSI0\n",
		byte_count, dma.m_to_device ? "from" : "to", dma.m_addr, dma.m_to_device ? "to" : "from");

	if (dma.m_irq)
		LOGMASKED(LOG_SCSI_DMA, "HPC3: Not yet implemented: SCSI DMA IRQ\n");

	uint8_t dma_buffer[512];
	memset(dma_buffer, 0, 512);
	if (dma.m_to_device)
	{
		// HPC3 DMA: host to device
		if (byte_count <= 512)
		{
			for (int i = 0; i < byte_count; i++)
			{
				dma_buffer[dma.m_big_endian ? BYTE4_XOR_BE(i) : BYTE4_XOR_LE(i)] = m_cpu_space->read_byte(dma.m_addr);
				dma.m_addr++;
				decrement_chain(channel);
				if (!dma.m_active)
					break;
			}

			if (channel)
				m_wd33c93_2->dma_write_data(byte_count, dma_buffer);
			else
				m_wd33c93->dma_write_data(byte_count, dma_buffer);
		}
		else
		{
			while (byte_count)
			{
				int sub_count = std::min(512, byte_count);

				for (int i = 0; i < sub_count; i++)
				{
					dma_buffer[dma.m_big_endian ? BYTE4_XOR_BE(i) : BYTE4_XOR_LE(i)] = m_cpu_space->read_byte(dma.m_addr);
					dma.m_addr++;
					decrement_chain(channel);
					if (!dma.m_active)
						break;
				}

				if (channel)
					m_wd33c93_2->dma_write_data(sub_count, dma_buffer);
				else
					m_wd33c93->dma_write_data(sub_count, dma_buffer);

				if (!dma.m_active)
				{
					break;
				}
				else
				{
					memset(dma_buffer, 0, sub_count);
					byte_count -= sub_count;
				}
			}
		}
	}
	else
	{
		// HPC3 DMA: device to host
		if (byte_count <= 512)
		{
			if (channel)
				m_wd33c93_2->dma_read_data(byte_count, dma_buffer);
			else
				m_wd33c93->dma_read_data(byte_count, dma_buffer);

			for (int i = 0; i < byte_count; i++)
			{
				m_cpu_space->write_byte(dma.m_big_endian ? BYTE4_XOR_BE(dma.m_addr) : BYTE4_XOR_LE(dma.m_addr), dma_buffer[i]);
				dma.m_addr++;
				decrement_chain(channel);
				if (!dma.m_active)
					break;
			}
		}
		else
		{
			while (byte_count)
			{
				int sub_count;
				if (channel)
					sub_count = m_wd33c93_2->dma_read_data(512, dma_buffer);
				else
					sub_count = m_wd33c93->dma_read_data(512, dma_buffer);

				for (int i = 0; i < sub_count; i++)
				{
					m_cpu_space->write_byte(dma.m_big_endian ? BYTE4_XOR_BE(dma.m_addr) : BYTE4_XOR_LE(dma.m_addr), dma_buffer[i]);
					dma.m_addr++;
					decrement_chain(channel);
					if (!dma.m_active)
						break;
				}

				if (!dma.m_active)
					break;
				else
					byte_count -= sub_count;
			}
		}
	}

	// clear DMA on the controller
	m_wd33c93->clear_dma();
}
#endif

WRITE_LINE_MEMBER(hpc3_device::scsi0_irq)
{
	if (state)
	{
		LOGMASKED(LOG_SCSI_IRQ, "Raising SCSI 0 IRQ\n");
		//if (m_wd33c93->get_dma_count() && m_scsi_dma[0].m_active)
		//  scsi_dma(0);

		m_ioc2->raise_local_irq(0, ioc2_device::INT3_LOCAL0_SCSI0);
	}
	else
	{
		LOGMASKED(LOG_SCSI_IRQ, "Lowering SCSI 0 IRQ\n");
		m_ioc2->lower_local_irq(0, ioc2_device::INT3_LOCAL0_SCSI0);
	}
}

WRITE_LINE_MEMBER(hpc3_device::scsi1_irq)
{
	if (state)
	{
		LOGMASKED(LOG_SCSI_IRQ, "Raising SCSI 1 IRQ\n");

		//if (m_wd33c93_2->get_dma_count() && m_scsi_dma[1].m_active)
		//  scsi_dma(1);

		m_ioc2->raise_local_irq(0, ioc2_device::INT3_LOCAL0_SCSI1);
	}
	else
	{
		LOGMASKED(LOG_SCSI_IRQ, "Lowering SCSI 0 IRQ\n");

		m_ioc2->lower_local_irq(0, ioc2_device::INT3_LOCAL0_SCSI1);
	}
}

READ32_MEMBER(hpc3_device::eeprom_r)
{
	// Disabled - we don't have a dump from real hardware, and IRIX 5.x freaks out with default contents.
	uint32_t ret = (m_cpu_aux_ctrl & ~0x10);// | m_eeprom->do_read() << 4;
	logerror("%s: HPC Serial EEPROM Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
	return ret;
}

WRITE32_MEMBER(hpc3_device::eeprom_w)
{
	m_cpu_aux_ctrl = data;
	logerror("%s: HPC Serial EEPROM Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_eeprom->di_write(BIT(data, 3));
	m_eeprom->cs_write(BIT(data, 1));
	m_eeprom->clk_write(BIT(data, 2));
}
