// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Tandy Radio Shack TRS-80 Model II/12/16/16B/6000

    http://home.iae.nl/users/pb0aia/cm/modelii.html

    Model II/16: Z80 CPU board with the boot ROM, CTC, SIO and DMA,
    everything else on cards in the card cage.

    Model 12/16B/6000: main logic board 8000166 with the Z80, CTC, SIO,
    DMA, PIO, FD2793, 80K RAM, boot ROM and sound, and the video/keyboard
    connector J0 that takes either the video/keyboard board or the
    26-6017 card cage.

*/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/trs80m2/slot.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/input_merger.h"
#include "machine/output_latch.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "sound/beep.h"

#include "softlist_dev.h"
#include "speaker.h"

#include <algorithm>


namespace {

class trs80m2_base_state : public driver_device
{
protected:
	trs80m2_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "u12"),
		m_ctc(*this, "u19"),
		m_dmac(*this, "u20"),
		m_sio(*this, "u18"),
		m_bus(*this, "bus"),
		m_rom(*this, "u12")
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cpu_board(machine_config &config) ATTR_COLD;

	void kbirq_w(int state);

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dma_device> m_dmac;
	required_device<z80sio_device> m_sio;
	required_device<trs80m2_bus_device> m_bus;
	required_region_ptr<uint8_t> m_rom;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;

	uint8_t m_boot_rom = 0;
};


class trs80m2_state : public trs80m2_base_state
{
public:
	trs80m2_state(const machine_config &mconfig, device_type type, const char *tag) :
		trs80m2_base_state(mconfig, type, tag),
		m_slot(*this, "slot%u", 1U)
	{
	}

	void trs80m2(machine_config &config) ATTR_COLD;
	void trs80m16(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	void z80_mem(address_map &map) ATTR_COLD;
	void z80_io(address_map &map) ATTR_COLD;

	void rom_enable_w(uint8_t data);
	void update_rom_tap();

	required_device_array<trs80m2_slot_device, 7> m_slot;

	memory_passthrough_handler m_rom_tap;
	bool m_rom_tap_active = false;
};


class trs80m12_state : public trs80m2_base_state
{
public:
	trs80m12_state(const machine_config &mconfig, device_type type, const char *tag) :
		trs80m2_base_state(mconfig, type, tag),
		m_pio(*this, "u22"),
		m_fdc(*this, "u46"),
		m_floppies(*this, "u46:%u", 0U),
		m_centronics(*this, "j2"),
		m_cent_data_out(*this, "cent_data_out"),
		m_dmarq(*this, "dmarq"),
		m_beep(*this, "beep"),
		m_j0(*this, "j0"),
		m_config(*this, "CONFIG"),
		m_ram(*this, "ram", 0x10000, ENDIANNESS_LITTLE),
		m_ram2(*this, "ram2", 0x10000, ENDIANNESS_LITTLE),
		m_lo(*this, "lo")
	{
	}

	void trs80m12(machine_config &config) ATTR_COLD;
	void trs80m16b(machine_config &config) ATTR_COLD;
	void trs80m16bhd(machine_config &config) ATTR_COLD;
	void tandy6k(machine_config &config) ATTR_COLD;
	void tandy6khd(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	void main_board(machine_config &config) ATTR_COLD;
	void m12_mem(address_map &map) ATTR_COLD;
	void m12_io(address_map &map) ATTR_COLD;

	static constexpr u32 LO_ENTRIES = 64;

	void install_ram() ATTR_COLD;
	uint8_t *page_ram(int page, offs_t &end) ATTR_COLD;
	void page_w(uint8_t entry);
	void update_lo();
	void m12_rom_enable_w(uint8_t data);
	void fdc_reset_w(uint8_t data);
	void drvslt_w(uint8_t data);
	void select_floppy();
	uint8_t pio_pa_r();
	void pio_pa_w(uint8_t data);

	required_device<z80pio_device> m_pio;
	required_device<wd2793_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_merger_device> m_dmarq;
	required_device<beep_device> m_beep;
	required_device<trs80m2_slot_device> m_j0;
	required_ioport m_config;
	memory_share_creator<uint8_t> m_ram;
	memory_share_creator<uint8_t> m_ram2;
	memory_view m_lo;

	floppy_image_device *m_floppy = nullptr;
	bool m_ram_installed = false;
	uint8_t m_page = 0;
	uint8_t m_drvslt = 0xff;
	uint8_t m_centronics_busy = 0;
	uint8_t m_centronics_fault = 0;
	uint8_t m_centronics_perror = 0;
	uint8_t m_centronics_select = 0;
};


//**************************************************************************
//  COMMON
//**************************************************************************

void trs80m2_base_state::kbirq_w(int state)
{
	m_ctc->trg3(!state);
}

void trs80m2_base_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).specific(m_program);
	m_maincpu->space(AS_IO).specific(m_io);

	m_ctc->trg3(1);

	save_item(NAME(m_boot_rom));
}

void trs80m2_base_state::machine_reset()
{
	m_boot_rom = 1;
}


//**************************************************************************
//  MODEL II CPU BOARD
//**************************************************************************

void trs80m2_state::update_rom_tap()
{
	if (bool(m_boot_rom) == m_rom_tap_active)
		return;

	m_rom_tap.remove();
	m_rom_tap_active = bool(m_boot_rom);

	if (m_rom_tap_active)
	{
		m_rom_tap = m_maincpu->space(AS_PROGRAM).install_read_tap(0x0000, 0x0fff, "boot_rom",
				[this] (offs_t offset, uint8_t &data, uint8_t mem_mask)
				{
					data = (offset < 0x800) ? m_rom[offset] : 0xff;
				});
	}
}

void trs80m2_state::rom_enable_w(uint8_t data)
{
	m_boot_rom = BIT(data, 0);
	update_rom_tap();
}

void trs80m2_state::z80_mem(address_map &map)
{
	map.unmap_value_low();
	m_bus.lookup()->page_map(map);
}

void trs80m2_state::z80_io(address_map &map)
{
	map.unmap_value_low();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xf4, 0xf7).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0xf8, 0xf8).rw(m_dmac, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0xf9, 0xf9).w(FUNC(trs80m2_state::rom_enable_w));
}

void trs80m2_state::machine_reset()
{
	trs80m2_base_state::machine_reset();

	update_rom_tap();
}

void trs80m2_state::device_post_load()
{
	trs80m2_base_state::device_post_load();

	update_rom_tap();
}


//**************************************************************************
//  MODEL 12 MAIN LOGIC BOARD
//**************************************************************************

uint8_t *trs80m12_state::page_ram(int page, offs_t &end)
{
	switch (page)
	{
	case 1:
		end = 0x7fff;
		return &m_ram[0x8000];

	case 2: case 3:
		if (m_config->read() == 2)
		{
			end = 0x7fff;
			return &m_ram2[(page & 1) * 0x8000];
		}
		break;

	case 14: case 15:
		if (m_config->read() == 1)
		{
			end = 0x3fff;
			return &m_ram2[0];
		}
		break;
	}

	return nullptr;
}

void trs80m12_state::install_ram()
{
	for (uint32_t i = 0; i < trs80m2_bus_device::PAGE_ENTRIES; i++)
	{
		offs_t end = 0x7fff;
		uint8_t *const ram = trs80m2_bus_device::entry_lowp(i) ? &m_ram[0] : page_ram(trs80m2_bus_device::entry_page(i), end);

		if (!ram)
			continue;

		offs_t const top = trs80m2_bus_device::entry_vram(i) ? std::min<offs_t>(end, 0x77ff) : end;
		m_bus->page_view(i).install_ram(0x8000, 0x8000 + top, ram);
		m_bus->claim_window(i);
	}

	for (uint32_t i = 0; i < LO_ENTRIES; i++)
	{
		offs_t end = 0x7fff;
		uint8_t *const ram = BIT(i, 4) ? page_ram(i & 0x0f, end) : &m_ram[0];

		if (ram)
			m_lo[i].install_ram(0x0000, end, ram);

		if (BIT(i, 5))
			m_lo[i].install_rom(0x0000, 0x07ff, 0x0800, &m_rom[0]);
	}
}

void trs80m12_state::update_lo()
{
	m_lo.select((m_boot_rom << 5) | (trs80m2_bus_device::entry_lowp(m_page) << 4) | trs80m2_bus_device::entry_page(m_page));
}

void trs80m12_state::page_w(uint8_t entry)
{
	m_page = entry;
	update_lo();
}

void trs80m12_state::m12_rom_enable_w(uint8_t data)
{
	m_boot_rom = BIT(data, 0);
	update_lo();
}

void trs80m12_state::fdc_reset_w(uint8_t data)
{
	m_fdc->mr_w(0);
	m_fdc->mr_w(1);
}

void trs80m12_state::drvslt_w(uint8_t data)
{
	/*

	    bit     signal

	    0       DS0
	    1       DS1
	    2       DS2
	    3       DS3
	    4
	    5
	    6       SIDE
	    7       DDEN

	*/

	m_drvslt = data;

	select_floppy();

	if (m_floppy)
		m_floppy->ss_w(!BIT(data, 6));

	m_fdc->dden_w(!BIT(data, 7));
}

void trs80m12_state::select_floppy()
{
	m_floppy = nullptr;

	for (int i = 0; i < 4; i++)
		if (!BIT(m_drvslt, i))
			m_floppy = m_floppies[i]->get_device();

	m_fdc->set_floppy(m_floppy);
}

uint8_t trs80m12_state::pio_pa_r()
{
	/*

	    bit     signal      description

	    0       INTRQ       FDC interrupt request
	    1       TWOSID      2-sided diskette
	    2       DSKCHG      door open
	    3       PRIME       printer reset (output)
	    4       FAULT       printer fault
	    5       PSEL        printer select
	    6       PE          paper empty
	    7       BUSY        printer busy

	*/

	uint8_t data = 0;

	data |= m_fdc->intrq_r() ? 0x01 : 0x00;
	data |= (m_floppy && !m_floppy->twosid_r()) ? 0x02 : 0x00;
	data |= (m_floppy && !m_floppy->dskchg_r()) ? 0x04 : 0x00;
	data |= m_centronics_fault << 4;
	data |= m_centronics_select << 5;
	data |= m_centronics_perror << 6;
	data |= m_centronics_busy << 7;

	return data;
}

void trs80m12_state::pio_pa_w(uint8_t data)
{
	m_centronics->write_init(BIT(data, 3));
}

void trs80m12_state::m12_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0x7fff).view(m_lo);

	for (uint32_t i = 0; i < LO_ENTRIES; i++)
		m_lo[i];

	m_bus.lookup()->page_map(map);
}

void trs80m12_state::m12_io(address_map &map)
{
	map.unmap_value_low();
	map.global_mask(0xff);
	map(0xa0, 0xa0).lw8(NAME([this] (uint8_t data) { m_beep->set_state(BIT(data, 0)); }));
	map(0xa8, 0xa9).lw8(NAME([this] (uint8_t data) { m_bus->lowp_w(BIT(data, 0)); }));
	map(0xe0, 0xe3).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xe4, 0xe7).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0xe8, 0xe8).w(FUNC(trs80m12_state::fdc_reset_w));
	map(0xef, 0xef).w(FUNC(trs80m12_state::drvslt_w));
	map(0xf0, 0xf3).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xf4, 0xf7).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0xf8, 0xf8).rw(m_dmac, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0xf9, 0xf9).w(FUNC(trs80m12_state::m12_rom_enable_w));
}

void trs80m12_state::machine_start()
{
	trs80m2_base_state::machine_start();

	std::fill_n(&m_ram[0], 0x10000, 0xff);
	std::fill_n(&m_ram2[0], 0x10000, 0xff);

	save_item(NAME(m_page));
	save_item(NAME(m_drvslt));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_fault));
	save_item(NAME(m_centronics_perror));
	save_item(NAME(m_centronics_select));
}

void trs80m12_state::machine_reset()
{
	if (!m_ram_installed)
	{
		m_ram_installed = true;
		install_ram();
	}

	trs80m2_base_state::machine_reset();

	m_bus->lowp_w(0);
	update_lo();

	m_beep->set_state(0);
}

void trs80m12_state::device_post_load()
{
	trs80m2_base_state::device_post_load();

	select_floppy();
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

INPUT_PORTS_START( trs80m2 )
INPUT_PORTS_END

INPUT_PORTS_START( trs80m12 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x01, "Second RAM bank" )
	PORT_CONFSETTING( 0x00, DEF_STR( None ) )
	PORT_CONFSETTING( 0x01, "16K, pages 14/15 (E38-E39)" )
	PORT_CONFSETTING( 0x02, "64K, pages 2/3" )
INPUT_PORTS_END

INPUT_PORTS_START( trs80m16b )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x00, "Second RAM bank" )
	PORT_CONFSETTING( 0x00, DEF_STR( None ) )
	PORT_CONFSETTING( 0x01, "16K, pages 14/15 (E38-E39)" )
	PORT_CONFSETTING( 0x02, "64K, pages 2/3" )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void trs80m2_floppies(device_slot_interface &device)
{
	device.option_add("8ssdd", FLOPPY_8_SSDD); // Shugart SA-800
	device.option_add("8dsdd", FLOPPY_8_DSDD); // Shugart SA-850
}

const z80_daisy_config trs80m2_daisy_chain[] =
{
	{ "u19" },
	{ "u18" },
	{ "u20" },
	{ "slot1" },
	{ "slot2" },
	{ "slot3" },
	{ "slot4" },
	{ "slot5" },
	{ "slot6" },
	{ "slot7" },
	{ nullptr }
};

const z80_daisy_config trs80m12_daisy_chain[] =
{
	{ "u19" },
	{ "u18" },
	{ "u20" },
	{ "u22" },
	{ "j0" },
	{ nullptr }
};

void trs80m2_base_state::cpu_board(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->busack_cb().set(m_dmac, FUNC(z80dma_device::bai_w));

	TRS80M2_BUS(config, m_bus);
	m_bus->set_cpu(m_maincpu);
	m_bus->int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_bus->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_bus->kbirq_callback().set(FUNC(trs80m2_base_state::kbirq_w));

	Z80CTC(config, m_ctc, 8_MHz_XTAL / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80DMA(config, m_dmac, 8_MHz_XTAL / 2);
	m_dmac->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSREQ);
	m_dmac->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dmac->in_mreq_callback().set([this] (offs_t offset) -> uint8_t { return m_program.read_byte(offset); });
	m_dmac->out_mreq_callback().set([this] (offs_t offset, uint8_t data) { m_program.write_byte(offset, data); });
	m_dmac->in_iorq_callback().set([this] (offs_t offset) -> uint8_t { return m_io.read_byte(offset); });
	m_dmac->out_iorq_callback().set([this] (offs_t offset, uint8_t data) { m_io.write_byte(offset, data); });

	Z80SIO(config, m_sio, 8_MHz_XTAL / 2); // SIO/0
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}

void trs80m2_state::trs80m2(machine_config &config)
{
	cpu_board(config);
	m_maincpu->set_daisy_config(trs80m2_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80m2_state::z80_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80m2_state::z80_io);

	m_bus->xferrq_callback().set(m_dmac, FUNC(z80dma_device::rdy_w));

	m_ctc->set_clk<0>(8_MHz_XTAL / 2 / 2);
	m_ctc->set_clk<1>(8_MHz_XTAL / 2 / 2);
	m_ctc->set_clk<2>(8_MHz_XTAL / 2 / 2);
	m_ctc->zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<1>().set(m_sio, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<2>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	TRS80M2_SLOT(config, m_slot[0], 8_MHz_XTAL / 2, m_bus, trs80m2_cards, "fdc");
	TRS80M2_SLOT(config, m_slot[1], 8_MHz_XTAL / 2, m_bus, trs80m2_cards, "vidkb");
	TRS80M2_SLOT(config, m_slot[2], 8_MHz_XTAL / 2, m_bus, trs80m2_cards, "ram64k");
	TRS80M2_SLOT(config, m_slot[3], 8_MHz_XTAL / 2, m_bus, trs80m2_cards, nullptr);
	TRS80M2_SLOT(config, m_slot[4], 8_MHz_XTAL / 2, m_bus, trs80m2_cards, nullptr);
	TRS80M2_SLOT(config, m_slot[5], 8_MHz_XTAL / 2, m_bus, trs80m2_cards, nullptr);
	TRS80M2_SLOT(config, m_slot[6], 8_MHz_XTAL / 2, m_bus, trs80m2_cards, nullptr);

	SOFTWARE_LIST(config, "flop_list").set_original("trs80m2_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("trs80m2_hdd");
}

void trs80m2_state::trs80m16(machine_config &config)
{
	trs80m2(config);

	m_slot[3]->set_default_option("m16");
	m_slot[4]->set_default_option("128k");

	SOFTWARE_LIST(config, "flop16_list").set_original("trs80m16_flop");
	SOFTWARE_LIST(config, "hdd16_list").set_original("trs80m16_hdd");
}

void trs80m12_state::main_board(machine_config &config)
{
	cpu_board(config);
	m_maincpu->z80_set_m1_cycles(4 + 1);
	m_maincpu->set_daisy_config(trs80m12_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80m12_state::m12_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80m12_state::m12_io);

	m_bus->xferrq_callback().set(m_dmarq, FUNC(input_merger_device::in_w<1>));
	m_bus->useirq_callback().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_bus->page_callback().set(FUNC(trs80m12_state::page_w));

	INPUT_MERGER_ANY_HIGH(config, m_dmarq).output_handler().set(m_dmac, FUNC(z80dma_device::rdy_w));

	m_ctc->set_clk<0>(8_MHz_XTAL / 2 / 2);
	m_ctc->set_clk<2>(8_MHz_XTAL / 2 / 2);
	m_ctc->zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<0>().append(m_sio, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<2>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	Z80PIO(config, m_pio, 8_MHz_XTAL / 2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(trs80m12_state::pio_pa_r));
	m_pio->out_pa_callback().set(FUNC(trs80m12_state::pio_pa_w));
	m_pio->out_pb_callback().set(m_cent_data_out, FUNC(output_latch_device::write));
	m_pio->out_brdy_callback().set([this] (int state) { m_centronics->write_strobe(!state); });

	WD2793(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(m_pio, FUNC(z80pio_device::pa0_w));
	m_fdc->drq_wr_callback().set(m_dmarq, FUNC(input_merger_device::in_w<0>));
	FLOPPY_CONNECTOR(config, m_floppies[0], trs80m2_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[1], trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[2], trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[3], trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_pio, FUNC(z80pio_device::strobe_b));
	m_centronics->busy_handler().set([this] (int state) { m_centronics_busy = state; });
	m_centronics->fault_handler().set([this] (int state) { m_centronics_fault = state; });
	m_centronics->perror_handler().set([this] (int state) { m_centronics_perror = state; });
	m_centronics->select_handler().set([this] (int state) { m_centronics_select = state; });

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1558).add_route(ALL_OUTPUTS, "mono", 0.25);

	TRS80M2_SLOT(config, m_j0, 8_MHz_XTAL / 2, m_bus, trs80m12_j0_cards, "vidkb12");

	SOFTWARE_LIST(config, "flop_list").set_original("trs80m2_flop");
}

void trs80m12_state::trs80m12(machine_config &config)
{
	main_board(config);
}

void cage_16b(device_t *device)
{
	device->subdevice<trs80m2_slot_device>("j5")->set_default_option("vidkb12");
	device->subdevice<trs80m2_slot_device>("j6")->set_default_option("m16b");
	device->subdevice<trs80m2_slot_device>("j7")->set_default_option("256k");
}

void cage_16bhd(device_t *device)
{
	cage_16b(device);
	device->subdevice<trs80m2_slot_device>("j1")->set_default_option("hdc");
}

void cage_6k(device_t *device)
{
	// TODO: the Tandy 6000 keyboard (tandy6k_kbd) has no dumped 8048 ROM, so the Model II keyboard stands in for it
	device->subdevice<trs80m2_slot_device>("j5")->set_default_option("vidkb12");
	device->subdevice<trs80m2_slot_device>("j6")->set_default_option("t6000");
	device->subdevice<trs80m2_slot_device>("j7")->set_default_option("512k");
}

void cage_6khd(device_t *device)
{
	cage_6k(device);
	device->subdevice<trs80m2_slot_device>("j1")->set_default_option("hdc");
}

void trs80m12_state::trs80m16b(machine_config &config)
{
	main_board(config);

	m_j0->set_default_option("cage");
	m_j0->set_option_machine_config("cage", cage_16b);

	SOFTWARE_LIST(config, "flop16b_list").set_original("trs80m16b_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("trs80m16b_hdd");
	SOFTWARE_LIST(config, "flop16_list").set_compatible("trs80m16_flop");
}

void trs80m12_state::trs80m16bhd(machine_config &config)
{
	trs80m16b(config);

	m_j0->set_option_machine_config("cage", cage_16bhd);
}

void trs80m12_state::tandy6k(machine_config &config)
{
	trs80m16b(config);

	m_j0->set_option_machine_config("cage", cage_6k);
}

void trs80m12_state::tandy6khd(machine_config &config)
{
	trs80m16b(config);

	m_j0->set_option_machine_config("cage", cage_6khd);
}


//**************************************************************************
//  ROMS
//**************************************************************************

/*

    TRS-80 Model II/16 Z80 CPU Board ROM

    It would seem that every processor board I find has a different ROM on it!  It seems that the early ROMs
    don't boot directly from a hard drive.  But there seems to be many versions of ROMs.  I've placed them in
    order of serial number in the list below.  There also appears to be at least two board revisions, "C" and "D".

    cpu_c8ff.bin/hex:
    Mask Programmable PROM, Equivilant to Intel 2716 EPROM, with checksum C8FF came from a cpu board with
    serial number 120353 out of a Model II with serial number 2002102 and catalog number 26-6002.  The board
    was labeled, "Revision C".  This appears to be an early ROM and according to a very helpful fellow
    collector, Aaron in Australia, doesn't allow boot directly from a hard disk.

    cpu_9733.bin/hex:
    An actual SGS-Ates (Now STMicroelectronics) 2716 EPROM, with checksum 9733 came from a cpu board with
    serial number 161993 out of a pile of random cards that I have.  I don't know what machine it originated
    from.  The board was labeled, "Revision C".  This appears to be a later ROM in that it is able to boot
    directly from an 8MB hard disk.  The EPROM had a windows sticker on it labeled, "U54".

    cpu_2119.bin/hex:
    An actual Texas Instruments TMS2516 EPROM, with checksum 2119 came from a cpu board with serial number
    178892 out of a Model 16 with serial number 64014509 and catalog number 26-4002.  The board was labeled,
    "Revision D".  This appears to be a later ROM and does appear to allow boot directly from an 8MB hard disk.

    cpu_2bff.bin/hex:
    Mask Programmable PROM, Equivilant to Intel 2716 EPROM, with checksum 2BFF came from a cpu board with
    serial number 187173 our of a pile of random cards that I have.  I don't know what machine it originated
    from.  The board was labeled, "Revision D".  This appears to be a later ROM in that it is able to boot
    directly from an 8MB hard disk.

    Checksums in parentheses are the MEMII "Shadow Rom Checksum" values used by Tandy's ROM checksum list.

*/

#define TRS80M2_BIOS \
	ROM_SYSTEM_BIOS( 0, "c8ff", "Version 1 (0A79)" ) \
	ROMX_LOAD( "8043216.u11", 0x0000, 0x0800, CRC(7017a373) SHA1(1c7127fcc99fc351a40d3a3199ba478e783c452e), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "432f", "Floppy only (432F)" ) \
	ROMX_LOAD( "cpu_432f.u11", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "2bff", "Version 2 (1981-07-29, 4A6A)" ) \
	ROMX_LOAD( "8047316.u11", 0x0000, 0x0800, CRC(c6c71d8b) SHA1(7107e2cbbe769851a4460680c2deff8e76a101b5), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "2119", "Version 3 (1982-05-07, 88E2)" ) \
	ROMX_LOAD( "cpu_2119.u11", 0x0000, 0x0800, CRC(7a663049) SHA1(f308439ce266df717bfe79adcdad6024b4faa141), ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 4, "1bbe", "Version 3 (1982-05-07, F42A)" ) \
	ROMX_LOAD( "cpu_1bbe.u11", 0x0000, 0x0800, CRC(8edceea7) SHA1(3d797acedd8a71a82c695129ca764f85aa9022b2), ROM_BIOS(4) ) \
	ROM_SYSTEM_BIOS( 5, "fc86", "Version 4 (1982-11-18, BFC0)" ) \
	ROMX_LOAD( "cpu_fc86.u11", 0x0000, 0x0800, CRC(764d113f) SHA1(2fb31bee2b74bf0fd00de54c0173fba0bbaf928f), ROM_BIOS(5) ) \
	ROM_SYSTEM_BIOS( 6, "9733", "Version 5 (1983-07-29, C9AC)" ) \
	ROMX_LOAD( "u54.u11", 0x0000, 0x0800, CRC(823924b1) SHA1(aee0625bcbd8620b28ab705e15ad9bea804c8476), ROM_BIOS(6) )

#define TRS80M12_BIOS \
	ROM_SYSTEM_BIOS( 0, "fc86", "Version 4 (1982-11-18, BFC0)" ) \
	ROMX_LOAD( "cpu_fc86.u54", 0x0000, 0x0800, CRC(764d113f) SHA1(2fb31bee2b74bf0fd00de54c0173fba0bbaf928f), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "9733", "Version 5 (1983-07-29, C9AC)" ) \
	ROMX_LOAD( "9733.u54", 0x0000, 0x0800, CRC(823924b1) SHA1(aee0625bcbd8620b28ab705e15ad9bea804c8476), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "1a95", "WD1010 (1A95)" ) \
	ROMX_LOAD( "u54_1a95.u54", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "cb11", "WD1010 (CB11)" ) \
	ROMX_LOAD( "u54_cb11.u54", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 4, "e996", "WD1010 (E996)" ) \
	ROMX_LOAD( "u54_e996.u54", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(4) ) \
	ROM_SYSTEM_BIOS( 5, "c745", "WD1010 (C745)" ) \
	ROMX_LOAD( "u54_c745.u54", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(5) )

#define TRS80M12_PLDS \
	ROM_LOAD( "82s153_m12.u50", 0x000, 0x0eb, NO_DUMP ) \
	ROM_LOAD( "82s153_cpm.u50", 0x0eb, 0x0eb, NO_DUMP ) \
	ROM_LOAD( "pal12l6.u10", 0x1d6, 0x034, NO_DUMP ) \
	ROM_LOAD( "pal16l8.u21", 0x20a, 0x104, NO_DUMP ) \
	ROM_LOAD( "pal16r6.u47", 0x30e, 0x104, NO_DUMP )

ROM_START( trs80m2 )
	ROM_REGION( 0x800, "u12", 0 )
	ROM_DEFAULT_BIOS( "9733" )
	TRS80M2_BIOS
ROM_END

ROM_START( trs80m16 )
	ROM_REGION( 0x800, "u12", 0 )
	ROM_DEFAULT_BIOS( "9733" )
	TRS80M2_BIOS
ROM_END

ROM_START( trs80m12 )
	ROM_REGION( 0x800, "u12", 0 )
	ROM_DEFAULT_BIOS( "fc86" )
	TRS80M12_BIOS

	ROM_REGION( 0x412, "plds", 0 )
	TRS80M12_PLDS
ROM_END

ROM_START( trs80m16b )
	ROM_REGION( 0x800, "u12", 0 )
	ROM_DEFAULT_BIOS( "fc86" )
	TRS80M12_BIOS

	ROM_REGION( 0x412, "plds", 0 )
	TRS80M12_PLDS

ROM_END

ROM_START( trs80m16bhd )
	ROM_REGION( 0x800, "u12", 0 )
	ROM_DEFAULT_BIOS( "9733" )
	TRS80M12_BIOS

	ROM_REGION( 0x412, "plds", 0 )
	TRS80M12_PLDS

ROM_END

ROM_START( tandy6k )
	ROM_REGION( 0x800, "u12", 0 )
	ROM_DEFAULT_BIOS( "9733" )
	TRS80M12_BIOS

	ROM_REGION( 0x412, "plds", 0 )
	TRS80M12_PLDS

ROM_END

ROM_START( tandy6khd )
	ROM_REGION( 0x800, "u12", 0 )
	ROM_DEFAULT_BIOS( "9733" )
	TRS80M12_BIOS

	ROM_REGION( 0x412, "plds", 0 )
	TRS80M12_PLDS

ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME         PARENT   COMPAT  MACHINE      INPUT      CLASS           INIT        COMPANY              FULLNAME               FLAGS
COMP( 1979, trs80m2,     0,       0,      trs80m2,     trs80m2,   trs80m2_state,  empty_init, "Tandy Radio Shack", "TRS-80 Model II",     MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1982, trs80m16,    trs80m2, 0,      trs80m16,    trs80m2,   trs80m2_state,  empty_init, "Tandy Radio Shack", "TRS-80 Model 16",     MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1982, trs80m12,    trs80m2, 0,      trs80m12,    trs80m12,  trs80m12_state, empty_init, "Tandy Radio Shack", "TRS-80 Model 12",     MACHINE_SUPPORTS_SAVE )
COMP( 1983, trs80m16b,   trs80m2, 0,      trs80m16b,   trs80m16b, trs80m12_state, empty_init, "Tandy Radio Shack", "TRS-80 Model 16B",    MACHINE_SUPPORTS_SAVE )
COMP( 1983, trs80m16bhd, trs80m2, 0,      trs80m16bhd, trs80m16b, trs80m12_state, empty_init, "Tandy Radio Shack", "TRS-80 Model 16B HD", MACHINE_SUPPORTS_SAVE )
COMP( 1985, tandy6k,     trs80m2, 0,      tandy6k,     trs80m16b, trs80m12_state, empty_init, "Tandy Radio Shack", "Tandy 6000",          MACHINE_SUPPORTS_SAVE )
COMP( 1985, tandy6khd,   trs80m2, 0,      tandy6khd,   trs80m16b, trs80m12_state, empty_init, "Tandy Radio Shack", "Tandy 6000 HD",       MACHINE_SUPPORTS_SAVE )
