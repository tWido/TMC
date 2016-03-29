#ifndef LOG_HPP
#define	LOG_HPP
#include <time.h>
#include <sstream>

#define LOG(x) if(use_log) LogMessage(x);

FILE *log;
bool use_log = false;

void LogInit(){
    std::stringstream name("log-");
    time_t actual = time(0);
    struct tm *now = localtime(&actual);
    name << now->tm_year << "-" << now->tm_mon+1 << "-"<< now->tm_mday << "-" << now->tm_hour << ":"<< now->tm_min;
    log = fopen(name.str().c_str(),"w");
    if (log == NULL) printf("Failed to create log\n");
}

void LogMessage(const char* mess){
    fprintf(log, "%s", mess);
}

void LogEnd(){
    fclose(log);
}

#endif	/* LOG_HPP */

