// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    Neo Geo Memory card functions

*********************************************************************/
#ifndef MAME_NEOGEO_NG_MEMCARD_H
#define MAME_NEOGEO_NG_MEMCARD_H

#pragma once

#include "imagedev/memcard.h"


class ng_memcard_device : public device_t, public device_memcard_image_interface
{
public:
	// construction/destruction
	ng_memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "neo"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	// bus interface
	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

	// control lines
	void lock1_w(int state);
	void unlock2_w(int state);
	void regsel_w(int state);

	bool present() { return is_loaded(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_memcard_data[0x800];
	uint8_t m_lock1;
	uint8_t m_unlock2;
	uint8_t m_regsel;
};


// device type definition
DECLARE_DEVICE_TYPE(NG_MEMCARD, ng_memcard_device)


#endif // MAME_NEOGEO_NG_MEMCARD_H
