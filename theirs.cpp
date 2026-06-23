#include <libraries needed>

class universalHasch{
    private:
    long long a;
    long long b;
    long long m;

public:
    UniversalHash() : a(1), b(0), m(1) {}

    void init(long long table_size) {
        m = table_size > 0 ? table_size : 1;
        a = (rand() % (PRIME_P - 1)) + 1; 
        b = rand() % PRIME_P;             
    }

    long long compute(int x) const {
        return ((a * x + b) % PRIME_P) % m;
    }

    long long getM() const { return m; }
}

vector<(int,int)> faiDiventareLeggibile(string bho){
     // fa diventare stringhe di questo tipo routeviews/routeviews|5 293|1239|5779 206.190.32.0/19 i 134.55.24.6
    //  vettori fatti così {(293,1239),(1239,5779)}
}

void bargraph(vector<int> v, string titolo, int maxValue ){

}

struct pezzo{
    int nome pezzo
    pezzo* genitore
    vector<pezzi*> figli
    vector<int> albero genealogico //hachiato
}

class tree{
    private:
    vector<list<pezzo>>

    public:
    creaalbero(bho)

    tree aggiungiFiglio()

    void pota( int foglia){ //(inteso come tagliare non esclamazione bergamasca !!)
        //prende una voglia e taglia fino l primo ramo con 2 figli
    }

    int contaFoglie(tree alberoDaContare){
        // conta le foglie hahahahahahahha
    }

}

class edge{
private:
    int neighbor;
    int weight;
public:
    Edge(int n, int w) : neighbor(n), weight(w) {}
    int getNeighbor() const { return neighbor; }
    int getWeight() const { return weight; }
    void incrementWeight() { weight += 1; }

}

class Node {
private:
    int id;
    UniversalHash hash_func;
    vector<list<Edge>> adjacency_hash_table;
    int num_edges;

    // Il rehashing appartiene al Nodo: quando si riempie troppo, si ingrandisce da solo
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
    Node(int node_id, int expected_neighbors = 4) : id(node_id), num_edges(0) {
        hash_func.init(expected_neighbors * 2 + 1);
        adjacency_hash_table.resize(hash_func.getM());
    }

    int getId() const { return id; }

    void addOrUpdateNeighbor(int neighbor_id, int& global_max_weight) {
        // Se la tabella è troppo piena (es. > 70%), si ingrandisce prima di inserire
        if (num_edges >= hash_func.getM() * 0.7) {
            rehashAdjacency();
        }

        long long idx = hash_func.compute(neighbor_id);
        
        // Controlla se il vicino esiste già
        for (auto& edge : adjacency_hash_table[idx]) {
            if (edge.getNeighbor() == neighbor_id) {
                edge.incrementWeight();
                if (edge.getWeight() > global_max_weight) global_max_weight = edge.getWeight();
                return;
            }
        }
        
        // Se non esiste, lo aggiunge
        adjacency_hash_table[idx].push_back(Edge(neighbor_id, 1));
        num_edges++;
        if (1 > global_max_weight) global_max_weight = 1;
    }
};

class UniversalHashGraph{
    private:
        UniversalHash global_hash_func;
        vector<list<Node>> node_hash_table;
        int num_nodes;
        int max_edge_weight;

    public:
    UniversalHashGraph creagrafo(int dimensione iniziale=2000)

    int getMaxLenght()

    void aggiungiNodo()

    node* trova nodo()

    void aggiungiArco()

    tree alberoPuntato(int nodo1, int nodo2, int capsie=-1){

    }

    int distanzaTraDuenodi(){
        // come da algoritmo desctitto
    }

    int camminiMinMax(UniversalHashGraph g, int nodo1, int nodo2, int lunghezzamassima=-1){
    if(lunghezzamassima=-1){lunghezzamassima =camminominimo(nodo1,  nodo2)}
    t=tree(nodo1,nodo2,lunghezzamassima)
    return contafoglie(t);
    }


    void stampaNodo(nodo){
        //stampa il nodo e tutte le informazioni adiacenti
    }

    void stampaListaNodi(){
        // guess what
    }

    void infoGraph(){
        int a=node_hash_table.size, numarc=0,nodcon=0
        for lista

    }


}

UniversalHashGraph Grafiamooooo(string filename) {
    ifstream infile(filename);
    UniversalHashGraph g; 
    string myText;

    if (!infile.is_open()) {
        cout << "Errore: Impossibile aprire il file " << filename << endl;
        return g;
    }

    while (getline(infile, myText)) {
        if (myText.empty()) continue;

       auto nodi_cammino = faiDiventareLeggibile(myText)
        
        for (size_t i = 0; i < nodi_cammino.size() - 1; ++i) {
            int u = nodi_cammino[i];
            int v = nodi_cammino[i+1];
            g.aggiungiArco(u, v); 
        }   
    }

    infile.close(); 
    return g; 
}

int main(){

    //tutte le cose per randomizzare
    cout << "ciao dammi il nome del file o mi arrabbio"
    cin >> nomefile

    cout << printo lista nodi

    cout << "cosa vuoi fare?(dammi una riposta in fretta o mi ANNOIOOOOO)" \n
    cout << "lista delle cose da fare"
    switch case ..
                ..
                ..
                .. // tra le cose da fare ce ovviamentr chiudere tutto (in generale le cose sono piu o meno in bigezione con le funzioni private di universalHashGraph)

     // in base a che nummero poi ti chiede le informazioni necessareie per le fare le  varie funzioni
    //  poi vabbe fa la cosa e ti dice quanto di ha messo e tutte cose ppoi ti richhiede 
    
}