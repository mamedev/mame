// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/* Xerox NoteTaker, 1978
 * Driver by Jonathan Gevaryahu

 *  Notetaker Team At Xerox PARC 1976-1981:
     Alan Kay - Team Lead
     Bruce Horn - BIOS code and NoteTaker Hardware/Electronics Design (Ethernet, others)
     Ted Kaehler - SmallTalk-76 code porting[2] and more ( http://tedkaehler.weather-dimensions.com/us/ted/index.html )
     Dan Ingalls - Later BitBlt engine and SmallTalk-76 kernel and more[3]
     Doug Fairbairn - NoteTaker Hardware/Electronics Design (EmulatorP, IOP, ADC, Alto Debug/Test Interface)
         ( http://www.computerhistory.org/atchm/author/dfairbairn/ )
     Ed Wakida - NoteTaker Hardware/Electronics Design (Tablet/Touch interface)
     Bob Nishimura - NoteTaker Hardware/Electronics Design (PSU/Cabling/Machining/Battery)
     James "Jim" Leung - NoteTaker Hardware/Electronics Design (Alto Debug/Test Interface)
     Ron Freeman - NoteTaker Hardware/Electronics Design (Keyboard, Disk/Display)
     Ben Sato - NoteTaker Hardware/Electronics Design (Memory/Timing, ECC)
     B. Wang - NoteTaker Hardware/Electronics Design (Keyboard)
     Larry Tesler - NoteTaker Hardware/Electronics Design (Ethernet, others)
     Dale Mann - NoteTaker Hardware/Electronics Design (Ethernet, Circuit Assembly)
     Lawrence "Larry" D. Stewart - NoteTaker Hardware/Electronics Design (Mouse)
     Jim Althoff - Smalltalk-78 and Smalltalk-80
     Adele Goldberg - Smalltalk team
     Diana Merry-Shapiro - Original BitBlt from Smalltalk-72
     Dave Robson - Smalltalk team
     Ted Strollo - IC/VLSI design (MPC580 cell library)
     Bert Sutherland - Manager of Systems Science Laboratory (SSL)
     Terri Doughty - Administration, Editing
     Chris Jeffers - Smalltalk music
     <there are probably others I've missed>

 * History of the machine can be found at http://freudenbergs.de/bert/publications/Ingalls-2014-Smalltalk78.pdf

 * The notetaker has an 8-slot backplane, with the following cards in it:
   * I/O Processor card (8086@8Mhz, 8259pic, 4k ROM, Keyboard UART, DAC1200 (multiplexed to 2 channels))
   * Emulation Processor card (8086@5Mhz, 8259pic, 8k of local RAM with Parity check logic)
   * Disk/Display card (WD1791 FDC, CRT5027 CRTC, EIA UART, AD571 ADC, 8->1 Analog Multiplexer)
   * Memory Control Module \_ (bus control, buffering, refresh, Parity/ECC/Syndrome logic lives on these boards)
   * Memory Data Module    /
   * Memory Storage Module x2 (the 4116 DRAMs live on these boards)
   * Battery Module *OR* debugger module type A or B (debugger module has an
     i8255 on it for alto<->notetaker comms, and allows alto to halt the cpus
     [type A and B can debug either the emulator cpu or the iop respectively]
     and dump registers to alto screen, etc)

   * In 1980-1981 an Ethernet card with another 8086 on it was developed, but
     it is unclear if this was ever fully functional, or if smalltalk-78
     could even use it.

 * Prototypes only, 10 units[2] manufactured 1978-1980
   Known surviving units:
   * One at CHM (missing? mouse, no media, has BIOP-2.0 roms)
   * One at Xerox Museum at PARC (with mouse and 2? floppies, floppies were not imaged to the best of my knowledge, unknown roms)
   * Rumor has it at least a few of the remaining units survived beyond these two.

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
* Hack together a functional boot disk using the recovered EP bootloader and smalltalk engine code from the alto disks.
* Harris 6402 keyboard UART (within keyboard, next to MCU)
* The harris 6402 UART is pin compatible with WD TR1865 UART, as well as the AY-3-1015A/D
   (These are 5v-only versions of the older WD TR1602 and AY-5-1013 parts which used 5v and 12v)
* HLE for the missing i8748[5] MCU in the keyboard which reads the mouse quadratures and buttons and talks serially to the Keyboard UART
* floppy controller wd1791 and its interrupt
  According to [3] and [5] the format is double density/MFM, 128 bytes per sector, 16 sectors per track, 1 or 2 sided, for 170K or 340K per disk. Drive spins at 300RPM.
* hook up the DiskInt from the wd1791 either using m_fdc->intrq_r() polling or using device lines (latter is a much better idea)

WIP:
* crt5027 video controller - the iop side is hooked up, screen drawing 'works' but is scrambled due to not emulating the clock chain halting and clock changing yet. The crt5027 core also needs the odd/even interrupt hooked up, and proper interlace support as well as clock change/screen resize support (down to DC/no clock, which I guess should be a 1x1 single black pixel!)
* pic8259 interrupt controller - this is attached as a device, but only the vblank interrupt is hooked to it yet.
* Harris 6402 serial/EIA UART - connected to iop, other end isn't connected anywhere, interrupt is not connected
* Harris 6402 keyboard UART (within notetaker) - connected to iop, other end isn't connected anywhere, interrupt is not connected
* The DAC, its FIFO and its timer are hooked up and the v2.0 bios beeps, but the stereo latches are not hooked up at all, DAC is treated as mono for now

DONE:
* i/o cpu i/o area needs the memory map worked out per the schematics - done
* figure out the correct memory maps for the 256kB of shared ram, and what part of ram constitutes the framebuffer - done
  - 256k of shared ram maps at 00000-3ffff for both cpus with special mem regs at fffec,fffee. the ram mirrors 4 times on the emulatorcpu only, iop the 40000-fffff area is open bus.
  - framebuffer, at least for bios 1.5, lives from 0x4000-0xd5ff, exactly 640x480 pixels 1bpp, interlaced (even? plane is at 4000-8aff, odd? plane is at 8b00-d5ff); however the starting address of the framebuffer is configurable to any address within the 0x0000-0x1ffff range? (this exact range is unclear)
* figure out how the emulation-cpu boots and where its 8k of local ram maps to - done
  - both cpus boot, reset and system int controls are accessed at fffea from either cpu; emulatorcpu's 8k of ram lives at the beginning of its address space, but can be disabled in favor of mainram at the same addresses
* 82s147 DISKSEP PROM regenerated from original BCPL code
* SETMEMREQ PROM retyped from binary listing
* TIMING PROM retyped from binary listing
*/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/pic8259.h"
#include "machine/wd_fdc.h"
#include "sound/dac.h"
#include "video/tms9927.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_VIDEO           (1U << 1)
#define LOG_READOP_STATUS   (1U << 2)
#define LOG_FIFO            (1U << 3)
#define LOG_SPC_DSP         (1U << 4)
#define LOG_FIFO_VERBOSE    (1U << 5)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class notetaker_state : public driver_device
{
public:
	notetaker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_iop_cpu(*this, "iop_cpu"),
		m_iop_pic(*this, "iop_pic8259"),
		m_ep_cpu(*this, "ep_cpu"),
		m_ep_pic(*this, "ep_pic8259"),
		m_kbduart(*this, "kbduart"),
		m_eiauart(*this, "eiauart"),
		m_crtc(*this, "crt5027"),
		m_dac(*this, "dac"),
		m_fdc(*this, "wd1791"),
		m_floppy0(*this, "wd1791:0"),
		m_floppy(nullptr)
	{
	}

	void notetakr(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void driver_start() override;

	void iop_io(address_map &map) ATTR_COLD;
	void iop_mem(address_map &map) ATTR_COLD;
	void ep_io(address_map &map) ATTR_COLD;
	void ep_mem(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_iop_cpu;
	required_device<pic8259_device> m_iop_pic;
	required_device<cpu_device> m_ep_cpu;
	required_device<pic8259_device> m_ep_pic;
	required_device<ay31015_device> m_kbduart;
	required_device<ay31015_device> m_eiauart;
	required_device<crt5027_device> m_crtc;
	required_device<dac_word_interface> m_dac;
	required_device<fd1791_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	floppy_image_device *m_floppy;

	std::unique_ptr<uint16_t[]> m_mainram;

	// screen
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	// basic io
	void ipcon_reg_w(uint16_t data);
	void epcon_reg_w(uint16_t data);
	void fifo_reg_w(uint16_t data);
	void fifo_bus_w(uint16_t data);
	void disk_reg_w(uint16_t data);
	void load_disp_addr_w(uint16_t data);

	// uarts
	uint16_t read_op_status_r();
	void load_key_ctl_reg_w(uint16_t data);
	void key_data_reset_w(uint16_t data);
	void key_chip_reset_w(uint16_t data);
	uint16_t read_eia_status_r();
	void load_eia_ctl_reg_w(uint16_t data);
	void eia_data_reset_w(uint16_t data);
	void eia_chip_reset_w(uint16_t data);

	// mem map stuff
	uint16_t iop_r(offs_t offset);
	void iop_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ep_mainram_r(offs_t offset, uint16_t mem_mask);
	void ep_mainram_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// IPConReg
	uint8_t m_boot_seq_done = 0;
	uint8_t m_proc_lock = 0;
	uint8_t m_char_ctr = 0;
	uint8_t m_disable_rom = 0;
	uint8_t m_corr_on_q = 0;
	uint8_t m_led_ind6 = 0;
	uint8_t m_led_ind7 = 0;
	uint8_t m_led_ind8 = 0;

	//  FIFOReg
	uint8_t m_tablet_y_on = 0;
	uint8_t m_tablet_x_on = 0;
	uint8_t m_fr_sel2 = 0;
	uint8_t m_fr_sel1 = 0;
	uint8_t m_fr_sel0 = 0;
	uint8_t m_sh_conb = 0;
	uint8_t m_sh_cona = 0;
	uint8_t m_set_sh = 0;

	// DiskReg
	uint8_t m_adc_spd0 = 0;
	uint8_t m_adc_spd1 = 0;
	uint8_t m_stopword_clock_q = 0;
	uint8_t m_clr_diskcont_q = 0;
	uint8_t m_prog_bitclk1 = 0;
	uint8_t m_prog_bitclk2 = 0;
	uint8_t m_prog_bitclk3 = 0;
	uint8_t m_an_sel4 = 0;
	uint8_t m_an_sel2 = 0;
	uint8_t m_an_sel1 = 0;
	uint8_t m_drive_sel1 = 0;
	uint8_t m_drive_sel2 = 0;
	uint8_t m_drive_sel3 = 0;
	uint8_t m_side_select = 0;
	uint8_t m_disk_5v_on = 0;
	uint8_t m_disk_12v_on = 0;

	// output fifo, for DAC
	uint16_t m_outfifo[16]; // technically three 74LS225 5bit*16stage FIFO chips, arranged as a 16 stage, 12-bit wide fifo (one bit unused per chip)
	uint8_t m_outfifo_count = 0;
	uint8_t m_outfifo_tail_ptr = 0;
	uint8_t m_outfifo_head_ptr = 0;

	// fifo timer
	emu_timer *m_fifo_timer = nullptr;
	TIMER_CALLBACK_MEMBER(timer_fifoclk);

	// framebuffer display starting address
	uint16_t m_disp_addr = 0;

	void iop_reset();
	void ep_reset();
};

TIMER_CALLBACK_MEMBER(notetaker_state::timer_fifoclk)
{
	//pop a value off the fifo and send it to the dac.
	if (m_outfifo_count == 0)
		LOGMASKED(LOG_FIFO_VERBOSE, "output fifo is EMPTY! repeating previous sample!\n");

	uint16_t data = m_outfifo[m_outfifo_tail_ptr];
	// if fifo is empty (tail ptr == head ptr), do not increment the tail ptr, otherwise do.
	if (m_outfifo_count > 0)
	{
		m_outfifo_tail_ptr++;
		m_outfifo_count--;
	}
	m_outfifo_tail_ptr &= 0xf;
	m_dac->write(data);
	m_fifo_timer->adjust(attotime::from_hz(((960_kHz_XTAL / 10) / 4) / ((m_fr_sel0 << 3) + (m_fr_sel1 << 2) + (m_fr_sel2 << 1) + 1)));
}

uint32_t notetaker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// have to figure out what resolution we're drawing to here and draw appropriately to screen
	// code borrowed/stolen from video/mac.cpp
	uint32_t const video_base = (m_disp_addr << 3) & 0x1ffff;
	uint16_t const *video_ram_field1 = &m_mainram[video_base / 2];
	uint16_t const *video_ram_field2 = &m_mainram[(video_base + 0x4b00) / 2];

	LOGMASKED(LOG_VIDEO, "Video Base = 0x%05x\n", video_base);

	for (int y = 0; y < 480; y++)
	{
		uint16_t *const line = &bitmap.pix(y);

		for (int x = 0; x < 640; x += 16)
		{
			uint16_t const word = *((y & 1) ? video_ram_field2 : video_ram_field1)++;
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = BIT(word, 15 - b);
			}
		}
	}
	return 0;
}

void notetaker_state::ipcon_reg_w(uint16_t data)
{
	m_boot_seq_done = BIT(data, 7);
	m_proc_lock     = BIT(data, 6); // bus lock for this processor (hold other processor in wait state)
	m_char_ctr      = BIT(data, 5); // battery charge control (incorrectly called 'Char counter' in source code)
	m_disable_rom   = BIT(data, 4); // disable rom at 0000-0fff
	m_corr_on_q     = BIT(data, 3); // CorrectionOn (ECC correction enabled); also LedInd5
	m_led_ind6      = BIT(data, 2);
	m_led_ind7      = BIT(data, 1);
	m_led_ind8      = BIT(data, 0);
	popmessage("LEDS: CR1: %d, CR2: %d, CR3: %d, CR4: %d", BIT(data, 2), BIT(data, 3), BIT(data, 1), BIT(data, 0)); // cr1 and 2 are in the reverse order as expected, according to the schematic
}

/* handlers for the two system hd6402s (ay-5-1013 equivalent) */
/* * Keyboard hd6402 */
uint16_t notetaker_state::read_op_status_r() // 74ls368 hex inverter at #l7 provides 4 bits, inverted
{
	uint16_t data = 0xfff0;
	data |= (m_outfifo_count >= 1) ? 0 : 0x08; // m_fifo_out_rdy is true if the fifo has at least 1 word in it, false otherwise
	data |= (m_outfifo_count < 16) ? 0 : 0x04; // m_fifo_in_rdy is true if the fifo has less than 16 words in it, false otherwise
	// note /SWE is permanently enabled, so we don't enable it here for HD6402 reading
	data |= m_kbduart->dav_r()  ? 0 : 0x02; // DR - pin 19
	data |= m_kbduart->tbmt_r() ? 0 : 0x01; // TBRE - pin 22

	LOGMASKED(LOG_READOP_STATUS, "ReadOPStatus read, returning %04x\n", data);

	return data;
}

void notetaker_state::load_key_ctl_reg_w(uint16_t data)
{
	m_kbduart->write_cs(0);
	m_kbduart->write_np(BIT(data, 4)); // PI - pin 35
	m_kbduart->write_tsb(BIT(data, 3)); // SBS - pin 36
	m_kbduart->write_nb2(BIT(data, 2)); // CLS2 - pin 37
	m_kbduart->write_nb1(BIT(data, 1)); // CLS1 - pin 38
	m_kbduart->write_eps(BIT(data, 0)); // EPE - pin 39
	m_kbduart->write_cs(1);
}

void notetaker_state::key_data_reset_w(uint16_t data)
{
	m_kbduart->write_rdav(0); // DDR - pin 18
	m_kbduart->write_rdav(1); // ''
}

void notetaker_state::key_chip_reset_w(uint16_t data)
{
	m_kbduart->write_xr(0); // MR - pin 21
	m_kbduart->write_xr(1); // ''
}

/* FIFO (DAC) Stuff and ADC stuff */
void notetaker_state::fifo_reg_w(uint16_t data)
{
	m_set_sh = BIT(data, 15);
	m_sh_cona = BIT(data, 14);
	m_sh_conb = BIT(data, 13);
	m_fr_sel0 = BIT(data, 12);
	m_fr_sel1 = BIT(data, 11);
	m_fr_sel2 = BIT(data, 10);
	m_tablet_x_on = BIT(data, 9);
	m_tablet_y_on = BIT(data, 8);
	m_fifo_timer->adjust(attotime::from_hz(((960_kHz_XTAL / 10) / 4) / ((m_fr_sel0 << 3) + (m_fr_sel1 << 2) + (m_fr_sel2 << 1) + 1)));
	/* FIFO timer is clocked by 960khz divided by 10 (74ls162 decade counter),
	divided by 4 (mc14568B with divider 1 pins set to 4), divided by
	1,3,5,7,9,11,13,15 (or 0,2,4,6,8,10,12,14?)
	*/
	// todo: handle tablet and sample/hold stuff as well
	LOGMASKED(LOG_FIFO, "Write to 0x60 fifo_reg_w of %04x; fifo timer set to %d hz\n", data, ((960'000 / 10) / 4) / ((m_fr_sel0 << 3) + (m_fr_sel1 << 2) + (m_fr_sel2 << 1) + 1));
}

void notetaker_state::fifo_bus_w(uint16_t data)
{
	if (m_outfifo_count == 16)
	{
		LOGMASKED(LOG_SPC_DSP, "outfifo was full, write ignored!\n");
		return;
	}
	m_outfifo[m_outfifo_head_ptr] = data >> 4;
	m_outfifo_head_ptr++;
	m_outfifo_count++;
	m_outfifo_head_ptr&=0xF;
}

void notetaker_state::disk_reg_w(uint16_t data)
{
	/* See http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19781023_More_NoteTaker_IO_Information.pdf
	   but note that bit 12 (called bit 3 in documentation) was changed between
	   oct 1978 and 1979 to reset the disk controller digital-PLL as
	   ClrDiskCont' rather than acting as ProgBitClk0, which is permanently
	   wired high instead, meaning only the 4.5Mhz - 18Mhz dot clocks are
	   available for the CRTC. */
	m_adc_spd0 = BIT(data, 15);
	m_adc_spd1 = BIT(data, 14);
	m_stopword_clock_q = BIT(data, 13);
	//if (!(m_clr_diskcont_q) && (data & 0x1000)) m_floppy->device_reset(); // reset on rising edge
	m_clr_diskcont_q = BIT(data, 12); // originally ProgBitClk0, but co-opted later to reset the FDC's external PLL
	m_prog_bitclk1 = BIT(data, 11);
	m_prog_bitclk2 = BIT(data, 10);
	m_prog_bitclk3 = BIT(data, 9);
	m_an_sel4 = BIT(data, 8);
	m_an_sel2 = BIT(data, 7);
	m_an_sel1 = BIT(data, 6);
	m_drive_sel1 = BIT(data, 5);
	m_drive_sel2 = BIT(data, 4); // drive 2 not present on hardware, but could work if present
	m_drive_sel3 = BIT(data, 3); // drive 3 not present on hardware, but could work if present
	m_side_select = BIT(data, 2);
	m_disk_5v_on = BIT(data, 1);
	m_disk_12v_on = BIT(data, 0);

	// ADC stuff
	//TODO

	// FDC stuff
	// first handle the motor stuff; we'll clobber whatever was in m_floppy, then reset it to what it should be
	m_floppy = m_floppy0->get_device();

	// Disk5VOn and 12VOn can be thought of as a crude MotorOn signal as the motor won't run with either? of them missing.
	// However, a tech note involves adding a patch so that MotorOn is only activated if the drive is actually selected.
	m_floppy->mon_w(!(m_disk_5v_on && m_disk_12v_on && m_drive_sel1));

	//m_floppy = m_floppy1->get_device();
	//m_floppy->mon_w(!(m_disk_5v_on && m_disk_12v_on && m_drive_sel2)); // Disk5VOn and 12VOn can be thought of as a crude MotorOn signal as the motor won't run with either? of them missing.
	//m_floppy = m_floppy2->get_device();
	//m_floppy->mon_w(!(m_disk_5v_on && m_disk_12v_on && m_drive_sel3)); // Disk5VOn and 12VOn can be thought of as a crude MotorOn signal as the motor won't run with either? of them missing.

	// now restore m_floppy state to what it should be
	if (m_drive_sel1)
		m_floppy = m_floppy0->get_device();
	else
		m_floppy = nullptr;

	m_fdc->set_floppy(m_floppy); // select the floppy
	if (m_floppy)
	{
		m_floppy->ss_w(m_side_select);
	}

	// CRTC clock rate stuff
	//TODO
}

void notetaker_state::load_disp_addr_w(uint16_t data)
{
	m_disp_addr = data;
	// for future low level emulation: clear the current counter position here as well, as well as empty/reset the display fifo, and the setmemrq state.
}

/* EIA hd6402 */
uint16_t notetaker_state::read_eia_status_r() // 74ls368 hex inverter at #f1 provides 2 bits, inverted
{
	uint16_t data = 0xfffc;
	// note /SWE is permanently enabled, so we don't enable it here for HD6402 reading
	data |= m_eiauart->dav_r()  ? 0 : 0x02; // DR - pin 19
	data |= m_eiauart->tbmt_r() ? 0 : 0x01; // TBRE - pin 22
	return data;
}

void notetaker_state::load_eia_ctl_reg_w(uint16_t data)
{
	m_eiauart->write_cs(0);
	m_eiauart->write_np(BIT(data, 4)); // PI - pin 35
	m_eiauart->write_tsb(BIT(data, 3)); // SBS - pin 36
	m_eiauart->write_nb2(BIT(data, 2)); // CLS2 - pin 37
	m_eiauart->write_nb1(BIT(data, 1)); // CLS1 - pin 38
	m_eiauart->write_eps(BIT(data, 0)); // EPE - pin 39
	m_eiauart->write_cs(1);
}

void notetaker_state::eia_data_reset_w(uint16_t data)
{
	m_eiauart->write_rdav(0); // DDR - pin 18
	m_eiauart->write_rdav(1); // ''
}

void notetaker_state::eia_chip_reset_w(uint16_t data)
{
	m_eiauart->write_xr(0); // MR - pin 21
	m_eiauart->write_xr(1); // ''
}


/* These next two members are memory map related for the iop */
uint16_t notetaker_state::iop_r(offs_t offset)
{
	uint16_t *rom = (uint16_t *)(memregion("iop")->base());
	rom += 0x7f800;
	uint16_t *ram = m_mainram.get();
	if (m_boot_seq_done == 0 || (m_disable_rom == 0 && (offset & 0x7f800) == 0))
	{
		rom += offset & 0x7ff;
		return *rom;
	}
	else
	{
		// are we in the FFFE8-FFFEF area where the parity/int/reset/etc stuff lives?
		if (offset >= 0x7fff4)
		{
			logerror("attempt to read processor control regs at %d\n", offset << 1);
			return 0xffff;
		}
		ram += offset;
		return *ram;
	}
}

void notetaker_state::iop_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//uint16_t tempword;
	uint16_t *ram = m_mainram.get();
	if (m_boot_seq_done == 0 || (m_disable_rom == 0 && (offset & 0x7f800) == 0))
	{
		logerror("attempt to write %04X to ROM-mapped area at %06X ignored\n", data, offset << 1);
		return;
	}
	// are we in the FFFE8-FFFEF area where the parity/int/reset/etc stuff lives?
	if (offset >= 0x7fff4)
	{
		logerror("attempt to write processor control regs at %d with %02X ignored\n", offset << 1, data);
	}
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
mode 2: (during load of the emulatorcpu's firmware to the first 8k of shared ram which is on the emulatorcpu board)
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
*/

void notetaker_state::iop_mem(address_map &map)
{
	/*
	map(0x00000, 0x00fff).rom().region("iop", 0xff000); // rom is here if either BootSeqDone OR DisableROM are zero. the 1.5 source code and the schematics implies writes here are ignored while rom is enabled; if disablerom is 1 this goes to mainram
	map(0x01000, 0x3ffff).ram().share("mainram"); // 256k of ram (less 8k), shared between both processors. rom goes here if bootseqdone is 0
	// note 4000-d5ff is the framebuffer for the screen, in two sets of fields for odd/even interlace?
	map(0xff000, 0xfffe7).rom().region("iop", 0xff000); // rom is only banked in here if bootseqdone is 0, so the reset vector is in the proper place. otherwise the memory control regs live at fffe8-fffef
	//map(0xfffea, 0xfffeb).w(FUNC(notetaker_state::cpuCtl_w));
	//map(0xfffec, 0xfffed).r(FUNC(notetaker_state::parityErrHi_r));
	//map(0xfffee, 0xfffef).r(this. FUNC(notetaker_state::parityErrLo_r));
	map(0xffff0, 0xfffff).rom().region("iop", 0xffff0);
	*/
	map(0x00000, 0xfffff).rw(FUNC(notetaker_state::iop_r), FUNC(notetaker_state::iop_w)); // bypass MAME's memory map system as we need finer grained control
}

/* iop memory map comes from http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/memos/19790605_Definition_of_8086_Ports.pdf
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
void notetaker_state::iop_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x03).mirror(0x7e1c).rw(m_iop_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x20, 0x21).mirror(0x7e1e).w(FUNC(notetaker_state::ipcon_reg_w)); // I/O processor (rom mapping, etc) control register
	map(0x42, 0x42).mirror(0x7e10).r(m_kbduart, FUNC(ay31015_device::receive)); // read keyboard data
	map(0x44, 0x45).mirror(0x7e10).r(FUNC(notetaker_state::read_op_status_r)); // read keyboard fifo state
	map(0x48, 0x49).mirror(0x7e10).w(FUNC(notetaker_state::load_key_ctl_reg_w)); // kbd uart control register
	map(0x4a, 0x4a).mirror(0x7e10).w(m_kbduart, FUNC(ay31015_device::transmit)); // kbd uart data register
	map(0x4c, 0x4d).mirror(0x7e10).w(FUNC(notetaker_state::key_data_reset_w)); // kbd uart ddr switch (data reset)
	map(0x4e, 0x4f).mirror(0x7e10).w(FUNC(notetaker_state::key_chip_reset_w)); // kbd uart reset
	map(0x60, 0x61).mirror(0x7e1e).w(FUNC(notetaker_state::fifo_reg_w)); // DAC sample and hold and frequency setup
	//map(0xa0, 0xa1).mirror(0x7e18).rw("debug8255", FUNC(8255_device::read), FUNC(8255_device::write)); // debugger board 8255
	map(0xc0, 0xc1).mirror(0x7e1e).w(FUNC(notetaker_state::fifo_bus_w)); // DAC data write to FIFO
	map(0x100, 0x101).mirror(0x7e1e).w(FUNC(notetaker_state::disk_reg_w)); // I/O register (adc speed, crtc pixel clock and clock enable, +5 and +12v relays for floppy, etc)
	map(0x120, 0x127).mirror(0x7e18).rw(m_fdc, FUNC(fd1791_device::read), FUNC(fd1791_device::write)).umask16(0x00ff); // floppy controller
	map(0x140, 0x15f).mirror(0x7e00).rw(m_crtc, FUNC(crt5027_device::read), FUNC(crt5027_device::write)).umask16(0x00ff); // crt controller
	map(0x160, 0x161).mirror(0x7e1e).w(FUNC(notetaker_state::load_disp_addr_w)); // loads the start address for the display framebuffer
	map(0x1a0, 0x1a1).mirror(0x7e10).r(FUNC(notetaker_state::read_eia_status_r)); // read eia fifo state
	map(0x1a2, 0x1a2).mirror(0x7e10).r(m_eiauart, FUNC(ay31015_device::receive)); // read eia data
	map(0x1a8, 0x1a9).mirror(0x7e10).w(FUNC(notetaker_state::load_eia_ctl_reg_w)); // eia uart control register
	map(0x1aa, 0x1aa).mirror(0x7e10).w(m_eiauart, FUNC(ay31015_device::transmit)); // eia uart data register
	map(0x1ac, 0x1ad).mirror(0x7e10).w(FUNC(notetaker_state::eia_data_reset_w)); // eia uart ddr switch (data reset)
	map(0x1ae, 0x1af).mirror(0x7e10).w(FUNC(notetaker_state::eia_chip_reset_w)); // eia uart reset
	//map(0x1c0, 0x1c1).mirror(0x7e1e).r(FUNC(notetaker_state::SelADCHi_r)); // ADC read
	//map(0x1e0, 0x1e1).mirror(0x7e1e).r(FUNC(notetaker_state::CRTSwitch_w)); // CRT power enable?
}

/* iop_pic8259 interrupts:
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

void notetaker_state::epcon_reg_w(uint16_t data)
{
	/*m_EP_LED1 = m_EP_ParityError; // if parity checking is enabled AND the last access was to the low 8k AND there was a parity error, the parity error latch is latched here. It triggers an interrupt.
	m_EP_LED2 = (data&0x40)?1:0;
	m_EP_LED3 = (data&0x20)?1:0;
	m_EP_LED4 = (data&0x10)?1:0;
	m_EP_LED_SelROM_q = (data&0x08)?1:0; // this doesn't appear to be hooked anywhere, andjust drives an LED
	// originally, SelROM_q enabled two 2716 EPROMS, later 82s137 PROMS to map code to the FFC00-FFFFF area but this was dropped in the 1979 design revision in favor of having the IOP write the boot vectors for the EP to the shared ram instead. See below for how the top two address bits are disconnected to allow this to work with the way the shared ram is mapped.
	m_EP_ProcLock = (data&0x04)?1:0; // bus lock for this processor (hold other processor in wait state)
	m_EP_SetParity_q = (data&0x02)?1:0; // enable parity checking on local ram if low
	m_EP_DisLMem_q = (data&0x01)?1:0; // if low, the low 8k of local memory is disabled and accesses the shared memory instead.
	popmessage("EP LEDS: CR1: %d, CR2: %d, CR3: %d, CR4: %d", (data&0x80)>>2, (data&0x40)>>3, (data&0x20)>>1, (data&0x10));*/
}

/*
Emulator cpu mem map:
(The top two address bits are disconnected, to allow the ram board, which maps itself only at 00000-3ffff, to appear at "ffff0" to the ep processor when /reset is de-asserted by the iop)
a19 a18 a17 a16  a15 a14 a13 a12  a11 a10 a9  a8   a7  a6  a5  a4   a3  a2  a1  a0   DisLMem_q
x   x   0   0    0   0   0   *    *   *   *   *    *   *   *   *    *   *   *   *    0                       RW  Local (fast) RAM
x   x   0   0    0   0   0   *    *   *   *   *    *   *   *   *    *   *   *   *    1                       RW  System/Shared RAM
<anything not all zeroes >   *    *   *   *   *    *   *   *   *    *   *   *   *    x                       RW  System/Shared RAM
   EXCEPT for the following, decoded by the EP board and superseding above:
x   x   1   1    1   1   1   1    1   1   1   1    1   1   0   x    x   x   x   x    x                       RW  FFFC0-FFFDF (trigger ILLINST interrupt on EP, data ignored?)
   And the following, decoded by the memory address logic board:
x   x   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   0   0   x    x                       .   Open Bus
x   x   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   0   1   x    x                       W   FFFEA (Multiprocessor Control (reset(bit 6)/int(bit 5)/boot(bit 4) for each processor; data bits 3,2,1,0 are 'processor address'; 0010 means IP, 0111 means EP; all others ignored.))
x   x   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   1   0   x    x                       R   FFFEC (Syndrome bits (gnd bit 15, parity bit 14, exp(syndrome) bits 13-8, bits 7-0 are the highest address bits)
x   x   1   1    1   1   1   1    1   1   1   1    1   1   1   0    1   1   1   x    x                       R   FFFEE (Parity Error Address: row bits 15-8, column bits 7-0; reading this also acknowledges a parity interrupt)
*/

uint16_t notetaker_state::ep_mainram_r(offs_t offset, u16 mem_mask)
{
	return m_mainram[offset + 0x2000/2];
}

void notetaker_state::ep_mainram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mainram[offset + 0x2000/2]);
}

void notetaker_state::ep_mem(address_map &map)
{
	map(0x00000, 0x01fff).mirror(0xc0000).ram(); // actually a banked block of ram, 8kb (4kw)
	map(0x02000, 0x3ffff).mirror(0xc0000).rw(FUNC(notetaker_state::ep_mainram_r), FUNC(notetaker_state::ep_mainram_w)); // 256k of ram (less 8k), shared between both processors, mirrored 4 times
	//map(0xfffc0, 0xfffdf).mirror(0xc0000).rw(FUNC(notetaker_state::proc_illinst_r), FUNC(notetaker_state::proc_illinst_w));
	//map(0xfffe0, 0xfffef).mirror(0xc0000).rw(FUNC(notetaker_state::proc_control_r), FUNC(notetaker_state::proc_control_w));
}

/* note everything in the emulatorcpu's io range is incompletely decoded; so if
   0x1800 is accessed it will write to both the debug 8255 AND the pic8259!
   I'm not sure the code abuses this or not, but it might do so to both write
   registers and clear parity at once, or something similar. */
/*
Emulator cpu i/o map:
a19 a18 a17 a16  a15 a14 a13 a12  a11 a10 a9  a8   a7  a6  a5  a4   a3  a2  a1  a0   DisLMem_q
x   x   x   x    x   x   x   x    1   x   x   x    x   x   x   x    x   x   *   x    x                       RW  8259
x   x   x   x    x   x   x   1    x   x   x   x    x   x   x   x    x   *   *   x    x                       RW  EP debugger 8255, same exact interface on both cpu and alto side as the IOP debugger 8255
x   x   x   x    x   x   1   x    x   x   x   x    x   x   x   x    x   x   x   x    x                       W   EPConReg
x   x   x   x    x   1   x   x    x   x   x   x    x   x   x   x    x   x   x   x    x                       W   Writing anything here clears the parity error latch
*/

void notetaker_state::ep_io(address_map &map)
{
	map.unmap_value_high();
	map(0x800, 0x803).mirror(0x07fc).rw(m_ep_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	//map(0x1000, 0x1001).mirror(0x07fe).rw("debug8255", FUNC(8255_device::read), FUNC(8255_device::write)); // debugger board 8255, is this the same one as the iop accesses? or are these two 8255s on separate cards?
	map(0x2000, 0x2001).mirror(0x07fe).w(FUNC(notetaker_state::epcon_reg_w)); // emu processor control reg & leds
	//map(0x4000, 0x4001).mirror(0x07fe).w(FUNC(notetaker_state::EmuClearParity_w)); // writes here clear the local 8k-ram parity error register
}

/* Input ports */

/* Floppy Image Interface */
static void notetaker_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

/* Machine Start; allocate timers and savestate stuff */
void notetaker_state::machine_start()
{
	// allocate RAM
	m_mainram = make_unique_clear<uint16_t[]>(0x100000/2);

	// allocate the DAC timer, and set it to fire NEVER. We'll set it up properly in IPReset.
	m_fifo_timer = timer_alloc(FUNC(notetaker_state::timer_fifoclk), this);
	m_fifo_timer->adjust(attotime::never);

	// FDC: /DDEN is tied permanently LOW so MFM mode is ALWAYS ON
	m_fdc->dden_w(0);

	// Keyboard UART: /SWE is tied permanently LOW
	m_kbduart->write_swe(0); // status word outputs are permanently enabled (pin 16 SFD(SWE) tied low, active)

	// EIA UART: /SWE is tied permanently LOW
	m_eiauart->write_swe(0); // status word outputs are permanently enabled (pin 16 SFD(SWE) tied low, active)

	// TODO: register savestate items
}

/* Machine Reset; this emulates the full system reset, triggered by ExtReset' (cardcage pin <50>) or the PowerOnReset' circuit */
void notetaker_state::machine_reset()
{
	iop_reset();
	ep_reset();
}

/* IP Reset; this emulates the IPReset' signal */
void notetaker_state::iop_reset()
{
	// reset the Keyboard UART
	m_kbduart->write_xr(0); // MR - pin 21
	m_kbduart->write_xr(1); // ''

	// reset the EIA UART
	m_eiauart->write_xr(0); // MR - pin 21
	m_eiauart->write_xr(1); // ''

	// reset the IPConReg ls273 latch at #f1
	ipcon_reg_w(0x0000);

	// Clear the DAC FIFO
	for (int i = 0; i < 16; i++)
		m_outfifo[i] = 0;
	m_outfifo_count = 0;
	m_outfifo_tail_ptr = 0;
	m_outfifo_head_ptr = 0;

	// reset the FIFOReg latch at #h9
	fifo_reg_w(0x0000);

	// reset the DiskReg latches at #c4 and #b4 on the disk/display/eia controller board
	disk_reg_w(0x0000);

	// reset the framebuffer display address counter:
	m_disp_addr = 0;
}

/* EP Reset; this emulates the EPReset' signal */
void notetaker_state::ep_reset()
{
	// TODO: force ep into reset and hold it there, until the iop releases it.
	// there's 6 'state' bits controllable by the memory mapped cpu control reg, which need to be reset for epcpu and iocpu separately
}

/* Input ports */
static INPUT_PORTS_START( notetakr )
INPUT_PORTS_END

void notetaker_state::notetakr(machine_config &config)
{
	/* basic machine hardware */
	/* IO CPU: 8086@8MHz */
	I8086(config, m_iop_cpu, 24_MHz_XTAL / 3); /* iD8086-2 @ E4A; 24Mhz crystal divided down to 8Mhz by i8284 clock generator */
	m_iop_cpu->set_addrmap(AS_PROGRAM, &notetaker_state::iop_mem);
	m_iop_cpu->set_addrmap(AS_IO, &notetaker_state::iop_io);
	m_iop_cpu->set_irq_acknowledge_callback("iop_pic8259", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_iop_pic, 0); // iP8259A-2 @ E6
	m_iop_pic->out_int_callback().set_inputline(m_iop_cpu, 0);

	/* Emulator CPU: 8086@5MHz */
	I8086(config, m_ep_cpu, 15_MHz_XTAL / 3);
	m_ep_cpu->set_disable(); // TODO: implement the cpu control bits so this doesn't execute garbage/zeroes before its firmware gets loaded
	m_ep_cpu->set_addrmap(AS_PROGRAM, &notetaker_state::ep_mem);
	m_ep_cpu->set_addrmap(AS_IO, &notetaker_state::ep_io);
	m_ep_cpu->set_irq_acknowledge_callback("ep_pic8259", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_ep_pic, 0); // iP8259A-2 @ E6
	m_ep_pic->out_int_callback().set_inputline(m_ep_cpu, 0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.975);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(250));
	screen.set_screen_update(FUNC(notetaker_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* Devices */
	CRT5027(config, m_crtc, (36_MHz_XTAL / 4) / 8); // See below
	/* the clock for the crt5027 is configurable rate; 36MHz xtal divided by 1*,
	   2, 3, 4, 5, 6, 7, or 8 (* because this is a 74s163 this setting probably
	   means divide by 1; documentation at
	   http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790605_Definition_of_8086_Ports.pdf
	   claims it is 1.5, which makes no sense) and secondarily divided by 8
	   (again by two to load the 16 bit output shifters after this).
	   on reset, bitclk is 000 so divider is (36mhz/8)/8; during boot it is
	   written with 101, changing the divider to (36mhz/4)/8 */
	// TODO: for now, we just hack it to the latter setting from start; this should be handled correctly in iop_reset();
	m_crtc->set_char_width(8); //(8 pixels per column/halfword, 16 pixels per fullword)
	// TODO: below is HACKED to trigger the odd/even int ir4 instead of vblank int ir7 since ir4 is required for anything to be drawn to screen.
	// hence with the hack this interrupt triggers twice as often as it should
	m_crtc->vsyn_callback().set(m_iop_pic, FUNC(pic8259_device::ir4_w)); // note this triggers interrupts on both the iop (ir7) and emulatorcpu (ir4)
	m_crtc->set_screen("screen");

	AY31015(config, m_kbduart); // HD6402, == AY-3-1015D
	m_kbduart->write_dav_callback().set(m_iop_pic, FUNC(pic8259_device::ir6_w)); // DataRecvd = KbdInt

	clock_device &kbdclock(CLOCK(config, "kbdclock", 960_kHz_XTAL)); // hard-wired to 960KHz xtal #f11 (60000 baud, 16 clocks per baud)
	kbdclock.signal_handler().set(m_kbduart, FUNC(ay31015_device::write_rcp));
	kbdclock.signal_handler().append(m_kbduart, FUNC(ay31015_device::write_tcp));

	AY31015(config, m_eiauart); // HD6402, == AY-3-1015D
	m_eiauart->write_dav_callback().set(m_iop_pic, FUNC(pic8259_device::ir3_w)); // EIADataReady = EIAInt

	// hard-wired through an mc14568b divider set to divide by 4, the result set to divide by 5; this resulting 4800hz signal being 300 baud (16 clocks per baud)
	clock_device &eiaclock(CLOCK(config, "eiaclock", ((960_kHz_XTAL / 10) / 4) / 5));
	eiaclock.signal_handler().set(m_eiauart, FUNC(ay31015_device::write_rcp));
	eiaclock.signal_handler().append(m_eiauart, FUNC(ay31015_device::write_tcp));

	/* Floppy */
	FD1791(config, m_fdc, (((24_MHz_XTAL / 3) / 2) / 2)); // 2mhz, from 24mhz ip clock divided by 6 via 8284, an additional 2 by LS161 at #e1 on display/floppy board
	FLOPPY_CONNECTOR(config, "wd1791:0", notetaker_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();
	// TODO: hook DAC up to two HA2425 (sample and hold) chips and hook those up to the speakers
	DAC1200(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 1); // unknown DAC
}

void notetaker_state::driver_start()
{
	// descramble the rom; the whole thing is a gigantic scrambled mess either to ease
	// interfacing with older xerox technologies which used A0 and D0 as the MSB bits
	// or maybe because someone screwed up somewhere along the line. we may never know.
	// see http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/schematics/19790423_Notetaker_IO_Processor.pdf pages 12 and onward
	uint16_t *romsrc = (uint16_t *)(memregion("iopload")->base());
	uint16_t *romdst = (uint16_t *)(memregion("iop")->base());
	// leave the src pointer alone, since we've only used a 0x1000 long address space
	romdst += 0x7f800; // set the dest pointer to 0xff000 (>>1 because 16 bits data)
	for (int i = 0; i < 0x800; i++)
	{
		uint16_t wordtemp = bitswap<16>(*romsrc, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); // data bus is completely reversed
		uint16_t addrtemp = bitswap<11>(i, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10); // address bus is completely reversed; 11-15 should always be zero
		uint16_t *temppointer = romdst + (addrtemp & 0x7ff);
		*temppointer = wordtemp;
		romsrc++;
	}
}

/* ROM definition */
/*
The notetaker, over its lifetime from 1978 to 1981, had three different classes of IOP roms, with multiple versions of each one.
These were:
BIOP - "Bootable", standalone "user" unit, running smalltalk-78 off of a boot disk, either single or double density; early notetakers used an fd1791 while later ones used a wd1791.
XIOP - "eXercizer" intended for initial testing of each NoteTaker system as assembled; only usable running tethered to a Xerox Alto running notex (notex.cm) as a hardware scripting language for system testing
MIOP - only bootable tethered to a Xerox Alto via a debug card, running smalltalk on the NoteTaker, but not booted off of the floppy disk.
The 'Z-iop' firmware 1.5 below seems to be a BIOP firmware.
*/

ROM_START( notetakr )
	ROM_REGION( 0x1000, "iopload", ROMREGION_ERASEFF ) // load roms here before descrambling
	ROM_SYSTEM_BIOS( 0, "v2.00", "Bootable IO Monitor v2.00" ) // dumped from Notetaker
	ROMX_LOAD( "biop__2.00_hi.b2716.h1", 0x0000, 0x0800, CRC(1119691d) SHA1(4c20b595b554e6f5489ab2c3fb364b4a052f05e3), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "biop__2.00_lo.b2716.g1", 0x0001, 0x0800, CRC(b72aa4c7) SHA1(85dab2399f906c7695dc92e7c18f32e2303c5892), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v1.50", "Bootable IO Monitor v1.50" ) // typed from the source listing at http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790620_Z-IOP_1.5_ls.pdf and scrambled
	ROMX_LOAD( "z-iop_1.50_hi.h1", 0x0000, 0x0800, CRC(122ffb5b) SHA1(b957fe24620e1aa98b3158dbcf459937dbd54bac), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "z-iop_1.50_lo.g1", 0x0001, 0x0800, CRC(2cb79a67) SHA1(692aafd2aeea27533f6288dbb1cb8678ea08fade), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION( 0x100000, "iop", ROMREGION_ERASEFF ) // area for descrambled roms
	// main ram, on 2 cards with parity/ecc/syndrome/timing/bus arbitration on another 2 cards

	// keyboard mcu which handles key scanning as well as reading the mouse quadratures, and issues state responses if requested by the iop
	ROM_REGION( 0x400, "kbmcu", ROMREGION_ERASEFF )
	ROM_LOAD( "keyboard.i8748.a10a", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x500, "proms", ROMREGION_ERASEFF )
	/* disk data separator prom from the disk/display module board:
	   there are two different versions of this prom, both generated by BCPL programs,
	   one from mid 1978 (Single density only? seems very buggy and might not even work)
	   and one from 1979 (which should work and appears here).
	   Note that the bit order for the state counter (data bits 6,5,4,3) may be
	   reversed vs the real machine, but since the prom address bus is the only
	   thing that ever sees the prom data bus, this prom will work even if the
	   bit order for those bits is backwards.
	*/
	// 1979 version
	ROM_LOAD( "disksep.82s147.a4", 0x000, 0x200, CRC(38363714) SHA1(c995d2702573f5afb5fc919150d3a5661013f999) )

	// memory cas/ras/write state machine prom from the memory address logic board; the contents of this are listed in:
	// http://www.bitsavers.org/pdf/xerox/notetaker/schematics/19781027_Memory_Address_Timing.pdf
	ROM_LOAD( "timingprom.82s147.b1", 0x200, 0x200, CRC(3003b50a) SHA1(77d9ffe4716c2297708b8e5ebce7f930619c3cc3) )

	// SETMEMRQ memory timing prom from the disk/display module board; The equations for this one are actually listed on the schematic and the prom dump can be generated from these:
	ROM_LOAD( "memreqprom.82s126.d9", 0x400, 0x100, CRC(56b2be8b) SHA1(5df0579ed8afeb59113700be6f2982ef85f64b44) )

	/*
	SetMemRq:
	Address:
	76543210
	|||||||\- WCtr.0 (MSB)
	||||||\-- WCtr.1
	|||||\--- WCtr.2
	||||\---- WCtr.3 (LSB)
	|||\----- RCtr.0 (MSB)
	||\------ RCtr.1
	|\------- RCtr.2
	\-------- RCtr.3 (LSB)

	The schematic has an error here, showing the SetMemRq_q output coming from data bit 0, in reality based on the listing it comes from data bit 3
	Data:
	3210
	|\\\- N/C (always zero)
	\---- SetMemRq_q

	Equation: SETMEMRQ == (
	  ((Rctr == 0) && ((Wctr == 0)||(Wctr == 4)||(Wctr == 8)))
	||((Rctr == 4) && ((Wctr == 4)||(Wctr == 8)||(Wctr == 12)))
	||((Rctr == 8) && ((Wctr == 8)||(Wctr == 12)||(Wctr == 0)))
	||((Rctr == 12) && ((Wctr == 12)||(Wctr == 0)||(Wctr == 4)))
	)
	The PROM output is SetMemRq_q and is inverted compared to the equation above.
	*/
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS            INIT           COMPANY  FULLNAME     FLAGS
COMP( 1978, notetakr, 0,      0,      notetakr, notetakr, notetaker_state, empty_init, "Xerox", "NoteTaker", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
