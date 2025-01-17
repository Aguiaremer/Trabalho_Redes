#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

void ler_dados(char *interface, char *ip, char *mascara)
{
	printf("\nDigite o nome da interface de rede (ex:enp0s3,wlan0): ");
	scanf("%s",interface);
	
	printf("\nDigite o endereco de IP (ex: 192.168.1.100): ");
	scanf("%s",ip);

	printf("\nDigite a mascara de Sub-rede (ex: 255.255.255.0) : ");
	scanf("%s",mascara);	
}

void receber_info(){
	//abre outro arquivo fifo para a leitura dos dados do app2
	int fd = open("fifo2",O_RDONLY);
        if (fd==-1){
                printf("Erro: nao foi possivel abrir o fifo");
        }

	//leitura da mensagem do app2
	char mensagem[100];
	read(fd,mensagem,sizeof(mensagem));
	printf("\n%s\n",mensagem);

	//fechando o arquivo fifo
	close(fd);
}

int main(int argc, char *argv[])
{
	// Chamada errada caso app1 nao tenha comandos
	if (argc<2){
		printf("Erro: Chamada da aplicacao incorreta\n");
		return -1;
	}
	
	char interface[20], ip[20], mascara[20];

	//cria o arquivo fifo para comunicacao
	if (mkfifo("fifo1",0777)==-1){
		if (errno != EEXIST){
			printf("Erro: Nao foi possivel criar o fifo\n");
			return -1;
		}	
	}

	//abre o arquivo para somente escrita
	int fd = open("fifo1",O_WRONLY);
	if (fd==-1){
		printf("Erro: nao foi possivel abrir o fifo");
	
	}
	
	//manda a informacao do comando para o app2
	write(fd,argv[1],strlen(argv[1])+1);

	//verifica qual comando foi selecionado
	if(strcmp(argv[1],"configurar_ip")==0){
		//chama a funcao para o usuario digitar as informacoes
		ler_dados(interface,ip,mascara);

		//manda as informacoes para o app2
		write(fd,interface,sizeof(interface));
	        write(fd,ip,sizeof(ip));
		write(fd,mascara,sizeof(mascara));

		printf("configuracoes enviadas\n");
	}
	else if(strcmp(argv[1],"requisitar_ip")==0){
		//o comando requistar precisa que envie a interface na chamada da funcao
		if(argc < 3){
			printf("Erro: informe o nome da interface\n");
			return -1;
		}

		//manda a interface para o app2
		write(fd,argv[2],sizeof(interface));

		printf("requisicao enviada\n");
	}
	else{
		printf("Erro: Comando invalido\n");
		return -1;
	}

	//fecha o arquivo fifo
	close (fd);
	
	//recebe os dados enviados pelo app2
	receber_info();

	return 0;
}



