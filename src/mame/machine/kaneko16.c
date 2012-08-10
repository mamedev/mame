/***************************************************************************


                            MCU Code Simulation

  CALC1 is a 40 pin DIP MCU of unknown type with unknown internal rom
  CALC3 is a NEC uPD78322 series MCU with 16K internal rom & 640 bytes of ram
TBSOP01 is a NEC uPD78324 series MCU with 32K internal rom & 1024 bytes of ram
TBSOP02 is likely the same NEC uPD78324 series MCU as the TBS0P01

Currently none of the MCUs' internal roms are dumped so simulation is used

***************************************************************************/

#include "emu.h"
#include "includes/kaneko16.h"

#include "kanekotb.h"	// TOYBOX MCU trojaning results
#include "machine/eeprom.h"


/***************************************************************************
                                TOYBOX MCU:

                                Bonk's Adventure
                                Blood Warrior
                                Great 1000 Miles Rally
                                ...
***************************************************************************/
/*
---------------------------------------------------------------------------
                                TOYBOX

94  Bonk's Adventure            TOYBOX?            TBSOP01
94  Blood Warrior               TOYBOX?            TBS0P01 452 9339PK001
94  Great 1000 Miles Rally      TOYBOX                                                  "MM0525-TOYBOX199","USMM0713-TB1994 "
95  Great 1000 Miles Rally 2    TOYBOX      KANEKO TBSOP02 454 9451MK002 (74 pin PQFP)  "USMM0713-TB1994 "
95  Jackie Chan                 TOYBOX                                                  "USMM0713-TB1994 "
95  Gals Panic 3                TOYBOX?            TBSOP01
---------------------------------------------------------------------------

All the considerations are based on the analysis of jchan, and to a fewer extent galpani3, and make references to the current driver sources:

MCU triggering:
---------------

the 4 JCHAN_MCU_COM_W(...) are in fact 2 groups:

AM_RANGE(0x330000, 0x330001) AM_WRITE(jchan_mcu_com0_w) // _[ these 2 are set to 0xFFFF
AM_RANGE(0x340000, 0x340001) AM_WRITE(jchan_mcu_com1_w) //  [ for MCU to execute cmd

AM_RANGE(0x350000, 0x350001) AM_WRITE(jchan_mcu_com2_w) // _[ these 2 are set to 0xFFFF
AM_RANGE(0x360000, 0x360001) AM_WRITE(jchan_mcu_com3_w) //  [ for MCU to return its status


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


Current feeling of devs is that this EEPROM might also play a role in the protection scheme,
but I (SV) feel that it is very unlikely because of the following, which has been verified:
if the checksum test fails at most 3 times, then the initial settings, stored in main68k ROM,
are loaded in RAM then saved with cmd 0x42 (see code @ $5196 & $50d4)
Note that this is valid for jchan only, other games haven't been looked at.

Others:
-------

There is one interesting MCU cmd $4 in jchan:
-> sub-cmd $3d, MCU writes the string "USMM0713-TB1994 "

The very same string is written by gtmr games (gtmre/gtmrusa/gtmr2) but apparently with no sub-cmd: this string is
probably the MCU model string, so this one should be in internal MCU ROM (another one for gtmr is "MM0525-TOYBOX199")

TODO: look at this one since this remark is only driver-based.
*/


static const UINT8 toybox_mcu_decryption_table[0x100] = {
0x7b,0x82,0xf0,0xbc,0x7f,0x1d,0xa2,0xc5,0x2a,0xfa,0x55,0xee,0x1a,0xd0,0x59,0x76,
0x5e,0x75,0x79,0x16,0xa5,0xf6,0x84,0xed,0x0f,0x2e,0xf2,0x36,0x61,0xac,0xcd,0xab,
0x01,0x3b,0x01,0x87,0x73,0xab,0xce,0x5d,0xd4,0x1d,0x68,0x2a,0x35,0xea,0x13,0x27,
0x00,0xaa,0x46,0x36,0x6e,0x65,0x80,0x7e,0x19,0xe2,0x96,0xab,0xac,0xa5,0x6c,0x63,
0x4a,0x6f,0x87,0xf6,0x6a,0xac,0x38,0xe2,0x1f,0x87,0xf9,0xaa,0xf5,0x41,0x60,0xa6,
0x42,0xb9,0x30,0xf2,0xc3,0x1c,0x4e,0x4b,0x08,0x10,0x42,0x32,0xbf,0xb2,0xc5,0x0f,
0x7a,0xab,0x97,0xf6,0xe7,0xb3,0x46,0xf8,0xec,0x2b,0x7d,0x5f,0xb1,0x10,0x03,0xe4,
0x0f,0x22,0xdf,0x8d,0x10,0x66,0xa7,0x7e,0x96,0xbd,0x5a,0xaf,0xaa,0x43,0xdf,0x10,
0x7c,0x04,0xe2,0x9d,0x66,0xd7,0xf0,0x02,0x58,0x8a,0x55,0x17,0x16,0xe2,0xe2,0x52,
0xaf,0xd9,0xf9,0x0d,0x59,0x70,0x86,0x3c,0x05,0xd1,0x52,0xa7,0xf0,0xbf,0x17,0xd0,
0x23,0x15,0xfe,0x23,0xf2,0x80,0x60,0x6f,0x95,0x89,0x67,0x65,0xc9,0x0e,0xfc,0x16,
0xd6,0x8a,0x9f,0x25,0x2c,0x0f,0x2d,0xe4,0x51,0xb2,0xa8,0x18,0x3a,0x5d,0x66,0xa0,
0x9f,0xb0,0x58,0xea,0x78,0x72,0x08,0x6a,0x90,0xb6,0xa4,0xf5,0x08,0x19,0x60,0x4e,
0x92,0xbd,0xf1,0x05,0x67,0x4f,0x24,0x99,0x69,0x1d,0x0c,0x6d,0xe7,0x74,0x88,0x22,
0x2d,0x15,0x7a,0xa2,0x37,0xa9,0xa0,0xb0,0x2c,0xfb,0x27,0xe5,0x4f,0xb6,0xcd,0x75,
0xdc,0x39,0xce,0x6f,0x1f,0xfe,0xcc,0xb5,0xe6,0xda,0xd8,0xee,0x85,0xee,0x2f,0x04,
};


static const UINT8 toybox_mcu_decryption_table_alt[0x100] = {
0x26,0x17,0xb9,0xcf,0x1a,0xf5,0x14,0x1e,0x0c,0x35,0xb3,0x66,0xa0,0x17,0xe9,0xe4,
0x90,0xf6,0xd5,0x35,0xac,0x95,0x49,0x43,0x64,0x0c,0x03,0x75,0x4d,0xda,0xb6,0xdf,
0x06,0xcf,0x83,0x9e,0x35,0x2c,0x71,0x2a,0xab,0xcc,0x65,0xd4,0x1f,0xb0,0x88,0x3c,
0xb7,0x87,0x35,0xc0,0x41,0x65,0x9f,0xa0,0xd5,0x8c,0x3e,0x06,0x53,0xdb,0x45,0x64,
0x09,0x1e,0xc5,0x8d,0x50,0x24,0xe2,0x4a,0x9b,0x99,0x77,0x25,0x43,0xa9,0x1d,0xac,
0x99,0x31,0x75,0xb5,0x53,0xab,0xad,0x5a,0x42,0x14,0xa1,0x52,0xac,0xec,0x5f,0xf8,
0x8c,0x78,0x05,0x47,0xea,0xb8,0xde,0x69,0x98,0x2d,0x8f,0x9d,0xfc,0x05,0xea,0xee,
0x77,0xbb,0xa9,0x31,0x01,0x00,0xea,0xd8,0x9c,0x43,0xb5,0x2f,0x4e,0xb5,0x1b,0xd2,
0x01,0x4b,0xc4,0xf8,0x76,0x92,0x59,0x4f,0x20,0x52,0xd9,0x7f,0xa9,0x19,0xe9,0x7c,
0x8d,0x3b,0xec,0xe0,0x60,0x08,0x2e,0xbd,0x27,0x8b,0xb2,0xfc,0x29,0xd8,0x39,0x8a,
0x4f,0x2f,0x6b,0x04,0x10,0xbd,0xa1,0x04,0xde,0xc0,0xd5,0x0f,0x04,0x86,0xd6,0xd8,
0xfd,0xb1,0x3c,0x4c,0xd1,0xc4,0xf1,0x5b,0xf5,0x8b,0xe3,0xc4,0x89,0x3c,0x39,0x86,
0xd2,0x92,0xc9,0xe5,0x2c,0x4f,0xe2,0x2f,0x2d,0xc5,0x35,0x09,0x94,0x47,0x3c,0x04,
0x40,0x8b,0x57,0x08,0xf6,0x74,0xe9,0xb8,0x36,0x4d,0xc5,0x26,0x13,0x3d,0x75,0xa0,
0xa8,0x29,0x09,0x8c,0x87,0xf7,0x13,0xaf,0x4c,0x38,0x0b,0x8a,0x7f,0x2c,0x62,0x27,
0x47,0xaa,0xda,0x07,0x92,0x8d,0xfd,0x1f,0xee,0x48,0x1a,0x53,0x3b,0x98,0x6a,0x72,
};

// I use a byteswapped MCU data rom to make the transfers to the 68k side easier
//  not sure if it's all 100% endian safe
DRIVER_INIT( decrypt_toybox_rom )
{

	UINT8 *src = (UINT8 *)machine.root_device().memregion("mcudata" )->base();

	int i;

	for (i=0;i<0x020000;i++)
	{
		src[i] = src[i] + toybox_mcu_decryption_table[(i^1)&0xff];
	}

	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"%s.mcudata", machine.system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(src, 0x20000, 1, fp);
			fclose(fp);
		}
	}
	#endif
}

DRIVER_INIT( decrypt_toybox_rom_alt )
{

	UINT8 *src = (UINT8 *)machine.root_device().memregion("mcudata" )->base();

	int i;

	for (i=0;i<0x020000;i++)
	{
		src[i] = src[i] + toybox_mcu_decryption_table_alt[(i^1)&0xff];
	}
}

void toxboy_handle_04_subcommand(running_machine& machine,UINT8 mcu_subcmd, UINT16*mcu_ram)
{
	UINT8 *src = (UINT8 *)machine.root_device().memregion("mcudata")->base()+0x10000;
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


void toybox_mcu_init(running_machine &machine)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	memset(state->m_toybox_mcu_com, 0, 4 * sizeof( UINT16) );
}

void kaneko16_state::toybox_mcu_com_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_)
{
	COMBINE_DATA(&m_toybox_mcu_com[_n_]);
	if (m_toybox_mcu_com[0] != 0xFFFF)	return;
	if (m_toybox_mcu_com[1] != 0xFFFF)	return;
	if (m_toybox_mcu_com[2] != 0xFFFF)	return;
	if (m_toybox_mcu_com[3] != 0xFFFF)	return;

	memset(m_toybox_mcu_com, 0, 4 * sizeof( UINT16 ) );
	(*m_toybox_mcu_run)(machine());
}

WRITE16_MEMBER(kaneko16_state::toybox_mcu_com0_w){ toybox_mcu_com_w(offset, data, mem_mask, 0); }
WRITE16_MEMBER(kaneko16_state::toybox_mcu_com1_w){ toybox_mcu_com_w(offset, data, mem_mask, 1); }
WRITE16_MEMBER(kaneko16_state::toybox_mcu_com2_w){ toybox_mcu_com_w(offset, data, mem_mask, 2); }
WRITE16_MEMBER(kaneko16_state::toybox_mcu_com3_w){ toybox_mcu_com_w(offset, data, mem_mask, 3); }

/*
    bonkadv and bloodwar test bit 0
*/
READ16_MEMBER(kaneko16_state::toybox_mcu_status_r)
{
	logerror("CPU %s (PC=%06X) : read MCU status\n", space.device().tag(), cpu_get_previouspc(&space.device()));
	return 0; // most games test bit 0 for failure
}


/***************************************************************************
                                Blood Warrior
***************************************************************************/

void bloodwar_mcu_run(running_machine &machine)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	UINT16 *kaneko16_mcu_ram = state->m_mcu_ram;
	UINT16 mcu_command	=	kaneko16_mcu_ram[0x0010/2];
	UINT16 mcu_offset	=	kaneko16_mcu_ram[0x0012/2] / 2;
	UINT16 mcu_data		=	kaneko16_mcu_ram[0x0014/2];

	switch (mcu_command >> 8)
	{
		case 0x02:	// Read from NVRAM
		{
			memcpy(&kaneko16_mcu_ram[mcu_offset], state->m_nvram_save, sizeof(state->m_nvram_save));
			logerror("%s : MCU executed command: %04X %04X (load NVRAM settings)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x42:	// Write to NVRAM
		{
			memcpy(state->m_nvram_save, &kaneko16_mcu_ram[mcu_offset], sizeof(state->m_nvram_save));
			logerror("%s : MCU executed command: %04X %04X (save NVRAM settings)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = machine.root_device().ioport("DSW1")->read();
			logerror("%s : MCU executed command: %04X %04X (read DSW)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x04:	// Protection
		{
			logerror("%s : MCU executed command: %04X %04X %04X\n", machine.describe_context(), mcu_command, mcu_offset*2, mcu_data);

			toxboy_handle_04_subcommand(machine, mcu_data, kaneko16_mcu_ram);

		}
		break;

		default:
			logerror("%s : MCU executed command: %04X %04X %04X (UNKNOWN COMMAND)\n", machine.describe_context(), mcu_command, mcu_offset*2, mcu_data);
		break;
	}
}

/***************************************************************************
                                Bonk's Adventure
***************************************************************************/

void bonkadv_mcu_run(running_machine &machine)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	UINT16 *kaneko16_mcu_ram = state->m_mcu_ram;
	UINT16 mcu_command	=	kaneko16_mcu_ram[0x0010/2];
	UINT16 mcu_offset	=	kaneko16_mcu_ram[0x0012/2] / 2;
	UINT16 mcu_data		=	kaneko16_mcu_ram[0x0014/2];

	switch (mcu_command >> 8)
	{

		case 0x02:	// Read from NVRAM
		{
			memcpy(&kaneko16_mcu_ram[mcu_offset], state->m_nvram_save, sizeof(state->m_nvram_save));
			logerror("%s : MCU executed command: %04X %04X (load NVRAM settings)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x42:	// Write to NVRAM
		{
			memcpy(state->m_nvram_save, &kaneko16_mcu_ram[mcu_offset], sizeof(state->m_nvram_save));
			logerror("%s : MCU executed command: %04X %04X (save NVRAM settings)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x43:	// Initialize NVRAM - MCU writes Default Data Set directly to NVRAM
		{
			memcpy(state->m_nvram_save, bonkadv_mcu_43, sizeof(bonkadv_mcu_43));
			logerror("%s : MCU executed command: %04X %04X (restore default NVRAM settings)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = machine.root_device().ioport("DSW1")->read();
			logerror("%s : MCU executed command: %04X %04X (read DSW)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x04:	// Protection
		{
			logerror("%s : MCU executed command: %04X %04X %04X\n", machine.describe_context(), mcu_command, mcu_offset*2, mcu_data);


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
		break;

		default:
			logerror("%s : MCU executed command: %04X %04X %04X (UNKNOWN COMMAND)\n", machine.describe_context(), mcu_command, mcu_offset*2, mcu_data);
		break;
	}
}

/***************************************************************************
                            Great 1000 Miles Rally
***************************************************************************/

/*
    MCU Tasks:

    - Write and ID string to shared RAM.
    - Access the EEPROM
    - Read the DSWs
*/

void gtmr_mcu_run(running_machine &machine)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	UINT16 *kaneko16_mcu_ram = state->m_mcu_ram;
	UINT16 mcu_command	=	kaneko16_mcu_ram[0x0010/2];
	UINT16 mcu_offset	=	kaneko16_mcu_ram[0x0012/2] / 2;
	UINT16 mcu_data		=	kaneko16_mcu_ram[0x0014/2];

	logerror("%s : MCU executed command: %04X %04X %04X\n", machine.describe_context(), mcu_command, mcu_offset*2, mcu_data);

	switch (mcu_command >> 8)
	{

		case 0x02:	// Read from NVRAM
		{
			memcpy(&kaneko16_mcu_ram[mcu_offset], state->m_nvram_save, sizeof(state->m_nvram_save));
		}
		break;

		case 0x42:	// Write to NVRAM
		{
			memcpy(state->m_nvram_save, &kaneko16_mcu_ram[mcu_offset], sizeof(state->m_nvram_save));
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = machine.root_device().ioport("DSW1")->read();
		}
		break;

		case 0x04:	// TEST (2 versions)
		{
			toxboy_handle_04_subcommand(machine, mcu_data, kaneko16_mcu_ram);
		}
		break;
	}

}

