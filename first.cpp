#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "PPCD.cpp" // plik sluzacy do lepszego wykrywania kolizji obiektow. Zostal on sciagniety z Internetu

int FPS;
int SCREEN_W;
int SCREEN_H;
int COLUMN_W;
int COLUMN_H;
int COLUMNS_QUANTITY;
int COLUMN_VISIBLE_MIN;
float COLUMN_SPEED;
float GAP_SCALE;
float HERO_A;
float HERO_J;
int LAYERS_QUANTITY;

// DISPLAY is global because it's needed in some functions to draw backgrounds
ALLEGRO_DISPLAY *DISPLAY;

struct sprite {
	int width;
	int height;
	float x;
	float y;
	float a;
	float v;
	float jump;
	mask_t *mask;
} hero;

struct column {
	float x;
	float y_down;
	float y_up;
	float gap;
	bool visited;
};

#include "menu.h"
#include "game_over.h"

int main(int argc, char **argv) {
	
    // ===== ALLEGRO INIT FUNCTIONS =====
	al_init();
	al_init_image_addon();
	al_install_mouse();
	al_init_font_addon();
	al_init_ttf_addon();
	al_install_keyboard();
	// ==================================
	
	// ======================= CONFIGURATION FILE ============================
	ALLEGRO_CONFIG* cfg = al_load_config_file("config.cfg");
	
	FPS = atoi(al_get_config_value(cfg, "", "FPS"));
	SCREEN_W = atoi(al_get_config_value(cfg, "", "SCREEN_W"));
    SCREEN_H = atoi(al_get_config_value(cfg, "", "SCREEN_H"));
    COLUMN_W = atoi(al_get_config_value(cfg, "", "COLUMN_W"));
    COLUMN_H = atoi(al_get_config_value(cfg, "", "COLUMN_H"));
    COLUMNS_QUANTITY = atoi(al_get_config_value(cfg, "", "COLUMNS_QUANTITY"));
    LAYERS_QUANTITY = atoi(al_get_config_value(cfg, "", "LAYERS_QUANTITY"));
    
    const char *hero_name_temp = al_get_config_value(cfg, "", "hero_name");
    char hero_name[20];
    strncpy(hero_name, hero_name_temp, 20);
    
    al_destroy_config(cfg);
	// =======================================================================
	
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;
	ALLEGRO_BITMAP *hero_img, *column_img_up, *column_img_down, *background_layers[LAYERS_QUANTITY], *background, *loading;
	ALLEGRO_FONT *font = al_load_ttf_font("m04.TTF", 72, 0), *small_font = al_load_ttf_font("m04.TTF", 15, 0);
	
	// ============= HERO SIZE =============
	hero_img = al_load_bitmap(hero_name);
	hero.width = al_get_bitmap_width(hero_img);
	hero.height = al_get_bitmap_height(hero_img);
	// =====================================
	
	timer = al_create_timer(1.0 / FPS);
	
	DISPLAY = al_create_display(SCREEN_W, SCREEN_H);
	al_set_target_bitmap(al_get_backbuffer(DISPLAY));

    // ======================== EVENT QUEUE ========================
	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_display_event_source(DISPLAY));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_mouse_event_source());
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	// =============================================================
	
	
	background = al_load_bitmap("background.png");
	loading = al_load_bitmap("loading.png");
	al_draw_bitmap(loading, 0, 0, 0);
	al_flip_display();
	
	// ===================== PARALLAX BACKGROUND =====================
	char layer_name[30];
	
	for (int i = 0; i < LAYERS_QUANTITY; i++) {
		sprintf(layer_name, "layer_%c.png", '0' + i + 1);
		background_layers[i] = al_load_bitmap(layer_name);
	}

	float layer_x[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    float layer_dx[8] = {0, -0.7, -0.5, -0.3, -0.25, -0.2, -0.1, 0};
    int layer_w = al_get_bitmap_width(background_layers[0]);
	// ===============================================================
	
	column_img_down = al_load_bitmap("column_down.png");
	column_img_up = al_load_bitmap("column_up.png");
	
	// ================= COLLISIONS =================
	mask_t *column_up_mask, *column_down_mask;
	hero.mask = Mask_New(hero_img); //hero.mask already declared in struct
	column_up_mask = Mask_New(column_img_up);
	column_down_mask = Mask_New(column_img_down);
	int hero_center_x, hero_center_y;
	int column_center_x, column_center_y_up, column_center_y_down;
	// ==============================================
	
	al_start_timer(timer);
	
	column columns[COLUMNS_QUANTITY];
    
	// =======================
	bool redraw = true;
	bool you_lost = false;
	bool game_paused = true;
	bool display_menu = true;
	bool exit = false;
	// =======================
	
	// =========== SCORE ===========
	int score = 0;
	char score_str[20];
    sprintf(score_str, "%d", score);
    // =============================
   
    
	while (1) {
		ALLEGRO_EVENT ev;
		
	    if (display_menu) {
			menu(event_queue, background, &exit);
			new_game(&hero, columns, &score, &layer_dx[0]);
			display_menu = false;
			redraw = true;
		}
		
	    if (exit) break;
		al_wait_for_event(event_queue, &ev);

		switch(ev.type) {
		    case ALLEGRO_EVENT_TIMER:
				hero.v += hero.a;
				hero.y += hero.v;
				hero_center_x = (int)(hero.x + 0.5 * hero.width);
				hero_center_y = (int)(hero.y + 0.5 * hero.height);
				
				for (int i = 0; i < COLUMNS_QUANTITY; ++i) {
					columns[i].x += COLUMN_SPEED;
					if (columns[i].visited == false && columns[i].x <= SCREEN_W / 2 - 0.5 * COLUMN_W) {
						score++; 
						sprintf(score_str, "%d", score);
						columns[i].visited = true;
					}
					else if (columns[i].x < -COLUMN_W) {
						srand(time(NULL));
						columns[i].x =  columns[(i + COLUMNS_QUANTITY - 1) % COLUMNS_QUANTITY].x + 200 + rand() % (int)(hero.width);
						columns[i].gap = GAP_SCALE * (hero.height + hero.width) / 2 + 0.5 * (rand() % hero.height);
						columns[i].y_down = SCREEN_H - COLUMN_VISIBLE_MIN - (rand() % (int)(SCREEN_H - 2 * COLUMN_VISIBLE_MIN - columns[i].gap));
						columns[i].y_up = columns[i].y_down - columns[i].gap - COLUMN_H;
						columns[i].visited = false;
					}
					column_center_x = (int)(columns[i].x + 0.5 * COLUMN_W);
					column_center_y_up = (int)(columns[i].y_up + 0.5 * COLUMN_H);
					column_center_y_down = (int)(columns[i].y_down + 0.5 * COLUMN_H);
					
					// detect collisions with columns
					if (Mask_Collide(hero.mask, column_down_mask, hero_center_x - column_center_x, hero_center_y - column_center_y_down)) {you_lost = true;}
					if (Mask_Collide(hero.mask, column_up_mask, hero_center_x - column_center_x, hero_center_y - column_center_y_up)) {you_lost = true;}
				}
				// detect collisions with window
				if (hero.y < 0 || hero.y >= SCREEN_H - hero.height) {you_lost = true;}
				
				redraw = true;
		        break;

		    case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		        hero.v = -hero.jump;
		        break;
		        
		    case ALLEGRO_EVENT_KEY_DOWN:
		        if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {hero.v = -hero.jump;}
		        else if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {game_paused = true;}
		        break;
		}
		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) break;
		
		if (redraw && al_is_event_queue_empty(event_queue)) {
			redraw = false;
			
			// ========== DRAW PARALLAX BACKGROUND ==========
			for (int i = LAYERS_QUANTITY - 1; i >= 0; i--) {
			    layer_x[i] += layer_dx[i];
			    if (layer_x[i] <= -layer_w / 2) {layer_x[i] = 0;}
			    al_draw_bitmap(background_layers[i], layer_x[i], 0, 0);
			}
            // ==============================================
            
            // ================ DRAW COLLUMNS ================
			for (int i = 0; i < COLUMNS_QUANTITY; ++i) {
				if (columns[i].x >= -COLUMN_W && columns[i].x <= SCREEN_W) {
				    al_draw_bitmap(column_img_down, columns[i].x, columns[i].y_down, 0);
				    al_draw_bitmap(column_img_up, columns[i].x, columns[i].y_up, 0);
			    }
			}
			// ===============================================
			
			al_draw_text(font, al_map_rgb(255,255,255), SCREEN_W / 2, SCREEN_H / 5, ALLEGRO_ALIGN_CENTRE, score_str);
			al_draw_bitmap(hero_img, hero.x, hero.y, 0);
			al_flip_display();
		}
		if (game_paused) {
			al_draw_text(small_font, al_map_rgb(255,255,255), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTRE, "Press SPACE or click to continue");
			al_flip_display();
		}
		while (game_paused) {
			al_wait_for_event(event_queue, &ev);
			if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
			    if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE || ev.keyboard.keycode == ALLEGRO_KEY_SPACE) 
			        {game_paused = false; hero.v = -hero.jump;}
			if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) 
			    {game_paused = false;	hero.v = -hero.jump;}
		}
		if (you_lost) {
			ALLEGRO_BITMAP *bloody_background = NULL;
			al_draw_bitmap(background, 0, 0, 0);
			
			for (int i = 0; i < COLUMNS_QUANTITY; ++i) {
				if (columns[i].x >= -COLUMN_W && columns[i].x <= SCREEN_W) {
				    al_draw_bitmap(column_img_down, columns[i].x, columns[i].y_down, 0);
				    al_draw_bitmap(column_img_up, columns[i].x, columns[i].y_up, 0);
			    }
			}
			
			al_draw_bitmap(hero_img, hero.x, hero.y, 0);
			al_draw_text(font, al_map_rgb(255,255,255), SCREEN_W / 2, SCREEN_H / 5, ALLEGRO_ALIGN_CENTRE, "YOU LOST");
			
			bloody_background = al_create_bitmap(SCREEN_W, SCREEN_H);
			al_set_target_bitmap(bloody_background);
			al_clear_to_color(al_map_rgba(255, 0, 0, 127));
			al_set_target_bitmap(al_get_backbuffer(DISPLAY));
			al_draw_bitmap(bloody_background, 0, 0, 0);
			
			al_flip_display();

			// check score and update values
			game_over(&score, &display_menu, &game_paused, &you_lost, &redraw);
			// prepare for another game
			new_game(&hero, columns, &score, &layer_dx[0]);
			al_flush_event_queue(event_queue);
			sprintf(score_str, "%d", score);
	    }
	}
	
	Mask_Delete(column_up_mask);
	Mask_Delete(column_down_mask);
	Mask_Delete(hero.mask);
	
	al_destroy_timer(timer);
	al_destroy_bitmap(hero_img);
	al_destroy_bitmap(column_img_up);
	al_destroy_bitmap(column_img_down);
	al_destroy_bitmap(background);
	al_destroy_display(DISPLAY);
	al_destroy_event_queue(event_queue);
	
	return 0;
}

/* Command used to compile the game:
 *  g++ -Wall first.cpp -o first `pkg-config --cflags --libs allegro-5.0 allegro_acodec-5.0 allegro_audio-5.0 allegro_color-5.0 allegro_dialog-5.0 allegro_font-5.0 allegro_image-5.0 allegro_main-5.0 allegro_memfile-5.0 allegro_physfs-5.0 allegro_primitives-5.0 allegro_ttf-5.0`
*/
