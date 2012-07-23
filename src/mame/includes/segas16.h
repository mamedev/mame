
class segas1x_state : public driver_device
{
public:
	segas1x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_interrupt_timer(*this, "int_timer")
	{ }

	/* memory pointers */
//  UINT16 *  m_workram;  // this is used in the nvram handler, hence it cannot be added here
//  UINT16 *  m_paletteram;   // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  UINT16 *  m_tileram_0;    // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  UINT16 *  m_textram_0;    // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  UINT16 *  m_spriteram_0;  // this is used in the segaic16 mapper, hence it cannot be added here (yet)

	/* misc video */
	UINT8 m_road_priority;		// segaxbd
	bitmap_ind16 *m_tmp_bitmap;		// segaybd & segas18
	UINT8 m_grayscale_enable;		// segas18
	UINT8 m_vdp_enable;			// segas18
	UINT8 m_vdp_mixing;			// segas18

	/* misc common */
	UINT8 m_rom_board;			// segas16b
	UINT8 m_mj_input_num;		// segas16a & segas16b
	UINT8 m_mj_last_val;		// segas16b
	UINT8 m_adc_select;			// segahang & segaorun
	UINT8 m_timer_irq_state;		// segaxbd & segaybd
	UINT8 m_vblank_irq_state;		// segaorun, segaxbd & segaybd
	UINT8 m_misc_io_data[0x10];	// system18 & segaybd

	void (*m_i8751_vblank_hook)(running_machine &machine);
	const UINT8 *m_i8751_initial_config;

	read16_space_func m_custom_io_r;
	write16_space_func m_custom_io_w;

	/* misc system 16b */
	UINT8 m_atomicp_sound_divisor;
	UINT8 m_atomicp_sound_count;
	UINT8 m_disable_screen_blanking;
	UINT8 m_hwc_input_value;


	/* misc system 16a */
	UINT8 m_video_control;
	UINT8 m_mcu_control;
	UINT8 m_n7751_command;
	UINT32 m_n7751_rom_address;
	UINT8 m_last_buttons1;
	UINT8 m_last_buttons2;
	int m_read_port;

	void (*m_lamp_changed_w)(running_machine &machine, UINT8 changed, UINT8 newval);

	/* misc system 18 */
	UINT8 m_mcu_data;

	UINT8 m_wwally_last_x[3];
	UINT8 m_wwally_last_y[3];
	UINT8 m_lghost_value;
	UINT8 m_lghost_select;

	/* misc segaorun */
	UINT8 m_irq2_state;
	const UINT8 *m_custom_map;

	/* misc yboard */
	UINT8 m_analog_data[4];
	int m_irq2_scanline;

	/* misc xboard */
	UINT8 m_iochip_regs[2][8];
	UINT8 m_iochip_force_input;

	UINT8 (*m_iochip_custom_io_r[2])(offs_t offset, UINT8 portdata);	// currently unused
	void (*m_iochip_custom_io_w[2])(offs_t offset, UINT8 data);	// currently unused

	UINT8 m_adc_reverse[8];

	UINT8 m_gprider_hack;
	UINT16 *m_loffire_sync;


	/* devices */
	device_t *m_maincpu;
	device_t *m_soundcpu;
	device_t *m_soundcpu2;
	device_t *m_subcpu;
	device_t *m_subx;
	device_t *m_suby;
	device_t *m_mcu;
	device_t *m_ymsnd;
	device_t *m_ppi8255;
	device_t *m_n7751;
	device_t *m_ppi8255_1;
	device_t *m_ppi8255_2;
	optional_device<timer_device> m_interrupt_timer;
	device_t *m_315_5248_1;
	device_t *m_315_5250_1;
	device_t *m_315_5250_2;
};


/*----------- defined in video/segahang.c -----------*/

VIDEO_START( hangon );
VIDEO_START( sharrier );
SCREEN_UPDATE_IND16( hangon );

/*----------- defined in video/segas16a.c -----------*/

VIDEO_START( system16a );
SCREEN_UPDATE_IND16( system16a );

/*----------- defined in video/segas16b.c -----------*/

VIDEO_START( system16b );
VIDEO_START( timscanr );
SCREEN_UPDATE_IND16( system16b );

/*----------- defined in video/segas18.c -----------*/

VIDEO_START( system18 );
SCREEN_UPDATE_IND16( system18 );

void system18_set_grayscale(running_machine &machine, int enable);
void system18_set_vdp_enable(running_machine &machine, int eanble);
void system18_set_vdp_mixing(running_machine &machine, int mixing);

/*----------- defined in video/segaorun.c -----------*/

VIDEO_START( outrun );
VIDEO_START( shangon );
SCREEN_UPDATE_IND16( outrun );
SCREEN_UPDATE_IND16( shangon );

/*----------- defined in video/segaxbd.c -----------*/

VIDEO_START( xboard );
SCREEN_UPDATE_IND16( xboard );

/*----------- defined in video/segaybd.c -----------*/

VIDEO_START( yboard );
SCREEN_UPDATE_IND16( yboard );


/*----------- defined in machine/s16fd.c -----------*/

void *fd1094_get_decrypted_base(void);
void fd1094_machine_init(device_t *device);
void fd1094_driver_init(running_machine &machine, const char* tag, void (*set_decrypted)(running_machine &, UINT8 *));
