#include "allfile.h"

char s_port[50];
#define NOT_FOUNT "not_found.html"
#define NOT_IMPLEMENTED "not_implemented.html" 

struct http_info
{
    char method[100];
    char uri[1024];
    char http_version[100];
    char other_into[1024];
};
//entrance funtcion
void child_signal(int sig);
void *thread(void *sock);
void go_to_work(int fd);

//process method function
void process_dynamic_data(int fd, char *method,char *uri, char *filename, char *cgiargs, char *other_request_info);
void process_static_data(int fd,char *filename,int filesize);
void process_head_method(int connfd, int file_size, char *file_type);
void process_options_method(int fd, char *file_type, int file_size);
void process_post_method(int fd, char *method, char *uri, char *filename, char *cgiargs, char *other_request_info);

/* auxiliary function */
void read_request(rio_t *rp, char *other_request_info);
void get_file_type(char *filename,char *filetype);
int get_the_uri(char *uri, char *file_name, char *cgi_args);
int get_file_size(char *file_name, char *file_type);
void set_all_cgi_env(char *cgi_args, char *info, char *method, char *uri);
int is_http_protocol(char *version);
int read_php(int fd,char *file_name);

/*process error*/
void send_error(int fd);
void send_error_to_client(int fd,char *cause,char *errnum,char *shortmsg,char *longmsg);
void read_html(int fd,char *file_name);


void send_error(int fd)
{
    char status[] = "HTTP/1.0 400 Bad Request\r\n";
    char header[] = "Server: A Simple Web Server\r\nContent-Type: text/html\r\n\r\n";
    char body[] = "<html><head><title>Bad Request</title></head><body><p>请求出错，请检查！</p></body></html>";
    // 向客户端套接字发送信息
    write(fd, status, sizeof(status));
    write(fd, header, sizeof(header));
    write(fd, body, sizeof(body));
}

void process_head_method(int connfd,int file_size,char *file_type)
{
    char buf[MAXLINE];
    sprintf(buf,"HTTP/1.0 200 OK\r\n");
    Rio_writen(connfd,buf,strlen(buf));
    sprintf(buf,"Server: Min Web Server\r\n");
    Rio_writen(connfd,buf,strlen(buf));
    sprintf(buf,"Content-Length:%d\r\n",file_size);
    Rio_writen(connfd,buf,strlen(buf));
    sprintf(buf,"Content-Type: %s\r\n",file_type);
    Rio_writen(connfd,buf,strlen(buf));
}
void process_options_method(int fd,char *file_type,int file_size)
{
    char buf[MAXLINE];
    sprintf(buf,"HTTP/1.0 200 OK\r\n");
    Rio_writen(fd,buf,strlen(buf));
    time_t current_time;
    struct tm *timer;
    time(&current_time);
    timer=localtime(&current_time);
    sprintf(buf,"Date: %s\r\n",asctime(timer));
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Min Web Server\r\n");
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Allow: GET,POST,HEAD,OPTIONS\r\n");
    Rio_writen(fd,buf,strlen(buf));
    /* here will be write again */
    sprintf(buf,"Content-length:%d\r\n",file_size);
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-type: %s\r\n",file_type);
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Connection:close\r\n");
    Rio_writen(fd,buf,strlen(buf));
}
void process_dynamic_data(int fd,char *method,char *uri,char *filename,char *cgiargs,char *other_request_info)
{
    /*return the first part of HTTP response*/
    char buf[MAXLINE],*emptylist[]={NULL};
    sprintf(buf,"HTTP/1.0 200 OK\r\n");
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Server: Min Web Server\r\n");
    Rio_writen(fd,buf,strlen(buf));
    char file_type[80];

    get_file_type(filename,file_type);
    char *args[]={"php-cgi","first.phpkkfjefiefleifefefjie",NULL};
    char *envp[]={"PATH=/usr/bin",NULL};
	char *args2[]={"python","testpython.py",NULL};
    if(strcmp(file_type,"text/php")==0)
    {
        args[1]=filename;
        Dup2(fd,STDOUT_FILENO);
        execve("/usr/bin/php-cgi",args,environ);
        //execve("/usr/bin/php-cgi",args,envp); 
    }
    else if(strcmp(file_type,"text/py"==0))
    {
    	args2[1]=filename;
    	Dup2(fd,STDOUT_FILENO);
    	execve("/usr/bin/python",args2,environ);
    	//execve("/usr/bin/python",args2,envp);
	}
    else
    {
        set_all_cgi_env(cgiargs,other_request_info,method,uri);
        Dup2(fd,STDOUT_FILENO);
        Execve(filename,emptylist,environ);
    }
}
void process_post_method(int fd,char *method,char *uri,char *filename,char *cgiargs,char *other_request_info)
{
    /*return the first part of HTTP response*/
    char buf[MAXLINE],*emptylist[]={NULL};
    sprintf(buf,"HTTP/1.0 200 OK\r\n");
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Server: Min Web Server\r\n");
    Rio_writen(fd,buf,strlen(buf));
    char file_type[80];

    get_file_type(filename,file_type);
    char *args[]={"php-cgi","first.phpkkfjefiefleifefefjie",NULL};
    char *envp[]={"PATH=/usr/bin",NULL};

    if(strcmp(file_type,"text/php")==0)
    {
        args[1]=filename;
        set_all_cgi_env(cgiargs,other_request_info,method,uri);
        Dup2(fd,STDOUT_FILENO);
        //execve("/usr/bin/php-cgi",args,envp);
        execve("/usr/bin/php-cgi",args,environ);
    }
    else if(strcmp(file_type,"text/py"==0))
    {
    	args2[1]=filename;
    	Dup2(fd,STDOUT_FILENO);
    	execve("/usr/bin/python",args2,environ);
    	//execve("/usr/bin/python",args2,envp);
	}
    else
    {

        set_all_cgi_env(cgiargs,other_request_info,method,uri);
        Dup2(fd,STDOUT_FILENO);
        Execve(filename,emptylist,environ);
        Wait(NULL);
    }
}

void process_static_data(int fd,char *filename,int filesize)
{
    int srcfd;
    char *srcp,buf[MAXLINE*4],filetype[MAXLINE];
    get_file_type(filename,filetype);
    //HTTP response message
    sprintf(buf,"HTTP/1.0 200 OK\r\n");   //response line
    sprintf(buf,"%sTiny Web Server\r\n",buf);
    sprintf(buf,"%sConten-Length: %d\r\n",buf,filesize);
    //need a null line to stop reponse header
    sprintf(buf,"%sConten-Type: %s\r\n\r\n",buf,filetype);
    Rio_writen(fd,buf,strlen(buf));

    /*
     *before change
    srcfd=Open(filename,O_RDONLY,0);
    srcp=Mmap(0,filesize,PROT_READ,MAP_PRIVATE,srcfd,0);  //j将一个文件映射到调用进程的地址空间
    Close(srcfd);
    Rio_writen(fd,srcp,filesize);
    Munmap(srcp,filesize);
    */

    srcfd=Open(filename,O_RDONLY,0);
    srcp=(char *)Malloc(sizeof(char)*filesize);
    Rio_readn(srcfd,srcp,filesize);
    Close(srcfd);
    Rio_writen(fd,srcp,filesize);
    Free(srcp);
}


void child_signal(int sig)
{
    pid_t pid;
    while((pid=Waitpid(-1,NULL,WNOHANG))>0)
        ;
    //chdEnded=1;
}

//new function as follow

int is_http_protocol(char *version)
{
    int flags=0;
    if((strcasecmp(version,"HTTP/1.0")==0)||
            (strcasecmp(version,"HTTP/1.1")==0))
        flags=1;
    return flags;
}

void set_all_cgi_env(char *cgi_args,char *info,char *method,char *uri)
{
    char user_agent[MAXLINE],accept[MAXLINE],accept_lang[MAXLINE],accept_encoding[MAXLINE];
    char connection[MAXLINE];
    char *p;
    char file_name[MAXLINE];
    strcat(file_name,"/root/wing/minweb");
    strcat(file_name,uri);
    if(strstr(info,"User-Agent:")!=NULL)
    {
        p=strstr(info,"Accept:");
        if(p!=NULL)
        {
            strcpy(accept,p);
            *p='\0';
            strcpy(user_agent,info);
            p=strstr(accept,"Accept-Language:");
            if(p!=NULL)
            {
                strcpy(accept_lang,p);
                *p='\0';
            }
            p=strstr(accept_lang,"Accept-Encoding:");
            if(p!=NULL)
            {
                strcpy(accept_encoding,p);
                *p='\0';
            }
            p=strstr(accept_encoding,"Connection:");
            if(p!=NULL)
            {
                strcpy(connection,p);
                *p='\0';
            }
        }
    }
    setenv("COMSPEC","/usr/bin",1);
    setenv("DOCUMENT_ROOT","/root/wing/minweb",1);
    setenv("GATEWAY_INTERFACE","CGI/1.1",1);
    setenv("HTTP_ACCEPT",accept,1);
    setenv("HTTP_ACCEPT_ENCODING",accept_encoding,1);
    setenv("HTTP_ACCEPT_LANGUAGE",accept_lang,1);
    setenv("HTTP_CONNECTION",connection,1);
    setenv("HTTP_HOST","localhost:8080",1);
    setenv("HTTP_USER_AGENT",user_agent,1);
    setenv("QUERY_STRING",cgi_args,1);
    setenv("REMOTE_ADDR","::1",1);
    setenv("REMOTE_PORT","1294",1);
    setenv("REQUEST_METHOD",method,1);
    setenv("REQUEST_URI",uri,1);
    setenv("SCRIPT_FILENAME",file_name,1);
    setenv("SCRIPT_NAME","localhost",1);
    setenv("SERVER_ADDR","::1",1);
    setenv("SERVER_ADMIN","gdczt2012@163.com",1);
    setenv("SERVER_NAME","localhost",1);
    setenv("SERVER_PORT",s_port,1);
    setenv("SERVER_PROTOCOL","HTTP/1.1",1);
    setenv("SERVER_SIGNATURE","/usr/bin",1);
    setenv("SERVER_SOFTWARE","Min Web Server",1);
}

//old function as follow

int main(int argc,char *argv[])
{
    int listenfd,client_len,port;
    int *connfd;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    if(argc!=2)
    {
        fprintf(stderr,"usage: %s<port> \n",argv[0]);
        exit(1);
    }
    //Signal(SIGCHLD,child_signal);
    strcpy(s_port,argv[1]);
    port=atoi(argv[1]);
    listenfd=Open_listenfd(port);

    while(1)
    {
        //chdEnded=0;
        client_len=sizeof(clientaddr);
        connfd=Malloc(sizeof(int));
        *connfd=Accept(listenfd,(SA*)&clientaddr,&client_len);
        Pthread_create(&tid,NULL,thread,connfd);
    }
}
void *thread(void *sock)
{
    int connfd=*((int *)sock);
    Pthread_detach(pthread_self());
    Free(sock);
    go_to_work(connfd);
    Close(connfd);
    return NULL;
}

void go_to_work(int fd)
{
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    struct stat stat_buf;
    char file_type[MAXLINE];
    char file_name[MAXLINE],cgi_args[MAXLINE];
    int file_size;
    char other_request_info[MAXLINE*2];
    int static_or_dynamic;
    rio_t rio;

    Rio_readinitb(&rio,fd);
    Rio_readlineb(&rio,buf,MAXLINE);
    //sscanf(buf,"%s %s %s",method,uri,version);
    int i=0,j=0,k=0;
    //判断请求的方法前面是否有空格
    while(isspace(buf[i]))
    {
        i++;
    }
    if(i!=0)
    {
        j=i;
    }
    while(!isspace(buf[j]))
    {
        method[k]=buf[j];
        k++;
        j++;
    }
    method[k]='\0';
    //if(strcasecmp(method,"GET"))
        //request_error("the method is not implement");
    k=0;
    j++;
    //get the uri
    while(!isspace(buf[j]))
    {
        uri[k]=buf[j];
        k++;
        j++;
    }
    uri[k]='\0';
    ++j;
    k=0;
    //求出HTTP的版本
    while(!isspace(buf[j]))
    {
        version[k]=buf[j];
        k++;
        j++;
    }
    version[k]='\0';
    int is_http=is_http_protocol(version);
    if(!is_http)
    {
        send_error_to_client(fd,file_name,"505","Not Support This HTTP",
                     "Min Server not support this HTTP version.");
        return;
    }
    printf("method: %s,uri: %s,version: %s\n",method,uri,version);
    
    read_request(&rio,other_request_info);
    static_or_dynamic=get_the_uri(uri,file_name,cgi_args);
    printf("filename is:%s\n",file_name);
    printf("cgi_args is: %s\n",cgi_args);
    if(stat(file_name,&stat_buf)<0)
    {
        send_error_to_client(fd,file_name,"404","Not found",
                     "Min Web Server couldn't find this file");
        return ;
    }
    file_size=stat_buf.st_size;
    if(strcasecmp(method,"get")==0)
    {
        if(static_or_dynamic)
        {
            if(!(S_ISREG(stat_buf.st_mode)||!(S_IRUSR & stat_buf.st_mode)))
            {
                send_error_to_client(fd,file_name,"403","Forbidden",
                             "Min Web Server can't read the file");
                return;
            }
            process_static_data(fd,file_name,stat_buf.st_size);
        }
        else
        {
            if(!(S_ISREG(stat_buf.st_mode)||!(S_IXUSR & stat_buf.st_mode)))
            {
                send_error_to_client(fd,file_name,"403","Forbidden",
                             "Min Web Server can't run the CGI program");
                return;
            }
            process_dynamic_data(fd,method,uri,file_name,cgi_args,other_request_info);
        }
    }
    else if(strcasecmp(method,"post")==0)
    {
        if(stat(file_name,&stat_buf)<0)
        {
            send_error_to_client(fd,file_name,"404","Not found",
                         "Min Web Server couldn't find this file");
            return ;
        }
        //strcpy(cgi_args,other_request_info);
        process_post_method(fd,method,uri,file_name,cgi_args,other_request_info);

    }
    else if(strcasecmp(method,"head")==0)
    {
        get_file_type(file_name,file_type);
        process_head_method(fd,file_size,file_type);
        return;
    }
    else if(strcasecmp(method,"options")==0)
    {
        get_file_type(file_name,file_type);
        process_options_method(fd,file_type,file_size);
        return ;
    }
    else
    {
        send_error_to_client(fd,file_name,"501","Not Implement","Min Web Server can not support the request method");
        return;
    }
}

void read_request(rio_t *rp, char *other_request_info)
{
    char buf[MAXLINE];
    Rio_readlineb(rp,buf,MAXLINE);
    strcpy(other_request_info,buf);
    while(strcmp(buf,"\r\n"))
    {
        Rio_readlineb(rp,buf,MAXLINE);
        strcat(other_request_info,buf);
        printf("%s",buf);
    }
    return;
}

int get_the_uri(char *uri,char *file_name,char *cgi_args)
{
    char *ptr;
    if(!strstr(uri,"cgi-bin"))   //parse static content
    {
        strcpy(cgi_args,"");
        strcpy(file_name,".");
        strcat(file_name,uri);
        if(uri[strlen(uri)-1]=='/')
            strcat(file_name,"home.html");
        return 1;
    }
    else  //dynamic content
    {
        ptr=index(uri,'?');
        if(ptr)
        {
            strcpy(cgi_args,ptr+1);
            *ptr='\0';
        }
        else
            strcpy(cgi_args,"");
        strcpy(file_name,".");
        strcat(file_name,uri);
        return 0;
    }
}


void get_file_type(char *filename,char *filetype)
{
    if(strstr(filename,".html"))
        strcpy(filetype,"text/html");
    if(strstr(filename,".php"))
        strcpy(filetype,"text/php");
    if(strstr(filename,".py"))
        strcpy(filetype,"text/py");
    else if(strstr(filename,".gif"))
        strcpy(filetype,"image/gif");
    else if(strstr(filename,".jpg"))
        strcpy(filetype,"image/jpg");
    else if(strstr(filename,".jpeg"))
        strcpy(filetype,"image/jpeg");
    else if(strstr(filename,".mpg")||strstr(filename,".mp4"))
        strcpy(filetype,"video/mpg");
    else
        strcpy(filetype,"text/plain");

}

void read_html(int fd,char *file_name)
{
    char buf[1024];
    FILE *fp;
    char status[]="HTTP/1.0 200 OK\r\n";
    char header[] = "Server:Min Web Server\r\nContent-Type: text/html\r\n\r\n";
    Rio_writen(fd,status,strlen(status));
    Rio_writen(fd,status,strlen(header));
    fp=fopen(file_name,"r");
    if(fp==NULL)
    {
        send_error(fd);
        close(fd);
        return ;
    }
    Fgets(buf,sizeof(buf),fp);
    while(!feof(fp))
    {
        Rio_writen(fd,buf,strlen(buf));
        Fgets(buf,sizeof(buf),fp);

    }
    Fclose(fp);
    Close(fd);
}


void send_error_to_client(int fd, char *cause, char *status_code, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE],body[MAXLINE*4];
    sprintf(body,"<html><title>Min Web Server Error</title>");
    sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
    sprintf(body,"%s %s: %s\r\n",body,status_code,shortmsg);
    sprintf(body,"%s<p>%s: %s\r\n",body,longmsg,cause);
    sprintf(body,"%s<hr><em>The Min Web Server</em>\r\n",body);

    //HTTP respone
    sprintf(buf,"HTTP/1.0 %s %s\r\n",status_code,shortmsg);
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-type: text/html\r\n");
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-length: %d\r\n\r\n",(int)strlen(body));
    Rio_writen(fd,buf,strlen(buf));
    Rio_writen(fd,body,strlen(body));
}




