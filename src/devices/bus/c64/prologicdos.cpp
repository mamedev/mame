// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    REX Data Technology ProLogic-DOS Classic emulation

**********************************************************************/

#include "emu.h"
#include "prologicdos.h"

#include "geocable.h"

#include <algorithm>
#include <iterator>



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_PROLOGIC_DOS_CLASSIC, c64_prologic_dos_classic_device, "c64_pdc", "C64 ProLogic-DOS Classic cartridge")


//-------------------------------------------------
//  ROM( c64_pdc )
//-------------------------------------------------

ROM_START( c64_pdc )
	ROM_REGION( 0x2000, "ic4", 0 )
	ROM_LOAD( "180391_9612.0.ic4", 0x0000, 0x2000, CRC(fa554279) SHA1(a81853c2c0691b81e479214662b3ad39c3f8cef6) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_prologic_dos_classic_device::device_rom_region() const
{
	return ROM_NAME( c64_pdc );
}


//-------------------------------------------------
//  prologicdos_user_port_cards -
//-------------------------------------------------

static void prologicdos_user_port_cards(device_slot_interface &device)
{
	device.option_add("geocable", C64_GEOCABLE);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_prologic_dos_classic_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set(FUNC(c64_prologic_dos_classic_device::pia_irq_w));

	PIA6821(config, m_pia);
	m_pia->irqa_handler().set("irq", FUNC(input_merger_device::in_w<0>));
	m_pia->irqb_handler().set("irq", FUNC(input_merger_device::in_w<1>));
	m_pia->readpb_handler().set(FUNC(c64_prologic_dos_classic_device::pia_pb_r));
	m_pia->writepb_handler().set(FUNC(c64_prologic_dos_classic_device::pia_pb_w));
	m_pia->readpa_handler().set(FUNC(c64_prologic_dos_classic_device::pia_pa_r));
	m_pia->writepa_handler().set(FUNC(c64_prologic_dos_classic_device::pia_pa_w));
	m_pia->ca2_handler().set(FUNC(c64_prologic_dos_classic_device::pia_ca2_w));
	m_pia->cb2_handler().set(m_user, FUNC(pet_user_port_device::write_m));
	m_pia->cb2_handler().append(m_user, FUNC(pet_user_port_device::write_8));

	INPUT_MERGER_ALL_HIGH(config, m_pis_in);
	m_pis_in->output_handler().set(m_pia, FUNC(pia6821_device::ca1_w));
	m_pis_in->output_handler().append(m_pia, FUNC(pia6821_device::cb1_w));

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->set_passthrough();

	PET_USER_PORT(config, m_user, prologicdos_user_port_cards, nullptr);
	m_user->pb_handler().set(m_pis_in, FUNC(input_merger_device::in_w<2>));
	m_user->pc_handler().set(FUNC(c64_prologic_dos_classic_device::pb_w<0>));
	m_user->pd_handler().set(FUNC(c64_prologic_dos_classic_device::pb_w<1>));
	m_user->pe_handler().set(FUNC(c64_prologic_dos_classic_device::pb_w<2>));
	m_user->pf_handler().set(FUNC(c64_prologic_dos_classic_device::pb_w<3>));
	m_user->ph_handler().set(FUNC(c64_prologic_dos_classic_device::pb_w<4>));
	m_user->pj_handler().set(FUNC(c64_prologic_dos_classic_device::pb_w<5>));
	m_user->pk_handler().set(FUNC(c64_prologic_dos_classic_device::pb_w<6>));
	m_user->pl_handler().set(FUNC(c64_prologic_dos_classic_device::pb_w<7>));
}


//-------------------------------------------------
//  INPUT_PORTS( c64_pdc )
//-------------------------------------------------

static INPUT_PORTS_START( c64_pdc )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w))

	PORT_START("DIP")
	PORT_DIPNAME(0x01, 0x00, "ProLogic 1541") PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(0x01, DEF_STR( Off ))
	PORT_DIPSETTING(0x00, DEF_STR( On ))
	PORT_DIPNAME(0x02, 0x02, "ProLogic C-64") PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(0x00, DEF_STR( Off ))
	PORT_DIPSETTING(0x02, DEF_STR( On ))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_prologic_dos_classic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_pdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_prologic_dos_classic_device - constructor
//-------------------------------------------------

c64_prologic_dos_classic_device::c64_prologic_dos_classic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_PROLOGIC_DOS_CLASSIC, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_pia(*this, "ic5"),
	m_pis_in(*this, "pis_in"),
	m_exp(*this, "exp"),
	m_user(*this, "user"),
	m_romh(*this, "ic4"),
	m_dip(*this, "DIP"),
	m_drive{ { mconfig, *this, 0 }, { mconfig, *this, 1 } },
	m_user_pb(0xff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_prologic_dos_classic_device::device_start()
{
	std::fill(std::begin(m_drive_data), std::end(m_drive_data), 0xff);

	int drive = 0;

	for (device_t &device : device_enumerator(machine().root_device()))
	{
		if (drive < 2 && device.interface(m_drive[drive].m_other))
		{
			m_drive[drive].m_other->m_other = &m_drive[drive];
			drive++;
		}
	}

	save_item(NAME(m_drive_data));
	save_item(NAME(m_dip1));
	save_item(NAME(m_dip2));
	save_item(NAME(m_user_pb));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_prologic_dos_classic_device::device_reset()
{
	u8 dip = m_dip->read();
	m_dip1 = BIT(dip, 0);
	m_dip2 = BIT(dip, 1);

	for (auto &drive : m_drive)
		if (drive.m_other)
			drive.m_other->parallel_select_w(m_dip1);
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_prologic_dos_classic_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!romh)
	{
		data = m_romh->base()[offset & 0x1fff];
	}

	if (!io1 && ((offset & 0x5c) == 0x5c))
	{
		data = m_pia->read(offset & 0x03);
	}

	return m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_prologic_dos_classic_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1 && ((offset & 0x5c) == 0x5c))
	{
		m_pia->write(offset & 0x03, data);
	}

	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_prologic_dos_classic_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	int e_game = m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
	int game = !(sphi2 && ba && rw && m_slot->hiram() && m_dip2 && ((offset & 0xe000) == 0xe000));
	return e_game && game;
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_prologic_dos_classic_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}


//-------------------------------------------------
//  c64_ba_w - BA write
//-------------------------------------------------

void c64_prologic_dos_classic_device::c64_ba_w(int state)
{
	m_exp->ba_w(state);
}


//-------------------------------------------------
//  c64_phi2_w - phi2 write
//-------------------------------------------------

void c64_prologic_dos_classic_device::c64_phi2_w(int state)
{
	m_exp->phi2_w(state);
}


//-------------------------------------------------
//  c64_irq_w - IRQ write
//-------------------------------------------------

void c64_prologic_dos_classic_device::c64_irq_w(int state)
{
	m_exp->host_irq_w(state);
}


//-------------------------------------------------
//  c64_nmi_w - NMI write
//-------------------------------------------------

void c64_prologic_dos_classic_device::c64_nmi_w(int state)
{
	m_exp->host_nmi_w(state);
}


//-------------------------------------------------
//  pia_pb_w -
//-------------------------------------------------

void c64_prologic_dos_classic_device::pia_pb_w(uint8_t data)
{
	m_user->write_c(BIT(data, 0));
	m_user->write_d(BIT(data, 1));
	m_user->write_e(BIT(data, 2));
	m_user->write_f(BIT(data, 3));
	m_user->write_h(BIT(data, 4));
	m_user->write_j(BIT(data, 5));
	m_user->write_k(BIT(data, 6));
	m_user->write_l(BIT(data, 7));
}


//-------------------------------------------------
//  pia_pa_w -
//-------------------------------------------------

void c64_prologic_dos_classic_device::pia_pa_w(uint8_t data)
{
	for (auto &drive : m_drive)
		if (drive.m_other)
			drive.m_other->parallel_data_w(data);
}


//-------------------------------------------------
//  pia_ca2_w -
//-------------------------------------------------

void c64_prologic_dos_classic_device::pia_ca2_w(int state)
{
	for (auto &drive : m_drive)
		if (drive.m_other)
			drive.m_other->parallel_strobe_w(state);
}


//-------------------------------------------------
//  pis_in_w -
//-------------------------------------------------

void c64_prologic_dos_classic_device::pis_in_w(int drive, int state)
{
	if (drive)
		m_pis_in->in_w<1>(state);
	else
		m_pis_in->in_w<0>(state);
}


//-------------------------------------------------
//  drive_port::parallel_data_w -
//-------------------------------------------------

void c64_prologic_dos_classic_device::drive_port::parallel_data_w(uint8_t data)
{
	m_host.machine().scheduler().synchronize(timer_expired_delegate(FUNC(c64_prologic_dos_classic_device::drive_data_sync), &m_host), (m_index << 8) | data);
}


//-------------------------------------------------
//  drive_data_sync -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(c64_prologic_dos_classic_device::drive_data_sync)
{
	m_drive_data[BIT(param, 8)] = u8(param);
}


//-------------------------------------------------
//  drive_port::parallel_strobe_w -
//-------------------------------------------------

void c64_prologic_dos_classic_device::drive_port::parallel_strobe_w(int state)
{
	m_host.pis_in_w(m_index, state);
}
