#include "ultils.h"

bool Read_VARS(const char * pattern, const char * data, ...) 
{
    va_list ap;
    va_start(ap, data);

    while(*pattern && *data) 
    {
        while((*pattern && *data) && (*pattern == *data || *pattern == '*')) 
        { 
            pattern++; 
            data++; 
        }

        if (*pattern == '\0' || *data == '\0') 
        {
            va_end(ap);
            return *pattern == '\0';
        }
        
        if (*pattern++ != '%') 
        {
          va_end(ap);
          return false;
        }
        if (*pattern == 'f') 
        {
            pattern++;
            float* v = va_arg(ap, float*);
            
            char tmp[10];
            int i = 0;
            
            while (*data != '\0' && *data != *pattern) 
            {
                if (i >= 9) break;
                
                tmp[i++] = *data;
                data++;
            }
            tmp[i] = '\0';
            
            *v = atof(tmp);
        } 
        else if (*pattern == 'd') 
        {
            pattern++;
            int* v = va_arg(ap, int*);
            
            char tmp[10];
            int i = 0;
            
            while (*data != '\0' && *data != *pattern) 
            {
                if (i >= 9) break;
                
                tmp[i++] = *data;
                data++;
            }
            tmp[i] = '\0';
            
            *v = atoi(tmp);
        } 
        else if (*pattern == 's') 
        {
            pattern++;
            char* v = va_arg(ap, char*);
            char* tmp = v;
            int i = 0;
            
            while (*data != '\0' && *data != *pattern) 
            {
                if (i++ >= 127) break;
                *tmp++ = *data++;
            }
            *tmp = '\0';
        }
    }
    va_end(ap);
    
    return *pattern == '\0';
}

void convertUCS2(const char* ucs2, char* text)
{
    int val;
    
    while(*ucs2 != NULL)
    {
        val = 0;
        int hex1 = (int)(*ucs2);
        int hex2 = (int)(*(ucs2 + 1));

        if (hex1 != 48 || hex2 != 48) 
        {
            val += hex1 > 64 ? (hex1 - 65 + 10) * 16    : (hex1 - 48) * 16;
            val += hex2 > 64 ? (hex2 - 65 + 10)         : (hex2 - 48);
            *text++ = char(val);
            *text = '\0';
        }

        ucs2 += 2;
    }
}

void utf8tohex(const char* utf8, char* hex)
{
    
}
