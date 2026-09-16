// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM75 MCU

*/

#ifndef MAME_CPU_PPS41_MM75_H
#define MAME_CPU_PPS41_MM75_H

#pragma once

#include "mm76.h"

// pinout reference

/*
            ____   ____
   RIO8  1 |*   \_/    | 28 RIO7
   RIO1  2 |           | 27 RIO6
   RIO2  3 |           | 26 RIO5
   RIO3  4 |           | 25 INT0
   RIO4  5 |           | 24 PO
   DIO0  6 |           | 23 PI4
   DIO1  7 |   MM75    | 22 PI3
   DIO2  8 |           | 21 PI2
   DIO3  9 |           | 20 PI1
   DIO4 10 |           | 19 TEST
   DIO5 11 |           | 18 Vdd
   DIO6 12 |           | 17 VC
   DIO7 13 |           | 16 A
    Vss 14 |___________| 15 DIO8

*/

class mm75_device : public mm76_device
{
public:
	mm75_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	// opcode handlers
	virtual void op_ios() override;
	virtual void op_i2c() override;

	virtual void op_ibm() override;
	virtual void op_int1h() override;
};


DECLARE_DEVICE_TYPE(MM75, mm75_device)

#endif // MAME_CPU_PPS41_MM75_H
