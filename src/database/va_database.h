#ifndef VA_DATABASE_DATABASE_H_
#define VA_DATABASE_DATABASE_H_

#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/driver.h>

#include "va_object_meta.h"

namespace va {
struct Database {
	sql::Connection* m_conn = nullptr;
	sql::Driver* m_driver = nullptr;
	sql::Statement* m_statement = nullptr;
	sql::PreparedStatement* m_insert_prep_statement = nullptr;
	sql::PreparedStatement* m_select_prep_statement = nullptr;
	sql::ResultSet* m_res = nullptr;
	std::string m_database;

	Database(std::string& url, std::string& username, std::string& password, std::string& database, bool sync);
	~Database();

	auto create_table() -> void;
	auto create_database() -> void;
	auto select(int limit) -> void;
	auto insert(va::FrameMetadata* frame_meta) -> void;
};

} // namespace va

#endif
