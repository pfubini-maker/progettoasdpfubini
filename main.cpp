#include <iostream>
#include <vector>
#include <list>
#include <cstdlib>
#include <set>
#include <unordered_set>
#include <fstream>
#include <sstream> // <--- FONDAMENTALE per stringstream

using namespace std;
const long long PRIME_P = 2147483647;

class PathParser {
public:
    static vector<int> estraiCammino(const string& riga) {
        vector<int> cammino;
        stringstream ss(riga);
        string numero_str;

        while (getline(ss, numero_str, '|')) {
            if (!numero_str.empty()) {
                cammino.push_back(stoi(numero_str));
            }
        }
        return cammino;
    }
};

// Reinseriamo le classi del Grafo per farlo funzionare davvero
class Edge {
private:
    int neighbor;
    int weight;
public:
    Edge(int n, int w) : neighbor(n), weight(w) {}
    int getNeighbor() const { return neighbor; }
    int getWeight() const { return weight; }
    void incrementWeight() { weight += 1; }
};

struct UniversalHash {
    int a;
    int b;
    int m;

    void init(long long tableSize) {
        m = tableSize; // CORRETTO: rimosso "int" per non nascondere la variabile membro
        if (m <= 0) m = 1;
        a = (rand() % (PRIME_P - 1)) + 1;
        b = rand() % PRIME_P;
    }

    long long compute(int x) const {
        return ((a * x + b) % PRIME_P) % m;
    }
}; // <--- CORRETTO: Aggiunto il punto e virgola fondamentale!

class Node {
private:
    int id;
    UniversalHash hash_func;
    vector<list<Edge>> adjacency_hash_table;
    int num_edges;
public:
    Node(int node_id, int expected_neighbors = 4) : id(node_id), num_edges(0) {
        hash_func.init(expected_neighbors * 2 + 1);
        adjacency_hash_table.resize(hash_func.getM());
    }
    int getId() const { return id; }
    void addOrUpdateNeighbor(int neighbor_id, int& global_max_weight) {
        long long idx = hash_func.compute(neighbor_id);
        for (auto& edge : adjacency_hash_table[idx]) {
            if (edge.getNeighbor() == neighbor_id) {
                edge.incrementWeight();
                if (edge.getWeight() > global_max_weight) global_max_weight = edge.getWeight();
                return;
            }
        }
        adjacency_hash_table[idx].push_back(Edge(neighbor_id, 1));
        num_edges++;
    }
};

class UniversalHashGraph {
private:
    UniversalHash global_hash_func;
    vector<list<Node>> node_hash_table;
    int num_nodes;
    int max_edge_weight;
public:
    UniversalHashGraph(int expected_nodes = 16) : num_nodes(0), max_edge_weight(0) {
        global_hash_func.init(expected_nodes * 2 + 1);
        node_hash_table.resize(global_hash_func.getM());
    }
    int getMaxEdgeWeight() const { return max_edge_weight; }
    void aggiungiNodo(int node_id) {
        long long idx = global_hash_func.compute(node_id);
        for (const auto& node : node_hash_table[idx]) {
            if (node.getId() == node_id) return;
        }
        node_hash_table[idx].push_back(Node(node_id));
        num_nodes++;
    }
    Node* findNode(int node_id) {
        long long idx = global_hash_func.compute(node_id);
        for (auto& node : node_hash_table[idx]) {
            if (node.getId() == node_id) return &node;
        }
        return nullptr;
    }
    void aggiungiArco(int node_1, int node_2) {
        aggiungiNodo(node_1);
        aggiungiNodo(node_2);
        Node* n1 = findNode(node_1);
        if (n1 != nullptr) n1->addOrUpdateNeighbor(node_2, max_edge_weight);
        Node* n2 = findNode(node_2);
        if (n2 != nullptr) n2->addOrUpdateNeighbor(node_1, max_edge_weight);
    }
};

// Modificata in modo da ritornare l'oggetto Grafo popolato
UniversalHashGraph crealinee(string filename) {
    ifstream infile(filename);
    UniversalHashGraph g(100); // Inizializziamo il grafo interno alla funzione 
    string myText;

    if (!infile.is_open()) {
        cout << "Errore: Impossibile aprire il file " << filename << endl;
        return g;
    }

    while (getline(infile, myText)) {
        if (myText.empty()) continue;

        // Trova le posizioni degli spazi per isolare la parte con i "|"
        size_t pos1 = myText.find(' ');
        string stringa_cammino;

        if (pos1 != string::npos) {
            size_t pos2 = myText.find(' ', pos1 + 1);
            if (pos2 != string::npos) {
                // Sottostringa tra il primo e il secondo spazio
                stringa_cammino = myText.substr(pos1 + 1, pos2 - pos1 - 1);
            } else {
                stringa_cammino = myText.substr(pos1 + 1);
            }
        } else {
            // Se non ci sono spazi, l'intera riga è il cammino
            stringa_cammino = myText;
        }

        // Estraiamo il vettore di interi dal cammino isolato
        vector<int> nodi_cammino = PathParser::estraiCammino(stringa_cammino);
        
        // Colleghiamo i nodi consecutivi inserendo gli archi nel grafo 
        for (size_t i = 0; i < nodi_cammino.size() - 1; ++i) {
            int u = nodi_cammino[i];
            int v = nodi_cammino[i+1];
            g.aggiungiArco(u, v); // Popola il grafo usando la tua funzione 
        }   
    }

    infile.close(); // Chiude il file correttamente come richiesto [cite: 17]
    return g; // Ritorna il grafo compilato
}

int main() {
    // Nota: Assicurati che questo file esista nella stessa cartella dell'eseguibile!
    string filename = "percorsi prova.txt"; 
    
    UniversalHashGraph mioGrafo = crealinee(filename);
    
    cout << "File letto con successo! Lunghezza massima dell'arco: " 
         << mioGrafo.getMaxEdgeWeight() << endl; // Richiesto nella sezione 2 [cite: 45]

    return 0;
}