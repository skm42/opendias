package regressionTests::1_1_create_database;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
    startTimeout => 240,
    shutdown => 1,
  };
}

sub test {

  my $dbh = DBI->connect( "dbi:SQLite:dbname=/tmp/opendiastest/openDIAS.sqlite3", 
                          "", "", { RaiseError => 1, AutoCommit => 1, sqlite_use_immediate_transaction => 1 } );

  my $sth = $dbh->prepare("SELECT version FROM version");
  $sth->execute();

  while( my $hashRef = $sth->fetchrow_hashref() ) {
    if ( $hashRef->{version} eq "6" ) {
      o_log("Correct DB version.");
    }
    else {
      o_log("Incorrect DB version.");
      $dbh->disconnect();
      return 1;
    }
  }

  $dbh->disconnect();

  # compare the generated database schema
  system("/usr/bin/sqlite3 /tmp/opendiastest/openDIAS.sqlite3 '.dump' > /tmp/generated.sql");
  system("/usr/bin/sqlite3 config/defaultdatabase.sqlite3 '.dump' > /tmp/reference.sql");
  o_log("Diff of generated-to-reference database: \n" . `diff /tmp/generated.sql /tmp/reference.sql` );

  return 0;
}

return 1;

