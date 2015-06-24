// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// ***************************************
// Driver for HP 64000 development system
// ***************************************
//
// Documentation used for this driver:
// [1]  HP, manual 64100-90910, dec 83 rev. - Model 64100A mainframe service manual
//
// TODO:
// - Slot selection mechanism & low 32KW RAM
// - Periodic interrupt
// - Beeper
// - Various DIP switches
// - Floppy I/F
// - RS232 I/F

#include "emu.h"
#include "cpu/hphybrid/hphybrid.h"
#include "video/i8275.h"

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

class hp64k_state : public driver_device
{
public:
		hp64k_state(const machine_config &mconfig, device_type type, const char *tag);

		//virtual void driver_start();
		//virtual void machine_start();
		virtual void video_start();
		virtual void machine_reset();

		UINT8 hp64k_crtc_filter(UINT8 data);
		DECLARE_WRITE16_MEMBER(hp64k_crtc_w);
		DECLARE_WRITE_LINE_MEMBER(hp64k_crtc_drq_w);
		DECLARE_WRITE_LINE_MEMBER(hp64k_crtc_vrtc_w);

		I8275_DRAW_CHARACTER_MEMBER(crtc_display_pixels);

		DECLARE_READ16_MEMBER(hp64k_rear_sw_r);

		IRQ_CALLBACK_MEMBER(hp64k_irq_callback);
		void hp64k_update_irl(void);
		DECLARE_WRITE16_MEMBER(hp64k_irl_mask_w);

		TIMER_DEVICE_CALLBACK_MEMBER(hp64k_kb_scan);
		DECLARE_READ16_MEMBER(hp64k_kb_r);

private:
		required_device<hp_5061_3011_cpu_device> m_cpu;
		required_device<i8275_device> m_crtc;
		required_device<palette_device> m_palette;
		required_ioport m_io_key0;
		required_ioport m_io_key1;
		required_ioport m_io_key2;
		required_ioport m_io_key3;

		// Character generator
		const UINT8 *m_chargen;

		UINT32 m_crtc_ptr;
		bool m_crtc_drq;
		bool m_vrtc;

		// Interrupt handling
		UINT8 m_irl_mask;
		UINT8 m_irl_pending;

		// State of keyboard
		ioport_value m_kb_state[ 4 ];
		UINT8 m_kb_row_col;
		bool m_kb_scan_on;
		bool m_kb_pressed;
};

static ADDRESS_MAP_START(cpu_mem_map , AS_PROGRAM , 16 , hp64k_state)
		AM_RANGE(0x0000 , 0x3fff) AM_ROM
		AM_RANGE(0x8000 , 0x8001) AM_WRITE(hp64k_crtc_w)
		AM_RANGE(0x8002 , 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(cpu_io_map , AS_IO , 16 , hp64k_state)
		// PA = 0, IC = [0..3]
		// Keyboard input
		AM_RANGE(HP_MAKE_IOADDR(0 , 0) , HP_MAKE_IOADDR(0 , 3))   AM_READ(hp64k_kb_r)
		// PA = 7, IC = 2
		// Rear-panel switches
		AM_RANGE(HP_MAKE_IOADDR(7 , 2) , HP_MAKE_IOADDR(7 , 2))   AM_READ(hp64k_rear_sw_r)
		// PA = 12, IC = [0..3]
		// Interrupt mask
		AM_RANGE(HP_MAKE_IOADDR(12 , 0) , HP_MAKE_IOADDR(12 , 3)) AM_WRITE(hp64k_irl_mask_w)
ADDRESS_MAP_END

hp64k_state::hp64k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig , type , tag),
		m_cpu(*this , "cpu"),
		m_crtc(*this , "crtc"),
		m_palette(*this , "palette"),
		m_io_key0(*this , "KEY0"),
		m_io_key1(*this , "KEY1"),
		m_io_key2(*this , "KEY2"),
		m_io_key3(*this , "KEY3")
{
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
}

UINT8 hp64k_state::hp64k_crtc_filter(UINT8 data)
{
		bool inv = (data & 0xe0) == 0xe0;

		return inv ? (data & 0xf2) : data;
}

WRITE16_MEMBER(hp64k_state::hp64k_crtc_w)
{
		m_crtc->write(space , offset == 0 , hp64k_crtc_filter((UINT8)data));
}

WRITE_LINE_MEMBER(hp64k_state::hp64k_crtc_drq_w)
{
		bool crtc_drq = state != 0;
		bool prev_crtc = m_crtc_drq;
		m_crtc_drq = crtc_drq;

		if (!prev_crtc && crtc_drq) {
				address_space& prog_space = m_cpu->space(AS_PROGRAM);

				UINT8 data = prog_space.read_byte(m_crtc_ptr);
				m_crtc_ptr++;

				m_crtc->dack_w(prog_space , 0 , hp64k_crtc_filter(data));
		}
}

WRITE_LINE_MEMBER(hp64k_state::hp64k_crtc_vrtc_w)
{
		bool vrtc = state != 0;

		if (!m_vrtc && vrtc) {
				m_crtc_ptr = 0xf9f0 << 1;
		}
		m_vrtc = vrtc;
}

I8275_DRAW_CHARACTER_MEMBER(hp64k_state::crtc_display_pixels)
{
		const rgb_t *palette = m_palette->palette()->entry_list_raw();
		UINT8 chargen_byte = m_chargen[ linecount  | ((unsigned)charcode << 4) ];
		bool lvid , livid;
		UINT16 pixels_lvid , pixels_livid;
		unsigned i;

		if (vsp) {
				pixels_lvid = pixels_livid = ~0;
		} else if (lten) {
				pixels_livid = ~0;
				if (rvv) {
						pixels_lvid = ~0;
				} else {
						pixels_lvid = 0;
				}
		} else if (rvv) {
				pixels_lvid = ~0;
				pixels_livid = (UINT16)chargen_byte << 1;
		} else {
				pixels_lvid = ~((UINT16)chargen_byte << 1);
				pixels_livid = ~0;
		}

		for (i = 0; i < 9; i++) {
				lvid = (pixels_lvid & (1U << (8 - i))) != 0;
				livid = (pixels_livid & (1U << (8 - i))) != 0;

				if (!lvid) {
						// Normal brightness
						bitmap.pix32(y , x + i) = palette[ 2 ];
				} else if (livid) {
						// Black
						bitmap.pix32(y , x + i) = palette[ 0 ];
				} else {
						// Half brightness
						bitmap.pix32(y , x + i) = palette[ 1 ];
				}
		}

}

READ16_MEMBER(hp64k_state::hp64k_rear_sw_r)
{
		// TEST
		return ~0;
}

IRQ_CALLBACK_MEMBER(hp64k_state::hp64k_irq_callback)
{
		if (irqline == HPHYBRID_IRL) {
				return 0xff00 | (m_irl_mask & m_irl_pending);
		} else {
				return ~0;
		}
}

void hp64k_state::hp64k_update_irl(void)
{
		m_cpu->set_input_line(HPHYBRID_IRL , (m_irl_mask & m_irl_pending) != 0);
}

WRITE16_MEMBER(hp64k_state::hp64k_irl_mask_w)
{
		m_irl_mask = (UINT8)data;
		hp64k_update_irl();
}

TIMER_DEVICE_CALLBACK_MEMBER(hp64k_state::hp64k_kb_scan)
{
		if (m_kb_scan_on) {
				unsigned i;

				ioport_value input[ 4 ];
				input[ 0 ] = m_io_key0->read();
				input[ 1 ] = m_io_key1->read();
				input[ 2 ] = m_io_key2->read();
				input[ 3 ] = m_io_key3->read();

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

READ16_MEMBER(hp64k_state::hp64k_kb_r)
{
		UINT16 ret = 0xff00 | m_kb_row_col;

		if (m_kb_pressed) {
				BIT_SET(ret , 7);
		}

		m_kb_scan_on = true;
		BIT_CLR(m_irl_pending , 0);
		hp64k_update_irl();

		return ret;
}

static INPUT_PORTS_START(hp64k)
				// Keyboard is arranged in a 8 x 16 matrix. Of the 128 possible positions, only 77 are used.
				// For key arrangement on the matrix, see [1] pg 334
				// Keys are mapped on bit b of KEYn
				// where b = (row & 1) << 4 + column, n = row >> 1
				// column = [0..15]
				// row = [0..7]
				PORT_START("KEY0")
				PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)      PORT_CHAR(UCHAR_SHIFT_2)
				PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
				PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')
				PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
				PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')
				PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
				PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
				PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
				PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
				PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(13) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(14) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(15) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')
				PORT_BIT(BIT_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
				PORT_BIT(BIT_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(19) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(20) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')
				PORT_BIT(BIT_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')
				PORT_BIT(BIT_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')
				PORT_BIT(BIT_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0')
				PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('=')
				PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('^') PORT_CHAR('~')
				PORT_BIT(BIT_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)         PORT_CHAR('\\') PORT_CHAR('|')
				PORT_BIT(BIT_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)
				PORT_BIT(BIT_MASK(30) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(31) , IP_ACTIVE_HIGH , IPT_UNUSED)

				PORT_START("KEY1")
				PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
				PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('"')
				PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
				PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
				PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
				PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('&')
				PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)            PORT_NAME("RECALL")
				PORT_BIT(BIT_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)           PORT_NAME("CLRLINE")
				PORT_BIT(BIT_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)           PORT_NAME("CAPS")
				PORT_BIT(BIT_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)           PORT_NAME("RESET")
				PORT_BIT(BIT_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)            PORT_NAME("SK1")
				PORT_BIT(BIT_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)            PORT_NAME("SK2")
				PORT_BIT(BIT_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)            PORT_NAME("SK3")
				PORT_BIT(BIT_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)            PORT_NAME("SK4")
				PORT_BIT(BIT_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(22) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)            PORT_NAME("SK5")
				PORT_BIT(BIT_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)            PORT_NAME("SK6")
				PORT_BIT(BIT_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)            PORT_NAME("SK7")
				PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)            PORT_NAME("SK8")
				PORT_BIT(BIT_MASK(28) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(29) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(30) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(31) , IP_ACTIVE_HIGH , IPT_UNUSED)

				PORT_START("KEY2")
				PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)        PORT_CHAR(UCHAR_SHIFT_1)
				PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
				PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
				PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
				PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')
				PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
				PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')
				PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
				PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(13) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)        PORT_NAME("INSCHAR")
				PORT_BIT(BIT_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)           PORT_NAME("DELCHAR")
				PORT_BIT(BIT_MASK(16) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(17) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
				PORT_BIT(BIT_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
				PORT_BIT(BIT_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
				PORT_BIT(BIT_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(22) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
				PORT_BIT(BIT_MASK(24) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(25) , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('@') PORT_CHAR('`')
				PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('[') PORT_CHAR('{')
				PORT_BIT(BIT_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)    PORT_CHAR('_') PORT_CHAR(UCHAR_MAMEKEY(DEL))
				PORT_BIT(BIT_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)          PORT_NAME("ROLLUP")
				PORT_BIT(BIT_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)            PORT_CHAR(UCHAR_MAMEKEY(UP))
				PORT_BIT(BIT_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)          PORT_NAME("NEXTPG")

				PORT_START("KEY3")
				PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
				PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
				PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_UNUSED)
				PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
				PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
				PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR('+')
				PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR(':') PORT_CHAR('*')
				PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR(']') PORT_CHAR('}')
				PORT_BIT(BIT_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)         PORT_CHAR(13)
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

INPUT_PORTS_END

static MACHINE_CONFIG_START(hp64k , hp64k_state)
				MCFG_CPU_ADD("cpu" , HP_5061_3011 , 6250000)
				MCFG_CPU_PROGRAM_MAP(cpu_mem_map)
				MCFG_CPU_IO_MAP(cpu_io_map)
				MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(hp64k_state , hp64k_irq_callback)
				MCFG_QUANTUM_TIME(attotime::from_hz(100))

				// Actual keyboard refresh rate should be between 1 and 2 kHz
				MCFG_TIMER_DRIVER_ADD_PERIODIC("kb_timer" , hp64k_state , hp64k_kb_scan , attotime::from_hz(100))

				// Clock = 25 MHz / 9 * (112/114)
				MCFG_DEVICE_ADD("crtc" , I8275 , 2729045)
				MCFG_I8275_CHARACTER_WIDTH(9)
				MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(hp64k_state , crtc_display_pixels)
				MCFG_I8275_DRQ_CALLBACK(WRITELINE(hp64k_state , hp64k_crtc_drq_w))
				MCFG_I8275_VRTC_CALLBACK(WRITELINE(hp64k_state , hp64k_crtc_vrtc_w))

				MCFG_SCREEN_ADD("screen" , RASTER)
				MCFG_SCREEN_UPDATE_DEVICE("crtc" , i8275_device , screen_update)
				MCFG_SCREEN_REFRESH_RATE(60)
				MCFG_PALETTE_ADD_MONOCHROME_GREEN_HIGHLIGHT("palette")
MACHINE_CONFIG_END

ROM_START(hp64k)
				ROM_REGION(0x8000 , "cpu" , ROMREGION_16BIT | ROMREGION_BE | ROMREGION_INVERT)
				ROM_LOAD16_BYTE("64100_80022.bin" , 0x0000 , 0x1000 , CRC(38b2aae5) SHA1(bfd0f126bfaf3724dc501979ad2d46afc41913aa))
				ROM_LOAD16_BYTE("64100_80020.bin" , 0x0001 , 0x1000 , CRC(ac01b436) SHA1(be1e827ea1393a95abb02a52ab5cc35dc2cd96e4))
				ROM_LOAD16_BYTE("64100_80023.bin" , 0x2000 , 0x1000 , CRC(6b4bc2ce) SHA1(00e6c58ccae9640dc81cb3e92db90a8c69b02a93))
				ROM_LOAD16_BYTE("64100_80021.bin" , 0x2001 , 0x1000 , CRC(74f9d33c) SHA1(543a845a992b0ceac3e0491acdfb178df0adeb1f))
				ROM_LOAD16_BYTE("64100_80026.bin" , 0x4000 , 0x1000 , CRC(a74e834b) SHA1(a2ff9765628985d9bab4cb44ba23257a9b8d0965))
				ROM_LOAD16_BYTE("64100_80024.bin" , 0x4001 , 0x1000 , CRC(2e15a1d2) SHA1(ce4330f8f8015a26c02f0965b95baf7dfd615512))
				ROM_LOAD16_BYTE("64100_80027.bin" , 0x6000 , 0x1000 , CRC(b93c0e7a) SHA1(b239446d3d6e9d3dba6c0278b2771abe1623e1ad))
				ROM_LOAD16_BYTE("64100_80025.bin" , 0x6001 , 0x1000 , CRC(e6353085) SHA1(48d78835c798f2caf6ee539057676d4f3c8a4df9))

				ROM_REGION(0x800 , "chargen" , 0)
				ROM_LOAD("1816_1496_82S191.bin" , 0 , 0x800 , CRC(32a52664) SHA1(8b2a49a32510103ff424e8481d5ed9887f609f2f))
ROM_END

/*    YEAR  NAME       PARENT    COMPAT MACHINE INPUT     INIT              COMPANY       FULLNAME */
COMP( 1979, hp64k,     0,        0,     hp64k,  hp64k,    driver_device, 0, "HP",      "HP 64000" , GAME_NO_SOUND)
