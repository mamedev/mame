// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    BusTek/BusLogic BT-54x series PC/AT SCSI host adapters

    The earlier version of the BT-542B, with BusTek labels, has a NCR 86C05
    bus controller where a later version has the 80C20 ASIC instead. It is
    believed that these chips are largely compatible with each other, as
    are the NCR 53CF94 and Emulex FAS216 SCSI controllers.

***************************************************************************/

#include "emu.h"
#include "bt54x.h"

#include "machine/ncr5390.h"
//#include "machine/ncr86c05.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"

DEFINE_DEVICE_TYPE(BT545S, bt545s_device, "bt545s", "BusLogic BT-545S Fast SCSI Host Adapter")

bt545s_device::bt545s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, BT545S, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_mpu(*this, "mpu")
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
{
}

void bt545s_device::device_start()
{
}

u8 bt545s_device::local_status_r()
{
	return 0;
}

void bt545s_device::local_map(address_map &map)
{
	map(0x00000, 0x01fff).ram();
	//map(0x02000, 0x0201f).rw("busintf", FUNC(ncr86c05_device::local_read), FUNC(ncr86c05_device::local_write));
	map(0x02080, 0x0208f).m("scsi:7:fsc", FUNC(ncr53cf94_device::map));
	map(0x02180, 0x02180).r(FUNC(bt545s_device::local_status_r));
	map(0xf8000, 0xfffff).rom().region("mpu", 0);
}

static void fast_scsi_devices(device_slot_interface &device)
{
	// FIXME: these device options are placeholders
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("fsc", NCR53CF94); // FAS216
}

void bt545s_device::fsc_config(device_t *device)
{
	ncr53cf94_device &fsc = downcast<ncr53cf94_device &>(*device);

	fsc.set_clock(40_MHz_XTAL);

	fsc.irq_handler_cb().set(m_mpu, FUNC(i80188_cpu_device::int0_w)); // mostly polled
	//fsc.drq_handler_cb().set("busintf", FUNC(ncr86c05_device::dma_req_w));
}

void bt545s_device::device_add_mconfig(machine_config &config)
{
	I80188(config, m_mpu, 40_MHz_XTAL / 2); // SAB80188-1-N; clock guessed
	m_mpu->set_addrmap(AS_PROGRAM, &bt545s_device::local_map);

	//ncr86c05_device &busintf(NCR86C05(config, "busintf", 0));
	//busintf.mint_callback().set(m_mpu, FUNC(i80188_cpu_device::int1_w));
	//busintf.dma_ack_callback().set("scsi:7:fsc", FUNC(ncr53cf94_device::dma_w));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", fast_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", fast_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", fast_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", fast_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", fast_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", fast_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", fast_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", fast_scsi_devices, "fsc", true).set_option_machine_config("fsc", [this] (device_t *device) { fsc_config(device); });

	PC8477A(config, m_fdc, 24_MHz_XTAL); // actually PC8477BV
}

ROM_START(bt545s)
	ROM_REGION(0x4000, "bios", 0) // "(C) Copyright 1992     BIOS Version 4.50"
	ROM_LOAD("u15_27128_5002026-4.50.bin", 0x0000, 0x4000, CRC(1bd3247b) SHA1(9d46a99f4b3057e94ef422f387218de2c4553c1a))

	ROM_REGION(0x8000, "mpu", 0) // "(C) Copyright 1992 BusLogic Inc.      542BH92/10/05"
	ROM_LOAD("u2_27256_5002005-3.31.bin", 0x0000, 0x8000, CRC(20473714) SHA1(797a8dba182049949f7a5c14d8bef4b4e908305b))
ROM_END

const tiny_rom_entry *bt545s_device::device_rom_region() const
{
	return ROM_NAME(bt545s);
}
