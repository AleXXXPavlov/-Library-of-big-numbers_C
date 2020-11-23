
#include "bnb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

// ------------------------------------------ ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ ----------------------------------------------------

const unsigned int NOTATION = 1000000000; // система счисления 10 ^ n, в которой записаны числа в массив
const int NUM = 9; // максимальное количество цифр в любой ячейке хранения

// Функция для преобразования символа в цифру
int char_to_int(char);

// Функция для преобразования числа в символ
char int_to_char(int);

// Функция для умножения большого числа на число типа int
int bn_mul_int(bn*, int);

// Функция деления большого числа на int, возвращающая остаток
int bn_div_int(bn*, long long);

// Функция для прибавления к большому числу положительное число типа int
int bn_add_to_abs_int(bn*, int);

// Аналог присваивания
int Analog_assignment(bn*, bn*);

// Функция для нахождения расстояние между асолютными значениями чисел
int Sub_Abs(bn*, bn const*);

// Функция для нахождения произведения двух чисел школьным методом
int bn_mul_col(bn*, bn const*);

// Функиця для вычисления степени большого числа
bn* bn_pow(bn const*, int);

// Функция для добавления нулей для приведения к четному размеру массива
int bn_add_nulls_begin(bn*, size_t);

// Функция для удаления впереди идущих нулей большого числа
int Clean_Nulls_Front(bn*);

// Функция для сравнивания чисел с одинаковым знаком
int bn_abs_cmp(bn const*, bn const*);

// Функция для побитового сдвига числа вправо
int bn_shift_right(bn*);

// Функция для подсчета n-ого числа Фибоначчи
int PrintFibo(int num);

// Функция для вывода структуры в консоль
int bn_print(bn const*);

// Функия для вывода структуры, как она расположена в памяти
int bn_print_ASM(bn const*);

// Функция для ввода строки произвольной длины
char* GetStr();

// Функция для получения числа в 10-й системе счисления в виде строки
char* GiveStr(bn const*);

// Функция для взятия корня большого числа
bn* bn_sqrt(bn const*);

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
		ptr_bn = NULL;
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

	// Копирование значений полей структуры
	ptr_cbn->size = Obj->size;
	ptr_cbn->sign = Obj->sign;
	
	free(ptr_cbn->ptr_body);
	ptr_cbn->ptr_body = NULL;

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
	if (Obj == NULL || str == NULL)
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

	free(Obj->ptr_body);
	Obj->ptr_body = NULL;
	Obj->ptr_body = (int*)calloc(Obj->size, sizeof(int));
	if (Obj->ptr_body == NULL)
	{
		bn_delete(Obj);
		return BN_NO_MEMORY;
	}

	char* str_c = (char*)malloc((length + 1) * sizeof(char)); // копия исходной строки для прохода по ней со сменой некоторых значений
	if (str_c == NULL)
	{
		bn_delete(Obj);
		return BN_NO_MEMORY;
	}
	str_c[length] = '\0';
	memcpy(str_c, str, length);

	size_t j = 0; //индекс текущего элемента в массиве
	size_t j_hold;
	((length - i) % NUM == 0) ? (j_hold = Obj->size) : (j_hold = Obj->size - 1);

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

	Obj->sign = sign;
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
	free(str);

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
		bn* Obj2_c = bn_init(Obj2);
		int res_ass = Analog_assignment(Obj1, Obj2_c);

		bn_neg(Obj1);
		bn_delete(Obj2_c);

		return res_ass;
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
			bn_delete(bn_null);
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
	return bn_mul_col(Obj1, Obj2);
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
	int res_err = bn_add_nulls_begin(Obj_r, Obj1->size); // добавление ведущих нулей для инициализации
	if (res_err != BN_OK)
	{
		return res_err;
	}

	bn* Obj_cur = bn_new(); // текущее делимое

	for (long int i = Obj1->size - 1; i >= 0; --i)
	{
		res_err = bn_shift_right(Obj_cur);
		if (res_err != BN_OK)
		{
			return res_err;
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

				res_err = bn_init_int(Obj_cmp, mid_now);
				if (res_err != BN_OK)
				{
					return res_err;
				}

				int res_err = bn_mul_to(Obj_cmp, Obj2_c);
				if (res_err != BN_OK)
				{
					return res_err;
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
			res_err = bn_init_int(Obj_sub, curr_num);
			if (res_err != BN_OK)
			{
				return res_err;
			}

			res_err = bn_mul_to(Obj_sub, Obj2_c);
			if (res_err != BN_OK)
			{
				return res_err;
			}

			res_err = bn_sub_to(Obj_cur, Obj_sub);
			if (res_err != BN_OK)
			{
				return res_err;
			}
			bn_delete(Obj_sub);
		}
	}

	Obj_r->sign = Obj1->sign * Obj2->sign;
	res_err = Clean_Nulls_Front(Obj_cur);
	if (res_err != BN_OK)
	{
		return res_err;
	}

	if (Obj_r->sign == -1 && Obj_cur->sign != 0)
	{
		bn* Obj_ed = bn_new();
		Obj_ed->sign = 1;
		Obj_ed->ptr_body[0] = 1;

		res_err = bn_sub_to(Obj_r, Obj_ed);
		bn_delete(Obj_ed);

		if (res_err != BN_OK)
		{
			return res_err;
		}
	}

	bn_delete(Obj_cur);
	bn_delete(Obj2_c);

	res_err = Clean_Nulls_Front(Obj_r);
	if (res_err != BN_OK)
	{
		return res_err;
	}

    res_err = Analog_assignment(Obj1, Obj_r);
	bn_delete(Obj_r);
	return res_err;
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
		bn_delete(Obj_c);
		return res_err;
	}

	res_err = bn_mul_to(Obj_c, Obj2);
	if (res_err != BN_OK)
	{
		bn_delete(Obj_c);
		return res_err;
	}

	res_err = bn_sub_to(Obj1, Obj_c);
	if (res_err != BN_OK)
	{
		bn_delete(Obj_c);
		return res_err;
	}

	res_err = Clean_Nulls_Front(Obj1);
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

		int res_err = Analog_assignment(Obj, bn_ed);
		bn_delete(bn_ed);

		return res_err;
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
	bn_delete(Obj_c);

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
	if (root == 2)
	{
		bn* Obj_r = bn_sqrt(Obj);
		int res_err = Analog_assignment(Obj, Obj_r);
		
		bn_delete(Obj_r);
		return res_err;
	}


	if (Obj->size == 1)
	{
		bn* Obj_r = bn_new();
		int res_err = bn_init_int(Obj_r, (int)pow(Obj->ptr_body[0], 1 / (double)root));
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
	bn* Obj_curr = bn_init(Obj_r);

	for (;;)
	{
		
		if (Obj_curr->size / root) Obj_curr->size /= root;
		
		bn* Obj_oper1 = bn_pow(Obj_r, root);
		bn* Obj_oper2 = bn_pow(Obj_r, root - 1);

		res_err = bn_sub_to(Obj_oper1, Obj_c);
		if (res_err != BN_OK)
		{
			bn_delete(Obj_oper1);
			bn_delete(Obj_oper2);
			return res_err;
		}

		res_err = bn_mul_to(Obj_oper2, Obj_root);
		if (res_err != BN_OK) {
			bn_delete(Obj_oper1);
			bn_delete(Obj_oper2);
			return res_err;
		}

		res_err = bn_div_to(Obj_oper1, Obj_oper2);
		if (res_err != BN_OK) {
			bn_delete(Obj_oper1);
			bn_delete(Obj_oper2);
			return res_err;
		}

		res_err = bn_sub_to(Obj_curr, Obj_oper1);
		if (res_err != BN_OK)
		{
			bn_delete(Obj_oper1);
			bn_delete(Obj_oper2);
			return res_err;
		}

		bn_delete(Obj_oper1);
		bn_delete(Obj_oper2);

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
		
	}

	bn_delete(Obj_curr);
	bn_delete(Obj_root);

	bn* Obj_ed = bn_new();
	Obj_ed->sign = 1;
	Obj_ed->ptr_body[0] = 1;
	
	bn* Obj_oper = bn_pow(Obj_r, root);
	while (bn_cmp(Obj_oper, Obj_c) == 1)
	{
		res_err = bn_sub_to(Obj_r, Obj_ed);
		if (res_err != BN_OK)
		{
			bn_delete(Obj_oper);
			bn_delete(Obj_ed);
			bn_delete(Obj_c);
			bn_delete(Obj_r);
			return res_err;
		}

		res_err = Analog_assignment(Obj_oper, Obj_r);
		if (res_err != BN_OK) {
			bn_delete(Obj_oper);
			bn_delete(Obj_ed);
			bn_delete(Obj_c);
			bn_delete(Obj_r);
			return res_err;
		}

		res_err = bn_pow_to(Obj_oper, root);
		if (res_err != BN_OK) {
			bn_delete(Obj_oper);
			bn_delete(Obj_ed);
			bn_delete(Obj_c);
			bn_delete(Obj_r);
			return res_err;
		}
	}

	bn_delete(Obj_oper);
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
char* bn_to_string(bn const* Obj, int radix)
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

	bn* Obj_c = bn_init(Obj);

	int str_size = 10;
	size_t i = 0;
	size_t ind_minus = 0;

	char* str = (char*)malloc(str_size * sizeof(char));
	for (size_t j = 0; j < str_size; ++j)
	{
		str[j] = '\0';
	}
	if (Obj->sign == -1)
	{
		str[i] = '-';
		++i;
		++ind_minus;
	}

	int curr_mod = 0;
	while (Obj_c->sign != 0)
	{
		curr_mod = bn_div_int(Obj_c, radix);

		str[i++] = int_to_char(curr_mod);
		if (i == str_size - 1)
		{
			str_size *= 2;
			str = (char*)realloc(str, str_size * sizeof(int));
			for (size_t j = i + 1; j < str_size; ++j)
			{
				str[j] = '\0';
			}
		}
	}

	bn_delete(Obj_c);

	str = (char*)realloc(str, (i + 1) * sizeof(char));

	char* left = str + ind_minus;
	char* right = str + i - 1;

	char c;
	while (left < right)
	{
		c = *left;
		*(left++) = *right;
		*(right--) = c;
	}

	return str;
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
	if (res_mul != BN_OK)
	{
		return res_mul;
	}

	bn_delete(Obj_add);

	return BN_OK;
}

int bn_div_int(bn* Obj, long long number)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}
	if (number == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}

	long long curr_div = 0,
			  curr_mod = 0;

	for (int i = Obj->size - 1; i >= 0; --i)
	{
		curr_div = NOTATION * curr_mod + Obj->ptr_body[i];

		Obj->ptr_body[i] = curr_div / number;
		curr_mod = curr_div - Obj->ptr_body[i] * number;
	}

	int res_err = Clean_Nulls_Front(Obj);
	if (res_err != BN_OK)
	{
		return res_err;
	}

	return curr_mod;
}

int Analog_assignment(bn* Obj1, bn* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_NULL_OBJECT;
	}

	Obj1->sign = Obj2->sign;

	if (Obj1->ptr_body != NULL)
	{
		free(Obj1->ptr_body);
		Obj1->ptr_body = NULL;
	}

	Obj1->ptr_body = (int*)malloc(sizeof(int) * Obj2->size);
	if (Obj1->ptr_body == NULL)
	{
		return BN_NO_MEMORY;
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
int bn_mul_col(bn* Obj1, bn const* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_OK;
	}

	int res_err;
	bn* Obj_r = bn_new();

	if (Obj1->sign == 0 || Obj2->sign == 0)
	{
		res_err =  Analog_assignment(Obj1, Obj_r);
		bn_delete(Obj_r);
		return res_err;
	}

	int res_add = bn_add_nulls_begin(Obj_r, Obj1->size + Obj2->size);
	if (res_add != BN_OK)
	{
		return res_add;
	}

	Obj_r->sign = Obj1->sign * Obj2->sign;

	int flag;
	long long curr;

	for (size_t i = 0; i < Obj1->size; ++i)
	{
		flag = 0; // флаг переноса

		for (size_t j = 0; j < Obj2->size || flag != 0; ++j)
		{
			curr = (long long)Obj_r->ptr_body[i + j] + flag + (long long)Obj1->ptr_body[i] * (j < Obj2->size ? Obj2->ptr_body[j] : 0);
			flag = (int)(curr / NOTATION);
			Obj_r->ptr_body[i + j] = (int)(curr % NOTATION);
		}
	}

	int res_cl = Clean_Nulls_Front(Obj_r);
	if (res_cl != BN_OK)
	{
		return res_cl;
	}

	res_err = Analog_assignment(Obj1, Obj_r);
	bn_delete(Obj_r);
	return res_err;
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

	Obj->ptr_body = (int*)realloc(Obj->ptr_body, newsize * sizeof(int));
	if (Obj->ptr_body == NULL)
	{
		return BN_NO_MEMORY;
	}

	for (size_t i = Obj->size; i < newsize; ++i)
	{
		Obj->ptr_body[i] = 0;
	}

	Obj->size = newsize;
	return BN_OK;
}

int Clean_Nulls_Front(bn* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	int i = Obj->size - 1;
	for (; Obj->ptr_body[i] == 0 && i > 0; --i);
	if (Obj->ptr_body[i] == 0)
	{
		--i;
	}

	if (i == -1)
	{
		free(Obj->ptr_body);
		Obj->ptr_body = (int*)calloc(1, sizeof(int));
		Obj->size = 1;
		Obj->sign = 0;

		return BN_OK;
	}

	Obj->ptr_body = (int*)realloc(Obj->ptr_body, (i + 1) * sizeof(int));
	if (Obj->ptr_body == NULL)
	{
		return BN_NO_MEMORY;
	}

	Obj->size = i + 1;

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

	for (size_t i = Obj->size - 1; i > 0; --i)
	{
		arr[i] = arr[i - 1];
	}
	arr[0] = 0;

	Obj->ptr_body = NULL;
	Obj->ptr_body = arr;

	int res_err = Clean_Nulls_Front(Obj);

	return res_err;
}

int PrintFibo(int num)
{
	if (num < 1)
	{
		printf("Проверьте корректность введенных данных.\n");
		return BN_OK;
	}
	if (num == 1 || num == 2)
	{
		printf("1");
		return BN_OK;
	}

	bn* Obj1 = bn_new();
	Obj1->sign = 1;
	Obj1->ptr_body[0] = 1;

	bn* Obj2 = bn_new();
	Obj2->sign = 1;
	Obj2->ptr_body[0] = 1;

	int res_err;

	int j = 2;
	for (; j < num; ++j)
	{
		if (j % 2 == 0)
		{
			res_err = bn_add_to(Obj1, Obj2);
			if (res_err != BN_OK)
			{
				return res_err;
			}
		}
		else
		{
			res_err = bn_add_to(Obj2, Obj1);
			if (res_err != BN_OK)
			{
				return res_err;
			}
		}
	}

	if (j % 2 == 0)
	{
		bn_delete(Obj1);
		
		for (int i = Obj2->size - 1; i != -1; --i) {
			if (i != Obj2->size - 1 && Obj2->ptr_body[i] != 0)
			{
				int el_size = (int)ceil(log10(Obj2->ptr_body[i]));
				if ((el_size < NUM))
				{
					for (int j = 0; j < NUM - el_size; ++j)
					{
						printf("0");
					}
				}
				printf("%d", Obj2->ptr_body[i]);
			}
			else if (i == Obj2->size - 1)
			{
				printf("%d", Obj2->ptr_body[i]);
			}
			else if (i == 0 && Obj2->ptr_body[i] == 0)
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

		bn_delete(Obj2);
	}
	else
	{
		bn_delete(Obj2);
		
		for (int i = Obj1->size - 1; i != -1; --i) {
			if (i != Obj1->size - 1 && Obj1->ptr_body[i] != 0)
			{
				int el_size = (int)ceil(log10(Obj1->ptr_body[i]));
				if ((el_size < NUM))
				{
					for (int j = 0; j < NUM - el_size; ++j)
					{
						printf("0");
					}
				}
				printf("%d", Obj1->ptr_body[i]);
			}
			else if (i == Obj1->size - 1)
			{
				printf("%d", Obj1->ptr_body[i]);
			}
			else if (i == 0 && Obj1->ptr_body[i] == 0)
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

		bn_delete(Obj1);
	}

	return BN_OK;
}

bn* bn_sqrt(bn const* Obj)
{
	if (Obj == NULL)
	{
		return NULL;
	}
	if (Obj->sign < 0)
	{
		printf("\nПроверьте корректность данных\n");
		return NULL;
	}
	if (Obj->sign == 0)
	{
		bn* Obj_r = bn_new();
		return Obj_r;
	}

	int ind_now = (Obj->size + 1) / 2;

	bn* Obj_curr = bn_new();
	bn_add_nulls_begin(Obj_curr, ind_now);
	Obj_curr->size = ind_now;
	Obj_curr->sign = 1;
	
	--ind_now;
	
	bn* Obj_c = bn_new();
	int l_hold, r_hold, curr_num, k; 
	while (ind_now >= 0)
	{
		l_hold = 0;
		r_hold = NOTATION;
		curr_num = 0;
		
		while (l_hold <= r_hold)
		{
			k = (l_hold + r_hold) >> 1;
			Obj_curr->ptr_body[ind_now] = k;

			Analog_assignment(Obj_c, Obj_curr);
			bn_pow_to(Obj_c, 2);

			if (bn_cmp(Obj_c, Obj) != 1)
			{
				curr_num = k;
				l_hold = k + 1;
			}
			else
			{
				r_hold = k - 1;
			}
		}
		
		Obj_curr->ptr_body[ind_now] = curr_num;
		--ind_now;
	}

	bn_delete(Obj_c);

	int res_err = Clean_Nulls_Front(Obj_curr);
	if (res_err != BN_OK)
	{
		return NULL;
	}

	return Obj_curr;
}

int bn_print(bn const* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}

	printf("-----------------------------------------------------------\n");
	printf("Sign = %d\nLength = %ld\nAbsolute value = ", Obj->sign, Obj->size);

	const char* str = bn_to_string(Obj, 10);
	printf("%s", str);
	

	printf("\n-----------------------------------------------------------\n");

	return BN_OK;
}

int bn_print_ASM(bn const* Obj)
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

char* GetStr()
{
	int size = 0;
	int capacity = 50;

	char* str = (char*)malloc(capacity * sizeof(char));
	char c = getchar();
	if (c == '\n')
	{
		c = getchar();
	}

	while (c != '\n')
	{
		str[size++] = c;

		if (size >= capacity)
		{
			capacity *= 2;
			str = (char*)realloc(str, capacity * sizeof(char));
		}

		c = getchar();
	}
	str = (char*)realloc(str, (size + 1) * sizeof(char));
	str[size] = '\0';

	return str;
}

int main() {
	bn *a = bn_new(); // a = 0
	bn *b = bn_init(a); // b то>е = 0
	int code = bn_init_string(a, "123456789012345678");
	
	code = bn_init_int(b, 999); // b = 999
	code = bn_add_to(a, b); // a = 123456789012346677
	code = bn_pow_to(b, 5); // b = 999^5
	code = bn_root_to(b, 5); // b = 999 ###################################################################
	code = bn_init_int(a, 0); // a = 0
	code = bn_div_to(b,a); // OOPS, 999/0
	if (code != 0) {
		printf("bn_div_to failed, code=%d\n", code);
	}

	bn *c = bn_new();
	code = bn_init_int(c, 222);
	bn *d = bn_new();
	code = bn_init_int(d, 333);
	bn *e = bn_add(c, d); // e = 555
	bn *f = bn_mod(e, c); // f = 111
	char *r1 = bn_to_string(f,10); // r1 -> "111"
	printf("f=%s\n", r1);
	free(r1);

	code = bn_cmp(c, d);
	if (code < 0) {
	printf("c < d\n");
	}
	else if (code == 0) {
	printf("c == d\n");
	}
	else {
	printf("c > d\n");
	}
	code = bn_neg(b); // b = -999;
	int bsign = bn_sign(b); // bsign = -1
	code = bn_abs(b); // b = 999;
	bn_delete(f); // Gабудете удалит+ - провалите тесты
	bn_delete(e);
	bn_delete(d);
	bn_delete(c);
	bn_delete(b);
	bn_delete(a);
	return 0;
}
































