#include "sqlPool.h"
#include <fstream>
#include <iostream>
#include "log/log.h"
using namespace Json;


bool sqlPool::ParseJson()
{
    ifstream ifs("/root/project/MyTinyWebServer/mysqlpool/dbconf.json");
    if(!ifs.is_open())
    {
        LOG_ERROR("数据库文件打开失败!");
        return false;
    }
    Reader rd;
    Value root;
    rd.parse(ifs,root);
    if(root.isObject()){
        m_database = root["dbName"].asString();
        m_user = root["user"].asString();
        m_passwd = root["passwd"].asString();
        m_ip = root["ip"].asString();
        m_port = root["port"].asInt();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_timeout = root["timeout"].asInt();
        m_maxIdelTime = root["maxIdleTime"].asInt();
        return true;
    }
    return false;
}

void sqlPool::addConnection()
{
    sqlCon* con = new sqlCon(m_user,m_passwd,m_database,m_ip,m_port);
    if(!con)
    {
      LOG_ERROR("sql连接创建失败,添加连接失败");
      return;  
    }
    con->refreshTime();
    m_queue.push(con);
}


shared_ptr<sqlCon> sqlPool::getConnection()
{
    unique_lock<mutex> locker(m_mutex);
    while(m_queue.empty()){
        if(cv_status::timeout == m_cond.wait_for(locker,chrono::milliseconds(m_timeout)))
        {
            if(m_queue.empty()){
                return nullptr;
            }
        }  
    }
    //使用make_shared方法就无法自定义删除器
    shared_ptr<sqlCon> con(m_queue.front(),[this](sqlCon* con){
        lock_guard<mutex> gurad(m_mutex);
        con->refreshTime();
        m_queue.push(con);
        std::cout<<"智能指针回归啦"<<std::endl;
        std::cout<<m_queue.size()<<std::endl;
    });
    
    m_queue.pop();
    m_cond.notify_all();
    return con;
}


sqlPool::sqlPool()
{
    if(!ParseJson()){
        return;
    }
    for(int i=0;i<m_minSize;++i){
        addConnection();
    }
}

sqlPool::~sqlPool()
{
    std::cout<<"sqlpool析构"<<std::endl;
    while(!m_queue.empty()){
        sqlCon* con = m_queue.front();
        m_queue.pop();
        delete con;
    }
    m_cond.notify_all();
    std::cout<<"sqlpool析构完成"<<std::endl;
}