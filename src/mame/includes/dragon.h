// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    dragon.h

    Dragon Family

***************************************************************************/

#pragma once

#ifndef __DRAGON__
#define __DRAGON__


#include "includes/coco12.h"
#include "imagedev/printer.h"
#include "machine/mos6551.h"
#include "video/mc6845.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PRINTER_TAG     "printer"
#define ACIA_TAG        "acia"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dragon_state : public coco12_state
{
public:
	dragon_state(const machine_config &mconfig, device_type type, const char *tag)
	: coco12_state(mconfig, type, tag),
		m_printer(*this, PRINTER_TAG)
	{
	}

	required_device<printer_image_device> m_printer;

protected:
	virtual void pia1_pa_changed(uint8_t data) override;
};


/* dragon64 has an ACIA chip */
class dragon64_state : public dragon_state
{
public:
	dragon64_state(const machine_config &mconfig, device_type type, const char *tag)
	: dragon_state(mconfig, type, tag),
		m_acia(*this, ACIA_TAG)
	{
	}

	required_device<mos6551_device> m_acia;

protected:
	virtual DECLARE_READ8_MEMBER( ff00_read ) override;
	virtual DECLARE_WRITE8_MEMBER( ff00_write ) override;

	virtual void pia1_pb_changed(uint8_t data) override;
	void page_rom(bool romswitch);
};


/* dragon200e has a character generator */
class dragon200e_state : public dragon64_state
{
public:
	dragon200e_state(const machine_config &mconfig, device_type type, const char *tag)
		: dragon64_state(mconfig, type, tag),
		m_char_rom(*this, "chargen")
	{
	}

	MC6847_GET_CHARROM_MEMBER(char_rom_r);

private:
	required_memory_region m_char_rom;
};


/* d64plus has a HD6845 and character generator */
class d64plus_state : public dragon64_state
{
public:
	d64plus_state(const machine_config &mconfig, device_type type, const char *tag)
		: dragon64_state(mconfig, type, tag),
		m_hd6845(*this, "hd6845"),
		m_videoram(*this, "videoram"),
		m_char_rom(*this, "chargen")
	{
	}

	MC6847_GET_CHARROM_MEMBER(char_rom_r);

private:
	required_device<hd6845_device> m_hd6845;
	required_memory_region m_videoram;
	required_memory_region m_char_rom;
};

#endif /* __DRAGON__ */
