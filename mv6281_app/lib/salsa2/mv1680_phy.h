#ifndef _MV1680_PHY_H_
#define _MV1680_PHY_H_

#define PHY_WRITE_COMMAND       0
#define PHY_READ_COMMAND        1

#define SALSA2_SMI0_MANAGE_REG  0x04004054
#define SALSA2_SMI1_MANAGE_REG  0x05004054

#define MV1680_PHY_CONTROL_REG          0x0
#define         AUTONEG_RESTART         (1 << 9)
#define         POWER_DOWN              (1 << 11)
#define         SPEED_SEL_LSB           (1 << 13) // 00: 10Mbps,   01: 100Mbps
#define         SPEED_SEL_MSB           (1 << 6)  // 10: 1000Mbps, 11: reserved
#define         DUPLEX_MODE             (1 << 8)  // 0: Half, 1: Full
#define MV1680_PHY_STATUS_REG           0x1
#define MV1680_PHY_AUTONEG_ADV_REG      0x4
#define         AUTONEG_10BASETX_HALF   (1 << 5)
#define         AUTONEG_10BASETX_FULL   (1 << 6)
#define         AUTONEG_100BASETX_HALF  (1 << 7)
#define         AUTONEG_100BASETX_FULL  (1 << 8)
#define MV1680_PHY_GIGA_CONTROL_REG     0x9
#define         AUTONEG_1000BASETX_HALF (1 << 8)
#define         AUTONEG_1000BASETX_FULL (1 << 9)
#define MV1680_PHY_GIGA_STATUS_REG      0x10

#define PORT_SPEED_AUTO  0
#define PORT_SPEED_10    10
#define PORT_SPEED_100   100
#define PORT_SPEED_1000  1000
#define PORT_DUPLEX_AUTO 0
#define PORT_DUPLEX_HALF 1
#define PORT_DUPLEX_FULL 2
#define PORT_POWER_ON	0xfe
#define PORT_POWER_OFF	0xff

extern int mv1680_get_phy_reg(int phyad, int regad, int *value);
extern int mv1680_set_phy_reg(int phyad, int regad, int value);
extern int mv1680_port_config(int phyad, int speed, int duplex);


#endif
