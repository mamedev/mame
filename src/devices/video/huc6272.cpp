// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese
/***************************************************************************

    Hudson/NEC HuC6272 "King" device

    TODO:
    - Use NSCSI instead of legacy one!

***************************************************************************/

#include "emu.h"

#include "video/huc6272.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(HUC6272, huc6272_device, "huc6272", "Hudson HuC6272 \"King\"")

ADDRESS_MAP_START(huc6272_device::microprg_map)
	AM_RANGE(0x00, 0x0f) AM_RAM AM_SHARE("microprg_ram")
ADDRESS_MAP_END

ADDRESS_MAP_START(huc6272_device::kram_map)
	AM_RANGE(0x000000, 0x0fffff) AM_RAM AM_SHARE("kram_page0")
	AM_RANGE(0x100000, 0x1fffff) AM_RAM AM_SHARE("kram_page1")
ADDRESS_MAP_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  huc6272_device - constructor
//-------------------------------------------------

huc6272_device::huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HUC6272, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		m_program_space_config("microprg", ENDIANNESS_LITTLE, 16, 4, 0, address_map_constructor(), address_map_constructor(FUNC(huc6272_device::microprg_map), this)),
		m_data_space_config("kram", ENDIANNESS_LITTLE, 32, 21, 0, address_map_constructor(), address_map_constructor(FUNC(huc6272_device::kram_map), this)),
		m_microprg_ram(*this, "microprg_ram"),
		m_kram_page0(*this, "kram_page0"),
		m_kram_page1(*this, "kram_page1"),
		m_scsibus(*this, "scsi"),
		m_scsi_data_in(*this, "scsi_data_in"),
		m_scsi_data_out(*this, "scsi_data_out"),
		m_scsi_ctrl_in(*this, "scsi_ctrl_in"),
		m_irq_changed_cb(*this)
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void huc6272_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void huc6272_device::device_start()
{
	m_irq_changed_cb.resolve_safe();

	assert( m_huc6271_tag != nullptr );

	m_huc6271 = machine().device<huc6271_device>(m_huc6271_tag);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void huc6272_device::device_reset()
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector huc6272_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_space_config),
		std::make_pair(AS_DATA,    &m_data_space_config)
	};
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_dword - read a dword at the given address
//-------------------------------------------------

inline uint32_t huc6272_device::read_dword(offs_t address)
{
	return space(AS_DATA).read_dword(address << 2);
}


//-------------------------------------------------
//  write_dword - write a dword at the given address
//-------------------------------------------------

inline void huc6272_device::write_dword(offs_t address, uint32_t data)
{
	space(AS_DATA).write_dword(address << 2, data);
}

void huc6272_device::write_microprg_data(offs_t address, uint16_t data)
{
	space(AS_PROGRAM).write_word(address << 1, data);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ32_MEMBER( huc6272_device::read )
{
	uint32_t res = 0;

	if((offset & 1) == 0)
	{
		/*
		xxxx xxxx ---- ---- ---- ---- ---- ---- Sub Channel Buffer
		---- ---- x--- ---- ---- ---- ---- ---- SCSI RST flag
		---- ---- -x-- ---- ---- ---- ---- ---- SCSI BUSY flag
		---- ---- --x- ---- ---- ---- ---- ---- SCSI REQ flag
		---- ---- ---x ---- ---- ---- ---- ---- SCSI MSG flag
		---- ---- ---- x--- ---- ---- ---- ---- SCSI CD flag
		---- ---- ---- -x-- ---- ---- ---- ---- SCSI IO flag
		---- ---- ---- --x- ---- ---- ---- ---- SCSI SEL flag
		---- ---- ---- ---- -x-- ---- ---- ---- SCSI IRQ pending
		---- ---- ---- ---- --x- ---- ---- ---- DMA IRQ pending
		---- ---- ---- ---- ---x ---- ---- ---- CD Sub Channel IRQ pending
		---- ---- ---- ---- ---- x--- ---- ---- Raster IRQ pending
		---- ---- ---- ---- ---- -x-- ---- ---- ADPCM IRQ pending
		---- ---- ---- ---- ---- ---- -xxx xxxx register read-back
		*/
		res = m_register & 0x7f;
		res |= (m_scsi_ctrl_in->read() & 0xff) << 16;
	}
	else
	{
		switch(m_register)
		{
			case 0x00: // SCSI data in
				res = m_scsi_data_in->read() & 0xff;
				break;

			case 0x05: // SCSI bus status
				res = m_scsi_ctrl_in->read() & 0xff;
				res|= (m_scsi_data_in->read() << 8);
				break;


			/*
			x--- ---- ---- ---- ----
			*/
			case 0x0c: // KRAM load address
				res = (m_kram_addr_r & 0x3ffff) | ((m_kram_inc_r & 0x1ff) << 18) | ((m_kram_page_r & 1) << 31);
				break;

			case 0x0d: // KRAM write address
				res = (m_kram_addr_w & 0x3ffff) | ((m_kram_inc_w & 0x1ff) << 18) | ((m_kram_page_w & 1) << 31);
				break;

			case 0x0e: // KRAM read data
				res = read_dword((m_kram_addr_r)|(m_kram_page_r<<18));
				m_kram_addr_r += (m_kram_inc_r & 0x200) ? ((m_kram_inc_r & 0x1ff) - 0x200) : (m_kram_inc_r & 0x1ff);
				break;

			case 0x0f:
				res = m_page_setting;
				break;
			//default: printf("%04x\n",m_register);
		}
	}

	return res;
}

WRITE32_MEMBER( huc6272_device::write )
{
	if((offset & 1) == 0)
		m_register = data & 0x7f;
	else
	{
		switch(m_register)
		{
			case 0x00: // SCSI data out
				m_scsi_data_out->write(data & 0xff);
				break;
			case 0x01: // SCSI command
				//m_scsibus->write_bsy(BIT(data, 0)); // bus?
				m_scsibus->write_atn(BIT(data, 1));
				m_scsibus->write_sel(BIT(data, 2));
				m_scsibus->write_ack(BIT(data, 4));
				m_scsibus->write_rst(BIT(data, 7));

				break;

			case 0x02: // SCSI mode
				break;

			case 0x03: // SCSI target command
				m_scsibus->write_io(BIT(data, 0));
				m_scsibus->write_cd(BIT(data, 1));
				m_scsibus->write_msg(BIT(data, 2));
				break;

			case 0x05: // SCSI bus status
				// bits 7-0: SCSI DMA trigger?
				m_scsi_data_out->write((data >> 8) & 0xff);
				break;

			case 0x06: // SCSI input data
			case 0x07: // SCSI DMA trigger
			case 0x08: // SCSI subcode
			case 0x09: // SCSI DMA start address
			case 0x0a: // SCSI DMA size
			case 0x0b: // SCSI DMA control
				break;
			/*
			---- ---- ---- ---- ----
			*/
			case 0x0c: // KRAM load address
				m_kram_addr_r = (data & 0x0003ffff);
				m_kram_inc_r =  (data & 0x0ffc0000) >> 18;
				m_kram_page_r = (data & 0x80000000) >> 31;
				break;

			case 0x0d: // KRAM write address
				m_kram_addr_w = (data & 0x0003ffff);
				m_kram_inc_w =  (data & 0x0ffc0000) >> 18;
				m_kram_page_w = (data & 0x80000000) >> 31;
				break;

			case 0x0e: // KRAM write data
				// TODO: handle non-dword cases?
				write_dword((m_kram_addr_w)|(m_kram_page_w<<18),data);
				m_kram_addr_w += (m_kram_inc_w & 0x200) ? ((m_kram_inc_w & 0x1ff) - 0x200) : (m_kram_inc_w & 0x1ff);
				break;

			/*
			---x ---- ---- ---- ADPCM page setting
			---- ---x ---- ---- RAINBOW page setting
			---- ---- ---x ---- BG page setting
			---- ---- ---- ---x SCSI page setting
			*/
			case 0x0f:
				m_page_setting = data;
				break;

			//
			// xxxx ---- ---- ---- BG3 mode setting
			// ---- xxxx ---- ---- BG2 mode setting
			// ---- ---- xxxx ---- BG1 mode setting
			// ---- ---- ---- xxxx BG0 mode setting
			//
			// 0001 - 4 color palette
			// 0010 - 16 color palette
			// 0011 - 256 color palette
			// 0100 - 64k color
			// 0101 - 16M color
			// 1001 - 4 color palette block mode
			// 1010 - 16 color palette block mode
			// 1011 - 256 color palette block mode
			// others - unused/invalid
			case 0x10:
				for(int i=0;i<4;i++)
					m_bg[i].mode = (data >> i*4) & 0x0f;

				break;

			/*
			---x ---- ---- ---- BG0 rotation enable
			---- xxx- ---- ---- BG3 priority
			---- ---x xx-- ---- BG2 priority
			---- ---- --xx x--- BG1 priority
			---- ---- ---- -xxx BG0 priority
			*/
			case 0x12:
				for(int i=0;i<4;i++)
					m_bg[i].priority = (data >> i*3) & 0x07;

				// TODO: rotation enable
				break;

			case 0x13:
				m_micro_prg.index = data & 0xf;
				break;

			case 0x14:
				write_microprg_data(m_micro_prg.index,data & 0xffff);
				m_micro_prg.index ++;
				m_micro_prg.index &= 0xf;
				break;

			case 0x15:
				m_micro_prg.ctrl = data & 1;

				break;

			// case 0x16: wrap-around enable

			// BAT and CG address setters
			case 0x20: m_bg[0].bat_address = data * 1024;  break;
			case 0x21: m_bg[0].cg_address = data * 1024;   break;
			case 0x22: m_bg0sub.bat_address = data * 1024; break;
			case 0x23: m_bg0sub.cg_address = data * 1024;  break;
			case 0x24: m_bg[1].bat_address = data * 1024;  break;
			case 0x25: m_bg[1].cg_address = data * 1024;   break;
			case 0x26: m_bg[2].bat_address = data * 1024;  break;
			case 0x27: m_bg[2].cg_address = data * 1024;   break;
			case 0x28: m_bg[3].bat_address = data * 1024;  break;
			case 0x29: m_bg[3].cg_address = data * 1024;   break;

			// Height & Width setters
			case 0x2c:
			case 0x2d:
			case 0x2e:
			case 0x2f:
			{
				uint8_t reg_offs = m_register & 3;
				m_bg[reg_offs].height = 1 << (data & 0x000f);
				m_bg[reg_offs].width = 1 << ((data & 0x00f0) >> 4);
				if(reg_offs == 0)
				{
					m_bg0sub.height = 1 << ((data & 0x0f00) >> 8);
					m_bg0sub.width = 1 << ((data & 0xf000) >> 12);
				}
				break;
			}

			// X & Y scroll values
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			{
				uint8_t reg_offs = (m_register & 6) >> 1;

				if(m_register & 1)
					m_bg[reg_offs].yscroll = data & 0xffff;
				else
					m_bg[reg_offs].xscroll = data & 0xffff;
				break;
			}

			//default: printf("%04x %04x %08x\n",m_register,data,mem_mask);
		}
	}
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(huc6272_device::device_add_mconfig)
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSI_RST_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit7))
	MCFG_SCSI_BSY_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit6))
	MCFG_SCSI_REQ_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit5))
	MCFG_SCSI_MSG_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit4))
	MCFG_SCSI_CD_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit3))
	MCFG_SCSI_IO_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit2))
	MCFG_SCSI_SEL_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit1))

	MCFG_SCSI_DATA_INPUT_BUFFER("scsi_data_in")

	MCFG_SCSI_OUTPUT_LATCH_ADD("scsi_data_out", "scsi")
	MCFG_DEVICE_ADD("scsi_ctrl_in", INPUT_BUFFER, 0)
	MCFG_DEVICE_ADD("scsi_data_in", INPUT_BUFFER, 0)

	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "cdrom", SCSICD, SCSI_ID_1)
MACHINE_CONFIG_END
