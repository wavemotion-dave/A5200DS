#ifndef _ANTIC_H_
#define _ANTIC_H_

#include "atari.h"

/*
 * Offset to registers in custom relative to start of antic memory addresses.
 */

#define _DMACTL     0x00
#define _CHACTL     0x01
#define _DLISTL     0x02
#define _DLISTH     0x03
#define _HSCROL     0x04
#define _VSCROL     0x05
#define _PMBASE     0x07
#define _CHBASE     0x09
#define _WSYNC      0x0a
#define _VCOUNT     0x0b
#define _PENH       0x0c
#define _PENV       0x0d
#define _NMIEN      0x0e
#define _NMIRES     0x0f
#define _NMIST      0x0f

extern UBYTE CHACTL;
extern UBYTE CHBASE;
extern UWORD dlist;
extern UBYTE DMACTL;
extern UBYTE HSCROL;
extern UBYTE NMIEN;
extern UBYTE NMIST;
extern UBYTE PMBASE;
extern UBYTE VSCROL;

extern int break_ypos;
extern int ypos;
extern UBYTE wsync_halt;

#define NMIST_C	6
#define NMI_C	12

extern UBYTE global_artif_mode;

extern UBYTE PENH_input;
extern UBYTE PENV_input;

void ANTIC_Initialise(void);
void ANTIC_Reset(void);
void ANTIC_Frame(int draw_display);
UBYTE ANTIC_GetByte(UWORD addr);
void ANTIC_PutByte(UWORD addr, UBYTE byte);

extern UBYTE ANTIC_Get_VCOUNT(UWORD addr);
extern UBYTE ANTIC_Get_PENH(UWORD addr);
extern UBYTE ANTIC_Get_PENV(UWORD addr);
extern UBYTE ANTIC_Get_NMIST(UWORD addr);

UBYTE ANTIC_GetDLByte(UWORD *paddr);
UWORD ANTIC_GetDLWord(UWORD *paddr);

/* always call ANTIC_UpdateArtifacting after changing global_artif_mode */
void ANTIC_UpdateArtifacting(void);

#define XPOS xpos

#endif /* _ANTIC_H_ */
