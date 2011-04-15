#!/usr/bin/perl
use strict;
use lib ( "." );
use Policy::Memcache;
use Data::Dumper;

my $mc = Policy::Memcache->new;
my $ip = shift;
my $cmd = shift || 'get';
chomp( $ip );

if( $cmd eq 'get' ){
	my ($val,$ss) = $mc->get( $ip );
	print "$ip:$ss => $val\n";
} elsif( $cmd eq 'increment' ){
	# Instantiate a new object, but with the overhead
	# of the DNS recursor
	$mc = Policy::Memcache->new( -policy => 'ss' );
	$mc->increment( $ip );
} else {
	die( "Unknown command: $cmd\n" );
}

exit 0;
