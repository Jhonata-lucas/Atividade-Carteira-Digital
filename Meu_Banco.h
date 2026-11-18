ASSINATURAS DAS FUN

#ifndef MEU_BANCO_H
#define MEU_BANCO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CONTAS 2
#define MAX_HISTORICO 5
#define TAM_NOME 50
#define ARQUIVO_BANCO "banco.dat"

#define CORRENTE 1
#define SALARIO 2
#define POUPANCA 3

typedef struct {
    int numero;
    int tipo;
    float saldo;
} Conta;

typedef struct {
    char tipo;              /* D = deposito, S = saque, E = emprestimo */
    float valor;
    int numeroConta;
    time_t dataHora;
} Transacao;

typedef struct {
    char cliente[TAM_NOME];
    Conta contas[MAX_CONTAS];
    int quantidadeContas;
    Transacao historico[MAX_HISTORICO];
    int quantidadeTransacoes;
    int emprestimoRealizado;
} Cliente;

typedef struct {
    Cliente *clientes;
    int quantidadeClientes;
    int proximoNumeroConta;
    int aberto;
} Banco;

/* Inicializacao, memoria e arquivos */
void inicializarBanco(Banco *banco);
void liberarBanco(Banco *banco);
int carregarBanco(Banco *banco, const char *nomeArquivo);
int salvarBanco(const Banco *banco, const char *nomeArquivo);

/* Menu e operacoes */
void abrirBanco(Banco *banco);
void fecharBanco(Banco *banco);
void cadastrarCliente(Banco *banco);
void depositar(Banco *banco);
void sacar(Banco *banco);
void solicitarEmprestimo(Banco *banco);
void consultarHistorico(const Banco *banco);
void mostrarRelatorios(const Banco *banco);
void realizarSorteio(const Banco *banco);
int proximoFibonacci(int anterior, int atual, int modulo);

#endif
