## memhash
a slow time delay function intended to limit its speed by DRAM memory latency

It works by having two buffers:
- a small 8 ints (256bit) key buffer and 
- a very large (up to gigabytes) "entropy" big buffer. 

The scrambling function just changes deterministically at each step:
- an internal single int state "t" mixed from its previous value and
  current key buffer and the entropy
- next element in key buffer
- previous element in big entropy buffer.

So its state is intended to be single threaded (un-paralelizable) and dependent
on all previous states.

### Installing & test run:

Tested on ubuntu
basic requirements should be a gcc compiler and optionally 
- a bash shell 
- git to get all three files in a single command

$ git clone https://github.com/Blimpyway/publicrandom

After having the three files in a folder run: 

$ ./memhash.sh 

it just compiles the .c sources,
generates the big "entropy" buffer and runs memhash 
          
### Files:

memhash.c - the actual time delay function, compiles to memhash binary 
mkbigbuf.c - a pseudo-random generator for a large initial chaotic buffer
memhash.sh - a shell script that just compiles and runs a test.


### compiling memhash

$ gcc -O3 -o memhash memhash.c 

### Command line:

$ memhash <num_rounds> <key_seed> 

Where:

num_rounds is the number of scramble rounds. Each round involves a randomised 
  read/write in a large (dozens of megabytes to gigabytes) array while
  also updates the key. My system's average is 88-100ns/round for >200MB status
  array. That's close to Geekbench memory latency of 82ns.

key_seed = a 256 bit as hexadecimal string from which will compute a
  256 biy delayed key.

Environment: 
MEMHASHFILE - memhash assumes there is a large 256M binary file called 
bigbuf.dat as source for deterministic chaos.
A different initial entropy file can be specified via MEMHASHFILE environment
variable.

MEMHASHSIZE - how big the status array to be used. Each element is a 32bit 
  integer, so the actual size will be 4 times this number. For sake of doing
  some statistics, this buffer is coppied (kept a copy) to find out how much
  the buffer was changed by the key scrambling 


### mkbigbuf - a pseudo random generator for 256MByte buffer
Compiling it: 
$gcc -O3 -o mkbigbuf mkbigbuf.c

Compiles the pseudorandom generator for bigbuf.dat file. 
It is automatically compiled and executed by memhash.sh 
if no local bigbuf.dat exist

Running: 
$ ./mkbigbuf [dat_file_name]

if dat_file_name is not specified, it will create a file named bigbuf.dat

It includes some crude checks on randomness, like 
- number of odd/even 32bit integers in the big buffer
- number of bit changes (next bit different than previous)
- counts 4bit 0..f digits
- and 4bit pairings (how often a digit is followed by the same digit). 

### memhash.sh
a utility frontend  to play with different buffer sizes and other
runtime parameters. Just follow its comments. 


