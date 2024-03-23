// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith, Carl
/*
    machine/rmnimbus.c

    Machine driver for the Research Machines Nimbus.

    Phill Harvey-Smith
    2009-11-29.

*/

/*

    SCSI/SASI drives supported by RM Nimbus machines

Native SCSI - format with HDFORM.EXE

Drive           Capacity    Tracks  Heads   Sec/Track       Blocks
RO652-20        20MB        306     4       34              41616
ST225N          20MB        615     4       17              41721
ST125N          20MB        407     4       26              41921
8425S-30        20MB                                        41004
CP3020          20MB        623     2       33              41118
ST225NP         20MB        615     4       17              41720
CP3040          40MB        1026    2       40              82080

Via Xebec S1410 SASI to MFM bridge board - format with WINFORM.EXE
NP05-10S         8MB        160     6       17              16320
NP04-20T        16MB        320     6       17              32640
NP03-20         15MB        306     6       17              31212
R352-10         10MB        306     4       17              20808
NP04-50         40MB        699     7       17              83181
NP04-55         44MB        754     7       17              89726

Via Adaptec ACB4070 SCSI to RLL bridge board - format with ADAPT.EXE
NEC D5147       60MB        615     8       26              127920
ST227R          60MB        820     6       26              127920

After formating, the drives need to have a partition table put on them with
STAMP.EXE and then formatted in the normal way for a dos system drive with
Format /s.

The tracks, heads and sectors/track can be used with chdman createhd
to create a blank hard disk which can then be formatted with the RM tools.
The important thing when doing this is to make sure that if using the Native
SCSI tools, that the disk has the  same number of blocks as specified above,
even if you have to use unusual geometry to do so !
Currently, only the ST225N and ST125N can be formatted as the other native
drives and Xebec board expect the WRITE BUFFER (0x3B) and READ BUFFER (0x3C)
with mode 0 commands to be implemented and the Adaptec board uses unknown
command 0xE4.


for example:

chdman createhd -o ST125N.chd -chs 41921,1,1 -ss 512
(the actual geometry can't be used because the block count won't match)

*/

#include "emu.h"
#include <functional>

#include "rmnimbus.h"
#include "imagedev/floppy.h"

#define LOG_SIO             (1U << 1)
#define LOG_DISK_HDD        (1U << 2)
#define LOG_DISK            (1U << 3)
#define LOG_PC8031          (1U << 4)
#define LOG_PC8031_186      (1U << 5)
#define LOG_PC8031_PORT     (1U << 6)
#define LOG_IOU             (1U << 7)
#define LOG_RAM             (1U << 8)

#define VERBOSE (0)
#include "logmacro.h"



/*-------------------------------------------------------------------------*/
/* Defines, constants, and global variables                                */
/*-------------------------------------------------------------------------*/

/* External int vectors for chained interrupts */
#define EXTERNAL_INT_DISK       0x80
#define EXTERNAL_INT_MSM5205    0x84
#define EXTERNAL_INT_MOUSE_YU   0x88
#define EXTERNAL_INT_MOUSE_YD   0x89
#define EXTERNAL_INT_MOUSE_XL   0x8A
#define EXTERNAL_INT_MOUSE_XR   0x8B
#define EXTERNAL_INT_PC8031_8C  0x8c
#define EXTERNAL_INT_PC8031_8E  0x8E
#define EXTERNAL_INT_PC8031_8F  0x8F

#define HDC_DRQ_MASK        0x40
#define FDC_SIDE()          ((m_nimbus_drives.reg400 & 0x10) >> 4)
#define FDC_MOTOR()         ((m_nimbus_drives.reg400 & 0x20) >> 5)
#define FDC_DRIVE()         (fdc_driveno(m_nimbus_drives.reg400 & 0x0f))
#define HDC_DRQ_ENABLED()   ((m_nimbus_drives.reg400 & 0x40) ? 1 : 0)
#define FDC_DRQ_ENABLED()   ((m_nimbus_drives.reg400 & 0x80) ? 1 : 0)

/* 8031/8051 Peripheral controller */

#define IPC_OUT_ADDR        0x01
#define IPC_OUT_READ_PEND   0x02
#define IPC_OUT_BYTE_AVAIL  0x04

#define IPC_IN_ADDR         0x01
#define IPC_IN_BYTE_AVAIL   0x02
#define IPC_IN_READ_PEND    0x04

/* IO unit */

#define DISK_INT_ENABLE         0x01
#define MSM5205_INT_ENABLE      0x04
#define MOUSE_INT_ENABLE        0x08
#define PC8031_INT_ENABLE       0x10

#define CONTROLLER_NONE         0x00
#define CONTROLLER_LEFT         0x01
#define CONTROLLER_RIGHT        0x02
#define CONTROLLER_DOWN         0x04
#define CONTROLLER_UP           0x08
#define CONTROLLER_BUTTON0      0x10
#define CONTROLLER_BUTTON1      0x20

// Frequency in Hz to poll for mouse movement.
#define MOUSE_POLL_FREQUENCY    500

#define MOUSE_INT_ENABLED(state)     (((state)->m_iou_reg092 & MOUSE_INT_ENABLE) ? 1 : 0)

#define LINEAR_ADDR(seg,ofs)    ((seg<<4)+ofs)

#define OUTPUT_SEGOFS(mess,seg,ofs)  logerror("%s=%04X:%04X [%08X]\n",mess,seg,ofs,((seg<<4)+ofs))


void rmnimbus_state::external_int(uint8_t vector, bool state)
{
	if(!state && (vector != m_vector))
		return;

	m_vector = vector;

	m_maincpu->int0_w(state);
}

uint8_t rmnimbus_state::cascade_callback()
{
	m_maincpu->int0_w(0);
	return !m_vector ? m_z80sio->m1_r() : m_vector;
}

void rmnimbus_state::machine_reset()
{
	/* CPU */
	iou_reset();
	fdc_reset();
	hdc_reset();
	pc8031_reset();
	rmni_sound_reset();
	memory_reset();
	mouse_js_reset();

	/* USER VIA 6522 port B is connected to the BBC user port */
	m_via->write_pb(0xff);
}

void rmnimbus_state::machine_start()
{
	m_nimbus_mouse.m_mouse_timer = timer_alloc(FUNC(rmnimbus_state::do_mouse), this);

	m_voice_enabled=false;
	m_fdc->dden_w(0);
	//m_fdc->overide_delays(64,m_fdc->get_cmd_delay());
}

#define CBUFLEN 32

offs_t rmnimbus_state::dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	unsigned call;
	char    callname[CBUFLEN];
	offs_t result = 0;

	// decode and document (some) INT XX calls
	if (opcodes.r8(pc) == 0xCD)
	{
		call = opcodes.r8(pc+1);
		switch (call)
		{
			case 0x20 :
				strcpy(callname, "(dos terminate)");
				break;

			case 0x21 :
				strcpy(callname, "(dos function)");
				break;

			case 0xf0 :
				strcpy(callname, "(sub_bios)");
				break;

			case 0xf3 :
				strcpy(callname, "(dispatch handler)");
				break;

			case 0xf5 :
				strcpy(callname, "(event handler)");
				break;

			case 0xf6 :
				strcpy(callname, "(resource message)");
				break;

			default :
				strcpy(callname, "");
		}
		util::stream_format(stream, "int   %02xh %s",call,callname);
		result = 2;
	}
	return result;
}

/*
    The Nimbus has 3 banks of memory each of which can be either 16x4164 or 16x41256 giving
    128K or 512K per bank. These banks are as follows :

    bank0   on nimbus motherboard.
    bank1   first half of expansion card.
    bank2   second half of expansion card.

    The valid combinations are :

    bank0       bank1       bank2       total
    128K                                128K
    128K        128K                    256K
    128K        128K        128K        384K
    128K        512K                    640K (1)
    512K        128K                    640K (2)
    512K        512K                    1024K
    512K        512K        512K        1536K

    It will be noted that there are two possible ways of getting 640K, we emulate method 2
    (above).

    To allow for the greatest flexibility, the Nimbus allows 4 methods of mapping the
    banks of ram into the 1M addressable by the 81086.

    With only 128K banks present, they are mapped into the first 3 blocks of 128K in
    the memory map giving a total of up to 384K.

    If any of the blocks are 512K, then the block size is set to 512K and the map arranged
    so that the bottom block is a 512K block (if both 512K and 128K blocks are available).

    This is all determined by the value written to port 80 :-

    port80 = 0x07   start       end
        block0      0x00000     0x1FFFF
        block1      0x20000     0x3FFFF
        block2      0x40000     0x5FFFF

    port80 = 0x1F
        block0      0x00000     0x7FFFF
        block1      0x80000     0xEFFFF (0x9FFFF if 128K (2))

    port80 = 0x0F
        block1      0x00000     0x7FFFF
        block0      0x80000     0xEFFFF (0x9FFFF if 128K (1))

    port80 = 0x17
        block1      0x00000     0x7FFFF
        block2      0x80000     0xEFFFF

*/

struct nimbus_meminfo
{
	offs_t  start;      /* start address of bank */
	offs_t  end;        /* End address of bank */
};

static const struct nimbus_meminfo memmap[] =
{
	{ 0x00000, 0x1FFFF },
	{ 0x20000, 0x3FFFF },
	{ 0x40000, 0x5FFFF },
	{ 0x60000, 0x7FFFF },
	{ 0x80000, 0x9FFFF },
	{ 0xA0000, 0xBFFFF },
	{ 0xC0000, 0xDFFFF },
	{ 0xE0000, 0xEFFFF }
};

struct nimbus_block
{
	int     blockbase;
	int     blocksize;
};

typedef nimbus_block nimbus_blocks[3];

static const nimbus_blocks ramblocks[] =
{
	{{ 0, 128 },    { 000, 000 },   { 000, 000 }} ,
	{{ 0, 128 },    { 128, 128 },   { 000, 000 }} ,
	{{ 0, 128 },    { 128, 128 },   { 256, 128 }} ,
	{{ 0, 512 },    { 000, 000 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 128 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 512 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 512 },   { 1024, 512 } }
};

void rmnimbus_state::nimbus_bank_memory()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int     ramsize = m_ram->size();
	int     ramblock = 0;
	int     blockno;
	char    bank[10];
	uint8_t   *ram    = &m_ram->pointer()[0];
	uint8_t   *map_blocks[3];
	uint8_t   *map_base;
	int     map_blockno;
	int     block_ofs;

	uint8_t   ramsel = (m_mcu_reg080 & 0x1F);

	// Invalid ramsel, return.
	if((ramsel & 0x07)!=0x07)
		return;

	switch (ramsize / 1024)
	{
		case 128    : ramblock=0; break;
		case 256    : ramblock=1; break;
		case 384    : ramblock=2; break;
		case 512    : ramblock=3; break;
		case 640    : ramblock=4; break;
		case 1024   : ramblock=5; break;
		case 1536   : ramblock=6; break;
	}

	map_blocks[0]  = ram;
	map_blocks[1]  = (ramblocks[ramblock][1].blocksize==0) ? nullptr : &ram[ramblocks[ramblock][1].blockbase*1024];
	map_blocks[2]  = (ramblocks[ramblock][2].blocksize==0) ? nullptr : &ram[ramblocks[ramblock][2].blockbase*1024];

	//LOGMASKED(LOG_RAM, "\n\nmcu_reg080=%02X, ramblock=%d, map_blocks[0]=%X, map_blocks[1]=%X, map_blocks[2]=%X\n",m_mcu_reg080,ramblock,(int)map_blocks[0],(int)map_blocks[1],(int)map_blocks[2]);

	for(blockno=0;blockno<8;blockno++)
	{
		sprintf(bank,"bank%d",blockno);

		switch (ramsel)
		{
			case 0x07   : (blockno<3) ? map_blockno=blockno : map_blockno=-1; break;
			case 0x1F   : (blockno<4) ? map_blockno=0 : map_blockno=1; break;
			case 0x0F   : (blockno<4) ? map_blockno=1 : map_blockno=0; break;
			case 0x17   : (blockno<4) ? map_blockno=1 : map_blockno=2; break;
			default     : map_blockno=-1;
		}
		block_ofs=(ramsel==0x07) ? 0 : ((blockno % 4)*128);


		LOGMASKED(LOG_RAM, "mapped %s",bank);

		if((map_blockno>-1) && (block_ofs < ramblocks[ramblock][map_blockno].blocksize) &&
			(map_blocks[map_blockno]!=nullptr))
		{
			map_base=(ramsel==0x07) ? map_blocks[map_blockno] : &map_blocks[map_blockno][block_ofs*1024];

			membank(bank)->set_base(map_base);
			space.install_readwrite_bank(memmap[blockno].start, memmap[blockno].end, membank(bank));
			//LOGMASKED(LOG_RAM, ", base=%X\n",(int)map_base);
		}
		else
		{
			space.nop_readwrite(memmap[blockno].start, memmap[blockno].end);
			LOGMASKED(LOG_RAM, "NOP\n");
		}
	}
}

uint8_t rmnimbus_state::nimbus_mcu_r()
{
	return m_mcu_reg080;
}

void rmnimbus_state::nimbus_mcu_w(uint8_t data)
{
	m_mcu_reg080=data;

	nimbus_bank_memory();
}

void rmnimbus_state::memory_reset()
{
	m_mcu_reg080=0x07;
	nimbus_bank_memory();
}

/*

Z80SIO, used for the keyboard interface

*/

/* Z80 SIO/2 */

void rmnimbus_state::sio_interrupt(int state)
{
	LOGMASKED(LOG_SIO, "SIO Interrupt state=%02X\n",state);

	external_int(0, state);
}

/* Floppy disk */

void rmnimbus_state::fdc_reset()
{
	m_nimbus_drives.reg400=0;
	m_scsi_ctrl_out->write(0);
}

void rmnimbus_state::nimbus_fdc_intrq_w(int state)
{
	LOGMASKED(LOG_DISK, "nimbus_drives_intrq = %d\n",state);

	if(m_iou_reg092 & DISK_INT_ENABLE)
	{
		external_int(EXTERNAL_INT_DISK,state);
	}
}

void rmnimbus_state::nimbus_fdc_drq_w(int state)
{
	LOGMASKED(LOG_DISK, "nimbus_drives_drq_w(%d)\n", state);

	m_maincpu->drq1_w(state && FDC_DRQ_ENABLED());
}

int rmnimbus_state::nimbus_fdc_enmf_r()
{
	return false;
}

uint8_t rmnimbus_state::fdc_driveno(uint8_t drivesel)
{
	switch (drivesel)
	{
		case 0x01: return 0;
		case 0x02: return 1;
		case 0x04: return 2;
		case 0x08: return 3;
		case 0x10: return 4;
		case 0x20: return 5;
		case 0x40: return 6;
		case 0x80: return 7;
		default: return 0;
	}
}

/*
    0x410 read bits

    0   Ready from floppy
    1   Index pulse from floppy
    2   Motor on from floppy
    3   MSG from HDD
    4   !BSY from HDD
    5   !I/O from HDD
    6   !C/D
    7   !REQ from HDD
*/

uint8_t rmnimbus_state::scsi_r(offs_t offset)
{
	int result = 0;

	int pc=m_maincpu->pc();
	char drive[5];
	floppy_image_device *floppy;

	sprintf(drive, "%d", FDC_DRIVE());
	floppy = m_fdc->subdevice<floppy_connector>(drive)->get_device();

	switch(offset*2)
	{
		case 0x00 :
			result |= m_scsi_req << 7;
			result |= m_scsi_cd << 6;
			result |= m_scsi_io << 5;
			result |= m_scsi_bsy << 4;
			result |= m_scsi_msg << 3;
			if(floppy)
			{
				result |= FDC_MOTOR() << 2;
				result |= (!floppy->idx_r()) << 1;
				result |= (floppy->dskchg_r()) << 0;
			}
			break;
		case 0x08 :
			result = m_scsi_data_in->read();
			hdc_post_rw();
			break;
		default:
			break;
	}

	LOGMASKED(LOG_DISK_HDD, "Nimbus HDCR at pc=%08X from %04X data=%02X\n",pc,(offset*2)+0x410,result);

	return result;
}

/*
    0x400 write bits

    0   drive 0 select
    1   drive 1 select
    2   drive 2 select
    3   drive 3 select
    4   side select
    5   fdc motor on
    6   hdc drq enabled
    7   fdc drq enabled
*/
void rmnimbus_state::fdc_ctl_w(uint8_t data)
{
	uint8_t old_drq = m_nimbus_drives.reg400 & HDC_DRQ_MASK;
	char drive[5];
	floppy_image_device *floppy;

	m_nimbus_drives.reg400 = data;

	sprintf(drive, "%d", FDC_DRIVE());
	floppy = m_fdc->subdevice<floppy_connector>(drive)->get_device();

	m_fdc->set_floppy(floppy);
	if(floppy)
	{
		floppy->ss_w(FDC_SIDE());
		floppy->mon_w(!FDC_MOTOR());
	}

	// if we enable hdc drq with a pending condition, act on it
	if((data & HDC_DRQ_MASK) && (!old_drq))
		set_scsi_drqlat(false, false);
}

/*
    0x410 write bits

    0   SCSI reset
    1   SCSI SEL
    2   SCSI IRQ Enable
*/

void rmnimbus_state::scsi_w(offs_t offset, uint8_t data)
{
	int pc=m_maincpu->pc();

	LOGMASKED(LOG_DISK_HDD, "Nimbus HDCW at %05X write of %02X to %04X\n",pc,data,(offset*2)+0x410);

	switch(offset*2)
	{
		case 0x00 :
			m_scsi_ctrl_out->write(data);
			break;

		case 0x08 :
			m_scsi_data_out->write(data);
			hdc_post_rw();
			break;
	}
}

void rmnimbus_state::hdc_reset()
{
	m_scsi_iena = 0;
	m_scsi_msg = 0;
	m_scsi_bsy = 0;
	m_scsi_io = 0;
	m_scsi_cd = 0;
	m_scsi_req = 0;

	// Latched req, IC11b
	m_scsi_reqlat = 0;
}

/*
    The SCSI code outputs a 1 to indicate an active line, even though it is active low
    The inputs on the RM schematic are fed through inverters, but because of the above
    we don't need to invert them, unless the schematic uses the signal directly
    For consistency we will invert msg before latching.
*/

void rmnimbus_state::check_scsi_irq()
{
	nimbus_fdc_intrq_w(m_scsi_io && m_scsi_cd && m_scsi_req && m_scsi_iena);
}

void rmnimbus_state::write_scsi_iena(int state)
{
	m_scsi_iena = state;
	check_scsi_irq();
}

// This emulates the 74LS74 latched version of req
void rmnimbus_state::set_scsi_drqlat(bool   clock, bool clear)
{
	if (clear)
		m_scsi_reqlat = 0;
	else if (clock)
		m_scsi_reqlat = 1;

	if(m_scsi_reqlat)
		hdc_drq(true);
	else
		hdc_drq(false);
}

void rmnimbus_state::hdc_post_rw()
{
	if(m_scsi_req)
		m_scsibus->write_ack(1);

	// IC17A, IC17B, latched req cleared by SCSI data read or write, or C/D= command
	set_scsi_drqlat(false, true);
}

void rmnimbus_state::hdc_drq(bool state)
{
	m_maincpu->drq1_w(HDC_DRQ_ENABLED() && !m_scsi_cd && state);
}

void rmnimbus_state::write_scsi_bsy(int state)
{
	m_scsi_bsy = state;
}

void rmnimbus_state::write_scsi_cd(int state)
{
	m_scsi_cd = state;

	// IC17A, IC17B, latched req cleared by SCSI data read or write, or C/D= command
	set_scsi_drqlat(false, !m_scsi_cd);

	check_scsi_irq();
}

void rmnimbus_state::write_scsi_io(int state)
{
	m_scsi_io = state;

	if (m_scsi_io)
	{
		m_scsi_data_out->write(0);
	}
	check_scsi_irq();
}

void rmnimbus_state::write_scsi_msg(int state)
{
	m_scsi_msg = !state;
}

void rmnimbus_state::write_scsi_req(int state)
{
	// Detect rising edge on req, IC11b, clock
	int rising = ((m_scsi_req == 0) && (state == 1));

	// This is the state of the actual line from the SCSI
	m_scsi_req = state;

	// Latched req, is forced low by C/D being set to command
	set_scsi_drqlat(rising, m_scsi_cd);

	if (!m_scsi_reqlat)
		m_scsibus->write_ack(0);

	check_scsi_irq();
}

void rmnimbus_state::nimbus_voice_w(offs_t offset, uint8_t data)
{
	if (offset == 0xB0)
		m_voice_enabled = true;
	else if (offset == 0xB2)
		m_voice_enabled = false;
}

/* 8031/8051 Peripheral controller 80186 side */

void rmnimbus_state::pc8031_reset()
{
	logerror("peripheral controller reset\n");

	memset(&m_ipc_interface,0,sizeof(m_ipc_interface));
}


#if 0
void rmnimbus_state::ipc_dumpregs()
{
	logerror("in_data=%02X, in_status=%02X, out_data=%02X, out_status=%02X\n",
				m_ipc_interface.ipc_in, m_ipc_interface.status_in,
				m_ipc_interface.ipc_out, m_ipc_interface.status_out);

}
#endif

uint8_t rmnimbus_state::nimbus_pc8031_r(offs_t offset)
{
	int pc=m_maincpu->pc();
	uint8_t   result;

	switch(offset*2)
	{
		case 0x00   : result=m_ipc_interface.ipc_out;
						m_ipc_interface.status_in   &= ~IPC_IN_READ_PEND;
						m_ipc_interface.status_out  &= ~IPC_OUT_BYTE_AVAIL;
						break;

		case 0x02   : result=m_ipc_interface.status_out;
						break;

		default : result=0; break;
	}

	LOGMASKED(LOG_PC8031_186, "Nimbus PCIOR %08X read of %04X returns %02X\n",pc,(offset*2)+0xC0,result);

	return result;
}

void rmnimbus_state::nimbus_pc8031_w(offs_t offset, uint8_t data)
{
	int pc=m_maincpu->pc();

	switch(offset*2)
	{
		case 0x00   : m_ipc_interface.ipc_in=data;
						m_ipc_interface.status_in   |= IPC_IN_BYTE_AVAIL;
						m_ipc_interface.status_in   &= ~IPC_IN_ADDR;
						m_ipc_interface.status_out  |= IPC_OUT_READ_PEND;
						break;

		case 0x02   : m_ipc_interface.ipc_in=data;
						m_ipc_interface.status_in   |= IPC_IN_BYTE_AVAIL;
						m_ipc_interface.status_in   |= IPC_IN_ADDR;
						m_ipc_interface.status_out  |= IPC_OUT_READ_PEND;
						break;
	}

	LOGMASKED(LOG_PC8031_186, "Nimbus PCIOW %08X write of %02X to %04X\n",pc,data,(offset*2)+0xC0);
}

/* 8031/8051 Peripheral controller 8031/8051 side */

uint8_t rmnimbus_state::nimbus_pc8031_iou_r(offs_t offset)
{
	int pc=m_iocpu->pc();
	uint8_t   result = 0;

	switch (offset & 0x01)
	{
		case 0x00   : result=m_ipc_interface.ipc_in;
						m_ipc_interface.status_out  &= ~IPC_OUT_READ_PEND;
						m_ipc_interface.status_in   &= ~IPC_IN_BYTE_AVAIL;
						break;

		case 0x01   : result=m_ipc_interface.status_in;
						break;
	}

	if(((offset==2) || (offset==3)) && (m_iou_reg092 & PC8031_INT_ENABLE))
		external_int(EXTERNAL_INT_PC8031_8C, true);

	LOGMASKED(LOG_PC8031, "8031: PCIOR %04X read of %04X returns %02X\n",pc,offset,result);

	return result;
}

void rmnimbus_state::nimbus_pc8031_iou_w(offs_t offset, uint8_t data)
{
	int pc=m_iocpu->pc();

	LOGMASKED(LOG_PC8031, "8031 PCIOW %04X write of %02X to %04X\n",pc,data,offset);

	switch(offset & 0x03)
	{
		case 0x00   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out  |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out  &= ~IPC_OUT_ADDR;
						m_ipc_interface.status_in   |= IPC_IN_READ_PEND;
						break;

		case 0x01   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out   |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out   |= IPC_OUT_ADDR;
						m_ipc_interface.status_in    |= IPC_IN_READ_PEND;
						break;

		case 0x02   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out  |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out  &= ~IPC_OUT_ADDR;
						m_ipc_interface.status_in   |= IPC_IN_READ_PEND;
						if(m_iou_reg092 & PC8031_INT_ENABLE)
							external_int(EXTERNAL_INT_PC8031_8F, true);
						break;

		case 0x03   : m_ipc_interface.ipc_out=data;
						//m_ipc_interface.status_out   |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out   |= IPC_OUT_ADDR;
						m_ipc_interface.status_in    |= IPC_IN_READ_PEND;
						if(m_iou_reg092 & PC8031_INT_ENABLE)
							external_int(EXTERNAL_INT_PC8031_8E, true);
						break;
	}
}

uint8_t rmnimbus_state::nimbus_pc8031_port1_r()
{
	int pc=m_iocpu->pc();
	uint8_t   result = (m_eeprom_bits & ~4) | (m_eeprom->do_read() << 2);

	LOGMASKED(LOG_PC8031_PORT, "8031: PCPORTR %04X read of P1 returns %02X\n",pc,result);

	return result;
}

uint8_t rmnimbus_state::nimbus_pc8031_port3_r()
{
	int pc=m_iocpu->pc();
	uint8_t   result = 0;

	LOGMASKED(LOG_PC8031_PORT, "8031: PCPORTR %04X read of P3 returns %02X\n",pc,result);

	return result;
}

void rmnimbus_state::nimbus_pc8031_port1_w(uint8_t data)
{
	int pc=m_iocpu->pc();

	m_eeprom->cs_write((data & 8) ? 1 : 0);

	if(!(data & 8))
		m_eeprom_state = 0;
	else if(!(data & 2) || (m_eeprom_state == 2))
		m_eeprom_state = 2;
	else if((data & 8) && (!(m_eeprom_bits & 8)))
		m_eeprom_state = 1;
	else if((!(data & 1)) && (m_eeprom_bits & 1) && (m_eeprom_state == 1))
		m_eeprom_state = 2; //wait until 1 clk after cs rises to set di else it's seen as a start bit

	m_eeprom->di_write(((data & 2) && (m_eeprom_state == 2)) ? 1 : 0);
	m_eeprom->clk_write((data & 1) ? 1 : 0);
	m_eeprom_bits = data;

	LOGMASKED(LOG_PC8031_PORT, "8031 PCPORTW %04X write of %02X to P1\n",pc,data);
}

void rmnimbus_state::nimbus_pc8031_port3_w(uint8_t data)
{
	int pc = m_iocpu->pc();
	LOGMASKED(LOG_PC8031_PORT, "8031 PCPORTW %04X write of %02X to P3\n",pc,data);
}


/* IO Unit */
uint8_t rmnimbus_state::nimbus_iou_r(offs_t offset)
{
	int pc=m_maincpu->pc();
	uint8_t   result=0;

	if(offset==0)
	{
		result=m_iou_reg092;
	}

	LOGMASKED(LOG_IOU, "Nimbus IOUR %08X read of %04X returns %02X\n",pc,(offset*2)+0x92,result);

	return result;
}

void rmnimbus_state::nimbus_iou_w(offs_t offset, uint8_t data)
{
	int pc=m_maincpu->pc();

	LOGMASKED(LOG_IOU, "Nimbus IOUW %08X write of %02X to %04X\n",pc,data,(offset*2)+0x92);

	if(offset==0)
	{
		m_iou_reg092=data;
		m_msm->reset_w((data & MSM5205_INT_ENABLE) ? 0 : 1);
	}
}

void rmnimbus_state::iou_reset()
{
	m_iou_reg092=0x00;
	m_eeprom_state = 0;
}

/* Rompacks, not completely implemented */

uint8_t rmnimbus_state::nimbus_rompack_r(offs_t offset)
{
	logerror("Rompack read offset %02X, rompack address=%04X\n",offset,(m_ay8910_b*256)+m_ay8910_a);

	return 0;
}

void rmnimbus_state::nimbus_rompack_w(offs_t offset, uint8_t data)
{
	logerror("Rompack write offset %02X, data=%02X, rompack address=%04X\n",offset,data,(m_ay8910_b*256)+m_ay8910_a);
}

/*
    Sound hardware : AY8910

    I believe that the IO ports of the 8910 are used to control the ROMPack ports, however
    this is currently un-implemented (and may never be as I don't have any rompacks!).

    The registers are mapped as so :

    Address     0xE0                0xE2
    Read        Data                ????
    Write       Register Address    Data

*/

void rmnimbus_state::rmni_sound_reset()
{
	m_msm->reset_w(1);

	m_last_playmode = msm5205_device::S48_4B;
	m_msm->playmode_w(m_last_playmode);

	m_ay8910_a=0;
	m_ay8910_b=0;
}

void rmnimbus_state::nimbus_sound_ay8910_porta_w(uint8_t data)
{
	m_msm->data_w(data);

	// Mouse code needs a copy of this.
	// ROMpack lower address lines
	m_ay8910_a=data;
}

void rmnimbus_state::nimbus_sound_ay8910_portb_w(uint8_t data)
{
	// Only update msm5205 if voice is enabled.....
	if (m_voice_enabled  && ((data & 0x07) != m_last_playmode))
	{
		m_last_playmode = (data & 0x07);
		m_msm->playmode_w(m_last_playmode);
	}

	// ROMpack upper address lines
	m_ay8910_b=data;
}

void rmnimbus_state::nimbus_msm5205_vck(int state)
{
	if(m_iou_reg092 & MSM5205_INT_ENABLE)
		external_int(EXTERNAL_INT_MSM5205,state);
}

static const int MOUSE_XYA[4] = { 1, 1, 0, 0 };
static const int MOUSE_XYB[4] = { 0, 1, 1, 0 };

TIMER_CALLBACK_MEMBER(rmnimbus_state::do_mouse)
{
	uint8_t mouse_x;        // Current mouse X and Y
	uint8_t mouse_y;
	int8_t  xdiff;          // Difference from previous X and Y
	int8_t  ydiff;

	// Read mose positions and calculate difference from previous value
	mouse_x = m_io_mousex->read();
	mouse_y = m_io_mousey->read();

	xdiff = m_nimbus_mouse.m_mouse_x - mouse_x;
	ydiff = m_nimbus_mouse.m_mouse_y - mouse_y;

	if (m_io_config->read() & 0x01)
	{
		do_mouse_hle(xdiff, ydiff);
	}
	else
	{
		do_mouse_real(xdiff, ydiff);
	}

	// Update current mouse position
	m_nimbus_mouse.m_mouse_x = mouse_x;
	m_nimbus_mouse.m_mouse_y = mouse_y;
}

void rmnimbus_state::do_mouse_real(int8_t xdiff, int8_t ydiff)
{
	uint8_t intstate_x;     // Used to calculate if we should trigger interrupt
	uint8_t intstate_y;
	int     xint;           // X and Y interrupts to trigger
	int     yint;

	uint8_t   mxa;          // Values of quadrature encoders for X and Y
	uint8_t   mxb;
	uint8_t   mya;
	uint8_t   myb;

	// convert movement into emulated movement of quadrature encoder in mouse.
	if (xdiff < 0)
		m_nimbus_mouse.m_mouse_pcx++;
	else if (xdiff > 0)
		m_nimbus_mouse.m_mouse_pcx--;

	if (ydiff < 0)
		m_nimbus_mouse.m_mouse_pcy++;
	else if (ydiff > 0)
		m_nimbus_mouse.m_mouse_pcy--;

	// Compensate for quadrature wrap.
	m_nimbus_mouse.m_mouse_pcx &= 0x03;
	m_nimbus_mouse.m_mouse_pcy &= 0x03;

	// get value of mouse quadrature encoders for this wheel position
	mxa = MOUSE_XYA[m_nimbus_mouse.m_mouse_pcx]; // XA
	mxb = MOUSE_XYB[m_nimbus_mouse.m_mouse_pcx]; // XB
	mya = MOUSE_XYA[m_nimbus_mouse.m_mouse_pcy]; // YA
	myb = MOUSE_XYB[m_nimbus_mouse.m_mouse_pcy]; // YB

	// calculate interrupt state
	intstate_x = (mxb ^ mxa) ^ ((m_ay8910_a & 0x40) >> 6);
	intstate_y = (myb ^ mya) ^ ((m_ay8910_a & 0x80) >> 7);

	// Generate interrupts if enabled, otherwise return values in
	// mouse register
	if (MOUSE_INT_ENABLED(this))
	{
		if ((intstate_x==1) && (m_nimbus_mouse.m_intstate_x==0))
		{
			xint=mxa ? EXTERNAL_INT_MOUSE_XL : EXTERNAL_INT_MOUSE_XR;

			external_int(xint, true);
		}

		if ((intstate_y==1) && (m_nimbus_mouse.m_intstate_y==0))
		{
			yint=myb ? EXTERNAL_INT_MOUSE_YD : EXTERNAL_INT_MOUSE_YU;

			external_int(yint, true);
		}
	}
	else
	{
		m_nimbus_mouse.m_reg0a4 &= 0xF0;
		m_nimbus_mouse.m_reg0a4 |= ( mxb & 0x01) << 3; // XB
		m_nimbus_mouse.m_reg0a4 |= (~mxb & 0x01) << 2; // XA
		m_nimbus_mouse.m_reg0a4 |= (~myb & 0x01) << 1; // YA
		m_nimbus_mouse.m_reg0a4 |= ( myb & 0x01) << 0; // YB
	}

	// and interrupt state
	m_nimbus_mouse.m_intstate_x=intstate_x;
	m_nimbus_mouse.m_intstate_y=intstate_y;
}

void rmnimbus_state::do_mouse_hle(int8_t xdiff, int8_t ydiff)
{
	if (MOUSE_INT_ENABLED(this))
	{
		// bypass bios ISR and update mouse cursor position locations directly
		address_space &space = m_maincpu->space(AS_PROGRAM);

		if (xdiff)
		{
			uint16_t x = space.read_word_unaligned(m_nimbus_mouse.xpos_loc);
			if ((xdiff < 0) && (x != space.read_word_unaligned(m_nimbus_mouse.xmax_loc)))
			{
				x++;
			}
			else if (x != space.read_word_unaligned(m_nimbus_mouse.xmin_loc))
			{
				x--;
			}
			space.write_word_unaligned(m_nimbus_mouse.xpos_loc, x);
		}

		if (ydiff)
		{
			uint16_t y = space.read_word_unaligned(m_nimbus_mouse.ypos_loc);
			if ((ydiff < 0) && (y != space.read_word_unaligned(m_nimbus_mouse.ymin_loc)))
			{
				y--;
			}
			else if (y != space.read_word_unaligned(m_nimbus_mouse.ymax_loc))
			{
				y++;
			}
			space.write_word_unaligned(m_nimbus_mouse.ypos_loc, y);
		}
	}
	else
	{
		// store status to support polling operation of mouse
		// (not tested as no software seems to use this method!)
		if (xdiff || ydiff)
		{
			m_nimbus_mouse.m_reg0a4 = 0;
		}
		if (xdiff < 0)
		{
			m_nimbus_mouse.m_reg0a4 |= CONTROLLER_RIGHT;
		}
		else if (xdiff > 0)
		{
			m_nimbus_mouse.m_reg0a4 |= CONTROLLER_LEFT;
		}
		if (ydiff < 0)
		{
			m_nimbus_mouse.m_reg0a4 |= CONTROLLER_DOWN;
		}
		else if (ydiff > 0)
		{
			m_nimbus_mouse.m_reg0a4 |= CONTROLLER_UP;
		}

	}
}

void rmnimbus_state::mouse_js_reset()
{
	constexpr uint16_t bios_addresses[] = { 0x18cd, 0x196d, 0x196d, 0x1a6d, 0x1a6d, 0x1a77 };

	m_nimbus_mouse.m_mouse_x=128;
	m_nimbus_mouse.m_mouse_y=128;
	m_nimbus_mouse.m_mouse_pcx=0;
	m_nimbus_mouse.m_mouse_pcy=0;
	m_nimbus_mouse.m_intstate_x=0;
	m_nimbus_mouse.m_intstate_y=0;
	m_nimbus_mouse.m_reg0a4=0xC0;

	// calculate addresses for mouse related variable used by the bios mouse ISR
	auto bios_base = bios_addresses[system_bios() - 1];
	m_nimbus_mouse.xpos_loc = bios_base + 8;
	m_nimbus_mouse.ypos_loc = bios_base + 10;
	m_nimbus_mouse.xmin_loc = bios_base;
	m_nimbus_mouse.ymin_loc = bios_base + 4;
	m_nimbus_mouse.xmax_loc = bios_base + 2;
	m_nimbus_mouse.ymax_loc = bios_base + 6;

	// Setup timer to poll the mouse
	m_nimbus_mouse.m_mouse_timer->adjust(attotime::zero, 0, attotime::from_hz(MOUSE_POLL_FREQUENCY));

	m_selected_js_idx = 0;
}

uint8_t rmnimbus_state::nimbus_joystick_r()
{
	/* Only the joystick drection data is read from this port
	   (which corresponds to the the low nibble of the selected joystick port).
	   The joystick buttons are read from the mouse data port instead.
	   Unused bits are set to 1. */
	uint8_t result = m_io_joysticks[m_selected_js_idx]->read() | 0xf0;

	if (result & CONTROLLER_RIGHT)
	{
		// when the stick is pushed right the left bit must also be set!
		result |= CONTROLLER_LEFT;
	}

	if (result & CONTROLLER_UP)
	{
		// when the stick is pushed up the down bit must also be set!
		result |= CONTROLLER_DOWN;
	}

	return result;
}

void rmnimbus_state::nimbus_joystick_select(offs_t offset, uint8_t data)
{
	/* NB joystick 0 is selected by writing to address 0xa0, and
	   joystick 1 is selected by writing to address 0xa2 */
	if (offset % 2 == 0)
	{
		m_selected_js_idx = offset >> 1;
	}
}

uint8_t rmnimbus_state::nimbus_mouse_js_r()
{
	/*

	    bit     description

	    0       mouse XB
	    1       mouse XA
	    2       mouse YA
	    3       mouse YB
	    4       JOY 1-button or mouse rbutton
	    5       JOY 0-button or mouse lbutton
	    6       ?? always reads 1
	    7       ?? always reads 1

	*/
	uint8_t result = m_nimbus_mouse.m_reg0a4 | 0xc0;

	// set button bits if either mouse or joystick buttons are pressed
	result |= m_io_mouse_button->read();
	// NB only the button bits of the joystick(s) are read from this port
	result |= m_io_joysticks[0]->read() & 0x20;
	result |= m_io_joysticks[1]->read() & 0x10;

	return result;
}

// Clear mose latches
void rmnimbus_state::nimbus_mouse_js_w(uint8_t data)
{
	m_nimbus_mouse.m_reg0a4 = 0x00;
	//logerror("clear mouse latches\n");
}


/**********************************************************************
Parallel printer / User port.
The Nimbus parallel printer port card is almost identical to the circuit
in the BBC micro, so I have borrowed the driver code from the BBC :)

Port A output is buffered before being connected to the printer connector.
This means that they can only be operated as output lines.
CA1 is pulled high by a 4K7 resistor. CA1 normally acts as an acknowledge
line when a printer is used. CA2 is buffered so that it has become an open
collector output only. It usually acts as the printer strobe line.
***********************************************************************/

/* USER VIA 6522 port B is connected to the BBC user port */
void rmnimbus_state::nimbus_via_write_portb(uint8_t data)
{
}
