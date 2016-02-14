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

static void* luv_newuserdata(lua_State* L, size_t sz) {
  void* handle = malloc(sz);
  if (handle) {
    *(void**)lua_newuserdata(L, sizeof(void*)) = handle;
  }
  return handle;
}

static void* luv_checkudata(lua_State* L, int ud, const char* tname) {
  return *(void**) luaL_checkudata(L, ud, tname);
}

static uv_handle_t* luv_check_handle(lua_State* L, int index) {
  int isHandle;
  uv_handle_t* handle;
  if (!(handle = *(void**)lua_touserdata(L, index))) { goto fail; }
  lua_getfield(L, LUA_REGISTRYINDEX, "uv_handle");
  lua_getmetatable(L, index < 0 ? index - 1 : index);
  lua_rawget(L, -2);
  isHandle = lua_toboolean(L, -1);
  lua_pop(L, 2);
  if (isHandle) { return handle; }
  fail: luaL_argerror(L, index, "Expected uv_handle userdata");
  return NULL;
}

// Show the libuv type instead of generic "userdata"
static int luv_handle_tostring(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  switch (handle->type) {
#define XX(uc, lc) case UV_##uc: lua_pushfstring(L, "uv_"#lc"_t: %p", handle); break;
  UV_HANDLE_TYPE_MAP(XX)
#undef XX
    default: lua_pushfstring(L, "uv_handle_t: %p", handle); break;
  }
  return 1;
}

static int luv_is_active(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  int ret = uv_is_active(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushboolean(L, ret);
  return 1;
}

static int luv_is_closing(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  int ret = uv_is_closing(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushboolean(L, ret);
  return 1;
}

static void luv_close_cb(uv_handle_t* handle) {
  lua_State* L = luv_state(handle->loop);
  luv_handle_t* data = handle->data;
  if (!data) return;
  luv_call_callback(L, data, LUV_CLOSED, 0);
  luv_cleanup_handle(L, data);
  handle->data = NULL;
}

static int luv_close(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  if (uv_is_closing(handle)) {
    luaL_error(L, "handle %p is already closing", handle);
  }
  if (!lua_isnoneornil(L, 2)) {
    luv_check_callback(L, handle->data, LUV_CLOSED, 2);
  }
  uv_close(handle, luv_close_cb);
  return 0;
}

static void luv_gc_cb(uv_handle_t* handle) {
  luv_close_cb(handle);
  free(handle);
}

static int luv_handle_gc(lua_State* L) {
  void** udata = lua_touserdata(L, 1);
  uv_handle_t* handle = *udata;
  if (handle != NULL) {
    if (!uv_is_closing(handle))
      uv_close(handle, luv_gc_cb);
    else 
      free(*udata);

    *udata = NULL;
  }

  return 0;
}

static int luv_ref(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  uv_ref(handle);
  return 0;
}

static int luv_unref(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  uv_unref(handle);
  return 0;
}

static int luv_has_ref(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  int ret = uv_has_ref(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushboolean(L, ret);
  return 1;
}

static int luv_send_buffer_size(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  int value;
  int ret;
  if (lua_isnoneornil(L, 2)) {
    value = 0;
  }
  else {
    value = luaL_checkinteger(L, 2);
  }
  ret = uv_send_buffer_size(handle, &value);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_recv_buffer_size(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  int value;
  int ret;
  if (lua_isnoneornil(L, 2)) {
    value = 0;
  }
  else {
    value = luaL_checkinteger(L, 2);
  }
  ret = uv_recv_buffer_size(handle, &value);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_fileno(lua_State* L) {
  uv_handle_t* handle = luv_check_handle(L, 1);
  uv_os_fd_t fd;
  int ret = uv_fileno(handle, &fd);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, (LUA_INTEGER)(ptrdiff_t)fd);
  return 1;
}
