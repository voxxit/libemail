package Policy::Memcache;
use Cache::Memcached::Fast;
use Policy::SenderScore;
use DateTime;

# The absolute maximum number of emails from an IP within an hour.. seriously.
our $MAXCONNECT = 1000;
my $verbose = 0;

sub new {
	my ($class_name) = @_;
	my $self = {};
	bless ($self, $class_name);

	$self->{memd} = new Cache::Memcached::Fast( {
		servers => [ { address => 'localhost:11211' } ],
		namespace => 'my:',
		connect_timeout => 0.2,
		io_timeout => 0.5,
		close_on_error => 1,
		compress_ratio => 0.9,
		compress_methods => [ \&IO::Compress::Gzip::gzip,
				      \&IO::Uncompress::Gunzip::gunzip ],
		max_failures => 3,
		failure_timeout => 2,
		nowait => 1,
		hash_namespace => 1,
		serialize_methods => [ \&Storable::freeze, \&Storable::thaw ],
		max_size => 512 * 1024,
	});

	$self->{ss} = Policy::SenderScore->new;

	return $self;
}

sub increment {
	my ($self,$ip) = @_;
	die( "Invalid IP address format: $ip\n" ) unless $ip =~ /\d+\.\d+\.\d+\.\d+/;

	if(my $val = $self->{memd}->get( $ip )){
		$self->{memd}->incr( $ip )
			unless $val >= $MAXCONNECT;
	} else {
		# Get the senderscore
		my $ss = $self->{ss}->lookup( $ip );

		# Calculate how many hours until end of hour
		my $dt = DateTime->now;
		my $expiration = DateTime->new(
			year		=> $dt->year,
			month		=> $dt->month,
			day		=> $dt->day,
			hour		=> $dt->hour,
			minute		=> 0,
			second		=> 0,
			nanosecond	=> 0,
			#time_zone	=> $dt->time_zone,
		);
		my $expiration_epoch = $expiration->epoch + ( 60 * 60 );
		print "Start of the hour: " . scalar gmtime( $expiration->epoch ) . "\n" if $verbose;
		print "Expiring at: " . scalar gmtime( $expiration_epoch ) . "\n" if $verbose;
		my $delta = $expiration_epoch - time;
		print "Delta: " . $delta / 60 . " minutes\n" if $verbose;

		# Set it!
		$self->{memd}->set( $ip, 1, $delta );

		# Store the SS of the entry as well
		$self->{memd}->set( "ss:$ip", $ss, $delta );
	}

}

sub get {
	my ($self,$ip) = @_;
	die( "Invalid IP address format: $ip\n" ) unless $ip =~ /\d+\.\d+\.\d+\.\d+/;
	my $val = $self->{memd}->get($ip) || 0;
	my $ss  = $self->{memd}->get("ss:$ip") || -1;
	return $val,$ss;
}

sub flush {
	my ($self,$ip) = @_;
	$self->{memd}->delete( $ip );
}

1;

