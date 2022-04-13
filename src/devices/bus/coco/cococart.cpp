// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cococart.cpp

    CoCo/Dragon cartridge slot - typically used for "Program Paks"
    (which are simple ROMs) but in practice is the main extensibility
    mechanism for CoCo hardware

    CoCo and Dragon pinout listing
       ---  -------            ---  -------
         1  -12V                21  A2
         2  +12V                22  A3
         3  HALT                23  A4
         4  NMI                 24  A5
         5  RESET               25  A6
         6  EIN                 26  A7
         7  QIN                 27  A8
         8  CART                28  A9
         9  +5V                 29  A10
        10  D0                  30  A11
        11  D1                  31  A12
        12  D2                  32  CTS
        13  D3                  33  GND
        14  D4                  34  GND
        15  D5                  35  SND
        16  D6                  36  SCS
        17  D7                  37  A13
        18  R/!W                38  A14
        19  A0                  39  A15
        20  A1                  40  SLENB

    Notes:
        CTS - ROM read $C000-$FEFF ($FDFF on CoCo 3)
        SCS - Spare Chip Select:  IO space between $FF40-5F

*********************************************************************/

#include "emu.h"
#include "cococart.h"
#include "formats/rpk.h"

#include "coco_dcmodem.h"
#include "coco_fdc.h"
#include "coco_gmc.h"
#include "coco_ide.h"
#include "coco_max.h"
#include "coco_midi.h"
#include "coco_multi.h"
#include "coco_orch90.h"
#include "coco_pak.h"
#include "coco_psg.h"
#include "coco_ram.h"
#include "coco_rs232.h"
#include "coco_ssc.h"
#include "coco_stecomp.h"
#include "coco_sym12.h"
#include "coco_wpk.h"
#include "coco_wpk2p.h"

#include "dragon_amtor.h"
#include "dragon_claw.h"
#include "dragon_fdc.h"
#include "dragon_jcbsnd.h"
#include "dragon_jcbspch.h"
#include "dragon_msx2.h"
#include "dragon_serial.h"
#include "dragon_sprites.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//#define LOG_GENERAL   (1U << 0) //defined in logmacro.h already
#define LOG_CART (1U << 1) // shows cart line changes
#define LOG_NMI  (1U << 2) // shows switch changes
#define LOG_HALT (1U << 3) // shows switch changes
// #define VERBOSE (LOG_CART)

#include "logmacro.h"

#define LOGCART(...) LOGMASKED(LOG_CART,  __VA_ARGS__)
#define LOGNMI(...)  LOGMASKED(LOG_NMI,  __VA_ARGS__)
#define LOGHALT(...) LOGMASKED(LOG_HALT,  __VA_ARGS__)


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	TIMER_CART,
	TIMER_NMI,
	TIMER_HALT
};


// definitions of RPK PCBs in layout.xml
static const char *coco_rpk_pcbdefs[] =
{
	"standard",
	"paged16k",
	nullptr
};


// ...and their mappings to "default card slots"
static const char *coco_rpk_cardslottypes[] =
{
	"pak",
	"banked_16k"
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCOCART_SLOT, cococart_slot_device, "cococart_slot", "CoCo Cartridge Slot")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

ALLOW_SAVE_TYPE(cococart_slot_device::line_value);

//-------------------------------------------------
//  cococart_slot_device - constructor
//-------------------------------------------------
cococart_slot_device::cococart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, COCOCART_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_cococart_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_cart_callback(*this),
	m_nmi_callback(*this),
	m_halt_callback(*this), m_cart(nullptr)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cococart_slot_device::device_start()
{
	for(int i=0; i<TIMER_POOL; i++ )
	{
		m_cart_line.timer[i]    = timer_alloc(TIMER_CART);
		m_nmi_line.timer[i]     = timer_alloc(TIMER_NMI);
		m_halt_line.timer[i]    = timer_alloc(TIMER_HALT);
	}

	m_cart_line.timer_index     = 0;
	m_cart_line.delay           = 0;
	m_cart_line.value           = line_value::CLEAR;
	m_cart_line.line            = 0;
	m_cart_line.q_count         = 0;
	m_cart_callback.resolve();
	m_cart_line.callback = &m_cart_callback;

	m_nmi_line.timer_index      = 0;
	m_nmi_line.delay            = 0;
	m_nmi_line.value            = line_value::CLEAR;
	m_nmi_line.line             = 0;
	m_nmi_line.q_count          = 0;
	m_nmi_callback.resolve();
	m_nmi_line.callback = &m_nmi_callback;

	m_halt_line.timer_index     = 0;
	m_halt_line.delay           = 0;
	m_halt_line.value           = line_value::CLEAR;
	m_halt_line.line            = 0;
	m_halt_line.q_count         = 0;
	m_halt_callback.resolve();
	m_halt_line.callback = &m_halt_callback;

	m_cart = get_card_device();

	save_item(STRUCT_MEMBER(m_cart_line, timer_index));
	save_item(STRUCT_MEMBER(m_cart_line, delay));
	save_item(STRUCT_MEMBER(m_cart_line, value));
	save_item(STRUCT_MEMBER(m_cart_line, line));
	save_item(STRUCT_MEMBER(m_cart_line, q_count));

	save_item(STRUCT_MEMBER(m_nmi_line, timer_index));
	save_item(STRUCT_MEMBER(m_nmi_line, delay));
	save_item(STRUCT_MEMBER(m_nmi_line, value));
	save_item(STRUCT_MEMBER(m_nmi_line, line));
	save_item(STRUCT_MEMBER(m_nmi_line, q_count));

	save_item(STRUCT_MEMBER(m_halt_line, timer_index));
	save_item(STRUCT_MEMBER(m_halt_line, delay));
	save_item(STRUCT_MEMBER(m_halt_line, value));
	save_item(STRUCT_MEMBER(m_halt_line, line));
	save_item(STRUCT_MEMBER(m_halt_line, q_count));
}



//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void cococart_slot_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch(id)
	{
		case TIMER_CART:
			set_line(line::CART, m_cart_line, (line_value) param);
			break;

		case TIMER_NMI:
			set_line(line::NMI, m_nmi_line, (line_value) param);
			break;

		case TIMER_HALT:
			set_line(line::HALT, m_halt_line, (line_value) param);
			break;
	}
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 cococart_slot_device::cts_read(offs_t offset)
{
	u8 result = 0x00;
	if (m_cart)
		result = m_cart->cts_read(offset);
	return result;
}


//-------------------------------------------------
//  cts_write
//-------------------------------------------------

void cococart_slot_device::cts_write(offs_t offset, u8 data)
{
	if (m_cart)
		m_cart->cts_write(offset, data);
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 cococart_slot_device::scs_read(offs_t offset)
{
	u8 result = 0x00;
	if (m_cart)
		result = m_cart->scs_read(offset);
	return result;
}


//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void cococart_slot_device::scs_write(offs_t offset, u8 data)
{
	if (m_cart)
		m_cart->scs_write(offset, data);
}



//-------------------------------------------------
//  line_value_string
//-------------------------------------------------

const char *cococart_slot_device::line_value_string(line_value value)
{
	const char *s;
	switch(value)
	{
		case line_value::CLEAR:
			s = "CLEAR";
			break;
		case line_value::ASSERT:
			s = "ASSERT";
			break;
		case line_value::Q:
			s = "Q";
			break;
		default:
			throw false && "Invalid value";
	}
	return s;
}


//-------------------------------------------------
//  set_line
//-------------------------------------------------

void cococart_slot_device::set_line(line ln, coco_cartridge_line &line, cococart_slot_device::line_value value)
{
	if ((line.value != value) || (value == line_value::Q))
	{
		line.value = value;

		switch (ln)
		{
		case line::CART:
			LOGCART( "set_line: CART, value: %s\n", line_value_string(value));
			break;
		case line::NMI:
			LOGNMI( "set_line: NMI, value: %s\n", line_value_string(value));
			break;
		case line::HALT:
			LOGHALT( "set_line: HALT, value: %s\n", line_value_string(value));
			break;
		case line::SOUND_ENABLE:
			break;
		}

		// engage in a bit of gymnastics for this odious 'Q' value
		switch(line.value)
		{
			case line_value::CLEAR:
				line.line = 0x00;
				line.q_count = 0;
				break;

			case line_value::ASSERT:
				line.line = 0x01;
				line.q_count = 0;
				break;

			case line_value::Q:
				line.line = line.line ? 0x00 : 0x01;
				if (line.q_count++ < 4)
					set_line_timer(line, value);
				break;
		}

		/* invoke the callback, if present */
		if (!(*line.callback).isnull())
			(*line.callback)(line.line);
	}
}


//-------------------------------------------------
//  set_line_timer
//-------------------------------------------------

void cococart_slot_device::set_line_timer(coco_cartridge_line &line, cococart_slot_device::line_value value)
{
	// calculate delay; delay dependant on cycles per second
	attotime delay = (line.delay != 0)
			? clocks_to_attotime(line.delay)
			: attotime::zero;

	line.timer[line.timer_index]->adjust(delay, (int) value);
	line.timer_index = (line.timer_index + 1) % TIMER_POOL;
}


//-------------------------------------------------
//  twiddle_line_if_q
//-------------------------------------------------

void cococart_slot_device::twiddle_line_if_q(coco_cartridge_line &line)
{
	if (line.value == line_value::Q)
	{
		line.q_count = 0;
		set_line_timer(line, line_value::Q);
	}
}


//-------------------------------------------------
//  twiddle_q_lines - hack to support twiddling the
//  Q line
//-------------------------------------------------

void cococart_slot_device::twiddle_q_lines()
{
	twiddle_line_if_q(m_cart_line);
	twiddle_line_if_q(m_nmi_line);
	twiddle_line_if_q(m_halt_line);
}


//-------------------------------------------------
//  set_line_value
//-------------------------------------------------

void cococart_slot_device::set_line_value(cococart_slot_device::line which, cococart_slot_device::line_value value)
{
	switch (which)
	{
	case cococart_slot_device::line::CART:
		set_line_timer(m_cart_line, value);
		break;

	case cococart_slot_device::line::NMI:
		set_line_timer(m_nmi_line, value);
		break;

	case cococart_slot_device::line::HALT:
		set_line_timer(m_halt_line, value);
		break;

	case cococart_slot_device::line::SOUND_ENABLE:
		if (m_cart)
			m_cart->set_sound_enable(value != cococart_slot_device::line_value::CLEAR);
		break;
	}
}


//-------------------------------------------------
//  set_line_delay
//-------------------------------------------------

void cococart_slot_device::set_line_delay(cococart_slot_device::line which, int cycles)
{
	switch (which)
	{
	case cococart_slot_device::line::CART:
		m_cart_line.delay = cycles;
		break;

	case cococart_slot_device::line::NMI:
		m_nmi_line.delay = cycles;
		break;

	case cococart_slot_device::line::HALT:
		m_halt_line.delay = cycles;
		break;

	default:
		throw false;
	}
}


//-------------------------------------------------
//  get_line_value
//-------------------------------------------------

cococart_slot_device::line_value cococart_slot_device::get_line_value(cococart_slot_device::line which) const
{
	line_value result;
	switch (which)
	{
	case cococart_slot_device::line::CART:
		result = m_cart_line.value;
		break;

	case cococart_slot_device::line::NMI:
		result = m_nmi_line.value;
		break;

	case cococart_slot_device::line::HALT:
		result = m_halt_line.value;
		break;

	default:
		result = line_value::CLEAR;
		break;
	}
	return result;
}


//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

u8 *cococart_slot_device::get_cart_base()
{
	if (m_cart != nullptr)
		return m_cart->get_cart_base();
	return nullptr;
}


//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

u32 cococart_slot_device::get_cart_size()
{
	if (m_cart != nullptr)
		return m_cart->get_cart_size();

	return 0x8000;
}

//-------------------------------------------------
//  set_cart_base_update
//-------------------------------------------------

void cococart_slot_device::set_cart_base_update(cococart_base_update_delegate update)
{
	if (m_cart)
		m_cart->set_cart_base_update(update);
}


//-------------------------------------------------
//  read_coco_rpk
//-------------------------------------------------

static std::error_condition read_coco_rpk(std::unique_ptr<util::random_read> &&stream, rpk_file::ptr &result)
{
	// sanity checks
	static_assert(std::size(coco_rpk_pcbdefs) - 1 == std::size(coco_rpk_cardslottypes));

	// set up the RPK reader
	rpk_reader reader(coco_rpk_pcbdefs, false);

	// and read the RPK file
	return reader.read(std::move(stream), result);
}


//-------------------------------------------------
//  read_coco_rpk
//-------------------------------------------------

static std::error_condition read_coco_rpk(std::unique_ptr<util::random_read> &&stream, u8 *mem, offs_t cart_length, offs_t &actual_length)
{
	actual_length = 0;

	// open the RPK
	rpk_file::ptr file;
	std::error_condition err = read_coco_rpk(std::move(stream), file);
	if (err)
		return err;

	// for now, we are just going to load all sockets into the contiguous block of memory
	// that cartridges use
	offs_t pos = 0;
	for (const rpk_socket &socket : file->sockets())
	{
		// only ROM supported for now; if we see anything else it should have been caught in the RPK code
		assert(socket.type() == rpk_socket::socket_type::ROM);

		// read all bytes
		std::vector<uint8_t> contents;
		err = socket.read_file(contents);
		if (err)
			return err;

		// copy the bytes
		offs_t size = (offs_t) std::min(contents.size(), (size_t)cart_length - pos);
		memcpy(&mem[pos], &contents[0], size);
		pos += size;
	}

	// we're done!
	actual_length = pos;
	return std::error_condition();
}


//-------------------------------------------------
//  call_load
//-------------------------------------------------

image_init_result cococart_slot_device::call_load()
{
	if (m_cart)
	{
		memory_region *cart_mem = m_cart->get_cart_memregion();
		u8 *base = cart_mem->base();
		offs_t read_length, cart_length = cart_mem->bytes();

		if (loaded_through_softlist())
		{
			// loaded through softlist
			read_length = get_software_region_length("rom");
			memcpy(base, get_software_region("rom"), read_length);
		}
		else if (is_filetype("rpk"))
		{
			// RPK file
			util::core_file::ptr proxy;
			std::error_condition err = util::core_file::open_proxy(image_core_file(), proxy);
			if (!err)
				err = read_coco_rpk(std::move(proxy), base, cart_length, read_length);
			if (err)
				return image_init_result::FAIL;
		}
		else
		{
			// conventional ROM image
			read_length = fread(base, cart_length);
		}

		while (read_length < cart_length)
		{
			offs_t len = std::min(read_length, cart_length - read_length);
			memcpy(base + read_length, base, len);
			read_length += len;
		}
	}
	return image_init_result::PASS;
}


//-------------------------------------------------
//  get_default_card_software
//-------------------------------------------------

std::string cococart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	// this is the default for anything not in an RPK file
	int pcb_type = 0;

	// is this an RPK?
	if (hook.is_filetype("rpk"))
	{
		// RPK file
		rpk_file::ptr file;
		util::core_file::ptr proxy;
		std::error_condition err = util::core_file::open_proxy(*hook.image_file(), proxy);
		if (!err)
			err = read_coco_rpk(std::move(proxy), file);
		if (!err)
			pcb_type = file->pcb_type();
	}

	// lookup the default slot
	return software_get_default_slot(coco_rpk_cardslottypes[pcb_type]);
}


//**************************************************************************
//  DEVICE COCO CART INTERFACE - Implemented by devices that plug into
//  CoCo cartridge slots
//**************************************************************************

template class device_finder<device_cococart_interface, false>;
template class device_finder<device_cococart_interface, true>;

//-------------------------------------------------
//  device_cococart_interface - constructor
//-------------------------------------------------

device_cococart_interface::device_cococart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "cococart")
	, m_owning_slot(nullptr)
	, m_host(nullptr)
{
}


//-------------------------------------------------
//  ~device_cococart_interface - destructor
//-------------------------------------------------

device_cococart_interface::~device_cococart_interface()
{
}


//-------------------------------------------------
//  interface_config_complete
//-------------------------------------------------

void device_cococart_interface::interface_config_complete()
{
	m_owning_slot = dynamic_cast<cococart_slot_device *>(device().owner());
	m_host = m_owning_slot
			? dynamic_cast<device_cococart_host_interface *>(m_owning_slot->owner())
			: nullptr;
}


//-------------------------------------------------
//  interface_pre_start
//-------------------------------------------------

void device_cococart_interface::interface_pre_start()
{
	if (!m_owning_slot)
		throw emu_fatalerror("Expected device().owner() to be of type cococart_slot_device");
	if (!m_host)
		throw emu_fatalerror("Expected m_owning_slot->owner() to be of type device_cococart_host_interface");
}


//-------------------------------------------------
//  cts_read - Signifies a read where the CTS pin
//  on the cartridge slot was asserted ($C000-FFEF)
//-------------------------------------------------

u8 device_cococart_interface::cts_read(offs_t offset)
{
	return 0x00;
}


//-------------------------------------------------
//  cts_write - Signifies a write where the CTS pin
//  on the cartridge slot was asserted ($C000-FFEF)
//-------------------------------------------------

void device_cococart_interface::cts_write(offs_t offset, u8 data)
{
}


//-------------------------------------------------
//  scs_read - Signifies a read where the SCS pin
//  on the cartridge slot was asserted ($FF40-5F)
//-------------------------------------------------

u8 device_cococart_interface::scs_read(offs_t offset)
{
	return 0x00;
}


//-------------------------------------------------
//  scs_write - Signifies a write where the SCS pin
//  on the cartridge slot was asserted ($FF40-5F)
//-------------------------------------------------

void device_cococart_interface::scs_write(offs_t offset, u8 data)
{
}


//-------------------------------------------------
//  set_sound_enable
//-------------------------------------------------

void device_cococart_interface::set_sound_enable(bool sound_enable)
{
}


//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

u8 *device_cococart_interface::get_cart_base()
{
	return nullptr;
}


//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

u32 device_cococart_interface::get_cart_size()
{
	return 0x8000;
}


//-------------------------------------------------
//  set_cart_base_update
//-------------------------------------------------

void device_cococart_interface::set_cart_base_update(cococart_base_update_delegate update)
{
	m_update = update;
}


//-------------------------------------------------
//  cart_base_changed
//-------------------------------------------------

void device_cococart_interface::cart_base_changed(void)
{
	if (!m_update.isnull())
		m_update(get_cart_base());
	else
	{
		// propagate up to host interface
		device_cococart_interface *host = dynamic_cast<device_cococart_interface*>(m_host);
		host->cart_base_changed();
	}
}

/*-------------------------------------------------
    get_cart_memregion
-------------------------------------------------*/

memory_region *device_cococart_interface::get_cart_memregion()
{
	return 0;
}

//-------------------------------------------------
//  cartridge_space
//-------------------------------------------------

address_space &device_cococart_interface::cartridge_space()
{
	return host().cartridge_space();
}


//-------------------------------------------------
//  set_line_value
//-------------------------------------------------

void device_cococart_interface::set_line_value(cococart_slot_device::line line, cococart_slot_device::line_value value)
{
	owning_slot().set_line_value(line, value);
}


//-------------------------------------------------
//  coco_cart_add_basic_devices
//-------------------------------------------------

void coco_cart_add_basic_devices(device_slot_interface &device)
{
	// basic devices, on both the main slot and the Multi-Pak interface
	device.option_add_internal("banked_16k", COCO_PAK_BANKED);
	device.option_add_internal("pak", COCO_PAK);
	device.option_add("ccpsg", COCO_PSG);
	device.option_add("dcmodem", COCO_DCMODEM);
	device.option_add("gmc", COCO_PAK_GMC);
	device.option_add("ide", COCO_IDE);
	device.option_add("max", COCO_PAK_MAX);
	device.option_add("midi", COCO_MIDI);
	device.option_add("orch90", COCO_ORCH90);
	device.option_add("ram", COCO_PAK_RAM);
	device.option_add("rs232", COCO_RS232);
	device.option_add("ssc", COCO_SSC);
	device.option_add("ssfm", DRAGON_MSX2);
	device.option_add("stecomp", COCO_STEREO_COMPOSER);
	device.option_add("sym12", COCO_SYM12);
	device.option_add("wpk", COCO_WPK);
	device.option_add("wpk2", COCO_WPK2);
	device.option_add("wpkrs", COCO_WPKRS);
	device.option_add("wpk2p", COCO_WPK2P);
}


//-------------------------------------------------
//  coco_cart_add_fdcs
//-------------------------------------------------

void coco_cart_add_fdcs(device_slot_interface &device)
{
	// FDCs are optional because if they are on a Multi-Pak interface, they must
	// be on Slot 4
	device.option_add("cc2hdb1", COCO2_HDB1);
	device.option_add("cc3hdb1", COCO3_HDB1);
	device.option_add("cd6809_fdc", CD6809_FDC);
	device.option_add("cp450_fdc", CP450_FDC);
	device.option_add("fdc", COCO_FDC);
	device.option_add("fdcv11", COCO_FDC_V11);
}


//-------------------------------------------------
//  coco_cart_add_multi_pak
//-------------------------------------------------

void coco_cart_add_multi_pak(device_slot_interface &device)
{
	// and the Multi-Pak itself is optional because they cannot be daisy chained
	device.option_add("multi", COCO_MULTIPAK);
}


//-------------------------------------------------
//  dragon_cart_add_basic_devices
//-------------------------------------------------

void dragon_cart_add_basic_devices(device_slot_interface &device)
{
	device.option_add_internal("amtor", DRAGON_AMTOR);
	device.option_add("ccpsg", COCO_PSG);
	device.option_add("claw", DRAGON_CLAW);
	device.option_add("gmc", COCO_PAK_GMC);
	device.option_add("jcbsnd", DRAGON_JCBSND);
	device.option_add("jcbspch", DRAGON_JCBSPCH);
	device.option_add("max", COCO_PAK_MAX);
	device.option_add("midi", DRAGON_MIDI);
	device.option_add("orch90", COCO_ORCH90);
	device.option_add("pak", COCO_PAK);
	device.option_add("serial", DRAGON_SERIAL);
	device.option_add("ram", COCO_PAK_RAM);
	device.option_add("sprites", DRAGON_SPRITES);
	device.option_add("ssc", COCO_SSC);
	device.option_add("ssfm", DRAGON_MSX2);
	device.option_add("stecomp", COCO_STEREO_COMPOSER);
	device.option_add("sym12", COCO_SYM12);
	device.option_add("wpk2p", COCO_WPK2P);
}


//-------------------------------------------------
//  dragon_cart_add_fdcs
//-------------------------------------------------

void dragon_cart_add_fdcs(device_slot_interface &device)
{
	device.option_add("dragon_fdc", DRAGON_FDC);
	device.option_add("premier_fdc", PREMIER_FDC);
	device.option_add("sdtandy_fdc", SDTANDY_FDC);
}


//-------------------------------------------------
//  dragon_cart_add_multi_pak
//-------------------------------------------------

void dragon_cart_add_multi_pak(device_slot_interface &device)
{
	device.option_add("multi", DRAGON_MULTIPAK);
}
