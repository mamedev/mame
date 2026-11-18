// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Axell RM1 picture decoder used by the AG-2.
#ifndef MAME_VIDEO_AXELL_AG2_RM1_H
#define MAME_VIDEO_AXELL_AG2_RM1_H

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace axell_rm1
{

class decode_error : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

struct frame
{
	static constexpr std::uint32_t WIDTH = 640;
	static constexpr std::uint32_t HEIGHT = 480;

	std::vector<std::uint8_t> y;
	std::vector<std::uint8_t> cr;
	std::vector<std::uint8_t> cb;

	frame();
	std::vector<std::uint8_t> &plane(std::uint32_t index);
	const std::vector<std::uint8_t> &plane(std::uint32_t index) const;
};

class decoder
{
public:
	decoder() = default;

	void reset();

	// kind 0=I, 1=P, 2=B. Payload begins at the picture-specific header,
	// after the container's eight-byte packet header. The returned reference
	// remains valid until reset() or the next successful decode_packet().
	// Unsupported geometry/encoding, missing references or malformed streams
	// throw decode_error. A failed decode leaves existing references intact.
	const frame &decode_packet(std::uint32_t kind, const std::uint8_t *payload, std::size_t size);

private:
	std::shared_ptr<frame> m_past;
	std::shared_ptr<frame> m_future;
	std::shared_ptr<frame> m_current;
	std::array<std::uint8_t, 70 * 38> m_nest{};
	bool m_have_nest = false;
};

} // namespace axell_rm1

#endif // MAME_VIDEO_AXELL_AG2_RM1_H
