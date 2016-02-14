local exports = {}
exports.app = require('weblit/app')
exports.autoHeaders = require('weblit/auto-headers')
exports.etagCache = require('weblit/etag-cache')
exports.logger = require('weblit/logger')
exports.static = require('weblit/static')
exports.websocket = require('weblit/websocket')
return exports
