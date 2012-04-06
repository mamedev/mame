#include "machine/6821pia.h"

struct counter_state
{
	UINT8			control;
	UINT16			latch;
	UINT16			count;
	emu_timer *		timer;
	UINT8			timer_active;
	attotime		period;
};

class mcr68_state : public driver_device
{
public:
	mcr68_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	UINT16 m_control_word;
	UINT8 m_protection_data[5];
	attotime m_timing_factor;
	UINT8 m_sprite_clip;
	INT8 m_sprite_xoffset;
	UINT8 m_m6840_status;
	UINT8 m_m6840_status_read_since_int;
	UINT8 m_m6840_msb_buffer;
	UINT8 m_m6840_lsb_buffer;
	UINT8 m_m6840_irq_state;
	UINT8 m_m6840_irq_vector;
	struct counter_state m_m6840_state[3];
	UINT8 m_v493_irq_state;
	UINT8 m_v493_irq_vector;
	timer_expired_func m_v493_callback;
	const char *m_v493_callback_name;
	UINT8 m_zwackery_sound_data;
	attotime m_m6840_counter_periods[3];
	attotime m_m6840_internal_counter_period;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_READ16_MEMBER(zwackery_6840_r);
	DECLARE_WRITE16_MEMBER(xenophobe_control_w);
	DECLARE_WRITE16_MEMBER(blasted_control_w);
	DECLARE_READ16_MEMBER(spyhunt2_port_0_r);
	DECLARE_READ16_MEMBER(spyhunt2_port_1_r);
	DECLARE_WRITE16_MEMBER(spyhunt2_control_w);
	DECLARE_READ16_MEMBER(archrivl_port_1_r);
	DECLARE_WRITE16_MEMBER(archrivl_control_w);
	DECLARE_WRITE16_MEMBER(pigskin_protection_w);
	DECLARE_READ16_MEMBER(pigskin_protection_r);
	DECLARE_READ16_MEMBER(pigskin_port_1_r);
	DECLARE_READ16_MEMBER(pigskin_port_2_r);
	DECLARE_READ16_MEMBER(trisport_port_1_r);
	DECLARE_WRITE16_MEMBER(mcr68_6840_upper_w);
	DECLARE_WRITE16_MEMBER(mcr68_6840_lower_w);
	DECLARE_READ16_MEMBER(mcr68_6840_upper_r);
	DECLARE_READ16_MEMBER(mcr68_6840_lower_r);
	DECLARE_WRITE8_MEMBER(mcr68_6840_w_common);
	DECLARE_READ16_MEMBER(mcr68_6840_r_common);
	void reload_count(int counter);
	UINT16 compute_counter(int counter);
	DECLARE_WRITE16_MEMBER(mcr68_paletteram_w);
	DECLARE_WRITE16_MEMBER(zwackery_paletteram_w);
	DECLARE_WRITE16_MEMBER(mcr68_videoram_w);
	DECLARE_WRITE16_MEMBER(zwackery_videoram_w);
	DECLARE_WRITE16_MEMBER(zwackery_spriteram_w);
};


/*----------- defined in drivers/mcr68.c -----------*/

READ8_DEVICE_HANDLER( zwackery_port_2_r );


/*----------- defined in machine/mcr68.c -----------*/

extern const pia6821_interface zwackery_pia0_intf;
extern const pia6821_interface zwackery_pia1_intf;
extern const pia6821_interface zwackery_pia2_intf;


MACHINE_START( mcr68 );
MACHINE_RESET( mcr68 );
MACHINE_START( zwackery );
MACHINE_RESET( zwackery );


INTERRUPT_GEN( mcr68_interrupt );


/*----------- defined in video/mcr68.c -----------*/


VIDEO_START( mcr68 );
SCREEN_UPDATE_IND16( mcr68 );


VIDEO_START( zwackery );
SCREEN_UPDATE_IND16( zwackery );
