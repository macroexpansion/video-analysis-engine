#include "va_user_data.h"

va::UserData::UserData(va::Database* _va_database, int _save_interval)
	: va_database(_va_database), frame_count(0), save_interval(_save_interval) { }

va::UserData::UserData(va::Database* _va_database) : va_database(_va_database) {}

va::UserData::~UserData() { }
