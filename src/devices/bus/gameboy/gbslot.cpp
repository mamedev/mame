// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Game Boy cartridge slot

 ***************************************************************************/

#include "emu.h"
#include "gbslot.h"

#include "cartheader.h"
#include "carts.h"
#include "gbxfile.h"
#include "mmm01.h"

#include "bus/generic/slot.h"
#include "cpu/lr35902/lr35902d.h"

#include "disasmintf.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class dasm_helper : public util::disasm_interface::data_buffer
{
public:
	dasm_helper(u8 const *data, offs_t base_pc) : m_data(data), m_base_pc(base_pc)
	{ }

	virtual u8 r8(offs_t pc) const override
	{ return m_data[pc - m_base_pc]; }

	virtual u16 r16(offs_t pc) const override
	{ return r8(pc) | (u16(r8(pc + 1)) << 8); }

	virtual u32 r32(offs_t pc) const override
	{ return r16(pc) | (u32(r16(pc + 2)) << 16); }

	virtual u64 r64(offs_t pc) const override
	{ return r32(pc) | (u64(r32(pc + 4)) << 32); }

private:
	u8 const *const m_data;
	offs_t const m_base_pc;
};


std::string disassemble_entry_point(u8 const *header)
{
	dasm_helper helper(header, 0x0100);
	lr35902_disassembler dasm;
	std::ostringstream str;
	auto const result = dasm.disassemble(str, 0x0100, helper, helper);
	str << " (";
	bool first = true;
	for (unsigned i = 0; (result & util::disasm_interface::LENGTHMASK) > i; ++i)
	{
		util::stream_format(str, first ? "%02X" : " %02X", header[i]);
		first = false;
	}
	str << ')';
	return std::move(str).str();
}


std::string cart_title_text(u8 const *header)
{
	std::string_view title(
			reinterpret_cast<char const *>(&header[cartheader::OFFSET_TITLE - 0x100]),
			cartheader::OFFSET_LICENSEE_EXT - cartheader::OFFSET_TITLE);

	// best effort to strip off repurposed fields
	if (!title[cartheader::OFFSET_MANUFACTURER - cartheader::OFFSET_TITLE - 1])
		title.remove_suffix(cartheader::OFFSET_LICENSEE_EXT - cartheader::OFFSET_MANUFACTURER);
	if (BIT(title.back(), 7))
		title.remove_suffix(1);

	// strip NUL padding, short-circuit empty titles
	auto const last = title.find_last_not_of('\0');
	if (std::string_view::npos == last)
		return "\"\"";
	title.remove_suffix(title.length() - last - 1);

	// replace non-ASCII characters with octal escapes
	std::ostringstream str;
	str.setf(std::ios_base::oct, std::ios_base::basefield);
	str.fill('0');
	str << '"';
	while (!title.empty())
	{
		auto const invalid = std::find_if(
				title.begin(),
				title.end(),
				[] (char ch) { return (' ' > ch) || ('~' < ch); });
		auto const len = std::distance(title.begin(), invalid);
		if (len)
		{
			str << title.substr(0, len);
			title.remove_prefix(len);
		}
		else
		{
			str << '\\';
			str.width(3);
			str << unsigned(u8(title.front()));
			title.remove_prefix(1);
		}
	}
	str << '"';
	return std::move(str).str();
}


char const *cart_cgb_support_description(u8 const flags)
{
	if (!BIT(flags, 7))
		return "unsupported";
	else if (BIT(flags, 2, 2))
		return "PGB mode";
	else if (BIT(flags, 6))
		return "required";
	else
		return "enhanced";
}


char const *cart_sgb_support_description(u8 const *header)
{
	if (0x33 != header[cartheader::OFFSET_LICENSEE - 0x100])
		return "unaware";
	else if (0x03 != header[cartheader::OFFSET_SGB - 0x100])
		return "unsupported";
	else
		return "enhanced";
}


char const *cart_type_description(u8 type)
{
	switch (type)
	{
	case cartheader::TYPE_ROM:
		return "flat ROM";
	case cartheader::TYPE_MBC1:
		return "Nintendo MBC1";
	case cartheader::TYPE_MBC1_RAM:
		return "Nintendo MBC1 with RAM";
	case cartheader::TYPE_MBC1_RAM_BATT:
		return "Nintendo MBC1 with battery-backed RAM";
	case cartheader::TYPE_MBC2:
		return "Nintendo MBC2";
	case cartheader::TYPE_MBC2_BATT:
		return "Nintendo MBC2 with backup battery";
	case cartheader::TYPE_ROM_RAM:
		return "flat ROM with RAM";
	case cartheader::TYPE_ROM_RAM_BATT:
		return "flat ROM with battery-backed RAM";
	case cartheader::TYPE_MMM01:
		return "Nintendo MMM01";
	case cartheader::TYPE_MMM01_RAM:
		return "Nintendo MMM01 with RAM";
	case cartheader::TYPE_MMM01_RAM_BATT:
		return "Nintendo MMM01 with battery-backed RAM";
	case cartheader::TYPE_MBC3_RTC_BATT:
		return "Nintendo MBC3 with backup battery and real-time clock crystal";
	case cartheader::TYPE_MBC3_RTC_RAM_BATT:
		return "Nintendo MBC3 with battery-backed RAM and real-time clock crystal";
	case cartheader::TYPE_MBC3:
		return "Nintendo MBC3";
	case cartheader::TYPE_MBC3_RAM:
		return "Nintendo MBC3 with RAM";
	case cartheader::TYPE_MBC3_RAM_BATT:
		return "Nintendo MBC3 with battery-backed RAM";
	case cartheader::TYPE_MBC5:
		return "Nintendo MBC5";
	case cartheader::TYPE_MBC5_RAM:
		return "Nintendo MBC5 with RAM";
	case cartheader::TYPE_MBC5_RAM_BATT:
		return "Nintendo MBC5 with battery-backed RAM";
	case cartheader::TYPE_MBC5_RUMBLE:
		return "Nintendo MBC5 with vibration motor";
	case cartheader::TYPE_MBC5_RUMBLE_RAM:
		return "Nintendo MBC5 with RAM and vibration motor";
	case cartheader::TYPE_MBC5_RUMBLE_RAM_BATT:
		return "Nintendo MBC5 with battery-backed RAM and vibration motor";
	case cartheader::TYPE_MBC6:
		return "Nintendo MBC6 with Flash memory and battery-backed RAM";
	case cartheader::TYPE_MBC7_ACCEL_EEPROM:
		return "Nintendo MBC7 with EEPROM and two-axis accelerometer";
	case cartheader::TYPE_UNLICENSED_YONGYONG:
		return "Yong Yong pirate";
	case cartheader::TYPE_CAMERA:
		return "Nintendo Game Boy Camera with battery-backed RAM";
	case cartheader::TYPE_TAMA5:
		return "Bandai TAMA5 with battery-backed TAMA6 and real-time clock";
	case cartheader::TYPE_HUC3:
		return "Hudson Soft HuC-3 with battery-baked RAM";
	case cartheader::TYPE_HUC1_RAM_BATT:
		return "Hudson Soft HuC-1 with battery-baked RAM";
	default:
		return "unknown";
	}
}


std::string cart_rom_size_description(u8 size, memory_region *romregion)
{
	auto const actual = romregion ? romregion->bytes() : 0U;
	switch (size)
	{
	case cartheader::ROM_SIZE_32K:
	case cartheader::ROM_SIZE_64K:
	case cartheader::ROM_SIZE_128K:
	case cartheader::ROM_SIZE_256K:
	case cartheader::ROM_SIZE_512K:
		return util::string_format(
				"%u KiB actual %u%s",
				32U << (size - cartheader::ROM_SIZE_32K),
				actual,
				((u32(0x8000) << (size - cartheader::ROM_SIZE_32K)) != actual) ? " MISMATCH!" : "");
	case cartheader::ROM_SIZE_1024K:
	case cartheader::ROM_SIZE_2048K:
	case cartheader::ROM_SIZE_4096K:
	case cartheader::ROM_SIZE_8192K:
		return util::string_format(
				"%u MiB actual %u%s",
				1U << (size - cartheader::ROM_SIZE_1024K),
				actual,
				((u32(0x8000) << (size - cartheader::ROM_SIZE_32K)) != actual) ? " MISMATCH!" : "");
	case cartheader::ROM_SIZE_1152K:
	case cartheader::ROM_SIZE_1280K:
	case cartheader::ROM_SIZE_1536K:
		return util::string_format(
				"%u KiB actual %u%s",
				0x400U | (0x80U << (size - cartheader::ROM_SIZE_1152K)),
				actual,
				((u32(0x10'0000) | (u32(0x2'0000) << (size - cartheader::ROM_SIZE_1152K))) != actual) ? " MISMATCH!" : "");
	default:
		return util::string_format("unknown actual %u", actual);
	}
}


std::string cart_ram_size_description(u8 size)
{
	switch (size)
	{
	case cartheader::RAM_SIZE_0K:
		return "none";
	case cartheader::RAM_SIZE_2K:
	case cartheader::RAM_SIZE_8K:
	case cartheader::RAM_SIZE_32K:
	case cartheader::RAM_SIZE_128K:
		return util::string_format("%u KiB", 2U << ((size - cartheader::RAM_SIZE_2K) << 1));
	case cartheader::RAM_SIZE_64K:
		return "64 KiB";
	default:
		return "unknown";
	}
}


char const *cart_region_description(u8 region)
{
	switch (region)
	{
	case 0: return "Japan";
	case 1: return "export";
	default: return "unknown";
	}
}


std::string cart_licensee_description(u8 const *header)
{
	// TODO: fill in and correct lists, distinguish duplicates
	static constexpr std::pair<u8, char const *> BASE[] = {
													{ 0x01, "Nintendo" },
			{ 0x08, "Capcom" },                     { 0x09, "Hot-B" },                  { 0x0a, "Jaleco" },                     { 0x0b, "Coconuts Japan" },
			{ 0x0c, "Elite Systems" },
																																{ 0x13, "Electronic Arts" },
			{ 0x18, "Hudson Soft" },                { 0x19, "ITC Entertainment" },      { 0x1a, "Yanoman" },
													{ 0x1d, "Japan Clary" },                                                    { 0x1f, "Virgin" },
			{ 0x24, "PCM Complete" },               { 0x25, "San-X" },
			{ 0x28, "KEMCO" },                      { 0x29, "Seta" },
			{ 0x30, "Infogrames" },                 { 0x31, "Nintendo" },               { 0x32, "Bandai" },
			{ 0x34, "Konami" },                     { 0x35, "HectorSoft" },
			{ 0x38, "Capcom" },                     { 0x39, "Banpresto" },
																						{ 0x3e, "Gremlin" },
													{ 0x41, "Ubi Soft" },               { 0x42, "Atlus" },
			{ 0x44, "Malibu" },                                                         { 0x46, "Angel" },                      { 0x47, "Spectrum Holobyte" },
													{ 0x49, "Irem" },                   { 0x4a, "Virgin" },
													{ 0x4d, "Malibu" },                                                         { 0x4f, "U.S. Gold" },
			{ 0x50, "Absolute" },                   { 0x51, "Acclaim" },                { 0x52, "Activision" },                 { 0x53, "American Sammy" },
			{ 0x54, "GameTek" },                    { 0x55, "Park Place Productions" }, { 0x56, "LJN Entertainment" },          { 0x57, "Matchbox" },
													{ 0x59, "Milton Bradley" },         { 0x5a, "Mindscape" },                  { 0x5b, "Romstar" },
			{ 0x5c, "Naxat Soft" },                 { 0x5d, "Tradewest" },
			{ 0x60, "Titus" },                      { 0x61, "Virgin" },
																																{ 0x67, "Ocean" },
													{ 0x69, "Electronic Arts" },
																						{ 0x6e, "Elite Systems" },              { 0x6f, "Electro Brain Corp." },
			{ 0x70, "Infogrames" },                 { 0x71, "Interplay" },              { 0x72, "Broderbund" },                 { 0x73, "Sculptured Software" },
													{ 0x75, "SCi Games" },
			{ 0x78, "THQ" },                        { 0x79, "Accolade" },               { 0x7a, "Triffix Entertainment" },
			{ 0x7c, "Microprose" },                                                                                             { 0x7f, "Kemco" },
			{ 0x80, "Misawa Entertainment" },                                                                                   { 0x83, "LOZC" },
																						{ 0x86, "Tokuma Shoten Intermedia " },
																																{ 0x8b, "Bullet-Proof Software" },
			{ 0x8c, "Vic Tokai" },                                                      { 0x8e, "Ape" },                        { 0x8f, "I'Max Corp." },
													{ 0x91, "Chunsoft Co." },           { 0x92, "Video System" },               { 0x93, "Tsuburaya Productions Co." },
													{ 0x95, "Varie Corporation" },                                              { 0x97, "Kaneko" },
													{ 0x99, "Arc" },                    { 0x9a, "Nihon Bussan Co." },           { 0x9b, "Tecmo" },
			{ 0x9c, "Imagineer" },                  { 0x9d, "Banpresto" },                                                      { 0x9f, "Nova" },
													{ 0xa1, "Hori Electric Co." },      { 0xa2, "Bandai" },
			{ 0xa4, "Konami" },                                                         { 0xa6, "Kawada Co." },                 { 0xa7, "Takara" },
													{ 0xa9, "Technos Japan" },          { 0xaa, "Broderbund" },
			{ 0xac, "Toei Animation" },             { 0xad, "Toho Co." },                                                       { 0xaf, "Namco" },
			{ 0xb0, "Acclaim" },                    { 0xb1, "ASCII Corporation" },      { 0xb2, "Bandai" },
			{ 0xb4, "Enix" },                                                           { 0xb6, "HAL Laboratory" },             { 0xb7, "SNK" },
													{ 0xb9, "Pony Canyon" },            { 0xba, "Culture Brain" },              { 0xbb, "Sunsoft" },
													{ 0xbd, "Sony Imagesoft" },                                                 { 0xbf, "Sammy" },
			{ 0xc0, "Taito" },                                                          { 0xc2, "Kemco" },                      { 0xc3, "Square Soft" },
			{ 0xc4, "Tokuma Shoten Intermedia" },   { 0xc5, "Data East" },              { 0xc6, "Tonkinhouse" },
			{ 0xc8, "Koei" },                                                           { 0xca, "Ultra" },                      { 0xcb, "Vap" },
			{ 0xcc, "Use Corporation" },            { 0xcd, "Meldac" },                 { 0xce, "Pony Canyon" },                { 0xcf, "Angel" },
			{ 0xd0, "Taito" },                      { 0xd1, "Sofel" },                  { 0xd2, "quest" },                      { 0xd3, "Sigma Enterprises" },
			{ 0xd4, "ASK Kodansha Co." },                                               { 0xd6, "Naxat Soft" },                 { 0xd7, "Copya System" },
													{ 0xd9, "Banpresto" },              { 0xda, "Tomy" },                       { 0xdb, "LJN Entertainment" },
													{ 0xdd, "NCS Corporation" },        { 0xde, "Human" },                      { 0xdf, "Altron Corporation" },
			{ 0xe0, "Jaleco" },                     { 0xe1, "Towa Chiki" },             { 0xe2, "Yutaka" },                     { 0xe3, "Varie Corporation" },
													{ 0xe5, "Epoch Co." },                                                      { 0xe7, "Athena" },
			{ 0xe8, "Asmik Ace Entertainment" },    { 0xe9, "Natsume" },                { 0xea, "King Records Co." },           { 0xeb, "Atlus" },
			{ 0xec, "Epic/Sony Records" },                                              { 0xee, "IGS Co." },
			{ 0xf0, "A Wave" },                                                                                                 { 0xf3, "Extreme Entertainment Group" },
																																{ 0xff, "LJN Entertainment" } };
	static constexpr std::pair<u16, char const *> EXTENDED[] = {
			{ 0x3031, "Nintendo R&D1" },
			{ 0x3038, "Capcom" },
			{ 0x3133, "Electronic Arts" },
			{ 0x3138, "Hudson Soft" },
			{ 0x3139, "b-ai" },
			{ 0x3230, "KSS" },
			{ 0x3232, "pow" },
			{ 0x3234, "PCM Complete" },
			{ 0x3235, "San-X" },
			{ 0x3238, "Kemco Japan" },
			{ 0x3239, "Seta" },
			{ 0x3330, "Viacom" },
			{ 0x3331, "Nintendo" },
			{ 0x3332, "Bandai" },
			{ 0x3333, "Ocean/Acclaim" },
			{ 0x3334, "Konami" },
			{ 0x3335, "HectorSoft" },
			{ 0x3337, "Taito" },
			{ 0x3338, "Hudson" },
			{ 0x3339, "Banpresto" },
			{ 0x3431, "Ubi Soft" },
			{ 0x3432, "Atlus" },
			{ 0x3434, "Malibu" },
			{ 0x3436, "Angel" },
			{ 0x3437, "Bullet-Proof Software" },
			{ 0x3439, "Irem" },
			{ 0x3530, "Absolute" },
			{ 0x3531, "Acclaim" },
			{ 0x3532, "Activision" },
			{ 0x3533, "American Sammy" },
			{ 0x3534, "Konami" },
			{ 0x3535, "Hi Tech Entertainment" },
			{ 0x3536, "LJN Entertainment" },
			{ 0x3537, "Matchbox" },
			{ 0x3538, "Mattel" },
			{ 0x3539, "Milton Bradley" },
			{ 0x3630, "Titus" },
			{ 0x3631, "Virgin" },
			{ 0x3634, "LucasArts" },
			{ 0x3637, "Ocean" },
			{ 0x3639, "Electronic Arts" },
			{ 0x3730, "Infogrames" },
			{ 0x3731, "Interplay" },
			{ 0x3732, "Broderbund" },
			{ 0x3733, "Sculptured Software" },
			{ 0x3735, "SCi Games" },
			{ 0x3738, "THQ" },
			{ 0x3739, "Accolade" },
			{ 0x3830, "Misawa Entertainment" },
			{ 0x3833, "LOZC" },
			{ 0x3836, "Tokuma Shoten Intermedia" },
			{ 0x3837, "Tsukuda Original" },
			{ 0x3931, "Chunsoft Co." },
			{ 0x3932, "Video System" },
			{ 0x3933, "Ocean/Acclaim" },
			{ 0x3935, "Varie Corporation" },
			{ 0x3936, "Yonezawa/s'pal" },
			{ 0x3937, "Kaneko" },
			{ 0x3939, "Pack-In-Video Co." },
			{ 0x4134, "Konami (Yu-Gi-Oh!)" } };

	// 0x33 means two bytes of the title have been repurposed
	u8 const licensee = header[cartheader::OFFSET_LICENSEE - 0x100];
	if (0x33 == licensee)
	{
		u16 const ext = (u16(header[cartheader::OFFSET_LICENSEE_EXT - 0x100]) << 8) | header[cartheader::OFFSET_LICENSEE_EXT + 1 - 0x100];
		auto const found = std::lower_bound(
				std::begin(EXTENDED),
				std::end(EXTENDED),
				ext,
				[] (auto const &x, auto const &y) { return x.first < y; });
		if ((std::end(EXTENDED) != found) && (found->first == ext))
			return util::string_format("0x%02X (%s)", ext, found->second);
		else
			return util::string_format("0x%04X (unknown)", ext);
	}
	else
	{
		auto const found = std::lower_bound(
				std::begin(BASE),
				std::end(BASE),
				licensee,
				[] (auto const &x, auto const &y) { return x.first < y; });
		if ((std::end(BASE) != found) && (found->first == licensee))
			return util::string_format("0x%02X (%s)", licensee, found->second);
		else
			return util::string_format("0x%02X (unknown)", licensee);
	}
}


u32 get_mmm01_initial_low_page(u64 length)
{
	// this is inefficient but it works, offloading the logic to a common helper
	std::vector<unsigned> pages;
	auto const mask = device_generic_cart_interface::map_non_power_of_two(
			unsigned(length / 0x4000),
			[&pages] (unsigned entry, unsigned page) { pages.emplace_back(page); });
	return u32(pages[0x01fe & mask] * u32(0x4000));
}


bool is_wisdom_tree(std::string_view tag, util::random_read &file, u64 length, u64 offset, u8 const *header)
{
	// all Wisdom Tree games declare 32 KiB ROM
	if (cartheader::ROM_SIZE_32K != header[cartheader::OFFSET_ROM_SIZE - 0x100])
		return false;

	// no Wisdom Tree game declares cartridge RAM
	if (cartheader::RAM_SIZE_0K != header[cartheader::OFFSET_RAM_SIZE - 0x100])
		return false;

	// 8 MiB maximum supported program ROM
	if ((u32(0x8000) << 8) < length)
		return false;

	// only makes sense with more than 32 KiB program ROM
	if (0x8000 >= length)
		return false;

	// must be multiple of 32 KiB program ROM page size
	if (length & 0x7fff)
		return false;

	// assume Wisdom Tree
	osd_printf_verbose(
			"[%s] Assuming 0x%06X-byte cartridge declaring 32 KiB ROM and no RAM uses Wisdom Tree banking\n",
			tag,
			length);
	return true;
}


bool is_mbc30(std::string_view tag, util::random_read &file, u64 length, u64 offset, u8 const *header)
{
	// MBC30 supposedly has an additional ROM bank output
	if ((u32(0x4000) << 7) < length)
	{
		osd_printf_verbose(
				"[%s] Assuming 0x%06X-byte cartridge declaring MBC3 controller uses MBC30\n",
				tag,
				length);
		return true;
	}

	// MBC30 has three RAM bank outputs, supporting up to 64 KiB static RAM
	if (cartheader::RAM_SIZE_64K == header[cartheader::OFFSET_RAM_SIZE - 0x100])
	{
		osd_printf_verbose(
				"[%s] Assuming cartridge declaring MBC3 controller with 64 KiB RAM uses MBC30\n",
				tag);
		return true;
	}

	// MBC3 should be fine
	return false;
}


bool is_m161(std::string_view tag, util::random_read &file, u64 length, u64 offset, u8 const *header)
{
	// supports eight 32 KiB banks at most, doesn't make sense without at least two banks
	if (((u32(0x8000) << 3) < length) || (0x8000 >= length) || (length & 0x7fff))
		return false;

	// doesn't support banked RAM
	u8 const ramsize(header[cartheader::OFFSET_RAM_SIZE - 0x100]);
	switch (ramsize)
	{
	case cartheader::RAM_SIZE_0K:
	case cartheader::RAM_SIZE_2K:
	case cartheader::RAM_SIZE_8K:
		break;
	default:
		return false;
	}

	// check header checksum for the first page
	if (!cartheader::verify_header_checksum(header))
		return false;

	// there's one known cartridge using this scheme
	static constexpr u8 KNOWN_COLLECTIONS[][0x10] = {
			{ 'T', 'E', 'T', 'R', 'I', 'S', ' ', 'S', 'E', 'T', 0,   0,   0,   0,   0,   0 } };
	auto const known(
			std::find_if(
				std::begin(KNOWN_COLLECTIONS),
				std::end(KNOWN_COLLECTIONS),
				[header] (auto const &name)
				{
					return std::equal(std::begin(name), std::end(name), &header[cartheader::OFFSET_TITLE - 0x100]);
				}));
	if (std::end(KNOWN_COLLECTIONS) != known)
	{
		osd_printf_verbose("[%s] Detected known M161 game title\n", tag);
		return true;
	}

	// look for a valid header in the second page
	u8 header2[0x50];
	auto const [err, actual] = read_at(file, offset + 0x8000 + 0x0100, header2, sizeof(header2));
	if (err || (sizeof(header2) != actual) || !cartheader::verify_header_checksum(header2))
		return false;

	// make sure it fits with the cartridge RAM size declaration
	switch (header2[cartheader::OFFSET_TYPE - 0x100])
	{
	case cartheader::TYPE_ROM:
		osd_printf_verbose("[%s] Detected valid plain ROM game header at 0x8000, assuming M161 cartridge\n", tag);
		return true;
	case cartheader::TYPE_ROM_RAM:
	case cartheader::TYPE_ROM_RAM_BATT:
		switch (header[cartheader::OFFSET_RAM_SIZE - 0x100])
		{
		case cartheader::RAM_SIZE_2K:
			if (cartheader::RAM_SIZE_0K != ramsize)
			{
				osd_printf_verbose(
						"[%s] Detected valid plain ROM + 2 KiB RAM game header at 0x8000, assuming M161 cartridge\n",
						tag);
				return true;
			}
			return false;
		case cartheader::RAM_SIZE_8K:
			if (cartheader::RAM_SIZE_8K == ramsize)
			{
				osd_printf_verbose(
						"[%s] Detected valid plain ROM + 8 KiB RAM game header at 0x8000, assuming M161 cartridge\n",
						tag);
				return true;
			}
			return false;
		}
		return false;
	default:
		return false;
	}
}


char const *guess_mbc7_type(std::string_view tag, util::random_read &file, u64 length, u64 offset, u8 const *header)
{
	// there are only two games using MBC7 - by coincidence, the larger program requires the larger EEPROM
	if (0x100000 < length)
	{
		osd_printf_verbose("[%s] Assuming 0x%06X-byte MBC7 program expects 4 Kibit 93LC66 EEPROM\n", tag, length);
		return slotoptions::GB_MBC7_4K;
	}
	else
	{
		osd_printf_verbose("[%s] Assuming 0x%06X-byte MBC7 program expects 2 Kibit 93LC56 EEPROM\n", tag, length);
		return slotoptions::GB_MBC7_2K;
	}
}


bool read_gbx_footer_trailer(util::random_read &file, u64 length, gbxfile::trailer &trailer)
{
	if (sizeof(trailer) >= length)
		return false;

	auto const [err, actual] = read_at(file, length - sizeof(trailer), &trailer, sizeof(trailer));
	if (err || (sizeof(trailer) != actual))
		return false;

	trailer.swap();
	return (gbxfile::MAGIC_GBX == trailer.magic) && (length >= trailer.size);
}


std::optional<char const *> probe_gbx_footer(std::string_view tag, util::random_read &file, u64 &length, u64 &offset)
{
	// first try to read the trailer and look for a recognised version
	gbxfile::trailer trailer;
	if (!read_gbx_footer_trailer(file, length, trailer))
		return std::nullopt;
	else if ((1 != trailer.ver_maj) || ((sizeof(trailer) + sizeof(gbxfile::leader_1_0)) > trailer.size))
		return std::nullopt;

	// need to read the footer leader
	std::error_condition err;
	size_t actual;
	gbxfile::leader_1_0 leader;
	std::tie(err, actual) = read_at(file, length - trailer.size, &leader, sizeof(leader));
	if (err || (sizeof(leader) != actual))
	{
		osd_printf_warning(
				"[%s] Error reading GBX trailer leader - assuming file is not GBX format\n",
				tag);
		return std::nullopt;
	}
	leader.swap();

	// if the ROM would overlap the footer, assume it isn't GBX format
	if ((length - trailer.size) < leader.rom_bytes)
		return std::nullopt;

	// if we got this far, trust the program ROM size declared in the footer
	osd_printf_verbose(
			"[%s] Found GBX footer version %u.%u declaring 0x%06X-byte ROM\n",
			tag,
			trailer.ver_maj,
			trailer.ver_min,
			leader.rom_bytes);
	length = leader.rom_bytes;
	offset = 0;

	// map known GBX cartridge types to slot options
	char const *result = nullptr;
	switch (leader.cart_type)
	{
	case gbxfile::TYPE_PLAIN:
		result = slotoptions::GB_STD;
		break;
	case gbxfile::TYPE_M161:
		result = slotoptions::GB_M161;
		break;
	case gbxfile::TYPE_TAMA5:
		result = slotoptions::GB_TAMA5;
		break;
	case gbxfile::TYPE_WISDOM:
		result = slotoptions::GB_WISDOM;
		break;
	case gbxfile::TYPE_SACHEN1:
		result = slotoptions::GB_SACHEN1;
		break;
	case gbxfile::TYPE_SACHEN2:
		result = slotoptions::GB_SACHEN2;
		break;
	case gbxfile::TYPE_ROCKET:
		result = slotoptions::GB_ROCKET;
		break;
	case gbxfile::TYPE_MBC1:
	case gbxfile::TYPE_MBC1_COLL:
		result = slotoptions::GB_MBC1;
		break;
	case gbxfile::TYPE_MBC2:
		result = slotoptions::GB_MBC2;
		break;
	case gbxfile::TYPE_MBC3:
		if (((u32(0x4000) << 7) < leader.rom_bytes) || ((u32(0x2000) << 2) < leader.ram_bytes))
			result = slotoptions::GB_MBC30;
		else
			result = slotoptions::GB_MBC3;
		break;
	case gbxfile::TYPE_MBC5:
		result = slotoptions::GB_MBC5;
		break;
	case gbxfile::TYPE_MBC6:
		result = slotoptions::GB_MBC6;
		break;
	case gbxfile::TYPE_MBC7:
		{
			// need to probe for EEPROM size
			// TODO: does the GBX footer declare the EEPROM size as cartridge RAM size?
			u8 header[0x50];
			std::tie(err, actual) = read_at(file, offset + 0x100, header, sizeof(header));
			if (!err && (sizeof(header) == actual))
			{
				result = guess_mbc7_type(tag, file, length, offset, header);
			}
			else
			{
				osd_printf_warning(
						"[%s] Error reading cartridge header - defaulting to 93LC66 EEPROM for MBC7\n",
						tag);
				result = slotoptions::GB_MBC7_4K;
			}
		}
		break;
	case gbxfile::TYPE_MMM01:
		result = slotoptions::GB_MMM01;
		break;
	case gbxfile::TYPE_CAMERA:
		result = slotoptions::GB_CAMERA;
		break;
	case gbxfile::TYPE_HUC1:
		result = slotoptions::GB_HUC1;
		break;
	case gbxfile::TYPE_HUC3:
		result = slotoptions::GB_HUC3;
		break;
	case gbxfile::TYPE_TFANGBOOT:
		result = slotoptions::GB_TFANGBOOT;
		break;
	case gbxfile::TYPE_BBD:
		result = slotoptions::GB_BBD;
		break;
	case gbxfile::TYPE_DSHGGB81:
		result = slotoptions::GB_DSHGGB81;
		break;
	case gbxfile::TYPE_SINTAX:
		result = slotoptions::GB_SINTAX;
		break;
	case gbxfile::TYPE_LICHENG:
		result = slotoptions::GB_LICHENG;
		break;
	case gbxfile::TYPE_NEWGBCHK:
		result = slotoptions::GB_NEWGBCHK;
		break;
	case gbxfile::TYPE_VF001:
		result = slotoptions::GB_VF001;
		break;
	case gbxfile::TYPE_LIEBAO:
		result = slotoptions::GB_LIEBAO;
		break;
	case gbxfile::TYPE_NTNEW:
		result = slotoptions::GB_NTNEW;
		break;
	case gbxfile::TYPE_SLMULTI:
		result = slotoptions::GB_SLMULTI;
		break;
	}
	if (result)
	{
		osd_printf_verbose(
				"[%s] GBX cartridge type 0x%08X mapped to slot option '%s'\n",
				tag,
				leader.cart_type,
				result);
	}
	else
	{
		osd_printf_verbose(
				"[%s] Unknown GBX cartridge type 0x%08X\n",
				tag,
				leader.cart_type);
	}
	return result;
}


char const *guess_image_format(std::string_view tag, util::random_read &file, u64 &length, u64 &offset)
{
	// probe for GBX format - this can set the length and offset while still failing to select a slot option
	std::optional<char const *> const gbxoption = probe_gbx_footer(tag, file, length, offset);
	if (gbxoption)
		return *gbxoption;

	// some ROM images apparently have a 512-byte header
	if ((0x4000 < length) && ((length & 0x3fff) == 0x0200))
	{
		osd_printf_verbose("[%s] Assuming 0x%06X-byte cartridge file contains 0x0200-byte header\n", tag, length);
		length -= 0x2000;
		offset = 0x0200;
		return nullptr;
	}

	// assume it's a common .gb plain ROM image file
	osd_printf_verbose("[%s] Assuming 0x%06X-byte cartridge file is a plain ROM image\n", tag, length);
	offset = 0;
	return nullptr;
}


bool detect_mmm01(std::string_view tag, util::random_read &file, u64 length, u64 offset, u8 const *header)
{
	// MMM01 cartridges must be a multiple of 16 KiB no larger than 8 MiB
	if ((length & 0x3fff) || ((u32(0x4000) << 9) < length))
		return false;

	// now check for a valid cartridge header
	u32 const lastpage(get_mmm01_initial_low_page(length));
	u8 backheader[0x50];
	auto const [err, actual] = read_at(file, lastpage + 0x100, backheader, sizeof(backheader));
	if (err || (sizeof(backheader) != actual))
	{
		osd_printf_warning(
				"[%s] Error reading last page of program ROM - assuming cartridge does not use MMM01 controller\n",
				tag);
		return false;
	}
	if (!cartheader::verify_header_checksum(backheader))
		return false;


	// well-behaved game that declares MMM01 here
	u8 const backtype(backheader[cartheader::OFFSET_TYPE - 0x100]);
	switch (backtype)
	{
	case cartheader::TYPE_MMM01:
	case cartheader::TYPE_MMM01_RAM:
	case cartheader::TYPE_MMM01_RAM_BATT:
		osd_printf_verbose(
				"[%s] Found MMM01 header at 0x%06X, assuming cartridge uses MMM01 header\n",
				tag,
				lastpage);
		return true;
	case cartheader::TYPE_MBC3:
		{
			static constexpr u8 KNOWN_COLLECTIONS[][0x10] = {
					{ 'B', 'O', 'U', 'K', 'E', 'N', 'J', 'I', 'M', 'A', '2', ' ', 'S', 'E', 'T', 0 },
					{ 'B', 'U', 'B', 'B', 'L', 'E', 'B', 'O', 'B', 'B', 'L', 'E', ' ', 'S', 'E', 'T' },
					{ 'G', 'A', 'N', 'B', 'A', 'R', 'U', 'G', 'A', ' ', 'S', 'E', 'T', 0,   0,   0 },
					{ 'R', 'T', 'Y', 'P', 'E', ' ', '2', ' ', 'S', 'E', 'T', 0,   0,   0,   0,   0 } };
			auto const known(
					std::find_if(
						std::begin(KNOWN_COLLECTIONS),
						std::end(KNOWN_COLLECTIONS),
						[&backheader] (auto const &name)
						{
							return std::equal(std::begin(name), std::end(name), &backheader[cartheader::OFFSET_TITLE - 0x100]);
						}));
			if (std::end(KNOWN_COLLECTIONS) != known)
			{
				osd_printf_verbose("[%s] Detected known MMM01 game title\n", tag);
				return true;
			}
		}
		break;
	}

	// not a recognised MMM01 game
	return false;
}


char const *guess_cart_type(std::string_view tag, util::random_read &file, u64 length, u64 offset, u8 const *header)
{
	u8 const carttype = header[cartheader::OFFSET_TYPE - 0x0100];
	switch (carttype)
	{
	case cartheader::TYPE_ROM:
		if (is_wisdom_tree(tag, file, length, offset, header))
			return slotoptions::GB_WISDOM;
		return slotoptions::GB_STD;
	case cartheader::TYPE_MBC1:
	case cartheader::TYPE_MBC1_RAM:
	case cartheader::TYPE_MBC1_RAM_BATT:
		return slotoptions::GB_MBC1;
	// 0x04
	case cartheader::TYPE_MBC2:
	case cartheader::TYPE_MBC2_BATT:
		return slotoptions::GB_MBC2;
	case cartheader::TYPE_ROM_RAM:
	case cartheader::TYPE_ROM_RAM_BATT:
		return slotoptions::GB_STD;
	// 0x09
	// 0x0a
	case cartheader::TYPE_MMM01:
	case cartheader::TYPE_MMM01_RAM:
	case cartheader::TYPE_MMM01_RAM_BATT:
		return slotoptions::GB_MMM01;
	// 0x0e
	case cartheader::TYPE_MBC3_RTC_BATT:
		if (is_mbc30(tag, file, length, offset, header))
			return slotoptions::GB_MBC30;
		return slotoptions::GB_MBC3;
	case cartheader::TYPE_MBC3_RTC_RAM_BATT:
		if (is_m161(tag, file, length, offset, header))
			return slotoptions::GB_M161;
		else if (is_mbc30(tag, file, length, offset, header))
			return slotoptions::GB_MBC30;
		return slotoptions::GB_MBC3;
	case cartheader::TYPE_MBC3:
	case cartheader::TYPE_MBC3_RAM:
	case cartheader::TYPE_MBC3_RAM_BATT:
		if (is_mbc30(tag, file, length, offset, header))
			return slotoptions::GB_MBC30;
		return slotoptions::GB_MBC3;
	// 0x14
	case cartheader::TYPE_MBC5:
	case cartheader::TYPE_MBC5_RAM:
	case cartheader::TYPE_MBC5_RAM_BATT:
	case cartheader::TYPE_MBC5_RUMBLE:
	case cartheader::TYPE_MBC5_RUMBLE_RAM:
	case cartheader::TYPE_MBC5_RUMBLE_RAM_BATT:
		return slotoptions::GB_MBC5;
	// 0x1f
	case cartheader::TYPE_MBC6:
		return slotoptions::GB_MBC6;
	// 0x21
	case cartheader::TYPE_MBC7_ACCEL_EEPROM:
		return guess_mbc7_type(tag, file, length, offset, header);

	case cartheader::TYPE_UNLICENSED_YONGYONG:
		return slotoptions::GB_YONG;

	case cartheader::TYPE_CAMERA:
		return slotoptions::GB_CAMERA;
	case cartheader::TYPE_TAMA5:
		return slotoptions::GB_TAMA5;
	case cartheader::TYPE_HUC3:
		return slotoptions::GB_HUC3;
	case cartheader::TYPE_HUC1_RAM_BATT:
		return slotoptions::GB_HUC1;
	}

	if (0x8000 >= length)
	{
		osd_printf_warning(
				"[%s] Unknown cartridge type 0x%02X - defaulting to flat ROM for 0x%04X-byte cartridge\n",
				tag,
				carttype,
				length);
		return slotoptions::GB_STD;
	}
	else if ((u32(0x4000) << 7) >= length)
	{
		osd_printf_warning(
				"[%s] Unknown cartridge type 0x%02X - defaulting to MBC1 for 0x%06X-byte cartridge\n",
				tag,
				carttype,
				length);
		return slotoptions::GB_MBC1;
	}
	else
	{
		osd_printf_warning(
				"[%s] Unknown cartridge type 0x%02X - defaulting to MBC5 for 0x%06X-byte cartridge\n",
				tag,
				carttype,
				length);
		return slotoptions::GB_MBC5;
	}
}

} // anonymous namespace

} // namespace bus::gameboy



//**************************************************************************
//  gb_cart_slot_device
//**************************************************************************

gb_cart_slot_device::gb_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	gb_cart_slot_device_base(mconfig, GB_CART_SLOT, tag, owner, clock)
{
}


std::string gb_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	using namespace bus::gameboy;

	if (hook.image_file())
	{
		u64 len;
		if (hook.image_file()->length(len))
		{
			// if we can't get the file, use MBC1 - it's a common mapper and also works with flat ROMs
			osd_printf_warning("[%s] Error getting cartridge ROM length - defaulting to MBC1\n", tag());
			return slotoptions::GB_MBC1;
		}

		u64 offset;
		char const *const guess(guess_image_format(tag(), *hook.image_file(), len, offset));
		if (guess)
			return guess;

		u8 header[0x50];
		auto const [err, actual] = read_at(*hook.image_file(), offset + 0x0100, header, sizeof(header));
		if (err || (sizeof(header) != actual))
		{
			// reading header failed - guess based on size
			if (0x8000 >= len)
			{
				osd_printf_warning(
						"[%s] Error reading cartridge header - defaulting to flat ROM for 0x%04X-byte cartridge\n",
						tag(),
						len);
				return slotoptions::GB_STD;
			}
			else if ((u32(0x4000) << 7) >= len)
			{
				osd_printf_warning(
						"[%s] Error reading cartridge header - defaulting to MBC1 for 0x%06X-byte cartridge\n",
						tag(),
						len);
				return slotoptions::GB_MBC1;
			}
			else
			{
				osd_printf_warning(
						"[%s] Error reading cartridge header - defaulting to MBC5 for 0x%06X-byte cartridge\n",
						tag(),
						len);
				return slotoptions::GB_MBC5;
			}
		}

		// MMM01 controller doesn't initially map the first page
		if (detect_mmm01(tag(), *hook.image_file(), len, offset, header))
			return slotoptions::GB_MMM01;
		else
			return guess_cart_type(tag(), *hook.image_file(), len, offset, header);
	}

	// this will return the explicit setting for a software list item
	return software_get_default_slot(slotoptions::GB_STD);
}


void gb_cart_slot_device::device_reset_after_children()
{
	using namespace bus::gameboy;

	gb_cart_slot_device_base::device_reset_after_children();

	// read the header as seen by the system on reset
	u8 header[0x50];
	{
		auto const suppressor = machine().disable_side_effects();
		for (unsigned i = 0; std::size(header) > i; ++i)
			header[i] = cart_space().read_byte(0x100 + i);
	}

	// get some fields that are used multiple times
	u8 const cgb = header[cartheader::OFFSET_CGB - 0x100];
	u8 const type = header[cartheader::OFFSET_TYPE - 0x100];
	u8 const romsize = header[cartheader::OFFSET_ROM_SIZE - 0x100];
	u8 const ramsize = header[cartheader::OFFSET_RAM_SIZE - 0x100];
	u8 const region = header[cartheader::OFFSET_REGION - 0x100];
	u8 const headerchecksum = header[cartheader::OFFSET_CHECKSUM_HEADER - 0x100];
	u16 const romchecksum = (u16(header[cartheader::OFFSET_CHECKSUM_ROM - 0x100]) << 8) | header[cartheader::OFFSET_CHECKSUM_ROM + 1 - 0x100];

	// calculate checksums
	u8 const headercalc = cartheader::checksum(
			&header[cartheader::OFFSET_TITLE - 0x100],
			&header[cartheader::OFFSET_CHECKSUM_HEADER - 0x100]);
	u16 romcalc = 0U - (romchecksum >> 8) - (romchecksum & 0x00ff);
	memory_region *const romregion = memregion("rom");
	if (romregion)
	{
		u8 const *const data = &romregion->as_u8();
		romcalc = std::accumulate(&data[0], &data[romregion->bytes()], romcalc);
	}

	// log parsed header for informational purposes
	logerror(
			"Cartridge header information:\n"
			" * Entry point:     %s\n"
			" * Title:           %s\n"
			" * Game Boy Color:  %s\n"
			" * Super Game Boy:  0x%02X (%s)\n"
			" * Type:            0x%02X (%s)\n"
			" * ROM size:        0x%02X (%s)\n"
			" * RAM size:        0x%02X (%s)\n"
			" * Region:          0x%02X (%s)\n"
			" * Licensee:        %s\n"
			" * Revision:        0x%02X\n"
			" * Header checksum: 0x%02X (calculated 0x%02X%s)\n"
			" * ROM checksum:    0x%04X (calculated 0x%04X%s)\n",
			disassemble_entry_point(header),
			cart_title_text(header),
			cart_cgb_support_description(cgb),
			header[cartheader::OFFSET_SGB - 0x100],
			cart_sgb_support_description(header),
			type,
			cart_type_description(type),
			romsize,
			cart_rom_size_description(romsize, romregion),
			ramsize,
			cart_ram_size_description(ramsize),
			region,
			cart_region_description(region),
			cart_licensee_description(header),
			header[cartheader::OFFSET_REVISION - 0x100],
			headerchecksum,
			headercalc,
			(headerchecksum != headercalc) ? " MISMATCH!" : "",
			romchecksum,
			romcalc,
			(romchecksum != romcalc) ? " MISMATCH!" : "");
}


std::pair<std::error_condition, std::string> gb_cart_slot_device::load_image_file(util::random_read &file)
{
	using namespace bus::gameboy;

	bool proberam = true;
	auto len = length();
	u64 offset;

	// probe for GBX format
	memory_region *gbxregion = nullptr;
	gbxfile::trailer gbxtrailer;
	if (read_gbx_footer_trailer(file, len, gbxtrailer))
	{
		// try reading the GBX footer into temporary space before more checks
		std::unique_ptr<u8 []> const footer(new (std::nothrow) u8 [gbxtrailer.size]);
		if (!footer)
			return std::make_pair(std::errc::not_enough_memory, "Error allocating memory to read GBX file footer");
		auto const [err, actual] = read_at(file, len - gbxtrailer.size, footer.get(), gbxtrailer.size);
		if (err || (gbxtrailer.size != actual))
			return std::make_pair(err ? err : std::errc::io_error, "Error reading GBX file footer");
		if (1 != gbxtrailer.ver_maj)
		{
			// some unsupported GBX version - assume footer immediately follows ROM
			osd_printf_verbose(
					"[%s] Found found unsupported GBX file footer version %u.%u (%u bytes)\n",
					tag(),
					gbxtrailer.ver_maj,
					gbxtrailer.ver_min,
					gbxtrailer.size);
			LOG("Allocating %u byte GBX file footer region\n", gbxtrailer.size);
			gbxregion = machine().memory().region_alloc(subtag("gbx"), gbxtrailer.size, 1, ENDIANNESS_BIG);
			std::memcpy(gbxregion->base(), footer.get(), gbxtrailer.size);
			offset = 0;
			len -= gbxtrailer.size;
		}
		else if ((sizeof(gbxfile::leader_1_0) + sizeof(gbxtrailer)) > gbxtrailer.size)
		{
			osd_printf_verbose(
					"[%s] %u-byte GBX footer is too small to contain %u-byte leader and %u-byte trailer, assuming file is not GBX format\n",
					tag(),
					gbxtrailer.size,
					sizeof(gbxfile::leader_1_0),
					sizeof(gbxtrailer));
		}
		else
		{
			// ensure ROM doesn't overlap leader
			gbxfile::leader_1_0 leader;
			std::memcpy(&leader, footer.get(), sizeof(leader));
			leader.swap();
			if ((len - gbxtrailer.size) < leader.rom_bytes)
			{
				osd_printf_verbose(
						"[%s] GBX footer specifies 0x%06X-byte ROM which exceeds 0x%06X-byte content, assuming file is not GBX format\n",
						tag(),
						leader.rom_bytes,
						len - gbxtrailer.size);
			}
			else
			{
				// looks like supported GBX format
				osd_printf_verbose(
						"[%s] Found GBX file footer version %u.%u (%u bytes):\n"
						" * %u bytes ROM\n"
						" * %u bytes RAM\n"
						"%s"
						"%s"
						"%s",
						tag(),
						gbxtrailer.ver_maj,
						gbxtrailer.ver_min,
						gbxtrailer.size,
						leader.rom_bytes,
						leader.ram_bytes,
						leader.batt ? " * backup battery\n" : "",
						leader.rumble ? " * vibration motor\n" : "",
						leader.rtc ? " * real-time clock oscillator\n" : "");
				proberam = false;
				offset = 0;
				len = leader.rom_bytes;

				// copy the footer in case the cart needs it
				LOG("Allocating %u byte GBX file footer region\n", gbxtrailer.size);
				gbxregion = machine().memory().region_alloc(subtag("gbx"), gbxtrailer.size, 1, ENDIANNESS_BIG);
				std::memcpy(gbxregion->base(), footer.get(), gbxtrailer.size);

				// allocate RAM if appropriate
				if (leader.ram_bytes)
				{
					LOG(
							"Allocating %u byte cartridge %sRAM region\n",
							leader.ram_bytes,
							leader.batt ? "non-volatile " : "");
					machine().memory().region_alloc(subtag(leader.batt ? "nvram" : "ram"), leader.ram_bytes, 1, ENDIANNESS_LITTLE);
				}
			}
		}
	}

	// if it doesn't appear to be GBX format, check for apparent 512-byte header
	if (!gbxregion)
	{
		if ((0x4000 < len) && ((len & 0x3fff) == 0x0200))
		{
			osd_printf_verbose("[%s] Assuming 0x%06X-byte cartridge file contains 0x0200-byte header\n", tag(), len);
			len -= 0x2000;
			offset = 0x0200;
		}
		else
		{
			offset = 0;
		}
	}

	// load program ROM
	if (len)
	{
		LOG("Allocating %u byte cartridge ROM region\n", len);
		memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), len, 1, ENDIANNESS_LITTLE);
		auto const [err, actual] = read_at(file, offset, romregion->base(), len);
		if (err || (len != actual))
			return std::make_pair(err ? err : std::errc::io_error, "Error reading ROM data from cartridge file");

		// allocate cartridge RAM based on header if necessary
		if (proberam)
		{
			u8 const *const rombase = &romregion->as_u8();
			u32 basepage = 0;
			if (get_card_device()->device().type() == GB_ROM_MMM01)
				basepage = get_mmm01_initial_low_page(len);
			if ((basepage + cartheader::OFFSET_CHECKSUM_ROM) <= len)
				allocate_cart_ram(&rombase[basepage]);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}


void gb_cart_slot_device::allocate_cart_ram(u8 const *basepage)
{
	using namespace bus::gameboy;

	if (cartheader::verify_header_checksum(&basepage[0x100]))
	{
		osd_printf_verbose("[%s] Detecting cartridge RAM size/type from header\n", tag());
		bool nonvolatile;
		u8 const carttype = basepage[cartheader::OFFSET_TYPE];
		switch (carttype)
		{
		case cartheader::TYPE_MBC1_RAM_BATT:
		case cartheader::TYPE_MBC2_BATT:
		case cartheader::TYPE_ROM_RAM_BATT:
		case cartheader::TYPE_MMM01_RAM_BATT:
		case cartheader::TYPE_MBC3_RTC_BATT:
		case cartheader::TYPE_MBC3_RTC_RAM_BATT:
		case cartheader::TYPE_MBC3_RAM_BATT:
		case cartheader::TYPE_MBC5_RAM_BATT:
		case cartheader::TYPE_MBC5_RUMBLE_RAM_BATT:
		case cartheader::TYPE_CAMERA:
		case cartheader::TYPE_TAMA5:
		case cartheader::TYPE_HUC3:
		case cartheader::TYPE_HUC1_RAM_BATT:
			osd_printf_verbose(
					"[%s] Cartridge type 0x%02X assumed to preserve cartridge RAM contents\n",
					tag(),
					carttype);
			nonvolatile = true;
			break;
		default:
			nonvolatile = false;
		}

		u32 rambytes;
		if ((cartheader::TYPE_MBC2 == carttype) || (cartheader::TYPE_MBC2_BATT == carttype))
		{
			osd_printf_verbose(
					"[%s] Cartridge type 0x%02X has internal 512 nybble static RAM\n",
					tag(),
					carttype);
			rambytes = 512;
		}
		else
		{
			u8 const ramsize = basepage[cartheader::OFFSET_RAM_SIZE];
			switch (ramsize)
			{
			default:
				osd_printf_warning("[%s] Cartridge declares unknown RAM size 0x%02X\n", tag(), ramsize);
				[[fallthrough]];
			case cartheader::RAM_SIZE_0K:
				rambytes = 0;
				break;
			case cartheader::RAM_SIZE_2K:
				rambytes = 2 * 1024;
				break;
			case cartheader::RAM_SIZE_8K:
				rambytes = 8 * 1024;
				break;
			case cartheader::RAM_SIZE_32K:
				rambytes = 32 * 1024;
				break;
			case cartheader::RAM_SIZE_128K:
				rambytes = 128 * 1024;
				break;
			case cartheader::RAM_SIZE_64K:
				rambytes = 64 * 1024;
				break;
			}
			if (rambytes)
			{
				osd_printf_verbose(
						"[%s] Cartridge declares RAM size 0x%02X corresponding to %u bytes\n",
						tag(),
						ramsize,
						rambytes);
			}
		}

		if (rambytes)
		{
			LOG(
					"Allocating %u byte cartridge %sRAM region\n",
					rambytes,
					nonvolatile ? "non-volatile " : "");
			machine().memory().region_alloc(subtag(nonvolatile ? "nvram" : "ram"), rambytes, 1, ENDIANNESS_LITTLE);
		}
	}
}



//**************************************************************************
//  Device type definitions
//**************************************************************************

DEFINE_DEVICE_TYPE(GB_CART_SLOT, gb_cart_slot_device, "gb_cart_slot", "Game Boy Cartridge Slot")
