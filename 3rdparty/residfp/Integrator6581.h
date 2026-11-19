/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2023 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004, 2010 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef INTEGRATOR6581_H
#define INTEGRATOR6581_H

#include "Integrator.h"
#include "FilterModelConfig6581.h"

#include <cstdint>
#include <cassert>

// uncomment to enable use of the slope factor
// in the EKV model
// actually produces worse results, needs investigation
//#define SLOPE_FACTOR

#include "siddefs-fp.h"

namespace reSIDfp
{

/**
 * Find output voltage in inverting integrator SID op-amp circuits, using a
 * single fixpoint iteration step.
 *
 * A circuit diagram of a MOS 6581 integrator is shown below.
 *
 *                   +---C---+
 *                   |       |
 *     vi --o--Rw--o-o--[A>--o-- vo
 *          |      | vx
 *          +--Rs--+
 *
 * From Kirchoff's current law it follows that
 *
 *     IRw + IRs + ICr = 0
 *
 * Using the formula for current through a capacitor, i = C*dv/dt, we get
 *
 *     IRw + IRs + C*(vc - vc0)/dt = 0
 *     dt/C*(IRw + IRs) + vc - vc0 = 0
 *     vc = vc0 - n*(IRw(vi,vx) + IRs(vi,vx))
 *
 * which may be rewritten as the following iterative fixpoint function:
 *
 *     vc = vc0 - n*(IRw(vi,g(vc)) + IRs(vi,g(vc)))
 *
 * To accurately calculate the currents through Rs and Rw, we need to use
 * transistor models. Rs has a gate voltage of Vdd = 12V, and can be
 * assumed to always be in triode mode. For Rw, the situation is rather
 * more complex, as it turns out that this transistor will operate in
 * both subthreshold, triode, and saturation modes.
 *
 * The Shichman-Hodges transistor model routinely used in textbooks may
 * be written as follows:
 *
 *     Ids = 0                          , Vgst < 0               (subthreshold mode)
 *     Ids = K*W/L*(2*Vgst - Vds)*Vds   , Vgst >= 0, Vds < Vgst  (triode mode)
 *     Ids = K*W/L*Vgst^2               , Vgst >= 0, Vds >= Vgst (saturation mode)
 *
 * where
 *     K   = u*Cox/2 (transconductance coefficient)
 *     W/L = ratio between substrate width and length
 *     Vgst = Vg - Vs - Vt (overdrive voltage)
 *
 * This transistor model is also called the quadratic model.
 *
 * Note that the equation for the triode mode can be reformulated as
 * independent terms depending on Vgs and Vgd, respectively, by the
 * following substitution:
 *
 *     Vds = Vgst - (Vgst - Vds) = Vgst - Vgdt
 *
 *     Ids = K*W/L*(2*Vgst - Vds)*Vds
 *         = K*W/L*(2*Vgst - (Vgst - Vgdt)*(Vgst - Vgdt)
 *         = K*W/L*(Vgst + Vgdt)*(Vgst - Vgdt)
 *         = K*W/L*(Vgst^2 - Vgdt^2)
 *
 * This turns out to be a general equation which covers both the triode
 * and saturation modes (where the second term is 0 in saturation mode).
 * The equation is also symmetrical, i.e. it can calculate negative
 * currents without any change of parameters (since the terms for drain
 * and source are identical except for the sign).
 *
 * FIXME: Subthreshold as function of Vgs, Vgd.
 *
 *     Ids = I0*W/L*e^(Vgst/(Ut/k))   , Vgst < 0               (subthreshold mode)
 *
 * where
 *     I0 = (2 * uCox * Ut^2) / k
 *
 * The remaining problem with the textbook model is that the transition
 * from subthreshold to triode/saturation is not continuous.
 *
 * Realizing that the subthreshold and triode/saturation modes may both
 * be defined by independent (and equal) terms of Vgs and Vds,
 * respectively, the corresponding terms can be blended into (equal)
 * continuous functions suitable for table lookup.
 *
 * The EKV model (Enz, Krummenacher and Vittoz) essentially performs this
 * blending using an elegant mathematical formulation:
 *
 *     Ids = Is * (if - ir)
 *     Is = ((2 * u*Cox * Ut^2)/k) * W/L
 *     if = ln^2(1 + e^((k*(Vg - Vt) - Vs)/(2*Ut))
 *     ir = ln^2(1 + e^((k*(Vg - Vt) - Vd)/(2*Ut))
 *
 * For our purposes, the EKV model preserves two important properties
 * discussed above:
 *
 * - It consists of two independent terms, which can be represented by
 *   the same lookup table.
 * - It is symmetrical, i.e. it calculates current in both directions,
 *   facilitating a branch-free implementation.
 *
 * Rw in the circuit diagram above is a VCR (voltage controlled resistor),
 * as shown in the circuit diagram below.
 *
 *
 *                        Vdd
 *                           |
 *              Vdd         _|_
 *                 |    +---+ +---- Vw
 *                _|_   |
 *             +--+ +---o Vg
 *             |      __|__
 *             |      -----  Rw
 *             |      |   |
 *     vi -----o------+   +-------- vo
 *
 *
 * In order to calculalate the current through the VCR, its gate voltage
 * must be determined.
 *
 * Assuming triode mode and applying Kirchoff's current law, we get the
 * following equation for Vg:
 *
 *     u*Cox/2*W/L*((nVddt - Vg)^2 - (nVddt - vi)^2 + (nVddt - Vg)^2 - (nVddt - Vw)^2) = 0
 *     2*(nVddt - Vg)^2 - (nVddt - vi)^2 - (nVddt - Vw)^2 = 0
 *     (nVddt - Vg) = sqrt(((nVddt - vi)^2 + (nVddt - Vw)^2)/2)
 *
 *     Vg = nVddt - sqrt(((nVddt - vi)^2 + (nVddt - Vw)^2)/2)
 */
class Integrator6581 : public Integrator
{
    friend class State;

private:
    const double wlSnake;

#ifdef SLOPE_FACTOR
    // Slope factor n = 1/k
    // where k is the gate coupling coefficient
    // k = Cox/(Cox+Cdep) ~ 0.7 (depends on gate voltage)
    mutable double n;
#endif

    uint32_t nVddt_Vw_2;

    const uint16_t nVddt;
    const uint16_t nVt;
    const uint16_t nVmin;

    FilterModelConfig6581& fmc;

public:
    explicit Integrator6581(FilterModelConfig6581& new_fmc) :
        wlSnake(new_fmc.getWL_snake()),
#ifdef SLOPE_FACTOR
        n(1.4),
#endif
        nVddt_Vw_2(0),
        nVddt(new_fmc.getNormalizedValue(new_fmc.getVddt())),
        nVt(new_fmc.getNormalizedValue(new_fmc.getVth())),
        nVmin(new_fmc.getNVmin()),
        fmc(new_fmc) {}

    void setVw(uint16_t Vw) { nVddt_Vw_2 = ((nVddt - Vw) * (nVddt - Vw)) >> 1; }

    int32_t solve(int32_t vi) const override;
};

} // namespace reSIDfp

#endif
