/*
 * 単純なTCP Server （現在時刻送信）
 * 	./server
 *
  	gcc -o server server_time.c
 */


#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<stdlib.h>
#include<time.h>
#include <math.h>

#define BUFSIZE 256
int trump[13][4] = {};

int lt(int x, int y)
{
	return x < y;
}

int gt(int x, int y)
{
	return x > y;
}

int eq(int x, int y)
{
	return x == y;
}

void server(int argc, char *argv[]){
	int sock, socke;	//ソケット
	struct addrinfo hints, *res;	//getaddrinfo用
	FILE *fp;
	int err;
	char port[] = "10001";		//ポート番号
	struct sockaddr addr;		//accept用
	socklen_t	addrlen;	//accept用
	int optval = 1;			//setsockopt用
	char buf[BUFSIZE];
	
	//サーバ情報
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;	//IPv4
	hints.ai_socktype = SOCK_STREAM; //TCP
	hints.ai_flags = AI_PASSIVE; 	//server
	if((err = getaddrinfo(NULL, port, &hints, &res)) != 0){
		printf("getaddrinfo: %s\n", gai_strerror(err));
		exit(1);
	}
	sock = socket(res->ai_family, res->ai_socktype, 0); //ソケット作成
	if(sock < 0){
		perror("socket");
		exit(1);
	}

	//ポートを再利用
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(optval));
	//ソケットにアドレス、ポート番号を結びつける
	if(bind(sock, res->ai_addr, res->ai_addrlen) != 0){
		perror("bind");
		exit(1);
	}
	freeaddrinfo(res);	//メモリ解放

	//クライアントからの接続要求待機（接続要求を保留できる数）
	if(listen(sock, 5) != 0){
		perror("listen");
		exit(1);
	}
	
	/*
 		１クライアント毎の接続とデータ送受信
	*/
	addrlen = sizeof(addr);
	while(1)
	{
		//クライアントからの接続要求受付
		socke = accept(sock, &addr, &addrlen);
		if(socke < 0){
			perror("accept");
			exit(1);
		}
		//入出力をFILEに変更
		if((fp = fdopen(socke, "r+")) == NULL){
			perror("fdopen");
			exit(1);
		}
		
		//Draw two cards	
		int g,r;
		srand((unsigned int)time(NULL));
		int i;
		for(i = 0; i < 2; ++i){
			do
			{
				g = rand()%13;
				r = rand()%4;
			}
			while (trump[g][r] != 0);
			trump[g][r] = 1;
			fprintf(fp, "%d %d\n", g, r);
			fflush(fp);
		}

		//Hit or Stand?
		do
		{
			printf("%s", fgets(buf, BUFSIZE, fp));

			//Hit
			printf("%d", atoi(buf));
			if(atoi(buf) == 1 )
			{
				do
				{
					g = rand()%13;
					r = rand()%4;
				}
				while(trump[g][r] != 0);
				trump[g][r] = 1;
				fprintf(fp, "%d %d\n", g, r);
				fflush(fp);
			}
			else if(atoi(buf) == 2)
			{
				//stand
				
			}	
		}
		while (atoi(buf) == 1);

		close(socke);
	}
	close(sock);
}

void client(int argc, char *argv[]){
	int sock;
	struct addrinfo hints, *res;
	FILE *fp;
	char buf[BUFSIZE];
	char req[BUFSIZE];
	int err;
	char port[] = "10001";

	//IPadd変換、socket作成、connect
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if((err=getaddrinfo(NULL,port,&hints,&res)) != 0){

		printf("getaddrinfo: %s\n", gai_strerror(err));
		exit(1);
	}

	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sock < 0){
		perror("socket");
		exit(1);
	}
	
	//接続
	if(connect(sock, res->ai_addr, res->ai_addrlen)!=0){
		perror("connect");
		exit(1);
	}
	freeaddrinfo(res); //メモリ解放
		
	fp=fdopen(sock,"r+");

	int sum = 0;
	int i;
	int ace = 0;
	int a;
	//int phase = 0;

	for (i = 0; ; ++i)
	{
		if (i >= 2)
		{
			printf("Hit(1) or Stand(2)? : ");
			scanf("%d", &a);
			fprintf(fp, "%d\n", a);
			fflush(fp);
			if(a == 2)
			{
				int d;
				srand((unsigned int)time(NULL));
				d = 16 + rand()%5;
				printf("You : %d\n", sum);
				printf("Dealer : %d\n", d);

				if (ace)
				{
					int (*fl[3])(int, int) = {gt, eq, lt};
					char * str[3] = {"WIN","DRAW","LOSE"};
					int f;
					for (f = 0; f < sizeof(fl); ++f)
					{
						int i;
						for (i = 0; i < pow(2, ace); ++i)
						{
							int t = 0;
							int j;
							for (j = 0; j < ace; ++j)
							{
								t += (i & (1 << j)) ? 11 : 1;
							}
							printf("[%d]", sum + t);
							if(fl[f](sum + t, d))
							{
								puts(str[f]);
								goto loopend;
							}
						}
					}
					loopend:;
					break;
				}
				else
				{
					if( sum == d ) printf("DRAW!!\n");
					else if( sum > d ) printf("YOU WIN!!\n");
					else if( sum < d ) printf("YOU LOSE!!\n");
				}
				break;
			}		
		}
		if (fgets(buf, BUFSIZE, fp) != NULL)
		{
			printf("%s", buf);
			int n,s;
			sscanf(buf, "%d %d", &n, &s);
			n += 1;
			if (n >= 11)
			{
				sum += 10;
			}
			else if (n == 1)
			{
				++ace;
			}
			else
			{
				sum += n;
			}
			//Ace
			if (ace)
			{
				int i;
				for (i = 0; i < pow(2, ace); ++i)
				{
					int t = 0;
					int j;
					for (j = 0; j < ace; ++j)
					{
						t += (i & (1 << j)) ? 11 : 1;
					}
					printf("[%d]", sum + t);
					if ( (sum + t) == 21 )
					{
						printf("BLACK JACK!!!\n");
						break;
					}
				}
				printf("\n");
			}
			else
			{
				printf("[%d]\n", sum);
			}
		}
		if ( sum > 21 )
		{
			printf("GAME OVER\n");
			break;
		}
		else if ( sum == 21 )
		{
			printf("BLACK JACK!!!\n");
			break;
		}
	}
	//while(1){
	//	if( phase == 0 ){
	//	
	//	for (i = 0; i < 2; ++i)
	//	{
	//		if (fgets(buf, BUFSIZE,fp) != NULL)
	//		{
	//			printf("%s",buf);
	//			int n,s;
	//			sscanf(buf, "%d %d", &n, &s);
	//			n += 1;
	//			if (n >= 11)
	//			{
	//				sum+=10;
	//			}
	//			else if (n == 1)
	//			{
	//				//if(21 >=  sum + 11)
	//				//{
	//				//	sum+=11;
	//				//	ace
	//				//}
	//				//else
	//				//{
	//				//	sum+=1;
	//				//}
	//				++ace;
	//			}
	//			else
	//			{
	//				sum+=n;
	//			}
	//			//Ace
	//			for(i=0; i < pow(2, ace); ++i)
	//			{
	//				int t = 0;
	//				int j;
	//				for(j=0; j < ace; ++j)
	//				{
	//					t += 1 + 10 * (i & (1 << j));
	//				}
	//				printf("sum=%d | ", sum + t);
	//			}
	//				
	//			printf("%d\n", sum);
	//			
	//		}
	//		phase = 1;
	//	}
	//	}else{
	//		printf("Hit(1) or Stand(2)? : ");
	//		scanf("%d", &a);
	//		fprintf(fp, "%d\n", a);
	//		fflush(fp);
	//		
	//		if(a==2){
	//			int d;
	//			srand((unsigned int)time(NULL));
	//			d = 16 + rand()%5;
	//			printf("You : %d\n", sum);
	//			printf("Dealer : %d\n", d);
	//			if( sum == d ) printf("DRAW!!\n");
	//			else if( sum > d ) printf("YOU WIN!!\n");
	//			else if( sum < d ) printf("YOU LOSE!!\n");
	//			break;
	//		}		

	//		printf("%s", fgets(buf, BUFSIZE, fp));
	//		int n, s;
	//		sscanf(buf, "%d %d", &n, &s);
	//		n += 1;
	//		if( n>=11 ) n = 10;
	//		sum += n;
	//		printf("sum=%d\n", sum);
	//			
	//		for(i=0; i < pow(2, ace); ++i)
	//		{
	//			int t = 0;
	//			int j;
	//			for(j=0; j < ace; ++j)
	//			{
	//				t += 1 + 10 * (i & (1 << j));
	//			}
	//			printf("sum=%d | ", sum + t);
	//		}
	//		if(sum>21){
	//			printf("GAME OVER\n");
	//			break;
	//		}else if(sum==21){
	//			printf("BLACK JACK!!!\n");
	//			break;
	//		}
	//	}
	//}
	

	close(sock);
}




int main(int argc, char *argv[]){
	if (argv[1][0] == 's')
	{
		server(argc, argv);
	}
	else if (argv[1][0] == 'c')
	{
		client(argc, argv);
	}
}

