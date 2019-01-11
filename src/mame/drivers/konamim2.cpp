// license:BSD-3-Clause
// copyright-holders:Ville Linde
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
#include "imagedev/chd_cd.h"
#include "machine/terminal.h"
#include "softlist.h"
#include "screen.h"

struct CDE_DMA
{
	uint32_t dst_addr;
	int length;
	uint32_t next_dst_addr;
	int next_length;
	int dma_done;
};

class konamim2_state : public driver_device
{
public:
	konamim2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_main_ram(*this, "main_ram"),
		m_terminal(*this, "terminal"),
		m_in_country(*this, "COUNTRY"),
		m_in_card(*this, "CARD"),
		m_in_monitor(*this, "MONITOR"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub") { }

	required_shared_ptr<uint64_t> m_main_ram;
	required_device<generic_terminal_device> m_terminal;
	required_ioport m_in_country;
	required_ioport m_in_card;
	required_ioport m_in_monitor;

	uint32_t m_vdl0_address;
	uint32_t m_vdl1_address;
	uint32_t m_irq_enable;
	uint32_t m_irq_active;
	uint64_t m_unk3;
	uint32_t m_unk20004;
	int m_counter1;
	int m_cde_num_status_bytes;
	uint32_t m_cde_status_bytes[16];
	int m_cde_status_byte_ptr;
	uint32_t m_cde_command_bytes[16];
	int m_cde_command_byte_ptr;
	int m_cde_response;
	int m_cde_drive_state;
	int m_cde_enable_qchannel_reports;
	int m_cde_enable_seek_reports;
	int m_cde_qchannel_offset;
	cdrom_toc m_cde_toc;
	CDE_DMA m_cde_dma[2];
	DECLARE_READ64_MEMBER(irq_enable_r);
	DECLARE_WRITE64_MEMBER(irq_enable_w);
	DECLARE_READ64_MEMBER(irq_active_r);
	DECLARE_READ64_MEMBER(unk1_r);
	DECLARE_READ64_MEMBER(unk3_r);
	DECLARE_READ64_MEMBER(unk4_r);
	DECLARE_WRITE64_MEMBER(unk4_w);
	DECLARE_READ64_MEMBER(unk30000_r);
	DECLARE_READ64_MEMBER(unk30030_r);
	DECLARE_WRITE64_MEMBER(video_w);
	DECLARE_WRITE32_MEMBER(video_irq_ack_w);
	DECLARE_READ64_MEMBER(unk4000280_r);
	DECLARE_WRITE8_MEMBER(serial_w);
	DECLARE_WRITE64_MEMBER(unk4000418_w);
	DECLARE_WRITE64_MEMBER(reset_w);
	DECLARE_READ64_MEMBER(cde_r);
	DECLARE_WRITE64_MEMBER(cde_w);
	DECLARE_READ64_MEMBER(device2_r);
	template<bool maincpu> DECLARE_READ64_MEMBER(cpu_r);
	DECLARE_READ8_MEMBER(id3_r);
	DECLARE_READ8_MEMBER(id4_r);
	DECLARE_READ8_MEMBER(id5_r);
	DECLARE_READ8_MEMBER(id6_r);
	DECLARE_READ8_MEMBER(id7_r);

	void init_m2();
	virtual void video_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_m2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(m2);
	void cde_init();
	void cde_handle_command();
	void cde_handle_reports();
	void cde_dma_transfer(address_space &space, int channel, int next);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	void m2(machine_config &config);
	void _3do_m2(machine_config &config);
	void _3do_m2_main(address_map &map);
	void _3do_m2_main_m(address_map &map);
	void _3do_m2_main_s(address_map &map);
	void m2_main(address_map &map);
	void m2_main_m(address_map &map);
	void m2_main_s(address_map &map);
};


void konamim2_state::video_start()
{
}

uint32_t konamim2_state::screen_update_m2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, j;
	uint32_t fb_start = 0xffffffff;
	uint32_t fb_size;
	//uint32_t config;
	int height = 384,width = 512;

	if (m_vdl0_address != 0)
	{
		uint32_t cur_vdl_address = screen.frame_number() & 1 ? m_vdl0_address : m_vdl1_address;
		// TODO: this looks more likely to be a framebuffer copy, with parameters!
		fb_start = *(uint32_t*)&m_main_ram[(cur_vdl_address - 0x40000000) / 8] - 0x40000000;
		fb_size = m_main_ram[((cur_vdl_address - 0x40000000) / 8) + 2] >> 32;
		//config = m_main_ram[(cur_vdl_address - 0x40000000) / 8] >> 32;
		//popmessage("%08x",config);

		height = fb_size & 0x1ff;
		width = (fb_size >> 24) * 16;
	}

	if (fb_start <= 0x800000)
	{
		uint16_t *frame = (uint16_t*)&m_main_ram[fb_start/8];
		for (j=0; j < height; j++)
		{
			uint16_t *fb = &frame[(j*width)];
			uint16_t *d = &bitmap.pix16(j);
			for (i=0; i < width; i++)
			{
				d[i^3] = *fb++ & 0x7fff;
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}
	return 0;
}

READ64_MEMBER(konamim2_state::irq_enable_r)
{
	uint64_t r = 0;

	if (ACCESSING_BITS_32_63)
	{
		r |= (uint64_t)(m_irq_enable) << 32;
	}

	return r;
}

WRITE64_MEMBER(konamim2_state::irq_enable_w)
{
	if (ACCESSING_BITS_32_63)
	{
		m_irq_enable |= (uint32_t)(data >> 32);
	}
}

READ64_MEMBER(konamim2_state::irq_active_r)
{
	uint64_t r = 0;

	if (ACCESSING_BITS_32_63)
	{
		r |= (uint64_t)(m_irq_active) << 32;
	}

	return r;
}



READ64_MEMBER(konamim2_state::unk1_r)
{
	return 0xffffffffffffffffU;
	//return 0;
}

#ifdef UNUSED_FUNCTION
READ64_MEMBER(konamim2_state::unk2_r)
{
	if (ACCESSING_BITS_32_63)
	{
		return (uint64_t)0xa5 << 32;
	}
	return 0;
}
#endif

READ64_MEMBER(konamim2_state::unk3_r)
{
	//return 0xffffffffffffffffU;
	return m_unk3;
}

READ64_MEMBER(konamim2_state::unk4_r)
{
	uint64_t r = 0;
//  logerror("unk4_r: %08X, %08X%08X %s\n", offset, (uint32_t)(mem_mask>>32), (uint32_t)(mem_mask), machine().describe_context());

	if (ACCESSING_BITS_32_63)
	{
		// MCfg
		r |= (uint64_t)((0 << 13) | (5 << 10)) << 32;
	}
	if (ACCESSING_BITS_0_31)
	{
		r |= m_unk20004 & ~0x800000;
	}
	return r;
}

WRITE64_MEMBER(konamim2_state::unk4_w)
{
//  logerror("unk4_w: %08X%08X, %08X, %08X%08X %s\n", (uint32_t)(data >> 32), (uint32_t)(data),
//      offset, (uint32_t)(mem_mask>>32), (uint32_t)(mem_mask), machine().describe_context());

	if (ACCESSING_BITS_0_31)
	{
		if (data & 0x800000)
		{
			m_subcpu->set_input_line(PPC_IRQ, ASSERT_LINE);
		}

		m_unk20004 = (uint32_t)(data);
		return;
	}
}

READ64_MEMBER(konamim2_state::unk30000_r)
{
	m_counter1++;
	return (uint64_t)(m_counter1 & 0x7f) << 32;
}

READ64_MEMBER(konamim2_state::unk30030_r)
{
	if (ACCESSING_BITS_0_31)
	{
		return 1;
	}
	return 0;
}

WRITE64_MEMBER(konamim2_state::video_w)
{
	if (ACCESSING_BITS_32_63)
	{
		m_vdl0_address = (uint32_t)(data >> 32);
	}
	if (ACCESSING_BITS_0_31)
	{
		m_vdl1_address = (uint32_t)(data);
	}
}

WRITE32_MEMBER(konamim2_state::video_irq_ack_w)
{
	if (data & 0x8000)
	{
		m_irq_active &= ~0x800000;
		m_maincpu->set_input_line(PPC_IRQ, CLEAR_LINE);
	}
}



READ64_MEMBER(konamim2_state::unk4000280_r)
{
	// SysCfg

	uint32_t sys_config = 0x03600000;

	sys_config |= 0 << 0;  // Bit 0:       PAL/NTSC switch (default is selected by encoder)
	sys_config |= 0 << 2;                         // Bit 2-3:     Video Encoder (0 = MEIENC, 1 = VP536, 2 = BT9103, 3 = DENC)
	sys_config |= m_in_country->read() << 11;     // Bit 11-12:   Country
									//              0 = ???
									//              1 = UK
									//              2 = Japan
									//              3 = US
	sys_config |= m_in_card->read() << 15;        // Bit 15-18:   0x8 = AC-DevCard
									//              0xb = AC-CoreBoard
									//              0xc = DevCard (not allowed)
									//              0xe = Upgrade (not allowed)
									//              0xf = Multiplayer (not allowed)
	sys_config |= 3 << 29;          // Bit 29-30:   Audio chip (1 = CS4216, 3 = Asahi AK4309)

	return ((uint64_t)(sys_config) << 32);

}

WRITE8_MEMBER(konamim2_state::serial_w)
{
	m_terminal->write(space,0,data & 0xff);
}

WRITE64_MEMBER(konamim2_state::unk4000418_w)
{
}

WRITE64_MEMBER(konamim2_state::reset_w)
{
	if (ACCESSING_BITS_32_63)
	{
		if (data & 0x100000000U)
		{
			m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			m_unk3 = 0;
		}
	}
}


/*****************************************************************************/
/* CDE */

#define CDE_DRIVE_STATE_PAUSED          0x02
#define CDE_DRIVE_STATE_SEEK_DONE       0x03









void konamim2_state::cde_init()
{
	cdrom_file *cdfile = cdrom_open(machine().rom_load().get_disk_handle(":cdrom"));

	const cdrom_toc *toc = cdrom_get_toc(cdfile);

	if (cdfile)
	{
		memcpy(&m_cde_toc, toc, sizeof(cdrom_toc));

		/*
		printf("%d tracks\n", toc->numtrks);
		for (int i=0; i < toc->numtrks; i++)
		{
		    const cdrom_track_info *track = &toc->tracks[i];
		    printf("Track %d: type %d, subtype %d, datasize %d, subsize %d, frames %d, extraframes %d, physframeofs %d\n",
		            i, track->trktype, track->subtype, track->datasize, track->subsize,track->frames, track->extraframes, track->physframeofs);
		}
		*/

		cdrom_close(cdfile);
	}

	m_cde_drive_state = CDE_DRIVE_STATE_PAUSED;

	m_cde_num_status_bytes = 0;
	m_cde_status_byte_ptr = 0;
	m_cde_command_byte_ptr = 0;

	m_cde_response = 0;

	m_cde_enable_qchannel_reports = 0;
	m_cde_enable_seek_reports = 0;

	m_cde_qchannel_offset = 0;
}

void konamim2_state::cde_handle_command()
{
	switch (m_cde_command_bytes[0])
	{
		case 0x04:      // Set Speed
		{
			m_cde_num_status_bytes = 1;

			m_cde_status_bytes[0] = 0x04;
			m_cde_status_byte_ptr = 0;

//          osd_printf_debug("CDE: SET SPEED %02X, %02X\n", m_cde_command_bytes[1], m_cde_command_bytes[2]);
			break;
		}
		case 0x06:      // Audio Format / Data Format
		{
			m_cde_num_status_bytes = 1;

			m_cde_status_bytes[0] = 0x06;
			m_cde_status_byte_ptr = 0;

			if (m_cde_command_bytes[1] == 0x00)      // Audio Format
			{
//              osd_printf_debug("CDE: AUDIO FORMAT\n");
			}
			else if (m_cde_command_bytes[1] == 0x78) // Data Format
			{
//              osd_printf_debug("CDE: DATA FORMAT\n");
			}
			else
			{
				fatalerror("CDE: unknown command %02X, %02X\n", m_cde_command_bytes[0], m_cde_command_bytes[1]);
			}
			break;
		}
		case 0x08:      // Pause / Eject / Play
		{
			m_cde_num_status_bytes = 1;

			m_cde_status_bytes[0] = 0x08;
			m_cde_status_byte_ptr = 0;

			if (m_cde_command_bytes[1] == 0x00)      // Eject
			{
//              osd_printf_debug("CDE: EJECT command\n");
			}
			else if (m_cde_command_bytes[1] == 0x02) // Pause
			{
//              osd_printf_debug("CDE: PAUSE command\n");
				m_cde_drive_state = CDE_DRIVE_STATE_PAUSED;
			}
			else if (m_cde_command_bytes[1] == 0x03) // Play
			{
//              osd_printf_debug("CDE: PLAY command\n");
			}
			else
			{
				fatalerror("CDE: unknown command %02X, %02X\n", m_cde_command_bytes[0], m_cde_command_bytes[1]);
			}
			break;
		}
		case 0x09:      // Seek
		{
			m_cde_num_status_bytes = 1;

			m_cde_status_bytes[0] = 0x1b;
			m_cde_status_byte_ptr = 0;

			m_cde_drive_state = CDE_DRIVE_STATE_SEEK_DONE;

//          osd_printf_debug("CDE: SEEK %08X\n", (m_cde_command_bytes[1] << 16) | (m_cde_command_bytes[2] << 8) | (m_cde_command_bytes[3]));
			break;
		}
		case 0x0b:      // Get Drive State
		{
			m_cde_num_status_bytes = 0x3;

			m_cde_status_bytes[0] = 0x0b;
			m_cde_status_bytes[1] = 0x1b;
			m_cde_status_bytes[2] = m_cde_drive_state;
			m_cde_status_byte_ptr = 0;

			if (m_cde_command_bytes[1] & 0x02)
			{
				m_cde_enable_seek_reports = 1;
			}
			else
			{
				m_cde_enable_seek_reports = 0;
			}

//          osd_printf_debug("CDE: GET DRIVE STATE %02X\n", m_cde_command_bytes[1]);
			break;
		}
		case 0x0c:      // ?
		{
			m_cde_num_status_bytes = 1;

			m_cde_status_bytes[0] = 0x0c;
			m_cde_status_byte_ptr = 0;

			if (m_cde_command_bytes[1] == 0x02)
			{
				m_cde_enable_qchannel_reports = 1;
				m_cde_drive_state = CDE_DRIVE_STATE_PAUSED;
			}
			else if (m_cde_command_bytes[0] == 0x00)
			{
				m_cde_enable_qchannel_reports = 0;
			}

//          osd_printf_debug("CDE: UNKNOWN CMD 0x0c %02X\n", m_cde_command_bytes[1]);
			break;
		}
		case 0x0d:      // Get Switch State
		{
			m_cde_num_status_bytes = 0x4;

			m_cde_status_bytes[0] = 0x0d;
			m_cde_status_bytes[1] = 0x1d;
			m_cde_status_bytes[2] = 0x02;
			m_cde_status_byte_ptr = 0;

//          osd_printf_debug("CDE: GET SWITCH STATE %02X\n", m_cde_command_bytes[1]);
			break;
		}
		case 0x21:      // Mech type
		{
			m_cde_num_status_bytes = 0x8;

			m_cde_status_bytes[0] = 0x21;
			m_cde_status_bytes[1] = 0xff;
			m_cde_status_bytes[2] = 0x08;        // Max Speed
			m_cde_status_bytes[3] = 0xff;
			m_cde_status_bytes[4] = 0xff;
			m_cde_status_bytes[5] = 0xff;
			m_cde_status_bytes[6] = 0xff;
			m_cde_status_bytes[7] = 0xff;

			m_cde_status_byte_ptr = 0;

//          osd_printf_debug("CDE: MECH TYPE %02X, %02X, %02X\n", m_cde_command_bytes[1], m_cde_command_bytes[2], m_cde_command_bytes[3]);
			break;
		}
		case 0x83:      // Read ID
		{
			m_cde_num_status_bytes = 0xc;

			m_cde_status_bytes[0] = 0x03;
			m_cde_status_bytes[1] = 0xff;
			m_cde_status_bytes[2] = 0xff;
			m_cde_status_bytes[3] = 0xff;
			m_cde_status_bytes[4] = 0xff;
			m_cde_status_bytes[5] = 0xff;
			m_cde_status_bytes[6] = 0xff;
			m_cde_status_bytes[7] = 0xff;
			m_cde_status_bytes[8] = 0xff;
			m_cde_status_bytes[9] = 0xff;
			m_cde_status_bytes[10] = 0xff;
			m_cde_status_bytes[11] = 0xff;

			m_cde_status_byte_ptr = 0;

//          osd_printf_debug("CDE: READ ID\n");
			break;
		}
		default:
		{
			fatalerror("CDE: unknown command %08X\n", m_cde_command_bytes[0]);
		}
	}
}

void konamim2_state::cde_handle_reports()
{
	switch (m_cde_command_bytes[0])
	{
		case 0x09:
		{
			if (m_cde_enable_seek_reports)
			{
				m_cde_num_status_bytes = 0x2;
				m_cde_status_bytes[0] = 0x02;

				m_cde_status_byte_ptr = 0;

				m_cde_command_bytes[0] = 0x0c;

//              osd_printf_debug("CDE: SEEK REPORT\n");
			}
			break;
		}

		case 0x0b:
		{
			if (m_cde_enable_qchannel_reports)
			{
				int track, num_tracks;

				num_tracks = m_cde_toc.numtrks;
				track = m_cde_qchannel_offset % (num_tracks+3);

				m_cde_num_status_bytes = 0xb;
				m_cde_status_bytes[0] = 0x1c;

				/*
				m_cde_status_bytes[1] = 0x0;      // q-Mode
				m_cde_status_bytes[2] = 0x0;      // TNO
				m_cde_status_bytes[3] = 0x0;      // Index / Pointer
				m_cde_status_bytes[4] = 0x0;      // Min
				m_cde_status_bytes[5] = 0x0;      // Sec
				m_cde_status_bytes[6] = 0x0;      // Frac
				m_cde_status_bytes[7] = 0x0;      // Zero
				m_cde_status_bytes[8] = 0x0;      // A-Min
				m_cde_status_bytes[9] = 0x0;      // A-Sec
				m_cde_status_bytes[10] = 0x0;     // A-Frac
				*/

				if (track < num_tracks)
				{
					int time = lba_to_msf(m_cde_toc.tracks[track].physframeofs);

					m_cde_status_bytes[1] = 0x41;                    // q-Mode
					m_cde_status_bytes[2] = 0x0;                 // TNO (Lead-in track)
					m_cde_status_bytes[3] = track+1;             // Pointer
					m_cde_status_bytes[4] = 0x0;                 // Min
					m_cde_status_bytes[5] = 0x0;                 // Sec
					m_cde_status_bytes[6] = 0x0;                 // Frac
					m_cde_status_bytes[7] = 0x0;                 // Zero
					m_cde_status_bytes[8] = (time >> 16) & 0xff; // P-Min
					m_cde_status_bytes[9] = (time >>  8) & 0xff; // P-Sec
					m_cde_status_bytes[10] = time & 0xff;            // P-Frac
				}
				else
				{
					if (track == num_tracks+0)
					{
						m_cde_status_bytes[1] = 0x41;                    // q-Mode / Control
						m_cde_status_bytes[2] = 0x0;                 // TNO (Lead-in track)
						m_cde_status_bytes[3] = 0xa0;                    // Pointer
						m_cde_status_bytes[4] = 0x0;                 // Min
						m_cde_status_bytes[5] = 0x0;                 // Sec
						m_cde_status_bytes[6] = 0x0;                 // Frac
						m_cde_status_bytes[7] = 0x0;                 // Zero
						m_cde_status_bytes[8] = 1;                   // P-Min
						m_cde_status_bytes[9] = 0x0;                 // P-Sec
						m_cde_status_bytes[10] = 0x0;                    // P-Frac
					}
					else if (track == num_tracks+1)
					{
						m_cde_status_bytes[1] = 0x41;                    // q-Mode / Control
						m_cde_status_bytes[2] = 0x0;                 // TNO (Lead-in track)
						m_cde_status_bytes[3] = 0xa1;                    // Pointer
						m_cde_status_bytes[4] = 0x0;                 // Min
						m_cde_status_bytes[5] = 0x0;                 // Sec
						m_cde_status_bytes[6] = 0x0;                 // Frac
						m_cde_status_bytes[7] = 0x0;                 // Zero
						m_cde_status_bytes[8] = num_tracks;          // P-Min
						m_cde_status_bytes[9] = 0x0;                 // P-Sec
						m_cde_status_bytes[10] = 0x0;                    // P-Frac
					}
					else
					{
						int leadout_lba = m_cde_toc.tracks[num_tracks-1].physframeofs + m_cde_toc.tracks[num_tracks-1].frames;
						int leadout_time = lba_to_msf(leadout_lba);

						m_cde_status_bytes[1] = 0x41;                    // q-Mode / Control
						m_cde_status_bytes[2] = 0x0;                 // TNO (Lead-in track)
						m_cde_status_bytes[3] = 0xa2;                    // Pointer
						m_cde_status_bytes[4] = 0x0;                 // Min
						m_cde_status_bytes[5] = 0x0;                 // Sec
						m_cde_status_bytes[6] = 0x0;                 // Frac
						m_cde_status_bytes[7] = 0x0;                 // Zero
						m_cde_status_bytes[8] = (leadout_time >> 16) & 0xff; // P-Min
						m_cde_status_bytes[9] = (leadout_time >>  8) & 0xff; // P-Sec
						m_cde_status_bytes[10] = leadout_time & 0xff;            // P-Frac
					}
				}

				m_cde_qchannel_offset++;

				m_cde_status_byte_ptr = 0;
				m_cde_command_bytes[0] = 0x0c;

//              osd_printf_debug("CDE: QCHANNEL REPORT\n");
				break;
			}
		}
	}
}

void konamim2_state::cde_dma_transfer(address_space &space, int channel, int next)
{
	uint32_t address;
	//int length;
	int i;

	if (next)
	{
		address = m_cde_dma[channel].next_dst_addr;
		//length = m_cde_dma[channel].next_length;
	}
	else
	{
		address = m_cde_dma[channel].dst_addr;
		//length = m_cde_dma[channel].length;
	}

	for (i=0; i < m_cde_dma[channel].next_length; i++)
	{
		space.write_byte(address, 0xff);        // TODO: do the real transfer...
		address++;
	}
}

READ64_MEMBER(konamim2_state::cde_r)
{
	uint32_t r = 0;
	int reg = offset * 2;

	if (ACCESSING_BITS_0_31)
		reg++;

	switch (reg)
	{
		case 0x000/4:
		{
			r = (0x01) << 16;   // Device identifier, 1 = CDE
			break;
		}
		case 0x018/4:
		{
			r = 0x100038;

			r |= m_cde_dma[0].dma_done ? 0x400 : 0;
			r |= m_cde_dma[1].dma_done ? 0x800 : 0;
			break;
		}
		case 0x02c/4:
		{
			r = m_cde_status_bytes[m_cde_status_byte_ptr++];

			if (m_cde_status_byte_ptr <= m_cde_num_status_bytes)
			{
				r |= 0x100;
			}
			else
			{
				//if (cde_enable_reports &&
				//  !m_cde_response &&
				//  m_cde_command_bytes[0] != ((cde_report_type >> 8) & 0xff))

				if (!m_cde_response)
				{
					cde_handle_reports();

			//      m_cde_command_byte_ptr = 0;
			//      m_cde_command_bytes[m_cde_command_byte_ptr++] = 0x1c;

			//      m_cde_response = 1;
				}
			}

	//      printf("status byte %d\n", m_cde_status_byte_ptr);
			break;
		}

		case 0x2a0/4:
		{
			r = 0x20;
			break;
		}

		default:
		{
			break;
		}
	}

	if (reg & 1)
	{
		return (uint64_t)(r);
	}
	else
	{
		return (uint64_t)(r) << 32;
	}
}

WRITE64_MEMBER(konamim2_state::cde_w)
{
	int reg = offset * 2;
	uint32_t d;

	if (ACCESSING_BITS_0_31)
	{
		reg++;
		d = (uint32_t)(data);
	}
	else
	{
		d = (uint32_t)(data >> 32);
	}

	switch (reg)
	{
		case 0x028/4:       // Command write
		{
			if (d == 0x0180)
			{
				if (m_cde_response)
				{
					cde_handle_command();

					m_cde_response = 0;
				}

				m_cde_command_byte_ptr = 0;
			}
			else if (m_cde_command_byte_ptr == 0)
			{
				m_cde_num_status_bytes = 1;

				m_cde_status_bytes[0] = d & 0xff;
				m_cde_status_byte_ptr = 0;

				m_cde_response = 1;
			}

			if (d != 0x180)
			{
				m_cde_command_bytes[m_cde_command_byte_ptr++] = d;
			}

			break;
		}

		case 0x300/4:       // DMA Channel 0 enable
		{
//          osd_printf_debug("CDE: DMA0 enable %08X\n", d);

			if (d & 0x20)
			{
				m_cde_dma[0].dma_done = 1;

				cde_dma_transfer(space, 0, 0);
			}
			if (d & 0x40)
			{
				m_cde_dma[0].dma_done = 1;

				cde_dma_transfer(space, 0, 1);
			}
			break;
		}
		case 0x308/4:       // DMA Channel 0 destination address
		{
//          osd_printf_debug("CDE: DMA0 dst addr %08X\n", d);

			m_cde_dma[0].dst_addr = d;
			break;
		}
		case 0x30c/4:       // DMA Channel 0 length?
		{
//          osd_printf_debug("CDE: DMA0 length %08X\n", d);

			m_cde_dma[0].length = d;
			break;
		}
		case 0x318/4:       // DMA Channel 0 next destination address
		{
//          osd_printf_debug("CDE: DMA0 next dst addr %08X\n", d);

			m_cde_dma[0].next_dst_addr = d;
			break;
		}
		case 0x31c/4:       // DMA Channel 0 next length?
		{
//          osd_printf_debug("CDE: DMA0 next length %08X\n", d);

			m_cde_dma[0].next_length = d;
			break;
		}

		case 0x320/4:       // DMA Channel 1 enable
		{
//          osd_printf_debug("CDE: DMA1 enable %08X\n", d);
			break;
		}
		case 0x328/4:       // DMA Channel 1 destination address
		{
//          osd_printf_debug("CDE: DMA1 dst addr %08X\n", d);

			m_cde_dma[1].dst_addr = d;
			break;
		}
		case 0x32c/4:       // DMA Channel 1 length?
		{
//          osd_printf_debug("CDE: DMA1 length %08X\n", d);

			m_cde_dma[1].length = d;
			break;
		}
		case 0x338/4:       // DMA Channel 1 next destination address
		{
//          osd_printf_debug("CDE: DMA1 next dst addr %08X\n", d);

			m_cde_dma[1].next_dst_addr = d;
			break;
		}
		case 0x33c/4:       // DMA Channel 1 next length?
		{
//          osd_printf_debug("CDE: DMA1 next length %08X\n", d);

			m_cde_dma[1].next_length = d;
			break;
		}

		case 0x418/4:       // ???
		{
			if (d & 0x80000000)
			{
				m_irq_active &= ~0x8;
			}
			if (d & 0x60000000)
			{
				m_cde_dma[0].dma_done = 0;
				m_cde_dma[1].dma_done = 0;
			}
			break;
		}

		default:
		{
			break;
		}
	}
}

READ64_MEMBER(konamim2_state::device2_r)
{
	uint32_t r = 0;
	int reg = offset * 2;

	if (ACCESSING_BITS_0_31)
		reg++;

	switch (reg)
	{
		case 0x000/4:
		{
			r = (0x02) << 16;   // Device identifier
			break;
		}
		default:
		{
			break;
		}
	}

	if (reg & 1)
	{
		return (uint64_t)(r);
	}
	else
	{
		return (uint64_t)(r) << 32;
	}
}

template<bool maincpu> READ64_MEMBER(konamim2_state::cpu_r)
{
	uint64_t r = 0;

	if (ACCESSING_BITS_32_63)
	{
		r = (uint64_t)(maincpu ? 0 : 0x80000000);
		r |= m_in_monitor->read() << 30;
		return r << 32;
	}

	return 0;
}

READ8_MEMBER(konamim2_state::id3_r)
{
	return 0x03;
}

READ8_MEMBER(konamim2_state::id4_r)
{
	return 0x04;
}

READ8_MEMBER(konamim2_state::id5_r)
{
	return 0x05;
}

READ8_MEMBER(konamim2_state::id6_r)
{
	return 0x06;
}

READ8_MEMBER(konamim2_state::id7_r)
{
	return 0x07;
}

void konamim2_state::m2_main(address_map &map)
{
	map(0x00000000, 0x0000007f).ram(); // ???
	map(0x00010040, 0x00010047).rw(this, FUNC(konamim2_state::irq_enable_r), FUNC(konamim2_state::irq_enable_w));
	map(0x00010050, 0x00010057).r(this, FUNC(konamim2_state::irq_active_r));
	map(0x00020000, 0x00020007).rw(this, FUNC(konamim2_state::unk4_r), FUNC(konamim2_state::unk4_w));
	map(0x00020400, 0x000207ff).ram(); // ???
	map(0x00020800, 0x00020807).ram(); // ???
	map(0x00030000, 0x00030007).r(this, FUNC(konamim2_state::unk30000_r));
	map(0x00030010, 0x00030017).w(this, FUNC(konamim2_state::video_w));
	map(0x00030030, 0x00030037).r(this, FUNC(konamim2_state::unk30030_r));
	map(0x00030404, 0x00030407).w(this, FUNC(konamim2_state::video_irq_ack_w));

	map(0x01000000, 0x01000fff).rw(this, FUNC(konamim2_state::cde_r), FUNC(konamim2_state::cde_w));

	map(0x02000000, 0x02000fff).r(this, FUNC(konamim2_state::device2_r));

	map(0x03000001, 0x03000001).r(this, FUNC(konamim2_state::id3_r));

	map(0x04000001, 0x04000001).r(this, FUNC(konamim2_state::id4_r));
	map(0x04000017, 0x04000017).w(this, FUNC(konamim2_state::serial_w));
	map(0x04000018, 0x0400001f).r(this, FUNC(konamim2_state::unk1_r)); // serial status
	map(0x04000020, 0x04000027).w(this, FUNC(konamim2_state::reset_w));
	map(0x04000418, 0x0400041f).w(this, FUNC(konamim2_state::unk4000418_w)); // serial status ack
	map(0x04000208, 0x0400020f).r(this, FUNC(konamim2_state::unk3_r));
	map(0x04000280, 0x04000287).r(this, FUNC(konamim2_state::unk4000280_r));

	map(0x05000001, 0x05000001).r(this, FUNC(konamim2_state::id5_r));

	map(0x06000001, 0x06000001).r(this, FUNC(konamim2_state::id6_r));

	map(0x07000001, 0x07000001).r(this, FUNC(konamim2_state::id7_r));

	map(0x10000008, 0x10001007).noprw();     // ???

	map(0x20000000, 0x201fffff).rom().share("share2");
	map(0x40000000, 0x407fffff).ram().share("main_ram");
	map(0xfff00000, 0xffffffff).rom().region("boot", 0).share("share2");
}

void konamim2_state::m2_main_m(address_map &map)
{
	m2_main(map);
	map(0x10000000, 0x10000007).r(this, FUNC(konamim2_state::cpu_r<true>));
}

void konamim2_state::m2_main_s(address_map &map)
{
	m2_main(map);
	map(0x10000000, 0x10000007).r(this, FUNC(konamim2_state::cpu_r<false>));
}

void konamim2_state::_3do_m2_main(address_map &map)
{
//  ADDRESS_MAP_UNMAP_HIGH
	m2_main(map);

//  AM_RANGE(0x00000000, 0x000cffff) devices?
}

void konamim2_state::_3do_m2_main_m(address_map &map)
{
	_3do_m2_main(map);
	map(0x10000000, 0x10000007).r(this, FUNC(konamim2_state::cpu_r<true>));
}

void konamim2_state::_3do_m2_main_s(address_map &map)
{
	_3do_m2_main(map);
	map(0x10000000, 0x10000007).r(this, FUNC(konamim2_state::cpu_r<false>));
}


static INPUT_PORTS_START( m2 )
	// TODO: it's unknown if these are actual dip-switches or internal to something
	PORT_START("COUNTRY")
	PORT_CONFNAME( 0x03, 0x03, "Country" )
	PORT_CONFSETTING(    0x00, "<Invalid>" )
	PORT_CONFSETTING(    0x01, "UK" )
	PORT_CONFSETTING(    0x02, "Japan" )
	PORT_CONFSETTING(    0x03, "US" )

	PORT_START("CARD")
	PORT_CONFNAME( 0x0f, 0x0b, "Card Type" )
	PORT_CONFSETTING(    0x08, "AC-DevCard" )
	PORT_CONFSETTING(    0x0b, "AC-CoreBoard" )
	PORT_CONFSETTING(    0x0c, "DevCard (not allowed)" )
	PORT_CONFSETTING(    0x0e, "Upgrade (not allowed)" )
	PORT_CONFSETTING(    0x0f, "Multiplayer (not allowed)" )

	PORT_START("MONITOR")
	PORT_CONFNAME( 0x01, 0x00, "Monitor Type" )
	PORT_CONFSETTING(    0x01, "15 KHz" )
	PORT_CONFSETTING(    0x00, "24 KHz" )
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(konamim2_state::m2)
{
	/*
	 0x000001
	 0x000008
	 0x200000
	 0x800000 VBlank irq
	 */

	if (m_irq_enable & 0x800000)
	{
		//m_irq_enable |= 0x800000;
		m_irq_active |= 0x800000;
		device.execute().set_input_line(PPC_IRQ, ASSERT_LINE);
	}

	/*if (m_irq_enable & 0x8)
	{
	    m_irq_active |= 0x8;
	}*/


}

void konamim2_state::machine_reset()
{
	m_unk3 = 0xffffffffffffffffU;
	m_unk20004 = 0;
	cde_init();
}

MACHINE_CONFIG_START(konamim2_state::m2)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", PPC602, 66000000)   /* actually PPC602, 66MHz */
	MCFG_PPC_BUS_FREQUENCY(33000000)  /* Multiplier 2, Bus = 33MHz, Core = 66MHz */
	MCFG_DEVICE_PROGRAM_MAP(m2_main_m)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", konamim2_state,  m2)

	MCFG_DEVICE_ADD("sub", PPC602, 66000000)   /* actually PPC602, 66MHz */
	MCFG_PPC_BUS_FREQUENCY(33000000)  /* Multiplier 2, Bus = 33MHz, Core = 66MHz */
	MCFG_DEVICE_PROGRAM_MAP(m2_main_s)

	// TODO: declaring as second screen causes palette confusion (wants to use palette from the other screen?)
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(704, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_DRIVER(konamim2_state, screen_update_m2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRRGGGGGBBBBB("palette")


	/*cd-rom*/
	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("3do_m2_cdrom")

MACHINE_CONFIG_END

MACHINE_CONFIG_START(konamim2_state::_3do_m2)
	m2(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(_3do_m2_main_m)

	MCFG_DEVICE_MODIFY("sub")
	MCFG_DEVICE_PROGRAM_MAP(_3do_m2_main_s)

	MCFG_SOFTWARE_LIST_ADD("cd_list","3do_m2")
MACHINE_CONFIG_END


ROM_START( polystar )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.7k",  0x000000, 0x000080, CRC(66d02984) SHA1(d07c57d198c611b6ff67a783c20a3d038ba34cd1) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "623jaa02", 0, SHA1(e7d9e628a3e0e085e084e4e3630fa5e3a7345547) )
ROM_END

ROM_START( btltryst )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "636jac02", 0, SHA1(d36556a3a4b91058100924a9e9f1a58983399c6e) )
ROM_END

ROM_START( heatof11 )
	ROM_REGION64_BE( 0x200000, "boot", 0 )  /* boot rom */
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION( 0x2000, "timekeep", 0 ) /* timekeeper SRAM */
	ROM_LOAD( "dallas.5e",  0x000000, 0x002000, CRC(8611ff09) SHA1(6410236947d99c552c4a1f7dd5fd8c7a5ae4cba1) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "heatof11", 0, BAD_DUMP SHA1(5a0a2782cd8676d3f6dfad4e0f805b309e230d8b) )
ROM_END

ROM_START( evilngt )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION( 0x2000, "timekeep", 0 ) /* timekeeper SRAM */
	ROM_LOAD( "m48t58y.9n",   0x000000, 0x002000, CRC(e887ca1f) SHA1(54205f01b1ceba1d5f4d979fc30be1add8116e90) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "810a03.16h",  0x000000, 0x400000, CRC(4cd79d98) SHA1(12fea41cfc5c1b883ffbeda7e428dd1d1bf54d7f) )

	ROM_REGION( 0x80, "eeprom", 0 ) /* EEPROM default contents */
	ROM_LOAD( "93c46.7k",    0x000000, 0x000080, CRC(d7ba2e5e) SHA1(d729557555c6fc1cd433b14017952cc63ec73573) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "810uba02", 0, SHA1(e570470c1cbfe187d5bba8125616412f386264ba) )
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

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "810eaa02", 0, SHA1(d701b900eddc7674015823b2cb33e887bf107fa8) )
ROM_END

ROM_START( totlvice )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	// was converted from the following cue/bin pair, is this sufficient / good for this platform? - there are a lot of audio tracks that need verifying as non-corrupt
	//ROM_LOAD( "TotalVice-GQ639-EBA01.cue",  0, 0x00000555, CRC(55ef2f62) SHA1(8e31b3e62244e6090a93228dae377552340dcdeb) )
	//ROM_LOAD( "TotalVice-GQ639-EBA01.bin",  0, 0x1ec4db10, CRC(5882f8ba) SHA1(e589d500d99d2f4cff4506cd5ac9a5bfc8d30675) )
	DISK_REGION( "cdrom" )
	DISK_IMAGE( "639eba01", 0, BAD_DUMP  SHA1(d95c13575e015169b126f7e8492d150bd7e5ebda) )
ROM_END

ROM_START( totlvicu )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "639uac01", 0, BAD_DUMP SHA1(88431b8a0ce83c156c8b19efbba1af901b859404) )
ROM_END

ROM_START( totlvica )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) /* YMZ280B sound rom on sub board */
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "639aab01", 0, SHA1(bb99db2eeaecabfda8f20b7b06f714605bbd5b7c) )
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

void konamim2_state::init_m2()
{
}

GAME( 1997, polystar, 0,        m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Tobe! Polystars (ver JAA)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, totlvice, 0,        m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Total Vice (ver EBA)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, totlvicu, totlvice, m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Total Vice (ver UAC)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, totlvicj, totlvice, m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Total Vice (ver JAD)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, totlvica, totlvice, m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Total Vice (ver AAB)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, btltryst, 0,        m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Battle Tryst (ver JAC)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, heatof11, 0,        m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Heat of Eleven '98 (ver EAA)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, evilngt,  0,        m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Evil Night (ver UBA)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, evilngte, evilngt,  m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Evil Night (ver EAA)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, hellngt,  evilngt,  m2, m2, konamim2_state, init_m2, ROT0, "Konami", "Hell Night (ver EAA)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

CONS( 199?, 3do_m2, 0, 0,  _3do_m2, m2, konamim2_state, empty_init,    "3DO",    "3DO M2",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
