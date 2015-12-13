// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    M-Systems DiskOnChip G3 - Flash Disk with MLC NAND and M-Systems? x2 Technology

    (c) 2009 Tim Schuerewegen

*/

#ifndef __DOCG3_H__
#define __DOCG3_H__

#define MCFG_DISKONCHIP_G3_ADD(_tag, _size) \
	MCFG_DEVICE_ADD(_tag, DISKONCHIP_G3, 0) \
	static_cast<diskonchip_g3_device *>(device)->set_size(_size);

// ======================> diskonchip_g3_device

class diskonchip_g3_device : public device_t,
								public device_nvram_interface
{
public:
	// construction/destruction
	diskonchip_g3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_size(int _size) { m_size = _size; }
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;
public:
	DECLARE_READ16_MEMBER( sec_1_r );
	DECLARE_WRITE16_MEMBER( sec_1_w );
	DECLARE_READ16_MEMBER( sec_2_r );
	DECLARE_WRITE16_MEMBER( sec_2_w );
	DECLARE_READ16_MEMBER( sec_3_r );
	DECLARE_WRITE16_MEMBER( sec_3_w );
private:
	UINT32 g3_offset_data_1();
	UINT32 g3_offset_data_2();
	UINT32 g3_offset_data_3();
	UINT8 g3_read_data();
	void g3_write_data(UINT8 data);
	UINT16 sec_2_read_1000();
	UINT16 sec_2_read_1074();
	UINT8 sec_2_read_1042();
	UINT8 sec_2_read_1046();
	UINT8 sec_2_read_1048();
	UINT8 sec_2_read_1049();
	UINT8 sec_2_read_104A();
	UINT8 sec_2_read_104B();
	UINT8 sec_2_read_104C();
	UINT8 sec_2_read_104D();
	UINT8 sec_2_read_104E();
	UINT8 sec_2_read_104F();
	UINT8 sec_2_read_100E();
	UINT8 sec_2_read_1014();
	UINT8 sec_2_read_1022();
	UINT8 sec_2_read_1038();
	UINT16 sec_2_read16(UINT32 offset);
	UINT8 sec_2_read8(UINT32 offset);
	void sec_2_write_100C(UINT8 data);
	void sec_2_write_1032(UINT8 data);
	void g3_erase_block();
	void sec_2_write_1034(UINT8 data);
	void sec_2_write_1036(UINT8 data);
	void sec_2_write_1040(UINT16 data);
	void sec_2_write_100A(UINT8 data);
	void sec_2_write16(UINT32 offset, UINT16 data);
	void sec_2_write8(UINT32 offset, UINT8 data);

	int m_size;

	UINT32 m_planes;
	UINT32 m_blocks;
	UINT32 m_pages;
	UINT32 m_user_data_size;
	UINT32 m_extra_area_size;
	UINT8 *m_data[3];
	UINT32 m_data_size[3];
	UINT8  m_sec_2[0x800];
	UINT32 m_data_1036;
	UINT32 m_data_1036_count;
	UINT32 m_transfer_offset;
	UINT8  m_device;
	UINT32 m_block;
	UINT32 m_page;
	UINT32 m_plane;
	UINT32 m_transfersize;
	UINT8  m_test;
	UINT32 m_address_count;
};

// device type definition
extern const device_type DISKONCHIP_G3;

#endif /* __DOCG3_H__ */
