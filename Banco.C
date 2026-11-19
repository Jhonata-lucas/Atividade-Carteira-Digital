#include "meu_banco.h"

/* Funcoes simples para localizar contas e calcular saldos. */
int indiceConta(Cliente *cliente, int tipo) {
    for (int i = 0; i < cliente->quantidadeContas; i++)
        if (cliente->contas[i].tipo == tipo) return i;
    return -1;
}

float saldoTotal(Cliente *cliente) {
    float total = 0;
    for (int i = 0; i < cliente->quantidadeContas; i++)
        total += cliente->contas[i].saldo;
    return total;
}

void mostrarConta(Conta *conta) {
    printf("%04d-%02d", conta->numero, conta->tipo);
}

void registrarTransacao(Cliente *cliente, Conta *conta, char op, float valor) {
    if (cliente->quantidadeTransacoes == MAX_HISTORICO) {
        for (int i = 1; i < MAX_HISTORICO; i++)
            cliente->historico[i - 1] = cliente->historico[i];
        cliente->quantidadeTransacoes--;
    }
    Transacao *t = &cliente->historico[cliente->quantidadeTransacoes++];
    t->operacao = op;
    t->valor = valor;
    t->numero = conta->numero;
    t->tipo = conta->tipo;
    t->data = time(NULL);
}

/* Arquivo binario: quantidade, proximo numero e vetor de clientes. */
void inicializarBanco(Banco *banco) {
    banco->clientes = NULL;
    banco->quantidadeClientes = 0;
    banco->proximoNumeroConta = 1000;
    banco->aberto = 0;

    FILE *arquivo = fopen(ARQUIVO_BANCO, "rb");
    if (arquivo == NULL) return;

    int quantidade, proximo;
    if (fread(&quantidade, sizeof(int), 1, arquivo) == 1 &&
        fread(&proximo, sizeof(int), 1, arquivo) == 1 &&
        quantidade >= 0 && quantidade <= 100000 && proximo >= 1000) {
        Cliente *dados = NULL;
        if (quantidade > 0) dados = malloc(quantidade * sizeof(Cliente));
        if (quantidade == 0 ||
            (dados != NULL && fread(dados, sizeof(Cliente), quantidade, arquivo) == (size_t)quantidade)) {
            banco->clientes = dados;
            banco->quantidadeClientes = quantidade;
            banco->proximoNumeroConta = proximo;
        } else free(dados);
    }
    fclose(arquivo);
}

int salvarBanco(Banco *banco) {
    FILE *arquivo = fopen(ARQUIVO_BANCO, "wb");
    if (arquivo == NULL) {
        printf("Nao foi possivel salvar o banco.\n");
        return 0;
    }
    int ok = fwrite(&banco->quantidadeClientes, sizeof(int), 1, arquivo) == 1 &&
             fwrite(&banco->proximoNumeroConta, sizeof(int), 1, arquivo) == 1 &&
             fwrite(banco->clientes, sizeof(Cliente), banco->quantidadeClientes, arquivo) ==
             (size_t)banco->quantidadeClientes;
    fclose(arquivo);
    if (!ok) printf("Erro ao gravar os dados.\n");
    return ok;
}

void liberarBanco(Banco *banco) {
    free(banco->clientes);
    banco->clientes = NULL;
}

void abrirBanco(Banco *banco) {
    banco->aberto = 1;
    printf("Banco aberto.\n");
}

void fecharBanco(Banco *banco) {
    banco->aberto = 0;
    salvarBanco(banco);
    printf("Banco fechado.\n");
}

void cadastrarCliente(Banco *banco) {
    if (!banco->aberto) {
        printf("Abra o banco primeiro.\n");
        return;
    }

    Cliente novo = {0};
    int tipos[2];
    printf("Nome do cliente: ");
    scanf(" %49[^\n]", novo.nome);
    printf("Quantidade de contas (1 ou 2): ");
    scanf("%d", &novo.quantidadeContas);
    if (novo.quantidadeContas < 1 || novo.quantidadeContas > 2) {
        printf("Quantidade invalida.\n");
        return;
    }

    for (int i = 0; i < novo.quantidadeContas; i++) {
        printf("Tipo da conta %d (1-Corrente, 2-Salario, 3-Poupanca): ", i + 1);
        scanf("%d", &tipos[i]);
        if (tipos[i] < 1 || tipos[i] > 3 ||
            (i == 1 && (tipos[i] == tipos[0] ||
             (tipos[i] != POUPANCA && tipos[0] != POUPANCA)))) {
            printf("Combinacao de contas invalida.\n");
            return;
        }
    }

    Cliente *dados = realloc(banco->clientes,
                              (banco->quantidadeClientes + 1) * sizeof(Cliente));
    if (dados == NULL) {
        printf("Memoria insuficiente.\n");
        return;
    }
    banco->clientes = dados;
    for (int i = 0; i < novo.quantidadeContas; i++) {
        novo.contas[i].numero = banco->proximoNumeroConta;
        novo.contas[i].tipo = tipos[i];
        /* O saldo permanece zero pela inicializacao de novo. */
    }
    banco->proximoNumeroConta++;
    banco->clientes[banco->quantidadeClientes++] = novo;
    salvarBanco(banco);

    printf("Cliente cadastrado. Contas: ");
    for (int i = 0; i < novo.quantidadeContas; i++) {
        mostrarConta(&novo.contas[i]);
        printf(" ");
    }
    printf("\n");
}

void consultarHistoricoBanco(Banco *banco) {
    if (!banco->aberto) {
        printf("Abra o banco primeiro.\n");
        return;
    }
    float total = 0;
    for (int i = 0; i < banco->quantidadeClientes; i++)
        total += saldoTotal(&banco->clientes[i]);
    printf("\n== SALDO TOTAL DO BANCO ==\n%d clientes | R$ %.2f\n",
           banco->quantidadeClientes, total);

    printf("\n== CLIENTES APTOS A EMPRESTIMOS ==\n");
    for (int i = 0; i < banco->quantidadeClientes; i++) {
        Cliente *c = &banco->clientes[i];
        int corrente = indiceConta(c, CORRENTE), poupanca = indiceConta(c, POUPANCA);
        if (corrente >= 0 && poupanca >= 0 && c->contas[poupanca].saldo > 0 && !c->fezEmprestimo)
            printf("%s | %04d-01 | R$ %.2f\n", c->nome,
                   c->contas[corrente].numero, saldoTotal(c) * 2.5f);
    }

    printf("\n== CLIENTES COM SALDO NEGATIVO NA CONTA CORRENTE ==\n");
    for (int i = 0; i < banco->quantidadeClientes; i++) {
        Cliente *c = &banco->clientes[i];
        int corrente = indiceConta(c, CORRENTE);
        if (corrente >= 0 && c->contas[corrente].saldo < 0)
            printf("%s | %04d-01 | R$ %.2f\n", c->nome,
                   c->contas[corrente].numero, c->contas[corrente].saldo);
    }

    printf("\n== CLIENTES APTOS A COMPRA DE TITULOS ==\n");
    for (int i = 0; i < banco->quantidadeClientes; i++) {
        Cliente *c = &banco->clientes[i];
        int salario = indiceConta(c, SALARIO);
        if (salario >= 0 && c->contas[salario].saldo >= 1000 &&
            c->contas[salario].saldo <= 1500 && !c->possuiTitulo)
            printf("%s | %04d-02 | R$ %.2f\n", c->nome,
                   c->contas[salario].numero, c->contas[salario].saldo);
    }
}

/* Funcoes da atividade de Fibonacci realizada em sala. */
int fibonacci(int n) {
    int a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        int proximo = a + b;
        a = b;
        b = proximo;
    }
    return n == 0 ? 0 : b;
}

int somafibonacci(int n) {
    int soma = 0;
    for (int i = 0; i <= n; i++) soma += fibonacci(i);
    return soma;
}

int fatorial(int n) {
    int resultado = 1;
    for (int i = 2; i <= n; i++) resultado *= i;
    return resultado;
}

int ehpar(int n) { return n % 2 == 0; }

int ehprimo(int n) {
    if (n < 2) return 0;
    for (int i = 2; i < n; i++)
        if (n % i == 0) return 0;
    return 1;
}

void realizarSorteio(Banco *banco) {
    if (!banco->aberto) {
        printf("Abra o banco primeiro.\n");
        return;
    }
    if (banco->quantidadeClientes < 5) {
        printf("Sao necessarios pelo menos 5 clientes.\n");
        return;
    }

    /* Seleciona os cinco MAIORES saldos do banco. Todos precisam ter titulo. */
    int melhores[5] = {-1, -1, -1, -1, -1};
    for (int i = 0; i < banco->quantidadeClientes; i++) {
        int pos = 0;
        while (pos < 5 && melhores[pos] != -1 &&
               saldoTotal(&banco->clientes[i]) <= saldoTotal(&banco->clientes[melhores[pos]]))
            pos++;
        if (pos < 5) {
            for (int j = 4; j > pos; j--) melhores[j] = melhores[j - 1];
            melhores[pos] = i;
        }
    }
    for (int i = 0; i < 5; i++) {
        if (!banco->clientes[melhores[i]].possuiTitulo) {
            printf("Sorteio indisponivel: todos os cinco maiores saldos precisam ter titulo.\n");
            return;
        }
    }

    /* Cada participante informa seu numero, como no exercicio da aula.
       Limite de 10 evita estouro do fatorial em int. */
    int resultado[5], primeiro = -1, segundo = -1, terceiro = -1;
    float media = 0;
    for (int i = 0; i < 5; i++) {
        int numero;
        printf("Numero de %s (0 a 10): ", banco->clientes[melhores[i]].nome);
        scanf("%d", &numero);
        if (numero < 0 || numero > 10) {
            printf("Numero invalido. Sorteio cancelado.\n");
            return;
        }
        resultado[i] = (somafibonacci(numero) + fatorial(numero)) / 3;
        media += resultado[i] / 5.0f;
    }

    /* Mesmo criterio de 1o, 2o e 3o lugar do programa enviado. */
    for (int i = 0; i < 5; i++) {
        if (ehprimo(resultado[i]) &&
            (primeiro == -1 || resultado[i] > resultado[primeiro])) primeiro = i;
    }
    for (int i = 0; i < 5; i++) {
        if (i != primeiro && ehpar(resultado[i]) &&
            (segundo == -1 || resultado[i] > resultado[segundo])) segundo = i;
    }
    for (int i = 0; i < 5; i++) {
        if (i != primeiro && i != segundo && resultado[i] > media) {
            terceiro = i;
            break;
        }
    }

    /* Preferencia: 1o, depois 2o, depois 3o. */
    int vencedor = primeiro != -1 ? primeiro : (segundo != -1 ? segundo : terceiro);
    if (vencedor == -1) {
        printf("Nenhum participante atendeu aos criterios do sorteio.\n");
        return;
    }

    Cliente *c = &banco->clientes[melhores[vencedor]];
    int salario = indiceConta(c, SALARIO);
    float premio = c->contas[salario].saldo * 10;
    c->contas[salario].saldo += premio;
    salvarBanco(banco);
    printf("\n== CLIENTE CONTEMPLADO NO SORTEIO ==\n");
    printf("%s | %04d-02 | Premio: R$ %.2f\n", c->nome, c->contas[salario].numero, premio);
}

void mostrarAlerta(Cliente *c) {
    int corrente = indiceConta(c, CORRENTE);
    if (corrente >= 0 && c->contas[corrente].saldo < 0)
        printf("ALERTA: sua conta corrente esta com saldo negativo!\n");
}

void depositar(Cliente *c, Conta *conta) {
    float valor;
    printf("Valor do deposito: R$ ");
    scanf("%f", &valor);
    if (valor <= 0) { printf("Valor invalido.\n"); return; }
    conta->saldo += valor;
    registrarTransacao(c, conta, 'D', valor);
    printf("Saldo atualizado: R$ %.2f\n", conta->saldo);
}

void sacar(Cliente *c, Conta *conta) {
    float valor;
    printf("Valor do saque: R$ ");
    scanf("%f", &valor);
    if (valor <= 0) { printf("Valor invalido.\n"); return; }
    /* A conta corrente pode ficar negativa */
    if (conta->tipo != CORRENTE && valor > conta->saldo) {
        printf("Saldo insuficiente.\n");
        return;
    }
    conta->saldo -= valor;
    registrarTransacao(c, conta, 'S', valor);
    printf("Saldo atualizado: R$ %.2f\n", conta->saldo);
}

void solicitarEmprestimo(Cliente *c) {
    int corrente = indiceConta(c, CORRENTE), poupanca = indiceConta(c, POUPANCA);
    if (corrente < 0 || poupanca < 0 || c->contas[poupanca].saldo <= 0 || c->fezEmprestimo) {
        printf("Cliente nao atende aos requisitos de emprestimo.\n");
        return;
    }
    float valor = saldoTotal(c) * 2.5f;
    if (valor <= 0) { printf("Nao ha limite positivo para emprestimo.\n"); return; }
    c->contas[corrente].saldo += valor;
    c->fezEmprestimo = 1;
    registrarTransacao(c, &c->contas[corrente], 'E', valor);
    printf("Emprestimo de R$ %.2f creditado na conta corrente.\n", valor);
}

void adquirirTitulo(Cliente *c) {
    int salario = indiceConta(c, SALARIO);
    if (salario < 0 || c->contas[salario].saldo < 1000 ||
        c->contas[salario].saldo > 1500 || c->possuiTitulo) {
        printf("Compra indisponivel: e preciso ter conta salario com R$ 1000 a 1500 e nao ter titulo.\n");
        return;
    }
    c->possuiTitulo = 1;
    printf("Titulo de capitalizacao adquirido.\n");
}

void mostrarExtrato(Cliente *c) {
    printf("\n== ULTIMAS 5 TRANSACOES ==\n");
    if (c->quantidadeTransacoes == 0) printf("Nenhuma transacao.\n");
    for (int i = 0; i < c->quantidadeTransacoes; i++) {
        Transacao *t = &c->historico[i];
        printf("R$ %.2f : %c | Conta %04d-%02d\n",
               t->valor, t->operacao, t->numero, t->tipo);
    }
}

void acessarCliente(Banco *banco) {
    if (!banco->aberto) { printf("Abra o banco primeiro.\n"); return; }

    int numero, tipo, indiceCliente = -1, contaAtiva = -1;
    printf("Numero completo da conta (ex: 1000-01): ");
    if (scanf("%d-%d", &numero, &tipo) != 2) {
        printf("Numero invalido.\n");
        return;
    }
    for (int i = 0; i < banco->quantidadeClientes; i++) {
        Cliente *c = &banco->clientes[i];
        int pos = indiceConta(c, tipo);
        if (pos >= 0 && c->contas[pos].numero == numero) {
            indiceCliente = i;
            contaAtiva = pos;
            break;
        }
    }
    if (indiceCliente == -1) { printf("Conta nao encontrada.\n"); return; }

    Cliente *c = &banco->clientes[indiceCliente];
    printf("Bem-vindo(a), %s!\n", c->nome);
    mostrarAlerta(c);
    int opcao;
    do {
        Conta *conta = &c->contas[contaAtiva];
        printf("\n== MENU DO CLIENTE ==\nConta ativa: ");
        mostrarConta(conta);
        printf(" | Saldo: R$ %.2f\n", conta->saldo);
        printf("1 - Depositar\n2 - Sacar\n3 - Solicitar emprestimo\n");
        printf("4 - Adquirir titulo\n5 - Solicitar extrato\n");
        printf("6 - Trocar / consultar conta\n0 - Voltar ao menu principal\nOpcao: ");
        scanf("%d", &opcao);
        switch (opcao) {
            case 1: depositar(c, conta); salvarBanco(banco); break;
            case 2: sacar(c, conta); salvarBanco(banco); break;
            case 3: solicitarEmprestimo(c); salvarBanco(banco); break;
            case 4: adquirirTitulo(c); salvarBanco(banco); break;
            case 5: mostrarExtrato(c); break;
            case 6:
                for (int i = 0; i < c->quantidadeContas; i++) {
                    printf("%d - ", i + 1);
                    mostrarConta(&c->contas[i]);
                    printf(" | R$ %.2f\n", c->contas[i].saldo);
                }
                printf("Escolha a conta: ");
                int escolha;
                scanf("%d", &escolha);
                if (escolha >= 1 && escolha <= c->quantidadeContas)
                    contaAtiva = escolha - 1;
                else printf("Conta invalida.\n");
                break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
        mostrarAlerta(c);
    } while (opcao != 0);
}

