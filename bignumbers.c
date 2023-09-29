//#include <stdio.h>
#include <iostream>
//IF THESE TYPEDEFS CAUSE COMPATIBILITY ISSUES, ERASE THEM; THEY MIGHT OVERRIDE YOUR DEFAULT uintN_t(s)

typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;

typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long uint64_t;

/* 64bit: COMMENT THIS IF YOUR OS IS 32 BITS */

#define max_log 255
#define cap 4294967295
#define word_len 32
#define word_len_log 5

typedef uint32_t half_number;
typedef uint64_t number;

/* 64bit */

/* 32bit: DE-COMMENT THIS IF YOUR OS IS 32 BITS */

/*
#define max_log 511
#define cap 65535
#define word_len 16
#define word_len_log 4

typedef uint16_t half_number;
typedef uint32_t number;
*/

/* 32bit */


// STRUCT DEFINITION: big_number CONTAINS THE SIGN, THE LOG IN BASE 2^word_len AND A POINTER TO THE ACTUAL NUMBER
// THE NUMBER IS STORED IN BASE 2^word_len IN AN ARRAY OF half_number(s) (THE TYPE number IS RESERVED FOR OPERATIONS
// TO AVOID OVERFLOWS)

struct big_number_struct {
	uint8_t sign;					//0 = positive, 1 = negative, -1 = invalid
	uint16_t log;					//indicates last digit != 0 (BUT log(0) = 0)
	half_number digits[max_log + 1];		//little-endian.
};

typedef struct big_number_struct big_number;

// TO INIT A NUMBER DECLARE IT THIS WAY:
//	big_number <varname> = {0};

// IMPLEMENTED OPERATIONS: ADD, SUBTRACT, MULTIPLY, DIVIDE (ALSO RETURNS REMAINDER), POWER, POWERMODN, LEFT_SHIFT, RIGHT_SHIFT, NTHROOT
// void FUNCTIONS ARE IN PLACE

void copy(big_number* dst, big_number* src) {

	//only way to do a deep copy
	//I'm adopting x86 style, hope it's not a problem

	dst->sign = src->sign;
	dst->log = src->log;

	for (uint8_t i = 0; i <= dst->log; i++) {
		dst->digits[i] = src->digits[i];
	}

}

int8_t compare(big_number x1, big_number x2) {

	//returns 1 if x1 > x2, -1 if x1 < x2, 0 if they are equal

	if (x1.log > x2.log) return 1;
	if (x1.log < x2.log) return -1;

	for (int16_t i = x1.log; i >= 0; i--) {
		if (x1.digits[i] > x2.digits[i]) return 1;
		if (x1.digits[i] < x2.digits[i]) return -1;
	}
	return 0;
}

void unsigned_add(big_number *x1, big_number x2) {

	//sets x1 to x1 + x2 if x1 >= x2. Undefined behaviour otherwise
	//
	//the unsigned_ functions ALWAYS want the biggest operand as x1,
	//please never call them directly unless you're sure you're doing it properly

	uint8_t carriage = 0;
	number a, b, x3;

	for (uint16_t i = 0; i <= x2.log; i++) {
		a = x1->digits[i];
		b = x2.digits[i];
		x3 = a + b + carriage;

		carriage = x3 >> word_len;
		x1->digits[i] = x3 & cap;

	}

	for(uint16_t i = x2.log + 1; i <= x1->log; i++) {
		x3 = x1->digits[i] + carriage;
		carriage = x3 >> word_len;
		x1->digits[i] = x3 & cap;
	}

	if (carriage > 0 && x1->log < max_log) {
		x1->log++;
		x1->digits[x1->log] = carriage;
	}

}

void unsigned_add_2(big_number x1, big_number *x2) {
	
	//like unsigned_add but stores the result in x2 instead

	uint8_t carriage = 0;
	number a, b, x3;

	for (uint16_t i = 0; i <= x2->log; i++) {
		a = x1.digits[i];
		b = x2->digits[i];
		x3 = a + b + carriage;

		carriage = x3 >> word_len;
		x2->digits[i] = x3 & cap;
	}

	for(uint16_t i = x2->log + 1; i <= x1.log; i++) {
		x3 = x1.digits[i] + carriage;
		carriage = x3 >> word_len;
		x2->digits[i] = x3 & cap;
	}

	x2->log = x1.log;

	if (carriage > 0 && x2->log < max_log) {
		x2->log++;
		x2->digits[x2->log] = carriage;
	}
}

void unsigned_subtract(big_number *x1, big_number x2) {

	//sets x1 to x1 - x2 if x1 >= x2. Undefined behaviour otherwise
	//
	//the unsigned_ functions ALWAYS want the biggest operand as x1,
	//please never call them directly unless you're sure you're doing it properly

	uint8_t carriage_requested = 0;
	uint16_t results_log = 0;

	number a;
	number b;
	number x3;
	for (uint16_t i = 0; i <= x2.log; i++) {

		a = x1->digits[i];
		b = x2.digits[i];

		if (carriage_requested) {
			if (a > b) {
				x3 = a - b - 1;
				carriage_requested = 0;

			}
			else {
				x3 = a + cap - b;
				//carriage_requested stays = 1
			}
		}
		else {
			if (a >= b) {
				x3 = a - b;
				//carriage_requested stays = 0
			}
			else {
				x3 = a + cap + 1 - b;
				carriage_requested = 1;
			}

		}
		if (x3 != 0) results_log = i;
		x1->digits[i] = x3;
	}
	for (uint16_t i = x2.log + 1; i <= x1->log; i++) {

		a = x1->digits[i];
		b = 0;

		if (carriage_requested) {
			if (a > 0) {
				x3 = a - b - 1;
				carriage_requested = 0;

			}
			else {
				x3 = a + cap - b;
				//carriage_requested stays = 1
			}
		}
		else {
			x3 = a;

		}
		if (x3 != 0) results_log = i;
		x1->digits[i] = x3;
	}
	x1->log = results_log;
	if (carriage_requested) x1->sign = -1;	//the result is invalid

	return;
}

void unsigned_subtract_2(big_number x1, big_number *x2) {
	big_number tmp;
	copy(&tmp, &x1);
	unsigned_subtract(&tmp, *x2);
	copy(x2, &tmp);
}


big_number shift_left(big_number x1, uint16_t word_shift, uint8_t bit_shift) {

	//shifts x1 (word_shift * word_len + bit_shift) bits to the left;
	//this function could be implemented in place but considering its
	//usage in the library that's not my priority concern right now

	big_number result = { 0 };
	result.log = x1.log + word_shift;

	number carriage = 0;
	number a;

	for (uint16_t i = 0; i <= x1.log; i++) {
		a = x1.digits[i];
		a = a << bit_shift;
		a += carriage;

		carriage = a >> word_len;

		result.digits[i + word_shift] = a & cap;
	}

	if (carriage) {
		result.log++;

		result.digits[result.log] = carriage;
	}

	return result;
}

big_number shift_right(big_number x1, uint16_t word_shift, uint8_t bit_shift) {
	big_number result = { 0 };
	result.log = (int32_t)x1.log - (int32_t)word_shift;
	half_number mask = (1 << bit_shift) - 1;

	number carriage = 0;
	number a;
	for(int32_t i = x1.log; i >= word_shift; i--) {
			a = x1.digits[i];
			a += (carriage << word_len);
			
			carriage = a & mask;
			result.digits[i - word_shift] = (a >> bit_shift);
	}
	if(result.digits[result.log] == 0 && result.log > 0) 
		result.log--;

	return result;
}

big_number multiply(big_number x1, big_number x2) {

	//computes x1 * x2 and returns the result

	big_number result = { 0 };
	result.sign = (x1.sign + x2.sign - 2 * x1.sign * x2.sign);

	for (uint16_t i = 0; i <= x2.log; i++) {
		half_number mult = x2.digits[i];

		for (uint8_t j = 0; j < word_len; j++) {
			uint8_t bit = (mult >> j) & 1;	//I basically check the jth bit
			if (bit) {
				
				unsigned_add_2(shift_left(x1, i, j), &result);
			}

		}
	}

	return result;
}


big_number power(big_number x1, uint8_t exp) {	//standard pow is capped at exp = 255 -> it's already gonna be computationally challenging

	//computes x1 ^ exp and returns the result
	//the algorithm is not optimized (n multiplications instead of log(n) + 1)

	big_number result = { 0 };

	result.sign = x1.sign * (exp & 1);

	result.digits[0] = 1;

	for (uint16_t i = 0; i < exp; i++) {
		result = multiply(result, x1);
	}

	return result;
}


big_number divide(big_number* x1, big_number x2) {

	//computes integer division x1 / x2, stores the remainder in x1 and returns the result

	big_number shifted_x2;
	big_number result = { 0 };

	int16_t logdiff = ((int32_t)x1->log) - ((int32_t)x2.log);

	if (logdiff >= 0) result.log = logdiff;

	half_number to_add;

	while (logdiff >= 0) {				//don't worry, it will terminate... eventually

		//std::cout << logdiff << " ";

		int8_t start = 0;
		int8_t end = 31;
		int8_t mid;
		while (start <= end) {
			mid = (end + start) >> 1;
			shifted_x2 = shift_left(x2, logdiff, mid);
			int8_t cmp = compare(*x1, shifted_x2);

			if (cmp == 0) {
				for (uint16_t i = 0; i <= x1->log; i++) {
					x1->digits[i] = 0;
				}
				x1->log = 0;
				//add 2**mid to result

				to_add = 1 << mid;
				result.digits[logdiff] += to_add;

				return result;

			}
			else if (cmp > 0) {
				start = mid + 1;
			}
			else {
				end = mid - 1;
			}

		}

		if (end < 0) {
			logdiff--;
			continue;
		}

		to_add = 1 << end;
		shifted_x2 = shift_left(x2, logdiff, end);
		
		unsigned_subtract(x1, shifted_x2);
	
		result.digits[logdiff] += to_add;

		logdiff = x1->log - x2.log;

	}

	return result;
}

big_number pow_mod(big_number x1, big_number exp, big_number N) {

	//computes x1^exp mod N and returns the result. 
	//I tried to optimize it as best as I could, but I bet 
	//there's still room for improvement. Suggestions are welcome

	big_number result = { 0 };

	result.digits[0] = 1;

	for (uint16_t i = 0; i <= exp.log; i++) {

		half_number e = exp.digits[i];
		uint8_t missing_prod = word_len;

		while (e > 0) {
			uint8_t check = e & 1;

			if (check) {

				result = multiply(result, x1);

				divide(&result, N);
			}

			x1 = multiply(x1, x1);
			//might introduce some optimizations to reduce redundant modulos but not now (1)
			divide(&x1, N);

			missing_prod--;

			e = e >> 1;

		}
		if (i < exp.log) {
			for (uint8_t j = 0; j < missing_prod; j++) {
				x1 = multiply(x1, x1);
				//might introduce some optimizations to reduce redundant modulos but not now (2)
				divide(&x1, N);
			}
			
		}

	}

	return result;
}

big_number nth_root(big_number x1, uint8_t n) {

	//computes the integer nth root of x1
	//room for optimization: power func must be optimized

	big_number result = shift_right(x1, 0, 1);

	uint8_t last_bits = (x1.digits[0] & 3) + 2;
	big_number step = shift_right(x1, 0, 2);
	
	step.digits[0] += last_bits >> 2;

	while(step.log > 0 || step.digits[0] > 0) {

		int8_t cmp = compare(power(result, n), x1);

		if(cmp == 0) 
			return result;
		else if (cmp < 0)
			unsigned_add(&result, step);
		else
			unsigned_subtract(&result, step);

		step = shift_right(step, 0, 1);
	}
	return result;
	
}

// USER-FRIENDLY OPERATIONS: USE THESE IF YOU CAN'T BE BOTHERED TO CHECK THE NUMBERS BUT BE WARE THEY'RE LESS PERFORMANT

big_number add(big_number x1, big_number x2) {
	if (x1.sign == x2.sign) {
		if (compare(x1, x2) >= 0) {
			unsigned_add(&x1, x2);
			return x1;
		}

		unsigned_add(&x2, x1);
		return x2;
	}

	if (compare(x1, x2) >= 0) {
		unsigned_subtract(&x1, x2);
		return x1;
	}
	unsigned_subtract(&x2, x1);
	return x2;
}

big_number subtract(big_number x1, big_number x2) {
	x2.sign = 1 - x2.sign;
	return add(x1, x2);
}
