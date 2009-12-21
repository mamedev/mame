#include "driver.h"
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

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

GFXDECODE_START( sys16 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,	0, 1024 )
GFXDECODE_END


/* sound */

static void sound_cause_nmi( const device_config *device, int chip )
{
	/* upd7759 callback */
	cputag_set_input_line(device->machine, "soundcpu", INPUT_LINE_NMI, PULSE_LINE);
}


const upd7759_interface sys16_upd7759_interface =
{
	sound_cause_nmi
};

int sys18_sound_info[4*2];

