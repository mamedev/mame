/***************************************************************************

    Sega System 32/Multi 32 hardware

***************************************************************************/


typedef void (*sys32_output_callback)(int which, UINT16 data);
struct layer_info
{
	bitmap_t *	bitmap;
	UINT8 *		transparent;
};


class segas32_state : public driver_device
{
public:
	segas32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_z80_shared_ram;
	UINT8 m_v60_irq_control[0x10];
	timer_device *m_v60_irq_timer[2];
	UINT8 m_sound_irq_control[4];
	UINT8 m_sound_irq_input;
	UINT8 m_sound_dummy_value;
	UINT16 m_sound_bank;
	UINT8 m_misc_io_data[2][0x10];
	read16_space_func m_custom_io_r[2];
	write16_space_func m_custom_io_w[2];
	UINT8 m_analog_bank;
	UINT8 m_analog_value[4];
	UINT8 m_sonic_last[6];
	sys32_output_callback m_sw1_output;
	sys32_output_callback m_sw2_output;
	sys32_output_callback m_sw3_output;
	UINT16* m_dual_pcb_comms;
	UINT8 *m_ga2_dpram;
	UINT16 *m_system32_workram;
	UINT16 *m_system32_protram;
	UINT16 *m_system32_videoram;
	UINT16 *m_system32_spriteram;
	UINT16 *m_system32_paletteram[2];
	UINT16 m_system32_displayenable[2];
	UINT16 m_system32_tilebank_external;
	UINT16 m_arescue_dsp_io[6];
	UINT8 m_is_multi32;
	struct cache_entry *m_cache_head;
	struct layer_info m_layer_data[11];
	UINT16 m_mixer_control[2][0x40];
	UINT16 *m_solid_0000;
	UINT16 *m_solid_ffff;
	UINT8 m_sprite_render_count;
	UINT8 m_sprite_control_latched[8];
	UINT8 m_sprite_control[8];
	UINT32 *m_spriteram_32bit;
	void (*m_system32_prot_vblank)(device_t *device);
	int m_print_count;
};


/*----------- defined in machine/segas32.c -----------*/

READ16_HANDLER( arabfgt_protection_r );
READ16_HANDLER( arf_wakeup_protection_r );
WRITE16_HANDLER( arabfgt_protection_w );

READ16_HANDLER( brival_protection_r );
WRITE16_HANDLER( brival_protection_w );

READ16_HANDLER( darkedge_protection_r );
WRITE16_HANDLER( darkedge_protection_w );
void darkedge_fd1149_vblank(device_t *device);
WRITE16_HANDLER( jleague_protection_w );

READ16_HANDLER( dbzvrvs_protection_r );
WRITE16_HANDLER( dbzvrvs_protection_w );

extern const UINT8 ga2_v25_opcode_table[];
void decrypt_ga2_protrom(running_machine &machine);
READ16_HANDLER( ga2_dpram_r );
WRITE16_HANDLER( ga2_dpram_w );

WRITE16_HANDLER(sonic_level_load_protection);

READ16_HANDLER( arescue_dsp_r );
WRITE16_HANDLER( arescue_dsp_w );


/*----------- defined in video/segas32.c -----------*/

VIDEO_START(system32);
VIDEO_START(multi32);
SCREEN_UPDATE(system32);
SCREEN_UPDATE(multi32);
void system32_set_vblank(running_machine &machine, int state);

READ16_HANDLER( system32_videoram_r );
WRITE16_HANDLER( system32_videoram_w );
READ32_HANDLER( multi32_videoram_r );
WRITE32_HANDLER( multi32_videoram_w );

READ16_HANDLER( system32_spriteram_r );
WRITE16_HANDLER( system32_spriteram_w );
READ32_HANDLER( multi32_spriteram_r );
WRITE32_HANDLER( multi32_spriteram_w );

READ16_HANDLER( system32_paletteram_r );
WRITE16_HANDLER( system32_paletteram_w );
READ32_HANDLER( multi32_paletteram_0_r );
WRITE32_HANDLER( multi32_paletteram_0_w );
READ32_HANDLER( multi32_paletteram_1_r );
WRITE32_HANDLER( multi32_paletteram_1_w );

READ16_HANDLER( system32_sprite_control_r );
WRITE16_HANDLER( system32_sprite_control_w );
READ32_HANDLER( multi32_sprite_control_r );
WRITE32_HANDLER( multi32_sprite_control_w );

WRITE16_HANDLER( system32_mixer_w );
READ16_HANDLER( system32_mixer_r );
WRITE32_HANDLER( multi32_mixer_0_w );
WRITE32_HANDLER( multi32_mixer_1_w );
