return require('lib/tap')(function (test)

  test("Get all local http addresses", function (print, p, expect, uv)
    assert(uv.getaddrinfo(nil, "http", nil, expect(function (err, res)
      p(res, #res)
      assert(not err, err)
      assert(res[1].port == 80)
    end)))
  end)

  test("Get all local http addresses sync", function (print, p, expect, uv)
    local res = assert(uv.getaddrinfo(nil, "http"))
    p(res, #res)
    assert(res[1].port == 80)
  end)

  test("Get only ipv4 tcp adresses for luvit.io", function (print, p, expect, uv)
    assert(uv.getaddrinfo("luvit.io", nil, {
      socktype = "stream",
      family = "inet",
    }, expect(function (err, res)
      assert(not err, err)
      p(res, #res)
      assert(#res == 1)
    end)))
  end)

  -- FIXME: this test always fails on AppVeyor for some reason
  if _G.isWindows and not os.getenv'APPVEYOR' then
    test("Get only ipv6 tcp adresses for luvit.io", function (print, p, expect, uv)
      assert(uv.getaddrinfo("luvit.io", nil, {
        socktype = "stream",
        family = "inet6",
      }, expect(function (err, res)
        assert(not err, err)
        p(res, #res)
        assert(#res == 1)
      end)))
    end)
  end

  test("Get ipv4 and ipv6 tcp adresses for luvit.io", function (print, p, expect, uv)
    assert(uv.getaddrinfo("luvit.io", nil, {
      socktype = "stream",
    }, expect(function (err, res)
      assert(not err, err)
      p(res, #res)
      assert(#res > 0)
    end)))
  end)

  test("Get all adresses for luvit.io", function (print, p, expect, uv)
    assert(uv.getaddrinfo("luvit.io", nil, nil, expect(function (err, res)
      assert(not err, err)
      p(res, #res)
      assert(#res > 0)
    end)))
  end)

  test("Lookup local ipv4 address", function (print, p, expect, uv)
    assert(uv.getnameinfo({
      family = "inet",
    }, expect(function (err, hostname, service)
      p{err=err,hostname=hostname,service=service}
      assert(not err, err)
      assert(hostname)
      assert(service)
    end)))
  end)

  test("Lookup local ipv4 address sync", function (print, p, expect, uv)
    local hostname, service = assert(uv.getnameinfo({
      family = "inet",
    }))
    p{hostname=hostname,service=service}
    assert(hostname)
    assert(service)
  end)

  test("Lookup local 127.0.0.1 ipv4 address", function (print, p, expect, uv)
    assert(uv.getnameinfo({
      ip = "127.0.0.1",
    }, expect(function (err, hostname, service)
      p{err=err,hostname=hostname,service=service}
      assert(not err, err)
      assert(hostname)
      assert(service)
    end)))
  end)

  test("Lookup local ipv6 address", function (print, p, expect, uv)
    assert(uv.getnameinfo({
      family = "inet6",
    }, expect(function (err, hostname, service)
      p{err=err,hostname=hostname,service=service}
      assert(not err, err)
      assert(hostname)
      assert(service)
    end)))
  end)

  test("Lookup local ::1 ipv6 address", function (print, p, expect, uv)
    assert(uv.getnameinfo({
      ip = "::1",
    }, expect(function (err, hostname, service)
      p{err=err,hostname=hostname,service=service}
      assert(not err, err)
      assert(hostname)
      assert(service)
    end)))
  end)

  test("Lookup local port 80 service", function (print, p, expect, uv)
    assert(uv.getnameinfo({
      port = 80,
      family = "inet6",
    }, expect(function (err, hostname, service)
      p{err=err,hostname=hostname,service=service}
      assert(not err, err)
      assert(hostname)
      assert(service == "http")
    end)))
  end)

end)
