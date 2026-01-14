#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define BOARD_SIZE 10
#define SHIP_TYPES 5

// Ship definitions
typedef struct {
    char name[20];
    int length;
    int hits;
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

// Function prototypes
void initialize_game(Game *game, int game_mode);
void setup_board(Player *player);
void place_ships(Player *player);
void print_board_with_grid(Player *player, int show_ships);
void print_opponent_board(Player *opponent);
int is_valid_placement(Player *player, int ship_index, int x, int y, int direction);
void place_ship(Player *player, int ship_index, int x, int y, int direction);
int take_shot(Player *attacker, Player *defender, int x, int y);
void player_turn(Game *game);
void computer_turn(Game *game);
void computer_turn_enhanced(Game *game);
int is_game_over(Player *player);
void print_ship_status(Player *player);
int get_valid_coordinates(int *x, int *y);
void clear_input_buffer();
void show_menu();
void start_player_vs_computer(Game *game);
void start_computer_vs_computer(Game *game);

// Ship types
Ship ship_types[SHIP_TYPES] = {
    {"Carrier", 5, 0},
    {"Battleship", 4, 0},
    {"Cruiser", 3, 0},
    {"Submarine", 3, 0},
    {"Destroyer", 2, 0}
};

int main() {
    srand(time(NULL));
    Game game;
    int choice;
    
    printf("=== BATTLESHIP GAME ===\n\n");
    
    do {
        show_menu();
        printf("Enter your choice (1-3): ");
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
                printf("Thanks for playing!\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 3);
    
    return 0;
}

void show_menu() {
    printf("\n=== MAIN MENU ===\n");
    printf("1. Player vs Computer\n");
    printf("2. Computer vs Computer (Demo)\n");
    printf("3. Exit\n");
}

void start_player_vs_computer(Game *game) {
    printf("\n=== PLAYER BOARD SETUP ===\n");
    setup_board(&game->player1);
    
    printf("\n=== COMPUTER BOARD SETUP ===\n");
    setup_board(&game->player2);
    
    game->current_player = 1;
    
    while (!game->game_over) {
        if (game->current_player == 1) {
            printf("\n=== YOUR TURN ===\n");
            print_opponent_board(&game->player2);
            player_turn(game);
        } else {
            printf("\n=== COMPUTER'S TURN ===\n");
            computer_turn_enhanced(game);
        }
        
        game->game_over = is_game_over(&game->player1) || is_game_over(&game->player2);
        game->current_player = 3 - game->current_player;
    }
    
    // Game over
    printf("\n=== GAME OVER ===\n");
    if (is_game_over(&game->player1)) {
        printf("Computer wins! All your ships have been sunk.\n");
    } else {
        printf("You win! All computer ships have been sunk.\n");
    }
    
    printf("Press Enter to continue...");
    getchar();
}

void start_computer_vs_computer(Game *game) {
    printf("\n=== COMPUTER 1 BOARD SETUP ===\n");
    setup_board(&game->player1);
    
    printf("\n=== COMPUTER 2 BOARD SETUP ===\n");
    setup_board(&game->player2);
    
    game->current_player = 1;
    
    while (!game->game_over) {
        printf("\n=== COMPUTER %d'S TURN ===\n", game->current_player);
        computer_turn_enhanced(game);
        
        game->game_over = is_game_over(&game->player1) || is_game_over(&game->player2);
        game->current_player = 3 - game->current_player;
        
        // Pause for demonstration
        printf("Press Enter to continue...");
        getchar();
    }
    
    printf("\n=== GAME OVER ===\n");
    if (is_game_over(&game->player1)) {
        printf("Computer 2 wins!\n");
    } else {
        printf("Computer 1 wins!\n");
    }
    
    printf("Press Enter to continue...");
    getchar();
}

void initialize_game(Game *game, int game_mode) {
    // Initialize both players
    for (int p = 0; p < 2; p++) {
        Player *player = (p == 0) ? &game->player1 : &game->player2;
        
        // Initialize board
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                player->board[i][j].has_ship = 0;
                player->board[i][j].is_revealed = 0;
                player->board[i][j].ship = NULL;
            }
        }
        
        // Initialize ships
        for (int i = 0; i < SHIP_TYPES; i++) {
            player->ships[i] = ship_types[i];
        }
        
        player->ships_remaining = SHIP_TYPES;
        
        // Set computer flag based on game mode and player
        if (game_mode == 1) {
            player->is_computer = (p == 1);
        } else {
            player->is_computer = 1;
        }
    }
    
    game->game_over = 0;
    game->game_mode = game_mode;
}

void setup_board(Player *player) {
    place_ships(player);
    printf("Board setup complete!\n");
    print_board_with_grid(player, 1);
}

void place_ships(Player *player) {
    for (int i = 0; i < SHIP_TYPES; i++) {
        int x, y, direction;
        int placed = 0;
        
        if (player->is_computer) {
            do {
                x = rand() % BOARD_SIZE;
                y = rand() % BOARD_SIZE;
                direction = rand() % 2;
            } while (!is_valid_placement(player, i, x, y, direction));
            
            place_ship(player, i, x, y, direction);
            printf("Computer placed %s\n", player->ships[i].name);
        } else {
            printf("\nYour current board:\n");
            print_board_with_grid(player, 1);
            
            printf("Placing %s (length: %d)\n", player->ships[i].name, player->ships[i].length);
            
            do {
                printf("Enter starting coordinates (e.g., A5): ");
                if (!get_valid_coordinates(&x, &y)) {
                    continue;
                }
                
                printf("Enter direction (0 for horizontal, 1 for vertical): ");
                if (scanf("%d", &direction) != 1) {
                    printf("Invalid direction! Try again.\n");
                    clear_input_buffer();
                    continue;
                }
                clear_input_buffer();
                
                if (direction != 0 && direction != 1) {
                    printf("Direction must be 0 (horizontal) or 1 (vertical)! Try again.\n");
                    continue;
                }
                
                if (is_valid_placement(player, i, x, y, direction)) {
                    place_ship(player, i, x, y, direction);
                    placed = 1;
                } else {
                    printf("Invalid placement! Ship would be out of bounds or overlap another ship.\n");
                }
            } while (!placed);
        }
    }
}

int is_valid_placement(Player *player, int ship_index, int x, int y, int direction) {
    int length = player->ships[ship_index].length;
    
    if (direction == 0) {
        if (x + length > BOARD_SIZE) return 0;
        for (int i = 0; i < length; i++) {
            if (player->board[y][x + i].has_ship) return 0;
        }
    } else {
        if (y + length > BOARD_SIZE) return 0;
        for (int i = 0; i < length; i++) {
            if (player->board[y + i][x].has_ship) return 0;
        }
    }
    
    return 1;
}

void place_ship(Player *player, int ship_index, int x, int y, int direction) {
    int length = player->ships[ship_index].length;
    
    if (direction == 0) {
        for (int i = 0; i < length; i++) {
            player->board[y][x + i].has_ship = 1;
            player->board[y][x + i].ship = &player->ships[ship_index];
        }
    } else {
        for (int i = 0; i < length; i++) {
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
                    printf(" X |");
                } else {
                    printf(" O |");
                }
            } else if (show_ships && player->board[i][j].has_ship) {
                printf(" S |");
            } else {
                printf("   |");
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
    print_board_with_grid(opponent, 0);
}

void print_ship_status(Player *player) {
    printf("\nShip Status:\n");
    for (int i = 0; i < SHIP_TYPES; i++) {
        if (player->ships[i].hits >= player->ships[i].length) {
            printf("%s: SUNK!\n", player->ships[i].name);
        } else {
            printf("%s: %d/%d hits\n", player->ships[i].name, 
                   player->ships[i].hits, player->ships[i].length);
        }
    }
    printf("\n");
}

int take_shot(Player *attacker, Player *defender, int x, int y) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
        return -1;
    }
    
    if (defender->board[y][x].is_revealed) {
        return 0;
    }
    
    defender->board[y][x].is_revealed = 1;
    
    if (defender->board[y][x].has_ship) {
        defender->board[y][x].ship->hits++;
        
        if (defender->board[y][x].ship->hits == defender->board[y][x].ship->length) {
            defender->ships_remaining--;
            printf("You sunk the %s!\n", defender->board[y][x].ship->name);
        }
        
        return 2;
    }
    
    return 1;
}

void player_turn(Game *game) {
    Player *attacker = &game->player1;
    Player *defender = &game->player2;
    int x, y, result;
    
    do {
        printf("Enter target coordinates (e.g., B4): ");
        if (!get_valid_coordinates(&x, &y)) {
            continue;
        }
        
        result = take_shot(attacker, defender, x, y);
        
        if (result == -1) {
            printf("Invalid coordinates! Try again.\n");
        } else if (result == 0) {
            printf("You already shot there! Try again.\n");
        } else if (result == 1) {
            printf("Miss!\n");
        } else {
            printf("Hit!\n");
        }
    } while (result <= 0);
    
    printf("\nAfter your shot:\n");
    print_opponent_board(defender);
}

void computer_turn(Game *game) {
    Player *attacker = (game->current_player == 1) ? &game->player1 : &game->player2;
    Player *defender = (game->current_player == 1) ? &game->player2 : &game->player1;
    int x, y, result;
    
    do {
        x = rand() % BOARD_SIZE;
        y = rand() % BOARD_SIZE;
        result = take_shot(attacker, defender, x, y);
    } while (result == 0);
    
    printf("Computer shot at %c%d: ", 'A' + x, y + 1);
    if (result == 1) {
        printf("Miss!\n");
    } else {
        printf("Hit!\n");
    }
    
    printf("\nAfter computer's shot:\n");
    if (game->current_player == 1) {
        print_board_with_grid(defender, 1);
    } else {
        print_board_with_grid(defender, 1);
    }
}

void computer_turn_enhanced(Game *game) {
    static int last_hit_x = -1, last_hit_y = -1;
    static int hunt_mode = 0;
    static int direction = 0;
    static int attempts = 0;
    
    Player *attacker = (game->current_player == 1) ? &game->player1 : &game->player2;
    Player *defender = (game->current_player == 1) ? &game->player2 : &game->player1;
    
    int x, y, result;
    int valid_shot = 0;
    
    if (hunt_mode && attempts < 4) {
        // Continue hunting around the last hit
        int directions[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        
        do {
            x = last_hit_x + directions[direction][0];
            y = last_hit_y + directions[direction][1];
            
            if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && 
                !defender->board[y][x].is_revealed) {
                valid_shot = 1;
            } else {
                direction = (direction + 1) % 4;
                attempts++;
            }
        } while (!valid_shot && attempts < 4);
    }
    
    if (!valid_shot) {
        // Random shot
        do {
            x = rand() % BOARD_SIZE;
            y = rand() % BOARD_SIZE;
        } while (defender->board[y][x].is_revealed);
        hunt_mode = 0;
        attempts = 0;
    }
    
    result = take_shot(attacker, defender, x, y);
    
    printf("Computer shot at %c%d: ", 'A' + x, y + 1);
    if (result == 1) {
        printf("Miss!\n");
        if (hunt_mode) {
            direction = (direction + 1) % 4;
            attempts++;
        }
    } else if (result == 2) {
        printf("Hit!\n");
        if (!hunt_mode) {
            hunt_mode = 1;
            last_hit_x = x;
            last_hit_y = y;
            direction = 0;
            attempts = 0;
        }
    }
    
    printf("\nAfter computer's shot:\n");
    print_board_with_grid(defender, game->current_player == 2 || game->game_mode == 2);
}

int is_game_over(Player *player) {
    return player->ships_remaining == 0;
}

int get_valid_coordinates(int *x, int *y) {
    char input[10];
    
    if (scanf("%9s", input) != 1) {
        printf("Error reading input.\n");
        clear_input_buffer();
        return 0;
    }
    clear_input_buffer();
    
    if (strlen(input) < 2) {
        printf("Invalid input! Format should be like A1, B2, etc.\n");
        return 0;
    }
    
    *x = toupper(input[0]) - 'A';
    
    // Handle multi-digit numbers
    *y = 0;
    for (int i = 1; i < strlen(input); i++) {
        if (input[i] >= '0' && input[i] <= '9') {
            *y = *y * 10 + (input[i] - '0');
        } else {
            break;
        }
    }
    *y = *y - 1;
    
    if (*x < 0 || *x >= BOARD_SIZE || *y < 0 || *y >= BOARD_SIZE) {
        printf("Coordinates out of bounds! Use A-J and 1-10.\n");
        return 0;
    }
    
    return 1;
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}