#ifndef _PIA_H_
#define _PIA_H_

#include "atari.h"

#define _PORTA 0x00
#define _PORTB 0x01
#define _PACTL 0x02
#define _PBCTL 0x03

extern UBYTE PACTL;
extern UBYTE PBCTL;
extern UBYTE PORTA;
extern UBYTE PORTB;
extern UBYTE PORT_input[2];

extern UBYTE atari_os[0x800];

extern void PIA_Initialise(void);
extern void PIA_Reset(void);

#endif /* _PIA_H_ */
