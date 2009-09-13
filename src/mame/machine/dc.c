/*

    dc.c - Sega Dreamcast hardware

*/

#include "driver.h"
#include "debugger.h"
#include "dc.h"
#include "cpu/sh4/sh4.h"
#include "sound/aica.h"
#include "naomibd.h"
#include "naomi.h"

#define DEBUG_REGISTERS	(1)

#if DEBUG_REGISTERS

#define DEBUG_SYSCTRL	(0)
#define DEBUG_MAPLE	(0)
#define DEBUG_MAPLE_REGS	(0)
#define DEBUG_AICA_DMA (0)

#define ENABLE_MAPLE_IRQ (0)

#if DEBUG_SYSCTRL
static const char *const sysctrl_names[] =
{
	"CH2 DMA dest",
	"CH2 DMA length",
	"CH2 DMA start",
	"5f680c",
	"Sort DMA start link table addr",
	"Sort DMA link base addr",
	"Sort DMA link address bit width",
	"Sort DMA link address shift control",
	"Sort DMA start",
	"5f6824", "5f6828", "5f682c", "5f6830",
	"5f6834", "5f6838", "5f683c",
	"DBREQ # signal mask control",
	"BAVL # signal wait count",
	"DMA priority count",
	"CH2 DMA max burst length",
	"5f6850", "5f6854", "5f6858", "5f685c",
	"5f6860", "5f6864", "5f6868", "5f686c",
	"5f6870", "5f6874", "5f6878", "5f687c",
	"TA FIFO remaining",
	"TA texture memory bus select 0",
	"TA texture memory bus select 1",
	"FIFO status",
	"System reset", "5f6894", "5f6898",
	"System bus version",
	"SH4 root bus split enable",
	"5f68a4", "5f68a8", "5f68ac",
	"5f68b0", "5f68b4", "5f68b8", "5f68bc",
	"5f68c0", "5f68c4", "5f68c8", "5f68cc",
	"5f68d0", "5f68d4", "5f68d8", "5f68dc",
	"5f68e0", "5f68e4", "5f68e8", "5f68ec",
	"5f68f0", "5f68f4", "5f68f8", "5f68fc",
	"Normal IRQ status",
	"External IRQ status",
	"Error IRQ status", "5f690c",
	"Level 2 normal IRQ mask",
	"Level 2 external IRQ mask",
	"Level 2 error IRQ mask", "5f691c",
	"Level 4 normal IRQ mask",
	"Level 4 external IRQ mask",
	"Level 4 error IRQ mask", "5f692c",
	"Level 6 normal IRQ mask",
	"Level 6 external IRQ mask",
	"Level 6 error IRQ mask", "5f693c",
	"Normal IRQ PVR-DMA startup mask",
	"External IRQ PVR-DMA startup mask",
	"5f6948", "5f694c",
	"Normal IRQ G2-DMA startup mask",
	"External IRQ G2-DMA startup mask"
};
#endif

#if DEBUG_MAPLE
static const char *const maple_names[] =
{
	"5f6c00",
	"DMA command table addr",
	"5f6c08", "5f6c0c",
	"DMA trigger select",
	"DMA enable",
	"DMA start", "5f6c1c",
	"5f6c20", "5f6c24", "5f6c28", "5f6c2c",
	"5f6c30", "5f6c34", "5f6c38", "5f6c3c",
	"5f6c40", "5f6c44", "5f6c48", "5f6c4c",
	"5f6c50", "5f6c54", "5f6c58", "5f6c5c",
	"5f6c60", "5f6c64", "5f6c68", "5f6c6c",
	"5f6c70", "5f6c74", "5f6c78", "5f6c7c",
	"System control",
	"Status",
	"DMA hard trigger clear",
	"DMA address range",
	"5f6c90", "5f6c94", "5f6c98", "5f6c9c",
	"5f6ca0", "5f6ca4", "5f6ca8", "5f6cac",
	"5f6cb0", "5f6cb4", "5f6cb8", "5f6cbc",
	"5f6cc0", "5f6cc4", "5f6cc8", "5f6ccc",
	"5f6cd0", "5f6cd4", "5f6cd8", "5f6cdc",
	"5f6ce0", "5f6ce4",
	"MSB select", "5f6cec", "5f6cf0",
	"Txd address counter",
	"Rxd address counter",
	"Rxd base address"

};
#endif

#endif

static const UINT8 dc_controller_id[0x70] =
{
	0x00, 0x00, 0x00, 0x01, 0x00, 0x0f, 0x06, 0xfe, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
	0xff, 0x00, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x63, 0x61, 0x73, 0x74, 0x20,	0x43, 0x6f, 0x6e, 0x74,
	0x72, 0x6f, 0x6c, 0x6c, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,	0x20, 0x20, 0x20, 0x20,
	0x50, 0x72, 0x6f, 0x64, 0x75, 0x63, 0x65, 0x64, 0x20, 0x42, 0x79, 0x20,	0x6f, 0x72, 0x20, 0x55,
	0x6e, 0x64, 0x65, 0x72, 0x20, 0x4c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65,	0x20, 0x46, 0x72, 0x6f,
	0x6d, 0x20, 0x53, 0x45, 0x47, 0x41, 0x20, 0x45, 0x4e, 0x54, 0x45, 0x52,	0x50, 0x52, 0x49, 0x53,
	0x45, 0x53, 0x2c, 0x4c, 0x54, 0x44, 0x2e, 0x20, 0x20, 0x20, 0x20, 0x20,	0xae, 0x01, 0xf4, 0x01
};

static const UINT8 dc_controller_version[0x50] =
{
	0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x31, 0x2e, 0x30, 0x31, 0x30, 0x2c, 0x31, 0x39,
	0x39, 0x38, 0x2f, 0x30, 0x39, 0x2f, 0x32, 0x38, 0x2c, 0x33, 0x31, 0x35, 0x2d, 0x36, 0x32, 0x31,
	0x31, 0x2d, 0x41, 0x42, 0x20, 0x20, 0x20, 0x2c, 0x41, 0x6e, 0x61, 0x6c,	0x6f, 0x67, 0x20, 0x4d,
	0x6f, 0x64, 0x75, 0x6c, 0x65, 0x20, 0x3a, 0x20, 0x54, 0x68, 0x65, 0x20,	0x34, 0x74, 0x68, 0x20,
	0x45, 0x64, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x2e, 0x35, 0x2f, 0x38, 0x20,	0x20, 0x2b, 0x44, 0x46
};

// selected Maple registers
enum
{
	MAPLE_DMACMD = 1,
	MAPLE_DMATRIGGERSEL = 4,
	MAPLE_DMAENABLE = 5,
	MAPLE_DMASTART = 6,
	MAPLE_SYSCTRL = 0x20,
	MAPLE_STATUS = 0x21,
};

UINT32 dc_sysctrl_regs[0x200/4];
UINT32 dc_coin_counts[2];
static UINT32 maple_regs[0x100/4];
static UINT32 dc_rtcregister[4];
UINT32 g1bus_regs[0x100/4];
static UINT32 g2bus_regs[0x100/4];
UINT8 maple0x86data1[0x80];
static UINT8 maple0x86data2[0x400];
static emu_timer *dc_rtc_timer;
extern int jvsboard_type;

static const UINT32 maple0x82answer[]=
{
	0x07200083,0x2d353133,0x39343136,0x20202020,0x59504f43,0x48474952,0x45532054,0x45204147,
	0x05200083,0x5245544e,0x53495250,0x43205345,0x544c2c4f,0x20202e44,0x38393931,0x5c525043
};

// register decode helpers

// this accepts only 32-bit accesses
INLINE int decode_reg32_64(running_machine *machine, UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		mame_printf_verbose("%s:Wrong mask!\n", cpuexec_describe_context(machine));
//      debugger_break(machine);
	}

	if (mem_mask == U64(0xffffffff00000000))
	{
		reg++;
		*shift = 32;
 	}

	return reg;
}

// this accepts only 32 and 16 bit accesses
INLINE int decode_reg3216_64(running_machine *machine, UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 16&32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0x0000ffff00000000)) && (mem_mask != U64(0x000000000000ffff)) &&
	    (mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		mame_printf_verbose("%s:Wrong mask!\n", cpuexec_describe_context(machine));
//      debugger_break(machine);
	}

	if (mem_mask & U64(0x0000ffff00000000))
	{
		reg++;
		*shift = 32;
 	}

	return reg;
}

int dc_compute_interrupt_level(running_machine *machine)
{
	UINT32 ln,lx,le;

	ln=dc_sysctrl_regs[SB_ISTNRM] & dc_sysctrl_regs[SB_IML6NRM];
	lx=dc_sysctrl_regs[SB_ISTEXT] & dc_sysctrl_regs[SB_IML6EXT];
	le=dc_sysctrl_regs[SB_ISTERR] & dc_sysctrl_regs[SB_IML6ERR];
	if (ln | lx | le)
	{
		return 6;
	}

	ln=dc_sysctrl_regs[SB_ISTNRM] & dc_sysctrl_regs[SB_IML4NRM];
	lx=dc_sysctrl_regs[SB_ISTEXT] & dc_sysctrl_regs[SB_IML4EXT];
	le=dc_sysctrl_regs[SB_ISTERR] & dc_sysctrl_regs[SB_IML4ERR];
	if (ln | lx | le)
	{
		return 4;
	}

	ln=dc_sysctrl_regs[SB_ISTNRM] & dc_sysctrl_regs[SB_IML2NRM];
	lx=dc_sysctrl_regs[SB_ISTEXT] & dc_sysctrl_regs[SB_IML2EXT];
	le=dc_sysctrl_regs[SB_ISTERR] & dc_sysctrl_regs[SB_IML2ERR];
	if (ln | lx | le)
	{
		return 2;
	}

	return 0;
}

void dc_update_interrupt_status(running_machine *machine)
{
int level;

	if (dc_sysctrl_regs[SB_ISTERR])
	{
		dc_sysctrl_regs[SB_ISTNRM] |= IST_ERROR;
	}
	else
	{
		dc_sysctrl_regs[SB_ISTNRM] &= ~IST_ERROR;
	}

	if (dc_sysctrl_regs[SB_ISTEXT])
	{
		dc_sysctrl_regs[SB_ISTNRM] |= IST_G1G2EXTSTAT;
	}
	else
	{
		dc_sysctrl_regs[SB_ISTNRM] &= ~IST_G1G2EXTSTAT;
	}

	level=dc_compute_interrupt_level(machine);
	sh4_set_irln_input(cputag_get_cpu(machine, "maincpu"), 15-level);
}

/******************************************************
JVS init packets
The function packets goes like this:
[0] Function number
[1] Number of "devices"
[2] Parameter of the device
[3] End flag (always zero)
******************************************************/
static int jvs_mux_data;

static int jvsboard_init(int pos)
{
	// four bytes for every available function
	//1 digital inputs
	maple0x86data2[pos+10]=1;
	if(jvsboard_type == JVSBD_MAHJONG)
	{
		maple0x86data2[pos+11]=1; // number of players
		maple0x86data2[pos+12]=22; // switches per player (27 = mahjong)
		jvs_mux_data = 1;
	}
	else //if JVSBD_DEFAULT
	{
		maple0x86data2[pos+11]=2; // number of players
		maple0x86data2[pos+12]=9+4; // switches per player (27 = mahjong)
	}
	maple0x86data2[pos+13]=0;
	//2 coin slots
	maple0x86data2[pos+14]=2;
	maple0x86data2[pos+15]=2; // number of coin slots
	maple0x86data2[pos+16]=0;
	maple0x86data2[pos+17]=0;
	//3 ad stick
	maple0x86data2[pos+18]=3;
	maple0x86data2[pos+19]=2; // analog channels
	maple0x86data2[pos+20]=8; // bits per channel
	maple0x86data2[pos+21]=0;
	//4 rotary
	maple0x86data2[pos+22]=0;
	//5 keyboard
	//6 (touch?) screen
	//7 card reader
	//8 hopper out
	//9 driver out
	//10 analog out
	//11 character
	//12 backup

	maple0x86data2[pos+7]=13+2;
	return 13;
}

static int jvsboard_indirect_read(running_machine *machine, int pos)
{
	// report1,jvsbytes repeated for each function
	//1 digital inputs
	maple0x86data2[pos+ 9]=1; // report
	maple0x86data2[pos+10]=0; // bits TEST TILT1 TILT2 TILT3 ? ? ? ?
	if(jvsboard_type == JVSBD_MAHJONG)
	{
		maple0x86data2[pos+11]=0;
		switch(jvs_mux_data)
		{
			case 0x01: maple0x86data2[pos+12]=input_port_read(machine, "IN1"); break;
			case 0x02: maple0x86data2[pos+12]=input_port_read(machine, "IN2"); break;
			case 0x04: maple0x86data2[pos+12]=input_port_read(machine, "IN3"); break;
			case 0x08: maple0x86data2[pos+12]=input_port_read(machine, "IN4"); break;
			case 0x10: maple0x86data2[pos+12]=0; break;
		}
		maple0x86data2[pos+13]=0;
		maple0x86data2[pos+14]=0;

		jvs_mux_data<<=1;
		if(jvs_mux_data >= 0x20) { jvs_mux_data = 1; }
	}
	else
	{
		maple0x86data2[pos+11]=input_port_read(machine, "IN1"); // bits 1Pstart 1Pservice 1Pup 1Pdown 1Pleft 1Pright 1Ppush1 1Ppush2
		maple0x86data2[pos+12]=input_port_read(machine, "IN2"); // bits 1Ppush3 1Ppush4 1Ppush5 1Ppush6 1Ppush7 1Ppush8 ...
		maple0x86data2[pos+13]=input_port_read(machine, "IN3"); // bits 2Pstart 2Pservice 2Pup 2Pdown 2Pleft 2Pright 2Ppush1 2Ppush2
		maple0x86data2[pos+14]=input_port_read(machine, "IN4"); // bits 2Ppush3 2Ppush4 2Ppush5 2Ppush6 2Ppush7 2Ppush8 ...
	}
	//2 coin slots
	maple0x86data2[pos+15]=1; // report
	maple0x86data2[pos+16]=(dc_coin_counts[0] >> 8) & 0xff; // 1CONDITION, 1SLOT COIN(bit13-8)
	maple0x86data2[pos+17]=dc_coin_counts[0] & 0xff; // 1SLOT COIN(bit7-0)
	maple0x86data2[pos+18]=(dc_coin_counts[1] >> 8) & 0xff; // 2CONDITION, 2SLOT COIN(bit13-8)
	maple0x86data2[pos+19]=dc_coin_counts[1] & 0xff; // 2SLOT COIN(bit7-0)
	//3 ad stick
	maple0x86data2[pos+20]=1; // report
	maple0x86data2[pos+21]=0xff; // channel 1 bits 7-0
	maple0x86data2[pos+22]=0; // channel 1
	maple0x86data2[pos+23]=0; // channel 2 bits 7-0
	maple0x86data2[pos+24]=0xff; // channel 2
	//4 rotary
	maple0x86data2[pos+25]=0;
	maple0x86data2[pos+26]=0;
	//5 keyboard
	//6 (touch?) screen
	//7 card reader
	//8 hopper out
	//9 driver out
	//10 analog out
	//11 character
	//12 backup

	maple0x86data2[pos+7]=17+2;
	return 17;
}

static int jvsboard_direct_read(running_machine *machine)
{
	/* valid data check*/
	maple0x86data2[0x11] = 0x00;
	maple0x86data2[0x12] = 0x8e;
	maple0x86data2[0x13] = 0x01;
	maple0x86data2[0x14] = 0x00;
	maple0x86data2[0x15] = 0xff;
	maple0x86data2[0x16] = 0xe0;
	maple0x86data2[0x19] = 0x01;

	/* read the inputs */
	//1 digital inputs
	maple0x86data2[0x1a]=1;
	if(jvsboard_type == JVSBD_MAHJONG)
		maple0x86data2[0x1b]=1; //number of players
	else //if JVSBD_DEFAULT
		maple0x86data2[0x1b]=2; //number of players
	if(jvsboard_type == JVSBD_MAHJONG)
	{
		maple0x86data2[0x1c]=input_port_read(machine, "IN1");
		maple0x86data2[0x1d]=input_port_read(machine, "IN2");
		maple0x86data2[0x1e]=input_port_read(machine, "IN3");
		maple0x86data2[0x1f]=input_port_read(machine, "IN4");
	}
	else
	{
		maple0x86data2[0x1c]=input_port_read(machine, "IN1");
		maple0x86data2[0x1d]=input_port_read(machine, "IN2");
		maple0x86data2[0x1e]=input_port_read(machine, "IN3");
		maple0x86data2[0x1f]=input_port_read(machine, "IN4");
	}
	//2 coin slots
	maple0x86data2[0x20]=1;
	maple0x86data2[0x21]=(dc_coin_counts[0] >> 8) & 0xff; //coin counter read-back hi byte
	maple0x86data2[0x22]=dc_coin_counts[0] & 0xff; //coin counter read-back lo byte
	maple0x86data2[0x23]=(dc_coin_counts[1] >> 8) & 0xff;
	maple0x86data2[0x24]=dc_coin_counts[1] & 0xff;
	//3 ad stick
	maple0x86data2[0x25]=1;
	maple0x86data2[0x26]=1;
	maple0x86data2[0x27]=0;
	maple0x86data2[0x28]=0;
	maple0x86data2[0x29]=0;
	//4 rotary
	//5 keyboard
	//6 (touch?) screen
	//7 card reader
	//8 hopper out
	//9 driver out
	//10 analog out
	//11 character
	//12 backup
	maple0x86data2[0x2a]=0;
	/*0x2b-0x2f rotary inputs */
	//...

	return 8+0x11+16;
}

READ64_HANDLER( dc_sysctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x41) && (reg != 0x42) && (reg != 0x23) && (reg > 2))	// filter out IRQ status reads
	{
		mame_printf_verbose("SYSCTRL: [%08x] read %x @ %x (reg %x: %s), mask %" I64FMT "x (PC=%x)\n", 0x5f6800+reg*4, dc_sysctrl_regs[reg], offset, reg, sysctrl_names[reg], mem_mask, cpu_get_pc(space->cpu));
	}
	#endif

	return (UINT64)dc_sysctrl_regs[reg] << shift;
}

WRITE64_HANDLER( dc_sysctrl_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;
	UINT32 address;
	struct sh4_ddt_dma ddtdata;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = dc_sysctrl_regs[reg];
	dc_sysctrl_regs[reg] = dat; // 5f6800+off*4=dat
	switch (reg)
	{
		case SB_C2DST:
			address=dc_sysctrl_regs[SB_C2DSTAT];
			ddtdata.destination=address;
			/* 0 rounding size = 16 Mbytes */
			if(dc_sysctrl_regs[SB_C2DLEN] == 0) { dc_sysctrl_regs[SB_C2DLEN] = 0x1000000; }
			ddtdata.length=dc_sysctrl_regs[SB_C2DLEN];
			ddtdata.size=1;
			ddtdata.direction=0;
			ddtdata.channel=2;
			ddtdata.mode=25; //011001
			sh4_dma_ddt(cputag_get_cpu(space->machine, "maincpu"),&ddtdata);
			#if DEBUG_SYSCTRL
			if ((address >= 0x11000000) && (address <= 0x11FFFFFF))
				if (dc_sysctrl_regs[SB_LMMODE0])
					printf("SYSCTRL: Ch2 direct display lists dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]); // 1
				else
					mame_printf_verbose("SYSCTRL: Ch2 direct textures dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]); // 0
			else if ((address >= 0x13000000) && (address <= 0x13FFFFFF))
				if (dc_sysctrl_regs[SB_LMMODE1])
					printf("SYSCTRL: Ch2 direct display lists dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]); // 1
				else
					mame_printf_verbose("SYSCTRL: Ch2 direct textures dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]); // 0
			else if ((address >= 0x10800000) && (address <= 0x10ffffff))
				mame_printf_verbose("SYSCTRL: Ch2 YUV dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]);
			else if ((address >= 0x10000000) && (address <= 0x107fffff))
				mame_printf_verbose("SYSCTRL: Ch2 TA Display List dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]);
			else
				mame_printf_verbose("SYSCTRL: Ch2 unknown dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]);
			#endif
			if ((address >= 0x10000000) && (address <= 0x10ffffff))
			{
				dc_sysctrl_regs[SB_C2DSTAT]=address;
			}
			else
			{
				dc_sysctrl_regs[SB_C2DSTAT]=address+ddtdata.length;
			}

			dc_sysctrl_regs[SB_C2DLEN]=0;
			dc_sysctrl_regs[SB_C2DST]=0;
			dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_CH2;
			break;

		case SB_ISTNRM:
			dc_sysctrl_regs[SB_ISTNRM] = old & ~(dat | 0xC0000000); // bits 31,30 ro
			break;

		case SB_ISTEXT:
			dc_sysctrl_regs[SB_ISTEXT] = old;
			break;

		case SB_ISTERR:
			dc_sysctrl_regs[SB_ISTERR] = old & ~dat;
			break;
		case SB_SDST:
			#if DEBUG_SYSCTRL
			mame_printf_verbose("SYSCTRL: Sort-DMA not supported yet !!!\n");
			#endif
			break;
	}
	dc_update_interrupt_status(space->machine);

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x42) && (reg > 2))	// filter out IRQ acks and ch2 dma
	{
		mame_printf_verbose("SYSCTRL: write %" I64FMT "x to %x (reg %x), mask %" I64FMT "x\n", data>>shift, offset, reg, /*sysctrl_names[reg],*/ mem_mask);
	}
	#endif
}

// Naomi/Naomi2 implementation with Maple/JVS bridging
READ64_HANDLER( naomi_maple_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);

	#if DEBUG_MAPLE_REGS
	mame_printf_verbose("MAPLE:  Unmapped read %08x\n", 0x5f6c00+reg*4);
	#endif
	return (UINT64)maple_regs[reg] << shift;
}

WRITE64_HANDLER( naomi_maple_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;
	struct sh4_ddt_dma ddtdata;
	UINT32 buff[512];
	UINT32 endflag,port,pattern,length,command,dap,sap,destination;
	UINT32 subcommand;
	static int jvs_command = 0,jvs_address = 0;
	int chk;
	int a;
	int off,len,func;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = maple_regs[reg];

	#if DEBUG_MAPLE_REGS
	mame_printf_verbose("MAPLE: [%08x=%x] write %" I64FMT "x to %x (reg %x: %s), mask %" I64FMT "x\n", 0x5f6c00+reg*4, dat, data >> shift, offset, reg, maple_names[reg], mem_mask);
	#endif

	maple_regs[reg] = dat; // 5f6c00+reg*4=dat
	switch (reg)
	{
	case SB_MDST:
		maple_regs[reg] = old;
		if (!(old & 1) && (dat & 1) && maple_regs[SB_MDEN] & 1) // 0 -> 1
		{
			if (!(maple_regs[SB_MDTSEL] & 1))
			{
				maple_regs[reg] = 1;
				dat=maple_regs[SB_MDSTAR];
//              printf("Maple DMA: %08x %08x %08x %08x\n",maple_regs[SB_MDSTAR],maple_regs[SB_MDTSEL],maple_regs[SB_MDEN],maple_regs[SB_MDST]);
//              printf("           %08x %08x %08x %08x\n",maple_regs[SB_MSYS],maple_regs[SB_MST],maple_regs[SB_MSHTCL],maple_regs[SB_MMSEL]);
				while (1) // do transfers
				{
					ddtdata.source=dat;		// source address
					ddtdata.length=3;		// words to transfer
					ddtdata.size=4;			// bytes per word
					ddtdata.buffer=buff;	// destination buffer
					ddtdata.direction=0;	// 0 source to buffer, 1 buffer to source
					ddtdata.channel= -1;	// not used
					ddtdata.mode= -1;		// copy from/to buffer
					sh4_dma_ddt(cputag_get_cpu(space->machine, "maincpu"), &ddtdata);

					maple_regs[reg] = 0;
					endflag=buff[0] & 0x80000000;
					port=(buff[0] >> 16) & 3;
					pattern=(buff[0] >> 8) & 7;
					length=buff[0] & 255;
					//if(length == 0)
					//  length = 0x100;
					destination=buff[1];
					command=buff[2] & 255;
					dap=(buff[2] >> 8) & 255;
					sap=(buff[2] >> 16) & 255;
					func = buff[3];
					//buff[0]=0;
					//if(buff[1] == 0x700)
					//  printf("%08x %08x",buff[0],buff[2]);
					ddtdata.size=4;

					if (pattern == 0)
					{
						if (port > 0)
							buff[0]=0xffffffff;
						switch (command)
						{
							case 1:
							case 3:
								ddtdata.length=1;
								#if DEBUG_MAPLE
								printf("MAPLE: transfer command %x port %x length %x\n", command, port, length);
								#endif
								break;

							case 0x80: // get data and compute checksum
								ddtdata.length=length;
								ddtdata.direction=0;
								ddtdata.channel= -1;
								ddtdata.mode=-1;
								sh4_dma_ddt(cputag_get_cpu(space->machine, "maincpu"),&ddtdata);
								chk=0;
								for (a=1;a < length;a++)
								{
									chk=chk+(signed char)(buff[a] & 255);
									chk=chk+(signed char)((buff[a] >> 8) & 255);
									chk=chk+(signed char)((buff[a] >> 16) & 255);
									chk=chk+(signed char)((buff[a] >> 24) & 255);
								}
								chk=chk+(buff[0] & 255); // address (big endian !) last is 0xffffffff
								chk=chk+((buff[0] >> 8) & 255);
								chk=chk+((buff[0] >> 16) & 255);
								chk=chk+((buff[0] >> 24) & 255);
								buff[1]=chk;
								ddtdata.length=2;
								break;
							case 0x82: // get license string
								for (a=0;a < 16;a++)
								{
									buff[a]=maple0x82answer[a];
								}
								ddtdata.length=16;
								break;
							case 0x86:
								ddtdata.length=length;
								ddtdata.direction=0;
								ddtdata.channel= -1;
								ddtdata.mode=-1;
								sh4_dma_ddt(cputag_get_cpu(space->machine, "maincpu"),&ddtdata);

								subcommand = buff[0] & 0xff;
								#if DEBUG_MAPLE
								printf("MAPLE: transfer command %x port %x subcommand %x\n", command, port, subcommand);
								#endif
								if (subcommand == 3) // read data
								{
									for (a=0;a < 0x80;a+=4)
									{
										buff[1+a/4]=(maple0x86data1[a+3] << 24)+(maple0x86data1[a+2] << 16)+(maple0x86data1[a+1] << 8)+maple0x86data1[a];
									}
									ddtdata.length=0x84/4;
								}
								else if (subcommand == 0xb) // store data
								{
									off=(buff[0] >> 8) & 0xff;
									len=(buff[0] >> 16) & 0xff;
									for (a=0;a < len;a++)
										maple0x86data1[off+a]=(buff[1+a/4] >> (a & 3) * 8) & 255;
								}
								else if ((subcommand == 0x31) || (subcommand == 0x1))
								{
									ddtdata.length=1;
								}
								else if (subcommand == 0x17) // send command into jvs serial bus !!!
								{
									// Examples:
									// 17,*c295407 (77),*c295404,*c295405,*c295406,0,ff,2,f0,d9, 0 // ff broadcast
									// 17,*c295407 (77),*c295404,*c295405,*c295406,0,ff,2,f1,01, 0 // 01 can be any from 01 to 0x1f
									// 17,*c295407 (77),*c295404,*c295405,*c295406,0,01,1,10,    01,0 // 01 is address
									// Currently:
									// 17,?,?,?,?,?,slave address,length,jvs command bytes...
									jvs_address = (buff[1] >> 16) & 0xff; // slave address
									jvs_command = buff[2] & 0xff; // jvs command
									#if DEBUG_MAPLE
									printf("MAPLE: sent jvs command %x\n", jvs_command);
									#endif
									//buff[1] = 0xe4e3e2e1;
									ddtdata.length = 0;//2;
								}
								else if (subcommand == 0x15) // get response from previous jvs command
								{
									int tocopy, pos;
									// 15,0,0,0
									maple0x86data2[0] = 0xa0;
									for (a=1;a < 32;a++)
										maple0x86data2[a] = maple0x86data2[a-1] + 1;
									maple0x86data2[3] = maple0x86data2[3] | 0x0c;
									maple0x86data2[6] = maple0x86data2[6] & 0xcf;

									a = input_port_read(space->machine, "IN0"); // put keys here
									maple0x86data2[6] = maple0x86data2[6] | (a << 4);
									pos = 0x11;
									tocopy = 17;
									for (;;)
									{
										if (jvs_command)
											maple0x86data2[pos+2] = jvs_address;
										else
											maple0x86data2[pos+2] = 0;
										maple0x86data2[pos+3] = 0;
										maple0x86data2[pos+8] = 1;
										maple0x86data2[pos+1] = 0x8e;
										maple0x86data2[pos+9] = 1;
										maple0x86data2[pos+5] = 0xe0;
										// 4 + 1 + 0x10 + ?,8e,addr,0,?,?,addr?,len,status,report1,jvsbytes...
										ddtdata.length=11;
										tocopy += 10;
										switch (jvs_command)
										{
											case 0xf0: // reset
											case 0xf1: // set address
												break;
											case 0x10:
												strcpy((char *)(maple0x86data2+0x11+10), "SEGA ENTERPRISES,LTD.\nI/O BD JVS\n837-13551"); // name
												maple0x86data2[pos+7]=24+21+2;
												tocopy += 24+21;
												break;
											case 0x11:
												maple0x86data2[pos+10]=0x13; // version bcd
												maple0x86data2[pos+7]=1+2;
												tocopy += 1;
												break;
											case 0x12:
												maple0x86data2[pos+10]=0x30; // version bcd
												maple0x86data2[pos+7]=1+2;
												tocopy += 1;
												break;
											case 0x13:
												maple0x86data2[pos+10]=0x10; // version bcd
												maple0x86data2[pos+7]=1+2;
												tocopy += 1;
												break;
											case 0x14:
												{
													tocopy += jvsboard_init(pos);
												}
												break;
											case 0x21:
												maple0x86data2[pos+10]=(dc_coin_counts[0] >> 8) & 0xff; // bits 7-6 status bits 5-0 higer bits of coin count
												maple0x86data2[pos+11]=dc_coin_counts[0] & 0xff; // lower bits of coin count
												maple0x86data2[pos+12]=(dc_coin_counts[1] >> 8) & 0xff; // like previuos two but for second coin slot
												maple0x86data2[pos+13]=dc_coin_counts[1] & 0xff;
												maple0x86data2[pos+7]=4+2;
												tocopy += 4;
												break;
											case -1: // special case to read controls
											case -2:
												jvsboard_indirect_read(space->machine,pos);
												if (jvs_command == -1)
												{
													// ?
													maple0x86data2[pos+25]=0;
													maple0x86data2[pos+26]=0;
													maple0x86data2[pos+7]=17+2;
													tocopy += 17;
												}
												else
												{
													maple0x86data2[pos+7]=15+2;
													tocopy += 15;
												}
												break;
										}
										if (jvs_command == -1)
										{
											maple0x86data2[pos+4]=maple0x86data2[pos+7]+5;
											pos = pos+maple0x86data2[pos+4]+3;
											jvs_command = 0;
											continue;
										}
										else
										{
											maple0x86data2[pos+4]=maple0x86data2[pos+7]-1;
											break;
										}
									}
									for (a=0;a < tocopy;a=a+4)
										buff[1+a/4] = maple0x86data2[a] | (maple0x86data2[a+1] << 8) | (maple0x86data2[a+2] << 16) | (maple0x86data2[a+3] << 24);
									jvs_command=0;
									ddtdata.length = (tocopy+7)/4;
								}
								else if (subcommand == 0x33) // "direct mode" input port reading
								{
									int tocopy;

									maple0x86data2[0] = 0xa0;
									for (a=1;a < 32;a++)
										maple0x86data2[a] = maple0x86data2[a-1] + 1;
									maple0x86data2[3] = maple0x86data2[3] | 0x0c;
									maple0x86data2[6] = maple0x86data2[6] & 0xcf;

									a = input_port_read(space->machine, "IN0"); // put keys here
									maple0x86data2[6] = maple0x86data2[6] | (a << 4);

									tocopy = jvsboard_direct_read(space->machine);

									// command end flag
									maple0x86data2[0x18] = (jvs_command == -1) ? 19 : 17;
									maple0x86data2[0x15] = (jvs_command == -1) ? 24 : 16;

									for (a=0;a < tocopy;a=a+4)
										buff[1+a/4] = maple0x86data2[a] | (maple0x86data2[a+1] << 8) | (maple0x86data2[a+2] << 16) | (maple0x86data2[a+3] << 24);

									//jvs_command=0;
									ddtdata.length = (tocopy+7)/4;
								}
								else if (subcommand == 0x21)
								{
									// 21,*c295407 (77),*c295404,*c295405,*c295406,0,1,0
									jvs_address = (buff[1] >> 16) & 0xff; // slave address
									jvs_command = -2;
									ddtdata.length=2;
								}
								else if (subcommand == 0x27) //0x27
								{
									// 27,*c295407 (77),*c295404,*c295405,*c295406,0,01,1,0,0,0
									jvs_address = (buff[1] >> 16) & 0xff; // slave address
									jvs_command = -1;
									ddtdata.length=2;
								}
								else // 0x13
								{
									ddtdata.length=2;
								}
								break;
							default:
								#if DEBUG_MAPLE
								printf("MAPLE: unknown transfer command %x port %x\n", command, port);
								#endif
								ddtdata.length=0;
								endflag = 1; /*TODO: check this */
								//buff[0]=0xffffffff;
								break;
						}
					}
					ddtdata.destination=destination;
					ddtdata.buffer=buff;
					ddtdata.direction=1;
					sh4_dma_ddt(cputag_get_cpu(space->machine, "maincpu"),&ddtdata);

					if (endflag)
					{
						#if 1 //ENABLE_MAPLE_IRQ
						/*TODO: this fixes moeru but breaks other games, understand why.*/
						dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_MAPLE;
						dc_update_interrupt_status(space->machine);
						#endif
						break;
					}
					// skip fixed packet header
					dat += 8;
					// skip transfer data
					dat += (length + 1) * 4;
				} // do transfers
				maple_regs[reg] = 0;
			}
			else
			{
				#if DEBUG_MAPLE
				printf("MAPLE: hardware trigger not supported yet\n");
				#endif
			}
		}
		break;
	}
}

// implementation for Dreamcast and Atomiswave supports only standard Sega Dreamcast Maple commands and peripherals
READ64_HANDLER( dc_maple_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);

	#if DEBUG_MAPLE_REGS
	mame_printf_verbose("MAPLE:  Unmapped read %08x\n", 0x5f6c00+reg*4);
	#endif
	return (UINT64)maple_regs[reg] << shift;
}

WRITE64_HANDLER( dc_maple_w )
{
	int reg, func;
	UINT64 shift;
	UINT32 old,dat;
	struct sh4_ddt_dma ddtdata;
	UINT32 buff[512];
	UINT32 endflag,port,pattern,length,command,dap,sap,destination;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = maple_regs[reg];

	#if DEBUG_MAPLE_REGS
	mame_printf_verbose("MAPLE: [%08x=%x] write %" I64FMT "x to %x (reg %x: %s), mask %" I64FMT "x\n", 0x5f6c00+reg*4, dat, data >> shift, offset, reg, maple_names[reg], mem_mask);
	#endif

	maple_regs[reg] = dat; // 5f6c00+reg*4=dat
	switch (reg)
	{
	case SB_MDST:
		maple_regs[reg] = old;
		if (!(old & 1) && (dat & 1) && maple_regs[SB_MDEN] & 1) // 0 -> 1
		{
			if (!(maple_regs[SB_MDTSEL] & 1))
			{
				maple_regs[reg] = 1;
				dat=maple_regs[SB_MDSTAR];
//              printf("Maple DMA: %08x %08x %08x %08x\n",maple_regs[SB_MDSTAR],maple_regs[SB_MDTSEL],maple_regs[SB_MDEN],maple_regs[SB_MDST]);
//              printf("           %08x %08x %08x %08x\n",maple_regs[SB_MSYS],maple_regs[SB_MST],maple_regs[SB_MSHTCL],maple_regs[SB_MMSEL]);
				while (1) // do transfers
				{
					ddtdata.source=dat;		// source address
					ddtdata.length=3;		// words to transfer
					ddtdata.size=4;			// bytes per word
					ddtdata.buffer=buff;	// destination buffer
					ddtdata.direction=0;	// 0 source to buffer, 1 buffer to source
					ddtdata.channel= -1;	// not used
					ddtdata.mode= -1;		// copy from/to buffer
					sh4_dma_ddt(cputag_get_cpu(space->machine, "maincpu"), &ddtdata);

					maple_regs[reg] = 0;
					endflag=buff[0] & 0x80000000;
					port=(buff[0] >> 16) & 3;
					pattern=(buff[0] >> 8) & 7;
					length=buff[0] & 255;
					//if(length == 0)
					//  length = 0x100;
					destination=buff[1];
					command=buff[2] & 255;
					dap=(buff[2] >> 8) & 255;
					sap=(buff[2] >> 16) & 255;
					func = buff[3];
					//buff[0]=0;
					//if(buff[1] == 0x700)
					//  printf("%08x %08x",buff[0],buff[2]);
					ddtdata.size=4;

					if (pattern == 0)
					{
						char pL[8], pH[8];

						if (port > 0)
							buff[0]=0xffffffff;
						switch (command)
						{
							case 1: // AW/DC MAPLE_CMD_INFO
								ddtdata.length = sizeof(dc_controller_id)/4;

								buff[0] = 5;	// MAPLE_RESP_INFO
								memcpy(&buff[1], dc_controller_id, sizeof(dc_controller_id));
								break;

							case 2: // AW/DC MAPLE_CMD_EXT_INFO
								ddtdata.length = sizeof(dc_controller_id)+sizeof(dc_controller_version)/4;

								buff[0] = 6;	// MAPLE_RESP_EXT_INFO
								memcpy(&buff[1], dc_controller_id, sizeof(dc_controller_id));
								memcpy(&buff[1+(0x70/4)], dc_controller_version, sizeof(dc_controller_version));
								break;

							case 3:	// AW/DC MAPLE_CMD_RESET
								ddtdata.length=1;
								#if DEBUG_MAPLE
								printf("MAPLE: transfer command %x port %x length %x\n", command, port, length);
								#endif
								break;

							case 9: // AW/DC MAPLE_CMD_GET_COND
								buff[0] = 8;	// MAPLE_RESP_DATA

								sprintf(pL, "P%dL", port+1);
								sprintf(pH, "P%dH", port+1);

								buff[1] = func;
								buff[2] = input_port_read(space->machine, pH)<<8 | input_port_read(space->machine, pL) | 0xffff0000;
								buff[3] = 0xffffffff;
								ddtdata.length=(8/4)+1;
								break;
							default:
								#if DEBUG_MAPLE
								printf("MAPLE: unknown transfer command %x port %x\n", command, port);
								#endif
								ddtdata.length=0;
								endflag = 1; /*TODO: check this */
								//buff[0]=0xffffffff;
								break;
						}
					}
					ddtdata.destination=destination;
					ddtdata.buffer=buff;
					ddtdata.direction=1;
					sh4_dma_ddt(cputag_get_cpu(space->machine, "maincpu"),&ddtdata);

					if (endflag)
					{
						#if ENABLE_MAPLE_IRQ
						dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_MAPLE;
						dc_update_interrupt_status(space->machine);
						#endif
						break;
					}
					// skip fixed packet header
					dat += 8;
					// skip transfer data
					dat += (length + 1) * 4;
				} // do transfers
				maple_regs[reg] = 0;
			}
			else
			{
				#if DEBUG_MAPLE
				printf("MAPLE: hardware trigger not supported yet\n");
				#endif
			}
		}
		break;
	}
}

INPUT_CHANGED( dc_coin_slots_callback )
{
	UINT32 *counter = (UINT32 *)param;

	/* check for a 0 -> 1 transition */
	if (!oldval && newval)
		*counter += 1;
}

READ64_HANDLER( dc_gdrom_r )
{
	UINT32 off;

	if ((int)~mem_mask & 1)
	{
		off=(offset << 1) | 1;
	}
	else
	{
		off=offset << 1;
	}

	if (off*4 == 0x4c)
		return -1;
	if (off*4 == 8)
		return 0;

	return 0;
}

WRITE64_HANDLER( dc_gdrom_w )
{
	UINT32 dat,off;

	if ((int)~mem_mask & 1)
	{
		dat=(UINT32)(data >> 32);
		off=(offset << 1) | 1;
	}
	else
	{
		dat=(UINT32)data;
		off=offset << 1;
	}

	mame_printf_verbose("GDROM: [%08x=%x]write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x5f7000+off*4, dat, data, offset, mem_mask);
}

READ64_HANDLER( dc_g1_ctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	mame_printf_verbose("G1CTRL:  Unmapped read %08x\n", 0x5f7400+reg*4);
	return (UINT64)g1bus_regs[reg] << shift;
}

WRITE64_HANDLER( dc_g1_ctrl_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;
	struct sh4_ddt_dma ddtdata;
 	UINT8 *ROM;
 	UINT32 dmaoffset;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = g1bus_regs[reg];

	g1bus_regs[reg] = dat; // 5f7400+reg*4=dat
	mame_printf_verbose("G1CTRL: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x5f7400+reg*4, dat, data, offset, mem_mask);
	switch (reg)
	{
	case SB_GDST:
		if (dat & 1 && g1bus_regs[SB_GDEN] == 1) // 0 -> 1
		{
			if (g1bus_regs[SB_GDDIR] == 0)
			{
				mame_printf_verbose("G1CTRL: unsupported transfer\n");
				return;
			}
//          printf("ROM board DMA to %x len %x (PC %x)\n", g1bus_regs[SB_GDSTAR], g1bus_regs[SB_GDLEN], cpu_get_pc(space->cpu));
 			ROM = (UINT8 *)devtag_get_info_ptr(space->machine, "rom_board", DEVINFO_PTR_MEMORY);
 			dmaoffset = (UINT32)devtag_get_info_int(space->machine, "rom_board", DEVINFO_INT_DMAOFFSET);
			ddtdata.destination=g1bus_regs[SB_GDSTAR];		// destination address
			ddtdata.length=g1bus_regs[SB_GDLEN] >> 5;		// words to transfer
			ddtdata.size=32;			// bytes per word
 			ddtdata.buffer=ROM+dmaoffset;	// buffer address
			ddtdata.direction=1;	// 0 source to buffer, 1 buffer to destination
			ddtdata.channel= -1;	// not used
			ddtdata.mode= -1;		// copy from/to buffer
 			mame_printf_verbose("G1CTRL: transfer %x from ROM %08x to sdram %08x\n", g1bus_regs[SB_GDLEN], dmaoffset, g1bus_regs[SB_GDSTAR]);
			sh4_dma_ddt(cputag_get_cpu(space->machine, "maincpu"), &ddtdata);
			g1bus_regs[SB_GDST]=0;
			dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_GDROM;
			dc_update_interrupt_status(space->machine);
		}
		break;
	}
}

READ64_HANDLER( dc_g2_ctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	mame_printf_verbose("G2CTRL:  Unmapped read %08x\n", 0x5f7800+reg*4);
	return (UINT64)g2bus_regs[reg] << shift;
}

WRITE64_HANDLER( dc_g2_ctrl_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;
	static struct {
		UINT32 aica_addr;
		UINT32 root_addr;
		UINT32 size;
		UINT8 dir;
		UINT8 flag;
		UINT8 indirect;
		UINT8 start;
		UINT8 sel;
	}wave_dma;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	g2bus_regs[reg] = dat; // 5f7800+reg*4=dat

	switch (reg)
	{
		/*AICA Address register*/
		case SB_ADSTAG: wave_dma.aica_addr = dat; break;
		/*Root address (work ram)*/
		case SB_ADSTAR:	wave_dma.root_addr = dat; break;
		/*DMA size (in dword units, bit 31 is "set dma initiation enable setting to 0"*/
		case SB_ADLEN:
			wave_dma.size = dat & 0x7fffffff;
			wave_dma.indirect = (dat & 0x80000000)>>31;
			break;
		/*0 = root memory to aica / 1 = aica to root memory*/
		case SB_ADDIR: wave_dma.dir = (dat & 1); break;
		/*dma flag (active HIGH, bug in docs)*/
		case SB_ADEN: wave_dma.flag = (dat & 1); break;
		case SB_ADTSEL: wave_dma.sel = dat & 7; break;
		/*ready for dma'ing*/
		case SB_ADST:
			wave_dma.start = dat & 1;

			#if DEBUG_AICA_DMA
			printf("AICA: G2-DMA start \n");
			printf("DST %08x SRC %08x SIZE %08x IND %02x\n",wave_dma.aica_addr,wave_dma.root_addr,wave_dma.size,wave_dma.indirect);
			printf("SEL %08x ST  %08x FLAG %08x DIR %02x\n",wave_dma.sel,wave_dma.start,wave_dma.flag,wave_dma.dir);
			#endif

			//mame_printf_verbose("SB_ADST data %08x\n",dat);
			if(wave_dma.flag && wave_dma.start)
			{
				UINT32 src,dst,size;
				dst = wave_dma.aica_addr;
				src = wave_dma.root_addr;
				size = 0;
				/* 0 rounding size = 32 Mbytes */
				if(wave_dma.size == 0) { wave_dma.size = 0x200000; }

				if(wave_dma.dir == 0)
				{
					for(;size<wave_dma.size;size+=4)
					{
						memory_write_dword_64le(space,dst,memory_read_dword(space,src));
						src+=4;
						dst+=4;
					}
				}
				else
				{
					for(;size<wave_dma.size;size+=4)
					{
						memory_write_dword_64le(space,src,memory_read_dword(space,dst));
						src+=4;
						dst+=4;
					}
				}
				/* update the params*/
				wave_dma.aica_addr = g2bus_regs[SB_ADSTAG] = dst;
				wave_dma.root_addr = g2bus_regs[SB_ADSTAR] = src;
				wave_dma.size = g2bus_regs[SB_ADLEN] = 0;
				wave_dma.flag = (wave_dma.indirect & 1) ? 1 : 0;
				wave_dma.start = g2bus_regs[SB_ADST] = 0;
				/*TODO: this makes the sfz3upper to not play any bgm, understand why. */
				//dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_AICA;
				//dc_update_interrupt_status(space->machine);
			}
			break;

		case SB_ADSUSP:
		case SB_E1SUSP:
		case SB_E2SUSP:
		case SB_DDSUSP:
		case SB_G2APRO:
			break;

		default:
			/* might access the unhandled DMAs, so tell us if this happens. */
			printf("Unhandled G2 register [%08x] -> %08x\n",reg,dat);
			break;
	}
}

READ64_HANDLER( dc_modem_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);

	// from ElSemi: this makes Atomiswave do it's "verbose boot" with a Sammy logo and diagnostics instead of just running the cart.
	// our PVR emulation is apparently not good enough for that to work yet though.
	if (reg == 0x280/4)
	{
		return U64(0xffffffffffffffff);
	}

	mame_printf_verbose("MODEM:  Unmapped read %08x\n", 0x600000+reg*4);
	return 0;
}

WRITE64_HANDLER( dc_modem_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	mame_printf_verbose("MODEM: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x600000+reg*4, dat, data, offset, mem_mask);
}

READ64_HANDLER( dc_rtc_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg3216_64(space->machine, offset, mem_mask, &shift);
	mame_printf_verbose("RTC:  Unmapped read %08x\n", 0x710000+reg*4);
	return (UINT64)dc_rtcregister[reg] << shift;
}

WRITE64_HANDLER( dc_rtc_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;

	reg = decode_reg3216_64(space->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = dc_rtcregister[reg];
	dc_rtcregister[reg] = dat & 0xFFFF; // 5f6c00+off*4=dat
	switch (reg)
	{
	case RTC1:
		if (dc_rtcregister[RTC3])
			dc_rtcregister[RTC3] = 0;
		else
			dc_rtcregister[reg] = old;
		break;
	case RTC2:
		if (dc_rtcregister[RTC3] == 0)
			dc_rtcregister[reg] = old;
		else
			timer_adjust_periodic(dc_rtc_timer, attotime_zero, 0, ATTOTIME_IN_SEC(1));
		break;
	case RTC3:
		dc_rtcregister[RTC3] &= 1;
		break;
	}
	mame_printf_verbose("RTC: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x710000 + reg*4, dat, data, offset, mem_mask);
}

static TIMER_CALLBACK(dc_rtc_increment)
{
    dc_rtcregister[RTC2] = (dc_rtcregister[RTC2] + 1) & 0xFFFF;
    if (dc_rtcregister[RTC2] == 0)
        dc_rtcregister[RTC1] = (dc_rtcregister[RTC1] + 1) & 0xFFFF;
}

MACHINE_START( dc )
{
}

MACHINE_RESET( dc )
{
	/* halt the ARM7 */
	cputag_set_input_line(machine, "soundcpu", INPUT_LINE_RESET, ASSERT_LINE);

	memset(dc_sysctrl_regs, 0, sizeof(dc_sysctrl_regs));
	memset(maple_regs, 0, sizeof(maple_regs));
	memset(dc_rtcregister, 0, sizeof(dc_rtcregister));
	memset(dc_coin_counts, 0, sizeof(dc_coin_counts));

	dc_rtc_timer = timer_alloc(machine, dc_rtc_increment, 0);
	timer_adjust_periodic(dc_rtc_timer, attotime_zero, 0, ATTOTIME_IN_SEC(1));

	dc_sysctrl_regs[SB_SBREV] = 0x0b;
}

READ64_DEVICE_HANDLER( dc_aica_reg_r )
{
	//int reg;
	UINT64 shift;

	/*reg = */decode_reg32_64(device->machine, offset, mem_mask, &shift);

//  mame_printf_verbose("AICA REG: [%08x] read %" I64FMT "x, mask %" I64FMT "x\n", 0x700000+reg*4, (UINT64)offset, mem_mask);

	return (UINT64) aica_r(device, offset*2, 0xffff)<<shift;
}

WRITE64_DEVICE_HANDLER( dc_aica_reg_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg32_64(device->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	if (reg == (0x2c00/4))
	{
		if (dat & 1)
		{
			/* halt the ARM7 */
			cputag_set_input_line(device->machine, "soundcpu", INPUT_LINE_RESET, ASSERT_LINE);
		}
		else
		{
			/* it's alive ! */
			cputag_set_input_line(device->machine, "soundcpu", INPUT_LINE_RESET, CLEAR_LINE);
		}
    }

	aica_w(device, offset*2, dat, shift ? ((mem_mask>>32)&0xffff) : (mem_mask & 0xffff));

//  mame_printf_verbose("AICA REG: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x700000+reg*4, dat, data, offset, mem_mask);
}

READ32_DEVICE_HANDLER( dc_arm_aica_r )
{
	return aica_r(device, offset*2, 0xffff) & 0xffff;
}

WRITE32_DEVICE_HANDLER( dc_arm_aica_w )
{
	aica_w(device, offset*2, data, mem_mask&0xffff);
}

