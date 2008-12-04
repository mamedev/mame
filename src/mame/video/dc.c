/*
    dc.c - Dreamcast video emulation

*/

#include "driver.h"
#include "dc.h"
#include "cpu/sh4/sh4.h"
#include "render.h"
#include "rendutil.h"

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
	int testsprites_size, testsprites_toerase, testvertices_size;
	UINT32 ispbase;
	UINT32 fbwsof1;
	UINT32 fbwsof2;
	int busy;
	int valid;
} receiveddata;

typedef struct {
	int tafifo_pos, tafifo_mask, tafifo_vertexwords, tafifo_listtype;
	int start_render_received;
	int renderselect;
	int listtype_used;
	int alloc_ctrl_OPB_Mode, alloc_ctrl_PT_OPB, alloc_ctrl_TM_OPB, alloc_ctrl_T_OPB, alloc_ctrl_OM_OPB, alloc_ctrl_O_OPB;
	receiveddata grab[4];
	int grabsel;
	int grabsellast;
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
	mame_printf_verbose("PVRCTRL: [%08x] read %x @ %x (reg %x), mask %llx (PC=%x)\n", 0x5f7c00+reg*4, pvrctrl_regs[reg], offset, reg, mem_mask, cpu_get_pc(space->cpu));
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

	switch (reg)
	{
	case SB_PDST:
		#if DEBUG_PVRCTRL
		mame_printf_verbose("PVRCTRL: PVR-DMA start\n");
		#endif
		break;
	}

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
		pvrta_regs[reg] = (video_screen_get_vblank(space->machine->primary_screen) << 13) | (video_screen_get_hblank(space->machine->primary_screen) << 12) | (video_screen_get_vpos(space->machine->primary_screen) & 0x3ff);
		break;
	}

	#if DEBUG_PVRTA_REGS
	if (reg != 0x43)
		mame_printf_verbose("PVRTA: [%08x] read %x @ %x (reg %x), mask %llx (PC=%x)\n", 0x5f8000+reg*4, pvrta_regs[reg], offset, reg, mem_mask, cpu_get_pc(space->cpu));
	#endif
	return (UINT64)pvrta_regs[reg] << shift;
}

WRITE64_HANDLER( pvr_ta_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;
	#if DEBUG_PVRTA
	UINT32 sizera,offsetra,v;
	#endif
	int a;

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
			{
				for (a=0;a < 4;a++)
					if (state_ta.grab[a].busy == 1)
						state_ta.grab[a].busy = 0;
				state_ta.start_render_received = 0;
			}
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
		if (pvrta_regs[FPU_PARAM_CFG] & 0x200000)
			sizera=6;
		else
			sizera=5;
		offsetra=pvrta_regs[REGION_BASE];
		for (;;)
		{
			v=memory_read_dword(space,0x05000000+offsetra);
			mame_printf_verbose("Tile X:%d Y:%d\n  ", (v >> 2) & 0x3f, (v >> 8) & 0x3f);
			offsetra = offsetra+4;
			v=memory_read_dword(space,0x05000000+offsetra);
			if (!(v & 0x80000000))
				mame_printf_verbose("OLP %d ",v & 0xFFFFFC);
			offsetra = offsetra+4;
			v=memory_read_dword(space,0x05000000+offsetra);
			if (!(v & 0x80000000))
				mame_printf_verbose("OMVLP %d ",v & 0xFFFFFC);
			offsetra = offsetra+4;
			v=memory_read_dword(space,0x05000000+offsetra);
			if (!(v & 0x80000000))
				mame_printf_verbose("TLP %d ",v & 0xFFFFFC);
			offsetra = offsetra+4;
			v=memory_read_dword(space,0x05000000+offsetra);
			if (!(v & 0x80000000))
				mame_printf_verbose("TMVLP %d ",v & 0xFFFFFC);
			if (sizera == 6)
			{
				offsetra = offsetra+4;
				v=memory_read_dword(space,0x05000000+offsetra);
				if (!(v & 0x80000000))
					mame_printf_verbose("PTLP %d ",v & 0xFFFFFC);
			}
			mame_printf_verbose("\n");
			if (v & 0x80000000)
				break;
		}
		#endif
		// select buffer to draw using PARAM_BASE
		for (a=0;a < 4;a++)
		{
			if ((state_ta.grab[a].ispbase == pvrta_regs[PARAM_BASE]) && (state_ta.grab[a].valid == 1) && (state_ta.grab[a].busy == 0))
			{
				state_ta.grab[a].busy = 1;
				state_ta.renderselect = a;
				state_ta.start_render_received=1;
				state_ta.grab[a].fbwsof1=pvrta_regs[FB_W_SOF1];
				state_ta.grab[a].fbwsof2=pvrta_regs[FB_W_SOF2];
				break;
			}
		}
		if (a != 4)
			break;
		assert_always(0, "TA grabber error!\n");
		break;
	case TA_LIST_INIT:
		state_ta.tafifo_pos=0;
		state_ta.tafifo_mask=7;
		state_ta.tafifo_vertexwords=8;
		state_ta.tafifo_listtype= -1;
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
		// use TA_ISP_BASE and select buffer for grab data
		state_ta.grabsel = -1;
		// try to find already used buffer but not busy
		for (a=0;a < 4;a++)
		{
			if ((state_ta.grab[a].ispbase == pvrta_regs[TA_ISP_BASE]) && (state_ta.grab[a].busy == 0) && (state_ta.grab[a].valid == 1))
			{
				state_ta.grabsel=a;
				break;
			}
		}
		// try a buffer not used yet
		if (state_ta.grabsel < 0)
		{
			for (a=0;a < 4;a++)
			{
				if (state_ta.grab[a].valid == 0)
				{
					state_ta.grabsel=a;
					break;
				}
			}
		}
		// find a non busy buffer starting from the last one used
		if (state_ta.grabsel < 0)
		{
			for (a=0;a < 3;a++)
			{
				if (state_ta.grab[(state_ta.grabsellast+1+a) & 3].busy == 0)
				{
					state_ta.grabsel=a;
					break;
				}
			}
		}
		if (state_ta.grabsel < 0)
			assert_always(0, "TA grabber error!\n");
		state_ta.grabsellast=state_ta.grabsel;
		state_ta.grab[state_ta.grabsel].ispbase=pvrta_regs[TA_ISP_BASE];
		state_ta.grab[state_ta.grabsel].busy=0;
		state_ta.grab[state_ta.grabsel].valid=1;
		state_ta.grab[state_ta.grabsel].testsprites_size=0;
		state_ta.grab[state_ta.grabsel].testvertices_size=0;
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
	if ((reg != 0x14) && (reg != 0x15))
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
		mame_printf_debug("ta_fifo_poly_w:  Unmapped write64 %08x = %llx -> %08x %08x\n", 0x10000000+offset*8, data, tafifo_buff[state_ta.tafifo_pos], tafifo_buff[state_ta.tafifo_pos+1]);
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
		// Para Control
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
			// Group Control
			state_ta.groupcontrol=(tafifo_buff[0] >> 16) & 0xff;
			state_ta.groupen=(state_ta.groupcontrol >> 7) & 1;
			state_ta.striplen=(state_ta.groupcontrol >> 2) & 3;
			state_ta.userclip=(state_ta.groupcontrol >> 0) & 3;
			// Obj Control
			state_ta.objcontrol=(tafifo_buff[0] >> 0) & 0xffff;
			state_ta.shadow=(state_ta.objcontrol >> 7) & 1;
			state_ta.volume=(state_ta.objcontrol >> 6) & 1;
			state_ta.coltype=(state_ta.objcontrol >> 4) & 3;
			state_ta.texture=(state_ta.objcontrol >> 3) & 1;
			state_ta.offfset=(state_ta.objcontrol >> 2) & 1;
			state_ta.gouraud=(state_ta.objcontrol >> 1) & 1;
			state_ta.uv16bit=(state_ta.objcontrol >> 0) & 1;
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
			// decide number of words when not a vertex
			state_ta.tafifo_vertexwords=pvr_wordsvertex[state_ta.parameterconfig];
			if ((state_ta.paratype == 4) && ((state_ta.listtype != 1) && (state_ta.listtype != 3)))
				if (pvr_wordspolygon[state_ta.parameterconfig] == 16)
				{
					state_ta.tafifo_mask = 15;
					state_ta.tafifo_pos = 8;
					return;
				}
		}
		state_ta.tafifo_mask = 7;

		// now we heve all the needed words
		// here we should generate the data for the various tiles
		// for now, just interpret their meaning
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
			dc_update_interrupt_status(space->machine);
			state_ta.tafifo_listtype= -1; // no list being received
			state_ta.listtype_used |= (2+8);
		}
		else if (state_ta.paratype == 1)
		{ // user tile clip
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type 1 User Tile Clip\n");
			mame_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
			#endif
		}
		else if (state_ta.paratype == 2)
		{ // object list set
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
		{ // global parameter or vertex parameter
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type %d", state_ta.paratype);
			if (state_ta.paratype == 7)
				mame_printf_verbose(" End of Strip %d", state_ta.endofstrip);
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
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].positionx=u2f(tafifo_buff[1]);
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].positiony=u2f(tafifo_buff[2]);
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].sizex=u2f(tafifo_buff[4])-u2f(tafifo_buff[1]);
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].sizey=u2f(tafifo_buff[8])-u2f(tafifo_buff[2]);
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].u=u2f(tafifo_buff[13] & 0xffff0000);
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].v=u2f((tafifo_buff[13] & 0xffff) << 16);
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].du=u2f(tafifo_buff[14] & 0xffff0000)-state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].u;
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].dv=u2f((tafifo_buff[15] & 0xffff) << 16)-state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].v;
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].textureaddress=state_ta.textureaddress;
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].texturesizex=state_ta.textureusize;
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].texturesizey=state_ta.texturevsize;
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].texturemode=state_ta.scanorder+state_ta.vqcompressed*2;
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].texturesizes=state_ta.texturesizes;
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].texturepf=state_ta.pixelformat;
						state_ta.grab[state_ta.grabsel].showsprites[state_ta.grab[state_ta.grabsel].testsprites_size].texturepalette=state_ta.paletteselector;
						state_ta.grab[state_ta.grabsel].testsprites_size=state_ta.grab[state_ta.grabsel].testsprites_size+1;
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
					if (state_ta.grab[state_ta.grabsel].testvertices_size <= 65530)
					{
						state_ta.grab[state_ta.grabsel].showvertices[state_ta.grab[state_ta.grabsel].testvertices_size].x=u2f(tafifo_buff[1]);
						state_ta.grab[state_ta.grabsel].showvertices[state_ta.grab[state_ta.grabsel].testvertices_size].y=u2f(tafifo_buff[2]);
						state_ta.grab[state_ta.grabsel].showvertices[state_ta.grab[state_ta.grabsel].testvertices_size].endofstrip=state_ta.endofstrip;
					}
					state_ta.grab[state_ta.grabsel].testvertices_size=state_ta.grab[state_ta.grabsel].testvertices_size+1;
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
static void testdrawline(bitmap_t *bitmap, int index, int from, int to)
{
UINT32 *bmpaddr;
int ix, iy, i, inc, x, y, dx, dy, plotx, ploty;
int dxix, dyiy;
render_bounds line, clip;

	clip.x0=0;
	clip.y0=0;
	clip.x1=639;
	clip.y1=479;
	line.x0=state_ta.grab[index].showvertices[from].x;
	line.y0=state_ta.grab[index].showvertices[from].y;
	line.x1=state_ta.grab[index].showvertices[to].x;
	line.y1=state_ta.grab[index].showvertices[to].y;
	if (render_clip_line(&line, &clip))
		return;
	dx=line.x1-line.x0;
	dy=line.y1-line.y0;
	plotx=line.x0;
	ploty=line.y0;
	ix = abs(dx);
	iy = abs(dy);
	inc = max(ix,iy);
	x = y = 0;
	dxix = (dx ? dx/ix : 0);
	dyiy = (dy ? dy/iy : 0);

	for (i=0; i <= inc; ++i)
    {
		x += ix;  y += iy;

		if (x > inc)
		{
			x -= inc;
			plotx += dxix;
			bmpaddr = BITMAP_ADDR32(bitmap,ploty,plotx);
			*bmpaddr = MAKE_RGB(0, 0, 255);
		}
		if (y > inc)
		{
			y -= inc;
			ploty += dyiy;
			bmpaddr = BITMAP_ADDR32(bitmap,ploty,plotx);
			*bmpaddr = MAKE_RGB(0, 0, 255);
		}
	}
}
#endif

#if 0
INLINE UINT32 alpha_blend_r16_565(UINT32 d, UINT32 s, UINT8 level)
{
	int alphad = 256 - level;
	return ((((s & 0x001f) * level + (d & 0x001f) * alphad) >> 8)) |
		   ((((s & 0x07e0) * level + (d & 0x07e0) * alphad) >> 8) & 0x07e0) |
		   ((((s & 0xf800) * level + (d & 0xf800) * alphad) >> 8) & 0xf800);
}
#endif

static void testdrawscreen(const running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect)
{
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	int cs,x,y,dx,dy,xi,yi,a,rs,ns;
	float iu,iv,u,v;
	UINT32 addrp;
	UINT32 *bmpaddr;
	int xt,yt,cd;
	UINT32 c;
#if 0
	int stride;
	UINT16 *bmpaddr16;
	UINT32 k;
#endif

	if (state_ta.renderselect < 0)
		return;
	rs=state_ta.renderselect;
	c=pvrta_regs[ISP_BACKGND_T];
	c=memory_read_dword(space,0x05000000+((c&0xfffff8)>>1)+(3+3)*4);
	bitmap_fill(bitmap,cliprect,c);
#if 0
	stride=pvrta_regs[FB_W_LINESTRIDE] << 3;
	c=pvrta_regs[ISP_BACKGND_T];
	a=(c & 0xfffff8) >> 1;
	cs=(c >> 24) & 7;
	cs=cs+3; // cs*2+3
	c=memory_read_dword(space,0x05000000+a+3*4+(cs-1)*4);
	dx=(int)u2f(memory_read_dword(space,0x05000000+a+3*4+cs*4+0));
	dy=(int)u2f(memory_read_dword(space,0x05000000+a+3*4+2*cs*4+4));
	for (y=0;y < dy;y++)
	{
		addrp=state_ta.grab[rs].fbwsof1+y*stride;
		for (x=0;x < dx;x++)
		{
			bmpaddr16=((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1);
			*bmpaddr16=((c & 0xf80000) >> 8) | ((c & 0xfc00) >> 5) | ((c & 0xf8) >> 3);
			addrp+=2;
		}
	}
#endif

	ns=state_ta.grab[rs].testsprites_size;
	for (cs=0;cs < ns;cs++)
	{
		dx=state_ta.grab[rs].showsprites[cs].sizex;
		dy=state_ta.grab[rs].showsprites[cs].sizey;
		iu=state_ta.grab[rs].showsprites[cs].du/dx;
		iv=state_ta.grab[rs].showsprites[cs].dv/dy;
		cd=dilatechose[state_ta.grab[rs].showsprites[cs].texturesizes];

		if ((state_ta.grab[rs].showsprites[cs].positionx+dx) > 640)
			dx=640-state_ta.grab[rs].showsprites[cs].positionx;
		if ((state_ta.grab[rs].showsprites[cs].positiony+dy) > 480)
			dy=480-state_ta.grab[rs].showsprites[cs].positiony;
		xi=0;
		yi=0;

		if (state_ta.grab[rs].showsprites[cs].positionx < 0)
			xi=-state_ta.grab[rs].showsprites[cs].positionx;
		if (state_ta.grab[rs].showsprites[cs].positiony < 0)
			yi=-state_ta.grab[rs].showsprites[cs].positiony;

		for (y = yi;y < dy;y++)
		{
			for (x = xi;x < dx;x++)
			{
				// find the coordinates
				u=state_ta.grab[rs].showsprites[cs].u+iu*x;
				v=state_ta.grab[rs].showsprites[cs].v+iv*y;
				yt=v*(state_ta.grab[rs].showsprites[cs].texturesizey-1);
				xt=u*(state_ta.grab[rs].showsprites[cs].texturesizex-1);

				a=255;
				switch (state_ta.grab[rs].showsprites[cs].texturepf)
				{
				case 0: // 1555
					// find the address
					if (state_ta.grab[rs].showsprites[cs].texturemode == 1)
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+(state_ta.grab[rs].showsprites[cs].texturesizex*yt+xt)*2;
					else if (state_ta.grab[rs].showsprites[cs].texturemode == 0) // twiddled
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt])*2;
					else // vq-compressed
					{
						c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 1]);
						c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(state_ta.grab[rs].showsprites[cs].textureaddress+c));
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 1])*2;
					}
					// read datum
					c=*(((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1));
					// find the color and draw
					a=(((c & 0x8000) >> 8)*255)/0x80;
					c=MAKE_RGB((c&0x7c00) >> 7, (c&0x3e0) >> 2, (c&0x1f) << 3);
					break;
				case 1: // 565
					// find the address
					if (state_ta.grab[rs].showsprites[cs].texturemode == 1)
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+(state_ta.grab[rs].showsprites[cs].texturesizex*yt+xt)*2;
					else if (state_ta.grab[rs].showsprites[cs].texturemode == 0) // twiddled
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt])*2;
					else // vq-compressed
					{
						c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 1]);
						c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(state_ta.grab[rs].showsprites[cs].textureaddress+c));
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 1])*2;
					}
					// read datum
					c=*(((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1));
					// find the color and draw
					a=255;
					c=MAKE_RGB((c&0xf800) >> 8, (c&0x7e0) >> 3, (c&0x1f) << 3);
					break;
				case 2: // 4444
					// find the address
					if (state_ta.grab[rs].showsprites[cs].texturemode == 1)
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+(state_ta.grab[rs].showsprites[cs].texturesizex*yt+xt)*2;
					else if (state_ta.grab[rs].showsprites[cs].texturemode == 0) // twiddled
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt])*2;
					else // vq-compressed
					{
						c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 1]);
						c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(state_ta.grab[rs].showsprites[cs].textureaddress+c));
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 1])*2;
					}
					// read datum
					c=*(((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1));
					// find the color and draw
					a=(((c & 0xf000) >> 8)*255)/0xf0;
					c=MAKE_RGB((c&0xf00) >> 4, c&0xf0, (c&0xf) << 4);
					break;
				case 3: // yuv422
					break;
				case 4: // bumpmap
					break;
				case 5: // 4 bpp palette
					break;
				case 6: // 8 bpp palette
					if (state_ta.grab[rs].showsprites[cs].texturemode & 2) // vq-compressed
					{
						c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 2]);
						c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(state_ta.grab[rs].showsprites[cs].textureaddress+c));
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 3]);
					}
					else
					{
						addrp=state_ta.grab[rs].showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt]);
					}
					c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(addrp));
					c=((state_ta.grab[rs].showsprites[cs].texturepalette & 0x30) << 4) | c;
					c=pvrta_regs[0x1000/4+c];
					switch (pvrta_regs[PAL_RAM_CTRL])
					{
					case 0: // argb1555
						a=(((c & 0x8000) >> 8)*255)/0x80;
						c=MAKE_RGB((c&0x7c00) >> 7, (c&0x3e0) >> 2, (c&0x1f) << 3);
						break;
					case 1: // rgb565
						a=255;
						c=MAKE_RGB((c&0xf800) >> 8, (c&0x7e0) >> 3, (c&0x1f) << 3);
						break;
					case 2: // argb4444
						a=(((c & 0xf000) >> 8)*255)/0xf0;
						c=MAKE_RGB((c&0xf00) >> 4, c&0xf0, (c&0xf) << 4);
						break;
					case 3: // argb8888
						a=(c & 0xff000000) >> 24;
						c=MAKE_RGB((c&0xff0000) >> 16, (c&0xff00) >> 8, c&0xff);
						break;
					}
					break;
				case 7: // reserved
					break;
				} // switch
				bmpaddr=BITMAP_ADDR32(bitmap,state_ta.grab[rs].showsprites[cs].positiony+y,state_ta.grab[rs].showsprites[cs].positionx+x);
				*bmpaddr = alpha_blend_r32(*bmpaddr, c, a);
#if 0
				// write into framebuffer
				switch (pvrta_regs[FB_W_CTRL] & 7)
				{
				case 0: // 0555 KRGB 16 bit
					k=pvrta_regs[FB_W_CTRL] & 0x8000;
					addrp=state_ta.grab[s].fbwsof1+(state_ta.grab[s].showsprites[cs].positiony+y)*stride+(state_ta.grab[s].showsprites[cs].positionx+x)*2;
					bmpaddr16=((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1);
					*bmpaddr16=k | alpha_blend_r16(*bmpaddr16,((c & 0xf80000) >> 9) | ((c & 0xf800) >> 6) | ((c & 0xf8) >> 3),a);
					break;
				case 1: // 565 RGB 16 bit
					addrp=state_ta.grab[rs].fbwsof1+(state_ta.grab[rs].showsprites[cs].positiony+y)*stride+(state_ta.grab[rs].showsprites[cs].positionx+x)*2;
					bmpaddr16=((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1);
					//*bmpaddr16=alpha_blend_r16_565(*bmpaddr16,((c & 0xf80000) >> 8) | ((c & 0xfc00) >> 5) | ((c & 0xf8) >> 3),a);
					*bmpaddr16=((c & 0xf80000) >> 8) | ((c & 0xfc00) >> 5) | ((c & 0xf8) >> 3);
					break;
				case 2: // 4444 ARGB 16 bit
					break;
				case 3: // 1555 ARGB 16 bit
					break;
				case 4: // 888 RGB 24 bit packed
					break;
				case 5: // 0888 KRGB 32 bit
					break;
				case 6: // 8888 ARGB 32 bit
					break;
				case 7: // reserved
					break;
				} // switch
#endif
			}
		}
	}
	state_ta.grab[rs].busy=0;
#if DEBUG_VERTICES
	a = state_ta.grab[rs].testvertices_size;
	if (a > 65530)
		a = 65530;
	for (cs=1;cs < a;cs++)
	{
		testdrawline(bitmap,rs,cs-1,cs);  // draw a segment from vertex xi to vertex yi
		if ((cs > 1) && (state_ta.grab[rs].showvertices[cs-2].endofstrip == 0))
			testdrawline(bitmap,rs,cs-2,cs);
		if (state_ta.grab[rs].showvertices[cs].endofstrip == 1)
			cs++;
	}
#endif
}

static void testdrawscreenframebuffer(bitmap_t *bitmap,const rectangle *cliprect)
{
	int x,y,dy,xi;
	UINT32 addrp;
	UINT16 *fbaddr;
	UINT32 c;

	// only for rgb565 framebuffer
	xi=((pvrta_regs[FB_R_SIZE] & 0x3ff)+1) << 1;
	dy=((pvrta_regs[FB_R_SIZE] >> 10) & 0x3ff)+1;
	for (y=0;y < dy;y++)
	{
		addrp=pvrta_regs[FB_R_SOF1]+y*xi*2;
		for (x=0;x < xi;x++)
		{
			fbaddr=BITMAP_ADDR16(bitmap,y,x);
			c=*(((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1));
			*fbaddr = (UINT16)c;
			addrp+=2;
		}
	}
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
UINT32 a;

	a=dc_sysctrl_regs[SB_ISTNRM] | IST_VBL_OUT;
	dc_sysctrl_regs[SB_ISTNRM] = a; // V Blank-out interrupt
	dc_update_interrupt_status(machine);

	timer_adjust_oneshot(vbout_timer, attotime_never, 0);
}

VIDEO_START(dc)
{
	memset(pvrctrl_regs, 0, sizeof(pvrctrl_regs));
	memset(pvrta_regs, 0, sizeof(pvrta_regs));
	memset(state_ta.grab, 0, sizeof(state_ta.grab));
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
	state_ta.renderselect= -1;
	state_ta.grabsel=0;

	computedilated();

	vbout_timer = timer_alloc(machine, vbout, 0);
	timer_adjust_oneshot(vbout_timer, attotime_never, 0);
}

VIDEO_UPDATE(dc)
{
static int useframebuffer=1;

	if ((useframebuffer) && !state_ta.start_render_received)
	{
		if (pvrta_regs[VO_CONTROL] & (1 << 3))
			bitmap_fill(bitmap,cliprect,pvrta_regs[VO_BORDER_COL] & 0xFFFFFF);
		else
			testdrawscreenframebuffer(bitmap,cliprect);
		return 0;
	}

	if (state_ta.start_render_received)
	{
		useframebuffer=0;
		testdrawscreen(screen->machine,bitmap,cliprect);
		if (pvrta_regs[VO_CONTROL] & (1 << 3))
			bitmap_fill(bitmap,cliprect,pvrta_regs[VO_BORDER_COL] & 0xFFFFFF);
		state_ta.start_render_received=0;
		state_ta.renderselect= -1;
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOR_TSP;	// TSP end of render
		dc_update_interrupt_status(screen->machine);
		return 0;
	}
	else
		return 1;
}

void dc_vblank(running_machine *machine)
{
	dc_sysctrl_regs[SB_ISTNRM] |= IST_VBL_IN; // V Blank-in interrupt
	dc_update_interrupt_status(machine);

	timer_adjust_oneshot(vbout_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);
}

