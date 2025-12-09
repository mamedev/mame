// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************

    uPD177x

******************************************************************************


               ___ ___
      PB4   1 |*  u   | 28  PB3
      PB5   2 |       | 27  PB2
      PB6   3 |       | 26  PB1
      PB7   4 |       | 25  PA8
      PB8   5 |   u   | 24  PA7
      /AC   6 |   P   | 23  PA6
   /PW ON   7 |   D   | 22  PA5
      Vdd   8 |   1   | 21  PA4
       X1   9 |   7   | 20  PA3
       X0  10 |   7   | 19  PA2
   SOUND2  11 |   1   | 18  PA1
      Vda  12 |       | 17  CH2
   SOUND1  13 |       | 16  /EXINT
      GND  14 |_______| 15  CH1

****************************************************************************/


#ifndef MAME_CPU_UPD177X_UPD177X_H
#define MAME_CPU_UPD177X_UPD177X_H

class upd177x_cpu_device : public cpu_device
						, public device_sound_interface
{
public:
	upd177x_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto pb_out_cb() { return m_pb_out_cb.bind(); }

	u8 pa_r() { return m_pa; }
	void pa_w(u8 data) { m_pa = data; }
	void pb_w(u8 data) { m_pb = data; }

protected:
	upd177x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
	virtual uint32_t execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual void execute_run() override;

	virtual space_config_vector memory_space_config() const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void sound_stream_update(sound_stream &stream) override;

	void program_map(address_map &map) ATTR_COLD;

private:
	void op0xxx(u16 opcode);
	void op1xxx(u16 opcode);
	void op2xxx(u16 opcode);
	void op3xxx(u16 opcode);
	void op7xxx(u16 opcode);
	void op89xxx(u16 opcode);
	void opabxxx(u16 opcode);
	void opcdxxx(u16 opcode);
	void opefxxx(u16 opcode);
	void handle_timer();
	void set_noise_counter_bit();
	void m_cycle();

	address_space_config m_program_config;

	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific m_program;
	devcb_write8 m_pb_out_cb;
	sound_stream *m_channel;

	int m_icount;

	u16 m_pc; // 16 bits
	u8 m_sp;  // 3 bits
	u8 m_a;   // 8 bits
	u8 m_a_save; // 8 bits
	u8 m_h;   // 6 bits
	u8 m_n;   // 8 bits
	u8 m_timer;
	u16 m_counter;
	u8 m_x;   // 7 bits
	u8 m_y;   // 5 bits
	u8 m_pa;
	u8 m_pb;
	u8 m_ram[0x40];
	u8 m_md0;
	u8 m_md1;
	u8 m_pnc1;  // 7 bits
	u8 m_pnc2;  // 3 bits
	bool m_skip;
	bool m_skip_save;
	bool m_ts;
	bool m_ns;
	bool m_ss;
	u8 m_dac;
	bool m_dac_sign;
	bool m_tn_int;
	u8 m_noise_counter_bit;
};


class upd1771c_cpu_device : public upd177x_cpu_device
{
public:
	upd1771c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(UPD1771C, upd1771c_cpu_device)

#endif // MAME_CPU_UPD177X_UPD177X_H
