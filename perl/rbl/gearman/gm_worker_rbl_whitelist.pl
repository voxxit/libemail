#!/usr/bin/perl
use strict;

use Gearman::Worker;
use IO::All;
use Net::CIDR::Lite;
use Fcntl qw( SEEK_CUR SEEK_END SEEK_SET );
use Data::Dumper;
use FreezeThaw qw(freeze thaw cmpStr safeFreeze cmpStrHard);
use Time::HiRes qw(usleep ualarm gettimeofday tv_interval);

use constant WHITELIST => 'whitelist.cidr';

# my $ua = LWP::UserAgent->new;
# $ua->max_redirect( 0 );

my $worker = Gearman::Worker->new;
$worker->job_servers( '127.0.0.1' );
$worker->register_function( "rbl_whitelist" => sub {
	my $serialized = $_[0]->arg;
	my ($h_ref) = thaw $serialized;
	my $t0 = [gettimeofday];

	# Open up the whitelist fresh
	my $whitelist = Net::CIDR::Lite->new;
	my @whitelist_lines = io( WHITELIST )->slurp;
	for( @whitelist_lines ){
		chomp;
		$whitelist->add( $_ );
	}
	$whitelist->prep_find();	

	my ( $inputfile, $p ) = ( $h_ref->{inputfile}, $h_ref->{startstop} );
	my $filename;
	open FD, "<$inputfile" or die;
	seek( FD, $p->[0], SEEK_SET );
	my $current_position = tell( FD );

	$filename = sprintf 'out.%.15d',  $p->[1];

	open FO, ">$filename";

	until( $current_position >= $p->[1] ){
		my $line = <FD>;
		chomp( $line );
		my $ip = $1 if $line =~ /(\d+\.\d+\.\d+\.\d+)/;
		# print non IP lines as well
		print FO unless $ip;
		if( $whitelist->find( $ip ) ){
			print STDERR "Found $line in whitelist\n";
		} else {
			print FO "$line\n";
		}
		$current_position = tell( FD );
	}
	close FD;
	close FO;

	my $t1 = [gettimeofday];
	my $t0_t1 = tv_interval $t0, $t1;
	print " $t0_t1 s taken\n";

        return $filename;
	},
	timeout => 5,);

$worker->work while 1;

