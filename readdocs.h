/**
 * File              : readdocs.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.10.2022
 * Last Modified Date: 21.10.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef k_lib_readdocs_h__
#define k_lib_readdocs_h__

#ifdef __cplusplus
extern "C"{
#endif

/*
 * open docx and return it's text with NULL-terminated char array
 */
char * readdocs_docx(const char * filename);

/*
 * open doc and return it's text with NULL-terminated char array
 */
char * readdocs_doc(const char * filename);


/*
 * open rtf and return it's text with NULL-terminated char array
 */
char * readdocs_rtf(const char * filename);


#ifdef __cplusplus
}
#endif

#endif //k_lib_readdocs_h__
