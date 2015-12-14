// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    GI Joe

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/k054539.h"
#include "video/k053251.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/kvideodac.h"

class gijoe_state : public driver_device
{
public:
	gijoe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k054539(*this, "k054539"),
		m_tilemap(*this, "tilemap"),
		m_sprites(*this, "sprites"),
		m_mixer(*this, "mixer"),
		m_videodac(*this, "videodac"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2") { }

	/* misc */
	uint16_t m_cur_control2;
	int m_cur_vblankirq;
	int m_cur_objdmairq;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device<k054156_054157_device> m_tilemap;
	required_device<k053246_053247_device> m_sprites;
	required_device<k053251_device> m_mixer;
	required_device<kvideodac_device> m_videodac;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	DECLARE_WRITE_LINE_MEMBER(vblankirq_w);
	DECLARE_WRITE_LINE_MEMBER(objdmairq_w);

	DECLARE_READ16_MEMBER(control2_r);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_WRITE16_MEMBER(sound_cmd_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(sound_status_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void videodac_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);
	void mixer_init(bitmap_ind16 **bitmaps);
	void mixer_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);

	void sprites_wiring(uint32_t output, uint16_t &color, uint16_t &attr);
};
