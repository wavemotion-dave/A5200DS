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
#define CART_5200_64_SWAP   8
#define CART_5200_128       9
#define CART_5200_256       10
#define CART_5200_512       11
#define CART_LAST_SUPPORTED 11

struct cart_t
{
    char           *md5;
    unsigned char   type;
    unsigned char   control;
    unsigned char   use_analog;
    unsigned char   analog_speed;
    unsigned char   digital_min;
    unsigned char   digital_max;
    unsigned char   keys_debounced;
    unsigned char   frame_skip;
    short int       scale_x;
    short int       scale_y;
    short int       offset_x;
    short int       offset_y;
    unsigned char   artifacting;
    unsigned char   x_function;
};

extern struct cart_t myCart;

#define CTRL_JOY        1
#define CTRL_SWAP       2
#define CTRL_FROG       3
#define CTRL_QBERT      4
#define CTRL_ROBO       5
#define CTRL_SR         6

#define DIGITAL         0
#define ANALOG          1

#define NO_FS           0x00       // Display all frames (DSi always uses this as it has faster CPU)
#define YES_FSL         0x03       // A lighter version of frameskip. Displays 3 of 4 frames.
#define YES_FSH         0x01       // A heavier version of frameskip. Displays every other frame.

#define X_FIRE          0
#define X_PANUP         1
#define X_PANDN         2
#define X_PAND2         3

#define CART_MAX_SIZE	(512 * 1024)

#define CART_CANT_OPEN		-1	/* Can't open cartridge image file */
#define CART_BAD_FORMAT		-2	/* Unknown cartridge format */
#define CART_BAD_CHECKSUM	-3	/* Warning: bad CART checksum */
int CART_Insert(const char *filename);
void CART_Remove(void);
void CART_Start(void);
int Util_flen(FILE *fp);

#endif /* _CARTRIDGE_H_ */
