// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A590 / A2091

    DMAC based HD controller for the Amiga 500 and Zorro-II

***************************************************************************/

#include "a590.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type A590 = &device_creator<a590_device>;
const device_type A2091 = &device_creator<a2091_device>;

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
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( dmac_hdc )
	MCFG_DMAC_ADD("dmac", 0)
	MCFG_DMAC_SCSI_READ_HANDLER(READ8(dmac_hdc_device, dmac_scsi_r))
	MCFG_DMAC_SCSI_WRITE_HANDLER(WRITE8(dmac_hdc_device, dmac_scsi_w))
	MCFG_DMAC_INT_HANDLER(WRITELINE(dmac_hdc_device, dmac_int_w))
	MCFG_DMAC_CFGOUT_HANDLER(WRITELINE(dmac_hdc_device, dmac_cfgout_w))
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_1)
	MCFG_DEVICE_ADD("wd33c93", WD33C93, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_WD33C93_IRQ_CB(WRITELINE(dmac_hdc_device, scsi_irq_w))
MACHINE_CONFIG_END

machine_config_constructor dmac_hdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmac_hdc );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( dmac_hdc )
	ROM_REGION16_BE(0x8000, "bootrom", 0)
	ROM_DEFAULT_BIOS("v70")

	ROM_SYSTEM_BIOS(0, "v60", "Version 6.0")
	ROMX_LOAD("390388-03.u13", 0x0000, 0x2000, CRC(2e77bbff) SHA1(8a098845068f32cfa4d34a278cd290f61d35a52c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("390389-03.u12", 0x0001, 0x2000, CRC(b0b8cf24) SHA1(fcf4017505f4d441814b45d559c19eab43816b30), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("390388-03.u13", 0x4000, 0x2000, CRC(2e77bbff) SHA1(8a098845068f32cfa4d34a278cd290f61d35a52c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("390389-03.u12", 0x4001, 0x2000, CRC(b0b8cf24) SHA1(fcf4017505f4d441814b45d559c19eab43816b30), ROM_SKIP(1) | ROM_BIOS(1))

	// changelog v6.1: prevent accesses to location 0 by application programs
	ROM_SYSTEM_BIOS(1, "v61", "Version 6.1")
	ROMX_LOAD("390721-01.u13", 0x0000, 0x2000, CRC(00dbf615) SHA1(503940d04fb3b49eaa61100fd3a487018b35e25a), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390722-01.u12", 0x0001, 0x2000, CRC(c460cfdb) SHA1(0de457daec3b84f75e8fb344defe24ce56cda3e0), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390721-01.u13", 0x4000, 0x2000, CRC(00dbf615) SHA1(503940d04fb3b49eaa61100fd3a487018b35e25a), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390722-01.u12", 0x4001, 0x2000, CRC(c460cfdb) SHA1(0de457daec3b84f75e8fb344defe24ce56cda3e0), ROM_SKIP(1) | ROM_BIOS(2))

	// changelog v6.6: fixes dual SCSI problems with the wd33c93a controller
	ROM_SYSTEM_BIOS(2, "v66", "Version 6.6")
	ROMX_LOAD("390721-02.u13", 0x0000, 0x2000, CRC(c0871d25) SHA1(e155f18abb90cf820589c15e70559d3b6b391af8), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("390722-02.u12", 0x0001, 0x2000, CRC(e536bbb2) SHA1(fd7f8a6da18c1b02d07eb990c2467a24183ede12), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("390721-02.u13", 0x4000, 0x2000, CRC(c0871d25) SHA1(e155f18abb90cf820589c15e70559d3b6b391af8), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("390722-02.u12", 0x4001, 0x2000, CRC(e536bbb2) SHA1(fd7f8a6da18c1b02d07eb990c2467a24183ede12), ROM_SKIP(1) | ROM_BIOS(3))

	// final Commodore released version
	ROM_SYSTEM_BIOS(3, "v70", "Version 7.0")
	ROMX_LOAD("390721-03.u13", 0x0000, 0x2000, CRC(2942747a) SHA1(dbd7648e79c753337ff3e4f491de224bf05e6bb6), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD("390722-03.u12", 0x0001, 0x2000, CRC(a9ccffed) SHA1(149f5bd52e2d29904e3de483b9ad772448e9278e), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD("390721-03.u13", 0x4000, 0x2000, CRC(2942747a) SHA1(dbd7648e79c753337ff3e4f491de224bf05e6bb6), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD("390722-03.u12", 0x4001, 0x2000, CRC(a9ccffed) SHA1(149f5bd52e2d29904e3de483b9ad772448e9278e), ROM_SKIP(1) | ROM_BIOS(4))

	// third-party upgrade ROM, requires a small ROM adapter pcb
	ROM_SYSTEM_BIOS(4, "g614", "Guru-ROM 6.14")
	ROMX_LOAD("gururom_v614.bin", 0x0000, 0x8000, CRC(04e52f93) SHA1(6da21b6f5e8f8837d64507cd8a4d5cdcac4f426b), ROM_GROUPWORD | ROM_BIOS(5))

	// pal16l8a
	ROM_REGION(0x104, "ram_controller", 0)
	ROM_LOAD("390333-03.u5", 0x000, 0x104, CRC(dc4a8d9b) SHA1(761a1318106e49057f95258699076ec1079967ad))
ROM_END

const rom_entry *dmac_hdc_device::device_rom_region() const
{
	return ROM_NAME( dmac_hdc );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmac_hdc_device - constructor
//-------------------------------------------------

dmac_hdc_device::dmac_hdc_device(const machine_config &mconfig, device_type type, const char *tag,
	device_t *owner, UINT32 clock, const char *name, const char *shortname) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	m_int6(false),
	m_dmac(*this, "dmac"),
	m_wdc(*this, "wd33c93")
{
}

a590_device::a590_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dmac_hdc_device(mconfig, A590, tag, owner, clock, "CBM A590 HD Controller", "a590"),
	device_exp_card_interface(mconfig, *this),
	m_dips(*this, "dips"),
	m_jp1(*this, "jp1"),
	m_jp2(*this, "jp2"),
	m_jp4(*this, "jp4")
{
}

a2091_device::a2091_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dmac_hdc_device(mconfig, A2091, tag, owner, clock, "CBM A2091 HD Controller", "a2091"),
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
	m_dmac->set_address_space(m_slot->m_space);
	m_dmac->set_rom(memregion("bootrom")->base());
}

void a2091_device::device_start()
{
	set_zorro_device();

	// setup DMAC
	m_dmac->set_address_space(m_slot->m_space);
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
	case 0x48: return m_wdc->read(space, 0);
	case 0x49: return m_wdc->read(space, 1);
	}

	return 0xff;
}

WRITE8_MEMBER( dmac_hdc_device::dmac_scsi_w )
{
	switch (offset)
	{
	case 0x48: m_wdc->write(space, 0, data); break;
	case 0x49: m_wdc->write(space, 1, data); break;
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
