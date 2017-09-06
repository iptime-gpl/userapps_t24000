#ifndef _MV_SALSA2_API_H_
#define _MV_SALSA2_API_H_

#define SALSA2_SMI_READ_WRITE_STATUS_REG     0x1f
#define         SMI_WRITE_DONE_BIT      1       // High - completed
#define         SMI_READ_READY_BIT      0       // High - completed
#define SALSA2_SMI_WRITE_ADDRESS_MSB_REG             0x0
#define SALSA2_SMI_WRITE_ADDRESS_LSB_REG             0x1
#define SALSA2_SMI_WRITE_DATA_MSB_REG        0x2
#define SALSA2_SMI_WRITE_DATA_LSB_REG        0x3
#define SALSA2_SMI_READ_ADDRESS_MSB_REG      0x4
#define SALSA2_SMI_READ_ADDRESS_LSB_REG      0x5
#define SALSA2_SMI_READ_DATA_MSB_REG         0x6
#define SALSA2_SMI_READ_DATA_LSB_REG         0x7

#define SALSA2_BRIDGE_PORT_CONTROL_REG  0x02000000
#define         TRUNK_GROUP_ID          0
#define         TRUNK_GROUP_ID_MASK     0x1f
#define SALSA2_BRIDGE_PORT_CONTROL_STEP 0x1000

#define SALSA2_TRUNK_TABLE_REG          0x02040170 /*number of member in trunk , max 8 members */
#define SALSA2_TRUNK_TABLE_GROUP_MASK   0xf        /* 8 groups in 1 register */
#define SALSA2_TRUNK_TABLE_GROUP_SHIFT  4
#define SALSA2_TRUNK_TABLE_GROUP_COUNT  8
#define SALSA2_TRUNK_TABLE_STEP         0x4

#define SALSA2_TRUNK_MEMBER_TABLE_REG     0x06000100  /* 1<=n<32 */
#define SALSA2_TRUNK_MEMBER_PORT_MASK     0x1f
#define SALSA2_TRUNK_MEMBER_DEV_SHIFT     5
#define SALSA2_TRUNK_MEMBER_DEV_MASK     (0x1f << SALSA2_TRUNK_MEMBER_DEV_SHIFT)
#define SALSA2_TRUNK_MEMBER_PORTDEV_SHIFT 10
#define SALSA2_TRUNK_MEMBER_PORTDEV_MASK (SALSA2_TRUNK_MEMBER_PORT_MASK | SALSA2_TRUNK_MEMBER_DEV_MASK)
#define SALSA2_TRUNK_MEMBER_PORT_COUNT    2
#define SALSA2_TRUNK_MEMBER_TABLE_STEP    0x10

#define SALSA2_TRUNK_DESIGNATED_PORTS_HASH_REG  0x01800288
#define SALSA2_TRUNK_DESIGNATED_PORTS_HASH_STEP 0x1000
#define SALSA2_TRUNK_DESIGNATED_PORTS_HASH_COUNT 16

#define SALSA2_NON_TRUNK_MEMBER_REG     0x01800280  /* 0<=n<32, n=0:null trunk in which all ports are non-members  */
#define SALSA2_NON_TRUNK_MEMBER_STEP    0x1000

#define MV_SALSA2_MAJOR    201
#define MV_NEVENTS  16

#define MV_IOC_MAGIC  'E'
#define MV_SALSA2_REG_READ         _IOWR(MV_IOC_MAGIC, 211, int)
#define MV_SALSA2_REG_WRITE        _IOWR(MV_IOC_MAGIC, 212, int)
#define MV_88F6281_REG_READ        _IOWR(MV_IOC_MAGIC, 213, int)
#define MV_88F6281_REG_WRITE       _IOWR(MV_IOC_MAGIC, 214, int)
#define MV_SALSA2_MAC_REGIST       _IOWR(MV_IOC_MAGIC, 215, int)

#define SMI_TIMEOUT_COUNTER     (1000*10)

#define DSA_HLEN        4


#define MAX_TABLE_BASE_ADDR 	0x06100000
#define MAC_TABLE_ENTRY_SIZE	4


#define MAC_LOW32(macAddr) \
	(macAddr[5] | (macAddr[4] << 8) | (macAddr[3] << 16) | (macAddr[2] << 24))
#define MAC_HIGH16(macAddr) (macAddr[1] | (macAddr[0] << 8))
#define MAC_LOW16(macAddr)  (macAddr[5] | (macAddr[4] << 8))
#define MAC_HIGH32(macAddr) \
	(macAddr[3] |(macAddr[2] << 8) | (macAddr[1] << 16) | (macAddr[0] << 24))


typedef enum
{
    MAC_TABLE_FRWRD_E = 0,
    MAC_TABLE_DROP_E,
    MAC_TABLE_INTERV_E,
    MAC_TABLE_CNTL_E,
    MAC_TABLE_MIRROR_TO_CPU_E,
    MAC_TABLE_SOFT_DROP_E
} mac_table_cmd;

typedef struct
{
    unsigned char   macAddr[6];
    unsigned short port; // MC - no means
    unsigned char isStatic;
    unsigned short vlanId;
    unsigned short vidx; // UC - 0x1ff,  MC - multicast entry index
    unsigned char  srcTc;
    unsigned char  dstTc;
    mac_table_cmd  daCommand;
    mac_table_cmd  saCommand;
    unsigned char saClass;
    unsigned char daClass;
    unsigned char saCib;
    unsigned char daCib;
    unsigned char daRoute;
    unsigned char cosIpv4En;
    unsigned char mirrorToRxAnalyzerPortEn;
} mac_entry_s;

extern int mv_salsa2_read_reg(unsigned int portNumber, unsigned int switch_reg, unsigned int *switch_val);
extern int mv_salsa2_write_reg(unsigned int portNumber, unsigned int switch_reg, unsigned int switch_val);
extern int mv_salsa2_mac_addr_entry_write(mac_entry_s *entryPtr);
extern int mv_salsa2_convert_dsa_to_8021q_header(struct net_device *dev, struct sk_buff *skb);
extern int mv_salsa2_convert_8021q_to_dsa_header(struct net_device *dev, struct sk_buff *skb);
extern void mv_salsa2_dev_init(void);


#endif
