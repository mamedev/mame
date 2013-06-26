/***************************************************************************

    network.h

    Core network interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __NETWORK_H__
#define __NETWORK_H__

void network_init(running_machine &machine);

#endif /* __NETWORK_H__ */
