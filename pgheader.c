/*
Copyright (c) 2012, Michel Van den Bergh <michel.vandenbergh@uhasselt.be>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pgheader.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

const char * pgheader_version="1.0";
const char * pgheader_magic="@PG@";

static char * errmsg[]={
    "No error.",
    "OS error.",
    "Badly formatted input file.",
    "No header.",
    "Input and output file are the same.",
    "Bad parameter.",
    "Bad header."
};

#ifndef WIN32
#define O_BINARY 0x0
#endif

#ifdef _MSC_VER
  typedef unsigned __int64 uint64_t;
#else
  typedef unsigned long long int uint64_t;
#endif

static int int_from_file(FILE *f, int l, uint64_t *r){
    int i,c;
    for(i=0;i<l;i++){
        c=fgetc(f);
        if(c==EOF){
            return 1;
        }
        (*r)=((*r)<<8)+c;
    }
    return 0;
}


int pgheader_detect(const char *infile){
    FILE *fin;
    uint64_t r0,r1,r2;
    int i;

    fin=fopen(infile,"rb");
    if(!fin){
	return PGHEADER_OS_ERROR;
    }
    fseek(fin,0L,SEEK_END);
    if(ftell(fin)%16 !=0){
	fclose(fin);
	return PGHEADER_BAD_FORMAT;
    }
    fseek(fin,0L,SEEK_SET);
    r0=0;
    r1=0;
    r2=0;
    for(i=0;i<10;i++){
	if(int_from_file(fin,8,&r1)){
	    break;
	}
	if(int_from_file(fin,8,&r2)){
	    fclose(fin);
	    return PGHEADER_BAD_FORMAT;
	}
	if(r1<r0){
	    fclose(fin);
	    return PGHEADER_BAD_FORMAT;
	}
	r0=r1;
    }
    fclose(fin);
    return PGHEADER_NO_ERROR;
}

void pgheader_error(const char *prompt, int ret){
    switch(ret){
    case PGHEADER_NO_ERROR:
	break;
    case PGHEADER_OS_ERROR:
	perror(prompt);
	break;
    default:
	fprintf(stderr,"%s: %s\n",prompt,errmsg[ret]);
	break;
	
    }
}

int pgheader_create_raw(char **raw_header, const char *header, unsigned int *size){
    unsigned int b,i,j,k;

    b=strlen(header)+1;
    *size=2*(8*(b/8)+(b%8?8:0));
    *raw_header=malloc(*size);
    if(!(*raw_header)){
	return PGHEADER_OS_ERROR;
    }
    j=0;
    for(i=0;i<b;i++){
	if(i%8==0){
	    for(k=0;k<8;k++){
		(*raw_header)[j++]=0;
	    }
	}
	(*raw_header)[j++]=header[i];
    }
    for(k=j;k<(*size);k++){
	(*raw_header)[k]=0;
    }
    return PGHEADER_NO_ERROR;
}

int pgheader_parse(const char *header, char **variants, char **comment){
    int ret,count,j;
    char *header_dup;
    char *token;
    char *variant;
    int nbpredef;
    int cf;
    header_dup=strdup(header);
    *variants=malloc(strlen(header)+1);
    *comment=malloc(strlen(header)+1);
    ret=0;
    (*variants)[0]='\0';
    (*comment)[0]='\0';
    token=strtok(header_dup,"\x0a");
    cf=0;
    if(token){  /* MAGIC */
	token=strtok(NULL,"\x0a");
	if(token){  /* VERSION */
	    token=strtok(NULL,"\x0a");
	    if(token){ /* PREDEF */
		nbpredef=atoi(token);
		/* parse variant fields */
		token=strtok(NULL,"\x0a");
		if(token){ /* NBVARIANTS */
		    count=atoi(token);
		    cf++;
		    /* we allow a zero number of variants */
		    if(count>=0){
			for(j=0;j<count;j++){
			    variant=strtok(NULL,"\x0a");
			    cf++;
			    if((*variants)[0]!=0){
				strcat(*variants,"\x0a");
			    }
			    strcat(*variants,variant);
			}
		    }else{
			ret=1;
		    }
		    
		}else{
		    ret=1;
		}
	    }else{
		ret=1;
	    }
	}else{
	    ret=1;
	}
    }else{
	ret=1;
    }
    /* skip unknown fields */
    if(ret==0 && cf<=nbpredef){
	while((cf++)<nbpredef){
	    token=strtok(NULL,"\x0a");
	    if(!token){
		ret=1;
	    }
	}
    }
    /* parse comment fields */
    if(ret==0){
	token=strtok(NULL,"\x0a");
	while(token){
	    if((*comment)[0]!=0){
		strcat(*comment,"\x0a");
	    }
	    strcat(*comment,token);
	    token=strtok(NULL,"\x0a");
	}
    }else{
	free(*variants);
	free(*comment);
	free(header_dup);
	*variants=NULL;
	*comment=NULL;
	return PGHEADER_BAD_HEADER;
    }

    free(header_dup);
    return PGHEADER_NO_ERROR;

}

int pgheader_create(char **header, const char *variants, const char *comment){
    int i;
    unsigned int nbvariants;
    unsigned int nbheader;
    char c;


    /* Step 0: Validate variants: only lowercase, no spaces */

    for(i=0;i<strlen(variants);i++){
	c=variants[i];
	if(c==' ' || isupper(c)){
	    return PGHEADER_BAD_PARAMETER;
	}
    }

    /* Step 1: the number of variants is one more than the number of linefeeds */

    nbvariants=1;
    for(i=0;i<strlen(variants);i++){
	if(variants[i]==0x0a){
	    nbvariants++;
	}
	/* Quick hack: at most 998 variants */
	if(nbvariants>998){
	    return PGHEADER_BAD_PARAMETER;
	}
    }


    /* Step 2: estimate length */

    nbheader=
	strlen(pgheader_magic)+1
	+strlen(pgheader_version)+1
	+3/*nbpredef*/+1
	+3/*nbvariants*/+1
	+strlen(variants)+1
	+strlen(comment)+1;

    /* Step 3: allocate memory */

    *header=malloc(nbheader);
    if(!(*header)){
	return PGHEADER_OS_ERROR;
    }

    /* Step 4: fill header */

    strcpy(*header,pgheader_magic);
    strcat(*header,"\x0a");
    strcat(*header,pgheader_version); 
    strcat(*header,"\x0a");
    sprintf(*header+strlen(*header),"%d",nbvariants+1); /* predef */
    strcat(*header,"\x0a");
    sprintf(*header+strlen(*header),"%d",nbvariants);
    strcat(*header,"\x0a");
    strcat(*header,variants);
    strcat(*header,"\x0a");
    strcat(*header,comment);

    return PGHEADER_NO_ERROR;

}


int pgheader_read(char **header, const char *infile){
    int fin;
    char buf[16];
    unsigned int nbheader;
    unsigned int read_bytes;

    /* initial malloc */

    nbheader=2048;
    *header=malloc(nbheader);
    if(!(*header)){
	return PGHEADER_OS_ERROR;
    }
    read_bytes=0;

    /* open input file */

    fin=open(infile,O_RDONLY|O_BINARY);
    if(fin==-1){
	*header=NULL;
	return PGHEADER_OS_ERROR;
    }


    /* read bytes in input file */

    while(1){
	int j,r;
	int zero;
	int last;
	/* enlarge buffer if necessary */
	if(read_bytes>=nbheader){
	    nbheader*=2;
	    *header=realloc(*header,nbheader);
	}
	
	r=read(fin,buf,16);
	if(r<16){
	    free(*header);
	    *header=NULL;
	    close(fin);
	    return PGHEADER_BAD_FORMAT;
	}
	zero=1;
	for(j=0;j<8;j++){
	    if(buf[j]!=0){
		zero=0;
	    }
	}
	if(!zero){
	    /* if we encounter a non null record here it means 
	       we have not bailed out earlier */
	    free(*header);
	    *header=NULL;
	    close(fin);
	    return PGHEADER_NO_HEADER;
	}
	last=0;
	for(j=8;j<16;j++){
	    (*header)[read_bytes+j-8]=buf[j];
	    if(buf[j]==0){
		last=1;
	    }

	}
	if(last){
	    break;
	}
	read_bytes+=8;
    }
    close(fin);
    return PGHEADER_NO_ERROR;
}


int pgheader_write(const char *header, const char *infile, const char *outfile){
    int fin,fout,i,ret;
    char c;
    char buf[16];
    char *raw_header;
    unsigned int size;

    /* make sure we are dealing with a polyglot book */

    if((ret=pgheader_detect(infile))){
	return ret;
    }



    /* safety check! */
    if(!strcmp(infile,outfile)){
	return PGHEADER_NAME_COLLISION;
    }

    /* open files */
    fin=open(infile,O_RDONLY|O_BINARY);
    if(fin==-1){
	return PGHEADER_OS_ERROR;
    }
    fout=open(outfile,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,S_IRUSR|S_IWUSR);
    if(fout==-1){
	close(fin);
	return PGHEADER_OS_ERROR;
    }

    /* skip null records in input file */

    while(1){
	int j,r;
	int zero;
	r=read(fin,buf,16);
	if(r<16){
	    close(fin);
	    close(fout);
	    return PGHEADER_BAD_FORMAT;
	}
	zero=1;
	for(j=0;j<8;j++){
	    if(buf[j]!=0){
		zero=0;
	    }
	}
	if(!zero){
	    break;
	}
    }

    /* write header to output file */

    if((ret=pgheader_create_raw(&raw_header,header,&size))){
	return ret;
    }

    for(i=0;i<size;i++){
	c=raw_header[i];
	if(write(fout,&c,1)!=1){
	    close(fin);
	    close(fout);
	    return PGHEADER_OS_ERROR;
	}
    }

    free(raw_header);

    /* copy remaining records from input to output */

    if(write(fout,buf,16)!=16){
	close(fin);
	close(fout);
	return PGHEADER_OS_ERROR;
    }
        while((ret=read(fin,buf,16))==16){
	if(write(fout,buf,ret)!=16){
	    close(fin);
	    close(fout);
	    return PGHEADER_OS_ERROR;
	}
    };
    close(fin);
    close(fout);

    if(0<ret && ret<16){
	return PGHEADER_BAD_FORMAT;
    }else if(ret==-1){
	return PGHEADER_OS_ERROR;
    } 

    return PGHEADER_NO_ERROR;
}


int pgheader_delete(const char *infile, const char *outfile){
    int fin, fout;
    char buf[16];
    int ret;


    /* make sure we are dealing with a polyglot book */

    if((ret=pgheader_detect(infile))){
	return ret;
    }


    /* safety check! */
    if(!strcmp(infile,outfile)){
	return PGHEADER_NAME_COLLISION;
    }


    /* open files */
    fin=open(infile,O_RDONLY|O_BINARY);
    if(fin==-1){
	return PGHEADER_OS_ERROR;
    }
    fout=open(outfile,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,S_IRUSR|S_IWUSR);
    if(fout==-1){
	close(fin);
	return PGHEADER_OS_ERROR;
    }

    /* skip null records in input file */

    while(1){
	int j,r;
	int zero;
	r=read(fin,buf,16);
	if(r<16){
	    close(fin);
	    close(fout);
	    return PGHEADER_BAD_FORMAT;
	}
	zero=1;
	for(j=0;j<8;j++){
	    if(buf[j]!=0){
		zero=0;
	    }
	}
	if(!zero){
	    break;
	}
    }

    /* copy remaining records from input to output */

    if(write(fout,buf,16)!=16){
	close(fin);
	close(fout);
	return PGHEADER_OS_ERROR;
    }
    
    while((ret=read(fin,buf,16))==16){
	if(write(fout,buf,ret)!=16){
	    close(fin);
	    close(fout);
	    return PGHEADER_OS_ERROR;
	}
    };
    close(fin);
    close(fout);

    if(0<ret && ret<16){
	return PGHEADER_BAD_FORMAT;
    }else if(ret==-1){
	return PGHEADER_OS_ERROR;
    } 

    return PGHEADER_NO_ERROR;
}
