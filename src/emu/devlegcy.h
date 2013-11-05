// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devlegcy.h

    Legacy device helpers.

***************************************************************************/

#pragma once

#ifndef __DEVLEGCY_H__
#define __DEVLEGCY_H__

//**************************************************************************
//  MACROS
//**************************************************************************

#define DEVICE_START_NAME(name)     device_start_##name
#define DEVICE_START(name)          void DEVICE_START_NAME(name)(device_t *device)
#define DEVICE_START_CALL(name)     DEVICE_START_NAME(name)(device)

#define DEVICE_STOP_NAME(name)      device_stop_##name
#define DEVICE_STOP(name)           void DEVICE_STOP_NAME(name)(device_t *device)

#define DEVICE_RESET_NAME(name)     device_reset_##name
#define DEVICE_RESET(name)          void DEVICE_RESET_NAME(name)(device_t *device)
#define DEVICE_RESET_CALL(name)     DEVICE_RESET_NAME(name)(device)

#endif  /* __DEVLEGCY_H__ */
