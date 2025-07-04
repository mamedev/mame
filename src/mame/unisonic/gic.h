// license:BSD-3-Clause
// copyright-holders:David Viens
/***************************************************************************

    gic.h

   GI AY-3-8800-1 (Datasheet exists as AY-3-8500-1 Graphics Interface Chip)
   For the GIMINI "Challenger" programmable game system.

   Really only ever used in the Unisonic Champion 2711

***************************************************************************/

#ifndef MAME_UNISONIC_GIC_H
#define MAME_UNISONIC_GIC_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> gic_device

//Palette entries
#define GIC_BLACK 0
#define GIC_RED   1
#define GIC_GREEN 2
#define GIC_WHITE 3

#define GIC_CHAR_W 6
#define GIC_CHAR_H 8

#define GIC_LEFT_H 12
#define GIC_LEFT_W 6

#define GIC_RIGHT_H 6
#define GIC_RIGHT_W 13

class gic_device :  public device_t
					, public device_sound_interface
					, public device_video_interface
{
public:
	// construction/destruction
	gic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto ram_callback() { return m_ram.bind(); }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	inline bitmap_ind16 *get_bitmap() { return &m_bitmap; }

	// Global constants (non measured figures)
	static constexpr int START_ACTIVE_SCAN = 10;
	static constexpr int BORDER_SIZE       = GIC_CHAR_W*3;
	static constexpr int END_ACTIVE_SCAN   = 10 + GIC_CHAR_W*2 + 150 + GIC_CHAR_W*2;
	static constexpr int START_Y           = 1;
	static constexpr int SCREEN_HEIGHT     = GIC_CHAR_H*(GIC_LEFT_H+2);
	static constexpr int LINE_CLOCKS       = 455;
	static constexpr int LINES             = 262;

protected:
	gic_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	/* timers */
	TIMER_CALLBACK_MEMBER(vblank_tick);

	void draw_char_left (int x, int y, uint8_t code, bitmap_ind16 &bitmap);
	void draw_char_right(int x, int y, uint8_t code, bitmap_ind16 &bitmap,int bg_col);

	bitmap_ind16 m_bitmap;
	required_region_ptr<uint8_t> m_cgrom;          // internal chargen ROM

	emu_timer    *m_vblank_timer = nullptr;
	sound_stream *m_stream = nullptr;

	int m_audiocnt;
	int m_audioval;
	int m_audioreset;
	devcb_read8 m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(GIC, gic_device)

#endif  // MAME_UNISONIC_GIC_H
