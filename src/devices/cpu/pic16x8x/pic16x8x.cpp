// license: BSD-3-Clause
// copyright-holders: Tony La Porta, Grull Osgo, Dirk Best
/****************************************************************************

    Microchip PIC16 Mid-Range Devices

    14-bit instruction width, 8 level stack, interrupts

    PIC firmware image layout (16-bit data):
    - 0x0000 to 0x0fff: program data (may be less, depending on model)
    - 0x1000 to 0x3fff: empty
    - 0x4000 to 0x400f: user and configuration data
    - 0x4010 to 0x41ff: empty
    - 0x4200 to 0x42ff: eeprom data, 8-bit (may be less, depending on model)

    TODO:
    - Interrupt timing
    - EEPROM timing
    - Missing PIC16F628A features: Timer1, Timer2, USART, etc.

****************************************************************************/

#include "emu.h"
#include "pic16x8x.h"
#include "16x8xdsm.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// CONFIG register
constexpr u8 WDTE_FLAG = 0x04; // watchdog enable
constexpr u8 FOSC_FLAG = 0x03; // oscillator source select

// STATUS register
constexpr u8 IRP_FLAG = 0x80; // indirect register bank select
constexpr u8 RP1_FLAG = 0x40; // direct register bank select bit 1
constexpr u8 RP0_FLAG = 0x20; // direct register bank select bit 0
constexpr u8 TO_FLAG  = 0x10; // time out (watch dog) flag
constexpr u8 PD_FLAG  = 0x08; // power-down flag
constexpr u8 Z_FLAG   = 0x04; // zero flag
constexpr u8 DC_FLAG  = 0x02; // digit carry/borrow flag
constexpr u8 C_FLAG   = 0x01; // carry/borrow flag

// OPTION register
constexpr u8 RBPU_FLAG   = 0x80; // PORTB pull-up enable
constexpr u8 INTEDG_FLAG = 0x40; // interrupt edge select
constexpr u8 T0CS_FLAG   = 0x20; // TMR0 clock source select
constexpr u8 T0SE_FLAG   = 0x10; // TMR0 source edge select
constexpr u8 PSA_FLAG    = 0x08; // prescaler assignment
constexpr u8 PS_REG      = 0x07; // prescaler rate select

// INTCON register
constexpr u8 GIE_FLAG  = 0x80;
constexpr u8 EEIE_FLAG = 0x40;
constexpr u8 PEIE_FLAG = 0x40; // PIC16F628A
constexpr u8 T0IE_FLAG = 0x20;
constexpr u8 INTE_FLAG = 0x10;
constexpr u8 RBIE_FLAG = 0x08;
constexpr u8 T0IF_FLAG = 0x04;
constexpr u8 INTF_FLAG = 0x02;
constexpr u8 RBIF_FLAG = 0x01;

// EECON1 register
constexpr u8 EEIF_FLAG  = 0x10;
constexpr u8 WRERR_FLAG = 0x08;
constexpr u8 WREN_FLAG  = 0x04;
constexpr u8 EEWR_FLAG  = 0x02;
constexpr u8 EERD_FLAG  = 0x01;

// PCON register
constexpr u8 OSCF_FLAG = 0x08; // oscillator frequency
constexpr u8 POR_FLAG  = 0x02; // power-on reset
constexpr u8 BOR_FLAG  = 0x01; // brown-out reset

// interrupt vectors
constexpr u8 RESET_VECTOR = 0x00;
constexpr u8 INT_VECTOR   = 0x04;

constexpr u16 PC_MASK = 0x1fff;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PIC16CR83,  pic16cr83_device,  "pic16cr83",  "Microchip PIC16CR83")
DEFINE_DEVICE_TYPE(PIC16CR84,  pic16cr84_device,  "pic16cr84",  "Microchip PIC16CR84")
DEFINE_DEVICE_TYPE(PIC16F83,   pic16f83_device,   "pic16f83",   "Microchip PIC16F83")
DEFINE_DEVICE_TYPE(PIC16F84,   pic16f84_device,   "pic16f84",   "Microchip PIC16F84")
DEFINE_DEVICE_TYPE(PIC16F84A,  pic16f84a_device,  "pic16f84a",  "Microchip PIC16F84A")
DEFINE_DEVICE_TYPE(PIC16F628A, pic16f628a_device, "pic16f628a", "Microchip PIC16F628A")

pic16x8x_device::pic16x8x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, address_map_constructor program_map, address_map_constructor data_map, u16 eeprom_size, u8 status_mask, u8 porta_mask)
	: cpu_device(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_program_width(program_width)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, program_width, -1, program_map)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 9, 0, data_map)
	, m_region(*this, DEVICE_SELF)
	, m_CONFIG(0x3fff)
	, m_porta_mask(porta_mask)
	, m_status_mask(status_mask)
	, m_internal_eeprom_size(eeprom_size)
	, m_read_port(*this, 0)
	, m_write_port(*this)
{
}

pic16x83_device::pic16x83_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: pic16x8x_device(mconfig, type, tag, owner, clock, 9, address_map_constructor(FUNC(pic16x8x_device::rom_9), this), address_map_constructor(FUNC(pic16x8x_device::ram_6), this), 64, 0x3f, 0x1f)
{
}

pic16x84_device::pic16x84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: pic16x8x_device(mconfig, type, tag, owner, clock, 10, address_map_constructor(FUNC(pic16x8x_device::rom_10), this), address_map_constructor(FUNC(pic16x8x_device::ram_7), this), 64, 0x3f, 0x1f)
{
}

pic16cr83_device::pic16cr83_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x83_device(mconfig, PIC16CR83, tag, owner, clock)
{
}

pic16cr84_device::pic16cr84_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x84_device(mconfig, PIC16CR84, tag, owner, clock)
{
}

pic16f83_device::pic16f83_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x83_device(mconfig, PIC16F83, tag, owner, clock)
{
}

pic16f84_device::pic16f84_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x84_device(mconfig, PIC16F84, tag, owner, clock)
{
}

pic16f84a_device::pic16f84a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x84_device(mconfig, PIC16F84A, tag, owner, clock)
{
}

pic16f628a_device::pic16f628a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x8x_device(mconfig, PIC16F628A, tag, owner, clock, 11, address_map_constructor(FUNC(pic16x8x_device::rom_11), this), address_map_constructor(FUNC(pic16f628a_device::data_map), this), 128, 0xff, 0xff)
{
}


//**************************************************************************
//  INTERNAL MEMORY MAPS
//**************************************************************************

void pic16x8x_device::rom_9(address_map &map)
{
	map(0x0000, 0x01ff).rom();
}

void pic16x8x_device::rom_10(address_map &map)
{
	map(0x0000, 0x03ff).rom();
}

void pic16x8x_device::rom_11(address_map &map)
{
	map(0x0000, 0x07ff).rom();
}

void pic16x8x_device::rom_12(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void pic16x8x_device::core_regs(address_map &map, u8 mirror)
{
	map(0x00, 0x00).noprw().mirror(mirror); // "indirect addr." - not a physical register
	map(0x01, 0x01).rw(FUNC(pic16x8x_device::tmr0_r), FUNC(pic16x8x_device::tmr0_w));
	map(0x02, 0x02).rw(FUNC(pic16x8x_device::pcl_r), FUNC(pic16x8x_device::pcl_w)).mirror(mirror);
	map(0x03, 0x03).rw(FUNC(pic16x8x_device::status_r), FUNC(pic16x8x_device::status_w)).mirror(mirror);
	map(0x04, 0x04).rw(FUNC(pic16x8x_device::fsr_r), FUNC(pic16x8x_device::fsr_w)).mirror(mirror);
	map(0x05, 0x05).rw(FUNC(pic16x8x_device::porta_r), FUNC(pic16x8x_device::porta_w));
	map(0x06, 0x06).rw(FUNC(pic16x8x_device::portb_r), FUNC(pic16x8x_device::portb_w));
	map(0x07, 0x07).noprw().mirror(mirror); // not a physical register, returns 0
	map(0x08, 0x08).rw(FUNC(pic16x8x_device::eedata_r), FUNC(pic16x8x_device::eedata_w));
	map(0x09, 0x09).rw(FUNC(pic16x8x_device::eeadr_r), FUNC(pic16x8x_device::eeadr_w));
	map(0x0a, 0x0a).rw(FUNC(pic16x8x_device::pclath_r), FUNC(pic16x8x_device::pclath_w)).mirror(mirror);
	map(0x0b, 0x0b).rw(FUNC(pic16x8x_device::intcon_r), FUNC(pic16x8x_device::intcon_w)).mirror(mirror);
	map(0x81, 0x81).rw(FUNC(pic16x8x_device::option_r), FUNC(pic16x8x_device::option_w));
	map(0x85, 0x85).rw(FUNC(pic16x8x_device::trisa_r), FUNC(pic16x8x_device::trisa_w));
	map(0x86, 0x86).rw(FUNC(pic16x8x_device::trisb_r), FUNC(pic16x8x_device::trisb_w));
	map(0x88, 0x88).rw(FUNC(pic16x8x_device::eecon1_r), FUNC(pic16x8x_device::eecon1_w));
	map(0x89, 0x89).rw(FUNC(pic16x8x_device::eecon2_r), FUNC(pic16x8x_device::eecon2_w));
}

void pic16x8x_device::ram_6(address_map &map)
{
	// 0x00 - 0x0b SFR's Bank 0
	// 0x0c - 0x2f GPR's
	// 0x80 - 0x8b SFR's Bank 1
	// 0x8c - 0xaf GPR Mirrored to 0x0c - 0x2f
	core_regs(map, 0x80);
	map(0x0c, 0x2f).ram().mirror(0x80);
}

void pic16x8x_device::ram_7(address_map &map)
{
	// 0x00 - 0x0b SFR's Bank 0
	// 0x0c - 0x4f GPR's
	// 0x80 - 0x8b SFR's Bank 1
	// 0x8c - 0xcf GPR Mirrored to 0x0c - 0x4f
	core_regs(map, 0x80);
	map(0x0c, 0x4f).ram().mirror(0x80);
}

void pic16f628a_device::data_map(address_map &map)
{
	// bank 0
	map(0x000, 0x000).mirror(0x180).noprw(); // "indirect addr." - not a physical register
	map(0x001, 0x001).mirror(0x100).rw(FUNC(pic16f628a_device::tmr0_r), FUNC(pic16f628a_device::tmr0_w));
	map(0x002, 0x002).mirror(0x180).rw(FUNC(pic16f628a_device::pcl_r), FUNC(pic16f628a_device::pcl_w));
	map(0x003, 0x003).mirror(0x180).rw(FUNC(pic16f628a_device::status_r), FUNC(pic16f628a_device::status_w));
	map(0x004, 0x004).mirror(0x180).rw(FUNC(pic16f628a_device::fsr_r), FUNC(pic16f628a_device::fsr_w));
	map(0x005, 0x005).rw(FUNC(pic16f628a_device::porta_r), FUNC(pic16f628a_device::porta_w));
	map(0x006, 0x006).mirror(0x100).rw(FUNC(pic16f628a_device::portb_r), FUNC(pic16f628a_device::portb_w));
	map(0x007, 0x007).mirror(0x180).noprw();
	map(0x008, 0x008).mirror(0x180).noprw();
	map(0x009, 0x009).mirror(0x180).noprw();
	map(0x00a, 0x00a).mirror(0x180).rw(FUNC(pic16f628a_device::pclath_r), FUNC(pic16f628a_device::pclath_w));
	map(0x00b, 0x00b).mirror(0x180).rw(FUNC(pic16f628a_device::intcon_r), FUNC(pic16f628a_device::intcon_w));
	map(0x00c, 0x00c).rw(FUNC(pic16f628a_device::pir1_r), FUNC(pic16f628a_device::pir1_w));
	map(0x00d, 0x00d).mirror(0x180).noprw();
	map(0x00e, 0x00e).rw(FUNC(pic16f628a_device::tmr1l_r), FUNC(pic16f628a_device::tmr1l_w));
	map(0x00f, 0x00f).rw(FUNC(pic16f628a_device::tmr1h_r), FUNC(pic16f628a_device::tmr1h_w));
	map(0x010, 0x010).rw(FUNC(pic16f628a_device::t1con_r), FUNC(pic16f628a_device::t1con_w));
	map(0x011, 0x011).rw(FUNC(pic16f628a_device::tmr2_r), FUNC(pic16f628a_device::tmr2_w));
	map(0x012, 0x012).rw(FUNC(pic16f628a_device::t2con_r), FUNC(pic16f628a_device::t2con_w));
	map(0x013, 0x013).mirror(0x180).noprw();
	map(0x014, 0x014).mirror(0x180).noprw();
	map(0x015, 0x015).rw(FUNC(pic16f628a_device::ccpr1l_r), FUNC(pic16f628a_device::ccpr1l_w));
	map(0x016, 0x016).rw(FUNC(pic16f628a_device::ccpr1h_r), FUNC(pic16f628a_device::ccpr1h_w));
	map(0x017, 0x017).rw(FUNC(pic16f628a_device::ccp1con_r), FUNC(pic16f628a_device::ccp1con_w));
	map(0x018, 0x018).rw(FUNC(pic16f628a_device::rcsta_r), FUNC(pic16f628a_device::rcsta_w));
	map(0x019, 0x019).rw(FUNC(pic16f628a_device::txreg_r), FUNC(pic16f628a_device::txreg_w));
	map(0x01a, 0x01a).rw(FUNC(pic16f628a_device::rcreg_r), FUNC(pic16f628a_device::rcreg_w));
	map(0x01b, 0x01b).noprw();
	map(0x01c, 0x01c).noprw();
	map(0x01d, 0x01d).noprw();
	map(0x01e, 0x01e).mirror(0x180).noprw();
	map(0x01f, 0x01f).rw(FUNC(pic16f628a_device::cmcon_r), FUNC(pic16f628a_device::cmcon_w));
	map(0x020, 0x06f).ram();
	map(0x070, 0x07f).mirror(0x180).ram();
	// bank 1
	map(0x081, 0x081).mirror(0x100).rw(FUNC(pic16f628a_device::option_r), FUNC(pic16f628a_device::option_w));
	map(0x085, 0x085).rw(FUNC(pic16f628a_device::trisa_r), FUNC(pic16f628a_device::trisa_w));
	map(0x086, 0x086).mirror(0x100).rw(FUNC(pic16f628a_device::trisb_r), FUNC(pic16f628a_device::trisb_w));
	map(0x08c, 0x08c).rw(FUNC(pic16f628a_device::pie1_r), FUNC(pic16f628a_device::pie1_w));
	map(0x08e, 0x08e).rw(FUNC(pic16f628a_device::pcon_r), FUNC(pic16f628a_device::pcon_w));
	map(0x08f, 0x08f).noprw();
	map(0x090, 0x090).noprw();
	map(0x091, 0x091).noprw();
	map(0x092, 0x092).rw(FUNC(pic16f628a_device::pr2_r), FUNC(pic16f628a_device::pr2_w));
	map(0x095, 0x095).noprw();
	map(0x096, 0x096).noprw();
	map(0x097, 0x097).noprw();
	map(0x098, 0x098).rw(FUNC(pic16f628a_device::txsta_r), FUNC(pic16f628a_device::txsta_w));
	map(0x099, 0x099).rw(FUNC(pic16f628a_device::spbrg_r), FUNC(pic16f628a_device::spbrg_w));
	map(0x09a, 0x09a).rw(FUNC(pic16f628a_device::eedata_r), FUNC(pic16f628a_device::eedata_w));
	map(0x09b, 0x09b).rw(FUNC(pic16f628a_device::eeadr_r), FUNC(pic16f628a_device::eeadr_w));
	map(0x09c, 0x09c).rw(FUNC(pic16f628a_device::eecon1_r), FUNC(pic16f628a_device::eecon1_w));
	map(0x09d, 0x09d).rw(FUNC(pic16f628a_device::eecon2_r), FUNC(pic16f628a_device::eecon2_w));
	map(0x09f, 0x09f).rw(FUNC(pic16f628a_device::vrcon_r), FUNC(pic16f628a_device::vrcon_w));
	map(0x0a0, 0x0ef).ram();
	// bank 2
	map(0x105, 0x105).mirror(0x080).noprw();
	map(0x10c, 0x10c).mirror(0x080).noprw();
	map(0x10e, 0x10e).mirror(0x080).noprw();
	map(0x10f, 0x10f).mirror(0x080).noprw();
	map(0x110, 0x11f).mirror(0x080).noprw();
	map(0x120, 0x14f).ram();
	map(0x150, 0x16f).mirror(0x080).noprw();
	// bank 3
	map(0x190, 0x1ef).noprw();
}

device_memory_interface::space_config_vector pic16x8x_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//**************************************************************************
//  START/RESET
//**************************************************************************

void pic16x8x_device::device_start()
{
	space(AS_PROGRAM).cache(m_program);
	space(AS_DATA).specific(m_data);

	m_program_mask = (1 << m_program_width) - 1;
	m_PC = 0;
	m_PREVPC = 0;
	m_W = 0;
	m_OPTION = 0;
	m_ALU = 0;
	m_TMR0 = 0;
	m_STATUS = 0;
	m_FSR = 0;
	m_PCLATH = 0;
	m_INTCON = 0;
	m_EEDATA = 0; // actually unknown state
	m_EEADR = 0; // actually unknown state
	m_EECON1 = 0;
	std::fill(std::begin(m_port_data), std::end(m_port_data), 0);
	std::fill(std::begin(m_port_tris), std::end(m_port_tris), 0);
	std::fill(std::begin(m_STACK), std::end(m_STACK), 0);
	m_prescaler = 0;
	m_opcode.w = 0;
	m_delay_timer = 0;
	m_rtcc = 0;
	m_count_cycles = 0;
	m_inst_cycles = 0;
	m_status_write_protect = TO_FLAG | PD_FLAG;
	m_stack_pointer = 0;
	m_debugger_temp = 0;
	m_portb_mismatch_detect = 0xff;
	m_sleeping = false;

	m_eeprom_data = std::make_unique<u8[]>(m_internal_eeprom_size);
	m_eeprom_unlock_state = EEPROM_LOCKED;

	// setup watchdog timer
	m_wdt_timer = timer_alloc(FUNC(pic16x8x_device::wdt_timeout), this);

	// fetch configuration bits from firmware image if available
	if (m_region.found() && m_region->bytewidth() == 2 && (m_region->length() > 0x2007))
	{
		m_CONFIG = m_region->as_u16(0x2007);
		logerror("Writing %04x to the PIC16x8x configuration bits\n", m_CONFIG);
		restart_wdt();
	}

	// save states
	save_item(NAME(m_PC));
	save_item(NAME(m_PREVPC));
	save_item(NAME(m_CONFIG));
	save_item(NAME(m_W));
	save_item(NAME(m_OPTION));
	save_item(NAME(m_ALU));
	save_item(NAME(m_TMR0));
	save_item(NAME(m_STATUS));
	save_item(NAME(m_FSR));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_tris));
	save_item(NAME(m_EEDATA));
	save_item(NAME(m_EEADR));
	save_item(NAME(m_EECON1));
	save_item(NAME(m_PCLATH));
	save_item(NAME(m_INTCON));
	save_item(NAME(m_STACK));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_opcode.w));
	save_item(NAME(m_delay_timer));
	save_item(NAME(m_rtcc));
	save_item(NAME(m_count_cycles));
	save_item(NAME(m_status_write_protect));
	save_item(NAME(m_inst_cycles));
	save_item(NAME(m_stack_pointer));
	save_item(NAME(m_rb0));
	save_item(NAME(m_portb_mismatch_detect));
	save_item(NAME(m_sleeping));
	save_item(NAME(m_eeprom_unlock_state));
	save_pointer(NAME(m_eeprom_data), m_internal_eeprom_size);

	// debugger
	state_add(PIC16X8x_PC,     "PC",   m_PC).mask(PC_MASK).formatstr("%04X");
	state_add(PIC16X8x_W,      "W",    m_W).formatstr("%02X");
	state_add(PIC16X8x_ALU,    "ALU",  m_ALU).formatstr("%02X");
	state_add(PIC16X8x_CONFIG, "CNF",  m_CONFIG).formatstr("%04X");
	state_add(PIC16X8x_PSCL,   "PSCL", m_debugger_temp).callimport().formatstr("%3s");

	state_add(STATE_GENPC,     "GENPC",    m_PC).noshow();
	state_add(STATE_GENPCBASE, "CURPC",    m_PREVPC).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS", m_OPTION).formatstr("%13s").noshow();

	set_icountptr(m_icount);
}

void pic16f628a_device::device_start()
{
	pic16x8x_device::device_start();

	// register for save states
	save_item(NAME(m_PIR1));
	save_item(NAME(m_T1CON));
	save_item(NAME(m_T2CON));
	save_item(NAME(m_CCP1CON));
	save_item(NAME(m_RCSTA));
	save_item(NAME(m_CMCON));
	save_item(NAME(m_PIE1));
	save_item(NAME(m_PCON));
	save_item(NAME(m_TXSTA));
	save_item(NAME(m_VRCON));
}

void pic16x8x_device::device_reset()
{
	set_pc(RESET_VECTOR);
	m_PREVPC = m_PC;

	m_port_tris[PORTA] = 0xff;
	m_port_tris[PORTB] = 0xff;
	m_OPTION = 0xff;
	m_STATUS = TO_FLAG | PD_FLAG;
	m_PCLATH = 0;
	m_INTCON = 0;
	m_EECON1 = 0;

	m_prescaler = 0;
	m_delay_timer = 0;
	m_inst_cycles = 0;
	m_count_cycles = 0;
	m_status_write_protect = TO_FLAG | PD_FLAG;
	m_stack_pointer = 0;
	m_portb_mismatch_detect = 0xff;
	m_rb0 = 0;

	m_sleeping = false;
	m_eeprom_unlock_state = EEPROM_LOCKED;

	restart_wdt();
}

void pic16f628a_device::device_reset()
{
	pic16x8x_device::device_reset();

	m_PIR1 = 0x00;
	m_T1CON = 0x00;
	m_T2CON = 0x00;
	m_CCP1CON = 0x00;
	m_RCSTA = 0x00;
	m_CMCON = 0x00;
	m_PIE1 = 0x00;
	m_PCON = 0x08;
	m_TXSTA = 0x02;
	m_VRCON = 0x00;
}


//**************************************************************************
//  DEBUGGER
//**************************************************************************

void pic16x8x_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case PIC16X8x_PSCL:
			m_prescaler = m_debugger_temp;
			break;
	}
}

void pic16x8x_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	}
}

void pic16x8x_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case PIC16X8x_PSCL:
			str = string_format("%c%02X", ((m_OPTION & 0x08) ? 'W' : 'T'), m_prescaler);
			break;

		case STATE_GENFLAGS:
			str = string_format("%01x%c%c%c%c%c %c%c%c%03x",
				(m_STATUS & 0xe0) >> 5,
				m_STATUS & 0x10 ? '.':'O',      // WDT Overflow
				m_STATUS & 0x08 ? 'P':'D',      // Power/Down
				m_STATUS & 0x04 ? 'Z':'.',      // Zero
				m_STATUS & 0x02 ? 'c':'b',      // Nibble Carry/Borrow
				m_STATUS & 0x01 ? 'C':'B',      // Carry/Borrow

				m_OPTION & 0x20 ? 'C':'T',    // Counter/Timer
				m_OPTION & 0x10 ? 'N':'P',    // Negative/Positive
				m_OPTION & 0x08 ? 'W':'T',    // WatchDog/Timer
				m_OPTION & 0x08 ? (1<<(m_OPTION&7)) : (2<<(m_OPTION&7)));
			break;
	}
}

std::unique_ptr<util::disasm_interface> pic16x8x_device::create_disassembler()
{
	return std::make_unique<pic16x8x_disassembler>();
}


//**************************************************************************
//  EXECUTION
//**************************************************************************

void pic16x8x_device::execute_run()
{
	do
	{
		// check interrupts
		check_irqs();

		if (m_sleeping)
		{
			m_count_cycles = 0;
			m_inst_cycles = 1;

			debugger_instruction_hook(m_PC);
		}
		else
		{
			m_PREVPC = m_PC;

			debugger_instruction_hook(m_PC);

			m_opcode.w = m_program.read_word(m_PC);
			set_pc(m_PC + 1);

			const pic16x8x_opcode *op;

			if ((m_opcode.w & 0x3f80) != 0x0000)
				op = &s_opcode_main[(m_opcode.w >> 7) & 0x7f];
			else
				op = &s_opcode_00x[m_opcode.b.l & 0x7f];

			m_inst_cycles = op->cycles;
			m_status_write_protect = TO_FLAG | PD_FLAG;

			if (op->affects_alu_flags)
				m_status_write_protect |= Z_FLAG | DC_FLAG | C_FLAG;

			(this->*op->function)();

			m_status_write_protect = TO_FLAG | PD_FLAG;

			update_timer((m_OPTION & T0CS_FLAG) ? m_count_cycles : m_inst_cycles);
			m_count_cycles = 0;
		}

		m_icount -= m_inst_cycles;

	} while (m_icount > 0);
}

void pic16x8x_device::execute_set_input(int line, int state)
{
	switch (line)
	{
		// RTCC/RA4_T0CKI pin
		case PIC16x8x_T0CKI:
			if ((m_OPTION & T0CS_FLAG) && state != m_rtcc)
			{ // Count mode, edge triggered
				if (((m_OPTION & T0SE_FLAG) && !state) || (!(m_OPTION & T0SE_FLAG) && state))
					m_count_cycles++;
			}
			m_rtcc = state;
			break;

		case PIC16x8x_RB0INT:
		{
			const u8 new_rb0 = (state != CLEAR_LINE) ? 1 : 0;
			if (new_rb0 != m_rb0)
			{
				const bool rising = (new_rb0 > m_rb0);
				const bool trigger_rising = (m_OPTION & INTEDG_FLAG);

				if (rising == trigger_rising)
					m_INTCON |= INTF_FLAG;

				m_rb0 = new_rb0;
			}
			break;
		}

		default:
			break;
	}
}


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

bool pic16x8x_device::irq_active() const
{
	// check T0I, INT and RBI
	if ((m_INTCON >> 3) & m_INTCON & 0x07)
		return true;

	// check EEI
	if ((m_INTCON & EEIE_FLAG) && (m_EECON1 & EEIF_FLAG))
		return true;

	return false;
}

bool pic16f628a_device::irq_active() const
{
	// check T0I, INT, RBI
	if ((m_INTCON >> 3) & m_INTCON & 0x07)
		return true;

	// check PEI
	if ((m_INTCON & PEIE_FLAG) && (m_PIR1 & m_PIE1))
		return true;

	return false;
}

void pic16x8x_device::check_irqs()
{
	// port b change detection
	const u8 input_mask = m_port_tris[PORTB] & 0xf0;
	if (input_mask)
	{
		const u8 current_inputs = m_read_port[PORTB](PORTB, input_mask) & input_mask;
		if (current_inputs != (m_portb_mismatch_detect & input_mask))
			m_INTCON |= RBIF_FLAG;
	}

	// any interrupts currently active?
	if (!irq_active())
		return;

	// interrupts wake the cpu from sleep
	if (m_sleeping)
	{
		m_sleeping = false;
		restart_wdt();

		// we need to exit here since the instruction after SLEEP needs to be executed first
		return;
	}

	// service interrupt if GIE is set
	if (m_INTCON & GIE_FLAG)
	{
		m_INTCON &= ~GIE_FLAG;
		push_stack(m_PC);
		set_pc(INT_VECTOR);
		standard_irq_callback(0, m_PC);
	}
}


//**************************************************************************
//  TIMER
//**************************************************************************

void pic16x8x_device::update_timer(int counts)
{
	if (m_delay_timer > 0)
	{
		// timer increment inhibited
		int dt = m_delay_timer;
		m_delay_timer -= m_inst_cycles;
		counts -= dt;
	}

	if (m_delay_timer > 0 || counts <= 0)
		return;

	int tmr_inc = 0;

	if ((m_OPTION & PSA_FLAG) == 0)
	{
		// prescaler assigned to Timer0
		m_prescaler += counts;
		const int div = 2 << (m_OPTION & PS_REG);

		if (m_prescaler >= div)
		{
			tmr_inc = m_prescaler / div;
			m_prescaler %= div;
		}
	}
	else
	{
		// prescaler assigned to WDT, increment directly
		tmr_inc = counts;
	}

	if (tmr_inc > 0)
	{
		const u16 new_tmr0 = m_TMR0 + tmr_inc;

		if (new_tmr0 > 0xff)
			m_INTCON |= T0IF_FLAG;

		m_TMR0 = new_tmr0;
	}
}


//**************************************************************************
//  WATCHDOG
//**************************************************************************

TIMER_CALLBACK_MEMBER( pic16x8x_device::wdt_timeout )
{
	if (m_sleeping)
	{
		// when sleeping we just wake up
		m_sleeping = false;
		m_STATUS &= ~(TO_FLAG | PD_FLAG);
	}
	else
	{
		// during normal operation cause a reset
		wdt_reset();
	}

	restart_wdt();
}

void pic16x8x_device::wdt_reset()
{
	const u8 status_unaffected = m_STATUS & (PD_FLAG | Z_FLAG | DC_FLAG | C_FLAG);
	device_reset();
	m_STATUS = (0 | status_unaffected);
}

void pic16f628a_device::wdt_reset()
{
	const u8 pcon_unaffected = m_PCON & (POR_FLAG | BOR_FLAG);
	pic16x8x_device::wdt_reset();
	m_PCON = (OSCF_FLAG | pcon_unaffected);
}

void pic16x8x_device::restart_wdt()
{
	if (m_CONFIG & WDTE_FLAG)
	{
		// watchdog base time is ~18 msec with an optional prescaler applied
		const int prescale = (m_OPTION & PSA_FLAG) ? (1 << (m_OPTION & PS_REG)) : 1;
		m_wdt_timer->adjust(attotime::from_msec(18 * prescale));
	}
	else
	{
		// watchdog disabled
		m_wdt_timer->adjust(attotime::never);
	}
}


//**************************************************************************
//  EEPROM
//**************************************************************************

void pic16x8x_device::nvram_default()
{
	// populate from a memory region if present
	if (m_region.found())
	{
		u16 eeprom_start = 0x4200;
		u16 dump_size = eeprom_start + (m_internal_eeprom_size * 2);

		// check pic dump total size
		if (m_region->bytes() != dump_size)
			fatalerror("Region '%s' wrong size (expected size = 0x%x)\n", tag(), dump_size);

		// check pic dump bit width
		if (m_region->bytewidth() != 2)
			fatalerror("Region '%s' needs to be an 16-bit region\n", tag());

		// copy and convert eeprom data from memory region
		u16 const *const src = reinterpret_cast<u16 const *>(m_region->base() + eeprom_start);

		for (u16 i = 0; i < m_internal_eeprom_size; i++)
			m_eeprom_data[i] = src[i] & 0x00ff;
	}
}

bool pic16x8x_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_eeprom_data.get(), m_internal_eeprom_size);
	return !err && (actual == m_internal_eeprom_size);
}

bool pic16x8x_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_eeprom_data.get(), m_internal_eeprom_size);
	return !err && (actual == m_internal_eeprom_size);
}

u8 pic16x8x_device::eeprom_read(offs_t offs)
{
	if (offs < m_internal_eeprom_size)
		return m_eeprom_data[offs];
	else
		return 0xff;
}

void pic16x8x_device::eeprom_write(offs_t offs, u8 data)
{
	if (offs < m_internal_eeprom_size)
		m_eeprom_data[offs] = data;
}


//**************************************************************************
//  SPECIAL FUNCTION REGISTER
//**************************************************************************

u8 pic16x8x_device::get_regfile(offs_t offset)
{
	// indirect addressing
	if ((offset & 0x7f) == 0)
		offset = ((m_STATUS & IRP_FLAG) << 1) | m_FSR;

	return m_data.read_byte(offset);
}

void pic16x8x_device::store_regfile(offs_t offset, u8 data)
{
	// indirect addressing
	if ((offset & 0x7f) == 0)
		offset = ((m_STATUS & IRP_FLAG) << 1) | m_FSR;

	m_data.write_byte(offset, data);
}

u8 pic16x8x_device::tmr0_r()
{
	return m_TMR0;
}

void pic16x8x_device::tmr0_w(u8 data)
{
	// delay for this instruction and the following 2 cycles
	m_delay_timer = m_inst_cycles + 2;

	if ((m_OPTION & PSA_FLAG) == 0)
		m_prescaler = 0; // clear the prescaler

	m_TMR0 = data;
}

u8 pic16x8x_device::pcl_r()
{
	return m_PC;
}

void pic16x8x_device::pcl_w(u8 data)
{
	set_pc((m_PCLATH << 8) | data);
	m_inst_cycles++;
}

u8 pic16x8x_device::status_r()
{
	return m_STATUS;
}

void pic16x8x_device::status_w(u8 data)
{
	m_STATUS = ((m_STATUS & m_status_write_protect) | (data & u8(~m_status_write_protect))) & m_status_mask;
}

u8 pic16x8x_device::fsr_r()
{
	return m_FSR;
}

void pic16x8x_device::fsr_w(u8 data)
{
	m_FSR = data;
}

u8 pic16x8x_device::porta_r()
{
	u8 data = m_read_port[PORTA](PORTA, 0xff);
	data &= m_port_tris[PORTA];
	data |= (u8(~m_port_tris[PORTA]) & m_port_data[PORTA]);

	return data & m_porta_mask;
}

void pic16x8x_device::porta_w(u8 data)
{
	data &= m_porta_mask;
	const u8 mask = u8(~m_port_tris[PORTA]) & m_porta_mask;
	m_port_data[PORTA] = data;
	m_write_port[PORTA](PORTA, data & mask, mask);
}

u8 pic16x8x_device::portb_r()
{
	u8 data = m_read_port[PORTB](PORTB, 0xff);
	data &= m_port_tris[PORTB];
	data |= (u8(~m_port_tris[PORTB]) & m_port_data[PORTB]);

	// reading PORTB updates the change detection latch for RB7:RB4
	m_portb_mismatch_detect = data & 0xf0;

	return data;
}

void pic16x8x_device::portb_w(u8 data)
{
	// a write access to PORTB performs an internal read of the port
	// we need to call it to update the mismatch detection
	portb_r();

	const u8 mask = u8(~m_port_tris[PORTB]);
	m_port_data[PORTB] = data;
	m_write_port[PORTB](PORTB, data & mask, mask);
}

u8 pic16x8x_device::eedata_r()
{
	return m_EEDATA;
}

void pic16x8x_device::eedata_w(u8 data)
{
	m_EEDATA = data;
}

u8 pic16x8x_device::eeadr_r()
{
	return m_EEADR;
}

void pic16x8x_device::eeadr_w(u8 data)
{
	m_EEADR = data;
}

u8 pic16x8x_device::pclath_r()
{
	return m_PCLATH;
}

void pic16x8x_device::pclath_w(u8 data)
{
	m_PCLATH = data & 0x1f;
}

u8 pic16x8x_device::intcon_r()
{
	return m_INTCON;
}

void pic16x8x_device::intcon_w(u8 data)
{
	m_INTCON = data;
}

u8 pic16x8x_device::trisa_r()
{
	return m_port_tris[PORTA];
}

void pic16x8x_device::trisa_w(u8 data)
{
	data |= ~m_porta_mask; // unimplemented bits are forced to 1
	if (m_port_tris[PORTA] != data)
	{
		m_port_tris[PORTA] = data;
		const u8 mask = u8(~m_port_tris[PORTA]) & m_porta_mask;
		m_write_port[PORTA](PORTA, m_port_data[PORTA] & mask, mask);
	}
}

u8 pic16x8x_device::trisb_r()
{
	return m_port_tris[PORTB];
}

void pic16x8x_device::trisb_w(u8 data)
{
	if (m_port_tris[PORTB] != data)
	{
		m_port_tris[PORTB] = data;
		m_write_port[PORTB](PORTB, m_port_data[PORTB] & u8(~m_port_tris[PORTB]), u8(~m_port_tris[PORTB]));
	}
}

void pic16x8x_device::set_eeif()
{
	m_EECON1 |= EEIF_FLAG;
}

void pic16f628a_device::set_eeif()
{
	m_PIR1 |= 0x80;
}

u8 pic16x8x_device::eecon1_r()
{
	return m_EECON1;
}

void pic16x8x_device::eecon1_w(u8 data)
{
	// WREN can be enabled/disabled
	if (data & WREN_FLAG)
		m_EECON1 |= WREN_FLAG;
	else
		m_EECON1 &= ~WREN_FLAG;

	// EEIF and WERR can be cleared
	m_EECON1 &= (data | u8(~(EEIF_FLAG | WRERR_FLAG)));

	// reading?
	if (data & EERD_FLAG)
		m_EEDATA = eeprom_read(m_EEADR);

	// writing?
	if (data & EEWR_FLAG)
	{
		if ((m_EECON1 & WREN_FLAG) && (m_eeprom_unlock_state == EEPROM_AA_WRITTEN))
		{
			eeprom_write(m_EEADR, m_EEDATA);
			set_eeif();
		}
	}

	m_eeprom_unlock_state = EEPROM_LOCKED;
}

u8 pic16x8x_device::eecon2_r()
{
	return 0; // not a physical register
}

void pic16x8x_device::eecon2_w(u8 data)
{
	if (m_eeprom_unlock_state == EEPROM_LOCKED && data == 0x55)
		m_eeprom_unlock_state = EEPROM_55_WRITTEN;
	else if (m_eeprom_unlock_state == EEPROM_55_WRITTEN && data == 0xaa)
		m_eeprom_unlock_state = EEPROM_AA_WRITTEN;
	else
		m_eeprom_unlock_state = EEPROM_LOCKED;
}

u8 pic16x8x_device::option_r()
{
	return m_OPTION;
}

void pic16x8x_device::option_w(u8 data)
{
	const u8 old = m_OPTION;
	m_OPTION = data;

	// changing prescaler assignment or rate resets the watchdog timer
	if ((old ^ data) & (PSA_FLAG | PS_REG))
	{
		m_prescaler = 0;
		restart_wdt();
	}
}

u8 pic16f628a_device::pir1_r()
{
	return m_PIR1;
}

void pic16f628a_device::pir1_w(u8 data)
{
	m_PIR1 = data & 0xf7; // bit 3 is unimplemented and reads as 0
}

u8 pic16f628a_device::tmr1l_r()
{
	return 0;
}

void pic16f628a_device::tmr1l_w(u8 data)
{
	logerror("tmr1l_w %02x\n", data);
}

u8 pic16f628a_device::tmr1h_r()
{
	return 0;
}

void pic16f628a_device::tmr1h_w(u8 data)
{
	logerror("tmr1h_w %02x\n", data);
}

u8 pic16f628a_device::t1con_r()
{
	return m_T1CON;
}

void pic16f628a_device::t1con_w(u8 data)
{
	m_T1CON = data;
}

u8 pic16f628a_device::tmr2_r()
{
	return 0;
}

void pic16f628a_device::tmr2_w(u8 data)
{
	logerror("tmr2_w %02x\n", data);
}

u8 pic16f628a_device::t2con_r()
{
	return m_T2CON;
}

void pic16f628a_device::t2con_w(u8 data)
{
	m_T2CON = data;
}

u8 pic16f628a_device::ccpr1l_r()
{
	return 0;
}

void pic16f628a_device::ccpr1l_w(u8 data)
{
	logerror("ccpr1l_w %02x\n", data);
}

u8 pic16f628a_device::ccpr1h_r()
{
	return 0;
}

void pic16f628a_device::ccpr1h_w(u8 data)
{
	logerror("ccpr1h_w %02x\n", data);
}

u8 pic16f628a_device::ccp1con_r()
{
	return m_CCP1CON;
}

void pic16f628a_device::ccp1con_w(u8 data)
{
	m_CCP1CON = data;
}

u8 pic16f628a_device::rcsta_r()
{
	return m_RCSTA;
}

void pic16f628a_device::rcsta_w(u8 data)
{
	m_RCSTA = data;
}

u8 pic16f628a_device::txreg_r()
{
	return 0;
}

void pic16f628a_device::txreg_w(u8 data)
{
	logerror("txreg_w %02x\n", data);
}

u8 pic16f628a_device::rcreg_r()
{
	return 0;
}

void pic16f628a_device::rcreg_w(u8 data)
{
	logerror("rcreg_w %02x\n", data);
}

u8 pic16f628a_device::cmcon_r()
{
	return m_CMCON;
}

void pic16f628a_device::cmcon_w(u8 data)
{
	m_CMCON = data;
}

u8 pic16f628a_device::pie1_r()
{
	return m_PIE1;
}

void pic16f628a_device::pie1_w(u8 data)
{
	m_PIE1 = data & 0xf7; // bit 3 is unimplemented and reads as 0;
}

u8 pic16f628a_device::pcon_r()
{
	return m_PCON;
}

void pic16f628a_device::pcon_w(u8 data)
{
	m_PCON = data;
}

u8 pic16f628a_device::pr2_r()
{
	return 0;
}

void pic16f628a_device::pr2_w(u8 data)
{
	logerror("pr2_w %02x\n", data);
}

u8 pic16f628a_device::txsta_r()
{
	return m_TXSTA;
}

void pic16f628a_device::txsta_w(u8 data)
{
	m_TXSTA = data;
}

u8 pic16f628a_device::spbrg_r()
{
	return 0;
}

void pic16f628a_device::spbrg_w(u8 data)
{
	logerror("spbrg_w %02x\n", data);
}

u8 pic16f628a_device::vrcon_r()
{
	return m_VRCON;
}

void pic16f628a_device::vrcon_w(u8 data)
{
	m_VRCON = data;
}


//**************************************************************************
//  HELPER FUNCTIONS
//**************************************************************************

offs_t pic16x8x_device::addr() const
{
	return ((m_STATUS & (RP1_FLAG | RP0_FLAG)) << 2) | (m_opcode.b.l & 0x7f);
}

void pic16x8x_device::calc_zero_flag()
{
	if (m_ALU == 0)
		m_STATUS |= Z_FLAG;
	else
		m_STATUS &= ~Z_FLAG;
}

void pic16x8x_device::calc_add_flags(u8 augend)
{
	calc_zero_flag();

	if (augend > m_ALU)
		m_STATUS |= C_FLAG;
	else
		m_STATUS &= ~C_FLAG;

	if ((augend & 0x0f) > (m_ALU & 0x0f))
		m_STATUS |= DC_FLAG;
	else
		m_STATUS &= ~DC_FLAG;
}

void pic16x8x_device::calc_sub_flags(u8 minuend)
{
	calc_zero_flag();

	if (minuend < m_ALU)
		m_STATUS &= ~C_FLAG;
	else
		m_STATUS |= C_FLAG;

	if ((minuend & 0x0f) < (m_ALU & 0x0f))
		m_STATUS &= ~DC_FLAG;
	else
		m_STATUS |= DC_FLAG;
}

void pic16x8x_device::set_pc(u16 addr)
{
	m_PC = addr & PC_MASK;
}

u16 pic16x8x_device::pop_stack()
{
	m_stack_pointer = (m_stack_pointer - 1) & 0x07;
	return m_STACK[m_stack_pointer] & PC_MASK;
}

void pic16x8x_device::push_stack(u16 data)
{
	m_STACK[m_stack_pointer] = data & PC_MASK;
	m_stack_pointer = (m_stack_pointer + 1) & 0x07;
}

void pic16x8x_device::store_result(offs_t offset, u8 data)
{
	if (m_opcode.b.l & 0x80)
		store_regfile(offset, data);
	else
		m_W = data;
}


//**************************************************************************
//  INSTRUCTIONS
//**************************************************************************

void pic16x8x_device::op_illegal()
{
	logerror("PIC16x8x: PC=%03x, Illegal opcode = %04x\n", m_PREVPC, m_opcode.w);
}

void pic16x8x_device::op_addlw()
{
	u8 k = m_opcode.b.l;
	m_ALU = k + m_W;
	m_W = m_ALU;
	calc_add_flags(k);
}

void pic16x8x_device::op_addwf()
{
	u8 augend = get_regfile(addr());
	m_ALU = augend + m_W;
	store_result(addr(), m_ALU);
	calc_add_flags(augend);
}

void pic16x8x_device::op_andlw()
{
	m_ALU = m_opcode.b.l & m_W;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16x8x_device::op_andwf()
{
	m_ALU = get_regfile(addr()) & m_W;
	store_result(addr(), m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::op_bcf()
{
	m_ALU = get_regfile(addr());
	m_ALU &= ~(1 << bit_pos());
	store_regfile(addr(), m_ALU);
}

void pic16x8x_device::op_bsf()
{
	m_ALU = get_regfile(addr());
	m_ALU |= 1 << bit_pos();
	store_regfile(addr(), m_ALU);
}

void pic16x8x_device::op_btfss()
{
	if (BIT(get_regfile(addr()), bit_pos()))
	{
		set_pc(m_PC + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16x8x_device::op_btfsc()
{
	if (!BIT(get_regfile(addr()), bit_pos()))
	{
		set_pc(m_PC + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16x8x_device::op_call()
{
	push_stack(m_PC);
	set_pc(((m_PCLATH & 0x18) << 8 ) | (m_opcode.w & 0x07ff));
}


void pic16x8x_device::op_clrw()
{
	m_W = 0;
	m_STATUS |= Z_FLAG;
}

void pic16x8x_device::op_clrf()
{
	store_regfile(addr(), 0);
	m_STATUS |= Z_FLAG;
}

void pic16x8x_device::op_clrwdt()
{
	m_STATUS |= (TO_FLAG | PD_FLAG);
	restart_wdt();
}

void pic16x8x_device::op_comf()
{
	m_ALU = u8(~(get_regfile(addr())));
	store_result(addr(), m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::op_decf()
{
	m_ALU = get_regfile(addr()) - 1;
	store_result(addr(), m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::op_decfsz()
{
	m_ALU = get_regfile(addr()) - 1;
	store_result(addr(), m_ALU);
	if (m_ALU == 0)
	{
		set_pc(m_PC + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16x8x_device::op_goto()
{
	set_pc(((m_PCLATH & 0x18) << 8 ) | (m_opcode.w & 0x07ff));
}

void pic16x8x_device::op_incf()
{
	m_ALU = get_regfile(addr()) + 1;
	store_result(addr(), m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::op_incfsz()
{
	m_ALU = get_regfile(addr()) + 1;
	store_result(addr(), m_ALU);
	if (m_ALU == 0)
	{
		set_pc(m_PC + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16x8x_device::op_iorlw()
{
	m_ALU = m_opcode.b.l | m_W;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16x8x_device::op_iorwf()
{
	m_ALU = get_regfile(addr()) | m_W;
	store_result(addr(), m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::op_movf()
{
	m_ALU = get_regfile(addr());
	store_result(addr(), m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::op_movlw()
{
	m_W = m_opcode.b.l;
}

void pic16x8x_device::op_movwf()
{
	store_regfile(addr(), m_W);
}

void pic16x8x_device::op_nop()
{
	// Do nothing
}

void pic16x8x_device::op_option()
{
	option_w(m_W);
}

void pic16x8x_device::op_retfie()
{
	set_pc(pop_stack());
	m_INTCON |= GIE_FLAG;
}

void pic16x8x_device::op_retlw()
{
	m_W = m_opcode.b.l;
	set_pc(pop_stack());
}

void pic16x8x_device::op_return()
{
	set_pc(pop_stack());
}

void pic16x8x_device::op_rlf()
{
	m_ALU = get_regfile(addr());
	int carry = BIT(m_ALU, 7);
	m_ALU <<= 1;
	if (m_STATUS & C_FLAG) m_ALU |= 1;
	store_result(addr(), m_ALU);

	if (carry)
		m_STATUS |= C_FLAG;
	else
		m_STATUS &= ~C_FLAG;
}

void pic16x8x_device::op_rrf()
{
	m_ALU = get_regfile(addr());
	int carry = BIT(m_ALU, 0);
	m_ALU >>= 1;
	if (m_STATUS & C_FLAG) m_ALU |= 0x80;
	store_result(addr(), m_ALU);

	if (carry)
		m_STATUS |= C_FLAG;
	else
		m_STATUS &= ~C_FLAG;
}

void pic16x8x_device::op_sleep()
{
	m_STATUS |= TO_FLAG;
	m_STATUS &= ~PD_FLAG;
	m_sleeping = true;
	restart_wdt();
}

void pic16x8x_device::op_sublw()
{
	u8 minuend = m_opcode.b.l;
	m_ALU = minuend - m_W;
	m_W = m_ALU;
	calc_sub_flags(minuend);
}

void pic16x8x_device::op_subwf()
{
	u8 minuend = get_regfile(addr());
	m_ALU = minuend - m_W;
	store_result(addr(), m_ALU);
	calc_sub_flags(minuend);
}

void pic16x8x_device::op_swapf()
{
	u8 reg = get_regfile(addr());
	m_ALU = reg << 4 | reg >> 4;
	store_result(addr(), m_ALU);
}

void pic16x8x_device::op_tris()
{
	switch (m_opcode.b.l & 0x07)
	{
		case 5: trisa_w(m_W); return;
		case 6: trisb_w(m_W); return;
	}

	op_illegal();
}

void pic16x8x_device::op_xorlw()
{
	m_ALU = m_W ^ m_opcode.b.l;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16x8x_device::op_xorwf()
{
	m_ALU = get_regfile(addr()) ^ m_W;
	store_result(addr(), m_ALU);
	calc_zero_flag();
}


//**************************************************************************
//  INSTRUCTION DECODE TABLE
//**************************************************************************

#define OP(name, cycles, status) { &pic16x8x_device::op_##name, cycles, status }

const pic16x8x_device::pic16x8x_opcode pic16x8x_device::s_opcode_main[128]=
{
	OP(nop,   1, false), OP(movwf, 1, false), OP(clrw,   1, true ), OP(clrf,   1, true ), // 00
	OP(subwf, 1, true ), OP(subwf, 1, true ), OP(decf,   1, true ), OP(decf,   1, true ), // 04
	OP(iorwf, 1, true ), OP(iorwf, 1, true ), OP(andwf,  1, true ), OP(andwf,  1, true ), // 08
	OP(xorwf, 1, true ), OP(xorwf, 1, true ), OP(addwf,  1, true ), OP(addwf,  1, true ), // 0c
	OP(movf,  1, true ), OP(movf,  1, true ), OP(comf,   1, true ), OP(comf,   1, true ), // 10
	OP(incf,  1, true ), OP(incf,  1, true ), OP(decfsz, 1, false), OP(decfsz, 1, false), // 14
	OP(rrf,   1, true ), OP(rrf,   1, true ), OP(rlf,    1, true ), OP(rlf,    1, true ), // 18
	OP(swapf, 1, false), OP(swapf, 1, false), OP(incfsz, 1, false), OP(incfsz, 1, false), // 1c
	OP(bcf,   1, false), OP(bcf,   1, false), OP(bcf,    1, false), OP(bcf,    1, false), // 20
	OP(bcf,   1, false), OP(bcf,   1, false), OP(bcf,    1, false), OP(bcf,    1, false), // 24
	OP(bsf,   1, false), OP(bsf,   1, false), OP(bsf,    1, false), OP(bsf,    1, false), // 28
	OP(bsf,   1, false), OP(bsf,   1, false), OP(bsf,    1, false), OP(bsf,    1, false), // 2c
	OP(btfsc, 1, false), OP(btfsc, 1, false), OP(btfsc,  1, false), OP(btfsc,  1, false), // 30
	OP(btfsc, 1, false), OP(btfsc, 1, false), OP(btfsc,  1, false), OP(btfsc,  1, false), // 34
	OP(btfss, 1, false), OP(btfss, 1, false), OP(btfss,  1, false), OP(btfss,  1, false), // 38
	OP(btfss, 1, false), OP(btfss, 1, false), OP(btfss,  1, false), OP(btfss,  1, false), // 3c
	OP(call,  2, false), OP(call,  2, false), OP(call,   2, false), OP(call,   2, false), // 40
	OP(call,  2, false), OP(call,  2, false), OP(call,   2, false), OP(call,   2, false), // 44
	OP(call,  2, false), OP(call,  2, false), OP(call,   2, false), OP(call,   2, false), // 48
	OP(call,  2, false), OP(call,  2, false), OP(call,   2, false), OP(call,   2, false), // 4c
	OP(goto,  2, false), OP(goto,  2, false), OP(goto,   2, false), OP(goto,   2, false), // 50
	OP(goto,  2, false), OP(goto,  2, false), OP(goto,   2, false), OP(goto,   2, false), // 54
	OP(goto,  2, false), OP(goto,  2, false), OP(goto,   2, false), OP(goto,   2, false), // 58
	OP(goto,  2, false), OP(goto,  2, false), OP(goto,   2, false), OP(goto,   2, false), // 5c
	OP(movlw, 1, false), OP(movlw, 1, false), OP(movlw,  1, false), OP(movlw,  1, false), // 60
	OP(movlw, 1, false), OP(movlw, 1, false), OP(movlw,  1, false), OP(movlw,  1, false), // 64
	OP(retlw, 2, false), OP(retlw, 2, false), OP(retlw,  2, false), OP(retlw,  2, false), // 68
	OP(retlw, 2, false), OP(retlw, 2, false), OP(retlw,  2, false), OP(retlw,  2, false), // 6c
	OP(iorlw, 1, true ), OP(iorlw, 1, true ), OP(andlw,  1, true ), OP(andlw,  1, true ), // 70
	OP(xorlw, 1, true ), OP(xorlw, 1, true ), OP(xorlw,  1, true ), OP(xorlw,  1, true ), // 74
	OP(sublw, 1, true ), OP(sublw, 1, true ), OP(sublw,  1, true ), OP(sublw,  1, true ), // 78
	OP(addlw, 1, true ), OP(addlw, 1, true ), OP(addlw,  1, true ), OP(addlw,  1, true )  // 7c
};

const pic16x8x_device::pic16x8x_opcode pic16x8x_device::s_opcode_00x[128]=
{
	OP(nop,     1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 00
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 04
	OP(return,  2, false), OP(retfie,  2, false), OP(illegal, 1, false), OP(illegal, 1, false), // 08
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 0c
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 10
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 14
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 18
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 1c
	OP(nop,     1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 20
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 24
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 28
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 2c
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 30
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 34
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 38
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 3c
	OP(nop,     1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 40
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 44
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 48
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 4c
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 50
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 54
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 58
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 5c
	OP(nop,     1, false), OP(illegal, 1, false), OP(option,  1, false), OP(sleep,   1, false), // 60
	OP(clrwdt,  1, false), OP(tris,    1, false), OP(tris,    1, false), OP(tris,    1, false), // 64
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 68
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 6c
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 70
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 74
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), // 78
	OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false), OP(illegal, 1, false)  // 7c
};
