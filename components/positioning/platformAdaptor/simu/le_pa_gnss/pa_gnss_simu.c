/**
 * @file pa_gnss_simu.c
 *
 * Simulation implementation of @ref c_pa_gnss API.
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 */

#include <pa_gnss.h>

static le_event_Id_t        GnssEventId;

#ifdef GNSS_STUB_FAKE_POSITION

const pa_Gnss_Position_t StubPosition = {
        .latitude = +48858300,
        .longitude = +2294400,
        .altitude = 0,
        .hSpeed = 0,
        .track = 0,
        .hdop = 0,
        .vdop = 0,
        .timeInfo = {
                .hours = 0,
                .minutes = 0,
                .seconds = 0,
                .milliseconds = 0,
                .year = 2013,
                .month = 1,
                .day = 1,
        },
};

#endif /* GNSS_STUB_FAKE_POSITION */

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA gnss Module.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Init
(
    void
)
{
    GnssEventId = le_event_CreateIdWithRefCounting("gnssEventId");

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to release the PA gnss Module.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Release
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to start the gnss acquisition.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Start
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to stop the gnss acquisition.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Stop
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the rate of gps fix reception
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetAcquisitionRate
(
    uint32_t rate     ///< [IN] rate in seconds
)
{


    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register an handler for gnss position data notifications.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_gnss_AddPositionDataHandler
(
    pa_gnss_PositionDataHandlerFunc_t handler ///< [IN] The handler function.
)
{
    LE_FATAL_IF((handler==NULL),"gnss module cannot set handler");

    le_event_HandlerRef_t newHandlerPtr = le_event_AddHandler(
                                                            "gpsInformationHandler",
                                                            GnssEventId,
                                                            (le_event_HandlerFunc_t) handler);

    return newHandlerPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to remove a handler for gnss position data notifications.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
void pa_gnss_RemovePositionDataHandler
(
    le_event_HandlerRef_t    handlerRef ///< [IN] The handler reference.
)
{
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the location's data.
 *
 * @return LE_FAULT         The function cannot get internal position information
 * @return LE_NOT_POSSIBLE  The function cannot convert internal position information
 * @return LE_OK            The function succeeded.
 *
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the
 *       function will not return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetLastPositionData
(
    pa_Gnss_Position_Ref_t  positionRef   ///< [OUT] Reference to a position struct
)
{

#ifdef GNSS_STUB_FAKE_POSITION

    if (positionRef == NULL)
    {
        LE_WARN("Parameter are not valid");
        return LE_FAULT;
    }

    memcpy(positionRef, &StubPosition, sizeof(pa_Gnss_Position_t));
    return LE_OK;

#else /* GNSS_STUB_FAKE_POSITION */

    return LE_FAULT;

#endif /* GNSS_STUB_FAKE_POSITION */
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to load an 'Extended Ephemeris' file into the GNSS device.
 *
 * @return LE_FAULT         The function failed to load the 'Extended Ephemeris' file.
 * @return LE_NOT_FOUND     The file path does not exist
 * @return LE_OK            The function succeeded.
 *
 * @TODO    implementation
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_LoadExtendedEphemerisFile
(
    int32_t       fd      ///< [IN] extended ephemeris file descriptor
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the validity of the last injected Extended Ephemeris.
 *
 * @return LE_FAULT         The function failed to get the validity
 * @return LE_OK            The function succeeded.
 *
 * @TODO    implementation
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetExtendedEphemerisValidityTimes
(
    le_clk_Time_t *startTimePtr,    ///< [OUT] Start time
    le_clk_Time_t *stopTimePtr      ///< [OUT] Stop time
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to restart the GNSS device.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_ForceRestart
(
    pa_gnss_Restart_t  restartType ///< [IN] type of restart
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Component initialization function.
 *
 * This is not used because it will be called immediately at process start by the application
 * framework, and we want to wait until we can confirm that GNSS is available before starting
 * the platform adapter.
 *
 * See pa_gnss_Init().
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
}

