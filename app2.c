#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

int validar_ip(char *ip)
{
	int i = 0, num, ponto_count = 0;
	char temp[4];
 	int len = strlen(ip);
	
	//um ip no formato correto deve ter entre 7 a 15 caracteres ex(0.0.0.0 ou 255.255.255.255)
   	if (len < 7 || len > 15) {
        	return 0;
    	}


    	while (i < len) {
        	int j = 0;
		//dividindo o ip pelos pontos
        	while (i < len && ip[i] != '.') {
			// se nao for um digito e invalido
            		if (!isdigit(ip[i])) {
                		return 0;
            		}
            		temp[j++] = ip[i++];
        	}
        	temp[j] = '\0';

		//tranforma o caractere em numero e verifica se esta entre 0 a 255
        	num = atoi(temp);
        	if (num < 0 || num > 255) {
        		return 0;
        	}

		//conta os pontos
        	ponto_count++;
        	
		//se tiver mais de 3 pontos e invalido
        	if (ponto_count > 3 && i < len) {
        		return 0;
        	}

        	i++; 
    	}

	//retorna veredadeiro se tiver 3 pontos (aqui sao 4 pois o contador interpreta o final do IP como um ponto)
	return ponto_count == 4;
}

int validar_interface(char *interface) {
    	struct ifreq ifr;
	
	//abrir o socket
    	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (sockfd == -1) {
        	perror("Erro ao abrir socket");
        	return 0;
   	}

    	//copia o nome da interface com segurança
    	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    	//verifica se a interface é válida
    	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        	perror("Erro no ioctl (interface inválida ou problema de permissões)");
        	close(sockfd);
        	return 0;
    	}

    	close(sockfd);
    	return 1;
}
    	
int configurar_ip(char *interface, char *ip, char *mascara) {
    	struct ifreq ifr;
    	struct sockaddr_in sin;
    	int sockfd;

    	//abrir o socket
    	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (sockfd == -1) {
        	perror("Erro ao abrir socket");
        	return 0;
    	}

    	//copiar o nome da interface
    	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    	//configurar IP
    	sin.sin_family = AF_INET;
    	if (inet_pton(AF_INET, ip, &sin.sin_addr) <= 0) {
        	perror("Erro: Endereço IP inválido");
        	close(sockfd);
        	return 0;
    	}
    	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr_in));
    	if (ioctl(sockfd, SIOCSIFADDR, &ifr) == -1) {
        	perror("Erro ao configurar o IP");
        	close(sockfd);
        	return 0;
    	}

    	// Configurar máscara de sub-rede
    	if (inet_pton(AF_INET, mascara, &sin.sin_addr) <= 0) {
        	perror("Erro: Máscara de sub-rede inválida");
        	close(sockfd);
        	return 0;
    	}
    	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr_in));
    	if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) == -1) {
        	perror("Erro ao configurar a máscara de sub-rede");
        	close(sockfd);
        	return 0;
    	}

    	// Ativar a interface
    	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
        	perror("Erro ao obter flags da interface");
        	close(sockfd);
        	return 0;
    	}
    	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1) {
        	perror("Erro ao ativar a interface");
        	close(sockfd);
        	return 0;
    	}

    close(sockfd);
    return 1;
}

void obter_ip(char *interface, char *resultado) {
    	struct ifreq ifr;
    	struct sockaddr_in *ip_addr, *netmask_addr;

	//abrir o socket
    	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (sockfd == -1) {
        	perror("Erro ao abrir socket");
       		strcpy(resultado, "Erro ao abrir socket");
        	return;
    	}

	//pegar o endereco de ip
    	strncpy(ifr.ifr_name, interface, IFNAMSIZ);
    	if (ioctl(sockfd, SIOCGIFADDR, &ifr) == -1) {
        	perror("Erro ao obter IP");
        	strcpy(resultado, "Erro: Interface inválida ou sem IP configurado");
        	close(sockfd);
        	return;
    	}
    	ip_addr = (struct sockaddr_in *)&ifr.ifr_addr;

	//pegar a mascara de sub-rede
    	if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) == -1) {
        	perror("Erro ao obter máscara de sub-rede");
        	strcpy(resultado, "Erro ao obter máscara de sub-rede");
        	close(sockfd);
        	return;
    	}
    	netmask_addr = (struct sockaddr_in *)&ifr.ifr_netmask;

    	// Usando inet_ntoa diretamente nas sprintf para evitar o retorno de ponteiros estáticos
    	sprintf(resultado, "Interface: %s, Configured IP: %s, Netmask: %s",
            	interface,
            	inet_ntoa(ip_addr->sin_addr),
            	inet_ntoa(netmask_addr->sin_addr));

    	close(sockfd);
}


void receber_info(char *comando, char *interface, char *ip, char *mascara)
{
	//abre o aqruivo so para leitura
        int fd = open("fifo1", O_RDONLY);
        if (fd==-1){
                printf("Erro: nao foi possivel abrir o fifo");
        }
        read(fd,comando,20);
        printf("%s\n", comando);

        if (strcmp(comando,"configurar_ip")==0){
                read(fd,interface,20);
                read(fd,ip,20);
                read(fd,mascara,20);
		printf("interface: %s \nip: %s \nmascara: %s\n", interface, ip, mascara);
        }
        else if (strcmp(comando,"requisitar_ip")==0){
                read(fd,interface,20);
                printf("requisitando: %s\n",interface);
        }
        else{
                printf("Erro: comando invalido");
        }
        close(fd);
}


int main ()
{
	char comando[20], interface[20], ip[20], mascara[20];

	//le as informacoes do app1
	receber_info(comando,interface,ip,mascara);
	
	//cria um aqruivo fifo para mandar a mensagem para o app1
	if (mkfifo("fifo2",0777)==-1){
       		if (errno != EEXIST){
                        printf("Erro: Nao foi possivel criar o fifo\n");
                        return -1;
                }
        }	
	
	//abre o aqrivo so para escrita
	int fd = open("fifo2",O_WRONLY);
	if (fd==-1){
                printf("Erro: nao foi possivel abrir o fifo");
        }

	char mensagem[100];
	
	if (strcmp(comando,"configurar_ip")==0){
		if(validar_interface(interface)&&
		   validar_ip(ip)&&
		   validar_ip(mascara)&&
		   configurar_ip(interface,ip,mascara))//valida a interface, ip,mascara e configura o ip
		{
			strcpy(mensagem,"IP configurado com sucesso");	
		}
		else
		{
			strcpy(mensagem,"Erro ao tentar configurar o IP");
		}
	}
	else{
		if(validar_interface(interface)){
			char resultado[100];
        	        obter_ip(interface,resultado);
	                strcpy(mensagem,resultado);
		}
		else{
			strcpy(mensagem,"Interface nao pode ser validada");
		}
	}

	//manda a mensagem pro app1
	write(fd,mensagem,sizeof(mensagem));	
	close(fd);

	return 0;	
}
