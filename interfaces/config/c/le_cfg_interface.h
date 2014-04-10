/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */

/**
 *  @page api_config Configuration Tree API
 *
 *  @ref le_cfg_interface.h "Click here for the API reference documentation."
 *
 * <HR>
 * @ref cfg_transaction <br>
 * @ref cfg_iteration <br>
 * @ref cfg_transactWrite <br>
 * @ref cfg_transactRead <br>
 * @ref cfg_quick
 *
 *  The configuration tree API allows applications to read and write their specific configuration.
 *  Each application is given an isolated tree. The system utilities
 *  store their configuration in the "root" tree.
 *
 *  Paths in the tree look like traditional Unix style paths and take the form of:
 *
 *  @code /path/to/my/value @endcode
 *
 *  The path root is the root of the tree where the application has been given access. If the
 *  application has permission to access another tree, the path can also include the name
 *  of the other tree, followed by a colon.
 *
 *  @code secondTree:/path/to/my/value @endcode
 *
 *  A value named "value" is read from the tree named, "secondTree."
 *
 *  The tree is broken down into stems and leaves. A stem is a node that has at least one child
 *  node.  While a leaf has no children, but it can hold a value.
 *
 *  The configuration tree supports string, signed integer, boolean, floating point, and empty
 *  values. It's is encouraged to use stems and leafs for storing anything more complex to enhance
 *  readaility and debugging.  This also sidesteps nasty cross platform alignment
 *  issues.
 *
 * @todo Talk about the treeConfig, user limits, tree name, tree access.  Global timeouts.
 *
 *  @section cfg_transaction A Transactional Approach
 *
 *  The configuration tree makes use of simple transactions to work with its data.  Both read
 *  and write transactions are supported.  You want to use read transactions  to
 *  ensure you can atomically read multiple values from your configuration while keeping consistency
 *  with third parties trying to write data.
 *
 * To prevent any single client from locking out other clients, read and
 *  write transactions have their own configurable timeout.
 *
 *  During a write transaction, both reading and writing are allowed. If you
 *  write a value during a transaction and read from that value again, you will get the same value
 *  you wrote. Third-party clients will continue to see the old value.  It's not until you commit
 *  your transaction that third parties will begin to see your updated value.
 *
 *  During read transactions, writes are not permitted and are thrown away.
 *
 *  Transactions are started by creating an iterator. Either a read or write iterator can be
 *  created. To end the transaction, you can either delete the iterator to cancel the
 *  transaction; or, for write transactions, you can commit the iterator.
 *
 * You can have multiple read transactions against the tree. They won't
 * block other transactions from being creating. A read transaction won't block creating a write
 *  transaction either. A read transaction only blocks a write transaction from being
 *  comitted to the tree.
 *
 *  A write transaction in progress will also block creating another write transaction.
 *  If a write transaction is in progress when the request for another write transaction comes in,
 *  the second request will be blocked. The second request will remain blocked until the
 *  first transaction has been comitted or has timed out.
 *
 *
 *  @section cfg_iteration Iterating the Tree
 *
 *  This code sample will iterate a given node and print its contents:
 *
 *  @code
 *  static void PrintNode(le_cfg_IteratorRef_t iteratorRef)
 *  {
 *      do
 *      {
 *          char stringBuffer[MAX_CFG_STRING] = { 0 };
 *
 *          le_cfg_GetNodeName(iteratorRef, &stringBuffer, sizeof(stringBuffer));
 *
 *          switch (cfg_GetNodeType(iteratorRef))
 *          {
 *              case CFG_TYPE_STEM:
 *                  {
 *                      printf("%s/\n", stringBuffer);
 *
 *                      le_cfg_IteratorRef_t subIterator = le_cfg_CloneIterator(iteratorRef);
 *                      if (le_cfg_GoToFirstChild(subIterator) == LE_OK)
 *                      {
 *                          PrintNode(subIterator);
 *                      }
 *
 *                      le_cfg_DeleteIterator(subIterator);
 *                  }
 *                  break;
 *
 *              case CFG_TYPE_EMPTY:
 *                  printf("%s = *empty*\n", stringBuffer);
 *                  break;
 *
 *              case CFG_TYPE_BOOL:
 *                  {
 *                      bool value = false;
 *
 *                      le_cfg_GetBool(iteratorRef, stringBuffer, &value);
 *                      printf("%s = %s\n", stringBuffer, (value ? "true" : "false"));
 *                  }
 *                  break;
 *
 *              case CFG_TYPE_INT:
 *                  {
 *                      int32_t intValue = 0;
 *
 *                      le_cfg_GetInt(iteratorRef, stringBuffer, &intValue);
 *                      printf("%s = %d\n", stringBuffer, intValue);
 *                  }
 *                  break;
 *
 *              case CFG_TYPE_FLOAT:
 *                  {
 *                      double floatValue = 0.0;
 *
 *                      le_cfg_GetFloat(iteratorRef, stringBuffer, &floatValue);
 *                      printf("%s = %f\n", stringBuffer, floatValue);
 *                  }
 *                  break;
 *
 *              case CFG_TYPE_STRING:
 *                  printf("%s = ", stringBuffer);
 *                  le_cfg_GetString(iteratorRef, stringBuffer, stringBuffer);
 *                  printf("%s\n", stringBuffer);
 *                  break;
 *
 *              case CFG_TYPE_DENIED:
 *                  printf("%s = ** DENIED **\n", stringBuffer);
 *                  break;
 *
 *              case CFG_TYPE_BAD_PATH:
 *                  printf("%s is a bad path.\n")
 *                  break;
 *          }
 *      }
 *      while (le_cfg_GoToNextSibling(iteratorRef) == LE_OK);
 *  }
 *
 *
 *  le_cfg_IteratorRef_t iteratorRef = le_cfg_CreateReadIterator("/path/to/my/location");
 *
 *  PrintNode(iteratorRef);
 *  le_cfg_DeleteIterator(iteratorRef);
 *
 *
 *  @endcode
 *
 *
 *  @section cfg_transactWrite Writing Configuration Data, using a write transaction.
 *
 *  In this sample, the caller wants to update the devices IP address. It's done
 *  in a transaction so the data is written atomically.
 *
 *  @code
 *  static le_result_t SetIp4Static
 *  (
 *      le_cfg_iteratorRef_t currentIterRef,
 *      const char* interfaceNamePtr,
 *      const char* ipAddrPtr,
 *      const char* netMaskPtr
 *  )
 *  {
 *      le_result_t result;
 *
 *      LE_ASSERT(le_cfg_IsWriteable(currentIterRef));
 *
 *      le_cfg_IteratorRef_t iteratorRef = le_cfg_CloneIterator(currentIterRef);
 *
 *      // Change current tree position to the base ip4 node.
 *      char nameBuffer[MAX_CFG_STRING] = { 0 };
 *
 *      sprintf(nameBuffer, "/system/%s/ip4", interfaceNamePtr);
 *
 *      if (le_cfg_GoToNode(iteratorRef, nameBuffer) != LE_OK)
 *      {
 *          LE_FATAL("**Move failed: %s", LE_RESULT_TXT(result));
 *      }
 *
 *      le_cfg_SetString(iteratorRef, "addr", ipAddrPtr);
 *      le_cfg_SetString(iteratorRef, “mask”, netMaskPtr);
 *
 *      result = le_cfg_CommitWrite(iteratorRef);
 *
 *      if (result != LE_OK)
 *      {
 *          LE_CRIT("Failed to write IPv4 configuration for interface '%s'.  Error %s.",
 *                  interfaceNamePtr,
 *                  LE_RESULT_TXT(result));
 *      }
 *
 *      return result;
 *  }
 *  @endcode
 *
 *
 *  @section cfg_transactRead Reading configuration data with a read transaction.
 *
 *  @code
 *  static le_result_t GetIp4Static
 *  (
 *      le_cfg_iteratorRef_t currentIterRef,
 *      const char* interfaceNamePtr,
 *      char* ipAddrPtr,
 *      char* netMaskPtr
 *  )
 *  {
 *      le_cfg_IteratorRef_t iteratorRef = le_cfg_CloneIterator(currentIterRef);
 *
 *      // Change current tree position to the base ip4 node.
 *      char nameBuffer[MAX_CFG_STRING] = { 0 };
 *
 *      sprintf(nameBuffer, "/system/%s/ip4", interfaceNamePtr);
 *      le_cfg_GoToNode(iteratorRef, nameBuffer);
 *      if (le_cfg_GetIteratorState(iteratorRef) == CFG_TYPE_DENIED)
 *      {
 *          LE_FATAL("**DENIED**");
 *      }
 *
 *      le_cfg_GetString(iteratorRef, "addr", ipAddrPtr);
 *      le_cfg_GetString(iteratorRef, "mask", netMaskPtr);
 *
 *      return result;
 *  }
 *  @endcode
 *
 *
 *  @section cfg_quick Working without Transactions
 *
 *  It's possible to ignore iterators and transactions entirely (e.g., if all you need to do
 *  is read or write some simple values in the tree).
 *
 *  The non-transactional reads and writes work almost identically to the transactional versions only they
 *  don't just explictly take an iterator object. The "quick" functions internally use an
 *  implicit transaction.  This implicit transaction wraps one get or set, and does not protect
 *  your code from other activity in the system.
 *
 *  Because these functions don't take an explicit transaction, they can't work with relative
 *  paths.  If a relative path is given, the path will be considered relative to the tree's root.
 *
 *  Translating the last examples to their "quick" counterparts, you have the following code.
 *  Because each read is independant, there is no guarantee of
 *  consistency between them.  If another process changes one of the values while you
 *  read/write the other, the two values could be read out of sync.
 *
 *  @code
 *
 *  static le_result_t ClearIpInfo
 *  (
 *      const char* interfaceNamePtr
 *  )
 *  {
 *      le_result_t result;
 *
 *      char pathBuffer[MAX_CFG_STRING] = { 0 };
 *
 *      sprintf(pathBuffer, "/system/%s/ip4/", interfaceNamePtr);
 *
 *      result = le_cfg_QuickDeleteNode(pathBuffer);
 *
 *      if (result != LE_OK)
 *      {
 *          LE_CRIT("Failed to clear IPv4 configuration for interface '%s'.  Error %s.",
 *                  interfaceNamePtr,
 *                  LE_RESULT_TXT(result));
 *      }
 *
 *      return result;
 *  }
 *
 *  @endcode
 *
 *  Copyright (C) Sierra Wireless, Inc. 2014. All rights reserved. Use of this work is subject to license.
 */
/** @file le_cfg_interface.h

@ref api_config

*  Copyright (C) Sierra Wireless, Inc. 2014. All rights reserved. Use of this work is subject to license.
 */

#ifndef LE_CFG_INTERFACE_H_INCLUDE_GUARD
#define LE_CFG_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"

// User customizable include file
#include "configTypes.h"


//--------------------------------------------------------------------------------------------------
/**
 * Start the client main thread
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_StartClient
(
    const char* serviceInstanceName
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Stop the service for the current client thread
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_StopClient
(
    void
);


typedef void (*le_cfg_ChangeHandlerFunc_t)
(
    void* contextPtr
);

//--------------------------------------------------------------------------------------------------
/**
 *  Create a read transaction and open a new iterator for traversing the configuration tree.
 *
 *  @note: This action creates a read transaction that will exist for the lifetime of all
 *         active iterators. If the application holds the iterator for longer than the configured read
 *         transaction timeout, active iterators will become invalid and no longer return data.
 *
 *  @note: A tree transaction is global; a long held read transaction will block other users
 *         write transactions from being comitted.
 */
//--------------------------------------------------------------------------------------------------
le_cfg_iteratorRef_t le_cfg_CreateReadTxn
(
    const char* basePath
        ///< [IN]
        ///< Path to the location to create the new iterator.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Create a write transaction and open a new iterator for both reading and writing.
 &
 *  @note: This action creates a write transaction. If the application holds the iterator for
 *         longer than the configured write transaction timeout, the iterator will cancel the transaction.
 *         All further reads will fail to return data and all writes will be thrown away.
 *
 *  @note A tree transaction is global, so a long held read transaction will block other user's
 *        write transactions from being comitted.  However other trees in the system will be
 *        unaffected.
 *
 *  @return This will return a newly created iterator reference.
 */
//--------------------------------------------------------------------------------------------------
le_cfg_iteratorRef_t le_cfg_CreateWriteTxn
(
    const char* basePath
        ///< [IN]
        ///< Path to the location to create the new iterator.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Close the write iterator and commit the write transaction.  This updates the config tree
 *  with all of the writes that occured using the iterator.
 *
 *  If the transaction had timed out, or if the iterator has been moved out of bounds, the commit will
 *  fail.
 *
 *  @note: This operation will also delete the iterator object so you don't have to call
 *         DeleteIterator.
 *
 *  @note: All clones of this iterator also have to commit their write transactions before the
 *         transaction is actually comitted to the tree.
 *
 *  @return Will return one of the following:
 *
 *          - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid iterator for this request.
 *          - LE_NOT_PERMITTED - Attempted an iterator that has been moved out of bounds.
 *          - LE_TIMEOUT       - Transaction had timed out.
 *          - LE_CLOSED        - Transaction was canceled by one of the clones of this
 *                               iterator.  Nothing has been committed.
 *          - LE_WOULD_BLOCK   - Data has been committed to the parent transaction, but other
 *                               iterators of this transaction are still outstanding.  This data will
 *                               only be comitted to the live tree when all clones have been
 *                               comitted successfuly.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_CommitWrite
(
    le_cfg_iteratorRef_t iteratorRef
        ///< [IN]
        ///< Iterator object to commit.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Close and free the given iterator object.  If the iterator is a write iterator, the transaction
 *  will be canceled.  If the iterator is a read iterator, the transaction will be closed.
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_DeleteIterator
(
    le_cfg_iteratorRef_t iteratorRef
        ///< [IN]
        ///< Iterator object to close.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Change the stem that the iterator is pointing to.  The path passed can be an
 *  absolute or a relative path from the iterators current location.
 *
 *  Calling GoToNode with a path of "." will jump the iterator back to the first sub-item
 *  of the current stem.
 *
 *  @return Returns one of the following values:
 *
 *          - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid iterator for this request.
 *          - LE_NOT_PERMITTED - Attempted the iterator outside of the allowed area
 *                              (e.g., trying to change trees in an iterator is not permitted).
 *          - LE_NOT_POSSIBLE  - Could not look up permission information.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_GoToNode
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to move.

    const char* newPath
        ///< [IN]
        ///< Absolute or relative path from the current location
);

//--------------------------------------------------------------------------------------------------
/**
 *  Move the iterator to the parent of the node.
 *
 *  @return Return code will be one of the following values:
 *
 *          - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid iterator for this request.
 *          - LE_NOT_PERMITTED - Current node is the root node: has no parent.
 *          - LE_NOT_POSSIBLE  - Could not look up permission information.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_GoToParent
(
    le_cfg_iteratorRef_t iteratorRef
        ///< [IN]
        ///< Iterator to move.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Move the iterator to the the first child of the node where the iterator is currently pointed.
 *
 *  For read iterators without children, this function will fail. If the iterator is a write
 *  iterator, then a new node is automatically created.  If this node or newly created
 *  children of this node are not written to, then this node will not persist even if the iterator is
 *  comitted.
 *
 *  @return Return code will be one of the following values:
 *
 *          - LE_OK            - Move was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid iterator for this request.
 *          - LE_NOT_PERMITTED - Attempted to create a new node on a read iterator.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_GoToFirstChild
(
    le_cfg_iteratorRef_t iteratorRef
        ///< [IN]
        ///< Iterator object to move.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Jump the iterator to the next child node of the current node.  Assuming the following tree:
 *
 *  @code
 *  baseNode/
 *    childA/
 *      valueA
 *      valueB
 *  @endcode
 *
 *  If the iterator is moved to the path, "/baseNode/childA/valueA".  After the first
 *  GoToNextSibling the iterator will be pointing at valueB.  A second call to GoToNextSibling
 *  will cause the function to return LE_NOT_FOUND.
 *
 *  @return Returns one of the following values:
 *
 *          - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted to use an invalid iterator for this request.
 *          - LE_NOT_FOUND     - Iterator has reached the end of the current list of sub nodes.
 *                               Also returned if the the current node has no sub items.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_GoToNextSibling
(
    le_cfg_iteratorRef_t iteratorRef
        ///< [IN]
        ///< Iterator to iterate.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Check the iterator to see the interator represents a write transaction.
 *
 *  @return - true - Write transaction object.
 *          - false - Read only transaction object.
 */
//--------------------------------------------------------------------------------------------------
bool le_cfg_IsWriteable
(
    le_cfg_iteratorRef_t iteratorRef
        ///< [IN]
        ///< Iterator to query.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Check to see if the iterator reference points to a valid iterator object.
 *
 *  @note An iterator is considered no longer valid if one if its clones cancels the underlying
 *        transaction.
 *
 *  @return - true  - Object is valid and can be used.
 *          - false - Object is no longer valid either through a security violation or its
 *                    been cancelled.
 */
//--------------------------------------------------------------------------------------------------
bool le_cfg_IsValid
(
    le_cfg_iteratorRef_t iteratorRef
        ///< [IN]
        ///< Iterator to query.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Get path to the node where the iterator is currently pointed.
 *
 *  Assuming the following tree:
 *
 *  @code
 *  baseNode/
 *    childA/
 *      valueA
 *      valueB
 *  @endcode
 *
 *  If the iterator was currently pointing at valueA, GetPath would return the following path:
 *
 *  @code
 *  /baseNode/childA/valueA
 *  @endcode
 *
 *  @return - LE_OK            - Write was completed successfuly.
 *          - LE_OVERFLOW      - Supplied string buffer was not large enough to hold the value.
 *          - LE_BAD_PARAMETER - Supplied iterator reference was invalid.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_GetPath
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to move.

    char* newPath,
        ///< [OUT]
        ///< Absolute path to the iterator's current node.

    size_t newPathNumElements
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 *  Get the path to the parent of the node where the iterator is currently pointed.
 *
 *  Assuming the following tree:
 *
 *  @code
 *  baseNode/
 *    childA/
 *      valueA
 *      valueB
 *  @endcode
 *
 *  If the iterator was currently pointing at valueB,  GetParentPath would return the following
 *  path:
 *
 *  @code
 *  /baseNode/childA/
 *  @endcode
 *
 *  @return - LE_OK            - Write was completed successfuly.
 *          - LE_OVERFLOW      - Supplied string buffer was not large enough to hold the value.
 *          - LE_BAD_PARAMETER - Supplied iterator reference was invalid.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_GetParentPath
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to move.

    char* newPath,
        ///< [OUT]
        ///< Absolute path for the parent of the node that the
        ///<   iterator is residing on.

    size_t newPathNumElements
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 *  Get the type of node where the iterator is currently pointing.
 *
 *  @return le_cfg_nodeType_t value indicating the stored value.
 */
//--------------------------------------------------------------------------------------------------
le_cfg_nodeType_t le_cfg_GetNodeType
(
    le_cfg_iteratorRef_t iteratorRef
        ///< [IN]
        ///< Iterator object to use to read from the tree.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Get the name of the node where the iterator is currently pointing.
 *
 *  @return - LE_OK           - Write was completed successfuly.
 *          - LE_OVERFLOW      - Supplied string buffer was not large enough to hold the value.
 *          - LE_BAD_PARAMETER - Supplied iterator reference was invalid.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_GetNodeName
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator object to use to read from the tree.

    char* name,
        ///< [OUT]
        ///< Read the name of the node object.

    size_t nameNumElements
        ///< [IN]
);

typedef struct le_cfg_ChangeHandler* le_cfg_ChangeHandlerRef_t;

//--------------------------------------------------------------------------------------------------
/**
 * This function adds a handler ...
 */
//--------------------------------------------------------------------------------------------------
le_cfg_ChangeHandlerRef_t le_cfg_AddChangeHandler
(
    const char* newPath,
        ///< [IN]
        ///< Path to the object to watch.

    le_cfg_ChangeHandlerFunc_t handlerPtr,
        ///< [IN]

    void* contextPtr
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * This function removes a handler ...
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_RemoveChangeHandler
(
    le_cfg_ChangeHandlerRef_t addHandlerRef
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 *  Delete the node specified by the path.  If the node doesn't exist, nothing happens.  All child
 *  nodes are also deleted.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that is out of bounds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickDeleteNode
(
    const char* path
        ///< [IN]
        ///< Path to the node to delete.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Check if the given node is empty.  A node is also considered empty if it doesn't exist.
 *
 *  @return - LE_OK            - Write was completed successfuly.
 *            LE_NOT_PERMITTED - Node could not be found, or the requested tree could not be
 *                               accessed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickSetEmpty
(
    const char* path
        ///< [IN]
        ///< Absolute or relative path to read from.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Read a string value from the configuration tree.  If the stored value is not a string, the value
 *  will be converted into a string.
 *
 *  If the value is a number, a string with that number is returned.  If the value is empty, or
 *  the iterator is invalid, an empty string is returned.  If the value is boolean, the string,
 *  "true" or "false" is returned.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that is out of bounds, or in a tree
 *                               without necessary access.
 *          - LE_OVERFLOW      - Supplied string buffer was not large enough to hold the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickGetString
(
    const char* path,
        ///< [IN]
        ///< Path to read from.

    char* value,
        ///< [OUT]
        ///< Value read from the requested node.

    size_t valueNumElements
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 *  Write a string value to the configuration tree.
 *
 *  When strings are written to the configuration tree, an attempt is made to guess the type of the
 *  string.
 *
 *  The algorithim used for this guess is as follows:
 *
 *  - If the string is the literal value, "true" or "false" then the value is treated as a boolean.
 *  - If the string contains nothing but numeric characters, optionally starting with a - then it is
 *    treated as an integer.
 *  - If the value contains a decimal place, and/or an exponent,  it's treated as a float.
 *  - All other values are treated as a string.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that's out of bounds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickSetString
(
    const char* path,
        ///< [IN]
        ///< Path to the value to write.

    const char* value
        ///< [IN]
        ///< Value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Read a signed integer value from the configuration tree.  If the underlying value is not an
 *  integer, it will be converted.
 *
 *  If the value is a string, 0 is returned.  If the value is a float, a truncated int is
 *  returned.  If the value is a bool, a 0 or a 1 is returned.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that is out of bounds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickGetInt
(
    const char* path,
        ///< [IN]
        ///< Path to the value to write.

    int32_t* valuePtr
        ///< [OUT]
        ///< Value to read.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Write a signed integer value to the configuration tree.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that is out of bounds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickSetInt
(
    const char* path,
        ///< [IN]
        ///< Path to the value to write.

    int32_t value
        ///< [IN]
        ///< Value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Read a 64-bit floating point value from the configuration tree. If the underlying value is not
 *  a float, it will be converted.
 *
 *  If the value is an int, then a straight conversion will be performed.  Bool values will be read
 *  as either a floating point, 0 or 1.  Strings and empty values are read as 0.0.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that is out of bounds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickGetFloat
(
    const char* path,
        ///< [IN]
        ///< Path to the value to write.

    double* valuePtr
        ///< [OUT]
        ///< Value to read.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Write a 64-bit floating point value to the configuration tree.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that is out of bounds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickSetFloat
(
    const char* path,
        ///< [IN]
        ///< Path to the value to write.

    double value
        ///< [IN]
        ///< Value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Read a value from the tree as a boolean.  Empty values are considered false, non-zero values are
 *  considered true.
 *
 *  If the value is a non-empty string, a true is returned, false otherwise.  If the value is a
 *  number and non-zero, a true is returned, false otherwise.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that is out of bounds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickGetBool
(
    const char* path,
        ///< [IN]
        ///< Path to the value to write.

    bool* valuePtr
        ///< [OUT]
        ///< The value to read.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Write a boolean value to the configuration tree.
 *
 *  @return - LE_OK            - Commit was completed successfuly.
 *          - LE_BAD_PARAMETER - Attempted an invalid path for this request.
 *          - LE_NOT_PERMITTED - Attempted a path that is out of bounds.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_QuickSetBool
(
    const char* path,
        ///< [IN]
        ///< Path to the value to write.

    bool value
        ///< [IN]
        ///< Value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Delete the leaf or stem specified by the path. If the node doesn't exist, nothing happens.  All
 *  child nodes are also deleted.
 *
 *  If the path is empty, the iterator's current node is deleted.
 *
 *  Only valid during a write transaction.
 *
 *  @note If the iterator is invalid, or it's not writeable, this request will be ignored.
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_DeleteNode
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path
        ///< [IN]
        ///< Absolute or relative path to the node to delete.  If
        ///<   Absolute path is given, it is rooted off of the
        ///<   user's root node.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Check if the given node is empty.  A node is also considered empty if it doesn't yet exist.
 *
 *  If the path is empty, the iterator's current node is queried for emptiness.
 *
 *  Valid for both read and write transactions.
 */
//--------------------------------------------------------------------------------------------------
bool le_cfg_IsEmpty
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path
        ///< [IN]
        ///< Absolute or relative path to read from.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Clear out the leaf's value. If it doesn't exist it will be created, but won't have a value.
 *
 *  If the path is empty, the iterator's current node will be cleared.
 *
 *  Only valid during a write transaction.
 *
 *  @note If the iterator is invalid, or it's not writeable, this request will be ignored.
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_SetEmpty
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path
        ///< [IN]
        ///< Absolute or relative path to read from.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Read a string value from the configuration tree.  If the stored value is not a string, the value
 *  will be converted into a string.
 *
 *  If the value is a number, a string with that number is returned.  If the value is empty, or
 *  the iterator is invalid, an empty string is returned.  If the value is boolean, the string,
 *  "true" or "false" is returned.
 *
 *  Valid for both read and write transactions.
 *
 *  If the path is empty, the iterator's current node will be read.
 *
 *  If the iterator is invalid, an empty string is returned.
 *
 *  @return - LE_OK       - Write was completed successfuly.
 *          - LE_OVERFLOW - Supplied string buffer was not large enough to hold the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_cfg_GetString
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path,
        ///< [IN]
        ///< Absolute or relative path to read from.

    char* value,
        ///< [OUT]
        ///< Buffer to write the value into.

    size_t valueNumElements
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 *  Write a string value to the configuration tree.  If the iterator is invalid, the write
 *  request is ignored.
 *
 *  Only valid during a write transaction.
 *
 *  If the path is empty, the iterator's current node will be set.
 *
 *  @note If the iterator is invalid, or it's not writeable, this request will be ignored.
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_SetString
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path,
        ///< [IN]
        ///< Full or relative path to the value to write.

    const char* value
        ///< [IN]
        ///< Value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Read a signed integer value from the configuration tree. If the underlying value is not an
 *  integer, it will be converted.
 *
 *  If the value is a string, then 0 is returned. If the value is a float, a truncated int is
 *  returned. If the value is a bool, a 0 or a 1 is returned.
 *
 *  Valid for both read and write transactions.
 *
 *  If the path is empty, the iterator's current node will be read.
 *
 *  If the iterator is invalid, or the value is empty, a 0 is returned.
 */
//--------------------------------------------------------------------------------------------------
int32_t le_cfg_GetInt
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path
        ///< [IN]
        ///< Full or relative path to the value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Write a signed integer value to the configuration tree.  If the iterator is invalid, the
 *  write request is ignored.
 *
 *  Only valid during a write transaction.
 *
 *  If the path is empty, the iterator's current node will be set.
 *
 *  @note If the iterator is invalid, or it's not writeable, this request will be ignored.
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_SetInt
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path,
        ///< [IN]
        ///< Full or relative path to the value to write.

    int32_t value
        ///< [IN]
        ///< Value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Read a 64-bit floating point value from the configuration tree.  If the underlying value is not
 *  a float, it will be converted.
 *
 *  If the value is an int, then a straight conversion will be performed.  Bool values will be read
 *  as either a floating point, 0 or 1.  Strings and empty values are read as 0.0.
 *
 *  If the path is empty, the iterator's current node will be read.
 *
 *  If the iterator is invalid, or the value is empty, a 0.0 is returned.
 */
//--------------------------------------------------------------------------------------------------
double le_cfg_GetFloat
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path
        ///< [IN]
        ///< Full or relative path to the value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Write a 64-bit floating point value to the configuration tree.  If the iterator is invalid,
 *  the write request is ignored.
 *
 *  Only valid during a write transaction.
 *
 *  If the path is empty, the iterator's current node will be set.
 *
 *  @note If the iterator is invalid, or it's not writeable, this request will be ignored.
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_SetFloat
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path,
        ///< [IN]
        ///< Full or relative path to the value to write.

    double value
        ///< [IN]
        ///< Value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Read a value from the tree as a boolean.  Empty values are considered false, non-zero values are
 *  considered true.
 *
 *  If the value is a non-empty string, a true is returned, false otherwise. If the value is a
 *  number and non-zero, a true is returned, false otherwise.
 *
 *  Valid for both read and write transactions.
 *
 *  If the path is empty, then the iterator's current node will be read.
 *
 *  If the iterator is invalid, or the value is empty,  a false is returned.
 */
//--------------------------------------------------------------------------------------------------
bool le_cfg_GetBool
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path
        ///< [IN]
        ///< Full or relative path to the value to write.
);

//--------------------------------------------------------------------------------------------------
/**
 *  Write a boolean value to the configuration tree.  If the iterator is invalid, the write
 *  request is ignored.
 *
 *  Only valid during a write transaction.
 *
 *  If the path is empty, then the iterator's current node will be set.
 *
 *  @note If the iterator is invalid, or it's not writeable, this request will be ignored.
 */
//--------------------------------------------------------------------------------------------------
void le_cfg_SetBool
(
    le_cfg_iteratorRef_t iteratorRef,
        ///< [IN]
        ///< Iterator to use as a basis for the transaction.

    const char* path,
        ///< [IN]
        ///< Full or relative path to the value to write.

    bool value
        ///< [IN]
        ///< Value to write.
);


#endif // LE_CFG_INTERFACE_H_INCLUDE_GUARD
