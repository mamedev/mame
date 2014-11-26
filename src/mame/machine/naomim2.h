#ifndef _NAOMIM2_H_
#define _NAOMIM2_H_

#include "naomibd.h"
#include "315-5881_crypt.h"

#define MCFG_NAOMI_M2_BOARD_ADD(_tag, _key_tag, _eeprom_tag, _actel_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_M2_BOARD, _eeprom_tag, _actel_tag, _irq_cb) \
	naomi_m2_board::static_set_tags(*device, _key_tag);

class naomi_m2_board : public naomi_board
{
public:
	naomi_m2_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	UINT32 rom_cur_address;
	static const int RAM_SIZE = 65536;
	UINT8* ram;

	static void static_set_tags(device_t &device, const char *_key_tag);
	UINT16 read_callback(UINT32 addr);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void board_setup_address(UINT32 address, bool is_dma);
	virtual void board_get_buffer(UINT8 *&base, UINT32 &limit);
	virtual void board_advance(UINT32 size);
	virtual void board_write(offs_t offset, UINT16 data);

private:


	const char *key_tag;


	required_device<sega_315_5881_crypt_device> m_cryptdevice;
};

extern const device_type NAOMI_M2_BOARD;

#endif
