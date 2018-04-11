// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair


#ifndef MAME_MACHINE_SCC2698B_H
#define MAME_MACHINE_SCC2698B_H


class scc2698b_device : public device_t
{
public:
	scc2698b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	DECLARE_READ8_MEMBER(reg_r);
	DECLARE_WRITE8_MEMBER(reg_w);

	void write_reg(int offset, uint8_t data);
	uint8_t read_reg(int offset);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:

};

DECLARE_DEVICE_TYPE(SCC2698B, scc2698b_device)

#endif // MAME_MACHINE_SCC2698B_H
