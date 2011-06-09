
// main.c

// includes

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attack.h"
#include "board.h"
#include "book.h"
#include "book_make.h"
#include "book_merge.h"
#include "engine.h"
#include "epd.h"
#include "fen.h"
#include "gui.h"
#include "hash.h"
#include "list.h"
#include "main.h"
#include "mainloop.h"
#include "move.h"
#include "move_gen.h"
#include "option.h"
#include "piece.h"
#include "search.h"
#include "square.h"
#include "uci.h"
#include "util.h"
#include "xboard2uci.h"
#include "uci2uci.h"
#include "ini.h"

// constants


static const char * const Version = "1.4.44b";
static const char * const HelpMessage = "\
SYNTAX\n\
* polyglot [configfile] [-noini] [-ec engine] [-ed enginedirectory] [-en enginename] [-log] [-lf logfile] [-hash value] [-bk book] [-pg <name>=<value>]* [-uci <name>=<value>]*\n\
* polyglot make-book [-pgn inputfile] [-bin outputfile] [-max-ply ply] [-min-game games] [-min-score score] [-only-white] [-only-black] [-uniform]\n\
* polyglot merge-book -in1 inputfile1 -in2 inputfile2 [-out outputfile]\n\
* polyglot info-book [-bin inputfile] [-exact]\n\
* polyglot dump-book [-bin inputfile] -color color [-out outputfile]\n\
* polyglot [configfile] epd-test [engineoptions] [-epd inputfile] [-min-depth depth] [-max-depth depth] [-min-time time] [-max-time time] [-depth-delta delta]\n\
* polyglot perft [-fen fen] [-max-depth depth]\
";

static const int SearchDepth = 63;
static const double SearchTime = 3600.0;
static const int StringSize = 4096;

// variables

static bool Init;

// prototypes

static void init_book ();
static void stop_search  ();

// functions

// arg_shift_left()

static void arg_shift_left(char **argv, int index){
    int i;
    for(i=index; argv[i]!=NULL; i++){
        argv[i]=argv[i+1];
    }
}

// write_ini()

static void write_ini(const char *filename,
                      option_list_t *pg_options,
                      option_list_t *uci_options){
    option_t *opt;
    char tmp[StringSize];
    FILE *f;
    f=fopen(filename,"w");
    if(!f){
        my_fatal("write_ini(): Cannot open %s for writing.\n",filename);
    }
    fprintf(f,"; You may edit this file to set options for the\n"
              "; UCI engine whose PolyGlot name is %s.\n"
              "; You may also safely delete this file\n"
              "; to restore the default options.\n",
            option_get_string(Option,"EngineName"));
    fprintf(f,"[PolyGlot]\n");
    option_start_iter(pg_options);
    while((opt=option_next(pg_options))){
        if(!my_string_equal(opt->value,opt->default_)&&
           !IS_BUTTON(opt) &&
           (opt->mode & XBOARD)){
            snprintf(tmp,sizeof(tmp),"%s=%s\n",opt->name,opt->value);
            tmp[sizeof(tmp)-1]='\0';
            fprintf(f,"%s",tmp);
        }
    }
    fprintf(f,"[Engine]\n");
    option_start_iter(uci_options);
    while((opt=option_next(uci_options))){
        if(!my_string_equal(opt->value,opt->default_)&&
           !IS_BUTTON(opt)){
            snprintf(tmp,sizeof(tmp),"%s=%s\n",opt->name,opt->value);
            tmp[sizeof(tmp)-1]='\0';
            fprintf(f,"%s",tmp);
        }
    }
    fclose(f);
}

// write_ini_ex()

static void write_ini_ex(const char *filename,
			 ini_t *ini){
    ini_entry_t *entry;
    char tmp[StringSize];
    FILE *f;
    f=fopen(filename,"w");
    if(!f){
        my_fatal("write_ini_ex(): Cannot open %s for writing.\n",filename);
    }
    fprintf(f,"; You may edit this file to set options for the\n"
              "; UCI engine whose PolyGlot name is %s.\n"
              "; You may also safely delete this file\n"
              "; to restore the default options.\n",
            option_get_string(Option,"EngineName"));
    fprintf(f,"[PolyGlot]\n");
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
      if(my_string_case_equal(entry->section,"polyglot")){
	snprintf(tmp,sizeof(tmp),"%s=%s\n",
		 entry->name,
		 entry->value);
	tmp[sizeof(tmp)-1]='\0';
	fprintf(f,"%s",tmp);
      }
    }
    fprintf(f,"[Engine]\n");
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
      if(my_string_case_equal(entry->section,"engine")){
	snprintf(tmp,sizeof(tmp),"%s=%s\n",
		 entry->name,
		 entry->value);
	tmp[sizeof(tmp)-1]='\0';
	fprintf(f,"%s",tmp);
      }
    }
    fclose(f);
}


// main()

int main(int argc, char * argv[]) {
    ini_t ini[1],ini_save[1];
    ini_entry_t *entry;
    char *arg;
    int arg_index;
    bool NoIni, Persist;
 
    if(!DEBUG){
        printf("PolyGlot %s by Fabien Letouzey.\n",Version);
    }else{
        printf("PolyGlot %s by Fabien Letouzey (debug build).\n",Version);
    }

    if(argc>=2 && ((my_string_case_equal(argv[1],"help")) || (my_string_case_equal(argv[1],"-help")) || (my_string_case_equal(argv[1],"--help")) ||  (my_string_case_equal(argv[1],"-h")) ||  my_string_case_equal(argv[1],"/?"))){
        printf("%s\n",HelpMessage);
        return EXIT_SUCCESS;
    }

   // init

    Init = FALSE;

    util_init();
    option_init_pg();
    
    square_init();
    piece_init();
    attack_init();
    
    hash_init();

    my_random_init();

    ini_init(ini);
    ini_init(ini_save);

        // book utilities
    
    if (argc >= 2 && my_string_equal(argv[1],"make-book")) {
        book_make(argc,argv);
        return EXIT_SUCCESS;
    }
    
    if (argc >= 2 && my_string_equal(argv[1],"merge-book")) {
        book_merge(argc,argv);
        return EXIT_SUCCESS;
    }

    if (argc >= 2 && my_string_equal(argv[1],"dump-book")) {
        book_dump(argc,argv);
        return EXIT_SUCCESS;
    }
    
    if (argc >= 2 && my_string_equal(argv[1],"info-book")) {
        book_info(argc,argv);
        return EXIT_SUCCESS;
    }

        // perft
    
    if (argc >= 2 && my_string_equal(argv[1],"perft")) {
        do_perft(argc,argv);
        return EXIT_SUCCESS;
    }

        // TODO: If logging is enabled on the command line turn it on NOW
        // and do not allow it to be overridden later. 
    
        // What is the config file? This is very hacky right now.

        // Do we want a config file at all?

    arg_index=0;
    NoIni=FALSE;
    while((arg=argv[arg_index++])){
        if(my_string_equal(arg,"-noini")){
            NoIni=TRUE;
            break;
        }
    }
    arg_shift_left(argv,arg_index-1);
    if(NoIni){
        option_set(Option,"OptionFile","<empty>");
    }

        // Ok see if first argument looks like config file
    
    if(argv[1] && !my_string_equal(argv[1],"epd-test") && !(argv[1][0]=='-')){
                // first argument must be  config file
        if(!NoIni){
            option_set(Option,"OptionFile",argv[1]);
        }else{
                // ignore
        }
        arg_shift_left(argv,1);
    }else{
            // Config file is the default.
            // This has already been set above or in "option_init_pg()"
    }
    

        // if we use a config file: load it!
    
    if(!my_string_equal(option_get_string(Option,"OptionFile"),"<empty>")){
        if(ini_parse(ini,option_get_string(Option,"OptionFile"))){
            my_fatal("main(): Can't open file \"%s\": %s\n",
                   option_get_string(Option,"OptionFile"),
                   strerror(errno));
        }
    }
        // remind the reader of what options are in effect

    my_log("POLYGLOG Options from ini file\n");
    ini_disp(ini);

        // extract PG options
    
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
        if(my_string_case_equal(entry->section,"polyglot")){
            option_set(Option,entry->name,entry->value);
            option_set_default(Option,entry->name,entry->value);
        }
    }

        // start logging if required
    
    if (option_get_bool(Option,"Log")) {
        my_log_open(option_get_string(Option,"LogFile"));
    }

        // log welcome stuff
    
    if(!DEBUG){
        my_log("PolyGlot %s by Fabien Letouzey\n",Version);
    }else{
        my_log("PolyGlot %s by Fabien Letouzey (debug build)\n",Version);
    }    
    my_log("POLYGLOT *** START ***\n");
    my_log("POLYGLOT INI file \"%s\"\n",option_get_string(Option,"OptionFile"));

        // open book (presumably this should go else where)
    
    init_book();

        // scavenge command line for options necessary to start the engine
    
    arg_index=1;
    while((arg=argv[arg_index])){
        if(my_string_equal(arg,"-ec") && argv[arg_index+1]){
            option_set(Option,"EngineCommand",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-ed") && argv[arg_index+1]){
            option_set(Option,"EngineDir",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-en") && argv[arg_index+1]){
            option_set(Option,"EngineName",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        arg_index++;
    }

        // start engine
    
    engine_open(Engine);
    if(!engine_active(Engine)){
        my_fatal("Could not start \"%s\"\n",option_get(Option,"EngineCommand"));
    }

        // switch to UCI mode if necessary
    
    if (option_get_bool(Option,"UCI")) {
        my_log("POLYGLOT *** Switching to UCI mode ***\n");
    }

        // initialize uci parsing and send uci command. 
        // Parse options and wait for uciok
    
    uci_open(Uci,Engine);

        // get engine name from engine if not supplied in config file
    
    if (my_string_equal(option_get_string(Option,"EngineName"),"<empty>")) {
        option_set(Option,"EngineName",Uci->name);
    }

        // what is the name of the persist file?

    if(my_string_equal(option_get_string(Option,"PersistFile"),"<empty>")){
        char tmp[StringSize];
        snprintf(tmp,sizeof(tmp),"PG_%s.ini",
                 option_get_string(Option,"EngineName"));
        tmp[sizeof(tmp)-1]='\0';
        option_set(Option,"PersistFile",tmp);
    }

    // Load the persist file

    my_log("POLYGLOT PersistFile=%s\n",option_get_string(Option,"PersistFile"));   
    if(ini_parse(ini_save,option_get_string(Option,"PersistFile"))){
      my_log("POLYGLOT Unable to open PersistFile\n"); 
    }

    // Do we want to use the persist file?

    entry=ini_find(ini_save,"polyglot","Persist");
    if(!entry){
      Persist=option_get_bool(Option,"Persist");
    }else{
      Persist=(my_string_case_equal(entry->value,"false") || 
	       my_string_equal(entry->value,"0"))?FALSE:TRUE;
    }

    // if "Persist" now happens to be false, forget about the
    // persist file    

    if(!Persist){
      my_log("POLYGLOT Ignoring PersistFile");
      ini_clear(ini_save);
    }

    option_set(Option,"Persist",Persist?"true":"false");

    // parse the command line and merge remaining options

    arg_index=1;
    while((arg=argv[arg_index])){
        if(my_string_equal(arg,"-log")){
            ini_insert_ex(ini_save,"PolyGlot","Log","true");
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-lf") && argv[arg_index+1]){
            ini_insert_ex(ini_save,"PolyGlot","LogFile",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-hash") && argv[arg_index+1]){
            ini_insert_ex(ini_save,"Engine","Hash",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-bk") && argv[arg_index+1]){
            ini_insert_ex(ini_save,"PolyGlot","Book","true");
            ini_insert_ex(ini_save,"PolyGlot","BookFile",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if((my_string_equal(arg,"-pg")||my_string_equal(arg,"-uci")) &&
           argv[arg_index]){
            int ret;
            char section[StringSize];
            char name[StringSize];
            char value[StringSize];
            ret=ini_line_parse(argv[arg_index++],section,name,value);
            if(ret==NAME_VALUE){
                if(my_string_equal(arg,"-pg")){
                    ini_insert_ex(ini_save,"PolyGlot",name,value);
                }else{
                    ini_insert_ex(ini_save,"Engine",name,value);
                }
            }
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        arg_index++;
    }

        // remind the reader once again about options

    my_log("POLYGLOG Options from PersistFile and command line\n");
    ini_disp(ini_save);

        // extract PG options; this time do not set the default
        // polyglot_set_option() performs the necessary actions such
        // as opening the log file/opening book etcetera.
    
    ini_start_iter(ini_save);
    while((entry=ini_next(ini_save))){
        if(my_string_case_equal(entry->section,"polyglot")){
            polyglot_set_option(entry->name,entry->value);
        }
    }

        // done initializing
    
    Init = TRUE;
    
        // collect engine options from config file(s) and send to engine
    
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
        if(my_string_case_equal(entry->section,"engine")){
                // also updates value in Uci->option
            uci_send_option(Uci,entry->name,"%s",entry->value);
                // since this comes from the ini file, also update default
            option_set_default(Uci->option,entry->name,entry->value);
                // this is inherited, it probably does not work correctly
           if(my_string_case_equal(entry->name,"MultiPV") &&
              atoi(entry->value)>1){
               Uci->multipv_mode=TRUE;
           }
        }
    }
    ini_start_iter(ini_save);
    while((entry=ini_next(ini_save))){
        if(my_string_case_equal(entry->section,"engine")){
                // also updates value in Uci->option
            uci_send_option(Uci,entry->name,"%s",entry->value);
                // this is inherited, it probably does not work correctly
            if(my_string_case_equal(entry->name,"MultiPV") &&
               atoi(entry->value)>1){
                Uci->multipv_mode=TRUE;
            }
        }
    }
   
    
        // EPD test
    
    if (argv[1] && my_string_equal(argv[1],"epd-test")){
        argc=0;
        while((arg=argv[argc++]));
        epd_test(argc-1,argv);
        return EXIT_SUCCESS;
    }
    
        // Anything that hasn't been parsed yet is a syntax error

    if(argv[1]){
        my_fatal("main(): Unknown option: %s\n",argv[1]);
    }


    gui_init(GUI);
    mainloop();
    return EXIT_SUCCESS; 
}

// polyglot_set_option()

void polyglot_set_option(const char *name, const char *value){ // this must be cleaned up!
    option_t *opt;
    my_log("POLYGLOT Setting PolyGlot option %s=\"%s\"\n",name,value);
    if(my_string_case_equal(name,"Defaults")){
      option_start_iter(Uci->option);
      while((opt=option_next(Uci->option))){
	if(!IS_BUTTON(opt)){
	// also sets opt->value
	  uci_send_option(Uci,opt->name,opt->default_);
	}
      }
      option_start_iter(Option);
      while((opt=option_next(Option))){
	if(!IS_BUTTON(opt)){
	  polyglot_set_option(opt->name,opt->default_);
	}
      }
      xboard2uci_send_options();
    }
    option_set(Option,name,value);
    if(option_get_bool(Option,"Book")&&(my_string_case_equal(name,"BookFile")||my_string_case_equal(name,"Book"))){
        my_log("POLYGLOT *** SETTING BOOK ***\n");
        my_log("POLYGLOT BOOK \"%s\"\n",option_get_string(Option,"BookFile"));
        book_close();
        book_clear();
        book_open(option_get_string(Option,"BookFile"));
        if(!book_is_open()){
            my_log("POLYGLOT Unable to open book \"%s\"\n",option_get_string(Option,"BookFile"));
        }
    }else if(option_get_bool(Option,"Log")&&(my_string_case_equal(name,"LogFile") ||my_string_case_equal(name,"Log"))){
        my_log("POLYGLOT *** SWITCHING LOGFILE ***\n");
        my_log("POLYGLOT LOGFILE \"%s\"\n",option_get_string(Option,"LogFile"));
        my_log_close();
        my_log_open(option_get_string(Option,"LogFile"));
    }else if(option_get_bool(Option,"UseNice") &&(my_string_case_equal(name,"NiceValue")||my_string_case_equal(name,"UseNice"))){
        my_log("POLYGLOT Adjust Engine Piority\n");
        engine_set_nice_value(Engine,atoi(option_get_string(Option,"NiceValue")));
    }else if(my_string_case_equal(name,"Book") && !option_get_bool(Option,"Book")){
        book_close();
        book_clear();
    }else if(my_string_case_equal(name,"UseNice") && !option_get_bool(Option,"UseNice")){
        my_log("POLYGLOT Adjust Engine Piority\n");
        engine_set_nice_value(Engine,0);
    }else if(my_string_case_equal(name,"Log") && !option_get_bool(Option,"Log")){
        my_log("POLYGLOT QUIT LOGGING\n");
        my_log_close();
    }
}


// init_book()

static void init_book(){
    book_clear();
    if (option_get_bool(Option,"Book")){
        my_log("POLYGLOT *** SETTING BOOK ***\n");
        my_log("POLYGLOT BOOK \"%s\"\n",option_get_string(Option,"BookFile"));
        book_open(option_get_string(Option,"BookFile"));
        if(!book_is_open()){
            my_log("POLYGLOT Unable to open book \"%s\"\n",
                   option_get_string(Option,"BookFile"));
        }
    }
}


// quit()

void quit() {

    ini_t empty[1];

    ini_init(empty);

    my_log("POLYGLOT *** QUIT ***\n");
    
    if (Init) {
        
        stop_search();
        engine_send(Engine,"quit");
        my_log("POLYGLOT Closing engine\n");
        engine_close(Engine);
        
    }
    //    printf("def=%s val=%s\n",option_get_default(Option,"Persist"),option_get_string(Option,"Persist"));
    if(option_get_bool(Option,"Persist")){
        write_ini(option_get_string(Option,"PersistFile"),
                  Option,Uci->option);
    }else if(!my_string_case_equal(option_get_default(Option,"Persist"),
				  option_get_string(Option,"Persist"))){
      // Hack
      ini_insert_ex(empty,"PolyGlot","Persist","false");
      write_ini_ex(option_get_string(Option,"PersistFile"),empty);
    }else{
      write_ini_ex(option_get_string(Option,"PersistFile"),empty);
    }
    my_log("POLYGLOT Calling exit\n");
    exit(EXIT_SUCCESS);
}

// stop_search()

static void stop_search() {
    
    if (Init && Uci->searching) {
        
        ASSERT(Uci->searching);
        ASSERT(Uci->pending_nb>=1);
        
        my_log("POLYGLOT STOP SEARCH\n");
        
        if (option_get_bool(Option,"SyncStop")) {
            uci_send_stop_sync(Uci);
        } else {
            uci_send_stop(Uci);
        }
    }
}


// end of main.cpp

