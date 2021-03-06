#---------------------------------------------------------------------------------------------------
# Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
#---------------------------------------------------------------------------------------------------

find_package(Legato REQUIRED)

### Copy test setup script to test program output directory.
file( COPY testFwMessaging-Setup DESTINATION ${EXECUTABLE_OUTPUT_PATH} )


### TEST 1

set(TEST_NAME testFwMessaging-Test1)

mkexe(  ${TEST_NAME} -v
            messagingTest1.c
            burgerServer.c
        DEPENDS
            burgerProtocol.h
            burgerServer.h
            burgerServer.c
            messagingTest1.c
        )

add_test(${TEST_NAME} ${EXECUTABLE_OUTPUT_PATH}/${TEST_NAME})


### TEST 2

set(TEST_NAME testFwMessaging-Test2)

mkexe(  ${TEST_NAME}
            messagingTest2.c
            burgerServer.c
        DEPENDS
            burgerProtocol.h
            burgerServer.h
            burgerServer.c
            messagingTest2.c
        )

add_test(${TEST_NAME} ${EXECUTABLE_OUTPUT_PATH}/${TEST_NAME})


### TEST 3

set(TEST_NAME testFwMessaging-Test3)

mkexe(  ${TEST_NAME}-client
            messagingTest3-client.c
        DEPENDS
            messagingTest3-client.c
        )

mkexe(  ${TEST_NAME}-server
            messagingTest3-server.c
        DEPENDS
            messagingTest3-server.c
        )

mkexe(  ${TEST_NAME}
            messagingTest3.c
        DEPENDS
            messagingTest3.c
        )

add_test(${TEST_NAME} ${EXECUTABLE_OUTPUT_PATH}/${TEST_NAME})

