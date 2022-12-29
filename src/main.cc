#include <iostream>
#include <memory>

#include "va_database.h"
#include "va_engine.h"
#include "va_object_meta.h"

auto main(int argc, char** argv) -> int {
	try {
		std::string url { "tcp://127.0.0.1:3306" };
		std::string username { "root" };
		std::string password { "example" } ;
		std::string database { "va" } ;

		va::Database db { url, username, password, database, false };
		// db.select(5);

		std::unique_ptr<va::Engine> engine { std::make_unique<va::Engine>(argc, argv) };
		engine->set_database(&db);
		engine->run();
	} catch (std::invalid_argument& e) {
		g_printerr("%s", e.what());
	} catch (std::exception& e) {
		g_printerr("%s", e.what());
	}

	return EXIT_SUCCESS;
}
