
// xboard2uci.c

// includes

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "book.h"
#include "colour.h"
#include "engine.h"
#include "fen.h"
#include "game.h"
#include "gui.h"
#include "line.h"
#include "main.h"
#include "move.h"
#include "move_do.h"
#include "move_legal.h"
#include "option.h"
#include "parse.h"
#include "san.h"
#include "uci.h"
#include "uci2uci.h"
#include "util.h"
#include "xboard2uci.h"

// defines

#define StringSize 4096

// constants

static const bool UseDebug = FALSE;
static const bool DelayPong = FALSE;

// types

typedef struct {
   int state;
   bool computer[ColourNb];
   int exp_move;
   int hint_move;
   int resign_nb;
   my_timer_t timer[1];
} state_t;

typedef struct {
    bool has_feature_memory;
    bool has_feature_smp;
    bool has_feature_egt_nalimov;
    bool has_feature_egt_gaviota;
    bool analyse;
    bool computer;
    const char * name;
    bool ics;
    bool new_hack; // "new" is a C++ keyword
    bool ponder;
    int ping;
    bool post;
    int proto_ver;
    bool result;

    int mps;
    double base;
    double inc;
    
    bool time_limit;
    double time_max;
    
    bool depth_limit;
    int depth_max;
    
    double my_time;
    double opp_time;

    int node_rate;
} xb_t;

typedef enum { WAIT, THINK, PONDER, ANALYSE } dummy_state_t;

// variables

static state_t State[1];
static xb_t XB[1];

// prototypes

static void comp_move      (int move);
static void move_step      (int move);
static void board_update   ();

static void mess           ();
static void no_mess        (int move);

static void search_update  ();
static void search_clear   ();
static void update_remaining_time();
static int  report_best_score();
static bool kibitz_throttle (bool searching);
static void start_protected_command();
static void end_protected_command();

static bool active         ();
static bool ponder         ();
static bool ponder_ok      (int ponder_move);

static void stop_search    ();

static void send_board     (int extra_move);
static void send_pv        ();
static void send_info      ();

static void send_xboard_options ();

static void learn          (int result);


// functions

// xboard2uci_init()

void xboard2uci_init() {
   // init

   game_clear(Game);

   // state

   State->state = WAIT;

   State->computer[White] = FALSE;
   State->computer[Black] = TRUE;

   State->exp_move = MoveNone;
   State->hint_move = MoveNone;
   State->resign_nb = 0;
   my_timer_reset(State->timer);

   // yes there are engines that do not have the "Hash" option....
   XB->has_feature_memory= (option_find(Uci->option,"Hash")!=NULL);
   XB->has_feature_smp = (uci_thread_option(Uci)!=NULL);
   // TODO: support for other types of table bases
   // This is a quick hack. 
   XB->has_feature_egt_nalimov = (option_find(Uci->option,"NalimovPath")!=NULL);
   XB->has_feature_egt_gaviota = (option_find(Uci->option,"GaviotaTbPath")!=NULL);
   XB->analyse = FALSE;
   XB->computer = FALSE;
   XB->name = NULL;
   my_string_set(&XB->name,"<empty>");
   XB->ics = FALSE;
   XB->new_hack = TRUE;
   XB->ping = -1;
   XB->ponder = FALSE;
   XB->post = FALSE;
   XB->proto_ver = 1;
   XB->result = FALSE;

   XB->mps = 0;
   XB->base = 300.0;
   XB->inc = 0.0;

   XB->time_limit = FALSE;
   XB->time_max = 5.0;

   XB->depth_limit = FALSE;
   XB->depth_max = 127;

   XB->my_time = 300.0;
   XB->opp_time = 300.0;

   XB->node_rate = -1;
}

// xboard2uci_gui_step()

void xboard2uci_gui_step(char string[]) {

	int move;
	char move_string[256];
	board_t board[1];

		if (FALSE) {
         
		} else if (match(string,"accepted *")) {

			// ignore

		} else if (match(string,"analyze")) {

			State->computer[White] = FALSE;
			State->computer[Black] = FALSE;

			XB->analyse = TRUE;
			XB->new_hack = FALSE;
			ASSERT(!XB->result);
			XB->result = FALSE;

			mess();

		} else if (match(string,"bk")) {

			if (option_get_bool(Option,"Book")) {
				game_get_board(Game,board);
				book_disp(board);
			}

		} else if (match(string,"black")) {

			if (colour_is_black(game_turn(Game))) {

				State->computer[White] = TRUE;
				State->computer[Black] = FALSE;

				XB->new_hack = TRUE;
				XB->result = FALSE;

				mess();
			}

		} else if (match(string,"computer")) {

			XB->computer = TRUE;

		} else if (match(string,"draw")) {
			if(option_find(Uci->option,"UCI_DrawOffers")){
			    my_log("POLYGLOT draw from XB received");
				uci_send_option(Uci,"DrawOffer","%s","draw");}
		} else if (match(string,"easy")) {

			XB->ponder = FALSE;

			mess();

		} else if (match(string,"edit")) {

			// refuse

			gui_send(GUI,"Error (unknown command): %s",string);

		} else if (match(string,"exit")) {

			State->computer[White] = FALSE;
			State->computer[Black] = FALSE;

			XB->analyse = FALSE;

			mess();

		} else if (match(string,"force")) {

			State->computer[White] = FALSE;
			State->computer[Black] = FALSE;

			mess();

		} else if (match(string,"go")) {

			State->computer[game_turn(Game)] = TRUE;
			State->computer[colour_opp(game_turn(Game))] = FALSE;

			XB->new_hack = FALSE;
			ASSERT(!XB->result);
			XB->result = FALSE;

			mess();

		} else if (match(string,"hard")) {

			XB->ponder = TRUE;

			mess();

		} else if (match(string,"hint")) {
		    
		        move=MoveNone;
			game_get_board(Game,board);
			if (option_get_bool(Option,"Book")) {

				move = book_move(board,FALSE);
			}
			if(move==MoveNone && State->hint_move!=MoveNone){
			    move=State->hint_move;
			    
			}
			if (move != MoveNone && move_is_legal(move,board)) {
			    move_to_san(move,board,move_string,256);
			    gui_send(GUI,"Hint: %s",move_string);
			}

		} else if (match(string,"ics *")) {

			XB->ics = TRUE;

		} else if (match(string,"level * *:* *")) {

			XB->mps  = atoi(Star[0]);
			XB->base = ((double)atoi(Star[1])) * 60.0 + ((double)atoi(Star[2]));
			XB->inc  = ((double)atoi(Star[3]));

		} else if (match(string,"level * * *")) {

			XB->mps  = atoi(Star[0]);
			XB->base = ((double)atoi(Star[1])) * 60.0;
			XB->inc  = ((double)atoi(Star[2]));

		} else if (match(string,"name *")) {

			my_string_set(&XB->name,Star[0]);

		} else if (match(string,"new")) {

		    uci_send_isready_sync(Uci);
			my_log("POLYGLOT NEW GAME\n");

			option_set(Option,"Chess960","false");

			game_clear(Game);

			if (XB->analyse) {
				State->computer[White] = FALSE;
				State->computer[Black] = FALSE;
			} else {
				State->computer[White] = FALSE;
				State->computer[Black] = TRUE;
			}

			XB->new_hack = TRUE;
			XB->result = FALSE;

			XB->depth_limit = FALSE;
            XB->node_rate=-1;

			XB->computer = FALSE;
			my_string_set(&XB->name,"<empty>");

			board_update();
			mess();

			uci_send_ucinewgame(Uci);

		} else if (match(string,"nopost")) {

			XB->post = FALSE;

		} else if (match(string,"otim *")) {

			XB->opp_time = ((double)atoi(Star[0])) / 100.0;
			if (XB->opp_time < 0.0) XB->opp_time = 0.0;

		} else if (match(string,"pause")) {

			// refuse

			gui_send(GUI,"Error (unknown command): %s",string);

		} else if (match(string,"ping *")) {

			// HACK; TODO: answer only after an engine move

			if (DelayPong) {
				if (XB->ping >= 0) gui_send(GUI,"pong %d",XB->ping); // HACK: get rid of old ping
				XB->ping = atoi(Star[0]);
				uci_send_isready_sync(Uci);
			} else {
				ASSERT(XB->ping==-1);
				gui_send(GUI,"pong %s",Star[0]);
			}
        } else if (match(string,"nps *")) {
            
                // fake WB play-by-nodes mode
            XB->node_rate = atoi(Star[0]);
		} else if (match(string,"playother")) {

			State->computer[game_turn(Game)] = FALSE;
			State->computer[colour_opp(game_turn(Game))] = TRUE;

			XB->new_hack = FALSE;
			ASSERT(!XB->result);
			XB->result = FALSE;

			mess();

		} else if (match(string,"post")) {

			XB->post = TRUE;

		} else if (match(string,"protover *")) {
            XB->proto_ver = atoi(Star[0]);
            ASSERT(XB->proto_ver>=2);
            send_xboard_options();

		} else if (match(string,"quit")) {
			my_log("POLYGLOT *** \"quit\" from GUI ***\n");
			quit();
		} else if (match(string,"random")) {

			// ignore

		} else if (match(string,"rating * *")) {

			// ignore

		} else if (match(string,"remove")) {

			if (game_pos(Game) >= 2) {

				game_goto(Game,game_pos(Game)-2);

				ASSERT(!XB->new_hack);
				XB->new_hack = FALSE; // HACK?
				XB->result = FALSE;

				board_update();
				mess();
			}

		} else if (match(string,"rejected *")) {

			// ignore

		} else if (match(string,"reset")) { // protover 3?

			// refuse

			gui_send(GUI,"Error (unknown command): %s",string);

		} else if (FALSE
			|| match(string,"result * {*}")
			|| match(string,"result * {* }")
			|| match(string,"result * { *}")
			|| match(string,"result * { * }")) {

				my_log("POLYGLOT GAME END\n");

				XB->result = TRUE;

				mess();

				// book learning

				if (option_get_bool(Option,"Book") &&
                    option_get_bool(Option,"BookLearn")) {

					if (FALSE) {
					} else if (my_string_equal(Star[0],"1-0")) {
						learn(+1);
					} else if (my_string_equal(Star[0],"0-1")) {
						learn(-1);
					} else if (my_string_equal(Star[0],"1/2-1/2")) {
						learn(0);
					}
				}
		} else if (match(string,"resume")) {

			// refuse

			gui_send(GUI,"Error (unknown command): %s",string);

        } else if (match(string,"option *=*")   ||
                   match(string,"option * =*") ||
                   match(string,"option *= *") ||
                   match(string,"option * = *")
                   ){
            char *name=Star[0];
            char *value=Star[1];
            if(match(name, "Polyglot *")){
                char *pg_name=Star[0];
                polyglot_set_option(pg_name,value);
            }else{
                option_t *opt=option_find(Uci->option,name);
                if(opt){
                    if(my_string_case_equal(opt->type,"check")){
                       value=my_string_equal(value,"1")?"true":"false";
                    }
                    start_protected_command();
                    uci_send_option(Uci, name, "%s", value);
                    end_protected_command();
                }else{
                    gui_send(GUI,"Error (unknown option): %s",name); 
                }
            }
        } else if (match(string,"option *")){
            char *name=Star[0];
             if(match(name, "Polyglot *")){
                char *pg_name=Star[0];
                polyglot_set_option(pg_name,"<empty>");
	     }else{           
	       start_protected_command();
                // value is ignored
	       if(!uci_send_option(Uci, name, "%s", "<empty>")){
		 gui_send(GUI,"Error (unknown option): %s",name); 
	       }; 
	       end_protected_command();
	     }
        } else if (XB->has_feature_smp && match(string,"cores *")){
                int cores=atoi(Star[0]);
                if(cores>=1){
                    // updating the number of cores
                    my_log("POLYGLOT setting the number of cores to %d\n",cores);
                    start_protected_command();
                    uci_set_threads(Uci,cores); 
                    end_protected_command();
                } else{
                   // refuse
                    gui_send(GUI,"Error (unknown command): %s",string);
                }
        } else if (match(string,"egtpath * *")){
                char *type=Star[0];
                char *path=Star[1];
                if(my_string_empty(path)){
                    // refuse
                    gui_send(GUI,"Error (unknown command): %s",string);
                }else{
		    if(my_string_case_equal(type,"nalimov") && XB->has_feature_egt_nalimov){
			// updating NalimovPath
			my_log("POLYGLOT setting the Nalimov path to %s\n",path);
			start_protected_command();
			uci_send_option(Uci,"NalimovPath","%s",path);
			end_protected_command();
		    }else if(my_string_case_equal(type,"gaviota") && XB->has_feature_egt_gaviota){
			// updating GaviotaPath
			my_log("POLYGLOT setting the Gaviota path to %s\n",path);
			start_protected_command();
			uci_send_option(Uci,"GaviotaTbPath","%s",path);
			end_protected_command();
		    }else{
			// refuse
			gui_send(GUI,"Error (unsupported table base format): %s",string);
		    }
                }
        } else if (XB->has_feature_memory && match(string,"memory *")){
            int memory = atoi(Star[0]);
            int egt_cache;
            int real_memory;
            if(memory>=1){
                // updating the available memory
                option_t *opt;
                my_log("POLYGLOT setting the amount of memory to %dMb\n",memory);
                if(XB->has_feature_egt_nalimov && (opt=option_find(Uci->option,"NalimovCache"))){
                    egt_cache=atoi(opt->value);
                }else if(XB->has_feature_egt_gaviota && 
			 (opt=option_find(Uci->option,"GaviotaTbCache"))){
		    egt_cache=atoi(opt->value);
		}else{
                    egt_cache=0;
                }
                my_log("POLYGLOT EGTB Cache is %dMb\n",egt_cache);
                real_memory=memory-egt_cache;
                if(real_memory>0){
                    start_protected_command();
                    uci_send_option(Uci,"Hash", "%d", real_memory);
                    end_protected_command();
                }
            }else{
                // refuse
                gui_send(GUI,"Error (unknown command): %s",string);
            }

		} else if (match(string,"sd *")) {

			XB->depth_limit = TRUE;
			XB->depth_max = atoi(Star[0]);

		} else if (match(string,"setboard *")) {

			my_log("POLYGLOT FEN %s\n",Star[0]);

			if (!game_init(Game,Star[0])) my_fatal("xboard_step(): bad FEN \"%s\"\n",Star[0]);

			State->computer[White] = FALSE;
			State->computer[Black] = FALSE;

			XB->new_hack = TRUE; // HACK?
			XB->result = FALSE;

			board_update();
			mess();

		} else if (match(string,"st *")) {

			XB->time_limit = TRUE;
			XB->time_max = ((double)atoi(Star[0]));

		} else if (match(string,"time *")) {

			XB->my_time = ((double)atoi(Star[0])) / 100.0;
			if (XB->my_time < 0.0) XB->my_time = 0.0;

		} else if (match(string,"undo")) {

			if (game_pos(Game) >= 1) {

				game_goto(Game,game_pos(Game)-1);

				ASSERT(!XB->new_hack);
				XB->new_hack = FALSE; // HACK?
				XB->result = FALSE;

				board_update();
				mess();
			}

		} else if (match(string,"usermove *")) {

			game_get_board(Game,board);
			move = move_from_san(Star[0],board);

			if (move != MoveNone && move_is_legal(move,board)) {

				XB->new_hack = FALSE;
				ASSERT(!XB->result);
				XB->result = FALSE;

				move_step(move);
				no_mess(move);

			} else {

				gui_send(GUI,"Illegal move: %s",Star[0]);
			}

		} else if (match(string,"variant *")) {

			if (my_string_equal(Star[0],"fischerandom")) {
				option_set(Option,"Chess960","true");
			} else {
				option_set(Option,"Chess960","false");
			}

		} else if (match(string,"white")) {

			if (colour_is_white(game_turn(Game))) {

				State->computer[White] = FALSE;
				State->computer[Black] = TRUE;

				XB->new_hack = TRUE;
				XB->result = FALSE;

				mess();
			}

		} else if (match(string,"xboard")) {

			// ignore

		} else if (match(string,".")) { // analyse info

			if (State->state == ANALYSE) {
				int depth=Uci->best_depth;//HACK: don't clear engine-output window...

				ASSERT(Uci->searching);
				ASSERT(Uci->pending_nb>=1);

				if (Uci->root_move != MoveNone && move_is_legal(Uci->root_move,Uci->board)) {
					move_to_san(Uci->root_move,Uci->board,move_string,256);
					gui_send(GUI,"stat01: %.0f "S64_FORMAT" %d %d %d %s",Uci->time*100.0,Uci->node_nb,/*Uci->*/depth,Uci->root_move_nb-(Uci->root_move_pos+1),Uci->root_move_nb,move_string);
				} else {
					gui_send(GUI,"stat01: %.0f "S64_FORMAT" %d %d %d",Uci->time*100.0,Uci->node_nb,/*Uci->*/depth,0,0); // HACK
				}
			}

		} else if (match(string,"?")) { // move now

			if (State->state == THINK) {

				ASSERT(Uci->searching);
				ASSERT(Uci->pending_nb>=1);

				// HACK: just send "stop" to the engine

				if (Uci->searching) {
					my_log("POLYGLOT STOP SEARCH\n");
					engine_send(Engine,"stop");
				}
			}

		} else { // unknown command, maybe a move?

			game_get_board(Game,board);
			move = move_from_san(string,board);

			if (move != MoveNone && move_is_legal(move,board)) {

				XB->new_hack = FALSE;
				ASSERT(!XB->result);
				XB->result = FALSE;

				move_step(move);
				no_mess(move);

			} else if (move != MoveNone) {

				gui_send(GUI,"Illegal move: %s",string);

			} else {

				gui_send(GUI,"Error (unknown command): %s",string);
			}
		}
	return;
}

// xboard2uci_engine_step()

void xboard2uci_engine_step(char string[]) {

	int event;
    board_t board[1];
		event = uci_parse(Uci,string);

		// react to events

		if ((event & EVENT_READY) != 0) {

			// the engine is now ready

			if (!Uci->ready) {
				Uci->ready = TRUE;
                    //	if (XB->proto_ver >= 2) xboard_send(XBoard,"feature done=1");
			}

			if (!DelayPong && XB->ping >= 0) {
				gui_send(GUI,"pong %d",XB->ping);
				XB->ping = -1;
			}
		}

		if ((event & EVENT_MOVE) != 0 && State->state == THINK) {

			// the engine is playing a move

			// MEGA HACK: estimate remaining time because XBoard won't send it!

			my_timer_stop(State->timer);

			XB->my_time -= my_timer_elapsed_real(State->timer);
			XB->my_time += XB->inc;
			if (XB->mps != 0 && (game_move_nb(Game) + 1) % XB->mps == 0) XB->my_time += XB->base;

			if (XB->my_time < 0.0) XB->my_time = 0.0;

			// make sure to remember the ponder move

			State->hint_move=Uci->ponder_move;

			// play the engine move

			comp_move(Uci->best_move);

		}

		if ((event & EVENT_PV) != 0) {

			// the engine has sent a new PV

			send_pv();
		}
		if ((event & EVENT_INFO) != 0) {

			// the engine has sent info

			send_info();
		}
		if((event & (EVENT_DRAW|EVENT_RESIGN))!=0){
			my_log("POYGLOT draw offer/resign from engine\n");
			if(option_find(Uci->option,"UCI_DrawOffers")){
				if(event & EVENT_DRAW)
					gui_send(GUI,"offer draw");
				else
					gui_send(GUI,"resign");
			}
		}
		if(((event & EVENT_ILLEGAL_MOVE)!=0) && (State->state == THINK)){
		    game_get_board(Game,board);
		    if(board->turn==White){
			gui_send(GUI,"0-1 {polyglot: resign"
				 " (illegal engine move by white: %s)}",Uci->bestmove);
		    }else{
			gui_send(GUI,"1-0 {polyglot: resign"
				 " (illegal engine move by black: %s)}",Uci->bestmove);
		    }
		    board_disp(board);
		    XB->result = TRUE;
		    mess();
		}
}

// format_xboard_option_line

void format_xboard_option_line(char * option_line, option_t *opt){
    int j;
    char option_string[StringSize];
    char *tmp;
    strcpy(option_line,"");
        // buffer overflow alert
    strcat(option_line,"feature option=\"");
    if(opt->mode&PG){
        strcat(option_line,"Polyglot ");
    }
    sprintf(option_string,"%s",opt->name);
    strcat(option_line,option_string);
    sprintf(option_string," -%s",opt->type);
    strcat(option_line,option_string);
    if(!IS_BUTTON(opt->type) && strcmp(opt->type,"combo")){
        if(strcmp(opt->type,"check")){
            sprintf(option_string," %s",opt->value);
        }else{
            sprintf(option_string," %d",
                    my_string_case_equal(opt->value,"true")||
                    my_string_equal(opt->value,"1")
                    ?1:0);
        }
        strcat(option_line,option_string);
    }
    if(IS_SPIN(opt->type)){
        sprintf(option_string," %s",opt->min);
            strcat(option_line,option_string);
    }
    if(IS_SPIN(opt->type)){
        sprintf(option_string," %s",opt->max);
        strcat(option_line,option_string);
    }
    for(j=0;j<opt->var_nb;j++){
        if(!strcmp(opt->var[j],opt->value)){
            sprintf(option_string," *%s",opt->var[j]);
        }else{
            sprintf(option_string," %s",opt->var[j]);
        }
        strcat(option_line,option_string);
        if(j!=opt->var_nb-1){
            strcat(option_line," ///");
        }
    }
    strcat(option_line,"\"");
    if(option_get_bool(Option,"WbWorkArounds") &&
       (tmp=strstr(option_line,"Draw"))){
        *tmp='d';
        my_log("POLYGLOT Decapitalizing \"Draw\" in option \"%s\"\n",
               opt->name);
    }
}

// send_xboard_options()

static void send_xboard_options(){
    
    char egtfeature[StringSize];
    int tbs=0;
    
    gui_send(GUI,"feature done=0");
    
    gui_send(GUI,"feature analyze=1");
    gui_send(GUI,"feature colors=0");
    gui_send(GUI,"feature draw=1");
    gui_send(GUI,"feature ics=1");
    gui_send(GUI,"feature myname=\"%s\"",
             option_get_string(Option,"EngineName"));
    gui_send(GUI,"feature name=1");
    gui_send(GUI,"feature pause=0");
    gui_send(GUI,"feature ping=1");
    gui_send(GUI,"feature playother=1");
    gui_send(GUI,"feature sigint=1");
    gui_send(GUI,"feature reuse=1");
    gui_send(GUI,"feature san=0");
    gui_send(GUI,"feature setboard=1");
    gui_send(GUI,"feature sigint=0");
    gui_send(GUI,"feature sigterm=0");
    gui_send(GUI,"feature time=1");
    gui_send(GUI,"feature usermove=1");
    gui_send(GUI,"feature nps=1");
    if (XB->has_feature_memory){
        gui_send(GUI,"feature memory=1");
    }else{
        gui_send(GUI,"feature memory=0");
    }
    if (XB->has_feature_smp){
        gui_send(GUI,"feature smp=1");
    }else{
        gui_send(GUI,"feature smp=0");
    }
    egtfeature[0]='\0';
    strncat(egtfeature,"feature egt=\"",StringSize);
    if (XB->has_feature_egt_nalimov){
	tbs++;
	strncat(egtfeature,"nalimov",StringSize-strlen(egtfeature));
    }
    if (XB->has_feature_egt_gaviota){
	if(tbs>0){
	    strncat(egtfeature,",",StringSize-strlen(egtfeature));
	}
	strncat(egtfeature,"gaviota",StringSize-strlen(egtfeature));
    }
    strncat(egtfeature,"\"",StringSize-strlen(egtfeature));
    egtfeature[StringSize-1]='\0';
    gui_send(GUI,egtfeature);
    
    if (option_find(Uci->option,"UCI_Chess960")) {
        gui_send(GUI,"feature variants=\"normal,fischerandom\"");
    } else {
        gui_send(GUI,"feature variants=\"normal\"");
    }

    xboard2uci_send_options();
}

void xboard2uci_send_options(){
  char option_line[StringSize]="";
  const char * name;
  option_t *opt;
  
  option_start_iter(Uci->option);
  while((opt=option_next(Uci->option))){
    if(my_string_case_equal(opt->name,"UCI_AnalyseMode")) continue;
    if(my_string_case_equal(opt->name,"UCI_Opponent")) continue;
    if(my_string_case_equal(opt->name,"UCI_Chess960")) continue;
    if(my_string_case_equal(opt->name,"UCI_ShowCurrLine")) continue;
    if(my_string_case_equal(opt->name,"UCI_ShowRefutations")) continue;
    if(my_string_case_equal(opt->name,"UCI_ShredderbasesPath")) continue;
    if(my_string_case_equal(opt->name,"UCI_SetPositionValue")) continue;
    if(my_string_case_equal(opt->name,"UCI_DrawOffers")) continue;
    if(my_string_case_equal(opt->name,"Ponder")) continue;
    if(my_string_case_equal(opt->name,"Hash")) continue;
    if(my_string_case_equal(opt->name,"NalimovPath")) continue;
    if(my_string_case_equal(opt->name,"GaviotaTbPath")) continue;
    if((name=uci_thread_option(Uci))!=NULL &&
       my_string_case_equal(opt->name,name)) continue;
    format_xboard_option_line(option_line,opt);
    
    gui_send(GUI,"%s",option_line);
  }
  
  
  option_start_iter(Option);
  while((opt=option_next(Option))){
    if(opt->mode &XBOARD){
      format_xboard_option_line(option_line,opt);
      gui_send(GUI,"%s",option_line);
    }
  }       
  gui_send(GUI,"feature done=1"); 
  
}

// report_best_score()

static int report_best_score(){
    if(!option_get_bool(Option,"ScoreWhite") ||
       colour_is_white(Uci->board->turn)){
        return Uci->best_score;
    }else{
        return -Uci->best_score;
    }
}

// comp_move()

static void comp_move(int move) {

   board_t board[1];
   char string[256];

   ASSERT(move_is_ok(move));

   ASSERT(State->state==THINK);
   ASSERT(!XB->analyse);

   if(option_get_bool(Option,"RepeatPV"))
	   send_pv(); // to update time and nodes

   // send the move

   game_get_board(Game,board);

   if (move_is_castle(move,board) && option_get_bool(Option,"Chess960")) {
      if (!move_to_san(move,board,string,256)) my_fatal("comp_move(): move_to_san() failed\n"); // O-O/O-O-O
   } else {
      if (!move_to_can(move,board,string,256)) my_fatal("comp_move(): move_to_can() failed\n");
   }

   gui_send(GUI,"move %s",string);

   // resign?

   if (option_get_bool(Option,"Resign") && Uci->root_move_nb > 1) {

       if (Uci->best_score <= -abs(option_get_int(Option,"ResignScore"))) {

         State->resign_nb++;
         my_log("POLYGLOT %d move%s with resign score\n",State->resign_nb,(State->resign_nb>1)?"s":"");

         if (State->resign_nb >= option_get_int(Option,"ResignMoves")) {
            my_log("POLYGLOT *** RESIGN ***\n");
            gui_send(GUI,"resign");
         }

      } else {

         if (State->resign_nb > 0) my_log("POLYGLOT resign reset (State->resign_nb=%d)\n",State->resign_nb);
         State->resign_nb = 0;
      }
   }

   // play the move

   move_step(move);
   no_mess(move);
}

// move_step()

static void move_step(int move) {

   board_t board[1];
   char move_string[256];

   ASSERT(move_is_ok(move));

   // log

   game_get_board(Game,board);

   if (move != MoveNone && move_is_legal(move,board)) {

      move_to_san(move,board,move_string,256);
      my_log("POLYGLOT MOVE %s\n",move_string);

   } else {

      move_to_can(move,board,move_string,256);
      my_log("POLYGLOT ILLEGAL MOVE \"%s\"\n",move_string);
      board_disp(board);

      my_fatal("move_step(): illegal move \"%s\"\n",move_string);
   }

   // play the move

   game_add_move(Game,move);
   board_update();
}

// board_update()

static void board_update() {

   // handle game end

   ASSERT(!XB->result);

   switch (game_status(Game)) {
   case PLAYING:
      break;
   case WHITE_MATES:
      gui_send(GUI,"1-0 {White mates}");
      break;
   case BLACK_MATES:
      gui_send(GUI,"0-1 {Black mates}");
      break;
   case STALEMATE:
      gui_send(GUI,"1/2-1/2 {Stalemate}");
      break;
   case DRAW_MATERIAL:
      gui_send(GUI,"1/2-1/2 {Draw by insufficient material}");
      break;
   case DRAW_FIFTY:
      gui_send(GUI,"1/2-1/2 {Draw by fifty-move rule}");
      break;
   case DRAW_REPETITION:
      gui_send(GUI,"1/2-1/2 {Draw by repetition}");
      break;
   default:
      ASSERT(FALSE);
      break;
   }
}

// mess()

static void mess() {

   // clear state variables

   State->resign_nb = 0;
   State->exp_move = MoveNone;
   my_timer_reset(State->timer);

   // abort a possible search

   stop_search();

   // calculate the new state

   if (FALSE) {
   } else if (!active()) {
      State->state = WAIT;
      my_log("POLYGLOT WAIT\n");
   } else if (XB->analyse) {
      State->state = ANALYSE;
      my_log("POLYGLOT ANALYSE\n");
   } else if (State->computer[game_turn(Game)]) {
      State->state = THINK;
      my_log("POLYGLOT THINK\n");
   } else {
      State->state = WAIT;
      my_log("POLYGLOT WAIT\n");
   }

   search_update();
}

// no_mess()

static void no_mess(int move) {

   ASSERT(move_is_ok(move));

   // just received a move, calculate the new state

   if (FALSE) {

   } else if (!active()) {

      stop_search(); // abort a possible search

      State->state = WAIT;
      State->exp_move = MoveNone;

      my_log("POLYGLOT WAIT\n");

   } else if (State->state == WAIT) {

      ASSERT(State->computer[game_turn(Game)]);
      ASSERT(!State->computer[colour_opp(game_turn(Game))]);
      ASSERT(!XB->analyse);

      my_log("POLYGLOT WAIT -> THINK\n");

      State->state = THINK;
      State->exp_move = MoveNone;

   } else if (State->state == THINK) {

      ASSERT(!State->computer[game_turn(Game)]);
      ASSERT(State->computer[colour_opp(game_turn(Game))]);
      ASSERT(!XB->analyse);

      if (ponder() && ponder_ok(Uci->ponder_move)) {

         my_log("POLYGLOT THINK -> PONDER\n");

         State->state = PONDER;
         State->exp_move = Uci->ponder_move;

      } else {

         my_log("POLYGLOT THINK -> WAIT\n");

         State->state = WAIT;
         State->exp_move = MoveNone;
      }

   } else if (State->state == PONDER) {

      ASSERT(State->computer[game_turn(Game)]);
      ASSERT(!State->computer[colour_opp(game_turn(Game))]);
      ASSERT(!XB->analyse);

      if (move == State->exp_move && Uci->searching) {

         ASSERT(Uci->searching);
         ASSERT(Uci->pending_nb>=1);

         my_timer_start(State->timer);//also resets

         my_log("POLYGLOT PONDER -> THINK (*** HIT ***)\n");
         engine_send(Engine,"ponderhit");

         State->state = THINK;
         State->exp_move = MoveNone;

         send_pv(); // update display

         return; // do not launch a new search

      } else {

         my_log("POLYGLOT PONDER -> THINK (miss)\n");

         stop_search();

         State->state = THINK;
         State->exp_move = MoveNone;
      }

   } else if (State->state == ANALYSE) {

      ASSERT(XB->analyse);

      my_log("POLYGLOT ANALYSE -> ANALYSE\n");

      stop_search();

   } else {

      ASSERT(FALSE);
   }

   search_update();
}

// start_protected_command()

static void start_protected_command(){
    stop_search();
}

static void end_protected_command(){
    if(Uci->ready){ // not init faze
        uci_send_isready_sync(Uci); // gobble up spurious "bestmove"
    }
    update_remaining_time();
    search_update();   // relaunch search if necessary
}

// update_remaining_time()

static void update_remaining_time(){
   double reduce;
   if(State->timer->running){
       my_timer_stop(State->timer);
       reduce = my_timer_elapsed_real(State->timer);
       my_log("POLYGLOT reducing remaing time by %f seconds\n",reduce);
       XB->my_time -= reduce;
       if(XB->my_time<0.0){
           XB->my_time=0.0;
       }
   }
}


// search_update()

static void search_update() {

   int move;
   int move_nb;
   board_t board[1];

   ASSERT(!Uci->searching);



   
   // launch a new search if needed

   

   if (State->state == THINK || State->state == PONDER || State->state == ANALYSE) {

      // [VdB] moved up as we need the move number

       game_get_board(Game,Uci->board);

      // opening book

       if (State->state == THINK &&
           option_get_bool(Option,"Book") &&
           Uci->board->move_nb<option_get_int(Option,"BookDepth")
           ) {


         move = book_move(Uci->board,option_get_bool(Option,"BookRandom"));

         if (move != MoveNone && move_is_legal(move,Uci->board)) {

            my_log("POLYGLOT *BOOK MOVE*\n");

            search_clear(); // clears Uci->ponder_move
            Uci->best_move = move;

            board_copy(board,Uci->board);
            move_do(board,move);
            Uci->ponder_move = book_move(board,FALSE); // expected move = best book move

            Uci->best_pv[0] = Uci->best_move;
            Uci->best_pv[1] = Uci->ponder_move; // can be MoveNone
            Uci->best_pv[2] = MoveNone;

            comp_move(Uci->best_move);

            return;
         }
      }

      // engine search

      my_log("POLYGLOT START SEARCH\n");

      // options

      uci_send_option(Uci,"UCI_Chess960","%s",
                      option_get_bool(Option,"Chess960")?"true":"false");

      if (option_get_int(Option,"UCIVersion") >= 2) {
         uci_send_option(Uci,"UCI_Opponent","none none %s %s",(XB->computer)?"computer":"human",XB->name);
         uci_send_option(Uci,"UCI_AnalyseMode","%s",(XB->analyse)?"true":"false");
      }

      uci_send_option(Uci,"Ponder","%s",ponder()?"true":"false");

      // position

      move = (State->state == PONDER) ? State->exp_move : MoveNone;
      send_board(move); // updates Uci->board global variable

      // search

      if (State->state == THINK || State->state == PONDER) {

         engine_send_queue(Engine,"go");

         if (XB->time_limit) {

            // fixed time per move
             
             if(XB->node_rate > 0){
                 engine_send_queue(Engine,
                                   " nodes %.0f",
                                   XB->time_max*((double)XB->node_rate));
             }else{
		 double computed_time;
		 double st_fudge;
		 st_fudge=(double) option_get_int(Option,"STFudge");
		 my_log("POLYGLOT Giving engine %.0fmsec extra time.\n",st_fudge);
		 computed_time=XB->time_max*1000.0-st_fudge;
		 if(computed_time< 1.0){
		     computed_time=1.0;
		 }
                 engine_send_queue(Engine,
                                   " movetime %.0f",
                                   computed_time);
             }

         } else {

            // time controls

                 if(XB->node_rate > 0) {
                     double time;
                     move_nb = 40;
                     if (XB->mps != 0){
                         move_nb = XB->mps - (Uci->board->move_nb % XB->mps);
                     }
                     time = XB->my_time / move_nb;
                     if(XB->inc != 0){
                         time += XB->inc;
                     }
                     if(time > XB->my_time){
                         time = XB->my_time;
                     }
                     engine_send_queue(Engine,
                                       " nodes %.0f",
                                       time*XB->node_rate);
                 } else {
                     
                     if (colour_is_white(Uci->board->turn)) {
                         engine_send_queue(Engine,
                                           " wtime %.0f btime %.0f",
                                           XB->my_time*1000.0,XB->opp_time*1000.0);
                     } else {
                         engine_send_queue(Engine,
                                           " wtime %.0f btime %.0f",
                                           XB->opp_time*1000.0,XB->my_time*1000.0);
                     }
                     
                     if (XB->inc != 0.0){
                         engine_send_queue(Engine,
                                           " winc %.0f binc %.0f",
                                           XB->inc*1000.0,XB->inc*1000.0);
                     }
                     if (XB->mps != 0) {

                         move_nb = XB->mps - (Uci->board->move_nb % XB->mps);
                         ASSERT(move_nb>=1&&move_nb<=XB->mps);
                         
                         engine_send_queue(Engine," movestogo %d",move_nb);
                     }
                 }
         }
         if (XB->depth_limit) engine_send_queue(Engine," depth %d",XB->depth_max);

         if (State->state == PONDER) engine_send_queue(Engine," ponder");

         engine_send(Engine,""); // newline

      } else if (State->state == ANALYSE) {

         engine_send(Engine,"go infinite");

      } else {

         ASSERT(FALSE);
      }

      // init search info

      ASSERT(!Uci->searching);

      search_clear();

      Uci->searching = TRUE;
      Uci->pending_nb++;
   }
}

// search_clear()

static void search_clear() {

   uci_clear(Uci);

   // TODO: MOVE ME

   my_timer_start(State->timer);//also resets
}

// active()

static bool active() {

   // position state

   if (game_status(Game) != PLAYING) return FALSE; // game ended

   // xboard state

   if (XB->analyse) return TRUE; // analysing
   if (!State->computer[White] && !State->computer[Black]) return FALSE; // force mode
   if (XB->new_hack || XB->result) return FALSE; // unstarted or ended game

   return TRUE; // playing
}

// ponder()

static bool ponder() {

    return XB->ponder && (option_get_bool(Option,"CanPonder") ||
                          option_find(Uci->option,"Ponder"));
}
// ponder_ok()

static bool ponder_ok(int move) {
   int status;
   board_t board[1];

   ASSERT(move==MoveNone||move_is_ok(move));

   // legal ponder move?

   if (move == MoveNone) return FALSE;

   game_get_board(Game,board);
   if (!move_is_legal(move,board)) return FALSE;

   // UCI-legal resulting position?

   game_add_move(Game,move);

   game_get_board(Game,board);
   status = game_status(Game);

   game_rem_move(Game);

   if (status != PLAYING) return FALSE; // game ended

   if (option_get_bool(Option,"Book") && is_in_book(board)) {
      return FALSE;
   }

   return TRUE;
}

// stop_search()

static void stop_search() {

   if (Uci->searching) {

      ASSERT(Uci->searching);
      ASSERT(Uci->pending_nb>=1);

      my_log("POLYGLOT STOP SEARCH\n");

/*
      engine_send(Engine,"stop");
      Uci->searching = FALSE;
*/

      if (option_get_bool(Option,"SyncStop")) {
         uci_send_stop_sync(Uci);
      } else {
         uci_send_stop(Uci);
      }
	}
}

// send_board()

static void send_board(int extra_move) {

   char fen[256];
   int start, end;
   board_t board[1];
   int pos;
   int move;
   char string[256];

   ASSERT(extra_move==MoveNone||move_is_ok(extra_move));

   ASSERT(!Uci->searching);

   // init

   game_get_board(Game,Uci->board);
   if (extra_move != MoveNone) move_do(Uci->board,extra_move);

   board_to_fen(Uci->board,fen,256);
   my_log("POLYGLOT FEN %s\n",fen);

   ASSERT(board_can_play(Uci->board));

   // more init

   start = 0;
   end = game_pos(Game);
   ASSERT(end>=start);

   // position

   game_get_board_ex(Game,board,start);
   board_to_fen(board,string,256);

   engine_send_queue(Engine,"position");

   if (my_string_equal(string,StartFen)) {
      engine_send_queue(Engine," startpos");
   } else {
      engine_send_queue(Engine," fen %s",string);
   }

   // move list

   if (end > start || extra_move != MoveNone) engine_send_queue(Engine," moves");

   for (pos = start; pos < end; pos++) { // game moves

      move = game_move(Game,pos);

      move_to_can(move,board,string,256);
      engine_send_queue(Engine," %s",string);

      move_do(board,move);
   }

   if (extra_move != MoveNone) { // move to ponder on
      move_to_can(extra_move,board,string,256);
      engine_send_queue(Engine," %s",string);
   }

   // end

   engine_send(Engine,""); // newline
}

// send_info()

static void send_info() {
    int min_depth;
    if(option_get_bool(Option,"WbWorkArounds2")){
            // Silly bug in some versions of WinBoard.
            // depth <=1 clears the engine output window.
            // Why shouldn't an engine be allowed to send info at depth 1?
        min_depth=2;
    }else{
        min_depth=1;
    }
	 
    gui_send(GUI,"%d %+d %.0f "S64_FORMAT" %s",Uci->best_depth>min_depth?Uci->best_depth:min_depth,
	     0,0.0,0,Uci->info);  
}

// send_pv()

static void send_pv() {

   char pv_string[StringSize];
   board_t board[1];
   int move;
   char move_string[StringSize];

   ASSERT(State->state!=WAIT);

   if (Uci->best_depth == 0) return;

   // xboard search information

   if (XB->post) {

      if (State->state == THINK || State->state == ANALYSE) {

         line_to_san(Uci->best_pv,Uci->board,pv_string,StringSize);

		 if(Uci->depth==-1) //hack to clear the engine output window
             gui_send(GUI,"%d %+d %.0f "S64_FORMAT" ",0,report_best_score(),Uci->time*100.0,Uci->node_nb);

		 gui_send(GUI,"%d %+d %.0f "S64_FORMAT" %s",Uci->best_depth,report_best_score(),Uci->time*100.0,Uci->node_nb,pv_string);

      } else if (State->state == PONDER &&
                 option_get_bool(Option,"ShowPonder")) {

         game_get_board(Game,board);
         move = State->exp_move;

         if (move != MoveNone && move_is_legal(move,board)) {
            move_to_san(move,board,move_string,256);
            line_to_san(Uci->best_pv,Uci->board,pv_string,StringSize);
            gui_send(GUI,"%d %+d %.0f "S64_FORMAT" (%s) %s",Uci->best_depth,report_best_score(),Uci->time*100.0,Uci->node_nb,move_string,pv_string);
         }
      }
   }

   // kibitz

   if ((Uci->searching &&
        option_get_bool(Option,"KibitzPV") &&
        Uci->time >= option_get_double(Option,"KibitzDelay"))
       || (!Uci->searching && option_get_bool(Option,"KibitzMove"))) {

      if (State->state == THINK || State->state == ANALYSE) {

         line_to_san(Uci->best_pv,Uci->board,pv_string,StringSize);
         if(kibitz_throttle(Uci->searching)){
             gui_send(GUI,"%s depth=%d time=%.2f node="S64_FORMAT" speed=%.0f score=%+.2f pv=\"%s\"",option_get_string(Option,"KibitzCommand"),Uci->best_depth,Uci->time,Uci->node_nb,Uci->speed,((double)report_best_score())/100.0,pv_string);
         }
      } else if (State->state == PONDER) {

         game_get_board(Game,board);
         move = State->exp_move;

         if (move != MoveNone && move_is_legal(move,board)) {
            move_to_san(move,board,move_string,256);
            line_to_san(Uci->best_pv,Uci->board,pv_string,StringSize);
            if(kibitz_throttle(Uci->searching)){
                gui_send(GUI,"%s depth=%d time=%.2f node="S64_FORMAT" speed=%.0f score=%+.2f pv=\"(%s) %s\"",option_get_string(Option,"KibitzCommand"),Uci->best_depth,Uci->time,Uci->node_nb,Uci->speed,((double)report_best_score())/100.0,move_string,pv_string);
            }
         }
      }
   }
}

// kibitz_throttle()

static bool kibitz_throttle(bool searching){
    time_t curr_time;
    static time_t lastKibitzMove=0;
    static time_t lastKibitzPV=0;
    curr_time = time(NULL);
    if(searching){   // KibitzPV
        if(curr_time >=
           (option_get_int(Option,"KibitzInterval") + lastKibitzPV)){
            lastKibitzPV=curr_time;
            return TRUE;
        }
    }else{       // KibitzMove
        if(curr_time >=
           (option_get_int(Option,"KibitzInterval") + lastKibitzMove)){
            lastKibitzPV=curr_time;
            lastKibitzMove=curr_time;
            return TRUE;
        }        
    }
    return FALSE;
}

// learn()

static void learn(int result) {

   int pos;
   board_t board[1];
   int move;

   ASSERT(result>=-1&&result<=+1);

   ASSERT(XB->result);
   ASSERT(State->computer[White]||State->computer[Black]);

   // init

   pos = 0;

   if (FALSE) {
   } else if (State->computer[White]) {
      pos = 0;
   } else if (State->computer[Black]) {
      pos = 1;
      result = -result;
   } else {
      my_fatal("learn(): unknown side\n");
   }

   if (FALSE) {
   } else if (result > 0) {
      my_log("POLYGLOT *LEARN WIN*\n");
   } else if (result < 0) {
      my_log("POLYGLOT *LEARN LOSS*\n");
   } else {
      my_log("POLYGLOT *LEARN DRAW*\n");
   }

   // loop

   for (; pos < Game->size; pos += 2) {

      game_get_board_ex(Game,board,pos);
      move = game_move(Game,pos);

      book_learn_move(board,move,result);
   }

   book_flush();
}

// end of xboard2uci.c
