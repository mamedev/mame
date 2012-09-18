/***************************************************************************
    commodore c65 home computer
    peter.trauner@jk.uni-linz.ac.at
    documention
     www.funet.fi
 ***************************************************************************/

#include "emu.h"

#include "includes/cbm.h"
#include "includes/c65.h"
#include "includes/c64_legacy.h"
#include "cpu/m6502/m4510.h"
#include "sound/sid6581.h"
#include "machine/6526cia.h"
#include "machine/cbmiec.h"
#include "machine/ram.h"
#include "video/vic4567.h"

#define VERBOSE_LEVEL 0
#define DBG_LOG( MACHINE, N, M, A ) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", MACHINE.time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)





/*UINT8 *c65_basic; */
/*UINT8 *c65_kernal; */
/*UINT8 *c65_dos; */
/*UINT8 *c65_monitor; */
/*UINT8 *c65_graphics; */


static void c65_nmi( running_machine &machine )
{
	c65_state *state = machine.driver_data<c65_state>();
	device_t *cia_1 = machine.device("cia_1");
	int cia1irq = mos6526_irq_r(cia_1);

	if (state->m_nmilevel != (machine.root_device().ioport("SPECIAL")->read() & 0x80) || cia1irq)	/* KEY_RESTORE */
	{
		machine.device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, (machine.root_device().ioport("SPECIAL")->read() & 0x80) || cia1irq);

		state->m_nmilevel = (machine.root_device().ioport("SPECIAL")->read() & 0x80) || cia1irq;
	}
}


/***********************************************

    CIA Interfaces

***********************************************/

/*
 *  CIA 0 - Port A keyboard line select
 *  CIA 0 - Port B keyboard line read
 *
 *  flag cassette read input, serial request in
 *  irq to irq connected
 *
 *  see machine/cbm.c
 */

static READ8_DEVICE_HANDLER( c65_cia0_port_a_r )
{
	UINT8 cia0portb = mos6526_pb_r(device->machine().device("cia_0"), space, 0);

	return cbm_common_cia0_port_a_r(device, cia0portb);
}

static READ8_DEVICE_HANDLER( c65_cia0_port_b_r )
{
	c65_state *state = device->machine().driver_data<c65_state>();
	UINT8 value = 0xff;
	UINT8 cia0porta = mos6526_pa_r(device->machine().device("cia_0"), space, 0);

	value &= cbm_common_cia0_port_b_r(device, cia0porta);

	if (!(state->m_6511_port & 0x02))
		value &= state->m_keyline;

	return value;
}

static WRITE8_DEVICE_HANDLER( c65_cia0_port_b_w )
{
//  was there lightpen support in c65 video chip?
//  device_t *vic3 = device->machine().device("vic3");
//  vic3_lightpen_write(vic3, data & 0x10);
}

static void c65_irq( running_machine &machine, int level )
{
	c65_state *state = machine.driver_data<c65_state>();
	if (level != state->m_old_level)
	{
		DBG_LOG(machine, 3, "mos6510", ("irq %s\n", level ? "start" : "end"));
		machine.device("maincpu")->execute().set_input_line(M6510_IRQ_LINE, level);
		state->m_old_level = level;
	}
}

/* is this correct for c65 as well as c64? */
static void c65_cia0_interrupt( device_t *device, int level )
{
	c65_state *state = device->machine().driver_data<c65_state>();
	c65_irq (device->machine(), level || state->m_vicirq);
}

/* is this correct for c65 as well as c64? */
void c65_vic_interrupt( running_machine &machine, int level )
{
	c65_state *state = machine.driver_data<c65_state>();
	device_t *cia_0 = machine.device("cia_0");
#if 1
	if (level != state->m_vicirq)
	{
		c65_irq (machine, level || mos6526_irq_r(cia_0));
		state->m_vicirq = level;
	}
#endif
}

const mos6526_interface c65_cia0 =
{
	DEVCB_LINE(c65_cia0_interrupt),
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(c65_cia0_port_a_r),
	DEVCB_NULL,
	DEVCB_HANDLER(c65_cia0_port_b_r),
	DEVCB_HANDLER(c65_cia0_port_b_w)
};

/*
 * CIA 1 - Port A
 * bit 7 serial bus data input
 * bit 6 serial bus clock input
 * bit 5 serial bus data output
 * bit 4 serial bus clock output
 * bit 3 serial bus atn output
 * bit 2 rs232 data output
 * bits 1-0 vic-chip system memory bank select
 *
 * CIA 1 - Port B
 * bit 7 user rs232 data set ready
 * bit 6 user rs232 clear to send
 * bit 5 user
 * bit 4 user rs232 carrier detect
 * bit 3 user rs232 ring indicator
 * bit 2 user rs232 data terminal ready
 * bit 1 user rs232 request to send
 * bit 0 user rs232 received data
 *
 * flag restore key or rs232 received data input
 * irq to nmi connected ?
 */
static READ8_DEVICE_HANDLER( c65_cia1_port_a_r )
{
	c65_state *state = device->machine().driver_data<c65_state>();
	UINT8 value = 0xff;

	if (!state->m_iec->clk_r())
		value &= ~0x40;

	if (!state->m_iec->data_r())
		value &= ~0x80;

	return value;
}

static WRITE8_DEVICE_HANDLER( c65_cia1_port_a_w )
{
	c65_state *state = device->machine().driver_data<c65_state>();
	static const int helper[4] = {0xc000, 0x8000, 0x4000, 0x0000};

	state->m_iec->atn_w(!BIT(data, 3));
	state->m_iec->clk_w(!BIT(data, 4));
	state->m_iec->data_w(!BIT(data, 5));

	state->m_vicaddr = state->m_memory + helper[data & 0x03];
}

static WRITE_LINE_DEVICE_HANDLER( c65_cia1_interrupt )
{
	c65_nmi(device->machine());
}

const mos6526_interface c65_cia1 =
{
	DEVCB_LINE(c65_cia1_interrupt),
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(c65_cia1_port_a_r),
	DEVCB_HANDLER(c65_cia1_port_a_w),
	DEVCB_NULL,
	DEVCB_NULL
};

/***********************************************

    Memory Handlers

***********************************************/

/* processor has only 1 mega address space !? */
/* and system 8 megabyte */
/* dma controller and bankswitch hardware ?*/
static DECLARE_READ8_HANDLER( c65_read_mem );
static READ8_HANDLER( c65_read_mem )
{
	c65_state *state = space.machine().driver_data<c65_state>();
	UINT8 result;
	if (offset <= 0x0ffff)
		result = state->m_memory[offset];
	else
		result = space.read_byte(offset);
	return result;
}

static DECLARE_WRITE8_HANDLER( c65_write_mem );
static WRITE8_HANDLER( c65_write_mem )
{
	c65_state *state = space.machine().driver_data<c65_state>();
	if (offset <= 0x0ffff)
		state->m_memory[offset] = data;
	else
		space.write_byte(offset, data);
}

/* dma chip at 0xd700
  used:
   writing banknumber to offset 2
   writing hibyte to offset 1
   writing lobyte to offset 0
    cpu holded, dma transfer(data at address) executed, cpu activated

  command data:
   0 command (0 copy, 3 fill)
   1,2 length
   3,4,5 source
   6,7,8 dest
   9 subcommand
   10 mod

   version 1:
   seldom copy (overlapping) from 0x402002 to 0x402008
   (making place for new line in basic area)
   for whats this bit 0x400000, or is this really the address?
   maybe means add counter to address for access,
   so allowing up or down copies, and reordering copies

   version 2:
   cmd 0x30 used for this
*/
static void c65_dma_port_w( running_machine &machine, int offset, int value )
{
	c65_state *state = machine.driver_data<c65_state>();
	PAIR pair, src, dst, len;
	UINT8 cmd, fill;
	int i;
	address_space &space = *machine.device("maincpu")->memory().space(AS_PROGRAM);

	switch (offset & 3)
	{
	case 2:
	case 1:
		state->m_dma.data[offset & 3] = value;
		break;
	case 0:
		pair.b.h3 = 0;
		pair.b.h2 = state->m_dma.data[2];
		pair.b.h = state->m_dma.data[1];
		pair.b.l = state->m_dma.data[0]=value;
		cmd = c65_read_mem(space, pair.d++);
		len.w.h = 0;
		len.b.l = c65_read_mem(space, pair.d++);
		len.b.h = c65_read_mem(space, pair.d++);
		src.b.h3 = 0;
		fill = src.b.l = c65_read_mem(space, pair.d++);
		src.b.h = c65_read_mem(space, pair.d++);
		src.b.h2 = c65_read_mem(space, pair.d++);
		dst.b.h3 = 0;
		dst.b.l = c65_read_mem(space, pair.d++);
		dst.b.h = c65_read_mem(space, pair.d++);
		dst.b.h2 = c65_read_mem(space, pair.d++);

		switch (cmd)
		{
		case 0:
			if (src.d == 0x3ffff) state->m_dump_dma = 1;
			if (state->m_dump_dma)
				DBG_LOG(space.machine(), 1,"dma copy job",
						("len:%.4x src:%.6x dst:%.6x sub:%.2x modrm:%.2x\n",
						 len.w.l, src.d, dst.d, c65_read_mem(space, pair.d),
						 c65_read_mem(space, pair.d + 1) ) );
			if ((state->m_dma.version == 1)
				 && ( (src.d&0x400000) || (dst.d & 0x400000)))
			{
				if (!(src.d & 0x400000))
				{
					dst.d &= ~0x400000;
					for (i = 0; i < len.w.l; i++)
						c65_write_mem(space, dst.d--, c65_read_mem(space, src.d++));
				}
				else if (!(dst.d & 0x400000))
				{
					src.d &= ~0x400000;
					for (i = 0; i < len.w.l; i++)
						c65_write_mem(space, dst.d++, c65_read_mem(space, src.d--));
				}
				else
				{
					src.d &= ~0x400000;
					dst.d &= ~0x400000;
					for (i = 0; i < len.w.l; i++)
						c65_write_mem(space, --dst.d, c65_read_mem(space, --src.d));
				}
			}
			else
			{
				for (i = 0; i < len.w.l; i++)
					c65_write_mem(space, dst.d++, c65_read_mem(space, src.d++));
			}
			break;
		case 3:
			DBG_LOG(space.machine(), 3,"dma fill job",
					("len:%.4x value:%.2x dst:%.6x sub:%.2x modrm:%.2x\n",
					 len.w.l, fill, dst.d, c65_read_mem(space, pair.d),
					 c65_read_mem(space, pair.d + 1)));
				for (i = 0; i < len.w.l; i++)
					c65_write_mem(space, dst.d++, fill);
				break;
		case 0x30:
			DBG_LOG(space.machine(), 1,"dma copy down",
					("len:%.4x src:%.6x dst:%.6x sub:%.2x modrm:%.2x\n",
					 len.w.l, src.d, dst.d, c65_read_mem(space, pair.d),
					 c65_read_mem(space, pair.d + 1) ) );
			for (i = 0; i < len.w.l; i++)
				c65_write_mem(space, dst.d--,c65_read_mem(space, src.d--));
			break;
		default:
			DBG_LOG(space.machine(), 1,"dma job",
					("cmd:%.2x len:%.4x src:%.6x dst:%.6x sub:%.2x modrm:%.2x\n",
					 cmd,len.w.l, src.d, dst.d, c65_read_mem(space, pair.d),
					 c65_read_mem(space, pair.d + 1)));
		}
		break;
	default:
		DBG_LOG(space.machine(), 1, "dma chip write", ("%.3x %.2x\n", offset, value));
		break;
	}
}

static int c65_dma_port_r( running_machine &machine, int offset )
{
	/* offset 3 bit 7 in progress ? */
	DBG_LOG(machine, 2, "dma chip read", ("%.3x\n", offset));
    return 0x7f;
}

static void c65_6511_port_w( running_machine &machine, int offset, int value )
{
	c65_state *state = machine.driver_data<c65_state>();
	if (offset == 7)
	{
		state->m_6511_port = value;
	}
	DBG_LOG(machine, 2, "r6511 write", ("%.2x %.2x\n", offset, value));
}

static int c65_6511_port_r( running_machine &machine, int offset )
{
	int data = 0xff;

	if (offset == 7)
	{
		if (machine.root_device().ioport("SPECIAL")->read() & 0x20)
			data &= ~1;
	}
	DBG_LOG(machine, 2, "r6511 read", ("%.2x\n", offset));

	return data;
}

/* one docu states custom 4191 disk controller
 (for 2 1MB MFM disk drives, 1 internal, the other extern (optional) 1565
 with integrated 512 byte buffer

 0->0 reset ?

 0->1, 0->0, wait until 2 positiv, 1->0 ???

 0->0, 0 not 0 means no drive ???, other system entries


 reg 0 write/read
  0,1 written
  bit 1 set
  bit 2 set
  bit 3 set
  bit 4 set


 reg 0 read
  bit 0
  bit 1
  bit 2
  0..2 ->$1d4

 reg 1 write
  $01 written
  $18 written
  $46 written
  $80 written
  $a1 written
  $01 written, dec
  $10 written

 reg 2 read/write?(lsr)
  bit 2
  bit 4
  bit 5 busy waiting until zero, then reading reg 7
  bit 6 operation not activ flag!? or set overflow pin used
  bit 7 busy flag?

 reg 3 read/write?(rcr)
  bit 1
  bit 3
  bit 7 busy flag?

 reg 4
  track??
  0 written
  read -> $1d2
  cmp #$50
  bcs


 reg 5
  sector ??
  1 written
  read -> $1d3
  cmp #$b bcc


 reg 6
  head ??
  0 written
  read -> $1d1
  cmp #2 bcc

 reg 7 read
  #4e written
  12 times 0, a1 a1 a1 fe  written

 reg 8 read
  #ff written
  16 times #ff written

 reg 9
  #60 written

might use the set overflow input

$21a6c 9a6c format
$21c97 9c97 write operation
$21ca0 9ca0 get byte?
$21cab 9cab read reg 7
$21caf 9caf write reg 7
$21cb3
*/

#define FDC_LOST 4
#define FDC_CRC 8
#define FDC_RNF 0x10
#define FDC_BUSY 0x80
#define FDC_IRQ 0x200

#define FDC_CMD_MOTOR_SPIN_UP 0x10

#if 0
static void c65_fdc_state(void)
{
	c65_state *state = machine.driver_data<c65_state>();
	switch (state->m_fdc.state)
	{
	case FDC_CMD_MOTOR_SPIN_UP:
		if (machine.time() - state->m_fdc.time)
		{
			state->m_fdc.state = 0;
			state->m_fdc.status &= ~FDC_BUSY;
		}
		break;
	}
}
#endif

static void c65_fdc_w( running_machine &machine, int offset, int data )
{
	c65_state *state = machine.driver_data<c65_state>();
	DBG_LOG(machine, 1, "fdc write", ("%.5x %.2x %.2x\n", machine.device("maincpu")->safe_pc(), offset, data));
	switch (offset & 0xf)
	{
	case 0:
		state->m_fdc.reg[0] = data;
		break;
	case 1:
		state->m_fdc.reg[1] = data;
		switch (data & 0xf9)
		{
		case 0x20: // wait for motor spin up
			state->m_fdc.status &= ~(FDC_IRQ|FDC_LOST|FDC_CRC|FDC_RNF);
			state->m_fdc.status |= FDC_BUSY;
			state->m_fdc.time = machine.time();
			state->m_fdc.state = FDC_CMD_MOTOR_SPIN_UP;
			break;
		case 0: // cancel
			state->m_fdc.status &= ~(FDC_BUSY);
			state->m_fdc.state = 0;
			break;
		case 0x80: // buffered write
		case 0x40: // buffered read
		case 0x81: // unbuffered write
		case 0x41: // unbuffered read
		case 0x30:case 0x31: // step
			break;
		}
		break;
	case 2: case 3: // read only
		break;
	case 4:
		state->m_fdc.reg[offset & 0xf] = data;
		state->m_fdc.track = data;
		break;
	case 5:
		state->m_fdc.reg[offset & 0xf] = data;
		state->m_fdc.sector = data;
		break;
	case 6:
		state->m_fdc.reg[offset & 0xf] = data;
		state->m_fdc.head = data;
		break;
	case 7:
		state->m_fdc.buffer[state->m_fdc.cpu_pos++] = data;
		break;
	default:
		state->m_fdc.reg[offset & 0xf] = data;
		break;
	}
}

static int c65_fdc_r( running_machine &machine, int offset )
{
	c65_state *state = machine.driver_data<c65_state>();
	UINT8 data = 0;
	switch (offset & 0xf)
	{
	case 0:
		data = state->m_fdc.reg[0];
		break;
	case 1:
		data = state->m_fdc.reg[1];
		break;
	case 2:
		data = state->m_fdc.status;
		break;
	case 3:
		data = state->m_fdc.status >> 8;
		break;
	case 4:
		data = state->m_fdc.track;
		break;
	case 5:
		data = state->m_fdc.sector;
		break;
	case 6:
		data = state->m_fdc.head;
		break;
	case 7:
		data = state->m_fdc.buffer[state->m_fdc.cpu_pos++];
		break;
	default:
		data = state->m_fdc.reg[offset & 0xf];
		break;
	}
	DBG_LOG(machine, 1, "fdc read", ("%.5x %.2x %.2x\n", machine.device("maincpu")->safe_pc(), offset, data));
	return data;
}

/* version 1 ramcheck
   write 0:0
   read write read write 80000,90000,f0000
   write 0:8
   read write read write 80000,90000,f0000

   version 2 ramcheck???
   read 0:
   write 0:0
   read 0:
   first read and second read bit 0x80 set --> nothing
   write 0:0
   read 0
   write 0:ff
*/

static READ8_HANDLER( c65_ram_expansion_r )
{
	c65_state *state = space.machine().driver_data<c65_state>();
	UINT8 data = 0xff;
	if (space.machine().device<ram_device>(RAM_TAG)->size() > (128 * 1024))
		data = state->m_expansion_ram.reg;
	return data;
}

static WRITE8_HANDLER( c65_ram_expansion_w )
{
	c65_state *state = space.machine().driver_data<c65_state>();
	offs_t expansion_ram_begin;
	offs_t expansion_ram_end;

	if (space.machine().device<ram_device>(RAM_TAG)->size() > (128 * 1024))
	{
		state->m_expansion_ram.reg = data;

		expansion_ram_begin = 0x80000;
		expansion_ram_end = 0x80000 + (space.machine().device<ram_device>(RAM_TAG)->size() - 128*1024) - 1;

		if (data == 0x00) {
			space.install_readwrite_bank(expansion_ram_begin, expansion_ram_end,"bank16");
			state->membank("bank16")->set_base(space.machine().device<ram_device>(RAM_TAG)->pointer() + 128*1024);
		} else {
			space.nop_readwrite(expansion_ram_begin, expansion_ram_end);
		}
	}
}

static WRITE8_HANDLER( c65_write_io )
{
	sid6581_device *sid_0 = space.machine().device<sid6581_device>("sid_r");
	sid6581_device *sid_1 = space.machine().device<sid6581_device>("sid_l");
	device_t *vic3 = space.machine().device("vic3");

	switch (offset & 0xf00)
	{
	case 0x000:
		if (offset < 0x80)
			vic3_port_w(vic3, space, offset & 0x7f, data);
		else if (offset < 0xa0)
			c65_fdc_w(space.machine(), offset&0x1f,data);
		else
		{
			c65_ram_expansion_w(space, offset&0x1f, data, mem_mask);
			/*ram expansion crtl optional */
		}
		break;
	case 0x100:
	case 0x200:
	case 0x300:
		vic3_palette_w(vic3, space, offset - 0x100, data);
		break;
	case 0x400:
		if (offset<0x420) /* maybe 0x20 */
			sid_0->write(space, offset & 0x3f, data);
		else if (offset<0x440)
			sid_1->write(space, offset & 0x3f, data);
		else
			DBG_LOG(space.machine(), 1, "io write", ("%.3x %.2x\n", offset, data));
		break;
	case 0x500:
		DBG_LOG(space.machine(), 1, "io write", ("%.3x %.2x\n", offset, data));
		break;
	case 0x600:
		c65_6511_port_w(space.machine(), offset&0xff,data);
		break;
	case 0x700:
		c65_dma_port_w(space.machine(), offset&0xff, data);
		break;
	}
}

static WRITE8_HANDLER( c65_write_io_dc00 )
{
	device_t *cia_0 = space.machine().device("cia_0");
	device_t *cia_1 = space.machine().device("cia_1");

	switch (offset & 0xf00)
	{
	case 0x000:
		mos6526_w(cia_0, space, offset, data);
		break;
	case 0x100:
		mos6526_w(cia_1, space, offset, data);
		break;
	case 0x200:
	case 0x300:
		DBG_LOG(space.machine(), 1, "io write", ("%.3x %.2x\n", offset+0xc00, data));
		break;
	}
}

static READ8_HANDLER( c65_read_io )
{
	sid6581_device *sid_0 = space.machine().device<sid6581_device>("sid_r");
	sid6581_device *sid_1 = space.machine().device<sid6581_device>("sid_l");
	device_t *vic3 = space.machine().device("vic3");

	switch (offset & 0xf00)
	{
	case 0x000:
		if (offset < 0x80)
			return vic3_port_r(vic3, space, offset & 0x7f);
		if (offset < 0xa0)
			return c65_fdc_r(space.machine(), offset&0x1f);
		else
		{
			return c65_ram_expansion_r(space, offset&0x1f, mem_mask);
			/*return; ram expansion crtl optional */
		}
		break;
	case 0x100:
	case 0x200:
	case 0x300:
	/* read only !? */
		DBG_LOG(space.machine(), 1, "io read", ("%.3x\n", offset));
		break;
	case 0x400:
		if (offset < 0x420)
			return sid_0->read(space, offset & 0x3f);
		if (offset < 0x440)
			return sid_1->read(space, offset & 0x3f);
		DBG_LOG(space.machine(), 1, "io read", ("%.3x\n", offset));
		break;
	case 0x500:
		DBG_LOG(space.machine(), 1, "io read", ("%.3x\n", offset));
		break;
	case 0x600:
		return c65_6511_port_r(space.machine(), offset&0xff);
	case 0x700:
		return c65_dma_port_r(space.machine(), offset&0xff);
	}
	return 0xff;
}

static READ8_HANDLER( c65_read_io_dc00 )
{
	device_t *cia_0 = space.machine().device("cia_0");
	device_t *cia_1 = space.machine().device("cia_1");

	switch (offset & 0x300)
	{
	case 0x000:
		return mos6526_r(cia_0, space, offset);
	case 0x100:
		return mos6526_r(cia_1, space, offset);
	case 0x200:
	case 0x300:
		DBG_LOG(space.machine(), 1, "io read", ("%.3x\n", offset+0xc00));
		break;
	}
	return 0xff;
}


/*
d02f:
 init a5 96 written (seems to be switch to c65 or vic3 mode)
 go64 0 written
*/

/* bit 1 external sync enable (genlock)
   bit 2 palette enable
   bit 6 vic3 c65 character set */
void c65_bankswitch_interface( running_machine &machine, int value )
{
	c65_state *state = machine.driver_data<c65_state>();
	DBG_LOG(machine, 2, "c65 bankswitch", ("%.2x\n",value));

	if (state->m_io_on)
	{
		if (value & 1)
		{
			state->membank("bank8")->set_base(state->m_colorram + 0x400);
			state->membank("bank9")->set_base(state->m_colorram + 0x400);
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x0dc00, 0x0dfff, "bank8");
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_bank(0x0dc00, 0x0dfff, "bank9");
		}
		else
		{
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x0dc00, 0x0dfff, FUNC(c65_read_io_dc00));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x0dc00, 0x0dfff, FUNC(c65_write_io_dc00));
		}
	}

	state->m_io_dc00_on = !(value & 1);
#if 0
	/* cartridge roms !?*/
	if (value & 0x08)
		state->membank("bank1")->set_base(state->m_roml);
	else
		state->membank("bank1")->set_base(state->m_memory + 0x8000);

	if (value & 0x10)
		state->membank("bank2")->set_base(state->m_basic);
	else
		state->membank("bank2")->set_base(state->m_memory + 0xa000);
#endif
	if ((state->m_old_value^value) & 0x20)
	{
	/* bankswitching faulty when doing actual page */
		if (value & 0x20)
			state->membank("bank3")->set_base(state->m_basic);
		else
			state->membank("bank3")->set_base(state->m_memory + 0xc000);
	}
	state->m_charset_select = value & 0x40;
#if 0
	/* cartridge roms !?*/
	if (value & 0x80)
		state->membank("bank8")->set_base(state->m_kernal);
	else
		state->membank("bank6")->set_base(state->m_memory + 0xe000);
#endif
	state->m_old_value = value;
}

void c65_bankswitch( running_machine &machine )
{
	c65_state *state = machine.driver_data<c65_state>();
	int data, loram, hiram, charen;

	data = m4510_get_port(machine.device<legacy_cpu_device>("maincpu"));
	if (data == state->m_old_data)
		return;

	DBG_LOG(machine, 1, "bankswitch", ("%d\n", data & 7));
	loram = (data & 1) ? 1 : 0;
	hiram = (data & 2) ? 1 : 0;
	charen = (data & 4) ? 1 : 0;

	if ((!state->m_game && state->m_exrom) || (loram && hiram && !state->m_exrom))
		state->membank("bank1")->set_base(state->m_roml);
	else
		state->membank("bank1")->set_base(state->m_memory + 0x8000);

	if ((!state->m_game && state->m_exrom && hiram) || (!state->m_exrom))
		state->membank("bank2")->set_base(state->m_romh);
	else if (loram && hiram)
		state->membank("bank2")->set_base(state->m_basic);
	else
		state->membank("bank2")->set_base(state->m_memory + 0xa000);

	if ((!state->m_game && state->m_exrom) || (charen && (loram || hiram)))
	{
		state->m_io_on = 1;
		state->membank("bank6")->set_base(state->m_colorram);
		state->membank("bank7")->set_base(state->m_colorram);

		if (state->m_io_dc00_on)
		{
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x0dc00, 0x0dfff, FUNC(c65_read_io_dc00));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x0dc00, 0x0dfff, FUNC(c65_write_io_dc00));
		}
		else
		{
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x0dc00, 0x0dfff, "bank8");
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_bank(0x0dc00, 0x0dfff, "bank9");
			state->membank("bank8")->set_base(state->m_colorram + 0x400);
			state->membank("bank9")->set_base(state->m_colorram + 0x400);
		}
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x0d000, 0x0d7ff, FUNC(c65_read_io));
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x0d000, 0x0d7ff, FUNC(c65_write_io));
	}
	else
	{
		state->m_io_on = 0;
		state->membank("bank5")->set_base(state->m_memory + 0xd000);
		state->membank("bank7")->set_base(state->m_memory + 0xd800);
		state->membank("bank9")->set_base(state->m_memory + 0xdc00);
		if (!charen && (loram || hiram))
		{
			state->membank("bank4")->set_base(state->m_chargen);
			state->membank("bank6")->set_base(state->m_chargen + 0x800);
			state->membank("bank8")->set_base(state->m_chargen + 0xc00);
		}
		else
		{
			state->membank("bank4")->set_base(state->m_memory + 0xd000);
			state->membank("bank6")->set_base(state->m_memory + 0xd800);
			state->membank("bank8")->set_base(state->m_memory + 0xdc00);
		}
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x0d000, 0x0d7ff, "bank4");
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_bank(0x0d000, 0x0d7ff, "bank5");
	}

	if (!state->m_game && state->m_exrom)
	{
		state->membank("bank10")->set_base(state->m_romh);
	}
	else
	{
		if (hiram)
		{
			state->membank("bank10")->set_base(state->m_kernal);
		}
		else
		{
			state->membank("bank10")->set_base(state->m_memory + 0xe000);
		}
	}
	state->m_old_data = data;
}

#ifdef UNUSED_FUNCTION
void c65_colorram_write( running_machine &machine, int offset, int value )
{
	c65_state *state = machine.driver_data<c65_state>();
	state->m_colorram[offset & 0x7ff] = value | 0xf0;
}
#endif

/*
 * only 14 address lines
 * a15 and a14 portlines
 * 0x1000-0x1fff, 0x9000-0x9fff char rom
 */
int c65_dma_read( running_machine &machine, int offset )
{
	c65_state *state = machine.driver_data<c65_state>();
	if (!state->m_game && state->m_exrom)
	{
		if (offset < 0x3000)
			return state->m_memory[offset];
		return state->m_romh[offset & 0x1fff];
	}
	if ((state->m_vicaddr == state->m_memory) || (state->m_vicaddr == state->m_memory + 0x8000))
	{
		if (offset < 0x1000)
			return state->m_vicaddr[offset & 0x3fff];
		if (offset < 0x2000) {
			if (state->m_charset_select)
				return state->m_chargen[offset & 0xfff];
			else
				return state->m_chargen[offset & 0xfff];
		}
		return state->m_vicaddr[offset & 0x3fff];
	}
	return state->m_vicaddr[offset & 0x3fff];
}

int c65_dma_read_color( running_machine &machine, int offset )
{
	c65_state *state = machine.driver_data<c65_state>();
	if (state->m_c64mode)
		return state->m_colorram[offset & 0x3ff] & 0xf;
	return state->m_colorram[offset & 0x7ff];
}

static void c65_common_driver_init( running_machine &machine )
{
	c65_state *state = machine.driver_data<c65_state>();
	state->m_memory = auto_alloc_array_clear(machine, UINT8, 0x10000);
	state->membank("bank11")->set_base(state->m_memory + 0x00000);
	state->membank("bank12")->set_base(state->m_memory + 0x08000);
	state->membank("bank13")->set_base(state->m_memory + 0x0a000);
	state->membank("bank14")->set_base(state->m_memory + 0x0c000);
	state->membank("bank15")->set_base(state->m_memory + 0x0e000);

	cbm_common_init();
	state->m_keyline = 0xff;

	state->m_pal = 0;
	state->m_charset_select = 0;
	state->m_6511_port = 0xff;
	state->m_vicirq = 0;
	state->m_old_data = -1;

	/* C65 had no datasette port */
	state->m_tape_on = 0;
	state->m_game = 1;
	state->m_exrom = 1;

	/*memset(state->m_memory + 0x40000, 0, 0x800000 - 0x40000); */
}

DRIVER_INIT_MEMBER(c65_state,c65)
{
	m_dma.version = 2;
	c65_common_driver_init(machine());
}

DRIVER_INIT_MEMBER(c65_state,c65pal)
{
	m_dma.version = 1;
	c65_common_driver_init(machine());
	m_pal = 1;
}

MACHINE_START_MEMBER(c65_state,c65)
{
	/* clear upper memory */
	memset(machine().device<ram_device>(RAM_TAG)->pointer() + 128*1024, 0xff, machine().device<ram_device>(RAM_TAG)->size() -  128*1024);

//removed   cbm_drive_0_config (SERIAL, 10);
//removed   cbm_drive_1_config (SERIAL, 11);
	m_vicaddr = m_memory;

	m_c64mode = 0;

	c65_bankswitch_interface(machine(), 0xff);
	c65_bankswitch (machine());
}


INTERRUPT_GEN_MEMBER(c65_state::c65_frame_interrupt)
{
	int value;

	c65_nmi(machine());

	/* common keys input ports */
	cbm_common_interrupt(&device);

	/* c65 specific: function keys input ports */
	value = 0xff;

	value &= ~ioport("FUNCT")->read();
	m_keyline = value;
}
