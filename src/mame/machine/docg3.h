// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    M-Systems DiskOnChip G3 - Flash Disk with MLC NAND and M-Systems? x2 Technology

    (c) 2009 Tim Schuerewegen

*/

#ifndef MAME_MACHINE_DOCG3_H
#define MAME_MACHINE_DOCG3_H

#pragma once


// ======================> diskonchip_g3_device

class diskonchip_g3_device : public device_t, public device_nvram_interface
{
public:
	diskonchip_g3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	void set_size(int _size) { m_size = _size; }
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
public:
	uint16_t sec_1_r(offs_t offset);
	void sec_1_w(offs_t offset, uint16_t data);
	uint16_t sec_2_r(offs_t offset, uint16_t mem_mask = ~0);
	void sec_2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sec_3_r(offs_t offset);
	void sec_3_w(offs_t offset, uint16_t data);
private:
	uint32_t g3_offset_data_1();
	uint32_t g3_offset_data_2();
	uint32_t g3_offset_data_3();
	uint8_t g3_read_data();
	void g3_write_data(uint8_t data);
	uint16_t sec_2_read_1000();
	uint16_t sec_2_read_1074();
	uint8_t sec_2_read_1042();
	uint8_t sec_2_read_1046();
	uint8_t sec_2_read_1048();
	uint8_t sec_2_read_1049();
	uint8_t sec_2_read_104A();
	uint8_t sec_2_read_104B();
	uint8_t sec_2_read_104C();
	uint8_t sec_2_read_104D();
	uint8_t sec_2_read_104E();
	uint8_t sec_2_read_104F();
	uint8_t sec_2_read_100E();
	uint8_t sec_2_read_1014();
	uint8_t sec_2_read_1022();
	uint8_t sec_2_read_1038();
	uint16_t sec_2_read16(uint32_t offset);
	uint8_t sec_2_read8(uint32_t offset);
	void sec_2_write_100C(uint8_t data);
	void sec_2_write_1032(uint8_t data);
	void g3_erase_block();
	void sec_2_write_1034(uint8_t data);
	void sec_2_write_1036(uint8_t data);
	void sec_2_write_1040(uint16_t data);
	void sec_2_write_100A(uint8_t data);
	void sec_2_write16(uint32_t offset, uint16_t data);
	void sec_2_write8(uint32_t offset, uint8_t data);

	int m_size;

	uint32_t m_planes;
	uint32_t m_blocks;
	uint32_t m_pages;
	uint32_t m_user_data_size;
	uint32_t m_extra_area_size;
	std::unique_ptr<uint8_t[]> m_data[3];
	uint32_t m_data_size[3];
	uint8_t  m_sec_2[0x800];
	uint32_t m_data_1036;
	uint32_t m_data_1036_count;
	uint32_t m_transfer_offset;
	uint8_t  m_device;
	uint32_t m_block;
	uint32_t m_page;
	uint32_t m_plane;
	uint32_t m_transfersize;
	uint8_t  m_test;
	uint32_t m_address_count;
};

// device type definition
DECLARE_DEVICE_TYPE(DISKONCHIP_G3, diskonchip_g3_device)

#endif // MAME_MACHINE_DOCG3_H
