#ifndef bnb
#define bnb

struct bn_s;
typedef struct bn_s bn;

enum bn_codes {
	BN_OK, BN_NULL_OBJECT, BN_NO_MEMORY, BN_DIVIDE_BY_ZERO
};

bn* bn_new(); // ������� ����� BN
bn* bn_init(const bn*); // ������� ����� ������������� BN

// ���������������� �������� BN ���������� �������������� ������
int bn_init_string(bn*, const char*);

// ���������������� �������� BN �������������� ������
// � ������� ���������: radix
int bn_init_string_radix(bn*, const char*, int);

// ���������������� �������� BN �������� ����� ������
int bn_init_int(bn*, int);

// ���������� BN (���������� ������)
int bn_delete(bn*);

// ��������, ����������� +=, -=, *=, /=, %=
int bn_add_to(bn*, bn const*);
int bn_sub_to(bn*, bn const*);
int bn_mul_to(bn*, bn const*);
int bn_div_to(bn*, bn const*);
int bn_mod_to(bn*, bn const*);

// �������� ����� � ������� degree
int bn_pow_to(bn*, int);

// ������� ������ ������� reciprocal �� BN
int bn_root_to(bn*, int);

// ������� �������� x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const*, bn const*);
bn* bn_sub(bn const*, bn const*);
bn* bn_mul(bn const*, bn const*);
bn* bn_div(bn const*, bn const*);
bn* bn_mod(bn const*, bn const*);

// ������ ������������� BN � ������� ��������: radix � ���� ������
// ������ ����� ������������� ����������� �������.
const char* bn_to_string(bn const*, int);

// ���� ����� ������, ������� <0; ���� �����, ������� 0; ����� >0
int bn_cmp(bn const*, bn const*);

int bn_neg(bn*); // �������� ���� �� ���������������
int bn_abs(bn*); // ������� ������
int bn_sign(bn const*); //-1 ���� t<0; 0 ���� t = 0, 1 ���� t>0





#endif bnb
