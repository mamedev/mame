// license:BSD-3-Clause
// Taito E07-11: TMS320C51 with reconstructed internal ROM
// ROM contents deduced from taitojc.cpp driver and tms320c5x.cpp boot simulation
#ifndef TAITO_E07_H
#define TAITO_E07_H
#pragma once
#include "tms320c5x.h"

class taito_e07_device : public tms320c51_device
{
public:
    taito_e07_device(const machine_config &mconfig, const char *tag,
                     device_t *owner, uint32_t clock);
private:
    void taito_e07_internal_pgm(address_map &map);
    void taito_e07_internal_data(address_map &map); // Parent function (tms320c51_internal_data) is protected
    uint16_t internal_rom_r(offs_t offset);
    static const uint16_t s_rom[0x1000];
};

DECLARE_DEVICE_TYPE(TAITO_E07, taito_e07_device)
#endif