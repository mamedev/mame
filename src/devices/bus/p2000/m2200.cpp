// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 (M2200 Multi Purpose) Floppy Dics Controller Card

**********************************************************************/

#include "emu.h"
#include "m2200.h"

#define LOG_IRQ    (1U << 1)   // CTC / FDC interrupt
#define LOG_FDC    (1U << 2)   // Floppy disk controller states
#define LOG_RAMDSK (1U << 3)   // RAM disk access
#define LOG_CENT   (1U << 4)   // Centronics port

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define LOGIRQ(...)     LOGMASKED(LOG_IRQ, __VA_ARGS__)
#define LOGFDC(...)     LOGMASKED(LOG_FDC, __VA_ARGS__)
#define LOGRAMDSK(...)  LOGMASKED(LOG_RAMDSK, __VA_ARGS__)
#define LOGCENT(...)    LOGMASKED(LOG_CENT, __VA_ARGS__)

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P2000_FDC, p2000_fdc_device, "p2000_fdc", "P2000 Floppy Disk Controller")
DEFINE_DEVICE_TYPE(P2000_M2200, p2000_m2200_multipurpose_device, "p2000_m2200", "P2000 Miniware M2200 Multi Purpose FDC - 256K RAM disk")
DEFINE_DEVICE_TYPE(P2000_M2200D, p2000_m2200d_multipurpose_device, "p2000_m2200d", "P2000 Miniware M2200D Multi Purpose FDC - 64K RAM disk")

static DEVICE_INPUT_DEFAULTS_START( rs232 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


// Floppy formats 
void p2000_fdc_device::floppy_formats(format_registration &fr)
{
    fr.add(FLOPPY_P2000T_FORMAT);
}

//FLOPPY_DSK_FORMAT - FLOPPY_P2000T_FORMAT
static void p2000t_floppies(device_slot_interface &device)
{
    device.option_add("35ss40",  FLOPPY_35_SSDD);        // 3.5"  single sided 40 tracks
    device.option_add("35ds40",  FLOPPY_35_DD);          // 3.5"  double sided 40 tracks
    device.option_add("35ds80",  FLOPPY_35_HD);          // 3.5"  double sided 35 tracks
    device.option_add("525ss35", FLOPPY_525_SSSD);       // 5.25" single sided 35 tracks
    device.option_add("525ds35", FLOPPY_525_SD);         // 5.25" double sided 35 tracks
    device.option_add("525ss40", FLOPPY_525_SSDD);       // 5.25" single sided 40 tracks
    device.option_add("525ds40", FLOPPY_525_DD);         // 5.25" double sided 40 tracks
    device.option_add("525ds80", FLOPPY_525_QD);         // 5.25" double sided 80 tracks
}

#define P2000T_FDC_MOTOR_ON 0x00
#define P2000T_FDC_MOTOR_OFF 0x01

WRITE_LINE_MEMBER(p2000_fdc_device::fdc_interrupt)
{
    LOGIRQ("M2200/FDC Interrupt %d\n", state);
    m_slot->irq_w(state);
}

void p2000_fdc_device::device_add_mconfig(machine_config &config)
{
    Z80CTC(config, m_ctc, 4_MHz_XTAL);
    m_ctc->intr_callback().set(FUNC(p2000_fdc_device::fdc_interrupt));
    
    z80_device *maincpu = nullptr;
    maincpu = config.root_device().subdevice<z80_device>("maincpu");
    if (maincpu == nullptr)
        osd_printf_error("maincpu not found!");
    else 
        maincpu->set_daisy_config(get_z80_daisy_config());

    // The floppy drive driver
    UPD765A(config, m_fdc, 4_MHz_XTAL, true, true);
    m_fdc->intrq_wr_callback().set(FUNC(p2000_fdc_device::fdc_irq_trigger));
    m_fdc->idx_wr_callback().set(FUNC(p2000_fdc_device::fdc_index_trigger));
    m_fdc->hdl_wr_callback().set(FUNC(p2000_fdc_device::fdc_hdl_wr_trigger));
    
    // Drive 0 is not connected so start with drive 1
    for (int i = 1; i < m_num_of_drives; i++)
    {   
        FLOPPY_CONNECTOR(config, m_floppy[i], p2000t_floppies, m_floppy_def_param[i], floppy_formats); //.enable_sound(true);
    }
}

void p2000_m2200_multipurpose_device::device_add_mconfig(machine_config &config)
{
    Z80CTC(config, m_ctc, 2.5_MHz_XTAL);
    m_ctc->intr_callback().set(FUNC(p2000_m2200_multipurpose_device::fdc_interrupt));
    
    z80_device *maincpu = nullptr;
    maincpu = config.root_device().subdevice<z80_device>("maincpu");
    if (maincpu == nullptr)
        osd_printf_error("maincpu not found!");
    else 
        maincpu->set_daisy_config(get_z80_daisy_config());

    /* The floppy drive driver */
    UPD765A(config, m_fdc, 4_MHz_XTAL, true, true);
    m_fdc->intrq_wr_callback().set(FUNC(p2000_m2200_multipurpose_device::fdc_irq_trigger));
    m_fdc->idx_wr_callback().set(FUNC(p2000_m2200_multipurpose_device::fdc_index_trigger));
    m_fdc->hdl_wr_callback().set(FUNC(p2000_m2200_multipurpose_device::fdc_hdl_wr_trigger));
    
    // Drive 0 is not supported so start with drive 1
    for (int i = 1; i < m_num_of_drives; i++)
    {   
        FLOPPY_CONNECTOR(config, m_floppy[i], p2000t_floppies, m_floppy_def_param[i], floppy_formats); //.enable_sound(true);
    }

    /* Set up realtime clock */
    MC146818(config, m_rtc, 32.768_kHz_XTAL).set_24hrs(true);
	m_rtc->irq().set(m_ctc, FUNC(z80ctc_device::trg2));

    /* Set up RS232/RS422 port */
    Z80CTC(config, m_ctc2, 2.5_MHz_XTAL);
    m_ctc2->intr_callback().set(FUNC(p2000_m2200_multipurpose_device::fdc_interrupt));
    m_ctc2->set_clk<0>(2.5_MHz_XTAL);
	m_ctc2->set_clk<1>(2.5_MHz_XTAL);
	m_ctc2->set_clk<2>(2.5_MHz_XTAL);
	m_ctc2->set_clk<3>(2.5_MHz_XTAL);
	m_ctc2->zc_callback<0>().set("sio", FUNC(z80sio_device::rxca_w));
	m_ctc2->zc_callback<1>().set("sio", FUNC(z80sio_device::txca_w));
	
    clock_device &sio_clock(CLOCK(config, "sio_clock", 2.5_MHz_XTAL));
	sio_clock.signal_handler().set("sio", FUNC(z80sio_device::rxtxcb_w));
	
    z80sio_device &sio(Z80SIO(config, "sio", 2.5_MHz_XTAL));
    sio.out_int_callback().set(FUNC(p2000_m2200_multipurpose_device::fdc_interrupt));
    sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	
    sio.out_txdb_callback().set("rs422", FUNC(rs232_port_device::write_txd));
	
    rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr)); 
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	rs232.dcd_handler().set("sio", FUNC(z80sio_device::dcda_w));
	rs232.set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(rs232));
    rs232.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(rs232));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(rs232));

	rs232_port_device &rs422(RS232_PORT(config, "rs422", default_rs232_devices, nullptr));
	rs422.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	
    /* Set up centronics port */
    CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(p2000_m2200_multipurpose_device::centronics_ack_w));
    m_centronics->busy_handler().set(FUNC(p2000_m2200_multipurpose_device::centronics_busy_w));
    m_centronics->perror_handler().set(FUNC(p2000_m2200_multipurpose_device::centronics_paper_empty_w));
    m_centronics->select_handler().set(FUNC(p2000_m2200_multipurpose_device::centronics_printer_on_w));
    m_centronics->fault_handler().set(FUNC(p2000_m2200_multipurpose_device::centronics_error_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  Constructors
//-------------------------------------------------

p2000_fdc_device::p2000_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_M2200, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
        , m_ctc(*this, "ctc")
        , m_fdc(*this, "fdc")
        , m_floppy(*this, "fdc:%u", 0)
{
}

p2000_m2200_multipurpose_device::p2000_m2200_multipurpose_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : p2000_fdc_device(mconfig, tag, owner, clock)
    , m_rtc(*this, "rtc")
    , m_ctc2(*this, "ctc2")
    , m_sio(*this, "sio")
    , m_centronics(*this, "centronics")
{
}

p2000_m2200d_multipurpose_device::p2000_m2200d_multipurpose_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : p2000_m2200_multipurpose_device(mconfig, tag, owner, clock)
{
}

void p2000_fdc_device::device_start()
{
    // FDC Init ready control logic timer
	m_ready_control_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(p2000_fdc_device::ready_timer_cb), this));
    
    m_slot->io_space().install_readwrite_handler(0x88, 0x8b, read8sm_delegate(*m_ctc, FUNC(z80ctc_device::read)), write8sm_delegate(*m_ctc, FUNC(z80ctc_device::write)));
    m_slot->io_space().install_read_handler(0x8c, 0x8c, read8smo_delegate(*m_fdc, FUNC(upd765a_device::msr_r)));
    m_slot->io_space().install_readwrite_handler(0x8d, 0x8d, read8smo_delegate(*this, FUNC(p2000_fdc_device::fdc_read)), write8smo_delegate(*this, FUNC(p2000_fdc_device::fdc_write)));
    m_slot->io_space().install_readwrite_handler(0x90, 0x90, read8smo_delegate(*this, FUNC(p2000_fdc_device::fdc_fcdr)), write8smo_delegate(*this, FUNC(p2000_fdc_device::fdc_control)));
}

void p2000_m2200_multipurpose_device::device_start()
{
    // reserve memory for ram-disk  256KB or 64KB
	m_ramdisk = std::make_unique<uint8_t[]>(get_ramdrive_size());
    
    // FDC Init ready control logic timer
	m_ready_control_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(p2000_m2200_multipurpose_device::ready_timer_cb), this));

    // SIO - RS232/RS422 handlers
    m_slot->io_space().install_readwrite_handler(0x80, 0x83, read8sm_delegate(*m_ctc2, FUNC(z80ctc_device::read)), write8sm_delegate(*m_ctc2, FUNC(z80ctc_device::write)));
    m_slot->io_space().install_readwrite_handler(0x84, 0x87, read8sm_delegate(*m_sio, FUNC(z80sio_device::ba_cd_r)), write8sm_delegate(*m_sio, FUNC(z80sio_device::ba_cd_w)));
    
    // FDC - handlers
    m_slot->io_space().install_readwrite_handler(0x88, 0x8b, read8sm_delegate(*m_ctc, FUNC(z80ctc_device::read)), write8sm_delegate(*m_ctc, FUNC(z80ctc_device::write)));
    m_slot->io_space().install_read_handler(0x8c, 0x8c, read8smo_delegate(*m_fdc, FUNC(upd765a_device::msr_r)));
    m_slot->io_space().install_readwrite_handler(0x8d, 0x8d, read8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::fdc_read)), write8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::fdc_write)));
    m_slot->io_space().install_readwrite_handler(0x90, 0x90, read8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::fdc_fcdr)), write8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::fdc_control)));
    
    // RAM disk handlers
    m_slot->io_space().install_readwrite_handler(0x95, 0x95, read8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_95_r)), write8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_95_w)));
    m_slot->io_space().install_readwrite_handler(0x96, 0x96, read8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_96_r)), write8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_96_w)));
    m_slot->io_space().install_readwrite_handler(0x97, 0x97, read8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_97_r)), write8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_97_w)));

    // Real time clock handlers
    m_slot->io_space().install_readwrite_handler(0x9c, 0x9d, read8sm_delegate(*m_rtc, FUNC(mc146818_device::read)), write8sm_delegate(*m_rtc, FUNC(mc146818_device::write)));
    
    // Centronics handlers
    m_slot->io_space().install_write_handler(0x98, 0x98, write8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_98_w)));
    m_slot->io_space().install_read_handler(0x99, 0x99, read8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_99_r)));
    m_slot->io_space().install_readwrite_handler(0x9a, 0x9a, read8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_9a_r)), write8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_9a_w)));
    m_slot->io_space().install_readwrite_handler(0x9b, 0x9b, read8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_9b_r)), write8smo_delegate(*this, FUNC(p2000_m2200_multipurpose_device::port_9b_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void p2000_fdc_device::device_reset()
{
    LOG("FDC device reset\n");
    // Reset ready control logic
    m_ready_control_pulse_cnt = 0;
}

void p2000_m2200_multipurpose_device::device_reset()
{
    LOG("M2200 device reset\n");

    // Init RAM disk registers
    m_ramdisk_track = 0;
    m_ramdisk_sector = 0;
    m_ramdisk_sector = 0;
    
    // Preset RTC
    m_rtc->write(0, 10);    // REG_A 
    m_rtc->write(1, 0x20);  // progam correct cristal setttings
    m_rtc->write(0, 0);     // Back to default register

    // Reset ready control logic
    m_ready_control_pulse_cnt = 0;
    
    /* ack = 1, busy = 1, paper empty = 1, printer on = 1, error = 0 */
    m_centronics_status = 0x0f;
    m_centronics->write_strobe(1);
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************
/* 
  FDC 0x8c - 0x8f Floppy controller upd765a
       0x8c  Input Status FDC
       0x8d  !fdc_control_reg bit 0: data I/O disk memory
             fdc_control_reg bit 0: fdc status reg
       0x8e  n.a.
       0x8f  n.a. 
*/
uint8_t p2000_fdc_device::fdc_read()
{
    return (!BIT(m_fdc_control_reg, 0) ? m_fdc->fifo_r() : m_fdc->dma_r());
}

void p2000_fdc_device::fdc_write(uint8_t data)
{
    if (!BIT(m_fdc_control_reg, 0)) 
    {
        m_fdc->fifo_w(data);
    }
    else
    {
        m_fdc->dma_w(data);
    }
}

/* -------------------------------------------------
    FDC 0x90 control
      bit 0  FDC enable
             0: data transport
             1: register read/write
      bit 1  Terminal count FDC
      bit 2  FDC reset-n
      bit 3  motor on
------------------------------------------------- */
void p2000_fdc_device::fdc_control(uint8_t data)
{
    LOGFDC("FDC control = %02x \n", data);

    // BIT(data,1); --> Terminal count FDC
    if (BIT(m_fdc_control_reg, 0) && !BIT(data, 0))
    {   // Toggle tc when old fdc enable state was ative and is going to low
        LOGFDC("toggle tc (FCE = %02x)\n", BIT(m_fdc_control_reg, 0));
        m_fdc->tc_w(1);
    }
    m_fdc->tc_w(BIT(data,1));

    // BIT(data,2); --> FDC reset-n
    m_fdc->reset_w(BIT(data,2) ? 0 : 1);
    
    // BIT(data,3); --> motor on/off  (only update on change OR after reset)
    if ((BIT(data, 3) != BIT(m_fdc_control_reg, 3)) || m_fdc_control_reg < 0) 
    {
        //floppy_image_device *floppy = nullptr;
        for (int i = 1; i < m_num_of_drives; i++)
        {  
            LOGFDC("Flop %d motor = %s \n", i, BIT(data, 3) ? "on" : "off");
            if (m_floppy[i])
            {
                m_floppy[i]->get_device()->mon_w((BIT(data, 3) ? P2000T_FDC_MOTOR_ON : P2000T_FDC_MOTOR_OFF));
            }
            else
            {
                LOGFDC("Skip Flop %d\n", i);
            }
        }
    }
    
    // Store BIT(data,0); -->  FDC enable
    m_fdc_control_reg = data;
}

uint8_t p2000_fdc_device::fdc_fcdr()
{
    // DRQ line is represented in BIT 0 of port 90h
    return (m_fdc->get_drq() ? 0x01 : 0x0);
}

/* -------------------------------------------------
FDC - Ready control logic 
 FNR-N is connected to trg1 of ctc
    If upd765-HDL line is high -> logic is disabled and thus FNR-N line is low (FNR-N is connected to trg1 of ctc)
    FNR-N line gives single pulse when INDEX-N pulse was not dectected within defined time frame (5 disk revolutions)
    this mechanism was design to detect disk absense in drive (or no drive connect at all)
------------------------------------------------- */
WRITE_LINE_MEMBER(p2000_fdc_device::fdc_index_trigger)
{
    LOGFDC("FDC Index trigger (state = %02x)\n", state);
    if (m_fdc_hdl_line) 
    {
        // falling edge --> negative puls restart ready control network 
        if (m_fdc_index_n && !state) 
        {
            if (m_ready_control_pulse_cnt < 1)
            {
                // Only 1 index pulse restarts allowed per HDL line pulse --> one revoluition per read/write 
                LOGFDC("FDC Index trigger restart timer (pulse cnt %d) \n", m_ready_control_pulse_cnt);
                m_ready_control_timer->adjust(attotime::from_msec(m_ready_control_delay)); // single shot
                m_ready_control_pulse_cnt++;
            }
        }
    }
    m_fdc_index_n = state;
}

//-------------------------------------------------
//  FDC - implement WR trigger circuit
//-------------------------------------------------
WRITE_LINE_MEMBER(p2000_fdc_device::fdc_hdl_wr_trigger)
{
    m_fdc_hdl_line = state;
    if (state)
    {
        LOGFDC("FDC HDL WR trigger -> Set ready control timer\n");
		m_ready_control_timer->adjust(attotime::from_msec(m_ready_control_delay)); // single shot
    }
    else
    {   // Enabling the upd765-HDL line will cause a drop of trg1 (FNR-N line)
        LOGFDC("FDC HDL WR trigger -> Reset control timer\n");
        m_ctc->trg1(0);
        m_ready_control_timer->reset();
        m_ready_control_pulse_cnt = 0;
    }
}

//-------------------------------------------------
//  FDC - floppy controller int triggers ctc channel 0
//-------------------------------------------------
WRITE_LINE_MEMBER(p2000_fdc_device::fdc_irq_trigger)
{
    LOGIRQ("FDC IRQ %1x\n", state);
    m_ctc->trg0(state);
}

//-------------------------------------------------
//  FDC - implement read trigger ctc channel 1 (FDC error trigger)
//-------------------------------------------------
TIMER_CALLBACK_MEMBER(p2000_fdc_device::ready_timer_cb)
{
    LOGFDC("Ready control trigger (hdl = %1x)\n", m_fdc_hdl_line);
    // If the ready logic timer expired signal CTC trigger 1
    if (m_fdc_hdl_line) 
    {
        m_ctc->trg1(1);
        m_ctc->trg1(0);
    }
}

//-------------------------------------------------
//  dew_r - vblank trigger server check
//-------------------------------------------------
uint8_t p2000_fdc_device::dew_r()
{
	m_ctc->trg3(1);
    m_ctc->trg3(0);
    return 1;
}

/* -------------------------------------------------
    RAM disk
     1 sector has 256 bytes
     2 track has 16 sectors
     max 16 or 64 tracks 
------------------------------------------------- */
//-------------------------------------------------
//  RAM disk - set track number
//-------------------------------------------------
void p2000_m2200_multipurpose_device::port_95_w(uint8_t data) 
{
    m_ramdisk_track = data & get_ramdrive_max_tracks_mask();
    LOGRAMDSK("RAMDISK set trk=%d\n", m_ramdisk_track); 
} 

//-------------------------------------------------
//  RAM disk - read track number
//-------------------------------------------------
uint8_t p2000_m2200_multipurpose_device::port_95_r() 
{ 
    return m_ramdisk_track; 
}

//-------------------------------------------------
//  RAM disk - set sector number and reset byte counter
//-------------------------------------------------
void p2000_m2200_multipurpose_device::port_96_w(uint8_t data) 
{ 
    m_ramdisk_sector = data & 0x0F; // Max 16 sectors
    m_ramdisk_index = 0; // Reset "byte in sector"-counter
    LOGRAMDSK("RAMDISK set sec=%d\n", m_ramdisk_sector);
}

//-------------------------------------------------
//  RAM disk - read sector number
//-------------------------------------------------
uint8_t p2000_m2200_multipurpose_device::port_96_r() 
{ 
    return m_ramdisk_sector; 
} 

//-------------------------------------------------
//  RAM disk - read byte number and inc byte counter
//-------------------------------------------------
uint8_t p2000_m2200_multipurpose_device::port_97_r()
{
    uint8_t b = m_ramdisk[(m_ramdisk_track * 16 * 256) + (m_ramdisk_sector * 256) + m_ramdisk_index];
    m_ramdisk_index = (m_ramdisk_index + 1) & 0xff;
    return b;
}

//-------------------------------------------------
//  RAM disk - write byte number and inc byte counter
//-------------------------------------------------
void p2000_m2200_multipurpose_device::port_97_w(uint8_t data)
{
    m_ramdisk[(m_ramdisk_track * 16 * 256) + (m_ramdisk_sector * 256) + m_ramdisk_index] = data;
    m_ramdisk_index = (m_ramdisk_index + 1) & 0xff;
}

//-------------------------------------------------
//  Centronics - data register
//-------------------------------------------------
void p2000_m2200_multipurpose_device::port_98_w(uint8_t data)
{
    m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}

/* -------------------------------------------------
     Centronics - get status register
       BIT
        0    ackn
        1    busy
        2    paper empty
        3    printer on
        4    error-n
------------------------------------------------- */
WRITE_LINE_MEMBER(p2000_m2200_multipurpose_device::centronics_ack_w)
{
    LOGCENT("ACK-N %02x\n", state);
    if (state)
        m_centronics_status |= 0x1;
     else
        m_centronics_status &= ~0x1;
}

WRITE_LINE_MEMBER(p2000_m2200_multipurpose_device::centronics_busy_w)
{
    LOGCENT("BUSY-N %02x\n", state);
    if (state)
        m_centronics_status |= 0x2;
     else
        m_centronics_status &= ~0x2;
}

WRITE_LINE_MEMBER(p2000_m2200_multipurpose_device::centronics_paper_empty_w)
{
    LOGCENT("EMPTY-N %02x\n", state);
    if (state)
        m_centronics_status |= 0x4;
     else
        m_centronics_status &= ~0x4;
}

WRITE_LINE_MEMBER(p2000_m2200_multipurpose_device::centronics_printer_on_w)
{
    LOGCENT("printer-N %02x\n", state);
    if (state)
        m_centronics_status |= 0x8;
     else
        m_centronics_status &= ~0x8;
}

WRITE_LINE_MEMBER(p2000_m2200_multipurpose_device::centronics_error_w)
{
    LOGCENT("ERROR-N %02x\n", state);
    if (!state) // Inverted
        m_centronics_status |= 0x10;
     else
        m_centronics_status &= ~0x10;
}

uint8_t p2000_m2200_multipurpose_device::port_99_r()
{
    return m_centronics_status;
}

//-------------------------------------------------
//  Centronics - accessing port 9a will set strobe signal
//-------------------------------------------------

void p2000_m2200_multipurpose_device::port_9a_w(uint8_t data)
{
    LOGCENT("strobe-N 1\n");
    m_centronics->write_strobe(1);
}

uint8_t p2000_m2200_multipurpose_device::port_9a_r()
{
    LOGCENT("strobe-N 1\n");
    m_centronics->write_strobe(1);
    return 0;
}

//-------------------------------------------------
//  Centronics - accessing port 9a will reset strobe signal
//-------------------------------------------------
void p2000_m2200_multipurpose_device::port_9b_w(uint8_t data)
{
    LOGCENT("strobe-N 0\n");
    m_centronics->write_strobe(0);
}

uint8_t p2000_m2200_multipurpose_device::port_9b_r()
{
    LOGCENT("strobe-N 0\n");
    m_centronics->write_strobe(0);
    return 0;
}
    
