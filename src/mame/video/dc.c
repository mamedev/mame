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

static UINT32 pvrctrl_regs[0x100/4];
static UINT32 pvrta_regs[0x2000/4];
static const int pvr_parconfseq[] = {1,2,3,2,3,4,5,6,5,6,7,8,9,10,11,12,13,14,13,14,15,16,17,16,17,0,0,0,0,0,18,19,20,19,20,21,22,23,22,23};
static const int pvr_wordsvertex[24]  = {8,8,8,8,8,16,16,8,8,8, 8, 8,8,8,8,8,16,16, 8,16,16,8,16,16};
static const int pvr_wordspolygon[24] = {8,8,8,8,8, 8, 8,8,8,8,16,16,8,8,8,8, 8, 8,16,16,16,8, 8, 8};
static int pvr_parameterconfig[64];

UINT64 *dc_texture_ram;
static UINT32 tafifo_buff[32];
static int tafifo_pos, tafifo_mask, tafifo_vertexwords, tafifo_listtype;
static int start_render_received;
static int alloc_ctrl_OPB_Mode, alloc_ctrl_PT_OPB, alloc_ctrl_TM_OPB, alloc_ctrl_T_OPB, alloc_ctrl_ZM_OPB, alloc_ctrl_O_OPB;

static struct testsprites
{
	int positionx, positiony;
	int sizex, sizey;
	UINT32 textureaddress;
	float u, v, du, dv;
	int texturemode;
	int texturesizex, texturesizey, texturesizes, texturepf;
} showsprites[2048];

static int testsprites_size, toerasesprites;
static UINT32 dilated0[11][1024];
static UINT32 dilated1[11][1024];
static int dilatechose[64];

// register decode helper
INLINE int decode_reg_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0x00000000ffffffff)) && (mem_mask != U64(0xffffffff00000000)))
	{
		assert_always(0, "Wrong mask!\n");
	}

	if (mem_mask == U64(0x00000000ffffffff))
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
		pvrta_regs[reg] = (video_screen_get_vblank(0) << 13) | (video_screen_get_hblank(0) << 12) | (video_screen_get_vpos(0) & 0x3ff);
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
		}
		if (dat & 2)
		{
			#if DEBUG_PVRTA
			mame_printf_verbose("pvr_ta_w:  Core Pipeline soft reset\n");
			#endif
			if (start_render_received == 1)
				start_render_received = 0;
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
		start_render_received=1;
		break;
	case TA_ALLOC_CTRL:
		alloc_ctrl_OPB_Mode = dat & 0x100000; // 0 up 1 down
		alloc_ctrl_PT_OPB = (4 << ((dat >> 16) & 3)) & 0x38; // number of 32 bit words (0,8,16,32)
		alloc_ctrl_TM_OPB = (4 << ((dat >> 12) & 3)) & 0x38;
		alloc_ctrl_T_OPB = (4 << ((dat >> 8) & 3)) & 0x38;
		alloc_ctrl_ZM_OPB = (4 << ((dat >> 4) & 3)) & 0x38;
		alloc_ctrl_O_OPB = (4 << ((dat >> 0) & 3)) & 0x38;
		break;
	case TA_LIST_INIT:
		tafifo_pos=0;
		tafifo_mask=7;
		tafifo_vertexwords=8;
		tafifo_listtype= -1;
		toerasesprites=1;
	#if DEBUG_PVRTA
		mame_printf_verbose("TA_OL_BASE       %08x TA_OL_LIMIT  %08x\n", pvrta_regs[TA_OL_BASE], pvrta_regs[TA_OL_LIMIT]);
		mame_printf_verbose("TA_ISP_BASE      %08x TA_ISP_LIMIT %08x\n", pvrta_regs[TA_ISP_BASE], pvrta_regs[TA_ISP_LIMIT]);
		mame_printf_verbose("TA_ALLOC_CTRL    %08x\n", pvrta_regs[TA_ALLOC_CTRL]);
		mame_printf_verbose("TA_NEXT_OPB_INIT %08x\n", pvrta_regs[TA_NEXT_OPB_INIT]);
	#endif
		pvrta_regs[TA_NEXT_OPB] = pvrta_regs[TA_NEXT_OPB_INIT];
		pvrta_regs[TA_ITP_CURRENT] = pvrta_regs[TA_ISP_BASE];
		break;
	}

	#if DEBUG_PVRTA_REGS
	mame_printf_verbose("PVRTA: [%08x=%x] write %llx to %x (reg %x %x), mask %llx\n", 0x5f8000+reg*4, dat, data>>shift, offset, reg, (reg*4)+0x8000, mem_mask);
	#endif
}

WRITE64_HANDLER( ta_fifo_poly_w )
{
	UINT32 a;
	static UINT32 paracontrol,paratype,endofstrip,listtype,global_paratype,parameterconfig;
	static UINT32 groupcontrol,groupen,striplen,userclip;
	static UINT32 objcontrol,shadow,volume,coltype,texture,offfset,gouraud,uv16bit;
	static UINT32 textureusize,texturevsize,texturesizes,textureaddress,scanorder,pixelformat;
	static UINT32 srcalphainstr,dstalphainstr,srcselect,dstselect,fogcontrol,colorclamp,usealpha;
	static UINT32 ignoretexalpha,flipuv,clampuv,filtermode,sstexture,mmdadjust,tsinstruction;
	static UINT32 depthcomparemode,cullingmode,zwritedisable,cachebypass,dcalcctrl,volumeinstruction,mipmapped,vqcompressed,strideselect;

	if (!mem_mask) 	// 64 bit
	{
		tafifo_buff[tafifo_pos]=(UINT32)data;
		tafifo_buff[tafifo_pos+1]=(UINT32)(data >> 32);
		#if DEBUG_FIFO_POLY
		mame_printf_debug("ta_fifo_poly_w:  Unmapped write64 %08x = %llx -> %08x %08x\n", 0x10000000+offset*8, data, tafifo_buff[tafifo_pos], tafifo_buff[tafifo_pos+1]);
		#endif
		tafifo_pos += 2;
	}
	else
	{
		mame_printf_debug("ta_fifo_poly_w:  Only 64 bit writes supported!\n");
	}

	tafifo_pos &= tafifo_mask;
	if (tafifo_pos == 0)
	{
		paracontrol=(tafifo_buff[0] >> 24) & 0xff;
		// 0 end of list
		// 1 user tile clip
		// 2 object list set
		// 3 reserved
		// 4 polygon/modifier volume
		// 5 sprite
		// 6 reserved
		// 7 vertex
		paratype=(paracontrol >> 5) & 7;
		endofstrip=(paracontrol >> 4) & 1;
		listtype=(paracontrol >> 0) & 7;
		if ((paratype >= 4) && (paratype <= 6))
		{
			global_paratype = paratype;
			groupcontrol=(tafifo_buff[0] >> 16) & 0xff;
			groupen=(groupcontrol >> 7) & 1;
			striplen=(groupcontrol >> 2) & 3;
			userclip=(groupcontrol >> 0) & 3;
			objcontrol=(tafifo_buff[0] >> 0) & 0xffff;
			shadow=(objcontrol >> 7) & 1;
			volume=(objcontrol >> 6) & 1;
			coltype=(objcontrol >> 4) & 3;
			texture=(objcontrol >> 3) & 1;
			offfset=(objcontrol >> 2) & 1;
			gouraud=(objcontrol >> 1) & 1;
			uv16bit=(objcontrol >> 0) & 1;
		}

		if (toerasesprites == 1)
		{
			toerasesprites=0;
			testsprites_size=0;
		}

		// check if we need 8 words more
		if (tafifo_mask == 7)
		{
			parameterconfig = pvr_parameterconfig[objcontrol & 0x3d];
			// decide number of words per vertex
			if (paratype == 7)
			{
				if ((global_paratype == 5) || (tafifo_listtype == 1) || (tafifo_listtype == 3))
					tafifo_vertexwords = 16;
				if (tafifo_vertexwords == 16)
				{
					tafifo_mask = 15;
					tafifo_pos = 8;
					return;
				}
			}
			tafifo_vertexwords=pvr_wordsvertex[parameterconfig];
			if ((paratype == 4) && ((listtype != 1) && (listtype != 3)))
				if (pvr_wordspolygon[parameterconfig] == 16)
				{
					tafifo_mask = 15;
					tafifo_pos = 8;
					return;
				}
		}
		else
		{
			tafifo_mask = 7;
		}

		// now we heve all the needed words
		// interpret their meaning
		if (paratype == 0)
		{ // end of list
			a=0; // 6-10 0-3
			switch (tafifo_listtype)
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
			sysctrl_regs[SB_ISTNRM] |= a;
			update_interrupt_status();
			tafifo_listtype= -1; // no list being received
		}
		else if (paratype == 1)
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type 1 User Tile Clip\n");
			mame_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
			#endif
		}
		else if (paratype == 2)
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type 2 Object List Set at %08x\n", tafifo_buff[1]);
			mame_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
			#endif
		}
		else if (paratype == 3)
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type %x Unknown!\n", tafifo_buff[0]);
			#endif
		}
		else
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type %d End of Strip %d List Type %d\n", paratype, endofstrip, listtype);
			#endif

			// set type of list currently being recieved
			if ((paratype == 4) || (paratype == 5) || (paratype == 6))
			{
				if (tafifo_listtype < 0)
				{
					tafifo_listtype = listtype;
				}
			}

			if ((paratype == 4) || (paratype == 5))
			{ // quad or polygon
				depthcomparemode=(tafifo_buff[1] >> 29) & 7;
				cullingmode=(tafifo_buff[1] >> 27) & 3;
				zwritedisable=(tafifo_buff[1] >> 26) & 1;
				cachebypass=(tafifo_buff[1] >> 21) & 1;
				dcalcctrl=(tafifo_buff[1] >> 20) & 1;
				volumeinstruction=(tafifo_buff[1] >> 29) & 7;
				textureusize=1 << (3+((tafifo_buff[2] >> 3) & 7));
				texturevsize=1 << (3+(tafifo_buff[2] & 7));
				texturesizes=tafifo_buff[2] & 0x3f;
				srcalphainstr=(tafifo_buff[2] >> 29) & 7;
				dstalphainstr=(tafifo_buff[2] >> 26) & 7;
				srcselect=(tafifo_buff[2] >> 25) & 1;
				dstselect=(tafifo_buff[2] >> 24) & 1;
				fogcontrol=(tafifo_buff[2] >> 22) & 3;
				colorclamp=(tafifo_buff[2] >> 21) & 1;
				usealpha=(tafifo_buff[2] >> 20) & 1;
				ignoretexalpha=(tafifo_buff[2] >> 19) & 1;
				flipuv=(tafifo_buff[2] >> 17) & 3;
				clampuv=(tafifo_buff[2] >> 15) & 3;
				filtermode=(tafifo_buff[2] >> 13) & 3;
				sstexture=(tafifo_buff[2] >> 12) & 1;
				mmdadjust=(tafifo_buff[2] >> 8) & 1;
				tsinstruction=(tafifo_buff[2] >> 6) & 3;
				if (texture == 1)
				{
					textureaddress=(tafifo_buff[3] & 0x1FFFFF) << 3;
					scanorder=(tafifo_buff[3] >> 26) & 1;
					pixelformat=(tafifo_buff[3] >> 27) & 7;
					mipmapped=(tafifo_buff[3] >> 31) & 1;
					vqcompressed=(tafifo_buff[3] >> 30) & 1;
					strideselect=(tafifo_buff[3] >> 25) & 1;
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Texture %d x %d at %08x format %d\n", textureusize, texturevsize, (tafifo_buff[3] & 0x1FFFFF) << 3, pixelformat);
					#endif
				}
				if (paratype == 4)
				{
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Sprite\n");
					#endif
				}
				if (paratype == 5)
				{
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Polygon\n");
					#endif
				}
			}

			if (paratype == 7)
			{ // vertex
				if (global_paratype == 5)
				{
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Vertex sprite");
					for (a=1; a <= 11; a++)
					{
						mame_printf_verbose(" %f", u2f(tafifo_buff[a]));
					}
					mame_printf_verbose("\n");
					#endif
					if (texture == 1)
					{
						#if DEBUG_PVRDLIST
						mame_printf_verbose(" %f %f %f %f %f %f\n",u2f(tafifo_buff[13] & 0xffff0000),u2f((tafifo_buff[13] & 0xffff) << 16),u2f(tafifo_buff[14] & 0xffff0000),u2f((tafifo_buff[14] & 0xffff) << 16),u2f(tafifo_buff[15] & 0xffff0000),u2f((tafifo_buff[15] & 0xffff) << 16));
						#endif
/* test video start */
						showsprites[testsprites_size].positionx=u2f(tafifo_buff[1]);
						showsprites[testsprites_size].positiony=u2f(tafifo_buff[2]);
						showsprites[testsprites_size].sizex=u2f(tafifo_buff[4])-u2f(tafifo_buff[1]);
						showsprites[testsprites_size].sizey=u2f(tafifo_buff[8])-u2f(tafifo_buff[2]);
						showsprites[testsprites_size].u=u2f(tafifo_buff[13] & 0xffff0000);
						showsprites[testsprites_size].v=u2f((tafifo_buff[13] & 0xffff) << 16);
						showsprites[testsprites_size].du=u2f(tafifo_buff[14] & 0xffff0000)-showsprites[testsprites_size].u;
						showsprites[testsprites_size].dv=u2f((tafifo_buff[15] & 0xffff) << 16)-showsprites[testsprites_size].v;
						showsprites[testsprites_size].textureaddress=textureaddress;
						showsprites[testsprites_size].texturesizex=textureusize;
						showsprites[testsprites_size].texturesizey=texturevsize;
						showsprites[testsprites_size].texturemode=scanorder+vqcompressed*2;
						showsprites[testsprites_size].texturesizes=texturesizes;
						showsprites[testsprites_size].texturepf=pixelformat;
						testsprites_size=testsprites_size+1;
/* test video end */
					}
				}
				if ((tafifo_listtype == 1) || (tafifo_listtype == 3))
				{
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Vertex modifier volume");
					for (a=1; a <= 11; a++)
					{
						mame_printf_verbose(" %f", u2f(tafifo_buff[a]));
					}
					mame_printf_verbose("\n");
					#endif
				}
				if (global_paratype == 4)
				{
					#if DEBUG_PVRDLIST
					mame_printf_verbose(" Vertex polygon");
					mame_printf_verbose("\n");
					#endif
				}
			}
		}
	} // if (tafifo_pos == 0)
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

	for (b=0;b <= 10;b++)
		for (a=0;a < 1024;a++) {
			dilated0[b][a]=dilate0(a,b);
			dilated1[b][a]=dilate1(a,b);
		}
	for (b=0;b <= 7;b++)
		for (a=0;a < 7;a++)
			dilatechose[(b << 3) + a]=3+(a < b ? a : b);
}

static void testdrawscreen(bitmap_t *bitmap,const rectangle *cliprect)
{
	int cs,x,y,dx,dy,xi,yi,a;
	float iu,iv,u,v;
	UINT32 addrp;
	UINT32 *bmpaddr;
	int c,xt,yt,cd;

	fillbitmap(bitmap,program_read_dword_64le(0x05000000+6*4),cliprect);
	for (cs=0;cs < testsprites_size;cs++)
	{
		dx=showsprites[cs].sizex;
		dy=showsprites[cs].sizey;
		iu=showsprites[cs].du/dx;
		iv=showsprites[cs].dv/dy;
		cd=dilatechose[showsprites[cs].texturesizes];

		if ((showsprites[cs].positionx+dx) > 640)
			dx=640-showsprites[cs].positionx;
		if ((showsprites[cs].positiony+dy) > 480)
			dy=480-showsprites[cs].positiony;
		xi=0;
		yi=0;

		if (showsprites[cs].positionx < 0)
			xi=-showsprites[cs].positionx;
		if (showsprites[cs].positiony < 0)
			yi=-showsprites[cs].positiony;

		for (y = yi;y < dy;y++)
		{
			for (x = xi;x < dx;x++)
			{
				u=showsprites[cs].u+iu*x;
				v=showsprites[cs].v+iv*y;
				yt=v*(showsprites[cs].texturesizey-1);
				xt=u*(showsprites[cs].texturesizex-1);

				if (showsprites[cs].texturemode == 1)
					addrp=showsprites[cs].textureaddress+(showsprites[cs].texturesizex*yt+xt)*2;
				else if (showsprites[cs].texturemode == 0)
					addrp=showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt])*2;
				else
				{
					c=0x800+(dilated1[cd][xt >> 1] + dilated0[cd][yt >> 1]);
					c=*(((UINT8 *)dc_texture_ram) + BYTE_XOR_LE(showsprites[cs].textureaddress+c));
					addrp=showsprites[cs].textureaddress+c*8+(dilated1[cd][xt & 1] + dilated0[cd][yt & 1])*2;
				}

				c=*(((UINT16 *)dc_texture_ram) + (WORD2_XOR_LE(addrp) >> 1));
				if (showsprites[cs].texturepf == 2)
				{
					a=(c & 0xf000) >> 8;
					bmpaddr=BITMAP_ADDR32(bitmap,showsprites[cs].positiony+y,showsprites[cs].positionx+x);
					*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0xf00) >> 4, c&0xf0, (c&0xf) << 4), a);
				}
				else
				{
					a=(c & 0x7000) >> 7;
					bmpaddr=BITMAP_ADDR32(bitmap,showsprites[cs].positiony+y,showsprites[cs].positionx+x);
					*bmpaddr = alpha_blend_r32(*bmpaddr, MAKE_RGB((c&0x7c00) >> 7, (c&0x3e0) >> 2, (c&0x1f) << 3), a);
				}
			}
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
	tafifo_pos=0;
	tafifo_mask=7;
	tafifo_vertexwords=8;
	tafifo_listtype= -1;
	start_render_received=0;

	testsprites_size=0;
	toerasesprites=0;
	computedilated();
}

VIDEO_UPDATE(dc)
{
	int a;

	if (pvrta_regs[VO_CONTROL] & (1 << 3))
	{
		fillbitmap(bitmap,pvrta_regs[VO_BORDER_COL] & 0xFFFFFF,cliprect);
		return 0;
	}

	testdrawscreen(bitmap,cliprect);

	if (start_render_received)
	{
		start_render_received=0;
		a=4; // tsp end
		sysctrl_regs[SB_ISTNRM] |= a;
		update_interrupt_status();
	}
	return 0;
}

void dc_vblank(void)
{
	sysctrl_regs[SB_ISTNRM] |= 0x08; // V Blank-in interrupt
	update_interrupt_status();
}

