// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SNUG HSGPL card emulation.

    The HSGPL card is a card for the peripheral expansion box which simulates
    16 banks of 64 KiB GROM space, using flash memory to keep data persistent.

    Main usage:
    - store cartridges in the card for quick selection
    - replace the console GROMs, thus making it possible to patch or enhance the
      console operating system
    - provide necessary ROMs for the SGCPU (aka TI-99/4P)

    (So for using the ti99_4p emulation, you MUST have a properly set up HSGPL
    card. The best way to get this done is to prepare this card within the
    ti99_4ev emulation, then copy the contents to the ti99_4p nvram directory.)

    The card does not contain real GROM circuits, and accordingly, we do not
    use the grom emulation. The logic on the board, mainly contained in the
    MACH chip, simulates GROM behavior. With the possibility to upload data
    into the simulated GROMs, we can also enjoy GRAM behavior, although real
    GRAMs are not known to have become available ever.

    Contents:

    Supports 16 banks of 8 GROMs (8kbytes each) with 16 associated banks of
    32kbytes (8kbytes*4) of module ROM, 2 banks of 8 GRAMs with 2 associated
    banks of 32 kbytes of RAM, and 512kbytes of DSR.  Roms are implemented with
    512kbyte EEPROMs (1 for DSR, 2 for GROMs, 1 for cartridge ROM).  RAM is
    implemented with 128kbyte SRAMs (1 for GRAM, 1 for cartridge RAM - only the
    first 64kbytes of the cartridge RAM chip is used).

    CRU bits:
       Name    Equates Meaning
    >0 DEN     DSRENA  DSR Enable
    >1 GRMENA  GRMENA  Enable GRAM instead of GROM in banks 0 and 1
    >2 BNKINH* BNKENA  Disable banking
    >3 PG0     PG0
    >4 PG1     PG1
    >5 PG2     PG2     Paging-Bits for DSR-area
    >6 PG3     PG3
    >7 PG4     PG4
    >8 PG5     PG5
    >9 CRDENA  CRDENA  Activate memory areas of HSGPL (i.e. enable HSGPL GROM and ROM6 ports)
    >A WRIENA  WRIENA  write enable for RAM and GRAM (and flash GROM!)
    >B SCENA   SCARTE  Activate SuperCart-banking
    >C LEDENA  LEDENA  turn LED on
    >D -       -       free
    >E MBXENA  MBXENA  Activate MBX-Banking
    >F RAMENA  RAMENA  Enable RAM6000 instead of ROM6000 in banks 0 and 1


    Direct access ports for all memory areas (the original manual says
    >9880->989C and >9C80->9C9C for ROM6000, but this is clearly incorrect):

    Module bank Read    Write   Read ROM6000        Write ROM6000
                GROM    GROM
    0           >9800   >9C00   >9860 Offset >0000  >9C60 Offset >0000
    1           >9804   >9C04   >9860 Offset >8000  >9C60 Offset >8000
    2           >9808   >9C08   >9864 Offset >0000  >9C64 Offset >0000
    3           >980C   >9C0C   >9864 Offset >8000  >9C64 Offset >8000
    4           >9810   >9C10   >9868 Offset >0000  >9C68 Offset >0000
    5           >9814   >9C14   >9868 Offset >8000  >9C68 Offset >8000
    6           >9818   >9C18   >986C Offset >0000  >9C6C Offset >0000
    7           >981C   >9C1C   >986C Offset >8000  >9C6C Offset >8000
    8           >9820   >9C20   >9870 Offset >0000  >9C70 Offset >0000
    9           >9824   >9C24   >9870 Offset >8000  >9C70 Offset >8000
    10          >9828   >9C28   >9874 Offset >0000  >9C74 Offset >0000
    11          >982C   >9C2C   >9874 Offset >8000  >9C74 Offset >8000
    12          >9830   >9C30   >9878 Offset >0000  >9C78 Offset >0000
    13          >9834   >9C34   >9878 Offset >8000  >9C78 Offset >8000
    14          >9838   >9C38   >987C Offset >0000  >9C7C Offset >0000
    15          >983C   >9C3C   >987C Offset >8000  >9C7C Offset >8000

    Module bank Read    Write   Read RAM6000        Write RAM6000
                GRAM    GRAM
    16 (Ram)    >9880   >9C80   >98C0 Offset >0000  >9CC0 Offset >0000
    17 (Ram)    >9884   >9C84   >98C0 Offset >8000  >9CC0 Offset >8000

    DSR bank    Read    Write
    0 - 7       >9840   >9C40
    8 - 15      >9844   >9C44
    16 - 23     >9848   >9C48
    24 - 31     >984C   >9C4C
    32 - 39     >9850   >9C50
    40 - 47     >9854   >9C54
    48 - 55     >9858   >9C58
    56 - 63     >985C   >9C5C

    Notes:
    1. The bank numbering of the modules is not exactly the GROM bank numbering.
       The first 16 banks are numbered as expected, but bank 16 is DSR bank 0-7,
       bank 23 is DSR bank 56-63, bank 24 is ROM6000 of module bank 0, bank 31 is
       ROM6000 of module bank 25, and finally bank 32 and 33 and the GRAM banks,
       and bank 48 and 49 are the RAM banks.
       Only accesses to the GROM/GRAM addresses will change the module bank.

    2. Writing only works for areas set up as RAM.  To write to the
       FEEPROMs, you must used the algorithm specified by their respective
       manufacturer.

    CRDENA: This flag is used to turn on and off the HSGPL. It is used in
    particular at start-up when the DSR detects a cartridge in the
    cartridge slot. In that case the memory locations 6000-7fff must be
    deactivated on the HSGPL, or it would cause a collision with the
    cartridge. However, CRDENA must not completely turn off the HSGPL, or the
    console will not start up at all. At least GROMs 0, 1, and 2 must remain
    active.
    The technical specifications are not clear enough at this point.

    Raphael Nabet, 2003.

    Michael Zapf
    October 2010: Rewritten as device
    February 2012: Rewritten as class

*****************************************************************************/

#include "emu.h"
#include "hsgpl.h"

#define LOG_WARN        (1U<<1)   // Warnings
#define LOG_CONFIG      (1U<<2)   // Configuration
#define LOG_PORT        (1U<<3)
#define LOG_DSR         (1U<<4)
#define LOG_BANKING     (1U<<5)
#define LOG_CRU         (1U<<6)
#define LOG_ADDRESS     (1U<<7)
#define LOG_READ        (1U<<8)
#define LOG_WRITE       (1U<<9)
#define LOG_IGNORE      (1U<<10)

#define VERBOSE ( LOG_CONFIG | LOG_WARN )

#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(TI99_HSGPL, bus::ti99::peb, snug_high_speed_gpl_device, "ti99_hsgpl", "SNUG High-speed GPL card")

namespace bus { namespace ti99 { namespace peb {

#define CRU_BASE 0x1B00
#define SUPERCART_BASE 0x0800

#define RAM6_TAG "cartram128k"
#define GRAM_TAG "gram128k"

#define DSR_EEPROM "u9_dsr"
#define GROM_B_EEPROM "u4_grom"
#define GROM_A_EEPROM "u1_grom"
#define ROM6_EEPROM "u6_rom6"

snug_high_speed_gpl_device::snug_high_speed_gpl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_HSGPL, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_dsr_eeprom(*this, DSR_EEPROM),
	m_rom6_eeprom(*this, ROM6_EEPROM),
	m_grom_a_eeprom(*this, GROM_A_EEPROM),
	m_grom_b_eeprom(*this, GROM_B_EEPROM),
	m_ram6_memory(*this, RAM6_TAG),
	m_gram_memory(*this, GRAM_TAG),
	m_dsr_enabled(false),
	m_gram_enabled(false),
	m_bank_inhibit(false),
	m_dsr_page(0),
	m_card_enabled(false),
	m_write_enabled(false),
	m_supercart_enabled(false),
	m_led_on(false),
	m_mbx_enabled(false),
	m_ram_enabled(false),
	m_flash_mode(false),
	m_current_grom_port(0),
	m_current_bank(0),
	m_module_bank(0),
	m_waddr_LSB(false),
	m_raddr_LSB(false),
	m_grom_address(0)
{
}

/*
   Read hsgpl CRU interface. None here.
*/
READ8Z_MEMBER(snug_high_speed_gpl_device::crureadz)
{
	return;
}

/*
    Write hsgpl CRU interface
*/
WRITE8_MEMBER(snug_high_speed_gpl_device::cruwrite)
{
	// SuperCart handling - see gromport.c
	if (m_supercart_enabled && ((offset & 0xfff0)==SUPERCART_BASE))
	{
		if (data != 0)
		{
			LOGMASKED(LOG_CRU, "Supercart cru setting %04x\n", offset);
			m_current_bank = (offset-0x0802)>>2;
		}
		return;
	}

	// Common CRU handling
	if ((offset & 0xff00)==CRU_BASE)
	{
		int bit = (offset >> 1) & 0x0f;
		switch (bit)
		{
		case 0:
			m_dsr_enabled = (data != 0);
			LOGMASKED(LOG_CRU, "Set dsr_enabled=%x\n", data);
			break;
		case 1:
			m_gram_enabled = (data != 0);
			LOGMASKED(LOG_CRU, "Set gram_enabled=%x\n", data);
			break;
		case 2:
			m_bank_inhibit = (data != 0);
			LOGMASKED(LOG_CRU, "Set bank_inhibit=%x\n", data);
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			if (data)
				m_dsr_page |= (1 << (bit-3));
			else
				m_dsr_page &= ~(1 << (bit-3));
			LOGMASKED(LOG_CRU, "Set dsr_page=%d\n", m_dsr_page);
			break;
		case 9:
			m_card_enabled = data;
			LOGMASKED(LOG_CRU, "Set card_enabled=%x\n", data);
			break;
		case 10:
			m_write_enabled = data;
			LOGMASKED(LOG_CRU, "Set write_enabled=%x\n", data);
			break;
		case 11:
			m_supercart_enabled = data;
			// CHECK: Do we have to reset the bank?
			LOGMASKED(LOG_CRU, "Set supercart_enabled=%x\n", data);
			break;
		case 12:
			m_led_on = data;
			LOGMASKED(LOG_CRU, "Set led_on=%x\n", data);
			break;
		case 13:
			break;
		case 14:
			m_mbx_enabled = data;
			LOGMASKED(LOG_CRU, "Set mbx_enabled=%x\n", data);
			break;
		case 15:
			m_ram_enabled = data;
			LOGMASKED(LOG_CRU, "Set ram_enabled=%x\n", data);
			break;
		}
	}
}

/*
    Memory read
*/
READ8Z_MEMBER(snug_high_speed_gpl_device::readz)
{
	if ((offset & 0x7e000)==0x74000)
	{
		dsrspace_readz(space, offset & 0xffff, value, mem_mask);
	}

	if ((offset & 0x7e000)==0x76000)
	{
		cartspace_readz(space, offset & 0xffff, value, mem_mask);
	}

	// 1001 1wbb bbbb bba0
	if ((offset & 0x7fc01)==0x79800)
	{
		grom_readz(space, offset & 0xffff, value, mem_mask);
	}
}

/*
    Memory write
*/
WRITE8_MEMBER(snug_high_speed_gpl_device::write)
{
	if ((offset & 0x7e000)==0x76000)
	{
		cartspace_write(space, offset & 0xffff, data, mem_mask);
	}

	// 1001 1wbb bbbb bba0
	if ((offset & 0x7fc01)==0x79c00)
	{
		grom_write(space, offset & 0xffff, data, mem_mask);
	}
}

/*
    Specific read access: dsrspace
*/
void snug_high_speed_gpl_device::dsrspace_readz(address_space& space, offs_t offset, uint8_t* value, uint8_t mem_mask)
{
	if (m_dsr_enabled)
	{
		*value = m_dsr_eeprom->read(space, (offset & 0x1fff) | (m_dsr_page<<13), mem_mask);
		LOGMASKED(LOG_READ, "read dsr %04x[%02x] -> %02x\n", offset, m_dsr_page, *value);
	}
}

/*
    Specific read access: cartspace
*/
void snug_high_speed_gpl_device::cartspace_readz(address_space& space, offs_t offset, uint8_t* value, uint8_t mem_mask)
{
	if (!m_card_enabled || m_flash_mode)
	{
		LOGMASKED(LOG_IGNORE, "cartridge space read ignored (enable=%d, flash_mode=%d)\n", m_card_enabled, m_flash_mode);
		return;
	}

	if (m_module_bank < 16)
	{
		*value = m_rom6_eeprom->read(space, (offset & 0x1fff) | (m_current_bank<<13) | (m_current_grom_port<<15), mem_mask);
		LOGMASKED(LOG_READ, "cartridge space read %04x -> %02x\n", offset, *value);
	}
	else
	{
		if (m_module_bank==16 || m_module_bank==17)
		{
			*value = m_ram6_memory->pointer()[(offset & 0x1fff) | (m_current_bank<<13) | ((m_module_bank & 0x000f)<<15)];
		}
		else
		{
			LOGMASKED(LOG_WARN, "unknown 0x6000 port\n");
		}
	}
}

/*
    Specific read access: grom
    Although we have a complete emulation of a GROM circuit, we need to re-implement
    it here - which is indeed closer to reality, since the real HSGPL also
    emulates GROM instead of using proper ones.
*/
void snug_high_speed_gpl_device::grom_readz(address_space& space, offs_t offset, uint8_t* value, uint8_t mem_mask)
{
	if (machine().side_effects_disabled()) return;

	//activedevice_adjust_icount(-4);

	// 1001 10bb bbbb bba0
	int port = (offset & 0x3fc) >> 2;

	if (offset & 2)
	{   // Read GPL address. This must be available even when the rest
		// of the card is offline (card_enabled=0).
		m_waddr_LSB = false;

		if (m_raddr_LSB)
		{
			*value = ((m_grom_address + 1) & 0xff);
			m_raddr_LSB = false;
		}
		else
		{
			*value = ((m_grom_address + 1) >> 8) & 0xff;
			m_raddr_LSB = true;
		}
	}
	else
	{   /* read GPL data */
		bool bNew = (port != m_current_grom_port);
		m_current_grom_port = port;

		// It is not clear what effect a CRDENA=0 really has.
		// At least GROMs 0-2 must remain visible, or the console will lock up.
		if (m_card_enabled || m_grom_address < 0x6000)
		{
			if ((port < 2) && (m_gram_enabled))
			{
				*value = m_gram_memory->pointer()[m_grom_address | (port<<16)];
				m_module_bank = port + 16;
				if (bNew) LOGMASKED(LOG_PORT, "GRAM read access at %04x (GRMENA=1) - switch to bank %d\n", offset & 0xffff, m_module_bank);
			}
			else
			{
				if (port < 8)
				{
					if (!m_flash_mode)
					{
						*value = m_grom_a_eeprom->read(space, m_grom_address | (port<<16), mem_mask);
						m_module_bank = port;
						if (bNew) LOGMASKED(LOG_PORT, "GROM read access at %04x - switch to bank %d\n", offset & 0xffff, m_module_bank);
					}
				}
				else
				{
					if (port < 16)
					{
						*value = m_grom_b_eeprom->read(space, m_grom_address | ((port-8)<<16), mem_mask);
						m_module_bank = port;
						if (bNew) LOGMASKED(LOG_PORT, "GROM read access at %04x - switch to bank %d\n", offset & 0xffff, m_module_bank);
					}
					else
					{
						if (port < 24)
						{
							// 9840-985c
							// DSR banks 0-63 (8 KiB per bank, 8 banks per port)
							*value = m_dsr_eeprom->read(space, m_grom_address | ((port-16)<<16), mem_mask);
							// Don't change the module port
							if (bNew) LOGMASKED(LOG_DSR, "read access to DSR bank %d-%d (%04x)\n", (port-16)<<3, ((port-16)<<3)+7, offset);
						}
						else
						{
							if (port < 32)
							{
								// 9860-987c (ports 24-31)
								// Each ROM6 is available as 4 (sub)banks (switchable via 6000, 6002, 6004, 6006)
								// Accordingly, each port has two complete sets
								*value = m_rom6_eeprom->read(space, m_grom_address | ((port-24)<<16), mem_mask);
								if (bNew) LOGMASKED(LOG_PORT, "ROM6 read access for module bank %d-%d (%04x)\n", (port-24)<<1, ((port-24)<<1)+1, offset & 0xffff);
							}
							else
							{
								// 9880, 9884
								if (port==32 || port==33)
								{
									*value = m_gram_memory->pointer()[m_grom_address | ((port-32)<<16)];
									m_module_bank = port - 16;
									if (bNew) LOGMASKED(LOG_PORT, "GRAM read access at %04x  - switch to bank %d\n", offset & 0xffff, m_module_bank);
								}
								else
								{
									if (port==48 || port==49)
									{
//                                      *value = m_ram6_memory->pointer()[m_grom_address];
										*value = m_ram6_memory->pointer()[m_grom_address | ((port-48)<<16)];
										if (bNew) LOGMASKED(LOG_PORT, "RAM read access at %04x\n", offset & 0xffff);
									}
									else
									{
										LOGMASKED(LOG_WARN, "Attempt to read from undefined port 0x%0x; ignored.\n", port);
									}
								}
							}
						}
					}
				}
			}
		}
		// The address auto-increment should be done even when the card is
		// offline
		LOGMASKED(LOG_ADDRESS, "HSGPL GROM address %04x\n", m_grom_address);
		m_grom_address++;
		m_raddr_LSB = m_waddr_LSB = false;
	}
}

/*
    Specific write access: cartspace
*/
void snug_high_speed_gpl_device::cartspace_write(address_space& space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (!m_card_enabled || m_flash_mode)
	{
		LOGMASKED(LOG_IGNORE, "write ignored: card_enabled=%d, flash_mode=%d\n", m_card_enabled, m_flash_mode);
		return;
	}

	LOGMASKED(LOG_WRITE, "cartridge space write %04x <- %02x\n", offset, data);

	if (!m_bank_inhibit && (m_module_bank < 16))
	{
		if ((offset & 1) == 0)
		{
			if ((offset & 0x9ff0)!=0) LOGMASKED(LOG_WARN, "unplausible ROM6 write: %04x <- %02x\n", offset, data);
			m_current_bank = (offset>>1) & 3;
			LOGMASKED(LOG_BANKING, "select bank %d\n", m_current_bank);
		}
		return;     /* right??? */
	}

	if ((m_mbx_enabled) && (offset==0x6ffe))
	{   /* MBX: mapper at 0x6ffe */
		m_current_bank = data & 0x03;
		LOGMASKED(LOG_BANKING, "select bank MBX %d\n", m_current_bank);
		return;
	}

	// MBX: RAM in 0x6c00-0x6ffd (it is unclear whether the MBX RAM area is
	// enabled/disabled by the wriena bit).  I guess RAM is unpaged, but it is
	// not implemented
	if ((m_write_enabled) || ((m_mbx_enabled) && ((offset & 0xfc00)==0x6c00)))
	{
		if ((m_module_bank < 2) && (m_ram_enabled))
		{
			m_ram6_memory->pointer()[(offset & 0x1fff) | (m_current_bank<<13) | (m_module_bank<<15) ] = data;
		}
		else
		{   // keep in mind that these lines are also reached for port < 2
			// and !ram_enabled
			if (m_module_bank < 16)
			{
				LOGMASKED(LOG_WARN, "invalid write %04x <- %02x\n", offset, data);
			// feeprom is normally written to using GPL ports, and I don't know
			// whether writing through >6000 page is enabled
/*
            at29c040a_w(feeprom_rom6, 1 + 2*offset + 0x2000*hsgpl.cur_bank + 0x8000*port, data);
            at29c040a_w(feeprom_rom6, 2*offset + 0x2000*hsgpl.cur_bank + 0x8000*port, data >> 8);
*/
			}
			else
			{
				if (m_module_bank==16 || m_module_bank==17)
				{
					m_ram6_memory->pointer()[(offset & 0x1fff) | (m_current_bank<<13) | ((m_module_bank-16)<<15)] = data;
				}
				else
				{
					LOGMASKED(LOG_WARN, "unknown 0x6000 port\n");
				}
			}
		}
	}
}

/*
    Specific write access: grom_write
*/
void snug_high_speed_gpl_device::grom_write(address_space& space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (machine().side_effects_disabled()) return;

	//activedevice_adjust_icount(-4);

	// 1001 11bb bbbb bba0
	int port = (offset & 0x3fc) >> 2;

	if (offset & 2)
	{   // Write GPL address. This must be available even when the rest
		// of the card is offline (card_enabled=0).
		m_raddr_LSB = false;

		if (m_waddr_LSB)
		{
			m_grom_address = (m_grom_address & 0xFF00) | data;
			m_waddr_LSB = false;
		}
		else
		{
			m_grom_address = (data << 8) | (m_grom_address & 0xFF);
			m_waddr_LSB = true;
		}
	}
	else
	{
		bool bNew = (port != m_current_grom_port);
		m_current_grom_port = port;
		// It is not clear what effect a CRDENA=0 really has.
		// At least GROMs 0-2 must remain visible, or the console will lock up.
		if (m_card_enabled || m_grom_address < 0x6000)
		{
			/* write GPL data */
			if (m_write_enabled)
			{
				if ((port < 2) && (m_gram_enabled))
				{
					m_gram_memory->pointer()[m_grom_address | (port<<16)] = data;
					m_module_bank = port + 16;
					if (bNew) LOGMASKED(LOG_PORT, "GRAM write access at %04x (GRMENA=1) - switch to bank %d\n", offset & 0xffff, port);
				}
				else
				{
					if (port < 8)
					{
						m_grom_a_eeprom->write(space, m_grom_address | (port<<16), data, mem_mask);
						m_module_bank = port;
						if (bNew) LOGMASKED(LOG_PORT, "GROM write access at %04x - switch to bank %d\n", offset & 0xffff, port);
					}
					else
					{
						if (port < 16)
						{
							m_grom_b_eeprom->write(space, m_grom_address | ((port-8)<<16), data, mem_mask);
							m_module_bank = port;
							if (bNew) LOGMASKED(LOG_PORT, "GROM write access at %04x - switch to bank %d\n", offset & 0xffff, port);
						}
						else
						{
							if (port < 24)
							{
								m_dsr_eeprom->write(space, m_grom_address | ((port-16)<<16), data, mem_mask);
								if (bNew) LOGMASKED(LOG_DSR, "write access to DSR bank %d-%d (%04x)\n", (port-16)<<3, ((port-16)<<3)+7, offset);
							}
							else
							{
								if (port < 32)
								{
									m_rom6_eeprom->write(space, m_grom_address | ((port-24)<<16), data, mem_mask);
									if (bNew) LOGMASKED(LOG_PORT, "ROM6 write access for module bank %d-%d (%04x)\n", (port-24)<<1, ((port-24)<<1)+1,offset & 0xffff);
								}
								else
								{
									if (port==32 || port==33)
									{
										m_gram_memory->pointer()[m_grom_address | ((port-32)<<16)] = data;
										m_module_bank = port - 16;
										if (bNew) LOGMASKED(LOG_PORT, "GRAM write access at %04x - switch to bank %d\n", offset & 0xffff, m_module_bank);
									}
									else
									{
										if (port==48 || port==49)
										{
//                                          m_ram6_memory->pointer()[m_grom_address] = data;
											m_ram6_memory->pointer()[m_grom_address | ((port-48)<<16)] = data;
											if (bNew) LOGMASKED(LOG_PORT, "RAM write access at %04x\n", offset & 0xffff);
										}
										else
										{
											LOGMASKED(LOG_WARN, "Attempt to write to undefined port; ignored.\n");
										}
									}
								}
							}
						}
					}
				}
			}
		}
		// The address auto-increment should be done even when the card is
		// offline
		m_grom_address++;
		m_raddr_LSB = m_waddr_LSB = false;
	}
}


void snug_high_speed_gpl_device::device_start()
{
	save_item(NAME(m_dsr_enabled));
	save_item(NAME(m_gram_enabled));
	save_item(NAME(m_bank_inhibit));
	save_item(NAME(m_dsr_page));
	save_item(NAME(m_card_enabled));
	save_item(NAME(m_write_enabled));
	save_item(NAME(m_supercart_enabled));
	save_item(NAME(m_led_on));
	save_item(NAME(m_mbx_enabled));
	save_item(NAME(m_ram_enabled));
	save_item(NAME(m_flash_mode));
	save_item(NAME(m_current_grom_port));
	save_item(NAME(m_current_bank));
	save_item(NAME(m_module_bank));
	save_item(NAME(m_waddr_LSB));
	save_item(NAME(m_raddr_LSB));
	save_item(NAME(m_grom_address));
}

void snug_high_speed_gpl_device::device_reset()
{
	m_dsr_enabled = false;
	m_gram_enabled = false;
	m_bank_inhibit = false;
	m_dsr_page = 0;
	m_card_enabled = true;  // important, assumed to be enabled by default
	m_write_enabled = false;
	m_supercart_enabled = false;
	m_led_on = false;
	m_mbx_enabled = false;
	m_ram_enabled = false;
	m_flash_mode = (ioport("HSGPLMODE")->read()==0);

	m_current_grom_port = 0;
	m_current_bank = 0;

	m_waddr_LSB = false;
	m_raddr_LSB = false;
	m_grom_address = 0;
	m_module_bank = 0;
}

void snug_high_speed_gpl_device::device_stop()
{
}

// Flash setting is used to flash an empty HSGPL DSR ROM
INPUT_PORTS_START( ti99_hsgpl)
	PORT_START( "HSGPLMODE" )
	PORT_DIPNAME( 0x01, 0x01, "HSGPL mode" )
		PORT_DIPSETTING(    0x00, "Flash" )
		PORT_DIPSETTING(    0x01, "Normal" )
INPUT_PORTS_END

void snug_high_speed_gpl_device::device_add_mconfig(machine_config &config)
{
	AT29C040A(config, DSR_EEPROM);
	AT29C040A(config, GROM_B_EEPROM);
	AT29C040A(config, GROM_A_EEPROM);
	AT29C040A(config, ROM6_EEPROM);

	RAM(config, RAM6_TAG).set_default_size("128K").set_default_value(0);
	RAM(config, GRAM_TAG).set_default_size("128K").set_default_value(0);
}

ioport_constructor snug_high_speed_gpl_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ti99_hsgpl);
}

} } } // end namespace bus::ti99::peb
