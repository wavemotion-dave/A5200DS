#ifndef _CARTRIDGE_H_
#define _CARTRIDGE_H_

#include "config.h"
#include "atari.h"

#define CART_NONE		    0
#define CART_5200_4		    1
#define CART_5200_8		    2
#define CART_5200_EE_16	    3
#define CART_5200_NS_16	    4
#define CART_5200_32	    5
#define CART_5200_40	    6
#define CART_5200_64	    7
#define CART_5200_512       8
#define CART_LAST_SUPPORTED 8

struct cart_t
{
    char *md5;
    int  type;
    int  control;
    int  use_analog;
    int  analog_speed;
    int  digital_min;
    int  digital_max;
    int  keys_debounced;
    int  scale_x;
    int  scale_y;
    int  offset_x;
    int  offset_y;
};

extern struct cart_t myCart;

#define CTRL_JOY        1
#define CTRL_SWAP       2
#define CTRL_FROG       3
#define CTRL_QBERT      4
#define CTRL_ROBO       5

#define DIGITAL         0
#define ANALOG          1

#define CART_MAX_SIZE	(512 * 1024)

#define CART_CANT_OPEN		-1	/* Can't open cartridge image file */
#define CART_BAD_FORMAT		-2	/* Unknown cartridge format */
#define CART_BAD_CHECKSUM	-3	/* Warning: bad CART checksum */
int CART_Insert(const char *filename);
void CART_Remove(void);
void CART_Start(void);

#endif /* _CARTRIDGE_H_ */
