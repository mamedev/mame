// license:BSD-3-Clause
// copyright-holders:Mario Montminy
#include "emu.h"
#include "emuopts.h"
#include "video/alt_vector.h"

#define VERBOSE 0
#include "logmacro.h"


alt_vector_device_base::alt_vector_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) 
   : device_t(mconfig, type, tag, owner, clock)
{

}

int alt_vector_device_base::add_point(int x, int y, rgb_t color, int intensity) 
{ 
    return 0;
};

int alt_vector_device_base::update(screen_device &screen, const rectangle &cliprect) 
{ 
    return 0; 
};

