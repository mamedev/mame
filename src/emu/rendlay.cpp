// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    rendlay.cpp

    Core rendering layout parser and manager.

***************************************************************************/

#include "emu.h"
#include "render.h"
#include "rendlay.h"

#include "emuopts.h"
#include "rendfont.h"
#include "rendutil.h"
#include "video/rgbutil.h"

#include "nanosvg.h"
#include "unicode.h"
#include "vecstream.h"
#include "xmlfile.h"

#include <cctype>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#define LOG_GROUP_BOUNDS_RESOLUTION (1U << 1)
#define LOG_INTERACTIVE_ITEMS       (1U << 2)
#define LOG_DISK_DRAW               (1U << 3)
#define LOG_IMAGE_LOAD              (1U << 4)

//#define VERBOSE (LOG_GROUP_BOUNDS_RESOLUTION | LOG_INTERACTIVE_ITEMS | LOG_DISK_DRAW | LOG_IMAGE_LOAD)
#define LOG_OUTPUT_FUNC osd_printf_verbose
#include "logmacro.h"



/***************************************************************************
    STANDARD LAYOUTS
***************************************************************************/

// screenless layouts
#include "noscreens.lh"

// dual screen layouts
#include "dualhsxs.lh"
#include "dualhovu.lh"
#include "dualhuov.lh"

// triple screen layouts
#include "triphsxs.lh"

// quad screen layouts
#include "quadhsxs.lh"


namespace {

//**************************************************************************
//  CONSTANTS
//**************************************************************************

constexpr int LAYOUT_VERSION = 2;

enum
{
	LINE_CAP_NONE = 0,
	LINE_CAP_START = 1,
	LINE_CAP_END = 2
};

constexpr layout_group::transform identity_transform{{ {{ 1.0F, 0.0F, 0.0F }}, {{ 0.0F, 1.0F, 0.0F }}, {{ 0.0F, 0.0F, 1.0F }} }};



//**************************************************************************
//  HELPERS
//**************************************************************************

inline void render_bounds_transform(render_bounds &bounds, layout_group::transform const &trans)
{
	bounds = render_bounds{
			(bounds.x0 * trans[0][0]) + (bounds.y0 * trans[0][1]) + trans[0][2],
			(bounds.x0 * trans[1][0]) + (bounds.y0 * trans[1][1]) + trans[1][2],
			(bounds.x1 * trans[0][0]) + (bounds.y1 * trans[0][1]) + trans[0][2],
			(bounds.x1 * trans[1][0]) + (bounds.y1 * trans[1][1]) + trans[1][2] };
}

inline void alpha_blend(u32 &dest, u32 a, u32 r, u32 g, u32 b, u32 inva)
{
	rgb_t const dpix(dest);
	u32 const da(dpix.a());
	u32 const finala((a * 255) + (da * inva));
	u32 const finalr(r + (u32(dpix.r()) * da * inva));
	u32 const finalg(g + (u32(dpix.g()) * da * inva));
	u32 const finalb(b + (u32(dpix.b()) * da * inva));
	dest = rgb_t(finala / 255, finalr / finala, finalg / finala, finalb / finala);
}

inline void alpha_blend(u32 &dest, render_color const &c, float fill)
{
	u32 const a(c.a * fill * 255.0F);
	if (a)
	{
		u32 const r(u32(c.r * (255.0F * 255.0F)) * a);
		u32 const g(u32(c.g * (255.0F * 255.0F)) * a);
		u32 const b(u32(c.b * (255.0F * 255.0F)) * a);
		alpha_blend(dest, a, r, g, b, 255 - a);
	}
}



//**************************************************************************
//  ERROR CLASSES
//**************************************************************************

class layout_syntax_error : public std::invalid_argument { using std::invalid_argument::invalid_argument; };
class layout_reference_error : public std::out_of_range { using std::out_of_range::out_of_range; };

} // anonymous namespace


namespace emu::render::detail {

class layout_environment
{
private:
	class entry
	{
	public:
		entry(std::string &&name, std::string &&t)
			: m_name(std::move(name))
			, m_text(std::move(t))
			, m_text_valid(true)
		{ }
		entry(std::string &&name, s64 i)
			: m_name(std::move(name))
			, m_int(i)
			, m_int_valid(true)
		{ }
		entry(std::string &&name, double f)
			: m_name(std::move(name))
			, m_float(f)
			, m_float_valid(true)
		{ }
		entry(std::string &&name, std::string &&t, s64 i, int s)
			: m_name(std::move(name))
			, m_text(std::move(t))
			, m_int_increment(i)
			, m_shift(s)
			, m_text_valid(true)
			, m_generator(true)
		{ }
		entry(std::string &&name, std::string &&t, double i, int s)
			: m_name(std::move(name))
			, m_text(std::move(t))
			, m_float_increment(i)
			, m_shift(s)
			, m_text_valid(true)
			, m_generator(true)
		{ }
		entry(entry &&) = default;
		entry &operator=(entry &&) = default;

		void set(std::string &&t)
		{
			m_text = std::move(t);
			m_text_valid = true;
			m_int_valid = false;
			m_float_valid = false;
		}
		void set(s64 i)
		{
			m_int = i;
			m_text_valid = false;
			m_int_valid = true;
			m_float_valid = false;
		}
		void set(double f)
		{
			m_float = f;
			m_text_valid = false;
			m_int_valid = false;
			m_float_valid = true;
		}

		std::string const &name() const { return m_name; }
		bool is_generator() const { return m_generator; }

		std::string const &get_text()
		{
			if (!m_text_valid)
			{
				if (m_float_valid)
				{
					m_text = std::to_string(m_float);
					m_text_valid = true;
				}
				else if (m_int_valid)
				{
					m_text = std::to_string(m_int);
					m_text_valid = true;
				}
			}
			return m_text;
		}

		void increment()
		{
			if (is_generator())
			{
				// apply increment
				if (m_float_increment)
				{
					if (m_int_valid && !m_float_valid)
					{
						m_float = m_int;
						m_float_valid = true;
					}
					if (m_text_valid && !m_float_valid)
					{
						std::istringstream stream(m_text);
						stream.imbue(std::locale::classic());
						if (m_text[0] == '$')
						{
							stream.get();
							u64 uvalue;
							stream >> std::hex >> uvalue;
							m_float = uvalue;
						}
						else if ((m_text[0] == '0') && ((m_text[1] == 'x') || (m_text[1] == 'X')))
						{
							stream.get();
							stream.get();
							u64 uvalue;
							stream >> std::hex >> uvalue;
							m_float = uvalue;
						}
						else if (m_text[0] == '#')
						{
							stream.get();
							stream >> m_int;
							m_float = m_int;
						}
						else
						{
							stream >> m_float;
						}
						m_float_valid = bool(stream);
					}
					m_float += m_float_increment;
					m_int_valid = m_text_valid = false;
				}
				else
				{
					if (m_text_valid && !m_int_valid && !m_float_valid)
					{
						std::istringstream stream(m_text);
						stream.imbue(std::locale::classic());
						if (m_text[0] == '$')
						{
							stream.get();
							u64 uvalue;
							stream >> std::hex >> uvalue;
							m_int = s64(uvalue);
							m_int_valid = bool(stream);
						}
						else if ((m_text[0] == '0') && ((m_text[1] == 'x') || (m_text[1] == 'X')))
						{
							stream.get();
							stream.get();
							u64 uvalue;
							stream >> std::hex >> uvalue;
							m_int = s64(uvalue);
							m_int_valid = bool(stream);
						}
						else if (m_text[0] == '#')
						{
							stream.get();
							stream >> m_int;
							m_int_valid = bool(stream);
						}
						else if (m_text.find_first_of(".eE") != std::string::npos)
						{
							stream >> m_float;
							m_float_valid = bool(stream);
						}
						else
						{
							stream >> m_int;
							m_int_valid = bool(stream);
						}
					}

					if (m_float_valid)
					{
						m_float += m_int_increment;
						m_int_valid = m_text_valid = false;
					}
					else
					{
						m_int += m_int_increment;
						m_float_valid = m_text_valid = false;
					}
				}

				// apply shift
				if (m_shift)
				{
					if (m_float_valid && !m_int_valid)
					{
						m_int = s64(m_float);
						m_int_valid = true;
					}
					if (m_text_valid && !m_int_valid)
					{
						std::istringstream stream(m_text);
						stream.imbue(std::locale::classic());
						if (m_text[0] == '$')
						{
							stream.get();
							u64 uvalue;
							stream >> std::hex >> uvalue;
							m_int = s64(uvalue);
						}
						else if ((m_text[0] == '0') && ((m_text[1] == 'x') || (m_text[1] == 'X')))
						{
							stream.get();
							stream.get();
							u64 uvalue;
							stream >> std::hex >> uvalue;
							m_int = s64(uvalue);
						}
						else
						{
							if (m_text[0] == '#')
								stream.get();
							stream >> m_int;
						}
						m_int_valid = bool(stream);
					}
					if (0 > m_shift)
						m_int >>= -m_shift;
					else
						m_int <<= m_shift;
					m_text_valid = m_float_valid = false;
				}
			}
		}

		static bool name_less(entry const &lhs, entry const &rhs) { return lhs.name() < rhs.name(); }

	private:
		std::string m_name;
		std::string m_text;
		s64 m_int = 0, m_int_increment = 0;
		double m_float = 0.0, m_float_increment = 0.0;
		int m_shift = 0;
		bool m_text_valid = false;
		bool m_int_valid = false;
		bool m_float_valid = false;
		bool m_generator = false;
	};

	using entry_vector = std::vector<entry>;

	template <typename T, typename U>
	void try_insert(T &&name, U &&value)
	{
		entry_vector::iterator const pos(
				std::lower_bound(
					m_entries.begin(),
					m_entries.end(),
					name,
					[] (entry const &lhs, auto const &rhs) { return lhs.name() < rhs; }));
		if ((m_entries.end() == pos) || (pos->name() != name))
			m_entries.emplace(pos, std::forward<T>(name), std::forward<U>(value));
	}

	template <typename T, typename U>
	void set(T &&name, U &&value)
	{
		entry_vector::iterator const pos(
				std::lower_bound(
					m_entries.begin(),
					m_entries.end(),
					name,
					[] (entry const &lhs, auto const &rhs) { return lhs.name() < rhs; }));
		if ((m_entries.end() == pos) || (pos->name() != name))
			m_entries.emplace(pos, std::forward<T>(name), std::forward<U>(value));
		else
			pos->set(std::forward<U>(value));
	}

	void cache_device_entries()
	{
		if (!m_next && !m_cached)
		{
			try_insert("devicetag", device().tag());
			try_insert("devicebasetag", device().basetag());
			try_insert("devicename", device().name());
			try_insert("deviceshortname", device().shortname());
			util::ovectorstream tmp;
			unsigned i(0U);
			for (screen_device const &screen : screen_device_enumerator(machine().root_device()))
			{
				std::pair<u64, u64> const physaspect(screen.physical_aspect());
				s64 const w(screen.visible_area().width()), h(screen.visible_area().height());
				s64 xaspect(w), yaspect(h);
				util::reduce_fraction(xaspect, yaspect);

				tmp.seekp(0);
				util::stream_format(tmp, "scr%uphysicalxaspect", i);
				tmp.put('\0');
				try_insert(&tmp.vec()[0], s64(physaspect.first));

				tmp.seekp(0);
				util::stream_format(tmp, "scr%uphysicalyaspect", i);
				tmp.put('\0');
				try_insert(&tmp.vec()[0], s64(physaspect.second));

				tmp.seekp(0);
				util::stream_format(tmp, "scr%unativexaspect", i);
				tmp.put('\0');
				try_insert(&tmp.vec()[0], xaspect);

				tmp.seekp(0);
				util::stream_format(tmp, "scr%unativeyaspect", i);
				tmp.put('\0');
				try_insert(&tmp.vec()[0], yaspect);

				tmp.seekp(0);
				util::stream_format(tmp, "scr%uwidth", i);
				tmp.put('\0');
				try_insert(&tmp.vec()[0], w);

				tmp.seekp(0);
				util::stream_format(tmp, "scr%uheight", i);
				tmp.put('\0');
				try_insert(&tmp.vec()[0], h);

				++i;
			}
			m_cached = true;
		}
	}

	entry *find_entry(char const *begin, char const *end)
	{
		cache_device_entries();
		entry_vector::iterator const pos(
				std::lower_bound(
					m_entries.begin(),
					m_entries.end(),
					std::make_pair(begin, end - begin),
					[] (entry const &lhs, std::pair<char const *, std::ptrdiff_t> const &rhs)
					{ return 0 > std::strncmp(lhs.name().c_str(), rhs.first, rhs.second); }));
		if ((m_entries.end() != pos) && (pos->name().length() == (end - begin)) && !std::strncmp(pos->name().c_str(), begin, end - begin))
			return &*pos;
		else
			return m_next ? m_next->find_entry(begin, end) : nullptr;
	}

	template <typename... T>
	std::tuple<char const *, char const *, bool> get_variable_text(T &&... args)
	{
		entry *const found(find_entry(std::forward<T>(args)...));
		if (found)
		{
			std::string const &text(found->get_text());
			char const *const begin(text.c_str());
			return std::make_tuple(begin, begin + text.length(), true);
		}
		else
		{
			return std::make_tuple(nullptr, nullptr, false);
		}
	}

	std::pair<char const *, char const *> expand(char const *begin, char const *end)
	{
		// search for candidate variable references
		char const *start(begin);
		char const *pos(std::find_if(start, end, is_variable_start));
		while (pos != end)
		{
			char const *const term(std::find_if(pos + 1, end, [] (char ch) { return !is_variable_char(ch); }));
			if ((term == end) || !is_variable_end(*term))
			{
				// not a valid variable name - keep searching
				pos = std::find_if(term, end, is_variable_start);
			}
			else
			{
				// looks like a variable reference - try to look it up
				std::tuple<char const *, char const *, bool> const text(get_variable_text(pos + 1, term));
				if (std::get<2>(text))
				{
					// variable found
					if (begin == start)
						m_buffer.seekp(0);
					m_buffer.write(start, pos - start);
					m_buffer.write(std::get<0>(text), std::get<1>(text) - std::get<0>(text));
					start = term + 1;
					pos = std::find_if(start, end, is_variable_start);
				}
				else
				{
					// variable not found - move on
					pos = std::find_if(pos + 1, end, is_variable_start);
				}
			}
		}

		// short-circuit the case where no substitutions were made
		if (start == begin)
		{
			return std::make_pair(begin, end);
		}
		else
		{
			m_buffer.write(start, pos - start);
			m_buffer.put('\0');
			std::vector<char> const &vec(m_buffer.vec());
			if (vec.empty())
				return std::make_pair(nullptr, nullptr);
			else
				return std::make_pair(&vec[0], &vec[0] + vec.size() - 1);
		}
	}

	std::pair<char const *, char const *> expand(char const *str)
	{
		return expand(str, str + strlen(str));
	}

	int parse_int(char const *begin, char const *end, int defvalue)
	{
		std::istringstream stream;
		stream.imbue(std::locale::classic());
		int result;
		if (begin[0] == '$')
		{
			stream.str(std::string(begin + 1, end));
			unsigned uvalue;
			stream >> std::hex >> uvalue;
			result = int(uvalue);
		}
		else if ((begin[0] == '0') && ((begin[1] == 'x') || (begin[1] == 'X')))
		{
			stream.str(std::string(begin + 2, end));
			unsigned uvalue;
			stream >> std::hex >> uvalue;
			result = int(uvalue);
		}
		else if (begin[0] == '#')
		{
			stream.str(std::string(begin + 1, end));
			stream >> result;
		}
		else
		{
			stream.str(std::string(begin, end));
			stream >> result;
		}

		return stream ? result : defvalue;
	}

	std::string parameter_name(util::xml::data_node const &node)
	{
		char const *const attrib(node.get_attribute_string("name", nullptr));
		if (!attrib)
			throw layout_syntax_error("parameter lacks name attribute");
		std::pair<char const *, char const *> const expanded(expand(attrib));
		return std::string(expanded.first, expanded.second);
	}

	static constexpr bool is_variable_start(char ch)
	{
		return '~' == ch;
	}
	static constexpr bool is_variable_end(char ch)
	{
		return '~' == ch;
	}
	static constexpr bool is_variable_char(char ch)
	{
		return (('0' <= ch) && ('9' >= ch)) || (('A' <= ch) && ('Z' >= ch)) || (('a' <= ch) && ('z' >= ch)) || ('_' == ch);
	}

	entry_vector m_entries;
	util::ovectorstream m_buffer;
	std::shared_ptr<NSVGrasterizer> const m_svg_rasterizer;
	device_t &m_device;
	char const *const m_search_path;
	char const *const m_directory_name;
	layout_environment *const m_next = nullptr;
	bool m_cached = false;

public:
	layout_environment(device_t &device, char const *searchpath, char const *dirname)
		: m_svg_rasterizer(nsvgCreateRasterizer(), util::nsvg_deleter())
		, m_device(device)
		, m_search_path(searchpath)
		, m_directory_name(dirname)
	{
	}
	explicit layout_environment(layout_environment &next)
		: m_svg_rasterizer(next.m_svg_rasterizer)
		, m_device(next.m_device)
		, m_search_path(next.m_search_path)
		, m_directory_name(next.m_directory_name)
		, m_next(&next)
	{
	}
	layout_environment(layout_environment const &) = delete;

	device_t &device() const { return m_device; }
	running_machine &machine() const { return device().machine(); }
	bool is_root_device() const { return &device() == &machine().root_device(); }
	char const *search_path() const { return m_search_path; }
	char const *directory_name() const { return m_directory_name; }
	std::shared_ptr<NSVGrasterizer> const &svg_rasterizer() const { return m_svg_rasterizer; }

	void set_parameter(std::string &&name, std::string &&value)
	{
		set(std::move(name), std::move(value));
	}

	void set_parameter(std::string &&name, s64 value)
	{
		set(std::move(name), value);
	}

	void set_parameter(std::string &&name, double value)
	{
		set(std::move(name), value);
	}

	void set_parameter(util::xml::data_node const &node)
	{
		// do basic validation
		std::string name(parameter_name(node));
		if (node.has_attribute("start") || node.has_attribute("increment") || node.has_attribute("lshift") || node.has_attribute("rshift"))
			throw layout_syntax_error("start/increment/lshift/rshift attributes are only allowed for repeat parameters");
		char const *const value(node.get_attribute_string("value", nullptr));
		if (!value)
			throw layout_syntax_error("parameter lacks value attribute");

		// expand value and stash
		std::pair<char const *, char const *> const expanded(expand(value));
		set(std::move(name), std::string(expanded.first, expanded.second));
	}

	void set_repeat_parameter(util::xml::data_node const &node, bool init)
	{
		// two types are allowed here - static value, and start/increment/lshift/rshift
		std::string name(parameter_name(node));
		char const *const start(node.get_attribute_string("start", nullptr));
		if (start)
		{
			// simple validity checks
			if (node.has_attribute("value"))
				throw layout_syntax_error("start attribute may not be used in combination with value attribute");
			int const lshift(node.has_attribute("lshift") ? get_attribute_int(node, "lshift", -1) : 0);
			int const rshift(node.has_attribute("rshift") ? get_attribute_int(node, "rshift", -1) : 0);
			if ((0 > lshift) || (0 > rshift))
				throw layout_syntax_error("lshift/rshift attributes must be non-negative integers");

			// increment is more complex - it may be an integer or a floating-point number
			s64 intincrement(0);
			double floatincrement(0);
			char const *const increment(node.get_attribute_string("increment", nullptr));
			if (increment)
			{
				std::pair<char const *, char const *> const expanded(expand(increment));
				unsigned const hexprefix((expanded.first[0] == '$') ? 1U : ((expanded.first[0] == '0') && ((expanded.first[1] == 'x') || (expanded.first[1] == 'X'))) ? 2U : 0U);
				unsigned const decprefix((expanded.first[0] == '#') ? 1U : 0U);
				bool const floatchars(std::find_if(expanded.first, expanded.second, [] (char ch) { return ('.' == ch) || ('e' == ch) || ('E' == ch); }) != expanded.second);
				std::istringstream stream(std::string(expanded.first + hexprefix + decprefix, expanded.second));
				stream.imbue(std::locale::classic());
				if (!hexprefix && !decprefix && floatchars)
				{
					stream >> floatincrement;
				}
				else if (hexprefix)
				{
					u64 uvalue;
					stream >> std::hex >> uvalue;
					intincrement = s64(uvalue);
				}
				else
				{
					stream >> intincrement;
				}

				// reject obviously bad stuff
				if (!stream)
					throw layout_syntax_error("increment attribute must be a number");
			}

			// don't allow generator parameters to be redefined
			if (init)
			{
				entry_vector::iterator const pos(
						std::lower_bound(
							m_entries.begin(),
							m_entries.end(),
							name,
							[] (entry const &lhs, auto const &rhs) { return lhs.name() < rhs; }));
				if ((m_entries.end() != pos) && (pos->name() == name))
					throw layout_syntax_error("generator parameters must be defined exactly once per scope");

				std::pair<char const *, char const *> const expanded(expand(start));
				if (floatincrement)
					m_entries.emplace(pos, std::move(name), std::string(expanded.first, expanded.second), floatincrement, lshift - rshift);
				else
					m_entries.emplace(pos, std::move(name), std::string(expanded.first, expanded.second), intincrement, lshift - rshift);
			}
		}
		else if (node.has_attribute("increment") || node.has_attribute("lshift") || node.has_attribute("rshift"))
		{
			throw layout_syntax_error("increment/lshift/rshift attributes require start attribute");
		}
		else
		{
			char const *const value(node.get_attribute_string("value", nullptr));
			if (!value)
				throw layout_syntax_error("parameter lacks value attribute");
			std::pair<char const *, char const *> const expanded(expand(value));
			entry_vector::iterator const pos(
					std::lower_bound(
						m_entries.begin(),
						m_entries.end(),
						name,
						[] (entry const &lhs, auto const &rhs) { return lhs.name() < rhs; }));
			if ((m_entries.end() == pos) || (pos->name() != name))
				m_entries.emplace(pos, std::move(name), std::string(expanded.first, expanded.second));
			else if (pos->is_generator())
				throw layout_syntax_error("generator parameters must be defined exactly once per scope");
			else
				pos->set(std::string(expanded.first, expanded.second));
		}
	}

	void increment_parameters()
	{
		m_entries.erase(
				std::remove_if(
					m_entries.begin(),
					m_entries.end(),
					[] (entry &e)
					{
						if (!e.is_generator())
							return true;
						e.increment();
						return false;
					}),
				m_entries.end());
	}

	char const *get_attribute_string(util::xml::data_node const &node, char const *name, char const *defvalue)
	{
		char const *const attrib(node.get_attribute_string(name, nullptr));
		return attrib ? expand(attrib).first : defvalue;
	}

	int get_attribute_int(util::xml::data_node const &node, const char *name, int defvalue)
	{
		char const *const attrib(node.get_attribute_string(name, nullptr));
		if (!attrib)
			return defvalue;

		// similar to what XML nodes do
		std::pair<char const *, char const *> const expanded(expand(attrib));
		return parse_int(expanded.first, expanded.second, defvalue);
	}

	float get_attribute_float(util::xml::data_node const &node, char const *name, float defvalue)
	{
		char const *const attrib(node.get_attribute_string(name, nullptr));
		if (!attrib)
			return defvalue;

		// similar to what XML nodes do
		std::pair<char const *, char const *> const expanded(expand(attrib));
		std::istringstream stream(std::string(expanded.first, expanded.second));
		stream.imbue(std::locale::classic());
		float result;
		return (stream >> result) ? result : defvalue;
	}

	bool get_attribute_bool(util::xml::data_node const &node, char const *name, bool defvalue)
	{
		char const *const attrib(node.get_attribute_string(name, nullptr));
		if (!attrib)
			return defvalue;

		// first try yes/no strings
		std::pair<char const *, char const *> const expanded(expand(attrib));
		if (!std::strcmp("yes", expanded.first) || !std::strcmp("true", expanded.first))
			return true;
		if (!std::strcmp("no", expanded.first) || !std::strcmp("false", expanded.first))
			return false;

		// fall back to integer parsing
		return parse_int(expanded.first, expanded.second, defvalue ? 1 : 0) != 0;
	}

	void parse_bounds(util::xml::data_node const *node, render_bounds &result)
	{
		if (!node)
		{
			// default to unit rectangle
			result.x0 = result.y0 = 0.0F;
			result.x1 = result.y1 = 1.0F;
		}
		else
		{
			// horizontal position/size
			if (node->has_attribute("left"))
			{
				result.x0 = get_attribute_float(*node, "left", 0.0F);
				result.x1 = get_attribute_float(*node, "right", 1.0F);
			}
			else
			{
				float const width = get_attribute_float(*node, "width", 1.0F);
				if (node->has_attribute("xc"))
					result.x0 = get_attribute_float(*node, "xc", 0.0F) - (width / 2.0F);
				else
					result.x0 = get_attribute_float(*node, "x", 0.0F);
				result.x1 = result.x0 + width;
			}

			// vertical position/size
			if (node->has_attribute("top"))
			{
				result.y0 = get_attribute_float(*node, "top", 0.0F);
				result.y1 = get_attribute_float(*node, "bottom", 1.0F);
			}
			else
			{
				float const height = get_attribute_float(*node, "height", 1.0F);
				if (node->has_attribute("yc"))
					result.y0 = get_attribute_float(*node, "yc", 0.0F) - (height / 2.0F);
				else
					result.y0 = get_attribute_float(*node, "y", 0.0F);
				result.y1 = result.y0 + height;
			}

			// check for errors
			if ((result.x0 > result.x1) || (result.y0 > result.y1))
				throw layout_syntax_error(util::string_format("illegal bounds (%f-%f)-(%f-%f)", result.x0, result.x1, result.y0, result.y1));
		}
	}

	render_color parse_color(util::xml::data_node const *node)
	{
		// default to opaque white
		if (!node)
			return render_color{ 1.0F, 1.0F, 1.0F, 1.0F };

		// parse attributes
		render_color const result{
				get_attribute_float(*node, "alpha", 1.0F),
				get_attribute_float(*node, "red", 1.0F),
				get_attribute_float(*node, "green", 1.0F),
				get_attribute_float(*node, "blue", 1.0F) };

		// check for errors
		if ((0.0F > (std::min)({ result.r, result.g, result.b, result.a })) || (1.0F < (std::max)({ result.r, result.g, result.b, result.a })))
			throw layout_syntax_error(util::string_format("illegal RGBA color %f,%f,%f,%f", result.r, result.g, result.b, result.a));

		return result;
	}

	int parse_orientation(util::xml::data_node const *node)
	{
		// default to no transform
		if (!node)
			return ROT0;

		// parse attributes
		int result;
		int const rotate(get_attribute_int(*node, "rotate", 0));
		switch (rotate)
		{
		case 0:     result = ROT0;      break;
		case 90:    result = ROT90;     break;
		case 180:   result = ROT180;    break;
		case 270:   result = ROT270;    break;
		default:    throw layout_syntax_error(util::string_format("invalid rotate attribute %d", rotate));
		}
		if (get_attribute_bool(*node, "swapxy", false))
			result ^= ORIENTATION_SWAP_XY;
		if (get_attribute_bool(*node, "flipx", false))
			result ^= ORIENTATION_FLIP_X;
		if (get_attribute_bool(*node, "flipy", false))
			result ^= ORIENTATION_FLIP_Y;
		return result;
	}
};


class view_environment : public layout_environment
{
private:
	view_environment *const m_next_view = nullptr;
	char const *const m_name;
	u32 const m_visibility_mask = 0U;
	unsigned m_next_visibility_bit = 0U;

public:
	view_environment(layout_environment &next, char const *name)
		: layout_environment(next)
		, m_name(name)
	{
	}
	view_environment(view_environment &next, bool visibility)
		: layout_environment(next)
		, m_next_view(&next)
		, m_name(next.m_name)
		, m_visibility_mask(next.m_visibility_mask | (u32(visibility ? 1 : 0) << next.m_next_visibility_bit))
		, m_next_visibility_bit(next.m_next_visibility_bit + (visibility ? 1 : 0))
	{
		if (32U < m_next_visibility_bit)
			throw layout_syntax_error(util::string_format("view '%s' contains too many visibility toggles", m_name));
	}
	~view_environment()
	{
		if (m_next_view)
			m_next_view->m_next_visibility_bit = m_next_visibility_bit;
	}

	u32 visibility_mask() const { return m_visibility_mask; }
};

} // namespace emu::render::detail


namespace {

bool add_bounds_step(emu::render::detail::layout_environment &env, emu::render::detail::bounds_vector &steps, util::xml::data_node const &node)
{
	int const state(env.get_attribute_int(node, "state", 0));
	auto const pos(
			std::lower_bound(
				steps.begin(),
				steps.end(),
				state,
				[] (emu::render::detail::bounds_step const &lhs, int rhs) { return lhs.state < rhs; }));
	if ((steps.end() != pos) && (state == pos->state))
		return false;

	auto &ins(*steps.emplace(pos, emu::render::detail::bounds_step{ state, { 0.0F, 0.0F, 0.0F, 0.0F }, { 0.0F, 0.0F, 0.0F, 0.0F } }));
	env.parse_bounds(&node, ins.bounds);
	return true;
}

void set_bounds_deltas(emu::render::detail::bounds_vector &steps)
{
	if (steps.empty())
	{
		steps.emplace_back(emu::render::detail::bounds_step{ 0, { 0.0F, 0.0F, 1.0F, 1.0F }, { 0.0F, 0.0F, 0.0F, 0.0F } });
	}
	else
	{
		auto i(steps.begin());
		auto j(i);
		while (steps.end() != ++j)
		{
			assert(j->state > i->state);

			i->delta.x0 = (j->bounds.x0 - i->bounds.x0) / (j->state - i->state);
			i->delta.x1 = (j->bounds.x1 - i->bounds.x1) / (j->state - i->state);
			i->delta.y0 = (j->bounds.y0 - i->bounds.y0) / (j->state - i->state);
			i->delta.y1 = (j->bounds.y1 - i->bounds.y1) / (j->state - i->state);

			i = j;
		}
	}
}

void normalize_bounds(emu::render::detail::bounds_vector &steps, float x0, float y0, float xoffs, float yoffs, float xscale, float yscale)
{
	auto i(steps.begin());
	i->bounds.x0 = x0 + (i->bounds.x0 - xoffs) * xscale;
	i->bounds.x1 = x0 + (i->bounds.x1 - xoffs) * xscale;
	i->bounds.y0 = y0 + (i->bounds.y0 - yoffs) * yscale;
	i->bounds.y1 = y0 + (i->bounds.y1 - yoffs) * yscale;

	auto j(i);
	while (steps.end() != ++j)
	{
		j->bounds.x0 = x0 + (j->bounds.x0 - xoffs) * xscale;
		j->bounds.x1 = x0 + (j->bounds.x1 - xoffs) * xscale;
		j->bounds.y0 = y0 + (j->bounds.y0 - yoffs) * yscale;
		j->bounds.y1 = y0 + (j->bounds.y1 - yoffs) * yscale;

		i->delta.x0 = (j->bounds.x0 - i->bounds.x0) / (j->state - i->state);
		i->delta.x1 = (j->bounds.x1 - i->bounds.x1) / (j->state - i->state);
		i->delta.y0 = (j->bounds.y0 - i->bounds.y0) / (j->state - i->state);
		i->delta.y1 = (j->bounds.y1 - i->bounds.y1) / (j->state - i->state);

		i = j;
	}
}

render_bounds accumulate_bounds(emu::render::detail::bounds_vector const &steps)
{
	auto i(steps.begin());
	render_bounds result(i->bounds);
	while (steps.end() != ++i)
		result |= i->bounds;
	return result;
}

inline render_bounds interpolate_bounds(emu::render::detail::bounds_vector const &steps, int state)
{
	auto pos(
			std::lower_bound(
				steps.begin(),
				steps.end(),
				state,
				[] (emu::render::detail::bounds_step const &lhs, int rhs) { return lhs.state < rhs; }));
	if (steps.begin() == pos)
	{
		return pos->bounds;
	}
	else
	{
		--pos;
		render_bounds result(pos->bounds);
		result.x0 += pos->delta.x0 * (state - pos->state);
		result.x1 += pos->delta.x1 * (state - pos->state);
		result.y0 += pos->delta.y0 * (state - pos->state);
		result.y1 += pos->delta.y1 * (state - pos->state);
		return result;
	}
}


bool add_color_step(emu::render::detail::layout_environment &env, emu::render::detail::color_vector &steps, util::xml::data_node const &node)
{
	int const state(env.get_attribute_int(node, "state", 0));
	auto const pos(
			std::lower_bound(
				steps.begin(),
				steps.end(),
				state,
				[] (emu::render::detail::color_step const &lhs, int rhs) { return lhs.state < rhs; }));
	if ((steps.end() != pos) && (state == pos->state))
		return false;

	steps.emplace(pos, emu::render::detail::color_step{ state, env.parse_color(&node), { 0.0F, 0.0F, 0.0F, 0.0F } });
	return true;
}

void set_color_deltas(emu::render::detail::color_vector &steps)
{
	if (steps.empty())
	{
		steps.emplace_back(emu::render::detail::color_step{ 0, { 1.0F, 1.0F, 1.0F, 1.0F }, { 0.0F, 0.0F, 0.0F, 0.0F } });
	}
	else
	{
		auto i(steps.begin());
		auto j(i);
		while (steps.end() != ++j)
		{
			assert(j->state > i->state);

			i->delta.a = (j->color.a - i->color.a) / (j->state - i->state);
			i->delta.r = (j->color.r - i->color.r) / (j->state - i->state);
			i->delta.g = (j->color.g - i->color.g) / (j->state - i->state);
			i->delta.b = (j->color.b - i->color.b) / (j->state - i->state);

			i = j;
		}
	}
}

inline render_color interpolate_color(emu::render::detail::color_vector const &steps, int state)
{
	auto pos(
			std::lower_bound(
				steps.begin(),
				steps.end(),
				state,
				[] (emu::render::detail::color_step const &lhs, int rhs) { return lhs.state < rhs; }));
	if (steps.begin() == pos)
	{
		return pos->color;
	}
	else
	{
		--pos;
		render_color result(pos->color);
		result.a += pos->delta.a * (state - pos->state);
		result.r += pos->delta.r * (state - pos->state);
		result.g += pos->delta.g * (state - pos->state);
		result.b += pos->delta.b * (state - pos->state);
		return result;
	}
}

} // anonymous namespace



//**************************************************************************
//  LAYOUT ELEMENT
//**************************************************************************

layout_element::make_component_map const layout_element::s_make_component{
	{ "image",         &make_component<image_component>         },
	{ "text",          &make_component<text_component>          },
	{ "dotmatrix",     &make_dotmatrix_component<8>             },
	{ "dotmatrix5dot", &make_dotmatrix_component<5>             },
	{ "dotmatrixdot",  &make_dotmatrix_component<1>             },
	{ "simplecounter", &make_component<simplecounter_component> },
	{ "reel",          &make_component<reel_component>          },
	{ "led7seg",       &make_component<led7seg_component>       },
	{ "led8seg_gts1",  &make_component<led8seg_gts1_component>  },
	{ "led14seg",      &make_component<led14seg_component>      },
	{ "led14segsc",    &make_component<led14segsc_component>    },
	{ "led16seg",      &make_component<led16seg_component>      },
	{ "led16segsc",    &make_component<led16segsc_component>    },
	{ "rect",          &make_component<rect_component>          },
	{ "disk",          &make_component<disk_component>          }
};

//-------------------------------------------------
//  layout_element - constructor
//-------------------------------------------------

layout_element::layout_element(environment &env, util::xml::data_node const &elemnode)
	: m_machine(env.machine())
	, m_defstate(env.get_attribute_int(elemnode, "defstate", -1))
	, m_statemask(0)
	, m_foldhigh(false)
{
	// parse components in order
	bool first = true;
	render_bounds bounds = { 0.0, 0.0, 0.0, 0.0 };
	for (util::xml::data_node const *compnode = elemnode.get_first_child(); compnode; compnode = compnode->get_next_sibling())
	{
		make_component_map::const_iterator const make_func(s_make_component.find(compnode->get_name()));
		if (make_func == s_make_component.end())
			throw layout_syntax_error(util::string_format("unknown element component %s", compnode->get_name()));

		// insert the new component into the list
		component const &newcomp(*m_complist.emplace_back(make_func->second(env, *compnode)));

		// accumulate bounds
		if (first)
			bounds = newcomp.overall_bounds();
		else
			bounds |= newcomp.overall_bounds();
		first = false;

		// determine the maximum state
		std::pair<int, bool> const wrap(newcomp.statewrap());
		m_statemask |= wrap.first;
		m_foldhigh = m_foldhigh || wrap.second;
	}

	if (!m_complist.empty())
	{
		// determine the scale/offset for normalization
		float xoffs = bounds.x0;
		float yoffs = bounds.y0;
		float xscale = 1.0F / (bounds.x1 - bounds.x0);
		float yscale = 1.0F / (bounds.y1 - bounds.y0);

		// normalize all the component bounds
		for (component::ptr const &curcomp : m_complist)
			curcomp->normalize_bounds(xoffs, yoffs, xscale, yscale);
	}

	// allocate an array of element textures for the states
	m_elemtex.resize((m_statemask + 1) << (m_foldhigh ? 1 : 0));
}


//-------------------------------------------------
//  ~layout_element - destructor
//-------------------------------------------------

layout_element::~layout_element()
{
}



//**************************************************************************
//  LAYOUT GROUP
//**************************************************************************

//-------------------------------------------------
//  layout_group - constructor
//-------------------------------------------------

layout_group::layout_group(util::xml::data_node const &groupnode)
	: m_groupnode(groupnode)
	, m_bounds{ 0.0F, 0.0F, 0.0F, 0.0F }
	, m_bounds_resolved(false)
{
}


//-------------------------------------------------
//  ~layout_group - destructor
//-------------------------------------------------

layout_group::~layout_group()
{
}


//-------------------------------------------------
//  make_transform - create abbreviated transform
//  matrix for given destination bounds
//-------------------------------------------------

layout_group::transform layout_group::make_transform(int orientation, render_bounds const &dest) const
{
	assert(m_bounds_resolved);

	// make orientation matrix
	transform result{{ {{ 1.0F, 0.0F, 0.0F }}, {{ 0.0F, 1.0F, 0.0F }}, {{ 0.0F, 0.0F, 1.0F }} }};
	if (orientation & ORIENTATION_SWAP_XY)
	{
		std::swap(result[0][0], result[0][1]);
		std::swap(result[1][0], result[1][1]);
	}
	if (orientation & ORIENTATION_FLIP_X)
	{
		result[0][0] = -result[0][0];
		result[0][1] = -result[0][1];
	}
	if (orientation & ORIENTATION_FLIP_Y)
	{
		result[1][0] = -result[1][0];
		result[1][1] = -result[1][1];
	}

	// apply to bounds and force into destination rectangle
	render_bounds bounds(m_bounds);
	render_bounds_transform(bounds, result);
	result[0][0] *= (dest.x1 - dest.x0) / std::fabs(bounds.x1 - bounds.x0);
	result[0][1] *= (dest.x1 - dest.x0) / std::fabs(bounds.x1 - bounds.x0);
	result[0][2] = dest.x0 - ((std::min)(bounds.x0, bounds.x1) * (dest.x1 - dest.x0) / std::fabs(bounds.x1 - bounds.x0));
	result[1][0] *= (dest.y1 - dest.y0) / std::fabs(bounds.y1 - bounds.y0);
	result[1][1] *= (dest.y1 - dest.y0) / std::fabs(bounds.y1 - bounds.y0);
	result[1][2] = dest.y0 - ((std::min)(bounds.y0, bounds.y1) * (dest.y1 - dest.y0) / std::fabs(bounds.y1 - bounds.y0));
	return result;
}

layout_group::transform layout_group::make_transform(int orientation, transform const &trans) const
{
	assert(m_bounds_resolved);

	render_bounds const dest{
			m_bounds.x0,
			m_bounds.y0,
			(orientation & ORIENTATION_SWAP_XY) ? (m_bounds.x0 + m_bounds.y1 - m_bounds.y0) : m_bounds.x1,
			(orientation & ORIENTATION_SWAP_XY) ? (m_bounds.y0 + m_bounds.x1 - m_bounds.x0) : m_bounds.y1 };
	return make_transform(orientation, dest, trans);
}

layout_group::transform layout_group::make_transform(int orientation, render_bounds const &dest, transform const &trans) const
{
	transform const next(make_transform(orientation, dest));
	transform result{{ {{ 0.0F, 0.0F, 0.0F }}, {{ 0.0F, 0.0F, 0.0F }}, {{ 0.0F, 0.0F, 0.0F }} }};
	for (unsigned y = 0; 3U > y; ++y)
	{
		for (unsigned x = 0; 3U > x; ++x)
		{
			for (unsigned i = 0; 3U > i; ++i)
				result[y][x] += trans[y][i] * next[i][x];
		}
	}
	return result;
}


//-------------------------------------------------
//  resolve_bounds - calculate bounds taking
//  nested groups into consideration
//-------------------------------------------------

void layout_group::set_bounds_unresolved()
{
	m_bounds_resolved = false;
}

void layout_group::resolve_bounds(environment &env, group_map &groupmap)
{
	if (!m_bounds_resolved)
	{
		std::vector<layout_group const *> seen;
		resolve_bounds(env, groupmap, seen);
	}
}

void layout_group::resolve_bounds(environment &env, group_map &groupmap, std::vector<layout_group const *> &seen)
{
	if (seen.end() != std::find(seen.begin(), seen.end(), this))
	{
		// a wild loop appears!
		std::ostringstream path;
		for (layout_group const *const group : seen)
			path << ' ' << group->m_groupnode.get_attribute_string("name", nullptr);
		path << ' ' << m_groupnode.get_attribute_string("name", nullptr);
		throw layout_syntax_error(util::string_format("recursively nested groups %s", path.str()));
	}

	seen.push_back(this);
	if (!m_bounds_resolved)
	{
		m_bounds.set_xy(0.0F, 0.0F, 1.0F, 1.0F);
		environment local(env);
		bool empty(true);
		resolve_bounds(local, m_groupnode, groupmap, seen, empty, false, false, true);
	}
	seen.pop_back();
}

void layout_group::resolve_bounds(
		environment &env,
		util::xml::data_node const &parentnode,
		group_map &groupmap,
		std::vector<layout_group const *> &seen,
		bool &empty,
		bool vistoggle,
		bool repeat,
		bool init)
{
	LOGMASKED(LOG_GROUP_BOUNDS_RESOLUTION, "Group '%s' resolve bounds empty=%s vistoggle=%s repeat=%s init=%s\n",
			parentnode.get_attribute_string("name", ""), empty, vistoggle, repeat, init);
	bool envaltered(false);
	bool unresolved(true);
	for (util::xml::data_node const *itemnode = parentnode.get_first_child(); !m_bounds_resolved && itemnode; itemnode = itemnode->get_next_sibling())
	{
		if (!strcmp(itemnode->get_name(), "bounds"))
		{
			// use explicit bounds
			env.parse_bounds(itemnode, m_bounds);
			m_bounds_resolved = true;
		}
		else if (!strcmp(itemnode->get_name(), "param"))
		{
			envaltered = true;
			if (!unresolved)
			{
				LOGMASKED(LOG_GROUP_BOUNDS_RESOLUTION, "Environment altered%s, unresolving groups\n", envaltered ? " again" : "");
				unresolved = true;
				for (group_map::value_type &group : groupmap)
					group.second.set_bounds_unresolved();
			}
			if (!repeat)
				env.set_parameter(*itemnode);
			else
				env.set_repeat_parameter(*itemnode, init);
		}
		else if (!strcmp(itemnode->get_name(), "element") ||
				!strcmp(itemnode->get_name(), "backdrop") ||
				!strcmp(itemnode->get_name(), "screen") ||
				!strcmp(itemnode->get_name(), "overlay") ||
				!strcmp(itemnode->get_name(), "bezel") ||
				!strcmp(itemnode->get_name(), "cpanel") ||
				!strcmp(itemnode->get_name(), "marquee"))
		{
			render_bounds itembounds;
			util::xml::data_node const *boundsnode = itemnode->get_child("bounds");
			env.parse_bounds(boundsnode, itembounds);
			while (boundsnode)
			{
				boundsnode = boundsnode->get_next_sibling("bounds");
				if (boundsnode)
				{
					render_bounds b;
					env.parse_bounds(boundsnode, b);
					itembounds |= b;
				}
			}
			if (empty)
				m_bounds = itembounds;
			else
				m_bounds |= itembounds;
			empty = false;
			LOGMASKED(LOG_GROUP_BOUNDS_RESOLUTION, "Accumulate item bounds (%s %s %s %s) -> (%s %s %s %s)\n",
					itembounds.x0, itembounds.y0, itembounds.x1, itembounds.y1,
					m_bounds.x0, m_bounds.y0, m_bounds.x1, m_bounds.y1);
		}
		else if (!strcmp(itemnode->get_name(), "group"))
		{
			util::xml::data_node const *const itemboundsnode(itemnode->get_child("bounds"));
			if (itemboundsnode)
			{
				render_bounds itembounds;
				env.parse_bounds(itemboundsnode, itembounds);
				if (empty)
					m_bounds = itembounds;
				else
					m_bounds |= itembounds;
				empty = false;
				LOGMASKED(LOG_GROUP_BOUNDS_RESOLUTION, "Accumulate group '%s' reference explicit bounds (%s %s %s %s) -> (%s %s %s %s)\n",
						itemnode->get_attribute_string("ref", ""),
						itembounds.x0, itembounds.y0, itembounds.x1, itembounds.y1,
						m_bounds.x0, m_bounds.y0, m_bounds.x1, m_bounds.y1);
			}
			else
			{
				char const *ref(env.get_attribute_string(*itemnode, "ref", nullptr));
				if (!ref)
					throw layout_syntax_error("nested group must have ref attribute");

				group_map::iterator const found(groupmap.find(ref));
				if (groupmap.end() == found)
					throw layout_syntax_error(util::string_format("unable to find group %s", ref));

				int const orientation(env.parse_orientation(itemnode->get_child("orientation")));
				environment local(env);
				found->second.resolve_bounds(local, groupmap, seen);
				render_bounds const itembounds{
						found->second.m_bounds.x0,
						found->second.m_bounds.y0,
						(orientation & ORIENTATION_SWAP_XY) ? (found->second.m_bounds.x0 + found->second.m_bounds.y1 - found->second.m_bounds.y0) : found->second.m_bounds.x1,
						(orientation & ORIENTATION_SWAP_XY) ? (found->second.m_bounds.y0 + found->second.m_bounds.x1 - found->second.m_bounds.x0) : found->second.m_bounds.y1 };
				if (empty)
					m_bounds = itembounds;
				else
					m_bounds |= itembounds;
				empty = false;
				unresolved = false;
				LOGMASKED(LOG_GROUP_BOUNDS_RESOLUTION, "Accumulate group '%s' reference computed bounds (%s %s %s %s) -> (%s %s %s %s)\n",
						itemnode->get_attribute_string("ref", ""),
						itembounds.x0, itembounds.y0, itembounds.x1, itembounds.y1,
						m_bounds.x0, m_bounds.y0, m_bounds.x1, m_bounds.y1);
			}
		}
		else if (!strcmp(itemnode->get_name(), "repeat"))
		{
			int const count(env.get_attribute_int(*itemnode, "count", -1));
			if (0 >= count)
				throw layout_syntax_error("repeat must have positive integer count attribute");
			environment local(env);
			for (int i = 0; !m_bounds_resolved && (count > i); ++i)
			{
				resolve_bounds(local, *itemnode, groupmap, seen, empty, false, true, !i);
				local.increment_parameters();
			}
		}
		else if (!strcmp(itemnode->get_name(), "collection"))
		{
			if (!env.get_attribute_string(*itemnode, "name", nullptr))
				throw layout_syntax_error("collection must have name attribute");
			environment local(env);
			resolve_bounds(local, *itemnode, groupmap, seen, empty, true, false, true);
		}
		else
		{
			throw layout_syntax_error(util::string_format("unknown group element %s", itemnode->get_name()));
		}
	}

	if (envaltered && !unresolved)
	{
		LOGMASKED(LOG_GROUP_BOUNDS_RESOLUTION, "Environment was altered, marking groups unresolved\n");
		bool const resolved(m_bounds_resolved);
		for (group_map::value_type &group : groupmap)
			group.second.set_bounds_unresolved();
		m_bounds_resolved = resolved;
	}

	if (!vistoggle && !repeat)
	{
		LOGMASKED(LOG_GROUP_BOUNDS_RESOLUTION, "Marking group '%s' bounds resolved\n",
				parentnode.get_attribute_string("name", ""));
		m_bounds_resolved = true;
	}
}



//-------------------------------------------------
//  state_texture - return a pointer to a
//  render_texture for the given state, allocating
//  one if needed
//-------------------------------------------------

render_texture *layout_element::state_texture(int state)
{
	if (m_foldhigh && (state & ~m_statemask))
		state = (state & m_statemask) | (((m_statemask << 1) | 1) & ~m_statemask);
	else
		state &= m_statemask;
	assert(m_elemtex.size() > state);
	if (!m_elemtex[state].m_texture)
	{
		m_elemtex[state].m_element = this;
		m_elemtex[state].m_state = state;
		m_elemtex[state].m_texture = machine().render().texture_alloc(element_scale, &m_elemtex[state]);
	}
	return m_elemtex[state].m_texture;
}


//-------------------------------------------------
//  preload - perform expensive loading upfront
//  for all components
//-------------------------------------------------

void layout_element::preload()
{
	for (component::ptr const &curcomp : m_complist)
		curcomp->preload(machine());
}


//-------------------------------------------------
//  element_scale - scale an element by rendering
//  all the components at the appropriate
//  resolution
//-------------------------------------------------

void layout_element::element_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param)
{
	texture const &elemtex(*reinterpret_cast<texture const *>(param));

	// draw components that are visible in the current state
	for (auto const &curcomp : elemtex.m_element->m_complist)
	{
		if ((elemtex.m_state & curcomp->statemask()) == curcomp->stateval())
			curcomp->draw(elemtex.m_element->machine(), dest, elemtex.m_state);
	}
}


// image
class layout_element::image_component : public component
{
public:
	// construction/destruction
	image_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
		, m_rasterizer(env.svg_rasterizer())
		, m_searchpath(env.search_path() ? env.search_path() : "")
		, m_dirname(env.directory_name() ? env.directory_name() : "")
		, m_imagefile(env.get_attribute_string(compnode, "file", ""))
		, m_alphafile(env.get_attribute_string(compnode, "alphafile", ""))
		, m_data(get_data(compnode))
	{
	}

	// overrides
	virtual void preload(running_machine &machine) override
	{
		if (!m_bitmap.valid() && !m_svg)
			load_image(machine);
	}

protected:
	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, rectangle const &bounds, int state) override
	{
		if (!m_bitmap.valid() && !m_svg)
			load_image(machine);

		if (m_bitmap.valid())
			draw_bitmap(dest, bounds, state);
		else if (m_svg)
			draw_svg(dest, bounds, state);
	}

private:
	// internal helpers
	void draw_bitmap(bitmap_argb32 &dest, rectangle const &bounds, int state)
	{
		render_color const c(color(state));
		if (m_hasalpha || (1.0F > c.a))
		{
			bitmap_argb32 tempbitmap(dest.width(), dest.height());
			render_resample_argb_bitmap_hq(tempbitmap, m_bitmap, c);
			alpha_blend(tempbitmap, dest, bounds);
		}
		else
		{
			bitmap_argb32 destsub(dest, bounds);
			render_resample_argb_bitmap_hq(destsub, m_bitmap, c);
		}
	}

	void draw_svg(bitmap_argb32 &dest, rectangle const &bounds, int state)
	{
		// rasterise into a temporary bitmap
		float const xscale(bounds.width() / m_svg->width);
		float const yscale(bounds.height() / m_svg->height);
		float const drawscale((std::max)(xscale, yscale));
		bitmap_argb32 tempbitmap(int(m_svg->width * drawscale), int(m_svg->height * drawscale));
		nsvgRasterize(
				m_rasterizer.get(),
				m_svg.get(),
				0, 0, drawscale,
				reinterpret_cast<unsigned char *>(&tempbitmap.pix(0)),
				tempbitmap.width(), tempbitmap.height(),
				tempbitmap.rowbytes());

		// correct colour format and multiply by state colour
		bool havealpha(false);
		render_color const c(color(state));
		for (s32 y = 0; tempbitmap.height() > y; ++y)
		{
			u32 *dst(&tempbitmap.pix(y));
			for (s32 x = 0; tempbitmap.width() > x; ++x, ++dst)
			{
				u8 const *const src(reinterpret_cast<u8 const *>(dst));
				rgb_t const d(
						u8((float(src[3]) * c.a) + 0.5),
						u8((float(src[0]) * c.r) + 0.5),
						u8((float(src[1]) * c.g) + 0.5),
						u8((float(src[2]) * c.b) + 0.5));
				*dst = d;
				havealpha = havealpha || (d.a() < 255U);
			}
		}

		// find most efficient way to insert it in the target bitmap
		if (!havealpha)
		{
			if ((tempbitmap.width() == bounds.width()) && (tempbitmap.height() == bounds.height()))
			{
				for (s32 y = 0; tempbitmap.height() > y; ++y)
					std::copy_n(&tempbitmap.pix(y), bounds.width(), &dest.pix(y + bounds.top(), bounds.left()));
			}
			else
			{
				bitmap_argb32 destsub(dest, bounds);
				render_resample_argb_bitmap_hq(destsub, tempbitmap, render_color{ 1.0F, 1.0F, 1.0F, 1.0F });
			}
		}
		else if ((tempbitmap.width() == bounds.width()) && (tempbitmap.height() == bounds.height()))
		{
			alpha_blend(tempbitmap, dest, bounds);
		}
		else
		{
			bitmap_argb32 scaled(bounds.width(), bounds.height());
			render_resample_argb_bitmap_hq(scaled, tempbitmap, render_color{ 1.0F, 1.0F, 1.0F, 1.0F });
			tempbitmap.reset();
			alpha_blend(scaled, dest, bounds);
		}
	}

	void alpha_blend(bitmap_argb32 const &srcbitmap, bitmap_argb32 &dstbitmap, rectangle const &bounds)
	{
		for (s32 y0 = 0, y1 = bounds.top(); bounds.bottom() >= y1; ++y0, ++y1)
		{
			u32 const *src(&srcbitmap.pix(y0, 0));
			u32 *dst(&dstbitmap.pix(y1, bounds.left()));
			for (s32 x1 = bounds.left(); bounds.right() >= x1; ++x1, ++src, ++dst)
			{
				rgb_t const a(*src);
				u32 const aa(a.a());
				if (255 == aa)
				{
					*dst = *src;
				}
				else if (aa)
				{
					rgb_t const b(*dst);
					u32 const ba(b.a());
					if (ba)
					{
						u32 const ca((aa * 255) + (ba * (255 - aa)));
						*dst = rgb_t(
								u8(ca / 255),
								u8(((a.r() * aa * 255) + (b.r() * ba * (255 - aa))) / ca),
								u8(((a.g() * aa * 255) + (b.g() * ba * (255 - aa))) / ca),
								u8(((a.b() * aa * 255) + (b.b() * ba * (255 - aa))) / ca));
					}
					else
					{
						*dst = *src;
					}
				}
			}
		}
	}

	void load_image(running_machine &machine)
	{
		// if we have a filename, go with that
		emu_file file(m_searchpath.empty() ? m_dirname : m_searchpath, OPEN_FLAG_READ);
		if (!m_imagefile.empty())
		{
			std::string filename;
			if (!m_searchpath.empty())
				filename = m_dirname;
			if (!filename.empty() && !util::is_directory_separator(filename[filename.size() - 1]))
				filename.append(PATH_SEPARATOR);
			filename.append(m_imagefile);
			LOGMASKED(LOG_IMAGE_LOAD, "Image component attempt to load image file '%s'\n", filename);
			osd_file::error const imgerr = file.open(filename);
			if (osd_file::error::NONE == imgerr)
			{
				if (!load_bitmap(file))
				{
					LOGMASKED(LOG_IMAGE_LOAD, "Image component will attempt to parse file as SVG\n");
					load_svg(file);
				}
				file.close();
			}
			else
			{
				LOGMASKED(LOG_IMAGE_LOAD, "Image component unable to open image file '%s'\n", filename);
			}
		}
		else if (!m_data.empty())
		{
			load_image_data();
		}

		// load the alpha bitmap if specified
		if (!m_alphafile.empty())
		{
			if (m_bitmap.valid())
			{
				std::string filename;
				if (!m_searchpath.empty())
					filename = m_dirname;
				if (!filename.empty() && !util::is_directory_separator(filename[filename.size() - 1]))
					filename.append(PATH_SEPARATOR);
				filename.append(m_alphafile);
				LOGMASKED(LOG_IMAGE_LOAD, "Image component attempt to load alpha channel from file '%s'\n", filename);
				osd_file::error const alferr = file.open(filename);
				if (osd_file::error::NONE == alferr)
				{
					// TODO: no way to detect corner case where we had alpha from the image but the alpha PNG makes it entirely opaque
					if (render_load_png(m_bitmap, file, true))
						m_hasalpha = true;
					file.close();
				}
				else
				{
					LOGMASKED(LOG_IMAGE_LOAD, "Image component unable to open alpha channel file '%s'\n", filename);
				}
			}
			else if (m_svg)
			{
				osd_printf_warning("Component alpha channel file '%s' ignored for SVG image '%s'\n", m_alphafile, m_imagefile);
			}
		}

		// if we can't load an image, allocate a dummy one and report an error
		if (!m_bitmap.valid() && !m_svg)
		{
			// draw some stripes in the bitmap
			m_bitmap.allocate(100, 100);
			m_bitmap.fill(0);
			for (int step = 0; step < 100; step += 25)
				for (int line = 0; line < 100; line++)
					m_bitmap.pix((step + line) % 100, line % 100) = rgb_t(0xff,0xff,0xff,0xff);

			// log an error
			if (m_alphafile.empty())
				osd_printf_warning("Unable to load component image '%s'\n", m_imagefile);
			else
				osd_printf_warning("Unable to load component image '%s'/'%s'\n", m_imagefile, m_alphafile);
		}

		// clear out this stuff in case it's large
		if (!m_svg)
			m_rasterizer.reset();
		m_searchpath.clear();
		m_dirname.clear();
		m_imagefile.clear();
		m_alphafile.clear();
		m_data.clear();
	}

	void load_image_data()
	{
		// in-place Base64 decode
		static constexpr char base64chars[] =
				"\t\n\v\f\r +/0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
		static constexpr char base64tail[] =
				"\t\n\v\f\r =";
		std::string::size_type const tail(m_data.find_first_not_of(base64chars));
		std::string::size_type const end(m_data.find_first_not_of(base64tail, tail));
		if (std::string::npos == end)
		{
			LOGMASKED(LOG_IMAGE_LOAD, "Image component decoding Base64 image data\n");
			char *dst(&m_data[0]);
			unsigned trailing(0U);
			for (std::string::size_type i = 0U; (m_data.size() > i) && ('=' != m_data[i]); ++i)
			{
				u8 sym;
				if (('A' <= m_data[i]) && ('Z' >= m_data[i]))
					sym = m_data[i] - 'A';
				else if (('a' <= m_data[i]) && ('z' >= m_data[i]))
					sym = m_data[i] - 'a' + 26;
				else if (('0' <= m_data[i]) && ('9' >= m_data[i]))
					sym = m_data[i] - '0' + 52;
				else if ('+' == m_data[i])
					sym = 62;
				else if ('/' == m_data[i])
					sym = 63;
				else
					continue;
				if (trailing)
					*dst |= (sym << 2) >> trailing;
				else
					*dst = sym << 2;
				if (trailing >= 2U)
					++dst;
				trailing = (trailing + 6U) & 7U;
				if (trailing)
					*dst = sym << (8U - trailing);
			}
			m_data.resize(dst - &m_data[0]);
		}

		// make a file wrapper for the data and see if it looks like a bitmap
		util::core_file::ptr file;
		osd_file::error const filerr(util::core_file::open_ram(m_data.c_str(), m_data.size(), OPEN_FLAG_READ, file));
		bool const bitmapdata((osd_file::error::NONE == filerr) && file && load_bitmap(*file));
		file.reset();

		// if it didn't look like a bitmap, see if it looks like it might be XML and hence SVG
		if (!bitmapdata)
		{
			bool const utf16be((0xfe == u8(m_data[0])) && (0xff == u8(m_data[1])));
			bool const utf16le((0xff == u8(m_data[0])) && (0xfe == u8(m_data[1])));
			bool const utf8((0xef == u8(m_data[0])) && (0xbb == u8(m_data[1])) && (0xbf == u8(m_data[2])));
			std::string::size_type const found(m_data.find_first_not_of("\t\n\v\f\r "));
			bool const xmltag((std::string::npos != found) && ('<' == m_data[found]));
			if (utf16be || utf16le || utf8 || xmltag)
			{
				LOGMASKED(LOG_IMAGE_LOAD, "Image component will attempt to parse data as SVG\n");
				parse_svg(&m_data[0]);
			}
		}
	}

	bool load_bitmap(util::core_file &file)
	{
		ru_imgformat const format = render_detect_image(file);
		switch (format)
		{
		case RENDUTIL_IMGFORMAT_ERROR:
			LOGMASKED(LOG_IMAGE_LOAD, "Image component error detecting image file format\n");
			return false;

		case RENDUTIL_IMGFORMAT_PNG:
			LOGMASKED(LOG_IMAGE_LOAD, "Image component detected PNG file format\n");
			m_hasalpha = render_load_png(m_bitmap, file);
			return true;

		case RENDUTIL_IMGFORMAT_JPEG:
			LOGMASKED(LOG_IMAGE_LOAD, "Image component detected JPEG file format\n");
			render_load_jpeg(m_bitmap, file);
			return true;

		case RENDUTIL_IMGFORMAT_MSDIB:
			LOGMASKED(LOG_IMAGE_LOAD, "Image component detected Microsoft DIB file format\n");
			render_load_msdib(m_bitmap, file);
			return true;

		default:
			LOGMASKED(LOG_IMAGE_LOAD, "Image component failed to detect bitmap file format\n");
			return false;
		}
	}

	void load_svg(util::core_file &file)
	{
		u64 len(file.size());
		if ((std::numeric_limits<size_t>::max() - 1) < len)
		{
			osd_printf_warning("Component image '%s' is too large to read into memory\n", m_imagefile);
			return;
		}
		std::unique_ptr<char []> svgbuf(new (std::nothrow) char [size_t(len) + 1]);
		if (!svgbuf)
		{
			osd_printf_warning("Error allocating memory to read component image '%s'\n", m_imagefile);
			return;
		}
		svgbuf[len] = '\0';
		for (char *ptr = svgbuf.get(); len; )
		{
			u32 const block(u32(std::min<u64>(std::numeric_limits<u32>::max(), len)));
			u32 const read(file.read(ptr, block));
			if (!read)
			{
				osd_printf_warning("Error reading component image '%s'\n", m_imagefile);
				return;
			}
			ptr += read;
			len -= read;
		}
		parse_svg(svgbuf.get());
	}

	void parse_svg(char *svgdata)
	{
		if (!m_rasterizer)
		{
			osd_printf_warning("No SVG rasteriser available, won't attempt to parse component image '%s' as SVG\n", m_imagefile);
			return;
		}
		m_svg.reset(nsvgParse(svgdata, "px", 72));
		if (!m_svg)
		{
			osd_printf_warning("Failed to parse component image '%s' as SVG\n", m_imagefile);
			return;
		}
		if ((0.0F >= m_svg->width) || (0.0F >= m_svg->height))
		{
			osd_printf_warning("Parsing component image '%s' as SVG produced empty image\n", m_imagefile);
			m_svg.reset();
			return;
		}
	}

	static std::string get_data(util::xml::data_node const &compnode)
	{
		util::xml::data_node const *datanode(compnode.get_child("data"));
		if (datanode && datanode->get_value())
			return datanode->get_value();
		else
			return "";
	}

	// internal state
	util::nsvg_image_ptr            m_svg;              // parsed SVG image
	std::shared_ptr<NSVGrasterizer> m_rasterizer;       // SVG rasteriser
	bitmap_argb32                   m_bitmap;           // source bitmap for images
	bool                            m_hasalpha = false; // is there any alpha component present?

	// cold state
	std::string                     m_searchpath;       // asset search path (for lazy loading)
	std::string                     m_dirname;          // directory name of image file (for lazy loading)
	std::string                     m_imagefile;        // name of the image file (for lazy loading)
	std::string                     m_alphafile;        // name of the alpha file (for lazy loading)
	std::string                     m_data;             // embedded image data
};


// rectangle
class layout_element::rect_component : public component
{
public:
	// construction/destruction
	rect_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
	}

protected:
	// overrides
	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		render_color const c(color(state));
		if (1.0f <= c.a)
		{
			// optimise opaque pixels
			u32 const f(rgb_t(u8(c.r * 255), u8(c.g * 255), u8(c.b * 255)));
			s32 const width(bounds.width());
			for (u32 y = bounds.top(); y <= bounds.bottom(); ++y)
				std::fill_n(&dest.pix(y, bounds.left()), width, f);
		}
		else if (c.a)
		{
			// compute premultiplied colors
			u32 const a(c.a * 255.0F);
			u32 const r(u32(c.r * (255.0F * 255.0F)) * a);
			u32 const g(u32(c.g * (255.0F * 255.0F)) * a);
			u32 const b(u32(c.b * (255.0F * 255.0F)) * a);
			u32 const inva(255 - a);

			// we're translucent, add in the destination pixel contribution
			for (u32 y = bounds.top(); y <= bounds.bottom(); ++y)
			{
				u32 *dst(&dest.pix(y, bounds.left()));
				for (u32 x = bounds.left(); x <= bounds.right(); ++x, ++dst)
					alpha_blend(*dst, a, r, g, b, inva);
			}
		}
	}
};


// ellipse
class layout_element::disk_component : public component
{
public:
	// construction/destruction
	disk_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
	}

	// overrides
	virtual void draw(running_machine &machine, bitmap_argb32 &dest, int state) override
	{
		// compute premultiplied color
		render_color const c(color(state));
		u32 const f(rgb_t(u8(c.r * 255), u8(c.g * 255), u8(c.b * 255)));
		u32 const a(c.a * 255.0F);
		u32 const r(c.r * c.a * (255.0F * 255.0F * 255.0F));
		u32 const g(c.g * c.a * (255.0F * 255.0F * 255.0F));
		u32 const b(c.b * c.a * (255.0F * 255.0F * 255.0F));
		u32 const inva(255 - a);
		if (!a)
			return;

		// calculate the position and size
		render_bounds const curbounds = bounds(state);
		float const xcenter = (curbounds.x0 + curbounds.x1) * float(dest.width()) * 0.5F;
		float const ycenter = (curbounds.y0 + curbounds.y1) * float(dest.height()) * 0.5F;
		float const xradius = curbounds.width() * float(dest.width()) * 0.5F;
		float const yradius = curbounds.height() * float(dest.height()) * 0.5F;
		s32 const miny = s32(curbounds.y0 * float(dest.height()));
		s32 const maxy = s32(std::ceil(curbounds.y1 * float(dest.height()))) - 1;
		LOGMASKED(LOG_DISK_DRAW, "Draw disk: bounds (%s %s %s %s); (((x - %s) ** 2) / (%s ** 2) + ((y - %s) ** 2) / (%s ** 2)) = 1; rows [%s %s]\n",
				curbounds.x0, curbounds.y0, curbounds.x1, curbounds.y1, xcenter, xradius, ycenter, yradius, miny, maxy);

		if (miny == maxy)
		{
			// fits in a single row of pixels - integrate entire area of ellipse
			float const scale = xradius * yradius * 0.5F;
			s32 const minx = s32(curbounds.x0 * float(dest.width()));
			s32 const maxx = s32(std::ceil(curbounds.x1 * float(dest.width()))) - 1;
			float x1 = (float(minx) - xcenter) / xradius;
			u32 *dst = &dest.pix(miny, minx);
			for (s32 x = minx; maxx >= x; ++x, ++dst)
			{
				float const x0 = x1;
				x1 = (float(x + 1) - xcenter) / xradius;
				float const val = integral((std::max)(x0, -1.0F), (std::min)(x1, 1.0F)) * scale;
				alpha_blend(*dst, c, val);
			}
		}
		else
		{
			float const scale = xradius * yradius * 0.25F;
			float const ooyradius2 = 1.0F / (yradius * yradius);
			auto const draw_edge_row =
					[&dest, &c, &curbounds, xcenter, xradius, scale, ooyradius2] (s32 row, float ycoord, bool cross_axis)
					{
						float const xval = xradius * std::sqrt((std::max)(1.0F - (ycoord * ycoord) * ooyradius2, 0.0F));
						float const l = xcenter - xval;
						float const r = xcenter + xval;
						if (!cross_axis)
						{
							s32 minx = s32(l);
							s32 maxx = s32(std::ceil(r)) - 1;
							float x1 = float(minx) - xcenter;
							u32 *dst = &dest.pix(row, minx);
							for (s32 x = minx; maxx >= x; ++x, ++dst)
							{
								float const x0 = x1;
								x1 = float(x + 1) - xcenter;
								float val = integral((std::max)(x0, -xval) / xradius, (std::min)(x1, xval) / xradius) * scale;
								val -= ((std::min)(float(x + 1), r) - (std::max)(float(x), l)) * ycoord;
								alpha_blend(*dst, c, val);
							}
						}
						else
						{
							s32 const minx = s32(curbounds.x0 * float(dest.width()));
							s32 const maxx = s32(std::ceil(curbounds.x1 * float(dest.width()))) - 1;
							float x1 = (float(minx) - xcenter) / xradius;
							u32 *dst = &dest.pix(row, minx);
							for (s32 x = minx; maxx >= x; ++x, ++dst)
							{
								float const x0 = x1;
								x1 = (float(x + 1) - xcenter) / xradius;
								float val = integral((std::max)(x0, -1.0F), (std::min)(x1, 1.0F));
								if (float(x + 1) <= l)
									val += integral((std::max)(x0, -1.0F), x1);
								else if (float(x) <= l)
									val += integral((std::max)(x0, -1.0F), -xval / xradius);
								if (float(x) >= r)
									val += integral(x0, (std::min)(x1, 1.0F));
								else if (float(x + 1) >= r)
									val += integral(xval / xradius, (std::min)(x1, 1.0F));
								val *= scale;
								val -= (std::max)(((std::min)(float(x + 1), r) - (std::max)(float(x), l)), 0.0F) * ycoord;
								alpha_blend(*dst, c, val);
							}
						}
					};

			// draw the top row - in a thin ellipse it may extend below the axis
			draw_edge_row(miny, ycenter - float(miny + 1), float(miny + 1) > ycenter);

			// draw rows above the axis
			s32 y = miny + 1;
			float ycoord1 = ycenter - float(y);
			float xval1 = std::sqrt((std::max)(1.0F - (ycoord1 * ycoord1) * ooyradius2, 0.0F));
			float l1 = xcenter - (xval1 * xradius);
			float r1 = xcenter + (xval1 * xradius);
			for ( ; (maxy > y) && (float(y + 1) <= ycenter); ++y)
			{
				float const xval0 = xval1;
				float const l0 = l1;
				float const r0 = r1;
				ycoord1 = ycenter - float(y + 1);
				xval1 = std::sqrt((std::max)(1.0F - (ycoord1 * ycoord1) * ooyradius2, 0.0F));
				l1 = xcenter - (xval1 * xradius);
				r1 = xcenter + (xval1 * xradius);
				s32 minx = int(l1);
				s32 maxx = int(std::ceil(r1)) - 1;
				u32 *dst = &dest.pix(y, minx);
				for (s32 x = minx; maxx >= x; ++x, ++dst)
				{
					if ((float(x) >= l0) && (float(x + 1) <= r0))
					{
						if (255 <= a)
							*dst = f;
						else
							alpha_blend(*dst, a, r, g, b, inva);
					}
					else
					{
						float val = 0.0F;
						if (float(x + 1) <= l0)
							val += integral((std::max)((float(x) - xcenter) / xradius, -xval1), (float(x + 1) - xcenter) / xradius);
						else if (float(x) <= l0)
							val += integral((std::max)((float(x) - xcenter) / xradius, -xval1), -xval0);
						else if (float(x) >= r0)
							val += integral((float(x) - xcenter) / xradius, (std::min)((float(x + 1) - xcenter) / xradius, xval1));
						else if (float(x + 1) >= r0)
							val += integral(xval0, (std::min)((float(x + 1) - xcenter) / xradius, xval1));
						val *= scale;
						if (float(x) <= l0)
							val -= ((std::min)(float(x + 1), l0) - (std::max)(float(x), l1)) * ycoord1;
						else if (float(x + 1) >= r0)
							val -= ((std::min)(float(x + 1), r1) - (std::max)(float(x), r0)) * ycoord1;
						val += (std::max)((std::min)(float(x + 1), r0) - (std::max)(float(x), l0), 0.0F);
						alpha_blend(*dst, c, val);
					}
				}
			}

			// row spanning the axis
			if ((maxy > y) && (float(y) < ycenter))
			{
				float const xval0 = xval1;
				float const l0 = l1;
				float const r0 = r1;
				ycoord1 = float(y + 1) - ycenter;
				xval1 = std::sqrt((std::max)(1.0F - (ycoord1 * ycoord1) * ooyradius2, 0.0F));
				l1 = xcenter - (xval1 * xradius);
				r1 = xcenter + (xval1 * xradius);
				s32 const minx = int(curbounds.x0 * float(dest.width()));
				s32 const maxx = int(std::ceil(curbounds.x1 * float(dest.width()))) - 1;
				u32 *dst = &dest.pix(y, minx);
				for (s32 x = minx; maxx >= x; ++x, ++dst)
				{
					if ((float(x) >= (std::max)(l0, l1)) && (float(x + 1) <= (std::min)(r0, r1)))
					{
						if (255 <= a)
							*dst = f;
						else
							alpha_blend(*dst, a, r, g, b, inva);
					}
					else
					{
						float val = 0.0F;
						if (float(x + 1) <= l0)
							val += integral((xcenter - float(x + 1)) / xradius, (std::min)((xcenter - float(x)) / xradius, 1.0F));
						else if (float(x) <= l0)
							val += integral(xval0, (std::min)((xcenter - float(x)) / xradius, 1.0F));
						else if (float(x) >= r0)
							val += integral((float(x) - xcenter) / xradius, (std::min)((float(x + 1) - xcenter) / xradius, 1.0F));
						else if (float(x + 1) >= r0)
							val += integral(xval0, (std::min)((float(x + 1) - xcenter) / xradius, 1.0F));
						if (float(x + 1) <= l1)
							val += integral((xcenter - float(x + 1)) / xradius, (std::min)((xcenter - float(x)) / xradius, 1.0F));
						else if (float(x) <= l1)
							val += integral(xval1, (std::min)((xcenter - float(x)) / xradius, 1.0F));
						else if (float(x) >= r1)
							val += integral((float(x) - xcenter) / xradius, (std::min)((float(x + 1) - xcenter) / xradius, 1.0F));
						else if (float(x + 1) >= r1)
							val += integral(xval1, (std::min)((float(x + 1) - xcenter) / xradius, 1.0F));
						val *= scale;
						val += (std::max)(((std::min)(float(x + 1), r0) - (std::max)(float(x), l0)), 0.0F) * (ycenter - float(y));
						val += (std::max)(((std::min)(float(x + 1), r1) - (std::max)(float(x), l1)), 0.0F) * (float(y + 1) - ycenter);
						alpha_blend(*dst, c, val);
					}
				}
				++y;
			}

			// draw rows below the axis
			for ( ; maxy > y; ++y)
			{
				float const ycoord0 = ycoord1;
				float const xval0 = xval1;
				float const l0 = l1;
				float const r0 = r1;
				ycoord1 = float(y + 1) - ycenter;
				xval1 = std::sqrt((std::max)(1.0F - (ycoord1 * ycoord1) * ooyradius2, 0.0F));
				l1 = xcenter - (xval1 * xradius);
				r1 = xcenter + (xval1 * xradius);
				s32 minx = int(l0);
				s32 maxx = int(std::ceil(r0)) - 1;
				u32 *dst = &dest.pix(y, minx);
				for (s32 x = minx; maxx >= x; ++x, ++dst)
				{
					if ((float(x) >= l1) && (float(x + 1) <= r1))
					{
						if (255 <= a)
							*dst = f;
						else
							alpha_blend(*dst, a, r, g, b, inva);
					}
					else
					{
						float val = 0.0F;
						if (float(x + 1) <= l1)
							val += integral((std::max)((float(x) - xcenter) / xradius, -xval0), (float(x + 1) - xcenter) / xradius);
						else if (float(x) <= l1)
							val += integral((std::max)((float(x) - xcenter) / xradius, -xval0), -xval1);
						else if (float(x) >= r1)
							val += integral((float(x) - xcenter) / xradius, (std::min)((float(x + 1) - xcenter) / xradius, xval0));
						else if (float(x + 1) >= r1)
							val += integral(xval1, (std::min)((float(x + 1) - xcenter) / xradius, xval0));
						val *= scale;
						if (float(x) <= l1)
							val -= ((std::min)(float(x + 1), l1) - (std::max)(float(x), l0)) * ycoord0;
						else if (float(x + 1) >= r1)
							val -= ((std::min)(float(x + 1), r0) - (std::max)(float(x), r1)) * ycoord0;
						val += (std::max)((std::min)(float(x + 1), r1) - (std::max)(float(x), l1), 0.0F);
						alpha_blend(*dst, c, val);
					}
				}
			}

			// last row is an inversion of the first
			draw_edge_row(maxy, float(maxy) - ycenter, float(maxy) < ycenter);
		}
	}

private:
	static float integral(float x0, float x1)
	{
		return integral(x1) - integral(x0);
	}

	static float integral(float x)
	{
		float const u(2.0F * std::asin(x));
		return u + std::sin(u);
	};
};


// text string
class layout_element::text_component : public component
{
public:
	// construction/destruction
	text_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
		m_string = env.get_attribute_string(compnode, "string", "");
		m_textalign = env.get_attribute_int(compnode, "align", 0);
	}

protected:
	// overrides
	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		auto font = machine.render().font_alloc("default");
		draw_text(*font, dest, bounds, m_string, m_textalign, color(state));
	}

private:
	// internal state
	std::string         m_string;                   // string for text components
	int                 m_textalign;                // text alignment to box
};


// 7-segment LCD
class layout_element::led7seg_component : public component
{
public:
	// construction/destruction
	led7seg_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
	}

protected:
	// overrides
	virtual int maxstate() const override { return 255; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		rgb_t const onpen = rgb_t(0xff, 0xff, 0xff, 0xff);
		rgb_t const offpen = rgb_t(0x20, 0xff, 0xff, 0xff);

		// sizes for computation
		int const bmwidth = 250;
		int const bmheight = 400;
		int const segwidth = 40;
		int const skewwidth = 40;

		// allocate a temporary bitmap for drawing
		bitmap_argb32 tempbitmap(bmwidth + skewwidth, bmheight);
		tempbitmap.fill(rgb_t(0x00,0x00,0x00,0x00));

		// top bar
		draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2, segwidth, BIT(state, 0) ? onpen : offpen);

		// top-right bar
		draw_segment_vertical(tempbitmap, 0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2, segwidth, BIT(state, 1) ? onpen : offpen);

		// bottom-right bar
		draw_segment_vertical(tempbitmap, bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2, segwidth, BIT(state, 2) ? onpen : offpen);

		// bottom bar
		draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2, segwidth, BIT(state, 3) ? onpen : offpen);

		// bottom-left bar
		draw_segment_vertical(tempbitmap, bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2, segwidth, BIT(state, 4) ? onpen : offpen);

		// top-left bar
		draw_segment_vertical(tempbitmap, 0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2, segwidth, BIT(state, 5) ? onpen : offpen);

		// middle bar
		draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight/2, segwidth, BIT(state, 6) ? onpen : offpen);

		// apply skew
		apply_skew(tempbitmap, 40);

		// decimal point
		draw_segment_decimal(tempbitmap, bmwidth + segwidth/2, bmheight - segwidth/2, segwidth, BIT(state, 7) ? onpen : offpen);

		// resample to the target size
		render_resample_argb_bitmap_hq(dest, tempbitmap, color(state));
	}
};


// 8-segment fluorescent (Gottlieb System 1)
class layout_element::led8seg_gts1_component : public component
{
public:
	// construction/destruction
	led8seg_gts1_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
	}

protected:
	// overrides
	virtual int maxstate() const override { return 255; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		rgb_t const onpen = rgb_t(0xff, 0xff, 0xff, 0xff);
		rgb_t const offpen = rgb_t(0x20, 0xff, 0xff, 0xff);
		rgb_t const backpen = rgb_t(0x00, 0x00, 0x00, 0x00);

		// sizes for computation
		int const bmwidth = 250;
		int const bmheight = 400;
		int const segwidth = 40;
		int const skewwidth = 40;

		// allocate a temporary bitmap for drawing
		bitmap_argb32 tempbitmap(bmwidth + skewwidth, bmheight);
		tempbitmap.fill(backpen);

		// top bar
		draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2, segwidth, (state & (1 << 0)) ? onpen : offpen);

		// top-right bar
		draw_segment_vertical(tempbitmap, 0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2, segwidth, (state & (1 << 1)) ? onpen : offpen);

		// bottom-right bar
		draw_segment_vertical(tempbitmap, bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2, segwidth, (state & (1 << 2)) ? onpen : offpen);

		// bottom bar
		draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2, segwidth, (state & (1 << 3)) ? onpen : offpen);

		// bottom-left bar
		draw_segment_vertical(tempbitmap, bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2, segwidth, (state & (1 << 4)) ? onpen : offpen);

		// top-left bar
		draw_segment_vertical(tempbitmap, 0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2, segwidth, (state & (1 << 5)) ? onpen : offpen);

		// horizontal bars
		draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, 2*bmwidth/3 - 2*segwidth/3, bmheight/2, segwidth, (state & (1 << 6)) ? onpen : offpen);
		draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3 + bmwidth/2, bmwidth - 2*segwidth/3, bmheight/2, segwidth, (state & (1 << 6)) ? onpen : offpen);

		// vertical bars
		draw_segment_vertical(tempbitmap, 0 + segwidth/3 - 8, bmheight/2 - segwidth/3 + 2, 2*bmwidth/3 - segwidth/2 - 4, segwidth + 8, backpen);
		draw_segment_vertical(tempbitmap, 0 + segwidth/3, bmheight/2 - segwidth/3, 2*bmwidth/3 - segwidth/2 - 4, segwidth, (state & (1 << 7)) ? onpen : offpen);

		draw_segment_vertical(tempbitmap, bmheight/2 + segwidth/3 - 2, bmheight - segwidth/3 + 8, 2*bmwidth/3 - segwidth/2 - 4, segwidth + 8, backpen);
		draw_segment_vertical(tempbitmap, bmheight/2 + segwidth/3, bmheight - segwidth/3, 2*bmwidth/3 - segwidth/2 - 4, segwidth, (state & (1 << 7)) ? onpen : offpen);

		// apply skew
		apply_skew(tempbitmap, 40);

		// resample to the target size
		render_resample_argb_bitmap_hq(dest, tempbitmap, color(state));
	}
};


// 14-segment LCD
class layout_element::led14seg_component : public component
{
public:
	// construction/destruction
	led14seg_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
	}

protected:
	// overrides
	virtual int maxstate() const override { return 16383; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		rgb_t const onpen = rgb_t(0xff, 0xff, 0xff, 0xff);
		rgb_t const offpen = rgb_t(0x20, 0xff, 0xff, 0xff);

		// sizes for computation
		int const bmwidth = 250;
		int const bmheight = 400;
		int const segwidth = 40;
		int const skewwidth = 40;

		// allocate a temporary bitmap for drawing
		bitmap_argb32 tempbitmap(bmwidth + skewwidth, bmheight);
		tempbitmap.fill(rgb_t(0x00, 0x00, 0x00, 0x00));

		// top bar
		draw_segment_horizontal(tempbitmap,
				0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2,
				segwidth, (state & (1 << 0)) ? onpen : offpen);

		// right-top bar
		draw_segment_vertical(tempbitmap,
				0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
				segwidth, (state & (1 << 1)) ? onpen : offpen);

		// right-bottom bar
		draw_segment_vertical(tempbitmap,
				bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
				segwidth, (state & (1 << 2)) ? onpen : offpen);

		// bottom bar
		draw_segment_horizontal(tempbitmap,
				0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
				segwidth, (state & (1 << 3)) ? onpen : offpen);

		// left-bottom bar
		draw_segment_vertical(tempbitmap,
				bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
				segwidth, (state & (1 << 4)) ? onpen : offpen);

		// left-top bar
		draw_segment_vertical(tempbitmap,
				0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
				segwidth, (state & (1 << 5)) ? onpen : offpen);

		// horizontal-middle-left bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
				segwidth, LINE_CAP_START, (state & (1 << 6)) ? onpen : offpen);

		// horizontal-middle-right bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
				segwidth, LINE_CAP_END, (state & (1 << 7)) ? onpen : offpen);

		// vertical-middle-top bar
		draw_segment_vertical_caps(tempbitmap,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
				segwidth, LINE_CAP_NONE, (state & (1 << 8)) ? onpen : offpen);

		// vertical-middle-bottom bar
		draw_segment_vertical_caps(tempbitmap,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
				segwidth, LINE_CAP_NONE, (state & (1 << 9)) ? onpen : offpen);

		// diagonal-left-bottom bar
		draw_segment_diagonal_1(tempbitmap,
				0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
				segwidth, (state & (1 << 10)) ? onpen : offpen);

		// diagonal-left-top bar
		draw_segment_diagonal_2(tempbitmap,
				0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
				segwidth, (state & (1 << 11)) ? onpen : offpen);

		// diagonal-right-top bar
		draw_segment_diagonal_1(tempbitmap,
				bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
				segwidth, (state & (1 << 12)) ? onpen : offpen);

		// diagonal-right-bottom bar
		draw_segment_diagonal_2(tempbitmap,
				bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
				segwidth, (state & (1 << 13)) ? onpen : offpen);

		// apply skew
		apply_skew(tempbitmap, 40);

		// resample to the target size
		render_resample_argb_bitmap_hq(dest, tempbitmap, color(state));
	}
};


// 16-segment LCD
class layout_element::led16seg_component : public component
{
public:
	// construction/destruction
	led16seg_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
	}

protected:
	// overrides
	virtual int maxstate() const override { return 65535; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		const rgb_t onpen = rgb_t(0xff, 0xff, 0xff, 0xff);
		const rgb_t offpen = rgb_t(0x20, 0xff, 0xff, 0xff);

		// sizes for computation
		int bmwidth = 250;
		int bmheight = 400;
		int segwidth = 40;
		int skewwidth = 40;

		// allocate a temporary bitmap for drawing
		bitmap_argb32 tempbitmap(bmwidth + skewwidth, bmheight);
		tempbitmap.fill(rgb_t(0x00, 0x00, 0x00, 0x00));

		// top-left bar
		draw_segment_horizontal_caps(tempbitmap,
			0 + 2*segwidth/3, bmwidth/2 - segwidth/10, 0 + segwidth/2,
			segwidth, LINE_CAP_START, (state & (1 << 0)) ? onpen : offpen);

		// top-right bar
		draw_segment_horizontal_caps(tempbitmap,
			0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, 0 + segwidth/2,
			segwidth, LINE_CAP_END, (state & (1 << 1)) ? onpen : offpen);

		// right-top bar
		draw_segment_vertical(tempbitmap,
			0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
			segwidth, (state & (1 << 2)) ? onpen : offpen);

		// right-bottom bar
		draw_segment_vertical(tempbitmap,
			bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
			segwidth, (state & (1 << 3)) ? onpen : offpen);

		// bottom-right bar
		draw_segment_horizontal_caps(tempbitmap,
			0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
			segwidth, LINE_CAP_END, (state & (1 << 4)) ? onpen : offpen);

		// bottom-left bar
		draw_segment_horizontal_caps(tempbitmap,
			0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight - segwidth/2,
			segwidth, LINE_CAP_START, (state & (1 << 5)) ? onpen : offpen);

		// left-bottom bar
		draw_segment_vertical(tempbitmap,
			bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
			segwidth, (state & (1 << 6)) ? onpen : offpen);

		// left-top bar
		draw_segment_vertical(tempbitmap,
			0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
			segwidth, (state & (1 << 7)) ? onpen : offpen);

		// horizontal-middle-left bar
		draw_segment_horizontal_caps(tempbitmap,
			0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
			segwidth, LINE_CAP_START, (state & (1 << 8)) ? onpen : offpen);

		// horizontal-middle-right bar
		draw_segment_horizontal_caps(tempbitmap,
			0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
			segwidth, LINE_CAP_END, (state & (1 << 9)) ? onpen : offpen);

		// vertical-middle-top bar
		draw_segment_vertical_caps(tempbitmap,
			0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
			segwidth, LINE_CAP_NONE, (state & (1 << 10)) ? onpen : offpen);

		// vertical-middle-bottom bar
		draw_segment_vertical_caps(tempbitmap,
			bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
			segwidth, LINE_CAP_NONE, (state & (1 << 11)) ? onpen : offpen);

		// diagonal-left-bottom bar
		draw_segment_diagonal_1(tempbitmap,
			0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
			bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
			segwidth, (state & (1 << 12)) ? onpen : offpen);

		// diagonal-left-top bar
		draw_segment_diagonal_2(tempbitmap,
			0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
			0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
			segwidth, (state & (1 << 13)) ? onpen : offpen);

		// diagonal-right-top bar
		draw_segment_diagonal_1(tempbitmap,
			bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
			0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
			segwidth, (state & (1 << 14)) ? onpen : offpen);

		// diagonal-right-bottom bar
		draw_segment_diagonal_2(tempbitmap,
			bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
			bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
			segwidth, (state & (1 << 15)) ? onpen : offpen);

		// apply skew
		apply_skew(tempbitmap, 40);

		// resample to the target size
		render_resample_argb_bitmap_hq(dest, tempbitmap, color(state));
	}
};


// 14-segment LCD with semicolon (2 extra segments)
class layout_element::led14segsc_component : public component
{
public:
	// construction/destruction
	led14segsc_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
	}

protected:
	// overrides
	virtual int maxstate() const override { return 65535; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		rgb_t const onpen = rgb_t(0xff, 0xff, 0xff, 0xff);
		rgb_t const offpen = rgb_t(0x20, 0xff, 0xff, 0xff);

		// sizes for computation
		int const bmwidth = 250;
		int const bmheight = 400;
		int const segwidth = 40;
		int const skewwidth = 40;

		// allocate a temporary bitmap for drawing, adding some extra space for the tail
		bitmap_argb32 tempbitmap(bmwidth + skewwidth, bmheight + segwidth);
		tempbitmap.fill(rgb_t(0x00, 0x00, 0x00, 0x00));

		// top bar
		draw_segment_horizontal(tempbitmap,
				0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2,
				segwidth, (state & (1 << 0)) ? onpen : offpen);

		// right-top bar
		draw_segment_vertical(tempbitmap,
				0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
				segwidth, (state & (1 << 1)) ? onpen : offpen);

		// right-bottom bar
		draw_segment_vertical(tempbitmap,
				bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
				segwidth, (state & (1 << 2)) ? onpen : offpen);

		// bottom bar
		draw_segment_horizontal(tempbitmap,
				0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
				segwidth, (state & (1 << 3)) ? onpen : offpen);

		// left-bottom bar
		draw_segment_vertical(tempbitmap,
				bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
				segwidth, (state & (1 << 4)) ? onpen : offpen);

		// left-top bar
		draw_segment_vertical(tempbitmap,
				0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
				segwidth, (state & (1 << 5)) ? onpen : offpen);

		// horizontal-middle-left bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
				segwidth, LINE_CAP_START, (state & (1 << 6)) ? onpen : offpen);

		// horizontal-middle-right bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
				segwidth, LINE_CAP_END, (state & (1 << 7)) ? onpen : offpen);

		// vertical-middle-top bar
		draw_segment_vertical_caps(tempbitmap,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
				segwidth, LINE_CAP_NONE, (state & (1 << 8)) ? onpen : offpen);

		// vertical-middle-bottom bar
		draw_segment_vertical_caps(tempbitmap,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
				segwidth, LINE_CAP_NONE, (state & (1 << 9)) ? onpen : offpen);

		// diagonal-left-bottom bar
		draw_segment_diagonal_1(tempbitmap,
				0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
				segwidth, (state & (1 << 10)) ? onpen : offpen);

		// diagonal-left-top bar
		draw_segment_diagonal_2(tempbitmap,
				0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
				segwidth, (state & (1 << 11)) ? onpen : offpen);

		// diagonal-right-top bar
		draw_segment_diagonal_1(tempbitmap,
				bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
				segwidth, (state & (1 << 12)) ? onpen : offpen);

		// diagonal-right-bottom bar
		draw_segment_diagonal_2(tempbitmap,
				bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
				segwidth, (state & (1 << 13)) ? onpen : offpen);

		// apply skew
		apply_skew(tempbitmap, 40);

		// comma tail
		draw_segment_diagonal_1(tempbitmap,
				bmwidth - (segwidth/2), bmwidth + segwidth,
				bmheight - (segwidth), bmheight + segwidth*1.5,
				segwidth/2, (state & (1 << 15)) ? onpen : offpen);

		// decimal point
		draw_segment_decimal(tempbitmap,
				bmwidth + segwidth/2, bmheight - segwidth/2,
				segwidth, (state & (1 << 14)) ? onpen : offpen);

		// resample to the target size
		render_resample_argb_bitmap_hq(dest, tempbitmap, color(state));
	}
};


// 16-segment LCD with semicolon (2 extra segments)
class layout_element::led16segsc_component : public component
{
public:
	// construction/destruction
	led16segsc_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
	{
	}

protected:
	// overrides
	virtual int maxstate() const override { return 262143; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		rgb_t const onpen = rgb_t(0xff, 0xff, 0xff, 0xff);
		rgb_t const offpen = rgb_t(0x20, 0xff, 0xff, 0xff);

		// sizes for computation
		int const bmwidth = 250;
		int const bmheight = 400;
		int const segwidth = 40;
		int const skewwidth = 40;

		// allocate a temporary bitmap for drawing
		bitmap_argb32 tempbitmap(bmwidth + skewwidth, bmheight + segwidth);
		tempbitmap.fill(rgb_t(0x00, 0x00, 0x00, 0x00));

		// top-left bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + 2*segwidth/3, bmwidth/2 - segwidth/10, 0 + segwidth/2,
				segwidth, LINE_CAP_START, (state & (1 << 0)) ? onpen : offpen);

		// top-right bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, 0 + segwidth/2,
				segwidth, LINE_CAP_END, (state & (1 << 1)) ? onpen : offpen);

		// right-top bar
		draw_segment_vertical(tempbitmap,
				0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
				segwidth, (state & (1 << 2)) ? onpen : offpen);

		// right-bottom bar
		draw_segment_vertical(tempbitmap,
				bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
				segwidth, (state & (1 << 3)) ? onpen : offpen);

		// bottom-right bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
				segwidth, LINE_CAP_END, (state & (1 << 4)) ? onpen : offpen);

		// bottom-left bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight - segwidth/2,
				segwidth, LINE_CAP_START, (state & (1 << 5)) ? onpen : offpen);

		// left-bottom bar
		draw_segment_vertical(tempbitmap,
				bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
				segwidth, (state & (1 << 6)) ? onpen : offpen);

		// left-top bar
		draw_segment_vertical(tempbitmap,
				0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
				segwidth, (state & (1 << 7)) ? onpen : offpen);

		// horizontal-middle-left bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
				segwidth, LINE_CAP_START, (state & (1 << 8)) ? onpen : offpen);

		// horizontal-middle-right bar
		draw_segment_horizontal_caps(tempbitmap,
				0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
				segwidth, LINE_CAP_END, (state & (1 << 9)) ? onpen : offpen);

		// vertical-middle-top bar
		draw_segment_vertical_caps(tempbitmap,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
				segwidth, LINE_CAP_NONE, (state & (1 << 10)) ? onpen : offpen);

		// vertical-middle-bottom bar
		draw_segment_vertical_caps(tempbitmap,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
				segwidth, LINE_CAP_NONE, (state & (1 << 11)) ? onpen : offpen);

		// diagonal-left-bottom bar
		draw_segment_diagonal_1(tempbitmap,
				0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
				segwidth, (state & (1 << 12)) ? onpen : offpen);

		// diagonal-left-top bar
		draw_segment_diagonal_2(tempbitmap,
				0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
				segwidth, (state & (1 << 13)) ? onpen : offpen);

		// diagonal-right-top bar
		draw_segment_diagonal_1(tempbitmap,
				bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
				0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
				segwidth, (state & (1 << 14)) ? onpen : offpen);

		// diagonal-right-bottom bar
		draw_segment_diagonal_2(tempbitmap,
				bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
				bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
				segwidth, (state & (1 << 15)) ? onpen : offpen);

		// apply skew
		apply_skew(tempbitmap, 40);

		// comma tail
		draw_segment_diagonal_1(tempbitmap,
				bmwidth - (segwidth/2), bmwidth + segwidth, bmheight - (segwidth), bmheight + segwidth*1.5,
				segwidth/2, (state & (1 << 17)) ? onpen : offpen);

		// decimal point (draw last for priority)
		draw_segment_decimal(tempbitmap,
				bmwidth + segwidth/2, bmheight - segwidth/2,
				segwidth, (state & (1 << 16)) ? onpen : offpen);

		// resample to the target size
		render_resample_argb_bitmap_hq(dest, tempbitmap, color(state));
	}
};


// row of dots for a dotmatrix
class layout_element::dotmatrix_component : public component
{
public:
	// construction/destruction
	dotmatrix_component(int dots, environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
		, m_dots(dots)
	{
	}

protected:
	// overrides
	virtual int maxstate() const override { return (1 << m_dots) - 1; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		const rgb_t onpen = rgb_t(0xff, 0xff, 0xff, 0xff);
		const rgb_t offpen = rgb_t(0xff, 0x20, 0x20, 0x20);

		// sizes for computation
		int bmheight = 300;
		int dotwidth = 250;

		// allocate a temporary bitmap for drawing
		bitmap_argb32 tempbitmap(dotwidth*m_dots, bmheight);
		tempbitmap.fill(rgb_t(0xff, 0x00, 0x00, 0x00));

		for (int i = 0; i < m_dots; i++)
			draw_segment_decimal(tempbitmap, ((dotwidth / 2) + (i * dotwidth)), bmheight / 2, dotwidth, BIT(state, i) ? onpen : offpen);

		// resample to the target size
		render_resample_argb_bitmap_hq(dest, tempbitmap, color(state));
	}

private:
	// internal state
	int m_dots;
};


// simple counter
class layout_element::simplecounter_component : public component
{
public:
	// construction/destruction
	simplecounter_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
		, m_digits(env.get_attribute_int(compnode, "digits", 2))
		, m_textalign(env.get_attribute_int(compnode, "align", 0))
		, m_maxstate(env.get_attribute_int(compnode, "maxstate", 999))
	{
	}

protected:
	// overrides
	virtual int maxstate() const override { return m_maxstate; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		auto font = machine.render().font_alloc("default");
		draw_text(*font, dest, bounds, string_format("%0*d", m_digits, state), m_textalign, color(state));
	}

private:
	// internal state
	int const   m_digits;       // number of digits for simple counters
	int const   m_textalign;    // text alignment to box
	int const   m_maxstate;
};


// fruit machine reel
class layout_element::reel_component : public component
{
	static constexpr unsigned MAX_BITMAPS = 32;

public:
	// construction/destruction
	reel_component(environment &env, util::xml::data_node const &compnode)
		: component(env, compnode)
		, m_searchpath(env.search_path() ? env.search_path() : "")
		, m_dirname(env.directory_name() ? env.directory_name() : "")
	{
		std::string symbollist = env.get_attribute_string(compnode, "symbollist", "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15");

		// split out position names from string and figure out our number of symbols
		m_numstops = 0;
		for (std::string::size_type location = symbollist.find(','); std::string::npos != location; location = symbollist.find(','))
		{
			m_stopnames[m_numstops] = symbollist.substr(0, location);
			symbollist.erase(0, location + 1);
			m_numstops++;
		}
		m_stopnames[m_numstops++] = symbollist;

		for (int i = 0; i < m_numstops; i++)
		{
			std::string::size_type const location = m_stopnames[i].find(':');
			if (location != std::string::npos)
			{
				m_imagefile[i] = m_stopnames[i].substr(location + 1);
				m_stopnames[i].erase(location);
			}
		}

		m_stateoffset = env.get_attribute_int(compnode, "stateoffset", 0);
		m_numsymbolsvisible = env.get_attribute_int(compnode, "numsymbolsvisible", 3);
		m_reelreversed = env.get_attribute_int(compnode, "reelreversed", 0);
		m_beltreel = env.get_attribute_int(compnode, "beltreel", 0);
	}

	// overrides
	virtual void preload(running_machine &machine) override
	{
		for (int i = 0; i < m_numstops; i++)
		{
			if (!m_imagefile[i].empty() && !m_bitmap[i].valid())
				load_reel_bitmap(i);
		}

	}

protected:
	virtual int maxstate() const override { return 65535; }

	virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state) override
	{
		if (m_beltreel)
		{
			draw_beltreel(machine, dest, bounds, state);
			return;
		}

		// state is a normalized value between 0 and 65536 so that we don't need to worry about how many motor steps here or in the .lay, only the number of symbols
		const int max_state_used = 0x10000;

		// shift the reels a bit based on this param, allows fine tuning
		int use_state = (state + m_stateoffset) % max_state_used;

		// compute premultiplied colors
		render_color const c(color(state));
		u32 const r = c.r * 255.0f;
		u32 const g = c.g * 255.0f;
		u32 const b = c.b * 255.0f;
		u32 const a = c.a * 255.0f;

		int curry = 0;
		int num_shown = m_numsymbolsvisible;

		int ourheight = bounds.height();

		auto font = machine.render().font_alloc("default");
		for (int fruit = 0;fruit<m_numstops;fruit++)
		{
			int basey;

			if (m_reelreversed)
			{
				basey = bounds.top() + ((use_state)*(ourheight/num_shown)/(max_state_used/m_numstops)) + curry;
			}
			else
			{
				basey = bounds.top() - ((use_state)*(ourheight/num_shown)/(max_state_used/m_numstops)) + curry;
			}

			// wrap around...
			if (basey < bounds.top())
				basey += ((max_state_used)*(ourheight/num_shown)/(max_state_used/m_numstops));
			if (basey > bounds.bottom())
				basey -= ((max_state_used)*(ourheight/num_shown)/(max_state_used/m_numstops));

			int endpos = basey+ourheight/num_shown;

			// only render the symbol / text if it's actually in view because the code is SLOW
			if ((endpos >= bounds.top()) && (basey <= bounds.bottom()))
			{
				if (!m_imagefile[fruit].empty() && !m_bitmap[fruit].valid())
					load_reel_bitmap(fruit);

				if (m_bitmap[fruit].valid()) // render gfx
				{
					bitmap_argb32 tempbitmap2(dest.width(), ourheight/num_shown);
					render_resample_argb_bitmap_hq(tempbitmap2, m_bitmap[fruit], c);

					for (int y = 0; y < ourheight/num_shown; y++)
					{
						int effy = basey + y;

						if (effy >= bounds.top() && effy <= bounds.bottom())
						{
							u32 const *const src = &tempbitmap2.pix(y);
							u32 *const d = &dest.pix(effy);
							for (int x = 0; x < dest.width(); x++)
							{
								int effx = x;
								if (effx >= bounds.left() && effx <= bounds.right())
								{
									u32 spix = rgb_t(src[x]).a();
									if (spix != 0)
									{
										d[effx] = src[x];
									}
								}
							}
						}
					}
				}
				else // render text (fallback)
				{
					// allocate a temporary bitmap
					bitmap_argb32 tempbitmap(dest.width(), dest.height());

					// get the width of the string
					float aspect = 1.0f;
					s32 width;

					while (1)
					{
						width = font->string_width(ourheight / num_shown, aspect, m_stopnames[fruit]);
						if (width < bounds.width())
							break;
						aspect *= 0.9f;
					}

					s32 curx = bounds.left() + (bounds.width() - width) / 2;

					// loop over characters
					std::string_view s = m_stopnames[fruit];
					while (!s.empty())
					{
						char32_t schar;
						int scharcount = uchar_from_utf8(&schar, s);

						if (scharcount == -1)
							break;

						// get the font bitmap
						rectangle chbounds;
						font->get_scaled_bitmap_and_bounds(tempbitmap, ourheight/num_shown, aspect, schar, chbounds);

						// copy the data into the target
						for (int y = 0; y < chbounds.height(); y++)
						{
							int effy = basey + y;

							if (effy >= bounds.top() && effy <= bounds.bottom())
							{
								u32 const *const src = &tempbitmap.pix(y);
								u32 *const d = &dest.pix(effy);
								for (int x = 0; x < chbounds.width(); x++)
								{
									int effx = curx + x + chbounds.left();
									if (effx >= bounds.left() && effx <= bounds.right())
									{
										u32 spix = rgb_t(src[x]).a();
										if (spix != 0)
										{
											rgb_t dpix = d[effx];
											u32 ta = (a * (spix + 1)) >> 8;
											u32 tr = (r * ta + dpix.r() * (0x100 - ta)) >> 8;
											u32 tg = (g * ta + dpix.g() * (0x100 - ta)) >> 8;
											u32 tb = (b * ta + dpix.b() * (0x100 - ta)) >> 8;
											d[effx] = rgb_t(tr, tg, tb);
										}
									}
								}
							}
						}

						// advance in the X direction
						curx += font->char_width(ourheight/num_shown, aspect, schar);
						s.remove_prefix(scharcount);
					}
				}
			}

			curry += ourheight/num_shown;
		}

		// free the temporary bitmap and font
	}

private:
	// internal helpers
	void draw_beltreel(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state)
	{
		const int max_state_used = 0x10000;

		// shift the reels a bit based on this param, allows fine tuning
		int use_state = (state + m_stateoffset) % max_state_used;

		// compute premultiplied colors
		render_color const c(color(state));
		u32 const r = c.r * 255.0f;
		u32 const g = c.g * 255.0f;
		u32 const b = c.b * 255.0f;
		u32 const a = c.a * 255.0f;

		int currx = 0;
		int num_shown = m_numsymbolsvisible;

		int ourwidth = bounds.width();

		auto font = machine.render().font_alloc("default");
		for (int fruit = 0;fruit<m_numstops;fruit++)
		{
			int basex;
			if (m_reelreversed==1)
			{
				basex = bounds.min_x + ((use_state)*(ourwidth/num_shown)/(max_state_used/m_numstops)) + currx;
			}
			else
			{
				basex = bounds.min_x - ((use_state)*(ourwidth/num_shown)/(max_state_used/m_numstops)) + currx;
			}

			// wrap around...
			if (basex < bounds.left())
				basex += ((max_state_used)*(ourwidth/num_shown)/(max_state_used/m_numstops));
			if (basex > bounds.right())
				basex -= ((max_state_used)*(ourwidth/num_shown)/(max_state_used/m_numstops));

			int endpos = basex+(ourwidth/num_shown);

			// only render the symbol / text if it's actually in view because the code is SLOW
			if ((endpos >= bounds.left()) && (basex <= bounds.right()))
			{
				if (!m_imagefile[fruit].empty() && !m_bitmap[fruit].valid())
					load_reel_bitmap(fruit);

				if (m_bitmap[fruit].valid()) // render gfx
				{
					bitmap_argb32 tempbitmap2(ourwidth/num_shown, dest.height());
					render_resample_argb_bitmap_hq(tempbitmap2, m_bitmap[fruit], c);

					for (int y = 0; y < dest.height(); y++)
					{
						int effy = y;

						if (effy >= bounds.top() && effy <= bounds.bottom())
						{
							u32 const *const src = &tempbitmap2.pix(y);
							u32 *const d = &dest.pix(effy);
							for (int x = 0; x < ourwidth/num_shown; x++)
							{
								int effx = basex + x;
								if (effx >= bounds.left() && effx <= bounds.right())
								{
									u32 spix = rgb_t(src[x]).a();
									if (spix != 0)
									{
										d[effx] = src[x];
									}
								}
							}
						}

					}
				}
				else // render text (fallback)
				{
					// get the width of the string
					float aspect = 1.0f;
					s32 width;
					while (1)
					{
						width = font->string_width(dest.height(), aspect, m_stopnames[fruit]);
						if (width < bounds.width())
							break;
						aspect *= 0.9f;
					}

					s32 curx = bounds.left();

					// allocate a temporary bitmap
					bitmap_argb32 tempbitmap(dest.width(), dest.height());

					// loop over characters
					std::string_view s = m_stopnames[fruit];
					while (!s.empty())
					{
						char32_t schar;
						int scharcount = uchar_from_utf8(&schar, s);

						if (scharcount == -1)
							break;

						// get the font bitmap
						rectangle chbounds;
						font->get_scaled_bitmap_and_bounds(tempbitmap, dest.height(), aspect, schar, chbounds);

						// copy the data into the target
						for (int y = 0; y < chbounds.height(); y++)
						{
							int effy = y;

							if (effy >= bounds.top() && effy <= bounds.bottom())
							{
								u32 const *const src = &tempbitmap.pix(y);
								u32 *const d = &dest.pix(effy);
								for (int x = 0; x < chbounds.width(); x++)
								{
									int effx = basex + curx + x;
									if (effx >= bounds.left() && effx <= bounds.right())
									{
										u32 spix = rgb_t(src[x]).a();
										if (spix != 0)
										{
											rgb_t dpix = d[effx];
											u32 ta = (a * (spix + 1)) >> 8;
											u32 tr = (r * ta + dpix.r() * (0x100 - ta)) >> 8;
											u32 tg = (g * ta + dpix.g() * (0x100 - ta)) >> 8;
											u32 tb = (b * ta + dpix.b() * (0x100 - ta)) >> 8;
											d[effx] = rgb_t(tr, tg, tb);
										}
									}
								}
							}
						}

						// advance in the X direction
						curx += font->char_width(dest.height(), aspect, schar);
						s.remove_prefix(scharcount);
					}
				}
			}

			currx += ourwidth/num_shown;
		}

		// free the temporary bitmap and font
	}

	void load_reel_bitmap(int number)
	{
		emu_file file(m_searchpath.empty() ? m_dirname : m_searchpath, OPEN_FLAG_READ);
		std::string filename;
		if (!m_searchpath.empty())
			filename = m_dirname;
		if (!filename.empty() && !util::is_directory_separator(filename[filename.size() - 1]))
			filename.append(PATH_SEPARATOR);
		filename.append(m_imagefile[number]);

		// load the basic bitmap
		if (file.open(filename) == osd_file::error::NONE)
			render_load_png(m_bitmap[number], file);

		// if we can't load the bitmap just use text rendering
		if (!m_bitmap[number].valid())
			m_imagefile[number].clear();

	}

	// internal state
	bitmap_argb32       m_bitmap[MAX_BITMAPS];      // source bitmap for images
	std::string         m_searchpath;               // asset search path (for lazy loading)
	std::string         m_dirname;                  // directory name of image file (for lazy loading)
	std::string         m_imagefile[MAX_BITMAPS];   // name of the image file (for lazy loading)

	// basically made up of multiple text strings / gfx
	int                 m_numstops;
	std::string         m_stopnames[MAX_BITMAPS];
	int                 m_stateoffset;
	int                 m_reelreversed;
	int                 m_numsymbolsvisible;
	int                 m_beltreel;
};


//-------------------------------------------------
//  make_component - create component of given type
//-------------------------------------------------

template <typename T>
layout_element::component::ptr layout_element::make_component(environment &env, util::xml::data_node const &compnode)
{
	return std::make_unique<T>(env, compnode);
}


//-------------------------------------------------
//  make_component - create dotmatrix component
//  with given vertical resolution
//-------------------------------------------------

template <int D>
layout_element::component::ptr layout_element::make_dotmatrix_component(environment &env, util::xml::data_node const &compnode)
{
	return std::make_unique<dotmatrix_component>(D, env, compnode);
}



//**************************************************************************
//  LAYOUT ELEMENT TEXTURE
//**************************************************************************

//-------------------------------------------------
//  texture - constructors
//-------------------------------------------------

layout_element::texture::texture()
	: m_element(nullptr)
	, m_texture(nullptr)
	, m_state(0)
{
}


layout_element::texture::texture(texture &&that) : texture()
{
	operator=(std::move(that));
}


//-------------------------------------------------
//  ~texture - destructor
//-------------------------------------------------

layout_element::texture::~texture()
{
	if (m_element != nullptr)
		m_element->machine().render().texture_free(m_texture);
}


//-------------------------------------------------
//  opearator= - move assignment
//-------------------------------------------------

layout_element::texture &layout_element::texture::operator=(texture &&that)
{
	using std::swap;
	swap(m_element, that.m_element);
	swap(m_texture, that.m_texture);
	swap(m_state, that.m_state);
	return *this;
}



//**************************************************************************
//  LAYOUT ELEMENT COMPONENT
//**************************************************************************

//-------------------------------------------------
//  component - constructor
//-------------------------------------------------

layout_element::component::component(environment &env, util::xml::data_node const &compnode)
	: m_statemask(env.get_attribute_int(compnode, "statemask", env.get_attribute_string(compnode, "state", "")[0] ? ~0 : 0))
	, m_stateval(env.get_attribute_int(compnode, "state", m_statemask) & m_statemask)
{
	for (util::xml::data_node const *child = compnode.get_first_child(); child; child = child->get_next_sibling())
	{
		if (!strcmp(child->get_name(), "bounds"))
		{
			if (!add_bounds_step(env, m_bounds, *child))
			{
				throw layout_syntax_error(
						util::string_format(
							"%s component has duplicate bounds for state",
							compnode.get_name()));
			}
		}
		else if (!strcmp(child->get_name(), "color"))
		{
			if (!add_color_step(env, m_color, *child))
			{
				throw layout_syntax_error(
						util::string_format(
							"%s component has duplicate color for state",
							compnode.get_name()));
			}
		}
	}
	set_bounds_deltas(m_bounds);
	set_color_deltas(m_color);
}


//-------------------------------------------------
//  normalize_bounds - normalize component bounds
//-------------------------------------------------

void layout_element::component::normalize_bounds(float xoffs, float yoffs, float xscale, float yscale)
{
	::normalize_bounds(m_bounds, 0.0F, 0.0F, xoffs, yoffs, xscale, yscale);
}


//-------------------------------------------------
//  statewrap - get state wraparound requirements
//-------------------------------------------------

std::pair<int, bool> layout_element::component::statewrap() const
{
	int result(0);
	bool fold;
	auto const adjustmask =
			[&result, &fold] (int val, int mask)
			{
				assert(!(val & ~mask));
				auto const splatright =
						[] (int x)
						{
							for (unsigned shift = 1; (sizeof(x) * 4) >= shift; shift <<= 1)
								x |= (x >> shift);
							return x;
						};
				int const unfolded(splatright(mask));
				int const folded(splatright(~mask | splatright(val)));
				if (unsigned(folded) < unsigned(unfolded))
				{
					result |= folded;
					fold = true;
				}
				else
				{
					result |= unfolded;
				}
			};
	adjustmask(stateval(), statemask());
	int max(maxstate());
	if (m_bounds.size() > 1U)
		max = (std::max)(max, m_bounds.back().state);
	if (m_color.size() > 1U)
		max = (std::max)(max, m_color.back().state);
	if (0 <= max)
		adjustmask(max, ~0);
	return std::make_pair(result, fold);
}


//-------------------------------------------------
//  overall_bounds - maximum bounds for all states
//-------------------------------------------------

render_bounds layout_element::component::overall_bounds() const
{
	return accumulate_bounds(m_bounds);
}


//-------------------------------------------------
//  bounds - bounds for a given state
//-------------------------------------------------

render_bounds layout_element::component::bounds(int state) const
{
	return interpolate_bounds(m_bounds, state);
}


//-------------------------------------------------
//  color - color for a given state
//-------------------------------------------------

render_color layout_element::component::color(int state) const
{
	return interpolate_color(m_color, state);
}


//-------------------------------------------------
//  preload - perform expensive operations upfront
//-------------------------------------------------

void layout_element::component::preload(running_machine &machine)
{
}


//-------------------------------------------------
//  draw - draw element to texture for a given
//  state
//-------------------------------------------------

void layout_element::component::draw(running_machine &machine, bitmap_argb32 &dest, int state)
{
	// get the local scaled bounds
	render_bounds const curbounds(bounds(state));
	rectangle pixelbounds(
			s32(curbounds.x0 * float(dest.width()) + 0.5F),
			s32(floorf(curbounds.x1 * float(dest.width()) - 0.5F)),
			s32(curbounds.y0 * float(dest.height()) + 0.5F),
			s32(floorf(curbounds.y1 * float(dest.height()) - 0.5F)));

	// based on the component type, add to the texture
	if (!pixelbounds.empty())
		draw_aligned(machine, dest, pixelbounds, state);
}


void layout_element::component::draw_aligned(running_machine &machine, bitmap_argb32 &dest, rectangle const &bounds, int state)
{
	// derived classes must override one form or other
	throw false;
}


//-------------------------------------------------
//  maxstate - maximum state drawn differently
//-------------------------------------------------

int layout_element::component::maxstate() const
{
	return -1;
}


//-------------------------------------------------
//  draw_text - draw text in the specified color
//-------------------------------------------------

void layout_element::component::draw_text(
		render_font &font,
		bitmap_argb32 &dest,
		const rectangle &bounds,
		std::string_view str,
		int align,
		const render_color &color)
{
	// compute premultiplied colors
	u32 const r(color.r * 255.0f);
	u32 const g(color.g * 255.0f);
	u32 const b(color.b * 255.0f);
	u32 const a(color.a * 255.0f);

	// get the width of the string
	float aspect = 1.0f;
	s32 width;

	while (1)
	{
		width = font.string_width(bounds.height(), aspect, str);
		if (width < bounds.width())
			break;
		aspect *= 0.9f;
	}

	// get alignment
	s32 curx;
	switch (align)
	{
		// left
		case 1:
			curx = bounds.left();
			break;

		// right
		case 2:
			curx = bounds.right() - width;
			break;

		// default to center
		default:
			curx = bounds.left() + (bounds.width() - width) / 2;
			break;
	}

	// allocate a temporary bitmap
	bitmap_argb32 tempbitmap(dest.width(), dest.height());

	// loop over characters
	while (!str.empty())
	{
		char32_t schar;
		int scharcount = uchar_from_utf8(&schar, str);

		if (scharcount == -1)
			break;

		// get the font bitmap
		rectangle chbounds;
		font.get_scaled_bitmap_and_bounds(tempbitmap, bounds.height(), aspect, schar, chbounds);

		// copy the data into the target
		for (int y = 0; y < chbounds.height(); y++)
		{
			int effy = bounds.top() + y;
			if (effy >= bounds.top() && effy <= bounds.bottom())
			{
				u32 const *const src = &tempbitmap.pix(y);
				u32 *const d = &dest.pix(effy);
				for (int x = 0; x < chbounds.width(); x++)
				{
					int effx = curx + x + chbounds.left();
					if (effx >= bounds.left() && effx <= bounds.right())
					{
						u32 spix = rgb_t(src[x]).a();
						if (spix != 0)
						{
							rgb_t dpix = d[effx];
							u32 ta = (a * (spix + 1)) >> 8;
							u32 tr = (r * ta + dpix.r() * (0x100 - ta)) >> 8;
							u32 tg = (g * ta + dpix.g() * (0x100 - ta)) >> 8;
							u32 tb = (b * ta + dpix.b() * (0x100 - ta)) >> 8;
							d[effx] = rgb_t(tr, tg, tb);
						}
					}
				}
			}
		}

		// advance in the X direction
		curx += font.char_width(bounds.height(), aspect, schar);
		str.remove_prefix(scharcount);
	}
}


//-------------------------------------------------
//  draw_segment_horizontal_caps - draw a
//  horizontal LED segment with definable end
//  and start points
//-------------------------------------------------

void layout_element::component::draw_segment_horizontal_caps(bitmap_argb32 &dest, int minx, int maxx, int midy, int width, int caps, rgb_t color)
{
	// loop over the width of the segment
	for (int y = 0; y < width / 2; y++)
	{
		u32 *const d0 = &dest.pix(midy - y);
		u32 *const d1 = &dest.pix(midy + y);
		int ty = (y < width / 8) ? width / 8 : y;

		// loop over the length of the segment
		for (int x = minx + ((caps & LINE_CAP_START) ? ty : 0); x < maxx - ((caps & LINE_CAP_END) ? ty : 0); x++)
			d0[x] = d1[x] = color;
	}
}


//-------------------------------------------------
//  draw_segment_horizontal - draw a horizontal
//  LED segment
//-------------------------------------------------

void layout_element::component::draw_segment_horizontal(bitmap_argb32 &dest, int minx, int maxx, int midy, int width, rgb_t color)
{
	draw_segment_horizontal_caps(dest, minx, maxx, midy, width, LINE_CAP_START | LINE_CAP_END, color);
}


//-------------------------------------------------
//  draw_segment_vertical_caps - draw a
//  vertical LED segment with definable end
//  and start points
//-------------------------------------------------

void layout_element::component::draw_segment_vertical_caps(bitmap_argb32 &dest, int miny, int maxy, int midx, int width, int caps, rgb_t color)
{
	// loop over the width of the segment
	for (int x = 0; x < width / 2; x++)
	{
		u32 *const d0 = &dest.pix(0, midx - x);
		u32 *const d1 = &dest.pix(0, midx + x);
		int tx = (x < width / 8) ? width / 8 : x;

		// loop over the length of the segment
		for (int y = miny + ((caps & LINE_CAP_START) ? tx : 0); y < maxy - ((caps & LINE_CAP_END) ? tx : 0); y++)
			d0[y * dest.rowpixels()] = d1[y * dest.rowpixels()] = color;
	}
}


//-------------------------------------------------
//  draw_segment_vertical - draw a vertical
//  LED segment
//-------------------------------------------------

void layout_element::component::draw_segment_vertical(bitmap_argb32 &dest, int miny, int maxy, int midx, int width, rgb_t color)
{
	draw_segment_vertical_caps(dest, miny, maxy, midx, width, LINE_CAP_START | LINE_CAP_END, color);
}


//-------------------------------------------------
//  draw_segment_diagonal_1 - draw a diagonal
//  LED segment that looks like a backslash
//-------------------------------------------------

void layout_element::component::draw_segment_diagonal_1(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	// compute parameters
	width *= 1.5;
	float ratio = (maxy - miny - width) / (float)(maxx - minx);

	// draw line
	for (int x = minx; x < maxx; x++)
		if (x >= 0 && x < dest.width())
		{
			u32 *const d = &dest.pix(0, x);
			int step = (x - minx) * ratio;

			for (int y = maxy - width - step; y < maxy - step; y++)
				if (y >= 0 && y < dest.height())
					d[y * dest.rowpixels()] = color;
		}
}


//-------------------------------------------------
//  draw_segment_diagonal_2 - draw a diagonal
//  LED segment that looks like a forward slash
//-------------------------------------------------

void layout_element::component::draw_segment_diagonal_2(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	// compute parameters
	width *= 1.5;
	float ratio = (maxy - miny - width) / (float)(maxx - minx);

	// draw line
	for (int x = minx; x < maxx; x++)
		if (x >= 0 && x < dest.width())
		{
			u32 *const d = &dest.pix(0, x);
			int step = (x - minx) * ratio;

			for (int y = miny + step; y < miny + step + width; y++)
				if (y >= 0 && y < dest.height())
					d[y * dest.rowpixels()] = color;
		}
}


//-------------------------------------------------
//  draw_segment_decimal - draw a decimal point
//-------------------------------------------------

void layout_element::component::draw_segment_decimal(bitmap_argb32 &dest, int midx, int midy, int width, rgb_t color)
{
	// compute parameters
	width /= 2;
	float ooradius2 = 1.0f / (float)(width * width);

	// iterate over y
	for (u32 y = 0; y <= width; y++)
	{
		u32 *const d0 = &dest.pix(midy - y);
		u32 *const d1 = &dest.pix(midy + y);
		float xval = width * sqrt(1.0f - (float)(y * y) * ooradius2);
		s32 left, right;

		// compute left/right coordinates
		left = midx - s32(xval + 0.5f);
		right = midx + s32(xval + 0.5f);

		// draw this scanline
		for (u32 x = left; x < right; x++)
			d0[x] = d1[x] = color;
	}
}


//-------------------------------------------------
//  draw_segment_comma - draw a comma tail
//-------------------------------------------------

void layout_element::component::draw_segment_comma(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	// compute parameters
	width *= 1.5;
	float ratio = (maxy - miny - width) / (float)(maxx - minx);

	// draw line
	for (int x = minx; x < maxx; x++)
	{
		u32 *const d = &dest.pix(0, x);
		int step = (x - minx) * ratio;

		for (int y = maxy; y < maxy  - width - step; y--)
			d[y * dest.rowpixels()] = color;
	}
}


//-------------------------------------------------
//  apply_skew - apply skew to a bitmap
//-------------------------------------------------

void layout_element::component::apply_skew(bitmap_argb32 &dest, int skewwidth)
{
	for (int y = 0; y < dest.height(); y++)
	{
		u32 *const destrow = &dest.pix(y);
		int offs = skewwidth * (dest.height() - y) / dest.height();
		for (int x = dest.width() - skewwidth - 1; x >= 0; x--)
			destrow[x + offs] = destrow[x];
		for (int x = 0; x < offs; x++)
			destrow[x] = 0;
	}
}



//**************************************************************************
//  LAYOUT VIEW
//**************************************************************************

struct layout_view::layer_lists { item_list backdrops, screens, overlays, bezels, cpanels, marquees; };


//-------------------------------------------------
//  layout_view - constructor
//-------------------------------------------------

layout_view::layout_view(
		layout_environment &env,
		util::xml::data_node const &viewnode,
		element_map &elemmap,
		group_map &groupmap)
	: m_effaspect(1.0f)
	, m_name(make_name(env, viewnode))
	, m_unqualified_name(env.get_attribute_string(viewnode, "name", ""))
	, m_defvismask(0U)
	, m_has_art(false)
{
	// parse the layout
	m_expbounds.x0 = m_expbounds.y0 = m_expbounds.x1 = m_expbounds.y1 = 0;
	view_environment local(env, m_name.c_str());
	layer_lists layers;
	local.set_parameter("viewname", std::string(m_name));
	add_items(layers, local, viewnode, elemmap, groupmap, ROT0, identity_transform, render_color{ 1.0F, 1.0F, 1.0F, 1.0F }, true, false, true);

	// can't support legacy layers and modern visibility toggles at the same time
	if (!m_vistoggles.empty() && (!layers.backdrops.empty() || !layers.overlays.empty() || !layers.bezels.empty() || !layers.cpanels.empty() || !layers.marquees.empty()))
		throw layout_syntax_error("view contains visibility toggles as well as legacy backdrop, overlay, bezel, cpanel and/or marquee elements");

	// create visibility toggles for legacy layers
	u32 mask(1U);
	if (!layers.backdrops.empty())
	{
		m_vistoggles.emplace_back("Backdrops", mask);
		for (item &backdrop : layers.backdrops)
			backdrop.m_visibility_mask = mask;
		m_defvismask |= mask;
		mask <<= 1;
	}
	if (!layers.overlays.empty())
	{
		m_vistoggles.emplace_back("Overlays", mask);
		for (item &overlay : layers.overlays)
			overlay.m_visibility_mask = mask;
		m_defvismask |= mask;
		mask <<= 1;
	}
	if (!layers.bezels.empty())
	{
		m_vistoggles.emplace_back("Bezels", mask);
		for (item &bezel : layers.bezels)
			bezel.m_visibility_mask = mask;
		m_defvismask |= mask;
		mask <<= 1;
	}
	if (!layers.cpanels.empty())
	{
		m_vistoggles.emplace_back("Control Panels", mask);
		for (item &cpanel : layers.cpanels)
			cpanel.m_visibility_mask = mask;
		m_defvismask |= mask;
		mask <<= 1;
	}
	if (!layers.marquees.empty())
	{
		m_vistoggles.emplace_back("Backdrops", mask);
		for (item &marquee : layers.marquees)
			marquee.m_visibility_mask = mask;
		m_defvismask |= mask;
		mask <<= 1;
	}

	// deal with legacy element groupings
	if (!layers.overlays.empty() || (layers.backdrops.size() <= 1))
	{
		// screens (-1) + overlays (RGB multiply) + backdrop (add) + bezels (alpha) + cpanels (alpha) + marquees (alpha)
		for (item &backdrop : layers.backdrops)
			backdrop.m_blend_mode = BLENDMODE_ADD;
		m_items.splice(m_items.end(), layers.screens);
		m_items.splice(m_items.end(), layers.overlays);
		m_items.splice(m_items.end(), layers.backdrops);
		m_items.splice(m_items.end(), layers.bezels);
		m_items.splice(m_items.end(), layers.cpanels);
		m_items.splice(m_items.end(), layers.marquees);
	}
	else
	{
		// multiple backdrop pieces and no overlays (Golly! Ghost! mode):
		// backdrop (alpha) + screens (add) + bezels (alpha) + cpanels (alpha) + marquees (alpha)
		for (item &screen : layers.screens)
		{
			if (screen.blend_mode() == -1)
				screen.m_blend_mode = BLENDMODE_ADD;
		}
		m_items.splice(m_items.end(), layers.backdrops);
		m_items.splice(m_items.end(), layers.screens);
		m_items.splice(m_items.end(), layers.bezels);
		m_items.splice(m_items.end(), layers.cpanels);
		m_items.splice(m_items.end(), layers.marquees);
	}

	// index items with keys supplied
	for (item &curitem : m_items)
	{
		if (!curitem.id().empty())
		{
			if (!m_items_by_id.emplace(curitem.id(), curitem).second)
				throw layout_syntax_error("view contains item with duplicate id attribute");
		}
	}

	// calculate metrics
	recompute(default_visibility_mask(), false);
	for (group_map::value_type &group : groupmap)
		group.second.set_bounds_unresolved();
}


//-------------------------------------------------
//  layout_view - destructor
//-------------------------------------------------

layout_view::~layout_view()
{
}


//-------------------------------------------------
//  get_item - get item by ID
//-------------------------------------------------

layout_view::item *layout_view::get_item(std::string const &id)
{
	auto const found(m_items_by_id.find(id));
	return (m_items_by_id.end() != found) ? &found->second : nullptr;
}


//-------------------------------------------------
//  has_screen - return true if this view contains
//  the specified screen
//-------------------------------------------------

bool layout_view::has_screen(screen_device &screen)
{
	return std::find_if(m_items.begin(), m_items.end(), [&screen] (auto &itm) { return itm.screen() == &screen; }) != m_items.end();
}


//-------------------------------------------------
//  has_visible_screen - return true if this view
//  has the given screen visble
//-------------------------------------------------

bool layout_view::has_visible_screen(screen_device &screen) const
{
	return std::find_if(m_screens.begin(), m_screens.end(), [&screen] (auto const &scr) { return &scr.get() == &screen; }) != m_screens.end();
}


//-------------------------------------------------
//  recompute - recompute the bounds and aspect
//  ratio of a view and all of its contained items
//-------------------------------------------------

void layout_view::recompute(u32 visibility_mask, bool zoom_to_screen)
{
	// reset the bounds and collected active items
	render_bounds scrbounds{ 0.0f, 0.0f, 0.0f, 0.0f };
	m_bounds = scrbounds;
	m_visible_items.clear();
	m_screen_items.clear();
	m_interactive_items.clear();
	m_interactive_edges_x.clear();
	m_interactive_edges_y.clear();
	m_screens.clear();

	// loop over items and filter by visibility mask
	bool first = true;
	bool scrfirst = true;
	for (item &curitem : m_items)
	{
		if ((visibility_mask & curitem.visibility_mask()) == curitem.visibility_mask())
		{
			render_bounds const rawbounds = accumulate_bounds(curitem.m_rawbounds);

			// accumulate bounds
			m_visible_items.emplace_back(curitem);
			if (first)
				m_bounds = rawbounds;
			else
				m_bounds |= rawbounds;
			first = false;

			// accumulate visible screens and their bounds bounds
			if (curitem.screen())
			{
				if (scrfirst)
					scrbounds = rawbounds;
				else
					scrbounds |= rawbounds;
				scrfirst = false;

				// accumulate active screens
				m_screen_items.emplace_back(curitem);
				m_screens.emplace_back(*curitem.screen());
			}

			// accumulate interactive elements
			if (!curitem.clickthrough() || curitem.has_input())
				m_interactive_items.emplace_back(curitem);
		}
	}

	// if we have an explicit bounds, override it
	if (m_expbounds.x1 > m_expbounds.x0)
		m_bounds = m_expbounds;

	render_bounds target_bounds;
	if (!zoom_to_screen || scrfirst)
	{
		// if we're handling things normally, the target bounds are (0,0)-(1,1)
		m_effaspect = ((m_bounds.x1 > m_bounds.x0) && (m_bounds.y1 > m_bounds.y0)) ? m_bounds.aspect() : 1.0f;
		target_bounds.x0 = target_bounds.y0 = 0.0f;
		target_bounds.x1 = target_bounds.y1 = 1.0f;
	}
	else
	{
		// if we're cropping, we want the screen area to fill (0,0)-(1,1)
		m_effaspect = ((scrbounds.x1 > scrbounds.x0) && (scrbounds.y1 > scrbounds.y0)) ? scrbounds.aspect() : 1.0f;
		target_bounds.x0 = (m_bounds.x0 - scrbounds.x0) / scrbounds.width();
		target_bounds.y0 = (m_bounds.y0 - scrbounds.y0) / scrbounds.height();
		target_bounds.x1 = target_bounds.x0 + (m_bounds.width() / scrbounds.width());
		target_bounds.y1 = target_bounds.y0 + (m_bounds.height() / scrbounds.height());
	}

	// determine the scale/offset for normalization
	float const xoffs = m_bounds.x0;
	float const yoffs = m_bounds.y0;
	float const xscale = target_bounds.width() / m_bounds.width();
	float const yscale = target_bounds.height() / m_bounds.height();

	// normalize all the item bounds
	for (item &curitem : items())
	{
		assert(curitem.m_rawbounds.size() == curitem.m_bounds.size());
		std::copy(curitem.m_rawbounds.begin(), curitem.m_rawbounds.end(), curitem.m_bounds.begin());
		normalize_bounds(curitem.m_bounds, target_bounds.x0, target_bounds.y0, xoffs, yoffs, xscale, yscale);
	}

	// sort edges of interactive items
	LOGMASKED(LOG_INTERACTIVE_ITEMS, "Recalculated view '%s' with %u interactive items\n",
			name(), m_interactive_items.size());
	m_interactive_edges_x.reserve(m_interactive_items.size() * 2);
	m_interactive_edges_y.reserve(m_interactive_items.size() * 2);
	for (unsigned i = 0; m_interactive_items.size() > i; ++i)
	{
		item &curitem(m_interactive_items[i]);
		render_bounds const curbounds(accumulate_bounds(curitem.m_bounds));
		LOGMASKED(LOG_INTERACTIVE_ITEMS, "%u: (%s %s %s %s) hasinput=%s clickthrough=%s\n",
				i, curbounds.x0, curbounds.y0, curbounds.x1, curbounds.y1, curitem.has_input(), curitem.clickthrough());
		m_interactive_edges_x.emplace_back(i, curbounds.x0, false);
		m_interactive_edges_x.emplace_back(i, curbounds.x1, true);
		m_interactive_edges_y.emplace_back(i, curbounds.y0, false);
		m_interactive_edges_y.emplace_back(i, curbounds.y1, true);
	}
	std::sort(m_interactive_edges_x.begin(), m_interactive_edges_x.end());
	std::sort(m_interactive_edges_y.begin(), m_interactive_edges_y.end());

	if (VERBOSE & LOG_INTERACTIVE_ITEMS)
	{
		for (edge const &e : m_interactive_edges_x)
			LOGMASKED(LOG_INTERACTIVE_ITEMS, "x=%s %c%u\n", e.position(), e.trailing() ? ']' : '[', e.index());
		for (edge const &e : m_interactive_edges_y)
			LOGMASKED(LOG_INTERACTIVE_ITEMS, "y=%s %c%u\n", e.position(), e.trailing() ? ']' : '[', e.index());
	}

	// additional actions typically supplied by script
	if (!m_recomputed.isnull())
		m_recomputed();
}


//-------------------------------------------------
//  set_prepare_items_callback - set handler called
//  before adding items to render target
//-------------------------------------------------

void layout_view::set_prepare_items_callback(prepare_items_delegate &&handler)
{
	m_prepare_items = std::move(handler);
}


//-------------------------------------------------
//  set_preload_callback - set handler called
//  after preloading elements
//-------------------------------------------------

void layout_view::set_preload_callback(preload_delegate &&handler)
{
	m_preload = std::move(handler);
}


//-------------------------------------------------
//  set_recomputed_callback - set handler called
//  after recomputing item bounds
//-------------------------------------------------

void layout_view::set_recomputed_callback(recomputed_delegate &&handler)
{
	m_recomputed = std::move(handler);
}


//-------------------------------------------------
//  preload - perform expensive loading upfront
//  for visible elements
//-------------------------------------------------

void layout_view::preload()
{
	for (item &curitem : m_visible_items)
	{
		if (curitem.element())
			curitem.element()->preload();
	}

	if (!m_preload.isnull())
		m_preload();
}


//-------------------------------------------------
//  resolve_tags - resolve tags
//-------------------------------------------------

void layout_view::resolve_tags()
{
	for (item &curitem : items())
		curitem.resolve_tags();
}


//-------------------------------------------------
//  add_items - add items, recursing for groups
//-------------------------------------------------

void layout_view::add_items(
		layer_lists &layers,
		view_environment &env,
		util::xml::data_node const &parentnode,
		element_map &elemmap,
		group_map &groupmap,
		int orientation,
		layout_group::transform const &trans,
		render_color const &color,
		bool root,
		bool repeat,
		bool init)
{
	bool envaltered(false);
	bool unresolved(true);
	for (util::xml::data_node const *itemnode = parentnode.get_first_child(); itemnode; itemnode = itemnode->get_next_sibling())
	{
		if (!strcmp(itemnode->get_name(), "bounds"))
		{
			// set explicit bounds
			if (root)
				env.parse_bounds(itemnode, m_expbounds);
		}
		else if (!strcmp(itemnode->get_name(), "param"))
		{
			envaltered = true;
			if (!unresolved)
			{
				unresolved = true;
				for (group_map::value_type &group : groupmap)
					group.second.set_bounds_unresolved();
			}
			if (!repeat)
				env.set_parameter(*itemnode);
			else
				env.set_repeat_parameter(*itemnode, init);
		}
		else if (!strcmp(itemnode->get_name(), "screen"))
		{
			layers.screens.emplace_back(env, *itemnode, elemmap, orientation, trans, color);
		}
		else if (!strcmp(itemnode->get_name(), "element"))
		{
			layers.screens.emplace_back(env, *itemnode, elemmap, orientation, trans, color);
			m_has_art = true;
		}
		else if (!strcmp(itemnode->get_name(), "backdrop"))
		{
			if (layers.backdrops.empty())
				osd_printf_warning("Warning: layout view '%s' contains deprecated backdrop element\n", name());
			layers.backdrops.emplace_back(env, *itemnode, elemmap, orientation, trans, color);
			m_has_art = true;
		}
		else if (!strcmp(itemnode->get_name(), "overlay"))
		{
			if (layers.overlays.empty())
				osd_printf_warning("Warning: layout view '%s' contains deprecated overlay element\n", name());
			layers.overlays.emplace_back(env, *itemnode, elemmap, orientation, trans, color);
			m_has_art = true;
		}
		else if (!strcmp(itemnode->get_name(), "bezel"))
		{
			if (layers.bezels.empty())
				osd_printf_warning("Warning: layout view '%s' contains deprecated bezel element\n", name());
			layers.bezels.emplace_back(env, *itemnode, elemmap, orientation, trans, color);
			m_has_art = true;
		}
		else if (!strcmp(itemnode->get_name(), "cpanel"))
		{
			if (layers.cpanels.empty())
				osd_printf_warning("Warning: layout view '%s' contains deprecated cpanel element\n", name());
			layers.cpanels.emplace_back(env, *itemnode, elemmap, orientation, trans, color);
			m_has_art = true;
		}
		else if (!strcmp(itemnode->get_name(), "marquee"))
		{
			if (layers.marquees.empty())
				osd_printf_warning("Warning: layout view '%s' contains deprecated marquee element\n", name());
			layers.marquees.emplace_back(env, *itemnode, elemmap, orientation, trans, color);
			m_has_art = true;
		}
		else if (!strcmp(itemnode->get_name(), "group"))
		{
			char const *ref(env.get_attribute_string(*itemnode, "ref", nullptr));
			if (!ref)
				throw layout_syntax_error("group instantiation must have ref attribute");

			group_map::iterator const found(groupmap.find(ref));
			if (groupmap.end() == found)
				throw layout_syntax_error(util::string_format("unable to find group %s", ref));
			unresolved = false;
			found->second.resolve_bounds(env, groupmap);

			layout_group::transform grouptrans(trans);
			util::xml::data_node const *const itemboundsnode(itemnode->get_child("bounds"));
			util::xml::data_node const *const itemorientnode(itemnode->get_child("orientation"));
			int const grouporient(env.parse_orientation(itemorientnode));
			if (itemboundsnode)
			{
				render_bounds itembounds;
				env.parse_bounds(itemboundsnode, itembounds);
				grouptrans = found->second.make_transform(grouporient, itembounds, trans);
			}
			else if (itemorientnode)
			{
				grouptrans = found->second.make_transform(grouporient, trans);
			}

			view_environment local(env, false);
			add_items(
					layers,
					local,
					found->second.get_groupnode(),
					elemmap,
					groupmap,
					orientation_add(grouporient, orientation),
					grouptrans,
					env.parse_color(itemnode->get_child("color")) * color,
					false,
					false,
					true);
		}
		else if (!strcmp(itemnode->get_name(), "repeat"))
		{
			int const count(env.get_attribute_int(*itemnode, "count", -1));
			if (0 >= count)
				throw layout_syntax_error("repeat must have positive integer count attribute");
			view_environment local(env, false);
			for (int i = 0; count > i; ++i)
			{
				add_items(layers, local, *itemnode, elemmap, groupmap, orientation, trans, color, false, true, !i);
				local.increment_parameters();
			}
		}
		else if (!strcmp(itemnode->get_name(), "collection"))
		{
			char const *name(env.get_attribute_string(*itemnode, "name", nullptr));
			if (!name)
				throw layout_syntax_error("collection must have name attribute");

			auto const found(std::find_if(m_vistoggles.begin(), m_vistoggles.end(), [name] (auto const &x) { return x.name() == name; }));
			if (m_vistoggles.end() != found)
				throw layout_syntax_error(util::string_format("duplicate collection name '%s'", name));

			m_defvismask |= u32(env.get_attribute_bool(*itemnode, "visible", true) ? 1 : 0) << m_vistoggles.size(); // TODO: make this less hacky
			view_environment local(env, true);
			m_vistoggles.emplace_back(name, local.visibility_mask());
			add_items(layers, local, *itemnode, elemmap, groupmap, orientation, trans, color, false, false, true);
		}
		else
		{
			throw layout_syntax_error(util::string_format("unknown view item %s", itemnode->get_name()));
		}
	}

	if (envaltered && !unresolved)
	{
		for (group_map::value_type &group : groupmap)
			group.second.set_bounds_unresolved();
	}
}

std::string layout_view::make_name(layout_environment &env, util::xml::data_node const &viewnode)
{
	char const *const name(env.get_attribute_string(viewnode, "name", nullptr));
	if (!name || !*name)
		throw layout_syntax_error("view must have non-empty name attribute");

	if (env.is_root_device())
	{
		return name;
	}
	else
	{
		char const *tag(env.device().tag());
		if (':' == *tag)
			++tag;
		return util::string_format("%s %s", tag, name);
	}
}



//**************************************************************************
//  LAYOUT VIEW ITEM
//**************************************************************************

//-------------------------------------------------
//  item - constructor
//-------------------------------------------------

layout_view::item::item(
		view_environment &env,
		util::xml::data_node const &itemnode,
		element_map &elemmap,
		int orientation,
		layout_group::transform const &trans,
		render_color const &color)
	: m_element(find_element(env, itemnode, elemmap))
	, m_output(env.device(), env.get_attribute_string(itemnode, "name", ""))
	, m_animoutput(env.device(), make_animoutput_tag(env, itemnode))
	, m_animinput_port(nullptr)
	, m_elem_state(m_element ? m_element->default_state() : 0)
	, m_animmask(make_animmask(env, itemnode))
	, m_animshift(get_state_shift(m_animmask))
	, m_input_port(nullptr)
	, m_input_field(nullptr)
	, m_input_mask(env.get_attribute_int(itemnode, "inputmask", 0))
	, m_input_shift(get_state_shift(m_input_mask))
	, m_clickthrough(env.get_attribute_bool(itemnode, "clickthrough", "yes"))
	, m_screen(nullptr)
	, m_orientation(orientation_add(env.parse_orientation(itemnode.get_child("orientation")), orientation))
	, m_color(make_color(env, itemnode, color))
	, m_blend_mode(get_blend_mode(env, itemnode))
	, m_visibility_mask(env.visibility_mask())
	, m_id(env.get_attribute_string(itemnode, "id", ""))
	, m_input_tag(make_input_tag(env, itemnode))
	, m_animinput_tag(make_animinput_tag(env, itemnode))
	, m_rawbounds(make_bounds(env, itemnode, trans))
	, m_have_output(env.get_attribute_string(itemnode, "name", "")[0])
	, m_input_raw(env.get_attribute_bool(itemnode, "inputraw", 0))
	, m_have_animoutput(!make_animoutput_tag(env, itemnode).empty())
	, m_has_clickthrough(env.get_attribute_string(itemnode, "clickthrough", "")[0])
{
	// fetch common data
	int index = env.get_attribute_int(itemnode, "index", -1);
	if (index != -1)
		m_screen = screen_device_enumerator(env.machine().root_device()).byindex(index);

	// sanity checks
	if (strcmp(itemnode.get_name(), "screen") == 0)
	{
		if (itemnode.has_attribute("tag"))
		{
			char const *const tag(env.get_attribute_string(itemnode, "tag", ""));
			m_screen = dynamic_cast<screen_device *>(env.device().subdevice(tag));
			if (!m_screen)
				throw layout_reference_error(util::string_format("invalid screen tag '%d'", tag));
		}
		else if (!m_screen)
		{
			throw layout_reference_error(util::string_format("invalid screen index %d", index));
		}
	}
	else if (!m_element)
	{
		throw layout_syntax_error(util::string_format("item of type %s require an element tag", itemnode.get_name()));
	}

	// this can be called before resolving tags, make it return something valid
	m_bounds = m_rawbounds;
	m_get_bounds = bounds_delegate(&emu::render::detail::bounds_step::get, &m_bounds.front());
}


//-------------------------------------------------
//  item - destructor
//-------------------------------------------------

layout_view::item::~item()
{
}


//-------------------------------------------------
//  set_element_state_callback - set callback to
//  obtain element state value
//-------------------------------------------------

void layout_view::item::set_element_state_callback(state_delegate &&handler)
{
	if (!handler.isnull())
		m_get_elem_state = std::move(handler);
	else
		m_get_elem_state = default_get_elem_state();
}


//-------------------------------------------------
//  set_animation_state_callback - set callback to
//  obtain animation state
//-------------------------------------------------

void layout_view::item::set_animation_state_callback(state_delegate &&handler)
{
	if (!handler.isnull())
		m_get_anim_state = std::move(handler);
	else
		m_get_anim_state = default_get_anim_state();
}


//-------------------------------------------------
//  set_bounds_callback - set callback to obtain
//  bounds
//-------------------------------------------------

void layout_view::item::set_bounds_callback(bounds_delegate &&handler)
{
	if (!handler.isnull())
		m_get_bounds = std::move(handler);
	else
		m_get_bounds = default_get_bounds();
}


//-------------------------------------------------
//  set_color_callback - set callback to obtain
//  color
//-------------------------------------------------

void layout_view::item::set_color_callback(color_delegate &&handler)
{
	if (!handler.isnull())
		m_get_color = std::move(handler);
	else
		m_get_color = default_get_color();
}


//-------------------------------------------------
//  resolve_tags - resolve tags, if any are set
//-------------------------------------------------

void layout_view::item::resolve_tags()
{
	// resolve element state output and set default value
	if (m_have_output)
	{
		m_output.resolve();
		if (m_element)
			m_output = m_element->default_state();
	}

	// resolve animation state output
	if (m_have_animoutput)
		m_animoutput.resolve();

	// resolve animation state input
	if (!m_animinput_tag.empty())
		m_animinput_port = m_element->machine().root_device().ioport(m_animinput_tag);

	// resolve element state input
	if (!m_input_tag.empty())
	{
		m_input_port = m_element->machine().root_device().ioport(m_input_tag);
		if (m_input_port)
		{
			// if there's a matching unconditional field, cache it
			for (ioport_field &field : m_input_port->fields())
			{
				if (field.mask() & m_input_mask)
				{
					if (field.condition().condition() == ioport_condition::ALWAYS)
						m_input_field = &field;
					break;
				}
			}

			// if clickthrough isn't explicitly configured, having an I/O port implies false
			if (!m_has_clickthrough)
				m_clickthrough = false;
		}
	}

	// choose optimal handlers
	m_get_elem_state = default_get_elem_state();
	m_get_anim_state = default_get_anim_state();
	m_get_bounds = default_get_bounds();
	m_get_color = default_get_color();
}


//-------------------------------------------------
//  default_get_elem_state - get default element
//  state handler
//-------------------------------------------------

layout_view::item::state_delegate layout_view::item::default_get_elem_state()
{
	if (m_have_output)
		return state_delegate(&item::get_output, this);
	else if (!m_input_port)
		return state_delegate(&item::get_state, this);
	else if (m_input_raw)
		return state_delegate(&item::get_input_raw, this);
	else if (m_input_field)
		return state_delegate(&item::get_input_field_cached, this);
	else
		return state_delegate(&item::get_input_field_conditional, this);
}


//-------------------------------------------------
//  default_get_anim_state - get default animation
//  state handler
//-------------------------------------------------

layout_view::item::state_delegate layout_view::item::default_get_anim_state()
{
	if (m_have_animoutput)
		return state_delegate(&item::get_anim_output, this);
	else if (m_animinput_port)
		return state_delegate(&item::get_anim_input, this);
	else
		return default_get_elem_state();
}


//-------------------------------------------------
//  default_get_bounds - get default bounds handler
//-------------------------------------------------

layout_view::item::bounds_delegate layout_view::item::default_get_bounds()
{
	return (m_bounds.size() == 1U)
			? bounds_delegate(&emu::render::detail::bounds_step::get, &m_bounds.front())
			: bounds_delegate(&item::get_interpolated_bounds, this);
}


//-------------------------------------------------
//  default_get_color - get default color handler
//-------------------------------------------------

layout_view::item::color_delegate layout_view::item::default_get_color()
{
	return (m_color.size() == 1U)
			? color_delegate(&emu::render::detail::color_step::get, &const_cast<emu::render::detail::color_step &>(m_color.front()))
			: color_delegate(&item::get_interpolated_color, this);
}


//-------------------------------------------------
//  get_state - get state when no bindings
//-------------------------------------------------

int layout_view::item::get_state() const
{
	return m_elem_state;
}


//-------------------------------------------------
//  get_output - get element state output
//-------------------------------------------------

int layout_view::item::get_output() const
{
	assert(m_have_output);
	return int(s32(m_output));
}


//-------------------------------------------------
//  get_input_raw - get element state input
//-------------------------------------------------

int layout_view::item::get_input_raw() const
{
	assert(m_input_port);
	return int(std::make_signed_t<ioport_value>((m_input_port->read() & m_input_mask) >> m_input_shift));
}


//-------------------------------------------------
//  get_input_field_cached - element state
//-------------------------------------------------

int layout_view::item::get_input_field_cached() const
{
	assert(m_input_port);
	assert(m_input_field);
	return ((m_input_port->read() ^ m_input_field->defvalue()) & m_input_mask) ? 1 : 0;
}


//-------------------------------------------------
//  get_input_field_conditional - element state
//-------------------------------------------------

int layout_view::item::get_input_field_conditional() const
{
	assert(m_input_port);
	assert(!m_input_field);
	ioport_field const *const field(m_input_port->field(m_input_mask));
	return (field && ((m_input_port->read() ^ field->defvalue()) & m_input_mask)) ? 1 : 0;
}


//-------------------------------------------------
//  get_anim_output - get animation output
//-------------------------------------------------

int layout_view::item::get_anim_output() const
{
	assert(m_have_animoutput);
	return int(unsigned((u32(s32(m_animoutput) & m_animmask) >> m_animshift)));
}


//-------------------------------------------------
//  get_anim_input - get animation input
//-------------------------------------------------

int layout_view::item::get_anim_input() const
{
	assert(m_animinput_port);
	return int(std::make_signed_t<ioport_value>((m_animinput_port->read() & m_animmask) >> m_animshift));
}


//-------------------------------------------------
//  get_interpolated_bounds - animated bounds
//-------------------------------------------------

void layout_view::item::get_interpolated_bounds(render_bounds &result) const
{
	assert(m_bounds.size() > 1U);
	result = interpolate_bounds(m_bounds, m_get_anim_state());
}


//-------------------------------------------------
//  get_interpolated_color - animated color
//-------------------------------------------------

void layout_view::item::get_interpolated_color(render_color &result) const
{
	assert(m_color.size() > 1U);
	result = interpolate_color(m_color, m_get_anim_state());
}


//-------------------------------------------------
//  find_element - find element definition
//-------------------------------------------------

layout_element *layout_view::item::find_element(view_environment &env, util::xml::data_node const &itemnode, element_map &elemmap)
{
	char const *const name(env.get_attribute_string(itemnode, !strcmp(itemnode.get_name(), "element") ? "ref" : "element", nullptr));
	if (!name)
		return nullptr;

	// search the list of elements for a match, error if not found
	element_map::iterator const found(elemmap.find(name));
	if (elemmap.end() != found)
		return &found->second;
	else
		throw layout_syntax_error(util::string_format("unable to find element %s", name));
}


//-------------------------------------------------
//  make_bounds - get transformed bounds
//-------------------------------------------------

layout_view::item::bounds_vector layout_view::item::make_bounds(
		view_environment &env,
		util::xml::data_node const &itemnode,
		layout_group::transform const &trans)
{
	bounds_vector result;
	for (util::xml::data_node const *bounds = itemnode.get_child("bounds"); bounds; bounds = bounds->get_next_sibling("bounds"))
	{
		if (!add_bounds_step(env, result, *bounds))
		{
			throw layout_syntax_error(
					util::string_format(
						"%s item has duplicate bounds for state",
						itemnode.get_name()));
		}
	}
	for (emu::render::detail::bounds_step &step : result)
	{
		render_bounds_transform(step.bounds, trans);
		if (step.bounds.x0 > step.bounds.x1)
			std::swap(step.bounds.x0, step.bounds.x1);
		if (step.bounds.y0 > step.bounds.y1)
			std::swap(step.bounds.y0, step.bounds.y1);
	}
	set_bounds_deltas(result);
	return result;
}


//-------------------------------------------------
//  make_color - get color inflection points
//-------------------------------------------------

layout_view::item::color_vector layout_view::item::make_color(
		view_environment &env,
		util::xml::data_node const &itemnode,
		render_color const &mult)
{
	color_vector result;
	for (util::xml::data_node const *color = itemnode.get_child("color"); color; color = color->get_next_sibling("color"))
	{
		if (!add_color_step(env, result, *color))
		{
			throw layout_syntax_error(
					util::string_format(
						"%s item has duplicate color for state",
						itemnode.get_name()));
		}
	}
	if (result.empty())
	{
		result.emplace_back(emu::render::detail::color_step{ 0, mult, { 0.0F, 0.0F, 0.0F, 0.0F } });
	}
	else
	{
		for (emu::render::detail::color_step &step : result)
			step.color *= mult;
		set_color_deltas(result);
	}
	return result;
}


//-------------------------------------------------
//  make_animoutput_tag - get animation output tag
//-------------------------------------------------

std::string layout_view::item::make_animoutput_tag(view_environment &env, util::xml::data_node const &itemnode)
{
	util::xml::data_node const *const animate(itemnode.get_child("animate"));
	if (animate)
		return env.get_attribute_string(*animate, "name", "");
	else
		return std::string();
}


//-------------------------------------------------
//  make_animmask - get animation state mask
//-------------------------------------------------

ioport_value layout_view::item::make_animmask(view_environment &env, util::xml::data_node const &itemnode)
{
	util::xml::data_node const *const animate(itemnode.get_child("animate"));
	return animate ? env.get_attribute_int(*animate, "mask", ~ioport_value(0)) : ~ioport_value(0);
}


//-------------------------------------------------
//  make_animinput_tag - get absolute tag for
//  animation input
//-------------------------------------------------

std::string layout_view::item::make_animinput_tag(view_environment &env, util::xml::data_node const &itemnode)
{
	util::xml::data_node const *const animate(itemnode.get_child("animate"));
	char const *tag(animate ? env.get_attribute_string(*animate, "inputtag", nullptr) : nullptr);
	return tag ? env.device().subtag(tag) : std::string();
}


//-------------------------------------------------
//  make_input_tag - get absolute input tag
//-------------------------------------------------

std::string layout_view::item::make_input_tag(view_environment &env, util::xml::data_node const &itemnode)
{
	char const *tag(env.get_attribute_string(itemnode, "inputtag", nullptr));
	return tag ? env.device().subtag(tag) : std::string();
}


//-------------------------------------------------
//  get_blend_mode - explicit or implicit blend
//-------------------------------------------------

int layout_view::item::get_blend_mode(view_environment &env, util::xml::data_node const &itemnode)
{
	// see if there's a blend mode attribute
	char const *const mode(env.get_attribute_string(itemnode, "blend", nullptr));
	if (mode)
	{
		if (!strcmp(mode, "none"))
			return BLENDMODE_NONE;
		else if (!strcmp(mode, "alpha"))
			return BLENDMODE_ALPHA;
		else if (!strcmp(mode, "multiply"))
			return BLENDMODE_RGB_MULTIPLY;
		else if (!strcmp(mode, "add"))
			return BLENDMODE_ADD;
		else
			throw layout_syntax_error(util::string_format("unknown blend mode %s", mode));
	}

	// fall back to implicit blend mode based on element type
	if (!strcmp(itemnode.get_name(), "screen"))
		return -1; // magic number recognised by render.cpp to allow per-element blend mode
	else if (!strcmp(itemnode.get_name(), "overlay"))
		return BLENDMODE_RGB_MULTIPLY;
	else
		return BLENDMODE_ALPHA;
}


//-------------------------------------------------
//  get_state_shift - shift to right-align LSB
//-------------------------------------------------

unsigned layout_view::item::get_state_shift(ioport_value mask)
{
	unsigned result(0U);
	while (mask && !BIT(mask, 0))
	{
		++result;
		mask >>= 1;
	}
	return result;
}



//**************************************************************************
//  LAYOUT VIEW VISIBILITY TOGGLE
//**************************************************************************

//-------------------------------------------------
//  visibility_toggle - constructor
//-------------------------------------------------

layout_view::visibility_toggle::visibility_toggle(std::string &&name, u32 mask)
	: m_name(std::move(name))
	, m_mask(mask)
{
	assert(mask);
}



//**************************************************************************
//  LAYOUT FILE
//**************************************************************************

//-------------------------------------------------
//  layout_file - constructor
//-------------------------------------------------

layout_file::layout_file(
		device_t &device,
		util::xml::data_node const &rootnode,
		char const *searchpath,
		char const *dirname)
	: m_device(device)
	, m_elemmap()
	, m_viewlist()
{
	try
	{
		environment env(device, searchpath, dirname);

		// find the layout node
		util::xml::data_node const *const mamelayoutnode = rootnode.get_child("mamelayout");
		if (!mamelayoutnode)
			throw layout_syntax_error("missing mamelayout node");

		// validate the config data version
		int const version = mamelayoutnode->get_attribute_int("version", 0);
		if (version != LAYOUT_VERSION)
			throw layout_syntax_error(util::string_format("unsupported version %d", version));

		// parse all the parameters, elements and groups
		group_map groupmap;
		add_elements(env, *mamelayoutnode, groupmap, false, true);

		// parse all the views
		for (util::xml::data_node const *viewnode = mamelayoutnode->get_child("view"); viewnode != nullptr; viewnode = viewnode->get_next_sibling("view"))
		{
			// the trouble with allowing errors to propagate here is that it wreaks havoc with screenless systems that use a terminal by default
			// e.g. intlc44 and intlc440 have a terminal on the TTY port by default and have a view with the front panel with the terminal screen
			// however, they have a second view with just the front panel which is very useful if you're using e.g. -tty null_modem with a socket
			// if the error is allowed to propagate, the entire layout is dropped so you can't select the useful view
			try
			{
				m_viewlist.emplace_back(env, *viewnode, m_elemmap, groupmap);
			}
			catch (layout_reference_error const &err)
			{
				osd_printf_warning("Error instantiating layout view %s: %s\n", env.get_attribute_string(*viewnode, "name", ""), err.what());
			}
		}

		// load the content of the first script node
		if (!m_viewlist.empty())
		{
			util::xml::data_node const *const scriptnode = mamelayoutnode->get_child("script");
			if (scriptnode)
				emulator_info::layout_script_cb(*this, scriptnode->get_value());
		}
	}
	catch (layout_syntax_error const &err)
	{
		// syntax errors are always fatal
		throw emu_fatalerror("Error parsing XML layout: %s", err.what());
	}
}


//-------------------------------------------------
//  ~layout_file - destructor
//-------------------------------------------------

layout_file::~layout_file()
{
}


//-------------------------------------------------
//  resolve_tags - resolve tags
//-------------------------------------------------

void layout_file::resolve_tags()
{
	for (layout_view &view : views())
		view.resolve_tags();

	if (!m_resolve_tags.isnull())
		m_resolve_tags();
}


//-------------------------------------------------
//  set_resolve_tags_callback - set callback for
//  additional tasks after resolving tags
//-------------------------------------------------

void layout_file::set_resolve_tags_callback(resolve_tags_delegate &&handler)
{
	m_resolve_tags = std::move(handler);
}


void layout_file::add_elements(
		environment &env,
		util::xml::data_node const &parentnode,
		group_map &groupmap,
		bool repeat,
		bool init)
{
	for (util::xml::data_node const *childnode = parentnode.get_first_child(); childnode; childnode = childnode->get_next_sibling())
	{
		if (!strcmp(childnode->get_name(), "param"))
		{
			if (!repeat)
				env.set_parameter(*childnode);
			else
				env.set_repeat_parameter(*childnode, init);
		}
		else if (!strcmp(childnode->get_name(), "element"))
		{
			char const *const name(env.get_attribute_string(*childnode, "name", nullptr));
			if (!name)
				throw layout_syntax_error("element lacks name attribute");
			if (!m_elemmap.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(env, *childnode)).second)
				throw layout_syntax_error(util::string_format("duplicate element name %s", name));
		}
		else if (!strcmp(childnode->get_name(), "group"))
		{
			char const *const name(env.get_attribute_string(*childnode, "name", nullptr));
			if (!name)
				throw layout_syntax_error("group lacks name attribute");
			if (!groupmap.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(*childnode)).second)
				throw layout_syntax_error(util::string_format("duplicate group name %s", name));
		}
		else if (!strcmp(childnode->get_name(), "repeat"))
		{
			int const count(env.get_attribute_int(*childnode, "count", -1));
			if (0 >= count)
				throw layout_syntax_error("repeat must have positive integer count attribute");
			environment local(env);
			for (int i = 0; count > i; ++i)
			{
				add_elements(local, *childnode, groupmap, true, !i);
				local.increment_parameters();
			}
		}
		else if (repeat || (strcmp(childnode->get_name(), "view") && strcmp(childnode->get_name(), "script")))
		{
			throw layout_syntax_error(util::string_format("unknown layout item %s", childnode->get_name()));
		}
	}
}
