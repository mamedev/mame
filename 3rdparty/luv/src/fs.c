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

static uv_fs_t* luv_check_fs(lua_State* L, int index) {
  uv_fs_t* req = luaL_checkudata(L, index, "uv_req");
  luaL_argcheck(L, req->type = UV_FS && req->data, index, "Expected uv_fs_t");
  return req;
}

static void luv_push_timespec_table(lua_State* L, const uv_timespec_t* t) {
  lua_createtable(L, 0, 2);
  lua_pushinteger(L, t->tv_sec);
  lua_setfield(L, -2, "sec");
  lua_pushinteger(L, t->tv_nsec);
  lua_setfield(L, -2, "nsec");
}

static void luv_push_stats_table(lua_State* L, const uv_stat_t* s) {
  const char* type = NULL;
  lua_createtable(L, 0, 23);
  lua_pushinteger(L, s->st_dev);
  lua_setfield(L, -2, "dev");
  lua_pushinteger(L, s->st_mode);
  lua_setfield(L, -2, "mode");
  lua_pushinteger(L, s->st_nlink);
  lua_setfield(L, -2, "nlink");
  lua_pushinteger(L, s->st_uid);
  lua_setfield(L, -2, "uid");
  lua_pushinteger(L, s->st_gid);
  lua_setfield(L, -2, "gid");
  lua_pushinteger(L, s->st_rdev);
  lua_setfield(L, -2, "rdev");
  lua_pushinteger(L, s->st_ino);
  lua_setfield(L, -2, "ino");
  lua_pushinteger(L, s->st_size);
  lua_setfield(L, -2, "size");
  lua_pushinteger(L, s->st_blksize);
  lua_setfield(L, -2, "blksize");
  lua_pushinteger(L, s->st_blocks);
  lua_setfield(L, -2, "blocks");
  lua_pushinteger(L, s->st_flags);
  lua_setfield(L, -2, "flags");
  lua_pushinteger(L, s->st_gen);
  lua_setfield(L, -2, "gen");
  luv_push_timespec_table(L, &s->st_atim);
  lua_setfield(L, -2, "atime");
  luv_push_timespec_table(L, &s->st_mtim);
  lua_setfield(L, -2, "mtime");
  luv_push_timespec_table(L, &s->st_ctim);
  lua_setfield(L, -2, "ctime");
  luv_push_timespec_table(L, &s->st_birthtim);
  lua_setfield(L, -2, "birthtime");
  if (S_ISREG(s->st_mode)) {
    type = "file";
  }
  else if (S_ISDIR(s->st_mode)) {
    type = "directory";
  }
  else if (S_ISLNK(s->st_mode)) {
    type = "link";
  }
  else if (S_ISFIFO(s->st_mode)) {
    type = "fifo";
  }
#ifdef S_ISSOCK
  else if (S_ISSOCK(s->st_mode)) {
    type = "socket";
  }
#endif
  else if (S_ISCHR(s->st_mode)) {
    type = "char";
  }
  else if (S_ISBLK(s->st_mode)) {
    type = "block";
  }
  if (type) {
    lua_pushstring(L, type);
    lua_setfield(L, -2, "type");
  }
}

static int luv_check_flags(lua_State* L, int index) {
  const char* string;
  if (lua_isnumber(L, index)) {
    return lua_tointeger(L, index);
  }
  else if (!lua_isstring(L, index)) {
    return luaL_argerror(L, index, "Expected string or integer for file open mode");
  }
  string = lua_tostring(L, index);

  if (strcmp(string, "r")   == 0) return O_RDONLY;
#ifdef O_SYNC
  if (strcmp(string, "rs")  == 0 ||
      strcmp(string, "sr")  == 0) return O_RDONLY | O_SYNC;
#endif
  if (strcmp(string, "r+")  == 0) return O_RDWR;
#ifdef O_SYNC
  if (strcmp(string, "rs+") == 0 ||
      strcmp(string, "sr+") == 0) return O_RDWR   | O_SYNC;
#endif
  if (strcmp(string, "w")   == 0) return O_TRUNC  | O_CREAT | O_WRONLY;
  if (strcmp(string, "wx")  == 0 ||
      strcmp(string, "xw")  == 0) return O_TRUNC  | O_CREAT | O_WRONLY | O_EXCL;
  if (strcmp(string, "w+")  == 0) return O_TRUNC  | O_CREAT | O_RDWR;
  if (strcmp(string, "wx+") == 0 ||
      strcmp(string, "xw+") == 0) return O_TRUNC  | O_CREAT | O_RDWR   | O_EXCL;
  if (strcmp(string, "a")   == 0) return O_APPEND | O_CREAT | O_WRONLY;
  if (strcmp(string, "ax")  == 0 ||
      strcmp(string, "xa")  == 0) return O_APPEND | O_CREAT | O_WRONLY | O_EXCL;
  if (strcmp(string, "a+")  == 0) return O_APPEND | O_CREAT | O_RDWR;
  if (strcmp(string, "ax+") == 0 ||
      strcmp(string, "xa+") == 0) return O_APPEND | O_CREAT | O_RDWR   | O_EXCL;

  return luaL_error(L, "Unknown file open flag '%s'", string);
}

static int luv_check_amode(lua_State* L, int index) {
  size_t i;
  int mode;
  const char* string;
  if (lua_isnumber(L, index)) {
    return lua_tointeger(L, index);
  }
  else if (!lua_isstring(L, index)) {
    return luaL_argerror(L, index, "Expected string or integer for file access mode check");
  }
  string = lua_tostring(L, index);
  mode = 0;
  for (i = 0; i < strlen(string); ++i) {
    switch (string[i]) {
      case 'r': case 'R':
        mode |= R_OK;
        break;
      case 'w': case 'W':
        mode |= W_OK;
        break;
      case 'x': case 'X':
        mode |= X_OK;
        break;
      default:
        return luaL_argerror(L, index, "Unknown character in access mode string");
    }
  }
  return mode;
}

/* Processes a result and pushes the data onto the stack
   returns the number of items pushed */
static int push_fs_result(lua_State* L, uv_fs_t* req) {
  luv_req_t* data = req->data;

  if (req->fs_type == UV_FS_ACCESS) {
    lua_pushboolean(L, req->result >= 0);
    return 1;
  }

  if (req->result < 0) {
    lua_pushnil(L);
    if (req->path) {
      lua_pushfstring(L, "%s: %s: %s", uv_err_name(req->result), uv_strerror(req->result), req->path);
    }
    else {
      lua_pushfstring(L, "%s: %s", uv_err_name(req->result), uv_strerror(req->result));
    }
    return 2;
  }

  switch (req->fs_type) {
    case UV_FS_CLOSE:
    case UV_FS_RENAME:
    case UV_FS_UNLINK:
    case UV_FS_RMDIR:
    case UV_FS_MKDIR:
    case UV_FS_FTRUNCATE:
    case UV_FS_FSYNC:
    case UV_FS_FDATASYNC:
    case UV_FS_LINK:
    case UV_FS_SYMLINK:
    case UV_FS_CHMOD:
    case UV_FS_FCHMOD:
    case UV_FS_CHOWN:
    case UV_FS_FCHOWN:
    case UV_FS_UTIME:
    case UV_FS_FUTIME:
      lua_pushboolean(L, 1);
      return 1;

    case UV_FS_OPEN:
    case UV_FS_SENDFILE:
    case UV_FS_WRITE:
      lua_pushinteger(L, req->result);
      return 1;

    case UV_FS_STAT:
    case UV_FS_LSTAT:
    case UV_FS_FSTAT:
      luv_push_stats_table(L, &req->statbuf);
      return 1;

    case UV_FS_MKDTEMP:
      lua_pushstring(L, req->path);
      return 1;

    case UV_FS_READLINK:
    case UV_FS_REALPATH:
      lua_pushstring(L, (char*)req->ptr);
      return 1;

    case UV_FS_READ:
      lua_pushlstring(L, data->data, req->result);
      return 1;

    case UV_FS_SCANDIR:
      // Expose the userdata for the request.
      lua_rawgeti(L, LUA_REGISTRYINDEX, data->req_ref);
      return 1;

    default:
      lua_pushnil(L);
      lua_pushfstring(L, "UNKNOWN FS TYPE %d\n", req->fs_type);
      return 2;
  }

}

static void luv_fs_cb(uv_fs_t* req) {
  lua_State* L = luv_state(req->loop);

  int nargs = push_fs_result(L, req);
  if (nargs == 2 && lua_isnil(L, -nargs)) {
    // If it was an error, convert to (err, value) format.
    lua_remove(L, -nargs);
    nargs--;
  }
  else {
    // Otherwise insert a nil in front to convert to (err, value) format.
    lua_pushnil(L);
    lua_insert(L, -nargs - 1);
    nargs++;
  }
  luv_fulfill_req(L, req->data, nargs);
  if (req->fs_type != UV_FS_SCANDIR) {
    luv_cleanup_req(L, req->data);
    req->data = NULL;
    uv_fs_req_cleanup(req);
  }
}

#define FS_CALL(func, req, ...) {                         \
  int ret, sync;                                          \
  luv_req_t* data = req->data;                            \
  sync = data->callback_ref == LUA_NOREF;                 \
  ret = uv_fs_##func(luv_loop(L), req, __VA_ARGS__,       \
                     sync ? NULL : luv_fs_cb);            \
  if (req->fs_type != UV_FS_ACCESS && ret < 0) {          \
    lua_pushnil(L);                                       \
    if (req->path) {                                      \
      lua_pushfstring(L, "%s: %s: %s", uv_err_name(req->result), uv_strerror(req->result), req->path); \
    }                                                     \
    else {                                                \
      lua_pushfstring(L, "%s: %s", uv_err_name(req->result), uv_strerror(req->result)); \
    }                                                     \
    lua_pushstring(L, uv_err_name(req->result));          \
    luv_cleanup_req(L, req->data);                        \
    req->data = NULL;                                     \
    uv_fs_req_cleanup(req);                               \
    return 3;                                             \
  }                                                       \
  if (sync) {                                             \
    int nargs = push_fs_result(L, req);                   \
    if (req->fs_type != UV_FS_SCANDIR) {                  \
      luv_cleanup_req(L, req->data);                      \
      req->data = NULL;                                   \
      uv_fs_req_cleanup(req);                             \
    }                                                     \
    return nargs;                                         \
  }                                                       \
  lua_rawgeti(L, LUA_REGISTRYINDEX, data->req_ref);       \
  return 1;                                               \
}

static int luv_fs_close(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(close, req, file);
}

static int luv_fs_open(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int flags = luv_check_flags(L, 2);
  int mode = luaL_checkinteger(L, 3);
  int ref = luv_check_continuation(L, 4);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(open, req, path, flags, mode);
}

static int luv_fs_read(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  int64_t len = luaL_checkinteger(L, 2);
  int64_t offset = luaL_checkinteger(L, 3);
  uv_buf_t buf;
  int ref;
  uv_fs_t* req;
  char* data = malloc(len);
  if (!data) return luaL_error(L, "Failure to allocate buffer");
  buf = uv_buf_init(data, len);
  ref = luv_check_continuation(L, 4);
  req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  // TODO: find out why we can't just use req->ptr for the base
  ((luv_req_t*)req->data)->data = buf.base;
  FS_CALL(read, req, file, &buf, 1, offset);
}

static int luv_fs_unlink(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(unlink, req, path);
}

static int luv_fs_write(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  uv_buf_t buf;
  int64_t offset;
  int ref;
  uv_fs_t* req;
  size_t count;
  uv_buf_t *bufs = NULL;

  if (lua_istable(L, 2)) {
    bufs = luv_prep_bufs(L, 2, &count);
    buf.base = NULL;
  }
  else if (lua_isstring(L, 2)) {
    luv_check_buf(L, 2, &buf);
    count = 1;
  }
  else {
    return luaL_argerror(L, 2, "data must be string or table of strings");
  }

  offset = luaL_checkinteger(L, 3);
  ref = luv_check_continuation(L, 4);
  req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  req->ptr = buf.base;
  ((luv_req_t*)req->data)->data = bufs;
  FS_CALL(write, req, file, bufs ? bufs : &buf, count, offset);
}

static int luv_fs_mkdir(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int mode = luaL_checkinteger(L, 2);
  int ref = luv_check_continuation(L, 3);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(mkdir, req, path, mode);
}

static int luv_fs_mkdtemp(lua_State* L) {
  const char* tpl = luaL_checkstring(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(mkdtemp, req, tpl);
}

static int luv_fs_rmdir(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(rmdir, req, path);
}

static int luv_fs_scandir(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int flags = 0; // TODO: find out what these flags are.
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(scandir, req, path, flags);
}

static int luv_fs_scandir_next(lua_State* L) {
  uv_fs_t* req = luv_check_fs(L, 1);
  uv_dirent_t ent;
  int ret = uv_fs_scandir_next(req, &ent);
  const char* type;
  if (ret == UV_EOF) {
    luv_cleanup_req(L, req->data);
    req->data = NULL;
    uv_fs_req_cleanup(req);
    return 0;
  }
  if (ret < 0) return luv_error(L, ret);
  lua_pushstring(L, ent.name);
  switch (ent.type) {
    case UV_DIRENT_UNKNOWN: return 1;
    case UV_DIRENT_FILE:    type = "file"; break;
    case UV_DIRENT_DIR:     type = "directory"; break;
    case UV_DIRENT_LINK:    type = "link"; break;
    case UV_DIRENT_FIFO:    type = "fifo"; break;
    case UV_DIRENT_SOCKET:  type = "socket"; break;
    case UV_DIRENT_CHAR:    type = "char"; break;
    case UV_DIRENT_BLOCK:   type = "block"; break;
    default: assert(0);
  }
  lua_pushstring(L, type);
  return 2;
}

static int luv_fs_stat(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(stat, req, path);
}

static int luv_fs_fstat(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(fstat, req, file);
}

static int luv_fs_lstat(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(lstat, req, path);
}

static int luv_fs_rename(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  const char* new_path = luaL_checkstring(L, 2);
  int ref = luv_check_continuation(L, 3);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(rename, req, path, new_path);
}

static int luv_fs_fsync(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(fsync, req, file);
}

static int luv_fs_fdatasync(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(fdatasync, req, file);
}

static int luv_fs_ftruncate(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  int64_t offset = luaL_checkinteger(L, 2);
  int ref = luv_check_continuation(L, 3);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(ftruncate, req, file, offset);
}

static int luv_fs_sendfile(lua_State* L) {
  uv_file out_fd = luaL_checkinteger(L, 1);
  uv_file in_fd = luaL_checkinteger(L, 2);
  int64_t in_offset = luaL_checkinteger(L, 3);
  size_t length = luaL_checkinteger(L, 4);
  int ref = luv_check_continuation(L, 5);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(sendfile, req, out_fd, in_fd, in_offset, length);
}

static int luv_fs_access(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int amode = luv_check_amode(L, 2);
  int ref = luv_check_continuation(L, 3);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(access, req, path, amode);
}

static int luv_fs_chmod(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int mode = luaL_checkinteger(L, 2);
  int ref = luv_check_continuation(L, 3);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(chmod, req, path, mode);
}

static int luv_fs_fchmod(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  int mode = luaL_checkinteger(L, 2);
  int ref = luv_check_continuation(L, 3);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(fchmod, req, file, mode);
}

static int luv_fs_utime(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  double atime = luaL_checknumber(L, 2);
  double mtime = luaL_checknumber(L, 3);
  int ref = luv_check_continuation(L, 4);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(utime, req, path, atime, mtime);
}

static int luv_fs_futime(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  double atime = luaL_checknumber(L, 2);
  double mtime = luaL_checknumber(L, 3);
  int ref = luv_check_continuation(L, 4);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(futime, req, file, atime, mtime);
}

static int luv_fs_link(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  const char* new_path = luaL_checkstring(L, 2);
  int ref = luv_check_continuation(L, 3);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(link, req, path, new_path);
}

static int luv_fs_symlink(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  const char* new_path = luaL_checkstring(L, 2);
  int flags = 0, ref;
  uv_fs_t* req;
  if (lua_type(L, 3) == LUA_TTABLE) {
    lua_getfield(L, 3, "dir");
    if (lua_toboolean(L, -1)) flags |= UV_FS_SYMLINK_DIR;
    lua_pop(L, 1);
    lua_getfield(L, 3, "junction");
    if (lua_toboolean(L, -1)) flags |= UV_FS_SYMLINK_JUNCTION;
    lua_pop(L, 1);
  }
  ref = luv_check_continuation(L, 4);
  req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);

  FS_CALL(symlink, req, path, new_path, flags);
}

static int luv_fs_readlink(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(readlink, req, path);
}

static int luv_fs_realpath(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  int ref = luv_check_continuation(L, 2);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(realpath, req, path);
}

static int luv_fs_chown(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  uv_uid_t uid = luaL_checkinteger(L, 2);
  uv_uid_t gid = luaL_checkinteger(L, 3);
  int ref = luv_check_continuation(L, 4);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(chown, req, path, uid, gid);
}

static int luv_fs_fchown(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  uv_uid_t uid = luaL_checkinteger(L, 2);
  uv_uid_t gid = luaL_checkinteger(L, 3);
  int ref = luv_check_continuation(L, 4);
  uv_fs_t* req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);
  FS_CALL(fchown, req, file, uid, gid);
}
