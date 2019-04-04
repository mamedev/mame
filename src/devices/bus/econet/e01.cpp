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
    IC21    Rockwell RC6522AP VIA behind to the right

*/

/*

    TODO:

    - centronics strobe
    - econet clock speed select
    - ADLC interrupts
    - ECONET device
    - artwork
    - hard disk

        E20: Rodime RO652 (-chs 306,4,17,512)
        E40S: Rodime RO3057S (-chs 680,5,26,512)
        E60S:

*/

#include "emu.h"
#include "e01.h"
#include "bus/scsi/scsihd.h"
#include "softlist.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define R65C102_TAG     "r65c102"
#define R6522_TAG       "ic21"
#define WD2793_TAG      "ic20"
#define MC6854_TAG      "mc6854"
#define HD146818_TAG    "hd146818"
#define CENTRONICS_TAG  "centronics"
#define SCSIBUS_TAG     "scsi"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ECONET_E01,  econet_e01_device,  "econet_e01",  "Acorn FileStore E01")
DEFINE_DEVICE_TYPE(ECONET_E01S, econet_e01s_device, "econet_e01s", "Acorn FileStore E01S")

econet_e01s_device::econet_e01s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: econet_e01_device(mconfig, ECONET_E01S, tag, owner, clock, TYPE_E01S)
{
}


//-------------------------------------------------
//  ROM( e01 )
//-------------------------------------------------

ROM_START( e01 )
	ROM_REGION( 0x10000, R65C102_TAG, 0 )
	//ROM_DEFAULT_BIOS("v131")
	//ROM_SYSTEM_BIOS( 0, "v131", "V 1.31" )
	ROM_LOAD( "0254,205-04 e01 fs",  0x0000, 0x8000, CRC(ae666c76) SHA1(0954119eb5cd09cdbadf76d60d812aa845838d5a) )
	ROM_LOAD( "0254,205-03 e01 mos", 0x8000, 0x8000, CRC(a13e8014) SHA1(6f44a1a48108c60a64a1774cb30c1a59c4a6a199) )
ROM_END


//-------------------------------------------------
//  ROM( e01s )
//-------------------------------------------------

ROM_START( e01s )
	ROM_REGION( 0x10000, R65C102_TAG, 0 )
	//ROM_DEFAULT_BIOS("v140")
	//ROM_SYSTEM_BIOS( 0, "v133", "V 1.33" ) // 0282,008-02 e01s rom
	ROM_LOAD( "e01sv133.rom",  0x0000, 0x10000, CRC(2a4a0032) SHA1(54ad68ceae44992293ccdd64ec88ad8520deec22) ) // which label?
	//ROM_SYSTEM_BIOS( 1, "v140", "V 1.40" )
	ROM_LOAD( "e01sv140.rom",  0x0000, 0x10000, CRC(5068fe86) SHA1(9b8740face15b5541e2375b3054988af00757931) ) // which label?
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *econet_e01_device::device_rom_region() const
{
	switch (m_variant)
	{
	default:
	case TYPE_E01:
		return ROM_NAME( e01 );

	case TYPE_E01S:
		return ROM_NAME( e01s );
	}
}


//-------------------------------------------------
//  MC146818_INTERFACE( rtc_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER(econet_e01_device::rtc_irq_w)
{
	m_rtc_irq = state;

	update_interrupts();
}


//-------------------------------------------------
//  mc6854_interface adlc_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( econet_e01_device::adlc_irq_w )
{
	m_adlc_irq = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( econet_e01_device::econet_data_w )
{
	m_econet->data_w(this, state);
}

WRITE_LINE_MEMBER(econet_e01_device::via_irq_w)
{
	m_via_irq = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( econet_e01_device::clk_en_w )
{
	m_clk_en = state;
}

FLOPPY_FORMATS_MEMBER( econet_e01_device::floppy_formats_afs )
	FLOPPY_AFS_FORMAT
FLOPPY_FORMATS_END0

WRITE_LINE_MEMBER( econet_e01_device::fdc_irq_w )
{
	m_fdc_irq = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( econet_e01_device::fdc_drq_w )
{
	m_fdc_drq = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( econet_e01_device::scsi_bsy_w )
{
	m_scsi_ctrl_in->write_bit1(state);

	if (state)
	{
		m_scsibus->write_sel(0);
	}
}

WRITE_LINE_MEMBER( econet_e01_device::scsi_req_w )
{
	m_scsi_ctrl_in->write_bit5(state);

	if (!state)
	{
		m_scsibus->write_ack(0);
	}

	m_hdc_irq = !state;
	update_interrupts();
}


//-------------------------------------------------
//  ADDRESS_MAP( e01_mem )
//-------------------------------------------------

void econet_e01_device::e01_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(econet_e01_device::read), FUNC(econet_e01_device::write));
	map(0xfc00, 0xfc00).mirror(0x00c3).rw(FUNC(econet_e01_device::rtc_address_r), FUNC(econet_e01_device::rtc_address_w));
	map(0xfc04, 0xfc04).mirror(0x00c3).rw(FUNC(econet_e01_device::rtc_data_r), FUNC(econet_e01_device::rtc_data_w));
	map(0xfc08, 0xfc08).mirror(0x00c0).r(FUNC(econet_e01_device::ram_select_r)).w(FUNC(econet_e01_device::floppy_w));
	map(0xfc0c, 0xfc0f).mirror(0x00c0).rw(WD2793_TAG, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0xfc10, 0xfc1f).mirror(0x00c0).rw(R6522_TAG, FUNC(via6522_device::read), FUNC(via6522_device::write));
	map(0xfc20, 0xfc23).mirror(0x00c0).rw(MC6854_TAG, FUNC(mc6854_device::read), FUNC(mc6854_device::write));
	map(0xfc24, 0xfc24).mirror(0x00c3).rw(FUNC(econet_e01_device::network_irq_disable_r), FUNC(econet_e01_device::network_irq_disable_w));
	map(0xfc28, 0xfc28).mirror(0x00c3).rw(FUNC(econet_e01_device::network_irq_enable_r), FUNC(econet_e01_device::network_irq_enable_w));
	map(0xfc2c, 0xfc2c).mirror(0x00c3).portr("FLAP");
	map(0xfc30, 0xfc30).mirror(0x00c0).rw(FUNC(econet_e01_device::hdc_data_r), FUNC(econet_e01_device::hdc_data_w));
	map(0xfc31, 0xfc31).mirror(0x00c0).r("scsi_ctrl_in", FUNC(input_buffer_device::bus_r));
	map(0xfc32, 0xfc32).mirror(0x00c0).w(FUNC(econet_e01_device::hdc_select_w));
	map(0xfc33, 0xfc33).mirror(0x00c0).w(FUNC(econet_e01_device::hdc_irq_enable_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void econet_e01_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	M65C02(config, m_maincpu, XTAL(8'000'000)/4); // Rockwell R65C102P3
	m_maincpu->set_addrmap(AS_PROGRAM, &econet_e01_device::e01_mem);

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(FUNC(econet_e01_device::rtc_irq_w));

	// devices
	via6522_device &via(VIA6522(config, R6522_TAG, 8_MHz_XTAL / 4));
	via.writepa_handler().set("cent_data_out", FUNC(output_latch_device::bus_w));
	via.irq_handler().set(FUNC(econet_e01_device::via_irq_w));

	MC6854(config, m_adlc, 0);
	m_adlc->out_irq_cb().set(FUNC(econet_e01_device::adlc_irq_w));
	m_adlc->out_txd_cb().set(FUNC(econet_e01_device::econet_data_w));

	WD2793(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(econet_e01_device::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(econet_e01_device::fdc_drq_w));

	for (int i = 0; i < 2; i++)
	{
		FLOPPY_CONNECTOR(config, m_floppy[i]);
		m_floppy[i]->option_add("35dd", FLOPPY_35_DD);
		m_floppy[i]->set_default_option("35dd");
		m_floppy[i]->set_formats(floppy_formats_afs);
	}

	software_list_device &softlist(SOFTWARE_LIST(config, "flop_ls_e01"));
	softlist.set_type("e01_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(R6522_TAG, FUNC(via6522_device::write_ca1));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	SCSI_PORT(config, m_scsibus);
	m_scsibus->set_data_input_buffer(m_scsi_data_in);
	m_scsibus->msg_handler().set(m_scsi_ctrl_in, FUNC(input_buffer_device::write_bit0));
	m_scsibus->bsy_handler().set(FUNC(econet_e01_device::scsi_bsy_w)); // bit1
	// bit 2 0
	// bit 3 0
	// bit 4 NIRQ
	m_scsibus->req_handler().set(FUNC(econet_e01_device::scsi_req_w)); // bit5
	m_scsibus->io_handler().set(m_scsi_ctrl_in, FUNC(input_buffer_device::write_bit6));
	m_scsibus->cd_handler().set(m_scsi_ctrl_in, FUNC(input_buffer_device::write_bit7));
	m_scsibus->set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));

	OUTPUT_LATCH(config, m_scsi_data_out);
	m_scsibus->set_output_latch(*m_scsi_data_out);

	INPUT_BUFFER(config, m_scsi_data_in);
	INPUT_BUFFER(config, m_scsi_ctrl_in);

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
	: econet_e01_device(mconfig, ECONET_E01, tag, owner, clock, TYPE_E01)
{
}


econet_e01_device::econet_e01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int variant)
	: device_t(mconfig, type, tag, owner, clock)
	, device_econet_interface(mconfig, *this)
	, m_maincpu(*this, R65C102_TAG)
	, m_fdc(*this, WD2793_TAG)
	, m_adlc(*this, MC6854_TAG)
	, m_rtc(*this, HD146818_TAG)
	, m_ram(*this, RAM_TAG)
	, m_scsibus(*this, SCSIBUS_TAG)
	, m_scsi_data_out(*this, "scsi_data_out")
	, m_scsi_data_in(*this, "scsi_data_in")
	, m_scsi_ctrl_in(*this, "scsi_ctrl_in")
	, m_floppy(*this, WD2793_TAG":%u", 0U)
	, m_rom(*this, R65C102_TAG)
	, m_centronics(*this, CENTRONICS_TAG)
	, m_adlc_ie(0)
	, m_hdc_ie(0)
	, m_rtc_irq(CLEAR_LINE)
	, m_via_irq(CLEAR_LINE)
	, m_hdc_irq(CLEAR_LINE)
	, m_fdc_irq(CLEAR_LINE)
	, m_fdc_drq(CLEAR_LINE)
	, m_adlc_irq(CLEAR_LINE)
	, m_clk_en(0)
	, m_ram_en(false)
	, m_variant(variant)
	, m_clk_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void econet_e01_device::device_start()
{
	// allocate timers
	m_clk_timer = timer_alloc();

	// register for state saving
	save_item(NAME(m_adlc_ie));
	save_item(NAME(m_hdc_ie));
	save_item(NAME(m_rtc_irq));
	save_item(NAME(m_via_irq));
	save_item(NAME(m_hdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_adlc_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void econet_e01_device::device_reset()
{
	m_clk_timer->adjust(attotime::zero, 0, attotime::from_hz(200000));
	m_ram_en = false;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void econet_e01_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (m_clk_en)
	{
		m_econet->clk_w(this, 1);
		m_econet->clk_w(this, 0);
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( econet_e01_device::read )
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

WRITE8_MEMBER( econet_e01_device::write )
{
	m_ram->pointer()[offset] = data;
}


//-------------------------------------------------
//  eprom_r - ROM/RAM select read
//-------------------------------------------------

READ8_MEMBER( econet_e01_device::ram_select_r )
{
	m_ram_en = true;

	return 0;
}


//-------------------------------------------------
//  floppy_w - floppy control write
//-------------------------------------------------

WRITE8_MEMBER( econet_e01_device::floppy_w )
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
	floppy_image_device *floppy = nullptr;

	if (!BIT(data, 0)) floppy = m_floppy[0]->get_device();
	if (!BIT(data, 1)) floppy = m_floppy[1]->get_device();

	m_fdc->set_floppy(floppy);

	// floppy side select
	if (floppy) floppy->ss_w(BIT(data, 2));

	// TODO NVRAM select
	//mc146818_stby_w(m_rtc, BIT(data, 3));

	// floppy density
	m_fdc->dden_w(BIT(data, 4));

	// floppy master reset
	if (!BIT(data, 5)) m_fdc->soft_reset();

	// TODO floppy test

	// mode LED
	machine().output().set_value("led_0", BIT(data, 7));
}


//-------------------------------------------------
//  network_irq_disable_r -
//-------------------------------------------------

READ8_MEMBER( econet_e01_device::network_irq_disable_r )
{
	network_irq_enable(0);

	return 0;
}


//-------------------------------------------------
//  network_irq_disable_w -
//-------------------------------------------------

WRITE8_MEMBER( econet_e01_device::network_irq_disable_w )
{
	network_irq_enable(0);
}


//-------------------------------------------------
//  network_irq_enable_r -
//-------------------------------------------------

READ8_MEMBER( econet_e01_device::network_irq_enable_r )
{
	network_irq_enable(1);

	return 0;
}


//-------------------------------------------------
//  network_irq_enable_w -
//-------------------------------------------------

WRITE8_MEMBER( econet_e01_device::network_irq_enable_w )
{
	network_irq_enable(1);
}


//-------------------------------------------------
//  hdc_data_r -
//-------------------------------------------------

READ8_MEMBER( econet_e01_device::hdc_data_r )
{
	uint8_t data = m_scsi_data_in->read();

	m_scsibus->write_ack(1);

	return data;
}


//-------------------------------------------------
//  hdc_data_w -
//-------------------------------------------------

WRITE8_MEMBER( econet_e01_device::hdc_data_w )
{
	m_scsi_data_out->write(data);

	m_scsibus->write_ack(1);
}


//-------------------------------------------------
//  hdc_select_w -
//-------------------------------------------------

WRITE8_MEMBER( econet_e01_device::hdc_select_w )
{
	m_scsibus->write_sel(1);
}


//-------------------------------------------------
//  hdc_irq_enable_w -
//-------------------------------------------------

WRITE8_MEMBER( econet_e01_device::hdc_irq_enable_w )
{
	hdc_irq_enable(BIT(data, 0));
}


//-------------------------------------------------
//  rtc_address_r -
//-------------------------------------------------

READ8_MEMBER( econet_e01_device::rtc_address_r )
{
	return m_rtc->read(0);
}


//-------------------------------------------------
//  rtc_address_w -
//-------------------------------------------------

WRITE8_MEMBER( econet_e01_device::rtc_address_w )
{
	m_rtc->write(0, data);
}


//-------------------------------------------------
//  rtc_data_r -
//-------------------------------------------------

READ8_MEMBER( econet_e01_device::rtc_data_r )
{
	return m_rtc->read(1);
}


//-------------------------------------------------
//  rtc_data_w -
//-------------------------------------------------

WRITE8_MEMBER( econet_e01_device::rtc_data_w )
{
	m_rtc->write(1, data);
}


//-------------------------------------------------
//  econet_clk_w -
//-------------------------------------------------

void econet_e01_device::econet_data(int state)
{
	m_adlc->set_rx(state);
}


//-------------------------------------------------
//  econet_clk_w -
//-------------------------------------------------

void econet_e01_device::econet_clk(int state)
{
	m_adlc->rxc_w(state);
	m_adlc->txc_w(state);
}
