// license:BSD-3-Clause
// copyright-holders:Pascal Gervaix
/***************************************************************************

    HP 150 (Series 100) -- Touchscreen personal computer (1983)

    An 8088 machine with HP's own firmware and OEM MS-DOS; not IBM PC
    compatible. Mass storage is external, on HP-IB.

    Hardware (HP 150 Technical Reference Manual 45625-90001, "TRM" below, sections 3 & 4):
      CPU      Intel 8088-2 @ 8 MHz (8284A from 24 MHz xtal, /3)
      IRQ      Intel 8259A                I/O 0010
      RTC      MM58167A                   I/O 0040-005F
      KBD/TS   Intel 8041A (UPI)          I/O 0018-0019
      HP-IB    TMS9914                    I/O 0004
      Serial   7201 MPSC + 8116T baud     I/O 0000 / 000C
      Video    SMC9007 (CRT9007) CRTC     mem D3000-D30BE (+ VATT latch D30BE)

    Memory map (8088 linear):
      00000-05FFF  firmware RAM / interrupt vectors
      06000-9FFFF  OS + user RAM (640K mapped; 256K standard, 640K maximum)
      B0000-B7FFF  firmware ROM (32K, fifth chip)
      B8000        system status LEDs
      BC000-BC0FF  CMOS config RAM (256x4)
      C0000-C7FFF  graphics RAM (32K)
      D0000-D2FFF  alphanumeric character RAM (12K, row-pointer table)
      D3000-D30BE  CRT9007 registers + VATT latch
      E0000-FFFFF  firmware ROM (128K, four chips)

    Usage:
      mame hp150 -ieee0 hp9133xv -hard <image.chd>
      mame hp150 -ieee0 hp9133xv -flop sysmasta
    The HP-IB slots -ieee0 to -ieee7 are empty by default (the base model
    shipped without a drive). The 9133XV's hard disc is at HP-IB address 0
    and its floppy drive at 2, so the HP 150 boots from the hard disc; to
    boot from a floppy, set the HPIB address of the floppy board to 0 and of
    the Winchester board to 2 in the Machine Configuration menu. The
    touchscreen needs -mouse -lightgun.

    Keys with no PC counterpart: f1-f8 = F1-F8, Clear line / Clear display
    = F9 / F10, Menu = F11, User/System = F12, Stop = Num Lock,
    Reset/Break = Pause, Extend = Left Alt / AltGr.

***************************************************************************/

#include "emu.h"

#include "bus/ieee488/ieee488.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "imagedev/printer.h"
#include "machine/com8116.h"
#include "machine/mm58167.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/tms9914.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "video/crt9007.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "multibyte.h"

#include <algorithm>

namespace {

// Microsoft serial mouse, offered by HP's Windows SETUP for this machine.
void hp150_rs232_devices(device_slot_interface &device)
{
	default_rs232_devices(device);
	device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
}

// Device options for the HP-IB bus attachment points.
void hp150_disk_devices(device_slot_interface &device)
{
	hp_ieee488_devices(device);
	remote488_devices(device);
}

// =================================================================================================
// Integral thermal printer (TPM 2674A) -- an option, not standard equipment.
//
// The interface is soldered on the front plane of every machine (TRM
// section 3): data latch, acknowledge flip-flop, open-collector NOCINT/IR4 line, status register at
// I/O 0x30. Only the printing mechanism is optional: it plugs into a pre-fitted receptacle and
// announces itself through a presence bit ("Bit 1=1 if the TPM is installed"). With an empty slot,
// status bits 1-2 read 0 and no acknowledge ever comes back, so the firmware skips the printer
// self-test.
// =================================================================================================

class device_hp150_tpm_interface : public device_interface
{
public:
	virtual void data_w(uint8_t data) = 0;                // byte handed over by the front-plane latch

protected:
	device_hp150_tpm_interface(const machine_config &mconfig, device_t &device)
		: device_interface(device, "hp150tpm")
	{
	}
};

class hp150_tpm_slot_device : public device_t, public device_single_card_slot_interface<device_hp150_tpm_interface>
{
public:
	template <typename T>
	hp150_tpm_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: hp150_tpm_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	hp150_tpm_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Front-plane view of the receptacle.
	bool installed() const { return m_card != nullptr; }
	bool paper_present() const { return installed(); }   // paper is not modelled: a fitted mechanism has paper
	void data_w(uint8_t data)
	{
		if (m_card)
			m_card->data_w(data);
	}

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_hp150_tpm_interface *m_card;
};

DEFINE_DEVICE_TYPE_PRIVATE(HP150_TPM_SLOT, hp150_tpm_slot_device, hp150_tpm_slot_device, "hp150_tpm_slot", "HP 150 integral printer receptacle")

hp150_tpm_slot_device::hp150_tpm_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP150_TPM_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_hp150_tpm_interface>(mconfig, *this)
	, m_card(nullptr)
{
}

void hp150_tpm_slot_device::device_start()
{
	m_card = get_card_device();
}

// The mechanism itself. Fitting it changes what the front-plane status register reports (installed,
// paper present -- TRM, section 3).
class hp150_tpm2674a_device : public device_t, public device_hp150_tpm_interface
{
public:
	hp150_tpm2674a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void data_w(uint8_t data) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<printer_image_device> m_printer;   // the byte stream, captured to a .prn image
};

DEFINE_DEVICE_TYPE_PRIVATE(HP150_TPM2674A, device_hp150_tpm_interface, hp150_tpm2674a_device, "hp150_tpm2674a", "HP 2674A integral thermal printer")

hp150_tpm2674a_device::hp150_tpm2674a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP150_TPM2674A, tag, owner, clock)
	, device_hp150_tpm_interface(mconfig, *this)
	, m_printer(*this, "printer")
{
}

void hp150_tpm2674a_device::device_start()
{
}

void hp150_tpm2674a_device::device_add_mconfig(machine_config &config)
{
	PRINTER(config, m_printer);
}

void hp150_tpm2674a_device::data_w(uint8_t data)
{
	m_printer->output(data);
}

void hp150_tpm_devices(device_slot_interface &device)
{
	device.option_add("2674a", HP150_TPM2674A);
}

class hp150_state : public driver_device
{
public:
	hp150_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_crtc(*this, "crtc")
		, m_pic(*this, "pic")
		, m_rtc(*this, "rtc")
		, m_hpib(*this, "hpib")
		, m_mpsc(*this, "mpsc")
		, m_brg(*this, "brg")
		, m_vram(*this, "vram")
		, m_gfx(*this, "gfx")
		, m_beep(*this, "beep")
		, m_tpm(*this, "tpm")
		, m_chargen(*this, "chargen")
		, m_touch_x(*this, "TOUCH_X")
		, m_touch_y(*this, "TOUCH_Y")
		, m_screen(*this, "screen")
		, m_nvram(*this, "nvram")
		, m_ieee(*this, IEEE488_TAG)
		, m_port(*this, "port%u", 1U)
	{ }

	void hp150(machine_config &config) ATTR_COLD;

	// Host keyboard -> HP 150 raw key address: positional scan codes read from the real keyboard,
	// not the ISV/Programmer's Reference keycode values.
	DECLARE_INPUT_CHANGED_MEMBER(key_changed);
	// Host pointer -> HP 150 touchscreen IR grid. Reported through the 8041 as tagged bytes, not
	// through a command: row (0-26) tagged 0x4, then screen column (0-79) halved and tagged 0x5.
	DECLARE_INPUT_CHANGED_MEMBER(touch_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// The HP 150 wires the TMS9914 register-select lines in a non-linear order
	// (TRM p.4-14, "HPIB Controller (9914)"): I/O
	// 0004-000b map to 9914 registers {1,5,3,7,0,4,2,6}.
	static constexpr uint8_t HPIB_REG[8] = { 1, 5, 3, 7, 0, 4, 2, 6 };
	static constexpr uint8_t KBD_FIFO = 16;
	// Typematic repeat: "Any key on the keyboard will auto repeat after 500 ms if held down", and
	// command 0x28 stops the current one (TRM p.3-46). Repeat rate ~10 characters/second, measured on a
	// real unit.
	static constexpr attotime KBD_REPEAT_DELAY = attotime::from_msec(500);
	static constexpr attotime KBD_REPEAT_RATE = attotime::from_msec(100);
	// Screen geometry: x in half alpha dots (an alpha dot is 2 pixels, a graphics dot 3), y in scan lines.
	static constexpr uint16_t GFX_W = 512, GFX_H = 390;            // graphics plane, in graphics dots
	static constexpr uint8_t ALPHA_X = 18 * 3, ALPHA_Y = 5;         // alpha area origin
	static constexpr uint8_t ALPHA_COLS = 80, ALPHA_ROWS = 27, CELL_W = 18, CELL_H = 14;   // CELL_W = 9 dots

	required_device<i8088_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<crt9007_device> m_crtc;
	required_device<pic8259_device> m_pic;
	required_device<mm58167_device> m_rtc;
	required_device<tms9914_device> m_hpib;
	required_device<upd7201_device> m_mpsc;   // 7201 MPSC datacomm @ I/O 0000-0003
	required_device<com8116_device> m_brg;    // 8116 baud-rate generator @ I/O 000c
	required_shared_ptr<uint8_t> m_vram;      // alpha character RAM @ d0000 (char+attr pairs)
	required_shared_ptr<uint8_t> m_gfx;       // graphics bitmap RAM @ c0000 (512x390, 1bpp, 64 B/row)
	required_device<beep_device> m_beep;      // bell/keyclick (8041 "beep bell" cmd 0x30-0x3f)
	required_device<hp150_tpm_slot_device> m_tpm;    // front-plane receptacle for the optional printing mechanism
	required_region_ptr<uint8_t> m_chargen;   // character-generator ROM (8 sets x 128 chars x 16 B; attr D2-D0 = set)
	required_ioport m_touch_x, m_touch_y;      // touchscreen pointer, 0-255 across the whole screen (see touch_report)
	required_device<screen_device> m_screen;
	required_device<nvram_device> m_nvram;
	required_device<ieee488_device> m_ieee;
	required_device_array<rs232_port_device, 2> m_port;   // Port 1 = 7201 channel A, Port 2 = channel B

	bool m_touch_down = false;                // contact held: a move must be reported, like the IR scan
	uint8_t m_touch_last_col = 0xff;
	uint8_t m_touch_last_row = 0xff;
	bool m_tpm_irq = false;                    // TPM acknowledge / NOCINT (IR4) pending
	emu_timer *m_beep_off_timer = nullptr;
	uint8_t m_cmos[0x100] = {};
	// Video attribute latch (d30be, TRM p.4-8): D4 DISPON = alpha display on,
	// D0 GREN = graphics display enable. Blink and cursor-shape bits (D3-D1) are not modelled.
	uint8_t m_vatt = 0;
	uint8_t m_crtc_shadow[0x40] = {};          // CRT9007 register read-back (see crtc_r)
	uint8_t m_kbd_resp[KBD_FIFO] = {};         // 8041 data byte FIFO
	uint8_t m_kbd_qual[KBD_FIFO] = {};         // status qualifier (nibble 7-4) per byte
	uint8_t m_kbd_resp_cnt = 0;
	emu_timer *m_kbd_rep_timer = nullptr;
	uint8_t m_kbd_rep_addr = 0xff;             // key address currently repeating (0xff = none)

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void touch_report();                       // queue the current row/col pair to the 8041 FIFO
	TIMER_CALLBACK_MEMBER(beep_off) { m_beep->set_state(0); }

	uint8_t hpib_r(offs_t o)
	{
		return m_hpib->read(HPIB_REG[o & 7]);
	}
	void hpib_w(offs_t o, uint8_t d)
	{
		m_hpib->write(HPIB_REG[o & 7], d);
	}
	// CMOS is 256 x 4 bits: only the low nibble is real RAM; the high nibble reads back as F.
	uint8_t cmos_r(offs_t o) { return 0xf0 | (m_cmos[o] & 0x0f); }
	void cmos_w(offs_t o, uint8_t d) { m_cmos[o] = 0xf0 | (d & 0x0f); }

	// 8116 baud-rate generator @ I/O 000c: one byte selects both port rates -- low nibble = Port 1
	// (7201 channel A), high nibble = Port 2 (channel B); each rate clocks the rx and tx of its own
	// port (TRM section 3, datacomm).
	void brg_w(uint8_t data)
	{
		m_brg->str_w(data & 0x0f);
		m_brg->stt_w(data >> 4);
	}

	// Datacomm control lines, read side (TRM p.4-12): bit 3 = DM (CC), bit 1 = OCR2 (SCF), bit 0 = OCR1 (CE),
	// 1 when the RS-232 line is on (TRM fig. A-6, A-12). The other bits are fixed: 0014 reads 0001 in bits 7-4;
	// 0016 reads 111 in bits 7-5 (as on a real HP 150), 1 in bit 4 (POD, Port 1 card present) and 0 in bit 2
	// (MTST, test jumper). OCR2 is on pin 12, which rs232_port carries as SI. Writes (clock select, OCD
	// outputs) are ignored.
	template <unsigned Port> uint8_t datacomm_r()
	{
		return (Port ? 0xf0 : 0x10)
				| (m_port[Port]->dsr_r() ? 0x00 : 0x08)
				| (m_port[Port]->si_r() ? 0x00 : 0x02)
				| (m_port[Port]->ri_r() ? 0x00 : 0x01);
	}

	void vatt_w(uint8_t data) { m_vatt = data; }

	// CRT9007 registers are wired to address lines A1-A6: register N lives at byte offset N*2.
	//
	// The POST reads the cursor registers back at their write addresses, 0x18/0x19 (the data sheet reads
	// them at 0x38/0x39), and checks them; what the board returns there is not known, so the driver returns
	// the last value written for every register the chip does not read.
	// Live: 0x15/0x16, cursor 0x38/0x39, status 0x3a, light pen 0x3b/0x3c.
	static bool crtc_live_reg(uint8_t r) { return r == 0x15 || r == 0x16 || (r >= 0x38 && r <= 0x3c); }
	uint8_t crtc_r(offs_t offset)
	{
		const uint8_t r = offset >> 1;
		if (crtc_live_reg(r))
			return m_crtc->read(r);
		return m_crtc_shadow[r & 0x3f];
	}
	void crtc_w(offs_t offset, uint8_t data)
	{
		const uint8_t r = offset >> 1;
		m_crtc_shadow[r & 0x3f] = data;
		if (r < 0x3d)                     // the CRT9007 decodes R00-R3C only
			m_crtc->write(r, data);
	}

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// --- 8041A keyboard/touchscreen controller (HLE) ---
	// I/O 0019 = status (R) / command (W); 0018 = data (R/W). UPI-41 semantics (TRM
	// p.3-44..3-47): status bit0=OBF (data ready), bit1=IBF (busy), bits7-4 = qualifier telling the
	// data type -- 0000 = key address (data bit7: 0 down / 1 up), 0001 = power-on.
	// Start-up the firmware expects: self-test (0x26) -> 0xf9, identify (0x21) -> 0x7f/0xff, enable-scan (0x24).
	void kbd_queue(uint8_t b, uint8_t qual = 0x1)
	{
		if (m_kbd_resp_cnt < KBD_FIFO)
		{
			m_kbd_resp[m_kbd_resp_cnt] = b;
			m_kbd_qual[m_kbd_resp_cnt] = qual;
			m_kbd_resp_cnt++;
		}
		else
		{
			logerror("8041 FIFO full, %02x dropped\n", b);
		}
	}
	TIMER_CALLBACK_MEMBER(kbd_repeat)
	{
		if (m_kbd_rep_addr == 0xff)
			return;
		kbd_queue(m_kbd_rep_addr & 0x7f, 0x0); // a repeat is another key-DOWN, never a break
		m_pic->ir3_w(1);
		m_kbd_rep_timer->adjust(KBD_REPEAT_RATE);
	}

	// Inject a key event: key-address (0-127), bit7 = 0 down / 1 up, qualifier 0000.
	void kbd_key(uint8_t addr, bool down)
	{
		kbd_queue((addr & 0x7f) | (down ? 0 : 0x80), 0x0);
		m_pic->ir3_w(1);
		// The last key pressed is the one that repeats; releasing it -- or any other key arriving --
		// ends that repeat, which is what N-key rollover on a single 8041 scan gives.
		if (down)
		{
			m_kbd_rep_addr = addr & 0x7f;
			m_kbd_rep_timer->adjust(KBD_REPEAT_DELAY);
		}
		else if ((addr & 0x7f) == m_kbd_rep_addr)
		{
			m_kbd_rep_addr = 0xff;
			m_kbd_rep_timer->reset();
		}
	}

	uint8_t kbd_status_r()
	{
		const uint8_t qual = m_kbd_resp_cnt ? m_kbd_qual[0] : 0x1;
		return (qual << 4) | (m_kbd_resp_cnt ? 0x01 : 0x00); // IBF always 0: the HLE is never busy
	}
	uint8_t kbd_data_r()
	{
		const uint8_t d = m_kbd_resp[0];
		if (machine().side_effects_disabled())
			return d;
		if (m_kbd_resp_cnt)
		{
			for (int i = 1; i < m_kbd_resp_cnt; i++)
			{
				m_kbd_resp[i - 1] = m_kbd_resp[i];
				m_kbd_qual[i - 1] = m_kbd_qual[i];
			}
			m_kbd_resp_cnt--;
		}
		if (!m_kbd_resp_cnt)
			m_pic->ir3_w(0);                    // OBF cleared -> drop keyboard interrupt
		return d;
	}
	void kbd_cmd_w(uint8_t data)
	{
		// Bell: cmd 0x30-0x3f "beep bell", freq set by the low nibble. The TRM (p.3-47)
		// gives only the two end points -- ~290 Hz at 0x30 and ~1.7 kHz at 0x3f.
		if ((data & 0xf0) == 0x30)
		{
			m_beep->set_clock(290 + (data & 0x0f) * 94);  // linear between the two end points (guessed)
			m_beep->set_state(1);
			m_beep_off_timer->adjust(attotime::from_msec(100));  // duration not documented (guessed)
			return;
		}
		switch (data)
		{
		case 0x10:                                          // release interrupt (ack); re-assert if more queued
			m_pic->ir3_w(m_kbd_resp_cnt ? 1 : 0);
			break;
		case 0x08:                                          // stop bell immediately
			m_beep->set_state(0);
			break;
		case 0x40:                                          // do one keyclick immediately (short tick)
			m_beep->set_clock(2000);                           // pitch and duration guessed
			m_beep->set_state(1);
			m_beep_off_timer->adjust(attotime::from_msec(2));
			break;
		case 0x28:                                          // stop key repeat (TRM p.3-46)
			m_kbd_rep_addr = 0xff;
			m_kbd_rep_timer->reset();
			break;
		case 0x26:                                          // self-test -> pass (no interrupt; polled)
			kbd_queue(0xf9);
			break;
		case 0x21:                                          // identify keyboard -> ID present + interrupt (IR3)
			kbd_queue(0x7f);
			m_pic->ir3_w(1);
			break;
		case 0x60:
			// Touchscreen detector-pair report, a pre-scan diagnostic (p.3-47): the addresses of blocked or dead
			// IRED/phototransistor pairs, ff,ff = none. Not the touch path (see touch_changed); polled, no interrupt.
			kbd_queue(0xff);
			kbd_queue(0xff);
			break;
		default:                                            // enable-scan/reset/keyclick-on/off: no response
			break;
		}
	}

	// Integral thermal printer (TPM 2674A) @ I/O 0x30 (TRM section 3, "Thermal (Integral)
	// Printer Interface"). WRITE = send a data byte to the TPM: it latches the byte, acknowledges,
	// and asserts NOCINT (the open-collector IR4 interrupt). READ = status: bit0 = acknowledge sent
	// (ready for the next byte), bit1 = TPM installed, bit2 = paper present (bits 3-7 unused);
	// reading the status clears the interrupt (NTPMRD rising edge).
	uint8_t printer_r()
	{
		const uint8_t st = (m_tpm->installed() ? 0x02 : 0x00)         // bit1: mechanism fitted
						 | (m_tpm->paper_present() ? 0x04 : 0x00)     // bit2: paper loaded
						 | (m_tpm_irq ? 0x01 : 0x00);                 // bit0: acknowledge received
		if (machine().side_effects_disabled())
			return st;
		if (m_tpm_irq)                                    // reading status clears NOCINT/IR4
		{
			m_tpm_irq = false;
			m_pic->ir4_w(0);
		}
		return st;
	}
	void printer_w(uint8_t data)
	{
		if (!m_tpm->installed())
			return;                 // NACK comes from the mechanism: no card, no acknowledge, no IR4
		m_tpm->data_w(data);
		m_tpm_irq = true;           // TPM acked -> assert NOCINT (IR4); cleared when status is read
		m_pic->ir4_w(1);
	}
};

void hp150_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	// DRAM is mapped by machine_start(); the POST sizes it by a walking-bit test over banks 0x40000..0xa0000.
	map(0xb0000, 0xb7fff).rom().region("firmware1", 0); // ROM 1 (5th firmware ROM, 32K, non-contiguous)
	map(0xb8000, 0xb8000).nopw();                        // system status LEDs (POST codes), not displayed
	map(0xbc000, 0xbc0ff).rw(FUNC(hp150_state::cmos_r), FUNC(hp150_state::cmos_w)); // CMOS config (256x4)
	map(0xc0000, 0xc7fff).mirror(0x8000).ram().share(m_gfx); // 32K graphics bitmap RAM (512x390 1bpp)
	// Alpha RAM (12K) + CRT9007 registers. The video region d0000-d3fff is decoded with
	// the top bits 0xc000 ignored, so it images three more times at d4000/d8000/dc000
	// (TRM memory map: "D4000-DFFFF reserved, Alpha RAM/CRTC image"). The OS uses
	// the image at d6000.
	map(0xd0000, 0xd2fff).mirror(0xc000).ram().share(m_vram); // 12K alphanumeric character RAM (+ images)
	map(0xd3000, 0xd307f).mirror(0xc000).rw(FUNC(hp150_state::crtc_r), FUNC(hp150_state::crtc_w)); // CRT9007 regs (A1-A6 -> *2)
	map(0xd30be, 0xd30be).mirror(0xc000).w(FUNC(hp150_state::vatt_w)); // video attribute latch
	map(0xe0000, 0xfffff).rom().region("firmware", 0); // 128K system firmware ROM
}

void hp150_state::io_map(address_map &map)
{
	map.unmap_value_high();
	// 7201 MPSC datacomm. HP 150 port decode: A0=control/data, A1=channel.
	// Port 0=chA data, 1=chA ctrl, 2=chB data, 3=chB ctrl.
	map(0x0000, 0x0003).rw(m_mpsc, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w));
	map(0x0004, 0x000b).rw(FUNC(hp150_state::hpib_r), FUNC(hp150_state::hpib_w)); // HP-IB TMS9914
	map(0x000c, 0x000c).w(FUNC(hp150_state::brg_w));                              // 8116 baud-rate generator
	map(0x0010, 0x0011).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));   // 8259A PIC (vectors INT 40h-47h)
	map(0x0014, 0x0014).r(FUNC(hp150_state::datacomm_r<0>)).nopw();       // Port 1 control lines
	map(0x0016, 0x0016).r(FUNC(hp150_state::datacomm_r<1>)).nopw();       // Port 2 control lines
	map(0x0018, 0x0018).r(FUNC(hp150_state::kbd_data_r)).nopw();   // 8041 data (read-only: no command takes an argument)
	map(0x0019, 0x0019).rw(FUNC(hp150_state::kbd_status_r), FUNC(hp150_state::kbd_cmd_w));  // 8041 status / command
	map(0x0030, 0x0030).rw(FUNC(hp150_state::printer_r), FUNC(hp150_state::printer_w)); // integral thermal printer (TPM): status (R) / data (W)
	map(0x0040, 0x005f).rw(m_rtc, FUNC(mm58167_device::read), FUNC(mm58167_device::write));   // RTC MM58167A
}

void hp150_state::machine_start()
{
	// DRAM: 256K on the mezzanine PCA, plus at most one memory board in an accessory slot
	// (128K/256K/384K, HP part numbers 45630A/45631A/45632A) -- hence the four sizes offered.
	m_maincpu->space(AS_PROGRAM).install_ram(0x00000, m_ram->size() - 1, m_ram->pointer());

	m_beep_off_timer = timer_alloc(FUNC(hp150_state::beep_off), this);
	m_kbd_rep_timer  = timer_alloc(FUNC(hp150_state::kbd_repeat), this);

	m_nvram->set_base(&m_cmos[0], sizeof(m_cmos));

	save_item(NAME(m_cmos));
	save_item(NAME(m_vatt));
	save_item(NAME(m_crtc_shadow));
	save_item(NAME(m_tpm_irq));
	save_item(NAME(m_kbd_resp));
	save_item(NAME(m_kbd_qual));
	save_item(NAME(m_kbd_resp_cnt));
	save_item(NAME(m_kbd_rep_addr));
	save_item(NAME(m_touch_down));
	save_item(NAME(m_touch_last_col));
	save_item(NAME(m_touch_last_row));
}

void hp150_state::machine_reset()
{
	// PIC IR2 is hard-wired to +5V (TRM p.3-19: "IR2 +5V, Video second
	// level tasks") -> permanently asserted. The firmware uses it as a software
	// interrupt: it unmasks IR2 and the request fires immediately.
	m_pic->ir2_w(1);

	// Reset is a power-on for the 8041 (TRM p.3-45): output buffer empty, scanning stopped, no key
	// left repeating. The PIC has just dropped IR3/IR4, so nothing pending may survive either.
	m_kbd_resp_cnt = 0;
	m_kbd_rep_addr = 0xff;
	m_kbd_rep_timer->reset();
	m_touch_last_col = m_touch_last_row = 0xff;   // a finger still held is reported again on its next move
	m_tpm_irq = false;                              // TPM acknowledge flip-flop
}

// Keyboard: a host key press sends the 8041 key address of the HP 150 key at the same position.
// The addresses were read off the hardware.
INPUT_CHANGED_MEMBER(hp150_state::key_changed)
{
	const bool make = (newval != 0);

	const uint8_t raw = param & 0x7f;

	kbd_key(raw, make);
}

// HP 150 touchscreen (27x40 IR beam grid, HLE). Protocol from the TRM section 3
// (p.3-44..3-49): a touch is reported on the normal 8041 data FIFO (port 0018) with a status qualifier
// (port 0019 bits 7-4) -- a pair [row addr, 0100][col addr, 0101], then an 0011 "release" code (data 0)
// when the scan finds no touch. Grid = 27 rows x 40 columns -> row = screen row (0-26), col reported =
// screen column / 2; the firmware scales to the AGIOS 0-79 x 0-47 space. Command 0x60 is a pre-scan
// dead-pair diagnostic, not this path (see kbd_cmd_w). Reporting is IR3-driven like a key event.
// param 0 = the contact itself, param 1 = a coordinate moved. A finger that slides keeps the beams
// broken at a new spot, so the grid reports the new row/col.
INPUT_CHANGED_MEMBER(hp150_state::touch_changed)
{
	if (param != 0)
	{
		if (m_touch_down)
			touch_report();
		return;
	}
	m_touch_down = (newval != 0);
	if (m_touch_down)
	{
		touch_report();
	}
	else
	{
		m_touch_last_col = m_touch_last_row = 0xff;
		kbd_queue(0x00, 0x3);       // 0011 = touchscreen release, data 0
		m_pic->ir3_w(1);
	}
}

uint32_t hp150_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// The HP 150 drives the CRT9007 in row-table addressing mode (address mode 2): TABLE_START
	// (regs 0x0c/0x0d, in char-cell units) points to a per-row table in the alpha RAM, and each
	// row's 80 char+attribute cells are fetched from the cell address held in its entry. Entries
	// are 2 cells (4 bytes): cell addr = low-cell char byte | (high-cell char byte << 8).
	// Glyphs come from the real character generator (region "chargen"), Roman8 codes.
	// Approximate renderer, not pixel-exact.
	bitmap.fill(rgb_t::black(), cliprect);

	// Graphics plane: 512x390, 1 bit/pixel, 64 bytes/row, at c0000. A graphics dot is 1.5 alpha dots
	// wide; the graphics area overhangs the 720x378 alpha area by about 6 scan lines at the top and 16-17
	// graphics dots on the left (TRM fig. 3-14); 5 lines and 18 dots, calibrated against a real unit.
	constexpr rgb_t FULL = rgb_t::white();
	constexpr rgb_t HALF = rgb_t(0xa6, 0xa6, 0xa6);   // half-bright, ~65%, estimated from photos of a real unit

	// The screen is the graphics area in half alpha dots, so that the char-gen half-dot shift can be
	// drawn.
	// graphics bytes are LSB-first (bit0 = leftmost pixel), opposite of the MSB-first alpha char-gen
	const auto gfx_dot = [this] (int x, int y)
	{
		const int gx = x / 3;
		return BIT(m_vatt, 0) && gx < GFX_W && y < GFX_H && BIT(m_gfx[(uint32_t(y) * (GFX_W / 8) + (gx >> 3)) & 0x7fff], gx & 7);
	};
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			if (gfx_dot(x, y))
				dst[x] = FULL;
		}
	}

	// DISPON off blanks the whole alpha plane, softkey labels and cursor included.
	if (!BIT(m_vatt, 4))
		return 0;

	constexpr uint32_t VSIZE = 0x3000;        // 12K alpha RAM
	const uint16_t table_start = get_u16le(&m_crtc_shadow[0x0c]) & 0x3fff;
	for (int row = 0; row < ALPHA_ROWS; row++)
	{
		const uint32_t te  = (uint32_t(table_start) * 2 + row * 4) % VSIZE;       // low-cell char byte
		const uint32_t teh = (te + 2) % VSIZE;                                    // high-cell char byte
		// Row pointer = 14-bit word offset from d0000 (TRM section 7); bits 14-15 select
		// double-height/width, unsupported by HP 150 HW.
		const uint16_t cell = (m_vram[te] | (uint16_t(m_vram[teh]) << 8)) & 0x3fff; // row start, in cells
		for (int col = 0; col < ALPHA_COLS; col++)
		{
			const uint32_t cb = (uint32_t(cell) * 2 + col * 2) % VSIZE;           // char byte of this cell
			const uint8_t raw = m_vram[cb];
			// Bit 7 of the character byte = end-of-line marker (TRM section 7,
			// "Character Code Structure"): when set, the video hardware blanks the rest of the
			// line. The firmware clears, scrolls and redraws by writing that marker.
			if (raw & 0x80)
				break;
			const uint8_t code = raw & 0x7f;
			// Attribute / char-set byte (odd byte of the cell pair, TRM section 7,
			// "Enhancement/Character Set Byte"): D4 = inverse video, D5 = underline, D6 = half-bright.
			// D7=security and D3=blink are not modelled.
			const uint8_t attr = m_vram[(cb + 1) % VSIZE];
			const bool inverse = BIT(attr, 4);
			const bool underline = BIT(attr, 5);
			const rgb_t fg = BIT(attr, 6) ? HALF : FULL;   // D6 half-bright
			// Control codes 0x00-0x1f are not displayed in normal mode (the char-gen holds their
			// mnemonic glyphs only for DISPLAY FUNCTIONS mode).
			const bool blank = (code < 0x20);
			// Char-gen is 8 sets x 128 chars x 16 bytes. The attribute byte's char-set code (D2-D0)
			// selects the set: 0=Roman, 1=Roman Extension (accents), 2=Line Drawing, 3=Math,
			// 4-7=bold/italic variants.
			const uint8_t *glyph = &m_chargen[((attr & 0x07) * 128 + code) * 16];
			const bool line_draw = (attr & 0x07) == 2;
			for (int gy = 0; gy < CELL_H; gy++)
			{
				const uint8_t b = blank ? 0xff : glyph[gy];
				// underline = scan line 13 of the 14-line cell, index 12 (TRM p. 3-28)
				const bool ul_line = underline && gy == 12;
				const int y = ALPHA_Y + row * CELL_H + gy;
				if (y > cliprect.bottom())
					break;
				if (y < cliprect.top())
					continue;
				uint32_t *const dst = &bitmap.pix(y);
				for (int gx = 0; gx < CELL_W; gx++)
				{
					const int x = ALPHA_X + col * CELL_W + gx;
					if (x < cliprect.left() || x > cliprect.right())
						continue;
					// glyph dot on = cols 0-6, inverted polarity (cleared bit = on); cols 7-8 = gap. Bit 7 set
					// shifts the row right by half a dot (TRM section 3, character cell).
					// Line Drawing repeats col 6 across the gap so rules join (observed on a real unit).
					const int d = gx - BIT(b, 7);
					const int dot = (line_draw && (d >> 1) > 6) ? 6 : (d >> 1);
					const bool pix_on = d >= 0 && dot < 7 && !BIT(b, 6 - dot);
					// Dot stream mixing (TRM fig. A-8, U715): inverse video inverts
					// alpha OR graphics, so graphics drawn inside an inverse field shows dark.
					const bool on = pix_on || ul_line || gfx_dot(x, y);
					dst[x] = (inverse ? !on : on) ? fg : rgb_t::black();
				}
			}
		}
	}

	// Text cursor: the firmware positions the CRT9007 hardware cursor via reg 0x18 (row) and
	// reg 0x19 (column).
	const uint8_t cr = m_crtc_shadow[0x18];
	const uint8_t cc = m_crtc_shadow[0x19];
	if (cr < ALPHA_ROWS && cc < ALPHA_COLS && BIT(screen.frame_number(), 4))   // ~2 Hz blink, rate guessed
	{
		for (int gy = 0; gy < CELL_H; gy++)
		{
			const int y = ALPHA_Y + cr * CELL_H + gy;
			if (y > cliprect.bottom())
				break;
			if (y < cliprect.top())
				continue;
			uint32_t *const dst = &bitmap.pix(y);
			for (int gx = 0; gx < CELL_W; gx++)
			{
				const int x = ALPHA_X + cc * CELL_W + gx;
				if (x >= cliprect.left() && x <= cliprect.right())
					dst[x] = (dst[x] == rgb_t::black()) ? FULL : rgb_t::black();
			}
		}
	}
	return 0;
}

void hp150_state::touch_report()
{
	// The lightgun range spans the whole screen (graphics area); the touch grid covers the alpha area only.
	const uint8_t col = std::clamp((int(m_touch_x->read()) * GFX_W * 3 / 256 - ALPHA_X) / CELL_W, 0, ALPHA_COLS - 1);
	const uint8_t row = std::clamp((int(m_touch_y->read()) * GFX_H / 256 - ALPHA_Y) / CELL_H, 0, ALPHA_ROWS - 1);
	if (col == m_touch_last_col && row == m_touch_last_row)
		return;                     // same cell: a rescan has nothing new to report
	m_touch_last_col = col;
	m_touch_last_row = row;
	kbd_queue(row,      0x4);   // 0100 = touchscreen row address (0-26)
	kbd_queue(col >> 1, 0x5);   // 0101 = touchscreen column address (= screen col / 2)
	m_pic->ir3_w(1);
}

// The firmware picks the character set from the "Keyboard" field of the CMOS; the PORT_CHARs below
// are the US layout.
INPUT_PORTS_START( hp150 )
	PORT_START("kb_fkeys")   // function row (R1) + Menu/System/Stop/Reset + delete keys
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x3d)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x3e)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x3f)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x38)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x0e)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x0d)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x0c)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x0b)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("User/System") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x0f)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Menu") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x08)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x3a)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset/Break") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(PAUSE)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x57)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear line") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x0a)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear display") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x09)

	// Key names per the "Keyboard Key Position Numbers" figure, TRM p.7-33.
	PORT_START("kb_nav")     // cursor + navigation column (Select / page / line / char)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x2a)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x32)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x33)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x31)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Select") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x2b)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Prev") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x21)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Next") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x29)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete line") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x11)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Insert line") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x12)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Insert char") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x1a)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete char") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x19)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Print") PORT_CODE(KEYCODE_PRTSCR) PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x72)

	PORT_START("kb_edit")    // editing / control / modifiers
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x23)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x49)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x13)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC (Del)") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x71)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x37)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x43)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x3b)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift L") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x41)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift R") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x42)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Extend (left)") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x36)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Extend (right)") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x35)

	PORT_START("kb_alpha1")  // letters A-M
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x53)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x77)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x75)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x55)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x4c)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x56)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x39)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x50)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x18)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x20)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x27)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x26)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x28)

	PORT_START("kb_alpha2")  // letters N-Z
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x70)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x1f)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x1e)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x4a)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x4d)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x54)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x4e)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x48)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x76)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x4b)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x74)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x4f)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x73)

	PORT_START("kb_digits")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x2c)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x51)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x44)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x45)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x46)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x47)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x40)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x10)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x17)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x16)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x3c)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x15)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x14)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x1d)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x1c)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x1b)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x25)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x24)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x2f)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x2e)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x2d)

	PORT_START("kb_numpad")  // numeric keypad (raw codes captured on hardware)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x6e)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x66)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x64)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x62)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x5e)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x5c)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x5a)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x5f)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x5d)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x5b)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x6a)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x60)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x58)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP Send") PORT_CODE(KEYCODE_EQUALS_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x59)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x63)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x61)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x67)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::key_changed), 0x65)

	// Touchscreen (27x40 IR beam grid): 0-255 across the whole screen, scaled to a cell by touch_report().
	PORT_START("TOUCH_X")
	PORT_BIT(0xff, 128, IPT_LIGHTGUN_X) PORT_MINMAX(0, 255) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::touch_changed), 1)
	PORT_START("TOUCH_Y")
	PORT_BIT(0xff, 128, IPT_LIGHTGUN_Y) PORT_MINMAX(0, 255) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(9) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::touch_changed), 1)
	PORT_START("TOUCH_BTN")
	// Mouse or gun button only: the default codes include Left Ctrl, which is the HP Ctrl key.
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch screen") PORT_CODE(MOUSECODE_BUTTON1) PORT_CODE(GUNCODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp150_state::touch_changed), 0)
INPUT_PORTS_END

void hp150_state::hp150(machine_config &config)
{
	I8088(config, m_maincpu, 24_MHz_XTAL / 3);   // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &hp150_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hp150_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

	// HP 150 IRQ map (TRM p.3-19): IR0=Video(retrace),
	// IR1=datacomm, IR2=video-2nd, IR3=keyboard/touch, IR4=printer, IR5=HP-IB,
	// IR6=unused, IR7=RTC.
	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	// Video : SMC CRT9007 (VPAC) @ d3000 (regs on A1-A6). Its single INT output is
	// the vertical-retrace interrupt -> IR0 (highest priority, TRM p.3-19).
	// Dot clock 25.7715 MHz, 9 dots per character, 115 characters x 415 lines (TRM p.3-29). The screen
	// counts half dots. The visible area is placed at the start of each line and frame; the real blanking
	// positions are not modelled.
	SCREEN(config, m_screen).set_color(rgb_t::green());
	m_screen->set_raw(25.7715_MHz_XTAL * 2, 2070, 0, GFX_W * 3, 415, 0, GFX_H);
	m_screen->set_screen_update(FUNC(hp150_state::screen_update));

	CRT9007(config, m_crtc, 25.7715_MHz_XTAL / 9);
	m_crtc->set_character_width(18);
	m_crtc->set_screen(m_screen);
	m_crtc->int_callback().set(m_pic, FUNC(pic8259_device::ir0_w));

	// Datacomm : 7201 MPSC (upd7201) @ I/O 0000-0003, clocked by an 8116 baud-rate
	// generator @ I/O 000c (5.0688 MHz crystal).
	UPD7201(config, m_mpsc, 24_MHz_XTAL / 6);
	m_mpsc->out_int_callback().set(m_pic, FUNC(pic8259_device::ir1_w));

	// 8116 baud generator @ I/O 000c: one rate per port (see brg_w).
	COM8116(config, m_brg, 5.0688_MHz_XTAL);
	m_brg->fr_handler().set(m_mpsc, FUNC(upd7201_device::rxca_w));     // low nibble -> Port 1 (chA) rx
	m_brg->fr_handler().append(m_mpsc, FUNC(upd7201_device::txca_w));  //                        + tx
	m_brg->ft_handler().set(m_mpsc, FUNC(upd7201_device::rxcb_w));     // high nibble -> Port 2 (chB) rx
	m_brg->ft_handler().append(m_mpsc, FUNC(upd7201_device::txcb_w));  //                        + tx

	// The two physical RS-232 connectors (TRM: "channel A -> Port 1, channel B -> Port 2").
	// The COM1/COM2 <-> Port1/Port2 mapping and the per-port serial params (baud/bits/parity) are set
	// by the native DEVCONFG utility via CMOS.
	m_mpsc->out_txda_callback().set(m_port[0], FUNC(rs232_port_device::write_txd));   // Port 1 = channel A (RS-232/422)
	m_mpsc->out_dtra_callback().set(m_port[0], FUNC(rs232_port_device::write_dtr));
	m_mpsc->out_rtsa_callback().set(m_port[0], FUNC(rs232_port_device::write_rts));
	RS232_PORT(config, m_port[0], hp150_rs232_devices, nullptr);
	m_port[0]->rxd_handler().set(m_mpsc, FUNC(upd7201_device::rxa_w));
	m_port[0]->dcd_handler().set(m_mpsc, FUNC(upd7201_device::dcda_w));
	m_port[0]->cts_handler().set(m_mpsc, FUNC(upd7201_device::ctsa_w));

	m_mpsc->out_txdb_callback().set(m_port[1], FUNC(rs232_port_device::write_txd));   // Port 2 = channel B (RS-232, peripherals)
	m_mpsc->out_dtrb_callback().set(m_port[1], FUNC(rs232_port_device::write_dtr));
	m_mpsc->out_rtsb_callback().set(m_port[1], FUNC(rs232_port_device::write_rts));
	RS232_PORT(config, m_port[1], hp150_rs232_devices, nullptr);
	m_port[1]->rxd_handler().set(m_mpsc, FUNC(upd7201_device::rxb_w));
	m_port[1]->dcd_handler().set(m_mpsc, FUNC(upd7201_device::dcdb_w));
	m_port[1]->cts_handler().set(m_mpsc, FUNC(upd7201_device::ctsb_w));

	// HP-IB (disk) : TMS9914 @ 4 MHz (24 MHz xtal /6, the system's peripheral clock).
	TMS9914(config, m_hpib, 24_MHz_XTAL / 6);
	m_hpib->dio_read_cb().set(m_ieee, FUNC(ieee488_device::dio_r));
	m_hpib->dio_write_cb().set(m_ieee, FUNC(ieee488_device::host_dio_w));
	m_hpib->eoi_write_cb().set(m_ieee, FUNC(ieee488_device::host_eoi_w));
	m_hpib->dav_write_cb().set(m_ieee, FUNC(ieee488_device::host_dav_w));
	m_hpib->nrfd_write_cb().set(m_ieee, FUNC(ieee488_device::host_nrfd_w));
	m_hpib->ndac_write_cb().set(m_ieee, FUNC(ieee488_device::host_ndac_w));
	m_hpib->ifc_write_cb().set(m_ieee, FUNC(ieee488_device::host_ifc_w));
	m_hpib->srq_write_cb().set(m_ieee, FUNC(ieee488_device::host_srq_w));
	m_hpib->atn_write_cb().set(m_ieee, FUNC(ieee488_device::host_atn_w));
	m_hpib->ren_write_cb().set(m_ieee, FUNC(ieee488_device::host_ren_w));
	m_hpib->int_write_cb().set(m_pic, FUNC(pic8259_device::ir5_w));

	IEEE488(config, m_ieee);
	m_ieee->eoi_callback().set(m_hpib, FUNC(tms9914_device::eoi_w));
	m_ieee->dav_callback().set(m_hpib, FUNC(tms9914_device::dav_w));
	m_ieee->nrfd_callback().set(m_hpib, FUNC(tms9914_device::nrfd_w));
	m_ieee->ndac_callback().set(m_hpib, FUNC(tms9914_device::ndac_w));
	m_ieee->ifc_callback().set(m_hpib, FUNC(tms9914_device::ifc_w));
	m_ieee->srq_callback().set(m_hpib, FUNC(tms9914_device::srq_w));
	m_ieee->atn_callback().set(m_hpib, FUNC(tms9914_device::atn_w));
	m_ieee->ren_callback().set(m_hpib, FUNC(tms9914_device::ren_w));
	// One HP-IB connector, devices daisy-chained: one attachment point per bus address, all empty by
	// default (the base model shipped without a drive, HP UK price list, April 1984). Each drive takes
	// its address from its own switches (Winchester 0, flexible disc 2 -- User's Manual 09133-90010
	// fig. 2-3); the firmware boots from the address in the CMOS "Op Sys Dev" field.
	for (int i = 0; i < 8; i++)
		IEEE488_SLOT(config, util::string_format("ieee%d", i).c_str(), i, hp150_disk_devices, nullptr);

	// Real-time clock (MM58167A) @ I/O 0040-005f, 32.768 kHz crystal. IRQ -> IR7.
	MM58167(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_pic, FUNC(pic8259_device::ir7_w));

	// Battery-backed CMOS config RAM (256x4) @ bc000.
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);   // defaults come from the "nvram" region

	// 256K standard on every SKU; one accessory slot may take a single memory board -- 128K (45630A),
	// 256K (45631A) or 384K (45632A), "only one expansion slot can be used for additional memory"
	// (HP UK price list, April 1984).
	RAM(config, m_ram).set_default_size("256K").set_extra_options("384K,512K,640K");

	// Bell / keyclick : the 8041 drives a beeper (TRM p.3-47, cmd 0x30-0x3f).
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 0).add_route(ALL_OUTPUTS, "mono", 0.50);

	// Receptacle for the optional integral printer (I/O 0x30); -tpm 2674a fits it and its .prn capture.
	HP150_TPM_SLOT(config, m_tpm, hp150_tpm_devices, nullptr);

	SOFTWARE_LIST(config, "flop_list").set_original("hp150_flop");
}

ROM_START( hp150 )
	// Each chip carries its part number and a CRC-16 trailer checked by the power-on test. The HP 150 II
	// set boots on this machine; HP documents the earlier REV E/F sets as HP 150 compatible
	// (HP 150 TRM Supplement 45625-90002, p.6-1).
	ROM_DEFAULT_BIOS("revd")
	ROM_SYSTEM_BIOS(0, "revd", "Rev. D")
	ROM_SYSTEM_BIOS(1, "150ii", "HP 150 II")

	// e0000-fffff, four 32K chips on the memory board
	ROM_REGION(0x20000, "firmware", 0)
	ROMX_LOAD("1818-3324.u64", 0x00000, 0x8000, CRC(500347b6) SHA1(b956abc45b9aa82486fe0128dec7d29b8de81e47), ROM_BIOS(0))
	ROMX_LOAD("1818-3437.u63", 0x08000, 0x8000, CRC(a8064faa) SHA1(aebab0fa62b1c5179c43f1260e720a43fe5b3f3e), ROM_BIOS(0))
	ROMX_LOAD("1818-3436.u62", 0x10000, 0x8000, CRC(48d2107e) SHA1(57ad5c2f27541d35c1843eca0fd6ef01f26debd9), ROM_BIOS(0))
	ROMX_LOAD("1818-3435.u61", 0x18000, 0x8000, CRC(12bd88a9) SHA1(32e7c5aa3819fe95502fd9015a9e38f747a95f57), ROM_BIOS(0))
	ROMX_LOAD("1818-3952.u64", 0x00000, 0x8000, CRC(e4ba0da5) SHA1(0766f8171e8eb08793e2975b15db81573c3e227d), ROM_BIOS(1))
	ROMX_LOAD("1818-3953.u63", 0x08000, 0x8000, CRC(d8249adc) SHA1(9dc827fdb7b8e4f0bee2a15244113b598bf0fcd5), ROM_BIOS(1))
	ROMX_LOAD("1818-3954.u62", 0x10000, 0x8000, CRC(996d9bc9) SHA1(694e044ecadd26149e0dee736d20a4ad8a084143), ROM_BIOS(1))
	ROMX_LOAD("1818-3955.u61", 0x18000, 0x8000, CRC(db9c17a8) SHA1(cc864bbbdddc8167425f54de6ba43377779331d2), ROM_BIOS(1))

	// b0000-b7fff, the fifth chip (TRM ROM DECODING p.3-59)
	ROM_REGION(0x8000, "firmware1", 0)
	ROMX_LOAD("1818-3325.u65", 0x0000, 0x8000, CRC(ea4eb924) SHA1(961203d8cad20abf451dc98f439162b430ff249f), ROM_BIOS(0))
	ROMX_LOAD("1818-3951.u65", 0x0000, 0x8000, CRC(9d09e4fd) SHA1(de01d634cbce21e2d0611033606f8a8cddd31ece), ROM_BIOS(1))

	// Character generator (16K) on the video card, not CPU-mapped.
	// 8 sets x 128 chars x 16 bytes/char; set 0 = USASCII/Roman8.
	// Per char: rows 1..14 used; bit 7 of a row = half-pixel shift (not a pixel); bits 6..0 = the
	// 7 pixels left-to-right, inverted polarity (bit cleared = pixel on).
	ROM_REGION(0x4000, "chargen", 0)
	ROM_LOAD("1818-3328.u513", 0x0000, 0x4000, CRC(7f1c1d3e) SHA1(75d0f282d5776486d9e33af9485611f76aab9f62))

	// 8041A keyboard/touchscreen controller (U510): mask ROM, emulated by HLE.
	ROM_REGION(0x400, "kbdc", 0)
	ROM_LOAD("8041a.u510", 0x000, 0x400, NO_DUMP)

	// Default CMOS, dumped from a real HP 150 set to its configuration defaults (Computer mode, US
	// keyboard, Op Sys Dev = HP-IB 0); POST compacts its records, so fields have no fixed offset.
	ROM_REGION(0x100, "nvram", 0)
	ROM_LOAD("hp150_nvram.bin", 0x0000, 0x100, CRC(99be09da) SHA1(08d59127d42324be257dd1c150e15425a3c8dba7))
ROM_END

} // anonymous namespace

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY            FULLNAME  FLAGS
COMP( 1983, hp150, 0,      0,      hp150,   hp150, hp150_state,  empty_init, "Hewlett-Packard", "HP 150", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_CONTROLS )
