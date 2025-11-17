// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A570

    DMAC based CD-ROM controller for the A500

    Notes:
    - Essentially turns the A500 into a CDTV
    - A prototype version is called A690
    - ROM label for the A690: "391298-01 V1.0 Copyright ©1991 CBM C480"
    - The ROM P/N 391298-01 seems to have been used for multiple versions
    - There are expansion slots for a 2 MB RAM expansion and a SCSI module
    - Uses the CR-512-B drive from MKE (Matsushita Kotobuki Electronics)
    - An FPGA is used in place of many discrete logic chips of the CDTV

    TODO:
    - Volume control (LC7883M)

***************************************************************************/

#include "emu.h"
#include "a570.h"
#include "speaker.h"

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_CPUSLOT_A570, bus::amiga::cpuslot::a570_device, "amiga_a570", "Commodore A570")

namespace bus::amiga::cpuslot {

a570_device::a570_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CPUSLOT_A570, tag, owner, clock),
	device_amiga_cpuslot_interface(mconfig, *this),
	m_irq(*this, "irq"),
	m_dmac(*this, "dmac"),
	m_tpi(*this, "tpi"),
	m_drive(*this, "drive"),
	m_config(*this, "config")
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void a570_device::map(address_map &map)
{
	map(0xdc8000, 0xdc87ff).mirror(0x07800).rw("nvram0", FUNC(at28c16_device::read), FUNC(at28c16_device::write)).umask16(0x00ff);
	map(0xdc8000, 0xdc87ff).mirror(0x07800).rw("nvram1", FUNC(at28c16_device::read), FUNC(at28c16_device::write)).umask16(0xff00);
	map(0xf00000, 0xf3ffff).mirror(0x40000).rom().region("bootrom", 0);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( a570 )
	PORT_START("config")
	PORT_CONFNAME(0x01, 0x00, "2 MB RAM Expansion")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x01, "Enabled")
INPUT_PORTS_END

ioport_constructor a570_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a570 );
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( firmware )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	// COMMODORE-AMIGA  391298-01  ©1992 V2.30
	ROM_LOAD("391298-01_v230.u20", 0x00000, 0x40000, CRC(30b54232) SHA1(ed7e461d1fff3cda321631ae42b80e3cd4fa5ebb))
ROM_END

const tiny_rom_entry *a570_device::device_rom_region() const
{
	return ROM_NAME( firmware );
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void a570_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irq);
	m_irq->output_handler().set([this] (int state) { m_host->int2_w(state); });

	AMIGA_DMAC_REV2(config, m_dmac, 28.37516_MHz_XTAL / 4); // 7M
	m_dmac->cfgout_cb().set([this] (int state) { m_host->cfgout_w(state); });
	m_dmac->int_cb().set(m_irq, FUNC(input_merger_any_high_device::in_w<0>));
	m_dmac->csx0_read_cb().set(m_drive, FUNC(cr511b_device::read));
	m_dmac->csx0_write_cb().set(m_drive, FUNC(cr511b_device::write));
	m_dmac->csx0_a4_read_cb().set(m_tpi, FUNC(tpi6525_device::read));
	m_dmac->csx0_a4_write_cb().set(m_tpi, FUNC(tpi6525_device::write));
	m_dmac->xdack_read_cb().set(m_drive, FUNC(cr511b_device::read));

	AT28C16(config, "nvram0", 0);
	AT28C16(config, "nvram1", 0);

	TPI6525(config, m_tpi, 0);
	m_tpi->out_irq_cb().set(m_irq, FUNC(input_merger_any_high_device::in_w<1>));
	m_tpi->out_pb_cb().set(FUNC(a570_device::tpi_portb_w));

	CR511B(config, m_drive, 0);
	m_drive->add_route(0, "speaker", 1.0, 0);
	m_drive->add_route(1, "speaker", 1.0, 1);
	m_drive->scor_cb().set(m_tpi, FUNC(tpi6525_device::i1_w)).invert();
	m_drive->stch_cb().set(m_tpi, FUNC(tpi6525_device::i2_w)).invert();
	m_drive->sten_cb().set(m_tpi, FUNC(tpi6525_device::i3_w));
	m_drive->sten_cb().append(FUNC(a570_device::sten_w));
	m_drive->drq_cb().set(m_tpi, FUNC(tpi6525_device::i4_w));
	m_drive->drq_cb().append(FUNC(a570_device::drq_w));

	SPEAKER(config, "speaker", 2).front();

	// TODO: Add stereo input for Amiga sound
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void a570_device::device_start()
{
	m_ram = make_unique_clear<uint16_t[]>(0x200000/2);

	m_dmac->set_address_space(&m_host->space());
	m_dmac->set_ram(m_ram.get());

	m_host->space().install_device(0x000000, 0xffffff, *this, &a570_device::map);

	// register for save states
	save_pointer(NAME(m_ram), 0x200000/2);
	save_item(NAME(m_sten));
}

void a570_device::sten_w(int state)
{
	m_sten = bool(state);
}

void a570_device::drq_w(int state)
{
	if (m_sten)
		m_dmac->xdreq_w(state);
}

void a570_device::tpi_portb_w(uint8_t data)
{
	// 7-------  daclch (lc7883m)
	// -6------  dacst (lc7883m)
	// --5-----  dacatt (lc7883m)
	// ---4----  weprom
	// ----3---  dten (drive)
	// -----2--  xaen (drive)
	// ------1-  enable (drive)
	// -------0  cmd (drive)

	m_drive->enable_w(BIT(data, 1));
	m_drive->cmd_w(BIT(data, 0));
}

// the dmac handles this
void a570_device::cfgin_w(int state) { m_dmac->configin_w(state); }

void a570_device::rst_w(int state)
{
	// call rst first as it will unmap memory
	m_dmac->rst_w(state);

	if (state == 0)
		m_dmac->ramsz_w(m_config->read() ? 3 : 0);
}

} // namespace bus::amiga::cpuslot
