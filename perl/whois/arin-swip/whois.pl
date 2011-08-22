#!/usr/bin/perl
use strict;
use Net::CIDR::Lite;
use Data::Dumper;

my $lookup = shift || die("Please submit a lookup phrase\n");
my $ip = shift;

unless( $ip ){
	# Create an IP from the lookup
	if( $lookup =~ /\d+\.\d+\.\d+\.\d+$/ ){
		$ip = $lookup;
	} elsif ( $lookup =~ /(\d+\.\d+\.\d+)\.0\/\d+/ ){
		$ip = $1 . ".1";
	} else { die("You must supply an IP to check against\n"); }
}

my @results = `whois $lookup`;

# Check for an ARIN SWIP...
my @SWIP = grep { /NET-(?:\d+-){4}\d+/ } @results;

if( scalar @SWIP > 0 ){
	my $cidr = Net::CIDR::Lite->new;
	my @possible;
	my $swipmap = {};
	for( @SWIP ){
		if( /^(.*)\((NET-.*?)\)\s+(\S+\s+-\s+\S+)/ ){
			my ($name,$handle,$range) = ($1,$2,$3);
			# Not all SWIPs will contain the IP in question, so
			# test for it
			my $cidr = Net::CIDR::Lite->new;
			$cidr->add_any( $range );
			if( $cidr->find( $ip ) ){
				push( @possible, [$cidr,$name,$handle] );
				my @a = $cidr->list;

				$swipmap->{"$a[0]"} = { 
					name => $name, 
					handle => $handle 
				};
			}
		}
	}

	# Check all possible entries for the smallest subnet,
	# and report on that one.
	my $smallest = 0;
	my $smallestcidr;
	for( @possible ){
		my ($cidr,$name,$handle) = @{$_};
		my @a = $cidr->list;
		my $test = $a[0];
		if( $test =~ /.*?\/(\d+)/ ){
			# Initialize
			unless( $smallest ){ 
				$smallest = $1;
				$smallestcidr = $test;
			}

			if( $smallest < $1 ){
				$smallest = $1;
				$smallestcidr = $test;
			}
		}
	}
	print $swipmap->{"$smallestcidr"}->{name},
	      $smallestcidr,"\n";
}
