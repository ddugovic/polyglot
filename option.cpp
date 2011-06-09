
// option.cpp

// includes

#include <cstdlib>
#include <cstring>

#include "option.h"
#include "util.h"

// constants

static const bool UseDebug = false;


// variables

option_t Option[] = {

    { "OptionFile",       "string","0","0",     "polyglot.ini", NULL,0,{},  PG}, 

   // options

    { "EngineName",       "string","0","0",     "<empty>"   , NULL,0,{},  PG}, 
    { "EngineDir",        "string","0","0",     "."         , NULL,0,{},  PG}, 
    { "EngineCommand",    "string","0","0",     "<empty>"   , NULL,0,{},  PG}, 

    { "Log",              "check","0","0",     "false"      , NULL,0,{},  PG|XBOARD|UCI}, 
    { "LogFile",          "string","0","0",     "polyglot.log", NULL,0,{},  PG|XBOARD|UCI}, 

    { "UCI",              "string","0","0",     "false"     , NULL,0,{},  PG}, 

    { "UseNice",          "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD|UCI}, 
    { "NiceValue",        "spin", "0","20",     "5"         , NULL,0,{},  PG|XBOARD|UCI}, 

    { "Chess960",         "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD}, 

    { "Resign",           "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD}, 
    { "ResignMoves",      "spin","0","10000",    "3"        , NULL,0,{},  PG|XBOARD}, 
    { "ResignScore",      "spin","0","10000",   "600"       , NULL,0,{},  PG|XBOARD}, 

    { "MateScore",        "spin","0","1000000", "10000"     , NULL,0,{},  PG|XBOARD}, 

    { "Book",             "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD|UCI}, 
    { "BookFile",         "string","0","0",     "book.bin"  , NULL,0,{},  PG|XBOARD|UCI}, 

    { "BookRandom",       "check","0","0",      "true"      , NULL,0,{},  PG|XBOARD|UCI}, 
    { "BookLearn",        "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD}, 

    { "KibitzMove",       "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD}, 
    { "KibitzPV",         "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD}, 

    { "KibitzCommand",    "string","0","0",     "tellall"   , NULL,0,{},  PG|XBOARD}, 
    { "KibitzDelay",      "check","0","10000",  "5"         , NULL,0,{},  PG|XBOARD}, 

    { "ShowPonder",       "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD}, 

   // work-arounds

    { "UCIVersion",       "spin","1","2",       "2"         , NULL,0,{},  PG|XBOARD}, 
    { "CanPonder",        "check","1","2",      "false"     , NULL,0,{},  PG|XBOARD}, 
    { "SyncStop",         "check","1","2",      "false"     , NULL,0,{},  PG|XBOARD}, 
    { "Affinity",	      "spin","-1","32",     "-1"        , NULL,0,{},  PG}, 
    { "RepeatPV",	      "check","0","0",      "false"     , NULL,0,{},  PG|XBOARD},
    { "PromoteWorkAround","check","0","0",      "false"     , NULL,0,{},  PG|XBOARD}, 

    { NULL,               NULL,"0","0",         NULL        , NULL,0,{},  0},
};

// prototypes

static option_t * option_find (const char var[]);

// functions

// option_init()

void option_init() {

    option_t *p=Option;
    const char * name;

    while(name=(p++)->name){
        option_set(name,option_get_default(name));
    }
}


// option_set()

bool option_set(const char name[], const char value[]) {

   option_t * opt;
   ASSERT(name!=NULL);
   ASSERT(value!=NULL);

   opt = option_find(name);
   if (opt == NULL) return false;

   my_string_set(&opt->value,value);

   if (UseDebug) my_log("POLYGLOT OPTION SET \"%s\" -> \"%s\"\n",opt->name,opt->value);

   return true;
}
// option_set()

bool option_set_default(const char name[], const char value[]) {

   option_t * opt;
   ASSERT(name!=NULL);
   ASSERT(value!=NULL);

   opt = option_find(name);
   if (opt == NULL) return false;

   opt->default_=my_strdup(value);

   if (UseDebug) my_log("POLYGLOT OPTION DEFAULT SET \"%s\" -> \"%s\"\n",opt->name,opt->default_);

   return true;
}

// option_get()

const char * option_get(const char name[]) {

   option_t * opt;

   ASSERT(name!=NULL);

   opt = option_find(name);
   if (opt == NULL) my_fatal("option_get(): unknown option \"%s\"\n",name);

   if (UseDebug) my_log("POLYGLOT OPTION GET \"%s\" -> \"%s\"\n",opt->name,opt->value);

   return opt->value;
}

// option_get_default()

const char * option_get_default(const char name[]) {

   option_t * opt;

   ASSERT(name!=NULL);

   opt = option_find(name);
   if (opt == NULL) my_fatal("option_get(): unknown option \"%s\"\n",name);

   if (UseDebug) my_log("POLYGLOT OPTION GET \"%s\" -> \"%s\"\n",opt->name,opt->value);

   return opt->default_;
}

// option_get_bool()

bool option_get_bool(const char name[]) {

   const char * value;

   value = option_get(name);

   if (false) {
   } else if (my_string_case_equal(value,"true") || my_string_case_equal(value,"yes") || my_string_equal(value,"1")) {
      return true;
   } else if (my_string_case_equal(value,"false") || my_string_case_equal(value,"no") || my_string_equal(value,"0")) {
      return false;
   }

   ASSERT(false);

   return false;
}

// option_get_double()

double option_get_double(const char name[]) {

   const char * value;

   value = option_get(name);

   return atof(value);
}

// option_get_int()

int option_get_int(const char name[]) {

   const char * value;

   value = option_get(name);

   return atoi(value);
}

// option_get_string()

const char * option_get_string(const char name[]) {

   const char * value;

   value = option_get(name);

   return value;
}

// option_find()

static option_t * option_find(const char name[]) {

   option_t * opt;

   ASSERT(name!=NULL);


   for (opt = &Option[0]; opt->name != NULL; opt++) {
      if (my_string_case_equal(opt->name,name)) return opt;
   }

   return NULL;
}

// end of option.cpp

