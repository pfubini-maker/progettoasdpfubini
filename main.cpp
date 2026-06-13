#include <iostream>
#include <vector>
#include <list>
#include <cstdlib>
#include <set>
#include <unordered_set> // Utilizzato per l'insieme hasciato dei nodi già percorsi nell'albero
#include <fstream>

using namespace std;
const long long PRIME_P = 2147483647;

class PathParser {
public:
    // Questa funzione prende la stringa "1239|3561|4374" e restituisce {1239, 3561, 4374}
    static vector<int> estraiCammino(const string& riga) {
        vector<int> cammino;
        stringstream ss(riga);
        string numero_str;

        // getline scansiona la stringa e si ferma ogni volta che incontra il carattere '|'
        while (getline(ss, numero_str, '|')) {
            if (!numero_str.empty()) {
                cammino.push_back(stoi(numero_str)); // Converte in int e aggiunge al vettore
            }
        }

        return cammino;
    }
};

struct UniversalHash {
    int a;
    int b;
    int m;

    void init (long long tableSize){
        int m=tableSize;
        if (m <= 0) m = 1;
        a = (rand() % (PRIME_P - 1)) + 1;
        b = rand() % PRIME_P;
    
    }

    long long compute(int x) const{
        return ((a * x + b) % PRIME_P) % m;
    }
}




UniversalHashGraph aggiungiArco(string filename) {
    UniversalHashGraph g;
    g.initGraph();

    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Errore nell'apertura del file: " << filename << endl;
        return g; // Ritorna un grafo vuoto
    }

    int node1, node2;
       
    // Use a while loop together with the getline() function to read the file line by line
    while (getline (filename, myText)) {
        find the position of spaces in the line
        size_t pos1 = myText.find(' ');
        size_t pos2 = myText.find(' ', pos1 + 1);

        myText.substr(pos1, pos2 - pos1);
        
       
   }


    infile.close();
    return g;
}
