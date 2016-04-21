void new_game(struct sprite* hero, struct column columns[], int *score, float *dx) {
	
	ALLEGRO_CONFIG* cfg = al_load_config_file("config.cfg");
	const char *difficulty = al_get_config_value(cfg, "", "difficulty");
    COLUMN_VISIBLE_MIN = atoi(al_get_config_value(cfg, difficulty, "COLUMN_VISIBLE_MIN"));
    GAP_SCALE = atof(al_get_config_value(cfg, difficulty, "GAP_SCALE"));
    HERO_A = atof(al_get_config_value(cfg, difficulty, "HERO_A"));
    HERO_J = atof(al_get_config_value(cfg, difficulty, "HERO_J"));
    COLUMN_SPEED = atof(al_get_config_value(cfg, difficulty, "COLUMN_SPEED"));
    al_destroy_config(cfg);
    
    hero->x = SCREEN_W / 2.0 - hero->width / 2.0;
	hero->y = SCREEN_H / 2.0 - hero->height / 2.0;
	hero->a = HERO_A;
	hero->v = 0;
	hero->jump = HERO_J;
	for (int i = 0; i < COLUMNS_QUANTITY; ++i) {
		srand(time(NULL) + 100 * i);
		if (!i) columns[i].x = 1.5 * SCREEN_W;
		else columns[i].x = columns[i - 1].x + 200 + rand() % (int)(hero->width);
		columns[i].gap = GAP_SCALE * (hero->height + hero->width) / 2 + 0.5 * (rand() % hero->height);
		columns[i].y_down = SCREEN_H - COLUMN_VISIBLE_MIN - (rand() % (int)(SCREEN_H - 2 * COLUMN_VISIBLE_MIN - columns[i].gap));
		columns[i].y_up = columns[i].y_down - columns[i].gap - COLUMN_H;
		columns[i].visited = false;
	}
	*score = 0;
	*dx = COLUMN_SPEED - 0.5;
}

void show_tutorial(void) {
	ALLEGRO_BITMAP *tutorial, *white_background;
	ALLEGRO_EVENT_QUEUE *simple_queue = NULL;
	
	white_background = al_create_bitmap(SCREEN_W, SCREEN_H);
    al_set_target_bitmap(white_background);
	al_clear_to_color(al_map_rgba(255, 255, 255, 200));
	al_set_target_bitmap(al_get_backbuffer(DISPLAY));
	al_draw_bitmap(white_background, 0, 0, 0);
	
	tutorial = al_load_bitmap("tutorial.png");
    al_draw_bitmap(tutorial, SCREEN_W / 2 - al_get_bitmap_width(tutorial) / 2, SCREEN_H / 2 - al_get_bitmap_height(tutorial) / 2, 0);
	al_flip_display();
	
	simple_queue = al_create_event_queue();
	al_register_event_source(simple_queue, al_get_mouse_event_source());
	al_register_event_source(simple_queue, al_get_keyboard_event_source());
	
	while (1) {
		ALLEGRO_EVENT ev;
	    al_wait_for_event(simple_queue, &ev);
	    if (ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
	        break;
    }
    al_destroy_bitmap(tutorial);
    al_destroy_bitmap(white_background);
}

void show_high_scores(void) {
	ALLEGRO_FONT *high_scores_font = al_load_ttf_font("retro_font.TTF", 20, 0);
	ALLEGRO_COLOR text_color;
	ALLEGRO_BITMAP *text_background;
	ALLEGRO_EVENT_QUEUE *simple_queue = NULL;
	
	text_background = al_create_bitmap(400, 440);
    al_set_target_bitmap(text_background);
	al_clear_to_color(al_map_rgba(62, 210, 27, 255));
	al_set_target_bitmap(al_get_backbuffer(DISPLAY));
	al_draw_bitmap(text_background, SCREEN_W / 2 - 200, SCREEN_H / 2 - 220, 0);
	
    struct winner {
	    char nickname[20];
	    char result[5];
	};
	winner list_of_winners[10];
	FILE *winners = fopen("winners.txt", "r");
	for (int i = 0; i < 10; i++) {
	    fscanf(winners, "%s", list_of_winners[i].nickname);
	    fscanf(winners, "%s", list_of_winners[i].result);
	}
	fclose(winners);
	for (int i = 0; i < 10; i++) {
		switch (i) {
			// gold
		    case 0:
		        text_color = al_map_rgb(249, 199, 0);
		        break;
		    // silver
		    case 1:
		        text_color = al_map_rgb(225, 224, 209);
		        break;
		    // bronze
		    case 2:
		        text_color = al_map_rgb(111, 31, 0);
		        break;
		    default:
		        text_color = al_map_rgb(0, 0, 0);
		}
	    al_draw_text(high_scores_font, text_color, SCREEN_W / 2 - 180, SCREEN_H / 2 - 220 + 40 * (i + 1), ALLEGRO_ALIGN_LEFT, list_of_winners[i].nickname);
	    al_draw_text(high_scores_font, text_color, SCREEN_W / 2 + 180, SCREEN_H / 2 - 220 + 40 * (i + 1), ALLEGRO_ALIGN_RIGHT, list_of_winners[i].result);
	}
	al_flip_display();
	
	simple_queue = al_create_event_queue();
	al_register_event_source(simple_queue, al_get_mouse_event_source());
	al_register_event_source(simple_queue, al_get_keyboard_event_source());
	
	while (1) {
		ALLEGRO_EVENT ev;
	    al_wait_for_event(simple_queue, &ev);
	    if (ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
	        break;
    }
}

void choose_difficulty(void) {
	ALLEGRO_FONT *difficulty_font = al_load_ttf_font("retro_font.TTF", 30, 0);
	ALLEGRO_COLOR text_color;
	ALLEGRO_BITMAP *text_background;
	ALLEGRO_EVENT_QUEUE *simple_queue = NULL;
	
	text_background = al_create_bitmap(400, 440);
    al_set_target_bitmap(text_background);
	al_clear_to_color(al_map_rgba(62, 210, 27, 255));
	al_set_target_bitmap(al_get_backbuffer(DISPLAY));
	al_draw_bitmap(text_background, SCREEN_W / 2 - 200, SCREEN_H / 2 - 220, 0);
	
	int number_of_difficulties = 3;
	const char *difficulties[] = {"easy", "medium", "hard"};
	
	simple_queue = al_create_event_queue();
	al_register_event_source(simple_queue, al_get_mouse_event_source());
	al_register_event_source(simple_queue, al_get_keyboard_event_source());
	
	int chosen_index;
	bool redraw = true;
	
	ALLEGRO_CONFIG* cfg = al_load_config_file("config.cfg");
	const char *old_difficulty = al_get_config_value(cfg, "", "difficulty");
    for (int i = 0; i < 3; i++) {
		if (strcmp(difficulties[i], old_difficulty) == 0) {
            chosen_index = i;
		}
	}
    al_destroy_config(cfg);

	while (1) {
		ALLEGRO_EVENT ev;
		al_draw_bitmap(text_background, SCREEN_W / 2 - 200, SCREEN_H / 2 - 220, 0);
	    for (int i = 0; i < 3; i++) {
		    if (i == chosen_index) {
				text_color = al_map_rgb(255, 255, 255);
				chosen_index = i;
			}
		    else text_color = al_map_rgb(0, 0, 0);
	        al_draw_text(difficulty_font, text_color, SCREEN_W / 2, (SCREEN_H * (i + 1)) / 4, ALLEGRO_ALIGN_CENTER, difficulties[i]);
	    }
	    if (redraw) al_flip_display();
		redraw = false;
	    al_wait_for_event(simple_queue, &ev);
	    if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
	        if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
				cfg = al_load_config_file("config.cfg");
				al_set_config_value(cfg, "", "difficulty", difficulties[chosen_index]);
				al_save_config_file("config.cfg", cfg);
				al_destroy_config(cfg);
				break;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
			    chosen_index = (chosen_index + 1) % number_of_difficulties;
			    redraw = true;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_UP) {
			    chosen_index = (chosen_index + number_of_difficulties - 1) % number_of_difficulties;
			    redraw = true;
			}
	    }
    }
}

void menu(ALLEGRO_EVENT_QUEUE *event_queue, ALLEGRO_BITMAP *background, bool *exit) {
	ALLEGRO_FONT *small_font = al_load_ttf_font("retro_font.TTF", 30, 0);
	enum {MENU_START, MENU_DIFFICULTY, MENU_HELP, MENU_HIGH_SCORES, MENU_EXIT};
	int menu_active = 0;
	const char *menu_options_str[] = {"START", "DIFFICULTY LEVEL", "HELP", "HIGH SCORES", "EXIT"};
	bool redraw = true;
	bool leave_menu = false;
    while (!leave_menu) {
		ALLEGRO_EVENT ev;
		// MENU_EXIT is always the last option
		al_wait_for_event(event_queue, &ev);
		if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
				switch (menu_active) {
				    case MENU_START:
				        leave_menu = true;
						break;
					case MENU_HELP:
					    show_tutorial();
					    al_flush_event_queue(event_queue);
						break;
					case MENU_HIGH_SCORES:
					    show_high_scores();
					    al_flush_event_queue(event_queue);
						break;
					case MENU_DIFFICULTY:
					    choose_difficulty();
					    al_flush_event_queue(event_queue);
						break;
					case MENU_EXIT:
						*exit = true;
						leave_menu = true;
						break;
				}
				redraw = true;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
			    menu_active = (menu_active + 1) % (MENU_EXIT + 1);
			    redraw = true;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_UP) {
			    menu_active = (menu_active + MENU_EXIT) % (MENU_EXIT + 1);
			    redraw = true;
			}
		}
		al_draw_bitmap(background, 0, 0, 0);
		for (int i = 0; i <= MENU_EXIT; i++){
			if (i == menu_active) {
			    al_draw_text(small_font, al_map_rgb(0, 255, 0), SCREEN_W / 2, 50 * (i + 1), ALLEGRO_ALIGN_CENTRE, menu_options_str[menu_active]);
			}
			else {
			    al_draw_text(small_font, al_map_rgb(0, 0, 0), SCREEN_W / 2, 50 * (i + 1), ALLEGRO_ALIGN_CENTRE, menu_options_str[i]);
			}
		}
		if (redraw) al_flip_display();
		redraw = false;
	}
}
