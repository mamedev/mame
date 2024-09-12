// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * NABU PC - NABU Network Adapter
 *
 *******************************************************************/

#ifndef MAME_BUS_NABUPC_ADAPTER_H
#define MAME_BUS_NABUPC_ADAPTER_H

#pragma once

#include "bus/rs232/rs232.h"

DECLARE_DEVICE_TYPE(NABUPC_NETWORK_LOCAL_ADAPTER, device_rs232_port_interface)
DECLARE_DEVICE_TYPE(NABUPC_NETWORK_REMOTE_ADAPTER, device_rs232_port_interface)

#endif // MAME_BUS_NABUPC_ADAPTER_H
