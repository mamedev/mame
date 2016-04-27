#include "emu.h"
#include "h8_dma.h"

const device_type H8_DMA         = &device_creator<h8_dma_device>;
const device_type H8_DMA_CHANNEL = &device_creator<h8_dma_channel_device>;

h8_dma_device::h8_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_DMA, "H8 DMA controller", tag, owner, clock, "h8_dma", __FILE__),
	dmach0(*this, "0"),
	dmach1(*this, "1")
{
}

void h8_dma_device::device_start()
{
	dmach0->set_id(0<<1);
	dmach1->set_id(1<<1);
}

void h8_dma_device::device_reset()
{
	dmabcr = 0x0000;
	dmawer = 0x00;
}

bool h8_dma_device::trigger_dma(int vector)
{
	// Don't shortcut! Both dmas may be started
	bool start0 = dmach0->start_test(vector);
	bool start1 = dmach1->start_test(vector);

	return start0 || start1;
}

void h8_dma_device::count_done(int id)
{
	if(id & 2)
		dmach1->count_done(id & 1);
	else
		dmach0->count_done(id & 1);
}

void h8_dma_device::clear_dte(int id)
{
	dmabcr &= ~(0x0010 << id);
}

READ8_MEMBER(h8_dma_device::dmawer_r)
{
	logerror("dmawer_r %02x\n", dmawer);
	return dmawer;
}

WRITE8_MEMBER(h8_dma_device::dmawer_w)
{
	dmawer = data;
	logerror("dmawer_w %02x\n", data);
}

READ8_MEMBER(h8_dma_device::dmatcr_r)
{
	logerror("dmatcr_r %02x\n", dmatcr);
	return dmatcr;
}

WRITE8_MEMBER(h8_dma_device::dmatcr_w)
{
	dmatcr = data;
	logerror("dmatcr_w %02x\n", data);
}

READ16_MEMBER(h8_dma_device::dmabcr_r)
{
	logerror("dmabcr_r %04x\n", dmabcr);
	return dmabcr;
}

WRITE16_MEMBER(h8_dma_device::dmabcr_w)
{
	COMBINE_DATA(&dmabcr);
	logerror("dmabcr_w %04x\n", dmabcr);
	dmach0->set_bcr(dmabcr & 0x4000, dmabcr & 0x1000, dmabcr >>  8, dmabcr >> 4, dmabcr >> 0);
	dmach1->set_bcr(dmabcr & 0x8000, dmabcr & 0x2000, dmabcr >> 10, dmabcr >> 6, dmabcr >> 2);
}



h8_dma_channel_device::h8_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_DMA_CHANNEL, "H8 DMA channel", tag, owner, clock, "h8_dma_channel", __FILE__),
	dmac(*this, "^"),
	cpu(*this, "^^")
{
}

void h8_dma_channel_device::set_info(const char *_intc, int _irq_base, int v0, int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8, int v9, int va, int vb, int vc, int vd, int ve, int vf)
{
	intc_tag = _intc;
	irq_base = _irq_base;
	activation_vectors[ 0] = v0;
	activation_vectors[ 1] = v1;
	activation_vectors[ 2] = v2;
	activation_vectors[ 3] = v3;
	activation_vectors[ 4] = v4;
	activation_vectors[ 5] = v5;
	activation_vectors[ 6] = v6;
	activation_vectors[ 7] = v7;
	activation_vectors[ 8] = v8;
	activation_vectors[ 9] = v9;
	activation_vectors[10] = va;
	activation_vectors[11] = vb;
	activation_vectors[12] = vc;
	activation_vectors[13] = vd;
	activation_vectors[14] = ve;
	activation_vectors[15] = vf;
	memset(state, 0, sizeof(state));
}

void h8_dma_channel_device::device_start()
{
}

void h8_dma_channel_device::device_reset()
{
	dmacr = 0x0000;
	mar[0] = mar[1] = 0;
	ioar[0] = ioar[1] = 0;
	etcr[0] = etcr[1] = 0;
	fae = sae = false;
	dta = dte = dtie = 0;
}

void h8_dma_channel_device::set_id(int id)
{
	state[0].id = id;
	state[1].id = id | 1;

}

READ16_MEMBER(h8_dma_channel_device::marah_r)
{
	logerror("marah_r %06x\n", mar[0]);
	return mar[0] >> 16;
}

WRITE16_MEMBER(h8_dma_channel_device::marah_w)
{
	if(ACCESSING_BITS_0_7)
		mar[0] = ((data & 0x00ff) << 16) | (mar[0] & 0xffff);
	logerror("marah_w %06x\n", mar[0]);
}

READ16_MEMBER(h8_dma_channel_device::maral_r)
{
	logerror("maral_r %06x\n", mar[0]);
	return mar[0];
}

WRITE16_MEMBER(h8_dma_channel_device::maral_w)
{
	mar[0] = (mar[0] & ~mem_mask) | (data & mem_mask);
	logerror("maral_w %06x\n", mar[0]);
}

READ16_MEMBER(h8_dma_channel_device::ioara_r)
{
	logerror("iorar_r %04x\n", ioar[0]);
	return ioar[0];
}

WRITE16_MEMBER(h8_dma_channel_device::ioara_w)
{
	COMBINE_DATA(&ioar[0]);
	logerror("ioara_w %04x\n", ioar[0]);
}

READ16_MEMBER(h8_dma_channel_device::etcra_r)
{
	logerror("etcra_r %04x\n", etcr[0]);
	return etcr[0];
}

WRITE16_MEMBER(h8_dma_channel_device::etcra_w)
{
	COMBINE_DATA(&etcr[0]);
	logerror("etcra_w %04x\n", etcr[0]);
}

READ16_MEMBER(h8_dma_channel_device::marbh_r)
{
	logerror("marbh_r %06x\n", mar[1]);
	return mar[1] >> 16;
}

WRITE16_MEMBER(h8_dma_channel_device::marbh_w)
{
	if(ACCESSING_BITS_0_7)
		mar[1] = ((data & 0x00ff) << 16) | (mar[1] & 0xffff);
	logerror("marbh_w %06x\n", mar[1]);
}

READ16_MEMBER(h8_dma_channel_device::marbl_r)
{
	logerror("marbl_r %06x\n", mar[1]);
	return mar[1];
}

WRITE16_MEMBER(h8_dma_channel_device::marbl_w)
{
	mar[1] = (mar[1] & ~mem_mask) | (data & mem_mask);
	logerror("marbl_w %06x\n", mar[1]);
}

READ16_MEMBER(h8_dma_channel_device::ioarb_r)
{
	logerror("ioarb_r %04x\n", ioar[1]);
	return ioar[1];
}

WRITE16_MEMBER(h8_dma_channel_device::ioarb_w)
{
	COMBINE_DATA(&ioar[1]);
	logerror("ioarb_w %04x\n", ioar[1]);
}

READ16_MEMBER(h8_dma_channel_device::etcrb_r)
{
	logerror("etcrb_r %04x\n", etcr[1]);
	return etcr[1];
}

WRITE16_MEMBER(h8_dma_channel_device::etcrb_w)
{
	COMBINE_DATA(&etcr[1]);
	logerror("etcrb_w %04x\n", etcr[1]);
}

READ16_MEMBER(h8_dma_channel_device::dmacr_r)
{
	logerror("dmacr_r %04x\n", dmacr);
	return dmacr;
}

WRITE16_MEMBER(h8_dma_channel_device::dmacr_w)
{
	COMBINE_DATA(&dmacr);
	logerror("dmacr_w %04x\n", dmacr);
	start_test(-1);
}

void h8_dma_channel_device::set_bcr(bool _fae, bool _sae, UINT8 _dta, UINT8 _dte, UINT8 _dtie)
{
	fae  = _fae;
	sae  = _sae;
	dta  = _dta  & 3;
	dte  = _dte  & 3;
	dtie = _dtie & 3;
	logerror("fae=%d sae=%d dta=%d dte=%d dtie=%d\n", fae, sae, dta & 3, dte & 3, dtie & 3);
	start_test(-1);
}

bool h8_dma_channel_device::start_test(int vector)
{
	if(fae) {
		if(dte != 3)
			return false;

		if(dmacr & 0x0800)
			throw emu_fatalerror("%s: DMA startup test in full address/block mode unimplemented.\n", tag());
		else {
			if((dmacr & 0x0006) == 0x0006) {
				start(0);
				return true;
			} else
				throw emu_fatalerror("%s: DMA startup test in full address/normal mode/dreq unimplemented.\n", tag());
		}
	} else {
		if(dte == 0)
			return false;

		throw emu_fatalerror("%s: DMA startup test in short address mode unimplemented.\n", tag());		
	}
}

void h8_dma_channel_device::start(int submodule)
{
	if(fae) {
		if(dmacr & 0x0800)
			throw emu_fatalerror("%s: DMA start in full address/block mode unimplemented.\n", tag());
		else {
			state[submodule].source = mar[0];
			state[submodule].dest = mar[1];
			state[submodule].count = etcr[0] ? etcr[0] : 0x10000;
			state[submodule].mode_16 = dmacr & 0x8000;
			INT32 step = state[submodule].mode_16 ? 2 : 1;
			state[submodule].incs = dmacr & 0x2000 ? dmacr & 0x4000 ? -step : step : 0;
			state[submodule].incd = dmacr & 0x0020 ? dmacr & 0x0040 ? -step : step : 0;
			cpu->set_current_dma(state + submodule);
		}
	} else {
		throw emu_fatalerror("%s: DMA start in short address mode unimplemented.\n", tag());		
	}
}


void h8_dma_channel_device::count_done(int submodule)
{
	logerror("count done on %d\n", submodule);
	if(fae) {
		if(dmacr & 0x0800)
			throw emu_fatalerror("%s: DMA count done full address/block mode unimplemented.\n", tag());
		else {
			dte &= ~1;
			dmac->clear_dte(state[0].id);
			if(dtie & 1)
				throw emu_fatalerror("%s: DMA end-of-transfer interrupt in full address/normal mode unimplemented.\n", tag());
		}
	} else {
		throw emu_fatalerror("%s: DMA count done in short address mode unimplemented.\n", tag());		
	}
}
