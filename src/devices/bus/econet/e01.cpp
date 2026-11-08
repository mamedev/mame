// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn FileStore E01/E01S network hard disk emulation

    http://chrisacorns.computinghistory.org.uk/Network/Econet.html
    http://chrisacorns.computinghistory.org.uk/Network/Pics/Acorn_FileStoreE01.html
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_FileStoreE01S.html
    http://www.heyrick.co.uk/econet/fs/emulator.html
    http://www.pdfio.com/k-1019481.html#

**********************************************************************/

/*

    The FileStore E01 is an Econet station in its own right which acts as a fileserver when connected to a network. It is a single unit
    which does not require a monitor or keyboard. Communication with the FileStore is done via another Econet station when the FileStore
    is in one of two "maintenance modes"

    The E01 can be seen as a slimmed-down BBC computer tailored for its function as a fileserver. It has a 6502 processor at its heart
    along with a 6522 VIA just like the BBC. It requires a Master series Econet module to be plugged in and connects to the network via
    an Econet port in the same way as any other station.

    The FileStore E01S was Acorns second generation Filestore replacing the FileStore E01. The FileStore is a dedicated Econet fileserver,
    it does not support a keyboard and monitor, instead you use an Econet attached station to logon and perform administrative tasks.

            Hitachi HD146818P Real Time Clock
            Rockwell R65C102P3 CPU
            2 x TMM27256D-20 white labelled EPROMs, TMSE01 MOS on left and E01 FS on the right
    IC20    WD2793-APL-02 floppy disc controller
            2 x NEC D41464C-12 64k x 4bit NMOS RAM ICs giving 64K memory
    IC21    Rockwell R65C22AP or CMD G65SC22P-2 VIA behind to the right

*/

#include "emu.h"
#include "e01.h"
#include "bus/nscsi/devices.h"
#include "formats/afs_dsk.h"
#include "machine/nscsi_bus.h"
#include "machine/rescap.h"
#include "softlist_dev.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define R65C102_TAG     "r65c102"
#define R6522_TAG       "ic21"
#define WD2793_TAG      "ic20"
#define MC6854_TAG      "mc6854"
#define HD146818_TAG    "hd146818"
#define CENTRONICS_TAG  "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ECONET_E01,  econet_e01_device,  "econet_e01",  "Acorn FileStore E01")
DEFINE_DEVICE_TYPE(ECONET_E01S, econet_e01s_device, "econet_e01s", "Acorn FileStore E01S")

econet_e01s_device::econet_e01s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: econet_e01_device(mconfig, ECONET_E01S, tag, owner, clock)
{
}


//-------------------------------------------------
//  ROM( e01 )
//-------------------------------------------------

ROM_START( e01 )
	ROM_REGION( 0x10000, R65C102_TAG, 0 )
	ROM_LOAD( "0254,205-04 e01 fs",  0x0000, 0x8000, CRC(ae666c76) SHA1(0954119eb5cd09cdbadf76d60d812aa845838d5a) )
	ROM_LOAD( "0254,205-03 e01 mos", 0x8000, 0x8000, CRC(a13e8014) SHA1(6f44a1a48108c60a64a1774cb30c1a59c4a6a199) )
ROM_END


//-------------------------------------------------
//  ROM( e01s )
//-------------------------------------------------

ROM_START( e01s )
	ROM_REGION( 0x10000, R65C102_TAG, 0 )
	ROM_DEFAULT_BIOS("v140")
	ROM_SYSTEM_BIOS( 0, "v133", "v1.33" ) // 0282,008-02 e01s rom
	ROMX_LOAD( "e01sv133.rom", 0x0000, 0x10000, CRC(2a4a0032) SHA1(54ad68ceae44992293ccdd64ec88ad8520deec22), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v140", "v1.40" )
	ROMX_LOAD( "e01sv140.rom", 0x0000, 0x10000, CRC(5068fe86) SHA1(9b8740face15b5541e2375b3054988af00757931), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *econet_e01_device::device_rom_region() const
{
	return ROM_NAME( e01 );
}

const tiny_rom_entry *econet_e01s_device::device_rom_region() const
{
	return ROM_NAME( e01s );
}


//-------------------------------------------------
//  MC146818_INTERFACE( rtc_intf )
//-------------------------------------------------

void econet_e01_device::rtc_irq_w(int state)
{
	m_rtc_irq = state;

	update_interrupts();
}


//-------------------------------------------------
//  mc6854_interface adlc_intf
//-------------------------------------------------

void econet_e01_device::adlc_irq_w(int state)
{
	m_adlc_irq = state;

	update_interrupts();
}

void econet_e01_device::econet_data_w(int state)
{
	m_econet->data_w(this, state);
}

void econet_e01_device::adlc_rts_w(int state)
{
	m_econet->rts_w(this, state);
}

void econet_e01_device::clk_det_w(int state)
{
	m_clk_present = state;

	m_adlc->set_dcd(!state);
	m_adlc->set_cts(!(m_clk_present && m_data_valid));
}

void econet_e01_device::via_irq_w(int state)
{
	m_via_irq = state;

	update_interrupts();
}

void econet_e01_device::clk_en_w(int state)
{
	m_clk_en = !state;

	update_econet_clk();
}

void econet_e01_device::floppy_formats_afs(format_registration &fr)
{
	fr.add(FLOPPY_AFS_FORMAT);
}

static void e01_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void econet_e01_device::fdc_irq_w(int state)
{
	m_fdc_irq = state;

	update_interrupts();
}

void econet_e01_device::fdc_drq_w(int state)
{
	m_fdc_drq = state;

	update_interrupts();
}

void econet_e01_device::fdc_hld_w(int state)
{
	m_fdc_hld = state;

	m_fdc->set_force_ready(state);

	for (auto &connector : m_floppy)
		if (connector->get_device())
			connector->get_device()->mon_w(!state);

	update_floppy();
}

void econet_e01_device::scsi_bsy_w(int state)
{
	if (state)
		m_scsi->sel_w(0);
}

void econet_e01_device::scsi_req_w(int state)
{
	if (!state)
		m_scsi->ack_w(0);

	m_hdc_irq = !state;
	update_interrupts();
}

void econet_e01_device::scsi_io_w(int state)
{
	m_scsi->write(state ? 0 : m_scsi_data);
}


//-------------------------------------------------
//  ADDRESS_MAP( e01_mem )
//-------------------------------------------------

void econet_e01_device::e01_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(econet_e01_device::read), FUNC(econet_e01_device::write));
	map(0xfc00, 0xfc00).mirror(0x00c3).w(m_rtc, FUNC(mc146818_device::address_w));
	map(0xfc04, 0xfc04).mirror(0x00c3).rw(FUNC(econet_e01_device::rtc_data_r), FUNC(econet_e01_device::rtc_data_w));
	map(0xfc08, 0xfc08).mirror(0x00c0).r(FUNC(econet_e01_device::ram_select_r)).w(FUNC(econet_e01_device::floppy_w));
	map(0xfc0c, 0xfc0f).mirror(0x00c0).rw(WD2793_TAG, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0xfc10, 0xfc1f).mirror(0x00c0).m(R6522_TAG, FUNC(via6522_device::map));
	map(0xfc20, 0xfc23).mirror(0x00c0).rw(MC6854_TAG, FUNC(mc6854_device::read), FUNC(mc6854_device::write));
	map(0xfc24, 0xfc24).mirror(0x00c3).rw(FUNC(econet_e01_device::network_irq_disable_r), FUNC(econet_e01_device::network_irq_disable_w));
	map(0xfc28, 0xfc28).mirror(0x00c3).rw(FUNC(econet_e01_device::network_irq_enable_r), FUNC(econet_e01_device::network_irq_enable_w));
	map(0xfc2c, 0xfc2c).mirror(0x00c3).portr(m_flap);
	map(0xfc30, 0xfc30).mirror(0x00c0).rw(FUNC(econet_e01_device::hdc_data_r), FUNC(econet_e01_device::hdc_data_w));
	map(0xfc31, 0xfc31).mirror(0x00c0).r(FUNC(econet_e01_device::hdc_status_r));
	map(0xfc32, 0xfc32).mirror(0x00c0).w(FUNC(econet_e01_device::hdc_select_w));
	map(0xfc33, 0xfc33).mirror(0x00c0).w(FUNC(econet_e01_device::hdc_irq_enable_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void econet_e01_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	R65C102(config, m_maincpu, XTAL(8'000'000)); // Rockwell R65C102P3
	m_maincpu->set_addrmap(AS_PROGRAM, &econet_e01_device::e01_mem);

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(FUNC(econet_e01_device::rtc_irq_w));

	// devices
	via6522_device &via(R65C22(config, R6522_TAG, 8_MHz_XTAL / 4));
	via.writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	via.ca2_handler().set(m_centronics, FUNC(centronics_device::write_strobe));
	via.cb2_handler().set(FUNC(econet_e01_device::clk_en_w));
	via.irq_handler().set(FUNC(econet_e01_device::via_irq_w));

	MC6854(config, m_adlc);
	m_adlc->out_irq_cb().set(FUNC(econet_e01_device::adlc_irq_w));
	m_adlc->out_txd_cb().set(FUNC(econet_e01_device::econet_data_w));
	m_adlc->out_rts_cb().set(FUNC(econet_e01_device::adlc_rts_w));

	TTL74123(config, m_clk_det, RES_K(39), CAP_N(2.2));
	m_clk_det->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_clk_det->set_a_pin_value(0);
	m_clk_det->set_clear_pin_value(1);
	m_clk_det->out_cb().set(FUNC(econet_e01_device::clk_det_w));

	WD2793(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(econet_e01_device::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(econet_e01_device::fdc_drq_w));
	m_fdc->hld_wr_callback().set(FUNC(econet_e01_device::fdc_hld_w));
	m_fdc->enmf_rd_callback().set_constant(0);

	FLOPPY_CONNECTOR(config, m_floppy[0], e01_floppies, "35dd", floppy_formats_afs).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], e01_floppies, "35dd", floppy_formats_afs).enable_sound(true);

	SOFTWARE_LIST(config, "flop_ls_e01").set_original("e01_flop");
	SOFTWARE_LIST(config, "hdd_ls_e01").set_original("e01_hdd");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(R6522_TAG, FUNC(via6522_device::write_ca1));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "e01_hd");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this] (device_t *device) {
			downcast<nscsi_callback_device &>(*device).bsy_callback().set(*this, FUNC(econet_e01_device::scsi_bsy_w));
			downcast<nscsi_callback_device &>(*device).req_callback().set(*this, FUNC(econet_e01_device::scsi_req_w));
			downcast<nscsi_callback_device &>(*device).io_callback().set(*this, FUNC(econet_e01_device::scsi_io_w));
		});

	// internal ram
	RAM(config, m_ram);
	m_ram->set_default_size("64K");
}


//-------------------------------------------------
//  INPUT_PORTS( e01 )
//-------------------------------------------------

static INPUT_PORTS_START( e01 )
	PORT_START("FLAP")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x40, 0x00, "Front Flap")
	PORT_CONFSETTING( 0x00, "Closed" )
	PORT_CONFSETTING( 0x40, "Open" )
	PORT_DIPNAME( 0x80, 0x00, "SW3")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )

	PORT_START("LINKS")
	PORT_CONFNAME( 0x08, 0x00, "Econet Clock Mark")
	PORT_CONFSETTING( 0x00, "1 us (J3)" )
	PORT_CONFSETTING( 0x08, "2 us (J4)" )
	PORT_CONFNAME( 0x01, 0x01, "J5")
	PORT_CONFSETTING( 0x00, "Fitted" )
	PORT_CONFSETTING( 0x01, "Open" )
	PORT_CONFNAME( 0x02, 0x02, "J6")
	PORT_CONFSETTING( 0x00, "Fitted" )
	PORT_CONFSETTING( 0x02, "Open" )
	PORT_CONFNAME( 0x04, 0x00, "J7")
	PORT_CONFSETTING( 0x00, "Fitted" )
	PORT_CONFSETTING( 0x04, "Open" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor econet_e01_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( e01 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_interrupts - update interrupt state
//-------------------------------------------------

inline void econet_e01_device::update_interrupts()
{
	int irq = (m_via_irq || (m_hdc_ie & m_hdc_irq) || m_rtc_irq) ? ASSERT_LINE : CLEAR_LINE;
	int nmi = (m_fdc_irq || m_fdc_drq || (m_adlc_ie & m_adlc_irq)) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(M6502_IRQ_LINE, irq);
	m_maincpu->set_input_line(M6502_NMI_LINE, nmi);
}


//-------------------------------------------------
//   network_irq_enable - network interrupt enable
//-------------------------------------------------

inline void econet_e01_device::network_irq_enable(int enabled)
{
	m_adlc_ie = enabled;

	update_interrupts();
}


//-------------------------------------------------
//   hdc_irq_enable - hard disk interrupt enable
//-------------------------------------------------

inline void econet_e01_device::hdc_irq_enable(int enabled)
{
	m_hdc_ie = enabled;

	update_interrupts();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  econet_e01_device - constructor
//-------------------------------------------------

econet_e01_device::econet_e01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: econet_e01_device(mconfig, ECONET_E01, tag, owner, clock)
{
}


econet_e01_device::econet_e01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_econet_interface(mconfig, *this)
	, m_maincpu(*this, R65C102_TAG)
	, m_fdc(*this, WD2793_TAG)
	, m_adlc(*this, MC6854_TAG)
	, m_clk_det(*this, "clk_det")
	, m_rtc(*this, HD146818_TAG)
	, m_ram(*this, RAM_TAG)
	, m_scsi(*this, "scsi:7:scsicb")
	, m_floppy(*this, WD2793_TAG":%u", 0U)
	, m_rom(*this, R65C102_TAG)
	, m_centronics(*this, CENTRONICS_TAG)
	, m_flap(*this, "FLAP")
	, m_links(*this, "LINKS")
	, m_led(*this, "led_0")
	, m_adlc_ie(false)
	, m_hdc_ie(false)
	, m_rtc_irq(false)
	, m_via_irq(false)
	, m_hdc_irq(false)
	, m_fdc_irq(false)
	, m_scsi_data(0)
	, m_fdc_drq(false)
	, m_adlc_irq(false)
	, m_clk_present(false)
	, m_data_valid(false)
	, m_fdc_hld(false)
	, m_floppy_sel(0)
	, m_floppy_side(false)
	, m_clk_en(false)
	, m_clk_tc(true)
	, m_rtc_ce(false)
	, m_ram_en(false)
	, m_clk_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void econet_e01_device::device_start()
{
	// allocate timers
	m_clk_timer = timer_alloc(FUNC(econet_e01_device::clk_tick), this);

	// register for state saving
	save_item(NAME(m_adlc_ie));
	save_item(NAME(m_hdc_ie));
	save_item(NAME(m_rtc_irq));
	save_item(NAME(m_via_irq));
	save_item(NAME(m_hdc_irq));
	save_item(NAME(m_scsi_data));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_adlc_irq));
	save_item(NAME(m_clk_present));
	save_item(NAME(m_data_valid));
	save_item(NAME(m_clk_en));
	save_item(NAME(m_clk_tc));
	save_item(NAME(m_rtc_ce));
	save_item(NAME(m_ram_en));
	save_item(NAME(m_fdc_hld));
	save_item(NAME(m_floppy_sel));
	save_item(NAME(m_floppy_side));
}


//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void econet_e01_device::device_post_load()
{
	update_floppy();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void econet_e01_device::device_reset()
{
	m_clk_timer->adjust(attotime::zero);
	m_ram_en = false;

	floppy_w(0);
}


//-------------------------------------------------
//  clk_tick - update our clock output
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(econet_e01_device::clk_tick)
{
	uint8_t const links = m_links->read();
	attotime const cp = attotime::from_ticks(BIT(links, 3) ? 4 : 2, 8_MHz_XTAL / 4);
	unsigned const load = 8 | (links & 0x07);

	if (load == 15)
		m_clk_tc = true;
	else
		m_clk_tc = !m_clk_tc;

	update_econet_clk();

	m_clk_timer->adjust(m_clk_tc ? cp : cp * (15 - load));
}


//-------------------------------------------------
//  update_econet_clk - drive the Econet clock
//-------------------------------------------------

void econet_e01_device::update_econet_clk()
{
	m_econet->clk_w(this, m_clk_en ? m_clk_tc : 1);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t econet_e01_device::read(offs_t offset)
{
	uint8_t data;

	if (m_ram_en)
	{
		data = m_ram->pointer()[offset];
	}
	else
	{
		data = m_rom->base()[offset];
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void econet_e01_device::write(offs_t offset, uint8_t data)
{
	m_ram->pointer()[offset] = data;
}


//-------------------------------------------------
//  eprom_r - ROM/RAM select read
//-------------------------------------------------

uint8_t econet_e01_device::ram_select_r()
{
	if (!machine().side_effects_disabled())
		m_ram_en = true;

	return 0;
}


//-------------------------------------------------
//  floppy_w - floppy control write
//-------------------------------------------------

void econet_e01_device::floppy_w(uint8_t data)
{
	/*

	    bit     description

	    0       floppy 1 select
	    1       floppy 2 select
	    2       floppy side select
	    3       NVRAM select
	    4       floppy density
	    5       floppy master reset
	    6       floppy test
	    7       mode LED

	*/

	// floppy select
	m_floppy_sel = data & 0x03;

	// floppy side select
	m_floppy_side = BIT(data, 2);

	update_floppy();

	// NVRAM select
	m_rtc_ce = BIT(data, 3);

	// floppy density
	m_fdc->dden_w(BIT(data, 4));

	// floppy master reset
	m_fdc->mr_w(BIT(data, 5));

	// TODO floppy test

	// mode LED
	m_led = BIT(data, 7);
}


//-------------------------------------------------
//  update_floppy - drive select is gated by HLD
//-------------------------------------------------

void econet_e01_device::update_floppy()
{
	floppy_image_device *floppy = nullptr;

	if (m_fdc_hld)
	{
		if (BIT(m_floppy_sel, 0)) floppy = m_floppy[0]->get_device();
		if (BIT(m_floppy_sel, 1)) floppy = m_floppy[1]->get_device();
	}

	m_fdc->set_floppy(floppy);

	for (auto &connector : m_floppy)
		if (connector->get_device())
			connector->get_device()->ss_w(m_floppy_side);
}


//-------------------------------------------------
//  rtc_data_r -
//-------------------------------------------------

uint8_t econet_e01_device::rtc_data_r()
{
	return m_rtc_ce ? m_rtc->data_r() : 0;
}


//-------------------------------------------------
//  rtc_data_w -
//-------------------------------------------------

void econet_e01_device::rtc_data_w(uint8_t data)
{
	if (m_rtc_ce)
		m_rtc->data_w(data);
}


//-------------------------------------------------
//  network_irq_disable_r -
//-------------------------------------------------

uint8_t econet_e01_device::network_irq_disable_r()
{
	if (!machine().side_effects_disabled())
		network_irq_enable(0);

	return 0;
}


//-------------------------------------------------
//  network_irq_disable_w -
//-------------------------------------------------

void econet_e01_device::network_irq_disable_w(uint8_t data)
{
	network_irq_enable(0);
}


//-------------------------------------------------
//  network_irq_enable_r -
//-------------------------------------------------

uint8_t econet_e01_device::network_irq_enable_r()
{
	if (!machine().side_effects_disabled())
		network_irq_enable(1);

	return 0;
}


//-------------------------------------------------
//  network_irq_enable_w -
//-------------------------------------------------

void econet_e01_device::network_irq_enable_w(uint8_t data)
{
	network_irq_enable(1);
}


//-------------------------------------------------
//  hdc_data_r -
//-------------------------------------------------

uint8_t econet_e01_device::hdc_data_r()
{
	uint8_t data = m_scsi->read();

	if (!machine().side_effects_disabled())
		m_scsi->ack_w(1);

	return data;
}


//-------------------------------------------------
//  hdc_status_r -
//-------------------------------------------------

uint8_t econet_e01_device::hdc_status_r()
{
	/*

	    bit     description

	    0       MSG
	    1       BSY
	    2       0
	    3       0
	    4       NIRQ
	    5       REQ
	    6       I/O
	    7       C/D

	*/

	uint8_t data = 0;

	data |= m_scsi->msg_r();
	data |= m_scsi->bsy_r() << 1;
	data |= !(m_hdc_ie && m_hdc_irq) << 4;
	data |= m_scsi->req_r() << 5;
	data |= m_scsi->io_r() << 6;
	data |= m_scsi->cd_r() << 7;

	return data;
}


//-------------------------------------------------
//  hdc_data_w -
//-------------------------------------------------

void econet_e01_device::hdc_data_w(uint8_t data)
{
	m_scsi_data = data;
	if (!m_scsi->io_r())
		m_scsi->write(data);
	m_scsi->ack_w(1);
}


//-------------------------------------------------
//  hdc_select_w -
//-------------------------------------------------

void econet_e01_device::hdc_select_w(uint8_t data)
{
	// SEL is left set after an unanswered selection, and nscsi targets only sample it on an edge
	m_scsi->sel_w(0);
	m_scsi->sel_w(1);
}


//-------------------------------------------------
//  hdc_irq_enable_w -
//-------------------------------------------------

void econet_e01_device::hdc_irq_enable_w(uint8_t data)
{
	hdc_irq_enable(BIT(data, 0));
}


//-------------------------------------------------
//  econet_clk_w -
//-------------------------------------------------

void econet_e01_device::econet_data(int state)
{
	m_adlc->set_rx(state);
}


//-------------------------------------------------
//  econet_valid -
//-------------------------------------------------

void econet_e01_device::econet_valid(int state)
{
	m_data_valid = state;

	m_adlc->set_cts(!(m_clk_present && m_data_valid));
}


//-------------------------------------------------
//  econet_clk -
//-------------------------------------------------

void econet_e01_device::econet_clk(int state)
{
	m_adlc->rxc_w(state);
	m_adlc->txc_w(state);
	m_clk_det->b_w(state);
}
