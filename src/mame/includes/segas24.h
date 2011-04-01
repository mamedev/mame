class segas24_state : public driver_device
{
public:
	segas24_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_fdc_status;
	int m_fdc_track;
	int m_fdc_sector;
	int m_fdc_data;
	int m_fdc_phys_track;
	int m_fdc_irq;
	int m_fdc_drq;
	int m_fdc_span;
	int m_fdc_index_count;
	UINT8 *m_fdc_pt;
	int m_track_size;
	int m_cur_input_line;
	UINT8 m_hotrod_ctrl_cur;
	UINT8 m_resetcontrol;
	UINT8 m_prev_resetcontrol;
	UINT8 m_curbank;
	UINT8 m_mlatch;
	const UINT8 *m_mlatch_table;
	UINT16 m_irq_timera;
	UINT8 m_irq_timerb;
	UINT8 m_irq_allow0;
	UINT8 m_irq_allow1;
	int m_irq_timer_pend0;
	int m_irq_timer_pend1;
	int m_irq_yms;
	int m_irq_vblank;
	int m_irq_sprite;
	timer_device *m_irq_timer;
	timer_device *m_irq_timer_clear;
	int m_turns;
	UINT16 *m_system24temp_sys16_shared_ram;
	UINT8  (*m_system24temp_sys16_io_io_r)(running_machine &machine, int port);
	void   (*m_system24temp_sys16_io_io_w)(running_machine &machine, int port, UINT8 data);
	void   (*m_system24temp_sys16_io_cnt_w)(address_space *space, UINT8 data);
	read16_space_func m_system24temp_sys16_io_iod_r;
	write16_space_func m_system24temp_sys16_io_iod_w;
	UINT8 m_system24temp_sys16_io_cnt;
	UINT8 m_system24temp_sys16_io_dir;
};


/*----------- defined in machine/segas24.c -----------*/

/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

/* New Code */

void system24temp_sys16_io_set_callbacks(
	running_machine &machine,
	UINT8 (*io_r)(running_machine &machine, int port),
	void  (*io_w)(running_machine &machine, int port, UINT8 data),
	void  (*cnt_w)(address_space *space, UINT8 data),
	read16_space_func iod_r,
	write16_space_func iod_w);

READ16_HANDLER ( system24temp_sys16_io_r );
WRITE16_HANDLER( system24temp_sys16_io_w );
READ32_HANDLER ( system24temp_sys16_io_dword_r );

/* End New Code */


/*----------- defined in machine/s24fd.c -----------*/

extern void s24_fd1094_machine_init(running_machine &machine);
extern void s24_fd1094_driver_init(running_machine &machine);


/*----------- defined in video/segas24.c -----------*/

VIDEO_START(system24);
SCREEN_UPDATE(system24);
