
// main.c

// includes

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
#include "util.h"


// constants


static const char * const Version = "1.4.61b";
static const char * const HelpMessage = "\
SYNTAX\n\
* polyglot [configfile] [-noini] [-ec engine] [-ed enginedirectory] [-en enginename] [-log true/false] [-lf logfile] [-pg <name>=<value>]* [-uci <name>=<value>]*\n\
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

static void stop_search  ();

// functions

// arg_shift_left()

static void arg_shift_left(char **argv, int index){
    int i;
    for(i=index; argv[i]!=NULL; i++){
        argv[i]=argv[i+1];
    }
}

// parse_args()

static void parse_args(ini_t *ini, char **argv){
    int arg_index;
    char *arg;
    arg_index=0;
    while((arg=argv[arg_index])){
        if(my_string_equal(arg,"-ec") && argv[arg_index+1]){
            ini_insert_ex(ini,"PolyGlot","EngineCommand",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }if(my_string_equal(arg,"-ed") && argv[arg_index+1]){
            ini_insert_ex(ini,"PolyGlot","EngineDir",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-en") && argv[arg_index+1]){
            ini_insert_ex(ini,"PolyGlot","EngineName",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-log") &&
           argv[arg_index+1] &&
           IS_BOOL(argv[arg_index+1])){
            ini_insert_ex(ini,
                          "PolyGlot",
                          "Log",
                          TO_BOOL(argv[arg_index+1])?"true":"false");
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-lf") && argv[arg_index+1]){
            ini_insert_ex(ini,"PolyGlot","LogFile",argv[arg_index+1]);
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        if(my_string_equal(arg,"-wb") &&
           argv[arg_index+1]&&
           IS_BOOL(argv[arg_index+1])){
               ini_insert_ex(ini,"PolyGlot",
                             "OnlyWbOptions",
                             TO_BOOL(argv[arg_index+1])?"true":"false");
               arg_shift_left(argv,arg_index);
               arg_shift_left(argv,arg_index);
               continue;
        }
        if((my_string_equal(arg,"-pg")||my_string_equal(arg,"-uci")) &&
           argv[arg_index+1]){
            int ret;
            char section[StringSize];
            char name[StringSize];
            char value[StringSize];
            ret=ini_line_parse(argv[arg_index+1],section,name,value);
            if(ret==NAME_VALUE){
                if(my_string_equal(arg,"-pg")){
                    ini_insert_ex(ini,"PolyGlot",name,value);
                }else{
                    ini_insert_ex(ini,"Engine",name,value);
                }
            }
            arg_shift_left(argv,arg_index);
            arg_shift_left(argv,arg_index);
            continue;
        }
        arg_index++;
    }
}


// make_ini()

static void make_ini(ini_t *ini){
    option_t *opt;
    ini_insert_ex(ini,"polyglot",
		  "EngineCommand",
		  option_get(Option,"EngineCommand"));
    ini_insert_ex(ini,"polyglot",
		  "EngineDir",
		  option_get(Option,"EngineDir"));
    option_start_iter(Option);
    while((opt=option_next(Option))){
        if(my_string_case_equal(opt->name,"SettingsFile")) continue;
        if(my_string_case_equal(opt->name,"EngineCommand")) continue;
        if(my_string_case_equal(opt->name,"EngineDir")) continue;
        if(!my_string_equal(opt->value,opt->default_)&& !IS_BUTTON(opt->type))
        {
            ini_insert_ex(ini,"polyglot",opt->name,opt->value);
        }
    }
    option_start_iter(Uci->option);
    while((opt=option_next(Uci->option))){
        if(!strncmp(opt->name,"UCI_",4) &&
            !my_string_case_equal(opt->name,"UCI_LimitStrength") &&
            !my_string_case_equal(opt->name,"UCI_Elo"))continue;
        if(!my_string_equal(opt->value,opt->default_)&&
           !IS_BUTTON(opt->type)){
            ini_insert_ex(ini,"engine",opt->name,opt->value);
        }
    }
}


// write_ini()

static void write_ini(const char *filename,
			 ini_t *ini){
  // TODO Quote, dequote
    const char *quote;
    ini_entry_t *entry;
    char tmp[StringSize];
    char tmp1[StringSize];
    char tmp2[StringSize];
    FILE *f;
    time_t t=time(NULL);
    f=fopen(filename,"w");
    if(!f){
      gui_send(GUI,"tellusererror write_ini(): %s: %s.",filename,strerror(errno));
      my_log("POLYGLOT write_ini(): %s: %s.\n",filename,strerror(errno));
      return;
    }
    fprintf(f,"; Created: %s\n",ctime(&t));
    fprintf(f,"[PolyGlot]\n");
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
      if(my_string_case_equal(entry->section,"polyglot")){
	  my_quote(tmp1,entry->name,ini_specials);
	  my_quote(tmp2,entry->value,ini_specials);
          snprintf(tmp,sizeof(tmp),"%s=%s\n",
                   tmp1,
                   tmp2);
	tmp[sizeof(tmp)-1]='\0';
	fprintf(f,"%s",tmp);
      }
    }
    fprintf(f,"[Engine]\n");
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
      if(my_string_case_equal(entry->section,"engine")){
        my_quote(tmp1,entry->name,ini_specials);
	my_quote(tmp2,entry->value,ini_specials);
	snprintf(tmp,sizeof(tmp),"%s=%s\n",
                     tmp1,
                     tmp2);
	tmp[sizeof(tmp)-1]='\0';
	fprintf(f,"%s",tmp);
      }
    }
    fclose(f);
}

// welcome_message()

void welcome_message(char *buf){
    if(!DEBUG){
        sprintf(buf,
                "PolyGlot %s by Fabien Letouzey.\n",
                Version);
    }else{
        sprintf(buf,
                "PolyGlot %s by Fabien Letouzey (debug build).\n",
                Version);
    }
}

int wb_select(){
    option_t *opt;
    option_start_iter(Option);
    while((opt=option_next(Option))){
        opt->mode&=~XBOARD;
        if(opt->mode & XBSEL){
            opt->mode|=XBOARD; 
        }
    }
}

// main()

int main(int argc, char * argv[]) {
    ini_t ini[1], ini_command[1];
    ini_entry_t *entry;
    char *arg;
    int arg_index;
    bool NoIni;
    option_t *opt;
    char welcome[StringSize];


    welcome_message(welcome);
 
    printf("%s",welcome);


    if(argc>=2 && ((my_string_case_equal(argv[1],"help")) || (my_string_case_equal(argv[1],"-help")) || (my_string_case_equal(argv[1],"--help")) ||  (my_string_case_equal(argv[1],"-h")) ||  my_string_case_equal(argv[1],"/?"))){
        printf("%s\n",HelpMessage);
        return EXIT_SUCCESS;
    }

   // init

    Init = FALSE;

    gui_init(GUI);

    util_init();
    option_init_pg();
    
    square_init();
    piece_init();
    attack_init();
    
    hash_init();

    my_random_init();

    ini_init(ini);
    ini_init(ini_command);

        // book utilities: do not touch these
    
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
    parse_args(ini_command,argv+1);
    if(NoIni){
        option_set(Option,"SettingsFile","<empty>");
    }

        // Ok see if first argument looks like config file
    
    if(argv[1] && !my_string_equal(argv[1],"epd-test") && !(argv[1][0]=='-')){
                // first argument must be  config file
        if(!NoIni){
            option_set(Option,"SettingsFile",argv[1]);
        }else{
                // ignore
        }
        arg_shift_left(argv,1);
    }else{
            // Config file is the default.
            // This has already been set above or in "option_init_pg()"
    }



        // if we use a config file: load it!
    
    if(!my_string_equal(option_get_string(Option,"SettingsFile"),"<empty>")){
        if(ini_parse(ini,option_get_string(Option,"SettingsFile"))){
            my_fatal("main(): Can't open config file \"%s\": %s\n",
                   option_get_string(Option,"SettingsFile"),
                   strerror(errno));
        }
    }

        // Extract some important options

    if((entry=ini_find(ini,"polyglot","EngineCommand"))){
        option_set(Option,entry->name,entry->value);
    }
    if((entry=ini_find(ini,"polyglot","EngineDir"))){
        option_set(Option,entry->name,entry->value);
    }
    if((entry=ini_find(ini,"polyglot","EngineName"))){
        option_set(Option,entry->name,entry->value);
    }
    if((entry=ini_find(ini,"polyglot","Log"))){
        polyglot_set_option(entry->name,entry->value);
    }
    if((entry=ini_find(ini,"polyglot","LogFile"))){
        polyglot_set_option(entry->name,entry->value);
    }
    
        // Concession to WB 4.4.0
        // Treat "polyglot_1st.ini" and "polyglot_2nd.ini" specially

    if(option_get_bool(Option,"WbWorkArounds3")){
        const char *SettingsFile=option_get(Option,"SettingsFile");
        if(strstr(SettingsFile,"polyglot_1st.ini")||
           strstr(SettingsFile,"polyglot_2nd.ini")){
            option_set(Option,"SettingsFile","<empty>");
        }
    }

        // Look at command line for logging option. It is important
        // to start logging as soon as possible.

     if((entry=ini_find(ini_command,"PolyGlot","Log"))){
         option_set(Option,entry->name,entry->value);
    }
    if((entry=ini_find(ini_command,"PolyGlot","LogFile"))){
        option_set(Option,entry->name,entry->value);
    }
    
       // start logging if required
    
    if (option_get_bool(Option,"Log")) {
        my_log_open(option_get_string(Option,"LogFile"));
    }

        // log welcome stuff
    
    my_log("%s",welcome);
    my_log("POLYGLOT *** START ***\n");
    if(!my_string_equal(option_get_string(Option,"SettingsFile"),"<empty>")){
        my_log("POLYGLOT INI file \"%s\"\n",option_get_string(Option,"SettingsFile"));
    }


        // scavenge command line for options necessary to start the engine


    if((entry=ini_find(ini_command,"PolyGlot","EngineCommand"))){
        option_set(Option,entry->name,entry->value);
    }
    if((entry=ini_find(ini_command,"PolyGlot","EngineDir"))){
        option_set(Option,entry->name,entry->value);
    }
    if((entry=ini_find(ini_command,"PolyGlot","EngineName"))){
        option_set(Option,entry->name,entry->value);
    }

    // Make sure that EngineCommand has been set
    if(my_string_case_equal(option_get(Option,"EngineCommand"),"<empty>")){
      my_fatal("main(): EngineCommand not set\n");
    }

        // start engine
    
    engine_open(Engine);

    if(!engine_active(Engine)){
        my_fatal("main(): Could not start \"%s\"\n",option_get(Option,"EngineCommand"));
    }

        // switch to UCI mode if necessary
    
    if (option_get_bool(Option,"UCI")) {
        my_log("POLYGLOT *** Switching to UCI mode ***\n");
    }

        // initialize uci parsing and send uci command. 
        // Parse options and wait for uciok
    
    // XXX
    uci_open(Uci,Engine);

    option_set_default(Option,"EngineName",Uci->name);

        // get engine name from engine if not supplied in config file or on
        // the command line

    if (my_string_equal(option_get_string(Option,"EngineName"),"<empty>")) {
        option_set(Option,"EngineName",Uci->name);
    }


        // In the case we have been invoked with NoIni or StandardIni
        // we still have to load a config file.

    if(my_string_equal(option_get_string(Option,"SettingsFile"),"<empty>")){

            //  construct the name of the ConfigFile from the EngineName
        
        char tmp[StringSize];
        char option_file[StringSize];
        int i;
        snprintf(tmp,sizeof(tmp),"%s.ini",
                 option_get_string(Option,"EngineName"));
        tmp[sizeof(tmp)-1]='\0';
        for(i=0;i<strlen(tmp);i++){
            if(tmp[i]==' '){
                tmp[i]='_';
            }
        }
        my_path_join(option_file,
                     option_get_string(Option,"SettingsDir"),
                     tmp);
    // Load the config file
        option_set(Option,"SettingsFile",option_file);

        my_log("POLYGLOT INI file \"%s\"\n",option_get_string(Option,"SettingsFile"));
        if(ini_parse(ini,option_file)){
            my_log("POLYGLOT Unable to open %s\n",
                   option_get_string(Option,"SettingsFile")); 
        }
    }


    // Parse the command line and merge remaining options.

    ini_start_iter(ini_command);
    while((entry=ini_next(ini_command))){
        ini_insert(ini,entry);
    }

        // Remind the reader about the options that are now in effect.

    my_log("POLYGLOG OPTIONS \n");
    ini_disp(ini);

            // extract PG options
    
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
        if(my_string_case_equal(entry->section,"polyglot")){
            opt=option_find(Option,entry->name);
            if(opt && !IS_BUTTON(opt->type)){
                polyglot_set_option(entry->name,entry->value);
            }
        }
    }

        // Cater to our biggest customer:-)
    
    if(option_get_bool(Option,"OnlyWbOptions")){
        wb_select();
    }

        // done initializing
    
    Init = TRUE;
    
        // collect engine options from config file(s) and send to engine
    
    ini_start_iter(ini);
    while((entry=ini_next(ini))){
        if(my_string_case_equal(entry->section,"engine")){
                // also updates value in Uci->option
            uci_send_option(Uci,entry->name,"%s",entry->value);
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
        // It seems that XBoard sometimes passes empty strings as arguments
        // to PolyGlot. We ignore these. 

    argc=1;
    while((arg=argv[argc++])){
        if(!my_string_equal(arg,"")){
            my_fatal("main(): Incorrect use of option: \"%s\"\n",argv[argc-1]);
        }
    }

    //    gui_init(GUI);
    mainloop();
    return EXIT_SUCCESS; 
}

// polyglot_set_option()

void polyglot_set_option(const char *name, const char *value){ // this must be cleaned up!
    ini_t ini[1];
    int ret;
    ini_init(ini);
    my_log("POLYGLOT Setting PolyGlot option \"%s=%s\"\n",name,value);
    if(my_string_case_equal(name,"Save")){
        ret=my_mkdir(option_get(Option,"SettingsDir"));
        if(ret){
            my_log("POLYGLOT polyglot_set_option(): %s: %s\n",
                   option_get(Option,"SettingsDir"),
                   strerror(errno));
        }
        make_ini(ini);
        write_ini(option_get(Option,"SettingsFile"),ini);
        return;
    }
//    if(my_string_equal(option_get(Option,name),value)){
//        my_log("Not setting PolyGlot option \"%s\" "
//               "since it already as the correct value.\n",
//               name);
//        return;
//    }
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
        my_log("POLYGLOT NEW LOGFILE \"%s\"\n",option_get_string(Option,"LogFile"));
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



// quit()

void quit() {
    my_log("POLYGLOT *** QUIT ***\n");
    if (Init && !Engine->pipex->quit_pending) {
        stop_search();
	Engine->pipex->quit_pending=TRUE;
        engine_send(Engine,"quit");
        engine_close(Engine);
        
    }
    my_sleep(200);
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


// end of main.c

