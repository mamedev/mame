// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

2017-11-02 Skeleton

Transitional Technology Inc. SCSI Q-bus host adapter, part of the QTx series
(x = 'S' for single-ended, x = 'D' for differential). UTx-1 was also provided for Unibus.

Chips: NCR 53C90A, Motorola MC68901P, Fujitsu 8464A-10L (8KB static ram), Xicor X24C44P (16x16 serial NOVRAM), and 14 undumped
Lattice PLDs.

Other: LED, 20MHz crystal. Next to the MC68901P is another chip just as large (48 pin DIL), with a huge "MFG. UNDER LICENSE FROM
       DIGITAL EQUIPMENT CORP." sticker covering all details. Assumed to be a Motorola MC68008 CPU.

************************************************************************************************************************************/

#include "emu.h"
#include "qtx.h"

#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/eepromser.h"


// device type definition
DEFINE_DEVICE_TYPE(TTI_QTS1, qts1_device, "qts1", "TTI QTS-1 SCSI Host Adapter")


qts1_device::qts1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TTI_QTS1, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_localcpu(*this, "localcpu")
	, m_mfp(*this, "mfp")
	, m_asc(*this, "scsibus:7:asc")
	, m_dma_address(0)
{
}

void qts1_device::device_start()
{
	save_item(NAME(m_dma_address));
}


u8 qts1_device::asc_r(offs_t offset)
{
	return m_asc->read(offset ^ 1);
}

void qts1_device::asc_w(offs_t offset, u8 data)
{
	m_asc->write(offset ^ 1, data);
}

void qts1_device::dma_address_w(offs_t offset, u8 data)
{
	m_dma_address &= ~(0xff000000 >> (offset * 8));
	m_dma_address |= u32(data) << (24 - offset * 8);
}

u8 qts1_device::io_status_r()
{
	return 0;
}

void qts1_device::channel_w(u8 data)
{
	switch (data & 0x03)
	{
	case 0:
		m_mfp->i4_w(0);
		break;

	case 1:
		m_mfp->i3_w(0);
		break;

	case 3:
		m_mfp->i5_w(0);
		break;
	}
}

void qts1_device::prg_map(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("firmware", 0);
	map(0x7e000, 0x7ffff).ram();
	map(0x80000, 0x80017).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write));
	map(0x80018, 0x8001f).ram();
	map(0x80020, 0x80029).rw(FUNC(qts1_device::asc_r), FUNC(qts1_device::asc_w));
	map(0x80070, 0x80077).w("bitlatch", FUNC(ls259_device::write_d0));
	map(0x80078, 0x8007b).w(FUNC(qts1_device::dma_address_w));
	map(0x8007c, 0x8007c).r(FUNC(qts1_device::io_status_r));
	map(0x8007d, 0x8007d).w(FUNC(qts1_device::channel_w));
}

void qts1_device::fc7_map(address_map &map)
{
	map(0xffff5, 0xffff5).r(m_mfp, FUNC(mc68901_device::get_vector));
}

static void qtx_scsi_devices(device_slot_interface &device)
{
	default_scsi_devices(device);
	device.option_add_internal("asc", NCR53C90A);
}

void qts1_device::asc_config(device_t *device)
{
	ncr53c90a_device &adapter = downcast<ncr53c90a_device &>(*device);

	adapter.set_clock(20_MHz_XTAL);

	adapter.irq_handler_cb().set(m_mfp, FUNC(mc68901_device::i7_w)).invert();
	//adapter.drq_handler_cb().set(?);
}

void qts1_device::device_add_mconfig(machine_config &config)
{
	M68008(config, m_localcpu, 20_MHz_XTAL / 2); // guess
	m_localcpu->set_addrmap(AS_PROGRAM, &qts1_device::prg_map);
	m_localcpu->set_addrmap(m68008_device::AS_CPU_SPACE, &qts1_device::fc7_map);

	MC68901(config, m_mfp, 20_MHz_XTAL / 2); // guess
	m_mfp->set_timer_clock(20_MHz_XTAL / 8); // guess
	m_mfp->out_tco_cb().set(m_mfp, FUNC(mc68901_device::rc_w));
	m_mfp->out_tco_cb().append(m_mfp, FUNC(mc68901_device::tc_w));
	m_mfp->out_so_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_mfp->out_irq_cb().set_inputline("localcpu", M68K_IRQ_2); // probably

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", qtx_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:1", qtx_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:2", qtx_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", qtx_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", qtx_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:5", qtx_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", qtx_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:7", qtx_scsi_devices, "asc", true).set_option_machine_config("asc", [this] (device_t *device) { asc_config(device); });

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_mfp, FUNC(mc68901_device::si_w));

	EEPROM_X24C44_16BIT(config, "novram").do_callback().set("mfp", FUNC(mc68901_device::i0_w));

	ls259_device &bitlatch(LS259(config, "bitlatch")); // U17
	bitlatch.q_out_cb<0>().set("novram", FUNC(eeprom_serial_x24c44_device::di_write));
	bitlatch.q_out_cb<1>().set("novram", FUNC(eeprom_serial_x24c44_device::clk_write));
	bitlatch.q_out_cb<2>().set("novram", FUNC(eeprom_serial_x24c44_device::cs_write));
}

ROM_START(qts1)
	ROM_REGION(0x8000, "firmware", 0)
	ROM_SYSTEM_BIOS(0, "v2.3", "Firmware Version 2.3")
	ROMX_LOAD("tti_10012000_rev2.3.bin", 0x0000, 0x8000, CRC(95a5bce8) SHA1(46d7c99e37ca5598aec2062dfd9759853a237c14), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.7", "Firmware Version 1.7")
	ROMX_LOAD("tti_10012000_rev1.7.bin", 0x0000, 0x8000, CRC(6660c059) SHA1(05d97009b5b8034dda520f655c73c474da97f822), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *qts1_device::device_rom_region() const
{
	return ROM_NAME(qts1);
}
