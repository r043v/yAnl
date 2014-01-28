/***************************************************************************************
*   Project Informations:
*      Project:    GP32 plugin system
*      Version:    1.00
*      Plateforme: GP 32
*      Date:       120/05/2004
*
*   File informations:
*      Name:       plgtypes.h
*      Description:Plugin types to used
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
/*! @file plgtypes.h
 *  @author DOUALOT Nicolas (mail:slubman.dndd@laposte.net , web:http://www.slubman.linux-fan.com)
 *  @author SAAS Alain (mail:al1saas@yahoo.fr , web:http://www.egloo.fr.st)
 *  @brief Types use by plugin libraries.
 *
 *  Let's you make plugins.
 */

 
#ifndef _PLGTYPES_H_
#define _PLGTYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

//! Firts part of PLG TAG.
#define PLG_TAG "PLG"

//! Major version.
#define PLG_VERSION 0

//! Directory where plugins are.
#define PLG_BASE_PATH "gp:\\gpsys\\plugins\\"

//! Unused
#define PLG_PARAM_PATH PLG_BASE_PATH

//! Unused
#define PLG_BINARY_PATH PLG_BASE_PATH

//! Name of the file to trasnmit parameters to plugin
#define PLG_PARAM_FILE "gp:\\gpsys\\param.tmp"

//! Name of the plugin write on normal exit (no power down)
#define PLG_RETURN_FILE "gp:\\gpsys\\exit.tmp"

//! Size of data the caller can pass to retrieve on plugin exit
#define PLG_TRANSMIT_BLOCK_SIZE 4 * 1024

/*! 
 * @brief PLG header type
 *
 * Presentation part
 *    0~3            : Tag("PLG") & Major version
 *    5~35           : Plugin name
 *    36~67          : Author name
 *    68~1091        : Plugin icon (same as FXE by default)
 *
 * Technical part
 *    1092           : is the plg is the plugin itself
 *    1093           : number of extension recognized by plugin ("n" is this number)
 *    1094~1094+3*n  : extensions list
 */
typedef struct plginfo
{
	// SMC matter
	char sPlgFileName[13];                 //!< PLG filename.

	// Presentation part
	char sTag[4];                          //!< PLG TAG must be "PLG\0".
	char sPlgName[33];                     //!< PLG name.
	char sAuthorName[33];                  //!< PLG Athor name.
	char sIcon[1024];                      //!< PLG icon.

	// Technical part
	char bIsSelf;                          //!< Is the PLG self contened or liked to a program ?
	char bNbExt;                           //!< Number of filetype handled by the plugin.
	char *psExt;                           //!< Null terminated string with all extension handled by the plugin.
}
PLGINFO;

/*! 
 * @brief PLGD header type
 */
typedef struct plgdinfo
{
// Public variables
	// SMC matter
	char sFileName[MAX_PATH_NAME_LEN];     //!< File to use in the plgin
	char sExt[4];                          //!< Extension of file to use in the plugin
	char sPlgName[33];                     //!< Name of the PLG

// Private variables
	char sCallProg[MAX_PATH_NAME_LEN];     //!< Internal purpose
	unsigned char bTransData[PLG_TRANSMIT_BLOCK_SIZE]; //!< Internal purpose
}
PLGDINFO;

#ifdef __cplusplus
}
#endif

#endif
