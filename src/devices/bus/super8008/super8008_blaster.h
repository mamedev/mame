// license:BSD-3-Clause
// copyright-holders:Jeremy English
/*********************************************************************
    
	Blaster for Scott's 8008 Supercomputer a.k.a. Master Blaster
	by Dr. Scott M. Baker
    
	Schematics:
	https://github.com/sbelectronics/8008-super 
    
	ROM Source Code:
	https://github.com/sbelectronics/h8/tree/master/h8-8008
    
	Demo:
	https://youtu.be/wurKTPdPhrI?si=aerTbgHIFm_8YwU2
    
	Write up:
	https://www.smbaker.com/master-blaster-an-8008-supercomputer
    
	MAME driver for Jim Loos 8008-SBC
	src/mame/homebrew/sbc8008.cpp
        
	This computer is based on Jim Loos 8008-SBC:
	https://github.com/jim11662418/8008-SBC    

*********************************************************************/

#ifndef MAME_BUS_SUPER8008_SUPER8008_BLASTER_H
#define MAME_BUS_SUPER8008_SUPER8008_BLASTER_H

#pragma once

#include "super8008.h"

// device type declaration
DECLARE_DEVICE_TYPE(SUPER8008_BLASTER, device_super8008_card_interface);

#endif  // MAME_BUS_SUPER8008_SUPER8008_BLASTER_H
