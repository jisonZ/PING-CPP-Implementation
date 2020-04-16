// C program to Implement Ping

// compile as -o ping
// run as sudo ./ping <hostname>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
//#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include "ip_icmp.h"
#include <vector>
#include <queue>
#include <cmath>
// Define the Packet Constants
// ping packet size
#define PING_PKT_S 64

// Automatic port number
#define PORT_NO 0

// Automatic port number
#define PING_SLEEP_RATE 1000000

// Gives the timeout delay for receiving packets
// in seconds
#define RECV_TIMEOUT 1

// Define the Ping Loop
int pingloop=1;

// ping packet structure
struct ping_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKT_S-sizeof(struct icmphdr)];
};

void intHandler(int dummy)
{
    pingloop=0;
}

// Calculating the Check Sum
unsigned short checksum(void *b, int len)
{
    unsigned short *buf = reinterpret_cast<unsigned short *>(b);
//    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con);
char* reverse_dns_lookup(char *ip_addr);
void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr,
               char *ping_dom, char *ping_ip, char *rev_host, char* print_stat);

int main(int argc, char *argv[])
{
    int sockfd;
    char *ip_addr, *reverse_hostname;
    struct sockaddr_in addr_con;
//    int addrlen = sizeof(addr_con);
//    char net_buf[NI_MAXHOST];

    if(argc !=3 || (strcmp(argv[2], "y") != 0 && strcmp(argv[2], "n") != 0))
    {
        printf("Check Argument Format: \n");
        printf("%s <address> y/n\n", argv[0]);
        printf("Please enter hostname or IP address for <address> and enter y or n for printing RTT stat results \n");
        return 0;
    }
    
// Take a hostname as an input, extract IP
    ip_addr = dns_lookup(argv[1], &addr_con);
    if(ip_addr==NULL)
    {
        printf("\nDNS lookup failed! Could not resolve hostname!\n");
        return 0;
    }

    reverse_hostname = reverse_dns_lookup(ip_addr);
    printf("\nTrying to connect to '%s' IP: %s\n",
                                    argv[1], ip_addr);
    printf("\nReverse Lookup domain: %s",
                        reverse_hostname);

    //socket()
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sockfd<0)
    {
        printf("\nSocket file descriptor not received!!\n");
        return 0;
    }
    else
        printf("\nSocket file descriptor %d received\n", sockfd);

    signal(SIGINT, intHandler);//catching interrupt

    //send pings continuously
    send_ping(sockfd, &addr_con, reverse_hostname,
                                ip_addr, argv[1], argv[2]);
    
    return 0;
}

// make a ping request
void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr,
               char *ping_dom, char *ping_ip, char *rev_host, char* print_stat)
{
    int ttl_val=64, msg_count=0, i, flag=1,
            msg_received_count=0;
        
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timespec time_start, time_end, tfs, tfe;
    long double rtt_msec=0, total_msec=0;
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tfs);

    // set socket options at ip to TTL and value to 64,
    // change to what you want by setting ttl_val
//    SOL_SOCKET or IPPROTO_IP
    
    if (setsockopt(ping_sockfd, IPPROTO_IP, IP_TTL,
            &ttl_val, sizeof(ttl_val)) != 0)
    {
        printf("\nSetting socket options to TTL failed!\n");
        return;
    }
    else
    {
        printf("\nSocket set to TTL..\n");
    }

    // setting timeout of recv setting
    setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO,
                (const char*)&tv_out, sizeof tv_out);

    std::priority_queue<long double, std::vector<long double>, std::greater<long double> > min;
    std::priority_queue<long double, std::vector<long double>, std::less<long double> > max;
    long double time_sum = 0;
    std::vector<long double> time_track;
    // send icmp packet in an infinite loop
    while(pingloop)
    {
        // flag is whether packet was sent or not
        flag=1;
    
        //filling packet
        bzero(&pckt, sizeof(pckt));
        
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();
        
        for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
            pckt.msg[i] = i+'0';
        
        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
        

        usleep(PING_SLEEP_RATE);

        //send packet
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        if ( sendto(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*) ping_addr,
            sizeof(*ping_addr)) <= 0)
        {
            printf("\nPacket Sending Failed!\n");
            flag=0;
        }

        //receive packet
        unsigned int addr_len = (unsigned int)sizeof((struct sockaddr*)&r_addr);

        if ( recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0,
            (struct sockaddr*)&r_addr, &addr_len) <= 0
            && msg_count>1)
        {
            printf("\nPacket receive failed!\n");
        }
        else
        {
            clock_gettime(CLOCK_MONOTONIC, &time_end);
            
            double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;
            rtt_msec = (time_end.tv_sec-
                        time_start.tv_sec) * 1000.0
                        + timeElapsed;
            
            time_sum += rtt_msec;
            max.push(rtt_msec);
            min.push(rtt_msec);
            time_track.push_back(rtt_msec);
            
            // if packet was not sent, don't receive
            if(flag)
            {
                if(!(pckt.hdr.type ==69 && pckt.hdr.code==0))
                {
                    printf("Error..Packet received with ICMP type %d code %d\n",
                           pckt.hdr.type, pckt.hdr.code);
                }
                else
                {
                    printf("%d bytes from %s (h: %s) (%s) msg_seq=%d ttl=%d rtt = %Lf ms.\n", PING_PKT_S, ping_dom, rev_host, ping_ip, msg_count, ttl_val, rtt_msec);

                    msg_received_count++;
                }
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &tfe);
    double timeElapsed = ((double)(tfe.tv_nsec -
                        tfs.tv_nsec))/1000000.0;
    
    total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0+
    timeElapsed;
                    
    printf("\n===%s ping statistics===\n", ping_ip);
    printf("\n%d packets sent, %d packets received, %f percent packet loss. Total time: %Lf ms.", msg_count, msg_received_count, ((msg_count - msg_received_count)/msg_count) * 100.0, total_msec);
    if (strcmp(print_stat, "y") == 0)
    {
        long double mean = time_sum/time_track.size();
        int variance = 0;
        for (int i = 0; i < time_track.size(); ++i)
        {
            variance += pow(time_track[i] - mean, 2);
        }
        variance = variance/time_track.size();
        
        printf("\nround-trip min/avg/max/stddev = %Lf / %Lf / %Lf / %Lf ms. \n",
               min.top(), max.top(), mean, (long double)sqrt(variance));
    }
}

char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con)
{
    printf("\nResolving DNS..\n");
    struct hostent *host_entity;
    char *ip=(char*)malloc(NI_MAXHOST*sizeof(char));

    if ((host_entity = gethostbyname(addr_host)) == NULL)
    {
        // No ip found for hostname
        return NULL;
    }
    
    //filling up address structure
    strcpy(ip, inet_ntoa(*(struct in_addr *)
                        host_entity->h_addr));

    (*addr_con).sin_family = host_entity->h_addrtype;
    (*addr_con).sin_port = htons (PORT_NO);
    (*addr_con).sin_addr.s_addr = *(long*)host_entity->h_addr;

    return ip;
}

char* reverse_dns_lookup(char *ip_addr)
{
    struct sockaddr_in temp_addr;
    socklen_t len;
    char buf[NI_MAXHOST], *ret_buf;
  
    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
    len = sizeof(struct sockaddr_in);
  
    if (getnameinfo((struct sockaddr *) &temp_addr, len, buf,
                    sizeof(buf), NULL, 0, NI_NAMEREQD))
    {
        printf("Could not resolve reverse lookup of hostname\n");
        return NULL;
    }
    ret_buf = (char*)malloc((strlen(buf) +1)*sizeof(char) );
    strcpy(ret_buf, buf);
    return ret_buf;
}

