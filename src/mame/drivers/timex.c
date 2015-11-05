// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/***************************************************************************

    NOTE: ****** Specbusy: press N, R, or E to boot *************


        Spectrum/Inves/TK90X etc. memory map:

    CPU:
        0000-3fff ROM
        4000-ffff RAM

        Spectrum 128/+2/+2a/+3 memory map:

        CPU:
                0000-3fff Banked ROM/RAM (banked rom only on 128/+2)
                4000-7fff Banked RAM
                8000-bfff Banked RAM
                c000-ffff Banked RAM

        TS2068 memory map: (Can't have both EXROM and DOCK active)
        The 8K EXROM can be loaded into multiple pages.

    CPU:
                0000-1fff     ROM / EXROM / DOCK (Cartridge)
                2000-3fff     ROM / EXROM / DOCK
                4000-5fff \
                6000-7fff  \
                8000-9fff  |- RAM / EXROM / DOCK
                a000-bfff  |
                c000-dfff  /
                e000-ffff /


Interrupts:

Changes:

29/1/2000   KT -    Implemented initial +3 emulation.
30/1/2000   KT -    Improved input port decoding for reading and therefore
            correct keyboard handling for Spectrum and +3.
31/1/2000   KT -    Implemented buzzer sound for Spectrum and +3.
            Implementation copied from Paul Daniel's Jupiter driver.
            Fixed screen display problems with dirty chars.
            Added support to load .Z80 snapshots. 48k support so far.
13/2/2000   KT -    Added Interface II, Kempston, Fuller and Mikrogen
            joystick support.
17/2/2000   DJR -   Added full key descriptions and Spectrum+ keys.
            Fixed Spectrum +3 keyboard problems.
17/2/2000   KT -    Added tape loading from WAV/Changed from DAC to generic
            speaker code.
18/2/2000   KT -    Added tape saving to WAV.
27/2/2000   KT -    Took DJR's changes and added my changes.
27/2/2000   KT -    Added disk image support to Spectrum +3 driver.
27/2/2000   KT -    Added joystick I/O code to the Spectrum +3 I/O handler.
14/3/2000   DJR -   Tape handling dipswitch.
26/3/2000   DJR -   Snapshot files are now classifed as snapshots not
            cartridges.
04/4/2000   DJR -   Spectrum 128 / +2 Support.
13/4/2000   DJR -   +4 Support (unofficial 48K hack).
13/4/2000   DJR -   +2a Support (rom also used in +3 models).
13/4/2000   DJR -   TK90X, TK95 and Inves support (48K clones).
21/4/2000   DJR -   TS2068 and TC2048 support (TC2048 Supports extra video
            modes but doesn't have bank switching or sound chip).
09/5/2000   DJR -   Spectrum +2 (France, Spain), +3 (Spain).
17/5/2000   DJR -   Dipswitch to enable/disable disk drives on +3 and clones.
27/6/2000   DJR -   Changed 128K/+3 port decoding (sound now works in Zub 128K).
06/8/2000   DJR -   Fixed +3 Floppy support
10/2/2001   KT  -   Re-arranged code and split into each model emulated.
            Code is split into 48k, 128k, +3, tc2048 and ts2048
            segments. 128k uses some of the functions in 48k, +3
            uses some functions in 128, and tc2048/ts2048 use some
            of the functions in 48k. The code has been arranged so
            these functions come in some kind of "override" order,
            read functions changed to use  READ8_HANDLER and write
            functions changed to use WRITE8_HANDLER.
            Added Scorpion256 preliminary.
18/6/2001   DJR -   Added support for Interface 2 cartridges.
xx/xx/2001  KS -    TS-2068 sound fixed.
            Added support for DOCK cartridges for TS-2068.
            Added Spectrum 48k Psycho modified rom driver.
            Added UK-2086 driver.
23/12/2001  KS -    48k machines are now able to run code in screen memory.
                Programs which keep their code in screen memory
                like monitors, tape copiers, decrunchers, etc.
                works now.
                Fixed problem with interrupt vector set to 0xffff (much
            more 128k games works now).
                A useful used trick on the Spectrum is to set
                interrupt vector to 0xffff (using the table
                which contain 0xff's) and put a byte 0x18 hex,
                the opcode for JR, at this address. The first
                byte of the ROM is a 0xf3 (DI), so the JR will
                jump to 0xfff4, where a long JP to the actual
                interrupt routine is put. Due to unideal
                bankswitching in MAME this JP were to 0001 what
                causes Spectrum to reset. Fixing this problem
                made much more software runing (i.e. Paperboy).
            Corrected frames per second value for 48k and 128k
            Sincalir machines.
                There are 50.08 frames per second for Spectrum
                48k what gives 69888 cycles for each frame and
                50.021 for Spectrum 128/+2/+2A/+3 what gives
                70908 cycles for each frame.
            Remaped some Spectrum+ keys.
                Presing F3 to reset was seting 0xf7 on keyboard
                input port. Problem occurred for snapshots of
                some programms where it was readed as pressing
                key 4 (which is exit in Tapecopy by R. Dannhoefer
                for example).
            Added support to load .SP snapshots.
            Added .BLK tape images support.
                .BLK files are identical to .TAP ones, extension
                is an only difference.
08/03/2002  KS -    #FF port emulation added.
                Arkanoid works now, but is not playable due to
                completly messed timings.

Initialisation values used when determining which model is being emulated:
 48K        Spectrum doesn't use either port.
 128K/+2    Bank switches with port 7ffd only.
 +3/+2a     Bank switches with both ports.

Notes:
 1. No contented memory.
 2. No hi-res colour effects (need contended memory first for accurate timing).
 3. Multiface 1 and Interface 1 not supported.
 4. Horace and the Spiders cartridge doesn't run properly.
 5. Tape images not supported:
    .TZX, .SPC, .ITM, .PAN, .TAP(Warajevo), .VOC, .ZXS.
 6. Snapshot images not supported:
    .ACH, .PRG, .RAW, .SEM, .SIT, .SNX, .ZX, .ZXS, .ZX82.
 7. 128K emulation is not perfect - the 128K machines crash and hang while
    running quite a lot of games.
 8. Disk errors occur on some +3 games.
 9. Video hardware of all machines is timed incorrectly.
10. EXROM and HOME cartridges are not emulated.
11. The TK90X and TK95 roms output 0 to port #df on start up.
12. The purpose of this port is unknown (probably display mode as TS2068) and
    thus is not emulated.

Very detailed infos about the ZX Spectrum +3e can be found at

http://www.z88forever.org.uk/zxplus3e/

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/spectrum.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"
#include "sound/ay8910.h"
#include "formats/tzx_cas.h"
#include "machine/spec_snqk.h"
#include "machine/beta.h"
#include "machine/ram.h"
#include "softlist.h"

/****************************************************************************************************/
/* TS2048 specific functions */


READ8_MEMBER( spectrum_state::ts2068_port_f4_r )
{
	return m_port_f4_data;
}

WRITE8_MEMBER( spectrum_state::ts2068_port_f4_w )
{
	m_port_f4_data = data;
	ts2068_update_memory();
}

READ8_MEMBER( spectrum_state::ts2068_port_ff_r )
{
	return m_port_ff_data;
}

WRITE8_MEMBER( spectrum_state::ts2068_port_ff_w )
{
		/* Bits 0-2 Video Mode Select
		   Bits 3-5 64 column mode ink/paper selection
		            (See ts2068_vh_screenrefresh for more info)
		   Bit  6   17ms Interrupt Inhibit
		   Bit  7   Cartridge (0) / EXROM (1) select
		*/
	m_port_ff_data = data;
	ts2068_update_memory();
	logerror("Port %04x write %02x\n", offset, data);
}

/*******************************************************************
 *
 *      Bank switch between the 3 internal memory banks HOME, EXROM
 *      and DOCK (Cartridges). The HOME bank contains 16K ROM in the
 *      0-16K area and 48K RAM fills the rest. The EXROM contains 8K
 *      ROM and can appear in every 8K segment (ie 0-8K, 8-16K etc).
 *      The DOCK is empty and is meant to be occupied by cartridges
 *      you can plug into the cartridge dock of the 2068.
 *
 *      The address space is divided into 8 8K chunks. Bit 0 of port
 *      #f4 corresponds to the 0-8K chunk, bit 1 to the 8-16K chunk
 *      etc. If the bit is 0 then the chunk is controlled by the HOME
 *      bank. If the bit is 1 then the chunk is controlled by either
 *      the DOCK or EXROM depending on bit 7 of port #ff. Note this
 *      means that that the Z80 can't see chunks of the EXROM and DOCK
 *      at the same time.
 *
 *******************************************************************/
void spectrum_state::ts2068_update_memory()
{
	UINT8 *messram = m_ram->pointer();
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *DOCK = m_dock_crt->base();
	UINT8 *ExROM = memregion("maincpu")->base() + 0x014000;
	UINT8 *ChosenROM;

	if (m_port_f4_data & 0x01)
	{
		if (m_port_ff_data & 0x80)
		{
				space.install_read_bank(0x0000, 0x1fff, "bank1");
				space.unmap_write(0x0000, 0x1fff);
				membank("bank1")->set_base(ExROM);
				logerror("0000-1fff EXROM\n");
		}
		else
		{
			if (m_dock_cart_type == TIMEX_CART_DOCK)
			{
				membank("bank1")->set_base(DOCK);
				space.install_read_bank(0x0000, 0x1fff, "bank1");
				if (m_ram_chunks & 0x01)
					space.install_write_bank(0x0000, 0x1fff, "bank9");
				else
					space.unmap_write(0x0000, 0x1fff);


			}
			else
			{
				space.nop_read(0x0000, 0x1fff);
				space.unmap_write(0x0000, 0x1fff);
			}
			logerror("0000-1fff Cartridge\n");
		}
	}
	else
	{
		ChosenROM = memregion("maincpu")->base() + 0x010000;
		membank("bank1")->set_base(ChosenROM);
		space.install_read_bank(0x0000, 0x1fff, "bank1");
		space.unmap_write(0x0000, 0x1fff);
		logerror("0000-1fff HOME\n");
	}

	if (m_port_f4_data & 0x02)
	{
		if (m_port_ff_data & 0x80)
		{
			membank("bank2")->set_base(ExROM);
			space.install_read_bank(0x2000, 0x3fff, "bank2");
			space.unmap_write(0x2000, 0x3fff);
			logerror("2000-3fff EXROM\n");
		}
		else
		{
			if (m_dock_cart_type == TIMEX_CART_DOCK)
			{
				membank("bank2")->set_base(DOCK+0x2000);
				space.install_read_bank(0x2000, 0x3fff, "bank2");
				if (m_ram_chunks & 0x02)
					space.install_write_bank(0x2000, 0x3fff, "bank10");
				else
					space.unmap_write(0x2000, 0x3fff);

			}
			else
			{
				space.nop_read(0x2000, 0x3fff);
				space.unmap_write(0x2000, 0x3fff);
			}
			logerror("2000-3fff Cartridge\n");
		}
	}
	else
	{
		ChosenROM = memregion("maincpu")->base() + 0x012000;
		membank("bank2")->set_base(ChosenROM);
		space.install_read_bank(0x2000, 0x3fff, "bank2");
		space.unmap_write(0x2000, 0x3fff);
		logerror("2000-3fff HOME\n");
	}

	if (m_port_f4_data & 0x04)
	{
		if (m_port_ff_data & 0x80)
		{
			membank("bank3")->set_base(ExROM);
			space.install_read_bank(0x4000, 0x5fff, "bank3");
			space.unmap_write(0x4000, 0x5fff);
			logerror("4000-5fff EXROM\n");
		}
		else
		{
			if (m_dock_cart_type == TIMEX_CART_DOCK)
			{
				membank("bank3")->set_base(DOCK+0x4000);
				space.install_read_bank(0x4000, 0x5fff, "bank3");
				if (m_ram_chunks & 0x04)
					space.install_write_bank(0x4000, 0x5fff, "bank11");
				else
					space.unmap_write(0x4000, 0x5fff);
			}
			else
			{
				space.nop_read(0x4000, 0x5fff);
				space.unmap_write(0x4000, 0x5fff);
			}
			logerror("4000-5fff Cartridge\n");
		}
	}
	else
	{
		membank("bank3")->set_base(messram);
		membank("bank11")->set_base(messram);
		space.install_read_bank(0x4000, 0x5fff, "bank3");
		space.install_write_bank(0x4000, 0x5fff, "bank11");
		logerror("4000-5fff RAM\n");
	}

	if (m_port_f4_data & 0x08)
	{
		if (m_port_ff_data & 0x80)
		{
			membank("bank4")->set_base(ExROM);
			space.install_read_bank(0x6000, 0x7fff, "bank4");
			space.unmap_write(0x6000, 0x7fff);
			logerror("6000-7fff EXROM\n");
		}
		else
		{
				if (m_dock_cart_type == TIMEX_CART_DOCK)
				{
					membank("bank4")->set_base(DOCK+0x6000);
					space.install_read_bank(0x6000, 0x7fff, "bank4");
					if (m_ram_chunks & 0x08)
						space.install_write_bank(0x6000, 0x7fff, "bank12");
					else
						space.unmap_write(0x6000, 0x7fff);
				}
				else
				{
					space.nop_read(0x6000, 0x7fff);
					space.unmap_write(0x6000, 0x7fff);
				}
				logerror("6000-7fff Cartridge\n");
		}
	}
	else
	{
		membank("bank4")->set_base(messram + 0x2000);
		membank("bank12")->set_base(messram + 0x2000);
		space.install_read_bank(0x6000, 0x7fff, "bank4");
		space.install_write_bank(0x6000, 0x7fff, "bank12");
		logerror("6000-7fff RAM\n");
	}

	if (m_port_f4_data & 0x10)
	{
		if (m_port_ff_data & 0x80)
		{
			membank("bank5")->set_base(ExROM);
			space.install_read_bank(0x8000, 0x9fff, "bank5");
			space.unmap_write(0x8000, 0x9fff);
			logerror("8000-9fff EXROM\n");
		}
		else
		{
			if (m_dock_cart_type == TIMEX_CART_DOCK)
			{
				membank("bank5")->set_base(DOCK+0x8000);
				space.install_read_bank(0x8000, 0x9fff,"bank5");
				if (m_ram_chunks & 0x10)
					space.install_write_bank(0x8000, 0x9fff,"bank13");
				else
					space.unmap_write(0x8000, 0x9fff);
			}
			else
			{
				space.nop_read(0x8000, 0x9fff);
				space.unmap_write(0x8000, 0x9fff);
			}
			logerror("8000-9fff Cartridge\n");
		}
	}
	else
	{
		membank("bank5")->set_base(messram + 0x4000);
		membank("bank13")->set_base(messram + 0x4000);
		space.install_read_bank(0x8000, 0x9fff,"bank5");
		space.install_write_bank(0x8000, 0x9fff,"bank13");
		logerror("8000-9fff RAM\n");
	}

	if (m_port_f4_data & 0x20)
	{
		if (m_port_ff_data & 0x80)
		{
			membank("bank6")->set_base(ExROM);
			space.install_read_bank(0xa000, 0xbfff, "bank6");
			space.unmap_write(0xa000, 0xbfff);
			logerror("a000-bfff EXROM\n");
		}
		else
		{
			if (m_dock_cart_type == TIMEX_CART_DOCK)
			{
				membank("bank6")->set_base(DOCK+0xa000);
				space.install_read_bank(0xa000, 0xbfff, "bank6");
				if (m_ram_chunks & 0x20)
					space.install_write_bank(0xa000, 0xbfff, "bank14");
				else
					space.unmap_write(0xa000, 0xbfff);

			}
			else
			{
				space.nop_read(0xa000, 0xbfff);
				space.unmap_write(0xa000, 0xbfff);
			}
			logerror("a000-bfff Cartridge\n");
		}
	}
	else
	{
		membank("bank6")->set_base(messram + 0x6000);
		membank("bank14")->set_base(messram + 0x6000);
		space.install_read_bank(0xa000, 0xbfff, "bank6");
		space.install_write_bank(0xa000, 0xbfff, "bank14");
		logerror("a000-bfff RAM\n");
	}

	if (m_port_f4_data & 0x40)
	{
		if (m_port_ff_data & 0x80)
		{
			membank("bank7")->set_base(ExROM);
			space.install_read_bank(0xc000, 0xdfff, "bank7");
			space.unmap_write(0xc000, 0xdfff);
			logerror("c000-dfff EXROM\n");
		}
		else
		{
			if (m_dock_cart_type == TIMEX_CART_DOCK)
			{
				membank("bank7")->set_base(DOCK+0xc000);
				space.install_read_bank(0xc000, 0xdfff, "bank7");
				if (m_ram_chunks & 0x40)
					space.install_write_bank(0xc000, 0xdfff, "bank15");
				else
					space.unmap_write(0xc000, 0xdfff);
			}
			else
			{
				space.nop_read(0xc000, 0xdfff);
				space.unmap_write(0xc000, 0xdfff);
			}
			logerror("c000-dfff Cartridge\n");
		}
	}
	else
	{
		membank("bank7")->set_base(messram + 0x8000);
		membank("bank15")->set_base(messram + 0x8000);
		space.install_read_bank(0xc000, 0xdfff, "bank7");
		space.install_write_bank(0xc000, 0xdfff, "bank15");
		logerror("c000-dfff RAM\n");
	}

	if (m_port_f4_data & 0x80)
	{
		if (m_port_ff_data & 0x80)
		{
			membank("bank8")->set_base(ExROM);
			space.install_read_bank(0xe000, 0xffff, "bank8");
			space.unmap_write(0xe000, 0xffff);
			logerror("e000-ffff EXROM\n");
		}
		else
		{
			if (m_dock_cart_type == TIMEX_CART_DOCK)
			{
				membank("bank8")->set_base(DOCK+0xe000);
				space.install_read_bank(0xe000, 0xffff, "bank8");
				if (m_ram_chunks & 0x80)
					space.install_write_bank(0xe000, 0xffff, "bank16");
				else
					space.unmap_write(0xe000, 0xffff);
			}
			else
			{
				space.nop_read(0xe000, 0xffff);
				space.unmap_write(0xe000, 0xffff);
			}
			logerror("e000-ffff Cartridge\n");
		}
	}
	else
	{
		membank("bank8")->set_base(messram + 0xa000);
		membank("bank16")->set_base(messram + 0xa000);
		space.install_read_bank(0xe000, 0xffff, "bank8");
		space.install_write_bank(0xe000, 0xffff, "bank16");
		logerror("e000-ffff RAM\n");
	}
}

static ADDRESS_MAP_START(ts2068_io, AS_IO, 8, spectrum_state )
	AM_RANGE(0x1f, 0x1f) AM_READ(spectrum_port_1f_r ) AM_MIRROR(0xff00)
	AM_RANGE(0x7f, 0x7f) AM_READ(spectrum_port_7f_r ) AM_MIRROR(0xff00)
	AM_RANGE(0xdf, 0xdf) AM_READ(spectrum_port_df_r ) AM_MIRROR(0xff00)
	AM_RANGE(0xf4, 0xf4) AM_READWRITE(ts2068_port_f4_r,ts2068_port_f4_w ) AM_MIRROR(0xff00)
	AM_RANGE(0xf5, 0xf5) AM_DEVWRITE("ay8912", ay8910_device, address_w ) AM_MIRROR(0xff00)
	AM_RANGE(0xf6, 0xf6) AM_DEVREADWRITE("ay8912", ay8910_device, data_r, data_w ) AM_MIRROR(0xff00)
	AM_RANGE(0xfe, 0xfe) AM_READWRITE(spectrum_port_fe_r,spectrum_port_fe_w )  AM_MIRROR(0xff00)  AM_MASK(0xffff)
	AM_RANGE(0xff, 0xff) AM_READWRITE(ts2068_port_ff_r,ts2068_port_ff_w ) AM_MIRROR(0xff00)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ts2068_mem, AS_PROGRAM, 8, spectrum_state )
	AM_RANGE(0x0000, 0x1fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank9")
	AM_RANGE(0x2000, 0x3fff) AM_READ_BANK("bank2") AM_WRITE_BANK("bank10")
	AM_RANGE(0x4000, 0x5fff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank11")
	AM_RANGE(0x6000, 0x7fff) AM_READ_BANK("bank4") AM_WRITE_BANK("bank12")
	AM_RANGE(0x8000, 0x9fff) AM_READ_BANK("bank5") AM_WRITE_BANK("bank13")
	AM_RANGE(0xa000, 0xbfff) AM_READ_BANK("bank6") AM_WRITE_BANK("bank14")
	AM_RANGE(0xc000, 0xdfff) AM_READ_BANK("bank7") AM_WRITE_BANK("bank15")
	AM_RANGE(0xe000, 0xffff) AM_READ_BANK("bank8") AM_WRITE_BANK("bank16")
ADDRESS_MAP_END


MACHINE_RESET_MEMBER(spectrum_state,ts2068)
{
	m_port_ff_data = 0;
	m_port_f4_data = 0;

	std::string region_tag;
	m_dock_crt = memregion(region_tag.assign(m_dock->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	m_dock_cart_type = m_dock_crt ? TIMEX_CART_DOCK : TIMEX_CART_NONE;

	ts2068_update_memory();
	MACHINE_RESET_CALL_MEMBER(spectrum);
}


/****************************************************************************************************/
/* TC2048 specific functions */


WRITE8_MEMBER( spectrum_state::tc2048_port_ff_w )
{
	m_port_ff_data = data;
	logerror("Port %04x write %02x\n", offset, data);
}

static ADDRESS_MAP_START(tc2048_io, AS_IO, 8, spectrum_state )
	AM_RANGE(0x00, 0x00) AM_READWRITE(spectrum_port_fe_r,spectrum_port_fe_w) AM_MIRROR(0xfffe) AM_MASK(0xffff)
	AM_RANGE(0x1f, 0x1f) AM_READ(spectrum_port_1f_r) AM_MIRROR(0xff00)
	AM_RANGE(0x7f, 0x7f) AM_READ(spectrum_port_7f_r) AM_MIRROR(0xff00)
	AM_RANGE(0xdf, 0xdf) AM_READ(spectrum_port_df_r) AM_MIRROR(0xff00)
	AM_RANGE(0xff, 0xff) AM_READWRITE(ts2068_port_ff_r,tc2048_port_ff_w)  AM_MIRROR(0xff00)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tc2048_mem, AS_PROGRAM, 8, spectrum_state )
	AM_RANGE( 0x0000, 0x3fff) AM_ROM
	AM_RANGE( 0x4000, 0xffff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(spectrum_state,tc2048)
{
	UINT8 *messram = m_ram->pointer();

	membank("bank1")->set_base(messram);
	membank("bank2")->set_base(messram);
	m_port_ff_data = 0;
	m_port_f4_data = -1;
	MACHINE_RESET_CALL_MEMBER(spectrum);
}


DEVICE_IMAGE_LOAD_MEMBER( spectrum_state, timex_cart )
{
	UINT32 size = m_dock->common_get_size("rom");

	if (image.software_entry() == NULL)
	{
		UINT8 *DOCK;
		int chunks_in_file = 0;
		dynamic_buffer header;
		header.resize(9);

		if (size % 0x2000 != 9)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "File corrupted");
			return IMAGE_INIT_FAIL;
		}
		if (image.software_entry() != NULL)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Loading from softlist is not supported yet");
			return IMAGE_INIT_FAIL;
		}

		m_dock->rom_alloc(0x10000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
		DOCK = m_dock->get_rom_base();

		// check header
		image.fread(&header[0], 9);

		for (int i = 0; i < 8; i++)
			if (header[i + 1] & 0x02) chunks_in_file++;

		if (chunks_in_file * 0x2000 + 0x09 != size)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "File corrupted");
			return IMAGE_INIT_FAIL;
		}

		switch (header[0])
		{
			case 0x00:  logerror ("DOCK cart\n");
				m_ram_chunks = 0;
				for (int i = 0; i < 8; i++)
				{
					m_ram_chunks = m_ram_chunks | ((header[i + 1] & 0x01) << i);
					if (header[i + 1] & 0x02)
						image.fread(DOCK + i * 0x2000, 0x2000);
					else
					{
						if (header[i + 1] & 0x01)
							memset(DOCK + i * 0x2000, 0x00, 0x2000);
						else
							memset(DOCK + i * 0x2000, 0xff, 0x2000);
					}
				}
				break;

			default:
				image.seterror(IMAGE_ERROR_UNSPECIFIED, "Cart type not supported");
				return IMAGE_INIT_FAIL;
		}

		logerror ("Cart loaded [Chunks %02x]\n", m_ram_chunks);
	}
	else
	{
		m_dock->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
		memcpy(m_dock->get_rom_base(), image.get_software_region("rom"), size);
	}

	return IMAGE_INIT_PASS;
}


/* F4 Character Displayer - tc2048 code is inherited from the spectrum */
static const gfx_layout ts2068_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	96,                 /* 96 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( ts2068 )
	GFXDECODE_ENTRY( "maincpu", 0x13d00, ts2068_charlayout, 0, 8 )
GFXDECODE_END

static MACHINE_CONFIG_DERIVED( ts2068, spectrum_128 )
	MCFG_CPU_REPLACE("maincpu", Z80, XTAL_14_112MHz/4)        /* From Schematic; 3.528 MHz */
	MCFG_CPU_PROGRAM_MAP(ts2068_mem)
	MCFG_CPU_IO_MAP(ts2068_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spectrum_state,  spec_interrupt)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_RESET_OVERRIDE(spectrum_state, ts2068 )

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(TS2068_SCREEN_WIDTH, TS2068_SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, TS2068_SCREEN_WIDTH-1, 0, TS2068_SCREEN_HEIGHT-1)
	MCFG_SCREEN_UPDATE_DRIVER(spectrum_state, screen_update_ts2068)
	MCFG_SCREEN_VBLANK_DRIVER(spectrum_state, screen_eof_timex)

	MCFG_GFXDECODE_MODIFY("gfxdecode", ts2068)

	MCFG_VIDEO_START_OVERRIDE(spectrum_state, ts2068 )

	/* sound */
	MCFG_SOUND_REPLACE("ay8912", AY8912, XTAL_14_112MHz/8)        /* From Schematic; 1.764 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* cartridge */
	MCFG_DEVICE_REMOVE("cartslot")
	MCFG_GENERIC_CARTSLOT_ADD("dockslot", generic_plain_slot, "timex_cart")
	MCFG_GENERIC_EXTENSIONS("dck,bin")
	MCFG_GENERIC_LOAD(spectrum_state, timex_cart)

	/* Software lists */
	MCFG_DEVICE_REMOVE("cart_list")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "timex_dock")

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( uk2086, ts2068 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( tc2048, spectrum )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tc2048_mem)
	MCFG_CPU_IO_MAP(tc2048_io)

	MCFG_MACHINE_RESET_OVERRIDE(spectrum_state, tc2048 )

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(TS2068_SCREEN_WIDTH, SPEC_SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, TS2068_SCREEN_WIDTH-1, 0, SPEC_SCREEN_HEIGHT-1)
	MCFG_SCREEN_UPDATE_DRIVER(spectrum_state, screen_update_tc2048)
	MCFG_SCREEN_VBLANK_DRIVER(spectrum_state, screen_eof_timex)

	MCFG_VIDEO_START_OVERRIDE(spectrum_state, spectrum_128 )

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K")
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(tc2048)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("tc2048.rom",0x0000,0x4000, CRC(f1b5fa67) SHA1(febb2d495b6eda7cdcb4074935d6e9d9f328972d))
ROM_END

ROM_START(ts2068)
	ROM_REGION(0x16000,"maincpu",0)
	ROM_LOAD("ts2068_h.rom",0x10000,0x4000, CRC(bf44ec3f) SHA1(1446cb2780a9dedf640404a639fa3ae518b2d8aa))
	ROM_LOAD("ts2068_x.rom",0x14000,0x2000, CRC(ae16233a) SHA1(7e265a2c1f621ed365ea23bdcafdedbc79c1299c))
ROM_END

ROM_START(uk2086)
	ROM_REGION(0x16000,"maincpu",0)
	ROM_LOAD("uk2086_h.rom",0x10000,0x4000, CRC(5ddc0ca2) SHA1(1d525fe5cdc82ab46767f665ad735eb5363f1f51))
	ROM_LOAD("ts2068_x.rom",0x14000,0x2000, CRC(ae16233a) SHA1(7e265a2c1f621ed365ea23bdcafdedbc79c1299c))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE     INPUT       INIT    COMPANY     FULLNAME */
COMP( 1984, tc2048,   spectrum, 0,      tc2048,     spectrum, driver_device,    0,      "Timex of Portugal",    "TC-2048" , 0)
COMP( 1983, ts2068,   spectrum, 0,      ts2068,     spectrum, driver_device,    0,      "Timex Sinclair",       "TS-2068" , 0)
COMP( 1986, uk2086,   spectrum, 0,      uk2086,     spectrum, driver_device,    0,      "Unipolbrit",           "UK-2086 ver. 1.2" , 0)
