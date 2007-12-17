#include "driver.h"
#include "system16.h"
#include "includes/system24.h"

/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

UINT16 *system24temp_sys16_shared_ram;
READ16_HANDLER( system24temp_sys16_shared_ram_r )
{
	return system24temp_sys16_shared_ram[offset];
}

WRITE16_HANDLER( system24temp_sys16_shared_ram_w )
{
	COMBINE_DATA(system24temp_sys16_shared_ram + offset);
}

/* The 315-5296
     8 8bits I/O ports, 3 output-only pins, some protection, and
     external/daughterboard chip selection on half of the address
     range
*/

static UINT8  (*system24temp_sys16_io_io_r)(int port);
static void   (*system24temp_sys16_io_io_w)(int port, UINT8 data);
static void   (*system24temp_sys16_io_cnt_w)(UINT8 data);
static READ16_HANDLER ((*system24temp_sys16_io_iod_r));
static WRITE16_HANDLER((*system24temp_sys16_io_iod_w));
static UINT8 system24temp_sys16_io_cnt, system24temp_sys16_io_dir;

void system24temp_sys16_io_set_callbacks(UINT8 (*io_r)(int port),
							  void  (*io_w)(int port, UINT8 data),
							  void  (*cnt_w)(UINT8 data),
							  READ16_HANDLER ((*iod_r)),
							  WRITE16_HANDLER((*iod_w)))
{
	system24temp_sys16_io_io_r = io_r;
	system24temp_sys16_io_io_w = io_w;
	system24temp_sys16_io_cnt_w = cnt_w;
	system24temp_sys16_io_iod_r = iod_r;
	system24temp_sys16_io_iod_w = iod_w;
	system24temp_sys16_io_cnt = 0x00;
	system24temp_sys16_io_dir = 0x00;
}

READ16_HANDLER ( system24temp_sys16_io_r )
{
	//  logerror("IO read %02x (%d:%x)\n", offset, cpu_getactivecpu(), activecpu_get_pc());
	if(offset < 8)
		return system24temp_sys16_io_io_r ? system24temp_sys16_io_io_r(offset) : 0xff;
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
			return system24temp_sys16_io_cnt;
		case 0xf:
			return system24temp_sys16_io_dir;
		default:
			logerror("IO control read %02x (%d:%x)\n", offset, cpu_getactivecpu(), activecpu_get_pc());
			return 0xff;
		}
	} else
		return system24temp_sys16_io_iod_r ? system24temp_sys16_io_iod_r(offset & 0x1f, mem_mask) : 0xff;
}

READ32_HANDLER(system24temp_sys16_io_dword_r)
{
	return system24temp_sys16_io_r(2*offset, mem_mask)|(system24temp_sys16_io_r(2*offset+1, mem_mask>>16)<<16);
}


WRITE16_HANDLER( system24temp_sys16_io_w )
{
	if(ACCESSING_LSB) {
		if(offset < 8) {
			if(!(system24temp_sys16_io_dir & (1 << offset))) {
				logerror("IO port write on input-only port (%d, [%02x], %02x, %d:%x)\n", offset, system24temp_sys16_io_dir, data & 0xff, cpu_getactivecpu(), activecpu_get_pc());
				return;
			}
			if(system24temp_sys16_io_io_w)
				system24temp_sys16_io_io_w(offset, data);
		} else if (offset < 0x20) {
			switch(offset) {
			case 0xe:
				system24temp_sys16_io_cnt = data;
				if(system24temp_sys16_io_cnt_w)
					system24temp_sys16_io_cnt_w(data & 7);
				break;
			case 0xf:
				system24temp_sys16_io_dir = data;
				break;
			default:
				logerror("IO control write %02x, %02x (%d:%x)\n", offset, data & 0xff, cpu_getactivecpu(), activecpu_get_pc());
			}
		}
	}
	if(offset >= 0x20 && system24temp_sys16_io_iod_w)
		system24temp_sys16_io_iod_w(offset & 0x1f, data, mem_mask);
}
