/***************************************************************************

	Seibu COP protection device

	(this header needs expanding)

***************************************************************************/

#include "emu.h"
#include "machine/seibu_cop.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SEIBU_COP = &device_creator<seibu_cop_device>;

#if 0
static ADDRESS_MAP_START( seibu_cop_vram, AS_0, 16, seibu_cop_device )
ADDRESS_MAP_END
#endif

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  seibu_cop_device - constructor
//-------------------------------------------------

seibu_cop_device::seibu_cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEIBU_COP, "seibu_cop", tag, owner, clock)
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


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_MEMBER( seibu_cop_device::read )
{
	return 0;
}

WRITE16_MEMBER( seibu_cop_device::write )
{
	switch(offset)
	{
		case 0x028/2:
			COMBINE_DATA(&m_dma_fill_val_lo);
			m_dma_fill_val = (m_dma_fill_val_lo) | (m_dma_fill_val_hi << 16);
			break;

		case 0x02a/2:
			COMBINE_DATA(&m_dma_fill_val_hi);
			m_dma_fill_val = (m_dma_fill_val_lo) | (m_dma_fill_val_hi << 16);
			break;

		case (0x05a/2): COMBINE_DATA(&m_pal_brightness_val); break;
		case (0x05c/2): COMBINE_DATA(&m_pal_brightness_mode); break;

		case 0x074/2:
			/*
				This sets up a DMA mode of some sort
					0x0e00: grainbow, cupsoc
					0x0a00: legionna, godzilla, denjinmk
					0x0600: heatbrl
					0x1e00: zeroteam, xsedae
				raiden2 and raidendx doesn't set this up, this could indicate that this is related to the non-private buffer DMAs
				(both only uses 0x14 and 0x15 as DMAs)
			*/
			COMBINE_DATA(&m_dma_unk_param);
			break;

		case (0x076/2):
			COMBINE_DATA(&m_cop_dma_fade_table);
			break;

		case (0x078/2): /* DMA source address */
			COMBINE_DATA(&m_cop_dma_src[m_cop_dma_trigger]);
			break;

		case (0x07a/2): /* DMA length */
			COMBINE_DATA(&m_cop_dma_size[m_cop_dma_trigger]);
			break;

		case (0x07c/2): /* DMA destination */
			COMBINE_DATA(&m_cop_dma_dst[m_cop_dma_trigger]);
			break;

		case (0x07e/2): /* DMA parameter */
			COMBINE_DATA(&m_cop_dma_exec_param);
			m_cop_dma_trigger = m_cop_dma_exec_param & 7;
			break;
	}
}
