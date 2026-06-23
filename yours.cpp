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

static const long long PRIME_P = 2147483647LL;

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


      // =========================================================================
     // UniversalHash
    // Implementa una funzione di hashing universale della forma:
   //      h(x) = ((a*x + b) mod PRIME_P) mod m
  // dove a e b sono scelti a caso ad ogni init() (quindi ad ogni rehash la
 // funzione cambia, come richiesto dall'hashing universale).
// =========================================================================
class UniversalHash {
private:
    long long a;   // coefficiente moltiplicativo casuale
    long long b;   // coefficiente additivo casuale
    long long m;   // dimensione della tabella su cui si proietta l'hash

public:
    // costruttore di default: valori neutri, va comunque chiamato init()
    // prima di usare compute() per davvero
    UniversalHash() : a(1), b(0), m(1) {}

    // (ri)inizializza la funzione di hash per una tabella di table_size
    // bucket, pescando nuovi coefficienti a caso: questo e' il punto in
    // cui la funzione "cambia" ogni volta che si fa un rehash
    void init(long long table_size) {
        m = table_size > 0 ? table_size : 1;
        a = (rand() % (PRIME_P - 1)) + 1;   // a in [1, PRIME_P-1], mai 0
        b = rand() % PRIME_P;               // b in [0, PRIME_P-1]
    }

    // calcola l'indice di bucket per la chiave x
    long long compute(int x) const {
        // attenzione: se x e' negativo, a*x puo' essere negativo;
        // riportiamo il risultato in [0, PRIME_P) prima del modulo finale
        long long val = ((a * (long long)x + b) % PRIME_P + PRIME_P) % PRIME_P;
        return val % m;
    }

    long long getM() const { return m; }
};

class Edge {
private:
    int neighbor;   // id del nodo vicino
    int weight;     // peso dell'arco (= quante volte il cammino lo attraversa)

public:
    Edge(int n, int w) : neighbor(n), weight(w) {}

    int getNeighbor() const { return neighbor; }
    int getWeight() const { return weight; }
    void incrementWeight() { weight += 1; }
};

class Node {
private:
    int id;
    UniversalHash hash_func;
    vector<list<Edge>> adjacency_hash_table;
    int num_edges;

    // raddoppia (circa) la tabella di adiacenza e re-inserisce tutti gli
    // archi gia' presenti usando la nuova funzione di hash
    void rehashAdjacency() {
        long long new_size = hash_func.getM() * 2 + 1;
        vector<list<Edge>> old_table = move(adjacency_hash_table);

        hash_func.init(new_size);
        adjacency_hash_table.clear();
        adjacency_hash_table.resize(new_size);
        num_edges = 0;

        for (auto& bucket : old_table) {
            for (auto& edge : bucket) {
                long long idx = hash_func.compute(edge.getNeighbor());
                adjacency_hash_table[idx].push_back(edge);
                num_edges++;
            }
        }
    }

public:
    Node(int node_id = 0, int expected_neighbors = 4) : id(node_id), num_edges(0) {
        hash_func.init(expected_neighbors * 2 + 1);
        adjacency_hash_table.resize(hash_func.getM());
    }

    int getId() const { return id; }
    int getNumEdges() const { return num_edges; }

    // accesso in sola lettura alla tabella di adiacenza, serve agli
    // algoritmi del grafo (BFS/DFS, albero puntato, stampa, infoGraph...)
    const vector<list<Edge>>& getAdjacencyTable() const { return adjacency_hash_table; }

    // aggiunge un vicino, o se gia' presente incrementa il peso dell'arco
    // verso quel vicino di 1. Tiene aggiornato anche il peso massimo
    // globale del grafo (passato per riferimento da UniversalHashGraph).
    void addOrUpdateNeighbor(int neighbor_id, int& global_max_weight) {
        // se la tabella e' troppo piena (> 70%), si ingrandisce prima di inserire
        if (num_edges >= hash_func.getM() * 0.7) {
            rehashAdjacency();
        }

        long long idx = hash_func.compute(neighbor_id);

        // controlla se il vicino esiste gia'
        for (auto& edge : adjacency_hash_table[idx]) {
            if (edge.getNeighbor() == neighbor_id) {
                edge.incrementWeight();
                if (edge.getWeight() > global_max_weight) global_max_weight = edge.getWeight();
                return;
            }
        }

        // se non esiste, lo aggiunge con peso iniziale 1
        adjacency_hash_table[idx].push_back(Edge(neighbor_id, 1));
        num_edges++;
        if (1 > global_max_weight) global_max_weight = 1;
    }

    // cerca il peso dell'arco verso neighbor_id; ritorna 0 se l'arco non esiste
    // (utile per distanzaTraDuenodi e per l'albero puntato)
    int getWeightTo(int neighbor_id) const {
        long long idx = hash_func.compute(neighbor_id);
        for (const auto& edge : adjacency_hash_table[idx]) {
            if (edge.getNeighbor() == neighbor_id) return edge.getWeight();
        }
        return 0;
    }
};


           // =========================================================================
          // pezzo
         // E' il "nodo" dell'albero usato da alberoPuntato/camminiMinMax. Ogni pezzo
        // rappresenta un nodo del grafo originale, raggiunto in un certo punto di
       // un cammino. Tiene:
      // - il nome del nodo del grafo che rappresenta
     // - il puntatore al genitore (per poter risalire/potare)
    // - i puntatori ai figli (le espansioni verso i vicini non ancora visitati)
   // - una lista hashata (qui rappresentata come hash table di bool/presenza)
  //   dei nodi gia' visitati in questo specifico cammino dalla radice fino
 //   a qui, per evitare di tornare su un nodo gia' percorso
// =========================================================================
struct pezzo {
    int nome_nodo;                 // id del nodo di grafo rappresentato
    pezzo* genitore;                // puntatore al nodo padre nell'albero (nullptr se radice)
    vector<pezzo*> figli;           // figli di questo pezzo nell'albero

    // "albero genealogico" hashato: invece di una vera hash table qui basta
    // un set hashato semplice realizzato come piccola tabella ad hashing
    // universale di interi, usata per il check "ho gia' visitato questo nodo
    // in questo cammino?" in O(1) atteso
    UniversalHash visited_hash;
    vector<list<int>> visited_table;   // tabella hash dei nodi gia' visti sul cammino fino a qui
    int visited_count;

    pezzo(int nome, pezzo* padre)
        : nome_nodo(nome), genitore(padre), visited_count(0) {
        visited_hash.init(7);
        visited_table.resize(visited_hash.getM());
        // un pezzo eredita l'insieme di nodi visitati del genitore + se stesso
        if (padre != nullptr) {
            for (const auto& bucket : padre->visited_table) {
                for (int v : bucket) {
                    insertVisited(v);
                }
            }
        }
        insertVisited(nome);
    }

    // inserisce un nodo nell'insieme hashato dei visitati, con rehash se
    // la tabella si riempie troppo (stesso criterio usato altrove: 70%)
    void insertVisited(int node_id) {
        if (visited_count >= (long long)(visited_table.size() * 0.7)) {
            // rehash: tabella piu' grande, ridistribuisco tutto
            vector<list<int>> old_table = move(visited_table);
            visited_hash.init(visited_hash.getM() * 2 + 1);
            visited_table.clear();
            visited_table.resize(visited_hash.getM());
            visited_count = 0;
            for (auto& bucket : old_table) {
                for (int v : bucket) {
                    long long idx = visited_hash.compute(v);
                    visited_table[idx].push_back(v);
                    visited_count++;
                }
            }
        }
        long long idx = visited_hash.compute(node_id);
        // evita duplicati
        for (int v : visited_table[idx]) {
            if (v == node_id) return;
        }
        visited_table[idx].push_back(node_id);
        visited_count++;
    }

    // controlla se un nodo e' gia' stato visitato in questo cammino, O(1) atteso
    bool isVisited(int node_id) const {
        long long idx = visited_hash.compute(node_id);
        for (int v : visited_table[idx]) {
            if (v == node_id) return true;
        }
        return false;
    }
};


// dichiarazione anticipata, serve a Node/UniversalHashGraph piu' sotto
class UniversalHashGraph;


      // =========================================================================
     // tree
    // Albero usato per rappresentare tutti i cammini semplici (senza ripetere
   // nodi) tra due vertici, fino a una lunghezza massima data. La radice e'
  // il nodo di partenza; ogni cammino dalla radice a una foglia corrisponde
 // a un cammino nel grafo.
// =========================================================================
class tree {
private:
    pezzo* radice;
    int nodo_arrivo;       // nodo target: quando lo si raggiunge, ci si ferma e non si espande oltre
    int lunghezza_massima; // cap sulla lunghezza (= peso totale) del cammino

public:
    tree() : radice(nullptr), nodo_arrivo(-1), lunghezza_massima(-1) {}

    // costruisce l'albero a partire da nodo1, fermandosi sui cammini che
    // raggiungono nodo2 o che superano lunghezza_massima, oppure che non
    // hanno piu' vicini espandibili (in quel caso vengono potati)
    void creaAlbero(UniversalHashGraph& g, int nodo1, int nodo2, int cap);

    // libera ricorsivamente la memoria di un sottoalbero
    static void distruggiSottoalbero(pezzo* p) {
        if (p == nullptr) return;
        for (pezzo* f : p->figli) distruggiSottoalbero(f);
        delete p;
    }

    ~tree() { distruggiSottoalbero(radice); }

    // espande ricorsivamente un pezzo aggiungendo un figlio per ogni
    // vicino raggiungibile rispettando il cap sulla lunghezza e senza
    // ripassare per nodi gia' visitati nel cammino corrente
    void espandi(UniversalHashGraph& g, pezzo* corrente, int lunghezza_corrente);

    // pota: cancella il pezzo "foglia" (senza figli), staccandolo dal
    // genitore. NON risale automaticamente al genitore: la risalita a
    // cascata (se il genitore rimane a sua volta senza figli) e' gestita
    // esplicitamente da espandi(), che e' l'unico punto che sa con
    // certezza quali pezzi dello stack di ricorsione sono ancora "vivi".
    // Farla risalire da sola qui dentro creerebbe un classico bug di
    // use-after-free: un padre piu' in alto nello stack di espandi()
    // potrebbe ritrovarsi cancellato mentre la sua chiamata e' ancora
    // in esecuzione.
    void pota(pezzo* foglia) {
        if (foglia == nullptr || !foglia->figli.empty()) return;

        pezzo* padre = foglia->genitore;
        if (padre != nullptr) {
            for (size_t i = 0; i < padre->figli.size(); ++i) {
                if (padre->figli[i] == foglia) {
                    padre->figli.erase(padre->figli.begin() + i);
                    break;
                }
            }
        } else {
            // foglia era la radice: l'albero diventa vuoto
            radice = nullptr;
        }
        delete foglia;
    }

    pezzo* getRadice() const { return radice; }

    // conta le foglie dell'albero: ricorsivo, caso base un nodo senza
    // figli conta 1, altrimenti somma il conteggio dei figli
    static int contaFoglie(pezzo* p) {
        if (p == nullptr) return 0;
        if (p->figli.empty()) return 1;
        int totale = 0;
        for (pezzo* f : p->figli) totale += contaFoglie(f);
        return totale;
    }

    int contaFoglieAlbero() const { return contaFoglie(radice); }
};


// =========================================================================
// UniversalHashGraph
// Il grafo vero e proprio: una tabella hash (lista di liste) di Node,
// indicizzata tramite hashing universale sull'id del nodo.
// =========================================================================
class UniversalHashGraph {
private:
    UniversalHash global_hash_func;
    vector<list<Node>> node_hash_table;
    int num_nodes;
    int max_edge_weight;   // peso massimo tra tutti gli archi del grafo

    // rehash della tabella dei nodi quando si riempie troppo (stesso
    // criterio di load factor 0.7 usato per le liste di adiacenza)
    void rehashNodes() {
        long long new_size = global_hash_func.getM() * 2 + 1;
        vector<list<Node>> old_table = move(node_hash_table);

        global_hash_func.init(new_size);
        node_hash_table.clear();
        node_hash_table.resize(new_size);

        for (auto& bucket : old_table) {
            for (auto& node : bucket) {
                long long idx = global_hash_func.compute(node.getId());
                node_hash_table[idx].push_back(move(node));
            }
        }
    }

public:
    // costruttore: crea il grafo con una tabella nodi di dimensione iniziale data
    UniversalHashGraph(int dimensione_iniziale = 2000) : num_nodes(0), max_edge_weight(0) {
        global_hash_func.init(dimensione_iniziale);
        node_hash_table.resize(global_hash_func.getM());
    }

    int getMaxLenght() const { return max_edge_weight; }
    int getNumNodes() const { return num_nodes; }

    // trova un nodo dato il suo id; ritorna puntatore al Node nella tabella
    // hash, oppure nullptr se non esiste. Non-const e const overload per
    // poter sia leggere che modificare il nodo trovato.
    Node* trovaNodo(int node_id) {
        long long idx = global_hash_func.compute(node_id);
        for (auto& node : node_hash_table[idx]) {
            if (node.getId() == node_id) return &node;
        }
        return nullptr;
    }

    const Node* trovaNodo(int node_id) const {
        long long idx = global_hash_func.compute(node_id);
        for (const auto& node : node_hash_table[idx]) {
            if (node.getId() == node_id) return &node;
        }
        return nullptr;
    }

    // aggiunge un nodo nuovo al grafo (se non esiste gia'); randomizza la
    // sua funzione di hash universale tramite il costruttore di Node
    void aggiungiNodo(int node_id) {
        if (trovaNodo(node_id) != nullptr) return;   // gia' presente, non fare nulla

        if (num_nodes >= global_hash_func.getM() * 0.7) {
            rehashNodes();
        }

        long long idx = global_hash_func.compute(node_id);
        node_hash_table[idx].push_back(Node(node_id));
        num_nodes++;
    }

    // aggiunge (o aggiorna il peso di) un arco non orientato tra nodo1 e
    // nodo2: se i nodi non esistono li crea, poi aggiorna la lista di
    // adiacenza di entrambi i lati (grafo non orientato => simmetrico)
    void aggiungiArco(int nodo1, int nodo2) {
        aggiungiNodo(nodo1);
        aggiungiNodo(nodo2);

        Node* n1 = trovaNodo(nodo1);
        Node* n2 = trovaNodo(nodo2);

        n1->addOrUpdateNeighbor(nodo2, max_edge_weight);
        n2->addOrUpdateNeighbor(nodo1, max_edge_weight);
    }

    // ---------------------------------------------------------------
    // albero puntato: costruisce l'albero di tutti i cammini semplici
    // da nodo1 a nodo2 con lunghezza (= somma dei pesi attraversati)
    // al massimo cap; se cap = -1, nessun limite esplicito (si ferma
    // comunque per assenza di nodi vicini espandibili).
    // ---------------------------------------------------------------
    tree alberoPuntato(int nodo1, int nodo2, int cap = -1) {
        tree t;
        t.creaAlbero(*this, nodo1, nodo2, cap);
        return t;
    }

    // ---------------------------------------------------------------
    // distanzaTraDuenodi: minimo percorso pesato tra due nodi.
    // Algoritmo (come descritto nel pdf, in pratica e' una variante di
    // Dijkstra/BFS pesato): tiene una tabella hash "lunghezza -> lista
    // di nodi raggiunti con quella lunghezza totale" e una tabella hash
    // dei nodi gia' espansi. Espande sempre i nodi alla lunghezza piu'
    // bassa corrente, e si ferma appena il nodo di arrivo viene scoperto
    // (perche', come in Dijkstra, non esiste un cammino piu' breve che
    // possa scoprirlo dopo).
    // ---------------------------------------------------------------
    int distanzaTraDuenodi(int nodo1, int nodo2) {
        if (nodo1 == nodo2) return 0;
        if (trovaNodo(nodo1) == nullptr || trovaNodo(nodo2) == nullptr) return -1;

        // "buckets per lunghezza": mappa lunghezza_totale -> lista di nodi
        // raggiunti con quella lunghezza, realizzata con hashing universale
        // (stessa idea della tabella di adiacenza: vector<list<...>> con
        // funzione di hash che proietta la lunghezza su un bucket).
        // E' l'equivalente della "bucket queue" di Dijkstra a pesi piccoli.
        struct LengthTable {
            UniversalHash h;
            vector<list<pair<int,int>>> table;   // bucket di (lunghezza, nodo)
            int count = 0;

            LengthTable() { h.init(16); table.resize(h.getM()); }

            void rehash() {
                vector<list<pair<int,int>>> old_table = move(table);
                h.init(h.getM() * 2 + 1);
                table.clear();
                table.resize(h.getM());
                count = 0;
                for (auto& bucket : old_table)
                    for (auto& pr : bucket) {
                        long long idx = h.compute(pr.first);
                        table[idx].push_back(pr);
                        count++;
                    }
            }

            void add(int length, int node_id) {
                if (count >= (int)(table.size() * 0.7)) rehash();
                long long idx = h.compute(length);
                table[idx].push_back({length, node_id});
                count++;
            }

            // estrae (rimuovendoli) tutti i nodi alla lunghezza minima presente
            // in tabella; ritorna {-1, {}} se la tabella e' vuota
            pair<int, list<int>> popMinLength() {
                if (count == 0) return {-1, {}};
                int min_len = -1;
                for (auto& bucket : table)
                    for (auto& pr : bucket)
                        if (min_len == -1 || pr.first < min_len) min_len = pr.first;

                list<int> risultato;
                for (auto& bucket : table) {
                    auto it = bucket.begin();
                    while (it != bucket.end()) {
                        if (it->first == min_len) {
                            risultato.push_back(it->second);
                            it = bucket.erase(it);
                            count--;
                        } else {
                            ++it;
                        }
                    }
                }
                return {min_len, risultato};
            }
        };

        // set hashato dei nodi gia' espansi (per non riespandere mai due volte)
        struct VisitedSet {
            UniversalHash h;
            vector<list<int>> table;
            int count = 0;

            VisitedSet() { h.init(16); table.resize(h.getM()); }

            void rehash() {
                vector<list<int>> old_table = move(table);
                h.init(h.getM() * 2 + 1);
                table.clear();
                table.resize(h.getM());
                count = 0;
                for (auto& bucket : old_table)
                    for (int v : bucket) {
                        long long idx = h.compute(v);
                        table[idx].push_back(v);
                        count++;
                    }
            }

            bool contains(int node_id) const {
                long long idx = h.compute(node_id);
                for (int v : table[idx]) if (v == node_id) return true;
                return false;
            }

            void insert(int node_id) {
                if (count >= (int)(table.size() * 0.7)) rehash();
                long long idx = h.compute(node_id);
                table[idx].push_back(node_id);
                count++;
            }
        };

        LengthTable frontier;
        VisitedSet espansi;

        frontier.add(0, nodo1);   // il nodo di partenza ha lunghezza 0

        while (true) {
            auto [lunghezza, nodi_da_espandere] = frontier.popMinLength();
            if (lunghezza == -1) break;   // niente piu' da esplorare

            for (int corrente : nodi_da_espandere) {
                if (espansi.contains(corrente)) continue;   // gia' espanso, salta
                espansi.insert(corrente);

                // come in Dijkstra: appena estraiamo il nodo target alla
                // lunghezza minima corrente, quella e' la distanza finale
                if (corrente == nodo2) return lunghezza;

                Node* nptr = trovaNodo(corrente);
                if (nptr == nullptr) continue;

                for (const auto& bucket : nptr->getAdjacencyTable()) {
                    for (const auto& edge : bucket) {
                        int vicino = edge.getNeighbor();
                        if (espansi.contains(vicino)) continue;
                        frontier.add(lunghezza + edge.getWeight(), vicino);
                    }
                }
            }
        }

        return -1;   // nodo2 non raggiungibile da nodo1
    }

    // ---------------------------------------------------------------
    // camminiMinMax: conta quanti cammini semplici esistono tra nodo1 e
    // nodo2 con lunghezza al massimo lunghezza_massima (se -1, usa la
    // distanza minima tra i due nodi come cap di default)
    // ---------------------------------------------------------------
    int camminiMinMax(int nodo1, int nodo2, int lunghezza_massima = -1) {
        if (lunghezza_massima == -1) {
            lunghezza_massima = distanzaTraDuenodi(nodo1, nodo2);
            if (lunghezza_massima == -1) return 0;   // non raggiungibile
        }
        tree t = alberoPuntato(nodo1, nodo2, lunghezza_massima);
        return t.contaFoglieAlbero();
    }

    vector<vector<int>> componentiConnesse() const {
    vector<vector<int>> componenti;

    struct VisitedSet {
        UniversalHash h;
        vector<list<int>> table;
        int count = 0;
        VisitedSet() { h.init(16); table.resize(h.getM()); }
        void rehash() {
            vector<list<int>> old = move(table);
            h.init(h.getM() * 2 + 1);
            table.clear(); table.resize(h.getM()); count = 0;
            for (auto& b : old) for (int v : b) {
                table[h.compute(v)].push_back(v); count++;
            }
        }
        bool contains(int id) const {
            for (int v : table[h.compute(id)]) if (v == id) return true;
            return false;
        }
        void insert(int id) {
            if (count >= (int)(table.size() * 0.7)) rehash();
            table[h.compute(id)].push_back(id); count++;
        }
    };

    VisitedSet assegnati;

    for (const auto& bucket : node_hash_table) {
        for (const auto& nodo_start : bucket) {
            if (assegnati.contains(nodo_start.getId())) continue;

            vector<int> componente;
            vector<int> coda;
            coda.push_back(nodo_start.getId());
            assegnati.insert(nodo_start.getId());

            for (size_t i = 0; i < coda.size(); ++i) {
                int corrente = coda[i];
                componente.push_back(corrente);
                const Node* n = trovaNodo(corrente);
                if (n == nullptr) continue;
                for (const auto& adj_bucket : n->getAdjacencyTable())
                    for (const auto& edge : adj_bucket) {
                        int vicino = edge.getNeighbor();
                        if (!assegnati.contains(vicino)) {
                            assegnati.insert(vicino);
                            coda.push_back(vicino);
                        }
                    }
            }
            componenti.push_back(componente);
        }
    }
    return componenti;
}

        static string leggiNomeCasuale(int fallback_index) {
            ifstream f("nomi.txt");
            if (!f.is_open()) return "Componente" + to_string(fallback_index);
            vector<string> nomi;
            string riga;
            while (getline(f, riga))
                if (!riga.empty()) nomi.push_back(riga);
            if (nomi.empty()) return "Componente" + to_string(fallback_index);
            return nomi[rand() % nomi.size()];
        }


    // ---------------------------------------------------------------
    // stampaNodo: stampa un nodo e tutta la sua lista di adiacenza
    // ---------------------------------------------------------------
    void stampaNodo(int node_id) const {
        svuotaFile("output.txt");

        const Node* n = trovaNodo(node_id);
        if (n == nullptr) {
            cout << "Nodo " << node_id << " non trovato nel grafo." << endl;
            return;
        }
        
        scriviSuFile("output.txt", "Nodo " + to_string(node_id) + " - vicini:\n");
        bool found_any = false;
        for (const auto& bucket : n->getAdjacencyTable()) {
            for (const auto& edge : bucket) {
                
                scriviSuFile("output.txt", "   -> " + to_string(edge.getNeighbor()) + " (peso: " + to_string(edge.getWeight()) + ")\n");
                cout << "finito di scrivere sul file ora puoi guardare il file output.txt" << endl;
                found_any = true;
            }
        }
        if (!found_any) cout << "   (nessun vicino)" << endl;
    }

    // ---------------------------------------------------------------
    // stampaListaNodi: stampa tutti i nodi presenti nel grafo
    // ---------------------------------------------------------------
    void stampaListaNodi() const {
        svuotaFile("output.txt");
        scriviSuFile("output.txt", "Lista dei nodi nel grafo (" + to_string(num_nodes) + " totali):\n");
        int i=0;
        auto start = chrono::high_resolution_clock::now();
        for (const auto& bucket : node_hash_table) {
            
            for (const auto& node : bucket) {
                i++;
                if(i%20==0){scriviSuFile("output.txt", "\n");}
                scriviSuFile("output.txt", "  - " + to_string(node.getId()));
            }
        }
          auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> elapsed = end - start;
        cout << "finito di scrivere sul file ora puoi guardare"<< endl;
        cout << "(operazione (scrittura) completata in " << elapsed.count() << " ms)" << endl;
    }

    // ---------------------------------------------------------------
    // infoGraph: stampa numero nodi, numero archi, nodo piu' connesso e
    // il grafico a barre delle frequenze dei pesi degli archi
    // ---------------------------------------------------------------
    void infoGraph() const {
        svuotaFile("output.txt");
        int numero_archi = 0;
        int nodo_piu_connesso = -1;
        int grado_massimo = -1;

        // vettore "istogramma dei pesi": v[p] = numero di archi di peso p
        // (ogni arco viene contato 2 volte scorrendo i nodi, quindi alla
        // fine va diviso per 2, come indicato nel pdf)
        vector<int> istogramma_pesi;

        for (const auto& bucket : node_hash_table) {
            for (const auto& node : bucket) {
                int grado_nodo = node.getNumEdges();
                numero_archi += grado_nodo;

                if (grado_nodo > grado_massimo) {
                    grado_massimo = grado_nodo;
                    nodo_piu_connesso = node.getId();
                }

                for (const auto& adj_bucket : node.getAdjacencyTable()) {
                    for (const auto& edge : adj_bucket) {
                        int p = edge.getWeight();
                        // se il vettore non e' abbastanza grande per il peso p, lo allunga
                        if ((int)istogramma_pesi.size() <= p) {
                            istogramma_pesi.resize(p + 1, 0);
                        }
                        // ogni arco viene visto da entrambi gli estremi quindi
                        // sommiamo 1 (verra' diviso per 2 alla fine, vedi sotto)
                        istogramma_pesi[p] += 1;
                    }
                }
            }
        }

        numero_archi /= 2;   // ogni arco e' stato contato 2 volte (non orientato)
        for (auto& v : istogramma_pesi) v /= 2;   // stesso discorso per l'istogramma

        
        scriviSuFile("output.txt", "===== INFO SUL GRAFO =====\n");
        
        scriviSuFile("output.txt", "Numero nodi: " + to_string(num_nodes) + "\n");
        scriviSuFile("output.txt", "Numero archi: " + to_string(numero_archi) + "\n");
        if (nodo_piu_connesso != -1) {
            scriviSuFile("output.txt", "Nodo piu' connesso: " + to_string(nodo_piu_connesso) + " (grado " + to_string(grado_massimo) + ")\n");
        }
        
        bargraph(istogramma_pesi, "Distribuzione pesi degli archi", max_edge_weight);
        cout << "finito di scrivere sul file ora puoi guardare il file output.txt" << endl;
    }

    // ---------------------------------------------------------------
    // bargraph: stampa un grafico a barre testuale del vettore v.
    // Ogni riga "peso N | ########## {count}".
    // ---------------------------------------------------------------
    // NOTA: il parametro maxValue non viene usato per lo scaling dei
    // cancelletti: come da formula del pdf, lo scaling usa il massimo
    // valore presente nel vettore v stesso (v[p]/max_in_v), non un tetto
    // globale esterno. maxValue resta nella firma per compatibilita' con
    // lo scheletro originale, ma e' qui solo a scopo informativo/futuro.
    static void bargraph(const vector<int>& v, const string& titolo, int /*maxValue*/, string file_output = "output.txt") {
        scriviSuFile(file_output, titolo + "\n");
        scriviSuFile(file_output, string(45, '_') + "\n");

        if (v.empty()) {
            scriviSuFile(file_output, "(nessun dato da stampare)\n");
            return;
        }

        // trova il valore massimo nel vettore, serve per scalare i cancelletti
        int max_in_v = 0;
        for (int val : v) if (val > max_in_v) max_in_v = val;
        if (max_in_v == 0) max_in_v = 1;   // evita divisione per zero

        // larghezza fissa della linea del terminale (k) e dello spazio
        // riservato al numero finale tra parentesi (9 caratteri: " (NNNNNN)")
        const int k = 80;          // caratteri massimi stampabili su una riga
        const int spazio_numero = 9;

        // calcola l etichetta "peso N |" piu' lunga possibile (in base al
        // numero di cifre dell'indice piu' grande del vettore)
        int max_index_digits = to_string(v.size() - 1).length();
        // "peso " (5) + cifre + " |" (2)
        int l = 5 + max_index_digits + 2;

        int x = k - (l + spazio_numero);
        if (x < 1) x = 1;   // sicurezza, non scendere mai sotto 1 cancelletto disponibile

        for (size_t p = 0; p < v.size(); ++p) {
            if (v[p] == 0) continue;   // pesi senza archi non vengono stampati

            string label = "peso " + to_string(p);
            // padding per allineare tutte le label alla stessa lunghezza l
            while ((int)label.length() < l-1) label += " ";
            label += "|";   // aggiunge il separatore verticale

            int num_cancelletti = (int)((double)x * v[p] / (double)max_in_v);
            if (num_cancelletti < 0) num_cancelletti = 0;

            scriviSuFile(file_output, label + string(num_cancelletti, '#')
                         + " {" + to_string(v[p]) + "}" + "\n");
        }
    }
};


   // =========================================================================
  // implementazione di tree::creaAlbero e tree::espandi (definite dopo
 // UniversalHashGraph perche' hanno bisogno della sua definizione completa)
// =========================================================================
void tree::creaAlbero(UniversalHashGraph& g, int nodo1, int nodo2, int cap) {
    nodo_arrivo = nodo2;
    lunghezza_massima = cap;

    radice = new pezzo(nodo1, nullptr);

    if (nodo1 == nodo2) return;   // caso degenere: partenza = arrivo, albero di un solo nodo

    espandi(g, radice, 0);
}

void tree::espandi(UniversalHashGraph& g, pezzo* corrente, int lunghezza_corrente) {
    // se questo pezzo e' gia' il nodo di arrivo, non si espande oltre
    // (e' una foglia valida, non va mai potata)
    if (corrente->nome_nodo == nodo_arrivo) {
        return;
    }

    Node* n = g.trovaNodo(corrente->nome_nodo);
    if (n == nullptr) {
        // nodo senza vicini noti nel grafo: nessun figlio possibile.
        // 'corrente' e' garantito vivo qui (siamo appena entrati nella
        // sua chiamata), quindi potarlo ora e' sicuro.
        pota(corrente);
        return;
    }

    // raccoglie tutti i vicini validi (non ancora visitati sul cammino
    // corrente, entro il cap di lunghezza) PRIMA di creare i figli, cosi'
    // non serve piu' rileggere n in seguito
    struct Candidato { int vicino; int nuova_lunghezza; };
    vector<Candidato> candidati;

    for (const auto& bucket : n->getAdjacencyTable()) {
        for (const auto& edge : bucket) {
            int vicino = edge.getNeighbor();
            int nuova_lunghezza = lunghezza_corrente + edge.getWeight();

            if (corrente->isVisited(vicino)) continue;   // niente cicli sul cammino
            if (lunghezza_massima != -1 && nuova_lunghezza > lunghezza_massima) continue;

            candidati.push_back({vicino, nuova_lunghezza});
        }
    }

    if (candidati.empty()) {
        // nessun vicino espandibile: 'corrente' e' ancora vivo (nessun
        // figlio e' stato creato), quindi potarlo ora e' sicuro
        pota(corrente);
        return;
    }

    // crea i figli e li espande uno per uno. Dopo ogni ricorsione su un
    // figlio, controlliamo SUBITO (mentre 'corrente' e' ancora garantito
    // vivo, perche' nessuno tranne noi puo' averlo potato) se quel figlio
    // e' rimasto senza figli a sua volta: in tal caso lo potiamo noi stessi,
    // qui, invece di lasciare che la potatura risalga da sola in modo
    // incontrollato.
    for (const auto& c : candidati) {
        pezzo* figlio = new pezzo(c.vicino, corrente);
        corrente->figli.push_back(figlio);

        if (c.vicino == nodo_arrivo) {
            // il figlio e' il nodo di arrivo: e' gia' una foglia valida,
            // non lo si espande oltre e non va potato
            continue;
        }

        espandi(g, figlio, c.nuova_lunghezza);

        // a questo punto 'figlio' e' stato o espanso con successo (ha
        // almeno un figlio a sua volta, oppure e' il nodo di arrivo da
        // qualche parte sotto), oppure e' stato gia' potato DA SE STESSO
        // dentro la chiamata sopra (nei due 'return; pota(corrente);'
        // iniziali di espandi). In entrambi i casi 'corrente' e' ancora
        // vivo: non e' mai stato toccato da nessuna pota() finora, perche'
        // pota() in questa implementazione non risale mai oltre il pezzo
        // passato esplicitamente.
    }

    // dopo aver tentato di espandere tutti i candidati, controlla se
    // 'corrente' e' rimasto senza figli (puo' succedere se TUTTI i figli
    // creati sono stati potati durante la loro espansione): in tal caso
    // tocca a 'corrente' stesso essere potato. Questo e' l'UNICO punto in
    // cui la potatura risale di un livello, ed e' sicuro perche' siamo
    // ancora dentro la chiamata di 'corrente', quindi e' garantito vivo.
    if (corrente->figli.empty()) {
        pota(corrente);
    }
}


     // =========================================================================
    // faiDiventareLeggibile
   // Estrae dalla riga di un file .all-paths il campo dell'AS path (secondo
  // campo separato da spazi, es. "293|1239|5779") e lo trasforma in un
 // vettore di coppie di nodi consecutivi: {(293,1239), (1239,5779)}.
// =========================================================================
vector<pair<int,int>> faiDiventareLeggibile(const string& riga) {
    vector<pair<int,int>> risultato;

    // spezza la riga sugli spazi per ottenere i campi
    stringstream ss(riga);
    string campo;
    vector<string> campi;
    while (ss >> campo) {
        campi.push_back(campo);
    }

    // il campo dell'AS path e' il secondo campo separato da spazi (indice 1):
    // es. "routeviews/routeviews|5 293|1239|5779 ..." -> campi[1] = "293|1239|5779"
    if (campi.size() < 2) return risultato;   // riga malformata, ignorala

    const string& as_path = campi[1];

    // spezza l'AS path sul carattere '|' per ottenere la sequenza di nodi
    vector<int> nodi;
    stringstream path_ss(as_path);
    string token;
    while (getline(path_ss, token, '|')) {
        if (token.empty()) continue;
        try {
            nodi.push_back(stoi(token));
        } catch (...) {
            // token non numerico: lo ignoriamo
        }
    }

    // crea le coppie di nodi consecutivi
    for (size_t i = 0; i + 1 < nodi.size(); ++i) {
        risultato.push_back({nodi[i], nodi[i+1]});
    }

    return risultato;
}


     // =========================================================================
    // Grafiamooooo
   // Legge un file .all-paths riga per riga, estrae i cammini con
  // faiDiventareLeggibile e costruisce il grafo aggiungendo via via gli
 // archi tra nodi consecutivi.
// =========================================================================
UniversalHashGraph Grafiamooooo(const string& filename, long long cap_righe=-1) {
    ifstream infile(filename);
    UniversalHashGraph g;
    string myText;

    if (!infile.is_open()) {
        cout << "il file non ce ): mi hai mentito ]:, non mi fido più " << filename << endl;
        return g;
    }

    while (getline(infile, myText) && (cap_righe == -1 || cap_righe > 0)) {
        if (myText.empty()) continue;

        auto nodi_cammino = faiDiventareLeggibile(myText);

        for (const auto& coppia : nodi_cammino) {
            g.aggiungiArco(coppia.first, coppia.second);
        }
        if (cap_righe > 0) {
            cap_righe--;
        }
    }

    infile.close();
    return g;
}


int main() {
    // inizializza il generatore di numeri casuali (usato dalle funzioni di hash)
    srand((unsigned int)time(nullptr));

    cout << "ciao dammi il nome del file o mi arrabbio (aggiungi anche il cap di righe che per gli algoritmi O((n+1)!) avidenze sperimentali mostrano che è utile)"<< endl;
    cout << "[premi invio e basta se ti senti coraggioso e vuoi che io lo legga tutto]" << endl;
    string nomefile;
    long long cap_righe = -1;
    cin >> nomefile;
    cin >> cap_righe;
        auto start = chrono::high_resolution_clock::now();
    UniversalHashGraph g = Grafiamooooo(nomefile, cap_righe);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> elapsed = end - start;
        cout << "(operazione completata in " << elapsed.count() << " ms)" << endl;



    cout << endl;
    g.stampaListaNodi();

    bool continua = true;
    while (continua) {
        cout << endl;
        cout << "Cosa vuoi fare?(dammi una riposta in fretta o mi ANNOIOOOOO)" << nomefile << endl;
        cout << "1) Stampa un nodo e i suoi vicini" << endl;
        cout << "2] Stampa la lista di tutti i nodi" << endl;
        cout << "3} Calcola la distanza minima tra due nodi" << endl;
        cout << "4) Conta i cammini min-max tra due nodi" << endl;
        cout << "5] Info sul grafo (numero nodi/archi, grafico pesi)" << endl;
        cout << "6} Cambia file da caricare" << endl;
        cout << "7) Componenti connesse" << endl;
        cout << "0/ Esci" << endl;
        cout << "Scegli (il primo giro è gratis)" << endl;

        int scelta;
        cin >> scelta;

        

        switch (scelta) {
            case 1: {
                cout << "Quale nodo? ";
                int nodo;
                cin >> nodo;

                // misura il tempo di esecuzione dell'operazione scelta
                auto start = chrono::high_resolution_clock::now();
                g.stampaNodo(nodo);
                break;
            }
            case 2: {
                g.stampaListaNodi();
                break;
            }
            case 3: {
                cout << "Nodo di partenza: ";
                int n1; cin >> n1;
                cout << "Nodo di arrivo: ";
                int n2; cin >> n2;
                    auto start = chrono::high_resolution_clock::now();
                int d = g.distanzaTraDuenodi(n1, n2);
                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double, milli> elapsed = end - start;
                if (d == -1) cout << "I due nodi non sono connessi." << endl;
                else cout << "Distanza minima: " << d << endl;
                    cout << "(operazione completata in " << elapsed.count() << " ms)" << endl;
                break;
            }
            case 4: {
                cout << "Nodo di partenza: ";
                int n1; cin >> n1;
                cout << "Nodo di arrivo: ";
                int n2; cin >> n2;
                cout << "Lunghezza massima (-1 per usare la distanza minima): ";
                int cap; cin >> cap;
                        // misura il tempo di esecuzione dell'operazione scelta
                        auto start = chrono::high_resolution_clock::now();
                int conteggio = g.camminiMinMax(n1, n2, cap);
                auto end = chrono::high_resolution_clock::now();
                        chrono::duration<double, milli> elapsed = end - start;
                cout << "Numero di cammini trovati: " << conteggio << endl;
                cout << "(operazione completata in " << elapsed.count() << " ms)" << endl;
                break;
            }
            case 5: {
                        // misura il tempo di esecuzione dell'operazione scelta
                        auto start = chrono::high_resolution_clock::now();
                g.infoGraph();
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> elapsed = end - start;
                cout << "(operazione completata in " << elapsed.count() << " ms)" << endl;
                break;
            }
            case 6: {
                cout << "Nuovo nome file [e numero di righe]: ";
                string nuovo_file;
                long long cap_righe;
                cin >> nuovo_file >> cap_righe;
                auto start = chrono::high_resolution_clock::now();
                g = Grafiamooooo(nuovo_file, cap_righe);
                g.stampaListaNodi();
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> elapsed = end - start;
                cout << "(operazione completata in " << elapsed.count() << " ms)" << endl;
                break;
            }
            case 7: {
                svuotaFile("output.txt");
                auto start = chrono::high_resolution_clock::now();
                vector<vector<int>> comp = g.componentiConnesse();
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> elapsed = end - start;

                vector<string> nomi_comp;
                for (int i = 0; i < (int)comp.size(); ++i)
                    nomi_comp.push_back(UniversalHashGraph::leggiNomeCasuale(i));

                
                string out = "===== COMPONENTI CONNESSE (" + to_string(comp.size()) + " totali) =====\n";
                
                for (int i = 0; i < (int)comp.size(); ++i)
                    out += "  [" + nomi_comp[i] + "] -> " + to_string(comp[i].size()) + " nodi\n";
                out += "(operazione completata in " + to_string(elapsed.count()) + " ms)\n";
                scriviSuFile("output.txt", out);
                cout << "file stampato con successo -> output.txt" << endl;

                bool continua_comp = true;
                while (continua_comp) {
                    cout << "\nCosa vuoi vedere?" << endl;
                    cout << "1) Stampa una componente specifica" << endl;
                    cout << "2} Stampa tutte le componenti" << endl;
                    cout << "0] Torna al menu principale" << endl;
                    int scelta_comp; cin >> scelta_comp;

                    switch (scelta_comp) {
                        case 1: {
                            for (int i = 0; i < (int)comp.size(); ++i)
                                cout << "  " << i << ") " << nomi_comp[i]
                                    << " (" << comp[i].size() << " nodi)" << endl;
                            cout << "Quale? ";
                            int idx; cin >> idx;
                            if (idx < 0 || idx >= (int)comp.size()) {
                                cout << "Indice non valido." << endl;
                                break;
                            }
                            svuotaFile("output.txt");
                            string s = "Componente [" + nomi_comp[idx] + "] ("
                                    + to_string(comp[idx].size()) + " nodi):\n";
                            for (int nodo : comp[idx])
                                s += "  - " + to_string(nodo) + "\n";
                            scriviSuFile("output.txt", s);
                            cout << "file stampato con successo -> output.txt" << endl;
                            break;
                        }
                        case 2: {
                            svuotaFile("output.txt");
                            string s = "===== TUTTE LE COMPONENTI CONNESSE =====\n";
                            for (int i = 0; i < (int)comp.size(); ++i) {
                                s += "\n[" + nomi_comp[i] + "] (" + to_string(comp[i].size()) + " nodi):\n";
                                for (int nodo : comp[i])
                                    s += "  - " + to_string(nodo) + "\n";
                            }
                            scriviSuFile("output.txt", s);
                            cout << "file stampato con successo -> output.txt" << endl;
                            break;
                        }
                        case 0:
                            continua_comp = false;
                            break;
                        default:
                            cout << "Scelta non valida." << endl;
                            break;
                    }
                }
                break;
            }
            case 0: {
                continua = false;
                cout << "Ciao ciao!" << endl;
                break;
            }
            default: {
                cout << "Scelta non valida, riprova." << endl;
                break;
            }
        }

        
      
    }

    return 0;
}