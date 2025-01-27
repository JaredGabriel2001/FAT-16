/*
Fazer um programa (em qualquer linguagem) que faça a leitura de uma imagem fat e apresente na tela:

    O número de FATs;
    Tambem é exibido:
        Setores por cluster;
        Setores por fat;
        Tamanho do setor.
    A posição inicial de cada FAT no disco;
    A posição inicial do setor de diretório raiz;
    A posição inicial da área de dados;
    Os arquivos e diretórios armazenados na raiz (apenas as entradas 8.3), com os seus respectivos nomes, primeiro cluster e tamanho.

Exercício desafio: apresentar os clusters ocupados por um arquivo, e seu conteúdo
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iomanip>

using namespace std;

struct BootSector {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t max_root_entries;
    uint16_t total_sectors_16;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t sectors_per_fat_32;
};

BootSector read_boot_sector(ifstream &file) {
    uint8_t boot_sector[512];
    file.read(reinterpret_cast<char *>(boot_sector), 512);

    BootSector bpb;
    memcpy(&bpb.bytes_per_sector, &boot_sector[11], sizeof(bpb.bytes_per_sector));
    bpb.sectors_per_cluster = boot_sector[13];
    memcpy(&bpb.reserved_sectors, &boot_sector[14], sizeof(bpb.reserved_sectors));
    bpb.num_fats = boot_sector[16];
    memcpy(&bpb.max_root_entries, &boot_sector[17], sizeof(bpb.max_root_entries));
    memcpy(&bpb.total_sectors_16, &boot_sector[19], sizeof(bpb.total_sectors_16));
    bpb.media_descriptor = boot_sector[21];
    memcpy(&bpb.sectors_per_fat_16, &boot_sector[22], sizeof(bpb.sectors_per_fat_16));
    memcpy(&bpb.sectors_per_track, &boot_sector[24], sizeof(bpb.sectors_per_track));
    memcpy(&bpb.num_heads, &boot_sector[26], sizeof(bpb.num_heads));
    memcpy(&bpb.hidden_sectors, &boot_sector[28], sizeof(bpb.hidden_sectors));
    memcpy(&bpb.total_sectors_32, &boot_sector[32], sizeof(bpb.total_sectors_32));
    memcpy(&bpb.sectors_per_fat_32, &boot_sector[36], sizeof(bpb.sectors_per_fat_32));

    return bpb;
}

void print_fat_positions(const BootSector &bpb) {
    uint32_t sectors_per_fat = bpb.sectors_per_fat_16 ? bpb.sectors_per_fat_16 : bpb.sectors_per_fat_32;
    uint32_t fat_start = bpb.reserved_sectors;

    cout << "Número de FATs: " << static_cast<int>(bpb.num_fats) << endl;
    cout << "Setores por cluster: " << static_cast<int>(bpb.sectors_per_cluster) << endl;
    cout << "Setores por FAT: " << sectors_per_fat << endl;
    cout << "Tamanho do setor: " << bpb.bytes_per_sector << " bytes" << endl;

    cout << "Posição inicial de cada FAT no disco:" << endl;
    for (uint8_t i = 0; i < bpb.num_fats; ++i) {
        cout << "FAT " << static_cast<int>(i + 1) << ": setor 0x" << hex << (fat_start + i * sectors_per_fat) << endl;
    }
}

void print_directory_positions(const BootSector &bpb) {
    uint32_t sectors_per_fat = bpb.sectors_per_fat_16 ? bpb.sectors_per_fat_16 : bpb.sectors_per_fat_32;
    uint32_t root_dir_start = bpb.reserved_sectors + bpb.num_fats * sectors_per_fat;
    uint32_t root_dir_sectors = ((bpb.max_root_entries * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
    uint32_t data_area_start = root_dir_start + root_dir_sectors;

    cout << "Posição inicial do setor de diretório raiz: 0x" << hex << root_dir_start << endl;
    cout << "Posição inicial da área de dados: 0x" << hex << data_area_start << endl;
}

void print_root_directory(const BootSector &bpb, ifstream &file) {
    uint32_t sectors_per_fat = bpb.sectors_per_fat_16 ? bpb.sectors_per_fat_16 : bpb.sectors_per_fat_32;
    uint32_t root_dir_start = bpb.reserved_sectors + bpb.num_fats * sectors_per_fat;
    uint32_t root_dir_sectors = ((bpb.max_root_entries * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;

    file.seekg(root_dir_start * bpb.bytes_per_sector);
    vector<uint8_t> root_dir(root_dir_sectors * bpb.bytes_per_sector);
    file.read(reinterpret_cast<char *>(root_dir.data()), root_dir.size());

    cout << "Arquivos e diretórios armazenados na raiz:" << endl;
    for (size_t i = 0; i < bpb.max_root_entries; ++i) {
        size_t entry_offset = i * 32;
        uint8_t first_byte = root_dir[entry_offset];

        if (first_byte == 0x00) {
            break; 
        }
        if (first_byte == 0xE5) {
            continue;
        }

        char name[9] = {0};
        char ext[4] = {0};
        memcpy(name, &root_dir[entry_offset], 8);
        memcpy(ext, &root_dir[entry_offset + 8], 3);
        uint8_t attr = root_dir[entry_offset + 11];
        uint16_t low_cluster;
        memcpy(&low_cluster, &root_dir[entry_offset + 26], sizeof(low_cluster));
        uint32_t size;
        memcpy(&size, &root_dir[entry_offset + 28], sizeof(size));

        uint32_t first_cluster = low_cluster; 

        if (attr & 0x10) {
            cout << "[DIR] " << name << endl;
        } else {
            cout << "[FILE] " << name << "." << ext << " - Primeiro cluster: " << dec << first_cluster << ", Tamanho: " << dec << size << " bytes" << endl;
        }
    }
}

void read_fat_image(const string &image_path) {
    ifstream file(image_path, ios::binary);
    if (!file) {
        cerr << "Erro ao abrir o arquivo: " << image_path << endl;
        return;
    }

    BootSector bpb = read_boot_sector(file);
    print_fat_positions(bpb);
    print_directory_positions(bpb);
    print_root_directory(bpb, file);

    file.close();
}

int main() {
    string image_path = "testfat.img";
    read_fat_image(image_path);
    return 0;
}

