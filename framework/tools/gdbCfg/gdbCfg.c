/** @file gdbCfg.c
 *
 * Tool used to configure an application so that gdb can be used to start the application's
 * processes individually.
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 */

#include "legato.h"
#include "interfaces.h"
#include "limit.h"
#include "configInstaller.h"


//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of processes that can be disabled.
 **/
//--------------------------------------------------------------------------------------------------
#define MAX_NUM_PROCS 256


//--------------------------------------------------------------------------------------------------
/**
 * Debug tool node in the config.  Used to indicate the debug tool that has modified an
 * application's configuration.
 */
//--------------------------------------------------------------------------------------------------
#define CFG_DEBUG_TOOL                  "debugTool"


//--------------------------------------------------------------------------------------------------
/**
 * Application name provided on the command line.
 **/
//--------------------------------------------------------------------------------------------------
static const char* AppName;


//--------------------------------------------------------------------------------------------------
/**
 * List of process names that have been provided on the command line.
 **/
//--------------------------------------------------------------------------------------------------
static const char* ProcNames[MAX_NUM_PROCS];


//--------------------------------------------------------------------------------------------------
/**
 * Number of process names that have been provided on the command line.
 **/
//--------------------------------------------------------------------------------------------------
static size_t NumProcs = 0;


//--------------------------------------------------------------------------------------------------
/**
 * true if the --reset option was specified on the command line.
 **/
//--------------------------------------------------------------------------------------------------
static bool DoReset = false;


//--------------------------------------------------------------------------------------------------
/**
 * Import object.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char src[LIMIT_MAX_PATH_BYTES];
    char dest[LIMIT_MAX_PATH_BYTES];
}
ImportObj_t;


//--------------------------------------------------------------------------------------------------
/**
 * Files to import for gdb.
 */
//--------------------------------------------------------------------------------------------------
const ImportObj_t GdbFilesImports[] = { {.src = "/usr/bin/gdbserver",    .dest = "/bin/"},
                                        {.src = "/lib/libdl.so.2",       .dest = "/lib/"},
                                        {.src = "/lib/libgcc_s.so.1",    .dest = "/lib/"} };


//--------------------------------------------------------------------------------------------------
/**
 * Directories to import for gdb.
 */
//--------------------------------------------------------------------------------------------------
const ImportObj_t GdbDirsImports[] = { {.src = "/proc", .dest = "/"} };


//--------------------------------------------------------------------------------------------------
/**
 * Prints a generic message on stderr so that the user is aware there is a problem, logs the
 * internal error message and exit.
 */
//--------------------------------------------------------------------------------------------------
#define INTERNAL_ERR(formatString, ...)                                                 \
            { fprintf(stderr, "Internal error check logs for details.\n");              \
              LE_FATAL(formatString, ##__VA_ARGS__); }


//--------------------------------------------------------------------------------------------------
/**
 * If the condition is true, print a generic message on stderr so that the user is aware there is a
 * problem, log the internal error message and exit.
 */
//--------------------------------------------------------------------------------------------------
#define INTERNAL_ERR_IF(condition, formatString, ...)                                   \
        if (condition) { INTERNAL_ERR(formatString, ##__VA_ARGS__); }


//--------------------------------------------------------------------------------------------------
/**
 * Prints help to stdout.
 */
//--------------------------------------------------------------------------------------------------
static void PrintHelp
(
    void
)
{
    puts(
        "NAME:\n"
        "    gdbCfg - Modify an application's configuration settings to make it suitable to run\n"
        "             gdb.\n"
        "\n"
        "SYNOPSIS:\n"
        "    gdbCfg appName [processName ...]\n"
        "    gdbCfg appName --reset\n"
        "\n"
        "DESCRIPTION:\n"
        "    gdbCfg appName [processName ...].\n"
        "       Adds gdbserver and /proc to the application's files section.  Removes the\n"
        "       specified processes from the application's procs section.\n"
        "\n"
        "    gdbCfg appName --reset\n"
        "       Resets the application to its original configuration.\n"
        "\n"
        "    gdbCfg --help\n"
        "        Display this help and exit.\n"
        );

    exit(EXIT_SUCCESS);
}


//--------------------------------------------------------------------------------------------------
/**
 * Adds files or directories to be imported to the application sandbox.
 */
//--------------------------------------------------------------------------------------------------
static void AddImportFiles
(
    le_cfg_IteratorRef_t cfgIter,           ///< [IN] Iterator to the application config.
    const ImportObj_t (*importsPtr)[],      ///< [IN] Imports to include in the application.
    size_t numImports                       ///< [IN] Number of import elements.
)
{
    // Find the last node under the 'files' or 'dirs' section.
    size_t nodeNum = 0;
    char nodePath[LIMIT_MAX_PATH_BYTES];

    while (1)
    {
        int n = snprintf(nodePath, sizeof(nodePath), "%zd", nodeNum);

        INTERNAL_ERR_IF(n >= sizeof(nodePath), "Node name is too long.");
        INTERNAL_ERR_IF(n < 0, "Format error.  %m");

        if (!le_cfg_NodeExists(cfgIter, nodePath))
        {
            break;
        }

        nodeNum++;
    }

    // Start adding files/directories at the end of the current list.
    int i;
    for (i = 0; i < numImports; i++)
    {
        // Add the source.
        int n = snprintf(nodePath, sizeof(nodePath), "%zd/src", nodeNum + i);

        INTERNAL_ERR_IF(n >= sizeof(nodePath), "Node name is too long.");
        INTERNAL_ERR_IF(n < 0, "Format error.  %m");

        le_cfg_SetString(cfgIter, nodePath, (*importsPtr)[i].src);

        // Add the destination.
        n = snprintf(nodePath, sizeof(nodePath), "%zd/dest", nodeNum + i);

        INTERNAL_ERR_IF(n >= sizeof(nodePath), "Node name is too long.");
        INTERNAL_ERR_IF(n < 0, "Format error.  %m");

        le_cfg_SetString(cfgIter, nodePath, (*importsPtr)[i].dest);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Configures the application for gdb.  Adds the gdbserver executable and required libraries to the
 * 'files' section and removes the list of processes from the 'procs' section in the config.
 */
//--------------------------------------------------------------------------------------------------
static void ConfigureGdb
(
    void
)
{
    le_cfg_ConnectService();
    le_cfgAdmin_ConnectService();

    // Get a write iterator to the application node.
    le_cfg_IteratorRef_t cfgIter = le_cfg_CreateWriteTxn("/apps");
    le_cfg_GoToNode(cfgIter, AppName);

    // Check if this is a temporary configuration that was previously created by this or a similar
    // tool.
    if (!le_cfg_IsEmpty(cfgIter, CFG_DEBUG_TOOL))
    {
        char debugTool[LIMIT_MAX_PATH_BYTES];

        // Don't need to check return code because the value is just informative and does not matter
        // if it is truncated.
        le_cfg_GetString(cfgIter, CFG_DEBUG_TOOL, debugTool, sizeof(debugTool), "");

        fprintf(stderr, "This application has already been configured for %s debug mode.\n", debugTool);
        exit(EXIT_FAILURE);
    }

    // Write into the config's debug tool node to indicate that this configuration has been modified.
    le_cfg_SetString(cfgIter, CFG_DEBUG_TOOL, "gdb");

    // Add gdbserver and libs to the app's 'requires/files' section.
    le_cfg_GoToNode(cfgIter, "requires/files");
    AddImportFiles(cfgIter, &GdbFilesImports, NUM_ARRAY_MEMBERS(GdbFilesImports));

    // Add /proc to the app's dirs section.
    le_cfg_GoToParent(cfgIter);
    le_cfg_GoToNode(cfgIter, "dirs");
    AddImportFiles(cfgIter, &GdbDirsImports, NUM_ARRAY_MEMBERS(GdbDirsImports));

    // Delete the list of processes.
    le_cfg_GoToParent(cfgIter);
    le_cfg_GoToParent(cfgIter);
    int i;
    for (i = 0; i < NumProcs; i++)
    {
        char nodePath[LIMIT_MAX_PATH_BYTES];

        int n = snprintf(nodePath, sizeof(nodePath), "procs/%s", ProcNames[i]);

        INTERNAL_ERR_IF(n >= sizeof(nodePath), "Node name is too long.");
        INTERNAL_ERR_IF(n < 0, "Format error.  %m");

        le_cfg_DeleteNode(cfgIter, nodePath);
    }

    le_cfg_CommitTxn(cfgIter);
}


//--------------------------------------------------------------------------------------------------
/**
 * Resets the application from its gdb configuration to its original configuration.
 */
//--------------------------------------------------------------------------------------------------
static void ResetApp
(
    void
)
{
    le_cfg_ConnectService();
    le_cfgAdmin_ConnectService();

    // Get a write iterator to the application node.
    le_cfg_IteratorRef_t cfgIter = le_cfg_CreateWriteTxn("/apps");
    le_cfg_GoToNode(cfgIter, AppName);

    // Check if this is a temporary configuration that was previously created by this or a similar
    // tool.
    if (le_cfg_IsEmpty(cfgIter, CFG_DEBUG_TOOL))
    {
        fprintf(stderr, "This application already has its original configuration.\n");
        exit(EXIT_FAILURE);
    }

    // Blow away what's in there now.
    le_cfg_GoToNode(cfgIter, "/apps");
    le_cfg_DeleteNode(cfgIter, AppName);

    le_cfg_CommitTxn(cfgIter);

    // NOTE: Currently there is a bug in the config DB where deletions and imports cannot be done in
    //       the same transaction so we must do it in two transactions.
    cfgInstall_Add(AppName);
}


//--------------------------------------------------------------------------------------------------
/**
 * Function called with the app name from the command line.
 **/
//--------------------------------------------------------------------------------------------------
static void HandleAppName
(
    const char* appName
)
{
    AppName = appName;

    // Now that we have received the only mandatory argument, we can allow less positional
    // arguments than callbacks.
    le_arg_AllowLessPositionalArgsThanCallbacks();
}


//--------------------------------------------------------------------------------------------------
/**
 * Function called with each process name from the command line.
 **/
//--------------------------------------------------------------------------------------------------
static void HandleProcessName
(
    const char* procName
)
{
    if (NumProcs >= MAX_NUM_PROCS)
    {
        fprintf(stderr, "Too many process names provided.\n");
        exit(EXIT_FAILURE);
    }

    ProcNames[NumProcs++] = procName;
}


//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    // SYNOPSIS:
    //     gdbCfg appName [processName ...]
    le_arg_AddPositionalCallback(HandleAppName);
    le_arg_AddPositionalCallback(HandleProcessName);
    le_arg_AllowMorePositionalArgsThanCallbacks();

    //     gdbCfg appName --reset
    //     Resets the application to its original configuration.
    le_arg_SetFlagVar(&DoReset, NULL, "reset");

    //     gdbCfg --help
    //         Display help and exit.
    le_arg_SetFlagCallback(PrintHelp, NULL, "help");

    le_arg_Scan();

    if (DoReset)
    {
        if (NumProcs != 0)
        {
            fprintf(stderr, "List of processes not valid with --reset option.\n");
            exit(EXIT_FAILURE);
        }
        ResetApp();
    }
    else
    {
        ConfigureGdb();
    }

    exit(EXIT_SUCCESS);
}
