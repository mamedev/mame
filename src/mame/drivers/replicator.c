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
#include "video/hd44780.h"
#include "rendlay.h"
#include "debugger.h"
#include "sound/dac.h"

#define MASTER_CLOCK    16000000

#define LOG_PORTS 0

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
	replicator_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcdc(*this, "hd44780"),
		m_dac(*this, "dac")
	{
	}

	virtual void machine_start();

	UINT8 m_port_a;
	UINT8 m_port_b;
	UINT8 m_port_c;
	UINT8 m_port_d;
	UINT8 m_port_e;
	UINT8 m_port_f;
	UINT8 m_port_g;
	UINT8 m_port_h;
	UINT8 m_port_j;
	UINT8 m_port_k;
	UINT8 m_port_l;

	UINT8 shift_register_value;

	required_device<avr8_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<dac_device> m_dac;

	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(port_w);
	DECLARE_DRIVER_INIT(replicator);
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(replicator);
};

void replicator_state::machine_start()
{
}

READ8_MEMBER(replicator_state::port_r)
{
	switch( offset )
	{
		case AVR8_IO_PORTA:
	{
#if LOG_PORTS
		printf("[%08X] Port A READ (A-axis signals + B-axis STEP&DIR)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
		case AVR8_IO_PORTB:
	{
#if LOG_PORTS
		printf("[%08X] Port B READ (SD-CS; 1280-MISO/MOSI/SCK; EX2-FAN/HEAT/PWR-CHECK; BLINK)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
		case AVR8_IO_PORTC:
	{
#if LOG_PORTS
		printf("[%08X] Port C READ (1280-EX1/EX2; LCD-signals; R&G-LED; DETECT)\n", m_maincpu->m_shifted_pc);
#endif
		return DETECT; //indicated that the Interface board is present.
	}
		case AVR8_IO_PORTD:
	{
#if LOG_PORTS
		printf("[%08X] Port D READ (SDA/SCL; 1280-EX-TX/RX)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
		case AVR8_IO_PORTE:
	{
#if LOG_PORTS
		printf("[%08X] Port E READ (1280-TX/RX; THERMO-signals)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
		case AVR8_IO_PORTF:
	{
#if LOG_PORTS
		printf("[%08X] Port F READ (X-axis & Y-axis signals)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
		case AVR8_IO_PORTG:
	{
#if LOG_PORTS
		printf("[%08X] Port G READ (BUZZ; Cutoff-sr-check; B-axis EN; 1280-EX3/EX4)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
		case AVR8_IO_PORTH:
	{
#if LOG_PORTS
		printf("[%08X] Port H READ (cuttoff-text/reset; EX1-FAN/HEAT/PWR-CHECK; SD-CD/SD-WP)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
		case AVR8_IO_PORTJ:
	{
#if LOG_PORTS
		printf("[%08X] Port J READ (Interface buttons; POTS-SCL; B-axis-POT)\n", m_maincpu->m_shifted_pc);
#endif
		return ioport("keypad")->read();
	}
		case AVR8_IO_PORTK:
	{
#if LOG_PORTS
		printf("[%08X] Port K READ (Z-axis signals; HBP-THERM; 1280-EX5/6/7)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
		case AVR8_IO_PORTL:
	{
#if LOG_PORTS
		printf("[%08X] Port L READ (HBP; EXTRA-FET; X-MIN/MAX; Y-MIN/MAX; Z-MIN/MAX)\n", m_maincpu->m_shifted_pc);
#endif
		return 0x00;
	}
	}
	return 0;
}

WRITE8_MEMBER(replicator_state::port_w)
{
	switch( offset )
	{
		case AVR8_IO_PORTA:
	{
		if (data == m_port_a) break;

#if LOG_PORTS
			UINT8 old_port_a = m_port_a;
			UINT8 changed = data ^ old_port_a;

		printf("[%08X] ", m_maincpu->m_shifted_pc);
			if(changed & A_AXIS_DIR) printf("[A] A_AXIS_DIR: %s\n", data & A_AXIS_DIR ? "HIGH" : "LOW");
			if(changed & A_AXIS_STEP) printf("[A] A_AXIS_STEP: %s\n", data & A_AXIS_STEP ? "HIGH" : "LOW");
			if(changed & A_AXIS_EN) printf("[A] A_AXIS_EN: %s\n", data & A_AXIS_EN ? "HIGH" : "LOW");
			if(changed & A_AXIS_POT) printf("[A] A_AXIS_POT: %s\n", data & A_AXIS_POT ? "HIGH" : "LOW");
			if(changed & B_AXIS_DIR) printf("[A] B_AXIS_DIR: %s\n", data & B_AXIS_DIR ? "HIGH" : "LOW");
			if(changed & B_AXIS_STEP) printf("[A] B_AXIS_STEP: %s\n", data & B_AXIS_STEP ? "HIGH" : "LOW");
#endif

		m_port_a = data;
		break;
	}
		case AVR8_IO_PORTB:
	{
		if (data == m_port_b) break;

#if LOG_PORTS
			UINT8 old_port_b = m_port_b;
			UINT8 changed = data ^ old_port_b;

		printf("[%08X] ", m_maincpu->m_shifted_pc);
			if(changed & SD_CS) printf("[B] SD Card Chip Select: %s\n", data & SD_CS ? "HIGH" : "LOW");
			if(changed & SCK_1280) printf("[B] 1280-SCK: %s\n", data & SCK_1280 ? "HIGH" : "LOW");
			if(changed & MOSI_1280) printf("[B] 1280-MOSI: %s\n", data & MOSI_1280 ? "HIGH" : "LOW");
			if(changed & MISO_1280) printf("[B] 1280-MISO: %s\n", data & MISO_1280 ? "HIGH" : "LOW");
			if(changed & EX2_PWR_CHECK) printf("[B] EX2-PWR-CHECK: %s\n", data & EX2_PWR_CHECK ? "HIGH" : "LOW");
			if(changed & EX2_HEAT) printf("[B] EX2_HEAT: %s\n", data & EX2_HEAT ? "HIGH" : "LOW");
			if(changed & EX2_FAN) printf("[B] EX2_FAN: %s\n", data & EX2_FAN ? "HIGH" : "LOW");
			if(changed & BLINK) printf("[B] BLINK: %s\n", data & BLINK ? "HIGH" : "LOW");
#endif

		m_port_b = data;
		break;
	}
		case AVR8_IO_PORTC:
	{
		if (data == m_port_c) break;

			UINT8 old_port_c = m_port_c;
			UINT8 changed = data ^ old_port_c;
#if LOG_PORTS
		printf("[%08X] ", m_maincpu->m_shifted_pc);
			if(changed & EX2_1280) printf("[C] EX2_1280: %s\n", data & EX2_1280 ? "HIGH" : "LOW");
			if(changed & EX1_1280) printf("[C] EX1_1280: %s\n", data & EX1_1280 ? "HIGH" : "LOW");
			if(changed & LCD_CLK) printf("[C] LCD_CLK: %s\n", data & LCD_CLK ? "HIGH" : "LOW");
			if(changed & LCD_DATA) printf("[C] LCD_DATA: %s\n", data & LCD_DATA ? "HIGH" : "LOW");
			if(changed & LCD_STROBE) printf("[C] LCD_STROBE: %s\n", data & LCD_STROBE ? "HIGH" : "LOW");
			if(changed & RLED) printf("[C] RLED: %s\n", data & RLED ? "HIGH" : "LOW");
			if(changed & GLED) printf("[C] GLED: %s\n", data & GLED ? "HIGH" : "LOW");
			if(changed & DETECT) printf("[C] DETECT: %s\n", data & DETECT ? "HIGH" : "LOW");
#endif

			if (changed & LCD_CLK){
		/* The LCD is interfaced by an 8-bit shift register (74HC4094). */
		if (data & LCD_CLK){//CLK positive edge
			shift_register_value = (shift_register_value << 1) | ((data & LCD_DATA) >> 3);
			//printf("[%08X] ", m_maincpu->m_shifted_pc);
			//printf("[C] LCD CLK positive edge. shift_register=0x%02X\n", shift_register_value);
				}
		}

			if(changed & LCD_STROBE){
		if (data & LCD_STROBE){ //STROBE positive edge
			bool RS = (shift_register_value >> 1) & 1;
			bool RW = (shift_register_value >> 2) & 1;
			bool enable = (shift_register_value >> 3) & 1;
			UINT8 lcd_data = shift_register_value & 0xF0;

			if (enable && RW==0){
			if (RS==0){
					m_lcdc->control_write(space, 0, lcd_data);
			} else {
					m_lcdc->data_write(space, 0, lcd_data);
			}
			}
				}
		}
			m_port_c = data;

			break;
		}
		case AVR8_IO_PORTD:
	{
		if (data == m_port_d) break;

#if LOG_PORTS
			UINT8 old_port_d = m_port_d;
			UINT8 changed = data ^ old_port_d;

		printf("[%08X] ", m_maincpu->m_shifted_pc);
			if(changed & PORTD_SCL) printf("[D] PORTD_SCL: %s\n", data & PORTD_SCL ? "HIGH" : "LOW");
			if(changed & PORTD_SDA) printf("[D] PORTD_SDA: %s\n", data & PORTD_SDA ? "HIGH" : "LOW");
			if(changed & EX_RX_1280) printf("[D] EX_RX_1280: %s\n", data & EX_RX_1280 ? "HIGH" : "LOW");
			if(changed & EX_TX_1280) printf("[D] EX_TX_1280: %s\n", data & EX_TX_1280 ? "HIGH" : "LOW");
#endif

			m_port_d = data;
		break;
	}
		case AVR8_IO_PORTE:
	{
		if (data == m_port_e) break;

#if LOG_PORTS
			UINT8 old_port_e = m_port_e;
			UINT8 changed = data ^ old_port_e;

		printf("[%08X] ", m_maincpu->m_shifted_pc);
			if(changed & RX_1280) printf("[E] 1280-RX: %s\n", data & RX_1280 ? "HIGH" : "LOW");
			if(changed & TX_1280) printf("[E] 1280-TX: %s\n", data & TX_1280 ? "HIGH" : "LOW");
			if(changed & THERMO_SCK) printf("[E] THERMO-SCK: %s\n", data & THERMO_SCK ? "HIGH" : "LOW");
			if(changed & THERMO_CS1) printf("[E] THERMO-CS1: %s\n", data & THERMO_CS1 ? "HIGH" : "LOW");
			if(changed & THERMO_CS2) printf("[E] THERMO-CS2: %s\n", data & THERMO_CS2 ? "HIGH" : "LOW");
			if(changed & THERMO_DO) printf("[E] THERMO-DO: %s\n", data & THERMO_DO ? "HIGH" : "LOW");
#endif

			m_port_e = data;
		break;
	}
		case AVR8_IO_PORTF:
	{
		if (data == m_port_f) break;

#if LOG_PORTS
			UINT8 old_port_f = m_port_f;
			UINT8 changed = data ^ old_port_f;

		printf("[%08X] ", m_maincpu->m_shifted_pc);
			if(changed & X_AXIS_DIR) printf("[F] X_AXIS_DIR: %s\n", data & X_AXIS_DIR ? "HIGH" : "LOW");
			if(changed & X_AXIS_STEP) printf("[F] X_AXIS_STEP: %s\n", data & X_AXIS_STEP ? "HIGH" : "LOW");
			if(changed & X_AXIS_EN) printf("[F] X_AXIS_EN: %s\n", data & X_AXIS_EN ? "HIGH" : "LOW");
			if(changed & X_AXIS_POT) printf("[F] X_AXIS_POT: %s\n", data & X_AXIS_POT ? "HIGH" : "LOW");
			if(changed & Y_AXIS_DIR) printf("[F] Y_AXIS_DIR: %s\n", data & Y_AXIS_DIR ? "HIGH" : "LOW");
			if(changed & Y_AXIS_STEP) printf("[F] Y_AXIS_STEP: %s\n", data & Y_AXIS_STEP ? "HIGH" : "LOW");
			if(changed & Y_AXIS_EN) printf("[F] Y_AXIS_EN: %s\n", data & Y_AXIS_EN ? "HIGH" : "LOW");
			if(changed & Y_AXIS_POT) printf("[F] Y_AXIS_POT: %s\n", data & Y_AXIS_POT ? "HIGH" : "LOW");
#endif

			m_port_f = data;
		break;
	}
		case AVR8_IO_PORTG:
	{
		if (data == m_port_g) break;

			UINT8 old_port_g = m_port_g;
			UINT8 changed = data ^ old_port_g;

#if LOG_PORTS
		printf("[%08X] ", m_maincpu->m_shifted_pc);
			if(changed & EX4_1280) printf("[G] EX4_1280: %s\n", data & EX4_1280 ? "HIGH" : "LOW");
			if(changed & EX3_1280) printf("[G] EX3_1280: %s\n", data & EX3_1280 ? "HIGH" : "LOW");
			if(changed & B_AXIS_EN) printf("[G] B_AXIS_EN: %s\n", data & B_AXIS_EN ? "HIGH" : "LOW");
			if(changed & CUTOFF_SR_CHECK) printf("[G] CUTOFF_SR_CHECK: %s\n", data & CUTOFF_SR_CHECK ? "HIGH" : "LOW");
			if(changed & BUZZ) printf("[G] BUZZ: %s\n", data & BUZZ ? "HIGH" : "LOW");
#endif

			if(changed & BUZZ){
		/* FIX-ME: What is the largest sample value allowed?
		 I'm using 0x3F based on what I see in src/mame/drivers/craft.c
		 But as the method is called "write_unsigned8", I guess we could have samples with values up to 0xFF, right?
		 Anyway... With the 0x3F value we'll get a sound that is not so loud, which may be less annoying... :-)
		*/
				UINT8 audio_sample = (data & BUZZ) ? 0x3F : 0;
				m_dac->write_unsigned8(audio_sample << 1);
		}

			m_port_g = data;
		break;
	}
		case AVR8_IO_PORTH:
	{
		if (data == m_port_h) break;

#if LOG_PORTS
			UINT8 old_port_h = m_port_h;
			UINT8 changed = data ^ old_port_h;

		printf("[%08X] ", m_maincpu->m_shifted_pc);
			if(changed & CUTOFF_TEST) printf("[H] CUTOFF_TEST: %s\n", data & CUTOFF_TEST ? "HIGH" : "LOW");
			if(changed & CUTOFF_RESET) printf("[H] CUTOFF_RESET: %s\n", data & CUTOFF_RESET ? "HIGH" : "LOW");
			if(changed & EX1_PWR_CHECK) printf("[H] EX1_PWR_CHECK: %s\n", data & EX1_PWR_CHECK ? "HIGH" : "LOW");
			if(changed & EX1_HEAT) printf("[H] EX1_HEAT: %s\n", data & EX1_HEAT ? "HIGH" : "LOW");
			if(changed & EX1_FAN) printf("[H] EX1_FAN: %s\n", data & EX1_FAN ? "HIGH" : "LOW");
			if(changed & SD_WP) printf("[H] SD_WP: %s\n", data & SD_WP ? "HIGH" : "LOW");
			if(changed & SD_CD) printf("[H] SD_CD: %s\n", data & SD_CD ? "HIGH" : "LOW");
#endif

		m_port_h = data;
		break;
	}
		case AVR8_IO_PORTJ:
	{
		if (data == m_port_j) break;

#if LOG_PORTS
			UINT8 old_port_j = m_port_j;
			UINT8 changed = data ^ old_port_j;

		printf("[%08X] ", m_maincpu->m_shifted_pc);

			if(changed & BUTTON_CENTER) printf("[J] BUTTON_CENTER: %s\n", data & BUTTON_CENTER ? "HIGH" : "LOW");
			if(changed & BUTTON_RIGHT) printf("[J] BUTTON_RIGHT: %s\n", data & BUTTON_RIGHT ? "HIGH" : "LOW");
			if(changed & BUTTON_LEFT) printf("[J] BUTTON_LEFT: %s\n", data & BUTTON_LEFT ? "HIGH" : "LOW");
			if(changed & BUTTON_DOWN) printf("[J] BUTTON_DOWN: %s\n", data & BUTTON_DOWN ? "HIGH" : "LOW");
			if(changed & BUTTON_UP) printf("[J] BUTTON_UP: %s\n", data & BUTTON_UP ? "HIGH" : "LOW");
			if(changed & POTS_SCL) printf("[J] POTS_SCL: %s\n", data & POTS_SCL ? "HIGH" : "LOW");
			if(changed & B_AXIS_POT) printf("[J] B_AXIS_POT: %s\n", data & B_AXIS_POT ? "HIGH" : "LOW");
#endif

		m_port_j = data;
		break;
	}
		case AVR8_IO_PORTK:
	{
		if (data == m_port_k) break;

#if LOG_PORTS
			UINT8 old_port_k = m_port_k;
			UINT8 changed = data ^ old_port_k;

		printf("[%08X] ", m_maincpu->m_shifted_pc);

			if(changed & Z_AXIS_DIR) printf("[K] Z_AXIS_DIR: %s\n", data & Z_AXIS_DIR ? "HIGH" : "LOW");
			if(changed & Z_AXIS_STEP) printf("[K] Z_AXIS_STEP: %s\n", data & Z_AXIS_STEP ? "HIGH" : "LOW");
			if(changed & Z_AXIS_EN) printf("[K] Z_AXIS_EN: %s\n", data & Z_AXIS_EN ? "HIGH" : "LOW");
			if(changed & Z_AXIS_POT) printf("[K] Z_AXIS_POT: %s\n", data & Z_AXIS_POT ? "HIGH" : "LOW");
			if(changed & EX7_1280) printf("[K] EX7_1280: %s\n", data & EX7_1280 ? "HIGH" : "LOW");
			if(changed & EX6_1280) printf("[K] EX6_1280: %s\n", data & EX6_1280 ? "HIGH" : "LOW");
			if(changed & EX5_1280) printf("[K] EX5_1280: %s\n", data & EX5_1280 ? "HIGH" : "LOW");
			if(changed & HBP_THERM) printf("[K] HBP_THERM: %s\n", data & HBP_THERM ? "HIGH" : "LOW");
#endif

		m_port_k = data;
		break;
	}
		case AVR8_IO_PORTL:
	{
		if (data == m_port_l) break;

#if LOG_PORTS
			UINT8 old_port_l = m_port_l;
			UINT8 changed = data ^ old_port_l;

		printf("[%08X] ", m_maincpu->m_shifted_pc);

			if(changed & X_MIN) printf("[L] X_MIN: %s\n", data & X_MIN ? "HIGH" : "LOW");
			if(changed & X_MAX) printf("[L] X_MAX: %s\n", data & X_MAX ? "HIGH" : "LOW");
			if(changed & Y_MIN) printf("[L] Y_MIN: %s\n", data & Y_MIN ? "HIGH" : "LOW");
			if(changed & Y_MAX) printf("[L] Y_MAX: %s\n", data & Y_MAX ? "HIGH" : "LOW");
			if(changed & HBP) printf("[L] HBP: %s\n", data & HBP ? "HIGH" : "LOW");
			if(changed & EXTRA_FET) printf("[L] EXTRA_FET: %s\n", data & EXTRA_FET ? "HIGH" : "LOW");
			if(changed & Z_MIN) printf("[L] Z_MIN: %s\n", data & Z_MIN ? "HIGH" : "LOW");
			if(changed & Z_MAX) printf("[L] Z_MAX: %s\n", data & Z_MAX ? "HIGH" : "LOW");
#endif

		m_port_l = data;
		break;
	}
	}
}

/****************************************************\
* Address maps                                       *
\****************************************************/

static ADDRESS_MAP_START( replicator_prg_map, AS_PROGRAM, 8, replicator_state )
	AM_RANGE(0x0000, 0x1FFFF) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( replicator_data_map, AS_DATA, 8, replicator_state )
	AM_RANGE(0x0200, 0x21FF) AM_RAM  /* ATMEGA1280 Internal SRAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( replicator_io_map, AS_IO, 8, replicator_state )
	AM_RANGE(AVR8_IO_PORTA, AVR8_IO_PORTL) AM_READWRITE( port_r, port_w )
ADDRESS_MAP_END

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

DRIVER_INIT_MEMBER(replicator_state, replicator)
{
}

void replicator_state::machine_reset()
{
	shift_register_value = 0;
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

PALETTE_INIT_MEMBER(replicator_state, replicator)
{
//These colors were picked with the color picker in Inkscape, based on a photo of the LCD used in the Replicator 1 3d printer:
	palette.set_pen_color(0, rgb_t(0xCA, 0xE7, 0xEB));
	palette.set_pen_color(1, rgb_t(0x78, 0xAB, 0xA8));
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

static GFXDECODE_START( replicator )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, hd44780_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( replicator, replicator_state )

	MCFG_CPU_ADD("maincpu", ATMEGA1280, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(replicator_prg_map)
	MCFG_CPU_DATA_MAP(replicator_data_map)
	MCFG_CPU_IO_MAP(replicator_io_map)

	MCFG_CPU_AVR8_EEPROM("eeprom")
	MCFG_CPU_AVR8_LFUSE(0xFF)
	MCFG_CPU_AVR8_HFUSE(0xDA)
	MCFG_CPU_AVR8_EFUSE(0xF4)
	MCFG_CPU_AVR8_LOCK(0x0F)

	/*TODO: Add an ATMEGA8U2 for USB-Serial communications */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(120, 18*2) //4x20 chars
	MCFG_SCREEN_VISIBLE_AREA(0, 120-1, 0, 18*2-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(replicator_state, replicator)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", replicator)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(4, 20)

	/* sound hardware */
	/* A piezo is connected to the PORT G bit 5 (OC0B pin driven by Timer/Counter #4) */
	MCFG_SPEAKER_STANDARD_MONO("buzzer")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(0, "buzzer", 1.00)

MACHINE_CONFIG_END

ROM_START( replica1 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v750")

	/* Version 5.1 release:
	   - Initial firmware release
	*/
	ROM_SYSTEM_BIOS( 0, "v51", "V 5.1" )
	ROMX_LOAD("mighty-mb40-v5.1.bin", 0x0000, 0x10b90, CRC(20d65cd1) SHA1(da18c3eb5a29a6bc1eecd92eaae6063fe29d0305), ROM_BIOS(1))

	/* Version 5.2 release:
	   - Nozzle Tolerance added to EEPROM
	   - Updated onboard menus
	   - X,Y calibration tool added
	*/
	ROM_SYSTEM_BIOS( 1, "v52", "V 5.2" )
	ROMX_LOAD("mighty-mb40-v5.2.bin", 0x0000, 0x126c4, CRC(555e47cf) SHA1(9d24a3dbeddce16669bb4d29c3366220ddf15d2a), ROM_BIOS(2))

	/* Version 5.5 release:
	   - Acceleration added to motor motion
	   - Digipot updates
	*/
	ROM_SYSTEM_BIOS( 2, "v55", "V 5.5" )
	ROMX_LOAD("mighty-mb40-v5.5.bin", 0x0000, 0x1a420, CRC(9327d7e4) SHA1(d734ba2bda12f50ec3ac0035ab11591909d9edde), ROM_BIOS(3))

	/* Version 6.2.0 release:
	   - Bug fix release to firmware 6.0
	   - Addresses wavy print issue above 1cm
	   - Left extruder prints with makerware.
	*/
	ROM_SYSTEM_BIOS( 3, "v620", "V 6.2.0" )
	ROMX_LOAD("mighty_one_v6.2.0.bin", 0x0000, 0x1cf54, CRC(00df6f48) SHA1(db05afc2e1ebc104fb04753634a911187e396556), ROM_BIOS(4))

	/* Version 7.0.0 release:
	   - Major upgrade to Stepper Motor Smoothness (via Sailfish team)
	   - X3G format introduced
	   - Heaters default to leaving 'preheat' on more of the time
	*/
	ROM_SYSTEM_BIOS( 4, "v700", "V 7.0.0" )
	ROMX_LOAD("mighty_one_v7.0.0.bin", 0x0000, 0x1cb52, CRC(aa2a5fcf) SHA1(934e642b0b2d007689249680bad03c9255ae016a), ROM_BIOS(5))

	/* Version 7.2.0 release:
	   - Removes support for S3G files
	   - X3G is the recognized format
	   - Minor bug fixes
	*/
	ROM_SYSTEM_BIOS( 5, "v720", "V 7.2.0" )
	ROMX_LOAD("mighty_one_v7.2.0.bin", 0x0000, 0x1cb80, CRC(5e546706) SHA1(ed4aaf7522d5a5beea7eb69bf2c85d7a89f8f188), ROM_BIOS(6))

	/* Version 7.3.0 release:
	   - Pause at Z Height
	   - Elapsed time displays during prints
	   - Minor bug fixes
	*/
	ROM_SYSTEM_BIOS( 6, "v730", "V 7.3.0" )
	ROMX_LOAD("mighty_one_v7.3.0.bin", 0x0000, 0x1d738, CRC(71811ff5) SHA1(6728ea600ab3ff4b589adca90b0d700d9b70bd18), ROM_BIOS(7))

	/* Version 7.4.0 (bugfix) release:
	   - Fixes issues with Z Pause and elapsed print time
	*/
	ROM_SYSTEM_BIOS( 7, "v740", "V 7.4.0" )
	ROMX_LOAD("mighty_one_v7.4.0.bin", 0x0000, 0x1b9e2, CRC(97b05a27) SHA1(76ca2c9c1db2e006e501c3177a8a1aa693dda0f9), ROM_BIOS(8))

	/* Version 7.5.0 (bugfix) release:
	   - Fixes issue with Heat Hold
	*/
	ROM_SYSTEM_BIOS( 8, "v750", "V 7.5.0" )
	ROMX_LOAD("mighty_one_v7.5.0.bin", 0x0000, 0x1b9c4, CRC(169d6709) SHA1(62b5aacd1bc46969042aea7a50531ec467a4ff1f), ROM_BIOS(9))

	/* Sailfish firmware image - Metam??quina experimental build v7.5.0 */
	ROM_SYSTEM_BIOS( 9, "v750mm", "V 7.5.0 - Metam??quina" )
	ROMX_LOAD("mighty_one_v7.5.0.mm.bin", 0x0000, 0x1ef9a, CRC(0d36d9e7) SHA1(a53899775b4c4eea87b6903758ebb75f06710a69), ROM_BIOS(10))


	/*Arduino MEGA bootloader */
	ROM_LOAD( "atmegaboot_168_atmega1280.bin", 0x1f000, 0x0f16, CRC(c041f8db) SHA1(d995ebf360a264cccacec65f6dc0c2257a3a9224) )

	/* on-die 4kbyte eeprom */
	ROM_REGION( 0x1000, "eeprom", ROMREGION_ERASEFF )
ROM_END

/*   YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY          FULLNAME */
COMP(2012, replica1,    0,        0,        replicator,    replicator, replicator_state,    replicator,    "Makerbot", "Replicator 1 desktop 3d printer", MACHINE_NOT_WORKING)
