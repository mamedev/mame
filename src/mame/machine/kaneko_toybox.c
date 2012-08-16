/* Kaneko 'Toybox' protection
 
 the following chips have been seen

TBSOP01 is a NEC uPD78324 series MCU with 32K internal rom & 1024 bytes of ram
TBSOP02 is likely the same NEC uPD78324 series MCU as the TBS0P01

I'm guessing the only actual difference between them is the decryption table used
although the Jackie Chan info below contradicts that..

Currently none of the MCUs' internal roms are dumped so simulation is used


94  Bonk's Adventure             TOYBOX?            TBSOP01
94  Blood Warrior                TOYBOX?            TBS0P01 452 9339PK001
94  Great 1000 Miles Rally       TOYBOX                                                  "MM0525-TOYBOX199","USMM0713-TB1994 "
94  Great 1000 Miles Rally EV/US TOYBOX                 
95  Great 1000 Miles Rally 2     TOYBOX      KANEKO TBSOP02 454 9451MK002 (74 pin PQFP)  "USMM0713-TB1994 "
95  Jackie Chan                  TOYBOX                                                  "USMM0713-TB1994 "
95  Gals Panic 3                 TOYBOX?            TBSOP01

 todo:

 bonk:
	Where does the hardcoded EEPROM default data come from (there is a command to restore defaults directly, not from RAM)
	Where does the data for the additional tables come from, a transfer mode none of the other games use is used. (related to src[offs+6] and src[offs+7] params? )


MCU parameters:
---------------

mcu_command = kaneko16_mcu_ram[0x0010/2];    // command nb
mcu_offset  = kaneko16_mcu_ram[0x0012/2]/2;  // offset in shared RAM where MCU will write
mcu_subcmd  = kaneko16_mcu_ram[0x0014/2];    // sub-command parameter, happens only for command #4


    the only MCU commands found in program code are:
    - 0x04: protection: provide data (see below) and code <<<---!!!
    - 0x03: read DSW
    - 0x02: load game settings \ stored in ATMEL AT93C46 chip,
    - 0x42: save game settings / 128 bytes serial EEPROM
	- 0x43: restore eeprom defaults (from internal ROM or data ROM?)

*/



#include "emu.h"
#include "kaneko_toybox.h"
#include "machine/eeprom.h"

const device_type KANEKO_TOYBOX = &device_creator<kaneko_toybox_device>;

kaneko_toybox_device::kaneko_toybox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KANEKO_TOYBOX, "kaneko_toybox_device", tag, owner, clock)
{
	m_tabletype = TABLE_NORMAL;
	m_gametype = GAME_NORMAL;
}

void kaneko_toybox_device::set_toybox_table(device_t &device, int tabletype)
{
	kaneko_toybox_device &dev = downcast<kaneko_toybox_device &>(device);
	dev.m_tabletype = tabletype;
}

void kaneko_toybox_device::set_toybox_gametype(device_t &device, int gametype)
{
	kaneko_toybox_device &dev = downcast<kaneko_toybox_device &>(device);
	dev.m_gametype = gametype;
}


void kaneko_toybox_device::device_start()
{
	m_toybox_mcuram = (UINT16*)auto_alloc_array_clear(this->machine(), UINT16, 0x10000/2);
	memset(m_toybox_mcu_com, 0, 4 * sizeof( UINT16) );
	toxboy_decrypt_rom(this->machine());


	save_pointer(NAME(m_toybox_mcuram), 0x10000/2);
	save_item(NAME(m_toybox_mcu_com[0]));
	save_item(NAME(m_toybox_mcu_com[1]));
	save_item(NAME(m_toybox_mcu_com[2]));
	save_item(NAME(m_toybox_mcu_com[3]));

}

void kaneko_toybox_device::device_reset()
{
	toybox_mcu_init(this->machine());
}


READ16_MEMBER(kaneko_toybox_device::toybox_mcu_ram_r)
{
	return m_toybox_mcuram[offset];
}


WRITE16_MEMBER(kaneko_toybox_device::toybox_mcu_ram_w)
{
	COMBINE_DATA(&m_toybox_mcuram[offset]);
}



#define MCU_RESPONSE(d) memcpy(&m_toybox_mcuram[mcu_offset], d, sizeof(d))



// I use a byteswapped MCU data rom to make the transfers to the 68k side easier
//  not sure if it's all 100% endian safe
void kaneko_toybox_device::toxboy_decrypt_rom(running_machine& machine)
{

	UINT8 *src = (UINT8 *)machine.root_device().memregion(":mcudata" )->base();

	int i;

	for (i=0;i<0x020000;i++)
	{
		if (m_tabletype == TABLE_NORMAL) src[i] = src[i] + toybox_mcu_decryption_table[(i^1)&0xff];
		else src[i] = src[i] + toybox_mcu_decryption_table_alt[(i^1)&0xff];
	}


}



void kaneko_toybox_device::toxboy_handle_04_subcommand(running_machine& machine,UINT8 mcu_subcmd, UINT16*mcu_ram)
{
	UINT8 *src = (UINT8 *)machine.root_device().memregion(":mcudata")->base()+0x10000;
	UINT8* dst = (UINT8 *)mcu_ram;

	int offs = (mcu_subcmd&0x3f)*8;
	int x;

	//UINT16 unused = src[offs+0] | (src[offs+1]<<8);
	UINT16 romstart = src[offs+2] | (src[offs+3]<<8);
	UINT16 romlength = src[offs+4] | (src[offs+5]<<8);
	UINT16 ramdest = mcu_ram[0x0012/2];
	//UINT16 extra = src[offs+6] | (src[offs+7]<<8); // BONK .. important :-(

	//printf("romstart %04x length %04x\n",romstart,romlength);

	for (x=0;x<romlength;x++)
	{
		dst[BYTE_XOR_LE(ramdest+x)] = src[(romstart+x)];
	}
}


void kaneko_toybox_device::toybox_mcu_init(running_machine &machine)
{
	memset(m_toybox_mcu_com, 0, 4 * sizeof( UINT16) );
}

void kaneko_toybox_device::toybox_mcu_com_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_)
{
	COMBINE_DATA(&m_toybox_mcu_com[_n_]);
	if (m_toybox_mcu_com[0] != 0xFFFF)	return;
	if (m_toybox_mcu_com[1] != 0xFFFF)	return;
	if (m_toybox_mcu_com[2] != 0xFFFF)	return;
	if (m_toybox_mcu_com[3] != 0xFFFF)	return;

	memset(m_toybox_mcu_com, 0, 4 * sizeof( UINT16 ) );
	toybox_mcu_run(machine());
}

WRITE16_MEMBER(kaneko_toybox_device::toybox_mcu_com0_w){ toybox_mcu_com_w(offset, data, mem_mask, 0); }
WRITE16_MEMBER(kaneko_toybox_device::toybox_mcu_com1_w){ toybox_mcu_com_w(offset, data, mem_mask, 1); }
WRITE16_MEMBER(kaneko_toybox_device::toybox_mcu_com2_w){ toybox_mcu_com_w(offset, data, mem_mask, 2); }
WRITE16_MEMBER(kaneko_toybox_device::toybox_mcu_com3_w){ toybox_mcu_com_w(offset, data, mem_mask, 3); }

/*
    bonkadv and bloodwar test bit 0
*/
READ16_MEMBER(kaneko_toybox_device::toybox_mcu_status_r)
{
	logerror("CPU %s (PC=%06X) : read MCU status\n", space.device().tag(), cpu_get_previouspc(&space.device()));
	return 0; // most games test bit 0 for failure
}



void kaneko_toybox_device::toybox_mcu_run(running_machine &machine)
{
	UINT16 *kaneko16_mcu_ram = m_toybox_mcuram;
	UINT16 mcu_command	=	kaneko16_mcu_ram[0x0010/2];
	UINT16 mcu_offset	=	kaneko16_mcu_ram[0x0012/2] / 2;
	UINT16 mcu_data		=	kaneko16_mcu_ram[0x0014/2];

	//printf("command %04x\n",mcu_command);

	switch (mcu_command >> 8)
	{
		case 0x02:	// Read from NVRAM
		{
			UINT8* nvdat = (UINT8*)&kaneko16_mcu_ram[mcu_offset];

			address_space *eeprom_space = machine.device<eeprom_device>(":eeprom")->space();

			for (int i=0;i<0x80;i++)
			{
				nvdat[i] = eeprom_space->read_byte(i);
			}

			logerror("%s : MCU executed command: %04X %04X (load NVRAM settings)\n", machine.describe_context(), mcu_command, mcu_offset*2);
	
		}
		break;

		case 0x42:	// Write to NVRAM
		{
			address_space *eeprom_space = machine.device<eeprom_device>(":eeprom")->space();
			UINT8* nvdat = (UINT8*)&kaneko16_mcu_ram[mcu_offset];
			for (int i=0;i<0x80;i++)
			{
				eeprom_space->write_byte(i, nvdat[i]);
			}

			logerror("%s : MCU executed command: %04X %04X (save NVRAM settings)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x43:	// Initialize NVRAM - MCU writes Default Data Set directly to NVRAM (from internal ROM, or from the data ROM?)
		{
			// only bonk seems to do this?
			if (m_gametype == GAME_BONK)
			{
				//memcpy(m_nvram_save, bonkadv_mcu_43, sizeof(bonkadv_mcu_43));
				
				
				address_space *eeprom_space = machine.device<eeprom_device>(":eeprom")->space();
				UINT8* nvdat = (UINT8*)&bonkadv_mcu_43[0];
				for (int i=0;i<0x80;i++)
				{
					eeprom_space->write_byte(i, nvdat[i]);
				}
				logerror("%s : MCU executed command: %04X %04X (restore default NVRAM settings)\n", machine.describe_context(), mcu_command, mcu_offset*2);
			}
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = machine.root_device().ioport(":DSW1")->read();
			logerror("%s : MCU executed command: %04X %04X (read DSW)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x04:	// Protection
		{
			logerror("%s : MCU executed command: %04X %04X %04X\n", machine.describe_context(), mcu_command, mcu_offset*2, mcu_data);

			if (m_gametype == GAME_BONK)
			{
				// bonk still needs these hacks
				switch(mcu_data)
				{
					// static, in this order, at boot/reset - these aren't understood, different params in Mcu data rom, data can't be found
					case 0x34: MCU_RESPONSE(bonkadv_mcu_4_34); break;
					case 0x30: MCU_RESPONSE(bonkadv_mcu_4_30); break;
					case 0x31: MCU_RESPONSE(bonkadv_mcu_4_31); break;
					case 0x32: MCU_RESPONSE(bonkadv_mcu_4_32); break;
					case 0x33: MCU_RESPONSE(bonkadv_mcu_4_33); break;

					// dynamic, per-level (29), in level order
					default:
						toxboy_handle_04_subcommand(machine, mcu_data, kaneko16_mcu_ram);
						break;

				}
			}
			else
			{
				toxboy_handle_04_subcommand(machine, mcu_data, kaneko16_mcu_ram);
			}

		}
		break;

		default:
			logerror("%s : MCU executed command: %04X %04X %04X (UNKNOWN COMMAND)\n", machine.describe_context(), mcu_command, mcu_offset*2, mcu_data);
		break;
	}
}

