// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  eivc.h
//
//  Inline implementations for MSVC compiler.
//
//============================================================

#ifndef MAME_OSD_EIVC_H
#define MAME_OSD_EIVC_H

#pragma once

#include <intrin.h>
#include <stdlib.h>

#pragma intrinsic(_BitScanReverse)

#if defined(_M_X64) || defined(_M_ARM64)
#pragma intrinsic(_BitScanReverse64)
#endif


#endif // MAME_OSD_EIVC_H
