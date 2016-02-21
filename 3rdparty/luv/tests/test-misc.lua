return require('lib/tap')(function (test)

  test("uv.guess_handle", function (print, p, expect, uv)
    local types = {
      [0] = assert(uv.guess_handle(0)),
      assert(uv.guess_handle(1)),
      assert(uv.guess_handle(2)),
    }
    p("stdio fd types", types)
  end)

  test("uv.version and uv.version_string", function (print, p, expect, uv)
    local version = assert(uv.version())
    local version_string = assert(uv.version_string())
    p{version=version, version_string=version_string}
    assert(type(version) == "number")
    assert(type(version_string) == "string")
  end)

  test("memory size", function (print, p, expect, uv)
    local rss = uv.resident_set_memory()
    local total = uv.get_total_memory()
    local free = uv.get_free_memory()
    p{rss=rss,total=total,free=free}
    assert(rss < total)
  end)

  test("uv.uptime", function (print, p, expect, uv)
    local uptime = assert(uv.uptime())
    p{uptime=uptime}
  end)

  test("uv.getrusage", function (print, p, expect, uv)
    local rusage = assert(uv.getrusage())
    p(rusage)
  end)

  test("uv.cpu_info", function (print, p, expect, uv)
    local info = assert(uv.cpu_info())
    p(info)
  end)

  test("uv.interface_addresses", function (print, p, expect, uv)
    local addresses = assert(uv.interface_addresses())
    for name, info in pairs(addresses) do
      p(name, addresses[name])
    end
  end)

  test("uv.loadavg", function (print, p, expect, uv)
    local avg = {assert(uv.loadavg())}
    p(avg)
    assert(#avg == 3)
  end)

  test("uv.exepath", function (print, p, expect, uv)
    local path = assert(uv.exepath())
    p(path)
  end)

  test("uv.os_homedir", function (print, p, expect, uv)
    local path = assert(uv.os_homedir())
    p(path)
  end)

  test("uv.cwd and uv.chdir", function (print, p, expect, uv)
    local old = assert(uv.cwd())
    p(old)
    assert(uv.chdir("/"))
    local cwd = assert(uv.cwd())
    p(cwd)
    assert(cwd ~= old)
    assert(uv.chdir(old))
  end)

  test("uv.hrtime", function (print, p, expect, uv)
    local time = assert(uv.hrtime())
    p(time)
  end)

  test("test_getpid", function (print, p, expect, uv)
    assert(uv.getpid())
  end)

end)
