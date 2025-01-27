# Alocação de arquivos
* Disco - Unidade de alocação 
    * Setor = 512 bytes
    * Cluster/bloco = 1 ou mais setores
* Sistema de arquivos
    * Conjunto de setores
    * meta-informações
        * Nome
        * Extensão
        * Tamanho
        * Proteção
        * Diretórios
    * Dados - Conteudo do arquivo
* Diretórios
    * Unico nivel
    * Dois niveis
    * Árvore
    * Grafo aciclico
        * Sequencial x acesso rápido

## Métodos de alocação
* Contiguo
* Lista ligada
    * Cada bloco possui um ponteiro para o próximo bloco
    * A alocação não precisa ser contigua
* Indexada
    * Mais moderna
    * Mais eficiente 
    * Bloco de indices armazena quais são os indices dos blocos que estão sendo utilizados por um arquivo

## Métodos de gerenciamento de espaços livres
* Lista ligada
    * Cada bloco possui um ponteiro para o próximo bloco livre
* Mapa de bits
    * Cada bloco possui um bit que indica se está livre ou ocupado

## FAT - File Allocation Table
* Lista ligada
* Tabela carregada na memória
    * A partir dela, temos os indices dos elementos presentes no disco
* FAT16
    * Tem um máximo de 2^16 clusters

* Composto por
    * Bootrecord
    * Tabela FAT1
    * Tabela FAT2
    * Root dir
    * Dados

### Bootrecord
* Primeiros 3 bytes disassembled viram um código de jump para a posição que deverá ser iniciada na RAM e executada
    * Instrução x86 JMP 
* Próximos 8 bytes são os identificadores OEM
    * 3 - 10 = Versão do DOS que está sendo usado
    * Os próximos 8 bytes refletem o nome da versão
* Numero de bytes por setor
* Numeros de setores por cluster
* Numero de setores reservador
* Numero de FATs
* Numero de entradas por diretório
* Numero total de setores em volume lógico
* Byte que indica o tipo de descritor de midia
* Numero de setores por FAT
* Numero de setores por trilha
* Numero de setores ocultos
* Contagem de setores

### Root dir
* Long FileName/8.3
* Campo atributo diferencia os dois tipos - Posição 11
    * Se for 0x0F - É LFN
* LFN
    * Armazena apenas o nome
* 8.3
    * Nome
    * Extensão
    * Atributos
    * Data
    * Hora
    * Tamanho
    * Endereço do primeiro cluster
* Se a primeira entrada do nome for == 0xE5, então o arquivo foi deletado

### Arquivo 1
* Nome - TESTE
* Extensão - TXT
* Atributos - arquivo -> 0x20
* Tam - 1103
* First cluster - 0x0038

##  Arquivo 2 - fat16_4sectorpercluster
### Infos do Bootrecord
a) Primeiros 3 bytes - 0xEB 0x3C 0x90 - JMP 0x3C -> 0x3C00
b) Próximos 8 - Versão do DOS - 0x6D 0x6B 0x66 0x73 0x2E 0x66 0x61 0x74 - mkfs.fat
c) bytes por setor - 0x0002 -> 0x0200 (little endian) = 512(dec)
d) setores por cluster - 0x04 -> 4
e) setores reservados - 0x0400 -> 0x0004 (little endian) = 4
f) numero de FATs - 0x02 = 2
g) numero de entradas de diretórios -> 0x0002 -> 0x0200 (little endian) = 512
h) numero total de setores no volume lógico -> 0x00A0 -> 0x0A00 (little endian) = 40960
i) tipo de descritor de midia - 0xF8
j) setores por FAT - 0x2800 -> 0x0028 (little endian) = 40
k) setores por trilha - 0x2000 -> 0x0020 (little endian) = 32

* Se cada fat tem 40 setores, então cada fat tem 20.480 bytes (40 * 512) (0x5000)

### Posições no disco
* fat1 - (reserved sectors) 4 * 512 = 2048 (0x800)
* fat2 - 2048 (0x800) + 20.480 (0x5000) = 0x5800
* root dir - 0x5800 (inicio fat2) + 20.480 (0x5000) = 0xA800
* inicio da area de dados - 0xA800 + 512(numero de entradas de diretórios) * 32 = 59392 (0xE800)

### Analisar as entradas 8.3 
* Analisar o bit 11, se for 0x0F é LFN (ignorar), se for 0x10 é diretório e se for 0x20 é arquivo
* Entrada 1
    * Nome - 
    * Atributo - 0x0F
    * First Cluster - 
    * size -
* Entrada 2
    * Nome - SUBDIR
    * Atributo - 0x10
    * First Cluster - 0x0D00 -> 0x000D (little endian) = 13
    * size - 0
* Entrada 3
    * Nome - 
    * Atributo - 0x0F
    * First Cluster - 
    * size -
* Entrada 4
    * Nome - TESTE, extensão C
    * Atributo - 0x20
    * First Cluster - 0x0500 -> 0x0005
    * size - 4341 bytes
* Entrada 5
    * Nome - 
    * Atributo - 0x0F
    * First Cluster - 
    * size -
* Entrada 6
    * Nome - TESTE1, extensão C
    * Atributo - 0x20
    * First Cluster - 0x0A00 -> 0x000A
    * size - 5929 bytes

### Acessar o conteudo pela FAT

* Entrada 4
    * Na entrada 5 da fat (cada entrada são 2 bytes) aponta para a entrada 6 (0x0600 -> 0x0006), que aponta para a entrada 7 (0x0700 -> 0x0007), que aponta para o fim de arquivo (0xFFFF) (ou seja, acesso sequencial nesse caso)
    * Inicio da area de dados + (5 (cluster)-2) * 4 (sector per cluster) * 512 (bytes per sector) -> 0xE800 + 0x1800 = 0x10000 (65.536) - cluster da entrada TESTE.C 
* Entrada 6
    * Na entrada 10 da fat (0x000A) aponta para a entrada 11 (0x000B), que aponta para a entrada 12 (0x000C) que aponta para o fim de arquivo (0xFFFF) (ou seja, acesso sequencial nesse caso)
    * Inicio da area de dados + (10 (cluster)-2) * 4 (sector per cluster) * 512 (bytes per sector) -> 0xE800 + 0x4000 = 0x12800 (75.776) - cluster da entrada TESTE1.C
    
## Exemplo 2 - fat16_4sectorpercluster
### 1 - Acessar o boot record e obter as seguintes informações
a) bytes per sector - 0x0002 -> 0x0200 (little endian) = 512(dec)
b) reserved sectors - 0x0400 -> 0x0004 (little endian) = 4
c) num fat - 0x0200 -> 0x0002 (little endian) = 2
d) sector per fat - 0x2800 -> 0x0028 (little endian) = 40
e) sector per cluster - 4
f) root dir entries - 512 entradas (cada entrada tem 32 bytes)

cada fat tem 40 setores * 512 bytes per sector = 20.480 (0x5000)


### 2 - Determinar as posições no disco da fat1, fat2, root, inicio da área de dados
* fat1 - reserved sectors (4) * 512 (dec) = 2048 (0x800)
* fat2 - posição 2048 (0x800) + 20.480 (0x5000) = 0x5800
* root - 0x5800 (inicio fat2) + 20.480 (0x5000) = 0xA800
* inicio da area de dados - 0xA800 + 512 * 32 = 59392 (0xE800)

### 3 - Posicionar no root dir e analisar as entradas 8.3
* Entrada 1
    * Nome - 0x0F
    * Atributo - 
    * First Cluster - 
    * size - 
* Entrada 2
    * Nome - SUBDIR
    * Atributo - 0x10 (dir)
    * First Cluster - 0x0d00 -> 0x000d (little endian) = 13
    * size - 0 
* Entrada 3
    * Nome - 0x0F
    * Atributo - 
    * First Cluster - 
    * size - 
* Entrada 4
    * Nome - TESTE, extensão C
    * Atributo - 0x20 (arq)
    * First Cluster - 0x0500 -> 0x0005 
    * size - 4341 bytes, cada cluster tem 2048 bytes, logo vai ocupar mais que um cluster (2.11, mais especificamente), então vai ocupar 3 clusters (245 bytes do ultimo cluster)
* Entrada 5
    * Nome - 0x0F
    * Atributo - 
    * First Cluster - 
    * size - 
* Entrada 6
    * Nome - TESTE1, extensão C
    * Atributo - 0x20 (arq)
    * First Cluster - 0x0A00 -> 0x000A
    * size - 5929 bytes
### 4 - Acessar o conteudo dos arquivos a partir da FAT
* Entrada 2
    * Na entrada numero 5 da fat, temos o valor 6 e o 6 aponta para o 7 (fim de arquivo)
    ![alt text](image.png)
    * Inicio da area de dados + (5 (cluster)-2) * 4 (sector per cluster) * 512 (bytes per sector) = 0x10000 (65.536) - cluster 1
    

### Exercicio
a) Qual o maior tamanho de partição suportada pela FAT16, considerando:
1. Cluster de 1 setor de 512 bytes
R: 2^16 * 512 = 32MB	
2. Cluster de 4 setores de 512 bytes
R: 2^16 * 512 * 4 = 128MB

## EXT2
* Organização
    * Boot
    * Block Grub
        * Superbloco
        * Group descriptor
        * Mapa de bits
        * 

1. Qual a organização de um disco formatado em EXT2?
R: Superblock (0 - 83(ou 1023 se for o extended)) -> Block Group Descriptor Table (0 - 31) -> Inodes(0 - 127) 
2. Como os nomes de arquivos e diretórios são acessados no ext2?
R: Em ext2 diretórios são armazenados no diretory entry que contem um numero de entradas com os seus conteudos.
3. Qual o método de alocação do ext2? Qual a função do inode?
R: Alocação Indexada. O Inode é uma estrutura no disco que representa um arquivo, diretório, links e etc. Eles não contém os dados do arquivo/diretório que eles representam. Ao invés disso eles apontam para os blocos que contém os dados.
4. Como é realizado o gerenciamento de espaço livre no ext2?
R: Os blocos livres são marcados com um bitmap de livres/alocados.



