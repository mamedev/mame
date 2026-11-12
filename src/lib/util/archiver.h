// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    archiver.h

    Archive writer interfaces

***************************************************************************/
#ifndef MAME_LIB_UTIL_ARCHIVER_H
#define MAME_LIB_UTIL_ARCHIVER_H

#pragma once

#include "ioprocs.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>


namespace util {

class archive_writer
{
public:
	using ptr = std::unique_ptr<archive_writer>;

	virtual ~archive_writer() = default;

	virtual std::pair<write_stream::ptr, std::error_condition> add_file(
			std::string_view name,
			std::chrono::system_clock::time_point modification_time,
			std::optional<std::uint64_t> size,
			std::optional<std::string_view> comment) noexcept = 0;

	virtual std::error_condition add_folder(
			std::string_view name,
			std::chrono::system_clock::time_point modification_time,
			std::optional<std::string_view> comment) noexcept = 0;

	virtual std::error_condition finalize() noexcept = 0;
};


std::pair<archive_writer::ptr, std::error_condition> pkzip_writer(
		random_write::ptr &&output,
		std::optional<std::uint64_t> physical_offset = std::nullopt,
		std::optional<std::uint64_t> logical_offset = std::nullopt) noexcept;

std::pair<archive_writer::ptr, std::error_condition> pkzip_writer(
		random_write &output,
		std::optional<std::uint64_t> physical_offset = std::nullopt,
		std::optional<std::uint64_t> logical_offset = std::nullopt) noexcept;

std::pair<archive_writer::ptr, std::error_condition> pkzip_writer(
		write_stream::ptr &&output,
		std::uint64_t offset = 0U) noexcept;

std::pair<archive_writer::ptr, std::error_condition> pkzip_writer(
		write_stream &output,
		std::uint64_t offset = 0U) noexcept;

} // namespace util

#endif // MAME_LIB_UTIL_ARCHIVER_H
