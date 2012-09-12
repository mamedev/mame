/*********************************************************************\
*
*   SGI IP22 Indigo2/Indy workstation
*
*   Todo: Fix tod clock set problem
*         Fix NVRAM saving
*         Fix SCSI DMA to handle chains properly
*         Probably many more things
*
*  Memory map:
*
*  18000000 - 1effffff      RESERVED - Unused
*  1f000000 - 1f3fffff      GIO - GFX
*  1f400000 - 1f5fffff      GIO - EXP0
*  1f600000 - 1f9fffff      GIO - EXP1 - Unused
*  1fa00000 - 1fa02047      Memory Controller
*  1fb00000 - 1fb1a7ff      HPC3 CHIP1
*  1fb80000 - 1fb9a7ff      HPC3 CHIP0
*  1fc00000 - 1fc7ffff      BIOS
*
*  References used:
*    MipsLinux: http://www.mips-linux.org/
*      linux-2.6.6/include/newport.h
*      linux-2.6.6/include/asm-mips/sgi/gio.h
*      linux-2.6.6/include/asm-mips/sgi/mc.h
*      linux-2.6.6/include/asm-mips/sgi/hpc3.h
*    NetBSD: http://www.netbsd.org/
*    gxemul: http://gavare.se/gxemul/
*
* Gentoo LiveCD r5 boot instructions:
*     mess -cdrom gentoor5.chd ip225015
*     enter the command interpreter and type "sashARCS".  press enter and
*     it'll autoboot.
*
* IRIX boot instructions:
*     mess -cdrom irix656inst1.chd ip225015
*     at the menu, choose either "run diagnostics" or "install system software"
*
\*********************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "sound/cdda.h"
#include "machine/sgi.h"
#include "machine/pckeybrd.h"
#include "machine/pc_lpt.h"
#include "includes/at.h"
#include "machine/8042kbdc.h"
#include "machine/pit8253.h"
#include "includes/ps2.h"
#include "machine/pcshare.h"
#include "video/newport.h"
#include "sound/dac.h"
#include "machine/nvram.h"
#include "machine/scsibus.h"
#include "machine/scsicd.h"
#include "machine/scsihd.h"
#include "machine/wd33c93.h"

typedef struct
{
	UINT8 nRegs[0x80];
	UINT8 nUserRAM[0x200];
	UINT8 nRAM[0x800];
} RTC_t;

typedef struct
{
	UINT32 nenetr_nbdp;
	UINT32 nenetr_cbp;
	UINT32 nunk0;
	UINT32 nunk1;
	UINT32 nIC_Unk0;
	UINT32 nSCSI0Descriptor;
	UINT32 nSCSI0DMACtrl;
} HPC3_t;

typedef struct
{
	UINT32 nIAR;
	UINT32 nIDR[4];
} HAL2_t;

typedef struct
{
	UINT8 nActive;
	UINT32 nCurPtr;
	UINT32 nDescPtr;
	UINT32 nNextPtr;
	UINT32 nWordsLeft;
} PBUS_DMA_t;

class ip22_state : public driver_device
{
public:
	ip22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_wd33c93(*this, "scsi:wd33c93"),
		m_unkpbus0(*this, "unkpbus0"),
		m_mainram(*this, "mainram") { }

	required_device<wd33c93_device> m_wd33c93;
	required_shared_ptr<UINT32> m_unkpbus0;
	required_shared_ptr<UINT32> m_mainram;
	RTC_t m_RTC;
	UINT32 m_int3_regs[64];
	UINT32 m_nIOC_ParReadCnt;
	/*UINT8 m_nIOC_ParCntl;*/
	HPC3_t m_HPC3;
	HAL2_t m_HAL2;
	PBUS_DMA_t m_PBUS_DMA;
	UINT32 m_nIntCounter;
	UINT8 m_dma_buffer[4096];
	DECLARE_READ32_MEMBER(hpc3_pbus6_r);
	DECLARE_WRITE32_MEMBER(hpc3_pbus6_w);
	DECLARE_READ32_MEMBER(hpc3_hd_enet_r);
	DECLARE_WRITE32_MEMBER(hpc3_hd_enet_w);
	DECLARE_READ32_MEMBER(hpc3_hd0_r);
	DECLARE_WRITE32_MEMBER(hpc3_hd0_w);
	DECLARE_READ32_MEMBER(hpc3_pbus4_r);
	DECLARE_WRITE32_MEMBER(hpc3_pbus4_w);
	DECLARE_READ32_MEMBER(rtc_r);
	DECLARE_WRITE32_MEMBER(rtc_w);
	DECLARE_WRITE32_MEMBER(ip22_write_ram);
	DECLARE_READ32_MEMBER(hal2_r);
	DECLARE_WRITE32_MEMBER(hal2_w);
	DECLARE_READ32_MEMBER(hpc3_pbusdma_r);
	DECLARE_WRITE32_MEMBER(hpc3_pbusdma_w);
	DECLARE_READ32_MEMBER(hpc3_unkpbus0_r);
	DECLARE_WRITE32_MEMBER(hpc3_unkpbus0_w);
	DECLARE_DRIVER_INIT(ip225015);
};


#define VERBOSE_LEVEL ( 0 )


INLINE void ATTR_PRINTF(3,4) verboselog(running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror("%08x: %s", machine.device("maincpu")->safe_pc(), buf);
	}
}



static const struct pit8253_config ip22_pit8254_config =
{
	{
		{
			1000000,				/* Timer 0: 1MHz */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			1000000,				/* Timer 1: 1MHz */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			1000000,				/* Timer 2: 1MHz */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};

#define RTC_DAY		state->m_RTC.nRAM[0x09]
#define RTC_HOUR	state->m_RTC.nRAM[0x08]
#define RTC_MINUTE	state->m_RTC.nRAM[0x07]
#define RTC_SECOND	state->m_RTC.nRAM[0x06]
#define RTC_HUNDREDTH	state->m_RTC.nRAM[0x05]

// interrupt sources handled by INT3
#define INT3_LOCAL0_FIFO	(0x01)
#define INT3_LOCAL0_SCSI0	(0x02)
#define INT3_LOCAL0_SCSI1	(0x04)
#define INT3_LOCAL0_ETHERNET	(0x08)
#define INT3_LOCAL0_MC_DMA	(0x10)
#define INT3_LOCAL0_PARALLEL	(0x20)
#define INT3_LOCAL0_GRAPHICS	(0x40)
#define INT3_LOCAL0_MAPPABLE0	(0x80)

#define INT3_LOCAL1_GP0		(0x01)
#define INT3_LOCAL1_PANEL	(0x02)
#define INT3_LOCAL1_GP2		(0x04)
#define INT3_LOCAL1_MAPPABLE1   (0x08)
#define INT3_LOCAL1_HPC_DMA 	(0x10)
#define INT3_LOCAL1_AC_FAIL     (0x20)
#define INT3_LOCAL1_VSYNC	(0x40)
#define INT3_LOCAL1_RETRACE	(0x80)

// raise a local0 interrupt
static void int3_raise_local0_irq(running_machine &machine, UINT8 source_mask)
{
	ip22_state *state = machine.driver_data<ip22_state>();
	// signal the interrupt is pending
	state->m_int3_regs[0] |= source_mask;

	// if it's not masked, also assert it now at the CPU
	if (state->m_int3_regs[1] & source_mask)
	{
		machine.device("maincpu")->execute().set_input_line(MIPS3_IRQ0, ASSERT_LINE);
	}
}

// lower a local0 interrupt
static void int3_lower_local0_irq(ip22_state *state, UINT8 source_mask)
{
	state->m_int3_regs[0] &= ~source_mask;
}

#ifdef UNUSED_FUNCTION
// raise a local1 interrupt
static void int3_raise_local1_irq(running_machine &machine, UINT8 source_mask)
{
	ip22_state *state = machine.driver_data<ip22_state>();
	// signal the interrupt is pending
	state->m_int3_regs[2] |= source_mask;

	// if it's not masked, also assert it now at the CPU
	if (state->m_int3_regs[2] & source_mask)
	{
		machine.device("maincpu")->execute().set_input_line(MIPS3_IRQ1, ASSERT_LINE);
	}
}

// lower a local1 interrupt
static void int3_lower_local1_irq(UINT8 source_mask)
{
	ip22_state *state = machine.driver_data<ip22_state>();
	state->m_int3_regs[2] &= ~source_mask;
}
#endif

READ32_MEMBER(ip22_state::hpc3_pbus6_r)
{
	device_t *lpt = machine().device("lpt_0");
	UINT8 ret8;
	switch( offset )
	{
	case 0x004/4:
		ret8 = pc_lpt_control_r(lpt, 0) ^ 0x0d;
		//verboselog(( machine, 0, "Parallel Control Read: %02x\n", ret8 );
		return ret8;
	case 0x008/4:
		ret8 = pc_lpt_status_r(lpt, 0) ^ 0x80;
		//verboselog(( machine, 0, "Parallel Status Read: %02x\n", ret8 );
		return ret8;
	case 0x030/4:
		//verboselog(( machine, 2, "Serial 1 Command Transfer Read, 0x1fbd9830: %02x\n", 0x04 );
		switch(space.device().safe_pc())
		{
			case 0x9fc1d9e4:	// interpreter (ip244415)
			case 0x9fc1d9e0:	// DRC (ip244415)
			case 0x9fc1f8e0:	// interpreter (ip224613)
			case 0x9fc1f8dc:	// DRC (ip224613)
			case 0x9fc204c8:	// interpreter (ip225015)
			case 0x9fc204c4:	// DRC (ip225015)
				return 0x00000005;
		}
		return 0x00000004;
	case 0x038/4:
		//verboselog(( machine, 2, "Serial 2 Command Transfer Read, 0x1fbd9838: %02x\n", 0x04 );
		return 0x00000004;
	case 0x40/4:
		return kbdc8042_8_r(&space, 0);
	case 0x44/4:
		return kbdc8042_8_r(&space, 4);
	case 0x58/4:
		return 0x20;	// chip rev 1, board rev 0, "Guinness" (Indy) => 0x01 for "Full House" (Indigo2)
	case 0x80/4:
	case 0x84/4:
	case 0x88/4:
	case 0x8c/4:
	case 0x90/4:
	case 0x94/4:
	case 0x98/4:
	case 0x9c/4:
	case 0xa0/4:
	case 0xa4/4:
	case 0xa8/4:
	case 0xac/4:
//      mame_printf_info("INT3: r @ %x mask %08x (PC=%x)\n", offset*4, mem_mask, activecpu_get_pc());
		return m_int3_regs[offset-0x80/4];
	case 0xb0/4:
		ret8 = pit8253_r(machine().device("pit8254"), 0);
		//verboselog(( machine, 0, "HPC PBUS6 IOC4 Timer Counter 0 Register Read: 0x%02x (%08x)\n", ret8, mem_mask );
		return ret8;
	case 0xb4/4:
		ret8 = pit8253_r(machine().device("pit8254"), 1);
		//verboselog(( machine, 0, "HPC PBUS6 IOC4 Timer Counter 1 Register Read: 0x%02x (%08x)\n", ret8, mem_mask );
		return ret8;
	case 0xb8/4:
		ret8 = pit8253_r(machine().device("pit8254"), 2);
		//verboselog(( machine, 0, "HPC PBUS6 IOC4 Timer Counter 2 Register Read: 0x%02x (%08x)\n", ret8, mem_mask );
		return ret8;
	case 0xbc/4:
		ret8 = pit8253_r(machine().device("pit8254"), 3);
		//verboselog(( machine, 0, "HPC PBUS6 IOC4 Timer Control Word Register Read: 0x%02x (%08x)\n", ret8, mem_mask );
		return ret8;
	default:
		//verboselog(( machine, 0, "Unknown HPC PBUS6 Read: 0x%08x (%08x)\n", 0x1fbd9800 + ( offset << 2 ), mem_mask );
		return 0;
	}
	return 0;
}

WRITE32_MEMBER(ip22_state::hpc3_pbus6_w)
{
	device_t *lpt = machine().device("lpt_0");
	char cChar;

	switch( offset )
	{
	case 0x004/4:
		//verboselog(( machine, 0, "Parallel Control Write: %08x\n", data );
		pc_lpt_control_w(lpt, 0, data ^ 0x0d);
		//m_nIOC_ParCntl = data;
		break;
	case 0x030/4:
		if( ( data & 0x000000ff ) >= 0x20 )
		{
			//verboselog(( machine, 2, "Serial 1 Command Transfer Write: %02x: %c\n", data & 0x000000ff, data & 0x000000ff );
		}
		else
		{
			//verboselog(( machine, 2, "Serial 1 Command Transfer Write: %02x\n", data & 0x000000ff );
		}
		cChar = data & 0x000000ff;
		if( cChar >= 0x20 || cChar == 0x0d || cChar == 0x0a )
		{
//          mame_printf_info( "%c", cChar );
		}
		break;
	case 0x034/4:
		if( ( data & 0x000000ff ) >= 0x20 )
		{
			//verboselog(( machine, 2, "Serial 1 Data Transfer Write: %02x: %c\n", data & 0x000000ff, data & 0x000000ff );
		}
		else
		{
			//verboselog(( machine, 2, "Serial 1 Data Transfer Write: %02x\n", data & 0x000000ff );
		}
		cChar = data & 0x000000ff;
		if( cChar >= 0x20 || cChar == 0x0d || cChar == 0x0a )
		{
//          mame_printf_info( "%c", cChar );
		}
		break;
	case 0x40/4:
		kbdc8042_8_w(&space, 0, data);
		break;
	case 0x44/4:
		kbdc8042_8_w(&space, 4, data);
		break;
	case 0x80/4:
	case 0x84/4:
	case 0x88/4:
	case 0x8c/4:
	case 0x90/4:
	case 0x94/4:
	case 0x98/4:
	case 0x9c/4:
	case 0xa0/4:
	case 0xa4/4:
//      mame_printf_info("INT3: w %x to %x (reg %d) mask %08x (PC=%x)\n", data, offset*4, offset-0x80/4, mem_mask, activecpu_get_pc());
		m_int3_regs[offset-0x80/4] = data;

		// if no local0 interrupts now, clear the input to the CPU
		if ((m_int3_regs[0] & m_int3_regs[1]) == 0)
		{
			machine().device("maincpu")->execute().set_input_line(MIPS3_IRQ0, CLEAR_LINE);
		}
		else
		{
			machine().device("maincpu")->execute().set_input_line(MIPS3_IRQ0, ASSERT_LINE);
		}

		// if no local1 interrupts now, clear the input to the CPU
		if ((m_int3_regs[2] & m_int3_regs[3]) == 0)
		{
			machine().device("maincpu")->execute().set_input_line(MIPS3_IRQ1, CLEAR_LINE);
		}
		else
		{
			machine().device("maincpu")->execute().set_input_line(MIPS3_IRQ1, ASSERT_LINE);
		}
		break;
	case 0xb0/4:
		//verboselog(( machine, 0, "HPC PBUS6 IOC4 Timer Counter 0 Register Write: 0x%08x (%08x)\n", data, mem_mask );
		pit8253_w(machine().device("pit8254"), 0, data & 0x000000ff);
		return;
	case 0xb4/4:
		//verboselog(( machine, 0, "HPC PBUS6 IOC4 Timer Counter 1 Register Write: 0x%08x (%08x)\n", data, mem_mask );
		pit8253_w(machine().device("pit8254"), 1, data & 0x000000ff);
		return;
	case 0xb8/4:
		//verboselog(( machine, 0, "HPC PBUS6 IOC4 Timer Counter 2 Register Write: 0x%08x (%08x)\n", data, mem_mask );
		pit8253_w(machine().device("pit8254"), 2, data & 0x000000ff);
		return;
	case 0xbc/4:
		//verboselog(( machine, 0, "HPC PBUS6 IOC4 Timer Control Word Register Write: 0x%08x (%08x)\n", data, mem_mask );
		pit8253_w(machine().device("pit8254"), 3, data & 0x000000ff);
		return;
	default:
		//verboselog(( machine, 0, "Unknown HPC PBUS6 Write: 0x%08x: 0x%08x (%08x)\n", 0x1fbd9800 + ( offset << 2 ), data, mem_mask );
		break;
	}
}

//UINT32 nHPC3_hd0_register;
//UINT32 nHPC3_hd0_regs[0x20];
//UINT32 nHPC3_hd1_regs[0x20];

READ32_MEMBER(ip22_state::hpc3_hd_enet_r)
{
	//running_machine &machine = machine();

	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 0, "HPC3 SCSI0DESC Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_HPC3.nSCSI0Descriptor );
		return m_HPC3.nSCSI0Descriptor;
	case 0x1004/4:
		//verboselog((machine, 0, "HPC3 SCSI0DMACTRL Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_HPC3.nSCSI0DMACtrl );
		return m_HPC3.nSCSI0DMACtrl;
	case 0x4000/4:
		//verboselog((machine, 2, "HPC3 ENETR CBP Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_HPC3.nenetr_nbdp );
		return m_HPC3.nenetr_cbp;
	case 0x4004/4:
		//verboselog((machine, 2, "HPC3 ENETR NBDP Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_HPC3.nenetr_nbdp );
		return m_HPC3.nenetr_nbdp;
	default:
		//verboselog((machine, 0, "Unknown HPC3 ENET/HDx Read: %08x (%08x)\n", 0x1fb90000 + ( offset << 2 ), mem_mask );
		return 0;
	}
	return 0;
}

WRITE32_MEMBER(ip22_state::hpc3_hd_enet_w)
{
	//running_machine &machine = machine();

	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 SCSI0DESC Write: %08x\n", data );
		m_HPC3.nSCSI0Descriptor = data;
		break;
	case 0x1004/4:
		//verboselog((machine, 2, "HPC3 SCSI0DMACTRL Write: %08x\n", data );
		m_HPC3.nSCSI0DMACtrl = data;
		break;
	case 0x4000/4:
		//verboselog((machine, 2, "HPC3 ENETR CBP Write: %08x\n", data );
		m_HPC3.nenetr_cbp = data;
		break;
	case 0x4004/4:
		//verboselog((machine, 2, "HPC3 ENETR NBDP Write: %08x\n", data );
		m_HPC3.nenetr_nbdp = data;
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 ENET/HDx write: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}

READ32_MEMBER(ip22_state::hpc3_hd0_r)
{
	//running_machine &machine = machine();

	switch( offset )
	{
	case 0x0000/4:
	case 0x4000/4:
//      //verboselog((machine, 2, "HPC3 HD0 Status Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, nHPC3_hd0_regs[0x17] );
		if (ACCESSING_BITS_0_7)
		{
			return m_wd33c93->read( space, 0 );
		}
		else
		{
			return 0;
		}
	case 0x0004/4:
	case 0x4004/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, nHPC3_hd0_regs[nHPC3_hd0_register] );
		if (ACCESSING_BITS_0_7)
		{
			return m_wd33c93->read( space, 1 );
		}
		else
		{
			return 0;
		}
	default:
		//verboselog((machine, 0, "Unknown HPC3 HD0 Read: %08x (%08x) [%x] PC=%x\n", 0x1fbc0000 + ( offset << 2 ), mem_mask, offset, space.device().safe_pc() );
		return 0;
	}
	return 0;
}

WRITE32_MEMBER(ip22_state::hpc3_hd0_w)
{
	//running_machine &machine = machine();

	switch( offset )
	{
	case 0x0000/4:
	case 0x4000/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register Select Write: %08x\n", data );
		if (ACCESSING_BITS_0_7)
		{
			m_wd33c93->write( space, 0, data & 0x000000ff );
		}
		break;
	case 0x0004/4:
	case 0x4004/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register %d Write: %08x\n", nHPC3_hd0_register, data );
		if (ACCESSING_BITS_0_7)
		{
			m_wd33c93->write( space, 1,  data & 0x000000ff );
		}
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 HD0 Write: %08x (%08x): %08x\n", 0x1fbc0000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}


READ32_MEMBER(ip22_state::hpc3_pbus4_r)
{
	//running_machine &machine = machine();

	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 0 Read: (%08x): %08x\n", mem_mask, m_HPC3.nunk0 );
		return m_HPC3.nunk0;
	case 0x000c/4:
		//verboselog((machine, 2, "Interrupt Controller(?) Read: (%08x): %08x\n", mem_mask, m_HPC3.nIC_Unk0 );
		return m_HPC3.nIC_Unk0;
	case 0x0014/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 1 Read: (%08x): %08x\n", mem_mask, m_HPC3.nunk1 );
		return m_HPC3.nunk1;
	default:
		//verboselog((machine, 0, "Unknown HPC3 PBUS4 Read: %08x (%08x)\n", 0x1fbd9000 + ( offset << 2 ), mem_mask );
		return 0;
	}
	return 0;
}

WRITE32_MEMBER(ip22_state::hpc3_pbus4_w)
{
	//running_machine &machine = machine();

	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 0 Write: %08x (%08x)\n", data, mem_mask );
		m_HPC3.nunk0 = data;
		break;
	case 0x000c/4:
		//verboselog((machine, 2, "Interrupt Controller(?) Write: (%08x): %08x\n", mem_mask, data );
		m_HPC3.nIC_Unk0 = data;
		break;
	case 0x0014/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 1 Write: %08x (%08x)\n", data, mem_mask );
		m_HPC3.nunk1 = data;
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 PBUS4 Write: %08x (%08x): %08x\n", 0x1fbd9000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}

#define RTC_SECONDS	state->m_RTC.nRegs[0x00]
#define RTC_SECONDS_A	state->m_RTC.nRegs[0x01]
#define RTC_MINUTES	state->m_RTC.nRegs[0x02]
#define RTC_MINUTES_A	state->m_RTC.nRegs[0x03]
#define RTC_HOURS	state->m_RTC.nRegs[0x04]
#define RTC_HOURS_A	state->m_RTC.nRegs[0x05]
#define RTC_DAYOFWEEK	state->m_RTC.nRegs[0x06]
#define RTC_DAYOFMONTH	state->m_RTC.nRegs[0x07]
#define RTC_MONTH	state->m_RTC.nRegs[0x08]
#define RTC_YEAR	state->m_RTC.nRegs[0x09]
#define RTC_REGISTERA	state->m_RTC.nRegs[0x0a]
#define RTC_REGISTERB	state->m_RTC.nRegs[0x0b]
#define RTC_REGISTERC	state->m_RTC.nRegs[0x0c]
#define RTC_REGISTERD	state->m_RTC.nRegs[0x0d]
#define RTC_MODELBYTE	state->m_RTC.nRegs[0x40]
#define RTC_SERBYTE0	state->m_RTC.nRegs[0x41]
#define RTC_SERBYTE1	state->m_RTC.nRegs[0x42]
#define RTC_SERBYTE2	state->m_RTC.nRegs[0x43]
#define RTC_SERBYTE3	state->m_RTC.nRegs[0x44]
#define RTC_SERBYTE4	state->m_RTC.nRegs[0x45]
#define RTC_SERBYTE5	state->m_RTC.nRegs[0x46]
#define RTC_CRC		state->m_RTC.nRegs[0x47]
#define RTC_CENTURY	state->m_RTC.nRegs[0x48]
#define RTC_DAYOFMONTH_A state->m_RTC.nRegs[0x49]
#define RTC_EXTCTRL0	state->m_RTC.nRegs[0x4a]
#define RTC_EXTCTRL1	state->m_RTC.nRegs[0x4b]
#define RTC_RTCADDR2	state->m_RTC.nRegs[0x4e]
#define RTC_RTCADDR3	state->m_RTC.nRegs[0x4f]
#define RTC_RAMLSB	state->m_RTC.nRegs[0x50]
#define RTC_RAMMSB	state->m_RTC.nRegs[0x51]
#define RTC_WRITECNT	state->m_RTC.nRegs[0x5e]

READ32_MEMBER(ip22_state::rtc_r)
{
	ip22_state *state = machine().driver_data<ip22_state>();

//  mame_printf_info("RTC_R: offset %x = %x (PC=%x)\n", offset, m_RTC.nRegs[offset], activecpu_get_pc());

	if( offset <= 0x0d )
	{
		switch( offset )
		{
		case 0x0000:
//          //verboselog((machine, 2, "RTC Seconds Read: %d \n", RTC_SECONDS );
			return RTC_SECONDS;
		case 0x0001:
//          //verboselog((machine, 2, "RTC Seconds Alarm Read: %d \n", RTC_SECONDS_A );
			return RTC_SECONDS_A;
		case 0x0002:
			//verboselog((machine, 3, "RTC Minutes Read: %d \n", RTC_MINUTES );
			return RTC_MINUTES;
		case 0x0003:
			//verboselog((machine, 3, "RTC Minutes Alarm Read: %d \n", RTC_MINUTES_A );
			return RTC_MINUTES_A;
		case 0x0004:
			//verboselog((machine, 3, "RTC Hours Read: %d \n", RTC_HOURS );
			return RTC_HOURS;
		case 0x0005:
			//verboselog((machine, 3, "RTC Hours Alarm Read: %d \n", RTC_HOURS_A );
			return RTC_HOURS_A;
		case 0x0006:
			//verboselog((machine, 3, "RTC Day of Week Read: %d \n", RTC_DAYOFWEEK );
			return RTC_DAYOFWEEK;
		case 0x0007:
			//verboselog((machine, 3, "RTC Day of Month Read: %d \n", RTC_DAYOFMONTH );
			return RTC_DAYOFMONTH;
		case 0x0008:
			//verboselog((machine, 3, "RTC Month Read: %d \n", RTC_MONTH );
			return RTC_MONTH;
		case 0x0009:
			//verboselog((machine, 3, "RTC Year Read: %d \n", RTC_YEAR );
			return RTC_YEAR;
		case 0x000a:
			//verboselog((machine, 3, "RTC Register A Read: %02x \n", RTC_REGISTERA );
			return RTC_REGISTERA;
		case 0x000b:
			//verboselog((machine, 3, "RTC Register B Read: %02x \n", RTC_REGISTERB );
			return RTC_REGISTERB;
		case 0x000c:
			//verboselog((machine, 3, "RTC Register C Read: %02x \n", RTC_REGISTERC );
			return RTC_REGISTERC;
		case 0x000d:
			//verboselog((machine, 3, "RTC Register D Read: %02x \n", RTC_REGISTERD );
			return RTC_REGISTERD;
		default:
			//verboselog((machine, 3, "Unknown RTC Read: %08x (%08x)\n", 0x1fbe0000 + ( offset << 2 ), mem_mask );
			return 0;
		}
	}
	if( offset >= 0x0e && offset < 0x40 )
	{
		return m_RTC.nRegs[offset];
	}
	if( offset >= 0x40 && offset < 0x80 && !( RTC_REGISTERA & 0x10 ) )
	{
		return m_RTC.nUserRAM[offset - 0x40];
	}
	if( offset >= 0x40 && offset < 0x80 && ( RTC_REGISTERA & 0x10 ) )
	{
		switch( offset )
		{
		case 0x0040:
			//verboselog((machine, 3, "RTC Model Byte Read: %02x\n", RTC_MODELBYTE );
			return RTC_MODELBYTE;
		case 0x0041:
			//verboselog((machine, 3, "RTC Serial Byte 0 Read: %02x\n", RTC_SERBYTE0 );
			return RTC_SERBYTE0;
		case 0x0042:
			//verboselog((machine, 3, "RTC Serial Byte 1 Read: %02x\n", RTC_SERBYTE1 );
			return RTC_SERBYTE1;
		case 0x0043:
			//verboselog((machine, 3, "RTC Serial Byte 2 Read: %02x\n", RTC_SERBYTE2 );
			return RTC_SERBYTE2;
		case 0x0044:
			//verboselog((machine, 3, "RTC Serial Byte 3 Read: %02x\n", RTC_SERBYTE3 );
			return RTC_SERBYTE3;
		case 0x0045:
			//verboselog((machine, 3, "RTC Serial Byte 4 Read: %02x\n", RTC_SERBYTE4 );
			return RTC_SERBYTE4;
		case 0x0046:
			//verboselog((machine, 3, "RTC Serial Byte 5 Read: %02x\n", RTC_SERBYTE5 );
			return RTC_SERBYTE5;
		case 0x0047:
			//verboselog((machine, 3, "RTC CRC Read: %02x\n", RTC_CRC );
			return RTC_CRC;
		case 0x0048:
			//verboselog((machine, 3, "RTC Century Read: %02x\n", RTC_CENTURY );
			return RTC_CENTURY;
		case 0x0049:
			//verboselog((machine, 3, "RTC Day of Month Alarm Read: %02x \n", RTC_DAYOFMONTH_A );
			return RTC_DAYOFMONTH_A;
		case 0x004a:
			//verboselog((machine, 3, "RTC Extended Control 0 Read: %02x \n", RTC_EXTCTRL0 );
			return RTC_EXTCTRL0;
		case 0x004b:
			//verboselog((machine, 3, "RTC Extended Control 1 Read: %02x \n", RTC_EXTCTRL1 );
			return RTC_EXTCTRL1;
		case 0x004e:
			//verboselog((machine, 3, "RTC SMI Recovery Address 2 Read: %02x \n", RTC_RTCADDR2 );
			return RTC_RTCADDR2;
		case 0x004f:
			//verboselog((machine, 3, "RTC SMI Recovery Address 3 Read: %02x \n", RTC_RTCADDR3 );
			return RTC_RTCADDR3;
		case 0x0050:
			//verboselog((machine, 3, "RTC RAM LSB Read: %02x \n", RTC_RAMLSB );
			return RTC_RAMLSB;
		case 0x0051:
			//verboselog((machine, 3, "RTC RAM MSB Read: %02x \n", RTC_RAMMSB );
			return RTC_RAMMSB;
		case 0x0053:
			return m_RTC.nRAM[ ( RTC_RAMMSB << 8 ) | RTC_RAMLSB ];
		case 0x005e:
			return RTC_WRITECNT;
		default:
			//verboselog((machine, 3, "Unknown RTC Ext. Reg. Read: %02x\n", offset );
			return 0;
		}
	}
	if( offset >= 0x80 )
	{
		return m_RTC.nUserRAM[ offset - 0x80 ];
	}
	return 0;
}

WRITE32_MEMBER(ip22_state::rtc_w)
{
	//running_machine &machine = machine();
	ip22_state *state = machine().driver_data<ip22_state>();
	RTC_WRITECNT++;

//  mame_printf_info("RTC_W: offset %x => %x (PC=%x)\n", data, offset, activecpu_get_pc());

	if( offset <= 0x0d )
	{
		switch( offset )
		{
		case 0x0000:
			//verboselog((machine, 3, "RTC Seconds Write: %02x \n", data );
			RTC_SECONDS = data;
			break;
		case 0x0001:
			//verboselog((machine, 3, "RTC Seconds Alarm Write: %02x \n", data );
			RTC_SECONDS_A = data;
			break;
		case 0x0002:
			//verboselog((machine, 3, "RTC Minutes Write: %02x \n", data );
			RTC_MINUTES = data;
			break;
		case 0x0003:
			//verboselog((machine, 3, "RTC Minutes Alarm Write: %02x \n", data );
			RTC_MINUTES_A = data;
			break;
		case 0x0004:
			//verboselog((machine, 3, "RTC Hours Write: %02x \n", data );
			RTC_HOURS = data;
			break;
		case 0x0005:
			//verboselog((machine, 3, "RTC Hours Alarm Write: %02x \n", data );
			RTC_HOURS_A = data;
			break;
		case 0x0006:
			//verboselog((machine, 3, "RTC Day of Week Write: %02x \n", data );
			RTC_DAYOFWEEK = data;
			break;
		case 0x0007:
			//verboselog((machine, 3, "RTC Day of Month Write: %02x \n", data );
			RTC_DAYOFMONTH = data;
			break;
		case 0x0008:
			//verboselog((machine, 3, "RTC Month Write: %02x \n", data );
			RTC_MONTH = data;
			break;
		case 0x0009:
			//verboselog((machine, 3, "RTC Year Write: %02x \n", data );
			RTC_YEAR = data;
			break;
		case 0x000a:
			//verboselog((machine, 3, "RTC Register A Write (Bit 7 Ignored): %02x \n", data );
			RTC_REGISTERA = data & 0x0000007f;
			break;
		case 0x000b:
			//verboselog((machine, 3, "RTC Register B Write: %02x \n", data );
			RTC_REGISTERB = data;
			break;
		case 0x000c:
			//verboselog((machine, 3, "RTC Register C Write (Ignored): %02x \n", data );
			break;
		case 0x000d:
			//verboselog((machine, 3, "RTC Register D Write (Ignored): %02x \n", data );
			break;
		default:
			//verboselog((machine, 3, "Unknown RTC Write: %08x (%08x): %08x\n", 0x1fbe0000 + ( offset << 2 ), mem_mask, data );
			break;
		}
	}
	if( offset >= 0x0e && offset < 0x40 )
	{
		m_RTC.nRegs[offset] = data;
		return;
	}
	if( offset >= 0x40 && offset < 0x80 && !( RTC_REGISTERA & 0x10 ) )
	{
		m_RTC.nUserRAM[offset - 0x40] = data;
		return;
	}
	if( offset >= 0x40 && offset < 0x80 && ( RTC_REGISTERA & 0x10 ) )
	{
		switch( offset )
		{
		case 0x0040:
		case 0x0041:
		case 0x0042:
		case 0x0043:
		case 0x0044:
		case 0x0045:
		case 0x0046:
			//verboselog((machine, 3, "Invalid write to RTC serial number byte %d: %02x\n", offset - 0x0040, data );
			break;
		case 0x0047:
			//verboselog((machine, 3, "RTC Century Write: %02x \n", data );
			RTC_CENTURY = data;
			break;
		case 0x0048:
			//verboselog((machine, 3, "RTC Century Write: %02x \n", data );
			RTC_CENTURY = data;
			break;
		case 0x0049:
			//verboselog((machine, 3, "RTC Day of Month Alarm Write: %02x \n", data );
			RTC_DAYOFMONTH_A = data;
			break;
		case 0x004a:
			//verboselog((machine, 3, "RTC Extended Control 0 Write: %02x \n", data );
			RTC_EXTCTRL0 = data;
			break;
		case 0x004b:
			//verboselog((machine, 3, "RTC Extended Control 1 Write: %02x \n", data );
			RTC_EXTCTRL1 = data;
			break;
		case 0x004e:
			//verboselog((machine, 3, "RTC SMI Recovery Address 2 Write: %02x \n", data );
			RTC_RTCADDR2 = data;
			break;
		case 0x004f:
			//verboselog((machine, 3, "RTC SMI Recovery Address 3 Write: %02x \n", data );
			RTC_RTCADDR3 = data;
			break;
		case 0x0050:
			//verboselog((machine, 3, "RTC RAM LSB Write: %02x \n", data );
			RTC_RAMLSB = data;
			break;
		case 0x0051:
			//verboselog((machine, 3, "RTC RAM MSB Write: %02x \n", data );
			RTC_RAMMSB = data;
			break;
		case 0x0053:
			m_RTC.nRAM[ ( RTC_RAMMSB << 8 ) | RTC_RAMLSB ] = data;
			break;
		default:
			//verboselog((machine, 3, "Unknown RTC Ext. Reg. Write: %02x: %02x\n", offset, data );
			break;
		}
	}
	if( offset >= 0x80 )
	{
		m_RTC.nUserRAM[ offset - 0x80 ] = data;
	}
}

// a bit hackish, but makes the memory detection work properly and allows a big cleanup of the mapping
WRITE32_MEMBER(ip22_state::ip22_write_ram)
{
	// if banks 2 or 3 are enabled, do nothing, we don't support that much memory
	if (sgi_mc_r(&space, 0xc8/4, 0xffffffff) & 0x10001000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}

	// if banks 0 or 1 have 2 membanks, also kill it, we only want 128 MB
	if (sgi_mc_r(&space, 0xc0/4, 0xffffffff) & 0x40004000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}
	COMBINE_DATA(&m_mainram[offset]);
}


#define H2_IAR_TYPE			0xf000
#define H2_IAR_NUM			0x0f00
#define H2_IAR_ACCESS_SEL	0x0080
#define H2_IAR_PARAM		0x000c
#define H2_IAR_RB_INDEX		0x0003

#define H2_ISR_TSTATUS		0x01
#define H2_ISR_USTATUS		0x02
#define H2_ISR_QUAD_MODE	0x04
#define H2_ISR_GLOBAL_RESET	0x08
#define H2_ISR_CODEC_RESET	0x10

READ32_MEMBER(ip22_state::hal2_r)
{
	//running_machine &machine = machine();

	switch( offset )
	{
	case 0x0010/4:
		//verboselog((machine, 0, "HAL2 Status read: 0x0004\n" );
		return 0x0004;
	case 0x0020/4:
		//verboselog((machine, 0, "HAL2 Revision read: 0x4011\n" );
		return 0x4011;
	}
	//verboselog((machine, 0, "Unknown HAL2 read: 0x%08x (%08x)\n", 0x1fbd8000 + offset*4, mem_mask );
	return 0;
}

WRITE32_MEMBER(ip22_state::hal2_w)
{
	//running_machine &machine = machine();

	switch( offset )
	{
	case 0x0010/4:
		//verboselog((machine, 0, "HAL2 Status Write: 0x%08x (%08x)\n", data, mem_mask );
		if( data & H2_ISR_GLOBAL_RESET )
		{
			//verboselog((machine, 0, "    HAL2 Global Reset\n" );
		}
		if( data & H2_ISR_CODEC_RESET )
		{
			//verboselog((machine, 0, "    HAL2 Codec Reset\n" );
		}
		break;
	case 0x0030/4:
		//verboselog((machine, 0, "HAL2 Indirect Address Register Write: 0x%08x (%08x)\n", data, mem_mask );
		m_HAL2.nIAR = data;
		switch( data & H2_IAR_TYPE )
		{
		case 0x1000:
			//verboselog((machine, 0, "    DMA Port\n" );
			switch( data & H2_IAR_NUM )
			{
			case 0x0100:
				//verboselog((machine, 0, "        Synth In\n" );
				break;
			case 0x0200:
				//verboselog((machine, 0, "        AES In\n" );
				break;
			case 0x0300:
				//verboselog((machine, 0, "        AES Out\n" );
				break;
			case 0x0400:
				//verboselog((machine, 0, "        DAC Out\n" );
				break;
			case 0x0500:
				//verboselog((machine, 0, "        ADC Out\n" );
				break;
			case 0x0600:
				//verboselog((machine, 0, "        Synth Control\n" );
				break;
			}
			break;
		case 0x2000:
			//verboselog((machine, 0, "    Bresenham\n" );
			switch( data & H2_IAR_NUM )
			{
			case 0x0100:
				//verboselog((machine, 0, "        Bresenham Clock Gen 1\n" );
				break;
			case 0x0200:
				//verboselog((machine, 0, "        Bresenham Clock Gen 2\n" );
				break;
			case 0x0300:
				//verboselog((machine, 0, "        Bresenham Clock Gen 3\n" );
				break;
			}
			break;
		case 0x3000:
			//verboselog((machine, 0, "    Unix Timer\n" );
			switch( data & H2_IAR_NUM )
			{
			case 0x0100:
				//verboselog((machine, 0, "        Unix Timer\n" );
				break;
			}
			break;
		case 0x9000:
			//verboselog((machine, 0, "    Global DMA Control\n" );
			switch( data & H2_IAR_NUM )
			{
			case 0x0100:
				//verboselog((machine, 0, "        DMA Control\n" );
				break;
			}
			break;
		}
		switch( data & H2_IAR_ACCESS_SEL )
		{
		case 0x0000:
			//verboselog((machine, 0, "    Write\n" );
			break;
		case 0x0080:
			//verboselog((machine, 0, "    Read\n" );
			break;
		}
		//verboselog((machine, 0, "    Parameter: %01x\n", ( data & H2_IAR_PARAM ) >> 2 );
		return;
		/* FIXME: this code is never excuted */
		//verboselog((machine, 0, "    Read Back Index: %01x\n", ( data & H2_IAR_RB_INDEX ) );
		break;
	case 0x0040/4:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 0 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_HAL2.nIDR[0] = data;
		return;
	case 0x0050/4:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 1 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_HAL2.nIDR[1] = data;
		return;
	case 0x0060/4:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 2 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_HAL2.nIDR[2] = data;
		return;
	case 0x0070/4:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 3 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_HAL2.nIDR[3] = data;
		return;
	}
	//verboselog((machine, 0, "Unknown HAL2 write: 0x%08x: 0x%08x (%08x)\n", 0x1fbd8000 + offset*4, data, mem_mask );
}

#define PBUS_CTRL_ENDIAN		0x00000002
#define PBUS_CTRL_RECV			0x00000004
#define PBUS_CTRL_FLUSH			0x00000008
#define PBUS_CTRL_DMASTART		0x00000010
#define PBUS_CTRL_LOAD_EN		0x00000020
#define PBUS_CTRL_REALTIME		0x00000040
#define PBUS_CTRL_HIGHWATER		0x0000ff00
#define PBUS_CTRL_FIFO_BEG		0x003f0000
#define PBUS_CTRL_FIFO_END		0x3f000000

#define PBUS_DMADESC_EOX		0x80000000
#define PBUS_DMADESC_EOXP		0x40000000
#define PBUS_DMADESC_XIE		0x20000000
#define PBUS_DMADESC_IPG		0x00ff0000
#define PBUS_DMADESC_TXD		0x00008000
#define PBUS_DMADESC_BC			0x00003fff


static TIMER_CALLBACK(ip22_dma)
{
	//ip22_state *state = machine.driver_data<ip22_state>();
	machine.scheduler().timer_set(attotime::never, FUNC(ip22_dma));
#if 0
	if( state->m_PBUS_DMA.nActive )
	{
		UINT16 temp16 = ( state->m_mainram[(state->m_PBUS_DMA.nCurPtr - 0x08000000)/4] & 0xffff0000 ) >> 16;
		INT16 stemp16 = (INT16)((temp16 >> 8) | (temp16 << 8));

		machine.device<dac_device>("dac")->write_signed16(stemp16 ^ 0x8000);

		state->m_PBUS_DMA.nCurPtr += 4;

		state->m_PBUS_DMA.nWordsLeft -= 4;
		if( state->m_PBUS_DMA.nWordsLeft == 0 )
		{
			if( state->m_PBUS_DMA.nNextPtr != 0 )
			{
				state->m_PBUS_DMA.nDescPtr = state->m_PBUS_DMA.nNextPtr;
				state->m_PBUS_DMA.nCurPtr = state->m_mainram[(state->m_PBUS_DMA.nDescPtr - 0x08000000)/4];
				state->m_PBUS_DMA.nWordsLeft = state->m_mainram[(state->m_PBUS_DMA.nDescPtr - 0x08000000)/4+1];
				state->m_PBUS_DMA.nNextPtr = state->m_mainram[(state->m_PBUS_DMA.nDescPtr - 0x08000000)/4+2];
			}
			else
			{
				state->m_PBUS_DMA.nActive = 0;
				return;
			}
		}
		machine.scheduler().timer_set(attotime::from_hz(44100), FUNC(ip22_dma));
	}
#endif
}

READ32_MEMBER(ip22_state::hpc3_pbusdma_r)
{
	//UINT32 channel = offset / (0x2000/4);
	//verboselog((machine(), 0, "PBUS DMA Channel %d Read: 0x%08x (%08x)\n", channel, 0x1fb80000 + offset*4, mem_mask );
	return 0;
}

WRITE32_MEMBER(ip22_state::hpc3_pbusdma_w)
{
	UINT32 channel = offset / (0x2000/4);

	switch( offset & 0x07ff )
	{
	case 0x0000/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Buffer Pointer Write: 0x%08x\n", channel, data );
		return;
	case 0x0004/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Descriptor Pointer Write: 0x%08x\n", channel, data );
		if( channel == 1 )
		{
			m_PBUS_DMA.nDescPtr = data;
			m_PBUS_DMA.nCurPtr = m_mainram[(m_PBUS_DMA.nDescPtr - 0x08000000)/4];
			m_PBUS_DMA.nWordsLeft = m_mainram[(m_PBUS_DMA.nDescPtr - 0x08000000)/4+1];
			m_PBUS_DMA.nNextPtr = m_mainram[(m_PBUS_DMA.nDescPtr - 0x08000000)/4+2];
			//verboselog((machine, 0, "nPBUS_DMA_DescPtr = %08x\n", m_PBUS_DMA.nDescPtr );
			//verboselog((machine, 0, "nPBUS_DMA_CurPtr = %08x\n", m_PBUS_DMA.nCurPtr );
			//verboselog((machine, 0, "nPBUS_DMA_WordsLeft = %08x\n", m_PBUS_DMA.nWordsLeft );
			//verboselog((machine, 0, "nPBUS_DMA_NextPtr = %08x\n", m_PBUS_DMA.nNextPtr );
		}
		return;
	case 0x1000/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Control Register Write: 0x%08x\n", channel, data );
		if( data & PBUS_CTRL_ENDIAN )
		{
			//verboselog((machine, 0, "    Little Endian\n" );
		}
		else
		{
			//verboselog((machine, 0, "    Big Endian\n" );
		}
		if( data & PBUS_CTRL_RECV )
		{
			//verboselog((machine, 0, "    RX DMA\n" );
		}
		else
		{
			//verboselog((machine, 0, "    TX DMA\n" );
		}
		if( data & PBUS_CTRL_FLUSH )
		{
			//verboselog((machine, 0, "    Flush for RX\n" );
		}
		if( data & PBUS_CTRL_DMASTART )
		{
			//verboselog((machine, 0, "    Start DMA\n" );
		}
		if( data & PBUS_CTRL_LOAD_EN )
		{
			//verboselog((machine, 0, "    Load Enable\n" );
		}
		//verboselog((machine, 0, "    High Water Mark: %04x bytes\n", ( data & PBUS_CTRL_HIGHWATER ) >> 8 );
		//verboselog((machine, 0, "    FIFO Begin: Row %04x\n", ( data & PBUS_CTRL_FIFO_BEG ) >> 16 );
		//verboselog((machine, 0, "    FIFO End: Rowe %04x\n", ( data & PBUS_CTRL_FIFO_END ) >> 24 );
		if( ( data & PBUS_CTRL_DMASTART ) || ( data & PBUS_CTRL_LOAD_EN ) )
		{
			machine().scheduler().timer_set(attotime::from_hz(44100), FUNC(ip22_dma));
			m_PBUS_DMA.nActive = 1;
		}
		return;
	}
	//verboselog((machine, 0, "Unknown PBUS DMA Channel %d Write: 0x%08x: 0x%08x (%08x)\n", channel, 0x1fb80000 + offset*4, data, mem_mask );
}

READ32_MEMBER(ip22_state::hpc3_unkpbus0_r)
{
	return 0;
	////verboselog((machine(), 0, "Unknown PBUS Read: 0x%08x (%08x)\n", 0x1fbc8000 + offset*4, mem_mask );
	//return m_unkpbus0[offset];
}

WRITE32_MEMBER(ip22_state::hpc3_unkpbus0_w)
{
	////verboselog((machine(), 0, "Unknown PBUS Write: 0x%08x = 0x%08x (%08x)\n", 0x1fbc8000 + offset*4, data, mem_mask );
	//COMBINE_DATA(&m_unkpbus0[offset]);
}

static ADDRESS_MAP_START( ip225015_map, AS_PROGRAM, 32, ip22_state )
	AM_RANGE( 0x00000000, 0x0007ffff ) AM_RAMBANK( "bank1" )	/* mirror of first 512k of main RAM */
	AM_RANGE( 0x08000000, 0x0fffffff ) AM_SHARE("mainram") AM_RAM_WRITE(ip22_write_ram)		/* 128 MB of main RAM */
	AM_RANGE( 0x1f0f0000, 0x1f0f1fff ) AM_READWRITE_LEGACY(newport_rex3_r, newport_rex3_w )
	AM_RANGE( 0x1fa00000, 0x1fa1ffff ) AM_READWRITE_LEGACY(sgi_mc_r, sgi_mc_w )
	AM_RANGE( 0x1fb90000, 0x1fb9ffff ) AM_READWRITE(hpc3_hd_enet_r, hpc3_hd_enet_w )
	AM_RANGE( 0x1fbb0000, 0x1fbb0003 ) AM_RAM	/* unknown, but read a lot and discarded */
	AM_RANGE( 0x1fbc0000, 0x1fbc7fff ) AM_READWRITE(hpc3_hd0_r, hpc3_hd0_w )
	AM_RANGE( 0x1fbc8000, 0x1fbcffff ) AM_READWRITE(hpc3_unkpbus0_r, hpc3_unkpbus0_w ) AM_SHARE("unkpbus0")
	AM_RANGE( 0x1fb80000, 0x1fb8ffff ) AM_READWRITE(hpc3_pbusdma_r, hpc3_pbusdma_w )
	AM_RANGE( 0x1fbd8000, 0x1fbd83ff ) AM_READWRITE(hal2_r, hal2_w )
	AM_RANGE( 0x1fbd8400, 0x1fbd87ff ) AM_RAM /* hack */
	AM_RANGE( 0x1fbd9000, 0x1fbd93ff ) AM_READWRITE(hpc3_pbus4_r, hpc3_pbus4_w )
	AM_RANGE( 0x1fbd9800, 0x1fbd9bff ) AM_READWRITE(hpc3_pbus6_r, hpc3_pbus6_w )
	AM_RANGE( 0x1fbdc000, 0x1fbdc7ff ) AM_RAM
	AM_RANGE( 0x1fbdd000, 0x1fbdd3ff ) AM_RAM
	AM_RANGE( 0x1fbe0000, 0x1fbe04ff ) AM_READWRITE(rtc_r, rtc_w )
	AM_RANGE( 0x1fc00000, 0x1fc7ffff ) AM_ROM AM_REGION( "user1", 0 )
	AM_RANGE( 0x20000000, 0x27ffffff ) AM_SHARE("mainram") AM_RAM_WRITE(ip22_write_ram)
ADDRESS_MAP_END


static TIMER_CALLBACK(ip22_timer)
{
	machine.scheduler().timer_set(attotime::from_msec(1), FUNC(ip22_timer));
}

static MACHINE_RESET( ip225015 )
{
	ip22_state *state = machine.driver_data<ip22_state>();
	state->m_HPC3.nenetr_nbdp = 0x80000000;
	state->m_HPC3.nenetr_cbp = 0x80000000;
	state->m_nIntCounter = 0;
	RTC_REGISTERB = 0x08;
	RTC_REGISTERD = 0x80;

	machine.scheduler().timer_set(attotime::from_msec(1), FUNC(ip22_timer));

	// set up low RAM mirror
	state->membank("bank1")->set_base(state->m_mainram);

	state->m_PBUS_DMA.nActive = 0;

	mips3drc_set_options(machine.device("maincpu"), MIPS3DRC_COMPATIBLE_OPTIONS | MIPS3DRC_CHECK_OVERFLOWS);
}

static void dump_chain(address_space *space, UINT32 ch_base)
{

	printf("node: %08x %08x %08x (len = %x)\n", space->read_dword(ch_base), space->read_dword(ch_base+4), space->read_dword(ch_base+8), space->read_dword(ch_base+4) & 0x3fff);

	if ((space->read_dword(ch_base+8) != 0) && !(space->read_dword(ch_base+4) & 0x80000000))
	{
		dump_chain(space, space->read_dword(ch_base+8));
	}
}

// HPC3 SCSI DMA control register bits
#define HPC3_DMACTRL_IRQ	(0x01)
#define HPC3_DMACTRL_ENDIAN	(0x02)
#define HPC3_DMACTRL_DIR	(0x04)
#define HPC3_DMACTRL_ENABLE	(0x10)


static void scsi_irq(running_machine &machine, int state)
{
	ip22_state *drvstate = machine.driver_data<ip22_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	if (state)
	{
		if (drvstate->m_wd33c93->get_dma_count())
		{
			printf("drvstate->m_wd33c93->get_dma_count() is %d\n", drvstate->m_wd33c93->get_dma_count() );
			if (drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_ENABLE)
			{
				if (drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_IRQ) logerror("IP22: Unhandled SCSI DMA IRQ\n");
			}

			// HPC3 DMA: host to device
			if ((drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_ENABLE) && (drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_DIR))
			{
				UINT32 wptr, tmpword;
				int words, dptr, twords;

				words = drvstate->m_wd33c93->get_dma_count();
				words /= 4;

				wptr = space->read_dword(drvstate->m_HPC3.nSCSI0Descriptor);
				drvstate->m_HPC3.nSCSI0Descriptor += words*4;
				dptr = 0;

				printf("DMA to device: %d words @ %x\n", words, wptr);

				dump_chain(space, drvstate->m_HPC3.nSCSI0Descriptor);

				if (words <= (512/4))
				{
					// one-shot
					//drvstate->m_wd33c93->get_dma_data(drvstate->m_wd33c93->get_dma_count(), drvstate->m_dma_buffer);

					while (words)
					{
						tmpword = space->read_dword(wptr);

						if (drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_ENDIAN)
						{
							drvstate->m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
							drvstate->m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
							drvstate->m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
							drvstate->m_dma_buffer[dptr] = tmpword&0xff;
						}
						else
						{
							drvstate->m_dma_buffer[dptr] = (tmpword>>24)&0xff;
							drvstate->m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
							drvstate->m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
							drvstate->m_dma_buffer[dptr+3] = tmpword&0xff;
						}

						wptr += 4;
						dptr += 4;
						words--;
					}

					words = drvstate->m_wd33c93->get_dma_count();
					drvstate->m_wd33c93->write_data(words, drvstate->m_dma_buffer);
				}
				else
				{
					while (words)
					{
						//drvstate->m_wd33c93->get_dma_data(512, drvstate->m_dma_buffer);
						twords = 512/4;
						drvstate->m_HPC3.nSCSI0Descriptor += 512;
						dptr = 0;

						while (twords)
						{
							tmpword = space->read_dword(wptr);

							if (drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_ENDIAN)
							{
								drvstate->m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
								drvstate->m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
								drvstate->m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
								drvstate->m_dma_buffer[dptr] = tmpword&0xff;
							}
							else
							{
								drvstate->m_dma_buffer[dptr] = (tmpword>>24)&0xff;
								drvstate->m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
								drvstate->m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
								drvstate->m_dma_buffer[dptr+3] = tmpword&0xff;
							}

							wptr += 4;
							dptr += 4;
							twords--;
						}

						drvstate->m_wd33c93->write_data(512, drvstate->m_dma_buffer);

						words -= (512/4);
					}
				}

				// clear DMA on the controller too
				drvstate->m_wd33c93->clear_dma();
#if 0
				UINT32 dptr, tmpword;
				UINT32 bc = space->read_dword(drvstate->m_HPC3.nSCSI0Descriptor + 4);
				UINT32 rptr = space->read_dword(drvstate->m_HPC3.nSCSI0Descriptor);
				int length = bc & 0x3fff;
				int xie = (bc & 0x20000000) ? 1 : 0;
				int eox = (bc & 0x80000000) ? 1 : 0;

				dump_chain(space, drvstate->m_HPC3.nSCSI0Descriptor);

				printf("PC is %08x\n", machine.device("maincpu")->safe_pc());
				printf("DMA to device: length %x xie %d eox %d\n", length, xie, eox);

				if (length <= 0x4000)
				{
					dptr = 0;
					while (length > 0)
					{
						tmpword = space->read_dword(rptr);
						if (drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_ENDIAN)
						{
							drvstate->m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
							drvstate->m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
							drvstate->m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
							drvstate->m_dma_buffer[dptr] = tmpword&0xff;
						}
						else
						{
							drvstate->m_dma_buffer[dptr] = (tmpword>>24)&0xff;
							drvstate->m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
							drvstate->m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
							drvstate->m_dma_buffer[dptr+3] = tmpword&0xff;
						}

						dptr += 4;
						rptr += 4;
						length -= 4;
					}

					length = space->read_dword(drvstate->m_HPC3.nSCSI0Descriptor+4) & 0x3fff;
					drvstate->m_wd33c93->write_data(length, drvstate->m_dma_buffer);

					// clear DMA on the controller too
					drvstate->m_wd33c93->clear_dma();
				}
				else
				{
					logerror("IP22: overly large host to device transfer, can't handle!\n");
				}
#endif
			}

			// HPC3 DMA: device to host
			if ((drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_ENABLE) && !(drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_DIR))
			{
				UINT32 wptr, tmpword;
				int words, sptr, twords;

				words = drvstate->m_wd33c93->get_dma_count();
				words /= 4;

				wptr = space->read_dword(drvstate->m_HPC3.nSCSI0Descriptor);
				sptr = 0;

//              mame_printf_info("DMA from device: %d words @ %x\n", words, wptr);

				dump_chain(space, drvstate->m_HPC3.nSCSI0Descriptor);

				if (words <= (1024/4))
				{
					// one-shot
					drvstate->m_wd33c93->get_dma_data(drvstate->m_wd33c93->get_dma_count(), drvstate->m_dma_buffer);

					while (words)
					{
						if (drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_ENDIAN)
						{
							tmpword = drvstate->m_dma_buffer[sptr+3]<<24 | drvstate->m_dma_buffer[sptr+2]<<16 | drvstate->m_dma_buffer[sptr+1]<<8 | drvstate->m_dma_buffer[sptr];
						}
						else
						{
							tmpword = drvstate->m_dma_buffer[sptr]<<24 | drvstate->m_dma_buffer[sptr+1]<<16 | drvstate->m_dma_buffer[sptr+2]<<8 | drvstate->m_dma_buffer[sptr+3];
						}

						space->write_dword(wptr, tmpword);
						wptr += 4;
						sptr += 4;
						words--;
					}
				}
				else
				{
					while (words)
					{
						drvstate->m_wd33c93->get_dma_data(512, drvstate->m_dma_buffer);
						twords = 512/4;
						sptr = 0;

						while (twords)
						{
							if (drvstate->m_HPC3.nSCSI0DMACtrl & HPC3_DMACTRL_ENDIAN)
							{
								tmpword = drvstate->m_dma_buffer[sptr+3]<<24 | drvstate->m_dma_buffer[sptr+2]<<16 | drvstate->m_dma_buffer[sptr+1]<<8 | drvstate->m_dma_buffer[sptr];
							}
							else
							{
								tmpword = drvstate->m_dma_buffer[sptr]<<24 | drvstate->m_dma_buffer[sptr+1]<<16 | drvstate->m_dma_buffer[sptr+2]<<8 | drvstate->m_dma_buffer[sptr+3];
							}
							space->write_dword(wptr, tmpword);

							wptr += 4;
							sptr += 4;
							twords--;
						}

						words -= (512/4);
					}
				}

				// clear DMA on the controller too
				drvstate->m_wd33c93->clear_dma();
			}
		}

		// clear HPC3 DMA active flag
		drvstate->m_HPC3.nSCSI0DMACtrl &= ~HPC3_DMACTRL_ENABLE;

		// set the interrupt
		int3_raise_local0_irq(machine, INT3_LOCAL0_SCSI0);
	}
	else
	{
		int3_lower_local0_irq(drvstate, INT3_LOCAL0_SCSI0);
	}
}

static const struct WD33C93interface wd33c93_intf =
{
	&scsi_irq,		/* command completion IRQ */
};

static void ip225015_exit(running_machine &machine)
{
}

static int ip22_get_out2(running_machine &machine) {
	return pit8253_get_output(machine.device("pit8254"), 2 );
}

static MACHINE_START( ip225015 )
{
	ip22_state *state = machine.driver_data<ip22_state>();
	sgi_mc_init(machine);

	// SCSI init
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(ip225015_exit),&machine));

	machine.device<nvram_device>("nvram_user")->set_base(state->m_RTC.nUserRAM, 0x200);
	machine.device<nvram_device>("nvram")->set_base(state->m_RTC.nRAM, 0x200);
}

DRIVER_INIT_MEMBER(ip22_state,ip225015)
{
	static const struct kbdc8042_interface at8042 =
	{
		KBDC8042_STANDARD, NULL, NULL, NULL, ip22_get_out2
	};

	// IP22 uses 2 pieces of PC-compatible hardware: the 8042 PS/2 keyboard/mouse
	// interface and the 8254 PIT.  Both are licensed cores embedded in the IOC custom chip.
	init_pc_common(machine(), PCCOMMON_KEYBOARD_AT, NULL);
	kbdc8042_init(machine(), &at8042);

	m_nIOC_ParReadCnt = 0;
}

static INPUT_PORTS_START( ip225015 )
	PORT_START("IN0")	// unused IN0
	PORT_START("DSW0")	// unused IN1
	PORT_START("DSW1")	// unused IN2
	PORT_START("DSW2")	// unused IN3
	PORT_INCLUDE( at_keyboard )		/* IN4 - IN11 */
INPUT_PORTS_END

static void rtc_update(ip22_state *state)
{
	RTC_SECONDS++;

	switch( RTC_REGISTERB & 0x04 )
	{
	case 0x00:	/* Non-BCD */
		if( RTC_SECONDS == 60 )
		{
			RTC_SECONDS = 0;
			RTC_MINUTES++;
		}
		if( RTC_MINUTES == 60 )
		{
			RTC_MINUTES = 0;
			RTC_HOURS++;
		}
		if( RTC_HOURS == 24 )
		{
			RTC_HOURS = 0;
			RTC_DAYOFMONTH++;
		}
		RTC_SECONDS_A = RTC_SECONDS;
		RTC_MINUTES_A = RTC_MINUTES;
		RTC_HOURS_A = RTC_HOURS;
		break;
	case 0x04:	/* BCD */
		if( ( RTC_SECONDS & 0x0f ) == 0x0a )
		{
			RTC_SECONDS -= 0x0a;
			RTC_SECONDS += 0x10;
		}
		if( ( RTC_SECONDS & 0xf0 ) == 0x60 )
		{
			RTC_SECONDS -= 0x60;
			RTC_MINUTES++;
		}
		if( ( RTC_MINUTES & 0x0f ) == 0x0a )
		{
			RTC_MINUTES -= 0x0a;
			RTC_MINUTES += 0x10;
		}
		if( ( RTC_MINUTES & 0xf0 ) == 0x60 )
		{
			RTC_MINUTES -= 0x60;
			RTC_HOURS++;
		}
		if( ( RTC_HOURS & 0x0f ) == 0x0a )
		{
			RTC_HOURS -= 0x0a;
			RTC_HOURS += 0x10;
		}
		if( RTC_HOURS == 0x24 )
		{
			RTC_HOURS = 0;
			RTC_DAYOFMONTH++;
		}
		RTC_SECONDS_A = RTC_SECONDS;
		RTC_MINUTES_A = RTC_MINUTES;
		RTC_HOURS_A = RTC_HOURS;
		break;
	}
}

static INTERRUPT_GEN( ip22_vbl )
{
	ip22_state *state = device->machine().driver_data<ip22_state>();
	state->m_nIntCounter++;
//  if( state->m_nIntCounter == 60 )
	{
		state->m_nIntCounter = 0;
		rtc_update(state);
	}
}

static const mips3_config config =
{
	32768,	/* code cache size */
	32768	/* data cache size */
};

static const pc_lpt_interface ip22_lpt_config =
{
	DEVCB_NULL /* no idea if the lpt irq is connected and where */
};

static MACHINE_CONFIG_START( ip225015, ip22_state )
	MCFG_CPU_ADD( "maincpu", R5000BE, 50000000*3 )
	MCFG_CPU_CONFIG( config )
	MCFG_CPU_PROGRAM_MAP( ip225015_map)
	MCFG_CPU_VBLANK_INT("screen", ip22_vbl)

	MCFG_MACHINE_START( ip225015 )
	MCFG_MACHINE_RESET( ip225015 )

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_ADD_0FILL("nvram_user")

	MCFG_PIT8254_ADD( "pit8254", ip22_pit8254_config )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1280+64, 1024+64)
	MCFG_SCREEN_VISIBLE_AREA(0, 1279, 0, 1023)
	MCFG_SCREEN_UPDATE_STATIC( newport )

	MCFG_PALETTE_LENGTH(65536)

	MCFG_VIDEO_START( newport )

	MCFG_PC_LPT_ADD("lpt_0", ip22_lpt_config)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD( "dac", DAC, 0 )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.5)

	MCFG_SOUND_ADD( "cdda",  CDDA, 0 )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SCSIBUS_ADD("scsi")
	MCFG_SCSIDEV_ADD("scsi:harddisk1", SCSIHD, SCSI_ID_1)
	MCFG_SCSIDEV_ADD("scsi:cdrom", SCSICD, SCSI_ID_4)
	MCFG_WD33C93_ADD("scsi:wd33c93", wd33c93_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ip224613, ip225015 )
	MCFG_CPU_REPLACE( "maincpu", R4600BE, 133333333 )
	MCFG_CPU_CONFIG( config )
	MCFG_CPU_PROGRAM_MAP( ip225015_map)
	MCFG_CPU_VBLANK_INT("screen", ip22_vbl)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ip244415, ip225015 )
	MCFG_CPU_REPLACE( "maincpu", R4600BE, 150000000 )
	MCFG_CPU_CONFIG( config )
	MCFG_CPU_PROGRAM_MAP( ip225015_map)
	MCFG_CPU_VBLANK_INT("screen", ip22_vbl)
MACHINE_CONFIG_END

ROM_START( ip225015 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip225015.bin", 0x000000, 0x080000, CRC(aee5502e) SHA1(9243fef0a3508790651e0d6d2705c887629b1280) )
ROM_END

ROM_START( ip224613 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip224613.bin", 0x000000, 0x080000, CRC(f1868b5b) SHA1(0dcbbd776e671785b9b65f3c6dbd609794a40157) )
ROM_END

ROM_START( ip244415 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip244415.bin", 0x000000, 0x080000, CRC(2f37825a) SHA1(0d48c573b53a307478820b85aacb57b868297ca3) )
ROM_END

/*     YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT     COMPANY   FULLNAME */
COMP( 1993, ip225015, 0,        0,        ip225015, ip225015, ip22_state, ip225015, "Silicon Graphics Inc", "Indy (R5000, 150MHz)", GAME_NOT_WORKING )
COMP( 1993, ip224613, 0,        0,        ip224613, ip225015, ip22_state, ip225015, "Silicon Graphics Inc", "Indy (R4600, 133MHz)", GAME_NOT_WORKING )
COMP( 1994, ip244415, 0,        0,        ip244415, ip225015, ip22_state, ip225015, "Silicon Graphics Inc", "Indigo2 (R4400, 150MHz)", GAME_NOT_WORKING )
