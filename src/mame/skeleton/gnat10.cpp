// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for CP/M system by GNAT Computers, Inc.

    For more background on this company, visit the following link:
    https://classictech.wordpress.com/computer-companies/gnat-computers-san-diego-calif/

****************************************************************************/

#include "emu.h"
//#include "bus/ieee488/ieee488.h"
//#include "bus/rs232/rs232.h"
//#include "machine/tms9914.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
//#include "machine/am9511.h"
//#include "machine/am9517a.h"
//#include "machine/i8155.h"
//#include "machine/i8251.h"
#include "machine/com8116.h"
//#include "machine/mm58167.h"
//#include "machine/tms9914.h"
#include "machine/wd_fdc.h"
//#include "machine/z80ctc.h"
//#include "machine/z80pio.h"
#include "machine/z80sio.h"
//#include "video/mc6845.h"
//#include "screen.h"
//#include "speaker.h"


namespace {

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
		, m_prom_disable(false)
	{
	}

	void gnat10(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 mem_r(offs_t offset);
	u8 floppy_status_r();
	void floppy_latch_w(u8 data);
	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);
	void baud0_w(u8 data);
	void baud1_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void video_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<com8116_device> m_brg;
	required_region_ptr<u8> m_monitor;
	required_shared_ptr<u8> m_ram;

	bool m_ram_on;
	bool m_prom_disable;
};

void gnat10_state::machine_start()
{
	save_item(NAME(m_ram_on));
	save_item(NAME(m_prom_disable));
}

void gnat10_state::machine_reset()
{
	m_ram_on = false;
	floppy_latch_w(0);
}


u8 gnat10_state::mem_r(offs_t offset)
{
	if (m_ram_on && (m_prom_disable || offset < 0xf800))
		return m_ram[offset];
	else
		return m_monitor[offset & 0x07ff];
}

u8 gnat10_state::floppy_status_r()
{
	if (!machine().side_effects_disabled())
	{
		// TODO: this also forces the motor on (it turns off on a timeout)
	}
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

	m_prom_disable = BIT(data, 6);

	m_fdc->mr_w(BIT(data, 7));
}

u8 gnat10_state::rtc_r(offs_t offset)
{
	return 0; //m_rtc->read(offset ^ 0x10);
}

void gnat10_state::rtc_w(offs_t offset, u8 data)
{
	//m_rtc->write(offset ^ 0x10, data);
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
	map(0x0000, 0xffff).r(FUNC(gnat10_state::mem_r)).writeonly().share("ram");
}

void gnat10_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	//map(0x00, 0x0f).rw("dmac", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	//map(0x10, 0x11).mirror(0xe).rw("apu", FUNC(am9511_device::read), FUNC(am9511_device::write));
	//map(0x20, 0x23).mirror(0xc).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	//map(0x30, 0x3f).rw("488c", FUNC(tms9914_device::read), FUNC(tms9914_device::write));
	//map(0x40, 0x40).mirror(0xf).rw(FUNC(gnat10_state::pdma1_r), FUNC(gnat10_state::pdma1_w));
	//map(0x50, 0x53).mirror(0xc).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x60, 0x63).mirror(0xc).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x70, 0x73).mirror(0xc).rw("sio0", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x80, 0x83).mirror(0xc).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	//map(0x90, 0x90).mirror(0xf).rw(FUNC(gnat10_state::pdma0_r), FUNC(gnat10_state::pdma0_w));
	map(0xa0, 0xa0).mirror(0xf).rw(FUNC(gnat10_state::floppy_status_r), FUNC(gnat10_state::floppy_latch_w));
	map(0xb0, 0xcf).rw(FUNC(gnat10_state::rtc_r), FUNC(gnat10_state::rtc_w));
	map(0xd0, 0xd0).mirror(0xf).w(FUNC(gnat10_state::baud0_w));
	map(0xe0, 0xe0).mirror(0xf).w(FUNC(gnat10_state::baud1_w));
	//map(0xf0, 0xf0).r(?); // undocumented switches?
}

void gnat10_state::video_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).mirror(0x800).rom().region("videocpu", 0);
	map(0x1000, 0x13ff).mirror(0xc00).ram();
	map(0x2000, 0x27ff).mirror(0x800).ram().share("vram");
	//map(0x3000, 0x3000).mirror(0xfff).w(FUNC(gnat10_state::reset_out_w));
	//map(0x4000, 0x40ff).mirror(0x700).rw("videopio", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	//map(0x4800, 0x4807).mirror(0x7f8).rw("videopio", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	//map(0x5000, 0x5001).mirror(0xffe).rw("videosio", FUNC(i8251_device::read), FUNC(i8251_device::write));
	//map(0x6000, 0x6000).mirror(0xfff).w(FUNC(gnat10_state::bell_w));
	//map(0x7000, 0x7000).mirror(0xffe).w("crtc", FUNC(mc6845_device::address_w));
	//map(0x7001, 0x7001).mirror(0xffe).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
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
	FLOPPY_CONNECTOR(config, m_floppy[0], gnat_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], gnat_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);

	COM5016_5(config, m_brg, 4.9152_MHz_XTAL); // BR1941-5
	m_brg->fr_handler().set("sio0", FUNC(z80sio_device::rxca_w));
	m_brg->fr_handler().append("sio0", FUNC(z80sio_device::txca_w));
	m_brg->fr_handler().append("sio1", FUNC(z80sio_device::rxca_w));
	m_brg->fr_handler().append("sio1", FUNC(z80sio_device::txca_w));
	m_brg->ft_handler().set("sio1", FUNC(z80sio_device::rxtxcb_w));

	// TODO: DMAC, peripheral ports

	i8085a_cpu_device &videocpu(I8085A(config, "videocpu", 18.432_MHz_XTAL / 4));
	videocpu.set_addrmap(AS_PROGRAM, &gnat10_state::video_map);
	videocpu.set_disable();

	// TODO: 8155, 8251A, 6845, video screen
}

ROM_START(gnat10)
	ROM_REGION(0x800, "monitor", 0)
	ROM_LOAD("gnat-507", 0x000, 0x800, CRC(72baa750) SHA1(7b78324b90b8c6f78c88a7dde8d53ea612ea1f7f)) // LF patched back to CR/LF in four instances

	ROM_REGION(0x800, "videocpu", 0)
	ROM_LOAD("videocpu.bin", 0x000, 0x800, NO_DUMP) // TMS2716 or TMS2732

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("chargen.bin", 0x000, 0x800, NO_DUMP) // TMS2716 or TMS2732
ROM_END

} // anonymous namespace


COMP(1980, gnat10, 0, 0, gnat10, gnat10, gnat10_state, empty_init, "GNAT Computers", "GNAT System 10", MACHINE_IS_SKELETON)
