#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#define PCI_VENDOR_NETWORK 0x8086
#define PCI_PRODUCT_NETWORK 0x100e

/* Register Set. (82543, 82544)
 *
 * Registers are defined to be 32 bits and  should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 *
 * RW - register is both readable and writable
 * RO - register is read only
 * WO - register is write only
 * R/clr - register is read only and is cleared when read
 * A - register array
 *
 * All registers are divided by 4 so they can be used as uint32_t[] indices.
 */
#define E1000_CTRL     (0x00000/4)  /* Device Control - RW */
#define E1000_CTRL_DUP (0x00004/4)  /* Device Control Duplicate (Shadow) - RW */
#define E1000_STATUS   (0x00008/4)  /* Device Status - RO */
#define E1000_EECD     (0x00010/4)  /* EEPROM/Flash Control - RW */
#define E1000_EERD     (0x00014/4)  /* EEPROM Read - RW */
#define E1000_CTRL_EXT (0x00018/4)  /* Extended Device Control - RW */

#endif	// JOS_KERN_E1000_H
