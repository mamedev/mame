// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

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
	void galaxygn_sound_irq(int state);
	void fantland_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fantland_nmi_enable_16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fantland_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fantland_soundlatch_16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t spriteram_16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t spriteram2_16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void spriteram_16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void spriteram2_16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void borntofi_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t borntofi_inputs_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void borntofi_msm5205_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value wheelrun_wheel_r(ioport_field &field, void *param);
	void machine_start_fantland();
	void machine_reset_fantland();
	void machine_start_borntofi();
	void machine_reset_borntofi();
	uint32_t screen_update_fantland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fantland_irq(device_t &device);
	void fantland_sound_irq(device_t &device);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void borntofi_adpcm_int_0(int state);
	void borntofi_adpcm_int_1(int state);
	void borntofi_adpcm_int_2(int state);
	void borntofi_adpcm_int_3(int state);
	void borntofi_adpcm_start( msm5205_device *device, int voice );
	void borntofi_adpcm_stop( msm5205_device *device, int voice );
	void borntofi_adpcm_int( msm5205_device *device, int voice );
};
