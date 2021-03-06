/*
 * Threading functions and definitions for cdtp
 */

#pragma once
#ifndef CDTP_THREADING_H
#define CDTP_THREADING_H

// Includes
#include "util.h"
#include "server.h"
#include <string.h>

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <pthread.h>
#endif

// Type definitions
typedef struct CDTPEventFunc  CDTPEventFunc;
typedef struct CDTPServeFunc  CDTPServeFunc;
typedef struct CDTPHandleFunc CDTPHandleFunc;

// Event thread function
#ifdef _WIN32
DWORD WINAPI _cdtp_event_thread(LPVOID func_info);
#else
void *_cdtp_event_thread(void *func_info);
#endif

// Call an event function using a thread
void _cdtp_start_event_thread(CDTPEventFunc *func_info);

// Call on_recv (server)
void _cdtp_start_thread_on_recv_server(void (*func)(int, void *, size_t, void *), int client_id, void *data, size_t data_len, void *arg);

// Call on_connect (server)
void _cdtp_start_thread_on_connect(void (*func)(int, void *), int client_id, void *arg);

// Call on_disconnect (server)
void _cdtp_start_thread_on_disconnect(void (*func)(int, void *), int client_id, void *arg);

// Call on_recv (client)
void _cdtp_start_thread_on_recv_client(void (*func)(void *, size_t, void *), void *data, size_t data_size, void *arg);

// Call on_disconnected (client)
void _cdtp_start_thread_on_disconnected(void (*func)(void *), void *arg);

// Serve function thread
#ifdef _WIN32
DWORD WINAPI _cdtp_serve_thread(LPVOID func_info);
#else
void *_cdtp_serve_thread(void *func_info);
#endif

// Call the serve function using a thread
#ifdef _WIN32
HANDLE _cdtp_start_serve_thread(void (*func)(CDTPServer *), CDTPServer *server);
#else
pthread_t _cdtp_start_serve_thread(void (*func)(CDTPServer *), CDTPServer *server);
#endif

// Handle function thread
#ifdef _WIN32
DWORD WINAPI _cdtp_handle_thread(LPVOID func_info);
#else
void *_cdtp_handle_thread(void *func_info);
#endif

// Call the handle function using a thread
#ifdef _WIN32
HANDLE _cdtp_start_handle_thread(void (*func)(CDTPClient *), CDTPClient *client);
#else
pthread_t _cdtp_start_handle_thread(void (*func)(CDTPClient *), CDTPClient *client);
#endif

#endif // CDTP_THREADING_H
