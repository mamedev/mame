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

#ifndef __RP5H01_H__
#define __RP5H01_H__


/***************************************************************************
    PARAMETERS
***************************************************************************/

/* these also work as the address masks */
enum {
	COUNTER_MODE_6_BITS = 0x3f,
	COUNTER_MODE_7_BITS = 0x7f
};

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class rp5h01_device : public device_t
{
	static UINT8 s_initial_data[0x10];

public:
	rp5h01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( enable_w );   /* /CE */
	DECLARE_WRITE_LINE_MEMBER( reset_w );    /* RESET */
	DECLARE_WRITE_LINE_MEMBER( cs_w );       /* CS */
	DECLARE_WRITE_LINE_MEMBER( clock_w );    /* DATA CLOCK (active low) */
	DECLARE_WRITE_LINE_MEMBER( test_w );     /* TEST */
	DECLARE_READ_LINE_MEMBER( counter_r );   /* COUNTER OUT */
	DECLARE_READ_LINE_MEMBER( data_r );      /* DATA */

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	int m_counter;
	int m_counter_mode;   /* test pin */
	int m_enabled;        /* chip enable */
	int m_old_reset;      /* reset pin state (level-triggered) */
	int m_old_clock;      /* clock pin state (level-triggered) */
	optional_region_ptr<UINT8> m_data;
};

extern const device_type RP5H01;


#define MCFG_RP5H01_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, RP5H01, 0)

/*
 * Device uses memory region
 * with the same tag as the one
 * assigned to device.
 */

#endif /* __RP5H01_H__ */
