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
#include <errno.h>

const char * pgheader_version="1.0";
const char * pgheader_magic="@PG@";

const char * pgheader_known_variants[]={
    "normal",
    "fischerandom",
    "bughouse",
    "crazyhouse",
    "losers",
    "antichess",
    "giveaway",
    "horde",
    "twokings",
    "kriegspiel",
    "atomic",
    "3check",
    "kingofthehill",
    "xiangqi",
    "shogi",
    "capablanca",
    "gothic",
    "falcon",
    "shatranj",
    "courier",
    "knightmate",
    "berolina",
    "janus",
    "cylinder",
    "super",
    "great",
    "unknown",
    NULL
};

static char * errmsg[]={
    "No error.",
    "OS error.",
    "Badly formatted input file.",
    "No header.",
    "Input and output file are the same.",
    "Bad parameter.",
    "Bad header.",
    "Self test failure."
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


static int is_version(char *version){
    int i=0;
    char c;
    int state;
    state=0;
    while(state!=50 && state<=99){
	c=version[i++];
	if(isdigit(c) && c!='0'){
	    c='1';
	}
	switch(state){
	case 0:
	    switch(c){
	    case '0':
		state=1;
		continue;
	    case '1':
		state=5;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	case 1:
	    switch(c){
	    case '.':
		state=2;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	case 5:
	    switch(c){
	    case '0':
	    case '1':
		state=5;
		continue;
	    case '.':
		state=2;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	case 2:
	    switch(c){
	    case '0':
		state=3;
		continue;
	    case '1':
		state=6;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	case 6:
	    switch(c){
	    case '0':
	    case '1':
		state=6;
		continue;
	    case '\0':
		state=50;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	case 3:
	    switch(c){
	    case '\0':
		state=50;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	default:
	    fprintf(stderr,"is_version(): Illegal state=%d\n",state);
	    return 0;
	}
    }
    if(state==50){
	return 1;
    }else{
	return 0;
    }
}


static int test_version(void){
    char **test;
    int i,ret;
    char *input[]= {"1,0","1.0","2.0","0.0","00.0","10.0","10.01","10.10",NULL};
    int result[]=  {0,    1,    1,    1,    0,     1,     0,      1      };
    test=input;
    i=0;
    ret=1;
    while((*test)!=NULL){
	if(is_version(*test)!=result[i]){
	    fprintf(stderr,"test_version(): test %d failed with test=\"%s\"\n",i,*test);
	    ret=0;
	}
	test++;
	i++;
    }
    return ret;
}

static int is_number(char *version){
    int i=0;
    char c;
    int state;
    state=0;
    while(state!=50 && state<=99){
	c=version[i++];
	if(isdigit(c) && c!='0'){
	    c='1';
	}
	switch(state){
	case 0:
	    switch(c){
	    case '0':
		state=1;
		continue;
	    case '1':
		state=5;
		continue;
	    default:
		state=100+state;
	    continue;
	    }
	case 1:
	    switch(c){
	    case '\0':
		state=50;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	case 5:
	    switch(c){
	    case '0':
	    case '1':
		state=5;
		continue;
	    case '\0':
		state=50;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	default:
	    fprintf(stderr,"is_number(): Illegal state=%d\n",state);
	    return 0;
	}
    }    
    if(state==50){
	return 1;
    }else{
	return 0;
    }
}

static int test_number(void){
    char **test;
    int i,ret;
    char *input[]= {"0","00","-2","02","0.0"," 0", NULL};
    int result[]=  {1,   0,  0,   0,    0,     0};
    test=input;
    i=0;
    ret=1;
    while((*test)!=NULL){
	if(is_number(*test)!=result[i]){
	    fprintf(stderr,"test_number(): test %d failed with test=\"%s\"\n",i,*test);
	    ret=0;
	}
	test++;
	i++;
    }
    return ret;
}

static int is_variant(char *variant){
    int i=0;
    char c;
    int state;
    state=0;
    while(state!=50 && state<=99){
	c=variant[i++];
	if(isupper(c)){
	    c='A';
	}else if(isprint(c) && c!=' '){
	    c='@';
	}
	switch(state){
	case 0:
	    switch(c){
	    case '@':
		state=0;
		continue;
	    case '\0':
		state=50;
		continue;
	    default:
		state=100+state;
		continue;
	    }
	default:
	    fprintf(stderr,"is_variant(): Illegal state=%d\n",state);
	    return 0;
	}
    }
    if(state==50){
	return 1;
    }else{
	return 0;
    }
}

int test_variant(void){
    char **test;
    int i,ret;
    char *input[]= {"normal",
		    " normal",
		    "normal@",
		    "nor mal",
		    "Normal",
		    NULL};
    int result[]=  {1,    0,    1,    0,    0};
    test=input;
    i=0;
    ret=1;
    while((*test)!=NULL){
	if(is_variant(*test)!=result[i]){
	    fprintf(stderr,"test_variant(): test %d failed with test=\"%s\"\n",i,*test);
	    ret=0;
	}
	test++;
	i++;
    }
    return ret;
}


variant_t pgheader_known_variant(const char *variant){
    variant_t i;
    for(i=NORMAL; i<UNKNOWN; i++){
        if(!strcasecmp(pgheader_known_variants[i],variant)){
            return i;
        }
    }
    return UNKNOWN;
}

static int is_predef(char *predef){
    int i=0;
    char c;
    int state;
    state=0;
    while(state!=50 && state<=99){
	c=predef[i++];
	switch(state){
	case 0:
	    switch(c){
	    case ' ':
		state=100+state;
		continue;
	    case '\0':
		state=50;
		continue;
	    default:
		state=1;
		continue;
	    }
	case 1:
	    switch(c){
	    case '\0':
		state=50;
		continue;
	    case ' ':
		state=2;
		continue;
	    default:
		state=1;
		continue;
	    }
	case 2:
	    switch(c){
	    case ' ':
		state=2;
		continue;
	    case '\0':
		state=100+state;
		continue;
	    default:
		state=1;
		continue;
	    }
	default:
	    fprintf(stderr,"is_predef(): Illegal state=%d\n",state);
	    return 0;
	}
    }
    if(state==50){
	return 1;
    }else{
	return 0;
    }
}

int test_predef(void){
    char **test;
    int i,ret;
    char *input[]= {"nobody@nowhere.com",
		    " nobody@nowhere.com",
		    "nobody@nowhere.com ",
		    "nobody nowhere.com",
		    " nobody nowhere.com",
		    "nobody nowhere.com ",
		    NULL};
    int result[]=  {1,    0,    0,    1,    0,     0};
    test=input;
    i=0;
    ret=1;
    while((*test)!=NULL){
	if(is_predef(*test)!=result[i]){
	    fprintf(stderr,"test_predef(): test %d failed with test=\"%s\"\n",i,*test);
	    ret=0;
	}
	test++;
	i++;
    }
    return ret;
}




static int is_magic(char *magic){
    if(!strcmp(magic,pgheader_magic)){
	return 1;
    }else{
	return 0;
    }
}


int pgheader_test(){
    int ret;
    ret=1;
    ret&=test_version();
    ret&=test_variant();
    ret&=test_predef();
    ret&=test_number();
    if(ret){
	return PGHEADER_NO_ERROR;
    }else{
	return PGHEADER_TEST_FAILURE;
    }
}

int pgheader_parse(const char *header, char **variants, char **comment){
    int state;
    int nbvariants=-1;
    int nbpredefs=-1;
    char *token;
    char *header_dup;
    header_dup=strdup(header);
    token=strtok(header_dup,"\x0a");
    *comment=malloc(strlen(header)+1);
    *variants=malloc(strlen(header)+1);
    (*variants)[0]='\0';
    (*comment)[0]='\0';

    state=0;
    while(token && state<=99){
	//	printf("state=%d token=\"%10s\" variants=\"%10s\" comment=\"%10s\" nbvariants=%d nbpredefs=%d\n",state,token,*variants,*comment,nbvariants,nbpredefs);
	switch(state){
	case 0:
	    if(is_magic(token)){
		state=1;
	    }else{
		state=100+state;
	    }
	    break;
	case 1:
	    if(is_version(token)){
		state=2;
	    }else{
		state=100+state;
	    }
	    break;
	case 2:
	    if(is_number(token)){
		nbpredefs=atoi(token);
		if(nbpredefs==0){
		    state=100+state;
		}else{
		    state=3;
		}
	    }else{
		state=100+state;
	    }
	    break;
	case 3:
	    if(is_number(token)){
		nbvariants=atoi(token);
		if(nbpredefs<=nbvariants){
		    state=100+state;
		}else{
		    nbpredefs--;
		    if(nbpredefs==0){
			state=50;
		    }else if(nbvariants==0){
			state=5;
		    }else{
			state=4;
		    }
		}
	    }else{
		state=100+state;
	    }
	    break;
	case 4:
	    if(is_variant(token)){
		if((*variants)[0]!=0){
		    strcat(*variants,"\x0a");
		}
		strcat(*variants,token);
		nbvariants--;
		nbpredefs--;
		if(nbvariants==0){
		    if(nbpredefs==0){
			state=50;
		    }else{
			state=5;
		    }
		}else{
		    state=4;
		}
	    }else{
		state=100+state;
	    }
	    break;
	case 5:
	    if(is_predef(token)){
		nbpredefs--;
		if(nbpredefs==0){
		    state=50;
		}else{
		    state=5;
		}
	    }else{
		state=100+state;
	    }
	    break;
	case 50:
	    if((*comment)[0]!=0){
		strcat(*comment,"\x0a");
	    }
	    strcat(*comment,token);
	    state=50;
	    break;
	}
	token=strtok(NULL,"\x0a");
    }
    free(header_dup);
    if(state==50){
	return PGHEADER_NO_ERROR;
    }else{
	free(*variants);
	free(*comment);
	*variants=NULL;
	*comment=NULL;
	return PGHEADER_BAD_HEADER;
    }

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



const char *pgheader_strerror(int pgerror){
    if(pgerror==PGHEADER_OS_ERROR){
	return strerror(errno);
    }else{
	return errmsg[pgerror];
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


int pgheader_create(char **header, const char *variants, const char *comment){
    int i;
    unsigned int nbvariants;
    unsigned int nbheader;
    char c;
    char *variants__;
    char *token;


    /* Step 1: Validate and count variants */

    nbvariants=0;
    variants__=strdup(variants);
    token=strtok(variants__,"\x0a");
    while(token){
	if(!is_variant(token)){
	    free(variants__);
	    return PGHEADER_BAD_PARAMETER;
	}
	nbvariants++;
	token=strtok(NULL,"0x0a");
    }
    free(variants__);
     

    /* Quick hack: at most 998 variants */
    if(nbvariants>998){
	return PGHEADER_BAD_PARAMETER;
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
    unsigned int size, ret;
    int i;
    
    /* read bytes in input file */

    ret=pgheader_read_raw(header,infile,&size);
    if(ret){
	return ret;
    }
    if(size==0){
	free(*header);
	*header=NULL;
	return PGHEADER_NO_HEADER;
    }

    for(i=0;i<size;i++){
	if((*header)[i]=='\0'){
	    return PGHEADER_NO_ERROR;
	}
    }
    free(*header);
    (*header)=NULL;
    return PGHEADER_BAD_HEADER;
}

int pgheader_read_raw(char **raw_header, const char *infile, unsigned int *size){
    int fin;
    char buf[16];
    unsigned int nbheader;
    /* initial malloc */

    nbheader=2048;
    *raw_header=malloc(nbheader);
    if(!(*raw_header)){
	return PGHEADER_OS_ERROR;
    }
    (*size)=0;

    /* open input file */

    fin=open(infile,O_RDONLY|O_BINARY);
    if(fin==-1){
	free(*raw_header);
	*raw_header=NULL;
	return PGHEADER_OS_ERROR;
    }


    /* read bytes in input file */

    *size=0;
    while(1){
	int j,r;
	int zero;
	r=read(fin,buf,16);
	if(r==0){
	    close(fin);
	    return PGHEADER_NO_ERROR;
	}
	if(r<16){
	    free(*raw_header);
	    *raw_header=NULL;
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
	    close(fin);
	    return PGHEADER_NO_ERROR;
	}
	/* enlarge buffer if necessary */
	if((*size)>=nbheader){
	    nbheader*=2;
	    *raw_header=realloc(*raw_header,nbheader);
	}
	for(j=8;j<16;j++){
	    (*raw_header)[(*size)+j-8]=buf[j];
	}
	(*size)+=8;

    }
    /* We never get here! */
    fprintf(stderr,"pgheader_read_raw(): internal error\n");
    close(fin);
    return PGHEADER_BAD_FORMAT;


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
