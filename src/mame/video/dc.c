/*
    dc.c - Dreamcast video emulation

*/

#include "driver.h"
#include "dc.h"
#include "cpu/sh4/sh4.h"

#define DEBUG_FIFO_POLY (0)
#define DEBUG_PVRCTRL	(0)
#define DEBUG_PVRTA	(0)
#define DEBUG_PVRTA_REGS (0)
#define DEBUG_PVRDLIST	(0)
#define DEBUG_VERTICES	(0)

static UINT32 pvrctrl_regs[0x100/4];
static UINT32 pvrta_regs[0x2000/4];
static const int pvr_parconfseq[] = {1,2,3,2,3,4,5,6,5,6,7,8,9,10,11,12,13,14,13,14,15,16,17,16,17,0,0,0,0,0,18,19,20,19,20,21,22,23,22,23};
static const int pvr_wordsvertex[24]  = {8,8,8,8,8,16,16,8,8,8, 8, 8,8,8,8,8,16,16, 8,16,16,8,16,16};
static const int pvr_wordspolygon[24] = {8,8,8,8,8, 8, 8,8,8,8,16,16,8,8,8,8, 8, 8,16,16,16,8, 8, 8};
static int pvr_parameterconfig[64];
static UINT32 dilated0[15][1024];
static UINT32 dilated1[15][1024];
static int dilatechose[64];

UINT64 *dc_texture_ram;
static UINT32 tafifo_buff[32];

static emu_timer *vbout_timer;

typedef struct {
	int tafifo_pos, tafifo_mask, tafifo_vertexwords, tafifo_listtype;
	int start_render_received;
	int listtype_used;
	int alloc_ctrl_OPB_Mode, alloc_ctrl_PT_OPB, alloc_ctrl_TM_OPB, alloc_ctrl_T_OPB, alloc_ctrl_OM_OPB, alloc_ctrl_O_OPB;
	struct testsprites
	{
		int positionx, positiony;
		int sizex, sizey;
		UINT32 textureaddress;
		float u, v, du, dv;
		int texturemode;
		int texturesizex, texturesizey, texturesizes, texturepf, texturepalette;
	} showsprites[2048];
#if DEBUG_VERTICES
	struct testvertices
	{
		int x;
		int y;
		int endofstrip;
	} showvertices[65536];
#endif
	int testsprites_size, toerasesprites, testvertices_size;
	UINT32 paracontrol,paratype,endofstrip,listtype,global_paratype,parameterconfig;
	UINT32 groupcontrol,groupen,striplen,userclip;
	UINT32 objcontrol,shadow,volume,coltype,texture,offfset,gouraud,uv16bit;
	UINT32 textureusize,texturevsize,texturesizes,textureaddress,scanorder,pixelformat;
	UINT32 srcalphainstr,dstalphainstr,srcselect,dstselect,fogcontrol,colorclamp,usealpha;
	UINT32 ignoretexalpha,flipuv,clampuv,filtermode,sstexture,mmdadjust,tsinstruction;
	UINT32 depthcomparemode,cullingmode,zwritedisable,cachebypass,dcalcctrl,volumeinstruction,mipmapped,vqcompressed,strideselect,paletteselector;
} pvrta_state;

static pvrta_state state_ta;

// register decode helper
INLINE int decode_reg_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		assert_always(0, "Wrong mask!\n");
	}

	if (mem_mask == U64(0xffffffff00000000))
	{
		reg++;
		*shift = 32;
 	}

	return reg;
}

READ64_HANDLER( pvr_ctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: [%08x] read %x @ %x (reg %x), mask %llx (PC=%x)\n", 0x5f7c00+reg*4, pvrctrl_regs[reg], offset, reg, mem_mask, activecpu_get_pc());
	#endif

	return (UINT64)pvrctrl_regs[reg] << shift;
}

WRITE64_HANDLER( pvr_ctrl_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: [%08x=%x] write %llx to %x (reg %x), mask %llx\n", 0x5f7c00+reg*4, dat, data>>shift, offset, reg, mem_mask);
	#endif

	pvrctrl_regs[reg] |= dat;
}

READ64_HANDLER( pvr_ta_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	switch (reg)
	{
	case SPG_STATUS:
		pvrta_regs[reg] = (video_screen_get_vblank(machine->primary_screen) << 13) | (video_screen_get_hblank(machine->primary_screen) << 12) | (video_screen_get_vpos(machine->primary_screen) & 0x3ff);
		break;
	}

	#if DEBUG_PVRTA_REGS
	mame_printf_verbose("PVRTA: [%08x] read %x @ %x (reg %x), mask %llx (PC=%x)\n", 0x5f8000+reg*4, pvrta_regs[reg], offset, reg, mem_mask, activecpu_get_pc());
	#endif
	return (UINT64)pvrta_regs[reg] << shift;
}

WRITE64_HANDLER( pvr_ta_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = pvrta_regs[reg];
	pvrta_regs[reg] = dat; // 5f8000+reg*4=dat
	switch (reg)
	{
	case SOFTRESET:
		if (dat & 1)
		{
			#if DEBUG_PVRTA
			mame_printf_verbose("pvr_ta_w:  TA soft reset\n");
			#endif
			state_ta.listtype_used=0;
		}
		if (dat & 2)
		{
			#if DEBUG_PVRTA
			mame_printf_verbose("pvr_ta_w:  Core Pipeline soft reset\n");
			#endif
			if (state_ta.start_render_received == 1)
				state_ta.start_render_received = 0;
		}
		if (dat & 4)
		{
			#if DEBUG_PVRTA
			mame_printf_verbose("pvr_ta_w:  sdram I/F soft reset\n");
			#endif
		}
		break;
	case STARTRENDER:
		#if DEBUG_PVRTA
		mame_printf_verbose("Start Render Received:\n");
		mame_printf_verbose("  Region Array at %08x\n",pvrta_regs[REGION_BASE]);
		mame_printf_verbose("  ISP/TSP Parameters at %08x\n",pvrta_regs[PARAM_BASE]);
		#endif
		state_ta.start_render_received=1;
		break;
	case TA_LIST_INIT:
		state_ta.tafifo_pos=0;
		state_ta.tafifo_mask=7;
		state_ta.tafifo_vertexwords=8;
		state_ta.tafifo_listtype= -1;
		state_ta.toerasesprites=1;
	#if DEBUG_PVRTA
		mame_printf_verbose("TA_OL_BASE       %08x TA_OL_LIMIT  %08x\n", pvrta_regs[TA_OL_BASE], pvrta_regs[TA_OL_LIMIT]);
		mame_printf_verbose("TA_ISP_BASE      %08x TA_ISP_LIMIT %08x\n", pvrta_regs[TA_ISP_BASE], pvrta_regs[TA_ISP_LIMIT]);
		mame_printf_verbose("TA_ALLOC_CTRL    %08x\n", pvrta_regs[TA_ALLOC_CTRL]);
		mame_printf_verbose("TA_NEXT_OPB_INIT %08x\n", pvrta_regs[TA_NEXT_OPB_INIT]);
	#endif
		pvrta_regs[TA_NEXT_OPB] = pvrta_regs[TA_NEXT_OPB_INIT];
		pvrta_regs[TA_ITP_CURRENT] = pvrta_regs[TA_ISP_BASE];
		state_ta.alloc_ctrl_OPB_Mode = pvrta_regs[TA_ALLOC_CTRL] & 0x100000; // 0 up 1 down
		state_ta.alloc_ctrl_PT_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 16) & 3)) & 0x38; // number of 32 bit words (0,8,16,32)
		state_ta.alloc_ctrl_TM_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 12) & 3)) & 0x38;
		state_ta.alloc_ctrl_T_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 8) & 3)) & 0x38;
		state_ta.alloc_ctrl_OM_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 4) & 3)) & 0x38;
		state_ta.alloc_ctrl_O_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 0) & 3)) & 0x38;
		state_ta.listtype_used |= (1+4);
		break;
	case TA_LIST_CONT:
	#if DEBUG_PVRTA
		mame_printf_verbose("List continuation processing\n");
	#endif
		state_ta.tafifo_listtype= -1; // no list being received
		state_ta.listtype_used |= (1+4);
		break;
	}

	#if DEBUG_PVRTA_REGS
	mame_printf_verbose("PVRTA: [%08x=%x] write %llx to %x (reg %x %x), mask %llx\n", 0x5f8000+reg*4, dat, data>>shift, offset, reg, (reg*4)+0x8000, mem_mask);
	#endif
}

WRITE64_HANDLER( ta_fifo_poly_w )
{
	UINT32 a;

	if (mem_mask == U64(0xffffffffffffffff)) 	// 64 bit
	{
		tafifo_buff[state_ta.tafifo_pos]=(UINT32)data;
		tafifo_buff[state_ta.tafifo_pos+1]=(UINT32)(data >> 32);
		#if DEBUG_FIFO_POLY
		mame_printf_debug("ta_fifo_poly_w:  Unmapped write64 %08x = %llx -> %08x %08x\n", 0x10000000+offset*8, data, tafifo_buff[tafifo_pos], tafifo_buff[tafifo_pos+1]);
		#endif
		state_ta.tafifo_pos += 2;
	}
	else
	{
		mame_printf_debug("ta_fifo_poly_w:  Only 64 bit writes supported!\n");
	}

	state_ta.tafifo_pos &= state_ta.tafifo_mask;
	if (state_ta.tafifo_pos == 0)
	{
		state_ta.paracontrol=(tafifo_buff[0] >> 24) & 0xff;
		// 0 end of list
		// 1 user tile clip
		// 2 object list set
		// 3 reserved
		// 4 polygon/modifier volume
		// 5 sprite
		// 6 reserved
		// 7 vertex
		state_ta.paratype=(state_ta.paracontrol >> 5) & 7;
		state_ta.endofstrip=(state_ta.paracontrol >> 4) & 1;
		state_ta.listtype=(state_ta.paracontrol >> 0) & 7;
		if ((state_ta.paratype >= 4) && (state_ta.paratype <= 6))
		{
			state_ta.global_paratype = state_ta.paratype;
			state_ta.groupcontrol=(tafifo_buff[0] >> 16) & 0xff;
			state_ta.groupen=(state_ta.groupcontrol >> 7) & 1;
			state_ta.striplen=(state_ta.groupcontrol >> 2) & 3;
			state_ta.userclip=(state_ta.groupcontrol >> 0) & 3;
			state_ta.objcontrol=(tafifo_buff[0] >> 0) & 0xffff;
			state_ta.shadow=(state_ta.objcontrol >> 7) & 1;
			state_ta.volume=(state_ta.objcontrol >> 6) & 1;
			state_ta.coltype=(state_ta.objcontrol >> 4) & 3;
			state_ta.texture=(state_ta.objcontrol >> 3) & 1;
			state_ta.offfset=(state_ta.objcontrol >> 2) & 1;
			state_ta.gouraud=(state_ta.objcontrol >> 1) & 1;
			state_ta.uv16bit=(state_ta.objcontrol >> 0) & 1;
		}

		if (state_ta.toerasesprites == 1)
		{
			state_ta.toerasesprites=0;
			state_ta.testsprites_size=0;
#if DEBUG_VERTICES
			state_ta.testvertices_size=0;
#endif
		}

		// check if we need 8 words more
		if (state_ta.tafifo_mask == 7)
		{
			state_ta.parameterconfig = pvr_parameterconfig[state_ta.objcontrol & 0x3d];
			// decide number of words per vertex
			if (state_ta.paratype == 7)
			{
				if ((state_ta.global_paratype == 5) || (state_ta.tafifo_listtype == 1) || (state_ta.tafifo_listtype == 3))
					state_ta.tafifo_vertexwords = 16;
				if (state_ta.tafifo_vertexwords == 16)
				{
					state_ta.tafifo_mask = 15;
					state_ta.tafifo_pos = 8;
					return;
				}
			}
			state_ta.tafifo_vertexwords=pvr_wordsvertex[state_ta.parameterconfig];
			if ((state_ta.paratype == 4) && ((state_ta.listtype != 1) && (state_ta.listtype != 3)))
				if (pvr_wordspolygon[state_ta.parameterconfig] == 16)
				{
					state_ta.tafifo_mask = 15;
					state_ta.tafifo_pos = 8;
					return;
				}
		}
		else
		{
			state_ta.tafifo_mask = 7;
		}

		// now we heve all the needed words
		// interpret their meaning
		if (state_ta.paratype == 0)
		{ // end of list
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type 0 End of List\n");
			#endif
			a=0; // 6-10 0-3
			switch (state_ta.tafifo_listtype)
			{
			case 0:
				a = 1 << 7;
				break;
			case 1:
				a = 1 << 8;
				break;
			case 2:
				a = 1 << 9;
				break;
			case 3:
				a = 1 << 10;
				break;
			case 4:
				a = 1 << 21;
				break;
			}
			dc_sysctrl_regs[SB_ISTNRM] |= a;
			update_interrupt_status();
			state_ta.tafifo_listtype= -1; // no list being received
			state_ta.listtype_used |= (2+8);
		}
		else if (state_ta.paratype == 1)
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type 1 User Tile Clip\n");
			mame_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
			#endif
		}
		else if (state_ta.paratype == 2)
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type 2 Object List Set at %08x\n", tafifo_buff[1]);
			mame_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
			#endif
		}
		else if (state_ta.paratype == 3)
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type %x Unknown!\n", tafifo_buff[0]);
			#endif
		}
		else
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type %d End of Strip %d", state_ta.paratype, state_ta.endofstrip);
			if (state_ta.listtype_used & 3)
				mame_printf_verbose(" List Type %d", state_ta.listtype);
			mame_printf_verbose("\n");
			#endif

			// set type of list currently being recieved
			if ((state_ta.paratype == 4) || (state_ta.paratype == 5) || (state_ta.paratype == 6))
			{
				if (state_ta.tafifo_listtype < 0)
				{
					state_ta.tafifo_listtype = state_ta.listtype;
				}
			}
			state_ta.listtype_used = state_ta.listtype_used ^ (state_ta.listtype_used & 3);

			if ((state_ta.paratype == 4) || (state_ta.paratype == 5))
			{ // quad or polygon
				state_ta.depthcomparemode=(tafifo_buff[1] >> 29) & 7;
				state_ta.cullingmode=(tafifo_buff[1] >> 27) & 3;
				state_ta.zwritedisable=(tafifo_buff[1] >> 26) & 1;
				state_ta.cachebypass=(tafifo_buff[1] >> 21) & 1;
				state_ta.dcalcctrl=(tafifo_buff[1] >> 20) & 1;
				state_ta.volumeinstruction=(tafifo_buff[1] >> 29) & 7;
				state_ta.textureusize=1 << (3+((tafifo_buff[2] >> 3) & 7));
				state_ta.texturevsize=1 << (3+(tafifo_buff[2] & 7));
				state_ta.texturesizes=tafifo_buff[2] & 0x3f;
				state_ta.srcalphainstr=(tafifo_buff[2] >> 29) & 7;
				state_ta.dstalphainstr=(tafifo_buff[2] >> 26) & 7;
				state_ta.srcselect=(tafifo_buff[2] >> 25) & 1;
				state_ta.dstselect=(tafifo_buff[2] >> 24) & 1;
				state_ta.fogcontrol=(tafifo_buff[2] >> 22) & 3;
				state_ta.colorclamp=(tafifo_buff[2] >> 21) & 1;
				state_ta.usealpha=(tafifo_buff[2] >> 20) & 1;
				state_ta.ignoretexalpha=(tafifo_buff[2] >> 19) & 1;
				state_ta.flipuv=(tafifo_buff[2] >> 17) & 3;
				state_ta.clampuv=(tafifo_buff[2] >> 15) & 3;
				state_ta.filtermode=(tafifo_buff[2] >> 13) & 3;
				state_ta.sstexture=(tafifo_buff[2] >> 12) & 1;
				state_ta.mmdadjust=(tafifo_buff[2] >> 8) & 1;
				state_ta.tsinstruction=(tafifo_buff[2] >> 6) & 3;
				if (state_ta.texture == 1)
				{
					state_ta.textureaddress=(tafifo_buff[3] & 0x1FFFFF) << 3;
					state_ta.scanorder=(tafifo_buff[3] >> 26) & 1;
					state_ta.pixelformat=(tafifo_buff[3] >> 27) & 7;
					state_ta.mipmapped=(tafifo_buff[3] >> 31) & 1;
					state_ta.vqcompressed=(tafifo_buff[3] >> 30) & 1;
					state_ta.strideselect=(tafifo_buff[3] >> 25) & 1;
					state_ta.paletteselector=(tafifo_buff[3] >> 21) & 0x3F;
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Texture %d x %d at %08x format %d\n", state_ta.textureusize, state_ta.texturevsize, (tafifo_buff[3] & 0x1FFFFF) << 3, state_ta.pixelformat);
					#endif
				}
				if (state_ta.paratype == 4)
				{ // polygon or mv
					if ((state_ta.tafifo_listtype == 1) || (state_ta.tafifo_listtype == 3))
					{
					#if DEBUG_PVRDLIST
						mame_printf_verbose(" Modifier Volume\n");
					#endif
					}
					else
					{
					#if DEBUG_PVRDLIST
						mame_printf_verbose(" Polygon\n");
					#endif
					}
				}
				if (state_ta.paratype == 5)
				{ // quad
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Sprite\n");
					#endif
				}
			}

			if (state_ta.paratype == 7)
			{ // vertex
				if ((state_ta.tafifo_listtype == 1) || (state_ta.tafifo_listtype == 3))
				{
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Vertex modifier volume");
					mame_printf_verbose(" A(%f,%f,%f) B(%f,%f,%f) C(%f,%f,%f)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]),
						u2f(tafifo_buff[3]), u2f(tafifo_buff[4]), u2f(tafifo_buff[5]), u2f(tafifo_buff[6]), u2f(tafifo_buff[7]),
						u2f(tafifo_buff[8]), u2f(tafifo_buff[9]));
					mame_printf_verbose("\n");
					#endif
				}
				else if (state_ta.global_paratype == 5)
				{
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Vertex sprite");
					mame_printf_verbose(" A(%f,%f,%f) B(%f,%f,%f) C(%f,%f,%f) D(%f,%f,)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]),
						u2f(tafifo_buff[3]), u2f(tafifo_buff[4]), u2f(tafifo_buff[5]), u2f(tafifo_buff[6]), u2f(tafifo_buff[7]),
						u2f(tafifo_buff[8]), u2f(tafifo_buff[9]), u2f(tafifo_buff[10]), u2f(tafifo_buff[11]));
					mame_printf_verbose("\n");
					#endif
					if (state_ta.texture == 1)
					{
						#if DEBUG_PVRDLIST
						mame_printf_verbose(" A(%f,%f) B(%f,%f) C(%f,%f)\n",u2f(tafifo_buff[13] & 0xffff0000),u2f((tafifo_buff[13] & 0xffff) << 16),u2f(tafifo_buff[14] & 0xffff0000),u2f((tafifo_buff[14] & 0xffff) << 16),u2f(tafifo_buff[15] & 0xffff0000),u2f((tafifo_buff[15] & 0xffff) << 16));
						#endif
/* test video start */
						state_ta.showsprites[state_ta.testsprites_size].positionx=u2f(tafifo_buff[1]);
						state_ta.showsprites[state_ta.testsprites_size].positiony=u2f(tafifo_buff[2]);
						state_ta.showsprites[state_ta.testsprites_size].sizex=u2f(tafifo_buff[4])-u2f(tafifo_buff[1]);
						state_ta.showsprites[state_ta.testsprites_size].sizey=u2f(tafifo_buff[8])-u2f(tafifo_buff[2]);
						state_ta.showsprites[state_ta.testsprites_size].u=u2f(tafifo_buff[13] & 0xffff0000);
						state_ta.showsprites[state_ta.testsprites_size].v=u2f((tafifo_buff[13] & 0xffff) << 16);
						state_ta.showsprites[state_ta.testsprites_size].du=u2f(tafifo_buff[14] & 0xffff0000)-state_ta.showsprites[state_ta.testsprites_size].u;
						state_ta.showsprites[state_ta.testsprites_size].dv=u2f((tafifo_buff[15] & 0xffff) << 16)-state_ta.showsprites[state_ta.testsprites_size].v;
						state_ta.showsprites[state_ta.testsprites_size].textureaddress=state_ta.textureaddress;
						state_ta.showsprites[state_ta.testsprites_size].texturesizex=state_ta.textureusize;
						state_ta.showsprites[state_ta.testsprites_size].texturesizey=state_ta.texturevsize;
						state_ta.showsprites[state_ta.testsprites_size].texturemode=state_ta.scanorder+state_ta.vqcompressed*2;
						state_ta.showsprites[state_ta.testsprites_size].texturesizes=state_ta.texturesizes;
						state_ta.showsprites[state_ta.testsprites_size].texturepf=state_ta.pixelformat;
						state_ta.showsprites[state_ta.testsprites_size].texturepalette=state_ta.paletteselector;
						state_ta.testsprites_size=state_ta.testsprites_size+1;
/* test video end */
					}
				}
				else if (state_ta.global_paratype == 4)
				{
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Vertex polygon");
					mame_printf_verbose(" V(%f,%f,%f)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]), u2f(tafifo_buff[3]));
					mame_printf_verbose("\n");
					#endif
#if DEBUG_VERTICES
					if (state_ta.testvertices_size < 65530)
					{
						state_ta.showvertices[state_ta.testvertices_size].x=u2f(tafifo_buff[1]);
						state_ta.showvertices[state_ta.testvertices_size].y=u2f(tafifo_buff[2]);
						state_ta.showvertices[state_ta.testvertices_size].endofstrip=state_ta.endofstrip;
					}
					state_ta.testvertices_size=state_ta.testvertices_size+1;
#endif
				}
			}
		}
	} // if (state_ta.tafifo_pos == 0)
}

WRITE64_HANDLER( ta_fifo_yuv_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	mame_printf_verbose("YUV FIFO: [%08x=%x] write %llx to %x, mask %llx\n", 0x10800000+reg*4, dat, data, offset, mem_mask);
}

/* test video start */
static UINT32 dilate0(UINT32 value,int bits) // dilate first "bits" bits in "value"
{
	UINT32 x,m1,m2,m3;
	int a;

	x = value;
	for (a=0;a < bits;a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + (x & m2) + ((x & m3) << 1);
	}
	return x;
}

static UINT32 dilate1(UINT32 value,int bits) // dilate first "bits" bits in "value"
{
	UINT32 x,m1,m2,m3;
	int a;

	x = value;
	for (a=0;a < bits;a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + ((x & m2) << 1) + ((x & m3) << 1);
	}
	return x;
}

static void computedilated(void)
{
	int a,b;

	for (b=0;b <= 14;b++)
		for (a=0;a < 1024;a++) {
			dilated0[b][a]=dilate0(a,b);
			dilated1[b][a]=dilate1(a,b);
		}
	for (b=0;b <= 7;b++)
		for (a=0;a < 7;a++)
			dilatechose[(b << 3) + a]=3+(a < b ? a : b);
}

#if DEBUG_VERTICES
static void testdrawline(bitmap_t *bitmap,int from,int to)
{
}
#endif

static void testdrawscreen(bitmap_t *bitmap,const rectangle *cliprect)
{
	int cs,x,y,dx,dy,xi,yi,a;
	float iu,iv,u,v;
	UINT32 addrp;
	UINT32 *bmpaddr;
	int xt,yt,cd;
	UINT32 c;

	c=pvrta_regs[ISP_BACKGND_T];
	fillbitmap(bitmap,program_read_dword_64le(0x05000000+((c&0xfffff8)>>1)+(3+3)*4),cliprect);

	for (cs=0;cs < state_ta.testsprites_size;cs++)
	{
		dx=state_ta.showsprites[cs].sizex;
		dy=state_ta.showsprites[cs].sizey;
		iu=state_ta.showsprites[cs].du/dx;
		iv=state_ta.showsprites[cs].dv/dy;
		cd=dilatechose[state_ta.showsprites[cs].texturesizes];

		if ((state_ta.showsprites[cs].positionx+dx) > 640)
			dx=640-state_ta.showsprites[cs].positionx;
		if ((state_ta.showsprites[cs].positiony+dy) > 480)
			dy=480-state_ta.showsprites[cs].positiony;
		xi=0;
		yi=0;

		if (state_ta.showsprites[cs].positionx < 0)
			xi=-state_ta.showsprites[cs].positionx;
		if (state_ta.showsprites[cs].positiony < 0)
			yi=-state_ta.showsprites[cs].positiony;

		for (y = yi;y < dy;y++)
		{
			for (x = xi;x < dx;x++)
			{
				// find the coordinates
				u=state_ta.showsprites[cs].u+iu*x;
				v=state_ta.showsprites[cs].v+iv*y;
				yt=v*(state_ta.showsprites[cs].texturesizey-1);
				xt=u*(state_ta.showsprites[cs].texturesizex-1);

				switch (state_ta.showsprites[cs].texturepf)
				{
				case 0: // 1555
					// find the address
					if (state_ta.showsprites[cs].texturemode == 1)
						addrp=state_ta.showsprites[cs].textureaddress+(state_ta.showsprites[cs].texturesizex*yt+xt)*2;
					else if (state_ta.showsprites[cs].texturemode == 0) // twiddled
						addrp=state_ta.showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt])*2;
					else // vq-compressed
					{
						c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 1]);
						c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(state_ta.showsprites[cs].textureaddress+c));
						addrp=state_ta.showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 1])*2;
					}
					// read datum
					c=*(((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1));
					// find the color and draw
					a=(((c & 0x8000) >> 8)*255)/0x80;
					bmpaddr=BITMAP_ADDR32(bitmap,state_ta.showsprites[cs].positiony+y,state_ta.showsprites[cs].positionx+x);
					*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0x7c00) >> 7, (c&0x3e0) >> 2, (c&0x1f) << 3), a);
					break;
				case 1: // 565
					// find the address
					if (state_ta.showsprites[cs].texturemode == 1)
						addrp=state_ta.showsprites[cs].textureaddress+(state_ta.showsprites[cs].texturesizex*yt+xt)*2;
					else if (state_ta.showsprites[cs].texturemode == 0) // twiddled
						addrp=state_ta.showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt])*2;
					else // vq-compressed
					{
						c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 1]);
						c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(state_ta.showsprites[cs].textureaddress+c));
						addrp=state_ta.showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 1])*2;
					}
					// read datum
					c=*(((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1));
					// find the color and draw
					bmpaddr=BITMAP_ADDR32(bitmap,state_ta.showsprites[cs].positiony+y,state_ta.showsprites[cs].positionx+x);
					*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0xf800) >> 8, (c&0x7e0) >> 3, (c&0x1f) << 3), 255);
					break;
				case 2: // 4444
					// find the address
					if (state_ta.showsprites[cs].texturemode == 1)
						addrp=state_ta.showsprites[cs].textureaddress+(state_ta.showsprites[cs].texturesizex*yt+xt)*2;
					else if (state_ta.showsprites[cs].texturemode == 0) // twiddled
						addrp=state_ta.showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt])*2;
					else // vq-compressed
					{
						c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 1]);
						c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(state_ta.showsprites[cs].textureaddress+c));
						addrp=state_ta.showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 1])*2;
					}
					// read datum
					c=*(((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1));
					// find the color and draw
					a=(((c & 0xf000) >> 8)*255)/0xf0;
					bmpaddr=BITMAP_ADDR32(bitmap,state_ta.showsprites[cs].positiony+y,state_ta.showsprites[cs].positionx+x);
					*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0xf00) >> 4, c&0xf0, (c&0xf) << 4), a);
					break;
				case 3: // yuv422
					break;
				case 4: // bumpmap
					break;
				case 5: // 4 bpp palette
					break;
				case 6: // 8 bpp palette
					if (state_ta.showsprites[cs].texturemode & 2) // vq-compressed
					{
						c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 2]);
						c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(state_ta.showsprites[cs].textureaddress+c));
						addrp=state_ta.showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 3]);
					}
					else
					{
						addrp=state_ta.showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt]);
					}
					c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(addrp));
					c=((state_ta.showsprites[cs].texturepalette & 0x30) << 4) | c;
					c=pvrta_regs[0x1000/4+c];
					switch (pvrta_regs[PAL_RAM_CTRL])
					{
					case 0: // argb1555
						a=(((c & 0x8000) >> 8)*255)/0x80;
						bmpaddr=BITMAP_ADDR32(bitmap,state_ta.showsprites[cs].positiony+y,state_ta.showsprites[cs].positionx+x);
						*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0x7c00) >> 7, (c&0x3e0) >> 2, (c&0x1f) << 3), a);
						break;
					case 1: // rgb565
						bmpaddr=BITMAP_ADDR32(bitmap,state_ta.showsprites[cs].positiony+y,state_ta.showsprites[cs].positionx+x);
						*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0xf800) >> 8, (c&0x7e0) >> 3, (c&0x1f) << 3), 255);
						break;
					case 2: // argb4444
						a=(((c & 0xf000) >> 8)*255)/0xf0;
						bmpaddr=BITMAP_ADDR32(bitmap,state_ta.showsprites[cs].positiony+y,state_ta.showsprites[cs].positionx+x);
						*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0xf00) >> 4, c&0xf0, (c&0xf) << 4), a);
						break;
					case 3: // argb8888
						a=(c & 0xff000000) >> 24;
						bmpaddr=BITMAP_ADDR32(bitmap,state_ta.showsprites[cs].positiony+y,state_ta.showsprites[cs].positionx+x);
						*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0xff0000) >> 16, (c&0xff00) >> 8, c&0xff), a);
						break;
					}
					break;
				case 7: // reserved
					break;
				}
			}
		}
	}
#if DEBUG_VERTICES
	xi = yi = -1;
	a = state_ta.testvertices_size;
	if (a > 65535)
		a = 65535;
	for (cs=0;cs < a;cs++)
	{
		xi=yi;
		yi=cs;
		if (xi >= 0)
			testdrawline(bitmap,xi,yi);  // draw a segment from vertex xi to vertex yi
		if (state_ta.showvertices[cs].endofstrip == 1)
			xi = yi = -1;
	}
#endif
}
/* test video end */

static void pvr_build_parameterconfig(void)
{
	int a,b,c,d,e,p;

	for (a = 0;a <= 63;a++)
		pvr_parameterconfig[a] = -1;
	p=0;
	// volume,col_type,texture,offset,16bit_uv
	for (a = 0;a <= 1;a++)
		for (b = 0;b <= 3;b++)
			for (c = 0;c <= 1;c++)
				if (c == 0)
				{
					for (d = 0;d <= 1;d++)
						for (e = 0;e <= 1;e++)
							pvr_parameterconfig[(a << 6) | (b << 4) | (c << 3) | (d << 2) | (e << 0)] = pvr_parconfseq[p];
					p++;
				}
				else
					for (d = 0;d <= 1;d++)
						for (e = 0;e <= 1;e++)
						{
							pvr_parameterconfig[(a << 6) | (b << 4) | (c << 3) | (d << 2) | (e << 0)] = pvr_parconfseq[p];
							p++;
						}
	for (a = 1;a <= 63;a++)
		if (pvr_parameterconfig[a] < 0)
			pvr_parameterconfig[a] = pvr_parameterconfig[a-1];
}

static TIMER_CALLBACK(vbout)
{
	dc_sysctrl_regs[SB_ISTNRM] |= IST_VBL_OUT; // V Blank-out interrupt
	update_interrupt_status();

	timer_adjust_oneshot(vbout_timer, attotime_never, 0);
}

VIDEO_START(dc)
{
	memset(pvrctrl_regs, 0, sizeof(pvrctrl_regs));
	memset(pvrta_regs, 0, sizeof(pvrta_regs));
	pvr_build_parameterconfig();

	// if the next 2 registers do not have the correct values, the naomi bios will hang
	pvrta_regs[PVRID]=0x17fd11db;
	pvrta_regs[REVISION]=0x11;
	pvrta_regs[VO_CONTROL]=0xC;
	pvrta_regs[SOFTRESET]=0x7;
	state_ta.tafifo_pos=0;
	state_ta.tafifo_mask=7;
	state_ta.tafifo_vertexwords=8;
	state_ta.tafifo_listtype= -1;
	state_ta.start_render_received=0;

	state_ta.testsprites_size=0;
	state_ta.toerasesprites=0;
	state_ta.testvertices_size=0;
	computedilated();

	vbout_timer = timer_alloc(vbout, 0);
	timer_adjust_oneshot(vbout_timer, attotime_never, 0);
}

VIDEO_UPDATE(dc)
{
	if (pvrta_regs[VO_CONTROL] & (1 << 3))
	{
		fillbitmap(bitmap,pvrta_regs[VO_BORDER_COL] & 0xFFFFFF,cliprect);
		return 0;
	}

	testdrawscreen(bitmap,cliprect);

	if (state_ta.start_render_received)
	{
		state_ta.start_render_received=0;
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOR_TSP;	// TSP end of render
		update_interrupt_status();
	}
	return 0;
}

void dc_vblank(running_machine *machine)
{
	dc_sysctrl_regs[SB_ISTNRM] |= IST_VBL_IN; // V Blank-in interrupt
	update_interrupt_status();

	timer_adjust_oneshot(vbout_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);
}

