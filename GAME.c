#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define BOARD_SIZE 7
#define SHIP_TYPES 5

// Platform-independent color detection
#define enable_colors() 1

// macOS-specific sound implementation
void play_sound_hit() {
    system("afplay /System/Library/Sounds/Blow.aiff 2>/dev/null");
}

void play_sound_miss() {
    system("afplay /System/Library/Sounds/Sosumi.aiff 2>/dev/null");
}

void play_sound_sunk() {
    system("afplay /System/Library/Sounds/Basso.aiff 2>/dev/null");
}

void play_sound_win() {
    system("afplay /System/Library/Sounds/Glass.aiff 2>/dev/null");
}

// Sound enable/disable flag
static int sound_enabled = 1;

// Color codes
static int colors_enabled = 1;
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_RESET   "\033[0m"

// Emoji symbols for ships, hits, and misses
#define EMOJI_HIT     "💥"  // Explosion for hit
#define EMOJI_MISS    "💧"  // Splash for miss
#define EMOJI_SHIP    "🚢"  // Ship
#define EMOJI_EMPTY   "🌊"  // Wave for empty water
#define EMOJI_SUNK    "💀"  // Skull for sunk ship

// Ship definitions
typedef struct {
    char name[20];
    int length;
    int hits;
    int sunk;
} Ship;

// Game board cell
typedef struct {
    int has_ship;
    int is_revealed;
    Ship *ship;
} Cell;

// Player structure
typedef struct {
    Cell board[BOARD_SIZE][BOARD_SIZE];
    Ship ships[SHIP_TYPES];
    int ships_remaining;
    int is_computer;
} Player;

// Game structure
typedef struct {
    Player player1;
    Player player2;
    int current_player;
    int game_over;
    int game_mode;
} Game;

// Global variable for best (fastest) time
double best_time_pvc = -1; // Player vs Computer
double best_time_cvc = -1; // Computer vs Computer

// Function prototypes
void initialize_game(Game *game, int game_mode);
void setup_board(Player *player, int show_setup);
void place_ships(Player *player);
void print_board_with_grid(Player *player, int show_ships);
void print_opponent_board(Player *opponent);
int is_valid_placement(Player *player, int ship_index, int x, int y, int direction);
void place_ship(Player *player, int ship_index, int x, int y, int direction);
int take_shot(Player *attacker, Player *defender, int x, int y);
void player_turn(Game *game);
void computer_turn_enhanced(Game *game);
int is_game_over(Player *player);
void print_ship_status(Player *player);
int get_valid_coordinates(int *x, int *y);
void clear_input_buffer();
void show_menu();
void start_player_vs_computer(Game *game);
void start_computer_vs_computer(Game *game);
void toggle_sound();
void test_sounds();
void play_victory_sound();

// Ship types
Ship ship_types[SHIP_TYPES] = {
    {"Carrier", 5, 0, 0},
    {"Battleship", 4, 0, 0},
    {"Cruiser", 3, 0, 0},
    {"Submarine", 3, 0, 0},
    {"Destroyer", 2, 0, 0}
};

// Get the appropriate symbol (macOS supports emojis well)
const char* get_hit_symbol() { return EMOJI_HIT; }
const char* get_miss_symbol() { return EMOJI_MISS; }
const char* get_ship_symbol() { return EMOJI_SHIP; }
const char* get_empty_symbol() { return EMOJI_EMPTY; }
const char* get_sunk_symbol() { return EMOJI_SUNK; }

void toggle_sound() {
    sound_enabled = !sound_enabled;
    printf("Sound %s\n", sound_enabled ? "🔊 ENABLED" : "🔇 DISABLED");
}

void test_sounds() {
    printf("Testing macOS sounds...\n");
    
    printf("Hit sound (Blow.aiff): ");
    if (sound_enabled) play_sound_hit();
    printf("💥 BOOM!\n");
    
    usleep(500000); // 0.5 second delay
    
    printf("Miss sound (Sosumi.aiff): ");
    if (sound_enabled) play_sound_miss();
    printf("💧 SPLASH!\n");
    
    usleep(500000);
    
    printf("Sunk sound (Basso.aiff): ");
    if (sound_enabled) play_sound_sunk();
    printf("💀 SUNK!\n");
    
    usleep(500000);
    
    printf("Victory sound (Glass.aiff): ");
    if (sound_enabled) play_sound_win();
    printf("🏆 VICTORY!\n");
}

void play_victory_sound() {
    if (sound_enabled) {
        play_sound_win();
        usleep(300000);
        play_sound_win();
        usleep(200000);
        play_sound_win();
    }
}

int main() {
    srand((unsigned int)time(NULL));
    Game game;
    int choice;

    printf("=== 🚢 BATTLESHIP GAME 💥 ===\n\n");
    printf("🎮 Designed for macOS 🍎\n");
    printf("🔊 Sound is currently: %s\n", sound_enabled ? "ON" : "OFF");
    printf("🎵 Using macOS system sounds for maximum compatibility\n\n");

    do {
        show_menu();
        printf("Enter your choice (1-5): ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch(choice) {
            case 1:
                initialize_game(&game, 1);
                start_player_vs_computer(&game);
                break;
            case 2:
                initialize_game(&game, 2);
                start_computer_vs_computer(&game);
                break;
            case 3:
                toggle_sound();
                break;
            case 4:
                test_sounds();
                break;
            case 5:
                printf("Thanks for playing! 👋\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 5);

    return 0;
}

void show_menu() {
    printf("\n=== 🎯 MAIN MENU 🎯 ===\n");
    printf("1. 🧍 Player vs Computer 🤖\n");
    printf("2. 🤖 Computer vs Computer 🤖 (Demo)\n");
    printf("3. %s Toggle Sound\n", sound_enabled ? "🔇" : "🔊");
    printf("4. 🔈 Test Sounds\n");
    printf("5. 🚪 Exit\n");
}

void initialize_game(Game *game, int game_mode) {
    for (int p = 0; p < 2; p++) {
        Player *player = (p == 0) ? &game->player1 : &game->player2;

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                player->board[i][j].has_ship = 0;
                player->board[i][j].is_revealed = 0;
                player->board[i][j].ship = NULL;
            }
        }

        for (int i = 0; i < SHIP_TYPES; i++) {
            player->ships[i] = ship_types[i];
        }

        player->ships_remaining = SHIP_TYPES;
        player->is_computer = (game_mode == 2 || p == 1);
    }

    game->current_player = 1;
    game->game_over = 0;
    game->game_mode = game_mode;
}

void setup_board(Player *player, int show_setup) {
    place_ships(player);
    if (show_setup) {
        printf("✅ Board setup complete!\n");
        print_board_with_grid(player, 1);
    } else {
        printf("✅ Computer board setup complete!\n");
        // Don't show computer's ship placement
    }
}

void place_ships(Player *player) {
    for (int i = 0; i < SHIP_TYPES; i++) {
        int x, y, direction, placed = 0;

        if (player->is_computer) {
            do {
                x = rand() % BOARD_SIZE;
                y = rand() % BOARD_SIZE;
                direction = rand() % 2;
            } while (!is_valid_placement(player, i, x, y, direction));

            place_ship(player, i, x, y, direction);
        } else {
            printf("\nYour current board:\n");
            print_board_with_grid(player, 1);
            printf("Placing %s %s (length: %d)\n", player->ships[i].name, get_ship_symbol(), player->ships[i].length);

            do {
                printf("Enter starting coordinates (e.g., A5): ");
                if (!get_valid_coordinates(&x, &y)) continue;

                printf("Enter direction (0 for horizontal, 1 for vertical): ");
                if (scanf("%d", &direction) != 1) {
                    printf("Invalid direction! Try again.\n");
                    clear_input_buffer();
                    continue;
                }
                clear_input_buffer();

                if (is_valid_placement(player, i, x, y, direction)) {
                    place_ship(player, i, x, y, direction);
                    placed = 1;
                } else {
                    printf("Invalid placement! Try again.\n");
                }
            } while (!placed);
        }
    }
}

int is_valid_placement(Player *player, int ship_index, int x, int y, int direction) {
    int length = player->ships[ship_index].length;
    if (direction == 0) {
        if (x + length > BOARD_SIZE) return 0;
        for (int i = 0; i < length; i++)
            if (player->board[y][x + i].has_ship) return 0;
    } else {
        if (y + length > BOARD_SIZE) return 0;
        for (int i = 0; i < length; i++)
            if (player->board[y + i][x].has_ship) return 0;
    }
    return 1;
}

void place_ship(Player *player, int ship_index, int x, int y, int direction) {
    int length = player->ships[ship_index].length;
    for (int i = 0; i < length; i++) {
        if (direction == 0) {
            player->board[y][x + i].has_ship = 1;
            player->board[y][x + i].ship = &player->ships[ship_index];
        } else {
            player->board[y + i][x].has_ship = 1;
            player->board[y + i][x].ship = &player->ships[ship_index];
        }
    }
}

void print_board_with_grid(Player *player, int show_ships) {
    printf("\n   ");
    // Print column headers (A-J)
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("  %c ", 'A' + i);
    }
    printf("\n");
    
    // Print top border
    printf("   +");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("---+");
    }
    printf("\n");
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        // Print row number with leading zero for single digits
        printf("%2d |", i + 1);
        
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (player->board[i][j].is_revealed) {
                if (player->board[i][j].has_ship) {
                    printf("💥 |");
                } else {
                    printf("💧 |");
                }
            } else if (show_ships && player->board[i][j].has_ship) {
                printf("🚢 |");
            } else {
                printf("🌊 |");
            }
        }
        printf("\n");
        
        // Print inner grid lines
        printf("   +");
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("---+");
        }
        printf("\n");
    }
    
    print_ship_status(player);
}

void print_opponent_board(Player *opponent) {
    printf("Opponent's Board:\n");
    print_board_with_grid(opponent, 0);  // Never show opponent's ships
}

void print_ship_status(Player *player) {
    printf("\nShip Status:\n");
    for (int i = 0; i < SHIP_TYPES; i++) {
        if (player->ships[i].hits >= player->ships[i].length) {
            printf("%s💀 SUNK! - %s%s\n", COLOR_RED, player->ships[i].name, COLOR_RESET);
        } else if (player->ships[i].hits > 0) {
            printf("%s🔥 HIT - %s: %d/%d hits%s\n", COLOR_YELLOW, player->ships[i].name,
                   player->ships[i].hits, player->ships[i].length, COLOR_RESET);
        } else {
            printf("%s✅ OK - %s: %d/%d hits%s\n", COLOR_GREEN, player->ships[i].name,
                   player->ships[i].hits, player->ships[i].length, COLOR_RESET);
        }
    }
    printf("\n");
}

int take_shot(Player *attacker, Player *defender, int x, int y) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return -1;
    if (defender->board[y][x].is_revealed) return 0;

    defender->board[y][x].is_revealed = 1;
    if (defender->board[y][x].has_ship) {
        defender->board[y][x].ship->hits++;
        
        // Check if ship is sunk
        if (defender->board[y][x].ship->hits == defender->board[y][x].ship->length) {
            defender->ships_remaining--;
            // Play sunk sound
            if (sound_enabled) play_sound_sunk();
        } else {
            // Play hit sound
            if (sound_enabled) play_sound_hit();
        }
        return 2;
    }
    
    // Play miss sound
    if (sound_enabled) play_sound_miss();
    return 1;
}

void player_turn(Game *game) {
    Player *attacker = &game->player1;
    Player *defender = &game->player2;
    int x, y, result;
    do {
        printf("Enter target coordinates (e.g., B4): ");
        if (!get_valid_coordinates(&x, &y)) continue;
        result = take_shot(attacker, defender, x, y);
        if (result == -1) printf("Invalid coordinates!\n");
        else if (result == 0) printf("Already shot!\n");
        else if (result == 1) printf("%s%s Miss! %s%s\n", COLOR_YELLOW, get_miss_symbol(), get_miss_symbol(), COLOR_RESET);
        else printf("%s%s Hit! %s%s\n", COLOR_RED, get_hit_symbol(), get_hit_symbol(), COLOR_RESET);
    } while (result <= 0);

    printf("\nAfter your shot:\n");
    print_opponent_board(defender);
}

void computer_turn_enhanced(Game *game) {
    static int last_hit_x = -1, last_hit_y = -1;
    static int hunt_mode = 0, direction = 0, attempts = 0;

    Player *attacker = (game->current_player == 1) ? &game->player1 : &game->player2;
    Player *defender = (game->current_player == 1) ? &game->player2 : &game->player1;

    int x, y, result, valid_shot = 0;
    if (hunt_mode && attempts < 4) {
        int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        do {
            x = last_hit_x + dirs[direction][0];
            y = last_hit_y + dirs[direction][1];
            if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE &&
                !defender->board[y][x].is_revealed) valid_shot = 1;
            else direction = (direction + 1) % 4, attempts++;
        } while (!valid_shot && attempts < 4);
    }

    if (!valid_shot) {
        do { x = rand() % BOARD_SIZE; y = rand() % BOARD_SIZE; }
        while (defender->board[y][x].is_revealed);
        hunt_mode = 0; attempts = 0;
    }

    result = take_shot(attacker, defender, x, y);
    printf("Computer shot at %c%d: ", 'A' + x, y + 1);
    if (result == 1) printf("%s%s Miss! %s%s\n", COLOR_YELLOW, get_miss_symbol(), get_miss_symbol(), COLOR_RESET);
    else if (result == 2) {
        printf("%s%s Hit! %s%s\n", COLOR_RED, get_hit_symbol(), get_hit_symbol(), COLOR_RESET);
        if (!hunt_mode) hunt_mode = 1, last_hit_x = x, last_hit_y = y;
    }

    if (game->game_mode == 2) {
        print_board_with_grid(defender, 1);  // Show ships in demo mode
    } else {
        print_opponent_board(defender);      // Don't show ships in PvC mode
    }
}

int is_game_over(Player *player) {
    return player->ships_remaining == 0;
}

int get_valid_coordinates(int *x, int *y) {
    char input[10];
    if (scanf("%9s", input) != 1) {
        clear_input_buffer(); return 0;
    }
    clear_input_buffer();
    *x = toupper(input[0]) - 'A';
    *y = atoi(input + 1) - 1;
    if (*x < 0 || *x >= BOARD_SIZE || *y < 0 || *y >= BOARD_SIZE) {
        printf("Out of bounds!\n"); return 0;
    }
    return 1;
}

void clear_input_buffer() {
    int c; while ((c = getchar()) != '\n' && c != EOF);
}

void start_player_vs_computer(Game *game) {
    printf("\n=== 🧍 PLAYER BOARD SETUP ===\n");
    setup_board(&game->player1, 1);

    printf("\n=== 🤖 COMPUTER BOARD SETUP ===\n");
    setup_board(&game->player2, 0);

    clock_t start_time = clock();

    game->current_player = 1;

    while (!game->game_over) {
        if (game->current_player == 1) {
            printf("\n=== 🎯 YOUR TURN ===\n");
            print_opponent_board(&game->player2);
            player_turn(game);
        } else {
            printf("\n=== 🤖 COMPUTER'S TURN ===\n");
            computer_turn_enhanced(game);
        }
        game->game_over = is_game_over(&game->player1) || is_game_over(&game->player2);
        game->current_player = 3 - game->current_player;
    }

    clock_t end_time = clock();
    double duration = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("\n=== 🏁 GAME OVER 🏁 ===\n");

    printf("\nFinal Player Board:\n");
    print_board_with_grid(&game->player1, 1);
    printf("\nFinal Computer Board:\n");
    print_board_with_grid(&game->player2, 1);

    if (is_game_over(&game->player1)) {
        printf("%s💻 Computer wins! 💀%s\n", COLOR_RED, COLOR_RESET);
    } else {
        printf("%s🎉 You win! 🏆%s\n", COLOR_GREEN, COLOR_RESET);
        play_victory_sound();
    }

    printf("⏱️  Game duration: %s%.2f seconds%s\n", COLOR_CYAN, duration, COLOR_RESET);

    if (best_time_pvc < 0) {
        best_time_pvc = duration;
        printf("%s🏆 First High Score (PvC): %.2f seconds!%s\n", COLOR_GREEN, best_time_pvc, COLOR_RESET);
    } else if (duration < best_time_pvc) {
        printf("%s🎯 New High Score! Previous: %.2fs, New: %.2fs%s\n", COLOR_YELLOW, best_time_pvc, duration, COLOR_RESET);
        best_time_pvc = duration;
    } else {
        printf("%s📊 Current High Score (PvC): %.2f seconds%s\n", COLOR_BLUE, best_time_pvc, COLOR_RESET);
        printf("%s⏰ Your time: %.2f seconds (%.2f seconds slower)%s\n", COLOR_CYAN, duration, duration - best_time_pvc, COLOR_RESET);
    }

    printf("Press Enter to continue...");
    clear_input_buffer();
    getchar();
}

void start_computer_vs_computer(Game *game) {
    printf("\n=== 🤖 COMPUTER 1 BOARD SETUP ===\n");
    setup_board(&game->player1, 1);
    printf("\n=== 🤖 COMPUTER 2 BOARD SETUP ===\n");
    setup_board(&game->player2, 1);

    clock_t start_time = clock();

    game->current_player = 1;

    while (!game->game_over) {
        printf("\n=== 🤖 COMPUTER %d'S TURN ===\n", game->current_player);
        computer_turn_enhanced(game);
        game->game_over = is_game_over(&game->player1) || is_game_over(&game->player2);
        game->current_player = 3 - game->current_player;
        printf("Press Enter to continue...");
        clear_input_buffer();
        getchar();
    }

    clock_t end_time = clock();
    double duration = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("\n=== 🏁 GAME OVER 🏁 ===\n");
    if (is_game_over(&game->player1)) {
        printf("%s🤖 Computer 2 wins! 💀%s\n", COLOR_RED, COLOR_RESET);
    } else {
        printf("%s🤖 Computer 1 wins! 🏆%s\n", COLOR_GREEN, COLOR_RESET);
        play_victory_sound();
    }

    printf("⏱️  Game duration: %s%.2f seconds%s\n", COLOR_CYAN, duration, COLOR_RESET);

    if (best_time_cvc < 0) {
        best_time_cvc = duration;
        printf("%s🏆 First High Score (CvsC): %.2f seconds!%s\n", COLOR_GREEN, best_time_cvc, COLOR_RESET);
    } else if (duration < best_time_cvc) {
        printf("%s🎯 New High Score! Previous: %.2fs, New: %.2fs%s\n", COLOR_YELLOW, best_time_cvc, duration, COLOR_RESET);
        best_time_cvc = duration;
    } else {
        printf("%s📊 Current High Score (CvsC): %.2f seconds%s\n", COLOR_BLUE, best_time_cvc, COLOR_RESET);
        printf("%s⏰ This game: %.2f seconds (%.2f seconds slower)%s\n", COLOR_CYAN, duration, duration - best_time_cvc, COLOR_RESET);
    }

    printf("Press Enter to continue...");
    clear_input_buffer();
    getchar();
}