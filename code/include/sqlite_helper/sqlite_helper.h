#ifndef SQLITE_HELPER_H_INCLUDED_
#define SQLITE_HELPER_H_INCLUDED_

#include "sqlite3.h"
#include <vector>
#include <string>

#define DB_FILE_PATH "./iot.db"

typedef int (*callback)(void*,int,char**,char**);

class db_helper
{
public:
    db_helper(std::string s_path);
    ~db_helper();
    int sql_exec_with_return(std::string s_sql);
    int sql_exec_multicol_return(std::string s_sql);
    std::vector<std::string> getsqlresult();

private:
    sqlite3 *m_db;
    std::string m_path;
    int db_open();
    int db_close();
    // 获取一列数据查询结果
    int db_exec(const char *sql);
    // 获取多列数据查询结果，依然在m_resultvec中
    int db_multicol_exec(const char *sql);
    // int sql_exec_with_return_callback(void* data, int argc, char **argv, char**azColName);
    std::vector<std::string> m_resultvec;
};

#endif