#ifndef YM3812INTF_H
#define YM3812INTF_H


struct YM3812interface
{
	void (*handler)(int linestate);
};

#define YM3526interface YM3812interface

struct Y8950interface
{
	void (*handler)(int linestate);

	int rom_region; /* delta-T ADPCM ROM/RAM region */

	read8_handler keyboardread;
	write8_handler keyboardwrite;
	read8_handler portread;
	write8_handler portwrite;
};


/* YM3812 */
READ8_HANDLER ( YM3812_status_port_0_r );
WRITE8_HANDLER( YM3812_control_port_0_w );
WRITE8_HANDLER( YM3812_write_port_0_w );

READ8_HANDLER ( YM3812_status_port_1_r );
WRITE8_HANDLER( YM3812_control_port_1_w );
WRITE8_HANDLER( YM3812_write_port_1_w );


/* YM3526 */
READ8_HANDLER ( YM3526_status_port_0_r );
WRITE8_HANDLER( YM3526_control_port_0_w );
WRITE8_HANDLER( YM3526_write_port_0_w );

READ8_HANDLER ( YM3526_status_port_1_r );
WRITE8_HANDLER( YM3526_control_port_1_w );
WRITE8_HANDLER( YM3526_write_port_1_w );


/* Y8950 */
READ8_HANDLER ( Y8950_status_port_0_r );
WRITE8_HANDLER( Y8950_control_port_0_w );
READ8_HANDLER ( Y8950_read_port_0_r );
WRITE8_HANDLER( Y8950_write_port_0_w );

READ8_HANDLER ( Y8950_status_port_1_r );
WRITE8_HANDLER( Y8950_control_port_1_w );
READ8_HANDLER ( Y8950_read_port_1_r );
WRITE8_HANDLER( Y8950_write_port_1_w );

#endif
