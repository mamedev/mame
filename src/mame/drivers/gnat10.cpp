// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for CP/M system by GNAT Computers, Inc.

    For more background on this company, visit the following link:
    https://classictech.wordpress.com/computer-companies/gnat-computers-san-diego-calif/

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/com8116.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"

class gnat10_state : public driver_device
{
public:
	gnat10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_brg(*this, "brg")
		, m_monitor(*this, "monitor")
		, m_ram(*this, "ram")
		, m_ram_on(false)
	{
	}

	void gnat10(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u8 ram_r(offs_t offset);
	u8 floppy_status_r();
	void floppy_latch_w(u8 data);
	void baud0_w(u8 data);
	void baud1_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<com8116_device> m_brg;
	required_region_ptr<u8> m_monitor;
	required_shared_ptr<u8> m_ram;

	bool m_ram_on;
};

void gnat10_state::machine_start()
{
	save_item(NAME(m_ram_on));
}

void gnat10_state::machine_reset()
{
	m_ram_on = false;
	floppy_latch_w(0);
}


u8 gnat10_state::ram_r(offs_t offset)
{
	if (m_ram_on)
		return m_ram[offset];
	else
		return m_monitor[offset & 0x07ff];
}

u8 gnat10_state::floppy_status_r()
{
	return m_fdc->drq_r() << 7 | m_fdc->intrq_r();
}

void gnat10_state::floppy_latch_w(u8 data)
{
	// D0 = DS1, D1 = DS2, D5 = SS
	floppy_image_device *floppy = nullptr;
	for (int i = 0; i < 2 && floppy == nullptr; i++)
		if (BIT(data, i))
			floppy = m_floppy[i]->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy != nullptr)
		floppy->ss_w(BIT(data, 5));

	m_fdc->mr_w(BIT(data, 7));
}

void gnat10_state::baud0_w(u8 data)
{
	m_brg->str_w(data & 0x0f);
	m_ram_on = true;
}

void gnat10_state::baud1_w(u8 data)
{
	m_brg->stt_w((data & 0xf0) >> 4);
}

void gnat10_state::mem_map(address_map &map)
{
	map(0x0000, 0xf7ff).r(FUNC(gnat10_state::ram_r)).writeonly().share("ram");
	map(0xf800, 0xffff).rom().region("monitor", 0);
}

void gnat10_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x63).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x70, 0x73).rw("sio0", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x80, 0x83).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0xa0, 0xa0).rw(FUNC(gnat10_state::floppy_status_r), FUNC(gnat10_state::floppy_latch_w));
	map(0xd0, 0xd0).w(FUNC(gnat10_state::baud0_w));
	map(0xe0, 0xe0).w(FUNC(gnat10_state::baud1_w));
}


static INPUT_PORTS_START(gnat10)
INPUT_PORTS_END

static void gnat_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void gnat10_state::gnat10(machine_config &config)
{
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &gnat10_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &gnat10_state::io_map);

	Z80SIO(config, "sio0", 16_MHz_XTAL / 4); // MK3884
	Z80SIO(config, "sio1", 16_MHz_XTAL / 4);

	FD1793(config, m_fdc, 16_MHz_XTAL / 16);
	FLOPPY_CONNECTOR(config, m_floppy[0], gnat_floppies, "525qd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], gnat_floppies, "525qd", floppy_image_device::default_floppy_formats);

	COM5016_5(config, m_brg, 4.9152_MHz_XTAL); // specific type unknown
	m_brg->fr_handler().set("sio0", FUNC(z80sio_device::rxca_w));
	m_brg->fr_handler().append("sio0", FUNC(z80sio_device::txca_w));
	m_brg->fr_handler().append("sio1", FUNC(z80sio_device::rxca_w));
	m_brg->fr_handler().append("sio1", FUNC(z80sio_device::txca_w));
	m_brg->ft_handler().set("sio1", FUNC(z80sio_device::rxtxcb_w));

	// TODO: video screen, peripheral ports
}

ROM_START(gnat10)
	ROM_REGION(0x800, "monitor", 0)
	ROM_LOAD("gnat-507", 0x000, 0x800, CRC(72baa750) SHA1(7b78324b90b8c6f78c88a7dde8d53ea612ea1f7f)) // LF patched back to CR/LF in four instances
ROM_END

COMP(1980, gnat10, 0, 0, gnat10, gnat10, gnat10_state, empty_init, "GNAT Computers", "GNAT System 10", MACHINE_IS_SKELETON)
