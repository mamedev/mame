/*
Konami M2 Hardware
Preliminary driver by Ville Linde


Konami M2 Hardware Overview
Konami, 1997-1998

This hardware is 3DO-based with two IBM Power PC CPUs.

There were only 5 known games on this hardware. They include....

Game                                                 Year    CD Codes                        Konami Part#
-------------------------------------------------------------------------------------------------
Battle Tryst                                         1998    636JAC02
Evil Night                                           1998    810UBA02
Hell Night (alt. Region title, same as Evil Night)   1998    810EAA02
Heat Of Eleven '98                                   1998    703EAA02
Tobe! Polystars                                      1997    623JAA02                        003894
Total Vice                                           1997    639UAC01, 639JAD01, 639AAB01


PCB Layouts
-----------

Top Board

[M]DFUP0882ZAM1
FZ-20B1AK 7BKSA03500 (sticker)
|---------------------------------------------------|
|            |--------------------|    |----------| |
|            |--------------------|    |----------| |
|    2902             |---|  |--------|             |
| AK4309 CY2292S|---| |*2 |  |  3DO   |  |-------|  |
|               |*1 | |---|  |        |  |IBM    |  |
|        18MHz  |---|        |        |  |POWERPC|  |
|                            |        |  |602    |  |
|                            |--------|  |-------|  |
|    D4516161  D4516161                             |
|                                 |---|  |-------|  |
|DSW                    |-------| |*3 |  |IBM    |  |
|                       |       | |---|  |POWERPC|  |
|    D4516161  D4516161 |  *4   |        |602    |  |
|                       |       |        |-------|  |
|                       |-------|                   |
|---------------------------------------------------|
Notes:
      AK4309  - Asahi Kasei Microsystems AK4309-VM Digital to Analog Converter (SOIC24)
      2902    - Japan Radio Co. JRC2902 Quad Operational Amplifier (SOIC14)
      CY2292S - Cypress CY2292S Three-PLL General-Purpose EPROM Programmable Clock Generator (SOIC16)
                XTALIN - 18.000MHz, XTALOUT - 18.000MHz, XBUF - 18.000MHz, CPUCLK - 25.2000MHz
                CLKA - , CLKB -  , CLKC - 16.9345MHz, CLKD -
      3DO     - 9701 B861131 VY21118- CDE2 3DO 02473-001-0F (QFP208)
      *1      - [M] JAPAN ASUKA 9651HX001 044 (QFP44)
      *2      - Motorola MC44200FT
      *3      - [M] BIG BODY 2 BU6244KS 704 157 (QFP56)
      *4      - Unknown BGA chip (Graphics Engine, with heatsink attached)
      DSW     - 2 position dip switch


Bottom Board

PWB403045B (C) 1997 KONAMI CO., LTD.
|----------------------------------------------------------|
|           CN16    |--------------------|    |----------| |
|LA4705             |--------------------|    |----------| |
|       NJM5532D                    9.83MHz                |
|                                   19.66MHz               |
|J                |--------|   93C46.7K                    |-|
|A                | 058232 |                BOOTROM.8Q     | |
|M                |--------|   |------|                    | |
|M       |------|              |003461|                    | |
|A       |056879|              |      |                    | |CN15
|        |      |              |------|                    | |
| TEST   |------|                                          | |
|                                                          | |
|   DSW                                                    | |
|                                                          |-|
|                                                          |
|----------------------------------------------------------|
Notes:
      056879     - Konami custom IC, location 10E (QFP120)
      058232     - Konami custom ceramic flat pack IC, DAC?
      003461     - Konami custom IC, location 11K (QFP100)
      CN16       - 4 pin connector for CD-DA in from CDROM
      CN15       - Standard (PC-compatible) 40 pin IDE CDROM flat cable connector and 4 pin power plug connector,
                   connected to Panasonic CR-583 8-speed CDROM drive.
      LA4705     - LA4705 Power Amplifier
      DSW        - 8 position dip switch
      BOOTROM.8Q - 16MBit MASKROM. Location 8Q (DIP42)
                   Battle Tryst       - 636A01.8Q
                   Evil Night         -       .8Q
                   Heat Of Eleven '98 -       .8Q
                   Polystars          - 623B01.8Q
                   Total Vice         -       .8Q
      93C46.7K   - 128bytes x8bit Serial EEPROM. Location 7K (DIP8)
                   NOTE! There is very mild protection to stop game-swapping. It is based on the information in the EEPROM
                   being the same as the Time Keeper NVRAM.
                   For example, in Evil Night, the first line of the NVRAM in hex is 474E38313000000019984541410002A601FEFE01
                   Looking at it in ascii:  GN810.....EAA.......
                   Hex 474E383130 = GN810
                   1998 = the year of the game
                   Hex 454141 = EAA (the version = europe english)
                   The numbers after this appear to be unimportant (at least with regards to swapping games anyway).
                   All the other data after the first line is used for high scores tables etc.
                   The important part is that the data in the EEPROM should be the same as the NVRAM, but the EEPROM data
                   is byte-swapped! If the two don't match, the check on 7K or the NVRAM will fail and the PCB will reboot
                   forever.

Some lower boards have two connectors underneath for a protection sub-board or sound board. These are detailed below....

GX636-PWB(A) (C) 1997 KONAMI CO., LTD.
|-------------------------|
| CN4 CN3  |---------|    |
|          |---------|CN2 |
|          PAL            |
|                         |
|             NVRAM       |
|                         |
|          |---------|    |
|          |---------|CN1 |
|-------------------------|
Notes:
      NVRAM  - With Heat of Eleven '98, uses Dallas DS1643 NonVolatile TimeKeeping RAM
               With Battle Tryst, uses ST M48T58Y-70PC1 NonVolatile TimeKeeping RAM
               With Poly Stars, a sub board is not used at all
      PAL    - PALCE16V8Q, stamped 'X636A1'
      CN3    - 4-pin sound cable tied to CN16 (CD-DA Input) on main lower board
      CN4    - 4-pin sound cable tied to CDROM analog audio output connector

GQ639 PWB 403327(A)
|-----------------------------------------|
|       639JAA02.xx                       |
|                                         |
|                                         |
|                   |---------|           |
|                   |---------|           |
|                                         |
|                                         |
|               PAL                       |
|                                         |
|                                         |
|                                         |
|                                         |
|                   |---------|           |
|      YMZ280B      |---------|           |
|                                         |
|      16.9344MHz                         |
|                                         |
|                                         |
|-----------------------------------------|
Notes:
      This PCB is used on Total Vice only.
      639JAA02.xx - 8MBit Sound data ROM (DIP42)
      PAL         - PAL16V8H stampd '       '


PWB0000047043 (C) 1998 KONAMI CO., LTD.
|-----------------------------------------|
| CN4     CN3                             |
|                                         |
|                                         |
|                   |---------|           |
|                   |---------|           |
|        16.9344MHz              M48T58Y  |
|                      PAL                |
|          YMZ280B                        |
|                                         |
|                                         |
|                                         |
|                                         |
|                   |---------|           |
|                   |---------|           |
|                                         |
|                                         |
|                                         |
|              810A03.16H                 |
|-----------------------------------------|
Notes:
      This PCB is used on Evil Night/Hell Night only.
      810A03.16H - 16MBit Sound data ROM (DIP42, byte mode)
      PAL        - PAL16V8H stamped 'N810B1'
      M48T58Y    - ST M48T58Y-70PC1 NonVolatile TimeKeeping RAM
      CN3        - 4-pin sound cable tied to CN16 (CD-DA Input) on main lower board
      CN4        - 4-pin sound cable tied to CDROM analog audio output connector
*/


#include "emu.h"
#include "cdrom.h"
#include "cpu/powerpc/ppc.h"


typedef struct
{
	UINT32 dst_addr;
	int length;
	UINT32 next_dst_addr;
	int next_length;
	int dma_done;
} CDE_DMA;

class konamim2_state : public driver_device
{
public:
	konamim2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT64 *m_main_ram;
	UINT32 m_vdl0_address;
	UINT32 m_vdl1_address;
	UINT32 m_irq_enable;
	UINT32 m_irq_active;
	UINT64 m_unk3;
	UINT32 m_unk20004;
	int m_counter1;
	int m_cde_num_status_bytes;
	UINT32 m_cde_status_bytes[16];
	int m_cde_status_byte_ptr;
	UINT32 m_cde_command_bytes[16];
	int m_cde_command_byte_ptr;
	int m_cde_response;
	int m_cde_drive_state;
	int m_cde_enable_qchannel_reports;
	int m_cde_enable_seek_reports;
	int m_cde_qchannel_offset;
	cdrom_toc m_cde_toc;
	CDE_DMA m_cde_dma[2];
};


static VIDEO_START( m2 )
{
}

static SCREEN_UPDATE( m2 )
{
	konamim2_state *state = screen->machine().driver_data<konamim2_state>();
	int i, j;

	UINT32 fb_start = 0xffffffff;
	if (state->m_vdl0_address != 0)
	{
		fb_start = *(UINT32*)&state->m_main_ram[(state->m_vdl0_address - 0x40000000) / 8] - 0x40000000;
	}

	if (fb_start <= 0x800000)
	{
		UINT16 *frame = (UINT16*)&state->m_main_ram[fb_start/8];
		for (j=0; j < 384; j++)
		{
			UINT16 *fb = &frame[(j*512)];
			UINT16 *d = BITMAP_ADDR16(bitmap, j, 0);
			for (i=0; i < 512; i++)
			{
				d[i^3] = *fb++ & 0x7fff;
			}
		}
	}
	else
	{
		bitmap_fill(bitmap, cliprect, 0);
	}
	return 0;
}

static READ64_HANDLER(irq_enable_r)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	UINT64 r = 0;

	if (ACCESSING_BITS_32_63)
	{
		r |= (UINT64)(state->m_irq_enable) << 32;
	}

	return r;
}

static WRITE64_HANDLER(irq_enable_w)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	if (ACCESSING_BITS_32_63)
	{
		state->m_irq_enable |= (UINT32)(data >> 32);
	}
}

static READ64_HANDLER(irq_active_r)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	UINT64 r = 0;

	if (ACCESSING_BITS_32_63)
	{
		r |= (UINT64)(state->m_irq_active) << 32;
	}

	return r;
}



static READ64_HANDLER(unk1_r)
{
	return U64(0xffffffffffffffff);
	//return 0;
}

#ifdef UNUSED_FUNCTION
static READ64_HANDLER(unk2_r)
{
	if (ACCESSING_BITS_32_63)
	{
		return (UINT64)0xa5 << 32;
	}
	return 0;
}
#endif

static READ64_HANDLER(unk3_r)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	//return U64(0xffffffffffffffff);
	return state->m_unk3;
}

static READ64_HANDLER(unk4_r)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	UINT64 r = 0;
//  logerror("unk4_r: %08X, %08X%08X at %08X\n", offset, (UINT32)(mem_mask>>32), (UINT32)(mem_mask), cpu_get_pc(&space->device()));

	if (ACCESSING_BITS_32_63)
	{
		// MCfg
		r |= (UINT64)((0 << 13) | (5 << 10)) << 32;
	}
	if (ACCESSING_BITS_0_31)
	{
		r |= state->m_unk20004 & ~0x800000;
	}
	return r;
}

static WRITE64_HANDLER(unk4_w)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
//  logerror("unk4_w: %08X%08X, %08X, %08X%08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data),
//      offset, (UINT32)(mem_mask>>32), (UINT32)(mem_mask), cpu_get_pc(&space->device()));

	if (ACCESSING_BITS_0_31)
	{
		if (data & 0x800000)
		{
//          mame_printf_debug("CPU '%s': CPU1 IRQ at %08X\n", space->device().tag(), cpu_get_pc(&space->device()));
			cputag_set_input_line(space->machine(), "sub", INPUT_LINE_IRQ0, ASSERT_LINE);
		}

		state->m_unk20004 = (UINT32)(data);
		return;
	}
}

static READ64_HANDLER(unk30000_r)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	state->m_counter1++;
	return (UINT64)(state->m_counter1 & 0x7f) << 32;
}

static READ64_HANDLER(unk30030_r)
{
	if (ACCESSING_BITS_0_31)
	{
		return 1;
	}
	return 0;
}

static WRITE64_HANDLER(video_w)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	if (ACCESSING_BITS_32_63)
	{
		state->m_vdl0_address = (UINT32)(data >> 32);
	}
	if (ACCESSING_BITS_0_31)
	{
		state->m_vdl1_address = (UINT32)(data);
	}
}

static WRITE64_HANDLER(video_irq_ack_w)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	if (ACCESSING_BITS_32_63)
	{
		if ((data >> 32) & 0x8000)
		{
			state->m_irq_active &= ~0x800000;
		}
	}
}



static READ64_HANDLER(unk4000280_r)
{
	// SysCfg

	UINT32 sys_config = 0x03600000;

	sys_config |= 0 << 0;			// Bit 0:       PAL/NTSC switch (default is selected by encoder)
	sys_config |= 0 << 2;			// Bit 2-3:     Video Encoder (0 = MEIENC, 1 = VP536, 2 = BT9103, 3 = DENC)
	sys_config |= 3 << 11;			// Bit 11-12:   Country
									//              0 = ???
									//              1 = UK
									//              2 = Japan
									//              3 = US
	sys_config |= 0xb << 15;		// Bit 15-18:   0x8 = AC-DevCard
									//              0xb = AC-CoreBoard
									//              0xc = DevCard (not allowed)
									//              0xe = Upgrade (not allowed)
									//              0xf = Multiplayer (not allowed)
	sys_config |= 3 << 29;			// Bit 29-30:   Audio chip (1 = CS4216, 3 = Asahi AK4309)

	return ((UINT64)(sys_config) << 32);

}

static WRITE64_HANDLER(unk4000010_w)
{
	if ((data & 0xff) == 0xd)
	{
//      mame_printf_debug("\n");
	}
	else
	{
//      mame_printf_debug("%c", (UINT8)(data & 0xff));
	}
}

static WRITE64_HANDLER(unk4000418_w)
{
}

static WRITE64_HANDLER(reset_w)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	if (ACCESSING_BITS_32_63)
	{
		if (data & U64(0x100000000))
		{
			cputag_set_input_line(space->machine(), "maincpu", INPUT_LINE_RESET, PULSE_LINE);
			state->m_unk3 = 0;
		}
	}
}


/*****************************************************************************/
/* CDE */

#define CDE_DRIVE_STATE_PAUSED			0x02
#define CDE_DRIVE_STATE_SEEK_DONE		0x03









static void cde_init(running_machine &machine)
{
	konamim2_state *state = machine.driver_data<konamim2_state>();
	cdrom_file *cd = cdrom_open(get_disk_handle(machine, "cdrom"));
	const cdrom_toc *toc = cdrom_get_toc(cd);

	if (cd)
	{
		memcpy(&state->m_cde_toc, toc, sizeof(cdrom_toc));
	}

	/*
    printf("%d tracks\n", toc->numtrks);
    for (i=0; i < toc->numtrks; i++)
    {
        const cdrom_track_info *track = &toc->tracks[i];
        printf("Track %d: type %d, subtype %d, datasize %d, subsize %d, frames %d, extraframes %d, physframeofs %d\n",
            i, track->trktype, track->subtype, track->datasize, track->subsize,track->frames, track->extraframes, track->physframeofs);
    }
    */

	if (cd)
	{
		cdrom_close(cd);
	}

	state->m_cde_drive_state = CDE_DRIVE_STATE_PAUSED;

	state->m_cde_num_status_bytes = 0;
	state->m_cde_status_byte_ptr = 0;
	state->m_cde_command_byte_ptr = 0;

	state->m_cde_response = 0;

	state->m_cde_enable_qchannel_reports = 0;
	state->m_cde_enable_seek_reports = 0;

	state->m_cde_qchannel_offset = 0;
}

static void cde_handle_command(running_machine &machine)
{
	konamim2_state *state = machine.driver_data<konamim2_state>();
	switch (state->m_cde_command_bytes[0])
	{
		case 0x04:		// Set Speed
		{
			state->m_cde_num_status_bytes = 1;

			state->m_cde_status_bytes[0] = 0x04;
			state->m_cde_status_byte_ptr = 0;

//          mame_printf_debug("CDE: SET SPEED %02X, %02X\n", state->m_cde_command_bytes[1], state->m_cde_command_bytes[2]);
			break;
		}
		case 0x06:		// Audio Format / Data Format
		{
			state->m_cde_num_status_bytes = 1;

			state->m_cde_status_bytes[0] = 0x06;
			state->m_cde_status_byte_ptr = 0;

			if (state->m_cde_command_bytes[1] == 0x00)		// Audio Format
			{
//              mame_printf_debug("CDE: AUDIO FORMAT\n");
			}
			else if (state->m_cde_command_bytes[1] == 0x78)	// Data Format
			{
//              mame_printf_debug("CDE: DATA FORMAT\n");
			}
			else
			{
				fatalerror("CDE: unknown command %02X, %02X\n", state->m_cde_command_bytes[0], state->m_cde_command_bytes[1]);
			}
			break;
		}
		case 0x08:		// Pause / Eject / Play
		{
			state->m_cde_num_status_bytes = 1;

			state->m_cde_status_bytes[0] = 0x08;
			state->m_cde_status_byte_ptr = 0;

			if (state->m_cde_command_bytes[1] == 0x00)		// Eject
			{
//              mame_printf_debug("CDE: EJECT command\n");
			}
			else if (state->m_cde_command_bytes[1] == 0x02)	// Pause
			{
//              mame_printf_debug("CDE: PAUSE command\n");
				state->m_cde_drive_state = CDE_DRIVE_STATE_PAUSED;
			}
			else if (state->m_cde_command_bytes[1] == 0x03)	// Play
			{
//              mame_printf_debug("CDE: PLAY command\n");
			}
			else
			{
				fatalerror("CDE: unknown command %02X, %02X\n", state->m_cde_command_bytes[0], state->m_cde_command_bytes[1]);
			}
			break;
		}
		case 0x09:		// Seek
		{
			state->m_cde_num_status_bytes = 1;

			state->m_cde_status_bytes[0] = 0x1b;
			state->m_cde_status_byte_ptr = 0;

			state->m_cde_drive_state = CDE_DRIVE_STATE_SEEK_DONE;

//          mame_printf_debug("CDE: SEEK %08X\n", (state->m_cde_command_bytes[1] << 16) | (state->m_cde_command_bytes[2] << 8) | (state->m_cde_command_bytes[3]));
			break;
		}
		case 0x0b:		// Get Drive State
		{
			state->m_cde_num_status_bytes = 0x3;

			state->m_cde_status_bytes[0] = 0x0b;
			state->m_cde_status_bytes[1] = 0x1b;
			state->m_cde_status_bytes[2] = state->m_cde_drive_state;
			state->m_cde_status_byte_ptr = 0;

			if (state->m_cde_command_bytes[1] & 0x02)
			{
				state->m_cde_enable_seek_reports = 1;
			}
			else
			{
				state->m_cde_enable_seek_reports = 0;
			}

//          mame_printf_debug("CDE: GET DRIVE STATE %02X\n", state->m_cde_command_bytes[1]);
			break;
		}
		case 0x0c:		// ?
		{
			state->m_cde_num_status_bytes = 1;

			state->m_cde_status_bytes[0] = 0x0c;
			state->m_cde_status_byte_ptr = 0;

			if (state->m_cde_command_bytes[1] == 0x02)
			{
				state->m_cde_enable_qchannel_reports = 1;
				state->m_cde_drive_state = CDE_DRIVE_STATE_PAUSED;
			}
			else if (state->m_cde_command_bytes[0] == 0x00)
			{
				state->m_cde_enable_qchannel_reports = 0;
			}

//          mame_printf_debug("CDE: UNKNOWN CMD 0x0c %02X\n", state->m_cde_command_bytes[1]);
			break;
		}
		case 0x0d:		// Get Switch State
		{
			state->m_cde_num_status_bytes = 0x4;

			state->m_cde_status_bytes[0] = 0x0d;
			state->m_cde_status_bytes[1] = 0x1d;
			state->m_cde_status_bytes[2] = 0x02;
			state->m_cde_status_byte_ptr = 0;

//          mame_printf_debug("CDE: GET SWITCH STATE %02X\n", state->m_cde_command_bytes[1]);
			break;
		}
		case 0x21:		// Mech type
		{
			state->m_cde_num_status_bytes = 0x8;

			state->m_cde_status_bytes[0] = 0x21;
			state->m_cde_status_bytes[1] = 0xff;
			state->m_cde_status_bytes[2] = 0x08;		// Max Speed
			state->m_cde_status_bytes[3] = 0xff;
			state->m_cde_status_bytes[4] = 0xff;
			state->m_cde_status_bytes[5] = 0xff;
			state->m_cde_status_bytes[6] = 0xff;
			state->m_cde_status_bytes[7] = 0xff;

			state->m_cde_status_byte_ptr = 0;

//          mame_printf_debug("CDE: MECH TYPE %02X, %02X, %02X\n", state->m_cde_command_bytes[1], state->m_cde_command_bytes[2], state->m_cde_command_bytes[3]);
			break;
		}
		case 0x83:		// Read ID
		{
			state->m_cde_num_status_bytes = 0xc;

			state->m_cde_status_bytes[0] = 0x03;
			state->m_cde_status_bytes[1] = 0xff;
			state->m_cde_status_bytes[2] = 0xff;
			state->m_cde_status_bytes[3] = 0xff;
			state->m_cde_status_bytes[4] = 0xff;
			state->m_cde_status_bytes[5] = 0xff;
			state->m_cde_status_bytes[6] = 0xff;
			state->m_cde_status_bytes[7] = 0xff;
			state->m_cde_status_bytes[8] = 0xff;
			state->m_cde_status_bytes[9] = 0xff;
			state->m_cde_status_bytes[10] = 0xff;
			state->m_cde_status_bytes[11] = 0xff;

			state->m_cde_status_byte_ptr = 0;

//          mame_printf_debug("CDE: READ ID\n");
			break;
		}
		default:
		{
			fatalerror("CDE: unknown command %08X\n", state->m_cde_command_bytes[0]);
			break;
		}
	}
}

static void cde_handle_reports(running_machine &machine)
{
	konamim2_state *state = machine.driver_data<konamim2_state>();
	switch (state->m_cde_command_bytes[0])
	{
		case 0x09:
		{
			if (state->m_cde_enable_seek_reports)
			{
				state->m_cde_num_status_bytes = 0x2;
				state->m_cde_status_bytes[0] = 0x02;

				state->m_cde_status_byte_ptr = 0;

				state->m_cde_command_bytes[0] = 0x0c;

//              mame_printf_debug("CDE: SEEK REPORT\n");
			}
			break;
		}

		case 0x0b:
		{
			if (state->m_cde_enable_qchannel_reports)
			{
				int track, num_tracks;

				num_tracks = state->m_cde_toc.numtrks;
				track = state->m_cde_qchannel_offset % (num_tracks+3);

				state->m_cde_num_status_bytes = 0xb;
				state->m_cde_status_bytes[0] = 0x1c;

				/*
                state->m_cde_status_bytes[1] = 0x0;      // q-Mode
                state->m_cde_status_bytes[2] = 0x0;      // TNO
                state->m_cde_status_bytes[3] = 0x0;      // Index / Pointer
                state->m_cde_status_bytes[4] = 0x0;      // Min
                state->m_cde_status_bytes[5] = 0x0;      // Sec
                state->m_cde_status_bytes[6] = 0x0;      // Frac
                state->m_cde_status_bytes[7] = 0x0;      // Zero
                state->m_cde_status_bytes[8] = 0x0;      // A-Min
                state->m_cde_status_bytes[9] = 0x0;      // A-Sec
                state->m_cde_status_bytes[10] = 0x0;     // A-Frac
                */

				if (track < num_tracks)
				{
					int time = lba_to_msf(state->m_cde_toc.tracks[track].physframeofs);

					state->m_cde_status_bytes[1] = 0x41;					// q-Mode
					state->m_cde_status_bytes[2] = 0x0;					// TNO (Lead-in track)
					state->m_cde_status_bytes[3] = track+1;				// Pointer
					state->m_cde_status_bytes[4] = 0x0;					// Min
					state->m_cde_status_bytes[5] = 0x0;					// Sec
					state->m_cde_status_bytes[6] = 0x0;					// Frac
					state->m_cde_status_bytes[7] = 0x0;					// Zero
					state->m_cde_status_bytes[8] = (time >> 16) & 0xff;	// P-Min
					state->m_cde_status_bytes[9] = (time >>  8) & 0xff;	// P-Sec
					state->m_cde_status_bytes[10] = time & 0xff;			// P-Frac
				}
				else
				{
					if (track == num_tracks+0)
					{
						state->m_cde_status_bytes[1] = 0x41;					// q-Mode / Control
						state->m_cde_status_bytes[2] = 0x0;					// TNO (Lead-in track)
						state->m_cde_status_bytes[3] = 0xa0;					// Pointer
						state->m_cde_status_bytes[4] = 0x0;					// Min
						state->m_cde_status_bytes[5] = 0x0;					// Sec
						state->m_cde_status_bytes[6] = 0x0;					// Frac
						state->m_cde_status_bytes[7] = 0x0;					// Zero
						state->m_cde_status_bytes[8] = 1;					// P-Min
						state->m_cde_status_bytes[9] = 0x0;					// P-Sec
						state->m_cde_status_bytes[10] = 0x0;					// P-Frac
					}
					else if (track == num_tracks+1)
					{
						state->m_cde_status_bytes[1] = 0x41;					// q-Mode / Control
						state->m_cde_status_bytes[2] = 0x0;					// TNO (Lead-in track)
						state->m_cde_status_bytes[3] = 0xa1;					// Pointer
						state->m_cde_status_bytes[4] = 0x0;					// Min
						state->m_cde_status_bytes[5] = 0x0;					// Sec
						state->m_cde_status_bytes[6] = 0x0;					// Frac
						state->m_cde_status_bytes[7] = 0x0;					// Zero
						state->m_cde_status_bytes[8] = num_tracks;			// P-Min
						state->m_cde_status_bytes[9] = 0x0;					// P-Sec
						state->m_cde_status_bytes[10] = 0x0;					// P-Frac
					}
					else
					{
						int leadout_lba = state->m_cde_toc.tracks[num_tracks-1].physframeofs + state->m_cde_toc.tracks[num_tracks-1].frames;
						int leadout_time = lba_to_msf(leadout_lba);

						state->m_cde_status_bytes[1] = 0x41;					// q-Mode / Control
						state->m_cde_status_bytes[2] = 0x0;					// TNO (Lead-in track)
						state->m_cde_status_bytes[3] = 0xa2;					// Pointer
						state->m_cde_status_bytes[4] = 0x0;					// Min
						state->m_cde_status_bytes[5] = 0x0;					// Sec
						state->m_cde_status_bytes[6] = 0x0;					// Frac
						state->m_cde_status_bytes[7] = 0x0;					// Zero
						state->m_cde_status_bytes[8] = (leadout_time >> 16) & 0xff;	// P-Min
						state->m_cde_status_bytes[9] = (leadout_time >>  8) & 0xff;	// P-Sec
						state->m_cde_status_bytes[10] = leadout_time & 0xff;			// P-Frac
					}
				}

				state->m_cde_qchannel_offset++;

				state->m_cde_status_byte_ptr = 0;
				state->m_cde_command_bytes[0] = 0x0c;

//              mame_printf_debug("CDE: QCHANNEL REPORT\n");
				break;
			}
		}
	}
}

static void cde_dma_transfer(address_space *space, int channel, int next)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	UINT32 address;
	//int length;
	int i;

	if (next)
	{
		address = state->m_cde_dma[channel].next_dst_addr;
		//length = state->m_cde_dma[channel].next_length;
	}
	else
	{
		address = state->m_cde_dma[channel].dst_addr;
		//length = state->m_cde_dma[channel].length;
	}

	for (i=0; i < state->m_cde_dma[channel].next_length; i++)
	{
		space->write_byte(address, 0xff);		// TODO: do the real transfer...
		address++;
	}
}

static READ64_HANDLER(cde_r)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	UINT32 r = 0;
	int reg = offset * 2;

	if (ACCESSING_BITS_0_31)
		reg++;

	switch (reg)
	{
		case 0x000/4:
		{
			r = (0x01) << 16;	// Device identifier, 1 = CDE
			break;
		}
		case 0x018/4:
		{
			r = 0x100038;

			r |= state->m_cde_dma[0].dma_done ? 0x400 : 0;
			r |= state->m_cde_dma[1].dma_done ? 0x800 : 0;
			break;
		}
		case 0x02c/4:
		{
			r = state->m_cde_status_bytes[state->m_cde_status_byte_ptr++];

			if (state->m_cde_status_byte_ptr <= state->m_cde_num_status_bytes)
			{
				r |= 0x100;
			}
			else
			{
				//if (cde_enable_reports &&
				//  !state->m_cde_response &&
				//  state->m_cde_command_bytes[0] != ((cde_report_type >> 8) & 0xff))

				if (!state->m_cde_response)
				{
					cde_handle_reports(space->machine());

			//      state->m_cde_command_byte_ptr = 0;
			//      state->m_cde_command_bytes[state->m_cde_command_byte_ptr++] = 0x1c;

			//      state->m_cde_response = 1;
				}
			}

	//      printf("status byte %d\n", state->m_cde_status_byte_ptr);
			break;
		}

		case 0x2a0/4:
		{
			r = 0x20;
			break;
		}

		default:
		{
//                      mame_printf_debug("cde_r: %08X at %08X\n", reg*4, cpu_get_pc(&space->device()));
			break;
		}
	}

	if (reg & 1)
	{
		return (UINT64)(r);
	}
	else
	{
		return (UINT64)(r) << 32;
	}
}

static WRITE64_HANDLER(cde_w)
{
	konamim2_state *state = space->machine().driver_data<konamim2_state>();
	int reg = offset * 2;
	UINT32 d;

	if (ACCESSING_BITS_0_31)
	{
		reg++;
		d = (UINT32)(data);
	}
	else
	{
		d = (UINT32)(data >> 32);
	}

	switch (reg)
	{
		case 0x028/4:		// Command write
		{
			//printf("cde_w: %08X, %08X at %08X\n", d, reg*4, cpu_get_pc(&space->device()));

			if (d == 0x0180)
			{
				if (state->m_cde_response)
				{
					cde_handle_command(space->machine());

					state->m_cde_response = 0;
				}

				state->m_cde_command_byte_ptr = 0;
			}
			else if (state->m_cde_command_byte_ptr == 0)
			{
				state->m_cde_num_status_bytes = 1;

				state->m_cde_status_bytes[0] = d & 0xff;
				state->m_cde_status_byte_ptr = 0;

				state->m_cde_response = 1;
			}

			if (d != 0x180)
			{
				state->m_cde_command_bytes[state->m_cde_command_byte_ptr++] = d;
			}

			break;
		}

		case 0x300/4:		// DMA Channel 0 enable
		{
//          mame_printf_debug("CDE: DMA0 enable %08X\n", d);

			if (d & 0x20)
			{
				state->m_cde_dma[0].dma_done = 1;

				cde_dma_transfer(space, 0, 0);
			}
			if (d & 0x40)
			{
				state->m_cde_dma[0].dma_done = 1;

				cde_dma_transfer(space, 0, 1);
			}
			break;
		}
		case 0x308/4:		// DMA Channel 0 destination address
		{
//          mame_printf_debug("CDE: DMA0 dst addr %08X\n", d);

			state->m_cde_dma[0].dst_addr = d;
			break;
		}
		case 0x30c/4:		// DMA Channel 0 length?
		{
//          mame_printf_debug("CDE: DMA0 length %08X\n", d);

			state->m_cde_dma[0].length = d;
			break;
		}
		case 0x318/4:		// DMA Channel 0 next destination address
		{
//          mame_printf_debug("CDE: DMA0 next dst addr %08X\n", d);

			state->m_cde_dma[0].next_dst_addr = d;
			break;
		}
		case 0x31c/4:		// DMA Channel 0 next length?
		{
//          mame_printf_debug("CDE: DMA0 next length %08X\n", d);

			state->m_cde_dma[0].next_length = d;
			break;
		}

		case 0x320/4:		// DMA Channel 1 enable
		{
//          mame_printf_debug("CDE: DMA1 enable %08X\n", d);
			break;
		}
		case 0x328/4:		// DMA Channel 1 destination address
		{
//          mame_printf_debug("CDE: DMA1 dst addr %08X\n", d);

			state->m_cde_dma[1].dst_addr = d;
			break;
		}
		case 0x32c/4:		// DMA Channel 1 length?
		{
//          mame_printf_debug("CDE: DMA1 length %08X\n", d);

			state->m_cde_dma[1].length = d;
			break;
		}
		case 0x338/4:		// DMA Channel 1 next destination address
		{
//          mame_printf_debug("CDE: DMA1 next dst addr %08X\n", d);

			state->m_cde_dma[1].next_dst_addr = d;
			break;
		}
		case 0x33c/4:		// DMA Channel 1 next length?
		{
//          mame_printf_debug("CDE: DMA1 next length %08X\n", d);

			state->m_cde_dma[1].next_length = d;
			break;
		}

		case 0x418/4:		// ???
		{
			if (d & 0x80000000)
			{
				state->m_irq_active &= ~0x8;
			}
			if (d & 0x60000000)
			{
				state->m_cde_dma[0].dma_done = 0;
				state->m_cde_dma[1].dma_done = 0;
			}
			break;
		}

		default:
		{
//                      mame_printf_debug("cde_w: %08X, %08X at %08X\n", d, reg*4, cpu_get_pc(&space->device()));
			break;
		}
	}
}

static READ64_HANDLER(device2_r)
{
	UINT32 r = 0;
	int reg = offset * 2;

	if (ACCESSING_BITS_0_31)
		reg++;

	switch (reg)
	{
		case 0x000/4:
		{
			r = (0x02) << 16;	// Device identifier
			break;
		}
		default:
		{
			break;
		}
	}

	if (reg & 1)
	{
		return (UINT64)(r);
	}
	else
	{
		return (UINT64)(r) << 32;
	}
}

static READ64_HANDLER(cpu_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_32_63)
	{
		r = (UINT64)((&space->device() != space->machine().device("maincpu")) ? 0x80000000 : 0);
		//r |= 0x40000000;  // sets Video-LowRes !?
		return r << 32;
	}

	return 0;
}

static ADDRESS_MAP_START( m2_main, AS_PROGRAM, 64 )
	AM_RANGE(0x00010040, 0x00010047) AM_READWRITE(irq_enable_r, irq_enable_w)
	AM_RANGE(0x00010050, 0x00010057) AM_READ(irq_active_r)
	AM_RANGE(0x00020000, 0x00020007) AM_READWRITE(unk4_r, unk4_w)
	AM_RANGE(0x00030000, 0x00030007) AM_READ(unk30000_r)
	AM_RANGE(0x00030010, 0x00030017) AM_WRITE(video_w)
	AM_RANGE(0x00030030, 0x00030037) AM_READ(unk30030_r)
	AM_RANGE(0x00030400, 0x00030407) AM_WRITE(video_irq_ack_w)
	AM_RANGE(0x01000000, 0x01000fff) AM_READWRITE(cde_r, cde_w)
	AM_RANGE(0x02000000, 0x02000fff) AM_READ(device2_r)
	AM_RANGE(0x04000010, 0x04000017) AM_WRITE(unk4000010_w)
	AM_RANGE(0x04000018, 0x0400001f) AM_READ(unk1_r)
	AM_RANGE(0x04000020, 0x04000027) AM_WRITE(reset_w)
	AM_RANGE(0x04000418, 0x0400041f) AM_WRITE(unk4000418_w)
	AM_RANGE(0x04000208, 0x0400020f) AM_READ(unk3_r)
	AM_RANGE(0x04000280, 0x04000287) AM_READ(unk4000280_r)
	AM_RANGE(0x10000000, 0x10000007) AM_READ(cpu_r)
	AM_RANGE(0x10000008, 0x10001007) AM_NOP		// ???
	AM_RANGE(0x20000000, 0x201fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x40000000, 0x407fffff) AM_RAM AM_SHARE("share3") AM_BASE_MEMBER(konamim2_state, m_main_ram)
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("boot", 0) AM_SHARE("share2")
ADDRESS_MAP_END

static INPUT_PORTS_START( m2 )
INPUT_PORTS_END


static const powerpc_config ppc602_config =
{
	33000000			/* Multiplier 2, Bus = 33MHz, Core = 66MHz */
};

static INTERRUPT_GEN(m2)
{
	konamim2_state *state = device->machine().driver_data<konamim2_state>();
	if (state->m_irq_enable & 0x800000)
	{
		state->m_irq_active |= 0x800000;
	}

	/*if (state->m_irq_enable & 0x8)
    {
        state->m_irq_active |= 0x8;
    }*/

	device_set_input_line(device, INPUT_LINE_IRQ0, ASSERT_LINE);
}

static MACHINE_CONFIG_START( m2, konamim2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC602, 33000000)	/* actually PPC602, 66MHz */
	MCFG_CPU_CONFIG(ppc602_config)
	MCFG_CPU_PROGRAM_MAP(m2_main)
	MCFG_CPU_VBLANK_INT("screen", m2)

	MCFG_CPU_ADD("sub", PPC602, 33000000)	/* actually PPC602, 66MHz */
	MCFG_CPU_CONFIG(ppc602_config)
	MCFG_CPU_PROGRAM_MAP(m2_main)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE(m2)

	MCFG_PALETTE_LENGTH(32768)
	MCFG_PALETTE_INIT(RRRRR_GGGGG_BBBBB)

	MCFG_VIDEO_START(m2)

MACHINE_CONFIG_END


ROM_START( polystar )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.7k",  0x000000, 0x000080, CRC(66d02984) SHA1(d07c57d198c611b6ff67a783c20a3d038ba34cd1) )

	DISK_REGION( "cdrom" ) // TODO: Add correct CHD
	DISK_IMAGE( "623jaa02", 0, BAD_DUMP SHA1(cd542931cdac95d05344d34421e223dd6e371bb5) )
ROM_END

ROM_START( btltryst )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "btltryst", 0, SHA1(c76326b0a0fcfe696a2ca019170d3abde40e773e) )
ROM_END

ROM_START( heatof11 )
	ROM_REGION64_BE( 0x200000, "boot", 0 )	/* boot rom */
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION( 0x2000, "timekeep", 0 ) /* timekeeper SRAM */
	ROM_LOAD( "dallas.5e",  0x000000, 0x002000, CRC(8611ff09) SHA1(6410236947d99c552c4a1f7dd5fd8c7a5ae4cba1) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "heatof11", 0, SHA1(5a0a2782cd8676d3f6dfad4e0f805b309e230d8b) )
ROM_END

ROM_START( evilngt )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION( 0x2000, "timekeep", 0 ) /* timekeeper SRAM */
	ROM_LOAD( "m48t58y.9n",   0x000000, 0x002000, CRC(e887ca1f) SHA1(54205f01b1ceba1d5f4d979fc30be1add8116e90) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "810a03.16h",  0x000000, 0x400000, CRC(4cd79d98) SHA1(12fea41cfc5c1b883ffbeda7e428dd1d1bf54d7f) )

	ROM_REGION( 0x80, "eeprom", 0 )	/* EEPROM default contents */
	ROM_LOAD( "93c46.7k",    0x000000, 0x000080, CRC(d7ba2e5e) SHA1(d729557555c6fc1cd433b14017952cc63ec73573) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "810uba02", 0, SHA1(ca3b957e3e33435391fbf630f9ed89aa08a70c8f) )
ROM_END

ROM_START( evilngte )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION( 0x1000, "timekeep", 0 ) /* timekeeper SRAM */
	ROM_LOAD( "m48t58y.u1",  0x000000, 0x001000, CRC(169bb8f4) SHA1(55c0bafab5d309fe69156489186e232aa87ca0dd) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "810a03.16h",  0x000000, 0x400000, CRC(4cd79d98) SHA1(12fea41cfc5c1b883ffbeda7e428dd1d1bf54d7f) )

	// TODO: Add CHD
ROM_END

ROM_START( hellngt )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION( 0x1000, "timekeep", 0 ) /* timekeeper SRAM */
	ROM_LOAD( "m48t58y.u1",  0x000000, 0x001000, CRC(169bb8f4) SHA1(55c0bafab5d309fe69156489186e232aa87ca0dd) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "810a03.16h",  0x000000, 0x400000, CRC(4cd79d98) SHA1(12fea41cfc5c1b883ffbeda7e428dd1d1bf54d7f) )

	DISK_REGION( "cdrom" ) // TODO: Add correct CHD
	DISK_IMAGE( "810eaa02", 0, BAD_DUMP SHA1(c407bad498cb87788ce332dbef5e8c6e19c1fd16) )
ROM_END

ROM_START( totlvice )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "639uac01", 0, SHA1(88431b8a0ce83c156c8b19efbba1af901b859404) )
ROM_END

ROM_START( totlvicj )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "cdrom" ) // Need a re-image
	DISK_IMAGE( "639jad01", 0, BAD_DUMP SHA1(39d41d5a9d1c40636d174c8bb8172b1121e313f8) )
ROM_END

ROM_START(3do_m2)
	ROM_REGION64_BE( 0x100000, "boot", 0 )
	ROM_SYSTEM_BIOS( 0, "panafz35", "Panasonic FZ-35S (3DO M2)" )
	ROMX_LOAD( "fz35_jpn.bin", 0x000000, 0x100000, CRC(e1c5bfd3) SHA1(0a3e27d672be79eeee1d2dc2da60d82f6eba7934), ROM_BIOS(1) )
ROM_END

static DRIVER_INIT( m2 )
{
	konamim2_state *state = machine.driver_data<konamim2_state>();
	state->m_unk3 = U64(0xffffffffffffffff);
	state->m_unk20004 = 0;
	cde_init(machine);
}

GAME( 1997, polystar, 0,        m2, m2, m2, ROT0, "Konami", "Tobe! Polystars (ver JAA)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1997, totlvice, 0,        m2, m2, m2, ROT0, "Konami", "Total Vice (ver UAC)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1997, totlvicj, totlvice, m2, m2, m2, ROT0, "Konami", "Total Vice (ver JAD)", GAME_NOT_WORKING | GAME_NO_SOUND )
//GAME( 1997, totlvica, totlvice, m2, m2, m2, ROT0, "Konami", "Total Vice (ver AAB)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1998, btltryst, 0,        m2, m2, m2, ROT0, "Konami", "Battle Tryst (ver JAC)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1998, heatof11, 0,        m2, m2, m2, ROT0, "Konami", "Heat of Eleven '98 (ver EAA)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1998, evilngt, 0,         m2, m2, m2, ROT0, "Konami", "Evil Night (ver UBA)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1998, evilngte,  evilngt,       m2, m2, m2, ROT0, "Konami", "Evil Night (ver EAA)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1998, hellngt,  evilngt,  m2, m2, m2, ROT0, "Konami", "Hell Night (ver EAA)", GAME_NOT_WORKING | GAME_NO_SOUND )

CONS( 199?, 3do_m2,     0,      0,    m2,    m2,	0,      "3DO",  "3DO M2",   	GAME_NOT_WORKING | GAME_NO_SOUND )
