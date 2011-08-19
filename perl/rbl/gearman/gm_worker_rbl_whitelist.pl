#!/usr/bin/perl
use strict;

use Gearman::Worker;
use IO::All;
use Net::CIDR::Lite;
use Fcntl qw( SEEK_CUR SEEK_END SEEK_SET );
use Data::Dumper;
use FreezeThaw qw(freeze thaw cmpStr safeFreeze cmpStrHard);
use Time::HiRes qw(usleep ualarm gettimeofday tv_interval);

use Sys::Syslog;

use constant WHITELIST => '/home/pblair/rbl-gearman/whitelist.cidr';
use constant ALARMTIME => 5;

our $running  = 0;
our $lastrun  = time ();
our $startrun = time ();
my $lease = shift || 30;

# Register an alarm for 30 seconds from now, that will check if we have
# re-run
$SIG{ALRM} = sub {
    my $now   = time ();
    my $delta = ( $now - $lastrun );

    unless ( $running ) {
        if ( $delta > $lease ) {
            syslog( "info", "No jobs run for $delta seconds" );
            die ( "No jobs run for $delta seconds\n" );
        }
    } ## end unless ( $running )

    alarm ALARMTIME;
};    ## end sub alarm_handler


alarm ALARMTIME;

###
### End of Alarm stuff
###

openlog( "gm-worker-rbl", "ndelay,pid", "local0" );
syslog( "info", "started" );

my $worker = Gearman::Worker->new;
$worker->job_servers( '127.0.0.1' );
$worker->register_function(
    "rbl_whitelist" => sub {

        # Register now as the last time that we did anything
        $running++;
        $lastrun = time ();

        syslog( "info",
                "Processing request under worker function [rbl_whitelist]" );

        my $serialized = $_[0]->arg;
        my ( $h_ref )  = thaw $serialized;
        my $t0         = [gettimeofday];

        # Open up the whitelist fresh
        my $whitelist       = Net::CIDR::Lite->new;
        my @whitelist_lines = io( WHITELIST )->slurp;
        for ( @whitelist_lines ) {
            chomp;
            $whitelist->add( $_ );
        }
        $whitelist->prep_find();

        my ( $inputfile, $p ) = ( $h_ref->{inputfile}, $h_ref->{startstop} );
        my $filename;
        open FD, "</home/pblair/rbl-gearman/$inputfile" or die;
        seek ( FD, $p->[0], SEEK_SET );
        my $current_position = tell ( FD );

        $filename = sprintf '/home/pblair/rbl-gearman/out.%.15d', $p->[1];

        open FO, ">$filename";

        until ( $current_position >= $p->[1] ) {
            my $line = <FD>;
            chomp ( $line );
            my $ip = $1 if $line =~ /(\d+\.\d+\.\d+\.\d+)/;

            # print non IP lines as well
            print FO unless $ip;
            if ( $whitelist->find( $ip ) ) {
                syslog( "info", "Found $line in whitelist" );
            } else {
                print FO "$line\n";
            }
            $current_position = tell ( FD );
        } ## end until ( $current_position...)
        close FD;
        close FO;

        my $t1 = [gettimeofday];
        my $t0_t1 = tv_interval $t0, $t1;
        syslog( "info", "Job complete: $t0_t1 s taken");

        $running--;

        return $filename;
    },
    timeout => 5, );

$worker->work while 1;

# vim: noai:ts=4:sw=4:nu
