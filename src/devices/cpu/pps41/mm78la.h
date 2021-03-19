// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM77LA/MM78LA MCU

*/

#ifndef MAME_CPU_PPS41_MM78LA_H
#define MAME_CPU_PPS41_MM78LA_H

#pragma once

#include "mm78.h"

// pinout reference

/*
            ____   ____                         ____   ____
     BP  1 |*   \_/    | 42 DIO9         BP  1 |*   \_/    | 40 DIO9
      A  2 |           | 41 DIO8          A  2 |           | 39 DIO8
    Vdd  3 |           | 40 DIO7        N/C  3 |           | 38 DIO7
     VC  4 |           | 39 DIO6         VC  4 |           | 37 DIO6
   TEST  5 |           | 38 DIO5        Vdd  5 |           | 36 DIO5
    Vss  6 |           | 37 DIO4        Vss  6 |           | 35 DIO4
    PI4  7 |           | 36 DIO3       TEST  7 |           | 34 DIO3
    PI8  8 |           | 35 DIO2        PI4  8 |           | 33 DIO2
    PI3  9 |           | 34 DIO1        PI8  9 |           | 32 DIO1
    PI7 10 |  MM78LA   | 33 DIO0        PI3 10 |  MM77LA   | 31 DIO0
    PI6 11 |           | 32 Vdd SPK     PI7 11 |           | 30 INT0
    PI2 12 |           | 31 SPK R2      PI6 12 |           | 29 SPK R2
    PI5 13 |           | 30 SPK R1      PI2 13 |           | 28 Vdd SPK
    PI1 14 |           | 29 RO01        PI5 14 |           | 27 SPK R1
     PO 15 |           | 28 RO02        PI1 15 |           | 26 RO01
   RO14 16 |           | 27 RO03         PO 16 |           | 25 RO02
   RO13 17 |           | 26 RO04       RO10 17 |           | 24 RO03
   RO12 18 |           | 25 RO05       RO09 18 |           | 23 RO04
   RO11 19 |           | 24 RO06       RO08 19 |           | 22 RO05
   RO10 20 |           | 23 RO07       RO07 20 |___________| 21 RO06
   RO09 21 |___________| 22 RO08

    MM78LA = aka MM95, MM77LA = aka B80xx (no official documentation known for latter, pinout has guesses)
*/

class mm78la_device : public mm78_device
{
public:
	mm78la_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	mm78la_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// opcode handlers
	virtual void op_sos() override;
	virtual void op_ros() override;
	virtual void op_skisl() override;
	virtual void op_ix() override;
	virtual void op_ox() override;
	virtual void op_ioa() override;
	virtual void op_ios() override;
};

class mm77la_device : public mm78la_device
{
public:
	mm77la_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
};


DECLARE_DEVICE_TYPE(MM78LA, mm78la_device)
DECLARE_DEVICE_TYPE(MM77LA, mm77la_device)

#endif // MAME_CPU_PPS41_MM78LA_H
