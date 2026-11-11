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
#include "codec_ver.h"

#include <inttypes.h>

int32_t WelsCreateSVCEncoder (ISVCEncoder**) {
  return 1; // Failure
}

void WelsDestroySVCEncoder (ISVCEncoder*) {
}

OpenH264Version WelsGetCodecVersion() {
  return g_stCodecVersion;
}

void WelsGetCodecVersionEx (OpenH264Version* pVersion) {
  *pVersion = g_stCodecVersion;
}
