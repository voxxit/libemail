#!/usr/bin/env node

var server = require( './server' );
var logger = require( 'console' );

console.log( "starting application" );

server.createServer();
