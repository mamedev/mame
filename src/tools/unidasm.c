/***************************************************************************

    mamedasm.c

    Generic MAME disassembler.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "cpuintrf.h"

CPU_DISASSEMBLE( adsp21xx );
CPU_DISASSEMBLE( alpha8201 );
CPU_DISASSEMBLE( arm );
CPU_DISASSEMBLE( arm7arm );
CPU_DISASSEMBLE( arm7thumb );
CPU_DISASSEMBLE( asap );
CPU_DISASSEMBLE( avr8 );
CPU_DISASSEMBLE( ccpu );
CPU_DISASSEMBLE( cdp1802 );
CPU_DISASSEMBLE( cop410 );
CPU_DISASSEMBLE( cop420 );
CPU_DISASSEMBLE( cop444 );
CPU_DISASSEMBLE( cp1610 );
CPU_DISASSEMBLE( cquestsnd );
CPU_DISASSEMBLE( cquestrot );
CPU_DISASSEMBLE( cquestlin );
CPU_DISASSEMBLE( dsp32c );
CPU_DISASSEMBLE( dsp56k );
CPU_DISASSEMBLE( hyperstone_generic );
CPU_DISASSEMBLE( esrip );
CPU_DISASSEMBLE( f8 );
CPU_DISASSEMBLE( g65816_generic );
CPU_DISASSEMBLE( h6280 );
CPU_DISASSEMBLE( h8 );
CPU_DISASSEMBLE( hd6309 );
CPU_DISASSEMBLE( i4004 );
CPU_DISASSEMBLE( i8085 );
CPU_DISASSEMBLE( x86_16 );
CPU_DISASSEMBLE( x86_32 );
CPU_DISASSEMBLE( x86_64 );
CPU_DISASSEMBLE( i860 );
CPU_DISASSEMBLE( i960 );
CPU_DISASSEMBLE( jaguargpu );
CPU_DISASSEMBLE( jaguardsp );
CPU_DISASSEMBLE( konami );
CPU_DISASSEMBLE( lh5801 );
CPU_DISASSEMBLE( lr35902 );
CPU_DISASSEMBLE( m37710_generic );
CPU_DISASSEMBLE( m6502 );
CPU_DISASSEMBLE( m65sc02 );
CPU_DISASSEMBLE( m65c02 );
CPU_DISASSEMBLE( m65ce02 );
CPU_DISASSEMBLE( m6510 );
CPU_DISASSEMBLE( deco16 );
CPU_DISASSEMBLE( m4510 );
CPU_DISASSEMBLE( m6800 );
CPU_DISASSEMBLE( m6801 );
CPU_DISASSEMBLE( m6802 );
CPU_DISASSEMBLE( m6803 );
CPU_DISASSEMBLE( hd63701 );
CPU_DISASSEMBLE( nsc8105 );
CPU_DISASSEMBLE( m68000 );
CPU_DISASSEMBLE( m68008 );
CPU_DISASSEMBLE( m68010 );
CPU_DISASSEMBLE( m68020 );
CPU_DISASSEMBLE( m68030 );
CPU_DISASSEMBLE( m68040 );
CPU_DISASSEMBLE( m6805 );
CPU_DISASSEMBLE( m6809 );
CPU_DISASSEMBLE( mb86233 );
CPU_DISASSEMBLE( mb88 );
CPU_DISASSEMBLE( mcs48 );
CPU_DISASSEMBLE( upi41 );
CPU_DISASSEMBLE( i8051 );
CPU_DISASSEMBLE( i8052 );
CPU_DISASSEMBLE( i80c51 );
CPU_DISASSEMBLE( i80c52 );
CPU_DISASSEMBLE( ds5002fp );
CPU_DISASSEMBLE( minx );
CPU_DISASSEMBLE( mips3be );
CPU_DISASSEMBLE( mips3le );
CPU_DISASSEMBLE( psxcpu_generic );
CPU_DISASSEMBLE( r3000be );
CPU_DISASSEMBLE( r3000le );
CPU_DISASSEMBLE( nec_generic );
CPU_DISASSEMBLE( pdp1 );
CPU_DISASSEMBLE( tx0_64kw );
CPU_DISASSEMBLE( tx0_8kw );
CPU_DISASSEMBLE( pic16c5x );
CPU_DISASSEMBLE( powerpc );
CPU_DISASSEMBLE( rsp );
CPU_DISASSEMBLE( s2650 );
CPU_DISASSEMBLE( saturn );
CPU_DISASSEMBLE( sc61860 );
CPU_DISASSEMBLE( se3208 );
CPU_DISASSEMBLE( sh2 );
CPU_DISASSEMBLE( sh4 );
CPU_DISASSEMBLE( sharc );
CPU_DISASSEMBLE( sm8500 );
CPU_DISASSEMBLE( spc700 );
CPU_DISASSEMBLE( ssem );
CPU_DISASSEMBLE( ssp1601 );
CPU_DISASSEMBLE( t11 );
CPU_DISASSEMBLE( t90 );
CPU_DISASSEMBLE( tlcs900 );
CPU_DISASSEMBLE( tms0980 );
CPU_DISASSEMBLE( tms1000 );
CPU_DISASSEMBLE( tms1100 );
CPU_DISASSEMBLE( tms32010 );
CPU_DISASSEMBLE( tms32025 );
CPU_DISASSEMBLE( tms32031 );
CPU_DISASSEMBLE( tms32051 );
CPU_DISASSEMBLE( tms34010 );
CPU_DISASSEMBLE( tms34020 );
CPU_DISASSEMBLE( tms57002 );
CPU_DISASSEMBLE( tms7000 );
CPU_DISASSEMBLE( upd7810 );
CPU_DISASSEMBLE( upd7807 );
CPU_DISASSEMBLE( upd7801 );
CPU_DISASSEMBLE( upd78c05 );
CPU_DISASSEMBLE( v60 );
CPU_DISASSEMBLE( v70 );
CPU_DISASSEMBLE( v810 );
CPU_DISASSEMBLE( z180 );
CPU_DISASSEMBLE( z8000 );
CPU_DISASSEMBLE( z80 );


enum _display_type
{
	_8bit,
	_16be,
	_16le,
	_24be,
	_24le,
	_32be,
	_32le,
	_64be,
	_64le
};
typedef enum _display_type display_type;


typedef struct _dasm_table_entry dasm_table_entry;
struct _dasm_table_entry
{
	const char *			name;
	display_type			display;
	INT8					pcshift;
	cpu_disassemble_func	func;
};


static const dasm_table_entry dasm_table[] =
{
	{ "adsp21xx",	_24le, -2, CPU_DISASSEMBLE_NAME(adsp21xx) },
	{ "alpha8201",	_8bit,  0, CPU_DISASSEMBLE_NAME(alpha8201) },
	{ "arm",		_32le,  0, CPU_DISASSEMBLE_NAME(arm) },
	{ "arm7",		_32le,  0, CPU_DISASSEMBLE_NAME(arm7arm) },
	{ "arm7thumb",	_16le,  0, CPU_DISASSEMBLE_NAME(arm7thumb) },
	{ "asap",		_32le,  0, CPU_DISASSEMBLE_NAME(asap) },
	{ "avr8",		_16le,  0, CPU_DISASSEMBLE_NAME(avr8) },
	{ "ccpu",		_8bit,  0, CPU_DISASSEMBLE_NAME(ccpu) },
	{ "cdp1802",	_8bit,  0, CPU_DISASSEMBLE_NAME(cdp1802) },
	{ "cop410",		_8bit,  0, CPU_DISASSEMBLE_NAME(cop410) },
	{ "cop420",		_8bit,  0, CPU_DISASSEMBLE_NAME(cop420) },
	{ "cop444",		_8bit,  0, CPU_DISASSEMBLE_NAME(cop444) },
	{ "cp1610",		_16be, -1, CPU_DISASSEMBLE_NAME(cp1610) },
	{ "cquestsnd",	_64be, -3, CPU_DISASSEMBLE_NAME(cquestsnd) },
	{ "cquestrot",	_64be, -3, CPU_DISASSEMBLE_NAME(cquestrot) },
	{ "cquestlin",	_64be, -3, CPU_DISASSEMBLE_NAME(cquestlin) },
	{ "dsp32c",		_32le,  0, CPU_DISASSEMBLE_NAME(dsp32c) },
	{ "dsp56k",		_16le, -1, CPU_DISASSEMBLE_NAME(dsp56k) },
	{ "hyperstone",	_16be,  0, CPU_DISASSEMBLE_NAME(hyperstone_generic) },
	{ "esrip",		_64be,  0, CPU_DISASSEMBLE_NAME(esrip) },
	{ "f8",			_8bit,  0, CPU_DISASSEMBLE_NAME(f8) },
	{ "g65816",		_8bit,  0, CPU_DISASSEMBLE_NAME(g65816_generic) },
	{ "h6280",		_8bit,  0, CPU_DISASSEMBLE_NAME(h6280) },
	{ "h8",			_16be,  0, CPU_DISASSEMBLE_NAME(h8) },
	{ "hd6309",		_8bit,  0, CPU_DISASSEMBLE_NAME(hd6309) },
	{ "i386",		_8bit,  0, CPU_DISASSEMBLE_NAME(x86_32) },
	{ "i4004",		_8bit,  0, CPU_DISASSEMBLE_NAME(i4004) },
	{ "i8085",		_8bit,  0, CPU_DISASSEMBLE_NAME(i8085) },
	{ "i80286",		_8bit,  0, CPU_DISASSEMBLE_NAME(x86_16) },
	{ "i8086",		_8bit,  0, CPU_DISASSEMBLE_NAME(x86_16) },
	{ "i960",		_32le,  0, CPU_DISASSEMBLE_NAME(i960) },
	{ "jaguargpu",	_16be,  0, CPU_DISASSEMBLE_NAME(jaguargpu) },
	{ "jaguardsp",	_16be,  0, CPU_DISASSEMBLE_NAME(jaguardsp) },
	{ "x86_16",		_8bit,  0, CPU_DISASSEMBLE_NAME(x86_16) },
	{ "x86_32",		_8bit,  0, CPU_DISASSEMBLE_NAME(x86_32) },
	{ "x86_64",		_8bit,  0, CPU_DISASSEMBLE_NAME(x86_64) },
	{ "konami",     _8bit,  0, CPU_DISASSEMBLE_NAME(konami) },
	{ "lh5801",     _8bit,  0, CPU_DISASSEMBLE_NAME(lh5801) },
	{ "lr35902",    _8bit,  0, CPU_DISASSEMBLE_NAME(lr35902) },
	{ "m37710",     _8bit,  0, CPU_DISASSEMBLE_NAME(m37710_generic) },
	{ "m6502",      _8bit,  0, CPU_DISASSEMBLE_NAME(m6502) },
	{ "m65sc02",    _8bit,  0, CPU_DISASSEMBLE_NAME(m65sc02) },
	{ "m65c02",     _8bit,  0, CPU_DISASSEMBLE_NAME(m65c02) },
	{ "m65ce02",    _8bit,  0, CPU_DISASSEMBLE_NAME(m65ce02) },
	{ "m6510",      _8bit,  0, CPU_DISASSEMBLE_NAME(m6510) },
	{ "deco16",     _8bit,  0, CPU_DISASSEMBLE_NAME(deco16) },
	{ "m4510",      _8bit,  0, CPU_DISASSEMBLE_NAME(m4510) },
	{ "m6800",      _8bit,  0, CPU_DISASSEMBLE_NAME(m6800) },
	{ "m6801",      _8bit,  0, CPU_DISASSEMBLE_NAME(m6801) },
	{ "m6802",      _8bit,  0, CPU_DISASSEMBLE_NAME(m6802) },
	{ "m6803",      _8bit,  0, CPU_DISASSEMBLE_NAME(m6803) },
	{ "hd63701",    _8bit,  0, CPU_DISASSEMBLE_NAME(hd63701) },
	{ "nsc8105",    _8bit,  0, CPU_DISASSEMBLE_NAME(nsc8105) },
	{ "m68000",     _16be,  0, CPU_DISASSEMBLE_NAME(m68000) },
	{ "m68008",     _16be,  0, CPU_DISASSEMBLE_NAME(m68008) },
	{ "m68010",     _16be,  0, CPU_DISASSEMBLE_NAME(m68010) },
	{ "m68020",     _16be,  0, CPU_DISASSEMBLE_NAME(m68020) },
	{ "m68030",     _16be,  0, CPU_DISASSEMBLE_NAME(m68030) },
	{ "m68040",     _16be,  0, CPU_DISASSEMBLE_NAME(m68040) },
	{ "m6805",      _8bit,  0, CPU_DISASSEMBLE_NAME(m6805) },
	{ "m6809",      _8bit,  0, CPU_DISASSEMBLE_NAME(m6809) },
	{ "mb86233",    _32le, -2, CPU_DISASSEMBLE_NAME(mb86233) },
	{ "mb88xx",     _8bit,  0, CPU_DISASSEMBLE_NAME(mb88) },
	{ "mcs48",      _8bit,  0, CPU_DISASSEMBLE_NAME(mcs48) },
	{ "upi41",      _8bit,  0, CPU_DISASSEMBLE_NAME(upi41) },
	{ "i8051",      _8bit,  0, CPU_DISASSEMBLE_NAME(i8051) },
	{ "i8052",      _8bit,  0, CPU_DISASSEMBLE_NAME(i8052) },
	{ "i80c51",     _8bit,  0, CPU_DISASSEMBLE_NAME(i80c51) },
	{ "i80c52",     _8bit,  0, CPU_DISASSEMBLE_NAME(i80c52) },
	{ "ds5002fp",   _8bit,  0, CPU_DISASSEMBLE_NAME(ds5002fp) },
	{ "minx",       _8bit,  0, CPU_DISASSEMBLE_NAME(minx) },
	{ "mips3be",    _32be,  0, CPU_DISASSEMBLE_NAME(mips3be) },
	{ "mips3le",    _32le,  0, CPU_DISASSEMBLE_NAME(mips3le) },
	{ "psxcpu",     _32le,  0, CPU_DISASSEMBLE_NAME(psxcpu_generic) },
	{ "r3000be",    _32be,  0, CPU_DISASSEMBLE_NAME(r3000be) },
	{ "r3000le",    _32le,  0, CPU_DISASSEMBLE_NAME(r3000le) },
	{ "nec",        _8bit,  0, CPU_DISASSEMBLE_NAME(nec_generic) },
	{ "pdp1",       _32be,  0, CPU_DISASSEMBLE_NAME(pdp1) },
	{ "tx0_64kw",   _32be, -2, CPU_DISASSEMBLE_NAME(tx0_64kw) },
	{ "tx0_8kw",    _32be, -2, CPU_DISASSEMBLE_NAME(tx0_8kw) },
	{ "pic16c5x",   _16le, -1, CPU_DISASSEMBLE_NAME(pic16c5x) },
	{ "powerpc",    _32be,  0, CPU_DISASSEMBLE_NAME(powerpc) },
	{ "rsp",        _32le,  0, CPU_DISASSEMBLE_NAME(rsp) },
	{ "s2650",      _8bit,  0, CPU_DISASSEMBLE_NAME(s2650) },
	{ "saturn",     _8bit,  0, CPU_DISASSEMBLE_NAME(saturn) },
	{ "sc61860",    _8bit,  0, CPU_DISASSEMBLE_NAME(sc61860) },
	{ "se3208",     _16le,  0, CPU_DISASSEMBLE_NAME(se3208) },
	{ "sh2",        _16be,  0, CPU_DISASSEMBLE_NAME(sh2) },
	{ "sh4",        _16le,  0, CPU_DISASSEMBLE_NAME(sh4) },
	{ "sharc",      _64le, -3, CPU_DISASSEMBLE_NAME(sharc) },
	{ "sm8500",     _8bit,  0, CPU_DISASSEMBLE_NAME(sm8500) },
	{ "spc700",     _8bit,  0, CPU_DISASSEMBLE_NAME(spc700) },
	{ "ssem",       _32le,  0, CPU_DISASSEMBLE_NAME(ssem) },
	{ "ssp1601",    _16be, -1, CPU_DISASSEMBLE_NAME(ssp1601) },
	{ "t11",        _16le,  0, CPU_DISASSEMBLE_NAME(t11) },
//	{ "t90",        _8bit,  0, CPU_DISASSEMBLE_NAME(t90) },
	{ "tlcs900",    _8bit,  0, CPU_DISASSEMBLE_NAME(tlcs900) },
	{ "tms0980",    _16be,  0, CPU_DISASSEMBLE_NAME(tms0980) },
	{ "tms1000",    _8bit,  0, CPU_DISASSEMBLE_NAME(tms1000) },
	{ "tms1100",    _8bit,  0, CPU_DISASSEMBLE_NAME(tms1100) },
	{ "tms32010",   _16be, -1, CPU_DISASSEMBLE_NAME(tms32010) },
	{ "tms32025",   _16be, -1, CPU_DISASSEMBLE_NAME(tms32025) },
	{ "tms32031",   _32le, -2, CPU_DISASSEMBLE_NAME(tms32031) },
	{ "tms32051",   _16le, -1, CPU_DISASSEMBLE_NAME(tms32051) },
	{ "tms34010",   _8bit,  3, CPU_DISASSEMBLE_NAME(tms34010) },
	{ "tms34020",   _8bit,  3, CPU_DISASSEMBLE_NAME(tms34020) },
	{ "tms57002",   _32le, -2, CPU_DISASSEMBLE_NAME(tms57002) },
	{ "tms7000",    _8bit,  0, CPU_DISASSEMBLE_NAME(tms7000) },
	{ "upd7810",    _8bit,  0, CPU_DISASSEMBLE_NAME(upd7810) },
	{ "upd7807",    _8bit,  0, CPU_DISASSEMBLE_NAME(upd7807) },
	{ "upd7801",    _8bit,  0, CPU_DISASSEMBLE_NAME(upd7801) },
	{ "upd78c05",   _8bit,  0, CPU_DISASSEMBLE_NAME(upd78c05) },
	{ "v60",        _8bit,  0, CPU_DISASSEMBLE_NAME(v60) },
	{ "v70",        _8bit,  0, CPU_DISASSEMBLE_NAME(v70) },
	{ "v810",       _16le,  0, CPU_DISASSEMBLE_NAME(v810) },
	{ "z180",       _8bit,  0, CPU_DISASSEMBLE_NAME(z180) },
//	{ "z8000",      _16be,  0, CPU_DISASSEMBLE_NAME(z8000) },
	{ "z80",		_8bit,  0, CPU_DISASSEMBLE_NAME(z80) },
};


void CLIB_DECL fatalerror(const char *text, ...)
{
	va_list arg;

	/* dump to the buffer; assume no one writes >2k lines this way */
	va_start(arg, text);
	vfprintf(stderr, text, arg);
	va_end(arg);
	
	exit(1);
}


void CLIB_DECL logerror(const char *format, ...)
{
	/* silent logerrors are allowed in disassemblers */
}


void CLIB_DECL mame_printf_debug(const char *format, ...)
{
	/* silent mame_printf_debugs are allowed in disassemblers */
}


int main(int argc, char *argv[])
{
	char buffer[1024];
	UINT8 oprom[10] = { 0x12, 0x34, 0x56, 0x78 };
	offs_t basepc = 0;
	int index;

//const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram
	for (index = 0; index < ARRAY_LENGTH(dasm_table); index++)
	{
		int result = (*dasm_table[index].func)(NULL, buffer, basepc, oprom, oprom);
		printf("%10s: (%d) %s\n", dasm_table[index].name, result & DASMFLAG_LENGTHMASK, buffer);
	}
	return 0;
}
