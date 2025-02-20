// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A2091

    DMAC based SCSI controller for the A2000

    Notes:
    - Hardware is almost identical to the A590 controller for the A500
    - Sold as "A2091-40" it was bundled with 40 MB Quantum ProDrive 40S
    - A2000 + A2091-40 = A2000HD
    - XT drives aren't usually supported, but a header to connect one
      can be soldered to the board (unknown how well this really works)

    TODO:
    - JP5 switch order/polarity
    - JP3/JP201 switches
    - Bootrom disable
    - SCSI drives (fatalerrors early if you enable a drive)
    - The XT drive should be a slot option (or removed once SCSI works)

***************************************************************************/

#include "emu.h"
#include "a2091.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"

#define LOG_XT (1U << 1)

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_A2091, bus::amiga::zorro::a2091_device, "amiga_a2091", "Commodore A2091 Hard Disk/SCSI Controller")

namespace bus::amiga::zorro {

a2091_device::a2091_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_A2091, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_irq(*this, "irq"),
	m_dmac(*this, "dmac"),
	m_wdc(*this, "scsi:7:wdc"),
	m_xt(*this, "xt"),
	m_jp1(*this, "jp1"),
	m_ram_size(0)
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( a2091 )
	PORT_START("jp1")
	PORT_CONFNAME(0x03, 0x03, "Installed RAM")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "512K")
	PORT_CONFSETTING(0x02, "1MB")
	PORT_CONFSETTING(0x03, "2MB")

	PORT_START("jp2")
	PORT_CONFNAME(0x01, 0x01, "Autoboot")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x01, "Enabled")

	// JP3 (would switch between generating INT2 or INT6)

	PORT_START("jp5")
	PORT_CONFNAME(0x01, 0x01, "LUN")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x01, "Enabled")
	PORT_CONFNAME(0x02, 0x02, "Time-out")
	PORT_CONFSETTING(0x00, "Short")
	PORT_CONFSETTING(0x02, "Long")
	PORT_CONFNAME(0x04, 0x04, "Reserved")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x04, "Enabled")

	// JP201
INPUT_PORTS_END

ioport_constructor a2091_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a2091 );
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( firmware )
	ROM_REGION16_BE(0x8000, "bootrom", 0)
	ROM_DEFAULT_BIOS("v70")

	ROM_SYSTEM_BIOS(0, "v592", "Version 5.92")
	ROMX_LOAD("390508-02.u13", 0x0000, 0x2000, CRC(f99e3934) SHA1(1aa7f789db48ad0b75f8bb6b3e5532fbf30f5766), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390509-02.u12", 0x0001, 0x2000, CRC(8f9dd6f8) SHA1(3a8a4639a498bf9a95cb11c45c13687efa714942), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390508-02.u13", 0x4000, 0x2000, CRC(f99e3934) SHA1(1aa7f789db48ad0b75f8bb6b3e5532fbf30f5766), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390509-02.u12", 0x4001, 0x2000, CRC(8f9dd6f8) SHA1(3a8a4639a498bf9a95cb11c45c13687efa714942), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(1, "v61", "Version 6.1")
	ROMX_LOAD("390721-01.u13", 0x0000, 0x2000, CRC(00dbf615) SHA1(503940d04fb3b49eaa61100fd3a487018b35e25a), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("390722-01.u12", 0x0001, 0x2000, CRC(c460cfdb) SHA1(0de457daec3b84f75e8fb344defe24ce56cda3e0), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("390721-01.u13", 0x4000, 0x2000, CRC(00dbf615) SHA1(503940d04fb3b49eaa61100fd3a487018b35e25a), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("390722-01.u12", 0x4001, 0x2000, CRC(c460cfdb) SHA1(0de457daec3b84f75e8fb344defe24ce56cda3e0), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v66", "Version 6.6")
	// COPYRIGHT ©1991 CBM  ALL RIGHTS RESERVED  390721-02V6.6 D464
	ROMX_LOAD("390721-02.u13", 0x0000, 0x2000, CRC(c0871d25) SHA1(e155f18abb90cf820589c15e70559d3b6b391af8), ROM_SKIP(1) | ROM_BIOS(2))
	// COPYRIGHT ©1991 CBM  ALL RIGHTS RESERVED  390722-02V6.6 F929
	ROMX_LOAD("390722-02.u12", 0x0001, 0x2000, CRC(e536bbb2) SHA1(fd7f8a6da18c1b02d07eb990c2467a24183ede12), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390721-02.u13", 0x4000, 0x2000, CRC(c0871d25) SHA1(e155f18abb90cf820589c15e70559d3b6b391af8), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("390722-02.u12", 0x4001, 0x2000, CRC(e536bbb2) SHA1(fd7f8a6da18c1b02d07eb990c2467a24183ede12), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "v70", "Version 7.0")
	// 360721-07  ©1994 CBM  V7.0 081C U13
	// A590 / A2091 390721-4  V7.0 ROM - (U13)  ©1993 CAI (different pn - same content?)
	ROMX_LOAD("390721-07.u13", 0x0000, 0x2000, CRC(2942747a) SHA1(dbd7648e79c753337ff3e4f491de224bf05e6bb6), ROM_SKIP(1) | ROM_BIOS(3))
	// 390722-07  ©1994 CBM  V7.0 3EF2 U12
	// A590 / A2091 390722-4  V7.0 ROM - (U12)  ©1993 CAI (different pn - same content?)
	ROMX_LOAD("390722-07.u12", 0x0001, 0x2000, CRC(a9ccffed) SHA1(149f5bd52e2d29904e3de483b9ad772448e9278e), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("390721-07.u13", 0x4000, 0x2000, CRC(2942747a) SHA1(dbd7648e79c753337ff3e4f491de224bf05e6bb6), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("390722-07.u12", 0x4001, 0x2000, CRC(a9ccffed) SHA1(149f5bd52e2d29904e3de483b9ad772448e9278e), ROM_SKIP(1) | ROM_BIOS(3))

	// third-party upgrade ROM, requires a small ROM adapter pcb
	ROM_SYSTEM_BIOS(4, "g614", "Guru-ROM 6.14")
	ROMX_LOAD("gururom_v614.bin", 0x0000, 0x8000, CRC(04e52f93) SHA1(6da21b6f5e8f8837d64507cd8a4d5cdcac4f426b), ROM_GROUPWORD | ROM_BIOS(4))

	// pal16l8a
	ROM_REGION(0x104, "ram_controller", 0)
	ROM_LOAD("390333-03.u5", 0x000, 0x104, CRC(dc4a8d9b) SHA1(761a1318106e49057f95258699076ec1079967ad))
ROM_END

const tiny_rom_entry *a2091_device::device_rom_region() const
{
	return ROM_NAME( firmware );
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void a2091_device::wd33c93_config(device_t *device)
{
	device->set_clock(28.37516_MHz_XTAL / 4); // 7M
	downcast<wd33c93a_device *>(device)->irq_cb().set(m_irq, FUNC(input_merger_any_high_device::in_w<0>));
	downcast<wd33c93a_device *>(device)->drq_cb().set(m_dmac, FUNC(amiga_dmac_device::sdreq_w));
}

void a2091_device::device_add_mconfig(machine_config &config)
{
	AMIGA_DMAC_REV2(config, m_dmac, 28.37516_MHz_XTAL / 4); // 7M
	m_dmac->set_rom("bootrom");
	m_dmac->cfgout_cb().set([this] (int state) { m_zorro->cfgout_w(state); });
	m_dmac->int_cb().set([this] (int state) { m_zorro->int2_w(state); });
	m_dmac->css_read_cb().set(m_wdc, FUNC(wd33c93a_device::indir_r));
	m_dmac->css_write_cb().set(m_wdc, FUNC(wd33c93a_device::indir_w));
	m_dmac->csx0_read_cb().set(FUNC(a2091_device::xt_r));
	m_dmac->csx0_write_cb().set(FUNC(a2091_device::xt_w));
	m_dmac->csx1_read_cb().set_ioport("jp5");
	m_dmac->xdack_read_cb().set(m_xt, FUNC(xt_hdc_device::dack_r));
	m_dmac->xdack_write_cb().set(m_xt, FUNC(xt_hdc_device::dack_w));

	INPUT_MERGER_ANY_HIGH(config, m_irq);
	m_irq->output_handler().set(m_dmac, FUNC(amiga_dmac_device::intx_w));

	NSCSI_BUS(config, "scsi", 0);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wdc", WD33C93A).machine_config([this] (device_t *device) { wd33c93_config(device); });

	XT_HDC(config, m_xt, 0);
	m_xt->irq_handler().set(m_irq, FUNC(input_merger_any_high_device::in_w<1>));
	m_xt->drq_handler().set(m_dmac, FUNC(amiga_dmac_device::xdreq_w));

	HARDDISK(config, "xt:primary");
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void a2091_device::device_start()
{
	// setup ram
	m_ram = make_unique_clear<uint16_t[]>(0x200000/2);

	// setup dmac
	m_dmac->set_address_space(&m_zorro->space());
	m_dmac->set_ram(m_ram.get());

	// register for save states
	save_pointer(NAME(m_ram), 0x200000/2);
	save_item(NAME(m_ram_size));
}

// the dmac handles this
void a2091_device::cfgin_w(int state) { m_dmac->configin_w(state); }

void a2091_device::busrst_w(int state)
{
	// call rst first as it will unmap memory
	m_dmac->rst_w(state);

	if (state == 0)
		m_dmac->ramsz_w(m_jp1->read() & 0x03);
}

uint8_t a2091_device::xt_r(offs_t offset)
{
	LOGMASKED(LOG_XT, "xt_r(%02x)\n", offset);

	switch (offset)
	{
		case 0: return m_xt->data_r();
		case 1: return m_xt->status_r();
		case 2: return 0x02; // jumper: 20/40 MB
	}

	return 0xff;
}

void a2091_device::xt_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_XT, "xt_w(%02x): %02x\n", offset, data);

	switch (offset)
	{
		case 0: m_xt->data_w(data); break;
		case 1: m_xt->reset_w(data); break;
		case 2: m_xt->select_w(data); break;
		case 3: m_xt->control_w(data); break;
	}
}

} // namespace bus::amiga::zorro
