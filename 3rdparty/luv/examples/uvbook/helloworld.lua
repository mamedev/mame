local uv = require('luv')

print('Now quitting.')
uv.run('default')
uv.loop_close()
