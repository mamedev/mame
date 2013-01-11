/*

    dc.c - Sega Dreamcast hardware

    MESS (DC home console) hardware overrides (GD-ROM drive etc)

    c230048 - 5 is written, want 6
    c0d9d9e - where bad happens, from routine @ c0da260

    c0d9d8e - R0 on return is the value to put in

    cfffee0 - stack location when bad happens

*/

#include "emu.h"
#include "cdrom.h"
#include "debugger.h"
#include "includes/dc.h"
#include "cpu/sh4/sh4.h"
#include "sound/aica.h"
#include "includes/naomi.h"
#include "machine/gdrom.h"
#include "imagedev/chd_cd.h"

#define ATAPI_CYCLES_PER_SECTOR (5000)  // TBD for Dreamcast

#define ATAPI_STAT_BSY     0x80
#define ATAPI_STAT_DRDY    0x40
#define ATAPI_STAT_DMARDDF 0x20
#define ATAPI_STAT_SERVDSC 0x10
#define ATAPI_STAT_DRQ     0x08
#define ATAPI_STAT_CORR    0x04
#define ATAPI_STAT_CHECK   0x01

#define ATAPI_INTREASON_COMMAND 0x01
#define ATAPI_INTREASON_IO      0x02
#define ATAPI_INTREASON_RELEASE 0x04

#define ATAPI_REG_DATA      0
#define ATAPI_REG_FEATURES  1
#define ATAPI_REG_INTREASON 2
#define ATAPI_REG_SAMTAG    3
#define ATAPI_REG_COUNTLOW  4
#define ATAPI_REG_COUNTHIGH 5
#define ATAPI_REG_DRIVESEL  6
#define ATAPI_REG_CMDSTATUS 7
#define ATAPI_REG_ERROR     16  // read-only ERROR (write is FEATURES)

#define ATAPI_REG_MAX 24

#define ATAPI_XFER_PIO      0x00
#define ATAPI_XFER_PIO_FLOW 0x08
#define ATAPI_XFER_MULTI_DMA    0x20
#define ATAPI_XFER_ULTRA_DMA    0x40

#define ATAPI_DATA_SIZE ( 64 * 1024 )

static UINT8 *atapi_regs;
static emu_timer *atapi_timer;
static gdrom_device *gdrom;
static UINT8 *atapi_data;
static int atapi_data_ptr, atapi_data_len, atapi_xferlen, atapi_xferbase, atapi_cdata_wait, atapi_xfermod;
static UINT32 gdrom_alt_status;
static UINT8 xfer_mode = ATAPI_XFER_PIO;

#define MAX_TRANSFER_SIZE ( 63488 )

static void gdrom_raise_irq(running_machine &machine)
{
	dc_state *state = machine.driver_data<dc_state>();

	state->dc_sysctrl_regs[SB_ISTEXT] |= IST_EXT_GDROM;
	dc_update_interrupt_status(machine);
}

static TIMER_CALLBACK( atapi_xfer_end )
{
	dc_state *state = machine.driver_data<dc_state>();

	UINT8 sector_buffer[ 4096 ];

	atapi_timer->adjust(attotime::never);

	printf("atapi_xfer_end atapi_xferlen = %d, atapi_xfermod=%d\n", atapi_xfermod, atapi_xferlen );

	mame_printf_debug("ATAPI: xfer_end.  xferlen = %d, atapi_xfermod = %d\n", atapi_xferlen, atapi_xfermod);

	while (atapi_xferlen > 0 )
	{
		struct sh4_ddt_dma ddtdata;

		// get a sector from the SCSI device
		gdrom->ReadData( sector_buffer, 2048 );

		atapi_xferlen -= 2048;

		// perform the DMA
		ddtdata.destination = atapi_xferbase;   // destination address
		ddtdata.length = 2048/4;
		ddtdata.size = 4;
		ddtdata.buffer = sector_buffer;
		ddtdata.direction=1;    // 0 source to buffer, 1 buffer to destination
		ddtdata.channel= -1;    // not used
		ddtdata.mode= -1;       // copy from/to buffer
		printf("ATAPI: DMA one sector to %x, %x remaining\n", atapi_xferbase, atapi_xferlen);
		sh4_dma_ddt(machine.device("maincpu"), &ddtdata);

		atapi_xferbase += 2048;
	}

	if (atapi_xfermod > MAX_TRANSFER_SIZE)
	{
		atapi_xferlen = MAX_TRANSFER_SIZE;
		atapi_xfermod = atapi_xfermod - MAX_TRANSFER_SIZE;
	}
	else
	{
		atapi_xferlen = atapi_xfermod;
		atapi_xfermod = 0;
	}

	if (atapi_xferlen > 0)
	{
		printf("ATAPI: starting next piece of multi-part transfer\n");
		atapi_regs[ATAPI_REG_COUNTLOW] = atapi_xferlen & 0xff;
		atapi_regs[ATAPI_REG_COUNTHIGH] = (atapi_xferlen>>8)&0xff;

		atapi_timer->adjust(machine.device<cpu_device>("maincpu")->cycles_to_attotime((ATAPI_CYCLES_PER_SECTOR * (atapi_xferlen/2048))));
	}
	else
	{
		printf("ATAPI: Transfer completed, dropping DRQ\n");
		atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRDY;
		gdrom_alt_status = ATAPI_STAT_DRDY;
		atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO | ATAPI_INTREASON_COMMAND;

		state->g1bus_regs[SB_GDST]=0;
		state->dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_GDROM;
		dc_update_interrupt_status(machine);
	}

	gdrom_raise_irq(machine);

	printf( "atapi_xfer_end: %d %d\n", atapi_xferlen, atapi_xfermod );
}

static READ32_HANDLER( atapi_r )
{
	running_machine &machine = space.machine();
	int reg, data;

	if (mem_mask == 0x0000ffff) // word-wide command read
	{
//      mame_printf_debug("ATAPI: packet read = %04x\n", atapi_data[atapi_data_ptr]);

		// assert IRQ and drop DRQ
		if (atapi_data_ptr == 0 && atapi_data_len == 0)
		{
			// get the data from the device
			if( atapi_xferlen > 0 )
			{
				gdrom->ReadData( atapi_data, atapi_xferlen );
				atapi_data_len = atapi_xferlen;
			}

			if (atapi_xfermod > MAX_TRANSFER_SIZE)
			{
				atapi_xferlen = MAX_TRANSFER_SIZE;
				atapi_xfermod = atapi_xfermod - MAX_TRANSFER_SIZE;
			}
			else
			{
				atapi_xferlen = atapi_xfermod;
				atapi_xfermod = 0;
			}

//          printf( "atapi_r: atapi_xferlen=%d\n", atapi_xferlen );
			if( atapi_xferlen != 0 )
			{
				atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_SERVDSC;
				gdrom_alt_status = ATAPI_STAT_DRQ | ATAPI_STAT_SERVDSC;
				atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
			}
			else
			{
				//mame_printf_debug("ATAPI: dropping DRQ\n");
				atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
				gdrom_alt_status = 0;
				atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
			}

			atapi_regs[ATAPI_REG_COUNTLOW] = atapi_xferlen & 0xff;
			atapi_regs[ATAPI_REG_COUNTHIGH] = (atapi_xferlen>>8)&0xff;

			gdrom_raise_irq(machine);
		}

		if( atapi_data_ptr < atapi_data_len )
		{
			data = atapi_data[atapi_data_ptr++];
			data |= ( atapi_data[atapi_data_ptr++] << 8 );
			if( atapi_data_ptr >= atapi_data_len )
			{
//              printf( "atapi_r: read all bytes\n" );
				atapi_data_ptr = 0;
				atapi_data_len = 0;

				if( atapi_xferlen == 0 )
				{
					atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
					gdrom_alt_status = 0;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
					gdrom_raise_irq(machine);
				}
			}
		}
		else
		{
			data = 0;
		}
	}
	else
	{
		reg = offset;

		// get read-only side of read-only/write-only registers from elsewhere
		if (reg == ATAPI_REG_FEATURES)
		{
			reg = ATAPI_REG_ERROR;
		}
		data = atapi_regs[reg];

		#if 0
		switch( reg )
		{
		case ATAPI_REG_DATA:
			printf( "atapi_r: data=%02x\n", data );
			break;
		case ATAPI_REG_ERROR:
			printf( "atapi_r: error=%02x\n", data );
			break;
		case ATAPI_REG_INTREASON:
			printf( "atapi_r: intreason=%02x\n", data );
			break;
		case ATAPI_REG_SAMTAG:
			printf( "atapi_r: samtag=%02x\n", data );
			break;
		case ATAPI_REG_COUNTLOW:
			printf( "atapi_r: countlow=%02x\n", data );
			break;
		case ATAPI_REG_COUNTHIGH:
			printf( "atapi_r: counthigh=%02x\n", data );
			break;
		case ATAPI_REG_DRIVESEL:
			printf( "atapi_r: drivesel=%02x\n", data );
			break;
		case ATAPI_REG_CMDSTATUS:
			printf( "atapi_r: cmdstatus=%02x\n", data );
			break;
		}
		#endif

		mame_printf_debug("ATAPI: read reg %d = %x (PC=%x)\n", reg, data, space.device().safe_pc());
	}

//  printf( "atapi_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static WRITE32_HANDLER( atapi_w )
{
	running_machine &machine = space.machine();
	int reg;

//  printf( "atapi_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if (mem_mask == 0x0000ffff) // word-wide command write
	{
//      printf("atapi_w: data=%04x\n", data );

//      printf("ATAPI: packet write %04x\n", data);
		atapi_data[atapi_data_ptr++] = data & 0xff;
		atapi_data[atapi_data_ptr++] = data >> 8;

		if (atapi_cdata_wait)
		{
//                  printf("ATAPI: waiting, ptr %d wait %d\n", atapi_data_ptr, atapi_cdata_wait);
			if (atapi_data_ptr == atapi_cdata_wait)
			{
				// send it to the device
				gdrom->WriteData( atapi_data, atapi_cdata_wait );

				// assert IRQ
				gdrom_raise_irq(machine);

				// not sure here, but clear DRQ at least?
				atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
			}
		}

		else if ( atapi_data_ptr == 12 )
		{
			int phase;

//          printf("atapi_w: command %02x\n", atapi_data[0]&0xff );

			// reset data pointer for reading SCSI results
			atapi_data_ptr = 0;
			atapi_data_len = 0;

			// send it to the SCSI device
			gdrom->SetCommand( atapi_data, 12 );
			gdrom->ExecCommand( &atapi_xferlen );
			gdrom->GetPhase( &phase );

			if (atapi_xferlen != -1)
			{
				printf("ATAPI: SCSI command %02x returned %d bytes from the device\n", atapi_data[0]&0xff, atapi_xferlen);

				// store the returned command length in the ATAPI regs, splitting into
				// multiple transfers if necessary
				atapi_xfermod = 0;
				if (atapi_xferlen > MAX_TRANSFER_SIZE)
				{
					atapi_xfermod = atapi_xferlen - MAX_TRANSFER_SIZE;
					atapi_xferlen = MAX_TRANSFER_SIZE;
				}

				atapi_regs[ATAPI_REG_COUNTLOW] = atapi_xferlen & 0xff;
				atapi_regs[ATAPI_REG_COUNTHIGH] = (atapi_xferlen>>8)&0xff;

				gdrom_alt_status = 0;   // (I guess?)

				if (atapi_xferlen == 0)
				{
					// if no data to return, set the registers properly
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRDY;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO|ATAPI_INTREASON_COMMAND;
				}
				else
				{
					// indicate data ready: set DRQ and DMA ready, and IO in INTREASON
					if (atapi_regs[ATAPI_REG_FEATURES] & 0x01)  // DMA feature
					{
						atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_BSY | ATAPI_STAT_DRDY | ATAPI_STAT_SERVDSC;
					}
					else
					{
						atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_SERVDSC | ATAPI_STAT_DRQ;
					}
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
				}

				switch( phase )
				{
				case SCSI_PHASE_DATAOUT:
					atapi_cdata_wait = atapi_xferlen;
					break;
				}

				// perform special ATAPI processing of certain commands
				switch (atapi_data[0]&0xff)
				{
					case 0x00: // BUS RESET / TEST UNIT READY
					case 0xbb: // SET CDROM SPEED
						atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
						break;

					case 0x45: // PLAY
						atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_BSY;
						atapi_timer->adjust( downcast<cpu_device *>(&space.device())->cycles_to_attotime(ATAPI_CYCLES_PER_SECTOR ) );
						break;
				}

				// assert IRQ
				gdrom_raise_irq(machine);
			}
			else
			{
						printf("ATAPI: SCSI device returned error!\n");

				atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_CHECK;
				atapi_regs[ATAPI_REG_ERROR] = 0x50; // sense key = ILLEGAL REQUEST
				atapi_regs[ATAPI_REG_COUNTLOW] = 0;
				atapi_regs[ATAPI_REG_COUNTHIGH] = 0;
			}
		}
	}
	else
	{
		reg = offset;
#if 0
		switch( reg )
		{
		case ATAPI_REG_DATA:
			printf( "atapi_w: data=%02x\n", data );
				break;
			case ATAPI_REG_FEATURES:
			printf( "atapi_w: features=%02x\n", data );
				break;
			case ATAPI_REG_INTREASON:
			printf( "atapi_w: intreason=%02x\n", data );
				break;
			case ATAPI_REG_SAMTAG:
			printf( "atapi_w: samtag=%02x\n", data );
				break;
			case ATAPI_REG_COUNTLOW:
			printf( "atapi_w: countlow=%02x\n", data );
				break;
			case ATAPI_REG_COUNTHIGH:
			printf( "atapi_w: counthigh=%02x\n", data );
				break;
			case ATAPI_REG_DRIVESEL:
			printf( "atapi_w: drivesel=%02x\n", data );
				break;
			case ATAPI_REG_CMDSTATUS:
			printf( "atapi_w: cmdstatus=%02x\n", data );
			break;
		}
#endif
		atapi_regs[reg] = data;
//      mame_printf_debug("ATAPI: reg %d = %x (offset %x mask %x PC=%x)\n", reg, data, offset, mem_mask, space.device().safe_pc());

		if (reg == ATAPI_REG_CMDSTATUS)
		{
			printf("ATAPI command %x issued! (PC=%x)\n", data, space.device().safe_pc());

			switch (data)
			{
				case 0xa0:  // PACKET
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ;
					gdrom_alt_status = ATAPI_STAT_DRQ;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_COMMAND;

					atapi_data_ptr = 0;
					atapi_data_len = 0;

					/* we have no data */
					atapi_xferlen = 0;
					atapi_xfermod = 0;

					atapi_cdata_wait = 0;
					break;

				case 0xa1:  // IDENTIFY PACKET DEVICE
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ;
					gdrom_alt_status = ATAPI_STAT_DRQ;

					atapi_data_ptr = 0;
					atapi_data_len = 512;

					/* we have no data */
					atapi_xferlen = 0;
					atapi_xfermod = 0;

					memset( atapi_data, 0, atapi_data_len );

					atapi_data[ 0 ^ 1 ] = 0x86; // ATAPI device, cmd set 6 compliant, DRQ within 3 ms of PACKET command
					atapi_data[ 1 ^ 1 ] = 0x00;

					memset( &atapi_data[ 46 ], ' ', 8 );
					atapi_data[ 46 ^ 1 ] = 'S';
					atapi_data[ 47 ^ 1 ] = 'E';

					memset( &atapi_data[ 54 ], ' ', 40 );
					atapi_data[ 54 ^ 1 ] = 'C';
					atapi_data[ 55 ^ 1 ] = 'D';
					atapi_data[ 56 ^ 1 ] = '-';
					atapi_data[ 57 ^ 1 ] = 'R';
					atapi_data[ 58 ^ 1 ] = 'O';
					atapi_data[ 59 ^ 1 ] = 'M';
					atapi_data[ 60 ^ 1 ] = ' ';
					atapi_data[ 61 ^ 1 ] = 'D';
					atapi_data[ 62 ^ 1 ] = 'R';
					atapi_data[ 63 ^ 1 ] = 'I';
					atapi_data[ 64 ^ 1 ] = 'V';
					atapi_data[ 65 ^ 1 ] = 'E';
					atapi_data[ 66 ^ 1 ] = ' ';
					atapi_data[ 67 ^ 1 ] = ' ';
					atapi_data[ 68 ^ 1 ] = ' ';
					atapi_data[ 69 ^ 1 ] = ' ';
					atapi_data[ 70 ^ 1 ] = '6';
					atapi_data[ 71 ^ 1 ] = '.';
					atapi_data[ 72 ^ 1 ] = '4';
					atapi_data[ 73 ^ 1 ] = '2';

					atapi_data[ 98 ^ 1 ] = 0x04; // IORDY may be disabled
					atapi_data[ 99 ^ 1 ] = 0x00;

					atapi_regs[ATAPI_REG_COUNTLOW] = 0;
					atapi_regs[ATAPI_REG_COUNTHIGH] = 2;

					gdrom_raise_irq(space.machine());
					break;

				case 0xef:  // SET FEATURES
					// set xfer mode?
					if (atapi_regs[ATAPI_REG_FEATURES] == 0x03)
					{
						printf("Set transfer mode to %x\n", atapi_regs[ATAPI_REG_COUNTLOW] & 0xf8);
						xfer_mode = atapi_regs[ATAPI_REG_COUNTLOW] & 0xf8;
					}
					else
					{
						printf("ATAPI: Unknown set features %x\n", atapi_regs[ATAPI_REG_FEATURES]);
					}

					atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
					gdrom_alt_status = 0;   // is this correct?

					atapi_data_ptr = 0;
					atapi_data_len = 0;

					gdrom_raise_irq(space.machine());
					break;

				default:
					mame_printf_debug("ATAPI: Unknown IDE command %x\n", data);
					break;
			}
		}
	}
}

void dreamcast_atapi_init(running_machine &machine)
{
	atapi_regs = auto_alloc_array_clear(machine, UINT8, ATAPI_REG_MAX);

	atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
	atapi_regs[ATAPI_REG_ERROR] = 1;
	atapi_regs[ATAPI_REG_COUNTLOW] = 0x14;
	atapi_regs[ATAPI_REG_COUNTHIGH] = 0xeb;

	atapi_data_ptr = 0;
	atapi_data_len = 0;
	atapi_cdata_wait = 0;

	atapi_timer = machine.scheduler().timer_alloc(FUNC(atapi_xfer_end));
	atapi_timer->adjust(attotime::never);

	gdrom = NULL;

	atapi_data = auto_alloc_array(machine, UINT8,  ATAPI_DATA_SIZE );

	state_save_register_global_pointer(machine,  atapi_regs, ATAPI_REG_MAX );
	state_save_register_global_pointer(machine,  atapi_data, ATAPI_DATA_SIZE / 2 );
	state_save_register_global(machine,  atapi_data_ptr );
	state_save_register_global(machine,  atapi_data_len );
	state_save_register_global(machine,  atapi_xferlen );
	state_save_register_global(machine,  atapi_xferbase );
	state_save_register_global(machine,  atapi_cdata_wait );
	state_save_register_global(machine,  atapi_xfermod );

	gdrom = machine.device<gdrom_device>( "cdrom" );
}

void dreamcast_atapi_reset(running_machine &machine)
{
	atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
	atapi_regs[ATAPI_REG_ERROR] = 1;
	atapi_regs[ATAPI_REG_COUNTLOW] = 0x14;
	atapi_regs[ATAPI_REG_COUNTHIGH] = 0xeb;

	atapi_data_ptr = 0;
	atapi_data_len = 0;
	atapi_cdata_wait = 0;

	atapi_xferlen = 0;
	atapi_xfermod = 0;
}

/*

 GDROM regsters:

 5f7018: alternate status/device control
 5f7080: data
 5f7084: error/features
 5f7088: interrupt reason/sector count
 5f708c: sector number
 5f7090: byte control low
 5f7094: byte control high
 5f7098: drive select
 5f709c: status/command

c002910 - ATAPI packet writes
c002796 - aux status read after that
c000776 - DMA triggered to c008000

*/

READ64_HANDLER( dc_mess_gdrom_r )
{
	UINT32 off;

	if ((int)~mem_mask & 1)
	{
		off=(offset << 1) | 1;
	}
	else
	{
		off=offset << 1;
	}

//  printf("gdrom_r: @ %x (off %x), mask %llx (PC %x)\n", offset, off, mem_mask, space.device().safe_pc());

	if (offset == 3)
	{
		return gdrom_alt_status;
	}
	else if (off >= 0x20)
	{
		return atapi_r(space, off-0x20, 0xff);
	}

	return 0;
}

WRITE64_HANDLER( dc_mess_gdrom_w )
{
	UINT32 dat,off;

	if ((int)~mem_mask & 1)
	{
		dat=(UINT32)(data >> 32);
		off=(offset << 1) | 1;
	}
	else
	{
		dat=(UINT32)data;
		off=offset << 1;
	}

//  printf("GDROM: [%08x=%x]write %llx to %x, mask %llx (PC %x)\n", 0x5f7000+off*4, dat, data, offset, mem_mask, space.device().safe_pc());

	if (off >= 0x20)
	{
		atapi_w(space, off-0x20, dat, (UINT32)mem_mask);
	}
}

// register decode helpers

// this accepts only 32-bit accesses
INLINE int decode_reg32_64(running_machine &machine, UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		mame_printf_verbose("%s:Wrong mask!\n", machine.describe_context());
//      debugger_break(machine);
	}

	if (mem_mask == U64(0xffffffff00000000))
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

READ64_HANDLER( dc_mess_g1_ctrl_r )
{
	dc_state *state = space.machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space.machine(), offset, mem_mask, &shift);
	mame_printf_verbose("G1CTRL:  Unmapped read %08x\n", 0x5f7400+reg*4);
	return (UINT64)state->g1bus_regs[reg] << shift;
}

WRITE64_HANDLER( dc_mess_g1_ctrl_w )
{
	dc_state *state = space.machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;
	UINT32 dat; //, old

	reg = decode_reg32_64(space.machine(), offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
//  old = state->g1bus_regs[reg];

	state->g1bus_regs[reg] = dat; // 5f7400+reg*4=dat
	mame_printf_verbose("G1CTRL: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x5f7400+reg*4, dat, data, offset, mem_mask);
	switch (reg)
	{
	case SB_GDST:
		if (dat & 1 && state->g1bus_regs[SB_GDEN] == 1) // 0 -> 1
		{
			if (state->g1bus_regs[SB_GDDIR] == 0)
			{
				printf("G1CTRL: unsupported transfer\n");
				return;
			}

			atapi_xferbase = state->g1bus_regs[SB_GDSTAR];
			atapi_timer->adjust(space.machine().device<cpu_device>("maincpu")->cycles_to_attotime((ATAPI_CYCLES_PER_SECTOR * (atapi_xferlen/2048))));
		}
		break;
	}
}
