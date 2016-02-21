/*
 *  Copyright 2014 The Luvit Authors. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
#ifndef LUV_LREQ_H
#define LUV_LREQ_H

#include "luv.h"

typedef struct {
  int req_ref; /* ref for uv_req_t's userdata */
  int callback_ref; /* ref for callback */
  int data_ref; /* ref for write data */
  void* data; /* extra data */
} luv_req_t;

/* Used in the top of a setup function to check the arg
   and ref the callback to an integer.
*/
static int luv_check_continuation(lua_State* L, int index);

/* setup a luv_req_t.  The userdata is assumed to be at the
   top of the stack.
*/
static luv_req_t* luv_setup_req(lua_State* L, int ref);

static void luv_fulfill_req(lua_State* L, luv_req_t* data, int nargs);

static void luv_cleanup_req(lua_State* L, luv_req_t* data);

#endif
