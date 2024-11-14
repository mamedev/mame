// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*******************************************************/
/*                                                     */
/* Yachiyo "Space Stranger/Space Stranger 2"           */
/* The cocktail version is also known as               */
/* "Super Space Stranger" but it is the same game      */
/*                                                     */
/*******************************************************/


/*
********************************************************

Space Stranger hardware info by Guru

This is a black/white Space Invaders clone made by Yachiyo Electric Co. Ltd. using colored cellophane
to make it look colored. The gameplay is mostly the same as any other Space Invaders game except the mother
ship changes direction randomly and if the player hits the shields 50 points are deducted from the score.
So be careful, score can go to 0 very quickly!
The sounds are very close to Taito Space Invaders, possibly even identical.
The hardware has a built-in service mode with RAM/ROM test and I/O check.
When enabling the I/O check, if ROMs or RAM are bad various sounds are made to denote the bad chips.
The following table summarizes the chip locations and sounds made. Note a bad CPU will also cause sounds
to be made on bootup but the I/O check or other in-game tests will not be available.

Chip#     Type       Sounds Made
------------------------------------------------------
38        4116 RAM   UFO Flying then Extra Ship
39        4116 RAM   Player Shoot then Extra Ship
40        4116 RAM   Player Explosion then Extra Ship
41        4116 RAM   Invader Explosion then Extra Ship
51        4116 RAM   UFO Flying then silence
52        4116 RAM   Player Shoot then silence
53        4116 RAM   Player Explosion then silence
54        4116 RAM   Invader Explosion then silence
58        EPROM 01   Player Shoot then Invader Explosion     \
59        EPROM 02   UFO Flying then Invader Explosion       |
60        EPROM 03   Invader Explosion                       |
61        EPROM 04   Player Explosion then Invader Explosion |
62        EPROM 05   Player Shoot then Player Explosion      | also with Invader movement sounds
63        EPROM 06   Player Explosion then UFO Flying        |
64        EPROM 07   Player Explosion                        |
65        EPROM 08   Player Shoot then UFO Flying            /

There are schematics available for Space Stranger but the scans are too small to read so completely useless.
There is another game known as 'Super Space Stranger' that has a service manual available with schematics.
It is also known as 'Mark Stranger 2' and is a color version of the same game.
There is a Super Space Stranger flyer that shows the game as b/w with color bars. The game has 'Super Space Stranger'
printed on the control panel of the b/w cocktail cabinet but the manual also just says 'Space Stranger' in many places,
so there is some uncertainty about exactly what the color version is called. The title screen of the color version just
says 'Space Stranger'. Super Space Stranger is said to be made by Banpresto but that is false because Banpresto didn't
exist until 1989. Additionally there is no evidence in the game or on the control panel or marquee or PCB stating any
other manufacturer other than Yachiyo.
It gets more complicated... the Super Space Stranger service manual has text at the bottom of each page 'HOEI SANGYO CORPORATION'
At the time Hoei (founded in 1977) was doing arcade-related work for Sega but there is no evidence to suggest the
PCB was manufactured by Hoei. It is possible Hoei made the PCB for Yachiyo since they were also doing arcade work
for Sega, although exactly what work (hardware or software) is unknown. Hoei changed their name to Coreland in 1982
and later in 1989 they became Banpresto.


Bottom Board
------------

YD7000-1A
   |------------------------------------------------|
   |             18.432MHz            |----------|  |
   |                        M5L8224   |   8080   |  |
   |                                  |----------|  |
   |                  7400   7404                   |
   |                                                |
   |           7474   7474   7408          7400     |
   |                                                |
|--|           7420   7404   7408   7408   7442     |
|                                                   |
|              7404   74161  7408   7408            |
|5                                         HSS_01   |
|0             7400   74161  74157  74157           |
|P                                                  |
|I             7474   74161  74157  74157  HSS_02   |
|N                                                  |
|              7402   74161  74157  74157           |
|E                                         HSS_03   |
|D             7408   7474   7408   7408            |
|G                                                  |
|E             7404   7410   74367  74367  HSS_04   |
|                                                   |
|C                    74165  74174  74174           |
|O                                         HSS_05   |
|N             7408   74165  74367  74367           |
|N                                                  |
|                                          HSS_06   |
|              7432          4116   4116            |
|--|                                                |
   |                         4116   4116   HSS_07   |
   |                  74121                         |
   |                         4116   4116            |
   |           7400   7432                 HSS_08   |
   |                         4116   4116            |
   |           10MHz  7404                 EMPTY    |
   |                                     DIP24_SKT  |
   |------------------------------------------------|
Notes: All IC's shown.
       8080 CPU  - AM8080 or M5L8080 CPU. Clock 2.048MHz (18.432/9 via M5L8224 Clock Generator IC)
       M5L8224   - Mitsubishi M5L8224 Clock Generator/Divider IC. Divider is fixed at divide-by-9
       HSS*      - 2708 EPROMs
       4116      - Mitsubishi M58759S-25 16k x1-bit DRAM compatible with 4116
       EMPTY     - Space for a 24 pin ROM but not populated with anything
       DIP24_SKT - DIP24 socket, empty


Top PCB
-------

YD7000-2B
   |----------------------------------------|
   |                                        |
   |                                        |
   |                                        |
   |                4030     4006           |
   |                                        |
   |                                        |
   |  LM380                  LM3900         |
|--|                                        |
|                                           |
|                            LM3900         |
|5    53200   7408                          |
|0                                      VR2 |
|P    74367   7414                          |
|I                  SW(5)    LM3900         |
|N    74367   7414                          |
|                                           |
|E    74367   7414           LM3900         |
|D                                      VR3 |
|G                                          |
|E                           LM3900         |
|                                           |
|C                                      VR7 |
|O                           LM3900         |
|N                                          |
|N                                      VR4 |
|                            LM3900         |
|          74174    7407                    |
|--|                                    VR1 |
   |               53204    SN76477         |
   |                                        |
   |                                    VR5 |
   |       74174   53217  1455  M53210      |
   |                                        |
   |                      1455         `VR6 |
   |                                        |
   |----------------------------------------|
Notes: All IC's shown.
       SN76477 - Texas Instruments Complex Sound Generator
       LM3900  - Quad Operational Amplifier
       LM380   - 2.5W Power Amplifier
       1455    - MC1455 equivalent to NE555 Timer
       VR1-7   - Volume Pots
                 VR1 -UFO flying sound (generated by the SN76477 IC)
                 VR2 -Shoot sound
                 VR3 -Player explosion sound
                 VR4 -Invader hit sound
                 VR5 -Bonus extra base sound
                 VR6 -Invader movement sound
                 VR7 -UFO hit sound
                 VR8 - Master volume pot (50k) mounted inside the cabinet (connected to edge connector, not mounted on the PCB)
       SW(5)   - 5-position DIP Switch marked 'SW'
       53204   - Mitsubishi branded logic chip equivalent to 7404
       53217   - Mitsubishi branded logic chip equivalent to 7417
       53210   - Mitsubishi branded logic chip equivalent to 7410
       53200   - Mitsubishi branded logic chip equivalent to 7400


Joiner Board (parts side shown)
------------

YD7000-M2
|-----------------|                                |-----------------|
|*  14-WAY CONN   |                                |*  14-WAY CONN   |
|      LEFT       |--------------------------------|      RIGHT      |
|                                                                    |
|   |------------------------------------------------------------|   | <--- Top board plugs in here
|   |------------------------------------------------------------|   |
|                                                                    |
|   |------------------------------------------------------------|   | <--- Bottom board plugs in here
|   |------------------------------------------------------------|   |
|                                                                    |
|--------------------------------------------------------------------|
Notes:
      The long slots are KEL 4800-100-035 100 pin female card edge connectors with 0.156" pin spacing.
      On the diagram above * denotes pin 1 of the 14 pin connectors.
      Space Stranger uses this board to join the top and bottom boards together and provides a way to wire up the
      controls/video/power etc to the machine via the left and right 14-way edge connectors.
      The game has a table/upright mode that can be changed by connecting two jumper pads on the joiner PCB with a solder blob.
      This is located at pin 40 on the top board and is conveniently made available on the back side of the joiner board
      with the two solder pads. When open it sets table mode, when closed it sets upright mode.

      Pinout of edge connector
      ------------------------

      LEFT PARTS    LEFT SOLDER               RIGHT PARTS    RIGHT SOLDER
      -------------------------               ---------------------------
          GND  1    A  GND                         GND  1    A  GND
          GND  2    B  GND                         GND  2    B  GND
           NC  3    C  COIN COUNTER                 NC  3    C  VR8 MASTER VOLUME POT RIGHT PIN   \ (left pin of volume pot unconnected)
           NC  4    D  NC                           NC  4    D  VR8 MASTER VOLUME POT MIDDLE PIN  /
     1P START  5    E  I/O CHECK                    5V  5    E  5V
     2P START  6    F  COIN                         5V  6    F  5V
      UNKNOWN  7    G  SERVICE COIN          *2P SHOOT  7    G  NC
     1P RIGHT  8    H  2P RIGHT                    12V  8    H  12V
      1P LEFT  9    I  2P LEFT                     12V  9    I  12V
     1P SHOOT 10    J  2P SHOOT                    -5V 10    J  -5V
       SPKR + 11    K  SPKR -                      -5V 11    K  -5V
           NC 12    L  NC                    *2P RIGHT 12    L  *2P LEFT A (* - 2P controls for table)
         SYNC 13    M  VIDEO                       12V 13    M  12V
          GND 14    N  GND                         12V 14    N  12V

Note: NC means No Connection.
To access the I/O Check Mode, ground R17 (side near edge connector) on the top board or ground pin E
of the left edge connector then power up the game.
The I/O test screen shows 3x 8-bit memory locations with bits that change when controls and settings
are adjusted. Note if sounds are heard, check the RAM/ROM test table above.
For the purpose of describing each bit, they will be called Byte A, B & C.
With all DIP switches off and controls at default positions, the 8-bit memory locations look like
this in the default state:
                          1 1 1 1 1 0 0 0 <-- Byte A
                          0 0 0 1 0 0 0 0 <-- Byte B
                          % 0 1 1 1 1 1 1 <-- Byte C
0 means open, 1 means closed. & denotes moving fast between 0 and 1
Refer to the table below for the meaning of each bit.
Bits described on each line are the position(s) denoted by the X and
as read from the screen left to right.

Byte A
------
X X X X X - - -  : Dip Switches 1-5. 1 when DSW is off, 0 when DSW is on
- - - - - X - -  : Fire Player 2 (Note when pressed, the characters on screen are flipped vertically. This is normal)
- - - - - - X -  : Left Player 2
- - - - - - - X  : Right Player 2

Byte B
------
X - - - - - - -  : Coin
- X - - - - - -  : 2 Player Start
- - X - - - - -  : 1 Player Start
- - - X - - - -  : I/O Check (always 1 when in the I/O check mode)
- - - - X - - -  : Table/Upright setting. 0 for table, 1 for upright
- - - - - X - -  : Fire Player 1
- - - - - - X -  : Left Player 1
- - - - - - - X  : Right Player 1

Byte C
------
X - - - - - - -  : Interrupt. This is always rapidly moving between 0 and 1
- X - - - - - -  : Rapid Fire (only used on color version)
- - X X X X X X  : Always 1


Color version Space Stranger 2 / Mark Stranger 2 etc
----------------------------------------------------
As well as a different PCB number (YD-7700), the color version PCB has one 64kbit mask ROM and one 2708 EPROM
for the code and an extra 2708 EPROM near the 10MHz crystal for color data and a few extra logic chips, so it
is easy to spot the color version just by looking at the bottom PCB. The top board is identical to the older
b/w version Space Stranger top board and sounds are also identical between versions.

Bottom Board
------------

YD7700-1G
   |------------------------------------------------|
   |             18.432MHz            |----------|  |
   |           7408         M5L8224   |   8080   |  |
   |                                  |----------|  |
   |    7408   7408   7400   7404                   |
   |                                                |
   |           7474   7474   7408          7400     |
   |                                                |
|--|           7420   7404   7408   7408   7442     |
|                                                   |
|              7404   74161  7408   7408            |
|5                                         DIP24    |
|0             7400   74161  74157  74157           |
|P                                                  |
|I             7474   74161  74157  74157  DIP24    |
|N                                                  |
|              7402   74161  74157  74157           |
|E                                         DIP24    |
|D             7408   7474   7408   7408            |
|G                                                  |
|E             7404   7410   74367  74367  DIP24    |
|                                                   |
|C                    74165  74174  74174           |
|O                                         DIP24    |
|N      7417   7408   74165  74367  74367           |
|N                                                  |
|                                          DIP24    |
|       7402   7432   7420   4116   4116            |
|--|                                                |
   |                         4116   4116   DIP24    |
   |    7400          74121                         |
   |                         4116   4116            |
   |    74174  7400   7432                 DIP24    |
   |                         4116   4116   8KMASK   |
   |           10MHz  7404                 2708     |
   |   2708                            74367 74367  |
   |------------------------------------------------|
Notes: All IC's shown.
       8080 CPU  - AM8080 or M5L8080 CPU. Clock 2.048MHz (18.432/9 via M5L8224 Clock Generator IC)
       M5L8224   - Mitsubishi M5L8224 Clock Generator/Divider IC. Divider is fixed at divide-by-9
       2708      - 2708 EPROMs
       4116      - Mitsubishi M58759S-25 16k x1-bit DRAM compatible with 4116
       DIP24     - DIP24 socket, empty. Note early revisions used 2708 EPROMs at these locations, and no 8K mask ROM


PCB video reference for Space Stranger 2: https://www.youtube.com/watch?v=mn8hCd_uqcI

TODO:
      * Using sound samples from Space Invaders (real PCB sounds virtually identical anyway). Get schematics and use netlist instead.
      * Fix attract mode and I/O test random sound (should be silent)
      * Screen should flash red when losing a life in Space Stranger 2 (see video reference)

********************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/samples.h"
#include "sound/sn76477.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "sstrangr.lh"


namespace {

class sstrangr_state : public driver_device
{
public:
	sstrangr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_sn(*this, "snsnd"),
		m_palette(*this, "palette"),
		m_ram(*this, "ram")
	{ }

	void sstrngr2(machine_config &config);
	void sstrangr(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<sn76477_device> m_sn;
	optional_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_ram;

	uint8_t m_flip_screen = 0;
	uint8_t m_last_sound1 = 0;
	uint8_t m_last_sound2 = 0;

	void port44_w(uint8_t data);
	void port42_w(uint8_t data);

	uint32_t screen_update_sstrangr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sstrngr2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void sstrangr_io_map(address_map &map) ATTR_COLD;
	void sstrangr_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Video system
 *
 *************************************/

void sstrangr_state::video_start()
{
	m_last_sound1 = m_last_sound2 = 0xff;

	save_item(NAME(m_flip_screen));
	save_item(NAME(m_last_sound1));
	save_item(NAME(m_last_sound2));
}

uint32_t sstrangr_state::screen_update_sstrangr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t x = offs << 3;
		int y = offs >> 5;
		uint8_t data = m_ram[offs];

		for (int i = 0; i < 8; i++)
		{
			pen_t pen;

			if (m_flip_screen)
			{
				pen = (data & 0x80) ? rgb_t::white() : rgb_t::black();
				data <<= 1;
			}
			else
			{
				pen = (data & 0x01) ? rgb_t::white() : rgb_t::black();
				data >>= 1;
			}

			bitmap.pix(y, x) = pen;

			x++;
		}
	}

	return 0;
}

uint32_t sstrangr_state::screen_update_sstrngr2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t *color_map_base = &memregion("proms")->base()[m_flip_screen ? 0x0000 : 0x0200];

	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		offs_t color_address = (offs >> 9 << 5) | (offs & 0x1f);

		uint8_t data = m_ram[offs];
		uint8_t fore_color = color_map_base[color_address] & 0x07;

		for (int i = 0; i < 8; i++)
		{
			uint8_t color;

			if (m_flip_screen)
			{
				color = (data & 0x80) ? fore_color : 0;
				data <<= 1;
			}
			else
			{
				color = (data & 0x01) ? fore_color : 0;
				data >>= 1;
			}

			bitmap.pix(y, x) = m_palette->pen_color(color);

			x++;
		}
	}

	return 0;
}


void sstrangr_state::port44_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_last_sound2;
	if (BIT(rising_bits, 0)) m_samples->start(4, 3);   // fleet move 1
	if (BIT(rising_bits, 1)) m_samples->start(4, 4);   // fleet move 2
	if (BIT(rising_bits, 2)) m_samples->start(4, 5);   // fleet move 3
	if (BIT(rising_bits, 3)) m_samples->start(4, 6);   // fleet move 4
	if (BIT(rising_bits, 4)) m_samples->start(3, 7);   // Hit Saucer

	m_sn->enable_w(!BIT(data, 0));     // Saucer Sound

	m_last_sound2 = data;

	m_flip_screen = data & 0x20;
}

void sstrangr_state::port42_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_last_sound1;
	if (BIT(rising_bits, 0)) logerror("42 0 data: %02x rb: %02x\n", data, rising_bits);
	if (BIT(rising_bits, 1)) m_samples->start(0, 0);   // Shoot
	if (BIT(rising_bits, 2)) m_samples->start(1, 1);   // Death
	if (BIT(rising_bits, 3)) m_samples->start(2, 2);   // Hit
	if (BIT(rising_bits, 4)) m_samples->start(5, 8);   // Bonus
	//if (BIT(rising_bits, 5)) logerror("42 bit 5"); // 1x after starting a game/life

	m_sn->enable_w(!BIT(data, 0));     // Saucer Sound

	m_last_sound1 = data;
}

void sstrangr_state::sstrangr_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram().share("ram");
	map(0x6000, 0x63ff).rom();
}


void sstrangr_state::sstrangr_io_map(address_map &map)
{
	map(0x41, 0x41).portr("DSW");
	map(0x42, 0x42).portr("INPUTS").w(FUNC(sstrangr_state::port42_w));
	map(0x44, 0x44).portr("EXT").w(FUNC(sstrangr_state::port44_w));
}


static INPUT_PORTS_START( sstrangr )
	PORT_START("DSW") // 1 x 5-dip bank
	PORT_DIPNAME( 0x03, 0x01, "Extra Play" )           PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x02, "4000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW:5" ) // Must be ACTIVE_LOW (OFF) for game to boot
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )  // This is an edge connector pin for testing ROM/RAM and I/O ports
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("EDGE:1")  // This is an edge connector pin that is
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )                                 // brought out to two solder pads on the
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )                                // Joiner PCB
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START("EXT")      // External switches
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const char *const sstrangr_sample_names[] =
{
	"*invaders",
	"1",        // shot/missile
	"2",        // base hit/explosion
	"3",        // invader hit
	"4",        // fleet move 1
	"5",        // fleet move 2
	"6",        // fleet move 3
	"7",        // fleet move 4
	"8",        // UFO/saucer hit
	"9",        // bonus base
	nullptr
};

void sstrangr_state::sstrangr(machine_config &config)
{
	// basic machine hardware
	I8080A(config, m_maincpu, XTAL(18'432'000)/9); // M5L8080AP, measured 2047840 Hz
	m_maincpu->set_addrmap(AS_PROGRAM, &sstrangr_state::sstrangr_map);
	m_maincpu->set_addrmap(AS_IO, &sstrangr_state::sstrangr_io_map);
	m_maincpu->set_periodic_int(FUNC(sstrangr_state::irq0_line_hold), attotime::from_hz(2*60));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(32*8, 262);     // vert size is a guess, taken from mw8080bw
	screen.set_visarea(0*8, 32*8-1, 4*8, 32*8-1);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(sstrangr_state::screen_update_sstrangr));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76477(config, m_sn);
	m_sn->set_noise_params(0, 0, 0);
	m_sn->set_decay_res(0);
	m_sn->set_attack_params(0, RES_K(100));
	m_sn->set_amp_res(RES_K(56));
	m_sn->set_feedback_res(RES_K(10));
	m_sn->set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(120));
	m_sn->set_oneshot_params(0, 0);
	m_sn->set_vco_mode(1);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.5);

	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(sstrangr_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*******************************************************/
/*                                                     */
/* Yachiyo "Space Stranger 2"                          */
/*                                                     */
/*******************************************************/

// Color version of Space Stranger, board has Stranger 2 written on it.
// Clarification required for the above comment.
// Stranger 2? Really? The number on the PCB is YD7700-1G with no other markings present.
// The game is actually known as Super Space Stranger according to the manual.

static INPUT_PORTS_START( sstrngr2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Extra Play" )            PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x02, "4000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR(Coinage) )        PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )  // This is an edge connector pin for testing ROM/RAM and I/O ports
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("EDGE:1")  // This is an edge connector pin that is
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )                                 // brought out to two solder pads on the
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )                                // Joiner PCB
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("EXT")      /* External switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x02, 0x00, "Player's Bullet Speed" )  PORT_DIPLOCATION("EDGE:2")  // This is an edge connector pin listed in the manual as 'Fast Shoot'
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void sstrangr_state::sstrngr2(machine_config &config)
{
	sstrangr(config);

	// video hardware
	subdevice<screen_device>("screen")->set_screen_update(FUNC(sstrangr_state::screen_update_sstrngr2));

	PALETTE(config, m_palette, palette_device::RBG_3BIT);
}



ROM_START( sstrangr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hss-01.58",     0x0000, 0x0400, CRC(feec7600) SHA1(787a6be4e24ce931e7678e777699b9f6789bc199) )
	ROM_LOAD( "hss-02.59",     0x0400, 0x0400, CRC(7281ff0b) SHA1(56649d1362be1b9f517cb8616cbf9e4f955e9a2d) )
	ROM_LOAD( "hss-03.60",     0x0800, 0x0400, CRC(a09ec572) SHA1(9c4ad811a6c0460403f9cdc9fe5381c460249ff5) )
	ROM_LOAD( "hss-04.61",     0x0c00, 0x0400, CRC(ec411aca) SHA1(b72eb6f7c3d69e2829280d1ab982099f6eff0bde) )
	ROM_LOAD( "hss-05.62",     0x1000, 0x0400, CRC(7b1b81dd) SHA1(3fa6e244e203fb75f92b19db7b4b18645b3f66a3) )
	ROM_LOAD( "hss-06.63",     0x1400, 0x0400, CRC(de383625) SHA1(7ec0d7171e771c4b43e026f3f50a88d8ab2236bb) )
	ROM_LOAD( "hss-07.64",     0x1800, 0x0400, CRC(2e41d0f0) SHA1(bba720b0c5a7bd47abb8bc8498a989e17dc52428) )
	ROM_LOAD( "hss-08.65",     0x1c00, 0x0400, CRC(bd14d0b0) SHA1(9665f639afef9c1291f2efc054216ff44c595b45) )
ROM_END

ROM_START( sstrangr2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4764.09",      0x0000, 0x2000, CRC(d88f86cc) SHA1(9f284ee50caf3c64bd04a79a798de620348881bc) )
	ROM_LOAD( "2708.10",      0x6000, 0x0400, CRC(eba304c1) SHA1(3fa6fbb29fa46c146283f69a712bfc51cbb2a43c) )

	ROM_REGION( 0x0400, "proms", 0 )        // color maps player 1/player 2
	ROM_LOAD( "2708.15",      0x0000, 0x0400, CRC(c176a89d) SHA1(955dd540dc3787091c3f34ae122a13e6b7523414) )
ROM_END

} // anonymous namespace


GAMEL( 1978, sstrangr,  0,        sstrangr, sstrangr, sstrangr_state, empty_init, ROT270, "Yachiyo Electronics, Ltd.", "Space Stranger",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_sstrangr )
GAME(  1979, sstrangr2, sstrangr, sstrngr2, sstrngr2, sstrangr_state, empty_init, ROT270, "Yachiyo Electronics, Ltd.", "Space Stranger 2", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
