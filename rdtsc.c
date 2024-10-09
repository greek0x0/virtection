#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <conio.h>

// Function declarations
int rdtsc_check();
int vmexit_check();

// Inline function to capture time difference using RDTSC
static inline unsigned long long rdtscd() {
    unsigned long long ret, ret2;
    unsigned eax, edx;

    // First RDTSC call
    __asm__ volatile("rdtsc" : "=a" (eax), "=d" (edx));
    ret  = ((unsigned long long)eax) | (((unsigned long long)edx) << 32);

    // Second RDTSC call
    __asm__ volatile("rdtsc" : "=a" (eax), "=d" (edx));
    ret2 = ((unsigned long long)eax) | (((unsigned long long)edx) << 32);

    return ret2 - ret;
}

// Inline function for RDTSC with VM exit
static inline unsigned long long rdtscd_vmexit() {
    unsigned long long ret, ret2;
    unsigned eax, edx;

    // First RDTSC call
    __asm__ volatile("rdtsc" : "=a" (eax), "=d" (edx));
    ret  = ((unsigned long long)eax) | (((unsigned long long)edx) << 32);

    // Force a VM exit via CPUID instruction
    __asm__ volatile("cpuid" : /* no output */ : "a"(0x00));

    // Second RDTSC call
    __asm__ volatile("rdtsc" : "=a" (eax), "=d" (edx));
    ret2 = ((unsigned long long)eax) | (((unsigned long long)edx) << 32);

    return ret2 - ret;
}

// Check for abnormal time difference using RDTSC
int rdtsc_check() {
    int i;
    unsigned long long average = 0;

    // Collect average RDTSC delta over 10 iterations
    for (i = 0; i < 10; i++) {
        average += rdtscd();
        Sleep(500);
    }
    average /= 10;

    // Return TRUE if detected, otherwise FALSE
    return (average < 750 && average > 0) ? FALSE : TRUE;
}

// Check for abnormal time difference after VM exit
int vmexit_check() {
    int i;
    unsigned long long average = 0;

    // Collect average RDTSC delta over 10 iterations with VM exit
    for (i = 0; i < 10; i++) {
        average += rdtscd_vmexit();
        Sleep(500);
    }
    average /= 10;

    // Return TRUE if detected, otherwise FALSE
    return (average < 1000 && average > 0) ? FALSE : TRUE;
}

// Run a check and print the result
void run(char *text, int (*callback)(), char *log, char *text_trace) {
    int output;

    // If a log is provided, run the callback without writing to log
    if (log) {
        output = callback();
    } else {
        output = callback(TRUE);
    }

    // Print the test result
    printf("\n%s: ", text);
    if (output == TRUE) {
        if (log) {
            printf(" Detected");
        }
    } else {
        printf(" Passed");
    }
}

int main(void) {
    // Run RDTSC and VM exit checks
    run("RDTSC Check (0)", &rdtsc_check, "trace rdtsc differences", "rdtsc_test");
    run("RDTSC Exit VM Check (0)", &vmexit_check, "trace forcing VM exit", "vm_exit_test");

    return 0;
}
