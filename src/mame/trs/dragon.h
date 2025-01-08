// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    dragon.h

    Dragon Family

***************************************************************************/

#ifndef MAME_TRS_DRAGON_H
#define MAME_TRS_DRAGON_H

#pragma once

#include "coco12.h"

#include "imagedev/printer.h"
#include "video/mc6845.h"

#include "emupal.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PRINTER_TAG     "printer"


INPUT_PORTS_EXTERN( dragon );



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
	void dragon_mem(address_map &map) ATTR_COLD;

protected:
	virtual void pia1_pa_changed(uint8_t data) override;

	static void dragon_cart(device_slot_interface &device);

private:
	required_device<printer_image_device> m_printer;
};


// dragon64 has an ACIA chip
class dragon64_state : public dragon_state
{
public:
	dragon64_state(const machine_config &mconfig, device_type type, const char *tag)
		: dragon_state(mconfig, type, tag)
		, m_acia(*this, "acia")
		, m_rombank(*this, "rombank%u", 0U)
	{
	}

	void tanodr64(machine_config &config);
	void dragon64(machine_config &config);
	void tanodr64h(machine_config &config);
	void dragon64h(machine_config &config);

protected:
	void d64_rom0(address_map &map) ATTR_COLD;
	void d64_rom1(address_map &map) ATTR_COLD;
	void d64_io0(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void pia1_pb_changed(uint8_t data) override;
	void page_rom(bool romswitch);

private:
	required_device<mos6551_device> m_acia;
	required_memory_bank_array<2> m_rombank;
};


// dragon200e has a character generator
class dragon200e_state : public dragon64_state
{
public:
	dragon200e_state(const machine_config &mconfig, device_type type, const char *tag)
		: dragon64_state(mconfig, type, tag)
		, m_char_rom(*this, "chargen")
		, m_lk1(*this, "LK1")
	{
	}

	uint8_t sam_read(offs_t offset);
	MC6847_GET_CHARROM_MEMBER(char_rom_r);

	void dragon200e(machine_config &config);
private:
	required_memory_region m_char_rom;
	required_ioport m_lk1;
};


// d64plus has a HD6845 and character generator
class d64plus_state : public dragon64_state
{
public:
	d64plus_state(const machine_config &mconfig, device_type type, const char *tag)
		: dragon64_state(mconfig, type, tag)
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_plus_ram(*this, "plus_ram", 0x10000, ENDIANNESS_BIG)
		, m_video_ram(*this, "video_ram", 0x800, ENDIANNESS_BIG)
		, m_pram_bank(*this, "pram_bank")
		, m_vram_bank(*this, "vram_bank")
		, m_char_rom(*this, "chargen")
	{
	}

	uint8_t d64plus_6845_disp_r();
	void d64plus_bank_w(uint8_t data);
	MC6845_UPDATE_ROW(crtc_update_row);

	void d64plus(machine_config &config);
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<hd6845s_device> m_crtc;
	required_device<palette_device> m_palette;
	memory_share_creator<uint8_t> m_plus_ram;
	memory_share_creator<uint8_t> m_video_ram;
	memory_bank_creator m_pram_bank;
	memory_bank_creator m_vram_bank;
	required_memory_region m_char_rom;
};

#endif // MAME_TRS_DRAGON_H
