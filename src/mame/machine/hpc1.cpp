// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC1 "High-performance Peripheral Controller" emulation

**********************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/hlemouse.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/sgikbd/sgikbd.h"
#include "machine/hpc1.h"
#include "speaker.h"

#define LOG_UNKNOWN     (1 << 0)
#define LOG_READS       (1 << 1)
#define LOG_WRITES      (1 << 2)
#define LOG_INT         (1 << 3)
#define LOG_EEPROM      (1 << 4)
#define LOG_SCSI        (1 << 5)
#define LOG_SCSI_DMA    (1 << 6)
#define LOG_DUART0      (1 << 7)
#define LOG_DUART1      (1 << 8)
#define LOG_DUART2      (1 << 9)
#define LOG_PIT         (1 << 10)
#define LOG_CHAIN       (1 << 11)
#define LOG_REGS        (LOG_UNKNOWN | LOG_READS | LOG_WRITES)
#define LOG_DUART       (LOG_DUART0 | LOG_DUART1 | LOG_DUART2)
#define LOG_ALL         (LOG_REGS | LOG_INT | LOG_EEPROM | LOG_SCSI | LOG_SCSI_DMA | LOG_DUART | LOG_PIT | LOG_CHAIN)

#define VERBOSE         (LOG_UNKNOWN)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_HPC1, hpc1_device, "hpc1", "SGI HPC1")

/*static*/ char const *const hpc1_device::RS232A_TAG = "rs232a";
/*static*/ char const *const hpc1_device::RS232B_TAG = "rs232b";

/*static*/ const XTAL hpc1_device::SCC_PCLK = 10_MHz_XTAL;
/*static*/ const XTAL hpc1_device::SCC_RXA_CLK = 3.6864_MHz_XTAL; // Needs verification
/*static*/ const XTAL hpc1_device::SCC_TXA_CLK = XTAL(0);
/*static*/ const XTAL hpc1_device::SCC_RXB_CLK = 3.6864_MHz_XTAL; // Needs verification
/*static*/ const XTAL hpc1_device::SCC_TXB_CLK = XTAL(0);

hpc1_device::hpc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HPC1, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_eeprom(*this, finder_base::DUMMY_TAG)
	, m_wd33c93(*this, "scsibus:0:wd33c93")
	, m_scc(*this, "scc%u", 0U)
	, m_pit(*this, "pit")
	, m_rtc(*this, "rtc")
{
}

void hpc1_device::device_start()
{
	save_item(NAME(m_misc_status));
	save_item(NAME(m_cpu_aux_ctrl));
	save_item(NAME(m_parbuf_ptr));
	save_item(NAME(m_local_int_status));
	save_item(NAME(m_local_int_mask));
	save_item(NAME(m_int_status));
	save_item(NAME(m_vme_int_mask));

	save_item(NAME(m_scsi_dma.m_desc));
	save_item(NAME(m_scsi_dma.m_addr));
	save_item(NAME(m_scsi_dma.m_ctrl));
	save_item(NAME(m_scsi_dma.m_length));
	save_item(NAME(m_scsi_dma.m_next));
	save_item(NAME(m_scsi_dma.m_irq));
	save_item(NAME(m_scsi_dma.m_drq));
	save_item(NAME(m_scsi_dma.m_to_mem));
	save_item(NAME(m_scsi_dma.m_active));

	save_item(NAME(m_duart_int_status));
}

void hpc1_device::device_reset()
{
	m_misc_status = 0;
	m_cpu_aux_ctrl = 0;
	m_parbuf_ptr = 0;
	memset(m_local_int_status, 0, sizeof(uint32_t) * 2);
	memset(m_local_int_mask, 0, sizeof(uint32_t) * 2);
	memset(m_int_status, 0, sizeof(bool) * 2);
	memset(m_vme_int_mask, 0, sizeof(uint32_t) * 2);

	m_scsi_dma.m_desc = 0;
	m_scsi_dma.m_addr = 0;
	m_scsi_dma.m_ctrl = 0;
	m_scsi_dma.m_length = 0;
	m_scsi_dma.m_next = 0;
	m_scsi_dma.m_irq = false;
	m_scsi_dma.m_drq = false;
	m_scsi_dma.m_to_mem = false;
	m_scsi_dma.m_active = false;

	m_duart_int_status = 0;

	m_cpu_space = &m_maincpu->space(AS_PROGRAM);
}

//**************************************************************************
//  DEVICE HARDWARE
//**************************************************************************

void hpc1_device::cdrom_config(device_t *device)
{
	//  cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	//  cdda->add_route(ALL_OUTPUTS, "^^mono", 1.0);
}

void hpc1_device::indigo_mice(device_slot_interface &device)
{
	device.option_add("sgimouse", SGI_HLE_SERIAL_MOUSE);
}

void hpc1_device::scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_SGI);
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void hpc1_device::wd33c93(device_t *device)
{
	device->set_clock(10000000);
	downcast<wd33c93_device *>(device)->irq_cb().set(*this, FUNC(hpc1_device::scsi_irq));
	downcast<wd33c93_device *>(device)->drq_cb().set(*this, FUNC(hpc1_device::scsi_drq));
}

void hpc1_device::device_add_mconfig(machine_config &config)
{
	SCC85C30(config, m_scc[0], SCC_PCLK);
	m_scc[0]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[0]->out_int_callback().set(FUNC(hpc1_device::duart0_int_w));
	m_scc[0]->out_txda_callback().set("keyboard", FUNC(sgi_keyboard_port_device::write_txd));

	SCC85C30(config, m_scc[1], SCC_PCLK);
	m_scc[1]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[1]->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtra_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsa_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtrb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_int_callback().set(FUNC(hpc1_device::duart1_int_w));

	SCC85C30(config, m_scc[2], SCC_PCLK);
	m_scc[2]->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc[2]->out_int_callback().set(FUNC(hpc1_device::duart2_int_w));

	SGIKBD_PORT(config, "keyboard", default_sgi_keyboard_devices, "hlekbd").rxd_handler().set(m_scc[0], FUNC(z80scc_device::rxa_w));

	rs232_port_device &mouseport(RS232_PORT(config, "mouseport", indigo_mice, "sgimouse"));
	mouseport.set_fixed(true);
	mouseport.rxd_handler().set(m_scc[0], FUNC(scc85c30_device::rxb_w));
	mouseport.cts_handler().set(m_scc[0], FUNC(scc85c30_device::ctsb_w));
	mouseport.dcd_handler().set(m_scc[0], FUNC(scc85c30_device::dcdb_w));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.cts_handler().set(m_scc[1], FUNC(scc85c30_device::ctsa_w));
	rs232a.dcd_handler().set(m_scc[1], FUNC(scc85c30_device::dcda_w));
	rs232a.rxd_handler().set(m_scc[1], FUNC(scc85c30_device::rxa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.cts_handler().set(m_scc[1], FUNC(scc85c30_device::ctsb_w));
	rs232b.dcd_handler().set(m_scc[1], FUNC(scc85c30_device::dcdb_w));
	rs232b.rxd_handler().set(m_scc[1], FUNC(scc85c30_device::rxb_w));

	NSCSI_BUS(config, "scsibus", 0);
	NSCSI_CONNECTOR(config, "scsibus:0").option_set("wd33c93", WD33C93)
		.machine_config([this](device_t *device) { wd33c93(device); });
	NSCSI_CONNECTOR(config, "scsibus:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsibus:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:4", scsi_devices, "cdrom", false);
	NSCSI_CONNECTOR(config, "scsibus:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:7", scsi_devices, nullptr, false);

	DP8573(config, m_rtc);

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(1000000);
	m_pit->set_clk<1>(1000000);
	m_pit->set_clk<2>(1000000);
	m_pit->out_handler<0>().set(FUNC(hpc1_device::timer0_int));
	m_pit->out_handler<1>().set(FUNC(hpc1_device::timer1_int));
	m_pit->out_handler<2>().set(FUNC(hpc1_device::timer2_int));

	SPEAKER(config, "mono").front_center();
}

//**************************************************************************
//  REGISTER ACCESS
//**************************************************************************

READ32_MEMBER(hpc1_device::read)
{
	if (offset >= 0x0e00/4 && offset <= 0x0e7c/4)
		return m_rtc->read(space, offset - 0xe00/4);

	switch (offset)
	{
	case 0x005c/4:
		LOGMASKED(LOG_UNKNOWN, "%s: HPC Unknown Read: %08x & %08x\n",
			machine().describe_context(), 0x1fb80000 + offset*4, mem_mask);
		return 0;
	case 0x0094/4:
		LOGMASKED(LOG_SCSI_DMA, "%s: HPC SCSI DMA Control Register Read: %08x & %08x\n", machine().describe_context(), m_scsi_dma.m_ctrl, mem_mask);
		return m_scsi_dma.m_ctrl;
	case 0x00ac/4:
		LOGMASKED(LOG_READS, "%s: HPC Parallel Buffer Pointer Read: %08x & %08x\n", machine().describe_context(), m_parbuf_ptr, mem_mask);
		return m_parbuf_ptr;
	case 0x00c0/4:
		LOGMASKED(LOG_READS, "%s: HPC Endianness Read: %08x & %08x\n", machine().describe_context(), 0x00000000, mem_mask);
		return 0x00000000;
	case 0x0120/4:
	{
		uint32_t ret = m_wd33c93->indir_addr_r() << 8;
		LOGMASKED(LOG_SCSI, "%s: HPC SCSI Offset 0 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		return ret;
	}
	case 0x0124/4:
	{
		uint32_t ret = m_wd33c93->indir_reg_r() << 8;
		LOGMASKED(LOG_SCSI, "%s: HPC SCSI Offset 1 Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		return ret;
	}
	case 0x01b0/4:
		LOGMASKED(LOG_READS, "%s: HPC Misc. Status Read: %08x & %08x\n", machine().describe_context(), m_misc_status, mem_mask);
		return m_misc_status;
	case 0x01bc/4:
	{
		uint32_t ret = (m_cpu_aux_ctrl & ~0x10) | m_eeprom->do_read() << 4;
		LOGMASKED(LOG_EEPROM, "%s: HPC Serial EEPROM Read: %08x & %08x\n", machine().describe_context(), ret, mem_mask);
		return ret;
	}
	case 0x01c0/4:
		LOGMASKED(LOG_INT, "%s: HPC Local Interrupt 0 Status Read: %08x & %08x\n", machine().describe_context(), m_local_int_status[0], mem_mask);
		return m_local_int_status[0];
	case 0x01c4/4:
		LOGMASKED(LOG_INT, "%s: HPC Local Interrupt 0 Mask Read: %08x & %08x\n", machine().describe_context(), m_local_int_mask[0], mem_mask);
		return m_local_int_mask[0];
	case 0x01c8/4:
		LOGMASKED(LOG_INT, "%s: HPC Local Interrupt 1 Status Read: %08x & %08x\n", machine().describe_context(), m_local_int_status[1], mem_mask);
		return m_local_int_status[1];
	case 0x01cc/4:
		LOGMASKED(LOG_INT, "%s: HPC Local Interrupt 1 Mask Read: %08x & %08x\n", machine().describe_context(), m_local_int_mask[1], mem_mask);
		return m_local_int_mask[1];
	case 0x01d4/4:
		LOGMASKED(LOG_INT, "%s: HPC VME Interrupt Mask 0 Read: %08x & %08x\n", machine().describe_context(), m_vme_int_mask[0], mem_mask);
		return m_vme_int_mask[0];
	case 0x01d8/4:
		LOGMASKED(LOG_INT, "%s: HPC VME Interrupt Mask 1 Read: %08x & %08x\n", machine().describe_context(), m_vme_int_mask[1], mem_mask);
		return m_vme_int_mask[1];
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
		uint32_t ret = m_scc[index]->ab_dc_r(0);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel B Control Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d04/4:
	case 0x0d14/4:
	case 0x0d24/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ab_dc_r(1);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel B Data Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d08/4:
	case 0x0d18/4:
	case 0x0d28/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ab_dc_r(2);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel A Control Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	case 0x0d0c/4:
	case 0x0d1c/4:
	case 0x0d2c/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		const uint32_t ret = m_scc[index]->ab_dc_r(3);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel A Data Read: %08x & %08x\n", machine().describe_context(), index, ret, mem_mask);
		return ret;
	}
	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown HPC Read: %08x & %08x\n", machine().describe_context(), 0x1fb80000 + offset*4, mem_mask);
		return 0;
	}
	return 0;
}

WRITE32_MEMBER(hpc1_device::write)
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
		m_scsi_dma.m_desc = data;
		fetch_chain();
		break;

	case 0x0094/4:
		LOGMASKED(LOG_SCSI_DMA, "%s: HPC SCSI DMA Control Register Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_scsi_dma.m_ctrl = data &~ (HPC_DMACTRL_FLUSH | HPC_DMACTRL_RESET);
		m_scsi_dma.m_to_mem = (m_scsi_dma.m_ctrl & HPC_DMACTRL_TO_MEM);
		m_scsi_dma.m_active = (m_scsi_dma.m_ctrl & HPC_DMACTRL_ENABLE);
		if (m_scsi_dma.m_drq && m_scsi_dma.m_active)
			do_scsi_dma();
		break;

	case 0x00ac/4:
		LOGMASKED(LOG_WRITES, "%s: HPC Parallel Buffer Pointer Write: %08x (%08x)\n", machine().describe_context(), data, mem_mask);
		m_parbuf_ptr = data;
		break;
	case 0x0120/4:
		if (ACCESSING_BITS_8_15)
		{
			LOGMASKED(LOG_SCSI, "%s: HPC SCSI Controller Address Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_wd33c93->indir_addr_w((uint8_t)(data >> 8));
		}
		break;
	case 0x0124/4:
		if (ACCESSING_BITS_8_15)
		{
			LOGMASKED(LOG_SCSI, "%s: HPC SCSI Controller Data Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_wd33c93->indir_reg_w((uint8_t)(data >> 8));
		}
		break;
	case 0x01b0/4:
		LOGMASKED(LOG_WRITES, "%s: HPC Misc. Status Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (BIT(data, 0))
			LOGMASKED(LOG_WRITES, "    Force DSP hard reset\n" );

		if (BIT(data, 1))
			LOGMASKED(LOG_WRITES, "    Force IRQA\n" );

		if (BIT(data, 2))
			LOGMASKED(LOG_WRITES, "    Set IRQA polarity high\n" );
		else
			LOGMASKED(LOG_WRITES, "    Set IRQA polarity low\n" );

		if (BIT(data, 3))
			LOGMASKED(LOG_WRITES, "    SRAM size: 32K\n" );
		else
			LOGMASKED(LOG_WRITES, "    SRAM size:  8K\n" );

		m_misc_status = data;
		break;
	case 0x01bc/4:
		m_cpu_aux_ctrl = data;
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
		LOGMASKED(LOG_INT, "%s: HPC Local Interrupt 0 Status Write (Ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x01c4/4:
	{
		LOGMASKED(LOG_INT, "%s: HPC Local Interrupt 0 Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old_mask = m_local_int_mask[0];
		m_local_int_mask[0] = data;
		if (old_mask != m_local_int_mask[0])
			update_irq(0);
		break;
	}
	case 0x01c8/4:
		LOGMASKED(LOG_INT, "%s: HPC Local Interrupt 1 Status Write (Ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x01cc/4:
	{
		LOGMASKED(LOG_INT, "%s: HPC Local Interrupt 1 Mask Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old_mask = m_local_int_mask[1];
		m_local_int_mask[1] = data;
		if (old_mask != m_local_int_mask[1])
			update_irq(1);
		break;
	}
	case 0x01d4/4:
		LOGMASKED(LOG_INT, "%s: HPC VME Interrupt Mask 0 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_vme_int_mask[0] = data;
		break;
	case 0x01d8/4:
		LOGMASKED(LOG_INT, "%s: HPC VME Interrupt Mask 1 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_vme_int_mask[1] = data;
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
		m_scc[index]->ab_dc_w(0, (uint8_t)data);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel B Control Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d04/4:
	case 0x0d14/4:
	case 0x0d24/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ab_dc_w(1, (uint8_t)data);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel B Data Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d08/4:
	case 0x0d18/4:
	case 0x0d28/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ab_dc_w(2, (uint8_t)data);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel A Control Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	case 0x0d0c/4:
	case 0x0d1c/4:
	case 0x0d2c/4:
	{
		const uint32_t index = (offset >> 2) & 3;
		m_scc[index]->ab_dc_w(3, (uint8_t)data);
		LOGMASKED(LOG_DUART0 << index, "%s: HPC DUART%d Channel A Data Write: %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		break;
	}
	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown HPC write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fb80000 + offset*4, data, mem_mask);
		break;
	}
}

//**************************************************************************
//  SCSI DMA
//**************************************************************************

void hpc1_device::dump_chain(uint32_t base)
{
	const uint32_t addr = m_cpu_space->read_dword(base);
	const uint32_t ctrl = m_cpu_space->read_dword(base+4);
	const uint32_t next = m_cpu_space->read_dword(base+8);

	LOGMASKED(LOG_CHAIN, "Chain Node:\n");
	LOGMASKED(LOG_CHAIN, "    Addr: %08x\n", addr);
	LOGMASKED(LOG_CHAIN, "    Ctrl: %08x\n", ctrl);
	LOGMASKED(LOG_CHAIN, "    Next: %08x\n", next);

	if (next != 0 && !BIT(addr, 31))
	{
		dump_chain(next & 0x0fffffff);
	}
}
void hpc1_device::fetch_chain()
{
	m_scsi_dma.m_ctrl = m_cpu_space->read_dword(m_scsi_dma.m_desc);
	m_scsi_dma.m_addr = m_cpu_space->read_dword(m_scsi_dma.m_desc+4);
	m_scsi_dma.m_next = m_cpu_space->read_dword(m_scsi_dma.m_desc+8);
	m_scsi_dma.m_length = m_scsi_dma.m_ctrl & 0x1fff;

	LOGMASKED(LOG_CHAIN, "Fetching chain from %08x:\n", m_scsi_dma.m_desc);
	LOGMASKED(LOG_CHAIN, "    Addr: %08x\n", m_scsi_dma.m_addr);
	LOGMASKED(LOG_CHAIN, "    Ctrl: %08x\n", m_scsi_dma.m_ctrl);
	LOGMASKED(LOG_CHAIN, "    Next: %08x\n", m_scsi_dma.m_next);
}

void hpc1_device::decrement_chain()
{
	m_scsi_dma.m_length--;
	if (m_scsi_dma.m_length == 0)
	{
		if (BIT(m_scsi_dma.m_addr, 31))
		{
			m_scsi_dma.m_active = false;
			m_scsi_dma.m_ctrl &= ~HPC_DMACTRL_ENABLE;
			return;
		}
		m_scsi_dma.m_desc = m_scsi_dma.m_next & 0x0fffffff;
		fetch_chain();
	}
}

WRITE_LINE_MEMBER(hpc1_device::scsi_drq)
{
	m_scsi_dma.m_drq = state;

	if (m_scsi_dma.m_drq && m_scsi_dma.m_active)
	{
		do_scsi_dma();
	}
}

void hpc1_device::do_scsi_dma()
{
	if (m_scsi_dma.m_to_mem)
		m_cpu_space->write_byte(m_scsi_dma.m_addr & 0x0fffffff, m_wd33c93->dma_r());
	else
		m_wd33c93->dma_w(m_cpu_space->read_byte(m_scsi_dma.m_addr & 0x0fffffff));

	m_scsi_dma.m_addr++;
	decrement_chain();

	if (!m_scsi_dma.m_active)
	{
		// clear HPC3 DMA active flag
		m_scsi_dma.m_ctrl &= ~HPC_DMACTRL_ENABLE;
	}
}

//**************************************************************************
//  PIT TIMERS
//**************************************************************************

void hpc1_device::set_timer_int_clear(uint32_t data)
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

WRITE_LINE_MEMBER(hpc1_device::timer0_int)
{
	LOGMASKED(LOG_PIT, "Timer0 Interrupt: %d\n", state);
	if (state)
		m_maincpu->set_input_line(MIPS3_IRQ2, ASSERT_LINE);
}

WRITE_LINE_MEMBER(hpc1_device::timer1_int)
{
	LOGMASKED(LOG_PIT, "Timer2 Interrupt: %d\n", state);
	if (state)
		m_maincpu->set_input_line(MIPS3_IRQ3, ASSERT_LINE);
}

WRITE_LINE_MEMBER(hpc1_device::timer2_int)
{
	LOGMASKED(LOG_PIT, "Timer2 Interrupt (Disabled): %d\n", state);
}

//**************************************************************************
//  SERIAL DUARTS
//**************************************************************************

WRITE_LINE_MEMBER(hpc1_device::duart0_int_w) { duart_int_w(0, state); }
WRITE_LINE_MEMBER(hpc1_device::duart1_int_w) { duart_int_w(1, state); }
WRITE_LINE_MEMBER(hpc1_device::duart2_int_w) { duart_int_w(2, state); }

void hpc1_device::duart_int_w(int channel, int state)
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

//**************************************************************************
//  INTERRUPTS
//**************************************************************************

void hpc1_device::raise_local_irq(int channel, uint8_t source_mask)
{
	m_local_int_status[channel] |= source_mask;
	update_irq(channel);
}

void hpc1_device::lower_local_irq(int channel, uint8_t source_mask)
{
	m_local_int_status[channel] &= ~source_mask;
	update_irq(channel);
}

void hpc1_device::update_irq(int channel)
{
	bool old_status = m_int_status[channel];
	m_int_status[channel] = (m_local_int_status[channel] & m_local_int_mask[channel]);

	if (old_status != m_int_status[channel])
	{
		LOGMASKED(LOG_INT, "%s IRQ%d: %02x & %02x\n", m_int_status[channel] ? "Asserting" : "Clearing", channel,
			m_local_int_status[channel], m_local_int_mask[channel]);
		m_maincpu->set_input_line(MIPS3_IRQ0 + channel, m_int_status[channel] ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER(hpc1_device::scsi_irq)
{
	if (state)
	{
		LOGMASKED(LOG_SCSI, "SCSI: Set IRQ\n");
		raise_local_irq(0, LOCAL0_SCSI);
	}
	else
	{
		LOGMASKED(LOG_SCSI, "SCSI: Clear IRQ\n");
		lower_local_irq(0, LOCAL0_SCSI);
	}
}
