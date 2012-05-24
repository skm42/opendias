package regressionTests::3_8_autoComplete;

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
    action => 'tagsAutoComplete',
  );

  my $entry = 'tag one';
  foreach my $docid (2,4) {
    o_log( "On document : $docid" );
    $data{docid} = $docid;
    for( my $x = 1 ; $x < length( $entry ) ; $x++) {
      my $partEntry = substr( $entry, 0, $x );
      $data{startsWith} = $partEntry;
      o_log( "Suggestions for an entry of: $partEntry" );
      o_log( Dumper( directRequest( \%data ) ) );
    }
  }

  $data{action} = 'titleAutoComplete';
  delete $data{docid};
  my $entry = 'Test 2 Title text';
  foreach my $docid (2,4) {
    o_log( "Show document options that are not already linked to docid: $docid" );
    $data{notLinkedTo} = $docid;
    for( my $x = 1 ; $x < length( $entry ) ; $x++) {
      my $partEntry = substr( $entry, 0, $x );
      $data{startsWith} = $partEntry;
      o_log( "Suggestions for an entry of: $partEntry" );
      o_log( Dumper( directRequest( \%data ) ) );
    }
  }

  return 0;
}

return 1;

