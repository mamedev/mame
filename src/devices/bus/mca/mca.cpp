// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM's Micro Channel bus, the PC version.
	Supports 16-bit and 32-bit cards.

***************************************************************************/

#include "emu.h"
#include "mca.h"

#define LOG_GENERIC		(1U <<	0)
#define LOG_SYSPORTS    (1U <<  2)
#define LOG_NVRAM       (1U <<  3)
#define LOG_TIMERS      (1U <<  4)
#define LOG_POST        (1U <<  5)
#define LOG_POS         (1U <<  6)

#define VERBOSE (LOG_GENERIC|LOG_SYSPORTS|LOG_NVRAM|LOG_TIMERS|LOG_POST|LOG_POS)
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define LOGGENERIC(...)    	LOGMASKED(LOG_GENERIC, __VA_ARGS__)
#define LOGSYSPORTS(...)    LOGMASKED(LOG_SYSPORTS, __VA_ARGS__)
#define LOGNVRAM(...)       LOGMASKED(LOG_NVRAM, __VA_ARGS__)
#define LOGTIMERS(...)      LOGMASKED(LOG_TIMERS, __VA_ARGS__)
#define LOGPOST(...)        LOGMASKED(LOG_POST, __VA_ARGS__)
#define LOGPOS(...)         LOGMASKED(LOG_POS, __VA_ARGS__)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_SLOT, mca16_slot_device, "mca16_slot", "16-bit MCA slot")
DEFINE_DEVICE_TYPE(MCA32_SLOT, mca32_slot_device, "mca32_slot", "32-bit MCA slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mca16_slot_device - constructor
//-------------------------------------------------
mca16_slot_device::mca16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_slot_device(mconfig, MCA16_SLOT, tag, owner, clock)
{
}

mca16_slot_device::mca16_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_mca_bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_slot_device::device_start()
{
	device_mca16_card_interface *const dev = dynamic_cast<device_mca16_card_interface *>(get_card_device());

	if (dev) dev->set_mcabus(m_mca_bus);

	// tell MCA bus that there is one slot with the specified tag
	downcast<mca16_device &>(*m_mca_bus).add_slot(tag());
}

/***************************
 * 16-bit MCA bus
 ***************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16, mca16_device, "mca16", "16-bit MCA bus")
DEFINE_DEVICE_TYPE(MCA32, mca32_device, "mca32", "32-bit MCA bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mca16_device - constructor
//-------------------------------------------------

mca16_device::mca16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_device(mconfig, MCA16, tag, owner, clock)
{
}

mca16_device::mca16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_mem16_config("mem16", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor()),
	m_io16_config("io16", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor()),
	m_memspace(*this, finder_base::DUMMY_TAG, -1),
	m_iospace(*this, finder_base::DUMMY_TAG, -1),
	m_memwidth(0),
	m_iowidth(0),
	m_allocspaces(false),
	m_out_irq_cb(*this),
	m_out_drq_cb(*this),
	m_write_iochrdy(*this),
	m_write_iochck(*this),
	m_cs_feedback(*this)
{
	std::fill(std::begin(m_dma_device), std::end(m_dma_device), nullptr);
	std::fill(std::begin(m_dma_eop), std::end(m_dma_eop), false);
}

device_memory_interface::space_config_vector mca16_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_MCA_MEM16,  &m_mem16_config),
		std::make_pair(AS_MCA_IO16,   &m_io16_config),	
	};
}

void mca16_device::set_dma_channel(uint8_t channel, device_mca16_card_interface *dev, bool do_eop)
{
	m_dma_device[channel] = dev;
	m_dma_eop[channel] = do_eop;
}

void mca16_device::unset_dma_channel(uint8_t channel)
{
	m_dma_device[channel] = NULL;
}

void mca16_device::add_slot(const char *tag)
{
	device_t *dev = subdevice(tag);
	add_slot(dynamic_cast<device_slot_interface *>(dev));
}

void mca16_device::add_slot(device_slot_interface *slot)
{
	m_slot_list.push_front(slot);
}

//-------------------------------------------------
//  device_config_complete - - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mca16_device::device_config_complete()
{
	printf("FIXME: MCA16 is not using its internal address space in device_config_complete\n");
	m_memspace.set_tag(*this, ":maincpu", AS_PROGRAM);
	m_iospace.set_tag(*this, ":maincpu", AS_IO);
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void mca16_device::device_resolve_objects()
{
	// resolve callbacks
	m_write_iochrdy.resolve_safe();
	m_write_iochck.resolve_safe();

	m_out_irq_cb.resolve_all_safe();
	m_out_drq_cb.resolve_all_safe();

	m_cs_feedback.resolve_safe();

	m_iowidth = m_iospace->data_width();
	m_memwidth = m_memspace->data_width();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_device::device_start()
{
	
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_device::device_reset()
{
}


template <typename R, typename W> void mca16_device::install_space(int spacenum, offs_t start, offs_t end, R rhandler, W whandler)
{
	LOG("%s\n", FUNCNAME);

	int buswidth;
	address_space *space;

	if (spacenum == AS_MCA_IO16)
	{
		space = m_iospace.target();
		buswidth = m_iowidth;
	}
	else if (spacenum == AS_MCA_MEM16)
	{
		space = m_memspace.target();
		buswidth = m_memwidth;
	}
	else
	{
		fatalerror("Unknown space passed to mca16_device::install_space!\n");
	}

	switch (buswidth)
	{
		case 8:
			space->install_read_handler(start, end, rhandler, 0);
			space->install_write_handler(start, end, whandler, 0);
			break;
		case 16:
			space->install_read_handler(start, end, rhandler, 0xffff);
			space->install_write_handler(start, end, whandler, 0xffff);
			break;
		case 32:
			if ((start % 4) == 0)
			{
				if ((end - start) == 1)
				{
					space->install_read_handler(start, end + 2, rhandler, 0x0000ffff);
					space->install_write_handler(start, end + 2, whandler, 0x0000ffff);
				}
				else
				{
					space->install_read_handler(start, end, rhandler, 0xffffffff);
					space->install_write_handler(start, end, whandler, 0xffffffff);
				}
			}
			else
			{
				// we handle just misaligned by 2
				space->install_read_handler(start - 2, end, rhandler, 0xffff0000);
				space->install_write_handler(start - 2, end, whandler, 0xffff0000);
			}
			break;
		default:
			fatalerror("MCA16: Bus width %d not supported\n", buswidth);
	}
}

template void mca16_device::install_space<read8_delegate,    write8_delegate   >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8_delegate whandler);
template void mca16_device::install_space<read8_delegate,    write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8m_delegate whandler);
template void mca16_device::install_space<read8_delegate,    write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8s_delegate whandler);
template void mca16_device::install_space<read8_delegate,    write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8sm_delegate whandler);
template void mca16_device::install_space<read8_delegate,    write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8mo_delegate whandler);
template void mca16_device::install_space<read8_delegate,    write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8smo_delegate whandler);

template void mca16_device::install_space<read8m_delegate,   write8_delegate   >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8_delegate whandler);
template void mca16_device::install_space<read8m_delegate,   write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8m_delegate whandler);
template void mca16_device::install_space<read8m_delegate,   write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8s_delegate whandler);
template void mca16_device::install_space<read8m_delegate,   write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8sm_delegate whandler);
template void mca16_device::install_space<read8m_delegate,   write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8mo_delegate whandler);
template void mca16_device::install_space<read8m_delegate,   write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8smo_delegate whandler);

template void mca16_device::install_space<read8s_delegate,   write8_delegate   >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8_delegate whandler);
template void mca16_device::install_space<read8s_delegate,   write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8m_delegate whandler);
template void mca16_device::install_space<read8s_delegate,   write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8s_delegate whandler);
template void mca16_device::install_space<read8s_delegate,   write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8sm_delegate whandler);
template void mca16_device::install_space<read8s_delegate,   write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8mo_delegate whandler);
template void mca16_device::install_space<read8s_delegate,   write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8smo_delegate whandler);

template void mca16_device::install_space<read8sm_delegate,  write8_delegate   >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8_delegate whandler);
template void mca16_device::install_space<read8sm_delegate,  write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8m_delegate whandler);
template void mca16_device::install_space<read8sm_delegate,  write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8s_delegate whandler);
template void mca16_device::install_space<read8sm_delegate,  write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8sm_delegate whandler);
template void mca16_device::install_space<read8sm_delegate,  write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8mo_delegate whandler);
template void mca16_device::install_space<read8sm_delegate,  write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8smo_delegate whandler);

template void mca16_device::install_space<read8mo_delegate,  write8_delegate   >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8_delegate whandler);
template void mca16_device::install_space<read8mo_delegate,  write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8m_delegate whandler);
template void mca16_device::install_space<read8mo_delegate,  write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8s_delegate whandler);
template void mca16_device::install_space<read8mo_delegate,  write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8sm_delegate whandler);
template void mca16_device::install_space<read8mo_delegate,  write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8mo_delegate whandler);
template void mca16_device::install_space<read8mo_delegate,  write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8smo_delegate whandler);

template void mca16_device::install_space<read8smo_delegate, write8_delegate   >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8_delegate whandler);
template void mca16_device::install_space<read8smo_delegate, write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8m_delegate whandler);
template void mca16_device::install_space<read8smo_delegate, write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8s_delegate whandler);
template void mca16_device::install_space<read8smo_delegate, write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8sm_delegate whandler);
template void mca16_device::install_space<read8smo_delegate, write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8mo_delegate whandler);
template void mca16_device::install_space<read8smo_delegate, write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8smo_delegate whandler);

template void mca16_device::install_space<read16sm_delegate, write16sm_delegate >(int spacenum, offs_t start, offs_t end, read16sm_delegate rhandler,  write16sm_delegate whandler);

void mca16_device::install_bank(offs_t start, offs_t end, uint8_t *data)
{
	m_memspace->install_ram(start, end, data);
}

void mca16_device::install_bank(offs_t start, offs_t end, memory_bank *bank)
{
	m_memspace->install_readwrite_bank(start, end, bank);
}

void mca16_device::unmap_bank(offs_t start, offs_t end)
{
	m_memspace->unmap_readwrite(start, end);
}

void mca16_device::install_rom(device_t *dev, offs_t start, offs_t end, const char *region)
{
	if (machine().root_device().memregion("mca")) {
		uint8_t *src = dev->memregion(region)->base();
		uint8_t *dest = machine().root_device().memregion("mca")->base() + start - 0xc0000;
		memcpy(dest,src, end - start + 1);
	} else {
		m_memspace->install_rom(start, end, machine().root_device().memregion(dev->subtag(region).c_str())->base());
		m_memspace->unmap_write(start, end);
	}
}

void mca16_device::unmap_rom(offs_t start, offs_t end)
{
	m_memspace->unmap_read(start, end);
}

bool mca16_device::is_option_rom_space_available(offs_t start, int size)
{
	for(int i = 0; i < size; i += 4096) // 4KB granularity
		if(m_memspace->get_read_ptr(start + i)) return false;
	return true;
}

void mca16_device::unmap_readwrite(offs_t start, offs_t end)
{
	m_memspace->unmap_readwrite(start, end);
}

uint8_t mca16_device::dack_r(int line)
{
	//LOG("MCA DACK R %d\n", line);

	if (m_dma_device[line])
		return m_dma_device[line]->dack_r(line);
	return 0xff;
}

void mca16_device::dack_w(int line, uint8_t data)
{
	//LOG("MCA DACK W %d\n", line);

	if (m_dma_device[line])
		return m_dma_device[line]->dack_w(line,data);
}

void mca16_device::dack_line_w(int line, int state)
{
	//LOG("MCA DACK line w %d\n", line);

	if (m_dma_device[line])
		m_dma_device[line]->dack_line_w(line, state);
}

void mca16_device::eop_w(int channel, int state)
{
	//LOG("MCA EOP ch %d %d\n", channel, state);

	if (m_dma_eop[channel] && m_dma_device[channel])
		m_dma_device[channel]->eop_w(state);
}

void mca16_device::set_ready(int state)
{
	m_write_iochrdy(state);
}

void mca16_device::nmi()
{
	// active low pulse
	m_write_iochck(0);
	m_write_iochck(1);
}

//**************************************************************************
//  DEVICE CONFIG MCA16 CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE MCA16 CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_mca16_card_interface - constructor
//-------------------------------------------------

device_mca16_card_interface::device_mca16_card_interface(const machine_config &mconfig, device_t &device, uint16_t card_id)
	: device_interface(device, "mca"),
		m_card_id(card_id), m_mca(nullptr), m_mca_dev(nullptr), m_next(nullptr)
{

}

//-------------------------------------------------
//  ~device_mca16_card_interface - destructor
//-------------------------------------------------

device_mca16_card_interface::~device_mca16_card_interface()
{
}

/// @brief Read a POS register from an MCA card
///
/// Reads one of the Peripheral Option Select registers from the
/// MCA card device. Bytes 0 and 1 are always the card ID.
/// Anything further must be implemented in the card device.
/// @param [in] offset The POS register to read.
/// @return The data byte read from POS.
uint8_t device_mca16_card_interface::pos_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch(offset)
		{
			case 0:
				// Adapter Identification b0-b7
				data = m_card_id & 0xff; break;
			case 1:
				// Adapter Identification b8-b15
				data = (m_card_id >> 8) & 0xff; break;
			case 2:
				// Option Select Data 1
				data = m_option_select[MCABus::POS::OPTION_SELECT_DATA_1]; break;
			case 3:
				// Option Select Data 2
				data = m_option_select[MCABus::POS::OPTION_SELECT_DATA_2]; break;
			case 4:
				// Option Select Data 3
				data = m_option_select[MCABus::POS::OPTION_SELECT_DATA_3]; break;
			case 5:
				// Option Select Data 4
				data = m_option_select[MCABus::POS::OPTION_SELECT_DATA_4]; break;
			case 6:
				// Subaddress Extension Low - most devices don't implement this
				data = m_option_select[MCABus::POS::SUBADDRESS_EXT_LO]; break;
			case 7:
				// Subaddress Extension High - most devices don't implement this
				data = m_option_select[MCABus::POS::SUBADDRESS_EXT_HI]; break;
		}
	return data; // out of range? driver setup error
}

/// @brief Write a POS register to an MCA card.
///
/// Writes one of the Peripheral Option Select registers in the
/// MCA card device. This typically changes resource assignment.
/// @param [in] offset The POS register to write.
/// @param [in] data The byte to write to the POS register.
void device_mca16_card_interface::pos_w(offs_t offset, uint8_t data)
{
	// Generic POS implementation that will call inherited write members.

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
		case 1:
			// Adapter Identification b8-b15
			break;
		case 2:
			// Option Select Data 1
            update_pos_data_1(data);
			break;
		case 3:
			// Option Select Data 2
            update_pos_data_2(data);
			break;
		case 4:
			// Option Select Data 3
			update_pos_data_3(data);
			break;
		case 5:
			// Option Select Data 4
			update_pos_data_4(data);
			break;
		case 6:
			// Subaddress Extension Low
			break;
		case 7:
			// Subaddress Extension High
			break;
		default:
			break;
	}
}

uint16_t device_mca16_card_interface::io16_r(offs_t offset)
{
	// If no special 16-bit handler, perform two 8-bit reads.
	uint16_t lo = io8_r(offset);
	uint16_t hi = io8_r(offset+1);

	return (hi << 8) | lo;
}

void device_mca16_card_interface::io16_w(offs_t offset, uint16_t data)
{
	io8_w(offset, data & 0xFF);
	io8_w(offset, (data & 0xFF00) >> 8);
}

uint16_t mca16_device::dack16_r(int line)
{
	if (m_dma_device[line])
		return dynamic_cast<device_mca16_card_interface *>(m_dma_device[line])->dack_r(line);
	return 0xffff;
}

void mca16_device::dack16_w(int line, uint16_t data)
{
	if (m_dma_device[line])
		return dynamic_cast<device_mca16_card_interface *>(m_dma_device[line])->dack_w(line,data);
}

//-------------------------------------------------
//  32-bit MCA bus
//-------------------------------------------------

mca32_device::mca32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca32_device(mconfig, MCA32, tag, owner, clock)
{
}

mca32_device::mca32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mca16_device(mconfig, type, tag, owner, clock),
	m_mem32_config("mem32", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor()),
	m_io32_config("io32", ENDIANNESS_LITTLE, 32, 16, 0, address_map_constructor())
{
	std::fill(std::begin(m_dma_device), std::end(m_dma_device), nullptr);
	std::fill(std::begin(m_dma_eop), std::end(m_dma_eop), false);
}

device_memory_interface::space_config_vector mca32_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_MCA_MEM32,  &m_mem32_config),
		std::make_pair(AS_MCA_IO32,   &m_io32_config),	
	};
}

void mca32_device::device_config_complete()
{
	printf("FIXME: MCA32 is not using its internal address space in device_config_complete\n");
	m_memspace.set_tag(*this, ":maincpu", AS_PROGRAM);
	m_iospace.set_tag(*this, ":maincpu", AS_IO);
}

/***********************************************
 * MCA 32-bit card device
 ***********************************************/

device_mca32_card_interface::device_mca32_card_interface(const machine_config &mconfig, device_t &device, uint16_t card_id)
	: device_mca16_card_interface(mconfig, device, card_id)
{
}

void device_mca32_card_interface::set_mca_device()
{
	m_mca = dynamic_cast<mca32_device *>(m_mca_dev);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

mca32_slot_device::mca32_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca32_slot_device(mconfig, MCA32_SLOT, tag, owner, clock)
{
}

mca32_slot_device::mca32_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mca16_slot_device(mconfig, type, tag, owner, clock)
{
}

void mca32_slot_device::device_start()
{
	device_mca32_card_interface *const dev = dynamic_cast<device_mca32_card_interface *>(get_card_device());

	if (dev) dev->set_mcabus(m_mca_bus);

	// tell MCA bus that there is one slot with the specified tag
	downcast<mca32_device &>(*m_mca_bus).add_slot(tag());
}

template <typename R, typename W> void mca32_device::install_space(int spacenum, offs_t start, offs_t end, R rhandler, W whandler)
{
	LOG("%s\n", FUNCNAME);

	int buswidth;
	address_space *space;

	if (spacenum == AS_MCA_IO32)
	{
		space = m_iospace.target();
		buswidth = m_iowidth;
	}
	else if (spacenum == AS_MCA_MEM32)
	{
		space = m_memspace.target();
		buswidth = m_memwidth;
	}
	else
	{
		fatalerror("Unknown space passed to mca32_device::install_space!\n");
	}

	switch (buswidth)
	{
		case 8:
			space->install_read_handler(start, end, rhandler, 0);
			space->install_write_handler(start, end, whandler, 0);
			break;
		case 16:
			space->install_read_handler(start, end, rhandler, 0xffff);
			space->install_write_handler(start, end, whandler, 0xffff);
			break;
		case 32:
			if ((start % 4) == 0)
			{
				if ((end - start) == 1)
				{
					space->install_read_handler(start, end + 2, rhandler, 0x0000ffff);
					space->install_write_handler(start, end + 2, whandler, 0x0000ffff);
				}
				else
				{
					space->install_read_handler(start, end, rhandler, 0xffffffff);
					space->install_write_handler(start, end, whandler, 0xffffffff);
				}
			}
			else
			{
				// we handle just misaligned by 2
				space->install_read_handler(start - 2, end, rhandler, 0xffff0000);
				space->install_write_handler(start - 2, end, whandler, 0xffff0000);
			}
			break;
		default:
			fatalerror("MCA16: Bus width %d not supported\n", buswidth);
	}
}

template void mca32_device::install_space<read8_delegate,    write8_delegate   >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8_delegate whandler);
template void mca32_device::install_space<read8_delegate,    write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8m_delegate whandler);
template void mca32_device::install_space<read8_delegate,    write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8s_delegate whandler);
template void mca32_device::install_space<read8_delegate,    write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8sm_delegate whandler);
template void mca32_device::install_space<read8_delegate,    write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8mo_delegate whandler);
template void mca32_device::install_space<read8_delegate,    write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8smo_delegate whandler);

template void mca32_device::install_space<read8m_delegate,   write8_delegate   >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8_delegate whandler);
template void mca32_device::install_space<read8m_delegate,   write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8m_delegate whandler);
template void mca32_device::install_space<read8m_delegate,   write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8s_delegate whandler);
template void mca32_device::install_space<read8m_delegate,   write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8sm_delegate whandler);
template void mca32_device::install_space<read8m_delegate,   write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8mo_delegate whandler);
template void mca32_device::install_space<read8m_delegate,   write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8smo_delegate whandler);

template void mca32_device::install_space<read8s_delegate,   write8_delegate   >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8_delegate whandler);
template void mca32_device::install_space<read8s_delegate,   write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8m_delegate whandler);
template void mca32_device::install_space<read8s_delegate,   write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8s_delegate whandler);
template void mca32_device::install_space<read8s_delegate,   write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8sm_delegate whandler);
template void mca32_device::install_space<read8s_delegate,   write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8mo_delegate whandler);
template void mca32_device::install_space<read8s_delegate,   write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8smo_delegate whandler);

template void mca32_device::install_space<read8sm_delegate,  write8_delegate   >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8_delegate whandler);
template void mca32_device::install_space<read8sm_delegate,  write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8m_delegate whandler);
template void mca32_device::install_space<read8sm_delegate,  write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8s_delegate whandler);
template void mca32_device::install_space<read8sm_delegate,  write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8sm_delegate whandler);
template void mca32_device::install_space<read8sm_delegate,  write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8mo_delegate whandler);
template void mca32_device::install_space<read8sm_delegate,  write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8smo_delegate whandler);

template void mca32_device::install_space<read8mo_delegate,  write8_delegate   >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8_delegate whandler);
template void mca32_device::install_space<read8mo_delegate,  write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8m_delegate whandler);
template void mca32_device::install_space<read8mo_delegate,  write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8s_delegate whandler);
template void mca32_device::install_space<read8mo_delegate,  write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8sm_delegate whandler);
template void mca32_device::install_space<read8mo_delegate,  write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8mo_delegate whandler);
template void mca32_device::install_space<read8mo_delegate,  write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8smo_delegate whandler);

template void mca32_device::install_space<read8smo_delegate, write8_delegate   >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8_delegate whandler);
template void mca32_device::install_space<read8smo_delegate, write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8m_delegate whandler);
template void mca32_device::install_space<read8smo_delegate, write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8s_delegate whandler);
template void mca32_device::install_space<read8smo_delegate, write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8sm_delegate whandler);
template void mca32_device::install_space<read8smo_delegate, write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8mo_delegate whandler);
template void mca32_device::install_space<read8smo_delegate, write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8smo_delegate whandler);
