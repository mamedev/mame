// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Universal disassembler.

****************************************************************************/

// the disassemblers assume they're in MAME and emu.h is a PCH, so we minimally pander to them
#include "coretmpl.h"
#include "disasmintf.h"

using offs_t = osd::u32;
using util::BIT;

#include "cpu/8x300/8x300dasm.h"
#include "cpu/adsp2100/2100dasm.h"
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
#include "cpu/bcp/bcpdasm.h"
#include "cpu/c33/c33dasm.h"
#include "cpu/capricorn/capricorn_dasm.h"
#include "cpu/ccpu/ccpudasm.h"
#include "cpu/cdc1700/cdc1700d.h"
#include "cpu/clipper/clipperd.h"
#include "cpu/cop400/cop410ds.h"
#include "cpu/cop400/cop420ds.h"
#include "cpu/cop400/cop424ds.h"
#include "cpu/cop400/cop444ds.h"
#include "cpu/cops1/cops1d.h"
#include "cpu/cosmac/cosdasm.h"
#include "cpu/cp1610/1610dasm.h"
#include "cpu/cr16b/cr16bdasm.h"
#include "cpu/cr16c/cr16cdasm.h"
#include "cpu/cubeqcpu/cubedasm.h"
#include "cpu/ddp516/ddp516d.h"
#include "cpu/dsp16/dsp16dis.h"
#include "cpu/dsp32/dsp32dis.h"
#include "cpu/dsp56000/dsp56000d.h"
#include "cpu/dsp56156/dsp56dsm.h"
#include "cpu/e0c6200/e0c6200d.h"
#include "cpu/e132xs/32xsdasm.h"
#include "cpu/es5510/es5510d.h"
#include "cpu/esrip/esripdsm.h"
#include "cpu/evolution/evod.h"
#include "cpu/f2mc16/f2mc16d.h"
#include "cpu/f8/f8dasm.h"
#include "cpu/fr/frdasm.h"
#include "cpu/g65816/g65816ds.h"
#include "cpu/gigatron/gigatrondasm.h"
#include "cpu/h6280/6280dasm.h"
#include "cpu/h8/gt913d.h"
#include "cpu/h8/h8d.h"
#include "cpu/h8/h8hd.h"
#include "cpu/h8/h8s2000d.h"
#include "cpu/h8/h8s2600d.h"
#include "cpu/h16/h16dasm.h"
#include "cpu/h8500/h8500dasm.h"
#include "cpu/hcd62121/hcd62121d.h"
#include "cpu/hd61700/hd61700d.h"
#include "cpu/hmcs40/hmcs40d.h"
#include "cpu/hmcs400/hmcs400d.h"
#include "cpu/hp2100/hp2100d.h"
#include "cpu/hpc/hpcdasm.h"
#include "cpu/hphybrid/hphybrid_dasm.h"
#include "cpu/ht1130/ht1130d.h"
#include "cpu/i386/i386dasm.h"
#include "cpu/i8008/8008dasm.h"
#include "cpu/i8085/8085dasm.h"
#include "cpu/i8089/i8089_dasm.h"
#include "cpu/i860/i860dis.h"
#include "cpu/i960/i960dis.h"
#include "cpu/ibm1800/ibm1800d.h"
#include "cpu/ie15/ie15dasm.h"
#include "cpu/interdata16/dasm16.h"
#include "cpu/jaguar/jagdasm.h"
#include "cpu/ks0164/ks0164d.h"
#include "cpu/lc57/lc57d.h"
#include "cpu/lc58/lc58d.h"
#include "cpu/lc8670/lc8670dsm.h"
#include "cpu/lh5801/5801dasm.h"
#include "cpu/lr35902/lr35902d.h"
#include "cpu/m16c/m16cdasm.h"
#include "cpu/m32c/m32cdasm.h"
#include "cpu/m37710/m7700ds.h"
#include "cpu/m6502/m4510d.h"
#include "cpu/m6502/m6502d.h"
#include "cpu/m6502/m6509d.h"
#include "cpu/m6502/m6510d.h"
#include "cpu/m6502/m65c02d.h"
#include "cpu/m6502/m65ce02d.h"
#include "cpu/m6502/m740d.h"
#include "cpu/m6502/r65c02d.h"
#include "cpu/m6502/r65c19d.h"
#include "cpu/m6502/xavixd.h"
#include "cpu/m6502/xavix2000d.h"
#include "cpu/m6800/6800dasm.h"
#include "cpu/m68000/m68kdasm.h"
#include "cpu/m6805/6805dasm.h"
#include "cpu/m6809/6x09dasm.h"
#include "cpu/m68hc16/cpu16dasm.h"
#include "cpu/m88000/m88000d.h"
#include "cpu/mb86233/mb86233d.h"
#include "cpu/mb86235/mb86235d.h"
#include "cpu/mb88xx/mb88dasm.h"
#include "cpu/mc68hc11/hc11dasm.h"
#include "cpu/mcs40/mcs40dasm.h"
#include "cpu/mcs48/mcs48dsm.h"
#include "cpu/mcs51/mcs51dasm.h"
#include "cpu/axc51/axc51dasm.h"
#include "cpu/mcs96/i8x9xd.h"
#include "cpu/mcs96/i8xc196d.h"
#include "cpu/melps4/melps4d.h"
#include "cpu/minx/minxd.h"
#include "cpu/mips/mips3dsm.h"
#include "cpu/mips/mips1dsm.h"
#include "cpu/mipsx/mipsxdasm.h"
#include "cpu/mk1/mk1dasm.h"
#include "cpu/mn1400/mn1400d.h"
#include "cpu/mn1610/mn1610d.h"
#include "cpu/mn1880/mn1880d.h"
#include "cpu/mn10200/mn102dis.h"
#include "cpu/mpk1839/kl1839vm1dasm.h"
#include "cpu/msm65x2/msm65x2d.h"
#include "cpu/nanoprocessor/nanoprocessor_dasm.h"
#include "cpu/nec/necdasm.h"
#include "cpu/nios2/nios2dasm.h"
#include "cpu/nova/novadasm.h"
#include "cpu/ns32000/ns32000d.h"
#include "cpu/nuon/nuondasm.h"
#include "cpu/olms66k/nx8dasm.h"
#include "cpu/pace/pacedasm.h"
#include "cpu/palm/palmd.h"
#include "cpu/patinhofeio/patinho_feio_dasm.h"
#include "cpu/pdp1/pdp1dasm.h"
#include "cpu/pdp8/pdp8dasm.h"
#include "cpu/pic16/pic16d.h"
#include "cpu/pic1670/pic1670d.h"
#include "cpu/pic16c5x/16c5xdsm.h"
#include "cpu/pic16c62x/16c62xdsm.h"
#include "cpu/pic17/pic17d.h"
#include "cpu/powerpc/ppc_dasm.h"
#include "cpu/pps4/pps4dasm.h"
#include "cpu/pps41/pps41d.h"
#include "cpu/psx/psxdasm.h"
#include "cpu/rii/riidasm.h"
#include "cpu/romp/rompdasm.h"
#include "cpu/rsp/rsp_dasm.h"
#include "cpu/rw5000/rw5000d.h"
#include "cpu/rx01/rx01dasm.h"
#include "cpu/s2650/2650dasm.h"
#include "cpu/saturn/saturnds.h"
#include "cpu/sc61860/scdasm.h"
#include "cpu/scmp/scmpdasm.h"
#include "cpu/score/scoredsm.h"
#include "cpu/scudsp/scudspdasm.h"
#include "cpu/se3208/se3208dis.h"
#include "cpu/sh/sh_dasm.h"
#include "cpu/sharc/sharcdsm.h"
#include "cpu/sigma2/sigma2d.h"
#include "cpu/sm510/sm510d.h"
#include "cpu/sm8500/sm8500d.h"
#include "cpu/sparc/sparcdasm.h"
#include "cpu/spc700/spc700ds.h"
#include "cpu/ssem/ssemdasm.h"
#include "cpu/ssp1601/ssp1601d.h"
#include "cpu/st62xx/st62xx_dasm.h"
#include "cpu/st9/st9dasm.h"
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
#include "cpu/tx0/tx0dasm.h"
#include "cpu/ucom4/ucom4d.h"
#include "cpu/unsp/unspdasm.h"
#include "cpu/upd177x/upd177xd.h"
#include "cpu/upd7725/dasm7725.h"
#include "cpu/upd7810/upd7810_dasm.h"
#include "cpu/upd78k/upd78k0d.h"
#include "cpu/upd78k/upd78k1d.h"
#include "cpu/upd78k/upd78k2d.h"
#include "cpu/upd78k/upd78k3d.h"
#include "cpu/upd78k/upd78k4d.h"
#include "cpu/v60/v60d.h"
#include "cpu/v620/v620dasm.h"
#include "cpu/v810/v810dasm.h"
#include "cpu/v850/v850dasm.h"
#include "cpu/vax/vaxdasm.h"
#include "cpu/vt50/vt50dasm.h"
#include "cpu/vt61/vt61dasm.h"
#include "cpu/we32000/we32100d.h"
#include "cpu/xa/xadasm.h"
#include "cpu/xavix2/xavix2d.h"
#include "cpu/xtensa/xtensad.h"
#include "cpu/z180/z180dasm.h"
#include "cpu/z8/z8dasm.h"
#include "cpu/z80/r800dasm.h"
#include "cpu/z80/z80dasm.h"
#include "cpu/z8000/8000dasm.h"

#include "corestr.h"
#include "eminline.h"
#include "endianness.h"
#include "ioprocs.h"
#include "osdfile.h"
#include "strformat.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#endif


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

// Configuration missing
struct nec_unidasm_t : nec_disassembler::config
{
	int mode;
	nec_unidasm_t() { mode = 1; }
	virtual ~nec_unidasm_t() override = default;
	virtual int get_mode() const override { return mode; }
} nec_unidasm;


static constexpr auto le = util::endianness::little;
static constexpr auto be = util::endianness::big;

struct dasm_table_entry
{
	const char *            name;
	util::endianness        endian;
	int8_t                  pcshift;
	std::function<util::disasm_interface *()> alloc;
};


struct options
{
	const char *            filename;
	offs_t                  basepc;
	uint8_t                 norawbytes;
	uint8_t                 xchbytes;
	uint8_t                 lower;
	uint8_t                 upper;
	uint8_t                 flipped;
	const dasm_table_entry *dasm;
	uint32_t                skip;
	uint32_t                count;
	bool                    octal;
};

static const dasm_table_entry dasm_table[] =
{
	{ "8x300",           be, -1, []() -> util::disasm_interface * { return new n8x300_disassembler; } },
	{ "adsp21xx",        le, -2, []() -> util::disasm_interface * { return new adsp21xx_disassembler; } },
	{ "alpha",           le,  0, []() -> util::disasm_interface * { return new alpha_disassembler; } },
	{ "alpha_nt",        le,  0, []() -> util::disasm_interface * { return new alpha_disassembler(alpha_disassembler::TYPE_NT); } },
	{ "alpha_unix",      le,  0, []() -> util::disasm_interface * { return new alpha_disassembler(alpha_disassembler::TYPE_UNIX); } },
	{ "alpha_vms",       le,  0, []() -> util::disasm_interface * { return new alpha_disassembler(alpha_disassembler::TYPE_VMS); } },
	{ "alto2",           be, -2, []() -> util::disasm_interface * { return new alto2_disassembler; } },
	{ "a5000",           le,  0, []() -> util::disasm_interface * { return new a5000_disassembler; } },
	{ "a5500",           le,  0, []() -> util::disasm_interface * { return new a5500_disassembler; } },
	{ "am29000",         be,  0, []() -> util::disasm_interface * { return new am29000_disassembler; } },
	{ "amis2000",        le,  0, []() -> util::disasm_interface * { return new amis2000_disassembler; } },
	{ "apexc",           be,  0, []() -> util::disasm_interface * { return new apexc_disassembler; } },
	{ "arc",             be,  0, []() -> util::disasm_interface * { return new arc_disassembler; } },
	{ "arcompact",       le,  0, []() -> util::disasm_interface * { return new arcompact_disassembler; } },
	{ "arm",             le,  0, []() -> util::disasm_interface * { return new arm_disassembler; } },
	{ "arm7",            le,  0, []() -> util::disasm_interface * { arm7_unidasm.t_flag = false; return new arm7_disassembler(&arm7_unidasm); } },
	{ "arm7_be",         be,  0, []() -> util::disasm_interface * { arm7_unidasm.t_flag = false; return new arm7_disassembler(&arm7_unidasm); } },
	{ "arm7thumb",       le,  0, []() -> util::disasm_interface * { arm7_unidasm.t_flag = true; return new arm7_disassembler(&arm7_unidasm); } },
	{ "arm7thumbb",      be,  0, []() -> util::disasm_interface * { arm7_unidasm.t_flag = true; return new arm7_disassembler(&arm7_unidasm); } },
	{ "asap",            le,  0, []() -> util::disasm_interface * { return new asap_disassembler; } },
	{ "avr8",            le,  0, []() -> util::disasm_interface * { return new avr8_disassembler; } },
	{ "axc51core",       le,  0, []() -> util::disasm_interface * { return new axc51core_disassembler; } },
	{ "axc208",          le,  0, []() -> util::disasm_interface * { return new ax208_disassembler; } },
	{ "b5000",           le,  0, []() -> util::disasm_interface * { return new b5000_disassembler; } },
	{ "b5500",           le,  0, []() -> util::disasm_interface * { return new b5500_disassembler; } },
	{ "b6000",           le,  0, []() -> util::disasm_interface * { return new b6000_disassembler; } },
	{ "b6100",           le,  0, []() -> util::disasm_interface * { return new b6100_disassembler; } },
	{ "c33",             le,  0, []() -> util::disasm_interface * { return new c33_disassembler; } },
	{ "capricorn",       le,  0, []() -> util::disasm_interface * { return new capricorn_disassembler; } },
	{ "ccpu",            le,  0, []() -> util::disasm_interface * { return new ccpu_disassembler; } },
	{ "cdc1700",         be, -1, []() -> util::disasm_interface * { return new cdc1700_disassembler; } },
	{ "cdp1801",         le,  0, []() -> util::disasm_interface * { return new cosmac_disassembler(cosmac_disassembler::TYPE_1801); } },
	{ "cdp1802",         le,  0, []() -> util::disasm_interface * { return new cosmac_disassembler(cosmac_disassembler::TYPE_1802); } },
	{ "cdp1805",         le,  0, []() -> util::disasm_interface * { return new cosmac_disassembler(cosmac_disassembler::TYPE_1805); } },
	{ "clipper",         le,  0, []() -> util::disasm_interface * { return new clipper_disassembler; } },
	{ "coldfire",        be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_COLDFIRE); } },
	{ "cop410",          le,  0, []() -> util::disasm_interface * { return new cop410_disassembler; } },
	{ "cop420",          le,  0, []() -> util::disasm_interface * { return new cop420_disassembler; } },
	{ "cop444",          le,  0, []() -> util::disasm_interface * { return new cop444_disassembler; } },
	{ "cop424",          le,  0, []() -> util::disasm_interface * { return new cop424_disassembler; } },
	{ "cp1610",          be, -1, []() -> util::disasm_interface * { return new cp1610_disassembler; } },
	{ "cpu16",           be,  0, []() -> util::disasm_interface * { return new cpu16_disassembler; } },
	{ "cr16a",           le,  0, []() -> util::disasm_interface * { return new cr16a_disassembler; } },
	{ "cr16b",           le,  0, []() -> util::disasm_interface * { return new cr16b_disassembler; } },
	{ "cr16c",           le,  0, []() -> util::disasm_interface * { return new cr16c_disassembler; } },
	{ "cquestlin",       be, -3, []() -> util::disasm_interface * { return new cquestlin_disassembler; } },
	{ "cquestrot",       be, -3, []() -> util::disasm_interface * { return new cquestrot_disassembler; } },
	{ "cquestsnd",       be, -3, []() -> util::disasm_interface * { return new cquestsnd_disassembler; } },
	{ "cyber18",         be, -1, []() -> util::disasm_interface * { return new cyber18_disassembler; } },
	{ "ddp516",          be, -1, []() -> util::disasm_interface * { return new ddp516_disassembler; } },
	{ "dp8344",          le, -1, []() -> util::disasm_interface * { return new dp8344_disassembler; } },
	{ "ds5002fp",        le,  0, []() -> util::disasm_interface * { return new ds5002fp_disassembler; } },
	{ "ds80c320",        le,  0, []() -> util::disasm_interface * { return new ds80c320_disassembler; } },
	{ "dsp16",           le, -1, []() -> util::disasm_interface * { return new dsp16_disassembler; } },
	{ "dsp32c",          le,  0, []() -> util::disasm_interface * { return new dsp32c_disassembler; } },
	{ "dsp56000",        be, -2, []() -> util::disasm_interface * { return new dsp56000_disassembler; } },
	{ "dsp56156",        le, -1, []() -> util::disasm_interface * { return new dsp56156_disassembler; } },
	{ "e0c6200",         be, -1, []() -> util::disasm_interface * { return new e0c6200_disassembler; } },
	{ "epg3231",         le, -1, []() -> util::disasm_interface * { return new epg3231_disassembler; } },
//  { "es5510",          be,  0, []() -> util::disasm_interface * { return new es5510_disassembler; } }, // Currently does nothing
	{ "esrip",           be,  0, []() -> util::disasm_interface * { return new esrip_disassembler; } },
	{ "evo",             le, -1, []() -> util::disasm_interface * { return new evolution_disassembler; } },
	{ "f2mc16",          le,  0, []() -> util::disasm_interface * { return new f2mc16_disassembler; } },
	{ "f8",              be,  0, []() -> util::disasm_interface * { return new f8_disassembler; } },
	{ "fr",              be,  0, []() -> util::disasm_interface * { return new fr_disassembler; } },
	{ "fscpu32",         be,  0, []() -> util::disasm_interface * { return new m68k_disassembler(m68k_disassembler::TYPE_CPU32); } },
	{ "g65816",          le,  0, []() -> util::disasm_interface * { return new g65816_disassembler(&g65816_unidasm); } },
	{ "gigatron",        be, -1, []() -> util::disasm_interface * { return new gigatron_disassembler; } },
	{ "gt913",           be,  0, []() -> util::disasm_interface * { return new gt913_disassembler; } },
	{ "h6280",           le,  0, []() -> util::disasm_interface * { return new h6280_disassembler; } },
	{ "h8",              be,  0, []() -> util::disasm_interface * { return new h8_disassembler; } },
	{ "h8h",             be,  0, []() -> util::disasm_interface * { return new h8h_disassembler; } },
	{ "h8s2000",         be,  0, []() -> util::disasm_interface * { return new h8s2000_disassembler; } },
	{ "h8s2600",         be,  0, []() -> util::disasm_interface * { return new h8s2600_disassembler; } },
	{ "h8500",           be,  0, []() -> util::disasm_interface * { return new h8500_disassembler; } },
	{ "h16",             be,  0, []() -> util::disasm_interface * { return new h16_disassembler; } },
	{ "hc11",            be,  0, []() -> util::disasm_interface * { return new hc11_disassembler; } },
	{ "hcd62121",        le,  0, []() -> util::disasm_interface * { return new hcd62121_disassembler; } },
	{ "hd6120",          be, -1, []() -> util::disasm_interface * { return new hd6120_disassembler; } },
	{ "hd61700",         le, -1, []() -> util::disasm_interface * { return new hd61700_disassembler; } },
	{ "hd6301",          be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(6301); } },
	{ "hd6309",          be,  0, []() -> util::disasm_interface * { return new hd6309_disassembler; } },
	{ "hd63701",         be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(63701); } },
	{ "hmcs40",          le, -1, []() -> util::disasm_interface * { return new hmcs40_disassembler; } },
	{ "hmcs400",         le, -1, []() -> util::disasm_interface * { return new hmcs400_disassembler; } },
	{ "hp2100",          be, -1, []() -> util::disasm_interface * { return new hp2100_disassembler; } },
	{ "hp21mx",          be, -1, []() -> util::disasm_interface * { return new hp21mx_disassembler; } },
	{ "hp_5061_3001",    be, -1, []() -> util::disasm_interface * { return new hp_5061_3001_disassembler; } },
	{ "hp_5061_3011",    be, -1, []() -> util::disasm_interface * { return new hp_5061_3011_disassembler; } },
	{ "hp_09825_67907",  be, -1, []() -> util::disasm_interface * { return new hp_09825_67907_disassembler; } },
	{ "hpc16083",        le,  0, []() -> util::disasm_interface * { return new hpc16083_disassembler; } },
	{ "hpc16164",        le,  0, []() -> util::disasm_interface * { return new hpc16164_disassembler; } },
	{ "hyperstone",      be,  0, []() -> util::disasm_interface * { return new hyperstone_disassembler(&hyperstone_unidasm); } },
	{ "ht1130",          le,  0, []() -> util::disasm_interface * { return new ht1130_disassembler; } },
	{ "i4004",           le,  0, []() -> util::disasm_interface * { return new i4004_disassembler; } },
	{ "i4040",           le,  0, []() -> util::disasm_interface * { return new i4040_disassembler; } },
	{ "i8008",           le,  0, []() -> util::disasm_interface * { return new i8008_disassembler; } },
	{ "i802x",           le,  0, []() -> util::disasm_interface * { return new mcs48_disassembler(false, true); } },
	{ "i8051",           le,  0, []() -> util::disasm_interface * { return new i8051_disassembler; } },
	{ "i8052",           le,  0, []() -> util::disasm_interface * { return new i8052_disassembler; } },
	{ "i8085",           le,  0, []() -> util::disasm_interface * { return new i8085_disassembler; } },
	{ "i8089",           le,  0, []() -> util::disasm_interface * { return new i8089_disassembler; } },
	{ "i80c52",          le,  0, []() -> util::disasm_interface * { return new i80c52_disassembler; } },
	{ "i860",            le,  0, []() -> util::disasm_interface * { return new i860_disassembler; } },
	{ "i8x9x",           le,  0, []() -> util::disasm_interface * { return new i8x9x_disassembler; } },
	{ "i8xc196",         le,  0, []() -> util::disasm_interface * { return new i8xc196_disassembler; } },
	{ "i8xc51fx",        le,  0, []() -> util::disasm_interface * { return new i8xc51fx_disassembler; } },
	{ "i8xc51gb",        le,  0, []() -> util::disasm_interface * { return new i8xc51gb_disassembler; } },
	{ "i960",            le,  0, []() -> util::disasm_interface * { return new i960_disassembler; } },
	{ "ibm1130",         be, -1, []() -> util::disasm_interface * { return new ibm1130_disassembler; } },
	{ "ibm1800",         be, -1, []() -> util::disasm_interface * { return new ibm1800_disassembler; } },
	{ "ie15",            le,  0, []() -> util::disasm_interface * { return new ie15_disassembler; } },
	{ "interdata16",     be,  0, []() -> util::disasm_interface * { return new interdata16_disassembler; } },
	{ "jaguardsp",       be,  0, []() -> util::disasm_interface * { return new jaguar_disassembler(jaguar_disassembler::variant::DSP); } },
	{ "jaguargpu",       be,  0, []() -> util::disasm_interface * { return new jaguar_disassembler(jaguar_disassembler::variant::GPU); } },
	{ "konami",          be,  0, []() -> util::disasm_interface * { return new konami_disassembler; } },
	{ "ks0164",          be,  0, []() -> util::disasm_interface * { return new ks0164_disassembler; } },
	{ "kl1839vm1",       be,  0, []() -> util::disasm_interface * { return new kl1839vm1_disassembler; } },
	{ "lc57",            be,  0, []() -> util::disasm_interface * { return new lc57_disassembler; } },
	{ "lc58",            be, -1, []() -> util::disasm_interface * { return new lc58_disassembler; } },
	{ "lc8670",          be,  0, []() -> util::disasm_interface * { return new lc8670_disassembler; } },
	{ "lh5801",          le,  0, []() -> util::disasm_interface * { return new lh5801_disassembler; } },
	{ "lr35902",         le,  0, []() -> util::disasm_interface * { return new lr35902_disassembler; } },
	{ "m146805",         be,  0, []() -> util::disasm_interface * { return new m146805_disassembler; } },
	{ "m16c",            le,  0, []() -> util::disasm_interface * { return new m16c_disassembler; } },
	{ "m32c",            le,  0, []() -> util::disasm_interface * { return new m32c_disassembler; } },
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
	{ "m68hc05",         be,  0, []() -> util::disasm_interface * { return new m68hc05_disassembler; } },
	{ "m740",            le,  0, []() -> util::disasm_interface * { return new m740_disassembler(&m740_unidasm); } },
	{ "mb86233",         le, -2, []() -> util::disasm_interface * { return new mb86233_disassembler; } },
	{ "mb86235",         le, -3, []() -> util::disasm_interface * { return new mb86235_disassembler; } },
	{ "mb88",            le,  0, []() -> util::disasm_interface * { return new mb88_disassembler; } },
	{ "mc88100",         be,  0, []() -> util::disasm_interface * { return new mc88100_disassembler; } },
	{ "mc88110",         be,  0, []() -> util::disasm_interface * { return new mc88110_disassembler; } },
	{ "mcs48",           le,  0, []() -> util::disasm_interface * { return new mcs48_disassembler(false, false); } },
	{ "minx",            le,  0, []() -> util::disasm_interface * { return new minx_disassembler; } },
	{ "mips1be",         be,  0, []() -> util::disasm_interface * { return new mips1_disassembler; } },
	{ "mips1le",         le,  0, []() -> util::disasm_interface * { return new mips1_disassembler; } },
	{ "mips3be",         be,  0, []() -> util::disasm_interface * { return new mips3_disassembler; } },
	{ "mips3le",         le,  0, []() -> util::disasm_interface * { return new mips3_disassembler; } },
	{ "mipsx",           be,  0, []() -> util::disasm_interface * { return new mipsx_disassembler; } },
	{ "mk1",             le,  0, []() -> util::disasm_interface * { return new mk1_disassembler; } },
	{ "mm5799",          le,  0, []() -> util::disasm_interface * { return new mm5799_disassembler; } },
	{ "mm76",            le,  0, []() -> util::disasm_interface * { return new mm76_disassembler; } },
	{ "mm78",            le,  0, []() -> util::disasm_interface * { return new mm78_disassembler; } },
	{ "mn10200",         le,  0, []() -> util::disasm_interface * { return new mn10200_disassembler; } },
	{ "mn1400",          le,  0, []() -> util::disasm_interface * { return new mn1400_disassembler; } },
	{ "mn1610",          be, -1, []() -> util::disasm_interface * { return new mn1610_disassembler; } },
	{ "mn1613",          be, -1, []() -> util::disasm_interface * { return new mn1613_disassembler; } },
	{ "mn1860",          be,  0, []() -> util::disasm_interface * { return new mn1860_disassembler; } },
	{ "mn1870",          be,  0, []() -> util::disasm_interface * { return new mn1870_disassembler; } },
	{ "mn1880",          be,  0, []() -> util::disasm_interface * { return new mn1880_disassembler; } },
	{ "msm65x2",         le,  0, []() -> util::disasm_interface * { return new msm65x2_disassembler; } },
	{ "nanoprocessor",   le,  0, []() -> util::disasm_interface * { return new hp_nanoprocessor_disassembler; } },
	{ "nec",             le,  0, []() -> util::disasm_interface * { return new nec_disassembler(&nec_unidasm); } },
	{ "nios2",           le,  0, []() -> util::disasm_interface * { return new nios2_disassembler; } },
	{ "nova",            be, -1, []() -> util::disasm_interface * { return new nova_disassembler; } },
	{ "ns32000",         le,  0, []() -> util::disasm_interface * { return new ns32000_disassembler; } },
	{ "nuon",            be,  0, []() -> util::disasm_interface * { return new nuon_disassembler; } },
	{ "nsc8105",         be,  0, []() -> util::disasm_interface * { return new m680x_disassembler(8105); } },
	{ "nx8_500s",        le,  0, []() -> util::disasm_interface * { return new nx8_500s_disassembler; } },
	{ "p8xc552",         le,  0, []() -> util::disasm_interface * { return new p8xc552_disassembler; } },
	{ "p8xc562",         le,  0, []() -> util::disasm_interface * { return new p8xc562_disassembler; } },
	{ "pace",            le, -1, []() -> util::disasm_interface * { return new pace_disassembler; } },
	{ "palm",            be,  0, []() -> util::disasm_interface * { return new palm_disassembler; } },
	{ "patinho_feio",    le,  0, []() -> util::disasm_interface * { return new patinho_feio_disassembler; } },
	{ "pdp1",            be, -2, []() -> util::disasm_interface * { return new pdp1_disassembler; } },
	{ "pdp8",            be, -1, []() -> util::disasm_interface * { return new pdp8_disassembler; } },
	{ "pic16",           le, -1, []() -> util::disasm_interface * { return new pic16_disassembler; } },
	{ "pic16c5x",        le, -1, []() -> util::disasm_interface * { return new pic16c5x_disassembler; } },
	{ "pic1670",         le, -1, []() -> util::disasm_interface * { return new pic1670_disassembler; } },
	{ "pic16c62x",       le, -1, []() -> util::disasm_interface * { return new pic16c62x_disassembler; } },
	{ "pic17",           le, -1, []() -> util::disasm_interface * { return new pic17_disassembler; } },
	{ "power",           be,  0, []() -> util::disasm_interface * { return new power_disassembler; } },
	{ "powerpc",         be,  0, []() -> util::disasm_interface * { return new powerpc_disassembler; } },
	{ "pps4",            le,  0, []() -> util::disasm_interface * { return new pps4_disassembler; } },
	{ "prime16s",        be, -1, []() -> util::disasm_interface * { return new prime16s_disassembler; } },
	{ "prime32r",        be, -1, []() -> util::disasm_interface * { return new prime32r_disassembler; } },
	{ "prime32s",        be, -1, []() -> util::disasm_interface * { return new prime32s_disassembler; } },
	{ "prime64r",        be, -1, []() -> util::disasm_interface * { return new prime64r_disassembler; } },
	{ "prime64v",        be, -1, []() -> util::disasm_interface * { return new prime64v_disassembler; } },
	{ "psxcpu",          le,  0, []() -> util::disasm_interface * { return new psxcpu_disassembler; } },
	{ "r65c02",          le,  0, []() -> util::disasm_interface * { return new r65c02_disassembler; } },
	{ "r65c19",          le,  0, []() -> util::disasm_interface * { return new r65c19_disassembler; } },
	{ "r800",            le,  0, []() -> util::disasm_interface * { return new r800_disassembler; } },
	{ "romp",            be,  0, []() -> util::disasm_interface * { return new romp_disassembler; } },
	{ "rsp",             le,  0, []() -> util::disasm_interface * { return new rsp_disassembler; } },
	{ "rupi44",          le,  0, []() -> util::disasm_interface * { return new rupi44_disassembler; } },
	{ "rx01",            le,  0, []() -> util::disasm_interface * { return new rx01_disassembler; } },
	{ "s2650",           be,  0, []() -> util::disasm_interface * { return new s2650_disassembler(&s2650_unidasm); } },
	{ "sab80515",        le,  0, []() -> util::disasm_interface * { return new sab80515_disassembler; } },
	{ "sab80c515",       le,  0, []() -> util::disasm_interface * { return new sab80c515_disassembler; } },
	{ "saturn",          le,  0, []() -> util::disasm_interface * { return new saturn_disassembler(&saturn_unidasm); } },
	{ "sc61860",         le,  0, []() -> util::disasm_interface * { return new sc61860_disassembler; } },
	{ "scmp",            be,  0, []() -> util::disasm_interface * { return new scmp_disassembler; } },
	{ "score7",          le,  0, []() -> util::disasm_interface * { return new score7_disassembler; } },
	{ "scudsp",          be, -2, []() -> util::disasm_interface * { return new scudsp_disassembler; } },
	{ "se3208",          le,  0, []() -> util::disasm_interface * { return new se3208_disassembler; } },
	{ "sh2",             be,  0, []() -> util::disasm_interface * { return new sh_disassembler(false); } },
	{ "sh4",             le,  0, []() -> util::disasm_interface * { return new sh_disassembler(true); } },
	{ "sh4be",           be,  0, []() -> util::disasm_interface * { return new sh_disassembler(true); } },
	{ "sharc",           le, -3, []() -> util::disasm_interface * { return new sharc_disassembler; } },
	{ "sigma2",          be, -1, []() -> util::disasm_interface * { return new sigma2_disassembler; } },
	{ "sm500",           le,  0, []() -> util::disasm_interface * { return new sm500_disassembler; } },
	{ "sm510",           le,  0, []() -> util::disasm_interface * { return new sm510_disassembler; } },
	{ "sm511",           le,  0, []() -> util::disasm_interface * { return new sm511_disassembler; } },
	{ "sm530",           le,  0, []() -> util::disasm_interface * { return new sm530_disassembler; } },
	{ "sm590",           le,  0, []() -> util::disasm_interface * { return new sm590_disassembler; } },
	{ "sm5a",            le,  0, []() -> util::disasm_interface * { return new sm5a_disassembler; } },
	{ "sm8500",          le,  0, []() -> util::disasm_interface * { return new sm8500_disassembler; } },
	{ "smc1102",         le,  0, []() -> util::disasm_interface * { return new smc1102_disassembler; } },
	{ "sparclite",       be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::sparclite); } },
	{ "sparcv7",         be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::v7); } },
	{ "sparcv8",         be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::v8); } },
	{ "sparcv9",         be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::v9); } },
	{ "sparcv9vis1",     be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::v9, sparc_disassembler::vis_1); } },
	{ "sparcv9vis2",     be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::v9, sparc_disassembler::vis_2); } },
	{ "sparcv9vis2p",    be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::v9, sparc_disassembler::vis_2p); } },
	{ "sparcv9vis3",     be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::v9, sparc_disassembler::vis_3); } },
	{ "sparcv9vis3b",    be,  0, []() -> util::disasm_interface * { return new sparc_disassembler(nullptr, sparc_disassembler::v9, sparc_disassembler::vis_3b); } },
	{ "spc700",          le,  0, []() -> util::disasm_interface * { return new spc700_disassembler; } },
	{ "ssem",            le,  0, []() -> util::disasm_interface * { return new ssem_disassembler; } },
	{ "ssp1601",         be, -1, []() -> util::disasm_interface * { return new ssp1601_disassembler; } },
	{ "st62xx",          le,  0, []() -> util::disasm_interface * { return new st62xx_disassembler; } },
	{ "st9",             be,  0, []() -> util::disasm_interface * { return new st9_disassembler; } },
	{ "st9p",            be,  0, []() -> util::disasm_interface * { return new st9p_disassembler; } },
	{ "superfx",         le,  0, []() -> util::disasm_interface * { return new superfx_disassembler(&superfx_unidasm); } },
	{ "t11",             le,  0, []() -> util::disasm_interface * { return new t11_disassembler; } },
	{ "tlcs870",         le,  0, []() -> util::disasm_interface * { return new tlcs870_disassembler; } },
	{ "tlcs900",         le,  0, []() -> util::disasm_interface * { return new tlcs900_disassembler; } },
	{ "tmp90c051",       le,  0, []() -> util::disasm_interface * { return new tmp90c051_disassembler; } },
	{ "tmp90840",        le,  0, []() -> util::disasm_interface * { return new tmp90840_disassembler; } },
	{ "tmp90844",        le,  0, []() -> util::disasm_interface * { return new tmp90844_disassembler; } },
	{ "tms0980",         be,  0, []() -> util::disasm_interface * { return new tms0980_disassembler; } },
	{ "tms1000",         le,  0, []() -> util::disasm_interface * { return new tms1000_disassembler; } },
	{ "tms1100",         le,  0, []() -> util::disasm_interface * { return new tms1100_disassembler; } },
	{ "tms1400",         le,  0, []() -> util::disasm_interface * { return new tms1400_disassembler; } },
	{ "tms2100",         le,  0, []() -> util::disasm_interface * { return new tms2100_disassembler; } },
	{ "tms2400",         le,  0, []() -> util::disasm_interface * { return new tms2400_disassembler; } },
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
	{ "tx0_8kwo",        be, -2, []() -> util::disasm_interface * { return new tx0_8kwo_disassembler; } },
	{ "ucom4",           le,  0, []() -> util::disasm_interface * { return new ucom4_disassembler; } },
	{ "unsp10",          be, -1, []() -> util::disasm_interface * { return new unsp_disassembler; } },
	{ "unsp12",          be, -1, []() -> util::disasm_interface * { return new unsp_12_disassembler; } },
	{ "unsp20",          be, -1, []() -> util::disasm_interface * { return new unsp_20_disassembler; } },
	{ "upd177x",         be, -1, []() -> util::disasm_interface * { return new upd177x_disassembler; } },
	{ "upd7725",         be, -2, []() -> util::disasm_interface * { return new necdsp_disassembler; } },
	{ "upd7801",         le,  0, []() -> util::disasm_interface * { return new upd7801_disassembler; } },
	{ "upd78c05",        le,  0, []() -> util::disasm_interface * { return new upd78c05_disassembler; } },
	{ "upd7807",         le,  0, []() -> util::disasm_interface * { return new upd7807_disassembler; } },
	{ "upd7810",         le,  0, []() -> util::disasm_interface * { return new upd7810_disassembler; } },
	{ "upd78014",        le,  0, []() -> util::disasm_interface * { return new upd78014_disassembler; } },
	{ "upd78024",        le,  0, []() -> util::disasm_interface * { return new upd78024_disassembler; } },
	{ "upd78044a",       le,  0, []() -> util::disasm_interface * { return new upd78044a_disassembler; } },
	{ "upd78054",        le,  0, []() -> util::disasm_interface * { return new upd78054_disassembler; } },
	{ "upd78064",        le,  0, []() -> util::disasm_interface * { return new upd78064_disassembler; } },
	{ "upd78078",        le,  0, []() -> util::disasm_interface * { return new upd78078_disassembler; } },
	{ "upd78083",        le,  0, []() -> util::disasm_interface * { return new upd78083_disassembler; } },
	{ "upd78138",        le,  0, []() -> util::disasm_interface * { return new upd78138_disassembler; } },
	{ "upd78148",        le,  0, []() -> util::disasm_interface * { return new upd78148_disassembler; } },
	{ "upd78214",        le,  0, []() -> util::disasm_interface * { return new upd78214_disassembler; } },
	{ "upd78218a",       le,  0, []() -> util::disasm_interface * { return new upd78218a_disassembler; } },
	{ "upd78224",        le,  0, []() -> util::disasm_interface * { return new upd78224_disassembler; } },
	{ "upd78234",        le,  0, []() -> util::disasm_interface * { return new upd78234_disassembler; } },
	{ "upd78244",        le,  0, []() -> util::disasm_interface * { return new upd78244_disassembler; } },
	{ "upd780024a",      le,  0, []() -> util::disasm_interface * { return new upd780024a_disassembler; } },
	{ "upd78312",        le,  0, []() -> util::disasm_interface * { return new upd78312_disassembler; } },
	{ "upd78322",        le,  0, []() -> util::disasm_interface * { return new upd78322_disassembler; } },
	{ "upd78328",        le,  0, []() -> util::disasm_interface * { return new upd78328_disassembler; } },
	{ "upd78334",        le,  0, []() -> util::disasm_interface * { return new upd78334_disassembler; } },
	{ "upd78352",        le,  0, []() -> util::disasm_interface * { return new upd78352_disassembler; } },
	{ "upd78356",        le,  0, []() -> util::disasm_interface * { return new upd78356_disassembler; } },
	{ "upd78366a",       le,  0, []() -> util::disasm_interface * { return new upd78366a_disassembler; } },
	{ "upd78372",        le,  0, []() -> util::disasm_interface * { return new upd78372_disassembler; } },
	{ "upd784026",       le,  0, []() -> util::disasm_interface * { return new upd784026_disassembler; } },
	{ "upd784038",       le,  0, []() -> util::disasm_interface * { return new upd784038_disassembler; } },
	{ "upd784046",       le,  0, []() -> util::disasm_interface * { return new upd784046_disassembler; } },
	{ "upd784054",       le,  0, []() -> util::disasm_interface * { return new upd784054_disassembler; } },
	{ "upd784216",       le,  0, []() -> util::disasm_interface * { return new upd784216_disassembler; } },
	{ "upd784218",       le,  0, []() -> util::disasm_interface * { return new upd784218_disassembler; } },
	{ "upd784225",       le,  0, []() -> util::disasm_interface * { return new upd784225_disassembler; } },
	{ "upd780065",       le,  0, []() -> util::disasm_interface * { return new upd780065_disassembler; } },
	{ "upd780988",       le,  0, []() -> util::disasm_interface * { return new upd78083_disassembler; } },
	{ "upd78k0kx1",      le,  0, []() -> util::disasm_interface * { return new upd78k0kx1_disassembler; } },
	{ "upd78k0kx2",      le,  0, []() -> util::disasm_interface * { return new upd78k0kx2_disassembler; } },
	{ "upi41",           le,  0, []() -> util::disasm_interface * { return new mcs48_disassembler(true, false); } },
	{ "v60",             le,  0, []() -> util::disasm_interface * { return new v60_disassembler; } },
	{ "v620",            be, -1, []() -> util::disasm_interface * { return new v620_disassembler; } },
	{ "v620f",           be, -1, []() -> util::disasm_interface * { return new v620f_disassembler; } },
	{ "v75",             be, -1, []() -> util::disasm_interface * { return new v75_disassembler; } },
	{ "v810",            le,  0, []() -> util::disasm_interface * { return new v810_disassembler; } },
	{ "v850",            le,  0, []() -> util::disasm_interface * { return new v850_disassembler; } },
	{ "v850es",          le,  0, []() -> util::disasm_interface * { return new v850es_disassembler; } },
	{ "v850e2",          le,  0, []() -> util::disasm_interface * { return new v850e2_disassembler; } },
	{ "vax",             le,  0, []() -> util::disasm_interface * { return new vax_disassembler; } },
	{ "vt50",            le,  0, []() -> util::disasm_interface * { return new vt50_disassembler; } },
	{ "vt52",            le,  0, []() -> util::disasm_interface * { return new vt52_disassembler; } },
	{ "vt61",            le, -1, []() -> util::disasm_interface * { return new vt61_disassembler; } },
	{ "we32100",         be,  0, []() -> util::disasm_interface * { return new we32100_disassembler; } },
	{ "x86_16",          le,  0, []() -> util::disasm_interface * { i386_unidasm.mode = 16; return new i386_disassembler(&i386_unidasm); } },
	{ "x86_32",          le,  0, []() -> util::disasm_interface * { i386_unidasm.mode = 32; return new i386_disassembler(&i386_unidasm); } },
	{ "x86_64",          le,  0, []() -> util::disasm_interface * { i386_unidasm.mode = 64; return new i386_disassembler(&i386_unidasm); } },
	{ "xa",              le,  0, []() -> util::disasm_interface * { return new xa_dasm; } },
	{ "xavix",           le,  0, []() -> util::disasm_interface * { return new xavix_disassembler; } },
	{ "xavix2000",       le,  0, []() -> util::disasm_interface * { return new xavix2000_disassembler; } },
	{ "xavix2",          le,  0, []() -> util::disasm_interface * { return new xavix2_disassembler; } },
	{ "xerox530",        be, -1, []() -> util::disasm_interface * { return new xerox530_disassembler; } },
	{ "xtensa",          le,  0, []() -> util::disasm_interface * { return new xtensa_disassembler; } },
	{ "z180",            le,  0, []() -> util::disasm_interface * { return new z180_disassembler; } },
	{ "z8",              be,  0, []() -> util::disasm_interface * { return new z8_disassembler; } },
	{ "z80",             le,  0, []() -> util::disasm_interface * { return new z80_disassembler; } },
	{ "z8000",           be,  0, []() -> util::disasm_interface * { return new z8000_disassembler(&z8000_unidasm); } },
};

class unidasm_data_buffer : public util::disasm_interface::data_buffer
{
public:
	std::vector<u8> data;
	offs_t base_pc = 0;
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
			switch(entry->endian) {
			case le:
				lr16 = [this](offs_t pc) -> u16 {
					const u16 *src = get_ptr<u16>(pc);
					return little_endianize_int16(src[0]);
				};
				lr32 = [this, page_mask](offs_t pc) -> u32 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= little_endianize_int16(get<u16>(disasm->pc_linear_to_real(lpc))) << (j*16);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				lr64 = [this, page_mask](offs_t pc) -> u64 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u64 r = 0;
					for(int j=0; j != 4; j++) {
						r |= u64(little_endianize_int16(get<u16>(disasm->pc_linear_to_real(lpc)))) << (j*16);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				break;

			case be:
				lr16 = [this](offs_t pc) -> u16 {
					const u16 *src = get_ptr<u16>(pc);
					return big_endianize_int16(src[0]);
				};
				lr32 = [this, page_mask](offs_t pc) -> u32 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= big_endianize_int16(get<u16>(disasm->pc_linear_to_real(lpc))) << ((1-j)*16);
						lpc = (lpc & ~page_mask) | ((lpc + 1) & page_mask);
					}
					return r;
				};
				lr64 = [this, page_mask](offs_t pc) -> u64 {
					offs_t lpc = disasm->pc_real_to_linear(pc);
					u64 r = 0;
					for(int j=0; j != 4; j++) {
						r |= u64(big_endianize_int16(get<u16>(disasm->pc_linear_to_real(lpc)))) << ((3-j)*16);
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
			switch(entry->endian) {
			case le:
				lr16 = [this](offs_t pc) -> u16 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					little_endianize_int16(p[0])
					: 0x0000;
				};
				lr32 = [this](offs_t pc) -> u32 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					little_endianize_int16(p[0]) |
					(little_endianize_int16(p[1]) << 16)
					: 0x00000000;
				};
				lr64 = [this](offs_t pc) -> u64 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					little_endianize_int16(p[0]) |
					(little_endianize_int16(p[1]) << 16) |
					(u64(little_endianize_int16(p[2])) << 32) |
					(u64(little_endianize_int16(p[3])) << 48)
					: 0x0000000000000000;
				};
				break;
			case be:
				lr16 = [this](offs_t pc) -> u16 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					big_endianize_int16(p[0])
					: 0x0000;
				};
				lr32 = [this](offs_t pc) -> u32 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					(big_endianize_int16(p[0]) << 16)|
					big_endianize_int16(p[1])
					: 0x00000000;
				};
				lr64 = [this](offs_t pc) -> u64 {
					const u16 *p = get_ptr<u16>(pc);
					return p ?
					(u64(big_endianize_int16(p[0])) << 48) |
					(u64(big_endianize_int16(p[1])) << 32) |
					(big_endianize_int16(p[2]) << 16) |
					big_endianize_int16(p[3])
					: 0x0000000000000000;
				};
				break;
			}
			break;

		case -2:
			lr8 = [](offs_t pc) -> u8 { abort(); };
			lr16 = [](offs_t pc) -> u16 { abort(); };
			switch(entry->endian) {
			case le:
				lr32 = [this](offs_t pc) -> u32 {
					const u32 *p = get_ptr<u32>(pc);
					return p ?
					little_endianize_int32(p[0])
					: 0x00000000;
				};
				lr64 = [this](offs_t pc) -> u64 {
					const u32 *p = get_ptr<u32>(pc);
					return p ?
					little_endianize_int32(p[0]) |
					(u64(little_endianize_int32(p[1])) << 32)
					: 0x0000000000000000;
				};
				break;
			case be:
				lr32 = [this](offs_t pc) -> u32 {
					const u32 *p = get_ptr<u32>(pc);
					return p ?
					big_endianize_int32(p[0])
					: 0x00000000;
				};
				lr64 = [this](offs_t pc) -> u64 {
					const u32 *p = get_ptr<u32>(pc);
					return p ?
					(u64(big_endianize_int32(p[0])) << 32) |
					big_endianize_int32(p[1])
					: 0x0000000000000000;
				};
				break;
			}
			break;

		case -3:
			lr8 = [](offs_t pc) -> u8 { abort(); };
			lr16 = [](offs_t pc) -> u16 { abort(); };
			lr32 = [](offs_t pc) -> u32 { abort(); };
			switch(entry->endian) {
			case le:
				lr64 = [this](offs_t pc) -> u64 {
					const u64 *p = get_ptr<u64>(pc);
					return p ?
					little_endianize_int64(p[0])
					: 0x0000000000000000;
				};
				break;
			case be:
				lr64 = [this](offs_t pc) -> u64 {
					const u64 *p = get_ptr<u64>(pc);
					return p ?
					big_endianize_int64(p[0])
					: 0x0000000000000000;
				};
				break;
			};
			break;

		case 3:
			lr8 = [](offs_t pc) -> u8 { abort(); };
			switch(entry->endian) {
			case le:
				lr16 = [this](offs_t pc) -> u16 {
					if(pc < base_pc)
						return 0x0000;
					offs_t delta = (pc - base_pc) >> 3;
					if(delta >= size)
						return 0x0000;
					return little_endianize_int16(reinterpret_cast<const u16 *>(&data[delta])[0]);
				};
				lr32 = [this](offs_t pc) -> u32 {
					if(pc < base_pc)
						return 0x00000000;
					offs_t delta = (pc - base_pc) >> 3;
					if(delta >= size + 2)
						return 0x00000000;
					auto p = reinterpret_cast<const u16 *>(&data[delta]);
					return little_endianize_int16(p[0]) | (u32(little_endianize_int16(p[1])) << 16);
				};
				break;
			case be:
				lr16 = [this](offs_t pc) -> u16 {
					if(pc < base_pc)
						return 0x0000;
					offs_t delta = (pc - base_pc) >> 3;
					if(delta >= size)
						return 0x0000;
					return big_endianize_int16(reinterpret_cast<const u16 *>(&data[delta])[0]);
				};
				lr32 = [this](offs_t pc) -> u32 {
					if(pc < base_pc)
						return 0x00000000;
					offs_t delta = (pc - base_pc) >> 3;
					if(delta >= size + 2)
						return 0x00000000;
					auto p = reinterpret_cast<const u16 *>(&data[delta]);
					return (u32(big_endianize_int16(p[0])) << 16) | big_endianize_int16(p[1]);
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

static int parse_number(const char *curarg, const char *default_format, u32 *value)
{
	int result;
	if(curarg[0] == '0') {
		if(tolower((uint8_t)curarg[1]) == 'x')
			result = sscanf(&curarg[2], "%x", value);
		else if(tolower((uint8_t)curarg[1]) == 'o')
			result = sscanf(&curarg[2], "%o", value);
		else
			result = sscanf(&curarg[1], "%o", value);
	}
	else if(curarg[0] == '$')
		result = sscanf(&curarg[1], "%x", value);
	else
		result = sscanf(&curarg[0], default_format, value);
	return result;
}

static int parse_options(int argc, char *argv[], options *opts)
{
	bool pending_base = false;
	bool pending_arch = false;
	bool pending_skip = false;
	bool pending_count = false;

	memset(opts, 0, sizeof(*opts));

	// loop through arguments
	for(unsigned arg = 1; arg < argc; arg++) {
		char *curarg = argv[arg];

		// is it a switch?
		if(curarg[0] == '-' && curarg[1] != '\0') {
			if(pending_base || pending_arch || pending_skip || pending_count)
				goto usage;

			if(tolower((uint8_t)curarg[1]) == 'a')
				pending_arch = true;
			else if(tolower((uint8_t)curarg[1]) == 'b')
				pending_base = true;
			else if(tolower((uint8_t)curarg[1]) == 'f')
				opts->flipped = true;
			else if(tolower((uint8_t)curarg[1]) == 'l')
				opts->lower = true;
			else if(tolower((uint8_t)curarg[1]) == 's')
				pending_skip = true;
			else if(tolower((uint8_t)curarg[1]) == 'c')
				pending_count = true;
			else if(tolower((uint8_t)curarg[1]) == 'n')
				opts->norawbytes = true;
			else if(tolower((uint8_t)curarg[1]) == 'u')
				opts->upper = true;
			else if(tolower((uint8_t)curarg[1]) == 'x')
				opts->xchbytes = true;
			else if(tolower((uint8_t)curarg[1]) == 'o')
				opts->octal = true;
			else
				goto usage;

		} else if(pending_base) {
		// base PC
			if(parse_number(curarg, "%x", &opts->basepc) != 1)
				goto usage;
			pending_base = false;

		} else if(pending_arch) {
			// architecture
			auto const arch = std::find_if(
					std::begin(dasm_table),
					std::end(dasm_table),
					[&curarg] (dasm_table_entry const &e) { return !core_stricmp(curarg, e.name); });
			if (std::end(dasm_table) == arch)
				goto usage;
			opts->dasm = &*arch;
			pending_arch = false;

		} else if(pending_skip) {
			// skip bytes
			if(parse_number(curarg, "%d", &opts->skip) != 1)
				goto usage;
			pending_skip = false;

		} else if(pending_count) {
			// size
			if(parse_number(curarg, "%d", &opts->count) != 1)
				goto usage;
			pending_count = false;

		} else if(opts->filename == nullptr) {
			// filename
			opts->filename = curarg;

		} else {
			// fail
			goto usage;
		}
	}

	// if we have a dangling option, error
	if(pending_base || pending_arch || pending_skip || pending_count)
		goto usage;

	// if no file or no architecture, fail
	if(opts->filename == nullptr || opts->dasm == nullptr)
		goto usage;

	return 0;

usage:
	printf("Usage: %s <filename> -arch <architecture> [-basepc <pc>] \n", argv[0]);
	printf("   [-norawbytes] [-xchbytes] [-flipped] [-upper] [-lower]\n");
	printf("   [-skip <n>] [-count <n>] [-octal]\n");
	printf("\n");
	printf("Supported architectures:");
	const int colwidth = 1 + std::strlen(std::max_element(std::begin(dasm_table), std::end(dasm_table), [](const dasm_table_entry &a, const dasm_table_entry &b) { return std::strlen(a.name) < std::strlen(b.name); })->name);
	const int columns = std::max(1, 80 / colwidth);
	const int numrows = (std::size(dasm_table) + columns - 1) / columns;
	for(unsigned curarch = 0; curarch < numrows * columns; curarch++) {
		const int row = curarch / columns;
		const int col = curarch % columns;
		const int index = col * numrows + row;
		if(col == 0)
			printf("\n  ");
		printf("%-*s", colwidth, (index < std::size(dasm_table)) ? dasm_table[index].name : "");
	}
	printf("\n");
	return 1;
};


int disasm_file(util::random_read &file, u64 length, options &opts)
{
	if(opts.skip >= length) {
		std::fprintf(stderr, "File '%s' is too short (only %llu bytes found)\n", opts.filename, (unsigned long long)length);
		return 1;
	}

	// Build the disasm object
	std::unique_ptr<util::disasm_interface> disasm(opts.dasm->alloc());
	u32 flags = disasm->interface_flags();

	// Compute the granularity in bytes (1-8)
	offs_t granularity = opts.dasm->pcshift < 0 ? disasm->opcode_alignment() << -opts.dasm->pcshift : disasm->opcode_alignment() >> opts.dasm->pcshift;

	// Build the base buffer and fill it (with a margin)
	unidasm_data_buffer base_buffer(disasm.get(), opts.dasm);
	u32 rounded_size = (((length - opts.skip) + (granularity - 1)) & ~(granularity - 1));
	base_buffer.data.resize(rounded_size + 8, 0x00);
	base_buffer.size = length;
	base_buffer.base_pc = opts.basepc;
	auto const [filerr, actual] = read_at(file, opts.skip, &base_buffer.data[0], length - opts.skip);
	if(filerr) {
		std::fprintf(stderr, "Error reading from file '%s' (%s)\n", opts.filename, filerr.message().c_str());
		return 1;
	}
	if(opts.xchbytes) {
		for(u64 offset = 0; offset < (length - opts.skip) - 1; offset += 2)
			std::swap(base_buffer.data[offset], base_buffer.data[offset + 1]);
	}

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

	// Compute the pc wraparound
	offs_t pclength = opts.dasm->pcshift < 0 ? rounded_size >> -opts.dasm->pcshift : rounded_size << opts.dasm->pcshift;
	offs_t limit = opts.basepc + pclength;
	offs_t pc_mask = limit ? util::make_bitmask<offs_t>(32 - count_leading_zeros_32(limit - 1)) : 0xffffffff;

	// Compute the page wraparound
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
	u32 granularity_shift = 31 - count_leading_zeros_32(disasm->opcode_alignment());

	// Number of pc steps to disassemble
	u32 count = pclength;

	if((count > opts.count) && (opts.count != 0))
		count = opts.count;

	// pc to string conversion
	std::function<std::string (offs_t pc)> pc_to_string;
	int aw = 32 - count_leading_zeros_32(pc_mask);
	bool is_octal = opts.octal; // Parameter?  Per-cpu config?
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

	if(is_octal) {
		switch(granularity) {
		case 1:
			dump_raw_bytes = [step = disasm->opcode_alignment(), &next_pc, &base_buffer](offs_t pc, offs_t size) -> std::string {
				std::string result = "";
				for(offs_t i=0; i != size; i++) {
					if(i)
						result += ' ';
					result += util::string_format("%03o", base_buffer.r8(pc));
					pc = next_pc(pc, step);
				}
				return result;
			};
			max_len = (max_len * 4) - 1;
			break;

		case 2:
			dump_raw_bytes = [step = disasm->opcode_alignment(), &next_pc, &base_buffer](offs_t pc, offs_t size) -> std::string {
				std::string result = "";
				for(offs_t i=0; i != size; i++) {
					if(i)
						result += ' ';
					result += util::string_format("%06o", base_buffer.r16(pc));
					pc = next_pc(pc, step);
				}
				return result;
			};
			max_len = (max_len * 7) - 1;
			break;

		case 4:
			dump_raw_bytes = [step = disasm->opcode_alignment(), &next_pc, &base_buffer](offs_t pc, offs_t size) -> std::string {
				std::string result = "";
				for(offs_t i=0; i != size; i++) {
					if(i)
						result += ' ';
					result += util::string_format("%011o", base_buffer.r32(pc));
					pc = next_pc(pc, step);
				}
				return result;
			};
			max_len = (max_len * 12) - 1;
			break;

		case 8:
			dump_raw_bytes = [step = disasm->opcode_alignment(), &next_pc, &base_buffer](offs_t pc, offs_t size) -> std::string {
				std::string result = "";
				for(offs_t i=0; i != size; i++) {
					if(i)
						result += ' ';
					result += util::string_format("%022o", base_buffer.r64(pc));
					pc = next_pc(pc, step);
				}
				return result;
			};
			max_len = (max_len * 23) - 1;
			break;
		}
	} else {
		switch(granularity) {
		case 1:
			dump_raw_bytes = [step = disasm->opcode_alignment(), &next_pc, &base_buffer](offs_t pc, offs_t size) -> std::string {
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
			dump_raw_bytes = [step = disasm->opcode_alignment(), &next_pc, &base_buffer](offs_t pc, offs_t size) -> std::string {
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
			dump_raw_bytes = [step = disasm->opcode_alignment(), &next_pc, &base_buffer](offs_t pc, offs_t size) -> std::string {
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
			dump_raw_bytes = [step = disasm->opcode_alignment(), &next_pc, &base_buffer](offs_t pc, offs_t size) -> std::string {
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

	return 0;
}


int main(int argc, char *argv[])
{
	// Parse options first
	options opts;
	if(parse_options(argc, argv, &opts))
		return 1;

	// Load the file
	void *data = nullptr;
	util::random_read::ptr file;
	u64 length = 0;
	if(std::strcmp(opts.filename, "-") != 0) {
		osd_file::ptr f;
		std::error_condition filerr = osd_file::open(std::string(opts.filename), OPEN_FLAG_READ, f, length);
		if(filerr) {
			std::fprintf(stderr, "Error opening file '%s' (%s)\n", opts.filename, filerr.message().c_str());
			return 1;
		}
		file = util::osd_file_read(std::move(f));
		if(!file) {
			std::fprintf(stderr, "Error when opening file '%s'\n", opts.filename);
			return 1;
		}
	}
	else
	{
#ifdef _WIN32
		if (_setmode(_fileno(stdin), _O_BINARY) == -1) {
#else
		if (!std::freopen(nullptr, "rb", stdin)) {
#endif
			std::fprintf(stderr, "Error reopening stdin in binary mode\n");
			return 1;
		}
		std::size_t allocated = 0x1000;
		data = std::malloc(allocated);
		while(!std::ferror(stdin) && !std::feof(stdin)) {
			if(length == allocated) {
				allocated <<= 1;
				data = std::realloc(data, allocated);
			}
			if(!data) {
				std::fprintf(stderr, "Error allocating buffer\n");
				return 1;
			}

			length += std::fread((u8 *)data + length, 1, allocated - length, stdin);
		}
		if(!length || (std::ferror(stdin) && !std::feof(stdin))) {
			std::fprintf(stderr, "Error reading from stdin\n");
			std::free(data);
			return 1;
		}
		file = util::ram_read(data, length);
		if(!file) {
			std::fprintf(stderr, "Error opening stdin as file\n");
			std::free(data);
			return 1;
		}
	}

	int result = disasm_file(*file, length, opts);

	file.reset();
	std::free(data);

	return result;
}
