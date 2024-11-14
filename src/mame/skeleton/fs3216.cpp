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
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
//#include "machine/com8116.h"
#include "machine/upd765.h"
#include "machine/x2212.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

class fs3216_state : public driver_device
{
public:
	fs3216_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_clb(*this, "clb")
		, m_ctc(*this, "ctc")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_earom(*this, "earom")
		, m_vecprom(*this, "vecprom")
		, m_videoram(*this, "videoram")
		, m_chargen(*this, "chargen")
	{
	}

	void fs3216(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(crt_update_row);

	void mmu_reg_w(offs_t offset, u16 data);
	u16 mmu_read(offs_t offset, u16 mem_mask);
	void mmu_write(offs_t offset, u16 data, u16 mem_mask);
	void mmu_reset_w(int state);
	void mmu_init_w(u16 data);

	u16 irq_r();
	u8 intack_r(offs_t offset);

	u8 ctc_r(offs_t offset);
	void ctc_w(offs_t offset, u8 data);
	u16 earom_recall_r();
	u16 earom_store_r();

	void fdc_int_w(int state);
	void fdc_drq_w(int state);
	void fdc_hdl_w(int state);
	void floppy_idx_w(int state);
	void fdc_us_w(u8 data);
	u16 floppy_select_r(offs_t offset);
	void floppy_select_w(offs_t offset, u16 data);
	void floppy_control_w(u8 data);
	u8 floppy_status_r();
	TIMER_CALLBACK_MEMBER(fdc_dma);
	u8 fdc_ram_r(offs_t offset);
	void fdc_ram_w(offs_t offset, u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void clb_map(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;
	void wdcpu_prog_map(address_map &map) ATTR_COLD;
	void wdcpu_bank_map(address_map &map) ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	required_device<address_map_bank_device> m_clb;
	required_device<z80ctc_device> m_ctc;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<x2212_device> m_earom;
	required_region_ptr<u8> m_vecprom;

	required_shared_ptr<u16> m_videoram;
	required_region_ptr<u8> m_chargen;

	u32 m_mmu_reg[4]{};
	bool m_from_reset = false;

	u8 m_floppy_status = 0;
	u8 m_floppy_control = 0;
	u8 m_floppy_select = 0;
	u8 m_fdc_select = 0;
	u16 m_fdc_dma_count = 0;
	emu_timer *m_fdc_dma_timer = nullptr;
	std::unique_ptr<u8[]> m_fdc_ram;
};


void fs3216_state::machine_start()
{
	m_fdc_ram = make_unique_clear<u8[]>(0x800);
	save_pointer(NAME(m_fdc_ram), 0x800);

	std::fill(std::begin(m_mmu_reg), std::end(m_mmu_reg), 0);

	m_floppy_status = 0x80;
	m_floppy_control = 0;
	m_floppy_select = 0;
	m_fdc_select = 0;
	m_fdc_dma_count = 0;

	m_fdc_dma_timer = timer_alloc(FUNC(fs3216_state::fdc_dma), this);

	save_item(NAME(m_mmu_reg));
	save_item(NAME(m_from_reset));
	save_item(NAME(m_floppy_status));
	save_item(NAME(m_floppy_control));
	save_item(NAME(m_floppy_select));
	save_item(NAME(m_fdc_select));
	save_item(NAME(m_fdc_dma_count));
}

void fs3216_state::machine_reset()
{
	m_from_reset = true;

	floppy_control_w(0);
	floppy_select_w(0, 0);
}


MC6845_UPDATE_ROW(fs3216_state::crt_update_row)
{
	u32 *px = &bitmap.pix(y);

	for (int i = 0; i < x_count; i++)
	{
		u16 chr = m_videoram[(ma + i) & 0x7ff];
		rgb_t fg = BIT(chr, 13) ? rgb_t::white() : rgb_t(0xc0, 0xc0, 0xc0);
		rgb_t bg = rgb_t::black();

		u16 dots = m_chargen[(chr & 0xff) << 4 | ra] << 1;
		if (ra == 9 && BIT(chr, 12))
			dots = 0x1ff;

		for (int n = 9; n > 0; n--, dots <<= 1)
			*px++ = BIT(dots, 8) ? fg : bg;
	}
}


void fs3216_state::mmu_reg_w(offs_t offset, u16 data)
{
	// 3x SN74LS374N for each of the four spaces
	u32 &reg = m_mmu_reg[offset >> 1];
	if (BIT(offset, 0))
		reg = (reg & 0xff0000) | data;
	else
		reg = (reg & 0x00ffff) | (data & 0x00ff) << 16;
}

u16 fs3216_state::mmu_read(offs_t offset, u16 mem_mask)
{
	const bool a23 = BIT(offset, 22);
	const bool mmu_disable = !a23 && BIT(m_maincpu->get_fc(), 2);
	const u32 mmu_reg = mmu_disable ? (m_from_reset ? 0xfffe00 : 0xfff000) : m_mmu_reg[(offset >> 20) & 3];

	if (!mmu_disable && !machine().side_effects_disabled())
	{
		// TODO: do limit check and cause BERR on failure
	}

	offs_t clbaddr = offset + ((mmu_reg & 0x000fff) << 9);
	clbaddr = (clbaddr & 0x1fffff) | (clbaddr & 0x100000) << 1;
	return m_clb->read16(clbaddr, mem_mask);
}

void fs3216_state::mmu_write(offs_t offset, u16 data, u16 mem_mask)
{
	const bool a23 = BIT(offset, 22);
	const bool mmu_disable = !a23 && BIT(m_maincpu->get_fc(), 2);
	const u32 mmu_reg = mmu_disable ? (m_from_reset ? 0xfffe00 : 0xfff000) : m_mmu_reg[(offset >> 20) & 3];

	if (!mmu_disable && !machine().side_effects_disabled())
	{
		// TODO: do limit/write protect check and cause BERR on failure
	}

	offs_t clbaddr = offset + ((mmu_reg & 0x000fff) << 9);
	clbaddr = (clbaddr & 0x1fffff) | (clbaddr & 0x100000) << 1;
	m_clb->write16(clbaddr, data, mem_mask);
}

void fs3216_state::mmu_reset_w(int state)
{
	if (state)
		m_from_reset = true;
}

void fs3216_state::mmu_init_w(u16 data)
{
	m_from_reset = BIT(data, 0);
}

u16 fs3216_state::irq_r()
{
	// TODO
	return 0xfff8;
}

u8 fs3216_state::intack_r(offs_t offset)
{
	// FIXME: all interrupts are vectored, but not all levels go through this PROM
	return m_vecprom[offset];
}

u8 fs3216_state::ctc_r(offs_t offset)
{
	return m_ctc->read(offset >> 1);
}

void fs3216_state::ctc_w(offs_t offset, u8 data)
{
	m_ctc->write(offset >> 1, data);
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

void fs3216_state::fdc_int_w(int state)
{
	if (state)
		m_floppy_status |= 0x02;
	else
		m_floppy_status &= 0xfd;
}

void fs3216_state::fdc_drq_w(int state)
{
	if (state)
	{
		m_floppy_status |= 0x01;
		if (BIT(m_floppy_control, 3) && !m_fdc_dma_timer->enabled())
		{
			m_fdc_dma_timer->adjust(attotime::from_hz(16_MHz_XTAL / 64));
			m_floppy_status |= 0x04;
		}
	}
	else
	{
		m_floppy_status &= 0xfa;
		m_fdc_dma_timer->adjust(attotime::never);
	}
}

void fs3216_state::fdc_hdl_w(int state)
{
	if (state)
		m_floppy_status |= 0x40;
	else
		m_floppy_status &= 0xbf;
}

void fs3216_state::floppy_idx_w(int state)
{
	if (state)
		m_floppy_status |= 0x20;
	else
		m_floppy_status &= 0xdf;

	if (BIT(m_floppy_select, 2))
		m_fdc->ready_w(state);
}

void fs3216_state::fdc_us_w(u8 data)
{
	m_fdc_select = data;
	if (!BIT(m_floppy_select, 2))
		m_fdc->set_floppy(m_floppy[m_fdc_select]->get_device());
}

u16 fs3216_state::floppy_select_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		floppy_select_w(offset, 0);
	return 0xffff;
}

void fs3216_state::floppy_select_w(offs_t offset, u16 data)
{
	if (m_floppy_select == offset)
		return;

	m_floppy_select = offset;
	if (BIT(offset, 2))
		m_fdc->set_floppy(m_floppy[offset & 3]->get_device());
	else
		m_fdc->set_floppy(m_floppy[m_fdc_select]->get_device());
}

void fs3216_state::floppy_control_w(u8 data)
{
	m_floppy_control = data;

	floppy_image_device *fd = m_floppy[BIT(m_floppy_select, 2) ? (m_floppy_select & 3) : m_fdc_select]->get_device();
	if (BIT(data, 5))
	{
		if (fd != nullptr)
			fd->mon_w(0);
		m_floppy_status |= 0x10;
	}
	else
	{
		if (fd != nullptr)
			fd->mon_w(1);
		m_floppy_status &= 0xef;
	}

	m_fdc->reset_w(!BIT(data, 1));
	if (!BIT(data, 1))
	{
		m_fdc_dma_count = 0;
		m_fdc->tc_w(0);
	}

	if (BIT(data, 3) && BIT(m_floppy_status, 0))
	{
		if (!m_fdc_dma_timer->enabled())
		{
			m_fdc_dma_timer->adjust(attotime::from_hz(16_MHz_XTAL / 64));
			m_floppy_status |= 0x04;
		}
	}
	else
	{
		m_fdc_dma_timer->adjust(attotime::never);
		m_floppy_status &= 0xfb;
	}

	m_fdc->set_unscaled_clock(16_MHz_XTAL / (BIT(data, 0) ? 4 : 2));
}

u8 fs3216_state::floppy_status_r()
{
	return m_floppy_status;
}

TIMER_CALLBACK_MEMBER(fs3216_state::fdc_dma)
{
	if (BIT(m_floppy_control, 4))
		m_fdc_ram[m_fdc_dma_count & 0x7ff] = m_fdc->dma_r();
	else
		m_fdc->dma_w(m_fdc_ram[m_fdc_dma_count & 0x7ff]);
	m_floppy_status &= 0xfb;

	++m_fdc_dma_count;
	if (BIT(m_fdc_dma_count, 11))
		m_fdc->tc_w(1);
}

u8 fs3216_state::fdc_ram_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_fdc_dma_count = offset;
		m_fdc->tc_w(BIT(offset, 11));
	}
	return m_fdc_ram[offset & 0x7ff];
}

void fs3216_state::fdc_ram_w(offs_t offset, u8 data)
{
	if (!machine().side_effects_disabled())
	{
		m_fdc_dma_count = offset;
		m_fdc->tc_w(BIT(offset, 11));
	}
	m_fdc_ram[offset & 0x7ff] = data;
}


void fs3216_state::main_map(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(fs3216_state::mmu_read), FUNC(fs3216_state::mmu_write));
}

void fs3216_state::clb_map(address_map &map)
{
	map(0x000000, 0x017fff).ram();
	map(0x780000, 0x783fff).rom().region("momrom", 0);
	map(0x792000, 0x792003).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x792010, 0x79201f).rw(FUNC(fs3216_state::floppy_select_r), FUNC(fs3216_state::floppy_select_w));
	map(0x792041, 0x792041).w(FUNC(fs3216_state::floppy_control_w));
	map(0x792051, 0x792051).r(FUNC(fs3216_state::floppy_status_r));
	map(0x794680, 0x79468f).rw(FUNC(fs3216_state::ctc_r), FUNC(fs3216_state::ctc_w)).umask16(0x00ff);
	map(0x794701, 0x794701).rw("dart", FUNC(z80dart_device::da_r), FUNC(z80dart_device::da_w));
	map(0x794709, 0x794709).rw("dart", FUNC(z80dart_device::ca_r), FUNC(z80dart_device::ca_w));
	map(0x794711, 0x794711).rw("dart", FUNC(z80dart_device::db_r), FUNC(z80dart_device::db_w));
	map(0x794719, 0x794719).rw("dart", FUNC(z80dart_device::cb_r), FUNC(z80dart_device::cb_w));
	map(0x796000, 0x797fff).rw(FUNC(fs3216_state::fdc_ram_r), FUNC(fs3216_state::fdc_ram_w)).umask16(0x00ff);
	map(0x7a0000, 0x7a1fff).rom().region("video", 0);
	map(0x7a4001, 0x7a4001).w("crtc", FUNC(mc6845_device::address_w));
	map(0x7a4003, 0x7a4003).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x7a8000, 0x7a8fff).ram().share("videoram"); // 2x M58725P
	map(0x7b0000, 0x7b1fff).rom().region("comm_a", 0);
	map(0x7c0000, 0x7c1fff).rom().region("comm_b", 0);
	//map(0x7e0000, 0x7e1fff).rom().region("wd1001_clb", 0);
	map(0x7f1000, 0x7f1001).r(FUNC(fs3216_state::irq_r));
	map(0x7f3000, 0x7f300f).w(FUNC(fs3216_state::mmu_reg_w));
	map(0x7f5000, 0x7f5001).w(FUNC(fs3216_state::mmu_init_w));
	map(0x7f6000, 0x7f6001).nopw();
	map(0x7f7000, 0x7f7001).r(FUNC(fs3216_state::earom_store_r));
	map(0x7f7200, 0x7f7201).r(FUNC(fs3216_state::earom_recall_r));
	map(0x7f7400, 0x7f75ff).rw(m_earom, FUNC(x2212_device::read), FUNC(x2212_device::write)).umask16(0x00ff);
}

void fs3216_state::fc7_map(address_map &map)
{
	map(0xfffff0, 0xffffff).r(FUNC(fs3216_state::intack_r)).umask16(0x00ff);
}

void fs3216_state::wdcpu_prog_map(address_map &map)
{
	map(0x0000, 0x03ff).rom().region("wdcpu", 0);
}

void fs3216_state::wdcpu_bank_map(address_map &map)
{
	map(0x000, 0x000).nopr();
}


static void fs3216_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void fs3216_state::fs3216(machine_config &config)
{
	M68000(config, m_maincpu, 44.2368_MHz_XTAL / 8); // 5.5 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &fs3216_state::main_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &fs3216_state::fc7_map);
	m_maincpu->reset_cb().set(FUNC(fs3216_state::mmu_reset_w));

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
	m_ctc->zc_callback<1>().append(m_ctc, FUNC(z80ctc_device::trg2));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	Z80DART(config, "dart", 44.2368_MHz_XTAL / 8); // Z8470BPS

	UPD765A(config, m_fdc, 16_MHz_XTAL / 2, false, false);
	m_fdc->intrq_wr_callback().set(FUNC(fs3216_state::fdc_int_w));
	m_fdc->drq_wr_callback().set(FUNC(fs3216_state::fdc_drq_w));
	m_fdc->hdl_wr_callback().set(FUNC(fs3216_state::fdc_hdl_w));
	m_fdc->idx_wr_callback().set(FUNC(fs3216_state::floppy_idx_w));
	m_fdc->us_wr_callback().set(FUNC(fs3216_state::fdc_us_w));

	for (int i = 0; i < 4; i++)
		FLOPPY_CONNECTOR(config, m_floppy[i], fs3216_floppies, i < 1 ? "525dd" : nullptr, floppy_image_device::default_mfm_floppy_formats);

	X2212(config, m_earom);

	mc6845_device &crtc(MC6845(config, "crtc", 14.58_MHz_XTAL / 9)); // HD46505RP
	crtc.set_char_width(9);
	crtc.set_show_border_area(false);
	crtc.set_update_row_callback(FUNC(fs3216_state::crt_update_row));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(14.58_MHz_XTAL, 900, 0, 720, 270, 0, 250);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	n8x305_cpu_device &wdcpu(N8X305(config, "wdcpu", 8_MHz_XTAL)); // N8X305I
	wdcpu.set_addrmap(AS_PROGRAM, &fs3216_state::wdcpu_prog_map);
	wdcpu.set_addrmap(AS_IO, &fs3216_state::wdcpu_bank_map);
}


static INPUT_PORTS_START(fs3216)
INPUT_PORTS_END


// XTALs on Mother Board (1001176-01 Rev. 5): 44.2368 MHz (22A), 16.000 MHz (2J)
// XTALs on Comm A board (1000014-01 2 Port / 1000171-01 4 Port Rev. 7; 10000065-01 Rev. 3): two K1135CM Dual Baud Rate Generators (7D, 8D)
// XTALs on Comm B board (1001651-01 Rev. G): none
// XTALs on Video Controller board (1000443-1 Rev. I): 14.580 MHz (1H)
// XTALs on WD-1001 CLB Disk Controller board (1473-008): 20.000 (Y1), 8.000 (Y2)
ROM_START(fs3216)
	ROM_REGION16_BE(0x4000, "momrom", 0)
	ROM_LOAD16_BYTE("17k_1260-02_h.bin", 0x0000, 0x2000, CRC(75ed6de8) SHA1(0360548493b778995ae436da475b6356945e1872))
	ROM_LOAD16_BYTE("15k_1260-01_l.bin", 0x0001, 0x2000, CRC(82695233) SHA1(0d69309f41306298bf6a4ba6928c53f908bb3f2c))

	ROM_REGION(0x100, "earom", 0)
	ROM_LOAD("sn1000044-08_x2212.bin", 0x000, 0x100, CRC(2bf1fec8) SHA1(e1bdda558364415131e68443013c608bb9c01451))

	ROM_REGION(0x20, "vecprom", 0)
	ROM_LOAD("12j_74s288.bin", 0x00, 0x20, CRC(8f7bf087) SHA1(de785f7ab79f0e58e411ec5cbc42991d1d8486b1))

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

} // anonymous namespace


COMP(1982, fs3216, 0, 0, fs3216, fs3216, fs3216_state, empty_init, "Fortune Systems", "Fortune 32:16", MACHINE_IS_SKELETON)
