#!/usr/bin/perl

# gnomeicu2ickle.pl perl script to convert GnomeICU contacts to ickle format
# works with gnomeicu 0.96 and ickle 0.2.x<3, others not tested, so no warranty
# for others
# Usage:
# ./gnomeicu2ickle.pl gnomeicu_config_filename_full_path


# (C) Copyright 2002 Jakub Suchy <jakub@salon.cz>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


if (!defined $ENV{'HOME'}) {
        print "Problem: Couldn't figure out home directory, environment\nvariable HOME is not set.\n\n";
        exit(-1);
    }
$home = $ENV{'HOME'};

$gicu_config = $ARGV[0];
$gicu_config = "$home/.gnome/GnomeICU" if ($gicu_config eq '');
if (!$gicu_config || !(-f $gicu_config)) {
    print "Problem: GnomeICU config file not specified or not exist\n";
    print "GnomeICU config file must be specified as first argument\n\n";
    exit(-1);
    }

$ickle_dir = "$home/.ickle/";
$ickle_users_dir = "$home/.ickle/contacts/";

die ("Problem: Creating directory $ickle_dir: $!") unless ( -d $ickle_dir or mkdir($ickle_dir, 0700));
die ("Problem: Creating directory $ickle_users_dir: $!") unless ( -d $ickle_users_dir or mkdir($ickle_users_dir, 0700));

open(DATA, "<$gicu_config")
  or die "Problem: Can't open GnomeICU config file:\n $!\n\n";

print "Using config file: $gicu_config \n";

print "Really replace your actual contact data with the info in this file?\n[y/N] ";
$input = lc(<STDIN>);
chomp $input;
die "Aborted!\n" if ($input ne "y");

while ($line = <DATA>) {
    chomp($line);
    # skip everything until [NewContacts]
    if ($line =~ /\[NewContacts\].*/) {
	$found_newcontacts = 1;
	next;
	}
    next if (!$found_newcontacts);
    # skip everything out of the [NewContacts] block
    last if ($found_newcontacts && $line =~ /^[:space:]*$/);
    
    # figure out contact's uin and nickname
    ($uin, $nickname) = split(/=/, $line, 2);
    # gnomeicu adds string ",Ssrv" after nickname to indicate
    # 'Force sending message through server'
    # should be useful when ickle support this too
    $nickname =~ s/,Ssrv$//;
    # gnomeicu uses a serial number to identify each contact, we don't need it
    $nickname =~ s/^[0-9]*,//;
    
    if ($uin !~ /^\d+$/) {
	# hmh, the file is possibly corrupted?
	print "Problem: $nickname has corrupted uin ($uin), skipping\n";
	next;
	}
    open(USER, ">${ickle_users_dir}${uin}.user")
      or die "Problem: Couldn't create ${ickle_users_dir}${uin}.user file:\n $!\n\n";
    
    print USER "uin = $uin\n";
    print USER "alias = $nickname\n";
    # gnomeicu doesn't support mobile number (afaik)
    print USER "mobile_no =\n";
    close(USER);
    print "Created user $nickname ($uin)\n";
    }

print "Finished successfully\n\n";
exit(0);
