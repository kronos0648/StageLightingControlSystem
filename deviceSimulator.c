#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


caddr_t addr;//mmap된 데이터
int device_num;//현재 조명 디바이스의 식별 번호
int red;//현재 조명 디바이스의 RGB 중 Red 값
int green;//현재 조명 디바이스의 RGB 중 Green 값
int blue;//현재 조명 디바이스의 RGB 중 Blue 값

void device_off();
void setRGB();
void listenRequest();

void device_off()//제어 시스템에서 현재 조명 디바이스의 종료를 요청할 때 종료
{
    printf("제어 시스템에서 %d번 디바이스의 전원을 종료했습니다.\n",device_num);
    exit(1);
}

void setRGB()//현재 조명 디바이스의 색상 설정
{
    char r[4];
    char g[4];
    char b[4];

    //RGB 중 R 값 mmap된 메모리에서 가져오기
    for(int i=0;i<3;i++)
    {
        r[i]=addr[(device_num-1)*11+i+1];
    }
    r[4]='\0';
    red=atoi(r);

    //RGB 중 G 값 mmap된 메모리에서 가져오기
    for(int i=0;i<3;i++)
    {
        g[i]=addr[(device_num-1)*11+i+4];
    }
    g[4]='\0';
    green=atoi(g);

    //RGB 중 B 값 mmap된 메모리에서 가져오기
    for(int i=0;i<3;i++)
    {
        b[i]=addr[(device_num-1)*11+i+7];
    }
    b[4]='\0';
    blue=atoi(b);

    printf("현재 %d번 조명 디바이스 색상 RGB 수치\n",device_num);
    printf("Red : %d\n",red);
    printf("Green : %d\n",green);
    printf("Blue : %d\n",blue);
    addr[(device_num-1)*11+10]='0';//요청 수신 플래그 해제
}

void listenRequest()//제어 시스템의 색상 변경 요청을 수신
{
    while(addr[(device_num-1)*11]=='1')//제어 시스템에서 전원 해제 요청을 하기 전까지 반복
    {
        if(addr[(device_num-1)*11+10]=='1')//색상 변경 요청을 받을 때
        {
            printf("제어 시스템이 %d번 디바이스에 색상 변경을 요청했습니다.\n",device_num);
            setRGB();//색상 변경
            printf("색상 변경을 완료했습니다.\n");
        }
    }
}

int main(int argc,char* argv[])
{
    int fd;
    struct stat statbuf;

    if(argc!=3)//매개변수를 잘못 받았을 때
    {
        fprintf(stderr,"Usage : %s (filename) (device number)\n",argv[0]);
        exit(1);
    }

    if((device_num=atoi(argv[2]))<=0)//디바이스 번호를 잘못 입력했을 때
    {
        printf("디바이스 번호는 1번부터 허용됩니다.\n");
        exit(1);
    }


    if(stat(argv[1],&statbuf)==-1)//파일의 정보 읽기
    {
        perror("stat");
        exit(1);
    }

    if((fd=open(argv[1],O_RDWR))==-1)//파일 열기
    {
        perror("open");
        exit(1);
    }
    
    //mmap으로 파일을 메모리 매핑
    addr=mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,(off_t)0);
    if(addr==MAP_FAILED)//메모리 매핑 실패 시
    {
        perror("mmap");
        exit(1);
    }
    close(fd);//파일 닫기

    if(strlen(addr)<device_num*11)//입력된 식별 번호에 해당하는 디바이스가 존재하지 않을 때
    {
        printf("%d번 디바이스에 대한 정보를 'File : %s'에서 읽지 못 했습니다.\n",device_num,argv[1]);
        exit(1);
    }

    if(addr[(device_num-1)*11]=='1')//입력된 식별 번호에 해당하는 디바이스가 이미 켜져 있을 때
    {
        printf("%d번 조명 디바이스는 이미 전원이 켜져 있습니다.\n",device_num);
        exit(1);
    }

    addr[(device_num-1)*11]='1'; //조명의 전원 상태를 On으로 변경

    printf("%d번 조명 디바이스 셋업 완료\n",device_num);
    setRGB(); //전원을 키고 현재 저장된 색상 옵션으로 조명 설정
    listenRequest(); //제어 시스템의 요청을 듣기 시작
    device_off();//요청 수신을 멈출 때 조명 디바이스 전원 해제
}