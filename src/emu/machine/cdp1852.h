/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
							_____   _____
			  CSI/_CSI   1 |*    \_/     | 24  Vdd
				  MODE   2 |			 | 23  _SR/SR
				   DI0   3 |			 | 22  DI7
				   DO0   4 |			 | 21  DO7
				   DI1   5 |			 | 20  DI6
				   DO1   6 |   CDP1852   | 19  DO6
				   DI2   7 |			 | 18  DI5
				   DO2   8 |			 | 17  DO5
				   DI3   9 |			 | 16  DI4
				   DO3  10 |			 | 15  DO4
				 CLOCK  11 |			 | 14  _CLEAR
				   Vss  12 |_____________| 13  CS2

**********************************************************************/

#ifndef __CDP1852__
#define __CDP1852__

#define CDP1852_CLOCK_HIGH	0

typedef UINT8 (*cdp1852_data_read_func) (const device_config *device);
#define CDP1852_DATA_READ(name) UINT8 name(const device_config *device)

typedef void (*cdp1852_data_write_func) (const device_config *device, UINT8 data);
#define CDP1852_DATA_WRITE(name) void name(const device_config *device, UINT8 data)

typedef void (*cdp1852_on_sr_changed_func) (const device_config *device, int level);
#define CDP1852_ON_SR_CHANGED(name) void name(const device_config *device, int level)

#define CDP1852		DEVICE_GET_INFO_NAME(cdp1852)

typedef enum _cdp1852_mode cdp1852_mode;
enum _cdp1852_mode {
	CDP1852_MODE_INPUT = 0,
	CDP1852_MODE_OUTPUT
};

/* interface */
typedef struct _cdp1852_interface cdp1852_interface;
struct _cdp1852_interface
{
	int clock;					/* the clock (pin 1) of the chip */

	int mode;					/* operation mode */

	/* this gets called for every external data read */
	cdp1852_data_read_func			data_r;

	/* this gets called for every external data write */
	cdp1852_data_write_func			data_w;

	/* this gets called for every change of the SR pin (pin 23) */
	cdp1852_on_sr_changed_func		on_sr_changed;
};
#define CDP1852_INTERFACE(name) const cdp1852_interface (name)=

/* device interface */
DEVICE_GET_INFO( cdp1852 );

/* data access */
READ8_DEVICE_HANDLER( cdp1852_data_r );
WRITE8_DEVICE_HANDLER( cdp1852_data_w );

#endif
