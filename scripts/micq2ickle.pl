#!/usr/bin/perl

#
# perl script to convert micq's contact list
# to ickle
# Orb <cyberwin@mail.ru>
#

print "micq2ickle.pl script\n";
print "--------------------\n\n";

if (!defined $ENV{'HOME'}) {
    print "Problem: Couldn't figure out home directory, environment\nvariable HOME is not set.\n\n";
    exit(-1);
}

$home = $ENV{'HOME'};
$micq_rc = "$home/.micqrc";
$ickle_dir = "$home/.ickle/";
$ickle_users_dir = "$home/.ickle/contacts/";

die ("Problem: File $micq_rc doesn't exist\n") unless (-f $micq_rc);

die ("Problem: Creating directory $ickle_dir: $!") unless ( -d $ickle_dir or mkdir($ickle_dir, 0700));
die ("Problem: Creating directory $ickle_users_dir: $!") unless ( -d $ickle_users_dir or mkdir($ickle_users_dir, 0700));

open(MF, "<$micq_rc");
@micqrc=<MF>;
close(MF);
chomp(@micqrc);

foreach $line (@micqrc) {
    my ($uin, @alias, $mobile_no);
    if($ok){
     if($line){
    	$line=~ s/^\*//;
	$line=~ s/^\s+//;
	($uin, @alias)=split(/\s+/, $line);
	open(USER, ">${ickle_users_dir}${uin}.user") or die("Couldn't create ${ickle_users_dir}${uin}.user\n");
	print USER "uin = $uin\n";
	print USER "alias = @alias\n";
	print USER "mobile_no = $mobile_no\n";
	close USER;
	print "Created user @alias ($uin)\n";
     }
    }
    if ($line eq "Contacts"){
	$ok=1;
    }
}

print "Finished successfully.\n\n";

