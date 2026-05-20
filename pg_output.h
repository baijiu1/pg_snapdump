//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_OUTPUT_H
#define PG_HEXRETRO_PG_OUTPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void csv_write_field(FILE *fp, const char *s, int is_last);
void csv_end_row(FILE *fp);

#endif //PG_HEXRETRO_PG_OUTPUT_H
