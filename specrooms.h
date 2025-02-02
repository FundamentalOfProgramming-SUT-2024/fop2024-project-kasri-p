#include <ncurses.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#define MIN_ROOM_SIZE 10
#define MAX_ROOM_SIZE 10

char **discovered_map;
int current_floor = 0;
int difficulty = 1;
int health = 100;
int hunger = 0;
void change_appearance(char username[], int *hero_x, int *hero_y, int *score);
void draw_color_menu(int current_selection);
void leader_board(int from_game, char username[], int hero_x, int hero_y, int score);
bool security_mode = false;
int spell_type = 0;
int weapon_selected = 0;
int frost = 0;
int hero_y, hero_x;


#define MAX_PROJECTILES 20

typedef struct {
    int x;
    int y;
    bool dropped;
    char weapon_char;
} Weapon;


struct Enemy;
typedef void (*EnemyAbility)(struct Enemy *);

typedef struct Enemy
{
    char symbol;
    int x;
    int y;
    int health;
    int damage;
    bool active;
    EnemyAbility special_ability;
    int moves_made; 
    bool can_move; 
} Enemy;

int current_hero_color = 1;
char **map;
char **temp_map;
int WIDTH, HEIGHT;

int num_rooms = 0;

typedef struct
{
    bool has_ancient_key;
    int gold_count;
    int speed_spells;
    int health_spells;
    int damage_spells;
    bool broke_key;

    int total_spells;
    int total_foods;

    bool mace;
    bool magic_wand;
    bool dagger;
    bool normal_arrow;
    bool sword;

    int dagger_count;
    int wand_count;
    int arrow_count;

    bool health_spell;
    bool speed_spell;
    bool damage_spell;
} Bag;
Bag bag;

typedef struct
{
    char username[100];
    time_t save_time;
    int score;
    int current_room;
    int current_floor;
    int current_hero_color;
} SaveMetadata;

typedef struct
{
    int x, y, width, height;
    int left_door_x, left_door_y;
    int right_door_x, right_door_y;
    int up_stair_x, up_stair_y;
    int down_stair_x, down_stair_y;
    bool is_room_nightmare;
    bool is_room_locked;
    bool is_room_old;
    bool is_room_security;
    bool is_room_door_discovered;
    bool is_room_enchanted;
    bool is_room_treasure;
    int sec_level;
    int password;
} Room;
Room rooms[6];

typedef struct
{
    char name[100];
    int count;
    int x;
    int y;
    int room;
} spells;

typedef struct
{
    char **map;
    char **temp_map;
    char **discovered_map;
    Room rooms[6];
    int num_rooms;
    bool floor_discovered;
} Floor;
Floor floors[4];

#define MAX_ENEMIES_PER_FLOOR 20
Enemy *enemies[4][MAX_ENEMIES_PER_FLOOR];
int enemy_counts[4];

void dragon_ability(Enemy *e)
{
    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            if (map[e->y + dy][e->x + dx] == '@')
            {
                health -= 15;
                mvprintw(0, 2, "Dragon breath! -15 HP");
                refresh();
            }
        }
    }
}

void frost_giant_ability(Enemy *e)
{
    if (abs(e->x - hero_x) <= 2 && abs(e->y - hero_y) <= 2)
    {
        init_pair(12, 23, COLOR_BLACK);
        attron(COLOR_PAIR(12));
        mvprintw(0, 2, "Frozen! Movement reduced");
        refresh();
    }
}

void ghost_ability(Enemy *e)
{
    int new_x = e->x + (rand() % 3) - 1;
    int new_y = e->y + (rand() % 3) - 1;

    if (new_x > 0 && new_x < WIDTH - 1 && new_y > 0 && new_y < HEIGHT - 1)
    {
        map[e->y][e->x] = temp_map[e->y][e->x]; 
        e->x = new_x;
        e->y = new_y;
        map[e->y][e->x] = 'g'; 
    }

    if (rand() % 100 < 20)
    {
        map[e->y][e->x] = '.';
        refresh();
        napms(500);
        map[e->y][e->x] = 'g';
    }
}

void serpent_ability(Enemy *e)
{
    if (abs(e->x - hero_x) <= 1 && abs(e->y - hero_y) <= 1)
    {
        health -= 5;
        mvprintw(0, 2, "Poisoned! -5 HP");
        refresh();
    }
}

void undead_ability(Enemy *e)
{
    if (e->health <= 0 && rand() % 100 < 30)
    {
        e->health = 50;
        mvprintw(0, 2, "Undead rises again!");
        refresh();
    }
}
void initialize_enemies()
{
    for (int f = 0; f < 4; f++)
    {
        for (int i = 0; i < enemy_counts[f]; i++)
        {
            if (enemies[f][i] != NULL)
            {
                free(enemies[f][i]);
                enemies[f][i] = NULL;
            }
        }
        enemy_counts[f] = 0;
    }

    for (int floor = 0; floor < 4; floor++)
    {
        int num_enemies = (floor + 1) * 3 + (rand() % 3);
        if (num_enemies > MAX_ENEMIES_PER_FLOOR)
        {
            num_enemies = MAX_ENEMIES_PER_FLOOR;
        }

        enemy_counts[floor] = num_enemies;

        for (int i = 0; i < num_enemies; i++)
        {
            enemies[floor][i] = (Enemy *)malloc(sizeof(Enemy));
            if (enemies[floor][i] == NULL)
            {
                mvprintw(0, 0, "Failed to allocate memory for enemy");
                refresh();
                return;
            }

            Enemy *e = enemies[floor][i];

            e->moves_made = 0;
            e->can_move = true;
            e->active = true;
            e->x = 0;
            e->y = 0;

            int type = rand() % 5;
            switch (type)
            {
            case 0:
                e->symbol = 'D';
                e->damage = 2;
                e->health = 5;
                e->special_ability = dragon_ability;
                break;
            case 1:
                e->symbol = 'F';
                e->damage = 4;
                e->health = 10;
                e->special_ability = frost_giant_ability;
                break;
            case 2:
                e->symbol = 'G';
                e->damage = 5;
                e->health = 15;
                e->special_ability = ghost_ability;
                break;
            case 3:
                e->symbol = 'S';
                e->damage = 6;
                e->health = 20;
                e->special_ability = serpent_ability;
                break;
            case 4:
                e->symbol = 'U';
                e->damage = 8;
                e->health = 30;
                e->special_ability = undead_ability;
                break;
            case 5: // ghost
                e->symbol = 'g';
                e->damage = 3;
                e->health = 10;
                e->special_ability = ghost_ability;
            case 6:
                e->symbol = 'R';
                e-> damage = 10;
                e-> health = 100;
            }

            int room = rand() % floors[floor].num_rooms;
            Room *r = &floors[floor].rooms[room];

            int max_attempts = 100;
            int attempts = 0;
            bool position_found = false;

            while (attempts < max_attempts)
            {
                int test_x = r->x + (rand() % (r->width - 2)) + 1;
                int test_y = r->y + (rand() % (r->height - 2)) + 1;

                if (floors[floor].map[test_y][test_x] == '.')
                {
                    e->x = test_x;
                    e->y = test_y;
                    floors[floor].map[test_y][test_x] = e->symbol;
                    position_found = true;
                    break;
                }
                attempts++;
            }

            if (!position_found)
            {
                e->active = false;
                mvprintw(0, 0, "Warning: Could not place enemy %d on floor %d", i, floor);
                refresh();
            }
        }
    }
}
void is_in_same_room(int x1, int y1, int x2, int y2, bool *same_room)
{
    *same_room = false;
    for (int i = 0; i < floors[current_floor].num_rooms; i++)
    {
        Room *r = &floors[current_floor].rooms[i];
        bool point1_in_room = (x1 >= r->x && x1 < r->x + r->width &&
                               y1 >= r->y && y1 < r->y + r->height);
        bool point2_in_room = (x2 >= r->x && x2 < r->x + r->width &&
                               y2 >= r->y && y2 < r->y + r->height);

        if (point1_in_room && point2_in_room)
        {
            *same_room = true;
            return;
        }
    }
}

void update_enemies()
{
    static int move_counter = 0;
    move_counter++;

    if (move_counter % 3 != 0)
    {
        return;
    }

    for (int i = 0; i < enemy_counts[current_floor]; i++)
    {
        Enemy *e = enemies[current_floor][i];

        if (e == NULL || !e->active)
        {
            continue;
        }

        bool same_room = false;
        is_in_same_room(e->x, e->y, hero_x, hero_y, &same_room);

        int prev_x = e->x;
        int prev_y = e->y;
        bool moved = false;

        if (same_room && !e->can_move && (e->symbol != 'g' && e->symbol != 'S'))
        {
            e->can_move = true;
            e->moves_made = 0;
        }

        bool should_pursue = (e->symbol == 'S') ||
                             (e->symbol == 'g') ||
                             (same_room && e->can_move && e->moves_made < 6);

        if (should_pursue)
        {
            int dx = 0;
            int dy = 0;

            if (hero_x > e->x)
                dx = 1;
            else if (hero_x < e->x)
                dx = -1;

            if (hero_y > e->y)
                dy = 1;
            else if (hero_y < e->y)
                dy = -1;

            bool can_move = true;
            switch (e->symbol)
            {
            case 'D': // Dragon
            case 'F': // Frost Giant
            case 'U': // Undead
            case 'G': // Giant
                can_move = e->can_move && e->moves_made < 6;
                break;
            case 'g':
            case 'S':
                can_move = true;
                break;
            }

            if (can_move)
            {
                if (dx != 0 && dy != 0)
                {
                    if (map[e->y + dy][e->x + dx] == '.')
                    {
                        e->x += dx;
                        e->y += dy;
                        moved = true;
                    }
                    else if (map[e->y][e->x + dx] == '.')
                    {
                        e->x += dx;
                        moved = true;
                    }
                    else if (map[e->y + dy][e->x] == '.')
                    {
                        e->y += dy;
                        moved = true;
                    }
                }
                else
                {
                    if (map[e->y + dy][e->x + dx] == '.' || (e->symbol == 'g'))
                    {
                        e->x += dx;
                        e->y += dy;
                        moved = true;
                    }
                }
            }
        }

        if (moved)
        {
            map[prev_y][prev_x] = '.';
            map[e->y][e->x] = e->symbol;

            if (e->symbol != 'g' && e->symbol != 'S')
            {
                e->moves_made++;
                if (e->moves_made >= 6)
                {
                    e->can_move = false;
                }
            }
        }

        if (abs(e->x - hero_x) <= 1 && abs(e->y - hero_y) <= 1 && e->active > 0)
        {
            health -= e->damage;
            mvprintw(0, 2, "Enemy attack! -%d HP", e->damage);
            refresh();

            if (rand() % 100 < 30 && e->special_ability != NULL)
            {
                e->special_ability(e);
            }
        }
        if(e->symbol == 'U') {
            if (rand() % 100 < 30 && e->special_ability != NULL)
            {
                e->special_ability(e);
            }
        }
    }
}

void draw_enemies()
{
    for (int i = 0; i < enemy_counts[current_floor]; i++)
    {
        Enemy *e = enemies[current_floor][i];
        if (e == NULL || !e->active || e->health <= 0 || e->can_move)
        {
            continue; 
        }

        if (e->y < 0 || e->y >= HEIGHT || e->x < 0 || e->x >= WIDTH)
        {
            continue; 
        }

        if (discovered_map[e->y][e->x])
        {
            mvaddch(e->y, e->x, e->symbol);
        }
    }
}



void make_room_security(int f, int i)
{
    Floor *floor = &floors[f];
    Room *room = &floors[f].rooms[i];

    int door_x = (i >= 3) ? room->left_door_x : room->right_door_x;
    int door_y = (i >= 3) ? room->left_door_y : room->right_door_y;

    room->is_room_locked = true;
    room->is_room_security = true;

    floor->map[door_y][door_x] = '@';
    floor->temp_map[door_y][door_x] = '@';

    int pass_gen_x = room->x + 1 + rand() % (room->width - 4);
    int pass_gen_y = room->y + 1 + rand() % (room->height - 4);

    floor->map[pass_gen_y][pass_gen_x] = '&';
    floor->temp_map[pass_gen_y][pass_gen_x] = '&';

    int is_room_old = rand() % 2;

    room->is_room_old = is_room_old;
    room->sec_level = 0;
}

void make_room_nightmare(int f, int r)
{
    Floor *floor = &floors[f];
    Room *room = &floor->rooms[r];

    room->is_room_nightmare = true;

    // Add more enemies specific to nightmare rooms
    for (int i = 0; i < enemy_counts[f]; i++)
    {
        Enemy *e = enemies[f][i];
        if (!e->active)
        {
            e->active = true;
            e->symbol = 'g'; 
            e->damage = 5;
            e->health = 15;
            e->special_ability = ghost_ability;

            int max_attempts = 100;
            while (max_attempts > 0)
            {
                int test_x = room->x + (rand() % (room->width - 2)) + 1;
                int test_y = room->y + (rand() % (room->height - 2)) + 1;

                if (floor->map[test_y][test_x] == '.')
                {
                    e->x = test_x;
                    e->y = test_y;
                    floor->map[test_y][test_x] = e->symbol;
                    break;
                }
                max_attempts--;
            }
        }
    }
}

void make_room_door_hidden(int f, int r)
{
    Floor *floor = &floors[f];
    Room *room = &floors[f].rooms[r];

    int door_x = (r >= 3) ? room->left_door_x : room->right_door_x;
    int door_y = (r >= 3) ? room->left_door_y : room->right_door_y;

    floor->temp_map[door_y][door_x] = '?';
    floor->map[door_y][door_x] = '?';
}

void make_spec_rooms()
{
    int num_of_security_rooms = 0;
    while (num_of_security_rooms <= 7)
    {
        int random_floor = rand() % 4;
        int random_room = rand() % 6;
        int num_of_security_rooms_in_floor = 0;
        for (int r = 0; r < 6; r++)
        {
            if (floors[random_floor].rooms[r].is_room_security && r != 5)
            {
                num_of_security_rooms++;
            }
        }
        if (num_of_security_rooms <= 1 &&
            !floors[random_floor].rooms[random_room].is_room_security && random_room != 5)
        {
            make_room_security(random_floor, random_room);
            num_of_security_rooms++;
        }
    }

    int nightmare_rooms_placed = 0;
    while (nightmare_rooms_placed < 3)
    {
        int random_floor = rand() % 4;
        int random_room = rand() % 6;
        Room *selected_room = &floors[random_floor].rooms[random_room];

        if (!selected_room->is_room_nightmare &&
            !selected_room->is_room_security &&
            !selected_room->is_room_enchanted &&
            random_room != 5)
        {

            make_room_nightmare(random_floor, random_room);
            nightmare_rooms_placed++;
        }
    }

    int enchanted_rooms_placed = 0;
    while (enchanted_rooms_placed < 2)
    {
        int random_floor = rand() % 4;
        int random_room = rand() % 6;
        Room *selected_room = &floors[random_floor].rooms[random_room];

        if (!selected_room->is_room_enchanted &&
            !selected_room->is_room_nightmare &&
            random_room != 5)
        {
            selected_room->is_room_enchanted = true;
            enchanted_rooms_placed++;

            for (int y = selected_room->y; y < selected_room->y + selected_room->height; y++)
            {
                for (int x = selected_room->x; x < selected_room->x + selected_room->width; x++)
                {
                    if (floors[random_floor].map[y][x] == '.' && floors[random_floor].map[y][x + 1] != '+' &&
                        floors[random_floor].map[y][x + 1] != '@' && floors[random_floor].map[y][x - 1] != '@' &&
                        floors[random_floor].map[y][x - 1] != '+')
                    {
                        // if (rand() % 10 == 0)
                        // {
                        //     floors[random_floor].map[y][x] = '*';
                        //     floors[random_floor].temp_map[y][x] = '*';
                        // }
                    }
                }
            }
        }
    }

    make_room_security(0, 0);
    // int num_of_nightmare_rooms = 0;


    // make_room_door_hidden(0, 0);
}
