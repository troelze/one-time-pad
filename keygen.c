/* Keygen
* Programmed by: Tres Oelze
* Description: This program creates a key file of specified length. 
* Course Name: CS 372 - 400
* Last Modified: 3/17/2019
*/

#include <stdio.h>
#include <stdlib.h>

/**********************************************
 * rand_char() generates a random character or space
 * takes no arguments and returns the random character. 
 * ********************************************/
char rand_char()
{
    
    char randomletter;
    int randnum = random() % 27;
    if (randnum == 26){
        randomletter = 'A' - 33;
    } else {
        randomletter = 'A' + randnum;
    }
    
    return randomletter;
}


int main (int argc, char *argv[])
{
    time_t t;
    srand((unsigned) time(&t));

    //check for valid input (single positive integer)
    if (argc != 2) {fprintf(stderr, "You must enter a single integer\n"); exit(0); }
    char *input_str = argv[1];
    int keylength = atoi(input_str);
    if (keylength < 0) {fprintf(stderr, "You must enter a positive integer\n"); exit(0); }

    //fill array of characters with random chars and print
    int len = keylength+1;
    char key[len];
    int i;
    for (i = 0; i < keylength; i++)
    {
        key[i] = rand_char();
    }

    key[keylength] = '\n';
    fprintf( stdout, key); 
    fflush(stdout);

    return 0;

}

