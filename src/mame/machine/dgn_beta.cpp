// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

  machine\dgn_beta.c (machine.c)

    Moved out of dragon.c, 2005-05-05, P.Harvey-Smith.

    I decided to move this out of the main Dragon/CoCo source files, as
    the Beta is so radically different from the other Dragon machines that
    this made more sense (to me at least).

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  References:
    Disassembly of Dragon Beta ROM, examination of only (known) surviving board.

  TODO:
    Pretty much everything !

    Display working with 6845 taking data from rom.

  2005-05-10
    Memory banking seems to be working, as documented in code comments below.

  2005-05-31
    CPU#2 now executes code correctly to do transfers from WD2797.

  2005-06-03

    When fed a standard OS-9 boot disk it reads in the boot file and attempts
    to start it, not being able to find init, it fails. Hopefully I will
    soon have an image of a Beta boot disk.

  2005-11-29

    Major track tracing excersise on scans of bare beta board, reveal where a
    whole bunch of the PIA lines go, especially the IRQs, most of them go back
    to the IRQ line on the main CPU.

  2005-12-07

    First booted to OS9 prompt, did not execute startup scripts.

  2005-12-08

    Fixed density setting on WD2797, so density of read data is now
    correctlty set as required by OS-9. This was the reason startup
    script was not being executed as Beta disks have a single denisty
    boot track, however the rest of the disk is double density.
    Booted completely to OS-9, including running startup script.

  2006-09-27

    Clean up of IRQ/FIRQ handling code allows correct booting again.

***************************************************************************/

#include <math.h>
#include "emu.h"
#include "debug/debugcon.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "includes/dgn_beta.h"
#include "machine/mos6551.h"
#include "imagedev/flopdrv.h"

#include "debug/debugcpu.h"
#include "debug/debugcon.h"
#include "machine/ram.h"

#define VERBOSE 0


#define LOG_BANK_UPDATE(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_DEFAULT_TASK(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_PAGE_WRITE(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_HALT(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_TASK(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_KEYBOARD(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_VIDEO(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_DISK(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_INTS(x) do { if (VERBOSE) logerror x; } while (0)


/* Debugging commands and handlers. */
static offs_t dgnbeta_dasm_override(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);
static void execute_beta_dat_log(running_machine &machine, int ref, int params, const char *param[]);
static void execute_beta_key_dump(running_machine &machine, int ref, int params, const char *param[]);

//static int DMA_NMI;               /* DMA cpu has received an NMI */

#define INVALID_KEYROW  -1          /* no ketrow selected */
#define NO_KEY_PRESSED  0x7F            /* retrurned by hardware if no key pressed */

// Info for bank switcher
struct bank_info_entry
{
	write8_delegate func;   // Pointer to write handler
	offs_t start;       // Offset of start of block
	offs_t end;     // offset of end of block
};

static const struct bank_info_entry bank_info[] =
{
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b0_w),(dgn_beta_state*)nullptr), 0x0000, 0x0fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b1_w),(dgn_beta_state*)nullptr), 0x1000, 0x1fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b2_w),(dgn_beta_state*)nullptr), 0x2000, 0x2fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b3_w),(dgn_beta_state*)nullptr), 0x3000, 0x3fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b4_w),(dgn_beta_state*)nullptr), 0x4000, 0x4fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b5_w),(dgn_beta_state*)nullptr), 0x5000, 0x5fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b6_w),(dgn_beta_state*)nullptr), 0x6000, 0x6fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b7_w),(dgn_beta_state*)nullptr), 0x7000, 0x7fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b8_w),(dgn_beta_state*)nullptr), 0x8000, 0x8fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_b9_w),(dgn_beta_state*)nullptr), 0x9000, 0x9fff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_bA_w),(dgn_beta_state*)nullptr), 0xA000, 0xAfff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_bB_w),(dgn_beta_state*)nullptr), 0xB000, 0xBfff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_bC_w),(dgn_beta_state*)nullptr), 0xC000, 0xCfff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_bD_w),(dgn_beta_state*)nullptr), 0xD000, 0xDfff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_bE_w),(dgn_beta_state*)nullptr), 0xE000, 0xEfff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_bF_w),(dgn_beta_state*)nullptr), 0xF000, 0xFBff },
	{ write8_delegate(FUNC(dgn_beta_state::dgnbeta_ram_bG_w),(dgn_beta_state*)nullptr), 0xFF00, 0xFfff }
};

#define is_last_page(page)  (((page==LastPage) || (page==LastPage+1)) ? 1 : 0)

//
// Memory pager, maps machine's 1Mb total address space into the 64K addressable by the 6809.
// This is in some way similar to the system used on the CoCo 3, except the beta uses 4K
// pages (instead of 8K on the CoCo), and has 16 task registers instead of the 2 on the CoCo.
//
// Each block of 16 page registers (1 for each task), is paged in at $FE00-$FE0F, the bottom
// 4 bits of the PIA register at $FCC0 seem to contain which task is active.
// bit 6 of the same port seems to enable the memory paging
//
// For the purpose of this driver any block that is not ram, and is not a known ROM block,
// is mapped to the first page of the boot rom, I do not know what happens in the real
// hardware, however this does allow the boot rom to correctly size the RAM.
// this should probably be considdered a hack !
//

void dgn_beta_state::UpdateBanks(int first, int last)
{
	address_space &space_0 = m_maincpu->space(AS_PROGRAM);
	address_space &space_1 = machine().device(DMACPU_TAG)->memory().space(AS_PROGRAM);
	int                 Page;
	UINT8               *readbank;
	int                 bank_start;
	int                 bank_end;
	int                 MapPage;
	char                page_num[10];

	LOG_BANK_UPDATE(("\n\nUpdating banks %d to %d at PC=$%X\n",first,last,space_0.device().safe_pc()));
	for(Page=first;Page<=last;Page++)
	{
		sprintf(page_num,"bank%d",Page+1);

		bank_start  = bank_info[Page].start;
		bank_end    = bank_info[Page].end;

		// bank16 and bank17 are mapped to the same page with a hole for the IO memory
		if (!is_last_page(Page))
			MapPage = m_PageRegs[m_TaskReg][Page].value;
		else
			MapPage = m_PageRegs[m_TaskReg][LastPage].value;

		//
		// Map block, $00-$BF are ram, $FC-$FF are Boot ROM
		//
		if ((MapPage*4) < ((m_ram->size() / 1024)-1))     // Block is ram
		{
			if (!is_last_page(Page))
			{
				readbank = &m_ram->pointer()[MapPage*RamPageSize];
				if(m_LogDatWrites)
					debug_console_printf(machine(), "Mapping page %X, pageno=%X, mess_ram)[%X]\n",Page,MapPage,(MapPage*RamPageSize));
			}
			else
			{
				readbank = &m_ram->pointer()[(MapPage*RamPageSize)-256];
				logerror("Error RAM in Last page !\n");
			}
			write8_delegate func = bank_info[Page].func;
			if (!func.isnull()) func.late_bind(*this);
			space_0.install_write_handler(bank_start, bank_end, func);
			space_1.install_write_handler(bank_start, bank_end, func);
		}
		else                    // Block is rom, or undefined
		{
			if (MapPage>0xfB)
			{
				if (Page!=IOPage+1)
					readbank=&m_system_rom[(MapPage-0xFC)*0x1000];
				else
					readbank=&m_system_rom[0x3F00];
			}
			else
				readbank=m_system_rom;

			space_0.unmap_write(bank_start, bank_end);
			space_1.unmap_write(bank_start, bank_end);
		}

		m_PageRegs[m_TaskReg][Page].memory=readbank;
		membank(page_num)->set_base(readbank);

		LOG_BANK_UPDATE(("UpdateBanks:MapPage=$%02X readbank=$%X\n",MapPage,(int)(FPTR)readbank));
		LOG_BANK_UPDATE(("PageRegsSet Task=%X Page=%x\n",m_TaskReg,Page));
		//LOG_BANK_UPDATE(("%X)\n",membank(Page+1)));
		LOG_BANK_UPDATE(("memory_install_write8_handler CPU=0\n"));
		LOG_BANK_UPDATE(("memory_install_write8_handler CPU=1\n"));
	}
}

//
void dgn_beta_state::SetDefaultTask()
{
//  UINT8 *videoram = m_videoram;
	int     Idx;

	LOG_DEFAULT_TASK(("SetDefaultTask()\n"));
	//if (VERBOSE) debug_console_printf(machine())->set_base("Set Default task\n");

	m_TaskReg=NoPagingTask;

	/* Reset ram pages */
	for(Idx=0;Idx<ROMPage-1;Idx++)
	{
		m_PageRegs[m_TaskReg][Idx].value=NoMemPageValue;
	}

	/* Reset RAM Page */
	m_PageRegs[m_TaskReg][RAMPage].value=RAMPageValue;

	/* Reset Video mem page */
	m_PageRegs[m_TaskReg][VideoPage].value=VideoPageValue;

	/* Reset rom page */
	m_PageRegs[m_TaskReg][ROMPage].value=ROMPageValue;

	/* Reset IO Page */
	m_PageRegs[m_TaskReg][LastPage].value=IOPageValue;
	m_PageRegs[m_TaskReg][LastPage+1].value=IOPageValue;

	UpdateBanks(0,LastPage+1);

	/* Map video ram to base of area it can use, that way we can take the literal RA */
	/* from the 6845 without having to mask it ! */
//  videoram=&m_ram->pointer()[TextVidBasePage*RamPageSize];
}

// Return the value of a page register
READ8_MEMBER(dgn_beta_state::dgn_beta_page_r)
{
	return m_PageRegs[m_PIATaskReg][offset].value;
}

// Write to a page register, writes to the register, and then checks to see
// if memory banking is active, if it is, it calls UpdateBanks, to actually
// setup the mappings.

WRITE8_MEMBER(dgn_beta_state::dgn_beta_page_w )
{
	m_PageRegs[m_PIATaskReg][offset].value=data;

	LOG_PAGE_WRITE(("PageRegWrite : task=$%X  offset=$%X value=$%X\n",m_PIATaskReg,offset,data));

	if (m_EnableMapRegs)
	{
		UpdateBanks(offset,offset);
		if (offset==15)
			UpdateBanks(offset+1,offset+1);
	}
}

/*********************** Memory bank write handlers ************************/
/* These actually write the data to the memory, and not to the page regs ! */
void dgn_beta_state::dgn_beta_bank_memory(int offset, int data, int bank)
{
	m_PageRegs[m_TaskReg][bank].memory[offset]=data;
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b0_w )
{
	dgn_beta_bank_memory(offset,data,0);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b1_w )
{
	dgn_beta_bank_memory(offset,data,1);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b2_w )
{
	dgn_beta_bank_memory(offset,data,2);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b3_w )
{
	dgn_beta_bank_memory(offset,data,3);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b4_w )
{
	dgn_beta_bank_memory(offset,data,4);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b5_w )
{
	dgn_beta_bank_memory(offset,data,5);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b6_w )
{
	dgn_beta_bank_memory(offset,data,6);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b7_w )
{
	dgn_beta_bank_memory(offset,data,7);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b8_w )
{
	dgn_beta_bank_memory(offset,data,8);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_b9_w )
{
	dgn_beta_bank_memory(offset,data,9);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_bA_w )
{
	dgn_beta_bank_memory(offset,data,10);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_bB_w )
{
	dgn_beta_bank_memory(offset,data,11);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_bC_w )
{
	dgn_beta_bank_memory(offset,data,12);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_bD_w )
{
	dgn_beta_bank_memory(offset,data,13);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_bE_w )
{
	dgn_beta_bank_memory(offset,data,14);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_bF_w )
{
	dgn_beta_bank_memory(offset,data,15);
}

WRITE8_MEMBER(dgn_beta_state::dgnbeta_ram_bG_w )
{
	dgn_beta_bank_memory(offset,data,16);
}

/*
The keyrow being scanned for any key is the lest significant bit
of the output shift register that is zero, most of the time there should
only be one row active e.g.

Shifter     Row being scanned
1111111110  0
1111111101  1
1111111011  2

etc.

Returns row number or -1 if none selected.

2006-12-03, P.Harvey-Smith, modified to scan from msb to lsb, and stop at
first row with zero, as the beta_test fills the shifter with zeros, rather
than using a walking zero as the OS-9 driver does. This meant that SelectKeyrow
never moved past the first row, by scanning for the last active row
the beta_test rom works, and it does not break the OS-9 driver :)
*/
int dgn_beta_state::SelectedKeyrow(dgn_beta_state *state, int Rows)
{
	int Idx;
	int Row;    /* Row selected */
	int Mask;   /* Mask to test row */
	int Found;  /* Set true when found */

	Row=INVALID_KEYROW; /* Pretend no rows selected */
	Mask=0x200;     /* Start with row 9 */
	Found=0;        /* Start with not found */
	Idx=9;

	while ((Mask>0) && !Found)
	{
		if((~Rows & Mask) && !Found)
		{
			Row=Idx;        /* Get row */
			Found=1;        /* Mark as found */
		}
		Idx=Idx-1;          /* Decrement row count */
		Mask=Mask>>1;           /* Select next bit */
	}

	return Row;
}

/* GetKeyRow, returns the value of a keyrow, checking for invalid rows */
/* and returning no key pressed if row is invalid */
int dgn_beta_state::GetKeyRow(dgn_beta_state *state, int RowNo)
{
	if(RowNo==INVALID_KEYROW)
		return NO_KEY_PRESSED;  /* row is invalid, so return no key down */
	else
		return m_Keyboard[RowNo];    /* Else return keyboard data */
}

/*********************************** PIA Handlers ************************/
/* PIA #0 at $FC20-$FC23 I46
    This handles:-
        The Printer port, side A,
        PB0 R16 (pullup) -> Printer Port (PL1)
        PB1 Printer Port
        PB2     Keyboard (any key).
        PB3 D4 -> R37 -> TR1 switching circuit -> PL5
        PB4 Keyboard Data out, clocked by CB2.
                positive edge clocks data out of the input shift register.
        PB5 Keyboard data in, clocked by PB4.
        PB6 R79 -> I99/6/7416 -> PL9/26/READY (from floppy)
        PB7 Printer port
        CB1 I36/39/6845(Horz Sync)
        CB2 Keyboard (out) Low loads input shift reg
*/
READ8_MEMBER(dgn_beta_state::d_pia0_pa_r)
{
	return 0;
}

WRITE8_MEMBER(dgn_beta_state::d_pia0_pa_w)
{
}

READ8_MEMBER(dgn_beta_state::d_pia0_pb_r)
{
	int RetVal;
	int Idx;
	int Selected;
	static const char *const keynames[] = {
		"KEY0", "KEY1", "KEY2", "KEY3", "KEY4",
		"KEY5", "KEY6", "KEY7", "KEY8", "KEY9"
	};

	LOG_KEYBOARD(("PB Read\n"));

	m_KAny_next = 0;

	Selected = SelectedKeyrow(this, m_RowShifter);

	/* Scan the whole keyboard, if output shifter is all low */
	/* This actually scans in the keyboard */
	if(m_RowShifter == 0x00)
	{
		for(Idx=0; Idx<NoKeyrows; Idx++)
		{
			m_Keyboard[Idx] = ioport(keynames[Idx])->read();

			if(m_Keyboard[Idx] != 0x7F)
				m_KAny_next = 1;
		}
	}
	else    /* Just scan current row, from previously read values */
	{
		if(GetKeyRow(this, Selected) != NO_KEY_PRESSED)
			m_KAny_next = 1;
	}

	RetVal = (m_KInDat_next<<5) | (m_KAny_next<<2);

	LOG_KEYBOARD(("FC22=$%02X KAny=%d\n", RetVal, m_KAny_next));

	return RetVal;
}

WRITE8_MEMBER(dgn_beta_state::d_pia0_pb_w)
{
	int InClkState;
	//int   OutClkState;

	LOG_KEYBOARD(("PB Write\n"));

	InClkState  = data & KInClk;
	//OutClkState   = data & KOutClk;

	LOG_KEYBOARD(("InClkState=$%02X OldInClkState=$%02X Keyrow=$%02X ",InClkState,(m_d_pia0_pb_last & KInClk),m_Keyrow));

	/* Input clock bit has changed state */
	if ((InClkState) != (m_d_pia0_pb_last & KInClk))
	{
		/* Clock in bit */
		if(InClkState)
		{
			m_KInDat_next=(~m_Keyrow & 0x40)>>6;
			m_Keyrow = ((m_Keyrow<<1) | 0x01) & 0x7F ;
			LOG_KEYBOARD(("Keyrow=$%02X KInDat_next=%X\n",m_Keyrow,m_KInDat_next));
		}
	}

	m_d_pia0_pb_last=data;
}

WRITE_LINE_MEMBER(dgn_beta_state::d_pia0_cb2_w)
{
	int RowNo;
	LOG_KEYBOARD(("\nCB2 Write\n"));

	/* load keyrow on rising edge of CB2 */
	if((state==1) && (m_d_pia0_cb2_last==0))
	{
		RowNo=SelectedKeyrow(this, m_RowShifter);
		m_Keyrow=GetKeyRow(this, RowNo);

		/* Output clock rising edge, clock CB2 value into rowshifterlow to high transition */
		/* In the beta the shift registers are a cmos 4015, and a cmos 4013 in series */
		m_RowShifter = (m_RowShifter<<1) | ((m_d_pia0_pb_last & KOutDat)>>4);
		m_RowShifter &= 0x3FF;
		LOG_KEYBOARD(("Rowshifter=$%02X Keyrow=$%02X\n",m_RowShifter,m_Keyrow));
		if (VERBOSE) debug_console_printf(machine(), "rowshifter clocked, value=%3X, RowNo=%d, Keyrow=%2X\n",m_RowShifter,RowNo,m_Keyrow);
	}

	m_d_pia0_cb2_last=state;
}


WRITE_LINE_MEMBER(dgn_beta_state::d_pia0_irq_a)
{
	cpu0_recalc_irq(state);
}

WRITE_LINE_MEMBER(dgn_beta_state::d_pia0_irq_b)
{
	cpu0_recalc_firq(state);
}

/* PIA #1 at $FC24-$FC27 I63
    This handles :-
        Mouse + Disk Select on side A
        Halt on DMA CPU         PA7
        Beeper                  PB0
        Halt on main CPU        PB1
        Character set select    PB6
        Baud rate               PB1..PB5 ????
*/

READ8_MEMBER(dgn_beta_state::d_pia1_pa_r)
{
	return 0;
}

WRITE8_MEMBER(dgn_beta_state::d_pia1_pa_w)
{
	int HALT_DMA;

	/* Only play with halt line if halt bit changed since last write */
	if((data & 0x80) != m_d_pia1_pa_last)
	{
		/* Bit 7 of $FF24, seems to control HALT on second CPU (through an inverter) */
		if(data & 0x80)
			HALT_DMA = ASSERT_LINE;
		else
			HALT_DMA = CLEAR_LINE;

		LOG_HALT(("DMA_CPU HALT=%d\n", HALT_DMA));
		machine().device(DMACPU_TAG)->execute().set_input_line(INPUT_LINE_HALT, HALT_DMA);

		/* CPU un-halted let it run ! */
		if (HALT_DMA == CLEAR_LINE)
			m_maincpu->yield();

		m_d_pia1_pa_last = data & 0x80;
	}

	/* Drive selects are binary encoded on PA0 & PA1 */
	floppy_image_device *floppy = nullptr;

	switch (~data & 0x03)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	case 2: floppy = m_floppy2->get_device(); break;
	case 3: floppy = m_floppy3->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	// not connected: bit 5 = ENP
	m_fdc->dden_w(BIT(data, 6));
	LOG_DISK(("Set density %s\n", BIT(data, 6) ? "low" : "high"));
}

READ8_MEMBER(dgn_beta_state::d_pia1_pb_r)
{
	return 0;
}

WRITE8_MEMBER(dgn_beta_state::d_pia1_pb_w)
{
	int HALT_CPU;

	/* Only play with halt line if halt bit changed since last write */
	if((data & 0x02) != m_d_pia1_pb_last)
	{
		/* Bit 1 of $FF26, seems to control HALT on primary CPU */
		if(data & 0x02)
			HALT_CPU = CLEAR_LINE;
		else
			HALT_CPU = ASSERT_LINE;

		LOG_HALT(("MAIN_CPU HALT=%d\n", HALT_CPU));
		m_maincpu->set_input_line(INPUT_LINE_HALT, HALT_CPU);

		m_d_pia1_pb_last = data & 0x02;

		/* CPU un-halted let it run ! */
		if (HALT_CPU == CLEAR_LINE)
			machine().device(DMACPU_TAG)->execute().yield();
	}
}

WRITE_LINE_MEMBER(dgn_beta_state::d_pia1_irq_a)
{
	cpu0_recalc_irq(state);
}

WRITE_LINE_MEMBER(dgn_beta_state::d_pia1_irq_b)
{
	cpu0_recalc_irq(state);
}

/* PIA #2 at FCC0-FCC3 I28
    This handles :-
        DAT task select PA0..PA3

        DMA CPU NMI PA7

        Graphics control PB0..PB7 ???
        VSYNC intutrupt CB2
*/
READ8_MEMBER(dgn_beta_state::d_pia2_pa_r)
{
	return 0;
}

WRITE8_MEMBER(dgn_beta_state::d_pia2_pa_w)
{
	int OldTask;
	int OldEnableMap;
	int NMI;

	LOG_TASK(("FCC0 write : $%02X\n", data));

	/* Bit 7 of $FFC0, seems to control NMI on second CPU */
	NMI=(data & 0x80);

	/* only take action if NMI changed */
	if(NMI != m_DMA_NMI_LAST)
	{
		LOG_INTS(("cpu1 NMI : %d\n", NMI));
		if(!NMI)
		{
			machine().device(DMACPU_TAG)->execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
			logerror("device_yield()\n");
			machine().device(DMACPU_TAG)->execute().yield();    /* Let DMA CPU run */
		}
		else
		{
			machine().device(DMACPU_TAG)->execute().set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		}

		m_DMA_NMI_LAST = NMI;   /* Save it for next time */
	}

	OldEnableMap = m_EnableMapRegs;
	/* Bit 6 seems to enable memory paging */
	if(data & 0x40)
		m_EnableMapRegs = 0;
	else
		m_EnableMapRegs = 1;

	/* Bits 0..3 seem to control which task register is selected */
	OldTask = m_PIATaskReg;
	m_PIATaskReg = data & 0x0F;

	LOG_TASK(("OldTask=$%02X EnableMapRegs=%d OldEnableMap=%d\n", OldTask, m_EnableMapRegs, OldEnableMap));

	// Maping was enabled or disabled, select apropreate task reg
	// and map it in
	if (m_EnableMapRegs != OldEnableMap)
	{
		if(m_EnableMapRegs)
			m_TaskReg = m_PIATaskReg;
		else
			m_TaskReg = NoPagingTask;

		UpdateBanks(0, IOPage + 1);
	}
	else
	{
		// Update ram banks only if task reg changed and mapping enabled
		if ((m_PIATaskReg != OldTask) && (m_EnableMapRegs))
		{
			m_TaskReg = m_PIATaskReg;
			UpdateBanks(0, IOPage + 1);
		}
	}
	LOG_TASK(("TaskReg=$%02X PIATaskReg=$%02X\n", m_TaskReg, m_PIATaskReg));
}

READ8_MEMBER(dgn_beta_state::d_pia2_pb_r)
{
	return 0;
}

WRITE8_MEMBER(dgn_beta_state::d_pia2_pb_w)
{
	/* Update top video address lines */
	dgnbeta_vid_set_gctrl(data);
}

WRITE_LINE_MEMBER(dgn_beta_state::d_pia2_irq_a)
{
	cpu0_recalc_irq(state);
}

WRITE_LINE_MEMBER(dgn_beta_state::d_pia2_irq_b)
{
	cpu0_recalc_irq(state);
}

/************************************ Recalculate CPU inturrupts ****************************/
/* CPU 0 */
void dgn_beta_state::cpu0_recalc_irq(int state)
{
	pia6821_device *pia_0 = machine().device<pia6821_device>( PIA_0_TAG );
	pia6821_device *pia_1 = machine().device<pia6821_device>( PIA_1_TAG );
	pia6821_device *pia_2 = machine().device<pia6821_device>( PIA_2_TAG );
	UINT8 pia0_irq_a = pia_0->irq_a_state();
	UINT8 pia1_irq_a = pia_1->irq_a_state();
	UINT8 pia1_irq_b = pia_1->irq_b_state();
	UINT8 pia2_irq_a = pia_2->irq_a_state();
	UINT8 pia2_irq_b = pia_2->irq_b_state();
	UINT8 IRQ;

	if (pia0_irq_a || pia1_irq_a || pia1_irq_b || pia2_irq_a || pia2_irq_b)
		IRQ = ASSERT_LINE;
	else
		IRQ = CLEAR_LINE;

	m_maincpu->set_input_line(M6809_IRQ_LINE, IRQ);
	LOG_INTS(("cpu0 IRQ : %d\n", IRQ));
}

void dgn_beta_state::cpu0_recalc_firq(int state)
{
	pia6821_device *pia_0 = machine().device<pia6821_device>( PIA_0_TAG );
	UINT8 pia0_irq_b = pia_0->irq_b_state();
	UINT8 FIRQ;

	if (pia0_irq_b)
		FIRQ = ASSERT_LINE;
	else
		FIRQ = CLEAR_LINE;

	m_maincpu->set_input_line(M6809_FIRQ_LINE, FIRQ);

	LOG_INTS(("cpu0 FIRQ : %d\n", FIRQ));
}

/* CPU 1 */

void dgn_beta_state::cpu1_recalc_firq(int state)
{
	machine().device(DMACPU_TAG)->execute().set_input_line(M6809_FIRQ_LINE, state);
	LOG_INTS(("cpu1 FIRQ : %d\n",state));
}

/********************************************************************************************/
/* Dragon Beta onboard FDC */
/********************************************************************************************/

/* The INTRQ line goes through pia2 ca1, in exactly the same way as DRQ from DragonDos does */
WRITE_LINE_MEMBER( dgn_beta_state::dgnbeta_fdc_intrq_w )
{
	device_t *device = machine().device(PIA_2_TAG);
	LOG_DISK(("dgnbeta_fdc_intrq_w(%d)\n", state));
	if(m_wd2797_written)
		downcast<pia6821_device *>(device)->ca1_w(state);
}

/* DRQ is routed through various logic to the FIRQ inturrupt line on *BOTH* CPUs */
WRITE_LINE_MEMBER( dgn_beta_state::dgnbeta_fdc_drq_w )
{
	LOG_DISK(("dgnbeta_fdc_drq_w(%d)\n", state));
	cpu1_recalc_firq(state);
}

READ8_MEMBER( dgn_beta_state::dgnbeta_wd2797_r )
{
	return m_fdc->read(space, offset & 0x03);
}

WRITE8_MEMBER( dgn_beta_state::dgnbeta_wd2797_w )
{
	m_wd2797_written = 1;
	m_fdc->write(space, offset & 0x03, data);
}

/* Scan physical keyboard into Keyboard array */
/* gonna try and sync this more closely with hardware as keyboard being scanned */
/* on *EVERY* vblank ! */
void dgn_beta_state::ScanInKeyboard(void)
{
#if 0
	int Idx;
	int Row;
	static const char *const keynames[] = {
		"KEY0", "KEY1", "KEY2", "KEY3", "KEY4",
		"KEY5", "KEY6", "KEY7", "KEY8", "KEY9"
	};

	LOG_KEYBOARD(("Scanning Host keyboard\n"));

	for(Idx=0; Idx<NoKeyrows; Idx++)
	{
		if (Idx < 10)
			Row = ioport(keynames[Idx])->read();

		else
			Row = 0x7f;

		m_Keyboard[Idx]=Row;
		LOG_KEYBOARD(("Keyboard[%d]=$%02X\n",Idx,Row));

		if (Row != 0x7F)
		{
			LOG_KEYBOARD(("Found Pressed Key\n"));
		}
	}
#endif
}

/* VBlank inturrupt */
void dgn_beta_state::dgn_beta_frame_interrupt (int data)
{
	pia6821_device *pia_2 = machine().device<pia6821_device>( PIA_2_TAG );

	/* Set PIA line, so it recognises inturrupt */
	if (!data)
		pia_2->cb2_w(ASSERT_LINE);
	else
		pia_2->cb2_w(CLEAR_LINE);

//    LOG_VIDEO(("Vblank\n"));
	ScanInKeyboard();
}

#ifdef UNUSED_FUNCTION
void dgn_beta_state::dgn_beta_line_interrupt (int data)
{
//  /* Set PIA line, so it recognises inturrupt */
//  if (data)
//  {
//      pia_0_cb1_w(machine, 0,ASSERT_LINE);
//  }
//  else
//  {
//      pia_0_cb1_w(machine, 0,CLEAR_LINE);
//  }
}
#endif


/********************************* Machine/Driver Initialization ****************************************/
void dgn_beta_state::machine_reset()
{
	pia6821_device *pia_0 = machine().device<pia6821_device>( PIA_0_TAG );
	pia6821_device *pia_1 = machine().device<pia6821_device>( PIA_1_TAG );
	pia6821_device *pia_2 = machine().device<pia6821_device>( PIA_2_TAG );

	logerror("dgn_beta_state::machine_reset()\n");

	m_system_rom = memregion(MAINCPU_TAG)->base();

	/* Make sure CPU 1 is started out halted ! */
	machine().device(DMACPU_TAG)->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	/* Reset to task 0, and map banks disabled, so standard memory map */
	/* with ram at $0000-$BFFF, ROM at $C000-FBFF, IO at $FC00-$FEFF */
	/* and ROM at $FF00-$FFFF */
	m_TaskReg = 0;
	m_PIATaskReg = 0;
	m_EnableMapRegs = 0;
	memset(m_PageRegs, 0, sizeof(m_PageRegs));    /* Reset page registers to 0 */
	SetDefaultTask();

	/* Set pullups on all PIA port A, to match what hardware does */
	pia_0->set_port_a_z_mask(0xFF);
	pia_1->set_port_a_z_mask(0xFF);
	pia_2->set_port_a_z_mask(0xFF);

	m_d_pia1_pa_last = 0x00;
	m_d_pia1_pb_last = 0x00;
	m_RowShifter = 0x00;         /* shift register to select row */
	m_Keyrow = 0x00;             /* Keyboard row being shifted out */
	m_d_pia0_pb_last = 0x00;     /* Last byte output to pia0 port b */
	m_d_pia0_cb2_last = 0x00;        /* Last state of CB2 */

	m_KInDat_next = 0x00;            /* Next data bit to input */
	m_KAny_next = 0x00;          /* Next value for KAny */

	m_DMA_NMI_LAST = 0x80;       /* start with DMA NMI inactive, as pulled up */
//  DMA_NMI = CLEAR_LINE;       /* start with DMA NMI inactive */

	m_videoram.set_target(m_ram->pointer(),m_videoram.bytes());     /* Point video ram at the start of physical ram */

	m_wd2797_written=0;

	m_maincpu->reset();
}

void dgn_beta_state::machine_start()
{
	logerror("MACHINE_START( dgnbeta )\n");

	if (machine().device<cpu_device>(MAINCPU_TAG)->debug()) {
		machine().device<cpu_device>(MAINCPU_TAG)->debug()->set_dasm_override(dgnbeta_dasm_override);
	}

	/* setup debug commands */
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_register_command(machine(), "beta_dat_log", CMDFLAG_NONE, 0, 0, 0, execute_beta_dat_log);
		debug_console_register_command(machine(), "beta_key_dump", CMDFLAG_NONE, 0, 0, 0, execute_beta_key_dump);
	}
	m_LogDatWrites=0;
}


/***************************************************************************
  OS9 Syscalls for disassembly
****************************************************************************/


static const char *const os9syscalls[] =
{
	"F$Link",          /* Link to Module */
	"F$Load",          /* Load Module from File */
	"F$UnLink",        /* Unlink Module */
	"F$Fork",          /* Start New Process */
	"F$Wait",          /* Wait for Child Process to Die */
	"F$Chain",         /* Chain Process to New Module */
	"F$Exit",          /* Terminate Process */
	"F$Mem",           /* Set Memory Size */
	"F$Send",          /* Send Signal to Process */
	"F$Icpt",          /* Set Signal Intercept */
	"F$Sleep",         /* Suspend Process */
	"F$SSpd",          /* Suspend Process */
	"F$ID",            /* Return Process ID */
	"F$SPrior",        /* Set Process Priority */
	"F$SSWI",          /* Set Software Interrupt */
	"F$PErr",          /* Print Error */
	"F$PrsNam",        /* Parse Pathlist Name */
	"F$CmpNam",        /* Compare Two Names */
	"F$SchBit",        /* Search Bit Map */
	"F$AllBit",        /* Allocate in Bit Map */
	"F$DelBit",        /* Deallocate in Bit Map */
	"F$Time",          /* Get Current Time */
	"F$STime",         /* Set Current Time */
	"F$CRC",           /* Generate CRC */
	"F$GPrDsc",        /* get Process Descriptor copy */
	"F$GBlkMp",        /* get System Block Map copy */
	"F$GModDr",        /* get Module Directory copy */
	"F$CpyMem",        /* Copy External Memory */
	"F$SUser",         /* Set User ID number */
	"F$UnLoad",        /* Unlink Module by name */
	"F$Alarm",         /* Color Computer Alarm Call (system wide) */
	nullptr,
	nullptr,
	"F$NMLink",        /* Color Computer NonMapping Link */
	"F$NMLoad",        /* Color Computer NonMapping Load */
	nullptr,
	nullptr,
	"F$TPS",           /* Return System's Ticks Per Second */
	"F$TimAlm",        /* COCO individual process alarm call */
	"F$VIRQ",          /* Install/Delete Virtual IRQ */
	"F$SRqMem",        /* System Memory Request */
	"F$SRtMem",        /* System Memory Return */
	"F$IRQ",           /* Enter IRQ Polling Table */
	"F$IOQu",          /* Enter I/O Queue */
	"F$AProc",         /* Enter Active Process Queue */
	"F$NProc",         /* Start Next Process */
	"F$VModul",        /* Validate Module */
	"F$Find64",        /* Find Process/Path Descriptor */
	"F$All64",         /* Allocate Process/Path Descriptor */
	"F$Ret64",         /* Return Process/Path Descriptor */
	"F$SSvc",          /* Service Request Table Initialization */
	"F$IODel",         /* Delete I/O Module */
	"F$SLink",         /* System Link */
	"F$Boot",          /* Bootstrap System */
	"F$BtMem",         /* Bootstrap Memory Request */
	"F$GProcP",        /* Get Process ptr */
	"F$Move",          /* Move Data (low bound first) */
	"F$AllRAM",        /* Allocate RAM blocks */
	"F$AllImg",        /* Allocate Image RAM blocks */
	"F$DelImg",        /* Deallocate Image RAM blocks */
	"F$SetImg",        /* Set Process DAT Image */
	"F$FreeLB",        /* Get Free Low Block */
	"F$FreeHB",        /* Get Free High Block */
	"F$AllTsk",        /* Allocate Process Task number */
	"F$DelTsk",        /* Deallocate Process Task number */
	"F$SetTsk",        /* Set Process Task DAT registers */
	"F$ResTsk",        /* Reserve Task number */
	"F$RelTsk",        /* Release Task number */
	"F$DATLog",        /* Convert DAT Block/Offset to Logical */
	"F$DATTmp",        /* Make temporary DAT image (Obsolete) */
	"F$LDAXY",         /* Load A [X,[Y]] */
	"F$LDAXYP",        /* Load A [X+,[Y]] */
	"F$LDDDXY",        /* Load D [D+X,[Y]] */
	"F$LDABX",         /* Load A from 0,X in task B */
	"F$STABX",         /* Store A at 0,X in task B */
	"F$AllPrc",        /* Allocate Process Descriptor */
	"F$DelPrc",        /* Deallocate Process Descriptor */
	"F$ELink",         /* Link using Module Directory Entry */
	"F$FModul",        /* Find Module Directory Entry */
	"F$MapBlk",        /* Map Specific Block */
	"F$ClrBlk",        /* Clear Specific Block */
	"F$DelRAM",        /* Deallocate RAM blocks */
	"F$GCMDir",        /* Pack module directory */
	"F$AlHRam",        /* Allocate HIGH RAM Blocks */
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"F$RegDmp",        /* Ron Lammardo's debugging register dump call */
	"F$NVRAM",         /* Non Volatile RAM (RTC battery backed static) read/write */
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"I$Attach",        /* Attach I/O Device */
	"I$Detach",        /* Detach I/O Device */
	"I$Dup",           /* Duplicate Path */
	"I$Create",        /* Create New File */
	"I$Open",          /* Open Existing File */
	"I$MakDir",        /* Make Directory File */
	"I$ChgDir",        /* Change Default Directory */
	"I$Delete",        /* Delete File */
	"I$Seek",          /* Change Current Position */
	"I$Read",          /* Read Data */
	"I$Write",         /* Write Data */
	"I$ReadLn",        /* Read Line of ASCII Data */
	"I$WritLn",        /* Write Line of ASCII Data */
	"I$GetStt",        /* Get Path Status */
	"I$SetStt",        /* Set Path Status */
	"I$Close",         /* Close Path */
	"I$DeletX"         /* Delete from current exec dir */
};


static offs_t dgnbeta_dasm_override(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options)
{
	unsigned call;
	unsigned result = 0;

	if ((oprom[0] == 0x10) && (oprom[1] == 0x3F))
	{
		call = oprom[2];
		if ((call < ARRAY_LENGTH(os9syscalls)) && os9syscalls[call])
		{
			sprintf(buffer, "OS9   %s", os9syscalls[call]);
			result = 3;
		}
	}
	return result;
}

static void execute_beta_dat_log(running_machine &machine, int ref, int params, const char *param[])
{
	dgn_beta_state *state = machine.driver_data<dgn_beta_state>();
	state->m_LogDatWrites=!state->m_LogDatWrites;

	debug_console_printf(machine, "DAT register write info set : %d\n",state->m_LogDatWrites);
}

static void execute_beta_key_dump(running_machine &machine, int ref, int params, const char *param[])
{
	dgn_beta_state *state = machine.driver_data<dgn_beta_state>();
	int Idx;

	for(Idx=0;Idx<NoKeyrows;Idx++)
	{
		debug_console_printf(machine, "KeyRow[%d]=%2X\n",Idx,state->m_Keyboard[Idx]);
	}
}
