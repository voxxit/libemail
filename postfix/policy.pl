#!/usr/bin/perl
#
use IO::Socket;
use threads;
use Proc::Daemon;
use Sys::Syslog qw( :DEFAULT setlogsock);
use Data::Dumper;

# Global config settings
my $TC = 15;
my $debug = 1;
our $pidfile = "/var/run/policy.pid";

# Param1: Client socket
# Param2: hash_ref
sub parse_postfix_input( $$ ) {
	my ($socket,$hashref) = @_;

	while( my $line = <$socket> ){
		return if $line =~ /^(\r|\n)*$/;
		#print "DEBUG: $line" if $debug;
		if( $line =~ /^(\w+?)=(.+)$/ ){
			$hashref->{$1} = $2;
		}
	}
}

sub process_policy_request( $ ){
	my ($href) = @_;

	# Do something with the href that we've consumed...
	return -1,'DUNNO';
}

sub process_client($){
	my ($socket) = @_;
	ACCEPT: while( my $client = $socket->accept() ){
		my $hash_ref = {};
		parse_postfix_input( $client, $hash_ref );

		#print "DEBUG: " . Dumper( $hash_ref ) . "\n";

		my $ret,$action = process_policy_request( $href );
		if( $action eq 'DISCARD' ){
			syslog('info', "DISCARDING: $hash_ref->{recipient} : $hash_ref->{sender}");
			print $client "action=discard\n\n";
			next ACCEPT;
		}
		print $client "action=dunno\n\n";
	}
}

sub handle_sig_int
{
	unlink( $pidfile );
	exit(0);
}

openlog('policy', '', 'mail');
syslog('info', 'launching in daemon mode') if $ARGV[0] eq 'quiet-quick-start';
Proc::Daemon::Init if $ARGV[0] eq 'quiet-quick-start';

# Attempt to parse in the redirect config

$SIG{INT} = \&handle_sig_int;

# Ignore client disconnects
$SIG{PIPE} = "IGNORE";

open PID, "+>", "$pidfile" or die("Cannot open $pidfile: $!\n");
print PID "$$";
close( PID );

$server = IO::Socket::INET->new(
    LocalPort => 1234,
    Type      => SOCK_STREAM,
    Reuse     => 1,
    Listen    => 10
  )
  or die
  "Couldn't be a tcp server on port $default_config->{serverport} : $@\n";

# Generate a number of listener threads
my @threads = ();
for( 1 .. $TC ){
	my $thread = threads->create( \&process_client, $server );
	push( @threads, $thread );
}

foreach my $thread ( @threads ){
	$thread->join();
}

unlink( $pidfile );
closelog;
exit( 0 );
