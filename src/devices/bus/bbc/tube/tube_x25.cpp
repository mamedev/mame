// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Econet X25 Gateway

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_EconetX25.html

    TODO:
    - everything to be verified when a schematic is found.

**********************************************************************/


#include "emu.h"
#include "tube_x25.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_X25, bbc_tube_x25_device, "bbc_tube_x25", "Econet X25 Gateway Co-Processor")


//-------------------------------------------------
//  ADDRESS_MAP( primary_mem )
//-------------------------------------------------

void bbc_tube_x25_device::primary_mem(address_map &map)
{
	map(0x0000, 0x7fff).bankrw("bank0");
	map(0x0000, 0x0fff).rom().region("boot", 0);
	map(0x8000, 0xffff).bankrw("bank1");
}

//-------------------------------------------------
//  ADDRESS_MAP( primary_io )
//-------------------------------------------------

void bbc_tube_x25_device::primary_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x1f).rw(m_ula, FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
}

//-------------------------------------------------
//  ADDRESS_MAP( secondary_mem )
//-------------------------------------------------

void bbc_tube_x25_device::secondary_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("x25", 0);
	map(0x8000, 0xffff).bankrw("bank1");
}

//-------------------------------------------------
//  ADDRESS_MAP( secondary_io )
//-------------------------------------------------

void bbc_tube_x25_device::secondary_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x04, 0x07).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
}

//-------------------------------------------------
//  ROM( tube_x25 )
//-------------------------------------------------

ROM_START(tube_x25)
	ROM_REGION(0x1000, "boot", 0)
	ROM_LOAD("0246,200_01-x25-boot.rom", 0x0000, 0x1000, CRC(8088edd9) SHA1(9692c77712d006596c89c18ae9cdf01d5be5b487))

	ROM_REGION(0x8000, "x25", 0)
	ROM_LOAD("0246,213_01-x25-lo-rom1.rom", 0x0000, 0x4000, CRC(43679e1d) SHA1(240a46f7fb9fb8eee834b36aa8526f1f3ff7e00c))
	ROM_LOAD("0246,214_01-x25-hi-rom2.rom", 0x4000, 0x4000, CRC(e77ce291) SHA1(3d9e3f58f878d1f7c743b05e371845b3f7627129))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ nullptr }
};


void bbc_tube_x25_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80[0], 12_MHz_XTAL / 2);
	m_z80[0]->set_addrmap(AS_PROGRAM, &bbc_tube_x25_device::primary_mem);
	m_z80[0]->set_addrmap(AS_IO, &bbc_tube_x25_device::primary_io);
	m_z80[0]->set_irq_acknowledge_callback(FUNC(bbc_tube_x25_device::irq_callback));

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_z80[0], INPUT_LINE_NMI);
	m_ula->pirq_handler().set_inputline(m_z80[0], INPUT_LINE_IRQ0);
	m_ula->prst_handler().set_inputline(m_z80[0], INPUT_LINE_RESET);

	Z80(config, m_z80[1], 12_MHz_XTAL / 4);
	m_z80[1]->set_addrmap(AS_PROGRAM, &bbc_tube_x25_device::secondary_mem);
	m_z80[1]->set_addrmap(AS_IO, &bbc_tube_x25_device::secondary_io);
	m_z80[1]->set_daisy_config(daisy_chain);

	z80sio_device& sio(Z80SIO(config, "sio", 12_MHz_XTAL / 4));
	sio.out_int_callback().set_inputline(m_z80[1], INPUT_LINE_IRQ0);

	z80ctc_device& ctc(Z80CTC(config, "ctc", 12_MHz_XTAL / 4));
	ctc.zc_callback<0>().set("sio", FUNC(z80sio_device::rxca_w));
	ctc.zc_callback<0>().append("sio", FUNC(z80sio_device::txca_w));
	ctc.zc_callback<1>().set("sio", FUNC(z80sio_device::rxcb_w));
	ctc.zc_callback<1>().append("sio", FUNC(z80sio_device::txcb_w));
	ctc.intr_callback().set_inputline(m_z80[1], INPUT_LINE_IRQ0);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_x25_device::device_rom_region() const
{
	return ROM_NAME( tube_x25 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_x25_device - constructor
//-------------------------------------------------

bbc_tube_x25_device::bbc_tube_x25_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_X25, tag, owner, clock)
	, device_bbc_tube_interface(mconfig, *this)
	, m_z80(*this, "z80_%u", 0)
	, m_ula(*this, "ula")
	, m_bank(*this, "bank%u", 0)
	, m_ram(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_x25_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x10000);

	m_bank[0]->set_base(m_ram.get());
	m_bank[1]->set_base(m_ram.get() + 0x8000);

	/* register for save states */
	save_pointer(NAME(m_ram), 0x10000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_x25_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_x25_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}


//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(bbc_tube_x25_device::irq_callback)
{
	return 0xfe;
}
