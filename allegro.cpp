// allegro.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <math.h>
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

#define TIMER_FPS 120
#define PIXEL_PER_SECOND 440.0
const double pixel_per_frame = PIXEL_PER_SECOND / TIMER_FPS;

typedef struct color {
    int r, g, b;
}COLOR;

typedef struct kindbeat {
    int kind;
    double beat, placein, placeout, soul_placeX, soul_placeY, radius;
    bool hit_true;
}KINDBEAT;

typedef struct songset {
    string name, composer, singer, background;
    int bpm;
}SONGSET;

typedef struct taiko_result {
    int perfect, good, miss, maxcombo, score;
    bool FC, AP, END_G;
}TAIKO_RESULT;

typedef struct fly_soul {
    double x, y;
}FLY_SOUL;

std::vector<double> get_flash_fun()
{
    int a, z;

    std::vector<double> flash_fun(TIMER_FPS * 3);

    for (a = 0; a < flash_fun.size(); a++) {
        flash_fun[a] = (a + 1) * (a + 1) * exp(-(a + 1) / (TIMER_FPS / 20.0));
    }

    double max = flash_fun[0];

    for (z = 1; z < flash_fun.size(); z++) {

        if (flash_fun[z] > max) {
            max = flash_fun[z];
        }
    }

    for (a = 0; a < flash_fun.size(); a++) {
        flash_fun[a] /= max;
    }
    
    return flash_fun;
}

TAIKO_RESULT Song_play(string);
string Song_select();
int Song_score(TAIKO_RESULT);
int door_close();
int door_open();
bool want_end(bool);

int main()
{
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();
    al_init_image_addon();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(16);
    al_init_ttf_addon();

    bool game_end = false;

    while(!game_end) {
        door_open();
        string name;
        name = Song_select();
        door_close();

        TAIKO_RESULT resultT;

        if (!name.empty()) {
            door_open();
            resultT = Song_play(name);
            door_close();

            if (!resultT.END_G) {
                door_open();
                Song_score(resultT);
                door_close();
            }
        }

        game_end = want_end(game_end);

    }

    return 0;
}

string Song_select()
{
    bool done = false;

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_DISPLAY* disp = al_create_display(1280, 720);
    ALLEGRO_FONT* font = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 24, 0);
    ALLEGRO_FONT* font_2 = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 28, 0);
    ALLEGRO_EVENT event;

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    fstream fs;
    fs.open("song_list.txt", ios::in);
    vector<SONGSET>songsvec;
    if (fs.is_open()) {
        SONGSET m;
        while (!fs.eof()) {
            fs >> m.name >> m.bpm >> m.composer >> m.singer >> m.background;
            songsvec.push_back(m);
        }
    }

    ALLEGRO_BITMAP* vocaloid_ch = al_load_bitmap("vocaloid_choose.png");
    ALLEGRO_BITMAP* fallen_angel_ch = al_load_bitmap("fallen_angel_choose.png");
    ALLEGRO_SAMPLE* song=NULL;// = al_load_sample("fallen_angel_master_ozone.wav");

    int selection = 0;
    bool selection_changed = true;

    ALLEGRO_SAMPLE_ID id;

    while (1) {

        if (selection_changed) {
            //draw BG and text
            al_clear_to_color(al_map_rgb(0, 0, 0));
            if (selection > 0) {
                al_draw_filled_rectangle(160, 240, 400, 480, al_map_rgb(200, 200, 200));
                al_draw_scaled_bitmap(fallen_angel_ch, 0, 0, al_get_bitmap_width(fallen_angel_ch), al_get_bitmap_height(fallen_angel_ch), 170, 250, 220, 220, 0);

                al_draw_filled_rectangle(460, 180, 820, 540, al_map_rgb(200, 200, 200));
                al_draw_scaled_bitmap(vocaloid_ch, 0, 0, al_get_bitmap_width(vocaloid_ch), al_get_bitmap_height(vocaloid_ch), 470, 190, 340, 340, 0);
            }

            if (selection < songsvec.size() - 1) {

                al_draw_filled_rectangle(460, 180, 820, 540, al_map_rgb(200, 200, 200));
                al_draw_scaled_bitmap(fallen_angel_ch, 0, 0, al_get_bitmap_width(fallen_angel_ch), al_get_bitmap_height(fallen_angel_ch), 470, 190, 340, 340, 0);

                al_draw_filled_rectangle(880, 240, 1120, 480, al_map_rgb(200, 200, 200));
                al_draw_scaled_bitmap(vocaloid_ch, 0, 0, al_get_bitmap_width(vocaloid_ch), al_get_bitmap_height(vocaloid_ch), 890, 250, 220, 220, 0);
            }

            al_draw_text(font, al_map_rgb(255, 255, 255), 640, 100, ALLEGRO_ALIGN_CENTER, "Song Select");

            al_draw_text(font, al_map_rgb(255, 255, 255), 640, 340, ALLEGRO_ALIGN_CENTER, songsvec[selection].name.c_str());

            if (selection > 0) {
                al_draw_text(font, al_map_rgb(255, 255, 255), 280, 350, ALLEGRO_ALIGN_CENTER, songsvec[selection - 1].name.c_str());
            }
            if (selection < songsvec.size() - 1) {
                al_draw_text(font, al_map_rgb(255, 255, 255), 1000, 350, ALLEGRO_ALIGN_CENTER, songsvec[selection + 1].name.c_str());
            }

            //for (int a = 0; a < songsvec.size(); a++) {
            //    if (songsvec[selection].name == songsvec[a].name) {
                    //ALLEGRO_SAMPLE*
            song = al_load_sample((songsvec[selection].name + ".wav").c_str());
            al_play_sample(song, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &id);
            cout << "selection " << selection << endl;

            //    }
            //}
            selection_changed = false;

            al_flip_display();
        }

        al_wait_for_event(queue, &event);
        cout << "event" << endl;

        switch (event.type)
        {
        case ALLEGRO_EVENT_KEY_DOWN:
            cout << "keydown " << event.keyboard.keycode << endl;
            if ((event.keyboard.keycode == ALLEGRO_KEY_W) && (selection > 0)) {
                selection--;
                selection_changed = true;
                al_stop_sample(&id);
            }
            if ((event.keyboard.keycode == ALLEGRO_KEY_E) && (selection < songsvec.size()-1)){
                selection++;
                selection_changed = true;
                al_stop_sample(&id);
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                done = true;
                selection = -1;
                al_stop_sample(&id);
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER){
                done = true;
                al_stop_sample(&id);
            }
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            selection = -1;
            al_stop_sample(&id);
            break;
        }

        if (done)
            break;
    }

    int a;
    for (a = 0; a < songsvec.size(); a++) {
        cout << songsvec[a].name << endl;
        cout << songsvec[a].bpm << endl;
        cout << songsvec[a].composer << endl;
        cout << songsvec[a].singer << endl;
        cout << songsvec[a].background << endl;
    }
    
    fs.close();
    al_destroy_sample(song);
    al_destroy_bitmap(vocaloid_ch);
    al_destroy_bitmap(fallen_angel_ch);
    al_destroy_font(font);
    al_destroy_font(font_2);
    al_destroy_display(disp);
    al_destroy_event_queue(queue);

    if (selection >= 0)
        return songsvec[selection].name;

    else
        return "";

    
}



TAIKO_RESULT Song_play(string name)
{
    std::vector<double> flash_fun = get_flash_fun();

    ALLEGRO_SAMPLE* taiko_C = al_load_sample("kick.wav");
    ALLEGRO_SAMPLE* taiko_S = al_load_sample("side.wav");
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / TIMER_FPS);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_DISPLAY* disp = al_create_display(1280, 720);
    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_BITMAP* fumofumo = al_load_bitmap((name + ".png").c_str());
    ALLEGRO_SAMPLE* song = al_load_sample((name + ".wav").c_str());
    ALLEGRO_BITMAP* Gaussian = al_load_bitmap("Gaussian_1.png");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));


    std::vector<KINDBEAT> beats;

    std::fstream fs((name + ".txt").c_str(), std::fstream::in);

    if (fs.is_open()) {
        while (!fs.eof()) {
            KINDBEAT a;
            fs >> a.kind;
            fs >> a.beat;
            fs >> a.placein;
            fs >> a.placeout;
            fs >> a.soul_placeX;
            fs >> a.soul_placeY;
            fs >> a.radius;

            a.placein = 400 + PIXEL_PER_SECOND * (a.beat + .03);

            beats.push_back(a);
        }
        printf("%zd", beats.size());
    }
    else
        printf("couldn't open gameplay files.\n");
    fs.close();

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;
    TAIKO_RESULT resultT;
    resultT.END_G = false;

    for (int a = 0; a < beats.size(); a++) {
        beats[a].hit_true = false;
    }

    al_play_sample(song, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

    int drum_C_L_flash_t = -1, drum_C_R_flash_t = -1, drum_S_L_flash_t = -1, drum_S_R_flash_t = -1,
        drum_C_touch = -1, drum_S_touch = -1,
        good = -1, perfect = -1, good_count = 0, perfect_count = 0, miss_count = 0, combo = 0, maxcombo = 0, * pmaxcombo, point = 0,
        old_score = 0, new_score = 0, display_score = 0, score_time = -1;

    char str_score[1000], combo_now[1000];

    double pass_count = 0;
    bool if_pass = false;

    pmaxcombo = &maxcombo;

    COLOR gray, blue, red;

    gray.r = 200;
    gray.g = 200;
    gray.b = 200;
    blue.r = 102;
    blue.g = 214;
    blue.b = 255;
    red.r = 253;
    red.g = 51;
    red.b = 51;

    double place = 0;
    double palabola_place = 0;
    bool blue_hit = false, red_hit = false;


    al_start_timer(timer);
    while (1)
    {
        al_wait_for_event(queue, &event);

        int x, min_red = 10000000, min_blue = 10000000, min_num_red = 0, min_num_blue = 0, min_place = 0;
        for (x = 0; x < beats.size() - 1; x++) {

            if ((fabs(beats[x].placein - 400) < min_red) && (beats[x].kind == 1)) {
                min_red = fabs(beats[x].placein - 400);
                min_num_red = x;
            }
            if ((fabs(beats[x].placein - 400) < min_blue) && (beats[x].kind == 2)) {
                min_blue = fabs(beats[x].placein - 400);
                min_num_blue = x;
            }

        }
        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            // game logic goes here.
            redraw = true;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            if ((event.keyboard.keycode == ALLEGRO_KEY_F) || (event.keyboard.keycode == ALLEGRO_KEY_J)) {
                al_play_sample(taiko_C, 3.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            if ((event.keyboard.keycode == ALLEGRO_KEY_D) || (event.keyboard.keycode == ALLEGRO_KEY_K)) {
                al_play_sample(taiko_S, 5.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            }


            if (((event.keyboard.keycode == ALLEGRO_KEY_F) || (event.keyboard.keycode == ALLEGRO_KEY_J)) && (beats[min_num_red].placein < 500) && (beats[min_num_red].placein > 300))
            {
                red_hit = true;
                printf("red hit.\n");
                beats[min_num_red].hit_true = true;
            }
            if (((event.keyboard.keycode == ALLEGRO_KEY_D) || (event.keyboard.keycode == ALLEGRO_KEY_K)) && (beats[min_num_blue].placein < 500) && (beats[min_num_blue].placein > 300))
            {
                blue_hit = true;
                printf("blue hit.\n");
                beats[min_num_blue].hit_true = true;
            }


            if (event.keyboard.keycode == ALLEGRO_KEY_D) {
                drum_S_L_flash_t = 0;
                double ww = beats[min_num_blue].placein;
                if (((ww < 500) && (ww > 425)) || ((ww < 375) && (ww > 300))) {
                    drum_S_touch = 0;
                    beats[min_num_blue].placein += 1000000;
                    good = 0;
                    good_count++;
                    combo++;
                    if (combo < 200) {
                        new_score += 500;
                        score_time = 0;
                    }
                    else {
                        new_score += 1000;
                        score_time = 0;
                    }
                }
                /*TODO: Change to else if and else to miss*/
                if ((ww >= 375) && (ww <= 425)) {
                    drum_S_touch = 0;
                    beats[min_num_blue].placein += 1000000;
                    perfect = 0;
                    perfect_count++;
                    combo++;
                    if (combo < 200) {
                        new_score += 1000;
                        score_time = 0;
                    }
                    else {
                        new_score += 2000;
                        score_time = 0;
                    }
                }
                if ((score_time >= 0) && (score_time < 30)) {
                    display_score = (old_score + new_score) / 2;
                    score_time++;
                }
                if (score_time >= 30) {
                    display_score = new_score;
                    old_score = new_score;
                    score_time = -1;
                }
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_F) {
                drum_C_L_flash_t = 0;
                double ww = beats[min_num_red].placein;
                if (((ww < 500) && (ww > 425)) || ((ww < 375) && (ww > 300))) {
                    drum_S_touch = 0;
                    beats[min_num_red].placein += 1000000;
                    good = 0;
                    good_count++;
                    combo++;
                    if (combo < 200) {
                        new_score += 500;
                        score_time = 0;
                    }
                    else {
                        new_score += 1000;
                        score_time = 0;
                    }
                }
                if ((ww >= 375) && (ww <= 425)) {
                    drum_S_touch = 0;
                    beats[min_num_red].placein += 1000000;
                    perfect = 0;
                    perfect_count++;
                    combo++;
                    if (combo < 200) {
                        new_score += 1000;
                        score_time = 0;
                    }
                    else {
                        new_score += 2000;
                        score_time = 0;
                    }
                }
                if ((score_time >= 0) && (score_time < 30)) {
                    display_score = (old_score + new_score) / 2;
                    score_time++;
                }
                if (score_time >= 30) {
                    display_score = new_score;
                    old_score = new_score;
                    score_time = -1;
                }
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_J) {
                drum_C_R_flash_t = 0;
                double ww = beats[min_num_red].placein;
                if (((ww < 500) && (ww > 425)) || ((ww < 375) && (ww > 300))) {
                    drum_S_touch = 0;
                    beats[min_num_red].placein += 1000000;
                    good = 0;
                    good_count++;
                    combo++;
                    if (combo < 200) {
                        new_score += 500;
                        score_time = 0;
                    }
                    else {
                        new_score += 1000;
                        score_time = 0;
                    }
                }
                if ((ww >= 375) && (ww <= 425)) {
                    drum_S_touch = 0;
                    beats[min_num_red].placein += 1000000;
                    perfect = 0;
                    perfect_count++;
                    combo++;
                    if (combo < 200) {
                        new_score += 1000;
                        score_time = 0;
                    }
                    else {
                        new_score += 2000;
                        score_time = 0;
                    }
                }
                if ((score_time >= 0) && (score_time < 30)) {
                    display_score = (old_score + new_score) / 2;
                    score_time++;
                }
                if (score_time >= 30) {
                    display_score = new_score;
                    old_score = new_score;
                    score_time = -1;
                }
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_K) {
                drum_S_R_flash_t = 0;
                double ww = beats[min_num_blue].placein;
                if (((ww < 500) && (ww > 425)) || ((ww < 375) && (ww > 300))) {
                    drum_S_touch = 0;
                    beats[min_num_blue].placein += 1000000;
                    good = 0;
                    good_count++;
                    combo++;
                    if (combo < 200) {
                        new_score += 500;
                        score_time = 0;
                    }
                    else {
                        new_score += 1000;
                        score_time = 0;
                    }
                }
                if ((ww >= 375) && (ww <= 425)) {
                    drum_S_touch = 0;
                    beats[min_num_blue].placein += 1000000;
                    perfect = 0;
                    perfect_count++;
                    combo++;
                    if (combo < 200) {
                        new_score += 1000;
                        score_time = 0;
                    }
                    else {
                        new_score += 2000;
                        score_time = 0;
                    }
                }
                if ((score_time >= 0) && (score_time < 30)) {
                    display_score = (old_score + new_score) / 2;
                    score_time++;
                }
                if (score_time >= 30) {
                    display_score = new_score;
                    old_score = new_score;
                    score_time = -1;
                }
            }
            
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                done = true;
            }
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }

        if (beats[beats.size() - 1].placein < 1000000) {
            int loop;
            for (loop = 0; loop < beats.size(); loop++) {
                if ((beats[loop].placein <= 300) && (beats[beats.size() - 1].placein >= 300)){
                    miss_count++;
                    if (beats[min_num_red].placein >= beats[min_num_blue].placein)
                        beats[min_num_blue].placein += 1000000;
                    else
                        beats[min_num_red].placein += 1000000;

                    if (combo > maxcombo) {
                        maxcombo = combo;
                    }
                    combo = 0;
                }
            }
        }

        double ratio = beats.size() / 20.0;

        double life = (2 * perfect_count) + (0.5 * good_count);
        life -= miss_count;
        int red_soul, yellow_soul;

        if (redraw && al_is_event_queue_empty(queue))
        {
            
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_scaled_bitmap(Gaussian, 0, 0, al_get_bitmap_width(Gaussian), al_get_bitmap_height(Gaussian), 0, 0, 1280, 720, 0);

            //al_draw_text(font, al_map_rgb(255, 255, 255), 1280 / 2, 720 / 2, ALLEGRO_ALIGN_CENTER, "fly away now.");
            al_draw_filled_rounded_rectangle(100, 100, 1280, 300, 20, 100, al_map_rgb(50, 50, 50));

            al_draw_filled_rectangle(500, 40, 950, 80, al_map_rgb(150, 150, 150));
            al_draw_filled_rectangle(950, 20, 1100, 80, al_map_rgb(150, 150, 150));
            al_draw_filled_circle(1100, 60, 40, al_map_rgb(200, 200, 200));
            al_draw_text(font, al_map_rgb(0, 0, 0), 1200, 60, ALLEGRO_ALIGN_CENTER, "Soul");


            if (life <= (ratio * 20)) {
                life = (2 * perfect_count) + (0.5 * good_count);
                life -= miss_count;
                yellow_soul = (life / ratio) - 15;
                for (int b = 0; b < yellow_soul; b++) {
                    al_draw_filled_rectangle(950 + (30 * b) + 2, 22, 950 + (30 * (b + 1)) - 4, 78, al_map_rgb(255, 242, 0));
                }
            }

            if (life > (ratio * 20)) {
                for (int b = 0; b < 5; b++) {
                    al_draw_filled_rectangle(950 + (30 * b) + 2, 22, 950 + (30 * (b + 1)) - 4, 78, al_map_rgb(255, 242, 0));
                }
            }
            
            if (life <= (ratio * 15)) {
                red_soul = life / ratio;
                for (int a = 0; a < red_soul; a++) {
                    al_draw_filled_rectangle(500 + (30 * a) + 2, 42, 500 + (30 * (a + 1)) - 4, 78, al_map_rgb(red.r, red.g, red.b));
                }
            }
            if (life > (ratio * 15)) {
                for (int a = 0; a < 15; a++) {
                    al_draw_filled_rectangle(500 + (30 * a) + 2, 42, 500 + (30 * (a + 1)) - 4, 78, al_map_rgb(red.r, red.g, red.b));
                }
            }


            //breath effect of drum top
            int r, g, b;
            double v;

            if (drum_S_L_flash_t == -1)
                v = 0;
            else if ((drum_S_L_flash_t >= 0) && (drum_S_L_flash_t <= 59)) {
                v = flash_fun[drum_S_L_flash_t];
                drum_S_L_flash_t++;
                if (drum_S_L_flash_t == 60)
                    drum_S_L_flash_t = -1;
            }
            r = (blue.r * v) + (gray.r * (1 - v));
            g = (blue.g * v) + (gray.g * (1 - v));
            b = (blue.b * v) + (gray.b * (1 - v));
            al_draw_filled_pieslice(200, 200, 100, 1.5708, 3.1415, al_map_rgb(r, g, b));

            if (drum_C_L_flash_t == -1)
                v = 0;
            else if ((drum_C_L_flash_t >= 0) && (drum_C_L_flash_t <= 59)) {
                v = flash_fun[drum_C_L_flash_t];
                drum_C_L_flash_t++;
                if (drum_C_L_flash_t == 60)
                    drum_C_L_flash_t = -1;
            }
            r = (red.r * v) + (gray.r * (1 - v));
            g = (red.g * v) + (gray.g * (1 - v));
            b = (red.b * v) + (gray.b * (1 - v));
            al_draw_filled_pieslice(200, 200, 70, 1.5708, 3.1415, al_map_rgb(r, g, b));

            if (drum_S_R_flash_t == -1)
                v = 0;
            else if ((drum_S_R_flash_t >= 0) && (drum_S_R_flash_t <= 59)) {
                v = flash_fun[drum_S_R_flash_t];
                drum_S_R_flash_t++;
                if (drum_S_R_flash_t == 60)
                    drum_S_R_flash_t = -1;
            }
            r = (blue.r * v) + (gray.r * (1 - v));
            g = (blue.g * v) + (gray.g * (1 - v));
            b = (blue.b * v) + (gray.b * (1 - v));
            al_draw_filled_pieslice(200, 200, 100, 4.7122, 3.1414, al_map_rgb(r, g, b));

            if (drum_C_R_flash_t == -1)
                v = 0;
            else if ((drum_C_R_flash_t >= 0) && (drum_C_R_flash_t <= 59)) {
                v = flash_fun[drum_C_R_flash_t];
                drum_C_R_flash_t++;
                if (drum_C_R_flash_t == 60)
                    drum_C_R_flash_t = -1;
            }
            r = (red.r * v) + (gray.r * (1 - v));
            g = (red.g * v) + (gray.g * (1 - v));
            b = (red.b * v) + (gray.b * (1 - v));
            al_draw_filled_pieslice(200, 200, 70, 4.7122, 3.1414, al_map_rgb(r, g, b));

            al_draw_pieslice(200, 200, 70, 1.5708, 3.1415, al_map_rgb(70, 70, 70), 3);
            al_draw_pieslice(200, 200, 70, 4.7122, 3.1414, al_map_rgb(70, 70, 70), 3);
            al_draw_pieslice(200, 200, 100, 1.5708, 3.1415, al_map_rgb(70, 70, 70), 3);
            al_draw_pieslice(200, 200, 100, 4.7122, 3.1414, al_map_rgb(70, 70, 70), 3);

            //central line
            al_draw_line(200, 100, 200, 300, al_map_rgb(70, 70, 70), 4);
            //hit point
            al_draw_filled_circle(400, 200, 55, al_map_rgb(gray.r, gray.g, gray.b));


            for (x = 0; x < beats.size(); x++) {

                if ((beats[x].placein > 300) && (beats[x].placein <= 1280) && (beats[x].kind == 1)) {
                    al_draw_filled_circle(beats[x].placein, 200, 45, al_map_rgb(255, 255, 255));
                    al_draw_filled_circle(beats[x].placein, 200, 40, al_map_rgb(red.r, red.g, red.b));
                    //good will initialized as 0 with disappear. half second exist.
                    if ((good >= 0) && (good < 60) && (perfect == -1)) {
                        al_draw_text(font, al_map_rgb(255, 255, 255), 400, 100, ALLEGRO_ALIGN_CENTER, "good!");
                        good++;
                    }
                    if ((perfect >= 0) && (perfect < 60)) {
                        al_draw_text(font, al_map_rgb(255, 255, 255), 400, 100, ALLEGRO_ALIGN_CENTER, "PERFECT!");
                        perfect++;
                    }
                    if (perfect >= 60)
                        perfect = -1;
                }

                if ((beats[x].placein > 300) && (beats[x].placein <= 1280) && (beats[x].kind == 2)) {
                    al_draw_filled_circle(beats[x].placein, 200, 45, al_map_rgb(255, 255, 255));
                    al_draw_filled_circle(beats[x].placein, 200, 40, al_map_rgb(blue.r, blue.g, blue.b));
                    if ((good >= 0) && (good < 60) && (perfect == -1)) {
                        al_draw_text(font, al_map_rgb(255, 255, 255), 400, 100, ALLEGRO_ALIGN_CENTER, "good!");
                        good++;
                    }
                    if ((perfect >= 0) && (perfect < 60)) {
                        al_draw_text(font, al_map_rgb(255, 255, 255), 400, 100, ALLEGRO_ALIGN_CENTER, "PERFECT!");
                        perfect++;
                    }
                    if (perfect >= 60)
                        perfect = -1;
                }

                if ((beats[x].hit_true) && (beats[x].soul_placeX <= 1100) && (beats[x].soul_placeY >= 60) && (beats[x].kind == 1)) {
                    al_draw_filled_circle(beats[x].soul_placeX, beats[x].soul_placeY, beats[x].radius + 5.0, al_map_rgb(255, 255, 255));
                    al_draw_filled_circle(beats[x].soul_placeX, beats[x].soul_placeY, beats[x].radius, al_map_rgb(red.r, red.g, red.b));
                    beats[x].soul_placeX += 13.33;
                    beats[x].soul_placeY -= 2.33;
                    beats[x].radius -= 0.0833;
                }
                if ((beats[x].hit_true) && (beats[x].soul_placeX <= 1100) && (beats[x].soul_placeY >= 60) && (beats[x].kind == 2)) {
                    al_draw_filled_circle(beats[x].soul_placeX, beats[x].soul_placeY, beats[x].radius + 5.0, al_map_rgb(255, 255, 255));
                    al_draw_filled_circle(beats[x].soul_placeX, beats[x].soul_placeY, beats[x].radius, al_map_rgb(blue.r, blue.g, blue.b));
                    beats[x].soul_placeX += 13.33;
                    beats[x].soul_placeY -= 2.33;
                    beats[x].radius -= 0.0833;
                }

                sprintf_s(str_score, 1000, "%d", new_score);
                sprintf_s(combo_now, 1000, "%d", combo);
                al_draw_text(font, al_map_rgb(255, 255, 255), 400, 50, ALLEGRO_ALIGN_CENTER, combo_now);
                al_draw_text(font, al_map_rgb(255, 255, 255), 400, 20, ALLEGRO_ALIGN_CENTER, str_score);
                beats[x].placein -= pixel_per_frame;
            }
            if (done)
                break;

            al_flip_display();
            
            redraw = false;
        }
    }
    
    resultT.FC = false;
    resultT.AP = false;

    printf("\nperfect : %d\ngood : %d\nmiss : %d\n", perfect_count, good_count, miss_count);
    if (combo != beats.size()) {
        printf("Pass!\nMaxcombo is %d", maxcombo);
    }

    if ((combo == beats.size()) && (perfect_count != beats.size())) {
        printf("Full combo!!\n");
        resultT.FC = true;
    }

    if (perfect_count == beats.size()) {
        printf("Congratulation! It's an All Perfect!\n");
        resultT.AP = true;
    }

    resultT.perfect = perfect_count;
    resultT.good = good_count;
    resultT.miss = miss_count;
    resultT.maxcombo = maxcombo;
    resultT.score = new_score;

    al_destroy_sample(song);
    al_destroy_bitmap(fumofumo);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    

    return resultT;
}



int Song_score(TAIKO_RESULT resultT)
{
    bool done = false;

    ALLEGRO_BITMAP* fumofumo = al_load_bitmap("fumofumo.png");
    ALLEGRO_SAMPLE* taikosound = al_load_sample("Taiko hit.wav");
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_DISPLAY* disp = al_create_display(1280, 720);
    ALLEGRO_FONT* font_score = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 72, 0);
    ALLEGRO_FONT* font_big = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 48, 0);
    ALLEGRO_FONT* font_small = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 24, 0);
    ALLEGRO_EVENT event;

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    al_clear_to_color(al_map_rgb(0, 0, 0));

    string perfect_count = to_string(resultT.perfect);
    string good_count = to_string(resultT.good);
    string miss_count = to_string(resultT.miss);
    string maxcombo = to_string(resultT.maxcombo);

    while (1) {

        al_draw_text(font_score, al_map_rgb(255, 255, 255), 640, 100, ALLEGRO_ALIGN_CENTER, "Song Score");

        al_draw_text(font_small, al_map_rgb(255, 255, 255), 750, 300, ALLEGRO_ALIGN_CENTER, (string("Perfect   ") + perfect_count).c_str());
        al_draw_text(font_small, al_map_rgb(255, 255, 255), 750, 350, ALLEGRO_ALIGN_CENTER, (string("Good   ") + good_count).c_str());
        al_draw_text(font_small, al_map_rgb(255, 255, 255), 750, 400, ALLEGRO_ALIGN_CENTER, (string("Miss   ") + miss_count).c_str());

        if (resultT.maxcombo != 0) {
            al_draw_text(font_small, al_map_rgb(255, 255, 255), 640, 450, ALLEGRO_ALIGN_CENTER, (string("maxcombo   ") + maxcombo).c_str());
            al_draw_text(font_big, al_map_rgb(255, 255, 255), 400, 200, ALLEGRO_ALIGN_CENTER, "PASS");
        }
        if(resultT.FC)
            al_draw_text(font_big, al_map_rgb(255, 255, 255), 400, 200, ALLEGRO_ALIGN_CENTER, "FULL COMBO!!");
        if (resultT.AP)
            al_draw_text(font_big, al_map_rgb(255, 255, 255), 400, 200, ALLEGRO_ALIGN_CENTER, "ALL PERFECT!!!!");

        al_flip_display();

        al_wait_for_event(queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_KEY_DOWN:

            if ((event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) || (event.keyboard.keycode == ALLEGRO_KEY_ENTER)) {
                done = true;
            }
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }

        if (done)
            break;
    }

    al_destroy_sample(taikosound);
    al_destroy_bitmap(fumofumo);
    al_destroy_font(font_small);
    al_destroy_font(font_big);
    al_destroy_font(font_score);
    al_destroy_display(disp);
    al_destroy_event_queue(queue);

    return 0;

}

int door_close() {
    int step_amount = 30;
    bool done = false;

    ALLEGRO_BITMAP* fumofumo = al_load_bitmap("fumofumo.png");
    ALLEGRO_BITMAP* left_door = al_load_bitmap("left.png");
    ALLEGRO_SAMPLE* taikosound = al_load_sample("Taiko hit.wav");
    ALLEGRO_DISPLAY* disp = al_create_display(1280, 720);
    ALLEGRO_FONT* font_big = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 28, 0);
    ALLEGRO_FONT* font_small = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 24, 0);

    al_clear_to_color(al_map_rgb(0, 0, 0));

    double move = 0;

    for (int a = 0; a < step_amount; a++) {
        al_draw_scaled_bitmap(left_door, 0, 0, al_get_bitmap_width(left_door), al_get_bitmap_height(left_door), move - 640, 0, 640, 720, 0);
        al_draw_scaled_bitmap(left_door, 0, 0, al_get_bitmap_width(left_door), al_get_bitmap_height(left_door), 1280 - move, 0, 640, 720, ALLEGRO_FLIP_HORIZONTAL);

        if (move <= 680)
            move += 30;
        else {
            done = true;
        }
        if (done)
            break;

        al_flip_display();
        al_rest(1.0 / step_amount);

        al_clear_to_color(al_map_rgb(0, 0, 0));
    }

    al_destroy_bitmap(left_door);
    al_destroy_sample(taikosound);
    al_destroy_bitmap(fumofumo);
    al_destroy_font(font_small);
    al_destroy_font(font_big);
    al_destroy_display(disp);

    return 0;
}



int door_open() {
    int step_amount = 30;
    bool done = false;

    ALLEGRO_BITMAP* fumofumo = al_load_bitmap("fumofumo.png");
    ALLEGRO_BITMAP* left_door = al_load_bitmap("left.png");
    ALLEGRO_SAMPLE* taikosound = al_load_sample("Taiko hit.wav");
    ALLEGRO_DISPLAY* disp = al_create_display(1280, 720);
    ALLEGRO_FONT* font_big = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 28, 0);
    ALLEGRO_FONT* font_small = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 24, 0);

    al_clear_to_color(al_map_rgb(0, 0, 0));

    double move = 0;

    for (int a = 0; a < step_amount; a++) {
        al_draw_scaled_bitmap(left_door, 0, 0, al_get_bitmap_width(left_door), al_get_bitmap_height(left_door), -move, 0, 640, 720, 0);
        al_draw_scaled_bitmap(left_door, 0, 0, al_get_bitmap_width(left_door), al_get_bitmap_height(left_door), 640 + move, 0, 640, 720, ALLEGRO_FLIP_HORIZONTAL);

        if (move <= 680)
            move += 30;
        else {
            done = true;
        }
        if (done)
            break;

        al_flip_display();
        al_rest(1.0 / step_amount);

        al_clear_to_color(al_map_rgb(0, 0, 0));
    }

    al_destroy_bitmap(left_door);
    al_destroy_sample(taikosound);
    al_destroy_bitmap(fumofumo);
    al_destroy_font(font_small);
    al_destroy_font(font_big);
    al_destroy_display(disp);

    return 0;
}

bool want_end(bool end_game)
{
    bool done = false, wantend = false, left = false;

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_DISPLAY* disp = al_create_display(1280, 720);
    ALLEGRO_FONT* font = al_load_ttf_font("C:\\Windows\\Fonts\\ARLRDBD.TTF", 24, 0);
    ALLEGRO_EVENT event;
    ALLEGRO_BITMAP* end_yes = al_load_bitmap("end_yes.png");
    ALLEGRO_BITMAP* end_no = al_load_bitmap("end_no.png");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    while (1) {

        al_clear_to_color(al_map_rgb(0, 0, 0));

        if (left) {
            al_draw_filled_rectangle(160, 240, 400, 480, al_map_rgb(200, 200, 200));
            al_draw_scaled_bitmap(end_yes, 0, 0, al_get_bitmap_width(end_yes), al_get_bitmap_height(end_yes), 170, 250, 220, 220, 0);

            al_draw_filled_rectangle(460, 180, 820, 540, al_map_rgb(200, 200, 200));
            al_draw_scaled_bitmap(end_no, 0, 0, al_get_bitmap_width(end_no), al_get_bitmap_height(end_no), 470, 190, 340,340, 0);
        }
        
        if (!left) {
            al_draw_filled_rectangle(460, 180, 820, 540, al_map_rgb(200, 200, 200));
            al_draw_scaled_bitmap(end_yes, 0, 0, al_get_bitmap_width(end_yes), al_get_bitmap_height(end_yes), 470, 190, 340, 340, 0);

            al_draw_filled_rectangle(880, 240, 1120, 480, al_map_rgb(200, 200, 200));
            al_draw_scaled_bitmap(end_no, 0, 0, al_get_bitmap_width(end_no), al_get_bitmap_height(end_no), 890, 250, 220, 220, 0);
        }


        al_draw_text(font, al_map_rgb(255, 255, 255), 640, 100, ALLEGRO_ALIGN_CENTER, "Wanna End?");

        if (left) {
            al_draw_text(font, al_map_rgb(255, 255, 255), 640, 340, ALLEGRO_ALIGN_CENTER, "No");
            al_draw_text(font, al_map_rgb(255, 255, 255), 280, 350, ALLEGRO_ALIGN_CENTER, "Yes");
        }
        else {
            al_draw_text(font, al_map_rgb(255, 255, 255), 640, 340, ALLEGRO_ALIGN_CENTER, "Yes");
            al_draw_text(font, al_map_rgb(255, 255, 255), 1000, 350, ALLEGRO_ALIGN_CENTER, "no");
        }


        al_flip_display();

        al_wait_for_event(queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_KEY_DOWN:
            if (event.keyboard.keycode == ALLEGRO_KEY_W) {
                left = false;
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_E) {
                left = true;
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                end_game = true;
                done = true;
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                if (!left)
                    end_game = true;
                done = true;
            }
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            end_game = true;
            break;
        }

        if (done)
            break;

    }



    al_destroy_bitmap(end_yes);
    al_destroy_bitmap(end_no);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_event_queue(queue);

    return end_game;
}