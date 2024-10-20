// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// ***************************************
// Driver for HP 64000 development system
// ***************************************
//
// Documentation used for this driver:
// [1]  HP, manual 64100-90910, dec 83 rev. - Model 64100A mainframe service manual
// [2]  HP, manual 64941-90902, apr 83 rev. - Model 64941A Flexible disc (Floppy) drive
//                                            controller service manual
//
// A 64100A system ("mainframe" in HP docs) is built around a 13 slot card cage.
// The first 4 slots are reserved for specific card types:
// J1   I/O card
// J2   Display and RAM card
// J3   CPU card
// J4   Floppy interface card
//
// The rest of the slots are for CPU emulators, logic analyzers and so on (i.e. those
// cards doing the main functions of a development system).
// This driver emulates the first 4 cards only.
//
// All cards are interconnected by 2 separate buses originating from the CPU:
// memory (16-bit data & 16-bit addresses) and I/O (16-bit data and 6-bit addresses) buses.
// The addresses on I/O bus are split in a 4-bit PA (peripheral address) and a 2-bit IC
// (register address). See also HP_MAKE_IOADDR.
// For the address mapping on the memory bus see [1] pg 229.
// Reading the schematics is complicated by the fact that all data & address
// lines of the buses are inverted.
//
// A brief description of each emulated card follows.
//
// **********
// CPU card (64100-66521 or 64100-66532)
//
// This board holds the HP custom CPU with its massive heatsink, the BIOS roms and little else.
// U30      5061-3011   HP "hybrid" CPU @ 6.25 MHz
// U8
// U9
// U10
// U11
// U18
// U19
// U20
// U21      2732        16kw of BIOS EPROMs
//
// **********
// I/O card (64100-66520)
//
// This board has most of the I/O circuits of the system.
// It interfaces:
// - Keyboard
// - RS232 line
// - IEEE-488/HP-IB bus
// - Miscellaneous peripherals (watchdog, beeper, interrupt registers, option DIP switches)
//
// Emulation of beeper sound is far from correct: it should be a 2500 Hz tone inside an
// exponentially decaying envelope (a bell sound) whereas in the emulation it's inside a
// simple rectangular envelope.
//
// U20      HP "PHI"    Custom HP-IB interface microcontroller
// U28      i8251       RS232 UART
//
// **********
// Display card (64100-66530)
//
// This card has the main DRAM of the system (64 kw) and the CRT controller that generates
// the video image.
// The framebuffer is stored in the main DRAM starting at a fixed location (0xf9f0) and it is
// fed into the CRTC by a lot of discrete TTL ICs. The transfer of framebuffer from DRAM to
// CRTC is designed to refresh the whole DRAM in parallel. For some mysterious reason the first
// display row is always blanked (its 40 words of RAM are even used for the stack!).
//
// U33      i8275       CRT controller
// U60      2716        Character generator ROM
// U23-U30
// U38-U45  HM4864      64 kw of DRAM
//
// **********
// Floppy I/F card (64941-66501)
//
// This card is optional. It interfaces 2 5.25" double-side double-density floppy drives.
// The interfacing between the 16-bit CPU and the 8-bit FDC (WD1791) is quite complex. It is
// based around a FSM that sequences the access of DMA or CPU to FDC. This FSM is implemented
// by 2 small PROMs for which no dump (AFAIK) is available.
// I tried to reverse engineer the FSM by looking at the schematics and applying some sensible
// assumptions. Then I did a sort of "clean room" re-implementation. It appears to work correctly.
//
// U4       FD1791A     Floppy disk controller
//
// A brief summary of the reverse-engineered interface of this card follows.
//
// IC Content
// ==========
// 0  DMA transfers, all words in a block but the last one
// 1  Floppy I/F register, detailed below
// 2  DMA transfers, last word in a block
// 3  Diagnostic registers (not emulated)
//
// Floppy I/F register has 2 formats, one for writing and one for reading.
// Reading this register should always be preceded by a write that starts
// the read operation (bit 11 = 0: see below).
//
// Floppy I/F register format when writing:
// Bit Content
// ===========
// 15  Clear interrupts (1)
// 14  Direction of DMA transfers (1 = write to FDC, 0 = read from FDC)
// 13  DMA enable (1)
// 12  Reset FDC (1)
// 11  Direction of access to FDC/drive control (1 = write, 0 = read)
// 10  Access to either FDC (1) or drive control (0): this selects the
//     content of lower byte (both when writing and reading)
//  9  ~A1 signal of FDC
//  8  ~A0 signal of FDC
//
// 7-0 FDC data (when bit 10 = 1)
// 7-0 Drive control (when bit 10 = 0)
//
// Floppy I/F register format when reading:
// Bit Content
// ===========
// 15  Interrupt from FDC pending (1)
// 14  Interrupt from DMA pending (1)
// 13  Drive 1 media changed (1)
// 12  Drive 1 write protected (1)
// 11  Drive 1 ready (0)
// 10  Drive 0 media changed (1)
//  9  Drive 0 write protected (1)
//  8  Drive 0 ready (0)
//
// 7-0 FDC data (when bit 10 = 1)
// 7-0 Drive control (when bit 10 = 0)
//
// Drive control register
// Bit Content
// ===========
//  7  Floppy side selection
//  6  N/U
//  5  Reset drive 1 media change (1)
//  4  Enable drive 1 motor (0)
//  3  Enable drive 1 (0)
//  2  Reset drive 0 media change (1)
//  1  Enable drive 0 motor (0)
//  0  Enable drive 0 (0)
//

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/hphybrid/hphybrid.h"
#include "imagedev/floppy.h"
#include "machine/74123.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "sound/beep.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/phi.h"
#include "bus/ieee488/ieee488.h"


namespace {

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

class hp64k_state : public driver_device
{
public:
	hp64k_state(const machine_config &mconfig, device_type type, const char *tag);

	void hp64k(machine_config &config);

private:
	virtual void driver_start() override;
	//virtual void machine_start();
	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t hp64k_crtc_filter(uint8_t data);
	void hp64k_crtc_w(offs_t offset, uint16_t data);
	void hp64k_crtc_drq_w(int state);
	void hp64k_crtc_vrtc_w(int state);

	I8275_DRAW_CHARACTER_MEMBER(crtc_display_pixels);

	uint16_t hp64k_rear_sw_r();

	uint8_t int_cb(offs_t offset);
	void hp64k_update_irl(void);
	void hp64k_irl_mask_w(uint16_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(hp64k_kb_scan);
	uint16_t hp64k_kb_r();

	TIMER_DEVICE_CALLBACK_MEMBER(hp64k_line_sync);
	uint16_t hp64k_deltat_r();
	void hp64k_deltat_w(uint16_t data);

	uint16_t hp64k_slot_r(offs_t offset);
	void hp64k_slot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void hp64k_slot_sel_w(offs_t offset, uint16_t data);

	uint16_t hp64k_flp_r(offs_t offset);
	void hp64k_flp_w(offs_t offset, uint16_t data);
	void hp64k_flp_drq_w(int state);
	void hp64k_flp_intrq_w(int state);
	void hp64k_update_floppy_dma(void);
	void hp64k_update_floppy_irq(void);
	void hp64k_update_drv_ctrl(void);
	void hp64k_floppy0_rdy(int state);
	void hp64k_floppy1_rdy(int state);
	void hp64k_floppy_idx_cb(floppy_image_device *floppy , int state);
	void hp64k_floppy_wpt_cb(floppy_image_device *floppy , int state);

	uint16_t hp64k_usart_r(offs_t offset);
	void hp64k_usart_w(offs_t offset, uint16_t data);
	void hp64k_rxrdy_w(int state);
	void hp64k_txrdy_w(int state);
	void hp64k_txd_w(int state);
	void hp64k_dtr_w(int state);
	void hp64k_rts_w(int state);
	void hp64k_loopback_w(uint16_t data);
	void hp64k_update_loopback(void);
	void hp64k_rs232_rxd_w(int state);
	void hp64k_rs232_dcd_w(int state);
	void hp64k_rs232_cts_w(int state);

	uint16_t hp64k_phi_r(offs_t offset);
	void hp64k_phi_w(offs_t offset, uint16_t data);
	void hp64k_phi_int_w(int state);
	int hp64k_phi_sys_ctrl_r();

	void hp64k_beep_w(offs_t offset, uint16_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(hp64k_beeper_off);

	void hp64k_baud_clk_w(int state);
	void cpu_io_map(address_map &map) ATTR_COLD;
	void cpu_mem_map(address_map &map) ATTR_COLD;

	required_device<hp_5061_3011_cpu_device> m_cpu;
	required_device<i8275_device> m_crtc;
	required_device<palette_device> m_palette;
	required_ioport_array<4> m_io_key;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device_array<ttl74123_device, 2> m_ss;
	required_ioport m_rear_panel_sw;
	required_ioport m_rs232_sw;
	required_device<beep_device> m_beeper;
	required_device<timer_device> m_beep_timer;
	required_device<com8116_device> m_baud_rate;
	required_ioport m_s5_sw;
	required_device<i8251_device> m_uart;
	required_device<rs232_port_device> m_rs232;
	required_device<phi_device> m_phi;

	// Character generator
	const uint8_t *m_chargen;

	uint32_t m_crtc_ptr;
	bool m_crtc_drq;
	bool m_vrtc;

	// Interrupt handling
	uint8_t m_irl_mask;
	uint8_t m_irl_pending;

	// State of keyboard
	ioport_value m_kb_state[ 4 ];
	uint8_t m_kb_row_col;
	bool m_kb_scan_on;
	bool m_kb_pressed;

	// Slot selection
	std::vector<uint16_t> m_low32k_ram;
	uint8_t m_slot_select;
	uint8_t m_slot_map;

	// Floppy I/F
	uint8_t m_floppy_in_latch_msb;    // U23
	uint8_t m_floppy_in_latch_lsb;    // U38
	uint8_t m_floppy_out_latch_msb;   // U22
	uint8_t m_floppy_out_latch_lsb;   // U37
	uint8_t m_floppy_if_ctrl;     // U24
	bool m_floppy_dmaen;
	bool m_floppy_dmai;
	bool m_floppy_mdci;
	bool m_floppy_intrq;
	bool m_floppy_drq;
	bool m_floppy_wpt[2];
	uint8_t m_floppy_drv_ctrl;    // U39
	uint8_t m_floppy_status;      // U25

	typedef enum {
		HP64K_FLPST_IDLE,
		HP64K_FLPST_DMAWR1,
		HP64K_FLPST_DMAWR2,
		HP64K_FLPST_DMARD1,
		HP64K_FLPST_DMARD2
	} floppy_state_t;

	floppy_state_t m_floppy_if_state;
	floppy_image_device *m_current_floppy;

	// RS232 I/F
	bool m_16x_clk;
	bool m_baud_clk;
	uint8_t m_16x_div;
	bool m_loopback;
	bool m_txd_state;
	bool m_dtr_state;
	bool m_rts_state;

	// HPIB I/F
	uint8_t m_phi_reg;
};

void hp64k_state::cpu_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).rw(FUNC(hp64k_state::hp64k_slot_r), FUNC(hp64k_state::hp64k_slot_w));
	map(0x8000, 0x8001).w(FUNC(hp64k_state::hp64k_crtc_w));
	map(0x8002, 0xffff).ram();
}

void hp64k_state::cpu_io_map(address_map &map)
{
	// PA = 0, IC = [0..3]
	// Keyboard input
	map(HP_MAKE_IOADDR( 0, 0), HP_MAKE_IOADDR( 0, 3)).r(FUNC(hp64k_state::hp64k_kb_r));
	// PA = 2, IC = [0..3]
	// Line sync interrupt clear/watchdog reset
	map(HP_MAKE_IOADDR( 2, 0), HP_MAKE_IOADDR( 2, 3)).rw(FUNC(hp64k_state::hp64k_deltat_r), FUNC(hp64k_state::hp64k_deltat_w));
	// PA = 4, IC = [0..3]
	// Floppy I/F
	map(HP_MAKE_IOADDR( 4, 0), HP_MAKE_IOADDR( 4, 3)).rw(FUNC(hp64k_state::hp64k_flp_r), FUNC(hp64k_state::hp64k_flp_w));
	// PA = 5, IC = [0..3]
	// Write to USART
	map(HP_MAKE_IOADDR( 5, 0), HP_MAKE_IOADDR( 5, 3)).w(FUNC(hp64k_state::hp64k_usart_w));
	// PA = 6, IC = [0..3]
	// Read from USART
	map(HP_MAKE_IOADDR( 6, 0), HP_MAKE_IOADDR( 6, 3)).r(FUNC(hp64k_state::hp64k_usart_r));
	// PA = 7, IC = 1
	// PHI
	map(HP_MAKE_IOADDR( 7, 1), HP_MAKE_IOADDR( 7, 1)).rw(FUNC(hp64k_state::hp64k_phi_r), FUNC(hp64k_state::hp64k_phi_w));
	// PA = 7, IC = 2
	// Rear-panel switches and loopback relay control
	map(HP_MAKE_IOADDR( 7, 2), HP_MAKE_IOADDR( 7, 2)).rw(FUNC(hp64k_state::hp64k_rear_sw_r), FUNC(hp64k_state::hp64k_loopback_w));
	// PA = 9, IC = [0..3]
	// Beeper control & interrupt status read
	map(HP_MAKE_IOADDR( 9, 0), HP_MAKE_IOADDR( 9, 3)).w(FUNC(hp64k_state::hp64k_beep_w));
	// PA = 10, IC = [0..3]
	// Slot selection
	map(HP_MAKE_IOADDR(10, 0), HP_MAKE_IOADDR(10, 3)).w(FUNC(hp64k_state::hp64k_slot_sel_w));
	// PA = 12, IC = [0..3]
	// Interrupt mask
	map(HP_MAKE_IOADDR(12, 0), HP_MAKE_IOADDR(12, 3)).w(FUNC(hp64k_state::hp64k_irl_mask_w));
}

hp64k_state::hp64k_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig , type , tag),
	m_cpu(*this , "cpu"),
	m_crtc(*this , "crtc"),
	m_palette(*this , "palette"),
	m_io_key(*this , "KEY%u" , 0U),
	m_fdc(*this , "fdc"),
	m_floppy(*this , "fdc:%u" , 0U),
	m_ss(*this , "fdc_rdy%u" , 0U),
	m_rear_panel_sw(*this , "rear_sw"),
	m_rs232_sw(*this , "rs232_sw"),
	m_beeper(*this , "beeper"),
	m_beep_timer(*this , "beep_timer"),
	m_baud_rate(*this , "baud_rate"),
	m_s5_sw(*this , "s5_sw"),
	m_uart(*this , "uart"),
	m_rs232(*this , "rs232"),
	m_phi(*this , "phi")
{
}

void hp64k_state::driver_start()
{
	// 32kW for lower RAM
	m_low32k_ram.resize(0x8000);
}

void hp64k_state::video_start()
{
	m_chargen = memregion("chargen")->base();
}

void hp64k_state::machine_reset()
{
	m_crtc_drq = false;
	m_vrtc = false;
	m_crtc_ptr = 0;
	m_irl_mask = 0;
	m_irl_pending = 0;
	memset(&m_kb_state[ 0 ] , 0 , sizeof(m_kb_state));
	m_kb_row_col = 0;
	m_kb_scan_on = true;
	m_slot_select = 0;
	m_slot_map = 3;
	m_floppy_if_ctrl = ~0;
	m_floppy_dmaen = false;
	m_floppy_dmai = false;
	m_floppy_mdci = false;
	m_floppy_intrq = false;
	m_floppy_drv_ctrl = ~0;
	m_floppy_if_state = HP64K_FLPST_IDLE;
	m_current_floppy = nullptr;
	m_floppy_wpt[0] = false;
	m_floppy_wpt[1] = false;
	m_beeper->set_state(0);
	m_baud_rate->str_w((m_s5_sw->read() >> 1) & 0xf);
	m_16x_clk = (m_rs232_sw->read() & 0x02) != 0;
	m_loopback = false;
	m_txd_state = true;
	m_dtr_state = true;
	m_rts_state = true;
	m_phi_reg = 0;
}

uint8_t hp64k_state::hp64k_crtc_filter(uint8_t data)
{
		bool inv = (data & 0xe0) == 0xe0;

		return inv ? (data & 0xf2) : data;
}

void hp64k_state::hp64k_crtc_w(offs_t offset, uint16_t data)
{
		m_crtc->write(offset == 0 , hp64k_crtc_filter((uint8_t)data));
}

void hp64k_state::hp64k_crtc_drq_w(int state)
{
		bool crtc_drq = state != 0;
		bool prev_crtc = m_crtc_drq;
		m_crtc_drq = crtc_drq;

		if (!prev_crtc && crtc_drq) {
				address_space& prog_space = m_cpu->space(AS_PROGRAM);

				uint16_t data = prog_space.read_word(m_crtc_ptr >> 1);
				data = m_crtc_ptr & 1 ? data & 0xff : data >> 8;

				m_crtc_ptr++;

				m_crtc->dack_w(hp64k_crtc_filter(data));
		}
}

void hp64k_state::hp64k_crtc_vrtc_w(int state)
{
		bool vrtc = state != 0;

		if (!m_vrtc && vrtc) {
				m_crtc_ptr = 0xf9f0 << 1;
		}
		m_vrtc = vrtc;
}

I8275_DRAW_CHARACTER_MEMBER(hp64k_state::crtc_display_pixels)
{
		rgb_t const *const palette = m_palette->palette()->entry_list_raw();
		uint8_t chargen_byte = m_chargen[ linecount  | ((unsigned)charcode << 4) ];
		uint16_t pixels_lvid , pixels_livid;

		using namespace i8275_attributes;
		if (BIT(attrcode , VSP)) {
				pixels_lvid = pixels_livid = ~0;
		} else if (BIT(attrcode , LTEN)) {
				pixels_livid = ~0;
				if (BIT(attrcode , RVV)) {
						pixels_lvid = ~0;
				} else {
						pixels_lvid = 0;
				}
		} else if (BIT(attrcode , RVV)) {
				pixels_lvid = ~0;
				pixels_livid = (uint16_t)chargen_byte << 1;
		} else {
				pixels_lvid = ~((uint16_t)chargen_byte << 1);
				pixels_livid = ~0;
		}

		for (unsigned i = 0; i < 9; i++) {
				bool const lvid = (pixels_lvid & (1U << (8 - i))) != 0;
				bool const livid = (pixels_livid & (1U << (8 - i))) != 0;

				if (!lvid) {
						// Normal brightness
						bitmap.pix(y , x + i) = palette[ 2 ];
				} else if (livid) {
						// Black
						bitmap.pix(y , x + i) = palette[ 0 ];
				} else {
						// Half brightness
						bitmap.pix(y , x + i) = palette[ 1 ];
				}
		}

}

uint16_t hp64k_state::hp64k_rear_sw_r()
{
		return m_rear_panel_sw->read() | 0x0020;
}

uint8_t hp64k_state::int_cb(offs_t offset)
{
		if (offset == 0) {
				return (m_irl_mask & m_irl_pending);
		} else {
				return 0xff;
		}
}

void hp64k_state::hp64k_update_irl(void)
{
		m_cpu->set_input_line(HPHYBRID_IRL , (m_irl_mask & m_irl_pending) != 0);
}

void hp64k_state::hp64k_irl_mask_w(uint16_t data)
{
		m_irl_mask = (uint8_t)data;
		hp64k_update_irl();
}

TIMER_DEVICE_CALLBACK_MEMBER(hp64k_state::hp64k_kb_scan)
{
		if (m_kb_scan_on) {
				unsigned i;

				ioport_value input[ 4 ];
				input[ 0 ] = m_io_key[ 0 ]->read();
				input[ 1 ] = m_io_key[ 1 ]->read();
				input[ 2 ] = m_io_key[ 2 ]->read();
				input[ 3 ] = m_io_key[ 3 ]->read();

				for (i = 0; i < 128; i++) {
						if (++m_kb_row_col >= 128) {
								m_kb_row_col = 0;
						}

						ioport_value mask = BIT_MASK(m_kb_row_col & 0x1f);
						unsigned idx = m_kb_row_col >> 5;

						if ((input[ idx ] ^ m_kb_state[ idx ]) & mask) {
								// key changed state
								m_kb_state[ idx ] ^= mask;
								m_kb_pressed = (m_kb_state[ idx ] & mask) != 0;
								m_kb_scan_on = false;
								BIT_SET(m_irl_pending , 0);
								hp64k_update_irl();
								break;
						}
				}
		}
}

uint16_t hp64k_state::hp64k_kb_r()
{
		uint16_t ret = 0xff00 | m_kb_row_col;

		if (m_kb_pressed) {
				BIT_SET(ret , 7);
		}

		m_kb_scan_on = true;
		BIT_CLR(m_irl_pending , 0);
		hp64k_update_irl();

		return ret;
}

TIMER_DEVICE_CALLBACK_MEMBER(hp64k_state::hp64k_line_sync)
{
		BIT_SET(m_irl_pending , 2);
		hp64k_update_irl();
}

uint16_t hp64k_state::hp64k_deltat_r()
{
		BIT_CLR(m_irl_pending , 2);
		hp64k_update_irl();
		return 0;
}

void hp64k_state::hp64k_deltat_w(uint16_t data)
{
		BIT_CLR(m_irl_pending , 2);
		hp64k_update_irl();
}

uint16_t hp64k_state::hp64k_slot_r(offs_t offset)
{
		if (m_slot_select == 0x0a) {
				// Slot 10 selected
				// On this (fictional) slot is allocated the lower 32KW of RAM

				switch (m_slot_map) {
				case 0:
						// IDEN
						// ID of 32KW RAM expansion
						return 0x402;

				case 1:
						// MAP1
						// Lower half of RAM
						return m_low32k_ram[ offset ];

				default:
						// MAP2&3
						// Upper half of RAM
						return m_low32k_ram[ offset + 0x4000 ];
				}
		} else {
				return 0;
		}
}

void hp64k_state::hp64k_slot_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
		if (m_slot_select == 0x0a && m_slot_map != 0) {
				if (m_slot_map != 1) {
						// MAP2&3
						offset += 0x4000;
				}
				m_low32k_ram[ offset ] &= ~mem_mask;
				m_low32k_ram[ offset ] |= (data & mem_mask);
		}
}

void hp64k_state::hp64k_slot_sel_w(offs_t offset, uint16_t data)
{
		m_slot_map = (uint8_t)offset;
		m_slot_select = (uint8_t)((data >> 8) & 0x3f);
}

uint16_t hp64k_state::hp64k_flp_r(offs_t offset)
{
		m_cpu->dmar_w(0);

		switch (offset) {
		case 0:
				// DMA transfer, not at TC
				if (m_floppy_if_state == HP64K_FLPST_DMARD2) {
						m_floppy_if_state = HP64K_FLPST_IDLE;
				} else {
						logerror("Read from IC=0 with floppy state %d\n" , m_floppy_if_state);
				}
				break;

		case 1:
				if (m_floppy_if_state != HP64K_FLPST_IDLE) {
						logerror("read from IC=1 with floppy state %d\n" , m_floppy_if_state);
				}
				break;

		case 2:
				// DMA transfer, at TC
				if (m_floppy_if_state == HP64K_FLPST_DMARD2) {
						m_floppy_if_state = HP64K_FLPST_IDLE;
						m_floppy_dmaen = false;
						m_floppy_dmai = true;
				} else {
						logerror("Read from IC=2 with floppy state %d\n" , m_floppy_if_state);
				}
				break;

		default:
				logerror("read from IC=%d\n" , offset);
		}

		hp64k_update_floppy_irq();

		return ((uint16_t)m_floppy_out_latch_msb << 8) | (uint16_t)m_floppy_out_latch_lsb;
}

void hp64k_state::hp64k_flp_w(offs_t offset, uint16_t data)
{
		m_cpu->dmar_w(0);

		if (offset == 3) {
				return;
		}

		m_floppy_in_latch_msb = (uint8_t)(data >> 8);
		m_floppy_in_latch_lsb = (uint8_t)data;

		switch (offset) {
		case 0:
				// DMA transfer, not at TC
				if (m_floppy_if_state == HP64K_FLPST_DMAWR1) {
						m_fdc->data_w(~m_floppy_in_latch_msb);
						m_floppy_if_state = HP64K_FLPST_DMAWR2;
				} else {
						logerror("write to IC=0 with floppy state %d\n" , m_floppy_if_state);
				}
				break;

		case 1:
				if (m_floppy_if_state != HP64K_FLPST_IDLE) {
						logerror("write to IC=1 with floppy state %d\n" , m_floppy_if_state);
				}
				// I/F control register
				m_floppy_if_ctrl = m_floppy_in_latch_msb;
				if (BIT(m_floppy_if_ctrl , 4)) {
						// FDC reset
						m_fdc->soft_reset();
				}
				if (BIT(m_floppy_if_ctrl , 7)) {
						// Interrupt reset
						m_floppy_dmai = false;
						m_floppy_mdci = false;
				}
				if (BIT(m_floppy_if_ctrl , 3)) {
						// Write (to either FDC or drive control)
						if (BIT(m_floppy_if_ctrl , 2)) {
								// FDC
								m_fdc->write(~m_floppy_if_ctrl & 3 , ~m_floppy_in_latch_lsb);
						} else {
								// Drive control
								m_floppy_drv_ctrl = m_floppy_in_latch_lsb;
								hp64k_update_drv_ctrl();
						}
				} else {
						// Read
						if (BIT(m_floppy_if_ctrl , 2)) {
								// FDC
								m_floppy_out_latch_lsb = ~m_fdc->read(~m_floppy_if_ctrl & 3);
						} else {
								// Drive control
								m_floppy_out_latch_lsb = m_floppy_drv_ctrl;
						}
				}
				// MSB of output latch is always filled with status register
				m_floppy_out_latch_msb = m_floppy_status;
				m_floppy_dmaen = BIT(m_floppy_if_ctrl , 5) != 0;
				hp64k_update_floppy_dma();
				break;

		case 2:
				// DMA transfer, at TC
				if (m_floppy_if_state == HP64K_FLPST_DMAWR1) {
						m_fdc->data_w(~m_floppy_in_latch_msb);
						m_floppy_if_state = HP64K_FLPST_DMAWR2;
						m_floppy_dmaen = false;
						m_floppy_dmai = true;
				} else {
						logerror("write to IC=2 with floppy state %d\n" , m_floppy_if_state);
				}
				break;
		}

		hp64k_update_floppy_irq();
}

void hp64k_state::hp64k_flp_drq_w(int state)
{
		m_floppy_drq = state;
		hp64k_update_floppy_dma();
}

void hp64k_state::hp64k_flp_intrq_w(int state)
{
		if (state && !m_floppy_intrq && !BIT(m_floppy_if_ctrl , 7)) {
				m_floppy_mdci = true;
				hp64k_update_floppy_irq();
		}
		m_floppy_intrq = state;
}

void hp64k_state::hp64k_update_floppy_dma(void)
{
		if (m_floppy_drq && (m_floppy_dmaen || m_floppy_if_state != HP64K_FLPST_IDLE)) {
				switch (m_floppy_if_state) {
				case HP64K_FLPST_IDLE:
						if (BIT(m_floppy_if_ctrl , 6)) {
								// DMA writes
								m_cpu->dmar_w(1);
								m_floppy_if_state = HP64K_FLPST_DMAWR1;
						} else {
								// DMA reads
								m_floppy_out_latch_msb = ~m_fdc->data_r();
								m_floppy_if_state = HP64K_FLPST_DMARD1;
						}
						break;

				case HP64K_FLPST_DMAWR2:
						m_fdc->data_w(~m_floppy_in_latch_lsb);
						m_floppy_if_state = HP64K_FLPST_IDLE;
						break;

				case HP64K_FLPST_DMARD1:
						m_floppy_out_latch_lsb = ~m_fdc->data_r();
						m_cpu->dmar_w(1);
						m_floppy_if_state = HP64K_FLPST_DMARD2;
						break;

				default:
						logerror("DRQ with floppy state %d\n" , m_floppy_if_state);
				}
		}
}

void hp64k_state::hp64k_update_floppy_irq(void)
{
		if (m_floppy_dmai) {
				BIT_SET(m_floppy_status , 6);
		} else {
				BIT_CLR(m_floppy_status , 6);
		}
		if (m_floppy_mdci) {
				BIT_SET(m_floppy_status , 7);
		} else {
				BIT_CLR(m_floppy_status , 7);
		}

		bool ir4 = m_floppy_dmai || m_floppy_mdci ||
				(BIT(m_floppy_status , 2) && !BIT(m_floppy_drv_ctrl , 0)) ||
				(BIT(m_floppy_status , 5) && !BIT(m_floppy_drv_ctrl , 3));

		if (ir4) {
				BIT_SET(m_irl_pending , 4);
		} else {
				BIT_CLR(m_irl_pending , 4);
		}

		hp64k_update_irl();
}

void hp64k_state::hp64k_update_drv_ctrl(void)
{
		floppy_image_device *floppy0 = m_floppy[0]->get_device();
		floppy_image_device *floppy1 = m_floppy[1]->get_device();

		floppy0->mon_w(BIT(m_floppy_drv_ctrl , 1));
		floppy1->mon_w(BIT(m_floppy_drv_ctrl , 4));
		floppy0->ss_w(!BIT(m_floppy_drv_ctrl , 7));
		floppy1->ss_w(!BIT(m_floppy_drv_ctrl , 7));

		if (BIT(m_floppy_drv_ctrl , 2)) {
				BIT_CLR(m_floppy_status , 2);
		}
		if (BIT(m_floppy_drv_ctrl , 5)) {
				BIT_CLR(m_floppy_status , 5);
		}
		hp64k_update_floppy_irq();

		// Drive selection logic:
		// m_floppy_drv_ctrl
		// Bit 3 0 - Drive selected
		// ========================
		//     0 0 - Invalid:both drives selected. Signals to/from drive 1 are routed to FDC anyway.
		//     0 1 - Drive 1
		//     1 0 - Drive 0
		//     1 1 - None
		floppy_image_device *new_drive;

		if (!BIT(m_floppy_drv_ctrl , 3)) {
				new_drive = m_floppy[1]->get_device();
		} else if (!BIT(m_floppy_drv_ctrl , 0)) {
				new_drive = m_floppy[0]->get_device();
		} else {
				new_drive = nullptr;
		}

		if (new_drive != m_current_floppy) {
				m_fdc->set_floppy(new_drive);

				floppy0->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&hp64k_state::hp64k_floppy_idx_cb, this));
				floppy1->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&hp64k_state::hp64k_floppy_idx_cb, this));

				floppy0->setup_wpt_cb(floppy_image_device::wpt_cb(&hp64k_state::hp64k_floppy_wpt_cb, this));
				floppy1->setup_wpt_cb(floppy_image_device::wpt_cb(&hp64k_state::hp64k_floppy_wpt_cb, this));

				m_current_floppy = new_drive;
		}
}

void hp64k_state::hp64k_floppy0_rdy(int state)
{
		if (state) {
				BIT_CLR(m_floppy_status , 0);
		} else {
				BIT_SET(m_floppy_status , 0);
		}
}

void hp64k_state::hp64k_floppy1_rdy(int state)
{
		if (state) {
				BIT_CLR(m_floppy_status , 3);
		} else {
				BIT_SET(m_floppy_status , 3);
		}
}

void hp64k_state::hp64k_floppy_idx_cb(floppy_image_device *floppy , int state)
{
		if (floppy == m_floppy[0]->get_device()) {
				m_ss[0]->a_w(!state);
		} else if (floppy == m_floppy[1]->get_device()) {
				m_ss[1]->a_w(!state);
		}

		if (floppy == m_current_floppy) {
				m_fdc->index_callback(floppy , state);
		}
}

void hp64k_state::hp64k_floppy_wpt_cb(floppy_image_device *floppy , int state)
{
		if (floppy == m_floppy[0]->get_device()) {
				logerror("floppy0_wpt %d\n" , state);
				if (m_floppy_wpt[0] && !state) {
						BIT_SET(m_floppy_status , 2);
						hp64k_update_floppy_irq();
				}
				if (state) {
						BIT_SET(m_floppy_status, 1);
				} else {
						BIT_CLR(m_floppy_status, 1);
				}
				m_floppy_wpt[0] = state;
		} else if (floppy == m_floppy[1]->get_device()) {
				logerror("floppy1_wpt %d\n" , state);
				if (m_floppy_wpt[1] && !state) {
						BIT_SET(m_floppy_status , 5);
						hp64k_update_floppy_irq();
				}
				if (state) {
						BIT_SET(m_floppy_status, 4);
				} else {
						BIT_CLR(m_floppy_status, 4);
				}
				m_floppy_wpt[1] = state;
		}
}

uint16_t hp64k_state::hp64k_usart_r(offs_t offset)
{
		uint16_t tmp = m_uart->read(~offset & 1);

		// bit 8 == bit 7 rear panel switches (modem/terminal) ???

		tmp |= (m_rs232_sw->read() << 8);

		if (BIT(m_rear_panel_sw->read() , 7)) {
				BIT_SET(tmp , 8);
		}

		return tmp;
}

void hp64k_state::hp64k_usart_w(offs_t offset, uint16_t data)
{
		m_uart->write(~offset & 1, data & 0xff);
}

void hp64k_state::hp64k_rxrdy_w(int state)
{
		if (state) {
				BIT_SET(m_irl_pending , 6);
		} else {
				BIT_CLR(m_irl_pending , 6);
		}

		hp64k_update_irl();
}

void hp64k_state::hp64k_txrdy_w(int state)
{
		if (state) {
				BIT_SET(m_irl_pending , 5);
		} else {
				BIT_CLR(m_irl_pending , 5);
		}

		hp64k_update_irl();
}

void hp64k_state::hp64k_txd_w(int state)
{
		m_txd_state = state;
		if (m_loopback) {
				m_uart->write_rxd(state);
		}
		m_rs232->write_txd(state);
}

void hp64k_state::hp64k_dtr_w(int state)
{
		m_dtr_state = state;
		if (m_loopback) {
				m_uart->write_dsr(state);
		}
		m_rs232->write_dtr(state);
}

void hp64k_state::hp64k_rts_w(int state)
{
	if (BIT(m_s5_sw->read() , 0)) {
		// Full duplex, RTS/ = 0
		state = 0;
	}
	m_rts_state = state;
	if (m_loopback) {
		m_uart->write_cts(state);
	}
	m_rs232->write_rts(state);
}

void hp64k_state::hp64k_loopback_w(uint16_t data)
{
	m_phi_reg = (uint8_t)((data >> 8) & 7);
	m_loopback = BIT(data , 11);
	hp64k_update_loopback();
}

void hp64k_state::hp64k_update_loopback(void)
{
	if (m_loopback) {
		m_uart->write_rxd(m_txd_state);
		m_uart->write_dsr(m_dtr_state);
		m_uart->write_cts(m_rts_state);
	} else {
		m_uart->write_rxd(m_rs232->rxd_r());
		m_uart->write_dsr(m_rs232->dcd_r());
		m_uart->write_cts(m_rs232->cts_r());
	}
}

void hp64k_state::hp64k_rs232_rxd_w(int state)
{
	if (!m_loopback) {
		m_uart->write_rxd(state);
	}
}

void hp64k_state::hp64k_rs232_dcd_w(int state)
{
	if (!m_loopback) {
		m_uart->write_dsr(state);
	}
}

uint16_t hp64k_state::hp64k_phi_r(offs_t offset)
{
	return m_phi->reg16_r(m_phi_reg);
}

void hp64k_state::hp64k_phi_w(offs_t offset, uint16_t data)
{
	m_phi->reg16_w(m_phi_reg , data);
}

void hp64k_state::hp64k_rs232_cts_w(int state)
{
	if (!m_loopback) {
		m_uart->write_cts(state);
	}
}

void hp64k_state::hp64k_phi_int_w(int state)
{
	if (state) {
		BIT_SET(m_irl_pending , 7);
	} else {
		BIT_CLR(m_irl_pending , 7);
	}

	hp64k_update_irl();
}

int hp64k_state::hp64k_phi_sys_ctrl_r()
{
	return BIT(m_rear_panel_sw->read() , 6);
}

void hp64k_state::hp64k_beep_w(offs_t offset, uint16_t data)
{
	if (!BIT(offset , 0)) {
		m_beeper->set_state(1);
		// Duration is bogus: in the real hw envelope decays exponentially with RC=~136 ms
		m_beep_timer->adjust(attotime::from_msec(130));
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp64k_state::hp64k_beeper_off)
{
	m_beeper->set_state(0);
}

void hp64k_state::hp64k_baud_clk_w(int state)
{
	if (!m_16x_clk) {
		if (state && !m_baud_clk) {
			m_16x_div++;
		}
		m_baud_clk = !!state;
		state = BIT(m_16x_div , 3);
	}
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

static INPUT_PORTS_START(hp64k)
	// Keyboard is arranged in a 8 x 16 matrix. Of the 128 possible positions, only 77 are used.
	// For key arrangement on the matrix, see [1] pg 334
	// Keys are mapped on bit b of KEYn
	// where b = (row & 1) << 4 + column, n = row >> 1
	// column = [0..15]
	// row = [0..7]
	PORT_START("KEY0")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_E)     PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(13) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(14) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(15) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)       PORT_CHAR('\t')
	PORT_BIT(BIT_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(BIT_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(19) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(20) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(BIT_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(BIT_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(BIT_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
	PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(BIT_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(BIT_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(BIT_MASK(30) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(31) , IP_ACTIVE_HIGH , IPT_UNUSED)

	PORT_START("KEY1")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)        PORT_NAME("RECALL")
	PORT_BIT(BIT_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)       PORT_NAME("CLRLINE")
	PORT_BIT(BIT_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)       PORT_NAME("CAPS")
	PORT_BIT(BIT_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)       PORT_NAME("RESET")
	PORT_BIT(BIT_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)        PORT_NAME("SK1")
	PORT_BIT(BIT_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)        PORT_NAME("SK2")
	PORT_BIT(BIT_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)        PORT_NAME("SK3")
	PORT_BIT(BIT_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)        PORT_NAME("SK4")
	PORT_BIT(BIT_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(22) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)        PORT_NAME("SK5")
	PORT_BIT(BIT_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)        PORT_NAME("SK6")
	PORT_BIT(BIT_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)        PORT_NAME("SK7")
	PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)        PORT_NAME("SK8")
	PORT_BIT(BIT_MASK(28) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(29) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(30) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(31) , IP_ACTIVE_HIGH , IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(13) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)    PORT_NAME("INSCHAR")
	PORT_BIT(BIT_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_NAME("DELCHAR")
	PORT_BIT(BIT_MASK(16) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(17) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(BIT_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(BIT_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(BIT_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(22) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(BIT_MASK(24) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(25) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(BIT_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)    PORT_CHAR('_') PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(BIT_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)      PORT_NAME("ROLLUP")
	PORT_BIT(BIT_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(BIT_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)      PORT_NAME("NEXTPG")

	PORT_START("KEY3")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(BIT_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT(BIT_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)          PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(BIT_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)         PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(BIT_MASK(16) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(17) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(19) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(20) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)
	PORT_BIT(BIT_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(BIT_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(BIT_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(BIT_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(BIT_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)        PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(BIT_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_END)           PORT_NAME("ROLLDN")
	PORT_BIT(BIT_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)          PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(BIT_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)          PORT_NAME("PREVPG")

	PORT_START("rear_sw")
	PORT_DIPNAME(0x8000 , 0x8000 , "E9-6 jumper")
	PORT_DIPSETTING(0x0000 , DEF_STR(Yes))
	PORT_DIPSETTING(0x8000 , DEF_STR(No))
	PORT_DIPNAME(0x4000 , 0x4000 , "E9-5 jumper")
	PORT_DIPSETTING(0x0000 , DEF_STR(Yes))
	PORT_DIPSETTING(0x4000 , DEF_STR(No))
	PORT_DIPNAME(0x2000 , 0x2000 , "E9-4 jumper")
	PORT_DIPSETTING(0x0000 , DEF_STR(Yes))
	PORT_DIPSETTING(0x2000 , DEF_STR(No))
	PORT_DIPNAME(0x1000 , 0x1000 , "E9-3 jumper")
	PORT_DIPSETTING(0x0000 , DEF_STR(Yes))
	PORT_DIPSETTING(0x1000 , DEF_STR(No))
	PORT_DIPNAME(0x0800 , 0x0800 , "E9-2 jumper")
	PORT_DIPSETTING(0x0000 , DEF_STR(Yes))
	PORT_DIPSETTING(0x0800 , DEF_STR(No))
	PORT_DIPNAME(0x0400 , 0x0400 , "E9-1 jumper")
	PORT_DIPSETTING(0x0000 , DEF_STR(Yes))
	PORT_DIPSETTING(0x0400 , DEF_STR(No))
	PORT_DIPNAME(0x0040 , 0x0000 , "System controller")
	PORT_DIPSETTING(0x0000 , DEF_STR(No))
	PORT_DIPSETTING(0x0040 , DEF_STR(Yes))
	PORT_DIPNAME(0x0018 , 0x0000 , "System source")
	PORT_DIPLOCATION("S1:!7,!6")
	PORT_DIPSETTING(0x0000 , "Sys bus")
	PORT_DIPSETTING(0x0008 , "Local storage-talk only")
	PORT_DIPSETTING(0x0010 , "Local storage-addressable")
	PORT_DIPSETTING(0x0018 , "Performance verification")
	PORT_DIPNAME(0x0300 , 0x0000 , "Upper bus address (N/U)")
	PORT_DIPLOCATION("S1:!2,!1")
	PORT_DIPSETTING(0x0000 , "0")
	PORT_DIPSETTING(0x0100 , "1")
	PORT_DIPSETTING(0x0200 , "2")
	PORT_DIPSETTING(0x0300 , "3")
	PORT_DIPNAME(0x0007 , 0x0000 , "System bus address")
	PORT_DIPLOCATION("S1:!5,!4,!3")
	PORT_DIPSETTING(0x0000 , "0")
	PORT_DIPSETTING(0x0001 , "1")
	PORT_DIPSETTING(0x0002 , "2")
	PORT_DIPSETTING(0x0003 , "3")
	PORT_DIPSETTING(0x0004 , "4")
	PORT_DIPSETTING(0x0005 , "5")
	PORT_DIPSETTING(0x0006 , "6")
	PORT_DIPSETTING(0x0007 , "7")
	PORT_DIPNAME(0x0080 , 0x0000 , "RS232 mode")
	PORT_DIPLOCATION("S4 IO:!8")
	PORT_DIPSETTING(0x0000 , "Terminal")
	PORT_DIPSETTING(0x0080 , "Modem")

	PORT_START("rs232_sw")
	PORT_DIPNAME(0xc0 , 0x00 , "Stop bits")
	PORT_DIPLOCATION("S4 IO:!2,!1")
	PORT_DIPSETTING(0x00 , "Invalid")
	PORT_DIPSETTING(0x40 , "1")
	PORT_DIPSETTING(0x80 , "1.5")
	PORT_DIPSETTING(0xc0 , "2")
	PORT_DIPNAME(0x20 , 0x00 , "Parity")
	PORT_DIPLOCATION("S4 IO:!3")
	PORT_DIPSETTING(0x00 , "Odd")
	PORT_DIPSETTING(0x20 , "Even")
	PORT_DIPNAME(0x10 , 0x00 , "Parity enable")
	PORT_DIPLOCATION("S4 IO:!4")
	PORT_DIPSETTING(0x00 , DEF_STR(No))
	PORT_DIPSETTING(0x10 , DEF_STR(Yes))
	PORT_DIPNAME(0x0c , 0x00 , "Char length")
	PORT_DIPLOCATION("S4 IO:!6,!5")
	PORT_DIPSETTING(0x00 , "5")
	PORT_DIPSETTING(0x04 , "6")
	PORT_DIPSETTING(0x08 , "7")
	PORT_DIPSETTING(0x0c , "8")
	PORT_DIPNAME(0x02 , 0x00 , "Baud rate factor")
	PORT_DIPLOCATION("S4 IO:!7")
	PORT_DIPSETTING(0x00 , "1x")
	PORT_DIPSETTING(0x02 , "16x")

	PORT_START("s5_sw")
	PORT_DIPNAME(0x01 , 0x00 , "Duplex")
	PORT_DIPLOCATION("S5 IO:!1")
	PORT_DIPSETTING(0x00 , "Half duplex")
	PORT_DIPSETTING(0x01 , "Full duplex")
	PORT_DIPNAME(0x1e , 0x00 , "Baud rate")
	PORT_DIPLOCATION("S5 IO:!5,!4,!3,!2")
	PORT_DIPSETTING(0x00 , "50")
	PORT_DIPSETTING(0x02 , "75")
	PORT_DIPSETTING(0x04 , "110")
	PORT_DIPSETTING(0x06 , "134.5")
	PORT_DIPSETTING(0x08 , "150")
	PORT_DIPSETTING(0x0a , "300")
	PORT_DIPSETTING(0x0c , "600")
	PORT_DIPSETTING(0x0e , "1200")
	PORT_DIPSETTING(0x10 , "1800")
	PORT_DIPSETTING(0x12 , "2000")
	PORT_DIPSETTING(0x14 , "2400")
	PORT_DIPSETTING(0x16 , "3600")
	PORT_DIPSETTING(0x18 , "4800")
	PORT_DIPSETTING(0x1a , "7200")
	PORT_DIPSETTING(0x1c , "9600")
	PORT_DIPSETTING(0x1e , "19200")
INPUT_PORTS_END

static void hp64k_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void hp64k_state::hp64k(machine_config &config)
{
	HP_5061_3011(config, m_cpu, 6250000);
	m_cpu->set_rw_cycles(6 , 6);
	m_cpu->set_relative_mode(true);
	m_cpu->set_addrmap(AS_PROGRAM, &hp64k_state::cpu_mem_map);
	m_cpu->set_addrmap(AS_IO, &hp64k_state::cpu_io_map);
	m_cpu->set_int_cb(FUNC(hp64k_state::int_cb));

	// Actual keyboard refresh rate should be between 1 and 2 kHz
	TIMER(config, "kb_timer").configure_periodic(FUNC(hp64k_state::hp64k_kb_scan), attotime::from_hz(100));

	// Line sync timer. A line frequency of 50 Hz is assumed.
	TIMER(config, "linesync_timer").configure_periodic(FUNC(hp64k_state::hp64k_line_sync), attotime::from_hz(50));

	// Clock = 25 MHz / 9 * (112/114)
	I8275(config, m_crtc, 2729045);
	m_crtc->set_screen("screen");
	m_crtc->set_character_width(9);
	m_crtc->set_display_callback(FUNC(hp64k_state::crtc_display_pixels));
	m_crtc->drq_wr_callback().set(FUNC(hp64k_state::hp64k_crtc_drq_w));
	m_crtc->vrtc_wr_callback().set(FUNC(hp64k_state::hp64k_crtc_vrtc_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));
	screen.set_refresh_hz(60);
	screen.set_size(720, 390);
	screen.set_visarea(0, 720-1, 0, 390-1);
	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	FD1791(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->set_force_ready(true); // should be able to get rid of this when fdc issue is fixed
	m_fdc->intrq_wr_callback().set(FUNC(hp64k_state::hp64k_flp_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(hp64k_state::hp64k_flp_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", hp64k_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats, true);
	FLOPPY_CONNECTOR(config, "fdc:1", hp64k_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats, true);

	TTL74123(config, m_ss[0], 0);
	m_ss[0]->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_ss[0]->set_resistor_value(RES_K(68.1));
	// Warning! Duration formula is not correct for LS123, actual capacitor is 10 uF
	m_ss[0]->set_capacitor_value(CAP_U(16));
	m_ss[0]->set_b_pin_value(1);
	m_ss[0]->set_clear_pin_value(1);
	m_ss[0]->out_cb().set(FUNC(hp64k_state::hp64k_floppy0_rdy));

	TTL74123(config, m_ss[1], 0);
	m_ss[1]->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_ss[1]->set_resistor_value(RES_K(68.1));
	m_ss[1]->set_capacitor_value(CAP_U(16));
	m_ss[1]->set_b_pin_value(1);
	m_ss[1]->set_clear_pin_value(1);
	m_ss[1]->out_cb().set(FUNC(hp64k_state::hp64k_floppy1_rdy));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2500).add_route(ALL_OUTPUTS, "mono", 1.00);

	TIMER(config, m_beep_timer).configure_generic(FUNC(hp64k_state::hp64k_beeper_off));

	COM8116(config, m_baud_rate, 5.0688_MHz_XTAL);
	m_baud_rate->fr_handler().set(FUNC(hp64k_state::hp64k_baud_clk_w));

	I8251(config, m_uart, 0);
	m_uart->rxrdy_handler().set(FUNC(hp64k_state::hp64k_rxrdy_w));
	m_uart->txrdy_handler().set(FUNC(hp64k_state::hp64k_txrdy_w));
	m_uart->txd_handler().set(FUNC(hp64k_state::hp64k_txd_w));
	m_uart->dtr_handler().set(FUNC(hp64k_state::hp64k_dtr_w));
	m_uart->rts_handler().set(FUNC(hp64k_state::hp64k_rts_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(hp64k_state::hp64k_rs232_rxd_w));
	m_rs232->dcd_handler().set(FUNC(hp64k_state::hp64k_rs232_dcd_w));
	m_rs232->cts_handler().set(FUNC(hp64k_state::hp64k_rs232_cts_w));

	PHI(config, m_phi, 0);
	m_phi->int_write_cb().set(FUNC(hp64k_state::hp64k_phi_int_w));
	m_phi->dmarq_write_cb().set(m_cpu, FUNC(hp_5061_3011_cpu_device::halt_w));
	m_phi->sys_cntrl_read_cb().set(FUNC(hp64k_state::hp64k_phi_sys_ctrl_r));
	m_phi->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_phi->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_phi->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_phi->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_phi->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_phi->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_phi->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_phi->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_phi->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_phi->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	ieee488_device &ieee(IEEE488(config, IEEE488_TAG));
	ieee.eoi_callback().set(m_phi, FUNC(phi_device::eoi_w));
	ieee.dav_callback().set(m_phi, FUNC(phi_device::dav_w));
	ieee.nrfd_callback().set(m_phi, FUNC(phi_device::nrfd_w));
	ieee.ndac_callback().set(m_phi, FUNC(phi_device::ndac_w));
	ieee.ifc_callback().set(m_phi, FUNC(phi_device::ifc_w));
	ieee.srq_callback().set(m_phi, FUNC(phi_device::srq_w));
	ieee.atn_callback().set(m_phi, FUNC(phi_device::atn_w));
	ieee.ren_callback().set(m_phi, FUNC(phi_device::ren_w));
	ieee.dio_callback().set(m_phi, FUNC(phi_device::bus_dio_w));
	IEEE488_SLOT(config, "ieee_rem", 0, remote488_devices, nullptr);
}

ROM_START(hp64k)
	ROM_REGION(0x8000, "cpu" , ROMREGION_16BIT | ROMREGION_BE | ROMREGION_INVERT)
	ROM_LOAD16_BYTE("64100_80022.bin" , 0x0000 , 0x1000 , CRC(38b2aae5) SHA1(bfd0f126bfaf3724dc501979ad2d46afc41913aa))
	ROM_LOAD16_BYTE("64100_80020.bin" , 0x0001 , 0x1000 , CRC(ac01b436) SHA1(be1e827ea1393a95abb02a52ab5cc35dc2cd96e4))
	ROM_LOAD16_BYTE("64100_80023.bin" , 0x2000 , 0x1000 , CRC(6b4bc2ce) SHA1(00e6c58ccae9640dc81cb3e92db90a8c69b02a93))
	ROM_LOAD16_BYTE("64100_80021.bin" , 0x2001 , 0x1000 , CRC(74f9d33c) SHA1(543a845a992b0ceac3e0491acdfb178df0adeb1f))
	ROM_LOAD16_BYTE("64100_80026.bin" , 0x4000 , 0x1000 , CRC(a74e834b) SHA1(a2ff9765628985d9bab4cb44ba23257a9b8d0965))
	ROM_LOAD16_BYTE("64100_80024.bin" , 0x4001 , 0x1000 , CRC(2e15a1d2) SHA1(ce4330f8f8015a26c02f0965b95baf7dfd615512))
	ROM_LOAD16_BYTE("64100_80027.bin" , 0x6000 , 0x1000 , CRC(b93c0e7a) SHA1(b239446d3d6e9d3dba6c0278b2771abe1623e1ad))
	ROM_LOAD16_BYTE("64100_80025.bin" , 0x6001 , 0x1000 , CRC(e6353085) SHA1(48d78835c798f2caf6ee539057676d4f3c8a4df9))

	ROM_REGION(0x800 , "chargen" , 0)
	ROM_LOAD("1816_1496_82s191.bin" , 0 , 0x800 , CRC(32a52664) SHA1(8b2a49a32510103ff424e8481d5ed9887f609f2f))
ROM_END

} // anonymous namespace


/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME */
COMP( 1979, hp64k, 0,      0,      hp64k,   hp64k, hp64k_state, empty_init, "HP",    "HP 64000" , 0)
