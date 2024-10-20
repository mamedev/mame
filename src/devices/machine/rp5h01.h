// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    RP5H01 - Ricoh 64x1bit(+8bit) PROM with 6/7-bit counter

****************************************************************************
                      ___________
            DATA   1 |*          | 8  COUNTER OUT
                     |           |
         _CE/Vpp   2 |   RP5H01  | 7  RESET
                     |   RF5H01  |
             Vcc   3 |           | 6  DATA CLOCK
                     |           |
             GND   4 |___________| 5  TEST

***************************************************************************/

#ifndef MAME_MACHINE_RP5H01_H
#define MAME_MACHINE_RP5H01_H


/***************************************************************************
    PARAMETERS
***************************************************************************/

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class rp5h01_device : public device_t
{
public:
	rp5h01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void enable_w(int state);   // /CE
	void reset_w(int state);    // RESET
	void cs_w(int state);       // CS
	void clock_w(int state);    // DATA CLOCK (active low)
	void test_w(int state);     // TEST
	int counter_r();            // COUNTER OUT
	int data_r();               // DATA

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	/* these also work as the address masks */
	enum {
		COUNTER_MODE_6_BITS = 0x3f,
		COUNTER_MODE_7_BITS = 0x7f
	};

	static uint8_t const s_initial_data[0x10];

	// internal state
	int m_counter;
	int m_counter_mode;   /* test pin */
	int m_enabled;        /* chip enable */
	int m_old_reset;      /* reset pin state (level-triggered) */
	int m_old_clock;      /* clock pin state (level-triggered) */
	uint8_t const *m_data;
	optional_region_ptr<uint8_t> m_rom;
};

DECLARE_DEVICE_TYPE(RP5H01, rp5h01_device)


/*
 * Device uses memory region
 * with the same tag as the one
 * assigned to device.
 */

#endif // MAME_MACHINE_RP5H01_H
