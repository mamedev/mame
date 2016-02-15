local exports = {}
exports.name = "creationix/coro-fs"
exports.version = "1.3.0"
exports.homepage = "https://github.com/luvit/lit/blob/master/deps/coro-fs.lua"
exports.description = "A coro style interface to the filesystem."
exports.tags = {"coro", "fs"}
exports.license = "MIT"
exports.author = { name = "Tim Caswell" }

local uv = require('luv')
local fs = exports
local pathJoin = require('path').join

local function noop() end

local function makeCallback()
  local thread = coroutine.running()
  return function (err, value, ...)
    if err then
      assert(coroutine.resume(thread, nil, err))
    else
      assert(coroutine.resume(thread, value == nil and true or value, ...))
    end
  end
end

function fs.mkdir(path, mode)
  uv.fs_mkdir(path, mode or 511, makeCallback())
  return coroutine.yield()
end
function fs.open(path, flags, mode)
  uv.fs_open(path, flags or "r", mode or 438, makeCallback())
  return coroutine.yield()
end
function fs.unlink(path)
  uv.fs_unlink(path, makeCallback())
  return coroutine.yield()
end
function fs.stat(path)
  uv.fs_stat(path, makeCallback())
  return coroutine.yield()
end
function fs.lstat(path)
  uv.fs_lstat(path, makeCallback())
  return coroutine.yield()
end
function fs.symlink(target, path)
  uv.fs_symlink(target, path, makeCallback())
  return coroutine.yield()
end
function fs.readlink(path)
  uv.fs_readlink(path, makeCallback())
  return coroutine.yield()
end
function fs.fstat(fd)
  uv.fs_fstat(fd, makeCallback())
  return coroutine.yield()
end
function fs.chmod(fd, path)
  uv.fs_chmod(fd, path, makeCallback())
  return coroutine.yield()
end
function fs.fchmod(fd, mode)
  uv.fs_fchmod(fd, mode, makeCallback())
  return coroutine.yield()
end
function fs.read(fd, length, offset)
  uv.fs_read(fd, length or 1024*48, offset or -1, makeCallback())
  return coroutine.yield()
end
function fs.write(fd, data, offset)
  uv.fs_write(fd, data, offset or -1, makeCallback())
  return coroutine.yield()
end
function fs.close(fd)
  uv.fs_close(fd, makeCallback())
  return coroutine.yield()
end
function fs.access(path, flags)
  uv.fs_access(path, flags or "", makeCallback())
  return coroutine.yield()
end
function fs.rename(path, newPath)
  uv.fs_rename(path, newPath, makeCallback())
  return coroutine.yield()
end
function fs.rmdir(path)
  uv.fs_rmdir(path, makeCallback())
  return coroutine.yield()
end
function fs.rmrf(path)
  local success, err
  success, err = fs.rmdir(path)
  if success then return success end
  if err:match("^ENOTDIR:") then return fs.unlink(path) end
  if not err:match("^ENOTEMPTY:") then return success, err end
  for entry in assert(fs.scandir(path)) do
    local subPath = pathJoin(path, entry.name)
    if entry.type == "directory" then
      success, err = fs.rmrf(pathJoin(path, entry.name))
    else
      success, err = fs.unlink(subPath)
    end
    if not success then return success, err end
  end
  return fs.rmdir(path)
end
function fs.scandir(path)
  uv.fs_scandir(path, makeCallback())
  local req, err = coroutine.yield()
  if not req then return nil, err end
  return function ()
    return uv.fs_scandir_next(req)
  end
end

function fs.readFile(path)
  local fd, stat, data, err
  fd, err = fs.open(path)
  if err then return nil, err end
  stat, err = fs.fstat(fd)
  if stat then
    data, err = fs.read(fd, stat.size)
  end
  uv.fs_close(fd, noop)
  return data, err
end

function fs.writeFile(path, data, mkdir)
  local fd, success, err
  fd, err = fs.open(path, "w")
  if err then
    if mkdir and string.match(err, "^ENOENT:") then
      success, err = fs.mkdirp(pathJoin(path, ".."))
      if success then return fs.writeFile(path, data) end
    end
    return nil, err
  end
  success, err = fs.write(fd, data)
  uv.fs_close(fd, noop)
  return success, err
end

function fs.mkdirp(path, mode)
  local success, err = fs.mkdir(path, mode)
  if success or string.match(err, "^EEXIST") then
    return true
  end
  if string.match(err, "^ENOENT:") then
    success, err = fs.mkdirp(pathJoin(path, ".."), mode)
    if not success then return nil, err end
    return fs.mkdir(path, mode)
  end
  return nil, err
end

function fs.chroot(base)
  local chroot = {
    base = base,
    fstat = fs.fstat,
    fchmod = fs.fchmod,
    read = fs.read,
    write = fs.write,
    close = fs.close,
  }
  local function resolve(path)
    assert(path, "path missing")
    return pathJoin(base, pathJoin(path))
  end
  function chroot.mkdir(path, mode)
    return fs.mkdir(resolve(path), mode)
  end
  function chroot.mkdirp(path, mode)
    return fs.mkdirp(resolve(path), mode)
  end
  function chroot.open(path, flags, mode)
    return fs.open(resolve(path), flags, mode)
  end
  function chroot.unlink(path)
    return fs.unlink(resolve(path))
  end
  function chroot.stat(path)
    return fs.stat(resolve(path))
  end
  function chroot.lstat(path)
    return fs.lstat(resolve(path))
  end
  function chroot.symlink(target, path)
    -- TODO: should we resolve absolute target paths or treat it as opaque data?
    return fs.symlink(target, resolve(path))
  end
  function chroot.readlink(path)
    return fs.readlink(resolve(path))
  end
  function chroot.chmod(path, mode)
    return fs.chmod(resolve(path), mode)
  end
  function chroot.access(path, flags)
    return fs.access(resolve(path), flags)
  end
  function chroot.rename(path, newPath)
    return fs.rename(resolve(path), resolve(newPath))
  end
  function chroot.rmdir(path)
    return fs.rmdir(resolve(path))
  end
  function chroot.rmrf(path)
    return fs.rmrf(resolve(path))
  end
  function chroot.scandir(path, iter)
    return fs.scandir(resolve(path), iter)
  end
  function chroot.readFile(path)
    return fs.readFile(resolve(path))
  end
  function chroot.writeFile(path, data, mkdir)
    return fs.writeFile(resolve(path), data, mkdir)
  end
  return chroot
end

return exports
