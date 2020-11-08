
#include "bnb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <locale.h>
#include <time.h>

// ------------------------------------------ ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ ----------------------------------------------------

unsigned int NOTATION = 1000000000; // система счисления 10 ^ n, в которой записаны числа в массив
int NUM = 9; // максимальное количество цифр в любой ячейке хранения

// Функция для преобразования символа в цифру
int char_to_int(char);

// Функция для преобразования числа в символ
char int_to_char(int);

// Функция для умножения большого числа на число типа int
int bn_mul_int(bn*, int);

// Функция деления большого числа на int
int bn_div_int(bn*, int);

// Функция для прибавления к большому числу положительное число типа int
int bn_add_to_abs_int(bn*, int);

// Аналог присваивания
int Analog_assignment(bn*, bn*);

// Функция для нахождения расстояние между асолютными значениями чисел
int Sub_Abs(bn*, bn const*);

// Функция для нахождения произведения двух чисел школьным методом
bn* bn_mul_col(bn*, bn const*);

// Функиця для вычисления степени большого числа
bn* bn_pow(bn const*, int);

// Функция для добавления нулей для приведения к четному размеру массива
int bn_add_nulls_begin(bn*, size_t);

// Функция для удаления впереди идущих нулей большого числа
int Clean_Nulls_Front(bn*);

// Функция для удаления лишних нулей
int Clean_Nulls_Back(bn*);

// Функция для сравнивания чисел с одинаковым знаком
int bn_abs_cmp(bn const*, bn const*);

// Функция для побитового сдвига числа вправо
int bn_shift_right(bn*);

// Функция для вывода структуры в консоль
int bn_print(bn const*); 

// Функия для вывода структуры, как она расположена в памяти
int bn_print1(bn const*);

// ------------------------------------------ КОНСТРУКТОРЫ / ДЕСТРУКТОР -----------------------------------------------------

/* Определения структуры bn и ее функций */
struct bn_s {
	int* ptr_body; // указатель на начало массива цифр
	size_t size; // размер массива
	int sign; // знак числа
};

/* Конструктор */
bn* bn_new() {
	bn* ptr_bn = (bn*)malloc(sizeof(bn)); // выделем место под структуру

	if (ptr_bn == NULL)
	{
		return NULL;
	}

	// создаем структуру со значением 0
	ptr_bn->size = 1;
	ptr_bn->sign = 0;
	ptr_bn->ptr_body = (int*)calloc(ptr_bn->size, sizeof(int));

	if (ptr_bn->ptr_body == NULL) 
	{
		free(ptr_bn);
		return NULL;
	}

	return ptr_bn;
}

/* Конструктор копирования */
bn* bn_init(const bn* Obj) {
	if (Obj == NULL) {
		return NULL;
	}

	bn* ptr_cbn = bn_new();
	if (ptr_cbn == Obj) {
		return ptr_cbn;
	}

	// Копирование значений полей структуры
	ptr_cbn->size = Obj->size;
	ptr_cbn->sign = Obj->sign;

	ptr_cbn->ptr_body = (int*)calloc(ptr_cbn->size, sizeof(int));
	if (ptr_cbn->ptr_body == NULL) {
		bn_delete(ptr_cbn);
		return NULL;
	}

	for (size_t i = 0; i < ptr_cbn->size; ++i)
	{
		ptr_cbn->ptr_body[i] = Obj->ptr_body[i];
	}

	return ptr_cbn;
}

/* Инициализация значения BN десятичным представлением строки */
int bn_init_string(bn* Obj, const char* str)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	size_t length = strlen(str); // длина строки
	size_t i = 0; // индекс текущего рассматриваемого символа
	int sign = 1; // знак числа

	if (str[0] == '-') // проверка на отрицательность
	{
		sign = -1;
		++i;
	}
	for (; i < length && str[i] == '0'; ++i);

	if (i == length) // строка закончилась
	{
		return BN_OK;
	}

	Obj->sign = sign;
	Obj->size = (int)ceil((double)(length - i) / NUM);

	size_t k = length - NUM; // первый индекс рассматриваемой подстроки
	Obj->ptr_body = (int*)calloc(Obj->size, sizeof(int));
	if (Obj->ptr_body == NULL)
	{
		return BN_NO_MEMORY;
	}

	char* str_c = (char*)malloc(length * sizeof(char)); // копия исходной строки для прохода по ней со сменой некоторых значений
	if (str_c == NULL)
	{
		return BN_NO_MEMORY;
	}
	memcpy(str_c, str, length);

	size_t j = 0; //индекс текущего элемента в массиве
	size_t j_hold;
	(length - i) % NUM == 0 ? j_hold = Obj->size : j_hold = Obj->size - 1;

	for (; j < j_hold; k -= NUM, ++j) // рассматриваем с конца подстроки длины NUM
	{
		Obj->ptr_body[j] = atoi(str_c + k);
		str_c[k] = '\0';
	}

	if (j != Obj->size) // если осталась не рассмотренная подстрока
	{
		Obj->ptr_body[Obj->size - 1] = atoi(str_c + i);
	}
	free(str_c);

	return BN_OK;
}

/* Функция для инициализации значения BN представлением строки в системе счисления: radix */
int bn_init_string_radix(bn* Obj, const char* str, int radix)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (radix <= 0)
	{
		printf("\nПроверьте корректность поставляемых данных.\n");
		return BN_OK;
	}
	
	size_t length = strlen(str); // длина строки
	size_t i = 0; // индекс текущего рассматриваемого символа
	int sign = 1; // знак числа

	if (str[0] == '-') // проверка на отрицательность
	{
		sign = -1;
		++i;
	}
	for (; i < length && str[i] == '0'; ++i);

	if (i == length) // строка закончилась
	{
		return BN_OK;
	}
	
	Obj->size = 1;
	
	int res; // код результатов операций
	for (size_t j = i; j < length; ++j)
	{	
		res = bn_mul_int(Obj, radix);
		if (res != BN_OK)
		{
			return res;
		}
		
	
		res = bn_add_to_abs_int(Obj, char_to_int(str[j]));
		if (res != BN_OK)
		{
			return res;
		}
	}
	
	return BN_OK;
}

/* Инициализация целым числом */
int bn_init_int(bn* Obj, int num)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (num == 0)
	{
		bn* Obj_null = bn_new();
		int res_ass = Analog_assignment(Obj, Obj_null);
		bn_delete(Obj_null);
		return res_ass;
	}

	int num_c = abs(num);
	
	int length = (int)ceil(log10(num_c)) + 2;
	if (num_c != (int)pow(10.0, (double)length - 2.0))		
	{
		--length;
	}
	
	char* str = (char*)malloc(length * sizeof(char));
	if (str == NULL)
	{
		return BN_NO_MEMORY;
	}
	
	str[length - 1] = '\0';
	for (int i = length - 2; i >= 0; --i)
	{
		str[i] = (num_c % 10) + '0';
		num_c /= 10;
	}
	
	int res_init = bn_init_string(Obj, str);

	if (res_init != BN_OK)
	{
		return res_init;
	}
	
	if (num < 0)
	{
		Obj->sign = -1;
	}

	return BN_OK;
}

/* Деструктор */
int bn_delete(bn* Obj) {
	if (Obj == NULL) {
		return BN_NULL_OBJECT;
	}

	if (Obj->ptr_body != NULL)
	{
		free(Obj->ptr_body);
		Obj->ptr_body = NULL;
	}
	free(Obj);
	Obj = NULL;

	return BN_OK;
}

// ------------------------------------------ ОПРЕДЕЛЕНИЯ ОСНОВНЫХ ФУНКЦИЙ ----------------------------------------------

/* Функция для сравнивания двух больших чисел */
int bn_cmp(bn const* Obj1, bn const* Obj2) {
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_NULL_OBJECT;
	}

	// сравнивание по знакам
	if (Obj1->sign > Obj2->sign)
	{
		return 1;
	}
	if (Obj1->sign < Obj2->sign)
	{
		return -1;
	}
	if (Obj1->sign == 0 && Obj2->sign == 0)
	{
		return 0;
	}

	// оба знака либо положительны, либо отрицательны
	return bn_abs_cmp(Obj1, Obj2) * Obj1->sign;
}

/* Функция для смены знака*/
int bn_neg(bn* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	Obj->sign = -(Obj->sign);
	return BN_OK;
}

/* Функция для взятия модуля */
int bn_abs(bn* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	if (Obj->sign == -1)
	{
		Obj->sign = 1;
	}
	return BN_OK;
}

/* Функция, возвращающая информацию о знаке */
int bn_sign(bn const* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	return Obj->sign;
}

/* Функция для прибавления одного большого числа к другому */
int bn_add_to(bn* Obj1, bn const* Obj2) {
	if (Obj1 == NULL || Obj2 == NULL) {
		return BN_NULL_OBJECT;
	}

	// Проверки на равенство нулю
	if (Obj2->sign == 0)
	{
		return BN_OK;
	}
	if (Obj1->sign == 0)
	{
		bn* Obj_c = bn_init(Obj2);
		int res_ass = Analog_assignment(Obj1, Obj_c);
		if (res_ass != BN_OK)
		{
			return res_ass;
		}

		return bn_delete(Obj_c);
	}

	// Сравнение знаков
	if (Obj1->sign == Obj2->sign)
	{
		/* Добавление куска памяти с 0, для сравнивания размеров */
		if (Obj1->size < Obj2->size)
		{
			int* arr = (int*)realloc(Obj1->ptr_body, Obj2->size * sizeof(int));
			if (arr == NULL)
			{
				return BN_NO_MEMORY;
			}
			else
			{
				Obj1->ptr_body = arr;
			}

			for (size_t j = Obj1->size; j < Obj2->size; ++j)
			{
				Obj1->ptr_body[j] = 0;
			}
			Obj1->size = Obj2->size;
		}

		size_t i = 0;
		int flag = 0; // параметр, сигнализирующий, о получении слишком большого числа в ячейке

		for (; i < Obj2->size; ++i) {
			Obj1->ptr_body[i] += flag + Obj2->ptr_body[i];

			flag = ((unsigned int)Obj1->ptr_body[i] >= NOTATION);
			if (flag != 0)
			{
				Obj1->ptr_body[i] -= NOTATION;
			}
		}

		if (flag != 0)
		{
			while (flag != 0 && i < Obj1->size)
			{
				Obj1->ptr_body[i] += flag;
				flag = ((unsigned int)Obj1->ptr_body[i] >= NOTATION);
				if (flag != 0)
				{
					Obj1->ptr_body[i] -= NOTATION;
					Obj1->ptr_body[i] += 1;
				}
				++i;
			}

			if (flag != 0)
			{
				int* arr = (int*)realloc(Obj1->ptr_body, (1 + Obj1->size) * sizeof(int));
				if (arr == NULL)
				{
					return BN_NO_MEMORY;
				}
				else
				{
					Obj1->ptr_body = arr;
				}
				++(Obj1->size);
				Obj1->ptr_body[Obj1->size - 1] = flag;
			}
		}

	}
	else // разные знаки 
	{
		bn* Obj_c = bn_init(Obj2); // временная копия Obj2 со знаком +
		bn_neg(Obj_c); // смена знака

		int code = bn_sub_to(Obj1, Obj_c);
		bn_delete(Obj_c);
		return code;
	}

	return BN_OK;
}

/* Функция для вычитания из одного большого числа другое */
int bn_sub_to(bn* Obj1, bn const* Obj2) {
	if (Obj1 == NULL || Obj2 == NULL) {
		return BN_NULL_OBJECT;
	}

	// Проверки на равенство нулю
	if (Obj1->sign == 0) {
		bn* Obj_c = bn_init(Obj2);
		int res_ass = Analog_assignment(Obj1, Obj_c);
		if (res_ass != BN_OK)
		{
			return res_ass;
		}
		bn_neg(Obj1);

		return bn_delete(Obj_c);
	}
	if (Obj2->sign == 0) {
		return BN_OK;
	}

	if (Obj1->sign == Obj2->sign)
	{
		int param_res = bn_cmp(Obj1, Obj2);
		
		if (param_res == 1)  // |Obj1| > |Obj2| and Obj1->sign = Obj2->sign = 1 || |Obj1| < |Obj2| and Obj1->sign = Obj2->sign = -1
		{
			return Sub_Abs(Obj1, Obj2);
		}
		else if (param_res == -1) // |Obj1| < |Obj2| and Obj1->sign = Obj2->sign = 1 || |Obj1| > |Obj2| and Obj1->sign = Obj2->sign = -1
		{
			bn* Obj_c = bn_init(Obj2);

			int res_err = Sub_Abs(Obj_c, Obj1);
			if (res_err != BN_OK)
			{
				return res_err;
			}

			res_err = Analog_assignment(Obj1, Obj_c);
			bn_neg(Obj1);
			bn_delete(Obj_c);
			
			return res_err;
		}
		else // param_res == 0
		{
			bn* bn_null = bn_new();
			int res_ass = Analog_assignment(Obj1, bn_null);
			return res_ass;
		}
	}
	else // разные знаки
	{
		bn* Obj_c = bn_init(Obj2);
		bn_neg(Obj_c);

		int code = bn_add_to(Obj1, Obj_c);
		bn_delete(Obj_c);
		return code;
	}

	return BN_OK;
}

/* Функция для умножения из одного большого числа другое */
int bn_mul_to(bn* Obj1, bn const* Obj2)
{
	bn* Obj_c = bn_mul_col(Obj1, Obj2);
	if (Obj_c == NULL)
	{
		return BN_NO_MEMORY;
	}

	int res_ass = Analog_assignment(Obj1, Obj_c);
	return res_ass;
}

/* Функция для деления одного большого числа на другое */
int bn_div_to(bn* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_NULL_OBJECT;
	}

	if (Obj2->sign == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}
	if (Obj1->sign == 0)
	{
		return BN_OK;
	}
	
	bn* Obj2_c = bn_init(Obj2); // копия второго объекта
	bn_abs(Obj2_c); // делаем знак положительным

	bn* Obj_r = bn_new(); // результат деления
	int res_add = bn_add_nulls_begin(Obj_r, Obj1->size); // добавление ведущих нулей для инициализации
	if (res_add != BN_OK)
	{
		return res_add;
	}

	bn* Obj_cur = bn_new(); // текущее делимое

	for (long int i = Obj1->size - 1; i >= 0; --i)
	{
		int res_sh = bn_shift_right(Obj_cur);
		if (res_sh != BN_OK)
		{
			return res_sh;
		}

		Obj_cur->ptr_body[0] = Obj1->ptr_body[i];
		Obj_cur->sign = 1;
		
		int curr_num = 0; // результат деления Obj_cur на делитель
		unsigned int l_hold = 0, // левая граница диапазона значения искомой ячейки числа
					 r_hold = NOTATION; // правая граница диапазона значения искомой ячейки числа 
		
		if (bn_cmp(Obj_cur, Obj2_c) != -1)
		{
			bn* Obj_cmp = bn_new(); // большое число для бинарного поиска

			while (l_hold <= r_hold)
			{
				int mid_now = (l_hold + r_hold) / 2;

				int res_init_int = bn_init_int(Obj_cmp, mid_now);
				if (res_init_int != BN_OK)
				{
					return res_init_int;
				}

				int res_mul1 = bn_mul_to(Obj_cmp, Obj2_c);
				if (res_mul1 != BN_OK)
				{
					return res_mul1;
				}

				int res_cmp1 = bn_cmp(Obj_cmp, Obj_cur);
				if (res_cmp1 != 1)
				{
					curr_num = mid_now;
					l_hold = mid_now + 1;
				}
				else
				{
					r_hold = mid_now - 1;
				}
			}
			
			bn_delete(Obj_cmp);
		}
		
		Obj_r->ptr_body[i] = curr_num;
		
		if (curr_num != 0)
		{
			bn* Obj_sub = bn_new();
			int res_init_int = bn_init_int(Obj_sub, curr_num);
			if (res_init_int != BN_OK)
			{
				return res_init_int;
			}

			int res_mul = bn_mul_to(Obj_sub, Obj2_c);
			if (res_mul != BN_OK)
			{
				return res_mul;
			}
			
			int res_sb = bn_sub_to(Obj_cur, Obj_sub);
			if (res_sb != BN_OK)
			{
				return res_mul;
			}
			bn_delete(Obj_sub);
		}
	}
	
	Obj_r->sign = Obj1->sign * Obj2->sign;
	if (Obj_cur->sign != 0 && Obj_r->sign == -1)
	{
		bn* Obj_ed = bn_new();
		Obj_ed->sign = 1;
		Obj_ed->ptr_body[0] = 1;

		int res_sub = bn_sub_to(Obj_r, Obj_ed);
		bn_delete(Obj_ed);

		if (res_sub != BN_OK)
		{
			return res_sub;
		}
	}

	bn_delete(Obj_cur); 
	bn_delete(Obj2_c);

	int res_cl = Clean_Nulls_Front(Obj_r);
	if (res_cl != BN_OK)
	{
		return res_cl;
	}
	if (Obj_r->ptr_body == NULL) // для случая, когда делимое меньше делителя 
	{
		bn_delete(Obj_r);
		bn* Obj_null = bn_new();
		
		int res_ass =  Analog_assignment(Obj1, Obj_null);
		bn_delete(Obj_null);
		
		return res_ass;
	}

	int res_ass = Analog_assignment(Obj1, Obj_r);
	bn_delete(Obj_r);
	return res_ass;
}

/* Функция для взятие остатка числа */
int bn_mod_to(bn* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_NULL_OBJECT;
	}

	if (Obj2->sign == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}
	if (Obj1->sign == 0)
	{
		return BN_OK;
	}

	bn* Obj_c = bn_init(Obj1);
	
	int res_err = bn_div_to(Obj_c, Obj2);
	if (res_err != BN_OK)
	{
		return res_err;
	}
	
	res_err = bn_mul_to(Obj_c, Obj2);
	if (res_err != BN_OK)
	{
		return res_err;
	}
	
	res_err = bn_sub_to(Obj1, Obj_c);
	if (res_err != BN_OK)
	{
		return res_err;
	}
	
	res_err = Clean_Nulls_Front(Obj1);
	if (res_err != BN_OK)
	{
		return res_err;
	}

	bn_delete(Obj_c);

	return res_err;
}

/* Функция для быстрого возведения в степень */
int bn_pow_to(bn* Obj, int degree)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	if (degree == 0) // степень - ноль 
	{
		bn* bn_ed = bn_new();
		bn_ed->sign = 1;
		bn_ed->ptr_body[0] = 1;

		int result = Analog_assignment(Obj, bn_ed);
		bn_delete(bn_ed);

		return result;
	}
	if (degree == 1 || Obj->sign == 0)
	{
		return BN_OK;
	}

	int abs_degree = abs(degree); // для возведения только в неотрицательную степень

	bn* Obj_c = bn_new();
	Obj_c->sign = 1;
	Obj_c->ptr_body[0] = 1;

	while (abs_degree)
	{
		if (!(abs_degree & 1))
		{
			abs_degree >>= 1;

			int res_mul = bn_mul_to(Obj, Obj);
			if (res_mul != BN_OK)
			{
				return res_mul;
			}
		}
		else
		{
			--abs_degree;

			int res_mul = bn_mul_to(Obj_c, Obj);
			if (res_mul != BN_OK)
			{
				return res_mul;
			}
		}
	}

	int res_ass = Analog_assignment(Obj, Obj_c);
	return res_ass;
}

/* Функция для взятия корня большого числа */
int bn_root_to(bn* Obj, int root)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (root < 1)
	{
		printf("\nПроверьте корректность передаваемых данных.\n");
		return BN_OK;
	}
	if (Obj->sign == 0)
	{
		return BN_OK;
	}
	if (root % 2 == 0 && Obj->sign == -1)
	{
		printf("\nОперация взятия такого корня из отрицательного числа невозможна.\n");
		return BN_OK;
	}
	
	if (Obj->size == 1)
	{
		bn* Obj_r = bn_new();
		int res_err = bn_init_int(Obj_r, pow(Obj->ptr_body[0], 1 / (double)root));
		if (res_err != BN_OK)
		{
			return res_err;
		}
		Obj_r->sign = Obj->sign;
		
		res_err = Analog_assignment(Obj, Obj_r);
		bn_delete(Obj_r);

		return res_err;
	}

	bn* Obj_r = bn_new();
	Obj_r->sign = 1;
	Obj_r->ptr_body[0] = 1;

	bn* Obj_root = bn_new();
	int res_err = bn_init_int(Obj_root, root);
	if (res_err != BN_OK)
	{
		return res_err;
	}

	bn* Obj_c = bn_init(Obj);
	res_err = bn_abs(Obj_c);
	if (res_err != BN_OK)
	{
		return res_err;
	}

	bool ind_r = false;

	for (;;)
	{
		bn* Obj_curr = bn_init(Obj_r);

		res_err = bn_sub_to(Obj_curr, bn_div(bn_sub(bn_pow(Obj_r, root), Obj_c), bn_mul(Obj_root, bn_pow(Obj_r, root-1))));
		if (res_err != BN_OK)
		{
			return res_err;
		}
		
		if (bn_cmp(Obj_r, Obj_curr) == 0 || (bn_cmp(Obj_r, Obj_curr) < 0 && ind_r))
		{
			break;
		}

		ind_r = bn_cmp(Obj_r, Obj_curr);

		res_err = Analog_assignment(Obj_r, Obj_curr);
		if (res_err != BN_OK)
		{
			return res_err;
		}

		bn_delete(Obj_curr);
	}

	bn* Obj_ed = bn_new();
	Obj_ed->sign = 1;
	Obj_ed->ptr_body[0] = 1;

	while (bn_cmp(bn_pow(Obj_r, root), Obj_c) == 1)
	{
		res_err = bn_sub_to(Obj_r, Obj_ed);
		if (res_err != BN_OK)
		{
			bn_delete(Obj_ed);
			return res_err;
		}
	}

	bn_delete(Obj_ed);
	bn_delete(Obj_c);

	Obj_r->sign = Obj->sign;
	res_err = Analog_assignment(Obj, Obj_r);
	bn_delete(Obj_r);

	return res_err;
}

/* Функция для суммы двух больших чисел */
bn* bn_add(bn const* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return NULL;
	}

	bn* Obj_r = bn_init(Obj1);

	int code = bn_add_to(Obj_r, Obj2);
	if (code != BN_OK)
	{
		return NULL;
	}

	return Obj_r;
}

/* Функция для вычитания двух больших чисел */
bn* bn_sub(bn const* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return NULL;
	}

	bn* Obj_r = bn_init(Obj1);

	int code = bn_sub_to(Obj_r, Obj2);
	if (code != BN_OK)
	{
		return NULL;
	}

	return Obj_r;
}

/* Функиця для вычисления произведения двух больших чисел */
bn* bn_mul(bn const* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return NULL;
	}

	bn* Obj_r = bn_init(Obj1);

	int code = bn_mul_to(Obj_r, Obj2);
	if (code != BN_OK)
	{
		return NULL;
	}

	return Obj_r;
}

/* Функиця для вычисления деления двух больших чисел */
bn* bn_div(bn const* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return NULL;
	}

	bn* Obj_r = bn_init(Obj1);

	int code = bn_div_to(Obj_r, Obj2);
	if (code != BN_OK)
	{
		return NULL;
	}

	return Obj_r;
}

/* Функиця для вычисления остатка от деления двух больших чисел */
bn* bn_mod(bn const* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return NULL;
	}

	bn* Obj_r = bn_init(Obj1);
	
	int code = bn_mod_to(Obj_r, Obj2);
	if (code != BN_OK)
	{
		return NULL;
	}

	return Obj_r;
}

/* Функция для предстваления BN в некоторой системе счисления radix в виде строки */
const char* bn_to_string(bn const* Obj, int radix)
{
	if (Obj == NULL)
	{
		return NULL;
	}
	if (radix <= 0)
	{
		printf("\nПроверьте корректность поставляемых данных.\n");
		return NULL;
	}
	if (Obj->sign == 0)
	{
		char* str_r = (char*)malloc(2 * sizeof(char));
		if (str_r == NULL)
		{
			return NULL;
		}

		str_r[0] = '0';
		str_r[1] = '\0';
		return str_r;
	}
	
	bn* Obj_rad = bn_new();
	int res_err = bn_init_int(Obj_rad, radix);
	if (res_err != BN_OK)
	{
		return NULL;
	}
	
	bn* Obj_c = bn_init(Obj);
	bn_abs(Obj_c);

	char* str = (char*)malloc((Obj->size * NUM + 1) * sizeof(char));
	str[Obj->size * NUM] = '\0';

	size_t i = 0;
	
	while (Obj_c->sign != 0)
	{
		bn* Obj_mod = bn_mod(Obj_c, Obj_rad);
		
		if (Obj_mod == NULL) 
		{
			bn_delete(Obj_c);
			bn_delete(Obj_rad);
			return NULL;
		}

		str[Obj->size * NUM - (i++) - 1] = int_to_char(Obj_mod->ptr_body[0]);

		res_err = bn_div_to(Obj_c, Obj_rad);
		if (res_err != BN_OK)
		{
			return NULL;
		}

		bn_delete(Obj_mod);
	}
	
	bn_delete(Obj_rad);
	bn_delete(Obj_c);
	
	char* str_r;
	size_t k = 0;

	if (Obj->sign == -1)
	{
		str_r = (char*)malloc((i + 1) * sizeof(char));
		if (str_r == NULL)
		{
			free(str);
			return NULL;
		}
		str_r[k++] = '-';
	}
	else
	{
		str_r = (char*)malloc(i * sizeof(char));
		if (str_r == NULL)
		{
			free(str);
			return NULL;
		}
	}

	for (size_t j = Obj->size * NUM - i; j < Obj->size * NUM + 1; ++j, ++k)
	{
		str_r[k] = str[j];
	}


	free(str);

	return str_r;
}

// -------------------------------------- ОПРЕДЕЛЕНИЯ ДОПОЛНИТЕЛЬНЫХ ФУНКЦИЙ -------------------------------------------
int char_to_int(char character)
{
	if (isdigit(character)) // если символ от 0 до 9
	{
		return character - '0';
	}
	if (isupper(character))
	{
		return character - 'A' + 10;
	}
	if (islower(character))
	{
		return character - 'a' + 10;
	}
	return BN_OK;
}

char int_to_char(int number)
{
	if (number < 10)
	{
		return number + '0';
	}
	return number + 'A' - 10;
}
int bn_mul_int(bn* Obj, int number)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	if (Obj->sign == 0 || number == 1)
	{
		return BN_OK;
	}
	if (number == -1)
	{
		return bn_abs(Obj);
	}
	if (number == 0)
	{
		bn* Obj_null = bn_new();
		int res_ass = Analog_assignment(Obj, Obj_null);
		bn_delete(Obj_null);

		return res_ass;
	}
	
	bn* Obj_mul = bn_new();
	int res_init = bn_init_int(Obj_mul, number);
	if (res_init != BN_OK)
	{
		return res_init;
	}

	int res_mul = bn_mul_to(Obj, Obj_mul);
	if (res_mul != BN_OK)
	{
		return res_mul;
	}

	bn_delete(Obj_mul);

	return res_mul; 
}

int bn_add_to_abs_int(bn* Obj, int number)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (number == 0)
	{
		return BN_OK;
	}

	bn* Obj_add = bn_new();
	int res_init = bn_init_int(Obj_add, number);
	if (res_init != BN_OK)
	{
		return res_init;
	}

	int res_mul = bn_add_to(Obj, Obj_add);
	bn_delete(Obj_add);

	return BN_OK;
}

int bn_div_int(bn* Obj, int number)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (number == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}

	if (number < 0)
	{
		number = abs(number);
		bn_neg(Obj);
	}

	bn* Obj_num = bn_new();
	int res_init = bn_init_int(Obj_num, number);
	if (res_init != BN_OK)
	{
		return res_init;
	}

	int res_div = bn_div_to(Obj, Obj_num);
	bn_delete(Obj_num);

	return res_div;
}

int Analog_assignment(bn* Obj1, bn* Obj2)
{

	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_NULL_OBJECT;
	}

	Obj1->sign = Obj2->sign;

	int* arr = (int*)realloc(Obj1->ptr_body, sizeof(int) * Obj2->size);
	if (arr == NULL)
	{
		return BN_NO_MEMORY;
	}
	else
	{
		Obj1->ptr_body = arr;
	}

	for (size_t i = 0; i < Obj2->size; ++i)
	{
		Obj1->ptr_body[i] = Obj2->ptr_body[i];
	}

	Obj1->size = Obj2->size;

	return BN_OK;
}

/* Функция для разности двух модулей */
int Sub_Abs(bn* Obj1, bn const* Obj2) {
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_NO_MEMORY;
	}

	int flag = 0; // параметр, сигнализирующий, о получении слишком маленького числа в ячейке
	size_t i = 0;

	for (; i < Obj1->size || flag != 0; ++i) {
		Obj1->ptr_body[i] -= flag + (i < Obj2->size ? Obj2->ptr_body[i] : 0);
		flag = Obj1->ptr_body[i] < 0;

		if (flag != 0)
		{
			Obj1->ptr_body[i] += NOTATION;
		}
	}

	return Clean_Nulls_Front(Obj1);
}

/* Функция для школьного перемножения */
bn* bn_mul_col(bn* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return NULL;
	}
	if (Obj1->sign == 0 || Obj2->sign == 0)
	{
		bn* Obj_r = bn_new(); // возвращаем нулевой объект
		return Obj_r;
	}

	size_t maxsize = Obj1->size + Obj2->size;

	bn* Obj_r = bn_new();
	int res_add = bn_add_nulls_begin(Obj_r, maxsize);
	if (res_add != BN_OK)
	{
		return NULL;
	}

	Obj_r->sign = Obj1->sign * Obj2->sign;

	for (size_t i = 0; i < Obj1->size; ++i)
	{
		int flag = 0; // флаг переноса

		for (size_t j = 0; j < Obj2->size || flag != 0; ++j)
		{
			long long curr = (long long)Obj_r->ptr_body[i + j] + flag + (long long)Obj1->ptr_body[i] * (j < Obj2->size ? Obj2->ptr_body[j] : 0);
			flag = (int)(curr / NOTATION);
			Obj_r->ptr_body[i + j] = (int)(curr % NOTATION);
		}
	}
	
	int res_cl = Clean_Nulls_Front(Obj_r);
	if (res_cl != BN_OK)
	{
		return NULL;
	}
	
	return Obj_r;
}

bn* bn_pow(bn const* Obj, int degree)
{
	if (Obj == NULL)
	{
		return NULL;
	}

	bn* Obj_r = bn_init(Obj);

	int code = bn_pow_to(Obj_r, degree);
	if (code != BN_OK)
	{
		return NULL;
	}

	return Obj_r;
}

int bn_add_nulls_begin(bn* Obj, size_t newsize)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (Obj->size > newsize)
	{
		return BN_NO_MEMORY;
	}
	if (Obj->size == newsize)
	{
		return BN_OK;
	}
	
	int* arr = (int*)realloc(Obj->ptr_body, newsize * sizeof(int));
	if (arr == NULL)
	{
		return BN_NO_MEMORY;
	}
	else
	{
		Obj->ptr_body = arr;

		for (size_t i = Obj->size; i < newsize; ++i)
		{
			Obj->ptr_body[i] = 0;
		}

		Obj->size = newsize;
		return BN_OK;
	}
}

int Clean_Nulls_Front(bn* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	size_t i = Obj->size - 1;
	for (; Obj->ptr_body[i] == 0; --i);

	if (i + 1 == 0)
	{
		free(Obj->ptr_body);
		Obj->ptr_body = (int*)calloc(1, sizeof(int));

		Obj->size = 1;
		Obj->sign = 0;
		return BN_OK;;
	}
	
	int* arr = (int*)calloc((i+1), sizeof(int));
	if (arr == NULL)
	{
		return BN_NO_MEMORY;
	}
	
	for (size_t j = 0; j < i + 1; ++j)
	{
		arr[j] = Obj->ptr_body[j];
	}

	Obj->ptr_body = arr;
	Obj->size = i + 1;

	return BN_OK;
}

int Clean_Nulls_Back(bn* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	size_t i = 0;
	for (; Obj->ptr_body[i] == 0; ++i);

	for (size_t j = 0; j < Obj->size - i; ++j)
	{
		Obj->ptr_body[j] = Obj->ptr_body[j + i];
	}
	
	int* arr = (int*)realloc(Obj->ptr_body, (Obj->size - i) * sizeof(int));
	if (arr == NULL)
	{
		return BN_NO_MEMORY;
	}
	else
	{
		Obj->ptr_body = arr;
		Obj->size -= i;
	}

	return BN_OK;
}

int bn_abs_cmp(bn const* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_NULL_OBJECT;
	}

	if (Obj1->size != Obj2->size) {
		int param_res0 = 0;
		if (Obj1->size > Obj2->size)
		{
			param_res0 = 1;
		}
		else if (Obj1->size < Obj2->size)
		{
			param_res0 = -1;
		}

		return param_res0 * Obj1->sign;
	}
	else // размеры равны
	{
		int param_res = 0;
		for (int i = Obj1->size - 1; i >= 0; --i)
		{
			if (Obj1->ptr_body[i] > Obj2->ptr_body[i])
			{
				param_res = 1;
				break;
			}
			else if (Obj1->ptr_body[i] < Obj2->ptr_body[i])
			{
				param_res = -1;
				break;
			}
		}

		return param_res * Obj1->sign;
	}
}

int bn_shift_right(bn* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	int* arr = (int*)realloc(Obj->ptr_body, (Obj->size + 1) * sizeof(int));
	if (arr == NULL)
	{
		return BN_NO_MEMORY;
	}

	arr[Obj->size] = 0;
	++Obj->size;
	Obj->ptr_body = arr;
	
	for (size_t i = Obj->size - 1; i > 0; --i)
	{
		Obj->ptr_body[i] = Obj->ptr_body[i - 1];
	}
	Obj->ptr_body[0] = 0;

	int res_err = Clean_Nulls_Front(Obj);
	if (res_err != BN_OK)
	{
		return res_err;
	}

	if (Obj == NULL)
	{
		bn* Obj_null = bn_new();
		res_err = Analog_assignment(Obj, Obj_null);
		bn_delete(Obj_null);

		return res_err;
	}

	return BN_OK;
}

int bn_print(bn const* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	printf("-----------------------------------------------------------\n");
	printf("Sign = %d\nLength = %ld\nAbsolute value = ", Obj->sign, Obj->size);

	for (int i = Obj->size - 1; i != -1; --i) {
		if (i != Obj->size - 1 && Obj->ptr_body[i] != 0)
		{
			int el_size = (int)ceil(log10(Obj->ptr_body[i]));
			if ((el_size < NUM))
			{
				for (int j = 0; j < NUM - el_size; ++j)
				{
					printf("0");
				}
			}
			printf("%d", Obj->ptr_body[i]);
		}
		else if (i == Obj->size - 1)
		{
			printf("%d", Obj->ptr_body[i]);
		}
		else if (i == 0 && Obj->ptr_body[i] == 0)
		{
			printf("%d", 0);
		}
		else // i != 0 && i != Obj->size - 1 && Obj->ptr_body[i] = 0
		{
			for (int i = 0; i < NUM; ++i)
			{
				printf("%d", 0);
			}
		}
	}
	printf("\n-----------------------------------------------------------\n");

	return BN_OK;
}

int bn_print1(bn const* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	printf("-----------------------------------------------------------\n");
	printf("Sign = %d\nLength = %ld\nAbsolute value = ", Obj->sign, Obj->size);

	for (int i = 0; i != Obj->size; ++i) {
		printf(" \\ %d \\ ", Obj->ptr_body[i]);
	}
	printf("\n-----------------------------------------------------------\n");

	return BN_OK;
}

// ---------------------------------------------------------------------------------------------------

int main()
{
	setlocale(LC_ALL, "RUS");

	bn* bn1 = bn_new();
	bn_init_string(bn1, "5647589345883495834568934583495834725687345438573458745468593568370160457");
	bn_print(bn1);

	unsigned int start = clock();
	int res = bn_root_to(bn1, 33);
	unsigned int end = clock();
	bn_print(bn1);

	printf("\nВремя работы в мс: %d\n\n", (end - start) / CLOCKS_PER_SEC);

	return 0;
}


