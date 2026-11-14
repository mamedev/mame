/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright © 2018 Endless Mobile, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *  - Mario Sanchez Prada <mario@endlessm.com>
 */

#include "codec_api.h"

/*
*  WelsGetDecoderCapability
*  @return: DecCapability information
*/
int WelsGetDecoderCapability (SDecoderCapability*) {
  return 3; // ERR_API_FAILED, see codec/decoder/core/inc/error_code.h
}

/*
*   WelsCreateDecoder
*   @return:    success in return 0, otherwise failed.
*/
long WelsCreateDecoder (ISVCDecoder**) {
  return 3; // ERR_API_FAILED, see codec/decoder/core/inc/error_code.h
}

/*
*   WelsDestroyDecoder
*/
void WelsDestroyDecoder (ISVCDecoder*) {
}
