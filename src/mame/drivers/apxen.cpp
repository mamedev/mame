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
	- DMA
	- Harddisk
	- Keyboard
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
#include "machine/mm58274c.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"
#include "machine/z8536.h"
#include "sound/sn76496.h"
#include "imagedev/floppy.h"
#include "formats/apridisk.h"
#include "bus/apricot/video/video.h"
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
	virtual void device_start() override;
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
		m_eeprom(*this, "eeprom"),
		m_pic(*this, "pic%u", 0U),
		m_daisy(*this, "daisy"),
		m_pit(*this, "pit"),
		m_cio(*this, "cio"),
		m_sio(*this, "sio"),
		m_video(*this, "video")
	{ }

	void apxen(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i80286_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_mem;
	required_device<address_map_bank_device> m_io;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device_array<pic8259_device, 2> m_pic;
	required_device<xen_daisy_device> m_daisy;
	required_device<pit8253_device> m_pit;
	required_device<z8536_device> m_cio;
	required_device<z80sio_device> m_sio;
	required_device<apricot_video_slot_device> m_video;

	void mem_map_base(address_map &map);
	void mem_map(address_map &map);
	void io_map_base(address_map &map);
	void io_map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER(apvid_w);

	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint8_t get_slave_ack(offs_t offset);
	void leds_w(uint8_t data);
	uint8_t cpu_control_r();
	void cpu_control_w(uint8_t data);
	void cpu_reset_w(uint8_t data);

	void cio_porta_w(uint8_t data);
	void cio_portb_w(uint8_t data);
	void cio_portc_w(uint8_t data);

	bool m_apvid;
	uint8_t m_cpu_control;
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
//  map(0xc80, 0xc87) FDC
//  map(0xc90, 0xc93) uPD7261 HDC
	map(0xca0, 0xca3).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xca4, 0xca7).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xcb0, 0xcb7).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
//  map(0xcc0, 0xccf) uPD71071 DMA 1
//  map(0xcd0, 0xcdf) uPD71071 DMA 2
	map(0xce0, 0xce0).rw(FUNC(apxen_state::cpu_control_r), FUNC(apxen_state::cpu_control_w));
	map(0xcf0, 0xcf0).w(FUNC(apxen_state::cpu_reset_w));
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( apxen )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

WRITE_LINE_MEMBER( apxen_state::apvid_w )
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

void apxen_state::cio_porta_w(uint8_t data)
{
	logerror("cio_porta_w: %02x\n", data);
}

void apxen_state::cio_portb_w(uint8_t data)
{
	logerror("cio_portb_w: %02x\n", data);
}

void apxen_state::cio_portc_w(uint8_t data)
{
	logerror("cio_portc_w: %02x\n", data);

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

	ADDRESS_MAP_BANK(config, m_io);
	m_io->set_addrmap(AS_PROGRAM, &apxen_state::io_map);
	m_io->set_data_width(16);
	m_io->set_addr_width(16);

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

	Z8536(config, m_cio, 4000000);
	m_cio->irq_wr_cb().set(m_pic[0], FUNC(pic8259_device::ir1_w));
	m_cio->pa_wr_cb().set(FUNC(apxen_state::cio_porta_w));
	m_cio->pb_wr_cb().set(FUNC(apxen_state::cio_portb_w));
	m_cio->pc_wr_cb().set(FUNC(apxen_state::cio_portc_w));

	Z80SIO(config, m_sio, 4000000);
	m_sio->out_dtrb_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write));
	m_sio->out_rtsb_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::cs_write));
	// channel a rs232, b keyboard

	MM58274C(config, "rtc", 32.768_kHz_XTAL);

	// video hardware
	APRICOT_VIDEO_SLOT(config, m_video, apricot_video_cards, "mono");
	m_video->apvid_handler().set(FUNC(apxen_state::apvid_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489(config, "sn", 2000000).add_route(ALL_OUTPUTS, "mono", 1.0);
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
