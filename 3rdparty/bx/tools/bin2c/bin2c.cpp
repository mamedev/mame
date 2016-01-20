/*
 * Copyright 2011-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <string>
#include <vector>

#include <bx/commandline.h>
#include <bx/readerwriter.h>
#include <bx/string.h>

class Bin2cWriter : public bx::WriterI
{
public:
	Bin2cWriter(bx::WriterI* _writer, const char* _name)
		: m_writer(_writer)
		, m_name(_name)
	{
	}

	virtual ~Bin2cWriter()
	{
	}

	virtual int32_t write(const void* _data, int32_t _size) BX_OVERRIDE
	{
		const char* data = (const char*)_data;
		m_buffer.insert(m_buffer.end(), data, data+_size);
		return _size;
	}

	void finish()
	{
#define HEX_DUMP_WIDTH 16
#define HEX_DUMP_SPACE_WIDTH 96
#define HEX_DUMP_FORMAT "%-" BX_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "." BX_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "s"
		const uint8_t* data = &m_buffer[0];
		uint32_t size = (uint32_t)m_buffer.size();

		bx::writePrintf(m_writer, "static const uint8_t %s[%d] =\n{\n", m_name.c_str(), size);

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

				ascii[asciiPos] = isprint(data[asciiPos]) && data[asciiPos] != '\\' ? data[asciiPos] : '.';
				asciiPos++;

				if (HEX_DUMP_WIDTH == asciiPos)
				{
					ascii[asciiPos] = '\0';
					bx::writePrintf(m_writer, "\t" HEX_DUMP_FORMAT "// %s\n", hex, ascii);
					data += asciiPos;
					hexPos = 0;
					asciiPos = 0;
				}
			}

			if (0 != asciiPos)
			{
				ascii[asciiPos] = '\0';
				bx::writePrintf(m_writer, "\t" HEX_DUMP_FORMAT "// %s\n", hex, ascii);
			}
		}

		bx::writePrintf(m_writer, "};\n");
#undef HEX_DUMP_WIDTH
#undef HEX_DUMP_SPACE_WIDTH
#undef HEX_DUMP_FORMAT

		m_buffer.clear();
	}

	bx::WriterI* m_writer;
	std::string m_filePath;
	std::string m_name;
	typedef std::vector<uint8_t> Buffer;
	Buffer m_buffer;
};

void help(const char* _error = NULL)
{
	if (NULL != _error)
	{
		fprintf(stderr, "Error:\n%s\n\n", _error);
	}

	fprintf(stderr
		, "bin2c, binary to C\n"
		  "Copyright 2011-2016 Branimir Karadzic. All rights reserved.\n"
		  "License: https://github.com/bkaradzic/bx#license-bsd-2-clause\n\n"
		);

	fprintf(stderr
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
		return EXIT_FAILURE;
	}

	const char* filePath = cmdLine.findOption('f');
	if (NULL == filePath)
	{
		help("Input file name must be specified.");
		return EXIT_FAILURE;
	}

	const char* outFilePath = cmdLine.findOption('o');
	if (NULL == outFilePath)
	{
		help("Output file name must be specified.");
		return EXIT_FAILURE;
	}

	const char* name = cmdLine.findOption('n');
	if (NULL == name)
	{
		name = "data";
	}

	void* data = NULL;
	size_t size = 0;

	bx::CrtFileReader fr;
	if (0 == bx::open(&fr, filePath) )
	{
		size = (size_t)bx::getSize(&fr);
		data = malloc(size);
		bx::read(&fr, data, size);

		bx::CrtFileWriter fw;
		if (0 == bx::open(&fw, outFilePath) )
		{
			Bin2cWriter writer(&fw, name);
			bx::write(&writer, data, size);
			writer.finish();
			bx::close(&fw);
		}

		free(data);
	}

	return 0;
}
