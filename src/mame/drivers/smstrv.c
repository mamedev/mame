/*
SMS Manufacturing PCB Driver
By Reip


smstrv (Reip):
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
    D780C-1 (Z80A)

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
planes of 16384*8 pixels per plane, one plane per color channel.
Screen resolution is 54x256, if I(LN) am properly understanding the
schematics on page 6.

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
  drivers on all pins EXCEPT pins PC3 thru PC0.
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
U32         1Green      DMPAL10L8NC	Decodes the gated by U33/U34)
   high address lines of the 8088, for mainboard ROM mapping. A
   different pal is probably used depending on whether the
   mainboard has 2764 or 27128 roms installed.
   SMS Sure Shot: dumped ok as truth table, mainboard has 4 2764s
   SMS Trivia: bad (chip shorted internally), mainboard has 2 27128s
   SMS Bingo: not dumped
   (schematic page 2)
U38         3Blue		    PAL10L8CN   Decodes the (gated by U36)
   high address lines of the z80 address bus, for mapping of the z80
   ROM, RAM, Counter control, 4 z80-to-8088 ports (2 one direction,
   2 the other), the ay-3-8910, and the two 8255 PPIs.
   (schematic page 10)
U39         3Green          PAL10L8CN   Accessory decoder to U38, helps
   with the 4 z80-to-8088 ports.
   (schematic page 10)
U40         1Red	          PAL10L8CN   Connects to the low bits of the
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
U142		1Brown          PAL14H4CN
U143		1Brown          PAL14H4CN
U144		1Brown          PAL14H4CN
U145		1Brown          PAL14H4CN



*/

#include "driver.h"
#include "sound/ay8910.h"
#include "machine/8255ppi.h"

INPUT_PORTS_EXTERN(ettrivia);


static READ8_DEVICE_HANDLER( r1 )
{
	int pc = activecpu_get_pc();
	if(pc != 0x81cb)
		printf("r1 @ %X\n",activecpu_get_pc());
	return mame_rand(device->machine);
}
static READ8_DEVICE_HANDLER( r2 )
{
	int pc = activecpu_get_pc();
	if(pc != 0x81cb)
		printf("r2 @ %X\n",activecpu_get_pc());
	return mame_rand(device->machine);
}
static READ8_DEVICE_HANDLER( r3 )
{
	int pc = activecpu_get_pc();
	if(pc != 0x81cb && pc != 0x90fa && pc != 0x911b && pc != 0x90d3 && pc != 0x90c4)
		printf("r3 @ %X\n",activecpu_get_pc());
	return mame_rand(device->machine) & ~1; //with 1 jumps back (infinite loop): a status ready for something?
}
static READ8_DEVICE_HANDLER( r4 )
{
	int pc = activecpu_get_pc();
	if(pc != 0x81cb)
		printf("r4 @ %X\n",activecpu_get_pc());
	return mame_rand(device->machine);
}
static READ8_DEVICE_HANDLER( r5 )
{
	int pc = activecpu_get_pc();
	if(pc != 0x81cb)
		printf("r5 @ %X\n",activecpu_get_pc());
	return mame_rand(device->machine);
}
static READ8_DEVICE_HANDLER( r6 )
{
	int pc = activecpu_get_pc();
	if(pc != 0x81cb)
		printf("r6 @ %X\n",activecpu_get_pc());

	return mame_rand(device->machine);
}

static WRITE8_DEVICE_HANDLER( w1 )
{
	/*
    static int old = 0;
    if(data != old)
        printf("w1 = %02X\n",old=data);
        */
}
static WRITE8_DEVICE_HANDLER( w2 )
{
	/*
    static int old = 0;
    if(data != old)
        printf("w2 = %02X\n",old=data);
        */
}
static WRITE8_DEVICE_HANDLER( w3 )
{
	/*
    static int old = 0;
    if(data != old)
        printf("w3 = %02X\n",old=data);
        */
}
static WRITE8_DEVICE_HANDLER( w4 )
{
	/*
    static int old = 0;
    if(data != old)
        printf("w4 = %02X\n",old=data);
        */
}
static WRITE8_DEVICE_HANDLER( w5 )
{
	/*
    static int old = 0;
    if(data != old)
        printf("w5 = %02X\n",old=data);
        */
}
static WRITE8_DEVICE_HANDLER( w6 )
{
	/*
    static int old = 0;
    if(data != old)
        printf("w6 = %02X\n",old=data);
        */
}

static ADDRESS_MAP_START( smstrv_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x007ff) AM_RAM
	AM_RANGE(0x00800, 0x00803) AM_DEVREADWRITE(PPI8255, "ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x01000, 0x01005) AM_RAM
	AM_RANGE(0x01800, 0x01803) AM_DEVREADWRITE(PPI8255, "ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x08000, 0x0ffff) AM_ROM
	AM_RANGE(0xf8000, 0xfffff) AM_ROM // mirror for vectors
ADDRESS_MAP_END


static VIDEO_START( smstrv )
{
}

static VIDEO_UPDATE( smstrv )
{
	return 0;
}

static const ppi8255_interface ppi8255_intf[2] =
{
	{
		r1,			/* Port A read */
		r2,			/* Port B read */
		r3,			/* Port C read */
		w1,			/* Port A write */
		w2,			/* Port B write */
		w3			/* Port C write */
	},
	{
		r4,			/* Port A read */
		r5,			/* Port B read */
		r6,			/* Port C read */
		w4,			/* Port A write */
		w5,			/* Port B write */
		w6			/* Port C write */
	}
};


static MACHINE_DRIVER_START( smstrv )
	MDRV_CPU_ADD("main", I8088,24000000/2)
	MDRV_CPU_PROGRAM_MAP(smstrv_map,0)
//  MDRV_CPU_IO_MAP(io_map,0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

//  MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(256)

//  MDRV_PALETTE_INIT(naughtyb)
	MDRV_VIDEO_START(smstrv)
	MDRV_VIDEO_UPDATE(smstrv)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

ROM_START( smstrv )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD( "sms.17",       0xf8000, 0x04000, CRC(af6ef980) SHA1(f0f98d1f91de718a63b87c5f1c6ee3bd854d1c1b) )
	ROM_LOAD( "sms.16",       0xfc000, 0x04000, CRC(b827d883) SHA1(68d6c2127ef9e537471c414ca7baa89c63997bbb) )
	ROM_COPY( "main",    0xf8000, 0x08000, 0x8000 )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "sms.26",       0x0000, 0x1000, CRC(e04bb922) SHA1(1df90720f11a5b736273f43272d7727b3020f848) )

	ROM_REGION( 0x4000, "user1", 0 )
	ROM_LOAD( "sms.d1",       0x000000, 0x4000, CRC(04f627c0) SHA1(c656b66c60059a1b068c4a7262f07f4c136c34c1) )
	ROM_LOAD( "sms.d2",       0x000000, 0x4000, CRC(13c9fe08) SHA1(6b7d055621ce578446d320f98f7a4cd095e756b0) )
	ROM_LOAD( "sms.d3",       0x000000, 0x4000, CRC(8c5f62ef) SHA1(34ac235358a71620a6619dbb16255c363f34df53) )
	ROM_LOAD( "sms.d4",       0x000000, 0x4000, CRC(76993bd1) SHA1(b9a97ab7c6d35f5fdda04342e0b3773618deedef) )
	ROM_LOAD( "sms.d5",       0x000000, 0x4000, CRC(f1a37ed7) SHA1(687a610319b21091cbc53232b47eb99dabe12f02) )
	ROM_LOAD( "sms.d6",       0x000000, 0x4000, CRC(5b12fd09) SHA1(15804480e65bfb3207d24a1679bb78d1ad491d70) )
	ROM_LOAD( "sms.d7",       0x000000, 0x4000, CRC(d09946b6) SHA1(b5827945ce380f09ee758c4296f06f00ef3cbd0a) )
	ROM_LOAD( "sms.d8",       0x000000, 0x4000, CRC(80096807) SHA1(a38b1b13365577c0c588b8e196ee1a6c774ce3a3) )
	ROM_LOAD( "sms.d9",       0x000000, 0x4000, CRC(c1691ec9) SHA1(95725fa315944c0786e2a32d483703173eb2e730) )
	ROM_LOAD( "sms.d10",      0x000000, 0x4000, CRC(df0da39f) SHA1(29103dca8b0c1967791e8ddd722153874e16bbda) )
	ROM_LOAD( "sms.d11",      0x000000, 0x4000, CRC(114b4aa6) SHA1(2621d1042b0774d60be88cc8d62613aa07c12552) )
	ROM_LOAD( "sms.d12",      0x000000, 0x4000, CRC(59a40e4f) SHA1(e726ce624c76ee527edc51c1e5757b7d433dcf8c) )
	ROM_LOAD( "sms.d13",      0x000000, 0x4000, CRC(9bb8dbad) SHA1(0dd9ed23e6794a86a12906b326e984a2d58cc4c6) )
	ROM_LOAD( "sms.d14",      0x000000, 0x4000, CRC(3bfe9b52) SHA1(0cdd9ec6ed784fab9272d50821994be5b0fd0532) )
	ROM_LOAD( "sms.d15",      0x000000, 0x4000, CRC(bec225fe) SHA1(13252894eca30e06354885a21ecad43965cfd3ef) )
	ROM_LOAD( "sms.d16",      0x000000, 0x4000, CRC(b700e7e6) SHA1(42b2c12c6af5f15d909e15ee3e7ca2e13e0142c2) )





/*
    ROM_LOAD( "sms.d17",      0x000000, 0x0001f3, CRC(e9eb78e7) SHA1(688e854e82c230d367c211f611e9a8298ab64399) )
    ROM_LOAD( "sms.38",       0x000000, 0x0001f3, CRC(be17ebde) SHA1(22c05eeafeadc8f55b55951c2060fb4873146cba) )
    ROM_LOAD( "sms.39",       0x000000, 0x0001f3, CRC(3299e803) SHA1(12f361d27497f6347ee26838fa9f675f6aac12c2) )
    ROM_LOAD( "sms.40",       0x000000, 0x0001f3, CRC(22881f1c) SHA1(646fdc4e4a423e1432b448140f2d92dd2304ff71) )
    ROM_LOAD( "sms.52",       0x000000, 0x0001f3, CRC(2e43ba5f) SHA1(8b87ee8ce21f5241260f2d0de4878096d8ecb5f5) )
    ROM_LOAD( "sms.58",       0x000000, 0x0001f3, CRC(020b5108) SHA1(f3221fbce40a9d6fdc2eece606e4eded3faf5f02) )
    ROM_LOAD( "sms.80",       0x000000, 0x0001f3, CRC(66e21ee5) SHA1(31c29a250f50dcdf531810e59068adfea4d2d9a3) )
    ROM_LOAD( "sms.94",       0x000000, 0x000283, CRC(c5fda3df) SHA1(4fdd597d25ed893cb005165b68e48567fbd2b1ce) )
    ROM_LOAD( "sms.109",      0x000000, 0x000283, CRC(15d05aaa) SHA1(57500b4825a1da943d79ee7df657efed56c4320e) )
    ROM_LOAD( "sms.110",      0x000000, 0x0001f3, CRC(6263b1e1) SHA1(6c8d92bcbbc2d196b5ac7765888eaf171671d651) )
    ROM_LOAD( "sms.128",      0x000000, 0x0001f3, CRC(fbaea5b0) SHA1(85a757485c26304d4ce718fd954aa4736cdc4752) )
    ROM_LOAD( "sms.129",      0x000000, 0x0001f3, CRC(4722fb3b) SHA1(adc0a3c0721acaa5b447c7aee771703caab80dd9) )
    ROM_LOAD( "sms.130",      0x000000, 0x0001f3, CRC(d3f0a6a5) SHA1(5e08b6104dfd3e463031b2b12619589a8f7b453c) )
    ROM_LOAD( "sms.140",      0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
    ROM_LOAD( "sms.141",      0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
    ROM_LOAD( "sms.142",      0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
    ROM_LOAD( "sms.143",      0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
    ROM_LOAD( "sms.144",      0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
    ROM_LOAD( "sms.145",      0x000000, 0x000283, CRC(031f662d) SHA1(6fa072db3203cdb95262d7778a6ee8310423b3df) )
    ROM_LOAD( "sms.d0",       0x000000, 0x0001f3, CRC(b1c221a7) SHA1(f63a022199a2d7b52c4c4827b170d49aae85e4e3) )
*/
ROM_END

GAME( 1984, smstrv, 0, smstrv, ettrivia, 0, ROT0, "SMS MFG CORP", "Trivia (sms)", GAME_NOT_WORKING )
