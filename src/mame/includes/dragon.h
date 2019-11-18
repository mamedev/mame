// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    dragon.h

    Dragon Family

***************************************************************************/

#ifndef MAME_INCLUDES_DRAGON_H
#define MAME_INCLUDES_DRAGON_H

#pragma once


#include "includes/coco12.h"
#include "imagedev/printer.h"
#include "machine/mos6551.h"
#include "video/mc6845.h"
#include "emupal.h"


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
		: coco12_state(mconfig, type, tag)
		, m_printer(*this, PRINTER_TAG)
	{
	}

	void dragon_base(machine_config &config);
	void dragon32(machine_config &config);
	void dragon_mem(address_map &map);
protected:
	virtual void pia1_pa_changed(uint8_t data) override;

private:
	required_device<printer_image_device> m_printer;
};


// dragon64 has an ACIA chip
class dragon64_state : public dragon_state
{
public:
	dragon64_state(const machine_config &mconfig, device_type type, const char *tag)
		: dragon_state(mconfig, type, tag)
		, m_acia(*this, ACIA_TAG)
	{
	}

	void tanodr64(machine_config &config);
	void dragon64(machine_config &config);
	void tanodr64h(machine_config &config);
	void dragon64h(machine_config &config);
protected:
	virtual DECLARE_READ8_MEMBER( ff00_read ) override;
	virtual DECLARE_WRITE8_MEMBER( ff00_write ) override;

	virtual void pia1_pb_changed(uint8_t data) override;
	void page_rom(bool romswitch);

private:
	required_device<mos6551_device> m_acia;
};


// dragon200e has a character generator
class dragon200e_state : public dragon64_state
{
public:
	dragon200e_state(const machine_config &mconfig, device_type type, const char *tag)
		: dragon64_state(mconfig, type, tag)
		, m_char_rom(*this, "chargen")
	{
	}

	MC6847_GET_CHARROM_MEMBER(char_rom_r);

	void dragon200e(machine_config &config);
private:
	required_memory_region m_char_rom;
};


// d64plus has a HD6845 and character generator
class d64plus_state : public dragon64_state
{
public:
	d64plus_state(const machine_config &mconfig, device_type type, const char *tag)
		: dragon64_state(mconfig, type, tag)
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_plus_ram(*this, "plus_ram")
		, m_video_ram(*this, "video_ram")
		, m_char_rom(*this, "chargen")
	{
	}

	DECLARE_READ8_MEMBER(d64plus_6845_disp_r);
	DECLARE_WRITE8_MEMBER(d64plus_bank_w);
	MC6845_UPDATE_ROW(crtc_update_row);

	void d64plus(machine_config &config);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<hd6845s_device> m_crtc;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_plus_ram;
	optional_shared_ptr<uint8_t> m_video_ram;
	required_memory_region m_char_rom;
};

#endif // MAME_INCLUDES_DRAGON_H
