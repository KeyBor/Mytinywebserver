#include "sqlPool.h"
#include <iostream>



//不用数据库连接池
int op1(int num)
{
    for(int i =0;i<num;++i){
        sqlCon con("root","8ik,9ol.","tinywebserver","127.0.0.1",3306);
        char* sql = "insert into user values('sqlpool','sqlpool')";
        bool flag = con.update(sql);
    }
    return 0;
}

void op2(int num,sqlPool* pool)
{
    
    for(int i =0;i<num;++i){
        auto con = pool->getConnection();
        if(con == nullptr){
            std::cout<<"这是空指针"<<std::endl;
            return;
        }
        char* sql = "insert into user values('sqlpool','sqlpool')";
        bool flag = con->update(sql);
    }
}


void test2(){
#if 1
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    sqlPool* pool = sqlPool::GetInstance();
    thread t1(op2,1000,pool);
    thread t2(op2,1000,pool);
    thread t3(op2,1000,pool);
    thread t4(op2,1000,pool);
    thread t5(op2,1000,pool);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    auto length = end -begin;
    std::cout<<"连接池，多线程，用时"<<length.count()/1000000<<"毫秒"<<std::endl;
#else
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    thread t1(op1,1000);
    thread t2(op1,1000);
    thread t3(op1,1000);
    thread t4(op1,1000);
    thread t5(op1,1000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    auto length = end -begin;
    std::cout<<"非连接池，多线程，用时"<<length.count()/1000000<<"毫秒"<<std::endl;
#endif
}


void test1(){
#if 1
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    sqlPool* pool = sqlPool::GetInstance();
    thread t1(op2,5000,pool);
    t1.join();
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    auto length = end -begin;
    std::cout<<"连接池，多线程，用时"<<length.count()/1000000<<"毫秒"<<std::endl;
#else
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    thread t1(op1,5000);
    t1.join();
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    auto length = end -begin;
    std::cout<<"非连接池，多线程，用时"<<length.count()/1000000<<"毫秒"<<std::endl;
#endif
}
int main(){
    test2();
    std::cout<<"end"<<std::endl;
    return 0;
}