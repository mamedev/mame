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
        Panel controller CPU: NEC uPD7810 @ 12 MHz
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

    MPC2000XL & MPC2000 Classic: same as 3000, except:
        Dual UART: MB89371A (V53's i8251 is used for panel comms with the 78C10)

    MPC2000 Classic:
        CPU: NEC V53
        Floppy: uPD72068
        SCSI: MB89352
        LCD:
        Dual UART: MB89371A
        (V53's 8251 is used for panel comms here)
        Panel controller CPU: NEC uPD7810 @ 12 MHz
        Sound DSP: L6048

    MPC1000:
        CPU, LCD, UART, panel controller, DSP: SH-3 7712 (HD6417712)

    MPC500:
        CPU, LCD, UART, panel controller, DSP: SH-3 7727 (HD6417727) @ 100 MHz

    MPC2500:
        CPU, LCD, UART, panel controller, DSP: SH-3 7727 (HD6417727) @ 160 MHz

***************************************************************************/

#include "emu.h"
#include "cpu/nec/v53.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "video/hd61830.h"
#include "bus/midi/midi.h"
#include "speaker.h"
#include "screen.h"
#include "emupal.h"

class mpc3000_state : public driver_device
{
public:
	mpc3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_dsp(*this, "dsp")
		, m_mdout(*this, "mdout")
	{ }

	void mpc3000(machine_config &config);

	void init_mpc3000();

private:
	required_device<v53_base_device> m_maincpu;
	required_device<hd61830_device> m_lcdc;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<midi_port_device> m_mdout;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mpc3000_map(address_map &map);
	void mpc3000_io_map(address_map &map);

	DECLARE_READ16_MEMBER(dsp_0008_hack_r);
	DECLARE_WRITE16_MEMBER(dsp_0008_hack_w);
	DECLARE_READ8_MEMBER(dma_memr_cb);
	DECLARE_PALETTE_INIT(mpc3000);
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

WRITE16_MEMBER(mpc3000_state::dsp_0008_hack_w)
{
	// this is related to the DSP's DMA capability.  The DSP
	// connects to the V53's DMA3 channel on both the MPCs and HNG64.
	m_maincpu->dreq3_w(data&0x1);
	m_dsp->l7a1045_sound_w(space,8/2,data,mem_mask);
}


READ16_MEMBER(mpc3000_state::dsp_0008_hack_r)
{
	// read in irq5
	return 0;
}

void mpc3000_state::mpc3000_io_map(address_map &map)
{
	map(0x0060, 0x0067).rw(m_dsp, FUNC(l7a1045_sound_device::l7a1045_sound_r), FUNC(l7a1045_sound_device::l7a1045_sound_w));
	map(0x0068, 0x0069).rw(FUNC(mpc3000_state::dsp_0008_hack_r), FUNC(mpc3000_state::dsp_0008_hack_w));
	map(0x00e0, 0x00e0).rw(m_lcdc, FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x00e2, 0x00e2).rw(m_lcdc, FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
}

READ8_MEMBER(mpc3000_state::dma_memr_cb)
{
	//logerror("dma_memr_cb: offset %x\n", offset);
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

PALETTE_INIT_MEMBER(mpc3000_state, mpc3000)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void mpc3000_state::mpc3000(machine_config &config)
{
	// V53A isn't devcb3 compliant yet.
	//V53A(config, m_maincpu, 16_MHz_XTAL);
	//m_maincpu->set_addrmap(AS_PROGRAM, &mpc3000_state::mpc3000_map);
	//m_maincpu->set_addrmap(AS_IO, &mpc3000_state::mpc3000_io_map);
	device_t *device = nullptr;
	MCFG_DEVICE_ADD("maincpu", V53A, 16_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(mpc3000_map)
	MCFG_DEVICE_IO_MAP(mpc3000_io_map)
	MCFG_V53_DMAU_OUT_HREQ_CB(WRITELINE("maincpu", v53_base_device, hack_w))
	MCFG_V53_DMAU_IN_MEMR_CB(READ8(*this, mpc3000_state, dma_memr_cb))
	MCFG_V53_DMAU_IN_IOR_3_CB(WRITE8("dsp", l7a1045_sound_device, dma_r_cb))
	MCFG_V53_DMAU_OUT_IOW_3_CB(WRITE8("dsp", l7a1045_sound_device, dma_w_cb))

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(80)
	MCFG_SCREEN_UPDATE_DEVICE("lcdc", hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(240, 64)   //6x20, 8x8
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 64-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(mpc3000_state, mpc3000)

	HD61830(config, m_lcdc, 4.9152_MHz_XTAL / 2 / 2);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	//mdin.rxd_handler().set(m_maincpu, FUNC());

	midiout_slot(MIDI_PORT(config, "mdout"));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	L7A1045(config, m_dsp, 16_MHz_XTAL);
	m_dsp->add_route(0, "lspeaker", 1.0);
	m_dsp->add_route(1, "rspeaker", 1.0);
}

static INPUT_PORTS_START( mpc3000 )
INPUT_PORTS_END

ROM_START( mpc3000 )
	ROM_REGION(0x80000, "maincpu", 0)   // V53 code
	ROM_LOAD16_BYTE( "mpc312ls.bin", 0x000000, 0x040000, CRC(d4fb6439) SHA1(555d388ed25f8b85638c325e7d9012eaa271ffa0) )
	ROM_LOAD16_BYTE( "mpc312ms.bin", 0x000001, 0x040000, CRC(80ee0ab9) SHA1(b8855118d59b8f73a3af5ff2e824cdaa0a9f564a) )

	ROM_REGION(0x80000, "subcpu", 0)    // uPD78C10 panel controller code
	ROM_LOAD( "mp3000__op_v1.0.am27c256__id0110.ic602.bin", 0x000000, 0x008000, CRC(b0b783d3) SHA1(a60016184fc07ba00dcc19ba4da60e78aceff63c) )

	ROM_REGION( 0x2000000, "dsp", ROMREGION_ERASE00 )   // sample RAM
ROM_END

void mpc3000_state::init_mpc3000()
{
}

CONS( 1994, mpc3000, 0, 0, mpc3000, mpc3000, mpc3000_state, init_mpc3000, "Akai / Roger Linn", "MPC-3000", MACHINE_NOT_WORKING )
