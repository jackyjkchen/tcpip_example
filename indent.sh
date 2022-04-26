#! /bin/sh
VERSION_CONTROL=none indent -i4 -l120 -nut -br -ce -brf -brs -cdw -bad -bap -as -nbc -npsl -nbfda -nbfde -nbs -ncs -npcs -nss -nprs -T HANDLE -T DWORD -T BYTE -T WSAEVENT -T WSADATA -T io_context_t -T SOCKET -T ThrdContext -T ThreadLock -T ULONG_PTR -T ThreadPool -T THRD_HANDLE -T PSYSTEM_LOGICAL_PROCESSOR_INFORMATION "$@"
