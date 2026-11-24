/*
 * LWCloneU2 (Due) – Ersatz für <util/atomic.h> der avr-libc auf ARM Cortex-M3.
 * ATOMIC_BLOCK(ATOMIC_RESTORESTATE) sperrt die Interrupts (PRIMASK) und stellt sie beim Verlassen
 * des Blocks wieder her – auch bei return/break (cleanup-Attribut von GCC).
 *
 * LWCloneU2 (Due) – replacement for avr-libc's <util/atomic.h> on ARM Cortex-M3.
 * ATOMIC_BLOCK(ATOMIC_RESTORESTATE) disables interrupts (PRIMASK) and restores them when leaving
 * the block – also on return/break (GCC cleanup attribute).
 */

#ifndef LWC_COMPAT_UTIL_ATOMIC_H
#define LWC_COMPAT_UTIL_ATOMIC_H

#include <stdint.h>

static inline uint32_t lwc_irq_save(void)
{
	uint32_t primask;
	__asm__ volatile ("mrs %0, primask\n\tcpsid i" : "=r" (primask) : : "memory");
	return primask;
}

static inline void lwc_irq_restore(uint32_t *primask)
{
	__asm__ volatile ("msr primask, %0" : : "r" (*primask) : "memory");
}

#define ATOMIC_RESTORESTATE
#define ATOMIC_FORCEON

#define ATOMIC_BLOCK(type) \
	for (uint32_t lwc_sreg_save __attribute__((cleanup(lwc_irq_restore))) = lwc_irq_save(), lwc_once = 1; \
	     lwc_once; lwc_once = 0)

#endif
