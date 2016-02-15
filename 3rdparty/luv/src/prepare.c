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
#include "luv.h"

static uv_prepare_t* luv_check_prepare(lua_State* L, int index) {
  uv_prepare_t* handle = luv_checkudata(L, index, "uv_prepare");
  luaL_argcheck(L, handle->type == UV_PREPARE && handle->data, index, "Expected uv_prepare_t");
  return handle;
}

static int luv_new_prepare(lua_State* L) {
  uv_prepare_t* handle = luv_newuserdata(L, sizeof(*handle));
  int ret = uv_prepare_init(luv_loop(L), handle);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  handle->data = luv_setup_handle(L);
  return 1;
}

static void luv_prepare_cb(uv_prepare_t* handle) {
  lua_State* L = luv_state(handle->loop);
  luv_handle_t* data = handle->data;
  luv_call_callback(L, data, LUV_PREPARE, 0);
}

static int luv_prepare_start(lua_State* L) {
  uv_prepare_t* handle = luv_check_prepare(L, 1);
  int ret;
  luv_check_callback(L, handle->data, LUV_PREPARE, 2);
  ret = uv_prepare_start(handle, luv_prepare_cb);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_prepare_stop(lua_State* L) {
  uv_prepare_t* handle = luv_check_prepare(L, 1);
  int ret = uv_prepare_stop(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

