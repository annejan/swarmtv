/* workarounds for MinGW */ 

char* strptime (const char *buf, const char *format, struct tm *timeptr); 

struct tm* localtime_r (const time_t *timer, struct tm *result);

char* strsep (char **stringp, const char *delim); 
