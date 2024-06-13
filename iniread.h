
#ifndef INI_READ_H
#define INI_READ_H

#include <stdlib.h>
#include <string.h>

typedef enum
{
	INIREAD_SECTION = 1,
        INIREAD_SUBSECTION = 2,
        INIREAD_STRING = 3,
	INIREAD_NUMBER = 4,
	INIREAD_TRUE = 5,
        INIREAD_FALSE = 6,
        INIREAD_EQUAL = 7,
        INIREAD_IDENTIFIER = 8
} iniread_token_type_t;

typedef struct
{
	char* value;
	iniread_token_type_t type;
} iniread_token_t;
typedef struct
{
	struct token_array {
		iniread_token_t* token;
		int used;
		int size;
	} token_array;
	unsigned char* contents;
} iniread_lexer_t;

typedef struct
{
	iniread_lexer_t lexer;
        unsigned char status;
        unsigned char* error_buffer;
} iniread_t;

typedef struct
{
        char* section;
        char* identifier;
        char* value;
        iniread_token_type_t type;
} iniread_key_t;

iniread_t iniread_open(const char* ini_file);
unsigned char* iniread_get_error(iniread_t ini_file);
iniread_key_t  iniread_get_key(iniread_t iniread, char* section, char* identifier);


void iniread_create_lexer(iniread_lexer_t* lexer);
char* iniread_add_character_to_char_array(char* charr, char ch);
iniread_token_type_t iniread_evaluate_token(char* buffer);
int iniread_compare_strings(char* string, char* string2);
char* iniread_get_token_name(iniread_token_type_t type);
char* iniread_trim(char* value, char key);


#ifdef INIREAD_IMPL
char* iniread_trim(char* value, char key)
{
        size_t s =0;
        while (value[s++] != '\0');
        char* x = malloc(s + 2);
        int a = 0;
        for (int i=0;i < s;i++)
        {
                if (value[i] != key)
                        x[a++] = value[i];
        }
        
        
        return x;
}
iniread_key_t  iniread_get_key(iniread_t iniread, char* section, char* identifier)
{
        iniread_key_t key;
        key.value  = malloc(1);
        key.section = section;
        key.identifier = identifier;
        char* active_section;
        
        for (int i=0;i<iniread.lexer.token_array.used;i++)
        {
                if (iniread.lexer.token_array.token[i].type == INIREAD_SECTION){
                        
                        iniread.lexer.token_array.token[i].value = iniread_trim(iniread.lexer.token_array.token[i].value, '[');
                        iniread.lexer.token_array.token[i].value = iniread_trim(iniread.lexer.token_array.token[i].value, ']');
                        active_section = iniread.lexer.token_array.token[i].value;
                }
                if (iniread_compare_strings(active_section, section) == 0){
                        if (iniread.lexer.token_array.token[i].type == INIREAD_EQUAL)
                        {
                                if ((iniread_compare_strings(iniread.lexer.token_array.token[i-1].value, identifier)) == 0)
                                {
                                        if (iniread.lexer.token_array.token[i + 1].type != INIREAD_SECTION && iniread.lexer.token_array.token[i + 1].type != INIREAD_SUBSECTION){
                                                if (iniread.lexer.token_array.token[i + 1].type == INIREAD_STRING)
                                                {
                                                        iniread.lexer.token_array.token[i + 1].value = iniread_trim(iniread.lexer.token_array.token[i + 1].value, '"');
                                                        key.value = iniread.lexer.token_array.token[i + 1].value;
                                                } else
                                                        key.value = iniread.lexer.token_array.token[i + 1].value;
                                                
                                                key.type = iniread.lexer.token_array.token[i + 1].type;
                                                
                                                
                                        } else
                                        {
                                                iniread.status = 1;
                                                iniread.error_buffer = "Could not get key value.";
                                        }
                                }
                        }
                }
        }
        
        if (*key.value == 0)
        {
                iniread.status = 1;
                iniread.error_buffer = "Could not get key value.";
        }
        return key;
        
}
unsigned char* iniread_get_error(iniread_t ini_file)
{
        return ini_file.error_buffer;
}
iniread_t iniread_open(const char* ini_file)
{
        iniread_t ini = {0};
        
        
        FILE *file = fopen(ini_file, "rb");
        if (file == NULL){
                ini.status = 1;
                ini.error_buffer = "failed to open file";
                return ini;
        }
        long bytes = 0;
        fseek(file, 0l, SEEK_END);
        bytes = ftell(file);
        fseek(file, 0l, SEEK_SET);
        
        ini.lexer.contents = malloc(bytes);
        if (fread(ini.lexer.contents, bytes, 1, file) == 0)
        {
                ini.status = 1;
                ini.error_buffer = "failed to obtain content";
                return ini;
        }
        
        iniread_create_lexer(&ini.lexer);
        
        
        
        return ini;
}


char *iniread_add_character_to_char_array(char* charr, char ch)
{
        int total_length = 0;
        while (charr[total_length++] != '\0');
        
        char* appended = malloc(total_length + 2);
        
        memcpy(appended, charr, total_length);
        appended[total_length-1] = ch;
        appended[total_length] = 0;
        
        return appended;
        
        
}

void iniread_create_lexer(iniread_lexer_t* lexer)
{
        
        size_t length = 0;
        while (lexer->contents[length++] != '\0');
        
        lexer->token_array.used = 0;
        lexer->token_array.size = 1024;
        lexer->token_array.token = malloc(sizeof(iniread_token_t) * lexer->token_array.size);
        
        lexer->contents = iniread_add_character_to_char_array(lexer->contents, 
                                                              ' '
                                                              );
        int i = 0;
        
        
        char* buffer = "\0";
        unsigned char enter_string = 0;
        unsigned char commented = 0;
        unsigned char value = 0;
        char* string = "\0";
        
        for (i; i < length; i++)
        {
                if (lexer->contents[i] == '\"' && !enter_string)
                        enter_string = 1;
                else if (lexer->contents[i] == '\"' && enter_string){
                        enter_string = 0;
                        string = iniread_add_character_to_char_array(string, 
                                                                     '\"'
                                                                     );
                        lexer->token_array.token[lexer->token_array.used].value = string;
                        
                        lexer->token_array.used += 1;
                        if (lexer->token_array.used  >= lexer->token_array.size){
                                lexer->token_array.size *= 2;
                                lexer->token_array.token = realloc(lexer->token_array.token,sizeof(iniread_token_t) * lexer->token_array.size);
                        }
                        string  = "\0";
                }
                if (enter_string)
                        string = iniread_add_character_to_char_array(string, 
                                                                     lexer->contents[i]
                                                                     );
                
                if (lexer->contents[i] == '#')
                        commented = 1;
                if (lexer->contents[i] == '\n' && commented )
                {
                        commented = 0;
                        value = 0;
                }
                
                if (( (lexer->contents[i] != ' ' ) && (lexer->contents[i] != '\t') && (lexer->contents[i] != '\n')) && !enter_string   && (lexer->contents[i] != '\"') && (lexer->contents[i] != '=')  && (lexer->contents[i] != '#')  && !commented )
                        buffer = iniread_add_character_to_char_array(buffer, 
                                                                     lexer->contents[i]
                                                                     );
                
                else
                {
                        if ((buffer != "\0") && (buffer != "\"") )
                        {
                                
                                lexer->token_array.token[lexer->token_array.used].value = buffer;
                                lexer->token_array.used += 1;
                                if (lexer->token_array.used  >= lexer->token_array.size){
                                        lexer->token_array.size *= 2;
                                        lexer->token_array.token = realloc(lexer->token_array.token,sizeof(iniread_token_t) * lexer->token_array.size);
                                }
                                buffer = "\0";
                        }
                        
                }
                
                
                if ((lexer->contents[i] == '='))
                {
                        enter_string = 0;
                        lexer->token_array.token[lexer->token_array.used].value = "=";
                        
                        lexer->token_array.used += 1;
                        if (lexer->token_array.used  >= lexer->token_array.size){
                                lexer->token_array.size *= 2;
                                lexer->token_array.token = realloc(lexer->token_array.token,sizeof(iniread_token_t) * lexer->token_array.size);
                        }
                        string  = "\0";
                        value  = 1;
                }
        };
        for (i = 0; i < lexer->token_array.used;i++)
                lexer->token_array.token[i].type = iniread_evaluate_token(lexer->token_array.token[i].value);
        if (enter_string)
        {
#ifdef INIREAD_ERRORS
                printf("iniread: error: non-terminated string.\n");
                exit(1);
#endif
        }
#if 0
        for (i = 0; i < lexer->token_array.used;i++)
                printf("%i [%s] [%s]\n", i, iniread_get_token_name(lexer->token_array.token[i].type), (lexer->token_array.token[i].value));
#endif
}
char* iniread_get_token_name(iniread_token_type_t type)
{
        switch(type)
        {
                case INIREAD_SECTION:
                {
                        return "section";
                } break;
                case INIREAD_SUBSECTION:
                {
                        return "subsection";
                } break;
                
                case INIREAD_STRING:
                {
                        return "string";
                } break;
                
                case INIREAD_NUMBER:
                {
                        return "number";
                } break;
                
                case INIREAD_TRUE:
                {
                        return "true";
                } break;
                
                case INIREAD_FALSE:
                {
                        return "false";
                } break;
                case INIREAD_EQUAL:
                {
                        return "equal";
                } break;
                
                case INIREAD_IDENTIFIER:
                {
                        return "identifier";
                } break;
        }
}
iniread_token_type_t iniread_evaluate_token(char* buffer)
{
        
        int size = 0;
        while (buffer[size++] != '\0');
        unsigned char is_string = 0;
        unsigned char is_section = 0;
        unsigned char is_number = 0;
        unsigned char is_subsection = 0;
        
        for (int i=0;i<size;i++)
        {
                
                
                
                
                
                if ((buffer[i] == '\"') && !is_section)
                        is_string = 1;
                else if ((buffer[i] == '[' || buffer[i] == ']') && !is_string)
                        is_section = 1;
                
                else if (is_section && buffer[i] == '.')
                {
                        is_section = 0;
                        is_subsection = 1;
                }
                
                if ((buffer[i] >= '0' && buffer[i] <= '9') && (!is_string) && ( !is_section ))
                {
                        is_number  = 1;
                }
                
        }
        
        
        
        int number = 0;
        if (is_subsection)
        {
                return INIREAD_SUBSECTION;
        }
        else if (is_string)
                return INIREAD_STRING;
        else if (is_section)
                return INIREAD_SECTION;
        else if (is_number)
                return INIREAD_NUMBER;
        else if (iniread_compare_strings(buffer, "=") == 0)
                return INIREAD_EQUAL;
        else if (iniread_compare_strings(buffer, "true") == 0)
                return INIREAD_TRUE;
        else if (iniread_compare_strings(buffer, "false") == 0)
                return INIREAD_FALSE;
        else
                return INIREAD_IDENTIFIER;
}


int iniread_compare_strings(char* string, char* string2)
{
        while (*string){
                if (*string != *string2)break;
                *string++;
                *string2++;
        };
        return *(const unsigned char*)string -  *(const unsigned char*)string2;
}
#endif



#endif
