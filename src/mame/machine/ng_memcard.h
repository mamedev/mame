// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    ng_memcard.h

    NEOGEO Memory card functions.

*********************************************************************/

#pragma once

#ifndef __NG_MEMCARD_H__
#define __NG_MEMCARD_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NEOGEO_MEMCARD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NG_MEMCARD, 0)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// ======================> ng_memcard_device

class ng_memcard_device :  public device_t,
							public device_image_interface
{
public:
	// construction/destruction
	ng_memcard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual iodevice_t image_type() const override { return IO_MEMCARD; }

	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override { return "neo"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	virtual bool call_load() override;
	virtual void call_unload() override;
	virtual bool call_create(int format_type, option_resolution *format_options) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	/* returns the index of the current memory card, or -1 if none */
	int present() { return is_loaded() ? 0 : -1; }
private:
	UINT8 m_memcard_data[0x800];
};


// device type definition
extern const device_type NG_MEMCARD;


#endif  /* __NG_MEMCARD_H__ */
