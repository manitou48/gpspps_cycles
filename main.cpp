// gpspps_cycles
#include "mbed.h"
 
#define ARM_DEMCR               (*(volatile uint32_t *)0xE000EDFC) // Debug Exception and Monitor Control
#define ARM_DEMCR_TRCENA                (1 << 24)        // Enable debugging & monitoring blocks
#define ARM_DWT_CTRL            (*(volatile uint32_t *)0xE0001000) // DWT control register
#define ARM_DWT_CTRL_CYCCNTENA          (1 << 0)                // Enable cycle count
#define ARM_DWT_CYCCNT          (*(volatile uint32_t *)0xE0001004) // Cycle count register
#define LAR (*(volatile uint32_t *)0xE0001FB0)   //lock for some STMF

InterruptIn gpspps(D7);  // LPC1768 p5, K64F/nucleo D7, L476 PD_0
volatile uint32_t ticks, cycles;
Timer t;
 
void pulse() {
    cycles = ARM_DWT_CYCCNT;
    ticks = 1;
}
 
int main() {
    uint32_t prev=0;
    
    printf("\nSystemCoreClock %d  %s %s\n",SystemCoreClock,__TIME__,__DATE__);
    gpspps.rise(&pulse);  // pps rising
    t.start();
    ARM_DEMCR |= ARM_DEMCR_TRCENA;   // enable debug/trace cycle counter
    LAR = 0xC5ACCE55;    // unlock
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;   // enable
    while(1) {           // wait around, interrupts will interrupt this!
        if (ticks) {
            uint32_t  d = cycles - prev;
            float ppm = (int32_t)(d - SystemCoreClock) / (SystemCoreClock / 1000000.);
            printf("%u cycles  %.3f ppm\n",d,ppm);
            ticks = 0;
            prev = cycles;
        }
    }
}