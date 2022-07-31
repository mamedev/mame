// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    KR1601RR1 1024x4 bit EAROM

    Same geometry as GI ER2401, but not pin-compatible.

    CS ER PR RD Ax
    -- -- -- -- --
     0  x  x  x  x  idle
     1  0  1  0  x  erase all
     1  0  0  0  A  erase single
     1  1  0  0  A  write
     1  1  1  1  A  read

****************************************************************************
                            _____   _____
                    A9   1 |*    \_/     | 24
                    CS   2 |             | 23  A8
                    D0   3 |             | 22  A7
                   _OV   4 |             | 21  A6
                    D1   5 |             | 20  A5
                    A0   6 |             | 19  A4
                    A3   7 |  KR1601RR1  | 18  _ER
                    A1   8 |             | 17
                    A2   9 |             | 16
                    D2  10 |             | 15  _UPR
                    D3  11 |             | 14  _PR
                   _U1  12 |_____________| 13  RD

***************************************************************************/

#ifndef MAME_MACHINE_KR1601RR1_H
#define MAME_MACHINE_KR1601RR1_H

#pragma once


// ======================> kr1601rr1_device

class kr1601rr1_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	kr1601rr1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	enum { EAROM_SIZE = 1024 };
	enum {
		EAROM_IDLE,
		EAROM_READ,
		EAROM_WRITE,
		EAROM_ERASE,
		EAROM_ERASE_ALL
	} m_earom_mode = EAROM_IDLE;

	uint8_t m_earom[EAROM_SIZE];
};

// device type definition
DECLARE_DEVICE_TYPE(KR1601RR1, kr1601rr1_device)

#endif // MAME_MACHINE_KR1601RR1_H
