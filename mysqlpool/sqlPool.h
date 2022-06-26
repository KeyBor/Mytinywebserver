#pragma once
#include <string>
#include <json/json.h>
#include <queue>
#include <thread>
#include <condition_variable>
#include "sqlCon.h"
#include <mutex>

using namespace std;
//使用单例设计模式
class sqlPool{
public:
    static sqlPool* GetInstance(){
        static sqlPool pool;
        return &pool;
    }

    void addConnection();
    sqlPool(sqlPool& pool) = delete;
    void operator=(sqlPool& pool) = delete;
    void produce();
    void recycle();
    ~sqlPool();
    shared_ptr<sqlCon> getConnection();
      
private:
    string m_user;
    string m_passwd;
    string m_database;
    string m_ip;
    unsigned short m_port;
    int m_minSize;
    int m_maxSize;
    int m_timeout;
    int m_maxIdelTime;
    queue<sqlCon*> m_queue;
    mutex m_mutex;
    condition_variable m_cond;
    int m_close_log = 0;
    int m_close=0;

private:
    bool ParseJson();
    sqlPool();

};