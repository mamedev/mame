// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*****************************************************************************

    74543: Octal Registered Transceiver
    (typically 74F543)

***********************************************************************

    Connection Diagram:
              _________
    LEBA*  1 |   | |   | 24  Vcc
    OEBA*  2 |   ---   | 23  CEBA*
       A0  3 |         | 22  B0
       A1  4 |         | 21  B1
       A2  5 |         | 20  B2
       A3  6 |         | 19  B3
       A4  7 |         | 18  B4
       A5  8 |         | 17  B5
       A6  9 |         | 16  B6
       A7 10 |         | 15  B7
    CEAB* 11 |         | 14  LEAB*
      GND 12 |_________| 13  OEAB*


    Logic Symbol:
               |      |
           ____|______|____
          |                |
          |   A0 ...  A7   |
          |                |
       --O| OEAB           |
       --O| OEBA           |
       --O| CEAB           |
       --O| CEBA           |
       --O| LEAB           |
       --O| LEBA           |
          |                |
          |   B0 ...  B7   |
          |________________|
               |      |
               |      |


***********************************************************************

    Data I/O control table:

       Inputs            Latch      Output
  CEAB* LEAB* OEAB*      Status     Buffers
  -----------------------------------------
   H     X     X         Latched    High Z     A->B flow shown
   X     H     X         Latched      -        B->A flow control is the same
   L     L     X       Transparent    -        except using CEBA*, LEBA*, OEBA*
   X     X     H            -       High Z
   L     X     L            -       Driving

   X = immaterial
**********************************************************************/

#ifndef MAME_MACHINE_74543_H
#define MAME_MACHINE_74543_H

#pragma once

class ttl74543_device : public device_t
{
public:
	ttl74543_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	auto outputa_cb() { return m_output_a.bind(); }
	auto outputb_cb() { return m_output_b.bind(); }

	// public interfaces
	void ceab_w(int state);
	void leab_w(int state);
	void oeab_w(int state);
	void ceba_w(int state);
	void leba_w(int state);
	void oeba_w(int state);

	void a_w(uint8_t a);
	void b_w(uint8_t a);
	void outputa_rz(uint8_t& value);
	void outputb_rz(uint8_t& value);

	// Preset values
	// for lines tied to H or L
	void set_ceab_pin_value(int value) { m_ceabpre = (value==0); }
	void set_leab_pin_value(int value) { m_leabpre = (value==0); }
	void set_oeab_pin_value(int value) { m_oeabpre = (value==0); }
	void set_ceba_pin_value(int value) { m_cebapre = (value==0); }
	void set_leba_pin_value(int value) { m_lebapre = (value==0); }
	void set_oeba_pin_value(int value) { m_oebapre = (value==0); }

protected:
	// device-level overrides
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	devcb_write8 m_output_a;
	devcb_write8 m_output_b;

	bool m_ceab;
	bool m_leab;
	bool m_oeab;
	bool m_ceba;
	bool m_leba;
	bool m_oeba;

	bool m_ceabpre;
	bool m_leabpre;
	bool m_oeabpre;
	bool m_cebapre;
	bool m_lebapre;
	bool m_oebapre;

	uint8_t m_latch;
};

// device type definition
DECLARE_DEVICE_TYPE(TTL74543, ttl74543_device)

#endif // MAME_MACHINE_74161_H
