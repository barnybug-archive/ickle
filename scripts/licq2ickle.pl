#!/usr/bin/perl

#
# perl script to convert licq's contact list across
# to ickle
# (c) Barnaby Gray 2001 <barnaby@beedesign.co.uk>
#

print "\n";
print <<"END";
licq2ickle.pl script
Version 2 - features:
- Conversion of contacts across
- Merging of history across
--------------------

END

if (!defined $ENV{'HOME'}) {
    print "Problem: Couldn't figure out home directory, environment\nvariable HOME is not set.\n\n";
    exit(-1);
}

$home = $ENV{'HOME'};
$licq_dir = "$home/.licq/";
$licq_users_dir = "$home/.licq/users/";
$licq_history_dir = "$home/.licq/history/";
$ickle_dir = "$home/.ickle/";
$ickle_users_dir = "$home/.ickle/contacts/";

die ("Problem: Directory $licq_dir doesn't exist\n") unless (-d $licq_dir);
die ("Problem: Directory $licq_users_dir doesn't exist\n") unless (-d $licq_users_dir);
die ("Problem: Directory $licq_history_dir doesn't exist\n") unless (-d $licq_history_dir);

@users = glob("${licq_users_dir}*.uin");

die ("Problem: Creating directory $ickle_dir: $!") unless ( -d $ickle_dir or mkdir($ickle_dir, 0700));
die ("Problem: Creating directory $ickle_users_dir: $!") unless ( -d $ickle_users_dir or mkdir($ickle_users_dir, 0700));

foreach $filename (@users) {
    my ($uin, $alias, $firstname, $lastname, $mobile_no);
    next if ($filename !~ /\/(\d+).uin$/);
    $uin = $1;

    open USER, "<$filename";
    while(<USER>) {
	chomp;
	if (/^Alias = (.+)$/) {
	    $alias = $1;
	} elsif (/^FirstName = (.+)$/) {
	    $firstname = $1;
	} elsif (/^LastName = (.+)$/) {
	    $lastname = $1;
	} elsif (/^CellularNumber = (.+)$/) {
	    $mobile_no = $1;
	    $mobile_no =~ s/\D//g;
	}
    }
    close USER;

    my $ickle_user_file = $ickle_users_dir.$uin.'.user';
    if (!-e $ickle_user_file) {
	if (open(USER, ">$ickle_user_file")) {
	    print USER "uin = $uin\n";
	    print USER "alias = $alias\n";
	    print USER "firstname = $firstname\n";
	    print USER "lastname = $lastname\n";
	    print USER "mobile_no = $mobile_no\n";
	    close USER;
	    print "Created user $alias ($uin)\n";
	} else {
	    print "Warning: Couldn't create $ickle_user_file\n";
	    next;
	}
    }

    # history merge
    my $licq_history_file = $licq_history_dir.$uin.'.history';
    if (-e $licq_history_file) {
	my %history_map;
	print "Merging history for $alias ($uin)...";
	if (!open(HISTORY, "<$licq_history_file")) {
	    print "\nWarning: Failed to open $licq_history_file\n";
	    next;
	}

	my $licq_count = 0;
	my $ickle_count = 0;
	my $merge_count = 0;

	my $body = 0;
	my $url = 0;
	my $event;
	LINE: while(<HISTORY>) {
	    chomp;

	    if (!$body) {
		next if (!/^\[ (.) \| (\d{4}) \| (\d{4}) \| (\d{4}) \| (\d+) \]$/);

		$entry = {};

		$entry->{Direction} = (($1 eq 'R') ? 'Received' : 'Sent');
		$entry->{Time} = $5;
		$entry->{Offline} = ((int($4) & 0x0001) ? 'No' : 'Yes');
		$entry->{Multiparty} = ((int($4) & 0x0002) ? 'Yes' : 'No');

		if (int($2) == 1) {
		    $entry->{Type} = 'Normal';
		} elsif (int($2) == 4) {
		    $entry->{Type} = 'URL';
		} else {
		    undef $entry;
		}

		$body = 1;
		$url = 0;

	    } else {

		$valid = /^:(.*)$/;
		if ($valid) {
		    if (defined $entry) {
			if ($entry->{Type} eq 'Normal') {
			    push @{ $entry->{Message} }, $1;
			} elsif ($entry{Type} eq 'URL') {
			    if ($url == 0) {
				$entry->{URL} = $1;
				$url = 1;
			    } else {
				push @{ $entry->{Message} }, $1;
			    }
			}
		    }

		} else {
		    if (defined $entry) {
			push @{ $history_map{ int($entry->{Time}) } }, $entry;
		    }

		    undef $entry;
		    $licq_count++;
		    $merge_count++;

		    $body = 0;
		    redo;
		}

	    }

	}
	close HISTORY;

	my $ickle_history_file = $ickle_users_dir.$uin.'.history';
	if (-e $ickle_history_file) {
	    if (!open(HISTORY, "<$ickle_history_file")) {
		print "Warning: Failed to open $ickle_history_file\n";
		next;
	    }

	    my $message_body = 0;
	    my $entry = {};
	    while (<HISTORY>) {
		chomp;
		if ($_ eq '') {
		    $ickle_count++;

		    # resolve collisions
		    my ($key, $value);
		    my $duplicate = 0;
		    my $msg1 = join "\n", @{$entry->{Message}};
		    ENTRY: foreach $en (@{ $history_map{ int($entry->{Time}) } }) {
			my $message = $en->{Message};

			my $msg2 = join "\n", @{$en->{Message}};
			next ENTRY if ($msg1 ne $msg2);

			$duplicate = 1;
			last ENTRY;
		    }

		    if (!$duplicate) {
			push @{ $history_map{ int($entry->{Time}) } }, $entry;
			$merge_count++;
		    }

		    $entry = {};
		    $message_body = 0;
		    next;
		}

		if (!$message_body) {
		    my ($key, $value) = split /:\s/, $_, 2;
		    if ($key eq 'Message') {
			$message_body = 1;
			$_ = $value;
		    } else {
			$entry->{$key} = $value;
		    }
		}
			
		if ($message_body) {
		    my $end_char = substr $_, -1;
		    $_ = substr $_, 0, -1;
		    push @{ $entry->{Message} }, $_;
		    if ($end_char ne '\\') {
			$message_body = 0;
		    }
		}

	    }
	    close HISTORY;
	}

	if (!open(HISTORY, ">$ickle_history_file")) {
	    print "\nWarning: Failed to open $ickle_history_file for write\n";
	    next;
	}

	foreach $key (sort { int($a) <=> int($b) } keys %history_map) {
	    foreach $entry (@{ $history_map{$key} }) {
		print HISTORY 'Type: '.$entry->{Type}."\n";
		print HISTORY 'Time: '.$entry->{Time}."\n";
		print HISTORY 'Direction: '.$entry->{Direction}."\n";
		print HISTORY 'Offline: '.$entry->{Offline}."\n";
		print HISTORY 'Multiparty: '.$entry->{Multiparty}."\n";
		if ($entry->{Type} eq 'URL') {
		    print HISTORY 'URL: '.$entry->{URL}."\n";
		}
		print HISTORY 'Message: ';
		for (my $a = 0; $a < @{$entry->{Message}}; $a++) {
		    print HISTORY $entry->{Message}[$a];
		    if ($a < @{$entry->{Message}} - 1) {
			print HISTORY "\\\n";
		    }
		}
		print HISTORY " \n";

		print HISTORY "\n";
	    }
	}

	close HISTORY;
	
	print "Read (licq:$licq_count/ickle:$ickle_count) merged:$merge_count\n";
    }
}

print "Finished successfully.\n\n";
print "Now run ickle and enjoy!\n\n";
