#ifndef QUANTUM_H
#define QUANTUM_H

#include <complex.h>
#include <stdbool.h>

// Порог отсечения для разреженного представления
#define EPSILON 1e-10

// Узел хэш-таблицы для хранения амплитуд
typedef struct StateNode {
    unsigned long index;        // Индекс состояния (от 0 до N-1)
    double complex amplitude;   // Комплексная амплитуда
    struct StateNode *next;
} StateNode;

// Квантовый регистр
typedef struct {
    int n_qubits;
    unsigned long N;            // 2^n
    StateNode **hash_table;     // Массив списков для хэш-таблицы
    unsigned long table_size;
} QuantumRegister;

// Основные функции
QuantumRegister* init_register(int n_qubits);
void free_register(QuantumRegister *reg);
void apply_hadamard_all(QuantumRegister *reg);
void apply_oracle(QuantumRegister *reg, unsigned long target);
void apply_diffusion(QuantumRegister *reg);
void apply_qft(QuantumRegister *reg, bool inverse);
void measure(QuantumRegister *reg);

// Вспомогательные функции хэш-таблицы
void set_amplitude(QuantumRegister *reg, unsigned long index, double complex val);
double complex get_amplitude(QuantumRegister *reg, unsigned long index);
void clean_sparse_state(QuantumRegister *reg);

#endif
