package regressionTests::1_4_various_kills_are_ignored;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
    shutdown => 1,
  }; 
} 

sub test {

  sleep(3);

	open(PIDFILE,$ENV{OPENDIAS_RUNLOCATION} . "/opendias.pid") or warn "no pidfile $!\n";
	my $pid=<PIDFILE>;
	close(PIDFILE);

	my $s;
	#should be target: foreach $s (0,1,2,3,4,5,6,7,8,11,12,13,14,15,16,17,18,19,20) {
	foreach $s (0,1,2,15) {
		print "killing $s $pid\n";
		kill($s,$pid);
		sleep 3
	}


  # try kills this way and that
  #system("kill -s HUP `cat " . $ENV{OPENDIAS_RUNLOCATION} . "/opendias.pid`");
  #sleep(3);

  #system("kill -s USR1 `cat " . $ENV{OPENDIAS_RUNLOCATION} . "/opendias.pid`");
  #sleep(3);

	#in perl 1 is defined to be true, why false here?
  return 0;
}

return 1;

