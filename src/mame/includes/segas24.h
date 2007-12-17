/*----------- defined in machine/system24.c -----------*/

/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

/* New Code */

extern UINT16 *system24temp_sys16_shared_ram;
READ16_HANDLER( system24temp_sys16_shared_ram_r );
WRITE16_HANDLER( system24temp_sys16_shared_ram_w );

void system24temp_sys16_io_set_callbacks(UINT8 (*io_r)(int port),
							void  (*io_w)(int port, UINT8 data),
							void  (*cnt_w)(UINT8 data),
							READ16_HANDLER ((*iod_r)),
							WRITE16_HANDLER((*iod_w)));
READ16_HANDLER ( system24temp_sys16_io_r );
WRITE16_HANDLER( system24temp_sys16_io_w );
READ32_HANDLER ( system24temp_sys16_io_dword_r );

/* End New Code */
