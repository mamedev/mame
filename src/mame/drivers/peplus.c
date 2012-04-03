/**********************************************************************************


    PLAYER'S EDGE PLUS (PE+)

    Driver by Jim Stolis.
    Layouts by Stephh.

    Special thanks to smf for I2C EEPROM support.

    --- Technical Notes ---

    Name:    Player's Edge Plus (PP0516) Double Bonus Draw Poker.
    Company: IGT - International Gaming Technology
    Year:    1987

    Hardware:

    CPU =  INTEL 83c02       ; I8052 compatible
    VIDEO = ROCKWELL 6545    ; CRTC6845 compatible
    SND =  AY-3-8912         ; AY8910 compatible

    History:

    This form of video poker machine has the ability to use different game roms.  The operator
    changes the game by placing the rom at U68 on the motherboard.  This driver is currently valid
    for the PP0516 game rom, but should work with all other compatible game roms as cpu, video,
    sound, and inputs is concerned.  Some games can share the same color prom and graphic roms,
    but this is not always the case.  It is best to confirm the game, color and graphic combinations.

    The game code runs in two different modes, game mode and operator mode.  Game mode is what a
    normal player would see when playing.  Operator mode is for the machine operator to adjust
    machine settings and view coin counts.  The upper door must be open in order to enter operator
    mode and so it should be mapped to an input bank if you wish to support it.  The operator
    has two additional inputs (jackpot reset and self-test) to navigate with, along with the
    normal buttons available to the player.

    A normal machine keeps all coin counts and settings in a battery-backed ram, and will
    periodically update an external eeprom for an even more secure backup.  This eeprom
    also holds the current game state in order to recover the player from a full power failure.


Additional notes
================

1) What are "set chips" ?

    They are meant to be used after you have already sucessfully put a new game in your machine.
    Lets say you have 'pepp0516' installed and you go through the setup. In a real machine,
    you may want to add a bill validator. The only way to do that is to un-socket the 'pepp0516'
    chip and put in the 'peset038' chip and then reboot the machine. Then this chip's program
    runs and you set the options and put the 'pepp0516' chip back in.

    The only way to simulate this is to fire up the 'pepp0516' game and set it up. Then exit the
    game and copy the pepp0516.nv file to peset038.nv, and then run the 'peset038' program.
    This is because they have to have the same eeprom and cmos data in memory to work. When you
    are done with the 'peset038' program, you copy the peset038.nv file back over the pepp0516.nv .
    'peset038' is just a utility program with one screen and 3 tested inputs.


2) Initialisation

  - Method 1 :
      * be sure the door is opened (if not, press 'O' by default)
      * "CMOS DATA" will be displayed
      * press the self-test button (default is 'K')
      * be sure the door is opened (if not, press 'O' by default)
      * "EEPROM DATA" will be displayed
      * press the self-test button (default is 'K')
      * be sure the door is closed (if not, press 'O' by default)

  - Method 2 :
      * be sure the door is opened (if not, press 'O' by default)
      * "CMOS DATA" will be displayed
      * press the self-test button (default is 'K') until a "beep" is heard
      * be sure the door is closed (if not, press 'O' by default)
      * press the jackpot reset button (default is 'L')
      * be sure the door is opened (if not, press 'O' by default)
      * "EEPROM DATA" will be displayed
      * press the self-test button (default is 'K')
      * be sure the door is closed (if not, press 'O' by default)


          gamename  method
          --------  ------
          pepp0043     2
          pepp0065     1
          pepp0158     2
          pepp0188     1
          pepp0250     1
          pepp0447     2
          pepp0516     1
          peps0014     1
          peps0022     1
          peps0043     1
          peps0045     1
          peps0308     1
          pebe0014     1
          peke1012     1
          peps0615     2
          peps0716     2
          pex2069p     2
          pexp0019     2
          pexp0112     2
          pexs0006     2
          pexmp006     2
          pexmp017     2
          pexmp024     2


2) Configuration

  - To configure a game :
      * be sure the door is opened (if not, press 'O' by default)
      * press the self-test button (default is 'K')
      * cycle through the screens with the self-test button (default is 'K')
      * close the door (default is 'O') to go back to the game and save the settings

2a) What are "set chips" ?

    They are meant to be used after you have already sucessfully put a new game in your machine.
    Lets say you have 'pepp0516' installed and you go through the setup. In a real machine,
    you may want to add a bill validator. The only way to do that is to un-socket the 'pepp0516'
    chip and put in the 'peset038' chip and then reboot the machine. Then this chip's program
    runs and you set the options and put the 'pepp0516' chip back in.

    The only way to simulate this is to fire up the 'pepp0516' game and set it up. Then exit the
    game and copy the pepp0516.nv file to peset038.nv, and then run the 'peset038' program.
    This is because they have to have the same eeprom and cmos data in memory to work. When you
    are done with the peset038 program, you copy the peset038.nv file back over the pepp0516.nv .
    'peset038' is just a utility program with one screen.

2b) About the "autohold" feature

    Depending on laws which vary from cities/country, this feature can available or not in the
    "operator mode". By default, it isn't available. To have this feature available in the
    "operator mode", a new chip has to be burnt with a bit set and a new checksum (game ID
    doesn't change though). Once the feature is available, it can be decided to turn the
    "autohold" option ON or OFF (default is OFF).
    To avoid having too many clones in MAME, a fake "Enable Autohold Feature" configuration
    option has been added for (poker) games that support it. If you change this option,
    you must leave the game, delete the .nv file, initialise and configure the game again.



Stephh's log (2007.11.28) :
  - split old peplus.c to peplus.c and pe_drvr.c (same as it was done for the NEOGEO)
  - Renamed sets :
      * 'peplus'   -> 'pepp0516' (so we get the game ID as for the other games)
  - added MACHINE_RESET definition (needed for fake "Enable Autohold Feature" configuration)
  - added generic/default layout, inputs and outputs
  - for each kind of game (poker, bjack, keno, slots) :
      * added two layouts (default is the "Bezel Lamps" for players, the other is "Debug Lamps")
      * added one INPUT_PORT definition
  - added fake "Enable Autohold Feature" configuration option for poker games that allow it
    and added a specific INPUT_PORT definition
  - for "set chips" :
      * added one fake layout
      * added one fake INPUT_PORT definition

***********************************************************************************/

#include "emu.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "video/mc6845.h"

#include "peplus.lh"
#include "pe_schip.lh"
#include "pe_poker.lh"
#include "pe_bjack.lh"
#include "pe_keno.lh"
#include "pe_slots.lh"


class peplus_state : public driver_device
{
public:
	peplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_cmos_ram(*this, "cmos") { }

	UINT8 *m_videoram;
	required_shared_ptr<UINT8> m_cmos_ram;
	UINT16 m_autohold_addr;
	tilemap_t *m_bg_tilemap;
	UINT8 m_wingboard;
	UINT8 m_jumper_e16_e17;
	UINT8 *m_program_ram;
	UINT8 *m_s3000_ram;
	UINT8 *m_s5000_ram;
	UINT8 *m_s7000_ram;
	UINT8 *m_sb000_ram;
	UINT8 *m_sd000_ram;
	UINT8 *m_sf000_ram;
	UINT16 m_vid_address;
	UINT8 *m_palette_ram;
	UINT8 *m_palette_ram2;
	UINT8 *m_io_port;
	UINT64 m_last_cycles;
	UINT8 m_coin_state;
	UINT64 m_last_door;
	UINT8 m_door_open;
	UINT64 m_last_coin_out;
	UINT8 m_coin_out_state;
	int m_sda_dir;
	DECLARE_WRITE8_MEMBER(peplus_bgcolor_w);
	DECLARE_WRITE8_MEMBER(peplus_crtc_display_w);
	DECLARE_WRITE8_MEMBER(peplus_io_w);
	DECLARE_WRITE8_MEMBER(peplus_duart_w);
	DECLARE_WRITE8_MEMBER(peplus_cmos_w);
	DECLARE_WRITE8_MEMBER(peplus_s3000_w);
	DECLARE_WRITE8_MEMBER(peplus_s5000_w);
	DECLARE_WRITE8_MEMBER(peplus_s7000_w);
	DECLARE_WRITE8_MEMBER(peplus_sb000_w);
	DECLARE_WRITE8_MEMBER(peplus_sd000_w);
	DECLARE_WRITE8_MEMBER(peplus_sf000_w);
	DECLARE_WRITE8_MEMBER(peplus_output_bank_a_w);
	DECLARE_WRITE8_MEMBER(peplus_output_bank_b_w);
	DECLARE_WRITE8_MEMBER(peplus_output_bank_c_w);
	DECLARE_READ8_MEMBER(peplus_io_r);
	DECLARE_READ8_MEMBER(peplus_duart_r);
	DECLARE_READ8_MEMBER(peplus_cmos_r);
	DECLARE_READ8_MEMBER(peplus_s3000_r);
	DECLARE_READ8_MEMBER(peplus_s5000_r);
	DECLARE_READ8_MEMBER(peplus_s7000_r);
	DECLARE_READ8_MEMBER(peplus_sb000_r);
	DECLARE_READ8_MEMBER(peplus_sd000_r);
	DECLARE_READ8_MEMBER(peplus_sf000_r);
	DECLARE_READ8_MEMBER(peplus_bgcolor_r);
	DECLARE_READ8_MEMBER(peplus_dropdoor_r);
	DECLARE_READ8_MEMBER(peplus_watchdog_r);
};


#define MASTER_CLOCK		XTAL_20MHz
#define CPU_CLOCK			((MASTER_CLOCK)/2)		/* divided by 2 - 7474 */
#define MC6845_CLOCK		((MASTER_CLOCK)/8/3)
#define SOUND_CLOCK			((MASTER_CLOCK)/12)


#define eeprom_NVRAM_SIZE   0x200 // 4k Bit

/* EEPROM is a X2404P 4K-bit Serial I2C Bus */
static const i2cmem_interface i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, 8, eeprom_NVRAM_SIZE
};

/* prototypes */
static WRITE_LINE_DEVICE_HANDLER(crtc_vsync);
static MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);

static const mc6845_interface mc6845_intf =
{
	"screen",				/* screen we are acting on */
	8,						/* number of pixels per video memory address */
	NULL,					/* before pixel update callback */
	NULL,					/* row update callback */
	NULL,					/* after pixel update callback */
	DEVCB_NULL,				/* callback for display state changes */
	DEVCB_NULL,				/* callback for cursor state changes */
	DEVCB_NULL,				/* HSYNC callback */
	DEVCB_LINE(crtc_vsync),	/* VSYNC callback */
	crtc_addr				/* update address callback */
};


/**************
* Memory Copy *
***************/

static void peplus_load_superdata(running_machine &machine, const char *bank_name)
{
	peplus_state *state = machine.driver_data<peplus_state>();
    UINT8 *super_data = machine.region(bank_name)->base();

    /* Distribute Superboard Data */
    memcpy(state->m_s3000_ram, &super_data[0x3000], 0x1000);
    memcpy(state->m_s5000_ram, &super_data[0x5000], 0x1000);
    memcpy(state->m_s7000_ram, &super_data[0x7000], 0x1000);
    memcpy(state->m_sb000_ram, &super_data[0xb000], 0x1000);
    memcpy(state->m_sd000_ram, &super_data[0xd000], 0x1000);
    memcpy(state->m_sf000_ram, &super_data[0xf000], 0x1000);
}


/*****************
* Write Handlers *
******************/

WRITE8_MEMBER(peplus_state::peplus_bgcolor_w)
{
	int i;

	for (i = 0; i < machine().total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (~data >> 0) & 0x01;
		bit1 = (~data >> 1) & 0x01;
		bit2 = (~data >> 2) & 0x01;
		r = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* green component */
		bit0 = (~data >> 3) & 0x01;
		bit1 = (~data >> 4) & 0x01;
		bit2 = (~data >> 5) & 0x01;
		g = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* blue component */
		bit0 = (~data >> 6) & 0x01;
		bit1 = (~data >> 7) & 0x01;
		bit2 = 0;
		b = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		palette_set_color(machine(), (15 + (i*16)), MAKE_RGB(r, g, b));
	}
}


/* ROCKWELL 6545 - Transparent Memory Addressing */

static MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr)
{
	peplus_state *state = device->machine().driver_data<peplus_state>();
	state->m_vid_address = address;
}

static WRITE8_DEVICE_HANDLER( peplus_crtc_mode_w )
{
	/* Reset timing logic */
}

static TIMER_CALLBACK(assert_lp_cb)
{
	downcast<mc6845_device *>((device_t*)ptr)->assert_light_pen_input();
}

static void handle_lightpen( device_t *device )
{
    int x_val = input_port_read_safe(device->machine(), "TOUCH_X",0x00);
    int y_val = input_port_read_safe(device->machine(), "TOUCH_Y",0x00);
    const rectangle &vis_area = device->machine().primary_screen->visible_area();
    int xt, yt;

    xt = x_val * vis_area.width() / 1024 + vis_area.min_x;
    yt = y_val * vis_area.height() / 1024 + vis_area.min_y;

     device->machine().scheduler().timer_set(device->machine().primary_screen->time_until_pos(yt, xt), FUNC(assert_lp_cb), 0, device);
}

static WRITE_LINE_DEVICE_HANDLER(crtc_vsync)
{
	cputag_set_input_line(device->machine(), "maincpu", 0, state ? ASSERT_LINE : CLEAR_LINE);
	handle_lightpen(device);
}

WRITE8_MEMBER(peplus_state::peplus_crtc_display_w)
{
	UINT8 *videoram = m_videoram;
	videoram[m_vid_address] = data;
	m_palette_ram[m_vid_address] = m_io_port[1];
	m_palette_ram2[m_vid_address] = m_io_port[3];

	m_bg_tilemap->mark_tile_dirty(m_vid_address);

	/* An access here triggers a device read !*/
	machine().device<mc6845_device>("crtc")->register_r(space, 0);
}

WRITE8_MEMBER(peplus_state::peplus_io_w)
{
	m_io_port[offset] = data;
}

WRITE8_MEMBER(peplus_state::peplus_duart_w)
{
	// Used for Slot Accounting System Communication
}

WRITE8_MEMBER(peplus_state::peplus_cmos_w)
{
	char bank_name[6];

	/* Test for Wingboard PAL Trigger Condition */
	if (offset == 0x1fff && m_wingboard && data < 5)
	{
		sprintf(bank_name, "user%d", data + 1);
		peplus_load_superdata(machine(), bank_name);
	}

	m_cmos_ram[offset] = data;
}

WRITE8_MEMBER(peplus_state::peplus_s3000_w)
{
	m_s3000_ram[offset] = data;
}

WRITE8_MEMBER(peplus_state::peplus_s5000_w)
{
	m_s5000_ram[offset] = data;
}

WRITE8_MEMBER(peplus_state::peplus_s7000_w)
{
	m_s7000_ram[offset] = data;
}

WRITE8_MEMBER(peplus_state::peplus_sb000_w)
{
	m_sb000_ram[offset] = data;
}

WRITE8_MEMBER(peplus_state::peplus_sd000_w)
{
	m_sd000_ram[offset] = data;
}

WRITE8_MEMBER(peplus_state::peplus_sf000_w)
{
	m_sf000_ram[offset] = data;
}

WRITE8_MEMBER(peplus_state::peplus_output_bank_a_w)
{
	output_set_value("pe_bnka0",(data >> 0) & 1); /* Coin Lockout */
	output_set_value("pe_bnka1",(data >> 1) & 1); /* Diverter */
	output_set_value("pe_bnka2",(data >> 2) & 1); /* Bell */
	output_set_value("pe_bnka3",(data >> 3) & 1); /* N/A */
	output_set_value("pe_bnka4",(data >> 4) & 1); /* Hopper 1 */
	output_set_value("pe_bnka5",(data >> 5) & 1); /* Hopper 2 */
	output_set_value("pe_bnka6",(data >> 6) & 1); /* specific to a kind of machine */
	output_set_value("pe_bnka7",(data >> 7) & 1); /* specific to a kind of machine */

    m_coin_out_state = 0;
    if(((data >> 4) & 1) || ((data >> 5) & 1))
        m_coin_out_state = 3;
}

WRITE8_MEMBER(peplus_state::peplus_output_bank_b_w)
{
	output_set_value("pe_bnkb0",(data >> 0) & 1); /* specific to a kind of machine */
	output_set_value("pe_bnkb1",(data >> 1) & 1); /* Deal Spin Start */
	output_set_value("pe_bnkb2",(data >> 2) & 1); /* Cash Out */
	output_set_value("pe_bnkb3",(data >> 3) & 1); /* specific to a kind of machine */
	output_set_value("pe_bnkb4",(data >> 4) & 1); /* Bet 1 / Bet Max */
	output_set_value("pe_bnkb5",(data >> 5) & 1); /* Change Request */
	output_set_value("pe_bnkb6",(data >> 6) & 1); /* Door Open */
	output_set_value("pe_bnkb7",(data >> 7) & 1); /* specific to a kind of machine */
}

WRITE8_MEMBER(peplus_state::peplus_output_bank_c_w)
{
	output_set_value("pe_bnkc0",(data >> 0) & 1); /* Coin In Meter */
	output_set_value("pe_bnkc1",(data >> 1) & 1); /* Coin Out Meter */
	output_set_value("pe_bnkc2",(data >> 2) & 1); /* Coin Drop Meter */
	output_set_value("pe_bnkc3",(data >> 3) & 1); /* Jackpot Meter */
	output_set_value("pe_bnkc4",(data >> 4) & 1); /* Bill Acceptor Enabled */
	output_set_value("pe_bnkc5",(data >> 5) & 1); /* SDS Out */
	output_set_value("pe_bnkc6",(data >> 6) & 1); /* N/A */
	output_set_value("pe_bnkc7",(data >> 7) & 1); /* Game Meter */
}

static WRITE8_DEVICE_HANDLER(i2c_nvram_w)
{
	peplus_state *state = device->machine().driver_data<peplus_state>();
	i2cmem_scl_write(device,BIT(data, 2));
	state->m_sda_dir = BIT(data, 1);
	i2cmem_sda_write(device,BIT(data, 0));
}


/****************
* Read Handlers *
****************/

READ8_MEMBER(peplus_state::peplus_io_r)
{
    return m_io_port[offset];
}

READ8_MEMBER(peplus_state::peplus_duart_r)
{
	// Used for Slot Accounting System Communication
	return 0x00;
}

READ8_MEMBER(peplus_state::peplus_cmos_r)
{
	return m_cmos_ram[offset];
}

READ8_MEMBER(peplus_state::peplus_s3000_r)
{
	return m_s3000_ram[offset];
}

READ8_MEMBER(peplus_state::peplus_s5000_r)
{
	return m_s5000_ram[offset];
}

READ8_MEMBER(peplus_state::peplus_s7000_r)
{
	return m_s7000_ram[offset];
}

READ8_MEMBER(peplus_state::peplus_sb000_r)
{
	return m_sb000_ram[offset];
}

READ8_MEMBER(peplus_state::peplus_sd000_r)
{
	return m_sd000_ram[offset];
}

READ8_MEMBER(peplus_state::peplus_sf000_r)
{
	return m_sf000_ram[offset];
}

/* Last Color in Every Palette is bgcolor */
READ8_MEMBER(peplus_state::peplus_bgcolor_r)
{
	return palette_get_color(machine(), 15); // Return bgcolor from First Palette
}

READ8_MEMBER(peplus_state::peplus_dropdoor_r)
{
	return 0x00; // Drop Door 0x00=Closed 0x02=Open
}

READ8_MEMBER(peplus_state::peplus_watchdog_r)
{
	return 0x00; // Watchdog
}

static READ8_DEVICE_HANDLER( peplus_input_bank_a_r )
{
	peplus_state *state = device->machine().driver_data<peplus_state>();
/*
        Bit 0 = COIN DETECTOR A
        Bit 1 = COIN DETECTOR B
        Bit 2 = COIN DETECTOR C
        Bit 3 = COIN OUT
        Bit 4 = HOPPER FULL
        Bit 5 = DOOR OPEN
        Bit 6 = LOW BATTERY
        Bit 7 = I2C EEPROM SDA
*/
	UINT8 bank_a = 0x50; // Turn Off Low Battery and Hopper Full Statuses
	UINT8 coin_optics = 0x00;
    UINT8 coin_out = 0x00;
	UINT64 curr_cycles = device->machine().firstcpu->total_cycles();
	UINT16 door_wait = 500;

	UINT8 sda = 0;
	if(!state->m_sda_dir)
	{
		sda = i2cmem_sda_read(device);
	}

	if ((input_port_read_safe(device->machine(), "SENSOR",0x00) & 0x01) == 0x01 && state->m_coin_state == 0) {
		state->m_coin_state = 1; // Start Coin Cycle
		state->m_last_cycles = device->machine().firstcpu->total_cycles();
	} else {
		/* Process Next Coin Optic State */
		if (curr_cycles - state->m_last_cycles > 600000/6 && state->m_coin_state != 0) {
			state->m_coin_state++;
			if (state->m_coin_state > 5)
				state->m_coin_state = 0;
			state->m_last_cycles = device->machine().firstcpu->total_cycles();
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

	if (state->m_wingboard)
		door_wait = 12345;

	if (curr_cycles - state->m_last_door > door_wait) {
		if ((input_port_read_safe(device->machine(), "DOOR",0xff) & 0x01) == 0x01) {
			state->m_door_open = (!state->m_door_open & 0x01);
		} else {
			state->m_door_open = 1;
		}
		state->m_last_door = device->machine().firstcpu->total_cycles();
	}

	if (curr_cycles - state->m_last_coin_out > 600000/12 && state->m_coin_out_state != 0) { // Guessing with 600000
		if (state->m_coin_out_state != 2) {
            state->m_coin_out_state = 2; // Coin-Out Off
        } else {
            state->m_coin_out_state = 3; // Coin-Out On
        }

		state->m_last_coin_out = device->machine().firstcpu->total_cycles();
	}

    switch (state->m_coin_out_state)
    {
        case 0x00: // No Coin-Out
	        coin_out = 0x00;
	        break;
        case 0x01: // First Coin-Out On
	        coin_out = 0x08;
	        break;
        case 0x02: // Coin-Out Off
	        coin_out = 0x00;
	        break;
        case 0x03: // Additional Coin-Out On
	        coin_out = 0x08;
	        break;
    }

	bank_a = (sda<<7) | bank_a | (state->m_door_open<<5) | coin_optics | coin_out;

	return bank_a;
}


/****************************
* Video/Character functions *
****************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	peplus_state *state = machine.driver_data<peplus_state>();
	UINT8 *videoram = state->m_videoram;
	int pr = state->m_palette_ram[tile_index];
	int pr2 = state->m_palette_ram2[tile_index];
	int vr = videoram[tile_index];

	int code = ((pr & 0x0f)*256) | vr;
	int color = (pr>>4) & 0x0f;

	// Access 2nd Half of CGs and CAP
	if (state->m_jumper_e16_e17 && (pr2 & 0x10) == 0x10)
	{
		code += 0x1000;
		color += 0x10;
	}

	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START( peplus )
{
	peplus_state *state = machine.driver_data<peplus_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 40, 25);
	state->m_palette_ram = auto_alloc_array(machine, UINT8, 0x3000);
	memset(state->m_palette_ram, 0, 0x3000);
	state->m_palette_ram2 = auto_alloc_array(machine, UINT8, 0x3000);
	memset(state->m_palette_ram2, 0, 0x3000);
}

static SCREEN_UPDATE_IND16( peplus )
{
	peplus_state *state = screen.machine().driver_data<peplus_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}

static PALETTE_INIT( peplus )
{
/*  prom bits
    7654 3210
    ---- -xxx   red component.
    --xx x---   green component.
    xx-- ----   blue component.
*/
	int i;

	for (i = 0;i < machine.total_colors();i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (~color_prom[i] >> 0) & 0x01;
		bit1 = (~color_prom[i] >> 1) & 0x01;
		bit2 = (~color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* green component */
		bit0 = (~color_prom[i] >> 3) & 0x01;
		bit1 = (~color_prom[i] >> 4) & 0x01;
		bit2 = (~color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* blue component */
		bit0 = (~color_prom[i] >> 6) & 0x01;
		bit1 = (~color_prom[i] >> 7) & 0x01;
		bit2 = 0;
		b = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( peplus )
			GFXDECODE_ENTRY( "gfx1", 0x00000, gfx_8x8x4_planar, 0, 16 )
GFXDECODE_END


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( peplus_map, AS_PROGRAM, 8, peplus_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_BASE(m_program_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( peplus_iomap, AS_IO, 8, peplus_state )
	// Battery-backed RAM (0x1000-0x01fff Extended RAM for Superboards Only)
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(peplus_cmos_r, peplus_cmos_w) AM_SHARE("cmos")

	// CRT Controller
	AM_RANGE(0x2008, 0x2008) AM_DEVWRITE_LEGACY("crtc", peplus_crtc_mode_w)
	AM_RANGE(0x2080, 0x2080) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x2081, 0x2081) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x2083, 0x2083) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(peplus_crtc_display_w)

    // Superboard Data
	AM_RANGE(0x3000, 0x3fff) AM_READWRITE(peplus_s3000_r, peplus_s3000_w) AM_BASE(m_s3000_ram)

	// Sound and Dipswitches
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("SW1")/* likely ay8910 input port, not direct */ AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)

    // Superboard Data
	AM_RANGE(0x5000, 0x5fff) AM_READWRITE(peplus_s5000_r, peplus_s5000_w) AM_BASE(m_s5000_ram)

	// Background Color Latch
	AM_RANGE(0x6000, 0x6000) AM_READ(peplus_bgcolor_r) AM_WRITE(peplus_bgcolor_w)

    // Bogus Location for Video RAM
	AM_RANGE(0x06001, 0x06400) AM_RAM AM_BASE(m_videoram)

    // Superboard Data
	AM_RANGE(0x7000, 0x7fff) AM_READWRITE(peplus_s7000_r, peplus_s7000_w) AM_BASE(m_s7000_ram)

	// Input Bank A, Output Bank C
	AM_RANGE(0x8000, 0x8000) AM_DEVREAD_LEGACY("i2cmem",peplus_input_bank_a_r) AM_WRITE(peplus_output_bank_c_w)

	// Drop Door, I2C EEPROM Writes
	AM_RANGE(0x9000, 0x9000) AM_READ(peplus_dropdoor_r) AM_DEVWRITE_LEGACY("i2cmem",i2c_nvram_w)

	// Input Banks B & C, Output Bank B
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0") AM_WRITE(peplus_output_bank_b_w)

    // Superboard Data
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE(peplus_sb000_r, peplus_sb000_w) AM_BASE(m_sb000_ram)

	// Output Bank A
	AM_RANGE(0xc000, 0xc000) AM_READ(peplus_watchdog_r) AM_WRITE(peplus_output_bank_a_w)

    // Superboard Data
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(peplus_sd000_r, peplus_sd000_w) AM_BASE(m_sd000_ram)

	// DUART
	AM_RANGE(0xe000, 0xe00f) AM_READWRITE(peplus_duart_r, peplus_duart_w)

    // Superboard Data
	AM_RANGE(0xf000, 0xffff) AM_READWRITE(peplus_sf000_r, peplus_sf000_w) AM_BASE(m_sf000_ram)

	/* Ports start here */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READ(peplus_io_r) AM_WRITE(peplus_io_w) AM_BASE(m_io_port)
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static CUSTOM_INPUT( peplus_input_r )
{
	UINT8 inp_ret = 0x00;
	UINT8 inp_read = input_port_read(field.machine(), (const char *)param);

	if (inp_read & 0x01) inp_ret = 0x01;
	if (inp_read & 0x02) inp_ret = 0x02;
	if (inp_read & 0x04) inp_ret = 0x03;
	if (inp_read & 0x08) inp_ret = 0x04;
	if (inp_read & 0x10) inp_ret = 0x05;
	if (inp_read & 0x20) inp_ret = 0x06;
	if (inp_read & 0x40) inp_ret = 0x07;

	return inp_ret;
}

static INPUT_PORTS_START( peplus )
	/* IN0 has to be defined for each kind of game */
	PORT_START("DOOR")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Upper Door") PORT_CODE(KEYCODE_O) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Lower Door") PORT_CODE(KEYCODE_I)

	PORT_START("SENSOR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin In") PORT_IMPULSE(1)

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Line Frequency" )
	PORT_DIPSETTING(    0x01, "60Hz" )
	PORT_DIPSETTING(    0x00, "50Hz" )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_schip )
	PORT_INCLUDE(peplus)
	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_poker )
	PORT_INCLUDE(peplus)

	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bill Acceptor") PORT_CODE(KEYCODE_U)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_bjack )
	PORT_INCLUDE(peplus)

	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Surrender") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Stand") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Insurance") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Double Down") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Split") PORT_CODE(KEYCODE_B)

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("Bill Acceptor") PORT_CODE(KEYCODE_U)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_keno )
	PORT_INCLUDE(peplus)

	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Erase") PORT_CODE(KEYCODE_B)

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("Bill Acceptor") PORT_CODE(KEYCODE_U)

	PORT_START("TOUCH_X")
	PORT_BIT( 0xffff, 0x200, IPT_LIGHTGUN_X ) PORT_MINMAX(0x00, 1024) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)
	PORT_START("TOUCH_Y")
	PORT_BIT( 0xffff, 0x200, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x00, 1024) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Light Pen") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_slots )
	PORT_INCLUDE(peplus)

	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("Bill Acceptor") PORT_CODE(KEYCODE_U)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* same as peplus_poker with additionnal fake option to enable the "Autohold" feature */
static INPUT_PORTS_START( peplus_pokah )
	PORT_INCLUDE(peplus_poker)

	/* If you change this option, you'll have to delete the .nv file next time you launch the game ! */
	PORT_START("AUTOHOLD")
	PORT_CONFNAME( 0x01, 0x00, "Enable Autohold Feature" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Yes ) )
INPUT_PORTS_END


/*************************
*     Machine Reset      *
*************************/

static MACHINE_RESET( peplus )
{
	/* AutoHold Feature Currently Disabled */
#if 0
	peplus_state *state = machine.driver_data<peplus_state>();

	// pepp0158
	state->m_program_ram[0xa19f] = 0x22; // RET - Disable Memory Test
	state->m_program_ram[0xddea] = 0x22; // RET - Disable Program Checksum
	state->m_autohold_addr = 0x5ffe; // AutoHold Address

	// pepp0188
	state->m_program_ram[0x9a8d] = 0x22; // RET - Disable Memory Test
	state->m_program_ram[0xf429] = 0x22; // RET - Disable Program Checksum
	state->m_autohold_addr = 0x742f; // AutoHold Address

	// pepp0516
	state->m_program_ram[0x9a24] = 0x22; // RET - Disable Memory Test
	state->m_program_ram[0xd61d] = 0x22; // RET - Disable Program Checksum
	state->m_autohold_addr = 0x5e7e; // AutoHold Address

	if (state->m_autohold_addr)
		state->m_program_ram[state->m_autohold_addr] = input_port_read_safe(machine, "AUTOHOLD",0x00) & 0x01;
#endif
}


/*************************
*     Machine Driver     *
*************************/

static MACHINE_CONFIG_START( peplus, peplus_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I80C32, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(peplus_map)
	MCFG_CPU_IO_MAP(peplus_iomap)

	MCFG_MACHINE_RESET(peplus)
	MCFG_NVRAM_ADD_0FILL("cmos")

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE((52+1)*8, (31+1)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 25*8-1)
	MCFG_SCREEN_UPDATE_STATIC(peplus)

	MCFG_GFXDECODE(peplus)
	MCFG_PALETTE_LENGTH(16*16*2)

	MCFG_MC6845_ADD("crtc", R6545_1, MC6845_CLOCK, mc6845_intf)
	MCFG_I2CMEM_ADD("i2cmem", i2cmem_interface)

	MCFG_PALETTE_INIT(peplus)
	MCFG_VIDEO_START(peplus)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8912, SOUND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END


/*****************
* Initialisation *
*****************/

/* Normal board */
static void peplus_init(running_machine &machine)
{
	peplus_state *state = machine.driver_data<peplus_state>();
	/* default : no address to patch in program RAM to enable autohold feature */
	state->m_autohold_addr = 0;
}


/*************************
*      Driver Init       *
*************************/

/* Normal board */
static DRIVER_INIT( peplus )
{
	peplus_state *state = machine.driver_data<peplus_state>();
	state->m_wingboard = FALSE;
	state->m_jumper_e16_e17 = FALSE;
	peplus_init(machine);
}

/* Superboard */
static DRIVER_INIT( peplussb )
{
	peplus_state *state = machine.driver_data<peplus_state>();
	state->m_wingboard = FALSE;
	state->m_jumper_e16_e17 = FALSE;
	peplus_load_superdata(machine, "user1");

	peplus_init(machine);
}

/* Superboard with Attached Wingboard */
static DRIVER_INIT( peplussbw )
{
	peplus_state *state = machine.driver_data<peplus_state>();
	state->m_wingboard = TRUE;
	state->m_jumper_e16_e17 = TRUE;
	peplus_load_superdata(machine, "user1");

	peplus_init(machine);
}


/*************************
*        Rom Load        *
*************************/

ROM_START( peset038 ) /* Normal board : Set Chip (Set038) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "set038.u68",   0x00000, 0x10000, CRC(9c4b1d1a) SHA1(8a65cd1d8e2d74c7b66f4dfc73e7afca8458e979) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0200, CRC(8020b65f) SHA1(e280b11315acba88799d8875fb2980bee9d5e687) )
ROM_END

ROM_START( pepp0043 ) /* Normal board : 10's or Better (PP0043) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0043.u68",   0x00000, 0x10000, CRC(04051a88) SHA1(e7a9ec2ab7f6f575245d47ee10a03f39c887d1b3) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",	 0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) )
	ROM_LOAD( "mgo-cg2004.u73",	 0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",	 0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) )
	ROM_LOAD( "mxo-cg2004.u75",	 0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0200, CRC(8020b65f) SHA1(e280b11315acba88799d8875fb2980bee9d5e687) )
ROM_END

ROM_START( pepp0065 ) /* Normal board : Jokers Wild Poker (PP0065) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0065.u68",   0x00000, 0x10000, CRC(76c1a367) SHA1(ea8be9241e9925b5a4206db6875e1572f85fa5fe) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0200, CRC(8020b65f) SHA1(e280b11315acba88799d8875fb2980bee9d5e687) )
ROM_END

ROM_START( pepp0158 ) /* Normal board : 4 of a Kind Bonus Poker (PP0158) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0158.u68",   0x00000, 0x10000, CRC(5976cd19) SHA1(6a461ea9ddf78dffa3cf8b65903ebf3127f23d45) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0200, CRC(8020b65f) SHA1(e280b11315acba88799d8875fb2980bee9d5e687) )
ROM_END

ROM_START( pepp0188 ) /* Normal board : Standard Draw Poker (PP0188) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0188.u68",   0x00000, 0x10000, CRC(cf36a53c) SHA1(99b578538ab24d9ff91971b1f77599272d1dbfc6) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0200, CRC(8020b65f) SHA1(e280b11315acba88799d8875fb2980bee9d5e687) )
ROM_END

ROM_START( pepp0250 ) /* Normal board : Double Down Stud Poker (PP0250) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0250.u68",   0x00000, 0x10000, CRC(4c919598) SHA1(fe73503c6ccb3c5746fb96be58cd5b740c819713) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0200, CRC(8020b65f) SHA1(e280b11315acba88799d8875fb2980bee9d5e687) )
ROM_END

ROM_START( pepp0447 ) /* Normal board : Standard Draw Poker (PP0447) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0447.u68",   0x00000, 0x10000, CRC(0ef0bb6c) SHA1(d0ef7a83417054f05d32d0a93ed0d5d618f4dfb9) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0200, CRC(8020b65f) SHA1(e280b11315acba88799d8875fb2980bee9d5e687) )
ROM_END

ROM_START( pepp0516 ) /* Normal board : Double Bonus Poker (PP0516) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0516.u68",   0x00000, 0x10000, CRC(d9da6e13) SHA1(421678d9cb42daaf5b21074cc3900db914dd26cf) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0200, CRC(8020b65f) SHA1(e280b11315acba88799d8875fb2980bee9d5e687) )
ROM_END

ROM_START( pebe0014 ) /* Normal board : Blackjack (BE0014) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "be0014.u68",   0x00000, 0x10000, CRC(232b32b7) SHA1(a3af9414577642fedc23b4c1911901cd31e9d6e0) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2036.u72",	 0x00000, 0x8000, CRC(0a168d06) SHA1(7ed4fb5c7bcacab077bcec030f0465c6eaf3ce1c) )
	ROM_LOAD( "mgo-cg2036.u73",	 0x08000, 0x8000, CRC(826b4090) SHA1(34390484c0faffe9340fd93d273b9292d09f97fd) )
	ROM_LOAD( "mbo-cg2036.u74",	 0x10000, 0x8000, CRC(46aac851) SHA1(28d84b49c6cebcf2894b5a15d935618f84093caa) )
	ROM_LOAD( "mxo-cg2036.u75",	 0x18000, 0x8000, CRC(60204a56) SHA1(2e3420da9e79ba304ca866d124788f84861380a7) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap707.u50", 0x0000, 0x0200, CRC(5bfeed62) SHA1(df47a2723a70a7c16fbf03b9f614e9b98751a59e) )
ROM_END

ROM_START( peke1012 ) /* Normal board : Keno (KE1012) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ke1012.u68",   0x00000, 0x10000, CRC(470e8c10) SHA1(f8a65a3a73477e9e9d2f582eeefa93b470497dfa) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1267.u72",	 0x00000, 0x8000, CRC(16498b57) SHA1(9c22726299af7204c4be1c6d8afc4c1b512ad918) )
	ROM_LOAD( "mgo-cg1267.u73",	 0x08000, 0x8000, CRC(80847c5a) SHA1(8422cd13a91c3c462af5efcfca8615e7eeaa2e52) )
	ROM_LOAD( "mbo-cg1267.u74",	 0x10000, 0x8000, CRC(ce7af8a7) SHA1(38675122c764b8fa9260246ea99ac0f0750da277) )
	ROM_LOAD( "mxo-cg1267.u75",	 0x18000, 0x8000, CRC(a4394303) SHA1(30a07028de35f74cc4fb776b0505ca743c8d7b5b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap1267.u50", 0x0000, 0x0200, CRC(3dac264f) SHA1(e9c9de42ffd64d4463bee6fa10886a53bc062ff8) )
ROM_END

ROM_START( peps0014 ) /* Normal board : Super Joker Slots (PS0014) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0014.u68",   0x00000, 0x10000, CRC(368c3f58) SHA1(ebefcefbb5386659680719936bff72ad61087343) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0916.u72",	 0x00000, 0x8000, CRC(d97049d9) SHA1(78f7bb33866ca92922a8b83d5f9ac459edd39176) )
	ROM_LOAD( "mgo-cg0916.u73",	 0x08000, 0x8000, CRC(6e075788) SHA1(e8e9d8b7943d62e31d1d58f870bc765cba65c203) )
	ROM_LOAD( "mbo-cg0916.u74",	 0x10000, 0x8000, CRC(a5cdf0f3) SHA1(23b2749fd2cb5b8462ce7c912005779b611f32f9) )
	ROM_LOAD( "mxo-cg0916.u75",	 0x18000, 0x8000, CRC(1f3a2d72) SHA1(8e07324d436980b628e007d30a835757c1f70f6d) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap0916.u50", 0x0000, 0x0200, CRC(c9a4f87c) SHA1(3c7c53fbf7573f07b334e0529bfd7ccf8d5339b5) )
ROM_END

ROM_START( peps0022 ) /* Normal board : Red White & Blue Slots (PS0022) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0022.u68",   0x00000, 0x10000, CRC(d65c0939) SHA1(d91f472a43f77f9df8845e97561540f988e522e3) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",	 0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",	 0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",	 0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",	 0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap0960.u50", 0x0000, 0x0200, CRC(83d67070) SHA1(4c50abbe750dbd4a461084b0bfc51e38df97e421) )
ROM_END

ROM_START( peps0043 ) /* Normal board : Double Diamond Slots (PS0043) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0043.u68",   0x00000, 0x10000, CRC(d612429c) SHA1(95eb4774482a930066456d517fb2e4f67d4df4cb) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1003.u72",	 0x00000, 0x8000, CRC(41ce0395) SHA1(ae90dbae30e4efed33f83ee7038fb2e5171c1945) )
	ROM_LOAD( "mgo-cg1003.u73",	 0x08000, 0x8000, CRC(5a383fa1) SHA1(27b1febbdda7332e8d474fc0cca683f451a07090) )
	ROM_LOAD( "mbo-cg1003.u74",	 0x10000, 0x8000, CRC(5ec00224) SHA1(bb70a4326cd1810b200e193a449061df62085f37) )
	ROM_LOAD( "mxo-cg1003.u75",	 0x18000, 0x8000, CRC(2ffacd52) SHA1(38126ac4998806a1ddd55e6aa1942044240d41d0) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap1003.u50", 0x0000, 0x0200, CRC(1fb7b69f) SHA1(cdb609f39ef1ca0ddf389a599f799c269c7163f9) )
ROM_END

ROM_START( peps0045 ) /* Normal board : Red White & Blue Slots (PS0045) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0045.u68",   0x00000, 0x10000, CRC(de180b84) SHA1(0d592d7d535b0aacbd62c18ac222da770fab7b85) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",	 0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",	 0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",	 0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",	 0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap0960.u50", 0x0000, 0x0200, CRC(83d67070) SHA1(4c50abbe750dbd4a461084b0bfc51e38df97e421) )
ROM_END

ROM_START( peps0308 ) /* Normal board : Double Jackpot Slots (PS0308) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0308.u68",   0x00000, 0x10000, CRC(fe30e081) SHA1(d216cbc6336727caf359e6b178c856ab2659cabd) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0911.u72",	 0x00000, 0x8000, CRC(48491b50) SHA1(9ec6d3ff34a08d40082a1347a46635838fd31afc) )
	ROM_LOAD( "mgo-cg0911.u73",	 0x08000, 0x8000, CRC(c1ff7d97) SHA1(78ab138ae9c7f9b3352f9b1ef5fbc473993bb8c8) )
	ROM_LOAD( "mbo-cg0911.u74",	 0x10000, 0x8000, CRC(202e0f9e) SHA1(51421dfd1b00a9e3b1e938d5bffaa3b7cd4c2b5e) )
	ROM_LOAD( "mxo-cg0911.u75",	 0x18000, 0x8000, CRC(d97740a2) SHA1(d76926d7fbbc24d2384a1079cb97e654600b134b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap0911.u50", 0x0000, 0x0200, CRC(79dc19c0) SHA1(9ebf998b73c3390cbb957b3dd3fec57b3c70a06d) )
ROM_END

ROM_START( peps0615 ) /* Normal board : Chaos Slots (PS0615) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0615.u68",   0x00000, 0x10000, CRC(d27dd6ab) SHA1(b3f065f507191682edbd93b07b72ed87bf6ae9b1) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2246.u72",	 0x00000, 0x8000, CRC(7c08c355) SHA1(2a154b81c6d9671cea55a924bffb7f5461747142) )
	ROM_LOAD( "mgo-cg2246.u73",	 0x08000, 0x8000, CRC(b3c16487) SHA1(c97232fadd086f604eaeb3cd3c2d1c8fe0dcfa70) )
	ROM_LOAD( "mbo-cg2246.u74",	 0x10000, 0x8000, CRC(e61331f5) SHA1(4364edc625d64151cbae40780b54cb1981086647) )
	ROM_LOAD( "mxo-cg2246.u75",	 0x18000, 0x8000, CRC(f0f4a27d) SHA1(3a10ab196aeaa5b50d47b9d3c5b378cfadd6fe96) )

	ROM_REGION( 0x200, "proms", 0 ) // WRONG CAP
    ROM_LOAD( "cap0960.u50", 0x0000, 0x0200, CRC(83d67070) SHA1(4c50abbe750dbd4a461084b0bfc51e38df97e421) )
ROM_END

ROM_START( peps0716 ) /* Normal board : River Gambler Slots (PS0716) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0716.u68",   0x00000, 0x10000, CRC(7615d7b6) SHA1(91fe62eec720a0dc2ebf48835065148f19499d16) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2266.u72",	 0x00000, 0x8000, CRC(590accd8) SHA1(4e1c963c50799eaa49970e25ecf9cb01eb6b09e1) )
	ROM_LOAD( "mgo-cg2266.u73",	 0x08000, 0x8000, CRC(b87ffa05) SHA1(92126b670b9cabeb5e2cc35b6e9c458088b18eea) )
	ROM_LOAD( "mbo-cg2266.u74",	 0x10000, 0x8000, CRC(e3df30e1) SHA1(c7d2ae9a7c7e53bfb6197b635efcb5dc231e4fe0) )
	ROM_LOAD( "mxo-cg2266.u75",	 0x18000, 0x8000, CRC(56271442) SHA1(61ad0756b9f6412516e46ef6625a4c3899104d4e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "cap2266.u50", 0x0000, 0x0200, CRC(ae8b52ac) SHA1(f58d40ee77d7f432dfe5f37954e43cab654c9a4c) )
ROM_END

ROM_START( pex2069p ) /* Superboard : Double Double Bonus Poker (X002069P) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002069p.u66",   0x00000, 0x10000, CRC(875ecfcf) SHA1(80472cb43b36e518216e60a9b4883a81163718a2) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",	 0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) )
	ROM_LOAD( "mgo-cg2185.u78",	 0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",	 0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",	 0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pexp0019 ) /* Superboard : Deuces Wild Poker (XP000019) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000019.u67",   0x00000, 0x10000, CRC(8ac876eb) SHA1(105b4aee2692ccb20795586ccbdf722c59db66cf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002025p.u66",   0x00000, 0x10000, CRC(f3dac423) SHA1(e9394d330deb3b8a1001e57e72a506cd9098f161) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",	 0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) )
	ROM_LOAD( "mgo-cg2185.u78",	 0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",	 0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",	 0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 ) // WRONG CAP
	ROM_LOAD( "capx2234.u43", 0x0000, 0x0200, CRC(519000fa) SHA1(31cd72643ca74a778418f944045e9e03937143d6) )
ROM_END

ROM_START( pexp0112 ) /* Superboard : White Hot Aces Poker (XP000112) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002035p.u66",   0x00000, 0x10000, CRC(dc3f0742) SHA1(d57cf3353b81f41895458019e47203f98645f17a) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",	 0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",	 0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",	 0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",	 0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexs0006 ) /* Superboard : Triple Triple Diamond Slots (XS000006) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xs000006.u67",   0x00000, 0x10000, CRC(4b11ca18) SHA1(f64a1fbd089c01bc35a5484e60b8834a2db4a79f) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000998s.u66",   0x00000, 0x10000, CRC(e29d4346) SHA1(93901ff65c8973e34ac1f0dd68bb4c4973da5621) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2361.u77",	 0x00000, 0x8000, CRC(c0eba866) SHA1(8f217aeb3e8028a5633d95e5612f1b55e601650f) )
	ROM_LOAD( "mgo-cg2361.u78",	 0x08000, 0x8000, CRC(345eaea2) SHA1(18ebb94a323e1cf671201db8b9f85d4f30d8b5ec) )
	ROM_LOAD( "mbo-cg2361.u79",	 0x10000, 0x8000, CRC(fa130af6) SHA1(aca5e52e00bc75a4801fd3f6c564e62ed4251d8e) )
	ROM_LOAD( "mxo-cg2361.u80",	 0x18000, 0x8000, CRC(7de1812c) SHA1(c7e23a10f20fc8b618df21dd33ac456e1d2cfe33) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2361.u43", 0x0000, 0x0200, CRC(93057296) SHA1(534bbf8ee80a22822d577f6685501f4c929987ef) )
ROM_END

ROM_START( pexmp006 ) /* Superboard : Multi-Poker (XMP00006) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00006.u67",   0x00000, 0x10000, CRC(d61f1677) SHA1(2eca1315d6aa310a54de2dfa369e443a07495b76) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00002p.u66",   0x00000, 0x10000, CRC(96cf471c) SHA1(9597bf6a80c392ee22dc4606db610fdaf032377f) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2174.u77",	 0x00000, 0x8000, CRC(bb666733) SHA1(dcaa1980b051a554cb0f443b1183a680edc9ad3f) )
	ROM_LOAD( "mgo-cg2174.u78",	 0x08000, 0x8000, CRC(cc46adb0) SHA1(6065aa5dcb9091ad80e499c7ee6dc629e79c865a) )
	ROM_LOAD( "mbo-cg2174.u79",	 0x10000, 0x8000, CRC(7291a0c8) SHA1(1068f35e6ef5fd88c584922860231840a90fb623) )
	ROM_LOAD( "mxo-cg2174.u80",	 0x18000, 0x8000, CRC(14f9480c) SHA1(59323f9fc5995277aea86d088893b6eb95b4e89b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexmp017 ) /* Superboard : 5-in-1 Wingboard (XMP00017) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00017.u67",   0x00000, 0x10000, CRC(129e6eaa) SHA1(1dd2b83a672a618f338b553a6cbd598b6d4ce672) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) )

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "x000188p.u66",   0x00000, 0x10000, CRC(3eb7580e) SHA1(86f2280542fb8a55767efd391d0fb04a12ed9408) )

	ROM_REGION( 0x10000, "user3", 0 )
	ROM_LOAD( "x000581p.u66",   0x00000, 0x10000, CRC(a4cfecc3) SHA1(b2c805781ba43bda9e208d8c16578dc96b6f58f7) )

	ROM_REGION( 0x10000, "user4", 0 )
	ROM_LOAD( "x000727p.u66",   0x00000, 0x10000, CRC(4828474c) SHA1(9836b76113a71802df30ca15f7c9a5790e6f1c5b) )

	ROM_REGION( 0x10000, "user5", 0 )
	ROM_LOAD( "x002036p.u66",   0x00000, 0x10000, CRC(69207baf) SHA1(fe038b969106ae5cdc8dde1c06497be9c7b5b8bf) )

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2298.u77",	 0x00000, 0x10000, CRC(8c35dc7f) SHA1(90e9566e816287e6248d7cab318dee3ad6fac871) )
	ROM_LOAD( "mgo-cg2298.u78",	 0x10000, 0x10000, CRC(3663174a) SHA1(c203a4a59f6bc1625d47f35426ffc5b4d279251a) )
	ROM_LOAD( "mbo-cg2298.u79",	 0x20000, 0x10000, CRC(9088cdbe) SHA1(dc62951c584463a1e795a774f5752f890d8e3f65) )
	ROM_LOAD( "mxo-cg2298.u80",	 0x30000, 0x10000, CRC(8d3aafc8) SHA1(931bc82398b94c63ed9f6f1bd95723aa801894cc) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2298.u43", 0x0000, 0x0200, CRC(77856036) SHA1(820487c8494965408402ddee6a54511906218e66) )
ROM_END

ROM_START( pexmp024 ) /* Superboard : Multi-Poker (XMP00024) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00024.u67",   0x00000, 0x10000, CRC(f2df8870) SHA1(bc7fa1d79da07093cf3d3508e226a9c490990e04) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00005p.u66",   0x00000, 0x10000, CRC(c832eac7) SHA1(747d57de602b44ae1276fe1009db1b6de0d2c64c) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2240.u77",	 0x00000, 0x8000, CRC(eedef2d4) SHA1(419a90e1f4a840625e6ac7afc2c24d13c908156d) )
	ROM_LOAD( "mgo-cg2240.u78",	 0x08000, 0x8000, CRC(c596b058) SHA1(d53824f869bceeda482e434cba9a77ba8ce2015f) )
	ROM_LOAD( "mbo-cg2240.u79",	 0x10000, 0x8000, CRC(ab1a58ee) SHA1(44963f27d5f5d8f9415d88c12b2d40f0ef55c559) )
	ROM_LOAD( "mxo-cg2240.u80",	 0x18000, 0x8000, CRC(75488ff7) SHA1(a34ae53847b5643b8c4dc182dc59b1fccf22d557) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT         INIT      ROT    COMPANY                                  FULLNAME                                                  FLAGS   LAYOUT */

/* Set chips */
GAMEL(1987, peset038, 0,      peplus,  peplus_schip, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (Set038) Set Chip",                      0,   layout_pe_schip )

/* Normal board : poker */
GAMEL(1987, pepp0043, 0,      peplus,  peplus_poker, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0043) 10's or Better",                0,   layout_pe_poker )
GAMEL(1987, pepp0065, 0,      peplus,  peplus_poker, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0065) Jokers Wild Poker",             0,   layout_pe_poker )
GAMEL(1987, pepp0158, 0,      peplus,  peplus_pokah, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0158) 4 of a Kind Bonus Poker",       0,   layout_pe_poker )
GAMEL(1987, pepp0188, 0,      peplus,  peplus_pokah, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0188) Standard Draw Poker",           0,   layout_pe_poker )
GAMEL(1987, pepp0250, 0,      peplus,  peplus_poker, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0250) Double Down Stud Poker",        0,   layout_pe_poker )
GAMEL(1987, pepp0447, 0,      peplus,  peplus_poker, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0447) Standard Draw Poker",           0,   layout_pe_poker )
GAMEL(1987, pepp0516, 0,      peplus,  peplus_pokah, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0516) Double Bonus Poker",            0,   layout_pe_poker )

/* Normal board : blackjack */
GAMEL(1994, pebe0014, 0,      peplus,  peplus_bjack, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (BE0014) Blackjack",                     0,   layout_pe_bjack )

/* Normal board : keno */
GAMEL(1994, peke1012, 0,      peplus,  peplus_keno,  peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (KE1012) Keno",                          0,   layout_pe_keno )

/* Normal board : slots machine */
GAMEL(1996, peps0014, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0014) Super Joker Slots",             0,   layout_pe_slots )
GAMEL(1996, peps0022, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0022) Red White & Blue Slots",        0,   layout_pe_slots )
GAMEL(1996, peps0043, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0043) Double Diamond Slots",          0,   layout_pe_slots )
GAMEL(1996, peps0045, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0045) Red White & Blue Slots",        0,   layout_pe_slots )
GAMEL(1996, peps0308, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0308) Double Jackpot Slots",          0,   layout_pe_slots )
GAMEL(1996, peps0615, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0615) Chaos Slots",                   0,   layout_pe_slots )
GAMEL(1996, peps0716, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0716) River Gambler Slots",           0,   layout_pe_slots )

/* Superboard : poker */
GAMEL(1995, pex2069p, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (X002069P) Double Double Bonus Poker",   0,   layout_pe_poker )
GAMEL(1995, pexp0019, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XP000019) Deuces Wild Poker",           0,   layout_pe_poker )
GAMEL(1995, pexp0112, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XP000112) White Hot Aces Poker",        0,   layout_pe_poker )

/* Superboard : multi-poker */
GAMEL(1995, pexmp006, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XMP00006) Multi-Poker",                 0,   layout_pe_poker )
GAMEL(1995, pexmp024, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XMP00024) Multi-Poker",                 0,   layout_pe_poker )

/* Superboard : multi-poker (wingboard) */
GAMEL(1995, pexmp017, 0,      peplus,  peplus_poker, peplussbw,ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XMP00017) 5-in-1 Wingboard",            0,   layout_pe_poker )

/* Superboard : slots machine */
GAMEL(1997, pexs0006, 0,      peplus,  peplus_slots, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XS000006) Triple Triple Diamond Slots", 0,   layout_pe_slots )
