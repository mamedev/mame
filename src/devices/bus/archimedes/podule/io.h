// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn BBC I/O Podule

**********************************************************************/

#ifndef MAME_BUS_ARCHIMEDES_PODULE_IO_H
#define MAME_BUS_ARCHIMEDES_PODULE_IO_H

#pragma once

#include "slot.h"

// device type definition
DECLARE_DEVICE_TYPE(ARC_BBCIO_AKA10, device_archimedes_podule_interface)
DECLARE_DEVICE_TYPE(ARC_UPMIDI_AKA12, device_archimedes_podule_interface)
DECLARE_DEVICE_TYPE(ARC_IOMIDI_AKA15, device_archimedes_podule_interface)
DECLARE_DEVICE_TYPE(ARC_MIDI_AKA16, device_archimedes_podule_interface)
DECLARE_DEVICE_TYPE(ARC_BBCIO_AGA30, device_archimedes_podule_interface)

#endif // MAME_BUS_ARCHIMEDES_PODULE_IO_H
