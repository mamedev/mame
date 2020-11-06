// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - connect CAPS LOCK to charom A12 on international variants

*/

#include "emu.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"
#include "bus/c64/exp.h"
#include "bus/cbmiec/cbmiec.h"
#include "bus/cbmiec/c1571.h"
#include "bus/cbmiec/c1581.h"
#include "bus/vic20/user.h"
#include "bus/pet/cass.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "imagedev/snapquik.h"
#include "cpu/m6502/m8502.h"
#include "cpu/z80/z80.h"
#include "machine/cbm_snqk.h"
#include "machine/input_merger.h"
#include "machine/mos6526.h"
#include "machine/mos8722.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "sound/mos6581.h"
#include "video/mc6845.h"
#include "video/mos6566.h"

#define M8502_TAG       "u6"
#define MOS8563_TAG     "u22"
#define MOS8564_TAG     "u21"
#define MOS8566_TAG     "u21"
#define MOS8721_TAG     "u11"
#define SCREEN_VIC_TAG  "screen"
#define SCREEN_VDC_TAG  "screen80"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"

class c128_state : public driver_device
{
public:
	c128_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "u10"),
		m_subcpu(*this, M8502_TAG),
		m_nmi(*this, "nmi"),
		m_mmu(*this, "u7"),
		m_pla(*this, MOS8721_TAG),
		m_vdc(*this, MOS8563_TAG),
		m_vic(*this, MOS8564_TAG),
		m_sid(*this, "u5"),
		m_cia1(*this, "u1"),
		m_cia2(*this, "u4"),
		m_iec(*this, CBM_IEC_TAG),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_exp(*this, "exp"),
		m_user(*this, "user"),
		m_ram(*this, RAM_TAG),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_from(*this, "from"),
		m_rom(*this, M8502_TAG),
		m_charom(*this, "charom"),
		m_color_ram(*this, "color_ram", 0x800, ENDIANNESS_LITTLE),
		m_row(*this, "ROW%u", 0),
		m_k(*this, "K%u", 0),
		m_lock(*this, "LOCK"),
		m_caps(*this, "CAPS"),
		m_40_80(*this, "40_80"),
		m_z80en(0),
		m_loram(1),
		m_hiram(1),
		m_charen(1),
		m_game(1),
		m_exrom(1),
		m_va14(1),
		m_va15(1),
		m_clrbank(0),
		m_cnt1(1),
		m_sp1(1),
		m_iec_data_out(1),
		m_cass_rd(1),
		m_iec_srq(1),
		m_vic_k(0x07),
		m_caps_lock(1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<m8502_device> m_subcpu;
	required_device<input_merger_device> m_nmi;
	required_device<mos8722_device> m_mmu;
	required_device<pla_device> m_pla;
	required_device<mos8563_device> m_vdc;
	required_device<mos6566_device> m_vic;
	required_device<mos6581_device> m_sid;
	required_device<mos6526_device> m_cia1;
	required_device<mos6526_device> m_cia2;
	required_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<c64_expansion_slot_device> m_exp;
	required_device<pet_user_port_device> m_user;
	required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<generic_slot_device> m_from;
	required_memory_region m_rom;
	required_memory_region m_charom;
	memory_share_creator<uint8_t> m_color_ram;
	required_ioport_array<8> m_row;
	required_ioport_array<3> m_k;
	required_ioport m_lock;
	required_ioport m_caps;
	required_ioport m_40_80;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	inline void check_interrupts();
	int read_pla(offs_t offset, offs_t ca, offs_t vma, int ba, int rw, int aec, int z80io, int ms3, int ms2, int ms1, int ms0);
	uint8_t read_memory(offs_t offset, offs_t vma, int ba, int aec, int z80io);
	void write_memory(offs_t offset, offs_t vma, uint8_t data, int ba, int aec, int z80io);
	inline void update_iec();

	uint8_t z80_r(offs_t offset);
	void z80_w(offs_t offset, uint8_t data);
	uint8_t z80_io_r(offs_t offset);
	void z80_io_w(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t vic_videoram_r(offs_t offset);
	uint8_t vic_colorram_r(offs_t offset);

	DECLARE_WRITE_LINE_MEMBER( mmu_z80en_w );
	DECLARE_WRITE_LINE_MEMBER( mmu_fsdir_w );
	DECLARE_READ_LINE_MEMBER( mmu_game_r );
	DECLARE_READ_LINE_MEMBER( mmu_exrom_r );
	DECLARE_READ_LINE_MEMBER( mmu_sense40_r );

	void vic_k_w(uint8_t data);

	uint8_t sid_potx_r();
	uint8_t sid_poty_r();

	DECLARE_WRITE_LINE_MEMBER( cia1_cnt_w );
	DECLARE_WRITE_LINE_MEMBER( cia1_sp_w );
	uint8_t cia1_pa_r();
	void cia1_pa_w(uint8_t data);
	uint8_t cia1_pb_r();
	void cia1_pb_w(uint8_t data);

	uint8_t cia2_pa_r();
	void cia2_pa_w(uint8_t data);

	uint8_t cpu_r();
	void cpu_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( iec_srq_w );
	DECLARE_WRITE_LINE_MEMBER( iec_data_w );

	uint8_t exp_dma_cd_r(offs_t offset);
	void exp_dma_cd_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER( exp_dma_w );
	DECLARE_WRITE_LINE_MEMBER( exp_reset_w );

	DECLARE_WRITE_LINE_MEMBER( write_restore );
	DECLARE_INPUT_CHANGED_MEMBER( caps_lock );

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_c128);

	uint8_t cia2_pb_r();
	void cia2_pb_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( write_user_pa2 ) { m_user_pa2 = state; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb0 ) { if (state) m_user_pb |= 1; else m_user_pb &= ~1; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb1 ) { if (state) m_user_pb |= 2; else m_user_pb &= ~2; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb2 ) { if (state) m_user_pb |= 4; else m_user_pb &= ~4; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb3 ) { if (state) m_user_pb |= 8; else m_user_pb &= ~8; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb4 ) { if (state) m_user_pb |= 16; else m_user_pb &= ~16; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb5 ) { if (state) m_user_pb |= 32; else m_user_pb &= ~32; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb6 ) { if (state) m_user_pb |= 64; else m_user_pb &= ~64; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb7 ) { if (state) m_user_pb |= 128; else m_user_pb &= ~128; }

	// memory state
	int m_z80en;
	int m_loram;
	int m_hiram;
	int m_charen;
	int m_game;
	int m_exrom;
	int m_reset;

	// video state
	int m_va14;
	int m_va15;
	int m_clrbank;

	// fast serial state
	int m_cnt1;
	int m_sp1;
	int m_iec_data_out;

	// interrupt state
	int m_exp_dma;
	int m_cass_rd;
	int m_iec_srq;

	// keyboard state
	uint8_t m_vic_k;
	int m_caps_lock;

	int m_user_pa2;
	int m_user_pb;
	void pal(machine_config &config);
	void ntsc(machine_config &config);
	void c128pal(machine_config &config);
	void c128(machine_config &config);
	void c128dcr(machine_config &config);
	void c128dcrp(machine_config &config);
	void c128d81(machine_config &config);
	void m8502_mem(address_map &map);
	void vdc_videoram_map(address_map &map);
	void vic_colorram_map(address_map &map);
	void vic_videoram_map(address_map &map);
	void z80_io(address_map &map);
	void z80_mem(address_map &map);
};



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

enum
{
	PLA_OUT_SDEN = 0,
	PLA_OUT_ROM4 = 1,
	PLA_OUT_ROM2 = 2,
	PLA_OUT_DIR = 3,
	PLA_OUT_ROML = 4,
	PLA_OUT_ROMH = 5,
	PLA_OUT_CLRBANK = 6,
	PLA_OUT_FROM1 = 7,
	PLA_OUT_ROM3 = 8,
	PLA_OUT_ROM1 = 9,
	PLA_OUT_IOCS = 10,
	PLA_OUT_DWE = 11,
	PLA_OUT_CASENB = 12,
	PLA_OUT_VIC = 13,
	PLA_OUT_IOACC = 14,
	PLA_OUT_GWE = 15,
	PLA_OUT_COLORRAM = 16,
	PLA_OUT_CHAROM = 17
};


QUICKLOAD_LOAD_MEMBER(c128_state::quickload_c128)
{
	return general_cbm_loadsnap(image, file_type, quickload_size, m_maincpu->space(AS_PROGRAM), 0, cbm_quick_sethiaddress);
}


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

inline void c128_state::check_interrupts()
{
	//int irq = m_cia1_irq || m_vic_irq || m_exp_irq;
	//int nmi = m_cia2_irq || !m_restore || m_exp_nmi;
	//int aec = m_exp_dma && m_z80_busack;
	//int rdy = m_vic_aec && m_z80en && m_vic_ba;
	//int busreq = !m_z80en || !(m_z80_busack && !aec)
}



//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read_pla -
//-------------------------------------------------

int c128_state::read_pla(offs_t offset, offs_t ca, offs_t vma, int ba, int rw, int aec, int z80io, int ms3, int ms2, int ms1, int ms0)
{
	int _128_256 = 1;
	int dmaack = 1;
	int vicfix = 1;
	int sphi2 = m_vic->phi0_r();

	m_game = m_exp->game_r(ca, sphi2, ba, rw, m_loram, m_hiram);
	m_exrom = m_exp->exrom_r(ca, sphi2, ba, rw, m_loram, m_hiram);

	uint32_t input = sphi2 << 26 | m_va14 << 25 | m_charen << 24 |
		m_hiram << 23 | m_loram << 22 | ba << 21 | VMA5 << 20 | VMA4 << 19 | ms0 << 18 | ms1 << 17 | ms2 << 16 |
		m_exrom << 15 | m_game << 14 | rw << 13 | aec << 12 | A10 << 11 | A11 << 10 | A12 << 9 | A13 << 8 |
		A14 << 7 | A15 << 6 | z80io << 5 | m_z80en << 4 | ms3 << 3 | vicfix << 2 | dmaack << 1 | _128_256;

	return m_pla->read(input);
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

uint8_t c128_state::read_memory(offs_t offset, offs_t vma, int ba, int aec, int z80io)
{
	int rw = 1, ms0 = 1, ms1 = 1, ms2 = 1, ms3 = 1, cas0 = 1, cas1 = 1;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	uint8_t data = 0xff;

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

	int plaout = read_pla(offset, ca, vma, ba, rw, aec, z80io, ms3, ms2, ms1, ms0);

	m_clrbank = BIT(plaout, PLA_OUT_CLRBANK);

	if (!BIT(plaout, PLA_OUT_CASENB))
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
	if (!BIT(plaout, PLA_OUT_ROM1))
	{
		// CR: data = m_rom1[(ms3 << 14) | ((BIT(ta, 14) && BIT(offset, 13)) << 13) | (ta & 0x1000) | (offset & 0xfff)];
		data = m_rom->base()[((BIT(ta, 14) && BIT(offset, 13)) << 13) | (ta & 0x1000) | (offset & 0xfff)];
	}
	if (!BIT(plaout, PLA_OUT_ROM2))
	{
		data = m_rom->base()[0x4000 | (offset & 0x3fff)];
	}
	if (!BIT(plaout, PLA_OUT_ROM3))
	{
		// CR: data = m_rom3[(BIT(offset, 15) << 14) | (offset & 0x3fff)];
		data = m_rom->base()[0x8000 | (offset & 0x3fff)];
	}
	if (!BIT(plaout, PLA_OUT_ROM4))
	{
		data = m_rom->base()[0xc000 | (ta & 0x1000) | (offset & 0x2fff)];
	}
	if (!BIT(plaout, PLA_OUT_CHAROM))
	{
		data = m_charom->base()[(ms3 << 12) | (ta & 0xf00) | sa];
	}
	if (!BIT(plaout, PLA_OUT_COLORRAM) && aec)
	{
		data = m_color_ram[(m_clrbank << 10) | (ta & 0x300) | sa] & 0x0f;
	}
	if (!BIT(plaout, PLA_OUT_VIC))
	{
		data = m_vic->read(offset & 0x3f);
	}
	if (!BIT(plaout, PLA_OUT_FROM1) && m_from->exists())
	{
		data = m_from->read_rom(offset & 0x7fff);
	}
	if (!BIT(plaout, PLA_OUT_IOCS) && BIT(offset, 10))
	{
		switch ((BIT(offset, 11) << 2) | ((offset >> 8) & 0x03))
		{
		case 0: // SID
			data = m_sid->read(offset & 0x1f);
			break;

		case 2: // CS8563
			if (BIT(offset, 0))
			{
				data = m_vdc->register_r();
			}
			else
			{
				data = m_vdc->status_r();
			}
			break;

		case 4: // CIA1
			data = m_cia1->read(offset & 0x0f);
			break;

		case 5: // CIA2
			data = m_cia2->read(offset & 0x0f);
			break;

		case 6: // I/O1
			io1 = 0;
			break;

		case 7: // I/O2
			io2 = 0;
			break;
		}
	}

	int roml = BIT(plaout, PLA_OUT_ROML);
	int romh = BIT(plaout, PLA_OUT_ROMH);

	data = m_exp->cd_r(ca, data, sphi2, ba, roml, romh, io1, io2);

	return m_mmu->read(offset, data);
}


//-------------------------------------------------
//  write_memory -
//-------------------------------------------------

void c128_state::write_memory(offs_t offset, offs_t vma, uint8_t data, int ba, int aec, int z80io)
{
	int rw = 0, ms0 = 1, ms1 = 1, ms2 = 1, ms3 = 1, cas0 = 1, cas1 = 1;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	offs_t ta = m_mmu->ta_r(offset, aec, &ms0, &ms1, &ms2, &ms3, &cas0, &cas1);
	offs_t ca = ta | (offset & 0xff);
	offs_t ma = ta | (offset & 0xff);
	offs_t sa = offset & 0xff;

	int plaout = read_pla(offset, ca, vma, ba, rw, aec, z80io, ms3, ms2, ms1, ms0);

	m_clrbank = BIT(plaout, PLA_OUT_CLRBANK);

	if (!BIT(plaout, PLA_OUT_CASENB) && !BIT(plaout, PLA_OUT_DWE))
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
	if (!BIT(plaout, PLA_OUT_COLORRAM) && !BIT(plaout, PLA_OUT_GWE))
	{
		m_color_ram[(m_clrbank << 10) | (ta & 0x300) | sa] = data & 0x0f;
	}
	if (!BIT(plaout, PLA_OUT_VIC))
	{
		m_vic->write(offset & 0x3f, data);
	}
	if (!BIT(plaout, PLA_OUT_IOCS) && BIT(offset, 10))
	{
		switch ((BIT(offset, 11) << 2) | ((offset >> 8) & 0x03))
		{
		case 0: // SID
			m_sid->write(offset & 0x1f, data);
			break;

		case 2: // CS8563
			if (BIT(offset, 0))
			{
				m_vdc->register_w(data);
			}
			else
			{
				m_vdc->address_w(data);
			}
			break;

		case 4: // CIA1
			m_cia1->write(offset & 0x0f, data);
			break;

		case 5: // CIA2
			m_cia2->write(offset & 0x0f, data);
			break;

		case 6: // I/O1
			io1 = 0;
			break;

		case 7: // I/O2
			io2 = 0;
			break;
		}
	}

	int roml = BIT(plaout, PLA_OUT_ROML);
	int romh = BIT(plaout, PLA_OUT_ROMH);

	m_exp->cd_w(ca, data, sphi2, ba, roml, romh, io1, io2);

	m_mmu->write(offset, data);
}


//-------------------------------------------------
//  z80_r -
//-------------------------------------------------

uint8_t c128_state::z80_r(offs_t offset)
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_w -
//-------------------------------------------------

void c128_state::z80_w(offs_t offset, uint8_t data)
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	write_memory(offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_io_r -
//-------------------------------------------------

uint8_t c128_state::z80_io_r(offs_t offset)
{
	int ba = 1, aec = 1, z80io = 0;
	offs_t vma = 0;

	return read_memory(offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_io_w -
//-------------------------------------------------

void c128_state::z80_io_w(offs_t offset, uint8_t data)
{
	int ba = 1, aec = 1, z80io = 0;
	offs_t vma = 0;

	write_memory(offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t c128_state::read(offs_t offset)
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void c128_state::write(offs_t offset, uint8_t data)
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	write_memory(offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

uint8_t c128_state::vic_videoram_r(offs_t offset)
{
	int ba = 0, aec = 0, z80io = 1;

	return read_memory(0, offset, ba, aec, z80io);
}


//-------------------------------------------------
//  vic_colorram_r -
//-------------------------------------------------

uint8_t c128_state::vic_colorram_r(offs_t offset)
{
	return m_color_ram[(m_clrbank << 10) | offset];
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

void c128_state::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c128_state::z80_r), FUNC(c128_state::z80_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

void c128_state::z80_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c128_state::z80_io_r), FUNC(c128_state::z80_io_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( m8502_mem )
//-------------------------------------------------

void c128_state::m8502_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c128_state::read), FUNC(c128_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

void c128_state::vic_videoram_map(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(c128_state::vic_videoram_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

void c128_state::vic_colorram_map(address_map &map)
{
	map(0x000, 0x3ff).r(FUNC(c128_state::vic_colorram_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( vdc_videoram_map )
//-------------------------------------------------

void c128_state::vdc_videoram_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( c128 )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::write_restore )
{
	m_nmi->in_w<1>(!state);
}

INPUT_CHANGED_MEMBER( c128_state::caps_lock )
{
	m_caps_lock = newval;
}

static INPUT_PORTS_START( c128 )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                                    PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                    PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                                    PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)                                    PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)             PORT_CHAR(13)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INST DEL") PORT_CODE(KEYCODE_BACKSPACE)       PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('I')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('P')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('+')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191,'^') PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR HOME") PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR(0xA3)

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN STOP") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)                               PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE)   PORT_CHAR(0x2190)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "K0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)             PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_F6)               PORT_CHAR('\t')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(PGUP))

	PORT_START( "K1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)          PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_F5)               PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START( "K2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NO SCROLL") PORT_CODE(KEYCODE_F12) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)             PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)              PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)              PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)                PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)             PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START( "RESTORE" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_PRTSCR) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c128_state, write_restore)

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)

	PORT_START( "40_80" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("40/80 DISPLAY") PORT_CODE(KEYCODE_F11) PORT_TOGGLE
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128_de )
//-------------------------------------------------

static INPUT_PORTS_START( c128_de )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z  { Y }") PORT_CODE(KEYCODE_Z)                   PORT_CHAR('Z')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y  { Z }") PORT_CODE(KEYCODE_Y)                   PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7  '  { 7  / }") PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0  { = }") PORT_CODE(KEYCODE_0)                   PORT_CHAR('0')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(",  <  { ; }") PORT_CODE(KEYCODE_COMMA)            PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Paragraph  \xE2\x86\x91  { \xc3\xbc }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00A7) PORT_CHAR(0x2191)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".  >  { : }") PORT_CODE(KEYCODE_STOP)             PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-  { '  ` }") PORT_CODE(KEYCODE_EQUALS)           PORT_CHAR('-')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+  { \xc3\x9f ? }") PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('+')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/  ?  { -  _ }") PORT_CODE(KEYCODE_SLASH)                 PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Sum  Pi  { ] \\ }") PORT_CODE(KEYCODE_DEL)                PORT_CHAR(0x03A3) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  { # ' }") PORT_CODE(KEYCODE_BACKSLASH)                 PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";  ]  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE)             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("*  `  { +  * }") PORT_CODE(KEYCODE_CLOSEBRACE)            PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\  { [  \xE2\x86\x91 }") PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)                    PORT_CHAR('_')

	PORT_MODIFY( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ASCII/DIN") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128_fr )
//-------------------------------------------------
#ifdef UNUSED_CODE
static INPUT_PORTS_START( c128_fr )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z  { W }") PORT_CODE(KEYCODE_Z)               PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4  $  { '  4 }") PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A  { Q }") PORT_CODE(KEYCODE_A)               PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W  { Z }") PORT_CODE(KEYCODE_W)               PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3  #  { \"  3 }") PORT_CODE(KEYCODE_3)        PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6  &  { Paragraph  6 }") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5  %  { (  5 }") PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8  (  { !  8 }") PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7  '  { \xc3\xa8  7 }") PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K  Large-  { \\ }") PORT_CODE(KEYCODE_K)      PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M  Large-/  { ,  ? }") PORT_CODE(KEYCODE_M)   PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0  { \xc3\xa0  0 }") PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9  )  { \xc3\xa7  9 }") PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(",  <  { ;  . }") PORT_CODE(KEYCODE_COMMA)                     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@  \xc3\xbb  { ^  \xc2\xa8 }") PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@') PORT_CHAR(0x00FB)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xb9  % }") PORT_CODE(KEYCODE_COLON)              PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".  >  { :  / }") PORT_CODE(KEYCODE_STOP)                      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-  \xc2\xb0  { -  _ }") PORT_CODE(KEYCODE_EQUALS)             PORT_CHAR('-') PORT_CHAR(0xB0)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+  \xc3\xab  { )  \xc2\xb0 }") PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('+') PORT_CHAR(0x00EB)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/  ?  { =  + }") PORT_CODE(KEYCODE_SLASH)                     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi  { *  ] }") PORT_CODE(KEYCODE_DEL)           PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  {\xE2\x86\x91  \\ }") PORT_CODE(KEYCODE_BACKSLASH)         PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";  ]  { M  Large-/ }") PORT_CODE(KEYCODE_QUOTE)               PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("*  `  { $  [ }") PORT_CODE(KEYCODE_CLOSEBRACE)                PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\  { @  # }") PORT_CODE(KEYCODE_BACKSLASH)                   PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q  { A }") PORT_CODE(KEYCODE_Q)               PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2  \"  { \xc3\xa9  2 }") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_   { <  > }") PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1  !  { &  1 }") PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!')

	PORT_MODIFY( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK ASCII/CC") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
INPUT_PORTS_END
#endif

//-------------------------------------------------
//  INPUT_PORTS( c128_it )
//-------------------------------------------------
#ifdef UNUSED_CODE
static INPUT_PORTS_START( c128_it )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z  { W }") PORT_CODE(KEYCODE_Z)                       PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4  $  { '  4 }") PORT_CODE(KEYCODE_4)                 PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W  { Z }") PORT_CODE(KEYCODE_W)                       PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3  #  { \"  3 }") PORT_CODE(KEYCODE_3)                PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6  &  { _  6 }") PORT_CODE(KEYCODE_6)                 PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5  %  { (  5 }") PORT_CODE(KEYCODE_5)                 PORT_CHAR('5') PORT_CHAR('%')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8  (  { &  8 }") PORT_CODE(KEYCODE_8)                 PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7  '  { \xc3\xa8  7 }") PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M  Large-/  { ,  ? }") PORT_CODE(KEYCODE_M)           PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0  { \xc3\xa0  0 }") PORT_CODE(KEYCODE_0)             PORT_CHAR('0')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9  )  { \xc3\xa7  9 }") PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(",  <   { ;  . }") PORT_CODE(KEYCODE_COMMA)            PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@  \xc3\xbb  { \xc3\xac  = }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR(0x00FB)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xb9  % }") PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".  >  { :  / }") PORT_CODE(KEYCODE_STOP)              PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-  \xc2\xb0  { -  + }") PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-') PORT_CHAR(0xB0)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+  \xc3\xab  { )  \xc2\xb0 }") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR(0x00EB)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/  ?  { \xc3\xb2  ! }") PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi  { *  ] }") PORT_CODE(KEYCODE_DEL)   PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  { \xE2\x86\x91  \\ }") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";  ]  { M }") PORT_CODE(KEYCODE_QUOTE)                PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("*  `  { $  [ }") PORT_CODE(KEYCODE_CLOSEBRACE)        PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\  { @  # }") PORT_CODE(KEYCODE_BACKSLASH2)          PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2  \"  { \xc3\xa9  2 }") PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)                PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1  !  { \xc2\xa3  1 }") PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
INPUT_PORTS_END
#endif

//-------------------------------------------------
//  INPUT_PORTS( c128_se )
//-------------------------------------------------

static INPUT_PORTS_START( c128_se )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7  '  { 7  / }") PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("]  { \xc3\xa2 }") PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR(']')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("[  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)        PORT_CHAR('[')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                                    PORT_CHAR('=')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                                     PORT_CHAR('-')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";  +") PORT_CODE(KEYCODE_BACKSLASH)               PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xc2\xa3  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0xA3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_CLOSEBRACE)                 PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(":  *") PORT_CODE(KEYCODE_BACKSLASH2)              PORT_CHAR(':') PORT_CHAR('*')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)            PORT_CHAR('_')

	PORT_MODIFY( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK ASCII/CC") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
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
	return BIT(m_40_80->read(), 0);
}


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

static GFXDECODE_START( gfx_c128 )
	GFXDECODE_ENTRY( "charom", 0x0000, gfx_8x8x1, 0, 1 )
GFXDECODE_END


//-------------------------------------------------
//  MOS8564_INTERFACE( vic_intf )
//-------------------------------------------------

void c128_state::vic_k_w(uint8_t data)
{
	m_vic_k = data;
}


//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

uint8_t c128_state::sid_potx_r()
{
	uint8_t data = 0xff;

	switch (m_cia1->pa_r() >> 6)
	{
	case 1: data = m_joy1->read_pot_x(); break;
	case 2: data = m_joy2->read_pot_x(); break;
	case 3:
		if (m_joy1->has_pot_x() && m_joy2->has_pot_x())
		{
			data = 1 / (1 / m_joy1->read_pot_x() + 1 / m_joy2->read_pot_x());
		}
		else if (m_joy1->has_pot_x())
		{
			data = m_joy1->read_pot_x();
		}
		else if (m_joy2->has_pot_x())
		{
			data = m_joy2->read_pot_x();
		}
		break;
	}

	return data;
}

uint8_t c128_state::sid_poty_r()
{
	uint8_t data = 0xff;

	switch (m_cia1->pa_r() >> 6)
	{
	case 1: data = m_joy1->read_pot_y(); break;
	case 2: data = m_joy2->read_pot_y(); break;
	case 3:
		if (m_joy1->has_pot_y() && m_joy2->has_pot_y())
		{
			data = 1 / (1 / m_joy1->read_pot_y() + 1 / m_joy2->read_pot_y());
		}
		else if (m_joy1->has_pot_y())
		{
			data = m_joy1->read_pot_y();
		}
		else if (m_joy2->has_pot_y())
		{
			data = m_joy2->read_pot_y();
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia1_intf )
//-------------------------------------------------

uint8_t c128_state::cia1_pa_r()
{
	/*

	    bit     description

	    PA0     COL0, JOYB0
	    PA1     COL1, JOYB1
	    PA2     COL2, JOYB2
	    PA3     COL3, JOYB3
	    PA4     COL4, FBTNB
	    PA5     COL5
	    PA6     COL6
	    PA7     COL7

	*/

	uint8_t data = 0xff;

	// joystick
	uint8_t joy_b = m_joy2->read_joy();

	data &= (0xf0 | (joy_b & 0x0f));
	data &= ~(!BIT(joy_b, 5) << 4);

	// keyboard
	uint8_t cia1_pb = m_cia1->pb_r();
	uint32_t row[8] = { m_row[0]->read(), m_row[1]->read() & m_lock->read(), m_row[2]->read(), m_row[3]->read(),
						m_row[4]->read(), m_row[5]->read(), m_row[6]->read(), m_row[7]->read() };

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(cia1_pb, i))
		{
			if (!BIT(row[7], i)) data &= ~0x80;
			if (!BIT(row[6], i)) data &= ~0x40;
			if (!BIT(row[5], i)) data &= ~0x20;
			if (!BIT(row[4], i)) data &= ~0x10;
			if (!BIT(row[3], i)) data &= ~0x08;
			if (!BIT(row[2], i)) data &= ~0x04;
			if (!BIT(row[1], i)) data &= ~0x02;
			if (!BIT(row[0], i)) data &= ~0x01;
		}
	}

	return data;
}

void c128_state::cia1_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     COL0, JOYB0
	    PA1     COL1, JOYB1
	    PA2     COL2, JOYB2
	    PA3     COL3, JOYB3
	    PA4     COL4, FBTNB
	    PA5     COL5
	    PA6     COL6
	    PA7     COL7

	*/

	m_joy2->joy_w(data & 0x1f);
}

uint8_t c128_state::cia1_pb_r()
{
	/*

	    bit     description

	    PB0     ROW0, JOYA0
	    PB1     ROW1, JOYA1
	    PB2     ROW2, JOYA2
	    PB3     ROW3, JOYA3
	    PB4     ROW4, FBTNA, _LP
	    PB5     ROW5
	    PB6     ROW6
	    PB7     ROW7

	*/

	uint8_t data = 0xff;

	// joystick
	uint8_t joy_a = m_joy1->read_joy();

	data &= (0xf0 | (joy_a & 0x0f));
	data &= ~(!BIT(joy_a, 5) << 4);

	// keyboard
	uint8_t cia1_pa = m_cia1->pa_r();

	if (!BIT(cia1_pa, 7)) data &= m_row[7]->read();
	if (!BIT(cia1_pa, 6)) data &= m_row[6]->read();
	if (!BIT(cia1_pa, 5)) data &= m_row[5]->read();
	if (!BIT(cia1_pa, 4)) data &= m_row[4]->read();
	if (!BIT(cia1_pa, 3)) data &= m_row[3]->read();
	if (!BIT(cia1_pa, 2)) data &= m_row[2]->read();
	if (!BIT(cia1_pa, 1)) data &= m_row[1]->read() & m_lock->read();
	if (!BIT(cia1_pa, 0)) data &= m_row[0]->read();

	if (!BIT(m_vic_k, 0)) data &= m_k[0]->read();
	if (!BIT(m_vic_k, 1)) data &= m_k[1]->read();
	if (!BIT(m_vic_k, 2)) data &= m_k[2]->read();

	return data;
}

void c128_state::cia1_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     ROW0, JOYA0
	    PB1     ROW1, JOYA1
	    PB2     ROW2, JOYA2
	    PB3     ROW3, JOYA3
	    PB4     ROW4, FBTNA, _LP
	    PB5     ROW5
	    PB6     ROW6
	    PB7     ROW7

	*/

	m_joy1->joy_w(data & 0x1f);

	m_vic->lp_w(BIT(data, 4));
}

WRITE_LINE_MEMBER( c128_state::cia1_cnt_w )
{
	m_cnt1 = state;
	m_user->write_4(state);

	update_iec();
}

WRITE_LINE_MEMBER( c128_state::cia1_sp_w )
{
	m_sp1 = state;
	m_user->write_5(state);

	update_iec();
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia2_intf )
//-------------------------------------------------

uint8_t c128_state::cia2_pa_r()
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

	uint8_t data = 0;

	// user port
	data |= m_user_pa2 << 2;

	// IEC bus
	data |= m_iec->clk_r() << 6;
	data |= m_iec->data_r() << 7;

	return data;
}

void c128_state::cia2_pa_w(uint8_t data)
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
	m_user->write_m(BIT(data, 2));

	// IEC bus
	m_iec->host_atn_w(!BIT(data, 3));
	m_iec->host_clk_w(!BIT(data, 4));
	m_iec_data_out = BIT(data, 5);

	update_iec();
}

uint8_t c128_state::cia2_pb_r()
{
	return m_user_pb;
}

void c128_state::cia2_pb_w(uint8_t data)
{
	m_user->write_c((data>>0)&1);
	m_user->write_d((data>>1)&1);
	m_user->write_e((data>>2)&1);
	m_user->write_f((data>>3)&1);
	m_user->write_h((data>>4)&1);
	m_user->write_j((data>>5)&1);
	m_user->write_k((data>>6)&1);
	m_user->write_l((data>>7)&1);
}

//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

uint8_t c128_state::cpu_r()
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

	uint8_t data = 0x07;

	// cassette sense
	data |= m_cassette->sense_r() << 4;

	// CAPS LOCK
	data |= m_caps_lock << 6;

	return data;
}

void c128_state::cpu_w(uint8_t data)
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

	m_iec->host_data_w(data_out);

	// fast serial clock in
	int srq_in = m_iec->srq_r();

	m_cia1->cnt_w(fsdir || srq_in);

	// fast serial clock out
	int srq_out = 1;

	if (fsdir) srq_out &= m_cnt1;

	m_iec->host_srq_w(srq_out);
}

WRITE_LINE_MEMBER( c128_state::iec_srq_w )
{
	update_iec();
}

WRITE_LINE_MEMBER( c128_state::iec_data_w )
{
	update_iec();
}


//-------------------------------------------------
//  C64_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

uint8_t c128_state::exp_dma_cd_r(offs_t offset)
{
	int ba = 0, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(offset, vma, ba, aec, z80io);
}

void c128_state::exp_dma_cd_w(offs_t offset, uint8_t data)
{
	int ba = 0, aec = 1, z80io = 1;
	offs_t vma = 0;

	return write_memory(offset, data, vma, ba, aec, z80io);
}

WRITE_LINE_MEMBER( c128_state::exp_dma_w )
{
	m_exp_dma = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c128_state::exp_reset_w )
{
	if (!state)
	{
		machine_reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( c128dcr_iec_devices )
//-------------------------------------------------

void c128dcr_iec_devices(device_slot_interface &device)
{
	device.option_add("c1571", C1571);
	device.option_add("c1571cr", C1571CR);
}


//-------------------------------------------------
//  SLOT_INTERFACE( c128d81_iec_devices )
//-------------------------------------------------

void c128d81_iec_devices(device_slot_interface &device)
{
	device.option_add("c1563", C1563);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( c64 )
//-------------------------------------------------

void c128_state::machine_start()
{
	// initialize memory
	uint8_t data = 0xff;

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
	save_item(NAME(m_exp_dma));
	save_item(NAME(m_vic_k));
	save_item(NAME(m_caps_lock));
}


void c128_state::machine_reset()
{
	m_maincpu->reset();
	m_reset = 1;

	m_mmu->reset();
	m_vic->reset();
	m_vdc->reset();
	m_sid->reset();
	m_cia1->reset();
	m_cia2->reset();

	m_iec->reset();
	m_exp->reset();

	m_user->write_3(0);
	m_user->write_3(1);
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( ntsc )
//-------------------------------------------------

void c128_state::ntsc(machine_config &config)
{
	// basic hardware
	Z80(config, m_maincpu, XTAL(14'318'181)*2/3.5/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &c128_state::z80_mem);
	m_maincpu->set_addrmap(AS_IO, &c128_state::z80_io);

	M8502(config, m_subcpu, XTAL(14'318'181)*2/3.5/8);
	m_subcpu->read_callback().set(FUNC(c128_state::cpu_r));
	m_subcpu->write_callback().set(FUNC(c128_state::cpu_w));
	m_subcpu->set_pulls(0x07, 0x20);
	m_subcpu->set_addrmap(AS_PROGRAM, &c128_state::m8502_mem);
	config.set_perfect_quantum(m_subcpu);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irq"));
	irq.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	irq.output_handler().append_inputline(m_subcpu, m8502_device::IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, m_nmi);
	m_nmi->output_handler().set_inputline(m_subcpu, m8502_device::NMI_LINE);

	// video hardware
	MOS8563(config, m_vdc, XTAL(16'000'000));
	m_vdc->set_screen(SCREEN_VDC_TAG);
	m_vdc->set_addrmap(0, &c128_state::vdc_videoram_map);
	m_vdc->set_show_border_area(true);
	m_vdc->set_char_width(8);

	screen_device &screen_vdc(SCREEN(config, SCREEN_VDC_TAG, SCREEN_TYPE_RASTER));
	screen_vdc.set_refresh_hz(60);
	screen_vdc.set_size(640, 200);
	screen_vdc.set_visarea(0, 640-1, 0, 200-1);
	screen_vdc.set_screen_update(MOS8563_TAG, FUNC(mos8563_device::screen_update));

	MOS8564(config, m_vic, XTAL(14'318'181)*2/3.5);
	m_vic->set_cpu(m_subcpu);
	m_vic->irq_callback().set("irq", FUNC(input_merger_device::in_w<1>));
	m_vic->k_callback().set(FUNC(c128_state::vic_k_w));
	m_vic->set_screen(SCREEN_VIC_TAG);
	m_vic->set_addrmap(0, &c128_state::vic_videoram_map);
	m_vic->set_addrmap(1, &c128_state::vic_colorram_map);

	screen_device &screen_vic(SCREEN(config, SCREEN_VIC_TAG, SCREEN_TYPE_RASTER));
	screen_vic.set_refresh_hz(VIC6567_VRETRACERATE);
	screen_vic.set_size(VIC6567_COLUMNS, VIC6567_LINES);
	screen_vic.set_visarea(0, VIC6567_VISIBLECOLUMNS - 1, 0, VIC6567_VISIBLELINES - 1);
	screen_vic.set_screen_update(MOS8564_TAG, FUNC(mos8564_device::screen_update));

	GFXDECODE(config, "gfxdecode", MOS8563_TAG, gfx_c128);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	MOS6581(config, m_sid, XTAL(14'318'181)*2/3.5/8);
	m_sid->potx().set(FUNC(c128_state::sid_potx_r));
	m_sid->poty().set(FUNC(c128_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "speaker", 0.5);

	// devices
	MOS8722(config, m_mmu, XTAL(14'318'181)*2/3.5/8);
	m_mmu->z80en().set(FUNC(c128_state::mmu_z80en_w));
	m_mmu->fsdir().set(FUNC(c128_state::mmu_fsdir_w));
	m_mmu->game().set(FUNC(c128_state::mmu_game_r));
	m_mmu->exrom().set(FUNC(c128_state::mmu_exrom_r));
	m_mmu->sense40().set(FUNC(c128_state::mmu_sense40_r));

	MOS8721(config, m_pla);

	MOS6526(config, m_cia1, XTAL(14'318'181)*2/3.5/8);
	m_cia1->set_tod_clock(60);
	m_cia1->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<0>));
	m_cia1->cnt_wr_callback().set(FUNC(c128_state::cia1_cnt_w));
	m_cia1->sp_wr_callback().set(FUNC(c128_state::cia1_sp_w));
	m_cia1->pa_rd_callback().set(FUNC(c128_state::cia1_pa_r));
	m_cia1->pa_wr_callback().set(FUNC(c128_state::cia1_pa_w));
	m_cia1->pb_rd_callback().set(FUNC(c128_state::cia1_pb_r));
	m_cia1->pb_wr_callback().set(FUNC(c128_state::cia1_pb_w));

	MOS6526(config, m_cia2, XTAL(14'318'181)*2/3.5/8);
	m_cia2->set_tod_clock(60);
	m_cia2->irq_wr_callback().set(m_nmi, FUNC(input_merger_device::in_w<0>));
	m_cia2->cnt_wr_callback().set(m_user, FUNC(pet_user_port_device::write_6));
	m_cia2->sp_wr_callback().set(m_user, FUNC(pet_user_port_device::write_7));
	m_cia2->pa_rd_callback().set(FUNC(c128_state::cia2_pa_r));
	m_cia2->pa_wr_callback().set(FUNC(c128_state::cia2_pa_w));
	m_cia2->pb_rd_callback().set(FUNC(c128_state::cia2_pb_r));
	m_cia2->pb_wr_callback().set(FUNC(c128_state::cia2_pb_w));
	m_cia2->pc_wr_callback().set(m_user, FUNC(pet_user_port_device::write_8));

	PET_DATASSETTE_PORT(config, m_cassette, cbm_datassette_devices, "c1530");
	m_cassette->read_handler().set(m_cia2, FUNC(mos6526_device::flag_w));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	m_joy1->trigger_wr_callback().set(m_vic, FUNC(mos8564_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

	C64_EXPANSION_SLOT(config, m_exp, XTAL(14'318'181)*2/3.5/8, c64_expansion_cards, nullptr);
	m_exp->irq_callback().set("irq", FUNC(input_merger_device::in_w<2>));
	m_exp->nmi_callback().set(m_nmi, FUNC(input_merger_device::in_w<2>));
	m_exp->reset_callback().set(FUNC(c128_state::exp_reset_w));
	m_exp->cd_input_callback().set(FUNC(c128_state::exp_dma_cd_r));
	m_exp->cd_output_callback().set(FUNC(c128_state::exp_dma_cd_w));
	m_exp->dma_callback().set(FUNC(c128_state::exp_dma_w));

	PET_USER_PORT(config, m_user, c64_user_port_cards, nullptr);
	m_user->p3_handler().set(FUNC(c128_state::exp_reset_w));
	m_user->p4_handler().set(m_cia1, FUNC(mos6526_device::cnt_w));
	m_user->p5_handler().set(m_cia1, FUNC(mos6526_device::sp_w));
	m_user->p6_handler().set(m_cia2, FUNC(mos6526_device::cnt_w));
	m_user->p7_handler().set(m_cia2, FUNC(mos6526_device::sp_w));
	m_user->p9_handler().set(m_iec, FUNC(cbm_iec_device::host_atn_w));
	m_user->pb_handler().set(m_cia2, FUNC(mos6526_device::flag_w));
	m_user->pc_handler().set(FUNC(c128_state::write_user_pb0));
	m_user->pd_handler().set(FUNC(c128_state::write_user_pb1));
	m_user->pe_handler().set(FUNC(c128_state::write_user_pb2));
	m_user->pf_handler().set(FUNC(c128_state::write_user_pb3));
	m_user->ph_handler().set(FUNC(c128_state::write_user_pb4));
	m_user->pj_handler().set(FUNC(c128_state::write_user_pb5));
	m_user->pk_handler().set(FUNC(c128_state::write_user_pb6));
	m_user->pl_handler().set(FUNC(c128_state::write_user_pb7));
	m_user->pm_handler().set(FUNC(c128_state::write_user_pa2));

	QUICKLOAD(config, "quickload", "p00,prg", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(c128_state::quickload_c128));

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("c128_cart").set_filter("NTSC");
	SOFTWARE_LIST(config, "flop_list").set_original("c128_flop").set_filter("NTSC");
	SOFTWARE_LIST(config, "from_list").set_original("c128_rom").set_filter("NTSC");
	SOFTWARE_LIST(config, "cart_list_c64").set_original("c64_cart").set_filter("NTSC");
	SOFTWARE_LIST(config, "cass_list_c64").set_original("c64_cass").set_filter("NTSC");
	SOFTWARE_LIST(config, "cart_list_vic10").set_original("vic10").set_filter("NTSC");
	// disk softlist split into originals and misc (homebrew and cracks)
	SOFTWARE_LIST(config, "flop525_orig").set_original("c64_flop_orig").set_filter("NTSC");
	SOFTWARE_LIST(config, "flop525_misc").set_compatible("c64_flop_misc").set_filter("NTSC");

	// function ROM
	GENERIC_SOCKET(config, "from", generic_plain_slot, "c128_rom", "bin,rom");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");
}


//-------------------------------------------------
//  machine_config( c128 )
//-------------------------------------------------

void c128_state::c128(machine_config &config)
{
	ntsc(config);
	cbm_iec_slot_device::add(config, m_iec, "c1571");
	m_iec->srq_callback().set(FUNC(c128_state::iec_srq_w));
	m_iec->data_callback().set(FUNC(c128_state::iec_data_w));
}


//-------------------------------------------------
//  machine_config( c128dcr )
//-------------------------------------------------

void c128_state::c128dcr(machine_config &config)
{
	ntsc(config);
	cbm_iec_slot_device::add(config, m_iec, "c1571"); // TODO c1571cr
	m_iec->srq_callback().set(FUNC(c128_state::iec_srq_w));
	m_iec->data_callback().set(FUNC(c128_state::iec_data_w));
}


//-------------------------------------------------
//  machine_config( c128d81 )
//-------------------------------------------------

void c128_state::c128d81(machine_config &config)
{
	ntsc(config);
	cbm_iec_slot_device::add(config, m_iec, nullptr);
	m_iec->srq_callback().set(FUNC(c128_state::iec_srq_w));
	m_iec->data_callback().set(FUNC(c128_state::iec_data_w));

	CBM_IEC_SLOT(config.replace(), "iec8", 8, c128d81_iec_devices, "c1563");
}


//-------------------------------------------------
//  machine_config( pal )
//-------------------------------------------------

void c128_state::pal(machine_config &config)
{
	// basic hardware
	Z80(config, m_maincpu, XTAL(17'734'472)*2/4.5/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &c128_state::z80_mem);
	m_maincpu->set_addrmap(AS_IO, &c128_state::z80_io);

	M8502(config, m_subcpu, XTAL(17'734'472)*2/4.5/8);
	m_subcpu->read_callback().set(FUNC(c128_state::cpu_r));
	m_subcpu->write_callback().set(FUNC(c128_state::cpu_w));
	m_subcpu->set_pulls(0x07, 0x20);
	m_subcpu->set_addrmap(AS_PROGRAM, &c128_state::m8502_mem);
	config.set_perfect_quantum(m_subcpu);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irq"));
	irq.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	irq.output_handler().append_inputline(m_subcpu, m8502_device::IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, m_nmi);
	m_nmi->output_handler().set_inputline(m_subcpu, m8502_device::NMI_LINE);

	// video hardware
	MOS8563(config, m_vdc, XTAL(16'000'000));
	m_vdc->set_screen(SCREEN_VDC_TAG);
	m_vdc->set_addrmap(0, &c128_state::vdc_videoram_map);
	m_vdc->set_show_border_area(true);
	m_vdc->set_char_width(8);

	screen_device &screen_vdc(SCREEN(config, SCREEN_VDC_TAG, SCREEN_TYPE_RASTER));
	screen_vdc.set_refresh_hz(60);
	screen_vdc.set_size(640, 200);
	screen_vdc.set_visarea(0, 640-1, 0, 200-1);
	screen_vdc.set_screen_update(MOS8563_TAG, FUNC(mos8563_device::screen_update));

	mos8566_device &mos8566(MOS8566(config, MOS8566_TAG, XTAL(17'734'472)*2/4.5));
	mos8566.set_cpu(M8502_TAG);
	mos8566.irq_callback().set("irq", FUNC(input_merger_device::in_w<1>));
	mos8566.k_callback().set(FUNC(c128_state::vic_k_w));
	mos8566.set_screen(SCREEN_VIC_TAG);
	mos8566.set_addrmap(0, &c128_state::vic_videoram_map);
	mos8566.set_addrmap(1, &c128_state::vic_colorram_map);

	screen_device &screen_vic(SCREEN(config, SCREEN_VIC_TAG, SCREEN_TYPE_RASTER));
	screen_vic.set_refresh_hz(VIC6569_VRETRACERATE);
	screen_vic.set_size(VIC6569_COLUMNS, VIC6569_LINES);
	screen_vic.set_visarea(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1);
	screen_vic.set_screen_update(MOS8566_TAG, FUNC(mos8566_device::screen_update));

	GFXDECODE(config, "gfxdecode", MOS8563_TAG, gfx_c128);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	MOS6581(config, m_sid, XTAL(17'734'472)*2/4.5/8);
	m_sid->potx().set(FUNC(c128_state::sid_potx_r));
	m_sid->poty().set(FUNC(c128_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "speaker", 0.5);

	// devices
	MOS8722(config, m_mmu, XTAL(17'734'472)*2/4.5/8);
	m_mmu->z80en().set(FUNC(c128_state::mmu_z80en_w));
	m_mmu->fsdir().set(FUNC(c128_state::mmu_fsdir_w));
	m_mmu->game().set(FUNC(c128_state::mmu_game_r));
	m_mmu->exrom().set(FUNC(c128_state::mmu_exrom_r));
	m_mmu->sense40().set(FUNC(c128_state::mmu_sense40_r));

	MOS8721(config, m_pla);

	MOS6526(config, m_cia1, XTAL(17'734'472)*2/4.5/8);
	m_cia1->set_tod_clock(50);
	m_cia1->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<0>));
	m_cia1->cnt_wr_callback().set(FUNC(c128_state::cia1_cnt_w));
	m_cia1->sp_wr_callback().set(FUNC(c128_state::cia1_sp_w));
	m_cia1->pa_rd_callback().set(FUNC(c128_state::cia1_pa_r));
	m_cia1->pa_wr_callback().set(FUNC(c128_state::cia1_pa_w));
	m_cia1->pb_rd_callback().set(FUNC(c128_state::cia1_pb_r));
	m_cia1->pb_wr_callback().set(FUNC(c128_state::cia1_pb_w));

	MOS6526(config, m_cia2, XTAL(17'734'472)*2/4.5/8);
	m_cia2->set_tod_clock(50);
	m_cia2->irq_wr_callback().set(m_nmi, FUNC(input_merger_device::in_w<0>));
	m_cia2->cnt_wr_callback().set(m_user, FUNC(pet_user_port_device::write_6));
	m_cia2->sp_wr_callback().set(m_user, FUNC(pet_user_port_device::write_7));
	m_cia2->pa_rd_callback().set(FUNC(c128_state::cia2_pa_r));
	m_cia2->pa_wr_callback().set(FUNC(c128_state::cia2_pa_w));
	m_cia2->pb_rd_callback().set(FUNC(c128_state::cia2_pb_r));
	m_cia2->pb_wr_callback().set(FUNC(c128_state::cia2_pb_w));
	m_cia2->pc_wr_callback().set(m_user, FUNC(pet_user_port_device::write_8));

	PET_DATASSETTE_PORT(config, m_cassette, cbm_datassette_devices, "c1530");
	m_cassette->read_handler().set(m_cia2, FUNC(mos6526_device::flag_w));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	m_joy1->trigger_wr_callback().set(MOS8566_TAG, FUNC(mos8566_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

	C64_EXPANSION_SLOT(config, m_exp, XTAL(17'734'472)*2/4.5/8, c64_expansion_cards, nullptr);
	m_exp->irq_callback().set("irq", FUNC(input_merger_device::in_w<2>));
	m_exp->nmi_callback().set(m_nmi, FUNC(input_merger_device::in_w<2>));
	m_exp->reset_callback().set(FUNC(c128_state::exp_reset_w));
	m_exp->cd_input_callback().set(FUNC(c128_state::exp_dma_cd_r));
	m_exp->cd_output_callback().set(FUNC(c128_state::exp_dma_cd_w));
	m_exp->dma_callback().set(FUNC(c128_state::exp_dma_w));

	PET_USER_PORT(config, m_user, c64_user_port_cards, nullptr);
	m_user->p3_handler().set(FUNC(c128_state::exp_reset_w));
	m_user->p4_handler().set(m_cia1, FUNC(mos6526_device::cnt_w));
	m_user->p5_handler().set(m_cia1, FUNC(mos6526_device::sp_w));
	m_user->p6_handler().set(m_cia2, FUNC(mos6526_device::cnt_w));
	m_user->p7_handler().set(m_cia2, FUNC(mos6526_device::sp_w));
	m_user->p9_handler().set(m_iec, FUNC(cbm_iec_device::host_atn_w));
	m_user->pb_handler().set(m_cia2, FUNC(mos6526_device::flag_w));
	m_user->pc_handler().set(FUNC(c128_state::write_user_pb0));
	m_user->pd_handler().set(FUNC(c128_state::write_user_pb1));
	m_user->pe_handler().set(FUNC(c128_state::write_user_pb2));
	m_user->pf_handler().set(FUNC(c128_state::write_user_pb3));
	m_user->ph_handler().set(FUNC(c128_state::write_user_pb4));
	m_user->pj_handler().set(FUNC(c128_state::write_user_pb5));
	m_user->pk_handler().set(FUNC(c128_state::write_user_pb6));
	m_user->pl_handler().set(FUNC(c128_state::write_user_pb7));
	m_user->pm_handler().set(FUNC(c128_state::write_user_pa2));

	QUICKLOAD(config, "quickload", "p00,prg", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(c128_state::quickload_c128));

	// software list
	SOFTWARE_LIST(config, "cart_list_vic10").set_original("vic10");
	SOFTWARE_LIST(config, "cart_list_c64").set_original("c64_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("c128_cart");
	SOFTWARE_LIST(config, "cass_list_c64").set_original("c64_cass");
	SOFTWARE_LIST(config, "flop_list_c64").set_original("c64_flop");
	SOFTWARE_LIST(config, "flop_list").set_original("c128_flop");
	SOFTWARE_LIST(config, "from_list").set_original("c128_rom");
	subdevice<software_list_device>("cart_list_vic10")->set_filter("PAL");
	subdevice<software_list_device>("cart_list_c64")->set_filter("PAL");
	subdevice<software_list_device>("cart_list")->set_filter("PAL");
	subdevice<software_list_device>("cass_list_c64")->set_filter("PAL");
	subdevice<software_list_device>("flop_list_c64")->set_filter("PAL");
	subdevice<software_list_device>("flop_list")->set_filter("PAL");
	subdevice<software_list_device>("from_list")->set_filter("PAL");

	// function ROM
	GENERIC_SOCKET(config, "from", generic_plain_slot, "c128_rom", "bin,rom");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");
}


//-------------------------------------------------
//  machine_config( c128pal )
//-------------------------------------------------

void c128_state::c128pal(machine_config &config)
{
	pal(config);
	cbm_iec_slot_device::add(config, m_iec, "c1571");
	m_iec->srq_callback().set(FUNC(c128_state::iec_srq_w));
	m_iec->data_callback().set(FUNC(c128_state::iec_data_w));
}


//-------------------------------------------------
//  machine_config( c128dcrp )
//-------------------------------------------------

void c128_state::c128dcrp(machine_config &config)
{
	pal(config);
	cbm_iec_slot_device::add(config, m_iec, "c1571"); // TODO c1571cr
	m_iec->srq_callback().set(FUNC(c128_state::iec_srq_w));
	m_iec->data_callback().set(FUNC(c128_state::iec_data_w));
}



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
	ROMX_LOAD( "318018-02.u33", 0x4000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6), ROM_BIOS(0) )
	ROMX_LOAD( "318019-02.u34", 0x8000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f), ROM_BIOS(0) )
	ROMX_LOAD( "318020-03.u35", 0xc000, 0x4000, CRC(1e94bb02) SHA1(e80ffbafae068cc0e42698ec5c5c39af46ac612a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(1) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(1) )
	ROMX_LOAD( "318020-05.u35", 0xc000, 0x4000, CRC(ba456b8e) SHA1(ceb6e1a1bf7e08eb9cbc651afa29e26adccf38ab), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(2) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(2) )
	ROMX_LOAD( "jiffydos c128.u35", 0xc000, 0x4000, CRC(4b7964de) SHA1(7d1898f32beae4b2ae610d469ce578a588efaa7c), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "quikslvr", "QuickSilver 128" ) // requires add-on cartridge
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(3) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(3) )
	ROMX_LOAD( "quicksilver128.u35", 0xc000, 0x4000, CRC(c2e74338) SHA1(916cdcc62eb631073aa7f096815dcf33b3229ca8), ROM_BIOS(3) )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "390059-01.u18", 0x0000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END

#define rom_c128p       rom_c128
#define rom_c128d       rom_c128
#define rom_c128dp      rom_c128
#define rom_c128dpr     rom_c128
#define rom_c128d81     rom_c128


//-------------------------------------------------
//  ROM( c128_de )
//-------------------------------------------------

ROM_START( c128_de )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_DEFAULT_BIOS("r4")
	ROM_LOAD( "251913-01.u32", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )
	ROM_SYSTEM_BIOS( 0, "r2", "Revision 2" )
	ROMX_LOAD( "318018-02.u33", 0x4000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6), ROM_BIOS(0) )
	ROMX_LOAD( "318019-02.u34", 0x8000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f), ROM_BIOS(0) )
	ROMX_LOAD( "315078-01.u35", 0xc000, 0x4000, CRC(a51e2168) SHA1(bcf82a89a8fc5d086bec2ff3bcbdecc8af2be3af), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(1) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(1) )
	ROMX_LOAD( "315078-02.u35", 0xc000, 0x4000, CRC(b275bb2e) SHA1(78ac5dcdd840b092ba1ee6d19b33af079613291f), ROM_BIOS(1) )

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

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "325181-01.bin", 0x0000, 0x2000, CRC(7a70d9b8) SHA1(aca3f7321ee7e6152f1f0afad646ae41964de4fb) ) // "C128 Char Sw/Fi"

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


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

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "390059-01.u18", 0x0000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END

#define rom_c128dcrp    rom_c128dcr


//-------------------------------------------------
//  ROM( c128dcr_de )
//-------------------------------------------------

ROM_START( c128dcr_de )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "318022-02.u34", 0x4000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )
	ROM_LOAD( "318077-01.u32", 0x0000, 0x4000, CRC(eb6e2c8f) SHA1(6b3d891fedabb5335f388a5d2a71378472ea60f4) )
	ROM_CONTINUE(              0xc000, 0x4000 )

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

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "325181-01.u18", 0x0000, 0x2000, CRC(7a70d9b8) SHA1(aca3f7321ee7e6152f1f0afad646ae41964de4fb) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT  COMPAT  MACHINE   INPUT    CLASS       INIT        COMPANY                        FULLNAME                               FLAGS
COMP( 1985, c128,       0,      0,      c128,     c128,    c128_state, empty_init, "Commodore Business Machines", "Commodore 128 (NTSC)",                MACHINE_SUPPORTS_SAVE )
COMP( 1985, c128p,      0,      0,      c128pal,  c128,    c128_state, empty_init, "Commodore Business Machines", "Commodore 128 (PAL)",                 MACHINE_SUPPORTS_SAVE )
COMP( 1985, c128_de,    c128,   0,      c128pal,  c128_de, c128_state, empty_init, "Commodore Business Machines", "Commodore 128 (Germany)",             MACHINE_SUPPORTS_SAVE )
//COMP( 1985, c128_fr,    c128,   0,      c128pal,  c128_fr, c128_state, empty_init, "Commodore Business Machines", "Commodore 128 (France)", MACHINE_SUPPORTS_SAVE )
//COMP( 1985, c128_no,    c128,   0,      c128pal,  c128_it, c128_state, empty_init, "Commodore Business Machines", "Commodore 128 (Norway)", MACHINE_SUPPORTS_SAVE )
COMP( 1985, c128_se,    c128,   0,      c128pal,  c128_se, c128_state, empty_init, "Commodore Business Machines", "Commodore 128 (Sweden/Finland)",      MACHINE_SUPPORTS_SAVE )
COMP( 1986, c128d,      c128,   0,      c128,     c128,    c128_state, empty_init, "Commodore Business Machines", "Commodore 128D (NTSC, prototype)",    MACHINE_SUPPORTS_SAVE )
COMP( 1986, c128dp,     c128,   0,      c128pal,  c128,    c128_state, empty_init, "Commodore Business Machines", "Commodore 128D (PAL)",                MACHINE_SUPPORTS_SAVE )

COMP( 1986, c128cr,     c128,   0,      c128,     c128,    c128_state, empty_init, "Commodore Business Machines", "Commodore 128CR (NTSC, prototype)",   MACHINE_SUPPORTS_SAVE )

COMP( 1987, c128dcr,    c128,   0,      c128dcr,  c128,    c128_state, empty_init, "Commodore Business Machines", "Commodore 128DCR (NTSC)",             MACHINE_SUPPORTS_SAVE )
COMP( 1987, c128dcrp,   c128,   0,      c128dcrp, c128,    c128_state, empty_init, "Commodore Business Machines", "Commodore 128DCR (PAL)",              MACHINE_SUPPORTS_SAVE )
COMP( 1987, c128dcr_de, c128,   0,      c128dcrp, c128_de, c128_state, empty_init, "Commodore Business Machines", "Commodore 128DCR (Germany)",          MACHINE_SUPPORTS_SAVE )
//COMP( 1986, c128dcr_it, c128,   0,      c128dcrp, c128_it, c128_state, empty_init, "Commodore Business Machines", "Commodore 128DCR (Italy)", MACHINE_SUPPORTS_SAVE )
COMP( 1987, c128dcr_se, c128,   0,      c128dcrp, c128_se, c128_state, empty_init, "Commodore Business Machines", "Commodore 128DCR (Sweden/Finland)",   MACHINE_SUPPORTS_SAVE )

COMP( 1986, c128d81,    c128,   0,      c128d81,  c128,    c128_state, empty_init, "Commodore Business Machines", "Commodore 128D/81 (NTSC, prototype)", MACHINE_SUPPORTS_SAVE )
