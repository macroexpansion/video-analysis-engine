#include <iostream>
#include <memory>

#include "database.h"
#include "engine.h"

int main(int argc, char** argv) {
	try {
		std::string url { "tcp://127.0.0.1:3306" };
		std::string username { "root" };
		std::string password { "example" } ;
		std::string database { "va" } ;

		VADatabase db { url, username, password, database, false };
		/* db.select();
		db.insert(10, "test10");
		db.select(); */

		/* VAEngine engine { argc, argv };
		engine.set_database(&db);
		engine.init(); */

		std::unique_ptr<VAEngine> engine { std::make_unique<VAEngine>(argc, argv) };
		engine->set_database(&db);
		engine->init();
	} catch (std::invalid_argument& e) {
		g_printerr("%s", e.what());
	} catch (std::exception& e) {
		g_printerr("%s", e.what());
	}

	return EXIT_SUCCESS;
}
