#include "database.h"

VADatabase::VADatabase(
	std::string& url,
	std::string& username, 
	std::string& password, 
	std::string& database,
	bool sync
) : m_statement(nullptr), m_res(nullptr), m_database(database)  {
	m_driver = get_driver_instance();
	m_conn = m_driver->connect(url, username, password);

	if (sync) {
		create_table();
	}

	m_conn->setSchema(database); // Connect to the MySQL test database
	m_prep_statement = m_conn->prepareStatement("INSERT INTO metadata(id, name) VALUES (?, ?)");
}

VADatabase::~VADatabase() {
	std::cout << "start VA DATABASE deallocate" << std::endl;
	if (m_res) {
		delete m_res;
	}
	if (m_statement) {
		delete m_statement;
	}
	if (m_prep_statement) {
		delete m_prep_statement;
	}
	if (m_conn) {
		delete m_conn;
	}
	std::cout << "finish VA DATABASE deallocate" << std::endl;
}

/* void VADatabase::create_database(std::string& database) {
	m_statement = m_conn->createStatement();
	m_statement->execute("CREAT DABASE")
} */

void VADatabase::create_table() {
	m_statement = m_conn->createStatement();
	m_statement->execute("USE " + m_database);
	m_statement->execute("DROP TABLE IF EXISTS metadata");
	m_statement->execute("CREATE TABLE metadata(id INT, name CHAR(50))");
	m_statement->execute("INSERT INTO metadata(id, name) VALUES (1, 'test')");
}

void VADatabase::select() {
	try {
		m_statement = m_conn->createStatement();
		m_res = m_statement->executeQuery("SELECT id, name FROM metadata LIMIT 1"); // replace with your statement
		while (m_res->next()) {
			std::cout << m_res->getInt("id") << std::endl;
			std::cout << m_res->getString("name") << std::endl;
		}
	} catch (sql::SQLException& e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ")" << std::endl;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
}

void VADatabase::insert(int id, char* name) {
	m_prep_statement = m_conn->prepareStatement("INSERT INTO metadata(id, name) VALUES (?, ?)");
	m_prep_statement->setInt(1, id);
	m_prep_statement->setString(2, name);
	m_prep_statement->execute();
}
