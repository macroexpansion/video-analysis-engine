#ifndef VA_ENGINE_USER_DATA_H_
#define VA_ENGINE_USER_DATA_H_

#include "va_database.h"

namespace va {
/**
 * Represent custom user data to inject to GStreamer pipeline
 */
struct UserData {
	va::Database* va_database;
	int frame_count;
	int save_interval;
	std::string video_file;

	// copy constructor and assignment
	UserData(const UserData& other) = default;
	UserData& operator=(const UserData& other) = default;

	// move constructor and assignment
	UserData(UserData&& other) = default;
	UserData& operator=(UserData&& other) = default;

	UserData(va::Database* _va_database, int _save_interval);
	UserData(va::Database* _va_database);
	~UserData();
};

} // namespace va

#endif
