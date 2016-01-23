
// option.c

// includes

#include <stdlib.h>
#include <string.h>

#include "option.h"
#include "util.h"


// defines

#define NNB { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL }

// constants

static const bool UseDebug = FALSE;

// variables


option_list_t Option[1];

option_t DefaultOptions[] = {
   // options

    { "SettingsFile",     "file","0","0",       "polyglot.ini", NULL,0,NNB,  PG|XBOARD|XBSEL},

    { "SettingsDir",      "path","0","0",       "<empty>"   , NULL,0,NNB,  PG},

    { "OnlyWbOptions",    "check","0","0",      "true"      , NULL,0,NNB,  PG|XBOARD},

    { "EngineName",       "string","0","0",     "<empty>"   , NULL,0,NNB,  PG},
    { "EngineDir",        "path","0","0",       "."         , NULL,0,NNB,  PG},
    { "EngineCommand",    "string","0","0",     "<empty>"   , NULL,0,NNB,  PG},

    { "Log",              "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD|XBSEL|UCI},
    { "LogFile",          "file","0","0",       "polyglot.log", NULL,0,NNB,  PG|XBOARD|XBSEL|UCI},

    { "UCI",              "check","0","0",      "false"     , NULL,0,NNB,  PG},

    { "UseNice",          "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD|UCI},
    { "NiceValue",        "spin", "0","20",     "5"         , NULL,0,NNB,  PG|XBOARD|UCI},

    { "Resign",           "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD|XBSEL},
    { "ResignMoves",      "spin","0","10000",    "3"        , NULL,0,NNB,  PG|XBOARD|XBSEL},
    { "QueenNeverResigns","check","0","0",      "true"      , NULL,0,NNB,  PG|XBOARD|XBSEL},
    { "ResignScore",      "spin","0","10000",   "600"       , NULL,0,NNB,  PG|XBOARD|XBSEL},
    { "HandleDraws",      "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD|XBSEL},
    { "ContemptScore",    "spin","0","10000",   "30"        , NULL,0,NNB,  PG|XBOARD|XBSEL},

    { "MateScore",        "spin","0","100000",  "10000"     , NULL,0,NNB,  PG|XBOARD},

    { "Book",             "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD|XBSEL|UCI},
    { "BookFile",         "file","0","0",       "book.bin"  , NULL,0,NNB,  PG|XBOARD|XBSEL|UCI},

    { "BookRandom",       "check","0","0",      "true"      , NULL,0,NNB,  PG|XBOARD|XBSEL|UCI},
    { "BookDepth",        "spin","0","256",     "256"       , NULL,0,NNB,  PG|XBOARD|XBSEL|UCI},
    { "BookThreshold",    "spin","0","1000",    "5"         , NULL,0,NNB,  PG|XBOARD|UCI},
    { "BookLearn",        "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD},
    { "ShowTbHits",       "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD|XBSEL},

    { "KibitzMove",       "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD},
    { "KibitzPV",         "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD},

    { "KibitzCommand",    "string","0","0",     "tellall"   , NULL,0,NNB,  PG|XBOARD},
    { "KibitzDelay",      "spin","0","1000",    "5"         , NULL,0,NNB,  PG|XBOARD},
    { "KibitzInterval",   "spin","0","1000",    "0"         , NULL,0,NNB,  PG|XBOARD},

    { "ShowPonder",       "check","0","0",      "true"      , NULL,0,NNB,  PG|XBOARD},
    { "ScoreWhite",       "check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD},

    { "STFudge",          "spin","0","1000",      "20"      , NULL,0,NNB,  PG|XBOARD|XBSEL},

   // work-arounds

    { "UCIVersion",       "spin","1","2",       "2"         , NULL,0,NNB,  PG|XBOARD},
    { "CanPonder",        "check","1","2",      "false"     , NULL,0,NNB,  PG|XBOARD},
    { "SyncStop",         "check","1","2",      "false"     , NULL,0,NNB,  PG|XBOARD|XBSEL},
    { "Affinity",         "spin","-1","32",     "-1"        , NULL,0,NNB,  PG},
    { "RepeatPV",         "check","0","0",      "true"      , NULL,0,NNB,  PG|XBOARD},
    { "PromoteWorkAround","check","0","0",      "false"     , NULL,0,NNB,  PG|XBOARD},

   // internal

    { "3Check",           "check","0","0",      "false"     , NULL,0,NNB,  PG},
    { "Atomic",           "check","0","0",      "false"     , NULL,0,NNB,  PG},
    { "Chess960",         "check","0","0",      "false"     , NULL,0,NNB,  PG},
    { "Horde",            "check","0","0",      "false"     , NULL,0,NNB,  PG},
    { "KingOfTheHill",    "check","0","0",      "false"     , NULL,0,NNB,  PG},
    { "RacingKings",      "check","0","0",      "false"     , NULL,0,NNB,  PG},
    { "Standard",         "check","0","0",      "true"      , NULL,0,NNB,  PG},

   // These options flag various hacks in the source to work around
   // WB quirks. They will eventually all be set to false. Probably
   // in 4.5.0

    { "WbWorkArounds",    "check","0","0",      "true"      , NULL,0,NNB,  PG},
    { "WbWorkArounds2",   "check","0","0",      "false"     , NULL,0,NNB,  PG},
    { "WbWorkArounds3",   "check","0","0",      "true"      , NULL,0,NNB,  PG},

    // Buttons

    { "Save",             "save","0","0",       "false"     , NULL,0,NNB,  PG|XBOARD|XBSEL},

    // Sentinel

    { NULL,               NULL,"0","0",         NULL        , NULL,0,NNB,  0},

};


// functions

// option_is_ok()

bool option_is_ok(const option_list_t *option) {
    if(option->option_nb<0 || option->option_nb>=OptionNb){
        return FALSE;
    }
    return TRUE;
}

// option_init_pg()

void option_init_pg() {

    int i;
    option_t *p=DefaultOptions;
    char *home_dir;
    char SettingsDir[StringSize];

    option_init(Option);
    while(p){
        if(p->name){
            option_insert(Option,p);
            p++;
        }else{
            break;
        }
    }
    for(i=0;i<Option->option_nb;i++){
        Option->options[i].value=my_strdup(Option->options[i].default_);
    }
#ifndef _WIN32
    home_dir=getenv("HOME");
    if(!home_dir){
        home_dir=".";
    }
    snprintf(SettingsDir,sizeof(SettingsDir),"%s/.polyglot",home_dir);
    SettingsDir[sizeof(SettingsDir)-1]='\0';
#else
    sprintf(SettingsDir,".\\_PG");
#endif
    option_set(Option,"SettingsDir",SettingsDir);
    option_set_default(Option,"SettingsDir",SettingsDir);
}

// option_init()

void option_init(option_list_t *option){
    ASSERT(option!=NULL);
    option->option_nb=0;
    option->iter=0;
    memset(option->options,0,sizeof(option->options));
}

// option_insert()

void option_insert(option_list_t *option, option_t *new_option){
    int i;
    option_t *opt;
    ASSERT(option!=NULL);
    ASSERT(new_option!=NULL);
    ASSERT(new_option->name!=NULL);
    opt=option_find(option,new_option->name);
    if(!opt){
        opt=&option->options[option->option_nb];
        option->option_nb++;
    }
    if(option->option_nb>=OptionNb){
        my_fatal("option_insert(): option list overflow\n");
    }
    if(new_option->name)     my_string_set(&opt->name,     new_option->name);
    if(new_option->value)    my_string_set(&opt->value,    new_option->value);
    if(new_option->min)      my_string_set(&opt->min,      new_option->min);
    if(new_option->max)      my_string_set(&opt->max,      new_option->max);
    if(new_option->default_) my_string_set(&opt->default_, new_option->default_);
    if(new_option->type)     my_string_set(&opt->type,     new_option->type);
    opt->var_nb=new_option->var_nb;
    for(i=0;i<new_option->var_nb;i++){
        my_string_set(&opt->var[i], new_option->var[i]);
    }
    opt->mode=new_option->mode;
}

// option_set()

bool option_set(option_list_t *option,
                const char name[],
                const char value[]) {

   option_t * opt;
   ASSERT(option!=NULL);
   ASSERT(name!=NULL);
   ASSERT(value!=NULL);

   opt = option_find(option,name);
   if (opt == NULL) return FALSE;

   if(my_string_case_equal(opt->type,"check")){
      value=(my_string_equal(value,"1")||
	     my_string_case_equal(value,"true"))?"true":"false";
   }

   my_string_set(&opt->value,value);

   if (UseDebug) my_log("POLYGLOT OPTION SET \"%s\" -> \"%s\"\n",opt->name,opt->value);

   return TRUE;
}

// option_set_default()

bool option_set_default(option_list_t *option,
                           const char name[],
                           const char value[]) {

   option_t * opt;
   ASSERT(name!=NULL);
   ASSERT(value!=NULL);

   opt = option_find(option,name);
   if (opt == NULL) return FALSE;

   if(my_string_case_equal(opt->type,"check")){
      value=(my_string_equal(value,"1")||
	     my_string_case_equal(value,"true"))?"true":"false";
   }

   my_string_set(&opt->default_,value);

   if (UseDebug) my_log("POLYGLOT OPTION DEFAULT SET \"%s\" -> \"%s\"\n",opt->name,opt->default_);

   return TRUE;
}

// option_get()

const char * option_get(option_list_t *option, const char name[]) {

   option_t * opt;

   ASSERT(name!=NULL);

   opt = option_find(option,name);
   if (opt == NULL) my_fatal("option_get(): unknown option \"%s\"\n",name);
   if (UseDebug) my_log("POLYGLOT OPTION GET \"%s\" -> \"%s\"\n",opt->name,opt->value);

   return opt->value;
}

// option_get_default()

const char * option_get_default(option_list_t *option, const char name[]) {

   option_t * opt;

   ASSERT(name!=NULL);

   opt = option_find(option,name);
   if (opt == NULL) my_fatal("option_get(): unknown option \"%s\"\n",name);

   if (UseDebug) my_log("POLYGLOT OPTION GET \"%s\" -> \"%s\"\n",opt->name,opt->value);

   return opt->default_;
}

// option_get_bool()

bool option_get_bool(option_list_t *option, const char name[]) {

   const char * value;

   value = option_get(option,name);

   if (FALSE) {
   } else if (my_string_case_equal(value,"true") || my_string_case_equal(value,"yes") || my_string_equal(value,"1")) {
      return TRUE;
   } else if (my_string_case_equal(value,"false") || my_string_case_equal(value,"no") || my_string_equal(value,"0")) {
      return FALSE;
   }

   ASSERT(FALSE);

   return FALSE;
}


// option_get_double()

double option_get_double(option_list_t *option, const char name[]) {

   const char * value;

   value = option_get(option,name);

   return atof(value);
}

// option_get_int()

int option_get_int(option_list_t *option, const char name[]) {

   const char * value;

   value = option_get(option,name);

   return atoi(value);
}

// option_get_string()

const char * option_get_string(option_list_t *option, const char name[]) {

   const char * value;

   value = option_get(option,name);

   return value;
}

// option_find()

option_t * option_find(option_list_t *option, const char name[]) {

   option_t * opt;
   int i;

   ASSERT(name!=NULL);
   for (i=0; i<option->option_nb; i++){
       opt=option->options+i;
       if (my_string_case_equal(opt->name,name)){
           return opt;
       }
   }

   return NULL;
}

// option_start_iter()

void option_start_iter(option_list_t *option){
    option->iter=0;
}

// option_next()

option_t * option_next(option_list_t *option){
    ASSERT(option->iter<=option->option_nb);
    if(option->iter==option->option_nb){
        return NULL;
    }
    return &option->options[option->iter++];

}

void option_free(option_t *option){
      int i;
      my_string_clear(&option->name);
      my_string_clear(&option->type);
      my_string_clear(&option->min);
      my_string_clear(&option->max);
      my_string_clear(&option->default_);
      my_string_clear(&option->value);
      for(i=0;i<option->var_nb;i++){
         my_string_clear(&option->var[i]);
      }
      option->var_nb=0;
      option->mode=0;
}

// option_clear()

void option_clear(option_list_t *option){
    int i;
    for (i = 0; i < option->option_nb; i++) {
        option_free(option->options+i);
   }
   option->option_nb=0;
}

// option_from_ini()

void option_from_ini(option_list_t *option,
                     ini_t *ini,
                     const char *section){
    ini_entry_t *entry;
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
        option_set(option,entry->name,entry->value);
        option_set_default(option,entry->name,entry->value);
    }
}

// end of option.cpp

