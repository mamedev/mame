// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A590 / A2091

    DMAC based HD controller for the Amiga 500 and Zorro-II

***************************************************************************/

#include "emu.h"
#include "a590.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A590,  a590_device,  "a590",  "CBM A590 HD Controller")
DEFINE_DEVICE_TYPE(A2091, a2091_device, "a2091", "CBM A2091 HD Controller")

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( a590_pcb )
	PORT_START("dips")
	PORT_DIPNAME(0x01, 0x01, "A590 Auto-Boot")
	PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(0x00, "Enabled")
	PORT_DIPSETTING(0x01, "Disabled")
	PORT_DIPNAME(0x02, 0x00, "A590 LUN")
	PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x02, "Enabled")
	PORT_DIPNAME(0x04, 0x04, "A590 Wait period")
	PORT_DIPLOCATION("DIP:3")
	PORT_DIPSETTING(0x00, "Short")
	PORT_DIPSETTING(0x04, "Long")
	PORT_DIPNAME(0x08, 0x00, "A590 Reserved")
	PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(0x00, "Enabled")
	PORT_DIPSETTING(0x08, "Disabled")
	PORT_START("jp1")
	PORT_DIPNAME(0x0f, 0x01, "A590 Memory size")
	PORT_DIPLOCATION("JP1:1,2,3,4")
	PORT_DIPSETTING(0x01, "Amnesia")
	PORT_DIPSETTING(0x02, "512K")
	PORT_DIPSETTING(0x04, "1MB")
	PORT_DIPSETTING(0x08, "2MB")
	PORT_START("jp2")
	PORT_DIPNAME(0x01, 0x00, "A590 Drive LED")
	PORT_DIPLOCATION("JP2:1")
	PORT_DIPSETTING(0x00, "XT Drive")
	PORT_DIPSETTING(0x01, "SCSI Drive")
	PORT_START("jp4")
	PORT_DIPNAME(0x01, 0x00, "A590 Interrupt")
	PORT_DIPLOCATION("JP4:1")
	PORT_DIPSETTING(0x00, "INT 2")
	PORT_DIPSETTING(0x01, "INT 6")
INPUT_PORTS_END

ioport_constructor a590_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a590_pcb );
}

static INPUT_PORTS_START( a2091_pcb )
	PORT_START("jp1")
	PORT_DIPNAME(0x0f, 0x01, "A2091 Memory size")
	PORT_DIPLOCATION("JP1:1,2,3,4")
	PORT_DIPSETTING(0x01, "0K")
	PORT_DIPSETTING(0x02, "512K")
	PORT_DIPSETTING(0x04, "1MB")
	PORT_DIPSETTING(0x08, "2MB")
	PORT_START("jp2")
	PORT_DIPNAME(0x01, 0x00, "A2091 Auto-Boot")
	PORT_DIPLOCATION("JP2:1")
	PORT_DIPSETTING(0x00, "Enabled")
	PORT_DIPSETTING(0x01, "Disabled")
	PORT_START("jp3")
	PORT_DIPNAME(0x01, 0x00, "A2091 Interrupt")
	PORT_DIPLOCATION("JP3:1")
	PORT_DIPSETTING(0x00, "INT 2")
	PORT_DIPSETTING(0x01, "INT 6")
	PORT_START("jp5")
	PORT_DIPNAME(0x01, 0x00, "A2091 LUN")
	PORT_DIPLOCATION("JP5:1")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x01, "Enabled")
	PORT_DIPNAME(0x02, 0x00, "A2091 Time-Out")
	PORT_DIPLOCATION("JP5:2")
	PORT_DIPSETTING(0x00, "Short")
	PORT_DIPSETTING(0x02, "Long")
	PORT_DIPNAME(0x04, 0x00, "A2091 Reserved")
	PORT_DIPLOCATION("JP5:3")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x02, "Enabled")
	PORT_START("jp201")
	PORT_DIPNAME(0x01, 0x00, "A2091 WD33C93 Clock")
	PORT_DIPLOCATION("JP201:1")
	PORT_DIPSETTING(0x00, "7 MHz")
	PORT_DIPSETTING(0x01, "14 MHz")
INPUT_PORTS_END

ioport_constructor a2091_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a2091_pcb );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dmac_hdc_device::wd33c93(device_t *device)
{
	device->set_clock(10000000);
	downcast<wd33c93a_device *>(device)->irq_cb().set(*this, FUNC(dmac_hdc_device::scsi_irq_w));
	downcast<wd33c93a_device *>(device)->drq_cb().set(*this, FUNC(dmac_hdc_device::scsi_drq_w));
}

void dmac_hdc_device::device_add_mconfig(machine_config &config)
{
	amiga_dmac_device &dmac(AMIGA_DMAC(config, "dmac", 0));
	dmac.scsi_read_handler().set(FUNC(dmac_hdc_device::dmac_scsi_r));
	dmac.scsi_write_handler().set(FUNC(dmac_hdc_device::dmac_scsi_w));
	dmac.int_handler().set(FUNC(dmac_hdc_device::dmac_int_w));
	dmac.cfgout_handler().set(FUNC(dmac_hdc_device::dmac_cfgout_w));

	NSCSI_BUS(config, "scsi", 0);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wd33c93", WD33C93A)
		.machine_config([this](device_t *device) { wd33c93(device); });
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( dmac_hdc )
	ROM_REGION16_BE(0x8000, "bootrom", 0)
	ROM_DEFAULT_BIOS("v70")

	ROM_SYSTEM_BIOS(0, "v46", "Version 4.6") // a590 only?
	ROMX_LOAD("390389-02.u13", 0x0000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0)) // checksum-16: d703
	ROMX_LOAD("390388-02.u12", 0x0001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0)) // checksum-16: e7e4
	ROMX_LOAD("390389-02.u13", 0x4000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("390388-02.u12", 0x4001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v592", "Version 5.92") // a2091 only?
	ROMX_LOAD("390508-02.u13", 0x0000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1)) // checksum-16: ?
	ROMX_LOAD("390509-02.u12", 0x0001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1)) // checksum-16: 288c
	ROMX_LOAD("390508-02.u13", 0x4000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("390509-02.u12", 0x4001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v60", "Version 6.0") // a590 only?
	ROMX_LOAD("390389-03.u13", 0x0000, 0x2000, CRC(2e77bbff) SHA1(8a098845068f32cfa4d34a278cd290f61d35a52c), ROM_SKIP(1) | ROM_BIOS(2)) // checksum-16: cbe8 (ok)
	ROMX_LOAD("390388-03.u12", 0x0001, 0x2000, CRC(b0b8cf24) SHA1(fcf4017505f4d441814b45d559c19eab43816b30), ROM_SKIP(1) | ROM_BIOS(2)) // checksum-16: dfa0 (ok)
	ROMX_LOAD("390389-03.u13", 0x4000, 0x2000, CRC(2e77bbff) SHA1(8a098845068f32cfa4d34a278cd290f61d35a52c), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390388-03.u12", 0x4001, 0x2000, CRC(b0b8cf24) SHA1(fcf4017505f4d441814b45d559c19eab43816b30), ROM_SKIP(1) | ROM_BIOS(2))

	// changelog v6.1: prevent accesses to location 0 by application programs
	ROM_SYSTEM_BIOS(3, "v61", "Version 6.1")
	ROMX_LOAD("390721-01.u13", 0x0000, 0x2000, CRC(00dbf615) SHA1(503940d04fb3b49eaa61100fd3a487018b35e25a), ROM_SKIP(1) | ROM_BIOS(3)) // checksum-16: f4b8 (ok)
	ROMX_LOAD("390722-01.u12", 0x0001, 0x2000, CRC(c460cfdb) SHA1(0de457daec3b84f75e8fb344defe24ce56cda3e0), ROM_SKIP(1) | ROM_BIOS(3)) // checksum-16: 088b (ok)
	ROMX_LOAD("390721-01.u13", 0x4000, 0x2000, CRC(00dbf615) SHA1(503940d04fb3b49eaa61100fd3a487018b35e25a), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("390722-01.u12", 0x4001, 0x2000, CRC(c460cfdb) SHA1(0de457daec3b84f75e8fb344defe24ce56cda3e0), ROM_SKIP(1) | ROM_BIOS(3))

	// changelog v6.6: fixes dual SCSI problems with the wd33c93a controller
	ROM_SYSTEM_BIOS(4, "v66", "Version 6.6")
	ROMX_LOAD("390721-02.u13", 0x0000, 0x2000, CRC(c0871d25) SHA1(e155f18abb90cf820589c15e70559d3b6b391af8), ROM_SKIP(1) | ROM_BIOS(4)) // checksum-16: d464 (ok)
	ROMX_LOAD("390722-02.u12", 0x0001, 0x2000, CRC(e536bbb2) SHA1(fd7f8a6da18c1b02d07eb990c2467a24183ede12), ROM_SKIP(1) | ROM_BIOS(4)) // checksum-16: f929 (ok)
	ROMX_LOAD("390721-02.u13", 0x4000, 0x2000, CRC(c0871d25) SHA1(e155f18abb90cf820589c15e70559d3b6b391af8), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD("390722-02.u12", 0x4001, 0x2000, CRC(e536bbb2) SHA1(fd7f8a6da18c1b02d07eb990c2467a24183ede12), ROM_SKIP(1) | ROM_BIOS(4))

	// final Commodore released version
	ROM_SYSTEM_BIOS(5, "v70", "Version 7.0") // also seen with -07
	ROMX_LOAD("390721-04.u13", 0x0000, 0x2000, CRC(2942747a) SHA1(dbd7648e79c753337ff3e4f491de224bf05e6bb6), ROM_SKIP(1) | ROM_BIOS(5)) // checksum-16: 081c (ok)
	ROMX_LOAD("390722-04.u12", 0x0001, 0x2000, CRC(a9ccffed) SHA1(149f5bd52e2d29904e3de483b9ad772448e9278e), ROM_SKIP(1) | ROM_BIOS(5)) // checksum-16: 3ef2 (ok)
	ROMX_LOAD("390721-04.u13", 0x4000, 0x2000, CRC(2942747a) SHA1(dbd7648e79c753337ff3e4f491de224bf05e6bb6), ROM_SKIP(1) | ROM_BIOS(5))
	ROMX_LOAD("390722-04.u12", 0x4001, 0x2000, CRC(a9ccffed) SHA1(149f5bd52e2d29904e3de483b9ad772448e9278e), ROM_SKIP(1) | ROM_BIOS(5))

	// third-party upgrade ROM, requires a small ROM adapter pcb
	ROM_SYSTEM_BIOS(6, "g614", "Guru-ROM 6.14")
	ROMX_LOAD("gururom_v614.bin", 0x0000, 0x8000, CRC(04e52f93) SHA1(6da21b6f5e8f8837d64507cd8a4d5cdcac4f426b), ROM_GROUPWORD | ROM_BIOS(6))

	// pal16l8a
	ROM_REGION(0x104, "ram_controller", 0)
	ROM_LOAD("390333-03.u5", 0x000, 0x104, CRC(dc4a8d9b) SHA1(761a1318106e49057f95258699076ec1079967ad))
ROM_END

const tiny_rom_entry *dmac_hdc_device::device_rom_region() const
{
	return ROM_NAME( dmac_hdc );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmac_hdc_device - constructor
//-------------------------------------------------

dmac_hdc_device::dmac_hdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_int6(false),
	m_dmac(*this, "dmac"),
	m_wdc(*this, "scsi:7:wd33c93")
{
}

a590_device::a590_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dmac_hdc_device(mconfig, A590, tag, owner, clock),
	device_exp_card_interface(mconfig, *this),
	m_dips(*this, "dips"),
	m_jp1(*this, "jp1"),
	m_jp2(*this, "jp2"),
	m_jp4(*this, "jp4")
{
}

a2091_device::a2091_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dmac_hdc_device(mconfig, A2091, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_jp1(*this, "jp1"),
	m_jp2(*this, "jp2"),
	m_jp3(*this, "jp3"),
	m_jp5(*this, "jp5"),
	m_jp201(*this, "jp201")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmac_hdc_device::device_start()
{
}

void a590_device::device_start()
{
	set_zorro_device();

	// setup DMAC
	m_dmac->set_address_space(&m_slot->space());
	m_dmac->set_rom(memregion("bootrom")->base());
}

void a2091_device::device_start()
{
	set_zorro_device();

	// setup DMAC
	m_dmac->set_address_space(&m_slot->space());
	m_dmac->set_rom(memregion("bootrom")->base());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmac_hdc_device::device_reset()
{
}

void dmac_hdc_device::resize_ram(int config)
{
	// allocate space for RAM
	switch (config & 0x0f)
	{
	case 0x01:
		m_ram.resize(0);
		m_dmac->ramsz_w(0);
		break;
	case 0x02:
		m_ram.resize(0x080000);
		m_dmac->ramsz_w(1);
		break;
	case 0x04:
		m_ram.resize(0x100000);
		m_dmac->ramsz_w(2);
		break;
	case 0x08:
		m_ram.resize(0x200000);
		m_dmac->ramsz_w(3);
		break;
	}

	m_dmac->set_ram(&m_ram[0]);
}

void a590_device::device_reset()
{
}

void a2091_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( a590_device::cfgin_w )
{
	// make sure we configure ourselves first
	m_int6 = m_jp4->read() & 0x01;
	resize_ram(m_dips->read() & 0x0f);

	// then tell the DMAC to start configuring
	m_dmac->configin_w(state);
}

WRITE_LINE_MEMBER( a2091_device::cfgin_w )
{
	// make sure we configure ourselves first
	m_int6 = m_jp3->read() & 0x01;
	resize_ram(m_jp1->read() & 0x0f);

	// then tell the DMAC to start configuring
	m_dmac->configin_w(state);
}

READ8_MEMBER( dmac_hdc_device::dmac_scsi_r )
{
	switch (offset)
	{
	case 0x48: return m_wdc->indir_addr_r();
	case 0x49: return m_wdc->indir_reg_r();
	}

	return 0xff;
}

WRITE8_MEMBER( dmac_hdc_device::dmac_scsi_w )
{
	switch (offset)
	{
	case 0x48: m_wdc->indir_addr_w(data); break;
	case 0x49: m_wdc->indir_reg_w(data); break;
	}
}

WRITE_LINE_MEMBER( dmac_hdc_device::dmac_int_w )
{
	if (m_int6)
		int6_w(state);
	else
		int2_w(state);
}

WRITE_LINE_MEMBER( dmac_hdc_device::scsi_irq_w )
{
	// should be or'ed with xt-ide IRQ
	m_dmac->intx_w(state);
}

WRITE_LINE_MEMBER( dmac_hdc_device::scsi_drq_w )
{
	m_dmac->xdreq_w(state);
}
