#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

caddr_t addr; //mmap된 데이터
char* filename;//mmap용 파일

void cli();
void device_off();
void control_off(int num);
void colorControl(int num);
void colorChange();

void device_off()//전원을 해제할 조명 장치 선택
{
    int run=1;
    while(run)
    {
        int select=0;
        printf("전원을 해제할 조명의 번호를 입력하세요.(음수 또는 0 입력 시 전원 Off 종료):");
        scanf("%d",&select);
        if(select<=0)//기능 탈출
        {
            run=0;
        }
        else if(strlen(addr)>=select*11)//입력한 번호에 해당하는 조명 장치가 있을 때
        {
            control_off(select);
        }
        
        else//입력한 번호에 해당하는 조명 장치가 없을 때
        {
            printf("해당 번호에 대응되는 조명이 없습니다.\n");
        }
    }
}

void control_off(int num)//선택한 번호에 대한 조명 장치 전원 해제
{
    if(addr[(num-1)*11]=='0')//해당 조명 장치 전원이 이미 꺼져 있을 때
    {
        printf("%d번 조명은 이미 꺼져 있습니다.\n",num);
        return;
    }
    addr[(num-1)*11]='0';
    printf("%d번 조명을 해제했습니다.\n",num);
}

void colorChange()//색상을 변경할 조명 장치 선택
{
    int run=1;
    
    while(run)
    {
        int select=0;
        printf("색상을 변경할 조명의 번호를 입력하세요.(음수 또는 0 입력 시 색상 변경 종료):");
        scanf("%d",&select);
        if(select<=0)//기능 탈출
        {
            run=0;
        }
        else if(strlen(addr)>=select*11)//해당 번호의 조명 장치가 있을 때
        {
            colorControl(select);
        }
        
        else //해당 번호의 조명 장치가 없을 때
        {
            printf("해당 번호에 대응하는 조명이 없습니다.\n");
        }
    }

}

void colorControl(int num)//선택된 조명에 대한 RGB 값 설정 후 해당 장치에 색상 변경 요청
{
    if(addr[(num-1)*11]=='0')
    {
        printf("해당 조명은 전원이 꺼져 있습니다.\n");
        return;
    }
    int r,g,b;
    printf("%d번 조명의 Red 값을 입력하세요(범위 : 0~255):");
    scanf("%d",&r);
    if(r<0||r>255)
    {
        printf("잘못된 값입니다.\n");
        return;
    }
    printf("%d번 조명의 Green 값을 입력하세요(범위 : 0~255):");
    scanf("%d",&g);
    if(g<0||g>255)
    {
        printf("잘못된 값입니다.\n");
        return;
    }
    printf("%d번 조명의 Blue 값을 입력하세요(범위 : 0~255):");
    scanf("%d",&b);
    if(b<0||b>255)
    {
        printf("잘못된 값입니다.처음부터 다시 입력해주세요.\n");
        return;
    }
    char rgb[10];
    char red[4];
    char green[4];
    char blue[4];
    
    //입력받은 색상 값 세자리 문자열로 변경
    sprintf(red,"%03d",r);
    sprintf(green,"%03d",g);
    sprintf(blue,"%03d",b);

    strcpy(rgb,red);
    strcat(rgb,green);
    strcat(rgb,blue);

    //해당 디바이스의 RGB 정보 대입
    for(int i=0;i<10;i++)
    {
        addr[(num-1)*11+i+1]=rgb[i];
    }

    addr[(num-1)*11+10]='1';//해당 장치에 색상 변경 요청
}


void cli()
{
    int run=1;
    while(run)
    {
        int select=0;
        printf("===================\n");
        printf("무대 조명 제어 시스템\n");
        printf("1.조명 전원 해제 2.조명 색상 변경 3.시스템 종료\n");
        scanf("%d",&select);
        switch(select)
        {

            case 1:
            device_off();
            break;

            case 2:
            colorChange();
            break;
            
            case 3:
            run=0;
            break;

            default:
            printf("잘못된 옵션입니다.\n");
            break;
        }
        
    }
}




int main(int argc,char *argv[])
{
    int fd;
    struct stat statbuf;

    if(argc!=2)//인자를 잘못 받았을 때
    {
        fprintf(stderr,"Usage : %s (filename)\n",argv[0]);
        exit(1);
    }

    if(stat(argv[1],&statbuf)==-1) //파일의 정보를 읽기
    {
        perror("stat");
        exit(1);
    }

    if((fd=open(argv[1],O_RDWR))==-1) //mmap용 파일 열기
    {
        perror("open");
        exit(1);
    }
    

    addr=mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,(off_t)0);//mmap으로 파일을 메모리에 매핑
    if(addr==MAP_FAILED)//메모리 매핑 실패 시
    {
        perror("mmap");
        exit(1);
    }
    close(fd);//파일 닫기
    filename=(char*)malloc(sizeof(argv[1]));
    strcpy(filename,argv[1]);//인자로 받은 파일 이름을 전역 변수로 저장
    cli();//사용자 인터페이스 출력
}

