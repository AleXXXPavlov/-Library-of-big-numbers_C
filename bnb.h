#ifndef bnb
#define bnb

struct bn_s;
typedef struct bn_s bn;

enum bn_codes {
	BN_OK, BN_NULL_OBJECT, BN_NO_MEMORY, BN_DIVIDE_BY_ZERO
};

bn* bn_new(); // Создать новое BN
bn* bn_init(const bn*); // Создать копию существующего BN

// Инициализировать значение BN десятичным представлением строки
int bn_init_string(bn*, const char*);

// Инициализировать значение BN представлением строки
// в системе счисления: radix
int bn_init_string_radix(bn*, const char*, int);

// Инициализировать значение BN заданным целым числом
int bn_init_int(bn*, int);

// Уничтожить BN (освободить память)
int bn_delete(bn*);

// Операции, аналогичные +=, -=, *=, /=, %=
int bn_add_to(bn*, bn const*);
int bn_sub_to(bn*, bn const*);
int bn_mul_to(bn*, bn const*);
int bn_div_to(bn*, bn const*); 
int bn_mod_to(bn*, bn const*); 

// Возвести число в степень degree
int bn_pow_to(bn*, int);

// Извлечь корень степени reciprocal из BN
int bn_root_to(bn*, int); //---------------------------------------------------------------------------------

// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const*, bn const*);
bn* bn_sub(bn const*, bn const*);
bn* bn_mul(bn const*, bn const*);
bn* bn_div(bn const*, bn const*); 
bn* bn_mod(bn const*, bn const*); 

// Выдать представление BN в системе счисления radix в виде строки
// Строку после использования потребуется удалить.
char* bn_to_string(bn const*, int); //---------------------------------------------------------------------------------

// Если левое меньше, вернуть <0; если равны, вернуть 0; иначе >0
int bn_cmp(bn const*, bn const*);

int bn_neg(bn*); // Изменить знак на противоположный
int bn_abs(bn*); // Вернуть модуль
int bn_sign(bn const*); //-1 если t<0; 0 если t = 0, 1 если t>0

#endif





































