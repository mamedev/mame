/* Dallas DS2404 RTC/NVRAM */

#include "driver.h"
#include "deprecat.h"
#include "ds2404.h"
#include <time.h>

typedef enum {
	DS2404_STATE_IDLE = 1,				/* waiting for ROM command, in 1-wire mode */
	DS2404_STATE_COMMAND,				/* waiting for memory command */
	DS2404_STATE_ADDRESS1,				/* waiting for address bits 0-7 */
	DS2404_STATE_ADDRESS2,				/* waiting for address bits 8-15 */
	DS2404_STATE_OFFSET,				/* waiting for ending offset */
	DS2404_STATE_INIT_COMMAND,
	DS2404_STATE_READ_MEMORY,			/* Read Memory command active */
	DS2404_STATE_WRITE_SCRATCHPAD,		/* Write Scratchpad command active */
	DS2404_STATE_READ_SCRATCHPAD,		/* Read Scratchpad command active */
	DS2404_STATE_COPY_SCRATCHPAD		/* Copy Scratchpad command active */
} DS2404_STATE;

typedef struct {
	UINT16 address;
	UINT16 offset;
	UINT16 end_offset;
	UINT8 a1, a2;
	UINT8 sram[512];	/* 4096 bits */
	UINT8 ram[32];		/* scratchpad ram, 256 bits */
	UINT8 rtc[5];		/* 40-bit RTC counter */
	DS2404_STATE state[8];
	int state_ptr;
} DS2404;

static DS2404 ds2404;

static void ds2404_rom_cmd(UINT8 cmd)
{
	switch(cmd)
	{
		case 0xcc:		/* Skip ROM */
			ds2404.state[0] = DS2404_STATE_COMMAND;
			ds2404.state_ptr = 0;
			break;

		default:
			fatalerror("DS2404: Unknown ROM command %02X", cmd);
			break;
	}
}

static void ds2404_cmd(UINT8 cmd)
{
	switch(cmd)
	{
		case 0x0f:		/* Write scratchpad */
			ds2404.state[0] = DS2404_STATE_ADDRESS1;
			ds2404.state[1] = DS2404_STATE_ADDRESS2;
			ds2404.state[2] = DS2404_STATE_INIT_COMMAND;
			ds2404.state[3] = DS2404_STATE_WRITE_SCRATCHPAD;
			ds2404.state_ptr = 0;
			break;

		case 0x55:		/* Copy scratchpad */
			ds2404.state[0] = DS2404_STATE_ADDRESS1;
			ds2404.state[1] = DS2404_STATE_ADDRESS2;
			ds2404.state[2] = DS2404_STATE_OFFSET;
			ds2404.state[3] = DS2404_STATE_INIT_COMMAND;
			ds2404.state[4] = DS2404_STATE_COPY_SCRATCHPAD;
			ds2404.state_ptr = 0;
			break;

		case 0xf0:		/* Read memory */
			ds2404.state[0] = DS2404_STATE_ADDRESS1;
			ds2404.state[1] = DS2404_STATE_ADDRESS2;
			ds2404.state[2] = DS2404_STATE_INIT_COMMAND;
			ds2404.state[3] = DS2404_STATE_READ_MEMORY;
			ds2404.state_ptr = 0;
			break;

		default:
			fatalerror("DS2404: Unknown command %02X", cmd);
			break;
	}
}

static UINT8 ds2404_readmem(void)
{
	if( ds2404.address < 0x200 )
	{
		return ds2404.sram[ ds2404.address ];
	}
	else if( ds2404.address >= 0x202 && ds2404.address <= 0x206 )
	{
		return ds2404.rtc[ ds2404.address - 0x202 ];
	}
	return 0;
}

static void ds2404_writemem(UINT8 value)
{
	if( ds2404.address < 0x200 )
	{
		ds2404.sram[ ds2404.address ] = value;
	}
	else if( ds2404.address >= 0x202 && ds2404.address <= 0x206 )
	{
		ds2404.rtc[ ds2404.address - 0x202 ] = value;
	}
}

WRITE8_HANDLER( DS2404_1W_reset_w )
{
	ds2404.state[0] = DS2404_STATE_IDLE;
	ds2404.state_ptr = 0;
}

WRITE8_HANDLER( DS2404_3W_reset_w )
{
	ds2404.state[0] = DS2404_STATE_COMMAND;
	ds2404.state_ptr = 0;
}

READ8_HANDLER( DS2404_data_r )
{
	UINT8 value;
	switch( ds2404.state[ds2404.state_ptr] )
	{
		case DS2404_STATE_IDLE:
		case DS2404_STATE_COMMAND:
		case DS2404_STATE_ADDRESS1:
		case DS2404_STATE_ADDRESS2:
		case DS2404_STATE_OFFSET:
		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			value = ds2404_readmem();
			return value;

		case DS2404_STATE_READ_SCRATCHPAD:
			if( ds2404.offset < 0x20 ) {
				value = ds2404.ram[ds2404.offset];
				ds2404.offset++;
				return value;
			}
			break;

		case DS2404_STATE_WRITE_SCRATCHPAD:
			break;

		case DS2404_STATE_COPY_SCRATCHPAD:
			break;
	}
	return 0;
}

WRITE8_HANDLER( DS2404_data_w )
{
	int i;

	switch( ds2404.state[ds2404.state_ptr] )
	{
		case DS2404_STATE_IDLE:
			ds2404_rom_cmd(data & 0xff);
			break;

		case DS2404_STATE_COMMAND:
			ds2404_cmd(data & 0xff);
			break;

		case DS2404_STATE_ADDRESS1:
			ds2404.a1 = data & 0xff;
			ds2404.state_ptr++;
			break;

		case DS2404_STATE_ADDRESS2:
			ds2404.a2 = data & 0xff;
			ds2404.state_ptr++;
			break;

		case DS2404_STATE_OFFSET:
			ds2404.end_offset = data & 0xff;
			ds2404.state_ptr++;
			break;

		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			break;

		case DS2404_STATE_READ_SCRATCHPAD:
			break;

		case DS2404_STATE_WRITE_SCRATCHPAD:
			if( ds2404.offset < 0x20 ) {
				ds2404.ram[ds2404.offset] = data & 0xff;
				ds2404.offset++;
			} else {
				/* Set OF flag */
			}
			break;

		case DS2404_STATE_COPY_SCRATCHPAD:
			break;
	}

	if( ds2404.state[ds2404.state_ptr] == DS2404_STATE_INIT_COMMAND ) {
		switch( ds2404.state[ds2404.state_ptr+1] )
		{
			case DS2404_STATE_IDLE:
			case DS2404_STATE_COMMAND:
			case DS2404_STATE_ADDRESS1:
			case DS2404_STATE_ADDRESS2:
			case DS2404_STATE_OFFSET:
			case DS2404_STATE_INIT_COMMAND:
				break;

			case DS2404_STATE_READ_MEMORY:
				ds2404.address = (ds2404.a2 << 8) | ds2404.a1;
				ds2404.address -= 1;
				break;

			case DS2404_STATE_WRITE_SCRATCHPAD:
				ds2404.address = (ds2404.a2 << 8) | ds2404.a1;
				ds2404.offset = ds2404.address & 0x1f;
				break;

			case DS2404_STATE_READ_SCRATCHPAD:
				ds2404.address = (ds2404.a2 << 8) | ds2404.a1;
				ds2404.offset = ds2404.address & 0x1f;
				break;

			case DS2404_STATE_COPY_SCRATCHPAD:
				ds2404.address = (ds2404.a2 << 8) | ds2404.a1;

				for( i=0; i <= ds2404.end_offset; i++ ) {
					ds2404_writemem( ds2404.ram[i] );
					ds2404.address++;
				}
				break;
		}
		ds2404.state_ptr++;
	}
}

WRITE8_HANDLER( DS2404_clk_w )
{
	switch( ds2404.state[ds2404.state_ptr] )
	{
		case DS2404_STATE_IDLE:
		case DS2404_STATE_COMMAND:
		case DS2404_STATE_ADDRESS1:
		case DS2404_STATE_ADDRESS2:
		case DS2404_STATE_OFFSET:
		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			ds2404.address++;
			break;

		case DS2404_STATE_READ_SCRATCHPAD:
			break;

		case DS2404_STATE_WRITE_SCRATCHPAD:
			break;

		case DS2404_STATE_COPY_SCRATCHPAD:
			break;
	}
}

static TIMER_CALLBACK( DS2404_tick )
{
	int i;
	for( i = 0; i < 5; i++ )
	{
		ds2404.rtc[ i ]++;
		if( ds2404.rtc[ i ] != 0 )
		{
			break;
		}
	}
}

void DS2404_init(int ref_year, int ref_month, int ref_day)
{
	struct tm ref_tm;
	time_t ref_time;
	time_t current_time;
	emu_timer *timer;

	memset( &ref_tm, 0, sizeof( ref_tm ) );
	ref_tm.tm_year = ref_year - 1900;
	ref_tm.tm_mon = ref_month - 1;
	ref_tm.tm_mday = ref_day;

	ref_time = mktime( &ref_tm );

	time( &current_time );
	current_time -= ref_time;

	ds2404.rtc[ 0 ] = 0x0;
	ds2404.rtc[ 1 ] = ( current_time >> 0 ) & 0xff;
	ds2404.rtc[ 2 ] = ( current_time >> 8 ) & 0xff;
	ds2404.rtc[ 3 ] = ( current_time >> 16 ) & 0xff;
	ds2404.rtc[ 4 ] = ( current_time >> 24 ) & 0xff;

	timer = timer_alloc( Machine, DS2404_tick , NULL);
	timer_adjust_periodic( timer, ATTOTIME_IN_HZ( 256 ), 0, ATTOTIME_IN_HZ( 256 ) );
}

void DS2404_load(mame_file *file)
{
	mame_fread(file, ds2404.sram, 512);
}

void DS2404_save(mame_file *file)
{
	mame_fwrite(file, ds2404.sram, 512);
}
