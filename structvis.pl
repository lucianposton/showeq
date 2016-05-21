#!/usr/bin/perl -w

use strict;

my $charProfile = << 'END'
/*
** Player Profile
** Length: 18496 Octets
** OpCode: CharProfileCode
*/
struct charProfileStruct
{
/*0000*/ uint32_t  checksum;           //
/*0004*/ char      name[64];           // Name of player
/*0068*/ char      lastName[32];       // Last name of player
/*0100*/ uint32_t  gender;             // Player Gender - 0 Male, 1 Female
/*0104*/ uint32_t  race;               // Player race
/*0108*/ uint32_t  class_;             // Player class
/*0112*/ uint32_t  unknown0112;        // *** Placeholder
/*0116*/ uint32_t  level;              // Level of player (might be one byte)
/*0120*/ uint32_t  bind_zone_id[5];    // Bind info (zone, x, y, z, heading)
/*0140*/ float     bind_x[5];          // 0 is normal bind
/*0160*/ float     bind_y[5];          // 5 is some weird point in newbie zone
/*0180*/ float     bind_z[5];          // (which is secondary bind for wiz/dru?)
/*0200*/ float     bind_heading[5];    // Unused slots show newbie bind
/*0220*/ uint32_t  deity;              // deity
/*0224*/ int32_t   guildID;            // guildID
/*0228*/ uint32_t  birthdayTime;       // character birthday
/*0232*/ uint32_t  lastSaveTime;       // character last save time
/*0236*/ uint32_t  timePlayedMin;      // time character played
/*0240*/ uint8_t   pvp;                // 1=pvp, 0=not pvp
/*0241*/ uint8_t   level1;             // Level of Player
/*0242*/ uint8_t   anon;               // 2=roleplay, 1=anon, 0=not anon     
/*0243*/ uint8_t   gm;                 // 0=no, 1=yes
/*0244*/ int8_t    guildstatus;        // 0=member, 1=officer, 2=guildleader
/*0245*/ uint8_t   unknown0245[7];     // *** Placeholder
/*0252*/ uint32_t  intoxication;       // Alcohol level (in ticks till sober?)
/*0256*/ uint32_t  spellSlotRefresh[9]; // Refresh time (millis)
/*0292*/ uint8_t   unknown0292[8];     // *** Placeholder
/*0300*/ uint8_t   haircolor;          // Player hair color
/*0301*/ uint8_t   beardcolor;         // Player beard color
/*0302*/ uint8_t   eyecolor1;          // Player left eye color
/*0303*/ uint8_t   eyecolor2;          // Player right eye color
/*0304*/ uint8_t   hairstyle;          // Player hair style
/*0305*/ uint8_t   beard;              // Player beard type
/*0306*/ uint8_t   unknown0302[6];     // *** Placeholder
/*0312*/ uint32_t  item_material[9];   // Item texture/material of worn items
/*0348*/ uint8_t   unknown0348[48];    // *** Placeholder
/*0396*/ Color_Struct item_tint[9];    // RR GG BB 00
/*0432*/ AA_Array  aa_array[240];   // AAs
/*2352*/ char      servername[32];     // server the char was created on
/*2384*/ char      title[32];          // Current character title
/*2416*/ char      suffix[32];         // Current character suffix
/*2448*/ uint8_t   unknown2448[4];     // *** Placeholder
/*2452*/ uint32_t  exp;                // Current Experience
/*2456*/ uint32_t  unknown2456;        // *** Placeholder
/*2460*/ uint32_t  points;             // Unspent Practice points
/*2464*/ uint32_t  MANA;               // Current MANA
/*2468*/ uint32_t  curHp;              // Current HP without +HP equipment
/*2472*/ uint32_t  unknown1512;        // 0x05
/*2476*/ uint32_t  STR;                // Strength
/*2480*/ uint32_t  STA;                // Stamina
/*2484*/ uint32_t  CHA;                // Charisma
/*2488*/ uint32_t  DEX;                // Dexterity
/*2492*/ uint32_t  INT;                // Intelligence
/*2496*/ uint32_t  AGI;                // Agility
/*2500*/ uint32_t  WIS;                // Wisdom
/*2504*/ uint8_t   face;               // Player face
/*2505*/ uint8_t   unknown2505[47];    // *** Placeholder
/*2552*/ uint8_t   languages[25];      // List of languages (MAX_KNOWN_LANGS)
/*2577*/ uint8_t   unknown2577[7];     // All 0x00 (language buffer?)
/*2584*/ int32_t   sSpellBook[400];    // List of the Spells in spellbook
/*4184*/ uint8_t   unknown4184[448];   // all 0xff after last spell    
/*4632*/ int32_t   sMemSpells[9]; // List of spells memorized
/*4668*/ uint8_t   unknown4668[32];    // *** Placeholder
/*4700*/ float     x;                  // Players x position
/*4704*/ float     y;                  // Players y position
/*4708*/ float     z;                  // Players z position
/*4712*/ float     heading;            // Players heading   
/*4716*/ uint8_t   unknown4716[4];     // *** Placeholder    
/*4720*/ uint32_t  platinum;           // Platinum Pieces on player
/*4724*/ uint32_t  gold;               // Gold Pieces on player
/*4728*/ uint32_t  silver;             // Silver Pieces on player
/*4732*/ uint32_t  copper;             // Copper Pieces on player
/*4736*/ uint32_t  platinum_bank;      // Platinum Pieces in Bank
/*4740*/ uint32_t  gold_bank;          // Gold Pieces in Bank
/*4744*/ uint32_t  silver_bank;        // Silver Pieces in Bank
/*4748*/ uint32_t  copper_bank;        // Copper Pieces in Bank
/*4752*/ uint32_t  platinum_cursor;    // Platinum Pieces on cursor
/*4756*/ uint32_t  gold_cursor;        // Gold Pieces on cursor
/*4760*/ uint32_t  silver_cursor;      // Silver Pieces on cursor
/*4764*/ uint32_t  copper_cursor;      // Copper Pieces on cursor
/*4768*/ uint32_t  platinum_shared;    // Shared platinum pieces
/*4772*/ uint8_t   unknown4772[24];    // Unknown - all zero
/*4796*/ uint32_t  skills[75];         // List of skills (MAX_KNOWN_SKILLS)
/*5096*/ uint8_t   unknown5096[312];   // *** Placeholder
/*5408*/ uint32_t  autosplit;          // 0 = off, 1 = on
/*5412*/ uint8_t   unknown5412[8];     // *** Placeholder
/*5420*/ uint32_t  saveCounter;        // Number of times your char has saved
/*5424*/ uint8_t   unknown5424[28];    // *** Placeholder
/*5452*/ uint32_t  expansions;         // Bitmask for expansions
/*5456*/ uint32_t  toxicity;           // Potion Toxicity (15=too toxic, each potion adds 3)
/*5460*/ uint8_t   unknown5460[16];    // *** Placeholder
/*5476*/ uint32_t  hunger;             // Food (ticks till next eat)
/*5480*/ uint32_t  thirst;             // Drink (ticks till next drink)
/*5484*/ uint8_t   unknown5484[20];    // *** Placeholder
/*5504*/ uint16_t  zoneId;             // see zones.h
/*5506*/ uint16_t  zoneInstance;       // Instance id
/*5508*/ spellBuff buffs[25];   // Buffs currently on the player
/*6008*/ char      groupMembers[384];// all the members in group, including self 
/*6392*/ uint8_t   unknown6392[668];   // *** Placeholder
/*7060*/ uint32_t  ldon_guk_points;    // Earned GUK points
/*7064*/ uint32_t  ldon_mir_points;    // Earned MIR points
/*7068*/ uint32_t  ldon_mmc_points;    // Earned MMC points
/*7072*/ uint32_t  ldon_ruj_points;    // Earned RUJ points
/*7076*/ uint32_t  ldon_tak_points;    // Earned TAK points
/*7080*/ uint32_t  ldon_avail_points;  // Available LDON points
/*7084*/ uint8_t   unknown6124[112];   // *** Placeholder
/*7196*/ uint32_t  tributeTime;        // Time remaining on tribute (millisecs)
/*7200*/ uint32_t  unknown6240;        // *** Placeholder
/*7204*/ uint32_t  careerTribute;      // Total favor points for this char
/*7208*/ uint32_t  unknown6248;        // *** Placeholder
/*7212*/ uint32_t  currentTribute;     // Current tribute points
/*7216*/ uint32_t  unknown6256;        // *** Placeholder
/*7220*/ uint32_t  tributeActive;      // 0 = off, 1=on
/*7224*/ TributeStruct tributes[5]; // Current tribute loadout
/*7264*/ uint32_t  disciplines[50]; // Known disciplines
/*7464*/ uint8_t   unknown6504[440];   // *** Placeholder
/*7904*/ uint32_t  endurance;          // Current endurance
/*7908*/ uint32_t  expGroupLeadAA;     // Current group lead AA exp (0-1000)
/*7912*/ uint32_t  expRaidLeadAA;      // Current raid lead AA exp (0-2000)
/*7916*/ uint32_t  groupLeadAAUnspent; // Unspent group lead AA points
/*7920*/ uint32_t  raidLeadAAUnspent;  // Unspent raid lead AA points
/*7924*/ uint32_t  leadershipAAs[32]; // Leader AA ranks
/*8052*/ uint8_t   unknown7092[132];   // *** Placeholder
/*8184*/ uint32_t  airRemaining;       // Air supply (seconds)
/*8188*/ uint8_t   unknown7228[4608];  // *** Placeholder
/*12796*/ uint32_t aa_spent;           // Number of spent AA points
/*12800*/ uint32_t expAA;              // Exp earned in current AA point
/*12804*/ uint32_t aa_unspent;         // Unspent AA points
/*12808*/ uint8_t  unknown11848[36];   // *** Placeholder
/*12844*/ BandolierStruct bandoliers[4]; // bandolier contents
/*14124*/ uint8_t  unknown13164[5120]; // *** Placeholder 
/*19244*/ InlineItem potionBelt[4]; // potion belt
/*19532*/ uint8_t  unknown18572[8];    // *** Placeholder
/*19540*/ uint32_t currentRadCrystals; // Current count of radiant crystals
/*19544*/ uint32_t careerRadCrystals;  // Total count of radiant crystals ever
/*19548*/ uint32_t currentEbonCrystals;// Current count of ebon crystals
/*19552*/ uint32_t careerEbonCrystals; // Total count of ebon crystals ever
/*19556*/ uint32_t unknown18596;       // *** Placeholder
}; /* 19560 */
END
;

# load struct
my @struct = ();

while ($charProfile =~ /^(.*)$/mcg) {
    my $line = $1;
    if ($line =~ /^.*\s(\S+)\s+(\S+)\;/) {
        my $type = $1;
        my $name = $2;

        push(@struct, { 'name' => $name, 'type' => $type } );
    }
}

# load data
my @data = ();
my $goodpacket = 0;
my $timestamp = "";

print "opening zone.log";
open(PACKET, "./zone.log");
while (my $line = <PACKET>) {
    # Sep 07 2004 23:13:30:968 [Decoded] [Server->Client] [Size: 11120]
    # [OPCode: 0x006b]
    print "-$line\n";

    # empty line is end of packet
    if ($line =~ /^$/) {
        if ($goodpacket) {
            print "\n$timestamp\n";
            decode(@data);
        }

        $goodpacket = 0;
        @data = ();
    }

    # looking for a timestamp
    if ($line =~ /^[A-Z][a-z]{2} \d{2} \d{4}/) {
        $timestamp = $line;
    }

    # looking for a charprofile packet
    if ($line =~ /^\[OPCode: 0x1f1c\]/) {
        $goodpacket = 1;
    }

    if ($goodpacket && $line =~ /^\d+ \| ([0-9a-f ]+) \|/) {
        $line = $1;
        while ($line =~ /([0-9a-f]{2})/mcg) {
            push(@data, hex($1));
        }
    }

}
close(PACKET);


sub decode {

    @data = @_;
    print "Loaded ". ($#data + 1) ." bytes.\n";

    # fill in struct
    my $offset = 0;
    foreach my $i (0..$#struct) {
        my $type = $struct[$i]{'type'};
        my $name = $struct[$i]{'name'};
        my $alen = 0;
        my $size = 0;

        if ($name =~ /(\S+)\[([0-9]+)\]/) {
            $name = $1;
            $alen = $2 - 1;
        }

        if ($alen > 0) {
            printf("%04d: %s\t%s[%d]: ", $offset, $type, $name, $alen + 1);
        } else {
            printf("%04d: %s\t%s: ", $offset, $type, $name);
        }

        if ($type eq "uint8_t") {
            $size = 1;
            foreach my $j (0..$alen) {
                print unpack("C", pack("C", $data[$offset + $j * $size])) ." ";
            }

        } elsif ($type eq "uint16_t") {
            $size = 2;
            foreach my $j (0..$alen) {
                print unpack("S", pack("CC", $data[$offset + $j * $size], $data[$offset + 1 + $j * $size])) ." ";
            }

        } elsif ($type eq "uint32_t") {
            $size = 4;
            foreach my $j (0..$alen) {
                print unpack("L", pack("CCCC", $data[$offset + $j * $size], $data[$offset + 1 + $j * $size],
                    $data[$offset + 2 + $j * $size], $data[$offset + 3 + $j * $size])) ." ";
            }

        } elsif ($type eq "int8_t") {
            $size = 1;
            foreach my $j (0..$alen) {
                print unpack("c", pack("C", $data[$offset + $j * $size])) ." ";
            }

        } elsif ($type eq "int16_t") {
            $size = 2;
            foreach my $j (0..$alen) {
                print unpack("s", pack("CC", $data[$offset + $j * $size], $data[$offset + 1 + $j * $size])) ." ";
            }

        } elsif ($type eq "int32_t") {
            $size = 4;
            foreach my $j (0..$alen) {
                print unpack("l", pack("CCCC", $data[$offset + $j * $size], $data[$offset + 1 + $j * $size],
                    $data[$offset + 2 + $j * $size], $data[$offset + 3 + $j * $size])) ." ";
            }

        } elsif ($type eq "float") {
            $size = 4;
            foreach my $j (0..$alen) {
                my $num = pack("CCCC", $data[$offset + 0], $data[$offset + 1],
                    $data[$offset + 2], $data[$offset + 3]);

                print unpack("f", $num) ." ";
            }

        } elsif ($type eq "char") {
            foreach my $j (0..$alen) {
                if ($data[$offset + $j] > 0x1f && $data[$offset + $j] < 0x80) {
                    printf("%c", $data[$offset+$j]);
                } else {
                    print ".";
                }
            }
            $size = 1;

        } elsif ($type eq "Color_Struct") {
            # B R G A
            $size = 4;
        } elsif ($type eq "AA_Array") {
            # AA value
            $size = 8;
        } elsif ($type eq "spellBuff") {
            # unknown0000 level unknown0002 unknown003 slotid duration effect playerId
            $size = 20;
                } elsif ($type eq "TributeStruct") {
                        # tribute value
                        $size = 8;
                } elsif ($type eq "BandolierStruct") {
                        # char[32] mainHand offHand range ammo
                        $size = 320;
        } elsif ($type eq "InlineItem") {
            # itemId icon itemName[64]
            $size = 72;
        } else {
            die "Unknown type: $type\n";

        }

        print "\n";

        $offset += ($size * ($alen + 1));
    }

    printf("%04d: END\n", $offset);
}
