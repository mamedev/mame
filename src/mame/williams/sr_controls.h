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
constexpr unsigned STEERING_MASK = 0x3f;
constexpr unsigned THROTTLE_MASK = 0x0f;
constexpr unsigned BRAKE_MASK = 0x03;

constexpr unsigned bounded(unsigned value, unsigned maximum)
{
	return value < maximum ? value : maximum;
}

// Steering and throttle use reflected binary (Gray) code.
constexpr std::uint8_t gray_position(unsigned position, unsigned maximum)
{
	unsigned const value = bounded(position, maximum);
	return std::uint8_t(value ^ (value >> 1));
}

constexpr std::uint8_t steering(unsigned position)
{
	return gray_position(position, STEERING_MASK);
}

constexpr std::uint8_t throttle(unsigned position)
{
	return gray_position(position, THROTTLE_MASK);
}

constexpr std::uint8_t brake(unsigned position)
{
	return std::uint8_t(bounded(position, BRAKE_MASK) ^ BRAKE_MASK);
}

} // namespace sr_controls
#endif
