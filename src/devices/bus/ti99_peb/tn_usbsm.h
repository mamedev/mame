// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Nouspikel USB / SmartMedia interface card
    See tn_usmsm.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __USBSMART__
#define __USBSMART__

#include "emu.h"
#include "peribox.h"
#include "machine/smartmed.h"
#include "machine/strata.h"

extern const device_type TI99_USBSM;

class nouspikel_usb_smartmedia_device : public ti_expansion_card_device
{
public:
	nouspikel_usb_smartmedia_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

protected:
	virtual void device_start(void) override;
	virtual void device_reset(void) override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

private:

	int         m_feeprom_page;
	int         m_sram_page;
	int         m_cru_register;
	bool        m_tms9995_mode;

	bool        m_enable_io;
	bool        m_enable_int;
	bool        m_enable_sm;
	bool        m_write_flash;

	UINT16      m_input_latch;
	UINT16      m_output_latch;
	std::vector<UINT16> m_ram;

	required_device<smartmedia_image_device> m_smartmedia;
	required_device<strataflash_device> m_flash;
};

#endif
