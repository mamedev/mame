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
	microdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~microdisc_device();

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_ADDRESS_MAP(map, 8);
	void port_314_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port_314_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port_318_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void fdc_hld_w(int state);

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

	uint8_t *microdisc_rom;
	floppy_image_device *floppies[4];
	uint8_t port_314;
	bool intrq_state, drq_state, hld_state;

	virtual void device_start() override;
	virtual void device_reset() override;
	const tiny_rom_entry *device_rom_region() const override;
	machine_config_constructor device_mconfig_additions() const override;

	void remap();
};

#endif
