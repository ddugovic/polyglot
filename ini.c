// includes

#include <stdio.h>
#include "option.h"
#include "ini.h"
#include "string.h"
#include "util.h"
#include "errno.h"

// types

typedef enum {
    START               =0,
    SECTION_NAME        =1,
    NAME                =2,
    NAME_SPACE          =3,
    START_VALUE         =4,
    VALUE               =5,
    VALUE_SPACE         =6,
    QUOTE_SPACE         =7,
    FINISHED            =8,
} parse_state_t;

// variables

const char *ini_specials=";\\#[]=";

// functions

// ini_line_parse()

line_type_t ini_line_parse(const char *line,
                                  char *section,
                                  char *name,
                                  char *value){
    int i;
    parse_state_t state=START;
    int name_index=0;
    int value_index=0;
    int section_index=0;
    int index=0;
    char c;
    int type=SYNTAX_ERROR;
    int spaces=0;
    bool quoted;
    while(state!=FINISHED){
        c=line[index++];
	quoted=FALSE;
	if(c=='\\'){
	  if(strchr(ini_specials,line[index])){
	    quoted=TRUE;
	    c=line[index++];
	  }
	}
	//	       printf("STATE=%d quoted=%d c=[%c]\n",state,quoted,c);
        switch(state){
            case START:
	      if(!quoted && ((c==';')||(c=='#')||(c=='\r')||
			     (c=='\n')||(c=='\0'))){
                    type=EMPTY_LINE;
                    state=FINISHED;
                }else if(!quoted && c=='['){
                    state=SECTION_NAME;
                }else if(quoted || c!=' '){
                    name[name_index++]=c;
                    state=NAME;
                }
                goto next;
                break;
            case NAME:
                if(!quoted && c=='='){
                    state=START_VALUE;
                }else if(!quoted && c==' '){
                    state=NAME_SPACE;
                    spaces=1;
                }else if(!quoted && ((c==';')||(c=='#')||(c=='\r')
				     ||(c=='\n')||(c=='\0'))){
                    type=SYNTAX_ERROR;
                    state=FINISHED;
                }else{
                    name[name_index++]=c;
                }
                goto next;
                break;    // we don't get here
            case NAME_SPACE:
                if(!quoted && c==' '){
                    spaces++;
                }else if(!quoted && c=='='){
                    state=START_VALUE;
                }else if(!quoted && ((c==';')||(c=='#')||(c=='\r')||
				     (c=='\n')||(c=='\0'))){
                    type=SYNTAX_ERROR;
                    state=FINISHED;
                }else{
                    for(i=0;i<spaces;i++){
                        name[name_index++]=' ';
                    }
                    spaces=0;
                    name[name_index++]=c;
                    state=NAME;
                }
                goto next;
                break;    // we don't get here
            case START_VALUE:
	        if(!quoted && ((c==';')||(c=='#')||(c=='\r')||
			     (c=='\n')||(c=='\0'))){
                    type=EMPTY_VALUE;
                    state=FINISHED;
                }else if(quoted || c!=' '){
                    value[value_index++]=c;
                    state=VALUE;
                }
                goto next;
                break;    // we don't get here
            case VALUE:
                if(!quoted && c==' '){
                    state=VALUE_SPACE;
                    spaces=1;
                }else if(!quoted && ((c=='\r' || c=='\n' || c==';' || 
				      c=='#'||(c=='\0')))){
                    type=NAME_VALUE;
                    state=FINISHED;
                }else{
                    value[value_index++]=c;
                }
                goto next;
                break;    // we don't get here
            case VALUE_SPACE:
                if(!quoted && c==' '){
                    spaces++;
                }else if(!quoted && ((c=='\r' || c=='\n' || c==';' || 
				      c=='#'||(c=='\0')))){
                    type=NAME_VALUE;
                    state=FINISHED;
                }else{
                    for(i=0;i<spaces;i++){
                        value[value_index++]=' ';
                    }
                    spaces=0;
		    value[value_index++]=c;
		    state=VALUE;
                }
                goto next;
                break;    // we don't get here
            case SECTION_NAME:
                if(!quoted && c==']'){
                    type=SECTION;
                    state=FINISHED;
                }else{
                    section[section_index++]=c;
                }
                goto next;
                break;    // we don't get here
            default:
                break;
        }
      next:        if(!c) break;
    }
    section[section_index]='\0';
    name[name_index]='\0';
    value[value_index]='\0';
    return type;
}

// ini_init()

void ini_init(ini_t *ini){
    memset(ini,0,sizeof(ini_t));
}

// ini_clear()

void ini_clear(ini_t *ini){
    int i;
    ini_entry_t * entry;
    for(i=0; i< ini->index; i++){
        entry=ini->entries+i;
        if(entry->name!=NULL){
            my_string_clear(&entry->name);
        }
        if(entry->value!=NULL){
            my_string_clear(&entry->value);
        }
        if(entry->comment!=NULL){
            my_string_clear(&entry->comment);
        }
    }
    ini->index=0;
}

// ini_copy()

void ini_copy(ini_t *dst, ini_t *src){
  int i;
  dst->index=src->index;
  dst->iter=src->iter;
  for(i=0;i<src->index;i++){
    my_string_set(&dst->entries[i].section,src->entries[i].section);
    my_string_set(&dst->entries[i].name,src->entries[i].name);
    my_string_set(&dst->entries[i].value,src->entries[i].value);
  }
}

// ini_find()

ini_entry_t *ini_find(ini_t *ini, const char *section, const char* name){
    int i;
    ini_entry_t * entry;
    for(i=0; i< ini->index; i++){
        entry=ini->entries+i;
        if(my_string_case_equal(entry->name,name) &&
           my_string_case_equal(entry->section,section)){
            return entry;
        }
    }
    return NULL;
}

// ini_insert()

void ini_insert(ini_t *ini, ini_entry_t *entry){
    ini_entry_t * ini_entry;
    ini_entry=ini_find(ini,entry->section,entry->name);
    if(ini_entry!=NULL){
        my_string_set(&ini_entry->value,entry->value);
    }else{
        if(ini->index>=IniEntriesNb){
            my_fatal("ini_insert(): too many options\n");
        }
        ini_entry=ini->entries+(ini->index++);
        my_string_set(&ini_entry->value,entry->value);
        my_string_set(&ini_entry->name,entry->name);
        my_string_set(&ini_entry->section,entry->section);
    }
}

// ini_insert_ex()

void ini_insert_ex(ini_t *ini,
                   const char *section,
                   const char *name,
                   const char *value){
    ini_entry_t entry[1];
    memset(entry,0,sizeof(ini_entry_t));
    my_string_set(&entry->section,section);
    my_string_set(&entry->name,name);
    my_string_set(&entry->value,value);
    ini_insert(ini,entry);
    my_string_clear(&entry->section);
    my_string_clear(&entry->name);
    my_string_clear(&entry->value);
}

// ini_parse()

int ini_parse(ini_t *ini, const char *filename){
    char name[StringSize];
    char value[StringSize];
    char section[StringSize];
    char line[StringSize];
    ini_entry_t entry[1];
    line_type_t result;
    const char *current_section=NULL;
    FILE *f;
    int line_nr=0;
    my_string_set(&current_section,"Main");
    memset(entry,0,sizeof(ini_entry_t));
    f=fopen(filename,"r");
    if(!f) {
            //    my_fatal("ini_parse(): Can't open file \"%s\": %s\n",
            //     filename,
            //     strerror(errno));
            // For now fail silently
        return -1;
    }
    while(TRUE){
        if(!fgets(line,StringSize,f)){
            break;
        }
        line_nr++;
        result=ini_line_parse(line,section,name,value);
        if(result==SECTION){
            my_string_set(&current_section,section);
        }else if(result==NAME_VALUE){
            ini_insert_ex(ini,current_section,name,value);
        }else if(result==SYNTAX_ERROR){
            my_fatal("ini_parse(): Syntax error in \"%s\": line %d\n",
                     filename,
                     line_nr);
            
        }else {  // empty line
        }
        
    }
    fclose(f);
    return 0;

}

// ini_disp()

void ini_disp(ini_t *ini){
    int i;
    for(i=0;i<ini->index;i++){
        my_log("POLYGLOT [%s] %s=\"%s\"\n",
               (ini->entries)[i].section,
               (ini->entries)[i].name,
               (ini->entries)[i].value);
    }
}

// ini_start_iter()

void ini_start_iter(ini_t *ini){
    ini->iter=0;
}

// ini_next()

ini_entry_t * ini_next(ini_t *ini){
    ASSERT(ini->iter<=ini->index);
    if(ini->iter==ini->index){
        return NULL;
    }
    return &ini->entries[ini->iter++];
}


