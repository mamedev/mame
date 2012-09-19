/*

    TODO:

	- cbm600
	- cbm700
	- read VIC video RAM thru PLA
	- read VIC color RAM thru PLA
	- user port
	- co-processor bus

*/

#include "includes/cbm2.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define P3 BIT(offset, 19)
#define P2 BIT(offset, 18)
#define P1 BIT(offset, 17)
#define P0 BIT(offset, 16)
#define A15 BIT(offset, 15)
#define A14 BIT(offset, 14)
#define A13 BIT(offset, 13)
#define A12 BIT(offset, 12)
#define A11 BIT(offset, 11)
#define A10 BIT(offset, 10)
#define VA12 BIT(va, 12)



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void p500_state::check_interrupts()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_vic_irq || m_tpi1_irq);	

	mos6526_flag_w(m_cia, m_cass_rd && m_user_flag);
}



//**************************************************************************
//  MEMORY MANAGEMENT UNIT
//**************************************************************************

//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void p500_state::bankswitch(offs_t offset, offs_t va, int srw, int sphi0, int sphi1, int sphi2, int ba, int ae, int bras, int bcas, int busy2, int refen,
	int *datxen, int *dramxen, int *clrniben, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr,
	int *clrnibcs, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *viccs, int *vidmatcs,
	int *csbank1, int *csbank2, int *csbank3, int *basiclocs, int *basichics, int *kernalcs,
	int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs, int *aec, int *vsysaden)
{
	*aec = !((m_statvid || ae) && sphi2);

	int clrnibcsb = 1;
	int procvid = 1;

	UINT32 input = P0 << 15 | P2 << 14 | bras << 13 | P1 << 12 | P3 << 11 | busy2 << 10 | m_statvid << 9 | sphi2 << 8 |
				   clrnibcsb << 7 | m_dramon << 6 | procvid << 5 | refen << 4 | m_vicdotsel << 3 | ba << 2 | *aec << 1 | srw;
	UINT32 data = m_pla1->read(input);

	int segf = BIT(data, 3);

	int bank0 = 1, vicen = 1;

	if (!*aec && !segf)
	{
		switch ((offset >> 13) & 0x07)
		{
		case 0: bank0 = 0; break;
		case 1: *csbank1 = 0; break;
		case 2: *csbank2 = 0; break;
		case 3: *csbank3 = 0; break;
		case 4: *basiclocs = 0; break;
		case 5: *basichics = 0; break;
		case 6:
			if (A12 && A11)
			{
				switch ((offset >> 8) & 0x07)
				{
				case 0: vicen = 0; break;
				case 1: *cs1 = 0; break;
				case 2: *sidcs = 0; break;
				case 3: *extprtcs = 0; break;
				case 4: *ciacs = 0; break;
				case 5: *aciacs = 0; break;
				case 6: *tript1cs = 0; break;
				case 7: *tript2cs = 0; break;
				}
			}
			break;

		case 7: *kernalcs = 0; break;
		}
	}

	input = VA12 << 15 | ba << 14 | A13 << 13 | A15 << 12 | A14 << 11 | A11 << 10 | A10 << 9 | A12 << 8 |
			sphi2 << 7 | vicen << 6 | m_statvid << 5 | m_vicdotsel << 4 | ae << 3 | segf << 2 | bcas << 1 | bank0;
	data = m_pla2->read(input);

	clrnibcsb = BIT(data, 0);
	if (!bcas) *clrnibcs = clrnibcsb;
	*extbufcs = BIT(data, 1);
	*discromcs = BIT(data, 2);
	*buframcs = BIT(data, 3);
	*charomcs = BIT(data, 4);
	procvid = BIT(data, 5);
	*viccs = BIT(data, 6);
	if (!bcas) *vidmatcs = BIT(data, 7);

	input = P0 << 15 | P2 << 14 | bras << 13 | P1 << 12 | P3 << 11 | busy2 << 10 | m_statvid << 9 | sphi2 << 8 |
			clrnibcsb << 7 | m_dramon << 6 | procvid << 5 | refen << 4 | m_vicdotsel << 3 | ba << 2 | *aec << 1 | srw;
	data = m_pla1->read(input);

	*datxen = BIT(data, 0);
	*dramxen = BIT(data, 1);
	*clrniben = BIT(data, 2);
	//*segf = BIT(data, 3);
	*_64kcasen = BIT(data, 4);
	*casenb = BIT(data, 5);
	*viddaten = BIT(data, 6);
	*viddat_tr = BIT(data, 7);

	*vsysaden = sphi1 || ba;
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

UINT8 p500_state::read_memory(address_space &space, offs_t offset, offs_t va, int sphi0, int sphi1, int sphi2, int ba, int ae, int bras, int bcas)
{
	int srw = 1, busy2 = 1, refen = 0;
	
	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(offset, va, srw, sphi0, sphi1, sphi2, ba, ae, bras, bcas, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);
/*
	if (!space.debugger_access() && !ae)
	logerror("read  %05x %u %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u : ",
		offset, datxen, dramxen, clrniben, _64kcasen, casenb, viddaten, viddat_tr,
		clrnibcs, extbufcs, discromcs, buframcs, charomcs, viccs, vidmatcs,
		csbank1, csbank2, csbank3, basiclocs, basichics, kernalcs,
		cs1, sidcs, extprtcs, ciacs, aciacs, tript1cs, tript2cs, aec, vsysaden);
*/
	UINT8 data = 0xff;

	if (ae)
	{
		data = m_vic->bus_r();
	}

	if (aec && !datxen && !_64kcasen)
	{
		data = m_ram->pointer()[offset & 0xffff];
		//if (!space.debugger_access() && !ae) logerror("64K\n");
	}
	else if (!aec && !viddaten && viddat_tr && !_64kcasen)
	{
		data = m_ram->pointer()[(m_vicbnksel << 14) | va];
		//if (!space.debugger_access() && !ae) logerror("64K\n");
	}
	else if (!dramxen && casenb && !P3)
	{
		switch ((offset >> 15) & 0x07)
		{
		case 1: data = m_ram->pointer()[0x10000 + (offset & 0xffff)]; break;
		case 2: if (m_ram->size() > 0x20000) data = m_ram->pointer()[0x20000 + (offset & 0xffff)]; break;
		case 3: if (m_ram->size() > 0x30000) data = m_ram->pointer()[0x30000 + (offset & 0xffff)]; break;
		}
		//if (!space.debugger_access() && !ae) logerror("CASEN\n");
	}
	else if (!datxen && !buframcs)
	{
		data = m_buffer_ram[offset & 0x7ff];
		//if (!space.debugger_access() && !ae) logerror("BUFRAM\n");
	}
	else if (!vsysaden && clrniben && !clrnibcs)
	{
		data = m_color_ram[offset & 0x3ff];
		//if (!space.debugger_access() && !ae) logerror("CLRNIB\n");
	}
	else if (vsysaden && !clrnibcs)
	{
		data = m_color_ram[va & 0x3ff];
		//if (!space.debugger_access() && !ae) logerror("CLRNIB\n");
	}
	else if (!datxen && !vsysaden && !viddaten && viddat_tr && !vidmatcs)
	{
		data = m_video_ram[offset & 0x3ff];
		//if (!space.debugger_access() && !ae) logerror("VIDMAT\n");
	}
	else if (vsysaden && !vidmatcs)
	{
		data = m_video_ram[va & 0x3ff];
		//if (!space.debugger_access() && !ae) logerror("VIDMAT\n");
	}
	else if (!datxen && (!basiclocs || !basichics))
	{
		data = m_basic[offset & 0x3fff];
		//if (!space.debugger_access() && !ae) logerror("BASIC\n");
	}
	else if (!datxen && !kernalcs)
	{
		data = m_kernal[offset & 0x1fff];
		//if (!space.debugger_access() && !ae) logerror("KERNAL\n");
	}
	else if (!datxen && !vsysaden && !viddaten && viddat_tr && !charomcs)
	{
		data = m_charom[offset & 0xfff];
		//if (!space.debugger_access() && !ae) logerror("CHAROM\n");
	}
	else if (vsysaden && !charomcs)
	{
		data = m_charom[va & 0xfff];
		//if (!space.debugger_access() && !ae) logerror("CHAROM\n");
	}
	else if (!datxen && !viddaten && viddat_tr && !viccs)
	{
		data = m_vic->read(space, offset & 0x3f);
		//if (!space.debugger_access() && !ae) logerror("VIC\n");
	}
	else if (!datxen && !sidcs)
	{
		data = m_sid->read(space, offset & 0x1f);
		//if (!space.debugger_access() && !ae) logerror("SID\n");
	}
	else if (!datxen && !ciacs)
	{
		data = m_cia->read(space, offset & 0x0f);
		//if (!space.debugger_access() && !ae) logerror("CIA\n");
	}
	else if (!datxen && !aciacs)
	{
		data = m_acia->read(space, offset & 0x03);
		//if (!space.debugger_access() && !ae) logerror("ACIA\n");
	}
	else if (!datxen && !tript1cs)
	{
		data = m_tpi1->read(space, offset & 0x07);
		//if (!space.debugger_access() && !ae) logerror("TPI1\n");
	}
	else if (!datxen && !tript2cs)
	{
		data = m_tpi2->read(space, offset & 0x07);
		//if (!space.debugger_access() && !ae) logerror("TPI2\n");
	}
	//else if (!space.debugger_access() && !ae) logerror("\n");

	if (!datxen) data = m_exp->read(space, offset & 0x1fff, data, csbank1, csbank2, csbank3);

	return data;
}


//-------------------------------------------------
//  write_memory -
//-------------------------------------------------

void p500_state::write_memory(address_space &space, offs_t offset, UINT8 data, int sphi0, int sphi1, int sphi2, int ba, int ae, int bras, int bcas)
{
	int srw = 0, busy2 = 1, refen = 0;
	offs_t va = 0xffff;
	
	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(offset, va, srw, sphi0, sphi1, sphi2, ba, ae, bras, bcas, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);
/*
	if (!space.debugger_access())
	logerror("write %05x %u %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u: ",
		offset, datxen, dramxen, clrniben, _64kcasen, casenb, viddaten, viddat_tr,
		clrnibcs, extbufcs, discromcs, buframcs, charomcs, viccs, vidmatcs,
		csbank1, csbank2, csbank3, basiclocs, basichics, kernalcs,
		cs1, sidcs, extprtcs, ciacs, aciacs, tript1cs, tript2cs, aec, vsysaden);
*/
	if (!aec && !datxen && !_64kcasen)
	{
		//logerror("64K RAM\n");
		m_ram->pointer()[offset & 0xffff] = data;
	}
	else if (!dramxen && casenb && !P3)
	{
		//logerror("CASENB\n");
		switch ((offset >> 15) & 0x07)
		{
		case 1: m_ram->pointer()[0x10000 + (offset & 0xffff)] = data; break;
		case 2: if (m_ram->size() > 0x20000) m_ram->pointer()[0x20000 + (offset & 0xffff)] = data; break;
		case 3: if (m_ram->size() > 0x30000) m_ram->pointer()[0x30000 + (offset & 0xffff)] = data; break;
		}
	}
	else if (!datxen && !buframcs)
	{
		//logerror("BUFRAM\n");
		m_buffer_ram[offset & 0x7ff] = data;
	}
	else if (!vsysaden && clrniben && !clrnibcs)
	{
		//logerror("CLRNIB\n");
		m_color_ram[offset & 0x3ff] = data;
	}
	else if (!datxen && !vsysaden && !viddaten && !viddat_tr && !vidmatcs)
	{
		//logerror("VIDMAT\n");
		m_video_ram[offset & 0x3ff] = data;
	}
	else if (vsysaden && !vidmatcs)
	{
		//logerror("VIDMAT\n");
		m_video_ram[va & 0x3ff] = data;
	}
	else if (!datxen && !viddaten && !viddat_tr && !viccs)
	{
		//logerror("VIC\n");
		m_vic->write(space, offset & 0x3f, data);
	}
	else if (!datxen && !sidcs)
	{
		//logerror("SID\n");
		m_sid->write(space, offset & 0x1f, data);
	}
	else if (!datxen && !ciacs)
	{
		//logerror("CIA\n");
		m_cia->write(space, offset & 0x0f, data);
	}
	else if (!datxen && !aciacs)
	{
		//logerror("ACIA\n");
		m_acia->write(space, offset & 0x03, data);
	}
	else if (!datxen && !tript1cs)
	{
		//logerror("TPI1\n");
		m_tpi1->write(space, offset & 0x07, data);
	}
	else if (!datxen && !tript2cs)
	{
		//logerror("TPI2\n");
		m_tpi2->write(space, offset & 0x07, data);
	}
	//else logerror("\n");

	if (!datxen) m_exp->write(space, offset & 0x1fff, data, csbank1, csbank2, csbank3);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( p500_state::read )
{
	int sphi0 = 1, sphi1 = 0, sphi2 = 1, ba = 0, ae = 1, bras = 1, bcas = 0;
	offs_t va = 0xffff;

	return read_memory(space, offset, va, sphi0, sphi1, sphi2, ba, ae, bras, bcas);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( p500_state::write )
{
	int sphi0 = 1, sphi1 = 0, sphi2 = 1, ba = 0, ae = 1, bras = 1, bcas = 0;

	write_memory(space, offset, data, sphi0, sphi1, sphi2, ba, ae, bras, bcas);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

READ8_MEMBER( p500_state::vic_videoram_r )
{
/*	int sphi0 = 0, sphi1 = 1, sphi2 = 0, ba = 1, ae = 0, bras = 0, bcas = 0;
	offs_t va = offset;

	return read_memory(space, 0, va, sphi0, sphi1, sphi2, ba, ae, bras, bcas);*/

	if (offset < 0x1000)
	{
		return m_charom[offset & 0xfff];
	}
	else
	{
		return m_video_ram[offset & 0x3ff];
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( p500_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( p500_mem, AS_PROGRAM, 8, p500_state )
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, p500_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_videoram_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, p500_state )
	AM_RANGE(0x000, 0x3ff) AM_RAM AM_SHARE("color_ram")
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( p500 )
//-------------------------------------------------

static INPUT_PORTS_START( p500 )
	PORT_START("PB0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT" \xC2\xA3") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(UCHAR_MAMEKEY(TILDE)) PORT_CHAR(0x00a3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xCF\x80") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0x03c0)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS/DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C=") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR/HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ?")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("OFF/RVS") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad CE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NORM/GRAPH") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 00")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN/STOP") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad /") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ENTER") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  vic2_interface vic_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( p500_state::vic_irq_w )
{
	m_vic_irq = state;

	check_interrupts();
}

static MOS6567_INTERFACE( vic_intf )
{
	SCREEN_TAG,
	M6509_TAG,
	DEVCB_DRIVER_LINE_MEMBER(p500_state, vic_irq_w),
	DEVCB_NULL, // RDY
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

READ8_MEMBER( p500_state::sid_potx_r )
{
	int sela = BIT(m_cia_pa, 6);
	int selb = BIT(m_cia_pa, 7);

	UINT8 data = 0;

	if (sela) data = m_joy1->pot_x_r();
	if (selb) data = m_joy2->pot_x_r();

	return data;
}

READ8_MEMBER( p500_state::sid_poty_r )
{
	int sela = BIT(m_cia_pa, 6);
	int selb = BIT(m_cia_pa, 7);

	UINT8 data = 0;

	if (sela) data = m_joy1->pot_y_r();
	if (selb) data = m_joy2->pot_y_r();

	return data;
}

static MOS6581_INTERFACE( sid_intf )
{
	DEVCB_DRIVER_MEMBER(p500_state, sid_potx_r),
	DEVCB_DRIVER_MEMBER(p500_state, sid_poty_r)
};


//-------------------------------------------------
//  tpi6525_interface tpi1_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( p500_state::tpi1_irq_w )
{
	m_tpi1_irq = state;

	check_interrupts();
}

READ8_MEMBER( p500_state::tpi1_pa_r )
{
	/*
	
	    bit     description
	
	    0       
	    1       
	    2       REN
	    3       ATN
	    4       DAV
	    5       EOI
	    6       NDAC
	    7       NRFD
	
	*/

	UINT8 data = 0;

	// IEEE-488
	data |= m_ieee->ren_r() << 2;
	data |= m_ieee->atn_r() << 3;
	data |= m_ieee->dav_r() << 4;
	data |= m_ieee->eoi_r() << 5;
	data |= m_ieee->ndac_r() << 6;
	data |= m_ieee->nrfd_r() << 7;

	return data;
}

WRITE8_MEMBER( p500_state::tpi1_pa_w )
{
	/*
	
	    bit     description
	
	    0       75161A DC
	    1       75161A TE
	    2       REN
	    3       ATN
	    4       DAV
	    5       EOI
	    6       NDAC
	    7       NRFD
	
	*/

	// IEEE-488
	m_ieee->ren_w(BIT(data, 2));
	m_ieee->atn_w(BIT(data, 3));
	m_ieee->dav_w(BIT(data, 4));
	m_ieee->eoi_w(BIT(data, 5));
	m_ieee->ndac_w(BIT(data, 6));
	m_ieee->nrfd_w(BIT(data, 7));
}

READ8_MEMBER( p500_state::tpi1_pb_r )
{
	/*
	
	    bit     description
	
	    0       IFC
	    1       SRQ
	    2       user port
	    3       user port
	    4       
	    5       
	    6       
	    7       CASS SW
	
	*/

	UINT8 data = 0;

	// IEEE-488
	data |= m_ieee->ifc_r();
	data |= m_ieee->srq_r() << 1;

	// cassette
	data |= m_cassette->sense_r() << 7;

	return data;
}

WRITE8_MEMBER( p500_state::tpi1_pb_w )
{
	/*
	
	    bit     description
	
	    0       IFC
	    1       SRQ
	    2       user port
	    3       user port
	    4       DRAMON
	    5       CASS WRT
	    6       CASS MTR
	    7       
	
	*/

	// IEEE-488
	m_ieee->ifc_w(BIT(data, 0));
	m_ieee->srq_w(BIT(data, 1));

	// memory
	m_dramon = BIT(data, 4);

	// cassette
	m_cassette->write(BIT(data, 5));
	m_cassette->motor_w(BIT(data, 6));
}

WRITE_LINE_MEMBER( p500_state::tpi1_ca_w )
{
	//logerror("STATVID %u\n", state);

	m_statvid = state;
}

WRITE_LINE_MEMBER( p500_state::tpi1_cb_w )
{
	//logerror("VICDOTSEL %u\n", state);

	m_vicdotsel = state;
}

static const tpi6525_interface tpi1_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(p500_state, tpi1_irq_w),
	DEVCB_DRIVER_MEMBER(p500_state, tpi1_pa_r),
	DEVCB_DRIVER_MEMBER(p500_state, tpi1_pa_w),
	DEVCB_DRIVER_MEMBER(p500_state, tpi1_pb_r),
	DEVCB_DRIVER_MEMBER(p500_state, tpi1_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(p500_state, tpi1_ca_w),
	DEVCB_DRIVER_LINE_MEMBER(p500_state, tpi1_cb_w)
};


//-------------------------------------------------
//  tpi6525_interface tpi2_intf
//-------------------------------------------------

WRITE8_MEMBER( p500_state::tpi2_pa_w )
{
	m_tpi2_pa = data;
}

WRITE8_MEMBER( p500_state::tpi2_pb_w )
{
	m_tpi2_pb = data;
}

READ8_MEMBER( p500_state::tpi2_pc_r )
{
	/*
	
	    bit     description
	
	    0       COLUMN 0
	    1       COLUMN 1
	    2       COLUMN 2
	    3       COLUMN 3
	    4       COLUMN 4
	    5       COLUMN 5
	    6       
	    7       
	
	*/

	UINT8 data = 0xff;

	if (!BIT(m_tpi2_pa, 0)) data &= ioport("PA0")->read();
	if (!BIT(m_tpi2_pa, 1)) data &= ioport("PA1")->read();
	if (!BIT(m_tpi2_pa, 2)) data &= ioport("PA2")->read();
	if (!BIT(m_tpi2_pa, 3)) data &= ioport("PA3")->read();
	if (!BIT(m_tpi2_pa, 4)) data &= ioport("PA4")->read();
	if (!BIT(m_tpi2_pa, 5)) data &= ioport("PA5")->read();
	if (!BIT(m_tpi2_pa, 6)) data &= ioport("PA6")->read();
	if (!BIT(m_tpi2_pa, 7)) data &= ioport("PA7")->read();
	if (!BIT(m_tpi2_pb, 0)) data &= ioport("PB0")->read();
	if (!BIT(m_tpi2_pb, 1)) data &= ioport("PB1")->read();
	if (!BIT(m_tpi2_pb, 2)) data &= ioport("PB2")->read();
	if (!BIT(m_tpi2_pb, 3)) data &= ioport("PB3")->read();
	if (!BIT(m_tpi2_pb, 4)) data &= ioport("PB4")->read();
	if (!BIT(m_tpi2_pb, 5)) data &= ioport("PB5")->read();
	if (!BIT(m_tpi2_pb, 6)) data &= ioport("PB6")->read();
	if (!BIT(m_tpi2_pb, 7)) data &= ioport("PB7")->read();

	return data;
}

WRITE8_MEMBER( p500_state::tpi2_pc_w )
{
	/*
	
	    bit     description
	
	    0       
	    1       
	    2       
	    3       
	    4       
	    5       
	    6       VICBNKSEL0
	    7       VICBNKSEL1
	
	*/

	m_vicbnksel = data >> 6;
}

static const tpi6525_interface tpi2_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(p500_state, tpi2_pa_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(p500_state, tpi2_pb_w),
	DEVCB_DRIVER_MEMBER(p500_state, tpi2_pc_r),
	DEVCB_DRIVER_MEMBER(p500_state, tpi2_pc_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  mos6526_interface cia_intf
//-------------------------------------------------

READ8_MEMBER( p500_state::cia_pa_r )
{
	/*
	
	    bit     description
	
	    0       user port
	    1       user port
	    2       user port
	    3       user port
	    4       user port
	    5       user port
	    6       LTPN, user port
	    7       GAME TRIGGER 24, user port
	
	*/

	UINT8 data = 0;

	// joystick
	data |= BIT(m_joy1->joy_r(), 5) << 6;
	data |= BIT(m_joy2->joy_r(), 5) << 7;

	return data;
}

WRITE8_MEMBER( p500_state::cia_pa_w )
{
	/*
	
	    bit     description
	
	    0       user port
	    1       user port
	    2       user port
	    3       user port
	    4       user port
	    5       user port
	    6       user port
	    7       user port
	
	*/

	m_cia_pa = data;
}

READ8_MEMBER( p500_state::cia_pb_r )
{
	/*
	
	    bit     description
	
	    0       GAME 10, user port
	    1       GAME 11, user port
	    2       GAME 12, user port
	    3       GAME 13, user port
	    4       GAME 20, user port
	    5       GAME 21, user port
	    6       GAME 22, user port
	    7       GAME 23, user port
	
	*/

	UINT8 data = 0;

	// joystick
	data |= m_joy1->joy_r() & 0x0f;
	data |= (m_joy2->joy_r() & 0x0f) << 4;

	return data;
}

WRITE8_MEMBER( p500_state::cia_pb_w )
{
	/*
	
	    bit     description
	
	    0       user port
	    1       user port
	    2       user port
	    3       user port
	    4       user port
	    5       user port
	    6       user port
	    7       user port
	
	*/
}

static const mos6526_interface cia_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(MOS6525_1_TAG, tpi6525_device, i2_w),
	DEVCB_NULL, // user port
	DEVCB_NULL, // user port
	DEVCB_NULL, // user port
	DEVCB_DRIVER_MEMBER(p500_state, cia_pa_r),
	DEVCB_DRIVER_MEMBER(p500_state, cia_pa_w),
	DEVCB_DRIVER_MEMBER(p500_state, cia_pb_r),
	DEVCB_DRIVER_MEMBER(p500_state, cia_pb_w),
};


//-------------------------------------------------
//  PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( p500_state::tape_read_w )
{
	m_cass_rd = state;

	check_interrupts();
}

static PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(p500_state, tape_read_w)
};


//-------------------------------------------------
//  IEEE488_INTERFACE( ieee488_intf )
//-------------------------------------------------

static IEEE488_INTERFACE( ieee488_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(MOS6525_1_TAG, tpi6525_device, i1_w),
	DEVCB_NULL,
	DEVCB_NULL
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void p500_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_tpi1->i0_w(m_todclk);

	m_todclk = !m_todclk;
}


//-------------------------------------------------
//  MACHINE_START( p500 )
//-------------------------------------------------

void p500_state::machine_start()
{
	// find memory regions
	m_basic = memregion("basic")->base();
	m_kernal = memregion("kernal")->base();
	m_charom = memregion("charom")->base();

	// allocate memory
	m_video_ram.allocate(0x400);
	m_buffer_ram.allocate(0x800);

	// allocate timer
	m_todclk_timer = timer_alloc();
	m_todclk_timer->adjust(attotime::from_hz(60 * 2), 0, attotime::from_hz(60 * 2));

	// state saving
	save_item(NAME(m_dramon));
	save_item(NAME(m_statvid));
	save_item(NAME(m_vicdotsel));
	save_item(NAME(m_vicbnksel));
	save_item(NAME(m_todclk));
	save_item(NAME(m_vic_irq));
	save_item(NAME(m_tpi1_irq));
	save_item(NAME(m_cass_rd));
	save_item(NAME(m_user_flag));
	save_item(NAME(m_tpi2_pa));
	save_item(NAME(m_tpi2_pb));
	save_item(NAME(m_cia_pa));
}


//-------------------------------------------------
//  MACHINE_RESET( p500 )
//-------------------------------------------------

void p500_state::machine_reset()
{
	m_dramon = 1;
	m_statvid = 1;
	m_vicdotsel = 1;
	m_vicbnksel = 0x03;
	m_vic_irq = CLEAR_LINE;
	m_tpi1_irq = CLEAR_LINE;
	m_cass_rd = 1;

	m_maincpu->reset();

	m_tpi1->reset();
	m_tpi2->reset();
	m_acia->reset();
	m_cia->reset();

	m_ieee->reset();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( p500 )
//-------------------------------------------------

static MACHINE_CONFIG_START( p500, p500_state )
	// basic hardware
	MCFG_CPU_ADD(M6509_TAG, M6509, VIC6567_CLOCK)
	MCFG_CPU_PROGRAM_MAP(p500_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6509_TAG)

	// video hardware
	MCFG_MOS6567_ADD(MOS6567_TAG, SCREEN_TAG, VIC6567_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6851_TAG, SID6581, VIC6567_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_PLS100_ADD(PLA1_TAG)
	MCFG_PLS100_ADD(PLA2_TAG)
	MCFG_TPI6525_ADD(MOS6525_1_TAG, tpi1_intf)
	MCFG_TPI6525_ADD(MOS6525_2_TAG, tpi2_intf)
	MCFG_ACIA6551_ADD(MOS6551A_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_TAG, VIC6567_CLOCK, 60, cia_intf)
	//MCFG_QUICKLOAD_ADD("quickload", cbm_p500, "p00,prg,t64", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_CBM_IEEE488_ADD(ieee488_intf, NULL)
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_CBM2_EXPANSION_SLOT_ADD(CBM2_EXPANSION_SLOT_TAG, VIC6567_CLOCK, cbm2_expansion_cards, NULL, NULL)
	//MCFG_CBM2_USER_PORT_ADD(CBM2_USER_PORT_TAG, user_intf, cbm2_user_port_cards, NULL, NULL)
	//MCFG_CBM2_SYSTEM_PORT_ADD(CBM2_SYSTEM_PORT_TAG, system_intf, cbm2_system_port_cards, NULL, NULL)

	// software list

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("256K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( p500 )
//-------------------------------------------------

ROM_START( p500 )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901236-01.u84", 0x0000, 0x2000, CRC(33eb6aa2) SHA1(7e3497ae2edbb38c753bd31ed1bf3ae798c9a976), ROM_BIOS(1) )
	ROMX_LOAD( "901235-01.u83", 0x2000, 0x2000, CRC(18a27feb) SHA1(951b5370dd7db762b8504a141f9f26de345069bb), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901236-02.u84", 0x0000, 0x2000, CRC(c62ab16f) SHA1(f50240407bade901144f7e9f489fa9c607834eca), ROM_BIOS(2) )
	ROMX_LOAD( "901235-02.u83", 0x2000, 0x2000, CRC(20b7df33) SHA1(1b9a55f12f8cf025754d8029cc5324b474c35841), ROM_BIOS(2) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROMX_LOAD( "901234-01.u82", 0x0000, 0x2000, CRC(67962025) SHA1(24b41b65c85bf30ab4e2911f677ce9843845b3b1), ROM_BIOS(1) )
	ROMX_LOAD( "901234-02.u82", 0x0000, 0x2000, CRC(f46bbd2b) SHA1(097197d4d08e0b82e0466a5f1fbd49a24f3d2523), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u76", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-02.u78", 0x00, 0xf5, CRC(6436b20b) SHA1(57ebebe771791288051afd1abe9b7500bd2df847) )

	ROM_REGION( 0xf5, PLA2_TAG, 0 )
	ROM_LOAD( "906114-03.u88", 0x00, 0xf5, CRC(668c073e) SHA1(1115858bb2dc91ea9e2016ba2e23ec94239358b4) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   INIT                        COMPANY                         FULLNAME                            FLAGS
COMP( 1983,	p500,	0,		0,		p500,		p500,	driver_device,		0,		"Commodore Business Machines",	"P500 ~ B128-40 ~ PET-II (NTSC)",	GAME_NOT_WORKING )
