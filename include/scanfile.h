#ifndef _SCANFILE_H_

#define _SCANFILE_H_

#include<string>
#include<deque>

#define MAX_PROCESS_NU 1

void get_files_name(std::string dir,std::deque<std::string> &stor);

int total_size(const std::deque<std::string> &files);

void* progress_bar(void* args);

#endif
