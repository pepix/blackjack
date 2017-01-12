/*
 * Blackjack
 * 	
 * 	gcc -lm -o servant servant.c
 *
 *	<Server>  $./servant s
 *	<Client>  $./servant c
 */


#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>

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

void server(int argc, char *argv[])
{
	int sock, socke;
	struct addrinfo hints, *res;
	FILE *fp;
	int err;
	char port[] = "10001";
	struct sockaddr addr;
	socklen_t	addrlen;
	int optval = 1;
	char buf[BUFSIZE];
	
	printf("◇♤♡♧\n");	
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if((err = getaddrinfo(NULL, port, &hints, &res)) != 0)
	{
		printf("getaddrinfo: %s\n", gai_strerror(err));
		exit(1);
	}
	sock = socket(res->ai_family, res->ai_socktype, 0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(optval));
	if(bind(sock, res->ai_addr, res->ai_addrlen) != 0)
	{
		perror("bind");
		exit(1);
	}
	freeaddrinfo(res);

	if(listen(sock, 5) != 0)
	{
		perror("listen");
		exit(1);
	}
	
	addrlen = sizeof(addr);
	while(1)
	{
		socke = accept(sock, &addr, &addrlen);
		if(socke < 0){
			perror("accept");
			exit(1);
		}
		if((fp = fdopen(socke, "r+")) == NULL)
		{
			perror("fdopen");
			exit(1);
		}
		
		//Draw two cards	
		int g,r;
		srand((unsigned int)time(NULL));
		int i;
		for(i = 0; i < 2; ++i)
		{
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
			fgets(buf, BUFSIZE, fp);
			//Hit
			if(atoi(buf) == 1 )
			{
				//printf("Hit\n");
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
			//Stand
			else if(atoi(buf) == 2)
			{
				//printf("Stand\n");
			}	
		}
		while (atoi(buf) == 1);

		close(socke);
	}
	close(sock);
}


void client(int argc, char *argv[])
{
	int sock;
	struct addrinfo hints, *res;
	FILE *fp;
	char buf[BUFSIZE];
	char req[BUFSIZE];
	int err;
	char port[] = "10001";

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if((err=getaddrinfo(NULL,port,&hints,&res)) != 0)
	{
		printf("getaddrinfo: %s\n", gai_strerror(err));
		exit(1);
	}

	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}
	
	if(connect(sock, res->ai_addr, res->ai_addrlen)!=0)
	{
		perror("connect");
		exit(1);
	}
	freeaddrinfo(res);
		
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
			printf("\nHit(1) or Stand(2)? : ");
			scanf("%d", &a);
			fprintf(fp, "%d\n", a);
			fflush(fp);
			if(a == 2)
			{
				int d;
				srand((unsigned int)time(NULL));
				d = 16 + rand()%5;
				//printf("You \t: %d\n", sum);
				//printf("Dealer \t: %d\n", d);

				if (ace)
				{
					int (*fl[3])(int, int) = {gt, eq, lt};
					char * str[3] = {"YOU WIN","DRAW","YOU LOSE"};
					int f;
					printf("\nDealer \t: %d\n", d);
					printf("You \t: ");
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
								printf("\n%s!!\n", str[f]);	
								goto loopend;
							}
						}
					}
					loopend:;
					break;
				}
				else
				{
					printf("\nYou \t: %d\n", sum);
					printf("Dealer \t: %d\n", d);
					if( sum == d ) printf("DRAW!!\n");
					else if( sum > d ) printf("YOU WIN!!\n");
					else if( sum < d ) printf("YOU LOSE!!\n");
				}
				break;
			}		
		}
		if (fgets(buf, BUFSIZE, fp) != NULL)
		{
			int r,s;
			char rank[] = "\0";
			char suit[] = "\0"; 
			//sscanf(buf, "%d %d", &n, &s);
			sscanf(buf, "%d %d", &r, &s);
			r = r + 1;
			sprintf(rank, "%d", r);
			sprintf(suit, "%d", s);
			switch(r)
			{
				case 1:  sprintf(rank, "A"); break;
				case 11: sprintf(rank, "J"); break;
				case 12: sprintf(rank, "Q"); break;
				case 13: sprintf(rank, "K"); break;
			}
			switch(s)
			{
				case 0:  sprintf(suit, "♠"); break;
				case 1:  sprintf(suit, "♡"); break;
				case 2:  sprintf(suit, "♢"); break;
				case 3:  sprintf(suit, "♣"); break;
				default: sprintf(suit, "not found");
			}
			printf("%s %s\t", suit, rank);
			if (r >= 11)
			{
				sum += 10;
			}
			else if (r == 1)
			{
				++ace;
			}
			else
			{
				sum += r;
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
						printf("\nBLACK JACK!!!\n");
						printf("YOU WIN!!\n");
						goto loopend2;
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
			printf("\nBLACK JACK!!!\n");
			printf("YOU WIN!!\n");
			break;
		}
	}
	loopend2:;
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




int main(int argc, char *argv[])
{
	if (argv[1][0] == 's')
	{
		server(argc, argv);
	}
	else if (argv[1][0] == 'c')
	{
		client(argc, argv);
	}
}

