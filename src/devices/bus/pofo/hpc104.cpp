// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-104 Memory Expander Plus emulation

**********************************************************************/

#include "emu.h"
#include "hpc104.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(POFO_HPC104,   pofo_hpc104_device,   "pofo_hpc104",   "Atari Portfolio HPC-104")
DEFINE_DEVICE_TYPE(POFO_HPC104_2, pofo_hpc104_2_device, "pofo_hpc104_2", "Atari Portfolio HPC-104 (Unit 2)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pofo_hpc104_device::device_add_mconfig(machine_config &config)
{
	PORTFOLIO_MEMORY_CARD_SLOT(config, m_ccm, portfolio_memory_cards, nullptr);

	PORTFOLIO_EXPANSION_SLOT(config, m_exp, XTAL(4'915'200), portfolio_expansion_cards, nullptr);
	m_exp->eint_wr_cb().set(DEVICE_SELF_OWNER, FUNC(portfolio_expansion_slot_device::eint_w));
	m_exp->wake_wr_cb().set(DEVICE_SELF_OWNER, FUNC(portfolio_expansion_slot_device::wake_w));
}


//-------------------------------------------------
//  INPUT_PORTS( hpc104 )
//-------------------------------------------------

static INPUT_PORTS_START( hpc104 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "Unit Number" )
	PORT_DIPSETTING(    0x00, "1 (0x1F000)" )
	PORT_DIPSETTING(    0x01, "2 (0x5F000)" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pofo_hpc104_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( hpc104 );
}


//-------------------------------------------------
//  INPUT_PORTS( hpc104_2 )
//-------------------------------------------------

static INPUT_PORTS_START( hpc104_2 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Unit Number" )
	PORT_DIPSETTING(    0x00, "1 (0x1F000)" )
	PORT_DIPSETTING(    0x01, "2 (0x5F000)" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pofo_hpc104_2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( hpc104_2 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pofo_hpc104_device - constructor
//-------------------------------------------------

pofo_hpc104_device::pofo_hpc104_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_portfolio_expansion_slot_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_ccm(*this, PORTFOLIO_MEMORY_CARD_SLOT_B_TAG),
	m_exp(*this, "exp"),
	m_nvram(*this, "nvram", 0x40000, ENDIANNESS_LITTLE),
	m_io_sw1(*this, "SW1")
{
}

pofo_hpc104_device::pofo_hpc104_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pofo_hpc104_device(mconfig, POFO_HPC104, tag, owner, clock)
{
}


//-------------------------------------------------
//  pofo_hpc104_2_device - constructor
//-------------------------------------------------

pofo_hpc104_2_device::pofo_hpc104_2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pofo_hpc104_device(mconfig, POFO_HPC104_2, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_resolve_objects - forward the inherited
//  address spaces down into the nested CCM/EXP slots
//-------------------------------------------------

void pofo_hpc104_device::device_resolve_objects()
{
	m_ccm->set_memspace(m_slot->memspace());
	m_exp->set_memspace(m_slot->memspace());
	m_exp->set_iospace(m_slot->iospace());
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pofo_hpc104_device::device_start()
{
	m_slot->iospace().install_write_tap(0x807c, 0x807c, "hpc104_ncc1",
		[this] (offs_t offset, u8 &data, u8)
		{
			m_ncc1_out = BIT(data, 0);
			if (LOG) osd_printf_info("%s %s [%s] NCC1 out %u\n", machine().time().as_string(), machine().describe_context(), tag(), m_ncc1_out);
			update_ccm_b_tap();
		});
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pofo_hpc104_device::device_reset()
{
	m_sw1 = BIT(m_io_sw1->read(), 0);

	const offs_t base = m_sw1 ? 0x5f000 : 0x1f000;
	m_slot->memspace().install_ram(base, base + 0x3ffff, m_nvram);

	m_ncc1_out = false;
	m_upstream_ccm_b = false;
	update_ccm_b_tap();
}


void pofo_hpc104_device::nvram_default()
{
}


bool pofo_hpc104_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_nvram, 0x40000);
	return !err && (actual == 0x40000);
}


bool pofo_hpc104_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_nvram, 0x40000);
	return !err;
}


void pofo_hpc104_device::ncc1_w(int state)
{
	if (LOG) osd_printf_info("%s %s [%s] ncc1_w %d\n", machine().time().as_string(), machine().describe_context(), tag(), state);

	m_upstream_ccm_b = state;
	update_ccm_b_tap();
}


void pofo_hpc104_device::update_ccm_b_tap()
{
	if (LOG) osd_printf_info("%s %s [%s] update_ccm_b_tap ncc1_out=%d upstream_ccm_b=%d -> select=%d\n", machine().time().as_string(), machine().describe_context(), tag(), m_ncc1_out, m_upstream_ccm_b, m_ncc1_out && m_upstream_ccm_b);

	m_ccm->ncc2_w(m_ncc1_out && m_upstream_ccm_b);
}
