#include "emu.h"
#include "includes/system16.h"
#include "sound/upd7759.h"

static void patch_codeX( UINT16 *mem, offs_t offset, int data )
{
	int aligned_offset = offset&0xfffffe;
	int old_word = mem[aligned_offset/2];

	if( offset&1 )
		data = (old_word&0xff00)|data;
	else
		data = (old_word&0x00ff)|(data<<8);

	mem[aligned_offset/2] = data;
}

void sys16_patch_code( running_machine *machine, const sys16_patch *data, int count )
{
	int i;
	UINT16 *mem = (UINT16 *)memory_region(machine, "maincpu");
	for (i=0; i<count; i++)
		patch_codeX(mem, data[i].offset, data[i].data);
}


MACHINE_RESET( sys16_onetime )
{
	sys16_bg1_trans=0;
	sys16_rowscroll_scroll=0;
	sys18_splittab_bg_x=0;
	sys18_splittab_bg_y=0;
	sys18_splittab_fg_x=0;
	sys18_splittab_fg_y=0;
}

