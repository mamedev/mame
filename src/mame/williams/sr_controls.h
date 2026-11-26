// license:BSD-3-Clause
// copyright-holders:Andrew Silverman
#ifndef MAME_WILLIAMS_SR_CONTROLS_H
#define MAME_WILLIAMS_SR_CONTROLS_H
#pragma once

#include <cstdint>

// Host positions to the settled digital levels presented at the CPU PIA.
// Optical transition times and contact/occlusion faults are outside this model.
namespace sr_controls {

// Usable bit masks from the respective cabinet encoder boards.
constexpr std::uint32_t STEERING_MASK = 0x3f;
constexpr std::uint32_t THROTTLE_MASK = 0x0f;
constexpr std::uint32_t BRAKE_MASK = 0x03;

constexpr std::uint32_t bounded(std::uint32_t value, std::uint32_t maximum)
{
	return value < maximum ? value : maximum;
}

// Steering and throttle use reflected binary (Gray) code.
constexpr std::uint8_t gray_position(std::uint32_t position, std::uint32_t maximum)
{
	std::uint32_t const value = bounded(position, maximum);
	return std::uint8_t(value ^ (value >> 1));
}

constexpr std::uint8_t steering(std::uint32_t position)
{
	return gray_position(position, STEERING_MASK);
}

constexpr std::uint8_t throttle(std::uint32_t position)
{
	return gray_position(position, THROTTLE_MASK);
}

constexpr std::uint8_t brake(std::uint32_t position)
{
	return std::uint8_t(bounded(position, BRAKE_MASK) ^ BRAKE_MASK);
}

} // namespace sr_controls
#endif
