#include "Tools.h"
#include <time.h>
#include <fstream>
#include <iostream>

TThread::TThread()
{

}

TThread::~TThread()
{

}

void TThread::Start()
{
#ifdef _WIN32
	DWORD hand = _beginthreadex(NULL, 0, (unsigned int(_stdcall*)(void*))ThreadProc, (void*)this, 0, &id);
#endif

#ifdef linux
	int ret = pthread_create(&id, NULL, ThreadProc, (void*)this);
#endif
}

void* TThread::ThreadProc(void* p)
{
	TThread* pv = (TThread*)p;
	pv->run();
	return NULL;
}

void CommonSleep(int millisecond)
{
#ifdef _WIN32
	Sleep(millisecond);
#endif

#ifdef linux
	usleep(millisecond * 1000);
#endif
}

void Date(char* date)
{
	time_t t = time(0);
	strftime(date, 21, "%Y-%m-%d", localtime(&t));
}

void DateTime(char* dt)
{
	time_t t = time(0);
	strftime(dt, 21, "%Y/%m/%d/%X", localtime(&t));
}

void WriteEventLog(const char* msg, const char* filename)
{
	char dt[21];
	DateTime(dt);
	try{
		FILE* f = fopen(filename, "a+");
		if (NULL == f)
			throw - 1;
		fprintf(f, "[%s]\t%s", dt, msg);
		fclose(f);
	}
	catch (...)
	{
		std::cout << "open file failed." << std::endl;
	}
}