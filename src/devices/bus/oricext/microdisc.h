// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_BUS_ORICEXT_MICRODISC_H
#define MAME_BUS_ORICEXT_MICRODISC_H

#include "oricext.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

DECLARE_DEVICE_TYPE(ORIC_MICRODISC, oric_microdisc_device)

class oric_microdisc_device : public device_t, public device_oricext_interface
{
public:
	oric_microdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~oric_microdisc_device();

protected:
	enum {
		P_IRQEN  = 0x01,
		P_ROMDIS = 0x02,
		P_DDS    = 0x04,
		P_DDEN   = 0x08,
		P_SS     = 0x10,
		P_DRIVE  = 0x60,
		P_EPROM  = 0x80
	};

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_io(address_space_installer &space) override;
	virtual void map_rom() override;

private:
	required_device<fd1793_device> fdc;
	required_region_ptr<uint8_t> microdisc_rom;
	required_device_array<floppy_connector, 4> floppies;

	uint8_t port_314;
	bool intrq_state, drq_state, hld_state;

	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void fdc_hld_w(int state);

	void port_314_w(uint8_t data);
	uint8_t port_314_r();
	uint8_t port_318_r();

	static void floppy_formats(format_registration &fr);
};

#endif // MAME_BUS_ORICEXT_MICRODISC_H
