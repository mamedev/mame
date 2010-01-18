/* Dallas DS2404 RTC/NVRAM */

#include "emu.h"
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

typedef struct _ds2404_state ds2404_state;
struct _ds2404_state {
	UINT16 address;
	UINT16 offset;
	UINT16 end_offset;
	UINT8 a1, a2;
	UINT8 sram[512];	/* 4096 bits */
	UINT8 ram[32];		/* scratchpad ram, 256 bits */
	UINT8 rtc[5];		/* 40-bit RTC counter */
	DS2404_STATE state[8];
	int state_ptr;
};

INLINE ds2404_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == DS2404);

	return (ds2404_state *)device->token;
}


static void ds2404_rom_cmd(ds2404_state *state, UINT8 cmd)
{
	switch(cmd)
	{
		case 0xcc:		/* Skip ROM */
			state->state[0] = DS2404_STATE_COMMAND;
			state->state_ptr = 0;
			break;

		default:
			fatalerror("DS2404: Unknown ROM command %02X", cmd);
			break;
	}
}

static void ds2404_cmd(ds2404_state *state, UINT8 cmd)
{
	switch(cmd)
	{
		case 0x0f:		/* Write scratchpad */
			state->state[0] = DS2404_STATE_ADDRESS1;
			state->state[1] = DS2404_STATE_ADDRESS2;
			state->state[2] = DS2404_STATE_INIT_COMMAND;
			state->state[3] = DS2404_STATE_WRITE_SCRATCHPAD;
			state->state_ptr = 0;
			break;

		case 0x55:		/* Copy scratchpad */
			state->state[0] = DS2404_STATE_ADDRESS1;
			state->state[1] = DS2404_STATE_ADDRESS2;
			state->state[2] = DS2404_STATE_OFFSET;
			state->state[3] = DS2404_STATE_INIT_COMMAND;
			state->state[4] = DS2404_STATE_COPY_SCRATCHPAD;
			state->state_ptr = 0;
			break;

		case 0xf0:		/* Read memory */
			state->state[0] = DS2404_STATE_ADDRESS1;
			state->state[1] = DS2404_STATE_ADDRESS2;
			state->state[2] = DS2404_STATE_INIT_COMMAND;
			state->state[3] = DS2404_STATE_READ_MEMORY;
			state->state_ptr = 0;
			break;

		default:
			fatalerror("DS2404: Unknown command %02X", cmd);
			break;
	}
}

static UINT8 ds2404_readmem(ds2404_state *state)
{
	if( state->address < 0x200 )
	{
		return state->sram[ state->address ];
	}
	else if( state->address >= 0x202 && state->address <= 0x206 )
	{
		return state->rtc[ state->address - 0x202 ];
	}
	return 0;
}

static void ds2404_writemem(ds2404_state *state, UINT8 value)
{
	if( state->address < 0x200 )
	{
		state->sram[ state->address ] = value;
	}
	else if( state->address >= 0x202 && state->address <= 0x206 )
	{
		state->rtc[ state->address - 0x202 ] = value;
	}
}

WRITE8_DEVICE_HANDLER( ds2404_1w_reset_w )
{
	ds2404_state *state = get_safe_token(device);
	state->state[0] = DS2404_STATE_IDLE;
	state->state_ptr = 0;
}

WRITE8_DEVICE_HANDLER( ds2404_3w_reset_w )
{
	ds2404_state *state = get_safe_token(device);
	state->state[0] = DS2404_STATE_COMMAND;
	state->state_ptr = 0;
}

READ8_DEVICE_HANDLER( ds2404_data_r )
{
	ds2404_state *state = get_safe_token(device);
	UINT8 value;
	switch( state->state[state->state_ptr] )
	{
		case DS2404_STATE_IDLE:
		case DS2404_STATE_COMMAND:
		case DS2404_STATE_ADDRESS1:
		case DS2404_STATE_ADDRESS2:
		case DS2404_STATE_OFFSET:
		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			value = ds2404_readmem(state);
			return value;

		case DS2404_STATE_READ_SCRATCHPAD:
			if( state->offset < 0x20 ) {
				value = state->ram[state->offset];
				state->offset++;
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

WRITE8_DEVICE_HANDLER( ds2404_data_w )
{
	ds2404_state *state = get_safe_token(device);
	int i;

	switch( state->state[state->state_ptr] )
	{
		case DS2404_STATE_IDLE:
			ds2404_rom_cmd(state, data & 0xff);
			break;

		case DS2404_STATE_COMMAND:
			ds2404_cmd(state, data & 0xff);
			break;

		case DS2404_STATE_ADDRESS1:
			state->a1 = data & 0xff;
			state->state_ptr++;
			break;

		case DS2404_STATE_ADDRESS2:
			state->a2 = data & 0xff;
			state->state_ptr++;
			break;

		case DS2404_STATE_OFFSET:
			state->end_offset = data & 0xff;
			state->state_ptr++;
			break;

		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			break;

		case DS2404_STATE_READ_SCRATCHPAD:
			break;

		case DS2404_STATE_WRITE_SCRATCHPAD:
			if( state->offset < 0x20 ) {
				state->ram[state->offset] = data & 0xff;
				state->offset++;
			} else {
				/* Set OF flag */
			}
			break;

		case DS2404_STATE_COPY_SCRATCHPAD:
			break;
	}

	if( state->state[state->state_ptr] == DS2404_STATE_INIT_COMMAND ) {
		switch( state->state[state->state_ptr+1] )
		{
			case DS2404_STATE_IDLE:
			case DS2404_STATE_COMMAND:
			case DS2404_STATE_ADDRESS1:
			case DS2404_STATE_ADDRESS2:
			case DS2404_STATE_OFFSET:
			case DS2404_STATE_INIT_COMMAND:
				break;

			case DS2404_STATE_READ_MEMORY:
				state->address = (state->a2 << 8) | state->a1;
				state->address -= 1;
				break;

			case DS2404_STATE_WRITE_SCRATCHPAD:
				state->address = (state->a2 << 8) | state->a1;
				state->offset = state->address & 0x1f;
				break;

			case DS2404_STATE_READ_SCRATCHPAD:
				state->address = (state->a2 << 8) | state->a1;
				state->offset = state->address & 0x1f;
				break;

			case DS2404_STATE_COPY_SCRATCHPAD:
				state->address = (state->a2 << 8) | state->a1;

				for( i=0; i <= state->end_offset; i++ ) {
					ds2404_writemem( state, state->ram[i] );
					state->address++;
				}
				break;
		}
		state->state_ptr++;
	}
}

WRITE8_DEVICE_HANDLER( ds2404_clk_w )
{
	ds2404_state *state = get_safe_token(device);
	switch( state->state[state->state_ptr] )
	{
		case DS2404_STATE_IDLE:
		case DS2404_STATE_COMMAND:
		case DS2404_STATE_ADDRESS1:
		case DS2404_STATE_ADDRESS2:
		case DS2404_STATE_OFFSET:
		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			state->address++;
			break;

		case DS2404_STATE_READ_SCRATCHPAD:
			break;

		case DS2404_STATE_WRITE_SCRATCHPAD:
			break;

		case DS2404_STATE_COPY_SCRATCHPAD:
			break;
	}
}

static TIMER_CALLBACK( ds2404_tick )
{
	ds2404_state *state = get_safe_token((running_device *)ptr);
	int i;
	for( i = 0; i < 5; i++ )
	{
		state->rtc[ i ]++;
		if( state->rtc[ i ] != 0 )
		{
			break;
		}
	}
}


static DEVICE_START( ds2404 )
{
	ds2404_config *config = (ds2404_config *)device->baseconfig().inline_config;
	ds2404_state *state = get_safe_token(device);

	struct tm ref_tm;
	time_t ref_time;
	time_t current_time;
	emu_timer *timer;

	memset( &ref_tm, 0, sizeof( ref_tm ) );
	ref_tm.tm_year = config->ref_year - 1900;
	ref_tm.tm_mon = config->ref_month - 1;
	ref_tm.tm_mday = config->ref_day;

	ref_time = mktime( &ref_tm );

	time( &current_time );
	current_time -= ref_time;

	state->rtc[ 0 ] = 0x0;
	state->rtc[ 1 ] = ( current_time >> 0 ) & 0xff;
	state->rtc[ 2 ] = ( current_time >> 8 ) & 0xff;
	state->rtc[ 3 ] = ( current_time >> 16 ) & 0xff;
	state->rtc[ 4 ] = ( current_time >> 24 ) & 0xff;

	timer = timer_alloc( device->machine, ds2404_tick , (void *)device);
	timer_adjust_periodic( timer, ATTOTIME_IN_HZ( 256 ), 0, ATTOTIME_IN_HZ( 256 ) );
}


static DEVICE_RESET( ds2404 )
{
}


static DEVICE_NVRAM( ds2404 )
{
	ds2404_state *state = get_safe_token(device);

	if (read_or_write)
		mame_fwrite(file, state->sram, sizeof(state->sram));
	else if (file)
		mame_fread(file, state->sram, sizeof(state->sram));
	else
		memset(state->sram, 0, sizeof(state->sram));
}


static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##ds2404##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET | DT_HAS_NVRAM | DT_HAS_INLINE_CONFIG
#define DEVTEMPLATE_NAME		"DS2404"
#define DEVTEMPLATE_FAMILY		"NVRAM"
#include "devtempl.h"
