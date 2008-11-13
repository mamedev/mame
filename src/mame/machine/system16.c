#include "driver.h"
#include "deprecat.h"
#include "system16.h"
#include "sound/upd7759.h"

UINT16 *sys16_workingram;
UINT16 *sys16_workingram2;
UINT16 *sys16_extraram;
UINT16 *sys16_extraram2;
UINT16 *sys16_extraram3;

static void patch_codeX( int offset, int data, const char *cpu ){
	int aligned_offset = offset&0xfffffe;
	UINT16 *mem = (UINT16 *)memory_region(Machine, cpu);
	int old_word = mem[aligned_offset/2];

	if( offset&1 )
		data = (old_word&0xff00)|data;
	else
		data = (old_word&0x00ff)|(data<<8);

	mem[aligned_offset/2] = data;
}

void sys16_patch_code( int offset, int data ){
	patch_codeX(offset,data,"main");
}


MACHINE_RESET( sys16_onetime ){
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

static void sound_cause_nmi( int chip ){
	/* upd7759 callback */
	cpu_set_input_line(Machine->cpu[1], INPUT_LINE_NMI, PULSE_LINE);
}


const upd7759_interface sys16_upd7759_interface =
{
	sound_cause_nmi
};

int sys18_sound_info[4*2];

