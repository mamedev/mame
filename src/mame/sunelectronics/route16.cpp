// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

Driver by Zsolt Vasvari

Notes
-----

Route 16:
        - Route 16 doesn't have the SN76477 chip. There is space on the PCB
          but it is not populated.

        - Has the added ability to turn off each bitplane individually.
          This looks like an afterthought, as one of the same bits that control
          the palette selection is doubly utilized as the bitmap enable bit.

        - New code to better emulate the protection in Route 16 was added in 0.194,
          but it turned out to harbour a bug (see MT 07310). Therefore the previous
          patches have been restored, and the protection routine has been nullified
          but is still there in case someone wants to revisit it.

Stratovox:
        - Has almost *electrically* identical hardware to Route 16 with the exception
          that it is physically different (2 PCB-set connected with flat cables) and
          Stratovox has the SN76477 chip and uses a DAC for voice. There are 3 volume
          pots on the PCB. One for music, one for speech and a master volume.

Space Echo:
        - When all astronauts are taken the game over tune ends with 5 bad notes,
          this appears to be a bug in the ROM from a changed instruction at 2EB3.

        - Service mode shows a garbled screen as most of the code for it has been
          replaced by other routines, however the sound tests still work. it's
          possible that the service switch isn't connected on the real hardware.

        - The game hangs if it doesn't pass the startup test, a best guess is implemented
          rather than patching out the test. code for the same test is in stratvox but
          isn't called, speakres has a very similar test but doesn't care about the result.

        - Interrupts per frame for cpu1 is a best guess based on how stratvox uses the DAC,
          writing up to 195 times per frame with each byte from the ROM written 4 times.
          spacecho writes one byte per interrupt so 195/4 or 48 is used. a lower number
          increases the chance of a sound interrupting itself, which for most sounds
          is buggy and causes the game to freeze until the first sound completes.

vscompmj:
        - Stuck notes (constant tone) in-game after the mahjong tiles are laid down.


Route 16/Stratovox memory map (preliminary)

 CPU1

 0000-2fff ROM
 4000-43ff Shared RAM
 8000-bfff Video RAM

 I/O Read

 48xx IN0 - DIP Switches
 50xx IN1 - Input Port 1
 58xx IN2 - Input Port 2
 60xx IN3 - Unknown (Speak & Rescue/Space Echo only)

 I/O Write

 48xx OUT0 - D0-D4 color select for VRAM 0
             D5    coin counter
 50xx OUT1 - D0-D4 color select for VRAM 1
             D5    VIDEO I/II (Flip Screen)
 58xx OUT2 - Unknown (Speak & Rescue/Space Echo only)

 I/O Port Write

 6800 AY-8910 Write Port
 6900 AY-8910 Control Port


 CPU2

 0000-1fff ROM
 4000-43ff Shared RAM
 8000-bfff Video RAM

 I/O Write

 2800      DAC output (Stratovox only)

***************************************************************************

Route 16 PCB Hardware Info by Guru

TVX-2 (TVX-3 also seen)
SUN ELECTRONICS CORPORATION
|------------------------------------|-----------|
| Z80             54  55  56  57  58 |59 SKT     |
|                                    |         X |
|                                    | MB8841  X | <--- Sub board on top containing 4 logic
|                                    |         X |      chips and an MB8841 microcontroller
| 10MHz                              |         X |      PCB Number: TVX-S1
|                                    |         X |
|  ^SN76477      MB7052.61           |---------X-|
|#VRS  AY-3-8910                               X |
|VOL                                           X |
|  MB3713        MB7052.59                       |
|1                                               |
|8                                             X |
|W                    MB8114                   X |
|A    DSW                                      X |
|Y                    MB8114                   X |
|   %555 %555                                  X |
|#VR2 %555 %555                                X |
|#VR1                                          X |
|#VR0   Z80        60  61  62  63              X |
|------------------------------------------------|
Notes:
      X         - Texas Instruments TMS4116 16k x1-bit DRAM (total 16 chips)
      MB8114    - Fujitsu MB8114 1k x4-bit SRAM, compatible with 2114
      MB7052    - Fujitsu MB7052 256b x4-bit BiPolar PROM (compatible with 82S129)
      AY-3-8910 - General Instrument AY-3-8910 Programmable Sound Generator (PSG). Clock 1.25MHz [10/8]
      Z80       - Clock 2.5MHz [10/4] (both)
      ^         - SN76477 not populated on Route 16 PCB
      %         - These 4x 555 Timer ICs are not populated on Route 16 PCB
      #         - These 4x Volume pots are not populated on Route 16 PCB
      MB3713    - Fujitsu MB3713 5.7W Mono Power AMP
      DSW       - 8-position DIP Switch
      54...63   - 2716 EPROM
      SKT       - 24-pin Socket for daughterboard connection
      MB8841    - Fujitsu MB8841 4-bit microcontroller containing 2k x8-bit mask ROM and 128b x4-bit static RAM

18-WAY PCB Edge Connector Pinout
----------------+----------------
    PARTS SIDE  |  SOLDER SIDE
----------------+----------------
       +5V | 1A | 1B | -5V
       +5V | 2A | 2B | -5V
 PL2 Right | 3A | 3B | PL1 Right
  PL2 Left | 4A | 4B | PL1 Left
    PL2 Up | 5A | 5B | PL1 Up
  PL2 Down | 6A | 6B | PL1 Down
PL2 Button | 7A | 7B | PL1 Button
           | 8A | 8B |
 PL2 Start | 9A | 9B | Test
 PL1 Start |10A | 10B| Coin
           |11A | 11B|
     Green |12A | 12B| Coin Counter
       Red |13A | 13B|
      Blue |14A | 14B|
      Sync |15A | 15B| Speaker +
       GND |16A | 16B| Speaker -
       GND |17A | 17B| +12V
       GND |18A | 18B| +12V

***************************************************************************/

#include "emu.h"
#include "route16.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/sn76477.h"

#include "speaker.h"

class speakres_state : public route16_state
{
public:
	speakres_state(const machine_config &mconfig, device_type type, const char *tag)
		: route16_state(mconfig, type, tag)
		, m_sn(*this, "snsnd")
		, m_dac(*this, "dac")
	{}

	void speakres(machine_config &config);
	void stratvox(machine_config &config);
	void spacecho(machine_config &config);

private:
	uint8_t speakres_in3_r();
	void speakres_out2_w(uint8_t data);
	void stratvox_sn76477_w(uint8_t data);
	void stratvox_dac_w(uint8_t data);
	DECLARE_MACHINE_START(speakres);

	void speakres_cpu1_map(address_map &map) ATTR_COLD;
	void stratvox_cpu1_map(address_map &map) ATTR_COLD;
	void stratvox_cpu2_map(address_map &map) ATTR_COLD;

	required_device<sn76477_device> m_sn;
	required_device<dac_byte_interface> m_dac;

	int m_speakres_vrx = 0;
};


/*************************************
 *
 *  Drivers specific initialization
 *
 *************************************/

MACHINE_START_MEMBER(speakres_state, speakres)
{
	save_item(NAME(m_speakres_vrx));
}

MACHINE_START_MEMBER(route16_state, jongpute)
{
	save_item(NAME(m_jongpute_port_select));
}

void route16_state::init_route16a()
{
	// hack out the protection
	u8 *rom = memregion("cpu1")->base();
	rom[0x105] = 0; // remove jp nz,4109
	rom[0x106] = 0;
	rom[0x107] = 0;

	rom[0x72a] = 0; // remove jp nz,4238
	rom[0x72b] = 0;
	rom[0x72c] = 0;
	init_route16c();
}

void route16_state::init_route16()
{
	save_item(NAME(m_protection_data));
	// hack out the protection
	u8 *rom = memregion("cpu1")->base();
	rom[0x105] = 0; // remove jp nz,4109
	rom[0x106] = 0;
	rom[0x107] = 0;

	rom[0x731] = 0; // remove jp nz,4238
	rom[0x732] = 0;
	rom[0x733] = 0;

	rom[0x0e9] = 0x3a; // remove call 2CCD

	rom[0x747] = 0xc3; // skip protection checking
	rom[0x748] = 0x56;
	rom[0x749] = 0x07;
}

void route16_state::init_route16c()
{
	save_item(NAME(m_protection_data));
	// hack out the protection
	u8 *rom = memregion("cpu1")->base();
	rom[0x0e9] = 0x3a; // remove call 2CD8

	rom[0x754] = 0xc3; // skip protection checking
	rom[0x755] = 0x63;
	rom[0x756] = 0x07;
}

void route16_state::init_route16d()
{
	save_item(NAME(m_protection_data));
	// hack out the protection
	u8 *rom = memregion("cpu1")->base();

	rom[0x0e9] = 0x3a; // remove call 2CCD

	rom[0x105] = 0; // remove jp nz,4109
	rom[0x106] = 0;
	rom[0x107] = 0;

	rom[0x735] = 0; // remove jp nz,4238
	rom[0x736] = 0;
	rom[0x737] = 0;

	rom[0x74b] = 0xc3; // skip protection checking
	rom[0x74c] = 0x5a;
	rom[0x74d] = 0x07;
}

void route16_state::init_vscompmj() // only opcodes encrypted
{
	uint8_t *rom = memregion("cpu1")->base();

	uint8_t unk0 = 0x00;
	uint8_t unk1 = 0x00;

	static const uint8_t xor_table_00[0x08][0x08] =
	{
		{ 0x04, 0x01, 0x14, 0x14, 0x05, 0x10, 0x54, 0x05 }, // 0x0x and 0x2x
		{ 0x15, 0x51, 0x01, 0x44, 0x50, 0x44, 0x11, 0x50 }, // 0x1x and 0x3x
		{ 0x14, 0x50, 0x41, 0x15, 0x50, 0x15, 0x15, 0x41 }, // 0x4x and 0x6x
		{ 0x11, 0x04, 0x40, 0x11, 0x11, 0x45, 0x10, 0x14 }, // 0x5x and 0x7x
		{ 0x40, unk0, 0x15, unk0, 0x01, 0x44, 0x14, 0x54 }, // 0x8x and 0xax
		{ 0x11, 0x40, unk0, unk0, 0x14, 0x01, 0x54, 0x51 }, // 0x9x and 0xbx
		{ 0x05, 0x45, 0x10, 0x55, 0x51, 0x15, 0x55, 0x11 }, // 0xcx and 0xex
		{ unk0, 0x41, 0x51, 0x10, 0x01, 0x44, 0x50, 0x50 }, // 0xdx and 0xfx
	};

	static const uint8_t xor_table_01[0x08][0x08] =
	{
		{ 0x41, 0x41, 0x45, 0x54, 0x44, 0x40, 0x55, 0x41 }, // 0x0x and 0x2x
		{ 0x14, 0x14, 0x04, 0x45, 0x44, 0x01, 0x05, 0x05 }, // 0x1x and 0x3x
		{ 0x40, 0x14, 0x01, 0x11, 0x45, 0x14, 0x04, 0x50 }, // 0x4x and 0x6x
		{ 0x04, 0x40, 0x55, 0x55, 0x44, 0x40, 0x55, 0x55 }, // 0x5x and 0x7x
		{ 0x01, 0x05, 0x14, 0x10, 0x01, unk1, 0x04, 0x04 }, // 0x8x and 0xax
		{ 0x10, 0x04, 0x51, 0x01, 0x04, 0x04, 0x45, 0x51 }, // 0x9x and 0xbx
		{ 0x11, 0x01, 0x44, 0x44, 0x05, 0x15, 0x10, 0x05 }, // 0xcx and 0xex
		{ unk1, 0x14, 0x05, unk1, 0x01, 0x41, 0x04, 0x40 }, // 0xdx and 0xfx
	};

	for (int i = 0; i < 0x8000; i++)
	{
		uint8_t x = rom[i];

		uint8_t row = (BIT(x, 4) +  (BIT(x, 6) << 1) + (BIT(x, 7) << 2));

		uint8_t xor_v = x & 0x07;

		switch(i & 0x01)
		{
			case 0x00: x ^= xor_table_00[row][xor_v]; break;
			case 0x01: x ^= xor_table_01[row][xor_v]; break;
		}

		m_decrypted_opcodes[i] = x;
	}
}

/*************************************
 *
 *  Shared RAM handling
 *
 *************************************/

template<bool cpu1> void route16_state::route16_sharedram_w(offs_t offset, uint8_t data)
{
	m_sharedram[offset] = data;

	// 4313-4319 are used in Route 16 as triggers to wake the other CPU
	if (offset >= 0x0313 && offset <= 0x0319 && data == 0xff)
	{
		// Let the other CPU run
		(cpu1 ? m_cpu1 : m_cpu2)->yield();
	}
}



/*************************************
 *
 *  Protection handling
 *
 *************************************/

uint8_t route16_state::routex_prot_read()
{
	if (m_cpu1->pc() == 0x2f) return 0xfb;

	logerror ("cpu '%s' (PC=%08X): unmapped prot read\n", m_cpu1->tag(), m_cpu1->pc());
	return 0x00;
}

// never called, see notes.
uint8_t route16_state::route16_prot_read()
{
	m_protection_data++;
	return (1 << ((m_protection_data >> 1) & 7));
}



/*************************************
 *
 *  Stratovox's extra sound effects
 *
 *************************************/

void speakres_state::stratvox_sn76477_w(uint8_t data)
{
	/***************************************************************
	 * AY8910 output bits are connected to...
	 * 7    - direct: 5V * 30k/(100+30k) = 1.15V - via DAC??
	 * 6    - SN76477 mixer C
	 * 5    - SN76477 mixer B
	 * 4    - SN76477 mixer A
	 * 3    - SN76477 envelope 2
	 * 2    - SN76477 envelope 1
	 * 1    - SN76477 vco
	 * 0    - SN76477 enable
	 ***************************************************************/
	m_sn->enable_w((data >> 0) & 1);
	m_sn->vco_w((data >> 1) & 1);
	m_sn->envelope_1_w((data >> 2) & 1);
	m_sn->envelope_2_w((data >> 3) & 1);
	m_sn->mixer_a_w((data >> 4) & 1);
	m_sn->mixer_b_w((data >> 5) & 1);
	m_sn->mixer_c_w((data >> 6) & 1);
}

void speakres_state::stratvox_dac_w(uint8_t data)
{
	// Data is written into a pair of MC14175B quad D flip-flops with complementary outputs.
	// Schematics indicate an inverting output is tapped for the most significant bit (F7 pin 3).
	m_dac->data_w(data ^ 0x80);
}



/***************************************************
 *
 *  Jongputer and T.T Mahjong's multiplexed ports
 *
 ***************************************************/

void route16_state::jongpute_input_port_matrix_w(uint8_t data)
{
	m_jongpute_port_select = data;
}


uint8_t route16_state::jongpute_p1_matrix_r()
{
	uint8_t ret = 0;

	switch (m_jongpute_port_select)
	{
	case 1:  ret = m_key[0]->read(); break;
	case 2:  ret = m_key[1]->read(); break;
	case 4:  ret = m_key[2]->read(); break;
	case 8:  ret = m_key[3]->read(); break;
	default: break;
	}

	return ret;
}

uint8_t route16_state::jongpute_p2_matrix_r()
{
	uint8_t ret = 0;

	switch (m_jongpute_port_select)
	{
	case 1:  ret = m_key[4]->read(); break;
	case 2:  ret = m_key[5]->read(); break;
	case 4:  ret = m_key[6]->read(); break;
	case 8:  ret = m_key[7]->read(); break;
	default: break;
	}

	return ret;
}


/***************************************************************************
  guessing that the unconnected IN3 and OUT2 on the stratvox schematic
  are hooked up for speakres and spacecho to somehow read the variable
  resistors (eg a voltage ramp), using a write to OUT2 as a trigger
  and then bits 0-2 of IN3 going low when each pot "matches". the VRx
  values can be seen when IN0=0x55 and p1b1 is held during power on.
  this would then be checking that the sounds are mixed correctly.
***************************************************************************/

uint8_t speakres_state::speakres_in3_r()
{
	int bit2=4, bit1=2, bit0=1;

	/* just using a counter, the constants are the number of reads
	   before going low, each read is 40 cycles apart. the constants
	   were chosen based on the startup tests and for vr0=vr2 */
	m_speakres_vrx++;
	if(m_speakres_vrx>0x300) bit0=0;        /* VR0 100k ohm - speech */
	if(m_speakres_vrx>0x200) bit1=0;        /* VR1  50k ohm - main volume */
	if(m_speakres_vrx>0x300) bit2=0;        /* VR2 100k ohm - explosion */

	return 0xf8|bit2|bit1|bit0;
}

void speakres_state::speakres_out2_w(uint8_t data)
{
	m_speakres_vrx=0;
}



/*************************************
 *
 *  CPU memory maps
 *
 *************************************/

void route16_state::route16_cpu1_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x3000, 0x3001).r(FUNC(route16_state::route16_prot_read));
	map(0x4000, 0x43ff).ram().w(FUNC(route16_state::route16_sharedram_w<true>)).share("sharedram");
	map(0x4800, 0x4800).portr("DSW").w(FUNC(route16_state::out0_w));
	map(0x5000, 0x5000).portr("P1").w(FUNC(route16_state::out1_w));
	map(0x5800, 0x5800).portr("P2");
	map(0x8000, 0xbfff).ram().share("videoram1");
}


void route16_state::routex_cpu1_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().w(FUNC(route16_state::route16_sharedram_w<true>)).share("sharedram");
	map(0x4800, 0x4800).portr("DSW").w(FUNC(route16_state::out0_w));
	map(0x5000, 0x5000).portr("P1").w(FUNC(route16_state::out1_w));
	map(0x5800, 0x5800).portr("P2");
	map(0x6400, 0x6400).r(FUNC(route16_state::routex_prot_read));
	map(0x8000, 0xbfff).ram().share("videoram1");
}


void speakres_state::stratvox_cpu1_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().share("sharedram");
	map(0x4800, 0x4800).portr("DSW").w(FUNC(speakres_state::out0_w));
	map(0x5000, 0x5000).portr("P1").w(FUNC(speakres_state::out1_w));
	map(0x5800, 0x5800).portr("P2");
	map(0x8000, 0xbfff).ram().share("videoram1");
}


void speakres_state::speakres_cpu1_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().share("sharedram");
	map(0x4800, 0x4800).portr("DSW").w(FUNC(speakres_state::out0_w));
	map(0x5000, 0x5000).portr("P1").w(FUNC(speakres_state::out1_w));
	map(0x5800, 0x5800).portr("P2").w(FUNC(speakres_state::speakres_out2_w));
	map(0x6000, 0x6000).r(FUNC(speakres_state::speakres_in3_r));
	map(0x8000, 0xbfff).ram().share("videoram1");
}


void route16_state::jongpute_cpu1_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().share("sharedram");
	map(0x4800, 0x4800).portr("DSW").w(FUNC(route16_state::out0_w));
	map(0x5000, 0x5000).r(FUNC(route16_state::jongpute_p2_matrix_r)).w(FUNC(route16_state::out1_w));
	map(0x5800, 0x5800).rw(FUNC(route16_state::jongpute_p1_matrix_r), FUNC(route16_state::jongpute_input_port_matrix_w));
	map(0x6800, 0x6800).w("ay8910", FUNC(ay8910_device::data_w));
	map(0x6900, 0x6900).w("ay8910", FUNC(ay8910_device::address_w));
	map(0x8000, 0xbfff).ram().share("videoram1");
}

void route16_state::vscompmj_cpu1_map(address_map &map)
{
	jongpute_cpu1_map(map);

	map(0x6900, 0x6900).r("ay8910", FUNC(ay8910_device::data_r)); // TODO: check this, stuck notes
	map(0x7000, 0x7fff).rom();
}

void route16_state::vscompmj_decrypted_opcodes(address_map &map)
{
	map(0x0000, 0x7fff).rom().share("decrypted_opcodes");
}


void route16_state::route16_cpu2_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram().w(FUNC(route16_state::route16_sharedram_w<false>)).share("sharedram");
	map(0x8000, 0xbfff).ram().share("videoram2");
}


void speakres_state::stratvox_cpu2_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2800, 0x2800).w(FUNC(speakres_state::stratvox_dac_w));
	map(0x4000, 0x43ff).ram().share("sharedram");
	map(0x8000, 0xbfff).ram().share("videoram2");
}


void route16_state::cpu1_io_map(address_map &map)
{
	map.global_mask(0x1ff);
	map(0x0000, 0x0000).mirror(0x00ff).w("ay8910", FUNC(ay8910_device::data_w));
	map(0x0100, 0x0100).mirror(0x00ff).w("ay8910", FUNC(ay8910_device::address_w));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( route16 )
	PORT_START("DSW")       /* DSW 1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )           PORT_DIPLOCATION("DSW:!1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_HIGH, "DSW:!2" )  // Manual says unused
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_HIGH, "DSW:!3" )  // Manual says unused
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )         PORT_DIPLOCATION("DSW:!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )  // Same as 0x08
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )         PORT_DIPLOCATION("DSW:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )     PORT_DIPLOCATION("DSW:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )     PORT_DIPLOCATION("DSW:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* Input Port 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")        /* Input Port 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static INPUT_PORTS_START( route16a )
	PORT_INCLUDE( route16 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )         PORT_DIPLOCATION("DSW:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )  // same as 0x00
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )  // same as 0x10
INPUT_PORTS_END


static INPUT_PORTS_START( stratvox )
	PORT_START("DSW")       /* IN0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )           PORT_DIPLOCATION("DSW:!1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, "Replenish Astronauts" )     PORT_DIPLOCATION("DSW:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x00, "2 Attackers At Wave" )      PORT_DIPLOCATION("DSW:!3,!4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Astronauts Kidnapped" )     PORT_DIPLOCATION("DSW:!5")
	PORT_DIPSETTING(    0x00, "Less Often" )
	PORT_DIPSETTING(    0x10, "More Often" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )         PORT_DIPLOCATION("DSW:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )     PORT_DIPLOCATION("DSW:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Demo Voices" )              PORT_DIPLOCATION("DSW:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")        /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static INPUT_PORTS_START( speakres )
	PORT_START("DSW")       /* IN0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )           PORT_DIPLOCATION("DSW:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, "2 Attackers At Wave" )      PORT_DIPLOCATION("DSW:!3,!4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )      PORT_DIPLOCATION("DSW:!5")
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "8000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )         PORT_DIPLOCATION("DSW:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )     PORT_DIPLOCATION("DSW:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Demo Voices" )              PORT_DIPLOCATION("DSW:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")        /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static INPUT_PORTS_START( spacecho )
	PORT_START("DSW")       /* IN0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )           PORT_DIPLOCATION("DSW:!1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, "Replenish Astronauts" )     PORT_DIPLOCATION("DSW:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x00, "2 Attackers At Wave" )      PORT_DIPLOCATION("DSW:!3,!4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Astronauts Kidnapped" )     PORT_DIPLOCATION("DSW:!5")
	PORT_DIPSETTING(    0x00, "Less Often" )
	PORT_DIPSETTING(    0x10, "More Often" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )         PORT_DIPLOCATION("DSW:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )     PORT_DIPLOCATION("DSW:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Demo Voices" )              PORT_DIPLOCATION("DSW:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")        /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static INPUT_PORTS_START( jongpute )
	PORT_START("DSW")       /* IN0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )        PORT_DIPLOCATION("DSW:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )        PORT_DIPLOCATION("DSW:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x08, "Timer Decrement Speed" )   PORT_DIPLOCATION("DSW:!3,!4")
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPSETTING(    0x08, "Normal" )
	PORT_DIPSETTING(    0x0c, "Slow" )
	PORT_DIPNAME( 0x10, 0x0, DEF_STR( Unknown ) )        PORT_DIPLOCATION("DSW:!5")
	PORT_DIPSETTING(    0x0, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )        PORT_DIPLOCATION("DSW:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )        PORT_DIPLOCATION("DSW:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )        PORT_DIPLOCATION("DSW:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_A ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_E ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_I ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_M ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_B ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_F ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_J ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_N ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_C ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_G ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_K ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_RON ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_D ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_H ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_L ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_PON ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END



/*************************************
 *
 *  Machine configs
 *
 *************************************/

void route16_state::route16(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_cpu1, 10_MHz_XTAL / 4); // verified on PCB
	m_cpu1->set_addrmap(AS_PROGRAM, &route16_state::route16_cpu1_map);
	m_cpu1->set_addrmap(AS_IO, &route16_state::cpu1_io_map);
	m_cpu1->set_vblank_int("screen", FUNC(route16_state::irq0_line_hold));

	Z80(config, m_cpu2, 10_MHz_XTAL / 4); // verified on PCB
	m_cpu2->set_addrmap(AS_PROGRAM, &route16_state::route16_cpu2_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-1);
	m_screen->set_refresh_hz(57);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(route16_state::screen_update_route16));

	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	AY8910(config, "ay8910", 10_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "speaker", 0.5);  // verified on PCB
}


void route16_state::routex(machine_config &config)
{
	route16(config);

	/* basic machine hardware */
	m_cpu1->set_addrmap(AS_PROGRAM, &route16_state::routex_cpu1_map);
}


void speakres_state::stratvox(machine_config &config)
{
	route16(config);

	/* basic machine hardware */
	m_cpu1->set_addrmap(AS_PROGRAM, &speakres_state::stratvox_cpu1_map);
	m_cpu2->set_addrmap(AS_PROGRAM, &speakres_state::stratvox_cpu2_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(speakres_state::screen_update_jongpute));

	/* sound hardware */
	subdevice<ay8910_device>("ay8910")->port_a_write_callback().set(FUNC(speakres_state::stratvox_sn76477_w));  // SN76477 commands (SN76477 not populated on Route 16 PCB)

	SN76477(config, m_sn);
	m_sn->set_noise_params(RES_K(47), RES_K(150), CAP_U(0.001));
	m_sn->set_decay_res(RES_M(3.3));
	m_sn->set_attack_params(CAP_U(1), RES_K(4.7));
	m_sn->set_amp_res(RES_K(200));
	m_sn->set_feedback_res(RES_K(55));
	m_sn->set_vco_params(5.0 * 2/ (2 + 10), CAP_U(0.022), RES_K(100));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(75));
	m_sn->set_oneshot_params(CAP_U(2.2), RES_K(4.7));
	m_sn->set_vco_mode(0);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(0, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "speaker", 0.5);

	DAC_8BIT_R2R(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25); // R = 15K, 2R = 30K (TODO: output filtering)
}

void speakres_state::speakres(machine_config &config)
{
	stratvox(config);

	/* basic machine hardware */
	m_cpu1->set_addrmap(AS_PROGRAM, &speakres_state::speakres_cpu1_map);

	MCFG_MACHINE_START_OVERRIDE(speakres_state, speakres)
}

void speakres_state::spacecho(machine_config &config)
{
	speakres(config);

	/* basic machine hardware */
	m_cpu2->set_periodic_int(FUNC(route16_state::irq0_line_hold), attotime::from_hz(48*60));
}

void route16_state::jongpute(machine_config &config)
{
	route16(config);
	m_cpu1->set_addrmap(AS_PROGRAM, &route16_state::jongpute_cpu1_map);
	m_cpu1->set_addrmap(AS_IO, address_map_constructor());

	MCFG_MACHINE_START_OVERRIDE(route16_state, jongpute)

	/* video hardware */
	m_screen->set_screen_update(FUNC(route16_state::screen_update_jongpute));

	PALETTE(config.replace(), m_palette, palette_device::BGR_3BIT);
}

void route16_state::vscompmj(machine_config &config)
{
	jongpute(config);

	m_cpu1->set_addrmap(AS_PROGRAM, &route16_state::vscompmj_cpu1_map);
	m_cpu1->set_addrmap(AS_OPCODES, &route16_state::vscompmj_decrypted_opcodes);
}

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( route16 )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "stvg54.a0",       0x0000, 0x0800, CRC(b8471cdc) SHA1(2a890782e15fa74a6c706b06f91216f427435700) )
	ROM_LOAD( "stvg55.a1",       0x0800, 0x0800, CRC(3ec52fe5) SHA1(451969b5caedd665231ef78cf262679d6d4c8507) )
	ROM_LOAD( "stvg56.a2",       0x1000, 0x0800, CRC(a8e92871) SHA1(68a709c14309d2b617997b76ae9d7b80fd326f39) )
	ROM_LOAD( "stvg57.a3",       0x1800, 0x0800, CRC(a0fc9fc5) SHA1(7013750c1b3d403b12eac10282a930538ed9c73e) )
	ROM_LOAD( "stvg58.a4",       0x2000, 0x0800, CRC(cc95c02c) SHA1(c0b85070883463a98098d72282c52e14822c204e) )
	ROM_LOAD( "stvg59.a5",       0x2800, 0x0800, CRC(a39ef648) SHA1(866095d9880b60b01f7ca66b332f5f6c4b41a5ac) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "stvg60.b0",       0x0000, 0x0800, CRC(fef605f3) SHA1(bfbffa0ded3e285c034f0ad832864021ef3f2256) )
	ROM_LOAD( "stvg61.b1",       0x0800, 0x0800, CRC(d0d6c189) SHA1(75cec891e20cf05aae354c8950857aea83c6dadc) )
	ROM_LOAD( "stvg62.b2",       0x1000, 0x0800, CRC(defc5797) SHA1(aec8179e647de70016e0e63b720f932752adacc1) )
	ROM_LOAD( "stvg63.b3",       0x1800, 0x0800, CRC(88d94a66) SHA1(163e952ada7c05110d1f1c681bd57d3b9ea8866e) )

	ROM_REGION( 0x800, "mcu", 0 ) // on a small daughterboard inserted at a6
	ROM_LOAD( "mb8841",       0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 )
	// The upper 128 bytes are 0's, used by the hardware to blank the display
	ROM_LOAD( "mb7052.59",    0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) // top bitmap
	ROM_LOAD( "mb7052.61",    0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) // bottom bitmap
ROM_END

 // 2 sets found, one on TVX1 and one on TVX2 PCB. TVX1 has lots of wire hacks. Both PCBs have an identical TVX-S1 sub board with Fujitsu MB8841 + logic.
 // There were two different bytes between the two versions, both in ROM a2:
 // 0x1dd is 0x46 in the TVX1 dump and 0x47 in the TVX2 one. 0x764 is 0x04 in the TVX1 dump and 0x06 in the TVX2 one.
 // Those cause the game to malfunction and it doesn't seem to be additional protection. The a2 ROM below is the one dumped from the TVX2 PCB.
ROM_START( route16d )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "a0.7.bin", 0x0000, 0x0800, CRC(025a4f63) SHA1(f1ced12c7667467c25f7fc595ae2e1b3aef4a29f) )
	ROM_LOAD( "a1.bin",   0x0800, 0x0800, CRC(ab6117fa) SHA1(99a96042ef9634fb32b7b5ca7e7e05f6637a7014) )
	ROM_LOAD( "a2.8.bin", 0x1000, 0x0800, CRC(5174f9ac) SHA1(162303589c30cbd2c1b701c32a1aa6f9008d3b35) )
	ROM_LOAD( "a3.bin",   0x1800, 0x0800, CRC(3ba19f10) SHA1(2e0f4a7b162f2ff2d17db0edc92f3c9804a8f8ee) )
	ROM_LOAD( "a4.bin",   0x2000, 0x0800, CRC(1da34bde) SHA1(b67420c6d7857303c137199f718de77ba7e38336) )
	ROM_LOAD( "a5.bin",   0x2800, 0x0800, CRC(74438cf5) SHA1(a7b7191f2a7b26577ee4459a6d3a83548f1c44ee) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "b0.bin", 0x0000, 0x0800, CRC(fef605f3) SHA1(bfbffa0ded3e285c034f0ad832864021ef3f2256) )
	ROM_LOAD( "b1.bin", 0x0800, 0x0800, CRC(49f265fc) SHA1(62e072828d1f64700dd2f3ed81417740e585bad4) )
	ROM_LOAD( "b2.bin", 0x1000, 0x0800, CRC(defc5797) SHA1(aec8179e647de70016e0e63b720f932752adacc1) )
	ROM_LOAD( "b3.bin", 0x1800, 0x0800, CRC(88d94a66) SHA1(163e952ada7c05110d1f1c681bd57d3b9ea8866e) )

	ROM_REGION( 0x800, "mcu", 0 ) // on a small daughterboard inserted at a6
	ROM_LOAD( "mb8841",       0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 )
	// The upper 128 bytes are 0's, used by the hardware to blank the display
	ROM_LOAD( "mb7052.59", 0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) // top bitmap
	ROM_LOAD( "mb7052.61", 0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) // bottom bitmap
ROM_END

ROM_START( route16a )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "tvg54.a0",     0x0000, 0x0800, CRC(aef9ffc1) SHA1(178d23e4963336ded93c13cb17940a4ae98270c5) )
	ROM_LOAD( "tvg55.a1",     0x0800, 0x0800, CRC(389bc077) SHA1(b0606f6e647e81ceae7148bda96bd4673a51e823) )
	ROM_LOAD( "tvg56.a2",     0x1000, 0x0800, CRC(1065a468) SHA1(4a707a42fb5a718043c173cb98ff3523eb274ccc) )
	ROM_LOAD( "tvg57.a3",     0x1800, 0x0800, CRC(0b1987f3) SHA1(9b8abd6ec1ae15ca0d5e4de6b8a7ebf6c929d767) )
	ROM_LOAD( "tvg58.a4",     0x2000, 0x0800, CRC(f67d853a) SHA1(7479e84082e78f8670cc50858ce6a006d3063413) )
	ROM_LOAD( "tvg59.a5",     0x2800, 0x0800, CRC(d85cf758) SHA1(5af21250ee44ab1a43b844ede5a777a3d33b78b5) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "tvg60.b0",     0x0000, 0x0800, CRC(0f9588a7) SHA1(dfaffec4dbabd98cdc21a416bd2966d9d3ae6ad1) )
	ROM_LOAD( "tvg61.b1",     0x0800, 0x0800, CRC(2b326cf9) SHA1(c6602a9440a982c39f5836c6ab72283b6f9241be) )
	ROM_LOAD( "tvg62.b2",     0x1000, 0x0800, CRC(529cad13) SHA1(b533d20df1f2580e237c3d60bfe3483486ad9a48) )
	ROM_LOAD( "tvg63.b3",     0x1800, 0x0800, CRC(3bd8b899) SHA1(bc0c7909dbf5ea85eba5a1bb815fdd98c3aa794e) )

	ROM_REGION( 0x800, "mcu", 0 ) // on a small daughterboard inserted at a6
	ROM_LOAD( "mb8841",       0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 )
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "mb7052.59",    0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "mb7052.61",    0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( route16c )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "route16.a0",   0x0000, 0x0800, CRC(8f9101bd) SHA1(b2c0156d41e295282387fb85fc272b031a6d1b64) )
	ROM_LOAD( "route16.a1",   0x0800, 0x0800, CRC(389bc077) SHA1(b0606f6e647e81ceae7148bda96bd4673a51e823) )
	ROM_LOAD( "route16.a2",   0x1000, 0x0800, CRC(1065a468) SHA1(4a707a42fb5a718043c173cb98ff3523eb274ccc) )
	ROM_LOAD( "route16.a3",   0x1800, 0x0800, CRC(0b1987f3) SHA1(9b8abd6ec1ae15ca0d5e4de6b8a7ebf6c929d767) )
	ROM_LOAD( "route16.a4",   0x2000, 0x0800, CRC(f67d853a) SHA1(7479e84082e78f8670cc50858ce6a006d3063413) )
	ROM_LOAD( "route16.a5",   0x2800, 0x0800, CRC(d85cf758) SHA1(5af21250ee44ab1a43b844ede5a777a3d33b78b5) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "route16.b0",   0x0000, 0x0800, CRC(0f9588a7) SHA1(dfaffec4dbabd98cdc21a416bd2966d9d3ae6ad1) )
	ROM_LOAD( "route16.b1",   0x0800, 0x0800, CRC(2b326cf9) SHA1(c6602a9440a982c39f5836c6ab72283b6f9241be) )
	ROM_LOAD( "route16.b2",   0x1000, 0x0800, CRC(529cad13) SHA1(b533d20df1f2580e237c3d60bfe3483486ad9a48) )
	ROM_LOAD( "route16.b3",   0x1800, 0x0800, CRC(3bd8b899) SHA1(bc0c7909dbf5ea85eba5a1bb815fdd98c3aa794e) )

	ROM_REGION( 0x800, "mcu", 0 ) // on a small daughterboard inserted at a6
	ROM_LOAD( "mb8841",       0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( route16b )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "vg-54",        0x0000, 0x0800, CRC(0c966319) SHA1(2f57e9a30dab864bbee2ccb0107c1b4212c5abaf) )
	ROM_LOAD( "vg-55",        0x0800, 0x0800, CRC(a6a8c212) SHA1(a4a695d401b1e495c863c6938296a99592df0e7d) )
	ROM_LOAD( "vg-56",        0x1000, 0x0800, CRC(5c74406a) SHA1(f106c27da6cac597afbabdef3ec7fa7d203905b0) )
	ROM_LOAD( "vg-57",        0x1800, 0x0800, CRC(313e68ab) SHA1(01fa83898123eb92a14bffc6fe774e00b083e86c) )
	ROM_LOAD( "vg-58",        0x2000, 0x0800, CRC(40824e3c) SHA1(bc157e6babf00d2119b389fdb9d5822e1c764f51) )
	ROM_LOAD( "vg-59",        0x2800, 0x0800, CRC(9313d2c2) SHA1(e08112f44ca454820752800d8b3b6408b73a4284) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "route16.b0",   0x0000, 0x0800, CRC(0f9588a7) SHA1(dfaffec4dbabd98cdc21a416bd2966d9d3ae6ad1) )
	ROM_LOAD( "vg-61",        0x0800, 0x0800, CRC(b216c88c) SHA1(d011ef9f3727f87ae3482e271a0c2496f76036b4) )
	ROM_LOAD( "route16.b2",   0x1000, 0x0800, CRC(529cad13) SHA1(b533d20df1f2580e237c3d60bfe3483486ad9a48) )
	ROM_LOAD( "route16.b3",   0x1800, 0x0800, CRC(3bd8b899) SHA1(bc0c7909dbf5ea85eba5a1bb815fdd98c3aa794e) )

	ROM_REGION( 0x800, "mcu", 0 ) // on a small daughterboard inserted at a6
	ROM_LOAD( "mb8841",       0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( route16bl )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "rt16.0",       0x0000, 0x0800, CRC(b1f0f636) SHA1(f21915ed40ebdf64970fb7e3cd8071ebfc4aa0b5) )
	ROM_LOAD( "rt16.1",       0x0800, 0x0800, CRC(3ec52fe5) SHA1(451969b5caedd665231ef78cf262679d6d4c8507) )
	ROM_LOAD( "rt16.2",       0x1000, 0x0800, CRC(a8e92871) SHA1(68a709c14309d2b617997b76ae9d7b80fd326f39) )
	ROM_LOAD( "rt16.3",       0x1800, 0x0800, CRC(a0fc9fc5) SHA1(7013750c1b3d403b12eac10282a930538ed9c73e) )
	ROM_LOAD( "rt16.4",       0x2000, 0x0800, CRC(6dcaf8c4) SHA1(27d84cc29f2b75280678e9c77f270ee39af50228) )
	ROM_LOAD( "rt16.5",       0x2800, 0x0800, CRC(63d7b05b) SHA1(d1e3473be283c92063674b9e69575081115bc456) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "rt16.6",       0x0000, 0x0800, CRC(fef605f3) SHA1(bfbffa0ded3e285c034f0ad832864021ef3f2256) )
	ROM_LOAD( "rt16.7",       0x0800, 0x0800, CRC(d0d6c189) SHA1(75cec891e20cf05aae354c8950857aea83c6dadc) )
	ROM_LOAD( "rt16.8",       0x1000, 0x0800, CRC(defc5797) SHA1(aec8179e647de70016e0e63b720f932752adacc1) )
	ROM_LOAD( "rt16.9",       0x1800, 0x0800, CRC(88d94a66) SHA1(163e952ada7c05110d1f1c681bd57d3b9ea8866e) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( routex )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "routex01.a0",  0x0000, 0x0800, CRC(99b500e7) SHA1(2561c04a1425d7ac3309faf29fcfde63a0cda4da) )
	ROM_LOAD( "rt16.1",       0x0800, 0x0800, CRC(3ec52fe5) SHA1(451969b5caedd665231ef78cf262679d6d4c8507) )
	ROM_LOAD( "rt16.2",       0x1000, 0x0800, CRC(a8e92871) SHA1(68a709c14309d2b617997b76ae9d7b80fd326f39) )
	ROM_LOAD( "rt16.3",       0x1800, 0x0800, CRC(a0fc9fc5) SHA1(7013750c1b3d403b12eac10282a930538ed9c73e) )
	ROM_LOAD( "routex05.a4",  0x2000, 0x0800, CRC(2fef7653) SHA1(ba3477da249ca402d096704e57ea638fde6abe9c) )
	ROM_LOAD( "routex06.a5",  0x2800, 0x0800, CRC(a39ef648) SHA1(866095d9880b60b01f7ca66b332f5f6c4b41a5ac) )
	ROM_LOAD( "routex07.a6",  0x3000, 0x0800, CRC(89f80c1c) SHA1(dff37e0f2446a99890135891c59dc501866a25cc) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "routex11.b0",  0x0000, 0x0800, CRC(b51edd1d) SHA1(1ca10afd6851875c98b1d29aee457234c20ce0bf) )
	ROM_LOAD( "rt16.7",       0x0800, 0x0800, CRC(d0d6c189) SHA1(75cec891e20cf05aae354c8950857aea83c6dadc) )
	ROM_LOAD( "rt16.8",       0x1000, 0x0800, CRC(defc5797) SHA1(aec8179e647de70016e0e63b720f932752adacc1) )
	ROM_LOAD( "rt16.9",       0x1800, 0x0800, CRC(88d94a66) SHA1(163e952ada7c05110d1f1c681bd57d3b9ea8866e) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( routexa )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "r1.bin", 0x0000, 0x0800, CRC(78c819c8) SHA1(41951aef3ad8be53aadb17892b5b3e13be61b50f) ) // unique
	ROM_LOAD( "r2.bin", 0x0800, 0x0800, CRC(3ec52fe5) SHA1(451969b5caedd665231ef78cf262679d6d4c8507) )
	ROM_LOAD( "r3.bin", 0x1000, 0x0800, CRC(a8e92871) SHA1(68a709c14309d2b617997b76ae9d7b80fd326f39) )
	ROM_LOAD( "r4.bin", 0x1800, 0x0800, CRC(a0fc9fc5) SHA1(7013750c1b3d403b12eac10282a930538ed9c73e) )
	ROM_LOAD( "r5.bin", 0x2000, 0x0800, CRC(2fef7653) SHA1(ba3477da249ca402d096704e57ea638fde6abe9c) )
	ROM_LOAD( "r6.bin", 0x2800, 0x0800, CRC(a39ef648) SHA1(866095d9880b60b01f7ca66b332f5f6c4b41a5ac) )
	ROM_LOAD( "r7.bin", 0x3000, 0x0800, CRC(2aeb3102) SHA1(7398dd43b1717aef8dc18210758db9fa828bd92e) ) // unique, FIXED BITS (010x011011111111) and 1ST AND 2ND HALF IDENTICAL, but confirmed on two different PCBs

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "r8.bin",  0x0000, 0x0800, CRC(fef605f3) SHA1(bfbffa0ded3e285c034f0ad832864021ef3f2256) ) // unique
	ROM_LOAD( "r9.bin",  0x0800, 0x0800, CRC(d0d6c189) SHA1(75cec891e20cf05aae354c8950857aea83c6dadc) )
	ROM_LOAD( "r10.bin", 0x1000, 0x0800, CRC(defc5797) SHA1(aec8179e647de70016e0e63b720f932752adacc1) )
	ROM_LOAD( "r11.bin", 0x1800, 0x0800, CRC(88d94a66) SHA1(163e952ada7c05110d1f1c681bd57d3b9ea8866e) )

	ROM_REGION( 0x0200, "proms", 0 ) // Intersil IM5623CPE proms compatible with 82s129
	// The upper 128 bytes are 0's, used by the hardware to blank the display
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) // top bitmap
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) // bottom bitmap
ROM_END

ROM_START( speakres )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "speakres.1",   0x0000, 0x0800, CRC(6026e4ea) SHA1(77975620b489f10e5b5de834e812c2802315e889) )
	ROM_LOAD( "speakres.2",   0x0800, 0x0800, CRC(93f0d4da) SHA1(bf3d2931d12a436bb4f0d0556806008ca722f070) )
	ROM_LOAD( "speakres.3",   0x1000, 0x0800, CRC(a3874304) SHA1(ca243364d077fa70d6c46b950ba6666617a56cc2) )
	ROM_LOAD( "speakres.4",   0x1800, 0x0800, CRC(f484be3a) SHA1(5befa61c5f3a3cde3d7d6cae2130021288ed8454) )
	ROM_LOAD( "speakres.5",   0x2000, 0x0800, CRC(61b12a67) SHA1(a1a636ecde16ffdc9f0bb460bd12f945ec66d36f) )
	ROM_LOAD( "speakres.6",   0x2800, 0x0800, CRC(220e0ab2) SHA1(9fb4abf50ff28995cb1f7ba807e15eb87127f520) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "speakres.7",   0x0000, 0x0800, CRC(d417be13) SHA1(6f1f76a911579b49bb0e1992296e7c3acf2bd517) )
	ROM_LOAD( "speakres.8",   0x0800, 0x0800, CRC(52485d60) SHA1(28b708a71d16428d1cd58f3b7aa326ccda85533c) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( speakresb )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "hmi1.27",      0x0000, 0x0800, CRC(6026e4ea) SHA1(77975620b489f10e5b5de834e812c2802315e889) )
	ROM_LOAD( "hmi2.28",      0x0800, 0x0800, CRC(93f0d4da) SHA1(bf3d2931d12a436bb4f0d0556806008ca722f070) )
	ROM_LOAD( "hmi3.29",      0x1000, 0x0800, CRC(a3874304) SHA1(ca243364d077fa70d6c46b950ba6666617a56cc2) )
	ROM_LOAD( "hmi4.30",      0x1800, 0x0800, CRC(f484be3a) SHA1(5befa61c5f3a3cde3d7d6cae2130021288ed8454) )
	ROM_LOAD( "hmi5.31",      0x2000, 0x0800, CRC(aa2aaabe) SHA1(eae34bc16ffa1c8dba966c367fae793c52e0cb61) )
	ROM_LOAD( "hmi6.32",      0x2800, 0x0800, CRC(220e0ab2) SHA1(9fb4abf50ff28995cb1f7ba807e15eb87127f520) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "hmi.33",       0x0000, 0x0800, CRC(beafe7c5) SHA1(058d08b4ded46f71053af6bec5e476e21f240608) )
	ROM_LOAD( "hmi.34",       0x0800, 0x0800, CRC(12ecd87b) SHA1(a279711f2a12574126aa626ae2c1acd45660231c) )

	ROM_REGION( 0x0200, "proms", 0 ) /* 6301 proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "hmi.62",       0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "hmi.64",       0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( stratvox )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "ls01.bin",     0x0000, 0x0800, CRC(bf4d582e) SHA1(456f37e16d037a30dc4c1c460ebf9a248bf1a57c) )
	ROM_LOAD( "ls02.bin",     0x0800, 0x0800, CRC(16739dd4) SHA1(cd1f7d1b52ca1ab458d11b969f4f1f5af3ec7353) )
	ROM_LOAD( "ls03.bin",     0x1000, 0x0800, CRC(083c28de) SHA1(82e159f218f60e9c06ff78f2e52572f8f5a6c530) )
	ROM_LOAD( "ls04.bin",     0x1800, 0x0800, CRC(b0927e3b) SHA1(cc5f030dcbc93d5265dbf17a2425acdb921ab18b) )
	ROM_LOAD( "ls05.bin",     0x2000, 0x0800, CRC(ccd25c4e) SHA1(d6d5722d746dd22cecacfea407e798f4531eea99) )
	ROM_LOAD( "ls06.bin",     0x2800, 0x0800, CRC(07a907a7) SHA1(0c41eac01ac9fd67ef19752c47414c4bd90324b4) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "ls07.bin",     0x0000, 0x0800, CRC(4d333985) SHA1(371405b92b2ee8040e48ec7ad715d1a960746aac) )
	ROM_LOAD( "ls08.bin",     0x0800, 0x0800, CRC(35b753fc) SHA1(179e21f531e8be507f1754159590c111be1b44ff) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( stratvoxa )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "sv-1",     0x0000, 0x0800, CRC(bf4d582e) SHA1(456f37e16d037a30dc4c1c460ebf9a248bf1a57c) )
	ROM_LOAD( "sv-2",     0x0800, 0x0800, CRC(16739dd4) SHA1(cd1f7d1b52ca1ab458d11b969f4f1f5af3ec7353) )
	ROM_LOAD( "sv-3",     0x1000, 0x0800, CRC(083c28de) SHA1(82e159f218f60e9c06ff78f2e52572f8f5a6c530) )
	ROM_LOAD( "sv-4",     0x1800, 0x0800, CRC(b0927e3b) SHA1(cc5f030dcbc93d5265dbf17a2425acdb921ab18b) )
	ROM_LOAD( "sv-5",     0x2000, 0x0800, CRC(ccd25c4e) SHA1(d6d5722d746dd22cecacfea407e798f4531eea99) )
	ROM_LOAD( "sv-6",     0x2800, 0x0800, CRC(07a907a7) SHA1(0c41eac01ac9fd67ef19752c47414c4bd90324b4) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "sv-7",     0x0000, 0x0800, CRC(4d333985) SHA1(371405b92b2ee8040e48ec7ad715d1a960746aac) )
	ROM_LOAD( "sv-8",     0x0800, 0x0800, CRC(9b2377e0) SHA1(caff09f280701204dbd0dcd093622ed42ceb57c4) ) // Female Voice

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( stratvoxb )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "j0-1",         0x0000, 0x0800, CRC(93c78274) SHA1(d7c8b5a064eaf96bcfd261b9857f06249477f6b8) )
	ROM_LOAD( "j0-2",         0x0800, 0x0800, CRC(93b2b02d) SHA1(f08772d581f9825976199f39cb6d85fb3aa83db0) )
	ROM_LOAD( "j0-3",         0x1000, 0x0800, CRC(655facb5) SHA1(1ffb1ed65c358846b3de4ead74e86f94ed6ff9df) )
	ROM_LOAD( "j0-4",         0x1800, 0x0800, CRC(b0927e3b) SHA1(cc5f030dcbc93d5265dbf17a2425acdb921ab18b) ) /* Same as ls04.bin of stratvox */
	ROM_LOAD( "j0-5",         0x2000, 0x0800, CRC(9d2178d9) SHA1(7b27dbb2add2c9dda4526c6f1bf52307fe2c6335) )
	ROM_LOAD( "j0-6",         0x2800, 0x0800, CRC(79118ffc) SHA1(d4659f1773e9d55d81185d6c59881c08528e2ab6) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "b0-a",         0x0000, 0x0800, CRC(4d333985) SHA1(371405b92b2ee8040e48ec7ad715d1a960746aac) ) /* Same as ls07.bin of stratvox */
	ROM_LOAD( "j0-a",         0x0800, 0x0800, CRC(3416a830) SHA1(9cbe773968e20455be3e107b29cb8d4dc38632a9) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( spacecho )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "rom.a0",       0x0000, 0x0800, CRC(40d74dce) SHA1(891d7fde1d4b0b66c38fa7f8933480e201c68113) )
	ROM_LOAD( "rom.a1",       0x0800, 0x0800, CRC(a5f0a34f) SHA1(359e7a9954dedb464f7456cd071db77b2219ab2c) )
	ROM_LOAD( "rom.a2",       0x1000, 0x0800, CRC(cbbb3acb) SHA1(3dc71683f31da39a544382b463ece39cca8124b3) )
	ROM_LOAD( "rom.a3",       0x1800, 0x0800, CRC(311050ca) SHA1(ed4a5cb7ec0306654178dae8f30b39b9c8db0ce3) )
	ROM_LOAD( "rom.a4",       0x2000, 0x0800, CRC(28943803) SHA1(4904e6d092494bfca064d25d094ab9e9049fa9ca) )
	ROM_LOAD( "rom.a5",       0x2800, 0x0800, CRC(851c9f28) SHA1(c7bb4e25b74eb71e8b394214f9cbd95f59a1fa58) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "rom.b0",       0x0000, 0x0800, CRC(db45689d) SHA1(057a8dc2629f57fdeebb6262de2bdd78b4e66dca) )
	ROM_LOAD( "rom.b2",       0x1000, 0x0800, CRC(1e074157) SHA1(cb2073415aff7804ac85e2137bef2005bf6cf239) )
	ROM_LOAD( "rom.b3",       0x1800, 0x0800, CRC(d50a8b20) SHA1(d733fa327d2e7dfe08c84015c6c326ed8ab39e3d) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

/*
rom.b0                  cb7.5b                  IDENTICAL
rom.a5                  cb6.2.3t                IDENTICAL
rom.a2                  c3.4.5t                 IDENTICAL
rom.a1                  c2.5t                   IDENTICAL
rom.a4                  cb5.3t                  IDENTICAL
rom.b3                  cb10.3b                 IDENTICAL
rom.b2                  cb9.4b                  IDENTICAL
rom.a3                  c4.4t                   IDENTICAL
rom.a0                  c11.5.6t                99.853516%
                        mb7052.6k               NO MATCH
                        mb7052.6m               NO MATCH

Only 3 bytes different between rom.a0 (spacecho) and c11.5.6t (spacecho2), at offset 0x8b.

Spacecho:    0x008b:  call $2929    cd 29 29

Spacech2:    0x008b:  im 1          ed 56
             0x008d:  nop           00

So... spacecho2 is avoiding to enter the sub at $2929.

*/
ROM_START( spacecho2 )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "c11.5.6t",     0x0000, 0x0800, CRC(90637f25) SHA1(820d2f326a5d8d0a04a0fca46b035624dfd7222c) )    // 3 bytes different at 0x8e
	ROM_LOAD( "c2.5t",        0x0800, 0x0800, CRC(a5f0a34f) SHA1(359e7a9954dedb464f7456cd071db77b2219ab2c) )
	ROM_LOAD( "c3.4.5t",      0x1000, 0x0800, CRC(cbbb3acb) SHA1(3dc71683f31da39a544382b463ece39cca8124b3) )
	ROM_LOAD( "c4.4t",        0x1800, 0x0800, CRC(311050ca) SHA1(ed4a5cb7ec0306654178dae8f30b39b9c8db0ce3) )
	ROM_LOAD( "cb5.3t",       0x2000, 0x0800, CRC(28943803) SHA1(4904e6d092494bfca064d25d094ab9e9049fa9ca) )
	ROM_LOAD( "cb6.2.3t",     0x2800, 0x0800, CRC(851c9f28) SHA1(c7bb4e25b74eb71e8b394214f9cbd95f59a1fa58) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "cb7.5b",       0x0000, 0x0800, CRC(db45689d) SHA1(057a8dc2629f57fdeebb6262de2bdd78b4e66dca) )
	ROM_LOAD( "cb9.4b",       0x1000, 0x0800, CRC(1e074157) SHA1(cb2073415aff7804ac85e2137bef2005bf6cf239) )
	ROM_LOAD( "cb10.3b",      0x1800, 0x0800, CRC(d50a8b20) SHA1(d733fa327d2e7dfe08c84015c6c326ed8ab39e3d) )

	ROM_REGION( 0x0200, "proms", 0 ) /* mb7052 proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "mb7052.6k",    0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "mb7052.6m",    0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

/*
Speak & Help

Single layer re-engineered pcb, very tidy and working.

All dumps are in label.location format, see the two
included photos for one of the pcb with and without the
speech? daughterboard plugged in for verification.

Roms are all mitsubishi 2716, proms are fujitsu MB7052.

https://youtu.be/YuWZ8hZ-MtY
Unique speech, as detailed in video, seems will require additional work to emulate correctly.
*/

ROM_START( speakhlp )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "b1.56t",       0x0000, 0x0800, CRC(ce009d85) SHA1(d8683d358ff04ffa0eef574e42a8f3885f538ecc) )
	ROM_LOAD( "b2.5t",        0x0800, 0x0800, CRC(935219f1) SHA1(83d41eb8af6dc5d44d578c01c123872e75fa927e) )
	ROM_LOAD( "b3.45t",       0x1000, 0x0800, CRC(083c28de) SHA1(82e159f218f60e9c06ff78f2e52572f8f5a6c530) )
	ROM_LOAD( "b4.4t",        0x1800, 0x0800, CRC(b0927e3b) SHA1(cc5f030dcbc93d5265dbf17a2425acdb921ab18b) )
	ROM_LOAD( "b5.3t",        0x2000, 0x0800, CRC(ccd25c4e) SHA1(d6d5722d746dd22cecacfea407e798f4531eea99) )
	ROM_LOAD( "b6.23t",       0x2800, 0x0800, CRC(a657dd4b) SHA1(4f6b85ccf5449d08f5c7f5dc6f59d0df276d9994) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "b07.5b",       0x0000, 0x0800, CRC(c9317d91) SHA1(b509ce371d89ad39acaefea732eb955a11df1ed9) )
	ROM_LOAD( "b09.4b",       0x1000, 0x0800, CRC(29310c32) SHA1(d5d5953111d81661ab98c950d94e5912fc907445) )
	ROM_LOAD( "b010.3b",      0x1800, 0x0800, CRC(4d567bc9) SHA1(6bc05213042d9069a054b2ae044f04938a9bfe06) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "prom.6k",      0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "prom.6m",      0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( ttmahjng )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "ju04",         0x0000, 0x1000, CRC(fe7c693a) SHA1(be0630557e0bcd9ec2e9542cc4a4d947889ec57a) )
	ROM_LOAD( "ju05",         0x1000, 0x1000, CRC(985723d3) SHA1(9d7499c48cfc242875a95d01459b8f3252ea41bc) )
	ROM_LOAD( "ju06",         0x2000, 0x1000, CRC(2cd69bc8) SHA1(a0a55c972291d043da9f76faf551dba790d5d103) )
	ROM_LOAD( "ju07",         0x3000, 0x1000, CRC(30e8ec63) SHA1(9c6a2b5e436b5e469c15f04c557839b6f07eb22e) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "ju01",         0x0000, 0x0800, CRC(0f05ca3c) SHA1(6af547b2ec4f69069b4ad62d96d109ec0105dd8b) )
	ROM_LOAD( "ju02",         0x0800, 0x0800, CRC(c1ffeceb) SHA1(18cf337ef2c9b51f1e9e4f08743225755c4ff420) )
	ROM_LOAD( "ju08",         0x1000, 0x0800, CRC(2dcc76b5) SHA1(1732bcf5492dda34425681e7f28775ad7a5e04af) )

	ROM_REGION( 0x0200, "proms", 0 )
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "ju03",         0x0000, 0x0100, CRC(27d47624) SHA1(ee04ce8043216be8b91413b546479419fca2b917) )
	ROM_LOAD( "ju09",         0x0100, 0x0100, CRC(27d47624) SHA1(ee04ce8043216be8b91413b546479419fca2b917) )
ROM_END

ROM_START( jongpute )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "j2",           0x0000, 0x1000, CRC(6690b6a4) SHA1(ab79faa1ed84d766eee652f3cbdc0296ddb80fe2) )
	ROM_LOAD( "j3",           0x1000, 0x1000, CRC(985723d3) SHA1(9d7499c48cfc242875a95d01459b8f3252ea41bc) )
	ROM_LOAD( "j4",           0x2000, 0x1000, CRC(f35ab1e6) SHA1(5b76d05ab9d8b2a88b408cf9e9297ec31a8de33a) )
	ROM_LOAD( "j5",           0x3000, 0x1000, CRC(77074618) SHA1(73329e945ea578bce1d04c80e09929bfb0e9875b) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "j6",           0x0000, 0x1000, CRC(54b349b0) SHA1(e5620b85a24a35d995860c7121f1ddf16f7ea168) )

	/* maybe used for pseudo sampling voice, "reach", that is not emulated yet */
	ROM_REGION( 0x1000, "unknown", 0 )
	ROM_LOAD( "j1",           0x0000, 0x1000, CRC(6d6ba272) SHA1(a4efd8daddbbf595ee46484578f544d7ed84e090) )

	ROM_REGION( 0x0200, "proms", 0 )
	/* not dumped, but ttmahjng roms seem to be compatible completely */
	ROM_LOAD( "ju03",         0x0000, 0x0100, BAD_DUMP CRC(27d47624) SHA1(ee04ce8043216be8b91413b546479419fca2b917) )
	ROM_LOAD( "ju09",         0x0100, 0x0100, BAD_DUMP CRC(27d47624) SHA1(ee04ce8043216be8b91413b546479419fca2b917) )
ROM_END

ROM_START( vscompmj )
	ROM_REGION( 0x8000, "cpu1", 0 ) // all 2732
	ROM_LOAD( "j2_1.0r",           0x0000, 0x1000, CRC(e112ac58) SHA1(a274080dfd89c547335f93cb8f99e80ec7b972df) )
	ROM_LOAD( "j2_2.0n",           0x1000, 0x1000, CRC(c751c041) SHA1(69063549e616fdd9d175b47275331986f1d3e0bd) )
	ROM_LOAD( "j2_3.0l",           0x2000, 0x1000, CRC(e85bf26b) SHA1(8bb6625433c9f86808a41bde7dd587bdc430b934) )
	ROM_LOAD( "j2_4.0k",           0x3000, 0x1000, CRC(ead1b054) SHA1(fa0940391968541cdfd3d306c7bfd6781617b580) )
	ROM_LOAD( "j2_5.0j",           0x7000, 0x1000, CRC(cbf49c08) SHA1(064054fd9e36c8a359926ade4fc10855d3058f01) )

	ROM_REGION( 0x2000, "cpu2", 0 )
	ROM_LOAD( "j2_6.0e",           0x0000, 0x1000, CRC(3a559328) SHA1(dd6333ddcc8aa6097d83b21cfde740b2cb7c908b) ) // 2732

	ROM_REGION( 0x0200, "proms", 0 )
	// The upper 128 bytes are 0's, used by the hardware to blank the display
	ROM_LOAD( "82s129.6k",         0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) )
	ROM_LOAD( "82s129.6h",         0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) )

	ROM_REGION( 0x0100, "proms2", 0 ) // currently unused by the emulation
	ROM_LOAD( "82s129.9r",         0x0000, 0x0100, CRC(20ac25d8) SHA1(6f06472ac7fcb22c9060092a2d456be5d3ca6d5f) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1981, route16,  0,        route16,  route16,  route16_state, init_route16,  ROT270, "Sun Electronics",                            "Route 16 (Sun Electronics, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, route16d, route16,  route16,  route16a, route16_state, init_route16d, ROT270, "Sun Electronics",                            "Route 16 (Sun Electronics, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, route16a, route16,  route16,  route16a, route16_state, init_route16a, ROT270, "Tehkan / Sun Electronics (Centuri license)", "Route 16 (Centuri license, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, route16b, route16,  route16,  route16,  route16_state, init_route16,  ROT270, "Tehkan / Sun Electronics (Centuri license)", "Route 16 (Centuri license, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, route16c, route16,  route16,  route16,  route16_state, init_route16c, ROT270, "Tehkan / Sun Electronics (Centuri license)", "Route 16 (Centuri license, set 3, bootleg?)", MACHINE_SUPPORTS_SAVE ) // similar to set 1 but with some protection removed?
GAME( 1981, route16bl,route16,  route16,  route16,  route16_state, empty_init,    ROT270, "bootleg (Leisure and Allied)",               "Route 16 (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, routex,   route16,  routex,   route16,  route16_state, empty_init,    ROT270, "bootleg",                                    "Route X (bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, routexa,  route16,  routex,   route16,  route16_state, empty_init,    ROT270, "bootleg",                                    "Route X (bootleg, set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1980, speakres, 0,        speakres, speakres, speakres_state, empty_init,   ROT270, "Sun Electronics",                 "Speak & Rescue", MACHINE_SUPPORTS_SAVE )
GAME( 1980, speakresb,speakres, speakres, speakres, speakres_state, empty_init,   ROT270, "bootleg",                         "Speak & Rescue (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, stratvox, speakres, stratvox, stratvox, speakres_state, empty_init,   ROT270, "Sun Electronics (Taito license)", "Stratovox (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, stratvoxa,speakres, stratvox, stratvox, speakres_state, empty_init,   ROT270, "Sun Electronics (Taito license)", "Stratovox (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, stratvoxb,speakres, stratvox, stratvox, speakres_state, empty_init,   ROT270, "bootleg",                         "Stratovox (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacecho, speakres, spacecho, spacecho, speakres_state, empty_init,   ROT270, "bootleg (Gayton Games)",          "Space Echo (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacecho2,speakres, spacecho, spacecho, speakres_state, empty_init,   ROT270, "bootleg (Gayton Games)",          "Space Echo (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, speakhlp, speakres, spacecho, spacecho, speakres_state, empty_init,   ROT270, "bootleg",                         "Speak & Help", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAME( 1981, jongpute, 0,        jongpute, jongpute, route16_state, empty_init,    ROT0,   "Alpha Denshi Co.",                 "Jongputer",   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING )  // sampling voice is not emulated, bug with colors makes tile recognition difficult
GAME( 1981, ttmahjng, jongpute, jongpute, jongpute, route16_state, empty_init,    ROT0,   "Alpha Denshi Co. (Taito license)", "T.T Mahjong", MACHINE_SUPPORTS_SAVE )
GAME( 1981, vscompmj, jongpute, vscompmj, jongpute, route16_state, init_vscompmj, ROT0,   "Nichibutsu",                       "VS Computer Mahjong", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING ) // decryption might be incomplete (attract resets), inputs seem read differently
