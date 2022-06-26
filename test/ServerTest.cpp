#include <gtest/gtest.h>
#include "config.h"
#include <string>
#include <sys/epoll.h>
#include "http/http_conn.h"
#include "threadpool/threadpool.h"

int setnonblocking(int fd);


void addfd(int epollfd, int fd, bool one_shot);
class webservertest{
public:
    webservertest();
    ~webservertest();
    void init(int port , string user, string passWord, string databaseName,
            int log_write , int opt_linger, int trigmode, int sql_num,
            int thread_num, int close_log, int actor_model);

    void thread_pool();
    void sql_pool();
    void log_write();
    void trig_mode();
    void eventListen();
    void eventLoop();
    bool dealclinetdata();
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

private:
//基础
int m_port;                //端口号
char* m_root;              //资源根目录
int m_log_write;
int m_close_log;
int m_actormodel;

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

};


webservertest::webservertest()
{
    //http_conn类对象
    users = new http_conn[MAX_FD];

    //root文件夹路径
    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);
}

webservertest::~webservertest()
{
    close(m_epollfd);
    close(m_listenfd);
    delete[] users;
    delete m_pool;
}

void webservertest::init(int port, string user, string passWord, string databaseName, int log_write, 
                     int opt_linger, int trigmode, int sql_num, int thread_num, int close_log, int actor_model)
{
    m_port = port;
    m_user = user;
    m_password = passWord;
    m_databaseName = databaseName;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_log_write = log_write;
    m_OPT_LINGER = opt_linger;
    m_TRIGMode = trigmode;
    m_close_log = close_log;
    m_actormodel = actor_model;
}

void webservertest::trig_mode()
{
    //LT + LT
    if (0 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 0;
    }
    //LT + ET
    else if (1 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
    //ET + LT
    else if (2 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
    //ET + ET
    else if (3 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}

void webservertest::log_write()
{
    if (0 == m_close_log)
    {
        //初始化日志
        if (1 == m_log_write)
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
        else
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
    }
}

void webservertest::sql_pool()
{
    //初始化数据库连接池
    m_connPool = sqlPool::GetInstance();
    //初始化数据库读取表
    users->initmysql_result(m_connPool);
}

void webservertest::thread_pool()
{
    //线程池
    m_pool = new threadpool<http_conn>(m_actormodel, m_connPool, m_thread_num);
}


void webservertest::eventListen()
{
    //网络编程基础步骤
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);

    //优雅关闭连接
    if (0 == m_OPT_LINGER)
    {
        struct linger tmp = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if (1 == m_OPT_LINGER)
    {
        struct linger tmp = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
    ret = listen(m_listenfd, 5);


    //epoll创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    addfd(m_epollfd,m_listenfd,false);

}


bool webservertest::dealclinetdata()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    if (0 == m_LISTENTrigmode)
    {
        int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
        if (connfd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        if (http_conn::m_user_count >= MAX_FD)
        {
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
    }

    else
    {
        while (1)
        {
            int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
            if (connfd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }
            if (http_conn::m_user_count >= MAX_FD)
            {
                LOG_ERROR("%s", "Internal server busy");
                break;
            }
        }
        return false;
    }
    return true;
}

void webservertest::dealwithread(int sockfd)
{

    //reactor
    if (1 == m_actormodel)
    {

        //若监测到读事件，将该事件放入请求队列
        m_pool->append(users + sockfd, 0);

        while (true)
        {
            if (1 == users[sockfd].improv)
            {
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (users[sockfd].read_once())
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
            m_pool->append_p(users + sockfd);

        }
    }
}

void webservertest::dealwithwrite(int sockfd)
{
    //reactor
    if (1 == m_actormodel)
    {
        m_pool->append(users + sockfd, 1);

        while (true)
        {
            if (1 == users[sockfd].improv)
            {
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (users[sockfd].write())
        {
            LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

        }

    }
}

void webservertest::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;

    while (!stop_server)
    {
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;

            //处理新到的客户连接
            if (sockfd == m_listenfd)
            {
                bool flag = dealclinetdata();
                if (false == flag)
                    continue;
            }
            //处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                dealwithread(sockfd);
            }
            else if (events[i].events & EPOLLOUT)
            {
                dealwithwrite(sockfd);
            }
        }
    }
}

class ServerTest: public ::testing::Test{
protected:
    void SetUp()
    {}

    void TearDown()
    {
    }

    static webservertest* server;
};

/*
    "ip":"127.0.0.1",
    "port":3306,
    "user":"root",
    "passwd":"8ik,9ol.",
    "dbName":"tinywebserver",
    "minSize":100,
    "maxSize":1024,
    "maxIdleTime":5000,
    "timeout":1000
*/
TEST_F(ServerTest,testserverinit)
{

    server->init(3306,"root","8ik,9ol.","tinywebserver",1,1,0,100,8,0,0);
    //日志
    server->log_write();

    //数据库
    server->sql_pool();

    //线程池
    server->thread_pool();

    //触发模式
    server->trig_mode();

    //监听
    server->eventListen();

    //运行
    server->eventLoop();
}

