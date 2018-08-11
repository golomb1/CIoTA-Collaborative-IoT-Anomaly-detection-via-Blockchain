//
// Created by master on 08/02/18.
//
#include <QApplication>
#include <iostream>
#include <src/ControlServer/GUI/mainwindow.h>
#include <headers/ExecutableUtils/CIoTA_Server.h>

#define NUM_OF_MANDATORY_ARGS    1
#define NUM_OF_OPT_ARGS          1
#define OPT_ARG_LOG              2


void printUsage(char* argv0){
    std::string programName(argv0);
    unsigned long pos = programName.find_last_of('/');
    programName = programName.substr(pos + 1);
    std::cout << "Usage \"" << programName << " configurationFile\"." << std::endl;
    std::cout << "configurationFile is the INI file containing CIoTA configurations." << std::endl;
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[]) {
    if (!(argc >= NUM_OF_MANDATORY_ARGS + 1 && argc <= NUM_OF_MANDATORY_ARGS + NUM_OF_OPT_ARGS + 1)) {
        printUsage(argv[0]);
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.setHashSize(SHA256_OUTPUT_SIZE);
    w.setCompleteBlockSize(5);
    w.show();

    QApplication* pA = &a;
    PrinterObserverListener<JumpTracer> listener(&w);
    if(!(runConfigurableServer(argv[1], [pA](){pA->exec();}, &w, &listener, &w)))
    {
        std::cout << "Can't load 'test.ini'\n";
        printUsage(argv[0]);
    }
    return EXIT_SUCCESS;
}