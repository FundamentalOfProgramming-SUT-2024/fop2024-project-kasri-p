#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <ncurses.h>

void signup_login();

typedef struct
{
    const char *name;
    const char *path;
    Mix_Music *music;
} MusicTrack;

static MusicTrack playlist[] = {
    {"ELden Ring", "/Users/kasra/Desktop/songs/menu.mp3", NULL},
    {"Malenia, Blade Of Miquella", "/Users/kasra/Desktop/songs/Malenia.mp3", NULL},
    {"Radahn, The Promised Consort", "/Users/kasra/Desktop/songs/radahn.mp3", NULL},
    {"Radagon,of The Golden Order", "/Users/kasra/Desktop/songs/radagon.mp3", NULL},
    {"Bayle the Dread", "/path/to/epic.mp3", NULL}};
static const int NUM_TRACKS = 5;
static int current_track_index = 0;
static bool music_initialized = false;
static int music_volume = MIX_MAX_VOLUME;
static bool music_enabled = true;

bool init_music_system()
{
    if (music_initialized)
        return true;

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        SDL_Quit();
        return false;
    }

    music_initialized = true;
    return true;
}

void load_music(int index)
{
    if (index < 0 || index >= NUM_TRACKS)
        return;

    if (playlist[index].music == NULL)
    {
        playlist[index].music = Mix_LoadMUS(playlist[index].path);
        if (playlist[index].music == NULL)
        {
            printf("Failed to load music %s! SDL_mixer Error: %s\n",
                   playlist[index].name, Mix_GetError());
        }
    }
}

void play_music(int index)
{
    if (!music_initialized && !init_music_system())
        return;

    if (index < 0 || index >= NUM_TRACKS)
        return;

    load_music(index);

    if (playlist[index].music != NULL)
    {
        Mix_HaltMusic();
        Mix_VolumeMusic(music_volume);
        if (Mix_PlayMusic(playlist[index].music, -1) == -1)
        {
            printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
            return;
        }
        current_track_index = index;
    }
}

void music()
{
    play_music(2);
}

void toggle_music()
{
    if (!music_initialized)
        return;

    if (Mix_PlayingMusic())
    {
        if (Mix_PausedMusic())
        {
            Mix_ResumeMusic();
        }
        else
        {
            Mix_PauseMusic();
        }
    }
    else
    {
        play_music(current_track_index);
    }
}

void adjust_volume(int delta)
{
    music_volume = music_volume + delta;
    if (music_volume < 0)
        music_volume = 0;
    if (music_volume > MIX_MAX_VOLUME)
        music_volume = MIX_MAX_VOLUME;
    Mix_VolumeMusic(music_volume);
}

void draw_music_selector(int selected)
{
    clear();
    boarder();

    attron(A_BOLD);
    mvprintw(LINES / 4, COLS / 2 - 12, "※ Music Selection ※");
    attroff(A_BOLD);

    mvprintw(LINES / 4 + 2, COLS / 2 - 15, "Now Playing: %s",
             playlist[current_track_index].name);

    int start_y = LINES / 2 - NUM_TRACKS;

    mvprintw(start_y - 1, COLS / 2 - 20, "+----------------------------------+");

    for (int i = 0; i < NUM_TRACKS; i++)
    {
        mvprintw(start_y + i, COLS / 2 - 20, "|");
        if (i == selected)
        {
            attron(A_REVERSE | A_BOLD);
            mvprintw(start_y + i, COLS / 2 - 18, "♪ %-30s", playlist[i].name);
            attroff(A_REVERSE | A_BOLD);
        }
        else
        {
            mvprintw(start_y + i, COLS / 2 - 18, "  %-30s", playlist[i].name);
        }
        mvprintw(start_y + i, COLS / 2 + 19, "|");
    }

    mvprintw(start_y + NUM_TRACKS, COLS / 2 - 20,
             "+----------------------------------+");

    mvprintw(LINES - 4, COLS / 2 - 20, "↑↓: Select   Enter: Play   Esc: Back");
}

void music_selector()
{
    int selected = current_track_index;

    while (1)
    {
        draw_music_selector(selected);
        refresh();

        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            selected = (selected == 0) ? NUM_TRACKS - 1 : selected - 1;
            break;
        case KEY_DOWN:
            selected = (selected == NUM_TRACKS - 1) ? 0 : selected + 1;
            break;
        case '\n':
            play_music(selected);
            break;
        case 27: // ESC
            for (int i = 0; i < LINES; i++)
            {
                for (int j = 0; j < COLS; j++)
                {
                    mvprintw(i, j, " ");
                }
            }
            return;
        }
    }
}

void music_setting(int from, char username[], int score)
{
    clear();
    boarder();

    const char *options[] = {
        "Toggle Music",
        "Change Music",
        "Volume Up",
        "Volume Down",
        "Back"};
    int num_options = 5;
    int selected = 0;

    while (1)
    {
        attron(A_BOLD);
        mvprintw(LINES / 4, COLS / 2 - 10, "♪ Music Settings ♪");
        attroff(A_BOLD);

        mvprintw(LINES / 4 + 2, COLS / 2 - 17, "+--------------------------------+");
        mvprintw(LINES / 4 + 3, COLS / 2 - 17, "| Current Volume: %-14d |",
                 (music_volume * 100) / MIX_MAX_VOLUME);
        mvprintw(LINES / 4 + 4, COLS / 2 - 17, "| Music Status: %-16s |",
                 Mix_PlayingMusic() ? (Mix_PausedMusic() ? "Paused" : "Playing") : "Stopped");
        mvprintw(LINES / 4 + 5, COLS / 2 - 17, "| Current Track: %-15s | ",
                 playlist[current_track_index].name);
        mvprintw(LINES / 4 + 6, COLS / 2 - 17, "+--------------------------------+");

        for (int i = 0; i < num_options; i++)
        {
            if (i == selected)
            {
                attron(A_REVERSE | A_BOLD);
                mvprintw(LINES / 2 + i * 2, COLS / 2 - strlen(options[i]) / 2, "► %s ◄",
                         options[i]);
                attroff(A_REVERSE | A_BOLD);
            }
            else
            {
                mvprintw(LINES / 2 + i * 2, COLS / 2 - strlen(options[i]) / 2, "  %s  ",
                         options[i]);
            }
        }

        refresh();

        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            selected = (selected == 0) ? num_options - 1 : selected - 1;
            break;
        case KEY_DOWN:
            selected = (selected == num_options - 1) ? 0 : selected + 1;
            break;
        case '\n':
            switch (selected)
            {
            case 0:
                toggle_music();
                break;
            case 1:
                music_selector();
                clear();
                break;
            case 2:
                adjust_volume(MIX_MAX_VOLUME / 10);
                break;
            case 3:
                adjust_volume(-MIX_MAX_VOLUME / 10);
                break;
            case 4:
                clear();
                if (from == 1)
                {
                    signup_login();
                }
                else if (from == 2)
                {
                    pre_game_menu(username);
                }
                else if (from == 3)
                {
                    pause_menu(username, hero_x, hero_y, score);
                }
            }
            break;
        case 27:
            clear();
            return;
        }
    }
}

void cleanup_music()
{
    for (int i = 0; i < NUM_TRACKS; i++)
    {
        if (playlist[i].music != NULL)
        {
            Mix_FreeMusic(playlist[i].music);
            playlist[i].music = NULL;
        }
    }

    if (music_initialized)
    {
        Mix_HaltMusic();
        Mix_CloseAudio();
        SDL_Quit();
        music_initialized = false;
    }
}

void play_nightmare_music()
{
    const char *path = "/Users/kasra/Desktop/songs/enchant.mp3";
}

void play_enchanted_music()
{
    const char *path = "/Users/kasra/Desktop/songs/jori.mp3";
}