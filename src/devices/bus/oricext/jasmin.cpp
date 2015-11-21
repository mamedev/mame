// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "jasmin.h"
#include "formats/oric_dsk.h"

const device_type JASMIN = &device_creator<jasmin_device>;

ROM_START( jasmin )
	ROM_REGION( 0x800, "jasmin", 0 )
	ROM_LOAD("jasmin.rom", 0, 0x800, CRC(37220e89) SHA1(70e59b8abd67092f050462abc6cb5271e4c15f01) )
ROM_END

FLOPPY_FORMATS_MEMBER( jasmin_device::floppy_formats )
	FLOPPY_ORIC_DSK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( jasmin_floppies )
	SLOT_INTERFACE( "3dsdd", FLOPPY_3_DSDD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( jasmin )
	MCFG_WD1770_ADD("fdc", XTAL_8MHz)
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(oricext_device, irq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", jasmin_floppies, "3dsdd", jasmin_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", jasmin_floppies, NULL,    jasmin_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", jasmin_floppies, NULL,    jasmin_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", jasmin_floppies, NULL,    jasmin_device::floppy_formats)
MACHINE_CONFIG_END

INPUT_PORTS_START( jasmin )
	PORT_START("JASMIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Boot") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHANGED_MEMBER(DEVICE_SELF, jasmin_device, boot_pressed, 0)
INPUT_PORTS_END

DEVICE_ADDRESS_MAP_START(map, 8, jasmin_device)
	AM_RANGE(0x3f4, 0x3f7) AM_DEVREADWRITE("fdc", wd1770_t, read, write)
	AM_RANGE(0x3f8, 0x3f8) AM_WRITE(side_sel_w)
	AM_RANGE(0x3f9, 0x3f9) AM_WRITE(fdc_reset_w)
	AM_RANGE(0x3fa, 0x3fa) AM_WRITE(ram_access_w)
	AM_RANGE(0x3fb, 0x3fb) AM_WRITE(rom_access_w)
	AM_RANGE(0x3fc, 0x3ff) AM_WRITE(select_w)
ADDRESS_MAP_END

jasmin_device::jasmin_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	oricext_device(mconfig, JASMIN, "Jasmin floppy drive interface", tag, owner, clock, "jasmin", __FILE__),
	fdc(*this, "fdc"), side_sel(false), fdc_reset(false), ram_access(false), rom_access(false), jasmin_rom(nullptr), cur_floppy(nullptr)
{
}

jasmin_device::~jasmin_device()
{
}

void jasmin_device::device_start()
{
	oricext_device::device_start();
	jasmin_rom = device().machine().root_device().memregion(this->subtag("jasmin").c_str())->base();
	cpu->space(AS_PROGRAM).install_device(0x0000, 0xffff, *this, &jasmin_device::map);

	for(int i=0; i<4; i++) {
		char name[32];
		sprintf(name, "fdc:%d", i);
		floppies[i] = subdevice<floppy_connector>(name)->get_device();
	}
}

void jasmin_device::device_reset()
{
	side_sel = fdc_reset = ram_access = rom_access = false;
	select[0] = select[1] = select[2] = select[3] = false;
	remap();
	cur_floppy = NULL;
	fdc->set_floppy(NULL);
}

const rom_entry *jasmin_device::device_rom_region() const
{
	return ROM_NAME( jasmin );
}

machine_config_constructor jasmin_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jasmin );
}

ioport_constructor jasmin_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( jasmin );
}

void jasmin_device::remap()
{
	if(rom_access) {
		if(ram_access) {
			bank_c000_r->set_base(ram+0xc000);
			bank_e000_r->set_base(ram+0xe000);
			bank_f800_r->set_base(jasmin_rom);
			bank_c000_w->set_base(ram+0xc000);
			bank_e000_w->set_base(ram+0xe000);
			bank_f800_w->set_base(junk_write);
		} else {
			bank_c000_r->set_base(junk_read);
			bank_e000_r->set_base(junk_read);
			bank_f800_r->set_base(jasmin_rom);
			bank_c000_w->set_base(junk_write);
			bank_e000_w->set_base(junk_write);
			bank_f800_w->set_base(junk_write);
		}
	} else {
		if(ram_access) {
			bank_c000_r->set_base(ram+0xc000);
			bank_e000_r->set_base(ram+0xe000);
			bank_f800_r->set_base(ram+0xf800);
			bank_c000_w->set_base(ram+0xc000);
			bank_e000_w->set_base(ram+0xe000);
			bank_f800_w->set_base(ram+0xf800);
		} else {
			bank_c000_r->set_base(rom+0x0000);
			bank_e000_r->set_base(rom+0x2000);
			bank_f800_r->set_base(rom+0x3800);
			bank_c000_w->set_base(junk_write);
			bank_e000_w->set_base(junk_write);
			bank_f800_w->set_base(junk_write);
		}
	}
}

INPUT_CHANGED_MEMBER(jasmin_device::boot_pressed)
{
	if(newval) {
		rom_access = true;
		remap();
		cpu->reset();
	}
}

WRITE8_MEMBER(jasmin_device::side_sel_w)
{
	side_sel = data & 1;
	if(cur_floppy)
		cur_floppy->ss_w(side_sel);
}

WRITE8_MEMBER(jasmin_device::fdc_reset_w)
{
	if((data & 1) != fdc_reset)
		fdc->soft_reset();
	fdc_reset = data & 1;
}

WRITE8_MEMBER(jasmin_device::ram_access_w)
{
	ram_access = data & 1;
	remap();
}

WRITE8_MEMBER(jasmin_device::rom_access_w)
{
	rom_access = data & 1;
	remap();
}

WRITE8_MEMBER(jasmin_device::select_w)
{
	select[offset] = data & 1;
	cur_floppy = NULL;
	for(int i=0; i != 4; i++)
		if(select[i]) {
			cur_floppy = floppies[i];
			break;
		}
	fdc->set_floppy(cur_floppy);
}
