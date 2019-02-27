// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Zenith Z-100

    15/07/2011 Skeleton driver.

    Commands:
    Press H to list all the commands.

    TODO:
    - remove parity check IRQ patch (understand what it really wants there!)
    - implement S-100 bus features;
    - irqs needs 8259 "auto-ack"-ing in order to work properly;
    - vertical scrolling isn't understood;

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
//#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/mc2661.h"
#include "machine/pic8259.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"

class z100_state : public driver_device
{
public:
	z100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia(*this, "pia%u", 0U),
		m_picm(*this, "pic8259_master"),
		m_pics(*this, "pic8259_slave"),
		m_fdc(*this, "z207_fdc"),
		m_floppies(*this, "z207_fdc:%u", 0U),
		m_epci(*this, "epci%u", 0U),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_floppy(nullptr)
	{ }

	void z100(machine_config &config);

	virtual void driver_init() override;

	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

private:
	virtual void machine_reset() override;
	virtual void video_start() override;

	DECLARE_READ8_MEMBER(z100_vram_r);
	DECLARE_WRITE8_MEMBER(z100_vram_w);
	DECLARE_READ8_MEMBER(keyb_data_r);
	DECLARE_READ8_MEMBER(keyb_status_r);
	DECLARE_WRITE8_MEMBER(keyb_command_w);
	DECLARE_WRITE8_MEMBER(z100_6845_address_w);
	DECLARE_WRITE8_MEMBER(z100_6845_data_w);
	DECLARE_WRITE8_MEMBER(floppy_select_w);
	DECLARE_WRITE8_MEMBER(floppy_motor_w);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE8_MEMBER(video_pia_A_w);
	DECLARE_WRITE8_MEMBER(video_pia_B_w);
	DECLARE_WRITE_LINE_MEMBER(video_pia_CA2_w);
	DECLARE_WRITE_LINE_MEMBER(video_pia_CB2_w);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void z100_io(address_map &map);
	void z100_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<pic8259_device> m_picm;
	required_device<pic8259_device> m_pics;
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	required_device_array<mc2661_device, 2> m_epci;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;

	std::unique_ptr<uint8_t[]> m_gvram;
	uint8_t m_keyb_press;
	uint8_t m_keyb_status;
	uint8_t m_vram_enable;
	uint8_t m_gbank;
	uint8_t m_display_mask;
	uint8_t m_flash;
	uint8_t m_clr_val;
	uint8_t m_crtc_vreg[0x100];
	uint8_t m_crtc_index;
	uint16_t m_start_addr;

	floppy_image_device *m_floppy;
};

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0xff00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0xff00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0xff00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0xff00) | (m_crtc_vreg[0x13] & 0xff))


void z100_state::video_start()
{
	m_gvram = make_unique_clear<uint8_t[]>(0x30000);
}

uint32_t z100_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,xi,yi;
	int dot;
	int i;

	bitmap.fill(0, cliprect);

	for(y=0;y<mc6845_v_display;y++)
	{
		for(x=0;x<mc6845_h_display;x++)
		{
			uint32_t base_offs;

			for(yi=0;yi<mc6845_tile_height;yi++)
			{
				base_offs = y * 0x800 + yi * 0x80 + x;

				for(xi=0;xi<8;xi++)
				{
					dot = 0;
					for(i=0;i<3;i++)
						dot |= ((m_gvram[(base_offs & 0xffff)+0x10000*i] >> (7-xi)) & 1) << i; // b, r, g

					dot &= m_display_mask;

					/* overwrite */
					if(m_flash)
						dot = m_display_mask;

					if(y*mc6845_tile_height+yi < 216 && x*8+xi < 640) /* TODO: safety check */
						bitmap.pix16(y*mc6845_tile_height+yi, x*8+xi) = m_palette->pen(dot);
				}
			}
		}
	}

	return 0;
}

READ8_MEMBER( z100_state::z100_vram_r )
{
	return m_gvram[offset];
}

WRITE8_MEMBER( z100_state::z100_vram_w )
{
	if(m_vram_enable)
	{
		int i;

		m_gvram[offset] = data;

		for(i=0;i<3;i++)
		{
			if(m_gbank & (1 << i))
				m_gvram[((offset) & 0xffff)+0x10000*i] = data;
		}
	}
}

void z100_state::z100_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x3ffff).ram(); // 128*2 KB RAM
//  AM_RANGE(0xb0000,0xbffff) AM_ROM // expansion ROM
	map(0xc0000, 0xeffff).rw(FUNC(z100_state::z100_vram_r), FUNC(z100_state::z100_vram_w)); // Blue / Red / Green
//  AM_RANGE(0xf0000,0xf0fff) // network card (NET-100)
//  AM_RANGE(0xf4000,0xf7fff) // MTRET-100 Firmware I expansion ROM
//  AM_RANGE(0xf8000,0xfbfff) // MTRET-100 Firmware II expansion ROM check ID 0x4550
	map(0xfc000, 0xfffff).rom().region("ipl", 0);
}

READ8_MEMBER( z100_state::keyb_data_r )
{
	if(m_keyb_press)
	{
		uint8_t res;

		res = m_keyb_press;

		m_keyb_press = 0;
		return res;
	}

	return m_keyb_press;
}

READ8_MEMBER( z100_state::keyb_status_r )
{
	if(m_keyb_status)
	{
		m_keyb_status = 0;
		return 1;
	}

	return m_keyb_status;
}

WRITE8_MEMBER( z100_state::keyb_command_w )
{
	// ...
}

WRITE8_MEMBER( z100_state::z100_6845_address_w )
{
	data &= 0x1f;
	m_crtc_index = data;
	m_crtc->address_w(data);
}

WRITE8_MEMBER( z100_state::z100_6845_data_w )
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(data);
}

// todo: side select?

WRITE8_MEMBER( z100_state::floppy_select_w )
{
	m_floppy = m_floppies[data & 0x03]->get_device();
	m_fdc->set_floppy(m_floppy);
}

WRITE8_MEMBER( z100_state::floppy_motor_w )
{
	if (m_floppy)
		m_floppy->mon_w(!BIT(data, 1));
}

void z100_state::z100_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
//  AM_RANGE (0x00, 0x3f) reserved for non-ZDS vendors
//  AM_RANGE (0x40, 0x5f) secondary Multiport card (Z-204)
//  AM_RANGE (0x60, 0x7f) primary Multiport card (Z-204)
//  AM_RANGE (0x80, 0x83) development board
//  AM_RANGE (0x98, 0x9f) Z-205 expansion memory boards
//  AM_RANGE (0xa0, 0xa3) network card (NET-100)
//  AM_RANGE (0xa4, 0xa7) gateway (reserved)
//  AM_RANGE (0xac, 0xad) Z-217 secondary disk controller (winchester)
//  AM_RANGE (0xae, 0xaf) Z-217 primary disk controller (winchester)
	map(0xb0, 0xb3).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0xb4, 0xb4).w(FUNC(z100_state::floppy_select_w));
	map(0xb5, 0xb5).w(FUNC(z100_state::floppy_motor_w));
//  z-207 secondary disk controller (wd1797)
//  AM_RANGE (0xcd, 0xce) ET-100 CRT Controller
//  AM_RANGE (0xd4, 0xd7) ET-100 Trainer Parallel I/O
	map(0xd8, 0xdb).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write)); //video board
	map(0xdc, 0xdc).w(FUNC(z100_state::z100_6845_address_w));
	map(0xdd, 0xdd).w(FUNC(z100_state::z100_6845_data_w));
//  AM_RANGE (0xde, 0xde) light pen
	map(0xe0, 0xe3).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write)); //main board
//  AM_RANGE (0xe4, 0xe7) 8253 PIT
	map(0xe8, 0xeb).rw(m_epci[0], FUNC(mc2661_device::read), FUNC(mc2661_device::write));
	map(0xec, 0xef).rw(m_epci[1], FUNC(mc2661_device::read), FUNC(mc2661_device::write));
	map(0xf0, 0xf1).rw(m_pics, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf2, 0xf3).rw(m_picm, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf4, 0xf4).r(FUNC(z100_state::keyb_data_r)); // -> 8041 MCU
	map(0xf5, 0xf5).rw(FUNC(z100_state::keyb_status_r), FUNC(z100_state::keyb_command_w));
//  AM_RANGE (0xf6, 0xf6) expansion ROM is present (bit 0, active low)
//  AM_RANGE (0xfb, 0xfb) timer irq status
//  AM_RANGE (0xfc, 0xfc) memory latch
//  AM_RANGE (0xfd, 0xfd) Hi-address latch
//  AM_RANGE (0xfe, 0xfe) Processor swap port
	map(0xff, 0xff).portr("DSW101");
}

INPUT_CHANGED_MEMBER(z100_state::key_stroke)
{
	if(newval && !oldval)
	{
		/* TODO: table */
		m_keyb_press = (uint8_t)(uintptr_t)(param) & 0xff;
		//m_picm->ir6_w(1);
		m_keyb_status = 1;
	}

	if(oldval && !newval)
	{
		m_keyb_press = 0;
		m_keyb_status = 0;
	}
}

/* Input ports */
INPUT_PORTS_START( z100 )
	PORT_START("KEY0") // 0x00 - 0x07
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-1") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x01)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-2") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x02)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-3") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x03)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-4") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x04)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x05)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x06)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x07)

	PORT_START("KEY1") // 0x08 - 0x0f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-0") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x08)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-1") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x09)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-2") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x0a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-3") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x0b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-4") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x0c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x0d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x0e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x0f)

	PORT_START("KEY2") // 0x10 - 0x17
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-0") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x10)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-1") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x11)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-2") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x12)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-3") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x13)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-4") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x14)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x15)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x16)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x17)

	PORT_START("KEY3") // 0x18 - 0x1f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-0") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x18)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-1") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x19)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-2") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x1a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-3") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x1b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-4") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x1c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x1d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x1e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x1f)

	PORT_START("KEY4") // 0x20 - 0x27
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-0") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x20)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-1") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x21)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-2") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x22)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-3") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x23)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-4") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x24)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x25)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x26)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x27)

	PORT_START("KEY5") // 0x28 - 0x2f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-0") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x28)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-1") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x29)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-2") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x2a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-3") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x2b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-4") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x2c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x2d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x2e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x2f)

	PORT_START("KEY6") // 0x30 - 0x37
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_IMPULSE(1) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x30)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x31)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x32)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x33)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x34)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x35)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x36)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x37)

	PORT_START("KEY7") // 0x38 - 0x3f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x38)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x39)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-2") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x3a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-3") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x3b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-4") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x3c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x3d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x3e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x3f)

	PORT_START("KEY8") // 0x40 - 0x47
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8-0") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x60)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x61)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x62)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x63)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x64)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x65)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x66)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x67)

	PORT_START("KEY9") // 0x48 - 0x4f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x68)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x69)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x6a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x6b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x6c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x6d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x6e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x6f)

	PORT_START("KEYA") // 0x50 - 0x57
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x70)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x71)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x72)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x73)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x74)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x75)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x76)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x77)

	PORT_START("KEYB") // 0x58 - 0x5f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x78)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x79)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x7a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-4") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x7b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x7c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x7d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x7e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DELETE key") PORT_CODE(KEYCODE_DEL) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x7f)

	PORT_START("KEYC") // 0x58 - 0x5f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HELP key") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, z100_state, key_stroke, 0x95)

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
INPUT_PORTS_END

READ8_MEMBER( z100_state::get_slave_ack )
{
	if (offset==7) { // IRQ = 7
		return m_pics->acknowledge();
	}
	return 0;
}

WRITE8_MEMBER( z100_state::video_pia_A_w )
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

WRITE8_MEMBER( z100_state::video_pia_B_w )
{
	m_start_addr = data << 4; //<- TODO
}

/* clear screen */
WRITE_LINE_MEMBER( z100_state::video_pia_CA2_w )
{
	int i;

	for(i=0; i<0x30000; i++)
		m_gvram[i] = m_clr_val;
}

WRITE_LINE_MEMBER( z100_state::video_pia_CB2_w )
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
}

static void z100_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_525_DD);
}

void z100_state::z100(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 14.318181_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &z100_state::z100_mem);
	m_maincpu->set_addrmap(AS_IO, &z100_state::z100_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(z100_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(8);

	/* devices */
	MC6845(config, m_crtc, 14.318181_MHz_XTAL / 8);    /* unknown clock, hand tuned to get ~50/~60 fps */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	PIC8259(config, m_picm, 0);
	m_picm->out_int_callback().set_inputline(m_maincpu, 0);
	m_picm->in_sp_callback().set_constant(1);
	m_picm->read_slave_ack_callback().set(FUNC(z100_state::get_slave_ack));

	PIC8259(config, m_pics, 0);
	m_pics->out_int_callback().set(m_picm, FUNC(pic8259_device::ir3_w));
	m_pics->in_sp_callback().set_constant(0);

	PIA6821(config, m_pia[0], 0);
	m_pia[0]->writepa_handler().set(FUNC(z100_state::video_pia_A_w));
	m_pia[0]->writepb_handler().set(FUNC(z100_state::video_pia_B_w));
	m_pia[0]->ca2_handler().set(FUNC(z100_state::video_pia_CA2_w));
	m_pia[0]->cb2_handler().set(FUNC(z100_state::video_pia_CB2_w));

	PIA6821(config, m_pia[1], 0);

	FD1797(config, m_fdc, 1_MHz_XTAL);

	FLOPPY_CONNECTOR(config, m_floppies[0], z100_floppies, "dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[1], z100_floppies, "dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[2], z100_floppies, "", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[3], z100_floppies, "", floppy_image_device::default_floppy_formats);

	MC2661(config, m_epci[0], 4.9152_MHz_XTAL); // First 2661-2 serial port (printer)
	m_epci[0]->txrdy_handler().set("epci0int", FUNC(input_merger_device::in_w<0>));
	m_epci[0]->rxrdy_handler().set("epci0int", FUNC(input_merger_device::in_w<1>));

	MC2661(config, m_epci[1], 4.9152_MHz_XTAL); // Second 2661-2 serial port (modem)
	m_epci[1]->txrdy_handler().set("epci1int", FUNC(input_merger_device::in_w<0>));
	m_epci[1]->rxrdy_handler().set("epci1int", FUNC(input_merger_device::in_w<1>));

	input_merger_device &epci0int(INPUT_MERGER_ANY_HIGH(config, "epci0int"));
	epci0int.output_handler().set(m_picm, FUNC(pic8259_device::ir4_w));

	input_merger_device &epci1int(INPUT_MERGER_ANY_HIGH(config, "epci1int"));
	epci1int.output_handler().set(m_picm, FUNC(pic8259_device::ir5_w));
}

/* ROM definition */
ROM_START( z100 )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "intel-d27128-1.bin", 0x0000, 0x4000, CRC(b21f0392) SHA1(69e492891cceb143a685315efe0752981a2d8143))

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF ) //8041
	ROM_LOAD( "mcu", 0x0000, 0x1000, NO_DUMP )
ROM_END

void z100_state::driver_init()
{
	uint8_t *ROM = memregion("ipl")->base();

	ROM[0xfc116 & 0x3fff] = 0x90; // patch parity IRQ check
	ROM[0xfc117 & 0x3fff] = 0x90;

	ROM[0xfc0d4 & 0x3fff] = 0x75; // patch checksum
}


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT         COMPANY                FULLNAME  FLAGS
COMP( 1982, z100, 0,      0,      z100,    z100,  z100_state, driver_init, "Zenith Data Systems", "Z-100",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
