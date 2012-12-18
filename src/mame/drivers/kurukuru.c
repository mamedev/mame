/**************************************************************************

  KURU KURU PYON PYON
  Taiyo Jidoki / Success


  Driver by Roberto Fresca & hap.


  This hardware seems to be a derivative of MSX2 'on steroids'.

  Depending how complex is turning the emulation, this driver
  could be merged later with tonton.c, since the platforms are
  sharing the hardware base...


***************************************************************************

  KURU KURU PYON PYON
  (c)SUCCESS / CABINET :TAIYO JIDOKI

  CPU   : 2x Sharp LH0080A Z80A

  MEM   : 1x Sharp LH5116H-10 (2KB SRAM)
          1x Fairchild 8464A-10L (8KB SRAM) + battery
          6x Sharp LH2464-15 (192KB Video DRAM total)

  SOUND : 1x Yamaha YM2149F
          1x OKI M5205

  VIDEO : 1x Unknown 64-legs VDP (seems to be from V9938/58 family).

  XTAL  : 1x 21477.27 kHz.
          1x 384 kHz. blue resonator (CSB 384 P) surely for the M5202.


  1x Texas Instruments RC4558P (Dual General-Purpose Operational Amplifier, DIP8)
  1x Fairchild MB3712 (5,7 Watt Audio Power Amplifier, SIP8).
  2x 8 DIP switches banks.

  3x PAL16L8A (IC12, IC26 & IC27)
  1x PAL12L6 (IC32)


***************************************************************************

  PCB Layout...

  .--------------------------------------------------------------------------------------------------.
  |                                   IC4                  IC5                                       |
  |    IC2            IC3           .------------------.  .----------------.            BATT         |
  |  .-----------.  .-----------.   | M5L27512K        |  | LH5116H-10     |          .---------.    |
  |  | LH2464-15 |  | SN74HC04N |   |                  |  | SHARP JAPAN    |          |  3.6 V  |    |
  |  '-----------'  '-----------'   | '4'              |  |                |         -+         +-   |
  |    IC6                          |                  |  '----------------'          | BATTERY |    |
  |  .-----------.    IC8           '------------------'                              '---------'    |
  |  | LH2464-15 |  .----------------------------.       IC9                      IC10               |
  |  '-----------'  | LH0080A Z80A-CPU-D         |     .-----------------.      .-----------------.  |
  |    IC7          | SHARP JAPAN                |     | 8464A-10L       |      | M5L27512K       |  |
  |  .-----------.  |                            |     | |F| JAPAN       |      |                 |  |
  |  | LH2464-15 |  |                            |     |                 |      | '10'            |  |
  |  '-----------'  '----------------------------'     |                 |      |                 |  |
  |    IC11           IC12            IC13             '-----------------'      '-----------------'  |
  |  .-----------.  .------------.  .------------.       IC17                     IC18               |
  |  | LH2464-15 |  | PAL16L8ACJ |  | SN74LS74AN |     .-----------------.      .-----------------.  |
  |  '-----------'  '------------'  '------------'     | MBM27256-25     |      | M5L27512K       |  |
  |    IC14           IC15            IC16             |                 |      |                 |  |
  |  .-----------.  .------------.  .-------------.    | 'KP 17L'        |      | '18'            |  |
  |  | LH2464-15 |  | SN74LS08N  |  | SN74LS125AN |    |                 |      |                 |  |
  |  '-----------'  '------------'  '-------------'    '-----------------'      '-----------------'  |
  |    IC19           IC20            IC21           IC22                         IC23               |
  |  .-----------.  .------------.  .------------. .--------------------------. .-----------------.  |
  |  | LH2464-15 |  | SN74LS174N |  | SN74LS174N | | LH0080A Z80A-CPU-D       | |                 |  |
  |  '-----------'  '------------'  '------------' | SHARP JAPAN              | |    E M P T Y    |  |
  |                                   X2           |                          | |   S O C K E T   |  |
  |                                 .--------.     |                          | |                 |  |
  |    X1             IC24          | | RES  |     '--------------------------' '-----------------'  |
  |  .-----.        .------------.  | |384kHz|                                                       |
  |  |XTAL |        |OKI MSM5205 |  '--------'                                                       |
  |  |21.47|        '------------'    IC25             IC26            IC27            IC28          |
  |  |727  |                        .-------------.  .------------.  .------------.  .------------.  |
  |  '-----'                        | SN74LS374N  |  | PAL16L8ACJ |  | PAL16L8ACJ |  | HD74LS153P |  |
  |                                 '-------------'  '------------'  '------------'  '------------'  |
  |    IC29                           IC30             DSW1            IC31            IC32          |
  |  .-----------------------------. .----.          .------------.  .------------.  .------------.  |
  |  | SANDED...                   | | AA |          |  11111111  |  | SN74LS174N |  | PAL12L6CN  |  |
  |  | (YAMAHA V9938 VDP)          | '----'          '------------'  '------------'  '------------'  |
  |  |                             |                   IC33                           '7908B-4'      |
  |  | '81500'                     |                 .------------.    IC34                          |
  |  |                             |                 | SN74LS245N |  .----------------------------.  |
  |  |                             |                 '------------'  | YAMAHA                     |  |
  |  '-----------------------------'               DSW2              | YM2149F                    |  |
  |                                              .------------.      |                            |  |
  |                                              |  01111110  |      |                            |  |
  |                                              '------------'      '----------------------------'  |
  |   IC35    IC36    IC37    IC38     IC39            IC40            IC41                          |
  |  .----.  .----.  .----.  .----.  .------------.  .------------.  .------------.                  |
  |  | AB |  | AB |  | AB |  | AB |  | HD74LS273P |  | SN74LS245N |  | SN74LS245N |  .------------.  |
  |  '----'  '----'  '----'  '----'  '------------'  '------------'  '------------'  |  IC43      |  |
  |                                      IC42                                        |.----------.|  |
  |                                    .----------.   ||||||||||||    ||||||||||||   ||MB3712 M92||  |
  |                                    | ULN2003A |   +++RESNET+++    +++RESNET+++   |'----------'|  |
  |                                    '----------'   ||||||||||||    ||||||||||||   '------------'  |
  |                                                                                                  |
  | 7908-B                                                                             SUCCESS CORP. |
  |         .----------.           .--.                                         .----------.         |
  |         |          | | | | | | |  | | | | | | | | | | | | | | | | | | | | | |          |         |
  |         |          | | | | | | |  | | | | | | | | | | | | | | | | | | | | | |          |         |
  '---------'          '-----------'  '-----------------------------------------'          '---------'
                       2x6 edge conn             2x21 edge connector


  AA =  Texas Instruments RC4558P T835AJ34.
  AB =  NEC C1663C 8926B.


***************************************************************************

  Notes....

  The game name could be translated as "Croak Croak Hop Hop"
  Kuru is the frog sound, and Pyon is the sound of jumps.

  The game is playable, even when you can't hear all sounds.

  Coin 1 (key 5) is not working properly and could hang the system.
  Once pressed, the game spits a message that means "Jammed Medal".
  For now, use Coin 2 (key 6) and Service (key 8) for credits...

  If you pressed Coin 1 and the game is not responding anymore, press RESET
  (key 0) and the game will reset to default values.


  In the Book Keeping, you can find the statistics...

  1st screen...

  - OMAKE:  Extra/Bonus.

  2nd screen...

  - TATE:   Vertical.
  - YOKO:   Horizontal.
  - NANAME: Diagonal.

  ...for each character (BOTE, OUME, PYOKO, KUNIO & PP).

  Also...

  - AKA:    Red.
  - KURO:   Black.


***************************************************************************

  Samples....

  There are 14 samples in the system.

  00: "boterin" (?)
  01:
  02: "hakase" ("professor")
  03: "pyokorin"
  04: "kunio"
  05: "pyon pyon"
  06:
  07:
  08: "oume"
  09: "haipaa" ("hyper")
  10: "ichi ni tsuite" ("on your marks")
  11: "youi" ("get ready")
  12: bang sound for the tadpoles landing in the right panel
  13: sound for reels when running


***************************************************************************

  TODO:

  - Hook up AY8910 output ports. Or unused?
  - Find why the use of coin 1 always jams. Hopper?


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "video/v9938.h"
#include "machine/nvram.h"

class kurukuru_state : public driver_device
{
public:
	kurukuru_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_v9938(*this, "v9938")
	{ }

	required_device<cpu_device> m_audiocpu;
	required_device<v9938_device> m_v9938;

	UINT8 m_sound_irq_cause;
	UINT8 m_adpcm_data;

	DECLARE_WRITE8_MEMBER(kurukuru_hopper_w);
	DECLARE_WRITE8_MEMBER(kurukuru_bankswitch_w);
	DECLARE_WRITE8_MEMBER(kurukuru_soundlatch_w);
	DECLARE_READ8_MEMBER(kurukuru_soundlatch_r);
	DECLARE_WRITE8_MEMBER(kurukuru_adpcm_reset_w);
	DECLARE_READ8_MEMBER(kurukuru_adpcm_timer_irqack_r);
	DECLARE_WRITE8_MEMBER(kurukuru_adpcm_data_w);

	void update_sound_irq(UINT8 cause);
	virtual void machine_start();
	virtual void machine_reset();
	TIMER_DEVICE_CALLBACK_MEMBER(kurukuru_vdp_scanline);
};

#define MAIN_CLOCK		XTAL_21_4772MHz
#define M5205_CLOCK		XTAL_384kHz

#define VDP_MEM            0x30000

/* from MSX2 driver, may be not accurate for this HW */
#define MSX2_XBORDER_PIXELS		16
#define MSX2_YBORDER_PIXELS		28
#define MSX2_TOTAL_XRES_PIXELS		256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS		212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS	8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS	14 * 2


/*************************************************
*                  Interrupts                    *
*************************************************/

static void kurukuru_vdp_interrupt(device_t *, v99x8_device &device, int i)
{
	device.machine().device("maincpu")->execute().set_input_line(0, (i ? ASSERT_LINE : CLEAR_LINE));
}


TIMER_DEVICE_CALLBACK_MEMBER(kurukuru_state::kurukuru_vdp_scanline)
{
	m_v9938->set_resolution(0);
	m_v9938->interrupt();
}


void kurukuru_state::update_sound_irq(UINT8 cause)
{
	m_sound_irq_cause = cause & 3;
	if (m_sound_irq_cause)
	{
		// use bit 0 for latch irq, and bit 1 for timer irq
		// latch irq vector is $ef (rst $28)
		// timer irq vector is $f7 (rst $30)
		// if both are asserted, the vector becomes $f7 AND $ef = $e7 (rst $20)
		const UINT8 irq_vector[4] = { 0x00, 0xef, 0xf7, 0xe7 };
		m_audiocpu->set_input_line_and_vector(0, ASSERT_LINE, irq_vector[m_sound_irq_cause]);
	}
	else
	{
		m_audiocpu->set_input_line(0, CLEAR_LINE);
	}
}


static void kurukuru_msm5205_vck(device_t *device)
{
	kurukuru_state *state = device->machine().driver_data<kurukuru_state>();
	state->update_sound_irq(state->m_sound_irq_cause | 2);
	msm5205_data_w(device, state->m_adpcm_data);
}


/*************************************************
*               Memory Map / I/O                 *
*************************************************/

// Main CPU

WRITE8_MEMBER(kurukuru_state::kurukuru_hopper_w)
{
	/* assume hopper related.
		$01 when coin 1 (jams)
		$20 when coin 2
		$40 when payout (jams) ...check
	*/
	if (data)
		logerror("kurukuru_hopper_w %02X @ %04X\n", data, space.device().safe_pc());
}

WRITE8_MEMBER(kurukuru_state::kurukuru_bankswitch_w)
{
	// d4,d5: bank
	// other bits: ?
	membank("bank1")->set_entry(data >> 4 & 3);
}

WRITE8_MEMBER(kurukuru_state::kurukuru_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data);
	update_sound_irq(m_sound_irq_cause | 1);
}


static ADDRESS_MAP_START( kurukuru_map, AS_PROGRAM, 8, kurukuru_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x6000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kurukuru_io, AS_IO, 8, kurukuru_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(kurukuru_hopper_w)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1")
	AM_RANGE(0x20, 0x20) AM_WRITE(kurukuru_soundlatch_w)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE( "v9938", v9938_device, read, write )
	AM_RANGE(0x90, 0x90) AM_WRITE(kurukuru_bankswitch_w)
	AM_RANGE(0xa0, 0xa0) AM_READ_PORT("IN0")
	AM_RANGE(0xb0, 0xb0) AM_READ_PORT("IN1")
	AM_RANGE(0xc0, 0xc0) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0xc8, 0xc8) AM_READ_PORT("DSW2")
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
ADDRESS_MAP_END



// Audio CPU

WRITE8_MEMBER(kurukuru_state::kurukuru_adpcm_data_w)
{
	m_adpcm_data = data & 0xf;
}

WRITE8_MEMBER(kurukuru_state::kurukuru_adpcm_reset_w)
{
	// d0: reset adpcm chip
	// other bits: ?
	msm5205_reset_w(machine().device("adpcm"), data & 1);
	update_sound_irq(m_sound_irq_cause);
}

READ8_MEMBER(kurukuru_state::kurukuru_soundlatch_r)
{
	update_sound_irq(m_sound_irq_cause & ~1);
	return soundlatch_byte_r(space, 0);
}

READ8_MEMBER(kurukuru_state::kurukuru_adpcm_timer_irqack_r)
{
	update_sound_irq(m_sound_irq_cause & ~2);
	return 0;
}


static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, kurukuru_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_io, AS_IO, 8, kurukuru_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_WRITE(kurukuru_adpcm_data_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(kurukuru_adpcm_reset_w)
	AM_RANGE(0x60, 0x60) AM_READ(kurukuru_soundlatch_r)
	AM_RANGE(0x70, 0x70) AM_READ(kurukuru_adpcm_timer_irqack_r)
ADDRESS_MAP_END


/*************************************************
*            Input Ports Definitions             *
*************************************************/

static INPUT_PORTS_START( kurukuru )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("1st (BOTE)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("2nd (OUME)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("3rd (PYOKO)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("4th (KUNIO)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("5th (PP)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("unknown N")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_M) PORT_NAME("unknown M")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_NAME("Bookkeeping")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8) PORT_NAME("Service")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Reset Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_A) PORT_NAME("Unknown 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE (2)								// coin 1 is not incrementing the credits and jams
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_S) PORT_NAME("Unknown 2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )											// payout write the pulses, but jams.

	PORT_START("DSW1")	// found in the PCB: 11111111
	PORT_DIPNAME( 0x07, 0x03, "Coinage A (100 Y)" )	PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x02, "1 Coin / 3 Medal" )
	PORT_DIPSETTING(    0x06, "1 Coin / 4 Medal" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Medal" )
	PORT_DIPSETTING(    0x05, "1 Coin / 6 Medal" )
	PORT_DIPSETTING(    0x03, "1 Coin / 10 Medal" )
	PORT_DIPSETTING(    0x07, "1 Coin / 11 Medal" )
	PORT_DIPSETTING(    0x04, "1 Coin / 20 Medal" )
	PORT_DIPSETTING(    0x00, "1 Coin / 50 Medal" )
	PORT_DIPNAME( 0x18, 0x08, "Coinage B (10 Y)" )	PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, "3 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x10, "2 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x18, "1 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x08, "1 Coin / 2 Medal" )
	PORT_DIPNAME( 0x20, 0x20, "Service Coinage" )	PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, "1 Pulse / 1 Medal" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 2 Medal" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )		PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x80, 0x00, "Repeat Bet")			PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")	// found in the PCB: 01111110
	PORT_DIPNAME( 0x07, 0x01, "Percentage" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x08, 0x08, "Winwave" )		PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, "Small" )
	PORT_DIPSETTING(    0x00, "Big" )
	PORT_DIPNAME( 0x10, 0x10, "M.Medal" )		PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "HG" )			PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(    0x60, "10-1" )
	PORT_DIPSETTING(    0x20, "20-1" )
	PORT_DIPSETTING(    0x40, "50-1" )
	PORT_DIPSETTING(    0x00, "100-1" )
	PORT_DIPNAME( 0x80, 0x80, "Bet Max" )		PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "10" )

INPUT_PORTS_END


/*************************************************
*        Machine Start & Reset Routines          *
*************************************************/

void kurukuru_state::machine_start()
{
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base(), 0x8000);
}

void kurukuru_state::machine_reset()
{
	update_sound_irq(0);
}


/*************************************************
*                Sound Interfaces                *
*************************************************/

static const ay8910_interface ay8910_intf =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_UNMAPPED,
	DEVCB_UNMAPPED,
	DEVCB_UNMAPPED,	// some writes...
	DEVCB_UNMAPPED	// some writes...
};

static const msm5205_interface msm5205_config =
{
	kurukuru_msm5205_vck,
	MSM5205_S48_4B		/* 8 kHz? */
};


/*************************************************
*                 Machine Driver                 *
*************************************************/

static MACHINE_CONFIG_START( kurukuru, kurukuru_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(kurukuru_map)
	MCFG_CPU_IO_MAP(kurukuru_io)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", kurukuru_state, kurukuru_vdp_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, MAIN_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_IO_MAP(audio_io)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_V9938_ADD("v9938", "screen", VDP_MEM)
	MCFG_V99X8_INTERRUPT_CALLBACK_STATIC(kurukuru_vdp_interrupt)

	MCFG_SCREEN_ADD("screen",RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, MSX2_TOTAL_YRES_PIXELS)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
	MCFG_SCREEN_UPDATE_DEVICE("v9938", v9938_device, screen_update)

	MCFG_PALETTE_LENGTH(512)
	MCFG_PALETTE_INIT( v9938 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", YM2149, MAIN_CLOCK/12)
	MCFG_SOUND_CONFIG(ay8910_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("adpcm", MSM5205, XTAL_384kHz)
	MCFG_SOUND_CONFIG(msm5205_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kurukuru )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "kp_17l.ic17",  0x00000, 0x08000, CRC(9b552ebc) SHA1(07d0e62b7fdad381963a345376b72ad31eb7b96d) )	// program code
	ROM_LOAD( "10.ic10",      0x08000, 0x10000, CRC(3d6012bc) SHA1(2764f70e0e0bef3f2f71dd6c78e0a4189057beca) )	// title + text + ingame gfx
	ROM_LOAD( "18.ic18",      0x18000, 0x10000, CRC(afb13c6a) SHA1(ac3cd40fad081f7a2b3d1fc72ea96282b9d1f4a3) )	// big frog gfx

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4.ic4",    0x00000, 0x10000, CRC(85d86f32) SHA1(f2aa93d702e6577f8f2204c74c44ac26d05be699) )	// code & adpcm samples

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "51.ic26",	  0x0000, 0x0104, CRC(ce4a601b) SHA1(07f5bbb327b220e5846927cbb91149174dd07b36) )
	ROM_LOAD( "52.ic27",	  0x0200, 0x0104, CRC(e23296a5) SHA1(4747923d201fcc5e0e752acbf50b41f0414e4ca8) )
	ROM_LOAD( "53.ic12",	  0x0400, 0x0104, CRC(2ac654f2) SHA1(18668c73781a55dcffc4bf4c107026b0e72a75d1) )
	ROM_LOAD( "7908b-4.ic32", 0x0600, 0x0034, CRC(bddf925e) SHA1(861cf5966444d0c0392241e5cfa08db475fb439a) )
ROM_END


/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT  ROT    COMPANY                   FULLNAME                       FLAGS  */
GAME( 199?, kurukuru, 0,      kurukuru, kurukuru, driver_device, 0,    ROT0, "Success / Taiyo Jidoki", "Kuru Kuru Pyon Pyon (Japan)",  0 )
