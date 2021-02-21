#ifndef _CMS_LOGGING_H
#define _CMS_LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* logDebug...
*
* Print a message to the debug log.
*******************************************************************************/
void logDebug(char *msg, ...);

/*******************************************************************************
* logInfo...
*
* Print a message to the info log.
*******************************************************************************/
void logInfo(char *msg, ...);

/*******************************************************************************
* logError...
*
* Print a message to the error log.
*******************************************************************************/
void logError(char *msg, ...);

#ifdef __cplusplus
}
#endif


#endif
