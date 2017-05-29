#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>

#include "character.h"
#include "ui.h"
#include "util.h"

ui_t *ui;

//Draw entire map
void draw_map(uint8_t map[]){
	attron(COLOR_PAIR(COLOR_DEFAULT));
	int i, j;
	for(i = 0; i < GAME_HEIGHT; i++){
		for (j = 0; j < GAME_WIDTH; j++){
			move(2+i, 1+j);
			uint8_t val = map[i*GAME_WIDTH+j];
			if(val == 0){
				addch(FLOOR);
			}else if (val == 1){
				addch(HALF_COVER);
			}else if(val == 2){
				addch(FULL_COVER);
			}else{
				addch(val);	
			}
		}
	}
	attroff(COLOR_PAIR(COLOR_DEFAULT));
}

void draw_player(character_t *pc){
	attron(COLOR_PAIR(pc->color));
	mvaddch(pc->y+2, pc->x+1, pc->c);
	attroff(COLOR_PAIR(pc->color));
}

void draw_player_move_rec(uint8_t *map, uint8_t *done, int initialD, int dist, int y, int x){
	move(y+2, x+1);
	uint8_t val = map[yx_to_index(y, x)];
	if(val == 0){
		addch(FLOOR);
	}else if (val == 1){
		addch(HALF_COVER);
	}else if(val == 2){
		addch(FULL_COVER);
	}else{
		addch(val);	
	}
	done[yx_to_index(y+initialD,x+initialD)] = dist;
	if(dist > 0){
		if(y-1 >= 0 && dist > done[yx_to_index(y-1, x)] && !map[yx_to_index(y-1, x)]){
			draw_player_move_rec(map, done, initialD, dist-1, y-1, x);
		}
		if(y+1 < GAME_HEIGHT && dist > done[yx_to_index(y+1, x)] && !map[yx_to_index(y+1, x)]){
			draw_player_move_rec(map, done, initialD, dist-1, y+1, x);
		}
		if(x-1 >= 0 && dist > done[yx_to_index(y, x-1)] && !map[yx_to_index(y, x-1)]){
			draw_player_move_rec(map, done, initialD, dist-1, y, x-1);
		}
		if(x+1 < GAME_WIDTH && dist > done[yx_to_index(y, x+1)] && !map[yx_to_index(y, x+1)]){
			draw_player_move_rec(map, done, initialD, dist-1, y, x+1);
		}
	}
}

void draw_player_move(uint8_t *map, character_t *player){
	int distance = 5;
	uint8_t done[GAME_HEIGHT * GAME_WIDTH];
	int i;
	int pX = ui->model->player->x;
	int pY = ui->model->player->y;
	
	for(i = 0; i < GAME_HEIGHT * GAME_WIDTH; i++){
		done[i] = 0;
	}
	attron(COLOR_PAIR(COLOR_MOVE));
	draw_player_move_rec(map, done, distance, distance, pY, pX);
	attroff(COLOR_PAIR(COLOR_MOVE));
	
}

void ui_draw(){
	attron(COLOR_PAIR(COLOR_DEFAULT));
	int i, j;
	for(i = 1; i < GAME_HEIGHT+1; i++){
		mvaddch(i+1, 0, ACS_VLINE);
		mvaddch(i+1, GAME_WIDTH+1, ACS_VLINE);
	}
	for(j = 1; j < GAME_WIDTH+1; j++){
		mvaddch(0+1, j, ACS_HLINE);
		mvaddch(GAME_HEIGHT+1+1, j, ACS_HLINE);
	}
	mvaddch(0+1,0, ACS_ULCORNER);
	mvaddch(0+1,GAME_WIDTH+1, ACS_URCORNER);
	mvaddch(GAME_HEIGHT+1+1,0, ACS_LLCORNER);
	mvaddch(GAME_HEIGHT+1+1,GAME_WIDTH+1, ACS_LRCORNER);
	
	attroff(COLOR_PAIR(COLOR_DEFAULT));
	draw_map(ui->model->map);
	draw_player_move(ui->model->map, ui->model->player);
	draw_player(ui->model->player);
}

void ui_message(const char* fmt, ...){
	va_list args;
	va_start(args, fmt);
	mvprintw(0, 0, LINE_CLEAR);
	mvprintw(0, 0, fmt, args);
	va_end(args);
}

void ui_prompt(char* input, const char* fmt, ...){
	int textKey;
	int index = 0;
	int escape = 0;
	curs_set(1);
	va_list args;
	va_start(args, fmt);
	mvprintw(0, 0, LINE_CLEAR);
	mvprintw(0, 0, fmt, args);
	va_end(args);
	textKey = getch();
	while(textKey != '\n'){
		if(is_path_char(textKey)){
			addch(textKey);
			input[index] = textKey;
			index++;
			input[index] = '\0';
		}else if(textKey == KEY_BACKSPACE){
			index--;
			input[index] = '\0';
			mvprintw(0, 0, LINE_CLEAR);
			mvprintw(0, 0, "Open: %s", input);
		}else if(textKey == 27){
			escape = 1;
			curs_set(0);
			input[0] = '\0';
			break;
		}
		textKey = getch();
	}
	if(!escape){
		if(index == 0){
			input[0] = '\0';
		}else{
			input[index] = '\0';
		}
	}
	curs_set(0);
}

ui_t *init_ui(const model_t *model){
	ui = malloc(sizeof(ui_t));
	ui->model = model;
	
	ui->draw = ui_draw;
	ui->prompt = ui_prompt;
	ui->message = ui_message;
	return ui;
}

void free_ui(ui_t* ui){
	free(ui);
}

