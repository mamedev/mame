// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_BOOTLEG_H
#define MAME_BUS_NES_BOOTLEG_H

#include "nxrom.h"


// ======================> nes_mbaby_device

class nes_mbaby_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_mbaby_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	nes_mbaby_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

	u16 m_irq_count;
	int m_irq_enable;
	u8 m_latch;

private:
};


// ======================> nes_asn_device

class nes_asn_device : public nes_mbaby_device
{
public:
	// construction/destruction
	nes_asn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
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
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	u16 m_irq_count;
	int m_irq_enable;

};


// ======================> nes_btl_cj_device

class nes_btl_cj_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_btl_cj_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_btl_dd_device

class nes_btl_dd_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_btl_dd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	void update_banks();

	u8 m_prg_reg[4];
	u8 m_chr_reg[8];
	u8 m_mirror[4];
	u8 m_irq_count;
	int m_irq_enable;

};


// ======================> nes_btl_dn_device

class nes_btl_dn_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_btl_dn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ prescaler is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	u8 m_clock_mode;
	u8 m_pending;
	u8 m_counter1;
	u8 m_counter2;
	u8 m_prescaler;
};


// ======================> nes_smb2j_device

class nes_smb2j_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_smb2j_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_ex(offs_t offset, u8 data) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	void update_irq(u8 data);
	void write_45(offs_t offset, u8 data);
	u16 m_irq_count;
	int m_irq_enable;

};


// ======================> nes_smb2ja_device

class nes_smb2ja_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_smb2ja_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	u16 m_irq_count;
	int m_irq_enable;

};


// ======================> nes_smb2jb_device

class nes_smb2jb_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_smb2jb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_ex(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	nes_smb2jb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bank67);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

	u16 m_irq_count;
	int m_irq_enable;
	u8 m_reg;

	virtual void write_45(offs_t offset, u8 data);

private:
	const u8 m_bank67;

};


// ======================> nes_n32_4in1_device

class nes_n32_4in1_device : public nes_smb2jb_device
{
public:
	// construction/destruction
	nes_n32_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
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
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg;
};


// ======================> nes_09034a_device

class nes_09034a_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_09034a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_ex(offs_t offset, u8 data) override;
	virtual u8 read_ex(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	u16 m_irq_count;
	int m_irq_enable;
	u8 m_reg;

};


// ======================> nes_l001_device

class nes_l001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_l001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	u16 m_irq_count;

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
	nes_batmanfs_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool sub1);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	const bool m_sub1;     // submapper 1: RoboCop
	u16 m_irq_count;
	int m_irq_enable;

};

// ======================> nes_robocopfs_device

class nes_robocopfs_device : public nes_batmanfs_device
{
public:
	// construction/destruction
	nes_robocopfs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
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
	virtual void device_start() override ATTR_COLD;

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
	virtual void device_start() override ATTR_COLD;

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
	virtual void device_start() override ATTR_COLD;

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
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_latch;
};


// ======================> nes_lh42_device

class nes_lh42_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lh42_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
};


// ======================> nes_lg25_device

class nes_lg25_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lg25_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
};


// ======================> nes_lh10_device

class nes_lh10_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lh10_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
};


// ======================> nes_lh51_device

class nes_lh51_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lh51_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
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
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	uint16_t m_irq_count;
	int m_irq_enable;
	uint8_t m_reg;
	uint8_t m_wram[0x2000];     // the board's own fixed 8K of PRG-RAM at $B800-$D7FF, whatever the header says

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
	virtual void device_start() override ATTR_COLD;

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
	virtual void device_start() override ATTR_COLD;

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
};


// ======================> nes_rt01_device

class nes_rt01_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_rt01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;

	virtual void pcb_reset() override;
};


// ======================> nes_yung08_device

class nes_yung08_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_yung08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_ex(offs_t offset, u8 data) override;
	virtual void write_l(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	void write_45(offs_t offset, u8 data);
	u16 m_irq_count;
	int m_irq_latch;

};



// ======================> nes_60106416l_device

class nes_60106416l_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_60106416l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();

	u8 m_reg;
};


// ======================> nes_ctc15_device

class nes_ctc15_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ctc15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// the IRQ counter counts falling edges of M2
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

private:
	u8 m_latch;
	u32 m_irq_count;
};


// ======================> nes_bitcorp4602_device

class nes_bitcorp4602_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bitcorp4602_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_ex(offs_t offset, u8 data) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// M2 at the cartridge connector: the IRQ counter counts CPU cycles
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

private:
	void write_regs(offs_t addr, u8 data);

	required_ioport m_dsw;
	u8 m_outer;             // DIP switches: the outer 128 KiB bank
	u8 m_sel2;              // $4120 bit 0
	u16 m_irq_count;
	bool m_irq_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_MARIOBABY,      nes_mbaby_device)
DECLARE_DEVICE_TYPE(NES_ASN,            nes_asn_device)
DECLARE_DEVICE_TYPE(NES_SMB3PIRATE,     nes_smb3p_device)
DECLARE_DEVICE_TYPE(NES_BTL_CONTRAJ,    nes_btl_cj_device)
DECLARE_DEVICE_TYPE(NES_BTL_DDRAGON,    nes_btl_dd_device)
DECLARE_DEVICE_TYPE(NES_BTL_DNINJA,     nes_btl_dn_device)
DECLARE_DEVICE_TYPE(NES_SMB2J,          nes_smb2j_device)
DECLARE_DEVICE_TYPE(NES_SMB2JA,         nes_smb2ja_device)
DECLARE_DEVICE_TYPE(NES_SMB2JB,         nes_smb2jb_device)
DECLARE_DEVICE_TYPE(NES_N32_4IN1,       nes_n32_4in1_device)
DECLARE_DEVICE_TYPE(NES_0353,           nes_0353_device)
DECLARE_DEVICE_TYPE(NES_09034A,         nes_09034a_device)
DECLARE_DEVICE_TYPE(NES_L001,           nes_l001_device)
DECLARE_DEVICE_TYPE(NES_BATMANFS,       nes_batmanfs_device)
DECLARE_DEVICE_TYPE(NES_ROBOCOPFS,      nes_robocopfs_device)
DECLARE_DEVICE_TYPE(NES_PALTHENA,       nes_palthena_device)
DECLARE_DEVICE_TYPE(NES_TOBIDASE,       nes_tobidase_device)
DECLARE_DEVICE_TYPE(NES_DH08,           nes_dh08_device)
DECLARE_DEVICE_TYPE(NES_LE05,           nes_le05_device)
DECLARE_DEVICE_TYPE(NES_LG25,           nes_lg25_device)
DECLARE_DEVICE_TYPE(NES_LH10,           nes_lh10_device)
DECLARE_DEVICE_TYPE(NES_LH28_LH54,      nes_lh28_lh54_device)
DECLARE_DEVICE_TYPE(NES_LH31,           nes_lh31_device)
DECLARE_DEVICE_TYPE(NES_LH32,           nes_lh32_device)
DECLARE_DEVICE_TYPE(NES_LH42,           nes_lh42_device)
DECLARE_DEVICE_TYPE(NES_LH51,           nes_lh51_device)
DECLARE_DEVICE_TYPE(NES_LH53,           nes_lh53_device)
DECLARE_DEVICE_TYPE(NES_2708,           nes_2708_device)
DECLARE_DEVICE_TYPE(NES_AC08,           nes_ac08_device)
DECLARE_DEVICE_TYPE(NES_MMALEE,         nes_mmalee_device)
DECLARE_DEVICE_TYPE(NES_RT01,           nes_rt01_device)
DECLARE_DEVICE_TYPE(NES_YUNG08,         nes_yung08_device)
DECLARE_DEVICE_TYPE(NES_60106416L,      nes_60106416l_device)
DECLARE_DEVICE_TYPE(NES_CTC15,          nes_ctc15_device)
DECLARE_DEVICE_TYPE(NES_BITCORP4602,    nes_bitcorp4602_device)

#endif // MAME_BUS_NES_BOOTLEG_H
