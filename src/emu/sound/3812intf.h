#pragma once

#ifndef __3812INTF_H__
#define __3812INTF_H__


typedef struct _ym3812_interface ym3812_interface;
struct _ym3812_interface
{
	void (*handler)(running_machine *machine, int linestate);
};

#define ym3526_interface ym3812_interface

typedef struct _y8950_interface y8950_interface;
struct _y8950_interface
{
	void (*handler)(running_machine *machine, int linestate);

	read8_space_func keyboardread;
	write8_space_func keyboardwrite;
	read8_space_func portread;
	write8_space_func portwrite;
};


/* YM3812 */
READ8_HANDLER ( ym3812_status_port_0_r );
WRITE8_HANDLER( ym3812_control_port_0_w );
READ8_HANDLER( ym3812_read_port_0_r );
WRITE8_HANDLER( ym3812_write_port_0_w );

READ8_HANDLER ( ym3812_status_port_1_r );
WRITE8_HANDLER( ym3812_control_port_1_w );
READ8_HANDLER( ym3812_read_port_1_r );
WRITE8_HANDLER( ym3812_write_port_1_w );


/* YM3526 */
READ8_HANDLER ( ym3526_status_port_0_r );
WRITE8_HANDLER( ym3526_control_port_0_w );
READ8_HANDLER( ym3526_read_port_0_r );
WRITE8_HANDLER( ym3526_write_port_0_w );

READ8_HANDLER ( ym3526_status_port_1_r );
WRITE8_HANDLER( ym3526_control_port_1_w );
READ8_HANDLER( ym3526_read_port_1_r );
WRITE8_HANDLER( ym3526_write_port_1_w );


/* Y8950 */
READ8_HANDLER ( y8950_status_port_0_r );
WRITE8_HANDLER( y8950_control_port_0_w );
READ8_HANDLER ( y8950_read_port_0_r );
WRITE8_HANDLER( y8950_write_port_0_w );

READ8_HANDLER ( y8950_status_port_1_r );
WRITE8_HANDLER( y8950_control_port_1_w );
READ8_HANDLER ( y8950_read_port_1_r );
WRITE8_HANDLER( y8950_write_port_1_w );

#endif /* __3812INTF_H__ */
