// license:BSD-3-Clause
// copyright-holders: kmg, Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bit Corp PCBs


 Here we emulate the following Bit Corp PCBs

 * Bit Corp 3150 [mapper 360]
 * Bit Corp 4602 [mapper 357]

 ***********************************************************************************************************/


#include "emu.h"
#include "bitcorp.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_BC3150, nes_bc3150_device, "nes_bc3150", "NES Cart Bit Corp 3150 PCB")
DEFINE_DEVICE_TYPE(NES_BC4602, nes_bc4602_device, "nes_bc4602", "NES Cart Bit Corp 4602 PCB")


nes_bc3150_device::nes_bc3150_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BC3150, tag, owner, clock)
	, m_dsw(*this, "DIPSW")
{
}

nes_bc4602_device::nes_bc4602_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BC4602, tag, owner, clock)
	, m_dsw(*this, "DIPSW")
	, m_dipsetting(0)
	, m_irq_count(0)
	, m_irq_enable(0)
	, irq_timer(nullptr)
{
}



//-------------------------------------------------
//  Dipswitch
//-------------------------------------------------

static INPUT_PORTS_START( bc3150_dsw )
	PORT_START("DIPSW")
	PORT_DIPNAME( 0x1f, 0x00, "Game" ) PORT_DIPLOCATION("SW:!1,!2,!3,!4,!5") PORT_CHANGED_MEMBER(DEVICE_SELF, nes_bc3150_device, dsw_changed, 0)
	PORT_DIPSETTING( 0x00, "Super Mary (Super Mario Bros.)" )
	PORT_DIPSETTING( 0x01, "Super Mary (Super Mario Bros.)" )
	PORT_DIPSETTING( 0x02, "Star War (Star Force)" )
	PORT_DIPSETTING( 0x03, "(F-1 Race)" )
	PORT_DIPSETTING( 0x04, "Lode Runner" )
	PORT_DIPSETTING( 0x05, "Raid on Bungeling Bay" )
	PORT_DIPSETTING( 0x06, "4-nin Uchi Mahjong" )
	PORT_DIPSETTING( 0x07, "Kinnikuman" )
	PORT_DIPSETTING( 0x08, "Choujikuu Yousai Macross" )
	PORT_DIPSETTING( 0x09, "Chack'n Pop" )
	PORT_DIPSETTING( 0x0a, "Ikki" )
	PORT_DIPSETTING( 0x0b, "Championship Lode Runner" )
	PORT_DIPSETTING( 0x0c, "Lunar Ball" )
	PORT_DIPSETTING( 0x0d, "Duck Hunt" )
	PORT_DIPSETTING( 0x0e, "Wild Gunman" )
	PORT_DIPSETTING( 0x0f, "Urban Champion" )
	PORT_DIPSETTING( 0x10, "(Ice Climber)" )
	PORT_DIPSETTING( 0x11, "Battle City" )
	PORT_DIPSETTING( 0x12, "Goblin (Pac-Man)" )
	PORT_DIPSETTING( 0x13, "Abeille (Galaxian)" )
	PORT_DIPSETTING( 0x14, "Popeye" )
	PORT_DIPSETTING( 0x15, "Devil World" )
	PORT_DIPSETTING( 0x16, "Exerion" )
	PORT_DIPSETTING( 0x17, "Donkey Kong" )
	PORT_DIPSETTING( 0x18, "Donkey Kong Jr." )
	PORT_DIPSETTING( 0x19, "Tennis" )
	PORT_DIPSETTING( 0x1a, "Field Combat" )
	PORT_DIPSETTING( 0x1b, "Golf" )
	PORT_DIPSETTING( 0x1c, "(Space Invaders)" )
	PORT_DIPSETTING( 0x1d, "Front Line" )
	PORT_DIPSETTING( 0x1e, "Zippy Race" )
	PORT_DIPSETTING( 0x1f, "Super Arabian" )
INPUT_PORTS_END

static INPUT_PORTS_START( bc4602_dsw )
	PORT_START("DIPSW")
	PORT_DIPNAME( 0x03, 0x00, "Game" ) PORT_DIPLOCATION("SW:!1,!2") PORT_CHANGED_MEMBER(DEVICE_SELF, nes_bc4602_device, dsw_changed, 0)
	PORT_DIPSETTING( 0x00, "Mr. Mary 2 (Super Mario Bros. 2 FDS)" )
	PORT_DIPSETTING( 0x01, "Brave Soldier (Argos no Senshi)" )
	PORT_DIPSETTING( 0x02, "Phoenix (Hi no Tori Hououhen)" )
	PORT_DIPSETTING( 0x03, "Booby Kids" )
INPUT_PORTS_END



//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_bc3150_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bc3150_dsw );
}

ioport_constructor nes_bc4602_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bc4602_dsw );
}



//--------------------------------------------------
//  DIP switch handlers
//--------------------------------------------------

INPUT_CHANGED_MEMBER( nes_bc3150_device::dsw_changed )
{
	update_banks();
}

INPUT_CHANGED_MEMBER( nes_bc4602_device::dsw_changed )
{
	m_dipsetting = newval;
	update_banks();
}




void nes_bc3150_device::pcb_reset()
{
	update_banks();
}

void nes_bc4602_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_dipsetting));
	save_item(NAME(m_reg));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_bc4602_device::pcb_reset()
{
	chr8(0, CHRRAM);

	m_dipsetting = m_dsw->read();
	m_reg[0] = m_reg[1] = 0;    // used by Mr Mary 2
	m_reg[2] = 0;               // used by UNROM games
	update_banks();

	m_irq_enable = 0;
	m_irq_count = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bit Corp 3150 board

 Games: 31 in 1

 This board contains NROM-128 and NROM-256 (SMB only)
 games with banks directly selected by 5 DIP switches
 that protrude through the front of the cartridge.

 NES 2.0: mapper 360

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bc3150_device::update_banks()
{
	u8 dsw = m_dsw->read();

	if (dsw > 1)
	{
		prg16_89ab(dsw);
		prg16_cdef(dsw);
	}
	else
		prg32(0);

	chr8(dsw, CHRROM);
	set_nt_mirroring(BIT(dsw, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Bit Corp 4602 board

 Games: 4 in 1

 Besides having the 2 DIP switches this board is basically
 a combination of the YUNG-08 SMB2 bootleg board and UNROM
 for the other 3 games. The YUNG-08 half is a slightly
 incompatible hack of the original bootleg.

 NES 2.0: mapper 357

 In MAME: Supported.

 TODO: Verify how DIP switches and register latches actually
 work in conjunction.

 -------------------------------------------------*/

void nes_bc4602_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count = (m_irq_count + 1) & 0x0fff;
			if (!m_irq_count)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_bc4602_device::update_banks()
{
	if (m_dipsetting)      // UNROM games
	{
		prg16_89ab(m_dipsetting << 3 | m_reg[2]);
		prg16_cdef(m_dipsetting << 3 | 0x07);
	}
	else                   // Mr Mary 2
	{
		prg8_89(1);    // fixed bank
		prg8_ab(0);    // fixed bank
		prg8_cd(m_reg[0] & 1 ? 3 : 4 + ((m_reg[0] & 0x06) >> 1));
		prg8_ef(8 + (!m_reg[1] << 1));
	}

	set_nt_mirroring(m_dipsetting == 3 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

u8 nes_bc4602_device::read_l(offs_t offset)
{
	LOG_MMC(("bc4602 read_l, offset: %04x, data: %02x\n", offset));

	offset += 0x100;
	if (offset >= 0x1000)                      // Mr Mary 2, 0x5000-0x5fff
		return m_prg[0x10000 + (offset & 0x0fff)];    // fixed 4K bank
	else if ((offset & 0x11ff) == 0x0122)      // Mr Mary 2, 0x4122
		return m_irq_enable;
	else                                       // all other 0x4...
		return get_open_bus();
}

u8 nes_bc4602_device::read_m(offs_t offset)
{
	LOG_MMC(("bc4602 read_m, offset: %04x, data: %02x\n", offset));
	return m_prg[(!m_reg[1] << 1) * 0x2000 + offset];
}

void nes_bc4602_device::write_45(offs_t offset, u8 data)
{
	switch (offset & 0x51ff)
	{
		case 0x4022:
			m_reg[0] = data;
			update_banks();
			break;
		case 0x4120:
			m_reg[1] = BIT(data, 0);
			update_banks();
			break;
		case 0x4122:
			m_irq_enable = BIT(data, 0);
			if (!m_irq_enable)
			{
				set_irq_line(CLEAR_LINE);
				m_irq_count = 0;
			}
			break;
	}
}

void nes_bc4602_device::write_ex(offs_t offset, u8 data)
{
	LOG_MMC(("bc4602 write_l, offset: %04x, data: %02x\n", offset, data));
	write_45(offset + 0x4020, data);
}

void nes_bc4602_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("bc4602 write_l, offset: %04x, data: %02x\n", offset, data));
	write_45(offset + 0x4100, data);
}

void nes_bc4602_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bc4602 write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg[2] = data & 0x07;
	update_banks();
}
