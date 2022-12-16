#!/usr/bin/perl

use lib 'lib';

use dmath;
use bom;
use bif;
use config;
use layout;
use CGI;
use CGI::Cookie;

my $cgi = new CGI;
my $login = $cgi->param('login');
my $passwd = $cgi->param('passwd');
my $passwd2 = $cgi->param('passwd2');

sub form {
  qq{
    <form action="register.cgi" method="get">
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
        <td>Password (again):</td>
        <td><input name="passwd2" type="password" size=40 value="$passwd2"></td>
      </tr>
      <tr>
        <td colspan=2 align=center>
	  <input type=submit value="Register New Account">
        </td>
      </tr>
    </table>
    </form>
  };
}

sub death {
  my %param = ('title' => 'Register New Account', @_);
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
try_txn {
  $acct->read;
  $abort = 1;
} catch { 
  $_ != '-7' and $abort = 1;
};
$abort and death 'error' => "Database error; please try again later.";

my $root_acct = new bom::acct $config::root_acctid;
my $abort = 0;
try_txn {
  $root_acct->read;
} catch {
  $abort = 1;
};
$abort and death 'error' => "Database error; please try again later.";

if ($passwd ne $passwd2) {
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

$acct->promise("");
$acct->flags(0);
$acct->passwd($passwd);
($$acct{'public'}, $$acct{'secret'}, $$acct{'modulus'}) =
 dmath::rsa_keygen(512);

my $abort = 0;
try {
  $client->authenticate($root_acct);
  $client->acct_create($acct);

  my $au = new bom::denom 'au';
  my $amount = new dmath::bz '10000';
  my $value = new bom::value $au, $amount;

  $client->credit($acct, $value);
} catch {
  $abort = 1;
};
$abort and death 'error' => "Cannot create new acct on server.";

my $abort = 0;
try_txn {
  $acct->write;
} catch {
  $abort = 1;
};
$abort and death 'error' => "Cannot create new acct locally.";

print layout::mime_header(
 'location' => ("$config::html_root/login.cgi?" .
  'login=' . CGI::escape($login) . '&' .
  'passwd=' . CGI::escape($passwd)));

bom_db_exit;

1;
