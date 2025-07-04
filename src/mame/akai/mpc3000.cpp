// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    mpc3000.cpp - Akai / Roger Linn MPC-3000 music workstation
    Skeleton by R. Belmont

    Hardware:
        CPU: NEC V53 (33 MHz?)
             8086-compatible CPU
             8237-compatible DMA controller
             8254-compatible timer
             8259-compatible IRQ controller
        Floppy: uPD72069
        SCSI: MB89352
        LCD: LC7981
        Quad-UART: TE7774
        Panel controller CPU: NEC uPD78C10AGQ @ 12 MHz
        Sound DSP: L7A1045-L6048
            DSP's wavedata bus is 16 bits wide and has 24 address bits

        DMA channel 0 is SCSI, 1 is floppy, 2 is IC31 (some sort of direct-audio stream?), and 3 is the L7A1045 DSP
        IRQ 3 is wire-OR of the 72069 FDC and 89352 SCSI
        IRQ 4 is wire-OR of all 4 TXRDYs on the TE7774
        IRQ 5 is wire-OR of RXRDY1, 2, and 3 on the TE7774
        IRQ 6 is the SMPTE sync in
        IRQ 7 is RXRDY4 on the TE7774

        TE7774 hookups: RXD1 is MIDI IN 1, RXD2 is MIDI IN 2, RXD3 and 4 are wire-ORed to the uPD7810's TX line.
                        TXD1-4 are MIDI OUTs 1, 2, 3, and 4.

    MPC2000XL &  Classic:
        CPU: NEC V53
        Floppy: uPD72068
        SCSI: MB89352
        LCD:
        Dual UART: MB89371A
        (V53's 8251 is used for panel comms here)
        Panel controller CPU: NEC uPD7810 @ 12 MHz
        Sound DSP: L6048

MPCs on other hardware:

    MPC4000:
        CPU, LCD, panel controller: SA1110 StrongARM @ 176 MHz
        SCSI: FAS236 (same as NCR 53CF96)
        IDE, UART, DMA controller: FPGA XC2S100-5TQ144C
        USB Host: SL811HST
        USB function controller: NET2890
        DSP, MIDI: MB87L185PFVS-G-BND (gate array?)

    MPC1000:
        CPU, LCD, UART, panel controller, DSP: SH-3 7712 (HD6417712)

    MPC500:
        CPU, LCD, UART, panel controller, DSP: SH-3 7727 (HD6417727) @ 100 MHz

    MPC2500:
        CPU, LCD, UART, panel controller, DSP: SH-3 7727 (HD6417727) @ 160 MHz

***************************************************************************/

#include "emu.h"
#include "cpu/nec/v5x.h"
#include "cpu/upd7810/upd7810.h"
#include "imagedev/floppy.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "video/hd61830.h"
#include "bus/midi/midi.h"
#include "bus/nscsi/devices.h"
#include "speaker.h"
#include "screen.h"
#include "emupal.h"
#include "machine/74259.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/mb87030.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"

class mpc3000_state : public driver_device
{
public:
	mpc3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_lcdc(*this, "lcdc")
		, m_dsp(*this, "dsp")
		, m_mdout(*this, "mdout")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
	{ }

	void mpc3000(machine_config &config);

	void init_mpc3000();

private:
	required_device<v53a_device> m_maincpu;
	required_device<upd7810_device> m_subcpu;
	required_device<hd61830_device> m_lcdc;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<midi_port_device> m_mdout;
	required_device<upd72069_device> m_fdc;
	required_device<floppy_connector> m_floppy;

	static void floppies(device_slot_interface &device);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mpc3000_map(address_map &map) ATTR_COLD;
	void mpc3000_io_map(address_map &map) ATTR_COLD;
	void mpc3000_sub_map(address_map &map) ATTR_COLD;

	uint16_t dsp_0008_hack_r();
	void dsp_0008_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dma_memr_cb(offs_t offset);
	void dma_memw_cb(offs_t offset, uint16_t data);
	void mpc3000_palette(palette_device &palette) const;
};

void mpc3000_state::machine_start()
{
}

void mpc3000_state::machine_reset()
{
}

void mpc3000_state::mpc3000_map(address_map &map)
{
	map(0x000000, 0x07ffff).mirror(0x80000).rom().region("maincpu", 0);
	map(0x300000, 0x3fffff).ram();
	map(0x500000, 0x500fff).ram(); // actually 8-bit battery-backed RAM
}

void mpc3000_state::dsp_0008_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// this is related to the DSP's DMA capability.  The DSP
	// connects to the V53's DMA3 channel on both the MPCs and HNG64.
	m_maincpu->dreq_w<3>(data&0x1);
	m_dsp->l7a1045_sound_w(8/2,data,mem_mask);
}


uint16_t mpc3000_state::dsp_0008_hack_r()
{
	// read in irq5
	return 0;
}

void mpc3000_state::mpc3000_io_map(address_map &map)
{
	map(0x0000, 0x0000).w("loledlatch", FUNC(hc259_device::write_nibble_d3));
	map(0x0020, 0x0020).w("hiledlatch", FUNC(hc259_device::write_nibble_d3));
	map(0x0060, 0x0067).rw(m_dsp, FUNC(l7a1045_sound_device::l7a1045_sound_r), FUNC(l7a1045_sound_device::l7a1045_sound_w));
	map(0x0068, 0x0069).rw(FUNC(mpc3000_state::dsp_0008_hack_r), FUNC(mpc3000_state::dsp_0008_hack_w));
	map(0x0080, 0x0087).rw("dioexp", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x00a0, 0x00bf).m("scsi:7:spc", FUNC(mb89352_device::map)).umask16(0x00ff);
	//map(0x00c0, 0x00c7).rw("sio", FUNC(te7774_device::read0), FUNC(te7774_device::write0)).umask16(0x00ff);
	//map(0x00c8, 0x00cf).rw("sio", FUNC(te7774_device::read1), FUNC(te7774_device::write1)).umask16(0x00ff);
	//map(0x00d0, 0x00d7).rw("sio", FUNC(te7774_device::read2), FUNC(te7774_device::write2)).umask16(0x00ff);
	//map(0x00d8, 0x00df).rw("sio", FUNC(te7774_device::read3), FUNC(te7774_device::write3)).umask16(0x00ff);
	map(0x00e0, 0x00e0).rw(m_lcdc, FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x00e2, 0x00e2).rw(m_lcdc, FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
	map(0x00e8, 0x00eb).m(m_fdc, FUNC(upd72069_device::map)).umask16(0x00ff);
	map(0x00f0, 0x00f7).rw("synctmr", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	map(0x00f8, 0x00ff).rw("adcexp", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
}

uint16_t mpc3000_state::dma_memr_cb(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset << 1);
}

void mpc3000_state::dma_memw_cb(offs_t offset, uint16_t data)
{
	m_maincpu->space(AS_PROGRAM).write_word(offset << 1, data);
}

void mpc3000_state::mpc3000_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("subcpu", 0);
}

void mpc3000_state::mpc3000_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void mpc3000_state::floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

void mpc3000_state::mpc3000(machine_config &config)
{
	V53A(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpc3000_state::mpc3000_map);
	m_maincpu->set_addrmap(AS_IO, &mpc3000_state::mpc3000_io_map);
	m_maincpu->out_hreq_cb().set(m_maincpu, FUNC(v53a_device::hack_w));
	m_maincpu->in_mem16r_cb().set(FUNC(mpc3000_state::dma_memr_cb));
	m_maincpu->out_mem16w_cb().set(FUNC(mpc3000_state::dma_memw_cb));
	m_maincpu->out_eop_cb().set("tc", FUNC(input_merger_device::in_w<0>));
	m_maincpu->in_ior_cb<0>().set("scsi:7:spc", FUNC(mb89352_device::dma_r));
	m_maincpu->out_iow_cb<0>().set("scsi:7:spc", FUNC(mb89352_device::dma_w));
	m_maincpu->out_dack_cb<1>().set("tc", FUNC(input_merger_device::in_w<1>));
	m_maincpu->in_ior_cb<1>().set(m_fdc, FUNC(upd72069_device::dma_r));
	m_maincpu->out_iow_cb<1>().set(m_fdc, FUNC(upd72069_device::dma_w));
	m_maincpu->in_io16r_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_r16_cb));
	m_maincpu->out_io16w_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_w16_cb));
	m_maincpu->set_tclk(4'000'000); // FIXME: DAWCK generated by DSP (also tied to V53 DSR input)
	m_maincpu->tout_handler<0>().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_maincpu->tout_handler<1>().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_maincpu->tout_handler<2>().set_inputline(m_maincpu, INPUT_LINE_IRQ2);

	// HC02 gates
	INPUT_MERGER_ANY_HIGH(config, "intp3").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
	INPUT_MERGER_ALL_HIGH(config, "tc").output_handler().set(m_fdc, FUNC(upd72069_device::tc_line_w));

	hc259_device &loledlatch(HC259(config, "loledlatch"));
	loledlatch.q_out_cb<0>().set_output("led0").invert(); // Edit Loop
	loledlatch.q_out_cb<1>().set_output("led1").invert(); // Simul Seq
	loledlatch.q_out_cb<2>().set_output("led2").invert(); // Transpose
	loledlatch.q_out_cb<3>().set_output("led3").invert(); // Wait For
	loledlatch.q_out_cb<4>().set_output("led4").invert(); // Count In
	loledlatch.q_out_cb<5>().set_output("led5").invert(); // Auto Punch
	loledlatch.q_out_cb<6>().set_output("led6").invert(); // Rec
	loledlatch.q_out_cb<7>().set_output("led7").invert(); // Over Dub

	hc259_device &hiledlatch(HC259(config, "hiledlatch"));
	hiledlatch.q_out_cb<0>().set_output("led8").invert(); // Play
	hiledlatch.q_out_cb<1>().set_output("led9").invert(); // Bank A
	hiledlatch.q_out_cb<2>().set_output("led10").invert(); // Bank B
	hiledlatch.q_out_cb<3>().set_output("led11").invert(); // Bank C
	hiledlatch.q_out_cb<4>().set_output("led12").invert(); // Bank D
	hiledlatch.q_out_cb<5>().set_output("led13").invert(); // Full Level
	hiledlatch.q_out_cb<6>().set_output("led14").invert(); // 16 Levels
	hiledlatch.q_out_cb<7>().set_output("led15").invert(); // After

	UPD78C10(config, m_subcpu, 12_MHz_XTAL);
	m_subcpu->set_addrmap(AS_PROGRAM, &mpc3000_state::mpc3000_sub_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(80);
	screen.set_screen_update("lcdc", FUNC(hd61830_device::screen_update));
	screen.set_size(240, 64);   //6x20, 8x8
	screen.set_visarea(0, 240-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(mpc3000_state::mpc3000_palette), 2);

	UPD72069(config, m_fdc, 16_MHz_XTAL); // clocked by V53 CLKOUT (TODO: upd72069 supports motor control)
	m_fdc->intrq_wr_callback().set("intp3", FUNC(input_merger_device::in_w<0>));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v53a_device::dreq_w<1>));

	FLOPPY_CONNECTOR(config, m_floppy, mpc3000_state::floppies, "35hd", floppy_image_device::default_mfm_floppy_formats);

	pit8254_device &pit(PIT8254(config, "synctmr", 0)); // MB89254
	pit.set_clk<0>(16_MHz_XTAL / 4);
	pit.set_clk<1>(16_MHz_XTAL / 4);
	pit.set_clk<2>(16_MHz_XTAL / 4);

	I8255(config, "adcexp"); // MB89255B
	I8255(config, "dioexp"); // MB89255B

	HD61830(config, m_lcdc, 4.9152_MHz_XTAL / 2 / 2); // LC7981

	//TE7774(config, "sio", 16_MHz_XTAL / 4);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	//mdin.rxd_handler().set(m_maincpu, FUNC());

	midiout_slot(MIDI_PORT(config, "mdout"));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("spc", MB89352).machine_config(
		[this](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(16_MHz_XTAL / 2);
			spc.out_irq_callback().set(":intp3", FUNC(input_merger_device::in_w<1>));
			spc.out_dreq_callback().set(m_maincpu, FUNC(v53a_device::dreq_w<0>));
		});

	SPEAKER(config, "speaker", 2).front();

	L7A1045(config, m_dsp, 16_MHz_XTAL);
	m_dsp->add_route(0, "speaker", 1.0, 0);
	m_dsp->add_route(1, "speaker", 1.0, 1);
}

static INPUT_PORTS_START( mpc3000 )
INPUT_PORTS_END

ROM_START( mpc3000 )
	ROM_REGION(0x80000, "maincpu", 0)   // V53 code
	ROM_SYSTEM_BIOS(0, "default", "ver 3.12")
	ROMX_LOAD( "mpc312ls.bin", 0x000000, 0x040000, CRC(d4fb6439) SHA1(555d388ed25f8b85638c325e7d9012eaa271ffa0), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "mpc312ms.bin", 0x000001, 0x040000, CRC(80ee0ab9) SHA1(b8855118d59b8f73a3af5ff2e824cdaa0a9f564a), ROM_SKIP(1) | ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "vailixi", "ver 3.50")
	ROMX_LOAD( "3.50 vailixi lse.bin", 0x000000, 0x040000, CRC(9f3031b5) SHA1(c270a92f8ed273a1ede16388bb8f30c85ac1faab), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "3.50 vailixi mso.bin", 0x000001, 0x040000, CRC(e62ebb26) SHA1(f6d080481de40bea2c94bbc96b222c5f9b7afaf4), ROM_SKIP(1) | ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "v311", "ver 3.11")
	ROMX_LOAD( "mpc311ls.bin", 0x000000, 0x040000, CRC(5a272061) SHA1(87cc8aef3233d95ad1febbd77b42f720d718baa3), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "mpc311ms.bin", 0x000001, 0x040000, CRC(a8e177de) SHA1(b63a5c27066c03f7de56659aff183f15d95277c5), ROM_SKIP(1) | ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(3, "v310", "ver 3.10")
	ROMX_LOAD( "mpc3000__ls__v3.10.am27c020__id0197.ic13_ls.bin", 0x000000, 0x040000, CRC(cbd1b3a6) SHA1(5464a57137549d9d9c47f9aafc2b89f4c0af8b31), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "mpc3000__ms__v3.10.am27c020__id0197.ic14_ms.bin", 0x000001, 0x040000, CRC(e2ba1904) SHA1(27a9f047c63964fac2b453f2317b77834490983d), ROM_SKIP(1) | ROM_BIOS(3) )

	ROM_REGION(0x8000, "subcpu", 0)    // uPD78C10 panel controller code
	ROM_LOAD( "mp3000__op_v1.0.am27c256__id0110.ic602.bin", 0x000000, 0x008000, CRC(b0b783d3) SHA1(a60016184fc07ba00dcc19ba4da60e78aceff63c) )

	ROM_REGION( 0x2000000, "dsp", ROMREGION_ERASE00 )   // sample RAM
ROM_END

void mpc3000_state::init_mpc3000()
{
}

CONS( 1994, mpc3000, 0, 0, mpc3000, mpc3000, mpc3000_state, init_mpc3000, "Akai / Roger Linn", "MPC-3000", MACHINE_NOT_WORKING )
