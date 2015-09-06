// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    apple2e.c - Apple IIe/IIc/IIc Plus and clones

    Next generation driver written in September/October 2014 by R. Belmont.
    Thanks to the original Apple II series driver's authors: Mike Balfour, Nathan Woods, and R. Belmont
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

        The machine has an internal "Apple 3.5" drive plus a custom gate array
        which emulates the functionality of the UniDisk 3.5's on-board 65C02.
        This gets around the fact that 1 MHz isn't sufficient to handle direct
        Woz-style control of a double-density 3.5" drive.

        External drive port allows IIgs-style daisy-chaining.

----------------------------------

TK3000 keyboard matrix

Data bus D0-D7 is X0-X7
Address bus A0-A11 is Y0-Y11

***************************************************************************/

#include "emu.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/kb3600.h"
#include "sound/speaker.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "formats/ap2_dsk.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/z80/z80.h"
#include "formats/ap_dsk35.h"
#include "machine/sonydriv.h"
#include "machine/appldriv.h"
#include "bus/rs232/rs232.h"
#include "machine/mos6551.h"
#include "video/apple2.h"

#include "bus/a2bus/a2bus.h"
#include "bus/a2bus/a2lang.h"
#include "bus/a2bus/a2diskii.h"
#include "bus/a2bus/a2diskiing.h"
#include "bus/a2bus/a2mockingboard.h"
#include "bus/a2bus/a2cffa.h"
#include "bus/a2bus/a2memexp.h"
#include "bus/a2bus/a2scsi.h"
#include "bus/a2bus/a2thunderclock.h"
#include "bus/a2bus/a2softcard.h"
#include "bus/a2bus/a2videoterm.h"
#include "bus/a2bus/a2ssc.h"
#include "bus/a2bus/a2swyft.h"
#include "bus/a2bus/a2themill.h"
#include "bus/a2bus/a2sam.h"
#include "bus/a2bus/a2alfam2.h"
#include "bus/a2bus/laser128.h"
#include "bus/a2bus/a2echoii.h"
#include "bus/a2bus/a2arcadebd.h"
#include "bus/a2bus/a2midi.h"
#include "bus/a2bus/a2zipdrive.h"
#include "bus/a2bus/a2applicard.h"
#include "bus/a2bus/a2ultraterm.h"
#include "bus/a2bus/a2pic.h"
#include "bus/a2bus/a2corvus.h"
#include "bus/a2bus/a2mcms.h"
#include "bus/a2bus/a2dx1.h"
#include "bus/a2bus/timemasterho.h"
#include "bus/a2bus/mouse.h"
#include "bus/a2bus/ezcgi.h"
#include "bus/a2bus/a2eauxslot.h"
#include "bus/a2bus/a2estd80col.h"
#include "bus/a2bus/a2eext80col.h"
#include "bus/a2bus/a2eramworks3.h"

#define A2_CPU_TAG "maincpu"
#define A2_KBDC_TAG "ay3600"
#define A2_BUS_TAG "a2bus"
#define A2_SPEAKER_TAG "speaker"
#define A2_CASSETTE_TAG "tape"
#define A2_UPPERBANK_TAG "inhbank"
#define IIC_ACIA1_TAG "acia1"
#define IIC_ACIA2_TAG "acia2"
#define IICP_IWM_TAG    "iwm"
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

#define IRQ_SLOT 0
#define IRQ_VBL  1
#define IRQ_MOUSEXY 2

class apple2e_state : public driver_device
{
public:
	apple2e_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, A2_CPU_TAG),
		m_ram(*this, RAM_TAG),
		m_rom(*this, "maincpu"),
		m_ay3600(*this, A2_KBDC_TAG),
		m_video(*this, A2_VIDEO_TAG),
		m_a2bus(*this, A2_BUS_TAG),
		m_a2eauxslot(*this, A2_AUXSLOT_TAG),
		m_joy1x(*this, "joystick_1_x"),
		m_joy1y(*this, "joystick_1_y"),
		m_joy2x(*this, "joystick_2_x"),
		m_joy2y(*this, "joystick_2_y"),
		m_joybuttons(*this, "joystick_buttons"),
		m_mouseb(*this, MOUSE_BUTTON_TAG),
		m_mousex(*this, MOUSE_XAXIS_TAG),
		m_mousey(*this, MOUSE_YAXIS_TAG),
		m_kbdrom(*this, "keyboard"),
		m_kbspecial(*this, "keyb_special"),
		m_sysconfig(*this, "a2_config"),
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
		m_laserudc(*this, LASER128_UDC_TAG),
		m_iicpiwm(*this, IICP_IWM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_device<ay3600_device> m_ay3600;
	required_device<a2_video_device> m_video;
	required_device<a2bus_device> m_a2bus;
	optional_device<a2eauxslot_device> m_a2eauxslot;
	required_ioport m_joy1x, m_joy1y, m_joy2x, m_joy2y, m_joybuttons;
	optional_ioport m_mouseb, m_mousex, m_mousey;
	optional_memory_region m_kbdrom;
	required_ioport m_kbspecial;
	required_ioport m_sysconfig;
	required_device<speaker_sound_device> m_speaker;
	optional_device<cassette_image_device> m_cassette;
	required_device<address_map_bank_device> m_upperbank;
	required_device<address_map_bank_device> m_0000bank;
	required_device<address_map_bank_device> m_0200bank;
	required_device<address_map_bank_device> m_0400bank;
	required_device<address_map_bank_device> m_0800bank;
	required_device<address_map_bank_device> m_2000bank;
	required_device<address_map_bank_device> m_4000bank;
	required_device<address_map_bank_device> m_c100bank;
	required_device<address_map_bank_device> m_c300bank;
	required_device<address_map_bank_device> m_c400bank;
	required_device<address_map_bank_device> m_c800bank;
	required_device<address_map_bank_device> m_lcbank;
	optional_device<mos6551_device> m_acia1, m_acia2;
	optional_device<applefdc_base_device> m_laserudc;
	optional_device<iwm_device> m_iicpiwm;

	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(ay3600_repeat);

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_PALETTE_INIT(apple2);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(ram0000_r);
	DECLARE_WRITE8_MEMBER(ram0000_w);
	DECLARE_READ8_MEMBER(ram0200_r);
	DECLARE_WRITE8_MEMBER(ram0200_w);
	DECLARE_READ8_MEMBER(ram0400_r);
	DECLARE_WRITE8_MEMBER(ram0400_w);
	DECLARE_READ8_MEMBER(ram0800_r);
	DECLARE_WRITE8_MEMBER(ram0800_w);
	DECLARE_READ8_MEMBER(ram2000_r);
	DECLARE_WRITE8_MEMBER(ram2000_w);
	DECLARE_READ8_MEMBER(ram4000_r);
	DECLARE_WRITE8_MEMBER(ram4000_w);
	DECLARE_READ8_MEMBER(auxram0000_r);
	DECLARE_WRITE8_MEMBER(auxram0000_w);
	DECLARE_READ8_MEMBER(auxram0200_r);
	DECLARE_WRITE8_MEMBER(auxram0200_w);
	DECLARE_READ8_MEMBER(auxram0400_r);
	DECLARE_WRITE8_MEMBER(auxram0400_w);
	DECLARE_READ8_MEMBER(auxram0800_r);
	DECLARE_WRITE8_MEMBER(auxram0800_w);
	DECLARE_READ8_MEMBER(auxram2000_r);
	DECLARE_WRITE8_MEMBER(auxram2000_w);
	DECLARE_READ8_MEMBER(auxram4000_r);
	DECLARE_WRITE8_MEMBER(auxram4000_w);
	DECLARE_READ8_MEMBER(c000_r);
	DECLARE_WRITE8_MEMBER(c000_w);
	DECLARE_READ8_MEMBER(c000_iic_r);
	DECLARE_WRITE8_MEMBER(c000_iic_w);
	DECLARE_READ8_MEMBER(c080_r);
	DECLARE_WRITE8_MEMBER(c080_w);
	DECLARE_READ8_MEMBER(c100_r);
	DECLARE_READ8_MEMBER(c100_int_r);
	DECLARE_READ8_MEMBER(c100_int_bank_r);
	DECLARE_WRITE8_MEMBER(c100_w);
	DECLARE_READ8_MEMBER(c300_r);
	DECLARE_READ8_MEMBER(c300_int_r);
	DECLARE_READ8_MEMBER(c300_int_bank_r);
	DECLARE_WRITE8_MEMBER(c300_w);
	DECLARE_READ8_MEMBER(c400_r);
	DECLARE_READ8_MEMBER(c400_int_r);
	DECLARE_READ8_MEMBER(c400_int_bank_r);
	DECLARE_WRITE8_MEMBER(c400_w);
	DECLARE_READ8_MEMBER(c800_r);
	DECLARE_WRITE8_MEMBER(c800_w);
	DECLARE_READ8_MEMBER(inh_r);
	DECLARE_WRITE8_MEMBER(inh_w);
	DECLARE_READ8_MEMBER(lc_r);
	DECLARE_WRITE8_MEMBER(lc_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_irq_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_inh_w);
	DECLARE_READ_LINE_MEMBER(ay3600_shift_r);
	DECLARE_READ_LINE_MEMBER(ay3600_control_r);
	DECLARE_WRITE_LINE_MEMBER(ay3600_data_ready_w);
	DECLARE_WRITE_LINE_MEMBER(ay3600_ako_w);
	DECLARE_READ8_MEMBER(memexp_r);
	DECLARE_WRITE8_MEMBER(memexp_w);

private:
	int m_speaker_state;
	int m_cassette_state, m_cassette_out;

	double m_joystick_x1_time;
	double m_joystick_y1_time;
	double m_joystick_x2_time;
	double m_joystick_y2_time;

	UINT16 m_lastchar, m_strobe;
	UINT8 m_transchar;
	bool m_anykeydown;
	int m_repeatdelay;

	int m_inh_slot;
	int m_cnxx_slot;

	bool m_page2;
	bool m_an0, m_an1, m_an2, m_an3;

	bool m_vbl, m_vblmask;

	bool m_xy, m_x0edge, m_y0edge;
	bool m_x0, m_x1, m_y0, m_y1;
	bool m_xirq, m_yirq;
	int last_mx, last_my, count_x, count_y;

	bool m_intcxrom;
	bool m_80store;
	bool m_slotc3rom;
	bool m_altzp;
	bool m_ramrd, m_ramwrt;
	bool m_lcram, m_lcram2, m_lcwriteenable;
	bool m_ioudis;
	bool m_romswitch;

	bool m_isiic, m_isiicplus;
	UINT8 m_iicplus_ce00[0x200];

	UINT8 *m_ram_ptr, *m_rom_ptr;
	int m_ram_size;

	UINT8 *m_aux_ptr, *m_aux_bank_ptr;

	int m_inh_bank;

	double m_x_calibration, m_y_calibration;

	device_a2bus_card_interface *m_slotdevice[8];
	device_a2eauxslot_card_interface *m_auxslotdevice;

	int m_irqmask;

	UINT8 m_exp_bankhior;
	int m_exp_addrmask;
	UINT8 m_exp_regs[0x10];
	UINT8 *m_exp_ram;
	int m_exp_wptr, m_exp_liveptr;

	void do_io(address_space &space, int offset, bool is_iic);
	UINT8 read_floatingbus();
	void update_slotrom_banks();
	void lc_update(int offset);
	UINT8 read_slot_rom(address_space &space, int slotbias, int offset);
	void write_slot_rom(address_space &space, int slotbias, int offset, UINT8 data);
	UINT8 read_int_rom(address_space &space, int slotbias, int offset);
	void auxbank_update();
	void raise_irq(int irq);
	void lower_irq(int irq);
	void update_iic_mouse();
};

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define JOYSTICK_DELTA          80
#define JOYSTICK_SENSITIVITY    50
#define JOYSTICK_AUTOCENTER     80

WRITE_LINE_MEMBER(apple2e_state::a2bus_irq_w)
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

WRITE_LINE_MEMBER(apple2e_state::a2bus_nmi_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

// TODO: this assumes /INH only on ROM, needs expansion to support e.g. phantom-slotting cards and etc.
WRITE_LINE_MEMBER(apple2e_state::a2bus_inh_w)
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
							m_upperbank->set_bank(1);
							m_inh_bank = 1;
						}
					}
					else
					{
						if (m_inh_bank != 0)
						{
							m_upperbank->set_bank(0);
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
			m_upperbank->set_bank(0);
			m_inh_bank = 0;
		}
	}
}

READ8_MEMBER(apple2e_state::memexp_r)
{
	UINT8 retval = m_exp_regs[offset];

	if (!m_exp_ram)
	{
		return 0xff;
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

WRITE8_MEMBER(apple2e_state::memexp_w)
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
	m_upperbank->set_bank(0);
	m_lcbank->set_bank(0);
	m_0000bank->set_bank(0);
	m_0200bank->set_bank(0);
	m_0400bank->set_bank(0);
	m_0800bank->set_bank(0);
	m_2000bank->set_bank(0);
	m_4000bank->set_bank(0);
	m_inh_bank = 0;

	// expansion RAM size
	if (m_ram_size > (128*1024))
	{
		m_exp_addrmask = m_ram_size - (128*1024) - 1;
		m_exp_ram = m_ram_ptr + (128*1024);
	}
	else    // no expansion
	{
		m_exp_addrmask = 0;
		m_exp_ram = NULL;
	}

	// precalculate joystick time constants
	m_x_calibration = attotime::from_usec(12).as_double();
	m_y_calibration = attotime::from_usec(13).as_double();

	// cache slot devices
	for (int i = 0; i <= 7; i++)
	{
		m_slotdevice[i] = m_a2bus->get_a2bus_card(i);
	}

	// and aux slot device if any
	m_aux_ptr = NULL;
	m_aux_bank_ptr = NULL;
	if (m_a2eauxslot)
	{
		m_auxslotdevice = m_a2eauxslot->get_a2eauxslot_card();
		if (m_auxslotdevice)
		{
			m_aux_ptr = m_auxslotdevice->get_vram_ptr();
			m_aux_bank_ptr = m_auxslotdevice->get_auxbank_ptr();
		}
	}
	else    // IIc has 128K right on the motherboard
	{
		m_auxslotdevice = NULL;

		if (m_ram_size >= (128*1024))
		{
			m_aux_ptr = &m_ram_ptr[0x10000];
			m_aux_bank_ptr = m_aux_ptr;
		}
	}

	// setup video pointers
	m_video->m_ram_ptr = m_ram_ptr;
	m_video->m_aux_ptr = m_aux_ptr;
	m_video->m_char_ptr = memregion("gfx1")->base();
	m_video->m_char_size = memregion("gfx1")->bytes();

	m_inh_slot = -1;
	m_cnxx_slot = CNXX_UNCLAIMED;

	// setup save states
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_cassette_state));
	save_item(NAME(m_joystick_x1_time));
	save_item(NAME(m_joystick_y1_time));
	save_item(NAME(m_joystick_x2_time));
	save_item(NAME(m_joystick_y2_time));
	save_item(NAME(m_lastchar));
	save_item(NAME(m_strobe));
	save_item(NAME(m_transchar));
	save_item(NAME(m_inh_slot));
	save_item(NAME(m_inh_bank));
	save_item(NAME(m_cnxx_slot));
	save_item(NAME(m_page2));
	save_item(NAME(m_an0));
	save_item(NAME(m_an1));
	save_item(NAME(m_an2));
	save_item(NAME(m_an3));
	save_item(NAME(m_intcxrom));
	save_item(NAME(m_80store));
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
	save_item(NAME(m_iicplus_ce00));
	save_item(NAME(m_exp_regs));
	save_item(NAME(m_exp_wptr));
	save_item(NAME(m_exp_liveptr));
	save_item(NAME(m_exp_bankhior));
	save_item(NAME(m_exp_addrmask));
	save_item(NAME(m_lcram));
	save_item(NAME(m_lcram2));
	save_item(NAME(m_lcwriteenable));
}

void apple2e_state::machine_reset()
{
	m_page2 = false;
	m_video->m_page2 = false;
	m_an0 = m_an1 = m_an2 = m_an3 = false;
	m_vbl = m_vblmask = false;
	m_slotc3rom = false;
	m_romswitch = false;
	m_irqmask = 0;
	m_anykeydown = false;
	m_repeatdelay = 10;
	m_xy = false;
	m_x0edge = false;
	m_y0edge = false;
	m_xirq = false;
	m_yirq = false;

	// IIe prefers INTCXROM default to off, IIc has it always on
	if (m_rom_ptr[0x3bc0] == 0x00)
	{
		m_intcxrom = true;
		m_slotc3rom = false;
		m_isiic = true;

		if (m_rom_ptr[0x3bbf] == 0x05)
		{
			m_isiicplus = true;
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

	m_80store = false;
	m_altzp = false;
	m_ramrd = false;
	m_ramwrt = false;
	m_ioudis = true;

	// LC default state: read ROM, write enabled, Dxxx bank 2
	m_lcram = false;
	m_lcram2 = true;
	m_lcwriteenable = true;

	m_exp_bankhior = 0xf0;

	// sync up the banking with the variables.
	// RESEARCH: how does RESET affect LC state and aux banking states?
	auxbank_update();
	update_slotrom_banks();
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

	if((scanline % 8) == 0)
	{
		machine().first_screen()->update_partial(machine().first_screen()->vpos());
	}

	if (m_isiic)
	{
		update_iic_mouse();
	}

	if (scanline == 192)
	{
		m_vbl = true;

		// update the video system's shadow copy of the system config
		m_video->m_sysconfig = m_sysconfig->read();

		if (m_vblmask)
		{
			raise_irq(IRQ_VBL);
		}

		// check for ctrl-reset
		if ((m_kbspecial->read() & 0x88) == 0x88)
		{
			m_maincpu->reset();
		}
	}
}

PALETTE_INIT_MEMBER(apple2e_state, apple2)
{
	m_video->palette_init_apple2(palette);
}

UINT32 apple2e_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool old_page2 = m_video->m_page2;

	// don't display page2 if 80store is set (we just saved the previous value, don't worry)
	if (m_80store)
	{
		m_video->m_page2 = false;
	}

	// always update the flash timer here so it's smooth regardless of mode switches
	m_video->m_flash = ((machine().time() * 4).seconds() & 1) ? true : false;

	if (m_video->m_graphics)
	{
		if (m_video->m_hires)
		{
			if (m_video->m_mix)
			{
				if ((m_video->m_dhires) && (m_video->m_80col))
				{
					m_video->dhgr_update(screen, bitmap, cliprect, 0, 159);
				}
				else
				{
					m_video->hgr_update(screen, bitmap, cliprect, 0, 159);
				}
				m_video->text_update(screen, bitmap, cliprect, 160, 191);
			}
			else
			{
				if ((m_video->m_dhires) && (m_video->m_80col))
				{
					m_video->dhgr_update(screen, bitmap, cliprect, 0, 191);
				}
				else
				{
					m_video->hgr_update(screen, bitmap, cliprect, 0, 191);
				}
			}
		}
		else    // lo-res
		{
			if (m_video->m_mix)
			{
				if ((m_video->m_dhires) && (m_video->m_80col))
				{
					m_video->dlores_update(screen, bitmap, cliprect, 0, 159);
				}
				else
				{
					m_video->lores_update(screen, bitmap, cliprect, 0, 159);
				}

				m_video->text_update(screen, bitmap, cliprect, 160, 191);
			}
			else
			{
				if ((m_video->m_dhires) && (m_video->m_80col))
				{
					m_video->dlores_update(screen, bitmap, cliprect, 0, 191);
				}
				else
				{
					m_video->lores_update(screen, bitmap, cliprect, 0, 191);
				}
			}
		}
	}
	else
	{
		m_video->text_update(screen, bitmap, cliprect, 0, 191);
	}

	m_video->m_page2 = old_page2;

	return 0;
}

/***************************************************************************
    I/O
***************************************************************************/
void apple2e_state::auxbank_update()
{
	int ramwr = (m_ramrd ? 1 : 0) | (m_ramwrt ? 2 : 0);

	m_0000bank->set_bank(m_altzp ? 1 : 0);
	m_0200bank->set_bank(ramwr);

	if (m_80store)
	{
		if (m_page2)
		{
			m_0400bank->set_bank(3);
		}
		else
		{
			m_0400bank->set_bank(0);
		}
	}
	else
	{
		m_0400bank->set_bank(ramwr);
	}

	m_0800bank->set_bank(ramwr);

	if ((m_80store) && (m_video->m_hires))
	{
		if (m_page2)
		{
			m_2000bank->set_bank(3);
		}
		else
		{
			m_2000bank->set_bank(0);
		}
	}
	else
	{
		m_2000bank->set_bank(ramwr);
	}

	m_4000bank->set_bank(ramwr);
}

void apple2e_state::update_slotrom_banks()
{
	int cxswitch = 0;

	// IIc and IIc+ have working (readable) INTCXROM/SLOTC3ROM switches, but
	// internal ROM is always present in the slots.
	if ((m_intcxrom) || (m_isiic))
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

	m_c100bank->set_bank(cxswitch);
	m_c400bank->set_bank(cxswitch);

//  printf("intcxrom %d cnxx_slot %d isiic %d romswitch %d\n", m_intcxrom, m_cnxx_slot, m_isiic, m_romswitch);
	if ((m_intcxrom) || (m_cnxx_slot < 0) || (m_isiic))
	{
		if (m_romswitch)
		{
			m_c800bank->set_bank(2);
		}
		else
		{
			m_c800bank->set_bank(1);
		}
	}
	else
	{
		m_c800bank->set_bank(0);
	}

	if ((!m_slotc3rom) || (m_isiic))
	{
		if (m_romswitch)
		{
			m_c300bank->set_bank(2);
		}
		else
		{
			m_c300bank->set_bank(1);
		}
	}
	else
	{
		m_c300bank->set_bank(0);
	}
}

void apple2e_state::lc_update(int offset)
{
	bool m_last_lcram = m_lcram;

	m_lcram = false;
	m_lcram2 = false;
	m_lcwriteenable = false;

	if (offset & 1)
	{
		m_lcwriteenable = true;
	}

	switch(offset & 0x03)
	{
		case 0x00:
		case 0x03:
			m_lcram = true;
			break;
	}

	if (!(offset & 8))
	{
		m_lcram2 = true;
	}

	if (m_lcram != m_last_lcram)
	{
		if (m_lcram)
		{
			m_lcbank->set_bank(1);
		}
		else
		{
			if (m_romswitch)
			{
				m_lcbank->set_bank(2);
			}
			else
			{
				m_lcbank->set_bank(0);
			}
		}
	}

	#if 0
	printf("LC: new state %c%c dxxx=%04x altzp=%d\n",
			m_lcram ? 'R' : 'x',
			m_lcwriteenable ? 'W' : 'x',
			m_lcram2 ? 0x1000 : 0x0000,
			m_altzp);
	#endif
}

// most softswitches don't care about read vs write, so handle them here
void apple2e_state::do_io(address_space &space, int offset, bool is_iic)
{
	if(space.debugger_access()) return;

	// Handle C058-C05F according to IOUDIS
	if ((offset & 0x58) == 0x58)
	{
		// IIc-specific switches
		if (m_isiic)
		{
			switch (offset)
			{
				case 0x58:  // DisXY
					m_xy = false; break;

				case 0x59:  // EnbXY
					m_xy = true; break;

				case 0x5a:  // DisVBL
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

		if (m_ioudis)
		{
			switch (offset)
			{
				case 0x5e:  // SETDHIRES
					m_video->m_dhires = true;
					break;

				case 0x5f:  // CLRDHIRES
					m_video->m_dhires = false;
					break;
			}
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

				// if LC is not enabled
				if (!m_lcram)
				{
					if (m_romswitch)
					{
						m_lcbank->set_bank(2);
					}
					else
					{
						m_lcbank->set_bank(0);
					}
				}
			}
			break;

		case 0x30:
			m_speaker_state ^= 1;
			m_speaker->level_w(m_speaker_state);
			break;

		case 0x48:  // (IIc only) clear mouse X/Y interrupt flags
			m_xirq = m_yirq = false;
			lower_irq(IRQ_MOUSEXY);
			break;

		case 0x50:  // graphics mode
			m_video->m_graphics = true; break;

		case 0x51:  // text mode
			m_video->m_graphics = false; break;

		case 0x52:  // no mix
			m_video->m_mix = false; break;

		case 0x53:  // mixed mode
			m_video->m_mix = true; break;

		case 0x54:  // set page 1
			m_page2 = false;
			m_video->m_page2 = false;
			auxbank_update();
			break;

		case 0x55:  // set page 2
			m_page2 = true;
			m_video->m_page2 = true;
			auxbank_update();
			break;

		case 0x56: // select lo-res
			m_video->m_hires = false;
			auxbank_update();
			break;

		case 0x57: // select hi-res
			m_video->m_hires = true;
			auxbank_update();
			break;

		case 0x58: // AN0 off
			m_an0 = false; break;

		case 0x59: // AN0 on
			m_an0 = true; break;

		case 0x5a: // AN1 off
			m_an1 = false; break;

		case 0x5b: // AN1 on
			m_an1 = true; break;

		case 0x5c: // AN2 off
			m_an2 = false; break;

		case 0x5d: // AN2 on
			m_an2 = true; break;

		case 0x5e: // AN3 off
			m_an3 = false; break;

		case 0x5f: // AN3 on
			m_an3 = true; break;

		case 0x68:  // IIgs STATE register, which ProDOS touches
			break;

		// trigger joypad read
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			if (is_iic)
			{
				m_vbl = false;
				lower_irq(IRQ_VBL);
			}

			m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_joy1x->read();
			m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_joy1y->read();
			m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_joy2x->read();
			m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_joy2y->read();
			break;

		default:
			logerror("do_io: unknown switch $C0%02X\n", offset);
			break;
	}
}

READ8_MEMBER(apple2e_state::c000_r)
{
	if(space.debugger_access()) return read_floatingbus();

	switch (offset)
	{
		case 0x00:  // keyboard latch
			return m_transchar | m_strobe;

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

		case 0x10:  // read any key down, reset keyboard strobe
			{
				UINT8 rv = m_transchar | (m_anykeydown ? 0x80 : 0x00);
				m_strobe = 0;
				return rv;
			}

		case 0x11:  // read LCRAM2 (LC Dxxx bank)
			return m_lcram2 ? 0x80 : 0x00;

		case 0x12:  // read LCRAM (is LC readable?)
			return m_lcram ? 0x80 : 0x00;

		case 0x13:  // read RAMRD
			return m_ramrd ? 0x80 : 0x00;

		case 0x14:  // read RAMWRT
			return m_ramwrt ? 0x80 : 0x00;

		case 0x15:  // read INTCXROM
			return m_intcxrom ? 0x80 : 0x00;

		case 0x16:  // read ALTZP
			return m_altzp ? 0x80 : 0x00;

		case 0x17:  // read SLOTC3ROM
			return m_slotc3rom ? 0x80 : 0x00;

		case 0x18:  // read 80STORE
			return m_80store ? 0x80 : 0x00;

		case 0x19:  // read VBLBAR
			return space.machine().first_screen()->vblank() ? 0x00 : 0x80;

		case 0x1a:  // read TEXT
			return m_video->m_graphics ? 0x00 : 0x80;

		case 0x1b:  // read MIXED
			return m_video->m_mix ? 0x80 : 0x00;

		case 0x1c:  // read PAGE2
			return m_page2 ? 0x80 : 0x00;

		case 0x1d:  // read HIRES
			return m_video->m_hires ? 0x80 : 0x00;

		case 0x1e:  // read ALTCHARSET
			return m_video->m_altcharset ? 0x80 : 0x00;

		case 0x1f:  // read 80COL
			return m_video->m_80col ? 0x80 : 0x00;

		case 0x60: // cassette in
		case 0x68:
			if (m_cassette)
			{
				return m_cassette->input() > 0.0 ? 0x80 : 0;
			}
			return 0;

		case 0x61:  // button 0 or Open Apple
		case 0x69:
			return ((m_joybuttons->read() & 0x10) || (m_kbspecial->read() & 0x10)) ? 0x80 : 0;

		case 0x62:  // button 1 or Solid Apple
		case 0x6a:
			return ((m_joybuttons->read() & 0x20) || (m_kbspecial->read() & 0x20)) ? 0x80 : 0;

		case 0x63:  // button 2 or SHIFT key
		case 0x6b:
			return ((m_joybuttons->read() & 0x40) || (m_kbspecial->read() & 0x06)) ? 0x80 : 0;

		case 0x64:  // joy 1 X axis
		case 0x6c:
			return (space.machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0;

		case 0x65:  // joy 1 Y axis
		case 0x6d:
			return (space.machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0;

		case 0x66: // joy 2 X axis
		case 0x6e:
			return (space.machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0;

		case 0x67: // joy 2 Y axis
		case 0x6f:
			return (space.machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0;

		case 0x7e:  // read IOUDIS
			return m_ioudis ? 0x80 : 0x00;

		case 0x7f:  // read DHIRES
			return m_video->m_dhires ? 0x00 : 0x80;

		default:
			do_io(space, offset, false);
			break;
	}

	return read_floatingbus();
}

WRITE8_MEMBER(apple2e_state::c000_w)
{
	if(space.debugger_access()) return;

	switch (offset)
	{
		case 0x00:  // 80STOREOFF
			m_80store = false;
			auxbank_update();
			break;

		case 0x01:  // 80STOREON
			m_80store = true;
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
			m_video->m_80col = false;
			break;

		case 0x0d:  // 80COLON
			m_video->m_80col = true;
			break;

		case 0x0e:  // ALTCHARSETOFF
			m_video->m_altcharset = false;
			break;

		case 0x0f:  // ALTCHARSETON
			m_video->m_altcharset = true;
			break;

		case 0x10:  // clear keyboard latch
			m_strobe = 0;
			break;

		case 0x20:  // cassette output
			if (m_cassette)
			{
				m_cassette_out ^= 1;
				m_cassette->output(m_cassette_out ? 1.0f : -1.0f);
			}
			break;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d:
			if (m_auxslotdevice)
			{
				m_auxslotdevice->write_c07x(space, offset & 0xf, data);

				// card may have banked auxram; get a new bank pointer
				m_aux_bank_ptr = m_auxslotdevice->get_auxbank_ptr();
			}
			do_io(space, offset, false);    // make sure it also side-effect resets the paddles as documented
			break;

		case 0x7e:  // SETIOUDIS
			m_ioudis = true; break;

		case 0x7f:  // CLRIOUDIS
			m_ioudis = false; break;

		default:
			do_io(space, offset, false);
			break;
	}
}

READ8_MEMBER(apple2e_state::c000_iic_r)
{
	if(space.debugger_access()) return read_floatingbus();

	switch (offset)
	{
		case 0x00:  // keyboard latch
			return m_transchar | m_strobe;

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

		case 0x10:  // read any key down, reset keyboard strobe
			{
				UINT8 rv = m_transchar | (m_anykeydown ? 0x80 : 0x00);
				m_strobe = 0;
				return rv;
			}

		case 0x11:  // read LCRAM2 (LC Dxxx bank)
			return m_lcram2 ? 0x80 : 0x00;
			break;

		case 0x12:  // read LCRAM (is LC readable?)
			return m_lcram ? 0x80 : 0x00;
			break;

		case 0x13:  // read RAMRD
			return m_ramrd ? 0x80 : 0x00;

		case 0x14:  // read RAMWRT
			return m_ramwrt ? 0x80 : 0x00;

		case 0x15:  // read & reset mouse X0 interrupt flag
			lower_irq(IRQ_MOUSEXY);
			return m_xirq ? 0x80 : 0x00;

		case 0x16:  // read ALTZP
			return m_altzp ? 0x80 : 0x00;

		case 0x17:  // read & reset mouse Y0 interrupt flag
			lower_irq(IRQ_MOUSEXY);
			return m_yirq ? 0x80 : 0x00;

		case 0x18:  // read 80STORE
			return m_80store ? 0x80 : 0x00;

		case 0x19:  // read VBL
			return m_vbl ? 0x80 : 0x00;

		case 0x1a:  // read TEXT
			return m_video->m_graphics ? 0x00 : 0x80;

		case 0x1b:  // read MIXED
			return m_video->m_mix ? 0x80 : 0x00;

		case 0x1c:  // read PAGE2
			return m_page2 ? 0x80 : 0x00;

		case 0x1d:  // read HIRES
			return m_video->m_hires ? 0x80 : 0x00;

		case 0x1e:  // read ALTCHARSET
			return m_video->m_altcharset ? 0x80 : 0x00;

		case 0x1f:  // read 80COL
			return m_video->m_80col ? 0x80 : 0x00;

		case 0x40:  // read XYMask (IIc only)
			return m_xy ? 0x80 : 0x00;

		case 0x41:  // read VBL mask (IIc only)
			return m_vblmask ? 0x80 : 0x00;

		case 0x42:  // read X0Edge (IIc only)
			return m_x0edge ? 0x80 : 0x00;

		case 0x43:  // read Y0Edge (IIc only)
			return m_y0edge ? 0x80 : 0x00;

		case 0x60: // 40/80 column switch (IIc only)
			return (m_sysconfig->read() & 0x04) ? 0x80 : 0;

		case 0x61:  // button 0 or Open Apple or mouse button 1
		case 0x69:
			return ((m_joybuttons->read() & 0x10) || (m_kbspecial->read() & 0x10)) ? 0x80 : 0;

		case 0x62:  // button 1 or Solid Apple
		case 0x6a:
			return ((m_joybuttons->read() & 0x20) || (m_kbspecial->read() & 0x20)) ? 0x80 : 0;

		case 0x63:  // mouse button 2 (no other function on IIc)
		case 0x6b:
			return m_mouseb->read() ? 0 : 0x80;

		case 0x64:  // joy 1 X axis
		case 0x6c:
			return (space.machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0;

		case 0x65:  // joy 1 Y axis
		case 0x6d:
			return (space.machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0;

		case 0x66: // mouse X1 (IIc only)
		case 0x6e:
			return m_x1 ? 0x80 : 0;

		case 0x67: // mouse Y1 (IIc only)
		case 0x6f:
			return m_y1 ? 0x80 : 0;

		case 0x7e:  // read IOUDIS
			m_vbl = false;
			lower_irq(IRQ_VBL);
			return m_ioudis ? 0x80 : 0x00;

		case 0x7f:  // read DHIRES
			return m_video->m_dhires ? 0x00 : 0x80;

		default:
			do_io(space, offset, true);
			break;
	}

	return read_floatingbus();
}

WRITE8_MEMBER(apple2e_state::c000_iic_w)
{
	if(space.debugger_access()) return;

	switch (offset)
	{
		case 0x00:  // 80STOREOFF
			m_80store = false;
			auxbank_update();
			break;

		case 0x01:  // 80STOREON
			m_80store = true;
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
			m_video->m_80col = false;
			break;

		case 0x0d:  // 80COLON
			m_video->m_80col = true;
			break;

		case 0x0e:  // ALTCHARSETOFF
			m_video->m_altcharset = false;
			break;

		case 0x0f:  // ALTCHARSETON
			m_video->m_altcharset = true;
			break;

		case 0x10:  // clear keyboard latch
			m_strobe = 0;
			break;

		case 0x78:
		case 0x79:
			m_vbl = false;
			lower_irq(IRQ_VBL);
			break;

		case 0x7e:  // SETIOUDIS
			m_ioudis = true;
			break;

		case 0x7f:  // CLRIOUDIS
			m_ioudis = false;
			break;

		default:
			do_io(space, offset, true);
			break;
	}
}

void apple2e_state::update_iic_mouse()
{
	int new_mx, new_my;

	// read the axes
	new_mx = m_mousex->read();
	new_my = m_mousey->read();

	// did X change?
	if (new_mx != last_mx)
	{
		int diff = new_mx - last_mx;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		count_x += diff;
		last_mx = new_mx;
	}

	// did Y change?
	if (new_my != last_my)
	{
		int diff = new_my - last_my;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

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

READ8_MEMBER(apple2e_state::c080_r)
{
	if(!space.debugger_access())
	{
		int slot;

		offset &= 0x7F;
		slot = offset / 0x10;

		if (slot == 0)
		{
			lc_update(offset & 0xf);
		}
		else
		{
			if (m_slotdevice[slot] != NULL)
			{
				return m_slotdevice[slot]->read_c0nx(space, offset % 0x10);
			}
		}
	}

	return read_floatingbus();
}

WRITE8_MEMBER(apple2e_state::c080_w)
{
	int slot;

	offset &= 0x7F;
	slot = offset / 0x10;

	if (slot == 0)
	{
		lc_update(offset & 0xf);
	}
	else
	{
		if (m_slotdevice[slot] != NULL)
		{
			m_slotdevice[slot]->write_c0nx(space, offset % 0x10, data);
		}
	}
}

UINT8 apple2e_state::read_slot_rom(address_space &space, int slotbias, int offset)
{
	int slotnum = ((offset>>8) & 0xf) + slotbias;

	if (m_slotdevice[slotnum] != NULL)
	{
		if ((m_cnxx_slot == CNXX_UNCLAIMED) && (m_slotdevice[slotnum]->take_c800()) && (!space.debugger_access()))
		{
			m_cnxx_slot = slotnum;
			update_slotrom_banks();
		}

		return m_slotdevice[slotnum]->read_cnxx(space, offset&0xff);
	}

	return read_floatingbus();
}

void apple2e_state::write_slot_rom(address_space &space, int slotbias, int offset, UINT8 data)
{
	int slotnum = ((offset>>8) & 0xf) + slotbias;

	if (m_slotdevice[slotnum] != NULL)
	{
		if ((m_cnxx_slot == CNXX_UNCLAIMED) && (m_slotdevice[slotnum]->take_c800()) && (!space.debugger_access()))
		{
			m_cnxx_slot = slotnum;
			update_slotrom_banks();
		}

		m_slotdevice[slotnum]->write_cnxx(space, offset&0xff, data);
	}
}

UINT8 apple2e_state::read_int_rom(address_space &space, int slotbias, int offset)
{
	if ((m_cnxx_slot == CNXX_UNCLAIMED) && (!space.debugger_access()))
	{
		m_cnxx_slot = CNXX_INTROM;
		update_slotrom_banks();
	}

	return m_rom_ptr[slotbias + offset];
}

READ8_MEMBER(apple2e_state::c100_r)  { return read_slot_rom(space, 1, offset); }
READ8_MEMBER(apple2e_state::c100_int_r)  { return read_int_rom(space, 0x100, offset); }
READ8_MEMBER(apple2e_state::c100_int_bank_r)  { return read_int_rom(space, 0x4100, offset); }
WRITE8_MEMBER(apple2e_state::c100_w) { write_slot_rom(space, 1, offset, data); }
READ8_MEMBER(apple2e_state::c300_r)  { return read_slot_rom(space, 3, offset); }
READ8_MEMBER(apple2e_state::c300_int_r)  { return read_int_rom(space, 0x300, offset); }
READ8_MEMBER(apple2e_state::c300_int_bank_r)  { return read_int_rom(space, 0x4300, offset); }
WRITE8_MEMBER(apple2e_state::c300_w) { write_slot_rom(space, 3, offset, data); }
READ8_MEMBER(apple2e_state::c400_r)  { return read_slot_rom(space, 4, offset); }
READ8_MEMBER(apple2e_state::c400_int_r)  { return read_int_rom(space, 0x400, offset); }
READ8_MEMBER(apple2e_state::c400_int_bank_r)  { return read_int_rom(space, 0x4400, offset); }
WRITE8_MEMBER(apple2e_state::c400_w) { write_slot_rom(space, 4, offset, data); }

READ8_MEMBER(apple2e_state::c800_r)
{
	if ((m_isiicplus) && (offset >= 0x600))
	{
		return m_iicplus_ce00[offset-0x600];
	}

	if (offset == 0x7ff)
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		update_slotrom_banks();
		return 0xff;
	}

	if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != NULL))
	{
		return m_slotdevice[m_cnxx_slot]->read_c800(space, offset&0xfff);
	}

	return read_floatingbus();
}

WRITE8_MEMBER(apple2e_state::c800_w)
{
	if ((m_isiicplus) && (offset >= 0x600))
	{
		m_iicplus_ce00[offset-0x600] = data;
		return;
	}

	if (offset == 0x7ff)
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		update_slotrom_banks();
		return;
	}

	if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != NULL))
	{
		m_slotdevice[m_cnxx_slot]->write_c800(space, offset&0xfff, data);
	}
}

READ8_MEMBER(apple2e_state::inh_r)
{
	if (m_inh_slot != -1)
	{
		return m_slotdevice[m_inh_slot]->read_inh_rom(space, offset + 0xd000);
	}

	assert(0);  // hitting inh_r with invalid m_inh_slot should not be possible
	return read_floatingbus();
}

WRITE8_MEMBER(apple2e_state::inh_w)
{
	if (m_inh_slot != -1)
	{
		m_slotdevice[m_inh_slot]->write_inh_rom(space, offset + 0xd000, data);
	}
}

READ8_MEMBER(apple2e_state::lc_r)
{
	if (m_altzp)
	{
		if (m_aux_bank_ptr)
		{
			if (offset < 0x1000)
			{
				if (m_lcram2)
				{
					return m_aux_bank_ptr[(offset & 0xfff) + 0xd000];
				}
				else
				{
					return m_aux_bank_ptr[(offset & 0xfff) + 0xc000];
				}
			}

			return m_aux_bank_ptr[(offset & 0x1fff) + 0xe000];
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

WRITE8_MEMBER(apple2e_state::lc_w)
{
	if (!m_lcwriteenable)
	{
		return;
	}

	if (m_altzp)
	{
		if (m_aux_bank_ptr)
		{
			if (offset < 0x1000)
			{
				if (m_lcram2)
				{
					m_aux_bank_ptr[(offset & 0xfff) + 0xd000] = data;
				}
				else
				{
					m_aux_bank_ptr[(offset & 0xfff) + 0xc000] = data;
				}
				return;
			}

			m_aux_bank_ptr[(offset & 0x1fff) + 0xe000] = data;
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

// floating bus code from old machine/apple2: needs to be reworked based on real beam position to enable e.g. Bob Bishop's screen splitter
UINT8 apple2e_state::read_floatingbus()
{
	enum
	{
		// scanner types
		kScannerNone = 0, kScannerApple2, kScannerApple2e,

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
		kClocksPerVSync   = kHClocks * kNTSCScanLines // FIX: NTSC only?
	};

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
	Hires    = m_video->m_hires ? 1 : 0;
	Mixed    = m_video->m_mix ? 1 : 0;
	Page2    = m_page2 ? 1 : 0;
	_80Store = m_80store ? 1 : 0;

	// calculate video parameters according to display standard
	//
	ScanLines  = 1 ? kNTSCScanLines : kPALScanLines; // FIX: NTSC only?
	// VSyncLine  = 1 ? kNTSCVSyncLine : kPALVSyncLine; // FIX: NTSC only?
	// ScanCycles = ScanLines * kHClocks;

	// calculate horizontal scanning state
	//
	h_clock = (i + kHPEClock) % kHClocks; // which horizontal scanning clock
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
	v_line  = i / kHClocks; // which vertical scanning line
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
	else
	{
		// N: text, so no higher address bits unless Apple ][, not Apple //e
		//
		if ((1) && // Apple ][? // FIX: check for Apple ][? (FB is most useful in old games)
			(kHPEClock <= h_clock) && // Y: HBL?
			(h_clock <= (kHClocks - 1)))
		{
			address |= 1 << 12; // Y: a12 (add $1000 to address!)
		}
	}

	return m_ram_ptr[address % m_ram_size]; // FIX: this seems to work, but is it right!?
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

READ8_MEMBER(apple2e_state::ram0000_r)  { return m_ram_ptr[offset]; }
WRITE8_MEMBER(apple2e_state::ram0000_w) { m_ram_ptr[offset] = data; }
READ8_MEMBER(apple2e_state::ram0200_r)  { return m_ram_ptr[offset+0x200]; }
WRITE8_MEMBER(apple2e_state::ram0200_w) { m_ram_ptr[offset+0x200] = data; }
READ8_MEMBER(apple2e_state::ram0400_r)  { return m_ram_ptr[offset+0x400]; }
WRITE8_MEMBER(apple2e_state::ram0400_w) { m_ram_ptr[offset+0x400] = data; }
READ8_MEMBER(apple2e_state::ram0800_r)  { return m_ram_ptr[offset+0x800]; }
WRITE8_MEMBER(apple2e_state::ram0800_w) { m_ram_ptr[offset+0x800] = data; }
READ8_MEMBER(apple2e_state::ram2000_r)  { return m_ram_ptr[offset+0x2000]; }
WRITE8_MEMBER(apple2e_state::ram2000_w) { m_ram_ptr[offset+0x2000] = data; }
READ8_MEMBER(apple2e_state::ram4000_r)  { return m_ram_ptr[offset+0x4000]; }
WRITE8_MEMBER(apple2e_state::ram4000_w) { m_ram_ptr[offset+0x4000] = data; }

READ8_MEMBER(apple2e_state::auxram0000_r)  { if (m_aux_bank_ptr) { return m_aux_bank_ptr[offset]; } else { return read_floatingbus(); } }
WRITE8_MEMBER(apple2e_state::auxram0000_w) { if (m_aux_bank_ptr) { m_aux_bank_ptr[offset] = data; } }
READ8_MEMBER(apple2e_state::auxram0200_r)  { if (m_aux_bank_ptr) { return m_aux_bank_ptr[offset+0x200]; } else { return read_floatingbus(); } }
WRITE8_MEMBER(apple2e_state::auxram0200_w) { if (m_aux_bank_ptr) { m_aux_bank_ptr[offset+0x200] = data; } }
READ8_MEMBER(apple2e_state::auxram0400_r)  { if (m_aux_bank_ptr) { return m_aux_bank_ptr[offset+0x400]; } else { return read_floatingbus(); } }
WRITE8_MEMBER(apple2e_state::auxram0400_w) { if (m_aux_bank_ptr) { m_aux_bank_ptr[offset+0x400] = data; } }
READ8_MEMBER(apple2e_state::auxram0800_r)  { if (m_aux_bank_ptr) { return m_aux_bank_ptr[offset+0x800]; } else { return read_floatingbus(); } }
WRITE8_MEMBER(apple2e_state::auxram0800_w) { if (m_aux_bank_ptr) { m_aux_bank_ptr[offset+0x800] = data; } }
READ8_MEMBER(apple2e_state::auxram2000_r)  { if (m_aux_bank_ptr) { return m_aux_bank_ptr[offset+0x2000]; } else { return read_floatingbus(); } }
WRITE8_MEMBER(apple2e_state::auxram2000_w) { if (m_aux_bank_ptr) { m_aux_bank_ptr[offset+0x2000] = data; } }
READ8_MEMBER(apple2e_state::auxram4000_r)  { if (m_aux_bank_ptr) { return m_aux_bank_ptr[offset+0x4000]; } else { return read_floatingbus(); } }
WRITE8_MEMBER(apple2e_state::auxram4000_w) { if (m_aux_bank_ptr) { m_aux_bank_ptr[offset+0x4000] = data; } }

static ADDRESS_MAP_START( apple2e_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x01ff) AM_DEVICE(A2_0000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0200, 0x03ff) AM_DEVICE(A2_0200_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0400, 0x07ff) AM_DEVICE(A2_0400_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0800, 0x1fff) AM_DEVICE(A2_0800_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x2000, 0x3fff) AM_DEVICE(A2_2000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x4000, 0xbfff) AM_DEVICE(A2_4000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc000, 0xc07f) AM_READWRITE(c000_r, c000_w)
	AM_RANGE(0xc080, 0xc0ff) AM_READWRITE(c080_r, c080_w)
	AM_RANGE(0xc100, 0xc2ff) AM_DEVICE(A2_C100_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc300, 0xc3ff) AM_DEVICE(A2_C300_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc400, 0xc7ff) AM_DEVICE(A2_C400_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc800, 0xcfff) AM_DEVICE(A2_C800_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xd000, 0xffff) AM_DEVICE(A2_UPPERBANK_TAG, address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( apple2c_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x01ff) AM_DEVICE(A2_0000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0200, 0x03ff) AM_DEVICE(A2_0200_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0400, 0x07ff) AM_DEVICE(A2_0400_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0800, 0x1fff) AM_DEVICE(A2_0800_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x2000, 0x3fff) AM_DEVICE(A2_2000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x4000, 0xbfff) AM_DEVICE(A2_4000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc000, 0xc07f) AM_READWRITE(c000_iic_r, c000_iic_w)
	AM_RANGE(0xc098, 0xc09b) AM_DEVREADWRITE(IIC_ACIA1_TAG, mos6551_device, read, write)
	AM_RANGE(0xc0a8, 0xc0ab) AM_DEVREADWRITE(IIC_ACIA2_TAG, mos6551_device, read, write)
	AM_RANGE(0xc080, 0xc0ff) AM_READWRITE(c080_r, c080_w)
	AM_RANGE(0xc100, 0xc2ff) AM_DEVICE(A2_C100_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc300, 0xc3ff) AM_DEVICE(A2_C300_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc400, 0xc7ff) AM_DEVICE(A2_C400_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc800, 0xcfff) AM_DEVICE(A2_C800_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xd000, 0xffff) AM_DEVICE(A2_UPPERBANK_TAG, address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( apple2c_memexp_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x01ff) AM_DEVICE(A2_0000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0200, 0x03ff) AM_DEVICE(A2_0200_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0400, 0x07ff) AM_DEVICE(A2_0400_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0800, 0x1fff) AM_DEVICE(A2_0800_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x2000, 0x3fff) AM_DEVICE(A2_2000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x4000, 0xbfff) AM_DEVICE(A2_4000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc000, 0xc07f) AM_READWRITE(c000_iic_r, c000_iic_w)
	AM_RANGE(0xc098, 0xc09b) AM_DEVREADWRITE(IIC_ACIA1_TAG, mos6551_device, read, write)
	AM_RANGE(0xc0a8, 0xc0ab) AM_DEVREADWRITE(IIC_ACIA2_TAG, mos6551_device, read, write)
	AM_RANGE(0xc0c0, 0xc0c3) AM_READWRITE(memexp_r, memexp_w)
	AM_RANGE(0xc080, 0xc0ff) AM_READWRITE(c080_r, c080_w)
	AM_RANGE(0xc100, 0xc2ff) AM_DEVICE(A2_C100_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc300, 0xc3ff) AM_DEVICE(A2_C300_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc400, 0xc7ff) AM_DEVICE(A2_C400_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc800, 0xcfff) AM_DEVICE(A2_C800_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xd000, 0xffff) AM_DEVICE(A2_UPPERBANK_TAG, address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( laser128_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x01ff) AM_DEVICE(A2_0000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0200, 0x03ff) AM_DEVICE(A2_0200_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0400, 0x07ff) AM_DEVICE(A2_0400_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x0800, 0x1fff) AM_DEVICE(A2_0800_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x2000, 0x3fff) AM_DEVICE(A2_2000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x4000, 0xbfff) AM_DEVICE(A2_4000_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc000, 0xc07f) AM_READWRITE(c000_r, c000_w)
//  AM_RANGE(0xc098, 0xc09b) AM_DEVREADWRITE(IIC_ACIA1_TAG, mos6551_device, read, write)
//  AM_RANGE(0xc0a8, 0xc0ab) AM_DEVREADWRITE(IIC_ACIA2_TAG, mos6551_device, read, write)
	AM_RANGE(0xc0d0, 0xc0d3) AM_READWRITE(memexp_r, memexp_w)
	AM_RANGE(0xc0e0, 0xc0ef) AM_DEVREADWRITE(LASER128_UDC_TAG, applefdc_base_device, read, write)
	AM_RANGE(0xc080, 0xc0ff) AM_READWRITE(c080_r, c080_w)
	AM_RANGE(0xc100, 0xc2ff) AM_DEVICE(A2_C100_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc300, 0xc3ff) AM_DEVICE(A2_C300_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc400, 0xc7ff) AM_DEVICE(A2_C400_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xc800, 0xcfff) AM_DEVICE(A2_C800_TAG, address_map_bank_device, amap8)
	AM_RANGE(0xd000, 0xffff) AM_DEVICE(A2_UPPERBANK_TAG, address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( r0000bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE(ram0000_r, ram0000_w)
	AM_RANGE(0x0200, 0x03ff) AM_READWRITE(auxram0000_r, auxram0000_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( r0200bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE(ram0200_r, ram0200_w) // wr 0 rd 0
	AM_RANGE(0x0200, 0x03ff) AM_READWRITE(auxram0200_r, ram0200_w) // wr 0 rd 1
	AM_RANGE(0x0400, 0x05ff) AM_READWRITE(ram0200_r, auxram0200_w) // wr 1 rd 0
	AM_RANGE(0x0600, 0x07ff) AM_READWRITE(auxram0200_r, auxram0200_w) // wr 1 rd 1
ADDRESS_MAP_END

static ADDRESS_MAP_START( r0400bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x03ff) AM_READWRITE(ram0400_r, ram0400_w) // wr 0 rd 0
	AM_RANGE(0x0400, 0x07ff) AM_READWRITE(auxram0400_r, ram0400_w)  // wr 0 rd 1
	AM_RANGE(0x0800, 0x0bff) AM_READWRITE(ram0400_r, auxram0400_w)  // wr 1 rd 0
	AM_RANGE(0x0c00, 0x0fff) AM_READWRITE(auxram0400_r, auxram0400_w) // wr 1 rd 1
ADDRESS_MAP_END

static ADDRESS_MAP_START( r0800bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x17ff) AM_READWRITE(ram0800_r, ram0800_w)
	AM_RANGE(0x2000, 0x37ff) AM_READWRITE(auxram0800_r, ram0800_w)
	AM_RANGE(0x4000, 0x57ff) AM_READWRITE(ram0800_r, auxram0800_w)
	AM_RANGE(0x6000, 0x77ff) AM_READWRITE(auxram0800_r, auxram0800_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( r2000bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(ram2000_r, ram2000_w)
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(auxram2000_r, ram2000_w)
	AM_RANGE(0x4000, 0x5fff) AM_READWRITE(ram2000_r, auxram2000_w)
	AM_RANGE(0x6000, 0x7fff) AM_READWRITE(auxram2000_r, auxram2000_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( r4000bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x00000, 0x07fff) AM_READWRITE(ram4000_r, ram4000_w)
	AM_RANGE(0x08000, 0x0ffff) AM_READWRITE(auxram4000_r, ram4000_w)
	AM_RANGE(0x10000, 0x17fff) AM_READWRITE(ram4000_r, auxram4000_w)
	AM_RANGE(0x18000, 0x1ffff) AM_READWRITE(auxram4000_r, auxram4000_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( c100bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE(c100_r, c100_w)
	AM_RANGE(0x0200, 0x03ff) AM_READ(c100_int_r) AM_WRITENOP
	AM_RANGE(0x0400, 0x05ff) AM_READ(c100_int_bank_r) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( c300bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x00ff) AM_READWRITE(c300_r, c300_w)
	AM_RANGE(0x0100, 0x01ff) AM_READ(c300_int_r) AM_WRITENOP
	AM_RANGE(0x0200, 0x02ff) AM_READ(c300_int_bank_r) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( c400bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x03ff) AM_READWRITE(c400_r, c400_w)
	AM_RANGE(0x0400, 0x07ff) AM_READ(c400_int_r) AM_WRITENOP
	AM_RANGE(0x0800, 0x0bff) AM_READ(c400_int_bank_r) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( c800bank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x07ff) AM_READWRITE(c800_r, c800_w)
	AM_RANGE(0x0800, 0x0fff) AM_ROM AM_REGION("maincpu", 0x800)
	AM_RANGE(0x1000, 0x17ff) AM_ROM AM_REGION("maincpu", 0x4800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( inhbank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x2fff) AM_DEVICE(A2_LCBANK_TAG, address_map_bank_device, amap8)
	AM_RANGE(0x3000, 0x5fff) AM_READWRITE(inh_r, inh_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lcbank_map, AS_PROGRAM, 8, apple2e_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM AM_REGION("maincpu", 0x1000) AM_WRITE(lc_w)
	AM_RANGE(0x3000, 0x5fff) AM_READWRITE(lc_r, lc_w)
	AM_RANGE(0x6000, 0x8fff) AM_ROM AM_REGION("maincpu", 0x5000) AM_WRITE(lc_w)
ADDRESS_MAP_END

/***************************************************************************
    KEYBOARD
***************************************************************************/

READ_LINE_MEMBER(apple2e_state::ay3600_shift_r)
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

READ_LINE_MEMBER(apple2e_state::ay3600_control_r)
{
	if (m_kbspecial->read() & 0x08)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

WRITE_LINE_MEMBER(apple2e_state::ay3600_data_ready_w)
{
	if (state == ASSERT_LINE)
	{
		UINT8 *decode = m_kbdrom->base();
		UINT16 trans;

		m_lastchar = m_ay3600->b_r();

		trans = m_lastchar & ~(0x1c0);  // clear the 3600's control/shift stuff
		trans |= (m_lastchar & 0x100)>>2;   // bring the 0x100 bit down to the 0x40 place
		trans <<= 2;                    // 4 entries per key
		trans |= (m_kbspecial->read() & 0x06) ? 0x00 : 0x01;    // shift is bit 1 (active low)
		trans |= (m_kbspecial->read() & 0x08) ? 0x00 : 0x02;    // control is bit 2 (active low)
		trans |= (m_kbspecial->read() & 0x01) ? 0x0000 : 0x0200;    // caps lock is bit 9 (active low)

		if (m_isiic)
		{
			if (m_sysconfig->read() & 0x08)
			{
				trans += 0x400; // go to DVORAK half of the ROM
			}
		}

		m_transchar = decode[trans];
		m_strobe = 0x80;

//      printf("new char = %04x (%02x)\n", m_lastchar, m_transchar);
	}
}

WRITE_LINE_MEMBER(apple2e_state::ay3600_ako_w)
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

static INPUT_PORTS_START( apple2_joystick )
	PORT_START("joystick_1_x")      /* Joystick 1 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P1 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_4_PAD)    PORT_CODE_INC(KEYCODE_6_PAD)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("joystick_1_y")      /* Joystick 1 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P1 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_8_PAD)    PORT_CODE_INC(KEYCODE_2_PAD)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("joystick_2_x")      /* Joystick 2 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P2 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("joystick_2_y")      /* Joystick 2 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P2 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("joystick_buttons")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(1)            PORT_CODE(KEYCODE_0_PAD)    PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_PLAYER(1)            PORT_CODE(KEYCODE_ENTER_PAD)PORT_CODE(JOYCODE_BUTTON2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(2)            PORT_CODE(JOYCODE_BUTTON1)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2_gameport )
	PORT_INCLUDE( apple2_joystick )
INPUT_PORTS_END

static INPUT_PORTS_START( apple2_sysconfig )
	PORT_START("a2_config")
	PORT_CONFNAME(0x03, 0x00, "Composite monitor type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x01, "B&W")
	PORT_CONFSETTING(0x02, "Green")
	PORT_CONFSETTING(0x03, "Amber")
INPUT_PORTS_END

static INPUT_PORTS_START( apple2c_sysconfig )
	PORT_START("a2_config")
	PORT_CONFNAME(0x03, 0x00, "Composite monitor type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x01, "B&W")
	PORT_CONFSETTING(0x02, "Green")
	PORT_CONFSETTING(0x03, "Amber")
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
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
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
INPUT_PORTS_END

static INPUT_PORTS_START( apple2e )
	PORT_INCLUDE( apple2e_common )
	PORT_INCLUDE( apple2_gameport )
	PORT_INCLUDE( apple2_sysconfig )
INPUT_PORTS_END

static INPUT_PORTS_START( apple2c )
	PORT_INCLUDE( apple2e_common )
	PORT_INCLUDE( apple2_gameport )
	PORT_INCLUDE( apple2c_sysconfig )

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

	PORT_INCLUDE( apple2_gameport )
	PORT_INCLUDE(apple2_sysconfig)
INPUT_PORTS_END

INPUT_PORTS_START( apple2ep )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
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
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
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

	PORT_INCLUDE( apple2_gameport )
	PORT_INCLUDE(apple2_sysconfig)
INPUT_PORTS_END

static SLOT_INTERFACE_START(apple2_cards)
	SLOT_INTERFACE("diskii", A2BUS_DISKII)  /* Disk II Controller Card */
	SLOT_INTERFACE("diskiing", A2BUS_DISKIING)  /* Disk II Controller Card, cycle-accurate version */
	SLOT_INTERFACE("mockingboard", A2BUS_MOCKINGBOARD)  /* Sweet Micro Systems Mockingboard */
	SLOT_INTERFACE("phasor", A2BUS_PHASOR)  /* Applied Engineering Phasor */
	SLOT_INTERFACE("cffa2", A2BUS_CFFA2)  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 65C02/65816 firmware */
	SLOT_INTERFACE("cffa202", A2BUS_CFFA2_6502)  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 6502 firmware */
	SLOT_INTERFACE("memexp", A2BUS_MEMEXP)  /* Apple II Memory Expansion Card */
	SLOT_INTERFACE("ramfactor", A2BUS_RAMFACTOR)    /* Applied Engineering RamFactor */
	SLOT_INTERFACE("thclock", A2BUS_THUNDERCLOCK)    /* ThunderWare ThunderClock Plus */
	SLOT_INTERFACE("softcard", A2BUS_SOFTCARD)  /* Microsoft SoftCard */
	SLOT_INTERFACE("videoterm", A2BUS_VIDEOTERM)    /* Videx VideoTerm */
	SLOT_INTERFACE("ssc", A2BUS_SSC)    /* Apple Super Serial Card */
	SLOT_INTERFACE("swyft", A2BUS_SWYFT)    /* IAI SwyftCard */
	SLOT_INTERFACE("themill", A2BUS_THEMILL)    /* Stellation Two The Mill (6809 card) */
	SLOT_INTERFACE("sam", A2BUS_SAM)    /* SAM Software Automated Mouth (8-bit DAC + speaker) */
	SLOT_INTERFACE("alfam2", A2BUS_ALFAM2)    /* ALF Apple Music II */
	SLOT_INTERFACE("echoii", A2BUS_ECHOII)    /* Street Electronics Echo II */
	SLOT_INTERFACE("ap16", A2BUS_IBSAP16)    /* IBS AP16 (German VideoTerm clone) */
	SLOT_INTERFACE("ap16alt", A2BUS_IBSAP16ALT)    /* IBS AP16 (German VideoTerm clone), alternate revision */
	SLOT_INTERFACE("vtc1", A2BUS_VTC1)    /* Unknown VideoTerm clone #1 */
	SLOT_INTERFACE("vtc2", A2BUS_VTC2)    /* Unknown VideoTerm clone #2 */
	SLOT_INTERFACE("arcbd", A2BUS_ARCADEBOARD)    /* Third Millenium Engineering Arcade Board */
	SLOT_INTERFACE("midi", A2BUS_MIDI)  /* Generic 6840+6850 MIDI board */
	SLOT_INTERFACE("zipdrive", A2BUS_ZIPDRIVE)  /* ZIP Technologies IDE card */
	SLOT_INTERFACE("echoiiplus", A2BUS_ECHOPLUS)    /* Street Electronics Echo Plus (Echo II + Mockingboard clone) */
	SLOT_INTERFACE("scsi", A2BUS_SCSI)  /* Apple II SCSI Card */
	SLOT_INTERFACE("applicard", A2BUS_APPLICARD)    /* PCPI Applicard */
	SLOT_INTERFACE("aesms", A2BUS_AESMS)    /* Applied Engineering Super Music Synthesizer */
	SLOT_INTERFACE("ultraterm", A2BUS_ULTRATERM)    /* Videx UltraTerm (original) */
	SLOT_INTERFACE("ultratermenh", A2BUS_ULTRATERMENH)    /* Videx UltraTerm (enhanced //e) */
	SLOT_INTERFACE("aevm80", A2BUS_VTC2)    /* Applied Engineering ViewMaster 80 */
	SLOT_INTERFACE("parallel", A2BUS_PIC)   /* Apple Parallel Interface Card */
	SLOT_INTERFACE("corvus", A2BUS_CORVUS)  /* Corvus flat-cable HDD interface (see notes in a2corvus.c) */
	SLOT_INTERFACE("mcms1", A2BUS_MCMS1)  /* Mountain Computer Music System, card 1 of 2 */
	SLOT_INTERFACE("mcms2", A2BUS_MCMS2)  /* Mountain Computer Music System, card 2 of 2.  must be in card 1's slot + 1! */
	SLOT_INTERFACE("dx1", A2BUS_DX1)    /* Decillonix DX-1 sampler card */
	SLOT_INTERFACE("tm2ho", A2BUS_TIMEMASTERHO) /* Applied Engineering TimeMaster II H.O. */
	SLOT_INTERFACE("mouse", A2BUS_MOUSE)    /* Apple II Mouse Card */
	SLOT_INTERFACE("ezcgi", A2BUS_EZCGI)    /* E-Z Color Graphics Interface */
	SLOT_INTERFACE("ezcgi9938", A2BUS_EZCGI_9938)   /* E-Z Color Graphics Interface (TMS9938) */
	SLOT_INTERFACE("ezcgi9958", A2BUS_EZCGI_9958)   /* E-Z Color Graphics Interface (TMS9958) */
//  SLOT_INTERFACE("magicmusician", A2BUS_MAGICMUSICIAN)    /* Magic Musician Card */
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(apple2eaux_cards)
	SLOT_INTERFACE("std80", A2EAUX_STD80COL) /* Apple IIe Standard 80 Column Card */
	SLOT_INTERFACE("ext80", A2EAUX_EXT80COL) /* Apple IIe Extended 80 Column Card */
	SLOT_INTERFACE("rw3", A2EAUX_RAMWORKS3)  /* Applied Engineering RamWorks III */
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( apple2e, apple2e_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 1021800)     /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2e_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", apple2e_state, apple2_interrupt, "screen", 0, 1)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_DEVICE_ADD(A2_VIDEO_TAG, APPLE2_VIDEO, XTAL_14_31818MHz)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192)
	MCFG_SCREEN_UPDATE_DRIVER(apple2e_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(apple2e_state, apple2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(A2_SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* RAM */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* 0000 banking */
	MCFG_DEVICE_ADD(A2_0000_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(r0000bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200)

	/* 0200 banking */
	MCFG_DEVICE_ADD(A2_0200_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(r0200bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200)

	/* 0400 banking */
	MCFG_DEVICE_ADD(A2_0400_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(r0400bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x400)

	/* 0800 banking */
	MCFG_DEVICE_ADD(A2_0800_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(r0800bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	/* 2000 banking */
	MCFG_DEVICE_ADD(A2_2000_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(r2000bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	/* 4000 banking */
	MCFG_DEVICE_ADD(A2_4000_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(r4000bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)

	/* C100 banking */
	MCFG_DEVICE_ADD(A2_C100_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(c100bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200)

	/* C300 banking */
	MCFG_DEVICE_ADD(A2_C300_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(c300bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x100)

	/* C400 banking */
	MCFG_DEVICE_ADD(A2_C400_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(c400bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x400)

	/* C800 banking */
	MCFG_DEVICE_ADD(A2_C800_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(c800bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x800)

	/* built-in language card emulation */
	MCFG_DEVICE_ADD(A2_LCBANK_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(lcbank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x3000)

	/* /INH banking */
	MCFG_DEVICE_ADD(A2_UPPERBANK_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(inhbank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x3000)

	/* keyboard controller */
	MCFG_DEVICE_ADD("ay3600", AY3600, 0)
	MCFG_AY3600_MATRIX_X0(IOPORT("X0"))
	MCFG_AY3600_MATRIX_X1(IOPORT("X1"))
	MCFG_AY3600_MATRIX_X2(IOPORT("X2"))
	MCFG_AY3600_MATRIX_X3(IOPORT("X3"))
	MCFG_AY3600_MATRIX_X4(IOPORT("X4"))
	MCFG_AY3600_MATRIX_X5(IOPORT("X5"))
	MCFG_AY3600_MATRIX_X6(IOPORT("X6"))
	MCFG_AY3600_MATRIX_X7(IOPORT("X7"))
	MCFG_AY3600_MATRIX_X8(IOPORT("X8"))
	MCFG_AY3600_SHIFT_CB(READLINE(apple2e_state, ay3600_shift_r))
	MCFG_AY3600_CONTROL_CB(READLINE(apple2e_state, ay3600_control_r))
	MCFG_AY3600_DATA_READY_CB(WRITELINE(apple2e_state, ay3600_data_ready_w))
	MCFG_AY3600_AKO_CB(WRITELINE(apple2e_state, ay3600_ako_w))

	/* repeat timer.  15 Hz from page 7-15 of "Understanding the Apple IIe" */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("repttmr", apple2e_state, ay3600_repeat, attotime::from_hz(15))

	/* slot devices */
	MCFG_DEVICE_ADD("a2bus", A2BUS, 0)
	MCFG_A2BUS_CPU("maincpu")
	MCFG_A2BUS_OUT_IRQ_CB(WRITELINE(apple2e_state, a2bus_irq_w))
	MCFG_A2BUS_OUT_NMI_CB(WRITELINE(apple2e_state, a2bus_nmi_w))
	MCFG_A2BUS_OUT_INH_CB(WRITELINE(apple2e_state, a2bus_inh_w))
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl1", apple2_cards, NULL)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl2", apple2_cards, NULL)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl3", apple2_cards, NULL)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl4", apple2_cards, "mockingboard")
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl5", apple2_cards, NULL)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl6", apple2_cards, "diskiing")
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl7", apple2_cards, NULL)

	MCFG_DEVICE_ADD(A2_AUXSLOT_TAG, A2EAUXSLOT, 0)
	MCFG_A2EAUXSLOT_CPU("maincpu")
	MCFG_A2EAUXSLOT_OUT_IRQ_CB(WRITELINE(apple2e_state, a2bus_irq_w))
	MCFG_A2EAUXSLOT_OUT_NMI_CB(WRITELINE(apple2e_state, a2bus_nmi_w))
	MCFG_A2EAUXSLOT_SLOT_ADD(A2_AUXSLOT_TAG, "aux", apple2eaux_cards, "ext80")   // default to an extended 80-column card

	MCFG_SOFTWARE_LIST_ADD("flop525_list","apple2")

	MCFG_CASSETTE_ADD(A2_CASSETTE_TAG)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mprof3, apple2e )
	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2ee, apple2e )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)        /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2e_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tk3000, apple2e )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)        /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2e_map)

//  MCFG_CPU_ADD("subcpu", Z80, 1021800)    // schematics are illegible on where the clock comes from, but it *seems* to be the same as the 65C02 clock
//  MCFG_CPU_PROGRAM_MAP(tk3000_kbd_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2ep, apple2e )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)        /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2e_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2c, apple2ee )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)        /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2c_map)

	// IIc and friends have no cassette port
	MCFG_DEVICE_REMOVE(A2_CASSETTE_TAG)

	MCFG_A2BUS_SLOT_REMOVE("sl1")   // IIc has no slots, of course :)
	MCFG_A2BUS_SLOT_REMOVE("sl2")
	MCFG_A2BUS_SLOT_REMOVE("sl3")
	MCFG_A2BUS_SLOT_REMOVE("sl4")
	MCFG_A2BUS_SLOT_REMOVE("sl5")
	MCFG_A2BUS_SLOT_REMOVE("sl6")
	MCFG_A2BUS_SLOT_REMOVE("sl7")

	MCFG_DEVICE_ADD(IIC_ACIA1_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_14_31818MHz / 8) // ~1.789 MHz
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE(PRINTER_PORT_TAG, rs232_port_device, write_txd))

	MCFG_DEVICE_ADD(IIC_ACIA2_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)   // matches SSC so modem software is compatible
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("modem", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD(PRINTER_PORT_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(IIC_ACIA1_TAG, mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(IIC_ACIA1_TAG, mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(IIC_ACIA1_TAG, mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(IIC_ACIA1_TAG, mos6551_device, write_cts))

	MCFG_RS232_PORT_ADD(MODEM_PORT_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(IIC_ACIA2_TAG, mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(IIC_ACIA2_TAG, mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(IIC_ACIA2_TAG, mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(IIC_ACIA2_TAG, mos6551_device, write_cts))

	// TODO: populate the IIc's other virtual slots with ONBOARD_ADD
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_DISKIING, NULL)

	MCFG_A2EAUXSLOT_SLOT_REMOVE("aux")
	MCFG_DEVICE_REMOVE(A2_AUXSLOT_TAG)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("128K")
MACHINE_CONFIG_END

const applefdc_interface a2cp_interface =
{
	sony_set_lines,         /* set_lines */
	sony_set_enable_lines,  /* set_enable_lines */

	sony_read_data,         /* read_data */
	sony_write_data,    /* write_data */
	sony_read_status    /* read_status */
};

static const floppy_interface apple2cp_floppy35_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple35_iigs),
	"floppy_3_5"
};

static MACHINE_CONFIG_DERIVED( apple2cp, apple2c )
	MCFG_A2BUS_SLOT_REMOVE("sl6")
	MCFG_IWM_ADD(IICP_IWM_TAG, a2cp_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(apple2cp_floppy35_floppy_interface)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2c_iwm, apple2c )

	MCFG_A2BUS_SLOT_REMOVE("sl6")
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_IWM_FDC, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2c_mem, apple2c )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)        /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2c_memexp_map)

	MCFG_A2BUS_SLOT_REMOVE("sl6")
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_IWM_FDC, NULL)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("128K, 384K, 640K, 896K, 1152K")
MACHINE_CONFIG_END

const applefdc_interface fdc_interface =
{
	apple525_set_lines,         /* set_lines */
	apple525_set_enable_lines,  /* set_enable_lines */

	apple525_read_data,         /* read_data */
	apple525_write_data,    /* write_data */
	apple525_read_status    /* read_status */
};

static const floppy_interface floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple2),
	"floppy_5_25"
};

static MACHINE_CONFIG_DERIVED( laser128, apple2c )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)        /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(laser128_map)

	MCFG_APPLEFDC_ADD(LASER128_UDC_TAG, fdc_interface)
	MCFG_LEGACY_FLOPPY_APPLE_2_DRIVES_ADD(floppy_interface,15,16)

	MCFG_A2BUS_SLOT_REMOVE("sl6")

	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl1", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl2", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl3", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl4", A2BUS_LASER128, NULL)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl5", apple2_cards, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_LASER128, NULL)
	MCFG_A2BUS_SLOT_ADD("a2bus", "sl7", apple2_cards, NULL)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("128K, 384K, 640K, 896K, 1152K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( laser128ex2, apple2c )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)        /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(laser128_map)

	MCFG_APPLEFDC_ADD(LASER128_UDC_TAG, fdc_interface)
	MCFG_LEGACY_FLOPPY_APPLE_2_DRIVES_ADD(floppy_interface,15,16)

	MCFG_A2BUS_SLOT_REMOVE("sl6")

	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl1", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl2", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl3", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl4", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl5", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_LASER128, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl7", A2BUS_LASER128, NULL)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("128K, 384K, 640K, 896K, 1152K")
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(apple2e)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0133-a.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "342-0133-a.chr", 0x1000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD ( "342-0135-b.64", 0x0000, 0x2000, CRC(e248835e) SHA1(523838c19c79f481fa02df56856da1ec3816d16e))
	ROM_LOAD ( "342-0134-a.64", 0x2000, 0x2000, CRC(fc3d59d8) SHA1(8895a4b703f2184b673078f411f4089889b61c54))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(apple2euk)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "341-0160-a.chr", 0x0000, 0x2000, CRC(9be77112) SHA1(48aafa9a72002c495bc1f3d28150630ff89ca47e) )

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD ( "342-0135-b.64", 0x0000, 0x2000, CRC(e248835e) SHA1(523838c19c79f481fa02df56856da1ec3816d16e))
	ROM_LOAD ( "342-0134-a.64", 0x2000, 0x2000, CRC(fc3d59d8) SHA1(8895a4b703f2184b673078f411f4089889b61c54))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "341-0150-a.e12", 0x000, 0x800, CRC(66ffacd7) SHA1(47bb9608be38ff75429a989b930a93b47099648e) )
ROM_END

ROM_START(mprof3)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "mpf3.chr", 0x0000, 0x1000,CRC(2597bc19) SHA1(e114dcbb512ec24fb457248c1b53cbd78039ed20))
	ROM_LOAD ( "mpf3.chr", 0x1000, 0x1000,CRC(2597bc19) SHA1(e114dcbb512ec24fb457248c1b53cbd78039ed20))

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD ( "mpf3-cd.rom", 0x0000, 0x2000, CRC(5b662e06) SHA1(aa0db775ca78986480829fcc10f00e57629e1a7c))
	ROM_LOAD ( "mpf3-ef.rom", 0x2000, 0x2000, CRC(2c5e8b92) SHA1(befeb03e04b7c3ef36ef5829948a53880df85e92))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real mprof keyboard ROM
ROM_END

ROM_START(apple2ee)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "342-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD ( "342-0304-a.e10", 0x0000, 0x2000, CRC(443aa7c4) SHA1(3aecc56a26134df51e65e17f33ae80c1f1ac93e6)) /* PCB: "CD ROM // 342-0304", 2364 mask rom */
	ROM_LOAD ( "342-0303-a.e8", 0x2000, 0x2000, CRC(95e10034) SHA1(afb09bb96038232dc757d40c0605623cae38088e)) /* PCB: "EF ROM // 342-0303", 2364 mask rom */

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2eeuk)
	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD( "342-0273-a.chr", 0x0000, 0x2000, CRC(9157085a) SHA1(85479a509d6c8176949a5b20720567b7022aa631) )

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD ( "342-0304-a.e10", 0x0000, 0x2000, CRC(443aa7c4) SHA1(3aecc56a26134df51e65e17f33ae80c1f1ac93e6)) /* PCB: "CD ROM // 342-0304", 2364 mask rom */
	ROM_LOAD ( "342-0303-a.e8", 0x2000, 0x2000, CRC(95e10034) SHA1(afb09bb96038232dc757d40c0605623cae38088e)) /* PCB: "EF ROM // 342-0303", 2364 mask rom */

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0150-a.e12", 0x000, 0x800, CRC(66ffacd7) SHA1(47bb9608be38ff75429a989b930a93b47099648e) )
ROM_END

ROM_START(apple2ep)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "342-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD ("32-0349-b.128", 0x0000, 0x4000, CRC(1d70b193) SHA1(b8ea90abe135a0031065e01697c4a3a20d51198b)) /* should rom name be 342-0349-b? */

	ROM_REGION( 0x800, "keyboard", 0 )
	// chip printed markings say 342-0132-d, but internally text says "341-0132-d".  Go figure.
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2c)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD ( "a2c.128", 0x0000, 0x4000, CRC(f0edaa1b) SHA1(1a9b8aca5e32bb702ddb7791daddd60a89655729))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

// unlike the very unique TK2000, the TK3000 is a mostly stock enhanced IIe clone
ROM_START(tk3000)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "tk3000.f7",    0x000000, 0x002000, CRC(70157693) SHA1(a7922e2137f95271011042441d80466fba7bb828) )

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD( "tk3000.f4f6",  0x000000, 0x004000, CRC(5b1e8ab2) SHA1(f163e5753c18ff0e812a448e8da406f102600edf) )

	ROM_REGION(0x2000, "kbdcpu", 0)
	ROM_LOAD( "tk3000.e13",   0x000000, 0x002000, CRC(f9b860d3) SHA1(6a127f1458f43a00199d3dde94569b8928f05a53) )

	ROM_REGION(0x800, "keyboard", ROMREGION_ERASE00)
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // probably not this machine's actual ROM
ROM_END

ROM_START(prav8c)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "charrom.d20", 0x0000, 0x2000,CRC(935212cc) SHA1(934603a441c631bd841ea0d2ff39525474461e47))
	ROM_REGION(0x8000,"maincpu",0)
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

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("3420033a.256", 0x0000, 0x8000, CRC(c8b979b3) SHA1(10767e96cc17bad0970afda3a4146564e6272ba1))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(apple2c3)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("342-0445-a.256", 0x0000, 0x8000, CRC(bc5a79ff) SHA1(5338d9baa7ae202457b6500fde5883dbdc86e5d3))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(apple2c4)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("3410445b.256", 0x0000, 0x8000, CRC(06f53328) SHA1(015061597c4cda7755aeb88b735994ffd2f235ca))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(laser128)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("laser128.256", 0x0000, 0x8000, CRC(39e59ed3) SHA1(cbd2f45c923725bfd57f8548e65cc80b13bc18da))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real laser rom
ROM_END

ROM_START(las128ex)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("las128ex.256", 0x0000, 0x8000, CRC(b67c8ba1) SHA1(8bd5f82a501b1cf9d988c7207da81e514ca254b0))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real laser rom
ROM_END

ROM_START(las128e2)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD( "laser 128ex2 rom version 6.1.bin", 0x000000, 0x008000, CRC(7f911c90) SHA1(125754c1bd777d4c510f5239b96178c6f2e3236b) )

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, BAD_DUMP CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // need to dump real laser rom
ROM_END

ROM_START(apple2cp)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("341-0625-a.256", 0x0000, 0x8000, CRC(0b996420) SHA1(1a27ae26966bbafd825d08ad1a24742d3e33557c))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT    MACHINE      INPUT     INIT      COMPANY            FULLNAME */
COMP( 1983, apple2e,  0,        apple2,   apple2e,     apple2e, driver_device,  0,        "Apple Computer",    "Apple //e", MACHINE_SUPPORTS_SAVE )
COMP( 1983, apple2euk,apple2e,  0,        apple2e,     apple2euk,driver_device, 0,        "Apple Computer",    "Apple //e (UK)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, mprof3,   apple2e,  0,        mprof3,      apple2e, driver_device,  0,        "Multitech",         "Microprofessor III", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1985, apple2ee, apple2e,  0,        apple2ee,    apple2e, driver_device,  0,        "Apple Computer",    "Apple //e (enhanced)", MACHINE_SUPPORTS_SAVE )
COMP( 1985, apple2eeuk,apple2e, 0,        apple2ee,    apple2euk, driver_device,0,        "Apple Computer",    "Apple //e (enhanced, UK)", MACHINE_SUPPORTS_SAVE )
COMP( 1987, apple2ep, apple2e,  0,        apple2ep,    apple2ep, driver_device, 0,        "Apple Computer",    "Apple //e (Platinum)", MACHINE_SUPPORTS_SAVE )
COMP( 1984, apple2c,  0,        apple2,   apple2c,     apple2c, driver_device,  0,        "Apple Computer",    "Apple //c" , MACHINE_SUPPORTS_SAVE )
COMP( 1986, tk3000,   apple2c,  0,        tk3000,      apple2e, driver_device,  0,        "Microdigital",      "TK3000//e" , MACHINE_SUPPORTS_SAVE )
COMP( 1989, prav8c,   apple2e,  0,        apple2e,     apple2e, driver_device,  0,        "Pravetz",           "Pravetz 8C", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1987, laser128, apple2c,  0,        laser128,    apple2e, driver_device,  0,        "Video Technology",  "Laser 128 (version 4.2)", MACHINE_SUPPORTS_SAVE )
COMP( 1988, las128ex, apple2c,  0,        laser128,    apple2e, driver_device,  0,        "Video Technology",  "Laser 128ex (version 4.5)", MACHINE_SUPPORTS_SAVE )
COMP( 1988, las128e2, apple2c,  0,        laser128ex2, apple2e, driver_device,  0,        "Video Technology",  "Laser 128ex2 (version 6.1)", MACHINE_SUPPORTS_SAVE )
COMP( 1985, apple2c0, apple2c,  0,        apple2c_iwm, apple2c, driver_device,  0,        "Apple Computer",    "Apple //c (UniDisk 3.5)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2c3, apple2c,  0,        apple2c_mem, apple2c, driver_device,  0,        "Apple Computer",    "Apple //c (Original Memory Expansion)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2c4, apple2c,  0,        apple2c_mem, apple2c, driver_device,  0,        "Apple Computer",    "Apple //c (rev 4)", MACHINE_SUPPORTS_SAVE )
COMP( 1988, apple2cp, apple2c,  0,        apple2cp,    apple2c, driver_device,  0,        "Apple Computer",    "Apple //c Plus", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
