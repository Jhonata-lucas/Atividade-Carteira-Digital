#include "meu_banco.h"

int main(void) {
    Banco banco;
    int opcao;

    inicializarBanco(&banco);

    do {
        printf("\n========== BANCO DIGITAL ==========\n");
        printf("Estado: %s\n", banco.aberto ? "ABERTO" : "FECHADO");
        printf("1 - Abrir banco\n");
        printf("2 - Consultar historico do banco\n");
        printf("3 - Efetuar sorteio\n");
        printf("4 - Cadastrar cliente (banqueiro)\n");
        printf("5 - Acessar conta (cliente)\n");
        printf("6 - Fechar banco\n");
        printf("0 - Sair\n");
        printf("Opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: abrirBanco(&banco); break;
            case 2: consultarHistoricoBanco(&banco); break;
            case 3: realizarSorteio(&banco); break;
            case 4: cadastrarCliente(&banco); break;
            case 5: acessarCliente(&banco); break;
            case 6: fecharBanco(&banco); break;
            case 0: salvarBanco(&banco); break;
            default: printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    liberarBanco(&banco);
    printf("Sistema encerrado.\n");
    return 0;
}

