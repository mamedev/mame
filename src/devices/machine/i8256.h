// license:BSD-3-Clause
// copyright-holders:stonedDiscord
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

	auto inta_callback()    { return m_in_inta_cb.bind(); }
	auto int_callback()     { return m_out_int_cb.bind(); }
	auto extint_callback()  { return m_in_extint_cb.bind(); }

	void write_rxc(int state);
	void write_rxd(int state);
	void write_cts(int state);
	void write_txc(int state);

	auto txd_handler() { return m_txd_handler.bind(); }

	auto in_p2_callback()   { return m_in_p2_cb.bind(); }
	auto out_p2_callback()  { return m_out_p2_cb.bind(); }
	auto in_p1_callback()   { return m_in_p1_cb.bind(); }
	auto out_p1_callback()  { return m_out_p1_cb.bind(); }

	virtual void device_start() override;
	virtual void device_reset() override;

	TIMER_CALLBACK_MEMBER(timer_check);

	void write(offs_t offset, u8 data);
	uint8_t read(offs_t offset);

	uint8_t p1_r();
	void    p1_w(uint8_t data);
	uint8_t p2_r();
	void    p2_w(uint8_t data);

private:
	devcb_read_line m_in_inta_cb;
	devcb_write_line m_out_int_cb;
	devcb_read_line m_in_extint_cb;

	int32_t m_rxc;
	int32_t m_rxd;
	int32_t m_cts;
	int32_t m_txc;

	devcb_write_line m_txd_handler;

	devcb_read8 m_in_p2_cb;
	devcb_write8 m_out_p2_cb;
	devcb_read8 m_in_p1_cb;
	devcb_write8 m_out_p1_cb;

	uint8_t m_command1, m_command2, m_command3;
	uint8_t m_data_bits_count;
	parity_t m_parity;
	stop_bits_t m_stop_bits;

	uint8_t m_mode;
	uint8_t m_port1_control;
	uint8_t m_interrupts, m_current_interrupt_level;
	uint8_t m_tx_buffer, m_rx_buffer;
	uint8_t m_port1_int, m_port2_int;
	uint8_t m_timers[5];
	emu_timer *m_timer;

	uint8_t m_status, m_modification;

	uint8_t m_sync_byte_count, m_rxc_count, m_txc_count;
	uint8_t m_br_factor;
	uint8_t m_rxd_bits;
	uint8_t m_rx_data, m_tx_data;
	uint8_t m_sync1, m_sync2, m_sync8, m_sync16;

	void receive_clock();
	void sync1_rxc();
	void sync2_rxc();
	bool is_tx_enabled();
	void check_for_tx_start();
	void start_tx();
	void transmit_clock();
	void receive_character(uint8_t ch);

	enum // MUART REGISTERS
	{
		I8256_REG_CMD1,
		I8256_REG_CMD2,
		I8256_REG_CMD3,
		I8256_REG_MODE,
		I8256_REG_PORT1C,
		I8256_REG_INTEN,
		I8256_REG_INTAD,
		I8256_REG_BUFFER,
		I8256_REG_PORT1,
		I8256_REG_PORT2,
		I8256_REG_TIMER1,
		I8256_REG_TIMER2,
		I8256_REG_TIMER3,
		I8256_REG_TIMER4,
		I8256_REG_TIMER5,
		I8256_REG_STATUS,
	};

	enum
	{
		I8256_CMD1_FRQ,
		I8256_CMD1_8086,
		I8256_CMD1_BITI,
		I8256_CMD1_BRKI,
		I8256_CMD1_S0,
		I8256_CMD1_S1,
		I8256_CMD1_L0,
		I8256_CMD1_L1
	};

	enum
	{
		I8256_STOP_1,
		I8256_STOP_15,
		I8256_STOP_2,
		I8256_STOP_075
	};

	stop_bits_t stopBits[4] = {STOP_BITS_1, STOP_BITS_1_5, STOP_BITS_2, STOP_BITS_0};

	enum
	{
		I8256_CHARLEN_8,
		I8256_CHARLEN_7,
		I8256_CHARLEN_6,
		I8256_CHARLEN_5
	};

	enum
	{
		I8256_CMD2_B0,
		I8256_CMD2_B1,
		I8256_CMD2_B2,
		I8256_CMD2_B3,
		I8256_CMD2_C0,
		I8256_CMD2_C1,
		I8256_CMD2_EVEN_PARITY,
		I8256_CMD2_PARITY_ENABLE
	};

	enum
	{
		I8256_BAUD_TXC,
		I8256_BAUD_TXC64,
		I8256_BAUD_TXC32,
		I8256_BAUD_19200,
		I8256_BAUD_9600,
		I8256_BAUD_4800,
		I8256_BAUD_2400,
		I8256_BAUD_1200,
		I8256_BAUD_600,
		I8256_BAUD_300,
		I8256_BAUD_200,
		I8256_BAUD_150,
		I8256_BAUD_110,
		I8256_BAUD_100,
		I8256_BAUD_75,
		I8256_BAUD_50
	};
	const int baudRates[16] = { 0, 0, 0, 19200, 9600, 4800, 2400, 1200, 600, 300, 200, 150, 110, 100, 75, 50 };

	enum
	{
		I8256_SCLK_DIV5, // 5.12 MHz
		I8256_SCLK_DIV3, // 3.072 MHz
		I8256_SCLK_DIV2, // 2.048 MHz
		I8256_SCLK_DIV1  // 1.024 MHz
	};
	const char sysclockDivider[4] = {5,3,2,1};

	enum
	{
		I8256_CMD3_RST,
		I8256_CMD3_TBRK,
		I8256_CMD3_SBRK,
		I8256_CMD3_END,
		I8256_CMD3_NIE,
		I8256_CMD3_IAE,
		I8256_CMD3_RxE,
		I8256_CMD3_SET
	};

	enum
	{
		I8256_INT_TIMER1,
		I8256_INT_TIMER2,
		I8256_INT_EXTINT,
		I8256_INT_TIMER3,
		I8256_INT_RX,
		I8256_INT_TX,
		I8256_INT_TIMER4,
		I8256_INT_TIMER5
	};

	const char timer_interrupt[5] = {I8256_INT_TIMER1, I8256_INT_TIMER2, I8256_INT_TIMER3, I8256_INT_TIMER4, I8256_INT_TIMER5};

	enum
	{
		I8256_MODE_P2C0,
		I8256_MODE_P2C1,
		I8256_MODE_P2C2,
		I8256_MODE_CT2,
		I8256_MODE_CT3,
		I8256_MODE_T5C,
		I8256_MODE_T24,
		I8256_MODE_T35
	};

	enum // Upper / Lower
	{
		I8256_PORT2C_II,
		I8256_PORT2C_IO,
		I8256_PORT2C_OI,
		I8256_PORT2C_OO,
		I8256_PORT2C_HI,
		I8256_PORT2C_HO,
		I8256_PORT2C_DNU,
		I8256_PORT2C_TEST
	};

	enum
	{
		I8256_STATUS_FRAMING_ERROR,
		I8256_STATUS_OVERRUN_ERROR,
		I8256_STATUS_PARITY_ERROR,
		I8256_STATUS_BREAK,
		I8256_STATUS_TR_EMPTY,
		I8256_STATUS_TB_EMPTY,
		I8256_STATUS_RB_FULL,
		I8256_STATUS_INT
	};

	enum
	{
		I8256_MOD_DSC,
		I8256_MOD_TME,
		I8256_MOD_RS0,
		I8256_MOD_RS1,
		I8256_MOD_RS2,
		I8256_MOD_RS3,
		I8256_MOD_RS4,
		I8256_MOD_0
	};
};

DECLARE_DEVICE_TYPE(I8256, i8256_device)

#endif // MAME_MACHINE_I8256_H
