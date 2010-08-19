/*----------- defined in machine/segas24.c -----------*/

/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

/* New Code */

void system24temp_sys16_io_set_callbacks(UINT8 (*io_r)(running_machine *machine, int port),
							void  (*io_w)(running_machine *machine, int port, UINT8 data),
							void  (*cnt_w)(address_space *space, UINT8 data),
							read16_space_func iod_r,
							write16_space_func iod_w);
READ16_HANDLER ( system24temp_sys16_io_r );
WRITE16_HANDLER( system24temp_sys16_io_w );
READ32_HANDLER ( system24temp_sys16_io_dword_r );

/* End New Code */


/*----------- defined in drivers/segas24.c -----------*/

extern UINT16 *s24_mainram1;


/*----------- defined in machine/s24fd.c -----------*/

extern void s24_fd1094_machine_init(running_machine *machine);
extern void s24_fd1094_driver_init(running_machine *machine);


/*----------- defined in video/segas24.c -----------*/

VIDEO_START(system24);
VIDEO_UPDATE(system24);
