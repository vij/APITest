#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#endif

#ifdef linux
#include <unistd.h>
#include <pthread.h>
#endif

//�߳���
class TThread
{
public:
	TThread();
	virtual ~TThread();

	void Start();

private:
	virtual void run() = 0;
	static void* ThreadProc(void* p);

private:
#ifdef _WIN32
	unsigned int id;
#endif

#ifdef linux
	pthread_t id;
#endif
};

//sleep����������Linux��windows����λ������
void CommonSleep(int millisecond);
void Date(char* date);
void DateTime(char* dt);
void WriteEventLog(const char* mag, const char* filename);

#endif