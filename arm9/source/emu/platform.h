#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "config.h"
#include <stdio.h>

/* This include file defines prototypes for platform-specific functions. */

void Atari_Initialise(void);
int Atari_Exit(int run_monitor);
void Atari_DisplayScreen(void);

int Atari_PORT(int num);

#ifdef SUPPORTS_ATARI_CONFIGINIT
/* This function sets the configuration parameters to default values */
void Atari_ConfigInit(void);
#endif

#endif /* _PLATFORM_H_ */
