/*

    TODO:

    - connect CAPS LOCK to charom A12 on international variants
    - remove frame interrupt handler
    - expansion DMA

*/

#include "includes/c128.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define A15 BIT(offset, 15)
#define A14 BIT(offset, 14)
#define A13 BIT(offset, 13)
#define A12 BIT(offset, 12)
#define A11 BIT(offset, 11)
#define A10 BIT(offset, 10)
#define VMA5 BIT(vma, 13)
#define VMA4 BIT(vma, 12)



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

inline void c128_state::check_interrupts()
{
	int restore = BIT(ioport("SPECIAL")->read(), 7);

	int irq = m_cia1_irq || m_vic_irq || m_exp_irq;
	int nmi = m_cia2_irq || restore || m_exp_nmi;
	//int aec = m_exp_dma && m_z80_busack;
	//int rdy = m_vic_aec && m_z80en && m_vic_ba;
	//int busreq = !m_z80en || !(m_z80_busack && !aec)

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, irq);

	m_subcpu->set_input_line(M8502_IRQ_LINE, irq);
	m_subcpu->set_input_line(INPUT_LINE_NMI, nmi);

	int flag = m_cass_rd && m_iec_srq;

	m_cia1->flag_w(flag);
}



//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read_pla -
//-------------------------------------------------

void c128_state::read_pla(offs_t offset, offs_t ca, offs_t vma, int ba, int rw, int aec, int z80io, int ms3, int ms2, int ms1, int ms0,
		int *sden, int *dir, int *gwe, int *rom1, int *rom2, int *rom3, int *rom4, int *charom, int *colorram, int *vic,
		int *from1, int *romh, int *roml, int *dwe, int *ioacc, int *clrbank, int *iocs, int *casenb)
{
	int _128_256 = 1;
	int dmaack = 1;
	int vicfix = 1;
	int sphi2 = m_vic->phi0_r();

	m_game = m_exp->game_r(ca, sphi2, ba, rw, m_hiram);
	m_exrom = m_exp->exrom_r(ca, sphi2, ba, rw, m_hiram);

	UINT32 input = sphi2 << 26 | m_va14 << 25 | m_charen << 24 |
		m_hiram << 23 | m_loram << 22 | ba << 21 | VMA5 << 20 | VMA4 << 19 | ms0 << 18 | ms1 << 17 | ms2 << 16 |
		m_exrom << 15 | m_game << 14 | rw << 13 | aec << 12 | A10 << 11 | A11 << 10 | A12 << 9 | A13 << 8 |
		A14 << 7 | A15 << 6 | z80io << 5 | m_z80en << 4 | ms3 << 3 | vicfix << 2 | dmaack << 1 | _128_256;

	UINT32 data = m_pla->read(input);

	*sden = BIT(data, 0);
	*rom4 = BIT(data, 1);
	*rom2 = BIT(data, 2);
	*dir = BIT(data, 3);
	*roml = BIT(data, 4);
	*romh = BIT(data, 5);
	*clrbank = BIT(data, 6);
	*from1 = BIT(data, 7);
	*rom3 = BIT(data, 8);
	*rom1 = BIT(data, 9);
	*iocs = BIT(data, 10);
	*dwe = BIT(data, 11);
	*casenb = BIT(data, 12);
	*vic = BIT(data, 13);
	*ioacc = BIT(data, 14);
	*gwe = BIT(data, 15);
	*colorram = BIT(data, 16);
	*charom = BIT(data, 17);

	m_clrbank = *clrbank;
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

UINT8 c128_state::read_memory(address_space &space, offs_t offset, offs_t vma, int ba, int aec, int z80io)
{
	int rw = 1, ms0 = 1, ms1 = 1, ms2 = 1, ms3 = 1, cas0 = 1, cas1 = 1;
	int sden = 1, dir = 1, gwe = 1, rom1 = 1, rom2 = 1, rom3 = 1, rom4 = 1, charom = 1, colorram = 1, vic = 1,
		from1 = 1, romh = 1, roml = 1, dwe = 1, ioacc = 1, clrbank = 1, iocs = 1, casenb = 1;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	UINT8 data = 0xff;

	offs_t ta = m_mmu->ta_r(offset, aec, &ms0, &ms1, &ms2, &ms3, &cas0, &cas1);
	offs_t ma = 0;
	offs_t sa = 0;

	if (aec)
	{
		data = m_vic->bus_r();
		ma = ta | (offset & 0xff);
		sa = offset & 0xff;
	}
	else
	{
		ta &= ~0xf00;
		ta |= (vma & 0xf00);
		ma = (!m_va15 << 15) | (!m_va14 << 14) | vma;
		sa = vma & 0xff;
	}

	offs_t ca = ta | sa;

	read_pla(offset, ca, vma, ba, rw, aec, z80io, ms3, ms2, ms1, ms0,
		&sden, &dir, &gwe, &rom1, &rom2, &rom3, &rom4, &charom, &colorram, &vic,
		&from1, &romh, &roml, &dwe, &ioacc, &clrbank, &iocs, &casenb);

	if (!casenb)
	{
		if (!cas0)
		{
			data = m_ram->pointer()[ma];
		}
		if (!cas1)
		{
			data = m_ram->pointer()[0x10000 | ma];
		}
	}
	if (!rom1)
	{
		// CR: data = m_rom1[(ms3 << 14) | ((BIT(ta, 14) && BIT(offset, 13)) << 13) | (ta & 0x1000) | (offset & 0xfff)];
		data = m_rom1[((BIT(ta, 14) && BIT(offset, 13)) << 13) | (ta & 0x1000) | (offset & 0xfff)];
	}
	if (!rom2)
	{
		data = m_rom2[offset & 0x3fff];
	}
	if (!rom3)
	{
		// CR: data = m_rom3[(BIT(offset, 15) << 14) | (offset & 0x3fff)];
		data = m_rom3[offset & 0x3fff];
	}
	if (!rom4)
	{
		data = m_rom4[(ta & 0x1000) | (offset & 0x2fff)];
	}
	if (!charom)
	{
		data = m_charom[(ms3 << 12) | (ta & 0xf00) | sa];
	}
	if (!colorram && aec)
	{
		data = m_color_ram[(clrbank << 10) | (ta & 0x300) | sa] & 0x0f;
	}
	if (!vic)
	{
		data = m_vic->read(space, offset & 0x3f);
	}
	if (!from1)
	{
		data = m_from[offset & 0x7fff];
	}
	if (!iocs && BIT(offset, 10))
	{
		switch ((BIT(offset, 11) << 2) | ((offset >> 8) & 0x03))
		{
		case 0: // SID
			data = m_sid->read(space, offset & 0x1f);
			break;

		case 2: // CS8563
			if BIT(offset, 0)
			{
				data = m_vdc->register_r(space, 0);
			}
			else
			{
				data = m_vdc->status_r(space, 0);
			}
			break;

		case 4: // CIA1
			data = m_cia1->read(space, offset & 0x0f);
			break;

		case 5: // CIA2
			data = m_cia2->read(space, offset & 0x0f);
			break;

		case 6: // I/O1
			io1 = 0;
			break;

		case 7: // I/O2
			io2 = 0;
			break;
		}
	}

	data = m_exp->cd_r(space, ca, data, sphi2, ba, roml, romh, io1, io2);

	return m_mmu->read(offset, data);
}


//-------------------------------------------------
//  write_memory -
//-------------------------------------------------

void c128_state::write_memory(address_space &space, offs_t offset, offs_t vma, UINT8 data, int ba, int aec, int z80io)
{
	int rw = 0, ms0 = 1, ms1 = 1, ms2 = 1, ms3 = 1, cas0 = 1, cas1 = 1;
	int sden = 1, dir = 1, gwe = 1, rom1 = 1, rom2 = 1, rom3 = 1, rom4 = 1, charom = 1, colorram = 1, vic = 1,
		from1 = 1, romh = 1, roml = 1, dwe = 1, ioacc = 1, clrbank = 1, iocs = 1, casenb = 1;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	offs_t ta = m_mmu->ta_r(offset, aec, &ms0, &ms1, &ms2, &ms3, &cas0, &cas1);
	offs_t ca = ta | (offset & 0xff);
	offs_t ma = ta | (offset & 0xff);
	offs_t sa = offset & 0xff;

	read_pla(offset, ca, vma, ba, rw, aec, z80io, ms3, ms2, ms1, ms0,
		&sden, &dir, &gwe, &rom1, &rom2, &rom3, &rom4, &charom, &colorram, &vic,
		&from1, &romh, &roml, &dwe, &ioacc, &clrbank, &iocs, &casenb);

	if (!casenb && !dwe)
	{
		if (!cas0)
		{
			m_ram->pointer()[ma] = data;
		}
		if (!cas1)
		{
			m_ram->pointer()[0x10000 | ma] = data;
		}
	}
	if (!colorram && !gwe)
	{
		m_color_ram[(clrbank << 10) | (ta & 0x300) | sa] = data & 0x0f;
	}
	if (!vic)
	{
		m_vic->write(space, offset & 0x3f, data);
	}
	if (!iocs && BIT(offset, 10))
	{
		switch ((BIT(offset, 11) << 2) | ((offset >> 8) & 0x03))
		{
		case 0: // SID
			m_sid->write(space, offset & 0x1f, data);
			break;

		case 2: // CS8563
			if BIT(offset, 0)
			{
				m_vdc->register_w(space, 0, data);
			}
			else
			{
				m_vdc->address_w(space, 0, data);
			}
			break;

		case 4: // CIA1
			m_cia1->write(space, offset & 0x0f, data);
			break;

		case 5: // CIA2
			m_cia2->write(space, offset & 0x0f, data);
			break;

		case 6: // I/O1
			io1 = 0;
			break;

		case 7: // I/O2
			io2 = 0;
			break;
		}
	}

	m_exp->cd_w(space, ca, data, sphi2, ba, roml, romh, io1, io2);

	m_mmu->write(space, offset, data);
}


//-------------------------------------------------
//  z80_r -
//-------------------------------------------------

READ8_MEMBER( c128_state::z80_r )
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(space, offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_w -
//-------------------------------------------------

WRITE8_MEMBER( c128_state::z80_w )
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	write_memory(space, offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_io_r -
//-------------------------------------------------

READ8_MEMBER( c128_state::z80_io_r )
{
	int ba = 1, aec = 1, z80io = 0;
	offs_t vma = 0;

	return read_memory(space, offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_io_w -
//-------------------------------------------------

WRITE8_MEMBER( c128_state::z80_io_w )
{
	int ba = 1, aec = 1, z80io = 0;
	offs_t vma = 0;

	write_memory(space, offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( c128_state::read )
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(space, offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( c128_state::write )
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	write_memory(space, offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

READ8_MEMBER( c128_state::vic_videoram_r )
{
	int ba = 0, aec = 0, z80io = 1;

	return read_memory(space, 0, offset, ba, aec, z80io);
}


//-------------------------------------------------
//  vic_colorram_r -
//-------------------------------------------------

READ8_MEMBER( c128_state::vic_colorram_r )
{
	return m_color_ram[(m_clrbank << 10) | offset];
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_mem, AS_PROGRAM, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(z80_r, z80_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_io, AS_IO, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(z80_io_r, z80_io_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m8502_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( m8502_mem, AS_PROGRAM, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, c128_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_videoram_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, c128_state )
	AM_RANGE(0x000, 0x3ff) AM_READ(vic_colorram_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vdc_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vdc_videoram_map, AS_0, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( c128 )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c128_state::restore )
{
	check_interrupts();
}

INPUT_CHANGED_MEMBER( c128_state::caps_lock )
{
	m_caps_lock = newval;
}

static INPUT_PORTS_START( c128 )
	PORT_INCLUDE( common_cbm_keyboard )

	PORT_START( "K0" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)             PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_F6)               PORT_CHAR('\t')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(PGUP))

	PORT_START( "K1" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)          PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_F5)               PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START( "K2" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NO SCROLL") PORT_CODE(KEYCODE_F12) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)             PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)              PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)              PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)                PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)             PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START( "SPECIAL" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_PRTSCR) PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, restore, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("40/80 DISPLAY") PORT_CODE(KEYCODE_F11) PORT_TOGGLE

	PORT_INCLUDE( c64_controls )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128ger )
//-------------------------------------------------

static INPUT_PORTS_START( c128ger )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z  { Y }") PORT_CODE(KEYCODE_Z)                   PORT_CHAR('Z')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y  { Z }") PORT_CODE(KEYCODE_Y)                   PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  '  { 7  / }") PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0  { = }") PORT_CODE(KEYCODE_0)                   PORT_CHAR('0')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",  <  { ; }") PORT_CODE(KEYCODE_COMMA)            PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Paragraph  \xE2\x86\x91  { \xc3\xbc }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00A7) PORT_CHAR(0x2191)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".  >  { : }") PORT_CODE(KEYCODE_STOP)             PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-  { '  ` }") PORT_CODE(KEYCODE_EQUALS)           PORT_CHAR('-')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+  { \xc3\x9f ? }") PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('+')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/  ?  { -  _ }") PORT_CODE(KEYCODE_SLASH)                 PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Sum  Pi  { ] \\ }") PORT_CODE(KEYCODE_DEL)                PORT_CHAR(0x03A3) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  { # ' }") PORT_CODE(KEYCODE_BACKSLASH)                 PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  ]  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE)             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("*  `  { +  * }") PORT_CODE(KEYCODE_CLOSEBRACE)            PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\  { [  \xE2\x86\x91 }") PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)                    PORT_CHAR('_')

	PORT_MODIFY( "SPECIAL" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ASCII/DIN") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128fra )
//-------------------------------------------------
#ifdef UNUSED_CODE
static INPUT_PORTS_START( c128fra )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z  { W }") PORT_CODE(KEYCODE_Z)               PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4  $  { '  4 }") PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A  { Q }") PORT_CODE(KEYCODE_A)               PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W  { Z }") PORT_CODE(KEYCODE_W)               PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { \"  3 }") PORT_CODE(KEYCODE_3)        PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6  &  { Paragraph  6 }") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5  %  { (  5 }") PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8  (  { !  8 }") PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  '  { \xc3\xa8  7 }") PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K  Large-  { \\ }") PORT_CODE(KEYCODE_K)      PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M  Large-/  { ,  ? }") PORT_CODE(KEYCODE_M)   PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0  { \xc3\xa0  0 }") PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9  )  { \xc3\xa7  9 }") PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",  <  { ;  . }") PORT_CODE(KEYCODE_COMMA)                     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@  \xc3\xbb  { ^  \xc2\xa8 }") PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@') PORT_CHAR(0x00FB)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xb9  % }") PORT_CODE(KEYCODE_COLON)              PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".  >  { :  / }") PORT_CODE(KEYCODE_STOP)                      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-  \xc2\xb0  { -  _ }") PORT_CODE(KEYCODE_EQUALS)             PORT_CHAR('-') PORT_CHAR('\xB0')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+  \xc3\xab  { )  \xc2\xb0 }") PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('+') PORT_CHAR(0x00EB)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/  ?  { =  + }") PORT_CODE(KEYCODE_SLASH)                     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi  { *  ] }") PORT_CODE(KEYCODE_DEL)           PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  {\xE2\x86\x91  \\ }") PORT_CODE(KEYCODE_BACKSLASH)         PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  ]  { M  Large-/ }") PORT_CODE(KEYCODE_QUOTE)               PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("*  `  { $  [ }") PORT_CODE(KEYCODE_CLOSEBRACE)                PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\  { @  # }") PORT_CODE(KEYCODE_BACKSLASH)                   PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q  { A }") PORT_CODE(KEYCODE_Q)               PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2  \"  { \xc3\xa9  2 }") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_   { <  > }") PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1  !  { &  1 }") PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!')

	PORT_MODIFY( "SPECIAL" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK ASCII/CC") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
INPUT_PORTS_END
#endif

//-------------------------------------------------
//  INPUT_PORTS( c128ita )
//-------------------------------------------------
#ifdef UNUSED_CODE
static INPUT_PORTS_START( c128ita )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z  { W }") PORT_CODE(KEYCODE_Z)                       PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4  $  { '  4 }") PORT_CODE(KEYCODE_4)                 PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W  { Z }") PORT_CODE(KEYCODE_W)                       PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { \"  3 }") PORT_CODE(KEYCODE_3)                PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6  &  { _  6 }") PORT_CODE(KEYCODE_6)                 PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5  %  { (  5 }") PORT_CODE(KEYCODE_5)                 PORT_CHAR('5') PORT_CHAR('%')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8  (  { &  8 }") PORT_CODE(KEYCODE_8)                 PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  '  { \xc3\xa8  7 }") PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M  Large-/  { ,  ? }") PORT_CODE(KEYCODE_M)           PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0  { \xc3\xa0  0 }") PORT_CODE(KEYCODE_0)             PORT_CHAR('0')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9  )  { \xc3\xa7  9 }") PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",  <   { ;  . }") PORT_CODE(KEYCODE_COMMA)            PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@  \xc3\xbb  { \xc3\xac  = }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR(0x00FB)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xb9  % }") PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".  >  { :  / }") PORT_CODE(KEYCODE_STOP)              PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-  \xc2\xb0  { -  + }") PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-') PORT_CHAR('\xb0')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+  \xc3\xab  { )  \xc2\xb0 }") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR(0x00EB)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/  ?  { \xc3\xb2  ! }") PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi  { *  ] }") PORT_CODE(KEYCODE_DEL)   PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  { \xE2\x86\x91  \\ }") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  ]  { M }") PORT_CODE(KEYCODE_QUOTE)                PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("*  `  { $  [ }") PORT_CODE(KEYCODE_CLOSEBRACE)        PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\  { @  # }") PORT_CODE(KEYCODE_BACKSLASH2)          PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2  \"  { \xc3\xa9  2 }") PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)                PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1  !  { \xc2\xa3  1 }") PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
INPUT_PORTS_END
#endif

//-------------------------------------------------
//  INPUT_PORTS( c128swe )
//-------------------------------------------------

static INPUT_PORTS_START( c128swe )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  '  { 7  / }") PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("]  { \xc3\xa2 }") PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR(']')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)        PORT_CHAR('[')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                                    PORT_CHAR('=')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                                     PORT_CHAR('-')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  +") PORT_CODE(KEYCODE_BACKSLASH)               PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc2\xa3  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\xA3')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_CLOSEBRACE)                 PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  *") PORT_CODE(KEYCODE_BACKSLASH2)              PORT_CHAR(':') PORT_CHAR('*')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)            PORT_CHAR('_')

	PORT_MODIFY( "SPECIAL" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK ASCII/CC") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MOS8722_INTERFACE( mmu_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::mmu_z80en_w )
{
	if (state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

		if (m_reset)
		{
			m_subcpu->reset();

			m_reset = 0;
		}
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	m_z80en = state;
}

WRITE_LINE_MEMBER( c128_state::mmu_fsdir_w )
{
	update_iec();
}

READ_LINE_MEMBER( c128_state::mmu_game_r )
{
	return m_game;
}

READ_LINE_MEMBER( c128_state::mmu_exrom_r )
{
	return m_exrom;
}

READ_LINE_MEMBER( c128_state::mmu_sense40_r )
{
	return BIT(ioport("SPECIAL")->read(), 4);
}

static MOS8722_INTERFACE( mmu_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(c128_state, mmu_z80en_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, mmu_fsdir_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, mmu_game_r),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, mmu_exrom_r),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, mmu_sense40_r)
};


//-------------------------------------------------
//  mc6845_interface vdc_intf
//-------------------------------------------------

static GFXDECODE_START( c128 )
	GFXDECODE_ENTRY( "charom", 0x0000, gfx_8x8x1, 0, 1 )
GFXDECODE_END


static MC6845_INTERFACE( vdc_intf )
{
	SCREEN_VDC_TAG,
	false,
	8,
	NULL,
	NULL,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};


//-------------------------------------------------
//  MOS8564_INTERFACE( vic_intf )
//-------------------------------------------------

INTERRUPT_GEN_MEMBER( c128_state::frame_interrupt )
{
	check_interrupts();

	cbm_common_interrupt(&device);
}

WRITE_LINE_MEMBER( c128_state::vic_irq_w )
{
	m_vic_irq = state;

	check_interrupts();
}

WRITE8_MEMBER( c128_state::vic_k_w )
{
	m_vic_k = data;
}

static MOS8564_INTERFACE( vic_intf )
{
	SCREEN_VIC_TAG,
	M8502_TAG,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, vic_irq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c128_state, vic_k_w)
};


//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

READ8_MEMBER( c128_state::sid_potx_r )
{
	UINT8 data = 0xff;

	switch (m_cia1->pa_r() >> 6)
	{
	case 1: data = m_joy1->pot_x_r(); break;
	case 2: data = m_joy2->pot_x_r(); break;
	case 3: break; // TODO pot1 and pot2 in series
	}

	return data;
}

READ8_MEMBER( c128_state::sid_poty_r )
{
	UINT8 data = 0xff;

	switch (m_cia1->pa_r() >> 6)
	{
	case 1: data = m_joy1->pot_y_r(); break;
	case 2: data = m_joy2->pot_y_r(); break;
	case 3: break; // TODO pot1 and pot2 in series
	}

	return data;
}

static MOS6581_INTERFACE( sid_intf )
{
	DEVCB_DRIVER_MEMBER(c128_state, sid_potx_r),
	DEVCB_DRIVER_MEMBER(c128_state, sid_poty_r)
};


//-------------------------------------------------
//  MOS6526_INTERFACE( cia1_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::cia1_irq_w )
{
	m_cia1_irq = state;

	check_interrupts();
}

READ8_MEMBER( c128_state::cia1_pa_r )
{
	/*

	    bit     description

	    PA0     COL0, JOY B0
	    PA1     COL1, JOY B1
	    PA2     COL2, JOY B2
	    PA3     COL3, JOY B3
	    PA4     COL4, BTNB
	    PA5     COL5
	    PA6     COL6
	    PA7     COL7

	*/

	UINT8 cia0portb = m_cia1->pb_r();

	return cbm_common_cia0_port_a_r(m_cia1, cia0portb);
}

READ8_MEMBER( c128_state::cia1_pb_r )
{
	/*

	    bit     description

	    PB0     JOY A0
	    PB1     JOY A1
	    PB2     JOY A2
	    PB3     JOY A3
	    PB4     BTNA/_LP
	    PB5
	    PB6
	    PB7

	*/

	UINT8 data = 0xff;
	UINT8 cia0porta = m_cia1->pa_r();

	data &= cbm_common_cia0_port_b_r(m_cia1, cia0porta);

	if (!BIT(m_vic_k, 0)) data &= ~ioport("K0")->read();
	if (!BIT(m_vic_k, 1)) data &= ~ioport("K1")->read();
	if (!BIT(m_vic_k, 2)) data &= ~ioport("K2")->read();

	return data;
}

WRITE8_MEMBER( c128_state::cia1_pb_w )
{
	/*

	    bit     description

	    PB0     ROW0
	    PB1     ROW1
	    PB2     ROW2
	    PB3     ROW3
	    PB4     ROW4
	    PB5     ROW5
	    PB6     ROW6
	    PB7     ROW7

	*/

	m_vic->lp_w(BIT(data, 4));
}

WRITE_LINE_MEMBER( c128_state::cia1_cnt_w )
{
	m_cnt1 = state;

	update_iec();
}

WRITE_LINE_MEMBER( c128_state::cia1_sp_w )
{
	m_sp1 = state;

	update_iec();
}

static MOS6526_INTERFACE( cia1_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_irq_w),
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_cnt_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_sp_w),
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pa_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pb_r),
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pb_w)
};


//-------------------------------------------------
//  MOS6526_INTERFACE( cia2_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::cia2_irq_w )
{
	m_cia2_irq = state;

	check_interrupts();
}

READ8_MEMBER( c128_state::cia2_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2     USER PORT
	    PA3
	    PA4
	    PA5
	    PA6     CLK
	    PA7     DATA

	*/

	UINT8 data = 0;

	// user port
	data |= m_user->pa2_r() << 2;

	// IEC bus
	data |= m_iec->clk_r() << 6;
	data |= m_iec->data_r() << 7;

	return data;
}

WRITE8_MEMBER( c128_state::cia2_pa_w )
{
	/*

	    bit     description

	    PA0     _VA14
	    PA1     _VA15
	    PA2     USER PORT
	    PA3     ATN OUT
	    PA4     CLK OUT
	    PA5     DATA OUT
	    PA6
	    PA7

	*/

	// VIC banking
	m_va14 = BIT(data, 0);
	m_va15 = BIT(data, 1);

	// user port
	m_user->pa2_w(BIT(data, 2));

	// IEC bus
	m_iec->atn_w(!BIT(data, 3));
	m_iec->clk_w(!BIT(data, 4));
	m_iec_data_out = BIT(data, 5);

	update_iec();
}

static MOS6526_INTERFACE( cia2_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia2_irq_w),
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, pc2_w),
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, sp2_w),
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, cnt2_w),
	DEVCB_DRIVER_MEMBER(c128_state, cia2_pa_r),
	DEVCB_DRIVER_MEMBER(c128_state, cia2_pa_w),
	DEVCB_DEVICE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, pb_r),
	DEVCB_DEVICE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, pb_w)
};


//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

READ8_MEMBER( c128_state::cpu_r)
{
	/*

	    bit     description

	    P0      1
	    P1      1
	    P2      1
	    P3
	    P4      CASS SENSE
	    P5
	    P6      CAPS LOCK

	*/

	UINT8 data = 0x07;

	// cassette sense
	data |= m_cassette->sense_r() << 4;

	// CAPS LOCK
	data |= m_caps_lock << 6;

	return data;
}

WRITE8_MEMBER( c128_state::cpu_w )
{
	/*

	    bit     description

	    P0      LORAM
	    P1      HIRAM
	    P2      CHAREN
	    P3      CASS WRT
	    P4
	    P5      CASS MOTOR
	    P6

	*/

	// memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);

	// cassette write
	m_cassette->write(BIT(data, 3));

	// cassette motor
	m_cassette->motor_w(BIT(data, 5));
}


//-------------------------------------------------
//  CBM_IEC_INTERFACE( cbm_iec_intf )
//-------------------------------------------------

inline void c128_state::update_iec()
{
	int fsdir = m_mmu->fsdir_r();

	// fast serial data in
	int data_in = m_iec->data_r();

	m_cia1->sp_w(fsdir || data_in);

	// fast serial data out
	int data_out = !m_iec_data_out;

	if (fsdir) data_out &= m_sp1;

	m_iec->data_w(data_out);

	// fast serial clock in
	int srq_in = m_iec->srq_r();

	m_cia1->cnt_w(fsdir || srq_in);

	// fast serial clock out
	int srq_out = 1;

	if (fsdir) srq_out &= m_cnt1;

	m_iec->srq_w(srq_out);
}

WRITE_LINE_MEMBER( c128_state::iec_srq_w )
{
	update_iec();
}

WRITE_LINE_MEMBER( c128_state::iec_data_w )
{
	update_iec();
}

static CBM_IEC_INTERFACE( cbm_iec_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(c128_state, iec_srq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, iec_data_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::tape_read_w )
{
	m_cass_rd = state;

	check_interrupts();
}

static PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(c128_state, tape_read_w),
};


//-------------------------------------------------
//  C64_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

READ8_MEMBER( c128_state::exp_dma_r )
{
	int ba = 0, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(space, offset, vma, ba, aec, z80io);
}

WRITE8_MEMBER( c128_state::exp_dma_w )
{
	int ba = 0, aec = 1, z80io = 1;
	offs_t vma = 0;

	return write_memory(space, offset, data, vma, ba, aec, z80io);
}

WRITE_LINE_MEMBER( c128_state::exp_irq_w )
{
	m_exp_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c128_state::exp_nmi_w )
{
	m_exp_nmi = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c128_state::exp_dma_w )
{
	m_exp_dma = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c128_state::exp_reset_w )
{
	if (state == ASSERT_LINE)
	{
		machine_reset();
	}
}

static C64_EXPANSION_INTERFACE( expansion_intf )
{
	DEVCB_DRIVER_MEMBER(c128_state, exp_dma_r),
	DEVCB_DRIVER_MEMBER(c128_state, exp_dma_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, exp_irq_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, exp_nmi_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, exp_dma_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, exp_reset_w)
};


//-------------------------------------------------
//  C64_USER_PORT_INTERFACE( user_intf )
//-------------------------------------------------

static C64_USER_PORT_INTERFACE( user_intf )
{
	DEVCB_DEVICE_LINE_MEMBER(MOS6526_1_TAG, mos6526_device, sp_w),
	DEVCB_DEVICE_LINE_MEMBER(MOS6526_1_TAG, mos6526_device, cnt_w),
	DEVCB_DEVICE_LINE_MEMBER(MOS6526_2_TAG, mos6526_device, sp_w),
	DEVCB_DEVICE_LINE_MEMBER(MOS6526_2_TAG, mos6526_device, cnt_w),
	DEVCB_DEVICE_LINE_MEMBER(MOS6526_2_TAG, mos6526_device, flag_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, exp_reset_w)
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( c64 )
//-------------------------------------------------

void c128_state::machine_start()
{
	cbm_common_init();

	// find memory regions
	m_rom1 = memregion(M8502_TAG)->base();
	m_rom2 = m_rom1 + 0x4000;
	m_rom3 = m_rom1 + 0x8000;
	m_rom4 = m_rom1 + 0xc000;
	m_from = memregion("from")->base();
	m_charom = memregion("charom")->base();

	// allocate memory
	m_color_ram.allocate(0x800);

	// initialize memory
	UINT8 data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	// state saving
	save_item(NAME(m_z80en));
	save_item(NAME(m_loram));
	save_item(NAME(m_hiram));
	save_item(NAME(m_charen));
	save_item(NAME(m_game));
	save_item(NAME(m_exrom));
	save_item(NAME(m_reset));
	save_item(NAME(m_va14));
	save_item(NAME(m_va15));
	save_item(NAME(m_clrbank));
	save_item(NAME(m_cnt1));
	save_item(NAME(m_sp1));
	save_item(NAME(m_iec_data_out));
	save_item(NAME(m_cia1_irq));
	save_item(NAME(m_cia2_irq));
	save_item(NAME(m_vic_irq));
	save_item(NAME(m_exp_irq));
	save_item(NAME(m_exp_nmi));
	save_item(NAME(m_exp_dma));
	save_item(NAME(m_cass_rd));
	save_item(NAME(m_iec_srq));
	save_item(NAME(m_vic_k));
	save_item(NAME(m_caps_lock));
}


//-------------------------------------------------
//  MACHINE_RESET( c128 )
//-------------------------------------------------

void c128_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_maincpu->reset();
	m_reset = 1;

	m_mmu->reset();
	m_cia1->reset();
	m_cia2->reset();
	m_iec->reset();
	m_exp->reset();
	m_user->reset();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( ntsc, c128_state )
	// basic hardware
	MCFG_CPU_ADD(Z80A_TAG, Z80, VIC6567_CLOCK*2)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(z80_io)
	MCFG_CPU_VBLANK_INT_DRIVER(SCREEN_VIC_TAG, c128_state, frame_interrupt)
	MCFG_QUANTUM_PERFECT_CPU(Z80A_TAG)

	MCFG_CPU_ADD(M8502_TAG, M8502, VIC6567_CLOCK)
	MCFG_M8502_PORT_CALLBACKS(READ8(c128_state, cpu_r), WRITE8(c128_state, cpu_w))
	MCFG_M8502_PORT_PULLS(0x07, 0x20)
	MCFG_CPU_PROGRAM_MAP(m8502_mem)
	MCFG_CPU_VBLANK_INT_DRIVER(SCREEN_VIC_TAG, c128_state, frame_interrupt)
	MCFG_QUANTUM_PERFECT_CPU(M8502_TAG)

	// video hardware
	MCFG_MOS8563_ADD(MOS8563_TAG, SCREEN_VDC_TAG, VIC6567_CLOCK*2, vdc_intf, vdc_videoram_map)
	MCFG_MOS8564_ADD(MOS8564_TAG, SCREEN_VIC_TAG, VIC6567_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)
	MCFG_GFXDECODE(c128)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, SID6581, VIC6567_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_MOS8722_ADD(MOS8722_TAG, mmu_intf)
	MCFG_MOS8721_ADD(MOS8721_TAG)
	MCFG_MOS6526_ADD(MOS6526_1_TAG, VIC6567_CLOCK, 60, cia1_intf)
	MCFG_MOS6526_ADD(MOS6526_2_TAG, VIC6567_CLOCK, 60, cia2_intf)
	MCFG_QUICKLOAD_ADD("quickload", cbm_c64, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, "c1530", NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, VIC6567_CLOCK, expansion_intf, c64_expansion_cards, NULL, NULL)
	MCFG_C64_USER_PORT_ADD(C64_USER_PORT_TAG, user_intf, c64_user_port_cards, NULL, NULL)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "NTSC")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "NTSC")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c128", "c128_cart")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c128", "NTSC")
	MCFG_SOFTWARE_LIST_ADD("disk_list_c64", "c64_flop")
	MCFG_SOFTWARE_LIST_FILTER("disk_list_c64", "NTSC")
	MCFG_SOFTWARE_LIST_ADD("disk_list_c128", "c128_flop")
	MCFG_SOFTWARE_LIST_FILTER("disk_list_c128", "NTSC")
	MCFG_SOFTWARE_LIST_ADD("from_list", "c128_rom")
	MCFG_SOFTWARE_LIST_FILTER("from_list", "NTSC")

	// function ROM
	MCFG_CARTSLOT_ADD("from")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_INTERFACE("c128_rom")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128, ntsc )
	MCFG_CBM_IEC_ADD(cbm_iec_intf, "c1571")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128dcr )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128dcr, ntsc )
	MCFG_CBM_IEC_BUS_ADD(cbm_iec_intf)
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, c128dcr_iec_devices, "c1571", NULL) // TODO c1571cr
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128d81 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128d81, ntsc )
	MCFG_CBM_IEC_BUS_ADD(cbm_iec_intf)
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, c128d81_iec_devices, "c1563", NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal, c128_state )
	// basic hardware
	MCFG_CPU_ADD(Z80A_TAG, Z80, VIC6569_CLOCK*2)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(z80_io)
	MCFG_CPU_VBLANK_INT_DRIVER(SCREEN_VIC_TAG, c128_state, frame_interrupt)
	MCFG_QUANTUM_PERFECT_CPU(Z80A_TAG)

	MCFG_CPU_ADD(M8502_TAG, M8502, VIC6569_CLOCK)
	MCFG_M8502_PORT_CALLBACKS(READ8(c128_state, cpu_r), WRITE8(c128_state, cpu_w))
	MCFG_M8502_PORT_PULLS(0x07, 0x20)
	MCFG_CPU_PROGRAM_MAP(m8502_mem)
	MCFG_CPU_VBLANK_INT_DRIVER(SCREEN_VIC_TAG, c128_state, frame_interrupt)
	MCFG_QUANTUM_PERFECT_CPU(M8502_TAG)

	// video hardware
	MCFG_MOS8563_ADD(MOS8563_TAG, SCREEN_VDC_TAG, VIC6569_CLOCK*2, vdc_intf, vdc_videoram_map)
	MCFG_MOS8566_ADD(MOS8564_TAG, SCREEN_VIC_TAG, VIC6569_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)
	MCFG_GFXDECODE(c128)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, SID6581, VIC6569_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_MOS8722_ADD(MOS8722_TAG, mmu_intf)
	MCFG_MOS8721_ADD(MOS8721_TAG)
	MCFG_MOS6526_ADD(MOS6526_1_TAG, VIC6569_CLOCK, 50, cia1_intf)
	MCFG_MOS6526_ADD(MOS6526_2_TAG, VIC6569_CLOCK, 50, cia2_intf)
	MCFG_QUICKLOAD_ADD("quickload", cbm_c64, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, "c1530", NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, VIC6569_CLOCK, expansion_intf, c64_expansion_cards, NULL, NULL)
	MCFG_C64_USER_PORT_ADD(C64_USER_PORT_TAG, user_intf, c64_user_port_cards, NULL, NULL)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "PAL")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "PAL")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c128", "c128_cart")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c128", "PAL")
	MCFG_SOFTWARE_LIST_ADD("disk_list_c64", "c64_flop")
	MCFG_SOFTWARE_LIST_FILTER("disk_list_c64", "PAL")
	MCFG_SOFTWARE_LIST_ADD("disk_list_c128", "c128_flop")
	MCFG_SOFTWARE_LIST_FILTER("disk_list_c128", "PAL")
	MCFG_SOFTWARE_LIST_ADD("from_list", "c128_rom")
	MCFG_SOFTWARE_LIST_FILTER("from_list", "PAL")

	// function ROM
	MCFG_CARTSLOT_ADD("from")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_INTERFACE("c128_rom")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128pal )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128pal, pal )
	MCFG_CBM_IEC_ADD(cbm_iec_intf, "c1571")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128dcrp )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128dcrp, pal )
	MCFG_CBM_IEC_BUS_ADD(cbm_iec_intf)
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, c128dcr_iec_devices, "c1571", NULL) // TODO c1571cr
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL, NULL)
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( c128 )
//-------------------------------------------------

ROM_START( c128 )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_DEFAULT_BIOS("r4")
	ROM_LOAD( "251913-01.u32", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )
	ROM_SYSTEM_BIOS( 0, "r2", "Revision 2" )
	ROMX_LOAD( "318018-02.u33", 0x4000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6), ROM_BIOS(1) )
	ROMX_LOAD( "318019-02.u34", 0x8000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f), ROM_BIOS(1) )
	ROMX_LOAD( "318020-03.u35", 0xc000, 0x4000, CRC(1e94bb02) SHA1(e80ffbafae068cc0e42698ec5c5c39af46ac612a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(2) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(2) )
	ROMX_LOAD( "318020-05.u35", 0xc000, 0x4000, CRC(ba456b8e) SHA1(ceb6e1a1bf7e08eb9cbc651afa29e26adccf38ab), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(3) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(3) )
	ROMX_LOAD( "jiffydos c128.u35", 0xc000, 0x4000, CRC(4b7964de) SHA1(7d1898f32beae4b2ae610d469ce578a588efaa7c), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "quikslvr", "QuickSilver 128" ) // requires add-on cartridge
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(4) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(4) )
	ROMX_LOAD( "quicksilver128.u35", 0xc000, 0x4000, CRC(c2e74338) SHA1(916cdcc62eb631073aa7f096815dcf33b3229ca8), ROM_BIOS(4) )

	ROM_REGION( 0x8000, "from", 0 )
	ROM_CART_LOAD( "from", 0x0000, 0x8000, ROM_NOMIRROR )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "390059-01.u18", 0x0000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128p )
//-------------------------------------------------

#define rom_c128p               rom_c128


//-------------------------------------------------
//  ROM( c128_de )
//-------------------------------------------------

ROM_START( c128_de )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_DEFAULT_BIOS("r4")
	ROM_LOAD( "251913-01.u32", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )
	ROM_SYSTEM_BIOS( 0, "r2", "Revision 2" )
	ROMX_LOAD( "318018-02.u33", 0x4000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6), ROM_BIOS(1) )
	ROMX_LOAD( "318019-02.u34", 0x8000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f), ROM_BIOS(1) )
	ROMX_LOAD( "315078-01.u35", 0xc000, 0x4000, CRC(a51e2168) SHA1(bcf82a89a8fc5d086bec2ff3bcbdecc8af2be3af), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(2) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(2) )
	ROMX_LOAD( "315078-02.u35", 0xc000, 0x4000, CRC(b275bb2e) SHA1(78ac5dcdd840b092ba1ee6d19b33af079613291f), ROM_BIOS(2) )

	ROM_REGION( 0x8000, "from", 0 )
	ROM_CART_LOAD( "from", 0x0000, 0x8000, ROM_NOMIRROR )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "315079-01.u18", 0x00000, 0x2000, CRC(fe5a2db1) SHA1(638f8aff51c2ac4f99a55b12c4f8c985ef4bebd3) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128_se )
//-------------------------------------------------

ROM_START( c128_se )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "325182-01.u32", 0x0000, 0x4000, CRC(2aff27d3) SHA1(267654823c4fdf2167050f41faa118218d2569ce) ) // "C128 64 Sw/Fi"
	ROM_LOAD( "318018-02.u33", 0x4000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6) )
	ROM_LOAD( "318019-02.u34", 0x8000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f) )
	ROM_LOAD( "325189-01.u35", 0xc000, 0x4000, CRC(9526fac4) SHA1(a01dd871241c801db51e8ebc30fedfafd8cc506b) ) // "C128 Ker Sw/Fi"

	ROM_REGION( 0x8000, "from", 0 )
	ROM_CART_LOAD( "from", 0x0000, 0x8000, ROM_NOMIRROR )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "325181-01.bin", 0x0000, 0x2000, CRC(7a70d9b8) SHA1(aca3f7321ee7e6152f1f0afad646ae41964de4fb) ) // "C128 Char Sw/Fi"

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128d )
//-------------------------------------------------

#define rom_c128d       rom_c128


//-------------------------------------------------
//  ROM( c128dp )
//-------------------------------------------------

#define rom_c128dp      rom_c128


//-------------------------------------------------
//  ROM( c128dpr )
//-------------------------------------------------

#define rom_c128dpr     rom_c128d


//-------------------------------------------------
//  ROM( c128cr )
//-------------------------------------------------

ROM_START( c128cr )
	/* C128CR prototype, owned by Bo Zimmers
	PCB markings: "COMMODORE 128CR REV.3 // PCB NO.252270" and "PCB ASSY NO.250783"
	Sticker on rom cart shield: "C128CR  No.2 // ENG. SAMPLE // Jun/9/'86   KNT"
	3 ROMs (combined basic, combined c64/kernal, plain character rom)
	6526A-1 CIAs
	?prototype? 2568R1X VDC w/ 1186 datecode
	*/
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "252343-03.u34", 0x4000, 0x8000, CRC(bc07ed87) SHA1(0eec437994a3f2212343a712847213a8a39f4a7b) ) // "252343-03 // U34"
	ROM_LOAD( "252343-04.u32", 0x0000, 0x4000, CRC(cc6bdb69) SHA1(36286b2e8bea79f7767639fd85e12c5447c7041b) ) // "252343-04 // US // U32"
	ROM_CONTINUE(              0xc000, 0x4000 )

	ROM_REGION( 0x8000, "from", 0 )
	ROM_CART_LOAD( "from", 0x0000, 0x8000, ROM_NOMIRROR )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "390059-01.u18", 0x0000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) ) // "MOS // (C)1985 CBM // 390059-01 // M468613 8547H"

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128dcr )
//-------------------------------------------------

ROM_START( c128dcr )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "318022-02.u34", 0x4000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )
	ROM_LOAD( "318023-02.u32", 0x0000, 0x4000, CRC(eedc120a) SHA1(f98c5a986b532c78bb68df9ec6dbcf876913b99f) )
	ROM_CONTINUE(              0xc000, 0x4000 )

	ROM_REGION( 0x8000, "from", 0 )
	ROM_CART_LOAD( "from", 0x0000, 0x8000, ROM_NOMIRROR )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "390059-01.u18", 0x0000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128dcrp )
//-------------------------------------------------

#define rom_c128dcrp    rom_c128dcr


//-------------------------------------------------
//  ROM( c128dcr_de )
//-------------------------------------------------

ROM_START( c128dcr_de )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "318022-02.u34", 0x4000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )
	ROM_LOAD( "318077-01.u32", 0x0000, 0x4000, CRC(eb6e2c8f) SHA1(6b3d891fedabb5335f388a5d2a71378472ea60f4) )
	ROM_CONTINUE(              0xc000, 0x4000 )

	ROM_REGION( 0x8000, "from", 0 )
	ROM_CART_LOAD( "from", 0x0000, 0x8000, ROM_NOMIRROR )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "315079-01.u18", 0x0000, 0x2000, CRC(fe5a2db1) SHA1(638f8aff51c2ac4f99a55b12c4f8c985ef4bebd3) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128dcr_se )
//-------------------------------------------------

ROM_START( c128dcr_se )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "318022-02.u34", 0x4000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )
	ROM_LOAD( "318034-01.u32", 0x0000, 0x4000, CRC(cb4e1719) SHA1(9b0a0cef56d00035c611e07170f051ee5e63aa3a) )
	ROM_CONTINUE(              0xc000, 0x4000 )

	ROM_REGION( 0x8000, "from", 0 )
	ROM_CART_LOAD( "from", 0x0000, 0x8000, ROM_NOMIRROR )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "325181-01.u18", 0x0000, 0x2000, CRC(7a70d9b8) SHA1(aca3f7321ee7e6152f1f0afad646ae41964de4fb) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128d81 )
//-------------------------------------------------

#define rom_c128d81             rom_c128d



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT                COMPANY                        FULLNAME                                 FLAGS
COMP( 1985, c128,       0,      0,      c128,       c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128 (NTSC)",                  GAME_SUPPORTS_SAVE )
COMP( 1985, c128p,      0,      0,      c128pal,    c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128 (PAL)",                   GAME_SUPPORTS_SAVE )
COMP( 1985, c128_de,    c128,   0,      c128pal,    c128ger,    driver_device,  0,  "Commodore Business Machines", "Commodore 128 (Germany)",               GAME_SUPPORTS_SAVE )
//COMP( 1985, c128_fr,   c128,  0,   c128pal,  c128fra, driver_device, 0,  "Commodore Business Machines", "Commodore 128 (France)", GAME_SUPPORTS_SAVE )
//COMP( 1985, c128_no,   c128,  0,   c128pal,  c128ita, driver_device, 0,  "Commodore Business Machines", "Commodore 128 (Norway)", GAME_SUPPORTS_SAVE )
COMP( 1985, c128_se,    c128,   0,      c128pal,    c128swe,    driver_device,  0,  "Commodore Business Machines", "Commodore 128 (Sweden/Finland)",        GAME_SUPPORTS_SAVE )
COMP( 1986, c128d,      c128,   0,      c128,       c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128D (NTSC, prototype)",      GAME_SUPPORTS_SAVE )
COMP( 1986, c128dp,     c128,   0,      c128pal,    c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128D (PAL)",                  GAME_SUPPORTS_SAVE )

COMP( 1986, c128cr,     c128,   0,      c128,       c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128CR (NTSC, prototype)",     GAME_SUPPORTS_SAVE )

COMP( 1987, c128dcr,    c128,   0,      c128dcr,    c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128DCR (NTSC)",               GAME_SUPPORTS_SAVE )
COMP( 1987, c128dcrp,   c128,   0,      c128dcrp,   c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128DCR (PAL)",                GAME_SUPPORTS_SAVE )
COMP( 1987, c128dcr_de, c128,   0,      c128dcrp,   c128ger,    driver_device,  0,  "Commodore Business Machines", "Commodore 128DCR (Germany)",            GAME_SUPPORTS_SAVE )
//COMP( 1986, c128dcr_it,  c128,  0,   c128dcrp, c128ita, driver_device, 0,"Commodore Business Machines", "Commodore 128DCR (Italy)", GAME_SUPPORTS_SAVE )
COMP( 1987, c128dcr_se, c128,   0,      c128dcrp,   c128swe,    driver_device,  0,  "Commodore Business Machines", "Commodore 128DCR (Sweden/Finland)",     GAME_SUPPORTS_SAVE )

COMP( 1986, c128d81,    c128,   0,      c128d81,    c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128D/81 (NTSC, prototype)",   GAME_SUPPORTS_SAVE )
