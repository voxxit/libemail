#!/usr/bin/perl
use strict;
use Gearman::Client;
use Net::CIDR::Lite;
use Fcntl qw( SEEK_CUR SEEK_END SEEK_SET );
use FreezeThaw qw(freeze thaw cmpStr safeFreeze cmpStrHard);
use Sys::Syslog;
use Data::Dumper;
use constant BUCKETS => 50;

# This process takes a filehandle, and a position, nd walks back 
# until it finds  the begining of a new line, then returns that location
sub get_positions ( $$ ){
	my ($fh,$pos) = @_;

	return 0 if $pos == 0;

	my $newpos = sysseek( $fh, $pos, SEEK_SET );
	
	my $buf;
	until( $buf eq "\n" ){
		$newpos = sysseek( $fh, ($newpos -1), SEEK_SET );
		sysread( $fh, $buf, 1 ); 
		return ($newpos+1) if ( $buf eq "\n" );
	}
}

my $input = shift || die;
my $output = shift || die;

openlog( "gm-client-rbl", "ndelay,pid", "local0" );

open FD, "<$input" or die "Cannot open $input: $!\n";

my $length = sysseek( FD, 0, SEEK_END );

die( "Length smaller than number of buckets\n" ) if ( $length < BUCKETS );

my $bucket_size = int( int( $length ) / int( BUCKETS ) );
my $position = 0;
my @positions;
push( @positions, 0 );

for ( my $i = 0; $i < ( BUCKETS - 1) ; $i++ ){
	my $start = get_positions( \*FD, $position );
	push( @positions, $start ) unless $start == 0;
	$position += $bucket_size;
}

push( @positions, $length );
close FD;

my @P;
while( scalar @positions > 1 ){
	my $head = shift( @positions );
	push( @P, [ $head, $positions[ 0 ] ] );
}

my $client = Gearman::Client->new;
my @jobservers = ( '127.0.0.1' );
syslog( "info", "Attaching to job_servers: @jobservers" );
$client->job_servers( @jobservers );

my $taskset = $client->new_task_set;

my @completed_filenames;
for( @P ){
	# Place these into a hash ref
	my $h_ref = { startstop => $_, inputfile => $input };

	my $frozen = freeze $h_ref;
	syslog( "info", "Adding task [rbl_whitelist]" );
        $taskset->add_task( "rbl_whitelist" => $frozen, {
                on_complete => sub { 
			#print ${ $_[0] }, "\n" ;
			push( @completed_filenames, ${$_[0]} );
		}
        });
}

$taskset->wait;

# Concat all of the files
syslog( "info", "Concatenating files to $output" );
open FD, ">$output" || die;
for my $input ( @completed_filenames ){
	print "Opening $input for concatenation\n";
	open INPUT, "<$input";
	while( <INPUT> ){
		print FD $_;
	}
	close INPUT;
	unlink $input;
}

close FD;
