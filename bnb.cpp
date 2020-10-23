#include "bnb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int NOTATION = 10; // система счисления, с которой записаны числа в массив

int Sub_Abs(int*, int*, size_t, size_t);
int Analog_assignment(bn*, bn*);
int* bn_add_nulls(int*, size_t, size_t);
bn* bn_mul_to_col(bn*, bn*);

/* Определения структуры bn и ее функций */
struct bn_s {
	int* ptr_body; // указатель на начало массива цифр
	size_t size; // размер массива
	int sign; // знак числа
};

/* Конструктор */
bn* bn_new() {
	bn* ptr_bn = (bn*)malloc(sizeof(bn));

	if (ptr_bn == NULL) {
		return NULL;
	}

	ptr_bn->size = 1;
	ptr_bn->sign = 0;
	ptr_bn->ptr_body = calloc(ptr_bn->size, sizeof(int));

	if (ptr_bn->ptr_body == NULL) {
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
	if (ptr_cbn == NULL) {
		return NULL;
	}

	ptr_cbn->size = Obj->size;
	ptr_cbn->sign = Obj->sign;

	ptr_cbn->ptr_body = calloc(ptr_cbn->size, sizeof(int));
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

/* Деструктор */
int bn_delete(bn* Obj) {
	if (Obj == NULL) {
		return BN_NULL_OBJECT;
	}

	if (Obj->ptr_body != NULL)
		free(Obj->ptr_body);

	free(Obj);

	return BN_OK;
}

/* Инициализация числа представлением строки в 10-й системе счисления */
int bn_init_string(bn* Obj, const char* str) {
	if (Obj == NULL) {
		return BN_NULL_OBJECT;
	}

	size_t length = strlen(str);
	size_t i = 0;

	if (str[0] == '-')
	{
		Obj->sign = -1;
		++i;
	}
	if (i == 0) {
		while (i < length)
		{
			if (str[i] == '0')
			{
				++i;
			}
			else
			{
				break;
			}
		}
	}

	if (i < length) {
		if (Obj->sign == 0)
		{
			Obj->sign = 1;
		}
		Obj->size = length - i;

		int* arr_num = (int*)calloc(Obj->size, sizeof(int));
		if (arr_num != NULL) {
			for (size_t j = 0; j < Obj->size; ++j) {
				arr_num[j] = str[length - j - 1] & 0x0F;
			}
		}

		Obj->ptr_body = arr_num;
	}

	return BN_OK;
}

/* Инициализация числа представлением строки в некоторой системе счисления */
int bn_init_string_radix(bn* Obj, const char* str, int radix) {
	if (Obj == NULL) {
		return BN_NULL_OBJECT;
	}

	return BN_OK;
}

/* Функция для прибавления одного большого числа к другому */
int bn_add_to(bn* Obj1, bn const* Obj2) {
	if (Obj1 == NULL || Obj2 == NULL) {
		return BN_NULL_OBJECT;
	}

	// Проверки на равенство нулю
	if (Obj1->sign == 0) {
		Obj1 = bn_init(Obj2);
		return BN_OK;
	}
	if (Obj2->sign == 0) {
		return BN_OK;
	}

	// Сравнение знаков

	if (Obj1->sign == Obj2->sign)
	{
		/* Добавление куска памяти с 0, для сравнивания размеров */
		if (Obj1->size < Obj2->size) {

			Obj1->ptr_body = (int*)realloc(Obj1->ptr_body, Obj2->size * sizeof(int));
			if (Obj1->ptr_body == NULL)
			{
				return BN_NO_MEMORY;
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

			flag = (Obj1->ptr_body[i] >= NOTATION);
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
				flag = (Obj1->ptr_body[i] >= NOTATION);
				if (flag != 0)
				{
					Obj1->ptr_body[i] -= NOTATION;
					Obj1->ptr_body[i] += 1;
				}
				++i;
			}

			if (flag != 0)
			{
				Obj1->ptr_body = (int*)realloc(Obj1->ptr_body, (1 + Obj1->size) * sizeof(int));
				if (Obj1->ptr_body == NULL)
				{
					return BN_NO_MEMORY;
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
		Obj1 = bn_init(Obj2);
		bn_neg(Obj1);
		return BN_OK;
	}
	if (Obj2->sign == 0) {
		return BN_OK;
	}

	if (Obj1->sign == Obj2->sign)
	{
		int param_res = bn_cmp(Obj1, Obj2);

		if ((param_res * Obj1->sign) == 1)  // |Obj1| > |Obj2| and Obj1->sign = Obj2->sign = 1 || |Obj1| > |Obj2| and Obj1->sign = Obj2->sign = -1
		{
			return Sub_Abs(Obj1->ptr_body, Obj2->ptr_body, Obj1->size, Obj2->size);
		}
		else if ((param_res * Obj1->sign) == -1) // |Obj1| < |Obj2| and Obj1->sign = Obj2->sign = 1 || |Obj1| < |Obj2| and Obj1->sign = Obj2->sign = -1
		{
			bn* Obj_c = bn_init(Obj2);

			int result = Sub_Abs(Obj_c->ptr_body, Obj1->ptr_body, Obj_c->size, Obj1->size);
			Analog_assignment(Obj1, Obj_c);
			bn_neg(Obj1);

			bn_delete(Obj_c);
			return result;		
		}
		else // param_res == 0
		{
			bn* bn_null = bn_new();
			Obj1 = bn_init(bn_null);
			return BN_OK;
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

/* Функция для школьного умножения */
bn* bn_mul_to_col(bn* Obj1, bn* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return NULL;
	}
	if (Obj1->sign == 0 || Obj2->sign == 0)
	{
		bn* Obj_r = bn_new();
		return Obj_r;
	}

	// Дополняем числа нулями для равенства обоих размеров одному четному числу
	size_t maxsize = Obj1->size + Obj2->size;

	// Реализация наивного алгоритма перемножения
	bn* Obj_r = bn_new();
	Obj_r->ptr_body = bn_add_nulls(Obj_r->ptr_body, Obj_r->size, maxsize);
	Obj_r->size = maxsize;
	Obj_r->sign = Obj1->sign * Obj2->sign;

	for (size_t i = 0; i < Obj2->size; ++i)
	{
		int flag = 0; // флаг переноса

		for (size_t j = 0; j < Obj1->size; ++j)
		{
			Obj_r->ptr_body[i + j] += flag + Obj1->ptr_body[j] * Obj2->ptr_body[i];
			flag = Obj_r->ptr_body[i + j] / NOTATION;
			Obj_r->ptr_body[i + j] %= NOTATION;
		}

		Obj_r->ptr_body[i + Obj1->size] += flag;
	}


	return Obj_r;
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

/* Функция для разности двух модулей */
int Sub_Abs(int* ptr_body1, int* ptr_body2, size_t size1, size_t size2) {
	if (ptr_body1 == NULL || ptr_body2 == NULL)
	{
		return BN_NO_MEMORY;
	}

	int flag = 0; // параметр, сигнализирующий, о получении слишком маленького числа в ячейке
	size_t i = 0;

	for (; i < size1; ++i) {
		ptr_body1[i] -= flag + (i < size2 ? ptr_body2[i] : 0);
		flag = ptr_body1[i] < 0;

		if (flag != 0)
		{
			ptr_body1[i] += NOTATION;
		}
	}

	int count_zero = 0;
	if (ptr_body1[size1 - 1 - count_zero] == 0)
	{
		while (ptr_body1[size1 - 1 - count_zero] == 0)
		{
			++count_zero;
		}

		ptr_body1 = (int*)realloc(ptr_body1, (size1 - count_zero) * sizeof(int));
		if (ptr_body1 == NULL)
		{
			return BN_NO_MEMORY;
		}
	}
	size1 -= count_zero;;
	

	return BN_OK;
}

/* Аналог присваивания */
int Analog_assignment(bn* Obj1, bn* Obj2)
{
	if (Obj1 == NULL || Obj2 == NULL)
	{
		return BN_NULL_OBJECT;
	}

	Obj1->sign = Obj2->sign;
	
	Obj1->ptr_body = (int*)realloc(Obj1->ptr_body, sizeof(int) * Obj2->size);
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

/* Функция для добавления нулей для приведения к четному размеру массива */
int* bn_add_nulls(int* arr, size_t size, size_t num) 
{
	if (arr == NULL || num == NULL)
	{
		return NULL;
	}

	int* arr_copy = (int*)calloc(num, sizeof(int));
	if (arr_copy == NULL)
	{
		return NULL;
	}

	for (size_t i = 0; i < size; ++i)
	{
		arr_copy[i] = arr[i];
	}

	free(arr);

	return arr_copy;
}

/* Функция для вывода данных о большом числе */
int bn_print(bn const* Obj)
{
	if (Obj == NULL)
	{
		return BN_NULL_OBJECT;
	}
	printf("\nSign = %d\nLength = %d\nAbsolute value = ", Obj->sign, Obj->size);

	for (int i = Obj->size - 1; i != -1; --i) {
		printf("%d", Obj->ptr_body[i]);
	}
	printf("\n");

	return BN_OK;
}

int main()
{
	bn* bn1 = bn_new();	
	bn_init_string(bn1, "34546566");

	bn* bn2 = bn_new();
	bn_init_string(bn2, "-4747474747");

	bn_print(bn1);
	bn_print(bn2);

	bn* Obj_r = bn_mul_to_col(bn1, bn2);
	bn_print(Obj_r);
	return 0;
}

