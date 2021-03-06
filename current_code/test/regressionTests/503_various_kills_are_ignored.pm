package regressionTests::503_various_kills_are_ignored;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use strict;

sub test {

  # try kills this way and that
  system("kill -s HUP `cat /var/run/opendias.pid`");
  sleep(3);

  system("kill -s USR1 `cat /var/run/opendias.pid`");
  sleep(3);

  return 0;
}

return 1;

