#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define array_size(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))
#define FIXED_ROWS 2
#define STAGE_HEIGHT 20
#define LANE 5
#define MAX_WORD_LENGTH 10

void end_game(void);
void initialize(void);
void* run_game(void *);
void remove_element(int *, int, int);
void remove_element_by_value(int *, int, int*);
int random_number(int, int);

struct item {
	char title[MAX_WORD_LENGTH];
	int lane;
	int pos;
	int prev_pos;
	int wait;
};

char* words[] = {"tutorials", "codings","father", "mother","family","nonsense","rubbish","skills","runner","overcome","dream","company", "winner","program", "ultimate", "greatest","loving","deadline","nightmare", "ongoing","designer", "framework","plumber","doctor","baby", "singing"};
pthread_t t_game;
struct item items[LANE];
WINDOW *BOARD[LANE];
int life=10, score=0, sleep_game = 1000000;
int matches[LANE];
int total_matches = 0;

void main() {
	
	int i;
	char input[MAX_WORD_LENGTH];
	initialize();
	
	do {
		noecho();
		
		echo();
		mvprintw(STAGE_HEIGHT + 4 , 0, "Type a word and enter to match (q to exit). ");
		curs_set( 0 );
		scanw("%s", &input);
		
		if (strcmp(input, "q")== 0 || strcmp(input, "Q")== 0) {
			break;
		} else {
			for (i = 0; i < LANE; i++) {
				
				sprintf(input,"%-9s", input);
				if (strcmp(items[i].title, input) == 0) {
					matches[total_matches] = items[i].lane;
					total_matches++;
					
				}
				
			}
		}
		
		move(STAGE_HEIGHT + 4, 0); 
		clrtoeol();//clear line
		
    } while (1);
	
	end_game();
	exit(0);
	
}

int random_number(int min_num, int max_num)
{
    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num)
    {
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    } else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }

    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

void remove_element(int *array, int index, int array_length) {
   int i;
   for(i = index; i < array_length; i++) array[i] = array[i + 1];
}

void remove_element_by_value(int *array, int value, int *array_length) {
	int i;
	int length = *array_length;
	for(i = 0; i < length; i++) {
	   
		if (array[i] == value) {
			remove_element(array, i, *array_length);
			*array_length -= 1;
		}
	}
}

void *run_game(void* ptr) {
	
	int i,j,startx,starty;
	starty = 0, startx=0;
	char item[MAX_WORD_LENGTH];
	int total_words = array_size(words);
	
	srand(time(NULL));
	
	//draw lane
	for (i = 0; i < LANE; i++) {
		
        BOARD[i] = newwin(STAGE_HEIGHT, MAX_WORD_LENGTH + 2, starty, startx);
		startx += MAX_WORD_LENGTH+2;
		box(BOARD[i], '*', ' ');
		wrefresh(BOARD[i]);
	}
	
	BOARD[i+1] = newwin(3, (MAX_WORD_LENGTH+2) * LANE, STAGE_HEIGHT, 0);
	box(BOARD[i+1], '|', '-');
	wrefresh(BOARD[i+1]);
	
	while(1) { 
		if (life > 0) {
				
			
			for (i = 0; i < LANE; i++) {
				wattroff( BOARD[i], A_STANDOUT );
				mvwprintw( BOARD[i], items[i].prev_pos, 1, "%-10s", "" );

				if (items[i].wait <= 0) {
					int has_match = 0;
					
					for(j=0;j<total_matches;j++) {
						if (matches[j] == i) {
							has_match = 1;
							remove_element(matches, j, total_matches);
							total_matches--;
							break;
						}
					}

					wattron( BOARD[i], A_STANDOUT );
					sprintf(item, "%-10s",  items[i].title);
					mvwprintw( BOARD[i], items[i].pos, 1, "%s", item );
					wattroff( BOARD[i], A_STANDOUT );

					
					if (items[i].pos >= STAGE_HEIGHT - 1) {
						life--;
						items[i].wait = random_number(0,9);
						int rand = random_number(0, total_words-1);
						sprintf(items[i].title,"%-9s", words[rand]);
						items[i].prev_pos = STAGE_HEIGHT - 1;
						items[i].pos = 0;
					} else if (has_match) {
						score++;
						items[i].wait = random_number(0,9);
						int rand = random_number(0, total_words-1);
						sprintf(items[i].title,"%-9s", words[rand]);
						items[i].prev_pos = items[i].pos;
						items[i].pos = 0;
					} else {
						items[i].prev_pos = items[i].pos;
						items[i].pos++;
						
					}
				} else {
					items[i].wait--;
				}
				
				wrefresh(BOARD[i]);
			}
			
			//draw dashboard
			int level = floor(score/10);
			char dashboard[50];
			sprintf(dashboard, "LIFE: %d, SCORE: %d, LEVEL: %d       ", life, score,level + 1);
			
			mvwprintw( BOARD[i+1], 1, 2, "%s", dashboard );
			wrefresh(BOARD[i+1]);
			
			//game sleep
			int minus_time = (level * 50000);
			if (minus_time > 900000) {
				minus_time = 900000;
			}
			
			usleep(sleep_game - minus_time);
		} else {
			int board_id = floor(LANE / 2);
			mvwprintw( BOARD[board_id], STAGE_HEIGHT/ 2, 1, "%-10s", "GAME OVER!" );
			wrefresh(BOARD[board_id]);
			sleep(1);
		}
	}
}

void end_game(void) {
	int i;
	
	pthread_kill(t_game, SIGUSR1);
	
    //erase every box and delete each window
    for (i = 0; i <= LANE; i++) {
        wborder(BOARD[i], ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        wrefresh(BOARD[i]);
        delwin(BOARD[i]);
    }
	endwin();
}

void initialize(void) {
	int i;
	initscr();     // initialize Ncurses
	noecho();      // disable echoing of characters on the screen
	curs_set( 0 ); // hide the default screen cursor.
	cbreak();
	
	clear();
	mvprintw(LINES - 1, (COLS - 5) / 2, "Simple Typing Tutor Game");
    refresh();	
	
	srand(time(NULL));
	
	if ((LINES < 24) || (COLS < 80)) {
        endwin();
        puts("Your terminal needs to be at least 80x24");
        exit(2);
    }
		
	if(pthread_create(&t_game, NULL, run_game, NULL)!=0) {
		fprintf(stderr, "Error creating thread\n");
		exit(2);
	}
	
	int total_words = array_size(words);
	for(i=0;i<LANE;i++) {
		items[i].lane = i;
		items[i].wait = random_number(0, 5);
		items[i].pos = 0;
		items[i].prev_pos = 0-1;
		
		int rand = random_number(0, total_words-1);
		sprintf(items[i].title,"%-9s", words[rand]);
		
	}
	
}
