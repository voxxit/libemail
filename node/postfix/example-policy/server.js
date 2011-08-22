var net  = require( "net" ),
    conn = require( "./connection" ),
    console = require( 'console' );
//var cluster = require( 'cluster' );

var Server = exports;

Server.createServer = function(){

	var server = net.createServer();

	server.listen( 1337, '127.0.0.1' );

	server.on('connection', function(client) {
		conn.createConnection(client);
	});
};
