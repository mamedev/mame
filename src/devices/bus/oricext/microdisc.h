// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_BUS_ORICEXT_MICRODISC_H
#define MAME_BUS_ORICEXT_MICRODISC_H

#include "oricext.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

DECLARE_DEVICE_TYPE(MICRODISC, microdisc_device)

class microdisc_device : public oricext_device
{
public:
	microdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~microdisc_device();


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

	required_device<fd1793_device> fdc;

	uint8_t *microdisc_rom;
	floppy_image_device *floppies[4];
	uint8_t port_314;
	bool intrq_state, drq_state, hld_state;

	virtual void device_start() override;
	virtual void device_reset() override;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	void remap();

private:
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_hld_w);

	DECLARE_WRITE8_MEMBER(port_314_w);
	DECLARE_READ8_MEMBER(port_314_r);
	DECLARE_READ8_MEMBER(port_318_r);

	void map(address_map &map);

	DECLARE_FLOPPY_FORMATS(floppy_formats);
};

#endif // MAME_BUS_ORICEXT_MICRODISC_H
