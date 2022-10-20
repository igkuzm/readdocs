/**
 * File              : docx.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.10.2022
 * Last Modified Date: 20.10.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include "zip.h"
#include "readdocs.h"


/*
 * open docx and return it's text with NULL-terminated char array
 */
char * 
readdocs_docx(const char * filename)
{
	//open file with zip
	struct zip_t * zip =
		zip_open(filename,  ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
	if (!zip)
		return NULL;

	//open entry word/document.xml
	if (zip_entry_open(zip, "word/document.xml"))
		return NULL;;

	//get data
	void *buf;
	size_t bufsize;
	if (zip_entry_read(zip, &buf, &bufsize) < 0){
		if (buf)
			free(buf);
		return NULL;
	}

	//add NULL-termination to buffer
	buf = realloc(buf, bufsize + 1);
	if(!buf)
		return NULL;
	((char *)buf)[bufsize] = 0;

	//return buf
	return buf;
}
