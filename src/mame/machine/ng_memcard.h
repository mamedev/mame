// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    ng_memcard.h

    NEOGEO Memory card functions.

*********************************************************************/
#ifndef MAME_MACHINE_NG_MEMCARD_H
#define MAME_MACHINE_NG_MEMCARD_H

#pragma once

#include "imagedev/memcard.h"


// ======================> ng_memcard_device

class ng_memcard_device : public device_t, public device_memcard_image_interface
{
public:
	// construction/destruction
	ng_memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "neo"; }

	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;

	// bus interface
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	bool present() { return is_loaded(); }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_memcard_data[0x800];
};


// device type definition
DECLARE_DEVICE_TYPE(NG_MEMCARD, ng_memcard_device)


#endif // MAME_MACHINE_NG_MEMCARD_H
