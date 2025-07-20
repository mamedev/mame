// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************************************

Sharp MZ-6500

TODO:
- keyboard (from MCU thru PIO, serial i/f)
- "ERROR 06" during selfcheck (r/w from <reserved> "I/O slot" $f5 port);
- mz5500: "ERROR 12" when testing floppies;
- mz6550: hangs for missing second CTC;
- mz6550: hookup EMM to bus slot (tested in selfcheck, "ERROR 05");

**************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/rp5c01.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"
#include "video/upd7220.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class mz5500_state : public driver_device
{
public:
	mz5500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic_%u", 1U)
		, m_dmac(*this, "dmac")
		, m_ctc(*this, "ctc")
		, m_rtc(*this, "rtc")
		, m_hgdc(*this, "upd7220")
		, m_vram(*this, "videoram")
		, m_palette(*this, "palette")
		, m_user_bank(*this, "user_bank")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_psg(*this, "psg")
	{ }

	void mz5500(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_reset_cb);

protected:
	required_device<cpu_device> m_maincpu;
	required_device_array<pic8259_device, 2>  m_pic;
	required_device<am9517a_device> m_dmac;
	required_device<z80ctc_device> m_ctc;
	required_device<rp5c01_device> m_rtc;
	required_device<upd7220_device> m_hgdc;
	required_shared_ptr<u16> m_vram;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_user_bank;
	required_device<upd765a_device> m_fdc;
	optional_device_array<floppy_connector, 4> m_floppy;
	floppy_image_device *m_current_floppy;
	required_device<ay8912_device> m_psg;

	void io_map(address_map &map) ATTR_COLD;
	virtual void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
private:
	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, u8 data);
	void user_map(address_map &map);

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	void upd7220_map(address_map &map) ATTR_COLD;

	void psg_porta_w(u8 data);

	u8 dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, u8 data);
	void set_dma_channel(int channel, int state);

	u8 m_dma_offset[4];
	u8 m_dma_channel;
};

class mz6500_state : public mz5500_state
{
public:
	mz6500_state(const machine_config &mconfig, device_type type, const char *tag)
		: mz5500_state(mconfig, type, tag)
	{ }

	void mz6500(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
};


class mz6550_state : public mz6500_state
{
public:
	mz6550_state(const machine_config &mconfig, device_type type, const char *tag)
		: mz6500_state(mconfig, type, tag)
	{ }

	void mz6550(machine_config &config);

private:
	virtual void mem_map(address_map &map) override ATTR_COLD;
};

UPD7220_DISPLAY_PIXELS_MEMBER( mz5500_state::hgdc_display_pixels )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	int const gfx[3] = { m_vram[(address + 0x00000)], m_vram[(address + 0x8000)], m_vram[(address + 0x10000)] };

	for(u8 i=0; i<16; i++)
	{
		u8 pen = (BIT(gfx[0], i)) | (BIT(gfx[1], i) << 1) | (BIT(gfx[2], i) << 2);

		bitmap.pix(y, x + i) = palette[pen];
	}
}


u8 mz5500_state::vram_r(offs_t offset)
{
	return m_vram[offset >> 1] >> ((offset & 1) ? 8 : 0);
}

void mz5500_state::vram_w(offs_t offset, u8 data)
{
	int mask = (offset & 1) ? 8 : 0;
	offset >>= 1;
	m_vram[offset] &= 0xff00 >> mask;
	m_vram[offset] |= data << mask;
}

void mz5500_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x9ffff).ram();
	map(0xa0000, 0xbffff).m(m_user_bank, FUNC(address_map_bank_device::amap8));
	map(0xc0000, 0xeffff).rw(FUNC(mz5500_state::vram_r), FUNC(mz5500_state::vram_w));
	map(0xfc000, 0xfffff).rom().region("ipl", 0);
}

void mz5500_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x000f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0010, 0x0013).mirror(0xc).rw("pio", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0020, 0x0021).mirror(0xe).m(m_fdc, FUNC(upd765a_device::map));
	map(0x0030, 0x0033).mirror(0xc).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0040, 0x0043).mirror(0xc).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0050, 0x005f).lw8(NAME([this] (u8 data) {
		// TODO: no way to select different segments?
		for (int ch = 0; ch < 4; ch ++)
			m_dma_offset[ch] = data >> 4;
	}));
	map(0x0060, 0x0061).portr("SYSA");
//  map(0x0070, 0x0070) system port C
//  map(0x00cd, 0x00cd) MZ-1R32
	map(0x0100, 0x0103).mirror(0xc).rw(m_hgdc, FUNC(upd7220_device::read), FUNC(upd7220_device::write)).umask16(0x00ff);
//  map(0x0110, 0x011f) video address / data registers (priority)
//  map(0x0120, 0x012f) video registers
//  map(0x0130, 0x013f) video register
//  map(0x0140, 0x015f) palette pens
//  map(0x0200, 0x020f) z80sio
	// TODO: second ctc at 0x214-0x217, just one for MZ-5500
	map(0x0210, 0x0213).mirror(0x8).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0220, 0x022f).rw(m_rtc, FUNC(rp5c01_device::read), FUNC(rp5c01_device::write));
	map(0x0230, 0x0231).mirror(0xe).rw(m_psg, FUNC(ay8912_device::data_r), FUNC(ay8912_device::data_address_w));
//  map(0x0240, 0x0240) z80ctc vector ack
//  map(0x0250, 0x0250) z80sio vector ack
//  map(0x0270, 0x0270) system port B
}

void mz5500_state::user_map(address_map &map)
{
	map.unmap_value_high();
//  map(0x60000, 0x7ffff) MZ-1R32 EMM
	map(0x80000, 0x9ffff).rom().region("dictionary", 0x20000);
	map(0xa0000, 0xbffff).rom().region("dictionary", 0x00000);
	map(0xc0000, 0xdffff).rom().region("kanji", 0x20000);
	map(0xe0000, 0xfffff).rom().region("kanji", 0x00000);
}

void mz6550_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	mz6500_state::mem_map(map);
	map(0x0f8000, 0x0fffff).rom().region("ipl", 0);
	map(0xff8000, 0xffffff).rom().region("ipl", 0);
}

INPUT_CHANGED_MEMBER(mz5500_state::nmi_reset_cb)
{
	if (newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


// TODO: mz5500 is slightly different
// (SW3-4-5 was originally a drive selection option)
static INPUT_PORTS_START( mz6500 )
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// /RSTSW, front panel
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(mz5500_state::nmi_reset_cb), 0) PORT_NAME("Reset Switch")
	PORT_DIPNAME( 0x04, 0x04, "Display resolution" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x04, "High resolution (400)" )
	PORT_DIPSETTING(    0x00, "Medium resolution (200)" )
	PORT_DIPNAME( 0x08, 0x08, "Selfcheck mode" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// TODO: "fixed to ON" for MZ-5600, but definitely used in MZ-6500
	// MZ-5500 uses these SW in following way:
	// 3 4 5
	// 0 0 0 Standard MFD drive (54B) DT/DD, 1F02,07,09
	// 1 0 0 MZ-80BF
	// * * * <reserved>
	PORT_DIPNAME( 0x10, 0x10, "Floppy density" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x10, "2HD" )
	PORT_DIPSETTING(    0x00, "2D/2DD" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:4") // MZ-5600 "fixed to OFF"
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "CPU clock" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x40, "8 MHz" )
	PORT_DIPSETTING(    0x00, "5 MHz" )
	PORT_DIPNAME( 0x80, 0x00, "Use 8087 numerical coprocessor" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	// SW7 and SW8 at $270, "user-defined dips"
INPUT_PORTS_END


void mz5500_state::machine_start()
{
	m_fdc->set_rate(250000);
	m_current_floppy = nullptr;
}

void mz6500_state::machine_start()
{
	mz5500_state::machine_start();
	m_fdc->set_rate(500000);
}


void mz5500_state::machine_reset()
{
}


static void mz6500_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}

void mz5500_state::upd7220_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share("videoram");
}

uint8_t mz5500_state::dma_read_byte(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dma_channel] << 16) | offset;
	return program.read_byte(addr);
}


void mz5500_state::dma_write_byte(offs_t offset, uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dma_channel] << 16) | offset;
//  printf("%05x %d -> %02x\n", addr, m_dma_channel, data);
	program.write_byte(addr, data);
}

void mz5500_state::set_dma_channel(int channel, int state)
{
	if (!state) m_dma_channel = channel;
}


void mz5500_state::psg_porta_w(u8 data)
{
	// SL0-SL3
	// TODO: error 13 without this for drives B (first time) and C (second time onward)
	// However both msdos211 and cpm86 will be unhappy with this.
	//if ((data & 0xf) == 0)
	//{
	//	m_current_floppy = nullptr;
	//	m_fdc->set_floppy(nullptr);
	//}
	//else
	{
		for (int sl_i = 0; sl_i < 4; sl_i ++)
		{
			// Assume multiselect not doable
			if (BIT(data, sl_i))
			{
				m_current_floppy = m_floppy[sl_i]->get_device();
				m_fdc->set_floppy(m_current_floppy);

				if (m_current_floppy != nullptr)
				{
					// /MOTOR ON
					m_current_floppy->mon_w(BIT(data, 4));
				}
				break;
			}
		}
	}

	// TODO: MA0-MA1 also sends ir7 traps
	// m_pic[0]->ir7_w(BIT(!data, 5));
	// m_pic[1]->ir7_w(BIT(!data, 6));
	m_user_bank->set_bank((data & 0xe0) >> 5);
}


void mz5500_state::mz5500(machine_config &config)
{
	// MZ-5500: 5 MHz
	// MZ-5600 onward: 8 or 5 MHz modes, user settable
	// TODO: clocks for peripherals
	I8086(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mz5500_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mz5500_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_pic[0], FUNC(pic8259_device::inta_cb));

	ADDRESS_MAP_BANK(config, m_user_bank).set_map(&mz5500_state::user_map).set_options(ENDIANNESS_LITTLE, 8, 20, 0x20000);

	PIC8259(config, m_pic[0], 0);
	m_pic[0]->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->read_slave_ack_callback().set(m_pic[1], FUNC(pic8259_device::acknowledge));

	PIC8259(config, m_pic[1], 0);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir6_w));
	m_pic[1]->in_sp_callback().set_constant(0);

	AM9517A(config, m_dmac, 5000000);
	m_dmac->dreq_active_low();
	m_dmac->out_hreq_callback().set([this] (int state) {
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
		m_dmac->hack_w(state);
	});
	m_dmac->out_eop_callback().set([this] (int state) { m_fdc->tc_w(state);});
	m_dmac->in_memr_callback().set(FUNC(mz5500_state::dma_read_byte));
	m_dmac->out_memw_callback().set(FUNC(mz5500_state::dma_write_byte));
	// ch. 0: HDD
	m_dmac->in_ior_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_w));
	// ch. 2: memory refresh (from CTC ZC0)
	// ch. 3: user selectable
	m_dmac->out_dack_callback<0>().set([this] (int state) { set_dma_channel(0, state); });
	m_dmac->out_dack_callback<1>().set([this] (int state) { set_dma_channel(1, state); });
	m_dmac->out_dack_callback<2>().set([this] (int state) { set_dma_channel(2, state); });
	m_dmac->out_dack_callback<3>().set([this] (int state) { set_dma_channel(3, state); });

	i8255_device &pio(I8255A(config, "pio"));
//  pio.out_pa_callback() Data output to centronics (negated)
	/*
	 * x--- ---- FDC motor on status input
	 * -x-- ---- SIO /CI
	 * --x- ---- SIO /CD
	 * ---x ---- Keyboard SRK (output request)
	 * ---- x--- Keyboard DK (data bit)
	 * ---- -x-- Centronics PDTR (select signal)
	 * ---- --x- Centronics /PE
	 * ---- ---x Centronics /BUSY
	 */
	pio.in_pb_callback().set([this] () {
		u8 res = 0;

		if (m_current_floppy != nullptr)
			res |= m_current_floppy->mon_r() << 7;
		return res;
	});
//	pio.in_pc_callback().set([this] () {
//		return 0;
//	});
	/*
	 * -x-- ---- Centronics /ACK input
	 * --x- ---- Centronics /STROBE output
	 * ---- x--- Trap IRQ for Centronics /ACK
	 * ---- -x-- BSC external clock /EXCLK EN
	 * ---- --x- Keyboard STC (strobe) output
	 * ---- ---x Keyboard DC (data bit) output
	 */
	pio.out_pc_callback().set([this] (u8 data) {
		logerror("PIO PORTC: %02x\n", data);
	});

	Z80CTC(config, m_ctc, 5000000 / 2);
	m_ctc->intr_callback().set(m_pic[0], FUNC(pic8259_device::ir5_w));

	RP5C01(config, m_rtc, XTAL(32'768));
	m_rtc->out_alarm_callback().set(m_pic[1], FUNC(pic8259_device::ir0_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("upd7220", FUNC(upd7220_device::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	PALETTE(config, "palette").set_entries(8);

	// TODO: should reach ~56 Hz
	UPD7220(config, m_hgdc, 5000000 / 2);
	m_hgdc->set_addrmap(0, &mz5500_state::upd7220_map);
	m_hgdc->set_display_pixels(FUNC(mz5500_state::hgdc_display_pixels));
	m_hgdc->vsync_wr_callback().set(m_pic[0], FUNC(pic8259_device::ir0_w));

	// wants ready for detecting if disk is in.
	UPD765A(config, m_fdc, 5000000, true, true);
	m_fdc->intrq_wr_callback().set(m_pic[1], FUNC(pic8259_device::ir1_w));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq1_w)).invert();
	FLOPPY_CONNECTOR(config, "fdc:0", mz6500_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", mz6500_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", mz6500_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", mz6500_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("mz5500_flop");

	SPEAKER(config, "mono").front_center();

	// TODO: clock, discrete mixing
	AY8912(config, m_psg, 4000000);
	m_psg->port_a_write_callback().set(FUNC(mz5500_state::psg_porta_w));
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void mz6500_state::mz6500(machine_config &config)
{
	mz5500_state::mz5500(config);
	m_maincpu->set_clock(8000000);

	FLOPPY_CONNECTOR(config.replace(), "fdc:0", mz6500_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config.replace(), "fdc:1", mz6500_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

void mz6550_state::mz6550(machine_config &config)
{
	mz6500_state::mz6500(config);

	// TODO: still 8 MHz?
	I80286(config.replace(), m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mz6550_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mz6550_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_pic[0], FUNC(pic8259_device::inta_cb));
}


// TODO: actual romlabels for all
ROM_START( mz5500 )
	ROM_REGION16_LE( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(e5d47f08) SHA1(40f8463b140c18ebd596618907e78cd9f909e7f4))

	ROM_REGION( 0x40000, "dictionary", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom", 0x00000, 0x40000, CRC(422d8996) SHA1(6854f5f2b8a1975847b8ad85a2e038bc52f96d98))

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x00000, 0x40000, CRC(b618e25d) SHA1(1da93337fecde6c0f8a5bd68f3f0b3222a38d63e))
ROM_END

ROM_START( mz6500 )
	ROM_REGION16_LE( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(6c978ac4) SHA1(7872d7e6d9cda2ed9f47ed4833a5caa4dfe0e55c))

	ROM_REGION( 0x40000, "dictionary", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom", 0x00000, 0x40000, CRC(2df3cfd3) SHA1(d420ede09658c2626b0bb650a063d88b1783e554))

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x00000, 0x40000, CRC(b618e25d) SHA1(1da93337fecde6c0f8a5bd68f3f0b3222a38d63e))
ROM_END

ROM_START( mz6550 )
	ROM_REGION16_LE( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x8000, CRC(7a751f21) SHA1(4f89eb1400c72540c68fddd8ffc12d1161006fc9))

	ROM_REGION( 0x40000, "dictionary", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom", 0x00000, 0x40000,  CRC(2df3cfd3) SHA1(d420ede09658c2626b0bb650a063d88b1783e554))

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x00000, 0x40000, CRC(b618e25d) SHA1(1da93337fecde6c0f8a5bd68f3f0b3222a38d63e))
ROM_END


} // Anonymous namespace



COMP( 1984, mz5500, 0,      0, mz5500, mz6500, mz5500_state, empty_init, "Sharp", "MZ-5500", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// Released as MZ-5600 in U.K.
COMP( 1984, mz6500, mz5500, 0, mz6500, mz6500, mz6500_state, empty_init, "Sharp", "MZ-6500", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, mz6550, mz5500, 0, mz6550, mz6500, mz6550_state, empty_init, "Sharp", "MZ-6550", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
