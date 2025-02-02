#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "config.h"
#include <string.h> /* memcpy, memset */

#include "atari.h"

typedef UBYTE (*rdfunc)(UWORD addr);
typedef void (*wrfunc)(UWORD addr, UBYTE value);

extern UBYTE memory[65536];
extern rdfunc readmap[65536];
extern wrfunc *writemap;
extern UBYTE *mem_map[16];
extern UBYTE normal_memory[16];

// This one has to use the mem_map[] table to handle banked carts
inline UBYTE dGetByte(UWORD addr)
{
    return mem_map[addr >> 12][addr];
}

// Zero page will always be in the lower 16K which on a 5200 will never be swapped out / banked...
inline UBYTE zGetByte(UWORD addr)
{
    return memory[addr];
}

// Writing RAM - we can do this directly. If there is a special write, it would already be trapped by writemap[]
inline void dPutByte(UWORD addr, UBYTE data)
{
    memory[addr] = data;
}

// So that Antic can know about any banking of cart space that might be going on...
inline UBYTE *AnticMainMemLookup(unsigned int addr)
{
    return (UBYTE *) mem_map[addr >> 12] + addr;
}

#define dGetWord(x)             (dGetByte(x) + (dGetByte((x) + 1) << 8))
#define dGetWordAligned(x)      dGetWord(x)

inline void dCopyFromMem(unsigned int from, void* to, unsigned int size)
{
    memcpy((UBYTE*)to, AnticMainMemLookup((unsigned int)from), size);
}

inline void dCopyToMem(void*from, unsigned int to, unsigned int size)       
{
    memcpy(AnticMainMemLookup((unsigned int)to), (UBYTE*)from, size);
}

#define dFillMem(addr1, value, length)  memset(memory + (addr1), value, length)

void ROM_PutByte(UWORD addr, UBYTE byte);

#define GetByte(addr)       ((normal_memory[(addr)>>12]) ? dGetByte(addr) : (readmap[(addr)] ? (*readmap[(addr)])(addr) : dGetByte(addr)))
#define PutByte(addr,byte)  ((addr & 0xC000) ? ((wrfunc*)0x06860000)[(addr)]((addr), byte) : dPutByte(addr,byte))

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

void MEMORY_InitialiseMachine(void);
void MemStateSave(UBYTE SaveVerbose);
void MemStateRead(UBYTE SaveVerbose);
#define CopyROM(addr1, addr2, src) memcpy(memory + (addr1), src, (addr2) - (addr1) + 1)

inline void CopyFromMem(UWORD from, UBYTE *to, int size) {while (--size >= 0) {*to++ = memory[from];from++;}}
inline void CopyToMem(const UBYTE *from, UWORD to, int size) {while (--size >= 0) {memory[to++] = *from++;}}
extern void MEMORY_InitialiseMap(void);
#endif /* _MEMORY_H_ */
