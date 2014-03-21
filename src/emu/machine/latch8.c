/**********************************************************************

    8 bit latch interface and emulation

    2008/08     couriersud

**********************************************************************/

#include "emu.h"
#include "latch8.h"

void latch8_device::update(UINT8 new_val, UINT8 mask)
{
	/*  temporary hack until the discrete system is a device */
	UINT8 old_val = m_value;

	m_value = (m_value & ~mask) | (new_val & mask);

	if (m_has_node_map)
	{
		int i;
		UINT8 changed = old_val ^ m_value;
		for (i=0; i<8; i++)
			if (((changed & (1<<i)) != 0) && m_node_map[i] != 0) {
				if (i==0 && m_node_device_0!=NULL) discrete_sound_w(m_node_device_0, machine().driver_data()->generic_space(), m_node_map[i] , (m_value >> i) & 1);
				if (i==1 && m_node_device_1!=NULL) discrete_sound_w(m_node_device_1, machine().driver_data()->generic_space(), m_node_map[i] , (m_value >> i) & 1);
				if (i==2 && m_node_device_2!=NULL) discrete_sound_w(m_node_device_2, machine().driver_data()->generic_space(), m_node_map[i] , (m_value >> i) & 1);
				if (i==3 && m_node_device_3!=NULL) discrete_sound_w(m_node_device_3, machine().driver_data()->generic_space(), m_node_map[i] , (m_value >> i) & 1);
				if (i==4 && m_node_device_4!=NULL) discrete_sound_w(m_node_device_4, machine().driver_data()->generic_space(), m_node_map[i] , (m_value >> i) & 1);
				if (i==5 && m_node_device_5!=NULL) discrete_sound_w(m_node_device_5, machine().driver_data()->generic_space(), m_node_map[i] , (m_value >> i) & 1);
				if (i==6 && m_node_device_6!=NULL) discrete_sound_w(m_node_device_6, machine().driver_data()->generic_space(), m_node_map[i] , (m_value >> i) & 1);
				if (i==7 && m_node_device_7!=NULL) discrete_sound_w(m_node_device_7, machine().driver_data()->generic_space(), m_node_map[i] , (m_value >> i) & 1);
			}
	}
}

TIMER_CALLBACK_MEMBER( latch8_device::timerproc )
{
	UINT8 new_val = param & 0xFF;
	UINT8 mask = param >> 8;

	update( new_val, mask);
}

/* ----------------------------------------------------------------------- */

READ8_MEMBER( latch8_device::read )
{
	UINT8 res;

	assert(offset == 0);

	res = m_value;
	if (m_has_devread)
	{
		int i;
		for (i=0; i<8; i++)
		{			
			if (i==0 && !m_devread_0.isnull()) { res &= ~( 1 << i); res |= ((m_devread_0(space, 0, 0xff) >> m_from_bit[i]) & 0x01) << i; }
			if (i==1 && !m_devread_1.isnull()) { res &= ~( 1 << i); res |= ((m_devread_1(space, 0, 0xff) >> m_from_bit[i]) & 0x01) << i; }
			if (i==2 && !m_devread_2.isnull()) { res &= ~( 1 << i); res |= ((m_devread_2(space, 0, 0xff) >> m_from_bit[i]) & 0x01) << i; }
			if (i==3 && !m_devread_3.isnull()) { res &= ~( 1 << i); res |= ((m_devread_3(space, 0, 0xff) >> m_from_bit[i]) & 0x01) << i; }
			if (i==4 && !m_devread_4.isnull()) { res &= ~( 1 << i); res |= ((m_devread_4(space, 0, 0xff) >> m_from_bit[i]) & 0x01) << i; }
			if (i==5 && !m_devread_5.isnull()) { res &= ~( 1 << i); res |= ((m_devread_5(space, 0, 0xff) >> m_from_bit[i]) & 0x01) << i; }
			if (i==6 && !m_devread_6.isnull()) { res &= ~( 1 << i); res |= ((m_devread_6(space, 0, 0xff) >> m_from_bit[i]) & 0x01) << i; }
			if (i==7 && !m_devread_7.isnull()) { res &= ~( 1 << i); res |= ((m_devread_7(space, 0, 0xff) >> m_from_bit[i]) & 0x01) << i;	}			
		}
	}
	return (res & ~m_maskout) ^ m_xorvalue;
}


WRITE8_MEMBER( latch8_device::write )
{
	assert(offset == 0);

	if (m_nosync != 0xff)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(latch8_device::timerproc),this), (0xFF << 8) | data);
	else
		update(data, 0xFF);
}


WRITE8_MEMBER( latch8_device::reset_w )
{
	assert(offset == 0);

	m_value = 0;
}

/* read bit x                 */
/* return (latch >> x) & 0x01 */

UINT8 latch8_device::bitx_r( offs_t offset, int bit)
{
	assert( offset == 0);

	return (m_value >> bit) & 0x01;
}

READ8_MEMBER( latch8_device::bit0_r) { return bitx_r(offset, 0); }
READ8_MEMBER( latch8_device::bit1_r) { return bitx_r(offset, 1); }
READ8_MEMBER( latch8_device::bit2_r) { return bitx_r(offset, 2); }
READ8_MEMBER( latch8_device::bit3_r) { return bitx_r(offset, 3); }
READ8_MEMBER( latch8_device::bit4_r) { return bitx_r(offset, 4); }
READ8_MEMBER( latch8_device::bit5_r) { return bitx_r(offset, 5); }
READ8_MEMBER( latch8_device::bit6_r) { return bitx_r(offset, 6); }
READ8_MEMBER( latch8_device::bit7_r) { return bitx_r(offset, 7); }

READ8_MEMBER( latch8_device::bit0_q_r) { return bitx_r(offset, 0) ^ 1; }
READ8_MEMBER( latch8_device::bit1_q_r) { return bitx_r(offset, 1) ^ 1; }
READ8_MEMBER( latch8_device::bit2_q_r) { return bitx_r(offset, 2) ^ 1; }
READ8_MEMBER( latch8_device::bit3_q_r) { return bitx_r(offset, 3) ^ 1; }
READ8_MEMBER( latch8_device::bit4_q_r) { return bitx_r(offset, 4) ^ 1; }
READ8_MEMBER( latch8_device::bit5_q_r) { return bitx_r(offset, 5) ^ 1; }
READ8_MEMBER( latch8_device::bit6_q_r) { return bitx_r(offset, 6) ^ 1; }
READ8_MEMBER( latch8_device::bit7_q_r) { return bitx_r(offset, 7) ^ 1; }

/* write bit x from data into bit determined by offset */
/* latch = (latch & ~(1<<offset)) | (((data >> x) & 0x01) << offset) */

void latch8_device::bitx_w(int bit, offs_t offset, UINT8 data)
{
	UINT8 mask = (1<<offset);
	UINT8 masked_data = (((data >> bit) & 0x01) << offset);

	assert( offset < 8);

	/* No need to synchronize ? */
	if (m_nosync & mask)
		update(masked_data, mask);
	else
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(latch8_device::timerproc),this), (mask << 8) | masked_data);
}

WRITE8_MEMBER( latch8_device::bit0_w ) { bitx_w(0, offset, data); }
WRITE8_MEMBER( latch8_device::bit1_w ) { bitx_w(1, offset, data); }
WRITE8_MEMBER( latch8_device::bit2_w ) { bitx_w(2, offset, data); }
WRITE8_MEMBER( latch8_device::bit3_w ) { bitx_w(3, offset, data); }
WRITE8_MEMBER( latch8_device::bit4_w ) { bitx_w(4, offset, data); }
WRITE8_MEMBER( latch8_device::bit5_w ) { bitx_w(0, offset, data); }
WRITE8_MEMBER( latch8_device::bit6_w ) { bitx_w(0, offset, data); }
WRITE8_MEMBER( latch8_device::bit7_w ) { bitx_w(0, offset, data); }

const device_type LATCH8 = &device_creator<latch8_device>;

latch8_device::latch8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, LATCH8, "8 bit latch", tag, owner, clock, "latch8", __FILE__),
		m_value(0),
		m_has_node_map(0),
		m_has_devread(0),
		m_maskout(0),
		m_xorvalue(0),
		m_nosync(0),	
		m_node_device_0(*this),
		m_node_device_1(*this),
		m_node_device_2(*this),
		m_node_device_3(*this),
		m_node_device_4(*this),
		m_node_device_5(*this),
		m_node_device_6(*this),
		m_node_device_7(*this),
		m_devread_0(*this),
		m_devread_1(*this),
		m_devread_2(*this),
		m_devread_3(*this),
		m_devread_4(*this),
		m_devread_5(*this),
		m_devread_6(*this),
		m_devread_7(*this)
{
	memset(m_node_map, 0, sizeof(m_node_map));
	memset(m_from_bit, 0, sizeof(m_from_bit));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void latch8_device::device_start()
{
	m_devread_0.resolve();
	m_devread_1.resolve();
	m_devread_2.resolve();
	m_devread_3.resolve();
	m_devread_4.resolve();
	m_devread_5.resolve();
	m_devread_6.resolve();
	m_devread_7.resolve();
	
	/* setup nodemap */
	for (int i=0; i<8; i++)
		if (m_node_map[i] )
		{
			if (i==0 && m_node_device_0==NULL) fatalerror("Device %s: Bit %d has invalid discrete device\n", tag(), i);
			if (i==1 && m_node_device_1==NULL) fatalerror("Device %s: Bit %d has invalid discrete device\n", tag(), i);
			if (i==2 && m_node_device_2==NULL) fatalerror("Device %s: Bit %d has invalid discrete device\n", tag(), i);
			if (i==3 && m_node_device_3==NULL) fatalerror("Device %s: Bit %d has invalid discrete device\n", tag(), i);
			if (i==4 && m_node_device_4==NULL) fatalerror("Device %s: Bit %d has invalid discrete device\n", tag(), i);
			if (i==5 && m_node_device_5==NULL) fatalerror("Device %s: Bit %d has invalid discrete device\n", tag(), i);
			if (i==6 && m_node_device_6==NULL) fatalerror("Device %s: Bit %d has invalid discrete device\n", tag(), i);
			if (i==7 && m_node_device_7==NULL) fatalerror("Device %s: Bit %d has invalid discrete device\n", tag(), i);

			m_has_node_map = 1;
		}

	/* setup device read handlers */
	if (!m_devread_0.isnull()) m_has_devread = 1;
	if (!m_devread_1.isnull()) m_has_devread = 1;
	if (!m_devread_2.isnull()) m_has_devread = 1;
	if (!m_devread_3.isnull()) m_has_devread = 1;
	if (!m_devread_4.isnull()) m_has_devread = 1;
	if (!m_devread_5.isnull()) m_has_devread = 1;
	if (!m_devread_6.isnull()) m_has_devread = 1;
	if (!m_devread_7.isnull()) m_has_devread = 1;

	for (int i=0; i<8; i++) 
	{
		if (!m_devread_0.isnull() && m_node_device_0!=NULL) fatalerror("Device %s: Bit %d already has a handler.\n", tag(), i);
		if (!m_devread_1.isnull() && m_node_device_1!=NULL) fatalerror("Device %s: Bit %d already has a handler.\n", tag(), i);
		if (!m_devread_2.isnull() && m_node_device_2!=NULL) fatalerror("Device %s: Bit %d already has a handler.\n", tag(), i);
		if (!m_devread_3.isnull() && m_node_device_3!=NULL) fatalerror("Device %s: Bit %d already has a handler.\n", tag(), i);
		if (!m_devread_4.isnull() && m_node_device_4!=NULL) fatalerror("Device %s: Bit %d already has a handler.\n", tag(), i);
		if (!m_devread_5.isnull() && m_node_device_5!=NULL) fatalerror("Device %s: Bit %d already has a handler.\n", tag(), i);
		if (!m_devread_6.isnull() && m_node_device_6!=NULL) fatalerror("Device %s: Bit %d already has a handler.\n", tag(), i);
		if (!m_devread_7.isnull() && m_node_device_7!=NULL) fatalerror("Device %s: Bit %d already has a handler.\n", tag(), i);
	}	

	save_item(NAME(m_value));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void latch8_device::device_reset()
{
	m_value = 0;
}
