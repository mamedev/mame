// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    TDL-12 SCSI Host Adapter (© 1985 T.D. Systems)

************************************************************************************************************************************/

#include "emu.h"
#include "tdl12.h"

#include "bus/nscsi/devices.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/ncr5380.h"
#include "machine/z8536.h"


// device type definition
DEFINE_DEVICE_TYPE(TDL12, tdl12_device, "tdl12", "TDL-12 SCSI Host Adapter")


tdl12_device::tdl12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TDL12, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_tdcpu(*this, "tdcpu")
{
}

void tdl12_device::device_start()
{
}

u8 tdl12_device::latch_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		logerror("%s: Reading from latch %c\n", machine().describe_context(), offset ? 'H' : 'L');
	return 0;
}

void tdl12_device::latch_w(offs_t offset, u8 data)
{
	logerror("%s: Writing %02X to latch %c\n", machine().describe_context(), data, offset ? 'H' : 'L');
}

u8 tdl12_device::in40_r()
{
	// Used as low 8 bits of jump offset
	// Valid values include 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xd0, 0xff
	return 0xff;
}

void tdl12_device::out40_w(u8 data)
{
	logerror("%s: Output %02X to port 40\n", machine().describe_context(), data);
}

void tdl12_device::out70_w(u8 data)
{
	// Data written is irrelevant
	logerror("%s: Output to port 70\n", machine().describe_context());
}

void tdl12_device::out74_w(u8 data)
{
	// Data written is irrelevant
	logerror("%s: Output to port 74\n", machine().describe_context());
}

void tdl12_device::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("firmware", 0);
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8001).rw(FUNC(tdl12_device::latch_r), FUNC(tdl12_device::latch_w));
}

void tdl12_device::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("cio", FUNC(z8536_device::read), FUNC(z8536_device::write));
	map(0x40, 0x40).rw(FUNC(tdl12_device::in40_r), FUNC(tdl12_device::out40_w));
	map(0x70, 0x70).w(FUNC(tdl12_device::out70_w));
	map(0x74, 0x74).w(FUNC(tdl12_device::out74_w));
	map(0x80, 0x87).m("scsi:7:ncr5380", FUNC(ncr5380_device::map));
}

void tdl12_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_tdcpu, 12_MHz_XTAL / 2); // covered by "TD SYSTEMS INC." sticker (most likely Z80B)
	m_tdcpu->set_addrmap(AS_PROGRAM, &tdl12_device::mem_map);
	m_tdcpu->set_addrmap(AS_IO, &tdl12_device::io_map);

	z8536_device &cio(Z8536(config, "cio", 12_MHz_XTAL / 2)); // ST Z8536AB1
	cio.irq_wr_cb().set_inputline(m_tdcpu, 0);
	cio.pb_wr_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(0);
	cio.pb_wr_cb().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(1);
	cio.pc_rd_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)).lshift(3);
	cio.pc_wr_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(2);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR5380).machine_config([this](device_t *device) {
		(void)this;
	});

	EEPROM_93C46_16BIT(config, "eeprom"); // NMC9346N
}


ROM_START(tdl12)
	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("tdl-12_v180.bin", 0x0000, 0x4000, CRC(fd0f4468) SHA1(1cf0bf8702747ff3047691d2b661bb08d40754f0)) // Am(27128?)-25DC

	ROM_REGION16_LE(0x40, "proms", 0) // probably PDP-11 bootstrap code (address & CE inputs are in common)
	ROM_LOAD16_BYTE("m_tdl-11.bin", 0x00, 0x20, NO_DUMP) // N82S123AN
	ROM_LOAD16_BYTE("m_tdl-12.bin", 0x01, 0x20, NO_DUMP) // N82S123AN

	ROM_REGION(0x7a1, "plds", 0)
	ROM_LOAD("mst_tdl-13.bin", 0x000, 0x117, NO_DUMP) // TIBPAL16L8-25CN (socketed, next to A/B jumpers; also decodes address)
	ROM_LOAD("mst_tdl-21.bin", 0x117, 0x117, NO_DUMP) // TIBPAL16L8-25CN
	ROM_LOAD("mst_tdl-26.bin", 0x22e, 0x117, NO_DUMP) // TIBPAL16L8-25CN
	ROM_LOAD("mst_tdl-31.bin", 0x345, 0x117, NO_DUMP) // TIBPAL16L8-25CN
	ROM_LOAD("mst_tdl-40.bin", 0x45c, 0x117, NO_DUMP) // TIBPAL16L8-25CN
	ROM_LOAD("mst_tdl-41.bin", 0x573, 0x117, NO_DUMP) // TIBPAL16L8-25CN
	ROM_LOAD("mst_tdl-42.bin", 0x68a, 0x117, NO_DUMP) // TIBPAL16L8-25CN
ROM_END

const tiny_rom_entry *tdl12_device::device_rom_region() const
{
	return ROM_NAME(tdl12);
}
