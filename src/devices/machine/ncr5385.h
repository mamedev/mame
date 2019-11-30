// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************

    NCR 5385 SCSI Controller emulation

    TODO:
    - Everything. Currently, just enough is implemented to make the
      Philips VP415 CPU / Datagrabber board satisfied that the
      controller has passed its internal diagnostics.

************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 48  VCC
                    D1   2 |             | 47  D3
                    D0   3 |             | 46  D4
                 RESET   4 |             | 45  D5
                   ATN   5 |             | 44  D6
                   IGS   6 |             | 43  D7
                   I/O   7 |             | 42  BSYOUT
                   C/D   8 |             | 41  SB7
                   MSG   9 |             | 40  SB6
                   ACK  10 |             | 39  SB5
                   REQ  11 |             | 38  SB4
                  /ID2  12 |  NCR 5385E  | 37  SB3
                  /ID1  13 |             | 36  SB2
                  /ID0  14 |             | 35  SB1
                   ARB  15 |             | 34  SB9
                   CLK  16 |             | 33  SBP
                BSY IN  17 |             | 32  SELOUT
                SEL IN  18 |             | 31  /RD
                   INT  19 |             | 30  /WR
                 /SBEN  20 |             | 29  DREQ
                   /CS  21 |             | 28  TGS
                    A0  22 |             | 27  /DACK
                    A1  23 |             | 26  A3
                   GND  24 |_____________| 25  A2

************************************************************************/

#ifndef MAME_MACHINE_NCR5385_H
#define MAME_MACHINE_NCR5385_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ncr5385_device

class ncr5385_device : public device_t
{
public:
	// construction/destruction
	ncr5385_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq() { return m_int.bind(); }

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		STATE_IDLE,
		STATE_DIAGNOSTIC_GOOD_PARITY,
		STATE_DIAGNOSTIC_BAD_PARITY,
	};

	enum
	{
		DIAG_TURN_MISCOMPARE_INITIAL = 0x08,
		DIAG_TURN_MISCOMPARE_FINAL   = 0x10,
		DIAG_TURN_GOOD_PARITY        = 0x18,
		DIAG_TURN_BAD_PARITY         = 0x20,
		DIAG_COMPLETE = 0x80,

		DIAG_COMPLETE_BIT = 7,
	};

	enum
	{
		INT_FUNC_COMPLETE = 0x01,
		INT_INVALID_CMD   = 0x40,

		INT_FUNC_COMPLETE_BIT = 0,
		INT_INVALID_CMD_BIT   = 6,
	};

	enum
	{
		AUX_STATUS_TC_ZERO    = 0x02,
		AUX_STATUS_PAUSED     = 0x04,
		AUX_STATUS_PARITY_ERR = 0x40,
		AUX_STATUS_DATA_FULL  = 0x80,

		AUX_STATUS_TC_ZERO_BIT    = 1,
		AUX_STATUS_PAUSED_BIT     = 2,
		AUX_STATUS_PARITY_ERR_BIT = 6,
		AUX_STATUS_DATA_FULL_BIT  = 7,
	};

	enum
	{
		CTRL_SELECT   = 0x01,
		CTRL_RESELECT = 0x02,
		CTRL_PARITY   = 0x04,

		CTRL_SELECT_BIT   = 0,
		CTRL_RESELECT_BIT = 1,
		CTRL_PARITY_BIT   = 2,
	};

	devcb_write_line m_int;

	uint32_t m_state;
	uint8_t m_ctrl_reg;
	uint8_t m_int_reg;
	uint8_t m_aux_status_reg;
	uint8_t m_diag_status_reg;
};

// device type definition
DECLARE_DEVICE_TYPE(NCR5385, ncr5385_device)

#endif // MAME_MACHINE_NCR5385_H
