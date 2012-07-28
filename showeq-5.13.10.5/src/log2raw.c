/* log2raw.c
   This converts a file created using logData() (in packet.cpp) to a raw
   stream of bytes.  Each separate packet logged in the input file (stdin)
   will go to a separate output file.

   The output file "base" name is specified on the command line

   i.e. log2raw output_base < inputfile.log

   Created by cpphack
   Dec 2 2000

   Modifications and improvements are encouraged.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MAX_LINE_LEN 200
#define INITIAL_BUFFER_SIZE 32768

/* inputfile: pointer to the FILE to read
   bytes: gets set to the number of bytes read
   rawdata: will contain the data read from the file
*/
int ReadLog(FILE* inputfile, size_t* bytes, char* rawdata) {
  char line[MAX_LINE_LEN + 1];
  char* hexdata;

  int digitData = 0;
  int currentLinePos = 0;
  int pipePos = 0;
  int i;
  long int longint = 0;

  size_t currentBufferSize = INITIAL_BUFFER_SIZE;
  size_t currentBufferUsed = 0;

  *bytes = 0;

  while( (fgets(line,MAX_LINE_LEN, inputfile) != NULL) && (line[0] != '\n') ) {
    printf("input: %s", line);
    currentLinePos = 0;
    pipePos = 0;

    /* Check our buffer utilization and enlarge if needed */
    if (currentBufferUsed > ( currentBufferSize - MAX_LINE_LEN)) {
      /* If the next line would overflow our buffer, double the current buffer
	 size. */
      rawdata = (char *) realloc(rawdata,(currentBufferSize*2));
      currentBufferSize = currentBufferSize*2;
    }

    /* Remove spaces by replacing with nulls.  */
    while (line[currentLinePos] != '\n') {
      if (line[currentLinePos] == ' ') {
	line[currentLinePos] = 0;
      }

      /* Locate first pipe ('|') */
      if (line[currentLinePos] == '|' && pipePos == 0) {
	pipePos = currentLinePos;
      }

      currentLinePos++;
    } /* Done parsing this line */

    /* Get the first digits */
    digitData = atoi(line);

    for (i = 0; i < 16; i++) {
      /* Set this to the location of the next set of digits */
      hexdata = &line[pipePos] + 2 + i*3;

      if (hexdata[0] == 0)
	break;
    
      rawdata[digitData + i] = strtol(hexdata, &hexdata, 16);
      (*bytes)++;
      /* Keep track of how much of the malloc() area we've used */
      currentBufferUsed += sizeof(longint);
    }
  } /* Get next line */

  if (*bytes)
    return 1;
  
  return 0;
}

int main(int argc, char **argv) {
  size_t bytes; /* Set by the ReadLog subroutine */
  char* data = 0; /* Allocated and set by the ReadLog subroutine */
  char* outputfilename;
  const char* optstring = "hv";
  int blockno = 0;
  FILE* outputfile;

  while(getopt(argc, argv, optstring) >0) {
  }

  outputfilename = (char *) malloc(2048);
  data = (char *) malloc(INITIAL_BUFFER_SIZE);

  while( ReadLog(stdin, &bytes, data) ) {
    printf("Block[%.4d]: %x %x\n", blockno, data[0], data[1]);
    /* Write out the buffer */
    sprintf(outputfilename,"%s_%.4d.raw", argv[1], blockno);
    outputfile = fopen(outputfilename,"w");
    fwrite(data,1,bytes,outputfile);
    blockno++;
  }
  return 0;
}
