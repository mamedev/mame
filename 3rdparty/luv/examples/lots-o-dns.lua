local p = require('lib/utils').prettyPrint
local uv = require('luv')

uv.getaddrinfo(nil, 80, nil, p)

local domains = {
  "facebook.com",
  "google.com",
  "mail.google.com",
  "maps.google.com",
  "plus.google.com",
  "play.google.com",
  "apple.com",
  "hp.com",
  "yahoo.com",
  "mozilla.com",
  "developer.mozilla.com",
  "luvit.io",
  "creationix.com",
  "howtonode.org",
  "github.com",
  "gist.github.com"
}

local i = 1
local function next()
  uv.getaddrinfo(domains[i], nil, {
    v4mapped = true,
    all = true,
    addrconfig = true,
    canonname = true,
    numericserv = true,
    socktype = "STREAM"
  }, function (err, data)
    assert(not err, err)
    p(data)
    i = i + 1
    if i <= #domains then
      next()
    end
  end)
end
next();

repeat
  print("\nTick..")
until uv.run('once') == 0

print("done")
