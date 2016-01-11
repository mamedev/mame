// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp_taco.h

    HP TApe COntroller (5006-3012)

*********************************************************************/

#ifndef __HP_TACO_H__
#define __HP_TACO_H__

#define MCFG_TACO_IRQ_HANDLER(_devcb) \
        devcb = &hp_taco_device::set_irq_handler(*device , DEVCB_##_devcb);

#define MCFG_TACO_FLG_HANDLER(_devcb) \
        devcb = &hp_taco_device::set_flg_handler(*device , DEVCB_##_devcb);

#define MCFG_TACO_STS_HANDLER(_devcb) \
        devcb = &hp_taco_device::set_sts_handler(*device , DEVCB_##_devcb);

class hp_taco_device : public device_t
{
public:
	// construction/destruction
	hp_taco_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);
	hp_taco_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

        // static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<hp_taco_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_flg_handler(device_t &device, _Object object) { return downcast<hp_taco_device &>(device).m_flg_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_sts_handler(device_t &device, _Object object) { return downcast<hp_taco_device &>(device).m_sts_handler.set_callback(object); }
        
        // Register read/write
        DECLARE_WRITE16_MEMBER(reg_w);
        DECLARE_READ16_MEMBER(reg_r);

        // Flag & status read
        DECLARE_READ_LINE_MEMBER(flg_r);
        DECLARE_READ_LINE_MEMBER(sts_r);

        typedef UINT32 tape_pos_t;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
        
private:
        devcb_write_line m_irq_handler;
        devcb_write_line m_flg_handler;
        devcb_write_line m_sts_handler;

        // Registers
        UINT16 m_data_reg;
        UINT16 m_cmd_reg;
        UINT16 m_status_reg;
        UINT16 m_tach_reg;
        UINT16 m_checksum_reg;
        UINT16 m_timing_reg;

        // State
        bool m_irq;
        bool m_flg;
        bool m_sts;

        // Tape position
        tape_pos_t m_tape_pos;
        attotime m_start_time;

        // Timers
        emu_timer *m_tape_timer;

        void irq_w(bool state);
        void set_error(bool state);
        bool check_for_errors(void);
        static unsigned speed_to_tick_freq(bool fast);
        void update_tape_pos(void);
        static bool any_hole(UINT32 tape_pos_a , UINT32 tape_pos_b);
        UINT32 next_hole(bool fwd) const;
        static attotime time_to_distance(UINT32 distance, bool fast);
        attotime time_to_target(UINT32 target, bool fast) const;
        void start_tape(void);
        void stop_tape(void);
        void start_cmd_exec(UINT16 new_cmd_reg);
};

// device type definition
extern const device_type HP_TACO;

#endif /* __HP_TACO_H__ */
