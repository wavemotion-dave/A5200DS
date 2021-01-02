#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "config.h"
#include <string.h>	/* memcpy, memset */

#include "atari.h"

#define dGetByte(x)				(memory[x])
#define dPutByte(x, y)			(memory[x] = y)

#define dGetWord(x)				(memory[x] + (memory[(x) + 1] << 8))
#define dPutWord(x, y)			(memory[x] = (UBYTE) (y), memory[(x) + 1] = (UBYTE) ((y) >> 8))

#define dGetWordAligned(x)		dGetWord(x)
#define dPutWordAligned(x, y)	dPutWord(x, y)

#define dCopyFromMem(from, to, size)	memcpy(to, memory + (from), size)
#define dCopyToMem(from, to, size)		memcpy(memory + (to), from, size)
#define dFillMem(addr1, value, length)	memset(memory + (addr1), value, length)

extern UBYTE memory[65536 + 2];

#define RAM       0
#define ROM       1
#define HARDWARE  2

typedef UBYTE (*rdfunc)(UWORD addr);
typedef void (*wrfunc)(UWORD addr, UBYTE value);
extern rdfunc readmap[65536];
extern wrfunc writemap[65536];
void ROM_PutByte(UWORD addr, UBYTE byte);
#define GetByte(addr)		(readmap[(addr)] ? (*readmap[(addr)])(addr) : memory[addr])
#define PutByte(addr,byte)	(writemap[(addr)] ? (*writemap[(addr)])(addr, byte) : (memory[addr] = byte))
#define SetRAM(addr1, addr2) do { \
		int i; \
		for (i = (addr1); i <= (addr2); i++) { \
			readmap[i] = NULL; \
			writemap[i] = NULL; \
		} \
	} while (0)
#define SetROM(addr1, addr2) do { \
		int i; \
		for (i = (addr1); i <= (addr2); i++) { \
			readmap[i] = NULL; \
			writemap[i] = ROM_PutByte; \
		} \
	} while (0)

extern int have_basic;
extern int cartA0BF_enabled;

void MEMORY_InitialiseMachine(void);
void MemStateSave(UBYTE SaveVerbose);
void MemStateRead(UBYTE SaveVerbose);
void CopyFromMem(UWORD from, UBYTE *to, int size);
void CopyToMem(const UBYTE *from, UWORD to, int size);
void Cart809F_Disable(void);
void Cart809F_Enable(void);
void CartA0BF_Disable(void);
void CartA0BF_Enable(void);
#define CopyROM(addr1, addr2, src) memcpy(memory + (addr1), src, (addr2) - (addr1) + 1)
void get_charset(UBYTE *cs);

#endif /* _MEMORY_H_ */
