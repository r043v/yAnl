/***************************************************************************************
*   Project Informations:
*      Project:    GP32 plugin system
*      Version:    1.00
*      Plateforme: GP 32
*      Date:       120/05/2004
*
*   File informations:
*      Name:       libplg.h
*      Description:Functions for plugins use
*
*   Authors informations:
*      Author:     DOUALOT Nicolas
*      E-Mail:     slubman.dndd@linux-fan.com
*      site:       http://www.slubman.linuxfan.com
*
*      Author:     SAAS Alain
*      E-Mail:     al1saas@yahoo.fr
*      site:       http://www.egloo.fr.st
***************************************************************************************/
/*! @file libplg.h
 *  @author DOUALOT Nicolas (mail:slubman.dndd@laposte.net , web:http://www.slubman.linux-fan.com)
 *  @author SAAS Alain (mail:al1saas@yahoo.fr , web:http://www.egloo.fr.st)
 *  @brief Allow use of plugin
 *
 *  Let's you get informations and launch plugins in your prgoram.
 */

 
#ifndef _PLG_H_
#define _PLG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "plgtypes.h"

/*!
 * Init library
 *
 * This function scan plugin's diretory and build a "database".
 *
 * @return Number of plugins found.
 *
 * @see PlgFree().
 * @warning Must be call BEFORE any use of other function in library.
 */
extern unsigned long PlgInit(void);

/*!
 * Free memory allocate by library.
 *
 * This function scan plugin's diretory and build a "database".
 *
 * @see PlgInit().
 */
extern void PlgFree(void);


/*!
 * Give nummber of valid plugins.
 *
 * @return Number of plugins found.
 *
 * @see PlgInit().
 */
extern unsigned long PlgGetNum(void);

/*!
 * Search PLGs for a given extension.
 *
 * @param sExt       String with the file extension.
 * @param num        Pointer to an an unsigned long to store number of PLG in array.
 *
 * @return An array of PLGINFO with all matched PLGs.
 * @return NULL otherwise.
 *
 * @warning You MUST free the array with gm_free.
 */
extern PLGINFO *PlgSearchByExt(const char *sExt, unsigned long *num);

/*!
 * Search PLG by name.
 *
 * @param     sName  String with the searched plugin name.
 *
 * @return PLG found: a pointer to PLGINFO of PLG name sName.
 * @return NULL otherwise.
 *
 * @warning You MUST free the array with gm_free.
 */
extern PLGINFO *PlgSearchPlgByName(const char *sName);

/*!
 * Search PLG by filename.
 *
 * @param     sName  String with the searched plugin filename.
 *
 * @return PLG found: a pointer to PLGINFO of PLG filename sName.
 * @return NULL otherwise.
 *
 * @warning You MUST free the array with gm_free.
 */
extern PLGINFO *PlgSearchByFileName(const char *sName);

/*!
 * Launch a PLG.
 *
 * @param pPlg       Pointer to PLGINFO of plugin you want to launch
 * @param sFilePath  Full path of file wich must be used by the plugin.
 * @param sExt       Extension of file wich must be used by the plugin.
 * @param vSaveData  Data you want to retrieve if the plugin exit "normally".
 */
extern void PlgLaunch(PLGINFO * pPlg, char *sFilePath, char *sExt, void *vSaveData);


extern PLGINFO *pPlgList;
extern unsigned long dwPlgNum;
#ifdef __cplusplus
}
#endif

#endif

/*!
 * @example SampleSearch.c
 * An example on how to search for plugins.
 * 
 * This example is part of my firmware source code.
 */
