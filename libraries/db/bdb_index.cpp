/*
 * Copyright (c) 2018- μNEST Foundation, and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <graphene/db/bdb_index.hpp>

// File: bdb.cpp

graphene::db::bdb::bdb()
	: _db(bdb_env::getInstance().getDbEnv(), 0), _cFlags(DB_CREATE), _isOpen(false)
{
	
}

// Class constructor. Requires a path to the location
// where the database is located, and a database name
graphene::db::bdb::bdb(const char*path, bool allow_duplicate)
	: _db(bdb_env::getInstance().getDbEnv(), 0),  // Instantiate Db object
	_isOpen(false),
	_dbFileName(path),		// Database file name
	_cFlags(DB_CREATE)          // If the database doesn't yet exist, allow it to be created.
{
	open(_dbFileName.c_str(), allow_duplicate);
}

void graphene::db::bdb::open(const char* path, bool allow_duplicate)
{
	_dbFileName = path;

	try
	{
		// Redirect debugging information to std::cerr
		_db.set_error_stream(&std::cerr);

		// If this is a secondary database, support sorted duplicates
		if (allow_duplicate)
			_db.set_flags(DB_DUP | DB_DUPSORT);
		
		// Open the database
		int ret = _db.open(nullptr, _dbFileName.c_str(), nullptr, DB_BTREE, _cFlags, 0644);
		_isOpen = !ret;
	}
	// DbException is not a subclass of std::exception, so we
	// need to catch them both.
	catch (DbException &e)
	{
		std::cerr << "Error opening database: " << _dbFileName << "\n";
		std::cerr << e.what() << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << "Error opening database: " << _dbFileName << "\n";
		std::cerr << e.what() << std::endl;
	}
}

// Private member used to close a database. Called from the class
// destructor.
void graphene::db::bdb::close()
{
	// Close the db
	try
	{
		_db.close(0);
		_isOpen = false;
		std::cout << "Berkeley DB: " << _dbFileName << " is closed." << std::endl;
	}
	catch (DbException &e)
	{
		std::cerr << "Error closing database: " << _dbFileName << "\n";
		std::cerr << e.what() << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << "Error closing database: " << _dbFileName << "\n";
		std::cerr << e.what() << std::endl;
	}
}

graphene::db::bdb_env::bdb_env()
{
	//
	// Create an environment object and initialize it for error reporting.
	//
	_dbenv = new DbEnv((u_int32_t)0);
	_dbenv->set_error_stream(&std::cerr);
	_dbenv->set_errpfx("BTS::BDB");

}

void graphene::db::bdb_env::init(const char* home, const char* data_dir)
{ 
	// We want to specify the shared memory buffer pool cachesize,
	// but everything else is the default.
	//
	_dbenv->set_cachesize(0, 512 * 1024 * 1024, 0);
    _dbenv->set_tmp_dir(home);

	// Databases are in a subdirectory.
	_dbenv->set_data_dir(data_dir);

	// Open the environment with full transactional support.
	try {
		int ret = _dbenv->open(home,
			DB_CREATE | DB_PRIVATE | DB_INIT_CDB | DB_INIT_MPOOL, 0); // | DB_INIT_TXN | DB_INIT_LOCK | DB_INIT_LOG

		if(ret)
			std::cerr << "failed to init BerkeleyDB environment: ret=" << ret << "\n";
	}
	catch (DbException &dbe) {
		std::cerr << "Init BerkeleyDB environment: " << dbe.what() << "\n"; 
		exit(-1);
	}
}

int object_id_comp(Db* db, const Dbt* key1, const Dbt* key2, size_t* size)
{
    graphene::db::object_id_type* k1 = (graphene::db::object_id_type*)key1->get_data();
    graphene::db::object_id_type* k2 = (graphene::db::object_id_type*)key2->get_data();

    int64_t ii = k1->number - k2->number;
    return ii > 0 ? 1 : ii == 0 ? 0 : -1;
}
