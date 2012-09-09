/***************************************************************************

    commodore c128 home computer

    peter.trauner@jk.uni-linz.ac.at
    documentation:
     iDOC (http://www.softwolves.pp.se/idoc)
           Christian Janoff  mepk@c64.org

***************************************************************************/

#include "emu.h"
#include "includes/c128.h"
#include "includes/c64_legacy.h"
#include "includes/cbm.h"
#include "machine/cbmiec.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/6526cia.h"
#include "sound/sid6581.h"
#include "video/mos6566.h"
#include "video/mc6845.h"

#define MMU_PAGE1 ((((m_mmu[10]&0xf)<<8)|m_mmu[9])<<8)
#define MMU_PAGE0 ((((m_mmu[8]&0xf)<<8)|m_mmu[7])<<8)
#define MMU_VIC_ADDR ((m_mmu[6]&0xc0)<<10)
#define MMU_RAM_RCR_ADDR ((m_mmu[6]&0x30)<<14)
#define MMU_SIZE (c128_mmu_helper[m_mmu[6]&3])
#define MMU_BOTTOM (m_mmu[6]&4)
#define MMU_TOP (m_mmu[6]&8)
#define MMU_CPU8502 (m_mmu[5]&1)	   /* else z80 */
/* fastio output (c128_mmu[5]&8) else input */
#define MMU_FSDIR (m_mmu[5]&0x08)
#define MMU_GAME_IN (m_mmu[5]&0x10)
#define MMU_EXROM_IN (m_mmu[5]&0x20)
#define MMU_64MODE (m_mmu[5]&0x40)
#define MMU_40_IN (m_mmu[5]&0x80)

#define MMU_RAM_CR_ADDR ((m_mmu[0]&0xc0)<<10)
#define MMU_RAM_LO (m_mmu[0]&2)	   /* else rom at 0x4000 */
#define MMU_RAM_MID ((m_mmu[0]&0xc)==0xc)	/* 0x8000 - 0xbfff */
#define MMU_ROM_MID ((m_mmu[0]&0xc)==0)
#define MMU_EXTERNAL_ROM_MID ((m_mmu[0]&0xc)==8)
#define MMU_INTERNAL_ROM_MID ((m_mmu[0]&0xc)==4)

#define MMU_IO_ON (!(m_mmu[0]&1))   /* io window at 0xd000 */
#define MMU_ROM_HI ((m_mmu[0]&0x30)==0)	/* rom at 0xc000 */
#define MMU_EXTERNAL_ROM_HI ((m_mmu[0]&0x30)==0x20)
#define MMU_INTERNAL_ROM_HI ((m_mmu[0]&0x30)==0x10)
#define MMU_RAM_HI ((m_mmu[0]&0x30)==0x30)

#define MMU_RAM_ADDR (MMU_RAM_RCR_ADDR|MMU_RAM_CR_ADDR)

static const int c128_mmu_helper[4] =
{0x400, 0x1000, 0x2000, 0x4000};

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

/*
 two cpus share 1 memory system, only 1 cpu can run
 controller is the mmu

 mame has different memory subsystems for each cpu
*/


/* m8502 port
 in c128 mode
 bit 0 color
 bit 1 lores
 bit 2 hires
 3 textmode
 5 graphics (turned on ram at 0x1000 for video chip
*/







void c128_state::nmi()
{
	int cia1irq = mos6526_irq_r(m_cia2);

	if (m_nmilevel != (ioport("SPECIAL")->read() & 0x80) || cia1irq)	/* KEY_RESTORE */
	{
		if (1) // this was never valid, there is no active CPU during a timer firing!  cpu_getactivecpu() == 0)
		{
			/* z80 */
			m_maincpu->set_input_line(INPUT_LINE_NMI, (ioport("SPECIAL")->read() & 0x80) || cia1irq);
		}
		else
		{
			m_subcpu->set_input_line(INPUT_LINE_NMI, (ioport("SPECIAL")->read() & 0x80) || cia1irq);
		}

		m_nmilevel = (ioport("SPECIAL")->read() & 0x80) || cia1irq;
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

READ8_MEMBER( c128_state::cia1_pa_r )
{
	UINT8 cia0portb = mos6526_pb_r(m_cia1, 0);

	return cbm_common_cia0_port_a_r(m_cia1, cia0portb);
}

READ8_MEMBER( c128_state::cia1_pb_r )
{
	UINT8 value = 0xff;
	UINT8 cia0porta = mos6526_pa_r(m_cia1, 0);
	//vic2e_device_interface *intf = dynamic_cast<vic2e_device_interface*>(&m_vic);

	value &= cbm_common_cia0_port_b_r(m_cia1, cia0porta);
/*
	if (!intf->k0_r())
		value &= m_keyline[0];
	if (!intf->k1_r())
		value &= m_keyline[1];
	if (!intf->k2_r())
		value &= m_keyline[2];
*/
	return value;
}

WRITE8_MEMBER( c128_state::cia1_pb_w )
{
	m_vic->lp_w(BIT(data, 4));
}

void c128_state::irq(int level)
{
	if (level != m_old_level)
	{
		DBG_LOG(machine(), 3, "mos6510", ("irq %s\n", level ? "start" : "end"));

		if (0) // && (cpu_getactivecpu() == 0))
		{
			m_maincpu->set_input_line(0, level);
		}
		else
		{
			m_subcpu->set_input_line(M6510_IRQ_LINE, level);
		}

		m_old_level = level;
	}
}

WRITE_LINE_MEMBER( c128_state::cia1_irq_w )
{
	irq(state || m_vicirq);
}

WRITE_LINE_MEMBER( c128_state::vic_interrupt )
{
	if (state  != m_vicirq)
	{
		irq(state || mos6526_irq_r(m_cia1));
		m_vicirq = state;
	}
}

void c128_state::iec_data_out_w()
{
	int data = !m_data_out;

	/* fast serial data */
	if (MMU_FSDIR) data &= m_sp1;

	m_iec->data_w(data);
}

void c128_state::iec_srq_out_w()
{
	int srq = 1;

	/* fast serial clock */
	if (MMU_FSDIR) srq &= m_cnt1;

	m_iec->srq_w(srq);
}

WRITE_LINE_MEMBER( c128_state::cia1_cnt_w )
{
	/* fast clock out */
	m_cnt1 = state;

	iec_srq_out_w();
}

WRITE_LINE_MEMBER( c128_state::cia1_sp_w )
{
	/* fast data out */
	m_sp1 = state;

	iec_data_out_w();
}

const mos6526_interface c128_ntsc_cia0 =
{
	60,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_irq_w),
	DEVCB_NULL,	/* pc_func */
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_cnt_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_sp_w),
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pa_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pb_r),
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pb_w)
};

const mos6526_interface c128_pal_cia0 =
{
	50,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_irq_w),
	DEVCB_NULL,	/* pc_func */
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_cnt_w),
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia1_sp_w),
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pa_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pb_r),
	DEVCB_DRIVER_MEMBER(c128_state, cia1_pb_w)
};

WRITE_LINE_MEMBER( c128_state::iec_srq_w )
{
	if (!MMU_FSDIR)
	{
		mos6526_flag_w(m_cia1, state);
		mos6526_cnt_w(m_cia1, state);
	}
}

WRITE_LINE_MEMBER( c128_state::iec_data_w )
{
	if (!MMU_FSDIR)
	{
		mos6526_sp_w(m_cia1, state);
	}
}

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
READ8_MEMBER( c128_state::cia2_pa_r )
{
	UINT8 value = 0xff;

	if (!m_iec->clk_r())
		value &= ~0x40;

	if (!m_iec->data_r())
		value &= ~0x80;

	return value;
}

WRITE8_MEMBER( c128_state::cia2_pa_w )
{
	static const int helper[4] = {0xc000, 0x8000, 0x4000, 0x0000};

	m_data_out = BIT(data, 5);
	iec_data_out_w();

	m_iec->clk_w(!BIT(data, 4));

	m_iec->atn_w(!BIT(data, 3));

	m_vicaddr = m_memory + helper[data & 0x03];
	m_c128_vicaddr = m_memory + helper[data & 0x03] + m_va1617;
}

WRITE_LINE_MEMBER( c128_state::cia2_irq_w )
{
	nmi();
}

const mos6526_interface c128_ntsc_cia1 =
{
	60,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia2_irq_w),
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c128_state, cia2_pa_r),
	DEVCB_DRIVER_MEMBER(c128_state, cia2_pa_w),
	DEVCB_NULL,
	DEVCB_NULL
};

const mos6526_interface c128_pal_cia1 =
{
	50,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, cia2_irq_w),
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c128_state, cia2_pa_r),
	DEVCB_DRIVER_MEMBER(c128_state, cia2_pa_w),
	DEVCB_NULL,
	DEVCB_NULL
};

/***********************************************

    Memory Handlers

***********************************************/
WRITE8_MEMBER( c128_state::dma8726_port_w )
{
	DBG_LOG(machine(), 1, "dma write", ("%.3x %.2x\n",offset,data));
}

READ8_MEMBER( c128_state::dma8726_port_r )
{
	DBG_LOG(machine(), 1, "dma read", ("%.3x\n",offset));
	return 0xff;
}

WRITE8_MEMBER( c128_state::write_d000 )
{
	UINT8 c64_port6510 = m6510_get_port(m_subcpu);

	if (!m_write_io)
	{
		if (offset + 0xd000 >= m_ram_top)
			m_memory[0xd000 + offset] = data;
		else
			m_ram[0xd000 + offset] = data;
	}
	else
	{
		switch ((offset&0xf00)>>8)
		{
		case 0:case 1: case 2: case 3:
			m_vic->write(space, offset & 0x3f, data);
			break;
		case 4:
			sid6581_w(m_sid, offset & 0x3f, data);
			break;
		case 5:
			mmu8722_port_w(space, offset & 0xff, data);
			break;
		case 6: case 7:
			if (offset & 0x01)
				m_vdc->register_w(space, 0, data);
			else
				m_vdc->address_w(space, 0, data);
			break;
		case 8: case 9: case 0xa: case 0xb:
		    if (m_c64mode)
				m_colorram[(offset & 0x3ff)] = data | 0xf0;
		    else
				m_colorram[(offset & 0x3ff)|((c64_port6510&3)<<10)] = data | 0xf0; // maybe all 8 bit connected!
		    break;
		case 0xc:
			mos6526_w(m_cia1, offset, data);
			break;
		case 0xd:
			mos6526_w(m_cia2, offset, data);
			break;
		case 0xf:
			dma8726_port_w(space, offset&0xff,data);
			break;
		case 0xe:
			DBG_LOG(machine(), 1, "io write", ("%.3x %.2x\n", offset, data));
			break;
		}
	}
}



READ8_MEMBER( c128_state::read_io )
{
	if (offset < 0x400)
		return m_vic->read(space, offset & 0x3f);
	else if (offset < 0x500)
		return sid6581_r(m_sid, offset & 0xff);
	else if (offset < 0x600)
		return mmu8722_port_r(space, offset & 0xff);
	else if (offset < 0x800)
			if (offset & 0x01)
				return m_vdc->register_r(space, 0);
			else
				return m_vdc->status_r(space, 0);
	else if (offset < 0xc00)
		return m_colorram[offset & 0x3ff];
	else if (offset == 0xc00)
		{
			cia_set_port_mask_value(m_cia1, 0, ioport("CTRLSEL")->read() & 0x80 ? c64_keyline[8] : c64_keyline[9] );
			return mos6526_r(m_cia1, offset);
		}
	else if (offset == 0xc01)
		{
			cia_set_port_mask_value(m_cia1, 1, ioport("CTRLSEL")->read() & 0x80 ? c64_keyline[9] : c64_keyline[8] );
			return mos6526_r(m_cia1, offset);
		}
	else if (offset < 0xd00)
		return mos6526_r(m_cia1, offset);
	else if (offset < 0xe00)
		return mos6526_r(m_cia2, offset);
	else if ((offset >= 0xf00) & (offset <= 0xfff))
		return dma8726_port_r(space, offset&0xff);
	DBG_LOG(machine(), 1, "io read", ("%.3x\n", offset));
	return 0xff;
}

void c128_state::bankswitch_64(int reset)
{
	int data, loram, hiram, charen;

	if (!m_c64mode)
		return;

	data = m6510_get_port(m_subcpu) & 0x07;
	if ((m_old_data == data) && (m_old_exrom == m_exrom) && (m_old_game == m_game) && !reset)
		return;

	DBG_LOG(machine(), 1, "bankswitch", ("%d\n", data & 7));
	loram = (data & 1) ? 1 : 0;
	hiram = (data & 2) ? 1 : 0;
	charen = (data & 4) ? 1 : 0;

	if ((!m_game && m_exrom) || (loram && hiram && !m_exrom))
		membank("bank8")->set_base(m_roml);
	else
		membank("bank8")->set_base(m_memory + 0x8000);

	if ((!m_game && m_exrom && hiram) || (!m_exrom))
		membank("bank9")->set_base(m_romh);
	else if (loram && hiram)
		membank("bank9")->set_base(m_basic);
	else
		membank("bank9")->set_base(m_memory + 0xa000);

	if ((!m_game && m_exrom) || (charen && (loram || hiram)))
	{
		m_subcpu->memory().space(AS_PROGRAM)->install_read_handler(0xd000, 0xdfff, read8_delegate(FUNC(c128_state::read_io), this));
		m_write_io = 1;
	}
	else
	{
		m_subcpu->memory().space(AS_PROGRAM)->install_read_bank(0xd000, 0xdfff, "bank5");
		m_write_io = 0;
		if ((!charen && (loram || hiram)))
			membank("bank13")->set_base(m_chargen);
		else
			membank("bank13")->set_base(m_memory + 0xd000);
	}

	if (!m_game && m_exrom)
	{
		membank("bank14")->set_base(m_romh);
		membank("bank15")->set_base(m_romh+0x1f00);
		membank("bank16")->set_base(m_romh+0x1f05);
	}
	else
	{
		if (hiram)
		{
			membank("bank14")->set_base(m_kernal);
			membank("bank15")->set_base(m_kernal+0x1f00);
			membank("bank16")->set_base(m_kernal+0x1f05);
		}
		else
		{
			membank("bank14")->set_base(m_memory + 0xe000);
			membank("bank15")->set_base(m_memory + 0xff00);
			membank("bank16")->set_base(m_memory + 0xff05);
		}
	}
	m_old_data = data;
	m_old_exrom = m_exrom;
	m_old_game =m_game;
}

/* typical z80 configuration
   0x3f 0x3f 0x7f 0x3e 0x7e 0xb0 0x0b 0x00 0x00 0x01 0x00 */
void c128_state::bankswitch_z80()
{
	 m_ram = m_memory + MMU_RAM_ADDR;
	 m_va1617 = MMU_VIC_ADDR;
#if 1
	 membank("bank10")->set_base(m_z80);
	 membank("bank11")->set_base(m_ram + 0x1000);
	 if ( (( (ioport("SPECIAL")->read() & 0x06) == 0x02 ) && (MMU_RAM_ADDR >= 0x40000))
		  || (( (ioport("SPECIAL")->read() & 0x06) == 0x00) && (MMU_RAM_ADDR >= 0x20000)) )
		 m_ram = NULL;
#else
	 if (MMU_BOTTOM)
		 m_ram_bottom = MMU_SIZE;
	 else
		 m_ram_bottom = 0;

	 if (MMU_RAM_ADDR==0) { /* this is used in z80 mode for rom on/off switching !*/
		 membank("bank10")->set_base(m_z80);
		 membank("bank11")->set_base(m_z80 + 0x400);
	 }
	 else
	 {
		 membank("bank10")->set_base((m_ram_bottom > 0 ? m_memory : m_ram));
		 membank("bank11")->set_base((m_ram_bottom > 0x400 ? m_memory : m_ram) + 0x400);
	 }

	 membank("bank1")->set_base((m_ram_bottom > 0 ? m_memory : m_ram));
	 membank("bank2")->set_base((m_ram_bottom > 0x400 ? m_memory : m_ram) + 0x400);

	 membank("bank3")->set_base((m_ram_bottom > 0x1000 ? m_memory : m_ram) + 0x1000);
	 membank("bank4")->set_base((m_ram_bottom > 0x2000 ? m_memory : m_ram) + 0x2000);
	 membank("bank5")->set_base(m_ram + 0x4000);

	 if (MMU_TOP)
		 m_ram_top = 0x10000 - MMU_SIZE;
	 else
		 m_ram_top = 0x10000;

	 if (m_ram_top > 0xc000)
		membank("bank6")->set_base(m_ram + 0xc000);
	 else
		membank("bank6")->set_base(m_memory + 0xc000);

	 if (m_ram_top > 0xe000)
		membank("bank7")->set_base(m_ram + 0xe000);
	 else
		membank("bank7")->set_base(m_memory + 0xd000);

	 if (m_ram_top > 0xf000)
		membank("bank8")->set_base(m_ram + 0xf000);
	 else
		membank("bank8")->set_base(m_memory + 0xe000);

	 if (m_ram_top > 0xff05)
		membank("bank9")->set_base(m_ram + 0xff05);
	 else
		membank("bank9")->set_base(m_memory + 0xff05);

	 if ( (( (ioport("SPECIAL")->read() & 0x06) == 0x02 ) && (MMU_RAM_ADDR >= 0x40000))
		  || (( (ioport("SPECIAL")->read() & 0x06) == 0x00) && (MMU_RAM_ADDR >= 0x20000)) )
		 m_ram = NULL;
#endif
}

void c128_state::bankswitch_128(int reset)
{
	m_c64mode = MMU_64MODE;
	if (m_c64mode)
	{
		/* mmu works also in c64 mode, but this can wait */
		m_ram = m_memory;
		m_va1617 = 0;
		m_ram_bottom = 0;
		m_ram_top = 0x10000;

		membank("bank1")->set_base(m_memory);
		membank("bank2")->set_base(m_memory + 0x100);

		membank("bank3")->set_base(m_memory + 0x200);
		membank("bank4")->set_base(m_memory + 0x400);
		membank("bank5")->set_base(m_memory + 0x1000);
		membank("bank6")->set_base(m_memory + 0x2000);

		membank("bank7")->set_base(m_memory + 0x4000);

		membank("bank12")->set_base(m_memory + 0xc000);

		bankswitch_64(reset);
	}
	else
	{
		m_ram = m_memory + MMU_RAM_ADDR;
		m_va1617 = MMU_VIC_ADDR;
		membank("bank1")->set_base(m_memory + m_mmu_page0);
		membank("bank2")->set_base(m_memory + m_mmu_page1);
		if (MMU_BOTTOM)
			{
				m_ram_bottom = MMU_SIZE;
			}
		else
			m_ram_bottom = 0;
		membank("bank3")->set_base((m_ram_bottom > 0x200 ? m_memory : m_ram) + 0x200);
		membank("bank4")->set_base((m_ram_bottom > 0x400 ? m_memory : m_ram) + 0x400);
		membank("bank5")->set_base((m_ram_bottom > 0x1000 ? m_memory : m_ram) + 0x1000);
		membank("bank6")->set_base((m_ram_bottom > 0x2000 ? m_memory : m_ram) + 0x2000);

		if (MMU_RAM_LO)
		{
			membank("bank7")->set_base(m_ram + 0x4000);
		}
		else
		{
			membank("bank7")->set_base(m_c128_basic);
		}

		if (MMU_RAM_MID)
		{
			membank("bank8")->set_base(m_ram + 0x8000);
			membank("bank9")->set_base(m_ram + 0xa000);
		}
		else if (MMU_ROM_MID)
		{
			membank("bank8")->set_base(m_c128_basic + 0x4000);
			membank("bank9")->set_base(m_c128_basic + 0x6000);
		}
		else if (MMU_INTERNAL_ROM_MID)
		{
			membank("bank8")->set_base(m_internal_function);
			membank("bank9")->set_base(m_internal_function + 0x2000);
		}
		else
		{
			membank("bank8")->set_base(m_external_function);
			membank("bank9")->set_base(m_external_function + 0x2000);
		}

		if (MMU_TOP)
		{
			m_ram_top = 0x10000 - MMU_SIZE;
		}
		else
			m_ram_top = 0x10000;

		m_subcpu->memory().space(AS_PROGRAM)->install_read_handler(0xff00, 0xff04, read8_delegate(FUNC(c128_state::mmu8722_ff00_r), this));

		if (MMU_IO_ON)
		{
			m_write_io = 1;
			m_subcpu->memory().space(AS_PROGRAM)->install_read_handler(0xd000, 0xdfff, read8_delegate(FUNC(c128_state::read_io), this));
		}
		else
		{
			m_write_io = 0;
			m_subcpu->memory().space(AS_PROGRAM)->install_read_bank(0xd000, 0xdfff, "bank13");
		}


		if (MMU_RAM_HI)
		{
			if (m_ram_top > 0xc000)
			{
				membank("bank12")->set_base(m_ram + 0xc000);
			}
			else
			{
				membank("bank12")->set_base(m_memory + 0xc000);
			}
			if (!MMU_IO_ON)
			{
				if (m_ram_top > 0xd000)
				{
					membank("bank13")->set_base(m_ram + 0xd000);
				}
				else
				{
					membank("bank13")->set_base(m_memory + 0xd000);
				}
			}
			if (m_ram_top > 0xe000)
			{
				membank("bank14")->set_base(m_ram + 0xe000);
			}
			else
			{
				membank("bank14")->set_base(m_memory + 0xe000);
			}
			if (m_ram_top > 0xff05)
			{
				membank("bank16")->set_base(m_ram + 0xff05);
			}
			else
			{
				membank("bank16")->set_base(m_memory + 0xff05);
			}
		}
		else if (MMU_ROM_HI)
		{
			membank("bank12")->set_base(m_editor);
			if (!MMU_IO_ON) {
				membank("bank13")->set_base(m_c128_chargen);
			}
			membank("bank14")->set_base(m_c128_kernal);
			membank("bank16")->set_base(m_c128_kernal + 0x1f05);
		}
		else if (MMU_INTERNAL_ROM_HI)
		{
			membank("bank12")->set_base(m_internal_function);
			if (!MMU_IO_ON) {
				membank("bank13")->set_base(m_internal_function + 0x1000);
			}
			membank("bank14")->set_base(m_internal_function + 0x2000);
			membank("bank16")->set_base(m_internal_function + 0x3f05);
		}
		else					   /*if (MMU_EXTERNAL_ROM_HI) */
		{
			membank("bank12")->set_base(m_external_function);
			if (!MMU_IO_ON) {
				membank("bank13")->set_base(m_external_function + 0x1000);
			}
			membank("bank14")->set_base(m_external_function + 0x2000);
			membank("bank16")->set_base(m_external_function + 0x3f05);
		}

		if ( (( (ioport("SPECIAL")->read() & 0x06) == 0x02 ) && (MMU_RAM_ADDR >= 0x40000))
				|| (( (ioport("SPECIAL")->read() & 0x06) == 0x00) && (MMU_RAM_ADDR >= 0x20000)) )
			m_ram = NULL;
	}
}

// 128u4
// FIX-ME: are the bankswitch functions working in the expected way without the memory_set_context?
void c128_state::bankswitch(int reset)
{
	if (m_mmu_cpu != MMU_CPU8502)
	{
		if (!MMU_CPU8502)
		{
//          DBG_LOG(machine, 1, "switching to z80", ("active %d\n",cpu_getactivecpu()) );
//          memory_set_context(machine, 0);
			bankswitch_z80();
//          memory_set_context(machine, 1);
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		}
		else
		{
//          DBG_LOG(machine, 1, "switching to m6502", ("active %d\n",cpu_getactivecpu()) );
//          memory_set_context(machine, 1);
			bankswitch_128(reset);
//          memory_set_context(machine, 0);
			m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

			/* NPW 19-Nov-2005 - In the C128, CPU #0 starts out and hands over
             * control to CPU #1.  CPU #1 seems to execute garbage from 0x0000
             * up to 0x1100, at which point it finally hits some code
             * (presumably set up by CPU #1.) This always worked, but when I
             * changed the m8502 CPU core to use an internal memory map, it
             * started BRK-ing forever when trying to execute 0x0000.
             *
             * I am not sure whether the C128 actually executes this invalid
             * memory or if this is a bug in the C128 driver.  In any case, the
             * driver used to work with this behavior, so I am doing this hack
             * where I set CPU #1's PC to 0x1100 on reset.
             */
			if (cpu_get_reg(m_subcpu, STATE_GENPC) == 0x0000)
				cpu_set_reg(m_subcpu, STATE_GENPC, 0x1100);
		}
		m_mmu_cpu = MMU_CPU8502;
		return;
	}
	if (!MMU_CPU8502)
		bankswitch_z80();
	else
		bankswitch_128(reset);
}

void c128_state::mmu8722_reset()
{
	memset(m_mmu, 0, sizeof (m_mmu));
	m_mmu[5] |= 0x38;
	m_mmu[10] = 1;
	m_mmu_cpu = 0;
	m_mmu_page0 = 0;
	m_mmu_page1 = 0x0100;
	bankswitch(1);
}

WRITE8_MEMBER( c128_state::mmu8722_port_w )
{
	offset &= 0xf;
	switch (offset)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 8:
	case 10:
		m_mmu[offset] = data;
		break;
	case 5:
		m_mmu[offset] = data;
		bankswitch(0);
		iec_srq_out_w();
		iec_data_out_w();
		break;
	case 0:
	case 6:
		m_mmu[offset] = data;
		bankswitch(0);
		break;
	case 7:
		m_mmu[offset] = data;
		m_mmu_page0=MMU_PAGE0;
		break;
	case 9:
		m_mmu[offset] = data;
		m_mmu_page1=MMU_PAGE1;
		bankswitch(0);
		break;
	case 0xb:
		break;
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		break;
	}
}

READ8_MEMBER( c128_state::mmu8722_port_r )
{
	int data;

	offset &= 0x0f;
	switch (offset)
	{
	case 5:
		data = m_mmu[offset] | 6;
		if ( /*disk enable signal */ 0)
			data &= ~8;
		if (!m_game)
			data &= ~0x10;
		if (!m_exrom)
			data &= ~0x20;
		if (ioport("SPECIAL")->read() & 0x10)
			data &= ~0x80;
		break;
	case 0xb:
		/* hinybble number of 64 kb memory blocks */
		if ((ioport("SPECIAL")->read() & 0x06) == 0x02)		// 256KB RAM
			data = 0x4f;
		else if ((ioport("SPECIAL")->read() & 0x06) == 0x04)	//  1MB
			data = 0xf;
		else
			data = 0x2f;
		break;
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		data=0xff;
		break;
	default:
		data=m_mmu[offset];
	}
	return data;
}

WRITE8_MEMBER( c128_state::mmu8722_ff00_w )
{
	switch (offset)
	{
	case 0:
		m_mmu[offset] = data;
		bankswitch(0);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
#if 1
		m_mmu[0]= m_mmu[offset];
#else
		m_mmu[0]|= m_mmu[offset];
#endif
		bankswitch(0);
		break;
	}
}

READ8_MEMBER( c128_state::mmu8722_ff00_r )
{
	return m_mmu[offset];
}

WRITE8_MEMBER( c128_state::write_0000 )
{
	if (m_ram != NULL)
		m_ram[0x0000 + offset] = data;
}

WRITE8_MEMBER( c128_state::write_1000 )
{
	if (m_ram != NULL)
		m_ram[0x1000 + offset] = data;
}

WRITE8_MEMBER( c128_state::write_4000 )
{
	if (m_ram != NULL)
		m_ram[0x4000 + offset] = data;
}

WRITE8_MEMBER( c128_state::write_8000 )
{
	if (m_ram != NULL)
		m_ram[0x8000 + offset] = data;
}

WRITE8_MEMBER( c128_state::write_a000 )
{
	if (m_ram != NULL)
		m_ram[0xa000 + offset] = data;
}

WRITE8_MEMBER( c128_state::write_c000 )
{
	if (m_ram != NULL)
		m_ram[0xc000 + offset] = data;
}

WRITE8_MEMBER( c128_state::write_e000 )
{
	if (offset + 0xe000 >= m_ram_top)
		m_memory[0xe000 + offset] = data;
	else if (m_ram != NULL)
		m_ram[0xe000 + offset] = data;
}

WRITE8_MEMBER( c128_state::write_ff00 )
{
	if (!m_c64mode)
		mmu8722_ff00_w(space, offset, data);
	else if (m_ram!=NULL)
		m_memory[0xff00 + offset] = data;
}

WRITE8_MEMBER( c128_state::write_ff05 )
{
	if (offset + 0xff05 >= m_ram_top)
		m_memory[0xff05 + offset] = data;
	else if (m_ram!=NULL)
		m_ram[0xff05 + offset] = data;
}

/*
 * only 14 address lines
 * a15 and a14 portlines
 * 0x1000-0x1fff, 0x9000-0x9fff char rom
 */
READ8_MEMBER( c128_state::vic_dma_read )
{
	UINT8 c64_port6510 = m6510_get_port(m_subcpu);

	/* main memory configuration to include */
	if (m_c64mode)
	{
		if (!m_game && m_exrom)
		{
			if (offset < 0x3000)
				return m_memory[offset];
			return m_romh[offset & 0x1fff];
		}
		if (((m_vicaddr - m_memory + offset) & 0x7000) == 0x1000)
			return m_chargen[offset & 0xfff];
		return m_vicaddr[offset];
	}
	if (!(c64_port6510 & 4) && (((m_c128_vicaddr - m_memory + offset) & 0x7000) == 0x1000))
		return m_c128_chargen[offset & 0xfff];
	return m_c128_vicaddr[offset];
}

READ8_MEMBER( c128_state::vic_dma_read_color )
{
	UINT8 c64_port6510 = m6510_get_port(m_subcpu);

	if (m_c64mode)
		return m_colorram[offset & 0x3ff] & 0xf;
	else
		return m_colorram[(offset & 0x3ff)|((c64_port6510 & 0x3) << 10)] & 0xf;
}

/* 2008-09-01
    We need here the m6510 port handlers from c64, otherwise c128_common_driver_init
    seems unable to use correctly the timer.
    This will be probably fixed in a future clean up.
*/


WRITE8_MEMBER( c128_state::cpu_w )
{
	m_cassette->write(BIT(data, 3));

	m_cassette->motor_w(BIT(data, 5));

	bankswitch_64(0);

	m_memory[0x000] = m_subcpu->memory().space(AS_PROGRAM)->read_byte(0);
	m_memory[0x001] = m_subcpu->memory().space(AS_PROGRAM)->read_byte(1);

}

READ8_MEMBER( c128_state::cpu_r)
{
	UINT8 data = 0x07;

	if (m_cassette->sense_r())
		data &= ~0x10;
	else
		data |=  0x10;

	if (ioport("SPECIAL")->read() & 0x20)		/* Check Caps Lock */
		data &= ~0x40;
	else
		data |=  0x40;

	return data;
}

DRIVER_INIT_MEMBER(c128_state,c128)
{
	UINT8 *ram = memregion(Z80A_TAG)->base();

	m_memory = ram;

	m_c128_basic = ram + 0x100000;
	m_basic = ram + 0x108000;
	m_kernal = ram + 0x10a000;
	m_editor = ram + 0x10c000;
	m_z80 = ram + 0x10d000;
	m_c128_kernal = ram + 0x10e000;
	m_internal_function = ram + 0x110000;
	m_external_function = ram + 0x118000;
	m_chargen = ram + 0x120000;
	m_c128_chargen = ram + 0x121000;
	m_colorram = ram + 0x122000;
	m_vdcram = ram + 0x122800;
	m_c64_roml = auto_alloc_array(machine(), UINT8, 0x2000);
	m_c64_romh = auto_alloc_array(machine(), UINT8, 0x2000);

	m_game = 1;
	m_exrom = 1;
	m_pal = 0;
	m_c64mode = 0;
	m_vicirq = 0;

	m_cnt1 = 1;
	m_sp1 = 1;
	cbm_common_init();
	m_keyline[0] = m_keyline[1] = m_keyline[2] = 0xff;
}

DRIVER_INIT_MEMBER(c128_state,c128pal)
{
	DRIVER_INIT_CALL( c128 );

	m_pal = 1;
}

DRIVER_INIT_MEMBER(c128_state,c128d)
{
	DRIVER_INIT_CALL( c128 );
}

DRIVER_INIT_MEMBER(c128_state,c128dpal)
{
	DRIVER_INIT_CALL( c128pal );
}

DRIVER_INIT_MEMBER(c128_state,c128dcr)
{
	DRIVER_INIT_CALL( c128 );
}

DRIVER_INIT_MEMBER(c128_state,c128dcrp)
{
	DRIVER_INIT_CALL( c128pal );
}

DRIVER_INIT_MEMBER(c128_state,c128d81)
{
	DRIVER_INIT_CALL( c128 );
}

void c128_state::machine_start()
{
// This was in MACHINE_START( c64 ), but never called
// TO DO: find its correct use, when fixing c64 mode
	if (m_c64mode)
		bankswitch_64(1);
}

void c128_state::machine_reset()
{
	m_c128_vicaddr = m_vicaddr = m_memory;
	m_c64mode = 0;
	mmu8722_reset();
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}


INTERRUPT_GEN( c128_frame_interrupt )
{
	c128_state *state = device->machine().driver_data<c128_state>();
	static const char *const c128ports[] = { "KP0", "KP1", "KP2" };
	int i, value;
	//device_t *vic2e = device->machine().device("vic2e");
	//device_t *vdc8563 = device->machine().device("vdc8563");

	state->nmi();

	/* common keys input ports */
	cbm_common_interrupt(device);

	/* Fix Me! Currently, neither left Shift nor Shift Lock work in c128, but reading the correspondent input produces a bug!
    Hence, we overwrite the actual reading as it never happens */
	if ((device->machine().root_device().ioport("SPECIAL")->read() & 0x40))	//
		c64_keyline[1] |= 0x80;

	/* c128 specific: keypad input ports */
	for (i = 0; i < 3; i++)
	{
		value = 0xff;
		value &= ~device->machine().root_device().ioport(c128ports[i])->read();
		state->m_keyline[i] = value;
	}
}
