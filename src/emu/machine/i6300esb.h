// Intel i6300ESB southbridge

#ifndef I6300ESB_H
#define I6300ESB_H

#include "pci.h"

#define MCFG_I6300ESB_LPC_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, I6300ESB_LPC, 0x808625a1, 0x02, 0x060100, 0x00000000)

#define MCFG_I6300ESB_WATCHDOG_ADD(_tag, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, I6300ESB_WATCHDOG, 0x808625ab, 0x02, 0x088000, _subdevice_id)

class i6300esb_lpc_device : public pci_device {
public:
	i6300esb_lpc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void reset_all_mappings();
	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
						   UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space);

	virtual DECLARE_ADDRESS_MAP(config_map, 32);


protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(internal_io_map, 32);

	UINT32 gpio_base, fwh_sel1;
	UINT16 gen1_dec, lpc_en;
	UINT8 gpio_cntl, lpc_if_com_range, lpc_if_fdd_lpt_range, lpc_if_sound_range, fwh_dec_en1, siu_config_port;
	int siu_config_state;

	DECLARE_WRITE8_MEMBER (nop_w);

	// configuration space registers
	DECLARE_READ32_MEMBER (gpio_base_r);            // 58
	DECLARE_WRITE32_MEMBER(gpio_base_w);
	DECLARE_READ8_MEMBER  (gpio_cntl_r);            // 5c
	DECLARE_WRITE8_MEMBER (gpio_cntl_w);

	DECLARE_READ8_MEMBER  (lpc_if_com_range_r);     // e0
	DECLARE_WRITE8_MEMBER (lpc_if_com_range_w);
	DECLARE_READ8_MEMBER  (lpc_if_fdd_lpt_range_r); // e1
	DECLARE_WRITE8_MEMBER (lpc_if_fdd_lpt_range_w);
	DECLARE_READ8_MEMBER  (lpc_if_sound_range_r);   // e2
	DECLARE_WRITE8_MEMBER (lpc_if_sound_range_w);
	DECLARE_READ8_MEMBER  (fwh_dec_en1_r);          // e3
	DECLARE_WRITE8_MEMBER (fwh_dec_en1_w);
	DECLARE_READ16_MEMBER (gen1_dec_r);             // e4
	DECLARE_WRITE16_MEMBER(gen1_dec_w);
	DECLARE_READ16_MEMBER (lpc_en_r);               // e6
	DECLARE_WRITE16_MEMBER(lpc_en_w);
	DECLARE_READ32_MEMBER (fwh_sel1_r);             // e8
	DECLARE_WRITE32_MEMBER(fwh_sel1_w);

	DECLARE_READ32_MEMBER (unk_fc_r);               // fc
	DECLARE_WRITE32_MEMBER(unk_fc_w);


	// i/o space registers
	DECLARE_READ8_MEMBER  (siu_config_port_r);      // 4e
	DECLARE_WRITE8_MEMBER (siu_config_port_w);
	DECLARE_READ8_MEMBER  (siu_data_port_r);        // 4f
	DECLARE_WRITE8_MEMBER (siu_data_port_w);

	void map_bios(address_space *memory_space, UINT32 start, UINT32 end, int idsel);
};

class i6300esb_watchdog_device : public pci_device {
public:
	i6300esb_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(map, 32);
};

extern const device_type I6300ESB_LPC;
extern const device_type I6300ESB_WATCHDOG;

#endif
