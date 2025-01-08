// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarivad.h

    Atari VAD video controller device.

***************************************************************************/

#ifndef MAME_ATARI_ATARIVAD_H
#define MAME_ATARI_ATARIVAD_H

#include "atarimo.h"
#include "tilemap.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> atari_vad_device

// device type definition
DECLARE_DEVICE_TYPE(ATARI_VAD, atari_vad_device)

class atari_vad_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	template <typename T>
	atari_vad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: atari_vad_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}
	atari_vad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto scanline_int_cb() { return m_scanline_int_cb.bind(); }
	void set_xoffsets(int xoffset, int xoffset2) { m_pf_xoffset = xoffset; m_pf2_xoffset = xoffset2; }

	// getters
	tilemap_device &alpha() const { return *m_alpha_tilemap; }
	tilemap_device &playfield() const { return *m_playfield_tilemap; }
	tilemap_device &playfield2() const { return *m_playfield2_tilemap; }
	atari_motion_objects_device &mob() const { return *m_mob; }

	// read/write handlers
	uint16_t control_read(offs_t offset);
	void control_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// playfield/alpha tilemap helpers
	void alpha_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void playfield_upper_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void playfield_latched_lsb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void playfield_latched_msb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void playfield2_latched_msb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	void internal_control_write(offs_t offset, uint16_t newword);
	void update_pf_xscrolls();
	void update_parameter(uint16_t newword);
	TIMER_CALLBACK_MEMBER(scanline_int);
	TIMER_CALLBACK_MEMBER(update_tilerow);
	TIMER_CALLBACK_MEMBER(eof_update);

	// configuration state
	devcb_write_line   m_scanline_int_cb;

	// internal state
	optional_device<tilemap_device> m_alpha_tilemap;
	required_device<tilemap_device> m_playfield_tilemap;
	optional_device<tilemap_device> m_playfield2_tilemap;
	optional_device<atari_motion_objects_device> m_mob;
	optional_shared_ptr<uint16_t> m_eof_data;

	emu_timer *         m_scanline_int_timer;
	emu_timer *         m_tilerow_update_timer;
	emu_timer *         m_eof_timer;

	uint32_t              m_palette_bank;            // which palette bank is enabled
	//uint32_t              m_pf0_xscroll;             // playfield 1 xscroll
	uint32_t              m_pf0_xscroll_raw;         // playfield 1 xscroll raw value
	uint32_t              m_pf0_yscroll;             // playfield 1 yscroll
	uint32_t              m_pf1_xscroll_raw;         // playfield 2 xscroll raw value
	uint32_t              m_pf1_yscroll;             // playfield 2 yscroll
	uint32_t              m_mo_xscroll;              // sprite xscroll
	uint32_t              m_mo_yscroll;              // sprite xscroll

	uint16_t              m_control[0x40/2];          // control data

	int m_pf_xoffset;
	int m_pf2_xoffset;
};


#endif // MAME_ATARI_ATARIVAD_H
