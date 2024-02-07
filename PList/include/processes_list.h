#ifndef PROCESS_LIST_H
#define PROCESS_LIST_H
#include <bemapiset.h>

void ListProcesses(const int showMemoryUsage);
void ListDetailedProcessInfo(DWORD pid);

#endif // PROCESS_LIST_H
