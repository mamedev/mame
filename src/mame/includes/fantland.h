// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "screen.h"

class fantland_state : public driver_device
{
public:
	fantland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_msm3(*this, "msm3"),
		m_msm4(*this, "msm4"),
		m_spriteram(*this, "spriteram", 0),
		m_spriteram2(*this, "spriteram2", 0)  { }

	void fantland(machine_config &config);
	void wheelrun(machine_config &config);
	void borntofi(machine_config &config);
	void galaxygn(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(wheelrun_wheel_r);

private:
	/* memory pointers */
//  uint8_t *    m_spriteram;   // currently directly used in a 16bit map...
//  uint8_t *    m_spriteram_2; // currently directly used in a 16bit map...

	/* misc */
	uint8_t      m_nmi_enable;
	int        m_old_x[2];
	int        m_old_y[2];
	int        m_old_f[2];
	uint8_t      m_input_ret[2];
	int        m_adpcm_playing[4];
	int        m_adpcm_addr[2][4];
	int        m_adpcm_nibble[4];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<msm5205_device> m_msm1;
	optional_device<msm5205_device> m_msm2;
	optional_device<msm5205_device> m_msm3;
	optional_device<msm5205_device> m_msm4;

	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram2;
	DECLARE_WRITE_LINE_MEMBER(galaxygn_sound_irq);
	DECLARE_WRITE8_MEMBER(fantland_nmi_enable_w);
	DECLARE_WRITE16_MEMBER(fantland_nmi_enable_16_w);
	DECLARE_WRITE8_MEMBER(fantland_soundlatch_w);
	DECLARE_WRITE16_MEMBER(fantland_soundlatch_16_w);
	DECLARE_READ16_MEMBER(spriteram_16_r);
	DECLARE_READ16_MEMBER(spriteram2_16_r);
	DECLARE_WRITE16_MEMBER(spriteram_16_w);
	DECLARE_WRITE16_MEMBER(spriteram2_16_w);
	DECLARE_WRITE8_MEMBER(borntofi_nmi_enable_w);
	DECLARE_READ8_MEMBER(borntofi_inputs_r);
	DECLARE_WRITE8_MEMBER(borntofi_msm5205_w);
	DECLARE_MACHINE_START(fantland);
	DECLARE_MACHINE_RESET(fantland);
	DECLARE_MACHINE_START(borntofi);
	DECLARE_MACHINE_RESET(borntofi);
	uint32_t screen_update_fantland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(fantland_irq);
	INTERRUPT_GEN_MEMBER(fantland_sound_irq);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(borntofi_adpcm_int_0);
	DECLARE_WRITE_LINE_MEMBER(borntofi_adpcm_int_1);
	DECLARE_WRITE_LINE_MEMBER(borntofi_adpcm_int_2);
	DECLARE_WRITE_LINE_MEMBER(borntofi_adpcm_int_3);
	void borntofi_adpcm_start( msm5205_device *device, int voice );
	void borntofi_adpcm_stop( msm5205_device *device, int voice );
	void borntofi_adpcm_int( msm5205_device *device, int voice );
	void borntofi_map(address_map &map);
	void borntofi_sound_map(address_map &map);
	void fantland_map(address_map &map);
	void fantland_sound_iomap(address_map &map);
	void fantland_sound_map(address_map &map);
	void galaxygn_map(address_map &map);
	void galaxygn_sound_iomap(address_map &map);
	void wheelrun_map(address_map &map);
	void wheelrun_sound_map(address_map &map);
};
