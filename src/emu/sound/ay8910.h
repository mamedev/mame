#pragma once

#ifndef __AY8910_H__
#define __AY8910_H__

/*
AY-3-8910A: 2 I/O ports
AY-3-8912A: 1 I/O port
AY-3-8913A: 0 I/O port
AY8930: upper compatible with 8910.
In extended mode, it has higher resolution and duty ratio setting
YM2149: higher resolution
YM3439: same as 2149
YMZ284: 0 I/O port, different clock divider
YMZ294: 0 I/O port
*/

#define ALL_8910_CHANNELS -1

/* Internal resistance at Volume level 7. */

#define AY8910_INTERNAL_RESISTANCE	(356)
#define YM2149_INTERNAL_RESISTANCE	(353)

/*
 * Default values for resistor loads.
 * The macro should be used in AY8910interface if
 * the real values are unknown.
 */
#define AY8910_DEFAULT_LOADS		{1000, 1000, 1000}

/*
 * The following is used by all drivers not reviewed yet.
 * This will like the old behaviour, output between
 * 0 and 7FFF
 */
#define AY8910_LEGACY_OUTPUT		(1)

/*
 * Specifing the next define will simulate the special
 * cross channel mixing if outputs are tied together.
 * The driver will only provide one stream in this case.
 */
#define AY8910_SINGLE_OUTPUT		(2)

/*
 * The follwoing define is the default behaviour.
 * Output level 0 is 0V and 7ffff corresponds to 5V.
 * Use this to specify that a discrete mixing stage
 * follows.
 */
#define AY8910_DISCRETE_OUTPUT		(4)

/*
 * The follwoing define causes the driver to output
 * raw volume levels, i.e. 0 .. 15 and 0..31.
 * This is intended to be used in a subsequent
 * mixing modul (i.e. mpatrol ties 6 channels from
 * AY-3-8910 together). Do not use it now.
 */
/* TODO: implement mixing module */
#define AY8910_RAW_OUTPUT			(8)

typedef struct _ay8910_interface ay8910_interface;
struct _ay8910_interface
{
	int					flags;			/* Flags */
	int					res_load[3]; 	/* Load on channel in ohms */
	read8_space_func	portAread;
	read8_space_func	portBread;
	write8_space_func	portAwrite;
	write8_space_func	portBwrite;
};


void ay8910_set_volume(int chip,int channel,int volume);


READ8_HANDLER( ay8910_read_port_0_r );
READ8_HANDLER( ay8910_read_port_1_r );
READ8_HANDLER( ay8910_read_port_2_r );
READ8_HANDLER( ay8910_read_port_3_r );
READ8_HANDLER( ay8910_read_port_4_r );
READ16_HANDLER( ay8910_read_port_0_lsb_r );
READ16_HANDLER( ay8910_read_port_1_lsb_r );
READ16_HANDLER( ay8910_read_port_2_lsb_r );
READ16_HANDLER( ay8910_read_port_3_lsb_r );
READ16_HANDLER( ay8910_read_port_4_lsb_r );
READ16_HANDLER( ay8910_read_port_0_msb_r );
READ16_HANDLER( ay8910_read_port_1_msb_r );
READ16_HANDLER( ay8910_read_port_2_msb_r );
READ16_HANDLER( ay8910_read_port_3_msb_r );
READ16_HANDLER( ay8910_read_port_4_msb_r );

WRITE8_HANDLER( ay8910_control_port_0_w );
WRITE8_HANDLER( ay8910_control_port_1_w );
WRITE8_HANDLER( ay8910_control_port_2_w );
WRITE8_HANDLER( ay8910_control_port_3_w );
WRITE8_HANDLER( ay8910_control_port_4_w );
WRITE16_HANDLER( ay8910_control_port_0_lsb_w );
WRITE16_HANDLER( ay8910_control_port_1_lsb_w );
WRITE16_HANDLER( ay8910_control_port_2_lsb_w );
WRITE16_HANDLER( ay8910_control_port_3_lsb_w );
WRITE16_HANDLER( ay8910_control_port_4_lsb_w );
WRITE16_HANDLER( ay8910_control_port_0_msb_w );
WRITE16_HANDLER( ay8910_control_port_1_msb_w );
WRITE16_HANDLER( ay8910_control_port_2_msb_w );
WRITE16_HANDLER( ay8910_control_port_3_msb_w );
WRITE16_HANDLER( ay8910_control_port_4_msb_w );

WRITE8_HANDLER( ay8910_write_port_0_w );
WRITE8_HANDLER( ay8910_write_port_1_w );
WRITE8_HANDLER( ay8910_write_port_2_w );
WRITE8_HANDLER( ay8910_write_port_3_w );
WRITE8_HANDLER( ay8910_write_port_4_w );
WRITE16_HANDLER( ay8910_write_port_0_lsb_w );
WRITE16_HANDLER( ay8910_write_port_1_lsb_w );
WRITE16_HANDLER( ay8910_write_port_2_lsb_w );
WRITE16_HANDLER( ay8910_write_port_3_lsb_w );
WRITE16_HANDLER( ay8910_write_port_4_lsb_w );
WRITE16_HANDLER( ay8910_write_port_0_msb_w );
WRITE16_HANDLER( ay8910_write_port_1_msb_w );
WRITE16_HANDLER( ay8910_write_port_2_msb_w );
WRITE16_HANDLER( ay8910_write_port_3_msb_w );
WRITE16_HANDLER( ay8910_write_port_4_msb_w );

/*********** An interface for SSG of YM2203 ***********/

void *ay8910_start_ym(sound_type chip_type, const device_config *device, int clock, const ay8910_interface *intf);

void ay8910_stop_ym(void *chip);
void ay8910_reset_ym(void *chip);
void ay8910_set_clock_ym(void *chip, int clock);
void ay8910_write_ym(void *chip, int addr, int data);
int ay8910_read_ym(void *chip);

SND_GET_INFO( ay8910 );
SND_GET_INFO( ay8912 );
SND_GET_INFO( ay8913 );
SND_GET_INFO( ay8930 );
SND_GET_INFO( ym2149 );
SND_GET_INFO( ym3439 );
SND_GET_INFO( ymz284 );
SND_GET_INFO( ymz294 );

#endif /* __AY8910_H__ */
