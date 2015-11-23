// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "microdisc.h"
#include "formats/oric_dsk.h"

const device_type MICRODISC = &device_creator<microdisc_device>;

ROM_START( microdisc )
	ROM_REGION( 0x2000, "microdisc", 0 )
	ROM_LOAD ("microdis.rom", 0, 0x02000, CRC(a9664a9c) SHA1(0d2ef6e67322f48f4b7e08d8bbe68827e2074561) )
ROM_END

FLOPPY_FORMATS_MEMBER( microdisc_device::floppy_formats )
	FLOPPY_ORIC_DSK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( microdisc_floppies )
	SLOT_INTERFACE( "3dsdd", FLOPPY_3_DSDD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( microdisc )
	MCFG_FD1793_ADD("fdc", XTAL_8MHz/8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(microdisc_device, fdc_irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(microdisc_device, fdc_drq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(microdisc_device, fdc_hld_w))
	MCFG_WD_FDC_FORCE_READY

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", microdisc_floppies, "3dsdd", microdisc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", microdisc_floppies, NULL,    microdisc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", microdisc_floppies, NULL,    microdisc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", microdisc_floppies, NULL,    microdisc_device::floppy_formats)
MACHINE_CONFIG_END

DEVICE_ADDRESS_MAP_START(map, 8, microdisc_device)
	AM_RANGE(0x310, 0x313) AM_DEVREADWRITE("fdc", fd1793_t, read, write)
	AM_RANGE(0x314, 0x314) AM_READWRITE(port_314_r, port_314_w)
	AM_RANGE(0x318, 0x318) AM_READ(port_318_r)
ADDRESS_MAP_END

microdisc_device::microdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	oricext_device(mconfig, MICRODISC, "Microdisc floppy drive interface", tag, owner, clock, "microdisc", __FILE__),
	fdc(*this, "fdc"), microdisc_rom(nullptr), port_314(0), intrq_state(false), drq_state(false), hld_state(false)
{
}

microdisc_device::~microdisc_device()
{
}

void microdisc_device::device_start()
{
	oricext_device::device_start();
	microdisc_rom = device().machine().root_device().memregion(this->subtag("microdisc").c_str())->base();
	cpu->space(AS_PROGRAM).install_device(0x0000, 0xffff, *this, &microdisc_device::map);

	for(int i=0; i<4; i++) {
		char name[32];
		sprintf(name, "fdc:%d", i);
		floppies[i] = subdevice<floppy_connector>(name)->get_device();
	}
	intrq_state = drq_state = hld_state = false;
}

void microdisc_device::device_reset()
{
	port_314 = 0x00;
	irq_w(false);
	remap();
	fdc->set_floppy(floppies[0]);

	// The bootstrap checksums part of the high ram and if the sum is
	// 0 it goes wrong.
	ram[0xe000] = 0x42;
}

const rom_entry *microdisc_device::device_rom_region() const
{
	return ROM_NAME( microdisc );
}

machine_config_constructor microdisc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( microdisc );
}

void microdisc_device::remap()
{
	if(port_314 & P_ROMDIS) {
		bank_c000_r->set_base(rom+0x0000);
		bank_e000_r->set_base(rom+0x2000);
		bank_f800_r->set_base(rom+0x3800);
		bank_c000_w->set_base(junk_write);
		bank_e000_w->set_base(junk_write);
		bank_f800_w->set_base(junk_write);
	} else {
		bank_c000_r->set_base(ram+0xc000);
		bank_c000_w->set_base(ram+0xc000);
		if(port_314 & P_EPROM) {
			bank_e000_r->set_base(ram+0xe000);
			bank_f800_r->set_base(ram+0xf800);
			bank_e000_w->set_base(ram+0xe000);
			bank_f800_w->set_base(ram+0xf800);
		} else {
			bank_e000_r->set_base(microdisc_rom+0x0000);
			bank_f800_r->set_base(microdisc_rom+0x1800);
			bank_e000_w->set_base(junk_write);
			bank_f800_w->set_base(junk_write);
		}
	}
}

WRITE8_MEMBER(microdisc_device::port_314_w)
{
	port_314 = data;
	remap();
	floppy_image_device *floppy = floppies[(port_314 >> 5) & 3];
	fdc->set_floppy(floppy);
	fdc->dden_w(port_314 & P_DDEN);
	if(floppy) {
		floppy->ss_w(port_314 & P_SS ? 1 : 0);
		floppy->mon_w(0);
	}
	irq_w(intrq_state && (port_314 & P_IRQEN));
}

READ8_MEMBER(microdisc_device::port_314_r)
{
	return (intrq_state && (port_314 & P_IRQEN)) ? 0x7f : 0xff;
}

READ8_MEMBER(microdisc_device::port_318_r)
{
	return drq_state ? 0x7f : 0xff;
}

WRITE_LINE_MEMBER(microdisc_device::fdc_irq_w)
{
	intrq_state = state;
	irq_w(intrq_state && (port_314 & P_IRQEN));
}

WRITE_LINE_MEMBER(microdisc_device::fdc_drq_w)
{
	drq_state = state;
}

WRITE_LINE_MEMBER(microdisc_device::fdc_hld_w)
{
	logerror("hld %d\n", state);
	hld_state = state;
	floppies[(port_314 >> 5) & 3]->mon_w(!hld_state);
}
