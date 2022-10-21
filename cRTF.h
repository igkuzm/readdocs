/**
 * File              : cRTF.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 06.09.2021
 * Last Modified Date: 18.10.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef cRTF_h__
#define cRTF_h__
#endif

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdbool.h>

//convers C String (UFT8 codepage) to RTF unicode string (\u0000)
char *c_rtf_text_from_string(const char *string);

enum BUFTYPE {
	BUFTYPE_ANSI,
	BUFTYPE_UTF,
	BUFTYPE_SERVICE
};

enum TEXTATRR {
	TEXT_NORMAL = 0,
	TEXT_BOLD = 2,
	TEXT_ITALIC = 4,
	TEXT_OUTLINE = 8,
	TEXT_FONT = 16,
	TABLE_TABLE = 32,
	TABLE_CELL = 64,
	TABLE_NEWROW = 128,
};

typedef struct c_rtf_fonts_t {
	char ** fonts;
	int * fontnumbers;
	int count;
} cRTFFonts;

void c_rft_fonts_init(cRTFFonts *fonts);
void c_rft_fonts_add (cRTFFonts * fonts, char * fontname, int * fontnumber);

#ifdef __cplusplus
}
#endif


