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
#pragma once

// use berkeley db to store objects
//
#include <db_cxx.h> 
#include <graphene/db/index.hpp>  
#include <fc/log/file_appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/log/logger_config.hpp>

int object_id_comp(Db* db, const Dbt* key1, const Dbt* key2, size_t* size);

#define _BDB_DATA_VERSION       -1ll
#define _BDB_NEXT_ID            -2ll
#define _BDB_SDB_DONOTINDEX(k)  (((k) & 0xFFFFFFFFFFFFFF00) == 0xFFFFFFFFFFFFFF00 )
namespace graphene { namespace db {


class bdb
{
public:
    bdb();

    // Constructor requires a path to the database,
    // and a database name.
    bdb(const char* path, bool allow_duplicate = false);

    // Our destructor just calls our private close method.
    ~bdb() { close(); }

    inline Db &getDb() { return _db; }
    Db* operator ->() { return &_db; }

    void open(const char* path, bool allow_duplicate = false);
    void close();
    bool isOpen() const { return _isOpen; }

private:
    Db _db;
    std::string _dbFileName;
    u_int32_t _cFlags;
    bool _isOpen;
};

// a singleton to hold a berkeley db environment.

class bdb_env
{
public:
    ~bdb_env()
    {
        if (_dbenv != nullptr)
        {
            delete _dbenv;
            _dbenv = nullptr;
        }
    }
    static bdb_env& getInstance()
    {
        static bdb_env m_instance;
        return m_instance;
    }

    DbEnv* getDbEnv() const
    {
        return _dbenv;
    }

    operator DbEnv* () const
    {
        return _dbenv;
    }

    void init(const char* home, const char* data_dir);

private:
    bdb_env();

private:
    DbEnv *_dbenv;
};

template<typename ObjectType>
class bdb_secondary_index;

template<typename ObjectType>
class bdb_iterator;

template<typename ObjectType>
class bdb_index : public index
{
public:
    typedef ObjectType     object_type;

    bdb_index()
    {
        std::string file = fc::to_string(object_type::space_id) + "." + fc::to_string(object_type::type_id);
        _bdb->set_bt_compare(object_id_comp);
        _bdb.open(file.c_str());
    }

    virtual bool is_external_db() const override { return true; }

    // this function is just an override, do not rely on the returned obj
    // please use create_db() instead
    virtual const object&  create(const std::function<void(object&)>& constructor) override
    {
        object_type obj;
        obj.id = get_next_id();
        constructor(static_cast<object_type&>(obj));

        vector<char> v = obj.pack();
        Dbt data(v.data(), v.size());
        uint64_t id = (uint64_t)obj.id;
        Dbt key(&id, sizeof(id));

        auto ret = _bdb->put(nullptr, &key, &data, 0);

        FC_ASSERT(!ret, "Could not create object and insert into berkeley DB");
        use_next_id();
        return obj; // this return obj is not reliable,
    }

    virtual std::unique_ptr<object> create_db(const std::function<void(object&)>& constructor) override
    {
        std::unique_ptr<object> obj(new object_type());
        obj->id = get_next_id();
        constructor(static_cast<object_type&>(*obj));

        vector<char> v = obj->pack();
        Dbt data(v.data(), v.size());
        uint64_t id = (uint64_t)obj->id;
        Dbt key(&id, sizeof(id));

        int ret = 0;

        try {
            ret = _bdb->put(nullptr, &key, &data, DB_OVERWRITE_DUP);
        }
        catch (DbException& e)
        {
            ilog("Berkeley DB::put error, msg=${e}, key=${k} ",
                ("e", e.what())
                ("k", (std::string)obj->id));
            std::cerr << "Berkeley DB::put: " << e.what() << std::endl;
        }

        // _bdb->sync(0); // write to disk

        FC_ASSERT(!ret, "Could not create object and insert into berkeley DB");
        use_next_id();
        return obj;
    }

    // this function is just an override, do not rely on the returned obj
    // please use insert_db() instead
    virtual const object& insert(object&& obj) override
    {
        object_type *pObj = dynamic_cast<ObjectType*>(&obj);
        assert(nullptr != pObj);

        vector<char> v = static_cast<ObjectType&>(obj).pack();
        uint64_t id = (uint64_t)obj.id;
        Dbt key(&id, sizeof(id));
        Dbt data(v.data(), v.size());

        int ret = _bdb->put(nullptr, &key, &data, 0);

        FC_ASSERT(!ret, "Could not create object and insert into berkeley DB");
        return obj; // This is not allowed, but we have to return a lvalue.
    }

    virtual bool insert_db(object&& obj) override
    {
        object_type *pObj = dynamic_cast<ObjectType*>(&obj);
        assert(nullptr != pObj);

        vector<char> v = static_cast<ObjectType&>(obj).pack();
        uint64_t id = (uint64_t)obj.id;
        Dbt key(&id, sizeof(id));
        Dbt data(v.data(), v.size());

        int ret = _bdb->put(nullptr, &key, &data, 0);

        FC_ASSERT(!ret, "Could not insert object into berkeley db. ret=${ret}", ("ret", ret));
        return !ret;
    }

    virtual void modify(const object& obj, const std::function<void(object&)>& modify_callback) override
    {
        assert(nullptr != dynamic_cast<const ObjectType*>(&obj));

        Dbt data;;
        uint64_t id = (uint64_t)obj.id;
        Dbt key(&id, sizeof(id));

        auto ret = _bdb->get(nullptr, &key, &data, 0);
        FC_ASSERT(!ret, "Could not modify object, not found");

        object_type objFromDB;
        fc::raw::unpack<object_type>((const char*)data.get_data(), data.get_size(), objFromDB);

        modify_callback(objFromDB);

        vector<char> v = objFromDB.pack();
        Dbt modData(v.data(), v.size());
        ret = _bdb->put(nullptr, &key, &modData, 0);

        FC_ASSERT(!ret, "Could not modify object from berkeley db");
    }

    virtual void remove_db(object_id_type id) override
    {
        remove(id);
    }

    virtual void remove(object_id_type id) override
    {
        uint64_t uid = (uint64_t)id;
        Dbt key(&uid, sizeof(uid));
        try {
            auto status = _bdb->del(nullptr, &key, 0);
            FC_ASSERT(!status, "Could not delete object from berkeley db, error=${status}, id:${id}", ("status", status) ("id", (std::string)id));
        }
        catch (DbException& e)
        {
            wlog("Berkeley DB: remove(): ${id}, DbException: ${msg}", ("id", (std::string)id) ("msg", e.what()));
        }
    }

    virtual void remove(const object& obj) override
    {
        assert(nullptr != dynamic_cast<const ObjectType*>(&obj));
        remove(obj.id);
    }

    // callers have to free the returned obj 
    // do not use this function for this class, use find_db instead.
    virtual const object* find(object_id_type id)const override
    {
        assert(id.space() == ObjectType::space_id);
        assert(id.type() == ObjectType::type_id);

        return nullptr;
    }

    virtual std::unique_ptr<object> find_copy(object_id_type id)const override
    {
        return find_db(id);
    }

    virtual std::unique_ptr<object> find_db(object_id_type id) const override
    {
        uint64_t uid = (uint64_t)id;
        Dbt key(&uid, sizeof(uid));
        Dbt data;
        bdb& bdb_ = const_cast<bdb&>(_bdb);
        auto ret = bdb_->get(nullptr, &key, &data, 0);

        if (ret) //  == DB_NOTFOUND 
            return std::unique_ptr<object_type>(nullptr);

        std::unique_ptr<object> obj(new object_type());
        fc::raw::unpack<object_type>((const char*)data.get_data(), data.get_size(), dynamic_cast<object_type&>(*obj));

        return obj;
    }

    virtual void inspect_all_objects(std::function<void(const object&)> inspector)const override
    {
        ;
    }

    virtual fc::uint128 hash()const override {
        fc::uint128 result;
        // for (const auto& ptr : _objects)
        //	result += ptr->hash();

        return result;
    }

    void flush()
    {
        _bdb->sync(0);

        for (int ii = 0; ii < _s_indexs.size(); ++ii)
        {
            (*_s_indexs[ii])->sync(0);
        }
    }

    void save_next_id()
    {
        int64_t k = _BDB_NEXT_ID;
        Dbt key((void*)(&k), sizeof(k));

        object_id_type next_id = get_next_id();
        Dbt data(&next_id, sizeof(next_id));
        _bdb->put(nullptr, &key, &data, 0);
        _bdb->sync(0);

        std::cout << "Berkeley DB: Saved next_id: " << (std::string)next_id << std::endl;
    }

    void load_next_id()
    {
        int64_t k = _BDB_NEXT_ID;
        Dbt key((void*)(&k), sizeof(k));

        Dbt data;
        int ret = _bdb->get(nullptr, &key, &data, 0);
        if (!ret)
        {
            object_id_type next_id;
            memcpy(&next_id, data.get_data(), data.get_size());
            set_next_id(next_id);

            std::cout << "Berkeley DB: Loaded next_id: " << (std::string)next_id << std::endl;
        }
    }
    void save_data_version(fc::sha256& version)
    {
        int64_t k = _BDB_DATA_VERSION;
        Dbt key((void*)(&k), sizeof(k));

        Dbt data(&version, sizeof(fc::sha256));
        _bdb->put(nullptr, &key, &data, 0);
        _bdb->sync(0);

        std::cout << "Berkeley DB: Saved data_version: " << (std::string)version << std::endl;
    }

    bool load_data_version(fc::sha256& version)
    {
        int64_t k = _BDB_DATA_VERSION;
        Dbt key((void*)(&k), sizeof(k));

        Dbt data;
        data.set_data(&version);
        data.set_ulen(sizeof(fc::sha256));
        data.set_flags(DB_DBT_USERMEM);
        int ret = _bdb->get(nullptr, &key, &data, 0);
        if (!ret)
        {
            std::cout << "Berkeley DB: Loaded data_version: " << (std::string)version << std::endl;
        }
        return !ret;
    }

    int add_bdb_secondary_index(bdb_secondary_index<ObjectType>* secondary_index, int(*callback)(Db*, const Dbt*, const Dbt*, Dbt*))
    {
        std::unique_ptr<bdb_secondary_index<ObjectType>> sindexptr(secondary_index);
        _s_indexs.push_back(std::move(sindexptr));

        int pos = _s_indexs.size() - 1;

        int ret = _bdb->associate(nullptr, *secondary_index, callback, DB_CREATE); // DB_IMMUTABLE_KEY
        if (!ret)
        {
            std::cout << "Berkeley DB: add_secondary_index(): " << std::endl;
            return pos;
        }
        else
        {
            std::cerr << "Berkeley DB: failed add_secondary_index(): " << std::endl;
        }
        return -1;
    }

    std::unique_ptr<object_type> find_db_secondary(int pos, void* k, size_t size)
    {
        Dbt key(k, size);
        Dbt data;
        auto ret = _s_indexs[pos]->get(nullptr, &key, &data, 0);

        if (ret) //  == DB_NOTFOUND 
            return std::unique_ptr<object_type>(nullptr);

        std::unique_ptr<object_type> obj(new object_type());
        fc::raw::unpack<object_type>((const char*)data.get_data(), data.get_size(), *obj);

        return obj;
    }

    bdb_secondary_index<object_type>& get_bdb_secondary_index(int pos) const
    {
        return *_s_indexs[pos];
    }

    bdb_iterator<object_type> lower_bound(object_id_type id) const
    {
        return open_cursor(id);
    }

    bdb_iterator<object_type> upper_bound(object_id_type id) const
    {
        return open_cursor(id);
    }

    bdb_iterator<object_type> end() const
    {
        return bdb_iterator<object_type>::end();
    }

    void set_raw_data(bool b)
    {
        _raw_data = b;
    }


private:
    bdb _bdb;
    vector<std::unique_ptr<bdb_secondary_index<object_type>>> _s_indexs;

    bool _raw_data = false;  // no pack/unpack, put the raw struct

    bdb_iterator<object_type> open_cursor(object_id_type id, u_int32_t flags = DB_SET_RANGE) const
    {
        Dbc* cursorp;
        bdb& bdb_ = const_cast<bdb&>(_bdb);
        int ret = bdb_->cursor(nullptr, &cursorp, 0);
        if (ret)
            return end();

        Dbt skey(&id, sizeof(object_id_type));
        Dbt data;
        ret = cursorp->get(&skey, &data, flags);
        if (ret)
        {
            // FC_ASSERT(ret, "Berkeley DB: lower_bound() Could not find key");
            cursorp->close();
            return end();
        }

        object_type obj;
        fc::raw::unpack<ObjectType>((const char*)data.get_data(), data.get_size(), obj);

        return bdb_iterator<object_type>(cursorp, obj);
    }

    bool move_cursor(void* cursorp, ObjectType& obj, u_int32_t flags) const
    {
        Dbt skey, data;
        int ret = ((Dbc*)cursorp)->get(&skey, &data, flags);
        if (ret && ret != DB_NOTFOUND)
        {
            FC_ASSERT(ret, "Berkeley DB: cursor move error: ret=${ret}", ("ret", ret));
        }
        else if (!ret)
        {
            fc::raw::unpack<ObjectType>((const char*)data.get_data(), data.get_size(), obj);
            return true;
        }
        return false;
    }
};


template<typename ObjectType>
class primary_index<bdb_index<ObjectType>> : public bdb_index<ObjectType>, public base_primary_index
{
public:
    typedef bdb_index<ObjectType> DerivedIndex;
    typedef typename DerivedIndex::object_type object_type;

    primary_index(object_database& db)
        :base_primary_index(db), _next_id(object_type::space_id, object_type::type_id, 0) {}

    virtual uint8_t object_space_id()const override
    {
        return object_type::space_id;
    }

    virtual uint8_t object_type_id()const override
    {
        return object_type::type_id;
    }

    virtual object_id_type get_next_id()const override { return _next_id; }
    virtual void           use_next_id()override { ++_next_id.number; }
    virtual void           set_next_id(object_id_type id)override { _next_id = id; ilog("Berkeley DB: set_next_id(): ${id}", ("id", (std::string)id)); }

    fc::sha256 get_object_version()const
    {
        std::string desc = "1.0";//get_type_description<object_type>();
        return fc::sha256::hash(desc);
    }

    virtual void open(const path& db)override
    {
        fc::sha256 open_ver;

        DerivedIndex::load_next_id();
        if (DerivedIndex::load_data_version(open_ver))
            FC_ASSERT(open_ver == get_object_version(), "Incompatible Version, the serialization of objects in this db has changed");
    }

    virtual void save(const path& db) override
    {
        DerivedIndex::flush();
        DerivedIndex::save_next_id();
        auto ver = get_object_version();
        DerivedIndex::save_data_version(ver);
    }

    virtual const object&  load(const std::vector<char>& data)override
    {
        const auto& result = DerivedIndex::insert(fc::raw::unpack<object_type>(data));
        for (const auto& item : _sindex)
            item->object_inserted(result);
        return result;
    }


    virtual const object&  create(const std::function<void(object&)>& constructor)override
    {
        const auto& result = DerivedIndex::create(constructor);
        for (const auto& item : _sindex)
            item->object_inserted(result);
        on_add(result);
        return result;
    }

    virtual std::unique_ptr<object> create_db(const std::function<void(object&)>& constructor) override
    {
        auto objptr = DerivedIndex::create_db(constructor);
        const auto& result = static_cast<object_type&>(*objptr);
        for (const auto& item : _sindex)
            item->object_inserted(result);
        on_add(result);
        return objptr;
    }

    virtual const object& insert(object&& obj) override
    {
        const auto& result = DerivedIndex::insert(std::move(obj));
        for (const auto& item : _sindex)
            item->object_inserted(result);
        on_add(result);
        return result;
    }

    virtual void  remove(const object& obj) override
    {
        for (const auto& item : _sindex)
            item->object_removed(obj);
        on_remove(obj);
        DerivedIndex::remove(obj);
    }

    virtual void  remove(object_id_type id) override
    {
        DerivedIndex::remove(id);
    }

    virtual void  remove_db(object_id_type id) override
    {
        DerivedIndex::remove(id);
    }

    virtual void modify(const object& obj, const std::function<void(object&)>& m)override
    {
        save_undo(obj);
        for (const auto& item : _sindex)
            item->about_to_modify(obj);
        DerivedIndex::modify(obj, m);
        for (const auto& item : _sindex)
            item->object_modified(obj);
        on_modify(obj);
    }

    virtual void add_observer(const shared_ptr<index_observer>& o) override
    {
        _observers.emplace_back(o);
    }

    virtual void object_from_variant(const fc::variant& var, object& obj, uint32_t max_depth)const override
    {
        object_id_type id = obj.id;
        object_type* result = dynamic_cast<object_type*>(&obj);
        FC_ASSERT(result != nullptr);
        fc::from_variant(var, *result, max_depth);
        obj.id = id;
    }

    virtual void object_default(object& obj)const override
    {
        object_id_type id = obj.id;
        object_type* result = dynamic_cast<object_type*>(&obj);
        FC_ASSERT(result != nullptr);
        (*result) = object_type();
        obj.id = id;
    }

private:
    object_id_type _next_id;
};

typedef int(*key_comp_fun)(Db*, const Dbt*, const Dbt*, size_t*);

template<typename ObjectType>
class bdb_secondary_index
{
public:
    typedef ObjectType     object_type;

    bdb_secondary_index(bdb_secondary_index&& rv) : _bdb(rv._bdb)
    {
    }

    bdb_secondary_index(std::string tag, bool duplicate, key_comp_fun cmp_fun = nullptr)
    {
        std::string file = fc::to_string(object_type::space_id) + "." + fc::to_string(object_type::type_id) + "." + tag;
        if (nullptr != cmp_fun)
            _bdb->set_bt_compare(cmp_fun);
        _bdb.open(file.c_str(), duplicate);
        FC_ASSERT(_bdb.isOpen(), "Berkeley DB: Could not open file '${file}'", ("file", file));
    }

    operator Db* ()
    {
        return &_bdb.getDb();
    }
    Db* operator->()
    {
        return &_bdb.getDb();
    }

    void set_bt_key_compare(int(*bt_k_comp)(Db*, const Dbt*, const Dbt*, size_t*))
    {
        _bdb->set_bt_compare(bt_k_comp);
    }

    void* locate(void* key, u_int32_t size, ObjectType& obj) const
    {
        return open_cursor(key, size, obj, DB_SET);
    }

    void* lower_bound(void* key, u_int32_t size, ObjectType& obj) const
    {
        return open_cursor(key, size, obj);
    }

    void* upper_bound(void* key, u_int32_t size, ObjectType& obj) const
    {
        return open_cursor(key, size, obj);
    }

    bool get_next(void* cursorp, ObjectType& obj) const
    {
        return move_cursor(cursorp, obj, DB_NEXT);
    }
    bool get_previous(void* cursorp, ObjectType& obj) const
    {
        return move_cursor(cursorp, obj, DB_PREV);
    }

    void close_cursor(void* cursorp) const
    {
        if (cursorp != nullptr)
            ((Dbc*)cursorp)->close();
    }

    bool exists(void* k, u_int32_t size) const
    {
        Dbt key(k, size);
        bdb& bdb_ = const_cast<bdb&>(_bdb);
        auto ret = bdb_->exists(nullptr, &key, 0);

        return !ret; //  == DB_NOTFOUND  
    }

    std::unique_ptr<ObjectType> find_db(void* k, u_int32_t size) const
    {
        Dbt key(k, size);
        Dbt data;
        bdb& bdb_ = const_cast<bdb&>(_bdb);
        auto ret = bdb_->get(nullptr, &key, &data, 0);

        if (ret) //  == DB_NOTFOUND 
            return std::unique_ptr<ObjectType>(nullptr);

        std::unique_ptr<ObjectType> obj(new ObjectType());
        fc::raw::unpack<ObjectType>((const char*)data.get_data(), data.get_size(), dynamic_cast<ObjectType&>(*obj));

        return obj;
    }


private:
    Dbc* open_cursor(void* key, u_int32_t size, ObjectType& obj, u_int32_t flags = DB_SET_RANGE) const
    {
        Dbc* cursorp;
        bdb& bdb_ = const_cast<bdb&>(_bdb);
        int ret = bdb_->cursor(nullptr, &cursorp, 0);
        if (ret)
            return nullptr;

        Dbt skey(key, size);
        Dbt data;
        ret = cursorp->get(&skey, &data, flags);
        if (ret)
        {
            // FC_ASSERT(ret, "Berkeley DB: lower_bound() Could not find key");
            cursorp->close();
            return nullptr;
        }

        fc::raw::unpack<ObjectType>((const char*)data.get_data(), data.get_size(), obj);

        return cursorp;
    }

    bool move_cursor(void* cursorp, ObjectType& obj, u_int32_t flags) const
    {
        Dbt skey, data;
        int ret = ((Dbc*)cursorp)->get(&skey, &data, flags);
        if (ret && ret != DB_NOTFOUND)
        {
            FC_ASSERT(ret, "Berkeley DB: cursor move error: ret=${ret}", ("ret", ret));
        }
        else if (!ret)
        {
            fc::raw::unpack<ObjectType>((const char*)data.get_data(), data.get_size(), obj);
            return true;
        }
        return false;
    }

    bdb_iterator<object_type> end() const
    {
        return bdb_iterator<object_type>::end();
    }

private:
    bdb _bdb;

};

template<typename ObjectType>
class bdb_iterator
{
public:
    typedef ObjectType     object_type;

    bdb_iterator(Dbc* cursor, object_type& o) : _cursorp(cursor), _obj(o), _is_end(cursor == nullptr)
    {
    }

    bdb_iterator() : _cursorp(nullptr), _is_end(true)
    {
    }

    ~bdb_iterator()
    {
        if (_cursorp != nullptr)
            _cursorp->close();
    }

    static bdb_iterator<object_type> end()
    {
        bdb_iterator<object_type> the_end(true);
        return the_end;
    }

    explicit bdb_iterator(bool is_end) : _cursorp(nullptr), _is_end(is_end) {}

    object_type& operator* () const
    {
        return _obj;
    }

    bool operator == (const bdb_iterator& rhs) const
    {
        if (_is_end && rhs._is_end)
            return true;

        return false; //TODO
    }

    bool operator != (const bdb_iterator& rhs) const
    {
        return !(operator == (rhs));
    }

    bdb_iterator& operator ++ ()
    {
        move_cursor(DB_NEXT);
        return *this;
    }
    bdb_iterator operator ++ (int)
    {
        bdb_iterator temp = *this;
        temp._cursorp = nullptr;

        move_cursor(DB_NEXT);
        return temp;
    }
    bdb_iterator& operator -- ()
    {
        move_cursor(DB_PREV);
        return *this;
    }
    bdb_iterator operator -- (int)
    {
        bdb_iterator temp = *this;
        temp._cursorp = nullptr;

        move_cursor(DB_PREV);
        return temp;
    }

    object_type* operator->()
    {
        return &_obj;
    }

private:
    bool move_cursor(u_int32_t flags)
    {
        Dbt skey, data;
        int ret = _cursorp->get(&skey, &data, flags);
        if (ret && ret != DB_NOTFOUND)
        {
            FC_ASSERT(ret, "Berkeley DB: cursor move error: ret=${ret}", ("ret", ret));
        }
        else if (!ret)
        {
            fc::raw::unpack<object_type>((const char*)data.get_data(), data.get_size(), _obj);
            return true;
        }

        _is_end = true;
        _obj = object_type();

        return false;
    }

private:
    Dbc* _cursorp;
    object_type _obj;
    bool _is_end;
};

}
}
