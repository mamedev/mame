// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot XEN

    Codename "Candyfloss". The system is not IBM compatible but can run
    MS-DOS (and Windows) as well as Xenix.

    Models:
    - XEN FD (512 KB RAM, 2x 720 KB floppy)
    - XEN HD (1 MB RAM, 1x 720 KB or 1.2 MB floppy, 20M HDD)
    - XEN WS (1 MB RAM, no drives)

    TODO:
    - Boot ROM disable, wrap around mode
    - Floppy
      * FDC is slightly too slow (or CPU too fast), causing Error 28 on boot
        can be fixed by setting delay_register_commit to 12 in wd_fdc.cpp
      * Issues accessing the second drive
    - DMA (verify, seems to fast)
    - Harddisk
    - XEN keyboard (currently using the Apricot Xi keyboard)
    - RS232
    - Printer
    - Colour graphics board
    - Make xen_daisy_device generic?

    Notes:
    - Two graphics cards: Mono and colour boards

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i286.h"
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "machine/input_merger.h"
#include "machine/mm58274c.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/upd71071.h"
#include "machine/wd_fdc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"
#include "machine/z8536.h"
#include "sound/sn76496.h"
#include "imagedev/floppy.h"
#include "formats/apridisk.h"
#include "bus/apricot/keyboard/keyboard.h"
#include "bus/apricot/video/video.h"
#include "softlist_dev.h"
#include "speaker.h"


//**************************************************************************
//  XEN DAISY CHAIN ABSTRACTION
//**************************************************************************

class xen_daisy_device : public device_t, public z80_daisy_chain_interface
{
public:
	xen_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	uint8_t acknowledge();
	IRQ_CALLBACK_MEMBER(inta_cb);

protected:
	virtual void device_start() override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(XEN_DAISY, xen_daisy_device, "xen_daisy", "Apricot XEN daisy chain abstraction")

xen_daisy_device::xen_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XEN_DAISY, tag, owner, clock)
	, z80_daisy_chain_interface(mconfig, *this)
{
}

void xen_daisy_device::device_start()
{
}

uint8_t xen_daisy_device::acknowledge()
{
	device_z80daisy_interface *intf = daisy_get_irq_device();
	if (intf != nullptr)
		return intf->z80daisy_irq_ack();
	else
		return 0xff;
}

IRQ_CALLBACK_MEMBER(xen_daisy_device::inta_cb)
{
	return acknowledge();
}


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class apxen_state : public driver_device
{
public:
	apxen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mem(*this, "mem"),
		m_io(*this, "io"),
		m_dmac(*this, "dma"),
		m_eeprom(*this, "eeprom"),
		m_pic(*this, "pic%u", 0U),
		m_daisy(*this, "daisy"),
		m_pit(*this, "pit"),
		m_cio(*this, "cio"),
		m_sio(*this, "sio"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0U),
		m_video(*this, "apvideo"),  // "video" causes assert in debug build
		m_cur_floppy(nullptr)
	{ }

	void apxen(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i80286_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_mem;
	required_device<address_map_bank_device> m_io;
	required_device<upd71071_device> m_dmac;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device_array<pic8259_device, 2> m_pic;
	required_device<xen_daisy_device> m_daisy;
	required_device<pit8253_device> m_pit;
	required_device<z8536_device> m_cio;
	required_device<z80sio_device> m_sio;
	required_device<wd2797_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<apricot_video_slot_device> m_video;

	void mem_map_base(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void io_map_base(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	void apvid_w(int state);

	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint8_t get_slave_ack(offs_t offset);
	void leds_w(uint8_t data);
	uint8_t cpu_control_r();
	void cpu_control_w(uint8_t data);
	void cpu_reset_w(uint8_t data);

	uint8_t cio_porta_r();
	void cio_porta_w(uint8_t data);
	void cio_portb_w(uint8_t data);
	void cio_portc_w(uint8_t data);

	floppy_image_device *m_cur_floppy = nullptr;
	bool m_apvid = false;
	uint8_t m_cpu_control = 0;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void apxen_state::mem_map_base(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(apxen_state::mem_r), FUNC(apxen_state::mem_w));
}

void apxen_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram();
	map(0x080000, 0x0effff).noprw(); // silence memory test
	map(0x0f0000, 0x0fffff).rom().region("bios", 0);
	map(0xff0000, 0xffffff).rom().region("bios", 0);
}

void apxen_state::io_map_base(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(apxen_state::io_r), FUNC(apxen_state::io_w));
}

void apxen_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0xc00, 0xc00).w(FUNC(apxen_state::leds_w));
//  map(0xc20, 0xc20) printer
	map(0xc30, 0xc30).w("sn", FUNC(sn76489_device::write));
	map(0xc40, 0xc47).rw(m_cio, FUNC(z8536_device::read), FUNC(z8536_device::write)).umask16(0x00ff);
	map(0xc50, 0xc57).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)).umask16(0x00ff);
	map(0xc60, 0xc7f).rw("rtc", FUNC(mm58274c_device::read), FUNC(mm58274c_device::write)).umask16(0x00ff);
	map(0xc80, 0xc87).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write)).umask16(0x00ff);
//  map(0xc90, 0xc93) uPD7261 HDC
	map(0xca0, 0xca3).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xca4, 0xca7).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xcb0, 0xcb7).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0xcc0, 0xccf).rw(m_dmac, FUNC(upd71071_device::read), FUNC(upd71071_device::write));
//  map(0xcd0, 0xcdf) uPD71071 DMA 2 (optional, with XP box)
	map(0xce0, 0xce0).rw(FUNC(apxen_state::cpu_control_r), FUNC(apxen_state::cpu_control_w));
	map(0xcf0, 0xcf0).w(FUNC(apxen_state::cpu_reset_w));
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( apxen )
INPUT_PORTS_END


//**************************************************************************
//  FLOPPY
//**************************************************************************

void apxen_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_APRIDISK_FORMAT);
}

static void apricot_floppies(device_slot_interface &device)
{
	device.option_add("d32w", SONY_OA_D32W);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void apxen_state::apvid_w(int state)
{
	m_apvid = bool(state);
}


//**************************************************************************
//  MEMORY
//**************************************************************************

uint16_t apxen_state::mem_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	// pc/xi compatible mode vram
	if ((offset << 1) < 0x10000 && !m_apvid)
		m_video->mem_r(offset & (0xffff >> 1), data, mem_mask);

	// pc/xi video pointer mirror
	else if  ((offset << 1) >= 0xf0000 && (offset << 1) <= 0xf0fff && !m_apvid)
		m_video->mem_r((offset & (0xffff >> 1)) | (0x0e000 >> 1), data, mem_mask);

	// xen mode vram
	else if  ((offset << 1) >= 0xe0000 && (offset << 1) <= 0xeffff && m_apvid)
		m_video->mem_r(offset & (0xffff >> 1), data, mem_mask);

	// normal access
	else
		data &= m_mem->read16(offset, mem_mask);

	return data;
}

void apxen_state::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// pc/xi compatible mode vram
	if ((offset << 1) < 0x10000 && !m_apvid)
		m_video->mem_w(offset & (0xffff >> 1), data, mem_mask);

	// pc/xi video pointer mirror
	else if  ((offset << 1) >= 0xf0000 && (offset << 1) <= 0xf0fff && !m_apvid)
		m_video->mem_w((offset & (0xffff >> 1)) | (0x0e000 >> 1), data, mem_mask);

	// xen mode vram
	else if  ((offset << 1) >= 0xe0000 && (offset << 1) <= 0xeffff && m_apvid)
		m_video->mem_w(offset & (0xffff >> 1), data, mem_mask);

	// normal access
	else
		m_mem->write16(offset, data, mem_mask);
}

uint16_t apxen_state::io_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (!m_video->io_r(offset, data, mem_mask))
		data &= m_io->read16(offset, mem_mask);

	return data;
}

void apxen_state::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!m_video->io_w(offset, data, mem_mask))
		m_io->write16(offset, data, mem_mask);
}


//**************************************************************************
//  INTERRUPT HANDLING
//**************************************************************************

static const z80_daisy_config xen_daisy_chain[] =
{
	{ "cio" },
	{ "sio" },
	{ nullptr }
};

uint8_t apxen_state::get_slave_ack(offs_t offset)
{
	// z80 daisy chain
	if (offset == 1)
		return m_daisy->acknowledge();

	// slave pic
	if (offset == 2)
		return m_pic[1]->acknowledge();

	return 0x00;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void apxen_state::machine_start()
{
}

void apxen_state::machine_reset()
{
	// boot rom enabled, hard reset
	m_cpu_control = 0x00;
}

void apxen_state::leds_w(uint8_t data)
{
	logerror("leds_w: %02x\n", data);

	// 76543---  not used
	// -----2--  hdd led
	// ------1-  fdd led
	// -------0  voice led
}

uint8_t apxen_state::cpu_control_r()
{
	// only bits 0 and 1 can be read back
	return m_cpu_control & 0x03;
}

void apxen_state::cpu_control_w(uint8_t data)
{
	logerror("cpu_control_w: %02x\n", data);

	// 76543---  not used
	// -----2--  full memory (virtual mode) or 1 mb wraparound (real mode)
	// ------1-  hard/soft reset
	// -------0  boot rom enabled

	m_cpu_control = data;
}

void apxen_state::cpu_reset_w(uint8_t data)
{
	logerror("cpu_reset_w: %02x\n", data);

	// data written doesn't matter
	m_maincpu->reset();
}

uint8_t apxen_state::cio_porta_r()
{
	uint8_t data = 0x00;

	data |= m_cur_floppy ? (m_cur_floppy->dskchg_r() << 3) : 0;

	logerror("cio_porta_r: %02x\n", data);

	return data;
}

void apxen_state::cio_porta_w(uint8_t data)
{
	logerror("cio_porta_w: %02x\n", data);

	// 7-------  hdc interrupt (input)
	// -6------  floppy/tape select
	// --5-----  winchester select
	// ---4----  floppy select
	// ----3---  floppy disk change (input)
	// -----2--  floppy disk head load
	// ------10  floppy drive select

	if (BIT(data, 4) == 0)
	{
		if ((data & 0x03) < 2)
			m_cur_floppy = m_floppy[data & 0x03]->get_device();
		else
			m_cur_floppy = nullptr;

		m_fdc->set_floppy(m_cur_floppy);

		// motor always active for now
		if (m_cur_floppy)
			m_cur_floppy->mon_w(0);
	}
}

void apxen_state::cio_portb_w(uint8_t data)
{
	logerror("cio_portb_w: %02x\n", data);

	// 7-------  rs232 dts (input)
	// -65-----  rs232 baud select
	// ---4----  track 43 precomp
	// ----3---  centronics busy (input)
	// -----2--  centronics select (input)
	// ------1-  centronics fault (input)
	// -------0  centronics paper empty (input)
}

void apxen_state::cio_portc_w(uint8_t data)
{
	logerror("cio_portc_w: %02x\n", data);

	// 3---  centronics strobe
	// -2--  eeprom clock
	// --1-  winchester head select 3
	// ---0  floppy disk change reset

	if (BIT(data, 0) && m_cur_floppy)
		m_cur_floppy->dskchg_w(0);

	m_eeprom->clk_write(BIT(data, 2));
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void apxen_state::apxen(machine_config &config)
{
	I80286(config, m_maincpu, 15_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &apxen_state::mem_map_base);
	m_maincpu->set_addrmap(AS_IO, &apxen_state::io_map_base);
	m_maincpu->set_irq_acknowledge_callback(m_pic[0], FUNC(pic8259_device::inta_cb));

	ADDRESS_MAP_BANK(config, m_mem);
	m_mem->set_addrmap(AS_PROGRAM, &apxen_state::mem_map);
	m_mem->set_data_width(16);
	m_mem->set_addr_width(24);
	m_mem->set_endianness(ENDIANNESS_LITTLE);

	ADDRESS_MAP_BANK(config, m_io);
	m_io->set_addrmap(AS_PROGRAM, &apxen_state::io_map);
	m_io->set_data_width(16);
	m_io->set_addr_width(16);
	m_io->set_endianness(ENDIANNESS_LITTLE);

	UPD71071(config, m_dmac, 8000000);
	m_dmac->set_cpu_tag("maincpu");
	m_dmac->set_clock(8000000);
	m_dmac->out_eop_callback().set(m_pic[1], FUNC(pic8259_device::ir2_w));
	m_dmac->dma_read_callback<1>().set(m_fdc, FUNC(wd2797_device::data_r));
	m_dmac->dma_write_callback<1>().set(m_fdc, FUNC(wd2797_device::data_w));

	EEPROM_93C06_16BIT(config, m_eeprom); // NMC9306
	m_eeprom->do_callback().set(m_sio, FUNC(z80sio_device::ctsb_w));

	PIC8259(config, m_pic[0], 0);
	m_pic[0]->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->read_slave_ack_callback().set(FUNC(apxen_state::get_slave_ack));

	PIC8259(config, m_pic[1], 0);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir2_w));
	m_pic[1]->in_sp_callback().set_constant(0);

	XEN_DAISY(config, m_daisy);
	m_daisy->set_daisy_config(xen_daisy_chain);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(2000000);
	m_pit->out_handler<0>().set(m_sio, FUNC(z80sio_device::rxca_w));
	m_pit->set_clk<1>(2000000);
	m_pit->out_handler<1>().set(m_sio, FUNC(z80sio_device::txca_w));
	m_pit->set_clk<2>(2000000);
	m_pit->out_handler<2>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	INPUT_MERGER_ANY_HIGH(config, "z80int").output_handler().set(m_pic[0], FUNC(pic8259_device::ir1_w));

	Z8536(config, m_cio, 4000000);
	m_cio->irq_wr_cb().set("z80int", FUNC(input_merger_device::in_w<0>));
	m_cio->pa_rd_cb().set(FUNC(apxen_state::cio_porta_r));
	m_cio->pa_wr_cb().set(FUNC(apxen_state::cio_porta_w));
	m_cio->pb_wr_cb().set(FUNC(apxen_state::cio_portb_w));
	m_cio->pc_wr_cb().set(FUNC(apxen_state::cio_portc_w));

	Z80SIO(config, m_sio, 4000000);
	m_sio->out_int_callback().set("z80int", FUNC(input_merger_device::in_w<1>));
	// channel a: rs232
	m_sio->out_txdb_callback().set("kbd", FUNC(apricot_keyboard_bus_device::out_w));
	m_sio->out_dtrb_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write));
	m_sio->out_rtsb_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::cs_write));

	MM58274C(config, "rtc", 32.768_kHz_XTAL);

	// floppy
	WD2797(config, m_fdc, 2000000);
	m_fdc->intrq_wr_callback().set(m_pic[1], FUNC(pic8259_device::ir1_w));
	m_fdc->drq_wr_callback().set([this](int state) { m_dmac->dmarq(state, 1); });
	FLOPPY_CONNECTOR(config, "fdc:0", apricot_floppies, "d32w", apxen_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", apricot_floppies, "d32w", apxen_state::floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("apxen_flop");

	// video hardware
	APRICOT_VIDEO_SLOT(config, m_video, apricot_video_cards, "mono");
	m_video->apvid_handler().set(FUNC(apxen_state::apvid_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489(config, "sn", 2000000).add_route(ALL_OUTPUTS, "mono", 1.0);

	// keyboard
	APRICOT_KEYBOARD_INTERFACE(config, "kbd", apricot_keyboard_devices, "hle").in_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( apxen )
	ROM_REGION16_LE(0x10000, "bios", 0)
	// LO-XEN  3.1.3 7143 (checksum matches)
	ROM_LOAD16_BYTE("lo-xen_313.ic80", 0x0000, 0x8000, CRC(c2a1fd6e) SHA1(8dfc711dd910bc3d43c1120978ba199a13463068))
	// HI-XEN  3.1.3 9BF0 (checksum matches)
	ROM_LOAD16_BYTE("hi-xen_313.ic37", 0x0001, 0x8000, CRC(72ee2f09) SHA1(da11043d40a694802f6d3d27a4359067dd19c8e6))

	// default eeprom configured with 2 floppy drives and serial no. 123456
	ROM_REGION16_LE(0x20, "eeprom", 0)
	ROM_LOAD("eeprom.nv", 0x00, 0x20, CRC(c26d455e) SHA1(ff2d7af6ca21b2fba4c5a9e90926b5049a9fdc86))

	// should probably be moved elsewhere
	ROM_REGION(0x2000, "hdd", 0)
	ROM_LOAD("rodime_ro3055.bin", 0x0000, 0x2000, CRC(61d1544a) SHA1(2177a4c6409c0ee3d3e3e6c659085adf236f8726))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME       FLAGS
COMP( 1985, apxen, 0,      0,      apxen,   apxen, apxen_state, empty_init, "ACT",   "Apricot XEN", MACHINE_NOT_WORKING )
