#include "emu.h"
#include "includes/segas24.h"

/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

#ifdef UNUSED_FUNCTION
READ16_HANDLER( system24temp_sys16_shared_ram_r )
{
	segas24_state *state = space->machine().driver_data<segas24_state>();
	return state->system24temp_sys16_shared_ram[offset];
}

WRITE16_HANDLER( system24temp_sys16_shared_ram_w )
{
	segas24_state *state = space->machine().driver_data<segas24_state>();
	COMBINE_DATA(state->system24temp_sys16_shared_ram + offset);
}
#endif

/* The 315-5296
     8 8bits I/O ports, 3 output-only pins, some protection, and
     external/daughterboard chip selection on half of the address
     range
*/

void system24temp_sys16_io_set_callbacks(
	running_machine &machine,
	UINT8 (*io_r)(running_machine &machine, int port),
	void  (*io_w)(running_machine &machine, int port, UINT8 data),
	void  (*cnt_w)(address_space *space, UINT8 data),
	read16_space_func iod_r,
	write16_space_func iod_w)
{
	segas24_state *state = machine.driver_data<segas24_state>();
	state->system24temp_sys16_io_io_r = io_r;
	state->system24temp_sys16_io_io_w = io_w;
	state->system24temp_sys16_io_cnt_w = cnt_w;
	state->system24temp_sys16_io_iod_r = iod_r;
	state->system24temp_sys16_io_iod_w = iod_w;
	state->system24temp_sys16_io_cnt = 0x00;
	state->system24temp_sys16_io_dir = 0x00;
}

READ16_HANDLER ( system24temp_sys16_io_r )
{
	segas24_state *state = space->machine().driver_data<segas24_state>();
	//  logerror("IO read %02x (%s:%x)\n", offset, space->device().tag(), cpu_get_pc(&space->device()));
	if(offset < 8)
		return state->system24temp_sys16_io_io_r ? state->system24temp_sys16_io_io_r(space->machine(),offset) : 0xff;
	else if (offset < 0x20) {
		switch(offset) {
		case 0x8:
			return 'S';
		case 0x9:
			return 'E';
		case 0xa:
			return 'G';
		case 0xb:
			return 'A';
		case 0xe:
			return state->system24temp_sys16_io_cnt;
		case 0xf:
			return state->system24temp_sys16_io_dir;
		default:
			logerror("IO control read %02x (%s:%x)\n", offset, space->device().tag(), cpu_get_pc(&space->device()));
			return 0xff;
		}
	} else
		return state->system24temp_sys16_io_iod_r ? state->system24temp_sys16_io_iod_r(space, offset & 0x1f, mem_mask) : 0xff;
}

READ32_HANDLER(system24temp_sys16_io_dword_r)
{
	return system24temp_sys16_io_r(space, 2*offset, mem_mask)|(system24temp_sys16_io_r(space,2*offset+1, mem_mask>>16)<<16);
}


WRITE16_HANDLER( system24temp_sys16_io_w )
{
	segas24_state *state = space->machine().driver_data<segas24_state>();
	if(ACCESSING_BITS_0_7) {
		if(offset < 8) {
			if(!(state->system24temp_sys16_io_dir & (1 << offset))) {
				logerror("IO port write on input-only port (%d, [%02x], %02x, %s:%x)\n", offset, state->system24temp_sys16_io_dir, data & 0xff, space->device().tag(), cpu_get_pc(&space->device()));
				return;
			}
			if(state->system24temp_sys16_io_io_w)
				state->system24temp_sys16_io_io_w(space->machine(), offset, data);
		} else if (offset < 0x20) {
			switch(offset) {
			case 0xe:
				state->system24temp_sys16_io_cnt = data;
				if(state->system24temp_sys16_io_cnt_w)
					state->system24temp_sys16_io_cnt_w(space, data & 7);
				break;
			case 0xf:
				state->system24temp_sys16_io_dir = data;
				break;
			default:
				logerror("IO control write %02x, %02x (%s:%x)\n", offset, data & 0xff, space->device().tag(), cpu_get_pc(&space->device()));
			}
		}
	}
	if(offset >= 0x20 && state->system24temp_sys16_io_iod_w)
		state->system24temp_sys16_io_iod_w(space, offset & 0x1f, data, mem_mask);
}
