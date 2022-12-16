#!/usr/bin/perl

use lib 'lib';

use bom;
use bif;
use config;
use layout;
use CGI;
use CGI::Cookie;

my $cgi = new CGI;
my $login = $cgi->param('login');
my $passwd = $cgi->param('passwd');

sub form {
  qq{
    <form action="login.cgi" method="get">
    <table>
      <tr>
        <td>Login:</td>
        <td><input name="login" type="text" size=40 value="$login"></td>
      </tr>
      <tr>
        <td>Password:</td>
        <td><input name="passwd" type="password" size=40 value="$passwd"></td>
      </tr>
      <tr>
        <td colspan=2 align=center>
	  <input type=submit value="Login">
        </td>
      </tr>
    </table>
    </form>
  };
}

sub death {
  my %param = ('title' => 'Login', @_);
  print layout::mime_header(%param);
  print layout::header(%param);
  print layout::error($param{'error'}) if $param{'error'};
  print form;
  print layout::footer;
  bom_db_exit;
  exit 0;
}

bom_db_init($config::db_directory);

death unless $login;
my $acct = new bom::acct $login;

my $abort = 0;
my $exists = 0;
try_txn {
  $exists = $acct->exists;
} catch {
  $abort = 1;
};
$exists or death 'error' => "Account does not exist.";
$abort and death 'error' => "Database error (1); please try again later.";

my $abort = 0;
try_txn {
  $acct->read;
} catch {
  $abort = 1;
};
$abort and death 'error' => "Database error (2); please try again later.";

if ($$acct{'passwd'} ne $passwd) {
  death 'error' => "Passwords do not match.";
  exit 0;
}

my ($client, $session);
my $abort = 0;
try {
  $client = new bif::client $config::host;
  $session = $client->proxy_connect($config::host);
} catch {
  $abort = 1;
};
$abort and death 'error' => "Cannot connect to host.";

my $portfolio;

my $abort = 0;
try {
  $client->authenticate($acct);
  $portfolio = $client->balance;
} catch {
  $abort = 1;
};
$abort and death 'error' => "Cannot authenticate.";

my %cookie;
$cookie{'login'} = new CGI::Cookie
 '-name' => 'login', '-value' => $login;
$cookie{'session'} = new CGI::Cookie
 '-name' => 'session', '-value' => $session;

print layout::mime_header
 'cookie' => [ values(%cookie) ],
 'location' => "$config::html_root/balance.cgi";

1;
