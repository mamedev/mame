#include "machine/i8255.h"

class system1_state : public driver_device
{
public:
	system1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ppi8255(*this, "ppi8255"),
		m_ram(*this, "ram"),
		m_spriteram(*this, "spriteram"),
		m_nob_mcu_latch(*this, "nob_mcu_latch"),
		m_nob_mcu_status(*this, "nob_mcu_status"){ }

	optional_device<i8255_device>  m_ppi8255;
	required_shared_ptr<UINT8> m_ram;
	required_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_nob_mcu_latch;
	optional_shared_ptr<UINT8> m_nob_mcu_status;

	UINT8 *m_videoram;
	void (*m_videomode_custom)(running_machine &machine, UINT8 data, UINT8 prevdata);
	UINT8 m_mute_xor;
	UINT8 m_dakkochn_mux_data;
	UINT8 m_videomode_prev;
	UINT8 m_mcu_control;
	UINT8 m_nob_maincpu_latch;
	int m_nobb_inport23_step;
	UINT8 *m_mix_collide;
	UINT8 m_mix_collide_summary;
	UINT8 *m_sprite_collide;
	UINT8 m_sprite_collide_summary;
	bitmap_ind16 *m_sprite_bitmap;
	UINT8 m_video_mode;
	UINT8 m_videoram_bank;
	tilemap_t *m_tilemap_page[8];
	UINT8 m_tilemap_pages;

	DECLARE_WRITE8_MEMBER(videomode_w);
	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(soundport_w);
	DECLARE_WRITE8_MEMBER(mcu_control_w);
	DECLARE_WRITE8_MEMBER(mcu_io_w);
	DECLARE_READ8_MEMBER(mcu_io_r);
	DECLARE_WRITE8_MEMBER(nob_mcu_control_p2_w);
	DECLARE_READ8_MEMBER(nob_maincpu_latch_r);
	DECLARE_WRITE8_MEMBER(nob_maincpu_latch_w);
	DECLARE_READ8_MEMBER(nob_mcu_status_r);
	DECLARE_READ8_MEMBER(nobb_inport1c_r);
	DECLARE_READ8_MEMBER(nobb_inport22_r);
	DECLARE_READ8_MEMBER(nobb_inport23_r);
	DECLARE_WRITE8_MEMBER(nobb_outport24_w);
	DECLARE_READ8_MEMBER(nob_start_r);
	DECLARE_WRITE8_MEMBER(system1_videomode_w);
	DECLARE_READ8_MEMBER(system1_mixer_collision_r);
	DECLARE_WRITE8_MEMBER(system1_mixer_collision_w);
	DECLARE_WRITE8_MEMBER(system1_mixer_collision_reset_w);
	DECLARE_READ8_MEMBER(system1_sprite_collision_r);
	DECLARE_WRITE8_MEMBER(system1_sprite_collision_w);
	DECLARE_WRITE8_MEMBER(system1_sprite_collision_reset_w);
	DECLARE_READ8_MEMBER(system1_videoram_r);
	DECLARE_WRITE8_MEMBER(system1_videoram_w);
	DECLARE_WRITE8_MEMBER(system1_paletteram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(dakkochn_mux_data_r);
	DECLARE_CUSTOM_INPUT_MEMBER(dakkochn_mux_status_r);
	DECLARE_WRITE8_MEMBER(sound_control_w);
	DECLARE_DRIVER_INIT(nobb);
	DECLARE_DRIVER_INIT(wboy2);
	DECLARE_DRIVER_INIT(imsorry);
	DECLARE_DRIVER_INIT(pitfall2);
	DECLARE_DRIVER_INIT(dakkochn);
	DECLARE_DRIVER_INIT(bootleg);
	DECLARE_DRIVER_INIT(wboysys2);
	DECLARE_DRIVER_INIT(shtngmst);
	DECLARE_DRIVER_INIT(wboyo);
	DECLARE_DRIVER_INIT(swat);
	DECLARE_DRIVER_INIT(regulus);
	DECLARE_DRIVER_INIT(bank0c);
	DECLARE_DRIVER_INIT(blockgal);
	DECLARE_DRIVER_INIT(nob);
	DECLARE_DRIVER_INIT(mrviking);
	DECLARE_DRIVER_INIT(teddybb);
	DECLARE_DRIVER_INIT(flicky);
	DECLARE_DRIVER_INIT(bank44);
	DECLARE_DRIVER_INIT(myherok);
	DECLARE_DRIVER_INIT(wmatch);
	DECLARE_DRIVER_INIT(bank00);
	DECLARE_DRIVER_INIT(myheroj);
	DECLARE_DRIVER_INIT(ufosensi);
	DECLARE_DRIVER_INIT(nprinces);
	DECLARE_DRIVER_INIT(wbml);
	DECLARE_DRIVER_INIT(bootsys2);
	DECLARE_DRIVER_INIT(bullfgtj);
	DECLARE_DRIVER_INIT(wboy);
	DECLARE_DRIVER_INIT(hvymetal);
	DECLARE_DRIVER_INIT(gardiab);
	DECLARE_DRIVER_INIT(4dwarrio);
	DECLARE_DRIVER_INIT(choplift);
	DECLARE_DRIVER_INIT(seganinj);
	DECLARE_DRIVER_INIT(gardia);
	DECLARE_DRIVER_INIT(spatter);
	TILE_GET_INFO_MEMBER(tile_get_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_MACHINE_START(system2);
	DECLARE_VIDEO_START(system2);
};


/*----------- defined in video/system1.c -----------*/





DECLARE_WRITE8_DEVICE_HANDLER( system1_videoram_bank_w );



SCREEN_UPDATE_IND16( system1 );
SCREEN_UPDATE_IND16( system2 );
SCREEN_UPDATE_IND16( system2_rowscroll );
