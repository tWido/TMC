#ifndef LOG_HPP
#define	LOG_HPP
#define LOG(x) if(use_log) LogMessage(x);

FILE *log;
bool use_log = false;

void LogInit(){

}

void LogMessage(const char*){


}

#endif	/* LOG_HPP */

