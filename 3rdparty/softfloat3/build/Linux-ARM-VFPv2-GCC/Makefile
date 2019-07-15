
#=============================================================================
#
# This Makefile is part of the SoftFloat IEEE Floating-Point Arithmetic
# Package, Release 3e, by John R. Hauser.
#
# Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2017 The Regents of the
# University of California.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions, and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions, and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  3. Neither the name of the University nor the names of its contributors
#     may be used to endorse or promote products derived from this software
#     without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
# DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#=============================================================================

SOURCE_DIR ?= ../../source
SPECIALIZE_TYPE ?= ARM-VFPv2

SOFTFLOAT_OPTS ?= -DSOFTFLOAT_ROUND_ODD -DINLINE_LEVEL=5

DELETE = rm -f
C_INCLUDES = -I. -I$(SOURCE_DIR)/$(SPECIALIZE_TYPE) -I$(SOURCE_DIR)/include
COMPILE_C = \
  gcc -c -Werror-implicit-function-declaration $(SOFTFLOAT_OPTS) \
    $(C_INCLUDES) -O2 -o $@
MAKELIB = ar crs $@

OBJ = .o
LIB = .a

OTHER_HEADERS =

.PHONY: all
all: softfloat$(LIB)

OBJS_PRIMITIVES = \
  s_compare96M$(OBJ) \
  s_compare128M$(OBJ) \
  s_shortShiftLeft64To96M$(OBJ) \
  s_shortShiftLeftM$(OBJ) \
  s_shiftLeftM$(OBJ) \
  s_shortShiftRightM$(OBJ) \
  s_shortShiftRightJam64$(OBJ) \
  s_shortShiftRightJamM$(OBJ) \
  s_shiftRightJam32$(OBJ) \
  s_shiftRightJam64$(OBJ) \
  s_shiftRightJamM$(OBJ) \
  s_shiftRightM$(OBJ) \
  s_countLeadingZeros8$(OBJ) \
  s_countLeadingZeros16$(OBJ) \
  s_countLeadingZeros32$(OBJ) \
  s_countLeadingZeros64$(OBJ) \
  s_addM$(OBJ) \
  s_addCarryM$(OBJ) \
  s_addComplCarryM$(OBJ) \
  s_negXM$(OBJ) \
  s_sub1XM$(OBJ) \
  s_subM$(OBJ) \
  s_mul64To128M$(OBJ) \
  s_mul128MTo256M$(OBJ) \
  s_approxRecip_1Ks$(OBJ) \
  s_approxRecip32_1$(OBJ) \
  s_approxRecipSqrt_1Ks$(OBJ) \
  s_approxRecipSqrt32_1$(OBJ) \
  s_remStepMBy32$(OBJ) \

OBJS_SPECIALIZE = \
  softfloat_raiseFlags$(OBJ) \
  s_f16UIToCommonNaN$(OBJ) \
  s_commonNaNToF16UI$(OBJ) \
  s_propagateNaNF16UI$(OBJ) \
  s_f32UIToCommonNaN$(OBJ) \
  s_commonNaNToF32UI$(OBJ) \
  s_propagateNaNF32UI$(OBJ) \
  s_f64UIToCommonNaN$(OBJ) \
  s_commonNaNToF64UI$(OBJ) \
  s_propagateNaNF64UI$(OBJ) \
  extF80M_isSignalingNaN$(OBJ) \
  s_extF80MToCommonNaN$(OBJ) \
  s_commonNaNToExtF80M$(OBJ) \
  s_propagateNaNExtF80M$(OBJ) \
  f128M_isSignalingNaN$(OBJ) \
  s_f128MToCommonNaN$(OBJ) \
  s_commonNaNToF128M$(OBJ) \
  s_propagateNaNF128M$(OBJ) \

OBJS_OTHERS = \
  s_roundToUI32$(OBJ) \
  s_roundMToUI64$(OBJ) \
  s_roundToI32$(OBJ) \
  s_roundMToI64$(OBJ) \
  s_normSubnormalF16Sig$(OBJ) \
  s_roundPackToF16$(OBJ) \
  s_normRoundPackToF16$(OBJ) \
  s_addMagsF16$(OBJ) \
  s_subMagsF16$(OBJ) \
  s_mulAddF16$(OBJ) \
  s_normSubnormalF32Sig$(OBJ) \
  s_roundPackToF32$(OBJ) \
  s_normRoundPackToF32$(OBJ) \
  s_addMagsF32$(OBJ) \
  s_subMagsF32$(OBJ) \
  s_mulAddF32$(OBJ) \
  s_normSubnormalF64Sig$(OBJ) \
  s_roundPackToF64$(OBJ) \
  s_normRoundPackToF64$(OBJ) \
  s_addMagsF64$(OBJ) \
  s_subMagsF64$(OBJ) \
  s_mulAddF64$(OBJ) \
  s_tryPropagateNaNExtF80M$(OBJ) \
  s_invalidExtF80M$(OBJ) \
  s_normExtF80SigM$(OBJ) \
  s_roundPackMToExtF80M$(OBJ) \
  s_normRoundPackMToExtF80M$(OBJ) \
  s_addExtF80M$(OBJ) \
  s_compareNonnormExtF80M$(OBJ) \
  s_isNaNF128M$(OBJ) \
  s_tryPropagateNaNF128M$(OBJ) \
  s_invalidF128M$(OBJ) \
  s_shiftNormSigF128M$(OBJ) \
  s_roundPackMToF128M$(OBJ) \
  s_normRoundPackMToF128M$(OBJ) \
  s_addF128M$(OBJ) \
  s_mulAddF128M$(OBJ) \
  softfloat_state$(OBJ) \
  ui32_to_f16$(OBJ) \
  ui32_to_f32$(OBJ) \
  ui32_to_f64$(OBJ) \
  ui32_to_extF80M$(OBJ) \
  ui32_to_f128M$(OBJ) \
  ui64_to_f16$(OBJ) \
  ui64_to_f32$(OBJ) \
  ui64_to_f64$(OBJ) \
  ui64_to_extF80M$(OBJ) \
  ui64_to_f128M$(OBJ) \
  i32_to_f16$(OBJ) \
  i32_to_f32$(OBJ) \
  i32_to_f64$(OBJ) \
  i32_to_extF80M$(OBJ) \
  i32_to_f128M$(OBJ) \
  i64_to_f16$(OBJ) \
  i64_to_f32$(OBJ) \
  i64_to_f64$(OBJ) \
  i64_to_extF80M$(OBJ) \
  i64_to_f128M$(OBJ) \
  f16_to_ui32$(OBJ) \
  f16_to_ui64$(OBJ) \
  f16_to_i32$(OBJ) \
  f16_to_i64$(OBJ) \
  f16_to_ui32_r_minMag$(OBJ) \
  f16_to_ui64_r_minMag$(OBJ) \
  f16_to_i32_r_minMag$(OBJ) \
  f16_to_i64_r_minMag$(OBJ) \
  f16_to_f32$(OBJ) \
  f16_to_f64$(OBJ) \
  f16_to_extF80M$(OBJ) \
  f16_to_f128M$(OBJ) \
  f16_roundToInt$(OBJ) \
  f16_add$(OBJ) \
  f16_sub$(OBJ) \
  f16_mul$(OBJ) \
  f16_mulAdd$(OBJ) \
  f16_div$(OBJ) \
  f16_rem$(OBJ) \
  f16_sqrt$(OBJ) \
  f16_eq$(OBJ) \
  f16_le$(OBJ) \
  f16_lt$(OBJ) \
  f16_eq_signaling$(OBJ) \
  f16_le_quiet$(OBJ) \
  f16_lt_quiet$(OBJ) \
  f16_isSignalingNaN$(OBJ) \
  f32_to_ui32$(OBJ) \
  f32_to_ui64$(OBJ) \
  f32_to_i32$(OBJ) \
  f32_to_i64$(OBJ) \
  f32_to_ui32_r_minMag$(OBJ) \
  f32_to_ui64_r_minMag$(OBJ) \
  f32_to_i32_r_minMag$(OBJ) \
  f32_to_i64_r_minMag$(OBJ) \
  f32_to_f16$(OBJ) \
  f32_to_f64$(OBJ) \
  f32_to_extF80M$(OBJ) \
  f32_to_f128M$(OBJ) \
  f32_roundToInt$(OBJ) \
  f32_add$(OBJ) \
  f32_sub$(OBJ) \
  f32_mul$(OBJ) \
  f32_mulAdd$(OBJ) \
  f32_div$(OBJ) \
  f32_rem$(OBJ) \
  f32_sqrt$(OBJ) \
  f32_eq$(OBJ) \
  f32_le$(OBJ) \
  f32_lt$(OBJ) \
  f32_eq_signaling$(OBJ) \
  f32_le_quiet$(OBJ) \
  f32_lt_quiet$(OBJ) \
  f32_isSignalingNaN$(OBJ) \
  f64_to_ui32$(OBJ) \
  f64_to_ui64$(OBJ) \
  f64_to_i32$(OBJ) \
  f64_to_i64$(OBJ) \
  f64_to_ui32_r_minMag$(OBJ) \
  f64_to_ui64_r_minMag$(OBJ) \
  f64_to_i32_r_minMag$(OBJ) \
  f64_to_i64_r_minMag$(OBJ) \
  f64_to_f16$(OBJ) \
  f64_to_f32$(OBJ) \
  f64_to_extF80M$(OBJ) \
  f64_to_f128M$(OBJ) \
  f64_roundToInt$(OBJ) \
  f64_add$(OBJ) \
  f64_sub$(OBJ) \
  f64_mul$(OBJ) \
  f64_mulAdd$(OBJ) \
  f64_div$(OBJ) \
  f64_rem$(OBJ) \
  f64_sqrt$(OBJ) \
  f64_eq$(OBJ) \
  f64_le$(OBJ) \
  f64_lt$(OBJ) \
  f64_eq_signaling$(OBJ) \
  f64_le_quiet$(OBJ) \
  f64_lt_quiet$(OBJ) \
  f64_isSignalingNaN$(OBJ) \
  extF80M_to_ui32$(OBJ) \
  extF80M_to_ui64$(OBJ) \
  extF80M_to_i32$(OBJ) \
  extF80M_to_i64$(OBJ) \
  extF80M_to_ui32_r_minMag$(OBJ) \
  extF80M_to_ui64_r_minMag$(OBJ) \
  extF80M_to_i32_r_minMag$(OBJ) \
  extF80M_to_i64_r_minMag$(OBJ) \
  extF80M_to_f16$(OBJ) \
  extF80M_to_f32$(OBJ) \
  extF80M_to_f64$(OBJ) \
  extF80M_to_f128M$(OBJ) \
  extF80M_roundToInt$(OBJ) \
  extF80M_add$(OBJ) \
  extF80M_sub$(OBJ) \
  extF80M_mul$(OBJ) \
  extF80M_div$(OBJ) \
  extF80M_rem$(OBJ) \
  extF80M_sqrt$(OBJ) \
  extF80M_eq$(OBJ) \
  extF80M_le$(OBJ) \
  extF80M_lt$(OBJ) \
  extF80M_eq_signaling$(OBJ) \
  extF80M_le_quiet$(OBJ) \
  extF80M_lt_quiet$(OBJ) \
  f128M_to_ui32$(OBJ) \
  f128M_to_ui64$(OBJ) \
  f128M_to_i32$(OBJ) \
  f128M_to_i64$(OBJ) \
  f128M_to_ui32_r_minMag$(OBJ) \
  f128M_to_ui64_r_minMag$(OBJ) \
  f128M_to_i32_r_minMag$(OBJ) \
  f128M_to_i64_r_minMag$(OBJ) \
  f128M_to_f16$(OBJ) \
  f128M_to_f32$(OBJ) \
  f128M_to_f64$(OBJ) \
  f128M_to_extF80M$(OBJ) \
  f128M_roundToInt$(OBJ) \
  f128M_add$(OBJ) \
  f128M_sub$(OBJ) \
  f128M_mul$(OBJ) \
  f128M_mulAdd$(OBJ) \
  f128M_div$(OBJ) \
  f128M_rem$(OBJ) \
  f128M_sqrt$(OBJ) \
  f128M_eq$(OBJ) \
  f128M_le$(OBJ) \
  f128M_lt$(OBJ) \
  f128M_eq_signaling$(OBJ) \
  f128M_le_quiet$(OBJ) \
  f128M_lt_quiet$(OBJ) \

OBJS_ALL = $(OBJS_PRIMITIVES) $(OBJS_SPECIALIZE) $(OBJS_OTHERS)

$(OBJS_ALL): \
  $(OTHER_HEADERS) platform.h $(SOURCE_DIR)/include/primitiveTypes.h \
  $(SOURCE_DIR)/include/primitives.h
$(OBJS_SPECIALIZE) $(OBJS_OTHERS): \
  $(SOURCE_DIR)/include/softfloat_types.h $(SOURCE_DIR)/include/internals.h \
  $(SOURCE_DIR)/$(SPECIALIZE_TYPE)/specialize.h \
  $(SOURCE_DIR)/include/softfloat.h

$(OBJS_PRIMITIVES) $(OBJS_OTHERS): %$(OBJ): $(SOURCE_DIR)/%.c
	$(COMPILE_C) $(SOURCE_DIR)/$*.c

$(OBJS_SPECIALIZE): %$(OBJ): $(SOURCE_DIR)/$(SPECIALIZE_TYPE)/%.c
	$(COMPILE_C) $(SOURCE_DIR)/$(SPECIALIZE_TYPE)/$*.c

softfloat$(LIB): $(OBJS_ALL)
	$(DELETE) $@
	$(MAKELIB) $^

.PHONY: clean
clean:
	$(DELETE) $(OBJS_ALL) softfloat$(LIB)

