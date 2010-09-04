
class segas1x_state : public driver_device
{
public:
	segas1x_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  interrupt_timer(*this, "int_timer") { }

	/* memory pointers */
//  UINT16 *  workram;  // this is used in the nvram handler, hence it cannot be added here
//  UINT16 *  paletteram;   // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  UINT16 *  tileram_0;    // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  UINT16 *  textram_0;    // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  UINT16 *  spriteram_0;  // this is used in the segaic16 mapper, hence it cannot be added here (yet)

	/* misc video */
	UINT8 road_priority;		// segaxbd
	bitmap_t *tmp_bitmap;		// segaybd & segas18
	UINT8 grayscale_enable;		// segas18
	UINT8 vdp_enable;			// segas18
	UINT8 vdp_mixing;			// segas18

	/* misc common */
	UINT8 rom_board;			// segas16b
	UINT8 mj_input_num;		// segas16a & segas16b
	UINT8 mj_last_val;		// segas16b
	UINT8 adc_select;			// segahang & segaorun
	UINT8 timer_irq_state;		// segaxbd & segaybd
	UINT8 vblank_irq_state;		// segaorun, segaxbd & segaybd
	UINT8 misc_io_data[0x10];	// system18 & segaybd

	void (*i8751_vblank_hook)(running_machine *machine);
	const UINT8 *i8751_initial_config;

	read16_space_func custom_io_r;
	write16_space_func custom_io_w;

	/* misc system 16b */
	UINT8 atomicp_sound_divisor;
	UINT8 atomicp_sound_count;
	UINT8 disable_screen_blanking;
	UINT8 hwc_input_value;


	/* misc system 16a */
	UINT8 video_control;
	UINT8 mcu_control;
	UINT8 n7751_command;
	UINT32 n7751_rom_address;
	UINT8 last_buttons1;
	UINT8 last_buttons2;
	int read_port;

	void (*lamp_changed_w)(running_machine *machine, UINT8 changed, UINT8 newval);

	/* misc system 18 */
	UINT8 mcu_data;

	UINT8 wwally_last_x[3], wwally_last_y[3];
	UINT8 lghost_value, lghost_select;

	/* misc segaorun */
	UINT8 irq2_state;
	const UINT8 *custom_map;

	/* misc yboard */
	UINT8 analog_data[4];
	int irq2_scanline;

	/* misc xboard */
	UINT8 iochip_regs[2][8];
	UINT8 iochip_force_input;

	UINT8 (*iochip_custom_io_r[2])(offs_t offset, UINT8 portdata);	// currently unused
	void (*iochip_custom_io_w[2])(offs_t offset, UINT8 data);	// currently unused

	UINT8 adc_reverse[8];

	UINT8 gprider_hack;
	UINT16 *loffire_sync;


	/* devices */
	running_device *maincpu;
	running_device *soundcpu;
	running_device *subcpu;
	running_device *subx;
	running_device *suby;
	running_device *mcu;
	running_device *ymsnd;
	running_device *ppi8255;
	running_device *n7751;
	running_device *ppi8255_1;
	running_device *ppi8255_2;
	optional_device<timer_device> interrupt_timer;
	running_device *_315_5248_1;
	running_device *_315_5250_1;
	running_device *_315_5250_2;
};


/*----------- defined in video/segahang.c -----------*/

VIDEO_START( hangon );
VIDEO_START( sharrier );
VIDEO_UPDATE( hangon );

/*----------- defined in video/segas16a.c -----------*/

VIDEO_START( system16a );
VIDEO_UPDATE( system16a );

/*----------- defined in video/segas16b.c -----------*/

VIDEO_START( system16b );
VIDEO_START( timscanr );
VIDEO_UPDATE( system16b );

/*----------- defined in video/segas18.c -----------*/

VIDEO_START( system18 );
VIDEO_UPDATE( system18 );

void system18_set_grayscale(running_machine *machine, int enable);
void system18_set_vdp_enable(running_machine *machine, int eanble);
void system18_set_vdp_mixing(running_machine *machine, int mixing);

/*----------- defined in video/segaorun.c -----------*/

VIDEO_START( outrun );
VIDEO_START( shangon );
VIDEO_UPDATE( outrun );
VIDEO_UPDATE( shangon );

/*----------- defined in video/segaxbd.c -----------*/

VIDEO_START( xboard );
VIDEO_UPDATE( xboard );

/*----------- defined in video/segaybd.c -----------*/

VIDEO_START( yboard );
VIDEO_UPDATE( yboard );


/*----------- defined in machine/s16fd.c -----------*/

void *fd1094_get_decrypted_base(void);
void fd1094_machine_init(running_device *device);
void fd1094_driver_init(running_machine *machine, const char* tag, void (*set_decrypted)(running_machine *, UINT8 *));
