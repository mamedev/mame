/*

    TODO:

	- basic does not work
	- shift lock
	- Hungarian keyboard
	- cbm620hu charom banking?
	- read VIC video/color RAM thru PLA (Sphi2 = 1, AE = 0)
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
#define A0 BIT(offset, 0)
#define VA12 BIT(va, 12)



//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read_pla - low profile PLA read
//-------------------------------------------------

void cbm2_state::read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas, int busy2, 
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4)
{
	UINT32 input = P0 << 15 | P1 << 14 | P2 << 13 | P3 << 12 | busy2 << 11 | eras << 10 | ecas << 9 | refen << 8 | cas << 7 | ras << 6;
	UINT32 data = m_pla1->read(input);

	*casseg1 = BIT(data, 0);
	*rasseg1 = BIT(data, 1);
	*rasseg2 = BIT(data, 2);
	*casseg2 = BIT(data, 3);
	*rasseg4 = BIT(data, 4);
	*casseg4 = BIT(data, 5);
	*casseg3 = BIT(data, 6);
	*rasseg3 = BIT(data, 7);
}


//-------------------------------------------------
//  read_pla - high profile PLA read
//-------------------------------------------------

void cbm2hp_state::read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas, int busy2, 
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4)
{
	UINT32 input = ras << 13 | cas << 12 | refen << 11 | eras << 10 | ecas << 9 | busy2 << 8 | P3 << 3 | P2 << 2 | P1 << 1 | P0;
	UINT32 data = m_pla1->read(input);

	*casseg1 = BIT(data, 0);
	*casseg2 = BIT(data, 1);
	*casseg3 = BIT(data, 2);
	*casseg4 = BIT(data, 3);
	*rasseg1 = BIT(data, 4);
	*rasseg2 = BIT(data, 5);
	*rasseg3 = BIT(data, 6);
	*rasseg4 = BIT(data, 7);
}


//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void cbm2_state::bankswitch(offs_t offset, int busy2, int eras, int ecas, int refen, int cas, int ras, int *sysioen, int *dramen,
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *buframcs, int *extbufcs, int *vidramcs, 
	int *diskromcs, int *csbank1, int *csbank2, int *csbank3, int *basiccs, int *knbcs, int *kernalcs,
	int *crtccs, int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs)
{
	int rasseg1 = 1, rasseg2 = 1, rasseg3 = 1, rasseg4 = 1;

	this->read_pla(offset, ras, cas, refen, eras, ecas, busy2, casseg1, casseg2, casseg3, casseg4, &rasseg1, &rasseg2, &rasseg3, &rasseg4);

	int busen1 = m_dramon;
	int decoden = 0;
	*sysioen = !(P0 && P1 && P2 && P3) && busen1;
	*dramen = !((!(P0 && P1 && P2 && P3)) && busen1);

	if (!decoden && !*sysioen)
	{
		switch ((offset >> 13) & 0x07)
		{
		case 0:
			switch ((offset >> 11) & 0x03)
			{
			case 0: *buframcs = 0; break;
			case 1: *extbufcs = 0; break;
			case 2: // fallthru
			case 3: *diskromcs = 0; break;
			}
			break;

		case 1: *csbank1 = 0; break;
		case 2: *csbank2 = 0; break;
		case 3: *csbank3 = 0; break;
		case 4: *basiccs = 0; break;
		case 5: *knbcs = 0; break;
		case 6:
			switch ((offset >> 11) & 0x03)
			{
			case 2: *vidramcs = 0; break;
			case 3:
				switch ((offset >> 8) & 0x07)
				{
				case 0: *crtccs = 0; break;
				case 1: *cs1 = 0; break;
				case 2: *sidcs = 0; break;
				case 3: *extprtcs = 0; break;
				case 4: *ciacs = 0; break;
				case 5: *aciacs = 0; break;
				case 6: *tript1cs = 0; break;
				case 7: *tript2cs = 0; break;
				}
				break;
			}
			break;

		case 7: *kernalcs = 0; break;
		}
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( cbm2_state::read )
{
	int busy2 = 1, eras = 1, ecas = 1, refen = 0, cas = 0, ras = 1, sysioen = 1, dramen = 1;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, buframcs = 1, extbufcs = 1, vidramcs = 1;
	int diskromcs = 1, csbank1 = 1, csbank2 = 1, csbank3 = 1, basiccs = 1, knbcs = 1, kernalcs = 1;
	int crtccs = 1, cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;

	bankswitch(offset, busy2, eras, ecas, refen, cas, ras, &sysioen, &dramen,
		&casseg1, &casseg2, &casseg3, &casseg4, &buframcs, &extbufcs, &vidramcs,
		&diskromcs, &csbank1, &csbank2, &csbank3, &basiccs, &knbcs, &kernalcs,
		&crtccs, &cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs);
/*
	if (!space.debugger_access())
	logerror("r %05x %u %u - %u %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u %u %u %u %u %u %u\n", offset, sysioen, dramen,
		casseg1, casseg2, casseg3, casseg4, buframcs, extbufcs, vidramcs,
		diskromcs, csbank1, csbank2, csbank3, basiccs, knbcs, kernalcs,
		crtccs, cs1, sidcs, extprtcs, ciacs, aciacs, tript1cs, tript2cs);
*/
	UINT8 data = 0;

	if (!dramen)
	{
		if (!casseg1)
		{
			data = m_ram->pointer()[offset & 0xffff];
		}
		if (!casseg2)
		{
			data = m_ram->pointer()[0x10000 | (offset & 0xffff)];
		}
		if (!casseg3 && (m_ram->size() > 0x20000))
		{
			data = m_ram->pointer()[0x20000 | (offset & 0xffff)];
		}
		if (!casseg4 && (m_ram->size() > 0x30000))
		{
			data = m_ram->pointer()[0x30000 | (offset & 0xffff)];
		}
	}

	if (!sysioen)
	{
		if (!buframcs)
		{
			data = m_buffer_ram[offset & 0x7ff];
		}
		if (!vidramcs)
		{
			data = m_video_ram[offset & 0x7ff];
		}
		if (!basiccs || !knbcs)
		{
			data = m_basic[offset & 0x3fff];
		}
		if (!kernalcs)
		{
			data = m_kernal[offset & 0x1fff];
		}
		if (!crtccs)
		{
			if (A0)
			{
				data = m_crtc->register_r(space, 0);
			}
			else
			{
				data = m_crtc->status_r(space, 0);
			}
		}
		if (!sidcs)
		{
			data = m_sid->read(space, offset & 0x1f);
		}
		if (!ciacs)
		{
			data = m_cia->read(space, offset & 0x0f);
		}
		if (!aciacs)
		{
			data = m_acia->read(space, offset & 0x03);
		}
		if (!tript1cs)
		{
			data = m_tpi1->read(space, offset & 0x07);
		}
		if (!tript2cs)
		{
			data = m_tpi2->read(space, offset & 0x07);
		}

		data = m_exp->read(space, offset & 0x1fff, data, csbank1, csbank2, csbank3);
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( cbm2_state::write )
{
	int busy2 = 1, eras = 1, ecas = 1, refen = 0, cas = 0, ras = 1, sysioen = 1, dramen = 1;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, buframcs = 1, extbufcs = 1, vidramcs = 1;
	int diskromcs = 1, csbank1 = 1, csbank2 = 1, csbank3 = 1, basiccs = 1, knbcs = 1, kernalcs = 1;
	int crtccs = 1, cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;

	bankswitch(offset, busy2, eras, ecas, refen, cas, ras, &sysioen, &dramen,
		&casseg1, &casseg2, &casseg3, &casseg4, &buframcs, &extbufcs, &vidramcs,
		&diskromcs, &csbank1, &csbank2, &csbank3, &basiccs, &knbcs, &kernalcs,
		&crtccs, &cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs);
/*
	if (!space.debugger_access())
	logerror("w %05x %u %u - %u %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u %u %u %u %u %u %u\n", offset, sysioen, dramen,
		casseg1, casseg2, casseg3, casseg4, buframcs, extbufcs, vidramcs,
		diskromcs, csbank1, csbank2, csbank3, basiccs, knbcs, kernalcs,
		crtccs, cs1, sidcs, extprtcs, ciacs, aciacs, tript1cs, tript2cs);
*/
	if (!dramen)
	{
		if (!casseg1)
		{
			m_ram->pointer()[offset & 0xffff] = data;
		}
		if (!casseg2)
		{
			m_ram->pointer()[0x10000 | (offset & 0xffff)] = data;
		}
		if (!casseg3 && (m_ram->size() > 0x20000))
		{
			m_ram->pointer()[0x20000 | (offset & 0xffff)] = data;
		}
		if (!casseg4 && (m_ram->size() > 0x30000))
		{
			m_ram->pointer()[0x30000 | (offset & 0xffff)] = data;
		}
	}

	if (!sysioen)
	{
		if (!buframcs)
		{
			m_buffer_ram[offset & 0x7ff] = data;
		}
		if (!vidramcs)
		{
			m_video_ram[offset & 0x7ff] = data;
		}
		if (!basiccs || !knbcs)
		{
			m_basic[offset & 0x3fff] = data;
		}
		if (!kernalcs)
		{
			m_kernal[offset & 0x1fff] = data;
		}
		if (!crtccs)
		{
			if (A0)
			{
				m_crtc->register_w(space, 0, data);
			}
			else
			{
				m_crtc->address_w(space, 0, data);
			}
		}
		if (!sidcs)
		{
			m_sid->write(space, offset & 0x1f, data);
		}
		if (!ciacs)
		{
			m_cia->write(space, offset & 0x0f, data);
		}
		if (!aciacs)
		{
			m_acia->write(space, offset & 0x03, data);
		}
		if (!tript1cs)
		{
			m_tpi1->write(space, offset & 0x07, data);
		}
		if (!tript2cs)
		{
			m_tpi2->write(space, offset & 0x07, data);
		}

		m_exp->write(space, offset & 0x1fff, data, csbank1, csbank2, csbank3);
	}
}


//-------------------------------------------------
//  read_pla1 - P500 PLA #1 read
//-------------------------------------------------

void p500_state::read_pla1(offs_t offset, int bras, int busy2, int sphi2, int clrnibcsb, int procvid, int refen, int ba, int aec, int srw,
	int *datxen, int *dramxen, int *clrniben, int *segf, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr)
{
	UINT32 input = P0 << 15 | P2 << 14 | bras << 13 | P1 << 12 | P3 << 11 | busy2 << 10 | m_statvid << 9 | sphi2 << 8 |
			clrnibcsb << 7 | m_dramon << 6 | procvid << 5 | refen << 4 | m_vicdotsel << 3 | ba << 2 | aec << 1 | srw;
	
	UINT32 data = m_pla1->read(input);

	*datxen = BIT(data, 0);
	*dramxen = BIT(data, 1);
	*clrniben = BIT(data, 2);
	*segf = BIT(data, 3);
	*_64kcasen = BIT(data, 4);
	*casenb = BIT(data, 5);
	*viddaten = BIT(data, 6);
	*viddat_tr = BIT(data, 7);
}


//-------------------------------------------------
//  read_pla2 - P500 PLA #2 read
//-------------------------------------------------

void p500_state::read_pla2(offs_t offset, offs_t va, int ba, int sphi2, int vicen, int ae, int segf, int bcas, int bank0,
	int *clrnibcsb, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *procvid, int *viccs, int *vidmatcs)
{
	UINT32 input = VA12 << 15 | ba << 14 | A13 << 13 | A15 << 12 | A14 << 11 | A11 << 10 | A10 << 9 | A12 << 8 |
			sphi2 << 7 | vicen << 6 | m_statvid << 5 | m_vicdotsel << 4 | ae << 3 | segf << 2 | bcas << 1 | bank0;
	
	UINT32 data = m_pla2->read(input);

	*clrnibcsb = BIT(data, 0);
	*extbufcs = BIT(data, 1);
	*discromcs = BIT(data, 2);
	*buframcs = BIT(data, 3);
	*charomcs = BIT(data, 4);
	*procvid = BIT(data, 5);
	*viccs = BIT(data, 6);
	*vidmatcs = BIT(data, 7);
}


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
	*vsysaden = sphi1 || ba;

	int clrnibcsb = 1, procvid = 1, segf = 1;

	read_pla1(offset, bras, busy2, sphi2, clrnibcsb, procvid, refen, ba, *aec, srw, 
		datxen, dramxen, clrniben, &segf, _64kcasen, casenb, viddaten, viddat_tr);

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

	int vidmatcsb = 1;

	read_pla2(offset, va, ba, sphi2, vicen, ae, segf, bcas, bank0,
		&clrnibcsb, extbufcs, discromcs, buframcs, charomcs, &procvid, viccs, &vidmatcsb);

	*clrnibcs = clrnibcsb || bcas;
	*vidmatcs = vidmatcsb || bcas;

	read_pla1(offset, bras, busy2, sphi2, clrnibcsb, procvid, refen, ba, *aec, srw, 
		datxen, dramxen, clrniben, &segf, _64kcasen, casenb, viddaten, viddat_tr);
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

UINT8 p500_state::read_memory(address_space &space, offs_t offset, offs_t va, int sphi0, int sphi1, int sphi2, int ba, int ae, int bras, int bcas, UINT8 *clrnib)
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
	if (!space.debugger_access())
	logerror("r %05x %u %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u : ",
		va, datxen, dramxen, clrniben, _64kcasen, casenb, viddaten, viddat_tr,
		clrnibcs, extbufcs, discromcs, buframcs, charomcs, viccs, vidmatcs,
		csbank1, csbank2, csbank3, basiclocs, basichics, kernalcs,
		cs1, sidcs, extprtcs, ciacs, aciacs, tript1cs, tript2cs, aec, vsysaden);
*/
	UINT8 data = 0xff;
	*clrnib = 0xf;

	if (vsysaden)
	{
		if (!_64kcasen && !aec && !viddaten && !viddat_tr)
		{
			data = m_ram->pointer()[(m_vicbnksel << 14) | va];
		}
		if (!clrnibcs)
		{
			*clrnib = m_color_ram[va & 0x3ff];
		}
		if (!vidmatcs)
		{
			data = m_video_ram[va & 0x3ff];
		}
		if (!charomcs)
		{
			data = m_charom[va & 0xfff];
		}
	}
	
	if (clrniben)
	{
		if (!clrnibcs && !vsysaden)
		{
			data = m_color_ram[offset & 0x3ff];
		}
	}

	if (!dramxen)
	{
		if (casenb)
		{
			switch (offset >> 16)
			{
			case 1: data = m_ram->pointer()[0x10000 + (offset & 0xffff)]; break;
			case 2: if (m_ram->size() > 0x20000) data = m_ram->pointer()[0x20000 + (offset & 0xffff)]; break;
			case 3: if (m_ram->size() > 0x30000) data = m_ram->pointer()[0x30000 + (offset & 0xffff)]; break;
			}
		}
	}

	if (!datxen)
	{
		if (!_64kcasen && !aec)
		{
			data = m_ram->pointer()[offset & 0xffff];
		}
		if (!buframcs)
		{
			data = m_buffer_ram[offset & 0x7ff];
		}
		if (!vidmatcs && !vsysaden && !viddaten && viddat_tr)
		{
			data = m_video_ram[offset & 0x3ff];
		}
		if (!basiclocs || !basichics)
		{
			data = m_basic[offset & 0x3fff];
		}
		if (!kernalcs)
		{
			data = m_kernal[offset & 0x1fff];
		}
		if (!charomcs && !vsysaden && !viddaten && viddat_tr)
		{
			data = m_charom[offset & 0xfff];
		}
		if (!viccs && !viddaten && viddat_tr)
		{
			data = m_vic->read(space, offset & 0x3f);
		}
		if (!sidcs)
		{
			data = m_sid->read(space, offset & 0x1f);
		}
		if (!ciacs)
		{
			data = m_cia->read(space, offset & 0x0f);
		}
		if (!aciacs)
		{
			data = m_acia->read(space, offset & 0x03);
		}
		if (!tript1cs)
		{
			data = m_tpi1->read(space, offset & 0x07);
		}
		if (!tript2cs)
		{
			data = m_tpi2->read(space, offset & 0x07);
		}

		data = m_exp->read(space, offset & 0x1fff, data, csbank1, csbank2, csbank3);
	}

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
	logerror("w %05x %u %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u %u %u %u %u - %u %u %u %u %u %u %u - %u %u : ",
		offset, datxen, dramxen, clrniben, _64kcasen, casenb, viddaten, viddat_tr,
		clrnibcs, extbufcs, discromcs, buframcs, charomcs, viccs, vidmatcs,
		csbank1, csbank2, csbank3, basiclocs, basichics, kernalcs,
		cs1, sidcs, extprtcs, ciacs, aciacs, tript1cs, tript2cs, aec, vsysaden);
*/

	if (clrniben)
	{
		if (!clrnibcs && !vsysaden)
		{
			m_color_ram[offset & 0x3ff] = data & 0x0f;
		}
	}

	if (!dramxen)
	{
		if (casenb)
		{
			switch (offset >> 16)
			{
			case 1: m_ram->pointer()[0x10000 + (offset & 0xffff)] = data; break;
			case 2: if (m_ram->size() > 0x20000) m_ram->pointer()[0x20000 + (offset & 0xffff)] = data; break;
			case 3: if (m_ram->size() > 0x30000) m_ram->pointer()[0x30000 + (offset & 0xffff)] = data; break;
			}
		}
	}

	if (!datxen)
	{
		if (!_64kcasen && !aec)
		{
			m_ram->pointer()[offset & 0xffff] = data;
		}
		if (!buframcs)
		{
			m_buffer_ram[offset & 0x7ff] = data;
		}
		if (!vidmatcs && !vsysaden && !viddaten && !viddat_tr)
		{
			m_video_ram[offset & 0x3ff] = data;
		}
		if (!viccs && !viddaten && !viddat_tr)
		{
			m_vic->write(space, offset & 0x3f, data);
		}
		if (!sidcs)
		{
			m_sid->write(space, offset & 0x1f, data);
		}
		if (!ciacs)
		{
			m_cia->write(space, offset & 0x0f, data);
		}
		if (!aciacs)
		{
			m_acia->write(space, offset & 0x03, data);
		}
		if (!tript1cs)
		{
			m_tpi1->write(space, offset & 0x07, data);
		}
		if (!tript2cs)
		{
			m_tpi2->write(space, offset & 0x07, data);
		}

		m_exp->write(space, offset & 0x1fff, data, csbank1, csbank2, csbank3);
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( p500_state::read )
{
	int sphi0 = 1, sphi1 = 0, sphi2 = 1, ba = 0, ae = 1, bras = 1, bcas = 0;
	offs_t va = 0xffff;
	UINT8 clrnib = 0xf;

	return read_memory(space, offset, va, sphi0, sphi1, sphi2, ba, ae, bras, bcas, &clrnib);
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
	/*
	int sphi0 = 0, sphi1 = 1, sphi2 = 0, ba = 1, ae = 0, bras = 0, bcas = 0;
	offs_t va = offset;

	return read_memory(space, 0, va, sphi0, sphi1, sphi2, ba, ae, bras, bcas);
	*/
	/*
	int ba = 1, ae = 0, bras = 1, bcas = 0;
	UINT8 clrnib = 0xf;

	if (offset < 0x1000)
	{
		return read_memory(space, 0, offset, 0, 1, 0, ba, ae, bras, bcas, &clrnib);
	}
	else
	{
		return read_memory(space, 0, offset, 1, 0, 1, ba, ae, bras, bcas, &clrnib);
	}
	*/
	
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
//  ADDRESS_MAP( cbm2_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( cbm2_mem, AS_PROGRAM, 8, cbm2_state )
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


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
//  INPUT_PORTS( cbm2 )
//-------------------------------------------------

static INPUT_PORTS_START( cbm2 )
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

	PORT_START("LOCK")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cbm2hu )
//-------------------------------------------------

static INPUT_PORTS_START( cbm2hu )
	PORT_INCLUDE(cbm2)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cbm2sw )
//-------------------------------------------------

static INPUT_PORTS_START( cbm2sw )
	PORT_INCLUDE(cbm2)

	PORT_MODIFY("PA0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)

	PORT_MODIFY("PA1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)

	PORT_MODIFY("PA2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT" \xCF\x80") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(UCHAR_MAMEKEY(TILDE)) PORT_CHAR(0x03c0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(';') PORT_CHAR(':')
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  mc6845_interface crtc_intf
//-------------------------------------------------

static MC6845_UPDATE_ROW( lp_crtc_update_row )
{
	cbm2_state *state = device->machine().driver_data<cbm2_state>();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = state->m_video_ram[(ma + column) & 0x7ff];
		offs_t char_rom_addr = (ma & 0x1000) | (state->m_graphics << 11) | ((code & 0x7f) << 4) | (ra & 0x0f);
		UINT8 data = state->m_charom[char_rom_addr & 0xfff];

		for (int bit = 0; bit < 8; bit++)
		{
			int color = BIT(data, 7) ^ BIT(code, 7) ^ BIT(ma, 13);
			if (cursor_x == column) color ^= 1;
			
			bitmap.pix32(y, x++) = RGB_MONOCHROME_GREEN[color];

			data <<= 1;
		}
	}
}

static const mc6845_interface lp_crtc_intf =
{
	SCREEN_TAG,
	8,
	NULL,
	lp_crtc_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};


static MC6845_UPDATE_ROW( hp_crtc_update_row )
{
	cbm2_state *state = device->machine().driver_data<cbm2_state>();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = state->m_video_ram[(ma + column) & 0x7ff];
		offs_t char_rom_addr = (ma & 0x1000) | (state->m_graphics << 11) | ((code & 0x7f) << 4) | (ra & 0x0f);
		UINT8 data = state->m_charom[char_rom_addr & 0xfff];

		for (int bit = 0; bit < 8; bit++)
		{
			int color = BIT(data, 7) ^ BIT(code, 7) ^ BIT(ma, 13);
			if (cursor_x == column) color ^= 1;
			
			bitmap.pix32(y, x++) = RGB_MONOCHROME_GREEN[color];

			data <<= 1;
		}

		x++;
	}
}

static const mc6845_interface hp_crtc_intf =
{
	SCREEN_TAG,
	9,
	NULL,
	hp_crtc_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

//-------------------------------------------------
//  vic2_interface vic_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( p500_state::vic_irq_w )
{
	m_vic_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_vic_irq || m_tpi1_irq || m_user_irq);
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

READ8_MEMBER( cbm2_state::sid_potx_r )
{
	int sela = BIT(m_cia_pa, 6);
	int selb = BIT(m_cia_pa, 7);

	UINT8 data = 0;

	if (sela) data = m_joy1->pot_x_r();
	if (selb) data = m_joy2->pot_x_r();

	return data;
}

READ8_MEMBER( cbm2_state::sid_poty_r )
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
	DEVCB_DRIVER_MEMBER(cbm2_state, sid_potx_r),
	DEVCB_DRIVER_MEMBER(cbm2_state, sid_poty_r)
};


//-------------------------------------------------
//  tpi6525_interface tpi1_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( cbm2_state::tpi1_irq_w )
{
	m_tpi1_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_tpi1_irq || m_user_irq);
}

WRITE_LINE_MEMBER( p500_state::tpi1_irq_w )
{
	m_tpi1_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_vic_irq || m_tpi1_irq || m_user_irq);
}

READ8_MEMBER( cbm2_state::tpi1_pa_r )
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

WRITE8_MEMBER( cbm2_state::tpi1_pa_w )
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

READ8_MEMBER( cbm2_state::tpi1_pb_r )
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

WRITE8_MEMBER( cbm2_state::tpi1_pb_w )
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

WRITE_LINE_MEMBER( cbm2_state::tpi1_ca_w )
{
	m_graphics = state;
}

WRITE_LINE_MEMBER( p500_state::tpi1_ca_w )
{
	m_statvid = state;
}

WRITE_LINE_MEMBER( p500_state::tpi1_cb_w )
{
	m_vicdotsel = state;
}

static const tpi6525_interface tpi1_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(cbm2_state, tpi1_irq_w),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi1_pa_r),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi1_pa_w),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi1_pb_r),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi1_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(cbm2_state, tpi1_ca_w),
	DEVCB_NULL
};

static const tpi6525_interface p500_tpi1_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(p500_state, tpi1_irq_w),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi1_pa_r),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi1_pa_w),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi1_pb_r),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi1_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(p500_state, tpi1_ca_w),
	DEVCB_DRIVER_LINE_MEMBER(p500_state, tpi1_cb_w)
};


//-------------------------------------------------
//  tpi6525_interface tpi2_intf
//-------------------------------------------------

UINT8 cbm2_state::read_keyboard()
{
	UINT8 data = 0xff;

	if (!BIT(m_tpi2_pa, 0)) data &= ioport("PA0")->read();
	if (!BIT(m_tpi2_pa, 1)) data &= ioport("PA1")->read();
	if (!BIT(m_tpi2_pa, 2)) data &= ioport("PA2")->read();
	if (!BIT(m_tpi2_pa, 3)) data &= ioport("PA3")->read();
	if (!BIT(m_tpi2_pa, 4)) data &= ioport("PA4")->read();
	if (!BIT(m_tpi2_pa, 5)) data &= ioport("PA5")->read();
	if (!BIT(m_tpi2_pa, 6)) data &= ioport("PA6")->read();
	if (!BIT(m_tpi2_pa, 7)) data &= ioport("PA7")->read();
	if (!BIT(m_tpi2_pb, 0)) data &= ioport("PB0")->read() & ioport("LOCK")->read();
	if (!BIT(m_tpi2_pb, 1)) data &= ioport("PB1")->read();
	if (!BIT(m_tpi2_pb, 2)) data &= ioport("PB2")->read();
	if (!BIT(m_tpi2_pb, 3)) data &= ioport("PB3")->read();
	if (!BIT(m_tpi2_pb, 4)) data &= ioport("PB4")->read();
	if (!BIT(m_tpi2_pb, 5)) data &= ioport("PB5")->read();
	if (!BIT(m_tpi2_pb, 6)) data &= ioport("PB6")->read();
	if (!BIT(m_tpi2_pb, 7)) data &= ioport("PB7")->read();

	return data;
}

WRITE8_MEMBER( cbm2_state::tpi2_pa_w )
{
	m_tpi2_pa = data;
}

WRITE8_MEMBER( cbm2_state::tpi2_pb_w )
{
	m_tpi2_pb = data;
}

READ8_MEMBER( cbm2_state::tpi2_pc_r )
{
	/*
	
	    bit     description
	
	    0       COLUMN 0
	    1       COLUMN 1
	    2       COLUMN 2
	    3       COLUMN 3
	    4       COLUMN 4
	    5       COLUMN 5
	    6       0=PAL, 1=NTSC
	    7       0
	
	*/

	return (m_ntsc << 6) | (read_keyboard() & 0x3f);
}

READ8_MEMBER( cbm2hp_state::tpi2_pc_r )
{
	/*
	
	    bit     description
	
	    0       COLUMN 0
	    1       COLUMN 1
	    2       COLUMN 2
	    3       COLUMN 3
	    4       COLUMN 4
	    5       COLUMN 5
	    6       1
	    7       1
	
	*/

	return read_keyboard();
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

	return read_keyboard();
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
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi2_pa_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi2_pb_w),
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi2_pc_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const tpi6525_interface hp_tpi2_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi2_pa_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi2_pb_w),
	DEVCB_DRIVER_MEMBER(cbm2hp_state, tpi2_pc_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const tpi6525_interface p500_tpi2_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi2_pa_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(cbm2_state, tpi2_pb_w),
	DEVCB_DRIVER_MEMBER(p500_state, tpi2_pc_r),
	DEVCB_DRIVER_MEMBER(p500_state, tpi2_pc_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  mos6526_interface cia_intf
//-------------------------------------------------

READ8_MEMBER( cbm2_state::cia_pa_r )
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

WRITE8_MEMBER( cbm2_state::cia_pa_w )
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

READ8_MEMBER( cbm2_state::cia_pb_r )
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

WRITE8_MEMBER( cbm2_state::cia_pb_w )
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
	DEVCB_DRIVER_MEMBER(cbm2_state, cia_pa_r),
	DEVCB_DRIVER_MEMBER(cbm2_state, cia_pa_w),
	DEVCB_DRIVER_MEMBER(cbm2_state, cia_pb_r),
	DEVCB_DRIVER_MEMBER(cbm2_state, cia_pb_w),
};


//-------------------------------------------------
//  PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( cbm2_state::tape_read_w )
{
	m_cass_rd = state;

	mos6526_flag_w(m_cia, m_cass_rd && m_user_flag);
}

static PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(cbm2_state, tape_read_w)
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

void cbm2_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_tpi1->i0_w(m_todclk);

	m_todclk = !m_todclk;
}


//-------------------------------------------------
//  MACHINE_START( cbm2 )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2 )
{
	// find memory regions
	m_basic = memregion("basic")->base();
	m_kernal = memregion("kernal")->base();
	m_charom = memregion("charom")->base();

	// allocate memory
	m_video_ram.allocate(m_video_ram_size);
	m_buffer_ram.allocate(0x800);

	// allocate timer
	int todclk = (m_ntsc ? 60 : 50) * 2;

	m_todclk_timer = timer_alloc();
	m_todclk_timer->adjust(attotime::from_hz(todclk), 0, attotime::from_hz(todclk));

	// state saving
	save_item(NAME(m_dramon));
	save_item(NAME(m_graphics));
	save_item(NAME(m_ntsc));
	save_item(NAME(m_todclk));
	save_item(NAME(m_tpi1_irq));
	save_item(NAME(m_cass_rd));
	save_item(NAME(m_user_flag));
	save_item(NAME(m_tpi2_pa));
	save_item(NAME(m_tpi2_pb));
	save_item(NAME(m_cia_pa));
}


//-------------------------------------------------
//  MACHINE_START( cbm2_ntsc )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2_ntsc )
{
	m_ntsc = 1;

	MACHINE_START_CALL_MEMBER(cbm2);
}


//-------------------------------------------------
//  MACHINE_START( cbm2_pal )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2_pal )
{
	m_ntsc = 0;

	MACHINE_START_CALL_MEMBER(cbm2);
}


//-------------------------------------------------
//  MACHINE_START( p500 )
//-------------------------------------------------

MACHINE_START_MEMBER( p500_state, p500 )
{
	m_video_ram_size = 0x400;

	MACHINE_START_CALL_MEMBER(cbm2);

	// state saving
	save_item(NAME(m_statvid));
	save_item(NAME(m_vicdotsel));
	save_item(NAME(m_vicbnksel));
	save_item(NAME(m_vic_irq));
}


//-------------------------------------------------
//  MACHINE_START( p500_ntsc )
//-------------------------------------------------

MACHINE_START_MEMBER( p500_state, p500_ntsc )
{
	m_ntsc = 1;

	MACHINE_START_CALL_MEMBER(p500);
}


//-------------------------------------------------
//  MACHINE_START( p500_pal )
//-------------------------------------------------

MACHINE_START_MEMBER( p500_state, p500_pal )
{
	m_ntsc = 0;

	MACHINE_START_CALL_MEMBER(p500);
}


//-------------------------------------------------
//  MACHINE_RESET( cbm2 )
//-------------------------------------------------

MACHINE_RESET_MEMBER( cbm2_state, cbm2 )
{
	m_dramon = 1;
	m_graphics = 1;
	m_tpi1_irq = CLEAR_LINE;
	m_cass_rd = 1;
	m_user_irq = CLEAR_LINE;

	m_maincpu->reset();

	m_tpi1->reset();
	m_tpi2->reset();
	m_acia->reset();
	m_cia->reset();

	m_ieee->reset();
}


//-------------------------------------------------
//  MACHINE_RESET( p500 )
//-------------------------------------------------

MACHINE_RESET_MEMBER( p500_state, p500 )
{
	MACHINE_RESET_CALL_MEMBER(cbm2);

	m_statvid = 1;
	m_vicdotsel = 1;
	m_vicbnksel = 0x03;
	m_vic_irq = CLEAR_LINE;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( 128k )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( 128k )
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("256K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( 256k )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( 256k )
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( p500_ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( p500_ntsc, p500_state )
	MCFG_MACHINE_START_OVERRIDE(p500_state, p500_ntsc)
	MCFG_MACHINE_RESET_OVERRIDE(p500_state, p500)

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
	MCFG_TPI6525_ADD(MOS6525_1_TAG, p500_tpi1_intf)
	MCFG_TPI6525_ADD(MOS6525_2_TAG, p500_tpi2_intf)
	MCFG_ACIA6551_ADD(MOS6551A_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_TAG, VIC6567_CLOCK, 60, cia_intf)
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c8050")
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_CBM2_EXPANSION_SLOT_ADD(CBM2_EXPANSION_SLOT_TAG, VIC6567_CLOCK, cbm2_expansion_cards, NULL, NULL)
	//MCFG_CBM2_USER_PORT_ADD(CBM2_USER_PORT_TAG, user_intf, cbm2_user_port_cards, NULL, NULL)
	//MCFG_CBM2_SYSTEM_PORT_ADD(CBM2_SYSTEM_PORT_TAG, system_intf, cbm2_system_port_cards, NULL, NULL)

	// internal ram
	MCFG_FRAGMENT_ADD(128k)

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "p500_flop")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "cbm2_cart")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( p500_pal )
//-------------------------------------------------

static MACHINE_CONFIG_START( p500_pal, p500_state )
	MCFG_MACHINE_START_OVERRIDE(p500_state, p500_pal)
	MCFG_MACHINE_RESET_OVERRIDE(p500_state, p500)

	// basic hardware
	MCFG_CPU_ADD(M6509_TAG, M6509, VIC6569_CLOCK)
	MCFG_CPU_PROGRAM_MAP(p500_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6509_TAG)

	// video hardware
	MCFG_MOS6569_ADD(MOS6569_TAG, SCREEN_TAG, VIC6569_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6851_TAG, SID6581, VIC6569_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_PLS100_ADD(PLA1_TAG)
	MCFG_PLS100_ADD(PLA2_TAG)
	MCFG_TPI6525_ADD(MOS6525_1_TAG, p500_tpi1_intf)
	MCFG_TPI6525_ADD(MOS6525_2_TAG, p500_tpi2_intf)
	MCFG_ACIA6551_ADD(MOS6551A_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_TAG, VIC6569_CLOCK, 50, cia_intf)
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c8050")
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_CBM2_EXPANSION_SLOT_ADD(CBM2_EXPANSION_SLOT_TAG, VIC6569_CLOCK, cbm2_expansion_cards, NULL, NULL)
	//MCFG_CBM2_USER_PORT_ADD(CBM2_USER_PORT_TAG, user_intf, cbm2_user_port_cards, NULL, NULL)
	//MCFG_CBM2_SYSTEM_PORT_ADD(CBM2_SYSTEM_PORT_TAG, system_intf, cbm2_system_port_cards, NULL, NULL)

	// internal ram
	MCFG_FRAGMENT_ADD(128k)

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "p500_flop")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "cbm2_cart")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm2lp_ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm2lp_ntsc, cbm2_state )
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_ntsc)
	MCFG_MACHINE_RESET_OVERRIDE(cbm2_state, cbm2)

	// basic hardware
	MCFG_CPU_ADD(M6509_TAG, M6509, XTAL_18MHz/8)
	MCFG_CPU_PROGRAM_MAP(cbm2_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6509_TAG)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(MC68B45_TAG, mc6845_device, screen_update)

	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(768, 312)
	MCFG_SCREEN_VISIBLE_AREA(0, 768-1, 0, 312-1)

	MCFG_MC6845_ADD(MC68B45_TAG, MC6845, XTAL_18MHz/8, lp_crtc_intf)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6851_TAG, SID6581, XTAL_18MHz/8)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_PLS100_ADD(PLA1_TAG)
	MCFG_TPI6525_ADD(MOS6525_1_TAG, tpi1_intf)
	MCFG_TPI6525_ADD(MOS6525_2_TAG, tpi2_intf)
	MCFG_ACIA6551_ADD(MOS6551A_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_TAG, XTAL_18MHz/8, 60, cia_intf)
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c8050")
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_CBM2_EXPANSION_SLOT_ADD(CBM2_EXPANSION_SLOT_TAG, XTAL_18MHz/8, cbm2_expansion_cards, NULL, NULL)
	//MCFG_CBM2_USER_PORT_ADD(CBM2_USER_PORT_TAG, user_intf, cbm2_user_port_cards, NULL, NULL)
	//MCFG_CBM2_SYSTEM_PORT_ADD(CBM2_SYSTEM_PORT_TAG, system_intf, cbm2_system_port_cards, NULL, NULL)

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "cbm2_flop")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "cbm2_cart")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( b128 )
//-------------------------------------------------

static MACHINE_CONFIG_START( b128, cbm2_state )
	MCFG_FRAGMENT_ADD(cbm2lp_ntsc)
	MCFG_FRAGMENT_ADD(128k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( b256 )
//-------------------------------------------------

static MACHINE_CONFIG_START( b256, cbm2_state )
	MCFG_FRAGMENT_ADD(cbm2lp_ntsc)
	MCFG_FRAGMENT_ADD(256k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm2lp_pal )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm2lp_pal, cbm2_state )
	MCFG_FRAGMENT_ADD(cbm2lp_ntsc)

	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_pal)

	MCFG_DEVICE_REMOVE(MOS6526_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_TAG, XTAL_18MHz/8, 50, cia_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm610 )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm610, cbm2_state )
	MCFG_FRAGMENT_ADD(cbm2lp_pal)
	MCFG_FRAGMENT_ADD(128k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm620 )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm620, cbm2_state )
	MCFG_FRAGMENT_ADD(cbm2lp_pal)
	MCFG_FRAGMENT_ADD(256k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm2hp_ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm2hp_ntsc, cbm2hp_state )
	MCFG_FRAGMENT_ADD(cbm2lp_ntsc)

	MCFG_DEVICE_REMOVE(MC68B45_TAG)
	MCFG_MC6845_ADD(MC68B45_TAG, MC6845, XTAL_18MHz/8, hp_crtc_intf)

	// devices
	MCFG_DEVICE_REMOVE(MOS6525_2_TAG)
	MCFG_TPI6525_ADD(MOS6525_2_TAG, hp_tpi2_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( b128hp )
//-------------------------------------------------

static MACHINE_CONFIG_START( b128hp, cbm2hp_state )
	MCFG_FRAGMENT_ADD(cbm2hp_ntsc)
	MCFG_FRAGMENT_ADD(128k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( b256hp )
//-------------------------------------------------

static MACHINE_CONFIG_START( b256hp, cbm2hp_state )
	MCFG_FRAGMENT_ADD(cbm2hp_ntsc)
	MCFG_FRAGMENT_ADD(256k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( bx256hp )
//-------------------------------------------------

static MACHINE_CONFIG_START( bx256hp, cbm2hp_state )
	MCFG_FRAGMENT_ADD(b256hp)

	//MCFG_DEVICE_REMOVE(CBM2_SYSTEM_PORT_TAG)
	//MCFG_CBM2_SYSTEM_PORT_ADD(CBM2_SYSTEM_PORT_TAG, system_intf, cbm2_system_port_cards, "8088", NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm2hp_pal )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm2hp_pal, cbm2hp_state )
	MCFG_FRAGMENT_ADD(cbm2hp_ntsc)

	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_pal)

	// devices
	MCFG_DEVICE_REMOVE(MOS6525_2_TAG)
	MCFG_TPI6525_ADD(MOS6525_2_TAG, hp_tpi2_intf)

	MCFG_DEVICE_REMOVE(MOS6526_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_TAG, XTAL_18MHz/8, 50, cia_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm710 )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm710, cbm2hp_state )
	MCFG_FRAGMENT_ADD(cbm2hp_pal)
	MCFG_FRAGMENT_ADD(128k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm720 )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm720, cbm2hp_state )
	MCFG_FRAGMENT_ADD(cbm2hp_pal)
	MCFG_FRAGMENT_ADD(256k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm730 )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm730, cbm2hp_state )
	MCFG_FRAGMENT_ADD(cbm720)

	//MCFG_DEVICE_REMOVE(CBM2_SYSTEM_PORT_TAG)
	//MCFG_CBM2_SYSTEM_PORT_ADD(CBM2_SYSTEM_PORT_TAG, system_intf, cbm2_system_port_cards, "8088", NULL)
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( p500n )
//-------------------------------------------------

ROM_START( p500n )
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


//-------------------------------------------------
//  ROM( p500p )
//-------------------------------------------------

#define rom_p500p	rom_p500n


//-------------------------------------------------
//  ROM( b500 )
//-------------------------------------------------

ROM_START( b500 )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901243-01.u59",  0x0000, 0x2000, CRC(22822706) SHA1(901bbf59d8b8682b481be8b2de99b406fffa4bab) )
	ROM_LOAD( "901242-01a.u60", 0x2000, 0x2000, CRC(ef13d595) SHA1(2fb72985d7d4ab69c5780179178828c931a9f5b0) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "901244-01.u61",  0x0000, 0x2000, CRC(93414213) SHA1(a54a593dbb420ae1ac39b0acde9348160f7840ff) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.bin", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


//-------------------------------------------------
//  ROM( b128 )
//-------------------------------------------------

ROM_START( b128 )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901243-02b.u59", 0x0000, 0x2000, CRC(9d0366f9) SHA1(625f7337ea972a8bce2bdf2daababc0ed0b3b69b), ROM_BIOS(1) )
	ROMX_LOAD( "901242-02b.u60", 0x2000, 0x2000, CRC(837978b5) SHA1(56e8d2f86bf73ba36b3d3cb84dd75806b66c530a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901243-04a.u59", 0x0000, 0x2000, CRC(b0dcb56d) SHA1(08d333208060ee2ce84d4532028d94f71c016b96), ROM_BIOS(2) )
	ROMX_LOAD( "901242-04a.u60", 0x2000, 0x2000, CRC(de04ea4f) SHA1(7c6de17d46a3343dc597d9b9519cf63037b31908), ROM_BIOS(2) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(1) )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.bin", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


//-------------------------------------------------
//  ROM( b256 )
//-------------------------------------------------

ROM_START( b256 )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.bin", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


//-------------------------------------------------
//  ROM( cbm610 )
//-------------------------------------------------

#define rom_cbm610	rom_b128


//-------------------------------------------------
//  ROM( cbm620 )
//-------------------------------------------------

#define rom_cbm620	rom_b256


//-------------------------------------------------
//  ROM( cbm620hu )
//-------------------------------------------------

ROM_START( cbm620hu )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "610.u60", 0x0000, 0x4000, CRC(8eed0d7e) SHA1(9d06c5c3c012204eaaef8b24b1801759b62bf57e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernhun.bin", 0x0000, 0x2000, CRC(0ea8ca4d) SHA1(9977c9f1136ee9c04963e0b50ae0c056efa5663f) )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "charhun.bin", 0x0000, 0x2000, CRC(1fb5e596) SHA1(3254e069f8691b30679b19a9505b6afdfedce6ac) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.bin", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


//-------------------------------------------------
//  ROM( b128hp )
//-------------------------------------------------

ROM_START( b128hp )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901243-02b.u59", 0x0000, 0x2000, CRC(9d0366f9) SHA1(625f7337ea972a8bce2bdf2daababc0ed0b3b69b), ROM_BIOS(1) )
	ROMX_LOAD( "901242-02b.u60", 0x2000, 0x2000, CRC(837978b5) SHA1(56e8d2f86bf73ba36b3d3cb84dd75806b66c530a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901243-04a.u59", 0x0000, 0x2000, CRC(b0dcb56d) SHA1(08d333208060ee2ce84d4532028d94f71c016b96), ROM_BIOS(2) )
	ROMX_LOAD( "901242-04a.u60", 0x2000, 0x2000, CRC(de04ea4f) SHA1(7c6de17d46a3343dc597d9b9519cf63037b31908), ROM_BIOS(2) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(1) )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.bin", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END


//-------------------------------------------------
//  ROM( b256hp )
//-------------------------------------------------

ROM_START( b256hp )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.bin", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END


//-------------------------------------------------
//  ROM( bx256hp )
//-------------------------------------------------

ROM_START( bx256hp )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x1000, "8088", 0)
	ROM_LOAD( "8088.u14", 0x0000, 0x1000, CRC(195e0281) SHA1(ce8acd2a5fb6cbd70d837811d856d656544a1f97) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.bin", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END


//-------------------------------------------------
//  ROM( cbm710 )
//-------------------------------------------------

#define rom_cbm710	rom_b128hp


//-------------------------------------------------
//  ROM( cbm720 )
//-------------------------------------------------

#define rom_cbm720	rom_b256hp


//-------------------------------------------------
//  ROM( cbm730 )
//-------------------------------------------------

#define rom_cbm730	rom_bx256hp


//-------------------------------------------------
//  ROM( cbm720sw )
//-------------------------------------------------

ROM_START( cbm720sw )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "swe-901244-03.u61", 0x0000, 0x2000, CRC(87bc142b) SHA1(fa711f6082741b05a9c80744f5aee68dc8c1dcf4) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901233-03.u25", 0x0000, 0x1000, CRC(09518b19) SHA1(2e28491e31e2c0a3b6db388055216140a637cd09) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.bin", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   INIT                        COMPANY                         FULLNAME                            FLAGS
COMP( 1983,	p500n,		0,		0,		p500_ntsc,	cbm2,	driver_device,		0,		"Commodore Business Machines",	"P500 ~ C128-40 ~ PET-II (NTSC)",	GAME_NOT_WORKING )
COMP( 1983,	p500p,		p500n,	0,		p500_pal,	cbm2,	driver_device,		0,		"Commodore Business Machines",	"P500 ~ C128-40 ~ PET-II (PAL)",	GAME_NOT_WORKING )

COMP( 1983,	b500,		p500n,	0,		b128,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"B500 (NTSC)",						GAME_NOT_WORKING )
COMP( 1983,	b128,		p500n,	0,		b128,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"B128 (NTSC)",						GAME_NOT_WORKING )
COMP( 1983,	b256,		p500n,	0,		b256,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"B256 (NTSC)",						GAME_NOT_WORKING )
COMP( 1983,	cbm610,		p500n,	0,		cbm610,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"CBM 610 (PAL)",					GAME_NOT_WORKING )
COMP( 1983,	cbm620,		p500n,	0,		cbm620,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"CBM 620 (PAL)",					GAME_NOT_WORKING )
COMP( 1983,	cbm620hu,	p500n,	0,		cbm620,		cbm2hu,	driver_device,		0,		"Commodore Business Machines",	"CBM 620 (Hungary)",				GAME_NOT_WORKING )

COMP( 1983,	b128hp,		p500n,	0,		b128hp,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"B128-80HP (NTSC)",					GAME_NOT_WORKING )
COMP( 1983,	b256hp,		p500n,	0,		b256hp,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"B256-80HP (NTSC)",					GAME_NOT_WORKING )
COMP( 1983,	bx256hp,	p500n,	0,		bx256hp,	cbm2,	driver_device,		0,		"Commodore Business Machines",	"BX256-80HP (NTSC)",				GAME_NOT_WORKING )
COMP( 1983,	cbm710,		p500n,	0,		cbm710,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"CBM 710 (PAL)",					GAME_NOT_WORKING )
COMP( 1983,	cbm720,		p500n,	0,		cbm720,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"CBM 720 (PAL)",					GAME_NOT_WORKING )
COMP( 1983,	cbm720sw,	p500n,	0,		cbm720,		cbm2sw,	driver_device,		0,		"Commodore Business Machines",	"CBM 720 (Sweden/Finland)",			GAME_NOT_WORKING )
COMP( 1983,	cbm730,		p500n,	0,		cbm730,		cbm2,	driver_device,		0,		"Commodore Business Machines",	"CBM 730 (PAL)",					GAME_NOT_WORKING )
