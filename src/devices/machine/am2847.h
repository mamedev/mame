// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    AMD Am2847/Am2896 Quad 80/96-Bit Static Shift Registers

    Pin-compatible with:
    * 2532B
    * TMS3120
    * TMS3409
    * MK1007
    * 3347

***********************************************************************

    Connection Diagram:
              ___ ___
    OUT A  1 |*  u   | 16  Vss
     RC A  2 |       | 15  IN D
     IN A  3 |       | 14  RC D
    OUT B  4 |       | 13  OUT D
     RC B  5 |       | 12  Vgg
     IN B  6 |       | 11  CP
    OUT C  7 |       | 10  IN C
      Vdd  8 |_______| 9   RC C

    Logic Symbol:

                2      5      9      14
                |      |      |      |
           _____|______|______|______|_____
          |   RC A   RC B   RC C   RC D    |
     3 ---| IN A                     OUT A |--- 1
     6 ---| IN A                     OUT B |--- 4
    10 ---| IN A                     OUT C |--- 7
    15 ---| IN A                     OUT D |--- 13
    11 ---| CP                             |
          |________________________________|


**********************************************************************/

#ifndef MAME_MACHINE_AM2847_H
#define MAME_MACHINE_AM2847_H

#pragma once

class am2847_base_device : public device_t
{
public:
	void in_a_w(int state);
	void in_b_w(int state);
	void in_c_w(int state);
	void in_d_w(int state);
	void in_w(uint8_t data);

	void rc_a_w(int state);
	void rc_b_w(int state);
	void rc_c_w(int state);
	void rc_d_w(int state);
	void rc_w(uint8_t data);

	void cp_w(int state);

	uint8_t out_r() const { return m_out; }

protected:
	am2847_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, size_t size);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void init();
	void shift();

	class shifter
	{
	public:
		shifter() : m_buffer_size(0) { }
		void alloc(size_t size);
		void init();
		int shift(bool recycle, int in);
	protected:
		std::unique_ptr<uint16_t[]> m_buffer;
		size_t m_buffer_size;
	};

	shifter m_shifters[4];
	uint8_t m_in;
	uint8_t m_out;
	uint8_t m_rc;
	int m_cp;

	const size_t m_buffer_size;
};

class am2847_device : public am2847_base_device
{
public:
	// construction/destruction
	am2847_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class am2849_device : public am2847_base_device
{
public:
	// construction/destruction
	am2849_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class tms3409_device : public am2847_base_device
{
public:
	// construction/destruction
	tms3409_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(AM2847,  am2847_device)
DECLARE_DEVICE_TYPE(AM2849,  am2849_device)
DECLARE_DEVICE_TYPE(TMS3409, tms3409_device)

#endif // MAME_MACHINE_AM2847_H
