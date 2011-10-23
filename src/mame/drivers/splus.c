/**********************************************************************************

    S-PLUS (S+)

    Driver by Jim Stolis.

    --- Technical Notes ---

    Name:    S+
    Company: IGT - International Gaming Technology
    Year:    1994

    Hardware:

    CPU =  INTEL 83c02       ; I8052 compatible
    SND =  AY-3-8912         ; AY8910 compatible

    History:

***********************************************************************************/
#include "emu.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"

#include "splus.lh"

#define DEBUG_OUTPUT 0

class splus_state : public driver_device
{
public:
	splus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cmosl_ram(*this, "cmosl"), m_cmosh_ram(*this, "cmosh")
	{
		m_sda_dir = 0;
		m_coin_state = 0;
		m_last_cycles = 0;

		m_bank10 = 0x00;
		m_bank20 = 0x00;
		m_bank30 = 0x00;
		m_bank40 = 0x00;

		m_p1_reels = 0x00;
		m_p1_unknown = 0x00;
	}

	// Pointers to External RAM
	required_shared_ptr<UINT8> m_cmosl_ram;
	required_shared_ptr<UINT8> m_cmosh_ram;

	// Program and Reel Data
	UINT8 *m_program_ram;
	UINT8 *m_reel_ram;

	// IO Ports
	UINT8 *m_io_port;

	// EEPROM States
	int m_sda_dir;

	// Coin-In States
	UINT8 m_coin_state;
	UINT64 m_last_cycles;
	UINT64 m_last_coin_out;
	UINT8 m_coin_out_state;

	UINT8 m_bank10;
	UINT8 m_bank20;
	UINT8 m_bank30;
	UINT8 m_bank40;

	UINT8 m_p1_reels;
	UINT8 m_p1_unknown;

	INT16 m_stepper_pos[5];
	UINT8 m_stop_pos[5];

};

/* Static Variables */
#define MAX_STEPPER			200		// 1.8 Degree Stepper Motor = 200 full-steps per revolution, but 400 when in half-step mode
#define STEPPER_DIVISOR		9.09 //18.18    // To allow for 22 stop positions

static const UINT8 optics[200] = {
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

#define MASTER_CLOCK		XTAL_20MHz
#define CPU_CLOCK			((MASTER_CLOCK)/2)		/* divided by 2 - 7474 */
#define SOUND_CLOCK			((MASTER_CLOCK)/12)

/* Static Variables */
#define EEPROM_NVRAM_SIZE   0x200 // 4k Bit

/* EEPROM is a X2404P 4K-bit Serial I2C Bus */
static const i2cmem_interface i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, 8, EEPROM_NVRAM_SIZE
};

/*****************
* Write Handlers *
******************/

static WRITE8_HANDLER( splus_io_w )
{
	splus_state *state = space->machine().driver_data<splus_state>();

    // P1.0 = Reel 1 Controller
    // P1.1 = Reel 2 Controller
    // P1.2 = Reel 3 Controller
    // P1.3 = Reel 4 Controller
    // P1.4 = Reel 5 Controller
    // P1.5 = 7-seg display, door
    // P1.6 = 7-seg display, prog
    // P1.7 = Unknown
    int x = 0;

    // Process Port 1
    if (offset == 1 && ((data & 0x1f) != 0x00)) {

        // Unknown Bit 7
        state->m_p1_unknown = (~data & 0x80);

        // Stepper Motor Engaged
        if (((state->m_bank40 >> 0) & 1) == 0x01) {
            // Reel Controllers Only
            state->m_p1_reels = (data & 0x1f);

            // Loop through Reel Controllers
            for (x = 0; x < 5; x++) {
                // Test Reel Controller
                if (((state->m_p1_reels >> x) & 1) == 0x01) {
                    // Forward Direction
                    if (((state->m_bank10 >> 5) & 1) == 0x01) {
                        state->m_stepper_pos[x]++;
                        if (state->m_stepper_pos[x] == MAX_STEPPER)
                            state->m_stepper_pos[x] = 0;
                    } else {
                        state->m_stepper_pos[x]--;
                        if (state->m_stepper_pos[x] < 0)
                            state->m_stepper_pos[x] = MAX_STEPPER - 1;
                    }
					state->m_stop_pos[x] = (int)(state->m_stepper_pos[x] / STEPPER_DIVISOR);
				}
            }
        }
#if DEBUG_OUTPUT
        if ((data & 0x1f) == 0x01)
			mame_printf_info("Steppers %02X-%02X-%02X-%02X-%02X Motor=%02X Dir=%02X reels=%02X unk=%02X\n", state->m_stop_pos[0],state->m_stop_pos[1],state->m_stop_pos[2],state->m_stop_pos[3],state->m_stop_pos[4],((state->m_bank40 >> 0) & 1),((state->m_bank10 >> 5) & 1),(data & 0x1f), state->m_p1_unknown);
#endif
	}

	state->m_io_port[offset] = data;
}

static WRITE8_HANDLER( splus_load_pulse_w )
{
//  splus_state *state = space->machine().driver_data<splus_state>();

//  UINT8 out = 0;
//    out = ((~state->m_io_port[1] & 0xf0)>>4); // Output Bank
}

static WRITE8_HANDLER( splus_serial_w )
{
	splus_state *state = space->machine().driver_data<splus_state>();

    UINT8 out = 0;
    out = ((~state->m_io_port[1] & 0xe0)>>5); // Output Bank

	switch (out)
	{
		case 0x00: // Bank 10
            if (((state->m_bank10 >> 0) & 1) != ((data >> 0) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Coin Drop Meter =%02X\n",(data >> 0) & 1);
#endif
            }
            if (((state->m_bank10 >> 1) & 1) != ((data >> 1) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Coin Out Meter =%02X\n",(data >> 1) & 1);
#endif
            }
            if (((state->m_bank10 >> 2) & 1) != ((data >> 2) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Coin In Meter =%02X\n",(data >> 2) & 1);
#endif
            }
            if (((state->m_bank10 >> 3) & 1) != ((data >> 3) & 1)) {
                //mame_printf_info("B Switch for SDS =%02X\n",(data >> 3) & 1);
            }
            if (((state->m_bank10 >> 4) & 1) != ((data >> 4) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Hopper Drive 2 =%02X\n",(data >> 4) & 1);
#endif
            }
            if (((state->m_bank10 >> 5) & 1) != ((data >> 5) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Stepper Motor Direction =%02X\n",(data >> 5) & 1);
#endif
            }
            if (((state->m_bank10 >> 6) & 1) != ((data >> 6) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Mechanical Bell =%02X\n",(data >> 6) & 1);
#endif
            }
            if (((state->m_bank10 >> 7) & 1) != ((data >> 7) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Cancelled Credits Meter =%02X\n",(data >> 7) & 1);
#endif
            }
            state->m_bank10 = data;

	        output_set_value("s_bnk10",(data >> 0) & 1); // Coin Drop Meter
	        output_set_value("s_bnk11",(data >> 1) & 1); // Coin Out Meter
	        output_set_value("s_bnk12",(data >> 2) & 1); // Coin In Meter
	        output_set_value("s_bnk13",(data >> 3) & 1); // B Switch for SDS
	        output_set_value("s_bnk14",(data >> 4) & 1); // Hopper Drive 2
	        output_set_value("s_bnk15",(data >> 5) & 1); // Stepper Motor Direction
	        output_set_value("s_bnk16",(data >> 6) & 1); // Mechanical Bell
	        output_set_value("s_bnk17",(data >> 7) & 1); // Cancelled Credits Meter
			break;
		case 0x01: // Bank 20
            if (((state->m_bank20 >> 5) & 1) != ((data >> 5) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Games Played Meter =%02X\n",(data >> 5) & 1);
#endif
            }
            if (((state->m_bank20 >> 6) & 1) != ((data >> 6) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Bill Acceptor Enable =%02X\n",(data >> 6) & 1);
#endif
            }
            if (((state->m_bank20 >> 7) & 1) != ((data >> 7) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Jackpots Meter =%02X\n",(data >> 7) & 1);
#endif
            }
            state->m_bank20 = data;

	        output_set_value("s_bnk20",(data >> 0) & 1); // Payline Lamp 3
	        output_set_value("s_bnk21",(data >> 1) & 1); // Payline Lamp 4
	        output_set_value("s_bnk22",(data >> 2) & 1); // Payline Lamp 5
	        output_set_value("s_bnk23",(data >> 3) & 1); // Payline Lamp 6
	        output_set_value("s_bnk24",(data >> 4) & 1); // Door Optics Transmitter
	        output_set_value("s_bnk25",(data >> 5) & 1); // Games Played Meter
	        output_set_value("s_bnk26",(data >> 6) & 1); // Bill Acceptor Enable
	        output_set_value("s_bnk27",(data >> 7) & 1); // Jackpots Meter
			break;
		case 0x02: // Bank 30
            if (((state->m_bank30 >> 2) & 1) != ((data >> 2) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Handle Release =%02X\n",(data >> 2) & 1);
#endif
            }
            if (((state->m_bank30 >> 3) & 1) != ((data >> 3) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Diverter =%02X\n",(data >> 3) & 1);
#endif
            }
            if (((state->m_bank30 >> 4) & 1) != ((data >> 4) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Coin Lockout =%02X\n",(data >> 4) & 1);
#endif
            }
            if (((state->m_bank30 >> 5) & 1) != ((data >> 5) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Hopper Drive 1 =%02X\n",(data >> 5) & 1);
#endif
            }
            state->m_bank30 = data;

	        output_set_value("s_bnk30",(data >> 0) & 1); // Change Candle Lamp Bottom
	        output_set_value("s_bnk31",(data >> 1) & 1); // Change Candle Lamp Top
	        output_set_value("s_bnk32",(data >> 2) & 1); // Handle Release
	        output_set_value("s_bnk33",(data >> 3) & 1); // Diverter
	        output_set_value("s_bnk34",(data >> 4) & 1); // Coin Lockout
	        output_set_value("s_bnk35",(data >> 5) & 1); // Hopper Drive 1
	        output_set_value("s_bnk36",(data >> 6) & 1); // Payline Lamp 1
	        output_set_value("s_bnk37",(data >> 7) & 1); // Payline Lamp 2
            break;
		case 0x04: // Bank 40
            if (((state->m_bank40 >> 0) & 1) != ((data >> 0) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Stepper Motor Power Supply =%02X\n",(data >> 0) & 1);
#endif
            }
            if (((state->m_bank40 >> 3) & 1) != ((data >> 3) & 1)) {
#if DEBUG_OUTPUT
                mame_printf_info("Jackpot/Hand Pay Lamp =%02X\n",(data >> 3) & 1);
#endif
            }
            state->m_bank40 = data;

	        output_set_value("s_bnk40",(data >> 0) & 1); // Stepper Motor Power Supply
	        output_set_value("s_bnk41",(data >> 1) & 1); // Insert Coin Lamp
	        output_set_value("s_bnk42",(data >> 2) & 1); // Coin Accepted Lamp
	        output_set_value("s_bnk43",(data >> 3) & 1); // Jackpot/Hand Pay Lamp
	        output_set_value("s_bnk44",(data >> 4) & 1); // Play Max Credits Lamp
	        output_set_value("s_bnk45",(data >> 5) & 1); // Bet One Credit Lamp
	        output_set_value("s_bnk46",(data >> 6) & 1); // Cashout Credit Lamp
	        output_set_value("s_bnk47",(data >> 7) & 1); // Spin Button Lamp
			break;
	}
}

static WRITE8_HANDLER( splus_7seg_w )
{
	splus_state *state = space->machine().driver_data<splus_state>();

    static const UINT8 ls48_map[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

    UINT8 seg;
    UINT8 val;

    seg = ((~data & 0xf0)>>4); // Segment Number
    val = (~data & 0x0f); // Digit Value

    // Need to add ~state->m_io_port[1]-1 to seg value
    if (seg < 0x0a && (state->m_io_port[1] & 0xe0) == 0xe0)
        output_set_digit_value(seg, ls48_map[val]);
}

static WRITE8_HANDLER( splus_duart_w )
{
	// Used for Slot Accounting System Communication
}

static WRITE8_DEVICE_HANDLER(i2c_nvram_w)
{
	splus_state *state = device->machine().driver_data<splus_state>();
	i2cmem_scl_write(device,BIT(data, 2));
	state->m_sda_dir = BIT(data, 1);
	i2cmem_sda_write(device,BIT(data, 0));
}

/****************
* Read Handlers *
****************/

static READ8_HANDLER( splus_serial_r )
{
	splus_state *state = space->machine().driver_data<splus_state>();

    UINT8 coin_out = 0x00;
    UINT8 coin_optics = 0x00;
    UINT8 door_optics = 0x00;
    UINT32 curr_cycles = space->machine().firstcpu->total_cycles();

    UINT8 in = 0x00;
    UINT8 val = 0x00;
    in = ((~state->m_io_port[1] & 0xe0)>>5); // Input Bank

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
	        if ((input_port_read_safe(space->machine(),"SENSOR",0x00) & 0x01) == 0x01 && state->m_coin_state == 0) {
		        state->m_coin_state = 1; // Start Coin Cycle
		        state->m_last_cycles = space->machine().firstcpu->total_cycles();
#if DEBUG_OUTPUT
                mame_printf_info("coin=%02X\n", state->m_coin_state);
#endif
	        } else {
		        /* Process Next Coin Optic State */
		        if (curr_cycles - state->m_last_cycles > 10000 && state->m_coin_state != 0) {
			        state->m_coin_state++;
			        if (state->m_coin_state > 5)
				        state->m_coin_state = 0;
			        state->m_last_cycles = space->machine().firstcpu->total_cycles();
#if DEBUG_OUTPUT
                    mame_printf_info("coin=%02X\n", state->m_coin_state);
#endif
		        }
	        }

            // Set Coin State
	        switch (state->m_coin_state)
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
            if ((input_port_read_safe(space->machine(),"I10",0x08) & 0x08) == 0x08)
                door_optics = 0x08;
            else
                door_optics = (((state->m_bank20 >> 4) & 1) << 3); // Use Door Optics Transmitter

            // Test if Hopper 1 and Hopper 2 Motors On
            if (((state->m_bank10 >> 4) & 1) || ((state->m_bank30 >> 5) & 1)) {
                if (state->m_coin_out_state == 0)
                    state->m_coin_out_state = 3;
            } else {
                state->m_coin_out_state = 0;
            }

            // Process Coin Out
	        if (curr_cycles - state->m_last_coin_out > 700000 && state->m_coin_out_state != 0) {
		        if (state->m_coin_out_state != 2) {
                    state->m_coin_out_state = 2; // Coin-Out Off
                } else {
                    state->m_coin_out_state = 3; // Coin-Out On
                }

		        state->m_last_coin_out = space->machine().firstcpu->total_cycles();
	        }

            // Set Coin Out State
            switch (state->m_coin_out_state)
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
            val = val | (input_port_read_safe(space->machine(),"I10",0x40) & 0x40); // Handle/Spin Button
            val = val | (input_port_read_safe(space->machine(),"I10",0x80) & 0x80); // Jackpot Reset Key
			break;
		case 0x02: // Bank 20
            val = val | (input_port_read_safe(space->machine(),"I20",0x01) & 0x01); // Bet One Credit
            val = val | (input_port_read_safe(space->machine(),"I20",0x02) & 0x02); // Play Max Credits
            val = val | (input_port_read_safe(space->machine(),"I20",0x04) & 0x04); // Cash Out
            val = val | (input_port_read_safe(space->machine(),"I20",0x08) & 0x08); // Change Request
            val = val | 0x00; // Reel Mechanism
            val = val | (input_port_read_safe(space->machine(),"I20",0x20) & 0x20); // Self Test Button
            val = val | 0x40; // Card Cage
            val = val | 0x80; // Bill Acceptor
			break;
		case 0x04: // Bank 30
            // Reserved
            val = val | (input_port_read_safe(space->machine(),"I30",0x02) & 0x02); // Drop Door
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

static READ8_HANDLER( splus_m_reel_ram_r )
{
	splus_state *state = space->machine().driver_data<splus_state>();
	return state->m_reel_ram[offset];
}

static READ8_HANDLER( splus_io_r )
{
	splus_state *state = space->machine().driver_data<splus_state>();

    if (offset == 3)
        return state->m_io_port[offset] & 0xf3; // Ignore Int0 and Int1, or machine will loop forever waiting
    else
    	return state->m_io_port[offset];
}

static READ8_HANDLER( splus_duart_r )
{
	// Used for Slot Accounting System Communication
	return 0x00;
}

static READ8_HANDLER( splus_watchdog_r )
{
	return 0x00; // Watchdog
}

static READ8_HANDLER( splus_registers_r )
{
	return 0xff; // Reset Registers in Real Time Clock
}

static READ8_DEVICE_HANDLER( splus_reel_optics_r )
{
	splus_state *state = device->machine().driver_data<splus_state>();

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
    UINT8 reel_optics = 0x00;
	UINT8 sda = 0;

    // Return Reel Positions
    reel_optics = (optics[199-(state->m_stepper_pos[4])] & 0x10) | (optics[199-(state->m_stepper_pos[3])] & 0x08) | (optics[199-(state->m_stepper_pos[2])] & 0x04) | (optics[199-(state->m_stepper_pos[1])] & 0x02) | (optics[199-(state->m_stepper_pos[0])] & 0x01);

	if(!state->m_sda_dir)
	{
		sda = i2cmem_sda_read(device);
	}

	reel_optics = reel_optics | 0x40 | (sda<<7);

    return reel_optics;
}

/**************
* Driver Init *
***************/

static DRIVER_INIT( splus )
{
	splus_state *state = machine.driver_data<splus_state>();

	UINT8 *reel_data = machine.region( "reeldata" )->base();

    // Load Reel Data
    memcpy(state->m_reel_ram, &reel_data[0x0000], 0x2000);
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( splus_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_BASE_MEMBER(splus_state, m_program_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( splus_iomap, AS_IO, 8 )
	// Serial I/O
    AM_RANGE(0x0000, 0x0000) AM_READ(splus_serial_r) AM_WRITE(splus_serial_w)

    // Battery-backed RAM (Lower 4K) 0x1500-0x16ff eeprom staging area
	AM_RANGE(0x1000, 0x1fff) AM_RAM AM_SHARE("cmosl")

    // Watchdog, 7-segment Display
    AM_RANGE(0x2000, 0x2000) AM_READWRITE(splus_watchdog_r, splus_7seg_w)

    // DUART
    AM_RANGE(0x3000, 0x300f) AM_READWRITE(splus_duart_r, splus_duart_w)

	// Dip Switches, Sound
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("SW1") AM_DEVWRITE("aysnd", ay8910_address_w)
    AM_RANGE(0x4001, 0x4001) AM_DEVWRITE("aysnd", ay8910_data_w)

    // Reel Optics, EEPROM
    AM_RANGE(0x5000, 0x5000) AM_DEVREAD("i2cmem", splus_reel_optics_r) AM_DEVWRITE("i2cmem", i2c_nvram_w)

	// Reset Registers in Realtime Clock, Serial I/O Load Pulse
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(splus_registers_r, splus_load_pulse_w)

    // Battery-backed RAM (Upper 4K)
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_SHARE("cmosh")

    // SSxxxx Reel Chip
    AM_RANGE(0x8000, 0x9fff) AM_READ(splus_m_reel_ram_r) AM_BASE_MEMBER(splus_state, m_reel_ram)

	// Ports start here
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READ(splus_io_r) AM_WRITE(splus_io_w) AM_BASE_MEMBER(splus_state, m_io_port)
ADDRESS_MAP_END

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

static MACHINE_CONFIG_START( splus, splus_state )	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I80C32, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(splus_map)
	MCFG_CPU_IO_MAP(splus_iomap)

	// Fill NVRAM
	MCFG_NVRAM_ADD_0FILL("cmosl")
	MCFG_NVRAM_ADD_0FILL("cmosh")

	// video hardware (ALL FAKE, NO VIDEO)
    MCFG_PALETTE_LENGTH(16*16)
    MCFG_SCREEN_ADD("scrn", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE((52+1)*8, (31+1)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 25*8-1)

	MCFG_I2CMEM_ADD("i2cmem", i2cmem_interface)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8912, SOUND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

/*************************
*        Rom Load        *
*************************/

ROM_START( spss4240 ) /* Coral Reef (SS4240) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp1271.u52",   0x00000, 0x10000, CRC(dc164599) SHA1(7114652a733b26cd711dbe4d65dde065ba73619f) )

    ROM_REGION( 0x02000, "reeldata", 0 )
    ROM_LOAD( "ss4240.u53",   0x00000, 0x02000, CRC(c5715b9b) SHA1(8b0ca15b520a5c8e1ebec13e3a1dc304fb40aea0) )
ROM_END

/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME        PARENT  MACHINE  INPUT   INIT     ROT    COMPANY                                  FULLNAME                       FLAGS             LAYOUT  */
GAMEL( 1994, spss4240,   0,      splus,   splus,  splus,   ROT0,  "IGT - International Gaming Technology", "S-Plus (SS4240) Coral Reef",  GAME_NOT_WORKING, layout_splus )
