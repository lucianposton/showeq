/*
 * dump-seqdb.cpp
 *
 * A standalone utility included with ShowEQ
 * http://sourceforge.net/projects/seq
 *
 * This formats and displays the contents of the
 * spawn.db, item.db, or pet.db file
 *
 * Originally created by cpphack
 */

/* Plan to add:
   + working spawn.db parser (having problems finding valid data in the file)
   + pet.db parser
   + csv format output
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "conf.h"
#include "everquest.h"
#include "dump-seqdb.h"

#define OPTION_LIST "hLr:w:sip"

#define VERSION_OPTION                10

static struct option option_list[] = {
  {"help",no_argument,NULL,'h'},
  {"version",no_argument,NULL,VERSION_OPTION},
  {"long-format",no_argument,NULL,'L'},
  {"input-filename",required_argument,NULL,'r'},
  {"output-filename",required_argument,NULL,'w'},
  {"spawns",no_argument,NULL,'s'},
  {"items",no_argument,NULL,'i'},
  {"pets",no_argument,NULL,'p'},
  {0,0,0,0}
};

/* Make these available all over */
struct DumpSEQDBParams *params;
FILE *inputfile;
FILE *outputfile;

int main (int argc, char **argv) {
  int option;
  int option_index = 0;

  params = (struct DumpSEQDBParams *) malloc (sizeof (struct DumpSEQDBParams));

  /* Collect command line options and put them into paramater
     list (params) */
  while ((option = getopt_long(argc, argv, OPTION_LIST,
			       option_list, &option_index )) != -1)
    {
      switch(option) {
      case VERSION_OPTION:
	version();
	exit(0);
	break;

      case 'h':
	usage(argv[0]);
	exit(0);
	break; /* Somewhat unneeded... */

      case 'L':
	params->longformat = 1;
	break;

      case 'r':
	/* File to read for info */
	params->inputfile = optarg;
	break;

      case 'w':
	/* File to write to */
	params->outputfile = optarg;
	break;

      case 's':
	/* Read the file as spawn data */
	params->dbtype = 1;
	break;

      case 'i':
	/* Read the file as item data */
	params->dbtype = 2;
	break;
	
      case 'p':
	/* Read the file as pet data */
	params->dbtype = 3;
	break;

      default:
	usage(argv[0]);
	exit(1);
      }
    }

  /* Eventually this will check for all other format types, and default
     back to longformat. */
  if (!(params->longformat)) {
    params->longformat = 1;
  }

  /* Open up our input/output files, falling back to stdin/stdout
     if none specified */
  if (params->inputfile) {
    inputfile = fopen(params->inputfile, "r");
    if (inputfile == NULL) {
      printf ("Can't open '%s' for reading: %s", params->inputfile,
	      strerror(errno));
      exit(1);
    }
  } else {
    inputfile = stdin;
  }

  if (params->outputfile) {
    outputfile = fopen(params->outputfile, "a");
    if (outputfile == NULL) {
      printf ("Can't open '%s' for appending: %s", params->outputfile,
	      strerror(errno));
      exit(1);
    }
  } else {
    outputfile = stdout;
  }


  /* Choose your demangler. */
  switch (params->dbtype) {
  case 1: /* Spawns */
    parsespawns();
    break;

  case 2: /* Items */
    parseitems();
    break;
    
  case 3: /* Pets */
    parsepets();
    break;
   
  default:
    printf("Please specify the input data format (-i, -s, -p)\n");
  }

}

void version(void) {
  printf ("SEQ_dump_db %s\n",VERSION);
  printf ("Copyright (C) 1999-2000 ShowEQ Contributors\n");
  printf ("ShowEQ (including dump-seqdb) comes with NO WARRANTY.\n");
  printf ("You may redistribute copies of ShowEQ under the terms of\n");
  printf ("The GNU General Public License.\n");
  printf ("For updates and information, please visit http://sourceforge.net/projects/seq\n");
}

void usage (char *progname) {
  printf ("Usage:\n %s [<options>]\n\n", progname);
  printf ("  -h, --help                         shows this screen\n");
  printf ("      --version                      prints dump-seqdb version number\n");
  printf ("  -r, --input-filename=FILE          read from FILE\n");
  printf ("  -w, --output-filename=FILE         write to FILE\n");
  printf ("  -s, --spawns                       treat input as spawn data [broken]\n");
  printf ("  -i, --items                        treat input as item data\n");
  printf ("  -p, --pets                         treat input as pet data [broken]\n");
  printf ("  -L, --long-format                  print one attribute per line, labelled\n");
}

void parseitems(void) {
  struct itemStruct *items;

  /* malloc() a place to load all that item data */
  items = (struct itemStruct *) malloc (sizeof (struct itemStruct));

  if (params->longformat) {
    while (fread (items, sizeof(itemStruct), 1, inputfile)) {
      /* Begin the printing hell.  The helper script "dump-seqdb-prep.pl"
         made this a lot easier.
      */
      printf("*********\n");
      stringprint(LONG_FORMAT,"name",items->name);
      stringprint(LONG_FORMAT,"lore",items->lore);
      stringprint(LONG_FORMAT,"idfile",items->idfile);
      signednumprint(LONG_FORMAT,"flag",(long int) items->flag);
      unsignednumprint(LONG_FORMAT,"weight",(long int) items->weight);
      signednumprint(LONG_FORMAT,"nosave",(long int) items->nosave);
      signednumprint(LONG_FORMAT,"nodrop",(long int) items->nodrop);
      unsignednumprint(LONG_FORMAT,"size",(long int) items->size);
      unsignednumprint(LONG_FORMAT,"itemNr",(long int) items->itemNr);
      unsignednumprint(LONG_FORMAT,"iconNr",(long int) items->iconNr);
      signednumprint(LONG_FORMAT,"equipSlot",(long int) items->equipSlot);
      unsignednumprint(LONG_FORMAT,"equipableSlots",(long int) items->equipableSlots);
      signednumprint(LONG_FORMAT,"cost",(long int) items->cost);
      signednumprint(LONG_FORMAT,"STR",(long int) items->common.STR);
      signednumprint(LONG_FORMAT,"STA",(long int) items->common.STA);
      signednumprint(LONG_FORMAT,"CHA",(long int) items->common.CHA);
      signednumprint(LONG_FORMAT,"DEX",(long int) items->common.DEX);
      signednumprint(LONG_FORMAT,"INT",(long int) items->common.INT);
      signednumprint(LONG_FORMAT,"AGI",(long int) items->common.AGI);
      signednumprint(LONG_FORMAT,"WIS",(long int) items->common.WIS);
      signednumprint(LONG_FORMAT,"MR",(long int) items->common.MR);
      signednumprint(LONG_FORMAT,"FR",(long int) items->common.FR);
      signednumprint(LONG_FORMAT,"CR",(long int) items->common.CR);
      signednumprint(LONG_FORMAT,"DR",(long int) items->common.DR);
      signednumprint(LONG_FORMAT,"PR",(long int) items->common.PR);
      signednumprint(LONG_FORMAT,"HP",(long int) items->common.HP);
      signednumprint(LONG_FORMAT,"MANA",(long int) items->common.MANA);
      signednumprint(LONG_FORMAT,"AC",(long int) items->common.AC);
      unsignednumprint(LONG_FORMAT,"light",(long int) items->common.light);
      unsignednumprint(LONG_FORMAT,"delay",(long int) items->common.delay);
      unsignednumprint(LONG_FORMAT,"damage",(long int) items->common.damage);
      unsignednumprint(LONG_FORMAT,"range",(long int) items->common.range);
      unsignednumprint(LONG_FORMAT,"skill",(long int) items->common.skill);
      signednumprint(LONG_FORMAT,"magic",(long int) items->common.magic);
      signednumprint(LONG_FORMAT,"level0",(long int) items->common.level0);
      unsignednumprint(LONG_FORMAT,"material",(long int) items->common.material);
      unsignednumprint(LONG_FORMAT,"color",(long int) items->common.color);
      unsignednumprint(LONG_FORMAT,"spellId0",(long int) items->common.spellId0);
      unsignednumprint(LONG_FORMAT,"classes",(long int) items->common.classes);
      unsignednumprint(LONG_FORMAT,"races",(long int) items->common.normal.races);
      signednumprint(LONG_FORMAT,"numSlots",(long int) items->container.numSlots);
      unsignednumprint(LONG_FORMAT,"level",(long int) items->common.level);
      signednumprint(LONG_FORMAT,"number",(long int) items->common.number);
      unsignednumprint(LONG_FORMAT,"spellId",(long int) items->common.spellId);
    }
  } else {
    printf("Only long format presently supported");
    exit(1);
  }
}

void parsespawns(void) {
  struct dbSpawnStruct *spawn;

  spawn = (struct dbSpawnStruct *) malloc (sizeof (struct dbSpawnStruct));

  if (params->longformat) {
    while (fread (spawn, sizeof(dbSpawnStruct), 1, inputfile)) {
      dumpspawn( spawn->spawn.name, &(spawn->spawn));
      stringprint(LONG_FORMAT,"zoneName",spawn->zoneName);
    }
  } else {
    printf("Only long format presently supported");
    exit(1);
  }
}

void parsepets(void) {
  struct petStruct *pet;
  char name[50];

  pet = (struct petStruct *) malloc (sizeof (struct petStruct));

  if (params->longformat) {
    while (fread (pet, sizeof(petStruct), 1, inputfile)) {
      sprintf(name,"Owner of %s",pet->pet.name);
      dumpspawn( name, &pet->owner);
      sprintf(name,"Pet of %s",pet->owner.name);
      dumpspawn( name, &pet->pet);
    }
  } else {
    printf("Only long format presently supported");
    exit(1);
  }


}

/* Useful for the "pet" struct which is really just two spawnStructs
   stuck together.  The name distinguishes pet from owner */
void dumpspawn(const char *name, struct spawnStruct *spawns) {
  int equipment_index = 0;
  char equipment_string[15];

  /* Unfortunately, the data in my spawn.db file does not appear to
     match the format in everquest.h.  My guess is that the format
     changed at some point, rendering the file corrupted at some
     point in the middle. */

  printf("***** %s ******\n", name);
  signednumprint(LONG_FORMAT,"heading",(long int) spawns->heading);
  signednumprint(LONG_FORMAT,"deltaHeading",(long int) spawns->deltaHeading);
  signednumprint(LONG_FORMAT,"y",(long int) spawns->y);
  signednumprint(LONG_FORMAT,"x",(long int) spawns->x);
  signednumprint(LONG_FORMAT,"z",(long int) spawns->z);
  signednumprint(LONG_FORMAT,"deltaY",(long int) spawns->deltaY);
  signednumprint(LONG_FORMAT,"spacer1",(long int) spawns->spacer1);
  signednumprint(LONG_FORMAT,"deltaZ",(long int) spawns->deltaZ);
  signednumprint(LONG_FORMAT,"spacer2",(long int) spawns->spacer2);
  signednumprint(LONG_FORMAT,"deltaX",(long int) spawns->deltaX);
  unsignednumprint(LONG_FORMAT,"spawnId",(long int) spawns->spawnId);
  unsignednumprint(LONG_FORMAT,"petOwnerId",(long int) spawns->petOwnerId);
  signednumprint(LONG_FORMAT,"maxHp",(long int) spawns->maxHp);
  signednumprint(LONG_FORMAT,"curHp",(long int) spawns->curHp);
  unsignednumprint(LONG_FORMAT,"race",(long int) spawns->race);
  unsignednumprint(LONG_FORMAT,"NPC",(long int) spawns->NPC);
  unsignednumprint(LONG_FORMAT,"class_",(long int) spawns->class_);
  unsignednumprint(LONG_FORMAT,"level",(long int) spawns->level);
  unsignednumprint(LONG_FORMAT,"light",(long int) spawns->light);
  for (equipment_index = 0; equipment_index < 9; equipment_index++) {
    sprintf(equipment_string,"equipment[%d]",equipment_index);
    unsignednumprint(LONG_FORMAT,equipment_string,
		     (long int) spawns->equipment[equipment_index]);
  }
  stringprint(LONG_FORMAT,"name",spawns->name);
  stringprint(LONG_FORMAT,"lastname",spawns->lastname);
}


/* Genericish print routine for string values */
void stringprint(int format, char *name, char *string) {
  /* This is the user screen format, not a printf-like format */
  if (format == LONG_FORMAT) {
    printf("%s: '%s'\n", name, string);
  } else {
    printf("Unknown format type: %d\n", format);
    exit (1);
  }
}

void signednumprint(int format, char *name, long int value) {
  if (format == LONG_FORMAT) {
    printf("%s: %ld\n", name, value);
  } else {
    printf("Unknown format type: %d\n", format);
    exit (1);
  }
}

void unsignednumprint(int format, char *name, unsigned long int value) {
  if (format == LONG_FORMAT) {
    printf("%s: %lu\n", name, value);
  } else {
    printf("Unknown format type: %d\n", format);
    exit (1);
  }
}
