#ifndef MEU_BANCO_H
#define MEU_BANCO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CORRENTE 1
#define SALARIO 2
#define POUPANCA 3
#define MAX_CONTAS 2
#define MAX_HISTORICO 5
#define ARQUIVO_BANCO "banco.dat"

typedef struct {
    int numero;
    int tipo;
    float saldo;
} Conta;

typedef struct {
    char operacao;             /* D, S ou E */
    float valor;
    int numero;
    int tipo;
    time_t data;
} Transacao;

typedef struct {
    char nome[50];
    Conta contas[MAX_CONTAS];
    int quantidadeContas;
    int fezEmprestimo;
    int possuiTitulo;
    Transacao historico[MAX_HISTORICO];
    int quantidadeTransacoes;
} Cliente;

typedef struct {
    Cliente *clientes;
    int quantidadeClientes;
    int proximoNumeroConta;
    int aberto;
} Banco;

void inicializarBanco(Banco *banco);
int salvarBanco(Banco *banco);
void liberarBanco(Banco *banco);
void abrirBanco(Banco *banco);
void fecharBanco(Banco *banco);
void cadastrarCliente(Banco *banco);
void consultarHistoricoBanco(Banco *banco);
void realizarSorteio(Banco *banco);
void acessarCliente(Banco *banco);

#endif

