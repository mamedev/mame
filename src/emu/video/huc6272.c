/***************************************************************************

    Hudson/NEC HuC6272 "King" device

***************************************************************************/

#include "emu.h"
#include "video/huc6272.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type huc6272 = &device_creator<huc6272_device>;

static ADDRESS_MAP_START( huc6272_vram, AS_0, 16, huc6272_device )
	AM_RANGE(0x000000, 0x0fffff) AM_RAM
	AM_RANGE(0x100000, 0x1fffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *huc6272_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_dword - read a dword at the given address
//-------------------------------------------------

inline UINT32 huc6272_device::read_word(offs_t address)
{
	return space()->read_word(address << 1);
}


//-------------------------------------------------
//  write_dword - write a dword at the given address
//-------------------------------------------------

inline void huc6272_device::write_word(offs_t address, UINT32 data)
{
	space()->write_word(address << 1, data);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  huc6272_device - constructor
//-------------------------------------------------

huc6272_device::huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, huc6272, "huc6272", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  m_space_config("videoram", ENDIANNESS_LITTLE, 16, 32, 0, NULL, *ADDRESS_MAP_NAME(huc6272_vram))
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void huc6272_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void huc6272_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void huc6272_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ32_MEMBER( huc6272_device::read )
{
	UINT32 res = 0;

	if((offset & 1) == 0)
		res = m_register & 0x7f;
	else
	{
		switch(m_register)
		{
			/*
			x--- ---- ---- ---- ----
			*/
			case 0x0c: // KRAM load address
				res = (m_kram_addr_r & 0x3ffff) | ((m_kram_inc_r & 0x1ff) << 18) | ((m_kram_page_r & 1) << 31);
				break;

			case 0x0d: // KRAM write address
				res = (m_kram_addr_w & 0x3ffff) | ((m_kram_inc_w & 0x1ff) << 18) | ((m_kram_page_w & 1) << 31);
				break;

			case 0x0e:
				res = read_word((m_kram_addr_r)|(m_kram_page_r<<18));
				m_kram_addr_r += (m_kram_inc_r & 0x100) ? ((m_kram_inc_r & 0xff) - 0x100) : (m_kram_inc_r & 0xff);
				break;
			//default: printf("%04x\n",m_register);
		}
	}

	return res;
}

WRITE32_MEMBER( huc6272_device::write )
{
	if((offset & 1) == 0)
		m_register = data & 0x7f;
	else
	{
		switch(m_register)
		{
			/*
			---- ---- ---- ---- ----
			*/
			case 0x0c: // KRAM load address
				m_kram_addr_r = (data & 0x0003ffff);
				m_kram_inc_r =  (data & 0x07fc0000) >> 18;
				m_kram_page_r = (data & 0x80000000) >> 31;
				break;

			case 0x0d: // KRAM write address
				m_kram_addr_w = (data & 0x0003ffff);
				m_kram_inc_w =  (data & 0x07fc0000) >> 18;
				m_kram_page_w = (data & 0x80000000) >> 31;
				break;

			case 0x0e: // KRAM write VRAM
				write_word((m_kram_addr_w)|(m_kram_page_w<<18),data & 0xffff); /* TODO: there are some 32-bits accesses during BIOS? */
				m_kram_addr_w += (m_kram_inc_w & 0x100) ? ((m_kram_inc_w & 0xff) - 0x100) : (m_kram_inc_w & 0xff);
				break;

			//default: printf("%04x %04x\n",m_register,data);
		}
	}
}
