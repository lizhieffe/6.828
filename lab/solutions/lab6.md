# Lab 6

## Exercise 1

To take into account multiple CPUs all generating time interrupts, I introduced an array of tick counts, where the index in the array is the number of the CPU that generated the time interrupt. To associate tick counts with CPU numbers, I changed some of the methods in `time.c` to accept the CPU number as an argument.

## Exercises 2 & 3

Read/skimmed the relevant chapters of the Intel manual. Added the network vendor and product ID constants to the `e1000.h` file and added a function to attach and enable the network card. I chose to keep the network attach function in the `pci.c` file because that is where the bridge attach function also lives.

## Exercise 4

Mapped the register base memory in `MMIOBASE` -- `MMIOLIM` range and added a couple of register offsets to `e1000.h`. I used the same trick as in `lapic.c`, of dividing the offset by 4 so it can be used as an index in the register array.

## Exercise 5

I decided to statically allocated memory for the transmission queue and the packet buffers. I also moved the `pic_network_attach()` function over to the `e1000.c` file because it started growing and became became very E1000 specific.

## Exercise 6

I added the `e1000_transmit()` function, which takes two arguments: a pointer to the packet data and the length of the packet data. It took me some debugging to figure out that it was the software's responsibility to implement the wrap-around of the tail pointer. I also didn't immediately think to set the end of packet bit but soon realized that the buffers allocated will fit the entire packet. The `e1000_transmit()` function returns -1 if the descriptor queue is full. I thought it would be better for the caller to decide on the strategy to use in case no packets could be transmitted. Implementing a strategy in the driver code would force callers into using that specific strategy, resulting in a less composable and extensible design.

## Exercise 7 & 8

The `output()` function loops forever, at each iteration accepting a new transmission request. If the `sys_e1000_transmit()` system call returns `-E_E1000_TXBUF_FULL`, the packet is dropped. The system call will attempt to transmit the packet 20 times before giving up. Note that after each such attempt, `sys_e1000_transmit()` will yield the CPU so the system is not blocked. I think it's a fair trade off to drop a packet after 20 retries as the upper level protocols should be able to handle it.

While working on these two exercises I did run into a "fun" bug. I was bitwise OR-ing the command section of the transmission descriptor with the following hex literal `0x00001000`. The goal was to turn on the 3rd bit (indexed from 0). Except that even though `0x00001000` looks like binary, it's not, it's a hex literal :) So `0x00001000` is 4096 in decimal and `1 0000 0000 0000` in binary. To fix this I bitwise OR-ed the register with the hex literal `0x8`, which is `1000` in binary and everything worked as expected.

## Exercise 9 & 10

I modeled the receive initialization code after the transmit initialization code. I statically allocate the receive packet buffers and receive descriptors. The receive descriptors are then set to point to the receive buffers. The remaining initialization is done by turning bits on/off using bitwise OR/AND. I chose packet buffers large enough to hold the biggest possible Ethernet packet (2048 byte buffers vs 1518 byte Ethernet packet) and disabled long packet mode. All in all initialization was straight forward except for the MAC byte ordering, that took me a couple of tries to get right.
