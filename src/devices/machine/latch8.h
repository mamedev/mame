// license:BSD-3-Clause
// copyright-holders:Couriersud
/**********************************************************************

    8 bit latch interface and emulation

    Generic emulation of 74LS174/175, 74LS259 and other latches.
    Apart from providing synched latch operation, these
    latches can be configured to read their input bitwise from other
    devices as well and individual bits can be connected to
    discrete nodes.

    Please see audio/dkong.c for examples.

**********************************************************************/

#ifndef __LATCH8_H_
#define __LATCH8_H_

#include "sound/discrete.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class latch8_device : public device_t
{
public:
	latch8_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);


	/* write & read full byte */

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	/* reset the latch */

	DECLARE_WRITE8_MEMBER( reset_w );

	/* read bit x                 */
	/* return (latch >> x) & 0x01 */

	DECLARE_READ8_MEMBER( bit0_r );
	DECLARE_READ8_MEMBER( bit1_r );
	DECLARE_READ8_MEMBER( bit2_r );
	DECLARE_READ8_MEMBER( bit3_r );
	DECLARE_READ8_MEMBER( bit4_r );
	DECLARE_READ8_MEMBER( bit5_r );
	DECLARE_READ8_MEMBER( bit6_r );
	DECLARE_READ8_MEMBER( bit7_r );

	/* read inverted bit x        */
	/* return (latch >> x) & 0x01 */

	DECLARE_READ8_MEMBER( bit0_q_r );
	DECLARE_READ8_MEMBER( bit1_q_r );
	DECLARE_READ8_MEMBER( bit2_q_r );
	DECLARE_READ8_MEMBER( bit3_q_r );
	DECLARE_READ8_MEMBER( bit4_q_r );
	DECLARE_READ8_MEMBER( bit5_q_r );
	DECLARE_READ8_MEMBER( bit6_q_r );
	DECLARE_READ8_MEMBER( bit7_q_r );

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

	static void set_maskout(device_t &device, UINT32 maskout) { downcast<latch8_device &>(device).m_maskout = maskout; }
	static void set_xorvalue(device_t &device, UINT32 xorvalue) { downcast<latch8_device &>(device).m_xorvalue = xorvalue; }
	static void set_nosync(device_t &device, UINT32 nosync) { downcast<latch8_device &>(device).m_nosync = nosync; }

	template<class _Object> static devcb_base &set_write_0(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[0] = offset; return downcast<latch8_device &>(device).m_write_0.set_callback(object); }
	template<class _Object> static devcb_base &set_write_1(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[1] = offset; return downcast<latch8_device &>(device).m_write_1.set_callback(object); }
	template<class _Object> static devcb_base &set_write_2(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[2] = offset; return downcast<latch8_device &>(device).m_write_2.set_callback(object); }
	template<class _Object> static devcb_base &set_write_3(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[3] = offset; return downcast<latch8_device &>(device).m_write_3.set_callback(object); }
	template<class _Object> static devcb_base &set_write_4(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[4] = offset; return downcast<latch8_device &>(device).m_write_4.set_callback(object); }
	template<class _Object> static devcb_base &set_write_5(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[5] = offset; return downcast<latch8_device &>(device).m_write_5.set_callback(object); }
	template<class _Object> static devcb_base &set_write_6(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[6] = offset; return downcast<latch8_device &>(device).m_write_6.set_callback(object); }
	template<class _Object> static devcb_base &set_write_7(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[7] = offset; return downcast<latch8_device &>(device).m_write_7.set_callback(object); }

	template<class _Object> static devcb_base &set_read_0(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[0] = offset; return downcast<latch8_device &>(device).m_read_0.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[1] = offset; return downcast<latch8_device &>(device).m_read_1.set_callback(object); }
	template<class _Object> static devcb_base &set_read_2(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[2] = offset; return downcast<latch8_device &>(device).m_read_2.set_callback(object); }
	template<class _Object> static devcb_base &set_read_3(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[3] = offset; return downcast<latch8_device &>(device).m_read_3.set_callback(object); }
	template<class _Object> static devcb_base &set_read_4(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[4] = offset; return downcast<latch8_device &>(device).m_read_4.set_callback(object); }
	template<class _Object> static devcb_base &set_read_5(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[5] = offset; return downcast<latch8_device &>(device).m_read_5.set_callback(object); }
	template<class _Object> static devcb_base &set_read_6(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[6] = offset; return downcast<latch8_device &>(device).m_read_6.set_callback(object); }
	template<class _Object> static devcb_base &set_read_7(device_t &device, _Object object, UINT32 offset) { downcast<latch8_device &>(device).m_offset[7] = offset; return downcast<latch8_device &>(device).m_read_7.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_validity_check(validity_checker &valid) const override;

	TIMER_CALLBACK_MEMBER( timerproc );
	void update(UINT8 new_val, UINT8 mask);
	inline UINT8 bitx_r( offs_t offset, int bit);
	inline void bitx_w(int bit, offs_t offset, UINT8 data);
private:
	// internal state
	UINT8            m_value;
	UINT8            m_has_write;
	UINT8            m_has_read;

	/* only for byte reads, does not affect bit reads and node_map */
	UINT32           m_maskout;
	UINT32           m_xorvalue;  /* after mask */
	UINT32           m_nosync;

	devcb_write8    m_write_0;
	devcb_write8    m_write_1;
	devcb_write8    m_write_2;
	devcb_write8    m_write_3;
	devcb_write8    m_write_4;
	devcb_write8    m_write_5;
	devcb_write8    m_write_6;
	devcb_write8    m_write_7;

	devcb_read8     m_read_0;
	devcb_read8     m_read_1;
	devcb_read8     m_read_2;
	devcb_read8     m_read_3;
	devcb_read8     m_read_4;
	devcb_read8     m_read_5;
	devcb_read8     m_read_6;
	devcb_read8     m_read_7;

	UINT32           m_offset[8];
};

extern const device_type LATCH8;
/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

/* add device */
#define MCFG_LATCH8_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LATCH8, 0)

/* Bit mask specifying bits to be masked *out* */
#define MCFG_LATCH8_MASKOUT(_maskout) \
	latch8_device::set_maskout(*device, _maskout);

/* Bit mask specifying bits to be inverted */
#define MCFG_LATCH8_INVERT(_xor) \
	latch8_device::set_xorvalue(*device, _xor);

/* Bit mask specifying bits not needing cpu synchronization. */
#define MCFG_LATCH8_NOSYNC(_nosync) \
	latch8_device::set_nosync(*device, _nosync);

/* Write bit to discrete node */
#define MCFG_LATCH8_WRITE_0(_devcb, _node) \
	devcb = &latch8_device::set_write_0(*device, DEVCB_##_devcb, _node);

#define MCFG_LATCH8_WRITE_1(_devcb, _node) \
	devcb = &latch8_device::set_write_1(*device, DEVCB_##_devcb, _node);

#define MCFG_LATCH8_WRITE_2(_devcb, _node) \
	devcb = &latch8_device::set_write_2(*device, DEVCB_##_devcb, _node);

#define MCFG_LATCH8_WRITE_3(_devcb, _node) \
	devcb = &latch8_device::set_write_3(*device, DEVCB_##_devcb, _node);

#define MCFG_LATCH8_WRITE_4(_devcb, _node) \
	devcb = &latch8_device::set_write_4(*device, DEVCB_##_devcb, _node);

#define MCFG_LATCH8_WRITE_5(_devcb, _node) \
	devcb = &latch8_device::set_write_5(*device, DEVCB_##_devcb, _node);

#define MCFG_LATCH8_WRITE_6(_devcb, _node) \
	devcb = &latch8_device::set_write_6(*device, DEVCB_##_devcb, _node);

#define MCFG_LATCH8_WRITE_7(_devcb, _node) \
	devcb = &latch8_device::set_write_7(*device, DEVCB_##_devcb, _node);

/* Upon read, replace bits by reading from another device handler */
#define MCFG_LATCH8_READ_0(_devcb, _from_bit) \
	devcb = &latch8_device::set_read_0(*device, DEVCB_##_devcb, _from_bit);

#define MCFG_LATCH8_READ_1(_devcb, _from_bit) \
	devcb = &latch8_device::set_read_1(*device, DEVCB_##_devcb, _from_bit);

#define MCFG_LATCH8_READ_2(_devcb, _from_bit) \
	devcb = &latch8_device::set_read_2(*device, DEVCB_##_devcb, _from_bit);

#define MCFG_LATCH8_READ_3(_devcb, _from_bit) \
	devcb = &latch8_device::set_read_3(*device, DEVCB_##_devcb, _from_bit);

#define MCFG_LATCH8_READ_4(_devcb, _from_bit) \
	devcb = &latch8_device::set_read_4(*device, DEVCB_##_devcb, _from_bit);

#define MCFG_LATCH8_READ_5(_devcb, _from_bit) \
	devcb = &latch8_device::set_read_5(*device, DEVCB_##_devcb, _from_bit);

#define MCFG_LATCH8_READ_6(_devcb, _from_bit) \
	devcb = &latch8_device::set_read_6(*device, DEVCB_##_devcb, _from_bit);

#define MCFG_LATCH8_READ_7(_devcb, _from_bit) \
	devcb = &latch8_device::set_read_7(*device, DEVCB_##_devcb, _from_bit);


/* Accessor macros */

#define AM_LATCH8_READ(_tag) \
	AM_DEVREAD(_tag, latch8_device, read)

#define AM_LATCH8_READBIT(_tag, _bit) \
	AM_DEVREAD(_tag, latch8_device, bit ## _bit ## _q_r)

#define AM_LATCH8_WRITE(_tag) \
	AM_DEVWRITE(_tag, latch8_device, write)

#define AM_LATCH8_READWRITE(_tag) \
	AM_DEVREADWRITE(_tag, latch8_device, read, write)

#endif /* __LATCH8_H_ */
