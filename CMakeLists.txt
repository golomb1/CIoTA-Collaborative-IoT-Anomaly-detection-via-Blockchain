cmake_minimum_required(VERSION 3.8)
project(CIoTA_CompleteProject)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package( Qt5Core REQUIRED )
find_package( Qt5Widgets REQUIRED )
find_package( Qt5Gui REQUIRED )


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)



file(GLOB CPPUtils_Headers headers/CppUtils/*.h)
file(GLOB CPPUtils_Cpp src/CppUtils/*.cpp)
set(CPPUtils
        ${CPPUtils_Headers}
        ${CPPUtils_Cpp}
        headers/CppUtils/minilzo/lzoconf.h
        headers/CppUtils/minilzo/lzodefs.h)


file(GLOB Blockchain_Headers headers/Blockchain/*.h)
file(GLOB Blockchain_Cpp src/Blockchain/*.cpp)
set(Blockchain
        ${Blockchain_Headers}
        ${Blockchain_Cpp})


file(GLOB CIoTA_Headers headers/CIoTA/*.h headers/CIoTA/core/*.h)
file(GLOB CIoTA_Cpp src/CIoTA/*.cpp)
set(CIoTA
        ${CIoTA_Headers}
        ${CIoTA_Cpp})


file(GLOB JUMP_TRACER_Headers headers/JumpTracer/*.h)
file(GLOB JUMP_TRACER_Cpp src/JumpTracer/*.cpp)
set(JUMP_TRACER
        ${JUMP_TRACER_Headers}
        ${JUMP_TRACER_Cpp})


file(GLOB IDS_Headers headers/AnomalyDetection/*.h)
file(GLOB IDS_Cpp src/AnomalyDetection/*.cpp)
set(IDS ${IDS_Headers} ${IDS_Cpp})

file(GLOB SIMPLE_TCP_PROTOCOL_Headers headers/NetworkingUtils/SimpleTcpProtocol/*.h)
file(GLOB SIMPLE_TCP_PROTOCOL_Cpp src/NetworkingUtils/SimpleTcpProtocol/*.cpp)
file(GLOB NETWORK_UTILS_Headers headers/NetworkingUtils/*.h)
file(GLOB NETWORK_UTILS_Cpp src/NetworkingUtils/*.cpp)
set(NetworkingUtils
        ${NETWORK_UTILS_Headers}
        ${NETWORK_UTILS_Cpp}
        ${SIMPLE_TCP_PROTOCOL_Headers}
        ${SIMPLE_TCP_PROTOCOL_Cpp})


set(SOURCE_FILES ${CPPUtils} ${Blockchain} ${NetworkingUtils} ${CIoTA} ${IDS} ${JUMP_TRACER})


file(GLOB TEST_JUMPTRACER_HEADERS test/TestSuite1/JumpTracer/*.h)
file(GLOB TEST_JUMPTRACER_CPP     test/TestSuite1/JumpTracer/*.cpp)
file(GLOB TEST_CIOTA_HEADERS      test/TestSuite1/CIoTA/*.h)
file(GLOB TEST_CIOTA_CPP          test/TestSuite1/CIoTA/*.cpp)
file(GLOB TEST_BLOCKCHAIN_HEADERS test/TestSuite1/Blockchain/*.h)
file(GLOB TEST_BLOCKCHAIN_CPP     test/TestSuite1/Blockchain/*.cpp)
file(GLOB TEST_MOCK_HEADERS       test/Mocks/*.h)
file(GLOB TEST_MOCK_CPP           test/Mocks/*.cpp)

file(GLOB TEST_SUITE_1_HEADERS       test/TestSuite1/*.h)
file(GLOB TEST_SUITE_1_CPP           test/TestSuite1/*.cpp)
file(GLOB TEST_SIMULATOR_HEADERS     test/Simulator/*.h)
file(GLOB TEST_SIMULATOR_CPP         test/Simulator/*.cpp)
file(GLOB TEST_SIMULATOR_HEADERS     test/Simulator/*.h)
file(GLOB TEST_SIMULATOR_CPP         test/Simulator/*.cpp)

set(TEST_SOURCES
        test/TEST.h

        ${TEST_JUMPTRACER_HEADERS}
        ${TEST_JUMPTRACER_CPP}
        ${TEST_CIOTA_HEADERS}
        ${TEST_CIOTA_CPP}
        ${TEST_BLOCKCHAIN_HEADERS}
        ${TEST_BLOCKCHAIN_CPP}
        ${TEST_MOCK_HEADERS}
        ${TEST_MOCK_CPP}
        ${TEST_SUITE_1_HEADERS}
        ${TEST_SUITE_1_CPP}
        ${TEST_SIMULATOR_HEADERS}
        ${TEST_SIMULATOR_CPP})


file(GLOB NETWORK_TEST_HEADERS test/NetworkTest/*.h)
file(GLOB NETWORK_TEST_CPP test/NetworkTest/*.cpp)
set(NETWORK_TEST ${NETWORK_TEST_HEADERS} ${NETWORK_TEST_CPP}
        src/CppUtils/Utils.cpp)


set(MATRIXSSL_ROOT ${CMAKE_SOURCE_DIR}/matrixssl)

# Tests
add_definitions(-DLOG_LEVEL=0)
add_definitions(-DLOG_TO_CONSOLE=false)
add_executable(TestsSuite test/testSuite.cpp ${SOURCE_FILES} ${TEST_SOURCES})
add_executable(NetworkingUtilsTest test/testNetwork.cpp ${NETWORK_TEST} ${NetworkingUtils} ${CPPUtils})
add_executable(MockAgent test/MockAgent.cpp ${SOURCE_FILES} ${TEST_SOURCES})
add_executable(MockServer test/MockServer.cpp ${SOURCE_FILES} ${TEST_SOURCES})
add_executable(MockGUIServer test/MockGUIServer.cpp ${SOURCE_FILES} ${TEST_SOURCES}
        src/ControlServer/GUI/mainwindow.cpp)


# Application
add_executable(cliServer
        src/ExecutableUtils/cliServer.cpp
        src/ExecutableUtils/CIoTA_Server.cpp
        headers/ExecutableUtils/CIoTA_Server.h
        ${SOURCE_FILES})

add_executable(guiServer
        src/ExecutableUtils/guiServer.cpp
        src/ExecutableUtils/CIoTA_Server.cpp
        src/ControlServer/GUI/mainwindow.cpp
        headers/ExecutableUtils/CIoTA_Server.h
        ${SOURCE_FILES})

add_executable(agent
        src/ExecutableUtils/agent.cpp
        src/ExecutableUtils/CIoTA_Agent.cpp
        headers/ExecutableUtils/CIoTA_Agent.h
        ${SOURCE_FILES})

add_executable(agent_demo
        Demo/agent_demo.cpp
        Demo/PibrellaBlockchainListener.h
        Demo/PibrellaBlockchainListener.cpp
        src/ExecutableUtils/CIoTA_Agent.cpp
        headers/ExecutableUtils/CIoTA_Agent.h
        ${SOURCE_FILES})


file(COPY CIoTA.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(IN_LIBRARIES
        ${CMAKE_THREAD_LIBS_INIT}
        ${MATRIXSSL_ROOT}/crypto/libcrypt_s.a
        ${MATRIXSSL_ROOT}/core/libcore_s.a)


target_link_libraries(TestsSuite ${IN_LIBRARIES})
target_link_libraries(NetworkingUtilsTest ${IN_LIBRARIES})
target_link_libraries(MockAgent ${IN_LIBRARIES})
target_link_libraries(MockServer ${IN_LIBRARIES})
target_link_libraries(MockGUIServer  ${IN_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::Gui)
target_link_libraries(cliServer      ${IN_LIBRARIES})
target_link_libraries(guiServer      ${IN_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::Gui)
target_link_libraries(agent          ${IN_LIBRARIES})
target_link_libraries(agent_demo     ${IN_LIBRARIES})
