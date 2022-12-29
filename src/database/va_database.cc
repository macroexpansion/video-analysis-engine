#include "va_database.h"

va::Database::Database(
	std::string& url,
	std::string& username, 
	std::string& password, 
	std::string& database,
	bool sync
) : m_database(database)  {
	m_driver = get_driver_instance();
	m_conn = m_driver->connect(url, username, password);

	if (sync) {
		create_table();
	}

	m_conn->setSchema(database);

	m_insert_prep_statement = m_conn->prepareStatement("INSERT INTO metadata(video_file, object_label, class_id, box_left, box_top, box_width, box_height, timestamp) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
	m_select_prep_statement = m_conn->prepareStatement("SELECT id, video_file, object_label, class_id, box_left, box_top, box_width, box_height, timestamp FROM metadata LIMIT ?");
}

va::Database::~Database() {
	std::cout << "start VA DATABASE deallocate" << std::endl;
	delete m_res;
	delete m_statement;
	delete m_insert_prep_statement;
	delete m_select_prep_statement;
	delete m_conn;
	std::cout << "finish VA DATABASE deallocate" << std::endl;
}

/* auto Database::create_database(std::string& database) -> void {
	m_statement = m_conn->createStatement();
	m_statement->execute("CREAT DABASE")
} */

auto va::Database::create_table() -> void {
	m_statement = m_conn->createStatement();
	m_statement->execute("USE " + m_database);
	m_statement->execute("DROP TABLE IF EXISTS metadata");
	m_statement->execute("CREATE TABLE metadata(id INT AUTO_INCREMENT PRIMARY KEY, video_file TEXT(20), object_label TEXT(20), class_id INT, box_left FLOAT, box_top FLOAT, box_width FLOAT, box_height FLOAT, timestamp BIGINT)");
	m_statement->execute("INSERT INTO metadata(video_file, object_label, class_id, box_left, box_top, box_width, box_height, timestamp) VALUES ('video file', 'Car', 0, 5.5, 5.5, 5.5, 5.5, 100000000000)");

	std::cout << "Created new table" << std::endl;
}

auto va::Database::select(int limit) -> void {
	try {
		m_select_prep_statement->setInt(1, limit);
		std::unique_ptr<sql::ResultSet> res { m_select_prep_statement->executeQuery() };

		while (res->next()) {
			std::cout << "{ ";
			std::cout << res->getInt("id") << ", ";
			std::cout << res->getString("video_file") << ", ";
			std::cout << res->getString("object_label") << ", ";
			std::cout << res->getInt("class_id") << ", ";
			std::cout << res->getDouble("box_left") << ", ";
			std::cout << res->getDouble("box_top") << ", ";
			std::cout << res->getDouble("box_width") << ", ";
			std::cout << res->getDouble("box_height") << ", ";
			std::cout << res->getInt64("timestamp");
			std::cout << " }" << std::endl;
		}
	} catch (sql::SQLException& e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ")" << std::endl;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
}

auto va::Database::insert(va::FrameMetadata* frame_meta) -> void {
	m_insert_prep_statement->setString(1, frame_meta->video_file);
	for (va::ObjectMetadata object_meta : frame_meta->va_object_meta_list) {
		m_insert_prep_statement->setString(2, object_meta.object_label);
		m_insert_prep_statement->setInt(3, object_meta.class_id);
		m_insert_prep_statement->setDouble(4, object_meta.left);
		m_insert_prep_statement->setDouble(5, object_meta.top);
		m_insert_prep_statement->setDouble(6, object_meta.width);
		m_insert_prep_statement->setDouble(7, object_meta.height);
		m_insert_prep_statement->setInt64(8, object_meta.timestamp);

		m_insert_prep_statement->execute();
	}
}
