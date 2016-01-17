// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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

static ADDRESS_MAP_START( huc6272_vram, AS_0, 32, huc6272_device )
	AM_RANGE(0x000000, 0x0fffff) AM_RAM
	AM_RANGE(0x100000, 0x1fffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *huc6272_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_dword - read a dword at the given address
//-------------------------------------------------

inline UINT32 huc6272_device::read_dword(offs_t address)
{
	return space().read_dword(address << 2);
}


//-------------------------------------------------
//  write_dword - write a dword at the given address
//-------------------------------------------------

inline void huc6272_device::write_dword(offs_t address, UINT32 data)
{
	space().write_dword(address << 2, data);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  huc6272_device - constructor
//-------------------------------------------------

huc6272_device::huc6272_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, huc6272, "huc6272", tag, owner, clock, "huc6272", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("videoram", ENDIANNESS_LITTLE, 32, 32, 0, nullptr, *ADDRESS_MAP_NAME(huc6272_vram))
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
	{
		/*
		xxxx xxxx ---- ---- ---- ---- ---- ---- Sub Channel Buffer
		---- ---- x--- ---- ---- ---- ---- ---- SCSI RST flag
		---- ---- -x-- ---- ---- ---- ---- ---- SCSI BUSY flag
		---- ---- --x- ---- ---- ---- ---- ---- SCSI REQ flag
		---- ---- ---x ---- ---- ---- ---- ---- SCSI MSG flag
		---- ---- ---- x--- ---- ---- ---- ---- SCSI CD flag
		---- ---- ---- -x-- ---- ---- ---- ---- SCSI IO flag
		---- ---- ---- --x- ---- ---- ---- ---- SCSI SEL flag
		---- ---- ---- ---- -x-- ---- ---- ---- SCSI IRQ pending
		---- ---- ---- ---- --x- ---- ---- ---- DMA IRQ pending
		---- ---- ---- ---- ---x ---- ---- ---- CD Sub Channel IRQ pending
		---- ---- ---- ---- ---- x--- ---- ---- Raster IRQ pending
		---- ---- ---- ---- ---- -x-- ---- ---- ADPCM IRQ pending
		---- ---- ---- ---- ---- ---- -xxx xxxx register read-back
		*/
		res = m_register & 0x7f;
		res |= (0) << 16;
	}
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
				res = read_dword((m_kram_addr_r)|(m_kram_page_r<<18));
				m_kram_addr_r += (m_kram_inc_r & 0x100) ? ((m_kram_inc_r & 0xff) - 0x100) : (m_kram_inc_r & 0xff);
				break;

			case 0x0f:
				res = m_page_setting;
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
			case 0x09: // DMA addr
				//printf("%08x DMA ADDR\n",data);
				break;
			case 0x0a: // DMA size
				//printf("%08x DMA SIZE\n",data);
				break;
			case 0x0b: // DMA status
				//printf("%08x DMA STATUS\n",data);
				break;
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
				write_dword((m_kram_addr_w)|(m_kram_page_w<<18),data); /* TODO: there are some 32-bits accesses during BIOS? */
				m_kram_addr_w += (m_kram_inc_w & 0x100) ? ((m_kram_inc_w & 0xff) - 0x100) : (m_kram_inc_w & 0xff);
				break;

			/*
			---x ---- ---- ---- ADPCM page setting
			---- ---x ---- ---- RAINBOW page setting
			---- ---- ---x ---- BG page setting
			---- ---- ---- ---x SCSI page setting
			*/
			case 0x0f:
				m_page_setting = data;
				break;

			//
			// xxxx ---- ---- ---- BG3 mode setting
			// ---- xxxx ---- ---- BG2 mode setting
			// ---- ---- xxxx ---- BG1 mode setting
			// ---- ---- ---- xxxx BG0 mode setting
			//
			// 0001 - 4 color palette
			// 0010 - 16 color palette
			// 0011 - 256 color palette
			// 0100 - 64k color
			// 0101 - 16M color
			// 1001 - 4 color palette block mode
			// 1010 - 16 color palette block mode
			// 1011 - 256 color palette block mode
			// others - unused/invalid
			case 0x10:
				m_bgmode[0] = data & 0x0f;
				m_bgmode[1] = ( data >> 4 ) & 0x0f;
				m_bgmode[2] = ( data >> 8 ) & 0x0f;
				m_bgmode[3] = ( data >> 12 ) & 0x0f;
				break;

			case 0x13:
				m_micro_prg.addr = data & 0xf;
				break;

			case 0x14:
				m_micro_prg.data[m_micro_prg.addr] = data & 0xffff;
				m_micro_prg.addr++;
				m_micro_prg.addr &= 0xf;
				break;

			case 0x15:
				m_micro_prg.ctrl = data & 1;
				break;

			//default: printf("%04x %04x %08x\n",m_register,data,mem_mask);
		}
	}
}
