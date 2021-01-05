// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/***************************************************************************

  spec_snqk.cpp

  TODO:

    - Implement the following snapshot formats
      .89C (Tezxas)
      .SLT (Various emulators)
      .SZX (Spectaculator)
      .ZLS (ZX-Live!)
      .ZX (ZXSpectr)
      .ZX82 (Speculator '97)
      .ZXS (ZX32)

    - Implement the following quickload formats
      .$? (HoBeta)
      .__? (SpecEm)
      .BAS (BASin)
      .H/.B (Roman ZX)
      .RAW (Various emulators)
      .SCR variants

    - Cleanup of the .Z80 format

***************************************************************************/

#include "emu.h"
#include "ui/uimain.h"
#include "cpu/z80/z80.h"
#include "includes/spectrum.h"
#include "includes/spec128.h"
#include "includes/timex.h"
#include "includes/specpls3.h"
#include "sound/ay8910.h"
#include "machine/spec_snqk.h"

#define EXEC_NA "N/A"

//-------------------------------------------------
//  log_quickload - logs and displays useful
//  data for the end user
//-------------------------------------------------

void spectrum_state::log_quickload(const char *type, uint32_t start, uint32_t length, uint32_t exec, const char *exec_format)
{
	std::ostringstream tempstring;

	logerror("Loading %04X bytes of RAM at %04X\n", length, start);

	util::stream_format(tempstring, "Quickload type: %s   Length: %d bytes\n", type, length);
	util::stream_format(tempstring, "Start: 0x%04X   End: 0x%04X   Exec: ", start, start + length - 1);

	logerror("Quickload loaded.\n");
	if (!core_stricmp(exec_format, EXEC_NA))
		tempstring << "N/A";
	else
	{
		logerror("Execution can resume with ");
		logerror(exec_format, exec);
		logerror("\n");
		util::stream_format(tempstring, exec_format, exec);
	}

	machine().ui().popup_time(10, "%s", tempstring.str());
}

/*******************************************************************
 *
 *      Update the memory and paging of the spectrum being emulated
 *
 *      if port_7ffd_data is -1 then machine is 48K - no paging
 *      if port_1ffd_data is -1 then machine is 128K
 *      if neither port is -1 then machine is +2a/+3
 *
 *      Note: the 128K .SNA and .Z80 file formats do not store the
 *      port 1FFD setting so it is necessary to calculate the appropriate
 *      value for the ROM paging.
 *
 *******************************************************************/
void spectrum_state::update_paging()
{
	if (m_port_7ffd_data == -1)
		return;
	if (m_port_1ffd_data == -1)
		spectrum_128_update_memory();

	else
	{
		if (BIT(m_port_7ffd_data, 4))
			// Page in Spec 48K basic ROM
			m_port_1ffd_data = 0x04;
		else
			m_port_1ffd_data = 0x00;
		plus3_update_memory();
	}
}

/* Page in the 48K Basic ROM. Used when running 48K snapshots on a 128K machine. */
void spectrum_state::page_basicrom()
{
	if (m_port_7ffd_data == -1)
		return;
	m_port_7ffd_data |= 0x10;
	update_paging();
}


SNAPSHOT_LOAD_MEMBER(spectrum_state::snapshot_cb)
{
	size_t snapshot_size = image.length();
	std::vector<uint8_t> snapshot_data(snapshot_size);

	image.fread(&snapshot_data[0], snapshot_size);

	if (image.is_filetype("sna"))
	{
		if ((snapshot_size != SNA48_SIZE) && (snapshot_size != SNA128_SIZE_1) && (snapshot_size != SNA128_SIZE_2))
		{
			logerror("Invalid .SNA file size.\n");
			goto error;
		}
		setup_sna(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("sp"))
	{
		if ((snapshot_data[0] != 'S' && snapshot_data[1] != 'P') && (snapshot_size != SP_NEW_SIZE_16K && snapshot_size != SP_NEW_SIZE_48K))
		{
			if (snapshot_size != SP_OLD_SIZE)
			{
				logerror("Invalid .SP file size.\n");
				goto error;
			}
		}
		setup_sp(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("ach"))
	{
		if (snapshot_size != ACH_SIZE)
		{
			logerror("Invalid .ACH file size.\n");
			goto error;
		}
		setup_ach(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("prg"))
	{
		if (snapshot_size != PRG_SIZE)
		{
			logerror("Invalid .PRG file size.\n");
			goto error;
		}
		setup_prg(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("plusd"))
	{
		if ((snapshot_size != PLUSD48_SIZE) && (snapshot_size != PLUSD128_SIZE))
		{
			logerror("Invalid .PLUSD file size.\n");
			goto error;
		}
		setup_plusd(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("sem"))
	{
		if (snapshot_data[0] != 0x05 && snapshot_data[1] != 'S' && \
			snapshot_data[2] != 'P' && snapshot_data[3] != 'E' && \
			snapshot_data[4] != 'C' && snapshot_data[5] != '1')
		{
			if (snapshot_size != SEM_SIZE)
			{
				logerror("Invalid .SEM file size.\n");
				goto error;
			}
		}
		setup_sem(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("sit"))
	{
		if (snapshot_size != SIT_SIZE)
		{
			logerror("Invalid .SIT file size.\n");
			goto error;
		}
		setup_sit(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("zx"))
	{
		if (snapshot_size != ZX_SIZE)
		{
			logerror("Invalid .ZX file size.\n");
			goto error;
		}
		setup_zx(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("snp"))
	{
		if (snapshot_size != SNP_SIZE)
		{
			logerror("Invalid .SNP file size.\n");
			goto error;
		}
		setup_snp(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("snx"))
	{
		if (snapshot_data[0] != 'X' && snapshot_data[1] != 'S' && \
			snapshot_data[2] != 'N' && snapshot_data[3] != 'A')
		{
			logerror("Invalid .SNX file size.\n");
			goto error;
		}
		setup_snx(&snapshot_data[0], snapshot_size);
	}
	else if (image.is_filetype("frz"))
	{
		if (snapshot_size != FRZ_SIZE)
		{
			logerror("Invalid .FRZ file size.\n");
			goto error;
		}
		setup_frz(&snapshot_data[0], snapshot_size);
	}
	else
	{
		setup_z80(&snapshot_data[0], snapshot_size);
	}

	return image_init_result::PASS;

error:
	return image_init_result::FAIL;
}

/*******************************************************************
 *
 *      Load a .SP file.
 *
 *      There are two kinds of .SP files: "old" and "new".
 *
 *      The old version is always 49184 bytes long and is created
 *      by a leaked copy of the VGASpec emulator.
 *
 *      Subsequently Pedro Gimeno (the author of VGASpec) renamed it
 *      to "Spectrum" (but it's colloquially known as the "Spanish
 *      Spectrum emulator") and extended the header to break backward
 *      compatibility: the new format supports both 16K and 48K
 *      images and it's 16422 or 49190 bytes long.
 *
 *      http://www.formauri.es/personal/pgimeno/spec/spec.html
 *
 *      The formats are defined as follows:
 *
 *      Offset              Size    Description (all registers stored with LSB first)
 *      ------------------- ------- -------------------------------------------------
 *      VGASPEC SPECTRUM
 *      ------------------- ------- -------------------------------------------------
 *      N/A     0           2       "SP" (signature)
 *      N/A     2           2       Program length
 *      N/A     4           2       Program location
 *      0       6           2       BC
 *      2       8           2       DE
 *      4       10          2       HL
 *      6       12          2       AF
 *      8       14          2       IX
 *      10      16          2       IY
 *      12      18          2       BC'
 *      14      20          2       DE'
 *      16      22          2       HL'
 *      18      24          2       AF'
 *      20      26          1       R
 *      21      27          1       I
 *      22      28          2       SP
 *      24      30          2       PC
 *      26      32          2       0x00 (reserved for future use)
 *      28      34          1       Border color
 *      29      35          1       0x00 (reserved for future use)
 *      30      36          2       Status word
 *      32      38          16384/  RAM dump
 *                          49152
 *
 *  Status word:
 *  Bit     Description
 *  ------- ---------------------------------------------------
 *  15-8    Reserved for future use
 *  7-6     Reserved for internal use (0)
 *  5       Status of the flash attribute: 0=normal/1=inverse
 *  4       Interrupt pending for execution
 *  3       If 1, IM 0; if 0, bit 1 determines interrupt mode
 *              (Spectrum v0.99e had this behaviour reversed,
 *              and this bit was not used in versions previous
 *              to v 0.99e)
 *  2       IFF2: 0=DI/1=EI
 *  1       Interrupt mode (if bit 3 reset): 0=IM1/1=IM2
 *  0       IFF1: 0=DI/1=EI
 *
 *******************************************************************/
void spectrum_state::border_update(int data)
{
#if 0
	spectrum_EventList_Reset(machine());
	spectrum_border_set_last_color(machine(), data);
	spectrum_border_force_redraw(machine());
#endif
}

void spectrum_state::setup_sp(uint8_t *snapdata, uint32_t snapsize)
{
	int i, SP_OFFSET;
	uint8_t intr;
	uint16_t start, size, data, status;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (snapsize == SP_NEW_SIZE_16K || snapsize == SP_NEW_SIZE_48K)
	{
		SP_OFFSET = 0;
		start = (snapdata[SP_OFFSET +  5] << 8) | snapdata[SP_OFFSET +  4];
		size = (snapdata[SP_OFFSET +  3] << 8) | snapdata[SP_OFFSET +  2];
	}
	else
	{
		SP_OFFSET = SP_OLD_HDR - SP_NEW_HDR;
		start = BASE_RAM;
		size = 3*SPECTRUM_BANK;
	}

	data = (snapdata[SP_OFFSET + 13] << 8) | snapdata[SP_OFFSET + 12];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[SP_OFFSET +  7] << 8) | snapdata[SP_OFFSET +  6];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[SP_OFFSET +  9] << 8) | snapdata[SP_OFFSET +  8];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[SP_OFFSET + 11] << 8) | snapdata[SP_OFFSET + 10];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[SP_OFFSET + 25] << 8) | snapdata[SP_OFFSET + 24];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[SP_OFFSET + 19] << 8) | snapdata[SP_OFFSET + 18];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[SP_OFFSET + 21] << 8) | snapdata[SP_OFFSET + 20];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[SP_OFFSET + 23] << 8) | snapdata[SP_OFFSET + 22];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[SP_OFFSET + 15] << 8) | snapdata[SP_OFFSET + 14];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[SP_OFFSET + 17] << 8) | snapdata[SP_OFFSET + 16];
	m_maincpu->set_state_int(Z80_IY, data);


	data = snapdata[SP_OFFSET + 26];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[SP_OFFSET + 27];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[SP_OFFSET + 29] << 8) | snapdata[SP_OFFSET + 28];
	m_maincpu->set_state_int(Z80_SP, data);

	data = (snapdata[SP_OFFSET + 31] << 8) | snapdata[SP_OFFSET + 30];
	m_maincpu->set_state_int(Z80_PC, data);


	status = (snapdata[SP_OFFSET + 37] << 8) | snapdata[SP_OFFSET + 36];

	data = (BIT(status, 3) << 1) | BIT(status, 1);
	switch(data)
	{
		case 0: // case 2: in version 0.99e of the emulator
		m_maincpu->set_state_int(Z80_IM, 1);
		break;
		case 1: // case 3: in version 0.99e of the emulator
		m_maincpu->set_state_int(Z80_IM, 2);
		break;
		case 2: // case 0: in version 0.99e of the emulator
		case 3: // case 1: in version 0.99e of the emulator
		m_maincpu->set_state_int(Z80_IM, (uint64_t)0);
	}

	data = BIT(status, 0);
	m_maincpu->set_state_int(Z80_IFF1, data);

	data = BIT(status, 2);
	m_maincpu->set_state_int(Z80_IFF2, data);

	intr = BIT(status, 4) ? ASSERT_LINE : CLEAR_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	data = BIT(status, 5);
	m_flash_invert = data;
	logerror("FLASH state: %s\n", data ? "PAPER on INK" : "INK on PAPER");

	// Memory dump
	logerror("Loading %04X bytes of RAM at %04X\n", size, start);
	for (i = 0; i < size; i++)
		space.write_byte(start + i, snapdata[SP_OFFSET + SP_NEW_HDR + i]);

	// Set border color
	data = snapdata[SP_OFFSET + 34] & 0x07;
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	page_basicrom();

	//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
}

/*******************************************************************
 *
 *      Load a .SNA file.
 *
 *      This format was used by Arnt Gulbrandsen for its JPP
 *      emulator, and it's based on the format used by the Mirage
 *      Microdriver, a Microdrive backup accessory.
 *
 *      http://www.worldofspectrum.org/infoseekid.cgi?id=1000266
 *
 *      The 48K image is always 49179 bytes long.
 *
 *      .SNA snapshots save the PC on the top of the stack, thus
 *      deleting the original bytes: not a good feature for a snapshot...
 *
 *      It's been reported that snapshots saved at critical moments would
 *      subsequently crash on restore due to the fact that the stack was full
 *      or pointed to a ROM address. A workaround for this situation is to
 *      store 0x0000 into the word where the PC was located.
 *
 *      For sake of fidelity, the code that zeroes the top of the stack has
 *      been written but commented out.
 *
 *      Later, the format has been extended to store the whole Spectrum 128 status.
 *
 *      48K format as follows:
 *
 *      Offset  Size    Description (all registers stored with LSB first)
 *      ------- ------- -------------------------------------------------
 *      0       1       I
 *      1       2       HL'
 *      3       2       DE'
 *      5       2       BC'
 *      7       2       AF'
 *      9       2       HL
 *      11      2       DE
 *      13      2       BC
 *      15      2       IY
 *      17      2       IX
 *      19      1       IFF1/2: bit 0 contains IFF1, bit 2 contains IFF2 (0=DI/1=EI)
 *      20      1       R
 *      21      2       AF
 *      23      2       SP
 *      25      1       Interrupt mode (0=IM0/1=IM1/2=IM2)
 *      26      1       Border color
 *      27      49152   RAM dump
 *
 *      PC is stored on stack.
 *
 *      128K format as follows:
 *
 *      Offset  Size    Description (all registers stored with LSB first)
 *      ------- ------- -------------------------------------------------
 *      0       1       I
 *      1       2       HL'
 *      3       2       DE'
 *      5       2       BC'
 *      7       2       AF'
 *      9       2       HL
 *      11      2       DE
 *      13      2       BC
 *      15      2       IY
 *      17      2       IX
 *      19      1       IFF1/2: bit 0 contains IFF1, bit 2 contains IFF2 (0=DI/1=EI)
 *      20      1       R
 *      21      2       AF
 *      23      2       SP
 *      25      1       Interrupt mode (0=IM0/1=IM1/2=IM2)
 *      26      1       Border color
 *      27      16384   RAM bank 5 (0x4000-0x7fff)
 *      16411   16384   RAM bank 2 (0x8000-0xbfff)
 *      32795   16384   RAM bank n (0xc000-0xffff - currently paged bank)
 *      49179   2       PC
 *      49181   1       Port 0x7FFD data
 *      49182   1       TR-DOS rom paged (0=no/1=yes)
 *      49183   81920/  Remaining RAM banks in ascending order
 *              98304
 *
 *      PC is NOT stored on stack.
 *
 *      The bank in 0xc000 is always included even if it is page 2 or 5
 *      in which case it is included twice.
 *
 *******************************************************************/
void spectrum_state::setup_sna(uint8_t *snapdata, uint32_t snapsize)
{
	int i, j, usedbanks[8];
	long bank_offset;
	uint8_t intr;
	uint16_t data, addr;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if ((snapsize != SNA48_SIZE) && (m_port_7ffd_data == -1))
	{
		logerror("Can't load 128K .SNA file into 48K machine\n");
		return;
	}


	data = (snapdata[SNA48_OFFSET + 22] << 8) | snapdata[SNA48_OFFSET + 21];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[SNA48_OFFSET + 14] << 8) | snapdata[SNA48_OFFSET + 13];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[SNA48_OFFSET + 12] << 8) | snapdata[SNA48_OFFSET + 11];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[SNA48_OFFSET + 10] << 8) | snapdata[SNA48_OFFSET +  9];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[SNA48_OFFSET +  8] << 8) | snapdata[SNA48_OFFSET +  7];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[SNA48_OFFSET +  6] << 8) | snapdata[SNA48_OFFSET +  5];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[SNA48_OFFSET +  4] << 8) | snapdata[SNA48_OFFSET +  3];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[SNA48_OFFSET +  2] << 8) | snapdata[SNA48_OFFSET +  1];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[SNA48_OFFSET + 18] << 8) | snapdata[SNA48_OFFSET + 17];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[SNA48_OFFSET + 16] << 8) | snapdata[SNA48_OFFSET + 15];
	m_maincpu->set_state_int(Z80_IY, data);


	data = snapdata[SNA48_OFFSET + 20];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[SNA48_OFFSET +  0];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[SNA48_OFFSET + 24] << 8) | snapdata[SNA48_OFFSET + 23];
	m_maincpu->set_state_int(Z80_SP, data);


	data = snapdata[SNA48_OFFSET + 25] & 0x03;
	if (data == 3)
		data = 2;
	m_maincpu->set_state_int(Z80_IM, data);

	data = snapdata[SNA48_OFFSET + 19];
	m_maincpu->set_state_int(Z80_IFF1, BIT(data, 0));
	m_maincpu->set_state_int(Z80_IFF2, BIT(data, 2));

	intr = BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	if (snapsize == SNA48_SIZE)
		// 48K Snapshot
		page_basicrom();
	else
	{
		// 128K Snapshot
		m_port_7ffd_data = snapdata[SNA128_OFFSET + 2];
		logerror ("Port 7FFD:%02X\n", m_port_7ffd_data);
		if (snapdata[SNA128_OFFSET + 3])
		{
			/* TODO: page TR-DOS ROM when supported */
		}
		update_paging();
	}

	if (snapsize == SNA48_SIZE)
	{
		// Memory dump
		logerror("Loading %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
		for (i = 0; i < 3*SPECTRUM_BANK; i++)
			space.write_byte(BASE_RAM + i, snapdata[SNA48_HDR + i]);

		// Get PC from stack
		addr = m_maincpu->state_int(Z80_SP);

		if (addr < BASE_RAM || addr > 4*SPECTRUM_BANK - 2)
			logerror("Corrupted SP out of range:%04X", addr);
		else
			logerror("Fetching PC from the stack at SP:%04X\n", addr);

		data = (space.read_byte(addr + 1) << 8) | space.read_byte(addr + 0);
		m_maincpu->set_state_int(Z80_PC, data);

#if 0
		space.write_byte(addr + 0, 0); // It's been reported that zeroing these locations fixes the loading
		space.write_byte(addr + 1, 0); // of a few images that were snapshot at a "wrong" instant
#endif

		addr += 2;
		logerror("Fixing SP:%04X\n", addr);
		m_maincpu->set_state_int(Z80_SP, addr);

		// Set border color
		data = snapdata[SNA48_OFFSET + 26] & 0x07;
		m_port_fe_data = (m_port_fe_data & 0xf8) | data;
		border_update(data);
		logerror("Border color:%02X\n", data);

		//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
	}
	else
	{
		// Set up other RAM banks
		for (i = 0; i < 8; i++)
			usedbanks[i] = 0;

		usedbanks[5] = 1;                               // 0x4000-0x7fff
		usedbanks[2] = 1;                               // 0x8000-0xbfff
		usedbanks[m_port_7ffd_data & 0x07] = 1;    // Banked memory

		logerror("Loading %05X bytes of RAM at %04X\n", 8*SPECTRUM_BANK, BASE_RAM);
		logerror("Loading bank 5 from offset:0001B\n");
		logerror("Loading bank 2 from offset:0401B\n");
		logerror("Loading bank %d from offset:0801B\n", snapdata[SNA128_OFFSET + 2] & 0x07);
		for (i = 0; i < 3*SPECTRUM_BANK; i++)
			space.write_byte(BASE_RAM + i, snapdata[SNA48_HDR + i]);

		bank_offset = SNA48_SIZE + SNA128_HDR;
		for (i = 0; i < 8; i++)
		{
			if (!usedbanks[i])
			{
				logerror("Loading bank %d from offset:%05lX\n", i, bank_offset);
				m_port_7ffd_data &= 0xf8;
				m_port_7ffd_data += i;
				update_paging();
				for (j = 0; j < SPECTRUM_BANK; j++)
					space.write_byte(j + 3*SPECTRUM_BANK, snapdata[bank_offset + j]);
				bank_offset += SPECTRUM_BANK;
			}
		}

		// program counter
		data = (snapdata[SNA128_OFFSET + 1] << 8) | snapdata[SNA128_OFFSET + 0];
		m_maincpu->set_state_int(Z80_PC, data);

		// Set border color
		data = snapdata[SNA48_OFFSET + 26] & 0x07;
		m_port_fe_data = (m_port_fe_data & 0xf8) | data;
		border_update(data);
		logerror("Border color:%02X\n", data);
		data = m_port_7ffd_data & 0x07;

		// Reset paging
		m_port_7ffd_data = snapdata[SNA128_OFFSET + 2];
		update_paging();

		//logerror("Snapshot loaded.\nExecution resuming at bank:%d %s\n", data, m_maincpu->state_string(Z80_PC).c_str());
	}
}

/*******************************************************************
 *
 *      Load a .ACH file.
 *
 *      .ACH files were produced by a commercial Archimedes
 *      (hence the name) emulator called !Speccy.
 *
 *      The files are always 65792 bytes long and store both
 *      a copy of the ROM and the whole 48K RAM.
 *
 *
 *      Offset  Size    Description (all registers stored with LSB first
 *                      except when noted with *BE*)
 *      ------- ------- -------------------------------------------------
 *      0       4       A
 *      4       4       F
 *      8       4       B
 *      12      4       C
 *      16      4       D
 *      20      4       E
 *      24      4       H
 *      28      4       L
 *      32      4       PC
 *      36      4       0x00 (reserved for future use)
 *      40      4       SP
 *      44      104     0x00 (reserved for future use)
 *      148     4       R
 *      152     4       0x00 (reserved for future use)
 *      156     4       Border color
 *      160     4       0x00 (reserved for future use)
 *      164     2       Interrupt mode (0=IM0/1=IM1/2=IM2)
 *      166     24      0x00 (reserved for future use)
 *      190     1       I
 *      191     1       IFF1/2: (0=DI/1=EI)
 *      192     44      0x00 (reserved for future use)
 *      236     4       AF' *BE*
 *      240     4       BC' *BE*
 *      244     2       DE' *BE*
 *      246     2       HL' *BE*
 *      248     4       IX
 *      252     4       IY
 *      256     16384   ROM dump
 *      16640   49152   RAM dump
 *
 *******************************************************************/
void spectrum_state::setup_ach(uint8_t *snapdata, uint32_t snapsize)
{
	int i;
	uint8_t intr;
	uint16_t data;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	data = (snapdata[ACH_OFFSET +   0] << 8) | snapdata[ACH_OFFSET +   4];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[ACH_OFFSET +   8] << 8) | snapdata[ACH_OFFSET +  12];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[ACH_OFFSET +  16] << 8) | snapdata[ACH_OFFSET +  20];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[ACH_OFFSET +  24] << 8) | snapdata[ACH_OFFSET +  28];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[ACH_OFFSET + 236] << 8) | snapdata[ACH_OFFSET + 237];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[ACH_OFFSET + 240] << 8) | snapdata[ACH_OFFSET + 241];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[ACH_OFFSET + 244] << 8) | snapdata[ACH_OFFSET + 245];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[ACH_OFFSET + 246] << 8) | snapdata[ACH_OFFSET + 247];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[ACH_OFFSET + 249] << 8) | snapdata[ACH_OFFSET + 248];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[ACH_OFFSET + 253] << 8) | snapdata[ACH_OFFSET + 252];
	m_maincpu->set_state_int(Z80_IY, data);

	data = snapdata[ACH_OFFSET + 148];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[ACH_OFFSET + 190];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[ACH_OFFSET +  41] << 8) | snapdata[ACH_OFFSET +  40];
	m_maincpu->set_state_int(Z80_SP, data);

	data = (snapdata[ACH_OFFSET +  33] << 8) | snapdata[ACH_OFFSET +  32];
	m_maincpu->set_state_int(Z80_PC, data);

	data = snapdata[ACH_OFFSET + 164] & 0x03;
	if (data == 3)
		data = 0;
	m_maincpu->set_state_int(Z80_IM, data);

	data = snapdata[ACH_OFFSET + 191] ? 1 : 0;
	m_maincpu->set_state_int(Z80_IFF1, data);
	m_maincpu->set_state_int(Z80_IFF2, data);

	intr = snapdata[ACH_OFFSET + 191] ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	logerror("Skipping the 16K ROM dump at offset:%04X\n", ACH_OFFSET + 256);

	// Memory dump
	logerror("Loading %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
	for (i = 0; i < 3*SPECTRUM_BANK; i++)
		space.write_byte(BASE_RAM + i, snapdata[ACH_HDR + SPECTRUM_BANK + i]);

	// Set border color
	data = snapdata[ACH_OFFSET + 156] & 0x07;
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	page_basicrom();

	//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
}

/*******************************************************************
 *
 *      Load a .PRG file.
 *
 *      .PRG files were produced by an irish emulator called SpecEm
 *      written by Kevin Phair (thanks for the help, Kevin!)
 *
 *      http://code.google.com/p/specem/
 *
 *      The files are always 49408 bytes long and use some stack space
 *      to store registers.
 *
 *      The header actually mimics the file descriptor format used by
 *      the DISCiPLE/+D interface.
 *
 *      Offset  Size    Description (all registers stored with LSB first
 *                      except when noted with *BE*)
 *      ------- ------- -------------------------------------------------
 *      0       1       File type (always 0x05, corresponding to "48K Snapshot"
 *      1       10      Program name (padded with 0x20)
 *      11      2       Number of sectors occupied by the file *BE*
 *      13      1       Track number of the first sector of the file
 *      14      1       Sector number of the first sector of the file
 *      15      195     Sector allocation bitmap
 *      210     10      0x00 (reserved for future use)
 *      220     2       IY
 *      222     2       IX
 *      224     2       DE'
 *      226     2       BC'
 *      228     2       HL'
 *      230     2       AF'
 *      232     2       DE
 *      234     2       BC
 *      236     2       HL
 *      238     1       Junk created when saving I via LD A,I/PUSH AF
 *      239     1       I
 *      240     2       SP
 *      242     14      0x00 (reserved for future use)
 *      256     49152   RAM dump
 *
 *      IFF1/2, R, AF and PC are stored on the stack. Due to a bug in the
 *      BIOS, the snapshots created with the DISCiPLE have the AF' register
 *      replaced with the AF register.
 *
 *      It's unknown (but likely possible) that the snapshots could
 *      suffer from the same "top of the stack" bug as well as .SNA images.
 *
 *******************************************************************/
void spectrum_state::setup_prg(uint8_t *snapdata, uint32_t snapsize)
{
	int i;
	uint8_t intr;
	uint16_t addr, data;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	data = snapdata[PRG_OFFSET +   0];
	if (data != 0x05)
		logerror("Wrong DISCiPLE/+D file type: %02X instead of 05\n", data);

	data = (snapdata[PRG_OFFSET + 235] << 8) | snapdata[PRG_OFFSET + 234];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[PRG_OFFSET + 233] << 8) | snapdata[PRG_OFFSET + 232];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[PRG_OFFSET + 237] << 8) | snapdata[PRG_OFFSET + 236];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[PRG_OFFSET + 231] << 8) | snapdata[PRG_OFFSET + 230];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[PRG_OFFSET + 227] << 8) | snapdata[PRG_OFFSET + 226];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[PRG_OFFSET + 225] << 8) | snapdata[PRG_OFFSET + 224];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[PRG_OFFSET + 229] << 8) | snapdata[PRG_OFFSET + 228];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[PRG_OFFSET + 223] << 8) | snapdata[PRG_OFFSET + 222];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[PRG_OFFSET + 221] << 8) | snapdata[PRG_OFFSET + 220];
	m_maincpu->set_state_int(Z80_IY, data);

	data = snapdata[PRG_OFFSET + 239];
	m_maincpu->set_state_int(Z80_I, data);

	m_maincpu->set_state_int(Z80_IM, (data == 0x00 || data == 0x3f) ? 1 : 2);

	// Memory dump
	logerror("Loading %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
	for (i = 0; i < 3*SPECTRUM_BANK; i++)
		space.write_byte(BASE_RAM + i, snapdata[PRG_HDR + i]);

	addr = (snapdata[PRG_OFFSET + 241] << 8) | snapdata[PRG_OFFSET + 240];
	if (addr < BASE_RAM || addr > 4*SPECTRUM_BANK - 6)
		logerror("Corrupted SP out of range:%04X", addr);
	else
		logerror("Fetching registers IFF1/2, R, AF and PC from the stack at SP:%04X\n", addr);

	data = space.read_byte(addr + 0); // IFF1/2: (bit 2, 0=DI/1=EI)
	m_maincpu->set_state_int(Z80_IFF1, BIT(data, 2));
	m_maincpu->set_state_int(Z80_IFF2, BIT(data, 2));

	intr = BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	data = space.read_byte(addr + 1);
	m_maincpu->set_state_int(Z80_R, data);

	data = (space.read_byte(addr + 3) << 8) | space.read_byte(addr + 2);
	m_maincpu->set_state_int(Z80_AF, data);

	data = (space.read_byte(addr + 5) << 8) | space.read_byte(addr + 4);
	m_maincpu->set_state_int(Z80_PC, data);

#if 0
	space.write_byte(addr + 0, 0); // It's been reported that zeroing these locations fixes the loading
	space.write_byte(addr + 1, 0); // of a few images that were snapshot at a "wrong" instant
	space.write_byte(addr + 2, 0);
	space.write_byte(addr + 3, 0);
	space.write_byte(addr + 4, 0);
	space.write_byte(addr + 5, 0);
#endif

	addr += 6;
	logerror("Fixing SP:%04X\n", addr);
	m_maincpu->set_state_int(Z80_SP, addr);

	// Set border color
	data = (space.read_byte(0x5c48) >> 3) & 0x07; // Get the current border color from BORDCR system variable.
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	page_basicrom();

	//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
}

/*******************************************************************
 *
 *      Load a .PLUSD file.
 *
 *      .PLUSD files are the snapshots produced by the DISCiPLE or
 *      the +D disk interface, thus share the same internal organization
 *      of the .PRG files, with a couple of notable exceptions:
 *
 *      1) The filesystem metadata is missing
 *      2) Spectrum 128 snapshots (file type 9) are allowed
 *
 *      A commented disassembly of both BIOSes is avilable at:
 *
 *      http://www.biehold.nl/rudy/
 *
 *      48K snapshots are 49174 bytes long, 128K snapshots are 131095
 *      bytes long.
 *
 *      Philip Kendall's FUSE emulator at http://fuse-emulator.sourceforge.net
 *      supports this format, but no extension is officially provided.
 *      Therefore I have used .PLUSD - corrections are welcome!
 *
 *      48K snapshot format
 *      -------------------
 *
 *      Offset  Size    Description (all registers stored with LSB first
 *      ------- ------- -------------------------------------------------
 *      0       2       IY
 *      2       2       IX
 *      4       2       DE'
 *      6       2       BC'
 *      8       2       HL'
 *      10      2       AF'
 *      12      2       DE
 *      14      2       BC
 *      16      2       HL
 *      18      1       Junk created when saving I via LD A,I/PUSH AF
 *      19      1       I
 *      20      2       SP
 *      22      49152   RAM dump
 *
 *      128K snapshot format
 *      --------------------
 *
 *      Offset  Size    Description (all registers stored with LSB first
 *      ------- ------- -------------------------------------------------
 *      0       2       IY
 *      2       2       IX
 *      4       2       DE'
 *      6       2       BC'
 *      8       2       HL'
 *      10      2       AF'
 *      12      2       DE
 *      14      2       BC
 *      16      2       HL
 *      18      1       Junk created when saving I via LD A,I/PUSH AF
 *      19      1       I
 *      20      2       SP
 *      22      1       Port 0x7FFD data
 *      23      131072  RAM dump
 *
 *      The 8 RAM banks are stored in the order 0-7.
 *
 *      IFF1/2, R, AF and PC are stored on the stack. Due to a bug in the
 *      BIOS, the snapshots created with the DISCiPLE have the AF' register
 *      replaced with the AF register.
 *
 *      It's unknown (but likely possible) that the snapshots could
 *      suffer from the same "top of the stack" bug as well as .SNA images.
 *
 *******************************************************************/
void spectrum_state::setup_plusd(uint8_t *snapdata, uint32_t snapsize)
{
	int i, j;
	uint8_t intr;
	uint16_t addr = 0, data;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	data = (snapdata[PLUSD_OFFSET + 15] << 8) | snapdata[PLUSD_OFFSET + 14];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[PLUSD_OFFSET + 13] << 8) | snapdata[PLUSD_OFFSET + 12];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[PLUSD_OFFSET + 17] << 8) | snapdata[PLUSD_OFFSET + 16];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[PLUSD_OFFSET + 11] << 8) | snapdata[PLUSD_OFFSET + 10];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[PLUSD_OFFSET +  7] << 8) | snapdata[PLUSD_OFFSET +  6];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[PLUSD_OFFSET +  5] << 8) | snapdata[PLUSD_OFFSET +  4];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[PLUSD_OFFSET +  9] << 8) | snapdata[PLUSD_OFFSET +  8];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[PLUSD_OFFSET +  3] << 8) | snapdata[PLUSD_OFFSET +  2];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[PLUSD_OFFSET +  1] << 8) | snapdata[PLUSD_OFFSET +  0];
	m_maincpu->set_state_int(Z80_IY, data);

	data = snapdata[PLUSD_OFFSET + 19];
	m_maincpu->set_state_int(Z80_I, data);

	m_maincpu->set_state_int(Z80_IM, (data == 0x00 || data == 0x3f) ? 1 : 2);

	if (snapsize == PLUSD48_SIZE)
	{
		page_basicrom();

		// Memory dump
		logerror("Loading %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
		for (i = 0; i < 3*SPECTRUM_BANK; i++)
			space.write_byte(BASE_RAM + i, snapdata[PLUSD48_HDR + i]);
	}
	else
	{
		logerror("Loading %05X bytes of RAM at %04X\n", 8*SPECTRUM_BANK, BASE_RAM);
		for (i = 0; i < 8; i++)
		{
			switch (i)
			{
				case 5: addr = SPECTRUM_BANK;
						break;
				case 2: addr = 2*SPECTRUM_BANK;
						break;
				case 0:
				case 1:
				case 3:
				case 4:
				case 6:
				case 7: addr = 3*SPECTRUM_BANK;
						m_port_7ffd_data &= 0xf8;
						m_port_7ffd_data += i;
						update_paging();
						break;
			};
			logerror("Loading bank %d from offset:%05X\n", i, PLUSD128_HDR + i*SPECTRUM_BANK);
			for (j = 0; j < SPECTRUM_BANK; j++)
				space.write_byte(j + addr, snapdata[j + PLUSD128_HDR + i*SPECTRUM_BANK]);
		}
		m_port_7ffd_data = snapdata[PLUSD_OFFSET + 22];
		logerror("Port 7FFD:%02X\n", m_port_7ffd_data);
		logerror("Paging bank:%d\n", m_port_7ffd_data & 0x07);
		update_paging();
	}

	addr = (snapdata[PLUSD_OFFSET + 21] << 8) | snapdata[PLUSD_OFFSET + 20];
	if (addr < BASE_RAM || addr > 4*SPECTRUM_BANK - 6)
		logerror("Corrupted SP out of range:%04X", addr);
	else
		logerror("Fetching registers IFF1/2, R, AF and PC from the stack at SP:%04X\n", addr);

	data = space.read_byte(addr + 0); // IFF1/2: (bit 2, 0=DI/1=EI)
	m_maincpu->set_state_int(Z80_IFF1, BIT(data, 2));
	m_maincpu->set_state_int(Z80_IFF2, BIT(data, 2));

	intr = BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	data = space.read_byte(addr + 1);
	m_maincpu->set_state_int(Z80_R, data);

	data = (space.read_byte(addr + 3) << 8) | space.read_byte(addr + 2);
	m_maincpu->set_state_int(Z80_AF, data);

	data = (space.read_byte(addr + 5) << 8) | space.read_byte(addr + 4);
	m_maincpu->set_state_int(Z80_PC, data);

#if 0
	space.write_byte(addr + 0, 0); // It's been reported that zeroing these locations fixes the loading
	space.write_byte(addr + 1, 0); // of a few images that were snapshot at a "wrong" instant
	space.write_byte(addr + 2, 0);
	space.write_byte(addr + 3, 0);
	space.write_byte(addr + 4, 0);
	space.write_byte(addr + 5, 0);
#endif

	addr += 6;
	logerror("Fixing SP:%04X\n", addr);
	m_maincpu->set_state_int(Z80_SP, addr);

	// Set border color
	data = (space.read_byte(0x5c48) >> 3) & 0x07; // Get the current border color from BORDCR system variable.
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	//if (snapsize == PLUSD48_SIZE)
		//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
	//else
		//logerror("Snapshot loaded.\nExecution resuming at bank:%d %s\n", m_port_7ffd_data & 0x07, m_maincpu->state_string(Z80_PC).c_str());
}

/*******************************************************************
 *
 *      Load a .SEM file.
 *
 *      .SEM files were produced by a german emulator called SpecEmu
 *      written by Bernd Waschke.
 *
 *      The files are usually 49192 bytes long unless optional POKE
 *      blocks are stored at the end of the file.
 *
 *      Offset  Size    Description (all registers stored with LSB first)
 *      ------- ------- -------------------------------------------------
 *      0       1       0x05 (signature length)
 *      1       5       "SPEC1" (signature)
 *      6       49152   RAM dump
 *      49158   2       AF
 *      49160   2       BC
 *      49162   2       DE
 *      49164   2       HL
 *      49166   2       AF'
 *      49168   2       BC'
 *      49170   2       DE'
 *      49172   2       HL'
 *      49174   2       PC
 *      49176   2       SP
 *      49178   2       IX
 *      49180   2       IY
 *      49182   2       I
 *      49184   2       R
 *      49186   2       IFF1 (0=DI/1=EI)
 *      49188   2       IFF2 (0=DI/1=EI)
 *      49190   2       Interrupt mode (0=IM0/1=IM1/2=IM2)
 *
 *      Following these data, there are optional POKE blocks
 *
 *******************************************************************/
void spectrum_state::setup_sem(uint8_t *snapdata, uint32_t snapsize)
{
	int i;
	uint8_t intr;
	uint16_t data;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	data = (snapdata[SEM_OFFSET +  1] << 8) | snapdata[SEM_OFFSET +  0];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[SEM_OFFSET +  3] << 8) | snapdata[SEM_OFFSET +  2];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[SEM_OFFSET +  5] << 8) | snapdata[SEM_OFFSET +  4];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[SEM_OFFSET +  7] << 8) | snapdata[SEM_OFFSET +  6];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[SEM_OFFSET +  9] << 8) | snapdata[SEM_OFFSET +  8];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[SEM_OFFSET + 11] << 8) | snapdata[SEM_OFFSET + 10];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[SEM_OFFSET + 13] << 8) | snapdata[SEM_OFFSET + 12];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[SEM_OFFSET + 15] << 8) | snapdata[SEM_OFFSET + 14];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[SEM_OFFSET + 21] << 8) | snapdata[SEM_OFFSET + 20];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[SEM_OFFSET + 23] << 8) | snapdata[SEM_OFFSET + 22];
	m_maincpu->set_state_int(Z80_IY, data);


	data = snapdata[SEM_OFFSET + 26];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[SEM_OFFSET + 24];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[SEM_OFFSET + 19] << 8) | snapdata[SEM_OFFSET + 18];
	m_maincpu->set_state_int(Z80_SP, data);

	data = (snapdata[SEM_OFFSET + 17] << 8) | snapdata[SEM_OFFSET + 16];
	m_maincpu->set_state_int(Z80_PC, data);

	data = snapdata[SEM_OFFSET + 32];
	m_maincpu->set_state_int(Z80_IM, data);

	data = snapdata[SEM_OFFSET + 28];
	m_maincpu->set_state_int(Z80_IFF1, BIT(data, 0));
	data = snapdata[SEM_OFFSET + 30];
	m_maincpu->set_state_int(Z80_IFF2, BIT(data, 0));

	intr = BIT(snapdata[SEM_OFFSET + 30], 0) ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// Memory dump
	logerror("Loading %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
	for (i = 0; i < 3*SPECTRUM_BANK; i++)
		space.write_byte(BASE_RAM + i, snapdata[SEM_SIGNATURE + i]);

	// Set border color
	data = (space.read_byte(0x5c48) >> 3) & 0x07; // Get the current border color from BORDCR system variable.
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	page_basicrom();

	//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());

/* TODO: Decode the optional POKE bank at the end of the image */

}

/*******************************************************************
 *
 *      Load a .SIT file.
 *
 *      .SIT files were produced by a spanish emulator called Sinclair
 *      written by Pedro M. R. Salas.
 *
 *      http://www.ugr.es/~pedrom/sinclair.htm
 *
 *      The files are always 65564 bytes long and store both ROM and RAM.
 *
 *      Offset  Size    Description (all registers stored with LSB first)
 *      ------- ------- -------------------------------------------------
 *      0       2       BC
 *      2       2       DE
 *      4       2       HL
 *      6       2       AF
 *      8       2       IX
 *      10      2       IY
 *      12      2       SP
 *      14      2       PC
 *      16      1       R
 *      17      1       I
 *      18      2       BC'
 *      20      2       DE'
 *      22      2       HL'
 *      24      2       AF'
 *      26      1       Interrupt mode (0=IM0/1=IM1/2=IM2)
 *      27      1       Border color
 *      28      16384   ROM dump
 *      16412   49152   RAM dump
 *
 *******************************************************************/
void spectrum_state::setup_sit(uint8_t *snapdata, uint32_t snapsize)
{
	int i;
	uint8_t intr;
	uint16_t data;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	data = (snapdata[SIT_OFFSET +  7] << 8) | snapdata[SIT_OFFSET +  6];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[SIT_OFFSET +  1] << 8) | snapdata[SIT_OFFSET +  0];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[SIT_OFFSET +  3] << 8) | snapdata[SIT_OFFSET +  2];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[SIT_OFFSET +  5] << 8) | snapdata[SIT_OFFSET +  4];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[SIT_OFFSET + 25] << 8) | snapdata[SIT_OFFSET + 24];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[SIT_OFFSET + 19] << 8) | snapdata[SIT_OFFSET + 18];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[SIT_OFFSET + 21] << 8) | snapdata[SIT_OFFSET + 20];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[SIT_OFFSET + 23] << 8) | snapdata[SIT_OFFSET + 22];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[SIT_OFFSET +  9] << 8) | snapdata[SIT_OFFSET +  8];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[SIT_OFFSET + 11] << 8) | snapdata[SIT_OFFSET + 10];
	m_maincpu->set_state_int(Z80_IY, data);


	data = snapdata[SIT_OFFSET + 16];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[SIT_OFFSET + 17];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[SIT_OFFSET + 13] << 8) | snapdata[SIT_OFFSET + 12];
	m_maincpu->set_state_int(Z80_SP, data);

	data = (snapdata[SIT_OFFSET + 15] << 8) | snapdata[SIT_OFFSET + 14];
	m_maincpu->set_state_int(Z80_PC, data);

	data = snapdata[SIT_OFFSET + 26];
	m_maincpu->set_state_int(Z80_IM, data);

	data = 0x01; // .SIT snapshots don't specify whether interrupts are enabled or not, so I assume they are.
	m_maincpu->set_state_int(Z80_IFF1, data);
	m_maincpu->set_state_int(Z80_IFF2, data);

	intr = data ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// Memory dump
	logerror("Skipping the 16K ROM dump at offset:%04X\n", SIT_OFFSET + 28);
	logerror("Loading %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
	for (i = 0; i < 3*SPECTRUM_BANK; i++)
		space.write_byte(BASE_RAM + i, snapdata[SIT_HDR + SPECTRUM_BANK + i]);

	// Set border color
	data = snapdata[SIT_OFFSET + 27] & 0x07;
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	page_basicrom();

	//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
}

/*******************************************************************
 *
 *      Load a .ZX file.
 *
 *      .ZX files were produced by the Amiga emulator called KGB
 *
 *      The files are usually 49486 bytes and store at the beginning
 *      of the file the last 132 bytes of the ROM.
 *
 *      Offset  Size    Description (all registers stored with MSB first)
 *      ------- ------- -------------------------------------------------
 *      0       132     Last 132 bytes of ROM dump
 *      132     49152   RAM dump
 *      49284   132     0x00 (reserved for future use)
 *      49416   10      Various KGB settings
 *      49426   1       IFF1/2: (0=DI/1=EI)
 *      49427   2       Reserved (must be 0x0003)
 *      49429   1       KGB ColorMode (0=BW/1=Color)
 *      49430   4       0x00 (reserved for future use)
 *      49434   2       BC
 *      49436   2       BC'
 *      49438   2       DE
 *      49440   2       DE'
 *      49442   2       HL
 *      49444   2       HL'
 *      49446   2       IX
 *      49448   2       IY
 *      49450   1       I
 *      49451   1       R
 *      49452   3       0x00 (reserved for future use)
 *      49455   1       A'
 *      49456   1       0x00 (reserved for future use)
 *      49457   1       A
 *      49458   1       0x00 (reserved for future use)
 *      49459   1       F'
 *      49460   1       0x00 (reserved for future use)
 *      49461   1       F
 *      49462   2       0x00 (reserved for future use)
 *      49464   2       PC
 *      49466   2       0x00 (reserved for future use)
 *      49468   2       SP
 *      49470   2       KGB Soundmode (0=Simple/1=Pitch/2=RomOnly)
 *      49472   2       KGB HaltMode (0=NoHalt/1=Halt)
 *      49474   2       Interrupt mode (-1=IM0/0=IM1/1=IM2)
 *      49476   10      0x00 (reserved for future use)
 *
 *******************************************************************/
void spectrum_state::setup_zx(uint8_t *snapdata, uint32_t snapsize)
{
	int i;
	uint8_t intr;
	uint16_t data, mode;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	logerror("Skipping last 132 bytes of the 16K ROM dump at offset:0000\n");

	data = (snapdata[ZX_OFFSET + 173] << 8) | snapdata[ZX_OFFSET + 177];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[ZX_OFFSET + 150] << 8) | snapdata[ZX_OFFSET + 151];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[ZX_OFFSET + 154] << 8) | snapdata[ZX_OFFSET + 155];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[ZX_OFFSET + 158] << 8) | snapdata[ZX_OFFSET + 159];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[ZX_OFFSET + 171] << 8) | snapdata[ZX_OFFSET + 175];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[ZX_OFFSET + 152] << 8) | snapdata[ZX_OFFSET + 153];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[ZX_OFFSET + 156] << 8) | snapdata[ZX_OFFSET + 157];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[ZX_OFFSET + 160] << 8) | snapdata[ZX_OFFSET + 161];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[ZX_OFFSET + 162] << 8) | snapdata[ZX_OFFSET + 163];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[ZX_OFFSET + 164] << 8) | snapdata[ZX_OFFSET + 165];
	m_maincpu->set_state_int(Z80_IY, data);


	data = snapdata[ZX_OFFSET + 167];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[ZX_OFFSET + 166];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[ZX_OFFSET + 184] << 8) | snapdata[ZX_OFFSET + 185];
	m_maincpu->set_state_int(Z80_SP, data);

	data = (snapdata[ZX_OFFSET + 180] << 8) | snapdata[ZX_OFFSET + 181];
	m_maincpu->set_state_int(Z80_PC, data);

	mode = (snapdata[ZX_OFFSET + 190] << 8) | snapdata[ZX_OFFSET + 191];
	switch (mode)
	{
		case 0xffff:
		m_maincpu->set_state_int(Z80_IM, (uint64_t)0);
		break;
		case 0x00:
		m_maincpu->set_state_int(Z80_IM, 1);
		break;
		case 0x01:
		m_maincpu->set_state_int(Z80_IM, 2);
		break;
		default:
		logerror("Invalid IM:%04X\n", mode);
	}

	data = BIT(snapdata[ZX_OFFSET + 142], 0);
	m_maincpu->set_state_int(Z80_IFF1, data);
	m_maincpu->set_state_int(Z80_IFF2, data);

	intr = BIT(snapdata[ZX_OFFSET + 142], 0) ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// Memory dump
	logerror("Loading %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
	for (i = 0; i < 3*SPECTRUM_BANK; i++)
		space.write_byte(BASE_RAM + i, snapdata[132 + i]);

	// Set border color
	data = (space.read_byte(0x5c48) >> 3) & 0x07; // Get the current border color from BORDCR system variable.
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	page_basicrom();

	//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
}

/*******************************************************************
 *
 *      Load a .SNP file.
 *
 *      .SNP files were produced by a polish emulator called Nuclear ZX
 *      written by Radovan Garabik and Lubomir Salanci.
 *
 *      http://korpus.juls.savba.sk/~garabik/old/zx.html
 *
 *      The files are always 49183 bytes long.
 *
 *      Offset  Size    Description (all registers stored with LSB first)
 *      ------- ------- -------------------------------------------------
 *      0       49152   RAM dump
 *      49152   2       AF
 *      49154   1       Border color
 *      49155   1       0x00 (reserved for future use)
 *      49156   2       BC
 *      49158   2       DE
 *      49160   2       HL
 *      49162   2       PC
 *      49164   2       SP
 *      49166   2       IX
 *      49168   2       IY
 *      49170   1       0x00 (reserved for IFF2 but not implemented)
 *      49171   1       IFF1 (0=DI/other=EI)
 *      49172   1       Interrupt mode (0=IM0/1=IM1/2=IM2)
 *      49173   1       R
 *      49174   1       I
 *      49175   2       AF'
 *      49177   2       BC'
 *      49179   2       DE'
 *      49181   2       HL'
 *
 *******************************************************************/
void spectrum_state::setup_snp(uint8_t *snapdata, uint32_t snapsize)
{
	int i;
	uint8_t intr;
	uint16_t data;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	data = (snapdata[SNP_OFFSET +  1] << 8) | snapdata[SNP_OFFSET +  0];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[SNP_OFFSET +  5] << 8) | snapdata[SNP_OFFSET +  4];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[SNP_OFFSET +  7] << 8) | snapdata[SNP_OFFSET +  6];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[SNP_OFFSET +  9] << 8) | snapdata[SNP_OFFSET +  8];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[SNP_OFFSET + 24] << 8) | snapdata[SNP_OFFSET + 23];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[SNP_OFFSET + 26] << 8) | snapdata[SNP_OFFSET + 25];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[SNP_OFFSET + 28] << 8) | snapdata[SNP_OFFSET + 27];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[SNP_OFFSET + 30] << 8) | snapdata[SNP_OFFSET + 29];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[SNP_OFFSET + 15] << 8) | snapdata[SNP_OFFSET + 14];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[SNP_OFFSET + 17] << 8) | snapdata[SNP_OFFSET + 16];
	m_maincpu->set_state_int(Z80_IY, data);


	data = snapdata[SNP_OFFSET + 21];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[SNP_OFFSET + 22];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[SNP_OFFSET + 13] << 8) | snapdata[SNP_OFFSET + 12];
	m_maincpu->set_state_int(Z80_SP, data);

	data = (snapdata[SNP_OFFSET + 11] << 8) | snapdata[SNP_OFFSET + 10];
	m_maincpu->set_state_int(Z80_PC, data);


	data = snapdata[SNP_OFFSET + 20] & 0x03;
	m_maincpu->set_state_int(Z80_IM, data);

	data = BIT(snapdata[SNP_OFFSET + 19], 0);
	m_maincpu->set_state_int(Z80_IFF1, data);
	m_maincpu->set_state_int(Z80_IFF2, data);

	intr = BIT(snapdata[SNP_OFFSET + 19], 0) ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// Memory dump
	logerror("Loading %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
	for (i = 0; i < 3*SPECTRUM_BANK; i++)
		space.write_byte(BASE_RAM + i, snapdata[i]);

	// Set border color
	data = snapdata[SNP_OFFSET +  2] & 0x07;
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	page_basicrom();

	//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
}

/*******************************************************************
 *
 *      Load a .SNX file.
 *
 *      .SNX files were produced by an Atari ST emulator called Specci
 *      written by Christian Gandler
 *
 *      http://cd.textfiles.com/crawlycrypt1/apps/misc/zx_sp207/
 *
 *      The header is an extension of the .SNA format and the RAM dump
 *      is compressed with a simple run-length algorithm.
 *
 *      Offset  Size    Description (all registers stored with LSB first
 *                      except when noted with *BE*)
 *      ------- ------- -------------------------------------------------
 *      0       4       "XSNA" (signature)
 *      4       2       Header length *BE*
 *      6       1       I
 *      7       2       HL'
 *      9       2       DE'
 *      11      2       BC'
 *      13      2       AF'
 *      15      2       HL
 *      17      2       DE
 *      19      2       BC
 *      21      2       IY
 *      23      2       IX
 *      25      1       IFF1/2: bit 0 contains IFF1, bit 2 contains IFF2 (0=DI/1=EI)
 *      26      1       R
 *      27      2       AF
 *      29      2       SP
 *      31      1       Interrupt Mode (0=IM0/1=IM1/2=IM2)
 *      32      1       Border color
 *      33      1       Specci settings: Interface 1 (0=no/1=yes)
 *      34      1       Specci settings: FLASH emulation (0=no/1=yes)
 *      35      1       Specci settings: Attributes emulation (0=no/1=yes)
 *      36      1       Specci settings: Bit 7 - Keyboard (0=QWERTY/1=QWERTZ)
 *                                       Bit 6 - ULA emulation (0=no/1=yes)
 *                                       Bit 0,1 - Joystick interface
 *                                            00 = Kempston
 *                                            01 = IF2 Left
 *                                            10 = IF2 Right
 *                                            11 = Cursor/AGF/Protek
 *      37      1       Specci settings: R register emulation (0=no/1=yes)
 *                                       Bit 7 - EAR bit (0=Issue 3/1=Issue 2)
 *      38      1       Specci settings: Interrupt frequency (0=50 Hz/1=100 Hz)
 *      39      1       Specci settings: RS232 redirection (0=RS232/1=Centronics)
 *      40      1       Specci settings: Sound emulation
 *                                       Bit 4,7 - Frequency (0..4) for mode 2
 *                                       Bit 0,3 - Mode (0=off/1=direct/2=interrupt)
 *      41      1       Specci settings: Border emulation (0=off/1=direct/2=interrupt)
 *      42      1       Specci settings: IM2 hardware vector (0..255)
 *      43      ?????   RAM dump
 *
 *      PC is stored on stack.
 *
 *      The RAM dump is divided into short or long datablocks:
 *      if the block is composed of the same byte, it is replaced by
 *      a compressed version. The format of the various blocks follows:
 *
 *      Uncompressed short block
 *      ------------------------
 *      Offset  Size    Description
 *      ------- ------- -------------------------------------------------
 *      0       1       Marker byte. Values between 0xe0 and 0xef.
 *                      Low nibble is (block length - 1).
 *      1       ?????   Data block
 *
 *      Compressed short block
 *      ----------------------
 *      Offset  Size    Description
 *      ------- ------- -------------------------------------------------
 *      0       1       Marker byte. Values between 0xf0 and 0xff.
 *                      Low nibble is (block length - 1).
 *      1       1       Filler byte. This byte is repeated for the whole
 *                      length of the block.
 *
 *      Uncompressed long block
 *      -----------------------
 *      Offset  Size    Description
 *      ------- ------- -------------------------------------------------
 *      0       2       Block length in big endian format.
 *                      Values between 0x0000 and 0xdfff.
 *      2       1       Uncompressed block flag: 0xff.
 *      3       ?????   Data block
 *
 *      Compressed long block
 *      ---------------------
 *      Offset  Size    Description
 *      ------- ------- -------------------------------------------------
 *      0       2       Block length in big endian format.
 *                      Values between 0x0000 and 0xdfff.
 *      2       1       Compressed block flag: 0x00.
 *      3       1       Filler byte. This byte is repeated for the whole
 *                      length of the block.
 *
 *******************************************************************/
void spectrum_state::snx_decompress_block(address_space &space, uint8_t *source, uint16_t dest, uint16_t size)
{
	uint8_t counthi, countlo, compress, fill;
	uint16_t block = 0, count, i, j, numbytes;
	i = SNX_HDR - 1;
	numbytes = 0;

	while (numbytes < size)
	{
		counthi = source[++i];
		if (counthi >= 0xe0)
		{
			count = (counthi & 0x0f) + 1;     // Short block
			if ((counthi & 0xf0) == 0xf0)
				compress = SNX_COMPRESSED;
			else
				compress = SNX_UNCOMPRESSED;
			logerror("Block:%05d  Type:Short  Compr:%s  Offset:%04X  Len:%04X  ", block++, compress == SNX_COMPRESSED ? "Y" : "N", i-1, count);
		}
		else
		{
			countlo = source[++i];
			count = (counthi << 8) | countlo; // Long block
			compress = source[++i];
			logerror("Block:%05d  Type:Long   Compr:%s  Offset:%04X  Len:%04X  ", block++, compress == SNX_COMPRESSED ? "Y" : "N", i-3, count);
		}

		if (compress == SNX_COMPRESSED)
		{
			fill = source[++i];
			logerror("Dest:%04X  Filler:%02X\n", BASE_RAM + numbytes, fill);
			for(j = 0; j < count; j++)
				space.write_byte(BASE_RAM + numbytes + j, fill);
			numbytes += count;
		}
		else
		{
			logerror("Dest:%04X\n", BASE_RAM + numbytes);
			j = 0;
			while (j < count)
				space.write_byte(BASE_RAM + numbytes + j++, source[++i]);
			numbytes += count;
		}
	}
}

void spectrum_state::setup_snx(uint8_t *snapdata, uint32_t snapsize)
{
	uint8_t intr;
	uint16_t data, addr;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	data = (snapdata[SNX_OFFSET +  4] << 8) | snapdata[SNX_OFFSET +  5];
	if (data != 0x25)
		logerror("Corrupted header length: %02X instead of 0x25\n", data);

	data = (snapdata[SNX_OFFSET + 28] << 8) | snapdata[SNX_OFFSET + 27];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[SNX_OFFSET + 20] << 8) | snapdata[SNX_OFFSET + 19];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[SNX_OFFSET + 18] << 8) | snapdata[SNX_OFFSET + 17];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[SNX_OFFSET + 16] << 8) | snapdata[SNX_OFFSET + 15];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[SNX_OFFSET + 14] << 8) | snapdata[SNX_OFFSET + 13];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[SNX_OFFSET + 12] << 8) | snapdata[SNX_OFFSET + 11];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[SNX_OFFSET + 10] << 8) | snapdata[SNX_OFFSET +  9];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[SNX_OFFSET +  8] << 8) | snapdata[SNX_OFFSET +  7];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[SNX_OFFSET + 24] << 8) | snapdata[SNX_OFFSET + 23];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[SNX_OFFSET + 22] << 8) | snapdata[SNX_OFFSET + 21];
	m_maincpu->set_state_int(Z80_IY, data);


	data = snapdata[SNX_OFFSET + 26];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[SNX_OFFSET +  6];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[SNX_OFFSET + 30] << 8) | snapdata[SNX_OFFSET + 29];
	m_maincpu->set_state_int(Z80_SP, data);


	data = snapdata[SNX_OFFSET + 31] & 0x03;
	if (data == 3)
		data = 2;
	m_maincpu->set_state_int(Z80_IM, data);

	data = snapdata[SNX_OFFSET + 25];
	m_maincpu->set_state_int(Z80_IFF1, BIT(data, 0));
	m_maincpu->set_state_int(Z80_IFF2, BIT(data, 2));

	intr = BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// Memory dump
	logerror("Uncompressing %04X bytes of RAM at %04X\n", 3*SPECTRUM_BANK, BASE_RAM);
	snx_decompress_block(space, snapdata, BASE_RAM, 3*SPECTRUM_BANK);

	// get pc from stack
	addr = m_maincpu->state_int(Z80_SP);

	if (addr < BASE_RAM || addr > 4*SPECTRUM_BANK - 2)
		logerror("Corrupted SP out of range:%04X", addr);
	else
		logerror("Fetching PC from the stack at SP:%04X\n", addr);

	m_maincpu->set_state_int(Z80_PC, (space.read_byte(addr + 1) << 8) | space.read_byte(addr + 0));

#if 0
	space.write_byte(addr + 0, 0); // It's been reported that zeroing these locations fixes the loading
	space.write_byte(addr + 1, 0); // of a few images that were snapshot at a "wrong" instant
#endif

	addr += 2;
	logerror("Fixed the stack at SP:%04X\n", addr);
	m_maincpu->set_state_int(Z80_SP, addr);

	// Set border color
	data = snapdata[SNX_OFFSET + 32] & 0x07;
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	page_basicrom();

/* TODO: Enable/disable IF1 as per snapdata[SNX_OFFSET + 33] */

/* TODO: Enable/disable joysticks as per snapdata[SNX_OFFSET + 36] */

/* TODO: Enable selection of Issue 2/3 config switch as per snapdata[SNX_OFFSET + 37] */

	//logerror("Snapshot loaded.\nExecution resuming at %s\n", m_maincpu->state_string(Z80_PC).c_str());
}

/*******************************************************************
 *
 *      Load a .FRZ file.
 *
 *      .FRZ files were produced by the czech Amiga emulator CBSpeccy
 *      written by the CodeBusters. Kudos to Dmitriy Zhivilov and
 *      Ian Greenway for having reverse-engineered and shared the
 *      description of this format.
 *
 *      The original specs of this format were published on a
 *      russian electronics magazine.
 *
 *      The format is always 131114 bytes long and supports only
 *      Spectrum 128K images.
 *
 *      Offset  Size    Description (all registers stored with MSB first)
 *      ------- ------- -------------------------------------------------
 *      0       1       0x00 (reserved for future use)
 *      1       1       Port 0x7FFD data
 *      2       2       HL'
 *      4       2       HL
 *      6       2       DE'
 *      8       2       DE
 *      10      2       BC'
 *      12      2       BC
 *      14      2       AF'
 *      16      2       AF
 *      18      7       Emulation disk registers and T-states
 *      25      1       R
 *      26      2       PC
 *      28      2       SP
 *      30      1       I
 *      31      1       0xFF (reserved for future use)
 *      32      1       0x00 (reserved for future use)
 *      33      1       Interrupt mode (0=IM0/1=IM1/2=IM2)
 *      34      3       0x00 (reserved for future use)
 *      37      1       IFF1: bit 2 contains IFF1 (0=DI/1=EI)
 *      38      2       IY
 *      40      2       IX
 *      42      131072  RAM dump
 *
 *      The 8 16K banks are stored in the order 5, 2, 0, 1, 3, 4, 6, 7
 *
 *******************************************************************/
void spectrum_state::setup_frz(uint8_t *snapdata, uint32_t snapsize)
{
	int i, j;
	uint8_t intr;
	uint16_t addr, data;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (m_port_7ffd_data == -1)
	{
		logerror("Can't load 128K .FRZ file into 48K machine\n");
		return;
	}

	data = (snapdata[FRZ_OFFSET + 16] << 8) | snapdata[FRZ_OFFSET + 17];
	m_maincpu->set_state_int(Z80_AF, data);

	data = (snapdata[FRZ_OFFSET + 12] << 8) | snapdata[FRZ_OFFSET + 13];
	m_maincpu->set_state_int(Z80_BC, data);

	data = (snapdata[FRZ_OFFSET +  8] << 8) | snapdata[FRZ_OFFSET +  9];
	m_maincpu->set_state_int(Z80_DE, data);

	data = (snapdata[FRZ_OFFSET +  4] << 8) | snapdata[FRZ_OFFSET +  5];
	m_maincpu->set_state_int(Z80_HL, data);


	data = (snapdata[FRZ_OFFSET + 14] << 8) | snapdata[FRZ_OFFSET + 15];
	m_maincpu->set_state_int(Z80_AF2, data);

	data = (snapdata[FRZ_OFFSET + 10] << 8) | snapdata[FRZ_OFFSET + 11];
	m_maincpu->set_state_int(Z80_BC2, data);

	data = (snapdata[FRZ_OFFSET +  6] << 8) | snapdata[FRZ_OFFSET +  7];
	m_maincpu->set_state_int(Z80_DE2, data);

	data = (snapdata[FRZ_OFFSET +  2] << 8) | snapdata[FRZ_OFFSET +  3];
	m_maincpu->set_state_int(Z80_HL2, data);


	data = (snapdata[FRZ_OFFSET + 40] << 8) | snapdata[FRZ_OFFSET + 41];
	m_maincpu->set_state_int(Z80_IX, data);

	data = (snapdata[FRZ_OFFSET + 38] << 8) | snapdata[FRZ_OFFSET + 39];
	m_maincpu->set_state_int(Z80_IY, data);


	data = snapdata[FRZ_OFFSET + 25];
	m_maincpu->set_state_int(Z80_R, data);

	data = snapdata[FRZ_OFFSET + 30];
	m_maincpu->set_state_int(Z80_I, data);


	data = (snapdata[FRZ_OFFSET + 28] << 8) | snapdata[FRZ_OFFSET + 29];
	m_maincpu->set_state_int(Z80_SP, data);

	data = (snapdata[FRZ_OFFSET + 26] << 8) | snapdata[FRZ_OFFSET + 27];
	m_maincpu->set_state_int(Z80_PC, data);


	data = snapdata[FRZ_OFFSET + 33];
	m_maincpu->set_state_int(Z80_IM, data);

	data = snapdata[FRZ_OFFSET + 37];
	m_maincpu->set_state_int(Z80_IFF1, BIT(data, 2));
	m_maincpu->set_state_int(Z80_IFF2, BIT(data, 2));

	intr = BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, intr);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// Memory dump
	addr = 0;
	static const uint8_t banks[] = { 5, 2, 0, 1, 3, 4, 6, 7 };
	logerror("Loading %05X bytes of RAM at %04X\n", 8*SPECTRUM_BANK, BASE_RAM);
	for (i = 0; i < 8; i++)
	{
		switch (banks[i])
		{
			case 5: addr = SPECTRUM_BANK;
					break;
			case 2: addr = 2*SPECTRUM_BANK;
					break;
			case 0:
			case 1:
			case 3:
			case 4:
			case 6:
			case 7: addr = 3*SPECTRUM_BANK;
					m_port_7ffd_data &= 0xf8;
					m_port_7ffd_data += banks[i];
					update_paging();
					break;
		};
		logerror("Loading bank %d from offset:%05X\n", banks[i], FRZ_HDR + i*SPECTRUM_BANK);
		for (j = 0; j < SPECTRUM_BANK; j++)
			space.write_byte(j + addr, snapdata[j + FRZ_HDR + i*SPECTRUM_BANK]);
	}
	m_port_7ffd_data = snapdata[FRZ_OFFSET +  1];
	logerror("Port 7FFD:%02X\n", m_port_7ffd_data);
	logerror("Paging bank:%d\n", m_port_7ffd_data & 0x07);
	update_paging();

	// Set border color
	data = (space.read_byte(0x5c48) >> 3) & 0x07; // Get the current border color from BORDCR system variable.
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	//logerror("Snapshot loaded.\nExecution resuming at bank:%d %s\n", m_port_7ffd_data & 0x07, m_maincpu->state_string(Z80_PC).c_str());
}

void spectrum_state::z80_decompress_block(address_space &space, uint8_t *source, uint16_t dest, uint16_t size)
{
	uint8_t ch;
	int i;

	do
	{
		/* get byte */
		ch = source[0];

		/* either start 0f 0x0ed, 0x0ed, xx yy or single 0x0ed */
		if (ch == 0x0ed)
		{
			if (source[1] == 0x0ed)
			{
				/* 0x0ed, 0x0ed, xx yy - repetition */
				uint8_t count;
				uint8_t data;

				count = source[2];

				if (count == 0)
					return;

				data = source[3];

				source += 4;

				if (count > size)
					count = size;

				size -= count;

				for (i = 0; i < count; i++)
				{
					space.write_byte(dest, data);
					dest++;
				}
			}
			else
			{
				/* single 0x0ed */
				space.write_byte(dest, ch);
				dest++;
				source++;
				size--;
			}
		}
		else
		{
			/* not 0x0ed */
			space.write_byte(dest, ch);
			dest++;
			source++;
			size--;
		}
	}
	while (size > 0);
}

static SPECTRUM_Z80_SNAPSHOT_TYPE spectrum_identify_z80 (uint8_t *snapdata, uint32_t snapsize)
{
	uint8_t lo, hi, data;

	if (snapsize < 30)
		return SPECTRUM_Z80_SNAPSHOT_INVALID;   /* Invalid file */

	lo = snapdata[6] & 0x0ff;
	hi = snapdata[7] & 0x0ff;
	if (hi || lo)
		return SPECTRUM_Z80_SNAPSHOT_48K_OLD;   /* V1.45 - 48K only */

	lo = snapdata[30] & 0x0ff;
	hi = snapdata[31] & 0x0ff;
	data = snapdata[34] & 0x0ff;            /* Hardware mode */

	if ((hi == 0) && (lo == 23))
	{                       /* V2.01 */                         /* V2.01 format */
		switch (data)
		{
			case 0:
			case 1: return SPECTRUM_Z80_SNAPSHOT_48K;
			case 2: return SPECTRUM_Z80_SNAPSHOT_SAMRAM;
			case 3:
			case 4: return SPECTRUM_Z80_SNAPSHOT_128K;
			case 128: return SPECTRUM_Z80_SNAPSHOT_TS2068;
		}
	}

	if ((hi == 0) && (lo == 54))
	{                       /* V3.0x */                         /* V2.01 format */
		switch (data)
		{
			case 0:
			case 1:
			case 3: return SPECTRUM_Z80_SNAPSHOT_48K;
			case 2: return SPECTRUM_Z80_SNAPSHOT_SAMRAM;
			case 4:
			case 5:
			case 6: return SPECTRUM_Z80_SNAPSHOT_128K;
			case 128: return SPECTRUM_Z80_SNAPSHOT_TS2068;
		}
	}

	return SPECTRUM_Z80_SNAPSHOT_INVALID;
}

// supports 48k & 128k .Z80 files
void spectrum_state::setup_z80(uint8_t *snapdata, uint32_t snapsize)
{
	int i;
	uint8_t lo, hi, data;
	SPECTRUM_Z80_SNAPSHOT_TYPE z80_type;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	z80_type = spectrum_identify_z80(snapdata, snapsize);

	switch (z80_type)
	{
		case SPECTRUM_Z80_SNAPSHOT_INVALID:
				logerror("Invalid .Z80 file\n");
				return;
		case SPECTRUM_Z80_SNAPSHOT_48K_OLD:
		case SPECTRUM_Z80_SNAPSHOT_48K:
				logerror("48K .Z80 file\n");
				if (!strcmp(machine().system().name,"ts2068"))
					logerror("48K .Z80 file in TS2068\n");
				break;
		case SPECTRUM_Z80_SNAPSHOT_128K:
				logerror("128K .Z80 file\n");
				if (m_port_7ffd_data == -1)
				{
					logerror("Not a 48K .Z80 file\n");
					return;
				}
				if (!strcmp(machine().system().name,"ts2068"))
				{
					logerror("Not a TS2068 .Z80 file\n");
					return;
				}
				break;
		case SPECTRUM_Z80_SNAPSHOT_TS2068:
				logerror("TS2068 .Z80 file\n");
				if (strcmp(machine().system().name,"ts2068"))
					logerror("Not a TS2068 machine\n");
				break;
		case SPECTRUM_Z80_SNAPSHOT_SAMRAM:
				logerror("Hardware not supported - .Z80 file\n");
				return;
	}

	// AF
	hi = snapdata[0] & 0x0ff;
	lo = snapdata[1] & 0x0ff;
	m_maincpu->set_state_int(Z80_AF, (hi << 8) | lo);
	// BC
	lo = snapdata[2] & 0x0ff;
	hi = snapdata[3] & 0x0ff;
	m_maincpu->set_state_int(Z80_BC, (hi << 8) | lo);
	// HL
	lo = snapdata[4] & 0x0ff;
	hi = snapdata[5] & 0x0ff;
	m_maincpu->set_state_int(Z80_HL, (hi << 8) | lo);

	// SP
	lo = snapdata[8] & 0x0ff;
	hi = snapdata[9] & 0x0ff;
	m_maincpu->set_state_int(Z80_SP, (hi << 8) | lo);

	// I
	m_maincpu->set_state_int(Z80_I, (snapdata[10] & 0x0ff));

	// R
	data = (snapdata[11] & 0x07f) | ((snapdata[12] & 0x01) << 7);
	m_maincpu->set_state_int(Z80_R, data);

	// Set border color
	m_port_fe_data = (m_port_fe_data & 0xf8) | ((snapdata[12] & 0x0e) >> 1);
	border_update((snapdata[12] & 0x0e) >> 1);

	lo = snapdata[13] & 0x0ff;
	hi = snapdata[14] & 0x0ff;
	m_maincpu->set_state_int(Z80_DE, (hi << 8) | lo);

	lo = snapdata[15] & 0x0ff;
	hi = snapdata[16] & 0x0ff;
	m_maincpu->set_state_int(Z80_BC2, (hi << 8) | lo);

	lo = snapdata[17] & 0x0ff;
	hi = snapdata[18] & 0x0ff;
	m_maincpu->set_state_int(Z80_DE2, (hi << 8) | lo);

	lo = snapdata[19] & 0x0ff;
	hi = snapdata[20] & 0x0ff;
	m_maincpu->set_state_int(Z80_HL2, (hi << 8) | lo);

	hi = snapdata[21] & 0x0ff;
	lo = snapdata[22] & 0x0ff;
	m_maincpu->set_state_int(Z80_AF2, (hi << 8) | lo);

	lo = snapdata[23] & 0x0ff;
	hi = snapdata[24] & 0x0ff;
	m_maincpu->set_state_int(Z80_IY, (hi << 8) | lo);

	lo = snapdata[25] & 0x0ff;
	hi = snapdata[26] & 0x0ff;
	m_maincpu->set_state_int(Z80_IX, (hi << 8) | lo);

	// Interrupt Flip/Flop
	if (snapdata[27] == 0)
	{
		m_maincpu->set_state_int(Z80_IFF1, (uint64_t)0);
		// m_maincpu->set_state_int(Z80_IRQ_STATE, 0);
	}
	else
	{
		m_maincpu->set_state_int(Z80_IFF1, 1);
		// m_maincpu->set_state_int(Z80_IRQ_STATE, 1);
	}

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
//  m_maincpu->set_input_line(INPUT_LINE_NMI, data);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// IFF2
	if (snapdata[28] != 0)
	{
		data = 1;
	}
	else
	{
		data = 0;
	}
	m_maincpu->set_state_int(Z80_IFF2, data);

	// Interrupt Mode
	m_maincpu->set_state_int(Z80_IM, (snapdata[29] & 0x03));

	if (z80_type == SPECTRUM_Z80_SNAPSHOT_48K_OLD)
	{
		lo = snapdata[6] & 0x0ff;
		hi = snapdata[7] & 0x0ff;
		m_maincpu->set_state_int(Z80_PC, (hi << 8) | lo);

		page_basicrom();

		if ((snapdata[12] & 0x020) == 0)
		{
			logerror("Not compressed\n");   // not compressed
			for (i = 0; i < 49152; i++)
				space.write_byte(i + 16384, snapdata[30 + i]);
		}
		else
		{
			logerror("Compressed\n");   // compressed
			z80_decompress_block(space, snapdata + 30, 16384, 49152);
		}
	}
	else
	{
		uint8_t *pSource;
		int header_size;

		header_size = 30 + 2 + ((snapdata[30] & 0x0ff) | ((snapdata[31] & 0x0ff) << 8));

		lo = snapdata[32] & 0x0ff;
		hi = snapdata[33] & 0x0ff;
		m_maincpu->set_state_int(Z80_PC, (hi << 8) | lo);

		if ((z80_type == SPECTRUM_Z80_SNAPSHOT_128K) || ((z80_type == SPECTRUM_Z80_SNAPSHOT_TS2068) && !strcmp(machine().system().name,"ts2068")))
		{
			ay8910_device *ay8912 = subdevice<ay8910_device>("ay8912");

			// Only set up sound registers for 128K machine or TS2068!
			for (i = 0; i < 16; i++)
			{
				ay8912->address_w(i);
				ay8912->data_w(snapdata[39 + i]);
			}
			ay8912->address_w(snapdata[38]);
		}

		pSource = snapdata + header_size;

		if (z80_type == SPECTRUM_Z80_SNAPSHOT_48K)
			// Ensure 48K Basic ROM is used
			page_basicrom();

		do
		{
			unsigned short length;
			uint8_t page;
			int Dest = 0;

			length = (pSource[0] & 0x0ff) | ((pSource[1] & 0x0ff) << 8);
			page = pSource[2];

			if (z80_type == SPECTRUM_Z80_SNAPSHOT_48K || z80_type == SPECTRUM_Z80_SNAPSHOT_TS2068)
			{
				switch (page)
				{
					case 4: Dest = 0x08000; break;
					case 5: Dest = 0x0c000; break;
					case 8: Dest = 0x04000; break;
					default: Dest = 0; break;
				}
			}
			else
			{
				// 3 = bank 0, 4 = bank 1 ... 10 = bank 7
				if ((page >= 3) && (page <= 10))
				{
					// Page the appropriate bank into 0xc000 - 0xfff
					m_port_7ffd_data = page - 3;
					update_paging();
					Dest = 0x0c000;
				}
				else
					// Other values correspond to ROM pages
					Dest = 0x0;
			}

			if (Dest != 0)
			{
				if (length == 0x0ffff)
				{
					// block is uncompressed
					logerror("Not compressed\n");

					// not compressed
					for (i = 0; i < 16384; i++)
						space.write_byte(i + Dest, pSource[i]);
				}
				else
				{
					logerror("Compressed\n");

					// block is compressed
					z80_decompress_block(space, &pSource[3], Dest, 16384);
				}
			}

			// go to next block
			pSource += (3 + length);
		}
		while ((pSource - snapdata) < snapsize);

		if ((m_port_7ffd_data != -1) && (z80_type != SPECTRUM_Z80_SNAPSHOT_48K))
		{
			// Set up paging
			m_port_7ffd_data = (snapdata[35] & 0x0ff);
			update_paging();
		}
		if ((z80_type == SPECTRUM_Z80_SNAPSHOT_48K) && !strcmp(machine().system().name,"ts2068"))
		{
			m_port_f4_data = 0x03;
			m_port_ff_data = 0x00;
			ts2068_update_memory();
		}
		if (z80_type == SPECTRUM_Z80_SNAPSHOT_TS2068 && !strcmp(machine().system().name,"ts2068"))
		{
			m_port_f4_data = snapdata[35];
			m_port_ff_data = snapdata[36];
			ts2068_update_memory();
		}
	}
}

QUICKLOAD_LOAD_MEMBER(spectrum_state::quickload_cb)
{
	size_t quickload_size = image.length();
	std::vector<uint8_t> quickload_data(quickload_size);

	image.fread(&quickload_data[0], quickload_size);

	if (image.is_filetype("scr"))
	{
		if ((quickload_size != SCR_SIZE) && (quickload_size != SCR_BITMAP))
		{
			logerror("Invalid .SCR file size.\n");
			goto error;
		}
		setup_scr(&quickload_data[0], quickload_size);
	}
	else if (image.is_filetype("raw"))
	{
		if (quickload_size != RAW_SIZE)
		{
			logerror("Invalid .RAW file size.\n");
			goto error;
		}
		setup_raw(&quickload_data[0], quickload_size);
	}

	return image_init_result::PASS;

error:
	return image_init_result::FAIL;
}

/*******************************************************************
 *
 *      Load a .SCR file.
 *
 *      .SCR files are just a binary dump of the ZX Spectrum's
 *      display file, which is 256x192 pixels large.
 *
 *      These file are created using the Sinclair BASIC command
 *
 *      SAVE "filename" SCREEN$
 *
 *      where the keyword "SCREEN$" is a shortcut for "CODE 16384,6912"
 *      i.e. the loading address and the length of the data.
 *
 *      The format is organized as follows:
 *
 *      Offset   Size    Description
 *      -------- ------- -------------------------------------------------
 *      0        6144    Screen bitmap
 *      6144     768     Screen attributes
 *
 *      Some utilities and emulators support a B&W version of this
 *      format, which stores only the bitmap. These files can't be
 *      produced using the SAVE SCREEN$ command - rather they're
 *      created by the command
 *
 *      SAVE "filename" CODE 16384,6144
 *
 *******************************************************************/
void spectrum_state::setup_scr(uint8_t *quickdata, uint32_t quicksize)
{
	int i;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	for (i = 0; i < quicksize; i++)
		space.write_byte(i + BASE_RAM, quickdata[i]);

	log_quickload(quicksize == SCR_SIZE ? "SCREEN$" : "SCREEN$ (Mono)", BASE_RAM, quicksize, 0, EXEC_NA);
}

/*******************************************************************
 *
 *      Load a .RAW file.
 *
 *      .RAW files are a binary dump of the ZX Spectrum RAM
 *      and are created using the Sinclair BASIC command
 *
 *      SAVE *"b" CODE 16384,49152
 *
 *      which copies the whole RAM to the Interface 1's RS232 interface.
 *      The resulting file is always 49161 bytes long.
 *
 *      The format is organized as follows:
 *
 *      Offset  Size    Description (all registers stored with LSB first)
 *      ------- ------- -------------------------------------------------
 *      0       1       0x03 (BYTES file type)
 *      1       2       Image size
 *      3       2       Image start address
 *      5       4       0xFF
 *
 *      Since the size and the start address are encoded in the header,
 *      it would be possible to create .RAW images of any part of the RAM.
 *      However, no image of such type has ever surfaced.
 *
 *******************************************************************/
void spectrum_state::setup_raw(uint8_t *quickdata, uint32_t quicksize)
{
	int i;
	uint8_t data;
	uint16_t start, len;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	start = (quickdata[RAW_OFFSET + 4] << 8) | quickdata[RAW_OFFSET + 3];
	len   = (quickdata[RAW_OFFSET + 2] << 8) | quickdata[RAW_OFFSET + 1];

	for (i = 0; i < len; i++)
		space.write_byte(i + start, quickdata[i + RAW_HDR]);

	// Set border color
	data = (space.read_byte(0x5c48) >> 3) & 0x07; // Get the current border color from BORDCR system variable.
	m_port_fe_data = (m_port_fe_data & 0xf8) | data;
	border_update(data);
	logerror("Border color:%02X\n", data);

	log_quickload("BYTES", start, len, 0, EXEC_NA);
}
