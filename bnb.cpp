#include "bnb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int NOTATION = 10; // система счисления, с которой записаны числа в массив

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
bn* bn_init(const bn *Obj) {
	if (Obj == NULL) {
		return NULL;
	}

	bn* ptr_cbn = calloc(sizeof(bn), sizeof(int));
	if (ptr_cbn == NULL) {
		return NULL;
	}

	ptr_cbn->size = Obj->size;
	ptr_cbn->sign = Obj->sign;

	ptr_cbn->ptr_body = calloc(ptr_cbn->size, sizeof(int));
	if (ptr_cbn->ptr_body == NULL) {
		free(ptr_cbn);
		return NULL;
	}

	memcpy(ptr_cbn->ptr_body, Obj->ptr_body, Obj->size);
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

	Obj->sign = 1;
	if (str[0] == '-') {
		Obj->sign = -1;
		++i;
	}
	
	Obj->size = length - i;

	int* arr_num = (int*)calloc(Obj->size, sizeof(int));
	if (arr_num != NULL) {
		for (size_t j = 0; j < Obj->size; ++j) {
			arr_num[j] = str[length - j - 1] & 0x0F;
		}
	}

	Obj->ptr_body = arr_num;

	return 0;
}

/* Инициализация числа представлением строки в некоторой системе счисления */
int bn_init_string_radix(bn* Obj, const char* str, int radix) {
	if (Obj == NULL) {
		return BN_NULL_OBJECT;
	}

	return BN_OK;
}

/* Функция для прибавления одного большого числа к другому */
int bn_add_to(bn* Obj1, const bn* Obj2) {
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

	if (Obj1->sign == Obj2->sign) // случай, когда оба знака совпадают
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
	else // случай, когда у чисел разные знаки 
	{
		bn* Obj_c = bn_new();
		Obj_c = bn_init(Obj2); // временная копия Obj2 со знаком +

		Obj_c->sign = -(Obj_c->sign);

		int code =  bn_sub_to(Obj1, Obj_c);
		bn_delete(Obj_c);
		return code;
	}
	
	return BN_OK;
}

/* Функция для вычитания из одного большого числа другое */
int bn_sub_to(bn* Obj1, const bn* Obj2) {
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

int main() {
	bn *bn1 = bn_new();
	bn_init_string(bn1, "-79861676575787645234789058354867946580374654466");

	bn *bn2 = bn_new();
	bn_init_string(bn2, "-85967846765754653554739756945702456438025345687436587");
	
	bn* bn0 = bn_new();

	bn_print(bn1);
	bn_print(bn2);
	int result = bn_add_to(bn1, bn2);
	bn_print(bn1);

	bn_delete(bn1);
	bn_delete(bn2);

	printf("\n%d\n", result);
	return 0;
}
