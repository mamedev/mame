// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    HP Jornada PDA skeleton driver

    Notes:
    - GPIO1: IRQ from SA-1111

***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/sa1110.h"
#include "machine/sa1111.h"
#include "sound/uda1344.h"
#include "video/sed1356.h"
#include "screen.h"
#include "emupal.h"

#define LOG_MCU     (1 << 1)
#define LOG_ALL     (LOG_MCU)

#define VERBOSE     (LOG_ALL)
#include "logmacro.h"

#define SA1110_CLOCK 206000000

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
		, m_eeprom_data(*this, "eeprom")
		, m_epson(*this, "epson")
		, m_codec(*this, "codec")
		, m_kbd_port(*this, "KBD0")
	{ }

	void jornada720(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_changed);

	enum
	{
		KEY_ON_OFF,
		KEY_S,
		KEY_K,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_9
	};

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_reset_after_children() override;

	void main_map(address_map &map);

	void mcu_byte_received(uint16_t data);
	void eeprom_cmd_received(uint16_t data);

	enum mcu_state : int
	{
		MCU_IDLE,

		MCU_KBD_SEND_COUNT,
		MCU_KBD_SEND_CODES
	};

	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint32_t> m_ram;
	required_device<sa1110_periphs_device> m_sa_periphs;
	required_device<sa1111_device> m_companion;
	required_region_ptr<uint8_t> m_eeprom_data;
	required_device<sed1356_device> m_epson;
	required_device<uda1344_device> m_codec;

	required_ioport m_kbd_port;

	// MCU-related members
	int m_mcu_state;
	uint8_t m_mcu_key_send_idx;
	uint8_t m_mcu_key_codes[2][8];
	uint8_t m_mcu_key_count[2];

	// EEPROM-related members
	uint8_t m_eeprom_cmd;
	uint8_t m_eeprom_count;
	uint8_t m_eeprom_read_idx;
};

void jornada_state::main_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rom().region("firmware", 0);
	map(0x1a000000, 0x1a000fff).noprw(); // Debug Attachment Region
	map(0x40000000, 0x40001fff).m(m_companion, FUNC(sa1111_device::map));
	map(0x48000000, 0x481fffff).m(m_epson, FUNC(sed1356_device::map));
	map(0x48200000, 0x4827ffff).m(m_epson, FUNC(sed1356_device::vram_map));
	map(0x80050000, 0x80050023).rw(m_sa_periphs, FUNC(sa1110_periphs_device::uart3_r), FUNC(sa1110_periphs_device::uart3_w));
	map(0x80060000, 0x8006001b).rw(m_sa_periphs, FUNC(sa1110_periphs_device::mcp_r), FUNC(sa1110_periphs_device::mcp_w));
	map(0x80070000, 0x80070077).rw(m_sa_periphs, FUNC(sa1110_periphs_device::ssp_r), FUNC(sa1110_periphs_device::ssp_w));
	map(0x90000000, 0x9000001f).rw(m_sa_periphs, FUNC(sa1110_periphs_device::ostimer_r), FUNC(sa1110_periphs_device::ostimer_w));
	map(0x90010000, 0x9001001f).rw(m_sa_periphs, FUNC(sa1110_periphs_device::rtc_r), FUNC(sa1110_periphs_device::rtc_w));
	map(0x90020000, 0x9002001f).rw(m_sa_periphs, FUNC(sa1110_periphs_device::power_r), FUNC(sa1110_periphs_device::power_w));
	map(0x90030000, 0x90030007).rw(m_sa_periphs, FUNC(sa1110_periphs_device::reset_r), FUNC(sa1110_periphs_device::reset_w));
	map(0x90040000, 0x90040023).rw(m_sa_periphs, FUNC(sa1110_periphs_device::gpio_r), FUNC(sa1110_periphs_device::gpio_w));
	map(0x90050000, 0x90050023).rw(m_sa_periphs, FUNC(sa1110_periphs_device::intc_r), FUNC(sa1110_periphs_device::intc_w));
	map(0xc0000000, 0xc1ffffff).ram().share("ram");
	map(0xe0000000, 0xe0003fff).noprw(); // Cache-Flush Region 0
	map(0xe0100000, 0xe01003ff).noprw(); // Cache-Flush Region 1
}

void jornada_state::device_reset_after_children()
{
}

void jornada_state::mcu_byte_received(uint16_t data)
{
	const uint8_t raw_value = (uint8_t)(data >> 8);
	const uint8_t value = bitswap<8>(raw_value, 0, 1, 2, 3, 4, 5, 6, 7);

	uint8_t response = 0x11;
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
		default:
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_IDLE: Unknown (%02x), ignoring and sending TxDummy response\n", value);
			break;
		}
		break;

	case MCU_KBD_SEND_COUNT:
		if (value == 0x11)
		{
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_COUNT: TxDummy, entering KBD_SEND_CODES state\n");
			response = m_mcu_key_count[m_mcu_key_send_idx];
			m_mcu_state = MCU_KBD_SEND_CODES;
		}
		else
		{
			LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_COUNT: Unknown (%02x), sending ErrorCode response and returning to IDLE state\n");
			response = 0;
		}
		break;

	case MCU_KBD_SEND_CODES:
		if (value == 0x11)
		{
			m_mcu_key_count[m_mcu_key_send_idx]--;
			response = m_mcu_key_codes[m_mcu_key_send_idx][m_mcu_key_count[m_mcu_key_send_idx]];
			if (m_mcu_key_count[m_mcu_key_send_idx])
			{
				LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_CODES: TxDummy, sending scan code %02x with %d remaining\n", response, m_mcu_key_count[m_mcu_key_send_idx]);
			}
			else
			{
				LOGMASKED(LOG_MCU, "mcu_byte_received in MCU_KBD_SEND_CODES: TxDummy, sending scan code %02x and returning to IDLE state\n", response);
				m_mcu_state = MCU_IDLE;
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

void jornada_state::eeprom_cmd_received(uint16_t data)
{
	bool reset_state = true;
	uint8_t response = 0;

	if (m_eeprom_cmd == 0)
	{
		m_eeprom_cmd = (uint8_t)data;
		if (m_eeprom_cmd == 3)
			reset_state = false;
	}
	else if (m_eeprom_count == 0)
	{
		if (m_eeprom_cmd == 3)
		{
			m_eeprom_count = (uint8_t)data;
			m_eeprom_read_idx = 0;
			reset_state = false;
		}
	}
	else if (m_eeprom_read_idx < m_eeprom_count)
	{
		response = m_eeprom_data[m_eeprom_read_idx];
		m_eeprom_read_idx++;
		if (m_eeprom_read_idx < m_eeprom_count)
			reset_state = false;
	}

	m_companion->ssp_in((uint16_t)response);

	if (reset_state)
	{
		m_eeprom_cmd = 0;
		m_eeprom_count = 0;
		m_eeprom_read_idx = 0;
	}
}

INPUT_CHANGED_MEMBER(jornada_state::key_changed)
{
	uint8_t scan_code = 0;
	switch (param)
	{
	case KEY_ON_OFF:    scan_code = 0x7f;   break;
	case KEY_S:         scan_code = 0x32;   break;
	case KEY_K:         scan_code = 0x38;   break;
	case KEY_1:         scan_code = 0x11;   break;
	case KEY_2:         scan_code = 0x12;   break;
	case KEY_3:         scan_code = 0x13;   break;
	case KEY_4:         scan_code = 0x14;   break;
	case KEY_9:         scan_code = 0x19;   break;
	default: return;
	}

	m_sa_periphs->gpio_in<0>(1);
	m_sa_periphs->gpio_in<0>(0);

	const uint8_t key_recv_idx = 1 - m_mcu_key_send_idx;
	if (m_mcu_key_count[key_recv_idx] < 8)
	{
		m_mcu_key_codes[key_recv_idx][m_mcu_key_count[key_recv_idx]] = scan_code | (newval ? 0x00 : 0x80);
		m_mcu_key_count[key_recv_idx]++;
	}
}

static INPUT_PORTS_START( jornada720 )
	PORT_START("KBD0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("On/Off") PORT_CODE(KEYCODE_HOME) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_ON_OFF)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_K)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_3)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_4)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, jornada_state, key_changed, jornada_state::KEY_9)
INPUT_PORTS_END

void jornada_state::machine_start()
{
	save_item(NAME(m_mcu_state));
	save_item(NAME(m_mcu_key_send_idx));
	save_item(NAME(m_mcu_key_codes));
	save_item(NAME(m_mcu_key_count));

	save_item(NAME(m_eeprom_cmd));
	save_item(NAME(m_eeprom_count));
	save_item(NAME(m_eeprom_read_idx));
}

void jornada_state::machine_reset()
{
	m_mcu_state = MCU_IDLE;
	m_mcu_key_send_idx = 0;
	memset(m_mcu_key_codes[0], 0, 8);
	memset(m_mcu_key_codes[1], 0, 8);
	memset(m_mcu_key_count, 0, 2);

	m_eeprom_cmd = 0;
	m_eeprom_count = 0;
	m_eeprom_read_idx = 0;
}

void jornada_state::jornada720(machine_config &config)
{
	SA1110(config, m_maincpu, SA1110_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &jornada_state::main_map);

	SA1110_PERIPHERALS(config, m_sa_periphs, SA1110_CLOCK, m_maincpu);
	m_sa_periphs->ssp_out().set(FUNC(jornada_state::mcu_byte_received));

	SA1111(config, m_companion);
	m_companion->ssp_out().set(FUNC(jornada_state::eeprom_cmd_received));
	m_companion->l3_addr_out().set(m_codec, FUNC(uda1344_device::l3_addr_w));
	m_companion->l3_data_out().set(m_codec, FUNC(uda1344_device::l3_data_w));

	UDA1344(config, m_codec);
	m_codec->l3_ack_out().set(m_companion, FUNC(sa1111_device::l3wd_in));

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

	ROM_REGION( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "jorn720_eeprom.bin", 0x00, 0x80, CRC(54ffaaff) SHA1(5b8296782b6dc1c60b80169c071fb157d0681567) BAD_DUMP )
ROM_END

} // anonymous namespace

COMP( 2000, jorn720, 0, 0, jornada720, jornada720, jornada_state, empty_init, "Hewlett Packard", "Jornada 720", MACHINE_IS_SKELETON )
