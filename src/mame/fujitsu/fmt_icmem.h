// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*********************************************************************

    fmt_icmem.h

    FM Towns IC Memory Card

*********************************************************************/

#ifndef MAME_FUJITSU_FMT_ICMEM_H
#define MAME_FUJITSU_FMT_ICMEM_H

#pragma once

#include "imagedev/memcard.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class fmt_icmem_device : public device_t, public device_memcard_image_interface
{
public:
	// construction/destruction
	fmt_icmem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "icm"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	uint8_t static_mem_read(offs_t offset);
	void static_mem_write(offs_t offset, uint8_t data);
	uint8_t mem_read(offs_t offset);
	void mem_write(offs_t offset, uint8_t data);
	uint8_t status_r();
	uint8_t bank_r(offs_t offset);
	void bank_w(offs_t offset, uint8_t data);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_writeprotect;
	std::unique_ptr<uint8_t[]> m_memcard_ram;
	bool m_change;
	bool m_attr_select;
	uint8_t m_detect;
	uint8_t m_bank;
};


// device type definition
DECLARE_DEVICE_TYPE(FMT_ICMEM, fmt_icmem_device)


#endif  // MAME_FUJITSU_FMT_ICMEM_H
