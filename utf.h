/**
 * File              : utf.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 27.05.2022
 * Last Modified Date: 18.10.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef k_lib_utf_h__
#define k_lib_utf_h__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdint.h>

static void utf8_to_utf32(
		int count, 
		uint8_t *utf8_char, 
		void * user_data, 
		void (*callback)(void * user_data, uint32_t utf32_char));

static void utf8_file_to_utf32(
		FILE * file, 
		void * user_data, 
		int (*callback)(void * user_data, uint32_t utf32_char));

static int utf8_filepath_to_utf32(
		const char * filepath, 
		void * user_data, 
		int (*callback)(void * user_data, uint32_t utf32_char));

static void utf32_to_utf8(
		uint32_t utf32_char, 
		void * user_data, 
		int (*callback)(void * user_data, uint8_t utf8_char));


/*
 * IMP
 */

void 
utf8_to_utf32(
		int count, 
		uint8_t *utf8_char, 
		void * user_data, 
		void (*callback)(void * user_data, uint32_t utf32_char))
{
	uint8_t *ptr = utf8_char; //pointer (do not change value)
	uint32_t utf32_char = 0;

	switch (count) {
		case 4: {
			//take last 3 bit from first char
			uint32_t byte0 = (*ptr++ & 0b00000111) << 18;	
			
			//take last 6 bit from second char
			uint32_t byte1 = (*ptr++ & 0b00111111) << 12;	
			
			//take last 6 bit from third char
			uint32_t byte2 = (*ptr++ & 0b00111111) << 6;	
			
			//take last 6 bit from forth char
			uint32_t byte3 = *ptr++ & 0b00111111;	
			
			utf32_char = (byte0 | byte1 | byte2 | byte3);					

			break;
		} 

		case 3: {
			//take last 4 bit from first char
			uint32_t byte0 = (*ptr++ & 0b00001111) << 12;	
			
			//take last 6 bit from second char
			uint32_t byte1 = (*ptr++ & 0b00111111) << 6;	
			
			//take last 6 bit from third char
			uint32_t byte2 = *ptr++ & 0b00111111;	

			utf32_char = (byte0 | byte1 | byte2);

			break;
		} 

		case 2: {
			//take last 5 bit from first char
			uint32_t byte0 = (*ptr++ & 0b00011111) << 6;	
			
			//take last 6 bit from second char
			uint32_t byte1 = *ptr++ & 0b00111111;	

			utf32_char = (byte0 | byte1);

			break;
		} 

		case 1: {
			utf32_char = *ptr++;

			break;
		} 				

		default:
			break;
				
	}
	if (callback)
		if(utf32_char)
			callback(user_data, utf32_char);
}

void
utf32_to_utf8(
		uint32_t utf32_char,
		void * user_data,
		int (*callback)(void * user_data, uint8_t utf8_char)		
		)
{
	if (utf32_char > 0b00000000000001000000000000000000){ //4-byte
		uint8_t utf8_char;

		//get first byte - first 3 bit
		utf8_char = ((utf32_char & 0b00000000000111000000000000000000) >> 18) | 0b11110000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;

		//get second
		utf8_char = ((utf32_char & 0b00000000000000111111000000000000) >> 12) | 0b10000000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
		
		//get third
		utf8_char = ((utf32_char & 0b00000000000000000000111111000000) >> 6 ) | 0b10000000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;

		//get last
		utf8_char = ( utf32_char & 0b00000000000000000000000000111111)        | 0b10000000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
	}
	else if (utf32_char > 0b00000000000000000001000000000000){ //3-byte
		uint8_t utf8_char;
		
		//get first byte - first 4 bit
		utf8_char = ((utf32_char & 0b00000000000000001111000000000000) >> 12) | 0b11100000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;

		//get second
		utf8_char = ((utf32_char & 0b00000000000000000000111111000000) >> 6 ) | 0b10000000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
		
		//get last
		utf8_char = ( utf32_char & 0b00000000000000000000000000111111)        | 0b10000000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
	}
	else if (utf32_char > 0b00000000000000000000000001000000){ //2-byte
		uint8_t utf8_char;
		//get first byte - first 5 bit
		utf8_char = ((utf32_char & 0b00000000000000000000011111000000)>> 6) | 0b11000000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;

		//get last
		utf8_char = ( utf32_char & 0b00000000000000000000000000111111)      | 0b10000000;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
	}
	else { //ANSY
		uint8_t utf8_char = utf32_char;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
	}
}

void utf32_get_cb(void * user_data, uint32_t utf32_char){
	uint32_t *utf32_char_ = user_data;
	*utf32_char_ = utf32_char;
}

void 
utf8_string_to_utf32(
		const char * string, 
		void * user_data, 
		int (*callback)(void * user_data, uint32_t utf32_char))
{
	char *ptr = (char*)string; //pointer to string (do not change string value)
							   
	while (*ptr){ //iterate chars
		
		uint8_t utf8_char[4];
		int count = 0;

		//check utf8 char byte size
		if ((*ptr & 0b11110000) == 240) 
			count = 4;
		else if ((*ptr & 0b11100000) == 224) 
			count = 3;
		else if ((*ptr & 0b11000000) == 192) 
			count = 2;
		else								   
			count = 1;
		
		//fill array
		int i;
		for (i = 0; i < count; i++) {
			utf8_char[i] = *ptr++;
		}		
		
		//get utf32
		uint32_t utf32_char = 0;
		utf8_to_utf32(count, utf8_char, &utf32_char, utf32_get_cb);

		if (callback)
			//stop execution if callback is not 0
			if (callback(user_data, utf32_char))
				break;		
	}	
}

void 
utf8_file_to_utf32(
		FILE * file, 
		void * user_data, 
		int (*callback)(void * user_data, uint32_t utf32_char))
{
    while (1) { 
		
		char ch = fgetc(file);
		if (ch == EOF) { 
			break; 
		}

		uint8_t utf8_char[4];
		int count = 0;

		if ((ch & 0b11110000) == 240) 
			count = 4;
		else if ((ch & 0b11100000) == 224) 
			count = 3;
		else if ((ch & 0b11000000) == 192) 
			count = 2;
		else 
			count = 1;

		//fill array
		int i;
		utf8_char[0] = ch;
		for (i = 1; i < count; i++) {
			utf8_char[i] = fgetc(file);
		}		

		uint32_t utf32_char = 0;
		utf8_to_utf32(count, utf8_char, &utf32_char, utf32_get_cb);
		
		if (callback)
			//stop execution if callback is not 0
			if (callback(user_data, utf32_char))
				break;		
	}
		
}

int 
utf8_filepath_to_utf32(
		const char * filepath, 
		void * user_data, 
		int (*callback)(void * user_data, uint32_t utf32_char))
{
	FILE *file = fopen(filepath, "r");
	if (!file)
		return -1;

	utf8_file_to_utf32(file, user_data, callback);

	fclose(file);
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif //k_lib_utf_h__

