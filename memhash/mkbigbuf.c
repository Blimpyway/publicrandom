//
// Creates a pseudo randomish big file, just for testing KDF
// on my system is faster than /dev/urandom and 
// we need the same bigbuf.dat everywhere

#include <stdio.h>


// a big 256MByte buffer
#define FSIZE 67108864

// This is the big buffer
unsigned int fbl[FSIZE] = {0};

// previous in sequence
#define rotate15(x) ((x >> 17) | (x << 15))
#define lrotate1(x) ((x >> 1) | (x << 63))
#define rotate1(x) ((x >> 1) | (x << 31))

// one pass through big buffer
unsigned int randround(unsigned int start, unsigned int p1, unsigned int p2) {
    unsigned int finish, prev1, prev2;
    unsigned int t;
    // unsigned long t;
    t = start;
    // how many odd numbers in fbl, just a feedback for "randomness"
    unsigned int countodds = 0; 
    finish = start = start % FSIZE;
    prev1 = (start + FSIZE - 1) % FSIZE;
    prev2 = (prev1 + FSIZE - 1) % FSIZE;
    printf("Start randround(%08u) :", start);

    fbl[prev1] += p1;
    fbl[prev2] += p2;

    unsigned int pp;
    do {
        fbl[start] = fbl[prev1]  + rotate1(t); t ^=  fbl[prev2] + countodds;
        // t ^= fbl[start] = fbl[prev1] + fbl[prev2] ^ lrotate1(t);
        // t ^= fbl[start] = fbl[prev1] + (fbl[prev2] ^ rotate1(t));
        countodds += (fbl[start] & 1);
        prev2 = prev1; prev1 = start; start = (start + 1) % FSIZE;
    } while(start != finish);

    printf(" ** %+1.6f%% odd ints above 50%% \n", 
        100.0 * (((float) countodds / FSIZE) - 0.5)
        );

    // The return value is used as a "random" seed for next randround()
    return (unsigned int) t ;
}

// This will count bit pairs.. bit(N) = bit(N+1) .. should converge to 50%
long pbits = 0;
unsigned int pairbits(unsigned int n, unsigned int prev) {
    int i;
    for (i = 0; i < 32; i++) {
       pbits += ((n & 1) == prev);
       prev = n & 1;
       n = n >> 1;
    }
    return prev;
}

long countbits() {
    unsigned int prev = 0;
    int i;
    printf("\nCounting bit pairs.. "); fflush(stdout);
    for (i = 0; i < FSIZE; i++) {
       prev = pairbits(fbl[i], prev);
    } 
    float bitdif = 100.0 / (FSIZE * 16) * (pbits - FSIZE * 16);
    printf("%ld extra pairs, %2.6f%%\n", pbits - FSIZE * 16, bitdif);
    return pbits;
}

unsigned int hexcounts[16] = {}; 
unsigned int hexpairs() { 
    unsigned int pos, j, w, pairs = 0, next;
    unsigned int prev = (fbl[FSIZE - 1] >> 28);
    printf ("\nhex char stats, count variation from average %d : \n", FSIZE/2);
    for (pos = 0; pos < FSIZE; pos++) {
        w = fbl[pos];
        for(j = 0; j < 8; j++) {
            next = w & 0xf;
            w = w >> 4;
            hexcounts[next]++;
            pairs += (prev == next);
            prev = next;
        }
    }
   
    for(j = 0; j < 16; j++) {
        printf("%x:%+6d %s", 
                j, 
                hexcounts[j] - FSIZE / 2, 
                (j % 8) == 7 ? "\n" : "|"
              );
    }
    printf("\n1/%2.4f doubled (next as previous) hex chars\n\n", 
            FSIZE * 8.0 / pairs);
}

void savefile(char *fname) {
    FILE *fb;
    fb = fopen(fname,"wb");
    fwrite(fbl, FSIZE, sizeof(unsigned int), fb);
    close(fb);
    printf("\n%lu bytes saved in %s\n", FSIZE*sizeof(fbl[0]), fname);
}

int main(int argc, char* argv[]) {
    // file to save a "scrambled" randround sequence
    char *fname = "bigbuf.dat";
    if(argc > 1) fname = argv[argc-1];

    unsigned int i, j = 7; // 7 in my sleeve
    for (i = 0; i < 7; i++) {
        printf("%3u - ", i + 1);
        j ^= randround(j, i, j + i);
    }

    // print first 8 elements to spot any peculiar "un-random" properties, 
    // just a sign of something really broken
    /*
    printf("\n%08x %08x %08x %08x %08x %08x %08x %08x\n",
            fbl[0], fbl[1], fbl[2], fbl[3],
            fbl[4], fbl[5], fbl[6], fbl[8]
          );
    */


    savefile(fname); 
    //
    //  The follwing two are just statistical verifiers:
    //
    //  This counts how often next bit is the same as prev bit, should be
    //  close to equal:
    //
    countbits();
    //
    //  This utility counts the "hex" digits in the big buffer. That means every
    //  4 bits. Shows how many above average and how many 4bit digit doubles are 
    //  in the data set. So far frequency of doubles is close to 1/16 and had no
    //  unusual too many or too low - at an expected 32M each, variations are in
    //  +/-10000 for each digit. That's less than 0.05% 
    //
    hexpairs();
    return 0;
}
