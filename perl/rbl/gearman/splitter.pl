#!/usr/bin/perl
use strict;
use IO::All;
use Net::CIDR::Lite;
use Fcntl qw( SEEK_CUR SEEK_END SEEK_SET );
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

open FD, "<ips.txt" or die "Cannot open ips.txt: $!\n";

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

# Open the whitelist
my $whitelist = Net::CIDR::Lite->new;
my @whitelist_lines = io( 'whitelist.cidr' )->slurp;
for( @whitelist_lines ){
	chomp;
	print STDERR "Adding $_ to whitelist\n";
	$whitelist->add( $_ );
}
$whitelist->prep_find();

# Read the file in pieces, writing to new files for each bucket.
# This is the portion that would be executed by remote procedure.
POSITION: for my $p ( @P ){
	open FD, "<ips.txt" or die;
	seek( FD, $p->[0], SEEK_SET );
	my $current_position = tell( FD );

	open FO, ">out." . $p->[1];

	until( $current_position >= $p->[1] ){
		my $line = <FD>;
		chomp( $line );
		my $ip = $1 if $line =~ /(\d+\.\d+\.\d+\.\d+)/;
		if( $whitelist->find( $ip ) ){
			print STDERR "Found $line in whitelist\n";
		} else {
			print FO "$line\n";
		}
		$current_position = tell( FD );
	}
	close FD;
	close FO;
}
