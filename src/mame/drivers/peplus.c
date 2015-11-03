// license:BSD-3-Clause
// copyright-holders:Jim Stolis, Brian Troha
/**********************************************************************************


    PLAYER'S EDGE PLUS (PE+)

    Driver by Jim Stolis.
    Layouts by Stephh.

    Special thanks to smf for I2C EEPROM support.

    --- Technical Notes ---

    Hardware:

    CPU =  INTEL 80C32       ; I8052 compatible
    VIDEO = ROCKWELL 6545    ; CRTC6845 compatible
    SND =  AY-3-8912         ; AY8910 compatible

    History:

    This form of video poker machine has the ability to use different game roms.  The operator
    changes the game by placing the rom at U68 on the motherboard.  This driver currently supports
    several PE+ game roms, but should work with all other compatible game roms as far as cpu, video,
    sound, and inputs are concerned.  Some games can share the same color prom and graphic roms,
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

    They are meant to be used after you have already successfully put a new game in your machine.
    Lets say you have 'pepp0516' installed and you go through the setup. In a real machine,
    you may want to add a bill validator. The only way to do that is to un-socket the 'pepp0516'
    chip and put in the 'peset038' chip and then reboot the machine. Then this chip's program
    runs and you set the options and put the 'pepp0516' chip back in.

    The only way to simulate this is to fire up the 'pepp0516' game and set it up. Then exit the
    game and copy the cmos & i2cmem files from your 'pepp0516' directory (in NVRAM) to the peset038
    directory in NVRAM, and then run the 'peset038' program. This is because they have to have the
    same eeprom and cmos data in memory to work. When you are done with the 'peset038' program,
    you copy the cmos & i2cmem files back into the pepp0516 directory and restart the pepp0516 game.
    'peset038' is just a utility program with one screen and 3 tested inputs.


2) Initialization

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

  - Method 3 (earlier 32K French programs ONLY):
      * be sure the door is opened (if not, press 'O' by default)
      * "CODE ERREUR DE VALIDITE CMOS" will be displayed
      * press the self-test button (default is 'K')
      * be sure the door is closed (if not, press 'O' by default)
      * if you see "PORTE OUVERTE" on the bottom of the screen,  the secondary
      *    door is open, you'll need to close it (press 'M' by default)
      * when both doors are closed you should see "PORTE FERMEE" on the
      *    bottom of the screen. Begin normal play


3) Configuration

  - To configure a game :
      * be sure the door is opened (if not, press 'O' by default)
      * press the self-test button (default is 'K')
      * cycle through the screens with the self-test button (default is 'K')
      * close the door (default is 'O') to go back to the game and save the settings

3a) About the "autohold" feature

    Depending on local laws which vary from one jurisdiction to another, this feature may be
    available in the "operator mode". This requires a specific build for said jurisdiction.
    Currently the only dumped sets with the Auto Hold feature enabled are PP0197 & PP0419.


Stephh's log (2007.11.28) :
  - Renamed sets :
      * 'peplus'   -> 'pepp0516' (so we get the game ID as for the other games)
  - added generic/default layout, inputs and outputs
  - for each kind of game (poker, bjack, keno, slots) :
      * added two layouts (default is the "Bezel Lamps" for players, the other is "Debug Lamps")
      * added one INPUT_PORT definition
  - for "set chips" :
      * added one fake layout
      * added one fake INPUT_PORT definition

******************************************************************************************************************

Generally speaking for standard PE+ boards:

    Program roms are 64K and read as 27C512  (Jumper E15 is for 64K, E14 is for 32K)
CG Graphics roms are 32K and read as 27C256
      Color CAP PROM are 256 bytes and read as N82S135N (or compatible, IE: DM74LS471)
      Color CAPX PROM are 512 bytes and read as N82S147N (or compatible)
       Where CAPX & CAP share the same number, the CAPX has the same DATA as the CAP chip in
       the first 256 bytes, then just padded with 256 bytes of 0x00 at the end of the file.

Board type with program type

Standard PE+
 Program Types:
  BEnnnn Blackjack / 21 games
  KEnnnn Keno
  PPnnnn Poker games. Several different types of poker require specific CG graphics + CAP color prom
  IPnnnn International Poker games. Several different types of poker require specific CG graphics + CAP color prom
  PSnnnn Slot games. Each slot game requires specific CG graphics + CAP color prom
  MGnnnn Multi Game programs for the Player's Choice machines that had optional touchscreens and or printers

Super PE+
 Program Types
  XPnnnnnn  Poker Programs. Different options for each set, but all use the same XnnnnnnP data roms
   XnnnnnnP Poker Data. Contains poker game + paytable percentages
             Data roms will not work with every Program rom. Incompatible combos report: Incompatible Data EPROM
             X000055P is a good example, it works with 19 XP000xxx Program roms. Others may be as few as 2.
  XMPnnnnn  Multi-Poker Programs. Different options for each set, but all use the same XMnnnnnP data
             XMP00002 through XMP00006, XMP00020 & XMP00024 Use the XM000xxP Multi-Poker Data
             XMP00014, XMP00017 & XMP00030 Use the WING Board add-on and use the XnnnnnnP Poker Data (Not all are compatible!)
             XMP00013, XMP00022 & XMP00026 Use the WING Board add-on & CG2346 + CAPX2346 for Spanish paytables
             XMP00025 Uses the XM000xxP Multi-Poker Data roms and is for the International markets. Auto Hold always enabled.
   XMnnnnnP Multi-Poker Data. Contains poker games + paytable percentages: Requires specific CG graphics + CAP color prom
  XKnnnnnn  Spot Keno Programs. Different options for each set, but all use the same XnnnnnnK data roms
   XnnnnnnK Spot Keno Data. Uses CG2120 with CAPX1267
  XSnnnnnn  Slot Programs. Different options for each set, but all use the same XnnnnnnS data roms
  XnnnnnnT  Tournament Slot Programs? Different options for each set, but all use the same XnnnnnnS data roms
   XnnnnnnS Slot Data. Each set requires specific CG graphics + CAP color prom

The CG graphics + CAP color proms along with which program sets they belong to is a closely guarded secret by IGT.
 The only public information is from collectors who document and share such information.

NOTE:  Do NOT use the CG+CAP combos listed below as THE definitive absolute reference. There are other combos that
       worked to produce correct card graphics plus paytable information for many sets. So the combos listed below
       may not always be the "official" combo and a better or more correct combo may exist.

NOTE:  International PP0xxx sets support a Tournament mode.  You can toggle back and forth between standard and
       Tournament mode by pressing and holding Jackpot Reset (L key) and pressing Change Request (Y key)

NOTE:  XP000035 supports a Tournament mode.  You can toggle back and forth between standard and Tournament mode by
       pressing and holding Jackpot Reset (L key) and pressing Change Request (Y key)

NOTE:  Some CG graphics sets work with several "standard" game types, but will be included in a single set for
       illustration purposes and to archive the sets.

NOTE:  The Door Open cycling is currently not fully understood. Non Plus programs and the earlier 32K versions don't
       expect the Door Open bit to cycle. Later versions, Superboard & Wingboards require the Door Open cycling but
       at different rates. It's currently not know what if any universal value will work for all sets.

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
	enum
	{
		TIMER_ASSERT_LP
	};

	peplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_i2cmem(*this, "i2cmem"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_cmos_ram(*this, "cmos"),
		m_program_ram(*this, "prograram"),
		m_s3000_ram(*this, "s3000_ram"),
		m_s5000_ram(*this, "s5000_ram"),
		m_videoram(*this, "videoram"),
		m_s7000_ram(*this, "s7000_ram"),
		m_sb000_ram(*this, "sb000_ram"),
		m_sd000_ram(*this, "sd000_ram"),
		m_sf000_ram(*this, "sf000_ram"),
		m_io_port(*this, "io_port")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<r6545_1_device> m_crtc;
	required_device<i2cmem_device> m_i2cmem;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_cmos_ram;
	required_shared_ptr<UINT8> m_program_ram;
	required_shared_ptr<UINT8> m_s3000_ram;
	required_shared_ptr<UINT8> m_s5000_ram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_s7000_ram;
	required_shared_ptr<UINT8> m_sb000_ram;
	required_shared_ptr<UINT8> m_sd000_ram;
	required_shared_ptr<UINT8> m_sf000_ram;
	required_shared_ptr<UINT8> m_io_port;

	tilemap_t *m_bg_tilemap;
	UINT8 m_wingboard;
	UINT8 m_doorcycle;
	UINT16 door_wait;
	UINT8 m_jumper_e16_e17;
	UINT16 m_vid_address;
	UINT8 *m_palette_ram;
	UINT8 *m_palette_ram2;
	UINT64 m_last_cycles;
	UINT8 m_coin_state;
	UINT64 m_last_door;
	UINT8 m_door_open;
	UINT64 m_last_coin_out;
	UINT8 m_coin_out_state;
	int m_sda_dir;
	UINT8 m_bv_state;
	UINT8 m_bv_busy;
	UINT8 m_bv_pulse;
	UINT8 m_bv_denomination;
	UINT8 m_bv_protocol;
	UINT64 m_bv_cycles;
	UINT8 m_bv_last_enable_state;
	UINT8 m_bv_enable_state;
	UINT8 m_bv_enable_count;
	UINT8 m_bv_data_bit;
	UINT8 m_bv_loop_count;
	UINT16 id023_data;

	DECLARE_WRITE8_MEMBER(peplus_bgcolor_w);
	DECLARE_WRITE8_MEMBER(peplus_crtc_display_w);
	DECLARE_WRITE8_MEMBER(peplus_duart_w);
	DECLARE_WRITE8_MEMBER(peplus_cmos_w);
	DECLARE_WRITE8_MEMBER(peplus_output_bank_a_w);
	DECLARE_WRITE8_MEMBER(peplus_output_bank_b_w);
	DECLARE_WRITE8_MEMBER(peplus_output_bank_c_w);
	DECLARE_READ8_MEMBER(peplus_duart_r);
	DECLARE_READ8_MEMBER(peplus_bgcolor_r);
	DECLARE_READ8_MEMBER(peplus_dropdoor_r);
	DECLARE_READ8_MEMBER(peplus_watchdog_r);
	DECLARE_CUSTOM_INPUT_MEMBER(peplus_input_r);
	DECLARE_WRITE8_MEMBER(peplus_crtc_mode_w);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync);
	DECLARE_WRITE8_MEMBER(i2c_nvram_w);
	DECLARE_READ8_MEMBER(peplus_input_bank_a_r);
	DECLARE_READ8_MEMBER(peplus_input0_r);
	DECLARE_DRIVER_INIT(nonplus);
	DECLARE_DRIVER_INIT(peplus);
	DECLARE_DRIVER_INIT(peplussb);
	DECLARE_DRIVER_INIT(pepluss64);
	DECLARE_DRIVER_INIT(peplussbw);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_peplus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void peplus_load_superdata(const char *bank_name);
	DECLARE_PALETTE_INIT(peplus);
	void handle_lightpen();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

static const UINT8  id_022[8] = { 0x00, 0x01, 0x04, 0x09, 0x13, 0x16, 0x18, 0x00 };
static const UINT16 id_023[8] = { 0x4a6c, 0x4a7b, 0x4a4b, 0x4a5a, 0x4a2b, 0x4a0a, 0x4a19, 0x4a3a };


/**************
* Memory Copy *
***************/

void peplus_state::peplus_load_superdata(const char *bank_name)
{
	UINT8 *super_data = memregion(bank_name)->base();

	/* Distribute Superboard Data */
	memcpy(m_s3000_ram, &super_data[0x3000], 0x1000);
	memcpy(m_s5000_ram, &super_data[0x5000], 0x1000);
	memcpy(m_s7000_ram, &super_data[0x7000], 0x1000);
	memcpy(m_sb000_ram, &super_data[0xb000], 0x1000);
	memcpy(m_sd000_ram, &super_data[0xd000], 0x1000);
	memcpy(m_sf000_ram, &super_data[0xf000], 0x1000);
}


/*****************
* Write Handlers *
******************/

WRITE8_MEMBER(peplus_state::peplus_bgcolor_w)
{
	int i;

	for (i = 0; i < m_palette->entries(); i++)
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

		m_palette->set_pen_color((15 + (i*16)), rgb_t(r, g, b));
	}
}


/* ROCKWELL 6545 - Transparent Memory Addressing */

MC6845_ON_UPDATE_ADDR_CHANGED(peplus_state::crtc_addr)
{
	m_vid_address = address;
}


WRITE8_MEMBER(peplus_state::peplus_crtc_mode_w)
{
	/* Reset timing logic */
}

void peplus_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ASSERT_LP:
		m_crtc->assert_light_pen_input();
		break;
	default:
		assert_always(FALSE, "Unknown id in peplus_state::device_timer");
	}
}


void peplus_state::handle_lightpen()
{
	int x_val = read_safe(ioport("TOUCH_X"), 0x00);
	int y_val = read_safe(ioport("TOUCH_Y"), 0x00);
	const rectangle &vis_area = m_screen->visible_area();
	int xt, yt;

	xt = x_val * vis_area.width() / 1024 + vis_area.min_x;
	yt = y_val * vis_area.height() / 1024 + vis_area.min_y;

	timer_set(m_screen->time_until_pos(yt, xt), TIMER_ASSERT_LP, 0);
}

WRITE_LINE_MEMBER(peplus_state::crtc_vsync)
{
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
	handle_lightpen();
}

WRITE8_MEMBER(peplus_state::peplus_crtc_display_w)
{
	m_videoram[m_vid_address] = data;
	m_palette_ram[m_vid_address] = m_io_port[1];
	m_palette_ram2[m_vid_address] = m_io_port[3];

	m_bg_tilemap->mark_tile_dirty(m_vid_address);

	/* An access here triggers a device read !*/
	m_crtc->register_r(space, 0);
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
		peplus_load_superdata(bank_name);
	}

	m_cmos_ram[offset] = data;
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

	m_bv_enable_state = (data >> 4) & 1;
}

WRITE8_MEMBER(peplus_state::i2c_nvram_w)
{
	m_i2cmem->write_scl(BIT(data, 2));
	m_sda_dir = BIT(data, 1);
	m_i2cmem->write_sda(BIT(data, 0));
}


/****************
* Read Handlers *
****************/

READ8_MEMBER(peplus_state::peplus_duart_r)
{
	// Used for Slot Accounting System Communication
	return 0x00;
}

/* Last Color in Every Palette is bgcolor */
READ8_MEMBER(peplus_state::peplus_bgcolor_r)
{
	return m_palette->pen_color(15); // Return bgcolor from First Palette
}

READ8_MEMBER(peplus_state::peplus_dropdoor_r)
{
	return 0x00; // Drop Door 0x00=Closed 0x02=Open
}

READ8_MEMBER(peplus_state::peplus_watchdog_r)
{
	return 0x00; // Watchdog
}

READ8_MEMBER(peplus_state::peplus_input0_r)
{
/*
        PE+ bill validators have a dip switch setting to switch between ID-022 and ID-023 protocols.

        Emulating IGT IDO22 Pulse Protocol (IGT Smoke 2.2)
        ID022 protocol requires a 20ms on/off pulse x times for denomination followed by a 50ms stop pulse.
        The DBV then waits for at least 3 toggling (ACK) pulses of alternating 20ms each from the game.
        If no toggling received within 200ms, the bill was rejected by the game (e.g. Max Credits reached).
        Once toggling received, the DBV stacks the bill and sends a 10ms stacked pulses.

        Emulating IGT IDO23 Pulse Protocol (IGT 2.5)
        ID023 protocol requires a start pulse of 50ms ON followed by a 20ms pause.  Next a 15-bit data stream
        is sent based on the country code and denomination (see table below).  And finally a 90ms stop pulse.
        There is then a 200ms pause and the entire sequence is transmitted again two more times.
        The DBV then waits for the toggling much like the ID-022 protocol above, however ends with two 10ms
        stack pulses instead of one.

        Ticket handling has not been emulated.

        IDO23 Country Codes
        -------------------
        0x07 = Canada
        0x25 = USA

        IDO23 USA 15-bit Data Samples:
        ---------+--------------+--------------+-----------+
        Bill Amt | Country Code |  Denom Code  |  Checksum |
        ---------+--------------+--------------+-----------+
        $1       | 1 0 0 1 0 1  |  0 0 1 1 0   |  1 1 0 0  |
        $2       | 1 0 0 1 0 1  |  0 0 1 1 1   |  1 0 1 1  |
        $5       | 1 0 0 1 0 1  |  0 0 1 0 0   |  1 0 1 1  |
        $10      | 1 0 0 1 0 1  |  0 0 1 0 1   |  1 0 1 0  |
        $20      | 1 0 0 1 0 1  |  0 0 0 1 0   |  1 0 1 1  |
        $50      | 1 0 0 1 0 1  |  0 0 0 0 0   |  1 0 1 0  |
        $100     | 1 0 0 1 0 1  |  0 0 0 0 1   |  1 0 0 1  |
        Ticket   | 1 0 0 1 0 1  |  0 0 0 1 1   |  1 0 1 0  |
        ---------+--------------+--------------+-----------+

        Direction Data
        --------------
        A (FA) <-- [FRONT OF BILL] --> (FB) B
        D (BB) <-- [BACK OF BILL ] --> (BA) C

        Pulses are currently time via cpu cycles.
        833.3 cycles per millisecond
        10 ms = 8333 cycles
*/
	UINT64 curr_cycles = m_maincpu->total_cycles();

	// Allow Bill Insert if DBV Enabled
	if (m_bv_enable_state == 0x01 && ((read_safe(ioport("DBV"), 0xff) & 0x01) == 0x00)) {
		// If not busy
		if (m_bv_busy == 0) {
			m_bv_busy = 1;

			// Fetch Current Denomination and Protocol
			m_bv_denomination = ioport("BC")->read();
			m_bv_protocol = ioport("BP")->read();

			if (m_bv_protocol == 0) {
				// ID-022
				m_bv_denomination = id_022[m_bv_denomination];

				if (m_bv_denomination == 0)
					m_bv_state = 0x03; // $1 So Skip Credit Pulse
				else
					m_bv_state = 0x01; // Greater than $1 Needs Credit Pulse
			} else {
				// ID-023
				id023_data = id_023[m_bv_denomination];

				m_bv_data_bit = 14;
				m_bv_loop_count = 0;

				m_bv_state = 0x11;
			}

			m_bv_cycles = curr_cycles;
			m_bv_pulse = 1;
			m_bv_enable_count = 0;
		}
	}

	switch (m_bv_state)
	{
		case 0x00: // Not Active
			m_bv_busy = 0;
			break;
		case 0x01: // Credit Pulse 20ms ON
			if (curr_cycles - m_bv_cycles >= 833.3 * 20) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 0;
				m_bv_state++;
			}
			break;
		case 0x02: // Credit Pulse 20ms OFF
			if (curr_cycles - m_bv_cycles >= 833.3 * 20) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 1;

				m_bv_denomination--;

				if (m_bv_denomination == 0)
					m_bv_state++; // Done with Credit Pulse
				else
					m_bv_state = 0x01; // Continue Pulsing Denomination
			}
			break;
		case 0x03: // Stop Pulse 50ms ON
			if (curr_cycles - m_bv_cycles >= 833.3 * 50) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 0;

				// Reset Toggle Details
				m_bv_last_enable_state = m_bv_enable_state;
				m_bv_enable_count = 0;

				m_bv_state++;
			}
			break;
		case 0x04: // Begin Toggle Polling
			if (m_bv_enable_state != m_bv_last_enable_state) {
				m_bv_enable_count++;
				m_bv_last_enable_state = m_bv_enable_state;

				// Got Enough Toggles, Advance to Stacking
				if (m_bv_enable_count == 0x03) {
					m_bv_cycles = curr_cycles;
					m_bv_pulse = 1;
					m_bv_state++;
				}
			} else {
				// No Toggling Found, Game Rejected Bill
				if (curr_cycles - m_bv_cycles >= 833.3 * 200) {
					m_bv_pulse = 0;
					m_bv_state = 0x00;
				}
			}
			break;
		case 0x05: // Stacked Pulse 10ms ON
			if (curr_cycles - m_bv_cycles >= 833.3 * 10) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 0;
				m_bv_state = 0x00;
			}
			break;
		case 0x11: // Start Pulse 50ms ON
			if (curr_cycles - m_bv_cycles >= 833.3 * 50) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 0;
				m_bv_state++;
			}
			break;
		case 0x12: // Start Pulse 20ms OFF
			if (curr_cycles - m_bv_cycles >= 833.3 * 20) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 1;
				m_bv_state++;
			}
			break;
		case 0x13: // Data Sync Pulse 20ms ON
			if (curr_cycles - m_bv_cycles >= 833.3 * 20) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 1 - ((id023_data >> m_bv_data_bit) & 0x01);
				m_bv_state++;
			}
			break;
		case 0x14: // Data Value Pulse 20ms OFF
			if (curr_cycles - m_bv_cycles >= 833.3 * 20) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 1;

				if (m_bv_data_bit == 0) {
					m_bv_data_bit = 14; // Done with Data Stream
					m_bv_state++;
				} else {
					m_bv_data_bit--;
					m_bv_state = 0x13; // More Data Yet
				}
			}
			break;
		case 0x15: // Stop Pulse 90ms ON
			if (curr_cycles - m_bv_cycles >= 833.3 * 90) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 0;

				if (m_bv_loop_count >= 2) {
					m_bv_state = 0x17; // Done, Ready for Toggling
				} else {
					m_bv_loop_count++;
					m_bv_state++;
				}
			}
			break;
		case 0x16: // Repeat Pulse 200ms OFF
			if (curr_cycles - m_bv_cycles >= 833.3 * 200) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 1;
				m_bv_state = 0x11; // Repeat from Start
			}
			break;
		case 0x17: // Begin Toggle Polling
			if (m_bv_enable_state != m_bv_last_enable_state) {
				m_bv_enable_count++;
				m_bv_last_enable_state = m_bv_enable_state;

				// Got Enough Toggles, Advance to Stacking
				if (m_bv_enable_count == 0x03) {
					m_bv_cycles = curr_cycles;
					m_bv_pulse = 1;
					m_bv_state++;
				}
			} else {
				// No Toggling Found, Game Rejected Bill
				if (curr_cycles - m_bv_cycles >= 833.3 * 200) {
					m_bv_pulse = 0;
					m_bv_state = 0x00;
				}
			}
			break;
		case 0x18: // Stacked Pulse 10ms ON
			if (curr_cycles - m_bv_cycles >= 833.3 * 10) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 0;
				m_bv_state++;
			}
			break;
		case 0x19: // Stacked Pulse 10ms OFF
			if (curr_cycles - m_bv_cycles >= 833.3 * 10) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 1;
				m_bv_state++;
			}
			break;
		case 0x1a: // Stacked Pulse 10ms ON
			if (curr_cycles - m_bv_cycles >= 833.3 * 10) {
				m_bv_cycles = curr_cycles;
				m_bv_pulse = 0;
				m_bv_state = 0x00;
			}
			break;
	}

	if (m_bv_pulse == 1) {
		return (0x70 || ioport("IN0")->read()); // Add Bill Validator Credit Pulse
	} else {
		return ioport("IN0")->read();
	}
}

READ8_MEMBER(peplus_state::peplus_input_bank_a_r)
{
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
	UINT64 curr_cycles = m_maincpu->total_cycles();

	UINT8 sda = 0;
	if(!m_sda_dir)
	{
		sda = m_i2cmem->read_sda();
	}

	if ((read_safe(ioport("SENSOR"), 0x00) & 0x01) == 0x01 && m_coin_state == 0) {
		m_coin_state = 1; // Start Coin Cycle
		m_last_cycles = m_maincpu->total_cycles();
	} else {
		/* Process Next Coin Optic State */
		if (curr_cycles - m_last_cycles > 10000 && m_coin_state != 0) { // Must be below 100ms (833.3 x 100 cycles) or "Coin-in Timeout" error
			m_coin_state++;
			if (m_coin_state > 5)
				m_coin_state = 0;
			m_last_cycles = m_maincpu->total_cycles();
		}
	}

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

	if (curr_cycles - m_last_door > door_wait) {
		if ((read_safe(ioport("DOOR"), 0xff) & 0x01) == 0x01) {
			if (m_doorcycle) {
				m_door_open = (m_door_open ^ 0x01) & 0x01;
			} else {
				m_door_open = 0;
			}
		} else {
			m_door_open = 1;
		}
		m_last_door = m_maincpu->total_cycles();
	}

	if (curr_cycles - m_last_coin_out > 600000/12 && m_coin_out_state != 0) { // Must be below 700ms or it will time out
		if (m_coin_out_state != 2) {
			m_coin_out_state = 2; // Coin-Out Off
		} else {
			m_coin_out_state = 3; // Coin-Out On
		}

		m_last_coin_out = m_maincpu->total_cycles();
	}

	switch (m_coin_out_state)
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

	bank_a = (sda<<7) | bank_a | (m_door_open<<5) | coin_optics | coin_out;

	return bank_a;
}


/****************************
* Video/Character functions *
****************************/

TILE_GET_INFO_MEMBER(peplus_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int pr = m_palette_ram[tile_index];
	int pr2 = m_palette_ram2[tile_index];
	int vr = videoram[tile_index];

	int code = ((pr & 0x0f)*256) | vr;
	int color = (pr>>4) & 0x0f;

	// Access 2nd Half of CGs and CAP
	if (m_jumper_e16_e17 && (pr2 & 0x10) == 0x10)
	{
		code += 0x1000;
		color += 0x10;
	}

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void peplus_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(peplus_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 40, 25);
	m_palette_ram = auto_alloc_array(machine(), UINT8, 0x3000);
	memset(m_palette_ram, 0, 0x3000);
	m_palette_ram2 = auto_alloc_array(machine(), UINT8, 0x3000);
	memset(m_palette_ram2, 0, 0x3000);
}

UINT32 peplus_state::screen_update_peplus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

PALETTE_INIT_MEMBER(peplus_state, peplus)
{
	const UINT8 *color_prom = memregion("proms")->base();
	UINT32 proms_size = memregion("proms")->bytes();
/*  prom bits
    7654 3210
    ---- -xxx   red component.
    --xx x---   green component.
    xx-- ----   blue component.
*/
	int i;

	for (i = 0;i < palette.entries();i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (~color_prom[i % proms_size] >> 0) & 0x01;
		bit1 = (~color_prom[i % proms_size] >> 1) & 0x01;
		bit2 = (~color_prom[i % proms_size] >> 2) & 0x01;
		r = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* green component */
		bit0 = (~color_prom[i % proms_size] >> 3) & 0x01;
		bit1 = (~color_prom[i % proms_size] >> 4) & 0x01;
		bit2 = (~color_prom[i % proms_size] >> 5) & 0x01;
		g = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* blue component */
		bit0 = (~color_prom[i % proms_size] >> 6) & 0x01;
		bit1 = (~color_prom[i % proms_size] >> 7) & 0x01;
		bit2 = 0;
		b = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( peplus )
			GFXDECODE_ENTRY( "gfx1", 0x00000, gfx_8x8x4_planar, 0, 32 )
GFXDECODE_END


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( peplus_map, AS_PROGRAM, 8, peplus_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_SHARE("prograram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( peplus_iomap, AS_IO, 8, peplus_state )
	// Battery-backed RAM (0x1000-0x01fff Extended RAM for Superboards Only)
	AM_RANGE(0x0000, 0x1fff) AM_RAM_WRITE(peplus_cmos_w) AM_SHARE("cmos")

	// CRT Controller
	AM_RANGE(0x2008, 0x2008) AM_WRITE(peplus_crtc_mode_w)
	AM_RANGE(0x2080, 0x2080) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x2081, 0x2081) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x2083, 0x2083) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(peplus_crtc_display_w)

	// Superboard Data
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_SHARE("s3000_ram")

	// Sound and Dipswitches
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("SW1")/* likely ay8910 input port, not direct */ AM_DEVWRITE("aysnd", ay8910_device, data_w)

	// Superboard Data
	AM_RANGE(0x5000, 0x5fff) AM_RAM AM_SHARE("s5000_ram")

	// Background Color Latch
	AM_RANGE(0x6000, 0x6000) AM_READ(peplus_bgcolor_r) AM_WRITE(peplus_bgcolor_w)

	// Bogus Location for Video RAM
	AM_RANGE(0x06001, 0x06400) AM_RAM AM_SHARE("videoram")

	// Superboard Data
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_SHARE("s7000_ram")

	// Input Bank A, Output Bank C
	AM_RANGE(0x8000, 0x8000) AM_READ(peplus_input_bank_a_r) AM_WRITE(peplus_output_bank_c_w)

	// Drop Door, I2C EEPROM Writes
	AM_RANGE(0x9000, 0x9000) AM_READ(peplus_dropdoor_r) AM_WRITE(i2c_nvram_w)

	// Input Banks B & C, Output Bank B
	AM_RANGE(0xa000, 0xa000) AM_READ(peplus_input0_r) AM_WRITE(peplus_output_bank_b_w)

	// Superboard Data
	AM_RANGE(0xb000, 0xbfff) AM_RAM AM_SHARE("sb000_ram")

	// Output Bank A
	AM_RANGE(0xc000, 0xc000) AM_READ(peplus_watchdog_r) AM_WRITE(peplus_output_bank_a_w)

	// Superboard Data
	AM_RANGE(0xd000, 0xdfff) AM_RAM AM_SHARE("sd000_ram")

	// DUART
	AM_RANGE(0xe000, 0xe00f) AM_READWRITE(peplus_duart_r, peplus_duart_w)

	// Superboard Data
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("sf000_ram")

	/* Ports start here */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_RAM AM_SHARE("io_port")
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

CUSTOM_INPUT_MEMBER(peplus_state::peplus_input_r)
{
	UINT8 inp_ret = 0x00;
	UINT8 inp_read = ioport((const char *)param)->read();

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

	PORT_START("DBV")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Bill In") PORT_IMPULSE(1)

	PORT_START("BC")
	PORT_CONFNAME( 0x1f, 0x00, "Bill Choices" )
	PORT_CONFSETTING( 0x00, "$1" )
	PORT_CONFSETTING( 0x01, "$2" )
	PORT_CONFSETTING( 0x02, "$5" )
	PORT_CONFSETTING( 0x03, "$10" )
	PORT_CONFSETTING( 0x04, "$20" )
	PORT_CONFSETTING( 0x05, "$50" )
	PORT_CONFSETTING( 0x06, "$100" )

	PORT_START("BP")
	PORT_CONFNAME( 0x1f, 0x00, "Bill Protocol" )
	PORT_CONFSETTING( 0x00, "ID-022" )
	PORT_CONFSETTING( 0x01, "ID-023" )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Line Frequency" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "60Hz" )
	PORT_DIPSETTING(    0x00, "50Hz" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
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
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Card Cage") PORT_CODE(KEYCODE_M) PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( nonplus_poker )
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) // Bill Acceptor

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Card Cage") PORT_CODE(KEYCODE_M) PORT_TOGGLE

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x02, 0x02, "Credit" )        PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "Coin Play" )
	PORT_DIPSETTING(    0x00, "Credit Play" )
	PORT_DIPNAME( 0x04, 0x00, "Coin Acceptor" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Mechanical" )
	PORT_DIPSETTING(    0x00, "Optical" )
	PORT_DIPNAME( 0x08, 0x08, "Double Up" )     PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Progressive" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, "Max Hopper Pay" )    PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "300 Coins" )
	PORT_DIPSETTING(    0x40, "400 Coins" )
	PORT_DIPSETTING(    0x20, "600 Coins" )
	PORT_DIPSETTING(    0x00, "1000 Coins" )
	PORT_DIPNAME( 0x80, 0x00, "Show Pay Table" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) // Bill Acceptor

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Card Cage") PORT_CODE(KEYCODE_M) PORT_TOGGLE
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) // Bill Acceptor

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Card Cage") PORT_CODE(KEYCODE_M) PORT_TOGGLE
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) // Bill Acceptor

	PORT_START("TOUCH_X")
	PORT_BIT( 0xffff, 0x200, IPT_LIGHTGUN_X ) PORT_MINMAX(0x00, 1024) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)
	PORT_START("TOUCH_Y")
	PORT_BIT( 0xffff, 0x200, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x00, 1024) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Light Pen") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Card Cage") PORT_CODE(KEYCODE_M) PORT_TOGGLE
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) // Bill Acceptor

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, peplus_state,peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Card Cage") PORT_CODE(KEYCODE_M) PORT_TOGGLE
INPUT_PORTS_END


/*************************
*     Machine Reset      *
*************************/

void peplus_state::machine_reset()
{
}

/*************************
*     Machine Driver     *
*************************/

static MACHINE_CONFIG_START( peplus, peplus_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I80C32, XTAL_20MHz/2) /* 10MHz */
	MCFG_CPU_PROGRAM_MAP(peplus_map)
	MCFG_CPU_IO_MAP(peplus_iomap)

	MCFG_NVRAM_ADD_0FILL("cmos")

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE((52+1)*8, (31+1)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 25*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(peplus_state, screen_update_peplus)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", peplus)
	MCFG_PALETTE_ADD("palette", 16*16*2)
	MCFG_PALETTE_INIT_OWNER(peplus_state, peplus)

	MCFG_MC6845_ADD("crtc", R6545_1, "screen", XTAL_20MHz/8/3)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_ADDR_CHANGED_CB(peplus_state, crtc_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(peplus_state, crtc_vsync))

	MCFG_X2404P_ADD("i2cmem")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8912, XTAL_20MHz/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END



/*************************
*      Driver Init       *
*************************/

/* Non Plus board */
DRIVER_INIT_MEMBER(peplus_state,nonplus)
{
	door_wait = 500;
	m_doorcycle = FALSE;
	m_wingboard = FALSE;
	m_jumper_e16_e17 = FALSE;
}

/* Normal board */
DRIVER_INIT_MEMBER(peplus_state,peplus)
{
	door_wait = 500;
	m_doorcycle = TRUE;
	m_wingboard = FALSE;
	m_jumper_e16_e17 = FALSE;
}

/* Superboard */
DRIVER_INIT_MEMBER(peplus_state,peplussb)
{
	door_wait = 500;
	m_doorcycle = TRUE;
	m_wingboard = FALSE;
	m_jumper_e16_e17 = FALSE;
	peplus_load_superdata("user1");
}

/* Superboard with 64K CG rom set */
DRIVER_INIT_MEMBER(peplus_state,pepluss64)
{
	door_wait = 500;
	m_doorcycle = TRUE;
	m_wingboard = FALSE;
	m_jumper_e16_e17 = TRUE;
	peplus_load_superdata("user1");
}

/* Superboard with Attached Wingboard */
DRIVER_INIT_MEMBER(peplus_state,peplussbw)
{
	door_wait = 12345;
	m_doorcycle = TRUE;
	m_wingboard = TRUE;
	m_jumper_e16_e17 = TRUE;
	peplus_load_superdata("user1");
}


/*************************
*        Rom Load        *
*************************/

ROM_START( peset001 ) /* Normal board : Set Chip (Set001) - PE+ Set Denomination */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "set001.u68",   0x00000, 0x10000, CRC(03397ced) SHA1(89d8ba7e6706e6d34ae9aae09a8a631fff06a36f) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( peset004 ) /* Normal board : Set Chip (Set004) - PE+ Set Denomination / Enable Validator */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "set004.u68",   0x00000, 0x10000, CRC(b5729571) SHA1(fa3bb1fec81692a898213f9521ac0b2a4d1a8968) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

/* Known to exsist SET033 - PE+ Set Denomination / Enable Validator / SAS 4.0 */

ROM_START( peset038 ) /* Normal board : Set Chip (Set038) - PE+ Set Denomination / Enable Validator */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "set038.u68",   0x00000, 0x10000, CRC(9c4b1d1a) SHA1(8a65cd1d8e2d74c7b66f4dfc73e7afca8458e979) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( peivc006 ) /* Normal board : Clear Chip (IVC006) - PE+ Clear CMOS / E-Square */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ivc006.u68",   0x00000, 0x8000, CRC(9a408727) SHA1(cc2d9ba66c461ae81f9fae1e068981d8de093416) ) /* 27C256 EPROM */
	ROM_RELOAD(               0x08000, 0x8000)

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepk1024 ) /* Normal (non-plus) board : Aces and Faces 4 of a Kind Bonus Poker (PK1024) */
/*
                                      2-10 J-A
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  SF  RF  (Bonus)
--------------------------------------------------------------
  ????      1    2    3    4    5   8  25  50  50 250    800
  % Range: 95.3-97.3%  Optimum: 99.3%  Hit Frequency: 45.5%
     Programs Available: PK1024
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pk1024-pc095.u58",   0x00000, 0x8000, CRC(c0b6d093) SHA1(80f7dbd9dff52cd4e31a5243026814aa9edb98df) ) /* Game Version: PC095 */
	ROM_RELOAD(                     0x08000, 0x8000) /* 32K version build for the original PE boards (non-plus) */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mr0-cg745.u52",   0x00000, 0x4000, CRC(f8aee592) SHA1(fc5fb03698df24ebcf41ccfbce3a8fdd03ac9368) )
	ROM_LOAD( "mg0-cg745.u53",   0x08000, 0x4000, CRC(cdf88a59) SHA1(9ab32b7b8bbdf35ee5a45cd58d9a341555b03ee0) )
	ROM_LOAD( "mb0-cg745.u54",   0x10000, 0x4000, CRC(3fad0bc0) SHA1(3c8409c004bb40d7be6e5cbdd02b3fa3b5800342) )
	ROM_LOAD( "mx0-cg745.u55",   0x18000, 0x4000, CRC(a8cf2a59) SHA1(de623a95237f28827710a86ae86c52106b55b349) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u37", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0002 ) /* Normal board : Standard Draw Poker (PP0002) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BB       1    2    3    4    5   8  25  50 250   1000
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0002, X000002P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0002_a45-a74.u68",   0x00000, 0x10000, CRC(921ce116) SHA1(a3b83b6fcfa27cca7e392efc62568eb6495c136a) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0002a ) /* Normal board : Standard Draw Poker (PP0002) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BB       1    2    3    4    5   8  25  50 250   1000
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0002, X000002P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0002_a58-a6y.u68",   0x00000, 0x10000, CRC(65ed2956) SHA1(5ee9be5daee80fe1aa716fdb488f154e875d394d) ) /* Game Version: A58, Library Version: A6Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0008 ) /* Normal board : Standard Draw Poker (PP0008) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   CD       1    2    3    4    6   9  25  50 250    940
  % Range: 95.9-97.9%  Optimum: 99.9%  Hit Frequency: 45.3%
     Programs Available: PP0008, X002247P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0008_520-516.u68",   0x00000, 0x8000, CRC(7a02060f) SHA1(8672565bd62fa76aa738c1f8c6aeb0c0d22daf93) ) /* Game Version: 520, Library Version: 516 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0009 ) /* Normal board : Standard Draw Poker (PP0009) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   GA       1    2    3    4    5   6  25  50 250    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0009

This program set is superseded by PP0060
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0009_554-544.u68",   0x00000, 0x8000, CRC(2e3e45f7) SHA1(035994b20d3975bb2287f12b4a42d5fdae68b13b) ) /* Game Version: 554, Library Version: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0010 ) /* Normal board : Standard Draw Poker (PP0010) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   GB       1    2    3    4    5   6  25  50 250   1000
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 45.3%
     Programs Available: PP0010
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0010_a45-a74.u68",   0x00000, 0x10000, CRC(39440afb) SHA1(b2ca246d6854008cf5b7081e9842be6f6f0666b0) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0014 ) /* Normal board : Standard Draw Poker (PP0014) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   QJ       1    2    3    4    5   8  25  50 300    400
  % Range: 92.3-94.3%  Optimum: 96.3%  Hit Frequency: 45.6%
     Programs Available: PP0014
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0014_a2d-a48.u68",   0x00000, 0x10000, CRC(235ddd99) SHA1(c8623a6beeb31ce33a44e1cb6a235e6561f0b3de) ) /* Game Version: A2D, Library Version: A48 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0014a ) /* Normal board : Standard Draw Poker (PP0014) - 100 Coins In */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   QJ       1    2    3    4    5   8  25  50 300    400
  % Range: 92.3-94.3%  Optimum: 96.3%  Hit Frequency: 45.6%
     Programs Available: PP0014
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0014_654-523.u68",   0x00000, 0x8000, CRC(1cf42d0e) SHA1(a5564a6ff31a24e6052c95eaacce65c61dd1600d) ) /* Game Version: 665, Library Version: 523, Video Lib Ver: 523 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) ) /* Not 100% correct?? */
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0021 ) /* Normal board : Standard Draw Poker (PP0021) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0021

NOTE: REQUIRES Progressive link which is not currently supported
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0021_723-703.u68",   0x00000, 0x10000, CRC(58f2a68b) SHA1(72c0a29016b17f7e308a9e9b2d724771b5e26560) ) /* Game Version: 723, Library Version: 703 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0023 ) /* Normal board : Tens or Better (PP0023) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????     1     2    3    4    5   6  25  50 250    800
  % Range: 93.3-95.3%  Optimum: 97.3%  Hit Frequency: 45.4%
     Programs Available: PP0023
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0023_520-516.u68",   0x00000, 0x8000, CRC(883ff93e) SHA1(e355933ee6a316b5672e5a887e09c691ab242873) ) /* Game Version: 520, Library Version: 516, Video Lib Ver: 516 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0038 ) /* Normal board : Standard Draw Poker (PP0038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   PE       1    2    3    4    6   9  25  50 300    900
  % Range: 95.8-97.8%  Optimum: 99.8%  Hit Frequency: 45.4%
     Programs Available: PP0038, X002121P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0038_a45-a74.u68",   0x00000, 0x10000, CRC(85fe387e) SHA1(a0aa4cb422c04066d61d665943eced30b2eaf5b2) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg821.u72",   0x00000, 0x8000, CRC(e91f1192) SHA1(1a00027b681fad6b350366b2dff7411445a07f05) )
	ROM_LOAD( "mgo-cg821.u73",   0x08000, 0x8000, CRC(fa417bbc) SHA1(8c59d9156fb52099bf76b0a7e0da3a27518d6f19) )
	ROM_LOAD( "mbo-cg821.u74",   0x10000, 0x8000, CRC(4229457a) SHA1(aa1f26792279a834ed2025d6be58fa7ea38329fd) )
	ROM_LOAD( "mxo-cg821.u75",   0x18000, 0x8000, CRC(2da9729a) SHA1(3a5cf4c794e0057bdb705e0c7da541c3c8b48591) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0040 ) /* Normal board : Standard Draw Poker (PP0040) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   WA       1    2    3    4    5   7  20  50 300    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0040, X000040P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0040_a45-a74.u68",   0x00000, 0x10000, CRC(df3675b3) SHA1(668f33c97fa1c0b69a8601da02bd07e3c5df81b4) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0040a ) /* Normal board : Standard Draw Poker (PP0040) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   WA       1    2    3    4    5   7  20  50 300    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0040, X000040P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0040_979-a0c.u68",   0x00000, 0x10000, CRC(fef4fbfe) SHA1(9f07a2bee990181c9eb40e40b957aa9555ae2586) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0040b ) /* Normal board : Standard Draw Poker (PP0040) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   WA       1    2    3    4    5   7  20  50 300    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0040, X000040P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0040_a0b-a1s.u68",   0x00000, 0x10000, CRC(0530ffb3) SHA1(ae5568c05dd640b040535482d1ba6fb45323c585) ) /* Game Version: A0B, Library Version: A1S */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2014.u72",  0x00000, 0x8000, CRC(90220e65) SHA1(c03417e09b72c8f3afe182b15e41e9d9ae32a831) ) /* 09/01/94  @IGT  IGT-EURO */
	ROM_LOAD( "mgo-cg2014.u73",  0x08000, 0x8000, CRC(3189b3e3) SHA1(34c4c170dba74a50ffcbc5c5c97b37200b6d2509) )
	ROM_LOAD( "mbo-cg2014.u74",  0x10000, 0x8000, CRC(77650c39) SHA1(7e89682d0a192ef83288bc3ad22dea45129344f9) )
	ROM_LOAD( "mxo-cg2014.u75",  0x18000, 0x8000, CRC(af9c89a6) SHA1(e256259c20f5b1308e89c9fbb424d1396bccbcd1) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0041 ) /* Normal board : Standard Draw Poker (PP0041) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   WB       1    2    3    4    5   7  20  50 300   1000
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 45.3%
     Programs Available: PP0041
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0041_a45-a74.u68",   0x00000, 0x10000, CRC(406c8193) SHA1(006b9bf57263fb84ed752f0a44603837a68a2d71) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0042 ) /* Normal board : 10's or Better (PP0042) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P7A      1     1    3    4    6   9  25  50 300    800
  % Range: 86.8-88.8%  Optimum: 90.8%  Hit Frequency: 49.1%
     Programs Available: PP0042, X000042P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0042_979-a0c.u68",   0x00000, 0x10000, CRC(2f4f4e59) SHA1(ba74de70cf455f7e0b13d4757632d27af64a573b) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0042a ) /* Normal board : 10's or Better (PP0042) - Auto Hold & Progressive */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P7A      1     1    3    4    6   9  25  50 300    800
  % Range: 86.8-88.8%  Optimum: 90.8%  Hit Frequency: 49.1%
     Programs Available: PP0042, X000042P
*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 10/22/91  @IGT */
	ROM_LOAD( "pp0042_768-761.u68",   0x00000, 0x10000, CRC(424def20) SHA1(4a0c142d907c0651eef3eb0de57e6212ec268005) ) /* Game Version: 768, Library Version: 761, Video Lib Ver: 761 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0043 ) /* Normal board : 10's or Better (PP0043) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P7B      1     1    3    4    6   9  25  50 300   1000
  % Range: 87.4-89.4%  Optimum: 91.4%  Hit Frequency: 49.0%
     Programs Available: PP0043, X000043P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0043_a45-a74.u68",   0x00000, 0x10000, CRC(04051a88) SHA1(e7a9ec2ab7f6f575245d47ee10a03f39c887d1b3) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0043a ) /* Normal board : 10's or Better (PP0043) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P7B      1     1    3    4    6   9  25  50 300   1000
  % Range: 87.4-89.4%  Optimum: 91.4%  Hit Frequency: 49.0%
     Programs Available: PP0043, X000043P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0043_a43-a71.u68",   0x00000, 0x10000, CRC(7da397d7) SHA1(9b6479693f4d1224fce5635c3e7cff6463103e1e) ) /* Game Version: A43, Library Version: A71 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0043b ) /* Normal board : 10's or Better (PP0043) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P7B      1     1    3    4    6   9  25  50 300   1000
  % Range: 87.4-89.4%  Optimum: 91.4%  Hit Frequency: 49.0%
     Programs Available: PP0043, X000043P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0043_a0b-a1s.u68",   0x00000, 0x10000, CRC(be1561ab) SHA1(a3f6d306992acabb6a618a4035cc739f3c3c45e8) ) /* Game Version: A0B, Library Version: A1S */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2014.u72",  0x00000, 0x8000, CRC(90220e65) SHA1(c03417e09b72c8f3afe182b15e41e9d9ae32a831) ) /* 09/01/94  @IGT  IGT-EURO */
	ROM_LOAD( "mgo-cg2014.u73",  0x08000, 0x8000, CRC(3189b3e3) SHA1(34c4c170dba74a50ffcbc5c5c97b37200b6d2509) )
	ROM_LOAD( "mbo-cg2014.u74",  0x10000, 0x8000, CRC(77650c39) SHA1(7e89682d0a192ef83288bc3ad22dea45129344f9) )
	ROM_LOAD( "mxo-cg2014.u75",  0x18000, 0x8000, CRC(af9c89a6) SHA1(e256259c20f5b1308e89c9fbb424d1396bccbcd1) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0045 ) /* Normal board : 10's or Better (PP0045) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8A      1     1    3    4    5   8  25  50 300    800
  % Range: 84.6-86.6%  Optimum: 88.6%  Hit Frequency: 49.2%
     Programs Available: PP0045, X000045P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0045_a45-a74.u68",   0x00000, 0x10000, CRC(9c7cf6d7) SHA1(3da9829678b853d85146b66b40800257a8eaa151) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0045a ) /* Normal board : 10's or Better (PP0045) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8A      1     1    3    4    5   8  25  50 300    800
  % Range: 84.6-86.6%  Optimum: 88.6%  Hit Frequency: 49.2%
     Programs Available: PP0045, X000045P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0045_a45-a74.u68",   0x00000, 0x10000, CRC(9c7cf6d7) SHA1(3da9829678b853d85146b66b40800257a8eaa151) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1248.u72",  0x00000, 0x8000, CRC(4e2f9276) SHA1(25b84314e75408f9a057a89ac9154be7768813eb) ) /* Custom Gambler Downtown Reno card backs */
	ROM_LOAD( "mgo-cg1248.u73",  0x08000, 0x8000, CRC(5ecfb257) SHA1(aebe9580434007cae52091eb1814ce201698a4b0) )
	ROM_LOAD( "mbo-cg1248.u74",  0x10000, 0x8000, CRC(c4b84599) SHA1(e753537c4615b42aa726d19f19dfa934050ac54e) ) /* These graphics will work for many other standard poker sets */
	ROM_LOAD( "mxo-cg1248.u75",  0x18000, 0x8000, CRC(ac2ddd3c) SHA1(ce76c77a3d8b5d71d272b9982d43a35e913b54de) ) /* However there is no support for Deuces Wild sets */

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1248.u50", 0x0000, 0x0100, CRC(be238287) SHA1(9bfe601df8d3e40307c15a6b871e79f12fab3169) )
ROM_END

ROM_START( pepp0045b ) /* Normal board : 10's or Better (PP0045) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8A      1     1    3    4    5   8  25  50 300    800
  % Range: 84.6-86.6%  Optimum: 88.6%  Hit Frequency: 49.2%
     Programs Available: PP0045, X000045P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0045_a45-a74.u68",   0x00000, 0x10000, CRC(9c7cf6d7) SHA1(3da9829678b853d85146b66b40800257a8eaa151) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1150.u72",  0x00000, 0x8000, CRC(91186809) SHA1(1fc91926cf854411738128cb82d1fce6a8f8c8fd) ) /* Custom Par-A-Dice Riverboat Casino card backs */
	ROM_LOAD( "mgo-cg1150.u73",  0x08000, 0x8000, CRC(a1e66166) SHA1(6dfa3952636fb6cd24c725139624571ab279221c) )
	ROM_LOAD( "mbo-cg1150.u74",  0x10000, 0x8000, CRC(8c2c3c0e) SHA1(fe1f1648ff7751e75b874db83f4ec5f54e95083e) ) /* These graphics will work for many other standard poker sets */
	ROM_LOAD( "mxo-cg1150.u75",  0x18000, 0x8000, CRC(ece41352) SHA1(3c7afb98254c05ce139ca9675ecb415641552371) ) /* However there is no support for Deuces Wild sets */

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1150.u50", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0045c ) /* Normal board : 10's or Better (PP0045) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8A      1     1    3    4    5   8  25  50 300    800
  % Range: 84.6-86.6%  Optimum: 88.6%  Hit Frequency: 49.2%
     Programs Available: PP0045, X000045P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0045_a45-a74.u68",   0x00000, 0x10000, CRC(9c7cf6d7) SHA1(3da9829678b853d85146b66b40800257a8eaa151) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1072.u72",  0x00000, 0x8000, CRC(8e5cf3bf) SHA1(a8c2fde9105a37eddc218ae1476cdbfb0271e314) ) /* Custom Annie Oakley's Central City graphics */
	ROM_LOAD( "mgo-cg1072.u73",  0x08000, 0x8000, CRC(a3c85c1b) SHA1(9b810c5779dde21db6da5bac5cf797bad65c2c1b) )
	ROM_LOAD( "mbo-cg1072.u74",  0x10000, 0x8000, CRC(833371e1) SHA1(5d7a994aee61a751f89171885423276b86e872b6) ) /* These graphics will work for many other standard poker sets */
	ROM_LOAD( "mxo-cg1072.u75",  0x18000, 0x8000, CRC(0df703b3) SHA1(2042251cc9c11687ff7fd920213a448974ff3050) ) /* However there is no support for Deuces Wild sets */

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1072.u50", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0045d ) /* Normal board : 10's or Better (PP0045) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8A      1     1    3    4    5   8  25  50 300    800
  % Range: 84.6-86.6%  Optimum: 88.6%  Hit Frequency: 49.2%
     Programs Available: PP0045, X000045P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0045_a45-a74.u68",   0x00000, 0x10000, CRC(9c7cf6d7) SHA1(3da9829678b853d85146b66b40800257a8eaa151) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg881.u72",   0x00000, 0x8000, CRC(282a029f) SHA1(42b35761839d6379ddfb4eed20f90d9f7b145e64) ) /* Custom Las Vegas Rio graphics */
	ROM_LOAD( "mgo-cg881.u73",   0x08000, 0x8000, CRC(af433702) SHA1(fbd877c06eaab433332c94f135e13a8c041fa1a2) )
	ROM_LOAD( "mbo-cg881.u74",   0x10000, 0x8000, CRC(c5b0a0b3) SHA1(a989d21f4b10a09d3cfd0bbb9f53b4ad326561b9) ) /* These graphics will work for many other standard poker sets */
	ROM_LOAD( "mxo-cg881.u75",   0x18000, 0x8000, CRC(6a78bc1d) SHA1(7861465ab98df5219330d58a3e5a4bd37a393534) ) /* However there is no support for Deuces Wild sets */

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap881.u50", 0x0000, 0x0100, CRC(e51990d5) SHA1(41946722b61e955d37808761d451fc894e6adc8a) )
ROM_END

ROM_START( pepp0046 ) /* Normal board : 10's or Better (PP0046) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8B      1     1    3    4    5   8  25  50 300   1000
  % Range: 85.2-87.2%  Optimum: 89.2%  Hit Frequency: 49.0%
     Programs Available: PP0046, X000046P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0046_a45-a74.u68",   0x00000, 0x10000, CRC(fe5903f2) SHA1(963d1ade6051da19bb40b313221037c0fdfc0fc9) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0046a ) /* Normal board : 10's or Better (PP0046) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8B      1     1    3    4    5   8  25  50 300   1000
  % Range: 85.2-87.2%  Optimum: 89.2%  Hit Frequency: 49.0%
     Programs Available: PP0046, X000046P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0046_a58-a6y.u68",   0x00000, 0x10000, CRC(ea9094bf) SHA1(6154864b0ea0c8bc75085064ea71f8cb0ff312af) ) /* Game Version: A58, Library Version: A6Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0046b ) /* Normal board : 10's or Better (PP0046) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8B      1     1    3    4    5   8  25  50 300   1000
  % Range: 85.2-87.2%  Optimum: 89.2%  Hit Frequency: 49.0%
     Programs Available: PP0046, X000046P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0046_554-544.u68",   0x00000, 0x8000, CRC(d8687b76) SHA1(9bd0b71b60d26b46af58a8e77f4b05900a4075aa) ) /* Game Version: 554, Library Version: 544, Video Lib Ver: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0048 ) /* Normal board : Joker Poker (PP0048) */
/*
     Programs Available: PP0048
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0048_554-544.u68",   0x00000, 0x8000, CRC(c728af7e) SHA1(b4b56bfa34d2b4df22a8e29fae9b8e8f7d708089) ) /* Game Version: 554, Library Version: 544, Video Lib Ver: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0051 ) /* Normal board : Joker Poker (PP0051) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P17A      1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 44.7%
     Programs Available: PP0051

Superseded by PP0428 (Non Double-up) / PP0459

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0051_554-544.u68",   0x00000, 0x8000, CRC(66329607) SHA1(bc980257645225a24cd71a10b4a4cb592f878b3b) ) /* Game Version: 554, Library Version: 544, Video Lib Ver: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0053 ) /* Normal board : Joker Poker (Aces or Better) (PP0053) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P18B      1    1   2   3    5   6  20  50 100 200 500   1000
  % Range: 90.6-92.6%  Optimum: 94.6%  Hit Frequency: 39.2%
     Programs Available: PP0053, X000053P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0053_a45-a74.u68",   0x00000, 0x10000, CRC(0657b8a7) SHA1(4fb9762d84ef0e02dbab9f9da5a1dfdd9be2e86e) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0055 ) /* Normal board : Deuces Wild Poker (PP0055) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0055_961-984.u68",   0x00000, 0x10000, CRC(c6b897cc) SHA1(9ba200652db58e602f388c21aaf9b3f837412385) ) /* Game Version: 961, Library Version: 984 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0055a ) /* Normal board : Deuces Wild Poker (PP0055) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /*  10/23/95   @IGT  L95-2432  */
	ROM_LOAD( "pp0055_a47-a76.u68",   0x00000, 0x10000, CRC(adff06ea) SHA1(098409bd4474a69217e3cd17ee8c650005cc3e17) ) /* Game Version: A47, Library Version: A76 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0055b ) /* Normal board : Deuces Wild Poker (PP0055) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /*  10/23/95   @IGT  L95-2432  */
	ROM_LOAD( "pp0055_a47-a76.u68",   0x00000, 0x10000, CRC(adff06ea) SHA1(098409bd4474a69217e3cd17ee8c650005cc3e17) ) /* Game Version: A47, Library Version: A76 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1276.u72",   0x00000, 0x8000, CRC(9ebc89f2) SHA1(983ace4a9269dea1ddc6b9837df0f0db6b2a1c91) ) /* Custom Skyline Casino card backs */
	ROM_LOAD( "mgo-cg1276.u73",   0x08000, 0x8000, CRC(dc7ae9a0) SHA1(64384138a6eeb7adb27951fe9e8527f708872efb) )
	ROM_LOAD( "mbo-cg1276.u74",   0x10000, 0x8000, CRC(145b39ad) SHA1(cea3835428f609c5830fc35e5ade63762f71954c) )
	ROM_LOAD( "mxo-cg1276.u75",   0x18000, 0x8000, CRC(35e32287) SHA1(0cbea6f413c7cf76e343d9f7250fd6fcd87f6df9) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1276.u50", 0x0000, 0x0100, CRC(4ce5aef5) SHA1(e9a9f358aedeb9ed917162eafd6ffade66d460e8) )
ROM_END

ROM_START( pepp0055c ) /* Normal board : Deuces Wild Poker (PP0055) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0055_600-550.u68",   0x00000, 0x8000, CRC(3c00285e) SHA1(b9028de6962619e438ef927f8223e04dde5c7f87) ) /* Game Version: 600, Library Version: 550, Video Lib Ver: 550 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) )
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0057 ) /* Normal board : Deuces Wild Poker (PP0057) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P34A      1    2    2   3   5   9  15  25 200 250    800
  % Range: 96.8-98.8%  Optimum: 100.8%  Hit Frequency: 45.3%
     Programs Available: PP0057, X000057P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0057_a47-a76.u68",   0x00000, 0x10000, CRC(44ebb68d) SHA1(4864ba62c225a3ecd576d1a82fcbe1e30d65244d) ) /* Game Version: A47, Library Version: A76 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0057a ) /* Normal board : Deuces Wild Poker (PP0057) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P34A      1    2    2   3   5   9  15  25 200 250    800
  % Range: 96.8-98.8%  Optimum: 100.8%  Hit Frequency: 45.3%
     Programs Available: PP0057, X000057P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0057_540-508.u68",   0x00000, 0x8000, CRC(9a8281c2) SHA1(d411294062d7896a7f68a1c4a6295e18787dc7d6) ) /* Game Version: 540, Library Version: 508, Video Lib Ver: 508 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) )
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0059 ) /* Normal board : Two Pair or Better (PP0059) */
/*
PayTable  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------
   FA      2    3    5    7  11  50 100 250    800
  % Range: 89.5-91.5%  Optimum: 93.5%  Hit Frequency: 38.8%
     Programs Available: PP0059, PP0424 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0059_a45-a74.u68",   0x00000, 0x10000, CRC(6ff02f25) SHA1(b4a8476251044d0a7e3f232fa1ef4e31d8ef6775) ) /* Game Version: A45, Library Version: A75 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0059a ) /* Normal board : Two Pair or Better (PP0059) */
/*
PayTable  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------
   FA      2    3    5    7  11  50 100 250    800
  % Range: 89.5-91.5%  Optimum: 93.5%  Hit Frequency: 38.8%
     Programs Available: PP0059, PP0424 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0059_518-515.u68",   0x00000, 0x8000, CRC(18315252) SHA1(f6712f9edb487dfd7d4d5b83d6ba8d43c776c9bf) ) /* Game Version: 518, Library Version: 515, Video Lib Ver: 515 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0060 ) /* Normal board : Standard Draw Poker (PP0060) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   GA       1    2    3    4    5   6  25  50 250    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0060, X000060P & PP0420 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0060_a45-a74.u68",   0x00000, 0x10000, CRC(5d9e6c2f) SHA1(e1199a1fa57d84223ca87ea5b6ce4fda9afa0e1f) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0060a ) /* Normal board : Standard Draw Poker (PP0060) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   GA       1    2    3    4    5   6  25  50 250    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0060, X000060P & PP0420 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0060_979-a0c.u68",   0x00000, 0x10000, CRC(adedfcfd) SHA1(f974a9c51d4e53c2c44a4c5214d39557d3a36d99) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0060b ) /* Normal board : Standard Draw Poker (PP0060) - Cruise version - Tournament Mode capable */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   GA       1    2    3    4    5   6  25  50 250    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0060, X000060P & PP0420 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /*  11/13/97   @IGT  CRUIS  */
	ROM_LOAD( "pp0060_a6h-a8h.u68",   0x00000, 0x10000, CRC(81963084) SHA1(2493bb040b9d0ea5cfe77f8d07546d3a3ac3716a) ) /* Game Version: A6H, Library Version: A8H */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2002.u72",  0x00000, 0x8000, CRC(d9d03979) SHA1(9729cbb2e5472eb652f8f549dd85047abe11cae0) ) /* 08/30/94   @IGT  CRUIS */
	ROM_LOAD( "mgo-cg2002.u73",  0x08000, 0x8000, CRC(ad5bd2cd) SHA1(e5dacd2827f14dd9811311552b7e3816a36b9284) )
	ROM_LOAD( "mbo-cg2002.u74",  0x10000, 0x8000, CRC(7362f7f3) SHA1(fce4ce2cdd836e37382d39d8b167019cfc4c6166) )
	ROM_LOAD( "mxo-cg2002.u75",  0x18000, 0x8000, CRC(4560fdec) SHA1(63ec67afd378a06d74084bba72fbbe9be12e24d3) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0063 ) /* Normal board : 10's or Better (PP0063) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8A      1     1    3    4    5   8  25  50 300    800
  % Range: 84.6-86.6%  Optimum: 88.6%  Hit Frequency: 49.2%
     Programs Available: PP0063
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0063_518-515.u68",   0x00000, 0x8000, CRC(b31c7be7) SHA1(3203e76434db1e240f5b9642525eac9ea2726a03) ) /* Game Version: 518, Library Version: 515, Video Lib Ver: 515 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0064 ) /* Normal board : Joker Poker (PP0064) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    4   5  20  40 100 200 500    ???
     Programs Available: PP0064

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0064_666-515.u68",   0x00000, 0x8000, CRC(56409362) SHA1(3400734da785edac2af14d8b645f7e3ed04f96a0) ) /* Game Version: 666, Library Version: 515, Video Lib Ver: 515 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0065 ) /* Normal board : Joker Poker (Aces or Better) (PP0065) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P18A      1    1   2   3    5   6  20  50 100 200 500    800
  % Range: 89.8-91.8%  Optimum: 93.8%  Hit Frequency: 37.6%
     Programs Available: PP0065

Superseded by PP0429 (Non Double-up) / PP0458

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0065_944-a00.u68",   0x00000, 0x10000, CRC(76c1a367) SHA1(ea8be9241e9925b5a4206db6875e1572f85fa5fe) ) /* Game Version: 944, Library Version: A00 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0083 ) /* Normal board : Tens or Better (PP0083) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P39D     1     1    3    5    7  11  50 100 300    940
  % Range: 90.6-92.6%  Optimum: 94.6%  Hit Frequency: 48.6%
     Programs Available: PP0083
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0083_554-544.u68",   0x00000, 0x8000, CRC(d415a1dd) SHA1(5a7fef13a6cde7dad5957d8ea3f15d3ac92634cf) ) /* Game Version: 554, Library Version: 544, Video Lib Ver: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0085 ) /* Normal board : Joker Poker (Two Pair or Better) (PP0085) - Double Up always enabled */
/*
                                       w/J     w/oJ
PayTable   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
-----------------------------------------------------------
   NA       1   2   4    5   8  16 100 100 400 100    800
  % Range: 93.2-95.2%  Optimum: 97.2%  Hit Frequency: 30.1%
     Programs Available: PP0085
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0085_539-530.u68",   0x00000, 0x8000, CRC(f5325205) SHA1(737b25567633eca65ece42601ab0f3cf264fe150) ) /* Game Version: 539, Library Version: 530, Video Lib Ver: 530 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0089 ) /* Normal board : Standard Draw Poker (PP0089) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0089
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0089_554-544.u68",   0x00000, 0x8000, CRC(1d3e1b84) SHA1(354cfb5c00c9a4c9779bb56ff4541e58cedd442e) ) /* Game Version: 554, Library Version: 544, Video Lib Ver: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0103 ) /* Normal board : Deuces Wild Poker 1-100 Coin (PP0103) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P47A      1    2    2   3   4  13  16  25 200 250    800
  % Range: 92.8-94.8%  Optimum: 96.8%  Hit Frequency: 44.9%
     Programs Available: PP0103

This set erroneously swapped the intended payout of 5K & RF-with Deuce,
  with this set the 5K pays 25 while the RF-with Deuce pays 16

Superseded by PP0224 (Non Double-up) / PP0290

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0103_600-550.u68",   0x00000, 0x8000, CRC(1a9cc3ee) SHA1(55ee93cbfc90f517368a13fb71f5e50d575d703e) ) /* Game Version: 600, Library Version: 550, Video Lib Ver: 550 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) )
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0116 ) /* Normal board : Standard Draw Poker (PP0116) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0116
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0116_554-544.u68",   0x00000, 0x8000, CRC(27aba06b) SHA1(7976a2b2577c28e332091cbbcb4c7d53ffbea827) ) /* Game Version: 554, Library Version: 544, Video Lib Ver: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0116a ) /* Normal board : Standard Draw Poker (PP0116) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0116
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0116_554-544.u68",   0x00000, 0x8000, CRC(27aba06b) SHA1(7976a2b2577c28e332091cbbcb4c7d53ffbea827) ) /* Game Version: 554, Library Version: 544, Video Lib Ver: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg864.u72",   0x00000, 0x8000, CRC(f4ac28b9) SHA1(35c8a603120f35bd92905e0491e9ae5cd201e13f) ) /*  09/13/89   @ IGT  L89-1706  */
	ROM_LOAD( "mgo-cg864.u73",   0x08000, 0x8000, CRC(da8efcb9) SHA1(942c6b613074c52f6ed1c2fce78d46ef0f221c48) ) /* Custom Mirage casino card backs */
	ROM_LOAD( "mbo-cg864.u74",   0x10000, 0x8000, CRC(d48ac9cd) SHA1(a62d6c1daf199856aa1777d1d99fe81399215e36) ) /* These graphics will work for many other standard poker sets */
	ROM_LOAD( "mxo-cg864.u75",   0x18000, 0x8000, CRC(04f81245) SHA1(055271c6c502fad3be5f2d694a94f96bf3176404) ) /* However there is no support for Deuces Wild sets */

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap864.u50", 0x0000, 0x0100, CRC(c80e5743) SHA1(edf4e5a68905cc566077613d856bc90b8136a227) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0118 ) /* Normal board : Standard Draw Poker (PP0118) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K  SF  RF  (Bonus)
---------------------------------------------------------
  CA        1    2    3   4    6   9  25  50 250    800
  % Range: 95.5-97.5%  Optimum: 99.5%  Hit Frequency: 45.5%
     Programs Available: PP0118
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0118_554-544.u68",   0x00000, 0x8000, CRC(4025cb30) SHA1(742bfba5dbd8a3e38665045f84fd90e19e94d1f5) ) /* Game Version: 554, Library Version: 544, Video Lib Ver: 544 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0120 ) /* Normal board : Wild Sevens Poker (PP0120) */
/*
                                        w/7 Four w/o7
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  7s   RF  (Bonus)
-------------------------------------------------------------
  ????      1    2    3   4   4   9  12  20 200  250    800
  % Range: 93.1-95.1%  Optimum: 97.1%  Hit Frequency: 44.1%
     Programs Available: PP0120

Same payout as P59A (PP0124 / X000124P, Deuces Wild Poker) just swapping Sevens for Deuces

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0120_576-569.u68",   0x00000, 0x8000, CRC(4491be19) SHA1(cf8146a6ade1abb2e1ac9f0f3923e2be865f2fec) ) /* Game Version: 576, Library Version: 569, Video Lib Ver: 569 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) )
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0125 ) /* Normal board : Deuces Wild Poker (PP0125) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P62A      1    2    2   3   5   9  15  25 200 250    800
  % Range: 94.9-96.9%  Optimum: 98.9%  Hit Frequency: 44.4%
     Programs Available: PP0125, PP0418, X000291P, PP0291 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0125_600-550.u68",   0x00000, 0x8000, CRC(52a02dd4) SHA1(ae0dc8920a2a0bce2c19eb499af9b30b4552f53c) ) /* Game Version: 600, Library Version: 550, Video Lib Ver: 550 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) )
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0126 ) /* Normal board : Deuces Wild Poker (PP0126) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P63A      1    2    2   3   5   9  12  20 200 250    800
  % Range: 94.9-96.9%  Optimum: 98.9%  Hit Frequency: 45.4%
     Programs Available: PP0126, X000126P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0126_961-984.u68",   0x00000, 0x10000, CRC(aadb62ef) SHA1(85979d5932ef254241c363414c2093cc943e88a5) ) /* Game Version: 961, Library Version: 984 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0127 ) /* Normal board : Deuces Joker Wild Poker (PP0127) */
/*
                                         With  w/o  w/o  With
                                         Wild  JKR  Wild JKR
PayTable   3K   STR  FL  FH  4K  SF  5K   RF    4D   RF   4D  (Bonus)
---------------------------------------------------------------------
  P65N      1    2    3   3   3   6   9   12    25  250  1000  2000
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 50.4%
     Programs Available: PP0127
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0127_a47-a76.u68",   0x00000, 0x10000, CRC(2997aaac) SHA1(b52525154f4ae39a341ecf829c33449f31a8ce07) ) /* Game Version: A47, Library Version: A76 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0127a ) /* Normal board : Deuces Joker Wild Poker (PP0127) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
                                         With  w/o  w/o  With
                                         Wild  JKR  Wild JKR
PayTable   3K   STR  FL  FH  4K  SF  5K   RF    4D   RF   4D  (Bonus)
---------------------------------------------------------------------
  P65N      1    2    3   3   3   6   9   12    25  250  1000  2000
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 50.4%
     Programs Available: PP0127
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0127_999-a1a.u68",   0x00000, 0x10000, CRC(df09abd2) SHA1(bc8c6f01b3387c0d10ec380ec86f26673776bfb2) ) /* Game Version: 999, Library Version: A1A */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0130 ) /* Normal board : Aces & Faces (PP0130) */
/*

                                      2-10 J-K
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  ????      1    2    3    4    5   8  25  40  80  50 250    800
  % Range: 95.3-97.3%  Optimum: 99.3%  Hit Frequency: 45.5%
     Programs Available: PP0130
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0130_a4y-a6g.u68",   0x00000, 0x10000, CRC(bf9293f2) SHA1(79f5247a2d5447c89e281c618b09c7f7790176a2) ) /* Game Version: A4Y, Library Version: A6G */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg954.u72",   0x00000, 0x8000, CRC(1f9cd61e) SHA1(92a2ca3765ad4eeb7ab96538d4278e0a99d16638) )
	ROM_LOAD( "mgo-cg954.u73",   0x08000, 0x8000, CRC(ac1dd15d) SHA1(41ddbb05edc3d274a27d4938c29e6a1e7c785cd7) )
	ROM_LOAD( "mbo-cg954.u74",   0x10000, 0x8000, CRC(cfcb5740) SHA1(8a94536f3b4315c1e6a16a8e5043a80205a3aabe) )
	ROM_LOAD( "mxo-cg954.u75",   0x18000, 0x8000, CRC(94f63723) SHA1(f53867cc1c07235ff2aba1854459085dd0643c89) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap953.u50", 0x0000, 0x0100, CRC(6ece50ad) SHA1(bc5761303b09625850ba50263607d11871ea3ed3) )
ROM_END

ROM_START( pepp0132 ) /* Normal board : Standard Draw Poker (PP0132) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K  SF  RF  (Bonus)
---------------------------------------------------------
  CA        1    2    3   4    6   9  25  50 250    800
  % Range: 95.5-97.5%  Optimum: 99.5%  Hit Frequency: 45.5%
     Programs Available: PP0132, PP0447, X000447P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0132_896-914.u68",   0x00000, 0x10000, CRC(ad888692) SHA1(cbc78c546b8f4dc136cc376a0b7aed10faacbac6) ) /* Game Version: 896, Library Version: 914 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0150 ) /* Normal board : Standard Draw Poker (PP0150) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   KK       1    2    3    4    6   9  25  50 500    500
  % Range: 94.9-96.9%  Optimum: 98.9%  Hit Frequency: 45.5%
     Programs Available: PP0150, X000150P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0150_a45-a74.u68",   0x00000, 0x10000, CRC(8848dc4b) SHA1(121e885d253aa3c2a72de9e14d64e20d794e53bf) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0158 ) /* Normal board : 4 of a Kind Bonus Poker (PP0158) - 03/17/97   @ IGT  L97-0628 */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P77A      1    2    3    4    5   8  25  40  80  50 250    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.5%
     Programs Available: PP0158, X000158P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0158_a67-a8k.u68",   0x00000, 0x10000, CRC(715aeadf) SHA1(e90b1f0a1d4886882c9259d84c950076f9fd521d) ) /* Game Version: A67, Library Version: A8K */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2108.u72",   0x00000, 0x8000, CRC(4d2d2223) SHA1(4672d4302697cfa2e6d826f79cc3fa1bdfbd8315) ) /*  11/29/94   @ IGT  L94-2198  */
	ROM_LOAD( "mgo-cg2108.u73",   0x08000, 0x8000, CRC(74a75c2c) SHA1(0cdac698685849696402d289dcdc82df2aae7e49) )
	ROM_LOAD( "mbo-cg2108.u74",   0x10000, 0x8000, CRC(3f364248) SHA1(5111aa57ff8698dceb55b9d10f3acab7786c7da6) )
	ROM_LOAD( "mxo-cg2108.u75",   0x18000, 0x8000, CRC(b04d317b) SHA1(5c181fb0a58b216db511572e531cd0eea7a061f4) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap944.u50", 0x0000, 0x0100, CRC(8700bc0a) SHA1(71b0bea067fb4885b19145146149eafd01d87ad0) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0158a ) /* Normal board : 4 of a Kind Bonus Poker (PP0158) - 10/23/95   @ IGT  L95-2438 */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P77A      1    2    3    4    5   8  25  40  80  50 250    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.5%
     Programs Available: PP0158, X000158P

NOTE: This set also found with CG2003+CAP904

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0158_a46-a75.u68",   0x00000, 0x10000, CRC(5976cd19) SHA1(6a461ea9ddf78dffa3cf8b65903ebf3127f23d45) ) /* Game Version: A46, Library Version: A75, Video Lib ver A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1047.u72",  0x00000, 0x8000, CRC(a299d058) SHA1(ed090bacac07c4cf85eacc80f15489b4c18b33d3) ) /* Custom Skyline Casino card backs */
	ROM_LOAD( "mgo-cg1047.u73",  0x08000, 0x8000, CRC(d9aa8951) SHA1(c42a256368871a38ceaca8b66256e235dbe2adad) )
	ROM_LOAD( "mbo-cg1047.u74",  0x10000, 0x8000, CRC(5051fb59) SHA1(159559d6e40287e34b1538c51e779d22b910710d) ) /* These graphics will work for many other standard poker sets */
	ROM_LOAD( "mxo-cg1047.u75",  0x18000, 0x8000, CRC(2f86cf5a) SHA1(0a271f3d2353303fbab80c61fec8e2dda877c42b) ) /* For Deuces Wild sets use CG1276 + CAP1276 */

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1046.u50", 0x0000, 0x0100, CRC(883fa6a4) SHA1(76aa42912d3180dc0466be95f30d6d760996713b) ) /* Uses CAP1046 as stated or really CAP1047?? */
ROM_END

ROM_START( pepp0158b ) /* Normal board : 4 of a Kind Bonus Poker (PP0158) - 04/16/94   @ IGT  L94-1044 */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P77A      1    2    3    4    5   8  25  40  80  50 250    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.5%
     Programs Available: PP0158, X000158P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0158_a0n-a23.u68",   0x00000, 0x10000, CRC(f3f9b6da) SHA1(1aedcb5257890c52c633357f8b96e72fe51158f8) ) /* Game Version: A0N, Library Version: A23, Video Lib ver A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0158c ) /* Normal board : 4 of a Kind Bonus Poker (PP0158) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P77A      1    2    3    4    5   8  25  40  80  50 250    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.5%
     Programs Available: PP0158, X000158P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0158_813-824.u68",   0x00000, 0x10000, CRC(b82cec15) SHA1(6ff8867e88d57cd1874388c9e6f5e0e6d96029e9) ) /* Game Version: 813, Library Version: 824 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0158d ) /* Normal board : 4 of a Kind Bonus Poker (PP0158) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P77A      1    2    3    4    5   8  25  40  80  50 250    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.5%
     Programs Available: PP0158, X000158P

NOTE: While this is a 32K version, it does require DOOR OPEN cycling and isn't compatible with earlier non-plus boards
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0158_631-607.u68",   0x00000, 0x8000, CRC(5fe3498c) SHA1(f1405bf016d46904228cda88d8d94e2a956b2347) ) /* Game Version: 631, Library Version: 607, Video Lib Ver: 607 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0159 ) /* Normal board : Standard Draw Poker (PP0159) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11B      1    2    3    4    5   7  25  50 250   1000
  % Range: 92.5-94.5%  Optimum: 96.5%  Hit Frequency: 45.5%
     Programs Available: PP0159
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0159_a2l-a4g.u68",   0x00000, 0x10000, CRC(22a5aa5f) SHA1(dfad528825710e887f36c201dc2dca2ab162f0f8) ) /* Game Version: A2L, Library Version: A4G */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0171 ) /* Normal board : Joker Poker  (PP0171) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
   YA       1    1   2   3    5   7  15  50 100 200 400    800
  % Range: 92.4-94.4%  Optimum: 96.4%  Hit Frequency: 44.2%
     Programs Available: PP0171, X000171P
*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /*  10/23/95   @IGT  L95-2281  */
	ROM_LOAD( "pp0171_a45-a74.u68",   0x00000, 0x10000, CRC(7a68ee4b) SHA1(298ca0c87229929b61ddfdf8c0bac82e9df17e83) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0171a ) /* Normal board : Joker Poker  (PP0171) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
   YA       1    1   2   3    5   7  15  50 100 200 400    800
  % Range: 92.4-94.4%  Optimum: 96.4%  Hit Frequency: 44.2%
     Programs Available: PP0171, X000171P
*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /*  10/23/95   @IGT  L95-2281  */
	ROM_LOAD( "pp0171_a2e-a49.u68",   0x00000, 0x10000, CRC(efd6a33a) SHA1(72f5a4c9923f46a59a61b3b034b1275ebfeadac6) ) /* Game Version: A2E, Library Version: A49 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0178 ) /* Normal board : 4 of a Kind Bonus Poker w/ operator selectable special 4 of a Kind (PP0178) */
/*
                                          Aor?
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K*  SF  RF  (Bonus)
-----------------------------------------------------------------
  ????      1    2    3    4    6   9  25  25   50 250    800

* Operator selectable Special 4 of a Kind. Maxbet payout is 250 same as SF at Maxbet

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0178_645-627.u68",   0x00000, 0x10000, CRC(96cb3b13) SHA1(d8b0621e48b20142d25bf81d07d6716d9858f33e) ) /* Game Version: 645, Library Version: 627 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg954.u72",   0x00000, 0x8000, CRC(1f9cd61e) SHA1(92a2ca3765ad4eeb7ab96538d4278e0a99d16638) )
	ROM_LOAD( "mgo-cg954.u73",   0x08000, 0x8000, CRC(ac1dd15d) SHA1(41ddbb05edc3d274a27d4938c29e6a1e7c785cd7) )
	ROM_LOAD( "mbo-cg954.u74",   0x10000, 0x8000, CRC(cfcb5740) SHA1(8a94536f3b4315c1e6a16a8e5043a80205a3aabe) )
	ROM_LOAD( "mxo-cg954.u75",   0x18000, 0x8000, CRC(94f63723) SHA1(f53867cc1c07235ff2aba1854459085dd0643c89) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap953.u50", 0x0000, 0x0100, CRC(6ece50ad) SHA1(bc5761303b09625850ba50263607d11871ea3ed3) )
ROM_END

ROM_START( pepp0181 ) /* Normal board : Standard Draw Poker (PP0181) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0181
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0181_648-667.u68",   0x00000, 0x10000, CRC(b38ff1e1) SHA1(ae8d725a3352000c57cef4b7e7a39dbad940e9de) ) /* Game Version: 648, Library Version: 667 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0188 ) /* Normal board : Standard Draw Poker (PP0188) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P40A      1    2    3    4    6   9  15  40 250    800
  % Range: 86.5-88.5%  Optimum: 92.5%  Hit Frequency: 45.5%
     Programs Available: PP0188, X000188P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0188_986-a0u.u68",   0x00000, 0x10000, CRC(cf36a53c) SHA1(99b578538ab24d9ff91971b1f77599272d1dbfc6) ) /* Game Version: 986, Library Version: A0U */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0188a ) /* Normal board : Standard Draw Poker (PP0188) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P40A      1    2    3    4    6   9  15  40 250    800
  % Range: 86.5-88.5%  Optimum: 92.5%  Hit Frequency: 45.5%
     Programs Available: PP0188, X000188P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0188_a66-a8h.u68",   0x00000, 0x10000, CRC(72740894) SHA1(5b7109b6cbe67e0a951fd48b4daf09875abb75fc) ) /* Game Version: A66, Library Version: A8H */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0190 ) /* Normal board : Deuces Wild Poker (No Double-up) (PP0190) - 11/13/95   @ IGT  L99-0100 */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P57A      1    2    3   4   4   8  10  20 200 250    800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.5%
     Programs Available: PP0417, X000417P, X000190P & PP0190 - Non Double-up Only

NOTE: This later build works with CG1215 (and later Deuces Wild) graphics roms and is NOT compatible with CG773
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0190_a47-a76.u68",   0x00000, 0x10000, CRC(974f9d7a) SHA1(8fe65c568246fbf97b20cd2b05cccb23022dff65) ) /* Game Version: A47, Library Version: A76 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0190a ) /* Normal board : Deuces Wild Poker (No Double-up) (PP0190) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P57A      1    2    3   4   4   8  10  20 200 250    800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.5%
     Programs Available: PP0417, X000417P, X000190P & PP0190 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0190_642-623.u68",   0x00000, 0x10000, CRC(3b8a3203) SHA1(33bd285def754df34f4815cd713e2ff599c74f11) ) /* Game Version: 642, Library Version: 623 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) ) /* Should be MxO-CG2023.U7x & CAP773 (or CAP2023)?? */
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0190b ) /* Normal board : Deuces Wild Poker  (PP0190) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P57A      1    2    3   4   4   8  10  20 200 250    800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.5%
     Programs Available: PP0417, X000417P, X000190P & PP0190 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0190_a56-a7a.u68",   0x00000, 0x10000, CRC(26900a42) SHA1(e15537c1dff097b99d3b21801243289872330a2a) ) /* Game Version: A56, Library Version: A7A */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0197 ) /* Normal board : Standard Draw Poker (PP0197) - 10/23/95   @ IGT  L95-2452 */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0197, X000197P & PP0419 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0197_a45-a75.u68",   0x00000, 0x10000, CRC(6b5b3108) SHA1(2fafcf979db92d4f9f1fb0a2e9645fd71d8dd5c2) ) /* Game Version: A45, Library Version: A75 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0197a ) /* Normal board : Standard Draw Poker (PP0197) - 07/29/96   @ IGT  L96-1219 */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0197, X000197P & PP0419 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0197_a14-a2n.u68",   0x00000, 0x10000, CRC(ef472672) SHA1(785ce02b13894e5cb7575e75533451b96e3f4e6d) ) /* Game Version: A14, Library Version: A2N */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0197b ) /* Normal board : Standard Draw Poker (PP0197) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0197, X000197P & PP0419 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0197_979-a0c.u68",   0x00000, 0x10000, CRC(ae817534) SHA1(b2454609e8275aab00797966c0f4e68eae2911cd) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0197c ) /* Normal board : Standard Draw Poker (Auto Hold in options) (PP0197) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0197, X000197P & PP0419 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0197_689-654.u68",   0x00000, 0x10000, CRC(84a2fbde) SHA1(0515f2693d388e29cdf0de4d29708250c671e0a4) ) /* Game Version: 689, Library Version: 654 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0203 ) /* Normal board : 4 of a Kind Bonus Poker (PP0203) - 10/23/95   @ IGT  L95-2446 */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P90A      1    2    3    4    5   7  25  40  80  50 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.5%
     Programs Available: PP0203, X000203P, PP0590 & PP0409 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0203_a46-a75.u68",   0x00000, 0x10000, CRC(2955eeb5) SHA1(ac2483dbb92de84ab64d0d7e55acff196966ea1b) ) /* Game Version: A46, Library Version: A75, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0203a ) /* Normal board : 4 of a Kind Bonus Poker (PP0203) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P90A      1    2    3    4    5   7  25  40  80  50 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.5%
     Programs Available: PP0203, X000203P, PP0590 & PP0409 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0203_a0n-a23.u68",   0x00000, 0x10000, CRC(fce4ea36) SHA1(4c1be0cb3600bbcac768b942f7b8bddd5d626ef6) ) /* Game Version: A0N, Library Version: A23, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )


	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0203b ) /* Normal board : 4 of a Kind Bonus Poker (PP0203) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P90A      1    2    3    4    5   7  25  40  80  50 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.5%
     Programs Available: PP0203, X000203P, PP0590 & PP0409 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0203_813-824.u68",   0x00000, 0x10000, CRC(49ea6fb9) SHA1(ca595e30d786d28397eeef10786a811af1a5c74d) ) /* Game Version: 813, Library Version: 824 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0203c ) /* Normal board : 4 of a Kind Bonus Poker (PP0203) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P90A      1    2    3    4    5   7  25  40  80  50 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.5%
     Programs Available: PP0203, X000203P, PP0590 & PP0409 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0203_a35-a51.u68",   0x00000, 0x10000, CRC(7d38b599) SHA1(aee2f347c00c240e1dc0b662800708b1038d3ec8) ) /* Game Version: A35, Library Version: A51 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0203d ) /* Normal board : 4 of a Kind Bonus Poker (PP0203) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P90A      1    2    3    4    5   7  25  40  80  50 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.5%
     Programs Available: PP0203, X000203P, PP0590 & PP0409 - Non Double-up Only

NOTE: While this is a 32K version, it does require DOOR OPEN cycling and isn't compatible with earlier non-plus boards
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0203_631-607.u68",   0x00000, 0x8000, CRC(ad61ee10) SHA1(477d1b17c368ea194a460e839d7de4c2a9a256a4) ) /* Game Version: 631, Library Version: 607, Video Lib Ver: 607 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0219 ) /* Normal board : Standard Draw Poker - Auto Hold in Options (PP0219) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   GA       1    2    3    4    5   6  25  50 250    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0219
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0219_689-654.u68",   0x00000, 0x10000, CRC(2cd5cd21) SHA1(614264f0e346146420b44ebe9dc93b0799a70b5d) ) /* Game Version: 689, Library Version: 654 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg904.u72",   0x00000, 0x8000, CRC(75bac43f) SHA1(8e7bfba95aa6e027cdaf0d1535e5c630ee2f56d3) ) /* 02/26/90  @IGT  INT */
	ROM_LOAD( "mgo-cg904.u73",   0x08000, 0x8000, CRC(1222d844) SHA1(854eb790d5b5a5cfe8148e8b95e3ba3f06f33dce) )
	ROM_LOAD( "mbo-cg904.u74",   0x10000, 0x8000, CRC(0bf0168f) SHA1(254cef934a0d30c5a18a0b4773bb364fc21f8113) )
	ROM_LOAD( "mxo-cg904.u75",   0x18000, 0x8000, CRC(ff648f12) SHA1(58f8247a997e1b0b69bafed428d30822adef339e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0221 ) /* Normal board : Standard Draw Poker (PP0221) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0221, PP0449, X000449P & PP0585 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0221_979-a0c.u68",   0x00000, 0x10000, CRC(c45fb8f1) SHA1(fba8dce2954beb168624ba94b2a4fdd3b260da46) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0221a ) /* Normal board : Standard Draw Poker (PP0221) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0221, PP0449, X000449P & PP0585 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0221_896-914.u68",   0x00000, 0x10000, CRC(14d50334) SHA1(281c4467f57f91d0da98242b085973c06193085a) ) /* Game Version: 896, Library Version: 914 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0223 ) /* Normal board : Deuces Joker Wild Poker (PP0223) */
/*
                                         With  w/o  w/o  With
                                         Wild  JKR  Wild JKR
PayTable   3K   STR  FL  FH  4K  SF  5K   RF    4D   RF   4D  (Bonus)
---------------------------------------------------------------------
  P76N      1    1    3   3   3   6   9   12    25  250  1000  2000
  % Range: 89.6-91.6%  Optimum: 93.6%  Hit Frequency: 50.3%
     Programs Available: PP0223

Internally the program erroneously reports a 92.80% return. Superseded by PP0812
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0223_984-a0r.u68",   0x00000, 0x10000, CRC(7254255a) SHA1(345d5d567bcd2af9b374a3345a10f9999c34b2b5) ) /* Game Version: 984, Library Version: A0R */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0224 ) /* Normal board : Deuces Wild Poker (No Double-up) (PP0224) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P47A      1    2    2   3   4  13  16  25 200 250    800
  % Range: 92.8-94.8%  Optimum: 96.8%  Hit Frequency: 44.9%
     Programs Available: PP0290, X000224P & PP0224 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0224_a47-a76.u68",   0x00000, 0x10000, CRC(5d6881ad) SHA1(38953ffadea04df614b14c70177736039495c408) ) /* Game Version: A47, Library Version: A76 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0224a ) /* Normal board : Deuces Wild Poker (No Double-up) (PP0224) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P47A      1    2    2   3   4  13  16  25 200 250    800
  % Range: 92.8-94.8%  Optimum: 96.8%  Hit Frequency: 44.9%
     Programs Available: PP0290, X000224P & PP0224 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0224_961-984.u68",   0x00000, 0x10000, CRC(71d5e112) SHA1(528f06ad7ea7e1e297939c7b3ca0bb7faa8ce8c1) ) /* Game Version: 961, Library Version: 984 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0230 ) /* Normal board : Standard Draw Poker (PP0230) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0230
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0230_715-702.u68",   0x00000, 0x10000, CRC(0272d8d6) SHA1(659f9149ab5e26283eaccd31588183c21c291adb) ) /* Game Version: 715, Library Version: 702 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0242 ) /* Normal board : Deuces Wild Poker (PP0242) - Multi Regional / Multi Currency in English / Spanish - Tournament Mode capable */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P56A      1    2    3   3   4   8  10  20 200 250    800
  % Range: 89.4-91.4%  Optimum: 93.4%  Hit Frequency: 45.1%
     Programs Available: PP0242, X000242P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0242_a1c-a31.u68",   0x00000, 0x10000, CRC(cb7cdf2b) SHA1(989db6bf860637ef0c9d38c4ec824e2ab92acb89) ) /* Game Version: A1C, Library Version: A31 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1325.u72",  0x00000, 0x8000, CRC(ebb81436) SHA1(211cc0f881703b3cceb51c65209075154c9536db) )
	ROM_LOAD( "mgo-cg1325.u73",  0x08000, 0x8000, CRC(ef86e83a) SHA1(854fc31173c7647a9ed986f2fe58ec3795eb8542) )
	ROM_LOAD( "mbo-cg1325.u74",  0x10000, 0x8000, CRC(8387b4ba) SHA1(cab77982464e9e70e6ad4ecf51a5cafe7aefb478) )
	ROM_LOAD( "mxo-cg1325.u75",  0x18000, 0x8000, CRC(9dddc501) SHA1(a0ab8b3866b0ae018b3f6e0199bdc756d4e5f967) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0249 ) /* Normal board : Deuces Wild Poker (PP0249) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P129A     1    2    3   4   4  10  16  25 200 250    800
  % Range: 95.7-97.7%  Optimum: 99.7%  Hit Frequency: 44.3%
     Programs Available: PP0249

Superseded by PP0469

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0249_600-550.u68",   0x00000, 0x8000, CRC(39cab612) SHA1(6ef2e533df40d9ac331dc9e2fd3bf17187f70414) ) /* Game Version: 600, Library Version: 550, Video Lib Ver: 550 */
	ROM_RELOAD(                       0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) )
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0250 ) /* Normal board : Double Down Stud Poker (PP0250) */
/*
PayTable  6s-10s  Js+  2PR  3K   STR  FL  FH  4K  SF   RF  (Bonus)
------------------------------------------------------------------
  ????      1      2    3    4    6    9  12  50 200  1000  4000
  % Range: 94.3-96.3%  Optimum: 98.3%  Hit Frequency: ??.?%
     Programs Available: PP0250

NOTE: Newer version with DBV support and requires newer CG2015 instead of CG1019
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0250_a1l-a23.u68",   0x00000, 0x10000, CRC(ae4f1fb8) SHA1(473d1acb8549f86b9da17f9fbbceafc3a3efc6fe) ) /* Game Version: A1L, Library Version: A23 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2015.u72",   0x00000, 0x8000, CRC(7f73ee5c) SHA1(b6c5d423c8176555c1f32605c328ffbfcf94b656) ) /* Verified CG set for this version of PP0250 */
	ROM_LOAD( "mgo-cg2015.u73",   0x08000, 0x8000, CRC(de270e0e) SHA1(41b207f9380f623ab64dc42224275cccd43417ee) )
	ROM_LOAD( "mbo-cg2015.u74",   0x10000, 0x8000, CRC(02e623d9) SHA1(4c689293f5c5a8eb0b17861cf433ae1e01d83545) )
	ROM_LOAD( "mxo-cg2015.u75",   0x18000, 0x8000, CRC(0c96b7fc) SHA1(adde93f08db0b957daf77d57a7ab60af3b667f25) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0250a ) /* Normal board : Double Down Stud Poker (PP0250) */
/*
PayTable  6s-10s  Js+  2PR  3K   STR  FL  FH  4K  SF   RF  (Bonus)
------------------------------------------------------------------
  ????      1      2    3    4    6    9  12  50 200  1000  4000
  % Range: 94.3-96.3%  Optimum: 98.3%  Hit Frequency: ??.?%
     Programs Available: PP0250

NOTE: No DBV option and requires CG1019
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0250_733-778.u68",   0x00000, 0x10000, CRC(4c919598) SHA1(fe73503c6ccb3c5746fb96be58cd5b740c819713) ) /* Game Version: 733, Library Version: 778 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1019.u72",   0x00000, 0x8000, CRC(9086dc3c) SHA1(639baef8a9b347015d21817d69265700ff205774) ) /* Verified CG set for this version of PP0250 */
	ROM_LOAD( "mgo-cg1019.u73",   0x08000, 0x8000, CRC(fb538a19) SHA1(1ed480ebdf3ad210511e7e0a0dd4e28466219ae9) ) /* Superseded by CG2015 which also works */
	ROM_LOAD( "mbo-cg1019.u74",   0x10000, 0x8000, CRC(493bf604) SHA1(9cbce26ed328e6878ec5f6531ea140e1c17e6753) )
	ROM_LOAD( "mxo-cg1019.u75",   0x18000, 0x8000, CRC(064a5c80) SHA1(4d21a7a424258f74d4a1e78c123288799e316228) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0265 ) /* Normal board : 4 of a Kind Bonus Poker (PP0265) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P101A      1    2    3    4    5   6  25  40  80  50 250    800
  % Range: 92.5-94.5%  Optimum: 96.9%  Hit Frequency: 45.5%
     Programs Available: PP0265, X000265P, PP0403 & PP0410 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0265_a46-a75.u68",   0x00000, 0x10000, CRC(dccb5e2f) SHA1(4c1ff0f79d9441d0b7e8b31f95def1056945eb96) ) /* Game Version: A46, Library Version: A75, Video Lib ver A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0265a ) /* Normal board : 4 of a Kind Bonus Poker (PP0265) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P101A      1    2    3    4    5   6  25  40  80  50 250    800
  % Range: 92.5-94.5%  Optimum: 96.9%  Hit Frequency: 45.5%
     Programs Available: PP0265, X000265P, PP0403 & PP0410 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0265_a0n-a23.u68",   0x00000, 0x10000, CRC(7dad6907) SHA1(890bdafb8bf272e513e94c8016c9866ab39f8fa2) ) /* Game Version: A0N, Library Version: A23, Viedo Lib ver A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0265b ) /* Normal board : 4 of a Kind Bonus Poker (PP0265) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P101A      1    2    3    4    5   6  25  40  80  50 250    800
  % Range: 92.5-94.5%  Optimum: 96.9%  Hit Frequency: 45.5%
     Programs Available: PP0265, X000265P, PP0403 & PP0410 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0265_987-a0v.u68",   0x00000, 0x10000, CRC(471e59c0) SHA1(02aedff3feaa6fe88cd6eebbd3cd335ae1240228) ) /* Game Version: 987, Library Version: A0V */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0274 ) /* Normal board : Standard Draw Poker (PP0274) - Normal Poker & Tournament Mode capable */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K  SF  RF  (Bonus)
---------------------------------------------------------
  CA        1    2    3   4    6   9  25  50 250    800
  % Range: 95.5-97.5%  Optimum: 99.5%  Hit Frequency: 45.5%
     Programs Available: PP0274
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0274_741-728.u68",   0x00000, 0x10000, CRC(cd0b2890) SHA1(5f859dbc8d747a198c735a2bff42279c874928b0) ) /* Game Version: 741, Library Version: 728, Viedo Lib ver: 728 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0288 ) /* Normal board : Standard Draw Poker (PP0288) - Spanish */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   KK       1    2    3    4    6   9  25  50 500    500
  % Range: 94.9-96.9%  Optimum: 98.9%  Hit Frequency: 45.5%
     Programs Available: PP0288

Spanish version of PP0150

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0288_964-988.u68",   0x00000, 0x10000, CRC(f7e1cb4c) SHA1(651c4306764200611aae7280ce0a9756d42ccb21) ) /* Game Version: 964, Library Version: 988 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1152.u72",   0x00000, 0x8000, CRC(55aca7f6) SHA1(9a0b25908e4fee8346da4726f38993e233a2cb2f) )
	ROM_LOAD( "mgo-cg1152.u73",   0x08000, 0x8000, CRC(c5185314) SHA1(7fc385d2b44e68364f2b5a3702737c5d5fea5ea2) )
	ROM_LOAD( "mbo-cg1152.u74",   0x10000, 0x8000, CRC(bd19aef6) SHA1(77126ea73d0a5da4b8856b5ebcbbab84cfaef008) )
	ROM_LOAD( "mxo-cg1152.u75",   0x18000, 0x8000, CRC(d65f7362) SHA1(d56fa0d13126f1599d538c81a2c7ea1f3c94b62f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0290 ) /* Normal board : Deuces Wild Poker (PP0290) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P47A      1    2    2   3   4  13  16  25 200 250    800
  % Range: 92.8-94.8%  Optimum: 96.8%  Hit Frequency: 44.9%
     Programs Available: PP0290, X000224P & PP0224 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0290_a0v-a2d.u68",   0x00000, 0x10000, CRC(e2e6451c) SHA1(fa83b7a6d6c4ba1b3ee95b48ab6c3594477e536d) ) /* Game Version: A0V, Library Version: A2D */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0291 ) /* Normal board : Deuces Wild Poker (No Double-up) (PP0291) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P62A      1    2    3   4   4   9  15  25 200 250    800
  % Range: 94.9-96.9%  Optimum: 98.9%  Hit Frequency: 44.4%
     Programs Available: PP0125, PP0418, X000291P & PP0291 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0291_a0v-a2d.u68",   0x00000, 0x10000, CRC(4eabac97) SHA1(dc849bca8ac90536c361cd576ee81c50afd7071b) ) /* Game Version: A0V, Library Version: A2D */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0291a ) /* Normal board : Deuces Wild Poker (No Double-up) (PP0291) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P62A      1    2    3   4   4   9  15  25 200 250    800
  % Range: 94.9-96.9%  Optimum: 98.9%  Hit Frequency: 44.4%
     Programs Available: PP0125, PP0418, X000291P & PP0291 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0291_961-984.u68",   0x00000, 0x10000, CRC(64c83e70) SHA1(1c75c7c37a359d07b170ab644ebb98d4bce2affe) ) /* Game Version: 961, Library Version: 984 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0401 ) /* Normal board : 4 of a Kind Bonus Poker (No Double-up) (PP0401) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P90A      1    2    3    4    5   8  25  40  80  50 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.5%
     Programs Available: PP0401
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0401_789-812.u68",   0x00000, 0x10000, CRC(caad4496) SHA1(cecb469d318f25a3ad49eb0014b57f61e54183c6) ) /* Game Version: 789, Library Version: 812 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0409 ) /* Normal board : 4 of a Kind Bonus Poker (No Double-up) (PP0409) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P90A      1    2    3    4    5   7  25  40  80  50 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.5%
     Programs Available: PP0203, X000203P, PP0590 & PP0409 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0409_a45-a75.u68",   0x00000, 0x10000, CRC(a71fdad9) SHA1(a719787895ad035b2b930d2692930466a9bfb19d) ) /* Game Version: A46, Library Version: A75, Game Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0410 ) /* Normal board : 4 of a Kind Bonus Poker (No Double-up) (PP0410) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P101A      1    2    3    4    5   6  25  40  80  50 250    800
  % Range: 92.5-94.5%  Optimum: 96.9%  Hit Frequency: 45.5%
     Programs Available: PP0265, X000265P, PP0403 & PP0410 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0410_a46-a75.u68",   0x00000, 0x10000, CRC(41e904ae) SHA1(b63a47c91b9f76f11bd14ccb16dbb4ac86fe7926) ) /* Game Version: A46, Library Version: A75, Game Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0410a ) /* Normal board : 4 of a Kind Bonus Poker (No Double-up) (PP0410) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P101A      1    2    3    4    5   6  25  40  80  50 250    800
  % Range: 92.5-94.5%  Optimum: 96.9%  Hit Frequency: 45.5%
     Programs Available: PP0265, X000265P, PP0403 & PP0410 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0410_a0n-a23.u68",   0x00000, 0x10000, CRC(474f4809) SHA1(8d88647ab4719fdcf6ec0800386f32574b1da66d) ) /* Game Version: A0N, Library Version: A23, Game Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0417 ) /* Normal board : Deuces Wild Poker (No Double-up) (PP0417) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P57A      1    2    3   4   4   8  10  20 200 250    800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.5%
     Programs Available: PP0417, X000417P & PP0190 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0417_a0v-a2d.u68",   0x00000, 0x10000, CRC(3681e606) SHA1(e8e9105247b144ce1050464cb6b0594c9e483f84) ) /* Game Version: A0V, Library Version: A2D */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2133.u72",   0x00000, 0x8000, CRC(b21a789f) SHA1(c49f9b5f51c29bbc0e1392e86d6602bd44e46380) ) /*  02/02/95   @ IGT  L95-0276  */
	ROM_LOAD( "mgo-cg2133.u73",   0x08000, 0x8000, CRC(2b7db148) SHA1(d5ff5dde3589d28937d13dc5c4c38caa1ebf2d56) )
	ROM_LOAD( "mbo-cg2133.u74",   0x10000, 0x8000, CRC(6ed455b7) SHA1(e4f223606c19d09be501461f38520f423599e0a2) ) /* Supersedes CG1215 graphics set */
	ROM_LOAD( "mxo-cg2133.u75",   0x18000, 0x8000, CRC(095ea26d) SHA1(9bdd8afe67da2370c4ca2d8418f3afdaf7b557ff) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0417a ) /* Normal board : Deuces Wild Poker (No Double-up) (PP0417) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P57A      1    2    3   4   4   8  10  20 200 250    800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.5%
     Programs Available: PP0417, X000417P & PP0190 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0417_961-984.u68",   0x00000, 0x10000, CRC(4a1e7899) SHA1(b6f243d275da70841482843e05c1be22fd80c25c) ) /* Game Version: 961, Library Version: 984 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2133.u72",   0x00000, 0x8000, CRC(b21a789f) SHA1(c49f9b5f51c29bbc0e1392e86d6602bd44e46380) ) /*  02/02/95   @ IGT  L95-0276  */
	ROM_LOAD( "mgo-cg2133.u73",   0x08000, 0x8000, CRC(2b7db148) SHA1(d5ff5dde3589d28937d13dc5c4c38caa1ebf2d56) )
	ROM_LOAD( "mbo-cg2133.u74",   0x10000, 0x8000, CRC(6ed455b7) SHA1(e4f223606c19d09be501461f38520f423599e0a2) ) /* Supersedes CG1215 graphics set */
	ROM_LOAD( "mxo-cg2133.u75",   0x18000, 0x8000, CRC(095ea26d) SHA1(9bdd8afe67da2370c4ca2d8418f3afdaf7b557ff) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0419 ) /* Normal board : Standard Draw Poker - Auto Hold in Options (No Double-up) (PP0419) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.3-95.3%  Optimum: 97.3%  Hit Frequency: 45.5%
     Programs Available: PP0197, X000197P & PP0419 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0419_768-761.u68",   0x00000, 0x10000, CRC(204f9ffe) SHA1(adfdc8b22ba5cc69234ad88ab2f92393a5fb3aff) ) /* Game Version: 768, Library Version: 761, Game Lib ver: 761 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0420 ) /* Normal board : Standard Draw Poker (No Double-up) (PP0420) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   GA       1    2    3    4    5   6  25  50 250    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0060, X000060P & PP0420 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0420_896-914.u68",   0x00000, 0x10000, CRC(cdd923d2) SHA1(f7548159ea3c36c3fce481156ab0293d00f0fd0f) ) /* Game Version: 896, Library Version: 914 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0423 ) /* Normal board : Standard Draw Poker (No Double-up) (PP0423) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0423
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0423_a45-a74.u68",   0x00000, 0x10000, CRC(b717bb0f) SHA1(89243bec0dc5b2c3907ef6579dfc3fdd28977971) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0423a ) /* Normal board : Standard Draw Poker (No Double-up) (PP0423) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0423
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0423_896-914.u68",   0x00000, 0x10000, CRC(c996c539) SHA1(3874c294f4d223596ab537634aaf52bc0494ff85) ) /* Game Version: 896, Library Version: 914 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0423b ) /* Normal board : Standard Draw Poker (No Double-up) (PP0423) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0423
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0423_836-847.u68",   0x00000, 0x10000, CRC(12eec557) SHA1(3721d068223262260ad22698e7dca0440becc53e) ) /* Game Version: 836, Library Version: 847 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg740.u72",   0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",   0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",   0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",   0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0426 ) /* Normal board : Joker Poker (No Double-up) (PP0426) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
   YD       1    1   1   3    5   7  15  50 100 200 400    940
  % Range: 92.7-94.7%  Optimum: 96.7%  Hit Frequency: 44.1%
     Programs Available: PP0568, X000568P & PP0426 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0426_979-a0c.u68",   0x00000, 0x10000, CRC(0dca4bf1) SHA1(f2cfe250e78fc0995dda653d231f75090ff5c394) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0428 ) /* Normal board : Joker Poker (No Double-up) (PP0428) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P17A      1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.5-92.5%  Optimum: 95.5%  Hit Frequency: 44.7%
     Programs Available: PP0459, X000459P & PP0428 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0428_979-a0c.u68",   0x00000, 0x10000, CRC(a206a3bd) SHA1(48e1386027308684daee09370b5ee09d9eb645a8) ) /* Game Version: 979, Library Version: A0c */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0429 ) /* Normal board : Joker Poker (Aces or Better) (No Double-up) (PP0429) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P18A      1    1   2   3    5   6  20  50 100 200 500    800
  % Range: 89.8-91.8%  Optimum: 93.8%  Hit Frequency: 37.6%
     Programs Available: PP0458, X000458P & PP0429 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0429_979-a0c.u68",   0x00000, 0x10000, CRC(453484e7) SHA1(6ba80b72cdd8b3bd43c656400591f4d543a9d94f) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0429a ) /* Normal board : Joker Poker (Aces or Better) (No Double-up) (PP0429) - Must use a SET chip to set denomination*/
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P18A      1    1   2   3    5   6  20  50 100 200 500    800
  % Range: 89.8-91.8%  Optimum: 93.8%  Hit Frequency: 37.6%
     Programs Available: PP0458, X000458P & PP0429 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 05/17/93  @IGT  INT */
	ROM_LOAD( "pp0429_896-914.u68",   0x00000, 0x10000, CRC(f6de62b2) SHA1(6cc9c5dd83afbe0724b4c3905e231b50925b649a) ) /* Game Version: 896, Library Version: 914 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0430 ) /* Normal board : Deuces Joker Wild Poker (PP0430) */
/*
                                         With  w/o  w/o  With
                                         Wild  JKR  Wild JKR
PayTable   3K   STR  FL  FH  4K  SF  5K   RF    4D   RF   4D  (Bonus)
---------------------------------------------------------------------
  P73N      1    2    3   3   3   5   8   10    25  800  1000  2000
  % Range: 93.2-95.2%  Optimum: 97.2%  Hit Frequency: 50.5%
     Programs Available: PP0430, X000430P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0430_a0v-a2d.u68",   0x00000, 0x10000, CRC(afcc183a) SHA1(613608b27b730445379ee9312a625085dba942ae) ) /* Game Version: A0V, Library Version: A2D */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0431 ) /* Normal board : Deuces Joker Wild Poker (PP0431) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
                                         With  w/o  w/o  With
                                         Wild  JKR  Wild JKR
PayTable   3K   STR  FL  FH  4K  SF  5K   RF    4D   RF   4D  (Bonus)
---------------------------------------------------------------------
  P76N      1    1    3   3   3   6   9   12    25  800  1000  2000
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 50.4%
     Programs Available: PP0431, PP0812, PP0813, X000225P & PP0225 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0431_a56-a7a.u68",   0x00000, 0x10000, CRC(34abee12) SHA1(165e6c3d8fb5d6d83217ae5acc35059ac44f6848) ) /* Game Version: A56, Library Version: A7A */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0434 ) /* Normal board : Bonus Poker Deluxe (PP0434) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P200A     1    1    3    4    6   8  80  50 250    800
  % Range: 94.5-96.5%  Optimum: 98.5%  Hit Frequency: 45.2%
     Programs Available: PP0434, X000434P & PP0713 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0434_a45-a75.u68",   0x00000, 0x10000, CRC(e5c9ba19) SHA1(9a01457a54a0445a0f32affe2038366e681cada1) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0447 ) /* Normal board : Standard Draw Poker (PP0447) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K  SF  RF  (Bonus)
---------------------------------------------------------
  CA        1    2    3   4    6   9  25  50 250    800
  % Range: 95.5-97.5%  Optimum: 99.5%  Hit Frequency: 45.5%
     Programs Available: PP0132, PP0447, X000447P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0447_a45-a74.u68",   0x00000, 0x10000, CRC(0ef0bb6c) SHA1(d0ef7a83417054f05d32d0a93ed0d5d618f4dfb9) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0447a ) /* Normal board : Standard Draw Poker (PP0447) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K  SF  RF  (Bonus)
---------------------------------------------------------
  CA        1    2    3   4    6   9  25  50 250    800
  % Range: 95.5-97.5%  Optimum: 99.5%  Hit Frequency: 45.5%
     Programs Available: PP0447, X000447P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0447_979-a0c.u68",   0x00000, 0x10000, CRC(a62748ea) SHA1(585049160f7983047dd13a1715d1edbf3b9778ea) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0449 ) /* Normal board : Standard Draw Poker (PP0449) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0221, PP0449, X000449P & PP0585 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0449_a45-a74.u68",   0x00000, 0x10000, CRC(44699ad7) SHA1(89d9d3107384a6b474c51a34bd34936c36c4b3f5) ) /* Game Version: A45, Library Version: A75 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0449a ) /* Normal board : Standard Draw Poker (PP0449) - Multi Regional / Multi Currency in English / Spanish - Tournament Mode capable */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0221, PP0449, X000449P & PP0585 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0449_a19-a2x.u68",   0x00000, 0x10000, CRC(0bde4a5b) SHA1(dacce2a56ede8145fc22cad4cc75967aaea3b6e4) ) /* Game Version: A19, Library Version: A2X */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1426.u74",  0x00000, 0x8000, CRC(b99b3856) SHA1(a7de74bc712c68ae3da2d546c49dcd70e54c26a1) )
	ROM_LOAD( "mgo-cg1426.u73",  0x08000, 0x8000, CRC(d7145ea0) SHA1(7b23cf7840bab11f7ba9229e990e2c9dd995d59f) )
	ROM_LOAD( "mbo-cg1426.u72",  0x10000, 0x8000, CRC(5fd94bc5) SHA1(b2a23a6a8eb23fbefd7b16e7afb7eddad5f6656c) )
	ROM_LOAD( "mxo-cg1426.u75",  0x18000, 0x8000, CRC(74bc1556) SHA1(9afc00ec4643baa448e0131e1c7aeb3da4739f59) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0452 ) /* Normal board : Double Deuces Wild Poker (PP0452) */
/*
                                        w/D     wo/D
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P236A     1    2    2   3   4  11  16  25 400 250    800
  % Range: 95.6-97.6%  Optimum: 99.6%  Hit Frequency: 45.1%
     Programs Available: PP0452, X000452P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0452_726-706.u68",   0x00000, 0x10000, CRC(26413947) SHA1(66bc2fb3dd62aa9d8ab125665747d331a55e1868) ) /* Game Version: 726, Library Version: 706 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) )
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0454 ) /* Normal board : Bonus Poker Deluxe (PP0454) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P253A     1    1    3    4    5   7  80  50 250    800
  % Range: 92.3-94.3%  Optimum: 96.3%  Hit Frequency: 45.2%
     Programs Available: PP0454, X000454P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0454_a45-a75.u68",   0x00000, 0x10000, CRC(f15f751d) SHA1(6e73625bf3fe0461a171f72ab5478439207516b3) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0455 ) /* Normal board : Joker Poker (PP0455) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 P245D      1    1   2   3    5   7  18  50 100 200 400    940
  % Range: 95.3-97.3%  Optimum: 99.3%  Hit Frequency: 44.2%
     Programs Available: PP0455, X000455P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0455_a45-a74.u68",   0x00000, 0x10000, CRC(1b543c9d) SHA1(ac5409c5fa069b7b19fb82cf04da55c45bc95aa6) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0458 ) /* Normal board : Joker Poker (Aces or Better) (PP0458) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P18A      1    1   2   3    5   6  20  50 100 200 500    800
  % Range: 89.8-91.8%  Optimum: 93.8%  Hit Frequency: 37.6%
     Programs Available: PP0458, X000458P & PP0429 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0458_a45-a74.u68",   0x00000, 0x10000, CRC(856e97ee) SHA1(ca5db52290f1b25139e1afc16ecb5dc4be897771) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0459 ) /* Normal board : Joker Poker (PP0459) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P17A      1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.5-92.5%  Optimum: 95.5%  Hit Frequency: 44.7%
     Programs Available: PP0459, X000459P & PP0428 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0459_979-a0c.u68",   0x00000, 0x10000, CRC(46182faf) SHA1(2ae8b78babd0ee348ae37c0738017a8353eca60c) ) /* Game Version: 979, Library Version: A0c */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0467 ) /* Normal board : Uknown Bonus Poker (PP0467) */
/*
PayTable   Js+  2P  3K  STR  FL  FH  4K  ?? SF  RF  (Bonus)
-----------------------------------------------------------
  ????      1    2   3   4    5   8  25  25 50 250    800
     Programs Available: PP0467
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0467_823-851.u68",   0x00000, 0x10000, CRC(b19d48ae) SHA1(8a27a791e6f132d70d1ea02860e11fedb8515989) ) /* Game Version: 823, Library Version: 851 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0488 ) /* Normal board : Standard Draw Poker (PP0488) - 01/12/95   @ IGT  L95-0175 */
/*
PayTable   Js+  TP  3K  STR  FL  FH  4K  SF  RF  (Bonus)
--------------------------------------------------------
  ????      1    1   2   4    5   8  25  50 250   1000
  % Range: 98.4-100.4%  Optimum: 102.4%  Hit Frequency: ???

NOTE: Will work with the standard CG740 + CAP740 graphics for a non-localized game.
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0488_a30-a4v.u68",   0x00000, 0x10000, CRC(99849f5d) SHA1(643a303beda1c4a4619803071df5e612ab922eb9) ) /* Game Version: A30, Library Version: A4V */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2153.u72",   0x00000, 0x8000, CRC(004c9c8e) SHA1(ec3fa9d2c658de59e722d9979513d6b0c71d5742) ) /*  05/01/95   @ IGT  L95-1123  */
	ROM_LOAD( "mgo-cg2153.u73",   0x08000, 0x8000, CRC(e6843b35) SHA1(2d5219a3cb054ce8b470797c0496c7e24e94ed81) )
	ROM_LOAD( "mbo-cg2153.u74",   0x10000, 0x8000, CRC(e3e28611) SHA1(d040f1df6203dc0bd6a79a391fb90fb930f8dd1a) ) /* Custom Arizona Charlie's Casino card backs */
	ROM_LOAD( "mxo-cg2153.u75",   0x18000, 0x8000, CRC(3ae44f7e) SHA1(00d625b60bffef6ce622cb50a3aa93b92131f578) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap2153.u50", 0x0000, 0x0100, CRC(d02fca7e) SHA1(4384b4238d487b4c763983b27381f4c6c08eb605) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0508 ) /* Normal board : Loose Deuce Poker (PP0508) */
/*
                                       w/D     W/oD
PayTable   3K  STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
-----------------------------------------------------------
  P313A     1    2   2   3   4   8  12  25 500 250    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.2%
     Programs Available: PP0508, X000508P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0508_872-888.u68",   0x00000, 0x10000, CRC(41da6c1e) SHA1(75dc178bc48a58ccf7e87d91419c5dcd99af2d58) ) /* Game Version: 872, Library Version: 888 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg773.u72",   0x00000, 0x8000, CRC(73827e49) SHA1(f2b3f58aeac62b36ba60a408cf04c691b0564ace) )
	ROM_LOAD( "mgo-cg773.u73",   0x08000, 0x8000, CRC(af569952) SHA1(d28ae1c216a99bedc4315e61151934f53b932ef4) )
	ROM_LOAD( "mbo-cg773.u74",   0x10000, 0x8000, CRC(3b59799b) SHA1(b6da6e719f5cc475f2f7112d6a8fe346ea5d511e) )
	ROM_LOAD( "mxo-cg773.u75",   0x18000, 0x8000, CRC(75da0cd8) SHA1(4fb4eda9ae8e59884201368c7d8e4ff8b9967a4f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0509 ) /* Normal board : Standard Draw Poker (No Double-up) (PP0509) */
/*
PayTable   Js+  TP  3K  STR  FL  FH  4K  SF  RF  (Bonus)
--------------------------------------------------------
  ????      1    2   3    4   6  10  25  50 250    800
  % Range: 95.3-97.3%  Optimum: 99.3%  Hit Frequency: 45.4%
     Programs Available: PP0509
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0509_783-779.u68",   0x00000, 0x10000, CRC(a7c9b166) SHA1(3565070b9beba9aa50662253cafafa00f4f5abfa) ) /* Game Version: 782, Library Version: 779 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0510 ) /* Normal board : Standard Draw Poker (PP0510) - 11-24-92   @ IGT  L92-1614 */
/*
PayTable   Js+  TP  3K  STR  FL  FH  4K  SF  RF  (Bonus)
--------------------------------------------------------
  ????      1    2   3    4   7   9  25  50 250    800
  % Range: 95.3-97.3%  Optimum: 99.3%  Hit Frequency: 45.4%
     Programs Available: PP0510
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0510_782-779.u68",   0x00000, 0x10000, CRC(40ce3464) SHA1(230725ac3dd6eb6f891d4abfbcb4c41592531d4e) ) /* Game Version: 782, Library Version: 779 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0514 ) /* Normal board : Double Bonus Poker (PP0514) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P323A     1    1    3   5    7   9  50  80 160  50 250    800
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 43.2%
     Programs Available: PP0514, X000514P & PP0538 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0514_a46-a75.u68",   0x00000, 0x10000, CRC(53ca68c7) SHA1(2c46c89c6347bb8cf80b0ff85daabd0e925c87ec) ) /* Game Version: A46, Library Version: A75, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0514a ) /* Normal board : Double Bonus Poker (PP0514) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P323A     1    1    3   5    7   9  50  80 160  50 250    800
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 43.2%
     Programs Available: PP0514, X000514P & PP0538 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0514_a0n-a0y.u68",   0x00000, 0x10000, CRC(2d54260d) SHA1(cf891c5624331f9b2ef7fbb1e084cfc00f407691) ) /* Game Version: A0N, Library Version: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0514b ) /* Normal board : Double Bonus Poker (PP0514) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P323A     1    1    3   5    7   9  50  80 160  50 250    800
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 43.2%
     Programs Available: PP0514, X000514P & PP0538 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0514_813-824.u68",   0x00000, 0x10000, CRC(a4093ba7) SHA1(f4691af323a14bd4856aed84f1333bac285513ba) ) /* Game Version: 813, Library Version: 824 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0515 ) /* Normal board : Double Bonus Poker (PP0515) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P324A     1    1    3   5    7  10  50  80 160  50 250    800
  % Range: 96.2-98.2%  Optimum: 100.2%  Hit Frequency: 43.3%
     Programs Available: PP0515, X000515P & PP0539 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0515_a46-a75.u68",   0x00000, 0x10000, CRC(ad76895d) SHA1(cb82ad7b05e8962076ceed9e3aa6ead867e95539) ) /* Game Version: A46, Library Version: A75, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0515a ) /* Normal board : Double Bonus Poker (PP0515) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P324A     1    1    3   5    7  10  50  80 160  50 250    800
  % Range: 96.2-98.2%  Optimum: 100.2%  Hit Frequency: 43.3%
     Programs Available: PP0515, X000515P & PP0539 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0515_a0n-a23.u68",   0x00000, 0x10000, CRC(76389889) SHA1(992fb7d3f296d1cecab75ab4cf13fff1b7a5cc11) ) /* Game Version: A0N, Library Version: A23, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0515b ) /* Normal board : Double Bonus Poker (PP0515) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P324A     1    1    3   5    7  10  50  80 160  50 250    800
  % Range: 96.2-98.2%  Optimum: 100.2%  Hit Frequency: 43.3%
     Programs Available: PP0515, X000515P & PP0539 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0515_989-974.u68",   0x00000, 0x10000, CRC(bc9654ab) SHA1(ecf2401bfbfcfa22354cde517cf425a8db8ea961) ) /* Game Version: 989, Library Version: 974, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0515c ) /* Normal board : Double Bonus Poker (PP0515) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P324A     1    1    3   5    7  10  50  80 160  50 250    800
  % Range: 96.2-98.2%  Optimum: 100.2%  Hit Frequency: 43.3%
     Programs Available: PP0515, X000515P & PP0539 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0515_813-824.u68",   0x00000, 0x10000, CRC(6c06b0ea) SHA1(7907d10d7290080c33e46c9644f24d1d2fc6b3ff) ) /* Game Version: 813, Library Version: 824 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0516 ) /* Normal board : Double Bonus Poker (PP0516) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P325A     1    2    3   4    5   8  50  80 160  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 44.5%
     Programs Available: PP0516, X000516P & PP0540 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0516_a46-a75.u68",   0x00000, 0x10000, CRC(6e226711) SHA1(71930ec43c4b75ca50971242be79459976882546) ) /* Game Version: A46, Library Version: A75, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0516a ) /* Normal board : Double Bonus Poker (PP0516) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P325A     1    2    3   4    5   8  50  80 160  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 44.5%
     Programs Available: PP0516, X000516P & PP0540 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0516_989-974.u68",   0x00000, 0x10000, CRC(d9da6e13) SHA1(421678d9cb42daaf5b21074cc3900db914dd26cf) ) /* Game Version: 989, Library Version: 974 , Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0516b ) /* Normal board : Double Bonus Poker (PP0516) - Multi Regional / Multi Currency - Tournament Mode capable */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P325A     1    2    3   4    5   8  50  80 160  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 44.5%
     Programs Available: PP0516, X000516P & PP0540 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0516_a57-a7b.u68",   0x00000, 0x10000, CRC(66e16822) SHA1(552b5ff582197f39823c8c87a9429d3fc2117814) ) /* Game Version: A57, Library Version: A7B */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1348.u72",  0x00000, 0x8000, CRC(b2411211) SHA1(fb78da8c92be7b0ce174aecd0392875fdd3653e7) )
	ROM_LOAD( "mgo-cg1348.u73",  0x08000, 0x8000, CRC(06e97f8a) SHA1(bcdd33aa36746d71fb6ce804eb222ecd7b27d0d6) )
	ROM_LOAD( "mbo-cg1348.u74",  0x10000, 0x8000, CRC(5a4547fd) SHA1(ec28731253733b4ecdff341120ae8572995cffc6) )
	ROM_LOAD( "mxo-cg1348.u75",  0x18000, 0x8000, CRC(cdd8485f) SHA1(4af2f270ed40955bb11f0e427f4ad614fcb3157c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0531 ) /* Normal board : Joker Poker (PP0531) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 P182A      1    1   2   3    5   8  15  50 100 200 400    800
  % Range: 93.6-95.6%  Optimum: 97.6%  Hit Frequency: 44.1%
     Programs Available: PP0531
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0531_979-a0c.u68",   0x00000, 0x10000, CRC(6138e095) SHA1(6075613b5d818c26cdc5da1d05cff3af5d4cbf01) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0536 ) /* Normal board : Joker Poker (PP0536) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 P244D      1    1   2   3    5   7  17  50 100 200 400    940
  % Range: 94.4-96.4%  Optimum: 98.4%  Hit Frequency: 44.1%
     Programs Available: PP0536, X000536P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0536_a45-a74.u68",   0x00000, 0x10000, CRC(413f34fa) SHA1(1800819af18b33936482562bfe694009861a740f) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( pepp0538 ) /* Normal board : Double Bonus Poker (No Double-up) (PP0538) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P323A     1    1    3   5    7   9  50  80 160  50 250    800
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 43.2%
     Programs Available: PP0514, X000514P & PP0538 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0538_a46-a75.u68",   0x00000, 0x10000, CRC(f9e8dbe7) SHA1(dd745a48764f7da7314236016bf9c7fa67a78fad) ) /* Game Version: A46, Library Version: A75, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0540 ) /* Normal board : Double Bonus Poker (No Double-up) (PP0540) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P325A     1    2    3   4    5   8  50  80 160  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 44.5%
     Programs Available: PP0516, X000516P & PP0540 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0540_974-a0y.u68",   0x00000, 0x10000, CRC(b39e0c13) SHA1(a0dd05eb2927792efb1f432dbbd4a9a4fd6ad4c9) ) /* Game Version: 974, Library Version: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0542 ) /* Normal board : One Eyed Jacks (PP0542) Use SET001 to set Denomination for this game */
/*
                                             With    w/o
                                             Wild    Wild
PayTable    As  2PR  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
-----------------------------------------------------------------
  ????       1   1    1   2    4   5  10  50 100 100 400   1600
  % Range: 94.8-96.8%  Optimum: 98.8%  Hit Frequency: 44.5%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0542_905-923.u68",   0x00000, 0x10000, CRC(f4fe3db5) SHA1(18521a569aae8d89e82f9709edc03badae153dd4) ) /* Game Version: 905, Library Version: 923 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2243.u72",  0x00000, 0x8000, CRC(9c81a78e) SHA1(0c5f91443f051bfab4b1e6b0fde443fbb94aa4ec) ) /* Supersedes CG2020 */
	ROM_LOAD( "mgo-cg2243.u73",  0x08000, 0x8000, CRC(53ccbf75) SHA1(092406f74b8604f19800c473dd9ec46fe7fc77b2) )
	ROM_LOAD( "mbo-cg2243.u74",  0x10000, 0x8000, CRC(2ae69415) SHA1(275245ffed10fb6ec2ff9dd433c3f41ff07fe5ad) )
	ROM_LOAD( "mxo-cg2243.u75",  0x18000, 0x8000, CRC(fa6f300b) SHA1(ba72c3004e6fd4e78d8385a52ede566cf5143d10) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0542a ) /* Normal board : One Eyed Jacks (PP0542) Use SET001 to set Denomination for this game */
/*
                                             With    w/o
                                             Wild    Wild
PayTable    As  2PR  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
-----------------------------------------------------------------
  ????       1   1    1   2    4   5  10  50 100 100 400   1600
  % Range: 94.8-96.8%  Optimum: 98.8%  Hit Frequency: 44.5%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0542_905-923.u68",   0x00000, 0x10000, CRC(f4fe3db5) SHA1(18521a569aae8d89e82f9709edc03badae153dd4) ) /* Game Version: 905, Library Version: 923 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2020.u72",  0x00000, 0x8000, CRC(3dc6f3f6) SHA1(c701f3b0fa16a614c9a4094f385c3f75d72cef3b) ) /* CG2020, like 20/20 vision for 1 Eyed Jacks :-) */
	ROM_LOAD( "mgo-cg2020.u73",  0x08000, 0x8000, CRC(71847102) SHA1(a860ef28351f0b7b82c05db26712d50a6f5d3732) ) /* Superseded by CG2243 */
	ROM_LOAD( "mbo-cg2020.u74",  0x10000, 0x8000, CRC(710c717c) SHA1(4e2d463c9f94d446149374d5f66f88d403bd6064) )
	ROM_LOAD( "mxo-cg2020.u75",  0x18000, 0x8000, CRC(78d6eac3) SHA1(09b614abe5f5509f3050c2cec94dc794e2b4db0d) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0550 ) /* Normal board : Joker Poker (Two Pair or Better) (PP0550) */
/*
                                       w/J     w/oJ
PayTable   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
-----------------------------------------------------------
   NA       1   2   4    5   8  16 100 100 400 100    800
  % Range: 93.2-95.2%  Optimum: 97.2%  Hit Frequency: 30.1%
     Programs Available: PP0550, X000550P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0550_a6k-a9g.u68",   0x00000, 0x10000, CRC(1de4ee32) SHA1(0e06a43e7e3988cc2fddd1a57af724f5421d2ca4) ) /* Game Version: A6K, Library Version: A9G */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0555 ) /* Normal board : Standard Draw Poker (PP0555) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K  SF  RF  (Bonus)
---------------------------------------------------------
  CA        1    2    3   4    6   9  25  50 250    800
  % Range: 95.5-97.5%  Optimum: 99.5%  Hit Frequency: 45.5%
     Programs Available: PP0132, PP0447, PP0555, X000447P

Internally the program reports a 99.40% return.

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0555_966-991.u68",   0x00000, 0x10000, CRC(d3e125d3) SHA1(035141511bf19eeb4d0012f456a9f75140e6c308) ) /* Game Version: 966, Library Version: 991 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0568 ) /* Normal board : Joker Poker (PP0568) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
   YD       1    1   1   3    5   7  15  50 100 200 400    940
  % Range: 92.7-94.7%  Optimum: 96.7%  Hit Frequency: 44.1%
     Programs Available: PP0568, X000568P & PP0426 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0568_a45-a74.u68",   0x00000, 0x10000, CRC(a1015eef) SHA1(074dcd966a5da6867532c6e90e2bc98404c2247b) ) /* Game Version: A45, Library Version: A74 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0585 ) /* Normal board : Standard Draw Poker (No Double-up) (PP0585) */
/*

PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0221, PP0449, X000449P & PP0585 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0585_956-979.u68",   0x00000, 0x10000, CRC(419633df) SHA1(f755e7b6cdd95444a02d2172789d9d73f31f56c4) ) /* Game Version: 956, Library Version: 979 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0713 ) /* Normal board : Bonus Poker Deluxe (No Double-up) (PP0713) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P200A     1    1    3    4    6   8  80  50 250    800
  % Range: 94.5-96.5%  Optimum: 98.5%  Hit Frequency: 45.2%
     Programs Available: PP0434, X000434P & PP0713 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0713_979-a0c.u68",   0x00000, 0x10000, CRC(f3413bc6) SHA1(9cad89a5ab4be1f969e9acbafe2016069d4ee307) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0725 ) /* Normal board : Double Bonus Poker (PP0725) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P434A     1    1    3   4    6   9  50  80 160  50 250    800
  % Range: 92.4-94.4%  Optimum: 96.4%  Hit Frequency: 44.9%
     Programs Available: PP0725, X000725P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0725_a46-a75.u68",   0x00000, 0x10000, CRC(6679f095) SHA1(4cca3103610a75c8e515957ebea0cd75052a1100) )/* Game Version: A46, Library Version: A75, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0725a ) /* Normal board : Double Bonus Poker (PP0725) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P434A     1    1    3   4    6   9  50  80 160  50 250    800
  % Range: 92.4-94.4%  Optimum: 96.4%  Hit Frequency: 44.9%
     Programs Available: PP0725, X000725P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0725_a0n-a23.u68",   0x00000, 0x10000, CRC(70ecbe80) SHA1(d44acbaccfe9a8f7cb1217d071353d221f5baa35) )/* Game Version: A0N, Library Version: A23, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0726 ) /* Normal board : Double Bonus Poker (PP0726) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P435A     1    1    3   4    5   8  50  80 160  50 250    800
  % Range: 90.2-92.2%  Optimum: 94.2%  Hit Frequency: 45.1%
     Programs Available: PP0726, X000726P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0726_a46-a75.u68",   0x00000, 0x10000, CRC(7d80c8d7) SHA1(18ca72925c8bb5f5dcc00fa4133816f242292e1d) )/* Game Version: A46, Library Version: A75, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0728 ) /* Normal board : Double Bonus Poker (PP0728) - 11/30/95   @ IGT  L96-0247 */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
P437A/4K/5  1    1    3   4    5   6  50  80 160  50 250    800
  % Range: 87.1-89.1%  Optimum: 91.1%  Hit Frequency: 45.0%
     Programs Available: PP0728
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0728_a46-a75.u68",   0x00000, 0x10000, CRC(ecfbdb77) SHA1(072dc5d59b6a705591045162612b5101e85e1b10) )/* Game Version: A46, Library Version: A75, Video Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0750 ) /* Normal board : Standard Draw Poker (PP0750) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   KQ       1    2    3    4    6   9  25  50 500    625
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 45.5%
     Programs Available: PP0750
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0750_979-a0c.u68",   0x00000, 0x10000, CRC(2224c865) SHA1(6080c29d69c847603b79b7b15f65b32e36d305d8) ) /* Game Version: 979, Library Version: A0C */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /*  08/31/94   @ IGT  L95-0146  */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) )
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) ) /* Supersedes CG740 */
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0757 ) /* Normal board : Double Down Stud Joker Poker (Eights or Better) (PP0757) */
/*
                                            w/J      w/oJ
PayTable   8s+  2P  3K  STR  FL  FH  4K  SF  RF  5K   RF  (Bonus)
----------------------------------------------------------------
  ????      1    2   3   6    9  15  60 100 400 1000 1000  2000
  % Range: 90.3-92.3%  Optimum: 94.3%  Hit Frequency: ??.?%
     Programs Available: PP0757
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0757_a5v-a6d.u68",   0x00000, 0x10000, CRC(dcf3004f) SHA1(c095a874a8813f7100ec430e034530eeb93c4117) ) /* Game Version: A5V, Library Version: A6D */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2015.u72",   0x00000, 0x8000, CRC(7f73ee5c) SHA1(b6c5d423c8176555c1f32605c328ffbfcf94b656) ) /* Verified CG set for PP0760 set */
	ROM_LOAD( "mgo-cg2015.u73",   0x08000, 0x8000, CRC(de270e0e) SHA1(41b207f9380f623ab64dc42224275cccd43417ee) )
	ROM_LOAD( "mbo-cg2015.u74",   0x10000, 0x8000, CRC(02e623d9) SHA1(4c689293f5c5a8eb0b17861cf433ae1e01d83545) )
	ROM_LOAD( "mxo-cg2015.u75",   0x18000, 0x8000, CRC(0c96b7fc) SHA1(adde93f08db0b957daf77d57a7ab60af3b667f25) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0760 ) /* Normal board : Double Down Stud Poker (PP0760) */
/*
PayTable  8s-10s  Js+  2PR  3K   STR  FL  FH  4K  SF   RF  (Bonus)
------------------------------------------------------------------
  ????      1      2    3    4    6    9  12  50 200  1000  4000
  % Range: 86.8-88.8%  Optimum: 90.8%  Hit Frequency: ??.?%
     Programs Available: PP0760
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0760_a4v-a6d.u68",   0x00000, 0x10000, CRC(1c26076c) SHA1(612ac66bbb0827b81dc9c6bc23fa7558445481bc) ) /* Game Version: A4V, Library Version: A6D */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2015.u72",   0x00000, 0x8000, CRC(7f73ee5c) SHA1(b6c5d423c8176555c1f32605c328ffbfcf94b656) ) /* Verified CG set for PP0760 set */
	ROM_LOAD( "mgo-cg2015.u73",   0x08000, 0x8000, CRC(de270e0e) SHA1(41b207f9380f623ab64dc42224275cccd43417ee) )
	ROM_LOAD( "mbo-cg2015.u74",   0x10000, 0x8000, CRC(02e623d9) SHA1(4c689293f5c5a8eb0b17861cf433ae1e01d83545) )
	ROM_LOAD( "mxo-cg2015.u75",   0x18000, 0x8000, CRC(0c96b7fc) SHA1(adde93f08db0b957daf77d57a7ab60af3b667f25) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0763 ) /* Normal board : 4 of a Kind Bonus Poker (PP0763) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P597A      1    1    3    5    8  10  25  40  80  50 250    800
  % Range: 90.2-92.2%  Optimum: 94.2%  Hit Frequency: 42.7%
     Programs Available: PP0763, X000763P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0763_a46-a75.u68",   0x00000, 0x10000, CRC(8e329f30) SHA1(bf271164a4e90e11630a236fb55c70639bdb3e11) ) /* Game Version: A46, Library Version: A75, Game Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0764 ) /* Normal board : 4 of a Kind Bonus Poker (PP0764) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P596A      1    1    3    6    8  10  25  40  80  50 250    800
  % Range: 91.8-93.8%  Optimum: 95.8%  Hit Frequency: 42.3%
     Programs Available: PP0764, X000764P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0764_a46-a75.u68",   0x00000, 0x10000, CRC(9176732a) SHA1(4a9898334aa76c483757addb3a28ace71b008c7e) ) /* Game Version: A46, Library Version: A75, Game Lib ver: A0Y */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2003.u72",  0x00000, 0x8000, CRC(0d425f48) SHA1(b60aaf3f4bd76f75f72f6e8dda724bdf795cb521) ) /*  08/30/94   @ IGT  L95-0145  */
	ROM_LOAD( "mgo-cg2003.u73",  0x08000, 0x8000, CRC(add0afc4) SHA1(0519bf2f36cb67140933b2c533e625544f27d16b) )
	ROM_LOAD( "mbo-cg2003.u74",  0x10000, 0x8000, CRC(8649dec0) SHA1(0024d3a8fd85279552910b14b69b225bda93957f) )
	ROM_LOAD( "mxo-cg2003.u75",  0x18000, 0x8000, CRC(904631cd) SHA1(d280a2f16b51a04b3f601db3535980a765c60e6f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( pepp0775 ) /* Normal board : Royal Deuces Poker or Royal Sevens Poker?? (PP0775) */
/*
Paytable for Royal Deuces lined up with paytable from PP0775:

                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  ????      1    1    3   4   5   8  25  80 250 250    800
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: ???
     Programs Available: PP0775
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0775_a44-a73.u68",   0x00000, 0x10000, CRC(79a56642) SHA1(dfde6c12551e4f12a59e31c14fbfb9edb57e4fac) ) /* Game Version: A44, Library Version: A73 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2xxx.u72",  0x00000, 0x8000, NO_DUMP ) /* Unknown set needed for correct paytable graphics */
	ROM_LOAD( "mgo-cg2xxx.u73",  0x08000, 0x8000, NO_DUMP )
	ROM_LOAD( "mbo-cg2xxx.u74",  0x10000, 0x8000, NO_DUMP )
	ROM_LOAD( "mxo-cg2xxx.u75",  0x18000, 0x8000, NO_DUMP )
	ROM_LOAD( "mro-cg2312.u77",  0x00000, 0x8000, CRC(29a9d408) SHA1(af8c18833ea268b80fabf3b539f35c6782a0309d) ) /* WRONG?!?! Use until the correct set is verified! */
	ROM_LOAD( "mgo-cg2312.u78",  0x08000, 0x8000, CRC(b5ea2602) SHA1(82ee6d45dbc53ccf2d2a956daa83f41bb4a27384) ) /* Gives full paytable, but the hands listed are wrong */
	ROM_LOAD( "mbo-cg2312.u79",  0x10000, 0x8000, CRC(e349202c) SHA1(ef6a904112361425aef5824ae983c15d3456dc49) )
	ROM_LOAD( "mxo-cg2312.u80",  0x18000, 0x8000, CRC(1e0d3df8) SHA1(716d6bd2b41ef41a7da393e805651c378a16e00e) ) /* These graphics don't seem to work with anything else */

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2312.u43", 0x0000, 0x0200, CRC(66971da6) SHA1(6984a68bc2f01009ad6a7a0705c00e715c29bb65) )
ROM_END

ROM_START( pepp0812 ) /* Normal board : Deuces Joker Wild Poker (PP0812) */
/*
                                         With  w/o  w/o  With
                                         Wild  JKR  Wild JKR
PayTable   3K   STR  FL  FH  4K  SF  5K   RF    4D   RF   4D  (Bonus)
---------------------------------------------------------------------
  P76N      1    1    3   3   3   6   9   12    25  250  1000  2000
  % Range: 89.6-91.6%  Optimum: 93.6%  Hit Frequency: 50.3%
     Programs Available: PP0812
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0812_a47-a76.u68",   0x00000, 0x10000, CRC(0de2a7ec) SHA1(fedb8da0608328a9d33e46af18de25004b1d03de) ) /* Game Version: A47, Library Version: A76 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1215.u72",   0x00000, 0x8000, CRC(425f57be) SHA1(6d53ae86bec7189a35671a7f691e101a2ed4d8c4) ) /*  06/09/93   @ IGT  L93-1585  */
	ROM_LOAD( "mgo-cg1215.u73",   0x08000, 0x8000, CRC(0f66cd94) SHA1(9ac0cd01aca87e045c4fd6045ed907a092d6b2ee) )
	ROM_LOAD( "mbo-cg1215.u74",   0x10000, 0x8000, CRC(10f89e44) SHA1(cdc34970b0325a24cfd5c187a4b4dbf42be8fc93) )
	ROM_LOAD( "mxo-cg1215.u75",   0x18000, 0x8000, CRC(73c24e43) SHA1(f09beaf374ad371db2701767ce6ac5bdb13c445a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1215.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( pepp0816 ) /* Normal board : Treasure Chest Poker (PP0816) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K* SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   7  25  50 250    800
  % Range: 94.4-96.4%  Optimum: 98.4%  Hit Frequency: 45.5%
     Programs Available: PP0816

4K* - Treasure Chest bonus round for MAX Bet 4 of a Kind. Possible payouts: 140, 180, 250, 500 or 5000 Credits

 The Treasure Chest routine works as follows:
  1. The chest picked is used for animation purposes only. The chest DOES NOT determine the payout.
  2. A call is made to the random number generator for a number between 0 and 399 (400 possibilities)
  3. The number generated is used through a look up table to determine the bonus prize:

Number Generated  Award   Odds   Chance
-----------------------------------------
               0  5000   0.25%   1 in 400
   1 through  12  500    3.00%  12 in 400
  13 through  28  250    4.00%  16 in 400
  29 through  88  180   15.00%  60 in 400
  89 through 399  140   77.75% 311 in 400

Overall average for the 4 of a Kind bonus is 173.35 credits

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0816_a5f-a7k.u68",   0x00000, 0x10000, CRC(a1e21b56) SHA1(aa0a730b2ed48612c3b20831b1aa698a45f557c0) ) /* Game Version: A5F, Library Version: A7K */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2xxx.u72",  0x00000, 0x8000, NO_DUMP ) /* Unknown set needed for Treasure Chest bonus round graphics */
	ROM_LOAD( "mgo-cg2xxx.u73",  0x08000, 0x8000, NO_DUMP )
	ROM_LOAD( "mbo-cg2xxx.u74",  0x10000, 0x8000, NO_DUMP )
	ROM_LOAD( "mxo-cg2xxx.u75",  0x18000, 0x8000, NO_DUMP )
	ROM_LOAD( "mro-cg2004.u72",  0x00000, 0x8000, CRC(e5e40ea5) SHA1(e0d9e50b30cc0c25c932b2bf444990df1fb2c38c) ) /* WRONG CG set!! MAX Bet 4K "BONUS" graphics is missing and */
	ROM_LOAD( "mgo-cg2004.u73",  0x08000, 0x8000, CRC(12607f1e) SHA1(248e1ecee4e735f5943c50f8c350ca95b81509a7) ) /* all treasure chest graphics missing for bonus & attract screens */
	ROM_LOAD( "mbo-cg2004.u74",  0x10000, 0x8000, CRC(78c3fb9f) SHA1(2b9847c511888de507a008dec981778ca4dbcd6c) )
	ROM_LOAD( "mxo-cg2004.u75",  0x18000, 0x8000, CRC(5aaa4480) SHA1(353c4ce566c944406fce21f2c5045c856ef7a609) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap904.u50", 0x0000, 0x0100, CRC(0eec8336) SHA1(a6585c978dbc2f4f3818e3a5b92f8c28be23c4c0) ) /* BPROM type N82S135N verified */
ROM_END

ROM_START( peip0028 ) /* Normal board :  Joker Poker - French (IP0028) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 PI106B     1    1   2   3    5   6  20  50 100 200 500   1000
  % Range: 89.5-91.5%  Optimum: 93.5%  Hit Frequency: 39.2%
     Programs Available: IP0028

NOTE: Program states Theoretical Percentage (Pourcentage Theorique) of 90.2% but it's really 93.5%
      This program set is superseded by IP0074
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0028_ipp023.u68",   0x00000, 0x8000, CRC(011ddb51) SHA1(8734ddd1f06986efdb0bd83b2bc0a2303273dba9) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0029 ) /* Normal board : Joker Poker - French (IP0029) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 PI105A     1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.3%
     Programs Available: IP0029

NOTE: Program states Theoretical Percentage (Pourcentage Theorique) of 93.3% but it's really 95.0%
      This program set is superseded by IP0062
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0029_ipp023.u68",   0x00000, 0x8000, CRC(b79f91b0) SHA1(b6bed855497153121cb11d84836a737743124635) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0031 ) /* Normal board : Standard Draw Poker - French (IP0031) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  PI34G     1    2    3    4    5   6  25  50 250    333
  % Range: 88.2-90.2%  Optimum: 92.2%  Hit Frequency: 45.5%
     Programs Available: IP0031
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0031_ipp023.u68",   0x00000, 0x8000, CRC(06a22972) SHA1(189de42bf4611bc4cda08f9fa2c1f03b40222681) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0041 ) /* Normal board : Double Deuces Wild - French (IP0041) */
/*
                                       w/J     w/oJ
PayTable   3K  STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
-----------------------------------------------------------
 PI13A      1   2    2   3   4  11  16  25 400 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.1%
     Programs Available: IP0041

"Wild Deuces" are referred to as "Joker" in the program

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0041_ip103-il103.u68",   0x00000, 0x10000, CRC(cf3e66eb) SHA1(d93c25954e3a73533025b7e85562fe2e19021b61) ) /* Game Version: IP103, Library Version: IL103 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1293.u72",  0x00000, 0x8000, CRC(de4b2287) SHA1(5f8232748dd59c9dd6278b68ff35a100f4561d5e) )
	ROM_LOAD( "mgo-cg1293.u73",  0x08000, 0x8000, CRC(1399b459) SHA1(d4952452b5155902cb172be1df0be91a547a59a9) )
	ROM_LOAD( "mbo-cg1293.u74",  0x10000, 0x8000, CRC(76152774) SHA1(3356e144d8dacbd3a1a72dda282b22e4fee3c782) )
	ROM_LOAD( "mxo-cg1293.u75",  0x18000, 0x8000, CRC(fe0ad2a7) SHA1(54ec1b5c3446bf70bdca521583ed2a1eb1557004) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap773.u50", 0x0000, 0x0100, CRC(294b7b10) SHA1(a405a4b8547b713c5c02dacb19e7354095a7b584) )
ROM_END

ROM_START( peip0051 ) /* Normal board : Joker Poker - French (IP0051) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 PI106A     1    1   2   3    5   6  20  50 100 200 500    800
  % Range: 89.0-91.0%  Optimum: 93.0%  Hit Frequency: 39.2%
     Programs Available: IP0051, X002319P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0051_ip101-il101.u68",   0x00000, 0x10000, CRC(25149a28) SHA1(211772751263ad30f073a23de0a81ef6ab72a85f) ) /* Game Version: IP101, Library Version: IL101 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2094.u72",   0x00000, 0x8000, CRC(7c1810e0) SHA1(ec11740f393178d4f5c0506a64e3f996bc6d867c) )
	ROM_LOAD( "mgo-cg2094.u73",   0x08000, 0x8000, CRC(9f4dfe16) SHA1(bf5fdabd72fe259c7a489e77bd7a3d5a14062ce1) )
	ROM_LOAD( "mbo-cg2094.u74",   0x10000, 0x8000, CRC(697fa8cf) SHA1(4dff9a110ac987a25518295dfdc46eb3a46c3215) )
	ROM_LOAD( "mxo-cg2094.u75",   0x18000, 0x8000, CRC(d1a9c781) SHA1(8ee6a2fab99be7b2b95603c6420788c5d1143788) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( peip0058 ) /* Normal board : Standard Draw Poker - French (IP0058) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    1    3    6    7  10  25 100 250    800
  % Range: 88.0-90.0%  Optimum: 92.0%  Hit Frequency: 45.6%
     Programs Available: IP0058
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0058_ipp023.u68",   0x00000, 0x8000, CRC(db2aae6c) SHA1(398a52a73d4a0f71c09dfc4c7cf0c0a5c65ee941) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0062 ) /* Normal board : Joker Poker - French (IP0062) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 PI105A     1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.3%
     Programs Available: IP0062, X002318P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0062_ip105-il105.u68",   0x00000, 0x10000, CRC(bda6ae80) SHA1(8287306cd85fbe13f92fed24f0ca1c92dc19ed35) ) /* Game Version: IP105, Library Version: IL105 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2094.u72",   0x00000, 0x8000, CRC(7c1810e0) SHA1(ec11740f393178d4f5c0506a64e3f996bc6d867c) )
	ROM_LOAD( "mgo-cg2094.u73",   0x08000, 0x8000, CRC(9f4dfe16) SHA1(bf5fdabd72fe259c7a489e77bd7a3d5a14062ce1) )
	ROM_LOAD( "mbo-cg2094.u74",   0x10000, 0x8000, CRC(697fa8cf) SHA1(4dff9a110ac987a25518295dfdc46eb3a46c3215) )
	ROM_LOAD( "mxo-cg2094.u75",   0x18000, 0x8000, CRC(d1a9c781) SHA1(8ee6a2fab99be7b2b95603c6420788c5d1143788) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( peip0074 ) /* Normal board : Joker Poker - French (IP0074) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 PI106B     1    1   2   3    5   6  20  50 100 200 500   1000
  % Range: 89.5-91.5%  Optimum: 93.5%  Hit Frequency: 39.2%
     Programs Available: IP0074, X002320P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0074_ip105-il105.u68",   0x00000, 0x10000, CRC(3f3400ea) SHA1(27ae0a353afdf5a1707cfeeca1a3c9f31999d832) ) /* Game Version: IP105, Library Version: IL105 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2094.u72",   0x00000, 0x8000, CRC(7c1810e0) SHA1(ec11740f393178d4f5c0506a64e3f996bc6d867c) )
	ROM_LOAD( "mgo-cg2094.u73",   0x08000, 0x8000, CRC(9f4dfe16) SHA1(bf5fdabd72fe259c7a489e77bd7a3d5a14062ce1) )
	ROM_LOAD( "mbo-cg2094.u74",   0x10000, 0x8000, CRC(697fa8cf) SHA1(4dff9a110ac987a25518295dfdc46eb3a46c3215) )
	ROM_LOAD( "mxo-cg2094.u75",   0x18000, 0x8000, CRC(d1a9c781) SHA1(8ee6a2fab99be7b2b95603c6420788c5d1143788) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( peip0079 ) /* Normal board : Standard Draw Poker - French (IP0079) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
 PI112A     1    1    3    5    7  10  25  80 250    800
  % Range: 86.1-88.1%  Optimum: 90.1%  Hit Frequency: 45.6%
     Programs Available: IP0079
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0079_ip105-il105.u68",   0x00000, 0x10000, CRC(5b50369f) SHA1(f1478dc4bac5b392888c9a4232a5afa36f71be88) ) /* Game Version: IP105, Library Version: IL105 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2094.u72",   0x00000, 0x8000, CRC(7c1810e0) SHA1(ec11740f393178d4f5c0506a64e3f996bc6d867c) )
	ROM_LOAD( "mgo-cg2094.u73",   0x08000, 0x8000, CRC(9f4dfe16) SHA1(bf5fdabd72fe259c7a489e77bd7a3d5a14062ce1) )
	ROM_LOAD( "mbo-cg2094.u74",   0x10000, 0x8000, CRC(697fa8cf) SHA1(4dff9a110ac987a25518295dfdc46eb3a46c3215) )
	ROM_LOAD( "mxo-cg2094.u75",   0x18000, 0x8000, CRC(d1a9c781) SHA1(8ee6a2fab99be7b2b95603c6420788c5d1143788) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) ) /* BPROM type DM74LS471 (compatible with N82S135N) verified */
ROM_END

ROM_START( peip0101 ) /* Normal board :  Joker Poker - French (IP0101) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    4   6  20  50 100 200 500    800
  % Range: 91.9-93.9%  Optimum: 95.9%  Hit Frequency: 45.3%
     Programs Available: IP0101
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0101_ipp023.u68",   0x00000, 0x8000, CRC(22697a40) SHA1(8f7491cb0ddde4ee6460cc141005a47d197e4d1b) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0103 ) /* Normal board : Joker Poker - French (IP0103) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    4   5  20  40 100 200 500   1000
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 45.3%
     Programs Available: IP0103
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0103_ipp023.u68",   0x00000, 0x8000, CRC(b7c9db4e) SHA1(d0cdd63296fbd82a0f2548d2ff177540903be29d) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0105 ) /* Normal board :  Joker Poker - French (IP0105) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    4   6  15  40 100 200 500    800
  % Range: 87.2-89.2%  Optimum: 91.2%  Hit Frequency: 45.3%
     Programs Available: IP0105
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0105_ipp023.u68",   0x00000, 0x8000, CRC(c6d7db20) SHA1(f9c46c2bda068d627fa874bcbaeef5f2ba1a3039) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0108 ) /* Normal board : Standard Draw Poker - French (IP0108) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    6   9  25  50 250    800
  % Range: 94.2-96.2%  Optimum: 98.2%  Hit Frequency: 45.6%
     Programs Available: IP0108
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0108_ipp023.u68",   0x00000, 0x8000, CRC(b3d6ce51) SHA1(3550e9fb13c3b3edf7ac08dcc992029401265baa) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0111 ) /* Normal board : Joker Poker - French (IP0111) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    4   5  20  50 100 200 500   1000
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.3%
     Programs Available: IP0111
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0111_ipp023.u68",   0x00000, 0x8000, CRC(783461de) SHA1(ea60853481af04dadc81e1be36587c41da0f9c4f) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0112 ) /* Normal board : Standard Draw Poker - French (IP0112) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   6  25  50 250    800
  % Range: 90.0-92.0%  Optimum: 94.0%  Hit Frequency: 45.6%
     Programs Available: IP0112
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0112_ipp023.u68",   0x00000, 0x8000, CRC(ae24b4e8) SHA1(d904f54d83f3e3404a4a4a3fecc65e7b2acda148) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0114 ) /* Normal board : Standard Draw Poker - French (IP0114) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  PI34A     1    2    3    4    5   8  25  50 250    800
  % Range: 91.1-93.1%  Optimum: 96.3%  Hit Frequency: 45.6%
     Programs Available: IP0114
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0114_ipp023.u68",   0x00000, 0x8000, CRC(047f2e60) SHA1(e18aa8e66e2eb21144e2a8c825a6c23322dd7e64) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0115 ) /* Normal board :  Joker Poker - French (IP0115) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    5   6  20  50 100 200 500    800
  % Range: 94.1-96.1%  Optimum: 98.1%  Hit Frequency: 45.3%
     Programs Available: IP0115
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0115_ipp023.u68",   0x00000, 0x8000, CRC(3f5d8a67) SHA1(e197bf4439f8e92328cde81ea8bd103397c741a7) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0116 ) /* Normal board : Standard Draw Poker - French (IP0116) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  PI34B     1    2    3    4    5   6  25  50 250   1000
  % Range: 90.4-92.4%  Optimum: 94.4%  Hit Frequency: 45.6%
     Programs Available: IP0116
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0116_ipp023.u68",   0x00000, 0x8000, CRC(ab5bf95b) SHA1(6aa36d83682cd2170465af0c6eab0952b2f1cfa8) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0118 ) /* Normal board : Standard Draw Poker - French (IP0118) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   9  25  50 250    800
  % Range: 93.4-95.4%  Optimum: 97.4%  Hit Frequency: 45.6%
     Programs Available: IP0118
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0118_ipp023.u68",   0x00000, 0x8000, CRC(e3ffb758) SHA1(6bb2f031b833c00ad1071014641eb7104d7d268f) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( peip0120 ) /* Normal board : Standard Draw Poker - French (IP0120) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   7  25  50 250    800
  % Range: 91.1-93.1%  Optimum: 95.1%  Hit Frequency: 45.6%
     Programs Available: IP0120
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ip0120_ipp023.u68",   0x00000, 0x8000, CRC(a836cc58) SHA1(ee49961b3782d5bfd6c4f7cd453b04fee014ae02) )
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg820.u72",   0x00000, 0x8000, CRC(2638e91f) SHA1(13dfd29b4fcf7862fc497975ccf65a9aee618839) )
	ROM_LOAD( "mgo-cg820.u73",   0x08000, 0x8000, CRC(1fe4820e) SHA1(27d376ad78f3d05672a842665675ac1a0535b6bf) )
	ROM_LOAD( "mbo-cg820.u74",   0x10000, 0x8000, CRC(97756bb3) SHA1(cdaa5d3ce50b75799429d270c1b79fc2f91e2e2b) )
	ROM_LOAD( "mxo-cg820.u75",   0x18000, 0x8000, CRC(57aa951f) SHA1(72b290976774634ccbe081699650f8c60fd7d169) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap656.u50", 0x0000, 0x0100, CRC(038cabc6) SHA1(c6514b4f9dbed6ab2631f563f7e00648661ebdbb) )
ROM_END

ROM_START( pemg0183 ) /* Normal board : Montana Choice Multi-Game MG0183 - Requires a Printer (not yet supported) */
/*
MG0183 has 4 poker games:
  Jacks or Better
  Joker Wild Poker
  Four of a Kind Bonus Poker
  Deuces Wild Poker

Also uses a Dallas (Maxim) DS1216 SmartWatch RAM for RTC (Real Time Clock) functions

Came out of an IGT machine with belly glass calling it Montana Choice
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mg0183_756-782.u68",   0x00000, 0x10000, CRC(b89bcf75) SHA1(f436eb604c81ba6f08e1d11029ce8fff4f50dc3e) ) /* Stalls with "PRINTER ERROR" */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1209.u72",   0x00000, 0x8000, CRC(39b0cc43) SHA1(0a95a7122e64fed7355e762ff2eda2a7246d4693) )
	ROM_LOAD( "mgo-cg1209.u73",   0x08000, 0x8000, CRC(5285ffab) SHA1(e959bf2fec46ee62d7a625eb64f74635fd697643) )
	ROM_LOAD( "mbo-cg1209.u74",   0x10000, 0x8000, CRC(4604ac16) SHA1(b3a7c6c807eb2be7f451d2fcbb6455a66c155a46) )
	ROM_LOAD( "mxo-cg1209.u75",   0x18000, 0x8000, CRC(da344256) SHA1(1320c4a8b48a9e61a4607e0a9d08083fde2bd334) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1144.u50", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "cap1426.u50", 0x0000, 0x0100, CRC(6c7c3462) SHA1(b5481b548f4db460d27a4bfebb08188f36ca0c11) )

	ROM_REGION( 0x1000, "printer", 0 ) /* ROM from the printer driver PCB */
	ROM_LOAD( "lp_86.u9", 0x0000, 0x1000, CRC(cdd93c06) SHA1(96f0a6e231f355a0b82bb0e1e698edbd66ff3020) ) /* 2732 EPROM */
ROM_END

ROM_START( pemg0252 ) /* Normal board : Player's Choice Multi-Game MG0252 - Requires a Printer (not yet supported) */
/*
MG0252 has 4 poker games:
  Deuces Wild
  Jacks Better Bonus
  Deuces / Joker Wild
  Jacks or Better

Requires a printer for ticket payout (no coins) made by Star Micronics Co. Ltd. (Piscataway, NJ)
  40-column dot matrix
  3.25" wide, 2-ply paper roll
  One ply is the customer copy, the other is a carbon copy for auditing

  Printer PCB consists of a SCN8039HCBN40  Signetics / Intel (8048 compatible Single-Chip 8-Bit Microcontroller, ROM-less version) MCU
  8.00MHz OSC and the LP 86 (handwritten label) EPROM

  Some Player's Choice machines contain and use a touchscreen for input
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mg0252_752-778.u68",   0x00000, 0x10000, CRC(1d0ba4f1) SHA1(f906a11d171318a06fb0bb09783bd8e3b99f1ca9) ) /* Stalls with "PRINTER ERROR" */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2076.u72",   0x00000, 0x8000, CRC(84634f0e) SHA1(8f1b9aaa92e861f00569053c1112c2fb7eb577e8) )
	ROM_LOAD( "mgo-cg2076.u73",   0x08000, 0x8000, CRC(cd5dad56) SHA1(60c61f107860151c31be61504eb42fa93d0d41d9) )
	ROM_LOAD( "mbo-cg2076.u74",   0x10000, 0x8000, CRC(cda4ac28) SHA1(84a722f782563f713978403cd6b21492252721cf) )
	ROM_LOAD( "mxo-cg2076.u75",   0x18000, 0x8000, CRC(8e087d93) SHA1(25172001f5e0221aeda52fd51f4605eed24df806) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1426.u50", 0x0000, 0x0100, CRC(6c7c3462) SHA1(b5481b548f4db460d27a4bfebb08188f36ca0c11) )

	ROM_REGION( 0x1000, "printer", 0 ) /* ROM from the printer driver PCB */
	ROM_LOAD( "lp_86.u9", 0x0000, 0x1000, CRC(cdd93c06) SHA1(96f0a6e231f355a0b82bb0e1e698edbd66ff3020) ) /* 2732 EPROM */
ROM_END

ROM_START( pebe0014 ) /* Normal board : Blackjack (BE0014) */
/*
Paytable ID: BJ7

Deal: 1 Deck (52 cards)
      Shuffled before each hand

Progressive Jackpot: None

Game Rules:
  1. Dealer stands on any 17 or more
  2. Blackjack pays 2 for 1, all other wins pay 2 for 1
  3. Bet returned on pushes
  4. Player wins on six (6) cards totaling 21 or less
  5. Split allowed on 1st two cards if a pair, Aces only receive one card
  6. Insurance if dealer has an Ace showing
  7. Surrender only on 1st two cards and if dealer has no Ace (pays 1/2 of original bet)
  8. Double Down only on the 1st two cards
  9. Insurances pays 3 for 1

Optimum percentage payout: 98.3%

In game features that can be enabled/disabled:
  Auto Bet
  Double Down

Known to exist:
 BE0013 508-544 (Non Double-up version of BE0014)
 BE0013 528-A22 (Non Double-up version of BE0014)
 BE0014 526-906
 BE0014 527-936
 BE0017 532-A22
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "be0014_528-a22.u68",   0x00000, 0x10000, CRC(232b32b7) SHA1(a3af9414577642fedc23b4c1911901cd31e9d6e0) ) /* Game Version: 528, Library Version: A22 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2036.u72",  0x00000, 0x8000, CRC(0a168d06) SHA1(7ed4fb5c7bcacab077bcec030f0465c6eaf3ce1c) )
	ROM_LOAD( "mgo-cg2036.u73",  0x08000, 0x8000, CRC(826b4090) SHA1(34390484c0faffe9340fd93d273b9292d09f97fd) )
	ROM_LOAD( "mbo-cg2036.u74",  0x10000, 0x8000, CRC(46aac851) SHA1(28d84b49c6cebcf2894b5a15d935618f84093caa) )
	ROM_LOAD( "mxo-cg2036.u75",  0x18000, 0x8000, CRC(60204a56) SHA1(2e3420da9e79ba304ca866d124788f84861380a7) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap707.u50", 0x0000, 0x0100, CRC(9851ba36) SHA1(5a0a43c1e212ae8c173102ede9c57a3d95752f99) )
ROM_END

ROM_START( pebe0014a ) /* Normal board : Blackjack (BE0014) English / Spanish - Key on Credit */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "be0014_533-a85.u68",   0x00000, 0x10000, CRC(0ce8d349) SHA1(72cdc39e4da0e016dea5aef707a9db5f9a7d500b) ) /* Game Version: 533, Library Version: A85 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1339.u72",  0x00000, 0x8000, NO_DUMP ) /* Needed for correct graphics for currency */
	ROM_LOAD( "mgo-cg1339.u73",  0x08000, 0x8000, NO_DUMP )
	ROM_LOAD( "mbo-cg1339.u74",  0x10000, 0x8000, NO_DUMP )
	ROM_LOAD( "mxo-cg1339.u75",  0x18000, 0x8000, NO_DUMP )
	ROM_LOAD( "mro-cg2036.u72",  0x00000, 0x8000, CRC(0a168d06) SHA1(7ed4fb5c7bcacab077bcec030f0465c6eaf3ce1c) )
	ROM_LOAD( "mgo-cg2036.u73",  0x08000, 0x8000, CRC(826b4090) SHA1(34390484c0faffe9340fd93d273b9292d09f97fd) )
	ROM_LOAD( "mbo-cg2036.u74",  0x10000, 0x8000, CRC(46aac851) SHA1(28d84b49c6cebcf2894b5a15d935618f84093caa) )
	ROM_LOAD( "mxo-cg2036.u75",  0x18000, 0x8000, CRC(60204a56) SHA1(2e3420da9e79ba304ca866d124788f84861380a7) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap707.u50", 0x0000, 0x0100, CRC(9851ba36) SHA1(5a0a43c1e212ae8c173102ede9c57a3d95752f99) )
ROM_END

ROM_START( peke0004 ) /* Normal board : Keno 1-10 Spot (KE0004) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ke0004_566-a17.u68",  0x00000, 0x10000, CRC(dd3b8a70) SHA1(8e402fea9f2e055be309ba24f518b3b513d39ce8) ) /* Game Version: 566, Library Version: A17 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1273.u72",  0x00000, 0x8000, CRC(650f4af8) SHA1(e37d1d8c17924d21958420b5a94f12a415409091) )
	ROM_LOAD( "mgo-cg1273.u73",  0x08000, 0x8000, CRC(e20d65de) SHA1(e78192314ce4864dd02c39f72d8d1c4bc70bb95e) )
	ROM_LOAD( "mbo-cg1273.u74",  0x10000, 0x8000, CRC(61744af9) SHA1(1ab2a2b84dc1869fbb23e028c826490d39512ce7) )
	ROM_LOAD( "mxo-cg1273.u75",  0x18000, 0x8000, CRC(fee165ed) SHA1(26f2c35f091a496bb61789c701795eabb3e5735e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1267.u50", 0x0000, 0x0100, CRC(7051db57) SHA1(76751a3cc47d506983205decb07e99ca0c178a42) )
ROM_END

ROM_START( peke0017 ) /* Normal board : Keno 1-10 Spot (KE0017) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ke0017_560-a07.u68",  0x00000, 0x08000, CRC(a0f70116) SHA1(15808cd3245e2e5934f3365f95590da0be552e8b) ) /* Game Version: 560, Library Version: A07 */
	ROM_RELOAD(                      0x08000, 0x8000) /* 32K version built using earlier gaming libraries */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1016.u72",  0x00000, 0x8000, CRC(92072064) SHA1(ccd12303afb559a57f135f5feff1eada4394c45b) )
	ROM_LOAD( "mgo-cg1016.u73",  0x08000, 0x8000, CRC(fd54f031) SHA1(0990338d00574d798bed2c13ed2cf65118698a65) )
	ROM_LOAD( "mbo-cg1016.u74",  0x10000, 0x8000, CRC(6325ff0b) SHA1(cca693b42d458024d11badf02923f0aedc5252ba) )
	ROM_LOAD( "mxo-cg1016.u75",  0x18000, 0x8000, CRC(54345a8c) SHA1(928f1633343a1d81ef193ebd09de0d36c52057ca) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1016.u50", 0x0000, 0x0100, CRC(12e1be25) SHA1(501487bc729eb80fcf9e61705d3546de5e0d7cde) )
ROM_END

ROM_START( peke1006 ) /* Normal board : Keno 1-10 Spot (KE1006) - Payout 87.61%, Paytable 87-C */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ke1006_590-a77.u68",   0x00000, 0x10000, CRC(5f2a9aac) SHA1(67d24e376d0dcea30d68c3019919e02261c38d7d) ) /* Game Version: 590, Library Version: A77 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1267.u72",  0x00000, 0x8000, CRC(16498b57) SHA1(9c22726299af7204c4be1c6d8afc4c1b512ad918) )
	ROM_LOAD( "mgo-cg1267.u73",  0x08000, 0x8000, CRC(80847c5a) SHA1(8422cd13a91c3c462af5efcfca8615e7eeaa2e52) )
	ROM_LOAD( "mbo-cg1267.u74",  0x10000, 0x8000, CRC(ce7af8a7) SHA1(38675122c764b8fa9260246ea99ac0f0750da277) )
	ROM_LOAD( "mxo-cg1267.u75",  0x18000, 0x8000, CRC(a4394303) SHA1(30a07028de35f74cc4fb776b0505ca743c8d7b5b) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1267.u50", 0x0000, 0x0100, CRC(7051db57) SHA1(76751a3cc47d506983205decb07e99ca0c178a42) )
ROM_END

ROM_START( peke1012 ) /* Normal board : Keno 1-10 Spot (KE1012) - Payout 90.27%, Paytable 90-P */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ke1012_582-a77.u68",   0x00000, 0x10000, CRC(87f696ba) SHA1(de6cc7ff799218ae6fb75521243534484ef4b9a8) ) /* Game Version: 582, Library Version: A77 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1267.u72",  0x00000, 0x8000, CRC(16498b57) SHA1(9c22726299af7204c4be1c6d8afc4c1b512ad918) )
	ROM_LOAD( "mgo-cg1267.u73",  0x08000, 0x8000, CRC(80847c5a) SHA1(8422cd13a91c3c462af5efcfca8615e7eeaa2e52) )
	ROM_LOAD( "mbo-cg1267.u74",  0x10000, 0x8000, CRC(ce7af8a7) SHA1(38675122c764b8fa9260246ea99ac0f0750da277) )
	ROM_LOAD( "mxo-cg1267.u75",  0x18000, 0x8000, CRC(a4394303) SHA1(30a07028de35f74cc4fb776b0505ca743c8d7b5b) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1267.u50", 0x0000, 0x0100, CRC(7051db57) SHA1(76751a3cc47d506983205decb07e99ca0c178a42) )
ROM_END

ROM_START( peke1012a ) /* Normal board : Keno 1-10 Spot (KE1012) - Payout 90.27%, Paytable 90-P */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ke1012_576-a3u.u68",   0x00000, 0x10000, CRC(470e8c10) SHA1(f8a65a3a73477e9e9d2f582eeefa93b470497dfa) ) /* Game Version: 576, Library Version: A3U */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1267.u72",  0x00000, 0x8000, CRC(16498b57) SHA1(9c22726299af7204c4be1c6d8afc4c1b512ad918) )
	ROM_LOAD( "mgo-cg1267.u73",  0x08000, 0x8000, CRC(80847c5a) SHA1(8422cd13a91c3c462af5efcfca8615e7eeaa2e52) )
	ROM_LOAD( "mbo-cg1267.u74",  0x10000, 0x8000, CRC(ce7af8a7) SHA1(38675122c764b8fa9260246ea99ac0f0750da277) )
	ROM_LOAD( "mxo-cg1267.u75",  0x18000, 0x8000, CRC(a4394303) SHA1(30a07028de35f74cc4fb776b0505ca743c8d7b5b) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1267.u50", 0x0000, 0x0100, CRC(7051db57) SHA1(76751a3cc47d506983205decb07e99ca0c178a42) )
ROM_END

ROM_START( peke1013 ) /* Normal board : Keno 2-10 Spot (KE1013) - Payout 91.97%, Paytable 91-D */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ke1013_686-a8r.u68",   0x00000, 0x10000, CRC(97ae2ee7) SHA1(df680ff46320e21a352406e2eaf92003f86434a4) ) /* Game Version: 686, Library Version: A8R */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1267.u72",  0x00000, 0x8000, CRC(16498b57) SHA1(9c22726299af7204c4be1c6d8afc4c1b512ad918) )
	ROM_LOAD( "mgo-cg1267.u73",  0x08000, 0x8000, CRC(80847c5a) SHA1(8422cd13a91c3c462af5efcfca8615e7eeaa2e52) )
	ROM_LOAD( "mbo-cg1267.u74",  0x10000, 0x8000, CRC(ce7af8a7) SHA1(38675122c764b8fa9260246ea99ac0f0750da277) )
	ROM_LOAD( "mxo-cg1267.u75",  0x18000, 0x8000, CRC(a4394303) SHA1(30a07028de35f74cc4fb776b0505ca743c8d7b5b) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1267.u50", 0x0000, 0x0100, CRC(7051db57) SHA1(76751a3cc47d506983205decb07e99ca0c178a42) )
ROM_END

ROM_START( peke1013a ) /* Normal board : Keno 2-10 Spot (KE1013) - Payout 91.97%, Paytable 91-D */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ke1013_590-a77.u68",   0x00000, 0x10000, CRC(3b178f94) SHA1(c601150a728d750b73f949ba6e2d2979c4c4be2e) ) /* Game Version: 590, Library Version: A77 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1267.u72",  0x00000, 0x8000, CRC(16498b57) SHA1(9c22726299af7204c4be1c6d8afc4c1b512ad918) )
	ROM_LOAD( "mgo-cg1267.u73",  0x08000, 0x8000, CRC(80847c5a) SHA1(8422cd13a91c3c462af5efcfca8615e7eeaa2e52) )
	ROM_LOAD( "mbo-cg1267.u74",  0x10000, 0x8000, CRC(ce7af8a7) SHA1(38675122c764b8fa9260246ea99ac0f0750da277) )
	ROM_LOAD( "mxo-cg1267.u75",  0x18000, 0x8000, CRC(a4394303) SHA1(30a07028de35f74cc4fb776b0505ca743c8d7b5b) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1267.u50", 0x0000, 0x0100, CRC(7051db57) SHA1(76751a3cc47d506983205decb07e99ca0c178a42) )
ROM_END

ROM_START( peps0014 ) /* Normal board : Super Joker Slots (PS0014) - Payout 90.11% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0014_569-a2c.u68",   0x00000, 0x10000, CRC(368c3f58) SHA1(ebefcefbb5386659680719936bff72ad61087343) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0916.u72",  0x00000, 0x8000, CRC(d97049d9) SHA1(78f7bb33866ca92922a8b83d5f9ac459edd39176) )
	ROM_LOAD( "mgo-cg0916.u73",  0x08000, 0x8000, CRC(6e075788) SHA1(e8e9d8b7943d62e31d1d58f870bc765cba65c203) )
	ROM_LOAD( "mbo-cg0916.u74",  0x10000, 0x8000, CRC(a5cdf0f3) SHA1(23b2749fd2cb5b8462ce7c912005779b611f32f9) )
	ROM_LOAD( "mxo-cg0916.u75",  0x18000, 0x8000, CRC(1f3a2d72) SHA1(8e07324d436980b628e007d30a835757c1f70f6d) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap916.u50", 0x0000, 0x0100, CRC(b9a5ee21) SHA1(d3c952f594baca9dc234602d90c506dd537c4dcc) )
ROM_END

ROM_START( peps0021 ) /* Normal board : Red White & Blue Slots (PS0021) - Payout 92.51% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0021_569-a2c.u68",   0x00000, 0x10000, CRC(e87d5040) SHA1(e7478e845c888d97190f0398da4bfb043222a3c1) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",  0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",  0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",  0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",  0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0022 ) /* Normal board : Red White & Blue Slots (PS0022) - Payout 90.08% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0022_569-a2c.u68",   0x00000, 0x10000, CRC(d65c0939) SHA1(d91f472a43f77f9df8845e97561540f988e522e3) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",  0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",  0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",  0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",  0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0042 ) /* Normal board : Double Diamond Slots (PS0042) - Payout 92.58% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0042_569-a2c.u68",   0x00000, 0x10000, CRC(b891f04b) SHA1(e735de918e6d91fd87cc85ff40f187dc421a8cf2) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1003.u72",  0x00000, 0x8000, CRC(41ce0395) SHA1(ae90dbae30e4efed33f83ee7038fb2e5171c1945) )
	ROM_LOAD( "mgo-cg1003.u73",  0x08000, 0x8000, CRC(5a383fa1) SHA1(27b1febbdda7332e8d474fc0cca683f451a07090) )
	ROM_LOAD( "mbo-cg1003.u74",  0x10000, 0x8000, CRC(5ec00224) SHA1(bb70a4326cd1810b200e193a449061df62085f37) )
	ROM_LOAD( "mxo-cg1003.u75",  0x18000, 0x8000, CRC(2ffacd52) SHA1(38126ac4998806a1ddd55e6aa1942044240d41d0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1003.u50", 0x0000, 0x0100, CRC(cc400805) SHA1(f5ac48ad2a5df64da150f09f2ea5d910230bde56) )
ROM_END

ROM_START( peps0043 ) /* Normal board : Double Diamond Slots (PS0043) - Payout 90.10% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0043_569-a2c.u68",   0x00000, 0x10000, CRC(d612429c) SHA1(95eb4774482a930066456d517fb2e4f67d4df4cb) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1003.u72",  0x00000, 0x8000, CRC(41ce0395) SHA1(ae90dbae30e4efed33f83ee7038fb2e5171c1945) )
	ROM_LOAD( "mgo-cg1003.u73",  0x08000, 0x8000, CRC(5a383fa1) SHA1(27b1febbdda7332e8d474fc0cca683f451a07090) )
	ROM_LOAD( "mbo-cg1003.u74",  0x10000, 0x8000, CRC(5ec00224) SHA1(bb70a4326cd1810b200e193a449061df62085f37) )
	ROM_LOAD( "mxo-cg1003.u75",  0x18000, 0x8000, CRC(2ffacd52) SHA1(38126ac4998806a1ddd55e6aa1942044240d41d0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1003.u50", 0x0000, 0x0100, CRC(cc400805) SHA1(f5ac48ad2a5df64da150f09f2ea5d910230bde56) )
ROM_END

ROM_START( peps0045 ) /* Normal board : Red White & Blue Slots (PS0045) - Payout 87.56% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0045_569-a2c.u68",   0x00000, 0x10000, CRC(de180b84) SHA1(0d592d7d535b0aacbd62c18ac222da770fab7b85) ) /* 3 Coins Max / 3 Lines */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",  0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",  0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",  0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",  0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0047 ) /* Normal board : Wild Cherry Slots (PS0047) - Payout 90.20% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0047_569-a2c.u68",   0x00000, 0x10000, CRC(b7df1cf8) SHA1(5c5392b7b3a387ccb45fe96310b47078215f2ea0) ) /* 2 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1004.u72",  0x00000, 0x3000, BAD_DUMP CRC(631ca70e) SHA1(c4d9c4ebc9e90bd1704f154f1bf9b0ce91af35b4) ) /* These should each be 0x8000 bytes */
	ROM_LOAD( "mgo-cg1004.u73",  0x08000, 0x1000, BAD_DUMP CRC(28b4f718) SHA1(91ca3ebf288bb60f43fb0e7aace1f2ada2e978ba) ) /* Needs to redumped as standard 27C256 roms */
	ROM_LOAD( "mbo-cg1004.u74",  0x10000, 0x1000, BAD_DUMP CRC(542a3a45) SHA1(13569e5bac44c2cffd647c27cf40456494d4612e) )
	ROM_LOAD( "mxo-cg1004.u75",  0x18000, 0x1000, BAD_DUMP CRC(20242083) SHA1(f9c9bbe559516f1d02cd4f0bab69f0f7765780ca) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1004.u50", 0x0000, 0x0100, CRC(5eced808) SHA1(b40b8efa8cbc76cff7560c36939275eb360c6f11) )
ROM_END

ROM_START( peps0090 ) /* Normal board : Gold, Silver & Bronze (PS0090) - Payout 90.19% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0090_569-a2c.u68",   0x00000, 0x10000, CRC(5a727ff0) SHA1(6eed9d85620eff751c598d56807470f8753e8dd5) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1059.u72",   0x00000, 0x8000, CRC(96210de3) SHA1(10daa358f1fc507e9f4c788265c0acc57678fa40) ) /* Also contains graphics for Double Diamonds, use CAP1003 */
	ROM_LOAD( "mgo-cg1059.u73",   0x08000, 0x8000, CRC(cfb9a357) SHA1(a390bed240960efd8da6e7815a0b0d272133f20f) )
	ROM_LOAD( "mbo-cg1059.u74",   0x10000, 0x8000, CRC(6c159972) SHA1(b6fbebba2749534b7fcb9cd32fe17cdc673912f7) )
	ROM_LOAD( "mxo-cg1059.u75",   0x18000, 0x8000, CRC(7ec9d699) SHA1(45ec30370d2ef12511f897cb1155327ed4d2ce01) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1059.u50", 0x0000, 0x0100, CRC(a995258f) SHA1(5c33fb2a9a939cfdf4634f886690fa7ccc57fe52) )
ROM_END

ROM_START( peps0092 ) /* Normal board : Wild Cherry Slots (PS0092) - Payout 90.18% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0092_569-a2c.u68",   0x00000, 0x10000, CRC(d533f6d5) SHA1(9c470f7c474022445aeb45ee8c5757d1b6957a91) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1004.u72",  0x00000, 0x3000, BAD_DUMP CRC(631ca70e) SHA1(c4d9c4ebc9e90bd1704f154f1bf9b0ce91af35b4) ) /* These should each be 0x8000 bytes */
	ROM_LOAD( "mgo-cg1004.u73",  0x08000, 0x1000, BAD_DUMP CRC(28b4f718) SHA1(91ca3ebf288bb60f43fb0e7aace1f2ada2e978ba) ) /* Needs to redumped as standard 27C256 roms */
	ROM_LOAD( "mbo-cg1004.u74",  0x10000, 0x1000, BAD_DUMP CRC(542a3a45) SHA1(13569e5bac44c2cffd647c27cf40456494d4612e) )
	ROM_LOAD( "mxo-cg1004.u75",  0x18000, 0x1000, BAD_DUMP CRC(20242083) SHA1(f9c9bbe559516f1d02cd4f0bab69f0f7765780ca) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1004.u50", 0x0000, 0x0100, CRC(5eced808) SHA1(b40b8efa8cbc76cff7560c36939275eb360c6f11) )
ROM_END

ROM_START( peps0206 ) /* Normal board : Red White & Blue Slots (PS0206) - Payout 85.13% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0206_569-a2c.u68",   0x00000, 0x10000, CRC(e165efc0) SHA1(170f917740c63b0b00f424ce02bfd04dc48a1397) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",  0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",  0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",  0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",  0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0207 ) /* Normal board : Red White & Blue Slots (PS0207) - Payout 90.14% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0207_569-a2c.u68",   0x00000, 0x10000, CRC(e7c5f103) SHA1(6e420c151e07863b21a423f8743da360d6389cde) ) /* 3 Coins Max / 3 Lines */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",  0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",  0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",  0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",  0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0296 ) /* Normal board : Haywire Slots (PS0296) - Payout 90.00% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0296_561-959.u68",   0x00000, 0x10000, CRC(da871550) SHA1(99e7a4fc77731b185751622ba2e08a44ad8eb7f9) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1220.u72",   0x00000, 0x8000, CRC(ad101bc1) SHA1(64d801522d32c35ac0fd359a9b1ca51dfe2e7467) )
	ROM_LOAD( "mgo-cg1220.u73",   0x08000, 0x8000, CRC(22b64f11) SHA1(39f350433fc2c96b3848d5af3cc106290b7540c9) )
	ROM_LOAD( "mbo-cg1220.u74",   0x10000, 0x8000, CRC(8ba1ddb3) SHA1(d5d8621b14ed4873cb1343b97202a1536763eee8) )
	ROM_LOAD( "mxo-cg1220.u75",   0x18000, 0x8000, CRC(07bc5413) SHA1(fcba1b60a2eb6bba4f7bb5ef3e67ff23dd036bf5) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1228.u50", 0x0000, 0x0100, CRC(e15b6db9) SHA1(3c637d1ff95a34bfa0259f7fe74989535b2b3a25) )
ROM_END

ROM_START( peps0298 ) /* Normal board : Double Diamond Slots (PS0298) - Payout 87.42% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0298_569-a2c.u68",   0x00000, 0x10000, CRC(3af2eb50) SHA1(1b2e1036f78658da3821bcf88a48b5068b2421b2) ) /* 5 Coins Max / 5 Lines */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1003.u72",  0x00000, 0x8000, CRC(41ce0395) SHA1(ae90dbae30e4efed33f83ee7038fb2e5171c1945) )
	ROM_LOAD( "mgo-cg1003.u73",  0x08000, 0x8000, CRC(5a383fa1) SHA1(27b1febbdda7332e8d474fc0cca683f451a07090) )
	ROM_LOAD( "mbo-cg1003.u74",  0x10000, 0x8000, CRC(5ec00224) SHA1(bb70a4326cd1810b200e193a449061df62085f37) )
	ROM_LOAD( "mxo-cg1003.u75",  0x18000, 0x8000, CRC(2ffacd52) SHA1(38126ac4998806a1ddd55e6aa1942044240d41d0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1003.u50", 0x0000, 0x0100, CRC(cc400805) SHA1(f5ac48ad2a5df64da150f09f2ea5d910230bde56) )
ROM_END

ROM_START( peps0308 ) /* Normal board : Double Jackpot Slots (PS0308) - Payout 90.10% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0308_569-a2c.u68",   0x00000, 0x10000, CRC(fe30e081) SHA1(d216cbc6336727caf359e6b178c856ab2659cabd) ) /* 5 Coins Max / 5 Lines */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0911.u72",  0x00000, 0x8000, CRC(48491b50) SHA1(9ec6d3ff34a08d40082a1347a46635838fd31afc) )
	ROM_LOAD( "mgo-cg0911.u73",  0x08000, 0x8000, CRC(c1ff7d97) SHA1(78ab138ae9c7f9b3352f9b1ef5fbc473993bb8c8) )
	ROM_LOAD( "mbo-cg0911.u74",  0x10000, 0x8000, CRC(202e0f9e) SHA1(51421dfd1b00a9e3b1e938d5bffaa3b7cd4c2b5e) )
	ROM_LOAD( "mxo-cg0911.u75",  0x18000, 0x8000, CRC(d97740a2) SHA1(d76926d7fbbc24d2384a1079cb97e654600b134b) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap911.u50", 0x0000, 0x0100, CRC(f117e781) SHA1(ba9d850c93e5f3abc26b0ba51f67fa7c07e05f59) )
ROM_END

ROM_START( peps0364 ) /* Normal board : Red White & Blue Slots (PS0364) - Payout 90.09% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0364_569-a2c.u68",   0x00000, 0x10000, CRC(596c4ae4) SHA1(a06626fb7d17fd12c7514d435031924973e4ba55) ) /* 3 Coins Max / 1 Line (show alt graphics??) */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",  0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",  0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",  0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",  0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0366 ) /* Normal board : Double Diamonds Deluxe Slots (PS0366) - Payout 94.99% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0366_569-a2c.u68",   0x00000, 0x10000, CRC(32fd35c5) SHA1(8562608bc45328559b7c04ef4026384862bf2d51) ) /* 2 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1303.u72",  0x00000, 0x8000, CRC(f5bcc47f) SHA1(b132960a095996d1790df4dcedf14a29169fe667) )
	ROM_LOAD( "mgo-cg1303.u73",  0x08000, 0x8000, CRC(e16cc01b) SHA1(086f2ac533d868dbaa3852516b6fef344dddff13) )
	ROM_LOAD( "mbo-cg1303.u74",  0x10000, 0x8000, CRC(2c1ffea2) SHA1(efc16869f994415a03663205ca2396e4c26e25a3) )
	ROM_LOAD( "mxo-cg1303.u75",  0x18000, 0x8000, CRC(7c4578e0) SHA1(70b6cf02225a4804592f44c90365f370fb83281a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1303.u50", 0x0000, 0x0100, CRC(5341ea30) SHA1(63c8f7fa94dcb772c308b307f755a188b9b5e7eb) )
ROM_END

ROM_START( peps0372 ) /* Normal board : Double Diamonds Deluxe Slots (PS0372) - Payout 90.10% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0372_569-a2c.u68",   0x00000, 0x10000, CRC(45573591) SHA1(0a15313af506817528eb7319a0994b6993412965) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1303.u72",  0x00000, 0x8000, CRC(f5bcc47f) SHA1(b132960a095996d1790df4dcedf14a29169fe667) )
	ROM_LOAD( "mgo-cg1303.u73",  0x08000, 0x8000, CRC(e16cc01b) SHA1(086f2ac533d868dbaa3852516b6fef344dddff13) )
	ROM_LOAD( "mbo-cg1303.u74",  0x10000, 0x8000, CRC(2c1ffea2) SHA1(efc16869f994415a03663205ca2396e4c26e25a3) )
	ROM_LOAD( "mxo-cg1303.u75",  0x18000, 0x8000, CRC(7c4578e0) SHA1(70b6cf02225a4804592f44c90365f370fb83281a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1303.u50", 0x0000, 0x0100, CRC(5341ea30) SHA1(63c8f7fa94dcb772c308b307f755a188b9b5e7eb) )
ROM_END

ROM_START( peps0373 ) /* Normal board : Double Diamonds Deluxe Slots (PS0373) - Payout 87.56% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0373_583-a6c.u68",   0x00000, 0x10000, CRC(085bed76) SHA1(8775f7c9654f92eab616cdda4505cbde30154889) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1303.u72",  0x00000, 0x8000, CRC(f5bcc47f) SHA1(b132960a095996d1790df4dcedf14a29169fe667) )
	ROM_LOAD( "mgo-cg1303.u73",  0x08000, 0x8000, CRC(e16cc01b) SHA1(086f2ac533d868dbaa3852516b6fef344dddff13) )
	ROM_LOAD( "mbo-cg1303.u74",  0x10000, 0x8000, CRC(2c1ffea2) SHA1(efc16869f994415a03663205ca2396e4c26e25a3) )
	ROM_LOAD( "mxo-cg1303.u75",  0x18000, 0x8000, CRC(7c4578e0) SHA1(70b6cf02225a4804592f44c90365f370fb83281a) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1303.u50", 0x0000, 0x0100, CRC(5341ea30) SHA1(63c8f7fa94dcb772c308b307f755a188b9b5e7eb) )
ROM_END

ROM_START( peps0426 ) /* Normal board : Sizzling Sevens Slots (PS0268) - Payout 90.35% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0426_571-a3h.u68",   0x00000, 0x10000, CRC(b53771c1) SHA1(23fccd5facb98fc83b8903946435be4f15199ff8) ) /* 3 Coins Max / 1 Lines */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg1428.u72",  0x00000, 0x8000, CRC(90a4ef75) SHA1(effcaabcbc52b7fb3a85546b201f2628131a96fa) )
	ROM_LOAD( "mgo-cg1428.u73",  0x08000, 0x8000, CRC(78416e96) SHA1(4523339e00eacbae5cd1a9aabb3dce18ff1a604e) )
	ROM_LOAD( "mbo-cg1428.u74",  0x10000, 0x8000, CRC(b84034e2) SHA1(704962eed288c8e7bed288ae8f99576c9851c52b) )
	ROM_LOAD( "mxo-cg1428.u75",  0x18000, 0x8000, CRC(3da3cb07) SHA1(882ee4f3008ef44f09c3ffb2b4b8085cac05c93c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1428.u50", 0x0000, 0x0100, CRC(c15aae14) SHA1(9b2784ad3da7afdb7778cae9906d0a7c76df7a32) )
ROM_END

ROM_START( peps0581 ) /* Normal board : Red White & Blue Slots (PS0581) - Payout 85.06% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0581_571-a3h.u68",   0x00000, 0x10000, CRC(8730cbf3) SHA1(2e81aec6982909511a9782f60ff506215f9aac7c) ) /* 5 Coins Max / 5 Lines */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",  0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",  0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",  0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",  0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0615 ) /* Normal board : Chaos Slots (PS0615) - Payout 90.02% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0615_586-a6c.u68",   0x00000, 0x10000, CRC(d27dd6ab) SHA1(b3f065f507191682edbd93b07b72ed87bf6ae9b1) ) /* 3 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2246.u72",  0x00000, 0x8000, CRC(7c08c355) SHA1(2a154b81c6d9671cea55a924bffb7f5461747142) )
	ROM_LOAD( "mgo-cg2246.u73",  0x08000, 0x8000, CRC(b3c16487) SHA1(c97232fadd086f604eaeb3cd3c2d1c8fe0dcfa70) )
	ROM_LOAD( "mbo-cg2246.u74",  0x10000, 0x8000, CRC(e61331f5) SHA1(4364edc625d64151cbae40780b54cb1981086647) )
	ROM_LOAD( "mxo-cg2246.u75",  0x18000, 0x8000, CRC(f0f4a27d) SHA1(3a10ab196aeaa5b50d47b9d3c5b378cfadd6fe96) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap2246.u50", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) ) /* WRONG!! - Should be CAP2246 here */
ROM_END

ROM_START( peps0631 ) /* Normal board : Red White & Blue Slots (PS0631) - Payout 89.96% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0631_586-a6c.u68",   0x00000, 0x10000, CRC(3d4c52dd) SHA1(f6b31a77de52c0d6402b51349f34bdb687b27178) ) /* 3 Coins Max / 1 Line (show alt graphics??) */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg0960.u72",  0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",  0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",  0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",  0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0716 ) /* Normal board : River Gambler Slots (PS0716) - Payout 95.00% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps0716_589-a6x.u68",   0x00000, 0x10000, CRC(7615d7b6) SHA1(91fe62eec720a0dc2ebf48835065148f19499d16) ) /* 2 Coins Max / 1 Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2266.u72",  0x00000, 0x8000, CRC(590accd8) SHA1(4e1c963c50799eaa49970e25ecf9cb01eb6b09e1) )
	ROM_LOAD( "mgo-cg2266.u73",  0x08000, 0x8000, CRC(b87ffa05) SHA1(92126b670b9cabeb5e2cc35b6e9c458088b18eea) )
	ROM_LOAD( "mbo-cg2266.u74",  0x10000, 0x8000, CRC(e3df30e1) SHA1(c7d2ae9a7c7e53bfb6197b635efcb5dc231e4fe0) )
	ROM_LOAD( "mxo-cg2266.u75",  0x18000, 0x8000, CRC(56271442) SHA1(61ad0756b9f6412516e46ef6625a4c3899104d4e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap2266.u50", 0x0000, 0x0100, CRC(5aaff103) SHA1(9cfda9c095cb77a8bb761c131a0f358e79b97abc) )
ROM_END

ROM_START( pex0002p ) /* Superboard : Standard Draw Poker (X000002P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BB       1    2    3    4    5   8  25  50 250   1000
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0002, X000002P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000002p.u66",   0x00000, 0x10000, CRC(17cee391) SHA1(173e5775c3e887e16b4f0330d21873331dfb7c33) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0002pa ) /* Superboard : Standard Draw Poker (X000002P+XP000109) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BB       1    2    3    4    5   8  25  50 250   1000
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0002, X000002P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000109.u67",   0x00000, 0x10000, CRC(2e3347a7) SHA1(ef4f1822389ff67c00065b2c04897deabee2eba1) ) /* Monaco Region */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000002p.u66",   0x00000, 0x10000, CRC(17cee391) SHA1(173e5775c3e887e16b4f0330d21873331dfb7c33) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2201.u77",  0x00000, 0x8000, CRC(8f82a114) SHA1(dc4aaaa12442a66386d9bef969afa60a7e2e386b) ) /* Monaco Region */
	ROM_LOAD( "mgo-cg2201.u78",  0x08000, 0x8000, CRC(71797c5b) SHA1(15dff00aad8006855af98a2ad39fe1a6e87d7d24) )
	ROM_LOAD( "mbo-cg2201.u79",  0x10000, 0x8000, CRC(27201cbf) SHA1(9d197e04c36e94ff08bd76c7200ea4e8f345b8ab) )
	ROM_LOAD( "mxo-cg2201.u80",  0x18000, 0x8000, CRC(b79b6d11) SHA1(dcc30465e4de104c54b19e95e7216023576d90c7) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0006p ) /* Superboard : Standard Draw Poker (X000006P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   CB       1    2    3    4    6   9  25  50 250   1000
  % Range: 96.1-98.1%  Optimum: 100.1%  Hit Frequency: 45.2%
     Programs Available: PP0006, X000006P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000006p.u66",   0x00000, 0x10000, CRC(0ee609a1) SHA1(57043ac2c6ff4377479dd7b66d7e379053f3f602) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0040p ) /* Superboard : Standard Draw Poker (X000040P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   WA       1    2    3    4    5   7  20  50 300    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0040, X000040P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000040p.u66",   0x00000, 0x10000, CRC(f672c36e) SHA1(c44d78070b8f858cb2ef27c84b62acc8eec1bea8) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0042p ) /* Superboard : Standard Draw Poker (10's or Better) (X000042P+XP000038) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P7A      1     1    3    4    6   9  25  50 300    800
  % Range: 86.8-88.8%  Optimum: 90.8%  Hit Frequency: 49.1%
     Programs Available: PP0042, X000042P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000042p.u66",   0x00000, 0x10000, CRC(7f803cb5) SHA1(f8f2974c78c63a608a536f2c72cf7ccb7d1ba0eb) ) /* Standard Draw Poker (10's or Better) */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0045p ) /* Superboard : Standard Draw Poker (10's or Better) (X000045P+XP000038) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8A      1     1    3    4    5   8  25  50 300    800
  % Range: 84.6-86.6%  Optimum: 88.6%  Hit Frequency: 49.2%
     Programs Available: PP0045, X000045P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000045p.u66",   0x00000, 0x10000, CRC(5412e1f9) SHA1(a962b7731df2f534ea79b6d3e376abf45104df37) ) /* Standard Draw Poker (10's or Better) */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0046p ) /* Superboard : Standard Draw Poker (10's or Better) (X000046P+XP000038) */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P8B      1     1    3    4    5   8  25  50 300   1000
  % Range: 85.2-87.2%  Optimum: 89.2%  Hit Frequency: 49.0%
     Programs Available: PP0046, X000046P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000046p.u66",   0x00000, 0x10000, CRC(f19cdab4) SHA1(4502a8660100be69625e215610cf918e3ffc5e4f) ) /* Standard Draw Poker (10's or Better) */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0053p ) /* Superboard : Joker Poker (X000053P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P18B      1    1   2   3    5   6  20  50 100 200 500   1000
  % Range: 90.6-92.6%  Optimum: 94.6%  Hit Frequency: 39.2%
     Programs Available: PP0053, X000053P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000053p.u66",   0x00000, 0x10000, CRC(b247e455) SHA1(8d311956e46be62ee17de6fb2ae1594e623a78c0) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0054p ) /* Superboard : Deuces Wild Poker (X000054P+XP000038) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P21A      1    2    2   3   4   8  10  20 200 300    800
  % Range: 87.4-89.4%  Optimum: 91.4%  Hit Frequency: 45.3%
     Programs Available: PP0054, X000054P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000054p.u66",   0x00000, 0x10000, CRC(820b3738) SHA1(dfa389df0d27b69072b5ece5b624ef97551f4af1) ) /* Deuces Wild Poker - 03/22/95   @ IGT  L95-1142 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055p ) /* Superboard : Deuces Wild Poker (X000055P+XP000019) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000019.u67",   0x00000, 0x10000, CRC(8ac876eb) SHA1(105b4aee2692ccb20795586ccbdf722c59db66cf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pa ) /* Superboard : Deuces Wild Poker (X000055P+XP000022) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000022.u67",   0x00000, 0x10000, CRC(7930741b) SHA1(d3b83fd08a458cc794301ef612f8c7b13d4b2050) ) /*  09/28/95   @ IGT  L95-2028  */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2291.u77",  0x00000, 0x8000, CRC(db4e491c) SHA1(e371e7b236962a0f30640c683d3a0a302c51aee9) ) /* Custom The Orleans card backs */
	ROM_LOAD( "mgo-cg2291.u78",  0x08000, 0x8000, CRC(17bb35f8) SHA1(ba9e8aa3ff42b17c7be6ee46c70db22d8e60e52c) ) /* Compatible with most "standard" game sets */
	ROM_LOAD( "mbo-cg2291.u79",  0x10000, 0x8000, CRC(de1036e4) SHA1(774bbcda301754dc4a606974248847a2264c3827) )
	ROM_LOAD( "mxo-cg2291.u80",  0x18000, 0x8000, CRC(7049403c) SHA1(3a29a00fb8dfdb30dba757c1536151827ea09068) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2291.u43", 0x0000, 0x0200, CRC(6dfbb409) SHA1(10cd84e53344cb8502a268c41bdd41bc927e5544) )
ROM_END

ROM_START( pex0055pb ) /* Superboard : Deuces Wild Poker (X000055P+XP000023) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000023.u67",   0x00000, 0x10000, CRC(d2ad7dd3) SHA1(9c5fe5ca5a5a682566e96c6802b7164730cda919) ) /* No Double-up */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2399.u77",  0x00000, 0x8000, CRC(0424f4ba) SHA1(c8b192a6f63c8c9937cb3923d27b9ba2c39823cd) ) /* Custom The Fun Ships card backs */
	ROM_LOAD( "mgo-cg2399.u78",  0x08000, 0x8000, CRC(5848a2fa) SHA1(4173a473016b7a776d2b59bf3ded0be35bd43721) ) /* Also compatible with ACE$ Bonus Poker */
	ROM_LOAD( "mbo-cg2399.u79",  0x10000, 0x8000, CRC(5c3e16f6) SHA1(a4aa457f239527bffa6472e0d6f9d836f796b326) )
	ROM_LOAD( "mxo-cg2399.u80",  0x18000, 0x8000, CRC(bd7669d5) SHA1(4343a9764fd563e2e1cdd8558f2f53f77006b159) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2399.u43", 0x0000, 0x0200, NO_DUMP ) /* Should be CAPX2399 */
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) ) /* Wrong!! Should be CAPX2399 */
ROM_END

ROM_START( pex0055pc ) /* Superboard : Deuces Wild Poker (X000055P+XP000028) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000028.u67",   0x00000, 0x10000, CRC(1407fe54) SHA1(4615efbba9a58698e2cfd53c93fa133678101441) ) /* 01/15/96   @ IGT  L96-0716 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2374.u77",  0x00000, 0x8000, CRC(ceeb714d) SHA1(6de908d04bcaa243195943affa9ad0d725de5c81) ) /* Custom Horseshoe Casino card backs */
	ROM_LOAD( "mgo-cg2374.u78",  0x08000, 0x8000, CRC(d0fabad5) SHA1(438ebe074fa3eaa3073ef042f481449f416d0665) )
	ROM_LOAD( "mbo-cg2374.u79",  0x10000, 0x8000, CRC(9a0fbc8d) SHA1(aa39f47cbeaf8218fd2d753c9a350e9eab5df5d3) )
	ROM_LOAD( "mxo-cg2374.u80",  0x18000, 0x8000, CRC(99814562) SHA1(2d8e132f4cc4edd06332c0327927219513b22832) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2374.u43", 0x0000, 0x0200, CRC(f922e1b8) SHA1(4aa5291c59431c022dc0561a6e3b38209f60286a) )
ROM_END

ROM_START( pex0055pd ) /* Superboard : Deuces Wild Poker (X000055P+XP000035) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000035.u67",   0x00000, 0x10000, CRC(aa16e53b) SHA1(5a37c7af2c09be26e8734b36da765fd408754771) ) /* 07/08/96   @ IGT  L96-1587 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2389.u77",  0x00000, 0x8000, CRC(912d639f) SHA1(f66f5db847816754c4f1fbeb1171c4e6c1331039) ) /* Custom The Wild Wild West Casino card backs */
	ROM_LOAD( "mgo-cg2389.u78",  0x08000, 0x8000, CRC(87de1a4e) SHA1(4f8524eba297771d3292dd2fca9008546d0e3066) )
	ROM_LOAD( "mbo-cg2389.u79",  0x10000, 0x8000, CRC(7c248712) SHA1(4c2cfd7f46fa757438706095137ccf230c30f3a4) )
	ROM_LOAD( "mxo-cg2389.u80",  0x18000, 0x8000, CRC(4fdb7daf) SHA1(131d0a5c33f75f859522a4307c0e23273d5d4cb6) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2389.u43", 0x0000, 0x0200, NO_DUMP ) /* Should be CAPX2389, but 2 PCBs had CAPX1321 */
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) ) /* Wrong!! Should be CAPX2389 */
ROM_END

ROM_START( pex0055pe ) /* Superboard : Deuces Wild Poker (X000055P+XP000038) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2354.u77",  0x00000, 0x8000, CRC(e7913f01) SHA1(43b99f347f0566949cad4172bdf1462dbc2ad860) ) /* Custom Sunset Station Hotel-Casino card backs */
	ROM_LOAD( "mgo-cg2354.u78",  0x08000, 0x8000, CRC(317381b8) SHA1(196794335a7a85fd50f4e0da00ceb5ba6a93b36c) )
	ROM_LOAD( "mbo-cg2354.u79",  0x10000, 0x8000, CRC(ccc643fa) SHA1(9ca9b35f2eed46824d11a8a3b937eb1b2afb639c) )
	ROM_LOAD( "mxo-cg2354.u80",  0x18000, 0x8000, CRC(b06a49b8) SHA1(822ab7e53247e27feecbe96491bef0efea05212d) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2354.u43", 0x0000, 0x0200, CRC(c16e03ed) SHA1(59c0e98b40353e012f2dc2ce25dd46433449e8cc) )
ROM_END

ROM_START( pex0055pf ) /* Superboard : Deuces Wild Poker (X000055P+XP000040) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000040.u67",   0x00000, 0x10000, CRC(7b30b1d5) SHA1(394c964cf6269a4cd9b9debe8f4a5a0c96db06a7) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pg ) /* Superboard : Deuces Wild Poker (X000055P+XP000053) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055ph ) /* Superboard : Deuces Wild Poker (X000055P+XP000055) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000055.u67",   0x00000, 0x10000, CRC(339821e0) SHA1(127d4eff01136feaf1e3242d57433349afb7b6ca) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pi ) /* Superboard : Deuces Wild Poker (X000055P+XP000063) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000063.u67",   0x00000, 0x10000, CRC(008dcaf9) SHA1(34203f602a531c1a58febdf31fe7a94c2c09fcb4) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pj ) /* Superboard : Deuces Wild Poker (X000055P+XP000075) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000075.u67",   0x00000, 0x10000, CRC(79b3013f) SHA1(98e6f2c7756643bc9c2371c015cba7ed2c314a60) ) /* English / Spanish */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) ) /* Contains needed English / Spanish graphics */
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pk ) /* Superboard : Deuces Wild Poker (X000055P+XP000079) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000079.u67",   0x00000, 0x10000, CRC(fe9757b7) SHA1(8547f00f23e2e3cd4b36d006b760eca6a19f0710) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pl ) /* Superboard : Deuces Wild Poker (X000055P+XP000094) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000094.u67",   0x00000, 0x10000, CRC(97ff8171) SHA1(ca714f201a7425df81b830698f65640570ac5935) ) /* Coupon Compatible */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pm ) /* Superboard : Deuces Wild Poker (X000055P+XP000095) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000095.u67",   0x00000, 0x10000, CRC(6a1679ea) SHA1(421e8c9eacc8e397267a48cad7ae96f541b1c19a) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pn ) /* Superboard : Deuces Wild Poker (X000055P+XP000098) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000098.u67",   0x00000, 0x10000, CRC(12257ad8) SHA1(8f613377519850f8f711ccb827685dece018c735) ) /*  01/29/98   @ IGT  L98-0643  */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055po ) /* Superboard : Deuces Wild Poker (X000055P+XP000102) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000102.u67",   0x00000, 0x10000, CRC(76d37639) SHA1(c7190ee3bff135b39ce42428eadef3ca067508b4) ) /* English / Spanish */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) ) /* Contains needed English / Spanish graphics */
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pp ) /* Superboard : Deuces Wild Poker (X000055P+XP000104) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000104.u67",   0x00000, 0x10000, CRC(53dbef8a) SHA1(0be0d0f0ac33ae86e79ba7a1151a281774f80af9) ) /*  04/06/98   @ IGT  L98-0910  */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pq ) /* Superboard : Deuces Wild Poker (X000055P+XP000112) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) ) /* No Double-up */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0055pr ) /* Superboard : Deuces Wild Poker (X000055P+XP000126) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P32A      1    2    2   3   4  10  15  25 200 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 45.1%
     Programs Available: PP0055, X000055P, PP0723
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000126.u67",   0x00000, 0x10000, CRC(e41685ac) SHA1(a81ad3f352eebcc0684fc20c73a6935288207215) ) /* English / Spanish */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) ) /* Contains needed English / Spanish graphics */
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0057p ) /* Superboard : Deuces Wild Poker (X000057P+XP000038) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P34A      1    2    2   3   5   9  15  25 200 250    800
  % Range: 96.8-98.8%  Optimum: 100.8%  Hit Frequency: 45.3%
     Programs Available: PP0057, X000057P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000057p.u66",   0x00000, 0x10000, CRC(2046710a) SHA1(3fcc7c3069ea54d0e4982814aca1d7b327bb2074) ) /* Deuces Wild Poker - 05/04/95   @ IGT  L95-1143 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0060p ) /* Superboard : Standard Draw Poker (X000060P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   GA       1    2    3    4    5   6  25  50 250    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0060, X000060P & PP0420 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000060p.u66",   0x00000, 0x10000, CRC(1ec1ad4d) SHA1(19bc46fe86e0ff23c43ffc072b1c461b60481f0f) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0124p ) /* Superboard : Deuces Wild Poker (X000124P+XP000053) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
----------------------------------------------------------
  P59A      1    2    3   4   4   9  12  20 200 250    800
  % Range: 93.1-95.1%  Optimum: 97.1%  Hit Frequency: 44.4%
     Programs Available: PP0124, X000124P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000124p.u66",   0x00000, 0x10000, CRC(08096310) SHA1(34d41a67ecdab93be3af0cd33fcdacfe75da6c08) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0150p ) /* Superboard : Standard Draw Poker (X000150P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   KK       1    2    3    4    6   9  25  50 500    500
  % Range: 94.9-96.9%  Optimum: 98.9%  Hit Frequency: 45.5%
     Programs Available: PP0150, X000150P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000150p.u66",   0x00000, 0x10000, CRC(d10759fa) SHA1(eae633d03ac9db86520a70825ac0a59ee9ebc819) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0158p ) /* Superboard : 4 of a Kind Bonus Poker (X000158P+XP000038) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P77A      1    2    3    4    5   8  25  40  80  50 250    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.5%
     Programs Available: PP0158, X000158P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000158p.u66",   0x00000, 0x10000, CRC(51a8a294) SHA1(f76992729ceaca18af82ab2fb3403dc5a48b7e8a) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0171p ) /* Superboard : Joker Poker (X000171P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
   YA       1    1   2   3    5   7  15  50 100 200 400    800
  % Range: 92.4-94.4%  Optimum: 96.4%  Hit Frequency: 44.2%
     Programs Available: PP0171, X000171P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000171p.u66",   0x00000, 0x10000, CRC(01e7a3f7) SHA1(a81018fd0b659b3d83cfdc9e6db1f387779bbe98) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0188p ) /* Superboard : Standard Draw Poker (X000188P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P40A      1    2    3    4    6   9  15  40 250    800
  % Range: 86.5-88.5%  Optimum: 92.5%  Hit Frequency: 45.5%
     Programs Available: PP0188, X000188P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000188p.u66",   0x00000, 0x10000, CRC(3eb7580e) SHA1(86f2280542fb8a55767efd391d0fb04a12ed9408) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0190p ) /* Superboard : Deuces Wild Poker (X000190P+XP000053) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P57A      1    2    3   4   4   8  10  20 200 250    800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.5%
     Programs Available: PP0417, X000417P, X000190P & PP0190 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000190p.u66",   0x00000, 0x10000, CRC(17cc52f6) SHA1(90818df05c6bba3ffcbc2047c7eedea31abdc05a) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0197p ) /* Superboard : Standard Draw Poker (X000197P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250   1000
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: PP0197, X000197P & PP0419 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000197p.u66",   0x00000, 0x10000, CRC(13394826) SHA1(c9325b2ed47267b2918a3bf365b90338134ce9c7) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0203p ) /* Superboard : 4 of a Kind Bonus Poker (X000203P+XP000038) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P90A      1    2    3    4    5   7  25  40  80  50 250    800
  % Range: 94.0-96.0%  Optimum: 98.0%  Hit Frequency: 45.5%
     Programs Available: PP0203, X000203P, PP0590 & PP0409 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000203p.u66",   0x00000, 0x10000, CRC(8e5dc66e) SHA1(e3030598e42e7a76e608d5edfaf263aadc2caf85) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0224p ) /* Superboard : Deuces Wild Poker (X000224P+XP000053) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P47A      1    2    2   3   4  13  16  25 200 250    800
  % Range: 92.8-94.8%  Optimum: 96.8%  Hit Frequency: 44.9%
     Programs Available: PP0290, X000224P & PP0224 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000224p.u66",   0x00000, 0x10000, CRC(69ca6ac7) SHA1(e6f119b69f769a1f484a4ced3eb4d9e5406f0b09) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0225p ) /* Superboard : Dueces Joker Wild Poker 1-100 Coins (X000225P+XP000079) */
/*
                                         With  w/o  w/o  With
                                         Wild  JKR  Wild JKR
PayTable   3K   STR  FL  FH  4K  SF  5K   RF    4D   RF   4D  (Bonus)
---------------------------------------------------------------------
  P76N      1    1    3   3   3   6   9   12    25  800  1000  2000
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 50.4%
     Programs Available: PP0431, PP0812, PP0813, X000225P & PP0225 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000079.u67",   0x00000, 0x10000, CRC(fe9757b7) SHA1(8547f00f23e2e3cd4b36d006b760eca6a19f0710) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000225p.u66",   0x00000, 0x10000, CRC(d965dd5e) SHA1(1f3e3acb9319e26fa8563f57d1c75940a4445959) ) /* Dueces Joker Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2242.u77",  0x00000, 0x8000, CRC(963a7e7d) SHA1(ebb159f6c731a3f912382745ef9a9c6d4fa2fc99) )
	ROM_LOAD( "mgo-cg2242.u78",  0x08000, 0x8000, CRC(53eed56f) SHA1(e79f31c5c817b8b96b4970c1a702d1892961d441) )
	ROM_LOAD( "mbo-cg2242.u79",  0x10000, 0x8000, CRC(af092f50) SHA1(53a3536593bb14c4072e8a5ee9e05af332feceb1) )
	ROM_LOAD( "mxo-cg2242.u80",  0x18000, 0x8000, CRC(ecacb6b2) SHA1(32660adcc266fbbb3702a0cd30e25d11b953d23d) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0242p ) /* Superboard : Deuces Wild Poker (X000242P+XP000053) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P56A      1    2    3   3   4   8  10  20 200 250    800
  % Range: 89.4-91.4%  Optimum: 93.4%  Hit Frequency: 45.1%
     Programs Available: PP0242, X000242P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000242p.u66",   0x00000, 0x10000, CRC(e0292d63) SHA1(8d8ec5dc1abaf8e8a8a7451d3a814023d8195fb5) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0265p ) /* Superboard : 4 of a Kind Bonus Poker (X000265P+XP000038) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P101A      1    2    3    4    5   6  25  40  80  50 250    800
  % Range: 92.5-94.5%  Optimum: 96.9%  Hit Frequency: 45.5%
     Programs Available: PP0265, X000265P, PP0403 & PP0410 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000265p.u66",   0x00000, 0x10000, CRC(70a7ab9b) SHA1(3574f89406da12e6a48e5e2c45cad0ba7ee4caf3) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0291p ) /* Superboard : Deuces Wild Poker (X000291P+XP000053) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P62A      1    2    3   4   4   9  15  25 200 250    800
  % Range: 94.9-96.9%  Optimum: 98.9%  Hit Frequency: 44.4%
     Programs Available: PP0418, X000291P & PP0291 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000291p.u66",   0x00000, 0x10000, CRC(7aa72bbd) SHA1(f754b85579720abd2efd57efa31091bca3c01425) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0417p ) /* Superboard : Deuces Wild Poker (X000417P+XP000053) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
-----------------------------------------------------------
  P57A      1    2    3   4   4   8  10  20 200 250    800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.5%
     Programs Available: PP0417, X000417P & PP0190 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000417p.u66",   0x00000, 0x10000, CRC(8c80433d) SHA1(508f49522fa391ca39523829c0dda6af7c99a5fe) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0426p ) /* Superboard : Joker Poker (X000426P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
   YD       1    1   1   3    5   7  15  50 100 200 400    940
  % Range: 92.7-94.7%  Optimum: 96.7%  Hit Frequency: 44.1%
     Programs Available: PP0426, X000426P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000426p.u66",   0x00000, 0x10000, CRC(84a7a946) SHA1(e1e8ff3f16b0c51f723afba35d8f2b1a1451fa22) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0430p ) /* Superboard : Dueces Joker Wild Poker (X000430P+XP000079) */
/*
                                         With  w/o  w/o  With
                                         Wild  JKR  Wild JKR
PayTable   3K   STR  FL  FH  4K  SF  5K   RF    4D   RF   4D  (Bonus)
---------------------------------------------------------------------
  P73N      1    2    3   3   3   5   8   10    25  800  1000  2000
  % Range: 93.2-95.2%  Optimum: 97.2%  Hit Frequency: 50.5%
     Programs Available: PP0430, X000430P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000079.u67",   0x00000, 0x10000, CRC(fe9757b7) SHA1(8547f00f23e2e3cd4b36d006b760eca6a19f0710) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000430p.u66",   0x00000, 0x10000, CRC(905571e3) SHA1(fd506516fed22842df8e9dbb3683dcb4c459719b) ) /* Dueces Joker Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2242.u77",  0x00000, 0x8000, CRC(963a7e7d) SHA1(ebb159f6c731a3f912382745ef9a9c6d4fa2fc99) )
	ROM_LOAD( "mgo-cg2242.u78",  0x08000, 0x8000, CRC(53eed56f) SHA1(e79f31c5c817b8b96b4970c1a702d1892961d441) )
	ROM_LOAD( "mbo-cg2242.u79",  0x10000, 0x8000, CRC(af092f50) SHA1(53a3536593bb14c4072e8a5ee9e05af332feceb1) )
	ROM_LOAD( "mxo-cg2242.u80",  0x18000, 0x8000, CRC(ecacb6b2) SHA1(32660adcc266fbbb3702a0cd30e25d11b953d23d) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0434p ) /* Superboard : Bonus Poker Deluxe (X000434P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P200A     1    1    3    4    6   8  80  50 250    800
  % Range: 94.5-96.5%  Optimum: 98.5%  Hit Frequency: 45.2%
     Programs Available: PP0434, X000434P & PP0713 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000434p.u66",   0x00000, 0x10000, CRC(5a3ad28b) SHA1(d4f103c7ce3c4f72728450ab015aca8ef10cd79c) ) /* Bonus Poker Deluxe */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0447p ) /* Superboard : Standard Draw Poker (X000447P+XP000038) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K  SF  RF  (Bonus)
---------------------------------------------------------
  CA        1    2    3   4    6   9  25  50 250    800
  % Range: 95.5-97.5%  Optimum: 99.5%  Hit Frequency: 45.5%
     Programs Available: PP0447, X000447P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000447p.u66",   0x00000, 0x10000, CRC(4d3ab095) SHA1(337255658242816dc552432aee328fa52e556793) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0449p ) /* Superboard : Standard Draw Poker (X000449P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P11A      1    2    3    4    5   9  25  50 250    800
  % Range: 92.1-94.1%  Optimum: 96.1%  Hit Frequency: 45.5%
     Programs Available: PP0221, PP0449, X000449P & PP0585 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000449p.u66",   0x00000, 0x10000, CRC(6c22b0b3) SHA1(47cb1b8edb3e1d5d2055a7a31a1dfb46b4fd6391) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0451p ) /* Superboard : Bonus Poker Deluxe (X000451P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P253A     1    1    3    4    5   8  80  50 250    800
  % Range: 93.4-95.4%  Optimum: 97.4%  Hit Frequency: 45.2%
     Programs Available: PP0451, X000451P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000451p.u66",   0x00000, 0x10000, CRC(4f11e26c) SHA1(6cea3cbef530ef4ece2a4351cbd9ead5b66bb359) ) /* Bonus Poker Deluxe */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0452p ) /* Superboard : Double Deuces Wild Poker (X000452P+XP000038) */
/*
                                        w/D     wo/D
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
  P236A     1    2    2   3   4  11  16  25 400 250    800
  % Range: 95.6-97.6%  Optimum: 99.6%  Hit Frequency: 45.1%
     Programs Available: PP0452, X000452P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000452p.u66",   0x00000, 0x10000, CRC(a96c7a71) SHA1(6be1012e68035fbc9aa5e0e6ea3a6c54c1864b1b) ) /* Double Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0454p ) /* Superboard : Bonus Poker Deluxe (X000454P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P253A     1    1    3    4    5   7  80  50 250    800
  % Range: 92.3-94.3%  Optimum: 96.3%  Hit Frequency: 45.2%
     Programs Available: PP0454, X000454P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000454p.u66",   0x00000, 0x10000, CRC(93934ae4) SHA1(f243c66e23269e5509bf1306e9e37a579b08fda4) ) /* Bonus Poker Deluxe */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0455p ) /* Superboard : Joker Poker (X000455P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P245D     1    1   2   3    5   7  18  50 100 200 400    940
  % Range: 95.3-97.3%  Optimum: 99.3%  Hit Frequency: 44.2%
     Programs Available: PP0455, X000455P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000455p.u66",   0x00000, 0x10000, CRC(4992c51f) SHA1(8c70c59bdb16feba438230b30765076cebd44b53) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0458p ) /* Superboard : Joker Poker (X000458P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P18A      1    1   2   3    5   6  20  50 100 200 500    800
  % Range: 89.8-91.8%  Optimum: 93.8%  Hit Frequency: 37.6%
     Programs Available: PP0458, X000458P & PP0429 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000458p.u66",   0x00000, 0x10000, CRC(dcd20558) SHA1(22c99a265431b0ef8199d3cb69fbbc4aff822dc0) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0459p ) /* Superboard : Joker Poker (X000459P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P17A      1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 44.7%
     Programs Available: PP0459, X000459P & PP0428 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000459p.u66",   0x00000, 0x10000, CRC(03cef341) SHA1(3813c4781ca6d164880f6d06a7d6dbae29012e7d) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0459pa ) /* Superboard : Joker Poker (X000459P+XP000155) - Use SET038 to set Denomination for this game */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  P17A      1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 44.7%
     Programs Available: PP0459, X000459P & PP0428 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000155.u67",   0x00000, 0x10000, CRC(13d2cc01) SHA1(8d5a7d7a2e1e811d3f99a7eb4f662d02751d45a6) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000459p.u66",   0x00000, 0x10000, CRC(03cef341) SHA1(3813c4781ca6d164880f6d06a7d6dbae29012e7d) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0489p ) /* Superboard : Double Down Stud Deuces Wild Poker (X000489P+XP000038) */
/*

                                                  w/D      w/oD
PayTable   Ks+  2P   3K   STR  FL  FH  4K  SF  5K  RF  4D   RF  (Bonus)
-----------------------------------------------------------------------
  ????      1    1    2    3    4   5   6  12  15 100 400 1000   2000
  % Range: 91.3-93.3%  Optimum: 95.3%  Hit Frequency: 45.3%
     Programs Available: PP0489, X000489P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* Errors with INCOMPATIBLE EPROM error, no dumped program works with this DATA set */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000489p.u66",   0x00000, 0x10000, CRC(73d21c66) SHA1(0f863037a34549f4255dedda70b4401d288eee01) ) /* Double Down Stud Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0508p ) /* Superboard : Loose Deuce Deuces Wild Poker (X000508P+XP000038) */
/*
                                       w/D     W/oD
PayTable   3K  STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
-----------------------------------------------------------
  P313A     1    2   2   3   4   8  12  25 500 250    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.2%
     Programs Available: PP0508, X000508P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000508p.u66",   0x00000, 0x10000, CRC(5efde4b4) SHA1(ead7448464aecc03748f04e4d6e9f346d262cd96) ) /* Loose Deuce Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0514p ) /* Superboard : Double Bonus Poker (X000514P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P323A     1    1    3   5    7   9  50  80 160  50 250    800
  % Range: 95.1-97.1%  Optimum: 99.1%  Hit Frequency: 43.2%
     Programs Available: PP0514, X000514P & PP0538 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000514p.u66",   0x00000, 0x10000, CRC(32cf8696) SHA1(83992695d3af4de10d0e53e01558faad18cdc221) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0515p ) /* Superboard : Double Bonus Poker (X000515P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P324A     1    1    3   5    7  10  50  80 160  50 250    800
  % Range: 96.2-98.2%  Optimum: 100.2%  Hit Frequency: 43.3%
     Programs Available: PP0515, X000515P & PP0539 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000515p.u66",   0x00000, 0x10000, CRC(4311224a) SHA1(69e6657dacd6e09c2d1514417994adc561f63a83) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0516p ) /* Superboard : Double Bonus Poker (X000516P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P325A     1    2    3   4    5   9  50  80 160  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 44.5%
     Programs Available: PP0516, X000516P & PP0540 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000516p.u66",   0x00000, 0x10000, CRC(37f84ce7) SHA1(2e5157d02febec0ff31eb5a23254f7c49a486cf5) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0536p ) /* Superboard : Joker Poker (X000536P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 P244D      1    1   2   3    5   7  17  50 100 200 400    940
  % Range: 94.4-96.4%  Optimum: 98.4%  Hit Frequency: 44.1%
     Programs Available: PP0536, X000536P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000536p.u66",   0x00000, 0x10000, CRC(0b18dc1b) SHA1(07350fe258441f8565bfd875342823149b7757f1) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0537p ) /* Superboard : Standard Draw Poker (X000537P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
 P259A      1    2    3    4    5   7  35  50 250    800
  % Range: 92.2-94.2%  Optimum: 96.2%  Hit Frequency: 45.5%
     Programs Available: PP0537, X000537P

Some call this a 4 of a Kind Bonus Poker with all 4K paying the same. Internally it's classified as Standard Draw Poker

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000537p.u66",   0x00000, 0x10000, CRC(a0c97fde) SHA1(a152b5e99a425127246b2200b7599c17e28479bd) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0550p ) /* Superboard : Joker Poker (X000550P+XP000055) */
/*
                                       w/J     w/oJ
PayTable   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
-----------------------------------------------------------
   NA       1   2   4    5   8  16 100 100 400 100    800
  % Range: 93.2-95.2%  Optimum: 97.2%  Hit Frequency: 30.1%
     Programs Available: PP0550, X000550P

Internally the program erroneously reports a 95.50% return. Superseded by X002338P

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000055.u67",   0x00000, 0x10000, CRC(339821e0) SHA1(127d4eff01136feaf1e3242d57433349afb7b6ca) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000550p.u66",   0x00000, 0x10000, CRC(8a320403) SHA1(751a83ba25ffdae4b8d745bdec6ecdebf351efa0) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0557p ) /* Superboard : Standard Draw Poker (X000557P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   WA       1    2    3    4    5   7  20  50 300    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: X000557P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000557p.u66",   0x00000, 0x10000, CRC(98c16858) SHA1(d428ae712b7ee45ce6e4f7d9b1c75687655be140) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0568p ) /* Superboard : Joker Poker (X000568P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
   YD       1    1   1   3    5   7  15  50 100 200 400    940
  % Range: 92.7-94.7%  Optimum: 96.7%  Hit Frequency: 44.1%
     Programs Available: PP0568, X000568P & PP0426 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000568p.u66",   0x00000, 0x10000, CRC(570e941d) SHA1(db9227d044f55e8d038e3ea0ba72e42e68efcb30) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0578p ) /* Superboard : Standard Draw Poker (X000578P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   BA       1    2    3    4    5   8  25  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 45.3%
     Programs Available: X000578P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000578p.u66",   0x00000, 0x10000, CRC(08f34909) SHA1(b6f2f5b0aab289bb51cb67c85f0db0411321a2ae) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0581p ) /* Superboard : 4 of a Kind Bonus Poker (X000581P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P373A     1    2    2   4    5   8  25  40  80  50 250    800
  % Range: 87.7-89.7%  Optimum: 91.7%  Hit Frequency: 45.2%
     Programs Available: X000581P, PP0581 - Non Double-up Only
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000581p.u66",   0x00000, 0x10000, CRC(a4cfecc3) SHA1(b2c805781ba43bda9e208d8c16578dc96b6f58f7) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0588p ) /* Superboard : Joker Poker (X000588P+XP000038) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
   ZA       1    1   2   3    5   7  20  50 100 200 400    800
  % Range: 99.2-98.2%  Optimum: 100.2%  Hit Frequency: 44.2%
     Programs Available: PP0588, X000588
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000588p.u66",   0x00000, 0x10000, CRC(baa448cc) SHA1(0f1da407304f7dafbe06119d068f7caf99404cb4) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0725p ) /* Superboard : Double Bonus Poker (X000725P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P434A     1    1    3   4    6   9  50  80 160  50 250    800
  % Range: 92.4-94.4%  Optimum: 96.4%  Hit Frequency: 44.9%
     Programs Available: PP0725, X000725P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000725p.u66",   0x00000, 0x10000, CRC(a56f3910) SHA1(06d0d4a8722e033ff1fbe0947135952ce8274725) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0726p ) /* Superboard : Double Bonus Poker (X000726P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P435A     1    1    3   4    5   8  50  80 160  50 250    800
  % Range: 90.2-92.2%  Optimum: 94.2%  Hit Frequency: 45.1%
     Programs Available: PP0726, X000726P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000726p.u66",   0x00000, 0x10000, CRC(800eb7e5) SHA1(cb4c2749d025ab093f26967909d5f366f1cc9cba) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0727p ) /* Superboard : Double Bonus Poker (X000727P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P436A     1    1    3   4    5   9  50  80 160  50 250    800
  % Range: 91.3-93.3%  Optimum: 95.3%  Hit Frequency: 45.0%
     Programs Available: PP0727, X000727P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000727p.u66",   0x00000, 0x10000, CRC(4828474c) SHA1(9836b76113a71802df30ca15f7c9a5790e6f1c5b) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0763p ) /* Superboard : 4 of a Kind Bonus Poker (X000763P+XP000038) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P597A      1    1    3    5    8  10  25  40  80  50 250    800
  % Range: 90.2-92.2%  Optimum: 94.2%  Hit Frequency: 42.7%
     Programs Available: PP0763, X000763P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000763p.u66",   0x00000, 0x10000, CRC(bf7dda7d) SHA1(1a6089d1159c199199e608f3dd2ba7b45a29b31c) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex0764p ) /* Superboard : 4 of a Kind Bonus Poker (X000764P+XP000038) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
 P596A      1    1    3    6    8  10  25  40  80  50 250    800
  % Range: 91.8-93.8%  Optimum: 95.8%  Hit Frequency: 42.3%
     Programs Available: PP0764, X000764P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000764p.u66",   0x00000, 0x10000, CRC(0a1213d7) SHA1(208262fa3e9642789dcedcd3551ad876e5390707) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2010p ) /* Superboard : Nevada Bonus Poker (X002010P+XP000038) */
/*
                                         2-K
PayTable   Js+  2PR  3K  3A  STR  FL  FH  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P503A     1    1    3   6   5    8  10  25 200 100 250    800
  % Range: 94.8-96.8%  Optimum: 98.8%  Hit Frequency: 42.7%
     Programs Available: X002010P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002010p.u66",   0x00000, 0x10000, CRC(1a76f22e) SHA1(a269391682d44fdaf4fd68fa3e3ca7366509ce92) ) /* Nevada Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2016p ) /* Superboard : Fullhouse Bonus Poker (X002016P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P530A     1    1    3   5    8  12  25  40 200 100 250    800
  % Range: 95.4-97.4%  Optimum: 99.4%  Hit Frequency: 42.5%
     Programs Available: X002016P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002016p.u66",   0x00000, 0x10000, CRC(77fcac28) SHA1(2d9ea5aea24295d74a3257a217717ddfe3b99736) ) /* Full House Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2017p ) /* Superboard : Fullhouse Bonus Poker (X002017P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P531A     1    1    3   5    7  12  25  40 200 100 250    800
  % Range: 93.7-95.7%  Optimum: 97.7%  Hit Frequency: 43.1%
     Programs Available: X002017P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002017p.u66",   0x00000, 0x10000, CRC(16ac0b5b) SHA1(9d92b66cec4cea72bf2c04677691cc7343676d6f) ) /* Full House Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2018p ) /* Superboard : Fullhouse Bonus Poker (X002018P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P532A     1    1    3   5    6  12  25  40 200 100 250    800
  % Range: 92.4-94.4%  Optimum: 96.4%  Hit Frequency: 44.1%
     Programs Available: X002018P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002018p.u66",   0x00000, 0x10000, CRC(a7b79cfa) SHA1(89216fafffc64fda22a016a906483b76174c3f02) ) /* Full House Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2021p ) /* Superboard : Lucky Deal Poker (X002021P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P566AL    1    1    3   5    7  10  25  40  80  50 250    800
  % Range: 94.2-98.2%  Optimum: 98.2%  Hit Frequency: 43.0%
     Programs Available: X002021P

Straights or Better on the initial deal PAY DOUBLE!

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002021p.u66",   0x00000, 0x10000, CRC(bf7f6f41) SHA1(c04fcab15da546929f8e15037f33cd99da4ae286) ) /* Lucky Deal Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2024p ) /* Superboard : Double Bonus Poker (X002024P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P325A     1    2    3   4    5   9  50  80 160  50 250    800
  % Range: 93.8-95.8%  Optimum: 97.8%  Hit Frequency: 44.5%
     Programs Available: X002024P

NOTE: Same as X000516P, except MAX Coin (set to 5) CANNOT be changed - NJ jurisdiction
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002024p.u66",   0x00000, 0x10000, CRC(8f9ab38c) SHA1(703f1f5c5ca1ab7032019e41da6ffac6fc47929a) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2025p ) /* Superboard : Deuces Wild Bonus Poker (X002025P+XP000019) */
/*
                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
  P552A     1   1    3   4   4   9  25  20  40  80 200 400 250    800
  % Range: 95.5-97.5%  Optimum: 99.5%  Hit Frequency: 41.1%
     Programs Available: X002025P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000019.u67",   0x00000, 0x10000, CRC(8ac876eb) SHA1(105b4aee2692ccb20795586ccbdf722c59db66cf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002025p.u66",   0x00000, 0x10000, CRC(f3dac423) SHA1(e9394d330deb3b8a1001e57e72a506cd9098f161) ) /* Deuces Wild Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2026p ) /* Superboard : Deuces Wild Bonus Poker (X002026P+XP000019) */
/*
                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
  P553A     1   1    3   3   4  13  25  20  40  80 200 400 250    800
  % Range: 94.8-96.8%  Optimum: 98.8%  Hit Frequency: 44.6%
     Programs Available: X002026P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000019.u67",   0x00000, 0x10000, CRC(8ac876eb) SHA1(105b4aee2692ccb20795586ccbdf722c59db66cf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002026p.u66",   0x00000, 0x10000, CRC(7fcbc10a) SHA1(5d50b356ae1a3461a5916b469f85b690b086e675) ) /* Deuces Wild Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2027p ) /* Superboard : Deuces Wild Bonus Poker (X002027P+XP000019) */
/*
                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
  P554A     1   1    3   3   4  10  25  20  40  80 200 400 250    800
  % Range: 93.4-95.4%  Optimum: 97.4%  Hit Frequency: 44.6%
     Programs Available: X002027P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000019.u67",   0x00000, 0x10000, CRC(8ac876eb) SHA1(105b4aee2692ccb20795586ccbdf722c59db66cf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002027p.u66",   0x00000, 0x10000, CRC(40dbc35a) SHA1(56bf79738e35a22d1f23d76cd6197c8949eba3fb) ) /* Deuces Wild Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2028p ) /* Superboard : Deuces Wild Bonus Poker (X002028P+XP000019) */
/*
                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
  P555A     1   1    2   3   4  12  25  20  40  80 200 400 250    800
  % Range: 92.2-94.2%  Optimum: 96.2%  Hit Frequency: 44.9%
     Programs Available: X002028P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000019.u67",   0x00000, 0x10000, CRC(8ac876eb) SHA1(105b4aee2692ccb20795586ccbdf722c59db66cf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002028p.u66",   0x00000, 0x10000, CRC(4f4c43f1) SHA1(8aa731ebb7981cb8e481db8b9376073881a09db2) ) /* Deuces Wild Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2029p ) /* Superboard : Deuces Wild Bonus Poker (X002029P+XP000019) */
/*
                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
  P556A     1   1    2   3   4  10  25  20  40  80 200 400 250    800
  % Range: 91.3-93.3%  Optimum: 95.3%  Hit Frequency: 45.0%
     Programs Available: X002029P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000019.u67",   0x00000, 0x10000, CRC(8ac876eb) SHA1(105b4aee2692ccb20795586ccbdf722c59db66cf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002029p.u66",   0x00000, 0x10000, CRC(e2f6fb89) SHA1(4b60b580b00b4268d1cb9065ffe0d21f8fa6a931) ) /* Deuces Wild Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2031p ) /* Superboard : Lucky Deal Poker (X002031P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P566AL    1    1    3   5    7   9  25  40  80  50 250    800
  % Range: 93.0-95.0%  Optimum: 97.0%  Hit Frequency: 42.9%
     Programs Available: X002031P

Straights or Better on the initial deal PAY DOUBLE!

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002031p.u66",   0x00000, 0x10000, CRC(c883fbdd) SHA1(c0444ffdac1ffe542d0d6a65f3c810346e2b2e05) ) /* Lucky Deal Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2035p ) /* Superboard : White Hot Aces Poker (X002035P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P581A     1    1    3   4    5   7  50 120 240  80 250    800
  % Range: 93.4-95.4%  Optimum: 97.4%  Hit Frequency: 44.7%
     Programs Available: X002035P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002035p.u66",   0x00000, 0x10000, CRC(dc3f0742) SHA1(d57cf3353b81f41895458019e47203f98645f17a) ) /* White Hot Aces Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2036p ) /* Superboard : White Hot Aces Poker (X002036P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P582A     1    1    3   4    5   6  50 120 240  80 250    800
  % Range: 92.4-94.4%  Optimum: 96.4%  Hit Frequency: 44.7%
     Programs Available: X002036P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002036p.u66",   0x00000, 0x10000, CRC(69207baf) SHA1(fe038b969106ae5cdc8dde1c06497be9c7b5b8bf) ) /* White Hot Aces Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2037p ) /* Superboard : Nevada Bonus Poker (X002037P+XP000038) */
/*
                                         2-K
PayTable   Js+  2PR  3K  3A  STR  FL  FH  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P575A     1    1    3   6   5    8  11  25 200 100 250    800
  % Range: 95.8-97.8%  Optimum: 99.8%  Hit Frequency: 42.7%
     Programs Available: X002037P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002037p.u66",   0x00000, 0x10000, CRC(12aea90e) SHA1(26ff0e7b81271252573739f26db9d20f35af274b) ) /* Nevada Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2038p ) /* Superboard : Nevada Bonus Poker (X002038P+XP000038) */
/*
                                         2-K
PayTable   Js+  2PR  3K  3A  STR  FL  FH  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P576A     1    1    3   6   5    7  10  25 200 100 250    800
  % Range: 93.2-95.2%  Optimum: 97.2%  Hit Frequency: 43.4%
     Programs Available: X002038P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002038p.u66",   0x00000, 0x10000, CRC(58d01ba5) SHA1(6d4cde9c9e55967db2b661c7123cce9958a00639) ) /* Nevada Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2039p ) /* Superboard : Nevada Bonus Poker (X002039P+XP000038) */
/*
                                         2-K
PayTable   Js+  2PR  3K  3A  STR  FL  FH  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P577A     1    1    3   6   5    7   9  25 200 100 250    800
  % Range: 92.2-96.2%  Optimum: 96.2%  Hit Frequency: 43.4%
     Programs Available: X002039P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002039p.u66",   0x00000, 0x10000, CRC(98cb0d98) SHA1(ff5d7b085c8b11987c32e6639f013304b55cb2bc) ) /* Nevada Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2040p ) /* Superboard : Nevada Bonus Poker (X002040P+XP000038) */
/*
                                         2-K
PayTable   Js+  2PR  3K  3A  STR  FL  FH  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P578A     1    1    3   6   5    6   9  25 200 100 250    800
  % Range: 90.8-92.8%  Optimum: 94.8%  Hit Frequency: 44.2%
     Programs Available: X002040P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002040p.u66",   0x00000, 0x10000, CRC(38acb477) SHA1(894f5861ac84323e50e8972602251f2873988e6c) ) /* Nevada Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2042p ) /* Superboard : Triple Bonus Poker (X002042P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P604A     1    1    3   4    7  11  75 120 240  50 250    800
  % Range: 95.6-97.6%  Optimum: 99.6%  Hit Frequency: 34.7%
     Programs Available: X002042P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002042p.u66",   0x00000, 0x10000, CRC(12787a36) SHA1(4ef478cc5c23694ffa6733e2fc47c2b6f545c30a) ) /* Triple Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2043p ) /* Superboard : Triple Bonus Poker (X002043P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P605A     1    1    3   4    7  10  75 120 240  50 250    800
  % Range: 94.5-96.5%  Optimum: 98.5%  Hit Frequency: 34.7%
     Programs Available: X002043P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002043p.u66",   0x00000, 0x10000, CRC(2cec81ab) SHA1(d28fa9075c63e49d34c463f19e7741609b0b3343) ) /* Triple Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2044p ) /* Superboard : Triple Bonus Poker (X002044P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P606A     1    1    3   4    7   9  75 120 240  50 250    800
  % Range: 93.5-95.5%  Optimum: 97.5%  Hit Frequency: 34.7%
     Programs Available: X002044P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002044p.u66",   0x00000, 0x10000, CRC(158af97f) SHA1(452247d981f1202da8c44a31f0d3343184d3db41) ) /* Triple Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2045p ) /* Superboard : Triple Bonus Poker (X002045P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P607A     1    1    3   4    6   9  75 120 240  50 250    800
  % Range: 91.9-93.9%  Optimum: 95.9%  Hit Frequency: 32.5%
     Programs Available: X002045P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002045p.u66",   0x00000, 0x10000, CRC(75fe81db) SHA1(980bcc06b54a1ef78e3beac1db83b73e17a04818) ) /* Triple Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2066p ) /* Superboard : Double Double Bonus Poker (X002066P+XP000038) */
/*
                                                  2-4
                                                   4K    4A
                                      5-K 2-4     with   with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  A,2-4  2-4  SF  RF  (Bonus)
-----------------------------------------------------------------------------
  P505A     1    1    3   4    6   9  50  80 160   160   400  50 250    800
  % Range: 95.0-97.0%  Optimum: 99.0%  Hit Frequency: 44.7%
     Programs Available: X002066P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002066p.u66",   0x00000, 0x10000, CRC(01236011) SHA1(3edfee014705b3540386c5e42026ab93628b2597) ) /* Double Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2067p ) /* Superboard : Double Double Bonus Poker (X002067P+XP000038) */
/*
                                                  2-4
                                                   4K    4A
                                      5-K 2-4     with   with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  A,2-4  2-4  SF  RF  (Bonus)
-----------------------------------------------------------------------------
  P505A     1    1    3   4    5   9  50  80 160   160   400  50 250    800
  % Range: 93.9-95.9%  Optimum: 97.9%  Hit Frequency: 44.8%
     Programs Available: X002067P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002067p.u66",   0x00000, 0x10000, CRC(c62adf21) SHA1(25e037f91ef1860d1d45cae12f21bdd1c39ba264) ) /* Double Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2068p ) /* Superboard : Double Double Bonus Poker (X002068P+XP000038) */
/*
                                                   2-4    4A
                                      5-K 2-4      4K    with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  with A  2-4  SF  RF  (Bonus)
------------------------------------------------------------------------------
  P506A     1    1    3   4    5   8  50  80 160   160    400  50 250    800
  % Range: 92.8-94.8%  Optimum: 96.8%  Hit Frequency: 44.8%
     Programs Available: X002068P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002068p.u66",   0x00000, 0x10000, CRC(a5e4279d) SHA1(c4dfd1e77a03a94d9911d1754b5874200bfe6c71) ) /* Double Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2069p ) /* Superboard : Double Double Bonus Poker (X002069P+XP000038) */
/*
                                                  2-4
                                                   4K    4A
                                      5-K 2-4     with   with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  A,2-4  2-4  SF  RF  (Bonus)
-----------------------------------------------------------------------------
  P507A     1    1    3   4    5   7  50  80 160   160   400  50 250    800
  % Range: 91.6-93.6%  Optimum: 95.6%  Hit Frequency: 45.0%
     Programs Available: X002069P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002069p.u66",   0x00000, 0x10000, CRC(875ecfcf) SHA1(80472cb43b36e518216e60a9b4883a81163718a2) ) /* Double Double Bonus Poker - 08/17/95   @ IGT  L95-2088 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2070p ) /* Superboard : Double Double Bonus Poker (X002070P+XP000038) */
/*
                                                  2-4
                                                   4K    4A
                                      5-K 2-4     with   with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  A,2-4  2-4  SF  RF  (Bonus)
-----------------------------------------------------------------------------
  P508A     1    1    3   4    5   6  50  80 160   160   400  50 250    800
  % Range: 90.5-92.5%  Optimum: 94.5%  Hit Frequency: 45.0%
     Programs Available: X002070P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002070p.u66",   0x00000, 0x10000, CRC(e7732ff9) SHA1(de1a68040d46e78831a6c806f26f253b4ab014b5) ) /* Double Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2111p ) /* Superboard : 4 of a Kind Bonus Poker (X002111P+XP000038) */
/*
                                     5-K  2-4            Seq
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  RF      (Bonus)
-------------------------------------------------------------------------
  ????     1     2    3   4    5   7  25  40  80  50 250 250  800 / 10000
  % Range: 92.6-94.6%  Optimum: 98.6%  Hit Frequency: 45.0%
     Programs Available: X002111P

NOTE: Royal Flush bonus is 800 x MAX bet, Sequential Royal Flush is 10000 x MAX bet
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002111p.u66",   0x00000, 0x10000, CRC(f19dd63e) SHA1(0fe16cd0c75a9759e34bf95ce428e5b2da236215) ) /* 4 of a Kind Bonus Poker with Sequential Royal Flush - 01/17/96   @ IGT  L96-0184 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2121p ) /* Superboard : Standard Draw Poker (X002121P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   PE       1    2    3    4    6   9  25  50 300    900
  % Range: 95.8-97.8%  Optimum: 99.8%  Hit Frequency: 45.4%
     Programs Available: PP0038, X002121P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002121p.u66",   0x00000, 0x10000, CRC(53e5853b) SHA1(a66dc531ac762d8acd2ac5d4228c9c04fc9758df) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2121pa ) /* Superboard : Standard Draw Poker (X002121P+XP000037) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   PE       1    2    3    4    6   9  25  50 300    900
  % Range: 95.8-97.8%  Optimum: 99.8%  Hit Frequency: 45.4%
     Programs Available: PP0038, X002121P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000037.u67",   0x00000, 0x10000, CRC(9ee67f20) SHA1(8f23a5593efbd21894b99196889cd560871c4615) ) /* Monaco Region */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002121p.u66",   0x00000, 0x10000, CRC(53e5853b) SHA1(a66dc531ac762d8acd2ac5d4228c9c04fc9758df) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2201.u77",  0x00000, 0x8000, CRC(8f82a114) SHA1(dc4aaaa12442a66386d9bef969afa60a7e2e386b) ) /* Monaco Region */
	ROM_LOAD( "mgo-cg2201.u78",  0x08000, 0x8000, CRC(71797c5b) SHA1(15dff00aad8006855af98a2ad39fe1a6e87d7d24) )
	ROM_LOAD( "mbo-cg2201.u79",  0x10000, 0x8000, CRC(27201cbf) SHA1(9d197e04c36e94ff08bd76c7200ea4e8f345b8ab) )
	ROM_LOAD( "mxo-cg2201.u80",  0x18000, 0x8000, CRC(b79b6d11) SHA1(dcc30465e4de104c54b19e95e7216023576d90c7) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2150p ) /* Superboard : 4 of a Kind Bonus Poker (X002105P+XP000038) - $1 Denominations Only */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
 P769BA     1    1    3   4    6   9  75 120 240  50 250    800
  % Range: 91.9-93.9%  Optimum: 95.9%  Hit Frequency: 32.5%
     Programs Available: X002150P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002150p.u66",   0x00000, 0x10000, CRC(b4b531c4) SHA1(f5fa988d963cb0fe00aebc4eb99043d1b70f9516) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2172p ) /* Superboard : Ace$ Bonus Poker (X002172P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SA$  SF  RF  (Bonus)
----------------------------------------------------------------------
  P789AM    1    2    3   4    5   7  25  40  80  250  50 250    800
  % Range: 94.3-96.3%  Optimum: 98.3%  Hit Frequency: 45.5%
     Programs Available: X002172P

SA$ - Sequential ACE$ pays the same as RF

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002172p.u66",   0x00000, 0x10000, CRC(d4c44409) SHA1(8082b76a51fa131751b8c2c446cb29fb04f531dc) ) /* Ace$ Bonus Poker - 05/09/96   @ IGT  L96-1119 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2172.u77",  0x00000, 0x8000, CRC(45c84d0b) SHA1(796b6ff14adacf1cbca6acd9b4903d7420519f2b) ) /*  06/05/95   @ IGT  L95-1494  */
	ROM_LOAD( "mgo-cg2172.u78",  0x08000, 0x8000, CRC(fff049bc) SHA1(d4b340e0c932db767cade56a55b649c80750fee4) )
	ROM_LOAD( "mbo-cg2172.u79",  0x10000, 0x8000, CRC(1665c6be) SHA1(4650b9d8336434ce531c31b63e7780b47ef4e985) )
	ROM_LOAD( "mxo-cg2172.u80",  0x18000, 0x8000, CRC(cc9d64e4) SHA1(4bf78e49150ca6fb4a05f3320f712bbbabcacf56) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2172.u43", 0x0000, 0x0200, CRC(c9b4f1da) SHA1(c911da564cfb33218441de0f91e67a02191d76c2) ) /* Philips N82S147N */
ROM_END

ROM_START( pex2172pa ) /* Superboard : Ace$ Bonus Poker (X002172P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SA$  SF  RF  (Bonus)
----------------------------------------------------------------------
  P789AM    1    2    3   4    5   7  25  40  80  250  50 250    800
  % Range: 94.3-96.3%  Optimum: 98.3%  Hit Frequency: 45.5%
     Programs Available: X002172P

SA$ - Sequential ACE$ pays the same as RF

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002172p.u66",   0x00000, 0x10000, CRC(d4c44409) SHA1(8082b76a51fa131751b8c2c446cb29fb04f531dc) ) /* Ace$ Bonus Poker - 05/09/96   @ IGT  L96-1119 */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2399.u77",  0x00000, 0x8000, CRC(0424f4ba) SHA1(c8b192a6f63c8c9937cb3923d27b9ba2c39823cd) ) /* Custom The Fun Ships card backs */
	ROM_LOAD( "mgo-cg2399.u78",  0x08000, 0x8000, CRC(5848a2fa) SHA1(4173a473016b7a776d2b59bf3ded0be35bd43721) ) /* Also compatible with ACE$ Bonus Poker */
	ROM_LOAD( "mbo-cg2399.u79",  0x10000, 0x8000, CRC(5c3e16f6) SHA1(a4aa457f239527bffa6472e0d6f9d836f796b326) )
	ROM_LOAD( "mxo-cg2399.u80",  0x18000, 0x8000, CRC(bd7669d5) SHA1(4343a9764fd563e2e1cdd8558f2f53f77006b159) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2399.u43", 0x0000, 0x0200, NO_DUMP ) /* Should be CAPX2399 */
	ROM_LOAD( "capx2172.u43", 0x0000, 0x0200, CRC(c9b4f1da) SHA1(c911da564cfb33218441de0f91e67a02191d76c2) ) /* Wrong!! Should be CAPX2399 - Philips N82S147N */
ROM_END

ROM_START( pex2173p ) /* Superboard : Ace$ Bonus Poker (X002173P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SA$  SF  RF  (Bonus)
----------------------------------------------------------------------
  P790AM    1    2    3   4    5   6  25  40  80  250  50 250    800
  % Range: 93.1-95.1%  Optimum: 97.1%  Hit Frequency: 45.5%
     Programs Available: X002173P

SA$ - Sequential ACE$ pays the same as RF

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002173p.u66",   0x00000, 0x10000, CRC(509e50e1) SHA1(db4ee11f29107ff48ee5f5410c9d0e1a6a6fd567) ) /* Ace$ Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2172.u77",  0x00000, 0x8000, CRC(45c84d0b) SHA1(796b6ff14adacf1cbca6acd9b4903d7420519f2b) ) /*  06/05/95   @ IGT  L95-1494  */
	ROM_LOAD( "mgo-cg2172.u78",  0x08000, 0x8000, CRC(fff049bc) SHA1(d4b340e0c932db767cade56a55b649c80750fee4) )
	ROM_LOAD( "mbo-cg2172.u79",  0x10000, 0x8000, CRC(1665c6be) SHA1(4650b9d8336434ce531c31b63e7780b47ef4e985) )
	ROM_LOAD( "mxo-cg2172.u80",  0x18000, 0x8000, CRC(cc9d64e4) SHA1(4bf78e49150ca6fb4a05f3320f712bbbabcacf56) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2172.u43", 0x0000, 0x0200, CRC(c9b4f1da) SHA1(c911da564cfb33218441de0f91e67a02191d76c2) ) /* Philips N82S147N */
ROM_END

ROM_START( pex2179p ) /* Superboard : Double Bonus Poker (X002179P+XP000119) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P435A     1    1    3   4    5   8  50  80 160  50 250    800
  % Range: 90.2-92.2%  Optimum: 94.2%  Hit Frequency: 45.1%
     Programs Available: X002179P

NOTE: Same as X000726P, except MAX Coin (set to 5) CANNOT be changed - NJ jurisdiction
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000119.u67",   0x00000, 0x10000, CRC(f3e7b14c) SHA1(b225900afca61740e842743453346aaa7ed34687) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002179p.u66",   0x00000, 0x10000, CRC(8bdbdb31) SHA1(4ce2a652fd14b830f4d56883358f5dcb9fd29e36) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2180p ) /* Superboard : Double Bonus Poker (X002180P+XP000038) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P437A     1    1    3   4    5   6  50  80 160  50 250    800
  % Range: 88.0-90.0%  Optimum: 92.0%  Hit Frequency: 45.1%
     Programs Available: X002180P

NOTE: Same as X000728P, except MAX Coin (set to 5) CANNOT be changed - NJ jurisdiction
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002180p.u66",   0x00000, 0x10000, CRC(c929b3a5) SHA1(6b47c67275f6894bfcd52640aa32fbe1270dba9c) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2241p ) /* Superboard : 4 of a Kind Bonus Poker 1-100 Coins (X002241P+XP000079) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  ????      1    2    3    4    5   8  25  40  80  50 800    800
  % Range: 95.2-97.2%  Optimum: 99.2%  Hit Frequency: 45.5%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000079.u67",   0x00000, 0x10000, CRC(fe9757b7) SHA1(8547f00f23e2e3cd4b36d006b760eca6a19f0710) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002241p.u66",   0x00000, 0x10000, CRC(c6b45cf4) SHA1(3d47e8ff5c915c4b35e4a2ffe20fcc950e838454) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2242.u77",  0x00000, 0x8000, CRC(963a7e7d) SHA1(ebb159f6c731a3f912382745ef9a9c6d4fa2fc99) )
	ROM_LOAD( "mgo-cg2242.u78",  0x08000, 0x8000, CRC(53eed56f) SHA1(e79f31c5c817b8b96b4970c1a702d1892961d441) )
	ROM_LOAD( "mbo-cg2242.u79",  0x10000, 0x8000, CRC(af092f50) SHA1(53a3536593bb14c4072e8a5ee9e05af332feceb1) )
	ROM_LOAD( "mxo-cg2242.u80",  0x18000, 0x8000, CRC(ecacb6b2) SHA1(32660adcc266fbbb3702a0cd30e25d11b953d23d) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2244p ) /* Superboard : Double Bonus Poker 1-100 Coins (X002244P+XP000079) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  ????      1    1    3    5    7   9  50  80 160  50 800    800
  % Range: 91.2-97.1%  Optimum: 99.1%  Hit Frequency: 43.2%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000079.u67",   0x00000, 0x10000, CRC(fe9757b7) SHA1(8547f00f23e2e3cd4b36d006b760eca6a19f0710) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002244p.u66",   0x00000, 0x10000, CRC(06a3e60d) SHA1(8abeafab589406ff68898e8f90431c1a5f8d2de5) ) /* Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2242.u77",  0x00000, 0x8000, CRC(963a7e7d) SHA1(ebb159f6c731a3f912382745ef9a9c6d4fa2fc99) )
	ROM_LOAD( "mgo-cg2242.u78",  0x08000, 0x8000, CRC(53eed56f) SHA1(e79f31c5c817b8b96b4970c1a702d1892961d441) )
	ROM_LOAD( "mbo-cg2242.u79",  0x10000, 0x8000, CRC(af092f50) SHA1(53a3536593bb14c4072e8a5ee9e05af332feceb1) )
	ROM_LOAD( "mxo-cg2242.u80",  0x18000, 0x8000, CRC(ecacb6b2) SHA1(32660adcc266fbbb3702a0cd30e25d11b953d23d) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2245p ) /* Superboard : Standard Draw Poker (X002245P+XP000055) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   6  25  50 800    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: X002245P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000055.u67",   0x00000, 0x10000, CRC(339821e0) SHA1(127d4eff01136feaf1e3242d57433349afb7b6ca) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002245p.u66",   0x00000, 0x10000, CRC(df49a54c) SHA1(1c89d0a114e27e27117120c9e2fc36b124fe7761) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2242.u77",  0x00000, 0x8000, CRC(963a7e7d) SHA1(ebb159f6c731a3f912382745ef9a9c6d4fa2fc99) )
	ROM_LOAD( "mgo-cg2242.u78",  0x08000, 0x8000, CRC(53eed56f) SHA1(e79f31c5c817b8b96b4970c1a702d1892961d441) )
	ROM_LOAD( "mbo-cg2242.u79",  0x10000, 0x8000, CRC(af092f50) SHA1(53a3536593bb14c4072e8a5ee9e05af332feceb1) )
	ROM_LOAD( "mxo-cg2242.u80",  0x18000, 0x8000, CRC(ecacb6b2) SHA1(32660adcc266fbbb3702a0cd30e25d11b953d23d) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2245pa ) /* Superboard : Standard Draw Poker (X002245P+XP000079) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   6  25  50 800    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: X002245P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000079.u67",   0x00000, 0x10000, CRC(fe9757b7) SHA1(8547f00f23e2e3cd4b36d006b760eca6a19f0710) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002245p.u66",   0x00000, 0x10000, CRC(df49a54c) SHA1(1c89d0a114e27e27117120c9e2fc36b124fe7761) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2247p ) /* Superboard : Standard Draw Poker (X002247P+XP000038) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
   CD       1    2    3    4    6   9  25  50 250    940
  % Range: 95.9-97.9%  Optimum: 99.9%  Hit Frequency: 45.3%
     Programs Available: X002247P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002247p.u66",   0x00000, 0x10000, CRC(41aecb1a) SHA1(d4c5388b66b003e7f4449963d86f6b20f1954193) ) /* Standard Draw Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2250p ) /* Superboard : Shockwave Poker (X002250P+XP000050) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K* SF  RF  (Bonus)
---------------------------------------------------------
 P598BA     1    1    3   5    8  11  25 100 250    800
  % Range: 94.5-96.5%  Optimum: 98.5%  Hit Frequency: 42.6%
     Programs Available: X002250P

4K* - Getting a 4K hand sets the game in "Shockwave" mode for the next 10 dealt hands.
      While in shockwave mode, 4K pays the same as RF

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000050.u67",   0x00000, 0x10000, CRC(cf9e72d6) SHA1(fc5c679aae43df0bd563fbcc3e00a3274af1ed11) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002250p.u66",   0x00000, 0x10000, CRC(8d8810f9) SHA1(14262d83cf5f2511c3de7777336ac9df7270dab2) ) /* Shockwave Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2309.u77",  0x00000, 0x8000, CRC(fdef322c) SHA1(8024cb6fcba18b56168e853173b9856c4d011831) )
	ROM_LOAD( "mgo-cg2309.u78",  0x08000, 0x8000, CRC(f70b30c0) SHA1(e4acd0060b3d68b9f385cb60ed43a0988fca66a8) )
	ROM_LOAD( "mbo-cg2309.u79",  0x10000, 0x8000, CRC(1843eec7) SHA1(0d0b80cd4d458081394c2943023b2440c2c2e42c) )
	ROM_LOAD( "mxo-cg2309.u80",  0x18000, 0x8000, CRC(5c73d095) SHA1(078c6c815e8c48988f631d9d37018ea0b4bbfa19) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2309.u43", 0x0000, 0x0200, CRC(5da912cc) SHA1(6294f8be682e70e9052c9ae5f6865467e9dba2e3) )
ROM_END

ROM_START( pex2251p ) /* Superboard : Shockwave Poker (X002251P+XP000050) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K* SF  RF  (Bonus)
---------------------------------------------------------
  P719A     1    1    3   5    8  12  25 100 250    800
  % Range: 95.6-97.6%  Optimum: 99.6%  Hit Frequency: 42.6%
     Programs Available: X002251P

4K* - Getting a 4K hand sets the game in "Shockwave" mode for the next 10 dealt hands.
      While in shockwave mode, 4K pays the same as RF

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000050.u67",   0x00000, 0x10000, CRC(cf9e72d6) SHA1(fc5c679aae43df0bd563fbcc3e00a3274af1ed11) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002251p.u66",   0x00000, 0x10000, CRC(9069aa23) SHA1(299d5befce817e8334d4ac53470ff678775546ff) ) /* Shockwave Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2309.u77",  0x00000, 0x8000, CRC(fdef322c) SHA1(8024cb6fcba18b56168e853173b9856c4d011831) )
	ROM_LOAD( "mgo-cg2309.u78",  0x08000, 0x8000, CRC(f70b30c0) SHA1(e4acd0060b3d68b9f385cb60ed43a0988fca66a8) )
	ROM_LOAD( "mbo-cg2309.u79",  0x10000, 0x8000, CRC(1843eec7) SHA1(0d0b80cd4d458081394c2943023b2440c2c2e42c) )
	ROM_LOAD( "mxo-cg2309.u80",  0x18000, 0x8000, CRC(5c73d095) SHA1(078c6c815e8c48988f631d9d37018ea0b4bbfa19) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2309.u43", 0x0000, 0x0200, CRC(5da912cc) SHA1(6294f8be682e70e9052c9ae5f6865467e9dba2e3) )
ROM_END

ROM_START( pex2272p ) /* Superboard : Black Jack Bonus Poker (X002272P+XP000055) */
/*
Black Jack as in Jack of Spades/Clubs, not 21       With With
                                                     BJ   BJ  With
                                        5-K 2-4     5-K  2-4   BJ
PayTable   Js+ 2PR  STR  FL  FH  4K  SF  4K  4K  4A  4K   4K   4A  RF (Bonus)
-----------------------------------------------------------------------------
 P870BB     1   1    3    4   7   9  50  25  80 160 160  400  400 400   800
  % Range: 95.4-97.4%  Optimum: 99.4%  Hit Frequency: 43.5%
     Programs Available: X002272P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000055.u67",   0x00000, 0x10000, CRC(339821e0) SHA1(127d4eff01136feaf1e3242d57433349afb7b6ca) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002272p.u66",   0x00000, 0x10000, CRC(ee4f27b9) SHA1(1ee105430358ea27badd943bb6b18663e4029388) ) /* Black Jack Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2275p ) /* Superboard : Black Jack Bonus Poker (X002275P+XP000055) */
/*
Black Jack as in Jack of Spades/Clubs, not 21       With With
                                                     BJ   BJ  With
                                        5-K 2-4     5-K  2-4   BJ
PayTable   Js+ 2PR  STR  FL  FH  4K  SF  4K  4K  4A  4K   4K   4A  RF (Bonus)
-----------------------------------------------------------------------------
 P873BB     1   1    3    4   5   8  50  25  80 160 160  400  400 400   800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.8%
     Programs Available: X002275P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000055.u67",   0x00000, 0x10000, CRC(339821e0) SHA1(127d4eff01136feaf1e3242d57433349afb7b6ca) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002275p.u66",   0x00000, 0x10000, CRC(5ba4f5ab) SHA1(def069025ec4aa340646dfd7cfacc8ce836a210c) ) /* Black Jack Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2276p ) /* Superboard : Black Jack Bonus Poker (X002276P+XP000055) */
/*
Black Jack as in Jack of Spades/Clubs, not 21       With With
                                                     BJ   BJ  With
                                        5-K 2-4     5-K  2-4   BJ
PayTable   Js+ 2PR  STR  FL  FH  4K  SF  4K  4K  4A  4K   4K   4A  RF (Bonus)
-----------------------------------------------------------------------------
 P874BB     1   1    3    4   5   7  50  25  80 160 160  400  400 400   800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 44.9%
     Programs Available: X002276P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000055.u67",   0x00000, 0x10000, CRC(339821e0) SHA1(127d4eff01136feaf1e3242d57433349afb7b6ca) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002276p.u66",   0x00000, 0x10000, CRC(7a660f24) SHA1(d0b621a779bfc00668492cfb3dea65d1583fa4f1) ) /* Black Jack Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2283p ) /* Superboard : Barbaric Deuces Wild / Dealt Deuces Wild Bonus Poker (X002283P+XP000057) */
/*
                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
  P886A     1   1    3   3   4  10  25  20  40  80 200 400 250    800
  % Range: 94.6-96.6%  Optimum: 98.6%  Hit Frequency: 44.8%
     Programs Available: X002283P

Bonus "Dealt" payouts per Coin In are:
 Dealt 3 Deuces .....    3
 Dealt 4 Deuces .....  300
 Dealt 4 Deuces + Ace 1600

Belly glass can ordered as either Barbaric Deuces Wild or Dealt Deuces Wild Bonus Poker

Designed and co-created by Best Bet Products

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000057.u67",   0x00000, 0x10000, CRC(a1186020) SHA1(d42823aac1cb16521ecc0a09cba694374642cff7) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002283p.u66",   0x00000, 0x10000, CRC(90f7f7b3) SHA1(0c8460391303ed16f20c41472840d798950bb2c0) ) /* Barbaric Deuces Wild / Dealt Deuces Wild Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2325.u77",  0x00000, 0x8000, CRC(ae53d1f6) SHA1(bf28b8f784d6683bb352944b88d0b646d7313efd) )
	ROM_LOAD( "mgo-cg2325.u78",  0x08000, 0x8000, CRC(a637679e) SHA1(4cb24f1f907ae482419981cac49af19ca1cdbc99) )
	ROM_LOAD( "mbo-cg2325.u79",  0x10000, 0x8000, CRC(4a179b6d) SHA1(2ed51ed85444b939bbd48344f18fa97c146438ff) )
	ROM_LOAD( "mxo-cg2325.u80",  0x18000, 0x8000, CRC(afae8fd5) SHA1(7c6380f21fe8444234ada8d88a46d3a4f1623b29) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2325.u43", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) ) /* Wrong!! Should be CAPX2325 */
ROM_END

ROM_START( pex2284p ) /* Superboard : Barbaric Deuces Wild / Dealt Deuces Wild Bonus Poker (X002284P+XP000057) */
/*
                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
  P885A     1   1    2   3   4  12  25  20  40  80 200 400 250    800
  % Range: 93.5-95.5%  Optimum: 97.5%  Hit Frequency: 45.1%
     Programs Available: X002284P

Bonus "Dealt" payouts per Coin In are:
 Dealt 3 Deuces .....    3
 Dealt 4 Deuces .....  300
 Dealt 4 Deuces + Ace 1600

Belly glass can ordered as either Barbaric Deuces Wild or Dealt Deuces Wild Bonus Poker

Designed and co-created by Best Bet Products

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000057.u67",   0x00000, 0x10000, CRC(a1186020) SHA1(d42823aac1cb16521ecc0a09cba694374642cff7) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002284p.u66",   0x00000, 0x10000, CRC(2a3cb2a9) SHA1(76bfbf9a25913604454142716e1433ec73f0f0c9) ) /* Barbaric Deuces Wild / Dealt Deuces Wild Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2325.u77",  0x00000, 0x8000, CRC(ae53d1f6) SHA1(bf28b8f784d6683bb352944b88d0b646d7313efd) )
	ROM_LOAD( "mgo-cg2325.u78",  0x08000, 0x8000, CRC(a637679e) SHA1(4cb24f1f907ae482419981cac49af19ca1cdbc99) )
	ROM_LOAD( "mbo-cg2325.u79",  0x10000, 0x8000, CRC(4a179b6d) SHA1(2ed51ed85444b939bbd48344f18fa97c146438ff) )
	ROM_LOAD( "mxo-cg2325.u80",  0x18000, 0x8000, CRC(afae8fd5) SHA1(7c6380f21fe8444234ada8d88a46d3a4f1623b29) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2325.u43", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) ) /* Wrong!! Should be CAPX2325 */
ROM_END

ROM_START( pex2287p ) /* Superboard : Pay the Aces NO Faces Bonus (X002287P+XP000057) */
/*

                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  P888A     1    1    2    4    5   8  25  40  80  50 250    800
  % Range: 94.4-96.4%  Optimum: 98.4%  Hit Frequency: 55.6%
     Programs Available: X002287P

Bonus "Dealt" payouts per Coin In are:
 Dealt Royal Flush     ..... 1000
 Dealt 1 Ace no Faces  .....    1
 Dealt 2 Aces no Faces .....    2
 Dealt 3 Aces no Faces .....   50
 Dealt 4 Aces no Faces ..... 1500

Designed and co-created by Best Bet Products

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000057.u67",   0x00000, 0x10000, CRC(a1186020) SHA1(d42823aac1cb16521ecc0a09cba694374642cff7) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002287p.u66",   0x00000, 0x10000, CRC(f5a8f485) SHA1(4bf9ad2a75acd5445e97661efe8a39ceb8b97549) ) /* Pay the Acse NO Faces Bonus */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2325.u77",  0x00000, 0x8000, CRC(ae53d1f6) SHA1(bf28b8f784d6683bb352944b88d0b646d7313efd) )
	ROM_LOAD( "mgo-cg2325.u78",  0x08000, 0x8000, CRC(a637679e) SHA1(4cb24f1f907ae482419981cac49af19ca1cdbc99) )
	ROM_LOAD( "mbo-cg2325.u79",  0x10000, 0x8000, CRC(4a179b6d) SHA1(2ed51ed85444b939bbd48344f18fa97c146438ff) )
	ROM_LOAD( "mxo-cg2325.u80",  0x18000, 0x8000, CRC(afae8fd5) SHA1(7c6380f21fe8444234ada8d88a46d3a4f1623b29) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2325.u43", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) ) /* Wrong!! Should be CAPX2325 */
ROM_END

ROM_START( pex2297p ) /* Superboard : Jackpot Poker (X002297P+XP000053) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  P820AU    1    2    3    4    5   8  **  50 250    800
  % Range: 95.7-97.7%  Optimum: 99.7%  Hit Frequency: 45.5%
     Programs Available: X002297P

** 4K goes to a slot machine bonus routine. The average Jackpot Poker pay = 35.15 per Coin In
   Possible payouts are: 30, 26, 40, 80 & 400 per Coin In

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002297p.u66",   0x00000, 0x10000, CRC(7ebe809e) SHA1(5aafdf499455b8c96f6d780894cc442ed21e0dc2) ) /* Jackpot Poker */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2292.u77",  0x00000, 0x10000, CRC(10da5df3) SHA1(0975acc2151957c0d9996d5cc77ded6deefbb41a) )
	ROM_LOAD( "mgo-cg2292.u78",  0x10000, 0x10000, CRC(92fc6282) SHA1(8d765326dd604274dcf36e38440a1f9c404a020a) )
	ROM_LOAD( "mbo-cg2292.u79",  0x20000, 0x10000, CRC(996ab79c) SHA1(54469ce8de6aa35d6be996fc87b677d75f7cfa68) )
	ROM_LOAD( "mxo-cg2292.u80",  0x30000, 0x10000, CRC(d7efa5c9) SHA1(40aa7593b358c99f3f98c5d5ad11e186aff17b58) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2292.u43", 0x0000, 0x0200, CRC(2836f00b) SHA1(c6ff79977ed2eb24fdbfa378c72d44b9ec05a40f) )
ROM_END

ROM_START( pex2302p ) /* Superboard : Bonus Poker Deluxe (X002302P+XP000038) */
/*
PayTable   Js+  2PR  3K  STR  FL  FH  4K  SF  RF  (Bonus)
---------------------------------------------------------
  P902A     1    1    3   4    5   6  80  50 250    800
  % Range: 91.4-93.4%  Optimum: 95.4%  Hit Frequency: 45.2%
     Programs Available: X002302P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002302p.u66",   0x00000, 0x10000, CRC(8e52646e) SHA1(f6722778eb7e2981a00f8e4e5ea32f71a35e20e5) ) /* Bonus Poker Deluxe */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2303p ) /* Superboard : White Hot Aces Poker (X002303P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P903A     1    1    3   4    5   5  50 120 240  80 250    800
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 44.7%
     Programs Available: X002303P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002303p.u66",   0x00000, 0x10000, CRC(81cfd71b) SHA1(485a45412cad705d050b369c4cd1472a438374e8) ) /* White Hot Aces Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2306p ) /* Superboard : Triple Double Bonus Poker (X002306P+XP000112) */
/*
                                                  2-4
                                                   4K    4A
                                      5-K 2-4     with   with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  A,2-4  2-4  SF  RF  (Bonus)
-----------------------------------------------------------------------------
 P908BM     1    1    2   4    7   9  50  80 160   400   400  50 400    800
  % Range: 95.6-97.6%  Optimum: 99.6%  Hit Frequency: 43.3%
     Programs Available: X002306P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002306p.u66",   0x00000, 0x10000, CRC(ef36ea67) SHA1(8914ad20526fd63e14d9fa1901e9c779a11eb29d) ) /* Triple Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2307p ) /* Superboard : Triple Double Bonus Poker (X002307P+XP000112) */
/*
                                                  2-4
                                                   4K    4A
                                      5-K 2-4     with   with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  A,2-4  2-4  SF  RF  (Bonus)
-----------------------------------------------------------------------------
 P908BM     1    1    3   4    6   9  50  80 160   400   400  50 400    800
  % Range: 94.2-96.2%  Optimum: 98.2%  Hit Frequency: 44.1%
     Programs Available: X002307P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002307p.u66",   0x00000, 0x10000, CRC(c6d5db70) SHA1(017e1e382fb789e4cd8b410362ad5e82b61f61db) ) /* Triple Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2308p ) /* Superboard : Triple Double Bonus Poker (X002308P+XP000112) */
/*
                                                  2-4
                                                   4K    4A
                                      5-K 2-4     with   with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  A,2-4  2-4  SF  RF  (Bonus)
-----------------------------------------------------------------------------
 P909BM     1    1    2   4    5   9  50  80 160   400   400  50 400    800
  % Range: 93.0-95.0%  Optimum: 97.0%  Hit Frequency: 44.6%
     Programs Available: X002308P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002308p.u66",   0x00000, 0x10000, CRC(632fe9e4) SHA1(bb99a610f42aa32ad4729bb2bb4b99b4070977cf) ) /* Triple Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2310p ) /* Superboard : Triple Double Bonus Poker (X002310P+XP000112) */
/*
                                                  2-4
                                                   4K    4A
                                      5-K 2-4     with   with
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  A,2-4  2-4  SF  RF  (Bonus)
-----------------------------------------------------------------------------
 P911BM     1    1    2   4    5   7  50  80 160   400   400  50 400    800
  % Range: 90.9-92.9%  Optimum: 94.9%  Hit Frequency: 44.5%
     Programs Available: X002310P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002310p.u66",   0x00000, 0x10000, CRC(c006c3f1) SHA1(45c87a2f882147d1d132237cfa12ae47b202264f) ) /* Triple Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2312p ) /* Superboard : Triple Bonus Poker Plus (X002312P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P919BA    1    1    3   4    5   8  50 120 240 100 250    800
  % Range: 94.7-96.7%  Optimum: 98.7%  Hit Frequency: 44.7%
     Programs Available: X002312P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002312p.u66",   0x00000, 0x10000, CRC(d3b405b5) SHA1(321ce89a12c7d4849379731f45482c567c25b3a1) ) /* Triple Bonus Poker Plus */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2314p ) /* Superboard : Triple Bonus Poker Plus (X002314P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  P919BA    1    1    3   4    5   6  50 120 240 100 250    800
  % Range: 92.6-94.6%  Optimum: 96.6%  Hit Frequency: 44.7%
     Programs Available: X002314P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002314p.u66",   0x00000, 0x10000, CRC(bfc0acf0) SHA1(a6b7c228a84d0ea224ad945964c53de2d44e4a8d) ) /* Triple Bonus Poker Plus */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2374p ) /* Superboard : Super Aces Poker (X002374P+XP000112) */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
 P956A      1    1    3   4    5   6  50  80 400  60 250    800
  % Range: 93.7-95.7%  Optimum: 97.8%  Hit Frequency: 44.8%
     Programs Available: X002374P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002374p.u66",   0x00000, 0x10000, CRC(fc4b6c8d) SHA1(b101f9042bd54dbfdeed4c7a3acf3798096f6857) ) /* Super Aces Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2324.u77",  0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",  0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",  0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",  0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2377p ) /* Superboard : Super Double Bonus Poker (X002377P+XP000112) */
/*
                                          2-4 J-K
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4K  4A  SF  RF  (Bonus)
---------------------------------------------------------------------
 P977A      1    1    3   4    5   6  50  80 120 160  80 250    800
  % Range: 92.9-94.9%  Optimum: 96.9%  Hit Frequency: 45.2%
     Programs Available: X002377P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002377p.u66",   0x00000, 0x10000, CRC(541320d2) SHA1(670b17432e994fe1937091e5e96e1d58b9afbf29) ) /* Super Double Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2244.u77",  0x00000, 0x8000, CRC(25561458) SHA1(fe5d624e0e16956df589f3682bad9181bdc99956) ) /*  */
	ROM_LOAD( "mgo-cg2244.u78",  0x08000, 0x8000, CRC(b2de0a7a) SHA1(34f0ef951560f6f71e14c822baa4ccb1028b5028) )
	ROM_LOAD( "mbo-cg2244.u79",  0x10000, 0x8000, CRC(d2c12418) SHA1(dfb1aebaac23ff6e2cf556f228dbdb7c272a1b30) )
	ROM_LOAD( "mxo-cg2244.u80",  0x18000, 0x8000, CRC(8dc10a99) SHA1(92edb31f44e52609ed1ba2a53577048d424c6238) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2386p ) /* Superboard : 4 of a Kind Bonus Poker (X002386P+XP000038) */
/*
                                       5-K 2-4
PayTable   Js+  2PR  3K   STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
------------------------------------------------------------------
  ????      1    2    3    4    5   7  25  40  80  50 500    500
  % Range: 93.3-95.3%  Optimum: 97.3%  Hit Frequency: 42.7%
     Programs Available: X002386P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* 09/05/95   @ IGT  L95-2452 */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002386p.u66",   0x00000, 0x10000, CRC(3b2731e4) SHA1(aefe0fc2c2baf653cf3dc0e1394afbb55fb18f61) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2412p ) /* Superboard : Standard Draw with 5 decks - Two Pair or Better (X002412P+XP000096) */
/*
  % Range: 93.7-95.7%  Optimum: 97.7%  Hit Frequency: 44.6%
     Programs Available: X002412P

NOTE: This version uses 5 separate decks of cards, one deck for each HOLD button.
      So things like a suited 5K are possible. Sadly, there's no paytable displayed
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000096.u67",   0x00000, 0x10000, CRC(5aca14e1) SHA1(13bcb8069f9d704983632bb60db119f7308f9d80) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002412p.u66",   0x00000, 0x10000, CRC(43c250d1) SHA1(868a8ab1795b05c2e57a9b8bf3b2b6be688783e9) ) /* Deuces Wild Bonus Poker - French */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2315.u77",  0x00000, 0x8000, CRC(2c2bb000) SHA1(46135803c9a3066aaaccbf998d91ae3270ab99c4) )
	ROM_LOAD( "mgo-cg2315.u78",  0x08000, 0x8000, CRC(0be4e10e) SHA1(e050f0386edf6810d8bebaeb442eb9386af1b86f) )
	ROM_LOAD( "mbo-cg2315.u79",  0x10000, 0x8000, CRC(0942c045) SHA1(1ce6a0b32eaea64f27d4ee998716e0fddb64baf4) )
	ROM_LOAD( "mxo-cg2315.u80",  0x18000, 0x8000, CRC(93e3b230) SHA1(7a672fa5bef43208e3870e51e29c3c7b1d02e262) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2315.u43", 0x0000, 0x0200, CRC(690869af) SHA1(e057ce63a687f566d3ef181ac1829107073783f7) )
ROM_END

ROM_START( pex2419p ) /* Superboard : Deuces Wild Bonus Poker - French (X002419P+XP000064) */
/*
   Same payouts as X002027P English Deuces Wild Bonus Poker:

                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
 PI554A     1   1    3   3   4  13  25  20  40  80 200 400 250    800
  % Range: 93.4-95.4%  Optimum: 97.4%  Hit Frequency: 44.6%
     Programs Available: X002419P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000064.u67",   0x00000, 0x10000, CRC(bb958158) SHA1(5d171ba71f70c668c70e4afd59ef7a0283798bbd) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002419p.u66",   0x00000, 0x10000, CRC(a9a686c2) SHA1(40b8e2f4a4fab58161f161292024cecd046cc206) ) /* Deuces Wild Bonus Poker - French */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2353.u77",  0x00000, 0x8000, CRC(7ed7f7cd) SHA1(406b124b3db5335acf8f8987afbfa10d90e04351) )
	ROM_LOAD( "mgo-cg2353.u78",  0x08000, 0x8000, CRC(aab4e5fb) SHA1(7d6e048dc1a9d01900ba71fc23c884637f5850f2) )
	ROM_LOAD( "mbo-cg2353.u79",  0x10000, 0x8000, CRC(119f59cd) SHA1(52283feb21b880960efef06c780d4e22b31ea18c) )
	ROM_LOAD( "mxo-cg2353.u80",  0x18000, 0x8000, CRC(3bb871c1) SHA1(0b9439fd6565c742c1c7dda23a80bdd1d91d7293) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2307.u43", 0x0000, 0x0200, CRC(58d81338) SHA1(f0044ebbd0128d6fb74d850528ef02730c180f00) )
ROM_END

ROM_START( pex2420p ) /* Superboard : Deuces Wild Bonus Poker - French (X002420P+XP000064) */
/*
   Same payouts as X002028P English Deuces Wild Bonus Poker:

                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
 PI555A     1   1    2   3   4  12  25  20  40  80 200 400 250    800
  % Range: 92.2-94.2%  Optimum: 96.2%  Hit Frequency: 44.9%
     Programs Available: X002420P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000064.u67",   0x00000, 0x10000, CRC(bb958158) SHA1(5d171ba71f70c668c70e4afd59ef7a0283798bbd) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002420p.u66",   0x00000, 0x10000, CRC(8ed6595a) SHA1(2250cc1a75074640443a1aded7cef041e61f0016) ) /* Deuces Wild Bonus Poker - French */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2353.u77",  0x00000, 0x8000, CRC(7ed7f7cd) SHA1(406b124b3db5335acf8f8987afbfa10d90e04351) )
	ROM_LOAD( "mgo-cg2353.u78",  0x08000, 0x8000, CRC(aab4e5fb) SHA1(7d6e048dc1a9d01900ba71fc23c884637f5850f2) )
	ROM_LOAD( "mbo-cg2353.u79",  0x10000, 0x8000, CRC(119f59cd) SHA1(52283feb21b880960efef06c780d4e22b31ea18c) )
	ROM_LOAD( "mxo-cg2353.u80",  0x18000, 0x8000, CRC(3bb871c1) SHA1(0b9439fd6565c742c1c7dda23a80bdd1d91d7293) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2307.u43", 0x0000, 0x0200, CRC(58d81338) SHA1(f0044ebbd0128d6fb74d850528ef02730c180f00) )
ROM_END

ROM_START( pex2421p ) /* Superboard : Deuces Wild Bonus Poker - French (X002421P+XP000064) */
/*
   Same payouts as X002029P English Deuces Wild Bonus Poker:

                                   w/D 6-K 3-5         w/A w/oD
PayTable   3K  STR  FL  FH  4K  SF  RF  5K  5K  5A  4D  4D  RF  (Bonus)
-----------------------------------------------------------------------
 PI556A     1   1    2   3   4  10  25  20  40  80 200 400 250    800
  % Range: 91.3-93.3%  Optimum: 95.3%  Hit Frequency: 45.0%
     Programs Available: X002421P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000064.u67",   0x00000, 0x10000, CRC(bb958158) SHA1(5d171ba71f70c668c70e4afd59ef7a0283798bbd) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002421p.u66",   0x00000, 0x10000, CRC(ee6a2bb8) SHA1(7916a8cbe08cd66e5d3b4b1c5b4aaff108e79f59) ) /* Deuces Wild Bonus Poker - French */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2353.u77",  0x00000, 0x8000, CRC(7ed7f7cd) SHA1(406b124b3db5335acf8f8987afbfa10d90e04351) )
	ROM_LOAD( "mgo-cg2353.u78",  0x08000, 0x8000, CRC(aab4e5fb) SHA1(7d6e048dc1a9d01900ba71fc23c884637f5850f2) )
	ROM_LOAD( "mbo-cg2353.u79",  0x10000, 0x8000, CRC(119f59cd) SHA1(52283feb21b880960efef06c780d4e22b31ea18c) )
	ROM_LOAD( "mxo-cg2353.u80",  0x18000, 0x8000, CRC(3bb871c1) SHA1(0b9439fd6565c742c1c7dda23a80bdd1d91d7293) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2307.u43", 0x0000, 0x0200, CRC(58d81338) SHA1(f0044ebbd0128d6fb74d850528ef02730c180f00) )
ROM_END

ROM_START( pex2440p ) /* Superboard : Deuces Wild Poker (X002440P+XP000053) */
/*
                                        w/D     w/oD
PayTable   3K   STR  FL  FH  4K  SF  5K  RF  4D  RF  (Bonus)
------------------------------------------------------------
 P129A      1    2    3   4   4  10  16  25 200 250    800
  % Range: 95.7-97.7%  Optimum: 99.7%  Hit Frequency: 44.3%
     Programs Available: PP0469, X002440P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000053.u67",   0x00000, 0x10000, CRC(f4f1f986) SHA1(84cfc2c4a10ed24d3a971fe75041a4108ec1d7f2) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002440p.u66",   0x00000, 0x10000, CRC(2ecb28cc) SHA1(a7b902bdfbf8f5ceedc778b8408c39ee279a1a1d) ) /* Deuces Wild Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2461p ) /* Superboard : Joker Poker (X002461P+XP000055) */
/*
                                       w/J     w/oJ
PayTable   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
-----------------------------------------------------------
  NCJ       1   2   4    5   8  16 100 100 400 100    800
  % Range: 93.2-95.2%  Optimum: 97.2%  Hit Frequency: 30.1%
     Programs Available: X002461P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000055.u67",   0x00000, 0x10000, CRC(339821e0) SHA1(127d4eff01136feaf1e3242d57433349afb7b6ca) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002461p.u66",   0x00000, 0x10000, CRC(9eb7b3ac) SHA1(162353e0914bf86d36b653719fc71b56c265cca0) ) /* Joker Poker */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2185.u77",  0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) ) /*  07/10/95   @ IGT  L95-1506  */
	ROM_LOAD( "mgo-cg2185.u78",  0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",  0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",  0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2474p ) /* Superboard : Double Double Bonus Plus (X002474P+XP000038) */
/*
                                                                        2-4
                                      JJJ55  66633          222AA       4K    4A
                                      QQQ55  88844 5-K  2-4   or       with   with
PayTable   Js+  2PR  3K  STR  FL  FH  KKK55  TTT55  4K  4K  44422  4A  A,2-4  2-4  SF  RF  (Bonus)
--------------------------------------------------------------------------------------------------
  ????      1    1    3   4    5   7    25     25   50  80    85  160   160   400  50 250    800
  % Range: 94.1-96.1%  Optimum: 98.1%  Hit Frequency: 44.8%
     Programs Available: X002474P

NOTE: This DATA rom is compatible with the Multi-Poker XMP00030 + CG2451 set below
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) ) /* Errors with INCOMPATIBLE EPROM error, no dumped program works with this DATA set */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002474p.u66",   0x00000, 0x10000, CRC(74cc1423) SHA1(0522cee3a7e123ce51739c69f38915ca92bd03e5) ) /* Double Double Bonus Plus */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2275.u77",  0x00000, 0x8000, CRC(15d5d6b8) SHA1(61b6821d4cc059732bc3831bf19bf01aa3910b31) )
	ROM_LOAD( "mgo-cg2275.u78",  0x08000, 0x8000, CRC(bcb49579) SHA1(d5d9f523304582fa6f0a0c69aade77629bdec006) )
	ROM_LOAD( "mbo-cg2275.u79",  0x10000, 0x8000, CRC(9f893787) SHA1(0b79d5cbac920394d5f5c04d0d9d3727e0060366) )
	ROM_LOAD( "mxo-cg2275.u80",  0x18000, 0x8000, CRC(6187c68b) SHA1(7777b141fd1379d37d93a228b2e2159476c2b89e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx1321.u43", 0x0000, 0x0200, CRC(4b57569f) SHA1(fa29c0f627e7ce79951ec6dadec114864144f37d) )
ROM_END

ROM_START( pex2478p ) /* Superboard : Joker Poker - French (X002478P+XP000154) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 PI104A     1    1   2   3    5   7  15  50 100 300 400    800
  % Range: 92.0-94.0%  Optimum: 96.0%  Hit Frequency: 44.5%
     Programs Available: X002317P, X002478P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000154.u67",   0x00000, 0x10000, CRC(f5f9ba4d) SHA1(d59f477c0a22065a62ffbe44d802b19078fefbb8) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002478p.u66",   0x00000, 0x10000, CRC(c667f425) SHA1(a47432af0915ac5369c0c2470bb8086f7f021058) ) /* Joker Poker - French */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2452.u77",  0x00000, 0x8000, CRC(188cdf9e) SHA1(b575ee8c140589ed7d3c5c6cd21c2ea4806136c5) )
	ROM_LOAD( "mgo-cg2452.u78",  0x08000, 0x8000, CRC(eaae3a1c) SHA1(b46822c59f2176306fc7864f9c560e86d4237747) )
	ROM_LOAD( "mbo-cg2452.u79",  0x10000, 0x8000, CRC(38c94e65) SHA1(2bba913ed305062c232e58349c2ffff8b2ded563) )
	ROM_LOAD( "mxo-cg2452.u80",  0x18000, 0x8000, CRC(22080393) SHA1(885eecbd4a8255f8ffa01d3ad0f80ad6631c7c9a) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2307.u43", 0x0000, 0x0200, CRC(58d81338) SHA1(f0044ebbd0128d6fb74d850528ef02730c180f00) )
ROM_END

ROM_START( pex2479p ) /* Superboard : Joker Poker - French (X002479P+XP000154) */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 PI105A     1    1   2   3    4   5  20  40 100 200 400    800
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 44.5%
     Programs Available: X002318P, X002479P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000154.u67",   0x00000, 0x10000, CRC(f5f9ba4d) SHA1(d59f477c0a22065a62ffbe44d802b19078fefbb8) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002479p.u66",   0x00000, 0x10000, CRC(e95b3550) SHA1(8bd702fb81cef0b9782a9e6b404917fc302ae1ef) ) /* Joker Poker - French */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2452.u77",  0x00000, 0x8000, CRC(188cdf9e) SHA1(b575ee8c140589ed7d3c5c6cd21c2ea4806136c5) )
	ROM_LOAD( "mgo-cg2452.u78",  0x08000, 0x8000, CRC(eaae3a1c) SHA1(b46822c59f2176306fc7864f9c560e86d4237747) )
	ROM_LOAD( "mbo-cg2452.u79",  0x10000, 0x8000, CRC(38c94e65) SHA1(2bba913ed305062c232e58349c2ffff8b2ded563) )
	ROM_LOAD( "mxo-cg2452.u80",  0x18000, 0x8000, CRC(22080393) SHA1(885eecbd4a8255f8ffa01d3ad0f80ad6631c7c9a) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2307.u43", 0x0000, 0x0200, CRC(58d81338) SHA1(f0044ebbd0128d6fb74d850528ef02730c180f00) )
ROM_END

ROM_START( pex2480p ) /* Superboard : Joker Poker (Aces or Better) - French (X002480P+XP000154) */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
 PI106B     1    1   2   3    5   6  20  50 100 200 500   1000
  % Range: 89.5-91.5%  Optimum: 93.5%  Hit Frequency: 39.2%
     Programs Available: X002320P, X002480P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000154.u67",   0x00000, 0x10000, CRC(f5f9ba4d) SHA1(d59f477c0a22065a62ffbe44d802b19078fefbb8) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002480p.u66",   0x00000, 0x10000, CRC(a1ec5a5f) SHA1(a272f9f3f11756a78247fc5aa58f09ea83604fc0) ) /* Joker Poker - French */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2452.u77",  0x00000, 0x8000, CRC(188cdf9e) SHA1(b575ee8c140589ed7d3c5c6cd21c2ea4806136c5) )
	ROM_LOAD( "mgo-cg2452.u78",  0x08000, 0x8000, CRC(eaae3a1c) SHA1(b46822c59f2176306fc7864f9c560e86d4237747) )
	ROM_LOAD( "mbo-cg2452.u79",  0x10000, 0x8000, CRC(38c94e65) SHA1(2bba913ed305062c232e58349c2ffff8b2ded563) )
	ROM_LOAD( "mxo-cg2452.u80",  0x18000, 0x8000, CRC(22080393) SHA1(885eecbd4a8255f8ffa01d3ad0f80ad6631c7c9a) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2307.u43", 0x0000, 0x0200, CRC(58d81338) SHA1(f0044ebbd0128d6fb74d850528ef02730c180f00) )
ROM_END

ROM_START( pex2485p ) /* Superboard : Standard Draw Poker - French (X002480P+XP000154) */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
 PI103B     1    2    3    4    5   7  22  50 300   1000
  % Range: 90.4-92.4%  Optimum: 94.4%  Hit Frequency: 45.5%
     Programs Available: X002485P
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xp000154.u67",   0x00000, 0x10000, CRC(f5f9ba4d) SHA1(d59f477c0a22065a62ffbe44d802b19078fefbb8) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002485p.u66",   0x00000, 0x10000, CRC(2ed40148) SHA1(f3c211955ef159da8ab14cfecbdfa2deaa3110ae) ) /* Standard Draw Poker - French */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2452.u77",  0x00000, 0x8000, CRC(188cdf9e) SHA1(b575ee8c140589ed7d3c5c6cd21c2ea4806136c5) )
	ROM_LOAD( "mgo-cg2452.u78",  0x08000, 0x8000, CRC(eaae3a1c) SHA1(b46822c59f2176306fc7864f9c560e86d4237747) )
	ROM_LOAD( "mbo-cg2452.u79",  0x10000, 0x8000, CRC(38c94e65) SHA1(2bba913ed305062c232e58349c2ffff8b2ded563) )
	ROM_LOAD( "mxo-cg2452.u80",  0x18000, 0x8000, CRC(22080393) SHA1(885eecbd4a8255f8ffa01d3ad0f80ad6631c7c9a) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2307.u43", 0x0000, 0x0200, CRC(58d81338) SHA1(f0044ebbd0128d6fb74d850528ef02730c180f00) )
ROM_END

ROM_START( pekoc766 ) /* Superboard : Standard Draw Poker (PP0766) English / Spanish - Key On Credit */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   8  25  50 300    400
  % Range: 92.6-94.6%  Optimum: 96.4%  Hit Frequency: 45.6%
     Programs Available: PP0766 A5W-A6F

Same as US paytable QJ
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0766_a5w-a6f.u67",   0x00000, 0x10000, CRC(e6bfa03b) SHA1(c4a281ab441747db4fefb09f0f07d3718855a9ca) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0766_data_a5w-a6f.u66",   0x00000, 0x10000, CRC(636ceb06) SHA1(ca0f7e67f6c86d6aed2bbed2a70372b5d5799bb8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc801 ) /* Superboard : 10's or Better (PP0801) English / Spanish - Key On Credit */
/*
PayTable  10s+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????     1     1    3    4    5   8  25  50 300   1000
  % Range: 85.2-87.2%  Optimum: 89.2%  Hit Frequency: 49.0%
     Programs Available: PP0801 A5W-A6F

Same as US paytable P8B
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0801_a5w-a6f.u67",   0x00000, 0x10000, CRC(d026b27a) SHA1(fb54699444b1e1950288881d4c7950980535c0f6) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0801_data_a5w-a6f.u66",   0x00000, 0x10000, CRC(636ceb06) SHA1(ca0f7e67f6c86d6aed2bbed2a70372b5d5799bb8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc802 ) /* Superboard : Standard Draw Poker (PP0802) English / Spanish - Key On Credit */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   6  25  50 250   1000
  % Range: 91.0-93.0%  Optimum: 95.0%  Hit Frequency: 45.5%
     Programs Available: PP0802 A5W-A6F

Same as US paytable GA
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0803_a5w-a6f.u67",   0x00000, 0x10000, CRC(93ea790c) SHA1(ec331565c058b173e343a0d3f6c28bab7f0b10d8) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0802_data_a5w-a6f.u66",   0x00000, 0x10000, CRC(636ceb06) SHA1(ca0f7e67f6c86d6aed2bbed2a70372b5d5799bb8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc803 ) /* Superboard : Joker Poker (PP0830) English / Spanish - Key On Credit */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 44.7%
     Programs Available: PP0803 A5W-A6F, PP0803 A50-A6N

Same as US paytable P17A
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0803_a5w-a6f.u67",   0x00000, 0x10000, CRC(26ec73b3) SHA1(0f592d21e83b73f37943b80ded6e83ee7b9c3edf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0803_data_a5w-a6f.u66",   0x00000, 0x10000, CRC(636ceb06) SHA1(ca0f7e67f6c86d6aed2bbed2a70372b5d5799bb8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc803a ) /* Superboard : Joker Poker (PP0803) English / Spanish - Key On Credit */
/*
                                            w/J     w/oJ
PayTable   Ks+  2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    4   5  20  40 100 200 500    800
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 44.7%
     Programs Available: PP0803 A5W-A6F, PP0803 A50-A6N

Same as US paytable P17A
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0803_a50-a6n.u67",   0x00000, 0x10000, CRC(40c18868) SHA1(d0e899fd09c1b49e2b93671770e4981c0a3a3501) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0803_data_a50-a6n.u66",   0x00000, 0x10000, CRC(eea95084) SHA1(fddf0d645437f606a31f72a56183d9a879b29418) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc804 ) /* Superboard : Bonus poker Deluxe - Key On Credit */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    1    3    4    5   7  80  50 250    800
  % Range: 92.3-94.3%  Optimum: 96.3%  Hit Frequency: 45.2%
     Programs Available: PP0804 A5W-A6F

Same as US paytable P253A
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0804_a5w-a6f.u67",   0x00000, 0x10000, CRC(86a1a37b) SHA1(37c29120870e7ac613e4c06999cc52febb3dd3b0) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0804_data_a5w-a6f.u66",   0x00000, 0x10000, CRC(636ceb06) SHA1(ca0f7e67f6c86d6aed2bbed2a70372b5d5799bb8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc806 ) /* Superboard : Standard Draw Poker - Key On Credit */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    2    3    4    5   6  25  50 250   1000
  % Range: 91.5-93.5%  Optimum: 95.5%  Hit Frequency: 45.3%
     Programs Available: PP0806 A5W-A6F

Same as US paytable GB
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0806_a5w-a6f.u67",   0x00000, 0x10000, CRC(299b2f73) SHA1(c0adc3a4b7f3c5a0e99d85be7f77a42fd6fb5160) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0806_data_a5w-a6f.u66",   0x00000, 0x10000, CRC(636ceb06) SHA1(ca0f7e67f6c86d6aed2bbed2a70372b5d5799bb8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc818 ) /* Superboard : Joker Poker (Aces or Better) - Key On Credit */
/*
                                            w/J     w/oJ
PayTable   As   2P  3K  STR  FL  FH  4K  SF  RF  5K  RF  (Bonus)
----------------------------------------------------------------
  ????      1    1   2   3    4   6  15  50  80 200 500    800
  % Range: 83.6-85.6%  Optimum: 87.6%  Hit Frequency: ??.?%
     Programs Available: PP0818 A5W-A6F
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0818_a5w-a6f.u67",   0x00000, 0x10000, CRC(38b1f3ca) SHA1(d869fbacdd918b146072ca820530cc041aa54568) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0818_data_a5w-a6f.u66",   0x00000, 0x10000, CRC(636ceb06) SHA1(ca0f7e67f6c86d6aed2bbed2a70372b5d5799bb8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc819 ) /* Superboard : Bonus poker Deluxe - Key On Credit */
/*
PayTable   Js+  2PR  3K   STR  FL  FH  4K  SF  RF  (Bonus)
----------------------------------------------------------
  ????      1    1    2    4    6   9  60  50 250    800
  % Range: 83.6-85.6%  Optimum: 87.6%  Hit Frequency: ??.?%
     Programs Available: PP0818 A5W-A6F
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0819_a5w-a6f.u67",   0x00000, 0x10000, CRC(f84a0415) SHA1(b501cf3a165b65f8ad2d908c6cb70ea86c0c41e7) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0819_data_a5w-a6f.u66",   0x00000, 0x10000, CRC(636ceb06) SHA1(ca0f7e67f6c86d6aed2bbed2a70372b5d5799bb8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pekoc825 ) /* Superboard : White Hot Aces - Key On Credit */
/*
                                      5-K 2-4
PayTable   Js+  2PR  3K  STR  FL  FH  4K  4K  4A  SF  RF  (Bonus)
-----------------------------------------------------------------
  ????      1    1    2   4    7  10  50 120 240  50 250    800
  % Range: 91.4-93.4%  Optimum: 95.4%  Hit Frequency: 44.7%
     Programs Available: PP0825 A59-A7C
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp0825_a59-a7c.u67",   0x00000, 0x10000, CRC(f1b7b2e0) SHA1(afa2236541230f546ae55093b4f0389691467c97) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "pp0825_data_a59-a7c.u66",   0x00000, 0x10000, CRC(f343c99b) SHA1(9cf14c6f281d77485ef7244bd5bd64042cf5a85c) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2245.u77",  0x00000, 0x8000, CRC(60461758) SHA1(856aa5a2ec2d3dece8a94cd6c58ff0e2941d61b3) )
	ROM_LOAD( "mgo-cg2245.u78",  0x08000, 0x8000, CRC(d4939806) SHA1(2852ec153da620868330d0d51b73c779ee6cfc49) )
	ROM_LOAD( "mbo-cg2245.u79",  0x10000, 0x8000, CRC(86b2977b) SHA1(a086c05afeb6b2658975f06c33aa768efef92688) )
	ROM_LOAD( "mxo-cg2245.u80",  0x18000, 0x8000, CRC(fd95acea) SHA1(be8feb17e22915951ff9b68150674e369ea95758) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx733.u43", 0x0000, 0x0200, CRC(867efa71) SHA1(f9e303dfaa43d5e44dbd1671b3269c1a658dea89) )
ROM_END

ROM_START( pex0838s ) /* Superboard : Five Times Pay Slots (X000835S+XS000002) - Payout 90.01% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xs000002.u67",   0x00000, 0x10000, CRC(f25725e8) SHA1(a7a0022162f6aa3303f072b6fab3713bdc6b57ad) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000838s.u66",   0x00000, 0x10000, CRC(913d17ac) SHA1(37162ac4384954165d9cfe04811ff5fa2cdde71e) ) /* Five Times Pay Slots - 2 Coins Max / 1 Pay Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2351.u77",  0x00000, 0x8000, CRC(d861f650) SHA1(7b483c5241e6704e8a2c70edb5a315ed6b1ae85d) )
	ROM_LOAD( "mgo-cg2351.u78",  0x08000, 0x8000, CRC(3a853984) SHA1(7af217f85f3168a6f2b50a11450d2fa3ff1e0386) )
	ROM_LOAD( "mbo-cg2351.u79",  0x10000, 0x8000, CRC(d1dc724e) SHA1(3fa2e9f363b2984cf17eb96b294343613e0e610a) )
	ROM_LOAD( "mxo-cg2351.u80",  0x18000, 0x8000, CRC(7888aab5) SHA1(237feae404314c7e394e403e4385bd01b6ac61d7) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2351.u43", 0x0000, 0x0200, CRC(34c59e88) SHA1(bf4d7a54c964b1b723ec65a4ede40ad900dd0f08) )
ROM_END

ROM_START( pex0841s ) /* Superboard : Five Times Pay Slots (X000841S+XS000002) - Payout 92.51% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xs000002.u67",   0x00000, 0x10000, CRC(f25725e8) SHA1(a7a0022162f6aa3303f072b6fab3713bdc6b57ad) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000841s.u66",   0x00000, 0x10000, CRC(430cc466) SHA1(3cd4a942274930db260567344008880027d5538c) ) /* Five Times Pay Slots - 3 Coins Max / 1 Pay Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2351.u77",  0x00000, 0x8000, CRC(d861f650) SHA1(7b483c5241e6704e8a2c70edb5a315ed6b1ae85d) )
	ROM_LOAD( "mgo-cg2351.u78",  0x08000, 0x8000, CRC(3a853984) SHA1(7af217f85f3168a6f2b50a11450d2fa3ff1e0386) )
	ROM_LOAD( "mbo-cg2351.u79",  0x10000, 0x8000, CRC(d1dc724e) SHA1(3fa2e9f363b2984cf17eb96b294343613e0e610a) )
	ROM_LOAD( "mxo-cg2351.u80",  0x18000, 0x8000, CRC(7888aab5) SHA1(237feae404314c7e394e403e4385bd01b6ac61d7) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2351.u43", 0x0000, 0x0200, CRC(34c59e88) SHA1(bf4d7a54c964b1b723ec65a4ede40ad900dd0f08) )
ROM_END

ROM_START( pex0998s ) /* Superboard : Triple Triple Diamond Slots (X000998S+XS000006) - Payout 92.47% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xs000006.u67",   0x00000, 0x10000, CRC(4b11ca18) SHA1(f64a1fbd089c01bc35a5484e60b8834a2db4a79f) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000998s.u66",   0x00000, 0x10000, CRC(e29d4346) SHA1(93901ff65c8973e34ac1f0dd68bb4c4973da5621) ) /* Triple Triple Diamonds Slots - 2 Coins Max / 1 Pay Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2361.u77",  0x00000, 0x8000, CRC(c0eba866) SHA1(8f217aeb3e8028a5633d95e5612f1b55e601650f) )
	ROM_LOAD( "mgo-cg2361.u78",  0x08000, 0x8000, CRC(345eaea2) SHA1(18ebb94a323e1cf671201db8b9f85d4f30d8b5ec) )
	ROM_LOAD( "mbo-cg2361.u79",  0x10000, 0x8000, CRC(fa130af6) SHA1(aca5e52e00bc75a4801fd3f6c564e62ed4251d8e) )
	ROM_LOAD( "mxo-cg2361.u80",  0x18000, 0x8000, CRC(7de1812c) SHA1(c7e23a10f20fc8b618df21dd33ac456e1d2cfe33) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2361.u43", 0x0000, 0x0200, CRC(93057296) SHA1(534bbf8ee80a22822d577f6685501f4c929987ef) )
ROM_END

ROM_START( pex1087s ) /* Superboard : Double Double Diamond Slots (X001087S+XS000006) - Payout 94.95% */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xs000006.u67",   0x00000, 0x10000, CRC(4b11ca18) SHA1(f64a1fbd089c01bc35a5484e60b8834a2db4a79f) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x001087s.u66",   0x00000, 0x10000, CRC(f811cff6) SHA1(3bdb77774387602ba4d699e009afa8591559c33e) ) /* Double Double Diamonds Slots - 3 Coins Max / 1 Pay Line */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2415.u77",  0x00000, 0x8000, CRC(f135a9c1) SHA1(366f93ce14da86c237da62f0b252bd26d662c8b1) )
	ROM_LOAD( "mgo-cg2415.u78",  0x08000, 0x8000, CRC(3b1f5f13) SHA1(a12b8268f51cce4f71b1e451274f7e5e97bc4f3d) )
	ROM_LOAD( "mbo-cg2415.u79",  0x10000, 0x8000, CRC(f14c3a06) SHA1(4132c15323cf2c2cf001c8cdcebdadb533b07312) )
	ROM_LOAD( "mxo-cg2415.u80",  0x18000, 0x8000, CRC(c427fff8) SHA1(5f41ff4d4598609a753c2e986f2a8cd63aa87d30) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2415.u43", 0x0000, 0x0200, NO_DUMP ) /* Should be CAPX2415(?) */
	ROM_LOAD( "capx2361.u43", 0x0000, 0x0200, CRC(93057296) SHA1(534bbf8ee80a22822d577f6685501f4c929987ef) ) /* Wrong!! Should be CAPX2415(?) */
ROM_END

ROM_START( pexm001p ) /* Superboard : Multi-Poker (XM00001P) - Bonus Poker, Bonus Poker Dlx, Deuces Wild Poker, Jacks or Better & Dbl Bonus Poker */
/*
Combined average payout percent: 99.64%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P77A      99.20%
Bonus Poker Deluxe   P200A     98.50%
Deuces Wild Poker    P34A     100.80%
Jacks or Better      CA        99.50%
Double Bonus Poker   P324A    100.20%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00003.u67",   0x00000, 0x10000, CRC(41e33b3e) SHA1(cd5debfb59c4f0cc5d700a1c592a0dc203abcb66) ) /* Linkable Progressive, No Double Up */
	/* Known to be found with XMP00003, XMP00006 or XMP00024 programs */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00001p.u66",   0x00000, 0x10000, CRC(b1569f05) SHA1(c94409ad74c4585288780cc2f96957592554a250) ) /*  07/18/95   @ IGT  L95-1613  */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2174.u77",  0x00000, 0x8000, CRC(bb666733) SHA1(dcaa1980b051a554cb0f443b1183a680edc9ad3f) ) /*  07/26/95   @ IGT  L95-1616  */
	ROM_LOAD( "mgo-cg2174.u78",  0x08000, 0x8000, CRC(cc46adb0) SHA1(6065aa5dcb9091ad80e499c7ee6dc629e79c865a) )
	ROM_LOAD( "mbo-cg2174.u79",  0x10000, 0x8000, CRC(7291a0c8) SHA1(1068f35e6ef5fd88c584922860231840a90fb623) )
	ROM_LOAD( "mxo-cg2174.u80",  0x18000, 0x8000, CRC(14f9480c) SHA1(59323f9fc5995277aea86d088893b6eb95b4e89b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm002p ) /* Superboard : Multi-Poker (XM00002P) - Bonus Poker, Bonus Poker Dlx, Deuces Wild Poker, Jacks or Better & Dbl Bonus Poker */
/*
Combined average payout percent: 98.14%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P90A      98.00%
Bonus Poker Deluxe   P251A     97.40%
Deuces Wild Poker    P62A      98.90%
Jacks or Better      BA        97.30%
Double Bonus Poker   P323A     99.10%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00006.u67",   0x00000, 0x10000, CRC(d61f1677) SHA1(2eca1315d6aa310a54de2dfa369e443a07495b76) ) /*  07/25/96   @ IGT L96-2041  - Linkable Progressive */
	/* Known to be found with XMP00003, XMP00006 or XMP00024 programs */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00002p.u66",   0x00000, 0x10000, CRC(96cf471c) SHA1(9597bf6a80c392ee22dc4606db610fdaf032377f) ) /*  07/18/95   @ IGT  L95-1614  */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2174.u77",  0x00000, 0x8000, CRC(bb666733) SHA1(dcaa1980b051a554cb0f443b1183a680edc9ad3f) ) /*  07/26/95   @ IGT  L95-1616  */
	ROM_LOAD( "mgo-cg2174.u78",  0x08000, 0x8000, CRC(cc46adb0) SHA1(6065aa5dcb9091ad80e499c7ee6dc629e79c865a) )
	ROM_LOAD( "mbo-cg2174.u79",  0x10000, 0x8000, CRC(7291a0c8) SHA1(1068f35e6ef5fd88c584922860231840a90fb623) )
	ROM_LOAD( "mxo-cg2174.u80",  0x18000, 0x8000, CRC(14f9480c) SHA1(59323f9fc5995277aea86d088893b6eb95b4e89b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm003p ) /* Superboard : Multi-Poker (XM00003P) - Bonus Poker, Bonus Poker Dlx, Deuces Wild Poker, Jacks or Better & Dbl Bonus Poker */
/*
Combined average payout percent: 96.78%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P101A     96.90%
Bonus Poker Deluxe   P253A     96.30%
Deuces Wild Poker    P47A      96.80%
Jacks or Better      P11A      96.10%
Double Bonus Poker   P325A     97.80%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00024.u67",   0x00000, 0x10000, CRC(f2df8870) SHA1(bc7fa1d79da07093cf3d3508e226a9c490990e04) ) /* Standalone Progressive */
	/* Known to be found with XMP00003, XMP00006 or XMP00024 programs */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00003p.u66",   0x00000, 0x10000, CRC(55b44732) SHA1(8e0bbad3aaa7deca85ae641c444be3a513bdce50) ) /*  07/18/95   @ IGT  L95-1615  */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2174.u77",  0x00000, 0x8000, CRC(bb666733) SHA1(dcaa1980b051a554cb0f443b1183a680edc9ad3f) ) /*  07/26/95   @ IGT  L95-1616  */
	ROM_LOAD( "mgo-cg2174.u78",  0x08000, 0x8000, CRC(cc46adb0) SHA1(6065aa5dcb9091ad80e499c7ee6dc629e79c865a) )
	ROM_LOAD( "mbo-cg2174.u79",  0x10000, 0x8000, CRC(7291a0c8) SHA1(1068f35e6ef5fd88c584922860231840a90fb623) )
	ROM_LOAD( "mxo-cg2174.u80",  0x18000, 0x8000, CRC(14f9480c) SHA1(59323f9fc5995277aea86d088893b6eb95b4e89b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm004p ) /* Superboard : Multi-Poker (XM00004P) - Dbl Dbl Bonus Poker, Nevada Bonus Poker, Joker Poker, Dbl Bonus Poker & Dbl Deuces Poker */
/*
Combined average payout percent: 99.22%

Game Type           PayTable   Payout
-------------------------------------
Double Double Bonus  P505A     97.80%
Nevada Bonus Poker   P503A     98.80%
Joker Poker          ZA       100.20%
Double Bonus Poker   P324A    100.20%
Double Deuce Poker   P236A     99.60%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00002.u67",   0x00000, 0x10000, CRC(d5624ac8) SHA1(6b778b0e7ddb81123c6038920b3447e05a0556b2) ) /*  09/07/95   @ IGT  L95-2183  - Linkable Progressive */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00004p.u66",   0x00000, 0x10000, CRC(bafd160f) SHA1(7454fbf992d4d0668ef375b76ce2cae3324a5f75) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2196.u77",  0x00000, 0x8000, CRC(f2f95ad9) SHA1(92c105147d4cdcebb4c784d771b9cebc982a742f) )
	ROM_LOAD( "mgo-cg2196.u78",  0x08000, 0x8000, CRC(95980a94) SHA1(40b84b2f3b77584739f2eb8df49b64533c60e1e7) )
	ROM_LOAD( "mbo-cg2196.u79",  0x10000, 0x8000, CRC(38151131) SHA1(7730a342bcfab2c2acd84f93ce280eb5dc9666f3) )
	ROM_LOAD( "mxo-cg2196.u80",  0x18000, 0x8000, CRC(60f748b8) SHA1(61af0bac1d6c23f8e1aa3f0094fd56185aa6ae86) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm005p ) /* Superboard : Multi-Poker (XM00005P) - Bonus Poker, Dbl Dbl Bonus Poker, Joker Poker, Dbl Joker Poker & Dbl Bonus Poker */
/*
Combined average payout percent: 97.06%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P101A     96.90%
Double Double Bonus  P506A     96.70%
Joker Poker          NA        97.20%
Double Joker Poker   ?????     98.10%
Double Bonus Poker   P434A     96.40%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00004.u67",   0x00000, 0x10000, CRC(83184999) SHA1(b8483917b338be4fd3641b3990eea37072d36885) ) /* Linkable Progressive */
	/* Also known to be found with XMP00024 program */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00005p.u66",   0x00000, 0x10000, CRC(c832eac7) SHA1(747d57de602b44ae1276fe1009db1b6de0d2c64c) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2240.u77",  0x00000, 0x8000, CRC(eedef2d4) SHA1(419a90e1f4a840625e6ac7afc2c24d13c908156d) )
	ROM_LOAD( "mgo-cg2240.u78",  0x08000, 0x8000, CRC(c596b058) SHA1(d53824f869bceeda482e434cba9a77ba8ce2015f) )
	ROM_LOAD( "mbo-cg2240.u79",  0x10000, 0x8000, CRC(ab1a58ee) SHA1(44963f27d5f5d8f9415d88c12b2d40f0ef55c559) )
	ROM_LOAD( "mxo-cg2240.u80",  0x18000, 0x8000, CRC(75488ff7) SHA1(a34ae53847b5643b8c4dc182dc59b1fccf22d557) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm006p ) /* Superboard : Multi-Poker (XM00006P) - Bonus Poker, Bonus Poker Dlx, Deuces Wild Poker, Jacks or Better & Dbl Bonus Poker */
/*
Combined average payout percent: 99.04%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P77A      99.20%
Bonus Poker Deluxe   P200A     98.50%
Deuces Wild Poker    P62A      98.90%
Jacks or Better      CA        99.50%
Double Bonus Poker   P323A     99.10%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00020.u67",   0x00000, 0x10000, CRC(0f6d0706) SHA1(fccf6c93daab0694d1e35e7cdd6bae303a3fddd9) )
	/* Also known to be found with XMP00003, XMP00006 or XMP00024 programs */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00006p.u66",   0x00000, 0x10000, CRC(b464ee79) SHA1(8768e52c66881c8f327055124ff31bcad79fd027) ) /*  03/08/96   @ IGT  NV  */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2174.u77",  0x00000, 0x8000, CRC(bb666733) SHA1(dcaa1980b051a554cb0f443b1183a680edc9ad3f) ) /*  07/26/95   @ IGT  L95-1616  */
	ROM_LOAD( "mgo-cg2174.u78",  0x08000, 0x8000, CRC(cc46adb0) SHA1(6065aa5dcb9091ad80e499c7ee6dc629e79c865a) )
	ROM_LOAD( "mbo-cg2174.u79",  0x10000, 0x8000, CRC(7291a0c8) SHA1(1068f35e6ef5fd88c584922860231840a90fb623) )
	ROM_LOAD( "mxo-cg2174.u80",  0x18000, 0x8000, CRC(14f9480c) SHA1(59323f9fc5995277aea86d088893b6eb95b4e89b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm006pa ) /* Superboard : Multi-Poker (XM00006P) - Bonus Poker, Bonus Poker Dlx, Deuces Wild Poker, Jacks or Better & Dbl Bonus Poker */
/*
Combined average payout percent: 99.04%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P77A      99.20%
Bonus Poker Deluxe   P200A     98.50%
Deuces Wild Poker    P62A      98.90%
Jacks or Better      CA        99.50%
Double Bonus Poker   P323A     99.10%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00002.u67",   0x00000, 0x10000, CRC(d5624ac8) SHA1(6b778b0e7ddb81123c6038920b3447e05a0556b2) ) /*  09/07/95   @ IGT  L95-2183  - Linkable Progressive */
	/* Also known to be found with XMP00003, XMP00006, XMP00020 or XMP00024 programs */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00006p.u66",   0x00000, 0x10000, CRC(b464ee79) SHA1(8768e52c66881c8f327055124ff31bcad79fd027) ) /*  03/08/96   @ IGT  NV  */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2294.u77",  0x00000, 0x8000, CRC(2f707abc) SHA1(3ed14b165cc1c6ad1a0c8ceddbe6d10666de7a1e) ) /* Custom The Orleans graphics */
	ROM_LOAD( "mgo-cg2294.u78",  0x08000, 0x8000, CRC(3dc48f1a) SHA1(d0c4861eba3f37064c6a1b62488764e18a762461) ) /* Compatible with XM00001P, XM00002P, XM00003P & XM00006P */
	ROM_LOAD( "mbo-cg2294.u79",  0x10000, 0x8000, CRC(ce8aebdb) SHA1(0c08561016aeadee95e843299cccff3114d839e2) )
	ROM_LOAD( "mxo-cg2294.u80",  0x18000, 0x8000, CRC(5fba4c90) SHA1(259359b11af9a554364ae90989a23fc2c848d16c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2294.u43", 0x0000, 0x0200, CRC(641b72c0) SHA1(22f940defceee579fce8547c3469765be730ec56) )
ROM_END

ROM_START( pexm007p ) /* Superboard : Multi-Poker (XM00007P) - Bonus Poker, Dbl Dbl Bonus Poker, Deuces Wild Poker, Dbl Bonus Poker & Jacks or Better */
/*
Combined average payout percent: 97.38%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P90A      98.00%
Double Double Bonus  P506A     96.70%
Deuces Wild Poker    P59A      97.10%
Jacks or Better      BA        97.30%
Double Bonus Poker   P325A     97.80%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00006.u67",   0x00000, 0x10000, CRC(d61f1677) SHA1(2eca1315d6aa310a54de2dfa369e443a07495b76) ) /*  07/25/96   @ IGT L96-2041  - Linkable Progressive */
	/* Also known to be found with XMP00002 program */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00007p.u66",   0x00000, 0x10000, CRC(85a76416) SHA1(1bc3b9c2f687e68a085bfc5cf86d99fbd18cb9c7) ) /*  03/09/96   @ IGT  L96-0737  */

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2233.u77",  0x00000, 0x8000, CRC(8758866a) SHA1(49146560a7e79593a2ac0378dc3b300b96ef1015) ) /*  03/07/96   @ IGT  L96-0686  */
	ROM_LOAD( "mgo-cg2233.u78",  0x08000, 0x8000, CRC(45ac6cfd) SHA1(25ff276320fe51c56aea0cff099be17e4ce8f404) )
	ROM_LOAD( "mbo-cg2233.u79",  0x10000, 0x8000, CRC(9e9d702f) SHA1(75bb9adb49095b7cb87d2615bcf725e4a4774e25) )
	ROM_LOAD( "mxo-cg2233.u80",  0x18000, 0x8000, CRC(2f05ebcb) SHA1(90d00ee4ce2dcbfbe33e221efe4db45a4e484baa) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm008p ) /* Superboard : Multi-Poker (XM00008P) - Bonus Poker, Bonus Poker Dlx, Deuces Wild Poker, Jacks or Better & Dbl Bonus Poker */
/*
Combined average payout percent: 99.30%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P77A      99.20%
Bonus Poker Deluxe   P200A     98.50%
Deuces Wild Poker    P34A     100.80%
Double Bonus Poker   P324A    100.20%
Double Double Bonus  P505A     97.80%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00006.u67",   0x00000, 0x10000, CRC(d61f1677) SHA1(2eca1315d6aa310a54de2dfa369e443a07495b76) ) /*  07/25/96   @ IGT L96-2041  - Linkable Progressive */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00008p.u66",   0x00000, 0x10000, CRC(37ff1a79) SHA1(5b15245e79d8f1b984d254f4307f1a2219ce3ed2) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2228.u77",  0x00000, 0x8000, CRC(b8abca87) SHA1(b008e0c0b272bc024fc1bfee1aeb80b8df589e4a) )
	ROM_LOAD( "mgo-cg2228.u78",  0x08000, 0x8000, CRC(ad9b93ae) SHA1(3ffba64d4763bc0e107f6279b5cce4c0e7e4fa7a) )
	ROM_LOAD( "mbo-cg2228.u79",  0x10000, 0x8000, CRC(4ad376ef) SHA1(116160d507f734ecdf5e80a3e746ab10fb9b4d78) )
	ROM_LOAD( "mxo-cg2228.u80",  0x18000, 0x8000, CRC(8965ff2f) SHA1(9d5ba61504f9a7e3225ecaf72e81d65a9fbf3667) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm009p ) /* Superboard : Multi-Poker (XM00009P) - Aces & Faces, Bonus Poker Dlx, Deuces Wild Poker, Jacks or Better & Dbl Aces & Faces Poker */
/*
Combined average payout percent: 98.47%

Game Type           PayTable   Payout
-------------------------------------
Aces & Faces         ?????     99.30%
Bonus Poker Deluxe   P200A     98.50%
Deuces Wild Poker    P62A      98.90%
Jacks or Better      BB        97.80%
Double Aces & Faces  ?????     99.20%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00002.u67",   0x00000, 0x10000, CRC(d5624ac8) SHA1(6b778b0e7ddb81123c6038920b3447e05a0556b2) ) /*  09/07/95   @ IGT  L95-2183  - Linkable Progressive */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00009p.u66",   0x00000, 0x10000, CRC(e133d0bb) SHA1(7ed4fa335e230c28e6fc66f0c990bc7ead2b279d) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2xxx.u77",  0x00000, 0x8000, NO_DUMP ) /* This set requires an unknown CG graphics set for the correct banners on the MENU page */
	ROM_LOAD( "mgo-cg2xxx.u78",  0x08000, 0x8000, NO_DUMP ) /* Most likely CG2227, CG2229 or CG2239 */
	ROM_LOAD( "mbo-cg2xxx.u79",  0x10000, 0x8000, NO_DUMP )
	ROM_LOAD( "mxo-cg2xxx.u80",  0x18000, 0x8000, NO_DUMP )
	ROM_LOAD( "mro-cg2174.u77",  0x00000, 0x8000, CRC(bb666733) SHA1(dcaa1980b051a554cb0f443b1183a680edc9ad3f) ) /*  07/26/95   @ IGT  L95-1616  */
	ROM_LOAD( "mgo-cg2174.u78",  0x08000, 0x8000, CRC(cc46adb0) SHA1(6065aa5dcb9091ad80e499c7ee6dc629e79c865a) ) /* Close but banners on MEMU WRONG!! */
	ROM_LOAD( "mbo-cg2174.u79",  0x10000, 0x8000, CRC(7291a0c8) SHA1(1068f35e6ef5fd88c584922860231840a90fb623) )
	ROM_LOAD( "mxo-cg2174.u80",  0x18000, 0x8000, CRC(14f9480c) SHA1(59323f9fc5995277aea86d088893b6eb95b4e89b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexm013p ) /* Superboard : Multi-Poker (XM00013P) - Bonus Poker, Dbl Dbl Bonus Poker, Deuces Wild Poker, Jacks or Better & Joker Poker  */
/*
Combined average payout percent: 95.80%

Game Type           PayTable   Payout
-------------------------------------
Bonus Poker          P596A     95.80%
Double Double Bonus  P507A     95.60%
Deuces Wild Poker    P57A      96.00%
Jacks or Better      P11A      96.10%
Joker Poker          P17A      95.50%
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00025.u67",   0x00000, 0x10000, CRC(5d39ff71) SHA1(0a5f67e61ae0e8a08cc551ab4271ffc97c343ae3) ) /* International multi currency version - Auto Hold always on */
	/* Also compatible with XMP00002, XMP00003, XMP00004, XMP00006 and XMP00024 programs */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00013p.u66",   0x00000, 0x10000, CRC(4fde73f9) SHA1(f8eb6fb0585e8df9a7eb2ddc65bb20b120753d7a) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2296.u77",  0x00000, 0x8000, CRC(d0d92665) SHA1(2c686ee28b69ff975951ccafd8e5030fde640773) )
	ROM_LOAD( "mgo-cg2296.u78",  0x08000, 0x8000, CRC(d05fd16e) SHA1(f66b5ba8b4cf4f97ed46ec44cef43fed29bdd492) )
	ROM_LOAD( "mbo-cg2296.u79",  0x10000, 0x8000, CRC(6db6a435) SHA1(7ea0d6df1f7e0c4fe389437bf04d1f5a798c68ef) )
	ROM_LOAD( "mxo-cg2296.u80",  0x18000, 0x8000, CRC(4faeb79e) SHA1(f69277b729ba88860efc6b9a3d4956f245cc2943) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2174.u43", 0x0000, 0x0200, CRC(50bdad55) SHA1(958d463c7effb3457c1f9c44c9b7822339c04e8b) )
ROM_END

ROM_START( pexmp013 ) /* Superboard : 5-in-1 Wingboard (XMP00013) Program in Spanish, Requires Spanish Wingboard CG rom set */
/*

Known Wingboard compatible program roms:
   XMP00013 - Spanish
   XMP00014 (not dumped)
   XMP00017
   XMP00022 - Spanish (not dumped)
   XMP00026 - Spanish
   XMP00030

The CG2346 set seems to support all games supported in CG2298 as well as graphics support for the following XnnnnnnP Data game types:
  Triple Double Bonus
  Black Jack Bonus (comes up as Back Jack Poker)

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00013.u67",   0x00000, 0x10000, CRC(76ca7c2b) SHA1(cdcbfc648d007362bb50541e6415354c07815d66) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "x000188p.u66",   0x00000, 0x10000, CRC(3eb7580e) SHA1(86f2280542fb8a55767efd391d0fb04a12ed9408) ) /* Standard Draw Poker */

	ROM_REGION( 0x10000, "user3", 0 )
	ROM_LOAD( "x000516p.u66",   0x00000, 0x10000, CRC(37f84ce7) SHA1(2e5157d02febec0ff31eb5a23254f7c49a486cf5) ) /* Double Bonus Poker */

	ROM_REGION( 0x10000, "user4", 0 )
	ROM_LOAD( "x002275p.u66",   0x00000, 0x10000, CRC(5ba4f5ab) SHA1(def069025ec4aa340646dfd7cfacc8ce836a210c) ) /* Black Jack Bonus Poker */

	ROM_REGION( 0x10000, "user5", 0 )
	ROM_LOAD( "x002310p.u66",   0x00000, 0x10000, CRC(c006c3f1) SHA1(45c87a2f882147d1d132237cfa12ae47b202264f) ) /* Triple Double Bonus Poker */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2346.u77",  0x00000, 0x10000, CRC(3d721779) SHA1(01ac540eddeab5ecdba9b543c69fe7f4b53151a4) ) /* Game titles in English */
	ROM_LOAD( "mgo-cg2346.u78",  0x10000, 0x10000, CRC(a4a4856b) SHA1(db0e7528a63c80fab02b463dfb366d32061a93bb) ) /* Poker hands in Spanish */
	ROM_LOAD( "mbo-cg2346.u79",  0x20000, 0x10000, CRC(15253b57) SHA1(503b5cb514d9552ed7cf09f236aec63c81cfd828) )
	ROM_LOAD( "mxo-cg2346.u80",  0x30000, 0x10000, CRC(68ffb37e) SHA1(b6de07452e52a8c6f8657fbefef081aa9d86dbf0) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2346.u43", 0x0000, 0x0200, CRC(8df8ad29) SHA1(2d6a598fdc4290abe83a3d95c0ec8da6eb0f0e84) )
ROM_END

ROM_START( pexmp017 ) /* Superboard : 5-in-1 Wingboard (XMP00017) */
/*

Known Wingboard compatible program roms:
   XMP00013 - Spanish
   XMP00014 (not dumped)
   XMP00017
   XMP00022 - Spanish (not dumped)
   XMP00026 - Spanish
   XMP00030

Wingboard programs are not compatible with:
 Lucky Deal Poker, Shockwave Poker, Ace$ Bonus Poker, Dealt Deuces Bonus, Barbaric Deuces, Pay the Ace (No Face)
 and many other "specialty" poker games.

The CG2298 graphics can support the following XnnnnnnP Data game types:

  Bonus Poker, Bonus Poker Deluxe, Double Bonus Poker, Double Double Bonus Poker, Triple Bonus Poker
  Deuces Wild Poker, Deluxe Deuces Wild, Loose Deuces, Deuces Bonus, Double Deuces, Royal Deuces Poker
  Joker Poker, Double Joker Poker, Deuces Joker Wild Poker, Sevens or Better, Tens or Better, Jacks or Better
  Nevada Draw Poker, Nevada Bonus Poker, White Hot Aces, Double Double Aces & Faces, Odds & Ends Poker
  Two Pair, Crazy Eights and Full House Bonus

  Super Aces shows as just Bonus Poker
  Triple Bonus Poker Plus shows as just Triple Bonus

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00017.u67",   0x00000, 0x10000, CRC(129e6eaa) SHA1(1dd2b83a672a618f338b553a6cbd598b6d4ce672) ) /*  09/17/97   @ IGT  L97-2154  */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000055p.u66",   0x00000, 0x10000, CRC(e06819df) SHA1(36590c4588b8036908e63714fbb3e77d23e60eae) ) /* Deuces Wild Poker */

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "x000188p.u66",   0x00000, 0x10000, CRC(3eb7580e) SHA1(86f2280542fb8a55767efd391d0fb04a12ed9408) ) /* Standard Draw Poker */

	ROM_REGION( 0x10000, "user3", 0 )
	ROM_LOAD( "x000581p.u66",   0x00000, 0x10000, CRC(a4cfecc3) SHA1(b2c805781ba43bda9e208d8c16578dc96b6f58f7) ) /* Four of a Kind Bonus Poker */

	ROM_REGION( 0x10000, "user4", 0 )
	ROM_LOAD( "x000727p.u66",   0x00000, 0x10000, CRC(4828474c) SHA1(9836b76113a71802df30ca15f7c9a5790e6f1c5b) ) /* Double Bonus Poker */

	ROM_REGION( 0x10000, "user5", 0 )
	ROM_LOAD( "x002036p.u66",   0x00000, 0x10000, CRC(69207baf) SHA1(fe038b969106ae5cdc8dde1c06497be9c7b5b8bf) ) /* White Hot Aces */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2298.u77",  0x00000, 0x10000, CRC(8c35dc7f) SHA1(90e9566e816287e6248d7cab318dee3ad6fac871) )
	ROM_LOAD( "mgo-cg2298.u78",  0x10000, 0x10000, CRC(3663174a) SHA1(c203a4a59f6bc1625d47f35426ffc5b4d279251a) )
	ROM_LOAD( "mbo-cg2298.u79",  0x20000, 0x10000, CRC(9088cdbe) SHA1(dc62951c584463a1e795a774f5752f890d8e3f65) )
	ROM_LOAD( "mxo-cg2298.u80",  0x30000, 0x10000, CRC(8d3aafc8) SHA1(931bc82398b94c63ed9f6f1bd95723aa801894cc) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2298.u43", 0x0000, 0x0200, CRC(77856036) SHA1(820487c8494965408402ddee6a54511906218e66) )
ROM_END

ROM_START( pexmp017a ) /* Superboard : 5-in-1 Wingboard (XMP00017) */
/*

The CG2352 set supersedes CG2298. It's currently not known what has changed between the two sets.

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00017.u67",   0x00000, 0x10000, CRC(129e6eaa) SHA1(1dd2b83a672a618f338b553a6cbd598b6d4ce672) ) /*  09/17/97   @ IGT  L97-2154  */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000430p.u66",   0x00000, 0x10000, CRC(905571e3) SHA1(fd506516fed22842df8e9dbb3683dcb4c459719b) ) /* Dueces Joker Wild Poker */

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "x000451p.u66",   0x00000, 0x10000, CRC(4f11e26c) SHA1(6cea3cbef530ef4ece2a4351cbd9ead5b66bb359) ) /* Bonus Poker Deluxe */

	ROM_REGION( 0x10000, "user3", 0 )
	ROM_LOAD( "x000508p.u66",   0x00000, 0x10000, CRC(5efde4b4) SHA1(ead7448464aecc03748f04e4d6e9f346d262cd96) ) /* Loose Deuce Deuces Wild Poker */

	ROM_REGION( 0x10000, "user4", 0 )
	ROM_LOAD( "x000458p.u66",   0x00000, 0x10000, CRC(dcd20558) SHA1(22c99a265431b0ef8199d3cb69fbbc4aff822dc0) ) /* Joker Poker */

	ROM_REGION( 0x10000, "user5", 0 )
	ROM_LOAD( "x002045p.u66",   0x00000, 0x10000, CRC(75fe81db) SHA1(980bcc06b54a1ef78e3beac1db83b73e17a04818) ) /* Triple Bonus Poker */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2352.u77",  0x00000, 0x10000, CRC(ef87fccb) SHA1(8ac04626f1bcab68e930e954d92594e981fd22d6) )
	ROM_LOAD( "mgo-cg2352.u78",  0x10000, 0x10000, CRC(fb3144a2) SHA1(a4dc3d7175915ceb99e0faeb8928148b2b3996ea) )
	ROM_LOAD( "mbo-cg2352.u79",  0x20000, 0x10000, CRC(28f422d6) SHA1(db1fa033e109749acfe7bacb85fe717858a25904) )
	ROM_LOAD( "mxo-cg2352.u80",  0x30000, 0x10000, CRC(b13b46c3) SHA1(7400037bbf56f67bf2d58b35589d62d94dea4b9f) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2352.u43", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "capx2298.u43", 0x0000, 0x0200, CRC(77856036) SHA1(820487c8494965408402ddee6a54511906218e66) ) /* Wrong!! Should be CAPX2352 - However colors should be correct */
ROM_END

ROM_START( pexmp017b ) /* Superboard : 5-in-1 Wingboard (XMP00017) */
/*

The CG2426 set supersedes both CG2298 & CG2352 and adds graphics support for the following XnnnnnnP Data game types:
  Black Jack Bonus
  Super Double Bonus
  Triple Double Bonus

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00017.u67",   0x00000, 0x10000, CRC(129e6eaa) SHA1(1dd2b83a672a618f338b553a6cbd598b6d4ce672) ) /*  09/17/97   @ IGT  L97-2154  */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002272p.u66",   0x00000, 0x10000, CRC(ee4f27b9) SHA1(1ee105430358ea27badd943bb6b18663e4029388) ) /* Black Jack Bonus Poker */

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "x002029p.u66",   0x00000, 0x10000, CRC(e2f6fb89) SHA1(4b60b580b00b4268d1cb9065ffe0d21f8fa6a931) ) /* Deuces Wild Bonus Poker */

	ROM_REGION( 0x10000, "user3", 0 )
	ROM_LOAD( "x002040p.u66",   0x00000, 0x10000, CRC(38acb477) SHA1(894f5861ac84323e50e8972602251f2873988e6c) ) /* Nevada Bonus Poker */

	ROM_REGION( 0x10000, "user4", 0 )
	ROM_LOAD( "x002018p.u66",   0x00000, 0x10000, CRC(a7b79cfa) SHA1(89216fafffc64fda22a016a906483b76174c3f02) ) /* Full House Bonus Poker */

	ROM_REGION( 0x10000, "user5", 0 )
	ROM_LOAD( "x002307p.u66",   0x00000, 0x10000, CRC(c6d5db70) SHA1(017e1e382fb789e4cd8b410362ad5e82b61f61db) ) /* Triple Double Bonus Poker */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2426.u77",  0x00000, 0x10000, CRC(e7622901) SHA1(f653aaf02de840aef56d3efd7680572356e94da7) ) /*  05/29/98   @ IGT  L98-1765  */
	ROM_LOAD( "mgo-cg2426.u78",  0x10000, 0x10000, CRC(5c8388a0) SHA1(c883bf7969850d07f37fa0fd58f82cda4cf15654) )
	ROM_LOAD( "mbo-cg2426.u79",  0x20000, 0x10000, CRC(dc6e39aa) SHA1(7a7188757f5be25521a023d1315cfd7c395b6c25) )
	ROM_LOAD( "mxo-cg2426.u80",  0x30000, 0x10000, CRC(a32f42a2) SHA1(87ddc4dda7c198ed62a2a065507efe4d3a016236) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2298.u43", 0x0000, 0x0200, CRC(77856036) SHA1(820487c8494965408402ddee6a54511906218e66) )
ROM_END

ROM_START( pexmp026 ) /* Superboard : 5-in-1 Wingboard (XMP00026) Program in Spanish, Requires Spanish Wingboard CG rom set */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00026.u67",   0x00000, 0x10000, CRC(0b82387f) SHA1(8348c586cf692c5cbecfe7b52a4271e5aec55027) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000242p.u66",   0x00000, 0x10000, CRC(e0292d63) SHA1(8d8ec5dc1abaf8e8a8a7451d3a814023d8195fb5) ) /* Deuces Wild Poker */

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "x000150p.u66",   0x00000, 0x10000, CRC(d10759fa) SHA1(eae633d03ac9db86520a70825ac0a59ee9ebc819) ) /* Standard Draw Poker */

	ROM_REGION( 0x10000, "user3", 0 )
	ROM_LOAD( "x002044p.u66",   0x00000, 0x10000, CRC(158af97f) SHA1(452247d981f1202da8c44a31f0d3343184d3db41) ) /* Triple Bonus Poker */

	ROM_REGION( 0x10000, "user4", 0 )
	ROM_LOAD( "x002038p.u66",   0x00000, 0x10000, CRC(58d01ba5) SHA1(6d4cde9c9e55967db2b661c7123cce9958a00639) ) /* Nevada Bonus Poker */

	ROM_REGION( 0x10000, "user5", 0 )
	ROM_LOAD( "x002306p.u66",   0x00000, 0x10000, CRC(ef36ea67) SHA1(8914ad20526fd63e14d9fa1901e9c779a11eb29d) ) /* Triple Double Bonus Poker */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2346.u77",  0x00000, 0x10000, CRC(3d721779) SHA1(01ac540eddeab5ecdba9b543c69fe7f4b53151a4) ) /* Game titles in English */
	ROM_LOAD( "mgo-cg2346.u78",  0x10000, 0x10000, CRC(a4a4856b) SHA1(db0e7528a63c80fab02b463dfb366d32061a93bb) ) /* Poker hands in Spanish */
	ROM_LOAD( "mbo-cg2346.u79",  0x20000, 0x10000, CRC(15253b57) SHA1(503b5cb514d9552ed7cf09f236aec63c81cfd828) )
	ROM_LOAD( "mxo-cg2346.u80",  0x30000, 0x10000, CRC(68ffb37e) SHA1(b6de07452e52a8c6f8657fbefef081aa9d86dbf0) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2346.u43", 0x0000, 0x0200, CRC(8df8ad29) SHA1(2d6a598fdc4290abe83a3d95c0ec8da6eb0f0e84) )
ROM_END

ROM_START( pexmp030 ) /* Superboard : 5-in-1 Wingboard (XMP00030) */
/*

The CG2451 set supersedes CG2298, CG2352 & CG2426 and adds graphics support for the following XnnnnnnP Data game types:
  Double Double Bonus Plus

*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00030.u67",   0x00000, 0x10000, CRC(da3fcb6f) SHA1(114e581e5ebb5c40c3f3da2784122d3281f269ee) ) /*  11/12/00   @ IGT  L01-0197  */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002037p.u66",   0x00000, 0x10000, CRC(12aea90e) SHA1(26ff0e7b81271252573739f26db9d20f35af274b) ) /* Nevada Bonus Poker */

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "x002111p.u66",   0x00000, 0x10000, CRC(f19dd63e) SHA1(0fe16cd0c75a9759e34bf95ce428e5b2da236215) ) /* 4 of a Kind Bonus Poker with Sequential Royal Flush */

	ROM_REGION( 0x10000, "user3", 0 )
	ROM_LOAD( "x000455p.u66",   0x00000, 0x10000, CRC(4992c51f) SHA1(8c70c59bdb16feba438230b30765076cebd44b53) ) /* Joker Poker */

	ROM_REGION( 0x10000, "user4", 0 )
	ROM_LOAD( "x000006p.u66",   0x00000, 0x10000, CRC(0ee609a1) SHA1(57043ac2c6ff4377479dd7b66d7e379053f3f602) ) /* Standard Draw Poker */

	ROM_REGION( 0x10000, "user5", 0 )
	ROM_LOAD( "x002474p.u66",   0x00000, 0x10000, CRC(74cc1423) SHA1(0522cee3a7e123ce51739c69f38915ca92bd03e5) ) /* Double Double Bonus Plus */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2451.u77",  0x00000, 0x10000, CRC(ffc7dd2e) SHA1(277e0620cbe4358137179b3c98170e295470c651) )
	ROM_LOAD( "mgo-cg2451.u78",  0x10000, 0x10000, CRC(b1a746b5) SHA1(64bb1a4e57d693010004a585503e46892afecb8f) )
	ROM_LOAD( "mbo-cg2451.u79",  0x20000, 0x10000, CRC(ecfd6737) SHA1(4b563dbf76e81b49d2736c6eaeab243173e8e51b) )
	ROM_LOAD( "mxo-cg2451.u80",  0x30000, 0x10000, CRC(ac1395c9) SHA1(ce1fb78b0440c80da0d6b99b6f6369861da212df) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2298.u43", 0x0000, 0x0200, CRC(77856036) SHA1(820487c8494965408402ddee6a54511906218e66) )
ROM_END

ROM_START( pexmp030a ) /* Superboard : 5-in-1 Wingboard (XMP00030) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xmp00030.u67",   0x00000, 0x10000, CRC(da3fcb6f) SHA1(114e581e5ebb5c40c3f3da2784122d3281f269ee) ) /*  11/12/00   @ IGT  L01-0197  */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002066p.u66",   0x00000, 0x10000, CRC(01236011) SHA1(3edfee014705b3540386c5e42026ab93628b2597) ) /* Double Double Bonus Poker */

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "x000158p.u66",   0x00000, 0x10000, CRC(51a8a294) SHA1(f76992729ceaca18af82ab2fb3403dc5a48b7e8a) ) /* 4 of a Kind Bonus Poker */

	ROM_REGION( 0x10000, "user3", 0 )
	ROM_LOAD( "x000536p.u66",   0x00000, 0x10000, CRC(0b18dc1b) SHA1(07350fe258441f8565bfd875342823149b7757f1) ) /* Joker Poker */

	ROM_REGION( 0x10000, "user4", 0 )
	ROM_LOAD( "x002377p.u66",   0x00000, 0x10000, CRC(541320d2) SHA1(670b17432e994fe1937091e5e96e1d58b9afbf29) ) /* Super Double Bonus Poker */

	ROM_REGION( 0x10000, "user5", 0 )
	ROM_LOAD( "x002440p.u66",   0x00000, 0x10000, CRC(2ecb28cc) SHA1(a7b902bdfbf8f5ceedc778b8408c39ee279a1a1d) ) /* Deuces Wild Poker */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "mro-cg2426.u77",  0x00000, 0x10000, CRC(e7622901) SHA1(f653aaf02de840aef56d3efd7680572356e94da7) ) /*  05/29/98   @ IGT  L98-1765  */
	ROM_LOAD( "mgo-cg2426.u78",  0x10000, 0x10000, CRC(5c8388a0) SHA1(c883bf7969850d07f37fa0fd58f82cda4cf15654) )
	ROM_LOAD( "mbo-cg2426.u79",  0x20000, 0x10000, CRC(dc6e39aa) SHA1(7a7188757f5be25521a023d1315cfd7c395b6c25) )
	ROM_LOAD( "mxo-cg2426.u80",  0x30000, 0x10000, CRC(a32f42a2) SHA1(87ddc4dda7c198ed62a2a065507efe4d3a016236) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "capx2298.u43", 0x0000, 0x0200, CRC(77856036) SHA1(820487c8494965408402ddee6a54511906218e66) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT         INIT      ROT    COMPANY                                  FULLNAME                                                  FLAGS   LAYOUT */

/* Set chips */
GAMEL(1987, peset001, 0,         peplus,  peplus_schip, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (SET001) Set Chip",                      0,   layout_pe_schip )
GAMEL(1987, peset004, 0,         peplus,  peplus_schip, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (SET004) Set Chip",                      0,   layout_pe_schip )
GAMEL(1987, peset038, 0,         peplus,  peplus_schip, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (SET038) Set Chip",                      0,   layout_pe_schip )
GAMEL(1987, peivc006, 0,         peplus,  peplus_schip, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IVC006) Clear EEPROM Chip",             0,   layout_pe_schip )

/* Normal (non-plus) board : Poker */
GAMEL(1987, pepk1024,  0,        peplus, nonplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge (PK1024) Aces and Faces Bonus Poker",         0, layout_pe_poker )

/* Normal board : Poker */
GAMEL(1987, pepp0002,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0002) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0002a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0002) Standard Draw Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0008,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0008) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0009,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0009) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0010,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0010) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0014,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0014) Standard Draw Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0014a, pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0014) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0021,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0021) Standard Draw Poker",           MACHINE_NOT_WORKING, layout_pe_poker) /* Progressive with link ONLY */
GAMEL(1987, pepp0023,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0023) 10's or Better",                0, layout_pe_poker )
GAMEL(1987, pepp0038,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0038) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0040,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0040) Standard Draw Poker (set 1)",   0, layout_pe_poker )
GAMEL(1987, pepp0040a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0040) Standard Draw Poker (set 2)",   0, layout_pe_poker )
GAMEL(1987, pepp0040b, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0040) Standard Draw Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0041,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0041) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0042,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0042) 10's or Better (set 1)",        0, layout_pe_poker )
GAMEL(1987, pepp0042a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0042) 10's or Better (set 2)",        0, layout_pe_poker )
GAMEL(1987, pepp0043,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0043) 10's or Better",                0, layout_pe_poker )
GAMEL(1987, pepp0043a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0043) 10's or Better (International, set 1)",0, layout_pe_poker )
GAMEL(1987, pepp0043b, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0043) 10's or Better (International, set 2)",0, layout_pe_poker )
GAMEL(1987, pepp0045,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0045) 10's or Better",                0, layout_pe_poker )
GAMEL(1987, pepp0045a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0045) 10's or Better (Gambler Downtown Reno)", 0, layout_pe_poker )
GAMEL(1987, pepp0045b, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0045) 10's or Better (Par-A-Dice Riverboat Casino)", MACHINE_WRONG_COLORS, layout_pe_poker ) /* CAP1150 not dumped */
GAMEL(1987, pepp0045c, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0045) 10's or Better (Annie Oakley's Central City)", MACHINE_WRONG_COLORS, layout_pe_poker ) /* CAP1072 not dumped */
GAMEL(1987, pepp0045d, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0045) 10's or Better (Las Vegas Rio)", 0, layout_pe_poker )
GAMEL(1987, pepp0046,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0046) 10's or Better (set 1)",        0, layout_pe_poker )
GAMEL(1987, pepp0046a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0046) 10's or Better (International)",0, layout_pe_poker )
GAMEL(1987, pepp0046b, pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0046) 10's or Better (set 2)",        0, layout_pe_poker )
GAMEL(1987, pepp0048,  pepp0053, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0048) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0051,  pepp0053, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0051) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0053,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0053) Joker Poker (Aces or Better)",  0, layout_pe_poker )
GAMEL(1987, pepp0055,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0055) Deuces Wild Poker (set 1)",     0, layout_pe_poker )
GAMEL(1987, pepp0055a, pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0055) Deuces Wild Poker (set 2)",     0, layout_pe_poker )
GAMEL(1987, pepp0055b, pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0055) Deuces Wild Poker (set 2, Skyline Casino)", 0, layout_pe_poker )
GAMEL(1987, pepp0055c, pepp0055, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0055) Deuces Wild Poker (set 3)",     0, layout_pe_poker )
GAMEL(1987, pepp0057,  pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0057) Deuces Wild Poker (set 1)",     0, layout_pe_poker )
GAMEL(1987, pepp0057a, pepp0055, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0057) Deuces Wild Poker (set 2)",     0, layout_pe_poker )
GAMEL(1987, pepp0059,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0059) Two Pair or Better (set 1)",    0, layout_pe_poker )
GAMEL(1987, pepp0059a, pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0059) Two Pair or Better (set 2)",    0, layout_pe_poker )
GAMEL(1987, pepp0060,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0060) Standard Draw Poker (set 1)",   0, layout_pe_poker )
GAMEL(1987, pepp0060a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0060) Standard Draw Poker (set 2)",   0, layout_pe_poker )
GAMEL(1987, pepp0060b, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0060) Standard Draw Poker (Cruise)",  0, layout_pe_poker )
GAMEL(1987, pepp0063,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0063) 10's or Better",                0, layout_pe_poker )
GAMEL(1987, pepp0064,  pepp0053, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0064) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0065,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0065) Joker Poker (Aces or Better)",  0, layout_pe_poker )
GAMEL(1987, pepp0083,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0083) 10's or Better",                0, layout_pe_poker )
GAMEL(1987, pepp0085,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0085) Joker Poker (Two Pair or Better)", 0, layout_pe_poker )
GAMEL(1987, pepp0089,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0089) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0103,  pepp0055, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0103) Deuces Wild Poker",             0, layout_pe_poker )
GAMEL(1987, pepp0116,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0116) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0116a, pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0116) Standard Draw Poker (Mirage)",  0, layout_pe_poker )
GAMEL(1987, pepp0118,  pepp0002, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0118) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0120,  0,        peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0120) Wild Sevens Poker",             0, layout_pe_poker )
GAMEL(1987, pepp0125,  pepp0055, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0125) Deuces Wild Poker",             0, layout_pe_poker )
GAMEL(1987, pepp0126,  pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0126) Deuces Wild Poker",             0, layout_pe_poker )
GAMEL(1987, pepp0127,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0127) Deuces Joker Wild Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0127a, pepp0127, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0127) Deuces Joker Wild Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0130,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0130) Aces and Faces",                0, layout_pe_poker )
GAMEL(1987, pepp0132,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0132) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0150,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0150) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0158,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0158) 4 of a Kind Bonus Poker (set 1)", 0, layout_pe_poker )
GAMEL(1987, pepp0158a, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0158) 4 of a Kind Bonus Poker (set 2, Skyline Casino)", 0, layout_pe_poker )
GAMEL(1987, pepp0158b, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0158) 4 of a Kind Bonus Poker (set 3)", 0, layout_pe_poker )
GAMEL(1987, pepp0158c, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0158) 4 of a Kind Bonus Poker (set 4)", 0, layout_pe_poker )
GAMEL(1987, pepp0158d, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0158) 4 of a Kind Bonus Poker (set 5)", 0, layout_pe_poker )
GAMEL(1987, pepp0159,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0159) Standard Draw Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0171,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0171) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0171a, pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0171) Joker Poker (International)",   0, layout_pe_poker )
GAMEL(1987, pepp0178,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0178) 4 of a Kind Bonus Poker (Operator selectable special 4 of a Kind)", 0, layout_pe_poker )
GAMEL(1987, pepp0181,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0181) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0188,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0188) Standard Draw Poker (set 1)",   0, layout_pe_poker )
GAMEL(1987, pepp0188a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0188) Standard Draw Poker (set 2)",   0, layout_pe_poker )
GAMEL(1987, pepp0190,  pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0190) Deuces Wild Poker (set 1)",     0, layout_pe_poker )
GAMEL(1987, pepp0190a, pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0190) Deuces Wild Poker (set 2)",     0, layout_pe_poker )
GAMEL(1987, pepp0190b, pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0190) Deuces Wild Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0197,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0197) Standard Draw Poker (set 1)",   0, layout_pe_poker )
GAMEL(1987, pepp0197a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0197) Standard Draw Poker (set 2)",   0, layout_pe_poker )
GAMEL(1987, pepp0197b, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0197) Standard Draw Poker (set 3)",   0, layout_pe_poker )
GAMEL(1987, pepp0197c, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0197) Standard Draw Poker (set 4)",   0, layout_pe_poker )
GAMEL(1987, pepp0203,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0203) 4 of a Kind Bonus Poker (set 1)", 0, layout_pe_poker )
GAMEL(1987, pepp0203a, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0203) 4 of a Kind Bonus Poker (set 2)", 0, layout_pe_poker )
GAMEL(1987, pepp0203b, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0203) 4 of a Kind Bonus Poker (set 3)", 0, layout_pe_poker )
GAMEL(1987, pepp0203c, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0203) 4 of a Kind Bonus Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0203d, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0203) 4 of a Kind Bonus Poker (set 4)", 0, layout_pe_poker )
GAMEL(1987, pepp0219,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0219) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0221,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0221) Standard Draw Poker (set 1)",   0, layout_pe_poker )
GAMEL(1987, pepp0221a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0221) Standard Draw Poker (set 2)",   0, layout_pe_poker )
GAMEL(1987, pepp0223,  pepp0127, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0223) Deuces Joker Wild Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0224,  pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0224) Deuces Wild Poker (set 1)",     0, layout_pe_poker )
GAMEL(1987, pepp0224a, pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0224) Deuces Wild Poker (set 2)",     0, layout_pe_poker )
GAMEL(1987, pepp0230,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0230) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0242,  pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0242) Deuces Wild Poker (International English/Spanish)", 0, layout_pe_poker )
GAMEL(1987, pepp0249,  pepp0055, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0249) Deuces Wild Poker",             0, layout_pe_poker )
GAMEL(1987, pepp0250,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0250) Double Down Stud Poker (set 1)", 0, layout_pe_poker )
GAMEL(1987, pepp0250a, pepp0250, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0250) Double Down Stud Poker (set 2)", 0, layout_pe_poker )
GAMEL(1987, pepp0265,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0265) 4 of a Kind Bonus Poker (set 1)", 0, layout_pe_poker )
GAMEL(1987, pepp0265a, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0265) 4 of a Kind Bonus Poker (set 2)", 0, layout_pe_poker )
GAMEL(1987, pepp0265b, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0265) 4 of a Kind Bonus Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0274,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0274) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0288,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0288) Standard Draw Poker (Spanish)", 0, layout_pe_poker )
GAMEL(1987, pepp0290,  pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0290) Deuces Wild Poker",             0, layout_pe_poker )
GAMEL(1987, pepp0291,  pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0291) Deuces Wild Poker (set 1)",     0, layout_pe_poker )
GAMEL(1987, pepp0291a, pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0291) Deuces Wild Poker (set 2)",     0, layout_pe_poker )
GAMEL(1987, pepp0401,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0401) 4 of a Kind Bonus Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0409,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0409) 4 of a Kind Bonus Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0410,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0410) 4 of a Kind Bonus Poker (set 1)", 0, layout_pe_poker )
GAMEL(1987, pepp0410a, pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0410) 4 of a Kind Bonus Poker (set 2)", 0, layout_pe_poker )
GAMEL(1987, pepp0417,  pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0417) Deuces Wild Poker (set 1)",     0, layout_pe_poker )
GAMEL(1987, pepp0417a, pepp0055, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0417) Deuces Wild Poker (set 2)",     0, layout_pe_poker )
GAMEL(1987, pepp0419,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0419) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0420,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0420) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0423,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0423) Standard Draw Poker (set 1)",   0, layout_pe_poker )
GAMEL(1987, pepp0423a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0423) Standard Draw Poker (set 2)",   0, layout_pe_poker )
GAMEL(1987, pepp0423b, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0423) Standard Draw Poker (set 3)",   0, layout_pe_poker )
GAMEL(1987, pepp0426,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0426) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0428,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0428) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0429,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0429) Joker Poker (Aces or Better, set 1)",  0, layout_pe_poker )
GAMEL(1987, pepp0429a, pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0429) Joker Poker (Aces or Better, set 2)",  0, layout_pe_poker )
GAMEL(1987, pepp0430,  pepp0127, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0430) Deuces Joker Wild Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0431,  pepp0127, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0431) Deuces Joker Wild Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0434,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0434) Bonus Poker Deluxe",            0, layout_pe_poker )
GAMEL(1987, pepp0447,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0447) Standard Draw Poker (set 1)",   0, layout_pe_poker )
GAMEL(1987, pepp0447a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0447) Standard Draw Poker (set 2)",   0, layout_pe_poker )
GAMEL(1987, pepp0449,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0449) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0449a, pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0449) Standard Draw Poker (International English/Spanish)", 0, layout_pe_poker )
GAMEL(1987, pepp0452,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0452) Double Deuces Wild Poker",      0, layout_pe_poker )
GAMEL(1987, pepp0454,  pepp0434, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0454) Bonus Poker Deluxe",            0, layout_pe_poker )
GAMEL(1987, pepp0455,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0455) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0467,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0467) 4 of a Kind Bonus Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0458,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0458) Joker Poker (Aces or Better)",  0, layout_pe_poker )
GAMEL(1987, pepp0459,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0459) Joker Poker",                   0, layout_pe_poker )
GAMEL(1985, pepp0488,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0488) Standard Draw Poker (Arizona Charlie's)", 0, layout_pe_poker )
GAMEL(1987, pepp0508,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0508) Loose Deuce Deuces Wild! Poker",0, layout_pe_poker )
GAMEL(1987, pepp0509,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0509) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0510,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0510) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0514,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0514) Double Bonus Poker (set 1)",    0, layout_pe_poker )
GAMEL(1987, pepp0514a, pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0514) Double Bonus Poker (set 2)",    0, layout_pe_poker )
GAMEL(1987, pepp0514b, pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0514) Double Bonus Poker (set 3)",    0, layout_pe_poker )
GAMEL(1987, pepp0515,  pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0515) Double Bonus Poker (set 1)",    0, layout_pe_poker )
GAMEL(1987, pepp0515a, pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0515) Double Bonus Poker (set 2)",    0, layout_pe_poker )
GAMEL(1987, pepp0515b, pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0515) Double Bonus Poker (set 3)",    0, layout_pe_poker )
GAMEL(1987, pepp0515c, pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0515) Double Bonus Poker (set 4)",    0, layout_pe_poker )
GAMEL(1987, pepp0516,  pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0516) Double Bonus Poker (set 1)",    0, layout_pe_poker )
GAMEL(1987, pepp0516a, pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0516) Double Bonus Poker (set 2)",    0, layout_pe_poker )
GAMEL(1987, pepp0516b, pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0516) Double Bonus Poker (International)", 0, layout_pe_poker )
GAMEL(1987, pepp0531,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0531) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0536,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0536) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0538,  pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0538) Double Bonus Poker",            0, layout_pe_poker )
GAMEL(1987, pepp0540,  pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0540) Double Bonus Poker",            0, layout_pe_poker )
GAMEL(1987, pepp0542,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0542) One Eyed Jacks Wild Poker (CG2243)", 0, layout_pe_poker )
GAMEL(1987, pepp0542a, pepp0542, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0542) One Eyed Jacks Wild Poker (CG2020)", 0, layout_pe_poker )
GAMEL(1987, pepp0550,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0550) Joker Poker (Two Pair or Better)", 0, layout_pe_poker )
GAMEL(1987, pepp0555,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0555) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0568,  pepp0053, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0568) Joker Poker",                   0, layout_pe_poker )
GAMEL(1987, pepp0585,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0585) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0713,  pepp0434, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0713) Bonus Poker Deluxe",            0, layout_pe_poker )
GAMEL(1987, pepp0725,  pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0725) Double Bonus Poker (set 1)",    0, layout_pe_poker )
GAMEL(1987, pepp0725a, pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0725) Double Bonus Poker (set 2)",    0, layout_pe_poker )
GAMEL(1987, pepp0726,  pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0726) Double Bonus Poker",            0, layout_pe_poker )
GAMEL(1987, pepp0728,  pepp0514, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0728) Double Bonus Poker",            0, layout_pe_poker )
GAMEL(1987, pepp0750,  pepp0002, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0750) Standard Draw Poker",           0, layout_pe_poker )
GAMEL(1987, pepp0757,  pepp0250, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0757) Double Down Stud Joker Poker",  0, layout_pe_poker )
GAMEL(1987, pepp0760,  pepp0250, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0760) Double Down Stud Poker",        0, layout_pe_poker )
GAMEL(1987, pepp0763,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0763) 4 of a Kind Bonus Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0764,  pepp0158, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0764) 4 of a Kind Bonus Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0775,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0775) Royal Deuces Poker??",          MACHINE_IMPERFECT_GRAPHICS, layout_pe_poker ) /* Wrong CG graphics & CAP */
GAMEL(1987, pepp0812,  pepp0127, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0812) Deuces Joker Wild Poker",       0, layout_pe_poker )
GAMEL(1987, pepp0816,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0816) Treasure Chest Poker",          MACHINE_IMPERFECT_GRAPHICS, layout_pe_poker ) /* Wrong CG graphics & CAP - Missing "Bonus" at MAX Bet for 4 of a Kind & Treasure Chest graphics */

/* Normal board : International Poker */
GAMEL(1987, peip0028,  0,        peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0028) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0029,  peip0028, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0029) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0031,  0,        peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0031) Standard Draw Poker - French",  0, layout_pe_poker )
GAMEL(1987, peip0041,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0041) Double Deuces Wild Poker - French", 0, layout_pe_poker )
GAMEL(1987, peip0051,  peip0028, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0051) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0058,  peip0031, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0058) Standard Draw Poker - French",  0, layout_pe_poker )
GAMEL(1987, peip0062,  peip0028, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0062) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0074,  peip0028, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0074) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0079,  peip0031, peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0079) Standard Draw Poker - French",  0, layout_pe_poker )
GAMEL(1987, peip0101,  peip0028, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0101) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0103,  peip0028, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0103) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0105,  peip0028, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0105) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0108,  peip0031, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0108) Standard Draw Poker - French",  0, layout_pe_poker )
GAMEL(1987, peip0111,  peip0028, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0111) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0112,  peip0031, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0112) Standard Draw Poker - French",  0, layout_pe_poker )
GAMEL(1987, peip0114,  peip0031, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0114) Standard Draw Poker - French",  0, layout_pe_poker )
GAMEL(1987, peip0115,  peip0028, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0115) Joker Poker - French",          0, layout_pe_poker )
GAMEL(1987, peip0116,  peip0031, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0116) Standard Draw Poker - French",  0, layout_pe_poker )
GAMEL(1987, peip0118,  peip0031, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0118) Standard Draw Poker - French",  0, layout_pe_poker )
GAMEL(1987, peip0120,  peip0031, peplus,  peplus_poker, peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (IP0120) Standard Draw Poker - French",  0, layout_pe_poker )

/* Normal board : Multi-Game - Player's Choice - Some sets require a printer (not yet supported) */
GAMEL(1994, pemg0183,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Montana Choice (MG0183) Multi-Game",                        MACHINE_NOT_WORKING, layout_pe_poker) /* Needs printer support */
GAMEL(1994, pemg0252,  0,        peplus,  peplus_poker, peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Choice (MG0252) Multi-Game",                       MACHINE_NOT_WORKING, layout_pe_poker) /* Needs printer support */

/* Normal board : Blackjack */
GAMEL(1994, pebe0014,  0,        peplus, peplus_bjack,  peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (BE0014) Blackjack",                     0, layout_pe_bjack )
GAMEL(1994, pebe0014a, pebe0014, peplus, peplus_bjack,  peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (BE0014) Blackjack (International English/Spanish)", MACHINE_IMPERFECT_GRAPHICS, layout_pe_bjack ) /* Needs CG1339 graphics roms */

/* Normal board : Keno */
GAMEL(1994, peke0004,  0,        peplus, peplus_keno,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (KE0004) Keno",                          MACHINE_NOT_WORKING, layout_pe_keno )
GAMEL(1994, peke0017,  peke0004, peplus, peplus_keno,   peplus_state, nonplus,  ROT0,  "IGT - International Game Technology", "Player's Edge Plus (KE0017) Keno",                          MACHINE_NOT_WORKING, layout_pe_keno )
GAMEL(1994, peke1006,  0,        peplus, peplus_keno,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (KE1006) Keno",                          0, layout_pe_keno )
GAMEL(1994, peke1012,  peke1006, peplus, peplus_keno,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (KE1012) Keno (set 1)",                  0, layout_pe_keno )
GAMEL(1994, peke1012a, peke1006, peplus, peplus_keno,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (KE1012) Keno (set 2)",                  0, layout_pe_keno )
GAMEL(1994, peke1013,  peke1006, peplus, peplus_keno,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (KE1013) Keno (set 1)",                  0, layout_pe_keno )
GAMEL(1994, peke1013a, peke1006, peplus, peplus_keno,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (KE1013) Keno (set 2)",                  0, layout_pe_keno )

/* Normal board : Slots machine */
GAMEL(1996, peps0014, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0014) Super Joker Slots",             0, layout_pe_slots )
GAMEL(1996, peps0021, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0021) Red White & Blue Slots",        0, layout_pe_slots )
GAMEL(1996, peps0022, peps0021, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0022) Red White & Blue Slots",        0, layout_pe_slots )
GAMEL(1996, peps0042, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0042) Double Diamond Slots",          0, layout_pe_slots )
GAMEL(1996, peps0043, peps0042, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0043) Double Diamond Slots",          0, layout_pe_slots )
GAMEL(1996, peps0045, peps0021, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0045) Red White & Blue Slots",        0, layout_pe_slots )
GAMEL(1996, peps0047, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0047) Wild Cherry Slots",             MACHINE_NOT_WORKING, layout_pe_slots ) /* Needs MxO-CG1004.Uxx graphics roms redumped */
GAMEL(1996, peps0090, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0090) Gold, Silver & Bronze Slots",   0, layout_pe_slots )
GAMEL(1996, peps0092, peps0047, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0092) Wild Cherry Slots",             MACHINE_NOT_WORKING, layout_pe_slots ) /* Needs MxO-CG1004.Uxx graphics roms redumped */
GAMEL(1996, peps0206, peps0021, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0206) Red White & Blue Slots",        0, layout_pe_slots )
GAMEL(1996, peps0207, peps0021, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0207) Red White & Blue Slots",        0, layout_pe_slots )
GAMEL(1996, peps0296, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0296) Haywire Slots",                 0, layout_pe_slots )
GAMEL(1996, peps0298, peps0042, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0298) Double Diamond Slots",          0, layout_pe_slots )
GAMEL(1996, peps0308, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0308) Double Jackpot Slots",          0, layout_pe_slots )
GAMEL(1996, peps0364, peps0021, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0364) Red White & Blue Slots",        0, layout_pe_slots )
GAMEL(1996, peps0366, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0366) Double Diamond Deluxe Slots",   0, layout_pe_slots )
GAMEL(1996, peps0372, peps0366, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0372) Double Diamond Deluxe Slots",   0, layout_pe_slots )
GAMEL(1996, peps0373, peps0366, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0373) Double Diamond Deluxe Slots",   0, layout_pe_slots )
GAMEL(1996, peps0426, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0426) Sizzling Sevens Slots",         0, layout_pe_slots )
GAMEL(1996, peps0581, peps0021, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0581) Red White & Blue Slots",        0, layout_pe_slots )
GAMEL(1996, peps0615, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0615) Chaos Slots",                   MACHINE_WRONG_COLORS, layout_pe_slots ) /* CAP2246 not dumped */
GAMEL(1996, peps0631, peps0021, peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0631) Red White & Blue Slots",        0, layout_pe_slots )
GAMEL(1996, peps0716, 0,        peplus, peplus_slots,   peplus_state, peplus,   ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PS0716) River Gambler Slots",           0, layout_pe_slots )

/* Superboard : Poker */
GAMEL(1995, pex0002p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000002P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0002pa, pex0002p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000002P+XP000109) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0006p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000006P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0040p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000040P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0042p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000042P+XP000038) 10's or Better",      0, layout_pe_poker )
GAMEL(1995, pex0045p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000045P+XP000038) 10's or Better",      0, layout_pe_poker )
GAMEL(1995, pex0046p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000046P+XP000038) 10's or Better",      0, layout_pe_poker )
GAMEL(1995, pex0053p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000053P+XP000038) Joker Poker (Aces or Better)", 0, layout_pe_poker )
GAMEL(1995, pex0054p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000054P+XP000038) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000019) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pa, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000022) Deuces Wild Poker (The Orleans)", 0, layout_pe_poker )
GAMEL(1995, pex0055pb, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000023) Deuces Wild Poker (The Fun Ships)", MACHINE_WRONG_COLORS, layout_pe_poker ) /* CAPX2399 not dumped */
GAMEL(1995, pex0055pc, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000028) Deuces Wild Poker (Horseshoe)", 0, layout_pe_poker )
GAMEL(1995, pex0055pd, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000035) Deuces Wild Poker (The Wild Wild West Casino)", MACHINE_WRONG_COLORS, layout_pe_poker ) /* CAPX2389 not dumped */
GAMEL(1995, pex0055pe, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000038) Deuces Wild Poker (Sunset Station Hotel-Casino)", 0, layout_pe_poker )
GAMEL(1995, pex0055pf, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000040) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pg, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000053) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055ph, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000055) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pi, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000063) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pj, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000075) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pk, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000079) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pl, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000094) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pm, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000095) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pn, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000098) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055po, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000102) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pp, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000104) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pq, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000112) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0055pr, pex0055p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000055P+XP000126) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0057p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000057P+XP000038) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0060p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000060P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0124p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000124P+XP000053) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0150p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000150P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0158p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000158P+XP000038) 4 of a Kind Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex0171p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000171P+XP000038) Joker Poker",         0, layout_pe_poker )
GAMEL(1995, pex0188p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000188P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0190p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000190P+XP000053) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0197p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000197P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0203p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000203P+XP000038) 4 of a Kind Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex0224p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000224P+XP000053) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0225p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000225P+XP000079) Dueces Joker Wild Poker", 0,layout_pe_poker )
GAMEL(1995, pex0242p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000242P+XP000053) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0265p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000265P+XP000038) 4 of a Kind Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex0291p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000291P+XP000053) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0417p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000417P+XP000053) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex0426p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000426P+XP000038) Joker Poker",         0, layout_pe_poker )
GAMEL(1995, pex0430p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000430P+XP000079) Dueces Joker Wild Poker", 0,layout_pe_poker )
GAMEL(1995, pex0434p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000434P+XP000038) Bonus Poker Deluxe",  0, layout_pe_poker )
GAMEL(1995, pex0447p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000447P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0449p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000449P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0451p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000451P+XP000038) Bonus Poker Deluxe",  0, layout_pe_poker )
GAMEL(1995, pex0452p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000452P+XP000038) Double Deuces Wild Poker", 0, layout_pe_poker )
GAMEL(1995, pex0454p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000454P+XP000038) Bonus Poker Deluxe",  0, layout_pe_poker )
GAMEL(1995, pex0455p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000455P+XP000038) Joker Poker",         0, layout_pe_poker )
GAMEL(1995, pex0458p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000458P+XP000038) Joker Poker (Aces or Better)", 0, layout_pe_poker )
GAMEL(1995, pex0459p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000459P+XP000038) Joker Poker",         0, layout_pe_poker )
GAMEL(1995, pex0459pa, pex0459p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000459P+XP000155) Joker Poker",         0, layout_pe_poker )
GAMEL(1995, pex0489p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000489P+XP000038) Double Down Stud Deuces Wild Poker", MACHINE_NOT_WORKING, layout_pe_poker ) /* Needs unknown PE+ GAME POKER program to run */
GAMEL(1995, pex0508p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000508P+XP000038) Loose Deuce Deuces Wild! Poker", 0, layout_pe_poker )
GAMEL(1995, pex0514p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000514P+XP000038) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex0515p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000515P+XP000038) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex0516p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000516P+XP000038) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex0536p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000536P+XP000038) Joker Poker",         0, layout_pe_poker )
GAMEL(1995, pex0537p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000537P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0550p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000550P+XP000055) Joker Poker (Two Pair or Better)", 0, layout_pe_poker )
GAMEL(1995, pex0557p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000557P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0568p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000568P+XP000038) Joker Poker",         0, layout_pe_poker )
GAMEL(1995, pex0578p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000578P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex0581p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000581P+XP000038) 4 of a Kind Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex0588p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000588P+XP000038) Joker Poker",         0, layout_pe_poker )
GAMEL(1995, pex0725p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000725P+XP000038) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex0726p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000726P+XP000038) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex0727p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000727P+XP000038) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex0763p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000763P+XP000038) 4 of a Kind Bonus Poker", 0,layout_pe_poker )
GAMEL(1995, pex0764p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000764P+XP000038) 4 of a Kind Bonus Poker", 0,layout_pe_poker )
GAMEL(1995, pex2010p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002010P+XP000038) Nevada Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2016p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002016P+XP000038) Full House Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2017p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002017P+XP000038) Full House Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2018p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002018P+XP000038) Full House Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2021p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002021P+XP000112) Lucky Deal Poker",    0, layout_pe_poker )
GAMEL(1995, pex2024p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002024P+XP000038) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2025p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002025P+XP000019) Deuces Wild Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2026p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002026P+XP000019) Deuces Wild Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2027p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002027P+XP000019) Deuces Wild Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2028p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002028P+XP000019) Deuces Wild Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2029p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002029P+XP000019) Deuces Wild Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2031p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002031P+XP000112) Lucky Deal Poker",    0, layout_pe_poker )
GAMEL(1995, pex2035p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002035P+XP000112) White Hot Aces Poker", 0, layout_pe_poker )
GAMEL(1995, pex2036p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002036P+XP000112) White Hot Aces Poker", 0, layout_pe_poker )
GAMEL(1995, pex2037p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002037P+XP000038) Nevada Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2038p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002038P+XP000038) Nevada Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2039p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002039P+XP000038) Nevada Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2040p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002040P+XP000038) Nevada Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2042p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002042P+XP000038) Triple Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2043p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002043P+XP000038) Triple Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2044p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002044P+XP000038) Triple Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2045p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002045P+XP000038) Triple Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2066p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002066P+XP000038) Double Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2067p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002067P+XP000038) Double Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2068p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002068P+XP000038) Double Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2069p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002069P+XP000038) Double Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2070p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002070P+XP000038) Double Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2111p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002111P+XP000038) 4 of a Kind Bonus Poker (with Seq Royal Flush)", 0,layout_pe_poker )
GAMEL(1995, pex2121p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002121P+XP000038) Standard Draw Poker", 0,layout_pe_poker )
GAMEL(1995, pex2121pa, pex2121p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002121P+XP000037) Standard Draw Poker", 0,layout_pe_poker )
GAMEL(1995, pex2150p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002150P+XP000038) 4 of a Kind Bonus Poker", 0,layout_pe_poker )
GAMEL(1995, pex2172p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002172P+XP000112) Ace$ Bonus Poker",    0, layout_pe_poker )
GAMEL(1995, pex2172pa, pex2172p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002172P+XP000112) Ace$ Bonus Poker (The Fun Ships)", MACHINE_WRONG_COLORS, layout_pe_poker ) /* CAPX2399 not dumped */
GAMEL(1995, pex2173p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002173P+XP000038) Ace$ Bonus Poker",    0, layout_pe_poker )
GAMEL(1995, pex2179p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002179P+XP000119) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2180p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002180P+XP000038) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2241p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002241P+XP000079) 4 of a Kind Bonus Poker", 0,layout_pe_poker )
GAMEL(1995, pex2244p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002244P+XP000079) Double Bonus Poker",  0, layout_pe_poker )
GAMEL(1995, pex2245p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002245P+XP000055) Standard Draw Poker", 0,layout_pe_poker )
GAMEL(1995, pex2245pa, pex2245p,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002245P+XP000079) Standard Draw Poker", 0,layout_pe_poker )
GAMEL(1995, pex2247p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002247P+XP000038) Standard Draw Poker", 0, layout_pe_poker )
GAMEL(1995, pex2250p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002250P+XP000050) Shockwave Poker",     0, layout_pe_poker )
GAMEL(1995, pex2251p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002251P+XP000050) Shockwave Poker",     0, layout_pe_poker )
GAMEL(1995, pex2272p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002272P+XP000055) Black Jack Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2275p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002275P+XP000055) Black Jack Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2276p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002276P+XP000055) Black Jack Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2283p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002283P+XP000057) Barbaric Decues Wild Poker", 0, layout_pe_poker ) /* Undumped color CAPX2325 but should have correct colors anyways */
GAMEL(1995, pex2284p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002284P+XP000057) Barbaric Decues Wild Poker", 0, layout_pe_poker ) /* Undumped color CAPX2325 but should have correct colors anyways */
GAMEL(1995, pex2287p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002287P+XP000057) Pay the Aces NO Faces Bonus Poker", 0, layout_pe_poker ) /* Undumped color CAPX2325 but should have correct colors anyways */
GAMEL(1995, pex2297p,  0,         peplus,  peplus_poker, peplus_state, pepluss64,ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002297P+XP000053) Jackpot Poker",       0, layout_pe_poker )
GAMEL(1995, pex2302p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002302P+XP000038) Bonus Poker Deluxe",  0, layout_pe_poker )
GAMEL(1995, pex2303p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002303P+XP000112) White Hot Aces Poker", 0, layout_pe_poker )
GAMEL(1995, pex2306p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002306P+XP000112) Triple Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2307p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002307P+XP000112) Triple Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2308p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002308P+XP000112) Triple Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2310p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002310P+XP000112) Triple Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2312p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002312P+XP000112) Triple Bonus Poker Plus", 0, layout_pe_poker )
GAMEL(1995, pex2314p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002314P+XP000112) Triple Bonus Poker Plus", 0, layout_pe_poker )
GAMEL(1995, pex2374p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002374P+XP000112) Super Aces Poker",     0, layout_pe_poker )
GAMEL(1995, pex2377p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002377P+XP000112) Super Double Bonus Poker", 0, layout_pe_poker )
GAMEL(1995, pex2386p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002386P+XP000038) 4 of a Kind Bonus Poker", 0,layout_pe_poker )
GAMEL(1995, pex2412p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002412P+XP000096) Standard Draw with 5 decks (Two Pair or Better)", 0, layout_pe_poker )
GAMEL(1995, pex2419p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002419P+XP000064) Deuces Wild Bonus Poker - French", 0, layout_pe_poker )
GAMEL(1995, pex2420p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002420P+XP000064) Deuces Wild Bonus Poker - French", 0, layout_pe_poker )
GAMEL(1995, pex2421p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002421P+XP000064) Deuces Wild Bonus Poker - French", 0, layout_pe_poker )
GAMEL(1995, pex2440p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002440P+XP000053) Deuces Wild Poker",   0, layout_pe_poker )
GAMEL(1995, pex2461p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002461P+XP000055) Joker Poker (Two Pair or Better)", 0, layout_pe_poker )
GAMEL(1995, pex2474p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002474P+XP000038) Double Double Bonus Plus", MACHINE_NOT_WORKING, layout_pe_poker ) /* Needs unknown PE+ GAME POKER program to run */
GAMEL(1995, pex2478p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002478P+XP000154) Joker Poker - French", 0, layout_pe_poker )
GAMEL(1995, pex2479p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002479P+XP000154) Joker Poker - French", 0, layout_pe_poker )
GAMEL(1995, pex2480p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002480P+XP000154) Joker Poker (Aces or Better) - French", 0, layout_pe_poker )
GAMEL(1995, pex2485p,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X002485P+XP000154) Standard Draw Poker - French", 0, layout_pe_poker )

/* Superboard : Poker (Key On Credit) */
GAMEL(1995, pekoc766,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0766 A5W-A6F) Standard Draw Poker",  0, layout_pe_poker )
GAMEL(1995, pekoc801,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0801 A5W-A6F) 10's or Better",       0, layout_pe_poker )
GAMEL(1995, pekoc802,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0802 A5W-A6F) Standard Draw Poker",  0, layout_pe_poker )
GAMEL(1995, pekoc803,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0803 A5W-A6F) Joker Poker",          0, layout_pe_poker )
GAMEL(1995, pekoc803a, pekoc803,  peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0803 A50-A6N) Joker Poker",          0, layout_pe_poker )
GAMEL(1995, pekoc804,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0804 A5W-A6F) Bonus Poker Deluxe",   0, layout_pe_poker )
GAMEL(1995, pekoc806,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0806 A5W-A6F) Standard Draw Poker",  0, layout_pe_poker )
GAMEL(1995, pekoc818,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0818 A5W-A6F) Joker Poker (Aces or Better)", 0, layout_pe_poker )
GAMEL(1995, pekoc819,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0819 A5W-A6F) Bonus Poker Deluxe",   0, layout_pe_poker )
GAMEL(1995, pekoc825,  0,         peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (PP0825 A59-A7C) White Hot Aces",       0, layout_pe_poker )

/* Superboard : Multi-Poker */
GAMEL(1995, pexm001p,  0,        peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00001P+XMP00003) Multi-Poker",        0, layout_pe_poker )
GAMEL(1995, pexm002p,  pexm001p, peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00002P+XMP00006) Multi-Poker",        0, layout_pe_poker )
GAMEL(1995, pexm003p,  pexm001p, peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00003P+XMP00024) Multi-Poker",        0, layout_pe_poker )
GAMEL(1995, pexm004p,  0,        peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00004P+XMP00002) Multi-Poker",        0, layout_pe_poker )
GAMEL(1995, pexm005p,  0,        peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00005P+XMP00004) Multi-Poker",        0, layout_pe_poker )
GAMEL(1995, pexm006p,  pexm001p, peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00006P+XMP00020) Multi-Poker",        0, layout_pe_poker )
GAMEL(1995, pexm006pa, pexm001p, peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00006P+XMP00002) Multi-Poker (The Orleans)", 0, layout_pe_poker )
GAMEL(1995, pexm007p,  0,        peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00007P+XMP00006) Multi-Poker",        0, layout_pe_poker )
GAMEL(1995, pexm008p,  0,        peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00008P+XMP00006) Multi-Poker",        0, layout_pe_poker )
GAMEL(1995, pexm009p,  0,        peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00009P+XMP00002) Multi-Poker",        MACHINE_IMPERFECT_GRAPHICS, layout_pe_poker ) /* Needs unknown CG2??? graphics roms for correct MENU game banners */
GAMEL(1995, pexm013p,  0,        peplus,  peplus_poker, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XM00013P+XMP00025) Multi-Poker",        0, layout_pe_poker )

/* Superboard : Multi-Poker (Wingboard) */
GAMEL(1995, pexmp013,  0,        peplus,  peplus_poker, peplus_state, peplussbw,ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XMP00013) 5-in-1 Wingboard (CG2346) - Spanish", 0, layout_pe_poker )
GAMEL(1995, pexmp017,  0,        peplus,  peplus_poker, peplus_state, peplussbw,ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XMP00017) 5-in-1 Wingboard (CG2298)",   0, layout_pe_poker )
GAMEL(1995, pexmp017a, pexmp017, peplus,  peplus_poker, peplus_state, peplussbw,ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XMP00017) 5-in-1 Wingboard (CG2352)",   0, layout_pe_poker )
GAMEL(1995, pexmp017b, pexmp017, peplus,  peplus_poker, peplus_state, peplussbw,ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XMP00017) 5-in-1 Wingboard (CG2426)",   0, layout_pe_poker )
GAMEL(1995, pexmp026,  0,        peplus,  peplus_poker, peplus_state, peplussbw,ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XMP00026) 5-in-1 Wingboard (CG2346) - Spanish", 0, layout_pe_poker )
GAMEL(1995, pexmp030,  0,        peplus,  peplus_poker, peplus_state, peplussbw,ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XMP00030) 5-in-1 Wingboard (CG2451)",   0, layout_pe_poker )
GAMEL(1995, pexmp030a, pexmp030, peplus,  peplus_poker, peplus_state, peplussbw,ROT0,  "IGT - International Game Technology", "Player's Edge Plus (XMP00030) 5-in-1 Wingboard (CG2426)",   0, layout_pe_poker )

/* Superboard : Slots machine */
GAMEL(1997, pex0838s, 0,        peplus,  peplus_slots, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000838S+XS000002) Five Times Pay Slots",        0, layout_pe_slots )
GAMEL(1997, pex0841s, pex0838s, peplus,  peplus_slots, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000841S+XS000002) Five Times Pay Slots",        0, layout_pe_slots )
GAMEL(1997, pex0998s, 0,        peplus,  peplus_slots, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X000998S+XS000006) Triple Triple Diamond Slots", 0, layout_pe_slots )
GAMEL(1997, pex1087s, 0,        peplus,  peplus_slots, peplus_state, peplussb, ROT0,  "IGT - International Game Technology", "Player's Edge Plus (X001087S+XS000006) Double Double Diamond Slots", MACHINE_WRONG_COLORS, layout_pe_slots ) /* CAPX2415 not dumped */
