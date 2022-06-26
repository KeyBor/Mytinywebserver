#include "sqlCon.h"
#include <iostream>
sqlCon::sqlCon(string user,string password,string databaseName,string ip, unsigned short port):m_result(nullptr){
    //初始mysql连接
    m_conn = mysql_init(nullptr);
    if(m_conn == nullptr){
        exit(1);
    }
    //设置字符编码
    mysql_set_character_set(m_conn,"utf8");
    MYSQL* ptr = mysql_real_connect(m_conn,ip.c_str(),user.c_str(),password.c_str(),databaseName.c_str(),port,NULL,0);
    if(ptr != m_conn){
        m_error = mysql_error(m_conn);
        std::cout<<m_error<<std::endl;
        exit(1);
    }
}

sqlCon::~sqlCon(){
    if(m_conn != nullptr){
        mysql_close(m_conn);
    }
    freeResult();
}

bool sqlCon::update(char* sql){
    if(!mysql_query(m_conn,sql)){
        return true;
    }   
    return false;
}

bool sqlCon::query(char* sql){
    freeResult();
    if(!mysql_query(m_conn,sql)){
        m_result = mysql_store_result(m_conn);
        return true;
    }
    return false;
}

bool sqlCon::next(){
    if(m_result!= nullptr){
        m_row = mysql_fetch_row(m_result);
        if(m_row != nullptr){
            return true;
        }
    }
    return false;
}

string sqlCon::value(int index){
    int colCount = mysql_num_fields(m_result);
    if(index<0 || index> colCount){
        std::cout<<"index越界了!"<<endl;
        return string("");
    }
    char* str = m_row[index];
    //以数组返回上一次用 mysql_fetch_row() 取得的行中每个字段的长度，如果出错返回 FALSE
    unsigned long len = mysql_fetch_lengths(m_result)[index];
    return string(str,len);
}

bool sqlCon::transaction(){
    return mysql_autocommit(m_conn,false);
}
bool sqlCon::commit(){
    return mysql_commit(m_conn);
}

bool sqlCon::rollback(){
    return mysql_rollback(m_conn);
}

void sqlCon::freeResult(){
    if(m_result != nullptr){
        mysql_free_result(m_result);
    }
}

//刷新存活时间
void sqlCon::refreshTime(){
    m_fresh = chrono::steady_clock::now();
}
//获取存活的总时长
long long sqlCon::getAliveTime(){
    chrono::nanoseconds alivetime = chrono::steady_clock::now() - m_fresh;
    chrono::milliseconds dur = chrono::duration_cast<chrono::milliseconds>(alivetime);
    return dur.count();
}