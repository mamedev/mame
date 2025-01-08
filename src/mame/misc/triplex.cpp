// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/************************************************************************************************************

    Torch Triple X

    Known components:
    - 68010 CPU
    - 68451 MMU
    - 68450 DMA
    - 6845 CRTC
    - 6303 MPU
    - 6840 Timer
    - HD146818 RTC
    - 6850 UART
    - NCR5380 SCSI
    - AM7990 LANCE 20MHz
    - 8530 SCC
    - 20MHz XTAL
    - 16MHz XTAL
    - 4.9152MHz XTAL

    TODO:
    - emulate the OMTI 5200 controller board, with floppy.
    - dump keyboard MCU and implement full keyboard, with mouse.

************************************************************************************************************/

#include "emu.h"

#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/keyboard.h"
#include "cpu/m68000/m68010.h"
#include "cpu/m6800/m6801.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/am79c90.h"
#include "machine/hd63450.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/ncr5380.h"
#include "machine/z80scc.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class triplex_state : public driver_device
{
public:
	triplex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dmac(*this, "dmac")
		, m_mpu(*this, "mpu")
		, m_crtc(*this, "crtc")
		, m_kbd(*this, "kbd")
		, m_speaker(*this, "speaker")
		, m_vram(*this, "vram", 0x10000, ENDIANNESS_LITTLE)
		, m_vram_bank(*this, "vram_bank")
		, m_palette(*this, "palette")
		, m_ncr5380(*this, "scsi:7:ncr5380")
		, m_serial_c(*this, "serial_c")
	{
	}

	void triplex(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68010_device> m_maincpu;
	required_device<hd63450_device> m_dmac;
	required_device<hd6303r_cpu_device> m_mpu;
	required_device<mc6845_device> m_crtc;
	required_device<rs232_port_device> m_kbd;
	required_device<speaker_sound_device> m_speaker;
	memory_share_creator<uint8_t> m_vram;
	required_memory_bank m_vram_bank;
	required_device<palette_device> m_palette;
	required_device<ncr5380_device> m_ncr5380;
	required_device<rs232_port_device> m_serial_c;

	void mem_map(address_map &map) ATTR_COLD;
	void mpu_map(address_map &map) ATTR_COLD;

	u16 buserror_r(offs_t offset);
	void buserror_w(offs_t offset, u16 data);

	u8 mpu_p1_r();
	void mpu_p1_w(u8 data);
	u8 mpu_p2_r();
	void mpu_p2_w(u8 data);

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	u8 m_vmode = 0;
	int m_keyclk = 0;
};


void triplex_state::machine_start()
{
	m_vram_bank->configure_entries(0, 2, m_vram, 0x8000);

	save_item(NAME(m_vmode));
}

void triplex_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_vmode = 0;
}


void triplex_state::mpu_map(address_map &map)
{
	map(0x0100, 0x0107).rw("ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x0200, 0x0201).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0300, 0x033f).rw("rtc", FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0x0400, 0x0400).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x0401, 0x0401).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0500, 0x050f).w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x0600, 0x06ff).rw("xbus", FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w));
	map(0x0700, 0x07ff).rw("xbus", FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w));
	map(0x4000, 0x0bfff).bankrw("vram_bank");
	map(0xc000, 0xffff).rom().region("rom", 0);
}


void triplex_state::mem_map(address_map &map)
{
	map(0x000000, 0x00ffff).lrw8(NAME([this](offs_t offset) { return m_vram[offset]; }), NAME([this](offs_t offset, u8 data) { m_vram[offset] = data; }));
	map(0x040000, 0x040007).rw("scc", FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0x00ff);
	//map(0x060000, 0x060000) // IACK
	map(0x080000, 0x08003f).rw("dmac", FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	//map(0x0a0000, 0x0a00ff).rw("mmu", FUNC(mc68451_device::read), FUNC(mc68451_device::write));
	map(0x0c0000, 0x0c0003).rw("lance", FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0x0e0000, 0x0e000f).rw(m_ncr5380, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)).umask16(0xff00);
	map(0x100000, 0x1fffff).ram(); // main memory
	map(0x200000, 0x2fffff).ram(); // limpet board (VMEbus)
	map(0x300000, 0x7fffff).rw(FUNC(triplex_state::buserror_r), FUNC(triplex_state::buserror_w));
}


u16 triplex_state::buserror_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0x100000 + offset*2, true, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xffff;
}

void triplex_state::buserror_w(offs_t offset, u16 data)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0x100000 + offset*2, false, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}


u8 triplex_state::mpu_p1_r()
{
	// P10 FAIL
	return 0x00;
}

void triplex_state::mpu_p1_w(u8 data)
{
	//logerror("%s mpu_p1_w: %02x\n", machine().describe_context(), data);

	// P11 POWERUP

	// P12 VIDSEL
	m_vram_bank->set_entry(BIT(data, 2));

	// P13 PRESET
	m_maincpu->set_input_line(INPUT_LINE_HALT, BIT(data, 3));
	m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3));

	// P14 PROCINT
	//m_dmac->pcl0_w(BIT(data, 4));

	// P15/6 VMODE
	m_vmode = BIT(data, 5, 2);

	// P17 SSDTR
	m_serial_c->write_dtr(BIT(data, 7));
}


u8 triplex_state::mpu_p2_r()
{
	u8 data = 0x00;

	// P20 VSYNC
	data |= m_crtc->vsync_r() << 0;

	// P22 KEYCLK
	data |= m_keyclk << 2;

	// P23 KEYRX
	data |= m_kbd->rxd_r() << 3;

	//logerror("%s mpu_p2_r: %02x\n", machine().describe_context(), data);
	return data;
}

void triplex_state::mpu_p2_w(u8 data)
{
	//logerror("%s mpu_p2_w: %02x\n", machine().describe_context(), data);

	// P21 AUDIO
	m_speaker->level_w(BIT(data, 1));

	// P24 KEYTX
	m_kbd->write_txd(BIT(data, 4));
}


MC6845_UPDATE_ROW(triplex_state::crtc_update_row)
{
	// Mode   Horizontal   Vertical   Bits/pixel   Colours
	//  0        640         256          2           4
	//  1        320         256          4          16
	//  2        640         512          1           2
	//  3        320         512          2           4

	u32 *p = &bitmap.pix(y);
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	u16 offset = (y * 0x100) + 2;

	for (int x_pos = 0; x_pos < (x_count*2); x_pos++)
	{
		u8 data = m_vram[offset + x_pos];

		switch (m_vmode)
		{
		case 0: case 3:
			*p++ = palette[BIT(data, 6, 2)];
			*p++ = palette[BIT(data, 4, 2)];
			*p++ = palette[BIT(data, 2, 2)];
			*p++ = palette[BIT(data, 0, 2)];
			break;
		case 1:
			*p++ = palette[BIT(data, 4, 4)];
			*p++ = palette[BIT(data, 0, 4)];
			break;
		case 2:
			*p++ = palette[BIT(data, 7, 1)];
			*p++ = palette[BIT(data, 6, 1)];
			*p++ = palette[BIT(data, 5, 1)];
			*p++ = palette[BIT(data, 4, 1)];
			*p++ = palette[BIT(data, 3, 1)];
			*p++ = palette[BIT(data, 2, 1)];
			*p++ = palette[BIT(data, 1, 1)];
			*p++ = palette[BIT(data, 0, 1)];
			break;
		}
	}
}


MC6845_ON_UPDATE_ADDR_CHANGED(triplex_state::crtc_update_addr)
{
	//logerror("crtc_update_addr: address %04x strobe %d\n", address, strobe);
}


void triplex_keyboard_devices(device_slot_interface &device)
{
	device.option_add("keyboard", SERIAL_KEYBOARD);
}

static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_1200)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


void triplex_state::triplex(machine_config &config)
{
	M68010(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &triplex_state::mem_map);

	HD63450(config, m_dmac, 16_MHz_XTAL / 2, m_maincpu); // HD68450
	m_dmac->irq_callback().set_inputline(m_maincpu, M68K_IRQ_3);
	m_dmac->dma_read<0>().set(m_ncr5380, FUNC(ncr5380_device::dma_r));
	m_dmac->dma_write<0>().set(m_ncr5380, FUNC(ncr5380_device::dma_w));

	// TODO: MC68451 MMU

	am7990_device &lance(AM7990(config, "lance", 20_MHz_XTAL));
	lance.intr_out().set_inputline(m_maincpu, M68K_IRQ_2);
	lance.dma_in().set([this](offs_t offset) { return m_maincpu->space(AS_PROGRAM).read_word(offset); });
	lance.dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { m_maincpu->space(AS_PROGRAM).write_word(offset, data, mem_mask); });

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk"); // OMTI 5200 (with floppy controller)
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR5380).machine_config([this](device_t *device) {
		downcast<ncr5380_device&>(*device).irq_handler().set_inputline(m_maincpu, M68K_IRQ_4);
		downcast<ncr5380_device&>(*device).drq_handler().set(":dmac", FUNC(hd63450_device::drq0_w));
	});

	scc8530_device &scc(SCC8530N(config, "scc", 4.9152_MHz_XTAL));
	scc.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_5);
	scc.out_wreqa_callback().set("dmac", FUNC(hd63450_device::drq1_w));
	scc.out_dtra_callback().set("dmac", FUNC(hd63450_device::drq2_w));
	scc.out_wreqb_callback().set("dmac", FUNC(hd63450_device::drq3_w));
	scc.out_txda_callback().set("serial_a", FUNC(rs232_port_device::write_txd));
	scc.out_rtsa_callback().set("serial_a", FUNC(rs232_port_device::write_rts));
	scc.out_txdb_callback().set("serial_b", FUNC(rs232_port_device::write_txd));
	scc.out_rtsb_callback().set("serial_b", FUNC(rs232_port_device::write_rts));
	scc.out_dtrb_callback().set("serial_b", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &serial_a(RS232_PORT(config, "serial_a", default_rs232_devices, nullptr)); // X25
	serial_a.rxd_handler().set("scc", FUNC(scc85230_device::rxa_w));
	serial_a.cts_handler().set("scc", FUNC(scc85230_device::ctsa_w));
	serial_a.dcd_handler().set("scc", FUNC(scc85230_device::dcda_w));
	serial_a.rxc_handler().set("scc", FUNC(scc85230_device::rxca_w));

	rs232_port_device &serial_b(RS232_PORT(config, "serial_b", default_rs232_devices, nullptr)); // Terminal or modem
	serial_b.rxd_handler().set("scc", FUNC(scc85230_device::rxb_w));
	serial_b.cts_handler().set("scc", FUNC(scc85230_device::ctsb_w));
	serial_b.dcd_handler().set("scc", FUNC(scc85230_device::dcdb_w));

	// Service Processor
	HD6303R(config, m_mpu, 16_MHz_XTAL / 4);
	m_mpu->set_addrmap(AS_PROGRAM, &triplex_state::mpu_map);
	m_mpu->in_p1_cb().set(FUNC(triplex_state::mpu_p1_r));
	m_mpu->out_p1_cb().set(FUNC(triplex_state::mpu_p1_w));
	m_mpu->in_p2_cb().set(FUNC(triplex_state::mpu_p2_r));
	m_mpu->out_p2_cb().set(FUNC(triplex_state::mpu_p2_w));
	m_mpu->out_ser_tx_cb().set("kbd", FUNC(rs232_port_device::write_txd));

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_mpu, M6801_IRQ1_LINE);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.5);

	RS232_PORT(config, m_kbd, triplex_keyboard_devices, "keyboard");
	m_kbd->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 1024, 0, 720, 312, 0, 256);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	SY6845E(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(triplex_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(triplex_state::crtc_update_addr));
	m_crtc->out_vsync_callback().set_inputline(m_mpu, M6801_TIN_LINE);
	m_crtc->out_vsync_callback().append_inputline(m_maincpu, M68K_IRQ_6);

	PALETTE(config, "palette").set_format(palette_device::RGB_332_inverted, 16);

	mc146818_device &rtc(MC146818(config, "rtc", 32.768_kHz_XTAL));
	rtc.set_24hrs(true);
	rtc.set_binary(true);
	rtc.irq().set("irqs", FUNC(input_merger_device::in_w<0>));

	ptm6840_device &ptm(PTM6840(config, "ptm", 16_MHz_XTAL / 16));
	ptm.set_external_clocks(4.9152_MHz_XTAL/16, 4.9152_MHz_XTAL/16, 4.9152_MHz_XTAL/16);
	ptm.o1_callback().set("acia", FUNC(acia6850_device::write_rxc));
	ptm.o2_callback().set("acia", FUNC(acia6850_device::write_txc));
	ptm.o3_callback().set([this](int state) { m_keyclk = state; if (state) m_mpu->clock_serial(); }); // KEYCLK
	ptm.irq_callback().set("irqs", FUNC(input_merger_device::in_w<1>));

	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.txd_handler().set(m_serial_c, FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set(m_serial_c, FUNC(rs232_port_device::write_rts));
	acia.irq_handler().set("irqs", FUNC(input_merger_device::in_w<2>));

	RS232_PORT(config, m_serial_c, default_rs232_devices, "printer"); // Printer
	m_serial_c->rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	m_serial_c->dcd_handler().set("acia", FUNC(acia6850_device::write_dcd));
	m_serial_c->cts_handler().set("acia", FUNC(acia6850_device::write_cts));

	BBC_1MHZBUS_SLOT(config, "xbus", 16_MHz_XTAL / 16, bbcm_1mhzbus_devices, nullptr); // TODO: modem board

	//SOFTWARE_LIST(config, "flop_list").set_original("triplex_flop");
}


ROM_START(triplex)
	ROM_REGION(0x4000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "13", "Caretaker 1.3")
	ROMX_LOAD("caretaker_iss1.3.bin", 0x0000, 0x4000, CRC(a3031aa1) SHA1(4d0f9ca854bbf236890bb1c342977a493df1e3d4), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "12", "Caretaker 1.2")
	ROMX_LOAD("caretaker_iss1.2.bin", 0x0000, 0x4000, CRC(3b1aa08b) SHA1(92bd672d0e13921cd51a9254031270cd0bb34d73), ROM_BIOS(1))

	//DISK_REGION("scsi:0:harddisk")
	//DISK_IMAGE("torch_system5", 0, NO_DUMP) // HDD images at https://bitsavers.org/bits/Torch/, not verified until machine working.
ROM_END

} // anonymous namespace


//   YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT     CLASS           INIT         COMPANY             FULLNAME     FLAGS
COMP(1985, triplex,  0,      0,      triplex,   0,        triplex_state,  empty_init,  "Torch Computers",  "Triple X",  MACHINE_NOT_WORKING)
