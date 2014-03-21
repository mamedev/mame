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
	latch8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	
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
	
	static void set_discrete_node_0(device_t &device, const char *tag, UINT32 node) { downcast<latch8_device &>(device).m_node_device_0.set_tag(tag); downcast<latch8_device &>(device).m_node_map[0] = node;}
	static void set_discrete_node_1(device_t &device, const char *tag, UINT32 node) { downcast<latch8_device &>(device).m_node_device_1.set_tag(tag); downcast<latch8_device &>(device).m_node_map[1] = node;}
	static void set_discrete_node_2(device_t &device, const char *tag, UINT32 node) { downcast<latch8_device &>(device).m_node_device_2.set_tag(tag); downcast<latch8_device &>(device).m_node_map[2] = node;}
	static void set_discrete_node_3(device_t &device, const char *tag, UINT32 node) { downcast<latch8_device &>(device).m_node_device_3.set_tag(tag); downcast<latch8_device &>(device).m_node_map[3] = node;}
	static void set_discrete_node_4(device_t &device, const char *tag, UINT32 node) { downcast<latch8_device &>(device).m_node_device_4.set_tag(tag); downcast<latch8_device &>(device).m_node_map[4] = node;}
	static void set_discrete_node_5(device_t &device, const char *tag, UINT32 node) { downcast<latch8_device &>(device).m_node_device_5.set_tag(tag); downcast<latch8_device &>(device).m_node_map[5] = node;}
	static void set_discrete_node_6(device_t &device, const char *tag, UINT32 node) { downcast<latch8_device &>(device).m_node_device_6.set_tag(tag); downcast<latch8_device &>(device).m_node_map[6] = node;}
	static void set_discrete_node_7(device_t &device, const char *tag, UINT32 node) { downcast<latch8_device &>(device).m_node_device_7.set_tag(tag); downcast<latch8_device &>(device).m_node_map[7] = node;}
	
	template<class _Object> static devcb2_base &set_devread_0(device_t &device, _Object object, int from_bit) { downcast<latch8_device &>(device).m_from_bit[0] = from_bit; return downcast<latch8_device &>(device).m_devread_0.set_callback(object); }
	template<class _Object> static devcb2_base &set_devread_1(device_t &device, _Object object, int from_bit) { downcast<latch8_device &>(device).m_from_bit[1] = from_bit; return downcast<latch8_device &>(device).m_devread_1.set_callback(object); }
	template<class _Object> static devcb2_base &set_devread_2(device_t &device, _Object object, int from_bit) { downcast<latch8_device &>(device).m_from_bit[2] = from_bit; return downcast<latch8_device &>(device).m_devread_2.set_callback(object); }
	template<class _Object> static devcb2_base &set_devread_3(device_t &device, _Object object, int from_bit) { downcast<latch8_device &>(device).m_from_bit[3] = from_bit; return downcast<latch8_device &>(device).m_devread_3.set_callback(object); }
	template<class _Object> static devcb2_base &set_devread_4(device_t &device, _Object object, int from_bit) { downcast<latch8_device &>(device).m_from_bit[4] = from_bit; return downcast<latch8_device &>(device).m_devread_4.set_callback(object); }
	template<class _Object> static devcb2_base &set_devread_5(device_t &device, _Object object, int from_bit) { downcast<latch8_device &>(device).m_from_bit[5] = from_bit; return downcast<latch8_device &>(device).m_devread_5.set_callback(object); }
	template<class _Object> static devcb2_base &set_devread_6(device_t &device, _Object object, int from_bit) { downcast<latch8_device &>(device).m_from_bit[6] = from_bit; return downcast<latch8_device &>(device).m_devread_6.set_callback(object); }
	template<class _Object> static devcb2_base &set_devread_7(device_t &device, _Object object, int from_bit) { downcast<latch8_device &>(device).m_from_bit[7] = from_bit; return downcast<latch8_device &>(device).m_devread_7.set_callback(object); }
	
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	
	TIMER_CALLBACK_MEMBER( timerproc );
	void update(UINT8 new_val, UINT8 mask);
	inline UINT8 bitx_r( offs_t offset, int bit);
	inline void bitx_w(int bit, offs_t offset, UINT8 data);
private:
	// internal state
	UINT8            m_value;
	UINT8            m_has_node_map;
	UINT8            m_has_devread;
	
	/* only for byte reads, does not affect bit reads and node_map */
	UINT32           m_maskout;
	UINT32           m_xorvalue;  /* after mask */
	UINT32           m_nosync;
	UINT32           m_node_map[8];
	
	optional_device<discrete_device> m_node_device_0;
	optional_device<discrete_device> m_node_device_1;
	optional_device<discrete_device> m_node_device_2;
	optional_device<discrete_device> m_node_device_3;
	optional_device<discrete_device> m_node_device_4;
	optional_device<discrete_device> m_node_device_5;
	optional_device<discrete_device> m_node_device_6;
	optional_device<discrete_device> m_node_device_7;
	
	devcb2_read8     m_devread_0;
	devcb2_read8     m_devread_1;
	devcb2_read8     m_devread_2;
	devcb2_read8     m_devread_3;
	devcb2_read8     m_devread_4;
	devcb2_read8     m_devread_5;
	devcb2_read8     m_devread_6;
	devcb2_read8     m_devread_7;
	
	UINT32           m_from_bit[8];	
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
#define MCFG_LATCH8_DISCRETE_NODE_0(_tag, _node) \
	latch8_device::set_discrete_node_0(*device, "^" _tag, _node);

#define MCFG_LATCH8_DISCRETE_NODE_1(_tag, _node) \
	latch8_device::set_discrete_node_1(*device, "^" _tag, _node);

#define MCFG_LATCH8_DISCRETE_NODE_2(_tag, _node) \
	latch8_device::set_discrete_node_2(*device, "^" _tag, _node);

#define MCFG_LATCH8_DISCRETE_NODE_3(_tag, _node) \
	latch8_device::set_discrete_node_3(*device, "^" _tag, _node);

#define MCFG_LATCH8_DISCRETE_NODE_4(_tag, _node) \
	latch8_device::set_discrete_node_4(*device, "^" _tag, _node);

#define MCFG_LATCH8_DISCRETE_NODE_5(_tag, _node) \
	latch8_device::set_discrete_node_5(*device, "^" _tag, _node);

#define MCFG_LATCH8_DISCRETE_NODE_6(_tag, _node) \
	latch8_device::set_discrete_node_6(*device, "^" _tag, _node);

#define MCFG_LATCH8_DISCRETE_NODE_7(_tag, _node) \
	latch8_device::set_discrete_node_7(*device, "^" _tag, _node);

/* Upon read, replace bits by reading from another device handler */
#define MCFG_LATCH8_DEVREAD_0(_devcb, _from_bit) \
	devcb = &latch8_device::set_devread_0(*device, DEVCB2_##_devcb, _from_bit);
	
#define MCFG_LATCH8_DEVREAD_1(_devcb, _from_bit) \
	devcb = &latch8_device::set_devread_1(*device, DEVCB2_##_devcb, _from_bit);

#define MCFG_LATCH8_DEVREAD_2(_devcb, _from_bit) \
	devcb = &latch8_device::set_devread_2(*device, DEVCB2_##_devcb, _from_bit);

#define MCFG_LATCH8_DEVREAD_3(_devcb, _from_bit) \
	devcb = &latch8_device::set_devread_3(*device, DEVCB2_##_devcb, _from_bit);

#define MCFG_LATCH8_DEVREAD_4(_devcb, _from_bit) \
	devcb = &latch8_device::set_devread_4(*device, DEVCB2_##_devcb, _from_bit);

#define MCFG_LATCH8_DEVREAD_5(_devcb, _from_bit) \
	devcb = &latch8_device::set_devread_5(*device, DEVCB2_##_devcb, _from_bit);

#define MCFG_LATCH8_DEVREAD_6(_devcb, _from_bit) \
	devcb = &latch8_device::set_devread_6(*device, DEVCB2_##_devcb, _from_bit);

#define MCFG_LATCH8_DEVREAD_7(_devcb, _from_bit) \
	devcb = &latch8_device::set_devread_7(*device, DEVCB2_##_devcb, _from_bit);


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
