// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - floating bus writes to peripheral registers in m6502.c
    - sort out kernals between PAL/NTSC
    - tsuit215 test failures
        - IRQ (WRONG $DC0D)
        - NMI (WRONG $DD0D)
        - some CIA tests
    - PDC Clipper (C64 in a briefcase with 3" floppy, electroluminescent flat screen, thermal printer)

*/

#include "emu.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "bus/cbmiec/cbmiec.h"
#include "bus/cbmiec/c1541.h"
#include "bus/c64/exp.h"
#include "bus/vic20/user.h"
#include "bus/pet/cass.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6510.h"
#include "imagedev/snapquik.h"
#include "cbm_snqk.h"
#include "machine/input_merger.h"
#include "machine/mos6526.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "sound/mos6581.h"
#include "video/mos6566.h"


namespace {

#define MOS6567_TAG     "u19"
#define MOS6569_TAG     "u19"
#define MOS6581_TAG     "u18"
#define MOS6526_1_TAG   "u1"
#define MOS6526_2_TAG   "u2"
#define PLA_TAG         "u17"
#define SCREEN_TAG      "screen"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"
#define PET_USER_PORT_TAG     "user"

class c64_state : public driver_device
{
public:
	c64_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "u7"),
		m_nmi(*this, "nmi"),
		m_pla(*this, PLA_TAG),
		m_vic(*this, MOS6569_TAG),
		m_sid(*this, MOS6581_TAG),
		m_cia1(*this, MOS6526_1_TAG),
		m_cia2(*this, MOS6526_2_TAG),
		m_iec(*this, CBM_IEC_TAG),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_exp(*this, "exp"),
		m_user(*this, PET_USER_PORT_TAG),
		m_ram(*this, RAM_TAG),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_color_ram(*this, "color_ram", 0x400, ENDIANNESS_LITTLE),
		m_row(*this, "ROW%u", 0),
		m_lock(*this, "LOCK"),
		m_loram(1),
		m_hiram(1),
		m_charen(1),
		m_va14(1),
		m_va15(1),
		m_cass_rd(1),
		m_iec_srq(1)
	{ }

	// ROM
	uint8_t *m_basic;
	uint8_t *m_kernal;
	uint8_t *m_charom;

	required_device<m6510_device> m_maincpu;
	required_device<input_merger_device> m_nmi;
	required_device<pla_device> m_pla;
	required_device<mos6566_device> m_vic;
	required_device<mos6581_device> m_sid;
	required_device<mos6526_device> m_cia1;
	required_device<mos6526_device> m_cia2;
	optional_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<c64_expansion_slot_device> m_exp;
	required_device<pet_user_port_device> m_user;
	required_device<ram_device> m_ram;
	optional_device<pet_datassette_port_device> m_cassette;
	memory_share_creator<uint8_t> m_color_ram;
	optional_ioport_array<8> m_row;
	optional_ioport m_lock;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	[[maybe_unused]] void check_interrupts();
	int read_pla(offs_t offset, offs_t va, int rw, int aec, int ba);
	uint8_t read_memory(offs_t offset, offs_t va, int aec, int ba);
	void write_memory(offs_t offset, uint8_t data, int aec, int ba);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t vic_videoram_r(offs_t offset);
	uint8_t vic_colorram_r(offs_t offset);

	uint8_t sid_potx_r();
	uint8_t sid_poty_r();

	uint8_t cia1_pa_r();
	void cia1_pa_w(uint8_t data);
	uint8_t cia1_pb_r();
	void cia1_pb_w(uint8_t data);

	uint8_t cia2_pa_r();
	void cia2_pa_w(uint8_t data);

	uint8_t cpu_r();
	void cpu_w(uint8_t data);

	void write_restore(int state);
	void exp_dma_w(int state);
	void exp_reset_w(int state);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_c64);

	uint8_t cia2_pb_r();
	void cia2_pb_w(uint8_t data);

	void write_user_pa2(int state) { m_user_pa2 = state; }
	void write_user_pb0(int state) { if (state) m_user_pb |= 1; else m_user_pb &= ~1; }
	void write_user_pb1(int state) { if (state) m_user_pb |= 2; else m_user_pb &= ~2; }
	void write_user_pb2(int state) { if (state) m_user_pb |= 4; else m_user_pb &= ~4; }
	void write_user_pb3(int state) { if (state) m_user_pb |= 8; else m_user_pb &= ~8; }
	void write_user_pb4(int state) { if (state) m_user_pb |= 16; else m_user_pb &= ~16; }
	void write_user_pb5(int state) { if (state) m_user_pb |= 32; else m_user_pb &= ~32; }
	void write_user_pb6(int state) { if (state) m_user_pb |= 64; else m_user_pb &= ~64; }
	void write_user_pb7(int state) { if (state) m_user_pb |= 128; else m_user_pb &= ~128; }

	void update_cia1_flag() { m_cia1->flag_w(m_cass_rd & m_iec_srq); }
	void cass_rd_w(int state) { m_cass_rd = state; update_cia1_flag(); }
	void iec_srq_w(int state) { m_iec_srq = state; update_cia1_flag(); }

	// memory state
	int m_loram;
	int m_hiram;
	int m_charen;

	// video state
	int m_va14;
	int m_va15;

	// interrupt state
	int m_exp_dma;
	int m_cass_rd;
	int m_iec_srq;

	int m_user_pa2;
	int m_user_pb;

	void pal(machine_config &config);
	void ntsc(machine_config &config);
	void pet64(machine_config &config);
	void c64_mem(address_map &map) ATTR_COLD;
	void vic_colorram_map(address_map &map) ATTR_COLD;
	void vic_videoram_map(address_map &map) ATTR_COLD;

	offs_t dasm_zeropage(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname);
	offs_t dasm_vector(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname);
	offs_t dasm_zeropage_vector(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname);
	offs_t dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);
};


class sx64_state : public c64_state
{
public:
	sx64_state(const machine_config &mconfig, device_type type, const char *tag)
		: c64_state(mconfig, type, tag)
	{ }

	uint8_t cpu_r();
	void cpu_w(uint8_t data);
	void ntsc_sx(machine_config &config);
	void ntsc_dx(machine_config &config);
	void pal_sx(machine_config &config);
};


class c64c_state : public c64_state
{
public:
	c64c_state(const machine_config &mconfig, device_type type, const char *tag)
		: c64_state(mconfig, type, tag)
	{ }

	void pal_c(machine_config &config);
	void ntsc_c(machine_config &config);
};


class c64gs_state : public c64c_state
{
public:
	c64gs_state(const machine_config &mconfig, device_type type, const char *tag)
		: c64c_state(mconfig, type, tag)
	{ }

	uint8_t cpu_r();
	void cpu_w(uint8_t data);

	uint8_t cia1_pa_r();
	uint8_t cia1_pb_r();
	void pal_gs(machine_config &config);
};


class clipper_state : public c64_state
{
public:
	clipper_state(const machine_config &mconfig, device_type type, const char *tag)
		: c64_state(mconfig, type, tag)
	{ }

	void clipper(machine_config &config);
};


struct dasm_zeropage_data
{
	u8 addr;
	const char *name;
};

struct dasm_vector_data
{
	u16 addr;
	const char *name;
};

static const struct dasm_zeropage_data c64_zeropage_locations[] =
{
	{ 0x00, "D6510" }, { 0x01, "R6510" }, { 0x03, "ADRAY1" }, { 0x05, "ADRAY2" }, { 0x07, "CHARAC" }, { 0x08, "ENDCHR" }, { 0x09, "TRMPOS" }, { 0x0a, "VERCK" },
	{ 0x0b, "COUNT" }, { 0x0c, "DIMFLG" }, { 0x0d, "VALTYP" }, { 0x0e, "INTFLG" }, { 0x0f, "GARBFL" }, { 0x10, "SUBFLG" }, { 0x11, "INPFLG" }, { 0x12, "TANSGN" },
	{ 0x14, "LINNUM" }, { 0x16, "TEMPPT" }, { 0x17, "LASTPT" }, { 0x19, "TEMPST" }, { 0x22, "INDEX" }, { 0x26, "RESHO" }, { 0x2b, "TXTTAB" }, { 0x2d, "VARTAB" },
	{ 0x2f, "ARYTAB" }, { 0x31, "STREND" }, { 0x33, "FRETOP" }, { 0x35, "FRESPC" }, { 0x37, "MEMSIZ" }, { 0x39, "CURLIN" }, { 0x3b, "OLDLIN" }, { 0x3d, "OLDTXT" },
	{ 0x3f, "DATLIN" }, { 0x41, "DATPTR" }, { 0x43, "INPPTR" }, { 0x45, "VARNAM" }, { 0x47, "VARPNT" }, { 0x49, "FORPNT" }, { 0x61, "FACEXP" }, { 0x62, "FACHO" },
	{ 0x66, "FACSGN" }, { 0x67, "SGNFLG" }, { 0x68, "BITS" }, { 0x69, "ARGEXP" }, { 0x6a, "ARGHO" }, { 0x6e, "ARGSGN" }, { 0x6f, "ARISGN" }, { 0x70, "FACOV" },
	{ 0x71, "FBUFPT" }, { 0x73, "CHRGET" }, { 0x79, "CHRGOT" }, { 0x7a, "TXTPTR" }, { 0x8b, "RNDX" }, { 0x90, "STATUS" }, { 0x91, "STKEY" }, { 0x92, "SVXT" },
	{ 0x93, "VERCK" }, { 0x94, "C3PO" }, { 0x95, "BSOUR" }, { 0x96, "SYNO" }, { 0x98, "LDTND" }, { 0x99, "DFLTN" }, { 0x9a, "DFLTO" }, { 0x9b, "PRTY" },
	{ 0x9c, "DPSW" }, { 0x9d, "MSGFLG" }, { 0x9e, "PTR1" }, { 0x9f, "PTR2" }, { 0xa0, "TIME" }, { 0xa5, "CNTDN" }, { 0xa6, "BUFPNT" }, { 0xa7, "INBIT" },
	{ 0xa8, "BITCI" }, { 0xa9, "RINONE" }, { 0xaa, "RIDATA" }, { 0xab, "RIPRTY" }, { 0xac, "SAL" }, { 0xae, "EAL" }, { 0xb0, "CMPO" }, { 0xb2, "TAPE1" },
	{ 0xb4, "BITTS" }, { 0xb5, "NXTBIT" }, { 0xb6, "RODATA" }, { 0xb7, "FNLEN" }, { 0xb8, "LA" }, { 0xb9, "SA" }, { 0xba, "FA" }, { 0xbb, "FNADR" },
	{ 0xbc, "ROPRTY" }, { 0xbe, "FSBLK" }, { 0xbf, "MYCH" }, { 0xc0, "CAS1" }, { 0xc1, "STAL" }, { 0xc3, "MEMUSS" }, { 0xc5, "LSTX" }, { 0xc6, "NDX" },
	{ 0xc7, "RVS" }, { 0xc8, "INDX" }, { 0xc9, "LXSP" }, { 0xcb, "SFDX" }, { 0xcc, "BLNSW" }, { 0xcd, "BLNCT" }, { 0xce, "GDBLN" }, { 0xcf, "BLNON" },
	{ 0xd0, "CRSW" }, { 0xd1, "PNT" }, { 0xd3, "PNTR" }, { 0xd4, "QTSW" }, { 0xd5, "LNMX" }, { 0xd6, "TBLX" }, { 0xd8, "INSRT" }, { 0xd9, "LDTB1" },
	{ 0xf3, "USER" }, { 0xf5, "KEYTAB" }, { 0xf7, "RIBUF" }, { 0xf9, "ROBUF" }, { 0xfb, "FREKZB" }, { 0xff, "BASZPT" }
};

static const struct dasm_vector_data cbm_kernal_vectors[] =
{
	{ 0xff81, "CINT" }, { 0xff84, "IOINT" }, { 0xff87, "RAMTAS" }, { 0xff8a, "RESTOR" }, { 0xff8d, "VECTOR" }, { 0xff90, "SETMSG" }, { 0xff93, "SECOND" }, { 0xff96, "TKSA" },
	{ 0xff99, "MEMTOP" }, { 0xff9c, "MEMBOT" }, { 0xff9f, "SCNKEY" }, { 0xffa2, "SETTMO" }, { 0xffa5, "ACPTR" }, { 0xffa8, "CIOUT" }, { 0xffab, "UNTALK" }, { 0xffae, "UNLSN" },
	{ 0xffb1, "LISTEN" }, { 0xffb4, "TALK" }, { 0xffb7, "READST" }, { 0xffba, "SETLFS" }, { 0xffbd, "SETNAM" }, { 0xffc3, "CLOSE" }, { 0xffc6, "CHKIN" }, { 0xffc9, "CHKOUT" },
	{ 0xffcc, "CLRCHN" }, { 0xffcf, "CHRIN" }, { 0xffd2, "CHROUT" }, { 0xffd5, "LOAD" }, { 0xffd8, "SAVE" }, { 0xffdb, "SETTIM" }, { 0xffde, "RDTIM" }, { 0xffe1, "STOP" },
	{ 0xffe4, "GETIN" }, { 0xffe7, "CLALL" }, { 0xffea, "UDTIM" }, { 0xffed, "SCREEN" }, { 0xfff0, "PLOT" }, { 0xfff3, "IOBASE" }
};

offs_t c64_state::dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	switch (opcodes.r8(pc))
	{
	case 0x20: return dasm_vector(stream, pc, opcodes, "jsr %s");
	case 0x4c: return dasm_vector(stream, pc, opcodes, "jmp %s");
	case 0x6c: return dasm_zeropage_vector(stream, pc, opcodes, "jmp (%s)");
	case 0x65: return dasm_zeropage(stream, pc, opcodes, "adc %s");
	case 0x75: return dasm_zeropage(stream, pc, opcodes, "adc %s, x");
	case 0x61: return dasm_zeropage(stream, pc, opcodes, "adc (%s, x)");
	case 0x71: return dasm_zeropage(stream, pc, opcodes, "adc (%s), y");
	case 0x25: return dasm_zeropage(stream, pc, opcodes, "and %s");
	case 0x35: return dasm_zeropage(stream, pc, opcodes, "and %s, x");
	case 0x21: return dasm_zeropage(stream, pc, opcodes, "and (%s, x)");
	case 0x31: return dasm_zeropage(stream, pc, opcodes, "and (%s), y");
	case 0x06: return dasm_zeropage(stream, pc, opcodes, "asl %s");
	case 0x16: return dasm_zeropage(stream, pc, opcodes, "asl %s, x");
	case 0x24: return dasm_zeropage(stream, pc, opcodes, "bit %s");
	case 0xc5: return dasm_zeropage(stream, pc, opcodes, "cmp %s");
	case 0xd5: return dasm_zeropage(stream, pc, opcodes, "cmp %s, x");
	case 0xc1: return dasm_zeropage(stream, pc, opcodes, "cmp (%s, x)");
	case 0xd1: return dasm_zeropage(stream, pc, opcodes, "cmp (%s), y");
	case 0xe4: return dasm_zeropage(stream, pc, opcodes, "cpx %s");
	case 0xc4: return dasm_zeropage(stream, pc, opcodes, "cpy %s");
	case 0xc6: return dasm_zeropage(stream, pc, opcodes, "dec %s");
	case 0xd6: return dasm_zeropage(stream, pc, opcodes, "dec %s, x");
	case 0x45: return dasm_zeropage(stream, pc, opcodes, "eor %s");
	case 0x55: return dasm_zeropage(stream, pc, opcodes, "eor %s, x");
	case 0x41: return dasm_zeropage(stream, pc, opcodes, "eor (%s, x)");
	case 0x51: return dasm_zeropage(stream, pc, opcodes, "eor (%s), y");
	case 0xe6: return dasm_zeropage(stream, pc, opcodes, "inc %s");
	case 0xf6: return dasm_zeropage(stream, pc, opcodes, "inc %s, x");
	case 0xa5: return dasm_zeropage(stream, pc, opcodes, "lda %s");
	case 0xb5: return dasm_zeropage(stream, pc, opcodes, "lda %s, x");
	case 0xa1: return dasm_zeropage(stream, pc, opcodes, "lda (%s, x)");
	case 0xb1: return dasm_zeropage(stream, pc, opcodes, "lda (%s), y");
	case 0xa6: return dasm_zeropage(stream, pc, opcodes, "ldx %s");
	case 0xb6: return dasm_zeropage(stream, pc, opcodes, "ldx %s, y");
	case 0xa4: return dasm_zeropage(stream, pc, opcodes, "ldy %s");
	case 0xb4: return dasm_zeropage(stream, pc, opcodes, "ldy %s, x");
	case 0x46: return dasm_zeropage(stream, pc, opcodes, "lsr %s");
	case 0x56: return dasm_zeropage(stream, pc, opcodes, "lsr %s, x");
	case 0x05: return dasm_zeropage(stream, pc, opcodes, "ora %s");
	case 0x15: return dasm_zeropage(stream, pc, opcodes, "ora %s, x");
	case 0x01: return dasm_zeropage(stream, pc, opcodes, "ora (%s, x)");
	case 0x11: return dasm_zeropage(stream, pc, opcodes, "ora (%s), y");
	case 0x26: return dasm_zeropage(stream, pc, opcodes, "rol %s");
	case 0x36: return dasm_zeropage(stream, pc, opcodes, "rol %s, x");
	case 0x66: return dasm_zeropage(stream, pc, opcodes, "ror %s");
	case 0x76: return dasm_zeropage(stream, pc, opcodes, "ror %s, x");
	case 0xe5: return dasm_zeropage(stream, pc, opcodes, "sbc %s");
	case 0xf5: return dasm_zeropage(stream, pc, opcodes, "sbc %s, x");
	case 0xe1: return dasm_zeropage(stream, pc, opcodes, "sbc (%s, x)");
	case 0xf1: return dasm_zeropage(stream, pc, opcodes, "sbc (%s), y");
	case 0x85: return dasm_zeropage(stream, pc, opcodes, "sta %s");
	case 0x95: return dasm_zeropage(stream, pc, opcodes, "sta %s, x");
	case 0x81: return dasm_zeropage(stream, pc, opcodes, "sta (%s, x)");
	case 0x91: return dasm_zeropage(stream, pc, opcodes, "sta (%s), y");
	case 0x86: return dasm_zeropage(stream, pc, opcodes, "stx %s");
	case 0x96: return dasm_zeropage(stream, pc, opcodes, "stx (%s), y");
	case 0x84: return dasm_zeropage(stream, pc, opcodes, "sty %s");
	case 0x94: return dasm_zeropage(stream, pc, opcodes, "sty %s, x");
	}

	return 0;
}

offs_t c64_state::dasm_zeropage(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname)
{
	int item = 0;
	u8 operand = opcodes.r8(pc+1);

	while (c64_zeropage_locations[item].addr != 0xff)
	{
		if (c64_zeropage_locations[item].addr == operand)
		{
			std::ostringstream buffer;
			util::stream_format(buffer, opname, c64_zeropage_locations[item].name);
			stream << buffer.str();
			return 2 | util::disasm_interface::SUPPORTED;
		}
		item++;
	}

	return 0;
}

offs_t c64_state::dasm_vector(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname)
{
	int item = 0;
	u16 operand = opcodes.r16(pc+1);

	while (cbm_kernal_vectors[item].addr != 0xfff3)
	{
		if (cbm_kernal_vectors[item].addr == operand)
		{
			std::ostringstream buffer;
			util::stream_format(buffer, opname, cbm_kernal_vectors[item].name);
			stream << buffer.str();
			return 3 | util::disasm_interface::SUPPORTED;
		}
		item++;
	}

	return 0;
}

offs_t c64_state::dasm_zeropage_vector(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname)
{
	int item = 0;
	u16 operand = opcodes.r16(pc+1);

	if (operand < 0x100)
	{
		while (c64_zeropage_locations[item].addr != 0xff)
		{
			if (c64_zeropage_locations[item].addr == operand)
			{
				std::ostringstream buffer;
				util::stream_format(buffer, opname, c64_zeropage_locations[item].name);
				stream << buffer.str();
				return 3 | util::disasm_interface::SUPPORTED;
			}
			item++;
		}
	}

	return 0;
}


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define A15 BIT(offset, 15)
#define A14 BIT(offset, 14)
#define A13 BIT(offset, 13)
#define A12 BIT(offset, 12)
#define VA13 BIT(va, 13)
#define VA12 BIT(va, 12)

enum
{
	PLA_OUT_CASRAM = 0,
	PLA_OUT_BASIC  = 1,
	PLA_OUT_KERNAL = 2,
	PLA_OUT_CHAROM = 3,
	PLA_OUT_GRW    = 4,
	PLA_OUT_IO     = 5,
	PLA_OUT_ROML   = 6,
	PLA_OUT_ROMH   = 7
};


QUICKLOAD_LOAD_MEMBER(c64_state::quickload_c64)
{
	return general_cbm_loadsnap(image, m_maincpu->space(AS_PROGRAM), 0, cbm_quick_sethiaddress);
}


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void c64_state::check_interrupts()
{
	//int irq = m_cia1_irq || m_vic_irq || m_exp_irq;
	//int nmi = m_cia2_irq || !m_restore || m_exp_nmi;
	//int rdy = m_exp_dma && m_vic_ba;
}



//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read_pla -
//-------------------------------------------------

int c64_state::read_pla(offs_t offset, offs_t va, int rw, int aec, int ba)
{
	//int ba = m_vic->ba_r();
	//int aec = !m_vic->aec_r();
	int sphi2 = m_vic->phi0_r();
	int game = m_exp->game_r(offset, sphi2, ba, rw, m_loram, m_hiram);
	int exrom = m_exp->exrom_r(offset, sphi2, ba, rw, m_loram, m_hiram);
	int cas = 0;

	uint32_t input = VA12 << 15 | VA13 << 14 | game << 13 | exrom << 12 | rw << 11 | aec << 10 | ba << 9 | A12 << 8 |
		A13 << 7 | A14 << 6 | A15 << 5 | m_va14 << 4 | m_charen << 3 | m_hiram << 2 | m_loram << 1 | cas;

	return m_pla->read(input);
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

uint8_t c64_state::read_memory(offs_t offset, offs_t va, int aec, int ba)
{
	int rw = 1;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	int plaout = read_pla(offset, va, rw, !aec, ba);

	uint8_t data = 0xff;

	if (!aec)
	{
		data = m_vic->bus_r();
	}

	if (!BIT(plaout, PLA_OUT_CASRAM))
	{
		if (aec)
		{
			data = m_ram->pointer()[offset];
		}
		else
		{
			data = m_ram->pointer()[(!m_va15 << 15) | (!m_va14 << 14) | va];
		}
	}
	if (!BIT(plaout, PLA_OUT_BASIC))
	{
		data = m_basic[offset & 0x1fff];
	}
	if (!BIT(plaout, PLA_OUT_KERNAL))
	{
		data = m_kernal[offset & 0x1fff];
	}
	if (!BIT(plaout, PLA_OUT_CHAROM))
	{
		data = m_charom[offset & 0xfff];
	}
	if (!BIT(plaout, PLA_OUT_IO))
	{
		switch ((offset >> 8) & 0x0f)
		{
		case 0:
		case 1:
		case 2:
		case 3: // VIC
			data = m_vic->read(offset & 0x3f);
			break;

		case 4:
		case 5:
		case 6:
		case 7: // SID
			data = m_sid->read(offset & 0x1f);
			break;

		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb: // COLOR
			data = m_color_ram[offset & 0x3ff] & 0x0f;
			break;

		case 0xc: // CIA1
			data = m_cia1->read(offset & 0x0f);
			break;

		case 0xd: // CIA2
			data = m_cia2->read(offset & 0x0f);
			break;

		case 0xe: // I/O1
			io1 = 0;
			break;

		case 0xf: // I/O2
			io2 = 0;
			break;
		}
	}

	int roml = BIT(plaout, PLA_OUT_ROML);
	int romh = BIT(plaout, PLA_OUT_ROMH);
	return m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  write_memory -
//-------------------------------------------------

void c64_state::write_memory(offs_t offset, uint8_t data, int aec, int ba)
{
	int rw = 0;
	offs_t va = 0;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	int plaout = read_pla(offset, va, rw, !aec, ba);

	if (offset < 0x0002)
	{
		// write to internal CPU register
		data = m_vic->bus_r();
	}

	if (!BIT(plaout, PLA_OUT_CASRAM))
	{
		m_ram->pointer()[offset] = data;
	}
	if (!BIT(plaout, PLA_OUT_IO))
	{
		switch ((offset >> 8) & 0x0f)
		{
		case 0:
		case 1:
		case 2:
		case 3: // VIC
			m_vic->write(offset & 0x3f, data);
			break;

		case 4:
		case 5:
		case 6:
		case 7: // SID
			m_sid->write(offset & 0x1f, data);
			break;

		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb: // COLOR
			if (!BIT(plaout, PLA_OUT_GRW)) m_color_ram[offset & 0x3ff] = data & 0x0f;
			break;

		case 0xc: // CIA1
			m_cia1->write(offset & 0x0f, data);
			break;

		case 0xd: // CIA2
			m_cia2->write(offset & 0x0f, data);
			break;

		case 0xe: // I/O1
			io1 = 0;
			break;

		case 0xf: // I/O2
			io2 = 0;
			break;
		}
	}

	int roml = BIT(plaout, PLA_OUT_ROML);
	int romh = BIT(plaout, PLA_OUT_ROMH);
	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t c64_state::read(offs_t offset)
{
	int aec = 1, ba = 1;

	// VIC address bus is floating
	offs_t va = 0x3fff;

	return read_memory(offset, va, aec, ba);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void c64_state::write(offs_t offset, uint8_t data)
{
	int aec = 1, ba = 1;

	write_memory(offset, data, aec, ba);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

uint8_t c64_state::vic_videoram_r(offs_t offset)
{
	int aec = m_vic->aec_r(), ba = m_vic->ba_r();
	offs_t va = offset;

	// A15/A14 are not connected to VIC so they are floating
	//offset |= 0xc000;

	return read_memory(offset, va, aec, ba);
}


//-------------------------------------------------
//  vic_colorram_r -
//-------------------------------------------------

uint8_t c64_state::vic_colorram_r(offs_t offset)
{
	uint8_t data;

	if (m_vic->aec_r())
	{
		// TODO low nibble of last opcode
		data = 0x0f;
	}
	else
	{
		data = m_color_ram[offset] & 0x0f;
	}

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( c64_mem )
//-------------------------------------------------

void c64_state::c64_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c64_state::read), FUNC(c64_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

void c64_state::vic_videoram_map(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(c64_state::vic_videoram_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

void c64_state::vic_colorram_map(address_map &map)
{
	map(0x000, 0x3ff).r(FUNC(c64_state::vic_colorram_r));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( c64 )
//-------------------------------------------------

void c64_state::write_restore(int state)
{
	m_nmi->in_w<1>(!state);
}

static INPUT_PORTS_START( c64 )
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
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

	PORT_START( "RESTORE" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_PRTSCR) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_state, write_restore)

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c64sw )
//-------------------------------------------------

static INPUT_PORTS_START( c64sw )
	PORT_INCLUDE( c64 )

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                            PORT_CHAR('=')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR('-')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR(':') PORT_CHAR('*')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c64gs )
//-------------------------------------------------

static INPUT_PORTS_START( c64gs )
	// no keyboard
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( clipper )
//-------------------------------------------------

static INPUT_PORTS_START( clipper )
	PORT_INCLUDE( c64 )
	// TODO extra keys
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

uint8_t c64_state::sid_potx_r()
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

uint8_t c64_state::sid_poty_r()
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

uint8_t c64_state::cia1_pa_r()
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

void c64_state::cia1_pa_w(uint8_t data)
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

	m_joy2->joy_w(data & 0x1f);
}

uint8_t c64_state::cia1_pb_r()
{
	/*

	    bit     description

	    PB0     ROW0, JOY A0
	    PB1     ROW1, JOY A1
	    PB2     ROW2, JOY A2
	    PB3     ROW3, JOY A3
	    PB4     ROW4, BTNA, _LP
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

	return data;
}

void c64_state::cia1_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     ROW0, JOY A0
	    PB1     ROW1, JOY A1
	    PB2     ROW2, JOY A2
	    PB3     ROW3, JOY A3
	    PB4     ROW4, BTNA, _LP
	    PB5     ROW5
	    PB6     ROW6
	    PB7     ROW7

	*/

	m_joy1->joy_w(data & 0x1f);

	m_vic->lp_w(BIT(data, 4));
}

uint8_t c64gs_state::cia1_pa_r()
{
	/*

	    bit     description

	    PA0     JOY B0
	    PA1     JOY B1
	    PA2     JOY B2
	    PA3     JOY B3
	    PA4     BTNB
	    PA5
	    PA6
	    PA7

	*/

	uint8_t data = 0xff;

	// joystick
	uint8_t joy_b = m_joy2->read_joy();

	data &= (0xf0 | (joy_b & 0x0f));
	data &= ~(!BIT(joy_b, 5) << 4);

	return data;
}

uint8_t c64gs_state::cia1_pb_r()
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

	uint8_t data = 0xff;

	// joystick
	uint8_t joy_a = m_joy1->read_joy();

	data &= (0xf0 | (joy_a & 0x0f));
	data &= ~(!BIT(joy_a, 5) << 4);

	return data;
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia2_intf )
//-------------------------------------------------

uint8_t c64_state::cia2_pa_r()
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

void c64_state::cia2_pa_w(uint8_t data)
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
	m_iec->host_data_w(!BIT(data, 5));
}

uint8_t c64_state::cia2_pb_r()
{
	return m_user_pb;
}

void c64_state::cia2_pb_w(uint8_t data)
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

uint8_t c64_state::cpu_r()
{
	/*

	    bit     description

	    P0      1
	    P1      1
	    P2      1
	    P3
	    P4      CASS SENS
	    P5      0

	*/

	uint8_t data = 0x07;

	data |= m_cassette->sense_r() << 4;

	return data;
}

void c64_state::cpu_w(uint8_t data)
{
	/*

	    bit     description

	    P0      LORAM
	    P1      HIRAM
	    P2      CHAREN
	    P3      CASS WRT
	    P4
	    P5      CASS MOTOR

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
//  M6510_INTERFACE( sx64_cpu_intf )
//-------------------------------------------------

uint8_t sx64_state::cpu_r()
{
	/*

	    bit     description

	    P0      1
	    P1      1
	    P2      1
	    P3
	    P4
	    P5

	*/

	return 0x07;
}

void sx64_state::cpu_w(uint8_t data)
{
	/*

	    bit     description

	    P0      LORAM
	    P1      HIRAM
	    P2      CHAREN
	    P3
	    P4
	    P5

	*/

	// memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);
}


//-------------------------------------------------
//  M6510_INTERFACE( c64gs_cpu_intf )
//-------------------------------------------------

uint8_t c64gs_state::cpu_r()
{
	/*

	    bit     description

	    P0      1
	    P1      1
	    P2      1
	    P3
	    P4
	    P5

	*/

	return 0x07;
}

void c64gs_state::cpu_w(uint8_t data)
{
	/*

	    bit     description

	    P0      LORAM
	    P1      HIRAM
	    P2      CHAREN
	    P3
	    P4
	    P5

	*/

	// memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);
}


//-------------------------------------------------
//  C64_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

void c64_state::exp_dma_w(int state)
{
	if (m_exp_dma != state)
	{
		m_exp_dma = state;

		m_maincpu->set_input_line(INPUT_LINE_HALT, m_exp_dma);
	}
}

void c64_state::exp_reset_w(int state)
{
	if (!state)
	{
		machine_reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( sx1541_iec_devices )
//-------------------------------------------------

void sx1541_iec_devices(device_slot_interface &device)
{
	device.option_add("sx1541", SX1541);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( c64 )
//-------------------------------------------------

void c64_state::machine_start()
{
	// get pointers to ROMs
	if (memregion("basic") != nullptr)
	{
		m_basic = memregion("basic")->base();
		m_kernal = memregion("kernal")->base();
	}
	else
	{
		m_basic = memregion("kernal")->base();
		m_kernal = &m_basic[0x2000];
	}
	m_charom = memregion("charom")->base();

	// initialize memory
	uint8_t data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	// state saving
	save_item(NAME(m_loram));
	save_item(NAME(m_hiram));
	save_item(NAME(m_charen));
	save_item(NAME(m_va14));
	save_item(NAME(m_va15));
	save_item(NAME(m_exp_dma));
	save_item(NAME(m_cass_rd));
	save_item(NAME(m_iec_srq));
	save_item(NAME(m_user_pa2));
	save_item(NAME(m_user_pb));
}


void c64_state::machine_reset()
{
	m_maincpu->reset();

	m_vic->reset();
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

void c64_state::ntsc(machine_config &config)
{
	// basic hardware
	M6510(config, m_maincpu, XTAL(14'318'181)/14);
	m_maincpu->set_addrmap(AS_PROGRAM, &c64_state::c64_mem);
	m_maincpu->read_callback().set(FUNC(c64_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(c64_state::cpu_w));
	m_maincpu->set_pulls(0x17, 0xc8);
	m_maincpu->set_dasm_override(FUNC(c64_state::dasm_override));
	config.set_perfect_quantum(m_maincpu);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irq"));
	irq.output_handler().set_inputline(m_maincpu, m6510_device::IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, m_nmi);
	m_nmi->output_handler().set_inputline(m_maincpu, m6510_device::NMI_LINE);

	// video hardware
	mos6567_device &mos6567(MOS6567(config, MOS6567_TAG, XTAL(14'318'181)/14));
	mos6567.set_cpu(m_maincpu);
	mos6567.irq_callback().set("irq", FUNC(input_merger_device::in_w<1>));
	mos6567.set_screen(SCREEN_TAG);
	mos6567.set_addrmap(0, &c64_state::vic_videoram_map);
	mos6567.set_addrmap(1, &c64_state::vic_colorram_map);

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(VIC6567_VRETRACERATE);
	screen.set_size(VIC6567_COLUMNS, VIC6567_LINES);
	screen.set_visarea(0, VIC6567_VISIBLECOLUMNS - 1, 0, VIC6567_VISIBLELINES - 1);
	screen.set_screen_update(MOS6567_TAG, FUNC(mos6567_device::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, XTAL(14'318'181)/14);
	m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	PLS100(config, m_pla);

	MOS6526(config, m_cia1, XTAL(14'318'181)/14);
	m_cia1->set_tod_clock(60);
	m_cia1->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<0>));
	m_cia1->cnt_wr_callback().set(m_user, FUNC(pet_user_port_device::write_4));
	m_cia1->sp_wr_callback().set(m_user, FUNC(pet_user_port_device::write_5));
	m_cia1->pa_rd_callback().set(FUNC(c64_state::cia1_pa_r));
	m_cia1->pb_rd_callback().set(FUNC(c64_state::cia1_pb_r));
	m_cia1->pb_wr_callback().set(FUNC(c64_state::cia1_pb_w));

	MOS6526(config, m_cia2, XTAL(14'318'181)/14);
	m_cia2->set_tod_clock(60);
	m_cia2->irq_wr_callback().set(m_nmi, FUNC(input_merger_device::in_w<0>));
	m_cia2->cnt_wr_callback().set(m_user, FUNC(pet_user_port_device::write_6));
	m_cia2->sp_wr_callback().set(m_user, FUNC(pet_user_port_device::write_7));
	m_cia2->pa_rd_callback().set(FUNC(c64_state::cia2_pa_r));
	m_cia2->pa_wr_callback().set(FUNC(c64_state::cia2_pa_w));
	m_cia2->pb_rd_callback().set(FUNC(c64_state::cia2_pb_r));
	m_cia2->pb_wr_callback().set(FUNC(c64_state::cia2_pb_w));
	m_cia2->pc_wr_callback().set(m_user, FUNC(pet_user_port_device::write_8));

	PET_DATASSETTE_PORT(config, m_cassette, cbm_datassette_devices, "c1530");
	m_cassette->read_handler().set(FUNC(c64_state::cass_rd_w));

	cbm_iec_slot_device::add(config, m_iec, "c1541");
	m_iec->srq_callback().set(FUNC(c64_state::iec_srq_w));
	m_iec->data_callback().set(m_user, FUNC(pet_user_port_device::write_9));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	m_joy1->trigger_wr_callback().set(MOS6567_TAG, FUNC(mos6567_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

	C64_EXPANSION_SLOT(config, m_exp, XTAL(14'318'181)/14, c64_expansion_cards, nullptr);
	m_exp->irq_callback().set("irq", FUNC(input_merger_device::in_w<2>));
	m_exp->nmi_callback().set(m_nmi, FUNC(input_merger_device::in_w<2>));
	m_exp->reset_callback().set(FUNC(c64_state::exp_reset_w));
	m_exp->cd_input_callback().set(FUNC(c64_state::read));
	m_exp->cd_output_callback().set(FUNC(c64_state::write));
	m_exp->dma_callback().set(FUNC(c64_state::exp_dma_w));

	PET_USER_PORT(config, m_user, c64_user_port_cards, nullptr);
	m_user->p3_handler().set(FUNC(c64_state::exp_reset_w));
	m_user->p4_handler().set(m_cia1, FUNC(mos6526_device::cnt_w));
	m_user->p5_handler().set(m_cia1, FUNC(mos6526_device::sp_w));
	m_user->p6_handler().set(m_cia2, FUNC(mos6526_device::cnt_w));
	m_user->p7_handler().set(m_cia2, FUNC(mos6526_device::sp_w));
	m_user->p9_handler().set(m_iec, FUNC(cbm_iec_device::host_atn_w));
	m_user->pb_handler().set(m_cia2, FUNC(mos6526_device::flag_w));
	m_user->pc_handler().set(FUNC(c64_state::write_user_pb0));
	m_user->pd_handler().set(FUNC(c64_state::write_user_pb1));
	m_user->pe_handler().set(FUNC(c64_state::write_user_pb2));
	m_user->pf_handler().set(FUNC(c64_state::write_user_pb3));
	m_user->ph_handler().set(FUNC(c64_state::write_user_pb4));
	m_user->pj_handler().set(FUNC(c64_state::write_user_pb5));
	m_user->pk_handler().set(FUNC(c64_state::write_user_pb6));
	m_user->pl_handler().set(FUNC(c64_state::write_user_pb7));
	m_user->pm_handler().set(FUNC(c64_state::write_user_pa2));

	QUICKLOAD(config, "quickload", "p00,prg,t64", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(c64_state::quickload_c64));

	// software list
	SOFTWARE_LIST(config, "cart_list_vic10").set_original("vic10").set_filter("NTSC");
	SOFTWARE_LIST(config, "cart_list_c64").set_original("c64_cart").set_filter("NTSC");
	SOFTWARE_LIST(config, "cass_list").set_original("c64_cass").set_filter("NTSC");
	// disk softlist split into originals and misc (homebrew and cracks)
	SOFTWARE_LIST(config, "flop525_orig").set_original("c64_flop_orig").set_filter("NTSC");
	SOFTWARE_LIST(config, "flop525_misc").set_original("c64_flop_misc").set_filter("NTSC");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");
}


//-------------------------------------------------
//  machine_config( pet64 )
//-------------------------------------------------

void c64_state::pet64(machine_config &config)
{
	ntsc(config);
	// TODO monochrome green palette
}


//-------------------------------------------------
//  machine_config( ntsc_sx )
//-------------------------------------------------

void sx64_state::ntsc_sx(machine_config &config)
{
	ntsc(config);

	// basic hardware
	m_maincpu->read_callback().set(FUNC(sx64_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(sx64_state::cpu_w));
	m_maincpu->set_pulls(0x07, 0xc0);

	// devices
	CBM_IEC_SLOT(config.replace(), "iec8", 8, sx1541_iec_devices, "sx1541");
}


//-------------------------------------------------
//  machine_config( ntsc_dx )
//-------------------------------------------------

void sx64_state::ntsc_dx(machine_config &config)
{
	ntsc_sx(config);

	// devices
	CBM_IEC_SLOT(config.replace(), "iec9", 9, sx1541_iec_devices, "sx1541");
}


//-------------------------------------------------
//  machine_config( ntsc_c )
//-------------------------------------------------

void c64c_state::ntsc_c(machine_config &config)
{
	ntsc(config);
	MOS8580(config.replace(), m_sid, XTAL(14'318'181)/14);
	m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);
}


//-------------------------------------------------
//  machine_config( pal )
//-------------------------------------------------

void c64_state::pal(machine_config &config)
{
	// basic hardware
	M6510(config, m_maincpu, XTAL(17'734'472)/18);
	m_maincpu->set_addrmap(AS_PROGRAM, &c64_state::c64_mem);
	m_maincpu->read_callback().set(FUNC(c64_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(c64_state::cpu_w));
	m_maincpu->set_pulls(0x17, 0xc8);
	m_maincpu->set_dasm_override(FUNC(c64_state::dasm_override));
	config.set_perfect_quantum(m_maincpu);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irq"));
	irq.output_handler().set_inputline(m_maincpu, m6510_device::IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, m_nmi);
	m_nmi->output_handler().set_inputline(m_maincpu, m6510_device::NMI_LINE);

	// video hardware
	mos6569_device &mos6569(MOS6569(config, MOS6569_TAG, XTAL(17'734'472)/18));
	mos6569.set_cpu(m_maincpu);
	mos6569.irq_callback().set("irq", FUNC(input_merger_device::in_w<1>));
	mos6569.set_screen(SCREEN_TAG);
	mos6569.set_addrmap(0, &c64_state::vic_videoram_map);
	mos6569.set_addrmap(1, &c64_state::vic_colorram_map);

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(VIC6569_VRETRACERATE);
	screen.set_size(VIC6569_COLUMNS, VIC6569_LINES);
	screen.set_visarea(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1);
	screen.set_screen_update(MOS6569_TAG, FUNC(mos6569_device::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, XTAL(17'734'472)/18);
	m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	PLS100(config, m_pla);

	MOS6526(config, m_cia1, XTAL(17'734'472)/18);
	m_cia1->set_tod_clock(50);
	m_cia1->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<0>));
	m_cia1->cnt_wr_callback().set(m_user, FUNC(pet_user_port_device::write_4));
	m_cia1->sp_wr_callback().set(m_user, FUNC(pet_user_port_device::write_5));
	m_cia1->pa_rd_callback().set(FUNC(c64_state::cia1_pa_r));
	m_cia1->pb_rd_callback().set(FUNC(c64_state::cia1_pb_r));
	m_cia1->pb_wr_callback().set(FUNC(c64_state::cia1_pb_w));

	MOS6526(config, m_cia2, XTAL(17'734'472)/18);
	m_cia2->set_tod_clock(50);
	m_cia2->irq_wr_callback().set(m_nmi, FUNC(input_merger_device::in_w<0>));
	m_cia2->cnt_wr_callback().set(m_user, FUNC(pet_user_port_device::write_6));
	m_cia2->sp_wr_callback().set(m_user, FUNC(pet_user_port_device::write_7));
	m_cia2->pa_rd_callback().set(FUNC(c64_state::cia2_pa_r));
	m_cia2->pa_wr_callback().set(FUNC(c64_state::cia2_pa_w));
	m_cia2->pb_rd_callback().set(FUNC(c64_state::cia2_pb_r));
	m_cia2->pb_wr_callback().set(FUNC(c64_state::cia2_pb_w));
	m_cia2->pc_wr_callback().set(m_user, FUNC(pet_user_port_device::write_8));

	PET_DATASSETTE_PORT(config, m_cassette, cbm_datassette_devices, "c1530");
	m_cassette->read_handler().set(FUNC(c64_state::cass_rd_w));

	cbm_iec_slot_device::add(config, m_iec, "c1541");
	m_iec->srq_callback().set(FUNC(c64_state::iec_srq_w));
	m_iec->data_callback().set(m_user, FUNC(pet_user_port_device::write_9));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	m_joy1->trigger_wr_callback().set(MOS6569_TAG, FUNC(mos6569_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

	C64_EXPANSION_SLOT(config, m_exp, XTAL(17'734'472)/18, c64_expansion_cards, nullptr);
	m_exp->irq_callback().set("irq", FUNC(input_merger_device::in_w<2>));
	m_exp->nmi_callback().set(m_nmi, FUNC(input_merger_device::in_w<2>));
	m_exp->reset_callback().set(FUNC(c64_state::exp_reset_w));
	m_exp->cd_input_callback().set(FUNC(c64_state::read));
	m_exp->cd_output_callback().set(FUNC(c64_state::write));
	m_exp->dma_callback().set(FUNC(c64_state::exp_dma_w));

	PET_USER_PORT(config, m_user, c64_user_port_cards, nullptr);
	m_user->p3_handler().set(FUNC(c64_state::exp_reset_w));
	m_user->p4_handler().set(m_cia1, FUNC(mos6526_device::cnt_w));
	m_user->p5_handler().set(m_cia1, FUNC(mos6526_device::sp_w));
	m_user->p6_handler().set(m_cia2, FUNC(mos6526_device::cnt_w));
	m_user->p7_handler().set(m_cia2, FUNC(mos6526_device::sp_w));
	m_user->p9_handler().set(m_iec, FUNC(cbm_iec_device::host_atn_w));
	m_user->pb_handler().set(m_cia2, FUNC(mos6526_device::flag_w));
	m_user->pc_handler().set(FUNC(c64_state::write_user_pb0));
	m_user->pd_handler().set(FUNC(c64_state::write_user_pb1));
	m_user->pe_handler().set(FUNC(c64_state::write_user_pb2));
	m_user->pf_handler().set(FUNC(c64_state::write_user_pb3));
	m_user->ph_handler().set(FUNC(c64_state::write_user_pb4));
	m_user->pj_handler().set(FUNC(c64_state::write_user_pb5));
	m_user->pk_handler().set(FUNC(c64_state::write_user_pb6));
	m_user->pl_handler().set(FUNC(c64_state::write_user_pb7));
	m_user->pm_handler().set(FUNC(c64_state::write_user_pa2));

	QUICKLOAD(config, "quickload", "p00,prg,t64", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(c64_state::quickload_c64));

	// software list
	SOFTWARE_LIST(config, "cart_list_vic10").set_original("vic10").set_filter("PAL");
	SOFTWARE_LIST(config, "cart_list_c64").set_original("c64_cart").set_filter("PAL");
	SOFTWARE_LIST(config, "cass_list").set_original("c64_cass").set_filter("PAL");
	// disk softlist split into originals and misc (homebrew and cracks)
	SOFTWARE_LIST(config, "flop525_orig").set_original("c64_flop_orig").set_filter("PAL");
	SOFTWARE_LIST(config, "flop525_misc").set_original("c64_flop_misc").set_filter("PAL");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");
}


//-------------------------------------------------
//  machine_config( pal_sx )
//-------------------------------------------------

void sx64_state::pal_sx(machine_config &config)
{
	pal(config);

	// basic hardware
	m_maincpu->read_callback().set(FUNC(sx64_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(sx64_state::cpu_w));
	m_maincpu->set_pulls(0x07, 0xc0);

	// devices
	CBM_IEC_SLOT(config.replace(), "iec8", 8, sx1541_iec_devices, "sx1541");
}


//-------------------------------------------------
//  machine_config( pal_c )
//-------------------------------------------------

void c64c_state::pal_c(machine_config &config)
{
	pal(config);
	MOS8580(config.replace(), m_sid, XTAL(17'734'472)/18);
	m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);
}


//-------------------------------------------------
//  machine_config( pal_gs )
//-------------------------------------------------

void c64gs_state::pal_gs(machine_config &config)
{
	// basic hardware
	M6510(config, m_maincpu, XTAL(17'734'472)/18);
	m_maincpu->set_addrmap(AS_PROGRAM, &c64gs_state::c64_mem);
	m_maincpu->read_callback().set(FUNC(c64gs_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(c64gs_state::cpu_w));
	m_maincpu->set_pulls(0x07, 0xc0);
	m_maincpu->set_dasm_override(FUNC(c64_state::dasm_override));
	config.set_perfect_quantum(m_maincpu);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irq"));
	irq.output_handler().set_inputline(m_maincpu, m6510_device::IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, m_nmi);
	m_nmi->output_handler().set_inputline(m_maincpu, m6510_device::NMI_LINE);

	// video hardware
	mos8565_device &mos8565(MOS8565(config, MOS6569_TAG, XTAL(17'734'472)/18));
	mos8565.set_cpu(m_maincpu);
	mos8565.irq_callback().set("irq", FUNC(input_merger_device::in_w<1>));
	mos8565.set_screen(SCREEN_TAG);
	mos8565.set_addrmap(0, &c64_state::vic_videoram_map);
	mos8565.set_addrmap(1, &c64_state::vic_colorram_map);

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(VIC6569_VRETRACERATE);
	screen.set_size(VIC6569_COLUMNS, VIC6569_LINES);
	screen.set_visarea(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1);
	screen.set_screen_update(MOS6569_TAG, FUNC(mos8565_device::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MOS8580(config, m_sid, XTAL(17'734'472)/18);
	m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	PLS100(config, m_pla);

	MOS6526(config, m_cia1, XTAL(17'734'472)/18);
	m_cia1->set_tod_clock(50);
	m_cia1->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<0>));
	m_cia1->cnt_wr_callback().set(m_user, FUNC(pet_user_port_device::write_4));
	m_cia1->sp_wr_callback().set(m_user, FUNC(pet_user_port_device::write_5));
	m_cia1->pa_rd_callback().set(FUNC(c64gs_state::cia1_pa_r));
	m_cia1->pa_wr_callback().set(FUNC(c64_state::cia1_pa_w));
	m_cia1->pb_rd_callback().set(FUNC(c64gs_state::cia1_pb_r));
	m_cia1->pb_wr_callback().set(FUNC(c64_state::cia1_pb_w));

	MOS6526(config, m_cia2, XTAL(17'734'472)/18);
	m_cia2->set_tod_clock(50);
	m_cia2->irq_wr_callback().set(m_nmi, FUNC(input_merger_device::in_w<0>));
	m_cia2->cnt_wr_callback().set(m_user, FUNC(pet_user_port_device::write_6));
	m_cia2->sp_wr_callback().set(m_user, FUNC(pet_user_port_device::write_7));
	m_cia2->pa_rd_callback().set(FUNC(c64_state::cia2_pa_r));
	m_cia2->pa_wr_callback().set(FUNC(c64_state::cia2_pa_w));
	m_cia2->pb_rd_callback().set(FUNC(c64_state::cia2_pb_r));
	m_cia2->pb_wr_callback().set(FUNC(c64_state::cia2_pb_w));
	m_cia2->pc_wr_callback().set(m_user, FUNC(pet_user_port_device::write_8));

	cbm_iec_slot_device::add(config, m_iec, nullptr);
	m_iec->srq_callback().set(m_cia1, FUNC(mos6526_device::flag_w));
	m_iec->data_callback().set(m_user, FUNC(pet_user_port_device::write_9));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	m_joy1->trigger_wr_callback().set(MOS6569_TAG, FUNC(mos6569_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

	C64_EXPANSION_SLOT(config, m_exp, XTAL(17'734'472)/18, c64_expansion_cards, nullptr);
	m_exp->irq_callback().set("irq", FUNC(input_merger_device::in_w<2>));
	m_exp->nmi_callback().set(m_nmi, FUNC(input_merger_device::in_w<2>));
	m_exp->reset_callback().set(FUNC(c64_state::exp_reset_w));
	m_exp->cd_input_callback().set(FUNC(c64_state::read));
	m_exp->cd_output_callback().set(FUNC(c64_state::write));
	m_exp->dma_callback().set(FUNC(c64_state::exp_dma_w));

	PET_USER_PORT(config, m_user, c64_user_port_cards, nullptr);
	m_user->p3_handler().set(FUNC(c64_state::exp_reset_w));
	m_user->p4_handler().set(m_cia1, FUNC(mos6526_device::cnt_w));
	m_user->p5_handler().set(m_cia1, FUNC(mos6526_device::sp_w));
	m_user->p6_handler().set(m_cia2, FUNC(mos6526_device::cnt_w));
	m_user->p7_handler().set(m_cia2, FUNC(mos6526_device::sp_w));
	m_user->p9_handler().set(m_iec, FUNC(cbm_iec_device::host_atn_w));
	m_user->pb_handler().set(m_cia2, FUNC(mos6526_device::flag_w));
	m_user->pc_handler().set(FUNC(c64_state::write_user_pb0));
	m_user->pd_handler().set(FUNC(c64_state::write_user_pb1));
	m_user->pe_handler().set(FUNC(c64_state::write_user_pb2));
	m_user->pf_handler().set(FUNC(c64_state::write_user_pb3));
	m_user->ph_handler().set(FUNC(c64_state::write_user_pb4));
	m_user->pj_handler().set(FUNC(c64_state::write_user_pb5));
	m_user->pk_handler().set(FUNC(c64_state::write_user_pb6));
	m_user->pl_handler().set(FUNC(c64_state::write_user_pb7));
	m_user->pm_handler().set(FUNC(c64_state::write_user_pa2));

	QUICKLOAD(config, "quickload", "p00,prg,t64", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(c64_state::quickload_c64));

	// software list
	SOFTWARE_LIST(config, "cart_list_vic10").set_original("vic10").set_filter("PAL");
	SOFTWARE_LIST(config, "cart_list_c64").set_original("c64_cart").set_filter("PAL");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");
}


//-------------------------------------------------
//  machine_config( clipper )
//-------------------------------------------------

void clipper_state::clipper(machine_config &config)
{
	pal(config);

	// TODO extra hardware

	// software list
	SOFTWARE_LIST(config, "flop525").set_original("clipper_flop");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( c64 )
//-------------------------------------------------

ROM_START( c64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r3")
	ROM_SYSTEM_BIOS(0, "r1", "Kernal rev. 1" )
	ROMX_LOAD( "901227-01.u4", 0x0000, 0x2000, CRC(dce782fa) SHA1(87cc04d61fc748b82df09856847bb5c2754a2033), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "r2", "Kernal rev. 2" )
	ROMX_LOAD( "901227-02.u4", 0x0000, 0x2000, CRC(a5c687b3) SHA1(0e2e4ee3f2d41f00bed72f9ab588b83e306fdb13), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "r3", "Kernal rev. 3" )
	ROMX_LOAD( "901227-03.u4", 0x0000, 0x2000, CRC(dbe3e7c7) SHA1(1d503e56df85a62fee696e7618dc5b4e781df1bb), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos c64.u4", 0x0000, 0x2000, CRC(2f79984c) SHA1(31e73e66eccb28732daea8ec3ad1addd9b39a017), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "speeddos", "SpeedDOS" )
	ROMX_LOAD( "speed-dos.u4", 0x0000, 0x2000, CRC(5beb9ac8) SHA1(8896c8de9e26ef1396eb46020b2de346a3eeab7e), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(5, "speeddos20", "SpeedDOS-Plus+ v2.0" )
	ROMX_LOAD( "speed-dosplus.u4", 0x0000, 0x2000, CRC(10aee0ae) SHA1(6cebd4dc0c5e8c0b073586a3f1c43cc3349b9736), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(6, "speeddos27", "SpeedDOS-Plus+ v2.7" )
	ROMX_LOAD( "speed-dosplus27.u4", 0x0000, 0x2000, CRC(ff59995e) SHA1(c8d864e5fc7089af8afce97dc0a0224df11df1c3), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(7, "prodos", "Professional-DOS v1" )
	ROMX_LOAD( "prodos.u4", 0x0000, 0x2000, CRC(37ed83a2) SHA1(35f4f0fe03c0b7b3762b526ba855de41b496fb60), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(8, "prodos2", "Professional-DOS Release 2/4L2" )
	ROMX_LOAD( "prodos24l2.u4", 0x0000, 0x2000, CRC(41dad9fe) SHA1(fbf3dcc2ed40e58b07595740ea6fbff7ab19ebad), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(9, "prodos3", "Professional-DOS Release 3/5L2" )
	ROMX_LOAD( "prodos35l2.u4", 0x0000, 0x2000, CRC(2822eee7) SHA1(77356b84c1648018863d1c8dd5bc3a37485bc00e), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(10, "turborom", "Cockroach Turbo-ROM" )
	ROMX_LOAD( "turborom.u4", 0x0000, 0x2000, CRC(e6c763a2) SHA1(eff5a4b6bc65daa9421bd3856dd99a3195068e1c), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(11, "dosrom", "DOS-ROM v1.2" )
	ROMX_LOAD( "dosrom12.u4", 0x0000, 0x2000, CRC(ac030fc0) SHA1(0e4b38e81b49f55d52162154a44f0fffd2b0d04f), ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(12, "turborom2", "Datel Turbo ROM II (PAL)" )
	ROMX_LOAD( "turborom2.u4", 0x0000, 0x2000, CRC(ea3ba683) SHA1(4bb23f764a3d255119fbae37202ca820caa04e1f), ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(13, "mercury", "Mercury-ROM v3 (NTSC)" )
	ROMX_LOAD( "mercury3.u4", 0x0000, 0x2000, CRC(6eac46a2) SHA1(4e351aa5fcb97c4c21e565aa2c830cc09bd47533), ROM_BIOS(13) )
	ROM_SYSTEM_BIOS(14, "dolphin", "Dolphin-DOS 1.0" )
	ROMX_LOAD( "kernal-10-mager.u4", 0x0000, 0x2000, CRC(c9bb21bc) SHA1(e305216e50ff8a7acf102be6c6343e3d44a16233), ROM_BIOS(14) )
	ROM_SYSTEM_BIOS(15, "dolphin201au", "Dolphin-DOS 2.0 1 au" )
	ROMX_LOAD( "kernal-20-1_au.u4", 0x0000, 0x2000, CRC(7068bbcc) SHA1(325ce7e32609a8fc704aaa76f5eb4cd7d8099a92), ROM_BIOS(15) )
	ROM_SYSTEM_BIOS(16, "dolphin201", "Dolphin-DOS 2.0 1" )
	ROMX_LOAD( "kernal-20-1.u4", 0x0000, 0x2000, CRC(c9c4c44e) SHA1(7f5d8f08c5ed2182ffb415a3d777fdd922496d02), ROM_BIOS(16) )
	ROM_SYSTEM_BIOS(17, "dolphin202", "Dolphin-DOS 2.0 2" )
	ROMX_LOAD( "kernal-20-2.u4", 0x0000, 0x2000, CRC(ffaeb9bc) SHA1(5f6c1bad379da16f77bccb58e80910f307dfd5f8), ROM_BIOS(17) )
	ROM_SYSTEM_BIOS(18, "dolphin203", "Dolphin-DOS 2.0 3" )
	ROMX_LOAD( "kernal-20-3.u4", 0x0000, 0x2000, CRC(4fd511f2) SHA1(316fba280dcb29496d593c0c4e3ee9a19844054e), ROM_BIOS(18) )
	ROM_SYSTEM_BIOS(19, "dolphin30", "Dolphin-DOS 3.0" )
	ROMX_LOAD( "kernal-30.u4", 0x0000, 0x2000, CRC(5402d643) SHA1(733acb96fead2fb4df77840c5bb618f08439fc7e), ROM_BIOS(19) )
	ROM_SYSTEM_BIOS(20, "taccess", "TurboAccess v2.6" )
	ROMX_LOAD( "turboaccess26.u4", 0x0000, 0x2000, CRC(93de6cd9) SHA1(a74478f3b9153c13176eac80ebfacc512ae7cbf0), ROM_BIOS(20) )
	ROM_SYSTEM_BIOS(21, "ttrans301", "TurboTrans v3.0 1" )
	ROMX_LOAD( "turboaccess301.u4", 0x0000, 0x2000, CRC(b3304dcf) SHA1(4d47a265ef65e4823f862cfc3d514c2a71473580), ROM_BIOS(21) )
	ROM_SYSTEM_BIOS(22, "ttrans302", "TurboTrans v3.0 2" )
	ROMX_LOAD( "turboaccess302.u4", 0x0000, 0x2000, CRC(9e696a7b) SHA1(5afae75d66d539f4bb4af763f029f0ef6523a4eb), ROM_BIOS(22) )
	ROM_SYSTEM_BIOS(23, "tprocess", "Turbo-Process (PAL)" )
	ROMX_LOAD( "turboprocess.u4", 0x0000, 0x2000, CRC(e5610d76) SHA1(e3f35777cfd16cce4717858f77ff354763395ba9), ROM_BIOS(23) )
	ROM_SYSTEM_BIOS(24, "tprocessn", "Turbo-Process (NTSC)" )
	ROMX_LOAD( "turboprocessus.u4", 0x0000, 0x2000, CRC(7480b76a) SHA1(ef1664b5057ae3cc6d104fc2f5c1fb29ee5a1b2b), ROM_BIOS(24) )
	ROM_SYSTEM_BIOS(25, "exos3", "EXOS v3" )
	ROMX_LOAD( "exos3.u4", 0x0000, 0x2000, CRC(4e54d020) SHA1(f8931b7c0b26807f4de0cc241f0b1e2c8f5271e9), ROM_BIOS(25) )
	ROM_SYSTEM_BIOS(26, "exos4", "EXOS v4" )
	ROMX_LOAD( "exos4.u4", 0x0000, 0x2000, CRC(d5cf83a9) SHA1(d5f03a5c0e9d00032d4751ecc6bcd6385879c9c7), ROM_BIOS(26) )
	ROM_SYSTEM_BIOS(27, "digidos", "DigiDOS" )
	ROMX_LOAD( "digidos.u4", 0x0000, 0x2000, CRC(2b0c8e89) SHA1(542d6f61c318bced0642e7c2d4d3b34a0f13e634), ROM_BIOS(27) )
	ROM_SYSTEM_BIOS(28, "magnum", "Magnum Load" )
	ROMX_LOAD( "magnum.u4", 0x0000, 0x2000, CRC(b2cffcc6) SHA1(827c782c1723b5d0992c05c00738ae4b2133b641), ROM_BIOS(28) )
	ROM_SYSTEM_BIOS(29, "mercury31s", "Mercury-ROM v3.1s" )
	ROMX_LOAD( "mercury31s.u4", 0x0000, 0x2000, CRC(97aa5d2f) SHA1(9fc653e61c34225245036f266db14e05feeadb21), ROM_BIOS(29) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64_jp )
//-------------------------------------------------

ROM_START( c64_jp )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "906145-02.u4", 0x0000, 0x2000, CRC(3a9ef6f1) SHA1(4ff0f11e80f4b57430d8f0c3799ed0f0e0f4565d) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "906143-02.u5", 0x0000, 0x1000, CRC(1604f6c1) SHA1(0fad19dbcdb12461c99657b2979dbb5c2e47b527) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64p )
//-------------------------------------------------

#define rom_c64p rom_c64


//-------------------------------------------------
//  ROM( c64_se )
//-------------------------------------------------

ROM_START( c64_se )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernel.u4",  0x0000, 0x2000, CRC(f10c2c25) SHA1(e4f52d9b36c030eb94524eb49f6f0774c1d02e5e) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_SYSTEM_BIOS( 0, "default", "Swedish Characters" )
	ROMX_LOAD( "charswe.u5", 0x0000, 0x1000, CRC(bee9b3fd) SHA1(446ae58f7110d74d434301491209299f66798d8a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "alt", "Swedish Characters (Alt)" )
	ROMX_LOAD( "charswe2.u5", 0x0000, 0x1000, CRC(377a382b) SHA1(20df25e0ba1c88f31689c1521397c96968967fac), ROM_BIOS(1) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( pet64 )
//-------------------------------------------------

ROM_START( pet64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "901246-01.u4", 0x0000, 0x2000, CRC(789c8cc5) SHA1(6c4fa9465f6091b174df27dfe679499df447503c) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( edu64 )
//-------------------------------------------------

#define rom_edu64   rom_c64


//-------------------------------------------------
//  ROM( sx64 )
//-------------------------------------------------

ROM_START( sx64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.ud4", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS(0, "cbm", "Original" )
	ROMX_LOAD( "251104-04.ud3", 0x0000, 0x2000, CRC(2c5965d4) SHA1(aa136e91ecf3c5ac64f696b3dbcbfc5ba0871c98), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos sx64.ud3", 0x0000, 0x2000, CRC(2b5a88f5) SHA1(942c2150123dc30f40b3df6086132ef0a3c43948), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "1541flash", "1541 FLASH!" )
	ROMX_LOAD( "1541 flash.ud3", 0x0000, 0x2000, CRC(0a1c9b85) SHA1(0bfcaab0ae453b663a6e01cd59a9764805419e00), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "turborom", "Cockroach Turbo-ROM" )
	ROMX_LOAD( "turboromsx.u4", 0x0000, 0x2000, CRC(48579c30) SHA1(6c907fdd07c14e162eb8c8fb750b1bbaf69dccb4), ROM_BIOS(3) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.ud1", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.ue4", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( rom_sx64p )
//-------------------------------------------------

#define rom_sx64p   rom_sx64


//-------------------------------------------------
//  ROM( vip64 )
//-------------------------------------------------

ROM_START( vip64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.ud4", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernelsx.ud3", 0x0000, 0x2000, CRC(7858d3d7) SHA1(097cda60469492a8916c2677b7cce4e12a944bc0) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "charswe.ud1", 0x0000, 0x1000, CRC(bee9b3fd) SHA1(446ae58f7110d74d434301491209299f66798d8a) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.ue4", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( dx64 )
//-------------------------------------------------

// ROM_LOAD( "dx64kern.bin", 0x0000, 0x2000, CRC(58065128) ) TODO where is this illusive ROM?
#define rom_dx64    rom_sx64


//-------------------------------------------------
//  ROM( tesa6240 )
//-------------------------------------------------

ROM_START( tesa6240 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "tesa-basic.ud4", 0x0000, 0x2000, CRC(f319d661) SHA1(0033afa7d2fbff314d80427324633c5444fbf1cd) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "tesa-kernal.ud3", 0x0000, 0x2000, CRC(af638f9c) SHA1(a2c9c83f598623c9940949979ac643f12397e907) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "tesa-char.ud1", 0x0000, 0x1000, CRC(10765a90) SHA1(1b824df5a295d0479e830e272758640b9fe99344) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.ue4", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64c )
//-------------------------------------------------

ROM_START( c64c )
	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_DEFAULT_BIOS("cbm")
	ROM_SYSTEM_BIOS(0, "cbm", "Original" )
	ROMX_LOAD( "251913-01.u4", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pdc", "ProLogic-DOS Classic" )
	ROMX_LOAD( "pdc.u4", 0x0000, 0x4000, CRC(6b653b9c) SHA1(0f44a9c62619424a0cd48a90e1b377b987b494e0), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252715-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64cp )
//-------------------------------------------------

#define rom_c64cp       rom_c64c


//-------------------------------------------------
//  ROM( c64g )
//-------------------------------------------------

#define rom_c64g        rom_c64c


//-------------------------------------------------
//  ROM( c64c_es )
//-------------------------------------------------

ROM_START( c64c_es )
	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_LOAD( "251913-01.u4", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "325056-03.u5", 0x0000, 0x1000, CRC(c890c175) SHA1(4f57259fff9ef1963a4e87165a6f35ca23864c76) ) // aka 325245-01

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252715-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64c_se )
//-------------------------------------------------

ROM_START( c64c_se )
	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_LOAD( "325182-01.u4", 0x0000, 0x4000, CRC(2aff27d3) SHA1(267654823c4fdf2167050f41faa118218d2569ce) ) // 128/64 FI

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "cbm 64 skand.gen.u5", 0x0000, 0x1000, CRC(377a382b) SHA1(20df25e0ba1c88f31689c1521397c96968967fac) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252715-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64gs )
//-------------------------------------------------

ROM_START( c64gs )
	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_LOAD( "390852-01.u4", 0x0000, 0x4000, CRC(b0a9c2da) SHA1(21940ef5f1bfe67d7537164f7ca130a1095b067a) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252535-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( clipper )
//-------------------------------------------------

ROM_START( clipper )
	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernal.bin", 0x0000, 0x2000, CRC(13ca39ca) SHA1(d668e7980887a5b90fad693eba35fac49c7ad941) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "chr_gen.bin", 0x0000, 0x1000, CRC(a675a239) SHA1(9ad11a5de5bd7e43c43e985b31bed7ca96101fc5) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )

	ROM_REGION( 0x4000, "fdc", 0 )
	ROM_LOAD( "fdc.bin", 0x0000, 0x2000, CRC(44b0b1fc) SHA1(effcf165cb4ea32540a8a8c12781303dc36fa4b2) )
	ROM_LOAD( "fdc_12.bin", 0x2000, 0x2000, CRC(397a2219) SHA1(7eefcc871a805f45be4ba016fe9fc7d25318c431) )

	ROM_REGION( 0x6000, "sb", 0 )
	ROM_LOAD( "sb1.bin", 0x0000, 0x2000, CRC(400040be) SHA1(b290216f49b24355a1a2b25adfa96709c5d9c049) )
	ROM_LOAD( "sb2.bin", 0x2000, 0x2000, CRC(a3d7177a) SHA1(0f50381aecf3c5ea03cce358a3325b3e06939c37) )
	ROM_LOAD( "sb3.bin", 0x4000, 0x2000, CRC(7b1fc6c6) SHA1(900fe4be8d6348bf68dbda0c7ecefc84bda51202) )

	ROM_REGION( 0x1000, "thdr", 0 )
	ROM_LOAD( "thdr5.bin", 0x0000, 0x1000, CRC(b4296e62) SHA1(4b6edadbb810c409ece77d5834568fcc2e0bbd61) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY                        FULLNAME                                   FLAGS
COMP( 1982, c64,      0,      0,      ntsc,    c64,     c64_state,     empty_init, "Commodore Business Machines", "Commodore 64 (NTSC)",                     MACHINE_SUPPORTS_SAVE )
COMP( 1982, c64_jp,   c64,    0,      ntsc,    c64,     c64_state,     empty_init, "Commodore Business Machines", "Commodore 64 (Japan)",                    MACHINE_SUPPORTS_SAVE )
COMP( 1982, c64p,     c64,    0,      pal,     c64,     c64_state,     empty_init, "Commodore Business Machines", "Commodore 64 (PAL)",                      MACHINE_SUPPORTS_SAVE )
COMP( 1982, c64_se,   c64,    0,      pal,     c64sw,   c64_state,     empty_init, "Commodore Business Machines", "Commodore 64 / VIC-64S (Sweden/Finland)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, pet64,    c64,    0,      pet64,   c64,     c64_state,     empty_init, "Commodore Business Machines", "PET 64 / CBM 4064 (NTSC)",                MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS )
COMP( 1983, edu64,    c64,    0,      pet64,   c64,     c64_state,     empty_init, "Commodore Business Machines", "Educator 64 (NTSC)",                      MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS )
COMP( 1984, sx64,     c64,    0,      ntsc_sx, c64,     sx64_state,    empty_init, "Commodore Business Machines", "SX-64 / Executive 64 (NTSC)",             MACHINE_SUPPORTS_SAVE )
COMP( 1984, sx64p,    c64,    0,      pal_sx,  c64,     sx64_state,    empty_init, "Commodore Business Machines", "SX-64 / Executive 64 (PAL)",              MACHINE_SUPPORTS_SAVE )
COMP( 1984, vip64,    c64,    0,      pal_sx,  c64sw,   sx64_state,    empty_init, "Commodore Business Machines", "VIP-64 (Sweden/Finland)",                 MACHINE_SUPPORTS_SAVE )
COMP( 1984, dx64,     c64,    0,      ntsc_dx, c64,     sx64_state,    empty_init, "Commodore Business Machines", "DX-64 (NTSC)",                            MACHINE_SUPPORTS_SAVE )
COMP( 1984, tesa6240, c64,    0,      pal_sx,  c64,     sx64_state,    empty_init, "Tesa Etikett",                "Etikettendrucker 6240",                   MACHINE_SUPPORTS_SAVE )
COMP( 1984, clipper,  c64,    0,      clipper, clipper, clipper_state, empty_init, "Professional Data Computer",  "Clipper",                                 MACHINE_NOT_WORKING )
COMP( 1986, c64c,     c64,    0,      ntsc_c,  c64,     c64c_state,    empty_init, "Commodore Business Machines", "Commodore 64C (NTSC)",                    MACHINE_SUPPORTS_SAVE )
COMP( 1986, c64cp,    c64,    0,      pal_c,   c64,     c64c_state,    empty_init, "Commodore Business Machines", "Commodore 64C (PAL)",                     MACHINE_SUPPORTS_SAVE )
COMP( 1988, c64c_es,  c64,    0,      pal_c,   c64sw,   c64c_state,    empty_init, "Commodore Business Machines", "Commodore 64C (Spain)",                   MACHINE_SUPPORTS_SAVE )
COMP( 1986, c64c_se,  c64,    0,      pal_c,   c64sw,   c64c_state,    empty_init, "Commodore Business Machines", "Commodore 64C (Sweden/Finland)",          MACHINE_SUPPORTS_SAVE )
COMP( 1986, c64g,     c64,    0,      pal_c,   c64,     c64c_state,    empty_init, "Commodore Business Machines", "Commodore 64G (PAL)",                     MACHINE_SUPPORTS_SAVE )
CONS( 1990, c64gs,    c64,    0,      pal_gs,  c64gs,   c64gs_state,   empty_init, "Commodore Business Machines", "Commodore 64 Games System (PAL)",         MACHINE_SUPPORTS_SAVE )

