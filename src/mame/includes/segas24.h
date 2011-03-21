class segas24_state : public driver_device
{
public:
	segas24_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int fdc_status;
	int fdc_track;
	int fdc_sector;
	int fdc_data;
	int fdc_phys_track;
	int fdc_irq;
	int fdc_drq;
	int fdc_span;
	int fdc_index_count;
	UINT8 *fdc_pt;
	int track_size;
	int cur_input_line;
	UINT8 hotrod_ctrl_cur;
	UINT8 resetcontrol;
	UINT8 prev_resetcontrol;
	UINT8 curbank;
	UINT8 mlatch;
	const UINT8 *mlatch_table;
	UINT16 irq_timera;
	UINT8 irq_timerb;
	UINT8 irq_allow0;
	UINT8 irq_allow1;
	int irq_timer_pend0;
	int irq_timer_pend1;
	int irq_yms;
	int irq_vblank;
	int irq_sprite;
	timer_device *irq_timer;
	timer_device *irq_timer_clear;
	int turns;
	UINT16 *system24temp_sys16_shared_ram;
	UINT8  (*system24temp_sys16_io_io_r)(running_machine *machine, int port);
	void   (*system24temp_sys16_io_io_w)(running_machine *machine, int port, UINT8 data);
	void   (*system24temp_sys16_io_cnt_w)(address_space *space, UINT8 data);
	read16_space_func system24temp_sys16_io_iod_r;
	write16_space_func system24temp_sys16_io_iod_w;
	UINT8 system24temp_sys16_io_cnt;
	UINT8 system24temp_sys16_io_dir;
};


/*----------- defined in machine/segas24.c -----------*/

/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

/* New Code */

void system24temp_sys16_io_set_callbacks(
	running_machine *machine,
	UINT8 (*io_r)(running_machine *machine, int port),
	void  (*io_w)(running_machine *machine, int port, UINT8 data),
	void  (*cnt_w)(address_space *space, UINT8 data),
	read16_space_func iod_r,
	write16_space_func iod_w);

READ16_HANDLER ( system24temp_sys16_io_r );
WRITE16_HANDLER( system24temp_sys16_io_w );
READ32_HANDLER ( system24temp_sys16_io_dword_r );

/* End New Code */


/*----------- defined in machine/s24fd.c -----------*/

extern void s24_fd1094_machine_init(running_machine *machine);
extern void s24_fd1094_driver_init(running_machine *machine);


/*----------- defined in video/segas24.c -----------*/

VIDEO_START(system24);
SCREEN_UPDATE(system24);
