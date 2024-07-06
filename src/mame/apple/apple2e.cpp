// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    apple2e.cpp - Apple IIe/IIc/IIc Plus and clones

    Next generation driver written in September/October 2014 by R. Belmont.
    Thanks to the original Apple II series driver's authors: Mike Balfour, Nathan Woods, and R. Belmont.
    Special thanks to the Apple II Documentation Project/Antoine Vignau and Peter Ferrie.


    IIe: base of this driver.  64K RAM, slot 0 language card emulation without the double-read requirement,
         lowercase and SHIFT key on button 2, Open and Solid Apple buttons on joy buttons 0 and 1,
         auxiliary slot, built-in 80 column support if extra RAM added.

         Physical slot 0 was eliminated thanks to the built-in language card.

         Most of the write-only softswitches gained readback locations, necessary to make interrupt-driven
         software possible.

         Base 80-column card: 1K RAM, allows 80 columns and double-lo-res,
         no double-hi-res.

         Extended 80-column card: 64K RAM (including a second language card),
         allows 80 columns, double-lo-res, and double-hi-res.

         Revision A motherboards (very rare) don't support double-hi-res; it's unclear
         if double-lo-res works or not.  We emulate the much more common Rev B or later
         board.

    IIe enhanced: 65C02 CPU with more instructions, MouseText in the character generator.

    IIe platinum: Like enhanced but with added numeric keypad and extended 80-column card
         included in the box.  Keypad CLEAR generates ESC by default, one hardware mod
         made it generate CTRL-X instead.  (new keyboard encoder ROM?)

    NOTE: On real IIe and IIe enhanced h/w, pressing SHIFT and paddle button 2 will
    short out the power supply and cause a safety shutdown.  (We don't emulate
    this "feature", and it was relatively rare in real life as Apple joysticks only
    had buttons 0 and 1 normally).

    IIc: IIe enhanced shrunken into a pizzabox with a Disk II compatible
         half-height drive included in the case.

     No slots, but included functionality equivalent to the following slots
     on the motherboard:
     - 2 Super Serial Cards (modem and printer ports)
     - extended 80 column card / 128K RAM
     - Disk II IWM controller
     - Apple II Mouse card (firmware entry points are compatible,
       but the hardware implementation omits the 68705 and is quite different!)

     Has a 40/80 column switch and a QWERTY/DVORAK switch.

    IIc (UniDisk 3.5): IIc with ROM doubled to 32K and the ROMSWITCH register
         added to page between the original 16K ROM and the new added 16K.  The
         extra firmware space was dedicated to implementing the Protocol Converter,
         later renamed "SmartPort", which communicates with "smart" packet devices
         over the IWM bus.

         Partial AppleTalk code also exists in this ROM but it doesn't work and
         was not completed.

    IIc (Original Memory Expansion):
         Removes AppleTalk and adds support for a memory expansion card with up
         to 1 MB; this is identical both in hardware and firmware to the "Slinky"
         memory expansion card for the Apple IIe (a2bus/a2memexp.c).

    IIc (Revised Memory Expansion, Rev. 3):
        Fixes several nasty bugs in the Original Memory Expansion version.  Not
        currently dumped.

    IIc (Rev 4):
        Fixes memory size detection for memory cards with less than 1MB.  Fixes
        several screen hole errors introduced in Rev 3, and fixes Terminal Mode
        wherein the firmware can be put into a built-in terminal mode for simple
        tests with a modem.

    IIc Plus:
        Like IIc with memory expansion, but with licensed built-in Zip Chip which
        runs the 65C02 at 4 MHz turbo speed with a small cache RAM.

        The machine has an internal "Apple 3.5" drive plus a custom chip
        named "MIG" (Multidrive Interface Glue) which helps with the control
        of the drive.  This gets around the fact that 1 MHz isn't
        sufficient to handle direct Woz-style control of a double-density
        3.5" drive.

        External drive port allows IIgs-style daisy-chaining.

----------------------------------

TK3000 keyboard matrix:
Data bus D0-D7 is X0-X7
Address bus A0-A11 is Y0-Y11

IIc Plus CGGA speed control:
Fast: Store $08 to $C05B (Zip compatible)
Normal: Store $08 to $C05A (Zip compatible)
Lock: Store $A5 to $C05A (Zip compatible).
Unlock: Store $5A 4 times in a row to $C05A (Zip compatible).
Read state: Cached by the firmware in MIG RAM (see below).
Write state: Store speaker/slot byte at $C05C (Zip compatible).  Firmware always enables the paddle
delay at $C05F, regardless of what you pass in.

Previous versions of this driver stated that the IIc+ speaker/slot control byte was not Zip compatible; that
was a misunderstanding.

Accelerator control firmware saves/restores zero page locations 0-7 in MIG RAM page 2 locations $CE10-$CE17.
MIG RAM page 2 $CE02 is the speaker/slot bitfield and $CE03 is the paddle/accelerator bitfield.

***************************************************************************/

#include "emu.h"

#include "apple2video.h"
#include "apple2common.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/applefdintf.h"
#include "machine/ds1315.h"
#include "machine/iwm.h"
#include "machine/kb3600.h"
#include "machine/mos6551.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"

#include "bus/a2bus/cards.h"
#include "bus/a2bus/a2mockingboard.h"
#include "bus/a2bus/a2diskiing.h"
#include "bus/a2bus/a2iwm.h"
#include "bus/a2bus/laser128.h"
#include "bus/a2bus/ace2x00.h"
#include "bus/a2bus/a2eauxslot.h"
#include "bus/a2bus/a2eext80col.h"
#include "bus/a2bus/a2eramworks3.h"
#include "bus/a2bus/a2estd80col.h"
#include "bus/a2gameio/gameio.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"


namespace {

#define A2_CPU_TAG "maincpu"
#define A2_KBDC_TAG "ay3600"
#define A2_BUS_TAG "a2bus"
#define A2_SPEAKER_TAG "speaker"
#define A2_CASSETTE_TAG "tape"
#define A2_UPPERBANK_TAG "inhbank"
#define IIC_ACIA1_TAG "acia1"
#define IIC_ACIA2_TAG "acia2"
#define LASER128_UDC_TAG "l128udc"
#define PRINTER_PORT_TAG "printer"
#define MODEM_PORT_TAG "modem"
#define A2_AUXSLOT_TAG "auxbus"
#define A2_VIDEO_TAG "a2video"

#define A2_0000_TAG "r00bank"
#define A2_0200_TAG "r02bank"
#define A2_0400_TAG "r04bank"
#define A2_0800_TAG "r08bank"
#define A2_2000_TAG "r20bank"
#define A2_4000_TAG "r40bank"
#define A2_C100_TAG "c1bank"
#define A2_C300_TAG "c3bank"
#define A2_C400_TAG "c4bank"
#define A2_C800_TAG "c8bank"
#define A2_LCBANK_TAG "lcbank"

#define MOUSE_BUTTON_TAG    "mse_button"
#define MOUSE_XAXIS_TAG     "mse_x"
#define MOUSE_YAXIS_TAG     "mse_y"

#define CNXX_UNCLAIMED  -1
#define CNXX_INTROM     -2

static constexpr int IRQ_SLOT = 0;
static constexpr int IRQ_VBL = 1;
static constexpr int IRQ_MOUSEXY = 2;

class apple2e_state : public driver_device
{
public:
	apple2e_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, A2_CPU_TAG),
		m_screen(*this, "screen"),
		m_scantimer(*this, "scantimer"),
		m_acceltimer(*this, "acceltimer"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, "maincpu"),
		m_a2common(*this, "a2common"),
		m_cecbanks(*this, "cecexp"),
		m_ay3600(*this, A2_KBDC_TAG),
		m_video(*this, A2_VIDEO_TAG),
		m_a2bus(*this, A2_BUS_TAG),
		m_a2eauxslot(*this, A2_AUXSLOT_TAG),
		m_gameio(*this, "gameio"),
		m_mouseb(*this, MOUSE_BUTTON_TAG),
		m_mousex(*this, MOUSE_XAXIS_TAG),
		m_mousey(*this, MOUSE_YAXIS_TAG),
		m_kbdrom(*this, "keyboard"),
		m_kbspecial(*this, "keyb_special"),
		m_sysconfig(*this, "a2_config"),
		m_franklin_fkeys(*this, "franklin_fkeys"),
		m_speaker(*this, A2_SPEAKER_TAG),
		m_cassette(*this, A2_CASSETTE_TAG),
		m_upperbank(*this, A2_UPPERBANK_TAG),
		m_0000bank(*this, A2_0000_TAG),
		m_0200bank(*this, A2_0200_TAG),
		m_0400bank(*this, A2_0400_TAG),
		m_0800bank(*this, A2_0800_TAG),
		m_2000bank(*this, A2_2000_TAG),
		m_4000bank(*this, A2_4000_TAG),
		m_c100bank(*this, A2_C100_TAG),
		m_c300bank(*this, A2_C300_TAG),
		m_c400bank(*this, A2_C400_TAG),
		m_c800bank(*this, A2_C800_TAG),
		m_lcbank(*this, A2_LCBANK_TAG),
		m_acia1(*this, IIC_ACIA1_TAG),
		m_acia2(*this, IIC_ACIA2_TAG),
		m_iwm(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_ds1315(*this, "nsc"),
		m_printer_conn(*this, "parallel"),
		m_printer_out(*this, "laserprnout")
	{
		m_accel_laser = false;
		m_has_laser_mouse = false;
		m_isiic = false;
		m_isiicplus = false;
		m_iscec = false;
		m_iscecm = false;
		m_iscec2000 = false;
		m_isace500 = false;
		m_isace2200 = false;
		m_ace2200_axxx_bank = false;
		m_pal = false;
		m_cur_floppy = nullptr;
		m_devsel = 0;
		m_laser_speed = 0;
		m_laser_fdc_on = false;
	}

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<timer_device> m_scantimer, m_acceltimer;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_device<apple2_common_device> m_a2common;
	optional_memory_region m_cecbanks;
	required_device<ay3600_device> m_ay3600;
	required_device<a2_video_device> m_video;
	required_device<a2bus_device> m_a2bus;
	optional_device<a2eauxslot_device> m_a2eauxslot;
	required_device<apple2_gameio_device> m_gameio;
	optional_ioport m_mouseb, m_mousex, m_mousey;
	optional_memory_region m_kbdrom;
	required_ioport m_kbspecial;
	optional_ioport m_sysconfig;
	optional_ioport m_franklin_fkeys;
	required_device<speaker_sound_device> m_speaker;
	optional_device<cassette_image_device> m_cassette;
	memory_view m_upperbank, m_0000bank, m_0200bank, m_0400bank;
	memory_view m_0800bank, m_2000bank, m_4000bank, m_c100bank;
	memory_view m_c300bank, m_c400bank, m_c800bank, m_lcbank;
	optional_device<mos6551_device> m_acia1, m_acia2;
	optional_device<applefdintf_device> m_iwm;
	optional_device_array<floppy_connector, 4> m_floppy;
	required_device<ds1315_device> m_ds1315;
	optional_device<centronics_device>      m_printer_conn;
	optional_device<output_latch_device>    m_printer_out;

	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(accel_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(ay3600_repeat);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	u8 ram0000_r(offs_t offset);
	void ram0000_w(offs_t offset, u8 data);
	u8 ram0200_r(offs_t offset);
	void ram0200_w(offs_t offset, u8 data);
	u8 ram0400_r(offs_t offset);
	void ram0400_w(offs_t offset, u8 data);
	u8 ram0800_r(offs_t offset);
	void ram0800_w(offs_t offset, u8 data);
	u8 ram2000_r(offs_t offset);
	void ram2000_w(offs_t offset, u8 data);
	u8 ram4000_r(offs_t offset);
	u8 ram4000_ace2200_r(offs_t offset);
	void ram4000_w(offs_t offset, u8 data);
	u8 cec4000_r(offs_t offset);
	u8 cec8000_r(offs_t offset);
	void ram8000_w(offs_t offset, u8 data);
	u8 auxram0000_r(offs_t offset);
	void auxram0000_w(offs_t offset, u8 data);
	u8 auxram0200_r(offs_t offset);
	void auxram0200_w(offs_t offset, u8 data);
	u8 auxram0400_r(offs_t offset);
	void auxram0400_w(offs_t offset, u8 data);
	u8 auxram0800_r(offs_t offset);
	void auxram0800_w(offs_t offset, u8 data);
	u8 auxram2000_r(offs_t offset);
	void auxram2000_w(offs_t offset, u8 data);
	u8 auxram4000_r(offs_t offset);
	void auxram4000_w(offs_t offset, u8 data);
	u8 c000_r(offs_t offset);
	void c000_w(offs_t offset, u8 data);
	u8 c000_laser_r(offs_t offset);
	void c000_laser_w(offs_t offset, u8 data);
	u8 laserprn_busy_r();
	void laserprn_w(u8 data);
	TIMER_CALLBACK_MEMBER(update_laserprn_strobe);
	u8 c000_iic_r(offs_t offset);
	void c000_iic_w(offs_t offset, u8 data);
	u8 c080_r(offs_t offset);
	void c080_w(offs_t offset, u8 data);
	u8 c100_r(offs_t offset);
	u8 c100_int_r(offs_t offset);
	u8 c100_int_bank_r(offs_t offset);
	u8 c100_cec_r(offs_t offset);
	u8 c100_cec_bank_r(offs_t offset);
	void c100_w(offs_t offset, u8 data);
	u8 c300_r(offs_t offset);
	u8 c300_int_r(offs_t offset);
	u8 c300_int_bank_r(offs_t offset);
	u8 c300_cec_r(offs_t offset);
	u8 c300_cec_bank_r(offs_t offset);
	void c300_w(offs_t offset, u8 data);
	u8 c400_r(offs_t offset);
	u8 c400_int_r(offs_t offset);
	u8 c400_int_bank_r(offs_t offset);
	u8 c400_cec_r(offs_t offset);
	u8 c400_cec_bank_r(offs_t offset);
	void c400_w(offs_t offset, u8 data);
	void c400_cec_w(offs_t offset, u8 data);
	u8 c800_r(offs_t offset);
	u8 c800_int_r(offs_t offset);
	u8 c800_cec_r(offs_t offset);
	u8 c800_cec_bank_r(offs_t offset);
	u8 c800_b2_int_r(offs_t offset);
	void c800_w(offs_t offset, u8 data);
	u8 inh_r(offs_t offset);
	void inh_w(offs_t offset, u8 data);
	u8 lc_r(offs_t offset);
	void lc_w(offs_t offset, u8 data);
	u8 lc_romswitch_r(offs_t offset);
	void lc_romswitch_w(offs_t offset, u8 data);
	u8 laser_mouse_r(offs_t offset);
	void laser_mouse_w(offs_t offset, u8 data);
	void a2bus_irq_w(int state);
	void a2bus_nmi_w(int state);
	void a2bus_inh_w(int state);
	void busy_w(int state);
	int ay3600_shift_r();
	int ay3600_control_r();
	void ay3600_data_ready_w(int state);
	void ay3600_ako_w(int state);
	u8 memexp_r(offs_t offset);
	void memexp_w(offs_t offset, u8 data);
	u8 nsc_backing_r(offs_t offset);
	u8 ace500_c0bx_r(offs_t offset);
	void ace500_c0bx_w(offs_t offset, u8 data);

	void apple2cp(machine_config &config);
	void spectred(machine_config &config);
	void laser128(machine_config &config);
	void laser128o(machine_config &config);
	void laser128ex2(machine_config &config);
	void ace500(machine_config &config);
	void ace2200(machine_config &config);
	void apple2c_iwm(machine_config &config);
	void apple2c_mem(machine_config &config);
	void cec(machine_config &config);
	void mprof3(machine_config &config);
	void apple2e(machine_config &config);
	void apple2epal(machine_config &config);
	void apple2ep(machine_config &config);
	void apple2c(machine_config &config);
	void tk3000(machine_config &config);
	void apple2ee(machine_config &config);
	void apple2eepal(machine_config &config);
	void apple2c_map(address_map &map);
	void apple2c_memexp_map(address_map &map);
	void base_map(address_map &map);
	void laser128_map(address_map &map);
	void ace500_map(address_map &map);
	void ace2200_map(address_map &map);
	void spectred_keyb_map(address_map &map);
	void init_laser128();
	void init_128ex();
	void init_pal();
	void init_ace500();
	void init_ace2200();

	bool m_35sel, m_hdsel, m_intdrive;

private:
	int m_speaker_state, m_cassette_state, m_cassette_out;

	double m_joystick_x1_time, m_joystick_y1_time, m_joystick_x2_time, m_joystick_y2_time;

	u32 m_franklin_last_fkeys;
	u16 m_lastchar, m_strobe, m_franklin_strobe;
	u8 m_transchar;
	bool m_anykeydown;
	int m_repeatdelay;

	int m_inh_slot, m_cnxx_slot;

	bool m_an0, m_an1, m_an2, m_an3;

	bool m_vbl, m_vblmask;

	bool m_xy, m_x0edge, m_y0edge;
	bool m_x0, m_x1, m_y0, m_y1;
	bool m_xirq, m_yirq;
	int last_mx, last_my, count_x, count_y;

	bool m_intcxrom;
	bool m_slotc3rom;
	bool m_altzp;
	bool m_ramrd, m_ramwrt;
	bool m_lcram, m_lcram2, m_lcprewrite, m_lcwriteenable;
	bool m_ioudis;
	bool m_romswitch;
	bool m_mockingboard4c;
	bool m_intc8rom;
	bool m_reset_latch;

	bool m_isiic, m_isiicplus, m_iscec, m_iscecm, m_iscec2000, m_pal;
	u8 m_migram[0x800];
	u16 m_migpage;

	bool m_isace500, m_isace2200, m_ace_cnxx_bank, m_ace2200_axxx_bank;
	u16 m_ace500rombank;

	bool m_accel_unlocked;
	bool m_accel_fast;
	bool m_accel_present;
	bool m_accel_temp_slowdown;
	bool m_accel_laser;
	bool m_has_laser_mouse;
	bool m_laser_fdc_on;
	int m_accel_stage;
	u32 m_accel_speed;
	u8 m_accel_slotspk, m_accel_gameio, m_laser_speed;

	emu_timer *m_strobe_timer;
	u8  m_next_strobe;
	bool m_centronics_busy;

	u8 *m_ram_ptr, *m_rom_ptr, *m_cec_ptr;
	int m_ram_size;

	int m_cec_bank;

	u8 *m_aux_ptr, *m_aux_bank_ptr;
	u16 m_aux_mask;

	int m_inh_bank;

	double m_x_calibration, m_y_calibration;

	device_a2bus_card_interface *m_slotdevice[8];
	device_a2eauxslot_card_interface *m_auxslotdevice;

	int m_irqmask;

	u8 m_exp_bankhior;
	int m_exp_addrmask;
	u8 m_exp_regs[0x10];
	u8 *m_exp_ram;
	int m_exp_wptr, m_exp_liveptr;

	void do_io(int offset, bool is_iic);
	u8 read_floatingbus();
	void update_slotrom_banks();
	void lc_update(int offset, bool writing);
	u8 read_slot_rom(int slotbias, int offset);
	void write_slot_rom(int slotbias, int offset, u8 data);
	u8 read_int_rom(int slotbias, int offset);
	void auxbank_update();
	void lcrom_update();
	void cec_lcrom_update();
	void raise_irq(int irq);
	void lower_irq(int irq);
	void update_iic_mouse();
	void accel_full_speed();
	void accel_normal_speed();
	void accel_slot(int slot);
	void laser_calc_speed();

	u8 m_cec_remap[0x40000];

	u8 mig_r(u16 offset);
	void mig_w(u16 offset, u8 data);
	void phases_w(uint8_t phases);
	void sel35_w(int sel35);
	void devsel_w(uint8_t devsel);
	void hdsel_w(int hdsel);
	void recalc_active_device();

	floppy_image_device *m_cur_floppy;
	int m_devsel;

	u8 laser_motor_r(offs_t offset)
	{
		m_laser_fdc_on = (offset == 1);
		laser_calc_speed();
		return m_iwm->read(offset + 8);
	}

	void laser_motor_w(offs_t offset, u8 data)
	{
		m_laser_fdc_on = (offset == 1);
		laser_calc_speed();
		m_iwm->write(offset + 8, data);
	}

	offs_t dasm_trampoline(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

	void apple2e_common(machine_config &config, bool enhanced, bool rgb_option);
};


offs_t apple2e_state::dasm_trampoline(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	return m_a2common->dasm_override(stream, pc, opcodes, params);
}

u8 apple2e_state::mig_r(u16 offset)
{
	//printf("mig_r @ %x\n", offset + 0xc00);
	// MIG RAM window
	if ((offset >= 0x200) && (offset < 0x220))
	{
		return m_migram[m_migpage + (offset & 0x1f)];
	}

	// increment MIG RAM window and return previous value
	if ((offset >= 0x220) && (offset < 0x240))
	{
		u8 rv = m_migram[m_migpage + (offset & 0x1f)];
		m_migpage += 0x20;
		m_migpage &= 0x7ff;
		return rv;
	}

	if ((offset >= 0x240) && (offset < 0x260))
	{
		m_hdsel = false;
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(0);
		}
	}

	if ((offset >= 0x260) && (offset < 0x280))
	{
		m_hdsel = true;
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(1);
		}
	}

	// reset MIG RAM window
	if (offset == 0x2a0)
	{
		m_migpage = 0;
	}

	return read_floatingbus();
}

void apple2e_state::mig_w(u16 offset, u8 data)
{
	//printf("mig_w %x @ %x\n", data, offset + 0xc00);

	if (offset == 0x40)
	{
		m_iwm->reset();
		return;
	}

	if ((offset >= 0x80) && (offset < 0xa0))
	{
		//printf("MIG: enable internal drive on d2\n");
		m_intdrive = true;
		recalc_active_device();
		return;
	}

	if ((offset >= 0xc0) && (offset < 0xe0))
	{
		//printf("MIG: disable internal drive\n");
		m_intdrive = false;
		recalc_active_device();
		return;
	}

	// MIG RAM window
	if ((offset >= 0x200) && (offset < 0x220))
	{
		m_migram[m_migpage + (offset & 0x1f)] = data;
		return;
	}

	// increment MIG RAM window, but write value at old location first
	if ((offset >= 0x220) && (offset < 0x240))
	{
		m_migram[m_migpage + (offset & 0x1f)] = data;
		m_migpage += 0x20;
		m_migpage &= 0x7ff; // make sure we wrap
		return;
	}

	if ((offset >= 0x240) && (offset < 0x260))
	{
		m_35sel = false;
		recalc_active_device();
		return;
	}

	if ((offset >= 0x260) && (offset < 0x280))
	{
		m_35sel = true;
		recalc_active_device();
		return;
	}

	// reset MIG RAM window
	if (offset == 0x2a0)
	{
		m_migpage = 0;
	}
}

void apple2e_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
	{
		m_cur_floppy->seek_phase_w(phases);
	}
}

void apple2e_state::devsel_w(uint8_t devsel)
{
	m_devsel = devsel;
	recalc_active_device();
}

void apple2e_state::sel35_w(int sel35)
{
}

void apple2e_state::recalc_active_device()
{
	if (m_devsel == 1)
	{
		if (!m_35sel)
		{
			m_cur_floppy = m_floppy[0]->get_device();
		}
		else
		{
			m_cur_floppy = m_floppy[3]->get_device();
		}
	}
	else if (m_devsel == 2)
	{
		// as per http://apple2.guidero.us/doku.php/mg_notes/apple_iic/mig_chip intdrive + devsel = 2 enables
		// the internal drive, no 35sel is necessary.  If intdrive is clear, 35sel and devsel work as normal.
		if (m_intdrive)
		{
			m_cur_floppy = m_floppy[2]->get_device();
		}
		else if (!m_35sel)
		{
			m_cur_floppy = m_floppy[1]->get_device();
		}
		else    // should be external 3.5 #2, for a 3rd drive
		{
			m_cur_floppy = nullptr;
		}
	}
	else
	{
		m_cur_floppy = nullptr;
	}

	m_iwm->set_floppy(m_cur_floppy);

	if (m_cur_floppy)
	{
		m_cur_floppy->ss_w(m_hdsel ? 1 : 0);
	}
}

void apple2e_state::a2bus_irq_w(int state)
{
	if (state == ASSERT_LINE)
	{
		raise_irq(IRQ_SLOT);
	}
	else
	{
		lower_irq(IRQ_SLOT);
	}
}

void apple2e_state::a2bus_nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

// TODO: this assumes /INH only on ROM, needs expansion to support e.g. phantom-slotting cards and etc.
void apple2e_state::a2bus_inh_w(int state)
{
	if (state == ASSERT_LINE)
	{
		// assume no cards are pulling /INH
		m_inh_slot = -1;

		// scan the slots to figure out which card(s) are INHibiting stuff
		for (int i = 0; i <= 7; i++)
		{
			if (m_slotdevice[i])
			{
				// this driver only can inhibit from 0xd000-0xffff
				if ((m_slotdevice[i]->inh_start() == 0xd000) &&
					(m_slotdevice[i]->inh_end() == 0xffff))
				{
					if ((m_slotdevice[i]->inh_type() & INH_READ) == INH_READ)
					{
						if (m_inh_bank != 1)
						{
							m_upperbank.select(1);
							m_inh_bank = 1;
						}
					}
					else
					{
						if (m_inh_bank != 0)
						{
							m_upperbank.select(0);
							m_inh_bank = 0;
						}
					}

					m_inh_slot = i;
					break;
				}
			}
		}

		// if no slots are inhibiting, make sure ROM is fully switched in
		if ((m_inh_slot == -1) && (m_inh_bank != 0))
		{
			m_upperbank.select(0);
			m_inh_bank = 0;
		}
	}
}

void apple2e_state::busy_w(int state)
{
	m_centronics_busy = state;
}

u8 apple2e_state::memexp_r(offs_t offset)
{
	u8 retval = m_exp_regs[offset];

	if (!m_exp_ram)
	{
		return read_floatingbus();
	}

	if (offset == 3)
	{
		if (m_exp_liveptr <= m_exp_addrmask)
		{
			retval = m_exp_ram[m_exp_liveptr];
		}
		else
		{
			retval = 0xff;
		}
		m_exp_liveptr++;
		m_exp_regs[0] = m_exp_liveptr & 0xff;
		m_exp_regs[1] = (m_exp_liveptr>>8) & 0xff;
		m_exp_regs[2] = ((m_exp_liveptr>>16) & 0xff) | m_exp_bankhior;
	}

	return retval;
}

void apple2e_state::memexp_w(offs_t offset, u8 data)
{
	if (!m_exp_ram)
	{
		return;
	}

	switch (offset & 0xf)
	{
		case 0:
			m_exp_wptr &= ~0xff;
			m_exp_wptr |= data;
			m_exp_regs[0] = m_exp_wptr & 0xff;
			m_exp_regs[1] = (m_exp_wptr>>8) & 0xff;
			m_exp_regs[2] = ((m_exp_wptr>>16) & 0xff) | m_exp_bankhior;
			m_exp_liveptr = m_exp_wptr;
			break;

		case 1:
			m_exp_wptr &= ~0xff00;
			m_exp_wptr |= (data<<8);
			m_exp_regs[0] = m_exp_wptr & 0xff;
			m_exp_regs[1] = (m_exp_wptr>>8) & 0xff;
			m_exp_regs[2] = ((m_exp_wptr>>16) & 0xff) | m_exp_bankhior;
			m_exp_liveptr = m_exp_wptr;
			break;

		case 2:
			m_exp_wptr &= ~0xff0000;
			m_exp_wptr |= (data<<16);
			m_exp_regs[0] = m_exp_wptr & 0xff;
			m_exp_regs[1] = (m_exp_wptr>>8) & 0xff;
			m_exp_regs[2] = ((m_exp_wptr>>16) & 0xff) | m_exp_bankhior;
			m_exp_liveptr = m_exp_wptr;
			break;

		case 3:
//            printf("Write %02x to RAM[%x]\n", data, m_liveptr);
			if (m_exp_liveptr <= m_exp_addrmask)
			{
				m_exp_ram[m_exp_liveptr] = data;
			}
			m_exp_liveptr++;
			m_exp_regs[0] = m_exp_liveptr & 0xff;
			m_exp_regs[1] = (m_exp_liveptr>>8) & 0xff;
			m_exp_regs[2] = ((m_exp_liveptr>>16) & 0xff) | m_exp_bankhior;
			break;

		default:
			m_exp_regs[offset] = data;
			break;
	}
}

/***************************************************************************
    START/RESET
***************************************************************************/

void apple2e_state::machine_start()
{
	m_ram_ptr = m_ram->pointer();
	m_rom_ptr = m_rom->base();
	m_ram_size = m_ram->size();
	m_speaker_state = 0;
	m_speaker->level_w(m_speaker_state);
	m_cassette_state = 0;
	m_cassette_out = 0;
	if (m_cassette)
	{
		m_cassette->output(-1.0f);
	}
	m_upperbank.select(0);
	m_lcbank.select(0);
	m_0000bank.select(0);
	m_0200bank.select(0);
	m_0400bank.select(0);
	m_0800bank.select(0);
	m_2000bank.select(0);
	m_4000bank.select(0);
	m_inh_bank = 0;

	m_migpage = 0;
	memset(m_migram, 0, 0x200);

	// expansion RAM size
	if (m_ram_size > (128*1024))
	{
		m_exp_addrmask = m_ram_size - (128*1024) - 1;
		m_exp_ram = m_ram_ptr + (128*1024);
	}
	else    // no expansion
	{
		m_exp_addrmask = 0;
		m_exp_ram = nullptr;
	}

	// precalculate joystick time constants
	m_x_calibration = attotime::from_nsec(10800).as_double();
	m_y_calibration = attotime::from_nsec(10800).as_double();

	// cache slot devices
	for (int i = 0; i <= 7; i++)
	{
		m_slotdevice[i] = m_a2bus->get_a2bus_card(i);
	}

	// and aux slot device if any
	m_aux_ptr = nullptr;
	m_aux_bank_ptr = nullptr;
	m_aux_mask = 0xffff;
	if (m_a2eauxslot)
	{
		m_auxslotdevice = m_a2eauxslot->get_a2eauxslot_card();
		if (m_auxslotdevice)
		{
			m_aux_ptr = m_auxslotdevice->get_vram_ptr();
			m_aux_bank_ptr = m_auxslotdevice->get_auxbank_ptr();
			m_aux_mask =  m_auxslotdevice->get_auxbank_mask();
		}
	}
	else    // IIc has 128K right on the motherboard
	{
		m_auxslotdevice = nullptr;

		if (m_ram_size >= (128*1024))
		{
			m_aux_ptr = &m_ram_ptr[0x10000];
			m_aux_bank_ptr = m_aux_ptr;
		}
	}

	// setup video pointers
	m_video->set_ram_pointers(m_ram_ptr, m_aux_ptr);
	m_video->set_aux_mask(m_aux_mask);
	m_video->set_char_pointer(memregion("gfx1")->base(), memregion("gfx1")->bytes());

	int ram_size = 0x10000;
	if (m_ram_size < 0x10000)
	{
		ram_size = m_ram_size;
	}

	for (int adr = 0; adr < ram_size; adr += 2)
	{
		// invert the fill pattern order on the ACE 500 and 2200, as it interacts with
		// Franklin's monitor not returning the same values as Apple's plus some
		// bugs in DOS 3.3.
		if ((m_isace500) || (m_isace2200))
		{
			m_ram_ptr[adr] = 0xff;
			m_ram_ptr[adr+1] = 0;
		}
		else
		{
			m_ram_ptr[adr] = 0;
			m_ram_ptr[adr + 1] = 0xff;
		}

		if (m_ram_size >= (128*1024))
		{
			m_aux_ptr[adr] = 0;
			m_aux_ptr[adr+1] = 0xff;
		}
	}

	m_inh_slot = -1;
	m_cnxx_slot = CNXX_UNCLAIMED;
	m_mockingboard4c = false;

	// remap CEC banking
	if ((m_rom_ptr[0x7bb3] == 0x8d) || (m_rom_ptr[0x7bb3] == 0xea) || (m_rom_ptr[0x7bb3] == 0x06))
	{
		m_lcbank.select(3);
		m_cec_ptr = m_cecbanks->base();
		m_iscec = true;
		m_iscecm = false;
		m_iscec2000 = false;

		// CEC-M
		// write addr C0B0 change to addr C600
		if ((m_rom_ptr[0x8000+0x4600] == 0xff) && (m_rom_ptr[0x8000+0x4601] == 0xff))
		{
			m_iscecm = true;
		}

		// CEC-2000
		if (m_rom_ptr[0x7bb3] == 0x06)
		{
			m_iscec2000 = true;
		}

		// data is bit-order reversed (and byte interleaved, which the ROM loader takes care of)
		// let's do that in the modern MAME way
		for (int i=0; i<0x040000; i++)
		{
			m_cec_remap[i] = bitswap<8>(m_cec_ptr[i], 0, 1, 2, 3, 4, 5, 6, 7);
		}

		// remap cec gfx1 rom
		// for ALTCHARSET
		u8 *rom = memregion("gfx1")->base();
		for(int i=0; i<0x1000; i++)
		{
			rom[i+0x1000] = rom[i];
		}
		for(int i=0x040*8; i<0x80*8; i++)
		{
			rom[i] = rom[i+0x1000-0x040*8];
		}
	}
	else
	{
		m_iscec = false;
		m_iscecm = false;
		m_iscec2000 = false;
	}

	if ((m_has_laser_mouse) || (m_isace500) || (m_isace2200))
	{
		m_strobe_timer = timer_alloc(FUNC(apple2e_state::update_laserprn_strobe), this);
		m_next_strobe = 1U;
	}

	m_joystick_x1_time = m_joystick_x2_time = m_joystick_y1_time = m_joystick_y2_time = 0;
	m_reset_latch = false;

	// setup save states
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_cassette_state));
	save_item(NAME(m_joystick_x1_time));
	save_item(NAME(m_joystick_y1_time));
	save_item(NAME(m_joystick_x2_time));
	save_item(NAME(m_joystick_y2_time));
	save_item(NAME(m_lastchar));
	save_item(NAME(m_strobe));
	save_item(NAME(m_franklin_last_fkeys));
	save_item(NAME(m_franklin_strobe));
	save_item(NAME(m_transchar));
	save_item(NAME(m_inh_slot));
	save_item(NAME(m_inh_bank));
	save_item(NAME(m_cnxx_slot));
	save_item(NAME(m_an0));
	save_item(NAME(m_an1));
	save_item(NAME(m_an2));
	save_item(NAME(m_an3));
	save_item(NAME(m_intcxrom));
	save_item(NAME(m_slotc3rom));
	save_item(NAME(m_altzp));
	save_item(NAME(m_ramrd));
	save_item(NAME(m_ramwrt));
	save_item(NAME(m_ioudis));
	save_item(NAME(m_vbl));
	save_item(NAME(m_vblmask));
	save_item(NAME(m_romswitch));
	save_item(NAME(m_irqmask));
	save_item(NAME(m_anykeydown));
	save_item(NAME(m_repeatdelay));
	save_item(NAME(m_xy));
	save_item(NAME(m_x0edge));
	save_item(NAME(m_y0edge));
	save_item(NAME(last_mx));
	save_item(NAME(last_my));
	save_item(NAME(count_x));
	save_item(NAME(count_y));
	save_item(NAME(m_x0));
	save_item(NAME(m_x1));
	save_item(NAME(m_y0));
	save_item(NAME(m_y1));
	save_item(NAME(m_xirq));
	save_item(NAME(m_yirq));
	save_item(NAME(m_migram));
	save_item(NAME(m_migpage));
	save_item(NAME(m_exp_regs));
	save_item(NAME(m_exp_wptr));
	save_item(NAME(m_exp_liveptr));
	save_item(NAME(m_exp_bankhior));
	save_item(NAME(m_exp_addrmask));
	save_item(NAME(m_lcram));
	save_item(NAME(m_lcram2));
	save_item(NAME(m_lcprewrite));
	save_item(NAME(m_lcwriteenable));
	save_item(NAME(m_mockingboard4c));
	save_item(NAME(m_intc8rom));
	save_item(NAME(m_cec_bank));
	save_item(NAME(m_35sel));
	save_item(NAME(m_hdsel));
	save_item(NAME(m_intdrive));
	save_item(NAME(m_accel_unlocked));
	save_item(NAME(m_accel_stage));
	save_item(NAME(m_accel_fast));
	save_item(NAME(m_accel_present));
	save_item(NAME(m_accel_slotspk));
	save_item(NAME(m_accel_gameio));
	save_item(NAME(m_accel_temp_slowdown));
	save_item(NAME(m_accel_laser));
	save_item(NAME(m_accel_speed));
	save_item(NAME(m_next_strobe));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_ace500rombank));
	save_item(NAME(m_ace_cnxx_bank));
	save_item(NAME(m_ace2200_axxx_bank));
	save_item(NAME(m_laser_speed));
	save_item(NAME(m_laser_fdc_on));
	save_item(NAME(m_reset_latch));
}

void apple2e_state::machine_reset()
{
	// All MMU switches off (80STORE, RAMRD, RAMWRT, INTCXROM, ALTZP, SLOTC3ROM, PAGE2, HIRES, INTC8ROM)
	// Sather, Fig 5.13
	m_ramrd = false;
	m_ramwrt = false;
	m_altzp = false;
	m_slotc3rom = false;
	m_intc8rom = false;

	// Certain IOU switches off (80STORE, 80COL, ALTCHR, PAGE2, HIRES, AN0, AN1, AN2, AN3)
	// Sather, Fig 7.1
	m_video->a80store_w(false);
	m_video->a80col_w(false);
	m_video->altcharset_w(false);
	m_video->page2_w(false);
	m_video->res_w(0);

	// IIe IOU
	m_an0 = m_an1 = m_an2 = m_an3 = false;
	m_gameio->an0_w(0);
	m_gameio->an1_w(0);
	m_gameio->an2_w(0);
	m_gameio->an3_w(0);

	// IIc IOU
	m_ioudis = true;
	m_romswitch = false;

	// LC resets to read ROM, write RAM, no pre-write, bank 2
	// Sather, Fig 5.13
	m_lcram = false;
	m_lcram2 = true;
	m_lcprewrite = false;
	m_lcwriteenable = true;

	m_video->monohgr_w(m_iscecm);
	m_vbl = m_vblmask = false;
	m_irqmask = 0;
	m_strobe = 0;
	m_franklin_last_fkeys = 0;
	m_franklin_strobe = 0x80;
	m_transchar = 0;
	m_anykeydown = false;
	m_repeatdelay = 10;
	m_xy = false;
	m_x0edge = false;
	m_y0edge = false;
	m_xirq = false;
	m_yirq = false;
	m_mockingboard4c = false;
	m_cec_bank = 0;
	m_accel_unlocked = false;
	m_accel_stage = 0;
	m_accel_slotspk = 0x41; // speaker and slot 6 slow
	m_accel_gameio = 0x40;  // paddle delay on
	m_accel_present = false;
	m_accel_temp_slowdown = false;
	m_accel_fast = false;
	m_centronics_busy = false;
	m_35sel = false;

	// is Zip enabled?
	if (m_sysconfig.read_safe(0) & 0x10)
	{
		m_accel_present = true;
	}

	// IIe prefers INTCXROM default to off, IIc has it always on
	if (m_rom_ptr[0x3bc0] == 0x00)
	{
		m_intcxrom = true;
		m_slotc3rom = false;
		if (!m_isace500)
		{
			m_isiic = true;
		}

		if (m_rom_ptr[0x3bbf] == 0x05)
		{
			m_isiicplus = true;
			m_accel_present = true;
		}
		else
		{
			m_isiicplus = false;
		}
	}
	else
	{
		m_intcxrom = false;
		m_isiic = false;
		m_isiicplus = false;
	}

	u8 config = m_sysconfig.read_safe(0) & 0x30;

	if (((config & 0x10) == 0x10) || (m_isiicplus))
	{
		m_accel_speed = 4000000;    // Zip speed, set if present, even if not active initially

		if (((config & 0x20) == 0x20) || (m_isiicplus))
		{
			accel_full_speed();
			m_accel_fast = true;
		}
	}

	if (m_accel_laser)
	{
		m_accel_present = true;
		m_accel_speed = 1021800;
	}

	if (m_has_laser_mouse)
	{
	   a2bus_laser128_device *printer_slot = static_cast<a2bus_laser128_device *>(m_slotdevice[1]);

	   if (m_sysconfig->read() & 0x08)
	   {
		  printer_slot->set_parallel_printer(true);
	   }
	   else
	   {
		  printer_slot->set_parallel_printer(false);
	   }

	}

	m_exp_bankhior = 0xf0;

	// sync up the banking with the variables.
	lcrom_update();
	auxbank_update();
	update_slotrom_banks();
}

// called before machine_start() so we have to be careful
void apple2e_state::init_128ex()
{
	m_accel_laser = true;
	m_has_laser_mouse = true;
}

void apple2e_state::init_laser128()
{
	m_has_laser_mouse = true;
}

void apple2e_state::init_ace500()
{
	m_isace500 = true;
	m_ace_cnxx_bank = false;
}

void apple2e_state::init_ace2200()
{
	m_isace2200 = true;
}

void apple2e_state::init_pal()
{
	m_pal = true;
}

void apple2e_state::raise_irq(int irq)
{
	m_irqmask |= (1 << irq);

	if (m_irqmask)
	{
		m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	}
}

void apple2e_state::lower_irq(int irq)
{
	m_irqmask &= ~(1 << irq);

	if (!m_irqmask)
	{
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	}
}

/***************************************************************************
    VIDEO
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(apple2e_state::apple2_interrupt)
{
	int scanline = param;

	if ((m_isiic) || (m_has_laser_mouse) || (m_isace500))
	{
		update_iic_mouse();
	}

	if (scanline == 192)
	{
		m_vbl = true;

		if (m_vblmask)
		{
			raise_irq(IRQ_VBL);
		}

		// check for ctrl-reset
		if ((m_kbspecial->read() & 0x88) == 0x88)
		{
			if (!m_reset_latch)
			{
				m_reset_latch = true;
				m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

				// All MMU switches off (80STORE, RAMRD, RAMWRT, INTCXROM, ALTZP, SLOTC3ROM, PAGE2, HIRES, INTC8ROM)
				// Sather, Fig 5.13
				m_ramrd = false;
				m_ramwrt = false;
				m_altzp = false;
				m_slotc3rom = false;
				m_intc8rom = false;

				// reset intcxrom to default
				if ((m_isiic) || (m_isace500))
				{
					m_intcxrom = true;
				}
				else
				{
					m_intcxrom = false;
					m_slotc3rom = false;
				}

				// Certain IOU switches off (80STORE, 80COL, ALTCHR, PAGE2, HIRES, AN0, AN1, AN2, AN3)
				// Sather, Fig 7.1
				m_video->a80store_w(false);
				m_video->a80col_w(false);
				m_video->altcharset_w(false);
				m_video->page2_w(false);
				m_video->res_w(0);

				// IIe IOU
				m_an0 = m_an1 = m_an2 = m_an3 = false;
				m_gameio->an0_w(0);
				m_gameio->an1_w(0);
				m_gameio->an2_w(0);
				m_gameio->an3_w(0);

				// LC resets to read ROM, write RAM, no pre-write, bank 2
				// Sather, Fig 5.13
				m_lcram = false;
				m_lcram2 = true;
				m_lcprewrite = false;
				m_lcwriteenable = true;

				lcrom_update();
				auxbank_update();
				update_slotrom_banks();
			}
		}
		else    // user released Control-Reset
		{
			if (m_reset_latch)
			{
				m_reset_latch = false;
				// allow cards to see reset
				m_a2bus->reset_bus();
				m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
		}

		// check Franklin F-keys
		if ((m_isace500) || (m_isace2200))
		{
			const u32 uFkeys = m_franklin_fkeys->read();

			if ((uFkeys ^ m_franklin_last_fkeys) && uFkeys)
			{
				m_transchar = count_leading_zeros_32(uFkeys) + 0x20;
				m_strobe = 0x80;
				m_franklin_strobe = 0;
			}
			m_franklin_last_fkeys = uFkeys;
		}
	}
}

/***************************************************************************
    I/O
***************************************************************************/
void apple2e_state::accel_full_speed()
{
	m_maincpu->set_unscaled_clock(m_accel_speed);
}

void apple2e_state::accel_normal_speed()
{
	m_maincpu->set_unscaled_clock(1021800);
}

void apple2e_state::accel_slot(int slot)
{
	if ((m_accel_present) && (m_accel_slotspk & (1<<slot)))
	{
		m_accel_temp_slowdown = true;
		m_acceltimer->adjust(attotime::from_msec(52));
		accel_normal_speed();
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(apple2e_state::accel_timer)
{
	if (m_accel_fast)
	{
		accel_full_speed();
	}
	m_accel_temp_slowdown = false;
	m_acceltimer->adjust(attotime::never);
}

void apple2e_state::auxbank_update()
{
	int ramwr = (m_ramrd ? 1 : 0) | (m_ramwrt ? 2 : 0);

	if (!m_iscec)   // real Apple II
	{
		m_0000bank.select(m_altzp ? 1 : 0);
		m_0200bank.select(ramwr);

		if (m_video->get_80store())
		{
			if (m_video->get_page2())
			{
				m_0400bank.select(3);
			}
			else
			{
				m_0400bank.select(0);
			}
		}
		else
		{
			m_0400bank.select(ramwr);
		}

		m_0800bank.select(ramwr);

		if ((m_video->get_80store()) && (m_video->get_hires()))
		{
			if (m_video->get_page2())
			{
				m_2000bank.select(3);
			}
			else
			{
				m_2000bank.select(0);
			}
		}
		else
		{
			m_2000bank.select(ramwr);
		}

		m_4000bank.select(ramwr);
	}
	else    // CEC
	{
		if (m_ramrd)
		{
			m_4000bank.select(4);    // read CEC bank, write normal RAM
		}
		else
		{
			m_4000bank.select(0);    // read/write RAM
		}
	}
}

void apple2e_state::update_slotrom_banks()
{
	if (!m_iscec)
	{
		int cxswitch = 0;

		// IIc and IIc+ have working (readable) INTCXROM/SLOTC3ROM switches, but
		// internal ROM is always present in the slots.
		if ((m_intcxrom) || (m_isiic) || (m_isace500))
		{
			if (m_romswitch)
			{
				cxswitch = 2;
			}
			else
			{
				cxswitch = 1;
			}
		}

		m_c100bank.select(cxswitch);
		m_c400bank.select(cxswitch);

		//printf("intcxrom %d intc8rom %d cnxx_slot %d isiic %d romswitch %d\n", m_intcxrom, m_intc8rom, m_cnxx_slot, m_isiic, m_romswitch);
		if ((m_intcxrom) || (m_intc8rom) || (m_isiic))
		{
			if (m_romswitch)
			{
				m_c800bank.select(2);
			}
			else
			{
				m_c800bank.select(1);
			}
		}
		else
		{
			m_c800bank.select(0);
		}

		if ((m_intcxrom) || (!m_slotc3rom) || (m_isiic))
		{
			if (m_romswitch)
			{
				m_c300bank.select(2);
			}
			else
			{
				m_c300bank.select(1);
			}
		}
		else
		{
			m_c300bank.select(0);
		}
	}
	else    // CEC has only ROM here
	{
		if (!m_intcxrom)
		{
			m_c100bank.select(3);
			m_c400bank.select(3);
			m_c800bank.select(3);
		}
		else
		{
			m_c100bank.select(4);
			m_c400bank.select(4);
			m_c800bank.select(4);
		}

		if ((m_intcxrom) || (!m_slotc3rom))
		{
			m_c300bank.select(4);
		}
		else
		{
			m_c300bank.select(3);
		}
	}
}

void apple2e_state::lc_update(int offset, bool writing)
{
	bool old_lcram = m_lcram;

	//any even access disables pre-write and writing
	if ((offset & 1) == 0)
	{
		m_lcprewrite = false;
		m_lcwriteenable = false;
	}

	//any write disables pre-write
	//has no effect on write-enable if writing was enabled already
	if (writing == true)
	{
		m_lcprewrite = false;
	}
	//first odd read enables pre-write, second one enables writing
	else if ((offset & 1) == 1)
	{
		if (m_lcprewrite == false)
		{
			m_lcprewrite = true;
		}
		else
		{
			m_lcwriteenable = true;
		}
	}

	switch (offset & 3)
	{
		case 0:
		case 3:
		{
			m_lcram = true;
			break;
		}

		case 1:
		case 2:
		{
			m_lcram = false;
			break;
		}
	}

	m_lcram2 = false;

	if (!(offset & 8))
	{
		m_lcram2 = true;
	}

	if (m_lcram != old_lcram)
	{
		lcrom_update();
	}

	#if 0
	printf("LC: new state %c%c dxxx=%04x altzp=%d (PC=%x)\n",
			m_lcram ? 'R' : 'x',
			m_lcwriteenable ? 'W' : 'x',
			m_lcram2 ? 0x1000 : 0x0000,
			m_altzp, m_maincpu->pc());
	#endif
}

void apple2e_state::lcrom_update()
{
	if (m_iscec)
	{
		cec_lcrom_update();
	}
	else
	{
		if (m_lcram)
		{
			m_lcbank.select(1);
		}
		else
		{
			if (m_romswitch)
			{
				m_lcbank.select(2);
			}
			else
			{
				m_lcbank.select(0);
			}
		}
	}
}

void apple2e_state::cec_lcrom_update()
{
	if (m_altzp)
	{
		m_lcbank.select(5);
	}
	else
	{
		if (m_lcram)
		{
			m_lcbank.select(1);
		}
		else
		{
			m_lcbank.select(3);
		}
	}
}

// most softswitches don't care about read vs write, so handle them here
void apple2e_state::do_io(int offset, bool is_iic)
{
	if(machine().side_effects_disabled()) return;

	// Handle C058-C05F according to IOUDIS
	if ((offset & 0x58) == 0x58)
	{
		// IIc-specific switches
		if (((m_isiic || m_isace500) && (!m_accel_unlocked)) && (!m_ioudis))
		{
			switch (offset)
			{
				case 0x58:  // DisXY
					m_xy = false; break;

				case 0x59:  // EnbXY
					m_xy = true; break;

				case 0x5a:  // DisVBL
					lower_irq(IRQ_VBL);
					m_vblmask = false; break;

				case 0x5b:  // EnVBL
					m_vblmask = true; break;

				case 0x5c:  // RisX0Edge
					m_x0edge = false; break;

				case 0x5d:  // FalX0Edge
					m_x0edge = true; break;

				case 0x5e:  // RisY0Edge
					if (!m_ioudis)
					{
						m_y0edge = false;
					}
					break;

				case 0x5f:  // FalY0Edge
					if (!m_ioudis)
					{
						m_y0edge = true;
					}
					break;
			}
		}

		// IIe does not have IOUDIS (ref: on-h/w tests by TomCh)
		if ((m_ioudis) || (!m_isiic && !m_isace500))
		{
			switch (offset)
			{
				case 0x5e:  // SETDHIRES
					m_video->dhires_w(0);
					break;

				case 0x5f:  // CLRDHIRES
					m_video->dhires_w(1);
					break;
			}
		}
// ComputerEyes seems to indicate that the annuciators get tickled regardless of the IOUDIS state.
	}

	if ((offset & 0xf0) == 0x30) // speaker, $C030 is really 30-3f
	{
		m_speaker_state ^= 1;
		m_speaker->level_w(m_speaker_state);
		if ((m_accel_present) && (m_accel_slotspk & 1))
		{
			m_accel_temp_slowdown = true;
			m_acceltimer->adjust(attotime::from_msec(5));
			accel_normal_speed();
		}
		return;
	}

	switch (offset)
	{
		case 0x20:
			if (m_cassette)
			{
				m_cassette_state ^= 1;
				m_cassette->output(m_cassette_state ? 1.0f : -1.0f);
			}
			break;

		case 0x28:
			if (is_iic)
			{
				m_romswitch = !m_romswitch;
				update_slotrom_banks();
				lcrom_update();

				// MIG is reset when ROMSWITCH turns off
				if ((m_isiicplus) && !(m_romswitch))
				{
					m_migpage = 0;
					m_intdrive = false;
					m_35sel = false;
				}
			}
			break;

		case 0x40:  // utility strobe (not available on IIc)
			if (!is_iic)
			{
				m_gameio->strobe_w(0);
				m_gameio->strobe_w(1);
			}
			break;

		case 0x48:  // (IIc only) clear mouse X/Y interrupt flags
			m_xirq = m_yirq = false;
			lower_irq(IRQ_MOUSEXY);
			break;

		case 0x50:  // graphics mode
			m_video->txt_w(0);
			break;

		case 0x51:  // text mode
			m_video->txt_w(1);
			break;

		case 0x52:  // no mix
			m_video->mix_w(0);
			break;

		case 0x53:  // mixed mode
			m_video->mix_w(1);
			break;

		case 0x54:  // set page 1
			m_video->page2_w(false);
			m_video->scr_w(0);
			auxbank_update();
			break;

		case 0x55:  // set page 2
			m_video->page2_w(true);
			m_video->scr_w(1);
			auxbank_update();
			break;

		case 0x56: // select lo-res
			m_video->res_w(0);
			auxbank_update();
			break;

		case 0x57: // select hi-res
			m_video->res_w(1);
			auxbank_update();
			break;

		case 0x58: // AN0 off
			m_an0 = false;
			m_gameio->an0_w(0);
			break;

		case 0x59: // AN0 on
			m_an0 = true;
			m_gameio->an0_w(1);
			break;

		case 0x5a: // AN1 off
			m_an1 = false;
			m_gameio->an1_w(0);
			break;

		case 0x5b: // AN1 on
			m_an1 = true;
			m_gameio->an1_w(1);
			break;

		case 0x5c: // AN2 off
			m_an2 = false;
			m_gameio->an2_w(0);
			break;

		case 0x5d: // AN2 on
			m_an2 = true;
			m_gameio->an2_w(1);
			break;

		case 0x5e: // AN3 off
			m_an3 = false;
			m_gameio->an3_w(0);
			break;

		case 0x5f: // AN3 on
			m_an3 = true;
			m_gameio->an3_w(1);
			break;

		case 0x68:  // IIgs STATE register, which ProDOS touches
			break;

		// trigger joypad read
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			if ((is_iic) || (m_isace500))
			{
				m_vbl = false;
				lower_irq(IRQ_VBL);
			}

			// Zip paddle flag
			if ((m_accel_present) && (BIT(m_accel_gameio, 6)))
			{
				m_accel_temp_slowdown = true;
				m_acceltimer->adjust(attotime::from_msec(5));
				accel_normal_speed();
			}

			// 558 monostable one-shot timers; a running timer cannot be restarted
			if (machine().time().as_double() >= m_joystick_x1_time)
			{
				m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl0_r();
			}
			if (machine().time().as_double() >= m_joystick_y1_time)
			{
				m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl1_r();
			}
			if (machine().time().as_double() >= m_joystick_x2_time)
			{
				m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl2_r();
			}
			if (machine().time().as_double() >= m_joystick_y2_time)
			{
				m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl3_r();
			}
			break;

		default:
			logerror("do_io: unknown switch $C0%02X\n", offset);
			break;
	}
}

u8 apple2e_state::c000_r(offs_t offset)
{
	if(machine().side_effects_disabled()) return read_floatingbus();
	const u8 uFloatingBus7 = read_floatingbus() & 0x7f;

	if ((offset & 0xf0) == 0x00) // keyboard latch, $C000 is really 00-0F
	{
		return m_transchar | m_strobe;
	}

	switch (offset)
	{
		case 0x10:  // read any key down, reset keyboard strobe
			{
				u8 rv = m_transchar | (m_anykeydown ? 0x80 : 0x00);
				m_strobe = 0;
				return rv;
			}

		case 0x11:  // read LCRAM2 (LC Dxxx bank)
			return (m_lcram2 ? 0x80 : 0x00) | m_transchar;

		case 0x12:  // read LCRAM (is LC readable?)
			return (m_lcram ? 0x80 : 0x00) | m_transchar;

		case 0x13:  // read RAMRD
			return (m_ramrd ? 0x80 : 0x00) | m_transchar;

		case 0x14:  // read RAMWRT
			return (m_ramwrt ? 0x80 : 0x00) | m_transchar;

		case 0x15:  // read INTCXROM
			return (m_intcxrom ? 0x80 : 0x00) | m_transchar;

		case 0x16:  // read ALTZP
			return (m_altzp ? 0x80 : 0x00) | m_transchar;

		case 0x17:  // read SLOTC3ROM
			return (m_slotc3rom ? 0x80 : 0x00) | m_transchar;

		case 0x18:  // read 80STORE
			return (m_video->get_80store() ? 0x80 : 0x00) | m_transchar;

		case 0x19:  // read VBLBAR
			return (m_screen->vblank() ? 0x00 : 0x80) | m_transchar;

		case 0x1a:  // read TEXT
			return (m_video->get_graphics() ? 0x00 : 0x80) | m_transchar;

		case 0x1b:  // read MIXED
			return (m_video->get_mix() ? 0x80 : 0x00) | m_transchar;

		case 0x1c:  // read PAGE2
			return (m_video->get_page2() ? 0x80 : 0x00) | m_transchar;

		case 0x1d:  // read HIRES
			return (m_video->get_hires() ? 0x80 : 0x00) | m_transchar;

		case 0x1e:  // read ALTCHARSET
			return (m_video->get_altcharset() ? 0x80 : 0x00) | m_transchar;

		case 0x1f:  // read 80COL
			return (m_video->get_80col() ? 0x80 : 0x00) | m_transchar;

		case 0x26:  // Ace 2x00 DIP switches
			if (m_isace2200)
			{
				return (m_sysconfig->read() & 0x80) | uFloatingBus7;
			}
			break;

		case 0x27: // Ace 2x00 F key strobe
			if (m_isace2200)
			{
				m_strobe = 0;
				const u8 rv = m_franklin_strobe;
				m_franklin_strobe = 0x80;
				return rv;
			}
			break;

		case 0x60: // cassette in
		case 0x68:
			if (m_cassette)
			{
				return (m_cassette->input() > 0.0 ? 0x80 : 0) | uFloatingBus7;
			}
			return uFloatingBus7;

		case 0x61:  // button 0 or Open Apple
		case 0x69:
			return ((m_gameio->sw0_r() || (m_kbspecial->read() & 0x10)) ? 0x80 : 0) | uFloatingBus7;

		case 0x62:  // button 1 or Solid Apple
		case 0x6a:
			return ((m_gameio->sw1_r() || (m_kbspecial->read() & 0x20)) ? 0x80 : 0) | uFloatingBus7;

		case 0x63:  // button 2 or SHIFT key
		case 0x6b:
			return ((m_gameio->sw2_r() || (m_kbspecial->read() & 0x06)) ? 0x80 : 0) | uFloatingBus7;

		case 0x64:  // joy 1 X axis
		case 0x6c:
			if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
			return ((machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x65:  // joy 1 Y axis
		case 0x6d:
			if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
			return ((machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x66: // joy 2 X axis
		case 0x6e:
			if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
			return ((machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x67: // joy 2 Y axis
		case 0x6f:
			if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
			return ((machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x7e:  // read IOUDIS
			return (m_ioudis ? 0x00 : 0x80) | uFloatingBus7;

		case 0x7f:  // read DHIRES
			return (m_video->get_dhires() ? 0x00 : 0x80) | uFloatingBus7;

		default:
			do_io(offset, false);

			if (m_accel_unlocked)
			{
				if (offset == 0x5b)
				{
					// bit 7 is a 1.0035 millisecond clock; the value changes every 0.50175 milliseconds
					const int time = machine().time().as_ticks(1.0f / 0.00050175f);
					if (time & 1)
					{
						return 0x03;
					}
					else
					{
						return 0x83;
					}
				}
				else if (offset == 0x5c)
				{
					return m_accel_slotspk;
				}
			}
			break;
	}

	return read_floatingbus();
}

u8 apple2e_state::c000_laser_r(offs_t offset)
{
	u8 uFloatingBus7 = read_floatingbus() & 0x7f;

	switch (offset)
	{
		case 0x63:  // read mouse button
			return (m_mouseb->read() ? 0 : 0x80) | uFloatingBus7;

		case 0x66:  // read mouse xdir
			return (m_x1 ? 0x80 : 0) | uFloatingBus7;

		case 0x67:  // read mouse ydir
			return (m_y1 ? 0x80 : 0) | uFloatingBus7;
	}

	return c000_r(offset);
}

void apple2e_state::laser_calc_speed()
{
	if (m_laser_fdc_on)
	{
		accel_normal_speed();
		m_accel_fast = false;
		return;
	}

	switch ((m_laser_speed & 0xc0) >> 6)
	{
		case 0:
		case 1:
			accel_normal_speed();
			m_accel_fast = false;
			break;

		case 2:
			m_accel_speed = A2BUS_7M_CLOCK/3;   // 2.38 MHz
			m_accel_fast = true;
			accel_full_speed();
			break;

		case 3:
			m_accel_speed = A2BUS_7M_CLOCK/2;   // 3.58 MHz
			m_accel_fast = true;
			accel_full_speed();
			break;
	}
}

void apple2e_state::c000_laser_w(offs_t offset, u8 data)
{
	if ((m_accel_laser) && (offset == 0x74))
	{
		m_laser_speed = data;
		laser_calc_speed();
	}
	else
	{
		if ((offset & 0xf0) == 0x70)
		{
			lower_irq(IRQ_VBL);
		}

		c000_w(offset, data);
	}
}

u8 apple2e_state::laserprn_busy_r()
{
	u8 retval = read_floatingbus() & 0x7f;

	if (m_centronics_busy)
	{
		retval |= 0x80;
	}

	return retval;
}

void apple2e_state::laserprn_w(u8 data)
{
	m_printer_out->write(data);

   // generate strobe pulse after one clock cycle
	m_next_strobe = 0U;
	if (!m_strobe_timer->enabled())
	{
	   m_strobe_timer->adjust(attotime::from_hz(1021800));
	}
}

TIMER_CALLBACK_MEMBER(apple2e_state::update_laserprn_strobe)
{
	m_printer_conn->write_strobe(m_next_strobe);
	if (!m_next_strobe)
	{
		m_next_strobe = 1U;
		m_strobe_timer->adjust(attotime::from_hz(1021800));
	}
}

void apple2e_state::c000_w(offs_t offset, u8 data)
{
	if(machine().side_effects_disabled()) return;

	if ((offset & 0xf0) == 0x10) // clear keyboard latch, $C010 is really 10-1F
	{
		m_strobe = 0;
		return;
	}

	switch (offset)
	{
		case 0x00:  // 80STOREOFF
			m_video->a80store_w(false);
			auxbank_update();
			break;

		case 0x01:  // 80STOREON
			m_video->a80store_w(true);
			auxbank_update();
			break;

		case 0x02:  // RAMRDOFF
			m_ramrd = false;
			auxbank_update();
			break;

		case 0x03:  // RAMRDON
			m_ramrd = true;
			auxbank_update();
			break;

		case 0x04:  // RAMWRTOFF
			m_ramwrt = false;
			auxbank_update();
			break;

		case 0x05:  // RAMWRTON
			m_ramwrt = true;
			auxbank_update();
			break;

		case 0x06:  // INTCXROMOFF
			m_intcxrom = false;
			update_slotrom_banks();
			break;

		case 0x07:  // INTCXROMON
			m_intcxrom = true;
			update_slotrom_banks();
			break;

		case 0x08:  // ALTZPOFF
			m_altzp = false;
			auxbank_update();
			if (m_iscec)
			{
				cec_lcrom_update();
			}
			break;

		case 0x09:  // ALTZPON
			m_altzp = true;
			auxbank_update();
			if (m_iscec)
			{
				cec_lcrom_update();
			}
			break;

		case 0x0a:  // SETINTC3ROM
			m_slotc3rom = false;
			update_slotrom_banks();
			break;

		case 0x0b:  // SETSLOTC3ROM
			m_slotc3rom = true;
			update_slotrom_banks();
			break;

		case 0x0c:  // 80COLOFF
			m_video->a80col_w(false);
			break;

		case 0x0d:  // 80COLON
			m_video->a80col_w(true);
			break;

		case 0x0e:  // ALTCHARSETOFF
			m_video->altcharset_w(false);
			break;

		case 0x0f:  // ALTCHARSETON
			m_video->altcharset_w(true);
			break;

		case 0x20:  // cassette output
			if (m_cassette)
			{
				m_cassette_out ^= 1;
				m_cassette->output(m_cassette_out ? 1.0f : -1.0f);
			}
			break;

		case 0x5a:  // Zip accelerator unlock
			if (m_sysconfig.read_safe(0) & 0x10)
			{
				if (data == 0x5a)
				{
					m_accel_stage++;
					if (m_accel_stage == 4)
					{
						m_accel_unlocked = true;
					}
				}
				else if (data == 0xa5)
				{
					// lock
					m_accel_unlocked = false;
					m_accel_stage = 0;
				}
				else if (m_accel_unlocked)
				{
					// disable acceleration
					m_accel_fast = false;
					accel_normal_speed();
				}
			}
			break;

		case 0x5b: // Zip full speed
			if (m_accel_unlocked)
			{
				m_accel_fast = true;
				accel_full_speed();
			}
			break;

		case 0x5c: // Zip slot/speaker flags
			if (m_accel_unlocked)
			{
				m_accel_slotspk = data;
			}
			break;

		case 0x5f: // Zip game I/O flags
			if (m_accel_unlocked)
			{
				m_accel_gameio = data;
			}
			else
			{
				do_io(offset, false);
			}
			break;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d:
			if (m_isace2200)
			{
				if (offset == 0x78)
				{
					m_ace2200_axxx_bank = false;
				}
				else if (offset == 0x79)
				{
					m_ace2200_axxx_bank = true;
				}
			}
			if (m_auxslotdevice)
			{
				m_auxslotdevice->write_c07x(offset & 0xf, data);

				// card may have banked auxram; get a new bank pointer
				m_aux_bank_ptr = m_auxslotdevice->get_auxbank_ptr();
			}
			do_io(offset, false);    // make sure it also side-effect resets the paddles as documented
			break;

		case 0x7e:  // SETIOUDIS
			m_ioudis = true; break;

		case 0x7f:  // CLRIOUDIS
			m_ioudis = false; break;

		default:
			do_io(offset, false);
			break;
	}
}

u8 apple2e_state::c000_iic_r(offs_t offset)
{
	if(machine().side_effects_disabled()) return read_floatingbus();
	u8 uFloatingBus7 = read_floatingbus() & 0x7f;

	if ((offset & 0xf0) == 0x00) // keyboard latch, $C000 is really 00-0F
	{
		return m_transchar | m_strobe;
	}

	switch (offset)
	{
		case 0x10:  // read any key down, reset keyboard strobe
			{
				u8 rv = m_transchar | (m_anykeydown ? 0x80 : 0x00);
				m_strobe = 0;
				return rv;
			}

		case 0x11:  // read LCRAM2 (LC Dxxx bank), also reads like $C010 without strobe reset
			return (m_lcram2 ? 0x80 : 0x00) | m_transchar;

		case 0x12:  // read LCRAM (is LC readable?)
			return (m_lcram ? 0x80 : 0x00) | m_transchar;

		case 0x13:  // read RAMRD
			return (m_ramrd ? 0x80 : 0x00) | m_transchar;

		case 0x14:  // read RAMWRT
			return (m_ramwrt ? 0x80 : 0x00) | m_transchar;

		case 0x15:  // read & reset mouse X0 interrupt flag
			lower_irq(IRQ_MOUSEXY);
			return (m_xirq ? 0x80 : 0x00) | m_transchar;

		case 0x16:  // read ALTZP
			return (m_altzp ? 0x80 : 0x00) | m_transchar;

		case 0x17:  // read & reset mouse Y0 interrupt flag
			lower_irq(IRQ_MOUSEXY);
			return (m_yirq ? 0x80 : 0x00) | m_transchar;

		case 0x18:  // read 80STORE
			return (m_video->get_80store() ? 0x80 : 0x00) | m_transchar;

		case 0x19:  // read VBL
			return (m_vbl ? 0x80 : 0x00) | m_transchar;

		case 0x1a:  // read TEXT
			return (m_video->get_graphics() ? 0x00 : 0x80) | m_transchar;

		case 0x1b:  // read MIXED
			return (m_video->get_mix() ? 0x80 : 0x00) | m_transchar;

		case 0x1c:  // read PAGE2
			return (m_video->get_page2() ? 0x80 : 0x00) | m_transchar;

		case 0x1d:  // read HIRES
			return (m_video->get_hires() ? 0x80 : 0x00) | m_transchar;

		case 0x1e:  // read ALTCHARSET
			return (m_video->get_altcharset() ? 0x80 : 0x00) | m_transchar;

		case 0x1f:  // read 80COL
			return (m_video->get_80col() ? 0x80 : 0x00) | m_transchar;

		case 0x40:  // read XYMask (IIc only)
			return m_xy ? 0x80 : 0x00;

		case 0x41:  // read VBL mask (IIc only)
			return m_vblmask ? 0x80 : 0x00;

		case 0x42:  // read X0Edge (IIc only)
			return m_x0edge ? 0x80 : 0x00;

		case 0x43:  // read Y0Edge (IIc only)
			return m_y0edge ? 0x80 : 0x00;

		case 0x60: // 40/80 column switch (IIc only)
			return ((m_sysconfig->read() & 0x40) ? 0x80 : 0) | uFloatingBus7;

		case 0x61:  // button 0 or Open Apple or mouse button 1
		case 0x69:
			return ((m_gameio->sw0_r() || (m_kbspecial->read() & 0x10)) ? 0x80 : 0) | uFloatingBus7;

		case 0x62:  // button 1 or Solid Apple
		case 0x6a:
			return ((m_gameio->sw1_r() || (m_kbspecial->read() & 0x20)) ? 0x80 : 0) | uFloatingBus7;

		case 0x63:  // mouse button 2 (no other function on IIc)
		case 0x6b:
			return (m_mouseb->read() ? 0 : 0x80) | uFloatingBus7;

		case 0x64:  // joy 1 X axis
		case 0x6c:
			return ((machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x65:  // joy 1 Y axis
		case 0x6d:
			return ((machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x66: // mouse X1 (IIc only)
		case 0x6e:
			return (m_x1 ? 0x80 : 0) | uFloatingBus7;

		case 0x67: // mouse Y1 (IIc only)
		case 0x6f:
			return (m_y1 ? 0x80 : 0) | uFloatingBus7;

		case 0x7e:  // read IOUDIS
			m_vbl = false;
			lower_irq(IRQ_VBL);
			return (m_ioudis ? 0x80 : 0x00) | uFloatingBus7;

		case 0x7f:  // read DHIRES
			return (m_video->get_dhires() ? 0x00 : 0x80) | uFloatingBus7;

		default:
			do_io(offset, true);
			if ((m_accel_unlocked) && (offset == 0x5c))
			{
				return m_accel_slotspk;
			}
			break;
	}

	return read_floatingbus();
}

void apple2e_state::c000_iic_w(offs_t offset, u8 data)
{
	if(machine().side_effects_disabled()) return;

	if ((offset & 0xf0) == 0x10) // clear keyboard latch, $C010 is really 10-1F
	{
		m_strobe = 0;
		return;
	}

	switch (offset)
	{
		case 0x00:  // 80STOREOFF
			m_video->a80store_w(false);
			auxbank_update();
			break;

		case 0x01:  // 80STOREON
			m_video->a80store_w(true);
			auxbank_update();
			break;

		case 0x02:  // RAMRDOFF
			m_ramrd = false;
			auxbank_update();
			break;

		case 0x03:  // RAMRDON
			m_ramrd = true;
			auxbank_update();
			break;

		case 0x04:  // RAMWRTOFF
			m_ramwrt = false;
			auxbank_update();
			break;

		case 0x05:  // RAMWRTON
			m_ramwrt = true;
			auxbank_update();
			break;

		case 0x06:  // INTCXROMOFF
			m_intcxrom = false;
			update_slotrom_banks();
			break;

		case 0x07:  // INTCXROMON
			m_intcxrom = true;
			update_slotrom_banks();
			break;

		case 0x08:  // ALTZPOFF
			m_altzp = false;
			auxbank_update();
			break;

		case 0x09:  // ALTZPON
			m_altzp = true;
			auxbank_update();
			break;

		case 0x0a:  // SETINTC3ROM
			m_slotc3rom = false;
			update_slotrom_banks();
			break;

		case 0x0b:  // SETSLOTC3ROM
			m_slotc3rom = true;
			update_slotrom_banks();
			break;

		case 0x0c:  // 80COLOFF
			m_video->a80col_w(false);
			break;

		case 0x0d:  // 80COLON
			m_video->a80col_w(true);
			break;

		case 0x0e:  // ALTCHARSETOFF
			m_video->altcharset_w(false);
			break;

		case 0x0f:  // ALTCHARSETON
			m_video->altcharset_w(true);
			break;

		case 0x5a:  // IIC+ accelerator unlock
			if (m_isiicplus)
			{
				if (data == 0x5a)
				{
					m_accel_stage++;
					if (m_accel_stage == 4)
					{
						m_accel_unlocked = true;
					}
				}
				else if (data == 0xa5)
				{
					// lock
					m_accel_unlocked = false;
					m_accel_stage = 0;
				}
				else if (m_accel_unlocked)
				{
					m_accel_fast = false;
					accel_normal_speed();
				}
				else
				{
					do_io(offset, true);
				}
			}
			break;

		case 0x5b:  // Set fast speed on any write
			if (m_accel_unlocked)
			{
				m_accel_fast = true;
				accel_full_speed();
			}
			do_io(offset, true);
			break;

		case 0x5c:
			if (m_accel_unlocked)
			{
				m_accel_slotspk = data;
			}
			do_io(offset, true);
			break;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
				if (m_auxslotdevice)
				{
					m_auxslotdevice->write_c07x(offset & 0xf, data);

					// card may have banked auxram; get a new bank pointer
					m_aux_bank_ptr = m_auxslotdevice->get_auxbank_ptr();
				}
				break;

		case 0x78:  // IIc mirror of SETIOUDIS used by the mouse firmware
		case 0x7a:
		case 0x7c:
		case 0x7e:  // SETIOUDIS
			m_ioudis = true;
			m_vbl = false;
			lower_irq(IRQ_VBL);
			break;

		case 0x79:  // IIc mirror of CLRIOUDIS used by the mouse firmware
		case 0x7b:
		case 0x7d:
		case 0x7f:  // CLRIOUDIS
			m_ioudis = false;
			m_vbl = false;
			lower_irq(IRQ_VBL);
			break;

		default:
			do_io(offset, true);
			break;
	}
}

void apple2e_state::update_iic_mouse()
{
	int new_mx, new_my;

	// read the axes and check for changes
	new_mx = m_mousex->read();
	new_my = m_mousey->read();

	// did X change?
	if (new_mx != last_mx)
	{
		int diff = new_mx - last_mx;

		/* check for wrap */
		if (diff > 0x80)
		{
			diff -= 0x100;
		}
		else
		{
			if (diff < -0x80)
			{
				diff += 0x100;
			}
		}

		count_x += diff;
		last_mx = new_mx;
	}

	// did Y change?
	if (new_my != last_my)
	{
		int diff = new_my - last_my;

		/* check for wrap */
		if (diff > 0x80)
		{
			diff -= 0x100;
		}
		else
		{
			if (diff < -0x80)
			{
				diff += 0x100;
			}
		}

		count_y += diff;
		last_my = new_my;
	}

	if (count_x)
	{
		if (count_x < 0)
		{
			count_x++;
			m_x1 = false;
		}
		else
		{
			count_x--;
			m_x1 = true;
		}

		// x0 is going to state-flip, generate an IRQ
		if (((m_x0) && (m_x0edge)) || // falling?
			(!(m_x0) && !(m_x0edge))) // rising?
		{
			if (m_xy)
			{
				m_xirq = true;
				raise_irq(IRQ_MOUSEXY);
			}
		}

		m_x0 = !m_x0;
	}
	else if (count_y)
	{
		if (count_y < 0)
		{
			count_y++;
			m_y1 = true;
		}
		else
		{
			count_y--;
			m_y1 = false;
		}

		// y0 is going to state-flip, generate an IRQ
		if (((m_y0) && (m_y0edge)) || // falling?
			(!(m_y0) && !(m_y0edge))) // rising?
		{
			if (m_xy)
			{
				m_yirq = true;
				raise_irq(IRQ_MOUSEXY);
			}
		}

		m_y0 = !m_y0;
	}
}

u8 apple2e_state::laser_mouse_r(offs_t offset)
{
	u8 uFloatingBus7 = read_floatingbus() & 0x7f;

	switch (offset)
	{
		case 0x8:  // read X0Edge
			return (m_x0edge ? 0x80 : 0x00) | uFloatingBus7;

		case 0x9:  // read Y0Edge
			return (m_y0edge ? 0x80 : 0x00) | uFloatingBus7;

		case 0xa:  // read XYMask
			return (m_xy ? 0x80 : 0x00) | uFloatingBus7;

		case 0xb:  // read VBL mask
			return (m_vblmask ? 0x80 : 0x00) | uFloatingBus7;

		case 0xc: // mouse X1 IRQ status
			return (m_xirq ? 0x80 : 0) | uFloatingBus7;

		case 0xd: // mouse Y1 IRQ status
			return (m_yirq ? 0x80 : 0) | uFloatingBus7;

		case 0xe: // VBL interrupt status
			return ((m_irqmask & IRQ_VBL) ? 0x80 : 0) | uFloatingBus7;
	}

	return 0xff;
}

void apple2e_state::laser_mouse_w(offs_t offset, u8 data)
{
	// these are the same as the //c mouse registers, but with bit 4 of the
	// address inverted, pretty much.
	switch (offset)
	{
		case 0x0:  // RisX0Edge
			m_x0edge = false; break;

		case 0x1:  // FalX0Edge
			m_x0edge = true; break;

		case 0x2:  // RisY0Edge
			m_y0edge = false; break;

		case 0x3:  // FalY0Edge
			m_y0edge = true; break;

		case 0x4:  // DisXY
			m_xy = false; break;

		case 0x5:  // EnbXY
			m_xy = true; break;

		case 0x6:  // DisVBL
			lower_irq(IRQ_VBL);
			m_vblmask = false; break;

		case 0x7:  // EnVBL
			m_vblmask = true; break;

		case 0xf: // clear XY interrupt
			lower_irq(IRQ_MOUSEXY);
			m_xirq = m_yirq = false;
			break;
	}
}

u8 apple2e_state::c080_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		int slot;

		offset &= 0x7F;
		slot = offset / 0x10;

		if (slot == 0)
		{
			lc_update(offset & 0xf, false);
		}
		else
		{
			if (m_accel_present)
			{
				accel_slot(slot);
			}

			if ((m_isiicplus) && (slot == 6))
			{
				return m_iwm->read(offset % 0x10);
			}

			if (m_slotdevice[slot] != nullptr)
			{
				return m_slotdevice[slot]->read_c0nx(offset % 0x10);
			}
			else
			{
				if ((m_iscec) && (slot == 3))
				{
					return m_cec_bank;
				}
			}
		}
	}

	return read_floatingbus();
}

void apple2e_state::c080_w(offs_t offset, u8 data)
{
	int slot;

	offset &= 0x7F;
	slot = offset / 0x10;

	if (slot == 0)
	{
		lc_update(offset & 0xf, true);
	}
	else
	{
		if ((m_isiicplus) && (slot == 6))
		{
			m_iwm->write(offset % 0x10, data);
			return;
		}

		if (m_slotdevice[slot] != nullptr)
		{
			m_slotdevice[slot]->write_c0nx(offset % 0x10, data);
		}
		else
		{
			//if ((m_iscec) && (slot == 3))
			if ((m_iscec) && (!m_iscecm) && (slot == 3))
			{
				if (data != m_cec_bank)
				{
					m_cec_bank = data;
					if (data & 0x10)
					{
						m_video->monohgr_w(false);
					}
					else
					{
						m_video->monohgr_w(true);
					}

					auxbank_update();
					update_slotrom_banks();
				}
			}
		}
	}
}

u8 apple2e_state::read_slot_rom(int slotbias, int offset)
{
	int slotnum = ((offset>>8) & 0xf) + slotbias;

	if (m_slotdevice[slotnum] != nullptr)
	{
		if ((m_cnxx_slot == CNXX_UNCLAIMED) && (m_slotdevice[slotnum]->take_c800()) && (!machine().side_effects_disabled()))
		{
			m_cnxx_slot = slotnum;
			update_slotrom_banks();
		}

		return m_slotdevice[slotnum]->read_cnxx(offset&0xff);
	}

	return read_floatingbus();
}

void apple2e_state::write_slot_rom(int slotbias, int offset, u8 data)
{
	int slotnum = ((offset>>8) & 0xf) + slotbias;

	if ((m_iscec) && (m_iscecm) && ( slotnum == 6 ) && (!m_intcxrom) )
	{
		if (data != m_cec_bank)
		{
			m_cec_bank = data;
/*
            if (data & 0x10)
            {
                m_video->m_monohgr = false;
            }
            else
            {
                m_video->m_monohgr = true;
            }
*/
			auxbank_update();
			update_slotrom_banks();
		}
	}

	if (m_slotdevice[slotnum] != nullptr)
	{
		if ((m_cnxx_slot == CNXX_UNCLAIMED) && (m_slotdevice[slotnum]->take_c800()) && (!machine().side_effects_disabled()))
		{
			m_cnxx_slot = slotnum;
			update_slotrom_banks();
		}

		m_slotdevice[slotnum]->write_cnxx(offset&0xff, data);
	}
}

u8 apple2e_state::read_int_rom(int slotbias, int offset)
{
	int slot = ((slotbias + offset) >> 8) & 0xf;

	// slot 4 can't remap because the IRQ handler is there
	if ((m_isace500) && (m_ace_cnxx_bank) && (slot != 4))
	{
		slotbias += 0x4000;
		// even numbered slots come from $6x00 in this mode?
		if (!(slot & 1))
		{
			slotbias += 0x2000;
		}
		return m_rom_ptr[slotbias + offset];
	}

	return m_ds1315->read(slotbias + offset);
}

u8 apple2e_state::nsc_backing_r(offs_t offset) { return m_rom_ptr[offset]; }

u8 apple2e_state::c100_r(offs_t offset)  { accel_slot(1 + ((offset >> 8) & 0x7)); return read_slot_rom(1, offset); }
u8 apple2e_state::c100_int_r(offs_t offset)  { accel_slot(1 + ((offset >> 8) & 0x7)); return read_int_rom(0x100, offset); }
u8 apple2e_state::c100_int_bank_r(offs_t offset)  { accel_slot(1 + ((offset >> 8) & 0x7)); return read_int_rom(0x4100, offset); }
u8 apple2e_state::c100_cec_r(offs_t offset)  { return m_rom_ptr[0xc100 + offset]; }
u8 apple2e_state::c100_cec_bank_r(offs_t offset)  { return m_rom_ptr[0x4100 + offset]; }
void apple2e_state::c100_w(offs_t offset, u8 data) { accel_slot(1); write_slot_rom(1, offset, data); }
u8 apple2e_state::c300_r(offs_t offset)  { accel_slot(3 + ((offset >> 8) & 0x7)); return read_slot_rom(3, offset); }

u8 apple2e_state::c300_int_r(offs_t offset)
{
	accel_slot(3 + ((offset >> 8) & 0x7));
	if ((!m_slotc3rom) && !machine().side_effects_disabled())
	{
		m_intc8rom = true;
		update_slotrom_banks();
	}
	return read_int_rom(0x300, offset);
}

u8 apple2e_state::c300_int_bank_r(offs_t offset)
{
	accel_slot(3 + ((offset >> 8) & 0x7));
	if ((!m_slotc3rom) && !machine().side_effects_disabled())
	{
		m_intc8rom = true;
		update_slotrom_banks();
	}
	return read_int_rom(0x4300, offset);
}

void apple2e_state::c300_w(offs_t offset, u8 data)
{
	accel_slot(3 + ((offset >> 8) & 0x7));
	if ((!m_slotc3rom) && !machine().side_effects_disabled())
	{
		m_intc8rom = true;
		update_slotrom_banks();
	}

	write_slot_rom(3, offset, data);
}

u8 apple2e_state::c300_cec_r(offs_t offset)  { return m_rom_ptr[0xc300 + offset]; }
u8 apple2e_state::c300_cec_bank_r(offs_t offset)  { return m_rom_ptr[0x4300 + offset]; }

u8 apple2e_state::c400_r(offs_t offset)  { accel_slot(4 + ((offset >> 8) & 0x7)); return read_slot_rom(4, offset); }
u8 apple2e_state::c400_int_r(offs_t offset)
{
	accel_slot(4 + ((offset >> 8) & 0x7));
	if ((offset < 0x100) && (m_mockingboard4c))
	{
		return read_slot_rom(4, offset);
	}

	return read_int_rom(0x400, offset);
}

u8 apple2e_state::c400_int_bank_r(offs_t offset)
{
	accel_slot(4 + ((offset >> 8) & 0x7));
	if ((offset < 0x100) && (m_mockingboard4c))
	{
		return read_slot_rom(4, offset);
	}

	return read_int_rom(0x4400, offset);
}

void apple2e_state::c400_w(offs_t offset, u8 data)
{
	accel_slot(4 + ((offset >> 8) & 0x7));
	if ((m_isiic) && (offset < 0x100))
	{
		m_mockingboard4c = true;
	}

	write_slot_rom(4, offset, data);
}

u8 apple2e_state::c400_cec_r(offs_t offset)  { return m_rom_ptr[0xc400 + offset]; }
u8 apple2e_state::c400_cec_bank_r(offs_t offset)  { return m_rom_ptr[0x4400 + offset]; }

void apple2e_state::c400_cec_w(offs_t offset, u8 data)
{
	if ((m_iscecm))
	{
		write_slot_rom(4, offset, data);
	}
}

u8 apple2e_state::c800_r(offs_t offset)
{
	if ((offset == 0x7ff) && !machine().side_effects_disabled())
	{
		u8 rv = 0xff;

		if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != nullptr))
		{
			rv = m_slotdevice[m_cnxx_slot]->read_c800(offset&0xfff);
		}
		m_cnxx_slot = CNXX_UNCLAIMED;
		m_intc8rom = false;
		update_slotrom_banks();
		return rv;
	}

	if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		return m_slotdevice[m_cnxx_slot]->read_c800(offset&0xfff);
	}

	return read_floatingbus();
}

u8 apple2e_state::ace500_c0bx_r(offs_t offset)
{
	switch (offset)
	{
		// Printer "auto-LF" DIP switch in bit 7
		case 0x0:
			return (m_sysconfig->read() & 0x80);

		// Alt key status (0=pressed).  Reads as pressed for function keys.
		case 0x4:
			{
				m_strobe = 0;
				const u8 rv = m_franklin_strobe;
				m_franklin_strobe = 0x80;
				return rv;
			}
			break;

		// Used by the IRQ handler.  Appears to return altzp status in bit 7, same as $C016.
		case 0xc:
			return (m_altzp ? 0x80 : 0x00);

		default:
			logerror("c0bx_r @ %x (PC=%x)\n", offset, m_maincpu->pc());
			break;
	}

	return 0xff;
}


// $C0B8 - $C100-$C7FF alt bank
// $C0B9 - $C100-$C7FF main bank
// $C0BA - release $C800 force, use $CFFF mechanism
// $C0BB - force main rom bank at $C800
void apple2e_state::ace500_c0bx_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x8:
			m_ace_cnxx_bank = true;
			break;

		case 0x9:
			m_ace_cnxx_bank = false;
			break;

		case 0xa:
			m_ace500rombank = 0;
			break;

		case 0xb:
			m_ace500rombank = 0x7000;
			break;

		default:
			logerror("unknown c0bx_w %x (PC=%x)\n", offset, m_maincpu->pc());
			break;
	}
}

u8 apple2e_state::c800_int_r(offs_t offset)
{
	if ((offset == 0x7ff) && !machine().side_effects_disabled())
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		m_intc8rom = false;
		update_slotrom_banks();
	}

	if (m_iscec)
	{
		return m_rom_ptr[0x4800 + offset];
	}

	if (m_isace500)
	{
		return m_rom_ptr[m_ace500rombank + offset];
	}

	return m_rom_ptr[0x800 + offset];
}

u8 apple2e_state::c800_b2_int_r(offs_t offset)
{
	if ((m_isiicplus) && (m_romswitch) && (((offset >= 0x400) && (offset < 0x500)) || ((offset >= 0x600) && (offset < 0x700))))
	{
		return mig_r(offset-0x400);
	}

	if ((offset == 0x7ff) && !machine().side_effects_disabled())
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		m_intc8rom = false;
		update_slotrom_banks();
	}

	return m_rom_ptr[0x4800 + offset];
}

void apple2e_state::c800_w(offs_t offset, u8 data)
{
	if ((m_isace500) && (offset == 0x7ff))
	{
		// TODO: use a version of our conventional CnXX handling for this
		u8 page = (m_maincpu->pc() >> 8) & 0xf;
		switch (page)
		{
			case 1:
				m_ace500rombank = 0x4800;
				break;

			case 3:
				m_ace500rombank = 0x800;
				break;

			case 5:
				m_ace500rombank = 0x5800;
				break;
		}
		return;
	}

	if ((m_isiicplus) && (m_romswitch) && (((offset >= 0x400) && (offset < 0x500)) || ((offset >= 0x600) && (offset < 0x700))))
	{
		mig_w(offset-0x400, data);
		return;
	}

	if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		m_slotdevice[m_cnxx_slot]->write_c800(offset&0xfff, data);
	}

	if (offset == 0x7ff)
	{
		if (!machine().side_effects_disabled())
		{
			m_cnxx_slot = CNXX_UNCLAIMED;
			m_intc8rom = false;
			update_slotrom_banks();
		}
	}
}

u8 apple2e_state::c800_cec_r(offs_t offset)  { return m_rom_ptr[0xc800 + offset]; }
u8 apple2e_state::c800_cec_bank_r(offs_t offset)  { return m_rom_ptr[0x4800 + offset]; }

u8 apple2e_state::inh_r(offs_t offset)
{
	if (m_inh_slot != -1)
	{
		return m_slotdevice[m_inh_slot]->read_inh_rom(offset + 0xd000);
	}

	assert(0);  // hitting inh_r with invalid m_inh_slot should not be possible
	return read_floatingbus();
}

void apple2e_state::inh_w(offs_t offset, u8 data)
{
	if (m_inh_slot != -1)
	{
		m_slotdevice[m_inh_slot]->write_inh_rom(offset + 0xd000, data);
	}
}

u8 apple2e_state::lc_romswitch_r(offs_t offset)
{
	return m_rom_ptr[0x5000 + offset];
}

void apple2e_state::lc_romswitch_w(offs_t offset, u8 data)
{
	lc_w(offset, data);
}

u8 apple2e_state::lc_r(offs_t offset)
{
	if ((m_altzp) && (!m_iscec))
	{
		if (m_aux_bank_ptr)
		{
			if (offset < 0x1000)
			{
				if (m_lcram2)
				{
					return m_aux_bank_ptr[((offset & 0xfff) + 0xd000) & m_aux_mask];
				}
				else
				{
					return m_aux_bank_ptr[((offset & 0xfff) + 0xc000) & m_aux_mask];
				}
			}

			return m_aux_bank_ptr[((offset & 0x1fff) + 0xe000) & m_aux_mask];
		}
		else
		{
			return read_floatingbus();
		}
	}
	else
	{
		if (offset < 0x1000)
		{
			if (m_lcram2)
			{
				return m_ram_ptr[(offset & 0xfff) + 0xd000];
			}
			else
			{
				return m_ram_ptr[(offset & 0xfff) + 0xc000];
			}
		}

		return m_ram_ptr[(offset & 0x1fff) + 0xe000];
	}
}

void apple2e_state::lc_w(offs_t offset, u8 data)
{
	if (!m_lcwriteenable)
	{
		return;
	}

	if ((m_altzp) && (!m_iscec))
	{
		if (m_aux_bank_ptr)
		{
			if (offset < 0x1000)
			{
				if (m_lcram2)
				{
					m_aux_bank_ptr[((offset & 0xfff) + 0xd000) & m_aux_mask] = data;
				}
				else
				{
					m_aux_bank_ptr[((offset & 0xfff) + 0xc000) & m_aux_mask] = data;
				}
				return;
			}

			m_aux_bank_ptr[((offset & 0x1fff) + 0xe000) & m_aux_mask] = data;
		}
	}
	else
	{
		if (offset < 0x1000)
		{
			if (m_lcram2)
			{
				m_ram_ptr[(offset & 0xfff) + 0xd000] = data;
			}
			else
			{
				m_ram_ptr[(offset & 0xfff) + 0xc000] = data;
			}
			return;
		}

		m_ram_ptr[(offset & 0x1fff) + 0xe000] = data;
	}
}

// floating bus code from old machine/apple2: now works reasonably well with French Touch and Deater "vapor lock" stuff
u8 apple2e_state::read_floatingbus()
{
	enum
	{
		// scanner constants
		kHBurstClock      =    53, // clock when Color Burst starts
		kHBurstClocks     =     4, // clocks per Color Burst duration
		kHClock0State     =  0x18, // H[543210] = 011000
		kHClocks          =    65, // clocks per horizontal scan (including HBL)
		kHPEClock         =    40, // clock when HPE (horizontal preset enable) goes low
		kHPresetClock     =    41, // clock when H state presets
		kHSyncClock       =    49, // clock when HSync starts
		kHSyncClocks      =     4, // clocks per HSync duration
		kNTSCScanLines    =   262, // total scan lines including VBL (NTSC)
		kNTSCVSyncLine    =   224, // line when VSync starts (NTSC)
		kPALScanLines     =   312, // total scan lines including VBL (PAL)
		kPALVSyncLine     =   264, // line when VSync starts (PAL)
		kVLine0State      = 0x100, // V[543210CBA] = 100000000
		kVPresetLine      =   256, // line when V state presets
		kVSyncLines       =     4, // lines per VSync duration
	};

	const int kClocksPerVSync = kHClocks * (m_pal ? kPALScanLines : kNTSCScanLines);

	// vars
	//
	int i, Hires, Mixed, Page2, _80Store, ScanLines, /* VSyncLine, ScanCycles,*/
		h_clock, h_state, h_0, h_1, h_2, h_3, h_4, h_5,
		v_line, v_state, v_A, v_B, v_C, v_0, v_1, v_2, v_3, v_4, /* v_5, */
		_hires, addend0, addend1, addend2, sum, address;

	// video scanner data
	//
	i = m_maincpu->total_cycles() % kClocksPerVSync; // cycles into this VSync

	// machine state switches
	//
	Hires = (m_video->get_hires() && m_video->get_graphics()) ? 1 : 0;
	Mixed = m_video->get_mix() ? 1 : 0;
	Page2 = m_video->get_page2() ? 1 : 0;
	_80Store = m_video->get_80store() ? 1 : 0;

	// calculate video parameters according to display standard
	// we call this "PAL", but it's also for SECAM
	ScanLines  = m_pal ? kPALScanLines : kNTSCScanLines;

	// calculate horizontal scanning state
	h_clock = (i + 63) % kHClocks; // which horizontal scanning clock
	h_state = kHClock0State + h_clock; // H state bits
	if (h_clock >= kHPresetClock) // check for horizontal preset
	{
		h_state -= 1; // correct for state preset (two 0 states)
	}
	h_0 = (h_state >> 0) & 1; // get horizontal state bits
	h_1 = (h_state >> 1) & 1;
	h_2 = (h_state >> 2) & 1;
	h_3 = (h_state >> 3) & 1;
	h_4 = (h_state >> 4) & 1;
	h_5 = (h_state >> 5) & 1;

	// calculate vertical scanning state
	//
	v_line  = (i / kHClocks) + 192; // which vertical scanning line (MAME starts at blank, not the beginning of the scanning area)
	v_state = kVLine0State + v_line; // V state bits
	if ((v_line >= kVPresetLine)) // check for previous vertical state preset
	{
		v_state -= ScanLines; // compensate for preset
	}
	v_A = (v_state >> 0) & 1; // get vertical state bits
	v_B = (v_state >> 1) & 1;
	v_C = (v_state >> 2) & 1;
	v_0 = (v_state >> 3) & 1;
	v_1 = (v_state >> 4) & 1;
	v_2 = (v_state >> 5) & 1;
	v_3 = (v_state >> 6) & 1;
	v_4 = (v_state >> 7) & 1;
	//v_5 = (v_state >> 8) & 1;

	// calculate scanning memory address
	//
	_hires = Hires;
	if (Hires && Mixed && (v_4 & v_2))
	{
		_hires = 0; // (address is in text memory)
	}

	addend0 = 0x68; // 1            1            0            1
	addend1 =              (h_5 << 5) | (h_4 << 4) | (h_3 << 3);
	addend2 = (v_4 << 6) | (v_3 << 5) | (v_4 << 4) | (v_3 << 3);
	sum     = (addend0 + addend1 + addend2) & (0x0F << 3);

	address = 0;
	address |= h_0 << 0; // a0
	address |= h_1 << 1; // a1
	address |= h_2 << 2; // a2
	address |= sum;      // a3 - aa6
	address |= v_0 << 7; // a7
	address |= v_1 << 8; // a8
	address |= v_2 << 9; // a9
	address |= ((_hires) ? v_A : (1 ^ (Page2 & (1 ^ _80Store)))) << 10; // a10
	address |= ((_hires) ? v_B : (Page2 & (1 ^ _80Store))) << 11; // a11
	if (_hires) // hires?
	{
		// Y: insert hires only address bits
		//
		address |= v_C << 12; // a12
		address |= (1 ^ (Page2 & (1 ^ _80Store))) << 13; // a13
		address |= (Page2 & (1 ^ _80Store)) << 14; // a14
	}

	return m_ram_ptr[address % m_ram_size];
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

u8   apple2e_state::ram0000_r(offs_t offset)          { return m_ram_ptr[offset]; }
void apple2e_state::ram0000_w(offs_t offset, u8 data) { m_ram_ptr[offset] = data; }
u8   apple2e_state::ram0200_r(offs_t offset)          { return m_ram_ptr[offset+0x200]; }
void apple2e_state::ram0200_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x200] = data; }
u8   apple2e_state::ram0400_r(offs_t offset)          { return m_ram_ptr[offset+0x400]; }
void apple2e_state::ram0400_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x400] = data; }
u8   apple2e_state::ram0800_r(offs_t offset)          { return m_ram_ptr[offset+0x800]; }
void apple2e_state::ram0800_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x800] = data; }
u8   apple2e_state::ram2000_r(offs_t offset)          { return m_ram_ptr[offset+0x2000]; }
void apple2e_state::ram2000_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x2000] = data; }
u8   apple2e_state::ram4000_r(offs_t offset)          { return m_ram_ptr[offset+0x4000]; }
void apple2e_state::ram4000_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x4000] = data; }
void apple2e_state::ram8000_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x8000] = data; }

u8 apple2e_state::ram4000_ace2200_r(offs_t offset)
{
	if ((offset >= 0x6000) && (m_ace2200_axxx_bank))
	{
		return m_rom_ptr[0x4000 + (offset - 0x6000)];
	}
	return m_ram_ptr[offset+0x4000];
}

u8 apple2e_state::cec4000_r(offs_t offset)
{
	//printf("cec4000_r: ofs %x\n", offset);
	return m_cec_remap[((m_cec_bank & 0xf) << 14) + offset];
}

u8 apple2e_state::cec8000_r(offs_t offset)
{
	//printf("cec8000_r: ofs %x\n", offset);
	if (m_cec_bank & 0x20)
	{
		return m_rom_ptr[offset];
	}
	else
	{
		return m_rom_ptr[offset + 0x8000];
	}
}

u8   apple2e_state::auxram0000_r(offs_t offset)          { if (m_aux_bank_ptr) { return m_aux_bank_ptr[offset & m_aux_mask]; } else { return read_floatingbus(); } }
void apple2e_state::auxram0000_w(offs_t offset, u8 data) { if (m_aux_bank_ptr) { m_aux_bank_ptr[offset & m_aux_mask] = data; } }
u8   apple2e_state::auxram0200_r(offs_t offset)          { if (m_aux_bank_ptr) { return m_aux_bank_ptr[(offset+0x200) & m_aux_mask]; } else { return read_floatingbus(); } }
void apple2e_state::auxram0200_w(offs_t offset, u8 data) { if (m_aux_bank_ptr) { m_aux_bank_ptr[(offset+0x200) & m_aux_mask] = data; } }
u8   apple2e_state::auxram0400_r(offs_t offset)          { if (m_aux_bank_ptr) { return m_aux_bank_ptr[(offset+0x400) & m_aux_mask]; } else { return read_floatingbus(); } }
void apple2e_state::auxram0400_w(offs_t offset, u8 data) { if (m_aux_bank_ptr) { m_aux_bank_ptr[(offset+0x400) & m_aux_mask] = data; } }
u8   apple2e_state::auxram0800_r(offs_t offset)          { if (m_aux_bank_ptr) { return m_aux_bank_ptr[(offset+0x800) & m_aux_mask]; } else { return read_floatingbus(); } }
void apple2e_state::auxram0800_w(offs_t offset, u8 data) { if (m_aux_bank_ptr) { m_aux_bank_ptr[(offset+0x800) & m_aux_mask] = data; } }
u8   apple2e_state::auxram2000_r(offs_t offset)          { if (m_aux_bank_ptr) { return m_aux_bank_ptr[(offset+0x2000) & m_aux_mask]; } else { return read_floatingbus(); } }
void apple2e_state::auxram2000_w(offs_t offset, u8 data) { if (m_aux_bank_ptr) { m_aux_bank_ptr[(offset+0x2000) & m_aux_mask] = data; } }
u8   apple2e_state::auxram4000_r(offs_t offset)          { if (m_aux_bank_ptr) { return m_aux_bank_ptr[(offset+0x4000) & m_aux_mask]; } else { return read_floatingbus(); } }
void apple2e_state::auxram4000_w(offs_t offset, u8 data) { if (m_aux_bank_ptr) { m_aux_bank_ptr[(offset + 0x4000) & m_aux_mask] = data; } }

void apple2e_state::base_map(address_map &map)
{
	map(0x0000, 0x01ff).view(m_0000bank);
	m_0000bank[0](0x0000, 0x01ff).rw(FUNC(apple2e_state::ram0000_r), FUNC(apple2e_state::ram0000_w));
	m_0000bank[1](0x0000, 0x01ff).rw(FUNC(apple2e_state::auxram0000_r), FUNC(apple2e_state::auxram0000_w));

	map(0x0200, 0x03ff).view(m_0200bank);
	m_0200bank[0](0x0200, 0x03ff).rw(FUNC(apple2e_state::ram0200_r), FUNC(apple2e_state::ram0200_w));         // wr 0 rd 0
	m_0200bank[1](0x0200, 0x03ff).rw(FUNC(apple2e_state::auxram0200_r), FUNC(apple2e_state::ram0200_w));      // wr 0 rd 1
	m_0200bank[2](0x0200, 0x03ff).rw(FUNC(apple2e_state::ram0200_r), FUNC(apple2e_state::auxram0200_w));      // wr 1 rd 0
	m_0200bank[3](0x0200, 0x03ff).rw(FUNC(apple2e_state::auxram0200_r), FUNC(apple2e_state::auxram0200_w)); // wr 1 rd 1

	map(0x0400, 0x07ff).view(m_0400bank);
	m_0400bank[0](0x0400, 0x07ff).rw(FUNC(apple2e_state::ram0400_r), FUNC(apple2e_state::ram0400_w));         // wr 0 rd 0
	m_0400bank[1](0x0400, 0x07ff).rw(FUNC(apple2e_state::auxram0400_r), FUNC(apple2e_state::ram0400_w));      // wr 0 rd 1
	m_0400bank[2](0x0400, 0x07ff).rw(FUNC(apple2e_state::ram0400_r), FUNC(apple2e_state::auxram0400_w));      // wr 1 rd 0
	m_0400bank[3](0x0400, 0x07ff).rw(FUNC(apple2e_state::auxram0400_r), FUNC(apple2e_state::auxram0400_w)); // wr 1 rd 1

	map(0x0800, 0x1fff).view(m_0800bank);
	m_0800bank[0](0x0800, 0x1fff).rw(FUNC(apple2e_state::ram0800_r), FUNC(apple2e_state::ram0800_w));
	m_0800bank[1](0x0800, 0x1fff).rw(FUNC(apple2e_state::auxram0800_r), FUNC(apple2e_state::ram0800_w));
	m_0800bank[2](0x0800, 0x1fff).rw(FUNC(apple2e_state::ram0800_r), FUNC(apple2e_state::auxram0800_w));
	m_0800bank[3](0x0800, 0x1fff).rw(FUNC(apple2e_state::auxram0800_r), FUNC(apple2e_state::auxram0800_w));

	map(0x2000, 0x3fff).view(m_2000bank);
	m_2000bank[0](0x2000, 0x3fff).rw(FUNC(apple2e_state::ram2000_r), FUNC(apple2e_state::ram2000_w));
	m_2000bank[1](0x2000, 0x3fff).rw(FUNC(apple2e_state::auxram2000_r), FUNC(apple2e_state::ram2000_w));
	m_2000bank[2](0x2000, 0x3fff).rw(FUNC(apple2e_state::ram2000_r), FUNC(apple2e_state::auxram2000_w));
	m_2000bank[3](0x2000, 0x3fff).rw(FUNC(apple2e_state::auxram2000_r), FUNC(apple2e_state::auxram2000_w));

	map(0x4000, 0xbfff).view(m_4000bank);
	m_4000bank[0](0x4000, 0xbfff).rw(FUNC(apple2e_state::ram4000_r), FUNC(apple2e_state::ram4000_w));
	m_4000bank[1](0x4000, 0xbfff).rw(FUNC(apple2e_state::auxram4000_r), FUNC(apple2e_state::ram4000_w));
	m_4000bank[2](0x4000, 0xbfff).rw(FUNC(apple2e_state::ram4000_r), FUNC(apple2e_state::auxram4000_w));
	m_4000bank[3](0x4000, 0xbfff).rw(FUNC(apple2e_state::auxram4000_r), FUNC(apple2e_state::auxram4000_w));
	m_4000bank[4](0x4000, 0x7fff).rw(FUNC(apple2e_state::cec4000_r), FUNC(apple2e_state::ram4000_w));
	m_4000bank[4](0x8000, 0xbfff).rw(FUNC(apple2e_state::cec8000_r), FUNC(apple2e_state::ram8000_w));

	map(0xc000, 0xc07f).rw(FUNC(apple2e_state::c000_r), FUNC(apple2e_state::c000_w));
	map(0xc080, 0xc0ff).rw(FUNC(apple2e_state::c080_r), FUNC(apple2e_state::c080_w));

	map(0xc100, 0xc2ff).view(m_c100bank);
	m_c100bank[0](0xc100, 0xc2ff).rw(FUNC(apple2e_state::c100_r), FUNC(apple2e_state::c100_w));
	m_c100bank[1](0xc100, 0xc2ff).rw(FUNC(apple2e_state::c100_int_r), FUNC(apple2e_state::c100_w));
	m_c100bank[2](0xc100, 0xc2ff).rw(FUNC(apple2e_state::c100_int_bank_r), FUNC(apple2e_state::c100_w));
	m_c100bank[3](0xc100, 0xc2ff).r(FUNC(apple2e_state::c100_cec_r)).nopw();
	m_c100bank[4](0xc100, 0xc2ff).r(FUNC(apple2e_state::c100_cec_bank_r)).nopw();

	map(0xc300, 0xc3ff).view(m_c300bank);
	m_c300bank[0](0xc300, 0xc3ff).rw(FUNC(apple2e_state::c300_r), FUNC(apple2e_state::c300_w));
	m_c300bank[1](0xc300, 0xc3ff).rw(FUNC(apple2e_state::c300_int_r), FUNC(apple2e_state::c300_w));
	m_c300bank[2](0xc300, 0xc3ff).rw(FUNC(apple2e_state::c300_int_bank_r), FUNC(apple2e_state::c300_w));
	m_c300bank[3](0xc300, 0xc3ff).r(FUNC(apple2e_state::c300_cec_r)).nopw();
	m_c300bank[4](0xc300, 0xc3ff).r(FUNC(apple2e_state::c300_cec_bank_r)).nopw();

	map(0xc400, 0xc7ff).view(m_c400bank);
	m_c400bank[0](0xc400, 0xc7ff).rw(FUNC(apple2e_state::c400_r), FUNC(apple2e_state::c400_w));
	m_c400bank[1](0xc400, 0xc7ff).rw(FUNC(apple2e_state::c400_int_r), FUNC(apple2e_state::c400_w));
	m_c400bank[2](0xc400, 0xc7ff).rw(FUNC(apple2e_state::c400_int_bank_r), FUNC(apple2e_state::c400_w));
	m_c400bank[3](0xc400, 0xc7ff).rw(FUNC(apple2e_state::c400_cec_r), FUNC(apple2e_state::c400_cec_w));
	m_c400bank[4](0xc400, 0xc7ff).r(FUNC(apple2e_state::c400_cec_bank_r)).nopw();

	map(0xc800, 0xcfff).view(m_c800bank);
	m_c800bank[0](0xc800, 0xcfff).rw(FUNC(apple2e_state::c800_r), FUNC(apple2e_state::c800_w));
	m_c800bank[1](0xc800, 0xcfff).rw(FUNC(apple2e_state::c800_int_r), FUNC(apple2e_state::c800_w));
	m_c800bank[2](0xc800, 0xcfff).rw(FUNC(apple2e_state::c800_b2_int_r), FUNC(apple2e_state::c800_w));
	m_c800bank[3](0xc800, 0xcfff).r(FUNC(apple2e_state::c800_cec_r)).nopw();
	m_c800bank[4](0xc800, 0xcfff).r(FUNC(apple2e_state::c800_cec_bank_r)).nopw();

	map(0xd000, 0xffff).view(m_upperbank);
	m_upperbank[0](0xd000, 0xffff).view(m_lcbank);
	m_upperbank[1](0xd000, 0xffff).rw(FUNC(apple2e_state::inh_r), FUNC(apple2e_state::inh_w));

	m_lcbank[0](0x0d000, 0x0ffff).rom().region("maincpu", 0x1000).w(FUNC(apple2e_state::lc_w));
	m_lcbank[1](0x0d000, 0x0ffff).rw(FUNC(apple2e_state::lc_r), FUNC(apple2e_state::lc_w));
	m_lcbank[2](0x0d000, 0x0ffff).rw(FUNC(apple2e_state::lc_romswitch_r), FUNC(apple2e_state::lc_romswitch_w));
	m_lcbank[3](0x0d000, 0x0ffff).rom().region("maincpu", 0x5000).w(FUNC(apple2e_state::lc_w));
	m_lcbank[4](0x0d000, 0x0ffff).rw(FUNC(apple2e_state::lc_r), FUNC(apple2e_state::lc_w));
	m_lcbank[5](0x0d000, 0x1ffff).rom().region("maincpu", 0xd000).w(FUNC(apple2e_state::lc_w));
}

void apple2e_state::apple2c_map(address_map &map)
{
	base_map(map);
	map(0xc000, 0xc07f).rw(FUNC(apple2e_state::c000_iic_r), FUNC(apple2e_state::c000_iic_w));
	map(0xc098, 0xc09b).rw(m_acia1, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xc0a8, 0xc0ab).rw(m_acia2, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
}

void apple2e_state::apple2c_memexp_map(address_map &map)
{
	base_map(map);
	map(0xc000, 0xc07f).rw(FUNC(apple2e_state::c000_iic_r), FUNC(apple2e_state::c000_iic_w));
	map(0xc098, 0xc09b).rw(m_acia1, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xc0a8, 0xc0ab).rw(m_acia2, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xc0c0, 0xc0c3).rw(FUNC(apple2e_state::memexp_r), FUNC(apple2e_state::memexp_w));
}

void apple2e_state::laser128_map(address_map &map)
{
	base_map(map);
	map(0xc000, 0xc07f).rw(FUNC(apple2e_state::c000_laser_r), FUNC(apple2e_state::c000_laser_w));
	map(0xc090, 0xc097).w(FUNC(apple2e_state::laserprn_w));
	map(0xc098, 0xc09b).rw(m_acia1, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xc0a8, 0xc0ab).rw(m_acia2, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xc0c0, 0xc0cf).rw(FUNC(apple2e_state::laser_mouse_r), FUNC(apple2e_state::laser_mouse_w));
	map(0xc0d0, 0xc0d3).rw(FUNC(apple2e_state::memexp_r), FUNC(apple2e_state::memexp_w));
	map(0xc0e0, 0xc0ef).rw(m_iwm, FUNC(applefdintf_device::read), FUNC(applefdintf_device::write));
	map(0xc0e8, 0xc0e9).rw(FUNC(apple2e_state::laser_motor_r), FUNC(apple2e_state::laser_motor_w));
	map(0xc1c1, 0xc1c1).r(FUNC(apple2e_state::laserprn_busy_r));
}

void apple2e_state::ace500_map(address_map &map)
{
	base_map(map);
	map(0xc000, 0xc07f).rw(FUNC(apple2e_state::c000_iic_r), FUNC(apple2e_state::c000_iic_w));
	map(0xc0a8, 0xc0ab).rw(m_acia1, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xc090, 0xc097).w(FUNC(apple2e_state::laserprn_w));
	map(0xc0b0, 0xc0bf).rw(FUNC(apple2e_state::ace500_c0bx_r), FUNC(apple2e_state::ace500_c0bx_w));
	map(0xc1c1, 0xc1c1).r(FUNC(apple2e_state::laserprn_busy_r));
}

void apple2e_state::ace2200_map(address_map &map)
{
	base_map(map);

	// change the banking here to acommodate the Ace 2x00's ROM banking in at $A000
	m_4000bank[0](0x4000, 0xbfff).rw(FUNC(apple2e_state::ram4000_ace2200_r), FUNC(apple2e_state::ram4000_w));

	map(0xc090, 0xc097).w(FUNC(apple2e_state::laserprn_w));
	map(0xc1c1, 0xc1c1).r(FUNC(apple2e_state::laserprn_busy_r));
}

void apple2e_state::spectred_keyb_map(address_map &map)
{
		map(0x0000, 0x07ff).rom();
		map(0x0800, 0x0fff).ram();
}

/***************************************************************************
    KEYBOARD
***************************************************************************/

int apple2e_state::ay3600_shift_r()
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

int apple2e_state::ay3600_control_r()
{
	if (m_kbspecial->read() & 0x08)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

void apple2e_state::ay3600_data_ready_w(int state)
{
	if (state == ASSERT_LINE)
	{
		u8 *decode = m_kbdrom->base();
		u16 trans;

		// if the user presses a valid key to start the driver from the info screen,
		// we will see that key.  ignore keys in the first 25,000 cycles (in my tests,
		// the unwanted key shows up at 17030 cycles)
		if (m_maincpu->total_cycles() < 25000)
		{
			return;
		}

		m_lastchar = m_ay3600->b_r();

		trans = m_lastchar & ~(0x1c0);  // clear the 3600's control/shift stuff
		trans |= (m_lastchar & 0x100)>>2;   // bring the 0x100 bit down to the 0x40 place
		trans <<= 2;                    // 4 entries per key
		if (m_iscec)
		{
			trans |= (m_kbspecial->read() & 0x06) ? 0x01 : 0x00;    // shift is bit 1 (active high)
			trans |= (m_kbspecial->read() & 0x08) ? 0x02 : 0x00;    // control is bit 2 (active high)
		}
		else
		{
			trans |= (m_kbspecial->read() & 0x06) ? 0x00 : 0x01;    // shift is bit 1 (active low)
			trans |= (m_kbspecial->read() & 0x08) ? 0x00 : 0x02;    // control is bit 2 (active low)
		}
		trans |= (m_kbspecial->read() & 0x01) ? 0x0000 : 0x0200;    // caps lock is bit 9 (active low)

		if (m_isiic)
		{
			if (m_sysconfig->read() & 0x80)
			{
				trans += 0x400; // go to DVORAK half of the ROM
			}
		}

		m_transchar = decode[trans];
		m_strobe = 0x80;

		//printf("new char = %04x (%02x)\n", m_lastchar, m_transchar);
	}
}

void apple2e_state::ay3600_ako_w(int state)
{
	m_anykeydown = (state == ASSERT_LINE) ? true : false;

	if (m_anykeydown)
	{
		m_repeatdelay = 10;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(apple2e_state::ay3600_repeat)
{
	// is the key still down?
	if (m_anykeydown)
	{
		if (m_repeatdelay)
		{
			m_repeatdelay--;
		}
		else
		{
			m_strobe = 0x80;
		}
	}
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( apple2_sysconfig_accel )
	PORT_START("a2_config")
	PORT_CONFNAME(0x10, 0x00, "CPU type")
	PORT_CONFSETTING(0x00, "Standard")
	PORT_CONFSETTING(0x10, "4 MHz Zip Chip")

	PORT_CONFNAME(0x20, 0x00, "Bootup speed")
	PORT_CONFSETTING(0x00, "Standard")
	PORT_CONFSETTING(0x20, "4 MHz")
INPUT_PORTS_END

static INPUT_PORTS_START( laser128_sysconfig )
	PORT_START("a2_config")
	PORT_CONFNAME(0x08, 0x00, "Printer type")
	PORT_CONFSETTING(0x00, "Serial")
	PORT_CONFSETTING(0x08, "Parallel")
INPUT_PORTS_END

static INPUT_PORTS_START( apple2c_sysconfig )
	PORT_START("a2_config")
	PORT_CONFNAME(0x40, 0x40, "40/80 Columns")
	PORT_CONFSETTING(0x00, "80 columns")
	PORT_CONFSETTING(0x40, "40 columns")
	PORT_CONFNAME(0x80, 0x00, "QWERTY/DVORAK")
	PORT_CONFSETTING(0x00, "QWERTY")
	PORT_CONFSETTING(0x80, "DVORAK")

	PORT_CONFNAME(0x10, 0x00, "CPU type")
	PORT_CONFSETTING(0x00, "Standard")
	PORT_CONFSETTING(0x10, "4 MHz Zip Chip")
	PORT_CONFNAME(0x20, 0x00, "Bootup speed")
	PORT_CONFSETTING(0x00, "Standard")
	PORT_CONFSETTING(0x20, "4 MHz")
INPUT_PORTS_END

static INPUT_PORTS_START( apple2cp_sysconfig )
	PORT_START("a2_config")
	PORT_CONFNAME(0x04, 0x04, "40/80 Columns")
	PORT_CONFSETTING(0x00, "80 columns")
	PORT_CONFSETTING(0x04, "40 columns")
	PORT_CONFNAME(0x08, 0x00, "QWERTY/DVORAK")
	PORT_CONFSETTING(0x00, "QWERTY")
	PORT_CONFSETTING(0x08, "DVORAK")
INPUT_PORTS_END

	/*
	  Apple IIe & IIc key matrix (from "Sams ComputerFacts: Apple IIe" and "Sams ComputerFacts: Apple IIc")

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X0  | ESC |  1  |  2  |  3  |  4  |  6  |  5  |  7  |  8  |  9  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  | TAB |  Q  |  W  |  E  |  R  |  Y  |  T  |  U  |  I  |  O  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  |  A  |  D  |  S  |  H  |  F  |  G  |  J  |  K  | ;:  |  L  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  |  Z  |  X  |  C  |  V  |  B  |  M  |  N  | ,<  | .>  |  /? |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  |     |     |     |     |     |     | \|  | +=  |  0  | -_  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X5  |     |     |     |     |     |     | `~  |  P  | [{  | ]}  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X6  |     |     |     |     |     |     |RETRN| UP  | SPC | '"  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X7  |     |     |     |     |     |     | DEL |DOWN |LEFT |RIGHT|
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	*/

	/*
	  Apple IIe platinum key matrix

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X0  | ESC |  1  |  2  |  3  |  4  |  6  |  5  |  7  |  8  |  9  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  | TAB |  Q  |  W  |  E  |  R  |  Y  |  T  |  U  |  I  |  O  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  |  A  |  D  |  S  |  H  |  F  |  G  |  J  |  K  | ;:  |  L  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  |  Z  |  X  |  C  |  V  |  B  |  M  |  N  | ,<  | .>  |  /? |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  | KP/ |     | KP0 | KP1 | KP2 | KP3 | \|  | +=  |  0  | -_  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X5  |     |KPEsc| KP4 | KP5 | KP6 | KP7 | `~  |  P  | [{  | ]}  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X6  | KP* |     | KP8 | KP9 | KP. | KP+ |RETRN| UP  | SPC | '"  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X7  |     |     |     | KP- |KPENT|     | DEL |DOWN |LEFT |RIGHT|
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	*/

static INPUT_PORTS_START( apple2e_common )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)  PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")       PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")       PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2e_special )
	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

static INPUT_PORTS_START( ceci )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("F4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x240, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("F5")
	PORT_BIT(0x340, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("CN")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x300, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x340, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("EN")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_NAME("STOP")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_NAME("TEST")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("F1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x280, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("F2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x280, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("F3")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

static INPUT_PORTS_START( cecm )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("F4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x240, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("F5")
	PORT_BIT(0x340, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("CN")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x300, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x340, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("EN")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_NAME("STOP")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_NAME("TEST")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("F1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x280, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("F2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x280, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("F3")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

static INPUT_PORTS_START( zijini )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("F4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x240, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("EN")
	PORT_BIT(0x340, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("CN")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x300, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x340, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_NAME("STOP")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_NAME("TEST")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("F1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x280, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("F2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x280, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("F3")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2e )
	PORT_INCLUDE( apple2e_common )
	PORT_INCLUDE( apple2e_special )
	PORT_INCLUDE( apple2_sysconfig_accel )
INPUT_PORTS_END

static INPUT_PORTS_START( apple2c )
	PORT_INCLUDE( apple2e_common )
	PORT_INCLUDE( apple2e_special )
	PORT_INCLUDE( apple2c_sysconfig )

	PORT_START(MOUSE_BUTTON_TAG) /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START(MOUSE_XAXIS_TAG) /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(20) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START(MOUSE_YAXIS_TAG) /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(20) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( laser128 )
	PORT_INCLUDE( apple2e_common )
	PORT_INCLUDE( laser128_sysconfig )

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Triangle")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Triangle")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)

	PORT_START(MOUSE_BUTTON_TAG) /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START(MOUSE_XAXIS_TAG) /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(20) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START(MOUSE_YAXIS_TAG) /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(20) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( ace500 )
	PORT_INCLUDE( apple2e_common )

	PORT_START("a2_config")
	PORT_CONFNAME(0x80, 0x00, "Auto Line Feed for printer")
	PORT_CONFSETTING(0x80, DEF_STR(On))
	PORT_CONFSETTING(0x00, DEF_STR(Off))

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open F")       PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid F")      PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)

	PORT_START("franklin_fkeys")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)       PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)       PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)       PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)       PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)       PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)       PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)       PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)       PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)      PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)      PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)      PORT_CHAR(UCHAR_MAMEKEY(F12))

	PORT_START(MOUSE_BUTTON_TAG) /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START(MOUSE_XAXIS_TAG) /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(20) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START(MOUSE_YAXIS_TAG) /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(20) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2cp )
	PORT_INCLUDE( apple2e_common )
	PORT_INCLUDE( apple2e_special )
	PORT_INCLUDE( apple2cp_sysconfig )

	PORT_START(MOUSE_BUTTON_TAG) /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START(MOUSE_XAXIS_TAG) /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(20) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START(MOUSE_YAXIS_TAG) /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(20) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2euk )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR(0xa3)  // a3 is Unicode for the pound sign
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")   PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)

	PORT_INCLUDE(apple2_sysconfig_accel)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2ees )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR(0xa3)  // a3 is Unicode for the pound sign
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0xf1) PORT_CHAR(0xf1)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('`') PORT_CHAR(0xbf)  // inverted question mark
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('\'') PORT_CHAR('?')

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('~') PORT_CHAR('^')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR(0xc7) PORT_CHAR(0xa1) // c with cedilla / inverted exclamation point

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")   PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)

	PORT_INCLUDE(apple2_sysconfig_accel)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2efr )   // French AZERTY keyboard (Apple uses the Belgian AZERTY layout in France also)
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('&')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR(0xe9) // e with acute
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('\"')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('\'')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('(')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR(0xa7) // section sign
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR(0xe8) // e with grave
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('!')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(0xe7) // c with cedilla

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('?') PORT_CHAR(',')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR('.') PORT_CHAR(';')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('/') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('+') PORT_CHAR('=')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(0xa3) PORT_CHAR('`')  // UK pound (actually to the left of the return key on the QSDF row)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('_') PORT_CHAR('-')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(0xe0) // a with grave
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR(0xb0) PORT_CHAR(')') // degree symbol

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('>') PORT_CHAR('<')   // actually the key between left shift and W
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0xa8) PORT_CHAR('^') // diaresis
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('$') PORT_CHAR('*')

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")   PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)

	PORT_INCLUDE(apple2_sysconfig_accel)
INPUT_PORTS_END

INPUT_PORTS_START( apple2ep )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)  PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)   PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)   PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)   PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)   PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)   PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Esc") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)   PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)   PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)   PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)   PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)    PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")   PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)

	PORT_INCLUDE(apple2_sysconfig_accel)
INPUT_PORTS_END

static void apple2eaux_cards(device_slot_interface &device)
{
	device.option_add("std80", A2EAUX_STD80COL); // Apple IIe Standard 80 Column Card
	device.option_add("ext80", A2EAUX_EXT80COL); // Apple IIe Extended 80 Column Card
	device.option_add("rw3", A2EAUX_RAMWORKS3);  // Applied Engineering RamWorks III
}

void apple2e_state::apple2e_common(machine_config &config, bool enhanced, bool rgb_option)
{
	/* basic machine hardware */
	if (enhanced)
	{
		M65C02(config, m_maincpu, 1021800);
	}
	else
	{
		M6502(config, m_maincpu, 1021800);
	}
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::base_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	TIMER(config, m_scantimer, 0).configure_scanline(FUNC(apple2e_state::apple2_interrupt), "screen", 0, 1);
	config.set_maximum_quantum(attotime::from_hz(60));

	TIMER(config, m_acceltimer, 0).configure_generic(FUNC(apple2e_state::accel_timer));

	if (rgb_option)
	{
		APPLE2_VIDEO_COMPOSITE_RGB(config, m_video, XTAL(14'318'181)).set_screen(m_screen);
	}
	else
	{
		APPLE2_VIDEO_COMPOSITE(config, m_video, XTAL(14'318'181)).set_screen(m_screen);
	}

	APPLE2_COMMON(config, m_a2common, XTAL(14'318'181));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IIE, false, false>)));
	m_screen->set_palette(m_video);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, A2_SPEAKER_TAG).add_route(ALL_OUTPUTS, "mono", 0.4);

	/* DS1315 for no-slot clock */
	DS1315(config, m_ds1315, 0).read_backing().set(FUNC(apple2e_state::nsc_backing_r));

	/* RAM */
	RAM(config, m_ram).set_default_size("64K").set_default_value(0x00);

	/* keyboard controller */
	ay3600_device &kbdc(AY3600(config, "ay3600", 0));
	kbdc.x0().set_ioport("X0");
	kbdc.x1().set_ioport("X1");
	kbdc.x2().set_ioport("X2");
	kbdc.x3().set_ioport("X3");
	kbdc.x4().set_ioport("X4");
	kbdc.x5().set_ioport("X5");
	kbdc.x6().set_ioport("X6");
	kbdc.x7().set_ioport("X7");
	kbdc.x8().set_ioport("X8");
	kbdc.shift().set(FUNC(apple2e_state::ay3600_shift_r));
	kbdc.control().set(FUNC(apple2e_state::ay3600_control_r));
	kbdc.data_ready().set(FUNC(apple2e_state::ay3600_data_ready_w));
	kbdc.ako().set(FUNC(apple2e_state::ay3600_ako_w));

	/* repeat timer.  15 Hz from page 7-15 of "Understanding the Apple IIe" */
	timer_device &timer(TIMER(config, "repttmr", 0));
	timer.configure_periodic(FUNC(apple2e_state::ay3600_repeat), attotime::from_hz(15));

	/* slot devices */
	A2BUS(config, m_a2bus, 0);
	m_a2bus->set_space(m_maincpu, AS_PROGRAM);
	m_a2bus->irq_w().set(FUNC(apple2e_state::a2bus_irq_w));
	m_a2bus->nmi_w().set(FUNC(apple2e_state::a2bus_nmi_w));
	m_a2bus->inh_w().set(FUNC(apple2e_state::a2bus_inh_w));
	m_a2bus->dma_w().set_inputline(m_maincpu, INPUT_LINE_HALT);
	A2BUS_SLOT(config, "sl1", m_a2bus, apple2e_cards, nullptr);
	A2BUS_SLOT(config, "sl2", m_a2bus, apple2e_cards, nullptr);
	A2BUS_SLOT(config, "sl3", m_a2bus, apple2e_cards, nullptr);
	A2BUS_SLOT(config, "sl4", m_a2bus, apple2e_cards, "mockingboard");
	A2BUS_SLOT(config, "sl5", m_a2bus, apple2e_cards, nullptr);
	A2BUS_SLOT(config, "sl6", m_a2bus, apple2e_cards, "diskiing");
	A2BUS_SLOT(config, "sl7", m_a2bus, apple2e_cards, nullptr);

	A2EAUXSLOT(config, m_a2eauxslot, 0);
	m_a2eauxslot->set_space(m_maincpu, AS_PROGRAM);
	m_a2eauxslot->out_irq_callback().set(FUNC(apple2e_state::a2bus_irq_w));
	m_a2eauxslot->out_nmi_callback().set(FUNC(apple2e_state::a2bus_nmi_w));
	A2EAUXSLOT_SLOT(config, "aux", m_a2eauxslot, apple2eaux_cards, "ext80");   // default to an extended 80-column card

	APPLE2_GAMEIO(config, m_gameio, apple2_gameio_device::default_options, nullptr);

	/* softlist config for baseline A2E
	By default, filter lists where possible to compatible disks for A2E */
	SOFTWARE_LIST(config, "flop_a2_clean").set_original("apple2_flop_clcracked");
	SOFTWARE_LIST(config, "flop_a2_orig").set_compatible("apple2_flop_orig").set_filter(enhanced ? "A2EE" : "A2E");
	SOFTWARE_LIST(config, "flop_a2_misc").set_compatible("apple2_flop_misc");

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void apple2e_state::apple2e(machine_config &config)
{
	apple2e_common(config, false, true);
}

void apple2e_state::apple2epal(machine_config &config)
{
	apple2e(config);
	M6502(config.replace(), m_maincpu, 1016966);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::base_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));
	m_screen->set_raw(1016966 * 14, (65 * 7) * 2, 0, (40 * 7) * 2, 312, 0, 192);
}

void apple2e_state::mprof3(machine_config &config)
{
	apple2e(config);
	/* internal ram */
	m_ram->set_default_size("128K");
}

void apple2e_state::apple2ee(machine_config &config)
{
	apple2e_common(config, true, true);
}

void apple2e_state::apple2eepal(machine_config &config)
{
	apple2ee(config);
	M65C02(config.replace(), m_maincpu, 1016966);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::base_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	m_screen->set_raw(1016966 * 14, (65 * 7) * 2, 0, (40 * 7) * 2, 312, 0, 192);
}

void apple2e_state::spectred(machine_config &config)
{
	apple2e(config);
	i8035_device &keyb_mcu(I8035(config, "keyb_mcu", XTAL(4'000'000))); /* guessed frequency */
	keyb_mcu.set_addrmap(AS_PROGRAM, &apple2e_state::spectred_keyb_map);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IIE, false, true>)));

	// TODO: implement the actual interfacing to this 8035 MCU and
	//       and then remove the keyb CPU inherited from apple2e
}

void apple2e_state::tk3000(machine_config &config)
{
	apple2e(config);
	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::base_map);

//  z80_device &subcpu(Z80(config, "subcpu", 1021800));    // schematics are illegible on where the clock comes from, but it *seems* to be the same as the 65C02 clock
//  subcpu.set_addrmap(AS_PROGRAM, &apple2e_state::tk3000_kbd_map);
}

void apple2e_state::apple2ep(machine_config &config)
{
	apple2e(config);
	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::base_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));
}

void apple2e_state::apple2c(machine_config &config)
{
	apple2e_common(config, true, false);
	subdevice<software_list_device>("flop_a2_orig")->set_filter("A2C");  // Filter list to compatible disks for this machine.

	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::apple2c_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	// IIc and friends have no cassette port
	config.device_remove(A2_CASSETTE_TAG);

	config.device_remove("sl1");   // IIc has no slots, of course :)
	config.device_remove("sl2");
	config.device_remove("sl3");
	config.device_remove("sl4");
	config.device_remove("sl5");
	config.device_remove("sl6");
	config.device_remove("sl7");

	MOS6551(config, m_acia1, 0);
	m_acia1->set_xtal(XTAL(14'318'181) / 8); // ~1.789 MHz
	m_acia1->txd_handler().set(PRINTER_PORT_TAG, FUNC(rs232_port_device::write_txd));
	m_acia1->irq_handler().set(FUNC(apple2e_state::a2bus_irq_w));

	MOS6551(config, m_acia2, 0);
	m_acia2->set_xtal(1.8432_MHz_XTAL);   // matches SSC so modem software is compatible
	m_acia2->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_acia2->irq_handler().set(FUNC(apple2e_state::a2bus_irq_w));

	rs232_port_device &printer(RS232_PORT(config, PRINTER_PORT_TAG, default_rs232_devices, nullptr));
	printer.rxd_handler().set(m_acia1, FUNC(mos6551_device::write_rxd));
	printer.dcd_handler().set(m_acia1, FUNC(mos6551_device::write_dcd));
	printer.dsr_handler().set(m_acia1, FUNC(mos6551_device::write_dsr));
	printer.cts_handler().set(m_acia1, FUNC(mos6551_device::write_cts));

	rs232_port_device &modem(RS232_PORT(config, MODEM_PORT_TAG, default_rs232_devices, nullptr));
	modem.rxd_handler().set(m_acia2, FUNC(mos6551_device::write_rxd));
	modem.dcd_handler().set(m_acia2, FUNC(mos6551_device::write_dcd));
	modem.dsr_handler().set(m_acia2, FUNC(mos6551_device::write_dsr));
	modem.cts_handler().set(m_acia2, FUNC(mos6551_device::write_cts));

	// TODO: populate the IIc's other virtual slots with ONBOARD_ADD
	A2BUS_MOCKINGBOARD(config, "sl4", A2BUS_7M_CLOCK).set_onboard(m_a2bus); // Mockingboard 4C
	A2BUS_DISKIING(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);

	config.device_remove("aux");
	config.device_remove(A2_AUXSLOT_TAG);

	m_ram->set_default_size("128K").set_extra_options("128K");
}

void apple2e_state::apple2cp(machine_config &config)
{
	apple2c(config);
	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::apple2c_memexp_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	config.device_remove("sl4");
	config.device_remove("sl6");

	IWM(config, m_iwm, A2BUS_7M_CLOCK, 1021800*2);
	m_iwm->phases_cb().set(FUNC(apple2e_state::phases_w));
	m_iwm->sel35_cb().set(FUNC(apple2e_state::sel35_w));
	m_iwm->devsel_cb().set(FUNC(apple2e_state::devsel_w));

	applefdintf_device::add_525(config, m_floppy[0]);
	applefdintf_device::add_525(config, m_floppy[1]);
	applefdintf_device::add_35(config, m_floppy[2]);
	applefdintf_device::add_35(config, m_floppy[3]);

	m_ram->set_default_size("128K").set_extra_options("128K, 384K, 640K, 896K, 1152K");
}

void apple2e_state::apple2c_iwm(machine_config &config)
{
	apple2c(config);

	config.device_remove("sl6");
	A2BUS_IWM(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
}

void apple2e_state::apple2c_mem(machine_config &config)
{
	apple2c(config);
	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::apple2c_memexp_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	config.device_remove("sl6");
	A2BUS_IWM(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);

	m_ram->set_default_size("128K").set_extra_options("128K, 384K, 640K, 896K, 1152K");
}

void apple2e_state::laser128(machine_config &config)
{
	apple2c(config);
	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::laser128_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IIE, true, false>)));

	IWM(config, m_iwm, A2BUS_7M_CLOCK, 1021800 * 2);
	m_iwm->phases_cb().set(FUNC(apple2e_state::phases_w));
	m_iwm->devsel_cb().set(FUNC(apple2e_state::devsel_w));

	applefdintf_device::add_525(config, m_floppy[0]);
	applefdintf_device::add_525(config, m_floppy[1]);

	config.device_remove("sl4");
	config.device_remove("sl6");

	A2BUS_LASER128(config, "sl1", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl2", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl3", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl4", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_SLOT(config, "sl5", m_a2bus, apple2e_cards, nullptr);
	A2BUS_LASER128(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_SLOT(config, "sl7", m_a2bus, apple2e_cards, nullptr);

	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->busy_handler().set(FUNC(apple2e_state::busy_w));
	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);

	m_ram->set_default_size("128K").set_extra_options("128K, 384K, 640K, 896K, 1152K");
}

void apple2e_state::laser128o(machine_config &config)
{
	apple2c(config);
	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::laser128_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IIE, true, false>)));

	IWM(config, m_iwm, A2BUS_7M_CLOCK, 1021800 * 2);
	m_iwm->phases_cb().set(FUNC(apple2e_state::phases_w));
	m_iwm->devsel_cb().set(FUNC(apple2e_state::devsel_w));

	applefdintf_device::add_525(config, m_floppy[0]);
	applefdintf_device::add_525(config, m_floppy[1]);

	config.device_remove("sl4");
	config.device_remove("sl6");

	A2BUS_LASER128_ORIG(config, "sl1", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128_ORIG(config, "sl2", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128_ORIG(config, "sl3", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128_ORIG(config, "sl4", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_SLOT(config, "sl5", m_a2bus, apple2e_cards, nullptr);
	A2BUS_LASER128_ORIG(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_SLOT(config, "sl7", m_a2bus, apple2e_cards, nullptr);

	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->busy_handler().set(FUNC(apple2e_state::busy_w));
	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);

	// original Laser 128 doesn't have the Slinky memory expansion
	m_ram->set_default_size("128K").set_extra_options("128K");
}

void apple2e_state::laser128ex2(machine_config &config)
{
	apple2c(config);
	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::laser128_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IIE, true, false>)));

	IWM(config, m_iwm, A2BUS_7M_CLOCK, 1021800 * 2);
	m_iwm->phases_cb().set(FUNC(apple2e_state::phases_w));
	m_iwm->devsel_cb().set(FUNC(apple2e_state::devsel_w));

	applefdintf_device::add_525(config, m_floppy[0]);
	applefdintf_device::add_525(config, m_floppy[1]);

	config.device_remove("sl4");
	config.device_remove("sl6");

	A2BUS_LASER128(config, "sl1", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl2", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl3", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl4", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl5", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_LASER128(config, "sl7", A2BUS_7M_CLOCK).set_onboard(m_a2bus);

	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->busy_handler().set(FUNC(apple2e_state::busy_w));
	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);

	m_ram->set_default_size("128K").set_extra_options("128K, 384K, 640K, 896K, 1152K");
}

void apple2e_state::ace500(machine_config &config)
{
	apple2e_common(config, true, false);
	subdevice<software_list_device>("flop_a2_orig")->set_filter("A2C");  // Filter list to compatible disks for this machine.

	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::ace500_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IIE, true, true>)));

	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->busy_handler().set(FUNC(apple2e_state::busy_w));
	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);

	MOS6551(config, m_acia1, 0);
	m_acia1->set_xtal(1.8432_MHz_XTAL);
	m_acia1->txd_handler().set(MODEM_PORT_TAG, FUNC(rs232_port_device::write_txd));
	m_acia1->irq_handler().set(FUNC(apple2e_state::a2bus_irq_w));

	rs232_port_device &modem(RS232_PORT(config, MODEM_PORT_TAG, default_rs232_devices, nullptr));
	modem.rxd_handler().set(m_acia1, FUNC(mos6551_device::write_rxd));
	modem.dcd_handler().set(m_acia1, FUNC(mos6551_device::write_dcd));
	modem.dsr_handler().set(m_acia1, FUNC(mos6551_device::write_dsr));
	modem.cts_handler().set(m_acia1, FUNC(mos6551_device::write_cts));

	config.device_remove(A2_CASSETTE_TAG);
	config.device_remove("sl1");
	config.device_remove("sl2");
	config.device_remove("sl3");
	config.device_remove("sl4");
	config.device_remove("sl5");
	config.device_remove("sl6");
	config.device_remove("sl7");
	config.device_remove("aux");

	A2BUS_IWM(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);

	A2EAUX_FRANKLIN384(config, "aux", A2BUS_7M_CLOCK).set_onboard(m_a2eauxslot);

	m_ram->set_default_size("128K");
}

void apple2e_state::ace2200(machine_config &config)
{
	apple2e_common(config, false, false);
	M65C02(config.replace(), m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2e_state::ace2200_map);
	m_maincpu->set_dasm_override(FUNC(apple2e_state::dasm_trampoline));

	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IIE, true, true>)));

	// The Ace 2000 series has 3 physical slots, 2, 4/7, and 5.
	// 4/7 can be slot 4 or 7 via a jumper; we fix it to slot 7 here
	// because that's most useful (for e.g. cffa202).
	config.device_remove("sl1");
	config.device_remove("sl3");
	config.device_remove("sl4");
	config.device_remove("sl5");
	config.device_remove("sl6");

	A2BUS_ACE2X00_SLOT1(config, "sl1", A2BUS_7M_CLOCK).set_onboard(m_a2bus);
	A2BUS_SLOT(config, "sl5", m_a2bus, apple2e_cards, "mockingboard");
	A2BUS_ACE2X00_SLOT6(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);

	config.device_remove("aux");
	A2EAUX_FRANKLIN512(config, "aux", A2BUS_7M_CLOCK).set_onboard(m_a2eauxslot);

	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->busy_handler().set(FUNC(apple2e_state::busy_w));
	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);

	m_ram->set_default_size("128K");
}

void apple2e_state::cec(machine_config &config)
{
	apple2e_common(config, false, false);

	config.device_remove("sl3");
	config.device_remove("sl6");

	A2BUS_DISKIING(config, "sl6", A2BUS_7M_CLOCK).set_onboard(m_a2bus);

	SOFTWARE_LIST(config, "flop_cec").set_original("cecflop");

	// there is no aux slot, the "aux" side of the //e is used for additional ROM
	config.device_remove("aux");
	config.device_remove(A2_AUXSLOT_TAG);

	m_ram->set_default_size("64K");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(apple2e)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0133-a.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "342-0133-a.chr", 0x1000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ( "342-0135-b.64", 0x0000, 0x2000, CRC(e248835e) SHA1(523838c19c79f481fa02df56856da1ec3816d16e))
	ROM_LOAD ( "342-0134-a.64", 0x2000, 0x2000, CRC(fc3d59d8) SHA1(8895a4b703f2184b673078f411f4089889b61c54))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(apple2euk)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "341-0160-a.chr", 0x0000, 0x2000, CRC(9be77112) SHA1(48aafa9a72002c495bc1f3d28150630ff89ca47e) )

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ( "342-0135-b.64", 0x0000, 0x2000, CRC(e248835e) SHA1(523838c19c79f481fa02df56856da1ec3816d16e))
	ROM_LOAD ( "342-0134-a.64", 0x2000, 0x2000, CRC(fc3d59d8) SHA1(8895a4b703f2184b673078f411f4089889b61c54))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "341-0150-a.e12", 0x000, 0x800, CRC(66ffacd7) SHA1(47bb9608be38ff75429a989b930a93b47099648e) )
ROM_END

ROM_START(apple2ees)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "341-0212-a.e9", 0x000000, 0x002000, CRC(bc5575ef) SHA1(aa20c257255ef552295d32a3f56ccbb52b8716c3) )

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ( "342-0135-b.64", 0x0000, 0x2000, CRC(e248835e) SHA1(523838c19c79f481fa02df56856da1ec3816d16e))
	ROM_LOAD ( "342-0134-a.64", 0x2000, 0x2000, CRC(fc3d59d8) SHA1(8895a4b703f2184b673078f411f4089889b61c54))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "341-0211-a.f12", 0x000000, 0x000800, CRC(fac15d54) SHA1(abe019de22641b0647e829a1d4745759bdffe86a) )
ROM_END

ROM_START(mprof3)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "mpf3.chr", 0x0000, 0x1000,CRC(2597bc19) SHA1(e114dcbb512ec24fb457248c1b53cbd78039ed20))
	ROM_LOAD ( "mpf3.chr", 0x1000, 0x1000,CRC(2597bc19) SHA1(e114dcbb512ec24fb457248c1b53cbd78039ed20))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ( "mpf3-cd.rom", 0x0000, 0x2000, CRC(5b662e06) SHA1(aa0db775ca78986480829fcc10f00e57629e1a7c))
	ROM_LOAD ( "mpf3-ef.rom", 0x2000, 0x2000, CRC(2c5e8b92) SHA1(befeb03e04b7c3ef36ef5829948a53880df85e92))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real mprof keyboard ROM
ROM_END

ROM_START(apple2ee)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "342-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ( "342-0304-a.e10", 0x0000, 0x2000, CRC(443aa7c4) SHA1(3aecc56a26134df51e65e17f33ae80c1f1ac93e6)) /* PCB: "CD ROM // 342-0304", 2364 mask rom */
	ROM_LOAD ( "342-0303-a.e8", 0x2000, 0x2000, CRC(95e10034) SHA1(afb09bb96038232dc757d40c0605623cae38088e)) /* PCB: "EF ROM // 342-0303", 2364 mask rom */

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2eeuk)
	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD( "342-0273-a.chr", 0x0000, 0x2000, CRC(9157085a) SHA1(85479a509d6c8176949a5b20720567b7022aa631) )

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ( "342-0304-a.e10", 0x0000, 0x2000, CRC(443aa7c4) SHA1(3aecc56a26134df51e65e17f33ae80c1f1ac93e6)) /* PCB: "CD ROM // 342-0304", 2364 mask rom */
	ROM_LOAD ( "342-0303-a.e8", 0x2000, 0x2000, CRC(95e10034) SHA1(afb09bb96038232dc757d40c0605623cae38088e)) /* PCB: "EF ROM // 342-0303", 2364 mask rom */

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0150-a.e12", 0x000, 0x800, CRC(66ffacd7) SHA1(47bb9608be38ff75429a989b930a93b47099648e) )
ROM_END

ROM_START(apple2eefr)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "342-0274-a.e9", 0x0000, 0x2000, CRC(8f342081) SHA1(c81c1bbf237e70f8c3e5eef3c8fd5bd9b9f54d1e) )

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD( "342-0304-a.e3", 0x0000, 0x2000, CRC(443aa7c4) SHA1(3aecc56a26134df51e65e17f33ae80c1f1ac93e6) )
	ROM_LOAD( "342-0303-a.e5", 0x2000, 0x2000, CRC(95e10034) SHA1(afb09bb96038232dc757d40c0605623cae38088e) )

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "341-0326-a.f12", 0x0000, 0x0800, CRC(7e79f3fa) SHA1(17f22593e94c1e59ba110d4b32c3334ef418885c) )
ROM_END

ROM_START(apple2ep)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "342-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ("32-0349-b.128", 0x0000, 0x4000, CRC(1d70b193) SHA1(b8ea90abe135a0031065e01697c4a3a20d51198b)) /* should rom name be 342-0349-b? */

	ROM_REGION( 0x800, "keyboard", 0 )
	// chip printed markings say 342-0132-d, but internally text says "341-0132-d".  Go figure.
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2c)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ( "a2c.128", 0x0000, 0x4000, CRC(f0edaa1b) SHA1(1a9b8aca5e32bb702ddb7791daddd60a89655729)) /* should be 342-0272-A? */

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(spectred)
		ROM_REGION(0x8000,"gfx1",0)
		ROM_LOAD ( "spm-c_ed_06-08-85.u6", 0x0000, 0x4000, CRC(a1b9ffe4) SHA1(3cb281f19f91372e24685792b7bff778944f99ed) )
		ROM_CONTINUE(0x0000, 0x4000)    // first half of this ROM is empty

		ROM_REGION(0x10000,"maincpu",0)
		// these ROMs appear to have been dumped weirdly, or are wired weirdly in the real hardware.
		// The first 0x2000 of u51 seems to be garbage
		// u50 seems to have the halves duplicated, and D000 and E000 swapped
		ROM_LOAD ( "spm-c_ed_51-09-86.u51.h", 0x0000, 0x4000, CRC(fae8d36c) SHA1(69bed61513482ccb578b89c2fb8e7ba2258e82a5))
		ROM_COPY( "maincpu", 0x2000, 0x0000, 0x1000 )
		ROM_LOAD ( "spm-c_ed_50-09-86.u50.h", 0x2000, 0x1000, CRC(1fccaf24) SHA1(1de1438ee8789f83cbc97f75c0485d1fd0f58a38))
		ROM_CONTINUE(0x1000, 0x1000)
		ROM_CONTINUE(0x4000, 0x2000)

		ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
		ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // copied from apple2e

		ROM_REGION(0x1000, "keyb_mcu", 0)
	ROM_LOAD( "167_8980.u5", 0x0000, 0x1000, CRC(a501f197) SHA1(136c2b562999a6e340fe0e9a3776cea8c2e3647e) )
ROM_END

// unlike the very unique TK2000, the TK3000 is a mostly stock enhanced IIe clone
ROM_START(tk3000)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "tk3000.f7",    0x000000, 0x002000, CRC(70157693) SHA1(a7922e2137f95271011042441d80466fba7bb828) )

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD( "tk3000.f4f6",  0x000000, 0x004000, CRC(5b1e8ab2) SHA1(f163e5753c18ff0e812a448e8da406f102600edf) )

	ROM_REGION(0x2000, "kbdcpu", 0)
	ROM_LOAD( "tk3000.e13",   0x000000, 0x002000, CRC(f9b860d3) SHA1(6a127f1458f43a00199d3dde94569b8928f05a53) )

	ROM_REGION(0x800, "keyboard", ROMREGION_ERASE00)
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // probably not this machine's actual ROM
ROM_END

ROM_START(prav8c)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "charrom.d20", 0x0000, 0x2000,CRC(935212cc) SHA1(934603a441c631bd841ea0d2ff39525474461e47))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD ( "prom_cd.d46", 0x0000, 0x2000, CRC(195d0b48) SHA1(f8c4f3722159081f6950207f03bc85da30980c08))
	ROM_LOAD ( "prom_ef.d41", 0x2000, 0x2000, CRC(ec6aa2f6) SHA1(64bce893ebf0e22cd8f22436b97ef1bfeddf692f))

	// contains slot firmware for slots 1, 2, and 6 (6 is the usual Disk II f/w)
	ROM_REGION(0x2000,"unknown",0)
	ROM_LOAD ( "eprom.d38", 0x0000, 0x2000, CRC(c8d00b19) SHA1(13d156957ea68d0e7bc4be57cb1580c8b1399981))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // we don't know what this machine used
ROM_END

ROM_START(apple2c0)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("3420033a.256", 0x0000, 0x8000, CRC(c8b979b3) SHA1(10767e96cc17bad0970afda3a4146564e6272ba1))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(apple2c3)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("342-0445-a.256", 0x0000, 0x8000, CRC(bc5a79ff) SHA1(5338d9baa7ae202457b6500fde5883dbdc86e5d3))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(apple2c4)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("3410445b.256", 0x0000, 0x8000, CRC(06f53328) SHA1(015061597c4cda7755aeb88b735994ffd2f235ca))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(laser128)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "laser 128 video rom vt27-0706-0.bin", 0x0800, 0x0800, CRC(7884cc0f) SHA1(693a0a66191465825b8f7b5e746b463f3000e9cc) )
	ROM_CONTINUE(0x0000, 0x0800)    // international character set (how is this selected?)
	ROM_CONTINUE(0x1000, 0x1000)    // lo-res patterns, twice

	ROM_REGION(0x10000,"maincpu",0)
	ROM_SYSTEM_BIOS(0, "871222", "v4.3")
	ROMX_LOAD( "laser 128 v4.3 871222.bin", 0x000000, 0x008000, CRC(e091af13) SHA1(3232f7036a68b996fd4126d5e19e855c4d5c64df), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "870917", "v4.2")
	ROMX_LOAD( "laser 128 v4.2 870917.bin", 0x000000, 0x008000, CRC(39e59ed3) SHA1(cbd2f45c923725bfd57f8548e65cc80b13bc18da), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "870724", "v4.1")
	ROMX_LOAD( "laser 128 v4.1 870724.bin", 0x000000, 0x008000, CRC(ce087911) SHA1(f6dba711f0d727f1d13b0256f10ba62bde6d7f5b), ROM_BIOS(2) )

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real laser rom
ROM_END

ROM_START(laser128o)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD("laser 128 video rom vt27-0706-0.bin", 0x0800, 0x0800, CRC(7884cc0f) SHA1(693a0a66191465825b8f7b5e746b463f3000e9cc))
	ROM_CONTINUE(0x0000, 0x0800) // international character set (how is this selected?)
	ROM_CONTINUE(0x1000, 0x1000) // lo-res patterns, twice

	ROM_REGION(0x10000,"maincpu",0)
	ROM_SYSTEM_BIOS(0, "871212", "v3.3")
	ROMX_LOAD( "laser 128 v3.3 871212.bin", 0x000000, 0x008000, CRC(3f5deffe) SHA1(4e7195b941c51ba83d5ef16e1f78e3f62bccd8cd), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "870320", "v3.0")
	ROMX_LOAD( "laser 128 v3.0 870320.bin", 0x000000, 0x008000, CRC(145d39ff) SHA1(087e992548c2e9849d4262a0eb505548f846c7f5), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "870203", "v2.9")
	ROMX_LOAD( "laser 128 v2.9 870203.bin", 0x000000, 0x008000, CRC(7e12fe93) SHA1(d7be7ba05725111354e4bdbaaef620a2a8ea65f7), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(3, "860915", "860915")
	ROMX_LOAD( "laser 128 860915.bin", 0x000000, 0x008000, CRC(8d1a181d) SHA1(8fae95776c3d8a581c621cd258e118b8dfdb0cfd), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS(4, "860801", "860801")
	ROMX_LOAD( "laser 128 860801.bin", 0x000000, 0x008000, CRC(a88c2fcf) SHA1(ec163bb6e7e07cb256e0ed0f8d148cf85313e9f9), ROM_BIOS(4) )

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real laser rom
ROM_END

ROM_START(las128ex)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD("laser 128 video rom vt27-0706-0.bin", 0x0800, 0x0800, CRC(7884cc0f) SHA1(693a0a66191465825b8f7b5e746b463f3000e9cc))
	ROM_CONTINUE(0x0000, 0x0800) // international character set (how is this selected?)
	ROM_CONTINUE(0x1000, 0x1000) // lo-res patterns, twice

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("las128ex.256", 0x0000, 0x8000, CRC(b67c8ba1) SHA1(8bd5f82a501b1cf9d988c7207da81e514ca254b0))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real laser rom
ROM_END

ROM_START(las128e2)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD("laser 128 video rom vt27-0706-0.bin", 0x0800, 0x0800, CRC(7884cc0f) SHA1(693a0a66191465825b8f7b5e746b463f3000e9cc))
	ROM_CONTINUE(0x0000, 0x0800) // international character set (how is this selected?)
	ROM_CONTINUE(0x1000, 0x1000) // lo-res patterns, twice

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD( "laser 128ex2 rom version 6.1.bin", 0x000000, 0x008000, CRC(7f911c90) SHA1(125754c1bd777d4c510f5239b96178c6f2e3236b) )

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real laser rom
ROM_END

ROM_START(apple2cp)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("341-0625-a.256", 0x0000, 0x8000, CRC(0b996420) SHA1(1a27ae26966bbafd825d08ad1a24742d3e33557c))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(ceci)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x000000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x001000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )

	ROM_REGION(0x10000,"maincpu",0)
	ROM_SYSTEM_BIOS(0, "default", "ver 1.21")
	ROMX_LOAD( "u7.alt", 0x000000, 0x008000, CRC(23c87b3b) SHA1(ff9673f628390e9b0fb60732acc72b580200b5ae), ROM_BIOS(0) )
	ROMX_LOAD( "u35.alt", 0x008000, 0x008000, CRC(a4f40181) SHA1(db1ed69b2ed3280b1c90f76dbd637370d5bc11b0), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "older", "ver 1.1")
	ROMX_LOAD( "u7.tmm24256ap.bin", 0x000000, 0x008000, CRC(2e3f73b0) SHA1(a2967b3a9a040a1c47eb0fea9a632b833cd1c060), ROM_BIOS(1) )
	ROMX_LOAD( "u35.tmm24256ap.bin", 0x008000, 0x008000, CRC(7f3ee968) SHA1(f4fd7ceda4c9ab9bc626d6abb76b7fd2b5faf2da), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "diag", "self test")
	ROMX_LOAD( "u7.selftest.bin", 0x000000, 0x008000, CRC(4b045a44) SHA1(d2c716d0eb9f1c70e108d16c6a88d44b894e39fd), ROM_BIOS(2) )
	ROMX_LOAD( "u35.tmm24256ap.bin", 0x008000, 0x008000, CRC(7f3ee968) SHA1(f4fd7ceda4c9ab9bc626d6abb76b7fd2b5faf2da), ROM_BIOS(2) )

	ROM_REGION(0x40000,"cecexp",0)
	ROM_LOAD16_BYTE( "u33.mx231024-0059.bin", 0x000000, 0x020000, CRC(a2a59f35) SHA1(01c787e150bf1378088a9333ec9338387aae0f50) )
	ROM_LOAD16_BYTE( "u34.mx231024-0060.bin", 0x000001, 0x020000, CRC(f23470ce) SHA1(dc4cbe19e202d2afb56998ff04255b3171b58e14) )

	ROM_REGION(0x800,"keyboard",0)
	ROM_LOAD( "u26.9433c-0201.rcl-zh-16.bin", 0x000000, 0x000800, CRC(f3190603) SHA1(7efdf6f4ee0ed01ff06341c601496a43d06afd6b) )

	ROM_REGION(0x100,"slot6",0)
	ROM_LOAD( "u40.m2822.bin", 0x000000, 0x000100, CRC(b72a2c70) SHA1(bc39fbd5b9a8d2287ac5d0a42e639fc4d3c2f9d4) )
ROM_END

ROM_START(cece)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x000000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x001000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )

	ROM_REGION(0x10000,"maincpu",0)
	//ROM_SYSTEM_BIOS(0, "default", "ver 1.0")
	//ROMX_LOAD( "u20.rom10.27256.bin", 0x000000, 0x008000, CRC(51b92e5d) SHA1(654588f15910a04934d3579b12c14cc44f8ffd47), ROM_BIOS(0) )
	//ROMX_LOAD( "u14.rom20.27256.bin", 0x008000, 0x008000, CRC(1bbc7f3a) SHA1(32d8ac440a59dc0d2096eadfca057c27c7ad9cc1), ROM_BIOS(0) )
	ROM_LOAD( "u20.rom10.27256.bin", 0x000000, 0x008000, CRC(51b92e5d) SHA1(654588f15910a04934d3579b12c14cc44f8ffd47) )
	ROM_LOAD( "u14.rom20.27256.bin", 0x008000, 0x008000, CRC(1bbc7f3a) SHA1(32d8ac440a59dc0d2096eadfca057c27c7ad9cc1) )

	ROM_REGION(0x40000,"cecexp",0)
	ROM_LOAD16_BYTE( "u4.c3001.531000.bin", 0x000000, 0x020000, CRC(d255ea01) SHA1(6ee445c0e7938e2f5de796daac369eb297915129) )
	ROM_LOAD16_BYTE( "u7.c3002.531000.bin", 0x000001, 0x020000, CRC(31963b3b) SHA1(a255f7d0756ae67510f0bf325dc88371906df59d) )

	ROM_REGION(0x800,"keyboard",0)
	ROM_LOAD( "u26.9433c-0201.rcl-zh-16.bin", 0x000000, 0x000800, CRC(f3190603) SHA1(7efdf6f4ee0ed01ff06341c601496a43d06afd6b) )

	ROM_REGION(0x100,"slot6",0)
	ROM_LOAD( "u40.m2822.bin", 0x000000, 0x000100, CRC(b72a2c70) SHA1(bc39fbd5b9a8d2287ac5d0a42e639fc4d3c2f9d4) )
ROM_END

ROM_START(cecg)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x000000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x001000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )

	ROM_REGION(0x10000,"maincpu",0)
	// ROM_SYSTEM_BIOS(0, "default", "ver 1.0")
	// ROMX_LOAD( "u28.rom1.27256.bin", 0x000000, 0x008000, CRC(ea0f14e1) SHA1(8d184aaa3bfa0cb5f6635f0bd063287356108f5b), ROM_BIOS(0) )
	// ROMX_LOAD( "u29.rom2.27256.bin", 0x008000, 0x008000, CRC(ae1cae24) SHA1(3cba4ffed1a34e7a50bed6cd7ba1befff150fd18), ROM_BIOS(0) )
	ROM_LOAD( "u28.rom1.27256.bin", 0x000000, 0x008000, CRC(ea0f14e1) SHA1(8d184aaa3bfa0cb5f6635f0bd063287356108f5b) )
	ROM_LOAD( "u29.rom2.27256.bin", 0x008000, 0x008000, CRC(ae1cae24) SHA1(3cba4ffed1a34e7a50bed6cd7ba1befff150fd18) )

	ROM_REGION(0x40000,"cecexp",0)
	ROM_LOAD16_BYTE( "u4.c3001.531000.bin", 0x000000, 0x020000, CRC(d255ea01) SHA1(6ee445c0e7938e2f5de796daac369eb297915129) )
	ROM_LOAD16_BYTE( "u7.c3002.531000.bin", 0x000001, 0x020000, CRC(31963b3b) SHA1(a255f7d0756ae67510f0bf325dc88371906df59d) )

	ROM_REGION(0x800,"keyboard",0)
	ROM_LOAD( "u26.9433c-0201.rcl-zh-16.bin", 0x000000, 0x000800, CRC(f3190603) SHA1(7efdf6f4ee0ed01ff06341c601496a43d06afd6b) )

	ROM_REGION(0x100,"slot6",0)
	ROM_LOAD( "u40.m2822.bin", 0x000000, 0x000100, CRC(b72a2c70) SHA1(bc39fbd5b9a8d2287ac5d0a42e639fc4d3c2f9d4) )
ROM_END

ROM_START(cecm)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x000000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x001000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )

	ROM_REGION(0x10000,"maincpu",0)
	//ROM_SYSTEM_BIOS(0, "default", "ver 1.0")
	// ROMX_LOAD( "u8_st_27256fi_042f.bin", 0x000000, 0x008000, CRC(7a7ed6f0) SHA1(77ad9185607d65f05f3379f01445bb3fa33dbade), ROM_BIOS(0) )
	// ROMX_LOAD( "u35_st_27256fi_a53a.bin", 0x008000, 0x008000, CRC(0ffd658b) SHA1(72fadc11a0f391973e784a2e6f8ea5156d3305eb), ROM_BIOS(0) )
	ROM_LOAD( "u8_st_27256fi_042f.bin", 0x000000, 0x008000, CRC(7a7ed6f0) SHA1(77ad9185607d65f05f3379f01445bb3fa33dbade) )
	ROM_LOAD( "u35_st_27256fi_a53a.bin", 0x008000, 0x008000, CRC(0ffd658b) SHA1(72fadc11a0f391973e784a2e6f8ea5156d3305eb) )

	ROM_REGION(0x40000,"cecexp",0)
	ROM_LOAD16_BYTE( "u33.mx231024-0059.bin", 0x000000, 0x020000, CRC(a2a59f35) SHA1(01c787e150bf1378088a9333ec9338387aae0f50) )
	ROM_LOAD16_BYTE( "u34.mx231024-0060.bin", 0x000001, 0x020000, CRC(f23470ce) SHA1(dc4cbe19e202d2afb56998ff04255b3171b58e14) )

	ROM_REGION(0x800,"keyboard",0)
	ROM_LOAD( "u26.9433c-0201.rcl-zh-16.bin", 0x000000, 0x000800, CRC(f3190603) SHA1(7efdf6f4ee0ed01ff06341c601496a43d06afd6b) )
	// ROM_LOAD( "u10_2732_0081.bin", 0x000000, 0x00800, CRC(505eed67) SHA1(4acbee7c957528d1f9fbfd54464f85fb493175d7) )
ROM_END

ROM_START(cec2000)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x000000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x001000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )

	ROM_REGION(0x10000,"maincpu",0)
	//ROM_SYSTEM_BIOS(0, "default", "ver 3.0")
	//ROMX_LOAD( "u34.rom1.27256.bin", 0x000000, 0x008000, CRC(d8e001a5) SHA1(55febac557de02afc6186ffa5dd2dcc33b58b0ef), ROM_BIOS(0) )
	//ROMX_LOAD( "u41.rom2.27256.bin", 0x008000, 0x008000, CRC(6d7074ce) SHA1(d271a204a782f383783376f536fcc94a06bd81f6), ROM_BIOS(0) )
	ROM_LOAD( "u34.rom1.27256.bin", 0x000000, 0x008000, CRC(d8e001a5) SHA1(55febac557de02afc6186ffa5dd2dcc33b58b0ef) )
	ROM_LOAD( "u41.rom2.27256.bin", 0x008000, 0x008000, CRC(6d7074ce) SHA1(d271a204a782f383783376f536fcc94a06bd81f6) )

	ROM_REGION(0x40000,"cecexp",0)
	ROM_LOAD16_BYTE( "u4.c3001.531000.bin", 0x000000, 0x020000, CRC(d255ea01) SHA1(6ee445c0e7938e2f5de796daac369eb297915129) )
	ROM_LOAD16_BYTE( "u7.c3002.531000.bin", 0x000001, 0x020000, CRC(31963b3b) SHA1(a255f7d0756ae67510f0bf325dc88371906df59d) )

	ROM_REGION(0x800,"keyboard",0)
	ROM_LOAD( "u26.9433c-0201.rcl-zh-16.bin", 0x000000, 0x000800, CRC(f3190603) SHA1(7efdf6f4ee0ed01ff06341c601496a43d06afd6b) )

	ROM_REGION(0x100,"slot6",0)
	ROM_LOAD( "u40.m2822.bin", 0x000000, 0x000100, CRC(b72a2c70) SHA1(bc39fbd5b9a8d2287ac5d0a42e639fc4d3c2f9d4) )
ROM_END

ROM_START(zijini)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x000000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )
	ROM_LOAD( "u13.9433c-0202.rcl-zh-32.bin", 0x001000, 0x001000, CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be) )

	ROM_REGION(0x10000,"maincpu",0)
	//ROM_SYSTEM_BIOS(0, "default", "ver 1.0")
	//ROMX_LOAD( "u20.rom10.27256.bin", 0x000000, 0x008000, CRC(51b92e5d) SHA1(654588f15910a04934d3579b12c14cc44f8ffd47), ROM_BIOS(0) )
	//ROMX_LOAD( "u14.rom20.27256.bin", 0x008000, 0x008000, CRC(1bbc7f3a) SHA1(32d8ac440a59dc0d2096eadfca057c27c7ad9cc1), ROM_BIOS(0) )
	ROM_LOAD( "u20-m27256-3c91.bin", 0x000000, 0x008000, CRC(dc95ebba) SHA1(360d8f4be3ea94eb4cc37f8c489f22c1436f4a51) )
	ROM_LOAD( "u21-m27256-4e04.bin", 0x008000, 0x008000, CRC(0d7011bd) SHA1(322ed9805a76c0066170edc420419c7ef65ead8e) )

	ROM_REGION(0x40000,"cecexp",0)
	ROM_LOAD16_BYTE( "u4.c3001.531000.bin", 0x000000, 0x020000, CRC(d255ea01) SHA1(6ee445c0e7938e2f5de796daac369eb297915129) )
	ROM_LOAD16_BYTE( "u7.c3002.531000.bin", 0x000001, 0x020000, CRC(31963b3b) SHA1(a255f7d0756ae67510f0bf325dc88371906df59d) )

	ROM_REGION(0x800,"keyboard",0)
	ROM_LOAD( "u26.9433c-0201.rcl-zh-16.bin", 0x000000, 0x000800, CRC(f3190603) SHA1(7efdf6f4ee0ed01ff06341c601496a43d06afd6b) )

	ROM_REGION(0x100,"slot6",0)
	ROM_LOAD( "u40.m2822.bin", 0x000000, 0x000100, CRC(b72a2c70) SHA1(bc39fbd5b9a8d2287ac5d0a42e639fc4d3c2f9d4) )
ROM_END

ROM_START(ace2200)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD("franklin_ace2000_videorom.bin", 0x1000, 0x1000, CRC(545bdeea) SHA1(26ebc4b0d3080311f550090bc1b29807cb22d083))
	ROM_CONTINUE(0x0000, 0x1000)

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("franklin_ace2000_rom_u2_p2_rev6.bin", 0x000000, 0x002000, CRC(1ab6e4d3) SHA1(5bdb10089fbdadaee9772afa1f7439a51289636b))
	ROM_LOAD("franklin_ace2000_rom_u3_p1_rev6.bin", 0x002000, 0x002000, CRC(197f4936) SHA1(ec9de6da0ca6b6fd97fbef34eec64bf5c3c1b6c5))
	ROM_LOAD("franklin_ace2000_rom_u1_p3_rev6_franklinrom.bin", 0x004000, 0x002000, CRC(5cc150a7) SHA1(7ac8028bbf8cb7730f432e0bae32e364523555fb))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(ace500)
	ROM_REGION(0x2000,"gfx1",0)
	// not sure if the 500 and 2000 have the same character set.  First 4K is normal charset, second 4K is MouseText
	ROM_LOAD("franklin_ace2000_videorom.bin", 0x1000, 0x1000, CRC(545bdeea) SHA1(26ebc4b0d3080311f550090bc1b29807cb22d083))
	ROM_CONTINUE(0x0000, 0x1000)

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("franklin500-rom.bin", 0x004000, 0x004000, CRC(376c9104) SHA1(7f82706adc0bfb5f60c207c81271eb0ba8510a11))
	ROM_CONTINUE(0x0000, 0x4000)

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

} // anonymous namespace


/*    YEAR  NAME        PARENT   COMPAT  MACHINE      INPUT      CLASS          INIT        COMPANY             FULLNAME */
COMP( 1983, apple2e,    0,       apple2, apple2e,     apple2e,   apple2e_state, empty_init, "Apple Computer",   "Apple //e", MACHINE_SUPPORTS_SAVE )
COMP( 1983, apple2euk,  apple2e, 0,      apple2epal,  apple2euk, apple2e_state, init_pal,   "Apple Computer",   "Apple //e (UK)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, apple2ees,  apple2e, 0,      apple2epal,  apple2ees, apple2e_state, init_pal,   "Apple Computer",   "Apple //e (Spain)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, mprof3,     apple2e, 0,      mprof3,      apple2e,   apple2e_state, empty_init, "Multitech",        "Microprofessor III", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1985, apple2ee,   apple2e, 0,      apple2ee,    apple2e,   apple2e_state, empty_init, "Apple Computer",   "Apple //e (enhanced)", MACHINE_SUPPORTS_SAVE )
COMP( 1985, apple2eeuk, apple2e, 0,      apple2eepal, apple2euk, apple2e_state, init_pal,   "Apple Computer",   "Apple //e (enhanced, UK)", MACHINE_SUPPORTS_SAVE )
COMP( 1985, apple2eefr, apple2e, 0,      apple2eepal, apple2efr, apple2e_state, init_pal,   "Apple Computer",   "Apple //e (enhanced, France)", MACHINE_SUPPORTS_SAVE )
COMP( 1987, apple2ep,   apple2e, 0,      apple2ep,    apple2ep,  apple2e_state, empty_init, "Apple Computer",   "Apple //e (Platinum)", MACHINE_SUPPORTS_SAVE )
COMP( 1984, apple2c,    0,       apple2, apple2c,     apple2c,   apple2e_state, empty_init, "Apple Computer",   "Apple //c" , MACHINE_SUPPORTS_SAVE )
COMP( 1985?,spectred,   apple2e, 0,      spectred,    apple2e,   apple2e_state, empty_init, "Scopus/Spectrum",  "Spectrum ED" , MACHINE_SUPPORTS_SAVE )
COMP( 1986, tk3000,     apple2c, 0,      tk3000,      apple2e,   apple2e_state, empty_init, "Microdigital",     "TK3000//e" , MACHINE_SUPPORTS_SAVE )
COMP( 1989, prav8c,     apple2e, 0,      apple2e,     apple2e,   apple2e_state, empty_init, "Pravetz",          "Pravetz 8C", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1987, laser128,   apple2c, 0,      laser128,    laser128,  apple2e_state, init_laser128, "Video Technology", "Laser 128", MACHINE_SUPPORTS_SAVE )
COMP( 1987, laser128o,  apple2c, 0,      laser128o,   laser128,  apple2e_state, init_laser128, "Video Technology", "Laser 128 (original hardware)", MACHINE_SUPPORTS_SAVE )
COMP( 1988, las128ex,   apple2c, 0,      laser128,    laser128,  apple2e_state, init_128ex, "Video Technology", "Laser 128ex (version 4.5)", MACHINE_SUPPORTS_SAVE )
COMP( 1988, las128e2,   apple2c, 0,      laser128ex2, laser128,  apple2e_state, init_128ex, "Video Technology", "Laser 128ex2 (version 6.1)", MACHINE_SUPPORTS_SAVE )
COMP( 1985, apple2c0,   apple2c, 0,      apple2c_iwm, apple2c,   apple2e_state, empty_init, "Apple Computer",   "Apple //c (UniDisk 3.5)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2c3,   apple2c, 0,      apple2c_mem, apple2c,   apple2e_state, empty_init, "Apple Computer",   "Apple //c (Original Memory Expansion)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2c4,   apple2c, 0,      apple2c_mem, apple2c,   apple2e_state, empty_init, "Apple Computer",   "Apple //c (rev 4)", MACHINE_SUPPORTS_SAVE )
COMP( 1987, ceci,       0,       apple2, cec,         ceci,      apple2e_state, empty_init, "Shaanxi Province Computer Factory", "China Education Computer I", MACHINE_SUPPORTS_SAVE )
COMP( 1989, cece,       0,       apple2, cec,         ceci,      apple2e_state, empty_init, "Shaanxi Province Computer Factory", "China Education Computer E", MACHINE_SUPPORTS_SAVE )
COMP( 1989, cecg,       0,       apple2, cec,         ceci,      apple2e_state, empty_init, "Shaanxi Province Computer Factory", "China Education Computer G", MACHINE_SUPPORTS_SAVE )
COMP( 1989, cecm,       0,       apple2, cec,         cecm,      apple2e_state, empty_init, "Shaanxi Province Computer Factory", "China Education Computer M", MACHINE_SUPPORTS_SAVE )
COMP( 1991, cec2000,    0,       apple2, cec,         ceci,      apple2e_state, empty_init, "Shaanxi Province Computer Factory", "China Education Computer 2000", MACHINE_SUPPORTS_SAVE )
COMP( 1989, zijini,     0,       apple2, cec,         zijini,    apple2e_state, empty_init, "Nanjing Computer Factory", "Zi Jin I", MACHINE_SUPPORTS_SAVE )
COMP( 1988, apple2cp,   apple2c, 0,      apple2cp,    apple2cp,  apple2e_state, empty_init, "Apple Computer",   "Apple //c Plus", MACHINE_SUPPORTS_SAVE )
COMP( 1985, ace2200,    apple2e, 0,      ace2200,     ace500,    apple2e_state, init_ace2200,"Franklin Computer", "Franklin ACE 2200", MACHINE_SUPPORTS_SAVE)
COMP( 1986, ace500,     apple2c, 0,      ace500,      ace500,    apple2e_state, init_ace500,"Franklin Computer", "Franklin ACE 500", MACHINE_SUPPORTS_SAVE)
