// license:BSD-3-Clause
// copyright-holders:AJR

#include "coretmpl.h"
#include "endianness.h"
#include "unzip.h"

#include "osdfile.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <system_error>
#include <vector>


#define LOG_BITSWAP (0)

struct option_info
{
	char shortname;
	const char *longname;
};

enum
{
	OPTION_INTERLEAVE,
	OPTION_DATA_BITSWAP,
	OPTION_ARCHIVE_FILE,
	OPTION_START_OFFSET,
	OPTION_WORD_COUNT,
	OPTION_TRAILING_FILL,
	OPTION_WORD_BYTES,
	OPTION_HELP,
	MAX_OPTIONS
};

constexpr int MAX_SOURCE_BYTES = 32;
constexpr int MAX_SWAP_BITS = 256;
constexpr int MAX_WORD_BYTES = MAX_SWAP_BITS / 8;

static const option_info s_options[MAX_OPTIONS] =
{
	{ 'i', "interleave" },
	{ 'b', "data-bitswap" },
	{ 'z', "archive-file" },
	{ 's', "start-offset" },
	{ 'c', "word-count" },
	{ 'F', "trailing-fill" },
	{ 'w', "word-bytes" },
	{ 'h', "help" }
};

struct interleave_info
{
	int srccount = 1;
	int groupbits = 8;
	int groupbytes = 1;
	util::endianness endian = util::endianness::native;
	bool endian_specified = false;
	std::uint8_t trailing_fill = 0;

	bool parse(const char *optarg);
	void read_sources(const std::vector<std::vector<std::uint8_t>> &buffers, std::size_t srcindex, std::uint8_t *dest) const;
};

struct bitswap_info
{
	struct field_specifier
	{
		std::uint8_t pos;
		std::uint8_t len;
		std::uint8_t shift;
	};

	field_specifier bitspec[MAX_SWAP_BITS];
	int bitcount = 0;
	int swapcount = 0;
	std::uint64_t xorval[MAX_WORD_BYTES / 8]{};
	bool unswapped = false;

	bool parse(const char *optarg);
	void explode(int bits, util::endianness endian);

	bool has_xor() const noexcept { for (std::uint64_t val : xorval) { if (val != 0) return true; } return false; }
	inline void add_xor(unsigned pos, unsigned width) noexcept;

	// For swapping 64 bits or less
	std::uint64_t swap(const std::uint8_t *inbuf) const noexcept
	{
		std::uint64_t result = 0;
		for (int n = 0; n < swapcount; n++)
			result |= std::uint64_t(util::BIT(inbuf[bitspec[n].pos >> 3], bitspec[n].pos & 7, bitspec[n].len + 1)) << bitspec[n].shift;
		return result ^ xorval[0];
	}

	// For swapping more than 64 bits
	template <bool Reverse>
	void swap_long(const std::uint8_t *inbuf, std::uint64_t *outbuf, unsigned dstbuflen) const noexcept
	{
		std::fill_n(outbuf, dstbuflen, 0);
		for (int n = 0; n < swapcount; n++)
			outbuf[Reverse ? dstbuflen - 1 - (bitspec[n].shift >> 6) : bitspec[n].shift >> 6] |=
				std::uint64_t(util::BIT(inbuf[bitspec[n].pos >> 3], bitspec[n].pos & 7, bitspec[n].len + 1)) << (bitspec[n].shift & 63);
		if (Reverse)
		{
			for (int i = dstbuflen; i-- > 0; )
				outbuf[i] = swapendian_int64(outbuf[i] ^ xorval[dstbuflen - 1 - i]);
		}
		else
		{
			for (int i = 0; i < dstbuflen; i++)
				outbuf[i] ^= xorval[i];
		}
	}
};

struct output_info
{
	int word_bytes = 0;
	osd_file::ptr file;
	std::uint64_t writeoffs = 0;

	bool write(const std::uint8_t *bytebuf, const char *argv0);
};

bool interleave_info::parse(const char *optarg)
{
	srccount = 0;
	endian_specified = false;

	int ch = 0;
	while (optarg[ch] >= '0' && optarg[ch] <= '9')
		srccount = srccount * 10 + (optarg[ch++] - '0');
	if (optarg[ch] == '\0' && (srccount & (srccount - 1)) == 0)
	{
		groupbits = 8;
		groupbytes = 1;
		return true;
	}
	if (std::tolower(optarg[ch++]) != 'x')
		return false;
	groupbits = 0;
	while (optarg[ch] >= '0' && optarg[ch] <= '9')
		groupbits = groupbits * 10 + (optarg[ch++] - '0');
	groupbytes = (groupbits + 7) / 8;
	char endianch = std::tolower(optarg[ch]);
	if ((endianch == 'b' || endianch == 'l') && (std::tolower(optarg[ch + 1]) == 'e'))
	{
		endian = endianch == 'b' ? util::endianness::big : util::endianness::little;
		endian_specified = true;
		ch += 2;
	}
	return optarg[ch] == '\0';
}

void interleave_info::read_sources(const std::vector<std::vector<std::uint8_t>> &buffers, std::size_t srcindex, std::uint8_t *dest) const
{
	unsigned bytes = 0;
	for (int i = 0; i < srccount; i++)
	{
		if (srcindex >= buffers[i].size())
			std::fill_n(&dest[bytes], groupbytes, trailing_fill);
		else
		{
			std::size_t remaining = buffers[i].size() - srcindex;
			if (remaining < groupbytes)
			{
				std::copy_n(&buffers[i][srcindex], remaining, &dest[bytes]);
				std::fill_n(&dest[bytes + remaining], groupbytes - remaining, trailing_fill);
			}
			else
				std::copy_n(&buffers[i][srcindex], groupbytes, &dest[bytes]);
		}
		bytes += groupbytes;
	}
}

inline void bitswap_info::add_xor(unsigned pos, unsigned width) noexcept
{
	while (width + (pos & 63) > 64)
	{
		xorval[pos / 64] |= ~std::uint64_t(0) >> (pos & 63);
		unsigned newpos = (pos + std::min(width, 64U)) & ~63;
		width -= newpos - pos;
		pos = newpos;
	}
	xorval[pos / 64] |= std::uint64_t(~std::uint64_t(0) << (64 - width)) >> (pos & 63);
}

bool bitswap_info::parse(const char *optarg)
{
	bitcount = 0;
	swapcount = 0;
	std::fill(std::begin(xorval), std::end(xorval), 0);

	int ch = 0;
	while (optarg[ch] != '\0')
	{
		bool cpl = optarg[ch] == '/';
		if (cpl)
			ch++;

		if (optarg[ch] >= '0' && optarg[ch] <= '9')
		{
			unsigned n = optarg[ch++] - '0';
			while (optarg[ch] >= '0' && optarg[ch] <= '9')
			{
				n = n * 10 + (optarg[ch++] - '0');
				if (n >= MAX_SOURCE_BYTES * 8)
					return false;
			}

			unsigned m = n;
			if (optarg[ch] == '-')
			{
				ch++;
				if (optarg[ch] < '0' || optarg[ch] > '9')
					return false;
				m = optarg[ch++] - '0';
				while (optarg[ch] >= '0' && optarg[ch] <= '9')
				{
					m = m * 10 + (optarg[ch++] - '0');
					if (m >= MAX_SOURCE_BYTES * 8)
						return false;
				}
			}

			if (n < m)
				std::swap(n, m);

			int fieldlen = n - m + 1;
			if (swapcount >= MAX_SWAP_BITS || (bitcount + fieldlen) > MAX_SWAP_BITS)
				return false;

			// Combine this field with the previous one if the two are adjacent
			if (swapcount != 0 && bitspec[swapcount - 1].pos == n + 1 && bitspec[swapcount - 1].shift == bitcount)
			{
				bitspec[swapcount - 1].pos = m;
				bitspec[swapcount - 1].len += fieldlen;
				bitspec[swapcount - 1].shift = bitcount + fieldlen;
			}
			else
			{
				bitspec[swapcount].pos = m;
				bitspec[swapcount].len = fieldlen - 1;
				bitspec[swapcount].shift = bitcount + fieldlen;
				swapcount++;
			}

			if (cpl)
				add_xor(bitcount, fieldlen);
			bitcount += fieldlen;

			if (optarg[ch] != '\0' && optarg[ch++] != ',')
			{
				std::fprintf(stderr, "Expected comma after %u-%u field, saw 0x%02X instead\n", n, m, unsigned(optarg[ch - 1]));
				return false;
			}
		}
		else if (std::tolower(optarg[ch]) == 'z')
		{
			unsigned n = 1;
			ch++;
			if (optarg[ch] >= '1' && optarg[ch] <= '9')
			{
				n = optarg[ch++] - '0';
				while (optarg[ch] >= '0' && optarg[ch] <= '9')
				{
					n = n * 10 + (optarg[ch++] - '0');
					if (n > MAX_SOURCE_BYTES * 8)
						return false;
				}
			}

			if ((bitcount + n) > MAX_SWAP_BITS)
				return false;

			if (cpl)
				add_xor(bitcount, n);
			bitcount += n;

			if (optarg[ch] != '\0' && optarg[ch++] != ',')
			{
				std::fprintf(stderr, "Expected comma after Z%u field, saw 0x%02X instead\n", n, unsigned(optarg[ch - 1]));
				return false;
			}
		}
		else
			return false;
	}

	// Convert left aligned to right aligned now that bitcount is final
	for (int i = 0; i < swapcount; i++)
		bitspec[i].shift = bitcount - bitspec[i].shift;
	if (bitcount <= 64)
		xorval[0] >>= 64 - bitcount;
	else
	{
		int lwcount = unsigned(bitcount + 63) / 64;
		if ((bitcount & 63) != 0)
		{
			// Do a big barrel shift
			for (int i = lwcount - 1; i > 0; i--)
				xorval[i] = xorval[i] >> (64 - (bitcount & 63)) | xorval[i - 1] << (bitcount & 63);
			xorval[0] >>= 64 - (bitcount & 63);
		}
		std::reverse(&xorval[0], &xorval[lwcount]);
	}

	return true;
}

void bitswap_info::explode(int bytebits, util::endianness endian)
{
	int i = 0;
	while (i < swapcount)
	{
		// Split up fields if they cross 64-bit boundaries
		if ((bitspec[i].shift & 63) + bitspec[i].len >= 64)
		{
			unsigned msb = bitspec[i].shift + bitspec[i].len;
			std::copy_backward(&bitspec[i], &bitspec[swapcount], &bitspec[swapcount + 1]);
			bitspec[i].shift = msb & ~63;
			bitspec[i].len = msb & 63;
			bitspec[i].pos += bitspec[i].shift - bitspec[i + 1].shift;
			bitspec[i + 1].len -= (msb & 63) + 1;
			swapcount++;
		}

		// Split up fields by source bytes
		unsigned x = (bitspec[i].pos & (bytebits - 1)) + bitspec[i].len + 1;
		if (x > bytebits)
		{
			unsigned y = (x - 1) / bytebits;
			std::copy_backward(&bitspec[i + 1], &bitspec[swapcount], &bitspec[swapcount + y]);

			bitspec[i + y].pos = bitspec[i].pos;
			bitspec[i + y].len = bytebits - 1 - (bitspec[i].pos & (bytebits - 1));
			bitspec[i + y].shift = bitspec[i].shift;
			for (int k = i + y; --k >= i; )
			{
				bitspec[k].pos = bitspec[k + 1].pos + bitspec[k + 1].len + 1;
				bitspec[k].len = bytebits - 1 - (k == i ? -x & (bytebits - 1) : 0);
				bitspec[k].shift = bitspec[k + 1].shift + bitspec[k + 1].len + 1;
			}
			swapcount += y;
			i += y + 1;
		}
		else
			i++;
	}

	if (LOG_BITSWAP)
	{
		for (int i = 0; i < swapcount; i++)
		{
			const field_specifier &spec = bitspec[i];
			std::printf("output<%u:%u> = input<%u:%u>\n", spec.shift + spec.len, spec.shift, spec.pos + spec.len, spec.pos);
		}
	}

	// Translate logical byte numbers to physical byte numbers
	if (endian == util::endianness::big)
	{
		const int factor = 8 / bytebits;
		for (int i = 0; i < swapcount; i++)
			bitspec[i].pos = ((MAX_SOURCE_BYTES - 1) * 8 - (bitspec[i].pos & ~(bytebits - 1)) * factor) | (bitspec[i].pos & (bytebits - 1));
	}
	else if (bytebits < 8)
	{
		const int factor = 8 / bytebits;
		for (int i = 0; i < swapcount; i++)
			bitspec[i].pos = ((bitspec[i].pos & ~(bytebits - 1)) * factor) | (bitspec[i].pos & (bytebits - 1));
	}

	if (LOG_BITSWAP)
	{
		for (int i = 0; i < swapcount; i++)
		{
			const field_specifier &spec = bitspec[i];
			std::printf("output<%u:%u> = input byte %u<%u:%u> [%02X]\n", spec.shift + spec.len, spec.shift, spec.pos >> 3, (spec.pos & 7) + spec.len, spec.pos & 7, spec.pos);
		}
	}
}

static bool validate_options(const char *argv0, const interleave_info &interleave, const bitswap_info &data_bitswap, const output_info &output)
{
	if ((interleave.groupbits & (std::min(interleave.groupbits, 8) - 1)) != 0 || interleave.groupbits > MAX_SOURCE_BYTES * 8)
	{
		std::fprintf(stderr, "%s: Invalid number of grouping bits %d (must be 1, 2, 4 or a multiple of 8 not greater than %d)\n", argv0, interleave.groupbits, MAX_SOURCE_BYTES * 8);
		return false;
	}
	if ((interleave.groupbits <= 8 ? interleave.srccount : interleave.srccount * interleave.groupbits / 8) > MAX_SOURCE_BYTES)
	{
		std::fprintf(stderr, "%s: Too many inputs for %d-bit interleave\n", argv0, interleave.groupbits);
		return false;
	}

	const int srcbits = interleave.srccount * interleave.groupbits;
	if (data_bitswap.bitcount > output.word_bytes * 8)
	{
		std::fprintf(stderr, "%s: %d-bit words cannot fit into %d bytes each\n", argv0, data_bitswap.bitcount, output.word_bytes);
		return false;
	}
	for (int n = 0; n < data_bitswap.swapcount; n++)
	{
		if (data_bitswap.bitspec[n].pos + data_bitswap.bitspec[n].len >= srcbits)
		{
			std::fprintf(stderr, "%s: Invalid bitfield %d-%d specified (only %d bits per source word)\n", argv0,
				data_bitswap.bitspec[n].pos + data_bitswap.bitspec[n].len, data_bitswap.bitspec[n].pos, srcbits);
			return false;
		}
	}

	if (!interleave.endian_specified && (interleave.srccount > 1 || interleave.groupbits > 8 || output.word_bytes > 1))
	{
		if (interleave.groupbits < 8 || srcbits != output.word_bytes * 8)
		{
			std::fprintf(stderr, "%s: Endianness must be specified for transforming %dx%d bits into %d-byte words\n", argv0, interleave.srccount, interleave.groupbits, output.word_bytes);
			return false;
		}
		if (!data_bitswap.unswapped || data_bitswap.has_xor())
		{
			std::fprintf(stderr, "%s: Endianness must be specified for swapping/masking %d-bit words\n", argv0, srcbits);
			return false;
		}
	}

	if (output.word_bytes > MAX_WORD_BYTES)
	{
		std::fprintf(stderr, "%s: Too many bytes per output word (must be %u or less)\n", argv0, MAX_WORD_BYTES);
		return false;
	}

	return true;
}

bool output_info::write(const std::uint8_t *bytebuf, const char *argv0)
{
	std::uint32_t actual;
	std::error_condition err = file->write(bytebuf, writeoffs, word_bytes, actual);
	if (err)
	{
		std::fprintf(stderr, "%s: Error writing output: %s\n", argv0, err.message().c_str());
		return false;
	}
	if (word_bytes != actual)
	{
		std::fprintf(stderr, "%s: Error writing output\n", argv0);
		return false;
	}
	writeoffs += word_bytes;

	return true;
}

static void show_usage(FILE *file, const char *argv0)
{
	std::fprintf(file, "Usage: %s <options> <infile1> <infile2> ... <infileN> <outfile>\n"
					   "  Interleave and bitswap data from binary files.\n"
					   "  Options include:\n"
					   "    [-i|--interleave] <N>[x<B>[be|le]]\n"
					   "                                    Specify the number of input files, the\n"
					   "                                    number of bits to be read from each\n"
					   "                                    file into each input word, and the\n"
					   "                                    endianness of the input and output.\n"
					   "                                    Endianness must be specified for all\n"
					   "                                    nontrivial transformations of data.\n"
					   "    [-b|--data-bitswap] [[/]<N>[-<M>]|z[<N>]],...\n"
					   "                                    Specify a sequence of bitfields to be\n"
					   "                                    selected to form each output word.\n"
					   "                                    Each bitfield consists of one or more\n"
					   "                                    consecutive bits extracted from the input\n"
					   "                                    word, numbered from 0 = LSB. The fields\n"
					   "                                    are repacked in order and aligned to the\n"
					   "                                    right within each output word. / inverts\n"
					   "                                    fields; z inserts one or more zero bits in\n"
					   "                                    between bitfields; /z inserts one bits.\n"
					   "    [-z|--archive_file] <zipfile>   Locate and automatically decompress input\n"
					   "                                    files from within ZIP or 7zip archive.\n"
					   "    [-s|--start-offset] <offset>    Begin copying after skipping to the given\n"
					   "                                    byte offset in each input file.\n"
					   "    [-c|--word-count] <words>       Copy only the specified number of words\n"
					   "                                    instead of the remainder of all input.\n"
					   "    [-f|--trailing-fill] <value>    Pad data from short input files with the\n"
					   "                                    specified byte value (in hexadecimal).\n"
					   "    [-w|--word-bytes] <bytes>       Output the given number of bytes per word.\n"
					   "                                    If this option is unspecified, the size of\n"
					   "                                    the output word will be rounded up to the\n"
					   "                                    nearest power of 2.\n"
					   "    [-h|--help]                     Display this output.\n",
		argv0);
}

int main(int argc, char *argv[])
{
	interleave_info interleave;
	bitswap_info data_bitswap;
	const char *archive_filename = nullptr;
	output_info output;
	std::uint64_t start_offset = 0;
	std::uint32_t word_count = 0;
	std::uint32_t options_used = 0;

	int argi = 1;
	while (argi < argc)
	{
		if (argv[argi][0] != '-' || argv[argi][1] == '\0')
			break;

		int optindex = 0;
		const char *const optname = &argv[argi++][1];
		if (optname[1] == '\0')
		{
			// "--" terminates the option list
			if (optname[0] == '-')
				break;

			// Look up the option by shortname
			while (optindex != MAX_OPTIONS)
			{
				if (optname[0] == s_options[optindex].shortname)
					break;
				optindex++;
			}
		}
		else if (optname[0] == '-')
		{
			// Look up the option by longname
			while (optindex != MAX_OPTIONS)
			{
				if (!std::strcmp(optname + 1, s_options[optindex].longname))
					break;
				optindex++;
			}
		}
		else
			optindex = MAX_OPTIONS;
		if (optindex == MAX_OPTIONS)
		{
			std::fprintf(stderr, "%s: Unrecognized option -%s\n", argv[0], optname);
			show_usage(stderr, argv[0]);
			return 1;
		}

		// Options may be specified only once
		if (util::BIT(options_used, optindex))
		{
			std::fprintf(stderr, "%s: Duplicate option -%s\n", argv[0], optname);
			show_usage(stderr, argv[0]);
			return 1;
		}
		options_used |= 1 << optindex;

		// Each option takes one argument (except --help)
		if (argi + 1 == argc && optindex != OPTION_HELP)
		{
			std::fprintf(stderr, "%s: No argument for option -%s\n", argv[0], optname);
			show_usage(stderr, argv[0]);
			return 1;
		}

		// Process each option appropriately
		if (optindex == OPTION_INTERLEAVE)
		{
			if (!interleave.parse(argv[argi++]))
			{
				std::fprintf(stderr, "%s: Invalid interleave specification\n", argv[0]);
				return 1;
			}
		}
		else if (optindex == OPTION_DATA_BITSWAP)
		{
			if (!data_bitswap.parse(argv[argi++]))
			{
				std::fprintf(stderr, "%s: Invalid bitswap specification\n", argv[0]);
				return 1;
			}
		}
		else if (optindex == OPTION_ARCHIVE_FILE)
			archive_filename = argv[argi++];
		else if (optindex == OPTION_START_OFFSET)
			start_offset = std::strtoull(argv[argi++], nullptr, 0);
		else if (optindex == OPTION_WORD_COUNT)
			word_count = std::strtoul(argv[argi++], nullptr, 0);
		else if (optindex == OPTION_TRAILING_FILL)
		{
			const char *const optarg = argv[argi++];
			int i = optarg[0] == '0' && std::tolower(optarg[1]) == 'x' ? 2 : 0;
			if (optarg[i] == '\0' || (optarg[i + 1] != '\0' && optarg[i + 2] != '\0'))
			{
				std::fprintf(stderr, "%s: Invalid number of digits for -%s\n", argv[0], optname);
				return 1;
			}
			if (!std::isxdigit(optarg[i]) || (optarg[i + 1] != '\0' && !std::isxdigit(optarg[i + 1])))
			{
				std::fprintf(stderr, "%s: Invalid hex digit in -%s\n", argv[0], optname);
				return 1;
			}
			interleave.trailing_fill = std::strtoul(optarg, nullptr, 16);
		}
		else if (optindex == OPTION_WORD_BYTES)
		{
			const char *const optarg = argv[argi++];
			if (optarg[0] >= '1' && optarg[0] <= '9' && (optarg[1] == '\0' || (optarg[1] >= '0' && optarg[1] <= '9' && optarg[2] == '\0')))
			{
				output.word_bytes = optarg[0] - '0';
				if (optarg[1] != '\0')
					output.word_bytes = (output.word_bytes * 10) + optarg[1] - '0';
			}
			else
			{
				std::fprintf(stderr, "%s: Invalid number for -%s\n", argv[0], optname);
				return 1;
			}
		}
		else if (optindex != OPTION_HELP)
		{
			// Should never happen
			std::fprintf(stderr, "%s: Unhandled option -%s\n", argv[0], optname);
			return 1;
		}
	}

	// Validate the number of arguments following options
	if (util::BIT(options_used, OPTION_HELP))
	{
		show_usage(stdout, argv[0]);
		return 0;
	}
	else if (argc - argi < interleave.srccount + 1)
	{
		if (util::BIT(options_used, OPTION_INTERLEAVE))
			std::fprintf(stderr, "%s: Too few arguments following options (must have %d input filename%s followed by 1 output filename)\n", argv[0], interleave.srccount, interleave.srccount != 1 ? "s" : "");
		else
			show_usage(stderr, argv[0]);
		return 1;
	}
	else if (argc - argi > interleave.srccount + 1)
	{
		for (int argj = argi; argj < argc; argj++)
		{
			if (argv[argj][0] == '-')
			{
				std::fprintf(stderr, "%s: Too many arguments following options (option %s must precede input filenames)\n", argv[0], argv[argj]);
				return 1;
			}
		}
		std::fprintf(stderr, "%s: Too many arguments following options (must have %d input filename%s followed by 1 output filename)\n", argv[0], interleave.srccount, interleave.srccount != 1 ? "s" : "");
		return 1;
	}

	// Determine whether the data bitswap is unnecessary
	if (data_bitswap.bitcount == 0)
	{
		data_bitswap.swapcount = 1;
		data_bitswap.bitcount = interleave.srccount * interleave.groupbits;
		data_bitswap.bitspec[0].pos = 0;
		data_bitswap.bitspec[0].len = data_bitswap.bitcount - 1;
		data_bitswap.bitspec[0].shift = 0;
		data_bitswap.unswapped = true;
	}
	else if (data_bitswap.swapcount == 1
				&& data_bitswap.bitspec[0].pos == 0
				&& data_bitswap.bitspec[0].len == interleave.srccount * interleave.groupbits - 1
				&& data_bitswap.bitspec[0].shift == 0)
		data_bitswap.unswapped = true;

	// Supply a power-of-two size if the option was not specified
	if (!util::BIT(options_used, OPTION_WORD_BYTES))
	{
		output.word_bytes = 1;
		while (output.word_bytes < MAX_WORD_BYTES && data_bitswap.bitcount > output.word_bytes * 8)
			output.word_bytes *= 2;
	}

	// Verify that the combination of options is sensible
	if (!validate_options(argv[0], interleave, data_bitswap, output))
		return 1;

	std::vector<std::vector<std::uint8_t>> buffers;
	if (util::BIT(options_used, OPTION_ARCHIVE_FILE))
	{
		// Try opening as either .zip or .7z
		util::archive_file::ptr zip;
		std::error_condition err = util::archive_file::open_zip(archive_filename, zip);
		if (err && err != std::errc::no_such_file_or_directory)
			err = util::archive_file::open_7z(archive_filename, zip);
		if (err)
		{
			std::fprintf(stderr, "%s: Error opening archive file %s\n", argv[0], archive_filename);
			return 1;
		}

		for (int j = 0; j < interleave.srccount; j++)
		{
			// Be a bit permissive about filenames in archives
			const char *const filename = argv[argi + j];
			int header = zip->search(filename, false);
			if (header < 0)
				header = zip->search(filename, true);
			if (header < 0)
			{
				std::fprintf(stderr, "%s: Could not find %s in archive\n", argv[0], filename);
				return 1;
			}

			// Prepare a buffer to decompress into
			std::size_t length = zip->current_uncompressed_length();
			if (start_offset >= length)
			{
				std::fprintf(stderr, "%s: File %s is too short\n", argv[0], filename);
				return 1;
			}
			try
			{
				buffers.emplace_back(length);
			}
			catch (std::exception const &e)
			{
				std::fprintf(stderr, "%s: Exception while allocating buffer for decompressing %s: %s\n", argv[0], filename, e.what());
				return 1;
			}

			// Decompress the entire file (might be wasteful, but util::archive_file requires it)
			err = zip->decompress(buffers.back().data(), length);
			if (err)
			{
				std::fprintf(stderr, "%s: Error decompressing %s: %s\n", argv[0], filename, err.message().c_str());
				return 1;
			}
		}
	}
	else
	{
		// Load all the files directly
		for (int j = 0; j < interleave.srccount; j++)
		{
			// Open each file in turn, remembering its full length
			const char *const filename = argv[argi + j];
			osd_file::ptr file;
			std::uint64_t full_length;
			std::error_condition err = osd_file::open(filename, OPEN_FLAG_READ, file, full_length);
			if (err)
			{
				std::fprintf(stderr, "%s: Error opening %s: %s\n", argv[0], filename, err.message().c_str());
				return 1;
			}
			if (start_offset >= full_length)
			{
				std::fprintf(stderr, "%s: File %s is too short\n", argv[0], filename);
				return 1;
			}
			if (!util::BIT(options_used, OPTION_WORD_COUNT) && full_length - start_offset > std::numeric_limits<std::uint32_t>::max())
			{
				std::fprintf(stderr, "%s: File %s is too long\n", argv[0], filename);
				return 1;
			}

			// Prepare to read in the file
			std::uint32_t length_to_read = full_length - start_offset;
			if (word_count != 0)
			{
				std::size_t byte_count = std::size_t(word_count) * interleave.groupbytes;
				if (length_to_read > byte_count)
					length_to_read = byte_count;
			}
			try
			{
				buffers.emplace_back(length_to_read);
			}
			catch (std::exception const &e)
			{
				std::fprintf(stderr, "%s: Exception while allocating buffer for loading %s: %s\n", argv[0], filename, e.what());
				return 1;
			}

			// Read in the file
			std::uint32_t actual;
			err = file->read(buffers.back().data(), start_offset, length_to_read, actual);
			if (err)
			{
				std::fprintf(stderr, "%s: Error loading %s: %s\n", argv[0], filename, err.message().c_str());
				return 1;
			}
			if (length_to_read != actual)
			{
				std::fprintf(stderr, "%s: Problem loading %s\n", argv[0], filename);
				return 1;
			}
		}

		// Everything before the start offset has already been skipped
		start_offset = 0;
	}

	std::size_t standard_length = 0;
	if (util::BIT(options_used, OPTION_WORD_COUNT))
		standard_length = std::size_t(word_count) * interleave.groupbytes + start_offset;
	else
	{
		for (auto &buf : buffers)
		{
			if (buf.size() > standard_length)
				standard_length = buf.size();
		}

		// Round up to nearest multiple
		int residue = (standard_length - start_offset) % interleave.groupbytes;
		if (residue != 0)
			standard_length += interleave.groupbytes - residue;
	}

	for (int j = 0; j < interleave.srccount; j++)
	{
		if (buffers[j].size() < standard_length)
			std::fprintf(stderr, "%s: Input file %s is %lu bytes short (data will be padded with 0x%02X)\n",
				argv[0], argv[argi + j], (unsigned long)(standard_length - buffers[j].size()), interleave.trailing_fill);
	}

	// Open the output file
	std::uint64_t length;
	std::error_condition err = osd_file::open(argv[argi + interleave.srccount], OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, output.file, length);
	if (err)
	{
		std::fprintf(stderr, "%s: Error opening %s for output: %s\n", argv[0], argv[argi + interleave.srccount], err.message().c_str());
		return 1;
	}

	std::uint8_t wordbuf[MAX_SOURCE_BYTES];
	std::uint8_t *const inbufp = &wordbuf[interleave.endian == util::endianness::big ? MAX_SOURCE_BYTES - interleave.srccount * interleave.groupbytes : 0];
	if (data_bitswap.unswapped && !data_bitswap.has_xor() && interleave.groupbits >= 8)
	{
		// Simple case in which the input data needs no further processing besides interleaving and possibly zero padding
		std::uint8_t *const outbufp = &wordbuf[interleave.endian == util::endianness::big ? MAX_SOURCE_BYTES - output.word_bytes : 0];
		std::fill_n(wordbuf, MAX_SOURCE_BYTES, 0);

		for (std::size_t srcindex = start_offset; srcindex < standard_length; srcindex += interleave.groupbytes)
		{
			// Read all the data
			interleave.read_sources(buffers, srcindex, inbufp);

			// Write the same data
			if (!output.write(outbufp, argv[0]))
				return 1;
		}
	}
	else
	{
		data_bitswap.explode(std::min(interleave.groupbits, 8), interleave.endian);

		if (output.word_bytes <= 8)
		{
			std::uint64_t dstw;
			std::uint8_t *const outbufp =
				&reinterpret_cast<std::uint8_t *>(&dstw)[
					(output.word_bytes == 1 ? util::endianness::native == util::endianness::little : interleave.endian == util::endianness::little)
						? 0 : sizeof(dstw) - output.word_bytes
				];
			for (std::size_t srcindex = start_offset; srcindex < standard_length; srcindex += interleave.groupbytes)
			{
				// Read input data from all interleaved sources
				interleave.read_sources(buffers, srcindex, inbufp);

				// Bitswap and XOR the data word
				dstw = data_bitswap.swap(wordbuf);

				if (output.word_bytes != 1 && interleave.endian != util::endianness::native)
					dstw = swapendian_int64(dstw);
				if (!output.write(outbufp, argv[0]))
					return 1;
			}
		}
		else
		{
			std::uint64_t dstbuf[MAX_WORD_BYTES / 8];
			unsigned const dstbuflen = (output.word_bytes + 7) / 8;
			std::uint8_t *const outbufp =
				&reinterpret_cast<std::uint8_t *>(&dstbuf[0])[interleave.endian == util::endianness::little ? 0 : dstbuflen * 8 - output.word_bytes];

			for (std::size_t srcindex = start_offset; srcindex < standard_length; srcindex += interleave.groupbytes)
			{
				// Read input data from all interleaved sources
				interleave.read_sources(buffers, srcindex, inbufp);

				// Bitswap and XOR the data word
				if (interleave.endian != util::endianness::native)
					data_bitswap.swap_long<true>(wordbuf, dstbuf, dstbuflen);
				else
					data_bitswap.swap_long<false>(wordbuf, dstbuf, dstbuflen);

				if (!output.write(outbufp, argv[0]))
					return 1;
			}
		}
	}

	// All done
	return 0;
}
