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

static uv_pipe_t* luv_check_pipe(lua_State* L, int index) {
  uv_pipe_t* handle = luv_checkudata(L, index, "uv_pipe");
  luaL_argcheck(L, handle->type == UV_NAMED_PIPE && handle->data, index, "Expected uv_pipe_t");
  return handle;
}

static int luv_new_pipe(lua_State* L) {
  uv_pipe_t* handle;
  int ipc, ret;
  luaL_checktype(L, 1, LUA_TBOOLEAN);
  ipc = lua_toboolean(L, 1);
  handle = luv_newuserdata(L, sizeof(*handle));
  ret = uv_pipe_init(luv_loop(L), handle, ipc);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  handle->data = luv_setup_handle(L);
  return 1;
}

static int luv_pipe_open(lua_State* L) {
  uv_pipe_t* handle = luv_check_pipe(L, 1);
  uv_file file = luaL_checkinteger(L, 2);
  int ret = uv_pipe_open(handle, file);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_pipe_bind(lua_State* L) {
  uv_pipe_t* handle = luv_check_pipe(L, 1);
  const char* name = luaL_checkstring(L, 2);
  int ret = uv_pipe_bind(handle, name);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_pipe_connect(lua_State* L) {
  uv_pipe_t* handle = luv_check_pipe(L, 1);
  const char* name = luaL_checkstring(L, 2);
  int ref = luv_check_continuation(L, 3);
  uv_connect_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  uv_pipe_connect(req, handle, name, luv_connect_cb);
  return 1;
}

static int luv_pipe_getsockname(lua_State* L) {
  uv_pipe_t* handle = luv_check_pipe(L, 1);
  size_t len = 2*PATH_MAX;
  char buf[2*PATH_MAX];
  int ret = uv_pipe_getsockname(handle, buf, &len);
  if (ret < 0) return luv_error(L, ret);
  lua_pushlstring(L, buf, len);
  return 1;
}

static int luv_pipe_getpeername(lua_State* L) {
  uv_pipe_t* handle = luv_check_pipe(L, 1);
  size_t len = 2*PATH_MAX;
  char buf[2*PATH_MAX];
  int ret = uv_pipe_getpeername(handle, buf, &len);
  if (ret < 0) return luv_error(L, ret);
  lua_pushlstring(L, buf, len);
  return 1;
}

static int luv_pipe_pending_instances(lua_State* L) {
  uv_pipe_t* handle = luv_check_pipe(L, 1);
  int count = luaL_checkinteger(L, 2);
  uv_pipe_pending_instances(handle, count);
  return 0;
}

static int luv_pipe_pending_count(lua_State* L) {
  uv_pipe_t* handle = luv_check_pipe(L, 1);
  lua_pushinteger(L, uv_pipe_pending_count(handle));
  return 1;
}

static int luv_pipe_pending_type(lua_State* L) {
  uv_pipe_t* handle = luv_check_pipe(L, 1);
  uv_handle_type type = uv_pipe_pending_type(handle);
  const char* type_name;
  switch (type) {
#define XX(uc, lc) \
    case UV_##uc: type_name = #lc; break;
  UV_HANDLE_TYPE_MAP(XX)
#undef XX
    default: return 0;
  }
  lua_pushstring(L, type_name);
  return 1;
}
