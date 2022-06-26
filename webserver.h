#pragma once
#include <string>
#include <sys/epoll.h>
#include "http/http_conn.h"
#include "threadpool/threadpool.h"
#include "timer/lst_timer.h"

const int MAX_FD=65536; //最大文件描述符数量
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;   //最小超时单位 

class webserver{
public:
    webserver();
    ~webserver();
    void init(int port , string user, string passWord, string databaseName,
            int log_write , int opt_linger, int trigmode, int sql_num,
            int thread_num, int close_log, int actor_model);

    void thread_pool();
    void sql_pool();
    void log_write();
    void trig_mode();
    void eventListen();
    void eventLoop();
    void timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(util_timer *timer);
    void deal_timer(util_timer *timer, int sockfd);
    bool dealclinetdata();
    bool dealwithsignal(bool& timeout, bool& stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

private:
//基础
int m_port;                //端口号
char* m_root;              //资源根目录
int m_log_write;
int m_close_log;
int m_actormodel;

int m_pipefd[2];
int m_epollfd;             //控制epoll资源的文件描述符
http_conn* users;          //http连接的用户队列
int m_sql_num;

//数据库相关
sqlPool* m_connPool;
string m_user;             //数据库用户名
string m_password;         //数据库密码
string m_databaseName;     //数据库名


//线程池相关
threadpool<http_conn>* m_pool;
int m_thread_num;

//epoll_event相关
epoll_event events[MAX_EVENT_NUMBER];

int m_listenfd;            //监听用文件描述符
int m_OPT_LINGER;
int m_TRIGMode;              //设置触发模式
int m_LISTENTrigmode;
int m_CONNTrigmode;

//定时器相关
client_data* users_timer;
Utils utils;


};