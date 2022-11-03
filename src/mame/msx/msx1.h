// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX1_H
#define MAME_MSX_MSX1_H

#include "msx.h"

class msx1_state : public msx_state
{
public:
	msx1_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx_state(mconfig, type, tag)
	{
	}

	void ax150(machine_config &config);
	void ax170(machine_config &config);
	void ax230(machine_config &config);
	void canonv8(machine_config &config);
	void canonv10(machine_config &config);
	void canonv20(machine_config &config);
	void canonv20e(machine_config &config);
	void canonv25(machine_config &config);
	void cf1200(machine_config &config);
	void cf2000(machine_config &config);
	void cf2700(machine_config &config);
	void cf2700g(machine_config &config);
	void cf2700uk(machine_config &config);
	void cf3000(machine_config &config);
	void cf3300(machine_config &config);
	void cpc50a(machine_config &config);
	void cpc50b(machine_config &config);
	void cpc51(machine_config &config);
	void cpc88(machine_config &config);
	void cx5f(machine_config &config);
	void cx5f1(machine_config &config);
	void cx5mu(machine_config &config);
	void dgnmsx(machine_config &config);
	void dpc100(machine_config &config);
	void dpc180(machine_config &config);
	void dpc200(machine_config &config);
	void dpc200e(machine_config &config);
	void expert10(machine_config &config);
	void expert11(machine_config &config);
	void expert13(machine_config &config);
	void expertdp(machine_config &config);
	void expertpl(machine_config &config);
	void fmx(machine_config &config);
	void fdpc200(machine_config &config);
	void fpc500(machine_config &config);
	void fs1300(machine_config &config);
	void fs4000(machine_config &config);
	void fs4000a(machine_config &config);
	void fspc800(machine_config &config);
	void gfc1080(machine_config &config);
	void gfc1080a(machine_config &config);
	void gsfc80u(machine_config &config);
	void gsfc200(machine_config &config);
	void hb10(machine_config &config);
	void hb10p(machine_config &config);
	void hb20p(machine_config &config);
	void hb55(machine_config &config);
	void hb55d(machine_config &config);
	void hb55p(machine_config &config);
	void hb75(machine_config &config);
	void hb75d(machine_config &config);
	void hb75p(machine_config &config);
	void hb101(machine_config &config);
	void hb101p(machine_config &config);
	void hb201(machine_config &config);
	void hb201p(machine_config &config);
	void hb501p(machine_config &config);
	void hb701fd(machine_config &config);
	void hb8000(machine_config &config);
	void hc5(machine_config &config);
	void hc6(machine_config &config);
	void hc7(machine_config &config);
	void hotbi13b(machine_config &config);
	void hotbi13p(machine_config &config);
	void hx10(machine_config &config);
	void hx10d(machine_config &config);
	void hx10dp(machine_config &config);
	void hx10e(machine_config &config);
	void hx10f(machine_config &config);
	void hx10s(machine_config &config);
	void hx10sa(machine_config &config);
	void hx20(machine_config &config);
	void hx20e(machine_config &config);
	void hx20i(machine_config &config);
	void hx21(machine_config &config);
	void hx21f(machine_config &config);
	void hx22(machine_config &config);
	void hx22i(machine_config &config);
	void hx32(machine_config &config);
	void hx51i(machine_config &config);
	void jvchc7gb(machine_config &config);
	void mbh1(machine_config &config);
	void mbh1e(machine_config &config);
	void mbh2(machine_config &config);
	void mbh25(machine_config &config);
	void mbh50(machine_config &config);
	void ml8000(machine_config &config);
	void mlf48(machine_config &config);
	void mlf80(machine_config &config);
	void mlf110(machine_config &config);
	void mlf120(machine_config &config);
	void mlfx1(machine_config &config);
	void mpc10(machine_config &config);
	void mpc64(machine_config &config);
	void mpc100(machine_config &config);
	void mpc200(machine_config &config);
	void mpc200sp(machine_config &config);
	void mx10(machine_config &config);
	void mx15(machine_config &config);
	void mx64(machine_config &config);
	void mx101(machine_config &config);
	void nms801(machine_config &config);
	void perfect1(machine_config &config);
	void phc2(machine_config &config);
	void phc28(machine_config &config);
	void phc28l(machine_config &config);
	void phc28s(machine_config &config);
	void piopx7(machine_config &config);
	void piopx7uk(machine_config &config);
	void piopxv60(machine_config &config);
	void pv7(machine_config &config);
	void pv16(machine_config &config);
	void spc800(machine_config &config);
	void svi728(machine_config &config);
	void sx100(machine_config &config);
	void tadpc200(machine_config &config);
	void vg8000(machine_config &config);
	void vg8010(machine_config &config);
	void vg8010f(machine_config &config);
	void vg802000(machine_config &config);
	void vg802020(machine_config &config);
	void vg8020f(machine_config &config);
	void yc64(machine_config &config);
	void yis303(machine_config &config);
	void yis503(machine_config &config);
	void yis503f(machine_config &config);
};

#endif // MAME_MSX_MSX1_H
