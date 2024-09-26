// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

PET = Personal Electronic Transactor


http://www.6502.org/users/andre/petindex/boards.html

Static Board (PET 2001)
-----------------------

Four variations based on type of RAM(6550 or 2114) and ROM(6540 or 2316B).
4K or 8K static RAM (selected by jumper).
40 column display
A video interrupt interferes with disk drive operation.
Display timing not compatible with Basic 4.0.
ROM sockets:  A2  2K character      ROM sockets:  A2  2K character
 (2316B)      H1  C000-CFFF           (6540)       H1  C000-C7FF
              H2  D000-DFFF                        H2  D000-D7FF
              H3  E000-E7FF                        H3  E000-E7FF
              H4  F000-FFFF                        H4  F000-F7FF
              H5  C000-CFFF                        H5  C800-CFFF
              H6  D000-DFFF                        H6  D800-DFFF
              H7  F000-FFFF                        H7  F800-FFFF


           IEEE user tape #2
     +------####-####--##-+
     !                    #
     !                    #
     !                    # exp
     !                    # bus
     !                    #
     !                    #    2000 Series
     !                    !       circa 1977/78  Max RAM - 8k
     !       (2k) ROMS    !       [w/daughter board exp to 32k shown]
     !      F F E D D C C !
     !      8 0 0 8 0 8 0 !
     !                    !
tape #       RAM MEMORY   !
 #1  #                    !
     +--------------------+


Dynamic Board (PET/CBM 2001-N/2001-B/4000)
------------------------------------------

4K, 8K, 16K or 32K dynamic RAM (selected by jumper).
40 column display
Can run all versions of 40 column Basic (Basic 1 must be copied to 4K ROMs)
Can be jumpered to replace the older board.
ROM sockets:  UD3   9000-9FFF
              UD4   A000-AFFF
              UD5   B000-BFFF
              UD6   C000-CFFF
              UD7   D000-DFFF
              UD8   E000-E7FF
              UD9   F000-FFFF
              UF10  2K character


            IEEE user tape #1
     +------####-####--##-+
     !                   #!
     !                   #!
     !                   #! exp
     !        ROMS       #! bus
     !    F E D C B A 9  #!
     !                   #!    3000, 4000 Series
     !                    !       (3000 series is European version)
     !                    !       circa 1979/80  Max RAM - 32k
     !                    !
     !                    !
     !                    !
tape #      RAM MEMORY    !
 #2  #                    !
     +--------------------+


80 Column Board (CBM 8000)
--------------------------

16K or 32K RAM (selected by jumper).
Uses CTRC to generate 80 column display.
Can only run the 80 column version of Basic 4.0.
Not compatible with older boards.
ROM sockets:  UA3   2K or 4K character
              UD6   F000-FFFF
              UD7   E000-E7FF
              UD8   D000-DFFF
              UD9   C000-CFFF
              UD10  B000-BFFF
              UD11  A000-AFFF
              UD12  9000-9FFF

The layout is the same of the one used in Universal Boards below.


Universal Board (CBM 8000/PET 4000-12)
--------------------------------------

This is an 80 column board with jumpers for different configurations.
16K or 32K RAM (selected by jumper).
Uses CTRC to generate 40 or 80 column display (selected by jumpers).
Can only run Basic 4.0 versions that support the CRTC.
Can be jumpered to replace all older boards.
ROM sockets:  UA3   2K or 4K character
              UD6   F000-FFFF
              UD7   E000-E7FF
              UD8   D000-DFFF
              UD9   C000-CFFF
              UD10  B000-BFFF
              UD11  A000-AFFF
              UD12  9000-9FFF


           IEEE user tape #1
     +------####-####--##-+
     !                  # # tape
     !                  # #  #2
     !  R       exp bus # !
     !  A                #!
     !  M             9  #!
     !                A  #!     4000, 8000 Series
     !  M          R  B   !        circa 1981     Max RAM - 32k*
     !  E          O  C   !       [8296 layout not shown]
     !  M          M  D   !
     !  O          S  E   !
     !  R             F   !
     !  Y                 !
     !                spkr!
     +--------------------+
*/

/*

    TODO:

    - accurate video timing for non-CRTC models
    - High Speed Graphics board
    - keyboard layouts
        - Swedish
        - German
    - SuperPET
        - 6809
        - OS/9 MMU
    - 8296
        - PLA dumps
        - high resolution graphics
        - Malvern Particle Sizer OEM variant

*/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/ieee488/c2040.h"
#include "bus/ieee488/c8050.h"
#include "bus/ieee488/ieee488.h"
#include "bus/pet/cass.h"
#include "bus/pet/exp.h"
#include "bus/pet/user.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/snapquik.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "cbm_snqk.h"
#include "machine/input_merger.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"


namespace {

#define M6502_TAG       "f3"
#define M6522_TAG       "a5"
#define M6520_1_TAG     "g8"
#define M6520_2_TAG     "b8"
#define MC6845_TAG      "ub13"
#define SCREEN_TAG      "screen"
#define PLA1_TAG        "ue6"
#define PLA2_TAG        "ue5"
#define PET_USER_PORT_TAG "user"

class pet_state : public driver_device
{
public:
	pet_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M6502_TAG),
		m_via(*this, M6522_TAG),
		m_pia1(*this, M6520_1_TAG),
		m_pia2(*this, M6520_2_TAG),
		m_crtc(*this, MC6845_TAG),
		m_screen(*this, SCREEN_TAG),
		m_ieee(*this, IEEE488_TAG),
		m_palette(*this, "palette"),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_cassette2(*this, PET_DATASSETTE_PORT2_TAG),
		m_exp(*this, "exp"),
		m_user(*this, PET_USER_PORT_TAG),
		m_speaker(*this, "speaker"),
		m_cart_9000(*this, "cart_9000"),
		m_cart_a000(*this, "cart_a000"),
		m_cart_b000(*this, "cart_b000"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, M6502_TAG),
		m_char_rom(*this, "charom"),
		m_video_ram(*this, "video_ram", 0x800, ENDIANNESS_LITTLE),
		m_row(*this, "ROW%u", 0),
		m_lock(*this, "LOCK"),
		m_sync_timer(nullptr),
		m_sync_period(attotime::zero),
		m_key(0),
		m_sync(0),
		m_graphic(0),
		m_blanktv(0),
		m_user_diag(1)
	{ }

	void base_pet_devices(machine_config &config, const char *default_drive);
	void _4k(machine_config &config);
	void _8k(machine_config &config);
	void _16k(machine_config &config);
	void _32k(machine_config &config);
	void pet(machine_config &config);
	void pet2001n(machine_config &config, bool with_b000 = true);
	void pet2001n8(machine_config &config);
	void cbm3032(machine_config &config);
	void pet20018(machine_config &config);
	void pet2001n16(machine_config &config);
	void cbm3016(machine_config &config);
	void cbm3000(machine_config &config);
	void cbm4000(machine_config &config);
	void cbm4032f(machine_config &config);
	void cbm4032(machine_config &config);
	void cbm4016(machine_config &config);
	void cbm3008(machine_config &config);
	void pet2001(machine_config &config);
	void pet2001n32(machine_config &config);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void via_pa_w(uint8_t data);
	uint8_t via_pb_r();
	void via_pb_w(uint8_t data);
	void via_ca2_w(int state);
	void via_cb2_w(int state);

	uint8_t pia1_pa_r();
	uint8_t pia1_pb_r();
	void pia1_pa_w(uint8_t data);
	void pia1_ca2_w(int state);

	void user_diag_w(int state);

	MC6845_BEGIN_UPDATE( pet_begin_update );
	MC6845_UPDATE_ROW( pet40_update_row );

	TIMER_CALLBACK_MEMBER( sync_tick );

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_pet);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_MACHINE_START( pet40 );
	DECLARE_MACHINE_RESET( pet40 );

	void pet2001_mem(address_map &map) ATTR_COLD;

protected:
	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	optional_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<ieee488_device> m_ieee;
	required_device<palette_device> m_palette;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<pet_datassette_port_device> m_cassette2;
	required_device<pet_expansion_slot_device> m_exp;
	required_device<pet_user_port_device> m_user;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<generic_slot_device> m_cart_9000;
	optional_device<generic_slot_device> m_cart_a000;
	optional_device<generic_slot_device> m_cart_b000;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	memory_share_creator<uint8_t> m_video_ram;
	required_ioport_array<10> m_row;
	required_ioport m_lock;

	emu_timer *m_sync_timer;
	attotime m_sync_period;

	DECLARE_MACHINE_START( pet );
	DECLARE_MACHINE_START( pet2001 );
	DECLARE_MACHINE_RESET( pet );

	void update_speaker();

	enum
	{
		SEL0 = 0,
		SEL1,
		SEL2,
		SEL3,
		SEL4,
		SEL5,
		SEL6,
		SEL7,
		SEL8,
		SEL9,
		SELA,
		SELB,
		SELC,
		SELD,
		SELE,
		SELF
	};

	// keyboard state
	uint8_t m_key;

	// video state
	int m_sync;
	int m_graphic;
	int m_blanktv;
	int m_video_ram_size;

	// sound state
	int m_via_cb2;
	int m_pia1_pa7;

	uint8_t m_via_pa;

	int m_user_diag;
};


class pet2001b_state : public pet_state
{
public:
	pet2001b_state(const machine_config &mconfig, device_type type, const char *tag) :
		pet_state(mconfig, type, tag)
	{ }

	void pet2001b(machine_config &config, bool with_b000 = true);
	void pet2001b32(machine_config &config);
	void pet4000(machine_config &config);
	void pet4000b(machine_config &config);
	void pet4001b(machine_config &config);
	void pet4032f(machine_config &config);
	void pet4032b(machine_config &config);
	void pet4032(machine_config &config);
	void pet2001b16(machine_config &config);
	void cbm3032b(machine_config &config);
	void cbm4000b(machine_config &config);
	void cbm4032b(machine_config &config);
	void pet2001b8(machine_config &config);
	void pet4016(machine_config &config);

protected:
	uint8_t pia1_pb_r();

};


class pet80_state : public pet2001b_state
{
public:
	pet80_state(const machine_config &mconfig, device_type type, const char *tag) :
		pet2001b_state(mconfig, type, tag)
	{ }

	void pet80(machine_config &config);
	void pet8032(machine_config &config);

	DECLARE_MACHINE_START( pet80 );
	DECLARE_MACHINE_RESET( pet80 );

	MC6845_UPDATE_ROW( cbm8296_update_row );

protected:
	MC6845_UPDATE_ROW( pet80_update_row );
};


class superpet_state : public pet80_state
{
public:
	superpet_state(const machine_config &mconfig, device_type type, const char *tag)
		: pet80_state(mconfig, type, tag)
	{ }
	void superpet(machine_config &config);
};


class cbm8096_state : public pet80_state
{
public:
	cbm8096_state(const machine_config &mconfig, device_type type, const char *tag) :
		pet80_state(mconfig, type, tag)
	{ }
	void cbm8096(machine_config &config);
};


class cbm8296_state : public pet80_state
{
public:
	cbm8296_state(const machine_config &mconfig, device_type type, const char *tag) :
		pet80_state(mconfig, type, tag),
		m_basic_rom(*this, "basic"),
		m_editor_rom(*this, "editor"),
		m_ue5_rom(*this, "ue5_eprom"),
		m_ue6_rom(*this, "ue6_eprom"),
		m_pla1(*this, PLA1_TAG),
		m_pla2(*this, PLA2_TAG)
	{ }

	void cbm8296d(machine_config &config);
	void cbm8296(machine_config &config);

private:
	required_memory_region m_basic_rom;
	required_memory_region m_editor_rom;
	required_memory_region m_ue5_rom;
	required_memory_region m_ue6_rom;
	required_device<pla_device> m_pla1;
	required_device<pla_device> m_pla2;

	DECLARE_MACHINE_START( cbm8296 );
	DECLARE_MACHINE_RESET( cbm8296 );

	[[maybe_unused]] void read_pla1(offs_t offset, int phi2, int brw, int noscreen, int noio, int ramsela, int ramsel9, int ramon, int norom,
						int &cswff, int &cs9, int &csa, int &csio, int &cse, int &cskb, int &fa12, int &casena1);
	[[maybe_unused]] void read_pla2(offs_t offset, int phi2, int brw, int casena1, int &endra, int &noscreen, int &casena2, int &fa15);

	void read_pla1_eprom(offs_t offset, int phi2, int brw, int noscreen, int noio, int ramsela, int ramsel9, int ramon, int norom,
		int &cswff, int &cs9, int &csa, int &csio, int &cse, int &cskb, int &fa12, int &casena1);
	void read_pla2_eprom(offs_t offset, int phi2, int brw, int casena1, int &endra, int &noscreen, int &casena2, int &fa15);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t m_cr;
	void cbm8296_mem(address_map &map) ATTR_COLD;
};


static void cbm_pet_quick_sethiaddress( address_space &space, uint16_t hiaddress )
{
	space.write_byte(0x2e, hiaddress & 0xff);
	space.write_byte(0x2c, hiaddress & 0xff);
	space.write_byte(0x2a, hiaddress & 0xff);
	space.write_byte(0x2f, hiaddress >> 8);
	space.write_byte(0x2d, hiaddress >> 8);
	space.write_byte(0x2b, hiaddress >> 8);
}

QUICKLOAD_LOAD_MEMBER(pet_state::quickload_pet)
{
	return general_cbm_loadsnap(image, m_maincpu->space(AS_PROGRAM), 0, cbm_pet_quick_sethiaddress);
}



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  update_speaker -
//-------------------------------------------------

void pet_state::update_speaker()
{
	if (m_speaker)
	{
		int level = m_via_cb2 && m_pia1_pa7;

		m_speaker->level_w(level);
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t pet_state::read(offs_t offset)
{
	int sel = offset >> 12;
	int norom = m_exp->norom_r(offset, sel);
	uint8_t data = 0;

	data = m_exp->read(offset, data, sel);

	switch (sel)
	{
	case SEL0: case SEL1: case SEL2: case SEL3: case SEL4: case SEL5: case SEL6: case SEL7:
		if (offset < m_ram->size())
		{
			data = m_ram->pointer()[offset];
		}
		break;

	case SEL8:
		if (!(offset & 0x800))
		{
			data = m_video_ram[offset & (m_video_ram_size - 1)];
		}
		break;

	case SEL9:
		if (norom)
		{
			if (m_cart_9000 && m_cart_9000->exists())
				data = m_cart_9000->read_rom(offset & 0xfff);
			else
				data = m_rom->base()[offset - 0x9000];
		}
		break;

	case SELA:
		if (norom)
		{
			if (m_cart_a000 && m_cart_a000->exists())
				data = m_cart_a000->read_rom(offset & 0xfff);
			else
				data = m_rom->base()[offset - 0x9000];
		}
		break;

	case SELB:
		if (norom)
		{
			if (m_cart_b000 && m_cart_b000->exists())
				data = m_cart_b000->read_rom(offset & 0xfff);
			else
				data = m_rom->base()[offset - 0x9000];
		}
		break;

	case SELC: case SELD: case SELF:
		if (norom)
		{
			data = m_rom->base()[offset - 0x9000];
		}
		break;

	case SELE:
		if (BIT(offset, 11))
		{
			data = 0xff;

			if (BIT(offset, 4))
			{
				data &= m_pia1->read(offset & 0x03);
			}
			if (BIT(offset, 5))
			{
				data &= m_pia2->read(offset & 0x03);
			}
			if (BIT(offset, 6))
			{
				data &= m_via->read(offset & 0x0f);
			}
			if (m_crtc && BIT(offset, 7) && BIT(offset, 0))
			{
				data &= m_crtc->register_r();
			}
		}
		else if (norom)
		{
			data = m_rom->base()[offset - 0x9000];
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void pet_state::write(offs_t offset, uint8_t data)
{
	int sel = offset >> 12;

	m_exp->write(offset, data, sel);

	switch (sel)
	{
	case SEL0: case SEL1: case SEL2: case SEL3: case SEL4: case SEL5: case SEL6: case SEL7:
		if (offset < m_ram->size())
		{
			m_ram->pointer()[offset] = data;
		}
		break;

	case SEL8:
		if (!(offset & 0x800))
		{
			m_video_ram[offset & (m_video_ram_size - 1)] = data;
		}
		break;

	case SELE:
		if (BIT(offset, 11))
		{
			if (BIT(offset, 4))
			{
				m_pia1->write(offset & 0x03, data);
			}
			if (BIT(offset, 5))
			{
				m_pia2->write(offset & 0x03, data);
			}
			if (BIT(offset, 6))
			{
				m_via->write(offset & 0x0f, data);
			}
			if (m_crtc && BIT(offset, 7))
			{
				if (BIT(offset, 0))
				{
					m_crtc->register_w(data);
				}
				else
				{
					m_crtc->address_w(data);
				}
			}
		}
		break;
	}
}

//-------------------------------------------------
//  read_pla1 -
//-------------------------------------------------

void cbm8296_state::read_pla1(offs_t offset, int phi2, int brw, int noscreen, int noio, int ramsela, int ramsel9, int ramon, int norom,
	int &cswff, int &cs9, int &csa, int &csio, int &cse, int &cskb, int &fa12, int &casena1)
{
	uint32_t input = (offset & 0xff00) | phi2 << 7 | brw << 6 | noscreen << 5 | noio << 4 | ramsela << 3 | ramsel9 << 2 | ramon << 1 | norom;
	uint32_t data = m_pla1->read(input);

	cswff = BIT(data, 0);
	cs9 = BIT(data, 1);
	csa = BIT(data, 2);
	csio = BIT(data, 3);
	cse = BIT(data, 4);
	cskb = BIT(data, 5);
	fa12 = BIT(data, 6);
	casena1 = BIT(data, 7);
}

void cbm8296_state::read_pla1_eprom(offs_t offset, int phi2, int brw, int noscreen, int noio, int ramsela, int ramsel9, int ramon, int norom,
	int &cswff, int &cs9, int &csa, int &csio, int &cse, int &cskb, int &fa12, int &casena1)
{
	// PLA-EPROM adapter by Joachim Nemetz (Jogi)

	uint32_t input = (offset & 0xff00) | phi2 << 7 | brw << 6 | noscreen << 5 | noio << 4 | ramsela << 3 | ramsel9 << 2 | ramon << 1 | norom;
	input = bitswap<16>(input,13,8,9,7,12,14,11,10,6,5,4,3,2,1,0,15);

	uint8_t data = m_ue6_rom->base()[input];
	data = bitswap<8>(data,7,0,1,2,3,4,5,6);

	cswff = BIT(data, 0);
	cs9 = BIT(data, 1);
	csa = BIT(data, 2);
	csio = BIT(data, 3);
	cse = BIT(data, 4);
	cskb = BIT(data, 5);
	fa12 = BIT(data, 6);
	casena1 = BIT(data, 7);
}


//-------------------------------------------------
//  read_pla2 -
//-------------------------------------------------

void cbm8296_state::read_pla2(offs_t offset, int phi2, int brw, int casena1, int &endra, int &noscreen, int &casena2, int &fa15)
{
	uint32_t input = bitswap<8>(m_cr, 0,1,2,3,4,5,6,7) << 8 | ((offset >> 8) & 0xf8) | brw << 2 | phi2 << 1 | casena1;
	uint32_t data = m_pla2->read(input);

	endra = BIT(data, 4);
	noscreen = BIT(data, 5);
	casena2 = BIT(data, 6);
	fa15 = BIT(data, 7);
}

void cbm8296_state::read_pla2_eprom(offs_t offset, int phi2, int brw, int casena1, int &endra, int &noscreen, int &casena2, int &fa15)
{
	// PLA-EPROM adapter by Joachim Nemetz (Jogi)

	uint32_t input = bitswap<8>(m_cr, 0,1,2,3,4,5,6,7) << 8 | ((offset >> 8) & 0xf8) | brw << 2 | phi2 << 1 | casena1;
	input = bitswap<16>(input,13,8,9,7,12,14,11,10,6,5,4,3,2,1,0,15);

	uint8_t data = m_ue5_rom->base()[input];
	data = bitswap<8>(data,7,0,1,2,3,4,5,6);

	endra = BIT(data, 4);
	noscreen = BIT(data, 5);
	casena2 = BIT(data, 6);
	fa15 = BIT(data, 7);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t cbm8296_state::read(offs_t offset)
{
	int norom = m_exp->norom_r(offset, offset >> 12) && !BIT(m_cr, 7);
	int phi2 = 1, brw = 1, noscreen = 1, noio = BIT(m_cr, 6);
	int ramsela = BIT(m_via_pa, 0), ramsel9 = BIT(m_via_pa, 1), ramon = BIT(m_via_pa, 2);
	int cswff = 1, cs9 = 1, csa = 1, csio = 1, cse = 1, cskb = 1, fa12 = 1, fa15 = 1, casena1 = 1, casena2 = 1, endra = 1;

	read_pla1_eprom(offset, phi2, brw, noscreen, noio, ramsela, ramsel9, ramon, norom,
		cswff, cs9, csa, csio, cse, cskb, fa12, casena1);

	read_pla2_eprom(offset, phi2, brw, casena1, endra, noscreen, casena2, fa15);

	read_pla1_eprom(offset, phi2, brw, noscreen, noio, ramsela, ramsel9, ramon, norom,
		cswff, cs9, csa, csio, cse, cskb, fa12, casena1);

	//logerror("%s read  %04x : norom %u noio %u ramsela %u ramsel9 %u ramon %u / cswff %u cs9 %u csa %u csio %u cse %u cskb %u fa12 %u casena1 %u endra %u noscreen %u casena2 %u fa15 %u\n",machine().describe_context(),offset,norom,noio,ramsela,ramsel9,ramon,cswff,cs9,csa,csio,cse,cskb,fa12,casena1,endra,noscreen,casena2,fa15);

	uint8_t data = 0;

	offs_t drma = fa15 << 15 | (offset & 0x7e00) | BIT(offset, 0) << 8 | (offset & 0x1fe) >> 1;

	if (!endra && !casena1)
	{
		data = m_ram->pointer()[drma];
	}
	if (casena2)
	{
		data = m_ram->pointer()[0x10000 | drma];
	}
	if (!cs9)
	{
		if (m_cart_9000 && m_cart_9000->exists())
			data = m_cart_9000->read_rom(offset & 0xfff);
		else
			data = m_rom->base()[offset & 0xfff];
	}
	if (!csa)
	{
		if (m_cart_a000 && m_cart_a000->exists())
			data = m_cart_a000->read_rom(offset & 0xfff);
		else
			data = m_rom->base()[0x1000 | (offset & 0xfff)];
	}
	if (!cse)
	{
		data = m_editor_rom->base()[offset & 0xfff];
	}
	if (!cskb)
	{
		data = m_basic_rom->base()[(offset & 0x2fff) | fa12 << 12];
	}
	if (!csio)
	{
		data = 0xff;

		if (BIT(offset, 4))
		{
			data &= m_pia1->read(offset & 0x03);
		}
		if (BIT(offset, 5))
		{
			data &= m_pia2->read(offset & 0x03);
		}
		if (BIT(offset, 6))
		{
			data &= m_via->read(offset & 0x0f);
		}
		if (BIT(offset, 7) && BIT(offset, 0))
		{
			data &= m_crtc->register_r();
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void cbm8296_state::write(offs_t offset, uint8_t data)
{
	int norom = m_exp->norom_r(offset, offset >> 12) && !BIT(m_cr, 7);
	int phi2 = 1, brw = 0, noscreen = 1, noio = BIT(m_cr, 6);
	int ramsela = BIT(m_via_pa, 0), ramsel9 = BIT(m_via_pa, 1), ramon = BIT(m_via_pa, 2);
	int cswff = 1, cs9 = 1, csa = 1, csio = 1, cse = 1, cskb = 1, fa12 = 1, fa15 = 1, casena1 = 1, casena2 = 1, endra = 1;

	read_pla1_eprom(offset, phi2, brw, noscreen, noio, ramsela, ramsel9, ramon, norom,
		cswff, cs9, csa, csio, cse, cskb, fa12, casena1);

	read_pla2_eprom(offset, phi2, brw, casena1, endra, noscreen, casena2, fa15);

	read_pla1_eprom(offset, phi2, brw, noscreen, noio, ramsela, ramsel9, ramon, norom,
		cswff, cs9, csa, csio, cse, cskb, fa12, casena1);

	//logerror("%s write %04x : norom %u noio %u ramsela %u ramsel9 %u ramon %u / cswff %u cs9 %u csa %u csio %u cse %u cskb %u fa12 %u casena1 %u endra %u noscreen %u casena2 %u fa15 %u\n",machine().describe_context(),offset,norom,noio,ramsela,ramsel9,ramon,cswff,cs9,csa,csio,cse,cskb,fa12,casena1,endra,noscreen,casena2,fa15);

	offs_t drma = fa15 << 15 | (offset & 0x7e00) | BIT(offset, 0) << 8 | (offset & 0x1fe) >> 1;

	if (!endra && !casena1)
	{
		m_ram->pointer()[drma] = data;
	}
	if (casena2)
	{
		m_ram->pointer()[0x10000 | drma] = data;
	}
	if (!csio)
	{
		if (BIT(offset, 4))
		{
			m_pia1->write(offset & 0x03, data);
		}
		if (BIT(offset, 5))
		{
			m_pia2->write(offset & 0x03, data);
		}
		if (BIT(offset, 6))
		{
			m_via->write(offset & 0x0f, data);
		}
		if (BIT(offset, 7))
		{
			if (BIT(offset, 0))
			{
				m_crtc->register_w(data);
			}
			else
			{
				m_crtc->address_w(data);
			}
		}
	}
	if (cswff && ((offset & 0xff) == 0xf0))
	{
		m_cr = data;
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void pet_state::pet2001_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(pet_state::read), FUNC(pet_state::write));
}


void cbm8296_state::cbm8296_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(cbm8296_state::read), FUNC(cbm8296_state::write));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( pet )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Home  Clr Screen") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_MINUS) PORT_CHAR(0x2190)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('#')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('!')

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del  Inst") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('\\')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('$')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('"')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR('9')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR('7')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91 Pi") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('^') PORT_CHAR(0x03C0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR('/')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('W')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('D')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('A')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR('5')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('S')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR('3')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR(';')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('X')

	PORT_START( "ROW8" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR('-')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR('0')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL)  PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START( "ROW9" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR('.')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop Run") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('[')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Rvs Off") PORT_CODE(KEYCODE_TAB)

	PORT_START( "LOCK" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END


INPUT_PORTS_START( petb )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91") PORT_CODE(KEYCODE_DEL) PORT_CHAR('^')

	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del  Inst") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)        PORT_CHAR('\t')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Repeat") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START( "ROW8" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Home  Clr Screen") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Rvs Off") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START( "ROW9" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop Run") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x2190)

	PORT_START( "LOCK" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END


INPUT_PORTS_START( petb_de )
	PORT_INCLUDE( petb )
INPUT_PORTS_END


INPUT_PORTS_START( petb_fr )
	PORT_INCLUDE( petb )
INPUT_PORTS_END


INPUT_PORTS_START( petb_se )
	PORT_INCLUDE( petb )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

void pet_state::via_pa_w(uint8_t data)
{
	m_user->write_c((data>>0)&1);
	m_user->write_d((data>>1)&1);
	m_user->write_e((data>>2)&1);
	m_user->write_f((data>>3)&1);
	m_user->write_h((data>>4)&1);
	m_user->write_j((data>>5)&1);
	m_user->write_k((data>>6)&1);
	m_user->write_l((data>>7)&1);

	m_via_pa = data;
}

uint8_t pet_state::via_pb_r()
{
	/*

	  bit     description

	  PB0     _NDAC IN
	  PB1
	  PB2
	  PB3
	  PB4
	  PB5     SYNC IN
	  PB6     _NRFD IN
	  PB7     _DAV IN

	*/

	uint8_t data = 0;

	// video sync
	data |= (m_crtc ? m_crtc->vsync_r() : m_sync) << 5;

	// IEEE-488
	data |= m_ieee->ndac_r();
	data |= m_ieee->nrfd_r() << 6;
	data |= m_ieee->dav_r() << 7;

	return data;
}

void pet_state::via_pb_w(uint8_t data)
{
	/*

	  bit     description

	  PB0
	  PB1     _NRFD OUT
	  PB2     _ATN OUT
	  PB3     CASS WRITE
	  PB4     #2 CASS MOTOR
	  PB5
	  PB6
	  PB7

	*/

	// IEEE-488
	m_ieee->host_nrfd_w(BIT(data, 1));
	m_ieee->host_atn_w(BIT(data, 2));

	// cassette
	m_cassette->write(BIT(data, 3));
	m_cassette2->write(BIT(data, 3));
	m_cassette2->motor_w(BIT(data, 4));
}

void pet_state::via_ca2_w(int state)
{
	m_graphic = state;
}

void pet_state::via_cb2_w(int state)
{
	m_via_cb2 = state;
	update_speaker();

	m_user->write_m(state);
}


uint8_t pet_state::pia1_pa_r()
{
	/*

	  bit     description

	  PA0     KEY A
	  PA1     KEY B
	  PA2     KEY C
	  PA3     KEY D
	  PA4     #1 CASS SWITCH
	  PA5     #2 CASS SWITCH
	  PA6     _EOI IN
	  PA7     DIAG JUMPER

	*/

	uint8_t data = 0;

	// keyboard
	data |= m_key;

	// cassette
	data |= m_cassette->sense_r() << 4;
	data |= m_cassette2->sense_r() << 5;

	// IEEE-488
	data |= m_ieee->eoi_r() << 6;

	// diagnostic jumper
	data |= (m_user_diag && m_exp->diag_r()) << 7;

	return data;
}

void pet_state::pia1_pa_w(uint8_t data)
{
	/*

	  bit     description

	  PA0     KEY A
	  PA1     KEY B
	  PA2     KEY C
	  PA3     KEY D
	  PA4
	  PA5
	  PA6
	  PA7     SPEAKER

	*/

	// keyboard
	m_key = data & 0x0f;

	// speaker
	m_pia1_pa7 = BIT(data, 7);
	update_speaker();
}

uint8_t pet_state::pia1_pb_r()
{
	uint8_t data = 0xff;

	switch (m_key)
	{
	case 0: data &= m_row[0]->read(); break;
	case 1: data &= m_row[1]->read(); break;
	case 2: data &= m_row[2]->read(); break;
	case 3: data &= m_row[3]->read(); break;
	case 4: data &= m_row[4]->read(); break;
	case 5: data &= m_row[5]->read(); break;
	case 6: data &= m_row[6]->read(); break;
	case 7: data &= m_row[7]->read(); break;
	case 8: data &= m_row[8]->read() & m_lock->read(); break;
	case 9: data &= m_row[9]->read(); break;
	}

	return data;
}

uint8_t pet2001b_state::pia1_pb_r()
{
	uint8_t data = 0xff;

	switch (m_key)
	{
	case 0: data &= m_row[0]->read(); break;
	case 1: data &= m_row[1]->read(); break;
	case 2: data &= m_row[2]->read(); break;
	case 3: data &= m_row[3]->read(); break;
	case 4: data &= m_row[4]->read(); break;
	case 5: data &= m_row[5]->read(); break;
	case 6: data &= m_row[6]->read() & m_lock->read(); break;
	case 7: data &= m_row[7]->read(); break;
	case 8: data &= m_row[8]->read(); break;
	case 9: data &= m_row[9]->read(); break;
	}

	return data;
}

void pet_state::pia1_ca2_w(int state)
{
	m_ieee->host_eoi_w(state);

	m_blanktv = state;
}


void pet_state::user_diag_w(int state)
{
	m_user_diag = state;
}



//**************************************************************************
//  VIDEO
//**************************************************************************

TIMER_CALLBACK_MEMBER( pet_state::sync_tick )
{
	m_sync = !m_sync;

	m_pia1->cb1_w(m_sync);
}


//-------------------------------------------------
//  SCREEN_UPDATE( pet2001 )
//-------------------------------------------------

uint32_t pet_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pen = m_palette->pens();

	for (int y = 0; y < 200; y++)
	{
		for (int sx = 0; sx < 40; sx++)
		{
			int const sy = y / 8;
			offs_t const video_addr = (sy * 40) + sx;
			uint8_t const lsd = m_video_ram[video_addr];

			int const ra = y & 0x07;
			offs_t const char_addr = (m_graphic << 10) | ((lsd & 0x7f) << 3) | ra;
			uint8_t data = m_char_rom->base()[char_addr];

			for (int x = 0; x < 8; x++, data <<= 1)
			{
				int const color = (BIT(data, 7) ^ BIT(lsd, 7)) && m_blanktv;
				bitmap.pix(y, (sx * 8) + x) = pen[color];
			}
		}
	}

	return 0;
}


//-------------------------------------------------
//  MC6845 PET80
//-------------------------------------------------

MC6845_BEGIN_UPDATE( pet_state::pet_begin_update )
{
	bitmap.fill(rgb_t::black(), cliprect);
}

MC6845_UPDATE_ROW( pet80_state::pet80_update_row )
{
	int x = 0;
	int char_rom_mask = m_char_rom->bytes() - 1;
	const pen_t *pen = m_palette->pens();
	hbp = 80;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t lsd = 0, data = 0;
		uint8_t rra = ra & 0x07;
		int no_row = !(BIT(ra, 3) || BIT(ra, 4));
		int invert = BIT(ma, 12);
		int chr_option = BIT(ma, 13);

		// even character

		lsd = m_video_ram[((ma + column) << 1) & 0x7ff];

		offs_t char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int const video = (!((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) ^ invert) && de;
			bitmap.pix(vbp + y, hbp + x++) = pen[video];
		}

		// odd character

		lsd = m_video_ram[(((ma + column) << 1) + 1) & 0x7ff];

		char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int const video = (!((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) ^ invert) && de;
			bitmap.pix(vbp + y, hbp + x++) = pen[video];
		}
	}
}


//-------------------------------------------------
//  MC6845 PET40
//-------------------------------------------------

MC6845_UPDATE_ROW( pet_state::pet40_update_row )
{
	int x = 0;
	int char_rom_mask = m_char_rom->bytes() - 1;
	const pen_t *pen = m_palette->pens();
	hbp = 41;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t lsd = 0, data = 0;
		uint8_t rra = ra & 0x07;
		int no_row = !(BIT(ra, 3) || BIT(ra, 4));
		int invert = BIT(ma, 12);
		int chr_option = BIT(ma, 13);

		lsd = m_video_ram[(ma + column) & 0x3ff];

		offs_t char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int const video = (!((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) ^ invert) && de;
			bitmap.pix(vbp + y, hbp + x++) = pen[video];
		}
	}
}


//-------------------------------------------------
//  MC6845 CBM8296
//-------------------------------------------------

MC6845_UPDATE_ROW( pet80_state::cbm8296_update_row )
{
	int x = 0;
	int char_rom_mask = m_char_rom->bytes() - 1;
	const pen_t *pen = m_palette->pens();

	for (int column = 0; column < x_count; column++)
	{
		uint8_t lsd = 0, data = 0;
		uint8_t rra = ra & 0x07;
		int no_row = !BIT(ra, 3);
		int ra4 = BIT(ra, 4);
		int chr_option = BIT(ma, 13);
		offs_t vma = (ma + column) & 0x1fff;
		offs_t drma = 0x8000 | ra4 << 14 | ((vma & 0xf00) << 1) | (vma & 0xff);

		// even character

		lsd = m_ram->pointer()[drma];

		offs_t char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int const video = (((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) && de);
			bitmap.pix(vbp + y, hbp + x++) = pen[video];
		}

		// odd character

		lsd = m_ram->pointer()[drma | 0x100];

		char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int const video = (((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) && de);
			bitmap.pix(vbp + y, hbp + x++) = pen[video];
		}
	}
}


void cbm8296d_ieee488_devices(device_slot_interface &device)
{
	device.option_add("c8250lp", C8250LP);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

MACHINE_START_MEMBER( pet_state, pet )
{
	// initialize memory
	uint8_t data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	data = 0xff;

	for (offs_t offset = 0; offset < m_video_ram_size; offset++)
	{
		m_video_ram[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	if (!m_sync_timer)
		m_sync_timer = timer_alloc(FUNC(pet_state::sync_tick), this);

	// state saving
	save_item(NAME(m_key));
	save_item(NAME(m_sync));
	save_item(NAME(m_graphic));
	save_item(NAME(m_blanktv));
	save_item(NAME(m_user_diag));
}


MACHINE_START_MEMBER( pet_state, pet2001 )
{
	m_video_ram_size = 0x400;

	MACHINE_START_CALL_MEMBER(pet);
}


MACHINE_RESET_MEMBER( pet_state, pet )
{
	m_maincpu->reset();

	m_via->reset();
	m_pia1->reset();
	m_pia2->reset();

	m_exp->reset();

	m_ieee->host_ren_w(0);

	if (m_sync_period != attotime::zero)
		m_sync_timer->adjust(machine().time() + m_sync_period, 0, m_sync_period);
}


MACHINE_START_MEMBER( pet_state, pet40 )
{
	m_video_ram_size = 0x400;

	MACHINE_START_CALL_MEMBER(pet);
}


MACHINE_RESET_MEMBER( pet_state, pet40 )
{
	MACHINE_RESET_CALL_MEMBER(pet);

	m_crtc->reset();
}


MACHINE_START_MEMBER( pet80_state, pet80 )
{
	m_video_ram_size = 0x800;

	MACHINE_START_CALL_MEMBER(pet);
}


MACHINE_RESET_MEMBER( pet80_state, pet80 )
{
	MACHINE_RESET_CALL_MEMBER(pet);

	m_crtc->reset();
}


MACHINE_START_MEMBER( cbm8296_state, cbm8296 )
{
	MACHINE_START_CALL_MEMBER(pet80);

	// state saving
	save_item(NAME(m_cr));
	save_item(NAME(m_via_pa));
}


MACHINE_RESET_MEMBER( cbm8296_state, cbm8296 )
{
	MACHINE_RESET_CALL_MEMBER(pet80);

	m_cr = 0;
	m_via_pa = 0xff;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void pet_state::_4k(machine_config &config)
{
	RAM(config, m_ram);
	m_ram->set_default_size("4K");
	m_ram->set_extra_options("8K,16K,32K");
}

void pet_state::_8k(machine_config &config)
{
	RAM(config, m_ram);
	m_ram->set_default_size("8K");
	m_ram->set_extra_options("16K,32K");
}

void pet_state::_16k(machine_config &config)
{
	RAM(config, m_ram);
	m_ram->set_default_size("16K");
	m_ram->set_extra_options("32K");
}

void pet_state::_32k(machine_config &config)
{
	RAM(config, m_ram);
	m_ram->set_default_size("32K");
}

void pet_state::base_pet_devices(machine_config &config, const char *default_drive)
{
	input_merger_device &mainirq(INPUT_MERGER_ANY_HIGH(config, "mainirq")); // open collector
	mainirq.output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);
	mainirq.output_handler().append(m_exp, FUNC(pet_expansion_slot_device::irq_w));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MOS6522(config, m_via, XTAL(16'000'000)/16);
	m_via->readpb_handler().set(FUNC(pet_state::via_pb_r));
	m_via->writepa_handler().set(FUNC(pet_state::via_pa_w));
	m_via->writepb_handler().set(FUNC(pet_state::via_pb_w));
	m_via->ca2_handler().set(FUNC(pet_state::via_ca2_w));
	m_via->cb2_handler().set(FUNC(pet_state::via_cb2_w));
	m_via->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));

	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set(FUNC(pet_state::pia1_pa_r));
	m_pia1->readpb_handler().set(FUNC(pet_state::pia1_pb_r));
	m_pia1->writepa_handler().set(FUNC(pet_state::pia1_pa_w));
	m_pia1->ca2_handler().set(FUNC(pet_state::pia1_ca2_w));
	m_pia1->cb2_handler().set(PET_DATASSETTE_PORT_TAG, FUNC(pet_datassette_port_device::motor_w));
	m_pia1->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_pia1->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia2);
	m_pia2->readpa_handler().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_pia2->writepb_handler().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_pia2->ca2_handler().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_pia2->cb2_handler().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_pia2->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<3>));
	m_pia2->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<4>));

	ieee488_slot_device::add_cbm_defaults(config, default_drive);
	IEEE488(config, m_ieee, 0);
	m_ieee->srq_callback().set(m_pia2, FUNC(pia6821_device::cb1_w));
	m_ieee->atn_callback().set(m_pia2, FUNC(pia6821_device::ca1_w));

	PET_DATASSETTE_PORT(config, PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, "c2n").read_handler().set(m_pia1, FUNC(pia6821_device::ca1_w));
	PET_DATASSETTE_PORT(config, PET_DATASSETTE_PORT2_TAG, cbm_datassette_devices, nullptr).read_handler().set(M6522_TAG, FUNC(via6522_device::write_cb1));

	PET_EXPANSION_SLOT(config, m_exp, XTAL(16'000'000)/16, pet_expansion_cards, nullptr);
	m_exp->dma_read_callback().set(FUNC(pet_state::read));
	m_exp->dma_write_callback().set(FUNC(pet_state::write));

	PET_USER_PORT(config, m_user, pet_user_port_cards, nullptr);
	m_user->pb_handler().set(m_via, FUNC(via6522_device::write_ca1));
	m_user->pc_handler().set(m_via, FUNC(via6522_device::write_pa0));
	m_user->pd_handler().set(m_via, FUNC(via6522_device::write_pa1));
	m_user->pe_handler().set(m_via, FUNC(via6522_device::write_pa2));
	m_user->pf_handler().set(m_via, FUNC(via6522_device::write_pa3));
	m_user->ph_handler().set(m_via, FUNC(via6522_device::write_pa4));
	m_user->pj_handler().set(m_via, FUNC(via6522_device::write_pa5));
	m_user->pk_handler().set(m_via, FUNC(via6522_device::write_pa6));
	m_user->pl_handler().set(m_via, FUNC(via6522_device::write_pa7));
	m_user->pm_handler().set(m_via, FUNC(via6522_device::write_cb2));

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "p00,prg", CBM_QUICKLOAD_DELAY));
	quickload.set_load_callback(FUNC(pet_state::quickload_pet));
	quickload.set_interface("cbm_quik");

	SOFTWARE_LIST(config, "cass_list").set_original("pet_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("pet_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("pet_hdd");
	SOFTWARE_LIST(config, "quik_list").set_original("pet_quik");
}

void pet_state::pet(machine_config &config)
{
	base_pet_devices(config, "c4040");

	MCFG_MACHINE_START_OVERRIDE(pet_state, pet2001)
	MCFG_MACHINE_RESET_OVERRIDE(pet_state, pet)

	// basic machine hardware
	M6502(config, m_maincpu, XTAL(8'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &pet_state::pet2001_mem);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(320, 200);
	m_screen->set_visarea(0, 320-1, 0, 200-1);
	m_screen->set_screen_update(FUNC(pet_state::screen_update));
	m_sync_period = attotime::from_hz(120);

	m_user->p5_handler().set(FUNC(pet_state::user_diag_w));
}

void pet_state::pet2001(machine_config &config)
{
	pet(config);
	_4k(config);
}

void pet_state::pet20018(machine_config &config)
{
	pet(config);
	_8k(config);
}

void pet_state::pet2001n(machine_config &config, bool with_b000)
{
	pet(config);

	GENERIC_CARTSLOT(config, "cart_9000", generic_linear_slot, "pet_9000_rom", "bin,rom");
	GENERIC_CARTSLOT(config, "cart_a000", generic_linear_slot, "pet_a000_rom", "bin,rom");
	if (with_b000)
		GENERIC_CARTSLOT(config, "cart_b000", generic_linear_slot, "pet_b000_rom", "bin,rom");

	SOFTWARE_LIST(config, "rom_list").set_original("pet_rom");
}


void pet_state::pet2001n8(machine_config &config)
{
	pet2001n(config);
	_8k(config);
}

void pet_state::pet2001n16(machine_config &config)
{
	pet2001n(config);
	_16k(config);
}

void pet_state::pet2001n32(machine_config &config)
{
	pet2001n(config);
	_32k(config);
}

void pet_state::cbm3000(machine_config &config)
{
	pet2001n(config, false);
	// video hardware
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(320, 200);
	m_screen->set_visarea(0, 320-1, 0, 200-1);
	m_screen->set_screen_update(FUNC(pet_state::screen_update));
	m_sync_period = attotime::from_hz(100);
}

void pet_state::cbm3008(machine_config &config)
{
	cbm3000(config);
	_8k(config);
}

void pet_state::cbm3016(machine_config &config)
{
	cbm3000(config);
	_16k(config);
}

void pet_state::cbm3032(machine_config &config)
{
	cbm3000(config);
	_32k(config);
}

void pet2001b_state::pet2001b(machine_config &config, bool with_b000)
{
	pet2001n(config, with_b000);
	m_pia1->readpb_handler().set(FUNC(pet2001b_state::pia1_pb_r));
}

void pet2001b_state::pet2001b8(machine_config &config)
{
	pet2001b(config);
	_8k(config);
}

void pet2001b_state::pet2001b16(machine_config &config)
{
	pet2001b(config);
	_16k(config);
}

void pet2001b_state::pet2001b32(machine_config &config)
{
	pet2001b(config);
	_32k(config);
}

void pet2001b_state::cbm3032b(machine_config &config)
{
	pet2001b(config);
	// video hardware
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(320, 200);
	m_screen->set_visarea(0, 320-1, 0, 200-1);
	m_screen->set_screen_update(FUNC(pet_state::screen_update));
	m_sync_period = attotime::from_hz(100);

	_32k(config);
}

void pet2001b_state::pet4000(machine_config &config)
{
	pet2001n(config, false);
}

void pet2001b_state::pet4016(machine_config &config)
{
	pet4000(config);
	// RAM not upgradeable
	RAM(config, m_ram);
	m_ram->set_default_size("16K");
}

void pet2001b_state::pet4032(machine_config &config)
{
	pet4000(config);
	_32k(config);
}

void pet2001b_state::pet4032f(machine_config &config)
{
	pet4000(config);
	MCFG_MACHINE_START_OVERRIDE(pet_state, pet40)
	MCFG_MACHINE_RESET_OVERRIDE(pet_state, pet40)

	// video hardware
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(320, 250);
	m_screen->set_visarea(0, 320 - 1, 0, 250 - 1);
	m_screen->set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	m_sync_period = attotime::never;

	MC6845(config, m_crtc, XTAL(16'000'000)/16);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_begin_update_callback(FUNC(pet_state::pet_begin_update));
	m_crtc->set_update_row_callback(FUNC(pet_state::pet40_update_row));
	m_crtc->out_vsync_callback().set(M6520_1_TAG, FUNC(pia6821_device::cb1_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	_32k(config);
}

void pet_state::cbm4000(machine_config &config)
{
	pet2001n(config, false);
	// video hardware
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(320, 200);
	m_screen->set_visarea(0, 320-1, 0, 200-1);
	m_screen->set_screen_update(FUNC(pet_state::screen_update));
	m_sync_period = attotime::from_hz(100);
}

void pet_state::cbm4016(machine_config &config)
{
	cbm4000(config);
	// RAM not upgradeable
	RAM(config, m_ram);
	m_ram->set_default_size("16K");
}

void pet_state::cbm4032(machine_config &config)
{
	cbm4000(config);
	_32k(config);
}

void pet_state::cbm4032f(machine_config &config)
{
	cbm4000(config);
	MCFG_MACHINE_START_OVERRIDE(pet_state, pet40)
	MCFG_MACHINE_RESET_OVERRIDE(pet_state, pet40)

	// video hardware
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(320, 250);
	m_screen->set_visarea(0, 320 - 1, 0, 250 - 1);
	m_screen->set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	m_sync_period = attotime::never;

	MC6845(config, m_crtc, XTAL(16'000'000)/16);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_begin_update_callback(FUNC(pet_state::pet_begin_update));
	m_crtc->set_update_row_callback(FUNC(pet_state::pet40_update_row));
	m_crtc->out_vsync_callback().set(M6520_1_TAG, FUNC(pia6821_device::cb1_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	_32k(config);
}

void pet2001b_state::pet4000b(machine_config &config)
{
	pet2001b(config, false);
}

void pet2001b_state::pet4032b(machine_config &config)
{
	pet4000b(config);
	_32k(config);
}

void pet2001b_state::cbm4000b(machine_config &config)
{
	pet2001b(config, false);
	// video hardware
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(320, 200);
	m_screen->set_visarea(0, 320-1, 0, 200-1);
	m_screen->set_screen_update(FUNC(pet_state::screen_update));
	m_sync_period = attotime::from_hz(100);
}

void pet2001b_state::cbm4032b(machine_config &config)
{
	cbm4000b(config);
	_32k(config);
}

void pet80_state::pet80(machine_config &config)
{
	base_pet_devices(config, "c8050");

	MCFG_MACHINE_START_OVERRIDE(pet80_state, pet80)
	MCFG_MACHINE_RESET_OVERRIDE(pet80_state, pet80)

	// basic machine hardware
	M6502(config, m_maincpu, XTAL(16'000'000)/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &pet_state::pet2001_mem);

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(640, 250);
	screen.set_visarea(0, 640 - 1, 0, 250 - 1);
	screen.set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, XTAL(16'000'000)/16);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(2*8);
	m_crtc->set_begin_update_callback(FUNC(pet_state::pet_begin_update));
	m_crtc->set_update_row_callback(FUNC(pet80_state::pet80_update_row));
	m_crtc->out_vsync_callback().set(M6520_1_TAG, FUNC(pia6821_device::cb1_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	GENERIC_CARTSLOT(config, "cart_9000", generic_linear_slot, "pet_9000_rom", "bin,rom");
	GENERIC_CARTSLOT(config, "cart_a000", generic_linear_slot, "pet_a000_rom", "bin,rom");

	// software lists
	SOFTWARE_LIST(config, "rom_list").set_original("pet_rom");
}

void pet80_state::pet8032(machine_config &config)
{
	pet80(config);
	_32k(config);
}

void superpet_state::superpet(machine_config &config)
{
	pet8032(config);
	m_exp->set_default_option("superpet");
	SOFTWARE_LIST(config, "flop_list2").set_original("superpet_flop");
}

void cbm8096_state::cbm8096(machine_config &config)
{
	pet80(config);
	m_exp->set_default_option("64k");

	RAM(config, m_ram);
	m_ram->set_default_size("96K");

	SOFTWARE_LIST(config, "flop_list2").set_original("cbm8096_flop");
}

void cbm8296_state::cbm8296(machine_config &config)
{
	pet80(config);
	MCFG_MACHINE_START_OVERRIDE(cbm8296_state, cbm8296)
	MCFG_MACHINE_RESET_OVERRIDE(cbm8296_state, cbm8296)

	m_maincpu->set_addrmap(AS_PROGRAM, &cbm8296_state::cbm8296_mem);

	PLS100(config, PLA1_TAG);
	PLS100(config, PLA2_TAG);

	m_crtc->set_clock(XTAL(16'000'000)/16);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(2*8);
	m_crtc->set_update_row_callback(FUNC(pet80_state::cbm8296_update_row));
	m_crtc->out_vsync_callback().set(M6520_1_TAG, FUNC(pia6821_device::cb1_w));

	subdevice<ieee488_slot_device>("ieee8")->set_default_option("c8250");

	RAM(config, m_ram);
	m_ram->set_default_size("128K");

	SOFTWARE_LIST(config, "flop_list2").set_original("cbm8296_flop");
}

void cbm8296_state::cbm8296d(machine_config &config)
{
	cbm8296(config);
	ieee488_slot_device &ieee8(*subdevice<ieee488_slot_device>("ieee8"));
	cbm8296d_ieee488_devices(ieee8);
	ieee8.set_default_option("c8250lp");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( pet2001 )
//-------------------------------------------------

ROM_START( pet2001 )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS( "basic1r" )
	ROM_SYSTEM_BIOS( 0, "basic1o", "Original" )
	ROMX_LOAD( "901447-01.h1", 0x3000, 0x0800, CRC(a055e33a) SHA1(831db40324113ee996c434d38b4add3fd1f820bd), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "basic1r", "Revised" )
	ROMX_LOAD( "901447-09.h1", 0x3000, 0x0800, CRC(03cf16d0) SHA1(1330580c0614d3556a389da4649488ba04a60908), ROM_BIOS(1) )
	ROM_LOAD( "901447-02.h5", 0x3800, 0x0800, CRC(69fd8a8f) SHA1(70c0f4fa67a70995b168668c957c3fcf2c8641bd) )
	ROM_LOAD( "901447-03.h2", 0x4000, 0x0800, CRC(d349f2d4) SHA1(4bf2c20c51a63d213886957485ebef336bb803d0) )
	ROM_LOAD( "901447-04.h6", 0x4800, 0x0800, CRC(850544eb) SHA1(d293972d529023d8fd1f493149e4777b5c253a69) )
	ROM_LOAD( "901447-05.h3", 0x5000, 0x0800, CRC(9e1c5cea) SHA1(f02f5fb492ba93dbbd390f24c10f7a832dec432a) )
	ROM_LOAD( "901447-06.h4", 0x6000, 0x0800, CRC(661a814a) SHA1(960717282878e7de893d87242ddf9d1512be162e) )
	ROM_LOAD( "901447-07.h7", 0x6800, 0x0800, CRC(c4f47ad1) SHA1(d440f2510bc52e20c3d6bc8b9ded9cea7f462a9c) )

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-08.a2", 0x000, 0x800, CRC(54f32f45) SHA1(3e067cc621e4beafca2b90cb8f6dba975df2855b) )
ROM_END

#define rom_pet20018 rom_pet2001


//-------------------------------------------------
//  ROM( pet2001n )
//-------------------------------------------------

ROM_START( pet2001n )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-01.ud6", 0x3000, 0x1000, CRC(63a7fe4a) SHA1(3622111f486d0e137022523657394befa92bde44) )   // BASIC 2
	ROM_LOAD( "901465-02.ud7", 0x4000, 0x1000, CRC(ae4cb035) SHA1(1bc0ebf27c9bb62ad71bca40313e874234cab6ac) )   // BASIC 2
	ROM_LOAD( "901447-24.ud8", 0x5000, 0x0800, CRC(e459ab32) SHA1(5e5502ce32f5a7e387d65efe058916282041e54b) )   // Screen Editor (40 columns, no CRTC, Normal Keyb)
	ROM_LOAD( "901465-03.ud9", 0x6000, 0x1000, CRC(f02238e2) SHA1(38742bdf449f629bcba6276ef24d3daeb7da6e84) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END

#define rom_pet2001n16 rom_pet2001n
#define rom_pet2001n32 rom_pet2001n
#define rom_cbm3008 rom_pet2001n
#define rom_cbm3016 rom_pet2001n
#define rom_cbm3032 rom_pet2001n


//-------------------------------------------------
//  ROM( pet2001b )
//-------------------------------------------------

ROM_START( pet2001b )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-01.ud6", 0x3000, 0x1000, CRC(63a7fe4a) SHA1(3622111f486d0e137022523657394befa92bde44) )   // BASIC 2
	ROM_LOAD( "901465-02.ud7", 0x4000, 0x1000, CRC(ae4cb035) SHA1(1bc0ebf27c9bb62ad71bca40313e874234cab6ac) )   // BASIC 2
	ROM_LOAD( "901474-01.ud8", 0x5000, 0x0800, CRC(05db957e) SHA1(174ace3a8c0348cd21d39cc864e2adc58b0101a9) )   // Screen Editor (40 columns, no CRTC, Business Keyb)
	ROM_LOAD( "901465-03.ud9", 0x6000, 0x1000, CRC(f02238e2) SHA1(38742bdf449f629bcba6276ef24d3daeb7da6e84) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END

#define rom_pet2001b16 rom_pet2001b
#define rom_pet2001b32 rom_pet2001b
#define rom_cbm3032b rom_pet2001b


//-------------------------------------------------
//  ROM( pet4016 )
//-------------------------------------------------

ROM_START( pet4016 )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS( "basic4r" )
	ROM_SYSTEM_BIOS( 0, "basic4", "Original" )
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(1) ) // BASIC 4
	ROM_LOAD( "901465-20.ud6", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud7", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "901447-29.ud8", 0x5000, 0x0800, CRC(e5714d4c) SHA1(e88f56e5c54b0e8d8d4e8cb39a4647c803c1f51c) )   // Screen Editor (40 columns, no CRTC, Normal Keyb)
	ROM_LOAD( "901465-22.ud9", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END

#define rom_pet4032 rom_pet4016


//-------------------------------------------------
//  ROM( pet4032f )
//-------------------------------------------------

ROM_START( pet4032f )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS( "basic4r" )
	ROM_SYSTEM_BIOS( 0, "basic4", "Original" )
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(1) ) // BASIC 4
	ROM_LOAD( "901465-20.ud6", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud7", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "901499-01.ud7", 0x5000, 0x0800, CRC(5f85bdf8) SHA1(8cbf086c1ce4dfb2a2fe24c47476dfb878493dee) )   // Screen Editor (40 columns, CRTC 60Hz, Normal Keyb?)
	ROM_LOAD( "901465-22.ud9", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END


//-------------------------------------------------
//  ROM( cbm4016 )
//-------------------------------------------------

ROM_START( cbm4016 )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS( "basic4r" )
	ROM_SYSTEM_BIOS( 0, "basic4", "Original" )
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(1) ) // BASIC 4
	ROM_LOAD( "901465-20.ud6", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud7", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "901447-29.ud8", 0x5000, 0x0800, CRC(e5714d4c) SHA1(e88f56e5c54b0e8d8d4e8cb39a4647c803c1f51c) )   // Screen Editor (40 columns, no CRTC, Normal Keyb)
	ROM_LOAD( "901465-22.ud9", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END

#define rom_cbm4032 rom_cbm4016


//-------------------------------------------------
//  ROM( cbm4032f )
//-------------------------------------------------

ROM_START( cbm4032f )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS( "basic4r" )
	ROM_SYSTEM_BIOS( 0, "basic4", "Original" )
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(1) ) // BASIC 4
	ROM_LOAD( "901465-20.ud6", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud7", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "901498-01.ud7", 0x5000, 0x0800, CRC(3370e359) SHA1(05af284c914d53a52987b5f602466de75765f650) )   // Screen Editor (40 columns, CRTC 50Hz, Normal Keyb?)
	ROM_LOAD( "901465-22.ud9", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END


//-------------------------------------------------
//  ROM( pet4032b )
//-------------------------------------------------

ROM_START( pet4032b )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS( "basic4r" )
	ROM_SYSTEM_BIOS( 0, "basic4", "Original" )
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(1) ) // BASIC 4
	ROM_LOAD( "901465-20.ud6", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud7", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "901474-02.ud8", 0x5000, 0x0800, CRC(75ff4af7) SHA1(0ca5c4e8f532f914cb0bf86ea9900f20f0a655ce) )   // Screen Editor (40 columns, no CRTC, Business Keyb)
	ROM_LOAD( "901465-22.ud9", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END

#define rom_cbm4032b rom_pet4032b


//-------------------------------------------------
//  ROM( pet8032 )
//-------------------------------------------------

ROM_START( pet8032 )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-23.ud10", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )  // BASIC 4
	ROM_LOAD( "901465-20.ud9", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud8", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "901474-03.ud7", 0x5000, 0x0800, CRC(5674dd5e) SHA1(c605fa343fd77c73cbe1e0e9567e2f014f6e7e30) )   // Screen Editor (80 columns, CRTC 60Hz, Business Keyb)
	ROM_LOAD( "901465-22.ud6", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.ua3", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )    // Character Generator
ROM_END


//-------------------------------------------------
//  ROM( cbm8032 )
//-------------------------------------------------

ROM_START( cbm8032 )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-23.ud10", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )  // BASIC 4
	ROM_LOAD( "901465-20.ud9", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud8", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "901474-04.ud7", 0x5000, 0x0800, CRC(abb000e7) SHA1(66887061b6c4ebef7d6efb90af9afd5e2c3b08ba) )   // Screen Editor (80 columns, CRTC 50Hz, Business Keyb)
	ROM_LOAD( "901465-22.ud6", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-10.ua3", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )    // Character Generator
ROM_END

#define rom_cbm8096 rom_cbm8032


//-------------------------------------------------
//  ROM( cbm8032_de )
//-------------------------------------------------

ROM_START( cbm8032_de )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-23.ud10", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )  // BASIC 4
	ROM_LOAD( "901465-20.ud9", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud8", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "german.bin",    0x5000, 0x0800, CRC(1c1e597d) SHA1(7ac75ed73832847623c9f4f197fe7fb1a73bb41c) )
	ROM_LOAD( "901465-22.ud6", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "chargen.de", 0x0000, 0x800, CRC(3bb8cb87) SHA1(a4f0df13473d7f9cd31fd62cfcab11318e2fb1dc) )
ROM_END


//-------------------------------------------------
//  ROM( cbm8032_fr )
//-------------------------------------------------

ROM_START( cbm8032_fr )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-23.ud10", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )  // BASIC 4
	ROM_LOAD( "901465-20.ud9", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud8", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "8032_editor_80c_fr_a1ab.ud7", 0x5000, 0x1000, CRC(4d3d9918) SHA1(eedac298a201a28ad86a5939b569a46f9f12c16f) )
	ROM_LOAD( "901465-22.ud6", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "8032_chargen_80c_fr_b9c1.ua3", 0x0000, 0x1000, CRC(d7424620) SHA1(e74c0eabb921d4a34032b57a7ee61ce61ec8a10c) )
ROM_END


//-------------------------------------------------
//  ROM( cbm8032_se )
//-------------------------------------------------

ROM_START( cbm8032_se )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-23.ud10", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )  // BASIC 4
	ROM_LOAD( "901465-20.ud9", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud8", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "8000-ud7, screen-04.ud7", 0x5000, 0x0800, CRC(75901dd7) SHA1(2ead0d83255a344a42bb786428353ca48d446d03) )
	ROM_LOAD( "901465-22.ud6", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x800, "charom", 0 )
	ROM_LOAD( "901447-14.ua3", 0x0000, 0x800, CRC(48c77d29) SHA1(aa7c8ff844d16ec05e2b32acc586c58d9e35388c) )    // Character Generator
ROM_END


//-------------------------------------------------
//  ROM( superpet )
//-------------------------------------------------

ROM_START( superpet )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-23.ud10", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )  // BASIC 4
	ROM_LOAD( "901465-20.ud9", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud8", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "901474-04.ud7", 0x5000, 0x0800, CRC(abb000e7) SHA1(66887061b6c4ebef7d6efb90af9afd5e2c3b08ba) )   // Screen Editor (80 columns, CRTC 50Hz, Business Keyb)
	ROM_LOAD( "901465-22.ud6", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901640-01.ua3", 0x0000, 0x1000, CRC(ee8229c4) SHA1(bf346f11595a3e65e55d6aeeaa2c0cec807b66c7) )
ROM_END

#define rom_mmf9000 rom_superpet


//-------------------------------------------------
//  ROM( mmf9000_se )
//-------------------------------------------------

ROM_START( mmf9000_se )
	ROM_REGION( 0x7000, M6502_TAG, 0 )
	ROM_LOAD( "901465-23.ud10", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )  // BASIC 4
	ROM_LOAD( "901465-20.ud9", 0x3000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )   // BASIC 4
	ROM_LOAD( "901465-21.ud8", 0x4000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )   // BASIC 4
	ROM_LOAD( "8000-ud7, screen-04.ud7", 0x5000, 0x0800, CRC(75901dd7) SHA1(2ead0d83255a344a42bb786428353ca48d446d03) )
	ROM_LOAD( "901465-22.ud6", 0x6000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )   // Kernal

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901640-01 skand.gen.ua3", 0x0000, 0x1000, CRC(da1cd630) SHA1(35f472114ff001259bdbae073ae041b0759e32cb) )
ROM_END


//-------------------------------------------------
//  ROM( cbm8296 )
//-------------------------------------------------

ROM_START( cbm8296 )
	ROM_REGION( 0x2000, M6502_TAG, ROMREGION_ERASE00 )

	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "324746-01.ue7", 0x0000, 0x4000, CRC(03a25bb4) SHA1(e3e1431969bf317c885e47f3790e0bcbdf61fe77) )   // BASIC 4

	ROM_REGION( 0x1000, "editor", 0 )
	ROM_LOAD( "8296.ue8", 0x000, 0x800, CRC(a3475de6) SHA1(b715db83fd26458dfd254bef5c4aae636753f7f5) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901447-10.uc5", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )    // Character Generator

	ROM_REGION( 0x20, "prom", 0 )
	ROM_LOAD( "74s288.uc2", 0x00, 0x20, CRC(06030665) SHA1(19dc91ca49ecc20e66c646ba480d2c3bc70a62e6) ) // video/RAM timing

	ROM_REGION( 0x10000, "ue5_eprom", 0 )
	ROM_LOAD( "ue5.bin", 0x00000, 0x10000, CRC(f70b7b37) SHA1(fe0fbb0fa71775f3780134aa11dac5b761526148) )

	ROM_REGION( 0x10000, "ue6_eprom", 0 )
	ROM_LOAD( "ue6.bin", 0x00000, 0x10000, CRC(36952256) SHA1(e94d3e744a6aaff553bf260f25da0286436265d1) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "324744-01.ue6", 0x00, 0xf5, NO_DUMP ) // 8700-009

	ROM_REGION( 0xf5, PLA2_TAG, 0 )
	ROM_LOAD( "324745-01.ue5", 0x00, 0xf5, NO_DUMP ) // 8700-008
ROM_END


//-------------------------------------------------
//  ROM( cbm8296ed )
//-------------------------------------------------

ROM_START( cbm8296ed )
	ROM_REGION( 0x2000, M6502_TAG, 0 )
	ROM_LOAD( "oracle.ue10", 0x0000, 0x1000, CRC(1ee9485d) SHA1(f876933087c9633b0fafff4d1dc198017f250267) )  // Oracle 3.03
	ROM_LOAD( "paperclip.ue9", 0x1000, 0x1000, CRC(8fb11d4b) SHA1(1c0f883cd3b8ded42ec00d83f7e7f0887f91fec0) )  // Paperclip 2.84

	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "324746-01.ue7", 0x0000, 0x4000, CRC(03a25bb4) SHA1(e3e1431969bf317c885e47f3790e0bcbdf61fe77) )   // BASIC 4

	ROM_REGION( 0x1000, "editor", 0 )
	ROM_LOAD( "execudesk.ue8", 0x0000, 0x1000, CRC(bef0eaa1) SHA1(7ea63a2d651f516e96b8725195c13542ea495ebd) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901447-10.uc5", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )    // Character Generator

	ROM_REGION( 0x20, "prom", 0 )
	ROM_LOAD( "74s288.uc2", 0x00, 0x20, CRC(06030665) SHA1(19dc91ca49ecc20e66c646ba480d2c3bc70a62e6) ) // video/RAM timing

	ROM_REGION( 0x10000, "ue5_eprom", 0 )
	ROM_LOAD( "ue5.bin", 0x00000, 0x10000, CRC(f70b7b37) SHA1(fe0fbb0fa71775f3780134aa11dac5b761526148) )

	ROM_REGION( 0x10000, "ue6_eprom", 0 )
	ROM_LOAD( "ue6.bin", 0x00000, 0x10000, CRC(36952256) SHA1(e94d3e744a6aaff553bf260f25da0286436265d1) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "324744-01.ue6", 0x00, 0xf5, NO_DUMP ) // 8700-009

	ROM_REGION( 0xf5, PLA2_TAG, 0 )
	ROM_LOAD( "324745-01.ue5", 0x00, 0xf5, NO_DUMP ) // 8700-008
ROM_END


//-------------------------------------------------
//  ROM( cbm8296d )
//-------------------------------------------------

ROM_START( cbm8296d )
	ROM_REGION( 0x2000, M6502_TAG, ROMREGION_ERASE00 )

	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "324746-01.ue7", 0x0000, 0x4000, CRC(03a25bb4) SHA1(e3e1431969bf317c885e47f3790e0bcbdf61fe77) )   // BASIC 4

	ROM_REGION( 0x1000, "editor", 0 )
	ROM_LOAD( "324243-01.ue8", 0x0000, 0x1000, CRC(4000e833) SHA1(dafbdf8ba0a1fe7d7b9586ffbfc9e5390c0fcf6f) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901447-10.uc5", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )    // Character Generator

	ROM_REGION( 0x20, "prom", 0 )
	ROM_LOAD( "74s288.uc2", 0x00, 0x20, CRC(06030665) SHA1(19dc91ca49ecc20e66c646ba480d2c3bc70a62e6) ) // video/RAM timing

	ROM_REGION( 0x10000, "ue5_eprom", 0 )
	ROM_LOAD( "ue5.bin", 0x00000, 0x10000, CRC(f70b7b37) SHA1(fe0fbb0fa71775f3780134aa11dac5b761526148) )

	ROM_REGION( 0x10000, "ue6_eprom", 0 )
	ROM_LOAD( "ue6.bin", 0x00000, 0x10000, CRC(36952256) SHA1(e94d3e744a6aaff553bf260f25da0286436265d1) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "324744-01.ue6", 0x00, 0xf5, NO_DUMP ) // 8700-009

	ROM_REGION( 0xf5, PLA2_TAG, 0 )
	ROM_LOAD( "324745-01.ue5", 0x00, 0xf5, NO_DUMP ) // 8700-008
ROM_END


//-------------------------------------------------
//  ROM( cbm8296d_de )
//-------------------------------------------------

ROM_START( cbm8296d_de )
	ROM_REGION( 0x2000, M6502_TAG, ROMREGION_ERASE00 )

	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "324746-01.ue7", 0x0000, 0x4000, CRC(03a25bb4) SHA1(e3e1431969bf317c885e47f3790e0bcbdf61fe77) )   // BASIC 4

	ROM_REGION( 0x1000, "editor", 0 )
	ROM_LOAD( "324243-04.ue8", 0x0000, 0x1000, CRC(3fe48897) SHA1(c218ff3168514f1d5e7822ae1b1ac3e161523b33) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "324242-10.uc5", 0x0000, 0x1000, CRC(a5632a0f) SHA1(9616f7f18757cccefb702a945f954b644d5b17d1) )

	ROM_REGION( 0x20, "prom", 0 )
	ROM_LOAD( "74s288.uc2", 0x00, 0x20, CRC(06030665) SHA1(19dc91ca49ecc20e66c646ba480d2c3bc70a62e6) ) // video/RAM timing

	ROM_REGION( 0x10000, "ue5_eprom", 0 )
	ROM_LOAD( "ue5.bin", 0x00000, 0x10000, CRC(f70b7b37) SHA1(fe0fbb0fa71775f3780134aa11dac5b761526148) )

	ROM_REGION( 0x10000, "ue6_eprom", 0 )
	ROM_LOAD( "ue6.bin", 0x00000, 0x10000, CRC(36952256) SHA1(e94d3e744a6aaff553bf260f25da0286436265d1) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "324744-01.ue6", 0x00, 0xf5, NO_DUMP ) // 8700-009

	ROM_REGION( 0xf5, PLA2_TAG, 0 )
	ROM_LOAD( "324745-01.ue5", 0x00, 0xf5, NO_DUMP ) // 8700-008
ROM_END


//-------------------------------------------------
//  ROM( cbm8296gd )
//-------------------------------------------------

ROM_START( cbm8296gd )
	ROM_REGION( 0x2000, M6502_TAG, 0 )
	ROM_LOAD( "324992-02.ue10", 0x0000, 0x1000, CRC(2bac5baf) SHA1(03aa866e4bc4e38e95983a6a82ba925e710bede8) ) // HiRes Emulator
	ROM_LOAD( "324993-02.ue9", 0x1000, 0x1000, CRC(57444531) SHA1(74aa39888a6bc95762de767fce883203daca0d34) ) // HiRes BASIC

	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "324746-01.ue7", 0x0000, 0x4000, CRC(03a25bb4) SHA1(e3e1431969bf317c885e47f3790e0bcbdf61fe77) )   // BASIC 4

	ROM_REGION( 0x1000, "editor", 0 )
	ROM_LOAD( "324243-01.ue8", 0x0000, 0x1000, CRC(4000e833) SHA1(dafbdf8ba0a1fe7d7b9586ffbfc9e5390c0fcf6f) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901447-10.uc5", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )    // Character Generator

	ROM_REGION( 0x20, "prom", 0 )
	ROM_LOAD( "74s288.uc2", 0x00, 0x20, CRC(06030665) SHA1(19dc91ca49ecc20e66c646ba480d2c3bc70a62e6) ) // video/RAM timing

	ROM_REGION( 0x10000, "ue5_eprom", 0 )
	ROM_LOAD( "ue5.bin", 0x00000, 0x10000, CRC(f70b7b37) SHA1(fe0fbb0fa71775f3780134aa11dac5b761526148) )

	ROM_REGION( 0x10000, "ue6_eprom", 0 )
	ROM_LOAD( "ue6.bin", 0x00000, 0x10000, CRC(36952256) SHA1(e94d3e744a6aaff553bf260f25da0286436265d1) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "324744-01.ue6", 0x00, 0xf5, NO_DUMP ) // 8700-009

	ROM_REGION( 0xf5, PLA2_TAG, 0 )
	ROM_LOAD( "324745-01.ue5", 0x00, 0xf5, NO_DUMP ) // 8700-008
ROM_END


//-------------------------------------------------
//  ROM( cbm8296dgv_de )
//-------------------------------------------------

ROM_START( cbm8296dgv_de ) // SER.NO.WG 8947
	ROM_REGION( 0x2000, M6502_TAG, 0 )
	ROM_LOAD( "io gv.ue9", 0x1000, 0x1000, CRC(7adf50a0) SHA1(4f7abc5286e51f34cde98238410274715e766b31) ) // I/O MASTER (C)1982 J.PFEIFER

	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "324746-01.ue7", 0x0000, 0x4000, CRC(03a25bb4) SHA1(e3e1431969bf317c885e47f3790e0bcbdf61fe77) )

	ROM_REGION( 0x1000, "editor", 0 )
	ROM_LOAD( "ue8gv.ue8", 0x0000, 0x1000, CRC(8ad1fca9) SHA1(3c939092e51549696754c308b2a09f47c5d4d277) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "ua3gv.uc5", 0x000, 0x1000, CRC(d8035dc4) SHA1(cdf520a7dabf1b18aed15455b1dbefac15ff91f3) )

	ROM_REGION( 0x20, "prom", 0 )
	ROM_LOAD( "74s288.uc2", 0x00, 0x20, CRC(06030665) SHA1(19dc91ca49ecc20e66c646ba480d2c3bc70a62e6) )

	ROM_REGION( 0x10000, "ue5_eprom", 0 )
	ROM_LOAD( "ue5.bin", 0x00000, 0x10000, CRC(f70b7b37) SHA1(fe0fbb0fa71775f3780134aa11dac5b761526148) )

	ROM_REGION( 0x10000, "ue6_eprom", 0 )
	ROM_LOAD( "ue6.bin", 0x00000, 0x10000, CRC(36952256) SHA1(e94d3e744a6aaff553bf260f25da0286436265d1) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "324744-01.ue6", 0x00, 0xf5, NO_DUMP ) // 8700-009

	ROM_REGION( 0xf5, PLA2_TAG, 0 )
	ROM_LOAD( "324745-01.ue5", 0x00, 0xf5, NO_DUMP ) // 8700-008
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME           PARENT    COMPAT  MACHINE     INPUT    CLASS           INIT        COMPANY                        FULLNAME        FLAGS
COMP( 1977, pet2001,       0,        0,      pet2001,    pet,     pet_state,      empty_init, "Commodore Business Machines", "PET 2001-4",   MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1977, pet20018,      pet2001,  0,      pet20018,   pet,     pet_state,      empty_init, "Commodore Business Machines", "PET 2001-8",   MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001n,      0,        0,      pet2001n8,  pet,     pet_state,      empty_init, "Commodore Business Machines", "PET 2001-N8",  MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001n16,    pet2001n, 0,      pet2001n16, pet,     pet_state,      empty_init, "Commodore Business Machines", "PET 2001-N16", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001n32,    pet2001n, 0,      pet2001n32, pet,     pet_state,      empty_init, "Commodore Business Machines", "PET 2001-N32", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, cbm3008,       pet2001n, 0,      cbm3008,    pet,     pet_state,      empty_init, "Commodore Business Machines", "CBM 3008",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, cbm3016,       pet2001n, 0,      cbm3016,    pet,     pet_state,      empty_init, "Commodore Business Machines", "CBM 3016",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, cbm3032,       pet2001n, 0,      cbm3032,    pet,     pet_state,      empty_init, "Commodore Business Machines", "CBM 3032",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001b,      0,        0,      pet2001b8,  petb,    pet2001b_state, empty_init, "Commodore Business Machines", "PET 2001-B8",  MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001b16,    pet2001b, 0,      pet2001b16, petb,    pet2001b_state, empty_init, "Commodore Business Machines", "PET 2001-B16", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001b32,    pet2001b, 0,      pet2001b32, petb,    pet2001b_state, empty_init, "Commodore Business Machines", "PET 2001-B32", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, cbm3032b,      pet2001b, 0,      cbm3032b,   petb,    pet2001b_state, empty_init, "Commodore Business Machines", "CBM 3032B",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, pet4016,       0,        0,      pet4016,    pet,     pet2001b_state, empty_init, "Commodore Business Machines", "PET 4016",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, pet4032,       pet4016,  0,      pet4032,    pet,     pet2001b_state, empty_init, "Commodore Business Machines", "PET 4032",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, pet4032f,      pet4016,  0,      pet4032f,   pet,     pet2001b_state, empty_init, "Commodore Business Machines", "PET 4032 (Fat 40)",     MACHINE_SUPPORTS_SAVE )
COMP( 1980, cbm4016,       pet4016,  0,      cbm4016,    pet,     pet_state,      empty_init, "Commodore Business Machines", "CBM 4016",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, cbm4032,       pet4016,  0,      cbm4032,    pet,     pet_state,      empty_init, "Commodore Business Machines", "CBM 4032",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, cbm4032f,      pet4016,  0,      cbm4032f,   pet,     pet_state,      empty_init, "Commodore Business Machines", "CBM 4032 (Fat 40)",     MACHINE_SUPPORTS_SAVE )
COMP( 1980, pet4032b,      0,        0,      pet4032b,   petb,    pet2001b_state, empty_init, "Commodore Business Machines", "PET 4032B",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, cbm4032b,      pet4032b, 0,      cbm4032b,   petb,    pet2001b_state, empty_init, "Commodore Business Machines", "CBM 4032B",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, pet8032,       0,        0,      pet8032,    petb,    pet80_state,    empty_init, "Commodore Business Machines", "PET 8032",     MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8032,       pet8032,  0,      pet8032,    petb,    pet80_state,    empty_init, "Commodore Business Machines", "CBM 8032",     MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8032_de,    pet8032,  0,      pet8032,    petb_de, pet80_state,    empty_init, "Commodore Business Machines", "CBM 8032 (Germany)",        MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8032_fr,    pet8032,  0,      pet8032,    petb_fr, pet80_state,    empty_init, "Commodore Business Machines", "CBM 8032 (France)",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8032_se,    pet8032,  0,      pet8032,    petb_se, pet80_state,    empty_init, "Commodore Business Machines", "CBM 8032 (Sweden/Finland)", MACHINE_SUPPORTS_SAVE )
COMP( 1981, superpet,      pet8032,  0,      superpet,   petb,    superpet_state, empty_init, "Commodore Business Machines", "SuperPET SP-9000",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1981, mmf9000,       pet8032,  0,      superpet,   petb,    superpet_state, empty_init, "Commodore Business Machines", "MicroMainFrame 9000",       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1981, mmf9000_se,    pet8032,  0,      superpet,   petb_se, superpet_state, empty_init, "Commodore Business Machines", "MicroMainFrame 9000 (Sweden/Finland)",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8096,       pet8032,  0,      cbm8096,    petb,    cbm8096_state,  empty_init, "Commodore Business Machines", "CBM 8096",     MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296,       0,        0,      cbm8296,    petb,    cbm8296_state,  empty_init, "Commodore Business Machines", "CBM 8296",     MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296ed,     cbm8296,  0,      cbm8296d,   petb,    cbm8296_state,  empty_init, "Commodore Business Machines", "CBM 8296 ExecuDesk",        MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296d,      cbm8296,  0,      cbm8296d,   petb,    cbm8296_state,  empty_init, "Commodore Business Machines", "CBM 8296-D",   MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296d_de,   cbm8296,  0,      cbm8296d,   petb_de, cbm8296_state,  empty_init, "Commodore Business Machines", "CBM 8296-D (Germany)",      MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296gd,     cbm8296,  0,      cbm8296d,   petb,    cbm8296_state,  empty_init, "Commodore Business Machines", "CBM 8296GD",   MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296dgv_de, cbm8296,  0,      cbm8296d,   petb,    cbm8296_state,  empty_init, "Commodore Business Machines", "CBM 8296-D GV? (Germany)",  MACHINE_SUPPORTS_SAVE )
