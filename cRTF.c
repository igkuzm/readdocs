/**
 * File              : cRTF.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 06.09.2021
 * Last Modified Date: 18.10.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "cRTF.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <stdbool.h>
#include <stdarg.h>
#include "utf.h"

void c_rft_fonts_init(cRTFFonts *fonts){
	fonts->fonts = malloc(8);
	fonts->fontnumbers = malloc(8);
	fonts->count = 0;
}

void c_rft_fonts_add(cRTFFonts * fonts, char * fontname, int * fontnumber){
	//allocate new font name
	char * new_font = malloc(128);
	if (!new_font)
		return;
	//copy
	strncpy(new_font, fontname, 127);
	new_font[127] = 0;

	void * ptr;
	int count = fonts->count + 1; 
	
	//realloc fonts array 
	ptr = realloc(fonts->fonts, count*8 + 8);	
	if (!ptr)
		return;
	fonts->fonts = ptr;

	//realloc fontnumbres array	
	ptr = realloc(fonts->fontnumbers, count*8 + 8);	
	if (!ptr)
		return;	
	fonts->fontnumbers = ptr;

	//update fonts
	fonts->fonts[fonts->count] = new_font;
	fonts->fontnumbers[fonts->count] = *fontnumber;
	fonts->count = count;
}

void c_rtf_fonts_free(cRTFFonts * fonts){
	int i;
	for (i = 0; i < fonts->count; i++) {
		free(fonts->fonts[i]);
	}
	free(fonts->fonts);
	free(fonts->fontnumbers);
}
static const char *special_characters[][2] = 
    {
        { "par",       "\n"     } ,
        { "sect",      "\n\n"   } ,
        { "page",      "\n\n"   } ,
        { "line",      "\n"     } ,
        { "tab",       "\t"     } ,
        { "emdash",    "\u2014" } ,
        { "endash",    "\u2013" } ,
        { "emspace",   "\u2003" } ,
        { "enspace",   "\u2002" } ,
        { "qmspace",   "\u2005" } ,
        { "bullet",    "\u2022" } ,
        { "lquote",    "\u2018" } ,
        { "rquote",    "\u2019" } ,
        { "ldblquote", "\u201C" } ,
        { "rdblquote", "\u201D" } ,
		NULL
    };

static const char numbers[] = {
	'0','1','2','3','4','5','6','7','8','9', 0
};

static const char chars[] = {
	'\n', '\r', '\t', -1, 13, 10, '.', ',', 32, 0
};

struct parse_utf_cb_data{
	int *text_attr;
	int *fontnumber; 
	cRTFFonts *fonts;	
	int * inBrackets;
	void * user_data;
		int (*callback)(
			void * user_data, 
			char * buf, 
			int *text_attr, 
			int *fontnumber, 
			cRTFFonts *fonts, 
			int * inBrackets
			);
};
int parse_utf_cb(void *user_data, uint8_t utf8_char){
	struct parse_utf_cb_data *d = user_data;
					
	//printf("%c", utf8_char); //log

	char buffer[2];
	buffer[0] = utf8_char;
	buffer[1] = 0;	
				
	if (d->callback)
		if(d->callback(d->user_data, buffer, d->text_attr, d->fontnumber, d->fonts, d->inBrackets))
			return 1;

	return 0;
}

void clear_buf(char *buf){
	while (*buf)
		*buf++ = 0;
}

int parse_buf(
		char * buf,
		enum BUFTYPE *buftype,
		int *text_attr,
		int *fontnumber, 
		cRTFFonts *fonts,		
		int * inBrackets,
		void * user_data,
		int (*callback)(
			void * user_data, 
			char * buf, 
			int *text_attr, 
			int *fontnumber, 
			cRTFFonts *fonts, 
			int * inBrackets
			)
		)
{
	switch (*buftype){
		case BUFTYPE_ANSI: {
			if ((*text_attr & TEXT_FONT) == TEXT_FONT){
				c_rft_fonts_add(fonts, buf, fontnumber);
			}
			else
				if (callback)
					if (callback(user_data, buf, text_attr, fontnumber, fonts, inBrackets))
						return 1;
			while (*buf){
				/*printf("%c", *buf); //log*/
				*buf++ = 0;
			}
			break;
		}
		case BUFTYPE_UTF: {
			struct parse_utf_cb_data d = {
				.text_attr = text_attr,
				.fontnumber = fontnumber,
				.fonts = fonts,
				.inBrackets = inBrackets,
				.user_data = user_data,
				.callback = callback
			};
			uint32_t utf32_char;
			sscanf(buf, "%u", &utf32_char);

			utf32_to_utf8(utf32_char, &d, parse_utf_cb);

			while (*buf){
				//printf("%c", *buf); //log
				*buf++ = 0;
			}
			break;
		}
		case BUFTYPE_SERVICE: {
			//parse service words

			if (strcmp(buf, "b") == 0 || strcmp(buf, "b ") == 0) //add bold
				*text_attr |= TEXT_BOLD;
			else if (strcmp(buf, "b0") == 0 || strcmp(buf, "b0 ") == 0) //remove bold
				*text_attr &= ~TEXT_BOLD;

			else if (strcmp(buf, "i") == 0 || strcmp(buf, "i ") == 0) //add italic
				*text_attr |= TEXT_ITALIC;
			else if (strcmp(buf, "i0") == 0 || strcmp(buf, "i0 ") == 0) //remove italic
				*text_attr &= ~TEXT_ITALIC;				

			else if (strcmp(buf, "outl") == 0 || strcmp(buf, "outl ") == 0) //add outline
				*text_attr |= TEXT_OUTLINE;
			else if (strcmp(buf, "outl0") == 0 || strcmp(buf, "outl0 ") == 0) //remove outline
				*text_attr &= ~TEXT_OUTLINE;				

			else if (strcmp(buf, "fonttbl") == 0 || strcmp(buf, "fonttbl ") == 0) //text font
				*text_attr |= TEXT_FONT;				

			else if (strcmp(buf, "trowd") == 0 || strcmp(buf, "trowd ") == 0) {//add table
				*text_attr |= TABLE_TABLE;
				*text_attr |= TABLE_NEWROW; //add row
			} 
			else if (strcmp(buf, "lastrow") == 0 || strcmp(buf, "lastrow ") == 0) //remove table
				*text_attr &= ~TABLE_TABLE;			

			else if (strcmp(buf, "intbl") == 0 || strcmp(buf, "intbl ") == 0) //add table cell
				*text_attr |= TABLE_CELL;
			else if (strcmp(buf, "cell") == 0 || strcmp(buf, "cell ") == 0) {//remove table cell
				*text_attr &= ~TABLE_CELL;		
				*text_attr &= ~TABLE_NEWROW;	//remove New row	
			}
			else if (strcmp(buf, "row") == 0 || strcmp(buf, "row ") == 0) //add new row
				*text_attr |= TABLE_NEWROW;			

			else if (buf[0] == 'f'){ //ckeck font number
				bool next_number = false;
				for (int i = 0; i < 10; i++) 
					if (numbers[i] == buf[1])
						next_number = true;
				if(next_number){
					int fn = 0;
					if (sscanf(buf, "f%d", &fn) == 1)
						*fontnumber = fn;
				}
			} 
			
			//parse special chars
			else {
				char ** ptr = (char **)special_characters;
				while (*ptr) {
					char * key   = *ptr++;
					char * value = *ptr++;
					if (strcmp(buf, key) == 0){ //print sprecial char value
						if (callback)
							if (callback(user_data, value, text_attr, fontnumber, fonts, inBrackets))
								return 1;
						break;
					}
				}
			}
			
			while (*buf){
				/*printf("%c", *buf); //log*/
				*buf++ = 0;
			}
				/*printf(" "); //log*/
			break;			
		}
	}

	return 0;
}

void c_rtf_parse_file(
		FILE * file,
		void * user_data,
		int (*callback)(
			void * user_data, 
			char * buf, 
			int *text_attr, 
			int *fontnumber, 
			cRTFFonts *fonts, 
			int * inBrackets
			)
		)
{
	int inBrackets = 0;
	int text_attr  = 0;
	int fontnumber = 0;

	cRTFFonts fonts;
	c_rft_fonts_init(&fonts);

	enum BUFTYPE buftype = BUFTYPE_ANSI;
	
	char buffer[BUFSIZ] = {0};
	char * buf = buffer; //pointer to buffer

	int i = 0;
    
	while (1) { 
		
		char ch = fgetc(file);
		if (ch == EOF) { 
			break; 
		}

		parse:

		if (ch == '{'){
			//print buf and free
			*buf = 0;
			if (parse_buf(buffer, &buftype, &text_attr, &fontnumber, &fonts, &inBrackets, user_data, callback))
				break;
			buf = buffer;			
			//open brackets
			inBrackets++;
		}
		else if (ch == '}'){	
			//print buf and free
			*buf = 0;
			if (parse_buf(buffer, &buftype, &text_attr, &fontnumber, &fonts, &inBrackets, user_data, callback))
				break;
			buf = buffer;			
			//close brackets
			inBrackets--;
			//if TEXT_FONT - remove attribute
			if ((text_attr & TEXT_FONT) == TEXT_FONT)
				text_attr &= ~TEXT_FONT;

			//if TABLE_TABLE - remove attribute
			if ((text_attr & TABLE_TABLE) == TABLE_TABLE)
				text_attr &= ~TABLE_TABLE;
			
		}
		else if (ch == '\\'){			
			//print buf and free
			*buf = 0;
			if (parse_buf(buffer, &buftype, &text_attr, &fontnumber, &fonts, &inBrackets, user_data, callback))
				break;
			buf = buffer;
			//check service word
			char ch1 = fgetc(file);
			if (ch1 == 0    || ch1 == ' ' ||
				ch1 == '\t' || ch1 ==  -1 ||
				ch1 ==  13  || ch1 ==  10 || ch1 == '.' ||
				ch1 == ','  || ch1 == 32  || ch1 == ' '
				)
			{
				//add char
				*buf++ = ch1;
			}
			else if (ch1 == '\\' ) {
				//new line - dont add '\\'
			}			
			else if (ch1 == '\r' ) {
				//new line - dont add '\\'
				*buf++ = ch1;
			}
			else if (ch1 == '\n' ) {
				//new line - dont add '\\'
				*buf++ = ch1;
			}			
			else if (ch1 == 'u') { //unicode? show next
				//check is next number
				char ch2 = fgetc(file);
				bool next_number = false;
				for (int i = 0; i < 10; i++) 
					if (numbers[i] == ch2)
						next_number = true;
				if (next_number){
					//get UTF
					buftype = BUFTYPE_UTF;
					*buf++ = ch2;
					ch = fgetc(file); 
					while(1)
					{
						if (ch == 0    || ch == '\\' || ch == '\n'||
							ch == '\t' || ch == '\r' || ch ==  -1 ||
							ch ==  13  || ch ==  10  || ch == '.' ||
							ch == ','  || ch == 32
							)
						{
							*buf = 0;
							goto parse;							
						}
						*buf++ = ch;
						ch = fgetc(file); 
					}
				}
				else { //new service word
					//add new service word
					buftype = BUFTYPE_SERVICE;
					*buf++ = ch1;
					*buf++ = ch2;
				}
			}
			else {
				//new service word
				//add new service word
				buftype = BUFTYPE_SERVICE;
				*buf++ = ch1;
			}
		}		
		else if (ch == 0    || ch == '\n'||
				 ch == '\t' || ch ==  -1 ||
				 ch ==  13  || ch ==  10 || ch == '.' ||
				 ch == ','  || ch == 32
				)
		{
			*buf++ = ch;
			//print buf and free
			*buf = 0;
			if (parse_buf(buffer, &buftype, &text_attr, &fontnumber, &fonts, &inBrackets, user_data, callback))
				break;
			buf = buffer;
			//set buftype to ANSY				
			buftype = BUFTYPE_ANSI;
		}

		else if (ch == '\r'){			
			//return of carret
			*buf++ = ch;
		}		
		
		else {
			*buf++ = ch;
		}
	}

	//free memory
	c_rtf_fonts_free(&fonts);
}

struct c_rtf_text_from_string_data {
	char **text;
	size_t *len;
};

int c_rtf_text_from_string_cb(void *user_data, uint32_t utf32){
	struct c_rtf_text_from_string_data *d = user_data;
	char str[10];
	sprintf(str, "\\u%d", utf32);

	//realloc text
	size_t new_len = d->len[0] + strlen(str);
	char **text = d->text;
	void *ptr = realloc(*text, new_len*sizeof(char));
	if (!ptr)
		return 1;
	*text = ptr;
	d->len[0] = new_len;

	//add string to text
	strcat(*text, str);
	
	return 0;
}

char *c_rtf_text_from_string(const char *string){
	//allocate text
	char * str = "\\uc0";
	size_t len = strlen(str) + 1;
	char *text = malloc(len*sizeof(char));
	if (!text)
		return NULL;
	strcpy(text, str);

	//run
	struct c_rtf_text_from_string_data d = {
		.text = &text,
		.len = &len
	};

	utf8_string_to_utf32(string, &d, c_rtf_text_from_string_cb);
	return text;
}

//void
//c_rtf_alloc_text(cRTF *rtf)
//{
	//rtf->text=malloc(2*sizeof(char));
	//if (rtf->text == NULL) { //stop program to do not damage data
		//perror("rtf->text malloc");
		//exit(EXIT_FAILURE);  
	//}	
	//rtf->len=2;
	//sprintf(rtf->text, "");
//}

//void
//c_rtf_realloc_text(cRTF *rtf, int plus_size)
//{
	//rtf->len += plus_size;
	//rtf->text=realloc(rtf->text, rtf->len * sizeof(char));
	//if (rtf->text == NULL) { //stop program to do not damage data
		//perror("rtf->text realloc");
		//exit(EXIT_FAILURE);  
	//}				
//}

//int c_rtf_parse_rtf_callback(void *user_data, char *buf, enum BUFTYPE buftype, int inBrackets){
	//cRTF *rtf = user_data;
	
	//if (buftype == BUFTYPE_UTF || buftype == BUFTYPE_ANSI){
		//size_t len = strlen(buf);
		////realloc text
		//c_rtf_realloc_text(rtf, len * 8 + 8);
		////add buff
		//strcat(rtf->text, buf);
	//}

	//return 0;
//}

//int
//c_rtf_parse_rtf(const char *filename, cRTF *rtf)
//{
	//FILE * file = fopen(filename, "r");
	//if (!file)
		//return -1;

	//c_rtf_alloc_text(rtf);
	//c_rtf_parse_file(file, rtf, c_rtf_parse_rtf_callback);

	//fclose(file);
	//return 0;
//}

//void 
//c_rtf_print_rtf(const char *filename)
//{
	//cRTF rtf;
	//c_rtf_parse_rtf(filename, &rtf);
	//printf("%s", rtf.text);
	//free(rtf.text);
//}

//cRTFTable *
//c_rtf_table_new(int columns) 
//{
	//cRTFTable* table = malloc(sizeof(cRTFTable));
	//if (table == NULL) { //stop program to do not damage data
		//perror("cRTFTable malloc");
		//exit(EXIT_FAILURE);  
	//}	
	//table->argc = columns;	
	//table->next = NULL;
	//table->argv = NULL;
	//table->titles = NULL;	
	//return table;
//}

//void
//c_rtf_table_free(cRTFTable* table)
//{
	//cRTFTable *ptr = table;
	//while (ptr != NULL) {
		//if (ptr->titles != NULL) {
			//free(ptr->titles);
		//}
		//if (ptr->argv != NULL) {
			//free(ptr->argv);
		//}
		//cRTFTable *next = ptr->next;
		//free(ptr);
		//ptr=NULL;
		//ptr=next;
	//}
//}

//void c_rtf_table_set_columns(cRTFTable* table, int columns){
	//table->argc = columns;
//}

//void
//c_rtf_table_fill_row(cRTFTable* table, const char* argv[])
//{
	//for (int i = 0; i < table->argc; ++i) {
		//table->argv = malloc(BUFSIZ*table->argc*sizeof(char));
		//if (table->argv == NULL) { //stop program to do not damage data
			//perror("cRTFTable->argv malloc");
			//exit(EXIT_FAILURE);  
		//}		
		//strncpy(table->argv[i], argv[i], BUFSIZ-1);
		//table->argv[i][BUFSIZ - 1] = '\0';
	//}
//}

//void
//c_rtf_table_append_row_data(cRTFTable* table, const char* argv[])
//{
	//if (table->argv == NULL) {
		//c_rtf_table_fill_row(table, argv);	
	//} else {
		//cRTFTable* last = table->next;
		//while (last->next != NULL) {
			//last = last->next;	
		//}	
		//cRTFTable* new = c_rtf_table_new(table->argc);
		//c_rtf_table_fill_row(new, argv);
		//last->next = new;
	//}
//}

//void
//c_rtf_table_append_row(cRTFTable* table, ...)
//{
	//va_list valist;
	//va_start(valist, table);
	//const char **argv = malloc(table->argc * BUFSIZ * sizeof(char));
	//if (argv == NULL) {
		//perror("argv malloc");
		//exit(EXIT_FAILURE);
	//}
	//for (int i = 0; i < table->argc; ++i) {
		//char *arg = va_arg(valist, char*);
		//argv[i] = arg;
	//}
	//va_end(valist);
	//c_rtf_table_append_row_data(table, argv);
	//free(argv);
//}

//void
//c_rtf_table_set_titles_data(cRTFTable* table, const char* titles[])
//{
	//if (table->titles == NULL) {
		//table->titles = malloc(BUFSIZ*table->argc*sizeof(char));
		//if (table->titles == NULL) { //stop program to do not damage data
			//perror("cRTFTable->column_titles malloc");
			//exit(EXIT_FAILURE);  
		//}	
		
		//for (int i = 0; i < table->argc; ++i) {
			//strncpy(table->titles[i], titles[i], BUFSIZ-1);
			//table->titles[i][BUFSIZ - 1] = '\0';
		//}	
	//}
//}

//void
//c_rtf_table_set_titles(cRTFTable* table, ...)
//{
	//va_list valist;
	//va_start(valist, table);
	//const char **argv = malloc(table->argc * BUFSIZ * sizeof(char));
	//if (argv == NULL) {
		//perror("argv malloc");
		//exit(EXIT_FAILURE);
	//}
	//for (int i = 0; i < table->argc; ++i) {
		//char *arg = va_arg(valist, char*);
		//argv[i] = arg;
	//}
	//va_end(valist);
	//c_rtf_table_set_titles_data(table, argv);
	//free(argv);
//}

//int
//c_rtf_table_size(cRTFTable *table)
//{
	//int i = 0;
	//cRTFTable* ptr = table;
	//while (ptr != NULL) {
		//ptr = ptr->next;	
		//i++;
	//}
	//return i;
//}

//[>void<]
//[>c_rtf_table_foreach_row(cRTFTable* table, void* user_data, int (*callback)(int argc, char** argv, void* user_data))<]
//[>{<]
	//[>cRTFTable* ptr = table;<]
	//[>while (ptr != NULL) {<]
		//[>callback(table->argc, ptr->argv, user_data);<]
		//[>ptr = ptr->next;	<]
	//[>}	<]
//[>}<]

//RTFtable *create_RTFtable(char ***rows_val, char **column_names, int column_num, int rows_num){
	//RTFtable *table = malloc(sizeof(RTFtable));
	//if (table == NULL) {
		//errno = ENOMEM;
		//fprintf(stderr, "ERROR. Cannot allocate memory: %d (%s)\n", errno, strerror(errno));		
		//exit(errno);  
	//}	
	
	//table->rows_val = rows_val;
	//table->column_names = column_names;
	//table->column_num = column_num;
	//table->rows_num = rows_num;

	//return table;
//}

//char *RTFstringFromRTFtable(RTFtable *table, bool withBorders){
	////allocate output text
	//char *text=calloc(BUFSIZ*BUFSIZ, sizeof(char));
	//if (text == NULL) { //stop program to do not damage data
		//errno = ENOMEM;
		//fprintf(stderr, "ERROR. Cannot allocate memory: %d (%s)\n", errno, strerror(errno));		
		//exit(errno);  
	//}	
	//sprintf(text, "{\\rtf1\\ansi\\deff0\n");
	//sprintf(text, "%s\\trowd\n", text);
	//int i;

	////create cells
	//for (i = 0; i < table->column_num; ++i) {
		//if (withBorders) {
			//sprintf(text, "%s\\clbrdrt\\brdrs\\clbrdrl\\brdrs\\clbrdrb\\brdrs\\clbrdrr\\brdrs\n", text);
		//}
		//char cell[20];
		//sprintf(cell, "\\cellx%d000", i+1);
		//sprintf(text, "%s%s\n", text, cell);
	//}

	////make titles
	//for (i = 0; i < table->column_num; ++i) {
		//char cell[BUFSIZ];
		//sprintf(cell, "\\intbl \\qc \\b {%s} \\b0 \\cell\n", table->column_names[i]);
		//sprintf(text, "%s%s\n", text, cell);		
	//}	
	//sprintf(text, "%s\\row\n", text);		

	////make text
	//int k;
	//for (k = 0; k < table->rows_num; ++k) {
		//char **row = table->rows_val[k];
		//for (i = 0; i < table->column_num; ++i) {
			//char cell[BUFSIZ];
			//sprintf(cell, "\\intbl \\ql %s\\cell\n", row[i]);
			//sprintf(text, "%s%s\n", text, cell);		
		//}	
		//sprintf(text, "%s\\row\n", text);		
	//}
	
	////close table
	//sprintf(text, "%s}\n", text);		

	//return text;
//}
