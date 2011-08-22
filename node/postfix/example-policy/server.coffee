net  = require "net"
conn = require "./connection"
# cluster = require "cluster"

class Server
  createServer: ->
    serv = net.createServer()
    serv.listen 1337, "127.0.0.1"
    
    serv.on "connection", (client) ->
      conn.createConnection(client)

exports = Server