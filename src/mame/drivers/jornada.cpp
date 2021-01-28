// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    HP Jornada PDA skeleton driver

    To boot:
    - Start MAME with the debugger enabled
    - Use the following breakpoint command: bp 13E2C,R3==3 && R1==10
    - Close the debugger and allow the machine to run
    - When the breakpoint is hit, use the following command: R3=0
    - Close the debugger, booting will proceed

***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/m950x0.h"
#include "machine/sa1110.h"
#include "machine/sa1111.h"
#include "sound/uda1344.h"
#include "video/sed1356.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"

#define LOG_MCU     (1 << 1)
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

	enum : uint8_t
	{
		MCU_TXDUMMY     = 0x11,
		MCU_TXDUMMY2    = 0x88
	};


	enum
	{
		KEY_ON_OFF = 0x7f,
		KEY_S = 0x32,
		KEY_K = 0x38,
		KEY_1 = 0x11,
		KEY_2 = 0x12,
		KEY_3 = 0x13,
		KEY_4 = 0x14,
		KEY_9 = 0x19,
		KEY_TAB = 0x51,
		KEY_ENTER = 0x4c,
		KEY_A = 0x31,
		KEY_N = 0x46,
		KEY_L = 0x39,
		KEY_M = 0x47,
		KEY_P = 0x2a,
		KEY_C = 0x43,
		KEY_B = 0x45,
		KEY_ALT = 0x65,
		KEY_SPACE = 0x74,
		KEY_BACKSPACE = 0x2c,
		KEY_LSHIFT = 0x53,
		KEY_RSHIFT = 0x5c
	};

	enum
	{
		PEN_X,
		PEN_Y,
		PEN_BUTTON
	};

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_reset_after_children() override;

	static constexpr uint32_t SA1110_CLOCK = 206000000;

	void main_map(address_map &map);

	void mcu_assemble_touch_data();
	void mcu_byte_received(uint16_t data);
	void eeprom_data_received(uint16_t data);
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
	required_shared_ptr<uint32_t> m_ram;
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
	int m_mcu_state;
	uint8_t m_mcu_key_send_idx;
	uint8_t m_mcu_key_codes[2][8];
	uint8_t m_mcu_key_count[2];
	uint8_t m_mcu_key_idx[2];
	uint8_t m_mcu_touch_send_idx;
	uint8_t m_mcu_touch_data[2][8];
	uint8_t m_mcu_touch_count[2];
	uint8_t m_mcu_touch_idx[2];
	uint8_t m_mcu_battery_data[3];
	uint8_t m_mcu_battery_idx;
};

void jornada_state::main_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rom().region("firmware", 0);
	map(0x1a000000, 0x1a000fff).noprw(); // Debug Attachment Region
	map(0x40000000, 0x40001fff).m(m_companion, FUNC(sa1111_device::map));
	map(0x48000000, 0x481fffff).m(m_epson, FUNC(sed1356_device::map));
	map(0x48200000, 0x4827ffff).m(m_epson, FUNC(sed1356_device::vram_map));
	map(0x80000000, 0x80000033).rw(m_sa_periphs, FUNC(sa1110_periphs_device::udc_r), FUNC(sa1110_periphs_device::udc_w));
	map(0x80030000, 0x8003007b).rw(m_sa_periphs, FUNC(sa1110_periphs_device::icp_r), FUNC(sa1110_periphs_device::icp_w));
	map(0x80050000, 0x80050023).rw(m_sa_periphs, FUNC(sa1110_periphs_device::uart3_r), FUNC(sa1110_periphs_device::uart3_w));
	map(0x80060000, 0x8006001b).rw(m_sa_periphs, FUNC(sa1110_periphs_device::mcp_r), FUNC(sa1110_periphs_device::mcp_w));
	map(0x80070000, 0x80070077).rw(m_sa_periphs, FUNC(sa1110_periphs_device::ssp_r), FUNC(sa1110_periphs_device::ssp_w));
	map(0x90000000, 0x9000001f).rw(m_sa_periphs, FUNC(sa1110_periphs_device::ostimer_r), FUNC(sa1110_periphs_device::ostimer_w));
	map(0x90010000, 0x9001001f).rw(m_sa_periphs, FUNC(sa1110_periphs_device::rtc_r), FUNC(sa1110_periphs_device::rtc_w));
	map(0x90020000, 0x9002001f).rw(m_sa_periphs, FUNC(sa1110_periphs_device::power_r), FUNC(sa1110_periphs_device::power_w));
	map(0x90030000, 0x90030007).rw(m_sa_periphs, FUNC(sa1110_periphs_device::reset_r), FUNC(sa1110_periphs_device::reset_w));
	map(0x90040000, 0x90040023).rw(m_sa_periphs, FUNC(sa1110_periphs_device::gpio_r), FUNC(sa1110_periphs_device::gpio_w));
	map(0x90050000, 0x90050023).rw(m_sa_periphs, FUNC(sa1110_periphs_device::intc_r), FUNC(sa1110_periphs_device::intc_w));
	map(0x90060000, 0x90060013).rw(m_sa_periphs, FUNC(sa1110_periphs_device::ppc_r), FUNC(sa1110_periphs_device::ppc_w));
	map(0xb0000000, 0xb00000bf).rw(m_sa_periphs, FUNC(sa1110_periphs_device::dma_r), FUNC(sa1110_periphs_device::dma_w));
	map(0xc0000000, 0xc1ffffff).ram().share("ram");
	map(0xe0000000, 0xe0003fff).noprw(); // Cache-Flush Region 0
	map(0xe0100000, 0xe01003ff).noprw(); // Cache-Flush Region 1
}

void jornada_state::device_reset_after_children()
{
	m_sa_periphs->gpio_in<4>(0); // Flag as plugged into AC power
	m_sa_periphs->gpio_in<9>(1); // Pen input is active-high
	m_sa_periphs->gpio_in<26>(0); // Flag as charging
}

void jornada_state::mcu_assemble_touch_data()
{
	const uint16_t pen_x = m_pen_x->read();
	const uint16_t pen_y = m_pen_y->read();
	const uint8_t touch_recv_idx = 1 - m_mcu_touch_send_idx;
	m_mcu_touch_data[touch_recv_idx][0] = (uint8_t)pen_x;
	m_mcu_touch_data[touch_recv_idx][1] = (uint8_t)pen_x;
	m_mcu_touch_data[touch_recv_idx][2] = (uint8_t)pen_x;
	m_mcu_touch_data[touch_recv_idx][3] = (uint8_t)pen_y;
	m_mcu_touch_data[touch_recv_idx][4] = (uint8_t)pen_y;
	m_mcu_touch_data[touch_recv_idx][5] = (uint8_t)pen_y;
	m_mcu_touch_data[touch_recv_idx][6] = (uint8_t)((pen_x >> 8) * 0x15);
	m_mcu_touch_data[touch_recv_idx][7] = (uint8_t)((pen_y >> 8) * 0x15);
	m_mcu_touch_count[touch_recv_idx] = 8;
}

void jornada_state::mcu_byte_received(uint16_t data)
{
	const uint8_t raw_value = (uint8_t)(data >> 8);
	const uint8_t value = bitswap<8>(raw_value, 0, 1, 2, 3, 4, 5, 6, 7);

	uint8_t response = MCU_TXDUMMY;
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
				//machine().debug_break();
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
	}

	response = bitswap<8>(response, 0, 1, 2, 3, 4, 5, 6, 7);
	m_sa_periphs->ssp_in((uint16_t)response);
}

void jornada_state::eeprom_select(int state)
{
	m_nvram->select_w(!state);
}

void jornada_state::eeprom_data_received(uint16_t data)
{
	const uint8_t response = m_nvram->access((uint8_t)data);
	m_companion->ssp_in((uint16_t)response);
}

INPUT_CHANGED_MEMBER(jornada_state::key_changed)
{
	uint8_t scan_code = (uint8_t)param;

	m_sa_periphs->gpio_in<0>(1);
	m_sa_periphs->gpio_in<0>(0);

	const uint8_t key_recv_idx = 1 - m_mcu_key_send_idx;
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
			mcu_assemble_touch_data();
			m_sa_periphs->gpio_in<9>(1);
			m_sa_periphs->gpio_in<9>(0);
		}
		break;
	case PEN_BUTTON:
		if (newval)
		{
			m_sa_periphs->gpio_in<9>(0);
			mcu_assemble_touch_data();
		}
		else
		{
			m_sa_periphs->gpio_in<9>(1);
		}
		break;
	}
}

static INPUT_PORTS_START( jornada720 )
	PORT_START("KBD0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("On/Off") PORT_CODE(KEYCODE_HOME) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_ON_OFF)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_K)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_3)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_4)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_9)

	PORT_START("KBD1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_TAB)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_ENTER)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_N)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_M)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_P)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_C)

	PORT_START("KBD2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_B)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_ALT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_SPACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_BACKSPACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_LSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_RSHIFT)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PENX")
	PORT_BIT(0x3ff, 590, IPT_LIGHTGUN_X) PORT_NAME("Pen X") PORT_MINMAX(270, 910) PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, pen_changed, jornada_state::PEN_X)

	PORT_START("PENY")
	PORT_BIT(0x3ff, 500, IPT_LIGHTGUN_Y) PORT_NAME("Pen Y") PORT_MINMAX(180, 820) PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, pen_changed, jornada_state::PEN_Y)

	PORT_START("PENZ")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pen Touch") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, pen_changed, jornada_state::PEN_BUTTON)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void jornada_state::machine_start()
{
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
}

void jornada_state::machine_reset()
{
	m_mcu_state = MCU_IDLE;

	m_mcu_key_send_idx = 0;
	memset(m_mcu_key_codes[0], 0, 8);
	memset(m_mcu_key_codes[1], 0, 8);
	memset(m_mcu_key_count, 0, 2);
	memset(m_mcu_key_idx, 0, 2);

	m_mcu_touch_send_idx = 0;
	memset(m_mcu_touch_data[0], 0, 8);
	memset(m_mcu_touch_data[1], 0, 8);
	memset(m_mcu_touch_count, 0, 2);
	memset(m_mcu_touch_idx, 0, 2);

	memset(m_mcu_battery_data, 0, 3);
	m_mcu_battery_idx = 0;
}

void jornada_state::jornada720(machine_config &config)
{
	SA1110(config, m_maincpu, SA1110_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &jornada_state::main_map);

	SA1110_PERIPHERALS(config, m_sa_periphs, SA1110_CLOCK, m_maincpu);
	m_sa_periphs->ssp_out().set(FUNC(jornada_state::mcu_byte_received));

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
