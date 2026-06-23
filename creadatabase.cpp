#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <string>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

// SVUOTARE un file (trunc = tronca a 0 byte)
void svuotaFile(const string& filename) {
    ofstream f(filename, ios::trunc);
}   // si chiude da solo all'uscita dello scope

// SCRIVERE su file (app = aggiunge in fondo senza cancellare)
void scriviSuFile(const string& filename, const string& testo) {
    ofstream f(filename, ios::app);
    if (f.is_open()) {
        f << testo;
    }
}

// SOVRASCRIVERE tutto il file (out = ricomincia da capo)
void sovrascriviFile(const string& filename, const string& testo) {
    ofstream f(filename, ios::out);
    if (f.is_open()) {
        f << testo;
    }
}

string prendiLaPrimaParola(string riga) {
    stringstream ss(riga);
    string prima_parola;
    ss >> prima_parola;
    return prima_parola;
}


int main(){
    ifstream infile("named.txt");
    string myText;
    while (getline(infile, myText)){
        scriviSuFile("nomi.txt", prendiLaPrimaParola(myText) + "\n");
    }

    return 0;
}