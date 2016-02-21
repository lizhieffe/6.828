# Lab 6

## Exercise 1

To take into account multiple CPUs all generating time interrupts, I introduced an array of tick counts, where the index in the array is the number of the CPU that generated the time interrupt. To associate tick counts with CPU numbers, I changed some of the methods in `time.c` to accept the CPU number as an argument.

## Exercises 2 & 3

Read/skimmed the relevant chapters of the Intel manual. Added the network vendor and product ID constants to the `e1000.h` file and added a function to attach and enable the network card. I chose to keep the network attach function in the `pci.c` file because that is where the bridge attach function also lives.

## Exercise 4

Mapped the register base memory in `MMIOBASE` -- `MMIOLIM` range and added a couple of register offsets to `e1000.h`. I used the same trick as in `lapic.c`, of dividing the offset by 4 so it can be used as an index in the register array.

## Exercise 5

I decided to statically allocated memory for the transmission queue and the packet buffers. I also moved the `pic_network_attach()` function over to the `e1000.c` file because it started growing and became became very E1000 specific.
