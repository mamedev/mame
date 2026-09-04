// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    output.h

    OSD output interfaces

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_OUTPUT_H
#define MAME_OSD_INTERFACE_OUTPUT_H

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>


namespace osd {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// output item interface

class output_item
{
public:
	std::string_view const &name() const noexcept { return m_name; }
	std::string_view const &device_tag() const noexcept { return m_device_tag; }
	std::string const &qualified_name() const noexcept { return m_qualified_name; }
	std::int32_t const &value() const noexcept { return m_value; }

protected:
	output_item(std::string_view n, std::string_view d) :
		m_qualified_name((":" == d) ? std::string(n) : (std::string(d.substr(1)).append(":").append(n))),
		m_name(&m_qualified_name[m_qualified_name.length() - n.length()], n.length()),
		m_device_tag(d),
		m_value(0)
	{
	}
	output_item(output_item const &) = delete;
	output_item &operator=(output_item const &) = delete;

	std::string const m_qualified_name;
	std::string_view const m_name;
	std::string_view const m_device_tag;
	std::int32_t m_value;
};

} // namespace osd

#endif // MAME_OSD_INTERFACE_OUTPUT_H
