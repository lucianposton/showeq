/*
 * dump-seqdb.h
 *
 * ShowEQ Distributed under GPL
 * http://sourceforge.net/projects/seq
 */

#ifndef _SHOWEQ_DUMP_DB_H
#define _SHOWEQ_DUMP_DB_H

#define LONG_FORMAT 10

struct DumpSEQDBParams {
  const char *inputfile;
  const char *outputfile;

  int dbtype;   // 1=spawns, 2=items, 3=pets
  int longformat;

};

void version(void);
void usage (char *);
void parseitems(void);
void parsespawns(void);
void parsepets(void);
void dumpspawn(const char *, struct spawnStruct *);

void stringprint(int, char *, char *);
void signednumprint(int, char *, long int);
void unsignednumprint(int, char *, unsigned long int);


#endif
