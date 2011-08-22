var console = require( 'console' );

var connection = exports;

function Connection( client ){
	this.client = client;
	this.current_data = '';
	this.values = new Array();
	setupClient( this );
}

function setupClient( self ){
	self.client.on( 'data', function( data ){
		console.log("data");
		self.process_line( data );
	});
}

Connection.prototype.process_data = function(){
	console.log("Processing all of the data...");
	// Do something clever..
	console.log( this.values );
	this.client.end();
};

Connection.prototype.process_line = function (data) {
	this.current_data += data;

	if( /^\s*$/.test( data ) ){
		console.log( "End of data" );
		this.process_data();
	} else {
		var m = /(\S+?)=(.*)/.exec( data );
		if( m ){
			this.values[ m[1] ] = m[2];
		}
		else {
			console.log( "Invalid input:" + data );
			this.client.write( "DUNNO\r\n" );
			this.client.end();
		}
	}
};

exports.Connection = Connection;

exports.createConnection = function(client) {
	var s = new Connection(client);
	return s;
}

