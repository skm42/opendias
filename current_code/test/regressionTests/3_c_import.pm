package regressionTests::3_c_import;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use Data::Dumper;
use LWP::UserAgent;
use HTTP::Request::Common;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
    shutdown => 1,
  }; 
} 

sub test {

  #
  # Send to and receive from the application
  #
  my %default = (
    '__proto' => 'http://',
    '__domain' => 'localhost:' . $ENV{OPENDIAS_PORT},
    '__uri' => '/opendias/dynamic',
    '__encoding' => 'application/x-www-form-urlencoded',
    '__agent' => 'opendias-api-testing',
  );

  my $ua = LWP::UserAgent->new;
  $ua->timeout( 600 ); # 10 mins
  $ua->agent($default{__agent});

  my $file = "./regressionTests/inputs/3_c_import/test.pdf";
  my $req = POST(
                $default{__proto} . $default{__domain} . $default{__uri},
                Content_Type => 'form-data',
                Content => [
                    action => 'uploadfile',
                    uploadfile => ["$file"],
                    ftype => 'PDF',
                    ],
                );

  my $res = $ua->request( $req );

  my $result;
  if ($res->is_success) {
    $result = $res->content;
    o_log( "response was = " . Dumper( $result ) );
  }
  else {
    o_log( "Did not get a successful response.\n".Dumper($res)."\n".Dumper($ua) );
    return 1;
  }

  return 0;
}

return 1;

