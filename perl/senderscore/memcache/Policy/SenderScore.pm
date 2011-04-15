package Policy::SenderScore;
use strict;
use Net::DNS;
use Data::Dumper;

our $ZONENAME = "ZONENAME-OF-YOUR-SENDERSCORE-ZONE"

sub new {
	my ($class_name) = @_;
	my $self = {};
	bless ($self, $class_name);

	$self->{res} = Net::DNS::Resolver->new(
		nameservers => [qw( 1.2.3.4 )],	# IP Address of your resolver
		recurse     => 0,
		debug       => 0,
		udp_timeout => 1,
	);

	return $self;
}

sub dnslookup {
	my ($self,$ip) = @_;

	my $lookup = join( ".", reverse( split( /\./, $ip ) ) ) . "." . $ZONENAME;

	my $query = $self->{res}->search( $lookup );

	my @results;
	if( $query ){
		for my $rr ($query->answer) {
			next unless $rr->type eq "A";
			push( @results, $rr->address );
		}
	}
	return \@results;
}
	
sub lookup {
	my ($self,$ip) = @_;
	my $results = $self->dnslookup( $ip );

	my $highest = undef;
	for my $result ( @$results ){
		my @a = split( /\./, $result );
		my $z = $a[-1];
		$highest = $z if( $z > $highest );
	}

	return $highest;
}

1;
