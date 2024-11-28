// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    smartmed.h: header file for smartmed.c
*/

#ifndef MAME_MACHINE_SMARTMED_H
#define MAME_MACHINE_SMARTMED_H

#pragma once

#include "formats/imageutl.h"
#include "imagedev/memcard.h"
#include "machine/nandflash.h"

//#define SMARTMEDIA_IMAGE_SAVE


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class smartmedia_image_device : public nand_device, public device_memcard_image_interface
{
public:
	// construction/destruction
	smartmedia_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "sm_memc"; }
	virtual const char *file_extensions() const noexcept override { return "smc"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	// Because nand_device is a NVRAM device now, stub these to make it not do anything (read only)
	virtual void nvram_default() override { }
	virtual bool nvram_read(util::read_stream &file) override { return true; };
	virtual bool nvram_write(util::write_stream &file) override { return false; };

protected:
	virtual const software_list_loader &get_software_list_loader() const override;

	std::error_condition smartmedia_format_1();
	std::error_condition smartmedia_format_2();
	int detect_geometry(uint8_t id1, uint8_t id2);

	uint8_t m_mp_opcode;                  // multi-plane operation code
};


// device type definition
DECLARE_DEVICE_TYPE(SMARTMEDIA, smartmedia_image_device)

#endif // MAME_MACHINE_SMARTMED_H
