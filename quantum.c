#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "quantum.h"

#define TABLE_SIZE_FACTOR 2

// hash function
unsigned long hash(unsigned long index, unsigned long size) {
    return index % size;
}

QuantumRegister* init_register(int n_qubits) {
    QuantumRegister *reg = malloc(sizeof(QuantumRegister));
    reg->n_qubits = n_qubits;
    reg->N = 1UL << n_qubits;
    if(reg->N < 1024)
        reg->table_size = reg->N;
    else
        reg->table_size = 1024;
    reg->hash_table = calloc(reg->table_size, sizeof(StateNode*));
    
    // first state |0...0>
    set_amplitude(reg, 0, 1.0 + 0.0*I);
    return reg;
}

void set_amplitude(QuantumRegister *reg, unsigned long index, double complex val) {
    if (cabs(val) < EPSILON) return; // Не сохраняем малые значения

    unsigned long h = hash(index, reg->table_size);
    StateNode *node = reg->hash_table[h];
    while (node) {
        if (node->index == index) {
            node->amplitude = val;
            return;
        }
        node = node->next;
    }
    // create new node
    StateNode *new_node = malloc(sizeof(StateNode));
    new_node->index = index;
    new_node->amplitude = val;
    new_node->next = reg->hash_table[h];
    reg->hash_table[h] = new_node;
}

double complex get_amplitude(QuantumRegister *reg, unsigned long index) {
    unsigned long h = hash(index, reg->table_size);
    StateNode *node = reg->hash_table[h];
    while (node) {
        if (node->index == index) return node->amplitude;
        node = node->next;
    }
    return 0.0 + 0.0*I;
}

// Hadamara gate (greate superposition)
void apply_hadamard_all(QuantumRegister *reg) {
    double complex initial_amp = 1.0 / sqrt(reg->N);
    for (unsigned long i = 0; i < reg->N; i++) {
        set_amplitude(reg, i, initial_amp);
    }
}

// oracle: phase invertion for x0
void apply_oracle(QuantumRegister *reg, unsigned long target) {
    double complex amp = get_amplitude(reg, target);
    set_amplitude(reg, target, -amp);
}

// diffusion
void apply_diffusion(QuantumRegister *reg) {
    double complex sum = 0;
    // mean value
    for (int i = 0; i < reg->table_size; i++) {
        StateNode *n = reg->hash_table[i];
        while(n) {
            sum += n->amplitude;
            n = n->next;
        }
    }
    double complex avg = sum / reg->N;

    // renew amplitudes: a_i = 2*avg - a_i
    for (unsigned long i = 0; i < reg->N; i++) {
        double complex val = 2.0 * avg - get_amplitude(reg, i);
        set_amplitude(reg, i, val);
    }
}

// QFT (not ready yet)
void apply_qft(QuantumRegister *reg, bool inverse) { }

void measure(QuantumRegister *reg) {
    double max_prob = 0;
    unsigned long best_match = 0;
    int count = 0;

    for (int i = 0; i < reg->table_size; i++) {
        StateNode *n = reg->hash_table[i];
        while(n) {
            double prob = creal(n->amplitude * conj(n->amplitude));
            if (prob > max_prob) {
                max_prob = prob;
                best_match = n->index;
            }
            count++;
            n = n->next;
        }
    }
    printf("Результат: x=%lu, P(x)=%.4f, Ненулевых состояний: %d\n", best_match, max_prob, count);
}

void free_register(QuantumRegister *reg) {
    for (int i = 0; i < reg->table_size; i++) {
        StateNode *n = reg->hash_table[i];
        while(n) {
            StateNode *tmp = n;
            n = n->next;
            free(tmp);
        }
    }
    free(reg->hash_table);
    free(reg);
}

// main function
int main() {
    int n = 10;
    unsigned long x0 = 42;
    int iterations = floor(M_PI / 4.0 * sqrt(1UL << n));

    printf("Запуск алгоритма Гровера (n=%d, N=%lu, итераций=%d)\n", n, 1UL << n, iterations);

    QuantumRegister *reg = init_register(n);
    apply_hadamard_all(reg);

    for (int i = 0; i < iterations; i++) {
        apply_oracle(reg, x0);
        apply_diffusion(reg);
    }

    measure(reg);
    free_register(reg);

    return 0;
}
