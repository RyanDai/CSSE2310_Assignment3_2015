#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_DECKS_NUM 1000

struct Card {
    char rank;
    char suit;
};
typedef struct Card Card;


struct Player {
    FILE *send;
    FILE *receive;
    pid_t id;
};

typedef struct Player Player; 

struct Game {
    int score;
    Card cards[MAX_DECKS_NUM][52];
    Card aCards[26];
    Card bCards[26];
    Card cCards[26];
    Card dCards[26];
    Card lastPlayedCard;
    Card oneTrickCards[4];
    Card playedCards[52];
    int scores[4];
    int handCard;
    int playedCard;
    int diamondTwoRemovedFlag;
    int deck;
    int totalDeck;
    int totalCard;
    int numOfPlayer;
    Player player[4];
    int currentPlayer;
    char leadSuit;
    
};

typedef struct Game Game;

/*
 * Function: remove_newline
 *
 * remove the new line("\n") from deck
 *
 * line: the line need to be changed
 *
 * return void
 */ 
void remove_newline(char *line) {
    int newLine = strlen(line) - 1;
    if (line[newLine] == '\n') {
        line[newLine] = '\0';
    }
}

/*
 * Function: remove_comma
 *
 * remove the last comma(",") from deck
 *
 * line: the line need to be changed
 *
 * return void
 */ 
void remove_comma(char *line) {
    int newLine = strlen(line) - 1;
    if(line[newLine] == ',') {
        line[newLine] = '\0';
    }
}

/*
 * Function: read_card_rank
 *
 * read card's rank, transfer "T,J,Q,K,A" to 10,11,12,13,14
 *
 * card: the card need to be read
 *
 * return the value of card rank or 0 is value is invalid
 */ 
int read_card_rank(char card) {
    int i;
    
    char allCards[13] = {'2', '3', '4', '5', '6', '7', '8', '9', 
            'T', 'J', 'Q', 'K', 'A'};
    for(i = 0; i < 13; ++i) {
        if(card == allCards[i]) {
            return i + 2;
        }
    }
    return 0;
    
}

/*
 * Function: read_card_suit
 *
 * read card's suit
 *
 * card: the card need to be read
 *
 * return the suit or 0 if suit is invalid
 */ 
char read_card_suit(char card) {
    int i;
    char allCards[4] = {'H', 'D', 'C', 'S'};
    for(i = 0; i < 4; ++i) {
        if(card == allCards[i]) {
            return card;
        }
    }
    return 0;
    
}

/*
 * Function: read_one_line
 *
 * read one line content of deck
 *
 * g: struct Game
 * message: the one line need to be read
 *
 * return void
 */ 
void read_one_line(Game *g, char *message) {
    char *ptr;
    ptr = message;
    int cardRank;
    char cardSuit;
    
    
    while(1) {
        g->cards[g->totalDeck][g->totalCard].rank = *ptr;
        
        if((cardRank = read_card_rank(*ptr)) == 0 || *(++ptr) == '\0') {
            fprintf(stderr, "Error reading deck\n");
            fflush(stderr);
            exit(4);
        }
        g->cards[g->totalDeck][g->totalCard].suit = *ptr;
        if((cardSuit = read_card_suit(*ptr)) == 0) {
            fprintf(stderr, "Error reading deck\n");
            fflush(stderr);
            exit(4);
        }
        if(*(++ptr) != ',' && *ptr != '\0') {
            fprintf(stderr, "Error reading deck\n");
            fflush(stderr);
            exit(4);
        }
        
        g->totalCard++;
        if(*ptr++ == '\0') {
            break;
        }   
    }
}

/*
 * Function: divide_cards_two_ppl
 *
 * seperate 52 cards to 2 people
 *
 * g: struct Game
 *
 * return void
 */ 
void divide_cards_two_ppl(Game *g) {
    int i;
    int j;
    for(i = 0, j = 0; i < g->totalCard; i += 2, j++) {
        g->aCards[j].rank = g->cards[g->deck][i].rank;
        g->aCards[j].suit = g->cards[g->deck][i].suit;  
    }
    for(i = 1, j = 0; i < g->totalCard; i += 2, j++) {
        g->bCards[j].rank = g->cards[g->deck][i].rank;
        g->bCards[j].suit = g->cards[g->deck][i].suit;  
    }
}

/*
 * Function: divide_cards_four_ppl
 *
 * seperate 52 cards to 4 people
 *
 * g: struct Game
 *
 * return void
 */ 
void divide_cards_four_ppl(Game *g) {
    int i;
    int j;
    for(i = 0, j = 0; i < g->totalCard; i += 4, j++) {
        g->aCards[j].rank = g->cards[g->deck][i].rank;
        g->aCards[j].suit = g->cards[g->deck][i].suit;  
    }
    for(i = 1, j = 0; i < g->totalCard; i += 4, j++) {
        g->bCards[j].rank = g->cards[g->deck][i].rank;
        g->bCards[j].suit = g->cards[g->deck][i].suit;  
    }
    for(i = 2, j = 0; i < g->totalCard; i += 4, j++) {
        g->cCards[j].rank = g->cards[g->deck][i].rank;
        g->cCards[j].suit = g->cards[g->deck][i].suit;  
    }
    for(i = 3, j = 0; i < g->totalCard; i += 4, j++) {
        g->dCards[j].rank = g->cards[g->deck][i].rank;
        g->dCards[j].suit = g->cards[g->deck][i].suit;  
    }
}

/*
 * Function: divide_cards
 *
 * if number of player is 3, remove 2D and seperate 51 cards to 3 players
 *
 * g: struct Game
 *
 * return void
 */ 
void divide_cards(Game *g) {
    int i;
    int j;
    int pos2D;
    if(g->numOfPlayer == 2) {
        divide_cards_two_ppl(g);
    }
    if(g->numOfPlayer == 4) {
        divide_cards_four_ppl(g);
    }
    
    if(g->numOfPlayer == 3) {
        if(g->diamondTwoRemovedFlag == 0) { 
            for(i = 0; i < g->totalCard; ++i) {
                if(g->cards[g->deck][i].rank == '2' && 
                        g->cards[g->deck][i].suit == 'D') {
                    pos2D = i;
                }
            }
            for(i = pos2D; i < g->totalCard - 1; ++i) {
                g->cards[g->deck][i].rank = g->cards[g->deck][i + 1].rank;
                g->cards[g->deck][i].suit = g->cards[g->deck][i + 1].suit;
            }
            g->cards[g->deck][g->totalCard - 1].rank = '\0';
            g->cards[g->deck][g->totalCard - 1].suit = '\0';
            g->diamondTwoRemovedFlag = 1;
        }
        for(i = 0, j = 0; i < g->totalCard; i += 3, j++) {
            g->aCards[j].rank = g->cards[g->deck][i].rank;
            g->aCards[j].suit = g->cards[g->deck][i].suit;  
        }
        for(i = 1, j = 0; i < g->totalCard; i += 3, j++) {
            g->bCards[j].rank = g->cards[g->deck][i].rank;
            g->bCards[j].suit = g->cards[g->deck][i].suit;  
        }
        for(i = 2, j = 0; i < g->totalCard; i += 3, j++) {
            g->cCards[j].rank = g->cards[g->deck][i].rank;
            g->cCards[j].suit = g->cards[g->deck][i].suit;
              
        }
    }
}

/*
 * Function: read_deck
 *
 * read deck file, save it to buf if no errors
 *
 * g: struct Game
 * fileName: the deck file need to read
 *
 * return void
 */ 
void read_deck(char *fileName, Game *g) {
    
    char buf[2048];
    FILE *f = fopen(fileName, "r");
    if(!f) {
        fprintf(stderr, "Unable to access deckfile\n");
        fflush(stderr);
        exit(3);        
    }
    g->totalDeck = 0;
    while(fgets(buf, 2048, f) != NULL) {
        remove_newline(buf);
        remove_comma(buf);
        if(buf[0] == '.' && buf[1] == '\0') {
            
            g->totalDeck++;
            if(g->totalCard != 52) {
                fprintf(stderr, "Error reading deck\n");
                fflush(stderr);
                exit(4);
            }
            g->totalCard = 0;
        } else if(buf[0] == '#') {
            ;
        } else if(buf[0] == 0) {
            ;
        } else {
            read_one_line(g, buf);
        }
        
    }
    g->totalDeck++;

    if(g->totalCard != 52) {
        fprintf(stderr, "Error reading deck\n");
        fflush(stderr);
        exit(4);
    }
}

/*
 * Function: check_arguments
 *
 * check arguments passed into game
 *
 * argc: number of arguments
 * argv[]: arguments
 * g: struct Game
 *
 * return void
 */
void check_arguments(int argc, char *argv[], Game *g) {
    char *ptr;
    if(argc != 5 && argc != 6 && argc != 7) {
        fprintf(stderr, 
                "Usage: clubhub deckfile winscore "
                "prog1 prog2 [prog3 [prog4]]\n");
        fflush(stderr);
        exit(1);  
    }
    if(argc == 5) {
        g->numOfPlayer = 2;
        g->handCard = 26;
    } else if(argc == 6) {
        g->numOfPlayer = 3;
        g->handCard = 17;
    } else if(argc == 7) {
        g->numOfPlayer = 4;
        g->handCard = 13;
    }
    if((int) strtol(argv[2], &ptr, 10) <= 0 || *ptr) {
        fprintf(stderr, "Invalid score\n");
        fflush(stderr);
        exit(2);
    }
    g->score = (int) strtol(argv[2], &ptr, 10);
    
    read_deck(argv[1], g);
}

/*
 * Function: spawn_player
 *
 * create children and connect them with parent
 * 
 * player: the player need to create child
 * command: the string need to deal with
 * g: struct Game
 *
 * return void
 */
void spawn_player(Game *g, int player, char *command) {
    int i;
    int send[2];
    int receive[2];
    int file;
    
    if (pipe(send) != 0) {
        _exit(99);
    }
    if(pipe(receive) != 0) {
        _exit(99);
    }
    g->player[player].id = fork();
    if(g->player[player].id < 0) {
        
        fprintf(stderr, "Unable to start subprocess\n");
        fflush(stderr);
        exit(5);
    } else if(g->player[player].id > 0) {
        g->player[player].send = fdopen(send[1], "w");
        g->player[player].receive = fdopen(receive[0], "r");
        close(send[0]);
        close(receive[1]);
    } else {    
        close(send[1]);
        close(receive[0]);
        dup2(send[0], 0);
        dup2(receive[1], 1);
        close(send[0]);
        close(receive[1]);
           
        file = open("/dev/null", O_WRONLY);
        dup2(file, 2);
        close(file);
        
        for(i = 0; i < player; i++) {
            fclose(g->player[i].send);
            fclose(g->player[i].receive);            
        }
        
        char playerNumber[20];
        char playerName[20];
        
        sprintf(playerNumber, "%d", g->numOfPlayer);
        sprintf(playerName, "%c", 'A' + player);
        execlp(command, command, playerNumber, playerName, (char *)0);
        _exit(99);
    }
}

/*
 * Function: wait_clubber_start
 *
 * when send a message, wait the clubber to respond
 * 
 * g: struct Game
 *
 * return void
 */
void wait_clubber_start(Game *g) {
    char dash;
    int i;
        
    for(i = 0; i < g->numOfPlayer; i++) {    
        dash = fgetc(g->player[i].receive);
        if(dash == EOF) {
            waitpid(g->player[i].id, 0, 0);
            fclose(g->player[i].send);
            fclose(g->player[i].receive);
            fprintf(stderr, "Unable to start subprocess\n");
            fflush(stderr);
            exit(5);
        }
    }
}

/*
 * Function: insert_played_cards
 *
 * insert all played cards into an array
 * 
 * g: struct Game
 * lastPlayedCard: the card that just be played 
 *
 * return void
 */
void insert_played_cards(Card *cards, Card lastPlayedCard) {
    int i;
    for(i = 0; i < 52; ++i) {
        cards[i + 1] = cards[i];
    }
    cards[0] = lastPlayedCard;
}

/*
 * Function: convert_card_comparable
 *
 * convert cards that we can compare them, convert "S C D H" into "W X Y Z"
 * so we can sort them by ASCII value
 * 
 * card: the card need to transfer
 *
 * return the transferred card
 */
Card convert_card_comparable(Card card) {
    if(card.suit == 'S') {
        card.suit = 'W';
    }
    if(card.suit == 'C') {
        card.suit = 'X';
    }
    if(card.suit == 'D') {
        card.suit = 'Y';
    }
    if(card.suit == 'H') {
        card.suit = 'Z';
    }
    if(card.rank == 'T') {
        card.rank = 'B';
    }
    if(card.rank == 'J') {
        card.rank = 'C';
    }
    if(card.rank == 'Q') {
        card.rank = 'D';
    }
    if(card.rank == 'K') {
        card.rank = 'E';
    }
    if(card.rank == 'A') {
        card.rank = 'F';
    }
    return card;
}

/*
 * Function: compare_cards
 *
 * compare two cards with the rule: compare 
 * suit first, if same then compare rank.
 * 
 * firstCard: one of the card need to compared
 * secondCard: one of the card need to compared 
 *
 * return 1 if first card is larger, 0 is second card is larger
 */
int compare_cards(Card firstCard, Card secondCard) {
    firstCard = convert_card_comparable(firstCard);
    secondCard = convert_card_comparable(secondCard);
    if(firstCard.suit - secondCard.suit > 0) {
        return 1;
    }
    if(firstCard.suit - secondCard.suit < 0) {
        return 0;
    }
    if(firstCard.suit == secondCard.suit) {
        if(firstCard.rank - secondCard.rank > 0) {
            return 1;
        }
        if(firstCard.rank - secondCard.rank < 0) {
            return 0;
        }
    }
    return 0;
}

/*
 * Function: sort_cards
 *
 * sort an array of cards with rule in compare_cards function
 * 
 * g: struct Game
 * cards: the card array need to be sorted
 *
 * return void
 */
void sort_cards(Game *g, Card *cards) {
    int i;
    int j;
    Card tmpCard;
    for(i = 0; i < g->handCard; ++i) {
        for(j = i + 1; j < g->handCard; ++j) {
            if(compare_cards(cards[i], cards[j])) {
                tmpCard = cards[i];
                cards[i] = cards[j];
                cards[j] = tmpCard;
            }
        }
    }
}

/*
 * Function: output_hands
 *
 * print player's hand card into stdout at start of game
 * 
 * g: struct Game
 * cards: the card array need to be printed
 * playerName: which player's hands need to printed
 *
 * return void
 */
void output_hands(Game *g, char playerName, Card *cards) {
    int i;
    sort_cards(g, cards);
    fprintf(stdout, "Player (%c): ", playerName);
    fflush(stdout);
    for(i = 0; i < g->handCard; ++i) {
        if(i > 0) {
            fprintf(stdout, ",");
            fflush(stdout);
        }
        fprintf(stdout, "%c%c", cards[i].rank, cards[i].suit);
        fflush(stdout);
        
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

/*
 * Function: send_newround_once
 *
 * send newround to one player
 * 
 * g: struct Game
 * cards: the card array need to be sent
 * player: which player need to be sent
 *
 * return void
 */
void send_newround_once(Game *g, int player, Card *cards) {
    int i;
    
    fprintf(g->player[player].send, "newround ");
    fflush(g->player[player].send);
    for(i = 0; i < g->handCard; ++i) {
        if(i > 0) {
            fprintf(g->player[player].send, ",");
            fflush(g->player[player].send);
        }
        fprintf(g->player[player].send, "%c%c", cards[i].rank, cards[i].suit);
        fflush(g->player[player].send);
    }
    fprintf(g->player[player].send, "\n");
    fflush(g->player[player].send);
}

/*
 * Function: send_newround
 *
 * send newround to all the players
 * 
 * g: struct Game
 *
 * return void
 */
void send_newround(Game *g) {
    if(g->numOfPlayer == 2) {
        
        send_newround_once(g, 0, g->aCards);
        output_hands(g, 'A', g->aCards);
        
        send_newround_once(g, 1, g->bCards);
        output_hands(g, 'B', g->bCards);
    }
    if(g->numOfPlayer == 3) {
        send_newround_once(g, 0, g->aCards);
        output_hands(g, 'A', g->aCards);
        send_newround_once(g, 1, g->bCards);
        output_hands(g, 'B', g->bCards);
        send_newround_once(g, 2, g->cCards);
        output_hands(g, 'C', g->cCards);
    }
    if(g->numOfPlayer == 4) {
        send_newround_once(g, 0, g->aCards);
        output_hands(g, 'A', g->aCards);
        send_newround_once(g, 1, g->bCards);
        output_hands(g, 'B', g->bCards);
        send_newround_once(g, 2, g->cCards);
        output_hands(g, 'C', g->cCards);
        send_newround_once(g, 3, g->dCards);
        output_hands(g, 'D', g->dCards);
    }
}

/*
 * Function: send_newtrick
 *
 * send newtrick to the lead player
 * 
 * g: struct Game
 * player: which player need to be sent
 *
 * return void
 */
void send_newtrick(Game *g, int player) {
    fprintf(g->player[player].send, "newtrick");
    fprintf(g->player[g->currentPlayer].send, "\n");
    fflush(g->player[player].send);
}

/*
 * Function: check_not_in_played
 *
 * check if a played card is in the array of all played cards
 * 
 * cardRank: that card's rank
 * cardSuit: that card's suit
 * g: struct Game
 *
 * return 0 if in the array and 1 if not in his hands
 */
int check_not_in_played(Game *g, int cardRank, char cardSuit) {
    int i;
    if(cardRank == 10) {
        cardRank = 36;
    }
    if(cardRank == 11) {
        cardRank = 26;
    }
    if(cardRank == 12) {
        cardRank = 33;
    }
    if(cardRank == 13) {
        cardRank = 27;
    }
    if(cardRank == 14) {
        cardRank = 17;
    }
    for(i = 0; i < g->playedCard; ++i) {
        if((g->playedCards[i].rank - '0') == cardRank && 
                g->playedCards[i].suit == cardSuit) {
            return 0;
        }
    }
    return 1;
}

/*
 * Function: check_in_hand
 *
 * check if a played card is in that player's hand cards
 * 
 * cardRank: that card's rank
 * cardSuit: that card's suit
 * g: struct Game
 *
 * return 1 if in his hands and 0 if not in his hands
 */
int check_in_hand(Game *g, int cardRank, char cardSuit) {
    int i;
    if(cardRank == 10) {
        cardRank = 36;
    }
    if(cardRank == 11) {
        cardRank = 26;
    }
    if(cardRank == 12) {
        cardRank = 33;
    }
    if(cardRank == 13) {
        cardRank = 27;
    }
    if(cardRank == 14) {
        cardRank = 17;
    }
    if(g->currentPlayer == 0) {
        for(i = 0; i < g->handCard; ++i) {
            if((g->aCards[i].rank - '0') == cardRank && 
                    g->aCards[i].suit == cardSuit) {
                return 1;
            }
        }
    } else if(g->currentPlayer == 1) {
        for(i = 0; i < g->handCard; ++i) {
            if((g->bCards[i].rank - '0') == cardRank && 
                    g->bCards[i].suit == cardSuit) {
                return 1;
            }
        }
    } else if(g->currentPlayer == 2) {
        for(i = 0; i < g->handCard; ++i) {
            if((g->cCards[i].rank - '0') == cardRank && 
                    g->cCards[i].suit == cardSuit) {
                return 1;
            }
        }
    } else if(g->currentPlayer == 3) {
        for(i = 0; i < g->handCard; ++i) {
            if((g->dCards[i].rank - '0') == cardRank && 
                    g->dCards[i].suit == cardSuit) {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Function: output_lead_played
 *
 * print who led what card information to stdout
 * 
 * g: struct Game
 *
 * return void
 */
void output_lead_played(Game *g) {
    char playerName;
    if(g->currentPlayer == 0) {
        playerName = 'A';
    }
    if(g->currentPlayer == 1) {
        playerName = 'B';
    }
    if(g->currentPlayer == 2) {
        playerName = 'C';
    }
    if(g->currentPlayer == 3) {
        playerName = 'D';
    }
    
    fprintf(stdout, "Player %c led %c%c\n", playerName, 
            g->lastPlayedCard.rank, g->lastPlayedCard.suit);
    fflush(stdout);
}

/*
 * Function: output_not_lead_played
 *
 * print who played what card information to stdout
 * 
 * g: struct Game
 *
 * return void
 */
void output_not_lead_played(Game *g) {
    char playerName;
    if(g->currentPlayer == 0) {
        playerName = 'A';
    }
    if(g->currentPlayer == 1) {
        playerName = 'B';
    }
    if(g->currentPlayer == 2) {
        playerName = 'C';
    }
    if(g->currentPlayer == 3) {
        playerName = 'D';
    }
    
    fprintf(stdout, "Player %c played %c%c\n", playerName, 
            g->lastPlayedCard.rank, g->lastPlayedCard.suit);
    fflush(stdout);
}

/*
 * Function: check_lead_responce
 *
 * when leader played a card, check that card is valid
 * 
 * g: struct Game
 *
 * return void
 */
void check_lead_responce(Game *g) {
    int cardRank;
    char cardSuit;
    char message[2048];
    char *ptr;
    ptr = message;
    
    if (fgets(message, 2048, g->player[g->currentPlayer].receive) == NULL) {
        fprintf(stderr, "Player quit\n");
        fflush(stderr);
        exit(6);
    }
    
    if((cardRank = read_card_rank(*ptr)) == 0 || *(++ptr) == '\0') {
        fprintf(stderr, "Invalid message received from player\n");
        fflush(stderr);
        exit(7);
    }   
    if((cardSuit = read_card_suit(*ptr)) == 0) {
        fprintf(stderr, "Invalid message received from player\n");
        fflush(stderr);
        exit(7);
    }
    if(*(++ptr) != '\n') {
        fprintf(stderr, "Invalid message received from player\n");
        fflush(stderr);
        exit(7);
    }
    if(check_in_hand(g, cardRank, cardSuit) == 0) {
        fprintf(stderr, "Invalid play by player\n");
        fflush(stderr);
        exit(8);
    }
    g->lastPlayedCard.rank = message[0];
    g->lastPlayedCard.suit = message[1];
    
    if(check_not_in_played(g, cardRank, cardSuit) == 0) {
        fprintf(stderr, "Invalid play by player\n");
        fflush(stderr);
        exit(8);
    }
    
    g->leadSuit = message[1];
    
    output_lead_played(g);
    g->oneTrickCards[g->currentPlayer].rank = message[0];
    g->oneTrickCards[g->currentPlayer].suit = message[1];
    insert_played_cards(g->playedCards, g->lastPlayedCard);
    g->playedCard++;
    
}

/*
 * Function: check_not_lead_responce
 *
 * when a player (not leader) played a card, check that card is valid
 * 
 * g: struct Game
 *
 * return void
 */
void check_not_lead_responce(Game *g) {
    int cardRank;
    char cardSuit;
    char message[2048];
    char *ptr;
    ptr = message;
    if (fgets(message, 2048, g->player[g->currentPlayer].receive) == NULL) {
        fprintf(stderr, "Player quit\n");
        fflush(stderr);
        exit(6);
    }
    if((cardRank = read_card_rank(*ptr)) == 0 || *(++ptr) == '\0') {
        fprintf(stderr, "Invalid message received from player\n");
        fflush(stderr);
        exit(7);
    }   
    if((cardSuit = read_card_suit(*ptr)) == 0) {
        fprintf(stderr, "Invalid message received from player\n");
        fflush(stderr);
        exit(7);
    }
    if(*(++ptr) != '\n') {
        fprintf(stderr, "Invalid message received from player\n");
        fflush(stderr);
        exit(7);
    }
    if(check_in_hand(g, cardRank, cardSuit) == 0) {
        fprintf(stderr, "Invalid play by player\n");
        fflush(stderr);
        exit(8);
    }
    g->lastPlayedCard.rank = message[0];
    g->lastPlayedCard.suit = message[1];
    
    if(check_not_in_played(g, cardRank, cardSuit) == 0) {
        fprintf(stderr, "Invalid play by player\n");
        fflush(stderr);
        exit(8);
    }
    
    
    output_not_lead_played(g);
    g->oneTrickCards[g->currentPlayer].rank = message[0];
    g->oneTrickCards[g->currentPlayer].suit = message[1];
    
    insert_played_cards(g->playedCards, g->lastPlayedCard);
    g->playedCard++;
}

/*
 * Function: send_played
 *
 * send played what cards information to a player
 * 
 * g: struct Game
 * player: which player is sent the information
 *
 * return void
 */
void send_played(Game *g, int player) {
    fprintf(g->player[player].send, "played %c%c", 
            g->lastPlayedCard.rank, g->lastPlayedCard.suit);
    fprintf(g->player[player].send, "\n");
    fflush(g->player[player].send);
}

/*
 * Function: increment_current_player
 *
 * move to next player, if to the last player, return to first player
 * 
 * g: struct Game
 *
 * return void
 */
void increment_current_player(Game *g) {
    g->currentPlayer++;
    if(g->currentPlayer == g->numOfPlayer) {
        g->currentPlayer = 0;
    }
}

/*
 * Function: send_yourturn
 *
 * send youtturn to a player
 * 
 * g: struct Game
 *
 * return void
 */
void send_yourturn(Game *g) {
    fprintf(g->player[g->currentPlayer].send, "yourturn");
    fprintf(g->player[g->currentPlayer].send, "\n");
    fflush(g->player[g->currentPlayer].send);
}

/*
 * Function: send_trickover
 *
 * send trickover to a player
 * 
 * g: struct Game
 * player: wthe player to be sent
 *
 * return void
 */
void send_trickover(Game *g, int player) {
    fprintf(g->player[player].send, "trickover");
    fprintf(g->player[player].send, "\n");
    fflush(g->player[player].send);
}

/*
 * Function: find_trick_winner
 *
 * find who wins this trick, return his index
 * 
 * g: struct Game
 *
 * return the index of that player or -1 if errors happenes
 */
int find_trick_winner(Game *g) {
    int i;
    char currentHighestRank = '1';
    for(i = 0; i < g->numOfPlayer; ++i) {
        if(g->oneTrickCards[i].rank == 'A') {
            g->oneTrickCards[i].rank = 'F';
        }
        if(g->oneTrickCards[i].rank == 'T') {
            g->oneTrickCards[i].rank = 'B';
        }
        if(g->oneTrickCards[i].rank == 'J') {
            g->oneTrickCards[i].rank = 'C';
        }
        if(g->oneTrickCards[i].rank == 'Q') {
            g->oneTrickCards[i].rank = 'D';
        }
        if(g->oneTrickCards[i].rank == 'K') {
            g->oneTrickCards[i].rank = 'E';
        }
        
    }
    for(i = 0; i < g->numOfPlayer; ++i) {
        if(g->oneTrickCards[i].suit == g->leadSuit) {
            if(g->oneTrickCards[i].rank > currentHighestRank) {
                currentHighestRank = g->oneTrickCards[i].rank;
            }
        }
    }
    for(i = 0; i < g->numOfPlayer; ++i) {
        if(g->oneTrickCards[i].suit == g->leadSuit && 
                g->oneTrickCards[i].rank == currentHighestRank) {
            return i;
        }
    }
    return -1;
}

/*
 * Function: count_clubs
 *
 * count how many clubs played in this trick
 * 
 * g: struct Game
 *
 * return the number of clubs
 */
int count_clubs(Game *g) {
    int i;
    int totalClub = 0;
    for(i = 0; i < g->numOfPlayer; ++i) {
        if(g->oneTrickCards[i].suit == 'C') {
            totalClub++;
        }
    }
    return totalClub;
}

/*
 * Function: send_scores
 *
 * send scores information to a player
 * 
 * g: struct Game
 * player: the player to be sent
 *
 * return void
 */
void send_scores(Game *g, int player) {
    int i;
    fprintf(g->player[player].send, "scores ");
    for(i = 0; i < g->numOfPlayer; ++i) {
        if(i > 0) {
            fprintf(g->player[player].send, ",");
        }
        fprintf(g->player[player].send, "%d", g->scores[i]);
    }
    fprintf(g->player[player].send, "\n");
    fflush(g->player[player].send);
}

/*
 * Function: output_scores
 *
 * send scores information to stdout
 * 
 * g: struct Game
 *
 * return void
 */
void output_scores(Game *g) {
    int i;
    fprintf(stdout, "scores ");
    for(i = 0; i < g->numOfPlayer; ++i) {
        if(i > 0) {
            fprintf(stdout, ",");
        }
        fprintf(stdout, "%d", g->scores[i]);
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

/*
 * Function: send_end
 *
 * send end to a player
 * 
 * g: struct Game
 * player: the player tobe sent
 *
 * return void
 */
void send_end(Game *g, int player) {
    fprintf(g->player[player].send, "end\n");
    fflush(g->player[player].send);
}

/*
 * Function: output_winners
 *
 * send the winners into stdout
 * 
 * g: struct Game
 *
 * return void
 */
void output_winners(Game *g) {
    int t;
    int minScore;
    int multipleWinnerFlag;
    minScore = 999999; 
    multipleWinnerFlag = 0;
    
    for(t = 0; t < g->numOfPlayer; t++) {
        if(g->scores[t] < minScore) {
            minScore = g->scores[t];
        }
    }
    
    for(t = 0; t < g->numOfPlayer; t++) {
        if(g->scores[t] == minScore && multipleWinnerFlag == 0) {
            multipleWinnerFlag = 1;
            
            fprintf(stdout, "Winner(s): %c", 'A' + t);
            fflush(stdout);
            t++;
            
        }
        if(g->scores[t] == minScore && 
                multipleWinnerFlag == 1 && t < g->numOfPlayer) {
            fprintf(stdout, " %c", 'A' + t);
            fflush(stdout);
        }
        
    }
    fprintf(stdout, "\n");
    fflush(stdout);
    
}

/*
 * Function: start_to_play
 *
 * the main game loop
 * 
 * g: struct Game
 *
 * return void
 */
void start_to_play(Game *g) {
    int i, j, k;
    while(1) {
        divide_cards(g);
        send_newround(g);
        for(k = 0; k < g->handCard; ++k) {
            send_newtrick(g, g->currentPlayer);
            check_lead_responce(g);
            for(i = 0; i < g->numOfPlayer; ++i) {
                send_played(g, i);
            }
            increment_current_player(g);
            for(i = 0; i < g->numOfPlayer - 1; ++i) {
                send_yourturn(g);
                check_not_lead_responce(g);
                for(j = 0; j < g->numOfPlayer; ++j) {
                    send_played(g, j);
                }
                increment_current_player(g);
            }
            for(i = 0; i < g->numOfPlayer; ++i) {
                send_trickover(g, i);
            }        
            g->scores[find_trick_winner(g)] += count_clubs(g);
            for(i = 0; i < g->numOfPlayer; ++i) {
                send_scores(g, i);
            }
            g->currentPlayer = find_trick_winner(g);
        }
        output_scores(g);
        for(i = 0; i < g->numOfPlayer; ++i) {
            if(g->scores[i] >= g->score) {
                for(j = 0; j < g->numOfPlayer; ++j) {
                    send_end(g, j);
                }
                output_winners(g);
                exit(0);
            }
        }
        g->deck++;
        if(g->deck >= g->totalDeck) {
            g->deck = 0;
        }
    }
}

/*
 * Function: main
 *
 * the main function, here we start the game and
 * call other functions
 *
 * argc: number of arguments
 * argv[]: arguments
 *
 * return void
 */
int main(int argc, char *argv[]) {
    int i;
    Game *g = malloc(sizeof(Game));
    g->diamondTwoRemovedFlag = 0;
    g->currentPlayer = 0;
    check_arguments(argc, argv, g);
    for(i = 0; i < g->numOfPlayer; ++i) {
        spawn_player(g, i, argv[i + 3]);
    }
    wait_clubber_start(g);
    g->deck = 0;
    start_to_play(g);
    return 0;
}