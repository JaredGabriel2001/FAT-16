#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main() {
    const char* fileName = "testfat.img";

    ifstream file(fileName, ios::binary);
    if (!file) {
        cerr << "Erro ao abrir o arquivo: " << fileName << endl;
        return 1;
    }

    file.seekg(0, ios::end);
    streamsize fileSize = file.tellg();
    file.seekg(0, ios::beg);

    vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize)) {
        cerr << "Erro ao ler o arquivo: " << fileName << endl;
        return 1;
    }

    file.close();
    return 0;
}
