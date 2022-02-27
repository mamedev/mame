// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese
/**************************************************************************************************

    Hudson/NEC HuC6272 "King" device

    TODO:
    - Use NSCSI instead of legacy one;
    - Convert base mapping to address_map;
    - Convert I/O to space address, and make it honor mem_mask;
    - subclass "SCSICD" into SCSI-2 "CD-ROM DRIVE:FX"
      \- Crashes if CD-ROM is in, on unhandled command 0x28 "Read(10)";
      \- During POST it tries an unhandled 0x44 "Read Header";
      \- Derivative design of PCE drive, which in turn is a derivative of PC-8801-30 (cd drive)
         and PC-8801-31 (interface);
    - Implement video routines drawing and interface:
      \- BIOS main menu draws BG0 only as backdrop of the PCE VDCs with 16M mode (5);
    - Implement video mixing with other PCFX chips;
    - Implement microprogram (layer timings, sort of Sega Saturn VRAM cycle patterns);
    - Implement Rainbow transfers (NEC logo on POST);
    - Verify ADPCM transfers;

    ADPCM related patents:
    - https://patents.google.com/patent/US5692099
    - https://patents.google.com/patent/US6453286
    - https://patents.google.com/patent/US5548655A

**************************************************************************************************/

#include "emu.h"
#include "video/huc6272.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(HUC6272, huc6272_device, "huc6272", "Hudson HuC6272 \"King\"")

void huc6272_device::microprg_map(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00, 0x0f).ram().share("microprg_ram");
}

void huc6272_device::kram_map(address_map &map)
{
	if (!has_configured_map(1))
	{
		map(0x000000, 0x0fffff).ram().share("kram_page0");
		map(0x100000, 0x1fffff).ram().share("kram_page1");
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  huc6272_device - constructor
//-------------------------------------------------

huc6272_device::huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HUC6272, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		m_huc6271(*this, finder_base::DUMMY_TAG),
		m_cdda_l(*this, "cdda_l"),
		m_cdda_r(*this, "cdda_r"),
		m_program_space_config("microprg", ENDIANNESS_LITTLE, 16, 4, 0, address_map_constructor(FUNC(huc6272_device::microprg_map), this)),
		m_data_space_config("kram", ENDIANNESS_LITTLE, 32, 21, 0, address_map_constructor(FUNC(huc6272_device::kram_map), this)),
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

	save_item(NAME(m_register));
	save_item(NAME(m_kram_addr_r));
	save_item(NAME(m_kram_inc_r));
	save_item(NAME(m_kram_page_r));
	save_item(NAME(m_kram_addr_w));
	save_item(NAME(m_kram_inc_w));
	save_item(NAME(m_kram_page_w));
	save_item(NAME(m_page_setting));

	for (int bg = 0; bg < 4; bg++)
	{
		save_item(NAME(m_bg[bg].bat_address), bg);
		save_item(NAME(m_bg[bg].cg_address), bg);
		save_item(NAME(m_bg[bg].mode), bg);
		save_item(NAME(m_bg[bg].height), bg);
		save_item(NAME(m_bg[bg].width), bg);
		save_item(NAME(m_bg[bg].xscroll), bg);
		save_item(NAME(m_bg[bg].yscroll), bg);
		save_item(NAME(m_bg[bg].priority), bg);
	}

	save_item(NAME(m_bg0sub.bat_address));
	save_item(NAME(m_bg0sub.cg_address));
	save_item(NAME(m_bg0sub.height));
	save_item(NAME(m_bg0sub.width));

	save_item(NAME(m_micro_prg.index));
	save_item(NAME(m_micro_prg.ctrl));

	save_item(NAME(m_adpcm.rate));
	save_item(NAME(m_adpcm.status));
	save_item(NAME(m_adpcm.interrupt));
	for (int adpcm = 0; adpcm < 2; adpcm++)
	{
		save_item(NAME(m_adpcm.playing[adpcm]), adpcm);
		save_item(NAME(m_adpcm.control[adpcm]), adpcm);
		save_item(NAME(m_adpcm.start[adpcm]), adpcm);
		save_item(NAME(m_adpcm.end[adpcm]), adpcm);
		save_item(NAME(m_adpcm.imm[adpcm]), adpcm);
		save_item(NAME(m_adpcm.input[adpcm]), adpcm);
		save_item(NAME(m_adpcm.nibble[adpcm]), adpcm);
		save_item(NAME(m_adpcm.pos[adpcm]), adpcm);
		save_item(NAME(m_adpcm.addr[adpcm]), adpcm);
	}
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

uint32_t huc6272_device::read(offs_t offset)
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
		res |= (m_adpcm.interrupt << 10);
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

			case 0x53: // ADPCM status
				res = m_adpcm.status;

				m_adpcm.status = 0;
				m_adpcm.interrupt = 0;
				interrupt_update();
				break;
			//default: printf("%04x\n",m_register);
		}
	}

	return res;
}

void huc6272_device::write(offs_t offset, uint32_t data)
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

			case 0x50: // ADPCM control
			{
				for (int i = 0; i < 2; i++)
				{
					m_adpcm.playing[i] = BIT(data, i);
					if (!m_adpcm.playing[i])
					{
						m_adpcm.input[i] = -1;
						m_adpcm.pos[i] = 0;
					}
					else
					{
						m_adpcm.addr[i] = m_adpcm.start[i];
					}
				}

				m_adpcm.rate = (data & 0xc) >> 2;
				break;
			}

			// ADPCM channel control
			case 0x51:
			case 0x52:
			{
				uint8_t reg_offs = 1-(m_register & 1);
				m_adpcm.control[reg_offs] = data & 0x7;
				if (BIT(m_adpcm.control[reg_offs], 1) == 0)
					m_adpcm.status &= ~(1 << (2*reg_offs));

				if (BIT(m_adpcm.control[reg_offs], 2) == 0)
					m_adpcm.status &= ~(1 << (2*reg_offs+1));

				break;
			}

			// ADPCM start address
			case 0x58:
			case 0x5c:
				m_adpcm.start[(m_register >> 2) & 1] = (data << 8) & 0x3ffff;
				break;

			// ADPCM end address
			case 0x59:
			case 0x5d:
				m_adpcm.end[(m_register >> 2) & 1] = data & 0x3ffff;
				break;

			// ADPCM intermediate address
			case 0x5a:
			case 0x5e:
				m_adpcm.imm[(m_register >> 2) & 1] = (data << 6) & 0x3ffff;
				break;

			//default: printf("%04x %04x %08x\n",m_register,data,mem_mask);
		}
	}
}

uint8_t huc6272_device::adpcm_update(int chan)
{
	if (!m_adpcm.playing[chan])
		return 0;

	const unsigned rate = (1 << m_adpcm.rate);
	m_adpcm.pos[chan]++;
	if (m_adpcm.pos[chan] >= rate)
	{
		if (m_adpcm.input[chan] == -1)
		{
			m_adpcm.input[chan] = read_dword(((m_page_setting & 0x1000) << 6) | m_adpcm.addr[chan]);
			m_adpcm.addr[chan] = (m_adpcm.addr[chan] & 0x20000) | ((m_adpcm.addr[chan] + 1) & 0x1ffff);
			if (m_adpcm.addr[chan] == m_adpcm.imm[chan])
			{
				m_adpcm.status |= (1 << (chan*2+1));
				if (BIT(m_adpcm.control[chan], 2))
				{
					m_adpcm.interrupt = 1;
					interrupt_update();
				}
			}
			if (m_adpcm.addr[chan] > m_adpcm.end[chan])
			{
				m_adpcm.status |= (1 << (chan*2));
				if (BIT(m_adpcm.control[chan], 1))
				{
					m_adpcm.interrupt = 1;
					interrupt_update();
				}

				if (BIT(m_adpcm.control[chan],0)) // Ring Buffer
				{
					m_adpcm.addr[chan] = m_adpcm.start[chan];
				}
				else
				{
					m_adpcm.playing[chan] = 0;
					return 0;
				}
			}
			m_adpcm.nibble[chan] = 0;
		}
		else
		{
			m_adpcm.nibble[chan] += 4;
			if (m_adpcm.nibble[chan] >= 28)
				m_adpcm.input[chan] = -1;
		}
		m_adpcm.pos[chan] = 0;
	}

	return (m_adpcm.input[chan] >> m_adpcm.nibble[chan]) & 0xf;
}

uint8_t huc6272_device::adpcm_update_0()
{
	return adpcm_update(0);
}

uint8_t huc6272_device::adpcm_update_1()
{
	return adpcm_update(1);
}

void huc6272_device::cdda_update(offs_t offset, uint8_t data)
{
	if (offset)
		m_cdda_r->set_output_gain(ALL_OUTPUTS, float(data) / 63.0);
	else
		m_cdda_l->set_output_gain(ALL_OUTPUTS, float(data) / 63.0);
}

void huc6272_device::interrupt_update()
{
	if (m_adpcm.interrupt)
		m_irq_changed_cb(ASSERT_LINE);
	else
		m_irq_changed_cb(CLEAR_LINE);
}

void huc6272_device::cdrom_config(device_t *device)
{
	cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	cdda->add_route(0, "^^cdda_l", 1.0);
	cdda->add_route(1, "^^cdda_r", 1.0);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void huc6272_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, m_cdda_l).front_left();
	SPEAKER(config, m_cdda_r).front_right();

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_data_input_buffer("scsi_data_in");
	scsibus.rst_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit7));
	scsibus.bsy_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit6));
	scsibus.req_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit5));
	scsibus.msg_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit4));
	scsibus.cd_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit3));
	scsibus.io_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit2));
	scsibus.sel_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit1));

	output_latch_device &scsiout(OUTPUT_LATCH(config, "scsi_data_out"));
	scsibus.set_output_latch(scsiout);

	INPUT_BUFFER(config, "scsi_ctrl_in");
	INPUT_BUFFER(config, "scsi_data_in");

	scsibus.set_slot_device(1, "cdrom", SCSICD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
	scsibus.slot(1).set_option_machine_config("cdrom", cdrom_config);
}
