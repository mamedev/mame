// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_BOOTLEG_H
#define MAME_BUS_NES_BOOTLEG_H

#include "nxrom.h"


// ======================> nes_ax5705_device

class nes_ax5705_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ax5705_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void set_prg();
	uint8_t m_mmc_prg_bank[2];
	uint8_t m_mmc_vrom_bank[8];
};


// ======================> nes_sc127_device

class nes_sc127_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sc127_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint16_t m_irq_count;
	int m_irq_enable;
};


// ======================> nes_mbaby_device

class nes_mbaby_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_mbaby_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	static constexpr device_timer_id TIMER_IRQ = 0;

	uint8_t m_latch;
	int m_irq_enable;

	emu_timer *irq_timer;
	attotime timer_freq;
};


// ======================> nes_asn_device

class nes_asn_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_asn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_smb3p_device

class nes_smb3p_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_smb3p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
	virtual void pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	u16 m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_btl_dn_device

class nes_btl_dn_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_btl_dn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint16_t m_irq_count;
};


// ======================> nes_smb2j_device

class nes_smb2j_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_smb2j_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_ex(offs_t offset, uint8_t data) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint16_t m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_smb2ja_device

class nes_smb2ja_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_smb2ja_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint16_t m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_smb2jb_device

class nes_smb2jb_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_smb2jb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_ex(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint16_t m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_0353_device

class nes_0353_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_0353_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	u8 m_reg;
};


// ======================> nes_09034a_device

class nes_09034a_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_09034a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_ex(offs_t offset, uint8_t data) override;
	virtual uint8_t read_m(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg;
};


// ======================> nes_batmanfs_device

class nes_batmanfs_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_batmanfs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	u16 m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_palthena_device

class nes_palthena_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_palthena_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	u8 m_reg;
};


// ======================> nes_tobidase_device

class nes_tobidase_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tobidase_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_whirlwind_device

class nes_whirlwind_device : public nes_nrom_device
{
public:
	virtual u8 read_m(offs_t offset) override;
	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_whirlwind_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

	u8 m_reg;
};


// ======================> nes_dh08_device

class nes_dh08_device : public nes_whirlwind_device
{
public:
	// construction/destruction
	nes_dh08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_le05_device

class nes_le05_device : public nes_whirlwind_device
{
public:
	// construction/destruction
	nes_le05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_lh28_lh54_device

class nes_lh28_lh54_device : public nes_whirlwind_device
{
public:
	// construction/destruction
	nes_lh28_lh54_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_lh31_device

class nes_lh31_device : public nes_whirlwind_device
{
public:
	// construction/destruction
	nes_lh31_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_lh32_device

class nes_lh32_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lh32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_lh10_device

class nes_lh10_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lh10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_prg();
	uint8_t m_latch;
	uint8_t m_reg[8];
};


// ======================> nes_lh51_device

class nes_lh51_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lh51_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_lh53_device

class nes_lh53_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lh53_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override {}
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint16_t m_irq_count;
	int m_irq_enable;
	uint8_t m_reg;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
	attotime timer_freq;
};


// ======================> nes_2708_device

class nes_2708_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_2708_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[2];
};


// ======================> nes_ac08_device

class nes_ac08_device : public nes_nrom_device
{
public:
	// nes_ac08_device/destruction
	nes_ac08_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_ex(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_mmalee_device

class nes_mmalee_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_mmalee_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_shuiguan_device

class nes_shuiguan_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_shuiguan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint16_t m_irq_count;
	int m_irq_enable;
	uint8_t m_mmc_vrom_bank[8];

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_rt01_device

class nes_rt01_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_rt01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};



// device type definition
DECLARE_DEVICE_TYPE(NES_AX5705,         nes_ax5705_device)
DECLARE_DEVICE_TYPE(NES_SC127,          nes_sc127_device)
DECLARE_DEVICE_TYPE(NES_MARIOBABY,      nes_mbaby_device)
DECLARE_DEVICE_TYPE(NES_ASN,            nes_asn_device)
DECLARE_DEVICE_TYPE(NES_SMB3PIRATE,     nes_smb3p_device)
DECLARE_DEVICE_TYPE(NES_BTL_DNINJA,     nes_btl_dn_device)
DECLARE_DEVICE_TYPE(NES_SMB2J,          nes_smb2j_device)
DECLARE_DEVICE_TYPE(NES_SMB2JA,         nes_smb2ja_device)
DECLARE_DEVICE_TYPE(NES_SMB2JB,         nes_smb2jb_device)
DECLARE_DEVICE_TYPE(NES_0353,           nes_0353_device)
DECLARE_DEVICE_TYPE(NES_09034A,         nes_09034a_device)
DECLARE_DEVICE_TYPE(NES_BATMANFS,       nes_batmanfs_device)
DECLARE_DEVICE_TYPE(NES_PALTHENA,       nes_palthena_device)
DECLARE_DEVICE_TYPE(NES_TOBIDASE,       nes_tobidase_device)
DECLARE_DEVICE_TYPE(NES_DH08,           nes_dh08_device)
DECLARE_DEVICE_TYPE(NES_LE05,           nes_le05_device)
DECLARE_DEVICE_TYPE(NES_LH10,           nes_lh10_device)
DECLARE_DEVICE_TYPE(NES_LH28_LH54,      nes_lh28_lh54_device)
DECLARE_DEVICE_TYPE(NES_LH31,           nes_lh31_device)
DECLARE_DEVICE_TYPE(NES_LH32,           nes_lh32_device)
DECLARE_DEVICE_TYPE(NES_LH51,           nes_lh51_device)
DECLARE_DEVICE_TYPE(NES_LH53,           nes_lh53_device)
DECLARE_DEVICE_TYPE(NES_2708,           nes_2708_device)
DECLARE_DEVICE_TYPE(NES_AC08,           nes_ac08_device)
DECLARE_DEVICE_TYPE(NES_MMALEE,         nes_mmalee_device)
DECLARE_DEVICE_TYPE(NES_SHUIGUAN,       nes_shuiguan_device)
DECLARE_DEVICE_TYPE(NES_RT01,           nes_rt01_device)

#endif // MAME_BUS_NES_BOOTLEG_H
