//
// Created by master on 01/02/18.
//

#include <headers/JumpTracer/JumpTracer.h>
#include <headers/CppUtils/LockedPointer.h>
#include <dirent.h>
#include "headers/JumpTracer/Tracer.h"


static int perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                           int cpu, int group_fd, unsigned long flags) {
    int ret;
    ret = static_cast<int>(syscall(__NR_perf_event_open, hw_event, pid, cpu,
                                   group_fd, flags));
    return ret;
}

bool recordJump2(LockedPointer<JumpTracer> *tracer, std::atomic<bool> *flag, pid_t pid) {
    /*DIR *dir;
    struct dirent *ent;
    std::vector<std::string> files;
    if ((dir = opendir ("/home/master/Downloads/traces")) != nullptr) {
        // print all the files and directories within directory 
        while ((ent = readdir (dir)) != nullptr) {
            files.push_back(std::string("/home/master/Downloads/traces/") + std::string(ent->d_name));
        }
        closedir (dir);
        // Seed with a real random value, if available
        std::random_device r;
        std::cout << "HERE\n\n\n";

        // Choose a random mean between 1 and 6
        std::default_random_engine e1(r());
        std::uniform_int_distribution<int> uniform_dist(1, 3);
        int startFrom = uniform_dist(e1);
        std::cout << "HERE\n\n\n";
        while(true){
	   std::cout << "Finish 1 iteration" << std::endl;
	   for(int i = 1; i < 10; i++){
               std::string f = files.at(static_cast<unsigned long>(i));
               //std::cout << f.c_str() << "\n";
	       FILE* fd = fopen(f.c_str(), "r");
               for(int l=0; true; l++){
                  uint64_t ip;
                  if(fread(&ip, 1, sizeof(uint64_t), fd) == 0){
                      break;
                  }
                  tracer->use(&JumpTracer::consume,ip);
               }
               fclose(fd);
            }
        }
        return true;
    } else {
        // could not open directory 
        perror ("");
        return false;
    }*/
}

bool recordJump(LockedPointer<JumpTracer> *tracer, std::atomic<bool> *flag, pid_t pid) {
    struct perf_event_attr pe{};
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
    pe.size = sizeof(struct perf_event_attr);
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    // this filed set how many PERF_RECORD_SAMPLE need to happend before overflow notification.
    //pe.wakeup_events = 10;
    // 1 = count bytes for notification, 0 = count events.
    //pe.watermark = 0;
    pe.sample_type = PERF_SAMPLE_IP;
    pe.mmap = 1;
    pe.task = 0; // notify on fork/exit
    pe.precise_ip = 3;
    pe.sample_period = 3;
    pe.wakeup_events = 1;
    //pe.sample_max_stack = 32;

    int cpu = -1;
    LOG_TRACE_4("Start tracing pid {0:d}", pid );
    int fd = perf_event_open(&pe, pid, cpu, -1, 0);
    if (fd == -1) {
        throw std::system_error(errno, std::system_category(),
                                "recordJump - opening event counter failed");
    }
    
    size_t mmap_pages_count = 1;
    auto p_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    size_t len = p_size * (1 + mmap_pages_count);
    size_t DATA_SIZE = p_size * mmap_pages_count;

    auto *metadata = (struct perf_event_mmap_page *)
            mmap(nullptr, len, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (metadata == MAP_FAILED) {
        throw std::system_error(errno, std::system_category(), "recordJump - mmap failed");
    }
    ioctl(fd, PERF_EVENT_IOC_REFRESH, 0);
    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);


    // wait for specific interval
    char *data_page = ((char *) metadata) + p_size;
    uint64_t consumed = 0;
    uint64_t last_head = 0;

    // replace this with a flag.
    while (!(*flag)) {
        // wait for interval
        while (metadata->data_head == last_head);;
        last_head = metadata->data_head;

        while (consumed < last_head) {
            struct __attribute__((packed)) sample {
                struct perf_event_header header;
                uint64_t ip;
            } *here = (struct sample *) (data_page + consumed % DATA_SIZE);

            if (here->header.type == PERF_RECORD_SAMPLE) {
                // Do something with the sample which fields start at address header + 8
                //model->consumeAddress(here->ip);
                tracer->use(&JumpTracer::consume, here->ip);
            }
            consumed += here->header.size;
        }
        metadata->data_tail = last_head;
    }

    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    if (munmap(metadata, len) == -1) {
        throw std::system_error(errno, std::system_category(), "recordJump - munmap failed");
    }
    close(fd);
    return true;
}
