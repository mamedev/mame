// license:BSD-3-Clause
// copyright-holders:Jim Stolis
/**********************************************************************************

    S-PLUS (S+)

    Driver by Jim Stolis.

    --- Technical Notes ---

    Name:    S+
    Company: IGT - International Game Technology
    Year:    1994

    Hardware:

    CPU =  INTEL 83c02       ; I8052 compatible
    SND =  AY-3-8912         ; AY8910 compatible

    History:

***********************************************************************************/
#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "splus.lh"


namespace {

#define DEBUG_OUTPUT 0

class splus_state : public driver_device
{
public:
	splus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cmosl_ram(*this, "cmosl")
		, m_cmosh_ram(*this, "cmosh")
		, m_program_ram(*this, "program_ram")
		, m_reel_ram(*this, "reel_ram")
		, m_i10(*this, "I10")
		, m_i20(*this, "I20")
		, m_i30(*this, "I30")
		, m_sensor(*this, "SENSOR")
		, m_maincpu(*this, "maincpu")
		, m_i2cmem(*this, "i2cmem")
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "s_bnk%u%u", 0U, 0U)
	{
		m_sda_dir = 0;
		m_coin_state = 0;
		m_last_cycles = 0;

		m_bank10 = 0x00;
		m_bank20 = 0x00;
		m_bank30 = 0x00;
		m_bank40 = 0x00;

		m_io_port1 = 0x00;
		m_p1_reels = 0x00;
		m_p1_unknown = 0x00;
	}

	void splus(machine_config &config);

	void init_splus();

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}

	void splus_p1_w(uint8_t data);
	void splus_load_pulse_w(uint8_t data);
	void splus_serial_w(uint8_t data);
	void splus_7seg_w(uint8_t data);
	void splus_duart_w(uint8_t data);
	uint8_t splus_serial_r();
	uint8_t splus_m_reel_ram_r(offs_t offset);
	uint8_t splus_p3_r();
	uint8_t splus_duart_r();
	uint8_t splus_watchdog_r();
	uint8_t splus_registers_r();
	void i2c_nvram_w(uint8_t data);
	uint8_t splus_reel_optics_r();

	void splus_iomap(address_map &map) ATTR_COLD;
	void splus_map(address_map &map) ATTR_COLD;

	// EEPROM States
	int m_sda_dir;

	// Coin-In States
	uint8_t m_coin_state;
	uint64_t m_last_cycles;
	uint64_t m_last_coin_out;
	uint8_t m_coin_out_state;

	uint8_t m_bank10;
	uint8_t m_bank20;
	uint8_t m_bank30;
	uint8_t m_bank40;

	uint8_t m_io_port1;
	uint8_t m_p1_reels;
	uint8_t m_p1_unknown;

	int16_t m_stepper_pos[5];
	uint8_t m_stop_pos[5];

	virtual void machine_start() override
	{
		m_digits.resolve();
		m_leds.resolve();
	}

	// Pointers to External RAM
	required_shared_ptr<uint8_t> m_cmosl_ram;
	required_shared_ptr<uint8_t> m_cmosh_ram;

	// Program and Reel Data
	optional_shared_ptr<uint8_t> m_program_ram;
	required_shared_ptr<uint8_t> m_reel_ram;

	// IO Ports
	required_ioport m_i10;
	required_ioport m_i20;
	required_ioport m_i30;
	required_ioport m_sensor;
	required_device<i80c32_device> m_maincpu;
	required_device<i2c_x2404p_device> m_i2cmem;
	output_finder<9> m_digits;
	output_finder<5,8> m_leds;
};

/* Static Variables */
#define MAX_STEPPER         200     // 1.8 Degree Stepper Motor = 200 full-steps per revolution, but 400 when in half-step mode
#define STEPPER_DIVISOR     9.09 //18.18    // To allow for 22 stop positions

static const uint8_t optics[200] = {
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 };

#define MASTER_CLOCK        XTAL(20'000'000)
#define CPU_CLOCK           ((MASTER_CLOCK)/2)      /* divided by 2 - 7474 */
#define SOUND_CLOCK         ((MASTER_CLOCK)/12)


/*****************
* Write Handlers *
******************/

void splus_state::splus_p1_w(uint8_t data)
{
	// P1.0 = Reel 1 Controller
	// P1.1 = Reel 2 Controller
	// P1.2 = Reel 3 Controller
	// P1.3 = Reel 4 Controller
	// P1.4 = Reel 5 Controller
	// P1.5 = 7-seg display, door
	// P1.6 = 7-seg display, prog
	// P1.7 = Unknown

	m_io_port1 = data;

	// Process Port 1
	if ((data & 0x1f) != 0x00) {
		// Unknown Bit 7
		m_p1_unknown = (~data & 0x80);

		// Stepper Motor Engaged
		if (BIT(m_bank40, 0))
		{
			// Reel Controllers Only
			m_p1_reels = (data & 0x1f);

			// Loop through Reel Controllers
			for (int x = 0; x < 5; x++)
			{
				// Test Reel Controller
				if (BIT(m_p1_reels, x))
				{
					// Forward Direction
					if (BIT(m_bank10, 5))
					{
						m_stepper_pos[x]++;
						if (m_stepper_pos[x] == MAX_STEPPER)
							m_stepper_pos[x] = 0;
					}
					else
					{
						m_stepper_pos[x]--;
						if (m_stepper_pos[x] < 0)
							m_stepper_pos[x] = MAX_STEPPER - 1;
					}
					m_stop_pos[x] = (int)(m_stepper_pos[x] / STEPPER_DIVISOR);
				}
			}
		}
#if DEBUG_OUTPUT
		if ((data & 0x1f) == 0x01)
			osd_printf_info("Steppers %02X-%02X-%02X-%02X-%02X Motor=%02X Dir=%02X reels=%02X unk=%02X\n", m_stop_pos[0],m_stop_pos[1],m_stop_pos[2],m_stop_pos[3],m_stop_pos[4],BIT(m_bank40, 0),BIT(m_bank10, 5),(data & 0x1f), m_p1_unknown);
#endif
	}
}

void splus_state::splus_load_pulse_w(uint8_t data)
{
//  uint8_t out = 0;
//    out = ((~m_io_port1 & 0xf0)>>4); // Output Bank
}

void splus_state::splus_serial_w(uint8_t data)
{
	uint8_t out = 0;
	out = ((~m_io_port1 & 0xe0)>>5); // Output Bank

	switch (out)
	{
		case 0x00: // Bank 10
#if DEBUG_OUTPUT
			if (BIT(m_bank10, 0) != BIT(data, 0)) {
				osd_printf_info("Coin Drop Meter =%02X\n",BIT(data, 0);
			}
			if (BIT(m_bank10, 1) != BIT(data, 1)) {
				osd_printf_info("Coin Out Meter =%02X\n",BIT(data, 1);
			}
			if (BIT(m_bank10, 2) != BIT(data, 2)) {
				osd_printf_info("Coin In Meter =%02X\n",BIT(data, 2);
			}
			if (BIT(m_bank10, 3) != BIT(data, 3)) {
				//osd_printf_info("B Switch for SDS =%02X\n",BIT(data, 3);
			}
			if (BIT(m_bank10, 4) != BIT(data, 4)) {
				osd_printf_info("Hopper Drive 2 =%02X\n",BIT(data, 4);
			}
			if (BIT(m_bank10, 5) != BIT(data, 5)) {
				osd_printf_info("Stepper Motor Direction =%02X\n",BIT(data, 5);
			}
			if (BIT(m_bank10, 6) != BIT(data, 6)) {
				osd_printf_info("Mechanical Bell =%02X\n",BIT(data, 6);
			}
			if (BIT(m_bank10, 7) != BIT(data, 7)) {
				osd_printf_info("Cancelled Credits Meter =%02X\n",BIT(data, 7);
			}
#endif
			m_bank10 = data;

			m_leds[1][0] = BIT(data, 0); // Coin Drop Meter
			m_leds[1][1] = BIT(data, 1); // Coin Out Meter
			m_leds[1][2] = BIT(data, 2); // Coin In Meter
			m_leds[1][3] = BIT(data, 3); // B Switch for SDS
			m_leds[1][4] = BIT(data, 4); // Hopper Drive 2
			m_leds[1][5] = BIT(data, 5); // Stepper Motor Direction
			m_leds[1][6] = BIT(data, 6); // Mechanical Bell
			m_leds[1][7] = BIT(data, 7); // Cancelled Credits Meter
			break;
		case 0x01: // Bank 20
#if DEBUG_OUTPUT
			if (BIT(m_bank20, 5) != BIT(data, 5)) {
				osd_printf_info("Games Played Meter =%02X\n",BIT(data, 5);
			}
			if (BIT(m_bank20, 6) != BIT(data, 6)) {
				osd_printf_info("Bill Acceptor Enable =%02X\n",BIT(data, 6);
			}
			if (BIT(m_bank20, 7) != BIT(data, 7)) {
				osd_printf_info("Jackpots Meter =%02X\n",BIT(data, 7);
			}
#endif
			m_bank20 = data;

			m_leds[2][0] = BIT(data, 0); // Payline Lamp 3
			m_leds[2][1] = BIT(data, 1); // Payline Lamp 4
			m_leds[2][2] = BIT(data, 2); // Payline Lamp 5
			m_leds[2][3] = BIT(data, 3); // Payline Lamp 6
			m_leds[2][4] = BIT(data, 4); // Door Optics Transmitter
			m_leds[2][5] = BIT(data, 5); // Games Played Meter
			m_leds[2][6] = BIT(data, 6); // Bill Acceptor Enable
			m_leds[2][7] = BIT(data, 7); // Jackpots Meter
			break;
		case 0x02: // Bank 30
#if DEBUG_OUTPUT
			if (BIT(m_bank30, 2) != BIT(data, 2)) {
				osd_printf_info("Handle Release =%02X\n",BIT(data, 2);
			}
			if (BIT(m_bank30, 3) != BIT(data, 3)) {
				osd_printf_info("Diverter =%02X\n",BIT(data, 3);
			}
			if (BIT(m_bank30, 4) != BIT(data, 4)) {
				osd_printf_info("Coin Lockout =%02X\n",BIT(data, 4);
			}
			if (BIT(m_bank30, 5) != BIT(data, 5)) {
				osd_printf_info("Hopper Drive 1 =%02X\n",BIT(data, 5);
			}
#endif
			m_bank30 = data;

			m_leds[3][0] = BIT(data, 0); // Change Candle Lamp Bottom
			m_leds[3][1] = BIT(data, 1); // Change Candle Lamp Top
			m_leds[3][2] = BIT(data, 2); // Handle Release
			m_leds[3][3] = BIT(data, 3); // Diverter
			m_leds[3][4] = BIT(data, 4); // Coin Lockout
			m_leds[3][5] = BIT(data, 5); // Hopper Drive 1
			m_leds[3][6] = BIT(data, 6); // Payline Lamp 1
			m_leds[3][7] = BIT(data, 7); // Payline Lamp 2
			break;
		case 0x04: // Bank 40
#if DEBUG_OUTPUT
			if (BIT(m_bank40, 0) != BIT(data, 0)) {
				osd_printf_info("Stepper Motor Power Supply =%02X\n",BIT(data, 0);
			}
			if (BIT(m_bank40, 3) != BIT(data, 3)) {
				osd_printf_info("Jackpot/Hand Pay Lamp =%02X\n",BIT(data, 3);
			}
#endif
			m_bank40 = data;

			m_leds[4][0] = BIT(data, 0); // Stepper Motor Power Supply
			m_leds[4][1] = BIT(data, 1); // Insert Coin Lamp
			m_leds[4][2] = BIT(data, 2); // Coin Accepted Lamp
			m_leds[4][3] = BIT(data, 3); // Jackpot/Hand Pay Lamp
			m_leds[4][4] = BIT(data, 4); // Play Max Credits Lamp
			m_leds[4][5] = BIT(data, 5); // Bet One Credit Lamp
			m_leds[4][6] = BIT(data, 6); // Cashout Credit Lamp
			m_leds[4][7] = BIT(data, 7); // Spin Button Lamp
			break;
	}
}

void splus_state::splus_7seg_w(uint8_t data)
{
	static const uint8_t ls48_map[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	uint8_t seg;
	uint8_t val;

	seg = ((~data & 0xf0)>>4); // Segment Number
	val = (~data & 0x0f); // Digit Value

	// Need to add ~m_io_port1-1 to seg value
	if (seg < 9 && (m_io_port1 & 0xe0) == 0xe0)
		m_digits[seg] = ls48_map[val];
}

void splus_state::splus_duart_w(uint8_t data)
{
	// Used for Slot Accounting System Communication
}

void splus_state::i2c_nvram_w(uint8_t data)
{
	m_i2cmem->write_scl(BIT(data, 2));
	m_sda_dir = BIT(data, 1);
	m_i2cmem->write_sda(BIT(data, 0));
}

/****************
* Read Handlers *
****************/

uint8_t splus_state::splus_serial_r()
{
	uint8_t coin_out = 0x00;
	uint8_t coin_optics = 0x00;
	uint8_t door_optics = 0x00;
	uint32_t curr_cycles = m_maincpu->total_cycles();

	uint8_t in = 0x00;
	uint8_t val = 0x00;
	in = ((~m_io_port1 & 0xe0)>>5); // Input Bank

	switch (in)
	{
		case 0x00: // Bank 40
			// Reel #1 - 0=Low State, 1=High State
			// Reel #2 - The state of Reel 1-5 inputs depends upon where each reel has stopped
			// Reel #3
			// Reel #4
			// Reel #5
			// Unknown
			// Unknown
			// Unknown
			val = 0xaa;
			break;
		case 0x01: // Bank 10
			// Test for Coin-In
			if ((m_sensor->read() & 0x01) == 0x01 && m_coin_state == 0) {
				m_coin_state = 1; // Start Coin Cycle
				m_last_cycles = m_maincpu->total_cycles();
#if DEBUG_OUTPUT
				osd_printf_info("coin=%02X\n", m_coin_state);
#endif
			} else {
				/* Process Next Coin Optic State */
				if (curr_cycles - m_last_cycles > 10000 && m_coin_state != 0) {
					m_coin_state++;
					if (m_coin_state > 5)
						m_coin_state = 0;
					m_last_cycles = m_maincpu->total_cycles();
#if DEBUG_OUTPUT
					osd_printf_info("coin=%02X\n", m_coin_state);
#endif
				}
			}

			// Set Coin State
			switch (m_coin_state)
			{
				case 0x00: // No Coin
					coin_optics = 0x00;
					break;
				case 0x01: // Optic A
					coin_optics = 0x01;
					break;
				case 0x02: // Optic AB
					coin_optics = 0x03;
					break;
				case 0x03: // Optic ABC
					coin_optics = 0x07;
					break;
				case 0x04: // Optic BC
					coin_optics = 0x06;
					break;
				case 0x05: // Optic C
					coin_optics = 0x04;
					break;
			}

			// Determine Door Optics
			if ((m_i10->read() & 0x08) == 0x08)
				door_optics = 0x08;
			else
				door_optics = BIT(m_bank20, 4) << 3; // Use Door Optics Transmitter

			// Test if Hopper 1 and Hopper 2 Motors On
			if (BIT(m_bank10, 4) || BIT(m_bank30, 5)) {
				if (m_coin_out_state == 0)
					m_coin_out_state = 3;
			} else {
				m_coin_out_state = 0;
			}

			// Process Coin Out
			if (curr_cycles - m_last_coin_out > 700000 && m_coin_out_state != 0) {
				if (m_coin_out_state != 2) {
					m_coin_out_state = 2; // Coin-Out Off
				} else {
					m_coin_out_state = 3; // Coin-Out On
				}

				m_last_coin_out = m_maincpu->total_cycles();
			}

			// Set Coin Out State
			switch (m_coin_out_state)
			{
				case 0x00: // No Coin-Out
					coin_out = 0x00;
					break;
				case 0x01: // First Coin-Out On
					coin_out = 0x10;
					break;
				case 0x02: // Coin-Out Off
					coin_out = 0x00;
					break;
				case 0x03: // Additional Coin-Out On
					coin_out = 0x10;
					break;
			}

			val = val | coin_optics; // Coin In A B C
			val = val | door_optics; // Door Optics Receiver
			val = val | coin_out; // Hopper Coin OutR
			val = val | 0x00; // Hopper Full
			val = val | (m_i10->read() & 0x40); // Handle/Spin Button
			val = val | (m_i10->read() & 0x80); // Jackpot Reset Key
			break;
		case 0x02: // Bank 20
			val = val | (m_i20->read() & 0x01); // Bet One Credit
			val = val | (m_i20->read() & 0x02); // Play Max Credits
			val = val | (m_i20->read() & 0x04); // Cash Out
			val = val | (m_i20->read() & 0x08); // Change Request
			val = val | 0x00; // Reel Mechanism
			val = val | (m_i20->read() & 0x20); // Self Test Button
			val = val | 0x40; // Card Cage
			val = val | 0x80; // Bill Acceptor
			break;
		case 0x04: // Bank 30
			// Reserved
			val = val | (m_i30->read() & 0x02); // Drop Door
			// Jackpot to Credit Key
			// Reserved
			// Reserved
			// Reserved
			// Reserved
			// Reserved
			break;
	}
	return val;
}

uint8_t splus_state::splus_m_reel_ram_r(offs_t offset)
{
	return m_reel_ram[offset];
}

uint8_t splus_state::splus_p3_r()
{
	return 0xf3; // Ignore Int0 and Int1, or machine will loop forever waiting
}

uint8_t splus_state::splus_duart_r()
{
	// Used for Slot Accounting System Communication
	return 0x00;
}

uint8_t splus_state::splus_watchdog_r()
{
	return 0x00; // Watchdog
}

uint8_t splus_state::splus_registers_r()
{
	return 0xff; // Reset Registers in Real Time Clock
}

uint8_t splus_state::splus_reel_optics_r()
{
/*
        Bit 0 = REEL #1
        Bit 1 = REEL #2
        Bit 2 = REEL #3
        Bit 3 = REEL #4
        Bit 4 = REEL #5
        Bit 5 = ???
        Bit 6 = ???
        Bit 7 = I2C EEPROM SDA
*/
	uint8_t reel_optics = 0x00;
	uint8_t sda = 0;

	// Return Reel Positions
	reel_optics = (optics[199-(m_stepper_pos[4])] & 0x10) | (optics[199-(m_stepper_pos[3])] & 0x08) | (optics[199-(m_stepper_pos[2])] & 0x04) | (optics[199-(m_stepper_pos[1])] & 0x02) | (optics[199-(m_stepper_pos[0])] & 0x01);

	if(!m_sda_dir)
	{
		sda = m_i2cmem->read_sda();
	}

	reel_optics = reel_optics | 0x40 | (sda<<7);

	return reel_optics;
}

/**************
* Driver Init *
***************/

void splus_state::init_splus()
{
	uint8_t *reel_data = memregion( "reeldata" )->base();

	// Load Reel Data
	memcpy(m_reel_ram, &reel_data[0x0000], 0x2000);
}


/*************************
* Memory map information *
*************************/

void splus_state::splus_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void splus_state::splus_iomap(address_map &map)
{
	// Serial I/O
	map(0x0000, 0x0000).r(FUNC(splus_state::splus_serial_r)).w(FUNC(splus_state::splus_serial_w));

	// Battery-backed RAM (Lower 4K) 0x1500-0x16ff eeprom staging area
	map(0x1000, 0x1fff).ram().share("cmosl");

	// Watchdog, 7-segment Display
	map(0x2000, 0x2000).rw(FUNC(splus_state::splus_watchdog_r), FUNC(splus_state::splus_7seg_w));

	// DUART
	map(0x3000, 0x300f).rw(FUNC(splus_state::splus_duart_r), FUNC(splus_state::splus_duart_w));

	// Dip Switches, Sound
	map(0x4000, 0x4000).portr("SW1").w("aysnd", FUNC(ay8910_device::address_w));
	map(0x4001, 0x4001).w("aysnd", FUNC(ay8910_device::data_w));

	// Reel Optics, EEPROM
	map(0x5000, 0x5000).r(FUNC(splus_state::splus_reel_optics_r)).w(FUNC(splus_state::i2c_nvram_w));

	// Reset Registers in Realtime Clock, Serial I/O Load Pulse
	map(0x6000, 0x6000).rw(FUNC(splus_state::splus_registers_r), FUNC(splus_state::splus_load_pulse_w));

	// Battery-backed RAM (Upper 4K)
	map(0x7000, 0x7fff).ram().share("cmosh");

	// SSxxxx Reel Chip
	map(0x8000, 0x9fff).r(FUNC(splus_state::splus_m_reel_ram_r)).share("reel_ram");
}

/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( splus )
	PORT_START("I10")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("Door") PORT_CODE(KEYCODE_O) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("Spin") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)

	PORT_START("I20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)

	PORT_START("I30")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("Drop Door") PORT_CODE(KEYCODE_D) PORT_TOGGLE

	PORT_START("SENSOR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Coin In") PORT_IMPULSE(1)

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Hopper Limit 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Limit 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Sound Generator" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Speed/Bills to Hopper" ) // Either Game Speed or Bills to Hopper depending on game
	PORT_DIPSETTING(    0x08, "Normal/Set in Self Test" )
	PORT_DIPSETTING(    0x00, "Fast/Auto Exchange" )
	PORT_DIPNAME( 0x10, 0x10, "Progressives" )
	PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "Allow" )
	PORT_DIPNAME( 0x20, 0x20, "High/Low Progressives" )
	PORT_DIPSETTING(    0x20, "Single Level Alternating" )
	PORT_DIPSETTING(    0x00, "High/Low" )
	PORT_DIPNAME( 0x40, 0x40, "Double Progressives" )
	PORT_DIPSETTING(    0x40, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x80, 0x80, "Link Progressives" )
	PORT_DIPSETTING(    0x80, "Standalone" )
	PORT_DIPSETTING(    0x00, "Link" )
INPUT_PORTS_END


/*************************
*     Machine Driver     *
*************************/

void splus_state::splus(machine_config &config) // basic machine hardware
{
	I80C32(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &splus_state::splus_map);
	m_maincpu->set_addrmap(AS_IO, &splus_state::splus_iomap);
	m_maincpu->port_out_cb<1>().set(FUNC(splus_state::splus_p1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(splus_state::splus_p3_r));

	// Fill NVRAM
	NVRAM(config, "cmosl", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "cmosh", nvram_device::DEFAULT_ALL_0);

	// video hardware (ALL FAKE, NO VIDEO)
	PALETTE(config, "palette").set_entries(16*16);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(FUNC(splus_state::screen_update));
	screen.set_size((52+1)*8, (31+1)*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 25*8-1);
	screen.set_palette("palette");

	I2C_X2404P(config, m_i2cmem);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8912(config, "aysnd", SOUND_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.75);
}

/*************************
*        Rom Load        *
*************************/

ROM_START( spset005 ) /* Set Chip, not sure how it works but archive for future use */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "set005.u52",   0x00000, 0x10000, CRC(43b70e2e) SHA1(30d610c988fc7e7e9fd54ff378c4a1fecfe5fffe) ) /* 09/24/93   @ IGT L93-1769 */
	ROM_LOAD( "set017.u52",   0x00000, 0x10000, NO_DUMP ) /* When dumped make a separate romdef */
	ROM_LOAD( "set020.u52",   0x00000, 0x10000, NO_DUMP ) /* When dumped make a separate romdef */
ROM_END

ROM_START( spset015 ) /* Set Chip, not sure how it works but archive for future use */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "set015.u52",   0x00000, 0x10000, CRC(01641946) SHA1(81a0632ef96731907b28963293ab12db2073f4a6) )
ROM_END

ROM_START( spset026 ) /* Set Chip, not sure how it works but archive for future use */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "set026.u52",   0x00000, 0x10000, CRC(01641946) SHA1(81a0632ef96731907b28963293ab12db2073f4a6) ) /* 11/06/96   @ IGT NV */
ROM_END

ROM_START( spss4240 ) /* Coral Reef (SS4240) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp1271.u52",   0x00000, 0x10000, CRC(dc164599) SHA1(7114652a733b26cd711dbe4d65dde065ba73619f) )

	ROM_REGION( 0x02000, "reeldata", 0 )
	ROM_LOAD( "ss4240.u53",   0x00000, 0x02000, CRC(c5715b9b) SHA1(8b0ca15b520a5c8e1ebec13e3a1dc304fb40aea0) )
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//     YEAR  NAME      PARENT  MACHINE  INPUT  CLASS        INIT        ROT    COMPANY                                FULLNAME                       FLAGS                LAYOUT
GAMEL( 1993, spset005, 0,      splus,   splus, splus_state, empty_init, ROT0,  "IGT - International Game Technology", "S-Plus SET005 Set chip",  MACHINE_NOT_WORKING, layout_splus )
GAMEL( 1993, spset015, 0,      splus,   splus, splus_state, empty_init, ROT0,  "IGT - International Game Technology", "S-Plus SET015 Set chip",  MACHINE_NOT_WORKING, layout_splus )
GAMEL( 1996, spset026, 0,      splus,   splus, splus_state, empty_init, ROT0,  "IGT - International Game Technology", "S-Plus SET026 Set chip",  MACHINE_NOT_WORKING, layout_splus )

GAMEL( 1994, spss4240, 0,      splus,   splus, splus_state, init_splus, ROT0,  "IGT - International Game Technology", "S-Plus (SS4240) Coral Reef",  MACHINE_NOT_WORKING, layout_splus )
