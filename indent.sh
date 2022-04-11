#! /bin/sh
indent -i4 -l120 -nut -br -ce -brf -brs -cdw -bad -bap -as -nbc -npsl -nbfda -nbfde -nbs -ncs -npcs -nss -nprs -T HANDLE -T DWORD -T WSAEVENT -T WSADATA -T io_context_t -T SOCKET  "$@"
