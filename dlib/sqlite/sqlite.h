// Copyright (C) 2011  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_SQLiTE_H_
#define DLIB_SQLiTE_H_

#include "sqlite_abstract.h"

#include <iostream>
#include <vector>
#include "../algs.h"
#include <sqlite3.h>
#include "../smart_pointers.h"

// ----------------------------------------------------------------------------------------

namespace dlib
{
    namespace sqlite
    {

    // ------------------------------------------------------------------------------------

        struct sqlite_error : public error
        {
            sqlite_error(const std::string& message): error(message) {}
        };

    // ------------------------------------------------------------------------------------

        namespace impl
        {
            struct db_deleter
            {
                void operator()(
                    sqlite3* db
                )const 
                { 
                    sqlite3_close(db);
                }
            };
        }

    // ------------------------------------------------------------------------------------

        class database : noncopyable
        {
        public:
            database(
            ) 
            {
            }

            database (
                const std::string& file
            ) 
            {
                open(file);
            }

            bool is_open (
            ) const
            {
                return db.get() != 0;
            }

            void open (
                const std::string& file
            )
            {
                filename = file;
                sqlite3* ptr = 0;
                int status = sqlite3_open(file.c_str(), &ptr);
                db.reset(ptr, impl::db_deleter());
                if (status != SQLITE_OK)
                {
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
            }

            const std::string& get_database_filename (
            ) const
            {
                return filename;
            }

            inline void exec (
                const std::string& sql_statement
            );

        private:

            friend class statement;

            std::string filename;
            shared_ptr<sqlite3> db;
        };

    // ------------------------------------------------------------------------------------

        class statement : noncopyable
        {
        public:
            statement (
                database& db_,
                const std::string sql_statement
            ) : 
                needs_reset(false),
                step_status(SQLITE_DONE),
                at_first_step(true),
                db(db_.db),
                stmt(0),
                sql_string(sql_statement)
            {
                int status = sqlite3_prepare_v2(db.get(), 
                                             sql_string.c_str(),
                                             sql_string.size()+1,
                                             &stmt,
                                             NULL);

                if (status != SQLITE_OK)
                {
                    sqlite3_finalize(stmt);
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
                if (stmt == 0)
                {
                    throw sqlite_error("Invalid SQL statement");
                }
            }

            ~statement(
            )
            {
                sqlite3_finalize(stmt);
            }

            void exec(
            )
            {
                reset();

                step_status = sqlite3_step(stmt);
                needs_reset = true;
                if (step_status != SQLITE_DONE && step_status != SQLITE_ROW)
                {
                    if (step_status == SQLITE_ERROR)
                        throw sqlite_error(sqlite3_errmsg(db.get()));
                    else if (step_status == SQLITE_BUSY)
                        throw sqlite_error("sqlite::statement::exec() failed.  SQLITE_BUSY returned");
                    else
                        throw sqlite_error("sqlite::statement::exec() failed.");
                }
            }

            bool move_next (
            )
            {
                if (step_status == SQLITE_ROW)
                {
                    if (at_first_step)
                    {
                        at_first_step = false;
                        return true;
                    }
                    else
                    {
                        step_status = sqlite3_step(stmt);
                        if (step_status == SQLITE_DONE)
                        {
                            return false;
                        }
                        else if (step_status == SQLITE_ROW)
                        {
                            return true;
                        }
                        else
                        {
                            throw sqlite_error(sqlite3_errmsg(db.get()));
                        }
                    }
                }
                else
                {
                    return false;
                }
            }

            unsigned long get_num_columns(
            ) const
            {
                if( (at_first_step==false) && (step_status==SQLITE_ROW))
                {
                    return sqlite3_column_count(stmt);
                }
                else
                {
                    return 0;
                }
            }

            const std::string& get_sql_string (
            ) const
            {
                return sql_string;
            }


            const std::vector<char> get_column_as_blob (
                unsigned long idx
            ) const
            {
                const char* data = static_cast<const char*>(sqlite3_column_blob(stmt, idx));
                const int size = sqlite3_column_bytes(stmt, idx);

                return std::vector<char>(data, data+size);
            }

            template <typename T>
            void get_column_as_object (
                unsigned long idx,
                T& item
            ) const
            {
                const char* data = static_cast<const char*>(sqlite3_column_blob(stmt, idx));
                const int size = sqlite3_column_bytes(stmt, idx);
                std::istringstream sin(std::string(data,size));
                deserialize(item, sin);
            }

            const std::string get_column_as_text (
                unsigned long idx
            ) const
            {
                const char* data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, idx));
                return std::string(data);
            }

            double get_column_as_double (
                unsigned long idx
            ) const
            {
                return sqlite3_column_double(stmt, idx);
            }

            int get_column_as_int (
                unsigned long idx
            ) const
            {
                return sqlite3_column_int(stmt, idx);
            }

            int64 get_column_as_int64 (
                unsigned long idx
            ) const
            {
                return sqlite3_column_int64(stmt, idx);
            }

            const std::string get_column_name (
                unsigned long idx
            ) const
            {
                return std::string(sqlite3_column_name(stmt,idx));
            }

            unsigned long get_max_parameter_id (
            ) const
            {
                return  SQLITE_LIMIT_VARIABLE_NUMBER;
            }

            unsigned long get_parameter_id (
                const std::string& name
            ) const
            {
                return sqlite3_bind_parameter_index(stmt, name.c_str());
            }

            void bind_blob (
                unsigned long parameter_id,
                const std::vector<char>& item
            )
            {
                reset();
                int status = sqlite3_bind_blob(stmt, parameter_id, &item[0], item.size(), SQLITE_TRANSIENT);

                if (status != SQLITE_OK)
                {
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
            }

            template <typename T>
            void bind_object (
                unsigned long parameter_id,
                const T& item
            )
            {
                reset();
                std::ostringstream sout;
                serialize(item, sout);
                const std::string& str = sout.str();
                int status = sqlite3_bind_blob(stmt, parameter_id, str.data(), str.size(), SQLITE_TRANSIENT);

                if (status != SQLITE_OK)
                {
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
            }

            void bind_double (
                unsigned long parameter_id,
                const double& item
            )
            {
                reset();
                int status = sqlite3_bind_double(stmt, parameter_id, item);

                if (status != SQLITE_OK)
                {
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
            }

            void bind_int (
                unsigned long parameter_id,
                const int& item
            )
            {
                reset();
                int status = sqlite3_bind_int(stmt, parameter_id, item);

                if (status != SQLITE_OK)
                {
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
            }

            void bind_int64 (
                unsigned long parameter_id,
                const int64& item
            )
            {
                reset();
                int status = sqlite3_bind_int64(stmt, parameter_id, item);

                if (status != SQLITE_OK)
                {
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
            }

            void bind_null (
                unsigned long parameter_id
            )
            {
                reset();
                int status = sqlite3_bind_null(stmt, parameter_id);

                if (status != SQLITE_OK)
                {
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
            }

            void bind_text (
                unsigned long parameter_id,
                const std::string& item
            )
            {
                reset();
                int status = sqlite3_bind_text(stmt, parameter_id, item.c_str(), -1, SQLITE_TRANSIENT);

                if (status != SQLITE_OK)
                {
                    throw sqlite_error(sqlite3_errmsg(db.get()));
                }
            }

        private:

            void reset()
            {
                if (needs_reset)
                {
                    if (sqlite3_reset(stmt) != SQLITE_OK)
                    {
                        step_status = SQLITE_DONE;
                        throw sqlite_error(sqlite3_errmsg(db.get()));
                    }
                    needs_reset = false;
                    step_status = SQLITE_DONE;
                    at_first_step = true;
                }
            }

            bool needs_reset; // true if sqlite3_step() has been called more recently than sqlite3_reset() 
            int step_status;
            bool at_first_step;

            shared_ptr<sqlite3> db;
            sqlite3_stmt* stmt;
            std::string sql_string;
        };

    // ------------------------------------------------------------------------------------

        void database::
        exec (
            const std::string& sql_statement
        )
        {
            statement(*this, sql_statement).exec();
        }

    // ------------------------------------------------------------------------------------

    }
}

#endif // DLIB_SQLiTE_H_
