/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/allocator.h>
#include <bx/commandline.h>
#include <bx/file.h>
#include <bx/string.h>

#include <bx/debug.h>

class Bin2cWriter : public bx::WriterI
{
public:
	Bin2cWriter(bx::AllocatorI* _allocator, const bx::StringView& _name)
		: m_mb(_allocator)
		, m_mw(&m_mb)
		, m_name(_name)
		, m_outputAsCStr(false)
	{
	}

	virtual ~Bin2cWriter()
	{
	}

	virtual int32_t write(const void* _data, int32_t _size, bx::Error* _err) override
	{
		bool asCStr = true;

		const char* data = (const char*)_data;
		for (int32_t ii = 0; ii < _size && asCStr; ++ii)
		{
			char ch = data[ii];

			asCStr &= false
				| bx::isPrint(ch)
				| bx::isSpace(ch)
				;
		}

		m_outputAsCStr = asCStr;

		return bx::write(&m_mw, _data, _size, _err);
	}

	void output(bx::WriterI* _writer)
	{
		if (m_outputAsCStr)
		{
			outputString(_writer);
		}
		else
		{
			outputHex(_writer);
		}
	}

	void outputString(bx::WriterI* _writer)
	{
		const uint8_t* data = (const uint8_t*)m_mb.more(0);
		uint32_t size = uint32_t(bx::seek(&m_mw) );

		bx::Error err;

		bx::write(
			  _writer
			, &err
			, "static const char* %.*s = /* Generated with bin2c. */\n\t\""
			, m_name.getLength()
			, m_name.getPtr()
			);

		if (NULL != data)
		{
			bool escaped = false;

			for (uint32_t ii = 0; ii < size; ++ii)
			{
				const char ch = char(data[ii]);

				if (!escaped)
				{
					switch (ch)
					{
					case '\"': bx::write(_writer, "\\\"",        &err); break;
					case '\n': bx::write(_writer, "\\n\"\n\t\"", &err); break;
					case '\r': bx::write(_writer, "\\r",         &err); break;
					case '\\': escaped = true;                 BX_FALLTHROUGH;
					default:   bx::write(_writer, ch, &err);            break;
					}
				}
				else
				{
					switch (ch)
					{
					case '\n': bx::write(_writer, "\\\"\n\t\"", &err);  break;
					case '\r':                                 BX_FALLTHROUGH;
					case '\t': bx::write(_writer, "\\", &err); BX_FALLTHROUGH;
					default  : bx::write(_writer, ch,   &err);          break;
					}

					escaped = false;
				}
			}
		}

		bx::write(_writer, &err, "\"\n\t;\n");
	}

	void outputHex(bx::WriterI* _writer)
	{
#define HEX_DUMP_WIDTH 16
#define HEX_DUMP_SPACE_WIDTH 96
#define HEX_DUMP_FORMAT "%-" BX_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "." BX_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "s"
		const uint8_t* data = (const uint8_t*)m_mb.more(0);
		uint32_t size = uint32_t(bx::seek(&m_mw) );

		bx::Error err;

		bx::write(
			  _writer
			, &err
			, "static const uint8_t %.*s[%d] = /* Generated with bin2c. */\n{\n"
			, m_name.getLength()
			, m_name.getPtr()
			, size
			);

		if (NULL != data)
		{
			char hex[HEX_DUMP_SPACE_WIDTH+1];
			char ascii[HEX_DUMP_WIDTH+1];
			uint32_t hexPos = 0;
			uint32_t asciiPos = 0;
			for (uint32_t ii = 0; ii < size; ++ii)
			{
				bx::snprintf(&hex[hexPos], sizeof(hex)-hexPos, "0x%02x, ", data[asciiPos]);
				hexPos += 6;

				ascii[asciiPos] = bx::isPrint(data[asciiPos]) && data[asciiPos] != '\\' ? data[asciiPos] : '.';
				asciiPos++;

				if (HEX_DUMP_WIDTH == asciiPos)
				{
					ascii[asciiPos] = '\0';
					bx::write(_writer, &err, "\t" HEX_DUMP_FORMAT "// %s\n", hex, ascii);
					data += asciiPos;
					hexPos   = 0;
					asciiPos = 0;
				}
			}

			if (0 != asciiPos)
			{
				ascii[asciiPos] = '\0';
				bx::write(_writer, &err, "\t" HEX_DUMP_FORMAT "// %s\n", hex, ascii);
			}
		}

		bx::write(_writer, &err, "};\n");
#undef HEX_DUMP_WIDTH
#undef HEX_DUMP_SPACE_WIDTH
#undef HEX_DUMP_FORMAT
	}

	bx::MemoryBlock  m_mb;
	bx::MemoryWriter m_mw;
	bx::StringView   m_name;
	bool             m_outputAsCStr;
};

void error(const char* _format, ...)
{
	bx::WriterI* stdOut = bx::getStdOut();
	bx::Error err;

	va_list argList;
	va_start(argList, _format);
	bx::write(stdOut, &err, "Error:\n");
	bx::write(stdOut, _format, argList, &err);
	bx::write(stdOut, &err, "\n\n");
	va_end(argList);
}

void help(const char* _error = NULL)
{
	bx::WriterI* stdOut = bx::getStdOut();
	bx::Error err;

	if (NULL != _error)
	{
		error(_error);
	}

	bx::write(stdOut, &err
		, "bin2c, binary to C\n"
		  "Copyright 2011-2021 Branimir Karadzic. All rights reserved.\n"
		  "License: https://github.com/bkaradzic/bx#license-bsd-2-clause\n\n"
		);

	bx::write(stdOut, &err
		, "Usage: bin2c -f <in> -o <out> -n <name>\n"
		  "\n"
		  "Options:\n"
		  "  -f <file path>    Input file path.\n"
		  "  -o <file path>    Output file path.\n"
		  "  -n <name>         Array name.\n"
		  "\n"
		  "For additional information, see https://github.com/bkaradzic/bx\n"
		);
}

int main(int _argc, const char* _argv[])
{
	bx::CommandLine cmdLine(_argc, _argv);

	if (cmdLine.hasArg('h', "help") )
	{
		help();
		return bx::kExitFailure;
	}

	bx::FilePath filePath = cmdLine.findOption('f');
	if (filePath.isEmpty() )
	{
		help("Input file name must be specified.");
		return bx::kExitFailure;
	}

	bx::FilePath outFilePath = cmdLine.findOption('o');
	if (outFilePath.isEmpty() )
	{
		help("Output file name must be specified.");
		return bx::kExitFailure;
	}

	bx::StringView name = cmdLine.findOption('n');
	if (name.isEmpty() )
	{
		name.set("data");
	}

	void* data = NULL;
	uint32_t size = 0;

	bx::FileReader fr;
	if (bx::open(&fr, filePath) )
	{
		size = uint32_t(bx::getSize(&fr) );

		bx::DefaultAllocator allocator;
		data = BX_ALLOC(&allocator, size);
		bx::read(&fr, data, size);
		bx::close(&fr);

		bx::FileWriter fw;
		if (bx::open(&fw, outFilePath) )
		{
			Bin2cWriter writer(&allocator, name);
			bx::write(&writer, data, size);

			writer.output(&fw);
			bx::close(&fw);
		}
		else
		{
			bx::StringView path = outFilePath;
			error("Failed to open output file '%.*s'.\n", path.getLength(), path.getPtr() );
		}

		BX_FREE(&allocator, data);
	}
	else
	{
		bx::StringView path = filePath;
		error("Failed to open input file '%.*s'.\n", path.getLength(), path.getPtr() );
	}

	return 0;
}
