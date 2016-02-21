#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>

// LAB 6: Your driver code here

// Memory for transmit queue. Contains space for 32 descriptors.
// Base address of array is 16 byte aligned, as per specification.
// Length is NTXDESC * sizeof(struct tx_desc) = 512, which is 128 byte
// aligned.
struct tx_desc txq[NTXDESC] __attribute__ ((aligned (16)));

// Memory for transmit buffers. Array of size NTXDESC, where each element
// is of type struct packet. No alignment requirements.
struct packet tx_pkts[NTXDESC];

// Base address of memory mapped controller registers
volatile uint32_t *network_regs;

int
pci_network_attach(struct pci_func *pcif)
{
	pci_func_enable(pcif);
	network_regs = mmio_map_region((physaddr_t) pcif->reg_base[0], pcif->reg_size[0]);

	cprintf("status register value: %08x\n", network_regs[E1000_STATUS]);

	// Transmit initialization

	// Set TDBAL (Transmit Descriptor Base Address) to the above allocated address.
	network_regs[E1000_TDBAL] = PADDR(txq);
	network_regs[E1000_TDBAH] = 0x0;

	// Set TDLEN (Transmit Descriptor Length). Must be 128 byte aligned.
	network_regs[E1000_TDLEN] = NTXDESC * sizeof(struct tx_desc);

	// Write 0 to Transmit Descriptor Head (TDH) and to Transmit Descriptor Tail (TDHT)
	network_regs[E1000_TDH] = 0x0;
	network_regs[E1000_TDT] = 0x0;

	// Initialize the Transmit Control Register (TCTL) with the following
	// - set enable bit to 1 (TCTL.EN)
	// - set pad short packets bit to 1 (TCTL.PSP)
	// - set collision threshold to 0x10 (TCTL.CT) ?? See if can be removed
	// - set collision distance to 0x40 (TCTL.COLD)
	network_regs[E1000_TCTL] |= E1000_TCTL_EN;
	network_regs[E1000_TCTL] |= E1000_TCTL_PSP;
	network_regs[E1000_TCTL] |= E1000_TCTL_CT;
	network_regs[E1000_TCTL] |= E1000_TCTL_COLD;

	// Initialize the Transmit IPG register. The TIPG is broken up into three:
	//   -- IPGT, bits 0-9, set to 0xA
	//   -- IPGR1, bits 19-10, set to 0x8 (2/3 * 0xc)
	//   -- IPGR2, bits, 19-20, set to 0xc
	network_regs[E1000_TIPG] |= (0xA << E1000_TIPG_IPGT);
	network_regs[E1000_TIPG] |= (0x8 << E1000_TIPG_IPGR1);
	network_regs[E1000_TIPG] |= (0xC << E1000_TIPG_IPGR2);

	// Zero out the whole transmission descriptor queue
	memset(txq, 0, sizeof(struct tx_desc) * NTXDESC);

	// Initialize the transmit descriptors
	int i;
	for (i = 0; i < NTXDESC; i++) {
		txq[i].addr = PADDR(&tx_pkts[i]); 	// set packet buffer addr
		txq[i].cmd |= E1000_TXD_RS;			// set RS bit
		txq[i].cmd &= ~E1000_TXD_DEXT;		// set legacy descriptor mode
		txq[i].status |= E1000_TXD_DD;		// set DD bit, all descriptors
											// are initally free
	}

	return 1;
}

// Transmit packet stored in memory location pointed to by pkt to the
// network controller.
// Return 0 on success, -1 on full transmission queue.
int
e1000_transmit(char* pkt, size_t length)
{
	if (length > PKT_BUF_SIZE)
		panic("e1000_transmit: size of packet to transmit (%d) larger than max (2048)\n", length);

	size_t tail_idx = network_regs[E1000_TDT];

	// Next descriptor is free if the DD bit is set in the STATUS section of
	// the transmission descriptor.
	if (txq[tail_idx].status & E1000_TXD_DD) {
		memmove((void *) &tx_pkts[tail_idx], (void *) pkt, length);

		// Turn off DD bit
		txq[tail_idx].status &= ~E1000_TXD_DD;

		// Signal end of packet. Our packet buffers are 2048 bytes long,
		// which is larger than the max size of Ethernet packets (1518).
		// So we have one descriptor and one buffer per packet.
		txq[tail_idx].cmd |= E1000_TXD_EOP;

		txq[tail_idx].length = length;

		// Increment tail index
		tail_idx = (tail_idx + 1) % NTXDESC;
		network_regs[E1000_TDT] = tail_idx;

		return 0;
	} else {
		return -1;
	}
}