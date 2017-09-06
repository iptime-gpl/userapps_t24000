/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/
#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/reboot.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>
#include <linux/major.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/seq_file.h>

#include <asm/system.h>
#include <asm/dma.h>
#include <asm/io.h>

#include <linux/netdevice.h>
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "mv_eth_proc.h"

#ifdef CONFIG_MV_ETH_NFP
#include "../nfp_mgr/mv_nfp_mgr.h"
#endif

#include "mv_netdev.h"

//#define MV_DEBUG
#ifdef MV_DEBUG
#define DP printk
#else
#define DP(fmt,args...)
#endif


/* global variables from 'regdump' */
static struct proc_dir_entry *mv_eth_tool;

static unsigned int port = 0, q = 0, weight = 0, status = 0, mac[6] = {0,};
static unsigned int policy =0, command = 0, packet = 0;
static unsigned int value = 0;

#ifdef CONFIG_MV_ETH_NFP
static unsigned int  dip, sip, inport, outport;
static unsigned int  da[6] = {0,}, sa[6] = {0,};
static unsigned int  db_type;
#endif /* CONFIG_MV_ETH_NFP */

void run_com_srq(void) 
{
    void* port_hndl = mvEthPortHndlGet(port);

    if(port_hndl == NULL)
        return;

    if(q >= MV_ETH_RX_Q_NUM)
	    q = -1;

    switch(packet) {
	case PT_BPDU:
		mvEthBpduRxQueue(port_hndl, q);
		break;
	case PT_ARP:
		mvEthArpRxQueue(port_hndl, q);
		break;
	case PT_TCP:
		mvEthTcpRxQueue(port_hndl, q);
		break;
	case PT_UDP:
		mvEthUdpRxQueue(port_hndl, q);
		break;
	default:
		printk("eth proc unknown packet type.\n");	
    }
	
}

extern void    		ethMcastAdd(int port, char* macStr, int queue);
void run_com_sq(void) {

    char mac_addr[20];

    if(q >= MV_ETH_RX_Q_NUM)
	    q = -1;
    
    sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    ethMcastAdd(port, mac_addr, q);
}

extern void    	ethPortStatus (int port);
extern void    	ethPortQueues( int port, int rxQueue, int txQueue, int mode);
extern void    	ethPortMcast(int port);
extern void    	ethPortUcastShow(int port);
extern void    	ethPortRegs(int port);
extern void     ethTxPolicyRegs(int port);
extern void    	ethPortCounters(int port);
extern void 	ethPortRmonCounters(int port);

#ifdef CONFIG_MV_ETH_NFP_DUAL
extern void    eth_remote_port_status_print(int port, int mode);
#endif

void run_com_stats(void) {
	printk("\n\n#########################################################################################\n\n");
	switch(status) {
		case STS_PORT:
			printk("  PORT %d: GET ETH STATUS\n\n",port);
            mv_eth_status_print(port);
			ethPortStatus(port);
			break;

        case STS_PORT_MAC:
            ethPortUcastShow(port);
			ethPortMcast(port);
            break;

		case STS_PORT_Q:
			printk("  PORT %d: GET ETH STATUS ON Q %d\n\n",port,q);
			ethPortQueues(port, q, q, 1);
			break;

#if (MV_ETH_RX_Q_NUM > 1)
		case STS_PORT_RXP:
			printk("  PORT %d: GET ETH RX POLICY STATUS\n\n",port);
			printk("Not supported\n");
			break;
#endif /* MV_ETH_RX_Q_NUM > 1 */

		case STS_PORT_TOS_MAP:
			mv_eth_tos_map_show(port);
			break;

		case STS_PORT_TXP:
			printk("  PORT %d: GET ETH TX POLICY STATUS\n\n",port);
			ethTxPolicyRegs(port);
			break;

		case STS_PORT_REGS:
			printk("  PORT %d: GET ETH PORT REGS STATUS\n\n",port);
			ethPortRegs(port);
			break;

		case STS_PORT_MIB:
			ethPortCounters(port);
			ethPortRmonCounters(port);	
			break;

		case STS_PORT_STATS:
			printk("  PORT %d: GET ETH STATISTIC STATUS\n\n",port);
			mv_eth_stats_print(port);
			break;

        case STS_NETDEV:
			mv_eth_netdev_print(port);
            break;

#ifdef CONFIG_MV_ETH_NFP
		case STS_PORT_NFP_STATS:
			printk("  PORT %d: NFP statistics\n\n",port);
			mv_eth_nfp_stats_print(port);
			break;
#endif /* CONFIG_MV_ETH_NFP */

#ifdef CONFIG_MV_GATEWAY
        case STS_SWITCH_STATS:
            mv_gtw_switch_stats(port);
            break;
#endif /* CONFIG_MV_GATEWAY */

		default:
			printk(" Unknown status command \n");
	}
#ifdef CONFIG_MV_ETH_NFP_DUAL
    eth_remote_port_status_print(port, status);
#endif
}

int run_eth_com(const char *buffer) {

    int scan_count;
    scan_count = sscanf(buffer, ETH_CMD_STRING, ETH_SCANF_LIST);
    if( scan_count != ETH_LIST_LEN) {
	printk("eth command bad format %x != %x\n", scan_count, ETH_LIST_LEN );
	return 1;
    }
    switch(command) {

        case COM_TXDONE_Q:
            mv_eth_tx_done_quota = value;
            break;

#ifdef CONFIG_MV_ETH_SKB_REUSE
        case COM_SKB_REUSE:
            mv_eth_skb_reuse_enable = value;
            break;
#endif /* CONFIG_MV_ETH_SKB_REUSE */

#ifdef CONFIG_NET_SKB_RECYCLE
        case COM_SKB_RECYCLE:
            mv_eth_skb_recycle_enable = value;
	    break;
#endif /* CONFIG_NET_SKB_RECYCLE */

#ifdef CONFIG_MV_ETH_NFP
        case COM_NFP:
	    if (value) 
	    {
		printk("Enabling NFP\n");
		fp_mgr_enable();
	    }
	    else
	    {
		printk("Disabling NFP\n");
		fp_mgr_disable();
	    }
            break;
#endif /* CONFIG_MV_ETH_NFP */

	default:
            printk(" Unknown ETH command \n");
    }
    return 0;
}

/* Giga Queue commands */
int run_port_queue_cmd(const char *buffer) {

        int scan_count;

        scan_count = sscanf(buffer, QUEUE_CMD_STRING, QUEUE_SCANF_LIST);

        if( scan_count != QUEUE_LIST_LEN) {
                printk("eth port/queue command bad format %x != %x\n", scan_count, QUEUE_LIST_LEN );
                return 1;
        }

        switch(command) {
		case COM_TOS_MAP:
			mv_eth_tos_map_set(port, value, q);
			break;
	
		default:
			printk(" Unknown port/queue command \n");
	}
	return 0;
}

/* Giga Port commands */
int run_port_com(const char *buffer) {

	int scan_count;
    void*   port_hndl;

	scan_count = sscanf(buffer, PORT_CMD_STRING, PORT_SCANF_LIST);
    
	if( scan_count != PORT_LIST_LEN) {
		printk("eth port command bad format %x != %x\n", scan_count, PORT_LIST_LEN );
		return 1;
	}
    if( (port < 0) || (port > mvCtrlEthMaxPortGet()) )
        return 1;

    port_hndl = mvEthPortHndlGet(port);
    if(port_hndl == NULL)
        return 1;

    	switch(command) {
        	case COM_RX_COAL:
            	mvEthRxCoalSet(mvEthPortHndlGet(port), value);
            	break;

        	case COM_TX_COAL:
            	mvEthTxCoalSet(mvEthPortHndlGet(port), value);
        	break;

#ifdef ETH_MV_TX_EN
            case COM_TX_EN:
                if(value > CONFIG_MV_ETH_NUM_OF_RX_DESCR)
            {
                    printk("Eth TX_EN command bad param: value=%d\n", value);
                return 1;
            }

                eth_tx_en_config(port, value);
            break;
#endif /* ETH_MV_TX_EN */

#if (MV_ETH_VERSION >= 4)
        	case COM_EJP_MODE:
            		mvEthEjpModeSet(mvEthPortHndlGet(port), value);
            	break;
#endif /* (MV_ETH_VERSION >= 4) */
			case COM_LRO:
				mv_eth_set_lro(port, value);
				break;
			case COM_LRO_DESC:
				mv_eth_set_lro_desc(port, value);
				break;

			case COM_TX_NOQUEUE:
				mv_eth_set_noqueue(port, value);
			break;

  		default:
			printk(" Unknown port command \n");
    	}
   	return 0;
}

#ifdef CONFIG_MV_ETH_NFP
int run_ip_rule_set_com(const char *buffer)
{
    int scan_count, i;
    MV_FP_RULE  ip_rule;
    MV_STATUS   status = MV_OK;

    scan_count = sscanf(buffer, IP_RULE_STRING, IP_RULE_SCANF_LIST);

    if( scan_count != IP_RULE_LIST_LEN) {
	printk("eth proc bad format %x != %x\n", scan_count, IP_RULE_LIST_LEN);
	return 1;
    }
    memset(&ip_rule, 0, sizeof(ip_rule));
    
    printk("run_ip_rule_set_com: dip=%08x, sip=%08x, inport=%d, outport=%d\n", 
            dip, sip, inport, outport);

    ip_rule.routingInfo.dstIp = MV_32BIT_BE(dip);
    ip_rule.routingInfo.srcIp = MV_32BIT_BE(sip);
    ip_rule.routingInfo.defGtwIp = MV_32BIT_BE(dip);
    ip_rule.routingInfo.inIfIndex = inport;
    ip_rule.routingInfo.outIfIndex = outport;
    ip_rule.routingInfo.aware_flags = 0;

    for(i=0; i<MV_MAC_ADDR_SIZE; i++)
    {
        ip_rule.routingInfo.dstMac[i] = (MV_U8)(da[i] & 0xFF);
        ip_rule.routingInfo.srcMac[i] = (MV_U8)(sa[i] & 0xFF);;
    }
    ip_rule.mgmtInfo.actionType = MV_FP_ROUTE_CMD;
    ip_rule.mgmtInfo.ruleType = MV_FP_STATIC_RULE;

    status = fp_rule_set(&ip_rule);
    if(status != MV_OK)
    {
        printk("fp_rule_set FAILED: status=%d\n", status);
    }
    return status;
}

int run_ip_rule_del_com(const char *buffer)
{
    int scan_count;
    MV_STATUS status = MV_OK;

    scan_count = sscanf(buffer, IP_RULE_DEL_STRING, IP_RULE_DEL_SCANF_LIST);

    if( scan_count != IP_RULE_DEL_LIST_LEN) {
	printk("eth proc bad format %x != %x\n", scan_count, IP_RULE_DEL_LIST_LEN);
	return 1;
    }

    status = fp_rule_delete(MV_32BIT_BE(sip), MV_32BIT_BE(dip), MV_FP_STATIC_RULE);
    if(status != MV_OK)
    {
        printk("fp_rule_delete FAILED: status=%d\n", status);
    }
    return status;
}

int run_fp_db_print_com(const char *buffer)
{
    int scan_count;
    MV_STATUS status = MV_OK;

    scan_count = sscanf(buffer, NFP_DB_PRINT_STRING, NFP_DB_PRINT_SCANF_LIST);

    if( scan_count != NFP_DB_PRINT_LIST_LEN) {
	    printk("eth proc bad format %x != %x\n", scan_count, NFP_DB_PRINT_LIST_LEN);
	    return 1;
    }

    if (db_type == DB_ROUTING)
	    status = fp_rule_db_print(MV_FP_DATABASE); 
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT 
    else if (db_type == DB_NAT)
	    status = fp_nat_db_print(MV_FP_DATABASE);
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */
#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT
    else if (db_type == DB_FDB)
            status = fp_fdb_db_print(MV_FP_DATABASE);
#endif /* CONFIG_MV_ETH_NFP_FDB_SUPPORT */
#ifdef CONFIG_MV_ETH_NFP_PPP
    else if (db_type == DB_PPP)
            status = fp_ppp_db_print(MV_FP_DATABASE);
#endif /* CONFIG_MV_ETH_NFP_PPP */
#ifdef CONFIG_MV_ETH_NFP_SEC
    else if (db_type == DB_SEC)
            status = fp_sec_db_print(MV_FP_DATABASE);
#endif /* CONFIG_MV_ETH_NFP_SEC */
    else {
	    printk("Failed to print rule database: unknown DB type\n");
	    return 1;
    }
    return status;
}
#endif /* CONFIG_MV_ETH_NFP */

int run_com_general(const char *buffer) {

	int scan_count;

	scan_count = sscanf(buffer, PROC_STRING, PROC_SCANF_LIST);

	if( scan_count != LIST_LEN) {
		printk("eth proc bad format %x != %x\n", scan_count, LIST_LEN );
		return 1;
	}

	switch(command){
		case COM_SRQ:
			DP(" Port %x: Got SRQ command Q %x and packet type is %x <bpdu/arp/tcp/udp> \n",port,q,packet);
			run_com_srq();
			break;
		case COM_SQ:
			DP(" Port %x: Got SQ command Q %x mac %2x:%2x:%2x:%2x:%2x:%2x\n",port, q, 
				mac[0],  mac[1],  mac[2],  mac[3],  mac[4],  mac[5]);
			run_com_sq();
			break;

#if (MV_ETH_RX_Q_NUM > 1)
		case COM_SRP:
			DP(" Port %x: Got SRP command Q %x policy %x <Fixed/WRR> \n",port,q,policy); 
            printk("Not supported\n");
			break;
		case COM_SRQW:
			DP(" Port %x: Got SQRW command Q %x weight %x \n",port,q,weight);
			printk("Not supported\n");
			break;
		case COM_STP:
			DP("STP cmd - Unsupported: Port %x Q %x policy %x <WRR/FIXED> weight %x\n",port,q,policy,weight); 
			break;
#endif /* MV_ETH_RX_Q_NUM > 1 */

		case COM_STS:
			DP("  Port %x: Got STS command status %x\n",port,status);
			run_com_stats();
			break;
		default:
			printk("eth proc unknown command.\n");
	}
  	return 0;
}

//EFM, ysyoo
#ifdef CONFIG_EFM_ROUTER_PATCH
extern int gpio_direction_output(unsigned gpio, int value);
void efm_debug_reset_cpu(int value)
{
	if (value == 0 || value == 1 )
		gpio_direction_output(44, value);
	else
	{
		int i;
		gpio_direction_output(44, 0);
		for (i=0; i<30000; i++);
		gpio_direction_output(44, 1);
	}
}

extern unsigned int mv_salsa2_read_reg(unsigned int portNumber, unsigned int switch_reg, unsigned int *switch_val);
extern unsigned int mv_salsa2_write_reg(unsigned int portNumber, unsigned int switch_reg, unsigned int switch_val);

void efm_config_98dx3xx_cpu_port(void)
{
	unsigned int val;

        printk("============> Configure 98DX3xx REGs.... phy addr (%x) \n", mvBoardPhyAddrGet(0));
        mv_salsa2_read_reg(0, 0x4C,  &val);
        printk("===> 98DX3xx Device ID (0x4c) : %08x \n", val);

        mv_salsa2_read_reg(0, 0x50,  &val);
        printk("===> 98DX3xx Vendor ID (0x50) : %08x \n", val);

        mv_salsa2_read_reg(0, 0x0,  &val);
        printk("===> 98DX3xx Global control (0x0) : %08x \n", val);
        val |= (1 << 1); //CPUEn;
        mv_salsa2_write_reg(0, 0x0,  val);
        mv_salsa2_read_reg(0, 0x0,  &val);
        printk("===> 98DX3xx Global control after set : %08x \n", val);

        mv_salsa2_read_reg(0, 0xA0,  &val);
        printk("===> 98DX3xx CPU port control (0xa0) : %08x \n", val);
        val &= ~(1 << 12);
        val &= ~(1 << 29);
        val &= ~(1 << 30);
        val |= (2 << 27);
        mv_salsa2_write_reg(0, 0xA0,  val);
        mv_salsa2_read_reg(0, 0xA0,  &val);
        printk("===> 98DX3xx CPU port control after set : %08x \n", val);

        mv_salsa2_read_reg(0, 0xD4,  &val);
        printk("===> 98DX3xx IO control (0xd4) : %08x \n", val);
        val |= (1 << 19); // RGMII v1.8V
        mv_salsa2_write_reg(0, 0xD4,  val);
        mv_salsa2_read_reg(0, 0xD4,  &val);
        printk("===> 98DX3xx IO control after set : %08x \n", val);

        mv_salsa2_read_reg(0, 0xA4,  &val);
        printk("===> 98DX3xx CPU port Status (0xa4) : %08x \n", val);
        val &= 0xfffffffe; // Link Down
        mv_salsa2_write_reg(0, 0xA4,  val);

        val &= 0xffffffc0;
        val |= 0x001c; // Full Duplex, 1000Mbps
        mv_salsa2_write_reg(0, 0xA4,  val);

        val |= 0x1; // Link Up
        mv_salsa2_write_reg(0, 0xA4,  val);

        mv_salsa2_read_reg(0, 0xA4,  &val);
        printk("===> 98DX3xx CPU port Status: after set %08x \n", val);

}

void  efm_debug_dump_register(void)
{
        unsigned int val;
        unsigned short val16;
	int ethPortNo;

        printk("============> READ 98DX3xx REGs.... phy addr (%x) \n", mvBoardPhyAddrGet(0));
        mv_salsa2_read_reg(0, 0x4C,  &val);
        printk("===> 98DX3xx Device ID : %08x \n", val);

        mv_salsa2_read_reg(0, 0x50,  &val);
        printk("===> 98DX3xx Vendor ID : %08x \n", val);

        mv_salsa2_read_reg(0, 0x0,  &val);
        printk("===> 98DX3xx Global control : %08x \n", val);

        mv_salsa2_read_reg(0, 0xA0,  &val);
        printk("===> 98DX3xx CPU port control : %08x \n", val);

        mv_salsa2_read_reg(0, 0xA4,  &val);
        printk("===> 98DX3xx CPU port Status: %08x \n", val);

        mv_salsa2_read_reg(0, 0xD4,  &val);
        printk("===> 98DX3xx IO control : %08x \n", val);

        mv_salsa2_read_reg(0, 0x60,  &val);
        printk("===> 98DX3xx CPU good sent : %08x \n", val);
        mv_salsa2_read_reg(0, 0x64,  &val);
        printk("===> 98DX3xx CPU bad sent : %08x \n", val);
        mv_salsa2_read_reg(0, 0x70,  &val);
        printk("===> 98DX3xx CPU good rx : %08x \n", val);
        mv_salsa2_read_reg(0, 0x74,  &val);
        printk("===> 98DX3xx CPU bad rx : %08x \n", val);


        printk("============> READ CPU REGs \n");
	for (ethPortNo = 0; ethPortNo < 2; ethPortNo++)
	{
                printk("------> Port %d\n", ethPortNo);
                val = MV_REG_READ(ETH_PORT_CONFIG_REG(ethPortNo));
                printk("------>     ETH_PORT_CONFIG_REG: %08x \n", val);
                val = MV_REG_READ(ETH_PORT_CONFIG_EXTEND_REG(ethPortNo));
                printk("------>     ETH_PORT_CONFIG_EXTEND_REG: %08x \n", val);
                val = MV_REG_READ(ETH_GMII_SERIAL_PARAM_REG(ethPortNo));
                printk("------>     ETH_GMII_SERIAL_PARAM_REG: %08x \n", val);
                val = MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(ethPortNo));
                printk("------>     ETH_PORT_SERIAL_CTRL_REG: %08x \n", val);
                val = MV_REG_READ(ETH_PORT_SERIAL_CTRL_1_REG(ethPortNo));
                printk("------>     ETH_PORT_SERIAL_CTRL_1_REG: %08x \n", val);
                val = MV_REG_READ(ETH_PORT_STATUS_1_REG(ethPortNo));
                printk("------>     ETH_PORT_STATUS_1_REG: %08x \n", val);
                val = MV_REG_READ(ETH_PORT_STATUS_REG(ethPortNo));
                printk("------>     ETH_PORT_STATUS_REG: %08x \n", val);
                val = MV_REG_READ(MPP_OUTPUT_DRIVE_REG);
                printk("------>     IO Config Register: %08x \n", val);
	}

}
#endif

int mv_eth_tool_write (struct file *file, const char *buffer,
                      unsigned long count, void *data) {

	sscanf(buffer,"%x",&command);

	printk("---> mv_eth_tool_write command :%d (%s)\n", command, buffer);
	switch (command) {
		case COM_RX_COAL:
		case COM_TX_COAL:
        	case COM_TX_EN:
        	case COM_EJP_MODE:
		case COM_TX_NOQUEUE:
		case COM_LRO:
		case COM_LRO_DESC:
			run_port_com(buffer);
			break;
		case COM_TXDONE_Q:
        	case COM_SKB_REUSE:
                case COM_SKB_RECYCLE:
		case COM_NFP:
			run_eth_com(buffer);
			break;

		case COM_TOS_MAP:
            run_port_queue_cmd(buffer);
			break;

#ifdef CONFIG_MV_ETH_NFP
		case COM_IP_RULE_SET:
			run_ip_rule_set_com(buffer);
			break;
		case COM_IP_RULE_DEL:
			run_ip_rule_del_com(buffer);
			break;
		case COM_NFP_STATUS:
			fp_mgr_status();
			break;
		case COM_NFP_PRINT:
			run_fp_db_print_com(buffer);
			break;
#endif /* CONFIG_MV_ETH_NFP */

// EFM, ysyoo
#ifdef CONFIG_EFM_ROUTER_PATCH
		case 95:
			efm_config_98dx3xx_cpu_port();
			break;
		case 96:
			efm_debug_reset_cpu(3);
			break;
		case 97:
			efm_debug_reset_cpu(1);
			break;
		case 98:
			efm_debug_reset_cpu(0);
			break;
		case 99:
			efm_debug_dump_register();
			break;
#endif
		default:
			run_com_general(buffer);
			break;
	}
	return count;
}

static int proc_calc_metrics(char *page, char **start, off_t off,
                                 int count, int *eof, int len)
{
        if (len <= off+count) *eof = 1;
        *start = page + off;
        len -= off;
        if (len>count) len = count;
        if (len<0) len = 0;
        return len;
}



int mv_eth_tool_read (char *page, char **start, off_t off,
                            int count, int *eof, void *data) {
	unsigned int len = 0;

	//len  = sprintf(page, "\n");
	//len += sprintf(page+len, "\n");
	
   	return proc_calc_metrics(page, start, off, count, eof, len);
}



int __init start_mv_eth_tool(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
  mv_eth_tool = proc_net_create(FILE_NAME , 0666 , NULL);
#else
  mv_eth_tool = create_proc_entry(FILE_NAME , 0666 , init_net.proc_net);
#endif
  mv_eth_tool->read_proc = mv_eth_tool_read;
  mv_eth_tool->write_proc = mv_eth_tool_write;
  mv_eth_tool->nlink = 1;
  return 0;
}

module_init(start_mv_eth_tool);
