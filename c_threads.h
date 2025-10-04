/*
   Copyright 2025 Mahdi Almusaad (https://github.com/mahdialmusaad)
   under the MIT License (https://opensource.org/license/mit)

   Simple threading header to allow Windows OSs to run the C source files as it has its own threading interface.
   Only implements portable thread creation and joining, which is needed  for the given multithreaded C programs.

   Thanks, Microsoft.
*/

#ifdef _MSC_VER
/* Using Windows library */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define thread_func_t DWORD WINAPI
#define thread_arg_t LPVOID
#define thread_id_t HANDLE
#define pi_i64 long long

void pidef_create_thread(thread_id_t *thread_id, DWORD (*function)(thread_arg_t), thread_arg_t argument) {
	*thread_id = CreateThread(NULL, 0, function, argument, 0, NULL);
}

void pidef_join_thread(thread_id_t thread_id) {
	WaitForSingleObject(thread_id, INFINITE);
}
#else
/* Using POSIX threads */
#include <pthread.h>
#define thread_func_t void *
#define thread_arg_t void *
#define thread_id_t pthread_t
#define pi_i64 long

void pidef_create_thread(thread_id_t *thread_id, thread_func_t (*function)(thread_arg_t), thread_arg_t argument) {
	pthread_create(thread_id, NULL, function, argument);
}

void pidef_join_thread(thread_id_t thread_id) {
	pthread_join(thread_id, NULL);
}
#endif
