// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
Intel imm6-76 PROM programmer

Simple programmer for 1602/1702 and 1602A/1702A 256x8 static PMOS
UVEPROMs (1602/1602A have a metal lid preventing erasure but are
otherwise identical to 1702/1702A). Used in the INTELLEC® 4 and
INTELLEC® 8 development systems.

P1 universal edge connector (only used for power)

                  1    2
           GND    3    4  GND
                  5    6
                  7    8
                  9   10
                 11   12
                 13   14
                 15   16
                 17   18
                 19   20
                 21   22
                 23   24
                 25   26
                 27   28
                 29   30
                 31   32
                 33   34
                 35   36
                 37   38
                 39   40
                 41   42
          -10V   43   44  -10V
                 45   46
                 47   48
                 49   50
                 51   52
                 53   54
                 55   56
                 57   58
                 59   60
                 61   62
                 63   64
                 65   66
                 67   68
                 69   70
                 71   72
                 73   74
                 75   76
                 77   78
                 79   80
                 81   82
                 83   84
                 85   86
                 87   88
                 89   90
                 91   92
                 93   94
                 95   96
                 97   98
           +5V   99  100  +5V


J1 40-pin IDC connector (data/control)

           DI1    1    2  A0
           DI2    3    4  A1
           DI3    5    6  A2
           DI4    7    8  A3
           DI5    9   10  A4
           DI6   11   12  A5
           DI7   13   14  A6
           DI8   15   16  A7
           DO1   17   18
           DO2   19   20
           DO3   21   22
           DO4   23   24  DATA OUT ENABLE
           DO5   25   26  DATA IN + TRUE
           DO6   27   28  /DATA OUT + TRUE
           DO7   29   30  R/W (1702)
           DO8   31   32  R/W A (1702A)
           GND   33   34  GND
           GND   35   36  GND
           GND   37   38  GND
           GND   39   40  GND


J2 40-pin IDC connector (to PROM socket)

            D1    1    2  A0
            D2    3    4  A1
            D3    5    6  A2
            D4    7    8  A3
            D5    9   10  A4
            D6   11   11  A5
            D7   13   12  A6
            D8   15   16  A7
           GND   17   18  GND
          Vccs   19   20  Vccs
           Vdd   21   22  Vdd
           Vgg   23   24  /CS
           Vbb   25   26  PRGM
                 27   28
                 29   30
                 31   32
           GND   33   34  GND
                 35   36
                 37   38
                 39   40


J3 Six-pin connector on flying leads

        50V AC    1    2
        50V AC    3    4  +80V DC
/PRGM PROM PWR    5    6  GND


DO are open-collector TTL outputs with 5.6kΩ pullups.

DATA OUT ENABLE has an onboard 5.6kΩ pullup. Driving it low disables DO
outputs.

DATA IN + TRUE (if jumpered in) has a 5.6kΩ pullup and is XORed with the
DI inputs.  It should be pulled low in systems that use negative logic.
It is not jumpered in when used with INTELLEC 4 or INTELLEC 8 systems.

/DATA OUT + TRUE (if jumpered in) has a 5.6kΩ and is XORed with the PROM
data outputs.  It should be pulled low in systems that use positive
logic.  It is not jumpered in when used with INTELLEC 4 or INTELLEC 8
systems, the line is tied low with an onboard jumper.

/PRGM PROM PWR has a 27kΩ pullup to +80V DC and must be pulled low (to
GND) for programming voltage to be applied to the PROM.

When pulled low, R/W initiates a 2% programming duty cycle for writing
to a 1602 or 1702 PROM (150 millisecond cycle time).

When pulled low, RW A initiates a 20% programming duty cycle for writing
to a 1602A or 1702A PROM (15 millisecond cycle time).
*/
#ifndef MAME_INTEL_IMM6_76_H
#define MAME_INTEL_IMM6_76_H

#pragma once


class intel_imm6_76_device : public device_t, public device_image_interface
{
public:
	intel_imm6_76_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	// device_image_interface static info
	virtual bool        is_readable()                           const noexcept override { return true; }
	virtual bool        is_writeable()                          const noexcept override { return true; }
	virtual bool        is_creatable()                          const noexcept override { return true; }
	virtual bool        is_reset_on_load()                      const noexcept override { return false; }
	virtual bool        support_command_line_image_creation()   const noexcept override { return true; }
	virtual char const *file_extensions()                       const noexcept override { return "rom,bin"; }
	virtual char const *image_type_name()                       const noexcept override { return "promimage"; }
	virtual char const *image_brief_type_name()                 const noexcept override { return "prom"; }

	void di_w(u8 data);
	void a_w(u8 data);
	u8 do_r() const;
	void data_out_enable(int state);    // 1 = asserted
	void data_in_positive(int state);   // 1 = asserted
	void data_out_positive(int state);  // 0 = asserted
	void r_w(int state);                // 1 = read, 0 = write
	void r_w_a(int state);              // 1 = read, 0 = write
	void prgm_prom_pwr(int state);      // 0 = asserted

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(cycle_expired);
	TIMER_CALLBACK_MEMBER(cycle_a_expired);
	TIMER_CALLBACK_MEMBER(prg_expired);

	void trigger_prg();

	emu_timer   *m_cycle_tmr, *m_cycle_a_tmr, *m_prg_tmr;

	u8      m_data[256];
	u8      m_di, m_a;
	bool    m_do_enable, m_di_pos, m_do_pos;
	bool    m_r_w, m_r_w_a;
	bool    m_prgm_pwr;
	bool    m_cycle, m_cycle_a, m_prg;
};

DECLARE_DEVICE_TYPE(INTEL_IMM6_76, intel_imm6_76_device)

#endif // MAME_INTEL_IMM6_76_H
