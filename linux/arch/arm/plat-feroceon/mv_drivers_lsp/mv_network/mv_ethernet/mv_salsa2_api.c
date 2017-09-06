#include "mvCommon.h"  /* Should be included before mvSysHwConfig */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <net/ip.h>

#include "mvOs.h"
#include "dbg-trace.h"
#include "mvSysHwConfig.h"
#include "eth/mvEth.h"
#include "eth-phy/mvEthPhy.h"
#include "ctrlEnv/sys/mvSysGbe.h"

#include "mv_netdev.h"
#include "mv_salsa2_api.h"

#if defined (CONFIG_MV_ETHERNET)
extern int mv_eth_read_mii(unsigned int portNumber, unsigned int MIIReg, unsigned int* value);
extern int mv_eth_write_mii(unsigned int portNumber, unsigned int MIIReg, unsigned int data);
#endif

#define MAX_MGROUP              64
struct mgroup_table_s {
        unsigned char mac[6];
        unsigned short vidx;
};

struct mgroup_table_s mgroup_table[MAX_MGROUP];

static short get_mgroup_index(unsigned char *mac)
{
        short i, idx;

        idx = -1;
        for(i = 0; i < MAX_MGROUP; i++)
        {
                if (memcmp(mgroup_table[i].mac, mac, 6) == 0)
                        return i;
                if ((mgroup_table[i].vidx == 0) && (idx == -1))
                        idx = i;
        }

        return idx;
}

static unsigned short get_mgroup_vidx(unsigned char *mac)
{
        short i;

        for(i = 0; i < MAX_MGROUP; i++)
        {
                if (memcmp(mgroup_table[i].mac, mac, 6) == 0)
                        return mgroup_table[i].vidx;
        }

        return 0;
}


int mv_salsa2_read_reg(unsigned int portNumber, unsigned int switch_reg, unsigned int *switch_val)
{
        unsigned int reg_val, timeout;
        unsigned int msb, lsb;

        *switch_val = 0;
        timeout = 0;

        do
        {
                if (mv_eth_read_mii(portNumber, SALSA2_SMI_READ_WRITE_STATUS_REG, &reg_val) < 0)
		{
			printk("mv_salsa2_read_reg() other cmd check : mv_eth_read_mii() failed \n");
                        return 0;
		}	
                if (timeout++ > SMI_TIMEOUT_COUNTER)
		{
			printk("mv_salsa2_read_reg() other cmd check : timeout (%d) \n", SMI_TIMEOUT_COUNTER);
                        return 0;
		}
        } while (!(reg_val & (1<<SMI_READ_READY_BIT)));

        msb = (switch_reg>>16) & 0xffff;
        lsb = switch_reg & 0xffff;

        if (mv_eth_write_mii(portNumber, SALSA2_SMI_READ_ADDRESS_MSB_REG, msb) < 0)
	{	
		printk("mv_salsa2_read_reg() : mv_eth_write_mii(1) failed \n");
                return 0;
	}
        if (mv_eth_write_mii(portNumber, SALSA2_SMI_READ_ADDRESS_LSB_REG, lsb) < 0)
	{
		printk("mv_salsa2_read_reg() : mv_eth_write_mii(2) failed \n");
                return 0;
	}

        do
        {
                if (mv_eth_read_mii(portNumber, SALSA2_SMI_READ_WRITE_STATUS_REG, &reg_val) < 0)
		{	
			printk("mv_salsa2_read_reg() cmd rsp check : mv_eth_read_mii() failed \n");
                        return 0;
		}
                if (timeout++ > SMI_TIMEOUT_COUNTER)
		{
			printk("mv_salsa2_read_reg() cmd rsp check : timeout (%d) \n", SMI_TIMEOUT_COUNTER);
                        return 0;
		}
        } while (!(reg_val & (1<<SMI_READ_READY_BIT)));

        if (mv_eth_read_mii(portNumber, SALSA2_SMI_READ_DATA_MSB_REG, &msb ) < 0)
	{
		printk("mv_salsa2_read_reg() : mv_eth_write_mii(3) failed \n");
                return 0;
	}
        if (mv_eth_read_mii(portNumber, SALSA2_SMI_READ_DATA_LSB_REG, &lsb ) < 0)
	{
		printk("mv_salsa2_read_reg() : mv_eth_write_mii(4) failed \n");
                return 0;
	}

        *switch_val = ((msb & 0xffff) << 16) | (lsb & 0xffff);

        return 1;
}

int mv_salsa2_write_reg(unsigned int portNumber, unsigned int switch_reg, unsigned int switch_val)
{
        unsigned int reg_val, timeout;
        unsigned int msb, lsb;

        timeout = 0;

        do
        {
                if (mv_eth_read_mii(portNumber, SALSA2_SMI_READ_WRITE_STATUS_REG, &reg_val) < 0)
                        return 0;
                if (timeout++ > SMI_TIMEOUT_COUNTER)
                        return 0;
        } while (!(reg_val & (1<<SMI_WRITE_DONE_BIT)));

        msb = (switch_reg>>16) & 0xffff;
        lsb = switch_reg & 0xffff;

        if (mv_eth_write_mii(portNumber, SALSA2_SMI_WRITE_ADDRESS_MSB_REG, msb) < 0)
                return 0;
        if (mv_eth_write_mii(portNumber, SALSA2_SMI_WRITE_ADDRESS_LSB_REG, lsb) < 0)
                return 0;

        msb = (switch_val>>16) & 0xffff;
        lsb = switch_val & 0xffff;

        if (mv_eth_write_mii(portNumber, SALSA2_SMI_WRITE_DATA_MSB_REG, msb) < 0)
                return 0;
        if (mv_eth_write_mii(portNumber, SALSA2_SMI_WRITE_DATA_LSB_REG, lsb) < 0)
                return 0;

        return 1;
}

int mv_salsa2_mac_addr_entry_write(mac_entry_s *entryPtr)
{
	unsigned int hwData[MAC_TABLE_ENTRY_SIZE];
	unsigned char isTrunk = 0;
	unsigned char multiple = 0;
	unsigned int msgFromCpuReg3;
	int retry = 0;

	// Message from CPU register 0 (0x06000040) - 31:0 of Word 0
	// Message from CPU register 1 (0x06000044) - 63:32 of Word 0
	// Message from CPU register 2 (0x06000048) - 31:0 of Word 1
	// Message from CPU register 3 (0x0600004c) - 62:32 of Word 1

	/* build entry hw format */
	hwData[0] = 0; hwData[1] = 0;
	hwData[2] = 0; hwData[3] = 0;

	hwData[0] = (0x2 |       /* Message ID Always 0x2 */
	            (0x0 << 4) | /* Message Type 0 = New address */
		    (0x1 << 7) | /* Reserved 0x1 */ 	
		    (0x0 << 15) |
	            (MAC_LOW16(entryPtr->macAddr) << 16));

	hwData[1] = MAC_HIGH32(entryPtr->macAddr);

	hwData[2] = ((entryPtr->vlanId) | 
	             (1 << 13) |         /* set the aging bit */
	             (isTrunk << 14) |   /* trunk */
		     ((entryPtr->port & 0x1f) << 24)|
	             ((entryPtr->vidx & 0x3) << 30));

	hwData[3] = (((entryPtr->vidx >> 2) & 0x7f) |
		     ((entryPtr->srcTc & 0x3) << 12) |
	             ((entryPtr->dstTc & 0x3) << 15) | 
	             (((entryPtr->isStatic == 1) ? 1 : 0) << 18) |
	             (multiple << 19) |
	             (((entryPtr->cosIpv4En == 1) ? 1 : 0) << 21)| /*force l3 cos*/
	             (entryPtr->daCommand << 22) |
	             (entryPtr->saCommand << 24) | 
	             (((entryPtr->mirrorToRxAnalyzerPortEn == 1) ? 1 : 0) << 27) | /* RxSniff */
		     (1 << 31));    /* New Message  Trigger */

	/* wait for the previous write to end */
	do {
        	mv_salsa2_read_reg(0, 0x0600004c, &msgFromCpuReg3);
		if (retry++ > 100)
		{
			printk("Salsa2 msgFromCpuReg3 Error !!!!\n");
			return 0;
		}
	} while ((msgFromCpuReg3 & (1 << 31)) != 0);

	mv_salsa2_write_reg(0, 0x06000040, hwData[0]);
	mv_salsa2_write_reg(0, 0x06000044, hwData[1]);
	mv_salsa2_write_reg(0, 0x06000048, hwData[2]);
	mv_salsa2_write_reg(0, 0x0600004c, hwData[3]);

	return 0;
}

int mv_salsa2_mac_addr_entry_remove(mac_entry_s *entryPtr)
{
	unsigned int hwData[MAC_TABLE_ENTRY_SIZE];
	unsigned char isTrunk = 0;
	unsigned char multiple = 0;
	unsigned int msgFromCpuReg3;
	int retry = 0;

	// Message from CPU register 0 (0x06000040) - 31:0 of Word 0
	// Message from CPU register 1 (0x06000044) - 63:32 of Word 0
	// Message from CPU register 2 (0x06000048) - 31:0 of Word 1
	// Message from CPU register 3 (0x0600004c) - 62:32 of Word 1

	/* build entry hw format */
	hwData[0] = 0; hwData[1] = 0;
	hwData[2] = 0; hwData[3] = 0;

	hwData[0] = (0x2 |       /* Message ID Always 0x2 */
	            (0x3 << 4) | /* Message Type 3 = Aged out address */
		    (0x1 << 7) | /* Reserved 0x1 */ 	
		    (0x0 << 15) |
	            (MAC_LOW16(entryPtr->macAddr) << 16));

	hwData[1] = MAC_HIGH32(entryPtr->macAddr);

	hwData[2] = ((entryPtr->vlanId) | 
	             (1 << 13) |         /* set the aging bit */
	             (isTrunk << 14) |   /* trunk */
		     ((entryPtr->port & 0x1f) << 24)|
	             ((entryPtr->vidx & 0x3) << 30));

	hwData[3] = (((entryPtr->vidx >> 2) & 0x7f) |
		     ((entryPtr->srcTc & 0x3) << 12) |
	             ((entryPtr->dstTc & 0x3) << 15) | 
	             (0 << 18) | // not static
	             (multiple << 19) |
	             (((entryPtr->cosIpv4En == 1) ? 1 : 0) << 21)| /*force l3 cos*/
	             (entryPtr->daCommand << 22) |
	             (entryPtr->saCommand << 24) | 
	             (((entryPtr->mirrorToRxAnalyzerPortEn == 1) ? 1 : 0) << 27) | /* RxSniff */
		     (1 << 31));    /* New Message  Trigger */

	/* wait for the previous write to end */
	do {
        	mv_salsa2_read_reg(0, 0x0600004c, &msgFromCpuReg3);
		if (retry++ > 100)
		{
			printk("Salsa2 msgFromCpuReg3 Error !!!!\n");
			return 0;
		}
	} while ((msgFromCpuReg3 & (1 << 31)) != 0);

	mv_salsa2_write_reg(0, 0x06000040, hwData[0]);
	mv_salsa2_write_reg(0, 0x06000044, hwData[1]);
	mv_salsa2_write_reg(0, 0x06000048, hwData[2]);
	mv_salsa2_write_reg(0, 0x0600004c, hwData[3]);

	return 0;
}


int mv_salsa2_regist_mac(unsigned char *mac, unsigned short port, unsigned short vid)
{
	mac_entry_s mac_entry;
	short	midx;

	memset((char *)&mac_entry, 0x0, sizeof(mac_entry_s));

	//CPU MAC registration
	mac_entry.macAddr[0] = mac[0];
	mac_entry.macAddr[1] = mac[1];
	mac_entry.macAddr[2] = mac[2];
	mac_entry.macAddr[3] = mac[3];
	mac_entry.macAddr[4] = mac[4];
	mac_entry.macAddr[5] = mac[5];
	mac_entry.port = (mac[0] == 0x01) ? 0 : (port & 0x7fff);
	mac_entry.isStatic = 1;
	mac_entry.vlanId = vid;
	mac_entry.vidx = (mac[0] == 0x01) ? (port & 0x7fff) : 0x1ff;
	mac_entry.daCommand = (port == 0x1f) ? 3 : 0;
	mac_entry.saCommand = 0;

	midx = get_mgroup_index(mac);

	if (port & 0x8000)
	{
		if (mac[0] == 0x01)
		{
			memset(mgroup_table[midx].mac, 0x0, 6);
			mgroup_table[midx].vidx = 0;
		}
		mv_salsa2_mac_addr_entry_remove(&mac_entry);
	}
	else
	{
		if (mac[0] == 0x01)
		{
			memcpy(mgroup_table[midx].mac, mac, 6);
			mgroup_table[midx].vidx = mac_entry.vidx;
		}
		mv_salsa2_mac_addr_entry_write(&mac_entry);
	}

	printk("%02x-%02x-%02x-%02x-%02x-%02x, port=0x%x, vid=0x%x\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], port, vid);
	printk("%02x-%02x-%02x-%02x-%02x-%02x, me_port=0x%x, me_vidx=0x%x\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac_entry.port, mac_entry.vidx);

	return 0;
}

#define STP_STATE_GROUP_ENTRY_BASE	0x0a006000
#define STP_STATE_GROUP_ENTRY_SIZE	0x8

int mv_salsa2_set_stp_port_state(int stpidx, int port, int state)
{
	unsigned int reg, value;

	reg = STP_STATE_GROUP_ENTRY_BASE + (STP_STATE_GROUP_ENTRY_BASE * stpidx);
	if (port >= 16) 
	{	
		reg += 0x4;
		port -= 16;
	}	
	/*		Linux 	Salsa2
	LISTENING	1	1	
	LEARNING	2	2
	FORWARDING	3	3
	BLOCKING	4	1
	*/
	if (state == 4) state = 1;

	mv_salsa2_read_reg(0, reg, &value);
	value &= ~(0x3 << (port*2)); // clear old state
	value |= (state << (port*2)); // write new state
	mv_salsa2_write_reg(0, reg, value);

	return 0;
}

int mv_salsa2_convert_dsa_to_8021q_header(struct net_device *dev, struct sk_buff *skb)
{
        if (!strcmp(dev->name, "eth0"))
        {
                unsigned char new_header[4];
                unsigned char *dsa_header = skb->data+14; // 2bytes-padding, 6bytes-dstmac, 6bytes-srcmac
#if 0
                if (skb->data[2] == 0x01 && skb->data[3] == 0x80 &&
                    skb->data[4] == 0xc2 && skb->data[5] == 0x00 &&
                    skb->data[6] == 0x00 && skb->data[7] == 0x00) //BPDU
                {
                        size_t size = (skb->data[18] << 8) | skb->data[19];
                        unsigned char *ptr;

                        memcpy(skb->data+(ETH_ALEN*2)+2,
                                skb->data+(ETH_ALEN*2)+DSA_HLEN + 2,
                                size+2);
                        ptr = skb->data + 2 + ETH_HLEN + size;
                        *ptr++ = 0x0; *ptr++ = 0x0; *ptr++ = 0x0; *ptr = 0x0;

                }
                else
#endif
                {
			int srcPort_srcTrunk;
#if 0
			printk("dsa[0]=%08x, cmd=%02x ", dsa_header[0], dsa_header[0]&0xc0);
			printk("dsa[1]=%08x, trunk=%x ", dsa_header[1], dsa_header[1]&0x04);
			printk("dsa[2]=%08x ", dsa_header[2]);
			printk("dsa[3]=%08x\n ", dsa_header[3]);
#endif
                        new_header[0] = (ETH_P_8021Q >> 8) & 0xff;
                        new_header[1] = ETH_P_8021Q & 0xff;
                        new_header[2] = dsa_header[2] & ~0x10;
			srcPort_srcTrunk = (dsa_header[1]&0xf8)>>3;
			// TagCommand == FORWARD && Source is Trunk
			if (((dsa_header[0]&0xc0) == 0xc0) && (dsa_header[1]&0x04))
			{
				unsigned int reg, value;

				reg = SALSA2_TRUNK_MEMBER_TABLE_REG+SALSA2_TRUNK_MEMBER_TABLE_STEP*(srcPort_srcTrunk-1);
				mv_salsa2_read_reg(0, reg, &value);	
				new_header[3] = (unsigned char)((value & 0x1f) + 1); // first member port of trunk */
			}
			else
                        	new_header[3] = (unsigned char)(srcPort_srcTrunk + 1); // source port

                        if (skb->ip_summed == CHECKSUM_COMPLETE) {
                                __wsum c = skb->csum;
                                c = csum_add(c, csum_partial(new_header + 2, 2, 0));
                                c = csum_sub(c, csum_partial(dsa_header + 2, 2, 0));
                                skb->csum = c;
                        }

                        memcpy(dsa_header, new_header, DSA_HLEN);
                }
        }

	return 1;
}

int mv_salsa2_convert_8021q_to_dsa_header(struct net_device *dev, struct sk_buff *skb)
{
        if (!strcmp(dev->name, "eth0")) // to LAN
        {
                unsigned char *dsa_header = skb->data+12;
                unsigned char port = dsa_header[3]-1;

                if ((dsa_header[0] == 0x08 && dsa_header[1] == 0x00) ||
                    (dsa_header[0] == 0x08 && dsa_header[1] == 0x06))
                {
                        ; // no header
                }
		else
		{
                        // BDPU
                        if (skb->data[0] == 0x01 && skb->data[1] == 0x80 &&
                            skb->data[2] == 0xc2 && skb->data[3] == 0x00 &&
                            skb->data[4] == 0x00 && skb->data[5] == 0x00 && skb->data[18] == 0x42)
                        {
                                dsa_header[0] = 0x40;
                                dsa_header[1] = port << 3; // packet is tx to the designated port.
                                dsa_header[2] = 0x00;
                                dsa_header[3] = 0x01;
                        }
                        else if (skb->data[0] == 0x01 && skb->data[1] == 0x00 && skb->data[2] == 0x5e)
			{
				unsigned short vidx = get_mgroup_vidx(skb->data);

                                dsa_header[0] = 0x40;
				if (vidx) 
				{
					dsa_header[0] |= (vidx >> 4);
                                	dsa_header[1] = ((vidx & 0xf) << 4) | 0x04;
				}
				else
                                	dsa_header[1] = port << 3; // packet is tx to the designated port.
                                dsa_header[2] = 0x00;
                                dsa_header[3] = 0x01;
			}
                        else
                        {
                                dsa_header[0] = 0x40;
                                dsa_header[1] = port << 3; // packet is tx to the designated port.
                                dsa_header[2] = 0x00;
                                dsa_header[3] = 0x01;
                        }
		}
        }

	return 1;
}

void mv_salsa2_dev_init(void)
{
	unsigned int reg, val, i;
	
        printk("============> READ Salsa2 REGs.... phy addr (%x) \n", mvBoardPhyAddrGet(0));
        mv_salsa2_read_reg(0, 0x4C,  &val);
        printk("===> Salsa2 Device ID (0x4c) : %08x \n", val);

        mv_salsa2_read_reg(0, 0x50,  &val);
        printk("===> Salsa2 Vendor ID (0x50) : %08x \n", val);

        mv_salsa2_read_reg(0, 0x0,  &val);
        val |= (1 << 1); //CPUEn;
        val &= ~(1 << 3); //  802.1Q Bridge
        mv_salsa2_write_reg(0, 0x0,  val);
        mv_salsa2_read_reg(0, 0x0,  &val);
        printk("===> Salsa2 Global control (0x0) : %08x \n", val);

        mv_salsa2_read_reg(0, 0xA0,  &val);
        val |= (1 << 12);
        val &= ~(1 << 29);
        val &= ~(1 << 30);
        val |= (2 << 27);
        val |= (1 << 9); //MRU 1522 bytes
        mv_salsa2_write_reg(0, 0xA0,  val);
        mv_salsa2_read_reg(0, 0xA0,  &val);
        printk("===> Salsa2 CPU port control (0xa0) : %08x \n", val);

        mv_salsa2_read_reg(0, 0xD4,  &val);
        val |= (1 << 19); // RGMII v1.8V
        mv_salsa2_write_reg(0, 0xD4,  val);
        mv_salsa2_read_reg(0, 0xD4,  &val);
        printk("===> Salsa2 IO control (0xd4) : %08x \n", val);

        mv_salsa2_read_reg(0, 0xA4,  &val);
        val &= 0xfffffffe; // Link Down
        mv_salsa2_write_reg(0, 0xA4,  val);
        val &= 0xffffffc0;
        val |= 0x001c; // Full Duplex, 1000Mbps
        mv_salsa2_write_reg(0, 0xA4,  val);
        val |= 0x1; // Link Up
        mv_salsa2_write_reg(0, 0xA4,  val);
        mv_salsa2_read_reg(0, 0xA4,  &val);
        printk("===> Salsa2 CPU port Status (0xa4) : %08x \n", val);

        mv_salsa2_read_reg(0, 0x06000000,  &val);
	//val |= (1 << 4); // VlanLookupMode : IVL
	val &= ~(1 << 4); // VlanLookupMode : SVL
	val &= ~(1 << 9); // ForwMACUpdToCPU
        mv_salsa2_write_reg(0, 0x06000000,  val);
        mv_salsa2_read_reg(0, 0x06000000,  &val);
        printk("===> Salsa2 MAC Table Control (0x06000000) : %08x \n", val);

        mv_salsa2_read_reg(0, 0x02040000,  &val);
	val &= ~(0x1 << 24); // ForwCtrlLearnUpdMsg
	val &= ~(0x7 << 1); // ForwMACUpdToCPU, ForwQAToCPU, ForwNAToCPU
        mv_salsa2_write_reg(0, 0x02040000,  val);
        mv_salsa2_read_reg(0, 0x02040000,  &val);
        printk("===> Salsa2 Ingress Control Control (0x06000000) : %08x \n", val);

	for (i = 0; i < 4; i++)
	{	
		reg = 0x04004020 + 0x00800000*i;
		mv_salsa2_read_reg(0, reg, &val);
		val |= (1 << 4); // Don't clear on read
		mv_salsa2_write_reg(0, reg, val);
		mv_salsa2_read_reg(0, reg, &val);
                printk("===> Salsa2 MIB Control %d (0x%08x) : %08x \n", (i+1), reg, val);
	}
	

	{
		unsigned char macAddr[6];

#if	0
		//CPU MAC registration
		macAddr[0] = 0x00; macAddr[1] = 0x26; macAddr[2] = 0x66;
		macAddr[3] = 0xA0; macAddr[4] = 0xB0; macAddr[5] = 0xC0;
		mv_salsa2_regist_mac(macAddr, 0x1f, 1);
#endif

		//BPDU MAC registration
		macAddr[0] = 0x01; macAddr[1] = 0x80; macAddr[2] = 0xc2;
		macAddr[3] = 0x00; macAddr[4] = 0x00; macAddr[5] = 0x00;
		mv_salsa2_regist_mac(macAddr, 0x1f, 1);

#if 0 // Multicast Address TEST , IGMP
		macAddr[0] = 0x01; macAddr[1] = 0x00; macAddr[2] = 0x5e;
		macAddr[3] = 0x40; macAddr[4] = 0x3d; macAddr[5] = 0x88;
		mv_salsa2_regist_mac(macAddr, 257, 1);
		mv_salsa2_write_reg(0, 0x0a004004, 0x4000);	
#endif
	}

	memset((char *)mgroup_table, 0x0, sizeof(struct mgroup_table_s) * MAX_MGROUP);
}

typedef struct _mv_salsa2_data {
	union {
		struct {
			unsigned int port;
			unsigned int num;
			unsigned int value;
		} reg;
		struct {
			unsigned char addr[6];
			unsigned short port;
			unsigned short vid;
		} mac;
	} u;
} mv_salsa2_data;

static int mv_salsa2_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	mv_salsa2_data	data;
	unsigned int reg_base;

	switch (cmd)
	{
	case MV_SALSA2_REG_READ :
		if (copy_from_user(&data, (void *)arg, sizeof(data)))
			return -EFAULT;
		mv_salsa2_read_reg(0, data.u.reg.num, &data.u.reg.value);
		//printk("MV_SALSA2_REG_READ (%08x = %08x\n", data.u.reg.num, data.u.reg.value);
		if (copy_to_user ((void *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	case MV_SALSA2_REG_WRITE :
		 if (copy_from_user(&data, (void *)arg, sizeof(data)))
			return -EFAULT;
		mv_salsa2_write_reg(0, data.u.reg.num, data.u.reg.value);
		break;
	case MV_SALSA2_MAC_REGIST :
		 if (copy_from_user(&data, (void *)arg, sizeof(data)))
			return -EFAULT;
		mv_salsa2_regist_mac(data.u.mac.addr, data.u.mac.port, data.u.mac.vid);
		break;

	case MV_88F6281_REG_READ :
		if (copy_from_user(&data, (void *)arg, sizeof(data)))
			return -EFAULT;
		reg_base = (data.u.reg.port == 0xff) ? 0 : MV_ETH_REG_BASE(data.u.reg.port);
		data.u.reg.value  = MV_REG_READ(reg_base+data.u.reg.num);
		//printk("MV_88F6281_REG_READ (%08x = %08x\n", data.u.reg.num, data.u.reg.value);
		if (copy_to_user ((void *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	case MV_88F6281_REG_WRITE :
		 if (copy_from_user(&data, (void *)arg, sizeof(data)))
			return -EFAULT;
		reg_base = (data.u.reg.port == 0xff) ? 0 : MV_ETH_REG_BASE(data.u.reg.port);
		MV_REG_WRITE(reg_base+data.u.reg.num, data.u.reg.value);
		break;
	default :
		printk(KERN_EMERG "mv_eth_ioctl: un-defined ioctl number\n");
		return -ENOTTY;
	}

	return 0;
}

static int mv_salsa2_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int mv_salsa2_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations mv_salsa2_fops = {
    ioctl:   mv_salsa2_ioctl,
    open:    mv_salsa2_open,
    release: mv_salsa2_release,
};

static int __init mv_salsa2_init(void)
{
	int result;

	result  = register_chrdev(MV_SALSA2_MAJOR, "mv_salsa2", &mv_salsa2_fops);
	if (result < 0)
	{
		printk(KERN_INFO "mv_salsa2: can't set major number\n");
		return result;
	}

	return 0;
}

static void __exit mv_salsa2_exit(void)
{
	unregister_chrdev(MV_SALSA2_MAJOR, "mv_salsa2");
}

module_init(mv_salsa2_init);
module_exit(mv_salsa2_exit);
