// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mamedasm.c

    Generic MAME disassembler.

****************************************************************************/

// the disassemblers assume they're in MAME and emu.h is a PCH, so we minimally pander to them
#include "disasmintf.h"

using offs_t = osd::u32;
using util::BIT;

#include "cpu/8x300/8x300dasm.h"
#include "cpu/adsp2100/2100dasm.h"
#include "cpu/alph8201/8201dasm.h"
#include "cpu/alpha/alphad.h"
#include "cpu/alto2/alto2dsm.h"
#include "cpu/am29000/am29dasm.h"
#include "cpu/amis2000/amis2000d.h"
#include "cpu/apexc/apexcdsm.h"
#include "cpu/arc/arcdasm.h"
#include "cpu/arcompact/arcompactdasm.h"
#include "cpu/arm/armdasm.h"
#include "cpu/arm7/arm7dasm.h"
#include "cpu/asap/asapdasm.h"
#include "cpu/avr8/avr8dasm.h"
#include "cpu/capricorn/capricorn_dasm.h"
#include "cpu/ccpu/ccpudasm.h"
#include "cpu/clipper/clipperd.h"
#include "cpu/cop400/cop410ds.h"
#include "cpu/cop400/cop420ds.h"
#include "cpu/cop400/cop424ds.h"
#include "cpu/cop400/cop444ds.h"
#include "cpu/cosmac/cosdasm.h"
#include "cpu/cp1610/1610dasm.h"
#include "cpu/cubeqcpu/cubedasm.h"
#include "cpu/dsp16/dsp16dis.h"
#include "cpu/dsp32/dsp32dis.h"
#include "cpu/dsp56k/dsp56dsm.h"
#include "cpu/e0c6200/e0c6200d.h"
#include "cpu/e132xs/32xsdasm.h"
#include "cpu/es5510/es5510d.h"
#include "cpu/esrip/esripdsm.h"
#include "cpu/f8/f8dasm.h"
#include "cpu/g65816/g65816ds.h"
#include "cpu/h6280/6280dasm.h"
#include "cpu/h8/h8d.h"
#include "cpu/h8/h8hd.h"
#include "cpu/h8/h8s2000d.h"
#include "cpu/h8/h8s2600d.h"
#include "cpu/hcd62121/hcd62121d.h"
#include "cpu/hd61700/hd61700d.h"
#include "cpu/hmcs40/hmcs40d.h"
#include "cpu/hpc/hpcdasm.h"
#include "cpu/hphybrid/hphybrid_dasm.h"
#include "cpu/i386/i386dasm.h"
#include "cpu/i8008/8008dasm.h"
#include "cpu/i8085/8085dasm.h"
#include "cpu/i8089/i8089_dasm.h"
#include "cpu/i860/i860dis.h"
#include "cpu/i960/i960dis.h"
#include "cpu/ie15/ie15dasm.h"
#include "cpu/jaguar/jagdasm.h"
#include "cpu/lc8670/lc8670dsm.h"
#include "cpu/lh5801/5801dasm.h"
#include "cpu/lr35902/lr35902d.h"
#include "cpu/m37710/m7700ds.h"
#include "cpu/m6502/m4510d.h"
#include "cpu/m6502/m6502d.h"
#include "cpu/m6502/m6509d.h"
#include "cpu/m6502/m6510d.h"
#include "cpu/m6502/m65c02d.h"
#include "cpu/m6502/m65ce02d.h"
#include "cpu/m6502/m740d.h"
#include "cpu/m6502/xavixd.h"
#include "cpu/m6502/xavix2000d.h"
#include "cpu/m6800/6800dasm.h"
#include "cpu/m68000/m68kdasm.h"
#include "cpu/m6805/6805dasm.h"
#include "cpu/m6809/6x09dasm.h"
#include "cpu/mb86233/mb86233d.h"
#include "cpu/mb86235/mb86235d.h"
#include "cpu/mb88xx/mb88dasm.h"
#include "cpu/mc68hc11/hc11dasm.h"
#include "cpu/mcs40/mcs40dasm.h"
#include "cpu/mcs48/mcs48dsm.h"
#include "cpu/mcs51/mcs51dasm.h"
#include "cpu/mcs51/axc51-core_dasm.h"
#include "cpu/mcs96/i8x9xd.h"
#include "cpu/mcs96/i8xc196d.h"
#include "cpu/melps4/melps4d.h"
#include "cpu/minx/minxd.h"
#include "cpu/mips/mips3dsm.h"
#include "cpu/mips/mips1dsm.h"
#include "cpu/mn10200/mn102dis.h"
#include "cpu/nanoprocessor/nanoprocessor_dasm.h"
#include "cpu/nec/necdasm.h"
#include "cpu/ns32000/ns32000dasm.h"
#include "cpu/nuon/nuondasm.h"
#include "cpu/patinhofeio/patinho_feio_dasm.h"
#include "cpu/pdp1/pdp1dasm.h"
#include "cpu/pdp1/tx0dasm.h"
#include "cpu/pdp8/pdp8dasm.h"
#include "cpu/pic16c5x/16c5xdsm.h"
#include "cpu/pic16c62x/16c62xdsm.h"
#include "cpu/powerpc/ppc_dasm.h"
#include "cpu/pps4/pps4dasm.h"
#include "cpu/psx/psxdasm.h"
#include "cpu/rsp/rsp_dasm.h"
#include "cpu/s2650/2650dasm.h"
#include "cpu/saturn/saturnds.h"
#include "cpu/sc61860/scdasm.h"
#include "cpu/scmp/scmpdasm.h"
#include "cpu/score/scoredsm.h"
#include "cpu/scudsp/scudspdasm.h"
#include "cpu/se3208/se3208dis.h"
#include "cpu/sh/sh_dasm.h"
#include "cpu/sharc/sharcdsm.h"
#include "cpu/sm510/sm510d.h"
#include "cpu/sm8500/sm8500d.h"
#include "cpu/sparc/sparcdasm.h"
#include "cpu/spc700/spc700ds.h"
#include "cpu/ssem/ssemdasm.h"
#include "cpu/ssp1601/ssp1601d.h"
#include "cpu/st62xx/st62xx_dasm.h"
#include "cpu/superfx/sfx_dasm.h"
#include "cpu/t11/t11dasm.h"
#include "cpu/tlcs870/tlcs870d.h"
#include "cpu/tlcs90/tlcs90d.h"
#include "cpu/tlcs900/dasm900.h"
#include "cpu/tms1000/tms1k_dasm.h"
#include "cpu/tms32010/32010dsm.h"
#include "cpu/tms32025/32025dsm.h"
#include "cpu/tms32031/dis32031.h"
#include "cpu/tms32051/dis32051.h"
#include "cpu/tms32082/dis_mp.h"
#include "cpu/tms32082/dis_pp.h"
#include "cpu/tms34010/34010dsm.h"
#include "cpu/tms57002/57002dsm.h"
#include "cpu/tms7000/7000dasm.h"
#include "cpu/tms9900/9900dasm.h"
#include "cpu/tms9900/tms99com.h"
#include "cpu/ucom4/ucom4d.h"
#include "cpu/unsp/unspdasm.h"
#include "cpu/upd7725/dasm7725.h"
#include "cpu/upd7810/upd7810_dasm.h"
#include "cpu/v60/v60d.h"
#include "cpu/v810/v810dasm.h"
#include "cpu/z180/z180dasm.h"
#include "cpu/z8/z8dasm.h"
#include "cpu/z80/z80dasm.h"
#include "cpu/z8000/8000dasm.h"

#include "corefile.h"
#include "corestr.h"
#include "eminline.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include <ctype.h>

using u8 = util::u8;
using u16 = util::u16;
using u32 = util::u32;
using u64 = util::u64;

// Configuration classes

// Selected through dasm name
struct arm7_unidasm_t : public arm7_disassembler::config
{
	bool t_flag;
	arm7_unidasm_t() { t_flag = false; }
	virtual ~arm7_unidasm_t() override = default;
	virtual bool get_t_flag() const override { return t_flag; }
} arm7_unidasm;

// Configuration missing
struct g65816_unidasm_t : g65816_disassembler::config
{
	bool m_flag;
	bool x_flag;

	g65816_unidasm_t() { m_flag = false; x_flag = false; }
	virtual ~g65816_unidasm_t() override = default;
	virtual bool get_m_flag() const override { return m_flag; }
	virtual bool get_x_flag() const override { return x_flag; }
} g65816_unidasm;

// Configuration missing
struct m740_unidasm_t : m740_disassembler::config
{
	u32 inst_state_base;
	m740_unidasm_t() { inst_state_base = 0; }
	virtual ~m740_unidasm_t() override = default;
	virtual u32 get_state_base() const override { return inst_state_base; }
} m740_unidasm;

// Configuration missing
struct m7700_unidasm_t : m7700_disassembler::config
{
	bool m_flag;
	bool x_flag;

	m7700_unidasm_t() { m_flag = false; x_flag = false; }
	virtual ~m7700_unidasm_t() override = default;
	virtual bool get_m_flag() const override { return m_flag; }
	virtual bool get_x_flag() const override { return x_flag; }
} m7700_unidasm;


// Configuration missing
struct s2650_unidasm_t : s2650_disassembler::config
{
	bool z80_mnemonics;
	s2650_unidasm_t() { z80_mnemonics = false; }
	virtual ~s2650_unidasm_t() override = default;
	virtual bool get_z80_mnemonics_mode() const override { return z80_mnemonics; }
} s2650_unidasm;

// Configuration missing
struct saturn_unidasm_t : saturn_disassembler::config
{
	bool nonstandard_mnemonics;
	saturn_unidasm_t() { nonstandard_mnemonics = false; }
	virtual ~saturn_unidasm_t() override = default;
	virtual bool get_nonstandard_mnemonics_mode() const override { return nonstandard_mnemonics; }
} saturn_unidasm;

// Configuration missing
struct superfx_unidasm_t : superfx_disassembler::config
{
	u16 sfr;
	superfx_unidasm_t() { sfr = superfx_disassembler::SUPERFX_SFR_ALT0; }
	virtual ~superfx_unidasm_t() override = default;
	virtual u16 get_alt() const override { return sfr; }
} superfx_unidasm;

// Selected through dasm name
struct i386_unidasm_t : i386_disassembler::config
{
	int mode;
	i386_unidasm_t() { mode = 32; }
	virtual ~i386_unidasm_t() override = default;
	virtual int get_mode() const override { return mode; }
} i386_unidasm;

// Configuration missing
struct z8000_unidasm_t : z8000_disassembler::config
{
	bool segmented_mode;
	z8000_unidasm_t() { segmented_mode = false; }
	virtual ~z8000_unidasm_t() override = default;
	virtual bool get_segmented_mode() const override { return segmented_mode; }
} z8000_unidasm;

// Configuration missing
struct hyperstone_unidasm_t : hyperstone_disassembler::config
{
	bool h;
	u8 fp;
	hyperstone_unidasm_t() { h = false; fp = 0; }
	virtual ~hyperstone_unidasm_t() = default;

	virtual u8 get_fp() const { return fp; }
	virtual bool get_h() const { return h; }
} hyperstone_unidasm;



enum endianness { le, be };

struct dasm_table_entry
{
	const char *            name;
	endianness              endian;
	int8_t                  pcshift;
	std::function<util::disasm_interface *()> alloc;
};


struct options
{
	const char *            filename;
	offs_t                  basepc;
	uint8_t                 norawbytes;
	uint8_t                 lower;
	uint8_t                 upper;
	uint8_t                 flipped;
	int                     mode;
	const dasm_table_entry *dasm;
	uint32_t                skip;
	uint32_t                count;
};

static const dasm_table_entry dasm_table[] =
{
	{ "8x300",           be, -1, []() -> util::disasm_interface * { return new n8x300_disassembler; } },
	{ "adsp21xx",        le, -2, []() -> util::disasm_interface * { return new adsp21xx_disassembler; } },
	{ "alpha",           le,  0, []() -> util::disasm_interface * { return new alpha_disassembler; } },
	{ "alpha_nt",        le,  0, []() -> util::disasm_interface * { return new alpha_disassembler(alpha_disassembler::TYPE_NT); } },
	{ "alpha_unix",      le,  0, []() -> util::disasm_interface * { return new alpha_disassembler(alpha_disassembler::TYPE_UNIX); } },
	{ "alpha_vms",       le,  0, []() -> util::disasm_interface * { return new alpha_disassembler(alpha_disassembler::TYPE_VMS); } },
	{ "alpha8201",       le,  0, []() -> util::disasm_interface * { return new alpha8201_disassembler; } },
	{ "alto2",           be, -2, []() -> util::disasm_interface * { return new alto2_disassembler; } },
	{ "am29000",         be,  0, []() -> util::disasm_interface * { return new am29000_disassembler; } },
	{ "amis2000",        le,  0, []() -> util::disasm_interface * { return new amis2000_disassembler; } },
	{ "apexc",           be,  0, []() -> util::disasm_interface * { return new apexc_disassembler; } },
	{ "arc",             be,  0, []() -> util::disasm_interface * { return new arc_disassembler; } },
	{ "arcompact",       le,  0, []() -> util::disasm_interface * { return new arcompact_disassembler; } },
	{ "arm",             le,  0, []() -> util::disasm_interface * { return new arm_disassembler; } },
	{ "arm_be",          be,  0, []() -> util::disasm_interface * { return new arm_disassembler; } },
	{ "arm7",            le,  0, []() -> util::disasm_interface * { arm7_unidasm.t_flag = false; return new arm7_disassembler(&arm7_unidasm); } },
	{ "arm7_be",         be,  0, []() -> util::disasm_interface * { arm7_unidasm.t_flag = false; return new arm7_disassembler(&arm7_unidasm); } },
	{ "arm7thumb",       le,  0, []() -> util::disasm_interface * { arm7_unidasm.t_flag = true; return new arm7_disassembler(&arm7_unidasm); } },
	{ "arm7thumbb",      be,  0, []() -> util::disasm_interface * { arm7_unidasm.t_flag = true; return new arm7_disassembler(&arm7_unidasm); } },
	{ "asap",            le,  0, []() -> util::disasm_interface * { return new asap_disassembler; } },
	{ "avr8",            le,  0, []() -> util::disasm_interface * { return new avr8_disassembler; } },
	{ "axc51core",       le,  0, []() -> util::disasm_interface * { return new axc51core_disassembler; } },
	{ "capricorn",       le,  0, []() -> util::disasm_interface * { return new capricorn_disassembler; } },
	{ "ccpu",            le,  0, []() -> util::disasm_interface * { return new ccpu_disassembler; } },
	{ "cdp1801",         le,  0, []() -> util::disasm_interface * { return new cosmac_disassembler(cosmac_disassembler::TYPE_1801); } },
	{ "cdp1802",         le,  0, []() -> util::disasm_interface * { return new cosmac_disassembler(cosmac_disassembler::TYPE_1802); } },
	{ "clipper",         le,  0, []() -> util::disasm_interface * { return new clipper_disassembler; } },
	{ "coldfire",        be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_COLDFIRE); } },
	{ "cop410",          le,  0, []() -> util::disasm_interface * { return new cop410_disassembler; } },
	{ "cop420",          le,  0, []() -> util::disasm_interface * { return new cop420_disassembler; } },
	{ "cop444",          le,  0, []() -> util::disasm_interface * { return new cop444_disassembler; } },
	{ "cop424",          le,  0, []() -> util::disasm_interface * { return new cop424_disassembler; } },
	{ "cp1610",          be, -1, []() -> util::disasm_interface * { return new cp1610_disassembler; } },
	{ "cquestlin",       be, -3, []() -> util::disasm_interface * { return new cquestlin_disassembler; } },
	{ "cquestrot",       be, -3, []() -> util::disasm_interface * { return new cquestrot_disassembler; } },
	{ "cquestsnd",       be, -3, []() -> util::disasm_interface * { return new cquestsnd_disassembler; } },
	{ "ds5002fp",        le,  0, []() -> util::disasm_interface * { return new ds5002fp_disassembler; } },
	{ "dsp16",           le, -1, []() -> util::disasm_interface * { return new dsp16_disassembler; } },
	{ "dsp32c",          le,  0, []() -> util::disasm_interface * { return new dsp32c_disassembler; } },
	{ "dsp56k",          le, -1, []() -> util::disasm_interface * { return new dsp56k_disassembler; } },
	{ "e0c6200",         be, -1, []() -> util::disasm_interface * { return new e0c6200_disassembler; } },
//  { "es5510",          be,  0, []() -> util::disasm_interface * { return new es5510_disassembler; } }, // Currently does nothing
	{ "esrip",           be,  0, []() -> util::disasm_interface * { return new esrip_disassembler; } },
	{ "f8",              be,  0, []() -> util::disasm_interface * { return new f8_disassembler; } },
	{ "g65816",          le,  0, []() -> util::disasm_interface * { return new g65816_disassembler(&g65816_unidasm); } },
	{ "h6280",           le,  0, []() -> util::disasm_interface * { return new h6280_disassembler; } },
	{ "h8",              be,  0, []() -> util::disasm_interface * { return new h8_disassembler; } },
	{ "h8h",             be,  0, []() -> util::disasm_interface * { return new h8h_disassembler; } },
	{ "h8s2000",         be,  0, []() -> util::disasm_interface * { return new h8s2000_disassembler; } },
	{ "h8s2600",         be,  0, []() -> util::disasm_interface * { return new h8s2600_disassembler; } },
	{ "hc11",            be,  0, []() -> util::disasm_interface * { return new hc11_disassembler; } },
	{ "hcd62121",        le,  0, []() -> util::disasm_interface * { return new hcd62121_disassembler; } },
	{ "hd61700",         le,  0, []() -> util::disasm_interface * { return new hd61700_disassembler; } },
	{ "hd6301",          be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(6301); } },
	{ "hd6309",          be,  0, []() -> util::disasm_interface * { return new hd6309_disassembler; } },
	{ "hd63701",         be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(63701); } },
	{ "hmcs40",          le, -1, []() -> util::disasm_interface * { return new hmcs40_disassembler; } },
	{ "hp_5061_3001",    be, -1, []() -> util::disasm_interface * { return new hp_5061_3001_disassembler; } },
	{ "hp_5061_3011",    be, -1, []() -> util::disasm_interface * { return new hp_5061_3011_disassembler; } },
	{ "hp_09825_67907",  be, -1, []() -> util::disasm_interface * { return new hp_09825_67907_disassembler; } },
	{ "hpc16083",        le,  0, []() -> util::disasm_interface * { return new hpc16083_disassembler; } },
	{ "hpc16164",        le,  0, []() -> util::disasm_interface * { return new hpc16164_disassembler; } },
	{ "hyperstone",      be,  0, []() -> util::disasm_interface * { return new hyperstone_disassembler(&hyperstone_unidasm); } },
	{ "i4004",           le,  0, []() -> util::disasm_interface * { return new i4004_disassembler; } },
	{ "i4040",           le,  0, []() -> util::disasm_interface * { return new i4040_disassembler; } },
	{ "i8008",           le,  0, []() -> util::disasm_interface * { return new i8008_disassembler; } },
	{ "i802x",           le,  0, []() -> util::disasm_interface * { return new mcs48_disassembler(false, true); } },
	{ "i8051",           le,  0, []() -> util::disasm_interface * { return new i8051_disassembler; } },
	{ "i8052",           le,  0, []() -> util::disasm_interface * { return new i8052_disassembler; } },
	{ "i8085",           le,  0, []() -> util::disasm_interface * { return new i8085_disassembler; } },
	{ "i8089",           le,  0, []() -> util::disasm_interface * { return new i8089_disassembler; } },
	{ "i80c51",          le,  0, []() -> util::disasm_interface * { return new i80c51_disassembler; } },
	{ "i80c52",          le,  0, []() -> util::disasm_interface * { return new i80c52_disassembler; } },
	{ "i860",            le,  0, []() -> util::disasm_interface * { return new i860_disassembler; } },
	{ "i8x9x",           le,  0, []() -> util::disasm_interface * { return new i8x9x_disassembler; } },
	{ "i8xc196",         le,  0, []() -> util::disasm_interface * { return new i8xc196_disassembler; } },
	{ "i960",            le,  0, []() -> util::disasm_interface * { return new i960_disassembler; } },
	{ "ie15",            le,  0, []() -> util::disasm_interface * { return new ie15_disassembler; } },
	{ "jaguardsp",       be,  0, []() -> util::disasm_interface * { return new jaguar_disassembler(jaguar_disassembler::variant::DSP); } },
	{ "jaguargpu",       be,  0, []() -> util::disasm_interface * { return new jaguar_disassembler(jaguar_disassembler::variant::GPU); } },
	{ "konami",          be,  0, []() -> util::disasm_interface * { return new konami_disassembler; } },
	{ "lc8670",          be,  0, []() -> util::disasm_interface * { return new lc8670_disassembler; } },
	{ "lh5801",          le,  0, []() -> util::disasm_interface * { return new lh5801_disassembler; } },
	{ "lr35902",         le,  0, []() -> util::disasm_interface * { return new lr35902_disassembler; } },
	{ "m146805",         be,  0, []() -> util::disasm_interface * { return new m146805_disassembler; } },
	{ "m37710",          le,  0, []() -> util::disasm_interface * { return new m7700_disassembler(&m7700_unidasm); } },
	{ "m4510",           le,  0, []() -> util::disasm_interface * { return new m4510_disassembler; } },
	{ "m58846",          le, -1, []() -> util::disasm_interface * { return new melps4_disassembler; } },
	{ "m6502",           le,  0, []() -> util::disasm_interface * { return new m6502_disassembler; } },
	{ "m6509",           le,  0, []() -> util::disasm_interface * { return new m6509_disassembler; } },
	{ "m6510",           le,  0, []() -> util::disasm_interface * { return new m6510_disassembler; } },
	{ "m65c02",          le,  0, []() -> util::disasm_interface * { return new m65c02_disassembler; } },
	{ "m65ce02",         le,  0, []() -> util::disasm_interface * { return new m65ce02_disassembler; } },
	{ "m6800",           be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(6800); } },
	{ "m68000",          be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_68000); } },
	{ "m68008",          be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_68008); } },
	{ "m6801",           be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(6801); } },
	{ "m68010",          be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_68010); } },
	{ "m6802",           be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(6802); } },
	{ "m68020",          be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_68020); } },
	{ "m6803",           be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(6803); } },
	{ "m68030",          be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_68030); } },
	{ "m68040",          be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_68040); } },
	{ "m6805",           be,  0, []() -> util::disasm_interface * { return new m6805_disassembler; } },
	{ "m6808",           be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(6808); } },
	{ "m6809",           be,  0, []() -> util::disasm_interface * { return new m6809_disassembler; } },
	{ "m68340",          be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_68340); } },
	{ "m68hc05",         be,  0, []() -> util::disasm_interface * { return new m68hc05_disassembler; } },
	{ "m740",            le,  0, []() -> util::disasm_interface * { return new m740_disassembler(&m740_unidasm); } },
	{ "mb86233",         le, -2, []() -> util::disasm_interface * { return new mb86233_disassembler; } },
	{ "mb86235",         le, -3, []() -> util::disasm_interface * { return new mb86235_disassembler; } },
	{ "mb88",            le,  0, []() -> util::disasm_interface * { return new mb88_disassembler; } },
	{ "mcs48",           le,  0, []() -> util::disasm_interface * { return new mcs48_disassembler(false, false); } },
	{ "minx",            le,  0, []() -> util::disasm_interface * { return new minx_disassembler; } },
	{ "mips3be",         be,  0, []() -> util::disasm_interface * { return new mips3_disassembler; } },
	{ "mips3le",         le,  0, []() -> util::disasm_interface * { return new mips3_disassembler; } },
	{ "mn10200",         le,  0, []() -> util::disasm_interface * { return new mn10200_disassembler; } },
	{ "nanoprocessor",   le,  0, []() -> util::disasm_interface * { return new hp_nanoprocessor_disassembler; } },
	{ "nec",             le,  0, []() -> util::disasm_interface * { return new nec_disassembler; } },
	{ "ns32000",         le,  0, []() -> util::disasm_interface * { return new ns32000_disassembler; } },
	{ "nuon",            be,  0, []() -> util::disasm_interface * { return new nuon_disassembler; } },
	{ "nsc8105",         be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(8105); } },
	{ "patinho_feio",    le,  0, []() -> util::disasm_interface * { return new patinho_feio_disassembler; } },
	{ "pdp1",            be,  0, []() -> util::disasm_interface * { return new pdp1_disassembler; } },
	{ "pdp8",            be,  0, []() -> util::disasm_interface * { return new pdp8_disassembler; } },
	{ "pic16c5x",        le, -1, []() -> util::disasm_interface * { return new pic16c5x_disassembler; } },
	{ "pic16c62x",       le, -1, []() -> util::disasm_interface * { return new pic16c62x_disassembler; } },
	{ "powerpc",         be,  0, []() -> util::disasm_interface * { return new powerpc_disassembler; } },
	{ "pps4",            le,  0, []() -> util::disasm_interface * { return new pps4_disassembler; } },
	{ "psxcpu",          le,  0, []() -> util::disasm_interface * { return new psxcpu_disassembler; } },
	{ "mips1be",         be,  0, []() -> util::disasm_interface * { return new mips1_disassembler; } },
	{ "mips1le",         le,  0, []() -> util::disasm_interface * { return new mips1_disassembler; } },
	{ "rsp",             le,  0, []() -> util::disasm_interface * { return new rsp_disassembler; } },
	{ "s2650",           le,  0, []() -> util::disasm_interface * { return new s2650_disassembler(&s2650_unidasm); } },
	{ "saturn",          le,  0, []() -> util::disasm_interface * { return new saturn_disassembler(&saturn_unidasm); } },
	{ "sc61860",         le,  0, []() -> util::disasm_interface * { return new sc61860_disassembler; } },
	{ "scmp",            le,  0, []() -> util::disasm_interface * { return new scmp_disassembler; } },
	{ "scudsp",          be,  0, []() -> util::disasm_interface * { return new scudsp_disassembler; } },
	{ "se3208",          le,  0, []() -> util::disasm_interface * { return new se3208_disassembler; } },
	{ "sh2",             be,  0, []() -> util::disasm_interface * { return new sh_disassembler(false); } },
	{ "sh4",             le,  0, []() -> util::disasm_interface * { return new sh_disassembler(true); } },
	{ "sh4be",           be,  0, []() -> util::disasm_interface * { return new sh_disassembler(true); } },
	{ "sharc",           le, -3, []() -> util::disasm_interface * { return new sharc_disassembler; } },
	{ "sm500",           le,  0, []() -> util::disasm_interface * { return new sm500_disassembler; } },
	{ "sm510",           le,  0, []() -> util::disasm_interface * { return new sm510_disassembler; } },
	{ "sm511",           le,  0, []() -> util::disasm_interface * { return new sm511_disassembler; } },
	{ "sm530",           le,  0, []() -> util::disasm_interface * { return new sm530_disassembler; } },
	{ "sm590",           le,  0, []() -> util::disasm_interface * { return new sm590_disassembler; } },
	{ "sm5a",            le,  0, []() -> util::disasm_interface * { return new sm5a_disassembler; } },
	{ "sm8500",          le,  0, []() -> util::disasm_interface * { return new sm8500_disassembler; } },
	{ "sparcv7",         be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, 7); } },
	{ "sparcv8",         be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, 8); } },
	{ "sparcv9",         be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, 9); } },
	{ "sparcv9vis1",     be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, 9, sparc_disassembler::vis_1); } },
	{ "sparcv9vis2",     be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, 9, sparc_disassembler::vis_2); } },
	{ "sparcv9vis2p",    be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, 9, sparc_disassembler::vis_2p); } },
	{ "sparcv9vis3",     be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, 9, sparc_disassembler::vis_3); } },
	{ "sparcv9vis3b",    be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, 9, sparc_disassembler::vis_3b); } },
	{ "spc700",          le,  0, []() -> util::disasm_interface * { return new spc700_disassembler; } },
	{ "ssem",            le,  0, []() -> util::disasm_interface * { return new ssem_disassembler; } },
	{ "ssp1601",         be, -1, []() -> util::disasm_interface * { return new ssp1601_disassembler; } },
	{ "st62xx",          le,  0, []() -> util::disasm_interface * { return new st62xx_disassembler; } },
	{ "superfx",         le,  0, []() -> util::disasm_interface * { return new superfx_disassembler(&superfx_unidasm); } },
	{ "t11",             le,  0, []() -> util::disasm_interface * { return new t11_disassembler; } },
	{ "tlcs870",         le,  0, []() -> util::disasm_interface * { return new tlcs870_disassembler; } },
	{ "tlcs90",          le,  0, []() -> util::disasm_interface * { return new tlcs90_disassembler; } },
	{ "tlcs900",         le,  0, []() -> util::disasm_interface * { return new tlcs900_disassembler; } },
	{ "tms0980",         be,  0, []() -> util::disasm_interface * { return new tms0980_disassembler; } },
	{ "tms1000",         le,  0, []() -> util::disasm_interface * { return new tms1000_disassembler; } },
	{ "tms1100",         le,  0, []() -> util::disasm_interface * { return new tms1100_disassembler; } },
	{ "tms32010",        be, -1, []() -> util::disasm_interface * { return new tms32010_disassembler; } },
	{ "tms32025",        be, -1, []() -> util::disasm_interface * { return new tms32025_disassembler; } },
	{ "tms32031",        le, -2, []() -> util::disasm_interface * { return new tms32031_disassembler; } },
	{ "tms32051",        le, -1, []() -> util::disasm_interface * { return new tms32051_disassembler; } },
	{ "tms32082_mp",     be,  0, []() -> util::disasm_interface * { return new tms32082_mp_disassembler; } },
	{ "tms32082_pp",     be,  0, []() -> util::disasm_interface * { return new tms32082_pp_disassembler; } },
	{ "tms34010",        le,  3, []() -> util::disasm_interface * { return new tms34010_disassembler(false); } },
	{ "tms34020",        le,  3, []() -> util::disasm_interface * { return new tms34010_disassembler(true); } },
	{ "tms57002",        le, -2, []() -> util::disasm_interface * { return new tms57002_disassembler; } },
	{ "tms7000",         le,  0, []() -> util::disasm_interface * { return new tms7000_disassembler; } },
	{ "tms9900",         be,  0, []() -> util::disasm_interface * { return new tms9900_disassembler(TMS9900_ID); } },
	{ "tms9980",         be,  0, []() -> util::disasm_interface * { return new tms9900_disassembler(TMS9980_ID); } },
	{ "tms9995",         be,  0, []() -> util::disasm_interface * { return new tms9900_disassembler(TMS9995_ID); } },
	{ "tp0320",          be,  0, []() -> util::disasm_interface * { return new tp0320_disassembler; } },
	{ "tx0_64kw",        be, -2, []() -> util::disasm_interface * { return new tx0_64kw_disassembler; } },
	{ "tx0_8kw",         be, -2, []() -> util::disasm_interface * { return new tx0_8kw_disassembler; } },
	{ "ucom4",           le,  0, []() -> util::disasm_interface * { return new ucom4_disassembler; } },
	{ "unsp",            be,  0, []() -> util::disasm_interface * { return new unsp_disassembler; } },
	{ "upd7725",         be,  0, []() -> util::disasm_interface * { return new necdsp_disassembler; } },
	{ "upd7801",         le,  0, []() -> util::disasm_interface * { return new upd7801_disassembler; } },
	{ "upd7807",         le,  0, []() -> util::disasm_interface * { return new upd7807_disassembler; } },
	{ "upd7810",         le,  0, []() -> util::disasm_interface * { return new upd7810_disassembler; } },
	{ "upd78c05",        le,  0, []() -> util::disasm_interface * { return new upd78c05_disassembler; } },
	{ "upi41",           le,  0, []() -> util::disasm_interface * { return new mcs48_disassembler(true, false); } },
	{ "v60",             le,  0, []() -> util::disasm_interface * { return new v60_disassembler; } },
	{ "v810",            le,  0, []() -> util::disasm_interface * { return new v810_disassembler; } },
	{ "x86_16",          le,  0, []() -> util::disasm_interface * { i386_unidasm.mode = 16; return new i386_disassembler(&i386_unidasm); } },
	{ "x86_32",          le,  0, []() -> util::disasm_interface * { i386_unidasm.mode = 32; return new i386_disassembler(&i386_unidasm); } },
	{ "x86_64",          le,  0, []() -> util::disasm_interface * { i386_unidasm.mode = 64; return new i386_disassembler(&i386_unidasm); } },
	{ "xavix",           le,  0, []() -> util::disasm_interface * { return new xavix_disassembler; } },
	{ "xavix2000",       le,  0, []() -> util::disasm_interface * { return new xavix2000_disassembler; } },
	{ "z180",            le,  0, []() -> util::disasm_interface * { return new z180_disassembler; } },
	{ "z8",              be,  0, []() -> util::disasm_interface * { return new z8_disassembler; } },
	{ "z80",             le,  0, []() -> util::disasm_interface * { return new z80_disassembler; } },
	{ "z8000",           be,  0, []() -> util::disasm_interface * { return new z8000_disassembler(&z8000_unidasm); } },
};

class unidasm_data_buffer : public util::disasm_interface::data_buffer
{
public:
	std::vector<u8> data;
	offs_t base_pc;
	u32 size;

	unidasm_data_buffer(util::disasm_interface *disasm, const dasm_table_entry *entry);
	virtual ~unidasm_data_buffer() = default;

	void decrypt(const unidasm_data_buffer &buffer, bool opcode);

	virtual u8  r8 (offs_t pc) const override { return lr8 (pc); }
	virtual u16 r16(offs_t pc) const override { return lr16(pc); }
	virtual u32 r32(offs_t pc) const override { return lr32(pc); }
	virtual u64 r64(offs_t pc) const override { return lr64(pc); }

private:
	std::function<u8  (offs_t pc)> lr8;
	std::function<u16 (offs_t pc)> lr16;
	std::function<u32 (offs_t pc)> lr32;
	std::function<u64 (offs_t pc)> lr64;

	util::disasm_interface *disasm;
	const dasm_table_entry *entry;

	template<typename T> const T *get_ptr(offs_t pc) const {
		if(pc < base_pc)
			return nullptr;
		offs_t delta = (pc - base_pc) * sizeof(T);
		if(delta >= size)
			return nullptr;
		return reinterpret_cast<const T *>(&data[delta]);
	}

	template<typename T> T get(offs_t pc) const {
		auto p = get_ptr<T>(pc);
		return p ? *p : 0;
	}
};

struct dasm_line
{
	offs_t pc;
	offs_t size;
	std::string dasm;
};

unidasm_data_buffer::unidasm_data_buffer(util::disasm_interface *_disasm, const dasm_table_entry *_entry) : disasm(_disasm), entry(_entry)
{
	u32 flags = disasm->interface_flags();
	u32 page_mask = flags & util::disasm_interface::PAGED ? (1 << disasm->page_address_bits()) - 1 : 0;

	if(flags & util::disasm_interface::NONLINEAR_PC) {
		switch(entry->pcshift) {
		case -1:
			lr8  = [](offs_t pc) -> u8  { throw std::logic_error("debug_disasm_buffer::debug_data_buffer: r8 access on 16-bits granularity bus\n"); };
			lr16 = [this](offs_t pc) -> u16 {
				const u16 *src = get_ptr<u16>(pc);
				return src[0];
			};

			switch(entry->endian) {
			case le:
				lr32 = [this, page_mask](offs_t pc) -> u32 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u16>(disasm->pc_linear_to_real(lpc)) << (j*16);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				lr64 = [this, page_mask](offs_t pc) -> u64 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u64 r = 0;
					for(int j=0; j != 4; j++) {
						r |= u64(get<u16>(disasm->pc_linear_to_real(lpc))) << (j*16);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				break;

			case be:
				lr32 = [this, page_mask](offs_t pc) -> u32 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u16>(disasm->pc_linear_to_real(lpc)) << ((1-j)*16);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				lr64 = [this, page_mask](offs_t pc) -> u64 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u64 r = 0;
					for(int j=0; j != 4; j++) {
						r |= u64(get<u16>(disasm->pc_linear_to_real(lpc))) << ((3-j)*16);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				break;
			}
			break;

		case 0:
			lr8 = [this](offs_t pc) -> u8 {
				const u8 *src = get_ptr<u8>(pc);
				return src[0];
			};

			switch(entry->endian) {
			case le:
				lr16 = [this, page_mask](offs_t pc) -> u16 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u16 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u8>(disasm->pc_linear_to_real(lpc)) << (j*8);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				lr32 = [this, page_mask](offs_t pc) -> u32 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u8>(disasm->pc_linear_to_real(lpc)) << (j*8);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				lr64 = [this, page_mask](offs_t pc) -> u64 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u64 r = 0;
					for(int j=0; j != 8; j++) {
						r |= u64(get<u8>(disasm->pc_linear_to_real(lpc))) << (j*8);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				break;

			case be:
				lr16 = [this, page_mask](offs_t pc) -> u16 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u16 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u8>(disasm->pc_linear_to_real(lpc)) << ((1-j)*8);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				lr32 = [this, page_mask](offs_t pc) -> u32 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u8>(disasm->pc_linear_to_real(lpc)) << ((3-j)*8);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				lr64 = [this, page_mask](offs_t pc) -> u64 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u64 r = 0;
					for(int j=0; j != 8; j++) {
						r |= u64(get<u8>(disasm->pc_linear_to_real(lpc))) << ((7-j)*8);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				break;
			}
			break;
		}
	} else {
		switch(entry->pcshift) {
		case 0:
			lr8 = [this](offs_t pc) -> u8 {
				const u8 *p = get_ptr<u8>(pc);
				return p ?
				p[0]
				: 0x00;
			};
			switch(entry->endian) {
			case le:
				lr16 = [this](offs_t pc) -> u16 {
					const u8 *p = get_ptr<u8>(pc);
					return p ?
					p[0] |
					(p[1] << 8)
					: 0x0000;
				};
				lr32 = [this](offs_t pc) -> u32 {
					const u8 *p = get_ptr<u8>(pc);
					return p ?
					p[0] |
					(p[1] << 8) |
					(p[2] << 16) |
					(p[3] << 24)
					: 0x00000000;
				};
				lr64 = [this](offs_t pc) -> u64 {
					const u8 *p = get_ptr<u8>(pc);
					return p ?
					p[0] |
					(p[1] << 8) |
					(p[2] << 16) |
					(p[3] << 24) |
					(u64(p[4]) << 32) |
					(u64(p[5]) << 40) |
					(u64(p[6]) << 48) |
					(u64(p[7]) << 56)
					: 0x0000000000000000; };
				break;
			case be:
				lr16 = [this](offs_t pc) -> u16 {
					const u8 *p = get_ptr<u8>(pc);
					return p ?
					(p[0] << 8) |
					p[1]
					: 0x0000;
				};
				lr32 = [this](offs_t pc) -> u32 {
					const u8 *p = get_ptr<u8>(pc);
					return p ?
					(p[0] << 24) |
					(p[1] << 16) |
					(p[2] << 8) |
					p[3]
					: 0x00000000;
				};
				lr64 = [this](offs_t pc) -> u64 {
					const u8 *p = get_ptr<u8>(pc);
					return p ?
					(u64(p[0]) << 56) |
					(u64(p[1]) << 48) |
					(u64(p[2]) << 40) |
					(u64(p[3]) << 32) |
					(p[4] << 24) |
					(p[5] << 16) |
					(p[6] << 8) |
					p[7]
					: 0x0000000000000000; };
				break;
			}
			break;

		case -1:
			lr8 = [](offs_t pc) -> u8 { abort(); };
			lr16 = [this](offs_t pc) -> u16 {
				const u16 *p = get_ptr<u16>(pc);
				return p ?
				p[0]
				: 0x0000;
			};
			switch(entry->endian) {
			case le:
				lr32 = [this](offs_t pc) -> u32 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					p[0] |
					(p[1] << 16)
					: 0x00000000;
				};
				lr64 = [this](offs_t pc) -> u64 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					p[0] |
					(p[1] << 16) |
					(u64(p[2]) << 32) |
					(u64(p[3]) << 48)
					: 0x0000000000000000;
				};
				break;
			case be:
				lr32 = [this](offs_t pc) -> u32 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					(p[0] << 16)|
					p[1]
					: 0x00000000;
				};
				lr64 = [this](offs_t pc) -> u64 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					(u64(p[0]) << 48) |
					(u64(p[1]) << 32) |
					(p[2] << 16) |
					p[3]
					: 0x0000000000000000;
				};
				break;
			}
			break;

		case -2:
			lr8 = [](offs_t pc) -> u8 { abort(); };
			lr16 = [](offs_t pc) -> u16 { abort(); };
			lr32 = [this](offs_t pc) -> u32 {
				const u32 *p = get_ptr<u32>(pc);
				return p ?
				p[0]
				: 0x00000000;
			};
			switch(entry->endian) {
			case le:
				lr64 = [this](offs_t pc) -> u64 {
					const u32 *p = get_ptr<u32>(pc);
					return p ?
					p[0] |
					(u64(p[1]) << 32)
					: 0x0000000000000000;
				};
				break;
			case be:
				lr64 = [this](offs_t pc) -> u64 {
					const u32 *p = get_ptr<u32>(pc);
					return p ?
					(u64(p[0]) << 32) |
					p[1]
					: 0x0000000000000000;
				};
				break;
			}
			break;

		case -3:
			lr8 = [](offs_t pc) -> u8 { abort(); };
			lr16 = [](offs_t pc) -> u16 { abort(); };
			lr32 = [](offs_t pc) -> u32 { abort(); };
			lr64 = [this](offs_t pc) -> u64 {
				const u64 *p = get_ptr<u64>(pc);
				return p ?
				p[0]
				: 0x0000000000000000;
			};
			break;

		case 3:
			lr8 = [](offs_t pc) -> u8 { abort(); };
			lr16 = [this](offs_t pc) -> u16 {
				if(pc < base_pc)
					return 0x0000;
				offs_t delta = (pc - base_pc) >> 3;
				if(delta >= size)
					return 0x0000;
				return reinterpret_cast<const u16 *>(&data[delta])[0];
			};
			switch(entry->endian) {
			case le:
				lr32 = [this](offs_t pc) -> u32 {
					if(pc < base_pc)
						return 0x00000000;
					offs_t delta = (pc - base_pc) >> 3;
					if(delta >= size + 2)
						return 0x00000000;
					auto p = reinterpret_cast<const u16 *>(&data[delta]);
					return p[0] | (u32(p[1]) << 16);
				};
				break;
			case be:
				lr32 = [this](offs_t pc) -> u32 {
					if(pc < base_pc)
						return 0x00000000;
					offs_t delta = (pc - base_pc) >> 3;
					if(delta >= size + 2)
						return 0x00000000;
					auto p = reinterpret_cast<const u16 *>(&data[delta]);
					return (u32(p[0]) << 16) | p[1];
				};
				break;
			}
			lr64 = [](offs_t pc) -> u64 { abort(); };
			break;

		default:
			abort();
		}
	}
}

void unidasm_data_buffer::decrypt(const unidasm_data_buffer &buffer, bool opcode)
{
	abort();
}

static int parse_options(int argc, char *argv[], options *opts)
{
	bool pending_base = false;
	bool pending_arch = false;
	bool pending_mode = false;
	bool pending_skip = false;
	bool pending_count = false;

	memset(opts, 0, sizeof(*opts));

	// loop through arguments
	for(unsigned arg = 1; arg < argc; arg++) {
		char *curarg = argv[arg];

		// is it a switch?
		if(curarg[0] == '-') {
			if(pending_base || pending_arch || pending_mode || pending_skip || pending_count)
				goto usage;

			if(tolower((uint8_t)curarg[1]) == 'a')
				pending_arch = true;
			else if(tolower((uint8_t)curarg[1]) == 'b')
				pending_base = true;
			else if(tolower((uint8_t)curarg[1]) == 'f')
				opts->flipped = true;
			else if(tolower((uint8_t)curarg[1]) == 'l')
				opts->lower = true;
			else if(tolower((uint8_t)curarg[1]) == 'm')
				pending_mode = true;
			else if(tolower((uint8_t)curarg[1]) == 's')
				pending_skip = true;
			else if(tolower((uint8_t)curarg[1]) == 'c')
				pending_count = true;
			else if(tolower((uint8_t)curarg[1]) == 'n')
				opts->norawbytes = true;
			else if(tolower((uint8_t)curarg[1]) == 'u')
				opts->upper = true;
			else
				goto usage;
		}

		// base PC
		else if(pending_base)
		{
			int result;
			if(curarg[0] == '0' && curarg[1] == 'x')
				result = sscanf(&curarg[2], "%x", &opts->basepc);
			else if(curarg[0] == '$')
				result = sscanf(&curarg[1], "%x", &opts->basepc);
			else
				result = sscanf(&curarg[0], "%x", &opts->basepc);
			if(result != 1)
				goto usage;
			pending_base = false;
		}

		// mode
		else if(pending_mode)
		{
			if(sscanf(curarg, "%d", &opts->mode) != 1)
				goto usage;
			pending_mode = false;
		}

		// architecture
		else if(pending_arch) {
			int curarch;
			for(curarch = 0; curarch < ARRAY_LENGTH(dasm_table); curarch++)
				if(core_stricmp(curarg, dasm_table[curarch].name) == 0)
					break;
			if(curarch == ARRAY_LENGTH(dasm_table))
				goto usage;
			opts->dasm = &dasm_table[curarch];
			pending_arch = false;
		}

		// skip bytes
		else if(pending_skip) {
			if(sscanf(curarg, "%d", &opts->skip) != 1)
				goto usage;
			pending_skip = false;
		}

		// size
		else if(pending_count) {
			if(sscanf(curarg, "%d", &opts->count) != 1)
				goto usage;
			pending_count = false;
		}

		// filename
		else if(opts->filename == nullptr)
			opts->filename = curarg;

		// fail
		else
			goto usage;
	}

	// if we have a dangling option, error
	if(pending_base || pending_arch || pending_mode || pending_skip || pending_count)
		goto usage;

	// if no file or no architecture, fail
	if(opts->filename == nullptr || opts->dasm == nullptr)
		goto usage;
	return 0;

usage:
	printf("Usage: %s <filename> -arch <architecture> [-basepc <pc>] \n", argv[0]);
	printf("   [-mode <n>] [-norawbytes] [-flipped] [-upper] [-lower]\n");
	printf("   [-skip <n>] [-count <n>]\n");
	printf("\n");
	printf("Supported architectures:");
	const int colwidth = 1 + std::strlen(std::max_element(std::begin(dasm_table), std::end(dasm_table), [](const dasm_table_entry &a, const dasm_table_entry &b) { return std::strlen(a.name) < std::strlen(b.name); })->name);
	const int columns = std::max(1, 80 / colwidth);
	const int numrows = (ARRAY_LENGTH(dasm_table) + columns - 1) / columns;
	for(unsigned curarch = 0; curarch < numrows * columns; curarch++)
	{
		const int row = curarch / columns;
		const int col = curarch % columns;
		const int index = col * numrows + row;
		if(col == 0)
			printf("\n  ");
		printf("%-*s", colwidth, (index < ARRAY_LENGTH(dasm_table)) ? dasm_table[index].name : "");
	}
	printf("\n");
	return 1;
};


int main(int argc, char *argv[])
{
	// Parse options first
	options opts;
	if(parse_options(argc, argv, &opts))
		return 1;

	// Load the file
	void *data;
	uint32_t length;
	osd_file::error filerr = util::core_file::load(opts.filename, &data, length);
	if(filerr != osd_file::error::NONE)
	{
		fprintf(stderr, "Error opening file '%s'\n", opts.filename);
		return 1;
	}

	// Build the disasm object
	std::unique_ptr<util::disasm_interface> disasm(opts.dasm->alloc());
	u32 flags = disasm->interface_flags();

	// Compute the granularity in bytes (1-8)
	offs_t granularity = opts.dasm->pcshift < 0 ? disasm->opcode_alignment() << -opts.dasm->pcshift : disasm->opcode_alignment() >> opts.dasm->pcshift;

	// Build the base buffer and fill it (with a margin)
	unidasm_data_buffer base_buffer(disasm.get(), opts.dasm);
	u32 rounded_size = (((length - opts.skip) | (granularity - 1)) & ~(granularity - 1));
	base_buffer.data.resize(rounded_size + 8, 0x00);
	base_buffer.size = length;
	base_buffer.base_pc = opts.basepc;
	memcpy(&base_buffer.data[0], (const u8 *)data + opts.skip, length - opts.skip);

	// Build the decryption buffers if needed
	unidasm_data_buffer opcodes_buffer(disasm.get(), opts.dasm), params_buffer(disasm.get(), opts.dasm);
	if(flags & util::disasm_interface::INTERNAL_DECRYPTION) {
		opcodes_buffer.decrypt(base_buffer, true);
		if((flags & util::disasm_interface::SPLIT_DECRYPTION) == util::disasm_interface::SPLIT_DECRYPTION)
			params_buffer.decrypt(base_buffer, false);
	}

	// Select the buffers to actually use
	unidasm_data_buffer *popcodes = opcodes_buffer.data.empty() ? &base_buffer : &opcodes_buffer;
	unidasm_data_buffer *pparams  = params_buffer.data.empty() ? popcodes : &params_buffer;

	// Compute the pc warparound
	offs_t pclength = opts.dasm->pcshift < 0 ? rounded_size >> -opts.dasm->pcshift : rounded_size << opts.dasm->pcshift;
	offs_t limit = opts.basepc + pclength;
	offs_t pc_mask;
	if(!limit)
		pc_mask = 0xffffffff;
	else {
		for(pc_mask = 2; pc_mask && pc_mask < limit; pc_mask *= 2);
		pc_mask--;
	}

	// Compute the page warparound
	offs_t page_mask = flags & util::disasm_interface::PAGED ? (1 << disasm->page_address_bits()) - 1 : 0;

	// Next pc computation lambdas
	std::function<offs_t (offs_t, offs_t)> next_pc;
	std::function<offs_t (offs_t, offs_t)> next_pc_wrap;
	if(flags & util::disasm_interface::NONLINEAR_PC) {
		// lfsr pc is always paged
		next_pc = [pc_mask, page_mask, dis = disasm.get()](offs_t pc, offs_t size) {
			offs_t lpc = dis->pc_real_to_linear(pc);
			offs_t lpce = lpc + size;
			if((lpc ^ lpce) & ~page_mask)
				lpce = (lpc | page_mask) + 1;
			lpce &= pc_mask;
			return dis->pc_linear_to_real(lpce);
		};
		next_pc_wrap = [page_mask, dis = disasm.get()](offs_t pc, offs_t size) {
			offs_t lpc = dis->pc_real_to_linear(pc);
			offs_t lpce = (lpc & ~page_mask) | ((lpc + size) & page_mask);
			return dis->pc_linear_to_real(lpce);
		};

	} else if(flags & util::disasm_interface::PAGED) {
		next_pc = [pc_mask, page_mask](offs_t pc, offs_t size) {
			offs_t pce = pc + size;
			if((pc ^ pce) & ~page_mask)
				pce = (pc | page_mask) + 1;
			pce &= pc_mask;
			return pce;
		};
		next_pc_wrap = [page_mask](offs_t pc, offs_t size) {
			offs_t pce = (pc & ~page_mask) | ((pc + size) & page_mask);
			return pce;
		};

	} else {
		next_pc = [pc_mask](offs_t pc, offs_t size) {
			return (pc + size) & pc_mask;
		};
		next_pc_wrap = [pc_mask](offs_t pc, offs_t size) {
			return (pc + size) & pc_mask;
		};
	}

	// Compute the shift amount from pc delta to granularity-sized elements
	u32 granularity_shift = 31 - count_leading_zeros(disasm->opcode_alignment());

	// Number of pc steps to disassemble
	u32 count = pclength;

	if((count > opts.count) && (opts.count != 0))
		count = opts.count;

	// pc to string conversion
	std::function<std::string (offs_t pc)> pc_to_string;
	int aw = 32 - count_leading_zeros(pc_mask);
	bool is_octal = false; // Parameter?  Per-cpu config?
	if((flags & util::disasm_interface::PAGED2LEVEL) == util::disasm_interface::PAGED2LEVEL) {
		int bits1 = disasm->page_address_bits();
		int bits2 = disasm->page2_address_bits();
		int bits3 = aw - bits1 - bits2;
		offs_t sm1 = (1 << bits1) - 1;
		int sh2 = bits1;
		offs_t sm2 = (1 << bits2) - 1;
		int sh3 = bits1+bits2;

		if(is_octal) {
			int nc1 = (bits1+2)/3;
			int nc2 = (bits2+2)/3;
			int nc3 = (bits3+2)/3;
			pc_to_string = [nc1, nc2, nc3, sm1, sm2, sh2, sh3](offs_t pc) -> std::string {
				return util::string_format("%0*o:%0*o:%0*o",
										   nc3, pc >> sh3,
										   nc2, (pc >> sh2) & sm2,
										   nc1, pc & sm1);
			};
		} else {
			int nc1 = (bits1+3)/4;
			int nc2 = (bits2+3)/4;
			int nc3 = (bits3+3)/4;
			pc_to_string = [nc1, nc2, nc3, sm1, sm2, sh2, sh3](offs_t pc) -> std::string {
				return util::string_format("%0*x:%0*x:%0*x",
										   nc3, pc >> sh3,
										   nc2, (pc >> sh2) & sm2,
										   nc1, pc & sm1);
			};
		}

	} else if(flags & util::disasm_interface::PAGED) {
		int bits1 = disasm->page_address_bits();
		int bits2 = aw - bits1;
		offs_t sm1 = (1 << bits1) - 1;
		int sh2 = bits1;

		if(is_octal) {
			int nc1 = (bits1+2)/3;
			int nc2 = (bits2+2)/3;
			pc_to_string = [nc1, nc2, sm1, sh2](offs_t pc) -> std::string {
				return util::string_format("%0*o:%0*o",
										   nc2, pc >> sh2,
										   nc1, pc & sm1);
			};
		} else {
			int nc1 = (bits1+3)/4;
			int nc2 = (bits2+3)/4;
			pc_to_string = [nc1, nc2, sm1, sh2](offs_t pc) -> std::string {
				return util::string_format("%0*x:%0*x",
										   nc2, pc >> sh2,
										   nc1, pc & sm1);
			};
		}

	} else {
		int bits1 = aw;

		if(is_octal) {
			int nc1 = (bits1+2)/3;
			pc_to_string = [nc1](offs_t pc) -> std::string {
				return util::string_format("%0*o",
										   nc1, pc);
			};
		} else {
			int nc1 = (bits1+3)/4;
			pc_to_string = [nc1](offs_t pc) -> std::string {
				return util::string_format("%0*x",
										   nc1, pc);
			};
		}
	}

	// Lower/upper optional transform
	std::function<std::string (const std::string &)> tf;
	if(opts.lower)
		tf = [](const std::string &str) -> std::string {
			std::string result = str;
			std::transform(result.begin(), result.end(), result.begin(), [](char c) { return tolower(c); });
			return result;
		};
	else if(opts.upper)
		tf = [](const std::string &str) -> std::string {
			std::string result = str;
			std::transform(result.begin(), result.end(), result.begin(), [](char c) { return toupper(c); });
			return result;
		};
	else
		tf = [](const std::string &str) -> std::string {
			return str;
		};


	// Do the disassembly
	std::vector<dasm_line> dasm_lines;
	offs_t curpc = opts.basepc;
	for(u32 i=0; i < count;) {
		std::ostringstream stream;
		offs_t result = disasm->disassemble(stream, curpc, *popcodes, *pparams);
		offs_t len = result & util::disasm_interface::LENGTHMASK;
		dasm_lines.emplace_back(dasm_line{ curpc, len, stream.str() });
		curpc = next_pc(curpc, len);
		i += len;
	}

	// Compute the extrema
	offs_t max_len = 0;
	size_t max_text = 0;
	for(const auto &l : dasm_lines) {
		if(max_len < l.size)
			max_len = l.size;
		size_t s = l.dasm.size();
		if(max_text < s)
			max_text = s;
	}

	// Build the raw bytes generator and compute the field size
	max_len >>= granularity_shift;
	std::function<std::string (offs_t pc, offs_t size)> dump_raw_bytes;

	switch(granularity) {
	case 1:
		dump_raw_bytes = [step = disasm->opcode_alignment(), next_pc, base_buffer](offs_t pc, offs_t size) -> std::string {
			std::string result = "";
			for(offs_t i=0; i != size; i++) {
				if(i)
					result += ' ';
				result += util::string_format("%02x", base_buffer.r8(pc));
				pc = next_pc(pc, step);
			}
			return result;
		};
		max_len = (max_len * 3) - 1;
		break;

	case 2:
		dump_raw_bytes = [step = disasm->opcode_alignment(), next_pc, base_buffer](offs_t pc, offs_t size) -> std::string {
			std::string result = "";
			for(offs_t i=0; i != size; i++) {
				if(i)
					result += ' ';
				result += util::string_format("%04x", base_buffer.r16(pc));
				pc = next_pc(pc, step);
			}
			return result;
		};
		max_len = (max_len * 5) - 1;
		break;

	case 4:
		dump_raw_bytes = [step = disasm->opcode_alignment(), next_pc, base_buffer](offs_t pc, offs_t size) -> std::string {
			std::string result = "";
			for(offs_t i=0; i != size; i++) {
				if(i)
					result += ' ';
				result += util::string_format("%08x", base_buffer.r32(pc));
				pc = next_pc(pc, step);
			}
			return result;
		};
		max_len = (max_len * 9) - 1;
		break;

	case 8:
		dump_raw_bytes = [step = disasm->opcode_alignment(), next_pc, base_buffer](offs_t pc, offs_t size) -> std::string {
			std::string result = "";
			for(offs_t i=0; i != size; i++) {
				if(i)
					result += ' ';
				result += util::string_format("%016x", base_buffer.r64(pc));
				pc = next_pc(pc, step);
			}
			return result;
		};
		max_len = (max_len * 17) - 1;
		break;
	}

	if(opts.flipped) {
		if(opts.norawbytes)
			for(const auto &l : dasm_lines)
				util::stream_format(std::cout, "%-*s ; %s\n", max_text, tf(l.dasm), tf(pc_to_string(l.pc)));
		else
			for(const auto &l : dasm_lines)
				util::stream_format(std::cout, "%-*s ; %s: %s\n", max_text, tf(l.dasm), tf(pc_to_string(l.pc)), tf(dump_raw_bytes(l.pc, l.size >> granularity_shift)));
	} else {
		if(opts.norawbytes)
			for(const auto &l : dasm_lines)
				util::stream_format(std::cout, "%s: %s\n", tf(pc_to_string(l.pc)), tf(l.dasm));
		else
			for(const auto &l : dasm_lines)
				util::stream_format(std::cout, "%s: %-*s  %s\n", tf(pc_to_string(l.pc)), max_len, tf(dump_raw_bytes(l.pc, l.size >> granularity_shift)), tf(l.dasm));
	}

	free(data);

	return 0;
}
