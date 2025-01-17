# Processo-Seletivo-Venko-Networks

Esse é um repositório criado para a resolução da avaliação técnica do processo seletivo da Venko Networks.

## Como executar cada aplicação

### Aplicação 1 - Configuração e requisição de IP

Para executar a aplicação, utilize o terminal e execute o programa seguido de um dos comandos:

1. **Requisitar IP**
   Para requisitar o IP de uma interface específica, execute o comando informando o nome da interface, por exemplo:

   ```bash
   ./app1 requisitar_ip eth0
   ```

2. **Configurar IP**
   Para configurar um IP na interface, execute o comando abaixo:

   ```bash
   ./app1 configurar_ip
   ```

   O programa solicitará os dados necessários (endereço IP, máscara de sub-rede, etc.). Insira os dados conforme solicitado.

### Aplicação 2

  Para rodar a aplicação 2, execute o seguinte comando no terminal, mas **certifique-se de que a aplicação 1 já está em execução**:

```bash
   ./app2
   ```


## Método de comunicação utilizado

O método de comunicação utilizado entre as aplicações foi **pipes nomeados (FIFO)**.

## Possíveis melhorias futuras

- **Melhorias na comunicação:** É possível melhorar a comunicação entre as aplicações para torná-la mais robusta, utilizando outras técnicas de IPC (Inter-Process Communication) ou otimizações nas filas FIFO.
  
- **Melhoria na utilização de sockets:** A utilização de sockets pode ser otimizada para tornar a comunicação mais eficiente e flexível.
