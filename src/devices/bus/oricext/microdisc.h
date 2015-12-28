// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __MICRODISC_H__
#define __MICRODISC_H__

#include "oricext.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

extern const device_type MICRODISC;

class microdisc_device : public oricext_device
{
public:
	microdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~microdisc_device();

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_ADDRESS_MAP(map, 8);
	DECLARE_WRITE8_MEMBER(port_314_w);
	DECLARE_READ8_MEMBER(port_314_r);
	DECLARE_READ8_MEMBER(port_318_r);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_hld_w);

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

	required_device<fd1793_t> fdc;

	UINT8 *microdisc_rom;
	floppy_image_device *floppies[4];
	UINT8 port_314;
	bool intrq_state, drq_state, hld_state;

	virtual void device_start() override;
	virtual void device_reset() override;
	const rom_entry *device_rom_region() const override;
	machine_config_constructor device_mconfig_additions() const override;

	void remap();
};

#endif
