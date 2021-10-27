// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    language.cpp

    Multi-language support.

***************************************************************************/

#include "emu.h"
#include "language.h"

#include "emuopts.h"

#include "corestr.h"

#include <cstring>
#include <memory>
#include <new>
#include <unordered_map>
#include <utility>


namespace {

constexpr u32 MO_MAGIC = 0x950412de;
constexpr u32 MO_MAGIC_REVERSED = 0xde120495;

std::unique_ptr<u32 []> f_translation_data;
std::unordered_map<std::string_view, std::pair<char const *, u32> > f_translation_map;

} // anonymous namespace


void load_translation(emu_options &m_options)
{
	f_translation_data.reset();
	f_translation_map.clear();

	std::string name = m_options.language();
	if (name.empty())
		return;

	strreplace(name, " ", "_");
	strreplace(name, "(", "");
	strreplace(name, ")", "");
	emu_file file(m_options.language_path(), OPEN_FLAG_READ);
	if (file.open(name + PATH_SEPARATOR "strings.mo"))
	{
		osd_printf_error("Error opening translation file %s\n", name);
		return;
	}

	u64 const size = file.size();
	if (20 > size)
	{
		file.close();
		osd_printf_error("Error reading translation file %s: %u-byte file is too small to contain translation data\n", name, size);
		return;
	}

	f_translation_data.reset(new (std::nothrow) u32 [(size + 3) / 4]);
	if (!f_translation_data)
	{
		file.close();
		osd_printf_error("Failed to allocate %u bytes to load translation data file %s\n", size, name);
		return;
	}

	auto const read = file.read(f_translation_data.get(), size);
	file.close();
	if (read != size)
	{
		osd_printf_error("Error reading translation file %s: requested %u bytes but got %u bytes\n", name, size, read);
		f_translation_data.reset();
		return;
	}

	if ((f_translation_data[0] != MO_MAGIC) && (f_translation_data[0] != MO_MAGIC_REVERSED))
	{
		osd_printf_error("Error reading translation file %s: unrecognized magic number 0x%08X\n", name, f_translation_data[0]);
		f_translation_data.reset();
		return;
	}

	auto fetch_word =
			[reversed = f_translation_data[0] == MO_MAGIC_REVERSED, words = f_translation_data.get()] (size_t offset)
			{
				return reversed ? swapendian_int32(words[offset]) : words[offset];
			};

	// FIXME: check major/minor version number

	if ((fetch_word(3) % 4) || (fetch_word(4) % 4))
	{
		osd_printf_error("Error reading translation file %s: table offsets %u and %u are not word-aligned\n", name, fetch_word(3), fetch_word(4));
		f_translation_data.reset();
		return;
	}

	u32 const number_of_strings = fetch_word(2);
	u32 const original_table_offset = fetch_word(3) >> 2;
	u32 const translation_table_offset = fetch_word(4) >> 2;
	if ((4 * (original_table_offset + (u64(number_of_strings) * 2))) > size)
	{
		osd_printf_error("Error reading translation file %s: %u-entry original string table at offset %u extends past end of %u-byte file\n", name, number_of_strings, fetch_word(3), size);
		f_translation_data.reset();
		return;
	}
	if ((4 * (translation_table_offset + (u64(number_of_strings) * 2))) > size)
	{
		osd_printf_error("Error reading translation file %s: %u-entry translated string table at offset %u extends past end of %u-byte file\n", name, number_of_strings, fetch_word(4), size);
		f_translation_data.reset();
		return;
	}
	osd_printf_verbose("Reading translation file %s: %u strings, original table at word offset %u, translated table at word offset %u\n", name, number_of_strings, original_table_offset, translation_table_offset);

	char const *const data = reinterpret_cast<char const *>(f_translation_data.get());
	for (u32 i = 1; number_of_strings > i; ++i)
	{
		u32 const original_length = fetch_word(original_table_offset + (2 * i));
		u32 const original_offset = fetch_word(original_table_offset + (2 * i) + 1);
		if ((original_length + original_offset) >= size)
		{
			osd_printf_error("Error reading translation file %s: %u-byte original string %u at offset %u extends past end of %u-byte file\n", name, original_length, i, original_offset, size);
			continue;
		}
		if (data[original_length + original_offset])
		{
			osd_printf_error("Error reading translation file %s: %u-byte original string %u at offset %u is not correctly NUL-terminated\n", name, original_length, i, original_offset);
			continue;
		}

		u32 const translation_length = fetch_word(translation_table_offset + (2 * i));
		u32 const translation_offset = fetch_word(translation_table_offset + (2 * i) + 1);
		if ((translation_length + translation_offset) >= size)
		{
			osd_printf_error("Error reading translation file %s: %u-byte translated string %u at offset %u extends past end of %u-byte file\n", name, translation_length, i, translation_offset, size);
			continue;
		}
		if (data[translation_length + translation_offset])
		{
			osd_printf_error("Error reading translation file %s: %u-byte translated string %u at offset %u is not correctly NUL-terminated\n", name, translation_length, i, translation_offset);
			continue;
		}

		std::string_view const original(&data[original_offset], original_length);
		char const *const translation(&data[translation_offset]);
		auto const ins = f_translation_map.emplace(original, std::make_pair(translation, translation_length));
		if (!ins.second)
		{
			osd_printf_warning(
					"Loading translation file %s: translation %u '%s'='%s' conflicts with previous translation '%s'='%s'\n",
					name,
					i,
					original,
					translation,
					ins.first->first,
					ins.first->second.first);
		}
	}

	osd_printf_verbose("Loaded %u translations from file %s\n", f_translation_map.size(), name);
}


char const *lang_translate(char const *message)
{
	auto const found = f_translation_map.find(message);
	if (f_translation_map.end() != found)
		return found->second.first;
	return message;
}


std::string_view lang_translate(std::string_view message)
{
	auto const found = f_translation_map.find(message);
	if (f_translation_map.end() != found)
		return std::string_view(found->second.first, found->second.second);
	return message;
}


char const *lang_translate(char const *context, char const *message)
{
	if (!f_translation_map.empty())
	{
		auto const ctxlen(std::strlen(context));
		auto const msglen(std::strlen(message));
		std::string key;
		key.reserve(ctxlen + 1 + msglen);
		key.append(context, ctxlen);
		key.append(1, '\004');
		key.append(message, msglen);
		auto const found = f_translation_map.find(key);
		if (f_translation_map.end() != found)
			return found->second.first;
	}
	return message;
}


std::string_view lang_translate(char const *context, std::string_view message)
{
	return lang_translate(std::string_view(context), message);
}


std::string_view lang_translate(std::string_view context, std::string_view message)
{
	if (!f_translation_map.empty())
	{
		std::string key;
		key.reserve(context.length() + 1 + message.length());
		key.append(context);
		key.append(1, '\004');
		key.append(message);
		auto const found = f_translation_map.find(key);
		if (f_translation_map.end() != found)
			return std::string_view(found->second.first, found->second.second);
	}
	return message;
}
