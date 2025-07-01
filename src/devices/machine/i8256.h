// license:BSD-3-Clause
/**********************************************************************

    Intel 8256(AH) Multifunction microprocessor support controller emulation

**********************************************************************
                            _____   _____
                   AD0   1 |*    \_/     | 40  Vcc
                   AD1   2 |             | 39  P10
                   AD2   3 |             | 38  P11
                   AD3   4 |             | 37  P12
                   AD4   5 |             | 36  P13
                   DB5   6 |             | 35  P14
                   DB6   7 |             | 34  P15
                   DB7   8 |             | 33  P16
                   ALE   9 |             | 32  P17
                    RD  10 |    8256     | 31  P20
                    WR  11 |    8256AH   | 30  P21
                 RESET  12 |             | 29  P22
                    CS  13 |             | 28  P23
                  INTA  14 |             | 27  P24
                   INT  15 |             | 26  P25
                EXTINT  16 |             | 25  P26
                   CLK  17 |             | 24  P27
                   RxC  18 |             | 23  TxD
                   RxD  19 |             | 22  TxC
                   GND  20 |_____________| 21  CTS

**********************************************************************/

#ifndef MAME_MACHINE_I8256_H
#define MAME_MACHINE_I8256_H

#pragma once

#include "diserial.h"
#include "devcb.h"

class i8256_device : public device_t, public device_serial_interface
{
public:
    i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    auto extint_callback()  { return m_extint_cb.bind(); }
    auto inta_callback()    { return m_inta_cb.bind(); }
    auto in_p1_callback()   { return m_in_p1_cb.bind(); }
    auto in_p2_callback()   { return m_in_p2_cb.bind(); }
    auto out_p1_callback()  { return m_out_p1_cb.bind(); }
    auto out_p2_callback()  { return m_out_p2_cb.bind(); }

    virtual void device_start() override;
    virtual void device_reset() override;

	void write_cts(int state);
    void write_rxd(int state);
    void write_rxc(int state);
	void write_txc(int state);
	
    void write(offs_t offset, u8 data);
    uint8_t read(offs_t offset);

private:
    devcb_write_line m_extint_cb;
    devcb_write_line m_inta_cb;
    devcb_read8 m_in_p1_cb;
	devcb_read8 m_in_p2_cb;
	devcb_write8 m_out_p1_cb;
	devcb_write8 m_out_p2_cb;

	bool m_cts;
	bool m_rxd;
	bool m_rxc;
	bool m_txc;

    uint8_t m_command1, m_command2, m_command3, m_mode, m_interrupts;

    uint8_t m_port1_control, m_port1_int;
    uint8_t m_port2_int;

    void output_pc();

    emu_timer *m_timers[5];


    enum // MUART REGISTERS
	{
		REG_CMD1,
        REG_CMD2,
        REG_CMD3,
        REG_MODE,
        REG_PORT1C,
        REG_INTEN,
        REG_INTAD,
        REG_BUFFER,
        REG_PORT1,
        REG_PORT2,
        REG_TIMER1,
        REG_TIMER2,
        REG_TIMER3,
        REG_TIMER4,
        REG_TIMER5,
        REG_STATUS,
	};

    enum
    {
        CMD1_FRQ,
        CMD1_8086,
        CMD1_BITI,
        CMD1_BRKI,
        CMD1_S0,
        CMD1_S1,
        CMD1_L0,
        CMD1_L1
    };

    enum
    {
        STOP_1,
        STOP_15,
        STOP_2,
        STOP_075
    };

    enum
    {
        CHARLEN_8,
        CHARLEN_7,
        CHARLEN_6,
        CHARLEN_5
    };

    enum
    {
        CMD2_B0,
        CMD2_B1,
        CMD2_B2,
        CMD2_B3,
        CMD2_C0,
        CMD2_C1,
        CMD2_EP,
        CMD2_PEN
    };

    enum
    {
        BAUD_TXC,
        BAUD_TXC64,
        BAUD_TXC32,
        BAUD_19200,
        BAUD_9600,
        BAUD_4800,
        BAUD_2400,
        BAUD_1200,
        BAUD_600,
        BAUD_300,
        BAUD_200,
        BAUD_150,
        BAUD_110,
        BAUD_100,
        BAUD_75,
        BAUD_50
    };
    const int baudRates[16] = { 0, 0, 0, 19200, 9600, 4800, 2400, 1200, 600, 300, 200, 150, 110, 100, 75, 50 };

    enum
    {
        SCLK_DIV5, // 5.12 MHz
        SCLK_DIV3, // 3.072 MHz
        SCLK_DIV2, // 2.048 MHz
        SCLK_DIV1  // 1.024 MHz
    };
    const int sysclockDivider[4] = {5,3,2,1};

    enum
    {
        CMD3_RST,
        CMD3_TBRK,
        CMD3_SBRK,
        CMD3_END,
        CMD3_NIE,
        CMD3_IAE,
        CMD3_RxE,
        CMD3_SET
    };

    enum
    {
        MODE_P2C0,
        MODE_P2C1,
        MODE_P2C2,
        MODE_CT2,
        MODE_CT3,
        MODE_T5C,
        MODE_T24,
        MODE_T35
    };

    enum // Upper / Lower
    {
        PORT2C_II,
        PORT2C_IO,
        PORT2C_OI,
        PORT2C_OO,
        PORT2C_HI,
        PORT2C_HO,
        PORT2C_DNU,
        PORT2C_TEST
    };

    enum
    {
        STATUS_FE,
        STATUS_OE,
        STATUS_PE,
        STATUS_BD,
        STATUS_TRE,
        STATUS_TBE,
        STATUS_RBF,
        STATUS_INT
    };

    enum
    {
        MOD_DSC,
        MOD_TME,
        MOD_RS0,
        MOD_RS1,
        MOD_RS2,
        MOD_RS3,
        MOD_RS4,
        MOD_0
    };

};

DECLARE_DEVICE_TYPE(I8256, i8256_device)

#endif // MAME_MACHINE_I8256_H