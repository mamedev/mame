// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Data East 'Rohga' era hardware

*************************************************************************/

#include "sound/okim6295.h"
#include "cpu/h6280/h6280.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "machine/deco146.h"
#include "machine/deco104.h"
#include "emupal.h"

class rohga_state : public driver_device
{
public:
	rohga_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ioprot(*this, "ioprot"),
		m_decocomn(*this, "deco_common"),
		m_deco_tilegen(*this, "tilegen%u", 1),
		m_oki(*this, "oki%u", 1),
		m_spriteram(*this, "spriteram%u", 1),
		m_pf_rowscroll(*this, "pf%u_rowscroll", 1),
		m_sprgen(*this, "spritegen%u", 1),
		m_palette(*this, "palette")
	{ }

	void wizdfire(machine_config &config);
	void nitrobal(machine_config &config);
	void hangzo(machine_config &config);
	void schmeisr(machine_config &config);
	void rohga(machine_config &config);

	void init_wizdfire();
	void init_nitrobal();
	void init_schmeisr();
	void init_hangzo();
	void init_rohga();

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<deco_146_base_device> m_ioprot;
	required_device<decocomn_device> m_decocomn;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device_array<okim6295_device, 2> m_oki;
	optional_device_array<buffered_spriteram16_device, 2> m_spriteram;

	/* memory pointers */
	optional_shared_ptr_array<uint16_t, 4> m_pf_rowscroll;

	optional_device_array<decospr_device, 2> m_sprgen;

	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER(rohga_irq_ack_r);
	DECLARE_WRITE16_MEMBER(wizdfire_irq_ack_w);
	DECLARE_WRITE16_MEMBER(rohga_buffer_spriteram16_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);

	DECLARE_VIDEO_START(wizdfire);
	uint32_t screen_update_rohga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wizdfire(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_nitrobal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mixwizdfirelayer(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t pri, uint16_t primask);
	void mixnitroballlayer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(rohga_pri_callback);
	DECOSPR_COLOUR_CB_MEMBER(rohga_col_callback);
	DECOSPR_COLOUR_CB_MEMBER(schmeisr_col_callback);

	READ16_MEMBER( ioprot_r );
	WRITE16_MEMBER( ioprot_w );
	void hangzo_map(address_map &map);
	void hotb_base_map(address_map &map);
	void nitrobal_map(address_map &map);
	void rohga_map(address_map &map);
	void sound_map(address_map &map);
	void schmeisr_map(address_map &map);
	void wizdfire_map(address_map &map);
};
