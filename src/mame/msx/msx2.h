// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX2_H
#define MAME_MSX_MSX2_H

#include "msx.h"

class msx2_state : public msx2_base_state
{
public:
	msx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx2_base_state(mconfig, type, tag)
	{
	}

	void ax350(machine_config &config);
	void ax350ii(machine_config &config);
	void ax350iif(machine_config &config);
	void ax370(machine_config &config);
	void ax500(machine_config &config);
	void canonv25(machine_config &config);
	void canonv30(machine_config &config);
	void canonv30f(machine_config &config);
	void cpc300(machine_config &config);
	void cpc300e(machine_config &config);
	void cpc330k(machine_config &config);
	void cpc400(machine_config &config);
	void cpc400s(machine_config &config);
	void cpc61(machine_config &config);
	void cpg120(machine_config &config);
	void cx7128(machine_config &config);
	void cx7m128(machine_config &config);
	void expert20(machine_config &config);
	void expert3i(machine_config &config);
	void expert3t(machine_config &config);
	void expertac(machine_config &config);
	void expertdx(machine_config &config);
	void fpc900(machine_config &config);
	void kmc5000(machine_config &config);
	void mbh70(machine_config &config);
	void mlg1(machine_config &config);
	void mlg3(machine_config &config);
	void mlg10(machine_config &config);
	void mlg30(machine_config &config);
	void mlg30_2(machine_config &config);
	void mpc2300(machine_config &config);
	void mpc2500f(machine_config &config);
	void mpc25fd(machine_config &config);
	void mpc25fs(machine_config &config);
	void mpc27(machine_config &config);
	void fs4500(machine_config &config);
	void fs4600f(machine_config &config);
	void fs4700f(machine_config &config);
	void fs5000f2(machine_config &config);
	void fs5500f1(machine_config &config);
	void fs5500f2(machine_config &config);
	void fsa1(machine_config &config);
	void fsa1a(machine_config &config);
	void fsa1f(machine_config &config);
	void fsa1fm(machine_config &config);
	void fsa1fx(machine_config &config);
	void fsa1gt(machine_config &config);
	void fsa1st(machine_config &config);
	void fsa1mk2(machine_config &config);
	void fsa1wsx(machine_config &config);
	void fsa1wx(machine_config &config);
	void fsa1wxa(machine_config &config);
	void fstm1(machine_config &config);
	void hbf1(machine_config &config);
	void hbf1ii(machine_config &config);
	void hbf1xd(machine_config &config);
	void hbf1xdj(machine_config &config);
	void hbf1xv(machine_config &config);
	void hbf5(machine_config &config);
	void hbf500(machine_config &config);
	void hbf500_2(machine_config &config);
	void hbf500f(machine_config &config);
	void hbf500p(machine_config &config);
	void hbf700d(machine_config &config);
	void hbf700f(machine_config &config);
	void hbf700p(machine_config &config);
	void hbf700s(machine_config &config);
	void hbf900(machine_config &config);
	void hbf900a(machine_config &config);
	void hbf9p(machine_config &config);
	void hbf9pr(machine_config &config);
	void hbf9s(machine_config &config);
	void hbg900ap(machine_config &config);
	void hbg900p(machine_config &config);
	void hotbit20(machine_config &config);
	void hx23(machine_config &config);
	void hx23f(machine_config &config);
	void hx33(machine_config &config);
	void hx34(machine_config &config);
	void mbh3(machine_config &config);
	void nms8220(machine_config &config);
	void nms8245(machine_config &config);
	void nms8245f(machine_config &config);
	void nms8250(machine_config &config);
	void nms8255(machine_config &config);
	void nms8255f(machine_config &config);
	void nms8260(machine_config &config);
	void nms8280(machine_config &config);
	void nms8280f(machine_config &config);
	void nms8280g(machine_config &config);
	void phc23(machine_config &config);
	void phc23jb(machine_config &config);
	void phc35j(machine_config &config);
	void phc55fd2(machine_config &config);
	void phc70fd(machine_config &config);
	void phc70fd2(machine_config &config);
	void phc77(machine_config &config);
	void tpc310(machine_config &config);
	void tpp311(machine_config &config);
	void tps312(machine_config &config);
	void ucv102(machine_config &config);
	void vg8230(machine_config &config);
	void vg8235(machine_config &config);
	void vg8235f(machine_config &config);
	void vg8240(machine_config &config);
	void victhc80(machine_config &config);
	void victhc90(machine_config &config);
	void victhc95(machine_config &config);
	void victhc95a(machine_config &config);
	void y503iiir(machine_config &config);
	void y503iiire(machine_config &config);
	void yis604(machine_config &config);
	void y805128(machine_config &config);
	void y805128r2(machine_config &config);
	void y805128r2e(machine_config &config);
	void y805256(machine_config &config);
};

#endif // MAME_MSX_MSX2_H
