/**********************************************************************************

	needs modernizing, see "#if 0 //" fixme comments

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
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "splus.lh"
#include "video/awpvid.h"		//Fruit Machines Only


class splus_state : public driver_device
{
public:
	splus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{

		m_sda_dir = 0;
		m_coin_state = 0;
	}

/* Pointers to External RAM */
UINT8 *m_cmosl_ram;
UINT8 *m_cmosh_ram;
UINT8 *m_reel_ram;
UINT8 *m_program_ram;

/* IO Ports */
UINT8 *m_io_port;

/* EEPROM States */
int m_sda_dir;
UINT8 m_coin_state ;
UINT32 m_last_cycles;
};


/* Static Variables */
#define CMOS_NVRAM_SIZE     0x1000
#define EEPROM_NVRAM_SIZE   0x200 // 4k Bit


/*****************
* NVRAM Handlers *
******************/

static NVRAM_HANDLER( splus )
{
	splus_state *state = machine.driver_data<splus_state>();

	if (read_or_write)
	{	// writing
		file->write(state->m_cmosl_ram,CMOS_NVRAM_SIZE);
		file->write(state->m_cmosh_ram,CMOS_NVRAM_SIZE);
	}
	else
	{
		if (file)
		{
			file->read(state->m_cmosl_ram, CMOS_NVRAM_SIZE);
			file->read(state->m_cmosh_ram, CMOS_NVRAM_SIZE);
		}
		else
		{
			memset(state->m_cmosl_ram, 0, CMOS_NVRAM_SIZE);
			memset(state->m_cmosh_ram, 0, CMOS_NVRAM_SIZE);
		}
	}

#if 0 //fixme
    NVRAM_HANDLER_CALL(i2cmem_0);
#endif
}

/*****************
* Write Handlers *
******************/

static WRITE8_HANDLER( splus_io_w )
{
	splus_state *state = space->machine().driver_data<splus_state>();

  	state->m_io_port[offset] = data;
}

static WRITE8_HANDLER( splus_load_pulse_w )
{
//	splus_state *state = space->machine().driver_data<splus_state>();

//	UINT8 out = 0;
//    out = ((~state->m_io_port[1] & 0xf0)>>4); // Output Bank
}

static WRITE8_HANDLER( splus_serial_w )
{
	splus_state *state = space->machine().driver_data<splus_state>();

	UINT8 out;
    out = ((~state->m_io_port[1] & 0xf0)>>4); // Output Bank

	switch (out)
	{
		case 0x01: // Bank 10
	        output_set_value("s_bnk10",(data >> 0) & 1); // Coin Drop Meter
	        output_set_value("s_bnk11",(data >> 1) & 1); // Coin Out Meter
	        output_set_value("s_bnk12",(data >> 2) & 1); // Coin In Meter
	        output_set_value("s_bnk13",(data >> 3) & 1); // B Switch for SDS
	        output_set_value("s_bnk14",(data >> 4) & 1); // Hopper Drive 2
	        output_set_value("s_bnk15",(data >> 5) & 1); // Stepper Motor Direction
	        output_set_value("s_bnk16",(data >> 6) & 1); // Mechanical Bell
	        output_set_value("s_bnk17",(data >> 7) & 1); // Cancelled Credits Meter
			break;
        case 0x02: // Unknown
            break;
		case 0x03: // Bank 20
	        output_set_value("s_bnk20",(data >> 0) & 1); // Payline Lamp 3
	        output_set_value("s_bnk21",(data >> 1) & 1); // Payline Lamp 4
	        output_set_value("s_bnk22",(data >> 2) & 1); // Payline Lamp 5
	        output_set_value("s_bnk23",(data >> 3) & 1); // Payline Lamp 6
	        output_set_value("s_bnk24",(data >> 4) & 1); // Door Optics Transmitter
	        output_set_value("s_bnk25",(data >> 5) & 1); // Games Played Meter
	        output_set_value("s_bnk26",(data >> 6) & 1); // Bill Acceptor Enable
	        output_set_value("s_bnk27",(data >> 7) & 1); // Jackpots Meter
			break;
		case 0x05: // Bank 30
	        output_set_value("s_bnk30",(data >> 0) & 1); // Reserved
	        output_set_value("s_bnk31",(data >> 1) & 1); // Change Candle Lamp
	        output_set_value("s_bnk32",(data >> 2) & 1); // Handle Release
	        output_set_value("s_bnk33",(data >> 3) & 1); // Diverter
	        output_set_value("s_bnk34",(data >> 4) & 1); // Coin Lockout
	        output_set_value("s_bnk35",(data >> 5) & 1); // Hopper Drive 1
	        output_set_value("s_bnk36",(data >> 6) & 1); // Payline Lamp 1
	        output_set_value("s_bnk37",(data >> 7) & 1); // Payline Lamp 2
            break;
		case 0x09: // Bank 40
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

static WRITE8_HANDLER(i2c_nvram_w)
{
#if 0// fixme
	splus_state *state = space->machine().driver_data<splus_state>();

	i2cmem_write(0, I2CMEM_SCL, BIT(data, 2));
	state->m_sda_dir = BIT(data, 1);
	i2cmem_write(0, I2CMEM_SDA, BIT(data, 0));
#endif
}

/****************
* Read Handlers *
****************/

/* External RAM Callback for I8052 */
#if 0 // fixme
static READ32_HANDLER( splus_external_ram_iaddr )
{
	splus_state *state = space->machine().driver_data<splus_state>();
	if (mem_mask == 0xff) {
		return (state->m_io_port[2] << 8) | offset;
	} else {
		return offset;
	}
}
#endif

static READ8_HANDLER( splus_serial_r )
{
#if 0 // fixme
	splus_state *state = space->machine().driver_data<splus_state>();

    UINT8 coin_optics = 0x00;
    UINT32 curr_cycles = cpu_get_total_cycles(&space->device());

    UINT8 in = 0x00;
    UINT8 val = 0x00;
    in = ((~state->m_io_port[1] & 0xf0)>>4); // Input Bank

	switch (in)
	{
        case 0x02: // Unknown
            break;
		case 0x03: // Bank 10
	        if ((input_port_read_safe(machine,"SENSOR",0x00) & 0x01) == 0x01 && state->m_coin_state == 0) {
		        state->m_coin_state = 1; // Start Coin Cycle
		        state->m_last_cycles = cpu_get_total_cycles(&space->device());
	        } else {
		        /* Process Next Coin Optic State */
		        if (curr_cycles - m_last_cycles > 600000 && state->m_coin_state != 0) {
			        state->m_coin_state++;
			        if (state->m_coin_state > 5)
				        state->m_coin_state = 0;
			        m_last_cycles = cpu_get_total_cycles(&space->device());
		        }
	        }

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

            // Coin In A
            // Coin In B
            // Coin In C
            val = val | coin_optics;
            val = val | (input_port_read_safe(machine,"I10",0x08) & 0x08); // Door Optics Receiver
            val = val | 0x00; // Hopper Coin Out
            val = val | 0x00; // Hopper Full
            val = val | (input_port_read_safe(machine,"I10",0x40) & 0x40); // Handle/Spin Button
            val = val | (input_port_read_safe(machine,"I10",0x80) & 0x80); // Jackpot Reset Key
			break;
		case 0x05: // Bank 20
            val = val | (input_port_read_safe(machine,"I20",0x01) & 0x01); // Bet One Credit
            val = val | (input_port_read_safe(machine,"I20",0x02) & 0x02); // Play Max Credits
            val = val | (input_port_read_safe(machine,"I20",0x04) & 0x04); // Cash Out
            val = val | (input_port_read_safe(machine,"I20",0x08) & 0x08); // Change Request
            val = val | 0x00; // Reel Mechanism
            val = val | (input_port_read_safe(machine,"I20",0x20) & 0x20); // Self Test Button
            val = val | 0x40; // Card Cage
            val = val | 0x80; // Bill Acceptor
			break;
		case 0x09: // Bank 30
            // Reserved
            val = val | (input_port_read_safe(machine,"I30",0x02) & 0x02); // Drop Door
            // Jackpot to Credit Key
            // Reserved
            // Reserved
            // Reserved
            // Reserved
            // Reserved            
            break;
		case 0x01: // Bank 40
            // Reel #1 - 0=Low State, 1=High State
            // Reel #2 - The state of Reel 1-5 inputs depends upon where each reel has stopped
            // Reel #3
            // Reel #4
            // Reel #5
            // Unknown
            // Unknown
            // Unknown
            val = 0x07;
			break;
	}
	return val;
#endif
	return 0;
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

static READ8_HANDLER( splus_reel_optics_r )
{
	splus_state *state = space->machine().driver_data<splus_state>();

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
	UINT8 reel_optics = 0x1f;
	UINT8 sda = 0;

	if(!state->m_sda_dir)
	{
#if 0 // fixme
		sda = i2cmem_read(0, I2CMEM_SDA);
#endif
	}

	reel_optics = reel_optics | (sda<<7);

    return reel_optics;
}

/**************
* Driver Init *
***************/

static DRIVER_INIT( splus )
{
	splus_state *state = machine.driver_data<splus_state>();

	UINT8 *reel_data = machine.region( "user1" )->base();

    /* Load Reel Data */
    memcpy(state->m_reel_ram, &reel_data[0], 0x2000);

#if 0 // fixme
    /* External RAM callback */
	i8051_set_eram_iaddr_callback(splus_external_ram_iaddr);

    /* EEPROM is a X2404P 4K-bit Serial I2C Bus */
	i2cmem_init(0, I2CMEM_SLAVE_ADDRESS, 8, EEPROM_NVRAM_SIZE, NULL);
#endif
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( splus_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_BASE_MEMBER(splus_state, m_program_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( splus_datamap, AS_IO, 8 )
	// Serial I/O
    AM_RANGE(0x0000, 0x0000) AM_RAM AM_READ(splus_serial_r) AM_WRITE(splus_serial_w)

    // Battery-backed RAM (Lower 4K) 0x1500-0x16ff eeprom staging area
    AM_RANGE(0x1000, 0x1fff) AM_RAM AM_RAMBANK("b1") AM_BASE_MEMBER(splus_state, m_cmosl_ram)

    // Watchdog, 7-segment Display
    AM_RANGE(0x2000, 0x2000) AM_RAM AM_READWRITE(splus_watchdog_r, splus_7seg_w)

    // DUART
    AM_RANGE(0x3000, 0x300f) AM_RAM AM_READWRITE(splus_duart_r, splus_duart_w)

	// Dip Switches, Sound
	AM_RANGE(0x4000, 0x4000) AM_RAM AM_READ_PORT("SW1") AM_DEVWRITE("aysnd", ay8910_address_w)
    AM_RANGE(0x4001, 0x4001) AM_RAM AM_DEVWRITE("aysnd", ay8910_data_w)

    // Reel Optics, EEPROM
    AM_RANGE(0x5000, 0x5000) AM_RAM AM_READ(splus_reel_optics_r) AM_WRITE(i2c_nvram_w)

	// Reset Registers in Realtime Clock, Serial I/O Load Pulse
	AM_RANGE(0x6000, 0x6000) AM_RAM AM_READWRITE(splus_registers_r, splus_load_pulse_w)

    // Battery-backed RAM (Upper 4K)
    AM_RANGE(0x7000, 0x7fff) AM_RAM AM_RAMBANK("b2") AM_BASE_MEMBER(splus_state, m_cmosh_ram)

    // SSxxxx Reel Chip
    AM_RANGE(0x8000, 0x9fff) AM_RAM AM_READ(splus_m_reel_ram_r) AM_BASE_MEMBER(splus_state, m_reel_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( splus_iomap, AS_IO, 8 )
    ADDRESS_MAP_GLOBAL_MASK(0xff)

	// I/O Ports
	AM_RANGE(0x00, 0x03) AM_READ(splus_io_r) AM_WRITE(splus_io_w) AM_BASE_MEMBER(splus_state, m_io_port)
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
	MCFG_CPU_ADD("maincpu", I8052, 10000000*2)
	MCFG_CPU_PROGRAM_MAP(splus_map)
	MCFG_CPU_DATA_MAP(splus_datamap)
	MCFG_CPU_IO_MAP(splus_iomap)
    MCFG_CPU_VBLANK_INT("scrn", irq0_line_hold)
	
	MCFG_NVRAM_HANDLER(splus)

	// video hardware (ALL FAKE, NO VIDEO)
    MCFG_PALETTE_LENGTH(16*16)
    MCFG_SCREEN_ADD("scrn", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE((52+1)*8, (31+1)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 25*8-1)

	// sound hardware
	MCFG_SOUND_ADD("aysnd", AY8912, 10000000/8)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

/*************************
*        Rom Load        *
*************************/

ROM_START( spss4240 ) /* Coral Reef (SS4240) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp1271.u52",   0x00000, 0x10000, CRC(dc164599) SHA1(7114652a733b26cd711dbe4d65dde065ba73619f) )

    ROM_REGION( 0x02000, "user1", 0 )
    ROM_LOAD( "ss4240.u53",   0x00000, 0x02000, CRC(c5715b9b) SHA1(8b0ca15b520a5c8e1ebec13e3a1dc304fb40aea0) )
ROM_END

/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME        PARENT  MACHINE  INPUT   INIT     ROT    COMPANY                                  FULLNAME                       FLAGS             LAYOUT  */
GAMEL( 1994, spss4240,   0,      splus,   splus,  splus,   ROT0,  "IGT - International Gaming Technology", "S-Plus (SS4240) Coral Reef",  GAME_NOT_WORKING, layout_splus )
