//
// Created by 白杰 on 2026/5/20.
//

#include "pg_output.h"
bool need_quotes(const char *s) {
    for (; *s; s++) {
        if (*s == ',' || *s == '"' || *s == '\n' || *s == '\r') {
            return true;
        }
    }
    return false;
}

void csv_write_field(FILE *fp, const char *field, int is_last)
{
    int need_quote = 0;

    if (field == NULL)
        field = "";

    for (const char *p = field; *p; p++) {
        if (*p == ',' || *p == '"' || *p == '\n') {
            need_quote = 1;
            break;
        }
    }

    if (need_quote) {
        fputc('"', fp);
        for (const char *p = field; *p; p++) {
            if (*p == '"') {
                fputc('"', fp); // 转义
            }
            fputc(*p, fp);
        }
        fputc('"', fp);
    } else {
        fputs(field, fp);
    }

    if (is_last)
        fputc('\n', fp);
    else
        fputc(',', fp);
}

// 写一行结束
void csv_end_row(FILE *fp) {
    fputc('\n', fp);
}