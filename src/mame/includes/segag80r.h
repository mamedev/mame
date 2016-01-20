// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega G-80 raster hardware

*************************************************************************/
#include "sound/samples.h"
#include "machine/segag80.h"
#include "sound/sn76496.h"
#include "audio/segasnd.h"


class sega005_sound_device;

class segag80r_state : public driver_device
{
public:
	enum
	{
		TIMER_VBLANK_LATCH_CLEAR
	};

	segag80r_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_samples(*this, "samples"),
		m_speech(*this, "segaspeech"),
		m_usbsnd(*this, "usbsnd"),
		m_005snd(*this, "005"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_shared_ptr<UINT8> m_mainram;
	required_shared_ptr<UINT8> m_videoram;

	optional_device<sn76496_device> m_sn1;
	optional_device<sn76496_device> m_sn2;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<samples_device> m_samples;
	optional_device<speech_sound_device> m_speech;
	optional_device<usb_sound_device> m_usbsnd;
	optional_device<sega005_sound_device> m_005snd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	std::vector<UINT8> m_paletteram;

	UINT8 m_sound_state[2];
	UINT8 m_sound_rate;
	UINT16 m_sound_addr;
	UINT8 m_sound_data;
	UINT8 m_square_state;
	UINT8 m_square_count;
	UINT8 m_n7751_command;
	UINT8 m_n7751_busy;
	segag80_decrypt_func m_decrypt;
	UINT8 m_background_pcb;
	double m_rweights[3];
	double m_gweights[3];
	double m_bweights[2];
	UINT8 m_video_control;
	UINT8 m_video_flip;
	UINT8 m_vblank_latch;
	tilemap_t *m_spaceod_bg_htilemap;
	tilemap_t *m_spaceod_bg_vtilemap;
	UINT16 m_spaceod_hcounter;
	UINT16 m_spaceod_vcounter;
	UINT8 m_spaceod_fixed_color;
	UINT8 m_spaceod_bg_control;
	UINT8 m_spaceod_bg_detect;
	tilemap_t *m_bg_tilemap;
	UINT8 m_bg_enable;
	UINT8 m_bg_char_bank;
	UINT16 m_bg_scrollx;
	UINT16 m_bg_scrolly;
	UINT8 m_pignewt_bg_color_offset;
	DECLARE_WRITE8_MEMBER(mainram_w);
	DECLARE_WRITE8_MEMBER(vidram_w);
	DECLARE_WRITE8_MEMBER(monsterb_vidram_w);
	DECLARE_WRITE8_MEMBER(pignewt_vidram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_vidram_w);
	DECLARE_READ8_MEMBER(mangled_ports_r);
	DECLARE_READ8_MEMBER(spaceod_mangled_ports_r);
	DECLARE_READ8_MEMBER(spaceod_port_fc_r);
	DECLARE_WRITE8_MEMBER(coin_count_w);
	DECLARE_WRITE8_MEMBER(segag80r_videoram_w);
	DECLARE_READ8_MEMBER(segag80r_video_port_r);
	DECLARE_WRITE8_MEMBER(segag80r_video_port_w);
	DECLARE_READ8_MEMBER(spaceod_back_port_r);
	DECLARE_WRITE8_MEMBER(spaceod_back_port_w);
	DECLARE_WRITE8_MEMBER(monsterb_videoram_w);
	DECLARE_WRITE8_MEMBER(monsterb_back_port_w);
	DECLARE_WRITE8_MEMBER(pignewt_videoram_w);
	DECLARE_WRITE8_MEMBER(pignewt_back_color_w);
	DECLARE_WRITE8_MEMBER(pignewt_back_port_w);
	DECLARE_WRITE8_MEMBER(sindbadm_videoram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_back_port_w);
	DECLARE_WRITE8_MEMBER(astrob_sound_w);
	DECLARE_WRITE8_MEMBER(spaceod_sound_w);
	DECLARE_READ8_MEMBER(n7751_rom_r);
	DECLARE_READ8_MEMBER(n7751_command_r);
	DECLARE_READ8_MEMBER(n7751_t1_r);
	DECLARE_INPUT_CHANGED_MEMBER(service_switch);
	DECLARE_WRITE8_MEMBER(usb_ram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_soundport_w);
	DECLARE_WRITE8_MEMBER(sindbadm_misc_w);
	DECLARE_WRITE8_MEMBER(sindbadm_sn1_SN76496_w);
	DECLARE_WRITE8_MEMBER(sindbadm_sn2_SN76496_w);
	DECLARE_DRIVER_INIT(spaceod);
	DECLARE_DRIVER_INIT(sindbadm);
	DECLARE_DRIVER_INIT(pignewt);
	DECLARE_DRIVER_INIT(monsterb);
	DECLARE_DRIVER_INIT(005);
	DECLARE_DRIVER_INIT(monster2);
	DECLARE_DRIVER_INIT(astrob);
	TILE_GET_INFO_MEMBER(spaceod_get_tile_info);
	TILEMAP_MAPPER_MEMBER(spaceod_scan_rows);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	UINT32 screen_update_segag80r(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(segag80r_vblank_start);
	INTERRUPT_GEN_MEMBER(sindbadm_vblank_start);
	DECLARE_WRITE8_MEMBER(sega005_sound_a_w);
	DECLARE_WRITE8_MEMBER(sega005_sound_b_w);
	inline void sega005_update_sound_data();
	DECLARE_WRITE8_MEMBER(monsterb_sound_a_w);
	DECLARE_WRITE8_MEMBER(monsterb_sound_b_w);
	DECLARE_READ8_MEMBER(n7751_status_r);
	DECLARE_WRITE8_MEMBER(n7751_command_w);
	DECLARE_WRITE8_MEMBER(n7751_rom_control_w);
	DECLARE_WRITE8_MEMBER(n7751_p2_w);
	void vblank_latch_set();
	void g80_set_palette_entry(int entry, UINT8 data);
	void spaceod_bg_init_palette();
	void draw_videoram(bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *transparent_pens);
	void draw_background_spaceod(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background_page_scroll(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background_full_scroll(bitmap_ind16 &bitmap, const rectangle &cliprect);
	offs_t decrypt_offset(address_space &space, offs_t offset);
	inline UINT8 demangle(UINT8 d7d6, UINT8 d5d4, UINT8 d3d2, UINT8 d1d0);
	void monsterb_expand_gfx(const char *region);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


/*----------- defined in audio/segag80r.c -----------*/


class sega005_sound_device : public device_t,
									public device_sound_interface
{
public:
	sega005_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	emu_timer *m_sega005_sound_timer;
	sound_stream *m_sega005_stream;

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	TIMER_CALLBACK_MEMBER( sega005_auto_timer );
};

extern const device_type SEGA005;


MACHINE_CONFIG_EXTERN( astrob_sound_board );
MACHINE_CONFIG_EXTERN( 005_sound_board );
MACHINE_CONFIG_EXTERN( spaceod_sound_board );
MACHINE_CONFIG_EXTERN( monsterb_sound_board );

/*----------- defined in video/segag80r.c -----------*/

#define G80_BACKGROUND_NONE         0
#define G80_BACKGROUND_SPACEOD      1
#define G80_BACKGROUND_MONSTERB     2
#define G80_BACKGROUND_PIGNEWT      3
#define G80_BACKGROUND_SINDBADM     4
