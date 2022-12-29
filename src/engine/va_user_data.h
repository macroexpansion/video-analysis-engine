#ifndef VA_ENGINE_USER_DATA_H_
#define VA_ENGINE_USER_DATA_H_

#include "va_database.h"

namespace va {
/**
 * Check if running using config file or .h264 stream file
 */
struct UserData {
	va::Database* va_database;

	UserData(va::Database* _va_database);
	~UserData();
};

} // namespace va

#endif
