#pragma once

#include <mysql/mysql.h>
#include <string>
#include <chrono>
using namespace std;

class sqlCon
{
public:
    //建立连接
    sqlCon(string user,string password,string databaseName,string ip, unsigned short port=3306);
    //释放连接
    ~sqlCon();
    //更新数据
    bool update(char* sql);
    //查询数据
    bool query(char*sql);
    //遍历查询得到的结果集
    bool next();
    //获取结果集中的字段值
    string value(int index);
    //事务操作
    bool transaction();
    //提交事务
    bool commit();
    //回滚
    bool rollback();
    //刷新存活时间
    void refreshTime();
    //获取存活的总时长
    long long getAliveTime();

    inline MYSQL* get(){return m_conn;}
private:
    MYSQL* m_conn = nullptr;
    const char* m_error;
    MYSQL_RES* m_result;
    MYSQL_ROW m_row = nullptr;
    chrono::steady_clock::time_point m_fresh;
    int m_close_log = 0;
    void freeResult();

};