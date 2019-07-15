// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "jasmin.h"
#include "formats/oric_dsk.h"

DEFINE_DEVICE_TYPE(JASMIN, jasmin_device, "jasmin", "Jasmin floppy drive interface")

ROM_START( jasmin )
	ROM_REGION( 0x800, "jasmin", 0 )
	ROM_LOAD("jasmin.rom", 0, 0x800, CRC(37220e89) SHA1(70e59b8abd67092f050462abc6cb5271e4c15f01) )
ROM_END

FLOPPY_FORMATS_MEMBER( jasmin_device::floppy_formats )
	FLOPPY_ORIC_DSK_FORMAT
FLOPPY_FORMATS_END

static void jasmin_floppies(device_slot_interface &device)
{
	device.option_add("3dsdd", FLOPPY_3_DSDD);
}

INPUT_PORTS_START( jasmin )
	PORT_START("JASMIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Boot") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHANGED_MEMBER(DEVICE_SELF, jasmin_device, boot_pressed, nullptr)
INPUT_PORTS_END

void jasmin_device::map(address_map &map)
{
	map(0x3f4, 0x3f7).rw("fdc", FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x3f8, 0x3ff).w(m_fdlatch, FUNC(ls259_device::write_d0));
}

jasmin_device::jasmin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	oricext_device(mconfig, JASMIN, tag, owner, clock),
	m_fdc(*this, "fdc"),
	m_fdlatch(*this, "fdlatch"),
	m_floppies(*this, "fdc:%u", 0U),
	m_jasmin_rom(*this, "jasmin"),
	m_cur_floppy(nullptr)
{
}

jasmin_device::~jasmin_device()
{
}

void jasmin_device::device_start()
{
	oricext_device::device_start();
	cpu->space(AS_PROGRAM).install_device(0x0000, 0xffff, *this, &jasmin_device::map);
}

const tiny_rom_entry *jasmin_device::device_rom_region() const
{
	return ROM_NAME( jasmin );
}

void jasmin_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 8_MHz_XTAL);
	m_fdc->drq_wr_callback().set(FUNC(oricext_device::irq_w));

	FLOPPY_CONNECTOR(config, "fdc:0", jasmin_floppies, "3dsdd", jasmin_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", jasmin_floppies, nullptr, jasmin_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", jasmin_floppies, nullptr, jasmin_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", jasmin_floppies, nullptr, jasmin_device::floppy_formats);

	LS259(config, m_fdlatch); // U14
	m_fdlatch->q_out_cb<0>().set(FUNC(jasmin_device::side_sel_w));
	m_fdlatch->q_out_cb<1>().set(m_fdc, FUNC(wd1770_device::mr_w));
	m_fdlatch->q_out_cb<2>().set(FUNC(jasmin_device::ram_access_w));
	m_fdlatch->q_out_cb<3>().set(FUNC(jasmin_device::rom_access_w));
	m_fdlatch->q_out_cb<4>().set(FUNC(jasmin_device::select_w));
	m_fdlatch->q_out_cb<5>().set(FUNC(jasmin_device::select_w));
	m_fdlatch->q_out_cb<6>().set(FUNC(jasmin_device::select_w));
	m_fdlatch->q_out_cb<7>().set(FUNC(jasmin_device::select_w));
}

ioport_constructor jasmin_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( jasmin );
}

void jasmin_device::remap()
{
	if(m_fdlatch->q3_r()) {
		if(m_fdlatch->q2_r()) {
			bank_c000_r->set_base(ram+0xc000);
			bank_e000_r->set_base(ram+0xe000);
			bank_f800_r->set_base(m_jasmin_rom.target());
			bank_c000_w->set_base(ram+0xc000);
			bank_e000_w->set_base(ram+0xe000);
			bank_f800_w->set_base(junk_write);
		} else {
			bank_c000_r->set_base(junk_read);
			bank_e000_r->set_base(junk_read);
			bank_f800_r->set_base(m_jasmin_rom.target());
			bank_c000_w->set_base(junk_write);
			bank_e000_w->set_base(junk_write);
			bank_f800_w->set_base(junk_write);
		}
	} else {
		if(m_fdlatch->q2_r()) {
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
		m_fdlatch->write_bit(3, 1);
		cpu->reset();
	}
}

WRITE_LINE_MEMBER(jasmin_device::side_sel_w)
{
	if(m_cur_floppy)
		m_cur_floppy->ss_w(state);
}

WRITE_LINE_MEMBER(jasmin_device::ram_access_w)
{
	remap();
}

WRITE_LINE_MEMBER(jasmin_device::rom_access_w)
{
	remap();
}

WRITE_LINE_MEMBER(jasmin_device::select_w)
{
	m_cur_floppy = nullptr;
	for(int i=0; i != 4; i++)
		if(BIT(m_fdlatch->output_state(), 4+i)) {
			m_cur_floppy = m_floppies[i]->get_device();
			break;
		}
	m_fdc->set_floppy(m_cur_floppy);
}
