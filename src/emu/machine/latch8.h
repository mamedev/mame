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

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _latch8_devread latch8_devread;
struct _latch8_devread
{
	/* only for byte reads, does not affect bit reads and node_map */
	UINT32					from_bit;
	const char				*tag;
	read8_device_func		devread_handler;
	read8_space_func		read_handler;
};

typedef struct _latch8_config latch8_config;
struct _latch8_config
{
	/* only for byte reads, does not affect bit reads and node_map */
	UINT32					maskout;
	UINT32					xorvalue;  /* after mask */
	UINT32					nosync;
	UINT32					node_map[8];
	const char *			node_device[8];
	latch8_devread			devread[8];
};


DEVICE_GET_INFO( latch8 );

class latch8_device : public legacy_device_base
{
public:
       latch8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock);
	   
	   latch8_config m_inline_config;
	   
	   void set_maskout(UINT32 maskout) { m_inline_config.maskout = maskout; } 
	   void set_xorvalue(UINT32 xorvalue) { m_inline_config.xorvalue = xorvalue; } 
	   void set_nosync(UINT32 nosync) { m_inline_config.nosync = nosync; } 
	   
	   void set_discrete_node(const char *dev_tag, int bit, UINT32 node) { m_inline_config.node_device[bit] = dev_tag;  m_inline_config.node_map[bit] = node;  } 
	   void set_devread(int bit, const char *tag, read8_device_func handler, int from_bit) 
	   {
			m_inline_config.devread[bit].from_bit = from_bit; 
			m_inline_config.devread[bit].tag = tag; 
			m_inline_config.devread[bit].devread_handler = handler;			
	   }
	   void set_read(int bit, read8_space_func handler, int from_bit) 
	   {
			m_inline_config.devread[bit].from_bit = from_bit; 
			m_inline_config.devread[bit].read_handler = handler;			
	   }		
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
	static_cast<latch8_device *>(device)->set_maskout(_maskout);	

/* Bit mask specifying bits to be inverted */
#define MCFG_LATCH8_INVERT(_xor) \
	static_cast<latch8_device *>(device)->set_xorvalue(_xor);

/* Bit mask specifying bits not needing cpu synchronization. */
#define MCFG_LATCH8_NOSYNC(_nosync) \
	static_cast<latch8_device *>(device)->set_nosync(_nosync);

/* Write bit to discrete node */
#define MCFG_LATCH8_DISCRETE_NODE(_device, _bit, _node) \
	static_cast<latch8_device *>(device)->set_discrete_node(_device, _bit, _node);

/* Upon read, replace bits by reading from another device handler */
#define MCFG_LATCH8_DEVREAD(_bit, _tag, _handler, _from_bit) \
	static_cast<latch8_device *>(device)->set_devread(_bit, _tag, _handler, _from_bit);

/* Upon read, replace bits by reading from another machine handler */
#define MCFG_LATCH8_READ(_bit, _handler, _from_bit) \
	static_cast<latch8_device *>(device)->set_read(_bit, _handler, _from_bit);

/* Accessor macros */

#define AM_LATCH8_READ(_tag) \
	AM_DEVREAD_LEGACY(_tag, latch8_r)

#define AM_LATCH8_READBIT(_tag, _bit) \
	AM_DEVREAD_LEGACY(_tag, latch8_bit ## _bit ## _q_r)

#define AM_LATCH8_WRITE(_tag) \
	AM_DEVWRITE_LEGACY(_tag, latch8_w)

#define AM_LATCH8_READWRITE(_tag) \
	AM_DEVREADWRITE_LEGACY(_tag, latch8_r, latch8_w)

/* write & read full byte */

READ8_DEVICE_HANDLER( latch8_r );
WRITE8_DEVICE_HANDLER( latch8_w );

/* reset the latch */

WRITE8_DEVICE_HANDLER( latch8_reset );

/* read bit x                 */
/* return (latch >> x) & 0x01 */

READ8_DEVICE_HANDLER( latch8_bit0_r );
READ8_DEVICE_HANDLER( latch8_bit1_r );
READ8_DEVICE_HANDLER( latch8_bit2_r );
READ8_DEVICE_HANDLER( latch8_bit3_r );
READ8_DEVICE_HANDLER( latch8_bit4_r );
READ8_DEVICE_HANDLER( latch8_bit5_r );
READ8_DEVICE_HANDLER( latch8_bit6_r );
READ8_DEVICE_HANDLER( latch8_bit7_r );

/* read inverted bit x        */
/* return (latch >> x) & 0x01 */

READ8_DEVICE_HANDLER( latch8_bit0_q_r );
READ8_DEVICE_HANDLER( latch8_bit1_q_r );
READ8_DEVICE_HANDLER( latch8_bit2_q_r );
READ8_DEVICE_HANDLER( latch8_bit3_q_r );
READ8_DEVICE_HANDLER( latch8_bit4_q_r );
READ8_DEVICE_HANDLER( latch8_bit5_q_r );
READ8_DEVICE_HANDLER( latch8_bit6_q_r );
READ8_DEVICE_HANDLER( latch8_bit7_q_r );

/* write bit x from data into bit determined by offset */
/* latch = (latch & ~(1<<offset)) | (((data >> x) & 0x01) << offset) */

WRITE8_DEVICE_HANDLER( latch8_bit0_w );
WRITE8_DEVICE_HANDLER( latch8_bit1_w );
WRITE8_DEVICE_HANDLER( latch8_bit2_w );
WRITE8_DEVICE_HANDLER( latch8_bit3_w );
WRITE8_DEVICE_HANDLER( latch8_bit4_w );
WRITE8_DEVICE_HANDLER( latch8_bit5_w );
WRITE8_DEVICE_HANDLER( latch8_bit6_w );
WRITE8_DEVICE_HANDLER( latch8_bit7_w );

#endif /* __LATCH8_H_ */
