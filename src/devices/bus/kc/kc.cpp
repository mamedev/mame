// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    kc.c

    KC85_2/3/4/5 expansion slot emulation

**********************************************************************

        GND          1      30          GND
        GND          2      31          GND
        NC           3      32          NC
        DB7          4      33          DB6
        DB5          5      34          DB4
        DB3          6      35          DB2
        DB1          7      36          DB0
        /WR          8      37          /RD
        /WREQ        9      38          /IORQ
        IEO         10      39          NC
        AB14        11      40          AB15
        AB12        12      41          AB13
        AB10        13      42          AB11
        AB8         14      43          AB9
        /WAIT       15      44          /RFSH
        AB6         16      45          AB7
        AB4         17      46          AB5
        AB2         18      47          AB3
        AB0         19      48          AB1
        /RESET      20      49          /BUSRQ
        TATK        21      50          GND
        /M1         22      51          /HALT
        /NMI        23      52          /INT
        MEO         24      53          MEI
        /ZI         25      54          /BI
        /NC         26      55          /HELL
        /BUSAK      27      56          NC
        VCC         28      57          NC
        NC          29      58          NC

    Slots are organized into a chain (MEI -> MEO) , the first module that
    decode the address on bus, clear the MEO line for disable other modules
    with less priority.

************************************************************************

    Known KC85 modules

    Name    ID     Control     Emulated   Description

    M001    EF    ---- ---M       no      Digital IN/OUT
    M002    DA    ---- ---M       no      PIO 3
    M003    EE    ---- ---M       no      V.24
    M005          ---- ----       no      User
    M006    FC    AA-- ---M       yes     BASIC for KC85/2 (16KB ROM)
    M007          ---- ----       no      Adapter
    M008          ---- ----       no      Joystick
    M009    ED    ---- ---M       no      TLCM Spracheingabe/Datenkompr
    M010    E7    ---- ---M       no      ADU 1 (4 analog inputs)
    M011    F6    AA-- --WM       yes     64KB RAM
    M012    FB    AAA- ---M       yes     Texor (8KB ROM)
    M021          ---- ----       no      Joystick + Centronics
    M022    F4    AA-- --WM       yes     16KB RAM
    M024    F5    AA-- --WM       no      32KB RAM
    M025    F7    AAA- ---M       no      User PROM 8KB
    M026    FB    AAA- ---M       yes     FORTH (8KB ROM)
    M027    FB    AAA- ---M       yes     Development (8KB ROM)
    M028    FB    AA-- ---M       no      16KB EPROM(2x U2764)
    M029    E3    ---- ---M       no      DAU 1 (2 analog outputs)
    M030    DB    AAA- ---M       no      EPROMER 8KB EPROM
    M032    79    A-SS SSWM       yes     256KB segmented RAM (16 segments of 16KB)
    M033    01    AA0S ---M       yes     Typestar (8KB x 2 ROM)
    M034    7A    ASSS SSWM       yes     512KB segmented RAM (32 segments of 16KB)
    M035    7B    SSSS SSWM       yes     1MB segmented RAM (64 segments of 16KB)
    M036    78    A--S SSWM       yes     128KB segmented RAM (8 segments of 16KB)
    M040    F8    AA-- ---M       no      User PROM 8/16KB
    M045    70    AASS ---M       no      User 32KB segmented ROM (4 segments of 8KB)
    M046    71    AASS -S-M       no      User 64KB segmented ROM (8 segments of 8KB)
    M047    72    AASS SS-M       no      User 128KB segmented ROM (16 segments of 8KB)
    M048    73    SSSS SS-M       no      User 256KB segmented ROM (16 segments of 16KB)
    M051    EC    ---- ----       no      Scanner Module
    M052    FD    ---- ----       no      USB + NET (TCP/IP)
    M053    EE    ---- ---M       no      RS-232
    M061          ---- ----       no      3x E/A-Modul
    M120    F0    AAA- --WM       no      8KB CMOS-RAM
    M122    F1    AA-- --WM       no      16KB CMOS-RAM
    M124    F2    AA-- --WM       no      32KB CMOS-RAM

    D001          ---- ----       yes     Basis Device
    D002          ---- ----       yes     Bus driver expansion
    D003          ---- ----       no      PROM programmer for KC-PROM Module
    D004    A7    --A- -K-M       yes     Floppy Disk Interface
    D005          ---- ----       no      Komfort-Tastatur for KC85/4


    Control byte                  ID byte
    A - Base Address              01    - Autostart modules
    K - Power on/off              7x    - Segmented memory
    S - Active segment            Dx/Ex - IN/OUT modules
    W - Write enabled             Fx    - memory modules
    M - Module enabled

    Info taken from: http://www.mpm-kc85.de/html/ModulListe.htm

*********************************************************************/

#include "emu.h"
#include "kc.h"

#define  LOG    0


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type KCEXP_SLOT  = &device_creator<kcexp_slot_device>;
const device_type KCCART_SLOT = &device_creator<kccart_slot_device>;


//**************************************************************************
//    KC85 Expansion Interface
//**************************************************************************

//-------------------------------------------------
//  device_kcexp_interface - constructor
//-------------------------------------------------

device_kcexp_interface::device_kcexp_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_kcexp_interface - destructor
//-------------------------------------------------

device_kcexp_interface::~device_kcexp_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kcexp_slot_device - constructor
//-------------------------------------------------
kcexp_slot_device::kcexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, KCEXP_SLOT, "KC85 Expansion Slot", tag, owner, clock, "kcexp_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		m_out_irq_cb(*this),
		m_out_nmi_cb(*this),
		m_out_halt_cb(*this), m_cart(nullptr), m_next_slot_tag(nullptr), m_next_slot(nullptr)
{
}

kcexp_slot_device::kcexp_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_interface(mconfig, *this),
		m_out_irq_cb(*this),
		m_out_nmi_cb(*this),
		m_out_halt_cb(*this), m_cart(nullptr), m_next_slot_tag(nullptr), m_next_slot(nullptr)
{
}

void kcexp_slot_device::static_set_next_slot(device_t &device, const char *next_slot_tag)
{
	kcexp_slot_device &kc_slot = dynamic_cast<kcexp_slot_device &>(device);

	kc_slot.m_next_slot_tag = next_slot_tag;
}

//-------------------------------------------------
//  kcexp_slot_device - destructor
//-------------------------------------------------

kcexp_slot_device::~kcexp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kcexp_slot_device::device_start()
{
	m_cart = dynamic_cast<device_kcexp_interface *>(get_card_device());
	m_next_slot = m_next_slot_tag ? owner()->subdevice<kcexp_slot_device>(m_next_slot_tag) : nullptr;

	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
	m_out_halt_cb.resolve_safe();
}


/*-------------------------------------------------
    module id read
-------------------------------------------------*/

UINT8 kcexp_slot_device::module_id_r()
{
	UINT8 result = 0xff;

	if (m_cart)
		result = m_cart->module_id_r();

	return result;
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/
void kcexp_slot_device::control_w(UINT8 data)
{
	if (m_cart)
		m_cart->control_w(data);
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kcexp_slot_device::read(offs_t offset, UINT8 &data)
{
	if (m_cart)
		m_cart->read(offset, data);
}


/*-------------------------------------------------
    write
-------------------------------------------------*/

void kcexp_slot_device::write(offs_t offset, UINT8 data)
{
	if (m_cart)
		m_cart->write(offset, data);
}

/*-------------------------------------------------
    IO read
-------------------------------------------------*/

void kcexp_slot_device::io_read(offs_t offset, UINT8 &data)
{
	if (m_cart)
		m_cart->io_read(offset, data);
}


/*-------------------------------------------------
   IO write
-------------------------------------------------*/

void kcexp_slot_device::io_write(offs_t offset, UINT8 data)
{
	if (m_cart)
		m_cart->io_write(offset, data);
}

/*-------------------------------------------------
   MEI line write
-------------------------------------------------*/

WRITE_LINE_MEMBER( kcexp_slot_device::mei_w )
{
	if (LOG) logerror("KCEXP '%s': %s MEI line\n", tag(), state != CLEAR_LINE ? "ASSERT": "CLEAR");

	if (m_cart)
		m_cart->mei_w(state);
}

/*-------------------------------------------------
   MEO line write
-------------------------------------------------*/

WRITE_LINE_MEMBER( kcexp_slot_device::meo_w )
{
	if (LOG) logerror("KCEXP '%s': %s MEO line\n", tag(), state != CLEAR_LINE ? "ASSERT": "CLEAR");

	if (m_next_slot)
		m_next_slot->mei_w(state);
}


//**************************************************************************
//     KC85 Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  kccart_slot_device - constructor
//-------------------------------------------------
kccart_slot_device::kccart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		kcexp_slot_device(mconfig, KCCART_SLOT, "KC85 Cartridge Slot", tag, owner, clock, "kccart_slot", __FILE__),
		device_image_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  kccart_slot_device - destructor
//-------------------------------------------------

kccart_slot_device::~kccart_slot_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void kccart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}

/*-------------------------------------------------
    call load
-------------------------------------------------*/

bool kccart_slot_device::call_load()
{
	if (m_cart)
	{
		offs_t read_length;
		UINT8 *cart_base = m_cart->get_cart_base();

		if (cart_base != nullptr)
		{
			if (software_entry() == nullptr)
			{
				read_length = length();
				fread(m_cart->get_cart_base(), read_length);
			}
			else
			{
				read_length = get_software_region_length("rom");
				memcpy(m_cart->get_cart_base(), get_software_region("rom"), read_length);
			}
		}
		else
			return IMAGE_INIT_FAIL;
	}

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
    call softlist load
-------------------------------------------------*/

bool kccart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	machine().rom_load().load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}

/*-------------------------------------------------
    get default card software
-------------------------------------------------*/

std::string kccart_slot_device::get_default_card_software()
{
	return software_get_default_slot("standard");
}
