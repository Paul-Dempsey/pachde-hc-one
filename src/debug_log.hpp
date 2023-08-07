// define or undef VERBOSE_LOG, then include this file 

#ifdef VERBOSE_LOG
#define DebugLog(format, ...) DEBUG(format, ##__VA_ARGS__)
#else
#define DebugLog(format, ...) {}
#endif