/*
FILE:			cthread.h
CLASS:			CThread
AUTHOR:			Ciprian Miclaus
DATE:			30.06.2001
REVISIONS:	
DESCRIPTION:
	The header of CThread.

*/

#pragma once

//project dependencies

//win32 specific
#include <windows.h>

//end project dependencies

class CThread {

public:
  CThread();
  virtual ~CThread();

  CThread(const CThread&) = delete;
  CThread& operator=(const CThread&) = delete;

  bool CreateThread();

  void StartThread();

  void PauseThread();

  int IsCreated() {
    return (this->hThread != nullptr);
  }

  DWORD Timeout;

  HANDLE GetThreadHandle() {
    return this->hThread;
  }

  DWORD GetThreadId() {
    return this->hThreadId;
  }

  HANDLE GetMainThreadHandle() {
    return this->hMainThread;
  }

  DWORD GetMainThreadId() {
    return this->hMainThreadId;
  }

protected:

  //overrideable
  virtual unsigned long Process(void* parameter);

  DWORD hThreadId;
  HANDLE hThread;
  DWORD hMainThreadId;
  HANDLE hMainThread;

private:

  static int runProcess(void* Param);

};

struct param {
  CThread* pThread;
};
