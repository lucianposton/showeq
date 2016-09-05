#!/usr/bin/perl

# spells_en2spellsh.pl
# ShowEQ Distributed under GPL
# http://seq.sourceforge.net/
#
# Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net)
#
# Contributed to ShowEQ by Zaphod (dophaZ@users.sourceforge.net)
# for use under the terms of the GNU General Public Licence,
# incorporated herein by reference.
#

# This is just a quicky little Perl 5 script to create a spells.h file from
# a spells_en.txt file.

# Notes:
# + This Perl script requires the Unicode::String module that be retrieved off
#   of CPAN from the URL: http://search.cpan.org/search?module=Unicode::String

use Unicode::String qw(latin1 utf16 ucs2 utf8);

$infile = "spells_us.txt";
$outfile = "/tmp/staticspells.h";

# file fields
$spellIdField = 0;
$nameField = 1;

# Target type ID
$targetSelf = 0x06;

unless (open(SPELLSEN, "<$infile")) 
{
    die "Can't open $infile: $!\n";
}
print "Reading: $infile\n";

unless (open(SPELLSH, ">$outfile"))
{
    die "Can't open $outfile: $!\n";
}
print "Writing: $outfile\n";

$maxSpellId = 0;
$header = "/*
 * spells.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

/* File autogenerated from spells_en.txt */
// Format:
";

print SPELLSH $header;

print SPELLSH "// /* Spell ID */ { Spell Name },\n";

print SPELLSH "\n";

while($line = <SPELLSEN>)
{
#    print STDERR $count++, "\n";
    $u = utf8($line);
    $latin1line = $u->latin1;
#    print SPELLSH $latin1line;
    @fields = split(/\^/, $latin1line);
#    print STDERR "Fields: ", $#fields, "\n";
    $spellId = $fields[$spellIdField];
    $spellName = $fields[$nameField];

    $records[$spellId] = sprintf("/* 0x%04x - %5d */ { \"%s\" }, ", 
				 $spellId,
				 $spellId,
				 $spellName);

    $maxSpellId = $spellId if ($spellId > $maxSpellId);
}

$emptyCount = 0;
for ($spellId = 0; $spellId <= $maxSpellId; $spellId++)
{
    if ($records[$spellId]) 
    {
	print SPELLSH $records[$spellId], "\n";
    }
    else
    {
	printf SPELLSH "/* 0x%04x = %5d */ { NULL, },\n", $spellId, $spellId;
	$emptyCount++;
    }
}

print SPELLSH "// \n";
printf SPELLSH "// Max SpellId: 0x%04x = %5d\n", $maxSpellId, $maxSpellId;
print SPELLSH "// Number of Spells: ", $#records - $emptyCount, "\n";
print SPELLSH "// Empty Entries: ", $emptyCount, "\n";
