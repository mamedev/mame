// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Rock'n Rage

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/vlm5030.h"
#include "video/k007342.h"
#include "video/k007420.h"

class rockrage_state : public driver_device
{
public:
	rockrage_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007342(*this, "k007342"),
		m_k007420(*this, "k007420"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_rombank(*this, "rombank") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007342_device> m_k007342;
	required_device<k007420_device> m_k007420;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_memory_bank m_rombank;

	/* video-related */
	int        m_vreg;

	void rockrage_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rockrage_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rockrage_vreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rockrage_VLM5030_busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rockrage_speech_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette_init_rockrage(palette_device &palette);
	uint32_t screen_update_rockrage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rockrage_interrupt(device_t &device);
	K007342_CALLBACK_MEMBER(rockrage_tile_callback);
	K007420_CALLBACK_MEMBER(rockrage_sprite_callback);

};
