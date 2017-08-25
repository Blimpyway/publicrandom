/*
 * memhash.c - a time delay function throttled by memory latency
 *
 * Its purpose is to compute a delayed key, that means it doesn't
 * have to "protect" the key longer than the time it is required to run.
 * 
 * Having a huge chaotic state should make it safe for this purpose.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



// here are kept the 256 bit input key, scranbling state and output key
unsigned int keybuf[] = {0, 0, 0, 0, 0, 0, 0, 0} ;



// the bigbuf is a large array of random 32 bit ints, the impacts is a copy
// of it used only for statistics (counting changes).
// They are allocated in main() 
unsigned int *bigbuf, *impacts;

// Updates "keybuf" - where input, scrambling and output states are kept
void set_key(char *hexkey) {
    sscanf(hexkey, "%08x%08x%08x%08x%08x%08x%08x%08x",
        &keybuf[0], &keybuf[1], &keybuf[2], &keybuf[3], 
        &keybuf[4], &keybuf[5], &keybuf[6], &keybuf[7]); 
}

// Different "working sizes" 49978001 104729 27644437
// Not sure if prime number size has any impact on randomness
// If small enough to fit in cache it will run much faster
// Pulled off my sleeve a prime smaller than 64M ints which 
// will occupy almost 256MByte memory space for each bigbuf and impacts
unsigned int numk = 67108859 ; 

void reset_impacts() {
    memcpy(bigbuf, impacts, numk * sizeof(bigbuf[0]));
}

// returns 0 on failing to load the big array of ints
int load_bigbuf(char *fileName) {
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL) return 0;
    unsigned int numb = fread(impacts, sizeof(unsigned int), numk, fp);
    fclose(fp);
    reset_impacts();
    return numb;
}

// Shows how much of the bigbuf was changed.
unsigned int numimpacts = 0;
float zero_impacts() {
	unsigned int i;
    numimpacts = 0;
	for (i = 0; i < numk; i++) {
        numimpacts += (impacts[i] != bigbuf[i]); 
	}
	return (float) numimpacts / (float) numk;
}

void show_keys(char *label) {
    printf("%s %08x %08x %08x %08x %08x %08x %08x %08x\n", label, 
            keybuf[0], keybuf[1], keybuf[2], keybuf[3], 
            keybuf[4], keybuf[5], keybuf[6], keybuf[7]);
}


// Rotate left one bit. MSB becomes LSB
// #define rotate(x) ((x > 0x7FFFFFFF) | (x << 1))
#define rotate(x) ((x >> 31) | (x << 1))
#define rotate5(x) ((x >> 27) | (x << 5))
#define lrotate(x) ((x >> 61) | (x << 3))
// the scrambling function. Reads&writes both keybuf and the big random array
// with large enough bigbuf its speed is limited by memory latency
void scramble(unsigned int numsteps) {
    register  unsigned int k, pos, i, t;
    k = 0; pos = numk - 1; t = 1; i = 0;
    while(i < numsteps) {
        t ^= bigbuf[pos] + ~keybuf[k];
        k = i++ % 8; 
        pos = t % numk; 
        bigbuf[pos] ^= ~t;
        // keybuf[k] ^= t = rotate(t);
        keybuf[k] ^= t = rotate5(t);
    }
}


// hops aimlessly into bigbuf, to compare its speed with shuffling speed
// A speed close to shuffling speed gives a hint of how expensive is 
// actual shuffling processing vs. random memory updates
// if the return value is not used the gcc optimiser may skip it entirely
unsigned int  seekSpeed(unsigned int numsteps) {
	register unsigned int i = 0, pos; 
    pos = numsteps % numk;
	while (i++ < numsteps) {
		pos = (bigbuf[pos] ^ i) % numk;
	}
    return bigbuf[pos];
}


char *usage ="\n\
\tUsage:\n\n \
\t %s <num rounds> <key>\n\n\
\t where <num rounds> is the number of shuffle rounds, \n\
\t and <key> is a hex-encoded 256bit initial key\n\n";

void main (int argc, char* argv[]) {
    unsigned int loopsize;

    if (argc < 3) {
        printf(usage, argv[0]);
        exit(1);
    }

    int LOOPS = 0;
    char *sloops = getenv("MEMHASHLOOPS");
    if (sloops != NULL)  sscanf(sloops, "%d", &LOOPS);
    

    const char *MEMHASHSIZE = (char *) getenv("MEMHASHSIZE");

    if (MEMHASHSIZE != NULL) {
        sscanf(MEMHASHSIZE, "%d", &numk);
        if (LOOPS )   
            printf( "MEMHASHSIZE = %s\n", MEMHASHSIZE);
    } else 
        if (!LOOPS) 
            printf ("MEMHASHSIZE not in environment\n");

    bigbuf = malloc(numk * sizeof(unsigned int));
    impacts = malloc(numk * sizeof(unsigned int));

    sscanf(argv[1], "%d", &loopsize);

    set_key(argv[2]);

    if (!LOOPS)
        printf("size of bigbuf is %d bytes, running %d cycles \n\n",  
                numk * 4, loopsize
               );

    char *MEMHASHFILE = getenv("MEMHASHFILE");
    if (MEMHASHFILE == NULL) MEMHASHFILE = "./bigbuf.dat";
    if (!load_bigbuf(MEMHASHFILE)) {
        // exit(1);
        printf("MEMHASHFILE %s not found, using zero-filled big array\n",
                MEMHASHFILE);
    }

    if (LOOPS) {
        int i;
        for (i = 0; i < LOOPS; i++) {
            scramble(loopsize);
            printf("%08x%08x%08x%08x%08x%08x%08x%08x\n",
            keybuf[0], keybuf[1], keybuf[2], keybuf[3], 
            keybuf[4], keybuf[5], keybuf[6], keybuf[7]);
        }
    } else {
        show_keys("input :");
        clock_t stime = clock();
        scramble(loopsize);
        stime = clock() - stime;
        float tseconds = (float) stime / CLOCKS_PER_SEC ;
        show_keys("output:");
        printf("\nDone in %4.4f sec,  %4.2f ns/step, bigbuf %2.5f%% changed,", 
                tseconds,
                (tseconds / loopsize * 1000000000.0), 
                zero_impacts() * 100.0 );
        printf(" %u multi hit\n", loopsize - numimpacts);

        // Measure latency of random reads in bigbuf
        if (loopsize > 2000000) loopsize = 2000000;
        stime = clock();
        unsigned int ss = seekSpeed(loopsize); 
        // Use the result or optimizer skips it
        stime = clock() - stime;
        printf("bigbuf seek latency: %3.2f ns %u\b\b   \n",
                ((float) stime / CLOCKS_PER_SEC / loopsize * 1000000000.0), 
                ss / ss );
    }
    exit(0);
}


