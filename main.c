#include <stdio.h>
#define  INIREAD_IMPL
#define INIREAD_ERRORS
#include "iniread.h"

char* getvalue(iniread_t ini, char* section, char* id)
{
        /* get keys like this: */

        iniread_key_t key = iniread_get_key(ini, section,
                                            id);
        /* check for errors like this: */
        if (ini.status)
        {
                printf("error: %s\n", iniread_get_error(ini));
        }
        /* evaluate types like this: */
        if (key.type == INIREAD_TRUE)
        {
                printf("true");
        }
        /* return value like so */
        return key.value;
        
}

int main()
{
        /* Open like this: */
        iniread_t ini = iniread_open("dummy.ini");
        
        
        if (ini.status)
        {
                printf("error: %s\n", iniread_get_error(ini));
        } else
        {
                
                printf("[%s]", getvalue(ini, "engine", "runtime"));
                
        }
}
