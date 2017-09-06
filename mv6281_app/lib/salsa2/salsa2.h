#ifndef _SALSA2_H_
#define _SALSA2_H_

#ifdef USE_MV98DX324
#define SALSA2_NUM_OF_PORTS     24
#else
#define SALSA2_NUM_OF_PORTS     16
#endif


#define PORT_AUTONEGO_CONFIG_REG        0x1000000c
#define PORT_AUTONEGO_CONFIG_STEP       0x400

#define PORT_STATUS_REG         	0x10000010
#define PORT_STATUS_STEP        	0x400

#define PORTS_VID_REG              	0x02000004
#define PORTS_VID_STEP                  0x2000
#define INTERNAL_VID_ENTRY_REG		0x0a002000
#define INTERNAL_VID_ENTRY_STEP         0x10
#define VID_TO_INTERNAL_VID_REG		0x0a000000

#define FDB_BASE_ADDR_REG   		0x06100000

#define SALSA2_MIB_CONTROL1_REG	0x04004020 	// port0 ~ port5
#define SALSA2_MIB_CONTROL2_REG	0x04804020 	// port6 ~ port11
#define SALSA2_MIB_CONTROL3_REG	0x05004020 	// port12 ~ port17
#define SALSA2_MIB_CONTROL4_REG	0x05804020 	// port18 ~ port23
#define MIB_CONTROL_CAPTURE_PORT_MASK	0xe
#define MIB_CONTROL_CAPTURE_PORT	0x1
#define MIB_CONTROL_CAPTURE_TRIGGER	0x0

#define SALSA2_MIB_COUNTER1_REG	0x04010000 	// port0 ~ port5
#define SALSA2_MIB_COUNTER2_REG	0x04810000 	// port6 ~ port11
#define SALSA2_MIB_COUNTER3_REG	0x05010000 	// port12 ~ port17
#define SALSA2_MIB_COUNTER4_REG	0x05810000 	// port18 ~ port23
#define MIB_COUNTER_PORT_STEP	0x80
#define MIB_COUNTER_FIELD_STEP	0x4

#define SALSA2_MULTICAST_GROUP_REG	0x0a004000
#define SALSA2_MULTICAST_GROUP_STEP	0x4

#define SALSA2_INGRESS_MIRRORING_REG	0x06000060
#define 	RX_SNIFF_TARGET_PORT	11
#define 	RX_SNIFF_TARGET_MASK	(0x1f << RX_SNIFF_TARGET_PORT);

#define SALSA2_BRIDGE_PORT_CONTROL_REG	0x02000000
#define		SNIFFER_ENABLE       	24
#define		TRUNK_GROUP_ID	    	0  
#define		TRUNK_GROUP_ID_MASK   	0x1f
#define SALSA2_BRIDGE_PORT_CONTROL_STEP	0x1000

#define SALSA2_TRANSMIT_SNIFFER_REG	0x01800008
#define		TX_SNIFF_DEST_PORT	7	
#define		TX_SNIFF_DEST_MASK	(0x1f << TX_SNIFF_DEST_PORT);	

#define SALSA2_PORT_TRANSMIT_CONFIG_REG	0x01800000
#define		PORT_EGRESS_MIRR_EN	10
#define SALSA2_PORT_TRANSMIT_CONFIG_STEP	0x1000

#ifdef USE_TRUNK
#define SALSA2_TRUNK_TABLE_REG   	0x02040170 /*number of member in trunk , max 8 members */
#define SALSA2_TRUNK_TABLE_GROUP_MASK   0xf        /* 8 groups in 1 register */
#define SALSA2_TRUNK_TABLE_GROUP_SHIFT  4
#define SALSA2_TRUNK_TABLE_GROUP_COUNT 	8
#define SALSA2_TRUNK_TABLE_STEP   	0x4

#define SALSA2_TRUNK_MEMBER_TABLE_REG  	  0x06000100  /* 1<=n<32 */
#define SALSA2_TRUNK_MEMBER_PORT_MASK     0x1f
#define SALSA2_TRUNK_MEMBER_DEV_SHIFT     5
#define SALSA2_TRUNK_MEMBER_DEV_MASK     (0x1f << SALSA2_TRUNK_MEMBER_DEV_SHIFT)
#define SALSA2_TRUNK_MEMBER_PORTDEV_SHIFT 10
#define SALSA2_TRUNK_MEMBER_PORTDEV_MASK (SALSA2_TRUNK_MEMBER_PORT_MASK | SALSA2_TRUNK_MEMBER_DEV_MASK)
#define SALSA2_TRUNK_MEMBER_PORT_COUNT 	  2
#define SALSA2_TRUNK_MEMBER_TABLE_STEP 	  0x10

#define SALSA2_TRUNK_DESIGNATED_PORTS_HASH_REG	0x01800288
#define SALSA2_TRUNK_DESIGNATED_PORTS_HASH_STEP	0x1000
#define SALSA2_TRUNK_DESIGNATED_PORTS_HASH_COUNT 16

#define SALSA2_NON_TRUNK_MEMBER_REG	0x01800280  /* 0<=n<32, n=0:null trunk in which all ports are non-members  */
#define SALSA2_NON_TRUNK_MEMBER_STEP	0x1000
#endif


#define VLAN_CONFIG_FILE        "/etc/vlan_config"

#ifdef USE_TRUNK
#define TRUNK_ID_FILE            "/etc/trunk_id"
#define TRUNK_CONFIG_FILE        "/etc/trunk_config"
#endif

static unsigned short mib_counter_offset[] =
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

static unsigned short mib_counter_support[] =
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

static char mib_counter_name[][32] =
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

extern int mv_salsa2_read_register(unsigned int reg_addr, unsigned int *reg_val);
extern int mv_salsa2_write_register(unsigned int reg_addr, unsigned int reg_val);
extern int mv_salsa2_init(void);
extern int mv_salsa2_regist_mac(unsigned char *mac, unsigned short port, unsigned short vid);
extern int mv_salsa2_fdb_dump(void);
extern int mv_salsa2_port_config(int port, int speed, int duplex);
extern int mv_salsa2_port_status(int port, int *link, int *speed, int *duplex);
extern int mv_salsa2_port_mib(int port, unsigned long long *counter);
extern int mv_salsa2_port_mib_clear(void);
extern int mv_salsa2_port_mib_dump(int port);
extern int mv_salsa2_set_mirroring_port(int port, int enable);
extern int mv_salsa2_set_mirrored_port(int port, int enable);

extern int mv_salsa2_vlan_init(void);
extern int mv_salsa2_vlan_internal_vid_dump(void);
extern int mv_salsa2_vlan_add(char *vname, int port);
extern int mv_salsa2_vlan_remove(char *vname, int port);
extern int mv_salsa2_vlan_apply_system(void);

#ifdef USE_TRUNK
extern int mv_salsa2_trunk_init(void);
extern void mv_salsa2_trunk_hashmap_init(void);
extern int mv_salsa2_trunk_apply_system(int trunk_id, char *tname);
#endif

extern int mv_88F6281_read_register(int port, unsigned int reg_addr, unsigned int *reg_val);
extern int mv_88F6281_write_register(int port, unsigned int reg_addr, unsigned int reg_val);

#endif
