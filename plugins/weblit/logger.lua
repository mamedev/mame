
return function (req, res, go)
  -- Skip this layer for clients who don't send User-Agent headers.
  local userAgent = req.headers["user-agent"]
  if not userAgent then return go() end
  -- Run all inner layers first.
  go()
  -- And then log after everything is done
  --print(string.format("%s %s %s %s", req.method,  req.path, userAgent, res.code))
end
