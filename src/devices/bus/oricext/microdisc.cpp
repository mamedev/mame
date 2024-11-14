// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "microdisc.h"
#include "formats/oric_dsk.h"

DEFINE_DEVICE_TYPE(ORIC_MICRODISC, oric_microdisc_device, "oric_microdisc", "Microdisc floppy drive interface")

ROM_START( microdisc )
	ROM_REGION( 0x2000, "microdisc", 0 )
	ROM_LOAD ("microdis.rom", 0, 0x02000, CRC(a9664a9c) SHA1(0d2ef6e67322f48f4b7e08d8bbe68827e2074561) )
ROM_END

void oric_microdisc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ORIC_DSK_FORMAT);
}

static void microdisc_floppies(device_slot_interface &device)
{
	device.option_add("3dsdd", FLOPPY_3_DSDD);
	device.option_add("3dsqd", FLOPPY_3_DSQD);
}

void oric_microdisc_device::map_io(address_space_installer &space)
{
	space.install_read_handler(0x310, 0x313, read8sm_delegate(fdc, FUNC(fd1793_device::read)));
	space.install_write_handler(0x310, 0x313, write8sm_delegate(fdc, FUNC(fd1793_device::write)));

	space.install_read_handler(0x314, 0x314, read8smo_delegate(*this, FUNC(oric_microdisc_device::port_314_r)));
	space.install_write_handler(0x314, 0x314, write8smo_delegate(*this, FUNC(oric_microdisc_device::port_314_w)));

	space.install_read_handler(0x318, 0x318, read8smo_delegate(*this, FUNC(oric_microdisc_device::port_318_r)));
}

void oric_microdisc_device::map_rom()
{
	(*view)[2].unmap_write(0xe000, 0xffff);
	(*view)[2].install_rom(0xe000, 0xffff, microdisc_rom);
}

oric_microdisc_device::oric_microdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ORIC_MICRODISC, tag, owner, clock),
	device_oricext_interface(mconfig, *this),
	fdc(*this, "fdc"),
	microdisc_rom(*this, "microdisc"),
	floppies(*this, "fdc:%u", 0U),
	port_314(0), intrq_state(false), drq_state(false), hld_state(false)
{
}

oric_microdisc_device::~oric_microdisc_device()
{
}

void oric_microdisc_device::device_start()
{
	intrq_state = drq_state = hld_state = false;
}

void oric_microdisc_device::device_reset()
{
	port_314 = 0x00;
	view->select(2);
	irq_w(false);
	fdc->set_floppy(floppies[0]->get_device());
}

const tiny_rom_entry *oric_microdisc_device::device_rom_region() const
{
	return ROM_NAME( microdisc );
}

void oric_microdisc_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, fdc, 8_MHz_XTAL / 8);
	fdc->intrq_wr_callback().set(FUNC(oric_microdisc_device::fdc_irq_w));
	fdc->drq_wr_callback().set(FUNC(oric_microdisc_device::fdc_drq_w));
	fdc->hld_wr_callback().set(FUNC(oric_microdisc_device::fdc_hld_w));
	fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, "fdc:0", microdisc_floppies, "3dsqd", oric_microdisc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", microdisc_floppies, nullptr, oric_microdisc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", microdisc_floppies, nullptr, oric_microdisc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", microdisc_floppies, nullptr, oric_microdisc_device::floppy_formats);
}

void oric_microdisc_device::port_314_w(uint8_t data)
{
	port_314 = data;
	if(port_314 & P_ROMDIS)
		view->select(0);
	else if(port_314 & P_EPROM)
		view->select(1);
	else
		view->select(2);

	floppy_image_device *floppy = floppies[(port_314 >> 5) & 3]->get_device();
	fdc->set_floppy(floppy);
	fdc->dden_w(port_314 & P_DDEN);
	if(floppy) {
		floppy->ss_w(port_314 & P_SS ? 1 : 0);
		floppy->mon_w(0);
	}
	irq_w(intrq_state && (port_314 & P_IRQEN));
}

uint8_t oric_microdisc_device::port_314_r()
{
	return (intrq_state && (port_314 & P_IRQEN)) ? 0x7f : 0xff;
}

uint8_t oric_microdisc_device::port_318_r()
{
	return drq_state ? 0x7f : 0xff;
}

void oric_microdisc_device::fdc_irq_w(int state)
{
	intrq_state = state;
	irq_w(intrq_state && (port_314 & P_IRQEN));
}

void oric_microdisc_device::fdc_drq_w(int state)
{
	drq_state = state;
}

void oric_microdisc_device::fdc_hld_w(int state)
{
	hld_state = state;
	floppy_image_device *floppy = floppies[(port_314 >> 5) & 3]->get_device();
	if(floppy)
		floppy->mon_w(!hld_state);
}
