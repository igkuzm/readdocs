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
#include "ezxml.h"

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

	//allocate text
	char * text = (char *)malloc(bufsize * sizeof(char));
	if (!text)
		return NULL;

	//parse XML
	ezxml_t xml = ezxml_parse_str((char*)buf, bufsize);
	if (!xml)
		return NULL;

	//fill text with data
	size_t i = 0;
	ezxml_t body, p, r, t;
	for (body = ezxml_child(xml, "w:body"); body; body = body->next)
		for (p = ezxml_child(body, "w:p"); p; p = p->next){
			text[i++] =	'\n';
			for (r = ezxml_child(p, "w:r"); r; r = r->next)
				for (t = ezxml_child(r, "w:t"); t; t = t->next){
					char * ptr = t->txt;
					while(*ptr)
						text[i++] =	*ptr++;
				}
	}
	text[i] = 0;

	//return text
	return text;
}
