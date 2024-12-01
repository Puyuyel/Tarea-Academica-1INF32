#include <iostream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <vector>
#include <map>
#define NITERACIONES 1000
#define NIND 20
#define Tseleccion 0.5
#define Pcasamiento 0.7
#define Min_ut_k 0.65

using namespace std;

int P = 7; // Número de partes
int M = 5; // Número de máquinas
int C = 2; // Número de celdas

vector<vector<int>> r = {// Incidencia de partes a máquinas (r_ij)
    {1, 0, 1, 0, 1},
    {0, 1, 0, 1, 0},
    {1, 1, 0, 0, 1},
    {0, 0, 1, 1, 0},
    {1, 0, 0, 1, 1},
    {0, 1, 1, 0, 0},
    {1, 1, 0, 1, 0}
};

using namespace std;

bool aberracion(vector<int> cromo) {
    // Restriccion (1): Todas las máquinas están asignadas a una celda
    for (int i = P; i < P + M; i++)
        if (!cromo[i]) return true;

    // Restriccion (2): Todas las partes están asignadas a una celda
    for (int i = 0; i < P; i++)
        if (!cromo[i]) return true;

    // Restricción (3): Cada parte debe de estar en la misma celda de la máquina quen necesita para ser procesada 
    for (int i = 0; i < P; i++) {
        bool asignado = false;
        for (int j = 0; j < M; j++) {
            if (r[i][j] * cromo[P + j] == cromo[i])
                asignado = true;
        }
        if (!asignado) return true;
    }

    //Restriccion (4): Verificar que cada celda tenga una utilización mínima
    vector<int> suma1(C);
    vector<int> suma2(C);
    for (int i = 0; i < P; i++) {
        int indP = cromo[i] - 1;
        for (int j = P; j < P + M; j++) {
            int indM = cromo[j] - 1;
            if (indP == indM) {
                if (r[i][j - P]) suma1[indP]++;
                suma2[indP]++;
            }
        }
    }
    for (int i = 0; i < C; i++) {
        if (suma1[i] <= suma2[i] * Min_ut_k) return true;
    }

    return false;
}

int calculafitness(vector<int> cromo) {
    int suma = 0;
    for (int i = 0; i < P; i++) {
        int indP = cromo[i] - 1;
        for (int j = P; j < P + M; j++) {
            int indM = cromo[j] - 1;
            for (int k = 0; k < C; k++) {
                suma += (1 - 2 * r[i][j - P]) * (indM == indP);
            }
        }
    }
    return suma;
}

void muestrapoblacion(vector<vector<int>> poblacion) {
    for (int i = 0; i < poblacion.size(); i++) {
        for (int j = 0; j < poblacion[i].size(); j++) {
            cout << poblacion[i][j] << "  ";
        }
        cout << " fo=" << calculafitness(poblacion[i]) << endl;
    }
}

void generarPoblacionInicial(vector<vector<int>>&poblacion) {
    int cont = 0;
    while (cont < NIND) {
        vector<int> ind;
        for (int i = 0; i < P; i++) {
            ind.push_back((rand() % C) + 1);
        }
        for (int i = P; i < P + M; i++) {
            ind.push_back((rand() % C) + 1);
        }
        if (!aberracion(ind)) {
            poblacion.push_back(ind);
            cont++;
        }
    }
}

int muestramejor(vector<vector<int>> poblacion) {
    int mejor = 0;
    for (int i = 0; i < poblacion.size(); i++)
        if (calculafitness(poblacion[mejor]) > calculafitness(poblacion[i]))
            mejor = i;

    cout << endl << "La mejor solucion es:" << calculafitness(poblacion[mejor]) << endl;
    for (int i = 0; i < poblacion[mejor].size(); i++) {
        cout << poblacion[mejor][i] << "  ";
    }
    cout << endl;
    return calculafitness(poblacion[mejor]);
}

void calculasupervivencia(vector<vector<int>>poblacion, vector<int>&supervivencia) {
    int suma = 0;

    for (int i = 0; i < poblacion.size(); i++)
        suma += calculafitness(poblacion[i]);
    for (int i = 0; i < poblacion.size(); i++) {
        int fit = round(100 * (double) calculafitness(poblacion[i]) / suma);
        supervivencia.push_back(fit);
    }

}

void cargaruleta(vector<int>supervivencia, int *ruleta) {
    int ind = 0;
    for (int i = 0; i < supervivencia.size(); i++)
        for (int j = 0; j < supervivencia[i]; j++)
            ruleta[ind++] = i;
}

void generahijo(vector<int>padre, vector<int>madre,
        vector<int>&hijo) {
    int pos = round(padre.size() * Pcasamiento);
    for (int i = 0; i < pos; i++)
        hijo.push_back(padre[i]);
    for (int i = pos; i < madre.size(); i++)
        hijo.push_back(madre[i]);
}

void generahijoUniforme(vector<int>padre, vector<int>madre,
        vector<int>&hijo){
    int n = padre.size(); 
    for(int i=0; i<n; i++)
        if(rand()%2==0)
            hijo.push_back(padre[i]);
        else
            hijo.push_back(madre[i]);
}

void casamiento(vector<vector<int>>padres, vector<vector<int>>&poblacion) {
    for (int i = 0; i < padres.size(); i++)
        for (int j = 0; j < padres.size(); j++)
            if (i != j) {
                vector<int>cromo;
                vector<int>cromoUnif;
                generahijo(padres[i], padres[j], cromo);
                generahijoUniforme(padres[i], padres[j], cromoUnif);
                if(!aberracion(cromo))
                    poblacion.push_back(cromo);
                if(!aberracion(cromoUnif))
                    poblacion.push_back(cromoUnif);
            }
}

void seleccion(vector<vector<int>>&padres, vector<vector<int>>poblacion) {
    int ruleta[110]{-1};
    vector<int>supervivencia;
    calculasupervivencia(poblacion, supervivencia);
    cargaruleta(supervivencia, ruleta);
    int nseleccionados = poblacion.size() * Tseleccion;
    for (int i = 0; i < nseleccionados; i++) {
        int ind = rand() % 100;
        if (ruleta[ind]>-1)
            padres.push_back(poblacion[ruleta[ind]]);
    }
}

void mutacion(vector<vector<int>> &poblacion) {
    int cont = 0;
    vector<int> cromo = poblacion[rand() % poblacion.size()];
    int nmuta = rand() % (P + M);
    for (int i = nmuta; i < P + M; i++) {
        if (cromo[i] == 1)
            cromo[i] = C;
        else if (cromo[i] == C)
            cromo[i] = 1;
        else {
            double rx = (double) rand() / RAND_MAX;
            if (rx <= 0.5)
                cromo[i] -= 1;
            else
                cromo[i] += 1;
        }
    }
    if(!aberracion(cromo))
        poblacion.push_back(cromo);     
}

bool compara(vector<int>a, vector<int>b) {
    int suma = 0, sumb = 0;

    for (int i = 0; i < a.size(); i++)
        suma += calculafitness(a);
    for (int i = 0; i < b.size(); i++)
        sumb += calculafitness(b);
    return suma<sumb;
}

int compacta(vector<int>cromo) {
    int num = 0;
    for (int i = 0; i < cromo.size(); i++)
        num += pow(2, i) * cromo[i];

    return num;

}

void eliminaaberraciones(vector<vector<int>> &poblacion) {
    map<int, vector<int>> municos;

    for (int i = 0; i < poblacion.size(); i++) {
        int num = compacta(poblacion[i]);
        municos[num] = poblacion[i];
    }
    poblacion.clear();
    for (map<int, vector<int>>::iterator it = municos.begin();
            it != municos.end(); it++) {
        poblacion.push_back(it->second);
    }
}

void generarpoblacion(vector<vector<int>> &poblacion) {
    sort(poblacion.begin(), poblacion.end(), compara);
    if(poblacion.size() > NIND)
        poblacion.erase(poblacion.begin() + NIND, poblacion.end());
}

void CM() {
    int cont = 0;
    vector<vector<int>>poblacion;
    srand(time(NULL));
    generarPoblacionInicial(poblacion);
    muestrapoblacion(poblacion);

    while (1) {
        vector<vector<int>> padres;
        seleccion(padres, poblacion);
        casamiento(padres, poblacion);
        cout << endl;
        mutacion(poblacion);
        eliminaaberraciones(poblacion);
        generarpoblacion(poblacion);
        muestrapoblacion(poblacion);
        muestramejor(poblacion);
        cont++;
        if (cont == NITERACIONES) break;
    }
}

int main(int argc, char** argv) {

    CM();

    return 0;
}

