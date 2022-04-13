// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/svision.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_SVISION_H
#define MAME_INCLUDES_SVISION_H

#pragma once

#include "cpu/m6502/m65c02.h"
#include "machine/timer.h"
#include "audio/svis_snd.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "emupal.h"

struct tvlink_t
{
	uint32_t palette[4/*0x40?*/]; /* rgb8 */
	int palette_on;
};

class svision_state : public driver_device
{
public:
	svision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sound(*this, "custom")
		, m_cart(*this, "cartslot")
		, m_reg(*this, "reg")
		, m_videoram(*this, "videoram")
		, m_screen(*this, "screen")
		, m_joy(*this, "JOY")
		, m_joy2(*this, "JOY2")
		, m_palette(*this, "palette")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
	{ }

	void svisionp(machine_config &config);
	void svisions(machine_config &config);
	void tvlinkp(machine_config &config);
	void svision(machine_config &config);
	void svisionn(machine_config &config);
	void svision_base(machine_config &config);

	void init_svisions();
	void init_svision();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	struct svision_t
	{
		emu_timer *timer1 = nullptr;
		int timer_shot = 0;
	};

	struct svision_pet_t
	{
		int state = 0;
		int on = 0, clock = 0, data = 0;
		uint8_t input = 0;
		emu_timer *timer = nullptr;
	};

	DECLARE_WRITE_LINE_MEMBER(sound_irq_w);
	uint8_t svision_r(offs_t offset);
	void svision_w(offs_t offset, uint8_t data);
	uint8_t tvlink_r(offs_t offset);
	void tvlink_w(offs_t offset, uint8_t data);

	uint32_t screen_update_svision(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tvlink(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(frame_int_w);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void svision_palette(palette_device &palette) const;
	void svisionp_palette(palette_device &palette) const;
	void svisionn_palette(palette_device &palette) const;
	DECLARE_MACHINE_RESET(tvlink);

	enum
	{
		XSIZE = 0x00,
		XPOS  = 0x02,
		YPOS  = 0x03,
		BANK  = 0x26,
	};

	void check_irq();

	TIMER_CALLBACK_MEMBER(svision_pet_timer);
	TIMER_CALLBACK_MEMBER(svision_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(svision_pet_timer_dev);

	void svision_mem(address_map &map);
	void tvlink_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<svision_sound_device> m_sound;
	required_device<generic_slot_device> m_cart;
	required_shared_ptr<uint8_t> m_reg;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<screen_device> m_screen;
	required_ioport m_joy;
	optional_ioport m_joy2;
	required_device<palette_device> m_palette;

	required_memory_bank m_bank1;
	required_memory_bank m_bank2;

	memory_region *m_cart_rom = nullptr;

	svision_t m_svision;
	svision_pet_t m_pet;
	tvlink_t m_tvlink;
	bool m_dma_finished = false;
};

#endif // MAME_INCLUDES_SVISION_H
