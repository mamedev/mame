// license:BSD-3-Clause
// copyright-holders:Angelo Salese
// thanks-to: Fujix
/**************************************************************************************************

    PC-88VA (c) 1987 NEC

    Here be dragons, a mostly compatible PC-8801 with extra V3 Mode for superset.

    preliminary driver by Angelo Salese
    Special thanks to Fujix for his documentation translation help

    TODO:
    - video emulation is bare bones;
    - keyboard irq (mightmg2, hold F8 service mode menu, presumably rogueall)
    - Backport from PC-8801 main map, apply supersets where applicable;
      \- IDP has EMUL for upd3301
      \- In emulation mode HW still relies to a i8214, so it bridges thru
         main ICU in cascaded mode via IRQ7;
      \- (other stuff ...)
    - sorcer: disables FDC DMA mode, expects to continue loading thru PC80S31K PIO;
    - abunaten, cresmoon, rance2, pacman: flips FDC DS1 then changes FDD#0 track density to 96 TPI,
      gets stuck to infinite reti loop and jumps to EMM area;
    - pacmana: crashes after or during PC Engine OS POST, plays again with FDC settings;
    - rance: as above, triggers SETALC in V50;
    - upo: fails vblank on/off logic bit;
    - olteus: sometimes fails disk swap with an (A)bort (R)etry (F)ail;
    - famista: throws an "abnormal disk" error when left on title screen for a while;
    - hatisora: throws an (A)bort (R)etry (F)ail (hidden behind current screen brokenness);
    - Convert SASI from PC-9801 to a shared device, apparently it's same i/f;
    - Implement bus slot, which should still be PC-8801 EXPansion bus.

    (old notes, to be reordered)
    - fdc "intelligent mode" has 0x7f as irq vector ... 0x7f is ld a,a and it IS NOT correctly
      hooked up by the current z80 core
    - PC-88VA stock version has two bogus opcodes. One is at 0xf0b15, another at 0xf0b31.
      Making a patch for the latter makes the system to jump into a "DIP-Switch" display.
      bp f0b31,pc=0xf0b36,g
      Update: it never reaches latter with V30->V50 CPU switch fix;
    - Fix floppy motor hook-up (floppy believes to be always in even if empty drive);
    - Support for PC8801 compatible mode & PC80S31K (floppy interface);
    - What exact kind of garbage happens if you try to enable both direct and palette color
      modes to a graphic layer?

===================================================================================================

irq table (line - vector - source):
ICU
irq 0  - 08h - timer 1
irq 1  - 09h - keyboard irq
irq 2  - 0Ah - VRTC
irq 3  - 0Bh - UINT0 (B24)
irq 4  - 0Ch - RS-232C
irq 5  - 0Dh - UINT1 (B25)
irq 6  - 0Eh - UINT2 (B26)
irq 7  - N/A - Slave (either secondary i8259 or i8214)
i8259 slave
irq 8  - 10H - SGP
irq 9  - 11H - UINT3 (HDD, B27)
irq 10 - 12H - UINT4 (B28)
irq 11 - 13H - FDC
irq 12 - 14H - Sound
irq 13 - 15H - General timer 3 (mouse)
irq 14 - 16H - <reserved>
irq 15 - 17H - <reserved>

trap list (brief, for quick consultation):
brk 82h AH=01h <undocumented>, "paint" uses it
brk 8Ch AH=02h read calendar clock -> CH = hour, CL = minutes, DH = seconds, DL = 0

**************************************************************************************************/

#include "emu.h"
#include "pc88va.h"

#include "softlist_dev.h"
#include <iostream>


#define LOG_FDC     (1U << 2) // $1b0-$1b2 accesses
#define LOG_FDC2    (1U << 3) // $1b4-$1b6 accesses

#define VERBOSE (LOG_GENERAL | LOG_FDC)
#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGFDC(...)      LOGMASKED(LOG_FDC, __VA_ARGS__)
#define LOGFDC2(...)     LOGMASKED(LOG_FDC2, __VA_ARGS__)

// TODO: verify clocks
#define MASTER_CLOCK    XTAL(8'000'000) // may be XTAL(31'948'800) / 4? (based on PC-8801 and PC-9801)
#define FM_CLOCK        (XTAL(31'948'800) / 4) // 3993600


void pc88va_state::pc88va_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
//  map(0x80000, 0x9ffff).ram(); // EMM
	map(0xa0000, 0xdffff).m(m_sysbank, FUNC(address_map_bank_device::amap16));
	map(0xe0000, 0xeffff).bankr("rom00_bank");
	map(0xf0000, 0xfffff).bankr("rom10_bank");
}

/* 0x00000 - 0x3ffff Kanji ROM 1*/
/* 0x40000 - 0x4ffff Kanji ROM 2*/
/* 0x50000 - 0x53fff Backup RAM */
/* above that is a NOP presumably */
uint8_t pc88va_state::kanji_ram_r(offs_t offset)
{
	return m_kanjiram[offset];
}

void pc88va_state::kanji_ram_w(offs_t offset, uint8_t data)
{
	// TODO: there's an area that can be write protected
	m_kanjiram[offset] = data;
	m_gfxdecode->gfx(2)->mark_dirty(offset / 8);
	m_gfxdecode->gfx(3)->mark_dirty(offset / 32);
}


void pc88va_state::sysbank_map(address_map &map)
{
	// 0 select bus slot (?)
	// 1 tvram
	map(0x040000, 0x04ffff).ram().share("tvram");
	// 4 gvram
	map(0x100000, 0x13ffff).ram().share("gvram");
	// 8-9 kanji
	map(0x200000, 0x23ffff).rom().region("kanji", 0x00000);
	map(0x240000, 0x24ffff).rom().region("kanji", 0x40000);
	map(0x250000, 0x253fff).rw(FUNC(pc88va_state::kanji_ram_r),FUNC(pc88va_state::kanji_ram_w));
	// c-d dictionary
	map(0x300000, 0x37ffff).rom().region("dictionary", 0);
}

u8 pc88va_state::port40_r()
{
	u8 data = 0;
	// TODO: vblank logic fails with upo
	data = (m_screen->vpos() < 400) ? 0x20 : 0x00; // vblank
	data |= m_rtc->data_out_r() << 4;
	data |= (ioport("DSW")->read() & 1) ? 2 : 0;

	return data | 0xc0;
}

void pc88va_state::port40_w(offs_t offset, u8 data)
{
	m_rtc->stb_w((data & 2) >> 1);
	m_rtc->clk_w((data & 4) >> 2);
}

void pc88va_state::rtc_w(offs_t offset, u8 data)
{
	m_rtc->c0_w((data & 1) >> 0);
	m_rtc->c1_w((data & 2) >> 1);
	m_rtc->c2_w((data & 4) >> 2);
	m_rtc->data_in_w((data & 8) >> 3);

	// TODO: remaining bits
}

uint16_t pc88va_state::bios_bank_r()
{
	return m_bank_reg;
}

void pc88va_state::bios_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	-x-- ---- ---- ---- SMM (compatibility mode)
	---x ---- ---- ---- GMSP (VRAM drawing mode)
	---- xxxx ---- ---- SMBC (0xa0000 - 0xdffff RAM bank)
	---- ---- xxxx ---- RBC1 (0xf0000 - 0xfffff ROM bank)
	---- ---- ---- xxxx RBC0 (0xe0000 - 0xeffff ROM bank)
	*/
	COMBINE_DATA(&m_bank_reg);

	/* SMBC */
	m_sysbank->set_bank((m_bank_reg & 0xf00) >> 8);

	/* RBC1 */
	{
		uint8_t *ROM10 = memregion("rom10")->base();

		if((m_bank_reg & 0xe0) == 0x00)
			membank("rom10_bank")->set_base(&ROM10[(m_bank_reg & 0x10) ? 0x10000 : 0x00000]);
	}

	/* RBC0 */
	{
		uint8_t *ROM00 = memregion("rom00")->base();

		membank("rom00_bank")->set_base(&ROM00[(m_bank_reg & 0xf)*0x10000]); // TODO: docs says that only 0 - 5 are used, dunno why ...
	}
}

uint8_t pc88va_state::rom_bank_r()
{
	return 0xff; // bit 7 low is va91 rom bank status
}

uint8_t pc88va_state::key_r(offs_t offset)
{
	// holding F8 at POST to bring a service mode
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3",
											"KEY4", "KEY5", "KEY6", "KEY7",
											"KEY8", "KEY9", "KEYA", "KEYB",
											"KEYC", "KEYD", "KEYE", "KEYF" };

	return ioport(keynames[offset])->read();
}

void pc88va_state::backupram_wp_1_w(uint16_t data)
{
	m_backupram_wp = 1;
}

void pc88va_state::backupram_wp_0_w(uint16_t data)
{
	m_backupram_wp = 0;
}

uint8_t pc88va_state::hdd_status_r()
{
	return 0x20;
}

uint8_t pc88va_state::pc88va_fdc_r(offs_t offset)
{
	LOGFDC("Unhandled read $%04x\n", (offset << 1) + 0x1b0);

	switch(offset << 1)
	{
		case 0x00: return 0; // FDC mode register
		case 0x02: return 0; // FDC control port 0
		case 0x04: return 0; // FDC control port 1
		/* ---x ---- RDY: (0) Busy (1) Ready */
		case 0x06: // FDC control port 2
			return 0;
	}

	return 0xff;
}

TIMER_CALLBACK_MEMBER(pc88va_state::pc88va_fdc_timer)
{
	if(m_fdc_ctrl_2 & 4) // XTMASK
	{
		m_pic2->ir3_w(0);
		m_pic2->ir3_w(1);
	}
}

TIMER_CALLBACK_MEMBER(pc88va_state::pc88va_fdc_motor_start_0)
{
	m_fdd[0]->get_device()->mon_w(0);
	m_fdc_motor_status[0] = 1;
}

TIMER_CALLBACK_MEMBER(pc88va_state::pc88va_fdc_motor_start_1)
{
	m_fdd[1]->get_device()->mon_w(0);
	m_fdc_motor_status[1] = 1;
}

#if 0
void pc88va_state::pc88va_fdc_update_ready(floppy_image_device *, int)
{
	bool ready = m_fdc_ctrl_2 & 0x40;

	floppy_image_device *floppy;
	floppy = m_fdd[0]->get_device();
	if(floppy && ready)
		ready = floppy->ready_r();

	floppy = m_fdd[1]->get_device();
	if(floppy && ready)
		ready = floppy->ready_r();

	m_fdc->ready_w(ready);
}

#else

void pc88va_state::pc88va_fdc_update_ready(floppy_image_device *, int)
{
	if (!BIT(m_fdc_ctrl_2, 5))
		return;
	bool force_ready = (BIT(m_fdc_ctrl_2, 6));

	floppy_image_device *floppy0, *floppy1;
	floppy0 = m_fdd[0]->get_device();
	floppy1 = m_fdd[1]->get_device();
	if (!floppy0 && !floppy1)
		force_ready = false;

	//if(floppy && force_ready)
	//  ready = floppy->ready_r();

	//if(floppy && force_ready)
	//  ready = floppy->ready_r();

	if (force_ready)
	{
		m_fdc->set_ready_line_connected(0);
		m_fdc->ready_w(0);
	}
	else
		m_fdc->set_ready_line_connected(1);
}

#endif

void pc88va_state::pc88va_fdc_w(offs_t offset, uint8_t data)
{
	switch(offset << 1)
	{
		/*
		---- ---x MODE: FDC op mode (0) Intelligent (1) DMA
		*/
		case 0x00: // FDC mode register
			m_fdc_mode = data & 1;
			LOGFDC("$1b0 FDC op mode (%02x) %s mode\n"
				, data
				, m_fdc_mode ? "DMA" : "Intelligent (PIO)"
			);
			#if TEST_SUBFDC
			m_fdccpu->set_input_line(INPUT_LINE_HALT, (m_fdc_mode) ? ASSERT_LINE : CLEAR_LINE);
			#endif
			break;
		/*
		--x- ---- CLK: FDC clock selection (0) 4.8MHz (1) 8 MHz
		---x ---- DS1: Prohibition of the drive selection of FDC (0) Permission (1) Prohibition
		---- xx-- TD1/TD0: Drive 1/0 track density (0) 48 TPI (1) 96 TPI
		---- --xx RV1/RV0: Drive 1/0 mode selection (0) 2D and 2DD mode (1) 2HD mode
		*/
		case 0x02: // FDC control port 0
		{
			const bool clk = bool(BIT(data, 5));
			const bool rv1 = bool(BIT(data, 1));
			const bool rv0 = bool(BIT(data, 0));
			LOGFDC("$1b2 FDC control port 0 (%02x) %s CLK| %d DS1| %d%d TD1/TD0| %d%d RV1/RV0\n"
				, data
				, clk ? "  8 MHz" : "4.8 MHz"
				, !bool(BIT(data, 4))
				, bool(BIT(data, 3))
				, bool(BIT(data, 2))
				, rv1
				, rv0
			);
			m_fdd[0]->get_device()->set_rpm(rv0 ? 360 : 300);
			m_fdd[1]->get_device()->set_rpm(rv1 ? 360 : 300);

			//m_fdd[0]->get_device()->ds_w(!BIT(data, 4));
			//m_fdd[1]->get_device()->ds_w(!BIT(data, 4));

			// TODO: is this correct? sounds more like a controller clock change, while TD1/TD0 should do the rate change
			m_fdc->set_rate(clk ? 500000 : 250000);
			break;
		}
		/*
		---- x--- PCM: precompensation control (1) on
		---- --xx M1/M0: Drive 1/0 motor control (0) NOP (1) Change motor status
		*/
		case 0x04:
		{
			const bool m0 = bool(BIT(data, 0));
			const bool m1 = bool(BIT(data, 1));

			LOGFDC2("$1b4 FDC control port 1 (%02x) %d PCM| %d%d M1/M0\n"
				, data
				, bool(BIT(data, 3))
				, m1
				, m0
			);

			// TODO: fine grain motor timings
			// VAEG claims 600 msecs, must be more complex than that
			if( m0 )
			{
				m_fdd[0]->get_device()->mon_w(1);
				if(m_fdc_motor_status[0] == 0)
					m_motor_start_timer[0]->adjust(attotime::from_msec(505));
				else
					m_fdc_motor_status[0] = 0;
			}

			if( m1 )
			{
				m_fdd[1]->get_device()->mon_w(1);
				if(m_fdc_motor_status[1] == 0)
					m_motor_start_timer[1]->adjust(attotime::from_msec(505));
				else
					m_fdc_motor_status[1] = 0;
			}

			break;
		}

		/*
		 * FDC control port 2
		 * x--- ---- FDCRST: FDC Reset
		 * -xx- ---- FDCFRY FRYCEN: FDC force ready control
		 * -x0- ---- ignored
		 * -01- ---- force ready release
		 * -11- ---- force ready assert
		 * ---x ---- DMAE: DMA Enable (0) Prohibit DMA (1) Enable DMA
		 * ---- -x-- XTMASK: FDC timer IRQ mask (0) Disable (1) Enable
		 * ---- ---x TTRG: FDC timer trigger (0) FDC timer clearing (1) FDC timer start
		 */
		case 0x06:
		{
			const bool fdcrst = bool(BIT(data, 7));
			const bool ttrg = bool(BIT(data, 0));
			LOGFDC2("$1b6 FDC control port 2 (%02x) %d FDCRST| %d%d FDCFRY| %d DMAE| %d XTMASK| %d TTRG\n"
				, data
				, fdcrst
				, bool(BIT(data, 6))
				, bool(BIT(data, 5))
				, bool(BIT(data, 4))
				, bool(BIT(data, 2))
				, ttrg
			);

			if( ttrg )
				m_fdc_timer->adjust(attotime::from_msec(100));

			//if (!BIT(m_fdc_ctrl_2, 4) && BIT(data, 4))
			//  m_maincpu->dreq_w<2>(1);
			//m_dmac->dreq2_w(1);

			// TODO: 0 -> 1 transition?
			if( fdcrst )
				m_fdc->reset();

			m_fdc_ctrl_2 = data;

			//m_fdd[0]->get_device()->mon_w(!(BIT(data, 5)));

			pc88va_fdc_update_ready(nullptr, 0);

			break;
		}
	}
}


uint16_t pc88va_state::sysop_r()
{
	uint8_t sys_op;

	sys_op = ioport("SYSOP_SW")->read() & 3;

	return 0xfffc | sys_op; // docs says all the other bits are high
}

TIMER_CALLBACK_MEMBER(pc88va_state::t3_mouse_callback)
{
	if(m_timer3_io_reg & 0x80)
	{
		m_pic2->ir5_w(0);
		m_pic2->ir5_w(1);
		m_t3_mouse_timer->adjust(attotime::from_hz(120 >> (m_timer3_io_reg & 3)));
	}
}

/*
 * x--- ---- MINTEN (TCU irq enable)
 * ---- --xx MTP1/MTP0 general purpose timer 3 interval
 * ---- --00 120 Hz
 * ---- --01 60 Hz
 * ---- --10 30 Hz
 * ---- --11 15 Hz
 */
void pc88va_state::timer3_ctrl_reg_w(uint8_t data)
{
	m_timer3_io_reg = data;

	if(data & 0x80)
		m_t3_mouse_timer->adjust(attotime::from_hz(120 >> (m_timer3_io_reg & 3)));
	else
	{
		m_pic2->ir5_w(0);
		m_t3_mouse_timer->adjust(attotime::never);
	}
}

uint8_t pc88va_state::backupram_dsw_r(offs_t offset)
{
	if(offset == 0)
		return m_kanjiram[0x1fc2 / 2] & 0xff;

	return m_kanjiram[0x1fc6 / 2] & 0xff;
}

void pc88va_state::sys_port1_w(uint8_t data)
{
	// ...
}

#if !TEST_SUBFDC
uint8_t pc88va_state::no_subfdc_r()
{
	return machine().rand();
}
#endif

uint8_t pc88va_state::misc_ctrl_r()
{
	return m_misc_ctrl;
}

void pc88va_state::misc_ctrl_w(uint8_t data)
{
	m_misc_ctrl = data;

	m_sound_irq_enable = ((data & 0x80) == 0);

	if (m_sound_irq_enable)
		int4_irq_w(m_sound_irq_pending);
}

// TODO: I/O 0x00xx is almost same as pc8801
// (*) are specific N88 V1 / V2 ports

void pc88va_state::pc88va_io_map(address_map &map)
{
	map(0x0000, 0x000f).r(FUNC(pc88va_state::key_r)); // Keyboard ROW reading
	map(0x0010, 0x0010).w(FUNC(pc88va_state::rtc_w)); // Printer / Calendar Clock Interface
	map(0x0020, 0x0021).noprw(); // RS-232C
	map(0x0030, 0x0031).rw(FUNC(pc88va_state::backupram_dsw_r), FUNC(pc88va_state::sys_port1_w)); // 0x30 (R) DSW1 (W) Text Control Port 0 / 0x31 (R) DSW2 (W) System Port 1
	map(0x0032, 0x0032).rw(FUNC(pc88va_state::misc_ctrl_r), FUNC(pc88va_state::misc_ctrl_w));
//  map(0x0034, 0x0034) GVRAM Control Port 1
//  map(0x0035, 0x0035) GVRAM Control Port 2
	map(0x0040, 0x0040).rw(FUNC(pc88va_state::port40_r), FUNC(pc88va_state::port40_w)); // (R) System Port 4 (W) System port 3 (strobe port)
	map(0x0044, 0x0047).rw(m_opna, FUNC(ym2608_device::read), FUNC(ym2608_device::write));
//  map(0x0050, 0x005b) CRTC/backdrop on PC8801, causes HW trap on VA
//  map(0x005c, 0x005c) (R) GVRAM status
//  map(0x005c, 0x005f) (W) GVRAM selection
//  map(0x0060, 0x0068) DMA on PC8801, causes HW trap on VA
//  map(0x0070, 0x0070) ? (*)
//  map(0x0071, 0x0071) Expansion ROM select (*)
//  map(0x0078, 0x0078) Memory offset increment (*)
//  map(0x0080, 0x0081) HDD related
	map(0x0082, 0x0082).r(FUNC(pc88va_state::hdd_status_r));// HDD control, byte access 7-0
//  map(0x00bc, 0x00bf) d8255 1
//  map(0x00e2, 0x00e3) Expansion RAM selection (*)
//  map(0x00e4, 0x00e4) 8214 IRQ control (*)
//  map(0x00e6, 0x00e6) 8214 IRQ mask (*)
//  map(0x00e8, 0x00e9) ? (*)
//  map(0x00ec, 0x00ed) ? (*)
	#if TEST_SUBFDC
	map(0x00fc, 0x00ff).rw("d8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write)); // d8255 2, FDD
	#else
	map(0x00fc, 0x00ff).r(FUNC(pc88va_state::no_subfdc_r)).nopw();
	#endif

	map(0x0100, 0x0101).rw(FUNC(pc88va_state::screen_ctrl_r), FUNC(pc88va_state::screen_ctrl_w)); // Screen Control Register
	map(0x0102, 0x0103).w(FUNC(pc88va_state::gfx_ctrl_w));
	map(0x0106, 0x0109).w(FUNC(pc88va_state::video_pri_w)); // Palette Control Register (priority) / Direct Color Control Register (priority)
//  map(0x010a, 0x010b) Picture Mask Mode Register
//  map(0x010c, 0x010d) Color Palette Mode Register
//  map(0x010e, 0x010f) Backdrop Color Register
//  map(0x0110, 0x0111) Color Code/Plain Mask Register
//  map(0x0124, 0x0125) ? (related to Transparent Color of Graphic Screen 0)
//  map(0x0126, 0x0127) ? (related to Transparent Color of Graphic Screen 1)
	map(0x012e, 0x012f).w(FUNC(pc88va_state::text_transpen_w));
//  map(0x0130, 0x0137) Picture Mask Parameter
	map(0x0142, 0x0142).rw(FUNC(pc88va_state::idp_status_r), FUNC(pc88va_state::idp_command_w)); //Text Controller (IDP) - (R) Status (W) command
	map(0x0146, 0x0146).w(FUNC(pc88va_state::idp_param_w)); //Text Controller (IDP) - (R/W) Parameter
//  map(0x0148, 0x0149) Text control port 1
//  map(0x014c, 0x014f) Kanji CG Port, animefrm
	map(0x0150, 0x0151).r(FUNC(pc88va_state::sysop_r)); // System Operational Mode
	map(0x0152, 0x0153).rw(FUNC(pc88va_state::bios_bank_r), FUNC(pc88va_state::bios_bank_w)); // Memory Map Register
//  map(0x0154, 0x0155) Refresh Register (wait states)
	map(0x0156, 0x0156).r(FUNC(pc88va_state::rom_bank_r)); // ROM bank status
//  map(0x0158, 0x0159) Interruption Mode Modification (strobe), changes i8214 mode to i8259, cannot be changed back
//  map(0x015c, 0x015f) NMI mask port (strobe port)
//  map(0x0160, 0x016f) V50 DMAC
//  map(0x0180, 0x0180) read by Olteus
	map(0x0184, 0x0187).rw("pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
//  map(0x0188, 0x018b) V50 ICU
//  map(0x0190, 0x0191) System Port 5
//  map(0x0196, 0x0197) Keyboard sub CPU command port
	map(0x0198, 0x0199).w(FUNC(pc88va_state::backupram_wp_1_w)); //Backup RAM write inhibit
	map(0x019a, 0x019b).w(FUNC(pc88va_state::backupram_wp_0_w)); //Backup RAM write permission
//  map(0x01a0, 0x01a7) V50 TCU
	map(0x01a8, 0x01a8).w(FUNC(pc88va_state::timer3_ctrl_reg_w)); // General-purpose timer 3 control port
	map(0x01b0, 0x01b7).rw(FUNC(pc88va_state::pc88va_fdc_r), FUNC(pc88va_state::pc88va_fdc_w)).umask16(0x00ff);// FDC related (765)
	map(0x01b8, 0x01bb).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
//  map(0x01c0, 0x01c1) keyboard scan code, polled thru IRQ1 ...
	map(0x01c6, 0x01c7).nopw(); // ???
	map(0x01c8, 0x01cf).rw("d8255_3", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00); //i8255 3 (byte access)
//  map(0x01d0, 0x01d1) Expansion RAM bank selection
	map(0x0200, 0x027f).ram().share("fb_regs"); // Frame buffer 0-1-2-3 control parameter
	map(0x0300, 0x033f).ram().w(FUNC(pc88va_state::palette_ram_w)).share("palram"); // Palette RAM (xBBBBxRRRRxGGGG format)

//  map(0x0500, 0x05ff) SGP
//  map(0x1000, 0xfeff) user area (???)
	map(0xff00, 0xffff).noprw(); // CPU internal use
}

TIMER_CALLBACK_MEMBER(pc88va_state::pc8801fd_upd765_tc_to_zero)
{
	m_fdc->tc_w(false);
}

/* FDC subsytem CPU */
#if TEST_SUBFDC
void pc88va_state::pc88va_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x7fff).ram();
}

uint8_t pc88va_state::upd765_tc_r()
{
	m_fdc->tc_w(true);
	m_tc_clear_timer->adjust(attotime::from_usec(50));
	return 0;
}

void pc88va_state::fdc_irq_vector_w(uint8_t data)
{
	m_fdc_irq_opcode = data;
}

void pc88va_state::upd765_mc_w(uint8_t data)
{
	m_fdd[0]->get_device()->mon_w(!(data & 1));
	m_fdd[1]->get_device()->mon_w(!(data & 2));
}

void pc88va_state::pc88va_z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf0).w(FUNC(pc88va_state::fdc_irq_vector_w)); // Interrupt Opcode Port
//  map(0xf4, 0xf4) // Drive Control Port
	map(0xf8, 0xf8).rw(FUNC(pc88va_state::upd765_tc_r), FUNC(pc88va_state::upd765_mc_w)); // (R) Terminal Count Port (W) Motor Control Port
	map(0xfa, 0xfb).m(m_fdc, FUNC(upd765a_device::map));
	map(0xfc, 0xff).rw("d8255_2s", FUNC(i8255_device::read), FUNC(i8255_device::write));
}
#endif

void pc88va_state::opna_map(address_map &map)
{
	// TODO: confirm it really is ROMless
	// TODO: confirm size
	map(0x000000, 0x1fffff).ram();
}

/* TODO: active low or active high? */
static INPUT_PORTS_START( pc88va )
	PORT_START("KEY0")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("KEY1")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("* (PAD)") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("= (PAD)") // PORT_CODE(KEYCODE_EQUAL_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUAL_PAD))
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(", (PAD)") // PORT_CODE(KEYCODE_EQUAL_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUAL_PAD))
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(". (PAD)") // PORT_CODE(KEYCODE_EQUAL_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUAL_PAD))
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("RETURN (PAD)") //PORT_CODE(KEYCODE_RETURN_PAD) PORT_CHAR(UCHAR_MAMEKEY(RETURN_PAD))

	PORT_START("KEY2")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("@") // PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEY3")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY4")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("KEY5")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("[")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("\xC2\xA5") /* Yen */
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("]")
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("^")
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("-")

	PORT_START("KEY6")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0")  PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1 !")  PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3 #")  PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4 $")  PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5 %")  PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6 &")  PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7 '")  PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY7")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8 (")  PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9 )")  PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(": *")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("; +")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(", <")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(". >")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/ ?")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/ ?")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("KEY8")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CLR") // PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Up")  PORT_CODE(KEYCODE_UP)  /* Up */
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)  /* Right */
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("INSDEL")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("GRPH")
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("KEY9")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("STOP") // PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F1 (mirror)") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F2 (mirror)") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F3 (mirror)") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F4 (mirror)") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F5 (mirror)") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SPACES") // PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)

	PORT_START("KEYA")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)  /* Down */
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)  /* Left */
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("HELP") // PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("COPY") // PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("- (mirror)") // PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/") // PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("KEYB")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD)  PORT_NAME("Roll Up")  PORT_CODE(KEYCODE_PGUP)  /* Roll Up */
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD)  PORT_NAME("Roll Down")  PORT_CODE(KEYCODE_PGDN)  /* Roll Down */
	PORT_BIT(0xfc,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYC")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)

	PORT_START("KEYD")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) // Conversion?
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) // Decision?
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)

	/* TODO: I don't understand the meaning of several of these */
	PORT_START("KEYE")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD)
	PORT_BIT(0xc0,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYF")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "CRT Mode" )
	PORT_DIPSETTING(    0x01, "15.7 KHz" )
	PORT_DIPSETTING(    0x00, "24.8 KHz" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SPEED_SW")
	PORT_DIPNAME( 0x01, 0x01, "Speed Mode" )
	PORT_DIPSETTING(    0x01, "H Mode" )
	PORT_DIPSETTING(    0x00, "S Mode" )

	PORT_START("SYSOP_SW")
	PORT_DIPNAME( 0x03, 0x01, "System Operational Mode" )
//  PORT_DIPSETTING(    0x00, "Reserved" )
	PORT_DIPSETTING(    0x01, "N88 V2 Mode" )
	PORT_DIPSETTING(    0x02, "N88 V1 Mode" )
//  PORT_DIPSETTING(    0x03, "???" )

	PORT_START("OPN_PA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OPN_PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 2")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout pc88va_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout pc88va_chars_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

// same as above but with static size
static const gfx_layout pc88va_kanji_8x8 =
{
	8,8,
	0x4000/8,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout pc88va_kanji_16x16 =
{
	16,16,
	0x4000/32,
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

// debug only
static GFXDECODE_START( gfx_pc88va )
	GFXDECODE_ENTRY( "kanji",   0x00000, pc88va_chars_8x8,    0, 1 )
	GFXDECODE_ENTRY( "kanji",   0x00000, pc88va_chars_16x16,  0, 1 )
	GFXDECODE_ENTRY( nullptr,   0x00000, pc88va_kanji_8x8,    0, 1 )
	GFXDECODE_ENTRY( nullptr,   0x00000, pc88va_kanji_16x16,  0, 1 )
GFXDECODE_END

uint8_t pc88va_state::cpu_8255_c_r()
{
	return m_i8255_1_pc >> 4;
}

void pc88va_state::cpu_8255_c_w(uint8_t data)
{
	m_i8255_0_pc = data;
}

uint8_t pc88va_state::fdc_8255_c_r()
{
	return m_i8255_0_pc >> 4;
}

void pc88va_state::fdc_8255_c_w(uint8_t data)
{
	m_i8255_1_pc = data;
}

uint8_t pc88va_state::r232_ctrl_porta_r()
{
	uint8_t sw5, sw4, sw3, sw2,speed_sw;

	speed_sw = (ioport("SPEED_SW")->read() & 1) ? 0x20 : 0x00;
	sw5 = (ioport("DSW")->read() & 0x10);
	sw4 = (ioport("DSW")->read() & 0x08);
	sw3 = (ioport("DSW")->read() & 0x04);
	sw2 = (ioport("DSW")->read() & 0x02);

	return 0xc1 | sw5 | sw4 | sw3 | sw2 | speed_sw;
}

uint8_t pc88va_state::r232_ctrl_portb_r()
{
	uint8_t xsw1;

	xsw1 = (ioport("DSW")->read() & 1) ? 0 : 8;

	return 0xf7 | xsw1;
}

uint8_t pc88va_state::r232_ctrl_portc_r()
{
	return 0xff;
}

void pc88va_state::r232_ctrl_porta_w(uint8_t data)
{
	// ...
}

void pc88va_state::r232_ctrl_portb_w(uint8_t data)
{
	// ...
}

void pc88va_state::r232_ctrl_portc_w(uint8_t data)
{
	// ...
}

uint8_t pc88va_state::get_slave_ack(offs_t offset)
{
	if (offset==7) { // IRQ = 7
		return m_pic2->acknowledge();
	}
	return 0x00;
}

void pc88va_state::machine_start()
{
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	m_tc_clear_timer = timer_alloc(FUNC(pc88va_state::pc8801fd_upd765_tc_to_zero), this);
	m_tc_clear_timer->adjust(attotime::never);

	m_fdc_timer = timer_alloc(FUNC(pc88va_state::pc88va_fdc_timer), this);
	m_fdc_timer->adjust(attotime::never);

	m_motor_start_timer[0] = timer_alloc(FUNC(pc88va_state::pc88va_fdc_motor_start_0), this);
	m_motor_start_timer[1] = timer_alloc(FUNC(pc88va_state::pc88va_fdc_motor_start_1), this);
	m_motor_start_timer[0]->adjust(attotime::never);
	m_motor_start_timer[1]->adjust(attotime::never);

	m_t3_mouse_timer = timer_alloc(FUNC(pc88va_state::t3_mouse_callback), this);
	m_t3_mouse_timer->adjust(attotime::never);

	floppy_image_device *floppy;
	floppy = m_fdd[0]->get_device();
	if(floppy)
		floppy->setup_ready_cb(floppy_image_device::ready_cb(&pc88va_state::pc88va_fdc_update_ready, this));

	floppy = m_fdd[1]->get_device();
	if(floppy)
		floppy->setup_ready_cb(floppy_image_device::ready_cb(&pc88va_state::pc88va_fdc_update_ready, this));

	m_fdd[0]->get_device()->set_rpm(300);
	m_fdd[1]->get_device()->set_rpm(300);
	m_fdc->set_rate(250000);
}

void pc88va_state::machine_reset()
{
	uint8_t *ROM00 = memregion("rom00")->base();
	uint8_t *ROM10 = memregion("rom10")->base();

	membank("rom10_bank")->set_base(&ROM10[0x00000]);
	membank("rom00_bank")->set_base(&ROM00[0x00000]);

	m_bank_reg = 0x4100;
	m_sysbank->set_bank(1);
	m_backupram_wp = 1;

	/* default palette */
	{
		uint8_t i;
		for(i=0;i<32;i++)
			m_palette->set_pen_color(i,pal1bit((i & 2) >> 1),pal1bit((i & 4) >> 2),pal1bit(i & 1));
	}

	m_tsp.tvram_vreg_offset = 0;

	m_fdc_mode = 0;
	m_fdc_irq_opcode = 0x00; //0x7f ld a,a !

	#if TEST_SUBFDC
	m_fdccpu->set_input_line_vector(0, 0); // Z80
	#endif

	m_fdc_motor_status[0] = 0;
	m_fdc_motor_status[1] = 0;

	m_misc_ctrl = 0x80;
	m_sound_irq_enable = false;
	m_sound_irq_pending = false;
}

INTERRUPT_GEN_MEMBER(pc88va_state::pc88va_vrtc_irq)
{
	// TODO: verify when ack should happen
	m_maincpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
	m_maincpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
}

WRITE_LINE_MEMBER( pc88va_state::fdc_irq )
{
	if(m_fdc_mode)
	{
		// TODO: ugh why it doesn't follow state?
		//printf("%d\n",state);
		m_pic2->ir3_w(0);
		m_pic2->ir3_w(1);
	}
	#if TEST_SUBFDC
	else
		m_fdccpu->set_input_line(0, HOLD_LINE);
	#endif
}

WRITE_LINE_MEMBER( pc88va_state::tc_w )
{
	m_fdc->tc_w(state);
}

void pc88va_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_XDF_FORMAT);
	fr.add(FLOPPY_PC98FDI_FORMAT);
}

static void pc88va_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

// TODO: often dies
WRITE_LINE_MEMBER(pc88va_state::int4_irq_w)
{
	bool irq_state = m_sound_irq_enable & state;

	m_pic2->ir4_w(irq_state);
//  m_pic->r_w(7 ^ INT4_IRQ_LEVEL, !irq_state);
	m_sound_irq_pending = state;
}

void pc88va_state::pc88va(machine_config &config)
{
	V50(config, m_maincpu, MASTER_CLOCK); // μPD9002, aka V50 + μPD70008AC (for PC8801 compatibility mode) in place of 8080
	m_maincpu->set_addrmap(AS_PROGRAM, &pc88va_state::pc88va_map);
	m_maincpu->set_addrmap(AS_IO, &pc88va_state::pc88va_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(pc88va_state::pc88va_vrtc_irq));
	m_maincpu->icu_slave_ack_cb().set(m_pic2, FUNC(pic8259_device::acknowledge));
//  m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->set_tclk(MASTER_CLOCK);
	// "timer 1"
	m_maincpu->tout1_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
//  m_pit->out_handler<0>().set(m_pic1, FUNC(pic8259_device::ir0_w));
	// ch2 is FDC, ch0/3 are "user". ch1 is unused
	m_maincpu->out_hreq_cb().set(m_maincpu, FUNC(v50_device::hack_w));
	m_maincpu->out_eop_cb().set(FUNC(pc88va_state::tc_w));
	m_maincpu->in_ior_cb<2>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_maincpu->out_iow_cb<2>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_maincpu->in_memr_cb().set([this] (offs_t offset) { return m_maincpu->space(AS_PROGRAM).read_byte(offset); });
	m_maincpu->out_memw_cb().set([this] (offs_t offset, u8 data) { m_maincpu->space(AS_PROGRAM).write_byte(offset, data); });

#if TEST_SUBFDC
	z80_device &fdccpu(Z80(config, "fdccpu", 4000000));        /* 8 MHz */
	fdccpu.set_addrmap(AS_PROGRAM, &pc88va_state::pc88va_z80_map);
	fdccpu.set_addrmap(AS_IO, &pc88va_state::pc88va_z80_io_map);

	config.m_perfect_cpu_quantum = subtag("maincpu");
#endif

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: fully convert to set_raw (timings available)
	m_screen->set_raw(XTAL(42'105'200) / 2, 848, 0, 640, 448, 0, 400);
	m_screen->set_screen_update(FUNC(pc88va_state::screen_update));

	PALETTE(config, m_palette).set_entries(32);
//  m_palette->set_init(FUNC(pc88va_state::pc8801));
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pc88va);

	i8255_device &d8255_2(I8255(config, "d8255_2"));
	d8255_2.in_pa_callback().set("d8255_2s", FUNC(i8255_device::pb_r));
	d8255_2.in_pb_callback().set("d8255_2s", FUNC(i8255_device::pa_r));
	d8255_2.in_pc_callback().set(FUNC(pc88va_state::cpu_8255_c_r));
	d8255_2.out_pc_callback().set(FUNC(pc88va_state::cpu_8255_c_w));

	i8255_device &d8255_3(I8255(config, "d8255_3"));
	d8255_3.in_pa_callback().set(FUNC(pc88va_state::r232_ctrl_porta_r));
	d8255_3.out_pa_callback().set(FUNC(pc88va_state::r232_ctrl_porta_w));
	d8255_3.in_pb_callback().set(FUNC(pc88va_state::r232_ctrl_portb_r));
	d8255_3.out_pb_callback().set(FUNC(pc88va_state::r232_ctrl_portb_w));
	d8255_3.in_pc_callback().set(FUNC(pc88va_state::r232_ctrl_portc_r));
	d8255_3.out_pc_callback().set(FUNC(pc88va_state::r232_ctrl_portc_w));

	i8255_device &d8255_2s(I8255(config, "d8255_2s"));
	d8255_2s.in_pa_callback().set("d8255_2", FUNC(i8255_device::pb_r));
	d8255_2s.in_pb_callback().set("d8255_2", FUNC(i8255_device::pa_r));
	d8255_2s.in_pc_callback().set(FUNC(pc88va_state::fdc_8255_c_r));
	d8255_2s.out_pc_callback().set(FUNC(pc88va_state::fdc_8255_c_w));

#if 0
	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic1->in_sp_callback().set_constant(1);
	m_pic1->read_slave_ack_callback().set(FUNC(pc88va_state::get_slave_ack));
#endif

	PIC8259(config, m_pic2, 0);
	m_pic2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ7);
	m_pic2->in_sp_callback().set_constant(0);

	UPD765A(config, m_fdc, 4000000, true, true);
	m_fdc->intrq_wr_callback().set(FUNC(pc88va_state::fdc_irq));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v50_device::dreq_w<2>));
	FLOPPY_CONNECTOR(config, m_fdd[0], pc88va_floppies, "525hd", pc88va_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[1], pc88va_floppies, "525hd", pc88va_state::floppy_formats).enable_sound(true);
	SOFTWARE_LIST(config, "disk_list").set_original("pc88va");

	UPD4990A(config, m_rtc);

#if 0
	pit8253_device &pit8253(PIT8253(config, "pit8253", 0));
	pit8253.set_clk<0>(MASTER_CLOCK); /* general purpose timer 1 */
	pit8253.out_handler<0>().set(FUNC(pc88va_state::pc88va_pit_out0_changed));
	pit8253.set_clk<1>(MASTER_CLOCK); /* BEEP frequency setting */
	pit8253.set_clk<2>(MASTER_CLOCK); /* RS232C baud rate setting */
#endif

	ADDRESS_MAP_BANK(config, "sysbank").set_map(&pc88va_state::sysbank_map).set_options(ENDIANNESS_LITTLE, 16, 18+4, 0x40000);

	SPEAKER(config, m_lspeaker).front_left();
	SPEAKER(config, m_rspeaker).front_right();

	YM2608(config, m_opna, FM_CLOCK);
	m_opna->set_addrmap(0, &pc88va_state::opna_map);
	m_opna->irq_handler().set(FUNC(pc88va_state::int4_irq_w));
	// TODO: DE-9
//  m_opna->port_a_read_callback().set(FUNC(pc8801fh_state::opn_porta_r));
	m_opna->port_a_read_callback().set_ioport("OPN_PA");
	m_opna->port_b_read_callback().set_ioport("OPN_PB");
	// TODO: per-channel mixing is unconfirmed
	m_opna->add_route(0, m_lspeaker, 0.25);
	m_opna->add_route(0, m_rspeaker, 0.25);
	m_opna->add_route(1, m_lspeaker, 0.75);
	m_opna->add_route(2, m_rspeaker, 0.75);
}


ROM_START( pc88va2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x100000, "fdccpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vasubsys.rom", 0x0000, 0x2000, CRC(08962850) SHA1(a9375aa480f85e1422a0e1385acb0ea170c5c2e0) )

	ROM_REGION( 0x100000, "rom00", ROMREGION_ERASEFF ) // 0xe0000 - 0xeffff
	ROM_LOAD( "varom00_va2.rom",   0x00000, 0x80000, CRC(98c9959a) SHA1(bcaea28c58816602ca1e8290f534360f1ca03fe8) )
	ROM_LOAD( "varom08_va2.rom",   0x80000, 0x20000, CRC(eef6d4a0) SHA1(47e5f89f8b0ce18ff8d5d7b7aef8ca0a2a8e3345) )

	ROM_REGION( 0x20000, "rom10", 0 ) // 0xf0000 - 0xfffff
	ROM_LOAD( "varom1_va2.rom",    0x00000, 0x20000, CRC(7e767f00) SHA1(dd4f4521bfbb068f15ab3bcdb8d47c7d82b9d1d4) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION16_LE( 0x80000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "vafont_va2.rom", 0x00000, 0x50000, BAD_DUMP CRC(b40d34e4) SHA1(a0227d1fbc2da5db4b46d8d2c7e7a9ac2d91379f) ) // should be splitted

	ROM_REGION16_LE( 0x80000, "dictionary", 0 )
	ROM_LOAD( "vadic_va2.rom", 0x00000, 0x80000, CRC(a6108f4d) SHA1(3665db538598abb45d9dfe636423e6728a812b12) )
ROM_END

ROM_START( pc88va )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x100000, "fdccpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vasubsys.rom", 0x0000, 0x2000, CRC(08962850) SHA1(a9375aa480f85e1422a0e1385acb0ea170c5c2e0) )

	ROM_REGION( 0x100000, "rom00", ROMREGION_ERASEFF ) // 0xe0000 - 0xeffff
	ROM_LOAD( "varom00.rom", 0x00000, 0x80000, CRC(8a853b00) SHA1(1266ba969959ff25433ecc900a2caced26ef1a9e))
	ROM_LOAD( "varom08.rom", 0x80000, 0x20000, CRC(154803cc) SHA1(7e6591cd465cbb35d6d3446c5a83b46d30fafe95))

	ROM_REGION( 0x20000, "rom10", 0 ) // 0xf0000 - 0xfffff
	ROM_LOAD( "varom1.rom", 0x00000, 0x20000, CRC(0783b16a) SHA1(54536dc03238b4668c8bb76337efade001ec7826))

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION16_LE( 0x80000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "vafont.rom", 0x0000, 0x50000, BAD_DUMP CRC(faf7c466) SHA1(196b3d5b7407cb4f286ffe5c1e34ebb1f6905a8c)) // should be splitted

	ROM_REGION16_LE( 0x80000, "dictionary", 0 )
	ROM_LOAD( "vadic.rom",  0x0000, 0x80000, CRC(f913c605) SHA1(5ba1f3578d0aaacdaf7194a80e6d520c81ae55fb))
ROM_END




COMP( 1987, pc88va,  0,      0, pc88va, pc88va, pc88va_state, empty_init, "NEC", "PC-88VA",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1988, pc88va2, pc88va, 0, pc88va, pc88va, pc88va_state, empty_init, "NEC", "PC-88VA2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
//COMP( 1988, pc88va3, pc88va, 0, pc88va, pc88va, pc88va_state, empty_init, "NEC", "PC-88VA3", MACHINE_NOT_WORKING )
