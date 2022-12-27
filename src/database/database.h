#ifndef _VA_DATABASE_DATABASE_H_
#define _VA_DATABASE_DATABASE_H_

#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/driver.h>

struct VADatabase {
	sql::Connection* m_conn;
	sql::Driver* m_driver;
	sql::Statement* m_statement;
	sql::PreparedStatement* m_prep_statement;
	sql::ResultSet* m_res;
	std::string m_database;

	VADatabase(std::string& url, std::string& username, std::string& password, std::string& database, bool sync);
	~VADatabase();

	void create_table();
	void create_database();
	void select();
	void insert(int id, char* name);
};

#endif
