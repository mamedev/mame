// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Toshiba TMP96C141 microcontroller

*****************************************************************************

       (AN0)PS0  73 ----------------+ +---------------- 72  P42(_CS2/_CAS2)
       (AN1)PS1  74 --------------+ | | +-------------- 71  P41(_CS1/_CAS1)
       (AN2)PS2  75 ------------+ | | | | +------------ 70  P40(_CS0/_CAS0)
       (AN3)PS3  76 ----------+ | | | | | | +---------- 69  P37(_RAS)
            VCC  77 --------+ | | | | | | | | +-------- 68  P36(R/_W)
           VREF  78 ------+ | | | | | | | | | | +------ 67  P35(_BUSAK)
           AGND  79 ----+ | | | | | | | | | | | | +---- 66  P34(_BUSRQ)
            VSS  80 --+ | | | | | | | | | | | | | | +-- 65  P33(_WAIT)
                     _|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_
      (PG00)P60   1 |   _                             | 64  P32(_HWR)
      (PG01)P61   2 |  (_)                            | 63  P31(_WR)
      (PG02)P62   3 |                                 | 62  P30(_RD)
      (PG03)P63   4 |                                 | 61  P27(A7/A23)
      (PG10)P64   5 |                                 | 60  P26(A6/A22)
      (PG11)P65   6 |                                 | 59  P25(A5/A21)
      (PG12)P66   7 |                                 | 58  P24(A4/A20)
      (PG13)P67   8 |                                 | 57  P23(A3/A19)
       (TI0)P70   9 |                                 | 56  P22(A2/A18)
       (TO1)P71  10 |                                 | 55  P21(A1/A17)
       (TO2)P72  11 |                                 | 54  P20(A0/A16)
       (TO3)P73  12 |           TMP96C141AF           | 53  VSS
  (INT4/TI4)P80  13 |             (QFP80)             | 52  P17(AD15/A15)
  (INT5/TI5)P81  14 |                                 | 51  P16(AD14/A14)
       (TO4)P82  15 |                                 | 50  P15(AD13/A13)
       (TO5)P83  16 |                                 | 49  P14(AD12/A12)
  (INT6/TI6)P84  17 |                                 | 48  P13(AD11/A11)
  (INT7/TI7)P85  18 |                                 | 47  P12(AD10/A10)
       (TO6)P86  19 |                                 | 46  P11(AD9/A9)
       (TO7)P87  20 |                                 | 45  P10(AD8/A8)
           _NMI  21 |                                 | 44  P07(AD7)
        _WDTOUT  22 |                                 | 43  P06(AD6)
         _RESET  23 |                                 | 42  P05(AD5)
            CLK  24 |_________________________________| 41  P04(AD4)
                      | | | | | | | | | | | | | | | |
            VSS  25 --+ | | | | | | | | | | | | | | +-- 40  P03(AD3)
             X1  26 ----+ | | | | | | | | | | | | +---- 39  P02(AD2)
             X2  27 ------+ | | | | | | | | | | +------ 38  P01(AD1)
            _EA  28 --------+ | | | | | | | | +-------- 37  P00(AD0)
      (TXD0)P90  29 ----------+ | | | | | | +---------- 36  VCC
      (RXD0)P91  30 ------------+ | | | | +------------ 35  ALE
     (_CTS0)P92  31 --------------+ | | +-------------- 34  P95(SCLK1)
      (TXD1)P93  32 ----------------+ +---------------- 33  P94(RXD1)

****************************************************************************/

#ifndef MAME_CPU_TLCS900_TMP96C141_H
#define MAME_CPU_TLCS900_TMP96C141_H

#pragma once

#include "tlcs900.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tmp96c141_device

class tmp96c141_device : public tlcs900_device
{
public:
	// device type constructor
	tmp96c141_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// TODO: configuration helpers

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_set_input(int inputnum, int state) override;

	// tlcs900_device overrides
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

private:
	void internal_mem(address_map &map) ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(TMP96C141, tmp96c141_device)

#endif // MAME_CPU_TLCS900_TMP96C141_H
