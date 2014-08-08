#ifndef __MSX_CART_DISK_H
#define __MSX_CART_DISK_H

#include "bus/msx_cart/cartridge.h"
#include "machine/wd_fdc.h"
#include "imagedev/flopdrv.h"
#include "imagedev/floppy.h"


extern const device_type MSX_CART_VY0010;


class msx_cart_vy0010 : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_vy0010(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

	void post_load();

	DECLARE_FLOPPY_FORMATS(floppy_formats);

private:
	required_device<wd_fdc_analog_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;

	UINT8 m_side_control;
	UINT8 m_control;

	void set_side_control(UINT8 data);
	void set_control(UINT8 data);
};


#endif
