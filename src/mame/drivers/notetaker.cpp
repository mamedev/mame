// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/* Xerox NoteTaker, 1978
 * Driver by Jonathan Gevaryahu

 *  Notetaker Team At Xerox PARC 1976-1980:
     Alan Kay - Team Lead
     Bruce Horn - BIOS code and more
     Ted Kaehler - SmallTalk-76 code porting[2] and more ( http://tedkaehler.weather-dimensions.com/us/ted/index.html )
     Dan Ingalls - BitBlt engine and SmallTalk kernel and more[3]
     Doug Fairbairn - NoteTaker Hardware/Electronics Design ( http://www.computerhistory.org/atchm/author/dfairbairn/ )
     James Leung - NoteTaker Hardware/Electronics Design
     Ron Freeman - NoteTaker Hardware/Electronics Design
     <there are probably others I've missed>

 * History of the machine can be found at http://freudenbergs.de/bert/publications/Ingalls-2014-Smalltalk78.pdf

 * The notetaker has an 8-slot backplane, with the following cards in it:
   * I/O Processor card (8086@8Mhz, 8259pic, 4k ROM, Keyboard UART, DAC1200 (multiplexed to 2 channels))
   * Emulation Processor card (8086@5Mhz, 8259pic, 4k of local RAM with Parity check logic)
   * Disk/Display card (WD1791 FDC, CRT5027 CRTC, EIA UART, AD571 ADC, 8->1 Analog Multiplexer)
   * Memory Control Module \_ (bus control, buffering, refresh, Parity/ECC/Syndrome logic lives on these boards)
   * Memory Data Module    /
   * Memory Storage Module x2 (the 4116 DRAMs live on these boards)
   * Battery Module *OR* debugger module type A or B (debugger module has an i8255 on it for alto<->notetaker comms, and allows alto to halt the cpus [type A and B can debug either the emulator cpu or the iocpu respectively] and dump registers to alto screen, etc)

 * Prototypes only, 10 units[2] manufactured 1978-1980
   Known surviving units:
   * One at CHM (missing? mouse, no media)
   * One at Xerox Museum at PARC (with mouse and 2 floppies, floppies were not imaged to the best of my knowledge)

 * The NoteTaker used the BitBlt graphical operation (from SmallTalk-76) to do most graphical functions, in order to fit the SmallTalk code and programs within 256K of RAM[2]. The actual BitBlt code lives in ROM[3].

 * As far as I am aware, no media (world disks/boot disks) for the NoteTaker have survived (except maybe the two disks at Xerox Museum at PARC), but an incomplete dump of the Smalltalk-76 'world' which was used to bootstrap Smalltalk-78 originally did survive on the Alto disks at CHM

 * We are missing the dump for the i8748 Keyboard MCU which does row-column scanning and mouse quadrature reading, and talks to the main system via serial

 * see http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker for additional information
 * see http://xeroxalto.computerhistory.org/Filene/Smalltalk-76/ for the smalltalk-76 dump
 * see http://xeroxalto.computerhistory.org/Indigo/BasicDisks/Smalltalk14.bfs!1_/ for more notetaker/smalltalk related files, including SmallTalk-80 files based on the notetaker smalltalk-78

 References:
 * [1] http://freudenbergs.de/bert/publications/Ingalls-2014-Smalltalk78.pdf
 * [2] "Smalltalk and Object Orientation: An Introduction" By John Hunt, pages 45-46 [ISBN 978-3-540-76115-0]
 * [3] http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790620_Z-IOP_1.5_ls.pdf
 * [4] http://xeroxalto.computerhistory.org/Filene/Smalltalk-76/
 * [5] http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790118_NoteTaker_System_Manual.pdf

TODO: everything below.
* Get smalltalk-78 loaded as a rom and forced into ram on startup, since no boot disks have survived (or if any survived, they are not dumped)

* Harris 6402 keyboard UART (within keyboard, next to MCU)
* The harris 6402 UART is pin compatible with WD TR1865 UART, as well as the AY-3-1015A/D
   (These are 5v-only versions of the older WD TR1602 and AY-5-1013 parts which used 5v and 12v)
* HLE for the missing i8748[5] MCU in the keyboard which reads the mouse quadratures and buttons and talks serially to the Keyboard UART
* floppy controller wd1791 and its interrupt
  According to [3] and [5] the format is double density/MFM, 128 bytes per sector, 16 sectors per track, 1 or 2 sided, for 170K or 340K per disk. Drive spins at 300RPM.
* According to the schematics, we're missing an 82s147 DISKSEP.PROM used as a data separator
* hook up the DiskInt from the wd1791 either using m_fdc->intrq_r() polling or using device lines (latter is a much better idea)

WIP:
* crt5027 video controller - the iocpu side is hooked up, screen drawing 'works' but is scrambled due to not emulating the clock chain halting and clock changing yet. The crt5027 core also needs the odd/even interrupt hooked up, and proper interlace support as well as clock change/screen resize support (down to DC/no clock, which I guess should be a 1x1 single black pixel!)
* pic8259 interrupt controller - this is attached as a device, but only the vblank interrupt is hooked to it yet.
* Harris 6402 serial/EIA UART - connected to iocpu, other end isn't connected anywhere, interrupt is not connected
* Harris 6402 keyboard UART (within notetaker) - connected to iocpu, other end isn't connected anywhere, interrupt is not connected
* we're missing a dump of the 82s126 SETMEMRQ PROM used to handle memory arbitration between the crtc and the rest of the system, but the equations are on the schematic and I'm planning to regenerate the prom contents from that, see ROM_LOAD section
* The DAC, its FIFO and its timer are hooked up and the v2.0 bios beeps, but the stereo latches are not hooked up at all, DAC is treated as mono for now

DONE:
* i/o cpu i/o area needs the memory map worked out per the schematics - done
* figure out the correct memory maps for the 256kB of shared ram, and what part of ram constitutes the framebuffer - done
  - 256k of shared ram maps at 00000-3ffff for both cpus with special mem regs at fffec,fffee. the ram mirrors 4 times on the emulatorcpu only, iocpu the 40000-fffff area is open bus.
  - framebuffer, at least for bios 1.5, lives from 0x4000-0xd5ff, exactly 640x480 pixels 1bpp, interlaced (even? plane is at 4000-8aff, odd? plane is at 8b00-d5ff); however the starting address of the framebuffer is configurable to any address within the 0x0000-0x1ffff range? (this exact range is unclear)
* figure out how the emulation-cpu boots and where its 8k of local ram maps to - done
  - both cpus boot, reset and system int controls are accessed at fffea from either cpu; emulatorcpu's 8k of ram lives at the beginning of its address space, but can be disabled in favor of mainram at the same addressses
*/

#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "machine/ay31015.h"
#include "video/tms9927.h"
#include "sound/dac.h"
#include "machine/wd_fdc.h"

class notetaker_state : public driver_device
{
public:
	enum
	{
		TIMER_FIFOCLK,
	};

	notetaker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_iocpu(*this, "iocpu"),
		m_iopic(*this, "iopic8259"),
		m_kbduart(*this, "kbduart"),
		m_eiauart(*this, "eiauart"),
		m_crtc(*this, "crt5027"),
		m_dac(*this, "dac"),
		m_fdc(*this, "wd1791"),
		m_floppy0(*this, "wd1791:0"),
		m_floppy(NULL)
	{
	}
// devices
	required_device<cpu_device> m_iocpu;
	required_device<pic8259_device> m_iopic;
	required_device<ay31015_device> m_kbduart;
	required_device<ay31015_device> m_eiauart;
	required_device<crt5027_device> m_crtc;
	required_device<dac_device> m_dac;
	required_device<fd1791_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	floppy_image_device *m_floppy;

//declarations
	// screen
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	// basic io
	DECLARE_WRITE16_MEMBER(IPConReg_w);
	DECLARE_WRITE16_MEMBER(FIFOReg_w);
	DECLARE_WRITE16_MEMBER(FIFOBus_w);
	DECLARE_WRITE16_MEMBER(DiskReg_w);
	DECLARE_WRITE16_MEMBER(LoadDispAddr_w);

	// uarts
	DECLARE_READ16_MEMBER(ReadKeyData_r);
	DECLARE_READ16_MEMBER(ReadOPStatus_r);
	DECLARE_WRITE16_MEMBER(LoadKeyData_w);
	DECLARE_WRITE16_MEMBER(LoadKeyCtlReg_w);
	DECLARE_WRITE16_MEMBER(KeyDataReset_w);
	DECLARE_WRITE16_MEMBER(KeyChipReset_w);
	DECLARE_READ16_MEMBER(ReadEIAData_r);
	DECLARE_READ16_MEMBER(ReadEIAStatus_r);
	DECLARE_WRITE16_MEMBER(LoadEIAData_w);
	DECLARE_WRITE16_MEMBER(LoadEIACtlReg_w);
	DECLARE_WRITE16_MEMBER(EIADataReset_w);
	DECLARE_WRITE16_MEMBER(EIAChipReset_w);
	// mem map stuff
	DECLARE_READ16_MEMBER(iocpu_r);
	DECLARE_WRITE16_MEMBER(iocpu_w);
	DECLARE_DRIVER_INIT(notetakr);
	//variables
	//  IPConReg
	UINT8 m_BootSeqDone;
	UINT8 m_ProcLock;
	UINT8 m_CharCtr;
	UINT8 m_DisableROM;
	UINT8 m_CorrOn_q;
	UINT8 m_LedInd6;
	UINT8 m_LedInd7;
	UINT8 m_LedInd8;
	//  FIFOReg
	UINT8 m_TabletYOn;
	UINT8 m_TabletXOn;
	UINT8 m_FrSel2;
	UINT8 m_FrSel1;
	UINT8 m_FrSel0;
	UINT8 m_SHConB;
	UINT8 m_SHConA;
	UINT8 m_SetSH;
	// DiskReg
	UINT8 m_ADCSpd0;
	UINT8 m_ADCSpd1;
	UINT8 m_StopWordClock_q;
	UINT8 m_ClrDiskCont_q;
	UINT8 m_ProgBitClk1;
	UINT8 m_ProgBitClk2;
	UINT8 m_ProgBitClk3;
	UINT8 m_AnSel4;
	UINT8 m_AnSel2;
	UINT8 m_AnSel1;
	UINT8 m_DriveSel1;
	UINT8 m_DriveSel2;
	UINT8 m_DriveSel3;
	UINT8 m_SideSelect;
	UINT8 m_Disk5VOn;
	UINT8 m_Disk12VOn;
	// output fifo, for DAC
	UINT16 m_outfifo[16]; // technically three 74LS225 5bit*16stage FIFO chips, arranged as a 16 stage, 12-bit wide fifo (one bit unused per chip)
	UINT8 m_outfifo_count;
	UINT8 m_outfifo_tail_ptr;
	UINT8 m_outfifo_head_ptr;
	// fifo timer
	emu_timer *m_FIFO_timer;
	TIMER_CALLBACK_MEMBER(timer_fifoclk);
	// framebuffer display starting address
	UINT16 m_DispAddr;

// separate cpu resets
	void ip_reset();
	void ep_reset();

// overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

void notetaker_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_FIFOCLK:
		timer_fifoclk(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in notetaker_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(notetaker_state::timer_fifoclk)
{
	UINT16 data;
	//pop a value off the fifo and send it to the dac.
#ifdef FIFO_VERBOSE
	if (m_outfifo_count == 0) logerror("output fifo is EMPTY! repeating previous sample!\n");
#endif
	data = m_outfifo[m_outfifo_tail_ptr];
	// if fifo is empty (tail ptr == head ptr), do not increment the tail ptr, otherwise do.
	if (m_outfifo_count > 0)
	{
		m_outfifo_tail_ptr++;
		m_outfifo_count--;
	}
	m_outfifo_tail_ptr&=0xF;
	m_dac->write_unsigned16(data);
	m_FIFO_timer->adjust(attotime::from_hz(((XTAL_960kHz/10)/4)/((m_FrSel0<<3)+(m_FrSel1<<2)+(m_FrSel2<<1)+1))); // FIFO timer is clocked by 960khz divided by 10 (74ls162 decade counter), divided by 4 (mc14568B with divider 1 pins set to 4), divided by 1,3,5,7,9,11,13,15 (or 0,2,4,6,8,10,12,14?)
}

UINT32 notetaker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// have to figure out what resolution we're drawing to here and draw appropriately to screen
	// code borrowed/stolen from video/mac.cpp
	UINT32 video_base;
	UINT16 word;
	UINT16 *line;
	int y, x, b;

	video_base = (m_DispAddr << 3)&0x1FFFF;
#ifdef DEBUG_VIDEO
	logerror("Video Base = 0x%05x\n", video_base);
#endif
	const UINT16 *video_ram_field1 = (UINT16 *)(memregion("mainram")->base()+video_base);
	const UINT16 *video_ram_field2 = (UINT16 *)(memregion("mainram")->base()+video_base+0x4B00);

	for (y = 0; y < 480; y++)
	{
		line = &bitmap.pix16(y);

		for (x = 0; x < 640; x += 16)
		{
			if ((y&1)==0) word = *(video_ram_field1++); else word = *(video_ram_field2++);
			for (b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

WRITE16_MEMBER(notetaker_state::IPConReg_w)
{
	m_BootSeqDone = (data&0x80)?1:0;
	m_ProcLock = (data&0x40)?1:0; // bus lock for this processor (hold other processor in wait state)
	m_CharCtr = (data&0x20)?1:0; // battery charge control (incorrectly called 'Char counter' in source code)
	m_DisableROM = (data&0x10)?1:0; // disable rom at 0000-0fff
	m_CorrOn_q = (data&0x08)?1:0; // CorrectionOn (ECC correction enabled); also LedInd5
	m_LedInd6 = (data&0x04)?1:0;
	m_LedInd7 = (data&0x02)?1:0;
	m_LedInd8 = (data&0x01)?1:0;
	popmessage("LEDS: CR1: %d, CR2: %d, CR3: %d, CR4: %d", (data&0x04)>>2, (data&0x08)>>3, (data&0x02)>>1, (data&0x01)); // cr1 and 2 are in the reverse order as expected, according to the schematic
}

/* handlers for the two system hd6402s (ay-5-1013 equivalent) */
/* * Keyboard hd6402 */
READ16_MEMBER( notetaker_state::ReadKeyData_r )
{
	return 0xFF00||m_kbduart->get_received_data();
}

READ16_MEMBER( notetaker_state::ReadOPStatus_r ) // 74ls368 hex inverter at #l7 provides 4 bits, inverted
{
	UINT16 data = 0xFFF0;
	data |= (m_outfifo_count >= 1) ? 0 : 0x08; // m_FIFOOutRdy is true if the fifo has at least 1 word in it, false otherwise
	data |= (m_outfifo_count < 16) ? 0 : 0x04; // m_FIFOInRdy is true if the fifo has less than 16 words in it, false otherwise
	// note /SWE is permanently enabled, so we don't enable it here for HD6402 reading
	data |= m_kbduart->get_output_pin(AY31015_DAV ) ? 0 : 0x02; // DR - pin 19
	data |= m_kbduart->get_output_pin(AY31015_TBMT) ? 0 : 0x01; // TBRE - pin 22
#ifdef DEBUG_READOPSTATUS
	logerror("ReadOPStatus read, returning %04x\n", data);
#endif
	return data;
}

WRITE16_MEMBER( notetaker_state::LoadKeyData_w )
{
	m_kbduart->set_transmit_data(data&0xFF);
}

WRITE16_MEMBER( notetaker_state::LoadKeyCtlReg_w )
{
	m_kbduart->set_input_pin(AY31015_CS, 0);
	m_kbduart->set_input_pin(AY31015_NP,  BIT(data, 4)); // PI - pin 35
	m_kbduart->set_input_pin(AY31015_TSB, BIT(data, 3)); // SBS - pin 36
	m_kbduart->set_input_pin(AY31015_NB2, BIT(data, 2)); // CLS2 - pin 37
	m_kbduart->set_input_pin(AY31015_NB1, BIT(data, 1)); // CLS1 - pin 38
	m_kbduart->set_input_pin(AY31015_EPS, BIT(data, 0)); // EPE - pin 39
	m_kbduart->set_input_pin(AY31015_CS, 1);
}

WRITE16_MEMBER( notetaker_state::KeyDataReset_w )
{
	m_kbduart->set_input_pin(AY31015_RDAV, 0); // DDR - pin 18
	m_kbduart->set_input_pin(AY31015_RDAV, 1); // ''
}

WRITE16_MEMBER( notetaker_state::KeyChipReset_w )
{
	m_kbduart->set_input_pin(AY31015_XR, 0); // MR - pin 21
	m_kbduart->set_input_pin(AY31015_XR, 1); // ''
}

/* FIFO (DAC) Stuff and ADC stuff */
WRITE16_MEMBER(notetaker_state::FIFOReg_w)
{
	m_SetSH = (data&0x8000)?1:0;
	m_SHConA = (data&0x4000)?1:0;
	m_SHConB = (data&0x2000)?1:0;
	m_FrSel0 = (data&0x1000)?1:0;
	m_FrSel1 = (data&0x0800)?1:0;
	m_FrSel2 = (data&0x0400)?1:0;
	m_TabletXOn = (data&0x0200)?1:0;
	m_TabletYOn = (data&0x0100)?1:0;
	m_FIFO_timer->adjust(attotime::from_hz(((XTAL_960kHz/10)/4)/((m_FrSel0<<3)+(m_FrSel1<<2)+(m_FrSel2<<1)+1))); // FIFO timer is clocked by 960khz divided by 10 (74ls162 decade counter), divided by 4 (mc14568B with divider 1 pins set to 4), divided by 1,3,5,7,9,11,13,15 (or off,2,4,6,8,10,12,14?)
	logerror("Write to 0x60 FIFOReg_w of %04x; fifo timer set to %d hz\n", data, (((XTAL_960kHz/10)/4)/((m_FrSel0<<3)+(m_FrSel1<<2)+(m_FrSel2<<1)+1)));
}

WRITE16_MEMBER(notetaker_state::FIFOBus_w)
{
	if (m_outfifo_count == 16)
	{
#ifdef SPC_LOG_DSP
		logerror("outfifo was full, write ignored!\n");
#endif
		return;
	}
	m_outfifo[m_outfifo_head_ptr] = data;
	m_outfifo_head_ptr++;
	m_outfifo_count++;
	m_outfifo_head_ptr&=0xF;
}

WRITE16_MEMBER( notetaker_state::DiskReg_w )
{
	// See http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19781023_More_NoteTaker_IO_Information.pdf but note that bit 12 (called bit 3 in documentation) was changed between oct 1978 and 1979 to reset the disk controller digital-PLL as ClrDiskCont' rather than acting as ProgBitClk0, which is permanently wired high instead, meaning only the 4.5Mhz - 18Mhz dot clocks are available for the CRTC.
	m_ADCSpd0 = (data&0x8000)?1:0;
	m_ADCSpd1 = (data&0x4000)?1:0;
	m_StopWordClock_q = (data&0x2000)?1:0;
	//if ((!(m_ClrDiskCont_q)) && (data&0x1000)) m_floppy->device_reset(); // reset on rising edge
	m_ClrDiskCont_q = (data&0x1000)?1:0; // originally ProgBitClk0, but co-opted later to reset the FDC's external PLL
	m_ProgBitClk1 = (data&0x0800)?1:0;
	m_ProgBitClk2 = (data&0x0400)?1:0;
	m_ProgBitClk3 = (data&0x0200)?1:0;
	m_AnSel4 = (data&0x0100)?1:0;
	m_AnSel2 = (data&0x80)?1:0;
	m_AnSel1 = (data&0x40)?1:0;
	m_DriveSel1 = (data&0x20)?1:0;
	m_DriveSel2 = (data&0x10)?1:0; // drive 2 not present on hardware, but could work if present
	m_DriveSel3 = (data&0x08)?1:0; // drive 3 not present on hardware, but could work if present
	m_SideSelect = (data&0x04)?1:0;
	m_Disk5VOn = (data&0x02)?1:0;
	m_Disk12VOn = (data&0x01)?1:0;

	// ADC stuff
	//TODO

	// FDC stuff
	// first handle the motor stuff; we'll clobber whatever was in m_floppy, then reset it to what it should be
	m_floppy = m_floppy0->get_device();
	m_floppy->mon_w(!(m_Disk5VOn && m_Disk12VOn)); // Disk5VOn and 12VOn can be thought of as a crude MotorOn signal as the motor won't run with either? of them missing.
	//m_floppy = m_floppy0->get_device();
	//m_floppy->mon_w(!(m_Disk5VOn && m_Disk12VOn)); // Disk5VOn and 12VOn can be thought of as a crude MotorOn signal as the motor won't run with either? of them missing.
	//m_floppy = m_floppy0->get_device();
	//m_floppy->mon_w(!(m_Disk5VOn && m_Disk12VOn)); // Disk5VOn and 12VOn can be thought of as a crude MotorOn signal as the motor won't run with either? of them missing.
	// now restore m_floppy state to what it should be
	if (m_DriveSel1) m_floppy = m_floppy0->get_device();
	//else if (m_DriveSel2) m_floppy = m_floppy1->get_device();
	//else if (m_DriveSel3) m_floppy = m_floppy2->get_device();
	else m_floppy = nullptr;
	m_fdc->set_floppy(m_floppy); // select the floppy
	if (m_floppy)
	{
		m_floppy->ss_w(m_SideSelect);
	}
	// Disk5VOn and 12VOn can be thought of as a crude MotorOn signal as the motor won't run with either? of them missing.
	//m_floppy0->mon_w(!(m_Disk5VOn && m_Disk12VOn));
	//m_floppy1->mon_w(!(m_Disk5VOn && m_Disk12VOn));
	//m_floppy2->mon_w(!(m_Disk5VOn && m_Disk12VOn));

	// CRTC clock rate stuff
	//TODO
}

WRITE16_MEMBER( notetaker_state::LoadDispAddr_w )
{
	m_DispAddr = data;
	// for future low level emulation: clear the current counter position here as well, as well as empty/reset the display fifo, and the setmemrq state.
}

/* EIA hd6402 */
READ16_MEMBER( notetaker_state::ReadEIAData_r )
{
	return 0xFF00||m_eiauart->get_received_data();
}

READ16_MEMBER( notetaker_state::ReadEIAStatus_r ) // 74ls368 hex inverter at #f1 provides 2 bits, inverted
{
	UINT16 data = 0xFFFC;
	// note /SWE is permanently enabled, so we don't enable it here for HD6402 reading
	data |= m_eiauart->get_output_pin(AY31015_DAV ) ? 0 : 0x02; // DR - pin 19
	data |= m_eiauart->get_output_pin(AY31015_TBMT) ? 0 : 0x01; // TBRE - pin 22
	return data;
}

WRITE16_MEMBER( notetaker_state::LoadEIAData_w )
{
	m_eiauart->set_transmit_data(data&0xFF);
}

WRITE16_MEMBER( notetaker_state::LoadEIACtlReg_w )
{
	m_eiauart->set_input_pin(AY31015_CS, 0);
	m_eiauart->set_input_pin(AY31015_NP,  BIT(data, 4)); // PI - pin 35
	m_eiauart->set_input_pin(AY31015_TSB, BIT(data, 3)); // SBS - pin 36
	m_eiauart->set_input_pin(AY31015_NB2, BIT(data, 2)); // CLS2 - pin 37
	m_eiauart->set_input_pin(AY31015_NB1, BIT(data, 1)); // CLS1 - pin 38
	m_eiauart->set_input_pin(AY31015_EPS, BIT(data, 0)); // EPE - pin 39
	m_eiauart->set_input_pin(AY31015_CS, 1);
}

WRITE16_MEMBER( notetaker_state::EIADataReset_w )
{
	m_eiauart->set_input_pin(AY31015_RDAV, 0); // DDR - pin 18
	m_eiauart->set_input_pin(AY31015_RDAV, 1); // ''
}

WRITE16_MEMBER( notetaker_state::EIAChipReset_w )
{
	m_eiauart->set_input_pin(AY31015_XR, 0); // MR - pin 21
	m_eiauart->set_input_pin(AY31015_XR, 1); // ''
}


/* These next two members are memory map related for the iocpu */
READ16_MEMBER(notetaker_state::iocpu_r)
{
	UINT16 *rom = (UINT16 *)(memregion("iocpu")->base());
	rom += 0x7f800;
	UINT16 *ram = (UINT16 *)(memregion("mainram")->base());
	if ( (m_BootSeqDone == 0) || ((m_DisableROM == 0) && ((offset&0x7F800) == 0)) )
	{
		rom += (offset&0x7FF);
		return *rom;
	}
	else
	{
		// are we in the FFFE8-FFFEF area where the parity/int/reset/etc stuff lives?
		if (offset >= 0x7fff4) { logerror("attempt to read processor control regs at %d\n", offset<<1); return 0xFFFF; }
		ram += offset;
		return *ram;
	}
}

WRITE16_MEMBER(notetaker_state::iocpu_w)
{
	//UINT16 tempword;
	UINT16 *ram = (UINT16 *)(memregion("mainram")->base());
	if ( (m_BootSeqDone == 0) || ((m_DisableROM == 0) && ((offset&0x7F800) == 0)) )
	{
		logerror("attempt to write %04X to ROM-mapped area at %06X ignored\n", data, offset<<1);
		return;
	}
	// are we in the FFFE8-FFFEF area where the parity/int/reset/etc stuff lives?
	if (offset >= 0x7fff4) { logerror("attempt to write processor control regs at %d with %02X ignored\n", offset<<1, data); }
	COMBINE_DATA(&ram[offset]);
}

/* Address map comes from http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/schematics/19790423_Notetaker_IO_Processor.pdf
a19 a18 a17 a16  a15 a14 a13 a12  a11 a10 a9  a8   a7  a6  a5  a4   a3  a2  a1  a0   BootSeqDone  DisableROM
mode 0: (to get the boot vector and for maybe 1 or 2 instructions afterward)
x   x   x   x    x   x   x   x    *   *   *   *    *   *   *   *    *   *   *   *    0            x          R   ROM (writes are open bus)
mode 1: (during most of boot)
0   0   0   0    0   0   0   0    *   *   *   *    *   *   *   *    *   *   *   *    1            0          R   ROM (writes are open bus)
0   0   0   0    0   0   0   1    *   *   *   *    *   *   *   *    *   *   *   *    1            0          RW  RAM
0   0   0   0    0   0   1   *    *   *   *   *    *   *   *   *    *   *   *   *    1            0          RW  RAM
<anything not all zeroes>x   x    x   x   x   x    x   x   x   x    x   x   x   x    1            0          .   Open Bus
mode 2: (during load of the emulatorcpu's firmware to the first 4k of shared ram which is on the emulatorcpu board)
0   0   *   *    *   *   *   *    *   *   *   *    *   *   *   *    *   *   *   *    1            1          RW  RAM
<anything not all zeroes>x   x    x   x   x   x    x   x   x   x    x   x   x   x    1            1          .   Open Bus
   EXCEPT for the following, decoded by the memory address logic board:
1   1   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   0   0   x    1            x          .   Open Bus
1   1   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   0   1   x    1            x          W   FFFEA (Multiprocessor Control (reset/int/boot for each proc; data bits 3,2,1,0 = 0010 means IP, bits 3210 = 0111 means EP; all others ignored.))
1   1   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   1   0   x    1            x          R   FFFEC (Syndrome bits (gnd bit 15, parity bit 14, exp (ECC) bits 13-8, bits 7-0 are 'other' (?maybe highest address bits?)
1   1   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   1   1   x    1            x          R   FFFEE (Parity Error Address: row bits 15-8, column bits 7-0; reading this acknowledges a parity interrupt)

More or less:
BootSeqDone is 0, DisableROM is ignored, mem map is 0x00000-0xfffff reading is the 0x1000-long ROM, repeated every 0x1000 bytes. writing goes nowhere.
BootSeqDone is 1, DisableROM is 0,       mem map is 0x00000-0x00fff reading is the 0x1000-long ROM, remainder of memory map goes to RAM or open bus. writing the ROM area goes nowhere, writing RAM goes to RAM.
BootSeqDone is 1, DisableROM is 1,       mem map is entirely RAM or open bus for both reading and writing.


Emulator cpu mem map:
a19 a18 a17 a16  a15 a14 a13 a12  a11 a10 a9  a8   a7  a6  a5  a4   a3  a2  a1  a0   4KPage0'
x   x   0   0    0   0   0   0    *   *   *   *    *   *   *   *    *   *   *   *    0                       RW  Local (fast) RAM
x   x   0   0    0   0   0   0    *   *   *   *    *   *   *   *    *   *   *   *    1                       RW  System/Shared RAM
x   x   *   *    *   *   *   *    *   *   *   *    *   *   *   *    *   *   *   *    x                       RW  System/Shared RAM
   EXCEPT for the following, decoded by the EP board and superseding above:
1   1   1   1    1   1   1   1    1   1   1   1    1   1   0   x    x   x   x   x    x                       RW  FFFC0-FFFDF (trigger ILLINST interrupt on EP, data ignored?)
   And the following, decoded by the memory address logic board:
1   1   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   0   0   x    x                       .   Open Bus
1   1   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   0   1   x    x                       W   FFFEA (Multiprocessor Control (reset(bit 6)/int(bit 5)/boot(bit 4) for each processor; data bits 3,2,1,0 are 'processor address'; 0010 means IP, 0111 means EP; all others ignored.))
1   1   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   1   0   x    x                       R   FFFEC (Syndrome bits (gnd bit 15, parity bit 14, exp(syndrome) bits 13-8, bits 7-0 are the highest address bits)
1   1   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   1   1   x    x                       R   FFFEE (Parity Error Address: row bits 15-8, column bits 7-0; reading this also acknowledges a parity interrupt)
*/

static ADDRESS_MAP_START(notetaker_iocpu_mem, AS_PROGRAM, 16, notetaker_state)
	/*
	AM_RANGE(0x00000, 0x00fff) AM_ROM AM_REGION("iocpu", 0xff000) // rom is here if either BootSeqDone OR DisableROM are zero. the 1.5 source code and the schematics implies writes here are ignored while rom is enabled; if disablerom is 1 this goes to mainram
	AM_RANGE(0x01000, 0x3ffff) AM_RAM AM_REGION("mainram", 0) // 256k of ram (less 4k), shared between both processors. rom goes here if bootseqdone is 0
	// note 4000-d5ff is the framebuffer for the screen, in two sets of fields for odd/even interlace?
	AM_RANGE(0xff000, 0xfffe7) AM_ROM AM_REGION("iocpu", 0xff000) // rom is only banked in here if bootseqdone is 0, so the reset vector is in the proper place. otherwise the memory control regs live at fffe8-fffef
	//AM_RANGE(0xfffea, 0xfffeb) AM_WRITE(cpuCtl_w);
	//AM_RANGE(0xfffec, 0xfffed) AM_READ(parityErrHi_r);
	//AM_RANGE(0xfffee, 0xfffef) AM_READ(parityErrLo_r);
	AM_RANGE(0xffff0, 0xfffff) AM_ROM AM_REGION("iocpu", 0xffff0)
	*/
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(iocpu_r, iocpu_w) // bypass MAME's memory map system as we need finer grained control
ADDRESS_MAP_END

/* io memory map comes from http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/memos/19790605_Definition_of_8086_Ports.pdf
   and from the schematic at http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/schematics/19790423_Notetaker_IO_Processor.pdf
a19 a18 a17 a16  a15 a14 a13 a12  a11 a10 a9  a8   a7  a6  a5  a4   a3  a2  a1  a0
x   x   x   x    0   x   x   x    x   x   x   0    0   0   0   x    x   x   *   .       RW  IntCon (PIC8259)
x   x   x   x    0   x   x   x    x   x   x   0    0   0   1   x    x   x   x   .       W   IPConReg
x   x   x   x    0   x   x   x    x   x   x   0    0   1   0   x    0   0   0   .       .   KbdInt:Open Bus
x   x   x   x    0   x   x   x    x   x   x   0    0   1   0   x    0   0   1   .       R   KbdInt:ReadKeyData
x   x   x   x    0   x   x   x    x   x   x   0    0   1   0   x    0   1   0   .       R   KbdInt:ReadOPStatus
x   x   x   x    0   x   x   x    x   x   x   0    0   1   0   x    0   1   1   .       .   KbdInt:Open Bus
x   x   x   x    0   x   x   x    x   x   x   0    0   1   0   x    1   0   0   .       W   KbdInt:LoadKeyCtlReg
x   x   x   x    0   x   x   x    x   x   x   0    0   1   0   x    1   0   1   .       W   KbdInt:LoadKeyData
x   x   x   x    0   x   x   x    x   x   x   0    0   1   0   x    1   1   0   .       W   KbdInt:KeyDataReset
x   x   x   x    0   x   x   x    x   x   x   0    0   1   0   x    1   1   1   .       W   KbdInt:KeyChipReset
x   x   x   x    0   x   x   x    x   x   x   0    0   1   1   x    x   x   x   .       W   FIFOReg
x   x   x   x    0   x   x   x    x   x   x   0    1   0   0   x    x   x   x   .       .   Open Bus(originally ReadOpStatus)
x   x   x   x    0   x   x   x    x   x   x   0    1   0   1   x    x   *   *   .       RW  SelPIA (debugger board 8255 PIA)[Open Bus on normal units]
x   x   x   x    0   x   x   x    x   x   x   0    1   1   0   x    x   x   x   .       W   FIFOBus
x   x   x   x    0   x   x   x    x   x   x   0    1   1   1   x    x   x   x   .       .   Open Bus
x   x   x   x    0   x   x   x    x   x   x   1    0   0   0   x    x   x   x   .       RW  SelDiskReg
x   x   x   x    0   x   x   x    x   x   x   1    0   0   1   x    x   *   *   .       RW  SelDiskInt
x   x   x   x    0   x   x   x    x   x   x   1    0   1   0   *    *   *   *   .       W   SelCrtInt
x   x   x   x    0   x   x   x    x   x   x   1    0   1   1   x    x   x   x   .       W   LoadDispAddr
x   x   x   x    0   x   x   x    x   x   x   1    1   0   0   x    x   x   x   .       .   Open Bus(originally ReadEIAStatus)
x   x   x   x    0   x   x   x    x   x   x   1    1   0   1   x    0   0   0   .       R   SelEIA:ReadEIAStatus
x   x   x   x    0   x   x   x    x   x   x   1    1   0   1   x    0   0   1   .       R   SelEIA:ReadEIAData
x   x   x   x    0   x   x   x    x   x   x   1    1   0   1   x    0   1   0   .       .   SelEIA:Open Bus
x   x   x   x    0   x   x   x    x   x   x   1    1   0   1   x    0   1   1   .       .   SelEIA:Open Bus
x   x   x   x    0   x   x   x    x   x   x   1    1   0   1   x    1   0   0   .       W   SelEIA:LoadEIACtlReg
x   x   x   x    0   x   x   x    x   x   x   1    1   0   1   x    1   0   1   .       W   SelEIA:LoadEIAData
x   x   x   x    0   x   x   x    x   x   x   1    1   0   1   x    1   1   0   .       W   SelEIA:EIADataReset
x   x   x   x    0   x   x   x    x   x   x   1    1   0   1   x    1   1   1   .       W   SelEIA:EIAChipReset
x   x   x   x    0   x   x   x    x   x   x   1    1   1   0   x    x   x   x   .       R   SelADCHi
x   x   x   x    0   x   x   x    x   x   x   1    1   1   1   x    x   x   x   .       W   CRTSwitch
*/
static ADDRESS_MAP_START(notetaker_iocpu_io, AS_IO, 16, notetaker_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x7E1C) AM_DEVREADWRITE8("iopic8259", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x20, 0x21) AM_MIRROR(0x7E1E) AM_WRITE(IPConReg_w) // I/O processor (rom mapping, etc) control register
	AM_RANGE(0x42, 0x43) AM_MIRROR(0x7E10) AM_READ(ReadKeyData_r) // read keyboard data
	AM_RANGE(0x44, 0x45) AM_MIRROR(0x7E10) AM_READ(ReadOPStatus_r) // read keyboard fifo state
	AM_RANGE(0x48, 0x49) AM_MIRROR(0x7E10) AM_WRITE(LoadKeyCtlReg_w) // kbd uart control register
	AM_RANGE(0x4a, 0x4b) AM_MIRROR(0x7E10) AM_WRITE(LoadKeyData_w) // kbd uart data register
	AM_RANGE(0x4c, 0x4d) AM_MIRROR(0x7E10) AM_WRITE(KeyDataReset_w) // kbd uart ddr switch (data reset)
	AM_RANGE(0x4e, 0x4f) AM_MIRROR(0x7E10) AM_WRITE(KeyChipReset_w) // kbd uart reset
	AM_RANGE(0x60, 0x61) AM_MIRROR(0x7E1E) AM_WRITE(FIFOReg_w) // DAC sample and hold and frequency setup
	//AM_RANGE(0xa0, 0xa1) AM_MIRROR(0x7E18) AM_DEVREADWRITE("debug8255", 8255_device, read, write) // debugger board 8255
	AM_RANGE(0xc0, 0xc1) AM_MIRROR(0x7E1E) AM_WRITE(FIFOBus_w) // DAC data write to FIFO
	AM_RANGE(0x100, 0x101) AM_MIRROR(0x7E1E) AM_WRITE(DiskReg_w) // I/O register (adc speed, crtc pixel clock and clock enable, +5 and +12v relays for floppy, etc)
	AM_RANGE(0x120, 0x127) AM_MIRROR(0x7E18) AM_DEVREADWRITE8("wd1791", fd1791_t, read, write, 0x00FF) // floppy controller
	AM_RANGE(0x140, 0x15f) AM_MIRROR(0x7E00) AM_DEVREADWRITE8("crt5027", crt5027_device, read, write, 0x00FF) // crt controller
	AM_RANGE(0x160, 0x161) AM_MIRROR(0x7E1E) AM_WRITE(LoadDispAddr_w) // loads the start address for the display framebuffer
	AM_RANGE(0x1a0, 0x1a1) AM_MIRROR(0x7E10) AM_READ(ReadEIAStatus_r) // read keyboard fifo state
	AM_RANGE(0x1a2, 0x1a3) AM_MIRROR(0x7E10) AM_READ(ReadEIAData_r) // read keyboard data
	AM_RANGE(0x1a8, 0x1a9) AM_MIRROR(0x7E10) AM_WRITE(LoadEIACtlReg_w) // kbd uart control register
	AM_RANGE(0x1aa, 0x1ab) AM_MIRROR(0x7E10) AM_WRITE(LoadEIAData_w) // kbd uart data register
	AM_RANGE(0x1ac, 0x1ad) AM_MIRROR(0x7E10) AM_WRITE(EIADataReset_w) // kbd uart ddr switch (data reset)
	AM_RANGE(0x1ae, 0x1af) AM_MIRROR(0x7E10) AM_WRITE(EIAChipReset_w) // kbd uart reset
	//AM_RANGE(0x1c0, 0x1c1) AM_MIRROR(0x7E1E) AM_READ(SelADCHi_r) // ADC read
	//AM_RANGE(0x1e0, 0x1e1) AM_MIRROR(0x7E1E) AM_READ(CRTSwitch_w) // CRT power enable?
ADDRESS_MAP_END

/* iopic8259 interrupts:
irq0    parity error (parity error syndrome data will be in fffdx/fffex) - currently ignored
irq1    IPSysInt (interrupt triggered by the emulator cpu)
irq2    DiskInt (interrupt triggered by the IRQ or DRQ pins from the WD1791)
irq3    EIAInt  (interrupt triggered by the datareceived pin from the eiauart)
irq4    OddInt  (interrupt triggered by the O/E Odd/Even pin from the crt5027)
irq5    ADCInt  (interrupt triggered at the ADCSpd rate interrupt from 74c161 counter on the disk/display module to indicate adc conversion finished)
irq6    KbdInt  (interrupt triggered by the datareceived pin from the kbduart)
irq7    VSync   (interrupt from the VSYN VSync pin from the crt5027)
*/

/* writes during boot of io roms v2.0:
0x88 to port 0x020 (PCR; BootSeqDone(1), processor not locked(0), battery charger off(0), rom not disabled(0) correction off&cr4 off(1), cr3 on(0), cr2 on(0), cr1 on (0);)
0x0002 to port 0x100 (IOR write: enable 5v only relay control)
0x0003 to port 0x100 (IOR write: in addition to above, enable 12v relay control)
<dram memory 0x00000-0x3ffff is zeroed here>
0x13 to port 0x000 PIC (ICW1, 8085 vector 0b000[ignored], edge trigger mode, interval of 8, single mode (no cascade/ICW3), ICW4 needed )
0x08 to port 0x002 PIC (ICW2, T7-T3 = 0b00001)
0x0D to port 0x002 PIC (ICW4, SFNM OFF, Buffered Mode MASTER, Normal EOI, 8086 mode)
0xff to port 0x002 PIC (OCW1, All Ints Masked (disabled/suppressed))
0x0000 to port 0x04e (reset keyboard fifo/controller)
0x0000 to port 0x1ae (reset UART)
0x0016 to port 0x048 (kbd control reg write)
0x0005 to port 0x1a8 (UART control reg write)
0x5f to port 0x140 (reg0 95 horizontal lines)               \
0xf2 to port 0x142 (reg1 interlaced, hswidth=0xE, hsdelay=2) \
0x7d to port 0x144 (reg2 16 scans/row, 5 chars/datarow)       \
0x1d to port 0x146 (reg3 0 skew bits, 0x1D datarows/frame)     \_ set up CRTC
0x04 to port 0x148 (reg4 4 scan lines/frame)                   /
0x10 to port 0x14a (reg5 0x10 vdatastart)                     /
0x00 to port 0x154 (reset the crtc)                          /
0x1e to port 0x15a (reg8 load cursor line address = 0x1e)   /
0x0a03 to port 0x100 (IOR write: set bit clock to 12Mhz)
0x2a03 to port 0x100 (IOR write: enable crtc clock chain)
0x00 to port 0x15c (fire off crtc timing chain)
read from 0x0002 (byte wide) (IMR, read interrupt mask, will be 0xFF from above)
0xaf to port 0x002 PIC (mask out with 0xEF and 0xBF to unnmask interrupts IR4(OddInt) and IR6(KbdInt), and write back to IMR)
0x0400 to 0x060 (select DAC fifo frequency 2)
read from 0x44 (byte wide) to check input fifo status
... more stuff here missing relating to making the beep tone through fifo
(around pc=6b6) read keyboard uart until mouse button is clicked (WaitNoBug)
(around pc=6bc) read keyboard uart until mouse button is released (WaitBug)
0x2a23 to port 0x100 (select drive 1)
0x2a23 to port 0x100 (select drive 1)
0x3a23 to port 0x100 (unset disk separator clear (allow disk head reading))
0x3a27 to port 0x100 (select disk side 1)
0x3a07 to port 0x100 (unselect all drives)

*/

/* Emulator CPU */
/*
static ADDRESS_MAP_START(notetaker_emulatorcpu_mem, AS_PROGRAM, 16, notetaker_state)
    AM_RANGE(0x00000, 0x01fff) AM_MIRROR(0xC0000) AM_RAM // actually a banked block of ram, 8k (4k words)
    AM_RANGE(0x02000, 0x3ffff) AM_MIRROR(0xC0000) AM_RAM AM_BASE("mainram") // 256k of ram (less 8k), shared between both processors, mirrored 4 times
    AM_RANGE(0xFFFC0, 0xFFFDF) AM_READWRITE(proc_illinst_r, proc_illinst_w)
    AM_RANGE(0xFFFE0, 0xFFFEF) AM_READWRITE(proc_control_r, proc_control_w)
ADDRESS_MAP_END

// note everything in the emulatorcpu's io range is incompletely decoded; so if 0x1800 is accessed it will write to both the debug 8255 AND the pic8259! I'm not sure the code abuses this or not, but it might do so to both write registers and clear parity at once, or something similar.
static ADDRESS_MAP_START(notetaker_emulatorcpu_io, AS_IO, 16, notetaker_state)
    ADDRESS_MAP_UNMAP_HIGH
    AM_RANGE(0x800, 0x803) AM_MIRROR(0x07FC) AM_DEVREADWRITE8("emupic8259", pic8259_device, read, write, 0x00ff)
    AM_RANGE(0x1000, 0x1001) AM_MIRROR(0x07FE) AM_DEVREADWRITE("debug8255", 8255_device, read, write) // debugger board 8255, is this the same one as the iocpu accesses? or are these two 8255s on separate cards?
    AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x07FE) AM_WRITE(EPConReg_w) // emu processor control reg & leds
    AM_RANGE(0x4000, 0x4001) AM_MIRROR(0x07FE) AM_WRITE(EmuClearParity_w) // writes here clear the local 8k-ram parity error register
ADDRESS_MAP_END
*/

/* Input ports */

/* Floppy Image Interface */
static SLOT_INTERFACE_START( notetaker_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

/* Machine Start; allocate timers and savestate stuff */
void notetaker_state::machine_start()
{
	// allocate the DAC timer, and set it to fire NEVER. We'll set it up properly in IPReset.
	m_FIFO_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(notetaker_state::timer_fifoclk),this));
	m_FIFO_timer->adjust(attotime::never);
	// FDC: /DDEN is tied permanently LOW so MFM mode is ALWAYS ON
	m_fdc->dden_w(0);
	// Keyboard UART: /SWE is tied permanently LOW
	m_kbduart->set_input_pin(AY31015_SWE, 0); // status word outputs are permanently enabled (pin 16 SFD(SWE) tied low, active)
	// EIA UART: /SWE is tied permanently LOW
	m_eiauart->set_input_pin(AY31015_SWE, 0); // status word outputs are permanently enabled (pin 16 SFD(SWE) tied low, active)
	// savestate stuff
	// TODO: add me!
}

/* Machine Reset; this emulates the full system reset, triggered by ExtReset' (cardcage pin <50>) or the PowerOnReset' circuit */
void notetaker_state::machine_reset()
{
	ip_reset();
	ep_reset();
}

/* IP Reset; this emulates the IPReset' signal */
void notetaker_state::ip_reset()
{
	// reset the Keyboard UART
	m_kbduart->set_input_pin(AY31015_XR, 0); // MR - pin 21
	m_kbduart->set_input_pin(AY31015_XR, 1); // ''
	// reset the EIA UART
	m_eiauart->set_input_pin(AY31015_XR, 0); // MR - pin 21
	m_eiauart->set_input_pin(AY31015_XR, 1); // ''
	// reset the IPConReg latch at #f1
	m_BootSeqDone = 0;
	m_ProcLock = 0;
	m_CharCtr = 0;
	m_DisableROM = 0;
	m_CorrOn_q = 0;
	m_LedInd6 = 0;
	m_LedInd7 = 0;
	m_LedInd8 = 0;
	// Clear the DAC FIFO
	for (int i=0; i<16; i++) m_outfifo[i] = 0;
	m_outfifo_count = m_outfifo_tail_ptr = m_outfifo_head_ptr = 0;
	// reset the FIFOReg latch at #h9
	m_TabletYOn = 0;
	m_TabletXOn = 0;
	m_FrSel2 = 0;
	m_FrSel1 = 0;
	m_FrSel0 = 0;
	m_SHConB = 0;
	m_SHConA = 0;
	m_SetSH = 0;
	// handle consequences of above
	m_FIFO_timer->adjust(attotime::from_hz(((XTAL_960kHz/10)/4)/((m_FrSel0<<3)+(m_FrSel1<<2)+(m_FrSel2<<1)+1))); // FIFO timer is clocked by 960khz divided by 10 (74ls162 decade counter), divided by 4 (mc14568B with divider 1 pins set to 4), divided by 1,3,5,7,9,11,13,15 (or 0,2,4,6,8,10,12,14?)
	// todo: handle tablet and sample/hold stuff as well
	// reset the DiskReg latches at #c4 and #b4 on the disk/display/eia controller board
	m_ADCSpd0 = 0;
	m_ADCSpd1 = 0;
	m_StopWordClock_q = 0;
	m_ClrDiskCont_q = 0;
	m_ProgBitClk1 = 0;
	m_ProgBitClk2 = 0;
	m_ProgBitClk3 = 0;
	m_AnSel4 = 0;
	m_AnSel2 = 0;
	m_AnSel1 = 0;
	m_DriveSel1 = 0;
	m_DriveSel2 = 0;
	m_DriveSel3 = 0;
	m_SideSelect = 0;
	m_Disk5VOn = 0;
	m_Disk12VOn = 0;
	// handle the consequences of the above.
	// Disk12VOn probably runs the drive motor, and MotorOn is hard-wired to low/active, so turn the motor for all drives OFF
	m_floppy = m_floppy0->get_device();
	m_floppy->mon_w(1);
	//m_floppy = m_floppy1->get_device();
	//m_floppy->mon_w(1);
	//m_floppy = m_floppy2->get_device();
	//m_floppy->mon_w(1);
	m_floppy = nullptr; // select no drive
	// reset the Framebuffer Display Address:
	m_DispAddr = 0;
}

/* EP Reset; this emulates the EPReset' signal */
void notetaker_state::ep_reset()
{
}

/* Input ports */
static INPUT_PORTS_START( notetakr )
INPUT_PORTS_END

static MACHINE_CONFIG_START( notetakr, notetaker_state )
	/* basic machine hardware */
	/* IO CPU: 8086@8MHz */
	MCFG_CPU_ADD("iocpu", I8086, XTAL_24MHz/3) /* iD8086-2 @ E4A; 24Mhz crystal divided down to 8Mhz by i8284 clock generator */
	MCFG_CPU_PROGRAM_MAP(notetaker_iocpu_mem)
	MCFG_CPU_IO_MAP(notetaker_iocpu_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("iopic8259", pic8259_device, inta_cb)
	MCFG_PIC8259_ADD("iopic8259", INPUTLINE("iocpu", 0), VCC, NULL) // iP8259A-2 @ E6

	/* Emulator CPU: 8086@5MHz */
	/*MCFG_CPU_ADD("emulatorcpu", I8086, XTAL_15MHz/3)
	MCFG_CPU_PROGRAM_MAP(notetaker_emulatorcpu_mem)
	MCFG_CPU_IO_MAP(notetaker_emulatorcpu_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("emulatorpic8259", pic8259_device, inta_cb)
	MCFG_PIC8259_ADD("emulatorpic8259", INPUTLINE("emulatorcpu", 0), VCC, NULL) // iP8259A-2 @ E6
	*/

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.975)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(notetaker_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* Devices */
	MCFG_DEVICE_ADD( "crt5027", CRT5027, (XTAL_36MHz/4)/8) // the clock for the crt5027 is configurable rate; 36MHz xtal divided by 1*, 2, 3, 4, 5, 6, 7, or 8 (* because this is a 74s163 this setting probably means divide by 1; documentation at http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790605_Definition_of_8086_Ports.pdf claims it is 1.5, which makes no sense) and secondarily divided by 8 (again by two to load the 16 bit output shifters after this)
	// on reset, bitclk is 000 so divider is (36mhz/8)/8; during boot it is written with 101, changing the divider to (36mhz/4)/8
	// TODO: for now, we just hack it to the latter setting from start; this should be handled correctly in ip_reset();
	MCFG_TMS9927_CHAR_WIDTH(8) //(8 pixels per column/halfword, 16 pixels per fullword)
	// TODO: below is HACKED to trigger the odd/even int ir4 instead of vblank int ir7 since ir4 is required for anything to be drawn to screen! hence with the hack this interrupt triggers twice as often as it should
	MCFG_TMS9927_VSYN_CALLBACK(DEVWRITELINE("iopic8259", pic8259_device, ir4_w)) // note this triggers interrupts on both the iocpu (ir7) and emulatorcpu (ir4)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD( "kbduart", AY31015, 0 ) // HD6402, == AY-3-1015D
	MCFG_AY31015_RX_CLOCK(XTAL_960kHz) // hard-wired to 960KHz xtal #f11 (60000 baud, 16 clocks per baud)
	MCFG_AY31015_TX_CLOCK(XTAL_960kHz) // hard-wired to 960KHz xtal #f11 (60000 baud, 16 clocks per baud)

	MCFG_DEVICE_ADD( "eiauart", AY31015, 0 ) // HD6402, == AY-3-1015D
	MCFG_AY31015_RX_CLOCK(((XTAL_960kHz/10)/4)/5) // hard-wired through an mc14568b divider set to divide by 4, the result set to divide by 5; this resulting 4800hz signal being 300 baud (16 clocks per baud)
	MCFG_AY31015_TX_CLOCK(((XTAL_960kHz/10)/4)/5) // hard-wired through an mc14568b divider set to divide by 4, the result set to divide by 5; this resulting 4800hz signal being 300 baud (16 clocks per baud)

	/* Floppy */
	MCFG_FD1791_ADD("wd1791", (((XTAL_24MHz/3)/2)/2)) // 2mhz, from 24mhz ip clock divided by 6 via 8284, an additional 2 by LS161 at #e1 on display/floppy board
	MCFG_FLOPPY_DRIVE_ADD("wd1791:0", notetaker_floppies, "525dd", floppy_image_device::default_floppy_formats)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono") // TODO: should be stereo
	MCFG_SOUND_ADD("dac", DAC, 0) /* DAC1200, set up with two sample and hold HA2425 chips outside it to do stereo */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(notetaker_state,notetakr)
{
	// descramble the rom; the whole thing is a gigantic scrambled mess either to ease
	// interfacing with older xerox technologies which used A0 and D0 as the MSB bits
	// or maybe because someone screwed up somewhere along the line. we may never know.
	// see http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/schematics/19790423_Notetaker_IO_Processor.pdf pages 12 and onward
	UINT16 *romsrc = (UINT16 *)(memregion("iocpuload")->base());
	UINT16 *romdst = (UINT16 *)(memregion("iocpu")->base());
	UINT16 *temppointer;
	UINT16 wordtemp;
	UINT16 addrtemp;
		// leave the src pointer alone, since we've only used a 0x1000 long address space
		romdst += 0x7f800; // set the dest pointer to 0xff000 (>>1 because 16 bits data)
		for (int i = 0; i < 0x800; i++)
		{
			wordtemp = BITSWAP16(*romsrc, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); // data bus is completely reversed
			addrtemp = BITSWAP16(i, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10); // address bus is completely reversed; 11-15 should always be zero
			temppointer = romdst+(addrtemp&0x7FF);
			*temppointer = wordtemp;
			romsrc++;
		}
}

/* ROM definition */
ROM_START( notetakr )
	ROM_REGION( 0x1000, "iocpuload", ROMREGION_ERASEFF ) // load roms here before descrambling
	ROM_SYSTEM_BIOS( 0, "v2.00", "IO Monitor v2.00" ) // dumped from Notetaker
	ROMX_LOAD( "biop__2.00_hi.b2716.h1", 0x0000, 0x0800, CRC(1119691d) SHA1(4c20b595b554e6f5489ab2c3fb364b4a052f05e3), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "biop__2.00_lo.b2716.g1", 0x0001, 0x0800, CRC(b72aa4c7) SHA1(85dab2399f906c7695dc92e7c18f32e2303c5892), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v1.50", "IO Monitor v1.50" ) // typed from the source listing at http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790620_Z-IOP_1.5_ls.pdf and scrambled
	ROMX_LOAD( "z-iop_1.50_hi.h1", 0x0000, 0x0800, CRC(122ffb5b) SHA1(b957fe24620e1aa98b3158dbcf459937dbd54bac), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "z-iop_1.50_lo.g1", 0x0001, 0x0800, CRC(2cb79a67) SHA1(692aafd2aeea27533f6288dbb1cb8678ea08fade), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_REGION( 0x100000, "iocpu", ROMREGION_ERASEFF ) // area for descrambled roms
	ROM_REGION( 0x100000, "mainram", ROMREGION_ERASEFF ) // main ram, on 2 cards with parity/ecc/syndrome/timing/bus arbitration on another 2 cards
	ROM_REGION( 0x1000, "proms", ROMREGION_ERASEFF )
	ROM_LOAD( "disksep.prom.82s147.a4", 0x000, 0x200, NO_DUMP ) // disk data separator prom from the disk/display module board
	ROM_LOAD( "memcasraswrite.prom.82s147.b1", 0x200, 0x200, NO_DUMP ) // memory cas/ras/write state machine prom from the memory address logic board; the equations for this are listed in one of the documents on bitsavers
	ROM_LOAD( "setmemrq.prom.82s126.d9", 0x400, 0x100, NO_DUMP ) // SETMEMRQ memory timing prom from the disk/display module board; The equations for this one are actually listed on the schematic and the prom dump can be generated from these:
	/*
	SetMemRq:
	Address:
	01234567
	|||||||\- RCtr.3 (LSB)
	||||||\-- RCtr.2
	|||||\--- RCtr.1
	||||\---- RCtr.0 (MSB)
	|||\----- WCtr.3 (LSB)
	||\------ WCtr.2
	|\------- WCtr.1
	\-------- WCtr.0 (MSB)

	Data:
	0123
	|\\\- N/C (zero?)
	\---- SetMemRq

	Equation: SETMEMRQ == (
	  ((Rctr == 0) && ((Wctr == 0)||(Wctr == 4)||(Wctr == 8)))
	||((Rctr == 4) && ((Wctr == 4)||(Wctr == 8)||(Wctr == 12)))
	||((Rctr == 8) && ((Wctr == 8)||(Wctr == 12)||(Wctr == 0)))
	||((Rctr == 12) && ((Wctr == 12)||(Wctr == 0)||(Wctr == 4)))
	)
	(the setmemrq output might be inverted, as well)
	*/
ROM_END

/* Driver */

/*    YEAR      NAME  PARENT  COMPAT   MACHINE     INPUT            STATE      INIT  COMPANY     FULLNAME                FLAGS */
COMP( 1978, notetakr,      0,      0, notetakr, notetakr, notetaker_state, notetakr, "Xerox", "NoteTaker", MACHINE_IS_SKELETON)
//COMP( 1978, notetakr,      0,      0, notetakr, notetakr, driver_device, notetakr, "Xerox", "NoteTaker", MACHINE_IS_SKELETON)
