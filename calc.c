//code written by Shamil Abdurahmanov ID:241ADB070
//my github repository https://github.com/Mightymob0303/Shamil-Abdurahmanov-codes
//instruction how to run the code, visit my github repository and create/use a codespace and use my code there, run 'gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm' in the terminal.
//and write 'echo "2 + 2" > input.txt' in order to create a input file and write a mathematical expression in it, "2+2" is a example you can write whatever you want in your work
//and in the end type "./calc input.txt" to run the code 



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <dirent.h>     // opendir, readdir
#include <sys/stat.h>   // mkdir
#include <string.h>

static size_t error_position = 0;			//position indexes
static size_t lastnumstart = 0;			//index where the most recent number token began, helps us ereport errors when deviding by a 0 
static size_t lastprimestart = 0;			//an index where the most recent primary number began, a primary number in this case will be a parenthesis, this will help us report errors if we are missing _Imaginary

static int is_zero(double x) { return fabs(x) < 1e-15; }	//is_zero is used for float devision so our code doesnt return a inf/NaN
static int is_integral_double(double x) { return fabs(x - llround(x)) < 1e-12; }	//we use is_integral_double this decides if a double is close enough to a proper integer to print it as a integer, i.e 2.9999999999 = 3


int ensure_dir(const char *dirname) {
    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
        if (mkdir(dirname, 0775) != 0) {
            perror("mkdir");
            return 0;
        }
    }
    return 1;
}

static void skipspaces(const char* s, size_t length, size_t* i) {		//we use pointers to point to a string and to the integers inside the string
	while (*i < length && isspace((unsigned char)s[*i])) (*i)++;		 //we check that the size < length and use isspace to return a nonzero value if any whitespace charecters are detected 
}


static double parse_number(const char* s, size_t length, size_t* i) {
	skipspaces(s, length, i);		
	size_t start = *i;


	char* endp = NULL;
	double val = strtod(s + *i, &endp);

	if (endp == s + *i) {		//if endp == s+*i means no charecters were consumed aka there were no numbers at that position 

		if (!error_position) error_position = start + 1;		//we record the position where parsing failed and store it at error_position
		return 0.0;
	}


	lastnumstart = start + 1;


	*i += (size_t)(endp - (s + *i));
	return val;		//returns parsed number

}
static double parse_expression_at(const char* s, size_t length, size_t* i, int stop_at_rparen);		//we create a forward declaration of  parse_expression_at so later the compiler can return the double value with these parameters

static double parse_primary(const char* s, size_t length, size_t* i) {
	skipspaces(s, length, i);		
	if (*i >= length) {			
		if (!error_position) error_position = length + 1;	//correctly place error position 
		return 0;
	}
	if (s[*i] == '(') {		//we check if the expression has a parentheses
		lastprimestart = (*i) + 1;		 //remember where this parenthesized expression started at and store at lastprimestart
		(*i)++;
		double val = parse_expression_at(s, length, i, 1);		//we call the recursive function parse_expression_at which also calls parse_term,which calls parse_primary
		if (error_position)return 0;

		skipspaces(s, length, i);
		if (*i >= length || s[*i] != ')') {
    if (!error_position) {
        // Point at the last parsed number inside these parens (e.g., the '5' in your example).
        // Fallback to end-of-line if we never saw a number.
        error_position = (lastnumstart != 0) ? lastnumstart
                                             : ((*i < length) ? (*i + 1) : (length + 1));
    }
    return 0;
}

		(*i)++;
		return val;
	}
	double val = parse_number(s, length, i);
	if (error_position) return 0;

	lastprimestart = lastnumstart;		//tells the parser,  If this primary is just a number, its start position is the same as the number s start, this is used for precise error reporting
	return val;
}



static double parse_term(const char* s, size_t length, size_t* i) {
    double result = parse_primary(s, length, i);
    if (error_position) return 0;

    while (1) {
        skipspaces(s, length, i);

        // If we're at end *before* seeing an operator, we're done (not an error).
        if (*i >= length) break;

        char op = s[*i];
        if (op != '*' && op != '/') break;

        // Remember where the operator is for precise error reporting.
        size_t op_pos = *i;    // 0-based
        (*i)++;                // consume operator

        // There must be something after the operator.
        skipspaces(s, length, i);
        if (*i >= length) {
            if (!error_position) error_position = op_pos + 1;  // report at the operator (1-based)
            return 0;
        }

        double righthand = parse_primary(s, length, i);
        if (error_position) return 0;

        if (op == '*') {
            result *= righthand;
        } else {
            if (is_zero(righthand)) {
                if (!error_position) error_position = op_pos + 2;
                return 0.0;
            }
            result /= righthand;
        }
    }
    return result;
}


static double parse_expression_at(const char* s, size_t length, size_t* i, int stop_at_rparen) {
	double result = parse_term(s, length, i);		 //we parse the first term
	if (error_position) return 0.0;		//if parsing fails we set the correct error position

	while (1) {
		skipspaces(s, length, i);
		if (*i >= length) break;		//if we reach the end of the input we stop 

		if (s[*i] == ')') {		//if we find the closing parentheses we stop
			if (stop_at_rparen) {

				break;
			}
			else {

				if (!error_position) error_position = *i + 1;
				return 0.0;
			}
		}

		char op = s[*i];
		if (op != '+' && op != '-') {
			if (!error_position) error_position = *i + 1;
			return 0.0;
		}
		(*i)++;

		// Check if there's actually something after the operator
		skipspaces(s, length, i);
		if (*i >= length) {
			if (!error_position) error_position = *i + 1;
			return 0.0;
		}

		double righthand = parse_term(s, length, i);
		if (error_position) return 0.0;

		if (op == '+') result += righthand;
		else           result -= righthand;
	}
	return result;
}


static double evaluate_expression(const char* s, size_t length) {
	size_t idx = 0;
	return parse_expression_at(s, length, &idx, 0);

}

int main(int argc, char** argv) {
	
	const char *outdir = "labs_Sh-Abdurahmanov_241ADB070";
ensure_dir(outdir);
	
	char outpath[512];
snprintf(outpath, sizeof outpath,
         "%s/task1_Shamil_Abdurahmanov_241ADB070.txt", outdir);

	if (argc != 2) {		//Expect exactly one input file argument
		fprintf(stderr, "Usage: %s input.txt\n", argv[0]);
		return 1;
	}
	FILE* f = fopen(argv[1], "rb");	//Open file in binary mode (handles any line endings)
	if (!f) {
		perror("fopen");	//Print system error if file cannot open
		return 1;
	}
	char buffer[20000];		//Buffer large enough for multi - line inputs(up to ~10k chars)
	size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
	fclose(f);
	buffer[n] = '\0';	//Null terminate for safe string operations

	size_t pos = 0;
	while (pos < n) {	//Process each line separately

		size_t line_start = pos;	//Start index of current line
		size_t line_end = pos;		//End index of current line
		while (line_end < n && buffer[line_end] != '\n' && buffer[line_end] != '\r') {
			line_end++;
		}

		const char* line = buffer + line_start;
		size_t line_len = line_end - line_start;


		size_t t = 0;
		skipspaces(line, line_len, &t);
		if (t < line_len) {

			//Reset error tracking for each new line

			error_position = 0;
			lastnumstart = 0;
			lastprimestart = 0;

			double result = evaluate_expression(line, line_len);

			FILE *out = fopen(outpath, "w");
if (!out) { perror("fopen output"); return 1; }

if (error_position)
    fprintf(out, "ERROR:%zu\n", error_position);
else if (is_integral_double(result))
    fprintf(out, "%lld\n", (long long)llround(result));
else
    fprintf(out, "%.15g\n", result);

fclose(out);

		}


		if (line_end < n) {
			if (buffer[line_end] == '\r' && line_end + 1 < n && buffer[line_end + 1] == '\n') {
				pos = line_end + 2;
			}
			else {
				pos = line_end + 1;
			}
		}
		else {
			pos = line_end;
		}
	}
	return 0;
}

// Recursive descent parser https://en.wikipedia.org/wiki/Recursive_descent_parser
//Recursive descent parser https://www.geeksforgeeks.org/compiler-design/recursive-descent-parser/
//floating point guide https://floating-point-gui.de/errors/comparison/
//POSIX fopen(), fread(), fclose() documentation https://man7.org/linux/man-pages/man3/fopen.3.html
// c code basic arithmetic operations https://www.geeksforgeeks.org/c/arithmetic-operators-in-c/
//C Arithmetic Operators https://www.w3schools.com/c/c_operators_arithmetic.php
//c code buffer https://stackoverflow.com/questions/27993971/understanding-buffering-in-c

//parse a string https://stackoverflow.com/questions/924955/parse-a-string-in-c



