//--------------------------------------------------------------------------------------------------
/**
 * Implementation of the appUserAdd program, which installs a user account to the target system
 * according to an application's configuration settings in the Configuration Tree.
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 **/
//--------------------------------------------------------------------------------------------------

#include "legato.h"
#include "components/userAdderRemover/userAdderRemover.h"

COMPONENT_INIT
{
    // Get the command-line argument (there should only be one).
    if (le_arg_NumArgs() > 1)
    {
        LE_FATAL("Too many arguments.\n");
    }
    const char* appName = le_arg_GetArg(0);

    if (appName == NULL)
    {
        fprintf(stderr, "App name required.\n");
        LE_FATAL("App name required.");
    }

    // Do the work.
#ifdef ADD_USER
    userAddRemove_Add(appName);
#else
    #ifdef REMOVE_USER
        userAddRemove_Remove(appName);
    #else
        #error MUST define either ADD_USER or REMOVE_USER.
    #endif
#endif

    exit(EXIT_SUCCESS);
}
