package regressionTests::1_5_could_not_start_http_service;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use strict;
use IO::Socket::INET;

our $sock;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
    updateStartCommand => 'updateStartCommand',
    shutdown => 0,
  }; 
} 

sub updateStartCommand {
  # Start a port on the opendias port
  $sock = IO::Socket::INET->new( Listen => 1,
                                 LocalAddr => 'localhost',
                                 LocalPort => $ENV{OPENDIAS_PORT},
                                 Proto => 'tcp')
      || die "Could not create blocking socket: $!";

  o_log("Reserved the opendias port, to stop the service from starting correctly.") if $sock;
}

sub test {

  system("while [ \"`pidof valgrind.bin`\" != \"\" ]; do sleep 1; done");

  # close the overridding http port
  $sock->close();

  return 0;
}

return 1;

