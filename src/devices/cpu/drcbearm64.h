// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_CPU_DRCBEARM64_H
#define MAME_CPU_DRCBEARM64_H

#pragma once

#include "drcuml.h"

#include <memory>


namespace drc {

std::unique_ptr<drcbe_interface> make_drcbe_arm64(
		drcuml_state &drcuml,
		device_t &device,
		drc_cache &cache,
		uint32_t flags,
		int modes,
		int addrbits,
		int ignorebits);

} // namespace drc

#endif // MAME_CPU_DRCBEARM64_H
