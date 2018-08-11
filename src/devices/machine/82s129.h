// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    82S129/6 1K-bit TTL bipolar PROM

******************************************************************************

    Connection Diagrams:

             N Package
              ___ ___
       A6  1 |*  u   | 16  Vcc
       A5  2 |       | 15  A7
       A4  3 |       | 14  /CE2
       A3  4 |       | 13  /CE1
       A0  5 |       | 12  O1
       A1  6 |       | 11  O2
       A2  7 |       | 10  O3
      GND  8 |_______| 9   O4


             A Package

          3   2   1  20  19
          |   |   |   |   |
       /---------------------|
       | A5  A6  NC  Vcc A7  |
       |                     |
    4 -| A4             /CE2 |- 18
    5 -| A3             /CE1 |- 17
    6 -| A0               O1 |- 16
    7 -| A1               NC |- 15
    8 -| A2               O2 |- 14
       |                     |
       | NC  GND NC  O4  O3  |
       |_____________________|
          |   |   |   |   |
          9  10  11  12  13

**********************************************************************/

#ifndef MAME_MACHINE_82S129_H
#define MAME_MACHINE_82S129_H

#pragma once


class prom82s129_base_device : public device_t
{
public:
	auto out_callback() { return m_out_func.bind(); }
	auto o1_callback() { return m_o1_func.bind(); }
	auto o2_callback() { return m_o2_func.bind(); }
	auto o3_callback() { return m_o3_func.bind(); }
	auto o4_callback() { return m_o4_func.bind(); }

	DECLARE_WRITE_LINE_MEMBER( ce1_w );
	DECLARE_WRITE_LINE_MEMBER( ce2_w );

	DECLARE_WRITE8_MEMBER( a_w );
	DECLARE_WRITE_LINE_MEMBER( a0_w );
	DECLARE_WRITE_LINE_MEMBER( a1_w );
	DECLARE_WRITE_LINE_MEMBER( a2_w );
	DECLARE_WRITE_LINE_MEMBER( a3_w );
	DECLARE_WRITE_LINE_MEMBER( a4_w );
	DECLARE_WRITE_LINE_MEMBER( a5_w );
	DECLARE_WRITE_LINE_MEMBER( a6_w );
	DECLARE_WRITE_LINE_MEMBER( a7_w );

	DECLARE_READ8_MEMBER( output_r );
	DECLARE_READ_LINE_MEMBER( o1_r );
	DECLARE_READ_LINE_MEMBER( o2_r );
	DECLARE_READ_LINE_MEMBER( o3_r );
	DECLARE_READ_LINE_MEMBER( o4_r );

	uint8_t get_output() const { return m_out; }

protected:
	// construction/destruction
	prom82s129_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void write_line(uint8_t line, uint8_t state);
	void init();
	void update();

	required_memory_region  m_region;

	// callbacks
	devcb_write_line m_out_func;
	devcb_write_line m_o1_func;
	devcb_write_line m_o2_func;
	devcb_write_line m_o3_func;
	devcb_write_line m_o4_func;

	// inputs
	uint8_t m_ce1;      // pin 13
	uint8_t m_ce2;      // pin 14
	uint8_t m_a;        // pins 5,6,7,4,3,2,1,15 from LSB to MSB

	// outputs
	uint8_t m_out;      // pins 12-9 from LSB to MSB

	// data
	std::unique_ptr<uint8_t[]>  m_data;

	static const uint32_t PROM_SIZE;
};

class prom82s126_device : public prom82s129_base_device
{
public:
	prom82s126_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class prom82s129_device : public prom82s129_base_device
{
public:
	prom82s129_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(PROM82S126, prom82s126_device)
DECLARE_DEVICE_TYPE(PROM82S129, prom82s129_device)

#endif // MAME_MACHINE_82S129_H
