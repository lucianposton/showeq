#!/usr/bin/perl

$dir = 0;

while($_ = $ARGV[0], /^-/)
{
    shift;
    last if /^--$/;
    if (/^-D/) { $dir = 1; }
}

if (!$dir)
{
    migrate(@ARGV);
}
else
{
    migratedir(@ARGV);
}

sub migratedir
{
    local ($source, $dest, $comments) = @_;

    opendir(SOURCEDIR, $source) || die "Can't open '$source' directory: $!\n";
    @conffiles = grep  /^filters_.*\.conf$/, readdir SOURCEDIR;
    closedir(SOURCEDIR);
    
    foreach $file (@conffiles)
    {
	($destfile = $file) =~ s/^filters_(.*)\.conf$/$1.xml/;
#	print "$source/$file => $dest/$destfile\n";
	migrate("$source/$file", "$dest/$destfile", $comments);
    }
}

sub migrate
{
    local ($source, $dest, $comments) = @_;

    print "Migrating '$source' to '$dest'\n";

    # open source and destination files
    open(SOURCE_FILE, "<$source") || die "Can't find '$source' $SOURCE_FILE: $!\n";
    open(DEST_FILE, ">$dest") || die "Can't create '$dest' $DEST_FILE: $!\n";

    # print the XML header
    print DEST_FILE "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    print DEST_FILE "<!DOCTYPE seqfilters SYSTEM \"seqfilters.dtd\">\n";
    print DEST_FILE "<seqfilters>\n";

    $incomment = 0;
    $insection = 0;
    
    # iterate over the contents of the source file
    while ($line = <SOURCE_FILE>)
    {
	# handle empty lines
	if ($line =~ m/^\s*$/)
	{
	    next if (!$comments);
	    if ($incomment)
	    {
		print DEST_FILE "---> \n";
		$incomment = 0;
	    }
	    next;
	}

	# handle escaping of XML unfriendly characters
	$line =~ s/&/&amp;/g;
	$line =~ s/</&lt;/g;
	$line =~ s/>/&gt;/g;
	$line =~ s/\"/&quot;/g;

	# handle comments
	if ($line =~ m/^\#/)
	{
	    next if (!$comments);
	    if ($incomment)
	    {
		$line =~ s/^\#(.*)$/ $1 /;
	    }
	    else
	    {
		$line =~ s/^\#(.*)$/<--\n  $1 /;
		$incomment = 1;
	    }
	    
	    print DEST_FILE $line;
	    next;
	}
	if ($incomment)
	{
	    print DEST_FILE "---> \n";
	    $incomment = 0;
	}
	
	# handle section breaks
	if ($line =~ m/^\[/)
	{
	    print DEST_FILE "    </section>\n" if ($insection);
	    $insection = 1;
	    $line =~ s/^\[([^\]]+)\]\s*$/    <section name="$1">/;
	    print DEST_FILE $line, "\n";
	}
	else 
	{
	    # min and max default to 0
	    $min = 0;
	    $max = 0;
	    
            # fix line termination and errant trailing spaces
	    $line =~ s/^(.*?)\s*$/$1/;
	    
	    # seperate 
	    @line = split(m/;/, $line);

	    # process level ranges
	    if ($#line > 0)
	    {
		$range = $line[1];
		if ($range =~ m/^(\d+)-$/)
		{
		    $min = $1;
		    $max = 255;
		}
		elsif ($range =~ m/^-(\d+)$/)
		{
		    $min = 0;
		    $max = $1;
		}
		elsif ($range =~ m/^(\d+)-(\d+)/)
		{
		    $min = $1;
		    $max = $2;
		}
	    }

	    # print out in the XML filter format
	    print DEST_FILE "        <oldfilter>";
	    print DEST_FILE "<regex>", $line[0], "</regex>" if ($line[0]);

	    # handle outputting level ranges
	    if ($min || $max)
	    {
		print DEST_FILE "<level";
		print DEST_FILE " min=\"$min\"" if ($min);
		print DEST_FILE " max=\"$max\"" if ($max);
		print DEST_FILE "/>";
	    }

	    # close the filter entry
	    print DEST_FILE "</oldfilter>\n"
	}
    }
    
    # close the section
    print DEST_FILE "    </section>\n" if ($insection);
    print DEST_FILE "</seqfilters>\n";
}

