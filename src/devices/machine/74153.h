// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*****************************************************************************

  74153 Dual 4-line to 1-line data selectors/multiplexers


  Pin layout and functions to access pins:

  enable_w(0)        [1] /1G   VCC [16]
  b_w                [2] B     /2G [15]  enable_w(1)
  input_line_w(0,3)  [3] 1C3     A [14]  a_w
  input_line_w(0,2)  [4] 1C2   2C3 [13]  input_line_w(1,3)
  input_line_w(0,1)  [5] 1C1   2C2 [12]  input_line_w(1,2)
  input_line_w(0,0)  [6] 1C0   2C1 [11]  input_line_w(1,1)
  output_r(0)        [7] 1Y    2C0 [10]  input_line_w(1,0)
                     [8] GND    2Y [9]   output_r(1)


  Truth table (all logic levels indicate the actual voltage on the line):

            INPUTS         | OUTPUT
                           |
    G | B  A | C0 C1 C2 C3 | Y
    --+------+-------------+---
    H | X  X | X  X  X  X  | L
    L | L  L | X  X  X  X  | C0
    L | L  H | X  X  X  X  | C1
    L | H  L | X  X  X  X  | C2
    L | H  H | X  X  X  X  | C3
    --+------+-------------+---
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#ifndef TTL74153_H
#define TTL74153_H


typedef device_delegate<void (void)> ttl74153_output_delegate;

#define TTL74153_OUTPUT_CB(_name) void _name(void)


class ttl74153_device : public device_t
{
public:
	ttl74153_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~ttl74153_device() {}

	static void set_output_callback(device_t &device, ttl74153_output_delegate callback) { downcast<ttl74153_device &>(device).m_output_cb = callback; }

	/* must call update() after setting the inputs */
	void update();

	void a_w(int data);
	void b_w(int data);
	void input_line_w(int section, int input_line, int data);
	void enable_w(int section, int data);
	int output_r(int section);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	ttl74153_output_delegate m_output_cb;

	/* inputs */
	int m_a;                  /* pin 14 */
	int m_b;                  /* pin 2 */
	int m_input_lines[2][4];  /* pins 3-6,10-13 */
	int m_enable[2];          /* pins 1,15 */

	/* output */
	int m_output[2];          /* pins 7,9 */

	/* internals */
	int m_last_output[2];
};

extern const device_type TTL74153;


#define MCFG_74153_OUTPUT_CB(_class, _method) \
	ttl74153_device::set_output_callback(*device, ttl74153_output_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#endif
