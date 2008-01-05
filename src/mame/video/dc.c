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

UINT64 *dc_texture_ram;
static UINT32 tafifo_buff[32];
static int tafifo_pos, tafifo_mask, tafifo_vertexwords, tafifo_listtype;
static int start_render_received;

struct testsprites 
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
	mame_printf_verbose("PVRCTRL: read %x @ %x (reg %x), mask %llx (PC=%x)\n", pvrctrl_regs[reg], offset, reg, mem_mask, activecpu_get_pc());
	#endif

	return (UINT64)pvrctrl_regs[reg] << shift;
}

WRITE64_HANDLER( pvr_ctrl_w )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: write %llx to %x (reg %x), mask %llx\n", data>>shift, offset, reg, mem_mask);
	#endif

	pvrctrl_regs[reg] |= data >> shift;
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

	#if DEBUG_PVRTA
	mame_printf_verbose("PVRTA: read %x @ %x (reg %x), mask %llx (PC=%x)\n", pvrta_regs[reg], offset, reg, mem_mask, activecpu_get_pc());
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
		#if DEBUG_PVRTA_REGS
		if (dat & 1)
		{
			mame_printf_verbose("pvr_ta_w:  TA soft reset\n");
		}
		if (dat & 2)
		{
			mame_printf_verbose("pvr_ta_w:  Core Pipeline soft reset\n");
		}
		if (dat & 4)
		{
			mame_printf_verbose("pvr_ta_w:  sdram I/F soft reset\n");
		}
		#endif
		break;
	case STARTRENDER:
		start_render_received=1;
		break;
	case TA_LIST_INIT:
		tafifo_pos=0;
		tafifo_mask=7;
		tafifo_vertexwords=8;
		tafifo_listtype= -1;
		toerasesprites=1;
		break;
	}

	#if DEBUG_PVRTA
	mame_printf_verbose("PVRTA: write %llx to %x (reg %x %x), mask %llx\n", data>>shift, offset, reg, (reg*4)+0x8000, mem_mask);
	#endif
}

WRITE64_HANDLER( ta_fifo_poly_w )
{
	UINT32 a;
	static UINT32 paracontrol,paratype,endofstrip,listtype;
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
		logerror("ta_fifo_poly_w:  Only 64 bit writes supported!\n");
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

		if (toerasesprites == 1) 
		{
			toerasesprites=0;
			testsprites_size=0;
		}

		// check if we need 8 words more 
		if (tafifo_mask == 7) 
		{ 
			if ((paratype == 4) && (((coltype >= 2) && (offfset == 1)) || ((coltype >= 2) && (volume == 1)))) 
			{
				tafifo_mask = 15;
				tafifo_pos = 8;
				return;
			}
			
			if ((paratype == 7) && (tafifo_vertexwords == 16)) 
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
		if (tafifo_buff[0] == 0) 
		{
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
				break;
			}
			sysctrl_regs[SB_ISTNRM] |= a;
			update_interrupt_status();
			tafifo_listtype= -1;
		} 
		else 
		{
			#if DEBUG_PVRDLIST
			mame_printf_verbose("Para Type %d End of Strip %d List Type %d\n", paratype, endofstrip, listtype);
			#endif

			if (((paratype == 4) && (texture == 1)) || (paratype == 5)) 
			{
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

			if (paratype == 7) 
			{ // vertex
				#if DEBUG_PVRDLIST
				mame_printf_verbose(" test vertex ");
				for (a=1; a <= 11; a++)
				{
					mame_printf_verbose(" %e", u2f(tafifo_buff[a]));
				}
				mame_printf_verbose("\n");
				mame_printf_verbose(" %e %e %e %e %e %e\n",u2f(tafifo_buff[13] & 0xffff0000),u2f((tafifo_buff[13] & 0xffff) << 16),u2f(tafifo_buff[14] & 0xffff0000),u2f((tafifo_buff[14] & 0xffff) << 16),u2f(tafifo_buff[15] & 0xffff0000),u2f((tafifo_buff[15] & 0xffff) << 16));
				#endif
/* test video start */
				// pixely=u2f((tafifo_buff[13] & 0xffff) << 16)*1024
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
				showsprites[testsprites_size].texturemode=scanorder;
				showsprites[testsprites_size].texturesizes=texturesizes;
				showsprites[testsprites_size].texturepf=pixelformat;
				testsprites_size=testsprites_size+1;
/* test video end */
			}

			if (paratype != 7)
			{
				tafifo_vertexwords=8;
			}
			if (((paratype == 4) && ((texture == 1) || (coltype == 1))) || (paratype == 5))
			{
				tafifo_vertexwords=16;
			}
			if ((paratype == 4) || (paratype == 5) || (paratype == 6))
			{
				if (tafifo_listtype < 0)
				{
					tafifo_listtype = listtype;
				}
			}
		}
	} // if (tafifo_pos == 0)
}

WRITE64_HANDLER( ta_fifo_yuv_w )
{
	mame_printf_verbose("YUV FIFO: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

/* test video start */
UINT32 dilate0(UINT32 value,int bits) // dilate first "bits" bits in "value"
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

UINT32 dilate1(UINT32 value,int bits) // dilate first "bits" bits in "value"
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

void computedilated(void)
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

void testdrawscreen(bitmap_t *bitmap,const rectangle *cliprect)
{
	int cs,x,y,dx,dy,xi,yi,a;
	float iu,iv,u,v;
	UINT32 addrp;
	UINT32 *bmpaddr;
	int c,xt,yt,cd;

	fillbitmap(bitmap,MAKE_RGB(128,128,128),cliprect);
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
				else
					addrp=showsprites[cs].textureaddress+(dilated1[cd][xt] + dilated0[cd][yt])*2;

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

VIDEO_START(dc)
{
	memset(pvrctrl_regs, 0, sizeof(pvrctrl_regs));
	memset(pvrta_regs, 0, sizeof(pvrta_regs));

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

