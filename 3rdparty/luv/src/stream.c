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

static void luv_check_buf(lua_State *L, int idx, uv_buf_t *pbuf) {
    size_t len;
    pbuf->base = (char*)luaL_checklstring(L, idx, &len);
    pbuf->len = len;
}

static uv_stream_t* luv_check_stream(lua_State* L, int index) {
  int isStream;
  uv_stream_t* handle;
  if (!(handle = *(void**) lua_touserdata(L, index))) { goto fail; }
  lua_getfield(L, LUA_REGISTRYINDEX, "uv_stream");
  lua_getmetatable(L, index < 0 ? index - 1 : index);
  lua_rawget(L, -2);
  isStream = lua_toboolean(L, -1);
  lua_pop(L, 2);
  if (isStream) { return handle; }
  fail: luaL_argerror(L, index, "Expected uv_stream userdata");
  return NULL;
}

static void luv_shutdown_cb(uv_shutdown_t* req, int status) {
  lua_State* L = luv_state(req->handle->loop);
  luv_status(L, status);
  luv_fulfill_req(L, req->data, 1);
  luv_cleanup_req(L, req->data);
  req->data = NULL;
}

static int luv_shutdown(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_shutdown_t* req = lua_newuserdata(L, sizeof(*req));
  int ret;
  req->data = luv_setup_req(L, ref);
  ret = uv_shutdown(req, handle, luv_shutdown_cb);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  return 1;
}

static void luv_connection_cb(uv_stream_t* handle, int status) {
  lua_State* L = luv_state(handle->loop);
  luv_status(L, status);
  luv_call_callback(L, handle->data, LUV_CONNECTION, 1);
}

static int luv_listen(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  int backlog = luaL_checkinteger(L, 2);
  int ret;
  luv_check_callback(L, handle->data, LUV_CONNECTION, 3);
  ret = uv_listen(handle, backlog, luv_connection_cb);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_accept(lua_State* L) {
  uv_stream_t* server = luv_check_stream(L, 1);
  uv_stream_t* client = luv_check_stream(L, 2);
  int ret = uv_accept(server, client);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static void luv_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  (void)handle;
  buf->base = malloc(suggested_size);
  assert(buf->base);
  buf->len = suggested_size;
}

static void luv_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  lua_State* L = luv_state(handle->loop);
  int nargs;

  if (nread > 0) {
    lua_pushnil(L);
    lua_pushlstring(L, buf->base, nread);
    nargs = 2;
  }

  free(buf->base);
  if (nread == 0) return;

  if (nread == UV_EOF) {
    nargs = 0;
  }
  else if (nread < 0) {
    luv_status(L, nread);
    nargs = 1;
  }

  luv_call_callback(L, handle->data, LUV_READ, nargs);
}

static int luv_read_start(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  int ret;
  luv_check_callback(L, handle->data, LUV_READ, 2);
  ret = uv_read_start(handle, luv_alloc_cb, luv_read_cb);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_read_stop(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  int ret = uv_read_stop(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static void luv_write_cb(uv_write_t* req, int status) {
  lua_State* L = luv_state(req->handle->loop);
  luv_status(L, status);
  luv_fulfill_req(L, req->data, 1);
  luv_cleanup_req(L, req->data);
  req->data = NULL;
}

static uv_buf_t* luv_prep_bufs(lua_State* L, int index, size_t *count) {
  uv_buf_t *bufs;
  size_t i;
  *count = lua_rawlen(L, index);
  bufs = malloc(sizeof(uv_buf_t) * *count);
  for (i = 0; i < *count; ++i) {
    lua_rawgeti(L, index, i + 1);
    luv_check_buf(L, -1, &bufs[i]);
    lua_pop(L, 1);
  }
  return bufs;
}

static int luv_write(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  uv_write_t* req;
  int ret, ref;
  ref = luv_check_continuation(L, 3);
  req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  if (lua_istable(L, 2)) {
    size_t count;
    uv_buf_t *bufs = luv_prep_bufs(L, 2, &count);
    ret = uv_write(req, handle, bufs, count, luv_write_cb);
    free(bufs);
  }
  else if (lua_isstring(L, 2)) {
    uv_buf_t buf;
    luv_check_buf(L, 2, &buf);
    ret = uv_write(req, handle, &buf, 1, luv_write_cb);
  }
  else {
    return luaL_argerror(L, 2, "data must be string or table of strings");
  }
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  lua_pushvalue(L, 2);
  ((luv_req_t*)req->data)->data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  return 1;
}

static int luv_write2(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  uv_write_t* req;
  int ret, ref;
  uv_stream_t* send_handle;
  send_handle = luv_check_stream(L, 3);
  ref = luv_check_continuation(L, 4);
  req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  if (lua_istable(L, 2)) {
    size_t count;
    uv_buf_t *bufs = luv_prep_bufs(L, 2, &count);
    ret = uv_write2(req, handle, bufs, count, send_handle, luv_write_cb);
    free(bufs);
  }
  else if (lua_isstring(L, 2)) {
    uv_buf_t buf;
    luv_check_buf(L, 2, &buf);
    ret = uv_write2(req, handle, &buf, 1, send_handle, luv_write_cb);
  }
  else {
    return luaL_argerror(L, 2, "data must be string or table of strings");
  }
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  lua_pushvalue(L, 2);
  ((luv_req_t*)req->data)->data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  return 1;
}

static int luv_try_write(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  int ret;
  if (lua_istable(L, 2)) {
    size_t count;
    uv_buf_t *bufs = luv_prep_bufs(L, 2, &count);
    ret = uv_try_write(handle, bufs, count);
    free(bufs);
  }
  else if (lua_isstring(L, 2)) {
    uv_buf_t buf;
    luv_check_buf(L, 2, &buf);
    ret = uv_try_write(handle, &buf, 1);
  }
  else {
    return luaL_argerror(L, 2, "data must be string or table of strings");
  }
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_is_readable(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  lua_pushboolean(L, uv_is_readable(handle));
  return 1;
}

static int luv_is_writable(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  lua_pushboolean(L, uv_is_writable(handle));
  return 1;
}

static int luv_stream_set_blocking(lua_State* L) {
  uv_stream_t* handle = luv_check_stream(L, 1);
  int blocking, ret;
  luaL_checktype(L, 2, LUA_TBOOLEAN);
  blocking = lua_toboolean(L, 2);
  ret = uv_stream_set_blocking(handle, blocking);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

