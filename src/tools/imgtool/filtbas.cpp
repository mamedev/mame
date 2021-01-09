// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    filtbas.c

    Filter for Microsoft-style tokenized BASIC files

    BASIC files typically follow the following format:

        int8     $ff
        int16    <TOTAL LENGTH>
        ...
        int16    <PTR_NEXT_LINE>
        int16    <LINE_NUM>
        int8[]   <TOKENISED_DATA>
        int8     $00     End of line delimiter

*****************************************************************************/

#include <cstring>
#include <cstdarg>
#include <cctype>

#include "imgtool.h"
#include "formats/imageutl.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define EOLN (CRLF == 1 ? "\r" : (CRLF == 2 ? "\n" : (CRLF == 3 ? "\r\n" : NULL)))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct basictoken_tableent
{
	uint8_t shift;
	uint8_t base;
	const char *const *tokens;
	int num_tokens;
};



struct basictokens
{
	uint16_t baseaddress;
	uint8_t size_pos;
	unsigned int skip_bytes : 15;
	unsigned char bytes[20];
	unsigned int be : 1;
	const basictoken_tableent *entries;
	int num_entries;
};



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    basic_readfile - reads a file and decodes
    BASIC tokens into ASCII text
-------------------------------------------------*/

static imgtoolerr_t basic_readfile(const basictokens *tokens,
	imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &destf)
{
	imgtoolerr_t err;
	imgtool::stream::ptr mem_stream;
	uint8_t line_header[4];
	uint16_t line_number; //, address;
	uint8_t b, shift;
	int i;
	int in_string = false;
	const basictoken_tableent *token_table;
	const char *token;

	/* open a memory stream */
	mem_stream = imgtool::stream::open_mem(nullptr, 0);
	if (!mem_stream)
		return IMGTOOLERR_OUTOFMEMORY;

	/* read actual file */
	err = partition.read_file(filename, fork, *mem_stream, nullptr);
	if (err)
		return err;

	/* skip first few bytes */
	mem_stream->seek(tokens->skip_bytes, SEEK_SET);

	/* keep reading line headers */
	while(mem_stream->read(line_header, sizeof(line_header)) == sizeof(line_header))
	{
		/* pluck the address and line number out */
		if (tokens->be)
		{
			//address = (uint16_t)
			pick_integer_be(line_header, 0, 2);
			line_number = (uint16_t) pick_integer_be(line_header, 2, 2);
		}
		else
		{
			//address = (uint16_t)
			pick_integer_le(line_header, 0, 2);
			line_number = (uint16_t) pick_integer_le(line_header, 2, 2);
		}

		/* write the line number */
		destf.printf("%u ", (unsigned) line_number);
		shift = 0x00;

		in_string = false; // in case the last line didn't terminate a string

		while((mem_stream->read(&b, 1) > 0) && (b != 0x00))
		{
			if (b == 0x22)
				in_string = in_string ? false : true;

			if ((b & 0x80) && (!in_string))
			{
				token = nullptr;

				for (i = 0; i < tokens->num_entries; i++)
				{
					token_table = &tokens->entries[i];
					if (token_table->shift == shift)
					{
						if ((b - 0x80) < token_table->num_tokens)
						{
							token = token_table->tokens[b - 0x80];
							if (token)
								shift = 0x00;
						}
					}

					if (token_table->shift == b)
					{
						shift = token_table->shift;
						break;
					}
				}

				if (shift == 0x00)
					destf.puts(token ? token : "!");
			}
			else
			{
				destf.putc((char) b);
			}
		}

		destf.puts(EOLN);
	}

	return err;
}



/*-------------------------------------------------
    basic_writefile - translates ASCII text to
    BASIC tokens and writes it to a file
-------------------------------------------------*/

static imgtoolerr_t basic_writefile(const basictokens *tokens,
	imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	imgtool::stream::ptr mem_stream;
	char buf[1024];
	int eof = false;
	uint32_t len;
	char c;
	int i, j, pos, in_quotes;
	uint16_t line_number;
	uint8_t line_header[4];
	uint8_t file_size[2];
	const basictoken_tableent *token_table;
	const char *token;
	uint8_t token_shift, token_value;
	uint16_t address;

	/* open a memory stream */
	mem_stream = imgtool::stream::open_mem(nullptr, 0);
	if (!mem_stream)
		return IMGTOOLERR_OUTOFMEMORY;

	/* write header */
	mem_stream->write(tokens->bytes, tokens->skip_bytes);
	/* loop until the file is complete */
	while(!eof)
	{
		/* read a line */
		pos = 0;
		while((len = sourcef.read(&c, 1)) > 0)
		{
			/* break if at end of line */
			if ((c == '\r') || (c == '\n'))
				break;

			if (pos <= ARRAY_LENGTH(buf) - 1)
			{
				buf[pos++] = c;
			}
		}
		eof = (len == 0);
		buf[pos] = '\0';

		/* ignore lines that don't start with digits */
		if (isdigit(buf[0]))
		{
			/* start at beginning of line */
			pos = 0;

			/* read line number */
			line_number = 0;
			while(isdigit(buf[pos]))
			{
				line_number *= 10;
				line_number += (buf[pos++] - '0');
			}

			/* determine address */
			if (tokens->baseaddress != 0)
			{
				address = tokens->baseaddress + (uint16_t)mem_stream->size() + 4;
			}
			else
			{
				address = 0x0000;
			}

			/* set up line header */
			memset(&line_header, 0, sizeof(line_header));
			if (tokens->be)
			{
				place_integer_be(line_header, 0, 2, address);
				place_integer_be(line_header, 2, 2, line_number);
			}
			else
			{
				place_integer_le(line_header, 0, 2, address);
				place_integer_le(line_header, 2, 2, line_number);
			}

			/* emit line header */
			mem_stream->write(line_header, sizeof(line_header));

			/* skip spaces */
			while(isspace(buf[pos]))
				pos++;

			/* when we start out, we are not within quotation marks */
			in_quotes = false;

			/* read until end of line */
			while(buf[pos] != '\0')
			{
				token = nullptr;
				token_shift = 0;
				token_value = 0;

				if (buf[pos] == '\"')
				{
					/* flip quotation status */
					in_quotes = !in_quotes;
				}
				else if (!in_quotes)
				{
					for (i = 0; (token == nullptr) && (i < tokens->num_entries); i++)
					{
						bool found = false;
						token_table = &tokens->entries[i];
						for (j = 0; (token == nullptr) && (j < token_table->num_tokens); j++)
						{
							if (!strncmp(&buf[pos], token_table->tokens[j], strlen(token_table->tokens[j])))
							{
								token = token_table->tokens[j];
								token_shift = token_table->shift;
								token_value = token_table->base + j;
								pos += strlen(token);
								found = true;
								break;
							}
						}
						if (found)
							break;
					}
				}

				/* did we find a token? */
				if (token != nullptr)
				{
					/* emit the token */
					if (token_shift != 0)
						mem_stream->write(&token_shift, 1);
					mem_stream->write(&token_value, 1);
				}
				else
				{
					/* no token; emit the byte */
					mem_stream->write(&buf[pos++], 1);
				}
			}

			/* emit line terminator */
			mem_stream->fill(0x00, 1);
		}
	}

	/* emit program terminator */
	mem_stream->fill(0x00, 2);

	/* reset stream */
	mem_stream->seek(tokens->size_pos, SEEK_SET);

	/* this is somewhat gross */
	if (tokens->skip_bytes >= 3)
	{
		if (tokens->be)
		{
			place_integer_be(file_size, 0, 2, mem_stream->size());
		}
		else
		{
			place_integer_le(file_size, 0, 2, mem_stream->size());
		}
		mem_stream->write(file_size, 2);
		mem_stream->seek(0, SEEK_SET);
	}

	/* write actual file */
	return partition.write_file(filename, fork, *mem_stream, opts, nullptr);
}



/***************************************************************************
    TOKEN DEFINITIONS
***************************************************************************/

static const char *const cocobas_statements[] =
{
	"FOR",      /* 0x80 */
	"GO",       /* 0x81 */
	"REM",      /* 0x82 */
	"'",        /* 0x83 */
	"ELSE",     /* 0x84 */
	"IF",       /* 0x85 */
	"DATA",     /* 0x86 */
	"PRINT",    /* 0x87 */
	"ON",       /* 0x88 */
	"INPUT",    /* 0x89 */
	"END",      /* 0x8a */
	"NEXT",     /* 0x8b */
	"DIM",      /* 0x8c */
	"READ",     /* 0x8d */
	"RUN",      /* 0x8e */
	"RESTORE",  /* 0x8f */
	"RETURN",   /* 0x90 */
	"STOP",     /* 0x91 */
	"POKE",     /* 0x92 */
	"CONT",     /* 0x93 */
	"LIST",     /* 0x94 */
	"CLEAR",    /* 0x95 */
	"NEW",      /* 0x96 */
	"CLOAD",    /* 0x97 */
	"CSAVE",    /* 0x98 */
	"OPEN",     /* 0x99 */
	"CLOSE",    /* 0x9a */
	"LLIST",    /* 0x9b */
	"SET",      /* 0x9c */
	"RESET",    /* 0x9d */
	"CLS",      /* 0x9e */
	"MOTOR",    /* 0x9f */
	"SOUND",    /* 0xa0 */
	"AUDIO",    /* 0xa1 */
	"EXEC",     /* 0xa2 */
	"SKIPF",    /* 0xa3 */
	"TAB(",     /* 0xa4 */
	"TO",       /* 0xa5 */
	"SUB",      /* 0xa6 */
	"THEN",     /* 0xa7 */
	"NOT",      /* 0xa8 */
	"STEP",     /* 0xa9 */
	"OFF",      /* 0xaa */
	"+",        /* 0xab */
	"-",        /* 0xac */
	"*",        /* 0xad */
	"/",        /* 0xae */
	"^",        /* 0xaf */
	"AND",      /* 0xb0 */
	"OR",       /* 0xb1 */
	">",        /* 0xb2 */
	"=",        /* 0xb3 */
	"<",        /* 0xb4 */
	"DEL",      /* 0xb5 */
	"EDIT",     /* 0xb6 */
	"TRON",     /* 0xb7 */
	"TROFF",    /* 0xb8 */
	"DEF",      /* 0xb9 */
	"LET",      /* 0xba */
	"LINE",     /* 0xbb */
	"PCLS",     /* 0xbc */
	"PSET",     /* 0xbd */
	"PRESET",   /* 0xbe */
	"SCREEN",   /* 0xbf */
	"PCLEAR",   /* 0xc0 */
	"COLOR",    /* 0xc1 */
	"CIRCLE",   /* 0xc2 */
	"PAINT",    /* 0xc3 */
	"GET",      /* 0xc4 */
	"PUT",      /* 0xc5 */
	"DRAW",     /* 0xc6 */
	"PCOPY",    /* 0xc7 */
	"PMODE",    /* 0xc8 */
	"PLAY",     /* 0xc9 */
	"DLOAD",    /* 0xca */
	"RENUM",    /* 0xcb */
	"FN",       /* 0xcc */
	"USING",    /* 0xcd */
	"DIR",      /* 0xce */
	"DRIVE",    /* 0xcf */
	"FIELD",    /* 0xd0 */
	"FILES",    /* 0xd1 */
	"KILL",     /* 0xd2 */
	"LOAD",     /* 0xd3 */
	"LSET",     /* 0xd4 */
	"MERGE",    /* 0xd5 */
	"RENAME",   /* 0xd6 */
	"RSET",     /* 0xd7 */
	"SAVE",     /* 0xd8 */
	"WRITE",    /* 0xd9 */
	"VERIFY",   /* 0xda */
	"UNLOAD",   /* 0xdb */
	"DSKINI",   /* 0xdc */
	"BACKUP",   /* 0xdd */
	"COPY",     /* 0xde */
	"DSKI$",    /* 0xdf */
	"DSKO$",    /* 0xe0 */
	"DOS",      /* 0xe1 */
	"WIDTH",    /* 0xe2 */
	"PALETTE",  /* 0xe3 */
	"HSCREEN",  /* 0xe4 */
	"LPOKE",    /* 0xe5 */
	"HCLS",     /* 0xe6 */
	"HCOLOR",   /* 0xe7 */
	"HPAINT",   /* 0xe8 */
	"HCIRCLE",  /* 0xe9 */
	"HLINE",    /* 0xea */
	"HGET",     /* 0xeb */
	"HPUT",     /* 0xec */
	"HBUFF",    /* 0xed */
	"HPRINT",   /* 0xee */
	"ERR",      /* 0xef */
	"BRK",      /* 0xf0 */
	"LOCATE",   /* 0xf1 */
	"HSTAT",    /* 0xf2 */
	"HSET",     /* 0xf3 */
	"HRESET",   /* 0xf4 */
	"HDRAW",    /* 0xf5 */
	"CMP",      /* 0xf6 */
	"RGB",      /* 0xf7 */
	"ATTR"      /* 0xf8 */
};

static const char *const cocobas_functions[] =
{
	"SGN",      /* 0xff80 */
	"INT",      /* 0xff81 */
	"ABS",      /* 0xff82 */
	"USR",      /* 0xff83 */
	"RND",      /* 0xff84 */
	"SIN",      /* 0xff85 */
	"PEEK",     /* 0xff86 */
	"LEN",      /* 0xff87 */
	"STR$",     /* 0xff88 */
	"VAL",      /* 0xff89 */
	"ASC",      /* 0xff8a */
	"CHR$",     /* 0xff8b */
	"EOF",      /* 0xff8c */
	"JOYSTK",   /* 0xff8d */
	"LEFT$",    /* 0xff8e */
	"RIGHT$",   /* 0xff8f */
	"MID$",     /* 0xff90 */
	"POINT",    /* 0xff91 */
	"INKEY$",   /* 0xff92 */
	"MEM",      /* 0xff93 */
	"ATN",      /* 0xff94 */
	"COS",      /* 0xff95 */
	"TAN",      /* 0xff96 */
	"EXP",      /* 0xff97 */
	"FIX",      /* 0xff98 */
	"LOG",      /* 0xff99 */
	"POS",      /* 0xff9a */
	"SQR",      /* 0xff9b */
	"HEX$",     /* 0xff9c */
	"VARPTR",   /* 0xff9d */
	"INSTR",    /* 0xff9e */
	"TIMER",    /* 0xff9f */
	"PPOINT",   /* 0xffa0 */
	"STRING$",  /* 0xffa1 */
	"CVN",      /* 0xffa2 */
	"FREE",     /* 0xffa3 */
	"LOC",      /* 0xffa4 */
	"LOF",      /* 0xffa5 */
	"MKN$",     /* 0xffa6 */
	"AS",       /* 0xffa7 */
	"LPEEK",    /* 0xffa8 */
	"BUTTON",   /* 0xffa9 */
	"HPOINT",   /* 0xffaa */
	"ERNO",     /* 0xffab */
	"ERLIN"     /* 0xffac */
};

static const char *const dragonbas_statements[] =
{
	"FOR",      /* 0x80 */
	"GO",       /* 0x81 */
	"REM",      /* 0x82 */
	"'",        /* 0x83 */
	"ELSE",     /* 0x84 */
	"IF",       /* 0x85 */
	"DATA",     /* 0x86 */
	"PRINT",    /* 0x87 */
	"ON",       /* 0x88 */
	"INPUT",    /* 0x89 */
	"END",      /* 0x8a */
	"NEXT",     /* 0x8b */
	"DIM",      /* 0x8c */
	"READ",     /* 0x8d */
	"LET",      /* 0x8e */
	"RUN",      /* 0x8f */
	"RESTORE",  /* 0x90 */
	"RETURN",   /* 0x91 */
	"STOP",     /* 0x92 */
	"POKE",     /* 0x93 */
	"CONT",     /* 0x94 */
	"LIST",     /* 0x95 */
	"CLEAR",    /* 0x96 */
	"NEW",      /* 0x97 */
	"DEF",      /* 0x98 */
	"CLOAD",    /* 0x99 */
	"CSAVE",    /* 0x9a */
	"OPEN",     /* 0x9b */
	"CLOSE",    /* 0x9c */
	"LLIST",    /* 0x9d */
	"SET",      /* 0x9e */
	"RESET",    /* 0x9f */
	"CLS",      /* 0xa0 */
	"MOTOR",    /* 0xa1 */
	"SOUND",    /* 0xa2 */
	"AUDIO",    /* 0xa3 */
	"EXEC",     /* 0xa4 */
	"SKIPF",    /* 0xa5 */
	"DEL",      /* 0xa6 */
	"EDIT",     /* 0xa7 */
	"TRON",     /* 0xa8 */
	"TROFF",    /* 0xa9 */
	"LINE",     /* 0xaa */
	"PCLS",     /* 0xab */
	"PSET",     /* 0xac */
	"PRESET",   /* 0xad */
	"SCREEN",   /* 0xae */
	"PCLEAR",   /* 0xaf */
	"COLOR",    /* 0xb0 */
	"CIRCLE",   /* 0xb1 */
	"PAINT",    /* 0xb2 */
	"GET",      /* 0xb3 */
	"PUT",      /* 0xb4 */
	"DRAW",     /* 0xb5 */
	"PCOPY",    /* 0xb6 */
	"PMODE",    /* 0xb7 */
	"PLAY",     /* 0xb8 */
	"DLOAD",    /* 0xb9 */
	"RENUM",    /* 0xba */
	"TAB(",     /* 0xbb */
	"TO",       /* 0xbc */
	"SUB",      /* 0xbd */
	"FN",       /* 0xbe */
	"THEN",     /* 0xbf */
	"NOT",      /* 0xc0 */
	"STEP",     /* 0xc1 */
	"OFF",      /* 0xc2 */
	"+",        /* 0xc3 */
	"-",        /* 0xc4 */
	"*",        /* 0xc5 */
	"/",        /* 0xc6 */
	"^",        /* 0xc7 */
	"AND",      /* 0xc8 */
	"OR",       /* 0xc9 */
	">",        /* 0xca */
	"=",        /* 0xcb */
	"<",        /* 0xcc */
	"USING",    /* 0xcd */
	"AUTO",     /* 0xce */
	"BACKUP",   /* 0xcf */
	"BEEP",     /* 0xd0 */
	"BOOT",     /* 0xd1 */
	"CHAIN",    /* 0xd2 */
	"COPY",     /* 0xd3 */
	"CREATE",   /* 0xd4 */
	"DIR",      /* 0xd5 */
	"DRIVE",    /* 0xd6 */
	"DSKINIT",  /* 0xd7 */
	"FREAD",    /* 0xd8 */
	"FWRITE",   /* 0xd9 */
	"ERROR",    /* 0xda */
	"KILL",     /* 0xdb */
	"LOAD",     /* 0xdc */
	"MERGE",    /* 0xdd */
	"PROTECT",  /* 0xde */
	"WAIT",     /* 0xdf */
	"RENAME",   /* 0xe0 */
	"SAVE",     /* 0xe1 */
	"SREAD",    /* 0xe2 */
	"SWRITE",   /* 0xe3 */
	"VERIFY",   /* 0xe4 */
	"FROM",     /* 0xe5 */
	"FLREAD",   /* 0xe6 */
	"SWAP"      /* 0xe7 */
};

static const char *const dragonbas_functions[] =
{
	"SGN",      /* 0xff80 */
	"INT",      /* 0xff81 */
	"ABS",      /* 0xff82 */
	"POS",      /* 0xff83 */
	"RND",      /* 0xff84 */
	"SQR",      /* 0xff85 */
	"LOG",      /* 0xff86 */
	"EXP",      /* 0xff87 */
	"SIN",      /* 0xff88 */
	"COS",      /* 0xff89 */
	"TAN",      /* 0xff8a */
	"ATN",      /* 0xff8b */
	"PEEK",     /* 0xff8c */
	"LEN",      /* 0xff8d */
	"STR$",     /* 0xff8e */
	"VAL",      /* 0xff8f */
	"ASC",      /* 0xff90 */
	"CHR$",     /* 0xff91 */
	"EOF",      /* 0xff92 */
	"JOYSTK",   /* 0xff93 */
	"FIX",      /* 0xff94 */
	"HEX$",     /* 0xff95 */
	"LEFT$",    /* 0xff96 */
	"RIGHT$",   /* 0xff97 */
	"MID$",     /* 0xff98 */
	"POINT",    /* 0xff99 */
	"INKEY$",   /* 0xff9a */
	"MEM",      /* 0xff9b */
	"VARPTR",   /* 0xff9c */
	"INSTR",    /* 0xff9d */
	"TIMER",    /* 0xff9e */
	"PPOINT",   /* 0xff9f */
	"STRING$",  /* 0xffa0 */
	"USR",      /* 0xffa1 */
	"LOF",      /* 0xffa2 */
	"FREE",     /* 0xffa3 */
	"ERL",      /* 0xffa4 */
	"ERR",      /* 0xffa5 */
	"HIMEM",    /* 0xffa6 */
	"LOC",      /* 0xffa7 */
	"FRE$"      /* 0xffa8 */
};

static const char *const vzbas[] =
{
	"END",      /* 0x80 */
	"FOR",      /* 0x81 */
	"RESET",    /* 0x82 */
	"SET",      /* 0x83 */
	"CLS",      /* 0x84 */
	nullptr,       /* 0x85 */
	"RANDOM",   /* 0x86 */
	"NEXT",     /* 0x87 */
	"DATA",     /* 0x88 */
	"INPUT",    /* 0x89 */
	"DIM",      /* 0x8a */
	"READ",     /* 0x8b */
	"LET",      /* 0x8c */
	"GOTO",     /* 0x8d */
	"RUN",      /* 0x8e */
	"IF",       /* 0x8f */
	"RESTORE",  /* 0x90 */
	"GOSUB",    /* 0x91 */
	"RETURN",   /* 0x92 */
	"'",        /* 0x93 */
	"STOP",     /* 0x94 */
	"ELSE",     /* 0x95 */
	"COPY",     /* 0x96 */
	"COLOR",    /* 0x97 */
	"VERIFY",   /* 0x98 */
	"DEFINT",   /* 0x99 */
	"DEFSNG",   /* 0x9a */
	"DEFDBL",   /* 0x9b */
	"CRUN",     /* 0x9c */
	"MODE",     /* 0x9d */
	"ERROR",    /* 0x9e */
	"RESUME",   /* 0x9f */
	"OUT",      /* 0xa0 */
	"IN",       /* 0xa1 */
	nullptr,       /* 0xa2 */
	nullptr,       /* 0xa3 */
	nullptr,       /* 0xa4 */
	nullptr,       /* 0xa5 */
	nullptr,       /* 0xa6 */
	nullptr,       /* 0xa7 */
	nullptr,       /* 0xa8 */
	nullptr,       /* 0xa9 */
	nullptr,       /* 0xaa */
	nullptr,       /* 0xab */
	nullptr,       /* 0xac */
	nullptr,       /* 0xad */
	"(RESET)",  /* 0xae */
	"LPRINT",   /* 0xaf */
	nullptr,       /* 0xb0 */
	"POKE",     /* 0xb1 */
	"PRINT",    /* 0xb2 */
	"CONT",     /* 0xb3 */
	"LIST",     /* 0xb4 */
	"LLIST",    /* 0xb5 */
	"DELETE",   /* 0xb6 */
	"AUTO",     /* 0xb7 */
	"CLEAR",    /* 0xb8 */
	"CLOAD",    /* 0xb9 */
	"CSAVE",    /* 0xba */
	"NEW",      /* 0xbb */
	"TAB(",     /* 0xbc */
	"TO",       /* 0xbd */
	nullptr,       /* 0xbe */
	"USING",    /* 0xbf */
	"VARPTR",   /* 0xc0 */
	"USR",      /* 0xc1 */
	"ERL",      /* 0xc2 */
	"ERR",      /* 0xc3 */
	"STRING$",  /* 0xc4 */
	nullptr,       /* 0xc5 */
	"POINT",    /* 0xc6 */
	nullptr,       /* 0xc7 */
	"MEM",      /* 0xc8 */
	"INKEY$",   /* 0xc9 */
	"THEN",     /* 0xca */
	"NOT",      /* 0xcb */
	"STEP",     /* 0xcc */
	"+",        /* 0xcd */
	"-",        /* 0xce */
	"*",        /* 0xcf */
	"/",        /* 0xd0 */
	"^",        /* 0xd1 */
	"AND",      /* 0xd2 */
	"OR",       /* 0xd3 */
	">",        /* 0xd4 */
	"=",        /* 0xd5 */
	"<",        /* 0xd6 */
	"SGN",      /* 0xd7 */
	"INT",      /* 0xd8 */
	"ABS",      /* 0xd9 */
	"FRE",      /* 0xda */
	"INP",      /* 0xdb */
	"POS",      /* 0xdc */
	"SQR",      /* 0xdd */
	"AND",      /* 0xde */
	"LOG",      /* 0xdf */
	"EXP",      /* 0xe0 */
	"COS",      /* 0xe1 */
	"SIN",      /* 0xe2 */
	"TAN",      /* 0xe3 */
	"ATN",      /* 0xe4 */
	"PEEK",     /* 0xe5 */
	nullptr,       /* 0xe6 */
	nullptr,       /* 0xe7 */
	nullptr,       /* 0xe8 */
	nullptr,       /* 0xe9 */
	nullptr,       /* 0xea */
	nullptr,       /* 0xeb */
	nullptr,       /* 0xec */
	nullptr,       /* 0xed */
	nullptr,       /* 0xee */
	"CINT",     /* 0xef */
	"CSNG",     /* 0xf0 */
	"CDBL",     /* 0xf1 */
	"FIX",      /* 0xf2 */
	"LEN",      /* 0xf3 */
	"STR$",     /* 0xf4 */
	"VAL",      /* 0xf5 */
	"ASC",      /* 0xf6 */
	"CHR$",     /* 0xf7 */
	"LEFT$",    /* 0xf8 */
	"RIGHT$",   /* 0xf9 */
	"MID$",     /* 0xfa */
	nullptr,       /* 0xfb */
	nullptr,       /* 0xfc */
	nullptr,       /* 0xfd */
	nullptr,       /* 0xfe */
	nullptr        /* 0xff */
};

static const char *const bml3bas_statements[] =
{
	"END",      /* 0x80 */
	"FOR",      /* 0x81 */
	"NEXT",     /* 0x82 */
	"DATA",     /* 0x83 */
	"DIM",      /* 0x84 */
	"READ",     /* 0x85 */
	"LET",      /* 0x86 */
	"GO",       /* 0x87 */
	"RUN",      /* 0x88 */
	"IF",       /* 0x89 */
	"RESTORE",  /* 0x8a */
	"RETURN",   /* 0x8b */
	"REM",      /* 0x8c */
	"'",        /* 0x8d */
	"STOP",     /* 0x8e */
	"ELSE",     /* 0x8f */
	"TRON",     /* 0x90 */
	"TROFF",    /* 0x91 */
	"SWAP",     /* 0x92 */
	"DEFSTR",   /* 0x93 */
	"DEFINT",   /* 0x94 */
	"DEFSNG",   /* 0x95 */
	"DEFDBL",   /* 0x96 */
	"ON",       /* 0x97 */
	"WAIT",     /* 0x98 */
	"RENUM",    /* 0x99 */
	"EDIT",     /* 0x9a */
	"ERROR",    /* 0x9b */
	"RESUME",   /* 0x9c */
	"AUTO",     /* 0x9d */
	"DELETE",   /* 0x9e */
	"TERM",     /* 0x9f */
	"WIDTH",    /* 0xa0 */
	"UNLIST",   /* 0xa1 */
	"MON",      /* 0xa2 */
	"LOCATE",   /* 0xa3 */
	"CLS",      /* 0xa4 */
	"CONSOLE",  /* 0xa5 */
	"PSET",     /* 0xa6 */
	"PRESET",   /* 0xa7 */
	"MOTOR",    /* 0xa8 */
	"SKIPF",    /* 0xa9 */
	"SAVE",     /* 0xaa */
	"LOAD",     /* 0xab */
	"MERGE",    /* 0xac */
	"EXEC",     /* 0xad */
	"OPEN",     /* 0xae */
	"CLOSE",    /* 0xaf */
	"FILES",    /* 0xb0 */
	"COM",      /* 0xb1 */
	"KEY",      /* 0xb2 */
	"PAINT",    /* 0xb3 */
	"BEEP",     /* 0xb4 */
	"COLOR",    /* 0xb5 */
	"LINE",     /* 0xb6 */
	"DEF",      /* 0xb7 */
	"POKE",     /* 0xb8 */
	"PRINT",    /* 0xb9 */
	"CONT",     /* 0xba */
	"LIST",     /* 0xbb */
	"CLEAR",    /* 0xbc */
	"RANDOMIZE",/* 0xbd */
	"WHILE",    /* 0xbe */
	"WEND",     /* 0xbf */
	"NEW",      /* 0xc0 */
	"TAB(",     /* 0xc1 */
	"TO",       /* 0xc2 */
	"SUB",      /* 0xc3 */
	"FN",       /* 0xc4 */
	"SPC(",     /* 0xc5 */
	"USING",    /* 0xc6 */
	"USR",      /* 0xc7 */
	"ERL",      /* 0xc8 */
	"ERR",      /* 0xc9 */
	"OFF",      /* 0xca */
	"THEN",     /* 0xcb */
	"NOT",      /* 0xcc */
	"STEP",     /* 0xcd */
	"+",        /* 0xce */
	"-",        /* 0xcf */
	"*",        /* 0xd0 */
	"/",        /* 0xd1 */
	"^",        /* 0xd2 */
	"AND",      /* 0xd3 */
	"OR",       /* 0xd4 */
	"XOR",      /* 0xd5 */
	"EQV",      /* 0xd6 */
	"IMP",      /* 0xd7 */
	"MOD",      /* 0xd8 */
	"\\",       /* 0xd9 */
	">",        /* 0xda */
	"=",        /* 0xdb */
	"<"         /* 0xdc */
};

static const char *const bml3bas_functions[] =
{
	"SGN",      /* 0xff80 */
	"INT",      /* 0xff81 */
	"ABS",      /* 0xff82 */
	"FRE",      /* 0xff83 */
	"POS",      /* 0xff84 */
	"SQR",      /* 0xff85 */
	"LOG",      /* 0xff86 */
	"EXP",      /* 0xff87 */
	"COS",      /* 0xff88 */
	"SIN",      /* 0xff89 */
	"TAN",      /* 0xff8a */
	"ATN",      /* 0xff8b */
	"PEEK",     /* 0xff8c */
	"LEN",      /* 0xff8d */
	"STR$",     /* 0xff8e */
	"VAL",      /* 0xff8f */
	"ASC",      /* 0xff90 */
	"CHR$",     /* 0xff91 */
	"CINT",     /* 0xff92 */
	"CSNG",     /* 0xff93 */
	"CDBL",     /* 0xff94 */
	"FIX",      /* 0xff95 */
	"SPACE$",   /* 0xff96 */
	"HEX$",     /* 0xff97 */
	"OCT$",     /* 0xff98 */
	"LOF",      /* 0xff99 */
	"EOF",      /* 0xff9a */
	"PEN",      /* 0xff9b */
	"LEFT$",    /* 0xff9c */
	"RIGHT$",   /* 0xff9d */
	"MID$",     /* 0xff9e */
	"INSTR",    /* 0xff9f */
	"SCREEN",   /* 0xffa0 */
	"VARPTR",   /* 0xffa1 */
	"STRING$",  /* 0xffa2 */
	"RND",      /* 0xffa3 */
	"INKEY$",   /* 0xffa4 */
	"INPUT",    /* 0xffa5 */
	"CSRLIN",   /* 0xffa6 */
	"POINT",    /* 0xffa7 */
	"TIME",     /* 0xffa8 */
	"DATE"      /* 0xffa9 */
};


#ifdef BASIC_
/* ----------------------------------------------------------------------- *
 * CBM machines                                                            *
 * ----------------------------------------------------------------------- */
static const char *const basic_10[] = /* "BASIC 1.0" - supported by pet */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	NULL,               /* 0xcb */
	NULL,               /* 0xcc */
	NULL,               /* 0xcd */
	NULL,               /* 0xce */
	NULL,               /* 0xcf */
	NULL,               /* 0xd0 */
	NULL,               /* 0xd1 */
	NULL,               /* 0xd2 */
	NULL,               /* 0xd3 */
	NULL,               /* 0xd4 */
	NULL,               /* 0xd5 */
	NULL,               /* 0xd6 */
	NULL,               /* 0xd7 */
	NULL,               /* 0xd8 */
	NULL,               /* 0xd9 */
	NULL,               /* 0xda */
	NULL,               /* 0xdb */
	NULL,               /* 0xdc */
	NULL,               /* 0xdd */
	NULL,               /* 0xde */
	NULL,               /* 0xdf */
	NULL,               /* 0xe0 */
	NULL,               /* 0xe1 */
	NULL,               /* 0xe2 */
	NULL,               /* 0xe3 */
	NULL,               /* 0xe4 */
	NULL,               /* 0xe5 */
	NULL,               /* 0xe6 */
	NULL,               /* 0xe7 */
	NULL,               /* 0xe8 */
	NULL,               /* 0xe9 */
	NULL,               /* 0xea */
	NULL,               /* 0xeb */
	NULL,               /* 0xec */
	NULL,               /* 0xed */
	NULL,               /* 0xee */
	NULL,               /* 0xef */
	NULL,               /* 0xf0 */
	NULL,               /* 0xf1 */
	NULL,               /* 0xf2 */
	NULL,               /* 0xf3 */
	NULL,               /* 0xf4 */
	NULL,               /* 0xf5 */
	NULL,               /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_20[] = /* "BASIC 2.0" - supported by vic20 & clones, c64 & clones, cbm30xx series */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	NULL,               /* 0xcc */
	NULL,               /* 0xcd */
	NULL,               /* 0xce */
	NULL,               /* 0xcf */
	NULL,               /* 0xd0 */
	NULL,               /* 0xd1 */
	NULL,               /* 0xd2 */
	NULL,               /* 0xd3 */
	NULL,               /* 0xd4 */
	NULL,               /* 0xd5 */
	NULL,               /* 0xd6 */
	NULL,               /* 0xd7 */
	NULL,               /* 0xd8 */
	NULL,               /* 0xd9 */
	NULL,               /* 0xda */
	NULL,               /* 0xdb */
	NULL,               /* 0xdc */
	NULL,               /* 0xdd */
	NULL,               /* 0xde */
	NULL,               /* 0xdf */
	NULL,               /* 0xe0 */
	NULL,               /* 0xe1 */
	NULL,               /* 0xe2 */
	NULL,               /* 0xe3 */
	NULL,               /* 0xe4 */
	NULL,               /* 0xe5 */
	NULL,               /* 0xe6 */
	NULL,               /* 0xe7 */
	NULL,               /* 0xe8 */
	NULL,               /* 0xe9 */
	NULL,               /* 0xea */
	NULL,               /* 0xeb */
	NULL,               /* 0xec */
	NULL,               /* 0xed */
	NULL,               /* 0xee */
	NULL,               /* 0xef */
	NULL,               /* 0xf0 */
	NULL,               /* 0xf1 */
	NULL,               /* 0xf2 */
	NULL,               /* 0xf3 */
	NULL,               /* 0xf4 */
	NULL,               /* 0xf5 */
	NULL,               /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_20_super_expander_vic[] = /* "BASIC 2.0 with Super Expander" - supported by vic20 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"KEY",            /* 0xcc */
	"GRAPHIC",        /* 0xcd */
	"SCNCLR",         /* 0xce */
	"CIRCLE",         /* 0xcf */
	"DRAW",           /* 0xd0 */
	"REGION",         /* 0xd1 */
	"COLOR",          /* 0xd2 */
	"POINT",          /* 0xd3 */
	"SOUND",          /* 0xd4 */
	"CHAR",           /* 0xd5 */
	"PAINT",          /* 0xd6 */
	"RPOT",           /* 0xd7 */
	"RPEN",           /* 0xd8 */
	"RSND",           /* 0xd9 */
	"RCOLR",          /* 0xda */
	"RGR",            /* 0xdb */
	"RJOY",           /* 0xdc */
	"RDOT",           /* 0xdd */
	NULL,               /* 0xde */
	NULL,               /* 0xdf */
	NULL,               /* 0xe0 */
	NULL,               /* 0xe1 */
	NULL,               /* 0xe2 */
	NULL,               /* 0xe3 */
	NULL,               /* 0xe4 */
	NULL,               /* 0xe5 */
	NULL,               /* 0xe6 */
	NULL,               /* 0xe7 */
	NULL,               /* 0xe8 */
	NULL,               /* 0xe9 */
	NULL,               /* 0xea */
	NULL,               /* 0xeb */
	NULL,               /* 0xec */
	NULL,               /* 0xed */
	NULL,               /* 0xee */
	NULL,               /* 0xef */
	NULL,               /* 0xf0 */
	NULL,               /* 0xf1 */
	NULL,               /* 0xf2 */
	NULL,               /* 0xf3 */
	NULL,               /* 0xf4 */
	NULL,               /* 0xf5 */
	NULL,               /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_20_turtle_basic_10[] = /* "BASIC 2.0 with Turtle BASIC 1.0" - supported by vic20 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"GRAPHIC",        /* 0xcc */
	"OLD",            /* 0xcd */
	"TURN",           /* 0xce */
	"PEN",            /* 0xcf */
	"DRAW",           /* 0xd0 */
	"MOVE",           /* 0xd1 */
	"POINT",          /* 0xd2 */
	"KILL",           /* 0xd3 */
	"WRITE",          /* 0xd4 */
	"REPEAT",         /* 0xd5 */
	"SCREEN",         /* 0xd6 */
	"DOKE",           /* 0xd7 */
	"RELOC",          /* 0xd8 */
	"FILL",           /* 0xd9 */
	"RTIME",          /* 0xda */
	"BASE",           /* 0xdb */
	"PAUSE",          /* 0xdc */
	"POP",            /* 0xdd */
	"COLOR",          /* 0xde */
	"MERGE",          /* 0xdf */
	"CHAR",           /* 0xe0 */
	"TAKE",           /* 0xe1 */
	"SOUND",          /* 0xe2 */
	"VOL",            /* 0xe3 */
	"PUT",            /* 0xe4 */
	"PLACE",          /* 0xe5 */
	"CLS",            /* 0xe6 */
	"ACCEPT",         /* 0xe7 */
	"RESET",          /* 0xe8 */
	"GRAB",           /* 0xe9 */
	"RDOT",           /* 0xea */
	"PLR$",           /* 0xeb */
	"DEEK",           /* 0xec */
	"JOY",            /* 0xed */
	NULL,               /* 0xee */
	NULL,               /* 0xef */
	NULL,               /* 0xf0 */
	NULL,               /* 0xf1 */
	NULL,               /* 0xf2 */
	NULL,               /* 0xf3 */
	NULL,               /* 0xf4 */
	NULL,               /* 0xf5 */
	NULL,               /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_20_speech_basic_27[] = /* "BASIC 2.0 with Speech BASIC 2.7" - supported by c64 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"RESET",          /* 0xcc */
	"BASIC",          /* 0xcd */
	"HELP",           /* 0xce */
	"KEY",            /* 0xcf */
	"HIMEM",          /* 0xd0 */
	"DISK",           /* 0xd1 */
	"DIR",            /* 0xd2 */
	"BLOAD",          /* 0xd3 */
	"BSAVE",          /* 0xd4 */
	"MAP",            /* 0xd5 */
	"MEM",            /* 0xd6 */
	"PAUSE",          /* 0xd7 */
	"BLOCK",          /* 0xd8 */
	"HEAR",           /* 0xd9 */
	"RECORD",         /* 0xda */
	"PLAY",           /* 0xdb */
	"VOLDEF",         /* 0xdc */
	"COLDEF",         /* 0xdd */
	"HEX",            /* 0xde */
	"DEZ",            /* 0xdf */
	"SCREEN",         /* 0xe0 */
	"EXEC",           /* 0xe1 */
	"MON",            /* 0xe2 */
	"{LEFT ARROW}",   /* 0xe3 - A single character shaped as a left pointing arrow */
	"FROM",           /* 0xe4 */
	"SPEED",          /* 0xe5 */
	"OFF",            /* 0xe6 */
	NULL,               /* 0xe7 */
	NULL,               /* 0xe8 */
	NULL,               /* 0xe9 */
	NULL,               /* 0xea */
	NULL,               /* 0xeb */
	NULL,               /* 0xec */
	NULL,               /* 0xed */
	NULL,               /* 0xee */
	NULL,               /* 0xef */
	NULL,               /* 0xf0 */
	NULL,               /* 0xf1 */
	NULL,               /* 0xf2 */
	NULL,               /* 0xf3 */
	NULL,               /* 0xf4 */
	NULL,               /* 0xf5 */
	NULL,               /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_20_at_basic[] = /* "BASIC 2.0 with @BASIC" - supported by c64 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"TRACE",          /* 0xcc */
	"DELETE",         /* 0xcd */
	"AUTO",           /* 0xce */
	"OLD",            /* 0xcf */
	"DUMP",           /* 0xd0 */
	"FIND",           /* 0xd1 */
	"RENUMBER",       /* 0xd2 */
	"DLOAD",          /* 0xd3 */
	"DSAVE",          /* 0xd4 */
	"DVERIFY",        /* 0xd5 */
	"DIRECTORY"       /* 0xd6 */
	"CATALOG",        /* 0xd7 */
	"SCRATCH",        /* 0xd8 */
	"COLLECT",        /* 0xd9 */
	"RENAME",         /* 0xda */
	"COPY",           /* 0xdb */
	"BACKUP",         /* 0xdc */
	"DISK",           /* 0xdd */
	"HEADER",         /* 0xde */
	"APPEND",         /* 0xdf */
	"MERGE",          /* 0xe0 */
	"MLOAD",          /* 0xe1 */
	"MVERIFY",        /* 0xe2 */
	"MSAVE",          /* 0xe3 */
	"KEY",            /* 0xe4 */
	"BASIC",          /* 0xe5 */
	"RESET",          /* 0xe6 */
	"EXIT",           /* 0xe7 */
	"ENTER",          /* 0xe8 */
	"DOKE",           /* 0xe9 */
	"SET",            /* 0xea */
	"HELP",           /* 0xeb */
	"SCREEN",         /* 0xec */
	"LOMEM",          /* 0xed */
	"HIMEM",          /* 0xee */
	"COLOUR",         /* 0xef */
	"TYPE",           /* 0xf0 */
	"TIME",           /* 0xf1 */
	"DEEK",           /* 0xf2 */
	"HEX$",           /* 0xf3 */
	"BIN$",           /* 0xf4 */
	"OFF",            /* 0xf5 */
	"ALARM",          /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_20_simon_s_basic[] = /* "BASIC 2.0 with Simon's BASIC" - supported by c64 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	NULL,               /* 0xcc */
	NULL,               /* 0xcd */
	NULL,               /* 0xce */
	NULL,               /* 0xcf */
	NULL,               /* 0xd0 */
	NULL,               /* 0xd1 */
	NULL,               /* 0xd2 */
	NULL,               /* 0xd3 */
	NULL,               /* 0xd4 */
	NULL,               /* 0xd5 */
	NULL,               /* 0xd6 */
	NULL,               /* 0xd7 */
	NULL,               /* 0xd8 */
	NULL,               /* 0xd9 */
	NULL,               /* 0xda */
	NULL,               /* 0xdb */
	NULL,               /* 0xdc */
	NULL,               /* 0xdd */
	NULL,               /* 0xde */
	NULL,               /* 0xdf */
	NULL,               /* 0xe0 */
	NULL,               /* 0xe1 */
	NULL,               /* 0xe2 */
	NULL,               /* 0xe3 */
	NULL,               /* 0xe4 */
	NULL,               /* 0xe5 */
	NULL,               /* 0xe6 */
	NULL,               /* 0xe7 */
	NULL,               /* 0xe8 */
	NULL,               /* 0xe9 */
	NULL,               /* 0xea */
	NULL,               /* 0xeb */
	NULL,               /* 0xec */
	NULL,               /* 0xed */
	NULL,               /* 0xee */
	NULL,               /* 0xef */
	NULL,               /* 0xf0 */
	NULL,               /* 0xf1 */
	NULL,               /* 0xf2 */
	NULL,               /* 0xf3 */
	NULL,               /* 0xf4 */
	NULL,               /* 0xf5 */
	NULL,               /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}",           /* 0xff - A single character shaped as greek lowercase 'PI' */
	NULL,               /* 0x6400 */
	"HIRES",          /* 0x6401 */
	"PLOT",           /* 0x6402 */
	"LINE",           /* 0x6403 */
	"BLOCK",          /* 0x6404 */
	"FCHR",           /* 0x6405 */
	"FCOL",           /* 0x6406 */
	"FILL",           /* 0x6407 */
	"REC",            /* 0x6408 */
	"ROT",            /* 0x6409 */
	"DRAW",           /* 0x640a */
	"CHAR",           /* 0x640b */
	"HI COL",         /* 0x640c */
	"INV",            /* 0x640d */
	"FRAC",           /* 0x640e */
	"MOVE",           /* 0x640f */
	"PLACE",          /* 0x6410 */
	"UPB",            /* 0x6411 */
	"UPW",            /* 0x6412 */
	"LEFTW",          /* 0x6413 */
	"LEFTB",          /* 0x6414 */
	"DOWNB",          /* 0x6415 */
	"DOWNW",          /* 0x6416 */
	"RIGHTB",         /* 0x6417 */
	"RIGHTW",         /* 0x6418 */
	"MULTI",          /* 0x6419 */
	"COLOUR",         /* 0x641a */
	"MMOB",           /* 0x641b */
	"BFLASH",         /* 0x641c */
	"MOB SET",        /* 0x641d */
	"MUSIC",          /* 0x641e */
	"FLASH",          /* 0x641f */
	"REPEAT",         /* 0x6420 */
	"PLAY",           /* 0x6421 */
	">>",             /* 0x6422 */
	"CENTRE",         /* 0x6423 */
	"ENVELOPE",       /* 0x6424 */
	"CGOTO",          /* 0x6425 */
	"WAVE",           /* 0x6426 */
	"FETCH",          /* 0x6427 */
	"AT(",            /* 0x6428 */
	"UNTIL",          /* 0x6429 */
	">>",             /* 0x642a */
	">>",             /* 0x642b */
	"USE",            /* 0x642c */
	">>",             /* 0x642d */
	"GLOBAL",         /* 0x642e */
	">>",             /* 0x642f */
	"RESET",          /* 0x6430 */
	"PROC",           /* 0x6431 */
	"CALL",           /* 0x6432 */
	"EXEC",           /* 0x6433 */
	"END PROC",       /* 0x6434 */
	"EXIT",           /* 0x6435 */
	"END LOOP",       /* 0x6436 */
	"ON KEY",         /* 0x6437 */
	"DISABLE",        /* 0x6438 */
	"RESUME",         /* 0x6439 */
	"LOOP",           /* 0x643a */
	"DELAY",          /* 0x643b */
	">>",             /* 0x643c */
	">>",             /* 0x643d */
	">>",             /* 0x643e */
	">>",             /* 0x643f */
	"SECURE",         /* 0x6440 */
	"DISAPA",         /* 0x6441 */
	"CIRCLE",         /* 0x6442 */
	"ON ERROR",       /* 0x6443 */
	"NO ERROR",       /* 0x6444 */
	"LOCAL",          /* 0x6445 */
	"RCOMP",          /* 0x6446 */
	"ELSE",           /* 0x6447 */
	"RETRACE",        /* 0x6448 */
	"TRACE",          /* 0x6449 */
	"DIR",            /* 0x644a */
	"PAGE",           /* 0x644b */
	"DUMP",           /* 0x644c */
	"FIND",           /* 0x644d */
	"OPTION",         /* 0x644e */
	"AUTO",           /* 0x644f */
	"OLD",            /* 0x6450 */
	"JOY",            /* 0x6451 */
	"MOD",            /* 0x6452 */
	"DIV",            /* 0x6453 */
	">>",             /* 0x6454 */
	"DUP",            /* 0x6455 */
	"INKEY",          /* 0x6456 */
	"INST",           /* 0x6457 */
	"TEST",           /* 0x6458 */
	"LIN",            /* 0x6459 */
	"EXOR",           /* 0x645a */
	"INSERT",         /* 0x645b */
	"POT",            /* 0x645c */
	"PENX",           /* 0x645d */
	">>",             /* 0x645e */
	"PENY",           /* 0x645f */
	"SOUND",          /* 0x6460 */
	"GRAPHICS",       /* 0x6461 */
	"DESIGN",         /* 0x6462 */
	"RLOCMOB",        /* 0x6463 */
	"CMOB",           /* 0x6464 */
	"BCKGNDS",        /* 0x6465 */
	"PAUSE",          /* 0x6466 */
	"NRM",            /* 0x6467 */
	"MOB OFF",        /* 0x6468 */
	"OFF",            /* 0x6469 */
	"ANGL",           /* 0x646a */
	"ARC",            /* 0x646b */
	"COLD",           /* 0x646c */
	"SCRSV",          /* 0x646d */
	"SCRLD",          /* 0x646e */
	"TEXT",           /* 0x646f */
	"CSET",           /* 0x6470 */
	"VOL",            /* 0x6471 */
	"DISK",           /* 0x6472 */
	"HRDCPY",         /* 0x6473 */
	"KEY",            /* 0x6474 */
	"PAINT",          /* 0x6475 */
	"LOW COL",        /* 0x6476 */
	"COPY",           /* 0x6477 */
	"MERGE",          /* 0x6478 */
	"RENUMBER",       /* 0x6479 */
	"MEM",            /* 0x647a */
	"DETECT",         /* 0x647b */
	"CHECK",          /* 0x647c */
	"DISPLAY",        /* 0x647d */
	"ERR",            /* 0x647e */
	"OUT"             /* 0x647f */
};

static const char *const basic_20_exp_40[] = /* "BASIC 2.0 with BASIC 4.0 Expansion" - supported by c64 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"CONCAT",         /* 0xcc */
	"DOPEN",          /* 0xcd */
	"DCLOSE",         /* 0xce */
	"RECORD",         /* 0xcf */
	"HEADER",         /* 0xd0 */
	"COLLECT",        /* 0xd1 */
	"BACKUP",         /* 0xd2 */
	"COPY",           /* 0xd3 */
	"APPEND",         /* 0xd4 */
	"DSAVE",          /* 0xd5 */
	"DLOAD",          /* 0xd6 */
	"CATALOG",        /* 0xd7 */
	"RENAME",         /* 0xd8 */
	"SCRATCH",        /* 0xd9 */
	"DIRECTORY",      /* 0xda */
	"COLOR",          /* 0xdb */
	"COLD",           /* 0xdc */
	"KEY",            /* 0xdd */
	"DVERIFY",        /* 0xde */
	"DELETE"          /* 0xdf */
	"AUTO",           /* 0xe0 */
	"MERGE",          /* 0xe1 */
	"OLD",            /* 0xe2 */
	"MONITOR",        /* 0xe3 */
	NULL,               /* 0xe4 */
	NULL,               /* 0xe5 */
	NULL,               /* 0xe6 */
	NULL,               /* 0xe7 */
	NULL,               /* 0xe8 */
	NULL,               /* 0xe9 */
	NULL,               /* 0xea */
	NULL,               /* 0xeb */
	NULL,               /* 0xec */
	NULL,               /* 0xed */
	NULL,               /* 0xee */
	NULL,               /* 0xef */
	NULL,               /* 0xf0 */
	NULL,               /* 0xf1 */
	NULL,               /* 0xf2 */
	NULL,               /* 0xf3 */
	NULL,               /* 0xf4 */
	NULL,               /* 0xf5 */
	NULL,               /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_35[] = /* "BASIC 3.5" - supported by c16 & clones, except c364 */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"RGR",            /* 0xcc */
	"RCLR"            /* 0xcd */
	"RLUM"            /* 0xce */
	"JOY",            /* 0xcf */
	"RDOT"            /* 0xd0 */
	"DEC",            /* 0xd1 */
	"HEX$",           /* 0xd2 */
	"ERR$",           /* 0xd3 */
	"INSTR",          /* 0xd4 */
	"ELSE",           /* 0xd5 */
	"RESUME",         /* 0xd6 */
	"TRAP",           /* 0xd7 */
	"TRON",           /* 0xd8 */
	"TROFF",          /* 0xd9 */
	"SOUND",          /* 0xda */
	"VOL",            /* 0xdb */
	"AUTO",           /* 0xdc */
	"PUDEF",          /* 0xdd */
	"GRAPHIC",        /* 0xde */
	"PAINT",          /* 0xdf */
	"CHAR",           /* 0xe0 */
	"BOX",            /* 0xe1 */
	"CIRCLE",         /* 0xe2 */
	"GSHAPE",         /* 0xe3 */
	"SSHAPE",         /* 0xe4 */
	"DRAW",           /* 0xe5 */
	"LOCATE",         /* 0xe6 */
	"COLOR",          /* 0xe7 */
	"SCNCLR",         /* 0xe8 */
	"SCALE",          /* 0xe9 */
	"HELP",           /* 0xea */
	"DO",             /* 0xeb */
	"LOOP",           /* 0xec */
	"EXIT",           /* 0xed */
	"DIRECTORY",      /* 0xee */
	"DSAVE",          /* 0xef */
	"DLOAD",          /* 0xf0 */
	"HEADER",         /* 0xf1 */
	"SCRATCH",        /* 0xf2 */
	"COLLECT",        /* 0xf3 */
	"COPY",           /* 0xf4 */
	"RENAME",         /* 0xf5 */
	"BACKUP",         /* 0xf6 */
	"DELETE",         /* 0xf7 */
	"RENUMBER",       /* 0xf8 */
	"KEY",            /* 0xf9 */
	"MONITOR",        /* 0xfa */
	"USING",          /* 0xfb */
	"UNTIL",          /* 0xfc */
	"WHILE",          /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_35_magic_voice[] = /* "BASIC 3.5 with Magic Voice Speech Synthesizer" - supported by c364 */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"RGR",            /* 0xcc */
	"RCLR"            /* 0xcd */
	"RLUM"            /* 0xce */
	"JOY",            /* 0xcf */
	"RDOT"            /* 0xd0 */
	"DEC",            /* 0xd1 */
	"HEX$",           /* 0xd2 */
	"ERR$",           /* 0xd3 */
	"INSTR",          /* 0xd4 */
	"ELSE",           /* 0xd5 */
	"RESUME",         /* 0xd6 */
	"TRAP",           /* 0xd7 */
	"TRON",           /* 0xd8 */
	"TROFF",          /* 0xd9 */
	"SOUND",          /* 0xda */
	"VOL",            /* 0xdb */
	"AUTO",           /* 0xdc */
	"PUDEF",          /* 0xdd */
	"GRAPHIC",        /* 0xde */
	"PAINT",          /* 0xdf */
	"CHAR",           /* 0xe0 */
	"BOX",            /* 0xe1 */
	"CIRCLE",         /* 0xe2 */
	"GSHAPE",         /* 0xe3 */
	"SSHAPE",         /* 0xe4 */
	"DRAW",           /* 0xe5 */
	"LOCATE",         /* 0xe6 */
	"COLOR",          /* 0xe7 */
	"SCNCLR",         /* 0xe8 */
	"SCALE",          /* 0xe9 */
	"HELP",           /* 0xea */
	"DO",             /* 0xeb */
	"LOOP",           /* 0xec */
	"EXIT",           /* 0xed */
	"DIRECTORY",      /* 0xee */
	"DSAVE",          /* 0xef */
	"DLOAD",          /* 0xf0 */
	"HEADER",         /* 0xf1 */
	"SCRATCH",        /* 0xf2 */
	"COLLECT",        /* 0xf3 */
	"COPY",           /* 0xf4 */
	"RENAME",         /* 0xf5 */
	"BACKUP",         /* 0xf6 */
	"DELETE",         /* 0xf7 */
	"RENUMBER",       /* 0xf8 */
	"KEY",            /* 0xf9 */
	"MONITOR",        /* 0xfa */
	"USING",          /* 0xfb */
	"UNTIL",          /* 0xfc */
	"WHILE",          /* 0xfd */
	NULL,               /* 0xfe - Prefix for additional tokens */
	"{PI}",           /* 0xff - A single character shaped as greek lowercase 'PI' */
	NULL,               /* 0xfe00 */
	"RATE",           /* 0xfe01 */
	"VOC",            /* 0xfe02 */
	NULL,               /* 0xfe03 */
	"RDY",            /* 0xfe04 */
	NULL,               /* 0xfe05 */
	NULL,               /* 0xfe06 */
	NULL,               /* 0xfe07 */
	NULL,               /* 0xfe08 */
	NULL,               /* 0xfe09 */
	"SAY"             /* 0xfe0a */
};

static const char *const basic_40[] = /* "BASIC 4.0" - supported by cbm40xx & cbm80xx series, p500, cbm600 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"CONCAT",         /* 0xcc */
	"DOPEN",          /* 0xcd */
	"DCLOSE",         /* 0xce */
	"RECORD",         /* 0xcf */
	"HEADER",         /* 0xd0 */
	"COLLECT",        /* 0xd1 */
	"BACKUP",         /* 0xd2 */
	"COPY",           /* 0xd3 */
	"APPEND",         /* 0xd4 */
	"DSAVE",          /* 0xd5 */
	"DLOAD",          /* 0xd6 */
	"CATALOG",        /* 0xd7 */
	"RENAME",         /* 0xd8 */
	"SCRATCH",        /* 0xd9 */
	"DIRECTORY",      /* 0xda */
	NULL,               /* 0xdb */
	NULL,               /* 0xdc */
	NULL,               /* 0xdd */
	NULL,               /* 0xde */
	NULL,               /* 0xdf */
	NULL,               /* 0xe0 */
	NULL,               /* 0xe1 */
	NULL,               /* 0xe2 */
	NULL,               /* 0xe3 */
	NULL,               /* 0xe4 */
	NULL,               /* 0xe5 */
	NULL,               /* 0xe6 */
	NULL,               /* 0xe7 */
	NULL,               /* 0xe8 */
	NULL,               /* 0xe9 */
	NULL,               /* 0xea */
	NULL,               /* 0xeb */
	NULL,               /* 0xec */
	NULL,               /* 0xed */
	NULL,               /* 0xee */
	NULL,               /* 0xef */
	NULL,               /* 0xf0 */
	NULL,               /* 0xf1 */
	NULL,               /* 0xf2 */
	NULL,               /* 0xf3 */
	NULL,               /* 0xf4 */
	NULL,               /* 0xf5 */
	NULL,               /* 0xf6 */
	NULL,               /* 0xf7 */
	NULL,               /* 0xf8 */
	NULL,               /* 0xf9 */
	NULL,               /* 0xfa */
	NULL,               /* 0xfb */
	NULL,               /* 0xfc */
	NULL,               /* 0xfd */
	NULL,               /* 0xfe */
	"{PI}"            /* 0xff - A single character shaped as greek lowercase 'PI' */
};

static const char *const basic_70[] = /* "BASIC 7.0" - supported by c128 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"RGR",            /* 0xcc */
	"RCLR",           /* 0xcd */
	NULL,               /* 0xce - Prefix for additional tokens */
	"JOY",            /* 0xcf */
	"RDOT"            /* 0xd0 */
	"DEC",            /* 0xd1 */
	"HEX$",           /* 0xd2 */
	"ERR$",           /* 0xd3 */
	"INSTR",          /* 0xd4 */
	"ELSE",           /* 0xd5 */
	"RESUME",         /* 0xd6 */
	"TRAP",           /* 0xd7 */
	"TRON",           /* 0xd8 */
	"TROFF",          /* 0xd9 */
	"SOUND",          /* 0xda */
	"VOL",            /* 0xdb */
	"AUTO",           /* 0xdc */
	"PUDEF",          /* 0xdd */
	"GRAPHIC",        /* 0xde */
	"PAINT",          /* 0xdf */
	"CHAR",           /* 0xe0 */
	"BOX",            /* 0xe1 */
	"CIRCLE",         /* 0xe2 */
	"GSHAPE",         /* 0xe3 */
	"SSHAPE",         /* 0xe4 */
	"DRAW",           /* 0xe5 */
	"LOCATE",         /* 0xe6 */
	"COLOR",          /* 0xe7 */
	"SCNCLR",         /* 0xe8 */
	"SCALE",          /* 0xe9 */
	"HELP",           /* 0xea */
	"DO",             /* 0xeb */
	"LOOP",           /* 0xec */
	"EXIT",           /* 0xed */
	"DIRECTORY",      /* 0xee */
	"DSAVE",          /* 0xef */
	"DLOAD",          /* 0xf0 */
	"HEADER",         /* 0xf1 */
	"SCRATCH",        /* 0xf2 */
	"COLLECT",        /* 0xf3 */
	"COPY",           /* 0xf4 */
	"RENAME",         /* 0xf5 */
	"BACKUP",         /* 0xf6 */
	"DELETE",         /* 0xf7 */
	"RENUMBER",       /* 0xf8 */
	"KEY",            /* 0xf9 */
	"MONITOR",        /* 0xfa */
	"USING",          /* 0xfb */
	"UNTIL",          /* 0xfc */
	"WHILE",          /* 0xfd */
	NULL,               /* 0xfe - Prefix for additional tokens */
	"{PI}",           /* 0xff - A single character shaped as greek lowercase 'PI' */
	NULL,               /* 0xce00 */
	NULL,               /* 0xce01 */
	"POT",            /* 0xce02 */
	"BUMP",           /* 0xce03 */
	"PEN",            /* 0xce04 */
	"RSPPOS",         /* 0xce05 */
	"RSPRITE",        /* 0xce06 */
	"RSPCOLOR",       /* 0xce07 */
	"XOR",            /* 0xce08 */
	"RWINDOW",        /* 0xce09 */
	"POINTER",        /* 0xce0a */
	NULL,               /* 0xfe00 */
	NULL,               /* 0xfe01 */
	"BANK",           /* 0xfe02 */
	"FILTER",         /* 0xfe03 */
	"PLAY",           /* 0xfe04 */
	"TEMPO",          /* 0xfe05 */
	"MOVSPR",         /* 0xfe06 */
	"SPRITE",         /* 0xfe07 */
	"SPRCOLOR",       /* 0xfe08 */
	"RREG",           /* 0xfe09 */
	"ENVELOPE",       /* 0xfe0a */
	"SLEEP",          /* 0xfe0b */
	"CATALOG",        /* 0xfe0c */
	"DOPEN",          /* 0xfe0d */
	"APPEND",         /* 0xfe0e */
	"DCLOSE",         /* 0xfe0f */
	"BSAVE",          /* 0xfe10 */
	"BLOAD",          /* 0xfe11 */
	"RECORD",         /* 0xfe12 */
	"CONCAT",         /* 0xfe13 */
	"DVERIFY",        /* 0xfe14 */
	"DCLEAR",         /* 0xfe15 */
	"SPRSAV",         /* 0xfe16 */
	"COLLISION",      /* 0xfe17 */
	"BEGIN",          /* 0xfe18 */
	"BEND",           /* 0xfe19 */
	"WINDOW",         /* 0xfe1a */
	"BOOT",           /* 0xfe1b */
	"WIDTH",          /* 0xfe1c */
	"SPRDEF",         /* 0xfe1d */
	"QUIT",           /* 0xfe1e */
	"STASH",          /* 0xfe1f */
	NULL,               /* 0xfe20 */
	"FETCH",          /* 0xfe21 */
	NULL,               /* 0xfe22 */
	"SWAP",           /* 0xfe23 */
	"OFF",            /* 0xfe24 */
	"FAST",           /* 0xfe25 */
	"SLOW"            /* 0xfe26 */
};

static const char *const basic_100[] = /* "BASIC 10.0" - supported by c65 & clones */
{
	"END",            /* 0x80 */
	"FOR",            /* 0x81 */
	"NEXT",           /* 0x82 */
	"DATA",           /* 0x83 */
	"INPUT#",         /* 0x84 */
	"INPUT",          /* 0x85 */
	"DIM",            /* 0x86 */
	"READ",           /* 0x87 */
	"LET",            /* 0x88 */
	"GOTO",           /* 0x89 */
	"RUN",            /* 0x8a */
	"IF",             /* 0x8b */
	"RESTORE",        /* 0x8c */
	"GOSUB",          /* 0x8d */
	"RETURN",         /* 0x8e */
	"REM",            /* 0x8f */
	"STOP",           /* 0x90 */
	"ON",             /* 0x91 */
	"WAIT",           /* 0x92 */
	"LOAD",           /* 0x93 */
	"SAVE",           /* 0x94 */
	"VERIFY",         /* 0x95 */
	"DEF",            /* 0x96 */
	"POKE",           /* 0x97 */
	"PRINT#",         /* 0x98 */
	"PRINT",          /* 0x99 */
	"CONT",           /* 0x9a */
	"LIST",           /* 0x9b */
	"CLR",            /* 0x9c */
	"CMD",            /* 0x9d */
	"SYS",            /* 0x9e */
	"OPEN",           /* 0x9f */
	"CLOSE",          /* 0xa0 */
	"GET",            /* 0xa1 */
	"NEW",            /* 0xa2 */
	"TAB(",           /* 0xa3 */
	"TO",             /* 0xa4 */
	"FN",             /* 0xa5 */
	"SPC(",           /* 0xa6 */
	"THEN",           /* 0xa7 */
	"NOT",            /* 0xa8 */
	"STEP",           /* 0xa9 */
	"+",              /* 0xaa */
	"-",              /* 0xab */
	"*",              /* 0xac */
	"/",              /* 0xad */
	"^",              /* 0xae */
	"AND",            /* 0xaf */
	"OR",             /* 0xb0 */
	">",              /* 0xb1 */
	"=",              /* 0xb2 */
	"<",              /* 0xb3 */
	"SGN",            /* 0xb4 */
	"INT",            /* 0xb5 */
	"ABS",            /* 0xb6 */
	"USR",            /* 0xb7 */
	"FRE",            /* 0xb8 */
	"POS",            /* 0xb9 */
	"SQR",            /* 0xba */
	"RND",            /* 0xbb */
	"LOG",            /* 0xbc */
	"EXP",            /* 0xbd */
	"COS",            /* 0xbe */
	"SIN",            /* 0xbf */
	"TAN",            /* 0xc0 */
	"ATN",            /* 0xc1 */
	"PEEK",           /* 0xc2 */
	"LEN",            /* 0xc3 */
	"STR$",           /* 0xc4 */
	"VAL",            /* 0xc5 */
	"ASC",            /* 0xc6 */
	"CHR$",           /* 0xc7 */
	"LEFT$",          /* 0xc8 */
	"RIGHT$",         /* 0xc9 */
	"MID$",           /* 0xca */
	"GO",             /* 0xcb */
	"RGR",            /* 0xcc */
	"RCLR",           /* 0xcd */
	NULL,             /* 0xce - Prefix for additional tokens */
	"JOY",            /* 0xcf */
	"RDOT"            /* 0xd0 */
	"DEC",            /* 0xd1 */
	"HEX$",           /* 0xd2 */
	"ERR$",           /* 0xd3 */
	"INSTR",          /* 0xd4 */
	"ELSE",           /* 0xd5 */
	"RESUME",         /* 0xd6 */
	"TRAP",           /* 0xd7 */
	"TRON",           /* 0xd8 */
	"TROFF",          /* 0xd9 */
	"SOUND",          /* 0xda */
	"VOL",            /* 0xdb */
	"AUTO",           /* 0xdc */
	"PUDEF",          /* 0xdd */
	"GRAPHIC",        /* 0xde */
	"PAINT",          /* 0xdf */
	"CHAR",           /* 0xe0 */
	"BOX",            /* 0xe1 */
	"CIRCLE",         /* 0xe2 */
	"PASTE",          /* 0xe3 */
	"CUT",            /* 0xe4 */
	"LINE",           /* 0xe5 */
	"LOCATE",         /* 0xe6 */
	"COLOR",          /* 0xe7 */
	"SCNCLR",         /* 0xe8 */
	"SCALE",          /* 0xe9 */
	"HELP",           /* 0xea */
	"DO",             /* 0xeb */
	"LOOP",           /* 0xec */
	"EXIT",           /* 0xed */
	"DIR",            /* 0xee */
	"DSAVE",          /* 0xef */
	"DLOAD",          /* 0xf0 */
	"HEADER",         /* 0xf1 */
	"SCRATCH",        /* 0xf2 */
	"COLLECT",        /* 0xf3 */
	"COPY",           /* 0xf4 */
	"RENAME",         /* 0xf5 */
	"BACKUP",         /* 0xf6 */
	"DELETE",         /* 0xf7 */
	"RENUMBER",       /* 0xf8 */
	"KEY",            /* 0xf9 */
	"MONITOR",        /* 0xfa */
	"USING",          /* 0xfb */
	"UNTIL",          /* 0xfc */
	"WHILE",          /* 0xfd */
	NULL,               /* 0xfe - Prefix for additional tokens  */
	"{PI}",           /* 0xff - A single character shaped as greek lowercase 'PI' */
	NULL,               /* 0xce00 */
	NULL,               /* 0xce01 */
	"POT",            /* 0xce02 */
	"BUMP",           /* 0xce03 */
	"PEN",            /* 0xce04 */
	"RSPPOS",         /* 0xce05 */
	"RSPRITE",        /* 0xce06 */
	"RSPCOLOR",       /* 0xce07 */
	"XOR",            /* 0xce08 */
	"RWINDOW",        /* 0xce09 */
	"POINTER",        /* 0xce0a */
	NULL,               /* 0xfe00 */
	NULL,               /* 0xfe01 */
	"BANK",           /* 0xfe02 */
	"FILTER",         /* 0xfe03 */
	"PLAY",           /* 0xfe04 */
	"TEMPO",          /* 0xfe05 */
	"MOVSPR",         /* 0xfe06 */
	"SPRITE",         /* 0xfe07 */
	"SPRCOLOR",       /* 0xfe08 */
	"RREG",           /* 0xfe09 */
	"ENVELOPE",       /* 0xfe0a */
	"SLEEP",          /* 0xfe0b */
	"CATALOG",        /* 0xfe0c */
	"DOPEN",          /* 0xfe0d */
	"APPEND",         /* 0xfe0e */
	"DCLOSE",         /* 0xfe0f */
	"BSAVE",          /* 0xfe10 */
	"BLOAD",          /* 0xfe11 */
	"RECORD",         /* 0xfe12 */
	"CONCAT",         /* 0xfe13 */
	"DVERIFY",        /* 0xfe14 */
	"DCLEAR",         /* 0xfe15 */
	"SPRSAV",         /* 0xfe16 */
	"COLLISION",      /* 0xfe17 */
	"BEGIN",          /* 0xfe18 */
	"BEND",           /* 0xfe19 */
	"WINDOW",         /* 0xfe1a */
	"BOOT",           /* 0xfe1b */
	"WIDTH",          /* 0xfe1c */
	"SPRDEF",         /* 0xfe1d */
	"QUIT",           /* 0xfe1e */
	"DMA",            /* 0xfe1f */
	NULL,               /* 0xfe20 */
	"DMA",            /* 0xfe21 */
	NULL,               /* 0xfe22 */
	"DMA",            /* 0xfe23 */
	"OFF",            /* 0xfe24 */
	"FAST",           /* 0xfe25 */
	"SLOW",           /* 0xfe26 */
	"TYPE",           /* 0xfe27 */
	"BVERIFY",        /* 0xfe28 */
	"ECTORY",         /* 0xfe29 */
	"ERASE",          /* 0xfe2a */
	"FIND",           /* 0xfe2b */
	"CHANGE",         /* 0xfe2c */
	"SET",            /* 0xfe2d */
	"SCREEN",         /* 0xfe2e */
	"POLYGON",        /* 0xfe2f */
	"ELLIPSE",        /* 0xfe30 */
	"VIEWPORT",       /* 0xfe31 */
	"GCOPY",          /* 0xfe32 */
	"PEN",            /* 0xfe33 */
	"PALETTE",        /* 0xfe34 */
	"DMODE",          /* 0xfe35 */
	"DPAT",           /* 0xfe36 */
	"PIC",            /* 0xfe37 */
	"GENLOCK",        /* 0xfe38 */
	"FOREGROUND",     /* 0xfe39 */
	NULL,               /* 0xfe3a */
	"BACKGROUND",     /* 0xfe3b */
	"BORDER",         /* 0xfe3c */
	"HIGHLIGHT"       /* 0xfe3d */
};

#endif



/***************************************************************************
    COCO BASIC
***************************************************************************/

static const basictoken_tableent cocobas_tokenents[] =
{
	{ 0x00, 0x80,   cocobas_statements, ARRAY_LENGTH(cocobas_statements) },
	{ 0xff, 0x80,   cocobas_functions,  ARRAY_LENGTH(cocobas_functions) }
};

static const basictokens cocobas_tokens =
{
	0x2600,
	1,
	3,
	{0xFF, 0x00, 0x00},
	true,
	cocobas_tokenents,
	ARRAY_LENGTH(cocobas_tokenents)
};

static imgtoolerr_t cocobas_readfile(imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &destf)
{
	return basic_readfile(&cocobas_tokens, partition, filename, fork, destf);
}

static imgtoolerr_t cocobas_writefile(imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	return basic_writefile(&cocobas_tokens, partition, filename, fork, sourcef, opts);
}

void filter_cocobas_getinfo(uint32_t state, union filterinfo *info)
{
	switch(state)
	{
		case FILTINFO_STR_NAME:         info->s = "cocobas"; break;
		case FILTINFO_STR_HUMANNAME:    info->s = "CoCo Tokenized Basic Files"; break;
		case FILTINFO_PTR_READFILE:     info->read_file = cocobas_readfile; break;
		case FILTINFO_PTR_WRITEFILE:    info->write_file = cocobas_writefile; break;
	}
}



/***************************************************************************
    DRAGON BASIC
***************************************************************************/

static const basictoken_tableent dragonbas_tokenents[] =
{
	{ 0x00, 0x80,   dragonbas_statements,   ARRAY_LENGTH(dragonbas_statements) },
	{ 0xff, 0x80,   dragonbas_functions,    ARRAY_LENGTH(dragonbas_functions) }
};

static const basictokens dragonbas_tokens =
{
	0x2415,
	4,
	9,
	{0x55, 0x01, 0x24, 0x01, 0x00, 0x2A, 0x8B, 0x8D, 0xAA},
	true,
	dragonbas_tokenents,
	ARRAY_LENGTH(dragonbas_tokenents)
};

static imgtoolerr_t dragonbas_readfile(imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &destf)
{
	return basic_readfile(&dragonbas_tokens, partition, filename, fork, destf);
}

static imgtoolerr_t dragonbas_writefile(imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	return basic_writefile(&dragonbas_tokens, partition, filename, fork, sourcef, opts);
}

void filter_dragonbas_getinfo(uint32_t state, union filterinfo *info)
{
	switch(state)
	{
		case FILTINFO_STR_NAME:         info->s = "dragonbas"; break;
		case FILTINFO_STR_HUMANNAME:    info->s = "Dragon Tokenized Basic Files"; break;
		case FILTINFO_PTR_READFILE:     info->read_file = dragonbas_readfile; break;
		case FILTINFO_PTR_WRITEFILE:    info->write_file = dragonbas_writefile; break;
	}
}



/***************************************************************************
    VZBASIC
***************************************************************************/

static const basictoken_tableent vzbas_tokenents[] =
{
	{ 0x00, 0x80,   vzbas,  ARRAY_LENGTH(vzbas) }
};



static const basictokens vzbas_tokens =
{
	0x7ae9,
	0,
	0,
	{0x00},
	false,
	vzbas_tokenents,
	ARRAY_LENGTH(vzbas_tokenents)
};

static imgtoolerr_t vzbas_readfile(imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &destf)
{
	return basic_readfile(&vzbas_tokens, partition, filename, fork, destf);
}

static imgtoolerr_t vzbas_writefile(imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	return basic_writefile(&vzbas_tokens, partition, filename, fork, sourcef, opts);
}

void filter_vzbas_getinfo(uint32_t state, union filterinfo *info)
{
	switch(state)
	{
		case FILTINFO_STR_NAME:         info->s = "vzbas"; break;
		case FILTINFO_STR_HUMANNAME:    info->s = "Laser/VZ Tokenized Basic Files"; break;
		case FILTINFO_PTR_READFILE:     info->read_file = vzbas_readfile; break;
		case FILTINFO_PTR_WRITEFILE:    info->write_file = vzbas_writefile; break;
	}
}



/***************************************************************************
    BML3 BASIC
***************************************************************************/

static const basictoken_tableent bml3bas_tokenents[] =
{
	{ 0x00, 0x80,   bml3bas_statements, ARRAY_LENGTH(bml3bas_statements) },
	{ 0xff, 0x80,   bml3bas_functions,  ARRAY_LENGTH(bml3bas_functions) }
};

static const basictokens bml3bas_tokens =
{
	0x2600,
	1,
	3,
	{0xFF, 0x00, 0x00},
	true,
	bml3bas_tokenents,
	ARRAY_LENGTH(bml3bas_tokenents)
};

static imgtoolerr_t bml3bas_readfile(imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &destf)
{
	return basic_readfile(&bml3bas_tokens, partition, filename, fork, destf);
}

static imgtoolerr_t bml3bas_writefile(imgtool::partition &partition, const char *filename,
	const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	return basic_writefile(&bml3bas_tokens, partition, filename, fork, sourcef, opts);
}

void filter_bml3bas_getinfo(uint32_t state, union filterinfo *info)
{
	switch(state)
	{
		case FILTINFO_STR_NAME:         info->s = "bml3bas"; break;
		case FILTINFO_STR_HUMANNAME:    info->s = "Basic Master Level 3 Tokenized Basic Files"; break;
		case FILTINFO_PTR_READFILE:     info->read_file = bml3bas_readfile; break;
		case FILTINFO_PTR_WRITEFILE:    info->write_file = bml3bas_writefile; break;
	}
}
