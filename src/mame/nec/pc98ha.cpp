// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC98LT/HA class machine "Handy98" aka 1st Gen LCD PC98

TODO:
- compose common points from base pc98 class, decouple;
- floppy boot for inufuto games (should autoboot, at least mazy.d88 boots in pc9801 with fdd_2dd
  adapter);
- memory card handling (needs a working SW);
- identify LCDC used here, reg 2 is clearly H display (0x4f+1)*8=640
- when idle for some time buzzer farts until a key is pressed (?);
- add NVRAM saving:
- pinpoint NVRAM init switch source:
\- first port C read (pc98lt: i/o 0x35, PC=0xf841f) tests for bit 7,
   which initializes battery backup if on, but port C is in output mode there.
   Somehow obf irq is on at boot if battery failed?
- power handling;

TODO (pc98lt):
- remove timer hack:
\- definitely incorrect given the erratic cursor blinking in N88BASIC;

TODO (pc98ha):
- RTC is upd4991a (partially done), it's parallel instead of serial and incompatible with
  everything else ugh;
- EMS fails at boot, it's never ever really checked;
- MSDOS cannot detect EMS properly, is there a flag somewhere?
- JEIDA memory card interface (68pin cfr. "Super Daisenryaku HA",
  most likely same as NeoGeo JEIDA 3.0 memory cards);
- optional docking station (for floppy device only or can mount other stuff too?);

**************************************************************************************************/

#include "emu.h"
#include "pc98ha.h"

void pc98lt_state::lt_palette(palette_device &palette) const
{
	// TODO: confirm values
	palette.set_pen_color(0, 160, 168, 160);
	palette.set_pen_color(1, 48, 56, 16);
}

uint32_t pc98lt_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 16)
		{
			u16 pen = bitswap<16>(m_gvram[(y*640+x)/16], 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7);

			for (int xi = 0; xi < 16; xi++)
			{
				u8 dot = (pen >> xi) & 1;
				bitmap.pix(y, x+xi) = m_palette->pen(dot);
			}
		}

	return 0;
}

/*
 * Power Status Register
 *
 * x--- ---- docking station connected (HA only?)
 * -x-- ---- AC power supply connected
 * ---x ---- alarm enabled
 * ---- x--- unknown
 * ---- -x-- Lithium battery low (HA only?)
 * ---- --x- battery low
 * ---- ---x power off
 */
u8 pc98lt_state::power_status_r()
{
	return 0x80;
}

void pc98lt_state::power_control_w(offs_t offset, u8 data)
{
	// TODO: happens pretty often, supposed to halt the system and wake up on arbitrary event?
	if (BIT(data, 2))
		logerror("%s: power_control_w standby signal ON\n", machine().describe_context());

	// pc98lt: go to prompt (type "command" in main menu) and execute "poweroff.com"
	if (BIT(data, 0))
		logerror("%s: power_control_w power off signal ON\n", machine().describe_context());

	// pc98ha bit 1: flips between 0->1 on system boot failure, in tandem with standby mode held
	if (data & ~0x07)
		logerror("%s: power_control_w unknown signal sent %02x\n", machine().describe_context(), data);
}

// TODO: intentionally repeated from base pc98 until I understand what's going on here.
// (supposedly should be same from base pc98 minus the V50 integrations and whatever the "docking station" really adds up)
u8 pc98lt_state::floppy_mode_r(offs_t offset)
{
	// floppy "mode" identifies drive capabilities, if 2dd/2hd exclusive or mixed type.
	// and to my understanding it doesn't really read from write reg ...
	return (m_floppy_mode & 3) | 0xe4;
}

void pc98lt_state::floppy_mode_w(offs_t offset, u8 data)
{
	// bit 1: selects between 2hd and 2dd, not unlike base PC98
	m_floppy_mode = data & 3;
	m_fdc->subdevice<floppy_connector>("0")->get_device()->set_rpm(data & 0x02 ? 360 : 300);
	m_fdc->subdevice<floppy_connector>("1")->get_device()->set_rpm(data & 0x02 ? 360 : 300);

	m_fdc->set_rate(data & 0x02 ? 500000 : 250000);
}

u8 pc98lt_state::fdc_ctrl_r(offs_t offset)
{
	int ret = 0x6c;
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();

	if (floppy0 && floppy0->exists())
		ret |= 0x10;

	if (floppy1 && floppy1->exists())
		ret |= 0x10;

	return ret;
}

void pc98lt_state::fdc_ctrl_w(offs_t offset, u8 data)
{
	const int fdcrst = BIT(data, 7);

	if (BIT(m_fdc_ctrl, 7) != fdcrst)
		m_fdc->reset_w(BIT(data, 7));

	const int ttrg = BIT(data, 0);

	if( ttrg && !BIT(m_fdc_ctrl, 0) )
	{
		m_vfo_timer->adjust(attotime::from_msec(100), 1);
	}
	//else if (!ttrg && BIT(m_fdc_ctrl, 0) )
	//	m_vfo_timer->adjust(attotime::never);

	m_fdc_ctrl = data;
	if(BIT(data, 6))
	{
		m_fdc->set_ready_line_connected(0);
		m_fdc->ready_w(0);
	}
	else
		m_fdc->set_ready_line_connected(1);

	m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(!BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);
	m_fdc->subdevice<floppy_connector>("1")->get_device()->mon_w(!BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);
}

void pc98lt_state::lt_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x5ffff).ram(); // 384 KB
	map(0x60000, 0x9ffff).noprw();

	// no TVRAM
	map(0xa8000, 0xaffff).ram().share("gvram");
//  0xb0000-0xbffff unmapped GVRAM or mirror, check me
	map(0xc0000, 0xcffff).unmaprw(); // EMS area, not present here but checked
	map(0xd0000, 0xd3fff).bankrw("bram_bank");
	map(0xd4000, 0xd7fff).bankr("dict_bank");
	map(0xd8000, 0xdbfff).bankr("kanji_bank");
	map(0xe0000, 0xeffff).bankr("romdrv_bank");
	map(0xf0000, 0xfffff).rom().region("ipl", 0);
}

void pc98lt_state::lt_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x0000, 0x001f) // PIC (bit 3 ON slave / master), V50 internal / <undefined>
	map(0x0020, 0x0020).w(FUNC(pc98lt_state::rtc_w));
	map(0x0030, 0x0037).rw(m_ppi_sys, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
	map(0x0030, 0x0033).rw(m_sio_rs, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff); //i8251 RS232c / i8255 system port
	map(0x0040, 0x0047).rw(m_ppi_prn, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0040, 0x0043).rw(m_sio_kbd, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0xff00); //i8255 printer port / i8251 keyboard
//  map(0x0070, 0x007f) // PIT, V50 internal

	// floppy actually requires a docking station on PC98HA, density should be 2dd given the mapping
	map(0x00be, 0x00be).rw(FUNC(pc98lt_state::floppy_mode_r), FUNC(pc98lt_state::floppy_mode_w));
	map(0x00c8, 0x00cb).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x00cc, 0x00cc).rw(FUNC(pc98lt_state::fdc_ctrl_r), FUNC(pc98lt_state::fdc_ctrl_w));

//  map(0x00e0, 0x00ef) // uPD71071, V50 internal

//  map(0x0810, 0x0810) // <unknown device data>, LCDC?
//  map(0x0812, 0x0812) // <unknown device address> & 0xf

	map(0x0c10, 0x0c10).lrw8(
		NAME([this] () { return (m_bram_bank_reg & (m_bram_banks - 1)) | 0x40; }),
		NAME([this] (u8 data) { m_bram_bank_reg = data & (m_bram_banks - 1); m_bram_bank->set_entry(m_bram_bank_reg); })
	);
//  map(0x0f8e, 0x0f8e) // card slot status 1 (undefined on pc98lt?)
//  map(0x4810, 0x4810) // ?
	map(0x4c10, 0x4c10).lrw8(
		NAME([this] () { return (m_dict_bank_reg & 0x3f) | 0x40; }),
		NAME([this] (u8 data) { m_dict_bank_reg = data & 0x3f; m_dict_bank->set_entry(m_dict_bank_reg); })
	);
//  map(0x5e8e, 0x5e8e) // card slot status 2
//  map(0x6e8e, 0x6e8e) // modem control 1
//  map(0x7e8e, 0x7e8e) // modem control 2
	map(0x8810, 0x8810).rw(FUNC(pc98lt_state::power_status_r), FUNC(pc98lt_state::power_control_w));
	map(0x8c10, 0x8c10).lw8(NAME([this] (u8 data) { m_kanji_bank->set_entry(data & 0x0f); }));
//  map(0xc810, 0xc810) // ?
	map(0xcc10, 0xcc10).lrw8(
		NAME([this] () { return (m_romdrv_bank_reg & 0xf) | 0x40; }),
		NAME([this] (u8 data) { m_romdrv_bank_reg = data & 0xf; m_romdrv_bank->set_entry(m_romdrv_bank_reg); })
	);
}

/************************************
 *
 * 98HA specifics
 *
 ***********************************/

void pc98ha_state::ext_view_bank_w(offs_t offset, u8 data)
{
	if (m_ext_view_sel == 0x81)
		m_ramdrv_bank->set_entry(data & 0x7f);
	else
		logerror("External view SEL bank set %02x (view=%02x)\n", data, m_ext_view_sel);
}

void pc98ha_state::ext_view_sel_w(offs_t offset, u8 data)
{
	m_ext_view_sel = data;
	// either bit 7 ON or writing 0x80 to this port disables the external view.
	if (data & 0x80)
		m_ext_view.select(data & 0x3);
	if (data != 0x81)
		logerror("External view SEL line set %02x\n", data);
}

void pc98ha_state::ems_bank_w(offs_t offset, u8 data)
{
	m_ems_banks[offset]->set_entry(data & 0x7f);
}

u8 pc98ha_state::memcard_status_1_r(offs_t offset)
{
	// TODO: identify exact type
	// 0x0e: memory card present
	// bit 3 is checked at boot, PC=f82a5
	// mask 0xf0 is checked at PC=f8956 then periodically polled at PC=0xfd110
	// NeoGeo uses v3
	return 0x04;
}

u8 pc98ha_state::memcard_status_2_r(offs_t offset)
{
	// 0x46: memory card present
	return 0x40;
}

void pc98ha_state::ha_map(address_map &map)
{
	lt_map(map);
	map(0x00000, 0x9ffff).ram(); // 640 KB

	map(0xc0000, 0xc3fff).bankrw("ems_bank1");
	map(0xc4000, 0xc7fff).bankrw("ems_bank2");
	map(0xc8000, 0xcbfff).bankrw("ems_bank3");
	map(0xcc000, 0xcffff).bankrw("ems_bank4");

	map(0xdc000, 0xdffff).view(m_ext_view);
	m_ext_view[0](0xdc000, 0xdffff).unmaprw(); // unknown, accessed on MSDOS boot
	m_ext_view[1](0xdc000, 0xdffff).bankrw("ramdrv_bank");
	m_ext_view[2](0xdc000, 0xdffff).unmaprw(); // JEIDA memory card
	m_ext_view[3](0xdc000, 0xdffff).unmaprw();
}

void pc98ha_state::ha_io(address_map &map)
{
	lt_io(map);
	map(0x0020, 0x002f).unmaprw();
	map(0x0022, 0x0022).w(m_rtc_pio, FUNC(upd4991a_device::address_w));
	map(0x0023, 0x0023).rw(m_rtc_pio, FUNC(upd4991a_device::data_r), FUNC(upd4991a_device::data_w));
	map(0x08e0, 0x08e7).w(FUNC(pc98ha_state::ems_bank_w)).umask16(0xff00);
	map(0x0e8e, 0x0e8e).w(FUNC(pc98ha_state::ext_view_bank_w));
	map(0x0f8e, 0x0f8e).r(FUNC(pc98ha_state::memcard_status_1_r));
	map(0x1e8e, 0x1e8e).w(FUNC(pc98ha_state::ext_view_sel_w));
	map(0x5f8e, 0x5f8e).r(FUNC(pc98ha_state::memcard_status_2_r));
}


static INPUT_PORTS_START( pc98lt )
	PORT_START("SYSB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(RTC_TAG, FUNC(upd1990a_device::data_out_r))
	PORT_DIPNAME( 0x02, 0x00, "SYSB" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SYSC")
	PORT_DIPNAME( 0x01, 0x00, "SYSC" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("PRNB")
	PORT_DIPNAME( 0x01, 0x01, "PRNB" ) // checked on boot, should be 1 for 2DD format
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // CPUT LT/HA switch
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) // checked on boot
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(pc98lt_state::system_type_r))
INPUT_PORTS_END

static INPUT_PORTS_START( pc98ha )
	PORT_INCLUDE( pc98lt )

	PORT_MODIFY("SYSB")
	PORT_DIPNAME( 0x01, 0x00, "<rtc empty signal>" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_MODIFY("PRNB")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

// debug
static const gfx_layout gfx_16x16x1 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( gfx_pc98lt )
	GFXDECODE_ENTRY( "kanji",   0x00000, gfx_8x8x1,   0x000, 0x01 )
	GFXDECODE_ENTRY( "kanji",   0x00000, gfx_16x16x1, 0x000, 0x01 )
GFXDECODE_END

void pc98lt_state::machine_start()
{
	// TODO: make this and NVRAM saving to co-exist
	// we have a 16-bit host bus with a banked NVRAM window that also has different sizes depending on the model,
	// may consider to encapsulate instead.
	const u32 bram_size = memregion("backup")->bytes() / 2;
	uint16_t *bram = (uint16_t *)memregion("backup")->base();
	m_bram_banks = (bram_size * 2) / 0x4000;
	m_bram_ptr = make_unique_clear<uint16_t[]>(bram_size);

	for (int i = 0; i < bram_size; i++)
		m_bram_ptr[i] = bram[i];

	m_kanji_bank->configure_entries(  0, 0x10,                 memregion("kanji")->base(),   0x4000);
	m_bram_bank->configure_entries(   0, m_bram_banks,         m_bram_ptr.get(),             0x4000);
	m_romdrv_bank->configure_entries( 0, 0x10,                 memregion("romdrv")->base(), 0x10000);
	m_dict_bank->configure_entries(   0, 0x40,                 memregion("dict")->base(),    0x4000);

	m_vfo_timer = timer_alloc(FUNC(pc98lt_state::vfo_timer_cb), this);

	if (m_rtc != nullptr)
	{
		m_rtc->cs_w(1);
		m_rtc->oe_w(1);
	}
	m_sys_type = 0xc0 >> 6;

	save_item(NAME(m_bram_bank_reg));
	save_item(NAME(m_romdrv_bank_reg));
	save_item(NAME(m_dict_bank_reg));
	save_pointer(NAME(m_bram_ptr), bram_size);
}

void pc98ha_state::machine_start()
{
	pc98lt_state::machine_start();
	const u32 ems_banks = 0x80;
	const u32 ems_size = (ems_banks * 0x4000) / 2;

	m_ramdrv_bank->configure_entries(0, 0x80,                 memregion("ramdrv")->base(), 0x4000);

	m_ems_ram = make_unique_clear<uint16_t[]>(ems_size);
	for (int i = 0; i < 4; i++)
		m_ems_banks[i]->configure_entries(0, ems_banks, m_ems_ram.get(), 0x4000);

	save_item(NAME(m_ext_view_sel));
	save_pointer(NAME(m_ems_ram), ems_size);
}

void pc98lt_state::machine_reset()
{
	m_vfo_timer->adjust(attotime::never);
	m_dack = -1;
	m_fdc_ctrl = 0x80;
}

static void pc9801_floppies(device_slot_interface &device)
{
	device.option_add("525dd", TEAC_FD_55F);
	device.option_add("525hd", FLOPPY_525_HD);
//  device.option_add("35hd", FLOPPY_35_HD);
}

void pc98lt_state::uart_irq_check()
{
	m_maincpu->set_input_line(4, m_uart_irq_pending & m_uart_irq_mask ? ASSERT_LINE : CLEAR_LINE);
}

void pc98lt_state::tc_w(int state)
{
	switch(m_dack)
	{
		//case 2:
		case 3:
			m_fdc->tc_w(state);
			break;
	}
}

TIMER_CALLBACK_MEMBER(pc98lt_state::vfo_timer_cb)
{
	int state = (int)param;

	if(BIT(m_fdc_ctrl, 2) && state)
	{
		//m_maincpu->set_input_line(INPUT_LINE_IRQ6, ASSERT_LINE);
		m_fdc_irqs->in_w<1>(ASSERT_LINE);
		// TODO: arbitrary timing, unknown ack cycle
		m_vfo_timer->adjust(attotime::from_usec(100), 0);
	}
	else if (!state)
	{
		m_fdc_irqs->in_w<1>(CLEAR_LINE);
		//m_vfo_timer->adjust(attotime::from_msec(100), 1);
	}

}


void pc98lt_state::lt_config(machine_config &config)
{
	const XTAL xtal = XTAL(8'000'000);
	V50(config, m_maincpu, xtal); // µPD70216
	m_maincpu->set_addrmap(AS_PROGRAM, &pc98lt_state::lt_map);
	m_maincpu->set_addrmap(AS_IO, &pc98lt_state::lt_io);
	// TODO: jumps off the weeds if divided by / 4 after timer check, DMA issue?
//  m_maincpu->set_tclk(xtal / 4);
	m_maincpu->set_tclk(xtal / 100);
//  m_pit->out_handler<0>().set(m_pic1, FUNC(pic8259_device::ir0_w));
	m_maincpu->tout2_cb().set(m_sio_rs, FUNC(i8251_device::write_txc));
	m_maincpu->tout2_cb().append(m_sio_rs, FUNC(i8251_device::write_rxc));
//  m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->out_hreq_cb().set(m_maincpu, FUNC(v50_device::hack_w));
	m_maincpu->out_eop_cb().set(FUNC(pc98lt_state::tc_w));
//	m_maincpu->in_ior_cb<2>().set(m_fdc, FUNC(upd765a_device::dma_r));
//	m_maincpu->out_iow_cb<2>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_maincpu->in_ior_cb<3>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_maincpu->out_iow_cb<3>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_maincpu->in_memr_cb().set([this] (offs_t offset) { return m_maincpu->space(AS_PROGRAM).read_byte(offset); });
	m_maincpu->out_memw_cb().set([this] (offs_t offset, u8 data) { m_maincpu->space(AS_PROGRAM).write_byte(offset, data); });
	m_maincpu->out_dack_cb<0>().set([this] (int state) { if (!state) m_dack = 0; });
	m_maincpu->out_dack_cb<1>().set([this] (int state) { if (!state) m_dack = 1; });
	m_maincpu->out_dack_cb<2>().set([this] (int state) { if (!state) m_dack = 2; });
	m_maincpu->out_dack_cb<3>().set([this] (int state) { if (!state) m_dack = 3; });

	pc9801_serial(config);

	I8251(config, m_sio_kbd, 0);
	m_sio_kbd->txd_handler().set("keyb", FUNC(pc98_kbd_device::input_txd));
	m_sio_kbd->rxrdy_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_sio_kbd->write_cts(0);
	m_sio_kbd->write_dsr(0);

	clock_device &kbd_clock(CLOCK(config, "kbd_clock", 19'200));
	kbd_clock.signal_handler().set(m_sio_kbd, FUNC(i8251_device::write_rxc));
	kbd_clock.signal_handler().append(m_sio_kbd, FUNC(i8251_device::write_txc));

	PC98_KBD(config, m_keyb, 0);
	m_keyb->rxd_callback().set("sio_kbd", FUNC(i8251_device::write_rxd));

	I8255(config, m_ppi_sys, 0);
	// PC98LT/HA has no dips, port A acts as a RAM storage
	m_ppi_sys->in_pa_callback().set(m_ppi_sys, FUNC(i8255_device::pa_r));
	m_ppi_sys->in_pb_callback().set_ioport("SYSB");
//  m_ppi_sys->in_pc_callback().set_constant(0xa0); // 0x80 cpu triple fault reset flag?
	m_ppi_sys->out_pc_callback().set(FUNC(pc98lt_state::ppi_sys_beep_portc_w));

	I8255(config, m_ppi_prn, 0);
	m_ppi_prn->in_pb_callback().set_ioport("PRNB");

	UPD1990A(config, m_rtc);

	UPD765A(config, m_fdc, 8'000'000, false, true);
	m_fdc->intrq_wr_callback().set(m_fdc_irqs, FUNC(input_merger_device::in_w<0>));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v50_device::dreq_w<3>)).invert(); // 2dd
//	m_fdc->drq_wr_callback().append(m_maincpu, FUNC(v50_device::dreq_w<2>)).invert();
	FLOPPY_CONNECTOR(config, "fdc:0", pc9801_floppies, "525dd", pc9801_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc9801_floppies, "525dd", pc9801_state::floppy_formats);

	INPUT_MERGER_ANY_HIGH(config, m_fdc_irqs).output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ6);


	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	// TODO: copied verbatim from base PC98, verify clock et al.
	m_screen->set_raw(21.0526_MHz_XTAL, 848, 0, 640, 440, 0, 400);
	m_screen->set_screen_update(FUNC(pc98lt_state::screen_update));
//  m_screen->screen_vblank().set(FUNC(pc9801_state::vrtc_irq));

	PALETTE(config, m_palette, FUNC(pc98lt_state::lt_palette), 2);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pc98lt);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2400).add_route(ALL_OUTPUTS, "mono", 0.05);
}

void pc98ha_state::ha_config(machine_config &config)
{
	lt_config(config);
	const XTAL xtal = XTAL(10'000'000);
	V50(config.replace(), m_maincpu, xtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc98ha_state::ha_map);
	m_maincpu->set_addrmap(AS_IO, &pc98ha_state::ha_io);
	m_maincpu->set_tclk(xtal / 4);
//  m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	config.device_remove("rtc");
	UPD4991A(config, m_rtc_pio, 32'768);
}

// all ROMs in both sets needs at least chip renaming, and I haven't seen a single PCB pic from the net.
// dict.rom and ramdrv.bin definitely won't fit an even ROM size regardless,
// also backup.bin may not be factory default.

ROM_START( pc98lt )
	ROM_REGION16_LE( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom",      0x000000, 0x010000, BAD_DUMP CRC(b6a6a382) SHA1(3f1767cccc1ae02b3e48f6ee327d3ef4fad05750) )

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom",    0x000000, 0x040000, BAD_DUMP CRC(26a81aa2) SHA1(bf12e40c608ef6ef1ac38f6b0b3ca79260a50cef) )

	ROM_REGION16_LE( 0x10000, "backup", ROMREGION_ERASEFF )
	ROM_LOAD( "backup.bin",   0x000000, 0x010000, BAD_DUMP CRC(56d7ca00) SHA1(d17942e166f98af1d484e497e97d31da515973f7) )

	ROM_REGION( 0x100000, "dict", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom",     0x000000, 0x080000, BAD_DUMP CRC(421278ee) SHA1(f6066fc5085de521395ce1a8bb040536c1454c7e) )

	ROM_REGION( 0x100000, "romdrv", ROMREGION_ERASEFF )
	ROM_LOAD( "romdrv.rom",   0x000000, 0x080000, BAD_DUMP CRC(282ff6eb) SHA1(f4833e49dd9089ec40f5e86a713e08cd8c598578) )
ROM_END

ROM_START( pc98ha )
	ROM_REGION16_LE( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom",      0x000000, 0x010000, BAD_DUMP CRC(2f552bb9) SHA1(7f53bf95181d65b2f9942285da669d92c61247a3) )

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom",    0x000000, 0x040000, BAD_DUMP CRC(4be5ff2f) SHA1(261d28419a2ddebe3177a282952806d7bb036b40) )

	ROM_REGION16_LE( 0x40000, "backup", ROMREGION_ERASEFF )
	ROM_LOAD( "backup.bin",   0x000000, 0x040000, BAD_DUMP CRC(3c5b2a99) SHA1(f8e2f5a4c7601d4e81d5e9c83621107ed3f5a29a) )

	ROM_REGION( 0x100000, "dict", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom",     0x000000, 0x0c0000, BAD_DUMP CRC(6dc8493c) SHA1(3e04cdc3403a814969b6590cd78e239e72677fe5) )

	ROM_REGION( 0x100000, "romdrv", ROMREGION_ERASEFF )
	ROM_LOAD( "romdrv.rom",   0x000000, 0x100000, BAD_DUMP CRC(2f59127f) SHA1(932cb970c2b22408f7895dbf9df6dbc47f8e055b) )

	// $00 filled with odd size
	ROM_REGION( 0x200000, "ramdrv", ROMREGION_ERASEFF )
	ROM_LOAD( "ramdrv.bin",   0x000000, 0x160000, BAD_DUMP CRC(f2cec994) SHA1(c986ad6d8f810ac0a9657c1af26b6fec712d56ed) )
ROM_END


COMP( 1989, pc98lt,      0,        0, lt_config,         pc98lt,   pc98lt_state,        empty_init,   "NEC",   "PC-98LT", MACHINE_NOT_WORKING )
COMP( 1990, pc98ha,      0,        0, ha_config,         pc98ha,   pc98ha_state,        empty_init,   "NEC",   "PC-98HA (Handy98)", MACHINE_NOT_WORKING )
