#include <sqlite_helper.h>
#include <vector>

int sql_exec_with_return_callback(void* data, int argc, char **argv, char**azColName)
{
    if (argc != 1)
    {
        printf("column num is not 1.\n");
        return -1;
    }

    std::vector<std::string> *pRetValue = (std::vector<std::string>*)data;

    if (argv[0] != NULL)
    {
        pRetValue->push_back(argv[0]);
    }
    
    return 0;
}

int sql_exec_multicol_return_callback(void* data, int argc, char **argv, char**azColName)
{
    if (argc != 1)
    {
        printf("column num is %d.\n", argc);
        // return -1;
    }

    std::vector<std::string> *pRetValue = (std::vector<std::string>*)data;

    for (int i = 0; i < argc; i++)
    {
        if (argv[i] != NULL)
        {
            pRetValue->push_back(argv[i]);
        }
        else
        {
            pRetValue->push_back("");
        }
    }
    
    return 0;
}

db_helper::db_helper(std::string s_path)
{
    m_path = s_path;
    m_db = NULL;
}

db_helper::~db_helper()
{
    if (m_db)
    {
        db_close();
        m_db = NULL;
    }
    
    m_resultvec.clear();
}

int db_helper::sql_exec_with_return(std::string s_sql)
{
    db_open();
    m_resultvec.clear();
    if (m_db == NULL)
    {
        printf("m_db is NULL.\n");
        return -1;
    }
    int rc = 0;
    rc = db_exec(s_sql.c_str());

    db_close();
}

int db_helper::sql_exec_multicol_return(std::string s_sql)
{
    db_open();
        m_resultvec.clear();
    if (m_db == NULL)
    {
        printf("m_db is NULL.\n");
        return -1;
    }
    int rc = 0;
    rc = db_multicol_exec(s_sql.c_str());

    db_close();
}

std::vector<std::string> db_helper::getsqlresult()
{
    return m_resultvec;
}

int db_helper::db_open()
{
    int rc = sqlite3_open(m_path.c_str(), &m_db);
    if (rc != SQLITE_OK)
    {
        printf("open database: %s failed\n", sqlite3_errmsg(m_db));
        sqlite3_close(m_db);

        return -1;
    }

    printf("opened database successfully\n");

    return 0;
}

int db_helper::db_close()
{
    int rc = 0;
    if (m_db)
    {
        rc = sqlite3_close(m_db);
    }
    else
    {
        return 0;
    }
    
    
    if (rc != SQLITE_OK)
    {
        printf("close database: %s failed\n", sqlite3_errmsg(m_db));

        return -1;
    }
    m_db = NULL;

    printf("closed database successfully\n");

    return 0;
}

int db_helper::db_exec(const char *sql)
{
    char *errmsg = NULL;
    int rc = sqlite3_exec(m_db, sql, sql_exec_with_return_callback, (void*)&m_resultvec, &errmsg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);

        return -1;
    }

    return 0;
}

int db_helper::db_multicol_exec(const char *sql)
{
    char *errmsg = NULL;
    int rc = sqlite3_exec(m_db, sql, sql_exec_multicol_return_callback, (void*)&m_resultvec, &errmsg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);

        return -1;
    }

    return 0;
}
