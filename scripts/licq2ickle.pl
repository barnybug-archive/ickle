#!/usr/bin/perl

#
# perl script to convert licq's contact list across
# to ickle
# (c) Barnaby Gray 2001 <barnaby@beedesign.co.uk>
#

print "licq2ickle.pl script\n";
print "--------------------\n\n";

if (!defined $ENV{'HOME'}) {
    print "Problem: Couldn't figure out home directory, environment\nvariable HOME is not set.\n\n";
    exit(-1);
}

$home = $ENV{'HOME'};
$licq_dir = "$home/.licq/";
$licq_users_dir = "$home/.licq/users/";
$ickle_dir = "$home/.ickle/";
$ickle_users_dir = "$home/.ickle/contacts/";

die ("Problem: Directory $licq_dir doesn't exist\n") unless (-d $licq_dir);
die ("Problem: Directory $licq_users_dir doesn't exist\n") unless (-d $licq_users_dir);

@users = glob("${licq_users_dir}*.uin");

die ("Problem: Creating directory $ickle_dir: $!") unless ( -d $ickle_dir or mkdir($ickle_dir, 0700));
die ("Problem: Creating directory $ickle_users_dir: $!") unless ( -d $ickle_users_dir or mkdir($ickle_users_dir, 0700));

foreach $filename (@users) {
    my ($uin, $alias, $mobile_no);
    next if ($filename !~ /\/(\d+).uin$/);
    $uin = $1;

    open USER, "<$filename";
    while(<USER>) {
	chomp;
	if (/^Alias = (.+)$/) {
	    $alias = $1;
	} elsif (/^CellularNumber = (.+)$/) {
	    $mobile_no = $1;
	    $mobile_no =~ s/\D//g;
	}
    }
    close USER;

    open(USER, ">${ickle_users_dir}${uin}.user") or die("Couldn't create ${ickle_users_dir}${uin}.user\n");
    print USER "uin = $uin\n";
    print USER "alias = $alias\n";
    print USER "mobile_no = $mobile_no\n";
    close USER;
    
    print "Created user $alias ($uin)\n";
}

print "Finished successfully.\n\n";

