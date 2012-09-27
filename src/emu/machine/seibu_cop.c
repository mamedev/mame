/***************************************************************************

    Seibu COP protection device

    (this header needs expanding!)

***************************************************************************/

#include "emu.h"
#include "machine/seibu_cop.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SEIBU_COP = &device_creator<seibu_cop_device>;


static ADDRESS_MAP_START( seibu_cop_io, AS_0, 16, seibu_cop_device )
	AM_RANGE(0x0428, 0x0429) AM_WRITE(dma_fill_val_lo_w)
	AM_RANGE(0x042a, 0x042b) AM_WRITE(dma_fill_val_hi_w)

	AM_RANGE(0x045a, 0x045b) AM_WRITE(pal_brightness_val_w)
	AM_RANGE(0x045c, 0x045d) AM_WRITE(pal_brightness_mode_w)

	AM_RANGE(0x0474, 0x0475) AM_WRITE(dma_unk_param_w)
	AM_RANGE(0x0476, 0x0477) AM_WRITE(dma_pal_fade_table_w)
	AM_RANGE(0x0478, 0x0479) AM_WRITE(dma_src_w)
	AM_RANGE(0x047a, 0x047b) AM_WRITE(dma_size_w)
	AM_RANGE(0x047c, 0x047d) AM_WRITE(dma_dst_w)
	AM_RANGE(0x047e, 0x047f) AM_WRITE(dma_trigger_w)
ADDRESS_MAP_END


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT16 seibu_cop_device::read_word(offs_t address)
{
	return space().read_word(address);
}

//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void seibu_cop_device::write_word(offs_t address, UINT16 data)
{
	space().write_word(address, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  seibu_cop_device - constructor
//-------------------------------------------------

seibu_cop_device::seibu_cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEIBU_COP, "seibu_cop", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  m_space_config("io", ENDIANNESS_LITTLE, 16, 16, 0, NULL, *ADDRESS_MAP_NAME(seibu_cop_io))
{

}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void seibu_cop_device::device_config_complete()
{
	// inherit a copy of the static data
	const seibu_cop_interface *intf = reinterpret_cast<const seibu_cop_interface *>(static_config());
	if (intf != NULL)
		*static_cast<seibu_cop_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_mreq_cb, 0, sizeof(m_in_mreq_cb));
		memset(&m_out_mreq_cb, 0, sizeof(m_out_mreq_cb));
	}
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void seibu_cop_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void seibu_cop_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void seibu_cop_device::device_reset()
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *seibu_cop_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE16_MEMBER(seibu_cop_device::dma_fill_val_lo_w)
{
	COMBINE_DATA(&m_dma_fill_val_lo);
	m_dma_fill_val = (m_dma_fill_val_lo) | (m_dma_fill_val_hi << 16);
}

WRITE16_MEMBER(seibu_cop_device::dma_fill_val_hi_w)
{
	COMBINE_DATA(&m_dma_fill_val_hi);
	m_dma_fill_val = (m_dma_fill_val_lo) | (m_dma_fill_val_hi << 16);
}

WRITE16_MEMBER(seibu_cop_device::pal_brightness_val_w)
{
	COMBINE_DATA(&m_pal_brightness_val);

	/* TODO: add checks for bits 15-6 */
}

WRITE16_MEMBER(seibu_cop_device::pal_brightness_mode_w)
{
	COMBINE_DATA(&m_pal_brightness_mode);

	/* TODO: add checks for anything that isn't 4 or 5 */
}

WRITE16_MEMBER(seibu_cop_device::dma_unk_param_w)
{
	/*
		This sets up a DMA mode of some sort
			0x0e00: grainbow, cupsoc
			0x0a00: legionna, godzilla, denjinmk
			0x0600: heatbrl
			0x1e00: zeroteam, xsedae
		raiden2 and raidendx doesn't set this up, this could indicate that this is related to the non-private buffer DMAs
	    (both only uses 0x14 and 0x15 as DMAs afaik)
    */
	COMBINE_DATA(&m_dma_unk_param);
}

WRITE16_MEMBER(seibu_cop_device::dma_pal_fade_table_w)
{
	COMBINE_DATA(&m_dma_pal_fade_table);
}

WRITE16_MEMBER(seibu_cop_device::dma_src_w)
{
	COMBINE_DATA(&m_dma_src[m_dma_trigger]);
}

WRITE16_MEMBER(seibu_cop_device::dma_size_w)
{
	COMBINE_DATA(&m_dma_size[m_dma_trigger]);
}

WRITE16_MEMBER(seibu_cop_device::dma_dst_w)
{
	COMBINE_DATA(&m_dma_dst[m_dma_trigger]);
}

WRITE16_MEMBER(seibu_cop_device::dma_trigger_w)
{
	COMBINE_DATA(&m_dma_exec_param);
	m_dma_trigger = m_dma_exec_param & 7;
}

READ16_MEMBER( seibu_cop_device::read )
{
	return read_word(offset + (0x400/2));
}

WRITE16_MEMBER( seibu_cop_device::write )
{
	write_word(offset + (0x400/2),data);
}
