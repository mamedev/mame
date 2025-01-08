// license:BSD-3-Clause
// copyright-holders:Carl
// TODO: SCSI, requires NCR5380 BSY IRQs
#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/wd_fdc.h"
#include "machine/mc68681.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "machine/ncr5380.h"
#include "imagedev/floppy.h"
#include "formats/naslite_dsk.h"


namespace {

class lb186_state : public driver_device
{
public:
	lb186_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppies(*this, "fdc:%u", 0U)
		, m_scsi(*this, "scsibus:7:ncr5380")
	{
	}

	void lb186(machine_config &config);

private:
	void sio_out_w(uint8_t data);
	void drive_sel_w(uint8_t data);
	static void floppy_formats(format_registration &fr);
	static void ncr5380(device_t *device);
	void lb186_io(address_map &map) ATTR_COLD;
	void lb186_map(address_map &map) ATTR_COLD;

	required_device<i80186_cpu_device> m_maincpu;
	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	required_device<ncr5380_device> m_scsi;
};

void lb186_state::sio_out_w(uint8_t data)
{
	m_fdc->mr_w(BIT(data, 5));
	m_maincpu->tmrin1_w(BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);
}

void lb186_state::drive_sel_w(uint8_t data)
{
	m_fdc->dden_w(BIT(data, 5));

	unsigned int drive = data & 0xf;
	switch(drive)
	{
		case 0:
			return;
		case 1:
			drive = 0;
			break;
		case 2:
			drive = 1;
			break;
		case 4:
			drive = 2;
			break;
		case 8:
			drive = 3;
			break;
		default:
			logerror("More than one drive enabled!\n");
			return;
	}

	floppy_image_device *const floppy = m_floppies[drive]->get_device();
	m_fdc->set_floppy(floppy);
	floppy->ss_w(BIT(data, 4));
}

void lb186_state::lb186_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram(); // fixed 256k for now
	map(0xfc000, 0xfffff).rom().region("bios", 0);
}

void lb186_state::lb186_io(address_map &map)
{
	map.unmap_value_high();
	map(0x1000, 0x101f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x1080, 0x108f).rw(m_scsi, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)).umask16(0x00ff);
	map(0x1100, 0x1107).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0x1180, 0x1180).rw(m_scsi, FUNC(ncr5380_device::dma_r), FUNC(ncr5380_device::dma_w));
	map(0x1200, 0x1200).w(FUNC(lb186_state::drive_sel_w));
}

static void lb186_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void lb186_state::ncr5380(device_t *device)
{
	devcb_base *devcb;
	(void)devcb;
	downcast<ncr5380_device &>(*device).irq_handler().set(":maincpu", FUNC(i80186_cpu_device::int1_w));
	downcast<ncr5380_device &>(*device).drq_handler().set(":maincpu", FUNC(i80186_cpu_device::drq0_w));
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("ncr5380", NCR5380);
}

void lb186_state::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
}

void lb186_state::lb186(machine_config &config)
{
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &lb186_state::lb186_map);
	m_maincpu->set_addrmap(AS_IO, &lb186_state::lb186_io);

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));
	duart.a_tx_cb().set("rs232_1", FUNC(rs232_port_device::write_txd));
	duart.b_tx_cb().set("rs232_2", FUNC(rs232_port_device::write_txd));
	duart.outport_cb().set(FUNC(lb186_state::sio_out_w));

	rs232_port_device &rs232_1(RS232_PORT(config, "rs232_1", default_rs232_devices, "terminal"));
	rs232_1.rxd_handler().set("duart", FUNC(scn2681_device::rx_a_w));
	rs232_port_device &rs232_2(RS232_PORT(config, "rs232_2", default_rs232_devices, nullptr));
	rs232_2.rxd_handler().set("duart", FUNC(scn2681_device::rx_b_w));

	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	m_fdc->intrq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::int2_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq0_w));
	FLOPPY_CONNECTOR(config, m_floppies[0], lb186_floppies, "525dd", lb186_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[1], lb186_floppies, nullptr, lb186_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[2], lb186_floppies, nullptr, lb186_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[3], lb186_floppies, nullptr, lb186_state::floppy_formats);

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsibus:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:7", scsi_devices, "ncr5380", true).set_option_machine_config("ncr5380", lb186_state::ncr5380);
}

ROM_START( lb186 )
	ROM_REGION16_LE(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v335", "BIOS Version 3.35") // 28 January 1987
	ROMX_LOAD("a75515_v3.35.rom", 0x0000, 0x2000, CRC(245824fb) SHA1(b39ed91d421513f5912fdbc290aaa3f1b7d4f1e0), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("a75516_v3.35.rom", 0x0001, 0x2000, CRC(9d9a5e22) SHA1(070be31c622f50508e8cbdb797c79978b6a4b8f6), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ramdisk", "RAM Disk BIOS Version 1.00")
	ROMX_LOAD("a75523.rom", 0x0000, 0x2000, CRC(2d22e826) SHA1(e366e489f580b440131ad5212722391b60af90cd), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("a75524.rom", 0x0001, 0x2000, CRC(9c9b249c) SHA1(e988e92d9fa6fe66f89ef748021e9a0501d2807e), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

} // anonymous namespace


COMP( 1985, lb186, 0, 0, lb186, 0, lb186_state, empty_init, "Ampro Computers", "Little Board/186", MACHINE_NO_SOUND_HW )
