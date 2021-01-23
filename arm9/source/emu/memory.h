#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "config.h"
#include <string.h>	/* memcpy, memset */

#include "atari.h"

typedef UBYTE (*rdfunc)(UWORD addr);
typedef void (*wrfunc)(UWORD addr, UBYTE value);
extern UBYTE memory[65536];
extern rdfunc readmap[65536];
extern wrfunc writemap[65536];

#ifdef BUILD_BOSCONIAN
    extern UBYTE *bank_ptr;
    extern UWORD bosconian_bank;
    inline UBYTE mem_fetch(UWORD addr)
    {
        return ((bosconian_bank & ((addr>>1) ^ (addr)))  ? bank_ptr[addr] : memory[addr]);
    }
    #define dGetByte(x)				(mem_fetch(x))
#else // Normal Build
    #define dGetByte(x)				(memory[x])
#endif // Normal Build

#define dPutByte(x, y)			(memory[x] = y)

#define dGetWord(x)				(dGetByte(x) + (dGetByte((x) + 1) << 8))
#define dPutWord(x, y)			(memory[x] = (UBYTE) (y), memory[(x) + 1] = (UBYTE) ((y) >> 8))

#define dGetWordAligned(x)		dGetWord(x)
#define dPutWordAligned(x, y)	dPutWord(x, y)

#define dCopyFromMem(from, to, size)	memcpy(to, memory + (from), size)
#define dCopyToMem(from, to, size)		memcpy(memory + (to), from, size)
#define dFillMem(addr1, value, length)	memset(memory + (addr1), value, length)

void ROM_PutByte(UWORD addr, UBYTE byte);

#define GetByte(addr)		(readmap[(addr)] ? (*readmap[(addr)])(addr) : dGetByte(addr))
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

void MEMORY_InitialiseMachine(void);
void MemStateSave(UBYTE SaveVerbose);
void MemStateRead(UBYTE SaveVerbose);
#define CopyROM(addr1, addr2, src) memcpy(memory + (addr1), src, (addr2) - (addr1) + 1)

inline void CopyFromMem(UWORD from, UBYTE *to, int size) {while (--size >= 0) {*to++ = memory[from];from++;}}
inline void CopyToMem(const UBYTE *from, UWORD to, int size) {while (--size >= 0) {memory[to++] = *from++;}}

#endif /* _MEMORY_H_ */
