// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    HP Jornada PDA skeleton driver

***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/m950x0.h"
#include "machine/sa1110.h"
#include "machine/sa1111.h"
#include "sound/uda1344.h"
#include "video/sed1356.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"

#define LOG_MCU     (1U << 1)
#define LOG_ALL     (LOG_MCU)

#define VERBOSE     (0)
#include "logmacro.h"

namespace
{

class jornada_state : public driver_device
{
public:
	jornada_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_sa_periphs(*this, "onboard")
		, m_companion(*this, "companion")
		, m_epson(*this, "epson")
		, m_codec(*this, "codec")
		, m_nvram(*this, "nvram")
		, m_kbd_ports(*this, "KBD%u", 0U)
		, m_pen_x(*this, "PENX")
		, m_pen_y(*this, "PENY")
		, m_pen_button(*this, "PENZ")
	{ }

	void jornada720(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_changed);
	DECLARE_INPUT_CHANGED_MEMBER(pen_changed);

	enum : u8
	{
		MCU_TXDUMMY     = 0x11,
		MCU_TXDUMMY2    = 0x88
	};


	enum
	{
		KEY_Q = 0x21,
		KEY_W = 0x22,
		KEY_E = 0x23,
		KEY_R = 0x24,
		KEY_T = 0x25,
		KEY_Y = 0x26,
		KEY_U = 0x27,
		KEY_I = 0x28,
		KEY_O = 0x29,
		KEY_P = 0x2a,
		KEY_A = 0x31,
		KEY_S = 0x32,
		KEY_D = 0x33,
		KEY_F = 0x34,
		KEY_G = 0x35,
		KEY_H = 0x36,
		KEY_J = 0x37,
		KEY_K = 0x38,
		KEY_L = 0x39,
		KEY_Z = 0x41,
		KEY_X = 0x42,
		KEY_C = 0x43,
		KEY_V = 0x44,
		KEY_B = 0x45,
		KEY_N = 0x46,
		KEY_M = 0x47,

		KEY_0 = 0x1a,
		KEY_1 = 0x11,
		KEY_2 = 0x12,
		KEY_3 = 0x13,
		KEY_4 = 0x14,
		KEY_5 = 0x15,
		KEY_6 = 0x16,
		KEY_7 = 0x17,
		KEY_8 = 0x18,
		KEY_9 = 0x19,

		KEY_QL1 = 0x02,
		KEY_QL2 = 0x03,
		KEY_QL3 = 0x04,
		KEY_QL4 = 0x05,
		KEY_QL5 = 0x06,
		KEY_QL6 = 0x07,
		KEY_QL7 = 0x08,
		KEY_QL8 = 0x09,
		KEY_QL9 = 0x0a,
		KEY_QL10 = 0x0b,
		KEY_QL11 = 0x0c,

		KEY_SLASH = 0x78,
		KEY_BACKSLASH = 0x2b,
		KEY_MINUS = 0x1b,
		KEY_EQUALS = 0x1c,
		KEY_COMMA = 0x48,
		KEY_PERIOD = 0x49,
		KEY_QUOTE = 0x4b,
		KEY_COLON = 0x3a,

		KEY_ON_OFF = 0x7f,
		KEY_WIN = 0x71,
		KEY_FN = 0x66,
		KEY_BACKSPACE = 0x2c,
		KEY_CTRL = 0x72,
		KEY_ALT = 0x65,
		KEY_LSHIFT = 0x53,
		KEY_RSHIFT = 0x5c,
		KEY_DEL = 0x79,
		KEY_SPACE = 0x74,
		KEY_TAB = 0x51,
		KEY_ESC = 0x01,
		KEY_VOL_UP = 0x0e,
		KEY_VOL_DOWN = 0x0d,
		KEY_PLAY = 0x0f,
		KEY_UP = 0x5a,
		KEY_DOWN = 0x6a,
		KEY_LEFT = 0x69,
		KEY_RIGHT = 0x6b,
		KEY_ENTER = 0x4c
	};

	enum
	{
		PEN_X,
		PEN_Y,
		PEN_BUTTON
	};

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	static constexpr u32 SA1110_CLOCK = 206000000;

	void main_map(address_map &map) ATTR_COLD;

	void cpu_rts_to_mcu(int state);
	void mcu_assemble_touch_data();
	void mcu_process_byte(u8 value);
	void mcu_byte_received(u16 data);
	void eeprom_data_received(u16 data);
	void eeprom_select(int state);

	enum mcu_state : int
	{
		MCU_IDLE,

		MCU_KBD_SEND_COUNT,
		MCU_KBD_SEND_CODES,

		MCU_TOUCH_SEND_DATA,

		MCU_BATTERY_SEND_DATA
	};

	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u32> m_ram;
	required_device<sa1110_periphs_device> m_sa_periphs;
	required_device<sa1111_device> m_companion;
	required_device<sed1356_device> m_epson;
	required_device<uda1344_device> m_codec;
	required_device<m95020_device> m_nvram;

	required_ioport_array<3> m_kbd_ports;
	required_ioport m_pen_x;
	required_ioport m_pen_y;
	required_ioport m_pen_button;

	// MCU-related members
	bool m_cpu_to_mcu_rts;
	int m_mcu_state;
	u8 m_mcu_key_send_idx;
	u8 m_mcu_key_codes[2][8];
	u8 m_mcu_key_count[2];
	u8 m_mcu_key_idx[2];
	u8 m_mcu_touch_send_idx;
	u8 m_mcu_touch_data[2][8];
	u8 m_mcu_touch_count[2];
	u8 m_mcu_touch_idx[2];
	u8 m_mcu_battery_data[3];
	u8 m_mcu_battery_idx;
	u8 m_mcu_rx_fifo[8];
	u8 m_mcu_rx_count;
};

void jornada_state::main_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rom().region("firmware", 0);
	//map(0x1a000000, 0x1a000fff).noprw(); // Debug Attachment Region
	map(0x1a00013c, 0x1a00013f).noprw();
	map(0x1a000400, 0x1a000403).noprw();
	map(0x40000000, 0x40001fff).m(m_companion, FUNC(sa1111_device::map));
	map(0x48000000, 0x481fffff).m(m_epson, FUNC(sed1356_device::map));
	map(0x48200000, 0x4827ffff).m(m_epson, FUNC(sed1356_device::vram_map));
	map(0x80000000, 0xbfffffff).m(m_sa_periphs, FUNC(sa1110_periphs_device::map));
	map(0xc0000000, 0xc1ffffff).ram().share("ram");
	map(0xe0000000, 0xe0003fff).noprw(); // Cache-Flush Region 0
	map(0xe0100000, 0xe01003ff).noprw(); // Cache-Flush Region 1
}

void jornada_state::device_reset_after_children()
{
	driver_device::device_reset_after_children();

	m_sa_periphs->gpio_in<4>(0); // Flag as plugged into AC power
	m_sa_periphs->gpio_in<9>(1); // Pen input is active-low
	m_sa_periphs->gpio_in<26>(0); // Flag as charging
}

void jornada_state::cpu_rts_to_mcu(int state)
{
	const bool old = m_cpu_to_mcu_rts;
	m_cpu_to_mcu_rts = (bool)state;
	if (!old || m_cpu_to_mcu_rts || m_mcu_rx_count == 0)
		return;

	for (u8 i = 0; i < m_mcu_rx_count; i++)
	{
		mcu_process_byte(m_mcu_rx_fifo[i]);
	}
	m_mcu_rx_count = 0;
}

void jornada_state::mcu_assemble_touch_data()
{
	const u16 pen_x = m_pen_x->read();
	const u16 pen_y = m_pen_y->read();
	const u16 x0 = pen_x;
	const u16 x1 = (pen_x + 1) & 0x3ff;
	const u16 x2 = (pen_x - 1) & 0x3ff;
	const u16 y0 = pen_y;
	const u16 y1 = (pen_y + 1) & 0x3ff;
	const u16 y2 = (pen_y - 1) & 0x3ff;
	const u8 touch_recv_idx = 1 - m_mcu_touch_send_idx;
	m_mcu_touch_data[touch_recv_idx][0] = (u8)x0;
	m_mcu_touch_data[touch_recv_idx][1] = (u8)x1;
	m_mcu_touch_data[touch_recv_idx][2] = (u8)x2;
	m_mcu_touch_data[touch_recv_idx][3] = (u8)y0;
	m_mcu_touch_data[touch_recv_idx][4] = (u8)y1;
	m_mcu_touch_data[touch_recv_idx][5] = (u8)y2;
	m_mcu_touch_data[touch_recv_idx][6] = (u8)(((x0 >> 8) & 0x03) | ((x1 >> 6) & 0xc0) | ((x2 >> 4) & 0x30));
	m_mcu_touch_data[touch_recv_idx][7] = (u8)(((y0 >> 8) & 0x03) | ((y1 >> 6) & 0xc0) | ((y2 >> 4) & 0x30));
	m_mcu_touch_count[touch_recv_idx] = 8;
}

void jornada_state::mcu_process_byte(u8 value)
{
	u8 response = MCU_TXDUMMY;
	switch (m_mcu_state)
	{
	case MCU_IDLE:
		switch (value)
		{
		case 0x90:
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_IDLE: GetScanKeyCode, entering KBD_SEND_COUNT state\n");
			m_mcu_state = MCU_KBD_SEND_COUNT;
			m_mcu_key_send_idx = 1 - m_mcu_key_send_idx;
			break;
		case 0xa0:
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_IDLE: GetTouchSamples, entering TOUCH_SEND_DATA state\n");
			m_mcu_state = MCU_TOUCH_SEND_DATA;
			m_mcu_touch_send_idx = 1 - m_mcu_touch_send_idx;
			break;
		case 0xc0:
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_IDLE: GetBatteryData, entering BATTERY_SEND_DATA state\n");
			m_mcu_state = MCU_BATTERY_SEND_DATA;
			m_mcu_battery_idx = 0;
			m_mcu_battery_data[0] = 0xff; // LSB of main battery level
			m_mcu_battery_data[1] = 0xff; // LSB of backup battery level
			m_mcu_battery_data[2] = 0x0f; // MSBs of both battery levels (backup in 3:2, main in 1:0)
			break;
		default:
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_IDLE: Unknown (%02x), ignoring and sending TxDummy response\n", value);
			break;
		}
		break;

	case MCU_KBD_SEND_COUNT:
		if (value == MCU_TXDUMMY || value == MCU_TXDUMMY2)
		{
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_COUNT: TxDummy, entering KBD_SEND_CODES state\n");
			response = m_mcu_key_count[m_mcu_key_send_idx];
			m_mcu_state = MCU_KBD_SEND_CODES;
		}
		else
		{
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_COUNT: Unknown (%02x), sending ErrorCode response and returning to IDLE state\n", value);
			response = 0;
		}
		break;

	case MCU_BATTERY_SEND_DATA:
		if (value == MCU_TXDUMMY || value == MCU_TXDUMMY2)
		{
			response = m_mcu_battery_data[m_mcu_battery_idx];
			m_mcu_battery_idx++;
			if (m_mcu_battery_idx < 3)
			{
				LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_BATTERY_SEND_DATA: TxDummy, sending battery data %02x with %d remaining\n", response, 3 - m_mcu_battery_idx);
			}
			else
			{
				LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_BATTERY_SEND_DATA: TxDummy, sending battery data %02x and returning to IDLE state\n", response);
				m_mcu_state = MCU_IDLE;
				m_mcu_battery_idx = 0;
			}
		}
		else
		{
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_BATTERY_SEND_DATA: Unknown (%02x), sending ErrorCode response and returning to IDLE state\n", value);
			response = 0;
		}
		break;

	case MCU_TOUCH_SEND_DATA:
		if (value == MCU_TXDUMMY || value == MCU_TXDUMMY2)
		{
			m_mcu_touch_count[m_mcu_touch_send_idx]--;
			response = m_mcu_touch_data[m_mcu_touch_send_idx][m_mcu_touch_idx[m_mcu_touch_send_idx]];
			m_mcu_touch_idx[m_mcu_touch_send_idx]++;
			if (m_mcu_touch_count[m_mcu_touch_send_idx])
			{
				LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_TOUCH_SEND_DATA: TxDummy, sending touch data %02x with %d remaining\n", response, m_mcu_touch_count[m_mcu_touch_send_idx]);
			}
			else
			{
				LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_TOUCH_SEND_DATA: TxDummy, sending touch data %02x and returning to IDLE state\n", response);
				m_mcu_state = MCU_IDLE;
				m_mcu_touch_idx[m_mcu_touch_send_idx] = 0;
			}
		}
		else
		{
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_TOUCH_SEND_DATA: Unknown (%02x), sending ErrorCode response and returning to IDLE state\n", value);
			response = 0;
		}
		break;

	case MCU_KBD_SEND_CODES:
		if (value == MCU_TXDUMMY || value == MCU_TXDUMMY2)
		{
			m_mcu_key_count[m_mcu_key_send_idx]--;
			response = m_mcu_key_codes[m_mcu_key_send_idx][m_mcu_key_idx[m_mcu_key_send_idx]];
			m_mcu_key_idx[m_mcu_key_send_idx]++;
			if (m_mcu_key_count[m_mcu_key_send_idx])
			{
				LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_CODES: TxDummy, sending scan code %02x with %d remaining\n", response, m_mcu_key_count[m_mcu_key_send_idx]);
			}
			else
			{
				LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_CODES: TxDummy, sending scan code %02x and returning to IDLE state\n", response);
				m_mcu_state = MCU_IDLE;
				m_mcu_key_idx[m_mcu_key_send_idx] = 0;
			}
		}
		else
		{
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_CODES: Unknown (%02x), sending ErrorCode response and returning to IDLE state\n");
			response = 0;
		}
		break;

	default:
		LOGMASKED(LOG_MCU, "mcu_byte_received in %08x: %02x\n", m_mcu_state, value);
		break;
	}

	response = bitswap<8>(response, 0, 1, 2, 3, 4, 5, 6, 7);
	m_sa_periphs->ssp_in((u16)response);
}

void jornada_state::mcu_byte_received(u16 data)
{
	const u8 raw_value = (u8)(data >> 8);
	const u8 value = bitswap<8>(raw_value, 0, 1, 2, 3, 4, 5, 6, 7);

	if (m_mcu_rx_count == 0 && !m_cpu_to_mcu_rts)
	{
		mcu_process_byte(value);
		return;
	}

	m_mcu_rx_fifo[m_mcu_rx_count++] = value;
}

void jornada_state::eeprom_select(int state)
{
	m_nvram->select_w(!state);
}

void jornada_state::eeprom_data_received(u16 data)
{
	const u8 response = m_nvram->access((u8)data);
	m_companion->ssp_in((u16)response);
}

INPUT_CHANGED_MEMBER(jornada_state::key_changed)
{
	u8 scan_code = (u8)param;

	m_sa_periphs->gpio_in<0>(1);
	m_sa_periphs->gpio_in<0>(0);

	const u8 key_recv_idx = 1 - m_mcu_key_send_idx;
	if (m_mcu_key_count[key_recv_idx] < 8)
	{
		m_mcu_key_codes[key_recv_idx][m_mcu_key_count[key_recv_idx]] = scan_code | (newval ? 0x00 : 0x80);
		m_mcu_key_count[key_recv_idx]++;
	}
}

INPUT_CHANGED_MEMBER(jornada_state::pen_changed)
{
	switch (param)
	{
	case PEN_X:
	case PEN_Y:
		if (m_pen_button->read() && m_mcu_state == MCU_IDLE)
		{
			logerror("Pen move, queueing data\n");
			mcu_assemble_touch_data();
			m_sa_periphs->gpio_in<9>(1);
			m_sa_periphs->gpio_in<9>(0);
		}
		break;
	case PEN_BUTTON:
		if (newval)
		{
			logerror("PEN_BUTTON, newval set (assembling touch data)\n");
			m_sa_periphs->gpio_in<9>(0);
			mcu_assemble_touch_data();
		}
		else
		{
			logerror("PEN_BUTTON, newval not set\n");
			m_sa_periphs->gpio_in<9>(1);
		}
		break;
	}
}

static INPUT_PORTS_START( jornada720 )
	PORT_START("KBD0")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_Q)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_W)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_E)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_R)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_T)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_Y)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_U)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_I)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_O)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_P)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_A)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_S)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_D)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_F)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_G)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_H)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_J)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_K)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_L)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_Z)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_X)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_C)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_V)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_B)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_N)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_M)
	PORT_BIT(0xfc000000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KBD1")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_0)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_1)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_2)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_3)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_4)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_5)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_6)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_7)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_8)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)          PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_9)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_SLASH)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_BACKSLASH)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_MINUS)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)     PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_EQUALS)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_COMMA)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)       PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_PERIOD)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\'") PORT_CODE(KEYCODE_QUOTE)     PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QUOTE)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_COLON)
	PORT_BIT(0xfffc0000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KBD2")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Power")          PORT_CODE(KEYCODE_END)       PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_ON_OFF)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Windows Key")    PORT_CODE(KEYCODE_LALT)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_WIN)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Fn")             PORT_CODE(KEYCODE_RCONTROL)  PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_FN)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace")      PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_BACKSPACE)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")        PORT_CODE(KEYCODE_LCONTROL)  PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_CTRL)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt")            PORT_CODE(KEYCODE_RALT)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_ALT)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")     PORT_CODE(KEYCODE_LSHIFT)    PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_LSHIFT)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")    PORT_CODE(KEYCODE_RSHIFT)    PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_RSHIFT)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")         PORT_CODE(KEYCODE_DEL)       PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_DEL)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space")          PORT_CODE(KEYCODE_SPACE)     PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_SPACE)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")            PORT_CODE(KEYCODE_TAB)       PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_TAB)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape")         PORT_CODE(KEYCODE_ESC)       PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_ESC)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Volume Up")      PORT_CODE(KEYCODE_PGUP)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_VOL_UP)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Volume Down")    PORT_CODE(KEYCODE_PGDN)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_VOL_DOWN)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Play")           PORT_CODE(KEYCODE_END)       PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_PLAY)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up")             PORT_CODE(KEYCODE_UP)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_UP)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down")           PORT_CODE(KEYCODE_DOWN)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_DOWN)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left")           PORT_CODE(KEYCODE_LEFT)      PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_LEFT)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right")          PORT_CODE(KEYCODE_RIGHT)     PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_RIGHT)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 1")  PORT_CODE(KEYCODE_F1)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL1)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 2")  PORT_CODE(KEYCODE_F2)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL2)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 3")  PORT_CODE(KEYCODE_F3)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL3)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 4")  PORT_CODE(KEYCODE_F4)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL4)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 5")  PORT_CODE(KEYCODE_F5)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL5)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 6")  PORT_CODE(KEYCODE_F6)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL6)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 7")  PORT_CODE(KEYCODE_F7)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL7)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 8")  PORT_CODE(KEYCODE_F8)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL8)
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 9")  PORT_CODE(KEYCODE_F9)        PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL9)
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 10") PORT_CODE(KEYCODE_F10)       PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL10)
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Quicklaunch 11") PORT_CODE(KEYCODE_F10)       PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_QL11)
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter")          PORT_CODE(KEYCODE_ENTER)     PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_ENTER)
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PENX")
	PORT_BIT(0x3ff, 0x1ff, IPT_LIGHTGUN_X) PORT_NAME("Pen X") PORT_MINMAX(0, 1023) PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, pen_changed, jornada_state::PEN_X)

	PORT_START("PENY")
	PORT_BIT(0x3ff, 0x1ff, IPT_LIGHTGUN_Y) PORT_NAME("Pen Y") PORT_MINMAX(0, 1023) PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, pen_changed, jornada_state::PEN_Y)

	PORT_START("PENZ")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pen Touch") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, pen_changed, jornada_state::PEN_BUTTON)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void jornada_state::machine_start()
{
	save_item(NAME(m_cpu_to_mcu_rts));
	save_item(NAME(m_mcu_state));
	save_item(NAME(m_mcu_key_send_idx));
	save_item(NAME(m_mcu_key_codes));
	save_item(NAME(m_mcu_key_count));
	save_item(NAME(m_mcu_key_idx));
	save_item(NAME(m_mcu_touch_send_idx));
	save_item(NAME(m_mcu_touch_data));
	save_item(NAME(m_mcu_touch_count));
	save_item(NAME(m_mcu_touch_idx));
	save_item(NAME(m_mcu_battery_data));
	save_item(NAME(m_mcu_battery_idx));
	save_item(NAME(m_mcu_rx_fifo));
	save_item(NAME(m_mcu_rx_count));
}

void jornada_state::machine_reset()
{
	m_mcu_state = MCU_IDLE;
	m_cpu_to_mcu_rts = false;

	m_mcu_key_send_idx = 0;
	memset(m_mcu_key_codes[0], 0, sizeof(m_mcu_key_codes[0]));
	memset(m_mcu_key_codes[1], 0, sizeof(m_mcu_key_codes[1]));
	memset(m_mcu_key_count, 0, sizeof(m_mcu_key_count));
	memset(m_mcu_key_idx, 0, sizeof(m_mcu_key_idx));

	m_mcu_touch_send_idx = 0;
	memset(m_mcu_touch_data[0], 0, sizeof(m_mcu_touch_data[0]));
	memset(m_mcu_touch_data[1], 0, sizeof(m_mcu_touch_data[1]));
	memset(m_mcu_touch_count, 0, sizeof(m_mcu_touch_count));
	memset(m_mcu_touch_idx, 0, sizeof(m_mcu_touch_idx));

	memset(m_mcu_battery_data, 0, sizeof(m_mcu_battery_data));
	m_mcu_battery_idx = 0;

	memset(m_mcu_rx_fifo, 0, sizeof(m_mcu_rx_fifo));
	m_mcu_rx_count = 0;

	LOGMASKED(LOG_MCU, "MCU State: %08x\n", m_mcu_state);
}

void jornada_state::jornada720(machine_config &config)
{
	SA1110(config, m_maincpu, SA1110_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &jornada_state::main_map);

	SA1110_PERIPHERALS(config, m_sa_periphs, SA1110_CLOCK, m_maincpu);
	m_sa_periphs->ssp_out().set(FUNC(jornada_state::mcu_byte_received));
	m_sa_periphs->gpio_out<10>().set(FUNC(jornada_state::cpu_rts_to_mcu));

	SA1111(config, m_companion, 3.6864_MHz_XTAL, m_maincpu);
	m_companion->set_audio_codec_tag(m_codec);
	m_companion->pb_out<0>().set(FUNC(jornada_state::eeprom_select));
	m_companion->ssp_out().set(FUNC(jornada_state::eeprom_data_received));
	m_companion->l3_addr_out().set(m_codec, FUNC(uda1344_device::l3_addr_w));
	m_companion->l3_data_out().set(m_codec, FUNC(uda1344_device::l3_data_w));
	m_companion->i2s_out().set(m_codec, FUNC(uda1344_device::i2s_input_w));
	m_companion->irq_out().set(m_sa_periphs, FUNC(sa1110_periphs_device::gpio_in<1>));

	M95020(config, m_nvram);

	UDA1344(config, m_codec);
	m_codec->l3_ack_out().set(m_companion, FUNC(sa1111_device::l3wd_in));
	m_codec->add_route(0, "lspeaker", 0.5);
	m_codec->add_route(1, "rspeaker", 0.5);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SED1356(config, m_epson);
	m_epson->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(640, 240);
	screen.set_visarea(0, 640-1, 0, 240-1);
	screen.set_screen_update(m_epson, FUNC(sed1356_device::screen_update));
}

/***************************************************************************

  System driver(s)

***************************************************************************/

ROM_START( jorn720 )
	ROM_REGION32_LE( 0x2000000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "jornada720.bin", 0x0000000, 0x2000000, CRC(5fcd433a) SHA1(f05f7b377b582a7355bf119d74435f0ee6104cca) )

	ROM_REGION( 0x100, "nvram", ROMREGION_ERASE00 )
	ROM_LOAD( "jorn720_eeprom.bin", 0x000, 0x100, CRC(9bc1d53a) SHA1(793d6ff355e2e9b3e75574ff80edfa5af2aaeee6) BAD_DUMP )
ROM_END

} // anonymous namespace

COMP( 2000, jorn720, 0, 0, jornada720, jornada720, jornada_state, empty_init, "Hewlett Packard", "Jornada 720", MACHINE_IS_SKELETON )
