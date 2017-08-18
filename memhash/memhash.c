#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



// here are kept the 256 bitinput key, shuffling state and output key
unsigned int hpipe[] = {0, 0, 0, 0, 0, 0, 0, 0} ;



// the biglist is a large array of random 32 bit ints, the impacts is a copy
// of it used only for statistics (counting changes)
// They are allocated in main() 
unsigned int *biglist, *impacts;

// Different "working sizes" 49978001 104729 27644437
// Not sure if prime number size has any impact on randomness
// If small enough to fit in cache it will run much faster
// Updates "hpipe" - where input, shuffling and output states are kept
void set_key(char *bignum) {
    sscanf(bignum, "%08x%08x%08x%08x%08x%08x%08x%08x",
        &hpipe[0], &hpipe[1], &hpipe[2], &hpipe[3], 
        &hpipe[4], &hpipe[5], &hpipe[6], &hpipe[7]); 
}

unsigned int numk = 49978001 ; 

void reset_impacts() {
    memcpy(biglist, impacts, numk * sizeof(biglist[0]));
}

// returns 0 on failing to load the big array of ints
int load_biglist(char *fileName) {
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL) return 0;
    unsigned int numb = fread(impacts, sizeof(unsigned int), numk, fp);
    fclose(fp);
    reset_impacts();
    return numb;
}

// Shows how much of the biglist was changed.
unsigned int numimpacts = 0;
float zero_impacts() {
	unsigned int i;
    numimpacts = 0;
	for (i = 0; i < numk; i++) {
        numimpacts += (impacts[i] != biglist[i]); 
	}
	return (float) numimpacts / (float) numk;
}

void show_keys(char *label) {
    printf("%s %08x %08x %08x %08x %08x %08x %08x %08x\n", label, 
            hpipe[0], hpipe[1], hpipe[2], hpipe[3], 
            hpipe[4], hpipe[5], hpipe[6], hpipe[7]);
}


// Rotate left one bit. MSB becomes LSB
#define rotate(x) ((x > 2147483647) + x << 1)

// the shuffling function. Alters both hpipe and the big random array
void shuffle(unsigned int numsteps) {
    register  unsigned int pp, pos, t, i;
    pp = 0; pos = numk - 1; t = 1; i = 0;
    while(i < numsteps) {
        pp = i++ % 8;
        t ^= biglist[pos] + hpipe[pp];
        pos = t % numk;
        biglist[pos] ^= rotate(t);
        hpipe[pp] ^= t;
    }
}

// Just measures random lookups into biglist
// if the return value is not used the gcc optimiser may skip it entirely
unsigned int  seekSpeed(unsigned int numsteps) {
	register unsigned int i, pos; 
    i = 0; pos = numsteps % numk;
	while (i < numsteps) {
		pos = (biglist[pos] + i++) % numk;
	}
    return biglist[pos];
}

int LOOPS = 0;

void main (int argc, char* argv[]) {
    unsigned int loopsize;

    char *nloops = getenv("MEMHASHLOOPS");
    if (nloops != NULL)  sscanf(nloops, "%d", &LOOPS);
    

    const char *MEMHASHSIZE = (char *) getenv("MEMHASHSIZE");

    if (MEMHASHSIZE != NULL) {
        sscanf(MEMHASHSIZE, "%d", &numk);
        if (LOOPS )   
            printf( "MEMHASHSIZE = %s\n", MEMHASHSIZE);
    } else 
        if (!LOOPS) 
            printf ("MEMHASHSIZE undeclared in environment\n");

    biglist = malloc(numk * sizeof(unsigned int));
    impacts = malloc(numk * sizeof(unsigned int));

    sscanf(argv[1], "%d", &loopsize);

    set_key(argv[2]);

    if (!LOOPS)
        printf("size of biglist is %d bytes, running %d cycles \n\n",  
                numk * 4, loopsize
               );

    char *MEMHASHFILE = getenv("MEMHASHFILE");
    if (MEMHASHFILE == NULL) MEMHASHFILE = "./random.bin";
    if (!load_biglist(MEMHASHFILE)) {
        // exit(1);
        printf("MEMHASHFILE %s not found, using zero-filled big array\n",
                MEMHASHFILE);
    }

    if (LOOPS) {
        int i;
        for (i = 0; i < LOOPS; i++) {
            shuffle(loopsize);
            printf("%08x%08x%08x%08x%08x%08x%08x%08x\n",
            hpipe[0], hpipe[1], hpipe[2], hpipe[3], 
            hpipe[4], hpipe[5], hpipe[6], hpipe[7]);
        }
    } else {
        show_keys("input :");

        clock_t stime = clock();
        // shuffling is the "workhorse" KDF
        shuffle(loopsize);
        stime = clock() - stime;
        printf("Done in %4.5f sec, changed in biglist is %2.5f %%", 
                ((float) stime / CLOCKS_PER_SEC), 
                zero_impacts() * 100.0 );
        printf(" (%d multi hits)\n", loopsize - numimpacts);

        show_keys("output:");

        stime = clock();
        // Just do randomised reads from bignum array without writing
        unsigned int ss = seekSpeed(loopsize);
        stime = clock() - stime;
        printf("Random lookup time: %4.3f sec  -- %d\n",((float) stime / CLOCKS_PER_SEC), ss);
    }
    exit(0);
}


