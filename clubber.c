#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Game {
    int numOfPlayer;
    int currentPlayer;
    char playerID;
    int nextIsLeadFlag;
    char leadSuit;
    int diamondCards[13];
    int heartCards[13];
    int spadeCards[13];
    int clubCards[13];
    int playedDiamondCards[13];
    int playedHeartCards[13];
    int playedSpadeCards[13];
    int playedClubCards[13];
    int heartCardNumber;
    int diamondCardNumber;
    int clubCardNumber;
    int spadeCardNumber;
    int totalCardNumber;
    int playedHeartCardNumber;
    int playedDiamondCardNumber;
    int playedClubCardNumber;
    int playedSpadeCardNumber;
    int *scores;
};
                                                                      
typedef struct Game Game;                                            

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

    if(argc != 3) {
        fprintf(stderr, "Usage: player number_of_players myid\n");
        fflush(stderr);
        exit(1);  
    }
    
    if(argv[1][0] != '2' && argv[1][0] != '3' && argv[1][0] != '4') {
        fprintf(stderr, "Invalid player count\n");
        fflush(stderr);
        exit(2);
    }
    
    if(argv[1][1] != '\0') {
        fprintf(stderr, "Invalid player count\n");
        fflush(stderr);
        exit(2);
    }
    
    g->numOfPlayer = argv[1][0] - '0';
    g->scores = (int *) malloc(g->numOfPlayer * sizeof(int));
    
    if(argv[2][0] != 'A' && argv[2][0] != 'B' && 
            argv[2][0] != 'C' && argv[2][0] != 'D') {
        fprintf(stderr, "Invalid player ID\n");
        fflush(stderr);
        exit(3);
    }
    
    if(argv[2][1] != '\0') {
        fprintf(stderr, "Invalid player ID\n");
        fflush(stderr);
        exit(3);    
    }
    
    if(argv[2][0] - 'A' >= g->numOfPlayer) {
        fprintf(stderr, "Invalid player ID\n");
        fflush(stderr);
        exit(3);
    }
    g->playerID = argv[2][0]; 
    fprintf(stdout, "-");   
    fflush(stdout);
}

/*
 * Function: clear_array
 *
 * clear all of the arrays
 *
 * g: struct Game
 *
 * return void
 */
void clear_array(Game *g) {
    int i;
    for(i = 0; i < 13; ++i) {
        g->diamondCards[i] = 0;
        g->heartCards[i] = 0;
        g->spadeCards[i] = 0;
        g->clubCards[i] = 0;
        g->playedDiamondCards[i] = 0;
        g->playedHeartCards[i] = 0;
        g->playedSpadeCards[i] = 0;
        g->playedClubCards[i] = 0;
    }
    g->heartCardNumber = 0;
    g->diamondCardNumber = 0;
    g->clubCardNumber = 0;
    g->spadeCardNumber = 0;
    g->playedSpadeCardNumber = 0;
    g->playedClubCardNumber = 0;
    g->playedHeartCardNumber = 0;
    g->playedDiamondCardNumber = 0;
    g->totalCardNumber = 0;
}

/*
 * Function: read_scores
 *
 * read scores information from hub
 *
 * g: struct Game
 * buf: the buf we read
 *
 * return void
 */
void read_scores(char *buf, Game *g) {
    char *pos;
    int i;
    int num;
    i = 0;
    pos = buf;
    
    while(*pos && i < g->numOfPlayer) {
        num = strtol(pos, &pos, 10);
        if(num < 0) {
            fprintf(stderr, "Bad message from hub\n");
            fflush(stderr);
            exit(5);
        }
        
        if (*pos != ',' && *pos != 0 && *pos != '\n') {
            fprintf(stderr, "Bad message from hub\n");
            fflush(stderr);
            exit(5);                    
        }
        
        g->scores[i] = num;
        pos++;
        
        i++;
    }
    
    if(i != g->numOfPlayer) {
        fprintf(stderr, "Bad message from hub\n");
        fflush(stderr);
        exit(5);
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
    char allCards[13] = {'2', '3', '4', '5', '6', '7', 
            '8', '9', 'T', 'J', 'Q', 'K', 'A'};
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
 * Function: insert_into_array
 *
 * insert a integer into a integer array's first place if the
 * integer is not in the array already
 *
 * array: the array to store that integer
 * value: the integer to be inserted
 *
 * return 1 if insert success or 0 if there is already that integer
 * in the array
 */ 
int insert_into_array(int *array, int value) {
    int c;
    for(c = 0; c < 13; ++c) {
        if(array[c] == value) {
            return 0;
        }
    }
    for(c = 11; c >= 0; c--) {
        array[c + 1] = array[c];
        
    }
    array[0] = value;
    return 1;
}

/*
 * Function: bubble_sort
 *
 * sort the array
 *
 * array: the array need to be sorted
 * arraySize: the size of the array
 *
 * return void
 */ 
void bubble_sort(int *array, int arraySize) {
    int i, j, temp;
    for (i = 0; i < (arraySize - 1); ++i) {
        for (j = 0; j < arraySize - 1 - i; ++j) {
            if (array[j] > array[j + 1]) {
                temp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = temp;
            }
        }
    }
}  

/*
 * Function: add_into_played_array
 *
 * when a card is played, put it into played card array of that suit
 *
 * g: struct Game
 * cardRank: the rank of card
 * cardSuit: the suit of card
 *
 * return void
 */ 
void add_into_played_array(Game *g, int cardRank, char cardSuit) {
    if(cardSuit == 'H') {
        if(insert_into_array(g->playedHeartCards, cardRank)) {
            g->playedHeartCardNumber++;
            bubble_sort(g->playedHeartCards, g->playedHeartCardNumber);
        }
        g->currentPlayer++;
        if(g->nextIsLeadFlag == 1) {
            g->leadSuit = 'H';
            g->nextIsLeadFlag = 0;
        }
    }
    if(cardSuit == 'S') {
        if(insert_into_array(g->playedSpadeCards, cardRank)) {
            g->playedSpadeCardNumber++;
            bubble_sort(g->playedSpadeCards, g->playedSpadeCardNumber);
        }
        g->currentPlayer++;
        if(g->nextIsLeadFlag == 1) {
            g->leadSuit = 'S';
            g->nextIsLeadFlag = 0;
        }
    }
    if(cardSuit == 'D') {
        if(insert_into_array(g->playedDiamondCards, cardRank)) {
            g->playedDiamondCardNumber++;
            bubble_sort(g->playedDiamondCards, g->playedDiamondCardNumber);
        }
        g->currentPlayer++;
        if(g->nextIsLeadFlag == 1) {
            g->leadSuit = 'D';
            g->nextIsLeadFlag = 0;
        }
    }
    if(cardSuit == 'C') {
        if(insert_into_array(g->playedClubCards, cardRank)) {
            g->playedClubCardNumber++;
            bubble_sort(g->playedClubCards, g->playedClubCardNumber);
        }
        g->currentPlayer++;
        if(g->nextIsLeadFlag == 1) {
            g->leadSuit = 'C';
            g->nextIsLeadFlag = 0;
        }
    }
}

/*
 * Function: read_played_card
 *
 * read a played card, check it is valid
 *
 * g: struct Game
 * message: the card we read
 *
 * return void
 */ 
void read_played_card(Game *g, char *message) {
    int cardRank;
    char cardSuit;
    char *ptr;
    ptr = message;
    ptr += 6;
    if(*ptr != ' ' || *(++ptr) == '\0') {
        fprintf(stderr, "Bad message from hub\n");
        fflush(stderr);
        exit(5);
    }
    if((cardRank = read_card_rank(*ptr)) == 0 || *(++ptr) == '\0') {
        fprintf(stderr, "Bad message from hub\n");
        fflush(stderr);
        exit(5);
    }   
    if((cardSuit = read_card_suit(*ptr)) == 0) {
        fprintf(stderr, "Bad message from hub\n");
        fflush(stderr);
        exit(5);
    }
    if(*(++ptr) != ',' && *ptr != '\0') {
        fprintf(stderr, "Bad message from hub\n");
        fflush(stderr);
        exit(5);
    }
    add_into_played_array(g, cardRank, cardSuit);
} 

/*
 * Function: add_into_cards_array
 *
 * read a card, and add it into hand cards array
 *
 * g: struct Game
 * cardRank: the rank of card
 * cardSuit: the suit of card
 *
 * return void
 */ 
void add_into_cards_array(Game *g, int cardRank, char cardSuit) {
    if(cardSuit == 'H') {
        insert_into_array(g->heartCards, cardRank);
        g->heartCardNumber++;
        bubble_sort(g->heartCards, g->heartCardNumber);
    } else if(cardSuit == 'S') {
        insert_into_array(g->spadeCards, cardRank);
        g->spadeCardNumber++;
        bubble_sort(g->spadeCards, g->spadeCardNumber);
    } else if(cardSuit == 'C') {
        insert_into_array(g->clubCards, cardRank);
        g->clubCardNumber++;
        bubble_sort(g->clubCards, g->clubCardNumber);
    } else if(cardSuit == 'D') {
        insert_into_array(g->diamondCards, cardRank);
        g->diamondCardNumber++;
        bubble_sort(g->diamondCards, g->diamondCardNumber);
    }    
}

/*
 * Function: read_card_list
 *
 * read a card array
 *
 * g: struct Game
 * message: rhe string we read
 *
 * return void
 */
void read_card_list(Game *g, char *message) {
    char *ptr;
    ptr = message;
    int cardRank;
    char cardSuit;
    ptr += 8;
    if(*ptr != ' ' || *(++ptr) == '\0') {
        fprintf(stderr, "Bad message from hub\n");
        fflush(stderr);
        exit(5);
    }
    while(1) {
        if((cardRank = read_card_rank(*ptr)) == 0 || *(++ptr) == '\0') {
            fprintf(stderr, "Bad message from hub\n");
            fflush(stderr);
            exit(5);
        }
        if((cardSuit = read_card_suit(*ptr)) == 0) {
            fprintf(stderr, "Bad message from hub\n");
            fflush(stderr);
            exit(5);
        }
        if(*(++ptr) != ',' && *ptr != '\0') {
            fprintf(stderr, "Bad message from hub\n");
            fflush(stderr);
            exit(5);
        }
        add_into_cards_array(g, cardRank, cardSuit);
        g->totalCardNumber++;
        if(*ptr++ == '\0') {
            break;
        }
    }
    if((g->numOfPlayer == 2 && g->totalCardNumber++ != 26) ||
            (g->numOfPlayer == 3 && g->totalCardNumber++ != 17) ||
            (g->numOfPlayer == 4 && g->totalCardNumber++ != 13)) {
        fprintf(stderr, "Bad message from hub\n");
        fflush(stderr);
        exit(5);
    }
}

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
 * Function: print_hand
 *
 * print hand cards
 *
 * g: struct Game
 *
 * return void
 */ 
void print_hand(Game *g) {
    int i;
    int printed = 0;
    char allCards[13] = {'2', '3', '4', '5', '6', '7', '8', 
            '9', 'T', 'J', 'Q', 'K', 'A'};
    fprintf(stderr, "Hand: ");
    for(i = 0; i < g->spadeCardNumber; ++i) {
        if(g->spadeCards[i] != 0) {
            if(printed > 0) {
                fprintf(stderr, ",");
            }
            fprintf(stderr, "%cS", allCards[g->spadeCards[i] - 2]);
            fflush(stderr);
            printed++;
        }
    }
    for(i = 0; i < g->clubCardNumber; ++i) {
        if(g->clubCards[i] != 0) {
            if(printed > 0) {
                fprintf(stderr, ",");
            }
            fprintf(stderr, "%cC", allCards[g->clubCards[i] - 2]);
            fflush(stderr);
            printed++;
        }
    }
    for(i = 0; i < g->diamondCardNumber; ++i) {
        if(g->diamondCards[i] != 0) {
            if(printed > 0) {
                fprintf(stderr, ",");
            }
            fprintf(stderr, "%cD", allCards[g->diamondCards[i] - 2]);
            fflush(stderr);
            printed++;
        }
    }
    for(i = 0; i < g->heartCardNumber; ++i) {

        if(g->heartCards[i] != 0) {
            if(printed > 0) {
                fprintf(stderr, ",");
            }
            fprintf(stderr, "%cH", allCards[g->heartCards[i] - 2]);
            fflush(stderr);
            printed++;
        }
    }
    fprintf(stderr, "\n");
}

/*
 * Function: print_played
 *
 * print played card
 *
 * g: struct Game
 *
 * return void
 */ 
void print_played(Game *g) {
    int i;
    char allCards[13] = {'2', '3', '4', '5', '6', '7', '8', 
            '9', 'T', 'J', 'Q', 'K', 'A'};
    fprintf(stderr, "Played (S): ");
    for(i = 0; i < g->playedSpadeCardNumber; ++i) {
        if(g->playedSpadeCards[i] != 0) {
            if(i > 0) {
                fprintf(stderr, ",");
            }
            fprintf(stderr, "%c", allCards[g->playedSpadeCards[i] - 2]);
        }     
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Played (C): ");
    fflush(stderr);
    for(i = 0; i < g->playedClubCardNumber; ++i) {
        if(g->playedClubCards[i] != 0) {
            if(i > 0) {
                fprintf(stderr, ",");
            }
            fprintf(stderr, "%c", allCards[g->playedClubCards[i] - 2]);
        } 
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Played (D): ");
    fflush(stderr);
    for(i = 0; i < g->playedDiamondCardNumber; ++i) {
        if(g->playedDiamondCards[i] != 0) {
            if(i > 0) {
                fprintf(stderr, ",");
            }
            fprintf(stderr, "%c", allCards[g->playedDiamondCards[i] - 2]);
        } 
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Played (H): ");
    fflush(stderr);
    for(i = 0; i < g->playedHeartCardNumber; ++i) {
        if(g->playedHeartCards[i] != 0) {
            if(i > 0) {
                fprintf(stderr, ",");
            }
            fprintf(stderr, "%c", allCards[g->playedHeartCards[i] - 2]);
        } 
    }
    fprintf(stderr, "\n");
    fflush(stderr);
}

/*
 * Function: judge_lowest_card
 *
 * judge if a card is lowest, that is all cards lower than it is played
 *
 * g: struct Game
 * cards: the card array to be compared
 * arraySize: the size of that array
 *
 * return 1 if it is lowest and 0 if not
 */ 
int judge_lowest_card(int cardRank, int *cards, int arraySize) {


    if(cardRank == 2) {
        return 1;
    }

    if(arraySize == 0) {
        return 0;    
    }
    
    if(cardRank - 2 > arraySize) {
        return 0;
    }
    
    if(cards[cardRank - 3] != cardRank - 1) {
        return 0;
    }
    return 1;

}

/*
 * Function: play_card_out
 *
 * decide which card should be played if the current player is not lead
 *
 * g: struct Game
 *
 * return void
 */ 
void play_card_out(Game *g) {
    int alreadyPlayedFlag = 0;
    int i;
    char allCards[13] = {'2', '3', '4', '5', '6', '7', '8', 
            '9', 'T', 'J', 'Q', 'K', 'A'};
    for(i = 0; i < g->clubCardNumber; ++i) {
        if(g->clubCards[i] != 0) {
            if(judge_lowest_card(g->clubCards[i], g->playedClubCards, 
                    g->playedClubCardNumber)) {
                fprintf(stdout, "%cC\n", allCards[g->clubCards[i] - 2]);
                fflush(stdout);
                g->leadSuit = 'C';
                if(insert_into_array(g->playedClubCards, g->clubCards[i])) {
                    g->playedClubCardNumber++;
                    bubble_sort(g->playedClubCards, g->playedClubCardNumber);
                }
                g->clubCards[i] = 0;
                g->currentPlayer++;
                alreadyPlayedFlag = 1;
                break;
            }
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = 0; i < g->diamondCardNumber; ++i) {
            if(g->diamondCards[i] != 0) {
                fprintf(stdout, "%cD\n", allCards[g->diamondCards[i] - 2]);
                fflush(stdout);
                g->leadSuit = 'D';
                if(insert_into_array(g->playedDiamondCards, 
                        g->diamondCards[i])) {
                    g->playedDiamondCardNumber++;
                    bubble_sort(g->playedDiamondCards, 
                            g->playedDiamondCardNumber);
                }
                g->currentPlayer++;
                g->diamondCards[i] = 0;
                alreadyPlayedFlag = 1;
                break;
            }
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = 0; i < g->heartCardNumber; ++i) {
            if(g->heartCards[i] != 0) {
                fprintf(stdout, "%cH\n", allCards[g->heartCards[i] - 2]);
                fflush(stdout);
                g->leadSuit = 'H';
                if(insert_into_array(g->playedHeartCards, g->heartCards[i])) {
                    g->playedHeartCardNumber++;
                    bubble_sort(g->playedHeartCards, g->playedHeartCardNumber);
                }
                g->currentPlayer++;
                g->heartCards[i] = 0;
                alreadyPlayedFlag = 1;
                break;
            }
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = 0; i < g->spadeCardNumber; ++i) {
            if(g->spadeCards[i] != 0) {
                fprintf(stdout, "%cS\n", allCards[g->spadeCards[i] - 2]);
                fflush(stdout);
                g->leadSuit = 'S';
                if(insert_into_array(g->playedSpadeCards, g->spadeCards[i])) {
                    g->playedSpadeCardNumber++;
                    bubble_sort(g->playedSpadeCards, g->playedSpadeCardNumber);
                }
                g->currentPlayer++;
                g->spadeCards[i] = 0;
                alreadyPlayedFlag = 1;
                break;
            }
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = 0; i < g->clubCardNumber; ++i) {
            if(g->clubCards[i] != 0) {
                fprintf(stdout, "%cC\n", allCards[g->clubCards[i] - 2]);
                fflush(stdout);
                g->leadSuit = 'C';
                alreadyPlayedFlag = 1;
                if(insert_into_array(g->playedClubCards, g->clubCards[i])) {
                    g->playedClubCardNumber++;
                    bubble_sort(g->playedClubCards, g->playedClubCardNumber);
                }
                g->currentPlayer++;
                g->clubCards[i] = 0;
                break;
            }
        }
    }
}

/*
 * Function: follow_suit
 *
 * decide which card should be played if follow the suit
 *
 * g: struct Game
 *
 * return 1 if follow the suit, 0 if not
 */ 
int follow_suit(Game *g) {
    int i;
    char allCards[13] = {'2', '3', '4', '5', '6', '7', '8', 
            '9', 'T', 'J', 'Q', 'K', 'A'};
    if(g->leadSuit == 'C' && g->clubCardNumber != 0) {
        for(i = 0; i < g->clubCardNumber; ++i) {
            if(g->clubCards[i] != 0) {
                fprintf(stdout, "%cC\n", allCards[g->clubCards[i] - 2]);
                fflush(stdout);
                g->currentPlayer++;
                if(insert_into_array(g->playedClubCards, g->clubCards[i])) {
                    g->playedClubCardNumber++;
                    bubble_sort(g->playedClubCards, g->playedClubCardNumber);
                }
                g->clubCards[i] = 0;
                return 1;
            }
        }
    }
    if(g->leadSuit == 'H' && g->heartCardNumber != 0) {
        for(i = 0; i < g->heartCardNumber; ++i) {
            if(g->heartCards[i] != 0) {
                fprintf(stdout, "%cH\n", allCards[g->heartCards[i] - 2]);
                fflush(stdout);
                g->currentPlayer++;
                if(insert_into_array(g->playedHeartCards, g->heartCards[i])) {
                    g->playedHeartCardNumber++;
                    bubble_sort(g->playedHeartCards, g->playedHeartCardNumber);
                }
                g->heartCards[i] = 0;
                return 1;
            }
        }
    }
    if(g->leadSuit == 'D' && g->diamondCardNumber != 0) {
        for(i = 0; i < g->diamondCardNumber; ++i) {
            if(g->diamondCards[i] != 0) {
                fprintf(stdout, "%cD\n", allCards[g->diamondCards[i] - 2]);
                fflush(stdout);
                g->currentPlayer++;
                if(insert_into_array(g->playedDiamondCards, 
                        g->diamondCards[i])) {
                    g->playedDiamondCardNumber++;
                    bubble_sort(g->playedDiamondCards, 
                            g->playedDiamondCardNumber);
                }
                g->diamondCards[i] = 0;
                return 1;
            }
        }
    }
    if(g->leadSuit == 'S' && g->spadeCardNumber != 0) {
        for(i = 0; i < g->spadeCardNumber; ++i) {
            if(g->spadeCards[i] != 0) {
                fprintf(stdout, "%cS\n", allCards[g->spadeCards[i] - 2]);
                fflush(stdout);
                g->currentPlayer++;
                if(insert_into_array(g->playedSpadeCards, g->spadeCards[i])) {
                    g->playedSpadeCardNumber++;
                    bubble_sort(g->playedSpadeCards, g->playedSpadeCardNumber);
                }
                g->spadeCards[i] = 0;
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Function: last_to_play
 *
 * decide which card should be played if the current player is not lead
 * and the last one to play
 *
 * g: struct Game
 *
 * return void
 */ 
void last_to_play(Game *g) {
    int i;
    int alreadyPlayedFlag = 0;
    char allCards[13] = {'2', '3', '4', '5', '6', '7', '8', 
            '9', 'T', 'J', 'Q', 'K', 'A'};
    for(i = g->heartCardNumber - 1; i >= 0; i--) {
        if(g->heartCards[i] != 0) {
            fprintf(stdout, "%cH\n", allCards[g->heartCards[i] - 2]);
            fflush(stdout);
            alreadyPlayedFlag = 1;
            g->currentPlayer++;
            if(insert_into_array(g->playedHeartCards, g->heartCards[i])) {
                g->playedHeartCardNumber++;
                bubble_sort(g->playedHeartCards, g->playedHeartCardNumber);
            }
            g->heartCards[i] = 0;
            break;
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = g->diamondCardNumber - 1; i >= 0; i--) {
            if(g->diamondCards[i] != 0) {
                fprintf(stdout, "%cD\n", allCards[g->diamondCards[i] - 2]);
                fflush(stdout);
                alreadyPlayedFlag = 1;
                g->currentPlayer++;
                if(insert_into_array(g->playedDiamondCards, 
                        g->diamondCards[i])) {
                    g->playedDiamondCardNumber++;
                    bubble_sort(g->playedDiamondCards, 
                            g->playedDiamondCardNumber);
                }
                g->diamondCards[i] = 0;
                break;
            }
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = g->clubCardNumber - 1; i >= 0; i--) {
            if(g->clubCards[i] != 0) {
                fprintf(stdout, "%cC\n", allCards[g->clubCards[i] - 2]);
                fflush(stdout);
                alreadyPlayedFlag = 1;
                g->currentPlayer++;
                if(insert_into_array(g->playedClubCards, g->clubCards[i])) {
                    g->playedClubCardNumber++;
                    bubble_sort(g->playedClubCards, g->playedClubCardNumber);
                }
                g->clubCards[i] = 0;
                break;
            }
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = g->spadeCardNumber - 1; i >= 0; i--) {
            if(g->spadeCards[i] != 0) {
                fprintf(stdout, "%cS\n", allCards[g->spadeCards[i] - 2]);
                fflush(stdout);
                alreadyPlayedFlag = 1;
                g->currentPlayer++;
                if(insert_into_array(g->playedSpadeCards, g->spadeCards[i])) {
                    g->playedSpadeCardNumber++;
                    bubble_sort(g->playedSpadeCards, g->playedSpadeCardNumber);
                }
                g->spadeCards[i] = 0;
                break;
            }
        }
    }
    
}

/*
 * Function: not_last_to_play
 *
 * decide which card should be played if the current player is not lead
 * and not the last one to play
 *
 * g: struct Game
 *
 * return void
 */ 
void not_last_to_play(Game *g) {
    int i;
    int alreadyPlayedFlag = 0;
    char allCards[13] = {'2', '3', '4', '5', '6', '7', '8', 
            '9', 'T', 'J', 'Q', 'K', 'A'};
    for(i = g->clubCardNumber - 1; i >= 0; i--) {
        if(g->clubCards[i] != 0) {
            fprintf(stdout, "%cC\n", allCards[g->clubCards[i] - 2]);
            fflush(stdout);
            alreadyPlayedFlag = 1;
            g->currentPlayer++;
            if(insert_into_array(g->playedClubCards, g->clubCards[i])) {
                g->playedClubCardNumber++;
                bubble_sort(g->playedClubCards, g->playedClubCardNumber);
            }
            g->clubCards[i] = 0;
            break;
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = g->diamondCardNumber - 1; i >= 0; i--) {
            if(g->diamondCards[i] != 0) {
                fprintf(stdout, "%cD\n", allCards[g->diamondCards[i] - 2]);
                fflush(stdout);
                alreadyPlayedFlag = 1;
                g->currentPlayer++;
                if(insert_into_array(g->playedDiamondCards, 
                        g->diamondCards[i])) {
                    g->playedDiamondCardNumber++;
                    bubble_sort(g->playedDiamondCards, 
                            g->playedDiamondCardNumber);
                }
                g->diamondCards[i] = 0;
                break;
            }
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = g->heartCardNumber - 1; i >= 0; i--) {
            if(g->heartCards[i] != 0) {
                fprintf(stdout, "%cH\n", allCards[g->heartCards[i] - 2]);
                fflush(stdout);
                alreadyPlayedFlag = 1;
                g->currentPlayer++;
                if(insert_into_array(g->playedHeartCards, g->heartCards[i])) {
                    g->playedHeartCardNumber++;
                    bubble_sort(g->playedHeartCards, g->playedHeartCardNumber);
                }
                g->heartCards[i] = 0;
                break;
            }
        }
    }
    if(alreadyPlayedFlag == 0) {
        for(i = g->spadeCardNumber - 1; i >= 0; i--) {
            if(g->spadeCards[i] != 0) {
                fprintf(stdout, "%cS\n", allCards[g->spadeCards[i] - 2]);
                fflush(stdout);
                alreadyPlayedFlag = 1;
                g->currentPlayer++;
                if(insert_into_array(g->playedSpadeCards, g->spadeCards[i])) {
                    g->playedSpadeCardNumber++;
                    bubble_sort(g->playedSpadeCards, g->playedSpadeCardNumber);
                }
                g->spadeCards[i] = 0;
                break;
            }
        }
    }
}

/*
 * Function: print_scores
 *
 * print scores information to stderr
 *
 * g: struct Game
 *
 * return void
 */ 
void print_scores(Game *g) {
    int i;
    fprintf(stderr, "Scores: ");
    fflush(stderr);
    for(i = 0; i < g->numOfPlayer; ++i) {
        if(i > 0) {
            fprintf(stderr, ",");
            fflush(stderr);
        }
        fprintf(stderr, "%d", g->scores[i]);
        fflush(stderr);
    }
    fprintf(stderr, "\n");
    fflush(stderr);
}

/*
 * Function: your_turn_play
 *
 * play a card depends on lead or not, if not the last or not
 *
 * g: struct Game
 *
 * return void
 */ 
void your_turn_play(Game *g) {
    
    if(follow_suit(g) == 0) {
        if(g->currentPlayer == g->numOfPlayer - 1) {
            last_to_play(g);
        } else {
            not_last_to_play(g);
        }
    }
}
    
/*
 * Function: read_hub_message
 *
 * read a message from hub
 *
 * g: struct Game
 *
 * return void
 */ 
void read_hub_message(Game *g) {
    char message[2048];
    char printMessage[21];
    if (fgets(message, 2048, stdin) == NULL) {
        fprintf(stderr, "Unexpected loss of hub\n");
        fflush(stderr);
        exit(4);
    }
    remove_newline(message);
    strncpy(printMessage, message, 20);
    fprintf(stderr, "From hub:%s\n", printMessage);
    fflush(stderr);
    
    if(strncmp("newround", message, 8) == 0) {
        clear_array(g);
        read_card_list(g, message);
        g->nextIsLeadFlag = 1;
        print_hand(g);
        print_played(g);
        print_scores(g);
    } else if(strncmp("played", message, 6) == 0) {
        read_played_card(g, message);
        print_hand(g);
        print_played(g);
        print_scores(g);
    } else if(strncmp("newtrick", message, 8) == 0) {
        play_card_out(g);
        print_hand(g);
        print_played(g);
        print_scores(g);
    } else if(strncmp("trickover", message, 9) == 0) {
        print_hand(g);
        print_played(g);
        print_scores(g);
        g->nextIsLeadFlag = 1;
        g->currentPlayer = 0;
    } else if(strncmp("yourturn", message, 8) == 0) {
        your_turn_play(g);
        print_hand(g);
        print_played(g);
        print_scores(g);
    } else if(strncmp("scores ", message, 7) == 0) {
        read_scores(message + 7, g);
        print_hand(g);
        print_played(g);
        print_scores(g);
    } else if(strcmp("end", message) == 0) {
        exit(0);
    }
}

/*
 * Function: play_game
 *
 * the main game loop
 *
 * g: struct Game
 *
 * return void
 */ 
void play_game(Game *g) {
    while(1) {
        read_hub_message(g);
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
    Game *g = malloc(sizeof(Game));
    check_arguments(argc, argv, g);
    play_game(g);
    return 0;
}

