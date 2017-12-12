// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/*************************************************************************

    Metro Games

*************************************************************************/

#include "sound/okim6295.h"
#include "sound/ym2151.h"
#include "sound/es8712.h"
#include "video/k053936.h"
#include "video/imagetek_i4100.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "screen.h"

class metro_state : public driver_device
{
public:
	enum
	{
		TIMER_KARATOUR_IRQ,
		TIMER_MOUJA_IRQ
	};

	metro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_ymsnd(*this, "ymsnd"),
		m_essnd(*this, "essnd"),
		m_vdp(*this, "vdp"),
		m_vdp2(*this, "vdp2"),
		m_vdp3(*this, "vdp3"),
		m_k053936(*this, "k053936") ,
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_irq_enable(*this, "irq_enable"),
		m_irq_levels(*this, "irq_levels"),
		m_irq_vectors(*this, "irq_vectors"),
		m_input_sel(*this, "input_sel"),
		m_k053936_ram(*this, "k053936_ram")
	{ }

	DECLARE_READ16_MEMBER(metro_irq_cause_r);
	DECLARE_WRITE16_MEMBER(metro_irq_cause_w);
	DECLARE_WRITE16_MEMBER(mouja_irq_timer_ctrl_w);
	DECLARE_WRITE16_MEMBER(metro_soundlatch_w);
	DECLARE_READ16_MEMBER(metro_soundstatus_r);
	DECLARE_WRITE16_MEMBER(metro_soundstatus_w);
	DECLARE_WRITE8_MEMBER(metro_sound_rombank_w);
	DECLARE_WRITE8_MEMBER(daitorid_sound_rombank_w);
	DECLARE_READ8_MEMBER(metro_porta_r);
	DECLARE_WRITE8_MEMBER(metro_porta_w);
	DECLARE_WRITE8_MEMBER(metro_portb_w);
	DECLARE_WRITE8_MEMBER(daitorid_portb_w);
	DECLARE_WRITE16_MEMBER(metro_coin_lockout_1word_w);
	DECLARE_WRITE16_MEMBER(metro_coin_lockout_4words_w);
	DECLARE_READ16_MEMBER(balcube_dsw_r);
	DECLARE_READ16_MEMBER(gakusai_input_r);
	DECLARE_WRITE16_MEMBER(blzntrnd_sound_w);
	DECLARE_WRITE8_MEMBER(blzntrnd_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(puzzlet_irq_enable_w);
	DECLARE_WRITE16_MEMBER(puzzlet_portb_w);
	DECLARE_WRITE16_MEMBER(metro_k053936_w);
	DECLARE_CUSTOM_INPUT_MEMBER(custom_soundstatus_r);
	DECLARE_WRITE8_MEMBER(gakusai_oki_bank_hi_w);
	DECLARE_WRITE8_MEMBER(gakusai_oki_bank_lo_w);
	DECLARE_READ8_MEMBER(gakusai_eeprom_r);
	DECLARE_WRITE8_MEMBER(gakusai_eeprom_w);
	DECLARE_READ8_MEMBER(dokyusp_eeprom_r);
	DECLARE_WRITE8_MEMBER(dokyusp_eeprom_bit_w);
	DECLARE_WRITE8_MEMBER(dokyusp_eeprom_reset_w);
	DECLARE_WRITE8_MEMBER(mouja_sound_rombank_w);
	DECLARE_WRITE_LINE_MEMBER(vdp_blit_end_w);

	// vmetal
	DECLARE_WRITE8_MEMBER(vmetal_control_w);
	DECLARE_WRITE8_MEMBER(es8712_reset_w);
	DECLARE_WRITE_LINE_MEMBER(vmetal_es8712_irq);

	DECLARE_DRIVER_INIT(karatour);
	DECLARE_DRIVER_INIT(daitorid);
	DECLARE_DRIVER_INIT(blzntrnd);
	DECLARE_DRIVER_INIT(vmetal);
	DECLARE_DRIVER_INIT(mouja);
	DECLARE_DRIVER_INIT(balcube);
	DECLARE_DRIVER_INIT(gakusai);
	DECLARE_DRIVER_INIT(dharmak);
	DECLARE_DRIVER_INIT(puzzlet);
	DECLARE_DRIVER_INIT(metro);
	DECLARE_DRIVER_INIT(lastfortg);
	TILE_GET_INFO_MEMBER(metro_k053936_get_tile_info);
	TILE_GET_INFO_MEMBER(metro_k053936_gstrik2_get_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_gstrik2);
	DECLARE_VIDEO_START(blzntrnd);
	DECLARE_VIDEO_START(gstrik2);
	uint32_t screen_update_psac_vdp2_mix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(metro_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(metro_periodic_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(bangball_scanline);
	INTERRUPT_GEN_MEMBER(karatour_interrupt);
	INTERRUPT_GEN_MEMBER(puzzlet_interrupt);
	IRQ_CALLBACK_MEMBER(metro_irq_callback);
	DECLARE_READ_LINE_MEMBER(metro_rxd_r);

protected:
	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<device_t> m_ymsnd; // TODO set correct type
	optional_device<es8712_device> m_essnd;
	optional_device<imagetek_i4100_device> m_vdp;
	optional_device<imagetek_i4220_device> m_vdp2;
	optional_device<imagetek_i4300_device> m_vdp3;

	optional_device<k053936_device> m_k053936;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_irq_enable;
	optional_shared_ptr<uint16_t> m_irq_levels;
	optional_shared_ptr<uint16_t> m_irq_vectors;
	optional_shared_ptr<uint16_t> m_input_sel;
	optional_shared_ptr<uint16_t> m_k053936_ram;

	/* video-related */
	tilemap_t   *m_k053936_tilemap;

	/* irq_related */
	int         m_vblank_bit;
	int         m_blitter_bit;
	int         m_irq_line;
	uint8_t     m_requested_int[8];
	emu_timer   *m_mouja_irq_timer;
	emu_timer   *m_karatour_irq_timer;

	/* sound related */
	uint16_t      m_soundstatus;
	int         m_porta;
	int         m_portb;
	int         m_busy_sndcpu;
	bool        m_essnd_gate;

	/* misc */
	int         m_gakusai_oki_bank_lo;
	int         m_gakusai_oki_bank_hi;

	void update_irq_state();
	void metro_common();
	void gakusai_oki_bank_set();

	// blazing tornado
	bitmap_ind16 m_vdp_bitmap;
};
