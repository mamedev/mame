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

static uv_poll_t* luv_check_poll(lua_State* L, int index) {
  uv_poll_t* handle = luv_checkudata(L, index, "uv_poll");
  luaL_argcheck(L, handle->type == UV_POLL && handle->data, index, "Expected uv_poll_t");
  return handle;
}

static int luv_new_poll(lua_State* L) {
  int fd = luaL_checkinteger(L, 1);
  uv_poll_t* handle = luv_newuserdata(L, sizeof(*handle));
  int ret = uv_poll_init(luv_loop(L), handle, fd);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  handle->data = luv_setup_handle(L);
  return 1;
}

static int luv_new_socket_poll(lua_State* L) {
  int fd = luaL_checkinteger(L, 1);
  uv_poll_t* handle = luv_newuserdata(L, sizeof(*handle));
  int ret = uv_poll_init_socket(luv_loop(L), handle, fd);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  handle->data = luv_setup_handle(L);
  return 1;
}

// These are the same order as uv_run_mode which also starts at 0
static const char *const luv_pollevents[] = {
  "r", "w", "rw", NULL
};

static void luv_poll_cb(uv_poll_t* handle, int status, int events) {
  lua_State* L = luv_state(handle->loop);
  luv_handle_t* data = handle->data;
  const char* evtstr;

  if (status < 0) {
    fprintf(stderr, "%s: %s\n", uv_err_name(status), uv_strerror(status));
    lua_pushstring(L, uv_err_name(status));
  }
  else {
    lua_pushnil(L);
  }

  switch (events) {
    case UV_READABLE: evtstr = "r"; break;
    case UV_WRITABLE: evtstr = "w"; break;
    case UV_READABLE|UV_WRITABLE: evtstr = "rw"; break;
    default: evtstr = ""; break;
  }
  lua_pushstring(L, evtstr);

  luv_call_callback(L, data, LUV_POLL, 2);
}

static int luv_poll_start(lua_State* L) {
  uv_poll_t* handle = luv_check_poll(L, 1);
  int events, ret;
  switch (luaL_checkoption(L, 2, "rw", luv_pollevents)) {
    case 0: events = UV_READABLE; break;
    case 1: events = UV_WRITABLE; break;
    case 2: events = UV_READABLE | UV_WRITABLE; break;
    default: events = 0; /* unreachable */
  }
  luv_check_callback(L, handle->data, LUV_POLL, 3);
  ret = uv_poll_start(handle, events, luv_poll_cb);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_poll_stop(lua_State* L) {
  uv_poll_t* handle = luv_check_poll(L, 1);
  int ret = uv_poll_stop(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}
