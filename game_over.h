void enter_your_name(char name[]) {
    ALLEGRO_BITMAP *text_background = NULL;
    ALLEGRO_EVENT_QUEUE *simple_queue = NULL;
    ALLEGRO_USTR *str = al_ustr_new("");
    ALLEGRO_FONT *font = al_load_ttf_font("retro_font.TTF", 22, 0);
    int pos = (int)al_ustr_size(str);
    
    int text_box_width = 500;
    int text_box_height = 300;
    
    text_background = al_create_bitmap(text_box_width, text_box_height);
    al_set_target_bitmap(text_background);
	al_clear_to_color(al_map_rgba(255, 255, 255, 200));
	al_set_target_bitmap(al_get_backbuffer(DISPLAY));
	al_draw_bitmap(text_background, SCREEN_W / 2 - text_box_width / 2, SCREEN_H / 2 - text_box_height / 2, 0);
	
	simple_queue = al_create_event_queue();
	al_register_event_source(simple_queue, al_get_mouse_event_source());
	al_register_event_source(simple_queue, al_get_keyboard_event_source());
	al_draw_bitmap(text_background, SCREEN_W / 2 - text_box_width / 2, SCREEN_H / 2 - text_box_height / 2, 0);
	al_draw_text(font, al_map_rgb(0,0,0), SCREEN_W / 2, SCREEN_H / 2 - 50, ALLEGRO_ALIGN_CENTRE, "Enter your name:");
	al_flip_display();
	
	strncpy(name, "unknown", 20);
	
	while (1) {
	    ALLEGRO_EVENT ev;
	    al_wait_for_event(simple_queue, &ev);
	    if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
		    break;
		}
	    else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
	        if(ev.keyboard.unichar >= 32 && pos < 19 && ev.keyboard.keycode != ALLEGRO_KEY_SPACE) {
		        pos += al_ustr_append_chr(str, ev.keyboard.unichar);
		    }
		    else if(ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
			    if(al_ustr_prev(str, &pos)) al_ustr_truncate(str, pos);
			}
			al_ustr_to_buffer(str, name, 19);
			al_draw_bitmap(text_background, SCREEN_W / 2 - text_box_width / 2, SCREEN_H / 2 - text_box_height / 2, 0);
			al_draw_text(font, al_map_rgb(0,0,0), SCREEN_W / 2, SCREEN_H / 2 - 50, ALLEGRO_ALIGN_CENTRE, "Enter your name:");
			al_draw_text(font, al_map_rgb(0,0,0), SCREEN_W / 2, SCREEN_H / 2, ALLEGRO_ALIGN_CENTRE, name);
	    }
	    else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
		    if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
			    break;
			}
		}
	    al_flip_display();
	}
	al_ustr_free(str);
	al_destroy_bitmap(text_background);
}

void game_over(int *score, bool *display_menu, bool *game_paused, bool *you_lost, bool *redraw) {
	al_rest(1.5);
	bool name_entered = false;
	char player_name[20];
    struct winner {
	    char nickname[20];
	    int result;
	};
	winner list_of_winners[10];
	FILE *winners = fopen("winners.txt", "r");
	for (int i = 0; i < 10; i++) {
	        fscanf(winners, "%s", list_of_winners[i].nickname);
	        fscanf(winners, "%d", &list_of_winners[i].result);
	}
	fclose(winners);
	for (int i = 9; i >= 0; i--) {
	    if (*score > list_of_winners[i].result) {
			if (i < 9) {
				list_of_winners[i + 1].result = list_of_winners[i].result;
				strncpy(list_of_winners[i + 1].nickname, list_of_winners[i].nickname, 20);
			}
			if (!name_entered) {
				name_entered = true;
			    enter_your_name(player_name);
			}
			list_of_winners[i].result = *score;
			strncpy(list_of_winners[i].nickname, player_name, 20);
			list_of_winners[i].nickname[19] = '\0';
		}
	}
	winners = fopen("winners.txt", "w");
	for (int i = 0; i < 10; i++) {
	    fprintf(winners, "%s\n%d\n", list_of_winners[i].nickname, list_of_winners[i].result);
	}
	fclose(winners);
	if (name_entered) show_high_scores();
	*display_menu = true;
	*game_paused = true;
	*you_lost = false;
	*redraw = true;
}
