/***************************************************************************

    mamedasm.c

    Generic MAME disassembler.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "emu.h"
#include <ctype.h>

enum display_type
{
	_8bit,
	_8bitx,
	_16be,
	_16le,
	_24be,
	_24le,
	_32be,
	_32le,
	_40be,
	_40le,
	_48be,
	_48le,
	_56be,
	_56le,
	_64be,
	_64le
};


struct dasm_table_entry
{
	const char *			name;
	display_type			display;
	INT8					pcshift;
	cpu_disassemble_func	func;
};


struct options
{
	const char *			filename;
	offs_t					basepc;
	UINT8					norawbytes;
	UINT8					lower;
	UINT8					upper;
	UINT8					flipped;
	int						mode;
	const dasm_table_entry *dasm;
	UINT32					skip;
	UINT32					count;
};


CPU_DISASSEMBLE( adsp21xx );
CPU_DISASSEMBLE( alpha8201 );
CPU_DISASSEMBLE( arm );
CPU_DISASSEMBLE( arm7arm );
CPU_DISASSEMBLE( arm7thumb );
CPU_DISASSEMBLE( asap );
CPU_DISASSEMBLE( avr8 );
CPU_DISASSEMBLE( ccpu );
CPU_DISASSEMBLE( cop410 );
CPU_DISASSEMBLE( cop420 );
CPU_DISASSEMBLE( cop444 );
CPU_DISASSEMBLE( cosmac );
CPU_DISASSEMBLE( cp1610 );
CPU_DISASSEMBLE( cquestsnd );
CPU_DISASSEMBLE( cquestrot );
CPU_DISASSEMBLE( cquestlin );
CPU_DISASSEMBLE( dsp16a );
CPU_DISASSEMBLE( dsp32c );
CPU_DISASSEMBLE( dsp56k );
CPU_DISASSEMBLE( hyperstone_generic );
CPU_DISASSEMBLE( hd61700 );
CPU_DISASSEMBLE( esrip );
CPU_DISASSEMBLE( f8 );
CPU_DISASSEMBLE( g65816_generic );
CPU_DISASSEMBLE( h6280 );
CPU_DISASSEMBLE( h8 );
CPU_DISASSEMBLE( hd6309 );
CPU_DISASSEMBLE( i4004 );
CPU_DISASSEMBLE( i8008 );
CPU_DISASSEMBLE( i8085 );
CPU_DISASSEMBLE( x86_16 );
CPU_DISASSEMBLE( x86_32 );
CPU_DISASSEMBLE( x86_64 );
CPU_DISASSEMBLE( i960 );
CPU_DISASSEMBLE( jaguargpu );
CPU_DISASSEMBLE( jaguardsp );
CPU_DISASSEMBLE( konami );
CPU_DISASSEMBLE( lh5801 );
CPU_DISASSEMBLE( lr35902 );
CPU_DISASSEMBLE( m37710_generic );
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
CPU_DISASSEMBLE( pps4 );
CPU_DISASSEMBLE( tx0_64kw );
CPU_DISASSEMBLE( tx0_8kw );
CPU_DISASSEMBLE( pic16c5x );
CPU_DISASSEMBLE( pic16c62x );
CPU_DISASSEMBLE( powerpc );
CPU_DISASSEMBLE( rsp );
CPU_DISASSEMBLE( s2650 );
CPU_DISASSEMBLE( saturn );
CPU_DISASSEMBLE( sc61860 );
CPU_DISASSEMBLE( scmp );
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
CPU_DISASSEMBLE( tms3203x );
CPU_DISASSEMBLE( tms32051 );
CPU_DISASSEMBLE( tms34010 );
CPU_DISASSEMBLE( tms34020 );
//CPU_DISASSEMBLE( tms57002 );
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
CPU_DISASSEMBLE( z8 );


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
	{ "cop410",		_8bit,  0, CPU_DISASSEMBLE_NAME(cop410) },
	{ "cop420",		_8bit,  0, CPU_DISASSEMBLE_NAME(cop420) },
	{ "cop444",		_8bit,  0, CPU_DISASSEMBLE_NAME(cop444) },
	{ "cosmac",		_8bit,  0, CPU_DISASSEMBLE_NAME(cosmac) },
	{ "cp1610",		_16be, -1, CPU_DISASSEMBLE_NAME(cp1610) },
	{ "cquestsnd",	_64be, -3, CPU_DISASSEMBLE_NAME(cquestsnd) },
	{ "cquestrot",	_64be, -3, CPU_DISASSEMBLE_NAME(cquestrot) },
	{ "cquestlin",	_64be, -3, CPU_DISASSEMBLE_NAME(cquestlin) },
	{ "dsp16a",		_16le, -1, CPU_DISASSEMBLE_NAME(dsp16a) },
	{ "dsp32c",		_32le,  0, CPU_DISASSEMBLE_NAME(dsp32c) },
	{ "dsp56k",		_16le, -1, CPU_DISASSEMBLE_NAME(dsp56k) },
	{ "hyperstone",	_16be,  0, CPU_DISASSEMBLE_NAME(hyperstone_generic) },
	{ "hd61700",	_8bit,  0, CPU_DISASSEMBLE_NAME(hd61700) },
	{ "esrip",		_64be,  0, CPU_DISASSEMBLE_NAME(esrip) },
	{ "f8",			_8bit,  0, CPU_DISASSEMBLE_NAME(f8) },
	{ "g65816",		_8bit,  0, CPU_DISASSEMBLE_NAME(g65816_generic) },
	{ "h6280",		_8bit,  0, CPU_DISASSEMBLE_NAME(h6280) },
	{ "h8",			_16be,  0, CPU_DISASSEMBLE_NAME(h8) },
	{ "hd6309",		_8bit,  0, CPU_DISASSEMBLE_NAME(hd6309) },
	{ "i386",		_8bit,  0, CPU_DISASSEMBLE_NAME(x86_32) },
	{ "i4004",		_8bit,  0, CPU_DISASSEMBLE_NAME(i4004) },
	{ "i8008",		_8bit,  0, CPU_DISASSEMBLE_NAME(i8008) },
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
	{ "pps4",		_8bit,  0, CPU_DISASSEMBLE_NAME(pps4) },
	{ "tx0_64kw",   _32be, -2, CPU_DISASSEMBLE_NAME(tx0_64kw) },
	{ "tx0_8kw",    _32be, -2, CPU_DISASSEMBLE_NAME(tx0_8kw) },
	{ "pic16c5x",   _16le, -1, CPU_DISASSEMBLE_NAME(pic16c5x) },
	{ "pic16c62x",  _16le, -1, CPU_DISASSEMBLE_NAME(pic16c62x) },
	{ "powerpc",    _32be,  0, CPU_DISASSEMBLE_NAME(powerpc) },
	{ "rsp",        _32le,  0, CPU_DISASSEMBLE_NAME(rsp) },
	{ "s2650",      _8bit,  0, CPU_DISASSEMBLE_NAME(s2650) },
	{ "saturn",     _8bit,  0, CPU_DISASSEMBLE_NAME(saturn) },
	{ "sc61860",    _8bit,  0, CPU_DISASSEMBLE_NAME(sc61860) },
	{ "scmp",   	_8bit,  0, CPU_DISASSEMBLE_NAME(scmp) },
	{ "se3208",     _16le,  0, CPU_DISASSEMBLE_NAME(se3208) },
	{ "sh2",        _16be,  0, CPU_DISASSEMBLE_NAME(sh2) },
	{ "sh4",        _16le,  0, CPU_DISASSEMBLE_NAME(sh4) },
	{ "sharc",      _48le, -2, CPU_DISASSEMBLE_NAME(sharc) },
	{ "sm8500",     _8bit,  0, CPU_DISASSEMBLE_NAME(sm8500) },
	{ "spc700",     _8bit,  0, CPU_DISASSEMBLE_NAME(spc700) },
	{ "ssem",       _32le,  0, CPU_DISASSEMBLE_NAME(ssem) },
	{ "ssp1601",    _16be, -1, CPU_DISASSEMBLE_NAME(ssp1601) },
	{ "t11",        _16le,  0, CPU_DISASSEMBLE_NAME(t11) },
//  { "t90",        _8bit,  0, CPU_DISASSEMBLE_NAME(t90) },
	{ "tlcs900",    _8bit,  0, CPU_DISASSEMBLE_NAME(tlcs900) },
	{ "tms0980",    _16be,  0, CPU_DISASSEMBLE_NAME(tms0980) },
	{ "tms1000",    _8bit,  0, CPU_DISASSEMBLE_NAME(tms1000) },
	{ "tms1100",    _8bit,  0, CPU_DISASSEMBLE_NAME(tms1100) },
	{ "tms32010",   _16be, -1, CPU_DISASSEMBLE_NAME(tms32010) },
	{ "tms32025",   _16be, -1, CPU_DISASSEMBLE_NAME(tms32025) },
	{ "tms32031",   _32le, -2, CPU_DISASSEMBLE_NAME(tms3203x) },
	{ "tms32051",   _16le, -1, CPU_DISASSEMBLE_NAME(tms32051) },
	{ "tms34010",   _8bit,  3, CPU_DISASSEMBLE_NAME(tms34010) },
	{ "tms34020",   _8bit,  3, CPU_DISASSEMBLE_NAME(tms34020) },
	//  { "tms57002",   _32le, -2, CPU_DISASSEMBLE_NAME(tms57002) },
	{ "tms7000",    _8bit,  0, CPU_DISASSEMBLE_NAME(tms7000) },
	{ "upd7810",    _8bit,  0, CPU_DISASSEMBLE_NAME(upd7810) },
	{ "upd7807",    _8bit,  0, CPU_DISASSEMBLE_NAME(upd7807) },
	{ "upd7801",    _8bit,  0, CPU_DISASSEMBLE_NAME(upd7801) },
	{ "upd78c05",   _8bit,  0, CPU_DISASSEMBLE_NAME(upd78c05) },
	{ "v60",        _8bit,  0, CPU_DISASSEMBLE_NAME(v60) },
	{ "v70",        _8bit,  0, CPU_DISASSEMBLE_NAME(v70) },
	{ "v810",       _16le,  0, CPU_DISASSEMBLE_NAME(v810) },
	{ "z180",       _8bit,  0, CPU_DISASSEMBLE_NAME(z180) },
//  { "z8000",      _16be,  0, CPU_DISASSEMBLE_NAME(z8000) },
	{ "z80",		_8bit,  0, CPU_DISASSEMBLE_NAME(z80) },
	{ "z8",			_8bit,  0, CPU_DISASSEMBLE_NAME(z8) },
};

void CLIB_DECL logerror(const char *format, ...)
{
	/* silent logerrors are allowed in disassemblers */
}


void CLIB_DECL mame_printf_debug(const char *format, ...)
{
	/* silent mame_printf_debugs are allowed in disassemblers */
}


static int parse_options(int argc, char *argv[], options *opts)
{
	int pending_base = FALSE;
	int pending_arch = FALSE;
	int pending_mode = FALSE;
	int pending_skip = FALSE;
	int pending_count = FALSE;
	int curarch;
	int numrows;
	int arg;

	memset(opts, 0, sizeof(*opts));

	// loop through arguments
	for (arg = 1; arg < argc; arg++)
	{
		char *curarg = argv[arg];

		// is it a switch?
		if (curarg[0] == '-')
		{
			if (pending_base || pending_arch || pending_mode || pending_skip || pending_count)
				goto usage;

			if (tolower((UINT8)curarg[1]) == 'a')
				pending_arch = TRUE;
			else if (tolower((UINT8)curarg[1]) == 'b')
				pending_base = TRUE;
			else if (tolower((UINT8)curarg[1]) == 'f')
				opts->flipped = TRUE;
			else if (tolower((UINT8)curarg[1]) == 'l')
				opts->lower = TRUE;
			else if (tolower((UINT8)curarg[1]) == 'm')
				pending_mode = TRUE;
			else if (tolower((UINT8)curarg[1]) == 's')
				pending_skip = TRUE;
			else if (tolower((UINT8)curarg[1]) == 'c')
				pending_count = TRUE;
			else if (tolower((UINT8)curarg[1]) == 'n')
				opts->norawbytes = TRUE;
			else if (tolower((UINT8)curarg[1]) == 'u')
				opts->upper = TRUE;
			else
				goto usage;
		}

		// base PC
		else if (pending_base)
		{
			int result;
			if (curarg[0] == '0' && curarg[1] == 'x')
				result = sscanf(&curarg[2], "%x", &opts->basepc);
			else if (curarg[0] == '$')
				result = sscanf(&curarg[1], "%x", &opts->basepc);
			else
				result = sscanf(&curarg[0], "%x", &opts->basepc);
			if (result != 1)
				goto usage;
			pending_base = FALSE;
		}

		// mode
		else if (pending_mode)
		{
			if (sscanf(curarg, "%d", &opts->mode) != 1)
				goto usage;
			pending_mode = FALSE;
		}

		// architecture
		else if (pending_arch)
		{
			for (curarch = 0; curarch < ARRAY_LENGTH(dasm_table); curarch++)
				if (core_stricmp(curarg, dasm_table[curarch].name) == 0)
					break;
			if (curarch == ARRAY_LENGTH(dasm_table))
				goto usage;
			opts->dasm = &dasm_table[curarch];
			pending_arch = FALSE;
		}

		// skip bytes
		else if (pending_skip)
		{
			if (sscanf(curarg, "%d", &opts->skip) != 1)
				goto usage;
			pending_skip = FALSE;
		}

		// size
		else if (pending_count)
		{
			if (sscanf(curarg, "%d", &opts->count) != 1)
				goto usage;
			pending_count = FALSE;
		}

		// filename
		else if (opts->filename == NULL)
			opts->filename = curarg;

		// fail
		else
			goto usage;
	}

	// if we have a dangling option, error
	if (pending_base || pending_arch || pending_mode || pending_skip || pending_count)
		goto usage;

	// if no file or no architecture, fail
	if (opts->filename == NULL || opts->dasm == NULL)
		goto usage;
	return 0;

usage:
	printf("Usage: %s <filename> -arch <architecture> [-basepc <pc>] \n", argv[0]);
	printf("   [-mode <n>] [-norawbytes] [-flipped] [-upper] [-lower]\n");
	printf("   [-skip <n>] [-count <n>]\n");
	printf("\n");
	printf("Supported architectures:");
	numrows = (ARRAY_LENGTH(dasm_table) + 6) / 7;
	for (curarch = 0; curarch < numrows * 7; curarch++)
	{
		int row = curarch / 7;
		int col = curarch % 7;
		int index = col * numrows + row;
		if (col == 0)
			printf("\n  ");
		printf("%-11s", (index < ARRAY_LENGTH(dasm_table)) ? dasm_table[index].name : "");
	}
	printf("\n");
	return 1;
};


int main(int argc, char *argv[])
{
	file_error filerr;
	int displayendian;
	int displaychunk;
	UINT32 curbyte;
	UINT32 length;
	int maxchunks;
	UINT32 curpc;
	options opts;
	int numbytes;
	void *data;
	char *p;
	int result = 0;

	// parse options first
	if (parse_options(argc, argv, &opts))
		return 1;

	// load the file
	filerr = core_fload(opts.filename, &data, &length);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Error opening file '%s'\n", opts.filename);
		return 1;
	}

	// precompute parameters
	displaychunk = (opts.dasm->display / 2) + 1;
	displayendian = opts.dasm->display % 2;
	switch (displaychunk)
	{
		case 1:		maxchunks = 6;	break;
		case 2:		maxchunks = 3;	break;
		default:	maxchunks = 1;	break;
	}

	// run it
	try
	{
		if (length > opts.skip)
			length = length - opts.skip;
		if ((length > opts.count) && (opts.count != 0))
			length = opts.count;
		curpc = opts.basepc;
		for (curbyte = 0; curbyte < length; curbyte += numbytes)
		{
			UINT8 *oprom = (UINT8 *)data + opts.skip + curbyte;
			char buffer[1024];
			UINT32 pcdelta;
			int numchunks;

			// disassemble
			pcdelta = (*opts.dasm->func)(NULL, buffer, curpc, oprom, oprom, opts.mode) & DASMFLAG_LENGTHMASK;

			if (opts.dasm->pcshift < 0)
				numbytes = pcdelta << -opts.dasm->pcshift;
			else
				numbytes = pcdelta >> opts.dasm->pcshift;

			// force upper or lower
			if (opts.lower)
			{
				for (p = buffer; *p != 0; p++)
					*p = tolower((UINT8)*p);
			}
			else if (opts.upper)
			{
				for (p = buffer; *p != 0; p++)
					*p = toupper((UINT8)*p);
			}

			// round to the nearest display chunk
			numbytes = ((numbytes + displaychunk - 1) / displaychunk) * displaychunk;
			if (numbytes == 0)
				numbytes = displaychunk;
			numchunks = numbytes / displaychunk;

			// non-flipped case
			if (!opts.flipped)
			{
				// output the address
				printf("%08X: ", curpc);

				// output the raw bytes
				if (!opts.norawbytes)
				{
					int firstchunks = (numchunks < maxchunks) ? numchunks : maxchunks;
					int chunknum, bytenum;
					for (chunknum = 0; chunknum < firstchunks; chunknum++)
					{
						for (bytenum = 0; bytenum < displaychunk; bytenum++)
							printf("%02X", oprom[displayendian ? (displaychunk - 1 - bytenum) : bytenum]);
						printf(" ");
						oprom += displaychunk;
					}
					for ( ; chunknum < maxchunks; chunknum++)
						printf("%*s ", displaychunk * 2, "");
					printf(" ");
				}

				// output the disassembly
				printf("%s\n", buffer);

				// output additional raw bytes
				if (!opts.norawbytes && numchunks > maxchunks)
				{
					for (numchunks -= maxchunks; numchunks > 0; numchunks -= maxchunks)
					{
						int firstchunks = (numchunks < maxchunks) ? numchunks : maxchunks;
						int chunknum, bytenum;
						printf("          ");
						for (chunknum = 0; chunknum < firstchunks; chunknum++)
						{
							for (bytenum = 0; bytenum < displaychunk; bytenum++)
								printf("%02X", oprom[displayendian ? (displaychunk - 1 - bytenum) : bytenum]);
							printf(" ");
							oprom += displaychunk;
						}
						printf("\n");
					}
				}
			}

			// flipped case
			else
			{
				// output the disassembly and address
				printf("\t%-40s ; %08X", buffer, curpc);

				// output the raw bytes
				if (!opts.norawbytes)
				{
					int chunknum, bytenum;
					printf(": ");
					for (chunknum = 0; chunknum < numchunks; chunknum++)
					{
						for (bytenum = 0; bytenum < displaychunk; bytenum++)
							printf("%02X", oprom[displayendian ? (displaychunk - 1 - bytenum) : bytenum]);
						printf(" ");
						oprom += displaychunk;
					}
				}
				printf("\n");
			}

			// advance
			curpc += pcdelta;
		}
	}
	catch (emu_fatalerror &fatal)
	{
		fprintf(stderr, "%s\n", fatal.string());
		if (fatal.exitcode() != 0)
			result = fatal.exitcode();
	}
	catch (emu_exception &)
	{
		fprintf(stderr, "Caught unhandled emulator exception\n");
	}
	catch (std::bad_alloc &)
	{
		fprintf(stderr, "Out of memory!\n");
	}
	catch (...)
	{
		fprintf(stderr, "Caught unhandled exception\n");
	}

	osd_free(data);

	return result;
}
