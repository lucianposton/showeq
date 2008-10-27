/*
 * weapons.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */
 
#ifndef DOXYGEN_SHOULD_SKIP_THIS 
    "Generic",              // 0x00
    "1HSword",              // 0x01
    "2HSword",              // 0x02
    "Axe",                  // 0x03
    "Bow",                  // 0x04
    "Dagger",               // 0x05
    "Flute",                // 0x06
    "Mace",                 // 0x07
    "2HStaff",              // 0x08
    "DwarvenAxe",           // 0x09
    "Arrow",                // 0x0a
    NULL,                   // 0x0b - Unknown Weapon
    NULL,                   // 0x0c - Unknown Weapon
    NULL,                   // 0x0d - Unknown Weapon
    "WarHammer",            // 0x0e
    "Trumpet",              // 0x0f
    "Spear",                // 0x10
    "ShortSpear",           // 0x11
    "Club",                 // 0x12
    "MorningStar",          // 0x13
    "Rapier",               // 0x14
    "Lute",                 // 0x15
    NULL,                   // 0x16 - Unknown Weapon
    "Halberd",              // 0x17
    "2HHammer",             // 0x18
    "2HBattleAxe",          // 0x19
    "IcyBlade(P)",          // 0x1a
    "Book",                 // 0x1b
    "DarkBook",             // 0x1c
    "WrithingStaff",        // 0x1d
    NULL,                   // 0x1e - Unknown Weapon
    "SpikeClub",            // 0x1f
    "Broom",                // 0x20
    "HammFlat",             // 0x21
    "Shortsword",           // 0x22
    "Scepter",              // 0x23
    "Torch",                // 0x24
    "Cudgel",               // 0x25
    "Fishing Pole",         // 0x26
    "Scythe",               // 0x27
    "Harvester",            // 0x28
    "Scimitar",             // 0x29
    "Falchion",             // 0x2a
    "Pick",                 // 0x2b
    NULL,                   // 0x2c - Unknown Weapon
    "CrystalStaff",         // 0x2d
    "BoneWand",             // 0x2e
    "Wand",                 // 0x2f
    "Lantern",              // 0x30
    "Maul",                 // 0x31
    "Dirk",                 // 0x32
    "GoldScepter",          // 0x33
    "Shovel",               // 0x34
    "Flamberge",            // 0x35
    NULL,                   // 0x36 - Unknown Weapon
    NULL,                   // 0x37 - Unknown Weapon
    "Stein",                // 0x38
    "BroadSword",           // 0x39
    "BastardSword",         // 0x3a
    "MiningPick",           // 0x3b
    "BattleAxe",            // 0x3c
    "Whip",                 // 0x3d
    "FlameSword",           // 0x3e
    "Generic",              // 0x3f
    "Generic",              // 0x40
    "Letter",               // 0x41
    "Forge",                // 0x42
    "Doll",                 // 0x43
    "Manastone",            // 0x44
    "Oven",                 // 0x45
    "BrewBarrel",           // 0x46
    "Claws",                // 0x47
    "Stone",                // 0x48
    "Kiln",                 // 0x49
    "PotteryWheel",         // 0x4a
    "WoodenCrook",          // 0x4b
    "Vah Shir Sword",       // 0x4c 
    NULL,                   // 0x4d - Unknown Weapon
    NULL,                   // 0x4e - Unknown Weapon
    NULL,                   // 0x4f - Unknown Weapon
    "ExecutionerAxe",       // 0x50
    "Lamentation",          // 0x51
    "Fer'Esh",              // 0x52
    "Silver2HAxe",          // 0x53
    "GreatCleaver",         // 0x54
    "SerratedSword",        // 0x55
    "Falchion",             // 0x56
    "YkeshaSS",             // 0x57
    "Swarmcaller",          // 0x58
    NULL,                   // 0x59 - Unknown Weapon
    "SwordPassage",         // 0x5a
    "Ulak",                 // 0x5b
    "Wurmslayer",           // 0x5c
    NULL,                   // 0x5d - Unknown Weapon
    "Duster/Sapper",        // 0x5e
    "YkeshaTB",             // 0x5f
    NULL,                   // 0x60 - Unknown Weapon
    NULL,                   // 0x61 - Unknown Weapon
    "Tooth",                // 0x62
    "Bladecatcher",         // 0x63
    "SheerBlade",           // 0x64
    "Ketchata",             // 0x65
    NULL,                   // 0x66 - Unknown Weapon
    "PartisanSpear",        // 0x67
    "KunzarKu'juch",        // 0x68
    "Shan'Tok",             // 0x69
    NULL,                   // 0x6a - Unknown Weapon
    NULL,                   // 0x6b - Unknown Weapon
    "Sword",                // 0x6c
    NULL,                   // 0x6d - Unknown Weapon
    "LegChopper",           // 0x6e
    NULL,                   // 0x6f - Unknown Weapon
    NULL,                   // 0x70 - Unknown Weapon
    "Ta'Nak",               // 0x71
    NULL,                   // 0x72 - Unknown Weapon
    "Mallet",               // 0x73
    NULL,                   // 0x74 - Unknown Weapon
    "Re'Stek",              // 0x75
    "Envy",                 // 0x76
    "LupineDagger",         // 0x77
    "BlackTranslucentBlade",// 0x78
    "WhiteTranslucentBlade",// 0x79
    "Velium2hStaff",        // 0x7a
    "Ch'Ror",               // 0x7b
    NULL,                   // 0x7c - Unknown Weapon
    "SarnakSkullStaff",     // 0x7d
    "WoodStaff",            // 0x7e
    NULL,                   // 0x7f - Unknown Weapon
    "Loom",                 // 0x80
    NULL,                   // 0x81 - Unknown Weapon
    "MorningStar",          // 0x82
    "Defiance",             // 0x83
    "Harpoon",              // 0x84
    "Bowl",                 // 0x85
    NULL,                   // 0x86 - Unknown Weapon
    NULL,                   // 0x87 - Unknown Weapon
    "Claw",                 // 0x88
    "Tambourine",           // 0x89 - Unknown Weapon
    NULL,                   // 0x8a - Unknown Weapon
    NULL,                   // 0x8b - Unknown Weapon
    "Ragebringer",          // 0x8c
    "WarEpic1hBlue",        // 0x8d
    "WarEpic1hRed",         // 0x8e
    NULL,                   // 0x8f - Unknown Weapon
    NULL,                   // 0x90 - Unknown Weapon
    NULL,                   // 0x91 - Unknown Weapon
    NULL,                   // 0x92 - Unknown Weapon
    NULL,                   // 0x93 - Unknown Weapon
    "BardEpic",             // 0x94
    "RangerEpic",           // 0x95
    "DruidEpic",            // 0x96
    "MageEpic",             // 0x97
    NULL,                   // 0x98 - Unknown Weapon
    "NecroEpic",            // 0x99
    "ShamanEpic",           // 0x9a
    "WizardEpic",           // 0x9b
    "ClericEpic",           // 0x9c
    "EncEpic",              // 0x9d
    NULL,                   // 0x9e - Unknown Weapon
    "MonkEpic",             // 0x9f
    "PaladinEpic",          // 0xa0
    NULL,                   // 0xa1 - Unknown Weapon
    NULL,                   // 0xa2 - Unknown Weapon
    "RedJeweledSword",      // 0xa3
    "BlueJeweledSword",     // 0xa4
    NULL,                   // 0xa5 - Unknown Weapon
    "ShissarEmperorsSword", // 0xa6
    "JeweledDirk",          // 0xa7
    "VeliumSword",          // 0xa8
    NULL,                   // 0xa9 - Unknown Weapon
    "Velium2hAxe",          // 0xaa
    "VeliumAxe",            // 0xab
    "VeliumSpear",          // 0xac
    NULL,                   // 0xad - Unknown Weapon
    NULL,                   // 0xae - Unknown Weapon
    NULL,                   // 0xaf - Unknown Weapon
    NULL,                   // 0xb0 - Unknown Weapon
    NULL,                   // 0xb1 - Unknown Weapon
    "VeliumAltHammer",      // 0xb2
    "VeliumHammer",         // 0xb3
    "CrystalSword",         // 0xb4
    NULL,                   // 0xb5 - Unknown Weapon
    NULL,                   // 0xb6 - Unknown Weapon
    NULL,                   // 0xb7 - Unknown Weapon
    NULL,                   // 0xb8 - Unknown Weapon
    "GoldHammer",           // 0xb9
    "VeliumGreatStaff",     // 0xba
    "VeliumSpear",          // 0xbb
    "VeliumLongSword",      // 0xbc
    "VeliumMorningStar",    // 0xbd
    "VeliumTwoHandedSword", // 0xbe
    "VeliumShortSword",     // 0xbf
    "VeliumScimitar",       // 0xc0
    "VeliumWarhammer",      // 0xc1
    "VeliumDagger",         // 0xc2
    "VeliumRapier",         // 0xc3
    NULL,                   // 0xc4 - Unknown Weapon
    "VeliumLance",          // 0xc5
    "BFG",                  // 0xc6
    "Crossbow",             // 0xc7
    "QeynosShield",         // 0xc8
    "WoodenShield",         // 0xc9
    "KiteShield",           // 0xca
    "SmRoundShield",        // 0xcb
    "DarkwoodShield",       // 0xcc
    "BoneShield",           // 0xcd
    "DarkShield",           // 0xce
    "MarrsPromise",         // 0xcf
    NULL,                   // 0xd0 - Unknown Weapon
    "ShellShield1",         // 0xd1
    "ShimmerOrb",           // 0xd2
    "UnicornShield",        // 0xd3
    "NautilusShield",       // 0xd4
    "MistmooreShield",      // 0xd5
    "ChitinShield",         // 0xd6
    "FearShield",           // 0xd7
    "IksTargShield",        // 0xd8
    "FrogskinShield",       // 0xd9
    "ScaleShield",          // 0xda
    "ShellShield2",         // 0xdb
    NULL,                   // 0xdc - Unknown Weapon
    "SarnakShield",         // 0xdd
    NULL,                   // 0xde - Unknown Weapon
    "OrnateRunedShield",    // 0xdf
    NULL,                   // 0xe0 - Unknown Weapon
    NULL,                   // 0xe1 - Unknown Weapon
    "GreyShield",           // 0xe2
    NULL,                   // 0xe3 - Unknown Weapon
    "VeliumRoundShield",    // 0xe4
    NULL,                   // 0xe5 - Unknown Weapon
    "Vah Shir Shield",      // 0xe6
    NULL,                   // 0xe7 - Unknown Weapon
    NULL,                   // 0xe8 - Unknown Weapon
    NULL,                   // 0xe9 - Unknown Weapon
    NULL,                   // 0xea - Unknown Weapon
    NULL,                   // 0xeb - Unknown Weapon
    NULL,                   // 0xec - Unknown Weapon
    NULL,                   // 0xed - Unknown Weapon
    NULL,                   // 0xee - Unknown Weapon
    NULL,                   // 0xef - Unknown Weapon
    NULL,                   // 0xf0 - Unknown Weapon
    NULL,                   // 0xf1 - Unknown Weapon
    NULL,                   // 0xf2 - Unknown Weapon
    NULL,                   // 0xf3 - Unknown Weapon
    NULL,                   // 0xf4 - Unknown Weapon
    NULL,                   // 0xf5 - Unknown Weapon
    NULL,                   // 0xf6 - Unknown Weapon
    NULL,                   // 0xf7 - Unknown Weapon
    NULL,                   // 0xf8 - Unknown Weapon
    NULL,                   // 0xf9 - Unknown Weapon
    "Lollipop",             // 0xfa - Unknown Weapon
    NULL,                   // 0xfb - Unknown Weapon
    NULL,                   // 0xfc - Unknown Weapon
    NULL,                   // 0xfd - Unknown Weapon
    NULL,                   // 0xfe - Unknown Weapon
    NULL,                   // 0xff - Unknown Weapon
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

