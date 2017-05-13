/*
 * weapons.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */
 
#ifndef DOXYGEN_SHOULD_SKIP_THIS 
    NULL,                   // 0x00 -- Nothing
    "Sword",                // 0x01
    "2hSword",              // 0x02
    "Tomahawk",             // 0x03
    "Bow",                  // 0x04
    "Dagger",               // 0x05
    "Flute",                // 0x06
    "Mace",                 // 0x07
    "Staff",                // 0x08
    "MinoAxe",              // 0x09
    "Arrow",                // 0x0a
    "ArtifactSword",        // 0x0b
    "ArtifactStaff",        // 0x0c
    "ArtifactScepter",      // 0x0d
    "Warhammer",            // 0x0e
    "Horn",                 // 0x0f
    "Spear",                // 0x10
    "ShortSpear",           // 0x11
    "Club",                 // 0x12
    "MorningStar",          // 0x13
    "Rapier",               // 0x14
    "Lute",                 // 0x15
    "BluntSword",           // 0x16
    "Halberd",              // 0x17
    "2hHammer",             // 0x18
    "2hBattleAxe",          // 0x19
    "LongSword",            // 0x1a
    "Book",                 // 0x1b
    "OpenBook",             // 0x1c
    "SkullStaff",           // 0x1d
    "Fork",                 // 0x1e
    "SpikeClub",            // 0x1f
    "Broom",                // 0x20
    "WoodMallet",           // 0x21
    "Shortsword",           // 0x22
    "Scepter",              // 0x23
    "Torch",                // 0x24
    "GuardiansMace",        // 0x25
    "FishingPole",          // 0x26
    "Scythe",               // 0x27
    "Sickle",               // 0x28
    "Scimitar",             // 0x29
    "WingBlade",            // 0x2a
    "MiningPick",           // 0x2b
    "Katana",               // 0x2c
    "CrystalStaff",         // 0x2d
    "Wand",                 // 0x2e
    "SilverWand",           // 0x2f
    "Lantern",              // 0x30
    "Maul",                 // 0x31
    "Dirk",                 // 0x32
    "GoldScepter",          // 0x33
    "Shovel",               // 0x34
    "Dark2hSword",          // 0x35
    "Note",                 // 0x36
    "Pipe",                 // 0x37
    "Stein",                // 0x38
    "BroadSword",           // 0x39
    "BastardSword",         // 0x3a
    "Pick",                 // 0x3b
    "BattleAxe",            // 0x3c
    "Whip",                 // 0x3d
    "FlameSword",           // 0x3e
    "Generic",              // 0x3f
    "Bag",                  // 0x40
    "Letter",               // 0x41
    "Forge",                // 0x42
    "Doll",                 // 0x43
    "GlowingOrb",           // 0x44
    "Oven",                 // 0x45
    "BrewBarrel",           // 0x46
    "Claws",                // 0x47
    "CrystalBall",          // 0x48
    "Kiln",                 // 0x49
    "PotteryWheel",         // 0x4a
    "Crook",                // 0x4b
    "VahShirSword",         // 0x4c
    NULL,                   // 0x4d -- Nothing
    "CampFire",             // 0x4e
    "Trident",              // 0x4f
    "ExecutionerAxe",       // 0x50
    "Lamentation",          // 0x51
    "Fer'Esh",              // 0x52
    "Silver2hAxe",          // 0x53
    "Gold2hAxe",            // 0x54
    "SerratedSword",        // 0x55
    "Falchion",             // 0x56
    "Kukri",                // 0x57
    "Swarmcaller",          // 0x58
    "CurvedSword",          // 0x59
    "CeremonialSword",      // 0x5a
    "Ulak",                 // 0x5b
    "Wurmslayer",           // 0x5c
    "OrnateRuneBlade",      // 0x5d
    "KnuckleDusters",       // 0x5e
    "Claidhmore",           // 0x5f
    "ArtifactWarStaff",     // 0x60
    "CelestialSword",       // 0x61
    "Katar",                // 0x62
    "Bladecatcher",         // 0x63
    "SheerBlade",           // 0x64
    "Ketchata",             // 0x65
    "DwarvenSap",           // 0x66
    "Partisan",             // 0x67
    "KunzarKu'juch",        // 0x68
    "Shan'Tok",             // 0x69
    "SarnakCeremonialSword",// 0x6a
    "BarbedSword",          // 0x6b
    "AcryliaBlade",         // 0x6c
    "SwordOfSkyfire",       // 0x6d
    "LegChopper",           // 0x6e
    "SpikedStaff",          // 0x6f
    "KegPopper",            // 0x70
    "DiamondBlade",         // 0x71
    NULL,                   // 0x72 - Nothing
    "Mallet",               // 0x73
    NULL,                   // 0x74 - Nothing
    "Truncheon",            // 0x75
    "ParryingDagger",       // 0x76
    "LongDagger",           // 0x77
    "BlackTranslucentBlade",// 0x78
    "TranslucentBlade",     // 0x79
    "TranslucentStaff",     // 0x7a
    "tStaff",               // 0x7b
    "SoulBinder",           // 0x7c
    "SarnakSkullStaff",     // 0x7d
    "WarStaff",             // 0x7e
    "JeweledWarStaff",      // 0x7f
    "Loom",                 // 0x80
    "IksarSkullStaff",      // 0x81
    "FlangedMace",          // 0x82
    "DarkCutlass",          // 0x83
    "Harpoon",              // 0x84
    "Bowl",                 // 0x85
    "OrnateScepter",        // 0x86
    "FistLoadWeapon",       // 0x87
    "ClawsLeft",            // 0x88
    "Tambourine",           // 0x89
    "Mandolin",             // 0x8a
    "Stiletto",             // 0x8b
    "RogEpic",              // 0x8c
    "WarEpic1hBlue",        // 0x8d
    "WarEpic1hRed",         // 0x8e
    NULL,                   // 0x8f - Nothing
    NULL,                   // 0x90 - Nothing
    "SKEpic",               // 0x91
    "WarEpic2h",            // 0x92
    NULL,                   // 0x93 - Nothing
    "BrdEpic",              // 0x94
    "RngEpic",              // 0x95
    "DruEpic",              // 0x96
    "MagEpic",              // 0x97
    NULL,                   // 0x98 - Nothing
    "NecEpic",              // 0x99
    "ShmEpic",              // 0x9a
    "WizEpic",              // 0x9b
    "ClrEpic",              // 0x9c
    "EncEpic",              // 0x9d
    NULL,                   // 0x9e - Nothing
    "MnkEpic",              // 0x9f
    "PalEpic",              // 0xa0
    "VelBastardSword",      // 0xa1
    "RedJeweledShortSword", // 0xa2
    "RedJeweledSword",      // 0xa3
    "GreenJeweledSword",    // 0xa4
    "GreenJeweledFalchion", // 0xa5
    "ShissarEmperorsSword", // 0xa6
    "JeweledDirk",          // 0xa7
    "VeliumWarsword",       // 0xa8
    "VeliumBrawlStick",     // 0xa9
    "VeliumBattleAxe",      // 0xaa
    "Velium1hAxe",          // 0xab
    "JeweledSpear",         // 0xac
    "RedInlaidSword",       // 0xad
    "RedInlaidWarsword",    // 0xae
    "VelKatana",            // 0xaf
    "Naginata",             // 0xb0
    "VeliumDoubleHammer",   // 0xb1
    "VeliumMaul",           // 0xb2
    "VeliumBattlehammer",   // 0xb3
    "CrystalSword",         // 0xb4
    "GreatAxe",             // 0xb5
    "VeliumGreatAxe",       // 0xb6
    "BlueJeweledSword",     // 0xb7
    "Infestation",          // 0xb8
    "GoldHammer",           // 0xb9
    "VeliumGreatStaff",     // 0xba
    "VeliumSpear",          // 0xbb
    "VeliumLongSword",      // 0xbc
    "VeliumMorningStar",    // 0xbd
    "Velium2hSword",        // 0xbe
    "VeliumShortSword",     // 0xbf
    "VeliumScimitar",       // 0xc0
    "VeliumWarhammer",      // 0xc1
    "VeliumDagger",         // 0xc2
    "VeliumRapier",         // 0xc3
    "Snowchipper",          // 0xc4
    "VeliumLance",          // 0xc5
    "BFG",                  // 0xc6
    "Crossbow",             // 0xc7
    "QeynosShield",         // 0xc8
    "Shield",               // 0xc9
    "KiteShield",           // 0xca
    "RoundShield",          // 0xcb
    "DarkwoodShield",       // 0xcc
    "BoneShield",           // 0xcd
    "LeatherShield",        // 0xce
    "MarrsPromise",         // 0xcf
    "HideBuckler",          // 0xd0
    "Buckler",              // 0xd1
    "ShimmerOrb",           // 0xd2
    "UnicornShield",        // 0xd3
    "RibCageShield",        // 0xd4
    "MistmooreShield",      // 0xd5
    "ChitinShield",         // 0xd6
    "FearShield",           // 0xd7
    "IksTargShield",        // 0xd8
    "FrogskinShield",       // 0xd9
    "ScaleShield",          // 0xda
    "RedScaleShield",       // 0xdb
    "ShellShield",          // 0xdc
    "SarnakShield",         // 0xdd
    "OracleShield",         // 0xde
    "OrnateRuneShield",     // 0xdf
    "StoneShield",          // 0xe0
    "BarnacleShield",       // 0xe1
    "GreyHeaterShield",     // 0xe2
    "DragonShield",         // 0xe3
    "VeliumRoundShield",    // 0xe4
    NULL,                   // 0xe5 - Nothing
    NULL,                   // 0xe6 - Nothing
    NULL,                   // 0xe7 - Nothing
    NULL,                   // 0xe8 - Nothing
    NULL,                   // 0xe9 - Nothing
    NULL,                   // 0xea - Nothing
    NULL,                   // 0xeb - Nothing
    NULL,                   // 0xec - Nothing
    NULL,                   // 0xed - Nothing
    NULL,                   // 0xee - Nothing
    NULL,                   // 0xef - Nothing
    NULL,                   // 0xf0 - Nothing
    NULL,                   // 0xf1 - Nothing
    NULL,                   // 0xf2 - Nothing
    NULL,                   // 0xf3 - Nothing
    NULL,                   // 0xf4 - Nothing
    NULL,                   // 0xf5 - Nothing
    NULL,                   // 0xf6 - Nothing
    NULL,                   // 0xf7 - Nothing
    NULL,                   // 0xf8 - Nothing
    NULL,                   // 0xf9 - Nothing
    "Lollipop",             // 0xfa
    NULL,                   // 0xfb - Nothing
    NULL,                   // 0xfc - Nothing
    NULL,                   // 0xfd - Nothing
    NULL,                   // 0xfe - Nothing
    NULL,                   // 0xff - Nothing
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

