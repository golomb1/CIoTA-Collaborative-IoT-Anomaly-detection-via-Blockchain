//
// Created by master on 01/02/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_TRACER_H
#define CIOTA_COMPLETE_PROJECT_TRACER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <cmath>
#include <ctime>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cerrno>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <cinttypes>
#include <set>
#include <vector>
#include <map>
#include <cassert>
#include <iostream>
#include <link.h>
#include <elf.h>
#include <netinet/in.h>
#include <poll.h>
#include <atomic>
#include <thread>
#include <system_error>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <functional>
#include <headers/CppUtils/Logger.h>
#include <headers/CppUtils/LockedPointer.h>
#include <headers/JumpTracer/JumpTracer.h>


bool recordJump(LockedPointer<JumpTracer> *tracer, std::atomic<bool> *flag, pid_t pid);


#endif //CIOTA_COMPLETE_PROJECT_TRACER_H
