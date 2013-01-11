/*************************************************************************

    Atari Triple Hunt hardware

*************************************************************************/

#include "sound/discrete.h"
#include "sound/samples.h"


/* Discrete Sound Input Nodes */
#define TRIPLHNT_BEAR_ROAR_DATA NODE_01
#define TRIPLHNT_BEAR_EN        NODE_02
#define TRIPLHNT_SHOT_DATA      NODE_03
#define TRIPLHNT_SCREECH_EN     NODE_04
#define TRIPLHNT_LAMP_EN        NODE_05


class triplhnt_state : public driver_device
{
public:
	triplhnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_playfield_ram(*this, "playfield_ram"),
		m_vpos_ram(*this, "vpos_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_orga_ram(*this, "orga_ram"),
		m_code_ram(*this, "code_ram"){ }

	UINT8 m_cmos[16];
	UINT8 m_da_latch;
	UINT8 m_misc_flags;
	UINT8 m_cmos_latch;
	UINT8 m_hit_code;
	required_shared_ptr<UINT8> m_playfield_ram;
	required_shared_ptr<UINT8> m_vpos_ram;
	required_shared_ptr<UINT8> m_hpos_ram;
	required_shared_ptr<UINT8> m_orga_ram;
	required_shared_ptr<UINT8> m_code_ram;
	int m_sprite_zoom;
	int m_sprite_bank;
	bitmap_ind16 m_helper;
	tilemap_t* m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(triplhnt_misc_w);
	DECLARE_READ8_MEMBER(triplhnt_cmos_r);
	DECLARE_READ8_MEMBER(triplhnt_input_port_4_r);
	DECLARE_READ8_MEMBER(triplhnt_misc_r);
	DECLARE_READ8_MEMBER(triplhnt_da_latch_r);
	DECLARE_DRIVER_INIT(triplhnt);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_triplhnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(triplhnt_hit_callback);
};


/*----------- defined in drivers/triplhnt.c -----------*/

void triplhnt_set_collision(running_machine &machine, int data);

/*----------- defined in audio/triplhnt.c -----------*/

DISCRETE_SOUND_EXTERN( triplhnt );
extern const samples_interface triplhnt_samples_interface;
