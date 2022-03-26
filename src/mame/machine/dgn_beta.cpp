// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

  machine/dgn_beta.cpp

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

    Major track tracing exercise on scans of bare beta board, reveal where a
    whole bunch of the PIA lines go, especially the IRQs, most of them go back
    to the IRQ line on the main CPU.

  2005-12-07

    First booted to OS9 prompt, did not execute startup scripts.

  2005-12-08

    Fixed density setting on WD2797, so density of read data is now
    correctly set as required by OS-9. This was the reason startup
    script was not being executed as Beta disks have a single density
    boot track, however the rest of the disk is double density.
    Booted completely to OS-9, including running startup script.

  2006-09-27

    Clean up of IRQ/FIRQ handling code allows correct booting again.

***************************************************************************/

#include "emu.h"
#include "includes/dgn_beta.h"

#include "includes/coco.h" // for CoCo OS-9 disassembler enhancements

#include "cpu/m6809/m6809.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/mos6551.h"
#include "machine/ram.h"

#include "debug/debugcon.h"
#include "debugger.h"

#include <cmath>
#include <functional>

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


//static int DMA_NMI;               /* DMA cpu has received an NMI */

#define INVALID_KEYROW  -1          /* no keyrow selected */
#define NO_KEY_PRESSED  0x7F        /* returned by hardware if no key pressed */

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
// this should probably be considered a hack !
//

void dgn_beta_state::UpdateBanks(int first, int last)
{
	address_space &space_0 = m_maincpu->space(AS_PROGRAM);
	address_space &space_1 = m_dmacpu->space(AS_PROGRAM);
	int                 Page;
	uint8_t             *readbank;
	int                 bank_start;
	int                 bank_end;
	int                 MapPage;

	LOG_BANK_UPDATE(("\n\n%s Updating banks %d to %d\n", machine().describe_context(), first, last));
	for(Page=first;Page<=last;Page++)
	{
		bank_start  = Page < 16 ? Page * 0x1000 : 0xff00;
		bank_end    = Page < 15 ? bank_start + 0xfff : Page == 15 ? 0xfbff : 0xffff;

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
			uint8_t *base;
			if (!is_last_page(Page))
			{
				base = &m_ram->pointer()[MapPage*RamPageSize];
				if(m_LogDatWrites)
					machine().debugger().console().printf("Mapping page %X, pageno=%X, mess_ram)[%X]\n",Page,MapPage,(MapPage*RamPageSize));
			}
			else
			{
				base = &m_ram->pointer()[(MapPage*RamPageSize)-256];
				logerror("Error RAM in Last page !\n");
			}
			space_0.install_ram(bank_start, bank_end, base);
			space_1.install_ram(bank_start, bank_end, base);
		}
		else                    // Block is rom, or undefined
		{
			uint8_t *base;
			if (MapPage>0xfB)
			{
				if (Page!=IOPage+1)
					base=&m_system_rom[(MapPage-0xFC)*0x1000];
				else
					base=&m_system_rom[0x3F00];
			}
			else
				base=m_system_rom;

			space_0.install_rom(bank_start, bank_end, base);
			space_1.install_rom(bank_start, bank_end, base);
			space_0.unmap_write(bank_start, bank_end);
			space_1.unmap_write(bank_start, bank_end);
		}

		LOG_BANK_UPDATE(("UpdateBanks:MapPage=$%02X readbank=$%X\n",MapPage,(int)(uintptr_t)readbank));
		LOG_BANK_UPDATE(("PageRegsSet Task=%X Page=%x\n",m_TaskReg,Page));
		//LOG_BANK_UPDATE(("%X)\n",membank(Page+1)));
		LOG_BANK_UPDATE(("memory_install_write8_handler CPU=0\n"));
		LOG_BANK_UPDATE(("memory_install_write8_handler CPU=1\n"));
	}
}

//
void dgn_beta_state::SetDefaultTask()
{
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
uint8_t dgn_beta_state::dgn_beta_page_r(offs_t offset)
{
	return m_PageRegs[m_PIATaskReg][offset].value;
}

// Write to a page register, writes to the register, and then checks to see
// if memory banking is active, if it is, it calls UpdateBanks, to actually
// setup the mappings.

void dgn_beta_state::dgn_beta_page_w(offs_t offset, uint8_t data)
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
uint8_t dgn_beta_state::d_pia0_pa_r()
{
	// The hardware has pullup resistors on port A.
	return 0xff;
}

void dgn_beta_state::d_pia0_pa_w(uint8_t data)
{
}

uint8_t dgn_beta_state::d_pia0_pb_r()
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

void dgn_beta_state::d_pia0_pb_w(uint8_t data)
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
		if (VERBOSE) machine().debugger().console().printf("rowshifter clocked, value=%3X, RowNo=%d, Keyrow=%2X\n",m_RowShifter,RowNo,m_Keyrow);
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

uint8_t dgn_beta_state::d_pia1_pa_r()
{
	// The hardware has pullup resistors on port A.
	return 0xff;
}

void dgn_beta_state::d_pia1_pa_w(uint8_t data)
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
		m_dmacpu->set_input_line(INPUT_LINE_HALT, HALT_DMA);

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

	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(0);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(0);
	if (m_floppy2->get_device()) m_floppy2->get_device()->mon_w(0);
	if (m_floppy3->get_device()) m_floppy3->get_device()->mon_w(0);

	// not connected: bit 5 = ENP
	m_fdc->dden_w(BIT(data, 6));
	LOG_DISK(("Set density %s\n", BIT(data, 6) ? "low" : "high"));
}

uint8_t dgn_beta_state::d_pia1_pb_r()
{
	return 0;
}

void dgn_beta_state::d_pia1_pb_w(uint8_t data)
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
			m_dmacpu->yield();
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
        VSYNC interrupt CB2
*/
uint8_t dgn_beta_state::d_pia2_pa_r()
{
	// The hardware has pullup resistors on port A.
	return 0xff;
}

void dgn_beta_state::d_pia2_pa_w(uint8_t data)
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
			m_dmacpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
			logerror("device_yield()\n");
			m_dmacpu->yield();    /* Let DMA CPU run */
		}
		else
		{
			m_dmacpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
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

	// Mapping was enabled or disabled, select appropriate task reg
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

uint8_t dgn_beta_state::d_pia2_pb_r()
{
	return 0;
}

void dgn_beta_state::d_pia2_pb_w(uint8_t data)
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

/************************************ Recalculate CPU interrupts ****************************/
/* CPU 0 */
void dgn_beta_state::cpu0_recalc_irq(int state)
{
	uint8_t pia0_irq_a = m_pia_0->irq_a_state();
	uint8_t pia1_irq_a = m_pia_1->irq_a_state();
	uint8_t pia1_irq_b = m_pia_1->irq_b_state();
	uint8_t pia2_irq_a = m_pia_2->irq_a_state();
	uint8_t pia2_irq_b = m_pia_2->irq_b_state();
	uint8_t IRQ;

	if (pia0_irq_a || pia1_irq_a || pia1_irq_b || pia2_irq_a || pia2_irq_b)
		IRQ = ASSERT_LINE;
	else
		IRQ = CLEAR_LINE;

	m_maincpu->set_input_line(M6809_IRQ_LINE, IRQ);
	LOG_INTS(("cpu0 IRQ : %d\n", IRQ));
}

void dgn_beta_state::cpu0_recalc_firq(int state)
{
	uint8_t pia0_irq_b = m_pia_0->irq_b_state();
	uint8_t FIRQ;

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
	m_dmacpu->set_input_line(M6809_FIRQ_LINE, state);
	LOG_INTS(("cpu1 FIRQ : %d\n",state));
}

/********************************************************************************************/
/* Dragon Beta onboard FDC */
/********************************************************************************************/

/* The INTRQ line goes through pia2 ca1, in exactly the same way as DRQ from DragonDos does */
WRITE_LINE_MEMBER( dgn_beta_state::dgnbeta_fdc_intrq_w )
{
	LOG_DISK(("dgnbeta_fdc_intrq_w(%d)\n", state));

	if(m_wd2797_written)
		m_pia_2->ca1_w(state);
}

/* DRQ is routed through various logic to the FIRQ interrupt line on *BOTH* CPUs */
WRITE_LINE_MEMBER( dgn_beta_state::dgnbeta_fdc_drq_w )
{
	LOG_DISK(("dgnbeta_fdc_drq_w(%d)\n", state));
	cpu1_recalc_firq(state);
}

uint8_t dgn_beta_state::dgnbeta_wd2797_r(offs_t offset)
{
	return m_fdc->read(offset & 0x03);
}

void dgn_beta_state::dgnbeta_wd2797_w(offs_t offset, uint8_t data)
{
	m_wd2797_written = 1;
	m_fdc->write(offset & 0x03, data);
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

/* VBlank interrupt */
void dgn_beta_state::dgn_beta_frame_interrupt (int data)
{
	/* Set PIA line, so it recognises interrupt */
	if (!data)
		m_pia_2->cb2_w(ASSERT_LINE);
	else
		m_pia_2->cb2_w(CLEAR_LINE);

//    LOG_VIDEO(("Vblank\n"));
	ScanInKeyboard();
}

void dgn_beta_state::dgn_beta_line_interrupt(int data)
{
#if 0
	/* Set PIA line, so it recognises interrupt */
	if (data)
	{
		m_pia_0->cb1_w(ASSERT_LINE);
	}
	else
	{
		m_pia_0->cb1_w(CLEAR_LINE);
	}
#endif
}


/********************************* Machine/Driver Initialization ****************************************/
void dgn_beta_state::machine_reset()
{
	logerror("dgn_beta_state::machine_reset()\n");

	/* Make sure CPU 1 is started out halted ! */
	m_dmacpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	/* Reset to task 0, and map banks disabled, so standard memory map */
	/* with ram at $0000-$BFFF, ROM at $C000-FBFF, IO at $FC00-$FEFF */
	/* and ROM at $FF00-$FFFF */
	m_TaskReg = 0;
	m_PIATaskReg = 0;
	m_EnableMapRegs = 0;
	memset(m_PageRegs, 0, sizeof(m_PageRegs));    /* Reset page registers to 0 */
	SetDefaultTask();

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

	m_wd2797_written=0;

	m_maincpu->reset();
}

void dgn_beta_state::machine_start()
{
	logerror("MACHINE_START( dgnbeta )\n");

	/* setup debug commands */
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("beta_dat_log", CMDFLAG_NONE, 0, 0, std::bind(&dgn_beta_state::execute_beta_dat_log, this, _1));
		machine().debugger().console().register_command("beta_key_dump", CMDFLAG_NONE, 0, 0, std::bind(&dgn_beta_state::execute_beta_key_dump, this, _1));
	}
	m_LogDatWrites = false;
	m_wd2797_written = 0;
}


/***************************************************************************
  OS9 Syscalls for disassembly
****************************************************************************/

offs_t dgn_beta_state::dgnbeta_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	return coco_state::os9_dasm_override(stream, pc, opcodes, params);
}

void dgn_beta_state::execute_beta_dat_log(const std::vector<std::string> &params)
{
	m_LogDatWrites = !m_LogDatWrites;

	machine().debugger().console().printf("DAT register write info set : %d\n", m_LogDatWrites);
}

void dgn_beta_state::execute_beta_key_dump(const std::vector<std::string> &params)
{
	for (int idx = 0; idx < NoKeyrows; idx++)
	{
		machine().debugger().console().printf("KeyRow[%d]=%2X\n", idx, m_Keyboard[idx]);
	}
}
