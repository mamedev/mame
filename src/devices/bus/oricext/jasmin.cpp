// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "jasmin.h"
#include "formats/oric_dsk.h"
#include "formats/fs_oric_jasmin.h"

DEFINE_DEVICE_TYPE(ORIC_JASMIN, oric_jasmin_device, "oric_jasmin", "Jasmin floppy drive interface")

ROM_START( jasmin )
	ROM_REGION( 0x800, "jasmin", 0 )
	ROM_LOAD("jasmin.rom", 0, 0x800, CRC(37220e89) SHA1(70e59b8abd67092f050462abc6cb5271e4c15f01) )
ROM_END

void oric_jasmin_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ORIC_DSK_FORMAT);
	fr.add(FLOPPY_ORIC_JASMIN_FORMAT);

	fr.add(fs::ORIC_JASMIN);
}

static void jasmin_floppies(device_slot_interface &device)
{
	device.option_add("3dsdd", FLOPPY_3_DSDD);
}

INPUT_PORTS_START( jasmin )
	PORT_START("JASMIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Boot") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(oric_jasmin_device::boot_pressed), 0)
INPUT_PORTS_END

void oric_jasmin_device::map_io(address_space_installer &space)
{
	space.install_read_handler(0x3f4, 0x3f7, read8sm_delegate(m_fdc, FUNC(wd1770_device::read)));
	space.install_write_handler(0x3f4, 0x3f7, write8sm_delegate(m_fdc, FUNC(wd1770_device::write)));

	space.install_write_handler(0x3f8, 0x3ff, write8sm_delegate(m_fdlatch, FUNC(ls259_device::write_d0)));
}

void oric_jasmin_device::map_rom()
{
	(*view)[2].unmap_write(0xf800, 0xffff);
	(*view)[2].install_rom(0xf800, 0xffff, m_jasmin_rom);

	(*view)[3].unmap_readwrite(0xc000, 0xffff);
	(*view)[3].install_rom(0xf800, 0xffff, m_jasmin_rom);
}

oric_jasmin_device::oric_jasmin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ORIC_JASMIN, tag, owner, clock),
	device_oricext_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_fdlatch(*this, "fdlatch"),
	m_floppies(*this, "fdc:%u", 0U),
	m_jasmin_rom(*this, "jasmin"),
	m_cur_floppy(nullptr)
{
}

oric_jasmin_device::~oric_jasmin_device()
{
}

void oric_jasmin_device::device_start()
{
}

const tiny_rom_entry *oric_jasmin_device::device_rom_region() const
{
	return ROM_NAME( jasmin );
}

void oric_jasmin_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 8_MHz_XTAL);
	m_fdc->drq_wr_callback().set(FUNC(oric_jasmin_device::irq_w));

	FLOPPY_CONNECTOR(config, "fdc:0", jasmin_floppies, "3dsdd", oric_jasmin_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", jasmin_floppies, nullptr, oric_jasmin_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", jasmin_floppies, nullptr, oric_jasmin_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", jasmin_floppies, nullptr, oric_jasmin_device::floppy_formats);

	LS259(config, m_fdlatch); // U14
	m_fdlatch->q_out_cb<0>().set(FUNC(oric_jasmin_device::side_sel_w));
	m_fdlatch->q_out_cb<1>().set(m_fdc, FUNC(wd1770_device::mr_w));
	m_fdlatch->q_out_cb<2>().set(FUNC(oric_jasmin_device::ram_access_w));
	m_fdlatch->q_out_cb<3>().set(FUNC(oric_jasmin_device::rom_access_w));
	m_fdlatch->q_out_cb<4>().set(FUNC(oric_jasmin_device::select_w));
	m_fdlatch->q_out_cb<5>().set(FUNC(oric_jasmin_device::select_w));
	m_fdlatch->q_out_cb<6>().set(FUNC(oric_jasmin_device::select_w));
	m_fdlatch->q_out_cb<7>().set(FUNC(oric_jasmin_device::select_w));
}

ioport_constructor oric_jasmin_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( jasmin );
}

void oric_jasmin_device::remap()
{
	if(m_fdlatch->q3_r()) {
		if(m_fdlatch->q2_r())
			view->select(3);
		else
			view->select(2);
	} else {
		if(m_fdlatch->q2_r())
			view->select(1);
		else
			view->select(0);
	}
}

INPUT_CHANGED_MEMBER(oric_jasmin_device::boot_pressed)
{
	if(newval) {
		m_fdlatch->write_bit(3, 1);
		reset_w(1);
		reset_w(0);
	}
}

void oric_jasmin_device::side_sel_w(int state)
{
	if(m_cur_floppy)
		m_cur_floppy->ss_w(state);
}

void oric_jasmin_device::ram_access_w(int state)
{
	remap();
}

void oric_jasmin_device::rom_access_w(int state)
{
	remap();
}

void oric_jasmin_device::select_w(int state)
{
	m_cur_floppy = nullptr;
	for(int i=0; i != 4; i++)
		if(BIT(m_fdlatch->output_state(), 4+i)) {
			m_cur_floppy = m_floppies[i]->get_device();
			break;
		}
	m_fdc->set_floppy(m_cur_floppy);
}
