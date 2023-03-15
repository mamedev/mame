// license:BSD-3-Clause
// copyright-holders:smf,Barry Rodewald
/***************************************************************************

    x2212.h

    Xicor X2212 256 x 4 bit Nonvolatile Static RAM.

****************************************************************************
                             ____   ____
                     A7   1 |*   \_/    | 18  Vcc
                     A4   2 |           | 17  A6
                     A3   3 |           | 16  A5
                     A2   4 |           | 15  I/O4
                     A1   5 |   X2212   | 14  I/O3
                     A0   6 |           | 13  I/O2
                    /CS   7 |           | 12  I/O1
                    Vss   8 |           | 11  /WE
                 /STORE   9 |___________| 10  /ARRAY RECALL

***************************************************************************/

#ifndef MAME_MACHINE_X2212_H
#define MAME_MACHINE_X2212_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> x2212_device

class x2212_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	x2212_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// some systems (like many early Atari games) wire up the /STORE signal
	// to fire on power-down, effectively creating an "auto-save" functionality
	void set_auto_save(bool auto_save) { m_auto_save = auto_save; }

	// I/O operations
	u8 read(address_space &space, offs_t offset);
	void write(offs_t offset, uint8_t data);

	void store(int state);
	void recall(int state);

protected:
	x2212_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int size_data);

	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	// configuration state
	bool                        m_auto_save;

	// internal state
	std::unique_ptr<u8[]> m_sram;
	std::unique_ptr<u8[]> m_e2prom;

	bool        m_store;
	bool        m_array_recall;

	int const m_size_data;
	optional_region_ptr<u8> m_default_data;

	// internal helpers
	void do_store();
	void do_recall();
};

class x2210_device : public x2212_device
{
public:
	x2210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};


// device type definition
DECLARE_DEVICE_TYPE(X2212, x2212_device)
DECLARE_DEVICE_TYPE(X2210, x2210_device)

#endif // MAME_MACHINE_X2212_H
