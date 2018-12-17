// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarivad.h

    Atari VAD video controller device.

***************************************************************************/

#ifndef MAME_VIDEO_ATARIVAD_H
#define MAME_VIDEO_ATARIVAD_H

#include "video/atarimo.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ATARI_VAD_ADD(_tag, _screen, _intcb) \
	MCFG_DEVICE_ADD(_tag, ATARI_VAD, 0) \
	MCFG_VIDEO_SET_SCREEN(_screen) \
	downcast<atari_vad_device &>(*device).set_scanline_int_cb(DEVCB_##_intcb);

#define MCFG_ATARI_VAD_PLAYFIELD(_class, _gfxtag, _getinfo) \
	{ std::string fulltag(device->tag()); fulltag.append(":playfield"); device_t *device; \
	MCFG_TILEMAP_ADD(fulltag.c_str()) \
	MCFG_TILEMAP_GFXDECODE(_gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(2) \
	MCFG_TILEMAP_INFO_CB_DEVICE(DEVICE_SELF_OWNER, _class, _getinfo) \
	MCFG_TILEMAP_TILE_SIZE(8,8) \
	MCFG_TILEMAP_LAYOUT_STANDARD(SCAN_COLS, 64,64) }

#define MCFG_ATARI_VAD_PLAYFIELD2(_class, _gfxtag, _getinfo) \
	{ std::string fulltag(device->tag()); fulltag.append(":playfield2"); device_t *device; \
	MCFG_TILEMAP_ADD(fulltag.c_str()) \
	MCFG_TILEMAP_GFXDECODE(_gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(2) \
	MCFG_TILEMAP_INFO_CB_DEVICE(DEVICE_SELF_OWNER, _class, _getinfo) \
	MCFG_TILEMAP_TILE_SIZE(8,8) \
	MCFG_TILEMAP_LAYOUT_STANDARD(SCAN_COLS, 64,64) \
	MCFG_TILEMAP_TRANSPARENT_PEN(0) }

#define MCFG_ATARI_VAD_ALPHA(_class, _gfxtag, _getinfo) \
	{ std::string fulltag(device->tag()); fulltag.append(":alpha"); device_t *device; \
	MCFG_TILEMAP_ADD(fulltag.c_str()) \
	MCFG_TILEMAP_GFXDECODE(_gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(2) \
	MCFG_TILEMAP_INFO_CB_DEVICE(DEVICE_SELF_OWNER, _class, _getinfo) \
	MCFG_TILEMAP_TILE_SIZE(8,8) \
	MCFG_TILEMAP_LAYOUT_STANDARD(SCAN_ROWS, 64,32) \
	MCFG_TILEMAP_TRANSPARENT_PEN(0) }

#define MCFG_ATARI_VAD_MOB(_config, _gfxtag) \
	{ std::string fulltag(device->tag()); fulltag.append(":mob"); \
	ATARI_MOTION_OBJECTS(config, fulltag.c_str(), 0, "screen", _config).set_gfxdecode(_gfxtag); }



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
	atari_vad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template<class Object> devcb_base &set_scanline_int_cb(Object &&cb) { return m_scanline_int_cb.set_callback(std::forward<Object>(cb)); }

	// getters
	tilemap_device &alpha() const { return *m_alpha_tilemap; }
	tilemap_device &playfield() const { return *m_playfield_tilemap; }
	tilemap_device &playfield2() const { return *m_playfield2_tilemap; }
	atari_motion_objects_device &mob() const { return *m_mob; }

	// read/write handlers
	DECLARE_READ16_MEMBER(control_read);
	DECLARE_WRITE16_MEMBER(control_write);

	// playfield/alpha tilemap helpers
	DECLARE_WRITE16_MEMBER(alpha_w);
	DECLARE_WRITE16_MEMBER(playfield_upper_w);
	DECLARE_WRITE16_MEMBER(playfield_latched_lsb_w);
	DECLARE_WRITE16_MEMBER(playfield_latched_msb_w);
	DECLARE_WRITE16_MEMBER(playfield2_latched_msb_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// timer IDs
	enum
	{
		TID_SCANLINE_INT,
		TID_TILEROW_UPDATE,
		TID_EOF
	};

	// internal helpers
	void internal_control_write(offs_t offset, uint16_t newword);
	void update_pf_xscrolls();
	void update_parameter(uint16_t newword);
	void update_tilerow(emu_timer &timer, int scanline);
	void eof_update(emu_timer &timer);

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
};


#endif // MAME_VIDEO_ATARIVAD_H
