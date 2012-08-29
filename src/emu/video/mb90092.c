/***************************************************************************

	Fujitsu MB90092 OSD

	preliminary device by Angelo Salese

	TODO:
	- get a real charset ROM;

***************************************************************************/

#include "emu.h"
#include "video/mb90092.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type MB90092 = &device_creator<mb90092_device>;

static ADDRESS_MAP_START( mb90092_vram, AS_0, 16, mb90092_device )
	AM_RANGE(0x0000, 0x023f) AM_RAM // vram
ADDRESS_MAP_END

/* charset is undumped, but apparently a normal ASCII one is enough for the time being (for example "fnt0808.x1" in Sharp X1) */
ROM_START( mb90092 )
	ROM_REGION( 0x0800, "mb90092", 0 )
	ROM_LOAD("mb90092_char.bin",     0x0000, 0x0800, NO_DUMP )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *mb90092_device::device_rom_region() const
{
	return ROM_NAME( mb90092 );
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *mb90092_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT16 mb90092_device::read_word(offs_t address)
{
	return space()->read_word(address << 1);
}

//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void mb90092_device::write_word(offs_t address, UINT16 data)
{
	space()->write_word(address << 1, data);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb90092_device - constructor
//-------------------------------------------------

mb90092_device::mb90092_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MB90092, "mb90092", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  m_space_config("videoram", ENDIANNESS_LITTLE, 16, 16, 0, NULL, *ADDRESS_MAP_NAME(mb90092_vram))
{
	m_shortname = "mb90092";

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void mb90092_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb90092_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb90092_device::device_reset()
{
	int i;
	m_cmd_ff = 0;

	for(i=0;i<0x120;i++)
		write_word(i,0x00ff);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_MEMBER( mb90092_device::write )
{
	UINT16 dat;

	switch(m_cmd_ff)
	{
		case OSD_COMMAND:
			m_cmd = data & 0xf8;
			m_cmd_param = data & 7;
			//printf("cmd %02x\n",data);
			break;
		case OSD_DATA:
			dat = ((m_cmd_param & 7)<<7) | (data & 0x7f);
			switch(m_cmd)
			{
				case 0x80: // preset VRAM address
					m_osd_addr = dat;
					//printf("%04x %d %d\n",m_osd_addr,(m_osd_addr & 0x1f),(m_osd_addr & 0x1e0) >> 5);
					break;
				case 0x90: // write Character
					int x,y;
					x = (m_osd_addr & 0x1f);
					y = (m_osd_addr & 0x1e0) >> 5;
					//printf("%d %d\n",x,y);
					write_word(x+y*24,dat);

					/* handle address increments */
					x = ((x + 1) % 24);
					if(x == 0)
						y = ((y + 1) % 12);
					m_osd_addr = x + (y << 5);

					break;

			}
			break;
	}

	m_cmd_ff ^= 1;
}

UINT32 mb90092_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	UINT8 *pcg = memregion("mb90092")->base();
	UINT16 tile;

	for(y=0;y<12;y++)
	{
		for(x=0;x<24;x++)
		{
			int xi,yi;

			tile = read_word(x+y*24);

			/* TODO: charset hook-up is obviously WRONG so following mustn't be trusted at all */
			for(yi=0;yi<16;yi++)
			{
				for(xi=0;xi<16;xi++)
				{
					UINT8 pix;
					UINT32 pen;

					pix = (pcg[(tile*8)+(yi >> 1)] >> (7-(xi >> 1))) & 1;

					pen = pix ? 0xffffff : 0;
					if(tile == 0xff)
						pen = 0;

					bitmap.pix32(y*16+yi,x*16+xi) = pen;
				}
			}
		}
	}

	return 0;
}
