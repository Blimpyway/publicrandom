# memhash
a slow time delay function intended to limit its speed by DRAM memory latency
           
## Files:

memhash.c - the actual time delay function, compiles to memhash binary 
mkbigbuf.c - a pseudo-random generator for a large initial chaotic buffer
memhash.sh - a shell script that just compiles and runs a test.

$ gcc -O3 -o memhash memhash.c 

## Command line:

$ memhash <num_rounds> <key_seed> 

Where:
num_rounds is the number of scramble rounds. Each round involves a randomised 
  read/write in a large (dozens of megabytes to gigabytes) array while
  also updates the key. My system's average is 88-100ns/round for >200MB status
  array. That's close to Geekbench memory latency of 82ns.

key_seed = a 256 bit as hexadecimal string from which will compute a
  256 biy delayed key.

Environment: 
MEMHASHFILE
memhash assumes there is a large 256M binary file called bigbuf.dat 
A different initial status file can be specified via MEMHASHFILE environment
variable.

MEMHASHSIZE - how big the status array to be used. Each element is a 32bit 
  integer, so the actual size will be 4 times this number. For sake of doing
  some statistics, this buffer is coppied (kept a copy) to find out how much
  the buffer was changed by the key scrambling 


# mkbigbuf - a pseudo random generator for 256MByte buffer
---------------------
$gcc -O3 -o mkbigbuf mkbigbuf.c

Compiles the pseudorandom generator for bigbuf.dat file. 
It is automatically compiled and run by memhash.sh if no local bigbuf.dat exist

It includes some crude checks on randomness, like 
- number of odd/even 32bit integers in the big buffer
- number of bit changes (next bit different than previous)
- counts 4bit 0..f digits
- and 4bit pairings (how often a digit is followed by the same digit). 

# memhash.sh
-----------------------
memhash.sh - a utility frontend  to play with different buffer sizes and other
 runtime parameters. Just follow its comments. 
