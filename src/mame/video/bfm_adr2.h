// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef INC_BFMADDER2
#define INC_BFMADDER2

class bfm_adder2_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	bfm_adder2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void get_tile0_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile1_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void adder2_vbl(device_t &device);
	uint8_t screen_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void screen_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t normal_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void normal_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adder2_rom_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adder2_c001_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adder2_screen_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t adder2_vbl_ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void adder2_vbl_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t adder2_uart_ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void adder2_uart_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t adder2_uart_rx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void adder2_uart_tx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t adder2_irq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void vid_uart_tx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vid_uart_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vid_uart_rx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vid_uart_ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void adder2_decode_char_roms();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
private:

	int m_adder2_screen_page_reg;        // access/display select
	int m_adder2_c101;
	int m_adder2_rx;
	int m_adder_vbl_triggered;           // flag <>0, VBL IRQ triggered
	int m_adder2_acia_triggered;         // flag <>0, ACIA receive IRQ

	uint8_t m_adder_ram[0xE80];              // normal RAM
	uint8_t m_adder_screen_ram[2][0x1180];   // paged  display RAM

	tilemap_t *m_tilemap0;  // tilemap screen0
	tilemap_t *m_tilemap1;  // timemap screen1

	uint8_t m_adder2_data_from_sc2;
	uint8_t m_adder2_data_to_sc2;

	uint8_t m_adder2_data;
	uint8_t m_adder2_sc2data;

	optional_device<cpu_device> m_cpu;
};

// device type definition
extern const device_type BFM_ADDER2;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BFM_ADDER2_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BFM_ADDER2, 0)

#endif
