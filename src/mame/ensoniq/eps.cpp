// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
/***************************************************************************

    eps.cpp - Ensoniq EPS (Classic)

    Hardware:
        68000, HD63450 DMAC, MC68681 DUART, WD1772 FDC
        ES5504 DOC II (25 voices, 13-bit DAC)
        No ES5510 ESP

    Memory map:
        0x000000-0x007fff   QSRAM bottom (32K)
        0x200000-0x20001f   DOC II (ES5504) regs
        0x240000-0x2400ff   DMAC (68450) regs
        0x280001-0x28001f   DUART (68681) regs
        0x2c0001-0x2c0007   Floppy (WD1772) regs
        0x580000-0x5fffff   internal sound RAM (256K words)
        0xc00000-0xc0ffff   OS ROM (64K)
        0xff8000-0xffffff   QSRAM top (32K)

    Interrupts:
        DOC II is on autovector IRQ 1
        DMAC is on autovector IRQ 2
        DUART uses custom vector 0x40 (address 0x100) level 3

    Driver by R. Belmont and Parduz

***************************************************************************/

#include "emu.h"

#include "esqpanel.h"

#include "bus/midi/midi.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/floppy.h"
#include "machine/hd63450.h"
#include "machine/mc68681.h"
#include "machine/wd_fdc.h"
#include "sound/es5506.h"

#include "speaker.h"

#include "formats/esq16_dsk.h"
#include "formats/hxchfe_dsk.h"


//#define VERBOSE 1
#include "logmacro.h"


namespace {

// TODO: explore sharing the common floppy/analog glue with esq5505.cpp.

class eps_state : public driver_device
{
public:
	eps_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_duart(*this, "duart")
		, m_doc(*this, "doc")
		, m_dmac(*this, "mc68450")
		, m_fdc(*this, "wd1772")
		, m_floppy_connector(*this, "wd1772:0")
		, m_panel(*this, "panel")
		, m_mdout(*this, "mdout")
		, m_rom(*this, "osrom")
		, m_ram(*this, "osram")
	{ }

	void eps(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<es5504_device> m_doc;
	required_device<hd63450_device> m_dmac;
	required_device<wd1772_device> m_fdc;
	required_device<floppy_connector> m_floppy_connector;
	required_device<esqpanel_device> m_panel;
	required_device<midi_port_device> m_mdout;
	required_region_ptr<uint16_t> m_rom;
	required_shared_ptr<uint16_t> m_ram;

	uint8_t m_duart_io = 0;
	bool m_floppy_dskchg = false;
	bool m_floppy_is_loaded = false;
	bool m_floppy_is_active = false;
	emu_timer *m_dskchg_reset_timer = nullptr;
	uint16_t m_analog_values[8] = { };

	uint16_t lower_r(offs_t offset);
	void lower_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t analog_r();
	void analog_w(offs_t offset, uint16_t data);

	void duart_output(uint8_t data);
	void doc_irq(int state);

	static void floppy_drives(device_slot_interface &device);
	static void floppy_formats(format_registration &fr);
	void floppy_loaded(bool loaded);
	void floppy_load(floppy_image_device *floppy);
	void floppy_unload(floppy_image_device *floppy);
	void update_floppy_inputs();
	TIMER_CALLBACK_MEMBER(floppy_dskchg_reset);

	void eps_map(address_map &map) ATTR_COLD;
	void doc_map(address_map &map) ATTR_COLD;
	void eps_cpu_space_map(address_map &map) ATTR_COLD;
};

uint16_t eps_state::lower_r(offs_t offset)
{
	offset &= 0x3fff;

	if (!machine().side_effects_disabled() && m_maincpu->get_fc() == 0x6)
		return m_rom[offset];
	else
		return m_ram[offset];
}

void eps_state::lower_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset &= 0x3fff;

	if (m_maincpu->get_fc() != 0x6)
		COMBINE_DATA(&m_ram[offset]);
	else
		LOG("Write to ROM: %04x @ %04x (fc=%x)\n", uint32_t(data), uint32_t(offset), uint32_t(m_maincpu->get_fc()));
}

uint16_t eps_state::analog_r()
{
	const int sel = (BIT(m_duart_io, 6) << 0)  // OP6 = AN0
				  | (BIT(m_duart_io, 4) << 1)  // OP4 = AN1
				  | (BIT(m_duart_io, 5) << 2); // OP5 = AN2
	return m_analog_values[sel] & 0xfff8;
}

void eps_state::analog_w(offs_t offset, uint16_t data)
{
	m_analog_values[offset & 7] = data;
}

void eps_state::duart_output(uint8_t data)
{
	m_duart_io = data;
	m_floppy_is_active = BIT(data, 0); // OP0 = DSEL

	floppy_image_device *floppy = m_floppy_connector->get_device();
	// TODO: verify WD1772 signal conventions; these ones work in the VFX family.
	floppy->ss_w(BIT(data, 1)); // OP1 = SSEL
	floppy->ds_w(m_floppy_is_active ? 0 : 1);
	floppy->mon_w(!m_floppy_is_active);

	update_floppy_inputs();
}

void eps_state::doc_irq(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_1, state);
}

void eps_state::floppy_drives(device_slot_interface &device)
{
	device.option_add_internal("35dd", FLOPPY_35_DD);
}

void eps_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ESQIMG_FORMAT);
	fr.add(FLOPPY_HFE_FORMAT);
}

void eps_state::floppy_loaded(bool loaded)
{
	m_floppy_is_loaded = loaded;

	if (!loaded)
		m_floppy_dskchg = true;

	update_floppy_inputs();
}

void eps_state::floppy_load(floppy_image_device *)
{
	floppy_loaded(true);
}

void eps_state::floppy_unload(floppy_image_device *)
{
	floppy_loaded(false);
}

void eps_state::update_floppy_inputs()
{
	m_duart->ip0_w(m_floppy_is_active && m_floppy_is_loaded); // IP0 = DSTAT
	m_duart->ip1_w(m_floppy_dskchg ? 0 : 1); // IP1 = DSKCH

	if (m_floppy_dskchg && m_floppy_is_loaded && m_dskchg_reset_timer)
		m_dskchg_reset_timer->adjust(attotime::from_nsec(500));
}

TIMER_CALLBACK_MEMBER(eps_state::floppy_dskchg_reset)
{
	m_floppy_dskchg = !m_floppy_is_loaded;
	update_floppy_inputs();
}

void eps_state::machine_start()
{
	m_duart_io = 0;

	floppy_image_device *floppy = m_floppy_connector->get_device();
	m_fdc->set_floppy(floppy);
	floppy->ss_w(0);
	floppy->setup_load_cb(floppy_image_device::load_cb(&eps_state::floppy_load, this));
	floppy->setup_unload_cb(floppy_image_device::unload_cb(&eps_state::floppy_unload, this));
	m_floppy_dskchg = !floppy->exists();

	m_dskchg_reset_timer = timer_alloc(FUNC(eps_state::floppy_dskchg_reset), this);

	save_item(NAME(m_duart_io));
	save_item(NAME(m_floppy_dskchg));
	save_item(NAME(m_floppy_is_loaded));
	save_item(NAME(m_floppy_is_active));
	save_item(NAME(m_analog_values));
}

void eps_state::machine_reset()
{
	floppy_image_device *floppy = m_floppy_connector->get_device();
	if (floppy->exists())
		floppy_load(floppy);
	else
		floppy_unload(floppy);

	duart_output(0);

	// all values are 10 bits, left-justified within 16 bits
	m_analog_values[0] = 0x7fc0; // pitch mod: start in the center
	m_analog_values[1] = 0x0000; // patch select: nothing pressed
	m_analog_values[2] = 0xffc0; // mod wheel: at the bottom, no modulation
	m_analog_values[3] = 0xccc0; // data entry: somewhere in the middle
	m_analog_values[4] = 0xffc0; // control voltage / pedal: full on
	m_analog_values[5] = 0xffc0; // volume control: full on
	m_analog_values[6] = 0x7fc0; // battery voltage
	m_analog_values[7] = 0x5540; // vRef to check battery
}

void eps_state::eps_map(address_map &map)
{
	map(0x000000, 0x007fff).rw(FUNC(eps_state::lower_r), FUNC(eps_state::lower_w));
	map(0x200000, 0x20001f).rw(m_doc, FUNC(es5504_device::read), FUNC(es5504_device::write));
	map(0x240000, 0x2400ff).rw(m_dmac, FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0x280000, 0x28001f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x2c0000, 0x2c0007).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0x580000, 0x5fffff).ram().share("soundram");
	map(0xc00000, 0xc0ffff).rom().region("osrom", 0);
	map(0xff8000, 0xffffff).ram().share("osram");
}

void eps_state::doc_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share("soundram");
}

void eps_state::eps_cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff7, 0xfffff7).r(m_duart, FUNC(mc68681_device::get_irq_vector));
	map(0xfffff5, 0xfffff5).r(m_dmac, FUNC(hd63450_device::iack));
}

void eps_state::eps(machine_config &config)
{
	M68000(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &eps_state::eps_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &eps_state::eps_cpu_space_map);

	MC68681(config, m_duart, 10_MHz_XTAL / 2);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_3);
	m_duart->a_tx_cb().set(m_mdout, FUNC(midi_port_device::write_txd));
	m_duart->b_tx_cb().set(m_panel, FUNC(esqpanel_device::rx_w));
	m_duart->outport_cb().set(FUNC(eps_state::duart_output));
	m_duart->set_clocks(500'000, 500'000, 1'000'000, 1'000'000);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_duart, FUNC(mc68681_device::rx_a_w));
	midiout_slot(MIDI_PORT(config, "mdout"));

	ESQPANEL1X22(config, m_panel);
	m_panel->write_tx().set(m_duart, FUNC(mc68681_device::rx_b_w));
	m_panel->write_analog().set(FUNC(eps_state::analog_w));

	WD1772(config, m_fdc, 8_MHz_XTAL);
	FLOPPY_CONNECTOR(config, m_floppy_connector, eps_state::floppy_drives, "35dd", eps_state::floppy_formats, true);

	HD63450(config, m_dmac, 10_MHz_XTAL, m_maincpu, AS_PROGRAM);
	m_dmac->set_clocks(attotime::from_usec(32), attotime::from_nsec(450), attotime::from_usec(4), attotime::from_hz(15625 / 2));
	m_dmac->set_burst_clocks(attotime::from_usec(32), attotime::from_nsec(450), attotime::from_nsec(50), attotime::from_nsec(50));
	m_dmac->irq_callback().set_inputline(m_maincpu, M68K_IRQ_2);
	m_dmac->dma8_read<0>().set(m_fdc, FUNC(wd1772_device::data_r));
	m_dmac->dma8_write<0>().set(m_fdc, FUNC(wd1772_device::data_w));

	SPEAKER(config, "speaker", 2).front();

	ES5504(config, m_doc, 10_MHz_XTAL);
	m_doc->set_addrmap(0, &eps_state::doc_map);
	m_doc->irq_cb().set(FUNC(eps_state::doc_irq));
	m_doc->read_port_cb().set(FUNC(eps_state::analog_r));
	m_doc->add_route(0, "speaker", 1.0, 0);
	m_doc->add_route(1, "speaker", 1.0, 1);
}

static INPUT_PORTS_START( eps )
INPUT_PORTS_END

ROM_START( eps )
	ROM_REGION16_BE(0x10000, "osrom", 0)
	ROM_LOAD16_BYTE( "eps-l.u26", 0x000001, 0x008000, CRC(382beac1) SHA1(110e31edb03fcf7bbde3e17423b21929e5b32db2) )
	ROM_LOAD16_BYTE( "eps-h.u27", 0x000000, 0x008000, CRC(d8747420) SHA1(460597751386eb5f08465699b61381c4acd78065) )
ROM_END

} // Anonymous namespace


SYST( 1988, eps, 0, 0, eps, eps, eps_state, empty_init, "Ensoniq", "EPS", MACHINE_NOT_WORKING )
