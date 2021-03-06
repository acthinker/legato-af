 /**
  * This module implements the le_mrc's unit tests.
  *
  *
  * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
  *
  */

/*
 * Module need to be registered on the SIM home PLMN.
 *
 * Instruction to execute this test
 * 1) install application test.
 * 2) Start log trace 'logread -f | grep 'INFO'
 * 3) Start application 'app start mrcTest'
 * 4) check trace for the following INFO  trace:
 *     "======== Test MRC Modem Services implementation Test SUCCESS ========"
 */

#include "legato.h"
#include <interfaces.h>

/*
 * Flag set to 1 to perform the Radio Power test On AR7 platform
 * This test was previously deactivated, WP7 doesn't support it.
 */
#if AR7_DETECTED
#define TEST_MRC_POWER 1
#endif


//--------------------------------------------------------------------------------------------------
/**
 * Handler function for RAT change Notifications.
 *
 */
//--------------------------------------------------------------------------------------------------
static void TestRatHandler
(
    le_mrc_Rat_t rat,
    void*        contextPtr
)
{
    LE_INFO("New RAT: %d", rat);

    if (rat == LE_MRC_RAT_CDMA)
    {
        LE_INFO("Check RatHandler passed, RAT is LE_MRC_RAT_CDMA.");
    }
    else if (rat == LE_MRC_RAT_GSM)
    {
        LE_INFO("Check RatHandler passed, RAT is LE_MRC_RAT_GSM.");
    }
    else if (rat == LE_MRC_RAT_UMTS)
    {
        LE_INFO("Check RatHandler passed, RAT is LE_MRC_RAT_UMTS.");
    }
    else if (rat == LE_MRC_RAT_LTE)
    {
        LE_INFO("Check RatHandler passed, RAT is LE_MRC_RAT_LTE.");
    }
    else
    {
        LE_INFO("Check RatHandler failed, bad RAT.");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler function for Network Registration Notifications.
 *
 */
//--------------------------------------------------------------------------------------------------
static void TestNetRegHandler
(
    le_mrc_NetRegState_t state,
    void*                contextPtr
)
{
    LE_INFO("New Network Registration state: %d", state);

    if (state == LE_MRC_REG_NONE)
    {
        LE_INFO("Check NetRegHandler passed, state is LE_MRC_REG_NONE.");
    }
    else if (state == LE_MRC_REG_HOME)
    {
        LE_INFO("Check NetRegHandler passed, state is LE_MRC_REG_HOME.");
    }
    else if (state == LE_MRC_REG_SEARCHING)
    {
        LE_INFO("Check NetRegHandler passed, state is LE_MRC_REG_SEARCHING.");
    }
    else if (state == LE_MRC_REG_DENIED)
    {
        LE_INFO("Check NetRegHandler passed, state is LE_MRC_REG_DENIED.");
    }
    else if (state == LE_MRC_REG_ROAMING)
    {
        LE_INFO("Check NetRegHandler passed, state is LE_MRC_REG_ROAMING.");
    }
    else if (state == LE_MRC_REG_UNKNOWN)
    {
        LE_INFO("Check NetRegHandler passed, state is LE_MRC_REG_UNKNOWN.");
    }
    else
    {
        LE_INFO("Check NetRegHandler failed, bad Network Registration state.");
    }
}



//--------------------------------------------------------------------------------------------------
//                                       Test Functions
//--------------------------------------------------------------------------------------------------

#if TEST_MRC_POWER
//--------------------------------------------------------------------------------------------------
/**
 * Test: Radio Power Management.
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_Power()
{
    le_result_t   res;
    le_onoff_t    onoff;

    res = le_mrc_SetRadioPower(LE_OFF);
    LE_ASSERT(res == LE_OK);

    sleep(3);

    res = le_mrc_GetRadioPower(&onoff);
    LE_ASSERT(res == LE_OK);
    LE_ASSERT(onoff == LE_OFF);

    res = le_mrc_SetRadioPower(LE_ON);
    LE_ASSERT(res == LE_OK);

    sleep(3);

    res = le_mrc_GetRadioPower(&onoff);
    LE_ASSERT(res == LE_OK);
    LE_ASSERT(onoff == LE_ON);
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Test: Radio Access Technology.
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_GetRat()
{
    le_result_t   res;
    le_mrc_Rat_t  rat;

    res = le_mrc_GetRadioAccessTechInUse(&rat);
    LE_ASSERT(res == LE_OK);
    if (res == LE_OK)
    {
        LE_ASSERT((rat>=LE_MRC_RAT_UNKNOWN) && (rat<=LE_MRC_RAT_LTE));
    }
    LE_INFO("le_mrc_GetRadioAccessTechInUse return rat %d",rat);
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Network Registration State + Signal Quality.
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_GetStateAndQual()
{
    le_result_t           res;
    le_mrc_NetRegState_t  state;
    uint32_t              quality;

    res = le_mrc_GetNetRegState(&state);
    LE_ASSERT(res == LE_OK);
    if (res == LE_OK)
    {
        LE_ASSERT((state>=LE_MRC_REG_NONE) && (state<=LE_MRC_REG_UNKNOWN));
    }

    res = le_mrc_GetSignalQual(&quality);
    LE_ASSERT(res == LE_OK);
    if (res == LE_OK)
    {
        LE_ASSERT((quality>=0) && (quality<=5));
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Neighbor Cells Information.
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_GetNeighboringCellsInfo()
{
    le_mrc_NeighborCellsRef_t ngbrRef;
    le_mrc_CellInfoRef_t cellRef;
    uint32_t i = 0;
    uint32_t cid = 0;
    uint32_t lac = 0;
    int32_t rxLevel = 0;

    LE_INFO("Start Testle_mrc_GetNeighborCellsInfo");

    ngbrRef = le_mrc_GetNeighborCellsInfo();
    LE_ASSERT(ngbrRef);

    if (ngbrRef)
    {
        i = 0;

        cellRef = le_mrc_GetFirstNeighborCellInfo(ngbrRef);
        LE_ASSERT(cellRef);
        cid = le_mrc_GetNeighborCellId(cellRef);
        lac = le_mrc_GetNeighborCellLocAreaCode(cellRef);
        rxLevel = le_mrc_GetNeighborCellRxLevel(cellRef);
        LE_INFO("Cell #%d, cid=%d, lac=%d, rxLevel=%d", i, cid, lac, rxLevel);

        while ((cellRef = le_mrc_GetNextNeighborCellInfo(ngbrRef)) != NULL)
        {
            i++;
            cid = le_mrc_GetNeighborCellId(cellRef);
            lac = le_mrc_GetNeighborCellLocAreaCode(cellRef);
            rxLevel = le_mrc_GetNeighborCellRxLevel(cellRef);
            LE_INFO("Cell #%d, cid=%d, lac=%d, rxLevel=%d", i, cid, lac, rxLevel);
        }

        le_mrc_DeleteNeighborCellsInfo(ngbrRef);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Network Registration notification handling.
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_NetRegHdlr()
{
    le_mrc_NetRegStateEventHandlerRef_t testHdlrRef;

    testHdlrRef = le_mrc_AddNetRegStateEventHandler(TestNetRegHandler, NULL);
    LE_ASSERT(testHdlrRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: RAT change handling.
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_RatHdlr()
{
    le_mrc_RatChangeHandlerRef_t testHdlrRef;

    testHdlrRef = le_mrc_AddRatChangeHandler(TestRatHandler, NULL);
    LE_ASSERT(testHdlrRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Get the Current PLMN network.
 * le_mrc_GetCurrentNetworkMccMnc() API test
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_GetCurrentNetworkMccMnc()
{

    le_result_t res;
    char mcc[LE_MRC_MCC_BYTES] = {0}, mnc[LE_MRC_MNC_BYTES] = {0};

    /* Test mccStrNumElements limit */
    res = le_mrc_GetCurrentNetworkMccMnc(mcc, LE_MRC_MCC_BYTES-1, mnc, LE_MRC_MNC_BYTES);
    LE_ASSERT(res == LE_FAULT);

    /* Test mncStrNumElements limit */
    res = le_mrc_GetCurrentNetworkMccMnc(mcc, LE_MRC_MCC_BYTES, mnc, LE_MRC_MCC_BYTES-1);
    LE_ASSERT(res == LE_FAULT);

    res = le_mrc_GetCurrentNetworkMccMnc(mcc, LE_MRC_MCC_BYTES, mnc, LE_MRC_MCC_BYTES);
    LE_ASSERT(res == LE_OK);

    LE_INFO("Plmn MCC.%s MNC.%s",mcc,mnc);
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Get the Current network name.
 * le_mrc_GetCurrentNetworkName() API test
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_GetCurrentNetworkName()
{

    le_result_t res;
    char nameStr[100] = {0};

    res = le_mrc_GetCurrentNetworkName(nameStr, 1);
    LE_ASSERT(res == LE_OVERFLOW);

    res = le_mrc_GetCurrentNetworkName(nameStr, 100);;
    LE_ASSERT(res == LE_OK);

    LE_INFO("Plmn name.%s",nameStr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Current preeferred network operators.
 * Sim used must support preferred PLMNs storage
 *
 * le_mrc_GetPreferredOperatorsList() API test
 * le_mrc_GetFirstPreferredOperator() API test
 * le_mrc_GetPreferredOperatorDetails() API test
 * le_mrc_GetNextPreferredOperator() API test
 * le_mrc_DeletePreferredOperatorsList() API test
 * le_mrc_AddPreferredOperator() API test
 * le_mrc_RemovePreferredOperator() API test
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_PreferredPLMN()
{
    le_result_t res;
    char mccStr[LE_MRC_MCC_BYTES] = {0};
    char mncStr[LE_MRC_MNC_BYTES] = {0};
    le_mrc_PreferredOperatorRef_t optRef = NULL;
    int first = 0;
    int i = 0;
    le_mrc_RatBitMask_t ratMask;

    le_mrc_PreferredOperatorListRef_t prefPlmnList = le_mrc_GetPreferredOperatorsList();

    if (prefPlmnList)
    {
        optRef = le_mrc_GetFirstPreferredOperator(prefPlmnList);
        while (optRef)
        {
            res = le_mrc_GetPreferredOperatorDetails(optRef,
                            mccStr, LE_MRC_MCC_BYTES,
                            mncStr, LE_MRC_MNC_BYTES,
                            &ratMask);

            LE_ASSERT(res == LE_OK);
            LE_INFO("Get_detail Loop(%d) mcc.%s mnc %s, rat.%08X,  GSM %c, LTE %c, UMTS %c",
                first++, mccStr, mncStr, ratMask,
                (ratMask & LE_MRC_BITMASK_RAT_GSM ? 'Y':'N'),
                (ratMask & LE_MRC_BITMASK_RAT_LTE ? 'Y':'N'),
                (ratMask & LE_MRC_BITMASK_RAT_UMTS ? 'Y':'N')
            );
            optRef = le_mrc_GetNextPreferredOperator(prefPlmnList);
        }
        LE_INFO("No more preferred PLMN operator present in the modem List %d Displayed",i);
        le_mrc_DeletePreferredOperatorsList(prefPlmnList);
    }
    else
    {
        LE_ERROR("=== PreferredPLMN Test No Preferred PLMN list present in teh SIM => NA ====");
        return;
    }

    res = le_mrc_AddPreferredOperator("208", "01", 0);
    LE_ASSERT(res == LE_OK);
    res = le_mrc_AddPreferredOperator("208", "10", LE_MRC_BITMASK_RAT_UMTS);
    LE_ASSERT(res == LE_OK);
    res = le_mrc_AddPreferredOperator("311", "070", 0);
    LE_ASSERT(res == LE_OK);
    res = le_mrc_AddPreferredOperator("311", "70", 0);
    LE_ASSERT(res == LE_OK);

    res = le_mrc_RemovePreferredOperator("311", "70");
    LE_ASSERT(res == LE_OK);
    res = le_mrc_RemovePreferredOperator("311", "070");
    LE_ASSERT(res == LE_OK);
    res = le_mrc_RemovePreferredOperator("208", "10");
    LE_ASSERT(res == LE_OK);

    res = le_mrc_RemovePreferredOperator("311", "70");
    LE_ASSERT(res == LE_FAULT);

    prefPlmnList = le_mrc_GetPreferredOperatorsList();
    LE_ASSERT(prefPlmnList != NULL);

    if (prefPlmnList)
    {
        optRef = le_mrc_GetFirstPreferredOperator(prefPlmnList);
        while (optRef)
        {
            res = le_mrc_GetPreferredOperatorDetails(optRef,
                            mccStr, LE_MRC_MCC_BYTES,
                            mncStr, LE_MRC_MNC_BYTES,
                            &ratMask);

            LE_ASSERT(res == LE_OK);
            LE_INFO("Get_detail Loop(%d) mcc.%s mnc %s, rat.%08X,  GSM %c, LTE %c, UMTS %c",
                i++, mccStr, mncStr, ratMask,
                (ratMask & LE_MRC_BITMASK_RAT_GSM ? 'Y':'N'),
                (ratMask & LE_MRC_BITMASK_RAT_LTE ? 'Y':'N'),
                (ratMask & LE_MRC_BITMASK_RAT_UMTS ? 'Y':'N')
            );

            optRef = le_mrc_GetNextPreferredOperator(prefPlmnList);
        }
        LE_INFO("No more preferred PLMN operator present in the modem List %d Displayed, first %d",
            i, first);
        le_mrc_DeletePreferredOperatorsList(prefPlmnList);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Register mode.
 * This test doesn't work in roaming.!!
 *
 * le_mrc_SetAutomaticRegisterMode() API test
 * le_mrc_SetManualRegisterMode() API test
 * le_mrc_GetRegisterMode() API test
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_RegisterMode()
{
    le_result_t res;
    int cmpRes;
    char mccHomeStr[LE_MRC_MCC_BYTES] = {0};
    char mccStr[LE_MRC_MNC_BYTES] = {0};
    char mncHomeStr[LE_MRC_MCC_BYTES] = {0};
    char mncStr[LE_MRC_MNC_BYTES] = {0};
    bool isManualOrigin, isManual;

    // Get the home PLMN to compare results.
    res = le_sim_GetHomeNetworkMccMnc( mccHomeStr, LE_MRC_MCC_BYTES,
                    mncHomeStr, LE_MRC_MNC_BYTES);

    LE_ERROR_IF(res != LE_OK, "Home PLMN can't be retrives for test case");
    LE_ASSERT(res == LE_OK);
    LE_INFO("Home PLMN is mcc.%s mnc.%s", mccHomeStr, mncHomeStr);

    res = le_mrc_GetRegisterMode(&isManualOrigin,
                    mccStr, LE_MRC_MCC_BYTES, mncStr, LE_MRC_MNC_BYTES);
    LE_ASSERT(res == LE_OK);

    if (isManualOrigin == true)
    {
        LE_INFO("le_mrc_GetRegisterMode Manual(Y), mcc.%s mnc.%s",
                        mccStr, mncStr);
    }
    else
    {
        LE_INFO("le_mrc_GetRegisterMode Manual(N)");
    }

    res = le_mrc_SetAutomaticRegisterMode();
    LE_ASSERT(res == LE_OK);

    sleep(5);

    res = le_mrc_GetRegisterMode(&isManual, mccStr, LE_MRC_MCC_BYTES, mncStr, LE_MRC_MNC_BYTES);
    LE_ASSERT(res == LE_OK);
    LE_ASSERT(isManual == false);
    LE_INFO("le_mrc_GetRegisterMode Manual(N)");

    res = le_mrc_SetManualRegisterMode(mccHomeStr, mncHomeStr);
    LE_INFO("le_mrc_SetManualRegisterMode %s,%s return %d",mccHomeStr, mncHomeStr, res);
    LE_ASSERT(res == LE_OK);

    sleep(5);

    res = le_mrc_GetRegisterMode(&isManual, mccStr, LE_MRC_MCC_BYTES, mncStr, LE_MRC_MNC_BYTES);
    LE_ASSERT(res == LE_OK);
    LE_ASSERT(isManual == true);
    cmpRes = strcmp(mccHomeStr, mccStr);
    LE_WARN_IF(cmpRes, "Doesn't match mccHomeStr (%s) mccStr (%s)",mccHomeStr, mccStr)
    LE_ASSERT(cmpRes == 0);
    cmpRes = strcmp(mncHomeStr, mncStr);
    LE_WARN_IF(cmpRes, "Doesn't match mncHomeStr (%s) mncStr (%s)",mncHomeStr, mncStr)
    LE_ASSERT(cmpRes == 0);
    LE_INFO("le_mrc_GetRegisterMode Manual(Y), mcc.%s mnc.%s", mccStr, mncStr);

    res = le_mrc_SetAutomaticRegisterMode();
    LE_ASSERT(res == LE_OK);

    sleep(2);

    res = le_mrc_GetRegisterMode(&isManual, mccStr, LE_MRC_MCC_BYTES, mncStr, LE_MRC_MNC_BYTES);
    LE_ASSERT(res == LE_OK);
    LE_ASSERT(isManual == false);
    LE_INFO("le_mrc_GetRegisterMode Manual(N)");
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: rat preferences mode.
 *
 * le_mrc_GetRatPreferences() API test
 * le_mrc_SetRatPreferences() API test
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_RatPreferences()
{
    le_result_t res;
    le_mrc_RatBitMask_t bitMask = 0;
    le_mrc_RatBitMask_t bitMaskOrigin = 0;

#define  PRINT_RAT(x)   LE_INFO("Rat preferences %04X=> CDMA.%c GSM.%c LTE.%c UMTS.%c", x,\
    ( (x & LE_MRC_BITMASK_RAT_CDMA) ? 'Y' : 'N'),\
    ( (x & LE_MRC_BITMASK_RAT_GSM) ? 'Y' : 'N'),\
    ( (x & LE_MRC_BITMASK_RAT_LTE) ? 'Y' : 'N'),\
    ( (x & LE_MRC_BITMASK_RAT_UMTS) ? 'Y' : 'N') );

    // Get the current rat preference.
    res = le_mrc_GetRatPreferences(&bitMaskOrigin);
    LE_ASSERT(res == LE_OK);
    PRINT_RAT(bitMaskOrigin);

    if (bitMaskOrigin & LE_MRC_BITMASK_RAT_LTE)
    {
        res = le_mrc_SetRatPreferences(LE_MRC_BITMASK_RAT_LTE);
        LE_ASSERT(res == LE_OK);
        res = le_mrc_GetRatPreferences(&bitMask);
        PRINT_RAT(bitMask);
        LE_ASSERT(res == LE_OK);
        LE_ASSERT(bitMask == LE_MRC_BITMASK_RAT_LTE);
    }

    if (bitMaskOrigin & LE_MRC_BITMASK_RAT_GSM)
    {
        res = le_mrc_SetRatPreferences(LE_MRC_BITMASK_RAT_GSM);
        LE_ASSERT(res == LE_OK);
        res = le_mrc_GetRatPreferences(&bitMask);
        PRINT_RAT(bitMask);
        LE_ASSERT(res == LE_OK);
        LE_ASSERT(bitMask == LE_MRC_BITMASK_RAT_GSM);
    }

    if (bitMaskOrigin & LE_MRC_BITMASK_RAT_UMTS)
    {
        res = le_mrc_SetRatPreferences(LE_MRC_BITMASK_RAT_UMTS);
        LE_ASSERT(res == LE_OK);
        res = le_mrc_GetRatPreferences(&bitMask);
        PRINT_RAT(bitMask);
        LE_ASSERT(res == LE_OK);
        LE_ASSERT(bitMask == LE_MRC_BITMASK_RAT_UMTS);
    }

    res = le_mrc_SetRatPreferences(LE_MRC_BITMASK_RAT_ALL);
    LE_ASSERT(res == LE_OK);
    res = le_mrc_GetRatPreferences(&bitMask);
    PRINT_RAT(bitMask);
    LE_ASSERT(res == LE_OK);

    if (bitMaskOrigin & LE_MRC_BITMASK_RAT_CDMA)
    {
        res = le_mrc_SetRatPreferences(LE_MRC_BITMASK_RAT_CDMA);
        LE_ASSERT(res == LE_OK);
        res = le_mrc_GetRatPreferences(&bitMask);
        PRINT_RAT(bitMask);
        LE_ASSERT(res == LE_OK);
        LE_ASSERT(bitMask == LE_MRC_BITMASK_RAT_CDMA);
    }

    res = le_mrc_SetRatPreferences(bitMaskOrigin);
    LE_ASSERT(res == LE_OK);
    res = le_mrc_GetRatPreferences(&bitMask);
    PRINT_RAT(bitMask);
    LE_ASSERT(res == LE_OK);
    LE_ASSERT(bitMask == bitMaskOrigin);
}

//--------------------------------------------------------------------------------------------------
/**
 * Read scan information
 *
 */
//--------------------------------------------------------------------------------------------------
static void ReadScanInfo
(
    le_mrc_ScanInformationRef_t     scanInfoRef
)
{
    le_mrc_Rat_t rat;
    bool boolTest;
    le_result_t res;
    char mcc[LE_MRC_MCC_BYTES] = {0};
    char mnc[LE_MRC_MNC_BYTES] = {0};
    char nameStr[100] = {0};

    res = le_mrc_GetCellularNetworkMccMnc(scanInfoRef, mcc, LE_MRC_MCC_BYTES, mnc, LE_MRC_MCC_BYTES);
    LE_ASSERT(res == LE_OK);

    res = le_mrc_GetCellularNetworkName(scanInfoRef, nameStr, 1);
    LE_ASSERT(res == LE_OVERFLOW);
    res = le_mrc_GetCellularNetworkName(scanInfoRef, nameStr, 100);;
    LE_ASSERT(res == LE_OK);
    LE_INFO("1st cellular network name.%s", nameStr);

    rat = le_mrc_GetCellularNetworkRat(scanInfoRef);
    LE_ASSERT((rat>=LE_MRC_RAT_UNKNOWN) && (rat<=LE_MRC_RAT_LTE));
    LE_INFO("le_mrc_GetCellularNetworkRat returns rat %d", rat);

    boolTest = le_mrc_IsCellularNetworkInUse(scanInfoRef);
    LE_INFO("IsCellularNetworkInUse is %s", ( (boolTest) ? "true" : "false"));

    boolTest = le_mrc_IsCellularNetworkAvailable(scanInfoRef);
    LE_INFO("le_mrc_IsCellularNetworkAvailable is %s", ( (boolTest) ? "true" : "false"));

    boolTest = le_mrc_IsCellularNetworkHome(scanInfoRef);
    LE_INFO("le_mrc_IsCellularNetworkHome is %s", ( (boolTest) ? "true" : "false"));

    boolTest = le_mrc_IsCellularNetworkForbidden(scanInfoRef);
    LE_INFO("le_mrc_IsCellularNetworkForbidden is %s", ( (boolTest) ? "true" : "false"));
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Cellular Network Scan.
 *
 * le_mrc_PerformCellularNetworkScan() API test
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_PerformCellularNetworkScan()
{
    le_result_t res;
    le_mrc_RatBitMask_t bitMaskOrigin = 0;
    le_mrc_ScanInformationListRef_t scanInfoListRef;
    le_mrc_ScanInformationRef_t     scanInfoRef;

    // Get the current rat preference.
    res = le_mrc_GetRatPreferences(&bitMaskOrigin);
    LE_ASSERT(res == LE_OK);

    if (bitMaskOrigin & LE_MRC_BITMASK_RAT_GSM)
    {
        LE_INFO("Perform scan on GSM");
        scanInfoListRef = le_mrc_PerformCellularNetworkScan(LE_MRC_BITMASK_RAT_GSM);
    }
    else if (bitMaskOrigin & LE_MRC_BITMASK_RAT_UMTS)
    {
        LE_INFO("Perform scan on UMTS");
        scanInfoListRef = le_mrc_PerformCellularNetworkScan(LE_MRC_BITMASK_RAT_UMTS);
    }
    LE_ASSERT(scanInfoListRef != NULL);

    scanInfoRef = le_mrc_GetFirstCellularNetworkScan(scanInfoListRef);
    LE_ASSERT(scanInfoRef != NULL);
    ReadScanInfo(scanInfoRef);

    while ((scanInfoRef = le_mrc_GetNextCellularNetworkScan(scanInfoListRef)) != NULL)
    {
        ReadScanInfo(scanInfoRef);
    }

    le_mrc_DeleteCellularNetworkScan(scanInfoListRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: 2G/3G band Preferences mode.
 *
 * le_mrc_GetBandPreferences() API test
 * le_mrc_SetBandPreferences() API test
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_BandPreferences()
{
    le_result_t res;
    le_mrc_BandBitMask_t bandMask = 0;
    le_mrc_BandBitMask_t bandMaskOrigin = 0;

#define  PRINT_BAND(x)   LE_INFO("Band preferences 0x%016"PRIX64, x);

    // Get the current rat preference.
    res = le_mrc_GetBandPreferences(&bandMaskOrigin);
    LE_ASSERT(res == LE_OK);
    PRINT_BAND(bandMaskOrigin);

    LE_WARN_IF(( bandMaskOrigin == 0), "le_mrc_GetBandPreferences bandMaskOrigin = 0");

    if (bandMaskOrigin != 0)
    {
        res = le_mrc_SetBandPreferences(bandMaskOrigin);
        LE_ASSERT(res == LE_OK);

        // Get the current rat preference.
        res = le_mrc_GetBandPreferences(&bandMask);
        PRINT_BAND(bandMask);
        LE_ASSERT(res == LE_OK);
        LE_ASSERT(bandMask == bandMaskOrigin);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Test: Lte band Preferences mode.
 *
 * le_mrc_GetLteBandPreferences() API test
 * le_mrc_SetLteBandPreferences() API test
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_LteBandPreferences()
{
    le_result_t res;
    le_mrc_LteBandBitMask_t bandMask = 0;
    le_mrc_LteBandBitMask_t bandMaskOrigin = 0;

#define  PRINT_BANDLTE(x)   LE_INFO("LTE Band MRC preferences 0x%016X", x);

    // Get the current rat preference.
    res = le_mrc_GetLteBandPreferences(&bandMaskOrigin);
    LE_ASSERT(res == LE_OK);
    PRINT_BANDLTE(bandMaskOrigin);

    LE_WARN_IF(( bandMaskOrigin == 0), "Testle_mrc_LteBandPreferences bandMaskOrigin = 0");

    if (bandMaskOrigin != 0)
    {
        res = le_mrc_SetLteBandPreferences(bandMaskOrigin);
        LE_ASSERT(res == LE_OK);

        // Get the current rat preference.
        res = le_mrc_GetLteBandPreferences(&bandMask);
        PRINT_BANDLTE(bandMask);
        LE_ASSERT(res == LE_OK);
        LE_ASSERT(bandMask == bandMaskOrigin);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Test: TD-SCDMA band Preferences mode.
 *
 * le_mrc_GetTdScdmaBandPreferences() API test
 * le_mrc_SetTdScdmaBandPreferences() API test
 *
 */
//--------------------------------------------------------------------------------------------------
static void Testle_mrc_TdScdmaBandPreferences()
{
    le_result_t res;
    le_mrc_TdScdmaBandBitMask_t bandMask = 0;
    le_mrc_TdScdmaBandBitMask_t bandMaskOrigin = 0;

#define  PRINT_BANDCDMA(x)   LE_INFO("TD-SCDMA Band preferences 0x%016X", x);

    // Get the current rat preference.
    res = le_mrc_GetTdScdmaBandPreferences(&bandMaskOrigin);
    LE_ASSERT(res == LE_OK);
    PRINT_BANDCDMA(bandMaskOrigin);

    LE_WARN_IF(( bandMaskOrigin == 0), "le_mrc_GetTdScdmaBandPreferences bandMaskOrigin = 0");

    if (bandMaskOrigin != 0)
    {
        res = le_mrc_SetTdScdmaBandPreferences(bandMaskOrigin);
        LE_ASSERT(res == LE_OK);

        // Get the current rat preference.
        res = le_mrc_GetTdScdmaBandPreferences(&bandMask);
        LE_ASSERT(res == LE_OK);
        PRINT_BANDCDMA(bandMask);
        LE_ASSERT(bandMask == bandMaskOrigin);
    }
}


COMPONENT_INIT
{
    LE_INFO("======== Start MRC Modem Services implementation Test========");

    LE_INFO("======== GetStateAndQual Test ========");
    Testle_mrc_GetStateAndQual();
    LE_INFO("======== GetStateAndQual Test PASSED ========");

    LE_INFO("======== GetRat Test ========");
    Testle_mrc_GetRat();
    LE_INFO("======== GetRat Test PASSED ========");

    LE_INFO("======== GetNeighboringCellsInfo Test ========");
    Testle_mrc_GetNeighboringCellsInfo();
    LE_INFO("======== GetNeighboringCellsInfo Test PASSED ========");

    LE_INFO("======== NetRegHdlr Test ========");
    Testle_mrc_NetRegHdlr();
    LE_INFO("======== NetRegHdlr Test PASSED ========");

    LE_INFO("======== RatHdlr Test ========");
    Testle_mrc_RatHdlr();
    LE_INFO("======== RatHdlr Test PASSED ========");

    LE_INFO("======== GetCurrentNetworkMccMnc Test ========");
    Testle_mrc_GetCurrentNetworkMccMnc();
    LE_INFO("======== GetCurrentNetworkMccMnc Test PASSED ========");

    LE_INFO("======== GetCurrentNetworkName Test ========");
    Testle_mrc_GetCurrentNetworkName();
    LE_INFO("======== GetCurrentNetworkName Test PASSED ========");

    LE_INFO("======== PreferredPLMN Test ========");
    Testle_mrc_PreferredPLMN();
    LE_INFO("======== PreferredPLMN Test PASSED ========");

    LE_INFO("======== RegisterMode Test ========");
    Testle_mrc_RegisterMode();
    LE_INFO("======== RegisterMode Test PASSED ========");

    LE_INFO("======== RatPreferences Test ========");
    Testle_mrc_RatPreferences();
    LE_INFO("======== RatPreferences Test PASSED ========");

    LE_INFO("======== PerformCellularNetworkScan Test ========");
    Testle_mrc_PerformCellularNetworkScan();
    LE_INFO("======== PerformCellularNetworkScan Test PASSED ========");

    LE_INFO("======== BandPreferences Test ========");
    Testle_mrc_BandPreferences();
    LE_INFO("======== BandPreferences Test PASSED ========");

    LE_INFO("======== BandLtePreferences Test ========");
    Testle_mrc_LteBandPreferences();
    LE_INFO("======== BandLtePreferences Test PASSED ========");

    LE_INFO("======== BandTdScdmaPreferences Test ========");
    Testle_mrc_TdScdmaBandPreferences();
    LE_INFO("======== BandTdScdmaPreferences Test PASSED ========");

#if TEST_MRC_POWER
    LE_INFO("======== Power Test ========");
    Testle_mrc_Power();
    LE_INFO("======== Power Test PASSED ========");
#endif

    LE_INFO("======== Test MRC Modem Services implementation Test SUCCESS ========");
    exit(EXIT_SUCCESS);
}


