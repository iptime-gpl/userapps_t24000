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

#if defined (CONFIG_MV_ETHERNET)
extern int mv_eth_read_mii(unsigned int portNumber, unsigned int MIIReg, unsigned int* value);
extern int mv_eth_write_mii(unsigned int portNumber, unsigned int MIIReg, unsigned int data);
#endif

#define SW98DX3XX_SMI_READ_WRITE_STATUS_REG     0x1f
#define         SMI_WRITE_DONE_BIT      1       // High - completed
#define         SMI_READ_READY_BIT      0       // High - completed
#define SW98DX3XX_SMI_WRITE_ADDRESS_MSB_REG             0x0
#define SW98DX3XX_SMI_WRITE_ADDRESS_LSB_REG             0x1
#define SW98DX3XX_SMI_WRITE_DATA_MSB_REG        0x2
#define SW98DX3XX_SMI_WRITE_DATA_LSB_REG        0x3
#define SW98DX3XX_SMI_READ_ADDRESS_MSB_REG      0x4
#define SW98DX3XX_SMI_READ_ADDRESS_LSB_REG              0x5
#define SW98DX3XX_SMI_READ_DATA_MSB_REG         0x6
#define SW98DX3XX_SMI_READ_DATA_LSB_REG         0x7

#define SMI_TIMEOUT_COUNTER     1000

static int mv_98DX3xx_read_reg(unsigned int portNumber, unsigned int switch_reg, unsigned int *switch_val)
{
        unsigned int reg_val, timeout;
        unsigned int msb, lsb;

        *switch_val = 0;
        timeout = 0;

        do
        {
                if (mv_eth_read_mii(portNumber, SW98DX3XX_SMI_READ_WRITE_STATUS_REG, &reg_val) < 0)
                        return 0;
                if (timeout++ > SMI_TIMEOUT_COUNTER)
                        return 0;
        } while (!(reg_val & (1<<SMI_READ_READY_BIT)));

        msb = (switch_reg>>16) & 0xffff;
        lsb = switch_reg & 0xffff;

        if (mv_eth_write_mii(portNumber, SW98DX3XX_SMI_READ_ADDRESS_MSB_REG, msb) < 0)
                return 0;
        if (mv_eth_write_mii(portNumber, SW98DX3XX_SMI_READ_ADDRESS_LSB_REG, lsb) < 0)
                return 0;

        do
        {
                if (mv_eth_read_mii(portNumber, SW98DX3XX_SMI_READ_WRITE_STATUS_REG, &reg_val) < 0)
                        return 0;
                if (timeout++ > SMI_TIMEOUT_COUNTER)
                        return 0;
        } while (!(reg_val & (1<<SMI_READ_READY_BIT)));

        if (mv_eth_read_mii(portNumber, SW98DX3XX_SMI_READ_DATA_MSB_REG, &msb ) < 0)
                return 0;
        if (mv_eth_read_mii(portNumber, SW98DX3XX_SMI_READ_DATA_LSB_REG, &lsb ) < 0)
                return 0;

        *switch_val = ((msb & 0xffff) << 16) | (lsb & 0xffff);

        return 1;
}

static int mv_98DX3xx_write_reg(unsigned int portNumber, unsigned int switch_reg, unsigned int switch_val)
{
        unsigned int reg_val, timeout;
        unsigned int msb, lsb;

        timeout = 0;

        do
        {
                if (mv_eth_read_mii(portNumber, SW98DX3XX_SMI_READ_WRITE_STATUS_REG, &reg_val) < 0)
                        return 0;
                if (timeout++ > SMI_TIMEOUT_COUNTER)
                        return 0;
        } while (!(reg_val & (1<<SMI_WRITE_DONE_BIT)));

        msb = (switch_reg>>16) & 0xffff;
        lsb = switch_reg & 0xffff;

        if (mv_eth_write_mii(portNumber, SW98DX3XX_SMI_WRITE_ADDRESS_MSB_REG, msb) < 0)
                return 0;
        if (mv_eth_write_mii(portNumber, SW98DX3XX_SMI_WRITE_ADDRESS_LSB_REG, lsb) < 0)
                return 0;

        msb = (switch_val>>16) & 0xffff;
        lsb = switch_val & 0xffff;

        if (mv_eth_write_mii(portNumber, SW98DX3XX_SMI_WRITE_DATA_MSB_REG, msb) < 0)
                return 0;
        if (mv_eth_write_mii(portNumber, SW98DX3XX_SMI_WRITE_DATA_LSB_REG, lsb) < 0)
                return 0;

        return 1;
}

#define MV_EFM_MAJOR    201
#define MV_EFM_NEVENTS  16

#define MV_EFM_IOC_MAGIC  'E'
#define MV_EFM_98DX3_REG_READ         _IOWR(MV_EFM_IOC_MAGIC, 211, int)
#define MV_EFM_98DX3_REG_WRITE        _IOWR(MV_EFM_IOC_MAGIC, 212, int)
#define MV_EFM_88F6281_REG_READ        _IOWR(MV_EFM_IOC_MAGIC, 213, int)
#define MV_EFM_88F6281_REG_WRITE       _IOWR(MV_EFM_IOC_MAGIC, 214, int)

typedef struct _mv_efm_data {
	union {
		struct {
			unsigned int port;
			unsigned int num;
			unsigned int value;
		} reg;
	} u;
} mv_efm_data;

static int mv_efm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	mv_efm_data	data;
	unsigned int reg_base;

	switch (cmd)
	{
	case MV_EFM_98DX3_REG_READ :
		if (copy_from_user(&data, (void *)arg, sizeof(data)))
			return -EFAULT;
		mv_98DX3xx_read_reg(0, data.u.reg.num, &data.u.reg.value);
		//printk("MV_EFM_98DX3_REG_READ (%08x = %08x\n", data.u.reg.num, data.u.reg.value);
		if (copy_to_user ((void *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	case MV_EFM_98DX3_REG_WRITE :
		 if (copy_from_user(&data, (void *)arg, sizeof(data)))
			return -EFAULT;
		mv_98DX3xx_write_reg(0, data.u.reg.num, data.u.reg.value);
		break;
	case MV_EFM_88F6281_REG_READ :
		if (copy_from_user(&data, (void *)arg, sizeof(data)))
			return -EFAULT;
		reg_base = (data.u.reg.port == 0xff) ? 0 : MV_ETH_REG_BASE(data.u.reg.port);
		data.u.reg.value  = MV_REG_READ(reg_base+data.u.reg.num);
		//printk("MV_EFM_88F6281_REG_READ (%08x = %08x\n", data.u.reg.num, data.u.reg.value);
		if (copy_to_user ((void *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	case MV_EFM_88F6281_REG_WRITE :
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

static int mv_efm_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int mv_efm_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations mv_efm_fops = {
    ioctl:   mv_efm_ioctl,
    open:    mv_efm_open,
    release: mv_efm_release,
};


static int __init mv_efm_init(void)
{
	int result;

	result  = register_chrdev(MV_EFM_MAJOR, "mv_efm", &mv_efm_fops);
	if (result < 0)
	{
		printk(KERN_INFO "mv_efm: can't set major number\n");
		return result;
	}

	return 0;
}

static void __exit mv_efm_exit(void)
{
	unregister_chrdev(MV_EFM_MAJOR, "mv_efm");
}

module_init(mv_efm_init);
module_exit(mv_efm_exit);
