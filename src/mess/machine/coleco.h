// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Ben Bruscella, Sean Young
#pragma once

#ifndef __COLECO_CONTROLLERS__
#define __COLECO_CONTROLLERS__

UINT8 coleco_scan_paddles(running_machine &machine, UINT8 *joy_status0, UINT8 *joy_status1);
UINT8 coleco_paddle_read(running_machine &machine, int port, int joy_mode, UINT8 joy_status);

INPUT_PORTS_EXTERN( coleco );

#endif
