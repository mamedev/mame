/*********************************************************************

    memcard.h

    Memory card functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __MEMCARD_H__
#define __MEMCARD_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MEMCARD_ADD(_tag,_size) \
	MCFG_DEVICE_ADD(_tag, MEMCARD, 0) \
	memcard_device::static_set_size(*device, _size);
	
/***************************************************************************
    CONSTANTS
***************************************************************************/

/* memory card actions */
#define MEMCARD_CREATE          0
#define MEMCARD_INSERT          1
#define MEMCARD_EJECT           2

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// ======================> memcard_device

class memcard_device :  public device_t
{
public:
	// construction/destruction
	memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	static void static_set_size(device_t &device, int value);
	
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);	

	/* create a new memory card with the given index */
	int create(int index, int overwrite);

	/* "insert" a memory card with the given index and load its data */
	int insert(int index);

	/* "eject" a memory card and save its data */
	void eject();

	/* returns the index of the current memory card, or -1 if none */
	int present();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_stop();

	// memory card status
	int m_memcard_inserted;
	UINT8 *m_memcard_data;
	int m_size;
};


// device type definition
extern const device_type MEMCARD;

// device iterator
typedef device_type_iterator<&device_creator<memcard_device>, memcard_device> memcard_device_iterator;


#endif  /* __MEMCARD_H__ */
