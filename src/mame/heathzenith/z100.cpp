// license:BSD-3-Clause
// copyright-holders:Angelo Salese, AJR
/***************************************************************************

    Zenith Z-100

    15/07/2011 Skeleton driver.

    Commands:
    Press DELETE to abort boot, then press H to list all the commands.

    TODO:
    - implement S-100 bus features;
    - memory test hangs on the first pass;

============================================================================

Z207A       EQU 0B0H    ; Z-207 disk controller base port
  ; (See DEFZ207 to program controller)
Z217A       EQU 0AEH    ; Z-217 disk controller base port
  ; (See DEFZ217 to program controller)
ZGRNSEG     EQU 0E000H  ; Segment of green video plane
ZREDSEG     EQU 0D000H  ; Segment of red video plane
ZBLUSEG     EQU 0C000H  ; Segment of blue video plane
ZVIDEO      EQU 0D8H    ; Video 68A21 port
  ; PA0 -> enable red display
  ; PA1 -> enable green display
  ; PA2 -> enable blue display
  ; PA3 -> not flash screen
  ; PA4 -> not write multiple red
  ; PA5 -> not write multiple green
  ; PA6 -> not write multiple blue
  ; PA7 -> disable video RAM
  ; PB7-PB0 -> LA15-LA8
  ; CA1 - not used
  ; CA2 -> clear screen
  ; CB1 - not used
  ; CB2 -> value to write (0 or 1) on clear screen
  ; (see DEF6821 to program the 6821)
ZCRTC       EQU 0DCH    ; Video 6845 CRT-C port
  ; (see DEF6845 to program the 6845)
ZLPEN       EQU 0DEH    ; Light pen latch
  ZLPEN_BIT   EQU 00000111B   ; Bit hit by pen
  ZLPEN_ROW   EQU 11110000B   ; Row hit by pen
ZPIA        EQU 0E0H    ; Parallel printer plus light pen and
                                ;  video vertical retrace 68A21 port
  ; PA0 -> PDATA1
  ; PA1 -> PDATA2
  ; PA2 -> not STROBE
  ; PA3 -> not INIT
  ; PA4 <- VSYNC
  ; PA5 -> clear VSYNC flip flop
  ; PA6 <- light pen switch
  ; PA7 -> clear light pen flip flop
  ; PB0 <- BUSY
  ; PB1 <- not ERROR
  ; PB2 -> PDATA3
  ; PB3 -> PDATA4
  ; PB4 -> PDATA5
  ; PB5 -> PDATA6
  ; PB6 -> PDATA7
  ; PB7 -> PDATA8
  ; CA1 <- light pen hit (from flip flop)
  ; CA2 <- VSYNC (from flip flop)
  ; CB1 <- not ACKNLG
  ; CB2 <- BUSY
  ; (See DEF6821 to program the PIA)
ZTIMER      EQU 0E4H    ; Timer 8253 port
  ZTIMEVAL    EQU 2500    ; 100ms divide by N value
  ; (See DEF8253 to program the 8253)
ZTIMERS     EQU 0FBH    ; Timer interrupt status port
  ZTIMERS0    EQU 001H    ; Timer 0 interrupt
  ZTIMERS2    EQU 002H    ; Timer 2 interrupt
ZSERA       EQU 0E8H    ; First 2661-2 serial port
ZSERB       EQU 0ECH    ; Second 2661-2 serial port
  ; (See DEFEP2 to program 2661-2)
ZM8259A     EQU 0F2H    ; Master 8259A interrupt controller port
  ZINTEI      EQU 0       ; Parity error or S-100 pin 98 interrupt
  ZINTPS      EQU 1       ; Processor swap interrupt
  ZINTTIM     EQU 2       ; Timer interrupt
  ZINTSLV     EQU 3       ; Slave 8259A interrupt
  ZINTSA      EQU 4       ; Serial port A interrupt
  ZINTSB      EQU 5       ; Serial port B interrupt
  ZINTKD      EQU 6       ; Keyboard, Display, or Light pen interrupt
  ZINTPP      EQU 7       ; Parallel port interrupt
  ; (See DEF8259A to program the 8259A)
  ZM8259AI    EQU 64        ; Base interrupt number for master
ZS8259A     EQU 0F0H    ; Secondary 8259A interrupt controller port
  ZS8259AI    EQU 72        ; Base interrupt number for slave
  BIOSAI      EQU ZS8259AI+8    ; Base of BIOS generated interrupts
ZKEYBRD     EQU 0F4H    ; Keyboard port
  ZKEYBRDD    EQU ZKEYBRD+0   ; Keyboard data port
  ZKEYBRDC    EQU ZKEYBRD+1   ; Keyboard command port
    ZKEYRES     EQU 0       ; Reset command
    ZKEYARD     EQU 1       ; Autorepeat on command
    ZKEYARF     EQU 2       ; Autorepeat off command
    ZKEYKCO     EQU 3       ; Key click on command
    ZKEYKCF     EQU 4       ; Key click off command
    ZKEYCF      EQU 5       ; Clear keyboard FIFO command
    ZKEYCLK     EQU 6       ; Generate a click sound command
    ZKEYBEP     EQU 7       ; Generate a beep sound command
    ZKEYEK      EQU 8       ; Enable keyboard command
    ZKEYDK      EQU 9       ; Disable keyboard command
    ZKEYUDM     EQU 10      ; Enter UP/DOWN mode command
    ZKEYNSM     EQU 11      ; Enter normal scan mode command
    ZKEYEI      EQU 12      ; Enable keyboard interrupts command
    ZKEYDI      EQU 13      ; Disable keyboard interrupts command
  ZKEYBRDS    EQU ZKEYBRD+1   ; Keyboard status port
    ZKEYOBF     EQU 001H        ; Output buffer not empty
    ZKEYIBF     EQU 002H        ; Input buffer full
ZMCL        EQU 0FCH    ; Memory control latch
  ZMCLMS      EQU 00000011B   ; Map select mask
    ZSM0        EQU 0       ; Map select 0
    ZSM1        EQU 1       ; Map select 1
    ZSM2        EQU 2       ; Map select 2
    ZSM3        EQU 3       ; Map select 3
  ZMCLRM      EQU 00001100B   ; Monitor ROM mapping mask
    ZRM0        EQU 0*4         ; Power up mode - ROM everywhere on reads
    ZRM1        EQU 1*4         ; ROM at top of every 64K page
    ZRM2        EQU 2*4         ; ROM at top of 8088's addr space
    ZRM3        EQU 3*4         ; Disable ROM
  ZMCLPZ      EQU 00010000B   ; 0=Set Parity to the zero state
  ZMCLPK      EQU 00100000B   ; 0=Disable parity checking circuity
  ZMCLPF      EQU 01000000B   ; 0=Disable parity error flag
Z205BA      EQU 098H    ; Base address for Z-205 boards
  Z205BMC     EQU 8       ; Maximum of 8 Z-205 boards installed
ZHAL        EQU 0FDH    ; Hi-address latch
  ZHAL85      EQU 0FFH    ; 8085 Mask
  ZHAL88      EQU 0F0H    ; 8088 Mask
ZPSP        EQU 0FEH    ; Processor swap port
  ZPSPPS      EQU 10000000B   ; Processor select (0=8085, 1=8088)
  ZPSPPS5     EQU 00000000B   ; Select 8085
  ZPSPPS8     EQU 10000000B   ; Select 8088
  ZPSPSI      EQU 00000010B   ; Generate interrupt on swapping
  ZPSPI8      EQU 00000001B   ; 8088 processes all interrupts
ZDIPSW      EQU 0FFH    ; Configuration dip switches
  ZDIPSWBOOT      EQU 00000111B   ; Boot device field
  ZDIPSWAB    EQU 00001000B   ; 1=Auto boot(0=Manual boot)
  ZDIPSWRES   EQU 01110000B   ; Reserved
  ZDIPSWHZ    EQU 10000000B   ; 1=50Hz(0=60HZ)

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "bus/centronics/ctronics.h"
//#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "imagedev/floppy.h"
#include "machine/74123.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/rescap.h"
#include "machine/scn_pci.h"
#include "machine/wd_fdc.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class z100_state : public driver_device
{
public:
	z100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_pia(*this, "pia%u", 0U),
		m_picm(*this, "pic8259_master"),
		m_pics(*this, "pic8259_slave"),
		m_fdc(*this, "z207_fdc"),
		m_floppies(*this, "z207_fdc:%u", 0U),
		m_epci(*this, "epci%u", 0U),
		m_keyclick(*this, "keyclick"),
		m_keybeep(*this, "keybeep"),
		m_beeper(*this, "beeper"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_vrmm(*this, "vrmm"),
		m_vram_config(*this, "VRAM"),
		m_keys(*this, "COL%u", 0U),
		m_ctrl(*this, "CTRL"),
		m_floppy(nullptr)
	{ }

	void z100(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(kbd_reset);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	void memory_ctrl_w(uint8_t data);
	offs_t vram_map(offs_t offset) const;
	uint8_t z100_vram_r(offs_t offset);
	void z100_vram_w(offs_t offset, uint8_t data);
	void kbd_col_w(uint8_t data);
	uint8_t kbd_rows_r();
	int kbd_shift_row_r();
	void beep_update(int state);
	void floppy_select_w(uint8_t data);
	void floppy_motor_w(uint8_t data);
	uint8_t tmr_status_r();
	void tmr_status_w(uint8_t data);
	void timer_flipflop0_w(int state);
	void timer_flipflop1_w(int state);
	void vidint_w(int state);
	void vidint_enable_w(int state);

	u8 get_slave_ack(offs_t offset);
	void video_pia_A_w(u8 data);
	void video_pia_B_w(u8 data);
	void video_pia_CA2_w(int state);
	void video_pia_CB2_w(int state);

	MC6845_UPDATE_ROW(update_row);

	void z100_io(address_map &map) ATTR_COLD;
	void z100_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<pic8259_device> m_picm;
	required_device<pic8259_device> m_pics;
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	required_device_array<scn2661b_device, 2> m_epci;
	required_device<ttl74123_device> m_keyclick;
	required_device<ttl74123_device> m_keybeep;
	required_device<beep_device> m_beeper;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_vrmm;
	required_ioport m_vram_config;
	required_ioport_array<16> m_keys;
	required_ioport m_ctrl;

	std::unique_ptr<uint8_t[]> m_gvram;
	std::unique_ptr<uint32_t[]> m_parity;
	uint8_t m_kbd_col;
	uint8_t m_vram_enable;
	uint8_t m_gbank;
	uint8_t m_display_mask;
	uint8_t m_flash;
	uint8_t m_clr_val;
	uint8_t m_tmr_status;
	uint8_t m_start_addr;
	bool m_vidint_enable;
	uint8_t m_memory_ctrl;

	floppy_image_device *m_floppy;
};


void z100_state::machine_start()
{
	m_parity = make_unique_clear<uint32_t[]>(m_ram.bytes() / 32);
}

void z100_state::video_start()
{
	m_gvram = make_unique_clear<uint8_t[]>(0x30000);

	m_vidint_enable = false;
}

uint8_t z100_state::ram_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && BIT(m_memory_ctrl, 5))
	{
		uint32_t parity = m_parity[offset >> 5];
		if (BIT(parity, offset & 31))
			m_picm->ir0_w(1);
	}

	return m_ram[offset];
}

void z100_state::ram_w(offs_t offset, uint8_t data)
{
	if (!machine().side_effects_disabled())
	{
		uint32_t &parity = m_parity[offset >> 5];
		if (!BIT(m_memory_ctrl, 4) && BIT(population_count_32(data), 0))
			parity |= 1 << (offset & 31);
		else if (parity != 0)
			parity &= ~(1 << (offset & 31));
	}

	m_ram[offset] = data;
}

void z100_state::memory_ctrl_w(uint8_t data)
{
	m_memory_ctrl = data & 0x3f;
	if (!BIT(data, 5))
		m_picm->ir0_w(0);
}

MC6845_UPDATE_ROW(z100_state::update_row)
{
	uint32_t *const pix = &bitmap.pix(y);
	const uint16_t amask = m_vram_config->read() ? 0xfff : 0x7ff;

	for (int x = 0; x < x_count; x++)
	{
		for (int xi = 0; xi < 8; xi++)
		{
			int dot = 0;
			if (m_flash)
			{
				dot = m_display_mask;
			}
			else
			{
				for (int i = 0; i < 3; i++)
					dot |= ((m_gvram[((x + ma) & amask) << 4 | (ra & 0xf) | (0x10000*i)] >> (7-xi)) & 1) << i; // b, r, g

				if (x == cursor_x)
					dot ^= 7;

				dot &= m_display_mask;
			}

			pix[x*8+xi] = m_palette->pen(dot);
		}
	}
}

offs_t z100_state::vram_map(offs_t offset) const
{
	// Translate logical address to physical address
	return (offset & 0x30000) | (offset & 0x000f) << 4 | (offset & 0x0780) >> 7
		| ((m_vrmm[(offset & 0xf800) >> 8 | (offset & 0x0070) >> 4] + m_start_addr) & (m_vram_config->read() ? 0xff : 0x7f)) << 8;
}

uint8_t z100_state::z100_vram_r(offs_t offset)
{
	return m_gvram[vram_map(offset)];
}

void z100_state::z100_vram_w(offs_t offset, uint8_t data)
{
	if(m_vram_enable)
	{
		offset = vram_map(offset);
		m_gvram[offset] = data;

		for (int i = 0; i < 3; i++)
		{
			if (BIT(m_gbank, i))
				m_gvram[((offset) & 0xffff)+0x10000*i] = data;
		}
	}
}

void z100_state::z100_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x3ffff).rw(FUNC(z100_state::ram_r), FUNC(z100_state::ram_w)).share("ram"); // 128*2 KB RAM
//  map(0xb0000,0xbffff).rom(); // expansion ROM
	map(0xc0000, 0xeffff).rw(FUNC(z100_state::z100_vram_r), FUNC(z100_state::z100_vram_w)); // Blue / Red / Green
//  map(0xf0000,0xf0fff) // network card (NET-100)
//  map(0xf4000,0xf7fff) // MTRET-100 Firmware I expansion ROM
//  map(0xf8000,0xfbfff) // MTRET-100 Firmware II expansion ROM check ID 0x4550
	map(0xfc000, 0xfffff).rom().region("ipl", 0);
}

void z100_state::kbd_col_w(uint8_t data)
{
	m_kbd_col = data & 0x0f;

	m_keyclick->b_w(BIT(data, 7));
	m_keybeep->a_w((data & 0x82) == 0);
}

uint8_t z100_state::kbd_rows_r()
{
	if (m_kbd_col < 0x0c)
		return m_keys[m_kbd_col]->read();

	return 0xff;
}

int z100_state::kbd_shift_row_r()
{
	if ((m_kbd_col & 0x0c) == 0x0c)
		return m_keys[m_kbd_col]->read();

	return 1;
}

void z100_state::beep_update(int state)
{
	m_beeper->set_state(m_keyclick->q_r() | m_keybeep->q_r());
}

// todo: side select?

void z100_state::floppy_select_w(uint8_t data)
{
	m_floppy = m_floppies[data & 0x03]->get_device();
	m_fdc->set_floppy(m_floppy);
}

void z100_state::floppy_motor_w(uint8_t data)
{
	if (m_floppy)
		m_floppy->mon_w(!BIT(data, 1));
}

uint8_t z100_state::tmr_status_r()
{
	return m_tmr_status;
}

void z100_state::tmr_status_w(uint8_t data)
{
	m_tmr_status &= data & 3;
	if (m_tmr_status == 0)
		m_picm->ir2_w(0);
}

void z100_state::timer_flipflop0_w(int state)
{
	if (state)
	{
		m_tmr_status |= 1;
		m_picm->ir2_w(1);
	}
}

void z100_state::timer_flipflop1_w(int state)
{
	if (state)
	{
		m_tmr_status |= 2;
		m_picm->ir2_w(1);
	}
}

void z100_state::vidint_w(int state)
{
	m_pia[1]->pa4_w(state);

	if (state && m_vidint_enable)
		m_pia[1]->ca2_w(1);
}

void z100_state::vidint_enable_w(int state)
{
	m_vidint_enable = state;
	if (!m_vidint_enable)
		m_pia[1]->ca2_w(0);
}

void z100_state::z100_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
//  map(0x00, 0x3f) reserved for non-ZDS vendors
//  map(0x40, 0x5f) secondary Multiport card (Z-204)
//  map(0x60, 0x7f) primary Multiport card (Z-204)
//  map(0x80, 0x83) development board
//  map(0x98, 0x9f) Z-205 expansion memory boards
//  map(0xa0, 0xa3) network card (NET-100)
//  map(0xa4, 0xa7) gateway (reserved)
//  map(0xac, 0xad) Z-217 secondary disk controller (winchester)
//  map(0xae, 0xaf) Z-217 primary disk controller (winchester)
	map(0xb0, 0xb3).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0xb4, 0xb4).w(FUNC(z100_state::floppy_select_w));
	map(0xb5, 0xb5).w(FUNC(z100_state::floppy_motor_w));
//  z-207 secondary disk controller (wd1797)
//  map(0xcd, 0xce) ET-100 CRT Controller
//  map(0xd4, 0xd7) ET-100 Trainer Parallel I/O
	map(0xd8, 0xdb).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write)); //video board
	map(0xdc, 0xdc).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xdd, 0xdd).w(m_crtc, FUNC(mc6845_device::register_w));
//  map(0xde, 0xde) light pen
	map(0xe0, 0xe3).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write)); //main board
	map(0xe4, 0xe7).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xe8, 0xeb).rw(m_epci[0], FUNC(scn2661b_device::read), FUNC(scn2661b_device::write));
	map(0xec, 0xef).rw(m_epci[1], FUNC(scn2661b_device::read), FUNC(scn2661b_device::write));
	map(0xf0, 0xf1).rw(m_pics, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf2, 0xf3).rw(m_picm, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf4, 0xf5).rw("kbdc", FUNC(i8041a_device::upi41_master_r), FUNC(i8041a_device::upi41_master_w));
//  map(0xf6, 0xf6) expansion ROM is present (bit 0, active low)
	map(0xfb, 0xfb).rw(FUNC(z100_state::tmr_status_r), FUNC(z100_state::tmr_status_w));
	map(0xfc, 0xfc).w(FUNC(z100_state::memory_ctrl_w));
//  map(0xfd, 0xfd) Hi-address latch
//  map(0xfe, 0xfe) Processor swap port
	map(0xff, 0xff).portr("DSW101");
}

INPUT_CHANGED_MEMBER(z100_state::kbd_reset)
{
	if (m_ctrl->read() == 0)
		reset();
}

/* Input ports */
INPUT_PORTS_START( z100 )
	PORT_START("COL0") // 15
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)

	PORT_START("COL1") // 11
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)

	PORT_START("COL2") // 16
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)

	PORT_START("COL3") // 13
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL4") // 9
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CODE(KEYCODE_F7)

	PORT_START("COL5") // 12
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del/Ins Char") PORT_CODE(KEYCODE_F14)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del/Ins Line") PORT_CODE(KEYCODE_F13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL6") // 17
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)

	PORT_START("COL7") // 18
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("COL8") // 3
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RALT) PORT_NAME("Line Feed")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CODE(KEYCODE_DEL) PORT_NAME("Delete")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("COL9") // 7
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Return")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)

	PORT_START("COL10") // 10
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)

	PORT_START("COL11") // 14
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Back Space")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE) // ~ key is between BS and =
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)

	PORT_START("COL12") // 5
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Shift Left")

	PORT_START("COL13") // 8
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Shift Right")

	PORT_START("COL14") // 6
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK)

	PORT_START("COL15") // 1
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Fast Repeat")

	PORT_START("CTRL") // 2 & 4
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Ctrl") PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, kbd_reset, 0)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, kbd_reset, 0)

	PORT_START("DSW101")
	PORT_DIPNAME( 0x07, 0x00, "Default Auto-boot Device" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x08, 0x08, "Auto-boot" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) // Reserved
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) // Reserved
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) // Reserved
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Monitor" )
	PORT_DIPSETTING(    0x80, "PAL 50 Hz" )
	PORT_DIPSETTING(    0x00, "NTSC 60 Hz" )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Video Board" )
	PORT_CONFSETTING( 0x00, "Monochrome" )
	PORT_CONFSETTING( 0x01, "Color" )

	PORT_START("VRAM")
	PORT_DIPNAME( 0x01, 0x01, "Video Memory" ) PORT_DIPLOCATION("J307:1")
	PORT_DIPSETTING( 0x00, "32K" )
	PORT_DIPSETTING( 0x01, "64K" )
INPUT_PORTS_END

u8 z100_state::get_slave_ack(offs_t offset)
{
	if (offset==7) { // IRQ = 7
		return m_pics->acknowledge();
	}
	return 0;
}

void z100_state::video_pia_A_w(u8 data)
{
	/*
	all bits are active low
	x--- ---- -> disable video RAM
	-x-- ---- -> not write multiple blue
	--x- ---- -> not write multiple green
	---x ---- -> not write multiple red
	---- x--- -> not flash screen
	---- -x-- -> enable blue display
	---- --x- -> enable green display
	---- ---x -> enable red display
	*/

	m_vram_enable = ((data & 0x80) >> 7) ^ 1;
	m_gbank = bitswap<8>(((data & 0x70) >> 4) ^ 0x7,7,6,5,4,3,1,0,2);
	m_flash = ((data & 8) >> 3) ^ 1;
	m_display_mask = bitswap<8>((data & 7) ^ 7,7,6,5,4,3,1,0,2);
}

void z100_state::video_pia_B_w(u8 data)
{
	m_start_addr = data;
}

/* clear screen */
void z100_state::video_pia_CA2_w(int state)
{
	int i;

	for(i=0; i<0x30000; i++)
		m_gvram[i] = m_clr_val;
}

void z100_state::video_pia_CB2_w(int state)
{
	m_clr_val = (state & 1) ? 0x00 : 0xff;
}

void z100_state::machine_reset()
{
	int i;

	if(ioport("CONFIG")->read() & 1)
	{
		for(i=0;i<8;i++)
			m_palette->set_pen_color(i,pal1bit(i >> 1),pal1bit(i >> 2),pal1bit(i >> 0));
	}
	else
	{
		for(i=0;i<8;i++)
			m_palette->set_pen_color(i,pal3bit(0),pal3bit(i),pal3bit(0));
	}

	memory_ctrl_w(0);
}

static void z100_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_525_DD);
}

void z100_state::z100(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 15_MHz_XTAL / 3); // 5 MHz or 8 MHz depending on XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &z100_state::z100_mem);
	m_maincpu->set_addrmap(AS_IO, &z100_state::z100_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	I8085A(config, "cpu85", 10_MHz_XTAL).set_disable();

	i8041a_device &kbdc(I8041A(config, "kbdc", 6_MHz_XTAL));
	kbdc.p1_in_cb().set(FUNC(z100_state::kbd_rows_r));
	kbdc.p2_out_cb().set(FUNC(z100_state::kbd_col_w));
	kbdc.p2_out_cb().append("keydspyint", FUNC(input_merger_device::in_w<0>)).bit(4);
	kbdc.t0_in_cb().set_ioport("CTRL").bit(0);
	kbdc.t1_in_cb().set(FUNC(z100_state::kbd_shift_row_r));

	TTL74123(config, m_keyclick, RES_K(150), CAP_U(.1));
	m_keyclick->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_keyclick->set_a_pin_value(0);
	m_keyclick->set_b_pin_value(1);
	m_keyclick->set_clear_pin_value(1);
	m_keyclick->out_cb().set(FUNC(z100_state::beep_update));

	TTL74123(config, m_keybeep, RES_K(220), CAP_U(2.2));
	m_keybeep->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_keybeep->set_b_pin_value(1);
	m_keybeep->set_clear_pin_value(1);
	m_keybeep->out_cb().set(FUNC(z100_state::beep_update));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 1'000'000'000 / PERIOD_OF_555_ASTABLE_NSEC(RES_K(470), RES_K(470), CAP_U(.001)));
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.50);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.112_MHz_XTAL, 912, 0, 640, 258, 0, 216);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette).set_entries(8);

	/* devices */
	MC6845(config, m_crtc, 14.112_MHz_XTAL / 8); // 68A45
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(z100_state::update_row));
	m_crtc->out_vsync_callback().set(FUNC(z100_state::vidint_w));

	PIC8259(config, m_picm);
	m_picm->out_int_callback().set_inputline(m_maincpu, 0);
	m_picm->in_sp_callback().set_constant(1);
	m_picm->read_slave_ack_callback().set(FUNC(z100_state::get_slave_ack));

	PIC8259(config, m_pics);
	m_pics->out_int_callback().set(m_picm, FUNC(pic8259_device::ir3_w));
	m_pics->in_sp_callback().set_constant(0);

	pit8253_device &pit(PIT8253(config, "pit"));
	pit.set_clk<0>(4_MHz_XTAL / 16);
	pit.out_handler<0>().set(FUNC(z100_state::timer_flipflop0_w));
	pit.out_handler<0>().append("pit", FUNC(pit8253_device::write_clk1));
	pit.set_clk<2>(4_MHz_XTAL / 16);
	pit.out_handler<2>().set(FUNC(z100_state::timer_flipflop1_w));

	PIA6821(config, m_pia[0]);
	m_pia[0]->writepa_handler().set(FUNC(z100_state::video_pia_A_w));
	m_pia[0]->writepb_handler().set(FUNC(z100_state::video_pia_B_w));
	m_pia[0]->ca2_handler().set(FUNC(z100_state::video_pia_CA2_w));
	m_pia[0]->cb2_handler().set(FUNC(z100_state::video_pia_CB2_w));

	PIA6821(config, m_pia[1]);
	m_pia[1]->irqa_handler().set("keydspyint", FUNC(input_merger_device::in_w<1>));
	m_pia[1]->irqb_handler().set(m_picm, FUNC(pic8259_device::ir7_w));
	m_pia[1]->writepa_handler().set("centronics", FUNC(centronics_device::write_strobe)).bit(2);
	m_pia[1]->writepa_handler().append("centronics", FUNC(centronics_device::write_data0)).bit(0);
	m_pia[1]->writepa_handler().append("centronics", FUNC(centronics_device::write_data1)).bit(1);
	m_pia[1]->writepa_handler().append("centronics", FUNC(centronics_device::write_init)).bit(3);
	m_pia[1]->writepa_handler().append(FUNC(z100_state::vidint_enable_w)).bit(5);
	m_pia[1]->writepb_handler().set("centronics", FUNC(centronics_device::write_data2)).bit(2);
	m_pia[1]->writepb_handler().append("centronics", FUNC(centronics_device::write_data3)).bit(3);
	m_pia[1]->writepb_handler().append("centronics", FUNC(centronics_device::write_data4)).bit(4);
	m_pia[1]->writepb_handler().append("centronics", FUNC(centronics_device::write_data5)).bit(5);
	m_pia[1]->writepb_handler().append("centronics", FUNC(centronics_device::write_data6)).bit(6);
	m_pia[1]->writepb_handler().append("centronics", FUNC(centronics_device::write_data7)).bit(7);

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, nullptr));
	centronics.ack_handler().set(m_pia[1], FUNC(pia6821_device::cb1_w)).invert();
	centronics.busy_handler().set(m_pia[1], FUNC(pia6821_device::cb2_w));
	centronics.busy_handler().append(m_pia[1], FUNC(pia6821_device::pb0_w));
	centronics.perror_handler().set(m_pia[1], FUNC(pia6821_device::pb1_w));

	input_merger_device &keydspyint(INPUT_MERGER_ANY_HIGH(config, "keydspyint"));
	keydspyint.output_handler().set(m_picm, FUNC(pic8259_device::ir6_w));

	FD1797(config, m_fdc, 4_MHz_XTAL / 4);

	FLOPPY_CONNECTOR(config, m_floppies[0], z100_floppies, "dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[1], z100_floppies, "dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[2], z100_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[3], z100_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	SCN2661B(config, m_epci[0], 4.9152_MHz_XTAL); // First 2661-2 serial port (printer)
	m_epci[0]->txrdy_handler().set("epci0int", FUNC(input_merger_device::in_w<0>));
	m_epci[0]->rxrdy_handler().set("epci0int", FUNC(input_merger_device::in_w<1>));

	SCN2661B(config, m_epci[1], 4.9152_MHz_XTAL); // Second 2661-2 serial port (modem)
	m_epci[1]->txrdy_handler().set("epci1int", FUNC(input_merger_device::in_w<0>));
	m_epci[1]->rxrdy_handler().set("epci1int", FUNC(input_merger_device::in_w<1>));

	input_merger_device &epci0int(INPUT_MERGER_ANY_HIGH(config, "epci0int"));
	epci0int.output_handler().set(m_picm, FUNC(pic8259_device::ir4_w));

	input_merger_device &epci1int(INPUT_MERGER_ANY_HIGH(config, "epci1int"));
	epci1int.output_handler().set(m_picm, FUNC(pic8259_device::ir5_w));
}

/* ROM definition */
ROM_START( z100 )
	ROM_REGION(0x4000, "ipl", 0)
	ROM_LOAD("intel-d27128-1.bin", 0x0000, 0x4000, CRC(b21f0392) SHA1(69e492891cceb143a685315efe0752981a2d8143))

	ROM_REGION(0x0400, "kbdc", 0) // 8041A keyboard controller
	ROM_LOAD("444-109.u204", 0x0000, 0x0400, CRC(45181029) SHA1(0e89649364d25cf2d8669d2a293ee162e274cb64))

	// All PROMs are typed in from Zenith technical manual listings rather than dumped from real devices, and are accordingly marked BAD_DUMP
	ROM_REGION(0x0100, "iodec", 0) // 82S129 I/O Decoder PROM
	ROM_LOAD("444-101.u179", 0x0000, 0x0100, CRC(c952be82) SHA1(0edf9265d302f8478a310858eb6a9352f0cda17b) BAD_DUMP)

	ROM_REGION(0x0100, "memdec", 0) // 82S129 Memory Decoder PROM
	ROM_LOAD("444-104.u111", 0x0000, 0x0100, CRC(46edd69d) SHA1(5d4bafeaa4593e419bf94dba9e44c8b2be58727b) BAD_DUMP)

	ROM_REGION(0x0020, "status", 0) // 82S123 CPU Status Decode PROM
	ROM_LOAD("444-105.u226", 0x0000, 0x0020, CRC(98b084e9) SHA1(d968b9a1b1d2ba3ed40036c2192c9960a6c15e99) BAD_DUMP)

	ROM_REGION(0x0100, "vramsel", 0) // 82S129 Video RAM Select PROM
	ROM_LOAD("444-102.u371", 0x0000, 0x0100, CRC(4558f540) SHA1(55c9bad87b111537a6d386a6eb405169fb47304c) BAD_DUMP)

	ROM_REGION(0x0100, "viosel", 0) // 82S129 Video I/O Select PROM
	ROM_LOAD("444-103.u369", 0x0000, 0x0100, CRC(854cef15) SHA1(836b244dac0085bcfe8006fde0c5f19982969236) BAD_DUMP)

	ROM_REGION(0x0100, "vrmm", 0) // TBP18S22 Video RAM Mapping Module
	ROM_LOAD("444-127.u370", 0x0000, 0x0100, CRC(ac386f6b) SHA1(2b62b939d704d90edf59923a8a1a51ef1902f4d7) BAD_DUMP)
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY                FULLNAME  FLAGS
COMP( 1982, z100, 0,      0,      z100,    z100,  z100_state, empty_init, "Zenith Data Systems", "Z-100",  MACHINE_NOT_WORKING )
