/* Cave SH3 ( CAVE CV1000-B ) */
/* skeleton placeholder driver */


#include "emu.h"
#include "cpu/sh4/sh4.h"



class cavesh3_state : public driver_device
{
public:
	cavesh3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


VIDEO_START(cavesh3)
{
}

SCREEN_UPDATE(cavesh3)
{
	return 0;
}




static READ32_HANDLER( cavesh3_blitter_r )
{
//	UINT64 ret = space->machine().rand();
	static UINT32 i = 0;
	i^=0xffffffff;

	logerror("%08x cavesh3_blitter_r access at %08x (%08x) - mem_mask %08x\n",cpu_get_pc(&space->device()), offset, offset*4, mem_mask);

	switch (offset)
	{

		case 0x4:
			return i;

		case 0x9:
			return 0;

		default:
			logerror("no case for blit read\n");
			return 0;
	}


	return 0;


	
}

static WRITE32_HANDLER( cavesh3_blitter_w )
{
	logerror("%08x cavesh3_blitter_w access at %08x (%08x) -  %08x %08x\n",cpu_get_pc(&space->device()),offset, offset*8, data,mem_mask);
}

static READ64_HANDLER( ymz2770c_z_r )
{
	UINT64 ret = space->machine().rand();

	return ret ^ (ret<<32);
}

static WRITE64_HANDLER( ymz2770c_z_w )
{

}



// FLASH

#define FLASH_PAGE_SIZE	(2048+64)

UINT8 flash_page_data[FLASH_PAGE_SIZE];

typedef enum							{ STATE_IDLE = 0,	STATE_READ,		STATE_READ_ID,	STATE_READ_STATUS	} flash_state_t;
static const char *flash_state_name[] =	{ "IDLE",			"READ",			"READ_ID",		"READ_STATUS"		};

static flash_state_t flash_state;

static UINT8 flash_enab;

static UINT8 flash_cmd_seq;
static UINT32 flash_cmd_prev;

static UINT8 flash_addr_seq;
static UINT8 flash_read_seq;

static UINT16 flash_row, flash_col;
static UINT16 flash_page_addr;
static UINT16 flash_page_index;

static void flash_hard_reset(running_machine &machine)
{
//	logerror("%08x FLASH: RESET\n", cpuexec_describe_context(machine));

	flash_state = STATE_READ;

	flash_cmd_prev = -1;
	flash_cmd_seq = 0;

	flash_addr_seq = 0;
	flash_read_seq = 0;

	flash_row = 0;
	flash_col = 0;

	memset(flash_page_data, 0, FLASH_PAGE_SIZE);
	flash_page_addr = 0;
	flash_page_index = 0;
}

static WRITE8_HANDLER( flash_enab_w )
{
	logerror("%08x FLASH: enab = %02X\n", cpu_get_pc(&space->device()), data);
	//flash_enab = data;
	flash_enab = 1; // todo, why does it get turned off again instantly?
}

static void flash_change_state(running_machine &machine, flash_state_t state)
{
	flash_state = state;

	flash_cmd_prev = -1;
	flash_cmd_seq = 0;

	flash_read_seq = 0;
	flash_addr_seq = 0;

	logerror("flash_change_state - FLASH: state = %s\n", flash_state_name[state]);
}

static WRITE8_HANDLER( flash_cmd_w )
{
	if (!flash_enab)
		return;

	logerror("%08x FLASH: cmd = %02X (prev = %02X)\n", cpu_get_pc(&space->device()), data, flash_cmd_prev);

	if (flash_cmd_prev == -1)
	{
		flash_cmd_prev = data;

		switch (data)
		{
			case 0x00:	// READ
				flash_addr_seq = 0;
				break;

			case 0x70:	// READ STATUS
				flash_change_state( space->machine(), STATE_READ_STATUS );
				break;

			case 0x90:	// READ ID
				flash_change_state( space->machine(), STATE_READ_ID );
				break;

			case 0xff:	// RESET
				flash_change_state( space->machine(), STATE_IDLE );
				break;

			default:
			{
				logerror("%08x FLASH: unknown cmd1 = %02X\n", cpu_get_pc(&space->device()), data);
			}
		}
	}
	else
	{
		switch (flash_cmd_prev)
		{
			case 0x00:	// READ
				if (data == 0x30)
				{
					UINT8 *region = space->machine().region( "game" )->base();

					memcpy(flash_page_data, region + flash_row * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
					flash_page_addr = flash_col;
					flash_page_index = flash_row;

					flash_change_state( space->machine(), STATE_READ );

					logerror("%08x FLASH: caching page = %04X\n", cpu_get_pc(&space->device()), flash_row);
				}
				break;

			default:
			{
				logerror("%08x FLASH: unknown cmd2 = %02X (cmd1 = %02X)\n", cpu_get_pc(&space->device()), data, flash_cmd_prev);
			}
		}
	}
}

static WRITE8_HANDLER( flash_addr_w )
{
	if (!flash_enab)
		return;

	logerror("%08x FLASH: addr = %02X (seq = %02X)\n", cpu_get_pc(&space->device()), data, flash_addr_seq);

	switch( flash_addr_seq++ )
	{
		case 0:
			flash_col = (flash_col & 0xff00) | data;
			break;
		case 1:
			flash_col = (flash_col & 0x00ff) | (data << 8);
			break;
		case 2:
			flash_row = (flash_row & 0xff00) | data;
			break;
		case 3:
			flash_row = (flash_row & 0x00ff) | (data << 8);
			flash_addr_seq = 0;
			break;
	}
}

static READ8_HANDLER( flash_io_r )
{
	UINT8 data = 0x00;
	UINT32 old;

	if (!flash_enab)
		return 0xff;

	switch (flash_state)
	{
		case STATE_READ_ID:
			old = flash_read_seq;

			switch( flash_read_seq++ )
			{
				case 0:
					data = 0xEC;	// Manufacturer
					break;
				case 1:
					data = 0xF1;	// Device
					break;
				case 2:
					data = 0x00;	// XX
					break;
				case 3:
					data = 0x15;	// Flags
					flash_read_seq = 0;
					break;
			}

			logerror("%08x FLASH: read %02X from id(%02X)\n", cpu_get_pc(&space->device()), data, old);
			break;

		case STATE_READ:
			if (flash_page_addr > FLASH_PAGE_SIZE-1)
				flash_page_addr = FLASH_PAGE_SIZE-1;

			old = flash_page_addr;

			data = flash_page_data[flash_page_addr++];

//			logerror("%08x FLASH: read data %02X from addr %03X (page %04X)\n", cpu_get_pc(&space->device()), data, old, flash_page_index);
			break;

		case STATE_READ_STATUS:
			// bit 7 = writeable, bit 6 = ready, bit 5 = ready/true ready, bit 1 = fail(N-1), bit 0 = fail
			data = 0xe0;
			logerror("%08x FLASH: read status %02X\n", cpu_get_pc(&space->device()), data);
			break;

		default:
		{
			logerror("%08x FLASH: unknown read in state %s\n", cpu_get_pc(&space->device()), flash_state_name[flash_state]);
		}
	}

	return data;
}
/*
static READ8_HANDLER( flash_ready_r )
{
	return 1;
}

// FLASH interface

static READ32_HANDLER( ibara_flash_ready_r )
{
	// 400012a contains test bit (shown in service mode)
	return	((flash_ready_r(space, offset) ? 0x20 : 0x00) << 24) |
			input_port_read(space->machine(), "PORT_EF");
}
*/
static READ8_HANDLER( ibara_flash_io_r )
{
	switch (offset)
	{
		default:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:

		//	logerror("ibara_flash_io_r offset %04x\n", offset);
			return 0xff;

		case 0x00:
			return flash_io_r(space,offset);
	}
}

static WRITE8_HANDLER( ibara_flash_io_w )
{
	switch (offset)
	{
		default:
		case 0x00:
		case 0x03:
			logerror("unknown ibara_flash_io_w offset %04x data %02x\n", offset, data); // 03 enable/disable fgpa access?
			break;

		case 0x01:
			flash_cmd_w(space, offset, data);
			break;

		case 0x2:
			flash_addr_w(space, offset, data);
			break;
	}
}




static READ8_HANDLER( serial_rtc_eeprom_r )
{
	switch (offset)
	{
		default:
		logerror("unknown serial_rtc_eeprom_r access offset %02x\n", offset);
		return 0xff;
	}
}

static WRITE8_HANDLER( serial_rtc_eeprom_w )
{
	switch (offset)
	{
		case 0x01:
		// data & 0x00010000 = DATA
		// data & 0x00020000 = CLK
		// data & 0x00040000 = CE
		break;

		case 0x03:
			flash_enab_w(space,offset,data);
			return;

		default:
		logerror("unknown serial_rtc_eeprom_w access offset %02x data %02x\n",offset, data);
		break;
	}

}



static ADDRESS_MAP_START( cavesh3_map, AS_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x00200000, 0x003fffff) AM_ROM AM_REGION("maincpu", 0)

	/*       0x04000000, 0x07ffffff  SH3 Internal Regs (including ports) */
  
	AM_RANGE(0x0c000000, 0x0c7fffff) AM_RAM // work RAM
	AM_RANGE(0x0c800000, 0x0cffffff) AM_RAM // mirror of above on type B boards, extra ram on type D

	AM_RANGE(0x10000000, 0x10000007) AM_READWRITE8(ibara_flash_io_r, ibara_flash_io_w, U64(0xffffffffffffffff))
	AM_RANGE(0x10400000, 0x10400007) AM_READWRITE(ymz2770c_z_r, ymz2770c_z_w)
	AM_RANGE(0x10C00000, 0x10C00007) AM_READWRITE8(serial_rtc_eeprom_r, serial_rtc_eeprom_w, U64(0xffffffffffffffff))
	AM_RANGE(0x18000000, 0x18000057) AM_READWRITE32(cavesh3_blitter_r, cavesh3_blitter_w, U64(0xffffffffffffffff))

	AM_RANGE(0xf0000000, 0xf0ffffff) AM_RAM // mem mapped cache (sh3 internal?)
	/*       0xffffe000, 0xffffffff  SH3 Internal Regs 2 */
ADDRESS_MAP_END



static ADDRESS_MAP_START( cavesh3_port, AS_IO, 64 )
ADDRESS_MAP_END


static INPUT_PORTS_START( cavesh3 )
INPUT_PORTS_END


#define CAVE_CPU_CLOCK 12800000 * 8

static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, CAVE_CPU_CLOCK };




static INTERRUPT_GEN(cavesh3_interrupt)
{
	device_set_input_line(device, 2, HOLD_LINE);
}

static MACHINE_RESET( cavesh3 )
{
	flash_enab = 0;
	flash_hard_reset(machine);
}


static MACHINE_CONFIG_START( cavesh3, cavesh3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH3BE, CAVE_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(cavesh3_map)
	MCFG_CPU_IO_MAP(cavesh3_port)
	MCFG_CPU_VBLANK_INT("screen", cavesh3_interrupt)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE(cavesh3)
	MCFG_MACHINE_RESET(cavesh3)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(cavesh3)
MACHINE_CONFIG_END

/**************************************************

All roms are flash roms with no lables, so keep the
 version numbers attached to the roms that differ

**************************************************/

ROM_START( mushisam )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(0b5b30b2) SHA1(35fd1bb1561c30b311b4325bc8f4628f2fccd20b) ) /* (2004/10/12 MASTER VER.) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u2", 0x000000, 0x8400000, CRC(b1f826dc) SHA1(c287bd9f571d0df03d7fcbcf3c57c74ce564ab05) ) /* (2004/10/12 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( mushisama )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(9f1c7f51) SHA1(f82ae72ec03687904ca7516887080be92365a5f3) ) /* (2004/10/12 MASTER VER) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u2", 0x000000, 0x8400000, CRC(2cd13810) SHA1(40e45e201b60e63a060b68d4cc767eb64cfb99c2) ) /* (2004/10/12 MASTER VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( espgal2 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(09c908bb) SHA1(7d6031fd3542b3e1d296ff218feb40502fd78694) ) /* (2005/11/14 MASTER VER) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(222f58c7) SHA1(d47a5085a1debd9cb8c61d88cd39e4f5036d1797) ) /* (2005/11/14 MASTER VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(b9a10c22) SHA1(4561f95c6018c9716077224bfe9660e61fb84681) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c76b1ec4) SHA1(b98a53d41a995d968e0432ed824b0b06d93dcea8) )
ROM_END

ROM_START( mushitam )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(4a23e6c8) SHA1(d44c287bb88e6d413a8d35d75bc1b4928ad52cdf) ) /* (2005/09/09 MASTER VER) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u2", 0x000000, 0x8400000, CRC(3f93ff82) SHA1(6f6c250aa7134016ffb288d056bc937ea311f538) ) /* (2005/09/09 MASTER VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(701a912a) SHA1(85c198946fb693d99928ea2595c84ba4d9dc8157) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(6feeb9a1) SHA1(992711c80e660c32f97b343c2ce8184fddd7364e) )
ROM_END

ROM_START( futari15 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(e8c5f128) SHA1(45fb8066fdbecb83fdc2e14555c460d0c652cd5f) ) /* (2006/12/8.MAST VER. 1.54.) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u2", 0x000000, 0x8400000, CRC(b9eae1fc) SHA1(410f8e7cfcbfd271b41fb4f8d049a13a3191a1f9) ) /* (2006/12/8.MAST VER. 1.54.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futari15a )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(a609cf89) SHA1(56752fae9f42fa852af8ee2eae79e25ec7f17953) ) /* (2006/12/8 MAST VER 1.54) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u2", 0x000000, 0x8400000, CRC(b9d815f9) SHA1(6b6f668b0bbb087ffac65e4f0d8bd9d5b28eeb28) ) /* (2006/12/8 MAST VER 1.54) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futari10 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(b127dca7) SHA1(e1f518bc72fc1cdf69aefa89eafa4edaf4e84778) ) /* (2006/10/23 MASTER VER.) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(78ffcd0c) SHA1(0e2937edec15ce3f5741b72ebd3bbaaefffb556e) ) /* (2006/10/23 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futariblk )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(6db13c62) SHA1(6a53ce7f70b754936ccbb3a4674d4b2f03979644) ) /* (2007/12/11 BLACK LABEL VER) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(08c6fd62) SHA1(e1fc386b2b0e41906c724287cbf82304297e0150) ) /* (2007/12/11 BLACK LABEL VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( ibara )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(8e6c155d) SHA1(38ac2107dc7824836e2b4e04c7180d5ae43c9b79) ) /* (2005/03/22 MASTER VER..) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(55840976) SHA1(4982bdce84f9603adfed7a618f18bc80359ab81e) ) /* (2005/03/22 MASTER VER..) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(ee5e585d) SHA1(7eeba4ee693060e927f8c46b16e39227c6a62392) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(f0aa3cb6) SHA1(f9d137cd879e718811b2d21a0af2a9c6b7dca2f9) )
ROM_END

ROM_START( ibarablk )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(ee1f1f77) SHA1(ac276f3955aa4dde2544af4912819a7ae6bcf8dd) ) /* (2006/02/06. MASTER VER.) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(5e46be44) SHA1(bed5f1bf452f2cac58747ecabec3c4392566a3a7) ) /* (2006/02/06. MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a436bb22) SHA1(0556e771cc02638bf8814315ba671c2d442594f1) ) /* (2006/02/06 MASTER VER.) */
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(d11ab6b6) SHA1(2132191cbe847e2560423e4545c969f21f8ff825) ) /* (2006/02/06 MASTER VER.) */
ROM_END

ROM_START( ibarablka )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(a9d43839) SHA1(507696e616608c05893c7ac2814b3365e9cb0720) ) /* (2006/02/06 MASTER VER.) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(33400d96) SHA1(09c22b5431ac3726bf88c56efd970f56793f825a) ) /* (2006/02/06 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a436bb22) SHA1(0556e771cc02638bf8814315ba671c2d442594f1) ) /* (2006/02/06 MASTER VER.) */
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(d11ab6b6) SHA1(2132191cbe847e2560423e4545c969f21f8ff825) ) /* (2006/02/06 MASTER VER.) */
ROM_END

ROM_START( deathsml )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(1a7b98bf) SHA1(07798a4a846e5802756396b34df47d106895c1f1) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(d45b0698) SHA1(7077b9445f5ed4749c7f683191ccd312180fac38) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(aab718c8) SHA1(0e636c46d06151abd6f73232bc479dafcafe5327) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(83881d84) SHA1(6e2294b247dfcbf0ced155dc45c706f29052775d) )
ROM_END

ROM_START( mmpork )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(d06cfa42) SHA1(5707feb4b3e5265daf5926f38c38612b24106f1f) ) /* (2007/ 4/17 MASTER VER.) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(1ee961b8) SHA1(81a2eba704ac1cf7fc44fa7c6a3f50e3570c104f) ) /* (2007/ 4/17 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4a4b36df) SHA1(5db5ce6fa47e5ca3263d4bd19315890c6d29df66) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(ce83d07b) SHA1(a5947467c8f5b7c4b0ad8e32df2ee29b787e355f) )
ROM_END

ROM_START( mmmbnk )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x0000, 0x200000, CRC(5589d8c6) SHA1(43fbdb0effe2bc0e7135698757b6ee50200aecde) ) /* (2007/06/05 MASTER VER.) */

	ROM_REGION64_BE( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x0000, 0x8400000, CRC(f3b50c30) SHA1(962327798081b292b2d3fd3b7845c0197f9f2d8a) ) /* (2007/06/05 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4caaa1bf) SHA1(9b92c13eac05601da4d9bb3eb727c156974e9f0c) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(8e3a51ba) SHA1(e34cf9acb13c3d8ca6cd1306b060b1d429872abd) )
ROM_END




GAME( 2004, mushisam,  0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Mushihime Sama (2004/10/12 MASTER VER.)",                           GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2004, mushisama, mushisam,   cavesh3,    cavesh3,  0, ROT0, "Cave", "Mushihime Sama (2004/10/12 MASTER VER)",                            GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2005, espgal2,   0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "EspGaluda II (2005/11/14 MASTER VER)",                              GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2005, mushitam,  0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Mushihime Tama (2005/09/09 MASTER VER)",                            GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, futari15,  0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Mushihime Sama Futari Ver 1.5 (2006/12/8.MASTER VER. 1.54.)",       GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, futari15a, futari15,   cavesh3,    cavesh3,  0, ROT0, "Cave", "Mushihime Sama Futari Ver 1.5 (2006/12/8 MASTER VER 1.54)",         GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, futari10,  futari15,   cavesh3,    cavesh3,  0, ROT0, "Cave", "Mushihime Sama Futari Ver 1.0 (2006/10/23 MASTER VER.)",            GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2007, futariblk, futari15,   cavesh3,    cavesh3,  0, ROT0, "Cave", "Mushihime Sama Futari Black Label (2007/12/11 BLACK LABEL VER)",    GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, ibara,     0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Ibara (2005/03/22 MASTER VER..)",                                   GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, ibarablk,  0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Ibara Kuro - Black Label (2006/02/06. MASTER VER.)",                GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, ibarablka, ibarablk,   cavesh3,    cavesh3,  0, ROT0, "Cave", "Ibara Kuro - Black Label (2006/02/06 MASTER VER.)",                 GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2007, deathsml,  0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Death Smiles (2007/10/09 MASTER VER)",                              GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2007, mmpork,    0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Muchi Muchi Pork (2007/ 4/17 MASTER VER.)",                         GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2007, mmmbnk,    0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Medal Mahjong Moukari Bancho no Kiban (2007/06/05 MASTER VER.)",   GAME_NOT_WORKING | GAME_NO_SOUND )

/*

Known versions of games on this hardware (* denotes undumped):

MUSHIHIME SAMA
  "2004/10/12 MASTER VER"  - broken
  "2004/10/12 MASTER VER." - fixed 1
* "2004/10/12.MASTER VER." - fixed 2

MUSHIHIME TAMA
  "2005/09/09 MASTER VER"

ESPGALUDA II
  "2005/11/14 MASTER VER"

IBARA
  "2005/03/22 MASTER VER.."

IBARA BLACK LABEL
  "2006/02/06 MASTER VER."
  "2006/02/06. MASTER VER."

PINK SWEETS
* "2006/04/06 MASTER VER."
* "2006/04/06 MASTER VER..."
* "2006/04/06 MASTER VER...."
* "2006/05/18 MASTER VER."
* "2006/xx/xx MASTER VER"

MUSHIHIME SAMA FUTARI 1.0
* "2006/10/23 MASTER VER"  - Ultra unlockable
  "2006/10/23 MASTER VER." - Ultra unlockable
* "2006/10/23.MASTER VER." - Cannot unlock ultra

MUSHIHIME SAMA FUTARI 1.5
  "2006/12/8 MASTER VER 1.54"
  "2006/12/8.MASTER VER.1.54."

MUSHIHIME SAMA FUTARI BLACK LABEL
  "2007/12/11 BLACK LABEL VER"
* "2009/11/17 INTERNATIONAL BL"  ("Another Ver" on title screen)

MUCHI MUCHI PORK
  "2007/ 4/17 MASTER VER."
* 2 "period" ver, location of the periods unknown

MEDAL MAHJONG MOKUARI BANCHO NO KIBAN
  "2007/06/05 MASTER VER."

DEATH SMILES
  "2007/10/09 MASTER VER"

DEATH SMILES MEGA BLACK LABEL
* "2008/10/06 MEGABLACK LABEL VER"

DODONPACHI FUKKATSU 1.0
* "2008/05/16 MASTER VER"

DODONPACHI FUKKATSU 1.5
* "2008/06/23 MASTER VER 1.5"

DODONPACHI DAIFUKKATSU BLACK LABEL
* "2010/1/18 BLACK LABEL"

AKAI KATANA
* "2010/ 8/13 MASTER VER."
*  Home/Limited version, unknown date line, different gameplay from regular version, doesn't accept coins - permanent freeplay

MUSHIHIMESAMA 1.5 MATSURI VERSION
* 2011/5/23 CAVEMATSURI VER 1.5

*/
