// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
    SMS Manufacturing Corp hardware

    Driver by Mariusz Wojcieszek

    Preliminary driver by Reip
    Schematics provided by Exodus
    Sure Shot board provided by ranger_lennier
    Notes by Lord Nightmare

    Notes/ToDo:
    - pit8254 is not accessed by z80 (and there is no interrupt service routine), so
      it is not fitted in the driver
    - video registers can be read (to read framebuffer contents), this is not emulated
      as software does not use this feature
    - I8088 clock may be incorrect
    - Video Blanking Zone input - probably hblank or vblank
    - game speed may be not 100% correct - software does not use interrupts and
      Video Blanking Zone is not emulated (which is required to get proper timings)
    - implement video raw parameters

*/

/*
smssshot (Lord Nightmare)
SMS Sure Shot (poker?)
* Same exact board as sms trivia, but "COMPONENT SIDE REV 01"
  instead of rev 02.
* 3 chips were removed from the board; two are shown as removed
  in the schematic, one was removed later (an apparently
  unnecessary data buffer on one of the z80 external latches)
* Does NOT have a daughterboard with additional roms; in fact,
  connector J1 for the ribbon cable to the daughterboard isn't
  populated with pins at all
* Serial Number A-108 etched in board, on back.


smsbingo (Lord Nightmare)
SMS Bingo
Someone on the MW forums has this iirc, but it isn't dumped yet - LN



**** Notes from schematics (applies to all drivers):
Framebuffer is six tms4416 16384*4 chips; chips are arranged as three
planes of 16384*8 bits per plane, one plane per color channel.
Screen resolution is probably either 212x256 (two bits per plane, per
pixel) or 424x256 (one bit per plane per pixel), depending on the pals
installed. See schematics on page 6. The counters at U137 and U139
compare against binary 0b110101 (53) in comparitor at U138, and as soon
as it hits that, HDONE is pulled high on the next MCLR clock.
This means there will be 53 (0-52) counts in the X counter before HDONE;
as each count refers to one 8 bit address, and the addressed 4416 rams
have between all of them 24 bits of output (8 per plane), the minimum
horizontal displayed resolution is 53x256 (8 bits per color per pixel),
and maximum is 8 times that. With proper pals (and proper mixing
resistors or other PWM fun) it should be possible to get:
53x256 8,8,8 RGB
106x256 4,4,4 RGB
212x256 2,2,2 RGB <- this seems the most likely to me (LN)
424x256 1,1,1 RGB


* The socket at U50 and the 3 pin connector J3 is for an
  undumped intel 8050 MCU used for rs232 serial communication,
  for either linking together machines, or more likely for factory
  testing. The function of this internal rom is probably simple
  enough to HLE or to even rewrite from scratch, but I doubt the code
  on any of the dumped games even touches it, it was probably for use
  with a specific game or for a set of hardware test roms to report
  errors.
  (schematic page 4)
  The pinout of J3 is:
     pin 1 (toward bottom of pcb): rs232 input to pcb
     (pre-level shifted to 5v i.e. with a max232 or mc1489)
     pin 2 : ground
     pin 3 : rs232 output to elsewhere (to be sent to a max232 or
     mc1488 to shift to rs232 voltage levels)

* The 8255 PPI at U13 (connected to the 8088) is connected to 75451
  drivers on all pins EXCEPT pins PC3 through PC0.
  (schematic page 3)
  PA7 - Display Light 1
  PA6 - Display Light 2
  PA5 - Display Light 3
  PA4 - Display Light 4
  PA3 - Display Light 5
  PA2 - Bet Light
  PA1 - Deal Light
  PA0 - Draw Light
  PB7 - Stand Light
  PB6 - Cancel Light
  PB5 - Coin Lock out A
  PB4 - Coin Lock out B
  PB3 - Setup Light
  PB2 - Hopper Motor
  PB1 - Coin in Counter (mechanical counter inside the machine)
  PB0 - Knock off Counter (tilt? probably also a mechanical counter)
  PC7 - unused
  PC6 - unused
  PC5 - unused
  PC4 - Battery Charge control (for 8088 ram backup 3.6v Nicad)
  PC3 - (pulled high externally, input) - unused? possibly for an ABC hopper
  PC2 - (pulled high externally, input) - unused? possibly for an ABC hopper
  PC1 - (pulled high externally, input) - "Hopper Count",
      probably a beam to check the hopper coin out
  PC0 - "Video BZ" (Video Blanking Zone, is an input)

* The 8255 PPI at U2 (connected to the z80) is unused and not populated.
  (All 3 ports have +5V pullups on all pins)

* The 8255 PPI at U1 (connected to the z80) is used as follows:
  (All 3 ports have +5V pullups on all pins)
  PA7 - Lighted Button 1 (input)
  PA6 - Lighted Button 2 (input)
  PA5 - Lighted Button 3 (input)
  PA4 - Lighted Button 4 (input)
  PA3 - Lighted Button 5 (input)
  PA2 - Bet Button (input)
  PA1 - Deal Button (input)
  PA0 - Draw Button (input)
  PB7 - Stand Button (input)
  PB6 - Cancel Button (input)
  PB5 - Alt Coin (input)
  PB4 - Remote Knockoff (tilt? input)
  PB3 - Operator Mode (input)
  PB2 - Coin Error reset (input)
  PB1 - unused
  PB0 - unused
  PC7 - unused
  PC6 - unused
  PC5 - unused
  PC4 - unused
  PC3 - unused
  PC2 - unused
  PC1 - unused
  PC0 - Coin (input)


* The function of the pals is:
LOCATION    DOTS        TYPE        PURPOSE
U32         1Green      DMPAL10L8NC Decodes the gated by U33/U34)
   high address lines of the 8088, for mainboard ROM mapping. A
   different pal is probably used depending on whether the
   mainboard has 2764 or 27128 roms installed.
   SMS Sure Shot: dumped ok as truth table, mainboard has 4 2764s
   SMS Trivia: bad (chip shorted internally), mainboard has 2 27128s
   SMS Bingo: not dumped
   (schematic page 2)
U38         3Blue           PAL10L8CN   Decodes the (gated by U36)
   high address lines of the z80 address bus, for mapping of the z80
   ROM, RAM, Counter control, 4 z80-to-8088 ports (2 one direction,
   2 the other), the ay-3-8910, and the two 8255 PPIs.
   (schematic page 10)
U39         3Green          PAL10L8CN   Accessory decoder to U38, helps
   with the 4 z80-to-8088 ports.
   (schematic page 10)
U40         1Red              PAL10L8CN   Connects to the low bits of the
   8088 address bus for decoding writing to/reading from the 8088 side of
   the 4 z80-to-8088 ports.
   (schematic page 10, note this chip is mismarked as U9 on the page,
    it is the chip in the lower left)
U52         1Blue           PAL10L8CN   Decodes the (gated by U33/U34)
   high address lines of the 8088, for main memory mapping of ram,
   z80 communication, video, serial I/O (to U50), and the output-only
   8255 at U13 (which controls button lights and the coin hopper)
   SMS Sure Shot: dumped ok as truth table
   SMS Trivia: checksum 0, probably bad
   SMS Bingo: not dumped
   (schematic page 1)
U58         3Brown          DMPAL10H8NC Controls BDIR and BC1 on the
   ay-3-8910 given the low two address bits of the z80 bits, the
   ay-3-8910 enable line, and the buffered z80 RD and WR lines.
   (schematic page 12)
U80         2Blue           PAL10H8CN   State machine which controls StartH,
   StartV, and the related functions involving the shifters for framebuffer
   address and framebuffer output. Also lets framebuffer know when in hblank
   or vblank. Is separate from the other "Video BZ" thing.
   (schematic page 6)
U94         2Green          PAL14H4CN   State machine controls the
   read-modify-write logic for accessing the frame buffer (while outside
   of vblank and hblank?), may allow writing red green and blue plane bytes
   all to one address, one after the other
   (schematic page 7)
U109        2Brown          PAL14H4CN   Determines next state of the
   'Pixel control' hardware, i.e. H and V current line counters
   Also determines VBLANK/"Video BZ"
   (schematic page 5)
U110        2Red            PAL10L8CN   Translates output of U109
   before being sent to counters/color reg latch/etc.
   (schematic page 5)
U128        Blue-Brown-Blue PAL10H8CN   One of three 'sync' pals which
   control the memory and other timing subsystem, fed by a 4 bit counter.
   this particular pal has one external feedback bit.
   (schematic page 6)
U129        Red-Green-Red   DMPAL10H8NC Second of three 'sync' pals
   This one has 2 external feedback bits.
   (schematic page 6)
U130        3Red            PAL10H8CN   Third of three 'sync' pals'
   This one has 3 external feedback bits plus three extra inputs from elsewhere
   which are not readable on the schematic. Will trace them later.
   (schematic page 6)
U140        1Brown          PAL14H4CN   This and the next 5 pals are used
   to shift the framebuffer data, 4 bits at a time. This is done in parallel
   (8 bits per channel) for output. all 6 pals are the same.
   (schematic page 7)
U141        1Brown          PAL14H4CN
U142        1Brown          PAL14H4CN
U143        1Brown          PAL14H4CN
U144        1Brown          PAL14H4CN
U145        1Brown          PAL14H4CN


*/
#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "machine/nvram.h"


class smsmfg_state : public driver_device
{
public:
	smsmfg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen") { }

	UINT8 m_communication_port[4];
	UINT8 m_communication_port_status;
	bitmap_ind16 m_bitmap;
	UINT8 m_vid_regs[7];
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(link_r);
	DECLARE_WRITE8_MEMBER(link_w);
	DECLARE_READ8_MEMBER(z80_8088_r);
	DECLARE_READ8_MEMBER(p03_r);
	DECLARE_WRITE8_MEMBER(p03_w);
	DECLARE_WRITE8_MEMBER(video_w);
	DECLARE_READ8_MEMBER(ppi0_c_r);
	DECLARE_WRITE8_MEMBER(ppi0_a_w);
	DECLARE_WRITE8_MEMBER(ppi0_b_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_MACHINE_START(sureshot);
	UINT32 screen_update_sms(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};


/*************************************
 *
 *  Bankswitching (for trivia question roms)
 *
 *************************************/

WRITE8_MEMBER(smsmfg_state::bankswitch_w)
{
	membank("bank1")->set_entry(data);
}

/*************************************
 *
 *  8088 <-> Z80 communication
 *
 *************************************/

READ8_MEMBER(smsmfg_state::link_r)
{
	switch(offset)
	{
		case 0:
			m_communication_port_status &= ~0x01;
			return m_communication_port[0];
		case 1:
			m_communication_port_status &= ~0x02;
			return m_communication_port[1];
		case 2:
			return m_communication_port_status;
	}
	return 0;
}

WRITE8_MEMBER(smsmfg_state::link_w)
{
	switch(offset)
	{
		case 0:
			m_communication_port_status |= 0x08;
			m_communication_port[3] = data;
			break;
		case 1:
			m_communication_port_status |= 0x04;
			m_communication_port[2] = data;
			break;
	}
}

READ8_MEMBER(smsmfg_state::z80_8088_r)
{
	return m_communication_port_status;
}

READ8_MEMBER(smsmfg_state::p03_r)
{
	switch(offset)
	{
		case 0:
			m_communication_port_status &= ~0x08;
			return m_communication_port[3];
		case 1:
			m_communication_port_status &= ~0x04;
			return m_communication_port[2];
	}
	return 0;
}

WRITE8_MEMBER(smsmfg_state::p03_w)
{
	switch(offset)
	{
		case 0:
			m_communication_port_status |= 0x01;
			m_communication_port[0] = data;
			break;
		case 1:
			m_communication_port_status |= 0x02;
			m_communication_port[1] = data;
			break;
	}
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START(sms)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Draw")  /* Draw Button */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Deal")  /* Deal Button */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Bet")   /* Bet Button */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 )    /* Lighted Button 5 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )    /* Lighted Button 4 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )    /* Lighted Button 3 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    /* Lighted Button 2 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )    /* Lighted Button 1 */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Coin Error reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    /* Operator Mode */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Remote Knockoff */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Cancel")   /* Cancel Button */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Stand")     /* Stand Button */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )  PORT_IMPULSE(1) /* Coin */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/*************************************
 *
 *  8255 PPIs
 *
 *************************************/

READ8_MEMBER(smsmfg_state::ppi0_c_r)
{
/*
  PC7 - unused
  PC6 - unused
  PC5 - unused
  PC4 - Battery Charge control (for 8088 ram backup 3.6v Nicad)
  PC3 - (pulled high externally, input) - unused? possibly for an ABC hopper
  PC2 - (pulled high externally, input) - unused? possibly for an ABC hopper
  PC1 - (pulled high externally, input) - "Hopper Count",
      probably a beam to check the hopper coin out
  PC0 - "Video BZ" (Video Blanking Zone, is an input)
      it's probably vblank or hblank, 0 is always returned, games wait for this bit
      to become 0 before accesing video
*/
	return 0;
}

WRITE8_MEMBER(smsmfg_state::ppi0_a_w)
{
	//popmessage("Lamps: %d %d %d %d %d %d %d", BIT(data,7), BIT(data,6), BIT(data,5), BIT(data,4), BIT(data,3), BIT(data,2), BIT(data,1) );
	output_set_lamp_value(0, !BIT(data,7)); /* Display Light 1 */
	output_set_lamp_value(1, !BIT(data,6)); /* Display Light 2 */
	output_set_lamp_value(2, !BIT(data,5)); /* Display Light 3 */
	output_set_lamp_value(3, !BIT(data,4)); /* Display Light 4 */
	output_set_lamp_value(4, !BIT(data,3)); /* Display Light 5 */
	output_set_lamp_value(5, !BIT(data,2)); /* Bet Light */
	output_set_lamp_value(6, !BIT(data,1)); /* Deal Light */
	output_set_lamp_value(7, !BIT(data,0)); /* Draw Light */
}

WRITE8_MEMBER(smsmfg_state::ppi0_b_w)
{
	output_set_lamp_value(8, !BIT(data,7)); /* Stand Light */
	output_set_lamp_value(9, !BIT(data,6)); /* Cancel Light */

	coin_counter_w(machine(), 0, BIT(data,1));
	coin_lockout_w(machine(), 0, BIT(data,5));
	coin_lockout_w(machine(), 1, BIT(data,4));
}

/*************************************
 *
 *  Video
 *
 *************************************/

WRITE8_MEMBER(smsmfg_state::video_w)
{
	m_vid_regs[offset] = data;
	if ( offset == 5 )
	{
		int x,y;
		int xstart = m_vid_regs[0] + m_vid_regs[1]*256;
		int width = m_vid_regs[2];
		int ystart = m_vid_regs[3];
		int height = m_vid_regs[4];
		int color = m_vid_regs[5];

		if ( height == 0 )
			height = 256;

		if ( width == 0 )
			width = 256;

		for ( y = ystart; y < ystart + height; y++ )
		{
			for ( x = xstart; x < xstart + width; x++ )
			{
				if ( y < 256 )
				m_bitmap.pix16(y, x) = color;
			}
		}
	}
}

void smsmfg_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	save_item(NAME(m_vid_regs));
	save_item(NAME(m_bitmap));
}

UINT32 smsmfg_state::screen_update_sms(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( sms_map, AS_PROGRAM, 8, smsmfg_state )
	AM_RANGE(0x00000, 0x007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x00800, 0x00803) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x01000, 0x01007) AM_WRITE(video_w)
	AM_RANGE(0x01800, 0x01803) AM_READWRITE(link_r, link_w)
	AM_RANGE(0x04000, 0x07fff) AM_ROMBANK("bank1")
	AM_RANGE(0x04000, 0x04000) AM_WRITE(bankswitch_w)
	AM_RANGE(0x08000, 0x0ffff) AM_ROM
	AM_RANGE(0xf8000, 0xfffff) AM_ROM // mirror for vectors
ADDRESS_MAP_END

static ADDRESS_MAP_START( sureshot_map, AS_PROGRAM, 8, smsmfg_state )
	AM_RANGE(0x00000, 0x007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x02000, 0x02007) AM_WRITE(video_w)
	AM_RANGE(0x03000, 0x03003) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x03800, 0x03803) AM_READWRITE(link_r, link_w)
	AM_RANGE(0x08000, 0x0ffff) AM_ROM
	AM_RANGE(0xf8000, 0xfffff) AM_ROM // mirror for vectors
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8, smsmfg_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x3100, 0x3103) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x3381, 0x3382) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x3400, 0x3400) AM_READ(z80_8088_r)
	AM_RANGE(0x3500, 0x3501) AM_READWRITE(p03_r, p03_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Machine
 *
 *************************************/

void smsmfg_state::machine_start()
{
	membank("bank1")->configure_entries(0, 16, memregion("questions")->base(), 0x4000);

	save_item(NAME(m_communication_port_status));
	save_item(NAME(m_communication_port));
}

MACHINE_START_MEMBER(smsmfg_state,sureshot)
{
	save_item(NAME(m_communication_port_status));
	save_item(NAME(m_communication_port));
}

void smsmfg_state::machine_reset()
{
	m_communication_port_status = 0;
}

static MACHINE_CONFIG_START( sms, smsmfg_state )
	MCFG_CPU_ADD("maincpu", I8088, XTAL_24MHz/8)
	MCFG_CPU_PROGRAM_MAP(sms_map)

	MCFG_CPU_ADD("soundcpu", Z80, XTAL_16MHz/8)
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(smsmfg_state, ppi0_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(smsmfg_state, ppi0_b_w))
	MCFG_I8255_IN_PORTC_CB(READ8(smsmfg_state, ppi0_c_r))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x1b0, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x1af, 0, 0xff)
	MCFG_SCREEN_UPDATE_DRIVER(smsmfg_state, screen_update_sms)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_3BIT_BGR("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_16MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sureshot, sms )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sureshot_map)

	MCFG_MACHINE_START_OVERRIDE(smsmfg_state,sureshot)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
Triva looking board
Silk screened on front...
    P/N 1001
Etched in copper on font...
    COMPONENT SIDE REV 02
Etched in copper on back...
    COPYRIGHT SMS 1983 S/N      MADE IN USA
                mfg corp

Serial number A4675 etched in board

Large chips
    P8088
    AY-3-8910
    CDM6116 x2
    P8255A-5
    P8254
    D8255AC-5
    D780C-1

16 Mhz crystal by D780C (U21)
24 Mhz crystal by P8088 (u53)

Open sockets at U50 (40 pin), U15 (24 pin), U2 (40 pin), U25 (24 pin)

Bottom Board
.U17 - 27128
.U16 - 27128
.U26 - 2732 - stickered #26 073184
.U38 - DMPAL10L8NC - 3 blue dots on it - saved in jedec format
.U39 - DMPAL10L8NC - 3 green dots on it - saved in jedec format
.U40 - DMPAL10L8NC - 1 pink dot on it - saved in jedec format
.U110 - DMPAL10L8NC - 2 pink dots on it - saved in jedec format
.U52 - DMPAL10L8NC - not labeled - checksum was 0
.U32 - DMPAL10L8NC - stickered trivia U32 - couldn't read "device overcurrent"
.U58 - DMPAL10H8NC - 3 brown dots on it - saved in jedec format
.U80 - DMPAL10H8NC - 2 blue dots on it - saved in jedec format
.U130 - DMPAL10H8NC - 3 pink dots on it - saved in jedec format
.U129 - DMPAL10H8NC - pink-green-pink dots on it - saved in jedec format
.U128 - DMPAL10H8NC - blue-brown-blue dots on it - saved in jedec format
.U145 - DMPAL14H4NC - brown dot on it - saved in jedec format
.U144 - DMPAL14H4NC - brown dot on it - saved in jedec format
.U143 - DMPAL14H4NC - brown dot on it - saved in jedec format
.U142 - DMPAL14H4NC - brown dot on it - saved in jedec format
.U141 - DMPAL14H4NC - brown dot on it - saved in jedec format
.U140 - DMPAL14H4NC - brown dot on it - saved in jedec format
    U.145-U.140 had the same checksum

.U94 - DMPAL14H4NC - 2 green dots on it - saved in jedec format
.U109 - DMPAL14H4NC - 2 brown dots on it - saved in jedec format

Daughter Board
Etched in copper on top...
    SMS MFG M?I 2685        ? = a cage looking symbol

Read starting at top row, closest to connector to main board
.D0 - DMPAL10L8NC - 1 orange dot on it - saved in jedec format
.D1 - 27128 - couldn't read sticker -
.D2 - 27128 - couldn't read sticker -
.D3 - 27128 - couldn't read sticker -
.D4 - 27128 - stickered 4 MOVIES .1 ?2485   ? = can't read
.D5 - 27128 - stickered 3 ANYTHING .4 042485
.D6 - 27128 - stickered 2 ANYTHING .3 042485
.D7 - 27128 - stickered 1 ANYTHING .2 042485
.D8 - 27128 - stickered 0 ANYTHING .1 042485

2nd row - left to right
.D9 - 27128 - stickered 12 MUSIC .1 042485
.D10 - 27128 - stickered 13 MUSIC .1 042485
.D11 - 27128 - stickered 14 MUSIC .1 042485
.D12 - 27128 - stickered 15 MUSIC .1 042485

3rd row - left to right
.D13 - 27128 - stickered 11 SPORTS .4 042485
.D14 - 27128 - stickered 10 SPORTS .3 042485
.D15 - 27128 - stickered 9 SPORTS .2 042485
.D16 - 27128 - stickered 8 SPORTS .1 042485
.D17 - DMPAL10L8NC - 1 white dot on it - saved in jedec format


ROM text showed...
    COPYRIGHT 1984 SMS MFG CORP
    TRIVIA HANGUP
    SMART ALECS
*/

ROM_START( trvhang )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "sms.17",       0xf8000, 0x04000, CRC(af6ef980) SHA1(f0f98d1f91de718a63b87c5f1c6ee3bd854d1c1b) )
	ROM_LOAD( "sms.16",       0xfc000, 0x04000, CRC(b827d883) SHA1(68d6c2127ef9e537471c414ca7baa89c63997bbb) )
	ROM_COPY( "maincpu",    0xf8000, 0x08000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sms.26",       0x0000, 0x1000, CRC(e04bb922) SHA1(1df90720f11a5b736273f43272d7727b3020f848) )
	ROM_RELOAD(               0x1000, 0x1000 )

	ROM_REGION( 0x40000, "questions", 0 )
	// anything
	ROM_LOAD( "0anything1.d8", 0x00000, 0x4000, CRC(80096807) SHA1(a38b1b13365577c0c588b8e196ee1a6c774ce3a3) )
	ROM_LOAD( "1anything2.d7", 0x04000, 0x4000, CRC(d09946b6) SHA1(b5827945ce380f09ee758c4296f06f00ef3cbd0a) )
	ROM_LOAD( "2anything3.d6", 0x08000, 0x4000, CRC(5b12fd09) SHA1(15804480e65bfb3207d24a1679bb78d1ad491d70) )
	ROM_LOAD( "3anything4.d5", 0x0c000, 0x4000, CRC(f1a37ed7) SHA1(687a610319b21091cbc53232b47eb99dabe12f02) )
	// movies
	ROM_LOAD( "4movies1.d4",   0x10000, 0x4000, CRC(76993bd1) SHA1(b9a97ab7c6d35f5fdda04342e0b3773618deedef) )
	ROM_LOAD( "5movies1.d3",   0x14000, 0x4000, CRC(8c5f62ef) SHA1(34ac235358a71620a6619dbb16255c363f34df53) )
	ROM_LOAD( "6movies1.d2",   0x18000, 0x4000, CRC(13c9fe08) SHA1(6b7d055621ce578446d320f98f7a4cd095e756b0) )
	ROM_LOAD( "7movies1.d1",   0x1c000, 0x4000, CRC(04f627c0) SHA1(c656b66c60059a1b068c4a7262f07f4c136c34c1) )
	// sports
	ROM_LOAD( "8sports1.d16",  0x20000, 0x4000, CRC(b700e7e6) SHA1(42b2c12c6af5f15d909e15ee3e7ca2e13e0142c2) )
	ROM_LOAD( "9sports2.d15",  0x24000, 0x4000, CRC(bec225fe) SHA1(13252894eca30e06354885a21ecad43965cfd3ef) )
	ROM_LOAD( "10sports3.d14", 0x28000, 0x4000, CRC(3bfe9b52) SHA1(0cdd9ec6ed784fab9272d50821994be5b0fd0532) )
	ROM_LOAD( "11sports4.d13", 0x2c000, 0x4000, CRC(9bb8dbad) SHA1(0dd9ed23e6794a86a12906b326e984a2d58cc4c6) )
	// music
	ROM_LOAD( "12music1.d9",   0x30000, 0x4000, CRC(c1691ec9) SHA1(95725fa315944c0786e2a32d483703173eb2e730) )
	ROM_LOAD( "13music1.d10",  0x34000, 0x4000, CRC(df0da39f) SHA1(29103dca8b0c1967791e8ddd722153874e16bbda) )
	ROM_LOAD( "14music1.d11",  0x38000, 0x4000, CRC(114b4aa6) SHA1(2621d1042b0774d60be88cc8d62613aa07c12552) )
	ROM_LOAD( "15music1.d12",  0x3c000, 0x4000, CRC(59a40e4f) SHA1(e726ce624c76ee527edc51c1e5757b7d433dcf8c) )


	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "dmpal10l8nc.d17.bin", 0x000000, 0x0001f3, CRC(e9eb78e7) SHA1(688e854e82c230d367c211f611e9a8298ab64399) )
	ROM_LOAD( "dmpal10l8nc.38.bin",  0x000000, 0x0001f3, CRC(be17ebde) SHA1(22c05eeafeadc8f55b55951c2060fb4873146cba) )
	ROM_LOAD( "dmpal10l8nc.39.bin",  0x000000, 0x0001f3, CRC(3299e803) SHA1(12f361d27497f6347ee26838fa9f675f6aac12c2) )
	ROM_LOAD( "dmpal10l8nc.40.bin",  0x000000, 0x0001f3, CRC(22881f1c) SHA1(646fdc4e4a423e1432b448140f2d92dd2304ff71) )
	ROM_LOAD( "dmpal10l8nc.52.bin",  0x000000, 0x0001f3, CRC(2e43ba5f) SHA1(8b87ee8ce21f5241260f2d0de4878096d8ecb5f5) )
	ROM_LOAD( "dmpal10h8nc.58.bin",  0x000000, 0x0001f3, CRC(020b5108) SHA1(f3221fbce40a9d6fdc2eece606e4eded3faf5f02) )
	ROM_LOAD( "dmpal10h8nc.80.bin",  0x000000, 0x0001f3, CRC(66e21ee5) SHA1(31c29a250f50dcdf531810e59068adfea4d2d9a3) )
	ROM_LOAD( "dmpal14h4nc.94.bin",  0x000000, 0x000283, CRC(c5fda3df) SHA1(4fdd597d25ed893cb005165b68e48567fbd2b1ce) )
	ROM_LOAD( "dmpal14h4nc.109.bin", 0x000000, 0x000283, CRC(15d05aaa) SHA1(57500b4825a1da943d79ee7df657efed56c4320e) )
	ROM_LOAD( "dmpal10l8nc.110.bin", 0x000000, 0x0001f3, CRC(6263b1e1) SHA1(6c8d92bcbbc2d196b5ac7765888eaf171671d651) )
	ROM_LOAD( "dmpal10h8nc.128.bin", 0x000000, 0x0001f3, CRC(fbaea5b0) SHA1(85a757485c26304d4ce718fd954aa4736cdc4752) )
	ROM_LOAD( "dmpal10h8nc.129.bin", 0x000000, 0x0001f3, CRC(4722fb3b) SHA1(adc0a3c0721acaa5b447c7aee771703caab80dd9) )
	ROM_LOAD( "dmpal10h8nc.130.bin", 0x000000, 0x0001f3, CRC(d3f0a6a5) SHA1(5e08b6104dfd3e463031b2b12619589a8f7b453c) )
	ROM_LOAD( "dmpal14h4nc.140.bin", 0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
	ROM_LOAD( "dmpal14h4nc.141.bin", 0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
	ROM_LOAD( "dmpal14h4nc.142.bin", 0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
	ROM_LOAD( "dmpal14h4nc.143.bin", 0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
	ROM_LOAD( "dmpal14h4nc.144.bin", 0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
	ROM_LOAD( "dmpal14h4nc.145.bin", 0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
	ROM_LOAD( "dmpal10l8nc.d0.bin",  0x000000, 0x0001f3, CRC(b1c221a7) SHA1(f63a022199a2d7b52c4c4827b170d49aae85e4e3) )
ROM_END

/*
Etched in copper on back    COPYRIGHT SMS 1983
                    mfg corp. S/N   A-2043
                            A-2043 was hand etched

16MHz Crystal
24MHz Crystal
D780C
D8255AC-5
P8255A-5
P8088
AY-3-8910
6116    x2
P8254

Empty 40 pin socket at U2 and U50
Empty 24 pin socket at U25
Empty 28 pin socket at U18



.u16    28128   stickered   U-16
                TRIVIA -2
                011586

.u17    28128   stickered   U-16
                TRIVIA -2
                011586

.u19    28128   stickered   U-16
                TRIVIA DLXE
                021281

.u26    2732    stickered   #26
                073184


the pal's had colored dots on them
saved in JEDEC format
.32 dmpal10l8nc red red blue
.52 dmpal10l8nc red red white
.38 dmpal10l8nc blue    blue    blue
.39 dmpal10l8nc green   green   green
.40 dmpal10l8nc pink
.110    dmpal10l8nc pink    pink
.58 dmpal10h8nc brown   brown   brown
.80 dmpal10h8nc blue    blue
.128    dmpal10h8nc blue    brown   blue
.129    dmpal10h8nc pink    green   pink
.130    dmpal10h8nc pink    scratched off
.94 dmpal14h4nc green   green
.109    dmpal14h4nc brown   brown
.140    dmpal14h4nc brown
.141    dmpal14h4nc brown
.142    dmpal14h4nc brown
.143    dmpal14h4nc brown
.144    dmpal14h4nc brown
.145    dmpal14h4nc brown
*/

ROM_START( trvhanga )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "sms2.u17",   0xf8000, 0x04000, CRC(e5c880a1) SHA1(da777c4e126da2f03a663f8c8f565bda8520c883) )
	ROM_LOAD( "sms2.16",    0xfc000, 0x04000, CRC(85484aee) SHA1(7c282bd208bd644d5d57ac399942c95211e87bf4) )
	ROM_COPY( "maincpu",    0xf8000, 0x08000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sms2.26",       0x0000, 0x1000, CRC(e04bb922) SHA1(1df90720f11a5b736273f43272d7727b3020f848) )
	ROM_RELOAD(               0x1000, 0x1000 )

	ROM_REGION( 0x4000, "user", 0 )
	ROM_LOAD( "sms2.u19",    0x00000, 0x04000, CRC(5e10059b) SHA1(f0eb490d41009ffe1c80216f699557480110954b) ) // unknown rom (leftover from conversion?)

	ROM_REGION( 0x40000, "questions", 0 )
	// sex
	ROM_LOAD( "0sex1.d8", 0x00000, 0x4000, NO_DUMP )
	ROM_LOAD( "1sex2.d7", 0x04000, 0x4000, NO_DUMP )
	ROM_LOAD( "2sex3.d6", 0x08000, 0x4000, NO_DUMP )
	ROM_LOAD( "3sex4.d5", 0x0c000, 0x4000, NO_DUMP )
	// movies
	ROM_LOAD( "4movies1.d4",   0x10000, 0x4000, NO_DUMP )
	ROM_LOAD( "5movies1.d3",   0x14000, 0x4000, NO_DUMP )
	ROM_LOAD( "6movies1.d2",   0x18000, 0x4000, NO_DUMP )
	ROM_LOAD( "7movies1.d1",   0x1c000, 0x4000, NO_DUMP )
	// sports
	ROM_LOAD( "8sports1.d16",  0x20000, 0x4000, NO_DUMP )
	ROM_LOAD( "9sports2.d15",  0x24000, 0x4000, NO_DUMP )
	ROM_LOAD( "10sports3.d14", 0x28000, 0x4000, NO_DUMP )
	ROM_LOAD( "11sports4.d13", 0x2c000, 0x4000, NO_DUMP )
	// music
	ROM_LOAD( "12music1.d9",   0x30000, 0x4000, NO_DUMP )
	ROM_LOAD( "13music1.d10",  0x34000, 0x4000, NO_DUMP )
	ROM_LOAD( "14music1.d11",  0x38000, 0x4000, NO_DUMP )
	ROM_LOAD( "15music1.d12",  0x3c000, 0x4000, NO_DUMP )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "dmpal10l8nc.32.bin",  0x000000, 0x00002c, CRC(558ca47e) SHA1(4612e5dd9832bcbf6d7b3bce803f92ea2ee03b98) )
	ROM_LOAD( "dmpal10l8nc.38.bin",  0x000000, 0x00002c, CRC(84c39631) SHA1(8aa28be5418dd119883a9d400f23927e0cf8d7b4) )
	ROM_LOAD( "dmpal10l8nc.39.bin",  0x000000, 0x00002c, CRC(ab950780) SHA1(73e8eb8050ca337e58b34513a9cf522e4fb43b5d) )
	ROM_LOAD( "dmpal10l8nc.40.bin",  0x000000, 0x00002c, CRC(a2de5f30) SHA1(ea2293f6b979aa16ccf1c8d6c88ea484ef45ea6b) )
	ROM_LOAD( "dmpal10l8nc.52.bin",  0x000000, 0x00002c, CRC(5d09ff76) SHA1(0b977db9dc465a36975b935d3585f404b86de293) )
	ROM_LOAD( "dmpal10h8nc.58.bin",  0x000000, 0x00002c, CRC(3df46d79) SHA1(606040b69908635ab5166193c53557ffa524c9d3) )
	ROM_LOAD( "dmpal10h8nc.80.bin",  0x000000, 0x00002c, CRC(7bd4fbf5) SHA1(a2951bcf6af2a6d0211070c2dc49b8c1e9b78b9b) )
	ROM_LOAD( "dmpal14h4nc.94.bin",  0x000000, 0x00003c, CRC(43a4e3f1) SHA1(afc530cc52ac04abce33b7c61e256da1cb30eb23) )
	ROM_LOAD( "dmpal14h4nc.109.bin", 0x000000, 0x00003c, CRC(98542c78) SHA1(282d06701da1130d9ca0fcbd81ea89f1462693e2) )
	ROM_LOAD( "dmpal10l8nc.110.bin", 0x000000, 0x00002c, CRC(f59c1868) SHA1(cc75ce71057360e62ea3ea929f241dc6105f1362) )
	ROM_LOAD( "dmpal10h8nc.128.bin", 0x000000, 0x00002c, CRC(083cd773) SHA1(70a49a53d549b90c7a036f4afaf92759bf8e20c9) )
	ROM_LOAD( "dmpal10h8nc.129.bin", 0x000000, 0x00002c, CRC(c5841a1a) SHA1(f2929321b041114f771e9fba4cbe2fb36c26a053) )
	ROM_LOAD( "dmpal10h8nc.130.bin", 0x000000, 0x00002c, CRC(a4051372) SHA1(23b200f3950e583cc40b3cc82d62e274294e5593) )
	ROM_LOAD( "dmpal14h4nc.140.bin", 0x000000, 0x00003c, CRC(c921d183) SHA1(db7be592058456e83a8603cf839f5664ea0a0f76) )
	ROM_LOAD( "dmpal14h4nc.141.bin", 0x000000, 0x00003c, CRC(c921d183) SHA1(db7be592058456e83a8603cf839f5664ea0a0f76) )
	ROM_LOAD( "dmpal14h4nc.142.bin", 0x000000, 0x00003c, CRC(c921d183) SHA1(db7be592058456e83a8603cf839f5664ea0a0f76) )
	ROM_LOAD( "dmpal14h4nc.143.bin", 0x000000, 0x00003c, CRC(c921d183) SHA1(db7be592058456e83a8603cf839f5664ea0a0f76) )
	ROM_LOAD( "dmpal14h4nc.144.bin", 0x000000, 0x00003c, CRC(c921d183) SHA1(db7be592058456e83a8603cf839f5664ea0a0f76) )
	ROM_LOAD( "dmpal14h4nc.145.bin", 0x000000, 0x00003c, CRC(ab2af8de) SHA1(775495d47435c23eecf3defba15f5ca890836354) )
ROM_END

ROM_START( sureshot )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "u-19 hldly s.shot 020687.u19.a12.bin", 0xf8000, 0x02000, CRC(028bdb61) SHA1(e39c27cc6dec12de5a5e60d544f35448e49baee1) )
	ROM_LOAD( "u-18 hldly s.shot 020687.u18.a11.bin", 0xfa000, 0x02000, CRC(5aa083f1) SHA1(3eed1a7421e7abcc41a1bddf655b1e777d843898) )
	ROM_LOAD( "u-17 hldly s.shot 020687.u17.a10.bin", 0xfc000, 0x02000, CRC(a37432d6) SHA1(398462642ab0b34efdb6ff4756758057b9833e10) )
	ROM_LOAD( "u-16 hldly s.shot 020687.u16.a9.bin",  0xfe000, 0x02000, CRC(d7f756d5) SHA1(5c7f62b02b4d4836881c3da0604448c34ede674b) )
	ROM_COPY( "maincpu",    0xf8000, 0x08000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "26 404 073184.u26.b5.bin", 0x0000, 0x1000, CRC(e04bb922) SHA1(1df90720f11a5b736273f43272d7727b3020f848) )
	ROM_RELOAD(               0x1000, 0x1000 )
ROM_END

/*
Etched in copper on back    COPYRIGHT SMS 1983
                mfg corp
                S/N A-872       A-872 was etched
                MADE IN USA

Etched in copper on front   REV 03

Silkscreened on top     P/N 1001


.16 2764        handwritten sticker U16
.17 2764        handwritten sticker U17
.18 2764        handwritten sticker U18
.19 2764        handwritten sticker U19
.26 2732        handwritten sticker #26
.32 pal10l8     green dot sticker with 32 written on it
.52 pal10l8     blue dot sticker with 52 written on it
.58 pal10l8     3 blue dot stickers with 58 written on one
.40 pal10l8     red dot sticker with 40 written on it
.39 pal10l8     3 green dot stickers with 39 written on one
.38 pal10l8     3 blue dot stickers with 38 written on one
.80 pal10l8     2 blue dot stickers with 80 written on one
.94 pal14h4     2 green dot stickers with 94 written on one - was getting different values for each read
.109    pal14h4     2 brown dot stickers with 109 written on one
.110    pal10l8     2 red dot stickers with 110 written on one
.128    pal10h8     1 blue, 1 brown, and another blue dot sticker with 128 written on the first blue one
.129    pal10h8     1 green and 1 red dot sticker
.130    pal10h8     3 red dot stickers with 130 written on one
.140    pal14h4     1 brown sticker with 140 written on it
.141    pal14h4     1 brown sticker with 141 written on it
.142    pal14h4     1 brown sticker with 142 written on it
.143    pal14h4     1 brown sticker with 143 written on it
.144    pal14h4     1 brown sticker with 144 written on it
.145    pal14h4     1 brown sticker with 145 written on it
*/

ROM_START( secondch )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "u19.19", 0xf8000, 0x02000, CRC(5ab3d30f) SHA1(16120c6d6a9d494c12f6609e5cb1311a4b40672b) )
	ROM_LOAD( "u18.18", 0xfa000, 0x02000, CRC(941a1401) SHA1(92934d40bb256e18b996582c1af253d06732462f) )
	ROM_LOAD( "u17.17", 0xfc000, 0x02000, CRC(88717e9f) SHA1(01b78f3ddd78e74e799d5f8ffe2f3cbcf5e6b7a2) )
	ROM_LOAD( "u16.16", 0xfe000, 0x02000, CRC(6c9a0224) SHA1(01152024b48461c3b9ac63a9265129dabacd0462) )
	ROM_COPY( "maincpu",  0xf8000, 0x08000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "#26.26", 0x0000, 0x1000, CRC(e04bb922) SHA1(1df90720f11a5b736273f43272d7727b3020f848) )
	ROM_RELOAD(           0x1000, 0x1000 )
ROM_END

GAME( 1984, trvhang,  0, sms,      sms, driver_device, 0, ROT0, "SMS Manufacturing Corp.", "Trivia Hangup (question set 1)", MACHINE_SUPPORTS_SAVE ) /* Version Trivia-1-050185 */
GAME( 1984, trvhanga, 0, sms,      sms, driver_device, 0, ROT0, "SMS Manufacturing Corp.", "Trivia Hangup (question set 2)", MACHINE_NOT_WORKING ) /* Version Trivia-2-011586 */
GAME( 1985, sureshot, 0, sureshot, sms, driver_device, 0, ROT0, "SMS Manufacturing Corp.", "Sure Shot", MACHINE_SUPPORTS_SAVE )
GAME( 1985, secondch, 0, sureshot, sms, driver_device, 0, ROT0, "SMS Manufacturing Corp.", "Second Chance", MACHINE_SUPPORTS_SAVE )
