/**
 * File              : rtf.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.10.2022
 * Last Modified Date: 21.10.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readdocs.h"
#include "cRTF/cRTF.h"

struct str {
	char * buf;
	size_t len;
};

int callback(void *userdata, char *buf, int *t_attr, 
			int *nfonts, cRTFFonts *fonts, int *inBrack){

	struct str * str = userdata;
	
	//realloc str buffer
	size_t len = str->len + strlen(buf) + 1;
	void * ptr = realloc(str->buf, len * sizeof(char));
	if (!ptr)
		return 1;
	str->buf = ptr;
	str->len = len;

	//fill str buffer
	size_t i  = 0;
	while(*buf)
		str->buf[i++] = *buf++;

	//NULL-terminate str buffer 
	str->buf[i] = 0;

	return 0;
}

/*
 * open rtf and return it's text with NULL-terminated char array
 */
char * 
readdocs_rtf(const char * filename)
{
	//open file
	FILE *fp = fopen(filename, "r");
	if (!fp)
		return NULL;

	//init str to return
	struct str str = {
		.buf = NULL,
		.len = 0
	};
	
	//fill str buf
	c_rtf_parse_file(fp, &str, callback);	

	//return str buf
	return str.buf;
}
