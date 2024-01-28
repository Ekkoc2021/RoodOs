#ifndef TSS_H
#define TSS_H
#include "../include/types.h"
#include "../include/str.h"
#include "../include/roodos.h"
#include "../include/global.h"

void initTss(TSS *tss, GDT *gdt);
void update_tss_esp(TSS *tss, uint32_t esp0);
#endif