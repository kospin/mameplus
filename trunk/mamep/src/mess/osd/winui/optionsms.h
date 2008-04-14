#ifndef OPTIONSMS_H
#define OPTIONSMS_H

#include "mame.h"
#include "image.h"
#include "options.h"

enum
{
	MESS_COLUMN_IMAGES,
	MESS_COLUMN_GOODNAME,
	MESS_COLUMN_MANUFACTURER,
	MESS_COLUMN_YEAR,
	MESS_COLUMN_PLAYABLE,
	MESS_COLUMN_CRC,
	MESS_COLUMN_SHA1,
	MESS_COLUMN_MD5,
	MESS_COLUMN_MAX
};

void MessSetupSettings(core_options *settings);
void MessSetupGameOptions(core_options *opts, int driver_index);
void MessSetupGameVariables(core_options *settings, int driver_index);

void SetMessColumnWidths(int widths[]);
void GetMessColumnWidths(int widths[]);

void SetMessColumnOrder(int order[]);
void GetMessColumnOrder(int order[]);

void SetMessColumnShown(int shown[]);
void GetMessColumnShown(int shown[]);

void SetMessSortColumn(int column);
int  GetMessSortColumn(void);

void SetMessSortReverse(BOOL reverse);
BOOL GetMessSortReverse(void);

const WCHAR* GetSoftwareDirs(void);
void  SetSoftwareDirs(const WCHAR* paths);

void SetHashDirs(const WCHAR *dir);
const WCHAR *GetHashDirs(void);

void SetSelectedSoftware(int driver_index, const machine_config *config, const device_config *device, const WCHAR *software);
const WCHAR *GetSelectedSoftware(int driver_index, const machine_config *config, const device_config *device);

void SetExtraSoftwarePaths(int driver_index, const WCHAR *extra_paths);
const WCHAR *GetExtraSoftwarePaths(int driver_index);

void SetCurrentSoftwareTab(const char *shortname);
const char *GetCurrentSoftwareTab(void);

#endif /* OPTIONSMS_H */

