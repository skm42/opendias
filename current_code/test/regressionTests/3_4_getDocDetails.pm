package regressionTests::3_4_getDocDetails;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use Data::Dumper;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
    shutdown => 1,
  }; 
} 

sub test {

  my %data = (
    action => 'getDocDetail',
    docid => 3,
  );


  # Call getDocDetails
  o_log( "Doc Details, one tag" );
  o_log( Dumper( directRequest( \%data ) ) );

  # Call getDocDetails
  o_log( "Doc Details, two tags" );
  $data{docid} = 2;
  o_log( Dumper( directRequest( \%data ) ) );

  # Call getDocDetails
  o_log( "Doc Details, two linked documents" );
  $data{docid} = 4;
  o_log( Dumper( directRequest( \%data ) ) );

  return 0;
}

return 1;

