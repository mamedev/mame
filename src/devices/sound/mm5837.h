// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    National Semiconductor MM5837

    Digital Noise Source

               ___ ___
       VDD  1 |*  u   | 5  N/C
       VGG  2 |       | 6  N/C
    OUTPUT  3 |       | 7  N/C
       VSS  4 |_______| 8  N/C

***************************************************************************/

#ifndef MAME_SOUND_MM5837_H
#define MAME_SOUND_MM5837_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MM5837_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MM5837, 0)

#define MCFG_MM5837_VDD(_voltage) \
	downcast<mm5837_device &>(*device).set_vdd_voltage(_voltage);

#define MCFG_MM5837_OUTPUT_CB(_devcb) \
	devcb = &downcast<mm5837_device &>(*device).set_output_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mm5837_device : public device_t
{
public:
	// construction/destruction
	mm5837_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_vdd_voltage(int voltage) { m_vdd = voltage; }
	template <class Object> devcb_base &set_output_callback(Object &&cb) { return m_output_cb.set_callback(std::forward<Object>(cb)); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// taken from the book 'mims circuit scrapbook, vol. 1'
	// this is the frequency the chip runs at when given a vdd voltage of -0 to -15
	static constexpr int m_frequency[16] = { 0, 0, 0, 0, 0, 0, 1, 2267, 8731, 16382, 23531, 32564, 38347, 40010, 37800, 33173 };

	// callbacks
	devcb_write_line m_output_cb;

	// voltage (as positive number)
	int m_vdd;

	// output timer
	emu_timer *m_timer;

	// state
	uint32_t m_shift;
};

// device type definition
DECLARE_DEVICE_TYPE(MM5837, mm5837_device)

#endif // MAME_SOUND_MM5837_H
