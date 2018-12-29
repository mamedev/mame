// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Fortune 32:16.

    Also known as the Micromega 32 in France (distributed by Thomson).

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/8x300/8x300.h"
//#include "imagedev/floppy.h"
#include "machine/bankdev.h"
//#include "machine/com8116.h"
#include "machine/upd765.h"
#include "machine/x2212.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "video/mc6845.h"
#include "screen.h"


class fs3216_state : public driver_device
{
public:
	fs3216_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_clb(*this, "clb")
		, m_ctc(*this, "ctc")
		, m_fdc(*this, "fdc")
		, m_earom(*this, "earom")
	{
	}

	void fs3216(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	MC6845_UPDATE_ROW(update_row);

	DECLARE_READ16_MEMBER(mmu_read);
	DECLARE_WRITE16_MEMBER(mmu_write);
	DECLARE_WRITE_LINE_MEMBER(mmu_reset_w);
	void mmu_init_w(u16 data);

	DECLARE_READ8_MEMBER(ctc_r);
	DECLARE_WRITE8_MEMBER(ctc_w);
	void floppy_select_w(u8 data);
	u8 floppy_status_r();
	void fdc_reset_w(u16 data);
	u8 fdc_ram_r(offs_t offset);
	void fdc_ram_w(offs_t offset, u8 data);
	u16 earom_recall_r();
	u16 earom_store_r();

	void main_map(address_map &map);
	void clb_map(address_map &map);
	void wdcpu_prog_map(address_map &map);
	void wdcpu_bank_map(address_map &map);

	required_device<m68000_device> m_maincpu;
	required_device<address_map_bank_device> m_clb;
	required_device<z80ctc_device> m_ctc;
	required_device<upd765a_device> m_fdc;
	required_device<x2212_device> m_earom;

	std::unique_ptr<u8[]> m_fdc_ram;

	bool m_in_reset;
	bool m_mmu_init;
};


void fs3216_state::machine_start()
{
	m_fdc_ram = make_unique_clear<u8[]>(0x400);
	save_pointer(NAME(m_fdc_ram), 0x400);

	save_item(NAME(m_in_reset));
	save_item(NAME(m_mmu_init));
}

void fs3216_state::machine_reset()
{
	m_in_reset = true;
	m_mmu_init = false;

	// FIXME: fix the 68000 so that it doesn't read vectors during device_reset
	m_maincpu->reset();
}


MC6845_UPDATE_ROW(fs3216_state::update_row)
{
}


READ16_MEMBER(fs3216_state::mmu_read)
{
	if (m_in_reset)
	{
		if (m_mmu_init && !BIT(offset, 22) && !machine().side_effects_disabled())
			m_in_reset = false;
		else
			offset = (offset & 0x03ffff) | 0x1c0000;
	}

	// TODO: MMU segments
	return m_clb->read16(space, offset & 0x1fffff, mem_mask);
}

WRITE16_MEMBER(fs3216_state::mmu_write)
{
	if (m_in_reset)
	{
		if (m_mmu_init && !BIT(offset, 22) && !machine().side_effects_disabled())
			m_in_reset = false;
		else
			offset = (offset & 0x03ffff) | 0x1c0000;
	}

	// TODO: MMU segments
	m_clb->write16(space, offset & 0x1fffff, data, mem_mask);
}

WRITE_LINE_MEMBER(fs3216_state::mmu_reset_w)
{
	if (state)
	{
		m_in_reset = true;
		m_mmu_init = false;
	}
}

void fs3216_state::mmu_init_w(u16 data)
{
	m_mmu_init = true;
}

READ8_MEMBER(fs3216_state::ctc_r)
{
	return m_ctc->read(space, offset >> 1);
}

WRITE8_MEMBER(fs3216_state::ctc_w)
{
	m_ctc->write(space, offset >> 1, data);
}

u16 fs3216_state::earom_recall_r()
{
	if (!machine().side_effects_disabled())
	{
		m_earom->recall(1);
		m_earom->recall(0);
	}
	return 0xffff;
}

u16 fs3216_state::earom_store_r()
{
	if (!machine().side_effects_disabled())
	{
		m_earom->store(1);
		m_earom->store(0);
	}
	return 0xffff;
}

void fs3216_state::floppy_select_w(u8 data)
{
}

u8 fs3216_state::floppy_status_r()
{
	return 0xff;
}

void fs3216_state::fdc_reset_w(u16 data)
{
	m_fdc->soft_reset();
}

u8 fs3216_state::fdc_ram_r(offs_t offset)
{
	return m_fdc_ram[offset];
}

void fs3216_state::fdc_ram_w(offs_t offset, u8 data)
{
	m_fdc_ram[offset] = data;
}


void fs3216_state::main_map(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(fs3216_state::mmu_read), FUNC(fs3216_state::mmu_write));
}

void fs3216_state::clb_map(address_map &map)
{
	map(0x000000, 0x017fff).ram();
	map(0x380000, 0x383fff).rom().region("momrom", 0);
	map(0x392000, 0x392003).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x392041, 0x392041).w(FUNC(fs3216_state::floppy_select_w));
	map(0x392051, 0x392051).r(FUNC(fs3216_state::floppy_status_r));
	map(0x394680, 0x39468f).rw(FUNC(fs3216_state::ctc_r), FUNC(fs3216_state::ctc_w)).umask16(0x00ff);
	map(0x394701, 0x394701).rw("dart", FUNC(z80dart_device::da_r), FUNC(z80dart_device::da_w));
	map(0x394709, 0x394709).rw("dart", FUNC(z80dart_device::ca_r), FUNC(z80dart_device::ca_w));
	map(0x394711, 0x394711).rw("dart", FUNC(z80dart_device::db_r), FUNC(z80dart_device::db_w));
	map(0x394719, 0x394719).rw("dart", FUNC(z80dart_device::cb_r), FUNC(z80dart_device::cb_w));
	map(0x3f5000, 0x3f5001).w(FUNC(fs3216_state::mmu_init_w));
	map(0x3f6000, 0x3f6001).w(FUNC(fs3216_state::fdc_reset_w));
	map(0x3f6800, 0x3f6fff).rw(FUNC(fs3216_state::fdc_ram_r), FUNC(fs3216_state::fdc_ram_w)).umask16(0x00ff);
	map(0x3f7000, 0x3f7001).r(FUNC(fs3216_state::earom_recall_r));
	map(0x3f7200, 0x3f7201).r(FUNC(fs3216_state::earom_store_r));
	map(0x3f7400, 0x3f75ff).rw(m_earom, FUNC(x2212_device::read), FUNC(x2212_device::write)).umask16(0x00ff);
}

void fs3216_state::wdcpu_prog_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("wdcpu", 0);
}

void fs3216_state::wdcpu_bank_map(address_map &map)
{
	map(0x000, 0x000).nopr();
}


void fs3216_state::fs3216(machine_config &config)
{
	M68000(config, m_maincpu, 44.2368_MHz_XTAL / 8); // 5.5 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &fs3216_state::main_map);
	m_maincpu->set_reset_callback(write_line_delegate(FUNC(fs3216_state::mmu_reset_w), this));

	ADDRESS_MAP_BANK(config, m_clb);
	m_clb->set_addrmap(0, &fs3216_state::clb_map);
	m_clb->set_data_width(16);
	m_clb->set_addr_width(24);
	m_clb->set_endianness(ENDIANNESS_BIG);

	Z80CTC(config, m_ctc, 44.2368_MHz_XTAL / 8); // Z8430BPS
	m_ctc->set_clk<0>(44.2368_MHz_XTAL / 16); // CLK0 rate guessed
	m_ctc->set_clk<1>(44.2368_MHz_XTAL / 16); // CLK1 rate guessed
	m_ctc->zc_callback<0>().set("dart", FUNC(z80dart_device::rxca_w));
	m_ctc->zc_callback<0>().append("dart", FUNC(z80dart_device::txca_w));
	m_ctc->zc_callback<1>().set("dart", FUNC(z80dart_device::rxtxcb_w));

	Z80DART(config, "dart", 44.2368_MHz_XTAL / 8); // Z8470BPS

	UPD765A(config, m_fdc, 16_MHz_XTAL / 2, true, false);

	X2212(config, m_earom);

	mc6845_device &crtc(MC6845(config, "crtc", 14.58_MHz_XTAL / 10)); // HD46505RP; clock unknown
	crtc.set_char_width(10); // unknown
	crtc.set_show_border_area(false);
	crtc.set_update_row_callback(FUNC(fs3216_state::update_row), this);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.58_MHz_XTAL, 900, 0, 800, 270, 0, 250); // parameters guessed
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	n8x300_cpu_device &wdcpu(N8X300(config, "wdcpu", 20_MHz_XTAL / 2)); // N8X305I
	wdcpu.set_addrmap(AS_PROGRAM, &fs3216_state::wdcpu_prog_map);
	wdcpu.set_addrmap(AS_IO, &fs3216_state::wdcpu_bank_map);
}


static INPUT_PORTS_START(fs3216)
INPUT_PORTS_END


// XTALs on Mother Board (1001176-01 Rev. 5): 44.2368 MHz (22A), 16.000 MHz (2J)
// XTALs on Comm A board (1000014-01 2 Port / 1000171-01 4 Port Rev. 7; 10000065-01 Rev. 3): two K1135CM Dual Baud Rate Generators (7D, 8D)
// XTALs on Comm B board (1001651-01 Rev. G): none
// XTALs on Video Controller board (1000443-1 Rev. I): 14.580 MHz (1H)
// XTALs on WD-1001 CLB Disk Controller board (1473-008): 20.000 (Y1), 8.00? [somewhat defaced] (Y2)
ROM_START(fs3216)
	ROM_REGION16_BE(0x4000, "momrom", 0)
	ROM_LOAD16_BYTE("17k_1260-02_h.bin", 0x0000, 0x2000, CRC(75ed6de8) SHA1(0360548493b778995ae436da475b6356945e1872))
	ROM_LOAD16_BYTE("15k_1260-01_l.bin", 0x0001, 0x2000, CRC(82695233) SHA1(0d69309f41306298bf6a4ba6928c53f908bb3f2c))

	ROM_REGION16_BE(0x2000, "comm_a", 0)
	ROM_LOAD16_BYTE("1896-01_c90c3cb92588a2b4bb28bcf4bb8e2023.bin", 0x0000, 0x1000, CRC(ac4cdbd2) SHA1(e448a01a9809cccfb526ac1d4e97d9be3af1e5eb))
	ROM_LOAD16_BYTE("1895-01_fb20aa682a17028cdae2687fc47daef1.bin", 0x0001, 0x1000, CRC(82ebffb5) SHA1(3888b7ba07d0b25bfb9e0444215d4fa9ecd66273))

	ROM_REGION16_BE(0x2000, "comm_b", 0)
	ROM_LOAD16_BYTE("1658-04_b99b2d6b67222a571cc9879f98f2136f.bin", 0x0000, 0x1000, CRC(18b43218) SHA1(ded1419185693350ed2d5868819b0db0c2917ff3))
	ROM_LOAD16_BYTE("1658-03_8425b4008a0fa9092158eb4683110e0f.bin", 0x0001, 0x1000, CRC(83bcdf34) SHA1(782cc58b179b0a42dfe9e09f465582b1420c0c4a))

	ROM_REGION16_BE(0x2000, "video", 0)
	ROM_LOAD16_BYTE("1148-02_3dda8d9a72db50bfc8c2eab697b14952.bin", 0x0000, 0x1000, CRC(ce8f42a4) SHA1(4cfb967890de069270a068b4d4f1f5cf6a9a4c7b))
	ROM_LOAD16_BYTE("1148-01_da087b69caa08bf7c8433a2e089143ce.bin", 0x0001, 0x1000, CRC(72f4c435) SHA1(679fab926b45c99ab19baa75d0d7002d4a5d9299))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("370-01_baf9a45321f489df3ee41175f29105b6.bin", 0x0000, 0x1000, CRC(4d3f1c2a) SHA1(6b5b03757ea53a39675ef442d090e5608bd659c3))

	ROM_REGION16_BE(0x2000, "wd1001_clb", 0)
	ROM_LOAD16_BYTE("u29_1139-02_d54c89bf6a84505b3b9ae05f06597c8d.bin", 0x0000, 0x1000, CRC(396b709a) SHA1(dc1ddef8a16c0529bf76fcd5933ba52e0409e3f9))
	ROM_LOAD16_BYTE("u28_1139-01_3b99c99f03cb4788cc0142e7a9497cda.bin", 0x0001, 0x1000, CRC(c42b7678) SHA1(bec25327cf0bcc8edcb09605cbb609b5708b89f6))

	ROM_REGION16_LE(0x800, "wdcpu", 0) // 2x N82S181N
	ROM_LOAD16_BYTE("u35_ap2001r4m_a794e8b07817734303ede17f38a91e0b_ms2012.bin", 0x000, 0x400, CRC(833a60c9) SHA1(f6414623fc52d030df8814befda02928e2ac5771))
	ROM_LOAD16_BYTE("u53_ap2000r4l_9b8ee868fd1129000b3168350114b6da.bin", 0x001, 0x400, CRC(91b21c9b) SHA1(87fb5b1d5804f771782ceab74fd8d7c97e189bc7))

	ROM_REGION(0x400, "wd1001_prom", 0) // N82S181N (address decoding?)
	ROM_LOAD("u26_ap2002r4f_b3ae6f8966230689d34c82b3c9d817ac.bin", 0x000, 0x400, CRC(fcd31bff) SHA1(ae34c6eb6659dc992896b388be1badfab0fd7971))
ROM_END

COMP(1982, fs3216, 0, 0, fs3216, fs3216, fs3216_state, empty_init, "Fortune Systems", "Fortune 32:16", MACHINE_IS_SKELETON)
