// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood

/***************************************************************************

Puckman Pockimon

Seems to be based around genesis hardware, despite containing no original Sega chips

Supported:

Puckman Pockimon
(there should be a way to show Sun Mixing copyright, ROMs are the same on a version with the SM (c))

|---------------------------------------|
| VOL    4558    4MHz   PAL     62256   |
| U6612  U6614B                         |
| 3.579545MHz    555    PAL     |------||
| LM324     M6295        |----| |TV16B ||
|          ROM.U3   PAL  |YBOX| |      ||
|J   PAL                 |----| |      ||
|A         PAL      PAL         |------||
|M         PAL      PAL  PAL            |
|M         PAL      PAL     53.693175MHz|
|A   DSW2                               |
|          PAL      PAL    |------|     |
|    DSW1                  |TA06SD|     |
|          ROM.U5   ROM.U4 |      |     |
|                          |------|     |
|          ROM.U8   ROM.U7 62256  D41264|
|          *        *      62256  D41264|
|---------------------------------------|
Notes:
      Main CPU is 68000-based, but actual CPU chip is not known
      Master clock 53.693175MHz. CPU likely running at 53.693175/7 or /6 (??)
      U6612 (YM3812 clone?) clock 3.579545MHz
      U6614B (YM3014B clone?)
      M6295 clock 1.000MHz (4/4). Sample rate = 1000000/132
      VSync 60Hz
      HSync 16.24kHz
      62256 - 8k x8 SRAM (DIP28)
      D41264 - NEC D41264V-15V 64k x4 VRAM (ZIP24)
      * Unpopulated DIP32 socket
      Custom ICs -
                  Y-BOX TA891945 (QFP100)
                  TA-06SD 9933 B816453 (QFP128)
                  TV16B 0010 ME251271 (QFP160)

Some Puckman Pockimon boards have a different layout, lacking the U6612, U6614B and 3.579545MHz crystal.

*/

/* Genie's Hardware (contains no real Sega parts) */

/***************************************************************************
Puckman Pokemon Genie 2000
(c) 2000?  Manufacturer ?

Hardware looks bootleg-ish, but is newly manufactured.

CPU: ? (one of the SMD chips)
SND: OKI6295, U6612 (probably YM3812), U6614B (Probably YM3014B)
XTAL: 3.579545MHz, 4.0000MHz
OSC: 53.693175MHz
Other Chips: Possible CPU: TA-06SD 9933 B816453 128 pin square SMD
             GFX support chips: Y-BOX TA891945 100 pin SMD
                                TV16B 0010 ME251271 160 pin SMD

There are 13 PAL's on the PCB !

RAM: 62256 x 3, D41264 x 2 (ZIP Ram)
DIPS: 2 x 8 position
SW1:
                        1   2   3   4   5   6   7   8
coins   1coin 1 Cred.   off off off
        2c 1c           on  off off
        3c 1c           off on  off
        4c 1c           on  on  off
        5c 1c           off off on
        1c 2c           on  off on
        1c 3c           off on  on
        1c 4c           on  on  on

players 1                           off off off
        2                           on  off off
        3                           off on  off
        4                           on  on  off
        5                           off off on
        6                           on  off on
        7                           off on  on
        8                           on  on  on

diffic-
ulty    v.easy                                  off off
        normal                                  on  off
        diffic.                                 off on
        v. diffic.                              on  on


SW2

note position 3-8 not used

                    1   2   3   4   5   6   7   8
test mode   no      off
            yes     on

demo sound  yes         off
            no          on


ROMS:
PUCKPOKE.U3 M5M27C201   Sound
PUCKPOKE.U4 27C040--\
PUCKPOKE.U5 27C040---\
PUCKPOKE.U7 27C040----- Main program & GFX
PUCKPOKE.U8 27C4001---/

ROM sockets U63 & U64 empty

****************************************************************************/

#include "emu.h"

#include "megadriv.h"

#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"

namespace {

class puckpkmn_state : public md_ctrl_state
{
public:
	puckpkmn_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_ctrl_state(mconfig, type, tag),
		m_io_exp(*this, "EXP")
	{
	}

	void puckpkmn(machine_config &config) ATTR_COLD;
	void jingling(machine_config &config) ATTR_COLD;
	void puckpkmnb(machine_config &config) ATTR_COLD;

	void init_puckpkmn() ATTR_COLD;

protected:
	void puckpkmn_base_map(address_map &map) ATTR_COLD;

private:
	void puckpkmn_map(address_map &map) ATTR_COLD;
	void jingling_map(address_map &map) ATTR_COLD;
	void puckpkmnb_map(address_map &map) ATTR_COLD;

	optional_ioport m_io_exp;
};


class jzth_state : public puckpkmn_state
{
public:
	using puckpkmn_state::puckpkmn_state;

	void jzth(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void bl_710000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bl_710000_r();

	void jzth_map(address_map &map) ATTR_COLD;

	int m_protcount = 0;
};

class songjang_state : public puckpkmn_state
{
public:
	songjang_state(const machine_config &mconfig, device_type type, const char *tag) :
		puckpkmn_state(mconfig, type, tag)
	{
	}

	void songjang(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint16_t unhandled_protval_r();
	void unhandled_protval_w(offs_t offset, uint16_t data);

	uint16_t protval_r();
	void protval_w(uint16_t data);

	virtual uint16_t sj_70001c_r();

	void songjang_map(address_map &map) ATTR_COLD;

	uint16_t m_protval;
};

class shuifeng_state : public songjang_state
{
public:
	shuifeng_state(const machine_config &mconfig, device_type type, const char *tag) :
		songjang_state(mconfig, type, tag)
	{
	}

private:
	virtual uint16_t sj_70001c_r() override;
};

/************************************ Mega Drive Bootlegs *************************************/


void puckpkmn_state::puckpkmn_base_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x3fffff).rom();                             // Main 68k Program ROMs

	map(0x700010, 0x700011).portr("P2");
	map(0x700012, 0x700013).portr("P1");
	map(0x700014, 0x700015).portr("UNK");
	map(0x700016, 0x700017).portr("DSW1");
	map(0x700018, 0x700019).portr("DSW2");
	map(0x700023, 0x700023).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


void puckpkmn_state::puckpkmnb_map(address_map &map)
{
	puckpkmn_base_map(map);

	// Unknown reads/writes:
	map(0xa00000, 0xa00551).nopw();                            // ?
//  map(0xa10000, 0xa10001).nopr();                            // ? once
	map(0xa10002, 0xa10005).noprw();                           // ? alternative way of reading inputs ?
	map(0xa11100, 0xa11101).noprw();                           // ?
//  map(0xa10008, 0xa1000d).nopw();                            // ? once
//  map(0xa14000, 0xa14003).nopw();                            // ? once
	map(0xa11200, 0xa11201).nopw();                            // ?
}

void puckpkmn_state::puckpkmn_map(address_map &map)
{
	puckpkmnb_map(map);

	map(0x4b2476, 0x4b2477).lr16(NAME([] () { return uint16_t(0x3100); }));
	map(0x70001c, 0x70001d).lr16(NAME([] () { return uint16_t(0x000e); }));
}

void puckpkmn_state::jingling_map(address_map &map)
{
	puckpkmnb_map(map);

	map(0x4b2476, 0x4b2477).lr16(NAME([] () { return uint16_t(0x3400); }));
	map(0x70001c, 0x70001d).lr16(NAME([] () { return uint16_t(0x000e); }));
}


void jzth_state::jzth_map(address_map &map)
{
	puckpkmn_base_map(map);

	map(0x710000, 0x710001).rw(FUNC(jzth_state::bl_710000_r), FUNC(jzth_state::bl_710000_w)); // protection, will erase the VDP address causing writes to 0 unless this returns 0xe
}

void songjang_state::songjang_map(address_map &map)
{
	puckpkmn_base_map(map);

	map(0x01c000, 0x01cfff).nopw(); // writes to ROM area, buggy code?

	// fallthrough for unhandled cases, as the protection in this area just seems to expect readbacks of data written
	// but at a different address, maybe hardcoded per game in one of the custom programmed chips?
	map(0x400000, 0x4fffff).rw(FUNC(songjang_state::unhandled_protval_r), FUNC(songjang_state::unhandled_protval_w));

	// songjang
	map(0x412302, 0x412303).w(FUNC(songjang_state::protval_w)); // used with 0x432100 read
	map(0x426800, 0x426801).r(FUNC(songjang_state::protval_r)); // used with 0x468202 write
	map(0x432100, 0x432101).r(FUNC(songjang_state::protval_r)); // used with 0x412302 write
	map(0x468202, 0x468203).w(FUNC(songjang_state::protval_w)); // used with 0x426800 read

	// shuifeng
	map(0x45bdb2, 0x45bdb3).w(FUNC(songjang_state::protval_w)); // used with 0x465461 read
	map(0x45bdf2, 0x45bdf3).w(FUNC(songjang_state::protval_w)); // used with 0x465461 read, when exiting service mode, but colours go bad anyway?

	map(0x465460, 0x465461).r(FUNC(songjang_state::protval_r)); // used with 0x45bdb3 write
	map(0x467470, 0x467471).r(FUNC(songjang_state::protval_r)); // used with 0x45bdb3 write

	// both
	map(0x70001c, 0x70001d).r(FUNC(songjang_state::sj_70001c_r));
}

/*************************************
 *
 *  Games memory handlers
 *
 *************************************/

// Juezhan Tiānhuáng protection
void jzth_state::bl_710000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// protection value is read from  0x710000 after a series of writes.. and stored at ff0007
	// startup
	/*
	059ce0 writing to bl_710000_w ff08 ffff
	059d04 writing to bl_710000_w 000a ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000c ffff
	059d04 writing to bl_710000_w 000f ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0xe)
	059ce0 writing to bl_710000_w ff08 ffff
	059d04 writing to bl_710000_w 000a ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000c ffff
	059d04 writing to bl_710000_w 000f ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0xe)
	*/
	// before lv stage 3
	/*
	059ce0 writing to bl_710000_w 0008 ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000f ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0x4)
	*/
	// start level 3
	/*
	059ce0 writing to bl_710000_w ff08 ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000c ffff
	059d04 writing to bl_710000_w 000e ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0x5)

	// after end sequence
	059ce0 writing to bl_710000_w 0008 ffff
	059d04 writing to bl_710000_w 000a ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000c ffff
	059d04 writing to bl_710000_w 000f ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0xe)
	*/

	logerror("%s: writing to bl_710000_w %04x %04x\n", machine().describe_context(), data, mem_mask);

	m_protcount++;
}

uint16_t jzth_state::bl_710000_r()
{
	logerror("%s: reading from bl_710000_r\n", machine().describe_context());

	uint16_t ret;
	switch (m_protcount)
	{
	case 4: ret = 0x4; break;
	case 5: ret = 0x5; break;
	case 6: ret = 0xe; break;
	default: ret = 0xf;
	}

	if (!machine().side_effects_disabled())
		m_protcount = 0;

	return ret;
}

uint16_t songjang_state::unhandled_protval_r()
{
	popmessage("%s: unhandled protval_r\n", machine().describe_context());
	return m_protval;
}

void songjang_state::unhandled_protval_w(offs_t offset, uint16_t data)
{
	popmessage("%s: unhandled protval_w %08x %04x\n", machine().describe_context(), 0x400000 + (offset * 2) , data);
	m_protval = data & 0xff00;
}

uint16_t songjang_state::protval_r()
{
	logerror("%s: protval_r\n", machine().describe_context());
	return m_protval;
}

void songjang_state::protval_w(uint16_t data)
{
	logerror("%s: protval_w %04x\n", machine().describe_context(), data);
	m_protval = data & 0xff00;
}

uint16_t songjang_state::sj_70001c_r()
{
	logerror("%s: reading from sj_70001c_r\n", machine().describe_context());
	return 0x0002;
}

uint16_t shuifeng_state::sj_70001c_r()
{
	logerror("%s: reading from sj_70001c_r\n", machine().describe_context());
	return 0x0031;
}

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

INPUT_PORTS_START( ybox_common )
	PORT_INCLUDE( md_common )

	// the normal pad inputs aren't used for gameplay on these games
	PORT_MODIFY("PAD1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// Puckman Pockimon Input Ports
INPUT_PORTS_START( puckpkmn )
	PORT_INCLUDE( ybox_common )

	PORT_START("P2")    // $700011.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("P1")    // $700013.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("UNK")   // $700015.b

	PORT_START("DSW1")  // $700017.b
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x28, DEF_STR( Lives ) )  PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSW2")  // $700019.b
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")
INPUT_PORTS_END


// Juézhàn Tiānhuáng inputs
INPUT_PORTS_START( jzth )
	PORT_INCLUDE( ybox_common )

	PORT_START("P2")    // $700011.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P1")    // $700013.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("UNK")   // $700015.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  // $700017.b
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x28, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSW2")  // $700019.b
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")
INPUT_PORTS_END


INPUT_PORTS_START( songjang )
	PORT_INCLUDE( jzth )

	PORT_MODIFY("UNK")   // $700015.b - P1/P2 swapped compared to jzth
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW2")  // $700019.b
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2") // no Demo Sound dip on this one
INPUT_PORTS_END

/*************************************
 *
 *  Machine Configuration
 *
 *************************************/

void jzth_state::machine_start()
{
	puckpkmn_state::machine_start();

	m_protcount = 0;

	save_item(NAME(m_protcount));
}

void songjang_state::machine_start()
{
	puckpkmn_state::machine_start();
	m_protval = 0;
	save_item(NAME(m_protval));
}

void puckpkmn_state::puckpkmn(machine_config &config)
{
	md_ntsc(config);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_ioports[2]->set_in_handler(NAME([this] () { return m_io_exp.read_safe(0x3f); }));

	m_maincpu->set_addrmap(AS_PROGRAM, &puckpkmn_state::puckpkmn_map);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(4'000'000) / 4, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "speaker", 0.25, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 0.25, 1);

	// is there a 2nd YM? there seems to be one external to the SoC as well as the one assumed to be in the SoC
}

void puckpkmn_state::jingling(machine_config &config)
{
	puckpkmn(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &puckpkmn_state::jingling_map);
}

void puckpkmn_state::puckpkmnb(machine_config &config)
{
	puckpkmn(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &puckpkmn_state::puckpkmnb_map);
}


void jzth_state::jzth(machine_config &config)
{
	puckpkmn(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jzth_state::jzth_map);
}

void songjang_state::songjang(machine_config &config)
{
	puckpkmn(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &songjang_state::songjang_map);
}

/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

void puckpkmn_state::init_puckpkmn()
{
	init_megadrij();

	uint8_t *const rom = memregion("maincpu")->base();
	size_t const len = memregion("maincpu")->bytes();

	for (size_t i = 0; i < len; i++)
		rom[i] = bitswap<8>(rom[i], 1, 4, 2, 0, 7, 5, 3, 6);

	m_maincpu->set_tas_write_callback(*this, FUNC(puckpkmn_state::megadriv_tas_callback));

	// TODO: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

// Puckman Pockimon  (c)2000 Sun Mixing
ROM_START( puckpkmn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "200061.u5", 0x000000, 0x080000, CRC(502a5093) SHA1(6dc1c79d52ebb653cb2e4388f74fd975ec323566) )
	ROM_LOAD16_BYTE( "200060.u4", 0x000001, 0x080000, CRC(5f160c18) SHA1(5a5ce1b9a81afe836e435e9d6f16cf57b63cbd31) )
	ROM_LOAD16_BYTE( "200063.u8", 0x100000, 0x080000, CRC(0c29781e) SHA1(db442f9b588608b2ac04d65fd830103296599a6a) )
	ROM_LOAD16_BYTE( "200062.u7", 0x100001, 0x080000, CRC(00bbf9a9) SHA1(924c1ed85090c497ce89528082c15d1548a854a0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "206295.u3", 0x00000, 0x40000, CRC(7b066bac) SHA1(429616e21c672b07e0705bc63234249cac3af56f) )
ROM_END


/*
精靈家族/Jīnglíng Jiāzú (Traditional Chinese)
(c)2000 IBS Co. Ltd


PCB Layout
----------

|------------------------------------------------|
|UPC1241                      4.000MHz     6264  |
|    VOL             6295           555          |
|                                                |
|                                                |
|  LM324                                TV16B    |
|                    A.U3                        |
|                                                |
|                                                |
|J                                               |
|A                                   59.693175MHz|
|M                                               |
|M      DSW2(8)                                  |
|A                                               |
|                    62256    62256              |
|       DSW1(8)      PAL      PAL                |
|                                                |
|                         B.U59          TA-06S  |
|  PAL               TK-20K   PAL                |
|                             PAL       MB81461  |
|  PAL     PAL                          MB81461  |
|------------------------------------------------|
Notes:
      TV16B  - custom graphics chip (QFP160)
      TA-06S - custom chip (QFP128)
      TK-20K - custom chip, probably the CPU (QFP100). Clock unknown.
      M6295  - clock 1.000MHz [4/4]

      4x 1Mx8 Flash ROMs (B*.U59) are mounted onto a DIP42 carrier board to make a
      32MBit EPROM equivalent. It appears to contain graphics plus the main program.
      ROM A.U3 contains samples for the M6295.

*/

ROM_START( jingling )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b2.u59", 0x000000, 0x080000, CRC(3fbea2c7) SHA1(89f3770ae92c62714f0795ddd2f311a9532eb25a) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_IGNORE(0x080000)
	ROM_LOAD16_BYTE( "b1.u59", 0x000001, 0x080000, CRC(dc7b4254) SHA1(8ba5c5e8123e62e9af091971d0d0401d2df49350) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_IGNORE(0x080000)
	ROM_LOAD16_BYTE( "b4.u59", 0x100000, 0x080000, CRC(375c9f80) SHA1(9b0eb729e95c22355e4117eec596f90e10282492) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_IGNORE(0x080000)
	ROM_LOAD16_BYTE( "b3.u59", 0x100001, 0x080000, CRC(d5487df6) SHA1(d1d3d717e184a4e8e067665bbbe94e7cf45db478) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_IGNORE(0x080000)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "a.u3", 0x00000, 0x80000, CRC(77891c9b) SHA1(66f28b418a480a89ddb3fae3a7c2fe702c62364c) )
ROM_END


// Puckman Pockimon (no copyright, presumably a bootleg of the Sun Mixing version)
ROM_START( puckpkmnb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "puckpoke.u5", 0x000000, 0x080000, CRC(fd334b91) SHA1(cf8bf6645a4082ea4392937e169b1686c9c7e246) )
	ROM_LOAD16_BYTE( "puckpoke.u4", 0x000001, 0x080000, CRC(839cc76b) SHA1(e15662a7175db7a8e222dda176a8ed92e0d56e9d) )
	ROM_LOAD16_BYTE( "puckpoke.u8", 0x100000, 0x080000, CRC(7936bec8) SHA1(4b350105abe514fbfeabae1c6f3aeee695c3d07a) )
	ROM_LOAD16_BYTE( "puckpoke.u7", 0x100001, 0x080000, CRC(96b66bdf) SHA1(3cc2861ad9bc232cbe683e01b58090f832d03db5) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "puckpoke.u3", 0x00000, 0x40000, CRC(7b066bac) SHA1(429616e21c672b07e0705bc63234249cac3af56f) )
ROM_END


//決戰天皇/Juézhàn Tiānhuáng (Traditional Chinese)
ROM_START( jzth )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s.y.u5", 0x000000, 0x080000, CRC(a4a526b5) SHA1(85d0299caf91ff50b6870f845b9aacbd358ed81f) )
	ROM_LOAD16_BYTE( "s.y.u4", 0x000001, 0x080000, CRC(c16654eb) SHA1(dca4b772a3b9caa7be3fa01511c401b591c2e6f3) )
	ROM_LOAD16_BYTE( "s.y.u8", 0x100000, 0x080000, CRC(b62e1068) SHA1(2484ae49a4a2a2c551b3b84bbc0b4e40e5d281e7) )
	ROM_LOAD16_BYTE( "s.y.u7", 0x100001, 0x080000, CRC(27fe424c) SHA1(14bee8c16aac3d5b04123c994167531f817634fd) )
	ROM_LOAD16_BYTE( "s.y.u64", 0x200000, 0x080000, CRC(62f52886) SHA1(07fc9765274c03eff4a09f48a0b1b2b2afc6078e) )
	ROM_LOAD16_BYTE( "s.y.u63", 0x200001, 0x080000, CRC(a6a32c8c) SHA1(d0c779751e4af459e9bf63e55c5e2b19a243b70d) )
	ROM_LOAD16_BYTE( "s.y.u66", 0x300000, 0x080000, CRC(fa4a09f5) SHA1(67d77c91a994ecb8b29e7661c3a12e84a64eb837))
	ROM_LOAD16_BYTE( "s.y.u65", 0x300001, 0x080000, CRC(de64e526) SHA1(e3b3e5c95b8ae36c0c57f8c9a6f55084464c4c05) )

	ROM_REGION( 0x80000, "oki", 0 ) // there are 2 banks in here, so find bank switch
	ROM_LOAD( "s.y.u3", 0x00000, 0x40000, CRC(38eef2f2) SHA1(2f750dbf71fea0622e8493f0a8be7c43555ed5cf) )
	ROM_CONTINUE(0x40000,0x40000)
ROM_END

ROM_START( songjang )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom.u5", 0x000000, 0x080000, CRC(3de4aea5) SHA1(a4fd0799a6fd86ba3f0c4b4277190f4c332e48d6) )
	ROM_LOAD16_BYTE( "rom.u4", 0x000001, 0x080000, CRC(dc6cfc29) SHA1(e15d6e2187befbbd791c5046cb6d156801095203) )
	ROM_LOAD16_BYTE( "rom.u8", 0x100000, 0x080000, CRC(d4d2b262) SHA1(2023128b187a8cd1a3f021861da559b5ad48e984) )
	ROM_LOAD16_BYTE( "rom.u7", 0x100001, 0x080000, CRC(1e24f2c9) SHA1(562249b08a8fe317a606c16ce07a8009a0c5418a) )

	ROM_REGION( 0x80000, "oki", 0 ) // only one bank (2nd half is blank)
	ROM_LOAD( "rom.u3", 0x00000, 0x80000, CRC(44287ab4) SHA1(42de2010e73c036a2a0e01c9dd3c1c8aef1a54a1) )
ROM_END

ROM_START( shuifeng )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "shuihufengyunzhuan.u5", 0x000000, 0x080000, CRC(73208a1a) SHA1(1eff253fe48f9e553a238c4cfd41cc5e5839393f) )
	ROM_LOAD16_BYTE( "shuihufengyunzhuan.u4", 0x000001, 0x080000, CRC(e633c02c) SHA1(7ff1091a5b2e7d5e0e616509854fa0058d3cdcf7) )
	ROM_LOAD16_BYTE( "shuihufengyunzhuan.u8", 0x100000, 0x080000, CRC(31814eb0) SHA1(3a6b13931d02a0c3b8bd97dc51ac5771a97bd092) )
	ROM_LOAD16_BYTE( "shuihufengyunzhuan.u7", 0x100001, 0x080000, CRC(51caef9f) SHA1(252dbb29ae878aaf28cb4586e02d823296eaf378) )

	ROM_REGION( 0x80000, "oki", 0 ) // only one bank (2nd half is blank)
	ROM_LOAD( "shuihufengyunzhuan-oki.u3", 0x00000, 0x80000, CRC(44287ab4) SHA1(42de2010e73c036a2a0e01c9dd3c1c8aef1a54a1) )
ROM_END

} // anonymous namespace

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

// Genie Hardware (uses Genesis VDP) also has 'Sun Mixing Co' put into tile RAM
// Is 'Genie 2000' part of the title, and the parent set a bootleg?
GAME( 2000, puckpkmn,  0,        puckpkmn,  puckpkmn, puckpkmn_state, init_puckpkmn, ROT0, "Sun Mixing",  "Puckman Pockimon Genie 2000",           0 )
GAME( 2000, jingling,  puckpkmn, jingling,  puckpkmn, puckpkmn_state, init_puckpkmn, ROT0, "IBS Co. Ltd", "Jingling Jiazu Genie 2000",             0 )
GAME( 2000, puckpkmnb, puckpkmn, puckpkmnb, puckpkmn, puckpkmn_state, init_puckpkmn, ROT0, "bootleg",     "Puckman Pockimon Genie 2000 (bootleg)", 0 )
GAME( 2000, jzth,      0,        jzth,      jzth,     jzth_state,     init_puckpkmn, ROT0, "<unknown>",   "Juezhan Tianhuang",                     MACHINE_IMPERFECT_SOUND )

// some corruption on cutscenes between levels, emulation bug or glitch in the original conversion?
GAME( 200?, songjang,  0,        songjang,  songjang, songjang_state, init_puckpkmn, ROT0, "WAH LAP",     "Songjiangyanyi Final",                  MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, shuifeng,  0,        songjang,  jzth,     shuifeng_state, init_puckpkmn, ROT0, "WAH LAP",     "Shuihu Feng Yun Zhuan",                 MACHINE_IMPERFECT_GRAPHICS )
