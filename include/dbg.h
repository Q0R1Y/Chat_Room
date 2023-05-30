#ifndef _DBG_H_

#define _DBG_H_


//#define S_LOG_PATH "./s_log"
#define S_LOG_PATH "./s_log"
#define S_LOG_MODE "a+"

#define C_LOG_PATH "./c_log"
#define C_LOG_MODE "a+"

#define LOG(...) \
	do									\
	{									\
		extern FILE *log_file;			\
		fprintf(log_file,__VA_ARGS__);	\
		fprintf(log_file,"\n");			\
	}									\
	while(0)

#endif
