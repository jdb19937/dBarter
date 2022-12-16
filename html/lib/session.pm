package session;

use bom;
use bif;
use CGI;
use CGI::Cookie;

use config;

sub restore {
  my %cookie = fetch CGI::Cookie;

  my $session = $cookie{'session'} && $cookie{'session'}->value or
   throw "nosession";
  my $login = $cookie{'login'} && $cookie{'login'}->value or
   throw "nosession";
  
  my $client = new bif::client $config::host;
  $client->proxy_reconnect($session);
  $$client{'acct'} = new bom::acct $login;

  $client;
}

sub error {
  my $message = shift;
  print "Location: " . $config::html_root . "/error.cgi?" .
   "message=" . CGI::escape($message) . "\n\n";
  exit 0;
}

1;
