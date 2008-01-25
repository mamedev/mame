/*

    dc.c - Sega Dreamcast hardware

*/

#include "driver.h"
#include "deprecat.h"
#include "dc.h"
#include "cpu/sh4/sh4.h"

#define DEBUG_REGISTERS	(1)

#if DEBUG_REGISTERS

#define DEBUG_SYSCTRL	(1)
#define DEBUG_MAPLE	(0)

#if DEBUG_SYSCTRL
static const char *sysctrl_names[] =
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
static const char *maple_names[] =
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

UINT32 sysctrl_regs[0x200/4];
static UINT32 maple_regs[0x100/4];
static UINT32 dc_rtcregister[4];

static UINT32 maple0x82answer[]=
{
	0x07200083,0x2d353133,0x39343136,0x20202020,0x59504f43,0x48474952,0x45532054,0x45204147,
	0x05200083,0x5245544e,0x53495250,0x43205345,0x544c2c4f,0x20202e44,0x38393931,0x5c525043
};

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

int compute_interrupt_level(void)
{
	UINT32 ln,lx,le;

	ln=sysctrl_regs[SB_ISTNRM] & sysctrl_regs[SB_IML6NRM];
	lx=sysctrl_regs[SB_ISTEXT] & sysctrl_regs[SB_IML6EXT];
	le=sysctrl_regs[SB_ISTERR] & sysctrl_regs[SB_IML6ERR];
	if (ln | lx | le)
	{
		return 6;
	}

	ln=sysctrl_regs[SB_ISTNRM] & sysctrl_regs[SB_IML4NRM];
	lx=sysctrl_regs[SB_ISTEXT] & sysctrl_regs[SB_IML4EXT];
	le=sysctrl_regs[SB_ISTERR] & sysctrl_regs[SB_IML4ERR];
	if (ln | lx | le)
	{
		return 4;
	}

	ln=sysctrl_regs[SB_ISTNRM] & sysctrl_regs[SB_IML2NRM];
	lx=sysctrl_regs[SB_ISTEXT] & sysctrl_regs[SB_IML2EXT];
	le=sysctrl_regs[SB_ISTERR] & sysctrl_regs[SB_IML2ERR];
	if (ln | lx | le)
	{
		return 2;
	}

	return 0;
}

void update_interrupt_status(void)
{
int level;

	if (sysctrl_regs[SB_ISTERR])
	{
		sysctrl_regs[SB_ISTNRM] |= 0x80000000;
	}
	else
	{
		sysctrl_regs[SB_ISTNRM] &= 0x7fffffff;
	}

	if (sysctrl_regs[SB_ISTEXT])
	{
		sysctrl_regs[SB_ISTNRM] |= 0x40000000;
	}
	else
	{
		sysctrl_regs[SB_ISTNRM] &= 0xbfffffff;
	}

	level=compute_interrupt_level();
	cpunum_set_info_int(0,CPUINFO_INT_SH4_IRLn_INPUT,15-level);
}

READ64_HANDLER( dc_sysctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x42))	// filter out IRQ status reads
	{
		mame_printf_verbose("SYSCTRL: read %x @ %x (reg %x: %s), mask %llx (PC=%x)\n", sysctrl_regs[reg], offset, reg, sysctrl_names[reg], mem_mask, activecpu_get_pc());
	}
	#endif

	return (UINT64)sysctrl_regs[reg] << shift;
}

WRITE64_HANDLER( dc_sysctrl_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;
	struct sh4_ddt_dma ddtdata;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = sysctrl_regs[reg];
	sysctrl_regs[reg] = dat; // 5f6800+off*4=dat
	switch (reg)
	{
		case SB_C2DST:
			ddtdata.destination=sysctrl_regs[SB_C2DSTAT];
			ddtdata.length=sysctrl_regs[SB_C2DLEN];
			ddtdata.size=1;
			ddtdata.direction=0;
			ddtdata.channel=2;
			ddtdata.mode=25; //011001
			cpunum_set_info_ptr(0,CPUINFO_PTR_SH4_EXTERNAL_DDT_DMA,&ddtdata);
			sysctrl_regs[SB_C2DSTAT]=sysctrl_regs[SB_C2DSTAT]+ddtdata.length;
			sysctrl_regs[SB_C2DLEN]=0;
			sysctrl_regs[SB_C2DST]=0;
			sysctrl_regs[SB_ISTNRM] |= (1 << 19);
			break;

		case SB_ISTNRM:
			sysctrl_regs[SB_ISTNRM] = old & ~(dat | 0xC0000000); // bits 31,30 ro
			break;

		case SB_ISTEXT:
			sysctrl_regs[SB_ISTEXT] = old;
			break;

		case SB_ISTERR:
			sysctrl_regs[SB_ISTERR] = old & ~dat;
			break;
	}
	update_interrupt_status();

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x42))	// filter out IRQ acks
	{
		mame_printf_verbose("SYSCTRL: write %llx to %x (reg %x: %s), mask %llx\n", data>>shift, offset, reg, sysctrl_names[reg], mem_mask);
	}
	#endif
}

READ64_HANDLER( dc_maple_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	return (UINT64)maple_regs[reg] << shift;
}

WRITE64_HANDLER( dc_maple_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;
	struct sh4_ddt_dma ddtdata;
	UINT32 buff[512];
	UINT32 endflag,port,pattern,length,command,dap,sap,destination;
	int chk;
	int a;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = maple_regs[reg];

	#if DEBUG_MAPLE
	mame_printf_verbose("MAPLE: write %llx to %x (reg %x: %s), mask %llx\n", data >> shift, offset, reg, maple_names[reg], mem_mask);
	#endif

	maple_regs[reg] = dat; // 5f6c00+reg*4=dat
	switch (reg)
	{
	case SB_MDST:
		maple_regs[reg] = old;
		if (!(old & 1) && (dat & 1)) // 0 -> 1
		{
			if (!(maple_regs[SB_MDTSEL] & 1))
			{
				maple_regs[reg] = 1;
				dat=maple_regs[SB_MDSTAR];
				while (1) // do transfers
				{
					ddtdata.source=dat;
					ddtdata.length=3;
					ddtdata.size=4;
					ddtdata.buffer=buff;
					ddtdata.direction=0;
					ddtdata.channel= -1;
					ddtdata.mode= -1;
					cpunum_set_info_ptr(0, CPUINFO_PTR_SH4_EXTERNAL_DDT_DMA, &ddtdata);

					maple_regs[reg] = 0;
					endflag=buff[0] & 0x80000000;
					port=(buff[0] >> 16) & 3;
					pattern=(buff[0] >> 8) & 7;
					length=buff[0] & 255;
					destination=buff[1];
					command=buff[2] & 255;
					dap=(buff[2] >> 8) & 255;
					sap=(buff[2] >> 16) & 255;
					buff[0]=0;
					ddtdata.size=4;

					if (pattern == 0)
					{
						switch (command)
						{
							case 1:
								ddtdata.length=1;
								break;
							case 3:
								ddtdata.length=1;
								break;
							case 0x80: // compute checksum
								ddtdata.length=length;
								ddtdata.direction=0;
								ddtdata.channel= -1;
								ddtdata.mode=-1;
								cpunum_set_info_ptr(0,CPUINFO_PTR_SH4_EXTERNAL_DDT_DMA,&ddtdata);
								chk=0;
								for (a=1;a < length;a++)
								{
									chk=chk+(signed char)(buff[a] & 255);
									chk=chk+(signed char)((buff[a] >> 8) & 255);
									chk=chk+(signed char)((buff[a] >> 16) & 255);
									chk=chk+(signed char)((buff[a] >> 24) & 255);
								}
								chk=chk+(buff[0] & 255);
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
								cpunum_set_info_ptr(0,CPUINFO_PTR_SH4_EXTERNAL_DDT_DMA,&ddtdata);

								if ((buff[0] & 0xff) == 3)
								{
									buff[1]=0x14131211;
									for (a=0;a < 31;a++)
									{
										buff[2+a]=buff[1+a]+0x04040404;
									}
									buff[1]=0x1413b1b9; // put checksum
									buff[5]=0x8ab82221; // put checksum
									ddtdata.length=0x84/4;
								}
								else if (((buff[0] & 0xff) == 0x31) || ((buff[0] & 0xff) == 0xb) || ((buff[0] & 0xff) == 0x1))
								{
									ddtdata.length=1;
								}
								else if ((buff[0] & 0xff) == 0x17) // send command into jvs serial bus !!!
								{
									// 17,*c295407 (77),*c295404,*c295405,*c295406,0,ff,2,f0,d9, 0
									// 17,*c295407 (77),*c295404,*c295405,*c295406,0,ff,2,f1,01, 0
									// 17,*c295407 (77),*c295404,*c295405,*c295406,0,01,1,10,    01,0
									switch (buff[2] & 0xff) // jvs command
									{
										case 0xf0:
										case 0xf1:
										case 0x10:
											break;
									}
									buff[1]=0xe4e3e2e1;
									ddtdata.length=2;
								}
								else if ((buff[0] & 0xff) == 0x15)
								{
									// 15,0,0,0
									buff[1]=0xA4A3A2A1;
									for (a=0;a < 7;a++)
									{
										buff[2+a]=buff[1+a]+0x04040404;
									}
									buff[1]=buff[1] | 0x0c000000;
									buff[2]=buff[2] & 0xFFCFFFFF;

									a=readinputport(0); // mettere qui tasti
									buff[2]=buff[2] | (a << 20);
									*(((unsigned char *)buff)+0x18)=0;
									*(((unsigned char *)buff)+0x1d)=1;
									*(((unsigned char *)buff)+0x16)=0x8e;
									ddtdata.length=9;
								}
								else if ((buff[0] & 0xff) == 0x21)
								{
									// 21,*c295407 (77),*c295404,*c295405,*c295406,0,1,0
									ddtdata.length=1;
								}
								else //0x27
								{
									ddtdata.length=1;
								}
								break;
						}
					}
					ddtdata.destination=destination;
					ddtdata.buffer=buff;
					ddtdata.direction=1;
					cpunum_set_info_ptr(0,CPUINFO_PTR_SH4_EXTERNAL_DDT_DMA,&ddtdata);

					if (endflag)
					{
						break;
					}
					// skip fixed packet header
					dat += 8;
					// skip transfer data
					dat += ((command & 0xff) + 1) * 4;
				} // do transfers
				maple_regs[reg] = 0;
			}
		}
		break;
	}
#if DEBUG_MAPLE
	mame_printf_verbose("MAPLE: write %llx to %x, mask %llx\n", data, offset, mem_mask);
#endif
}

READ64_HANDLER( dc_gdrom_r )
{
	UINT32 off;

	if ((int)mem_mask & 1)
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

	if ((int)mem_mask & 1)
	{
		dat=(UINT32)(data >> 32);
		off=(offset << 1) | 1;
	}
	else
	{
		dat=(UINT32)data;
		off=offset << 1;
	}

	mame_printf_verbose("GDROM: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_g1_ctrl_r )
{
	return 0;
}

WRITE64_HANDLER( dc_g1_ctrl_w )
{
	mame_printf_verbose("G1CTRL: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_g2_ctrl_r )
{
	return 0;
}

WRITE64_HANDLER( dc_g2_ctrl_w )
{
	mame_printf_verbose("G2CTRL: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_modem_r )
{
	return 0;
}

WRITE64_HANDLER( dc_modem_w )
{
	mame_printf_verbose("MODEM: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_rtc_r )
{
	return 0;
}

WRITE64_HANDLER( dc_rtc_w )
{
UINT32 dat,off;
UINT32 old;

	if ((int)mem_mask & 1) {
		dat=(UINT32)(data >> 32);
		off=(offset << 1) | 1;
	} else {
		dat=(UINT32)data;
		off=offset << 1;
	}
	old = dc_rtcregister[off];
	dc_rtcregister[off] = dat & 0xFFFF; // 5f6c00+off*4=dat
/*  switch (off)
    {
    case RTC1:
        if (dc_rtcregister[RTC3])
            dc_rtcregister[RTC3] = 0;
        else
            dc_rtcregister[off] = old;
        break;
    case RTC2:
        if (dc_rtcregister[RTC3] == 0)
            dc_rtcregister[off] = old;
        break;
    case RTC3:
        dc_rtcregister[RTC3] &= 1;
        break;
    }*/
	mame_printf_verbose("RTC: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

/*static void dc_rtc_increment(void)
{
    dc_rtcregister[RTC2] = (dc_rtcregister[RTC2] + 1) & 0xFFFF;
    if (dc_rtcregister[RTC2] == 0)
        dc_rtcregister[RTC1] = (dc_rtcregister[RTC1] + 1) & 0xFFFF;
}*/

MACHINE_RESET( dc )
{
	/* halt the ARM7 */
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, ASSERT_LINE);

	memset(sysctrl_regs, 0, sizeof(sysctrl_regs));
	memset(maple_regs, 0, sizeof(maple_regs));
	memset(dc_rtcregister, 0, sizeof(dc_rtcregister));

	sysctrl_regs[SB_SBREV] = 0x0b;
}

READ64_HANDLER( dc_aica_reg_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

//  logerror("dc_aica_reg_r:  Unmapped read %08x\n", 0x700000+reg*4);
	return 0;
}

WRITE64_HANDLER( dc_aica_reg_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	if (reg == (0x2c00/4))
	{
		if (dat & 1)
		{
			/* halt the ARM7 */
			cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, ASSERT_LINE);
		}
		else
		{
			/* it's alive ! */
			cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, CLEAR_LINE);
		}
        }
	mame_printf_verbose("AICA REG: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}
