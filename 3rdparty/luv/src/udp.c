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

static uv_udp_t* luv_check_udp(lua_State* L, int index) {
  uv_udp_t* handle = luv_checkudata(L, index, "uv_udp");
  luaL_argcheck(L, handle->type == UV_UDP && handle->data, index, "Expected uv_udp_t");
  return handle;
}

static int luv_new_udp(lua_State* L) {
  uv_udp_t* handle = luv_newuserdata(L, sizeof(*handle));
  int ret = uv_udp_init(luv_loop(L), handle);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  handle->data = luv_setup_handle(L);
  return 1;
}

static int luv_udp_open(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  uv_os_sock_t sock = luaL_checkinteger(L, 2);
  int ret = uv_udp_open(handle, sock);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_udp_bind(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  const char* host = luaL_checkstring(L, 2);
  int port = luaL_checkinteger(L, 3);
  unsigned int flags = 0;
  struct sockaddr_storage addr;
  int ret;
  if (uv_ip4_addr(host, port, (struct sockaddr_in*)&addr) &&
      uv_ip6_addr(host, port, (struct sockaddr_in6*)&addr)) {
    return luaL_error(L, "Invalid IP address or port [%s:%d]", host, port);
  }
  if (lua_type(L, 4) == LUA_TTABLE) {
    luaL_checktype(L, 4, LUA_TTABLE);
    lua_getfield(L, 4, "reuseaddr");
    if (lua_toboolean(L, -1)) flags |= UV_UDP_REUSEADDR;
    lua_pop(L, 1);
    lua_getfield(L, 4, "ipv6only");
    if (lua_toboolean(L, -1)) flags |= UV_UDP_IPV6ONLY;
    lua_pop(L, 1);
  }
  ret = uv_udp_bind(handle, (struct sockaddr*)&addr, flags);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_udp_getsockname(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  struct sockaddr_storage address;
  int addrlen = sizeof(address);
  int ret = uv_udp_getsockname(handle, (struct sockaddr*)&address, &addrlen);
  if (ret < 0) return luv_error(L, ret);
  parse_sockaddr(L, &address, addrlen);
  return 1;
}

// These are the same order as uv_membership which also starts at 0
static const char *const luv_membership_opts[] = {
  "leave", "join", NULL
};

static int luv_udp_set_membership(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  const char* multicast_addr = luaL_checkstring(L, 2);
  const char* interface_addr = luaL_checkstring(L, 3);
  uv_membership membership = luaL_checkoption(L, 4, NULL, luv_membership_opts);
  int ret = uv_udp_set_membership(handle, multicast_addr, interface_addr, membership);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_udp_set_multicast_loop(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  int on, ret;
  luaL_checktype(L, 2, LUA_TBOOLEAN);
  on = lua_toboolean(L, 2);
  ret = uv_udp_set_multicast_loop(handle, on);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_udp_set_multicast_ttl(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  int ttl, ret;
  ttl = luaL_checkinteger(L, 2);
  ret = uv_udp_set_multicast_ttl(handle, ttl);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_udp_set_multicast_interface(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  const char* interface_addr = luaL_checkstring(L, 2);
  int ret = uv_udp_set_multicast_interface(handle, interface_addr);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_udp_set_broadcast(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  int on, ret;
  luaL_checktype(L, 2, LUA_TBOOLEAN);
  on = lua_toboolean(L, 2);
  ret =uv_udp_set_broadcast(handle, on);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_udp_set_ttl(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  int ttl, ret;
  ttl = luaL_checknumber(L, 2);
  ret = uv_udp_set_ttl(handle, ttl);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static void luv_udp_send_cb(uv_udp_send_t* req, int status) {
  lua_State* L = luv_state(req->handle->loop);
  luv_status(L, status);
  luv_fulfill_req(L, req->data, 1);
  luv_cleanup_req(L, req->data);
  req->data = NULL;
}

static int luv_udp_send(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  uv_udp_send_t* req;
  uv_buf_t buf;
  int ret, port, ref;
  const char* host;
  struct sockaddr_storage addr;
  luv_check_buf(L, 2, &buf);
  host = luaL_checkstring(L, 3);
  port = luaL_checkinteger(L, 4);
  if (uv_ip4_addr(host, port, (struct sockaddr_in*)&addr) &&
      uv_ip6_addr(host, port, (struct sockaddr_in6*)&addr)) {
    return luaL_error(L, "Invalid IP address or port [%s:%d]", host, port);
  }
  ref = luv_check_continuation(L, 5);
  req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  ret = uv_udp_send(req, handle, &buf, 1, (struct sockaddr*)&addr, luv_udp_send_cb);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  return 1;

}

static int luv_udp_try_send(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  uv_buf_t buf;
  int ret, port;
  const char* host;
  struct sockaddr_storage addr;
  luv_check_buf(L, 2, &buf);
  host = luaL_checkstring(L, 3);
  port = luaL_checkinteger(L, 4);
  if (uv_ip4_addr(host, port, (struct sockaddr_in*)&addr) &&
      uv_ip6_addr(host, port, (struct sockaddr_in6*)&addr)) {
    return luaL_error(L, "Invalid IP address or port [%s:%d]", host, port);
  }
  ret = uv_udp_try_send(handle, &buf, 1, (struct sockaddr*)&addr);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static void luv_udp_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
  lua_State* L = luv_state(handle->loop);

  // err
  if (nread < 0) {
    luv_status(L, nread);
  }
  else {
    lua_pushnil(L);
  }

  // data
  if (nread == 0) {
    if (addr) {
      lua_pushstring(L, "");
    }
    else {
      lua_pushnil(L);
    }
  }
  else if (nread > 0) {
    lua_pushlstring(L, buf->base, nread);
  }
  if (buf) free(buf->base);

  // address
  if (addr) {
    parse_sockaddr(L, (struct sockaddr_storage*)addr, sizeof *addr);
  }
  else {
    lua_pushnil(L);
  }

  // flags
  lua_newtable(L);
  if (flags & UV_UDP_PARTIAL) {
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "partial");
  }

  luv_call_callback(L, handle->data, LUV_RECV, 4);
}

static int luv_udp_recv_start(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  int ret;
  luv_check_callback(L, handle->data, LUV_RECV, 2);
  ret = uv_udp_recv_start(handle, luv_alloc_cb, luv_udp_recv_cb);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_udp_recv_stop(lua_State* L) {
  uv_udp_t* handle = luv_check_udp(L, 1);
  int ret = uv_udp_recv_stop(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}
