// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - 8088 board
    - CIA timers fail in burn-in test
    - cbm620hu charom banking?

*/

#include "includes/cbm2.h"
#include "bus/rs232/rs232.h"
#include "softlist.h"


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

static void cbmb_quick_sethiaddress(address_space &space, UINT16 hiaddress)
{
	space.write_byte(0xf0046, hiaddress & 0xff);
	space.write_byte(0xf0047, hiaddress >> 8);
}

QUICKLOAD_LOAD_MEMBER( cbm2_state, cbmb )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, m_maincpu->space(AS_PROGRAM), 0x10000, cbmb_quick_sethiaddress);
}

QUICKLOAD_LOAD_MEMBER( p500_state, p500 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, m_maincpu->space(AS_PROGRAM), 0, cbmb_quick_sethiaddress);
}

//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read_pla - low profile PLA read
//-------------------------------------------------

void cbm2_state::read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas,
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4)
{
	UINT32 input = P0 << 15 | P1 << 14 | P2 << 13 | P3 << 12 | m_busy2 << 11 | eras << 10 | ecas << 9 | refen << 8 | cas << 7 | ras << 6;
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

void cbm2hp_state::read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas,
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4)
{
	UINT32 input = ras << 13 | cas << 12 | refen << 11 | eras << 10 | ecas << 9 | m_busy2 << 8 | P3 << 3 | P2 << 2 | P1 << 1 | P0;
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

void cbm2_state::bankswitch(offs_t offset, int eras, int ecas, int refen, int cas, int ras, int *sysioen, int *dramen,
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *buframcs, int *extbufcs, int *vidramcs,
	int *diskromcs, int *csbank1, int *csbank2, int *csbank3, int *basiccs, int *knbcs, int *kernalcs,
	int *crtccs, int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs)
{
	int rasseg1 = 1, rasseg2 = 1, rasseg3 = 1, rasseg4 = 1;

	this->read_pla(offset, ras, cas, refen, eras, ecas, casseg1, casseg2, casseg3, casseg4, &rasseg1, &rasseg2, &rasseg3, &rasseg4);

	int decoden = 0;
	*sysioen = !(P0 && P1 && P2 && P3) && m_busen1;
	*dramen = !((!(P0 && P1 && P2 && P3)) && m_busen1);

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
	int eras = 1, ecas = 1, refen = 0, cas = 0, ras = 1, sysioen = 1, dramen = 1;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, buframcs = 1, extbufcs = 1, vidramcs = 1;
	int diskromcs = 1, csbank1 = 1, csbank2 = 1, csbank3 = 1, basiccs = 1, knbcs = 1, kernalcs = 1;
	int crtccs = 1, cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;

	bankswitch(offset, eras, ecas, refen, cas, ras, &sysioen, &dramen,
		&casseg1, &casseg2, &casseg3, &casseg4, &buframcs, &extbufcs, &vidramcs,
		&diskromcs, &csbank1, &csbank2, &csbank3, &basiccs, &knbcs, &kernalcs,
		&crtccs, &cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs);

	UINT8 data = 0xff;

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
		if (!extbufcs && m_extbuf_ram)
		{
			data = m_extbuf_ram[offset & 0x7ff];
		}
		if (!vidramcs)
		{
			data = m_video_ram[offset & 0x7ff];
		}
		if (!basiccs || !knbcs)
		{
			data = m_basic->base()[offset & 0x3fff];
		}
		if (!kernalcs)
		{
			data = m_kernal->base()[offset & 0x1fff];
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
		if (!extprtcs && m_ext_cia)
		{
			data = m_ext_cia->read(space, offset & 0x0f);
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
	int eras = 1, ecas = 1, refen = 0, cas = 0, ras = 1, sysioen = 1, dramen = 1;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, buframcs = 1, extbufcs = 1, vidramcs = 1;
	int diskromcs = 1, csbank1 = 1, csbank2 = 1, csbank3 = 1, basiccs = 1, knbcs = 1, kernalcs = 1;
	int crtccs = 1, cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;

	bankswitch(offset, eras, ecas, refen, cas, ras, &sysioen, &dramen,
		&casseg1, &casseg2, &casseg3, &casseg4, &buframcs, &extbufcs, &vidramcs,
		&diskromcs, &csbank1, &csbank2, &csbank3, &basiccs, &knbcs, &kernalcs,
		&crtccs, &cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs);

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
		if (!extbufcs && m_extbuf_ram)
		{
			m_extbuf_ram[offset & 0x7ff] = data;
		}
		if (!vidramcs)
		{
			m_video_ram[offset & 0x7ff] = data;
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
		if (!extprtcs && m_ext_cia)
		{
			m_ext_cia->write(space, offset & 0x0f, data);
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
//  ext_read -
//-------------------------------------------------

READ8_MEMBER( cbm2_state::ext_read )
{
#ifdef USE_PLA_DECODE
	int ras = 1, cas = 1, refen = 0, eras = 1, ecas = 0;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, rasseg1 = 1, rasseg2 = 1, rasseg3 = 1, rasseg4 = 1;

	this->read_pla(offset, ras, cas, refen, eras, ecas, &casseg1, &casseg2, &casseg3, &casseg4, &rasseg1, &rasseg2, &rasseg3, &rasseg4);
	UINT8 data = 0xff;

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

	return data;
#endif

	UINT8 data = 0;
	if (offset < 0x40000) data = m_ram->pointer()[offset];
	return data;
}


//-------------------------------------------------
//  ext_write -
//-------------------------------------------------

WRITE8_MEMBER( cbm2_state::ext_write )
{
#ifdef USE_PLA_DECODE
	int ras = 1, cas = 1, refen = 0, eras = 1, ecas = 0;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, rasseg1 = 1, rasseg2 = 1, rasseg3 = 1, rasseg4 = 1;

	this->read_pla(offset, ras, cas, refen, eras, ecas, &casseg1, &casseg2, &casseg3, &casseg4, &rasseg1, &rasseg2, &rasseg3, &rasseg4);

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
#endif

	if (offset < 0x40000) m_ram->pointer()[offset] = data;
}


//-------------------------------------------------
//  read_pla1 - P500 PLA #1 read
//-------------------------------------------------

void p500_state::read_pla1(offs_t offset, int busy2, int clrnibcsb, int procvid, int refen, int ba, int aec, int srw,
	int *datxen, int *dramxen, int *clrniben, int *segf, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr)
{
	int sphi2 = m_vic->phi0_r();
	int bras = 1;

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

void p500_state::read_pla2(offs_t offset, offs_t va, int ba, int vicen, int ae, int segf, int bank0,
	int *clrnibcsb, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *procvid, int *viccs, int *vidmatcs)
{
	int sphi2 = m_vic->phi0_r();
	int bcas = 1;

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

void p500_state::bankswitch(offs_t offset, offs_t va, int srw, int ba, int ae, int busy2, int refen,
	int *datxen, int *dramxen, int *clrniben, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr,
	int *clrnibcs, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *viccs, int *vidmatcs,
	int *csbank1, int *csbank2, int *csbank3, int *basiclocs, int *basichics, int *kernalcs,
	int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs, int *aec, int *vsysaden)
{
	int sphi2 = m_vic->phi0_r();
	int sphi1 = !sphi2;
	//int ba = !m_vic->ba_r();
	//int ae = m_vic->aec_r();
	int bcas = 0;

	*aec = !((m_statvid || ae) && sphi2);
	*vsysaden = sphi1 || ba;

	int clrnibcsb = 1, procvid = 1, segf = 1;

	read_pla1(offset, busy2, clrnibcsb, procvid, refen, ba, *aec, srw,
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

	read_pla2(offset, va, ba, vicen, ae, segf, bank0,
		&clrnibcsb, extbufcs, discromcs, buframcs, charomcs, &procvid, viccs, &vidmatcsb);

	*clrnibcs = clrnibcsb || bcas;
	*vidmatcs = vidmatcsb || bcas;

	read_pla1(offset, busy2, clrnibcsb, procvid, refen, ba, *aec, srw,
		datxen, dramxen, clrniben, &segf, _64kcasen, casenb, viddaten, viddat_tr);
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

UINT8 p500_state::read_memory(address_space &space, offs_t offset, offs_t va, int ba, int ae)
{
	int srw = 1, busy2 = 1, refen = 0;

	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(offset, va, srw, ba, ae, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);

	UINT8 data = 0xff;

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
			data = m_basic->base()[offset & 0x3fff];
		}
		if (!kernalcs)
		{
			data = m_kernal->base()[offset & 0x1fff];
		}
		if (!charomcs && !vsysaden && !viddaten && viddat_tr)
		{
			data = m_charom->base()[offset & 0xfff];
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

void p500_state::write_memory(address_space &space, offs_t offset, UINT8 data, int ba, int ae)
{
	int srw = 0, busy2 = 1, refen = 0;
	offs_t va = 0xffff;

	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(offset, va, srw, ba, ae, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);

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
	int ba = 0, ae = 1;
	offs_t va = 0xffff;

	return read_memory(space, offset, va, ba, ae);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( p500_state::write )
{
	int ba = 0, ae = 1;

	write_memory(space, offset, data, ba, ae);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

READ8_MEMBER( p500_state::vic_videoram_r )
{
	int srw = 1, busy2 = 1, refen = 0;
	int ba = !m_vic->ba_r(), ae = m_vic->aec_r();
	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(0, offset, srw, ba, ae, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);

	UINT8 data = 0xff;
//  UINT8 clrnib = 0xf;

	if (vsysaden)
	{
		if (!_64kcasen && !aec && !viddaten && !viddat_tr)
		{
			data = m_ram->pointer()[(m_vicbnksel << 14) | offset];
		}
/*      if (!clrnibcs)
        {
            clrnib = m_color_ram[offset & 0x3ff];
        }*/
		if (!vidmatcs)
		{
			data = m_video_ram[offset & 0x3ff];
		}
		if (!charomcs)
		{
			data = m_charom->base()[offset & 0xfff];
		}
	}

	return data;
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

READ8_MEMBER( p500_state::vic_colorram_r )
{
	int srw = 1, busy2 = 1, refen = 0;
	int ba = !m_vic->ba_r(), ae = m_vic->aec_r();
	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(0, offset, srw, ba, ae, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);

	UINT8 data = 0x0f;

	if (!clrnibcs)
	{
		data = m_color_ram[offset & 0x3ff];
	}

	return data;
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
//  ADDRESS_MAP( ext_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( ext_mem, AS_PROGRAM, 8, cbm2_state )
	AM_RANGE(0x00000, 0xeffff) AM_READWRITE(ext_read, ext_write)
	AM_RANGE(0xf0000, 0xf0fff) AM_MIRROR(0xf000) AM_ROM AM_REGION(EXT_I8088_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( ext_io )
//-------------------------------------------------

static ADDRESS_MAP_START( ext_io, AS_IO, 8, cbm2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0x0001) AM_MIRROR(0x1e) AM_DEVREADWRITE(EXT_I8259A_TAG, pic8259_device, read, write)
	AM_RANGE(0x0020, 0x0027) AM_MIRROR(0x18) AM_DEVREADWRITE(EXT_MOS6525_TAG, tpi6525_device, read, write)
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
	AM_RANGE(0x000, 0x3ff) AM_READ(vic_colorram_r)
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
//  INPUT_PORTS( cbm2_de )
//-------------------------------------------------

static INPUT_PORTS_START( cbm2_de )
	PORT_INCLUDE(cbm2)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cbm2_hu )
//-------------------------------------------------

static INPUT_PORTS_START( cbm2_hu )
	PORT_INCLUDE(cbm2)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cbm2_se )
//------------------------------------------------

static INPUT_PORTS_START( cbm2_se )
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
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( cbm2_state::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = m_video_ram[(ma + column) & 0x7ff];
		offs_t char_rom_addr = (ma & 0x1000) | (m_graphics << 11) | ((code & 0x7f) << 4) | (ra & 0x0f);
		UINT8 data = m_charom->base()[char_rom_addr & 0xfff];

		for (int bit = 0; bit < 9; bit++)
		{
			int color = BIT(data, 7) ^ BIT(code, 7) ^ BIT(ma, 13);
			if (cursor_x == column) color ^= 1;
			color &= de;

			bitmap.pix32(vbp + y, hbp + x++) = pen[color];

			if (bit < 8 || !m_graphics) data <<= 1;
		}
	}
}

//-------------------------------------------------
//  vic2_interface vic_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( p500_state::vic_irq_w )
{
	m_vic_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_vic_irq || m_tpi1_irq || m_user_irq);
}


//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

READ8_MEMBER( cbm2_state::sid_potx_r )
{
	UINT8 data = 0xff;

	switch (m_cia_pa >> 6)
	{
	case 1: data = m_joy1->pot_x_r(); break;
	case 2: data = m_joy2->pot_x_r(); break;
	case 3:
		if (m_joy1->has_pot_x() && m_joy2->has_pot_x())
		{
			data = 1 / (1 / m_joy1->pot_x_r() + 1 / m_joy2->pot_x_r());
		}
		else if (m_joy1->has_pot_x())
		{
			data = m_joy1->pot_x_r();
		}
		else if (m_joy2->has_pot_x())
		{
			data = m_joy2->pot_x_r();
		}
		break;
	}

	return data;
}

READ8_MEMBER( cbm2_state::sid_poty_r )
{
	UINT8 data = 0xff;

	switch (m_cia_pa >> 6)
	{
	case 1: data = m_joy1->pot_y_r(); break;
	case 2: data = m_joy2->pot_y_r(); break;
	case 3:
		if (m_joy1->has_pot_y() && m_joy2->has_pot_y())
		{
			data = 1 / (1 / m_joy1->pot_y_r() + 1 / m_joy2->pot_y_r());
		}
		else if (m_joy1->has_pot_y())
		{
			data = m_joy1->pot_y_r();
		}
		else if (m_joy2->has_pot_y())
		{
			data = m_joy2->pot_y_r();
		}
		break;
	}

	return data;
}


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

	    0       0
	    1       0
	    2       REN
	    3       ATN
	    4       DAV
	    5       EOI
	    6       NDAC
	    7       NRFD

	*/

	UINT8 data = 0;

	// IEEE-488
	data |= m_ieee2->ren_r() << 2;
	data |= m_ieee2->atn_r() << 3;
	data |= m_ieee2->dav_r() << 4;
	data |= m_ieee2->eoi_r() << 5;
	data |= m_ieee2->ndac_r() << 6;
	data |= m_ieee2->nrfd_r() << 7;

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
	m_ieee2->dc_w(BIT(data, 0));

	m_ieee1->te_w(BIT(data, 1));
	m_ieee2->te_w(BIT(data, 1));

	m_ieee2->ren_w(BIT(data, 2));
	m_ieee2->atn_w(BIT(data, 3));
	m_ieee2->dav_w(BIT(data, 4));
	m_ieee2->eoi_w(BIT(data, 5));
	m_ieee2->ndac_w(BIT(data, 6));
	m_ieee2->nrfd_w(BIT(data, 7));
}

READ8_MEMBER( cbm2_state::tpi1_pb_r )
{
	/*

	    bit     description

	    0       IFC
	    1       SRQ
	    2       user port PB2
	    3       user port PB3
	    4
	    5
	    6
	    7       CASS SW

	*/

	UINT8 data = 0;

	// IEEE-488
	data |= m_ieee2->ifc_r();
	data |= m_ieee2->srq_r() << 1;

	// user port
	data |= m_user->pb2_r() << 2;
	data |= m_user->pb3_r() << 3;

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
	    2       user port PB2
	    3       user port PB3
	    4       DRAMON
	    5       CASS WRT
	    6       CASS MTR
	    7

	*/

	// IEEE-488
	m_ieee2->ifc_w(BIT(data, 0));
	m_ieee2->srq_w(BIT(data, 1));

	// user port
	m_user->pb2_w(BIT(data, 2));
	m_user->pb3_w(BIT(data, 3));

	// memory
	m_dramon = BIT(data, 4);
	if (m_busy2) m_busen1 = m_dramon;

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

//-------------------------------------------------
//  tpi6525_interface tpi2_intf
//-------------------------------------------------

UINT8 cbm2_state::read_keyboard()
{
	UINT8 data = 0xff;

	if (!BIT(m_tpi2_pa, 0)) data &= m_pa0->read();
	if (!BIT(m_tpi2_pa, 1)) data &= m_pa1->read();
	if (!BIT(m_tpi2_pa, 2)) data &= m_pa2->read();
	if (!BIT(m_tpi2_pa, 3)) data &= m_pa3->read();
	if (!BIT(m_tpi2_pa, 4)) data &= m_pa4->read();
	if (!BIT(m_tpi2_pa, 5)) data &= m_pa5->read();
	if (!BIT(m_tpi2_pa, 6)) data &= m_pa6->read();
	if (!BIT(m_tpi2_pa, 7)) data &= m_pa7->read();
	if (!BIT(m_tpi2_pb, 0)) data &= m_pb0->read() & m_lock->read();
	if (!BIT(m_tpi2_pb, 1)) data &= m_pb1->read();
	if (!BIT(m_tpi2_pb, 2)) data &= m_pb2->read();
	if (!BIT(m_tpi2_pb, 3)) data &= m_pb3->read();
	if (!BIT(m_tpi2_pb, 4)) data &= m_pb4->read();
	if (!BIT(m_tpi2_pb, 5)) data &= m_pb5->read();
	if (!BIT(m_tpi2_pb, 6)) data &= m_pb6->read();
	if (!BIT(m_tpi2_pb, 7)) data &= m_pb7->read();

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
	    6       0
	    7       0

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

//-------------------------------------------------
//  MOS6526_INTERFACE( cia_intf )
//-------------------------------------------------

READ8_MEMBER( cbm2_state::cia_pa_r )
{
	/*

	    bit     description

	    0       IEEE-488 D0, user port 1D0
	    1       IEEE-488 D1, user port 1D1
	    2       IEEE-488 D2, user port 1D2
	    3       IEEE-488 D3, user port 1D3
	    4       IEEE-488 D4, user port 1D4
	    5       IEEE-488 D5, user port 1D5
	    6       IEEE-488 D6, user port 1D6, LTPN
	    7       IEEE-488 D7, user port 1D7, GAME TRIGGER 24

	*/

	UINT8 data = 0;

	// IEEE-488
	data |= m_ieee1->read(space, 0);

	// user port
	data &= m_user->d1_r(space, 0);

	// joystick
	data &= ~(!BIT(m_joy1->joy_r(), 5) << 6);
	data &= ~(!BIT(m_joy2->joy_r(), 5) << 7);

	return data;
}

WRITE8_MEMBER( cbm2_state::cia_pa_w )
{
	/*

	    bit     description

	    0       IEEE-488 D0, user port 1D0
	    1       IEEE-488 D1, user port 1D1
	    2       IEEE-488 D2, user port 1D2
	    3       IEEE-488 D3, user port 1D3
	    4       IEEE-488 D4, user port 1D4
	    5       IEEE-488 D5, user port 1D5
	    6       IEEE-488 D6, user port 1D6
	    7       IEEE-488 D7, user port 1D7

	*/

	// IEEE-488
	m_ieee1->write(space, 0, data);

	// user port
	m_user->d1_w(space, 0, data);

	// joystick
	m_cia_pa = data;
}

READ8_MEMBER( cbm2_state::cia_pb_r )
{
	/*

	    bit     description

	    0       user port 2D0, GAME10
	    1       user port 2D1, GAME11
	    2       user port 2D2, GAME12
	    3       user port 2D3, GAME13
	    4       user port 2D4, GAME20
	    5       user port 2D5, GAME21
	    6       user port 2D6, GAME22
	    7       user port 2D7, GAME23

	*/

	UINT8 data = 0;

	// joystick
	data |= m_joy1->joy_r() & 0x0f;
	data |= (m_joy2->joy_r() & 0x0f) << 4;

	// user port
	data &= m_user->d2_r(space, 0);

	return data;
}


//-------------------------------------------------
//  tpi6525_interface ext_tpi_intf
//-------------------------------------------------

void cbm2_state::set_busy2(int state)
{
	m_busy2 = state;

	if (m_busy2)
	{
		//m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

		m_busen1 = m_dramon;
	}
	else
	{
		//m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

		m_busen1 = 0;
	}
}

READ8_MEMBER( cbm2_state::ext_tpi_pb_r )
{
	/*

	    bit     description

	    0       _BUSY1
	    1       _BUSY2
	    2       _REQ
	    3       _ACK
	    4       DATA/_CMD
	    5       DIR
	    6       1
	    7       1

	*/

	UINT8 data = 0xc0;

	// _BUSY1
	data |= !m_busen1;

	// _BUSY2
	data |= m_busy2 << 1;

	// CIA
	data |= m_ext_tpi_pb & m_ext_cia_pb & 0x3c;

	return data;
}

WRITE8_MEMBER( cbm2_state::ext_tpi_pb_w )
{
	/*

	    bit     description

	    0
	    1       _BUSY2
	    2
	    3
	    4
	    5
	    6       CIA FLAG
	    7

	*/

	m_ext_tpi_pb = data;

	// _BUSY2
	if (!BIT(data, 1))
	{
		set_busy2(0);
	}

	// FLAG
	m_ext_cia->flag_w(BIT(data, 6));
}

WRITE8_MEMBER( cbm2_state::ext_tpi_pc_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       BSYCLK
	    6
	    7

	*/

	// _BUSY2
	if (BIT(data, 5))
	{
		set_busy2(1);
	}
}

//-------------------------------------------------
//  MOS6526_INTERFACE( ext_cia_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( cbm2_state::ext_cia_irq_w )
{
	m_tpi1->i3_w(!state);
}

READ8_MEMBER( cbm2_state::ext_cia_pb_r )
{
	/*

	    bit     description

	    0       _BUSY1
	    1       _BUSY2
	    2       _REQ
	    3       _ACK
	    4       DATA/_CMD
	    5       DIR
	    6       1
	    7       1

	*/

	UINT8 data = 0xc0;

	// _BUSY1
	data |= !m_busen1;

	// _BUSY2
	data |= m_busy2 << 1;

	// TPI
	data |= m_ext_tpi_pb & m_ext_cia_pb & 0x3c;

	return data;
}

WRITE8_MEMBER( cbm2_state::ext_cia_pb_w )
{
	/*

	    bit     description

	    0
	    1       _BUSY2
	    2
	    3
	    4
	    5
	    6       _INT1
	    7       _INT2

	*/

	m_ext_cia_pb = data;

	// _BUSY2
	if (!BIT(data, 1))
	{
		set_busy2(0);
	}

	if (!BIT(data, 6))
	{
		set_busy2(0);
	}

	m_ext_pic->ir0_w(!BIT(data, 6));
	m_ext_pic->ir7_w(BIT(data, 7));
}


//-------------------------------------------------
//  CBM2_USER_PORT_INTERFACE( user_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( cbm2_state::user_irq_w )
{
	m_user_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_tpi1_irq || m_user_irq);
}


//-------------------------------------------------
//  CBM2_USER_PORT_INTERFACE( p500_user_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( p500_state::user_irq_w )
{
	m_user_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_vic_irq || m_tpi1_irq || m_user_irq);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void cbm2_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_tpi1->i0_w(m_todclk);

	if (m_ext_pic) m_ext_pic->ir2_w(m_todclk);

	m_todclk = !m_todclk;
}


//-------------------------------------------------
//  MACHINE_START( cbm2 )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2 )
{
	// allocate memory
	m_video_ram.allocate(m_video_ram_size);
	m_buffer_ram.allocate(0x800);

	// allocate timer
	int todclk = (m_ntsc ? 60 : 50) * 2;

	m_todclk_timer = timer_alloc();
	m_todclk_timer->adjust(attotime::from_hz(todclk), 0, attotime::from_hz(todclk));

	// state saving
	save_item(NAME(m_dramon));
	save_item(NAME(m_busen1));
	save_item(NAME(m_busy2));
	save_item(NAME(m_graphics));
	save_item(NAME(m_ntsc));
	save_item(NAME(m_todclk));
	save_item(NAME(m_tpi1_irq));
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
//  MACHINE_START( cbm2x_ntsc )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2x_ntsc )
{
	// allocate memory
	m_extbuf_ram.allocate(0x800);

	MACHINE_START_CALL_MEMBER(cbm2_ntsc);
}


//-------------------------------------------------
//  MACHINE_START( cbm2x_pal )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2x_pal )
{
	// allocate memory
	m_extbuf_ram.allocate(0x800);

	MACHINE_START_CALL_MEMBER(cbm2_pal);
}


//-------------------------------------------------
//  MACHINE_START( p500 )
//-------------------------------------------------

MACHINE_START_MEMBER( p500_state, p500 )
{
	m_video_ram_size = 0x400;

	MACHINE_START_CALL_MEMBER(cbm2);

	// allocate memory
	m_color_ram.allocate(0x400);

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


MACHINE_RESET_MEMBER( cbm2_state, cbm2 )
{
	m_dramon = 1;
	m_busen1 = 1;
	m_busy2 = 1;
	m_graphics = 1;
	m_tpi1_irq = CLEAR_LINE;
	m_user_irq = CLEAR_LINE;

m_ext_tpi_pb = 0xff;
m_ext_cia_pb = 0xff;

	m_maincpu->reset();

	if (m_crtc) m_crtc->reset();
	m_sid->reset();
	m_tpi1->reset();
	m_tpi2->reset();
	m_acia->reset();
	m_cia->reset();

	m_ieee->reset();
}


MACHINE_RESET_MEMBER( p500_state, p500 )
{
	MACHINE_RESET_CALL_MEMBER(cbm2);

	m_vic->reset();

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
	MCFG_CPU_ADD(M6509_TAG, M6509, XTAL_14_31818MHz/14)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks
	MCFG_CPU_PROGRAM_MAP(p500_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6509_TAG)

	// video hardware
	MCFG_DEVICE_ADD(MOS6567_TAG, MOS6567, XTAL_14_31818MHz/14)
	MCFG_MOS6566_CPU(M6509_TAG)
	MCFG_MOS6566_IRQ_CALLBACK(WRITELINE(p500_state, vic_irq_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vic_videoram_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_1, vic_colorram_map)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(VIC6567_VRETRACERATE)
	MCFG_SCREEN_SIZE(VIC6567_COLUMNS, VIC6567_LINES)
	MCFG_SCREEN_VISIBLE_AREA(0, VIC6567_VISIBLECOLUMNS - 1, 0, VIC6567_VISIBLELINES - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS6567_TAG, mos6567_device, screen_update)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, MOS6581, XTAL_14_31818MHz/14)
	MCFG_MOS6581_POTX_CALLBACK(READ8(p500_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(p500_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// devices
	MCFG_PLS100_ADD(PLA1_TAG)
	MCFG_PLS100_ADD(PLA2_TAG)
	MCFG_DEVICE_ADD(MOS6525_1_TAG, TPI6525, 0)
	MCFG_TPI6525_OUT_IRQ_CB(WRITELINE(p500_state, tpi1_irq_w))
	MCFG_TPI6525_IN_PA_CB(READ8(cbm2_state, tpi1_pa_r))
	MCFG_TPI6525_OUT_PA_CB(WRITE8(cbm2_state, tpi1_pa_w))
	MCFG_TPI6525_IN_PB_CB(READ8(cbm2_state, tpi1_pb_r))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cbm2_state, tpi1_pb_w))
	MCFG_TPI6525_OUT_CA_CB(WRITELINE(p500_state, tpi1_ca_w))
	MCFG_TPI6525_OUT_CB_CB(WRITELINE(p500_state, tpi1_cb_w))
	MCFG_DEVICE_ADD(MOS6525_2_TAG, TPI6525, 0)
	MCFG_TPI6525_OUT_PA_CB(WRITE8(cbm2_state, tpi2_pa_w))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cbm2_state, tpi2_pb_w))
	MCFG_TPI6525_IN_PC_CB(READ8(p500_state, tpi2_pc_r))
	MCFG_TPI6525_OUT_PC_CB(WRITE8(p500_state, tpi2_pc_w))
	MCFG_DEVICE_ADD(MOS6551A_TAG, MOS6551, VIC6567_CLOCK)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_IRQ_HANDLER(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i4_w))
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_MOS6551_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_MOS6551_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_MOS6551_RXC_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_etc))
	MCFG_DEVICE_ADD(MOS6526_TAG, MOS6526A, XTAL_14_31818MHz/14)
	MCFG_MOS6526_TOD(60)
	MCFG_MOS6526_IRQ_CALLBACK(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i2_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, cnt_w))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, sp_w))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(cbm2_state, cia_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(cbm2_state, cia_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(cbm2_state, cia_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(DEVWRITE8(CBM2_USER_PORT_TAG, cbm2_user_port_device, d2_w))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, pc_w))
	MCFG_DS75160A_ADD(DS75160A_TAG, DEVREAD8(IEEE488_TAG, ieee488_device, dio_r), DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_DEVICE_ADD(DS75161A_TAG, DS75161A, 0)
	MCFG_DS75161A_IN_REN_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ren_r))
	MCFG_DS75161A_IN_IFC_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ifc_r))
	MCFG_DS75161A_IN_NDAC_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ndac_r))
	MCFG_DS75161A_IN_NRFD_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, nrfd_r))
	MCFG_DS75161A_IN_DAV_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, dav_r))
	MCFG_DS75161A_IN_EOI_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, eoi_r))
	MCFG_DS75161A_IN_ATN_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, atn_r))
	MCFG_DS75161A_IN_SRQ_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, srq_r))
	MCFG_DS75161A_OUT_REN_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ren_w))
	MCFG_DS75161A_OUT_IFC_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ifc_w))
	MCFG_DS75161A_OUT_NDAC_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ndac_w))
	MCFG_DS75161A_OUT_NRFD_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, nrfd_w))
	MCFG_DS75161A_OUT_DAV_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, dav_w))
	MCFG_DS75161A_OUT_EOI_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, eoi_w))
	MCFG_DS75161A_OUT_ATN_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, atn_w))
	MCFG_DS75161A_OUT_SRQ_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, srq_w))
	MCFG_CBM_IEEE488_ADD("c8050")
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i1_w))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, nullptr, DEVWRITELINE(MOS6526_TAG, mos6526_device, flag_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, nullptr)
	MCFG_VCS_CONTROL_PORT_TRIGGER_CALLBACK(DEVWRITELINE(MOS6567_TAG, mos6567_device, lp_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, nullptr)
	MCFG_CBM2_EXPANSION_SLOT_ADD(CBM2_EXPANSION_SLOT_TAG, XTAL_14_31818MHz/14, cbm2_expansion_cards, nullptr)
	MCFG_CBM2_USER_PORT_ADD(CBM2_USER_PORT_TAG, cbm2_user_port_cards, nullptr)
	MCFG_CBM2_USER_PORT_IRQ_CALLBACK(WRITELINE(p500_state, user_irq_w))
	MCFG_CBM2_USER_PORT_SP_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, sp_w))
	MCFG_CBM2_USER_PORT_CNT_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, cnt_w))
	MCFG_CBM2_USER_PORT_FLAG_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, flag_w))
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_cts))

	MCFG_QUICKLOAD_ADD("quickload", p500_state, p500, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	// internal ram
	MCFG_FRAGMENT_ADD(128k)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list", "cbm2_cart")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "p500_flop")
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "NTSC")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( p500_pal )
//-------------------------------------------------

static MACHINE_CONFIG_START( p500_pal, p500_state )
	MCFG_MACHINE_START_OVERRIDE(p500_state, p500_pal)
	MCFG_MACHINE_RESET_OVERRIDE(p500_state, p500)

	// basic hardware
	MCFG_CPU_ADD(M6509_TAG, M6509, XTAL_17_734472MHz/18)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks
	MCFG_CPU_PROGRAM_MAP(p500_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6509_TAG)

	// video hardware
	MCFG_DEVICE_ADD(MOS6569_TAG, MOS6569, XTAL_17_734472MHz/18)
	MCFG_MOS6566_CPU(M6509_TAG)
	MCFG_MOS6566_IRQ_CALLBACK(WRITELINE(p500_state, vic_irq_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vic_videoram_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_1, vic_colorram_map)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(VIC6569_VRETRACERATE)
	MCFG_SCREEN_SIZE(VIC6569_COLUMNS, VIC6569_LINES)
	MCFG_SCREEN_VISIBLE_AREA(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS6569_TAG, mos6569_device, screen_update)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, MOS6581, XTAL_17_734472MHz/18)
	MCFG_MOS6581_POTX_CALLBACK(READ8(p500_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(p500_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// devices
	MCFG_PLS100_ADD(PLA1_TAG)
	MCFG_PLS100_ADD(PLA2_TAG)
	MCFG_DEVICE_ADD(MOS6525_1_TAG, TPI6525, 0)
	MCFG_TPI6525_OUT_IRQ_CB(WRITELINE(p500_state, tpi1_irq_w))
	MCFG_TPI6525_IN_PA_CB(READ8(cbm2_state, tpi1_pa_r))
	MCFG_TPI6525_OUT_PA_CB(WRITE8(cbm2_state, tpi1_pa_w))
	MCFG_TPI6525_IN_PB_CB(READ8(cbm2_state, tpi1_pb_r))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cbm2_state, tpi1_pb_w))
	MCFG_TPI6525_OUT_CA_CB(WRITELINE(p500_state, tpi1_ca_w))
	MCFG_TPI6525_OUT_CB_CB(WRITELINE(p500_state, tpi1_cb_w))
	MCFG_DEVICE_ADD(MOS6525_2_TAG, TPI6525, 0)
	MCFG_TPI6525_OUT_PA_CB(WRITE8(cbm2_state, tpi2_pa_w))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cbm2_state, tpi2_pb_w))
	MCFG_TPI6525_IN_PC_CB(READ8(p500_state, tpi2_pc_r))
	MCFG_TPI6525_OUT_PC_CB(WRITE8(p500_state, tpi2_pc_w))
	MCFG_DEVICE_ADD(MOS6551A_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_IRQ_HANDLER(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i4_w))
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_DEVICE_ADD(MOS6526_TAG, MOS6526A, XTAL_17_734472MHz/18)
	MCFG_MOS6526_TOD(50)
	MCFG_MOS6526_IRQ_CALLBACK(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i2_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, cnt_w))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, sp_w))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(cbm2_state, cia_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(cbm2_state, cia_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(cbm2_state, cia_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(DEVWRITE8(CBM2_USER_PORT_TAG, cbm2_user_port_device, d2_w))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, pc_w))
	MCFG_DS75160A_ADD(DS75160A_TAG, DEVREAD8(IEEE488_TAG, ieee488_device, dio_r), DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_DEVICE_ADD(DS75161A_TAG, DS75161A, 0)
	MCFG_DS75161A_IN_REN_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ren_r))
	MCFG_DS75161A_IN_IFC_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ifc_r))
	MCFG_DS75161A_IN_NDAC_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ndac_r))
	MCFG_DS75161A_IN_NRFD_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, nrfd_r))
	MCFG_DS75161A_IN_DAV_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, dav_r))
	MCFG_DS75161A_IN_EOI_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, eoi_r))
	MCFG_DS75161A_IN_ATN_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, atn_r))
	MCFG_DS75161A_IN_SRQ_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, srq_r))
	MCFG_DS75161A_OUT_REN_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ren_w))
	MCFG_DS75161A_OUT_IFC_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ifc_w))
	MCFG_DS75161A_OUT_NDAC_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ndac_w))
	MCFG_DS75161A_OUT_NRFD_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, nrfd_w))
	MCFG_DS75161A_OUT_DAV_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, dav_w))
	MCFG_DS75161A_OUT_EOI_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, eoi_w))
	MCFG_DS75161A_OUT_ATN_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, atn_w))
	MCFG_DS75161A_OUT_SRQ_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, srq_w))
	MCFG_CBM_IEEE488_ADD("c8050")
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i1_w))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, nullptr, DEVWRITELINE(MOS6526_TAG, mos6526_device, flag_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, nullptr)
	MCFG_VCS_CONTROL_PORT_TRIGGER_CALLBACK(DEVWRITELINE(MOS6569_TAG, mos6569_device, lp_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, nullptr)
	MCFG_CBM2_EXPANSION_SLOT_ADD(CBM2_EXPANSION_SLOT_TAG, XTAL_17_734472MHz/18, cbm2_expansion_cards, nullptr)
	MCFG_CBM2_USER_PORT_ADD(CBM2_USER_PORT_TAG, cbm2_user_port_cards, nullptr)
	MCFG_CBM2_USER_PORT_IRQ_CALLBACK(WRITELINE(p500_state, user_irq_w))
	MCFG_CBM2_USER_PORT_SP_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, sp_w))
	MCFG_CBM2_USER_PORT_CNT_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, cnt_w))
	MCFG_CBM2_USER_PORT_FLAG_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, flag_w))
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_cts))

	MCFG_QUICKLOAD_ADD("quickload", p500_state, p500, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	// internal ram
	MCFG_FRAGMENT_ADD(128k)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list", "cbm2_cart")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "p500_flop")
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "PAL")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm2lp_ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( cbm2lp_ntsc, cbm2_state )
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_ntsc)
	MCFG_MACHINE_RESET_OVERRIDE(cbm2_state, cbm2)

	// basic hardware
	MCFG_CPU_ADD(M6509_TAG, M6509, XTAL_18MHz/9)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks
	MCFG_CPU_PROGRAM_MAP(cbm2_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6509_TAG)

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_UPDATE_DEVICE(MC68B45_TAG, mc6845_device, screen_update)

	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(768, 312)
	MCFG_SCREEN_VISIBLE_AREA(0, 768-1, 0, 312-1)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_MC6845_ADD(MC68B45_TAG, MC6845, SCREEN_TAG, XTAL_18MHz/9)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(9)
	MCFG_MC6845_UPDATE_ROW_CB(cbm2_state, crtc_update_row)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, MOS6581, XTAL_18MHz/9)
	MCFG_MOS6581_POTX_CALLBACK(READ8(cbm2_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(cbm2_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// devices
	MCFG_PLS100_ADD(PLA1_TAG)
	MCFG_DEVICE_ADD(MOS6525_1_TAG, TPI6525, 0)
	MCFG_TPI6525_OUT_IRQ_CB(WRITELINE(cbm2_state, tpi1_irq_w))
	MCFG_TPI6525_IN_PA_CB(READ8(cbm2_state, tpi1_pa_r))
	MCFG_TPI6525_OUT_PA_CB(WRITE8(cbm2_state, tpi1_pa_w))
	MCFG_TPI6525_IN_PA_CB(READ8(cbm2_state, tpi1_pb_r))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cbm2_state, tpi1_pb_w))
	MCFG_TPI6525_OUT_CA_CB(WRITELINE(cbm2_state, tpi1_ca_w))
	MCFG_DEVICE_ADD(MOS6525_2_TAG, TPI6525, 0)
	MCFG_TPI6525_OUT_PA_CB(WRITE8(cbm2_state, tpi2_pa_w))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cbm2_state, tpi2_pb_w))
	MCFG_TPI6525_IN_PC_CB(READ8(cbm2_state, tpi2_pc_r))
	MCFG_DEVICE_ADD(MOS6551A_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_IRQ_HANDLER(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i4_w))
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_DEVICE_ADD(MOS6526_TAG, MOS6526A, XTAL_18MHz/9)
	MCFG_MOS6526_TOD(60)
	MCFG_MOS6526_IRQ_CALLBACK(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i2_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, cnt_w))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, sp_w))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(cbm2_state, cia_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(cbm2_state, cia_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(cbm2_state, cia_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(DEVWRITE8(CBM2_USER_PORT_TAG, cbm2_user_port_device, d2_w))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE(CBM2_USER_PORT_TAG, cbm2_user_port_device, pc_w))
	MCFG_DS75160A_ADD(DS75160A_TAG, DEVREAD8(IEEE488_TAG, ieee488_device, dio_r), DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_DEVICE_ADD(DS75161A_TAG, DS75161A, 0)
	MCFG_DS75161A_IN_REN_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ren_r))
	MCFG_DS75161A_IN_IFC_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ifc_r))
	MCFG_DS75161A_IN_NDAC_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, ndac_r))
	MCFG_DS75161A_IN_NRFD_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, nrfd_r))
	MCFG_DS75161A_IN_DAV_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, dav_r))
	MCFG_DS75161A_IN_EOI_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, eoi_r))
	MCFG_DS75161A_IN_ATN_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, atn_r))
	MCFG_DS75161A_IN_SRQ_CB(DEVREADLINE(IEEE488_TAG, ieee488_device, srq_r))
	MCFG_DS75161A_OUT_REN_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ren_w))
	MCFG_DS75161A_OUT_IFC_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ifc_w))
	MCFG_DS75161A_OUT_NDAC_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, ndac_w))
	MCFG_DS75161A_OUT_NRFD_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, nrfd_w))
	MCFG_DS75161A_OUT_DAV_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, dav_w))
	MCFG_DS75161A_OUT_EOI_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, eoi_w))
	MCFG_DS75161A_OUT_ATN_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, atn_w))
	MCFG_DS75161A_OUT_SRQ_CB(DEVWRITELINE(IEEE488_TAG, ieee488_device, srq_w))
	MCFG_CBM_IEEE488_ADD("c8050")
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE(MOS6525_1_TAG, tpi6525_device, i1_w))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, nullptr,DEVWRITELINE(MOS6526_TAG, mos6526_device, flag_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, nullptr)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, nullptr)
	MCFG_CBM2_EXPANSION_SLOT_ADD(CBM2_EXPANSION_SLOT_TAG, XTAL_18MHz/9, cbm2_expansion_cards, nullptr)
	MCFG_CBM2_USER_PORT_ADD(CBM2_USER_PORT_TAG, cbm2_user_port_cards, nullptr)
	MCFG_CBM2_USER_PORT_IRQ_CALLBACK(WRITELINE(cbm2_state, user_irq_w))
	MCFG_CBM2_USER_PORT_SP_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, sp_w))
	MCFG_CBM2_USER_PORT_CNT_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, cnt_w))
	MCFG_CBM2_USER_PORT_FLAG_CALLBACK(DEVWRITELINE(MOS6526_TAG, mos6526_device, flag_w))
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(MOS6551A_TAG, mos6551_device, write_cts))

	MCFG_QUICKLOAD_ADD("quickload", cbm2_state, cbmb, "p00,prg,t64", CBM_QUICKLOAD_DELAY_SECONDS)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list", "cbm2_cart")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "cbm2_flop")
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "NTSC")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( b128 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( b128, cbm2lp_ntsc )
	MCFG_FRAGMENT_ADD(128k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( b256 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( b256, cbm2lp_ntsc )
	MCFG_FRAGMENT_ADD(256k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm2lp_pal )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm2lp_pal, cbm2lp_ntsc )
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_pal)

	MCFG_DEVICE_MODIFY(MOS6526_TAG)
	MCFG_MOS6526_TOD(50)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm610 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm610, cbm2lp_pal )
	MCFG_FRAGMENT_ADD(128k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm620 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm620, cbm2lp_pal )
	MCFG_FRAGMENT_ADD(256k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm2hp_ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( cbm2hp_ntsc, cbm2lp_ntsc, cbm2hp_state )
	MCFG_DEVICE_MODIFY(MOS6525_2_TAG)
	MCFG_TPI6525_IN_PC_CB(READ8(cbm2hp_state, tpi2_pc_r))
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( b128hp )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( b128hp, cbm2hp_ntsc )
	MCFG_FRAGMENT_ADD(128k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( b256hp )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( b256hp, cbm2hp_ntsc )
	MCFG_FRAGMENT_ADD(256k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( bx256hp )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( bx256hp, b256hp )
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2x_ntsc)

	MCFG_CPU_ADD(EXT_I8088_TAG, I8088, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(ext_mem)
	MCFG_CPU_IO_MAP(ext_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(EXT_I8259A_TAG, pic8259_device, inta_cb)

	MCFG_PIC8259_ADD(EXT_I8259A_TAG, INPUTLINE(EXT_I8088_TAG, INPUT_LINE_IRQ0), VCC, NULL)
	MCFG_DEVICE_ADD(EXT_MOS6525_TAG, TPI6525, 0)
	MCFG_TPI6525_IN_PA_CB(DEVREAD8(EXT_MOS6526_TAG, mos6526_device, pa_r))
	MCFG_TPI6525_IN_PB_CB(READ8(cbm2_state, ext_tpi_pb_r))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cbm2_state, ext_tpi_pb_w))
	MCFG_TPI6525_OUT_PC_CB(WRITE8(cbm2_state, ext_tpi_pc_w))
	MCFG_DEVICE_ADD(EXT_MOS6526_TAG, MOS6526, XTAL_18MHz/9)
	MCFG_MOS6526_TOD(60)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(cbm2_state, ext_cia_irq_w))
	MCFG_MOS6526_PA_INPUT_CALLBACK(DEVREAD8(EXT_MOS6525_TAG, tpi6525_device, pa_r))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(cbm2_state, ext_cia_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(cbm2_state, ext_cia_pb_w))

	MCFG_SOFTWARE_LIST_ADD("flop_list2", "bx256hp_flop")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm2hp_pal )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm2hp_pal, cbm2hp_ntsc )
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_pal)

	// devices
	MCFG_DEVICE_MODIFY(MOS6525_2_TAG)
	MCFG_TPI6525_IN_PC_CB(READ8(cbm2hp_state, tpi2_pc_r))

	MCFG_DEVICE_MODIFY(MOS6526_TAG)
	MCFG_MOS6526_TOD(50)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm710 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm710, cbm2hp_pal )
	MCFG_FRAGMENT_ADD(128k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm720 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm720, cbm2hp_pal )
	MCFG_FRAGMENT_ADD(256k)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( cbm730 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( cbm730, cbm720 )
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2x_pal)

	MCFG_CPU_ADD(EXT_I8088_TAG, I8088, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(ext_mem)
	MCFG_CPU_IO_MAP(ext_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(EXT_I8259A_TAG, pic8259_device, inta_cb)

	MCFG_PIC8259_ADD(EXT_I8259A_TAG, INPUTLINE(EXT_I8088_TAG, INPUT_LINE_IRQ0), VCC, NULL)
	MCFG_DEVICE_ADD(EXT_MOS6525_TAG, TPI6525, 0)
	MCFG_TPI6525_IN_PA_CB(DEVREAD8(EXT_MOS6526_TAG, mos6526_device, pa_r))
	MCFG_TPI6525_IN_PB_CB(READ8(cbm2_state, ext_tpi_pb_r))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cbm2_state, ext_tpi_pb_w))
	MCFG_TPI6525_OUT_PC_CB(WRITE8(cbm2_state, ext_tpi_pc_w))
	MCFG_DEVICE_ADD(EXT_MOS6526_TAG, MOS6526, XTAL_18MHz/9)
	MCFG_MOS6526_TOD(50)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(cbm2_state, ext_cia_irq_w))
	MCFG_MOS6526_PA_INPUT_CALLBACK(DEVREAD8(EXT_MOS6525_TAG, tpi6525_device, pa_r))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(cbm2_state, ext_cia_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(cbm2_state, ext_cia_pb_w))

	MCFG_SOFTWARE_LIST_ADD("flop_list2", "bx256hp_flop")
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

#define rom_p500p   rom_p500


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
	ROM_LOAD( "906114-04.u18", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
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
	ROM_LOAD( "906114-04.u18", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END

#define rom_cbm610  rom_b128
#define rom_cbm620  rom_b256


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
	ROM_LOAD( "906114-04.u18", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


//-------------------------------------------------
//  ROM( cbm620_hu )
//-------------------------------------------------

ROM_START( cbm620_hu )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "610.u60", 0x0000, 0x4000, CRC(8eed0d7e) SHA1(9d06c5c3c012204eaaef8b24b1801759b62bf57e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernhun.u61", 0x0000, 0x2000, CRC(0ea8ca4d) SHA1(9977c9f1136ee9c04963e0b50ae0c056efa5663f) )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "charhun.u25", 0x0000, 0x2000, CRC(1fb5e596) SHA1(3254e069f8691b30679b19a9505b6afdfedce6ac) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.u18", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
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
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

#define rom_cbm710  rom_b128hp


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
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

#define rom_cbm720  rom_b256hp


//-------------------------------------------------
//  ROM( bx256hp )
//-------------------------------------------------

ROM_START( bx256hp )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x1000, EXT_I8088_TAG, 0)
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
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

#define rom_cbm730  rom_bx256hp


//-------------------------------------------------
//  ROM( cbm720_de )
//-------------------------------------------------

ROM_START( cbm720_de )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "324866-03a.u61", 0x0000, 0x2000, CRC(554b008d) SHA1(1483a46924308d86f4c7f9cb71c34851c510fcf4) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "324867-02.u25", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END


//-------------------------------------------------
//  ROM( cbm720_se )
//-------------------------------------------------

ROM_START( cbm720_se )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "swe-901244-03.u61", 0x0000, 0x2000, CRC(87bc142b) SHA1(fa711f6082741b05a9c80744f5aee68dc8c1dcf4) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901233-03.u25", 0x0000, 0x1000, CRC(09518b19) SHA1(2e28491e31e2c0a3b6db388055216140a637cd09) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT                        COMPANY                         FULLNAME                    FLAGS
COMP( 1983, p500,       0,      0,      p500_ntsc,  cbm2,       driver_device,      0,      "Commodore Business Machines",  "P500 (NTSC)",              MACHINE_SUPPORTS_SAVE )
COMP( 1983, p500p,      p500,   0,      p500_pal,   cbm2,       driver_device,      0,      "Commodore Business Machines",  "P500 (PAL)",               MACHINE_SUPPORTS_SAVE )
COMP( 1983, b500,       0,      0,      b128,       cbm2,       driver_device,      0,      "Commodore Business Machines",  "B500",                     MACHINE_SUPPORTS_SAVE )
COMP( 1983, b128,       b500,   0,      b128,       cbm2,       driver_device,      0,      "Commodore Business Machines",  "B128",                     MACHINE_SUPPORTS_SAVE )
COMP( 1983, b256,       b500,   0,      b256,       cbm2,       driver_device,      0,      "Commodore Business Machines",  "B256",                     MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm610,     b500,   0,      cbm610,     cbm2,       driver_device,      0,      "Commodore Business Machines",  "CBM 610",                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm620,     b500,   0,      cbm620,     cbm2,       driver_device,      0,      "Commodore Business Machines",  "CBM 620",                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm620_hu,  b500,   0,      cbm620,     cbm2_hu,    driver_device,      0,      "Commodore Business Machines",  "CBM 620 (Hungary)",        MACHINE_SUPPORTS_SAVE )
COMP( 1983, b128hp,     0,      0,      b128hp,     cbm2,       driver_device,      0,      "Commodore Business Machines",  "B128-80HP",                MACHINE_SUPPORTS_SAVE )
COMP( 1983, b256hp,     b128hp, 0,      b256hp,     cbm2,       driver_device,      0,      "Commodore Business Machines",  "B256-80HP",                MACHINE_SUPPORTS_SAVE )
COMP( 1983, bx256hp,    b128hp, 0,      bx256hp,    cbm2,       driver_device,      0,      "Commodore Business Machines",  "BX256-80HP",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // 8088 co-processor is missing
COMP( 1983, cbm710,     b128hp, 0,      cbm710,     cbm2,       driver_device,      0,      "Commodore Business Machines",  "CBM 710",                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm720,     b128hp, 0,      cbm720,     cbm2,       driver_device,      0,      "Commodore Business Machines",  "CBM 720",                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm720_de,  b128hp, 0,      cbm720,     cbm2_de,    driver_device,      0,      "Commodore Business Machines",  "CBM 720 (Germany)",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm720_se,  b128hp, 0,      cbm720,     cbm2_se,    driver_device,      0,      "Commodore Business Machines",  "CBM 720 (Sweden/Finland)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm730,     b128hp, 0,      cbm730,     cbm2,       driver_device,      0,      "Commodore Business Machines",  "CBM 730",                  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // 8088 co-processor is missing
