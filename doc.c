/**
 * File              : doc.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.10.2022
 * Last Modified Date: 21.10.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "readdocs.h"
#include "libdoc/libdoc.h"

/*
 * open doc and return it's text with NULL-terminated char array
 */


char * 
readdocs_doc(const char * filename)
{
    assert(filename != NULL);
	
	//open binary file
	FILE * fp =
			fopen(filename, "r");
    assert(fp != NULL);

    char *buffer = NULL;
    char * src = NULL;
    size_t length;
    
	//read file
	if (fp)
    {
        assert(fseek (fp, 0, SEEK_END) == 0);
        assert((length = ftell(fp)) != -1);
        assert(fseek (fp, 0, SEEK_SET) == 0);
        assert((src = malloc(length)) != NULL);
        fread (src, 1, length, fp);
        fclose (fp);
    }
    
	//convert to txt
	doc2text(src, length, &buffer);
    free(src);
    puts(buffer);

	return buffer;
}
