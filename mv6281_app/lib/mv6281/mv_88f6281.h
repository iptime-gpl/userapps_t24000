#ifndef _MV_88F6281_H_
#define _MV_88F6281_H_

#define MV_88F6281_PORT0_MIB_COUNTER_REG	0x73000
#define MV_88F6281_PORT1_MIB_COUNTER_REG	0x77000
#define MIB_COUNTER_FIELD_STEP	0x4

static unsigned short mv_mib_counter_offset[] =
{
	0x0,	// Good Octets Rx	// 64 bits
	0x4,	// Good Octets Rx	// 64 bits
	0x8,	// Bad Octets Rx
	0xc,	// MAC Trans Err
	0x10,	// Good Frames Rx
	0x14,	// Bad Frames Rx
	0x18,	// Broadcast Frames Rx	
	0x1c,	// Multicast Framse Rx
	0x20,	// 64 Octets
	0x24,	// 65~127 Octets
	0x28,	// 128~255 Octets
	0x2c,	// 256~511 Octets
	0x30,	// 512~1023 Octets
	0x34,	// 1024~max Octets
	0x38,	// Good Octets Tx	// 64 bits
	0x3c,	// Good Octets Tx	// 64 bits
	0x40,	// Good Frames Tx
	0x44,	// Excessive Collision
	0x48,	// Multicast Framse Tx
	0x4c,	// Broadcast Frames Tx	
	0x50,	// Unrecog MAC Control Rx
	0x54,	// FC Sent
	0x58,	// Good FC Rx
	0x5c,	// Bad FC Rx
	0x60,	// Undersize
	0x64,	// Fragments
	0x68,	// Oversize
	0x6c, 	// Jabber
	0x70,	// MAC Rcv Error
	0x74,	// Bad CRC
	0x78,	// Collisions
	0x7c,	// Late Collision
	0xff,	// END
};

static unsigned short mv_mib_counter_support[] =
{
	2,	// Good Octets Rx	// 64 bits
	3,	// Good Octets Rx	// 64 bits
	1,	// Bad Octets Rx
	1,	// MAC Trans Err
	1,	// Good Frames Rx
	1,	// Bad Frames Rx
	1,	// Broadcast Frames Rx	
	1,	// Multicast Framse Rx
	1,	// 64 Octets
	1,	// 65~127 Octets
	1,	// 128~255 Octets
	1,	// 256~511 Octets
	1,	// 512~1023 Octets
	1,	// 1024~max Octets
	2,	// Good Octets Tx	// 64 bits
	3,	// Good Octets Tx	// 64 bits
	1,	// Good Frames Tx
	1,	// Excessive Collision
	1,	// Multicast Framse Tx
	1,	// Broadcast Frames Tx	
	1,	// Unrecog MAC Control Rx
	1,	// FC Sent
	1,	// Good FC Rx
	1,	// Bad FC Rx
	1,	// Undersize
	1,	// Fragments
	1,	// Oversize
	1, 	// Jabber
	1,	// MAC Rcv Error
	1,	// Bad CRC
	1,	// Collisions
	1,	// Late Collision
	0xff,	// END
};

static char mv_mib_counter_name[][32] =
{
	"Good OctetsLow Rx",
	"Good OctetsHigh Rx",
	"Bad Octets Rx",
	"MAC Trans Err",
	"Good Frames Rx",
	"Bad Frames Rx",
	"Broadcast Frames Rx",
	"Multicast Framse Rx",
	"64 Octets",
	"65~127 Octets",
	"128~255 Octets",
	"256~511 Octets",
	"512~1023 Octets",
	"1024~max Octets",
	"Good OctetsLow Tx",
	"Good OctetsHigh Tx",
	"Good Frames Tx",
	"Excessive Collision",
	"Multicast Framse Tx",
	"Broadcast Frames Tx",
	"Unrecog MAC Control Rx",
	"FC Sent",
	"Good FC Rx",
	"Bad FC Rx",
	"Undersize",
	"Fragments",
	"Oversize",
	"Jabber",
	"MAC Rcv Error",
	"Bad CRC",
	"Collisions",
	"Late Collision",
};

extern int mv_88f6281_read_register(int port, unsigned int reg_addr, unsigned int *reg_val);
extern int mv_88f6281_write_register(int port, unsigned int reg_addr, unsigned int reg_val);
extern int mv_88f6281_port_mib(int port, unsigned long long *counter);
extern int mv_88f6281_port_mib_clear(void);


#endif
