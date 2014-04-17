/*********************************************************************

    memcard.c

    Memory card functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "memcard.h"

// device type definition
const device_type MEMCARD = &device_creator<memcard_device>;

//-------------------------------------------------
//  memcard_device - constructor
//-------------------------------------------------

memcard_device::memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MEMCARD, "MEMCARD", tag, owner, clock, "memcard", __FILE__),		
	  m_memcard_inserted(-1)
{
}

//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void memcard_device::static_set_size(device_t &device, int value)
{
	memcard_device &card = downcast<memcard_device &>(device);
	card.m_size = value;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void memcard_device::device_start()
{
	/* initialize the memcard data structure */
	m_memcard_data = auto_alloc_array_clear(machine(), UINT8, m_size);	
	save_pointer(NAME(m_memcard_data), m_size);
	save_item(NAME(m_memcard_inserted));
}

//-------------------------------------------------
//  device_stop - device-specific shutdown
//-------------------------------------------------

void memcard_device::device_stop()
{
	eject();
}


/*-------------------------------------------------
    memcard_name - determine the name of a memcard
    file
-------------------------------------------------*/

INLINE void memcard_name(int index, char *buffer)
{
	sprintf(buffer, "memcard.%03d", index);
}


/*-------------------------------------------------
    memcard_create - create a new memory card with
    the given index
-------------------------------------------------*/

int memcard_device::create(int index, int overwrite)
{
	char name[16];

	/* create a name */
	memcard_name(index, name);

	/* if we can't overwrite, fail if the file already exists */
	astring fname(machine().basename(), PATH_SEPARATOR, name);
	if (!overwrite)
	{
		emu_file testfile(machine().options().memcard_directory(), OPEN_FLAG_READ);
		if (testfile.open(fname) == FILERR_NONE)
			return 1;
	}

	/* create a new file */
	emu_file file(machine().options().memcard_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open(fname);
	if (filerr != FILERR_NONE)
		return 1;

	/* initialize and then save the card */
	memset(m_memcard_data, 0, m_size);
	file.write(m_memcard_data, m_size);


	/* close the file */
	return 0;
}


/*-------------------------------------------------
    memcard_insert - insert an existing memory card
    with the given index
-------------------------------------------------*/

int memcard_device::insert(int index)
{	
	char name[16];

	/* if a card is already inserted, eject it first */
	if (m_memcard_inserted != -1)
		eject();
	assert(m_memcard_inserted == -1);

	/* create a name */
	memcard_name(index, name);

	/* open the file; if we can't, it's an error */
	emu_file file(machine().options().memcard_directory(), OPEN_FLAG_READ);
	file_error filerr = file.open(machine().basename(), PATH_SEPARATOR, name);
	if (filerr != FILERR_NONE)
		return 1;

	/* initialize and then load the card */
	file.read(m_memcard_data, m_size);

	/* close the file */
	m_memcard_inserted = index;
	return 0;
}


/*-------------------------------------------------
    memcard_eject - eject a memory card, saving
    its contents along the way
-------------------------------------------------*/

void memcard_device::eject()
{	
	char name[16];

	/* if no card is preset, just ignore */
	if (m_memcard_inserted == -1)
		return;

	/* create a name */
	memcard_name(m_memcard_inserted, name);

	/* open the file; if we can't, it's an error */
	emu_file file(machine().options().memcard_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open(machine().basename(), PATH_SEPARATOR, name);
	if (filerr != FILERR_NONE)
		return;

	/* initialize and then load the card */
	file.write(m_memcard_data, m_size);
	
	/* close the file */
	m_memcard_inserted = -1;
}


/*-------------------------------------------------
    memcard_present - return the currently loaded
    card index, or -1 if none
-------------------------------------------------*/

int memcard_device::present()
{
	return m_memcard_inserted;
}


READ8_MEMBER(memcard_device::read)
{
	return m_memcard_data[offset];
}

WRITE8_MEMBER(memcard_device::write)
{
	m_memcard_data[offset] = data;
}