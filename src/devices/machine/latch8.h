// license:BSD-3-Clause
// copyright-holders:Couriersud
/**********************************************************************

    8 bit latch interface and emulation

    Generic emulation of 74LS174/175, 74LS259 and other latches.
    Apart from providing synched latch operation, these
    latches can be configured to read their input bitwise from other
    devices as well.

    Please see audio/dkong.c for examples.

**********************************************************************/

#ifndef MAME_MACHINE_LATCH8_H
#define MAME_MACHINE_LATCH8_H

#include "sound/discrete.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class latch8_device : public device_t
{
public:
	latch8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	/* write & read full byte */

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	/* reset the latch */

	DECLARE_WRITE8_MEMBER( reset_w );

	/* read bit x                 */
	/* return (latch >> x) & 0x01 */

	DECLARE_READ_LINE_MEMBER( bit0_r ) { return BIT(m_value, 0); }
	DECLARE_READ_LINE_MEMBER( bit1_r ) { return BIT(m_value, 1); }
	DECLARE_READ_LINE_MEMBER( bit2_r ) { return BIT(m_value, 2); }
	DECLARE_READ_LINE_MEMBER( bit3_r ) { return BIT(m_value, 3); }
	DECLARE_READ_LINE_MEMBER( bit4_r ) { return BIT(m_value, 4); }
	DECLARE_READ_LINE_MEMBER( bit5_r ) { return BIT(m_value, 5); }
	DECLARE_READ_LINE_MEMBER( bit6_r ) { return BIT(m_value, 6); }
	DECLARE_READ_LINE_MEMBER( bit7_r ) { return BIT(m_value, 7); }

	/* read inverted bit x        */
	/* return (latch >> x) & 0x01 */

	DECLARE_READ_LINE_MEMBER( bit0_q_r ) { return BIT(m_value, 0) ^ 1; }
	DECLARE_READ_LINE_MEMBER( bit1_q_r ) { return BIT(m_value, 1) ^ 1; }
	DECLARE_READ_LINE_MEMBER( bit2_q_r ) { return BIT(m_value, 2) ^ 1; }
	DECLARE_READ_LINE_MEMBER( bit3_q_r ) { return BIT(m_value, 3) ^ 1; }
	DECLARE_READ_LINE_MEMBER( bit4_q_r ) { return BIT(m_value, 4) ^ 1; }
	DECLARE_READ_LINE_MEMBER( bit5_q_r ) { return BIT(m_value, 5) ^ 1; }
	DECLARE_READ_LINE_MEMBER( bit6_q_r ) { return BIT(m_value, 6) ^ 1; }
	DECLARE_READ_LINE_MEMBER( bit7_q_r ) { return BIT(m_value, 7) ^ 1; }

	/* write bit x from data into bit determined by offset */
	/* latch = (latch & ~(1<<offset)) | (((data >> x) & 0x01) << offset) */

	DECLARE_WRITE8_MEMBER( bit0_w );
	DECLARE_WRITE8_MEMBER( bit1_w );
	DECLARE_WRITE8_MEMBER( bit2_w );
	DECLARE_WRITE8_MEMBER( bit3_w );
	DECLARE_WRITE8_MEMBER( bit4_w );
	DECLARE_WRITE8_MEMBER( bit5_w );
	DECLARE_WRITE8_MEMBER( bit6_w );
	DECLARE_WRITE8_MEMBER( bit7_w );

	void set_maskout(uint32_t maskout) { m_maskout = maskout; }
	void set_xorvalue(uint32_t xorvalue) { m_xorvalue = xorvalue; }
	void set_nosync(uint32_t nosync) { m_nosync = nosync; }

	template <unsigned N, class Object> devcb_base &set_write_cb(Object &&cb) { return m_write_cb[N].set_callback(std::forward<Object>(cb)); }

	template <unsigned N, class Object> devcb_base &set_read_cb(Object &&cb) { return m_read_cb[N].set_callback(std::forward<Object>(cb)); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_validity_check(validity_checker &valid) const override;

	TIMER_CALLBACK_MEMBER( timerproc );
	void update(uint8_t new_val, uint8_t mask);
	inline void bitx_w(int bit, offs_t offset, uint8_t data);

private:
	// internal state
	uint8_t            m_value;
	uint8_t            m_has_write;
	uint8_t            m_has_read;

	/* only for byte reads, does not affect bit reads and node_map */
	uint32_t           m_maskout;
	uint32_t           m_xorvalue;  /* after mask */
	uint32_t           m_nosync;

	devcb_write_line   m_write_cb[8];
	devcb_read_line    m_read_cb[8];
};

DECLARE_DEVICE_TYPE(LATCH8, latch8_device)

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

/* add device */
#define MCFG_LATCH8_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LATCH8, 0)

/* Bit mask specifying bits to be masked *out* */
#define MCFG_LATCH8_MASKOUT(_maskout) \
	downcast<latch8_device &>(*device).set_maskout(_maskout);

/* Bit mask specifying bits to be inverted */
#define MCFG_LATCH8_INVERT(_xor) \
	downcast<latch8_device &>(*device).set_xorvalue(_xor);

/* Bit mask specifying bits not needing cpu synchronization. */
#define MCFG_LATCH8_NOSYNC(_nosync) \
	downcast<latch8_device &>(*device).set_nosync(_nosync);

/* Write bit to discrete node */
#define MCFG_LATCH8_WRITE_0(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_write_cb<0>(DEVCB_##_devcb);

#define MCFG_LATCH8_WRITE_1(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_write_cb<1>(DEVCB_##_devcb);

#define MCFG_LATCH8_WRITE_2(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_write_cb<2>(DEVCB_##_devcb);

#define MCFG_LATCH8_WRITE_3(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_write_cb<3>(DEVCB_##_devcb);

#define MCFG_LATCH8_WRITE_4(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_write_cb<4>(DEVCB_##_devcb);

#define MCFG_LATCH8_WRITE_5(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_write_cb<5>(DEVCB_##_devcb);

#define MCFG_LATCH8_WRITE_6(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_write_cb<6>(DEVCB_##_devcb);

#define MCFG_LATCH8_WRITE_7(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_write_cb<7>(DEVCB_##_devcb);

/* Upon read, replace bits by reading from another device handler */
#define MCFG_LATCH8_READ_0(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_read_cb<0>(DEVCB_##_devcb);

#define MCFG_LATCH8_READ_1(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_read_cb<1>(DEVCB_##_devcb);

#define MCFG_LATCH8_READ_2(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_read_cb<2>(DEVCB_##_devcb);

#define MCFG_LATCH8_READ_3(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_read_cb<3>(DEVCB_##_devcb);

#define MCFG_LATCH8_READ_4(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_read_cb<4>(DEVCB_##_devcb);

#define MCFG_LATCH8_READ_5(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_read_cb<5>(DEVCB_##_devcb);

#define MCFG_LATCH8_READ_6(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_read_cb<6>(DEVCB_##_devcb);

#define MCFG_LATCH8_READ_7(_devcb) \
	devcb = &downcast<latch8_device &>(*device).set_read_cb<7>(DEVCB_##_devcb);


#endif // MAME_MACHINE_LATCH8_H
