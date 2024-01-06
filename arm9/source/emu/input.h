#ifndef _A5200_INPUT_H_
#define _A5200_INPUT_H_

// Analog Speed Values...
#define ANA_SLOWEST 0
#define ANA_SLOWER  1
#define ANA_NORMAL  2
#define ANA_FAST    3
#define ANA_FASTER  4
#define ANA_FASTEST 5


/* key_code values */
#define AKEY_NONE -1

#define AKEY_SHFT           0x40
#define AKEY_CTRL           0x80
#define AKEY_SHFTCTRL       0xc0

/* 5200 key codes */
#define AKEY_5200_START     0x39
#define AKEY_5200_PAUSE     0x31
#define AKEY_5200_RESET     0x29
#define AKEY_5200_0         0x25
#define AKEY_5200_1         0x3f
#define AKEY_5200_2         0x3d
#define AKEY_5200_3         0x3b
#define AKEY_5200_4         0x37
#define AKEY_5200_5         0x35
#define AKEY_5200_6         0x33
#define AKEY_5200_7         0x2f
#define AKEY_5200_8         0x2d
#define AKEY_5200_9         0x2b
#define AKEY_5200_HASH      0x23
#define AKEY_5200_ASTERISK  0x27

/* key_consol masks */
/* Note: key_consol should be CONSOL_NONE if no consol key is pressed.
   When a consol key is pressed, corresponding bit should be cleared.
 */
 
#define CONSOL_NONE		0x07
#define CONSOL_START	0x01
#define CONSOL_SELECT	0x02
#define CONSOL_OPTION	0x04

extern int key_code;	/* regular Atari key code */
extern int key_shift;	/* Shift key pressed */
extern int key_consol;	/* Start, Select and Option keys */

/* Joysticks ----------------------------------------------------------- */

/* joystick position */

#define	STICK_LL		0x09
#define	STICK_BACK		0x0d
#define	STICK_LR		0x05
#define	STICK_LEFT		0x0b
#define	STICK_CENTRE	0x0f
#define	STICK_RIGHT		0x07
#define	STICK_UL		0x0a
#define	STICK_FORWARD	0x0e
#define	STICK_UR		0x06

/* joy_autofire values */
#define AUTOFIRE_OFF	0
#define AUTOFIRE_FIRE	1	/* Fire dependent */
#define AUTOFIRE_CONT	2	/* Continuous */

extern int joy_autofire[4];		/* autofire mode for each Atari port */

extern int joy_block_opposite_directions;	/* can't move joystick left
											   and right simultaneously */

extern int joy_multijoy;	/* emulate MultiJoy4 interface */

/* 5200 joysticks values */
extern int joy_5200_min;
extern int joy_5200_center;
extern int joy_5200_max;

/* Mouse --------------------------------------------------------------- */

/* mouse_mode values */
#define MOUSE_OFF		0
#define MOUSE_PAD		1	/* Paddles */
#define MOUSE_TOUCH		2	/* Atari touch tablet */
#define MOUSE_KOALA		3	/* Koala pad */
#define MOUSE_PEN		4	/* Light pen */
#define MOUSE_GUN		5	/* Light gun */
#define MOUSE_AMIGA		6	/* Amiga mouse */
#define MOUSE_ST		7	/* Atari ST mouse */
#define MOUSE_TRAK		8	/* Atari CX22 Trak-Ball */
#define MOUSE_JOY		9	/* Joystick */

extern int mouse_mode;			/* device emulated with mouse */
extern int mouse_port;			/* Atari port, to which the emulated device is attached */
extern int mouse_delta_x;		/* x motion since last frame */
extern int mouse_delta_y;		/* y motion since last frame */
extern int mouse_buttons;		/* buttons (b0=1: first button pressed, b1=1: 2nd pressed, etc. */
extern int mouse_speed;			/* how fast the mouse pointer moves */
extern int mouse_pot_min;		/* min. value of POKEY's POT register */
extern int mouse_pot_max;		/* max. value of POKEY's POT register */
extern int mouse_pen_ofs_h;		/* light pen/gun horizontal offset (for calibration) */
extern int mouse_pen_ofs_v;		/* light pen/gun vertical offset (for calibration) */
extern int mouse_joy_inertia;	/* how long the mouse pointer can move (time in Atari frames)
								   after a fast motion of mouse */

/* Functions ----------------------------------------------------------- */

void INPUT_Initialise(void);
void INPUT_Frame(void);
void INPUT_Scanline(void);
void INPUT_SelectMultiJoy(int no);
void INPUT_CenterMousePointer(void);
void INPUT_DrawMousePointer(void);

#endif /* __A5200_INPUT_H_ */
