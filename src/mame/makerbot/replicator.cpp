// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    Replicator 1 desktop 3d printer

    driver by Felipe Correa da Silva Sanches <fsanches@metamaquina.com.br>

    Changelog:

    2013 DEC 28 [Felipe Sanches]:
        * LCD now works. We can see the firmware boot screen :-)

    2013 DEC 24 [Felipe Sanches]:
        * declaration of internal EEPROM

    2013 DEC 18 [Felipe Sanches]:
        * Initial driver skeleton
*/

// TODO:
// * figure out what's wrong with the keypad inputs (interface seems to be blocked in the first screen)
// * fix avr8 timer/counter #0 (toggle OC0B) and #5 (overflow interrupt "Microsecond timer") so that we get the buzzer to work
// * figure-out correct size of internal EEPROM
// * emulate an SD Card
// * implement avr8 WDR (watchdog reset) opcode

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_PORT_A      (1U << 1)
#define LOG_PORT_B      (1U << 2)
#define LOG_PORT_C      (1U << 3)
#define LOG_PORT_D      (1U << 4)
#define LOG_PORT_E      (1U << 5)
#define LOG_PORT_F      (1U << 6)
#define LOG_PORT_G      (1U << 7)
#define LOG_PORT_H      (1U << 8)
#define LOG_PORT_J      (1U << 9)
#define LOG_PORT_K      (1U << 10)
#define LOG_PORT_L      (1U << 11)
#define LOG_LCD_CLK     (1U << 12)
#define LOG_LCD_SHIFT   (1U << 13)

#define VERBOSE         (0)
#include "logmacro.h"


namespace {

#define MASTER_CLOCK    16000000

//Port A bits:
//Bit 0 unused
//Bit 1 unused
#define A_AXIS_DIR (1 << 2)
#define A_AXIS_STEP (1 << 3)
#define A_AXIS_EN (1 << 4)
#define A_AXIS_POT (1 << 5)
#define B_AXIS_DIR (1 << 6)
#define B_AXIS_STEP (1 << 7)

//Port B bits:
#define SD_CS (1 << 0)
#define SCK_1280 (1 << 1)
#define MOSI_1280 (1 << 2)
#define MISO_1280 (1 << 3)
#define EX2_PWR_CHECK (1 << 4)
#define EX2_HEAT (1 << 5)
#define EX2_FAN (1 << 6)
#define BLINK (1 << 7)

//Port C bits:
#define EX2_1280 (1 << 0)
#define EX1_1280 (1 << 1)
#define LCD_CLK (1 << 2)
#define LCD_DATA (1 << 3)
#define LCD_STROBE (1 << 4)
#define RLED (1 << 5)
#define GLED (1 << 6)
#define DETECT (1 << 7)

//Port D bits:
#define PORTD_SCL (1 << 0)
#define PORTD_SDA (1 << 1)
#define EX_RX_1280 (1 << 2)
#define EX_TX_1280 (1 << 3)
//Bit 4 unused
//Bit 5 unused
//Bit 6 unused
//Bit 7 unused

//Port E bits:
#define RX_1280 (1 << 0)
#define TX_1280 (1 << 1)
#define THERMO_SCK (1 << 2)
#define THERMO_CS1 (1 << 3)
#define THERMO_CS2 (1 << 4)
#define THERMO_DO (1 << 5)
//Bit 6 unused
//Bit 7 unused

//Port F bits:
#define X_AXIS_DIR (1 << 0)
#define X_AXIS_STEP (1 << 1)
#define X_AXIS_EN (1 << 2)
#define X_AXIS_POT (1 << 3)
#define Y_AXIS_DIR (1 << 4)
#define Y_AXIS_STEP (1 << 5)
#define Y_AXIS_EN (1 << 6)
#define Y_AXIS_POT (1 << 7)

//Port G bits:
#define EX4_1280 (1 << 0)
#define EX3_1280 (1 << 1)
#define B_AXIS_EN (1 << 2)
//Bit 3 unused
#define CUTOFF_SR_CHECK (1 << 4)
#define BUZZ (1 << 5)
//Bit 6 unused
//Bit 7 unused

//Port H bits:
#define CUTOFF_TEST (1 << 0)
#define CUTOFF_RESET (1 << 1)
#define EX1_PWR_CHECK (1 << 2)
#define EX1_HEAT (1 << 3)
#define EX1_FAN (1 << 4)
#define SD_WP (1 << 5)
#define SD_CD (1 << 6)
//Bit 7 unused

//Port J bits:
#define BUTTON_CENTER (1 << 0)
#define BUTTON_RIGHT (1 << 1)
#define BUTTON_LEFT (1 << 2)
#define BUTTON_DOWN (1 << 3)
#define BUTTON_UP (1 << 4)
#define POTS_SCL (1 << 5)
#define B_AXIS_POT (1 << 6)
//Bit 7 unused

//Port K bits:
#define Z_AXIS_DIR (1 << 0)
#define Z_AXIS_STEP (1 << 1)
#define Z_AXIS_EN (1 << 2)
#define Z_AXIS_POT (1 << 3)
#define EX7_1280 (1 << 4)
#define EX6_1280 (1 << 5)
#define EX5_1280 (1 << 6)
#define HBP_THERM (1 << 7)

//Port L bits:
#define X_MIN (1 << 0)
#define X_MAX (1 << 1)
#define Y_MIN (1 << 2)
#define Y_MAX (1 << 3)
#define HBP (1 << 4)
#define EXTRA_FET (1 << 5)
#define Z_MIN (1 << 6)
#define Z_MAX (1 << 7)

/****************************************************\
* I/O devices                                        *
\****************************************************/

class replicator_state : public driver_device
{
public:
	replicator_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcdc(*this, "hd44780"),
		m_dac(*this, "dac"),
		m_io_keypad(*this, "keypad")
	{
	}

	void replicator(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void palette_init(palette_device &palette) const;

	void prg_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	uint8_t port_a_r();
	uint8_t port_b_r();
	uint8_t port_c_r();
	uint8_t port_d_r();
	uint8_t port_e_r();
	uint8_t port_f_r();
	uint8_t port_g_r();
	uint8_t port_h_r();
	uint8_t port_j_r();
	uint8_t port_k_r();
	uint8_t port_l_r();

	void port_a_w(uint8_t data);
	void port_b_w(uint8_t data);
	void port_c_w(uint8_t data);
	void port_d_w(uint8_t data);
	void port_e_w(uint8_t data);
	void port_f_w(uint8_t data);
	void port_g_w(uint8_t data);
	void port_h_w(uint8_t data);
	void port_j_w(uint8_t data);
	void port_k_w(uint8_t data);
	void port_l_w(uint8_t data);

	uint8_t m_port_a;
	uint8_t m_port_b;
	uint8_t m_port_c;
	uint8_t m_port_d;
	uint8_t m_port_e;
	uint8_t m_port_f;
	uint8_t m_port_g;
	uint8_t m_port_h;
	uint8_t m_port_j;
	uint8_t m_port_k;
	uint8_t m_port_l;

	uint8_t m_shift_register_value;

	required_device<atmega1280_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<dac_bit_interface> m_dac;
	required_ioport m_io_keypad;
};

uint8_t replicator_state::port_a_r()
{
	LOGMASKED(LOG_PORT_A, "%s: Port A READ (A-axis signals + B-axis STEP&DIR)\n", machine().describe_context());
	return 0;
}

uint8_t replicator_state::port_b_r()
{
	LOGMASKED(LOG_PORT_B, "%s: Port B READ (SD-CS; 1280-MISO/MOSI/SCK; EX2-FAN/HEAT/PWR-CHECK; BLINK)\n", machine().describe_context());
	return 0;
}

uint8_t replicator_state::port_c_r()
{
	LOGMASKED(LOG_PORT_C, "%s: Port C READ (1280-EX1/EX2; LCD-signals; R&G-LED; DETECT)\n", machine().describe_context());
	return DETECT; //indicated that the Interface board is present.
}

uint8_t replicator_state::port_d_r()
{
	LOGMASKED(LOG_PORT_D, "%s: Port D READ (SDA/SCL; 1280-EX-TX/RX)\n", machine().describe_context());
	return 0;
}

uint8_t replicator_state::port_e_r()
{
	LOGMASKED(LOG_PORT_E, "%s: Port E READ (1280-TX/RX; THERMO-signals)\n", machine().describe_context());
	return 0;
}

uint8_t replicator_state::port_f_r()
{
	LOGMASKED(LOG_PORT_F, "%s: Port F READ (X-axis & Y-axis signals)\n", machine().describe_context());
	return 0;
}

uint8_t replicator_state::port_g_r()
{
	LOGMASKED(LOG_PORT_G, "%s: Port G READ (BUZZ; Cutoff-sr-check; B-axis EN; 1280-EX3/EX4)\n", machine().describe_context());
	return 0;
}

uint8_t replicator_state::port_h_r()
{
	LOGMASKED(LOG_PORT_H, "%s: Port H READ (cuttoff-text/reset; EX1-FAN/HEAT/PWR-CHECK; SD-CD/SD-WP)\n", machine().describe_context());
	return 0;
}

uint8_t replicator_state::port_j_r()
{
	LOGMASKED(LOG_PORT_J, "%s: Port J READ (Interface buttons; POTS-SCL; B-axis-POT)\n", machine().describe_context());
	return m_io_keypad->read();
}

uint8_t replicator_state::port_k_r()
{
	LOGMASKED(LOG_PORT_K, "%s: Port K READ (Z-axis signals; HBP-THERM; 1280-EX5/6/7)\n", machine().describe_context());
	return 0;
}

uint8_t replicator_state::port_l_r()
{
	LOGMASKED(LOG_PORT_L, "%s: Port L READ (HBP; EXTRA-FET; X-MIN/MAX; Y-MIN/MAX; Z-MIN/MAX)\n", machine().describe_context());
	return 0;
}

void replicator_state::port_a_w(uint8_t data)
{
	if (data == m_port_a) return;

	const uint8_t old = m_port_a;
	const uint8_t changed = data ^ old;

	if (changed & A_AXIS_DIR)
		LOGMASKED(LOG_PORT_A, "%s: [A] A_AXIS_DIR: %d\n", machine().describe_context(), data & A_AXIS_DIR ? 1 : 0);
	if (changed & A_AXIS_STEP)
		LOGMASKED(LOG_PORT_A, "%s: [A] A_AXIS_STEP: %d\n", machine().describe_context(), data & A_AXIS_STEP ? 1 : 0);
	if (changed & A_AXIS_EN)
		LOGMASKED(LOG_PORT_A, "%s: [A] A_AXIS_EN: %d\n", machine().describe_context(), data & A_AXIS_EN ? 1 : 0);
	if (changed & A_AXIS_POT)
		LOGMASKED(LOG_PORT_A, "%s: [A] A_AXIS_POT: %d\n", machine().describe_context(), data & A_AXIS_POT ? 1 : 0);
	if (changed & B_AXIS_DIR)
		LOGMASKED(LOG_PORT_A, "%s: [A] B_AXIS_DIR: %d\n", machine().describe_context(), data & B_AXIS_DIR ? 1 : 0);
	if (changed & B_AXIS_STEP)
		LOGMASKED(LOG_PORT_A, "%s: [A] B_AXIS_STEP: %d\n", machine().describe_context(), data & B_AXIS_STEP ? 1 : 0);

	m_port_a = data;
}

void replicator_state::port_b_w(uint8_t data)
{
	if (data == m_port_b) return;

	const uint8_t old = m_port_b;
	const uint8_t changed = data ^ old;

	if (changed & SD_CS)
		LOGMASKED(LOG_PORT_B, "%s: [B] SD Card Chip Select: %d\n", machine().describe_context(), data & SD_CS ? 1 : 0);
	if (changed & SCK_1280)
		LOGMASKED(LOG_PORT_B, "%s: [B] 1280-SCK: %d\n", machine().describe_context(), data & SCK_1280 ? 1 : 0);
	if (changed & MOSI_1280)
		LOGMASKED(LOG_PORT_B, "%s: [B] 1280-MOSI: %d\n", machine().describe_context(), data & MOSI_1280 ? 1 : 0);
	if (changed & MISO_1280)
		LOGMASKED(LOG_PORT_B, "%s: [B] 1280-MISO: %d\n", machine().describe_context(), data & MISO_1280 ? 1 : 0);
	if (changed & EX2_PWR_CHECK)
		LOGMASKED(LOG_PORT_B, "%s: [B] EX2-PWR-CHECK: %d\n", machine().describe_context(), data & EX2_PWR_CHECK ? 1 : 0);
	if (changed & EX2_HEAT)
		LOGMASKED(LOG_PORT_B, "%s: [B] EX2_HEAT: %d\n", machine().describe_context(), data & EX2_HEAT ? 1 : 0);
	if (changed & EX2_FAN)
		LOGMASKED(LOG_PORT_B, "%s: [B] EX2_FAN: %d\n", machine().describe_context(), data & EX2_FAN ? 1 : 0);
	if (changed & BLINK)
		LOGMASKED(LOG_PORT_B, "%s: [B] BLINK: %d\n", machine().describe_context(), data & BLINK ? 1 : 0);

	m_port_b = data;
}

void replicator_state::port_c_w(uint8_t data)
{
	if (data == m_port_c) return;

	const uint8_t old_port_c = m_port_c;
	const uint8_t changed = data ^ old_port_c;

	if(changed & EX2_1280)
		LOGMASKED(LOG_PORT_C, "%s: [C] EX2_1280: %d\n", machine().describe_context(), data & EX2_1280 ? 1 : 0);
	if(changed & EX1_1280)
		LOGMASKED(LOG_PORT_C, "%s: [C] EX1_1280: %d\n", machine().describe_context(), data & EX1_1280 ? 1 : 0);
	if(changed & LCD_CLK)
		LOGMASKED(LOG_PORT_C, "%s: [C] LCD_CLK: %d\n", machine().describe_context(), data & LCD_CLK ? 1 : 0);
	if(changed & LCD_DATA)
		LOGMASKED(LOG_PORT_C, "%s: [C] LCD_DATA: %d\n", machine().describe_context(), data & LCD_DATA ? 1 : 0);
	if(changed & LCD_STROBE)
		LOGMASKED(LOG_PORT_C, "%s: [C] LCD_STROBE: %d\n", machine().describe_context(), data & LCD_STROBE ? 1 : 0);
	if(changed & RLED)
		LOGMASKED(LOG_PORT_C, "%s: [C] RLED: %d\n", machine().describe_context(), data & RLED ? 1 : 0);
	if(changed & GLED)
		LOGMASKED(LOG_PORT_C, "%s: [C] GLED: %d\n", machine().describe_context(), data & GLED ? 1 : 0);
	if(changed & DETECT)
		LOGMASKED(LOG_PORT_C, "%s: [C] DETECT: %d\n", machine().describe_context(), data & DETECT ? 1 : 0);

	if (changed & LCD_CLK)
	{
		/* The LCD is interfaced by an 8-bit shift register (74HC4094). */
		if (data & LCD_CLK) // CLK positive edge
		{
			m_shift_register_value = (m_shift_register_value << 1) | ((data & LCD_DATA) >> 3);
			LOGMASKED(LOG_LCD_CLK, "%s: [C] LCD CLK positive edge. shift_register=0x%02X\n", machine().describe_context(), m_shift_register_value);
		}
	}

	if (changed & LCD_STROBE)
	{
		if (data & LCD_STROBE) // STROBE positive edge
		{
			LOGMASKED(LOG_LCD_SHIFT, "%s: LCD shift register = %02X\n", machine().describe_context(), m_shift_register_value);
			m_lcdc->rs_w(BIT(m_shift_register_value, 1));
			m_lcdc->rw_w(BIT(m_shift_register_value, 2));
			m_lcdc->e_w(BIT(m_shift_register_value, 3));
			m_lcdc->db_w(m_shift_register_value & 0xF0);
		}
	}

	m_port_c = data;
}

void replicator_state::port_d_w(uint8_t data)
{
	if (data == m_port_d) return;

	const uint8_t old = m_port_d;
	const uint8_t changed = data ^ old;

	if (changed & PORTD_SCL)
		LOGMASKED(LOG_PORT_D, "%s: [D] PORTD_SCL: %d\n", machine().describe_context(), data & PORTD_SCL ? 1 : 0);
	if (changed & PORTD_SDA)
		LOGMASKED(LOG_PORT_D, "%s: [D] PORTD_SDA: %d\n", machine().describe_context(), data & PORTD_SDA ? 1 : 0);
	if (changed & EX_RX_1280)
		LOGMASKED(LOG_PORT_D, "%s: [D] EX_RX_1280: %d\n", machine().describe_context(), data & EX_RX_1280 ? 1 : 0);
	if (changed & EX_TX_1280)
		LOGMASKED(LOG_PORT_D, "%s: [D] EX_TX_1280: %d\n", machine().describe_context(), data & EX_TX_1280 ? 1 : 0);

	m_port_d = data;
}

void replicator_state::port_e_w(uint8_t data)
{
	if (data == m_port_e) return;

	const uint8_t old = m_port_e;
	const uint8_t changed = data ^ old;

	if (changed & RX_1280)
		LOGMASKED(LOG_PORT_E, "%s: [E] 1280-RX: %d\n", machine().describe_context(), data & RX_1280 ? 1 : 0);
	if (changed & TX_1280)
		LOGMASKED(LOG_PORT_E, "%s: [E] 1280-TX: %d\n", machine().describe_context(), data & TX_1280 ? 1 : 0);
	if (changed & THERMO_SCK)
		LOGMASKED(LOG_PORT_E, "%s: [E] THERMO-SCK: %d\n", machine().describe_context(), data & THERMO_SCK ? 1 : 0);
	if (changed & THERMO_CS1)
		LOGMASKED(LOG_PORT_E, "%s: [E] THERMO-CS1: %d\n", machine().describe_context(), data & THERMO_CS1 ? 1 : 0);
	if (changed & THERMO_CS2)
		LOGMASKED(LOG_PORT_E, "%s: [E] THERMO-CS2: %d\n", machine().describe_context(), data & THERMO_CS2 ? 1 : 0);
	if (changed & THERMO_DO)
		LOGMASKED(LOG_PORT_E, "%s: [E] THERMO-DO: %d\n", machine().describe_context(), data & THERMO_DO ? 1 : 0);

	m_port_e = data;
}

void replicator_state::port_f_w(uint8_t data)
{
	if (data == m_port_f) return;

	const uint8_t old = m_port_f;
	const uint8_t changed = data ^ old;

	if (changed & X_AXIS_DIR)
		LOGMASKED(LOG_PORT_F, "%s: [F] X_AXIS_DIR: %d\n", machine().describe_context(), data & X_AXIS_DIR ? 1 : 0);
	if (changed & X_AXIS_STEP)
		LOGMASKED(LOG_PORT_F, "%s: [F] X_AXIS_STEP: %d\n", machine().describe_context(), data & X_AXIS_STEP ? 1 : 0);
	if (changed & X_AXIS_EN)
		LOGMASKED(LOG_PORT_F, "%s: [F] X_AXIS_EN: %d\n", machine().describe_context(), data & X_AXIS_EN ? 1 : 0);
	if (changed & X_AXIS_POT)
		LOGMASKED(LOG_PORT_F, "%s: [F] X_AXIS_POT: %d\n", machine().describe_context(), data & X_AXIS_POT ? 1 : 0);
	if (changed & Y_AXIS_DIR)
		LOGMASKED(LOG_PORT_F, "%s: [F] Y_AXIS_DIR: %d\n", machine().describe_context(), data & Y_AXIS_DIR ? 1 : 0);
	if (changed & Y_AXIS_STEP)
		LOGMASKED(LOG_PORT_F, "%s: [F] Y_AXIS_STEP: %d\n", machine().describe_context(), data & Y_AXIS_STEP ? 1 : 0);
	if (changed & Y_AXIS_EN)
		LOGMASKED(LOG_PORT_F, "%s: [F] Y_AXIS_EN: %d\n", machine().describe_context(), data & Y_AXIS_EN ? 1 : 0);
	if (changed & Y_AXIS_POT)
		LOGMASKED(LOG_PORT_F, "%s: [F] Y_AXIS_POT: %d\n", machine().describe_context(), data & Y_AXIS_POT ? 1 : 0);

	m_port_f = data;
}

void replicator_state::port_g_w(uint8_t data)
{
	if (data == m_port_g) return;

	const uint8_t old = m_port_g;
	const uint8_t changed = data ^ old;

	if (changed & EX4_1280)
		LOGMASKED(LOG_PORT_G, "%s: [G] EX4_1280: %d\n", machine().describe_context(), data & EX4_1280 ? 1 : 0);
	if (changed & EX3_1280)
		LOGMASKED(LOG_PORT_G, "%s: [G] EX3_1280: %d\n", machine().describe_context(), data & EX3_1280 ? 1 : 0);
	if (changed & B_AXIS_EN)
		LOGMASKED(LOG_PORT_G, "%s: [G] B_AXIS_EN: %d\n", machine().describe_context(), data & B_AXIS_EN ? 1 : 0);
	if (changed & CUTOFF_SR_CHECK)
		LOGMASKED(LOG_PORT_G, "%s: [G] CUTOFF_SR_CHECK: %d\n", machine().describe_context(), data & CUTOFF_SR_CHECK ? 1 : 0);
	if (changed & BUZZ)
		LOGMASKED(LOG_PORT_G, "%s: [G] BUZZ: %d\n", machine().describe_context(), data & BUZZ ? 1 : 0);

	if (changed & BUZZ)
	{
		m_dac->write(BIT(data, 5));
	}

	m_port_g = data;
}

void replicator_state::port_h_w(uint8_t data)
{
	if (data == m_port_h) return;

	const uint8_t old = m_port_h;
	const uint8_t changed = data ^ old;

	if (changed & CUTOFF_TEST)
		LOGMASKED(LOG_PORT_H, "%s: [H] CUTOFF_TEST: %d\n", machine().describe_context(), data & CUTOFF_TEST ? 1 : 0);
	if (changed & CUTOFF_RESET)
		LOGMASKED(LOG_PORT_H, "%s: [H] CUTOFF_RESET: %d\n", machine().describe_context(), data & CUTOFF_RESET ? 1 : 0);
	if (changed & EX1_PWR_CHECK)
		LOGMASKED(LOG_PORT_H, "%s: [H] EX1_PWR_CHECK: %d\n", machine().describe_context(), data & EX1_PWR_CHECK ? 1 : 0);
	if (changed & EX1_HEAT)
		LOGMASKED(LOG_PORT_H, "%s: [H] EX1_HEAT: %d\n", machine().describe_context(), data & EX1_HEAT ? 1 : 0);
	if (changed & EX1_FAN)
		LOGMASKED(LOG_PORT_H, "%s: [H] EX1_FAN: %d\n", machine().describe_context(), data & EX1_FAN ? 1 : 0);
	if (changed & SD_WP)
		LOGMASKED(LOG_PORT_H, "%s: [H] SD_WP: %d\n", machine().describe_context(), data & SD_WP ? 1 : 0);
	if (changed & SD_CD)
		LOGMASKED(LOG_PORT_H, "%s: [H] SD_CD: %d\n", machine().describe_context(), data & SD_CD ? 1 : 0);

	m_port_h = data;
}

void replicator_state::port_j_w(uint8_t data)
{
	if (data == m_port_j) return;

	const uint8_t old = m_port_j;
	const uint8_t changed = data ^ old;

	if (changed & BUTTON_CENTER)
		LOGMASKED(LOG_PORT_J, "%s: [J] BUTTON_CENTER: %d\n", machine().describe_context(), data & BUTTON_CENTER ? 1 : 0);
	if (changed & BUTTON_RIGHT)
		LOGMASKED(LOG_PORT_J, "%s: [J] BUTTON_RIGHT: %d\n", machine().describe_context(), data & BUTTON_RIGHT ? 1 : 0);
	if (changed & BUTTON_LEFT)
		LOGMASKED(LOG_PORT_J, "%s: [J] BUTTON_LEFT: %d\n", machine().describe_context(), data & BUTTON_LEFT ? 1 : 0);
	if (changed & BUTTON_DOWN)
		LOGMASKED(LOG_PORT_J, "%s: [J] BUTTON_DOWN: %d\n", machine().describe_context(), data & BUTTON_DOWN ? 1 : 0);
	if (changed & BUTTON_UP)
		LOGMASKED(LOG_PORT_J, "%s: [J] BUTTON_UP: %d\n", machine().describe_context(), data & BUTTON_UP ? 1 : 0);
	if (changed & POTS_SCL)
		LOGMASKED(LOG_PORT_J, "%s: [J] POTS_SCL: %d\n", machine().describe_context(), data & POTS_SCL ? 1 : 0);
	if (changed & B_AXIS_POT)
		LOGMASKED(LOG_PORT_J, "%s: [J] B_AXIS_POT: %d\n", machine().describe_context(), data & B_AXIS_POT ? 1 : 0);

	m_port_j = data;
}

void replicator_state::port_k_w(uint8_t data)
{
	if (data == m_port_k) return;

	const uint8_t old = m_port_k;
	const uint8_t changed = data ^ old;

	if (changed & Z_AXIS_DIR)
		LOGMASKED(LOG_PORT_K, "%s: [K] Z_AXIS_DIR: %d\n", machine().describe_context(), data & Z_AXIS_DIR ? 1 : 0);
	if (changed & Z_AXIS_STEP)
		LOGMASKED(LOG_PORT_K, "%s: [K] Z_AXIS_STEP: %d\n", machine().describe_context(), data & Z_AXIS_STEP ? 1 : 0);
	if (changed & Z_AXIS_EN)
		LOGMASKED(LOG_PORT_K, "%s: [K] Z_AXIS_EN: %d\n", machine().describe_context(), data & Z_AXIS_EN ? 1 : 0);
	if (changed & Z_AXIS_POT)
		LOGMASKED(LOG_PORT_K, "%s: [K] Z_AXIS_POT: %d\n", machine().describe_context(), data & Z_AXIS_POT ? 1 : 0);
	if (changed & EX7_1280)
		LOGMASKED(LOG_PORT_K, "%s: [K] EX7_1280: %d\n", machine().describe_context(), data & EX7_1280 ? 1 : 0);
	if (changed & EX6_1280)
		LOGMASKED(LOG_PORT_K, "%s: [K] EX6_1280: %d\n", machine().describe_context(), data & EX6_1280 ? 1 : 0);
	if (changed & EX5_1280)
		LOGMASKED(LOG_PORT_K, "%s: [K] EX5_1280: %d\n", machine().describe_context(), data & EX5_1280 ? 1 : 0);
	if (changed & HBP_THERM)
		LOGMASKED(LOG_PORT_K, "%s: [K] HBP_THERM: %d\n", machine().describe_context(), data & HBP_THERM ? 1 : 0);

	m_port_k = data;
}

void replicator_state::port_l_w(uint8_t data)
{
	if (data == m_port_l) return;

	const uint8_t old_port_l = m_port_l;
	const uint8_t changed = data ^ old_port_l;

	if (changed & X_MIN)
		LOGMASKED(LOG_PORT_L, "%s: [L] X_MIN: %d\n", machine().describe_context(), data & X_MIN ? 1 : 0);
	if (changed & X_MAX)
		LOGMASKED(LOG_PORT_L, "%s: [L] X_MAX: %d\n", machine().describe_context(), data & X_MAX ? 1 : 0);
	if (changed & Y_MIN)
		LOGMASKED(LOG_PORT_L, "%s: [L] Y_MIN: %d\n", machine().describe_context(), data & Y_MIN ? 1 : 0);
	if (changed & Y_MAX)
		LOGMASKED(LOG_PORT_L, "%s: [L] Y_MAX: %d\n", machine().describe_context(), data & Y_MAX ? 1 : 0);
	if (changed & HBP)
		LOGMASKED(LOG_PORT_L, "%s: [L] HBP: %d\n", machine().describe_context(), data & HBP ? 1 : 0);
	if (changed & EXTRA_FET)
		LOGMASKED(LOG_PORT_L, "%s: [L] EXTRA_FET: %d\n", data & EXTRA_FET ? 1 : 0);
	if (changed & Z_MIN)
		LOGMASKED(LOG_PORT_L, "%s: [L] Z_MIN: %d\n", machine().describe_context(), data & Z_MIN ? 1 : 0);
	if (changed & Z_MAX)
		LOGMASKED(LOG_PORT_L, "%s: [L] Z_MAX: %d\n", machine().describe_context(), data & Z_MAX ? 1 : 0);

	m_port_l = data;
}

/****************************************************\
* Address maps                                       *
\****************************************************/

void replicator_state::prg_map(address_map &map)
{
	map(0x0000, 0x1FFFF).rom();
}

void replicator_state::data_map(address_map &map)
{
	map(0x0200, 0x21FF).ram();  /* ATMEGA1280 Internal SRAM */
}

/****************************************************\
* Input ports                                        *
\****************************************************/

static INPUT_PORTS_START( replicator )
	PORT_START("keypad")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CENTER") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_W)
INPUT_PORTS_END

/****************************************************\
* Machine definition                                 *
\****************************************************/

void replicator_state::machine_start()
{
	save_item(NAME(m_shift_register_value));
	save_item(NAME(m_port_a));
	save_item(NAME(m_port_b));
	save_item(NAME(m_port_c));
	save_item(NAME(m_port_d));
	save_item(NAME(m_port_e));
	save_item(NAME(m_port_f));
	save_item(NAME(m_port_g));
	save_item(NAME(m_port_h));
	save_item(NAME(m_port_j));
	save_item(NAME(m_port_k));
	save_item(NAME(m_port_l));
}

void replicator_state::machine_reset()
{
	m_shift_register_value = 0;
	m_port_a = 0;
	m_port_b = 0;
	m_port_c = 0;
	m_port_d = 0;
	m_port_e = 0;
	m_port_f = 0;
	m_port_g = 0;
	m_port_h = 0;
	m_port_j = 0;
	m_port_k = 0;
	m_port_l = 0;
}

void replicator_state::palette_init(palette_device &palette) const
{
	// These colors were picked with the color picker in Inkscape, based on a photo of the LCD used in the Replicator 1 3d printer:
	palette.set_pen_color(0, rgb_t(0xca, 0xe7, 0xeb));
	palette.set_pen_color(1, rgb_t(0x78, 0xab, 0xa8));
}

static const gfx_layout hd44780_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( gfx_replicator )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, hd44780_charlayout, 0, 1 )
GFXDECODE_END

void replicator_state::replicator(machine_config &config)
{
	ATMEGA1280(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &replicator_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &replicator_state::data_map);

	m_maincpu->set_eeprom_tag("eeprom");
	m_maincpu->set_low_fuses(0xff);
	m_maincpu->set_high_fuses(0xda);
	m_maincpu->set_extended_fuses(0xf4);
	m_maincpu->set_lock_bits(0x0f);

	m_maincpu->gpio_in<atmega1280_device::GPIOA>().set(FUNC(replicator_state::port_a_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOB>().set(FUNC(replicator_state::port_b_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOC>().set(FUNC(replicator_state::port_c_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOD>().set(FUNC(replicator_state::port_d_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOE>().set(FUNC(replicator_state::port_e_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOF>().set(FUNC(replicator_state::port_f_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOG>().set(FUNC(replicator_state::port_g_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOH>().set(FUNC(replicator_state::port_h_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOJ>().set(FUNC(replicator_state::port_j_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOK>().set(FUNC(replicator_state::port_k_r));
	m_maincpu->gpio_in<atmega1280_device::GPIOL>().set(FUNC(replicator_state::port_l_r));

	m_maincpu->gpio_out<atmega1280_device::GPIOA>().set(FUNC(replicator_state::port_a_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOB>().set(FUNC(replicator_state::port_b_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOC>().set(FUNC(replicator_state::port_c_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOD>().set(FUNC(replicator_state::port_d_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOE>().set(FUNC(replicator_state::port_e_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOF>().set(FUNC(replicator_state::port_f_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOG>().set(FUNC(replicator_state::port_g_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOH>().set(FUNC(replicator_state::port_h_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOJ>().set(FUNC(replicator_state::port_j_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOK>().set(FUNC(replicator_state::port_k_w));
	m_maincpu->gpio_out<atmega1280_device::GPIOL>().set(FUNC(replicator_state::port_l_w));

	/*TODO: Add an ATMEGA8U2 for USB-Serial communications */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_size(120, 18*2); //4x20 chars
	screen.set_visarea(0, 120-1, 0, 18*2-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(replicator_state::palette_init), 2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_replicator);

	HD44780(config, "hd44780", 270'000).set_lcd_size(4, 20); // TODO: clock not measured, datasheet typical clock used

	/* sound hardware */
	/* A piezo is connected to the PORT G bit 5 (OC0B pin driven by Timer/Counter #4) */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(0, "speaker", 0.5);
}

ROM_START( replica1 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v750")

	/* Version 5.1 release:
	- Initial firmware release
	*/
	ROM_SYSTEM_BIOS( 0, "v51", "V 5.1" )
	ROMX_LOAD("mighty-mb40-v5.1.bin", 0x0000, 0x10b90, CRC(20d65cd1) SHA1(da18c3eb5a29a6bc1eecd92eaae6063fe29d0305), ROM_BIOS(0))

	/* Version 5.2 release:
	- Nozzle Tolerance added to EEPROM
	- Updated onboard menus
	- X,Y calibration tool added
	*/
	ROM_SYSTEM_BIOS( 1, "v52", "V 5.2" )
	ROMX_LOAD("mighty-mb40-v5.2.bin", 0x0000, 0x126c4, CRC(555e47cf) SHA1(9d24a3dbeddce16669bb4d29c3366220ddf15d2a), ROM_BIOS(1))

	/* Version 5.5 release:
	- Acceleration added to motor motion
	- Digipot updates
	*/
	ROM_SYSTEM_BIOS( 2, "v55", "V 5.5" )
	ROMX_LOAD("mighty-mb40-v5.5.bin", 0x0000, 0x1a420, CRC(9327d7e4) SHA1(d734ba2bda12f50ec3ac0035ab11591909d9edde), ROM_BIOS(2))

	/* Version 6.2.0 release:
	- Bug fix release to firmware 6.0
	- Addresses wavy print issue above 1cm
	- Left extruder prints with makerware.
	*/
	ROM_SYSTEM_BIOS( 3, "v620", "V 6.2.0" )
	ROMX_LOAD("mighty_one_v6.2.0.bin", 0x0000, 0x1cf54, CRC(00df6f48) SHA1(db05afc2e1ebc104fb04753634a911187e396556), ROM_BIOS(3))

	/* Version 7.0.0 release:
	- Major upgrade to Stepper Motor Smoothness (via Sailfish team)
	- X3G format introduced
	- Heaters default to leaving 'preheat' on more of the time
	*/
	ROM_SYSTEM_BIOS( 4, "v700", "V 7.0.0" )
	ROMX_LOAD("mighty_one_v7.0.0.bin", 0x0000, 0x1cb52, CRC(aa2a5fcf) SHA1(934e642b0b2d007689249680bad03c9255ae016a), ROM_BIOS(4))

	/* Version 7.2.0 release:
	- Removes support for S3G files
	- X3G is the recognized format
	- Minor bug fixes
	*/
	ROM_SYSTEM_BIOS( 5, "v720", "V 7.2.0" )
	ROMX_LOAD("mighty_one_v7.2.0.bin", 0x0000, 0x1cb80, CRC(5e546706) SHA1(ed4aaf7522d5a5beea7eb69bf2c85d7a89f8f188), ROM_BIOS(5))

	/* Version 7.3.0 release:
	- Pause at Z Height
	- Elapsed time displays during prints
	- Minor bug fixes
	*/
	ROM_SYSTEM_BIOS( 6, "v730", "V 7.3.0" )
	ROMX_LOAD("mighty_one_v7.3.0.bin", 0x0000, 0x1d738, CRC(71811ff5) SHA1(6728ea600ab3ff4b589adca90b0d700d9b70bd18), ROM_BIOS(6))

	/* Version 7.4.0 (bugfix) release:
	- Fixes issues with Z Pause and elapsed print time
	*/
	ROM_SYSTEM_BIOS( 7, "v740", "V 7.4.0" )
	ROMX_LOAD("mighty_one_v7.4.0.bin", 0x0000, 0x1b9e2, CRC(97b05a27) SHA1(76ca2c9c1db2e006e501c3177a8a1aa693dda0f9), ROM_BIOS(7))

	/* Version 7.5.0 (bugfix) release:
	- Fixes issue with Heat Hold
	*/
	ROM_SYSTEM_BIOS( 8, "v750", "V 7.5.0" )
	ROMX_LOAD("mighty_one_v7.5.0.bin", 0x0000, 0x1b9c4, CRC(169d6709) SHA1(62b5aacd1bc46969042aea7a50531ec467a4ff1f), ROM_BIOS(8))

	/* Sailfish firmware image - Metam??quina experimental build v7.5.0 */
	ROM_SYSTEM_BIOS( 9, "v750mm", "V 7.5.0 - Metam??quina" )
	ROMX_LOAD("mighty_one_v7.5.0.mm.bin", 0x0000, 0x1ef9a, CRC(0d36d9e7) SHA1(a53899775b4c4eea87b6903758ebb75f06710a69), ROM_BIOS(9))


	/*Arduino MEGA bootloader */
	ROM_LOAD( "atmegaboot_168_atmega1280.bin", 0x1f000, 0x0f16, CRC(c041f8db) SHA1(d995ebf360a264cccacec65f6dc0c2257a3a9224) )

	/* on-die 4kbyte eeprom */
	ROM_REGION( 0x1000, "eeprom", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


/*   YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY     FULLNAME */
COMP(2012, replica1, 0,      0,      replicator, replicator, replicator_state, empty_init, "Makerbot", "Replicator 1 desktop 3d printer", MACHINE_NOT_WORKING)
