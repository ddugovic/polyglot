#ifndef PGHEADER_H
#define PGHEADER_H

#define PGHEADER_NO_ERROR 0
#define PGHEADER_OS_ERROR 1
#define PGHEADER_BAD_FORMAT 2
#define PGHEADER_NO_HEADER 3
#define PGHEADER_NAME_COLLISION 4
#define PGHEADER_BAD_PARAMETER 5
#define PGHEADER_BAD_HEADER 6


const char * pgheader_version;
const char * pgheader_magic;

/* This does some heuristic tests to make it reasonably sure we are dealing
   with a Polyglot book.
*/
int pgheader_detect(const char *infile);

/* "variants" is a linefeed separated list of supported variants.
   "comments" is a free format string which may contain linefeeds.
   "header" must be freed if the function returns without error.
*/
int pgheader_create(char **header, const char *variants, const char *comment);

/* "variants" and "comment" must be freed if the function returns 
   without error.
*/
int pgheader_parse(const char *header, char **variants, char **comment);

/*  "raw_header" is a byte array ready for prepending to a book file.
    It must be freed after use.
 */
int pgheader_create_raw(char **raw_header, const char *header, unsigned int *size);

/* "header" must be freed if the function returns without error.
*/
int pgheader_read(char **header, const char *infile);

int pgheader_write(const char *header, const char *infile, const char *outfile);
int pgheader_delete(const char *infile, const char *outfile);

void pgheader_error(const char *prompt, int ret);

#endif

