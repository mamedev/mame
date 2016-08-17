// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

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

#include "includes/pet.h"
#include "bus/ieee488/c2040.h"
#include "machine/cbm_snqk.h"
#include "softlist.h"


static void cbm_pet_quick_sethiaddress( address_space &space, UINT16 hiaddress )
{
	space.write_byte(0x2e, hiaddress & 0xff);
	space.write_byte(0x2c, hiaddress & 0xff);
	space.write_byte(0x2a, hiaddress & 0xff);
	space.write_byte(0x2f, hiaddress >> 8);
	space.write_byte(0x2d, hiaddress >> 8);
	space.write_byte(0x2b, hiaddress >> 8);
}

QUICKLOAD_LOAD_MEMBER( pet_state, cbm_pet )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, m_maincpu->space(AS_PROGRAM), 0, cbm_pet_quick_sethiaddress);
}



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void pet_state::check_interrupts()
{
	int irq = m_via_irq || m_pia1a_irq || m_pia1b_irq || m_pia2a_irq || m_pia2b_irq || m_exp_irq;

	m_maincpu->set_input_line(M6502_IRQ_LINE, irq);
	m_exp->irq_w(irq);
}


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

READ8_MEMBER( pet_state::read )
{
	int sel = offset >> 12;
	int norom = m_exp->norom_r(space, offset, sel);
	UINT8 data = 0;

	data = m_exp->read(space, offset, data, sel);

	switch (sel)
	{
	case SEL0: case SEL1: case SEL2: case SEL3: case SEL4: case SEL5: case SEL6: case SEL7:
		if (offset < m_ram->size())
		{
			data = m_ram->pointer()[offset];
		}
		break;

	case SEL8:
		data = m_video_ram[offset & (m_video_ram_size - 1)];
		break;

	case SEL9:
		if (norom)
		{
			if (m_cart_9000 && m_cart_9000->exists())
				data = m_cart_9000->read_rom(space, offset & 0xfff);
			else
				data = m_rom->base()[offset - 0x9000];
		}
		break;

	case SELA:
		if (norom)
		{
			if (m_cart_a000 && m_cart_a000->exists())
				data = m_cart_a000->read_rom(space, offset & 0xfff);
			else
				data = m_rom->base()[offset - 0x9000];
		}
		break;

	case SELB:
		if (norom)
		{
			if (m_cart_b000 && m_cart_b000->exists())
				data = m_cart_b000->read_rom(space, offset & 0xfff);
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
				data &= m_pia1->read(space, offset & 0x03);
			}
			if (BIT(offset, 5))
			{
				data &= m_pia2->read(space, offset & 0x03);
			}
			if (BIT(offset, 6))
			{
				data &= m_via->read(space, offset & 0x0f);
			}
			if (m_crtc && BIT(offset, 7) && BIT(offset, 0))
			{
				data &= m_crtc->register_r(space, 0);
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

WRITE8_MEMBER( pet_state::write )
{
	int sel = offset >> 12;

	m_exp->write(space, offset, data, sel);

	switch (sel)
	{
	case SEL0: case SEL1: case SEL2: case SEL3: case SEL4: case SEL5: case SEL6: case SEL7:
		if (offset < m_ram->size())
		{
			m_ram->pointer()[offset] = data;
		}
		break;

	case SEL8:
		m_video_ram[offset & (m_video_ram_size - 1)] = data;
		break;

	case SELE:
		if (BIT(offset, 11))
		{
			if (BIT(offset, 4))
			{
				m_pia1->write(space, offset & 0x03, data);
			}
			if (BIT(offset, 5))
			{
				m_pia2->write(space, offset & 0x03, data);
			}
			if (BIT(offset, 6))
			{
				m_via->write(space, offset & 0x0f, data);
			}
			if (m_crtc && BIT(offset, 7))
			{
				if (BIT(offset, 0))
				{
					m_crtc->register_w(space, 0, data);
				}
				else
				{
					m_crtc->address_w(space, 0, data);
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
	UINT32 input = (offset & 0xff00) | phi2 << 7 | brw << 6 | noscreen << 5 | noio << 4 | ramsela << 3 | ramsel9 << 2 | ramon << 1 | norom;
	UINT32 data = m_pla1->read(input);

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

	UINT32 input = (offset & 0xff00) | phi2 << 7 | brw << 6 | noscreen << 5 | noio << 4 | ramsela << 3 | ramsel9 << 2 | ramon << 1 | norom;
	input = BITSWAP16(input,13,8,9,7,12,14,11,10,6,5,4,3,2,1,0,15);

	UINT8 data = m_ue6_rom->base()[input];
	data = BITSWAP8(data,7,0,1,2,3,4,5,6);

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
	UINT32 input = BITSWAP8(m_cr, 0,1,2,3,4,5,6,7) << 8 | ((offset >> 8) & 0xf8) | brw << 2 | phi2 << 1 | casena1;
	UINT32 data = m_pla2->read(input);

	endra = BIT(data, 4);
	noscreen = BIT(data, 5);
	casena2 = BIT(data, 6);
	fa15 = BIT(data, 7);
}

void cbm8296_state::read_pla2_eprom(offs_t offset, int phi2, int brw, int casena1, int &endra, int &noscreen, int &casena2, int &fa15)
{
	// PLA-EPROM adapter by Joachim Nemetz (Jogi)

	UINT32 input = BITSWAP8(m_cr, 0,1,2,3,4,5,6,7) << 8 | ((offset >> 8) & 0xf8) | brw << 2 | phi2 << 1 | casena1;
	input = BITSWAP16(input,13,8,9,7,12,14,11,10,6,5,4,3,2,1,0,15);

	UINT8 data = m_ue5_rom->base()[input];
	data = BITSWAP8(data,7,0,1,2,3,4,5,6);

	endra = BIT(data, 4);
	noscreen = BIT(data, 5);
	casena2 = BIT(data, 6);
	fa15 = BIT(data, 7);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( cbm8296_state::read )
{
	int norom = m_exp->norom_r(space, offset, offset >> 12) && !BIT(m_cr, 7);
	int phi2 = 1, brw = 1, noscreen = 1, noio = BIT(m_cr, 6);
	int ramsela = BIT(m_via_pa, 0), ramsel9 = BIT(m_via_pa, 1), ramon = BIT(m_via_pa, 2);
	int cswff = 1, cs9 = 1, csa = 1, csio = 1, cse = 1, cskb = 1, fa12 = 1, fa15 = 1, casena1 = 1, casena2 = 1, endra = 1;

	read_pla1_eprom(offset, phi2, brw, noscreen, noio, ramsela, ramsel9, ramon, norom,
		cswff, cs9, csa, csio, cse, cskb, fa12, casena1);

	read_pla2_eprom(offset, phi2, brw, casena1, endra, noscreen, casena2, fa15);

	read_pla1_eprom(offset, phi2, brw, noscreen, noio, ramsela, ramsel9, ramon, norom,
		cswff, cs9, csa, csio, cse, cskb, fa12, casena1);

	//logerror("%s read  %04x : norom %u noio %u ramsela %u ramsel9 %u ramon %u / cswff %u cs9 %u csa %u csio %u cse %u cskb %u fa12 %u casena1 %u endra %u noscreen %u casena2 %u fa15 %u\n",machine().describe_context(),offset,norom,noio,ramsela,ramsel9,ramon,cswff,cs9,csa,csio,cse,cskb,fa12,casena1,endra,noscreen,casena2,fa15);

	UINT8 data = 0;

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
			data = m_cart_9000->read_rom(space, offset & 0xfff);
		else
			data = m_rom->base()[offset & 0xfff];
	}
	if (!csa)
	{
		if (m_cart_a000 && m_cart_a000->exists())
			data = m_cart_a000->read_rom(space, offset & 0xfff);
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
			data &= m_pia1->read(space, offset & 0x03);
		}
		if (BIT(offset, 5))
		{
			data &= m_pia2->read(space, offset & 0x03);
		}
		if (BIT(offset, 6))
		{
			data &= m_via->read(space, offset & 0x0f);
		}
		if (BIT(offset, 7) && BIT(offset, 0))
		{
			data &= m_crtc->register_r(space, 0);
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( cbm8296_state::write )
{
	int norom = m_exp->norom_r(space, offset, offset >> 12) && !BIT(m_cr, 7);
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
			m_pia1->write(space, offset & 0x03, data);
		}
		if (BIT(offset, 5))
		{
			m_pia2->write(space, offset & 0x03, data);
		}
		if (BIT(offset, 6))
		{
			m_via->write(space, offset & 0x0f, data);
		}
		if (BIT(offset, 7))
		{
			if (BIT(offset, 0))
			{
				m_crtc->register_w(space, 0, data);
			}
			else
			{
				m_crtc->address_w(space, 0, data);
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

//-------------------------------------------------
//  ADDRESS_MAP( pet2001_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( pet2001_mem, AS_PROGRAM, 8, pet_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( cbm8296_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( cbm8296_mem, AS_PROGRAM, 8, cbm8296_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( pet )
//-------------------------------------------------

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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR('/')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR('5')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR('3')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR(';')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')

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


//-------------------------------------------------
//  INPUT_PORTS( petb )
//-------------------------------------------------

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


//-------------------------------------------------
//  INPUT_PORTS( petb_de )
//-------------------------------------------------

INPUT_PORTS_START( petb_de )
	PORT_INCLUDE( petb )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( petb_fr )
//-------------------------------------------------

INPUT_PORTS_START( petb_fr )
	PORT_INCLUDE( petb )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( petb_se )
//-------------------------------------------------

INPUT_PORTS_START( petb_se )
	PORT_INCLUDE( petb )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

WRITE_LINE_MEMBER( pet_state::via_irq_w )
{
	m_via_irq = state;

	check_interrupts();
}

WRITE8_MEMBER( pet_state::via_pa_w )
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

READ8_MEMBER( pet_state::via_pb_r )
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

	UINT8 data = 0;

	// video sync
	data |= (m_crtc ? m_crtc->vsync_r() : m_sync) << 5;

	// IEEE-488
	data |= m_ieee->ndac_r();
	data |= m_ieee->nrfd_r() << 6;
	data |= m_ieee->dav_r() << 7;

	return data;
}

WRITE8_MEMBER( pet_state::via_pb_w )
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
	m_ieee->nrfd_w(BIT(data, 1));
	m_ieee->atn_w(BIT(data, 2));

	// cassette
	m_cassette->write(BIT(data, 3));
	m_cassette2->write(BIT(data, 3));
	m_cassette2->motor_w(BIT(data, 4));
}

WRITE_LINE_MEMBER( pet_state::via_ca2_w )
{
	m_graphic = state;
}

WRITE_LINE_MEMBER( pet_state::via_cb2_w )
{
	m_via_cb2 = state;
	update_speaker();

	m_user->write_m(state);
}


WRITE_LINE_MEMBER( pet_state::pia1_irqa_w )
{
	m_pia1a_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( pet_state::pia1_irqb_w )
{
	m_pia1b_irq = state;

	check_interrupts();
}

READ8_MEMBER( pet_state::pia1_pa_r )
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

	UINT8 data = 0;

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

WRITE8_MEMBER( pet_state::pia1_pa_w )
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

READ8_MEMBER( pet_state::pia1_pb_r )
{
	UINT8 data = 0xff;

	switch (m_key)
	{
	case 0: data &= m_row0->read(); break;
	case 1: data &= m_row1->read(); break;
	case 2: data &= m_row2->read(); break;
	case 3: data &= m_row3->read(); break;
	case 4: data &= m_row4->read(); break;
	case 5: data &= m_row5->read(); break;
	case 6: data &= m_row6->read(); break;
	case 7: data &= m_row7->read(); break;
	case 8: data &= m_row8->read() & m_lock->read(); break;
	case 9: data &= m_row9->read(); break;
	}

	return data;
}

READ8_MEMBER( pet2001b_state::pia1_pb_r )
{
	UINT8 data = 0xff;

	switch (m_key)
	{
	case 0: data &= m_row0->read(); break;
	case 1: data &= m_row1->read(); break;
	case 2: data &= m_row2->read(); break;
	case 3: data &= m_row3->read(); break;
	case 4: data &= m_row4->read(); break;
	case 5: data &= m_row5->read(); break;
	case 6: data &= m_row6->read() & m_lock->read(); break;
	case 7: data &= m_row7->read(); break;
	case 8: data &= m_row8->read(); break;
	case 9: data &= m_row9->read(); break;
	}

	return data;
}

WRITE_LINE_MEMBER( pet_state::pia1_ca2_w )
{
	m_ieee->eoi_w(state);

	m_blanktv = state;
}


WRITE_LINE_MEMBER( pet_state::pia2_irqa_w )
{
	m_pia2a_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( pet_state::pia2_irqb_w )
{
	m_pia2b_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( pet_state::user_diag_w )
{
	m_user_diag = state;
}



//**************************************************************************
//  VIDEO
//**************************************************************************

//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( sync_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( pet_state::sync_tick )
{
	m_sync = !m_sync;

	m_pia1->cb1_w(m_sync);
}


//-------------------------------------------------
//  SCREEN_UPDATE( pet2001 )
//-------------------------------------------------

UINT32 pet_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();

	for (int y = 0; y < 200; y++)
	{
		for (int sx = 0; sx < 40; sx++)
		{
			int sy = y / 8;
			offs_t video_addr = (sy * 40) + sx;
			UINT8 lsd = m_video_ram[video_addr];

			int ra = y & 0x07;
			offs_t char_addr = (m_graphic << 10) | ((lsd & 0x7f) << 3) | ra;
			UINT8 data = m_char_rom->base()[char_addr];

			for (int x = 0; x < 8; x++, data <<= 1)
			{
			int color = (BIT(data, 7) ^ BIT(lsd, 7)) && m_blanktv;
			bitmap.pix32(y, (sx * 8) + x) = pen[color];
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
	bitmap.fill(rgb_t::black);
}

MC6845_UPDATE_ROW( pet80_state::pet80_update_row )
{
	int x = 0;
	int char_rom_mask = m_char_rom->bytes() - 1;
	const pen_t *pen = m_palette->pens();
	hbp = 80;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 lsd = 0, data = 0;
		UINT8 rra = ra & 0x07;
		int no_row = !(BIT(ra, 3) || BIT(ra, 4));
		int invert = BIT(ma, 12);
		int chr_option = BIT(ma, 13);

		// even character

		lsd = m_video_ram[((ma + column) << 1) & 0x7ff];

		offs_t char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int video = (!((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) ^ invert) && de;
			bitmap.pix32(vbp + y, hbp + x++) = pen[video];
		}

		// odd character

		lsd = m_video_ram[(((ma + column) << 1) + 1) & 0x7ff];

		char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int video = (!((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) ^ invert) && de;
			bitmap.pix32(vbp + y, hbp + x++) = pen[video];
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
		UINT8 lsd = 0, data = 0;
		UINT8 rra = ra & 0x07;
		int no_row = !(BIT(ra, 3) || BIT(ra, 4));
		int invert = BIT(ma, 12);
		int chr_option = BIT(ma, 13);

		lsd = m_video_ram[(ma + column) & 0x3ff];

		offs_t char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int video = (!((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) ^ invert) && de;
			bitmap.pix32(vbp + y, hbp + x++) = pen[video];
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
		UINT8 lsd = 0, data = 0;
		UINT8 rra = ra & 0x07;
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
			int video = (((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) && de);
			bitmap.pix32(vbp + y, hbp + x++) = pen[video];
		}

		// odd character

		lsd = m_ram->pointer()[drma | 0x100];

		char_addr = (chr_option << 11) | (m_graphic << 10) | ((lsd & 0x7f) << 3) | rra;
		data = m_char_rom->base()[char_addr & char_rom_mask];

		for (int bit = 0; bit < 8; bit++, data <<= 1)
		{
			int video = (((BIT(data, 7) ^ BIT(lsd, 7)) && no_row) && de);
			bitmap.pix32(vbp + y, hbp + x++) = pen[video];
		}
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( cbm8296d_ieee488_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( cbm8296d_ieee488_devices )
	SLOT_INTERFACE("c8250lp", C8250LP)
SLOT_INTERFACE_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( pet )
//-------------------------------------------------

MACHINE_START_MEMBER( pet_state, pet )
{
	// allocate memory
	m_video_ram.allocate(m_video_ram_size);

	// initialize memory
	UINT8 data = 0xff;

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

	// state saving
	save_item(NAME(m_key));
	save_item(NAME(m_sync));
	save_item(NAME(m_graphic));
	save_item(NAME(m_blanktv));
	save_item(NAME(m_via_irq));
	save_item(NAME(m_pia1a_irq));
	save_item(NAME(m_pia1b_irq));
	save_item(NAME(m_pia2a_irq));
	save_item(NAME(m_pia2b_irq));
	save_item(NAME(m_exp_irq));
	save_item(NAME(m_user_diag));
}


//-------------------------------------------------
//  MACHINE_START( pet2001 )
//-------------------------------------------------

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

	m_ieee->ren_w(0);
}


//-------------------------------------------------
//  MACHINE_START( pet40 )
//-------------------------------------------------

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


//-------------------------------------------------
//  MACHINE_START( pet80 )
//-------------------------------------------------

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


//-------------------------------------------------
//  MACHINE_START( cbm8296 )
//-------------------------------------------------

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

//-------------------------------------------------
//  MACHINE_CONFIG( 4k )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( 4k )
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
	MCFG_RAM_EXTRA_OPTIONS("8K,16K,32K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( 8k )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( 8k )
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8K")
	MCFG_RAM_EXTRA_OPTIONS("16K,32K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( 8k )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( 16k )
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( 8k )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( 32k )
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet )
//-------------------------------------------------

static MACHINE_CONFIG_START( pet, pet_state )
	MCFG_MACHINE_START_OVERRIDE(pet_state, pet2001)
	MCFG_MACHINE_RESET_OVERRIDE(pet_state, pet)

	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_8MHz/8)
	MCFG_CPU_PROGRAM_MAP(pet2001_mem)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(pet_state, screen_update)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sync_timer", pet_state, sync_tick, attotime::from_hz(120))

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	// devices
	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_8MHz/8)
	MCFG_VIA6522_READPB_HANDLER(READ8(pet_state, via_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(pet_state, via_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(pet_state, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(pet_state, via_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(pet_state, via_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(pet_state, via_irq_w))

	MCFG_DEVICE_ADD(M6520_1_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(pet_state, pia1_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(pet_state, pia1_pb_r))
	MCFG_PIA_READCA1_HANDLER(DEVREADLINE(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, read))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(pet_state, pia1_pa_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(pet_state, pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, motor_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(pet_state, pia1_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(pet_state, pia1_irqb_w))

	MCFG_DEVICE_ADD(M6520_2_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(DEVREAD8(IEEE488_TAG, ieee488_device, dio_r))
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, ndac_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, dav_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(pet_state, pia2_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(pet_state, pia2_irqb_w))

	MCFG_CBM_IEEE488_ADD("c4040")
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE(M6520_2_TAG, pia6821_device, cb1_w))
	MCFG_IEEE488_ATN_CALLBACK(DEVWRITELINE(M6520_2_TAG, pia6821_device, ca1_w))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, "c2n", DEVWRITELINE(M6520_1_TAG, pia6821_device, ca1_w))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT2_TAG, cbm_datassette_devices, nullptr, DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_PET_EXPANSION_SLOT_ADD(PET_EXPANSION_SLOT_TAG, XTAL_8MHz/8, pet_expansion_cards, nullptr)
	MCFG_PET_EXPANSION_SLOT_DMA_CALLBACKS(READ8(pet_state, read), WRITE8(pet_state, write))

	MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, pet_user_port_cards, nullptr)
	MCFG_PET_USER_PORT_5_HANDLER(WRITELINE(pet_state, user_diag_w))
	MCFG_PET_USER_PORT_B_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_PET_USER_PORT_C_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa0))
	MCFG_PET_USER_PORT_D_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa1))
	MCFG_PET_USER_PORT_E_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa2))
	MCFG_PET_USER_PORT_F_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa3))
	MCFG_PET_USER_PORT_H_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa4))
	MCFG_PET_USER_PORT_J_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa5))
	MCFG_PET_USER_PORT_K_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa6))
	MCFG_PET_USER_PORT_L_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa7))
	MCFG_PET_USER_PORT_M_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_cb2))

	MCFG_QUICKLOAD_ADD("quickload", pet_state, cbm_pet, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cass_list", "pet_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "pet_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "pet_hdd")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001, pet )
	MCFG_FRAGMENT_ADD(4k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet20018 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet20018, pet )
	MCFG_FRAGMENT_ADD(8k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001n )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001n, pet )
	MCFG_GENERIC_CARTSLOT_ADD("cart_9000", generic_linear_slot, "pet_9000_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	MCFG_GENERIC_CARTSLOT_ADD("cart_a000", generic_linear_slot, "pet_a000_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	MCFG_GENERIC_CARTSLOT_ADD("cart_b000", generic_linear_slot, "pet_b000_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	MCFG_SOFTWARE_LIST_ADD("rom_list", "pet_rom")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001n8 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001n8, pet2001n )
	MCFG_FRAGMENT_ADD(8k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001n16 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001n16, pet2001n )
	MCFG_FRAGMENT_ADD(16k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001n32 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001n32, pet2001n )
	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm3000 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm3000, pet2001n )
	// video hardware
	MCFG_SCREEN_MODIFY(SCREEN_TAG)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(pet_state, screen_update)
	MCFG_DEVICE_REMOVE("sync_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sync_timer", pet_state, sync_tick, attotime::from_hz(100))
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm3008 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm3008, cbm3000 )
	MCFG_FRAGMENT_ADD(8k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm3016 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm3016, cbm3000 )
	MCFG_FRAGMENT_ADD(16k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm3032 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm3032, cbm3000 )
	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001b )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( pet2001b, pet2001n, pet2001b_state )
	MCFG_DEVICE_MODIFY(M6520_1_TAG)
	MCFG_PIA_READPB_HANDLER(READ8(pet2001b_state, pia1_pb_r))
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001b8 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001b8, pet2001b )
	MCFG_FRAGMENT_ADD(8k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001b16 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001b16, pet2001b )
	MCFG_FRAGMENT_ADD(16k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001b32 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001b32, pet2001b )
	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm3032b )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm3032b, pet2001b )
	// video hardware
	MCFG_SCREEN_MODIFY(SCREEN_TAG)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(pet_state, screen_update)
	MCFG_DEVICE_REMOVE("sync_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sync_timer", pet_state, sync_tick, attotime::from_hz(100))

	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet4000 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet4000, pet2001n )
	MCFG_DEVICE_REMOVE("cart_b000")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet4016 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet4016, pet4000 )
	// RAM not upgradeable
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet4032 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet4032, pet4000 )
	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet4032f )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet4032f, pet4000 )
	MCFG_MACHINE_START_OVERRIDE(pet_state, pet40)
	MCFG_MACHINE_RESET_OVERRIDE(pet_state, pet40)

	// video hardware
	MCFG_SCREEN_MODIFY(SCREEN_TAG)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 320 - 1, 0, 250 - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
	MCFG_DEVICE_REMOVE("sync_timer")

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, XTAL_16MHz/16)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_BEGIN_UPDATE_CB(pet_state, pet_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(pet_state, pet40_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE(M6520_1_TAG, pia6821_device, cb1_w))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm4000 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm4000, pet2001n )
	// video hardware
	MCFG_SCREEN_MODIFY(SCREEN_TAG)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(pet_state, screen_update)
	MCFG_DEVICE_REMOVE("sync_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sync_timer", pet_state, sync_tick, attotime::from_hz(100))

	MCFG_DEVICE_REMOVE("cart_b000")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm4016 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm4016, cbm4000 )
	// RAM not upgradeable
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm4032 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm4032, cbm4000 )
	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm4032f )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm4032f, cbm4000 )
	MCFG_MACHINE_START_OVERRIDE(pet_state, pet40)
	MCFG_MACHINE_RESET_OVERRIDE(pet_state, pet40)

	// video hardware
	MCFG_SCREEN_MODIFY(SCREEN_TAG)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 320 - 1, 0, 250 - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
	MCFG_DEVICE_REMOVE("sync_timer")

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, XTAL_16MHz/16)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_BEGIN_UPDATE_CB(pet_state, pet_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(pet_state, pet40_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE(M6520_1_TAG, pia6821_device, cb1_w))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet4000b )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet4000b, pet2001b )
	MCFG_DEVICE_REMOVE("cart_b000")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet4032b )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet4032b, pet4000b )
	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm4000b )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm4000b, pet2001b )
	// video hardware
	MCFG_SCREEN_MODIFY(SCREEN_TAG)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(pet_state, screen_update)
	MCFG_DEVICE_REMOVE("sync_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sync_timer", pet_state, sync_tick, attotime::from_hz(100))

	MCFG_DEVICE_REMOVE("cart_b000")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm4032b )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm4032b, cbm4000b )
	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet80 )
//-------------------------------------------------

static MACHINE_CONFIG_START( pet80, pet80_state )
	MCFG_MACHINE_START_OVERRIDE(pet80_state, pet80)
	MCFG_MACHINE_RESET_OVERRIDE(pet80_state, pet80)

	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(pet2001_mem)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 250 - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, XTAL_16MHz/16)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(2*8)
	MCFG_MC6845_BEGIN_UPDATE_CB(pet_state, pet_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(pet80_state, pet80_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE(M6520_1_TAG, pia6821_device, cb1_w))

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPB_HANDLER(READ8(pet_state, via_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(pet_state, via_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(pet_state, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(pet_state, via_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(pet_state, via_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(pet_state, via_irq_w))

	MCFG_DEVICE_ADD(M6520_1_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(pet_state, pia1_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(pet_state, pia1_pb_r))
	MCFG_PIA_READCA1_HANDLER(DEVREADLINE(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, read))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(pet_state, pia1_pa_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(pet_state, pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, motor_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(pet_state, pia1_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(pet_state, pia1_irqb_w))

	MCFG_DEVICE_ADD(M6520_2_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(DEVREAD8(IEEE488_TAG, ieee488_device, dio_r))
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, ndac_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, dav_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(pet_state, pia2_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(pet_state, pia2_irqb_w))

	MCFG_CBM_IEEE488_ADD("c8050")
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE(M6520_2_TAG, pia6821_device, cb1_w))
	MCFG_IEEE488_ATN_CALLBACK(DEVWRITELINE(M6520_2_TAG, pia6821_device, ca1_w))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, "c2n", DEVWRITELINE(M6520_1_TAG, pia6821_device, ca1_w))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT2_TAG, cbm_datassette_devices, nullptr, DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_PET_EXPANSION_SLOT_ADD(PET_EXPANSION_SLOT_TAG, XTAL_16MHz/16, pet_expansion_cards, nullptr)
	MCFG_PET_EXPANSION_SLOT_DMA_CALLBACKS(READ8(pet_state, read), WRITE8(pet_state, write))

	MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, pet_user_port_cards, nullptr)
	MCFG_PET_USER_PORT_B_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_PET_USER_PORT_C_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa0))
	MCFG_PET_USER_PORT_D_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa1))
	MCFG_PET_USER_PORT_E_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa2))
	MCFG_PET_USER_PORT_F_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa3))
	MCFG_PET_USER_PORT_H_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa4))
	MCFG_PET_USER_PORT_J_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa5))
	MCFG_PET_USER_PORT_K_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa6))
	MCFG_PET_USER_PORT_L_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_pa7))
	MCFG_PET_USER_PORT_M_HANDLER(DEVWRITELINE(M6522_TAG, via6522_device, write_cb2))

	MCFG_QUICKLOAD_ADD("quickload", pet_state, cbm_pet, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	MCFG_GENERIC_CARTSLOT_ADD("cart_9000", generic_linear_slot, "pet_9000_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	MCFG_GENERIC_CARTSLOT_ADD("cart_a000", generic_linear_slot, "pet_a000_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cass_list", "pet_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "pet_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "pet_hdd")
	MCFG_SOFTWARE_LIST_ADD("rom_list", "pet_rom")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet8032 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet8032, pet80 )
	MCFG_FRAGMENT_ADD(32k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( superpet )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( superpet, pet8032, superpet_state )
	MCFG_DEVICE_REMOVE(PET_EXPANSION_SLOT_TAG)
	MCFG_PET_EXPANSION_SLOT_ADD(PET_EXPANSION_SLOT_TAG, XTAL_16MHz/16, pet_expansion_cards, "superpet")
	MCFG_PET_EXPANSION_SLOT_DMA_CALLBACKS(READ8(pet_state, read), WRITE8(pet_state, write))

	MCFG_SOFTWARE_LIST_ADD("flop_list2", "superpet_flop")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm8096 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( cbm8096, pet80, cbm8096_state )
	MCFG_DEVICE_REMOVE(PET_EXPANSION_SLOT_TAG)
	MCFG_PET_EXPANSION_SLOT_ADD(PET_EXPANSION_SLOT_TAG, XTAL_16MHz/16, pet_expansion_cards, "64k")
	MCFG_PET_EXPANSION_SLOT_DMA_CALLBACKS(READ8(pet_state, read), WRITE8(pet_state, write))

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("96K")

	MCFG_SOFTWARE_LIST_ADD("flop_list2", "cbm8096_flop")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm8296 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( cbm8296, pet80, cbm8296_state )
	MCFG_MACHINE_START_OVERRIDE(cbm8296_state, cbm8296)
	MCFG_MACHINE_RESET_OVERRIDE(cbm8296_state, cbm8296)

	MCFG_CPU_MODIFY(M6502_TAG)
	MCFG_CPU_PROGRAM_MAP(cbm8296_mem)

	MCFG_PLS100_ADD(PLA1_TAG)
	MCFG_PLS100_ADD(PLA2_TAG)

	MCFG_DEVICE_REMOVE(MC6845_TAG)
	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, XTAL_16MHz/16)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(2*8)
	MCFG_MC6845_UPDATE_ROW_CB(pet80_state, cbm8296_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE(M6520_1_TAG, pia6821_device, cb1_w))

	MCFG_DEVICE_MODIFY("ieee8")
	MCFG_SLOT_DEFAULT_OPTION("c8250")

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")

	MCFG_SOFTWARE_LIST_ADD("flop_list2", "cbm8296_flop")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm8296d )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm8296d, cbm8296 )
	MCFG_DEVICE_MODIFY("ieee8")
	MCFG_DEVICE_SLOT_INTERFACE(cbm8296d_ieee488_devices, "c8250lp", false)
MACHINE_CONFIG_END



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
	ROMX_LOAD( "901447-01.h1", 0x3000, 0x0800, CRC(a055e33a) SHA1(831db40324113ee996c434d38b4add3fd1f820bd), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic1r", "Revised" )
	ROMX_LOAD( "901447-09.h1", 0x3000, 0x0800, CRC(03cf16d0) SHA1(1330580c0614d3556a389da4649488ba04a60908), ROM_BIOS(2) )
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
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(2) ) // BASIC 4
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
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(2) ) // BASIC 4
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
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(2) ) // BASIC 4
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
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(2) ) // BASIC 4
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
	ROMX_LOAD( "901465-19.ud5", 0x2000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic4r", "Revised" )
	ROMX_LOAD( "901465-23.ud5", 0x2000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(2) ) // BASIC 4
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



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT                COMPANY                         FULLNAME        FLAGS
COMP( 1977, pet2001,    0,          0,      pet2001,    pet,        driver_device,  0,  "Commodore Business Machines",  "PET 2001-4",   MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1977, pet20018,   pet2001,    0,      pet20018,   pet,        driver_device,  0,  "Commodore Business Machines",  "PET 2001-8",   MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001n,   0,          0,      pet2001n8,  pet,        driver_device,  0,  "Commodore Business Machines",  "PET 2001-N8",  MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001n16, pet2001n,   0,      pet2001n16, pet,        driver_device,  0,  "Commodore Business Machines",  "PET 2001-N16", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001n32, pet2001n,   0,      pet2001n32, pet,        driver_device,  0,  "Commodore Business Machines",  "PET 2001-N32", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, cbm3008,    pet2001n,   0,      cbm3008,    pet,        driver_device,  0,  "Commodore Business Machines",  "CBM 3008",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, cbm3016,    pet2001n,   0,      cbm3016,    pet,        driver_device,  0,  "Commodore Business Machines",  "CBM 3016",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, cbm3032,    pet2001n,   0,      cbm3032,    pet,        driver_device,  0,  "Commodore Business Machines",  "CBM 3032",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001b,   0,          0,      pet2001b8,  petb,       driver_device,  0,  "Commodore Business Machines",  "PET 2001-B8",  MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001b16, pet2001b,   0,      pet2001b16, petb,       driver_device,  0,  "Commodore Business Machines",  "PET 2001-B16", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, pet2001b32, pet2001b,   0,      pet2001b32, petb,       driver_device,  0,  "Commodore Business Machines",  "PET 2001-B32", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, cbm3032b,   pet2001b,   0,      cbm3032b,   petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 3032B",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, pet4016,    0,          0,      pet4016,    pet,        driver_device,  0,  "Commodore Business Machines",  "PET 4016",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, pet4032,    pet4016,    0,      pet4032,    pet,        driver_device,  0,  "Commodore Business Machines",  "PET 4032",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, pet4032f,   pet4016,    0,      pet4032f,   pet,        driver_device,  0,  "Commodore Business Machines",  "PET 4032 (Fat 40)",     MACHINE_SUPPORTS_SAVE )
COMP( 1980, cbm4016,    pet4016,    0,      cbm4016,    pet,        driver_device,  0,  "Commodore Business Machines",  "CBM 4016",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, cbm4032,    pet4016,    0,      cbm4032,    pet,        driver_device,  0,  "Commodore Business Machines",  "CBM 4032",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, cbm4032f,   pet4016,    0,      cbm4032f,   pet,        driver_device,  0,  "Commodore Business Machines",  "CBM 4032 (Fat 40)",     MACHINE_SUPPORTS_SAVE )
COMP( 1980, pet4032b,   0,          0,      pet4032b,   petb,       driver_device,  0,  "Commodore Business Machines",  "PET 4032B",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, cbm4032b,   pet4032b,   0,      cbm4032b,   petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 4032B",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, pet8032,    0,          0,      pet8032,    petb,       driver_device,  0,  "Commodore Business Machines",  "PET 8032",     MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8032,    pet8032,    0,      pet8032,    petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 8032",     MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8032_de, pet8032,    0,      pet8032,    petb_de,    driver_device,  0,  "Commodore Business Machines",  "CBM 8032 (Germany)",           MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8032_fr, pet8032,    0,      pet8032,    petb_fr,    driver_device,  0,  "Commodore Business Machines",  "CBM 8032 (France)",            MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8032_se, pet8032,    0,      pet8032,    petb_se,    driver_device,  0,  "Commodore Business Machines",  "CBM 8032 (Sweden/Finland)",    MACHINE_SUPPORTS_SAVE )
COMP( 1981, superpet,   pet8032,    0,      superpet,   petb,       driver_device,  0,  "Commodore Business Machines",  "SuperPET SP-9000",             MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1981, mmf9000,    pet8032,    0,      superpet,   petb,       driver_device,  0,  "Commodore Business Machines",  "MicroMainFrame 9000",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1981, mmf9000_se, pet8032,    0,      superpet,   petb_se,    driver_device,  0,  "Commodore Business Machines",  "MicroMainFrame 9000 (Sweden/Finland)",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1981, cbm8096,    pet8032,    0,      cbm8096,    petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 8096",                     MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296,    0,          0,      cbm8296,    petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 8296",                     MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296ed,  cbm8296,    0,      cbm8296d,   petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 8296 ExecuDesk",           MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296d,   cbm8296,    0,      cbm8296d,   petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 8296-D",                   MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296d_de,cbm8296,    0,      cbm8296d,   petb_de,    driver_device,  0,  "Commodore Business Machines",  "CBM 8296-D (Germany)",         MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296gd,  cbm8296,    0,      cbm8296d,   petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 8296GD",                   MACHINE_SUPPORTS_SAVE )
COMP( 1984, cbm8296dgv_de,cbm8296,    0,      cbm8296d,   petb,       driver_device,  0,  "Commodore Business Machines",  "CBM 8296-D GV? (Germany)",        MACHINE_SUPPORTS_SAVE )
